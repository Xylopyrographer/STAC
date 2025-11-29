#!/usr/bin/env python3
"""
Roland Smart Tally Server (STS) Emulator
Version: 1.1.0
Python: 3.13.x (latest stable 3.13 release)

A unified emulator for testing STAC devices with Roland V-60HD and V-160HD protocols.
Supports multiple simultaneous STAC connections, error injection, per-STAC random states,
and detailed logging.
"""

import socket
import threading
import time
import random
import string
from datetime import datetime
from dataclasses import dataclass, field
from typing import Dict, Optional, List
from enum import Enum
import sys
import signal
import os
import select
import termios
import tty

# Force unbuffered I/O for all output (needed when run via subprocess)
sys.stdout.reconfigure(line_buffering=False)
sys.stderr.reconfigure(line_buffering=False)


class TallyState(Enum):
    """Tally states matching Roland protocol"""
    UNSELECTED = "unselected"
    SELECTED = "selected"
    ONAIR = "onair"


class SwitcherModel(Enum):
    """Supported Roland video switcher models"""
    V60HD = "V-60HD"
    V160HD = "V-160HD"


@dataclass
class EmulatorConfig:
    """Configuration for the STS emulator"""
    host: str = "0.0.0.0"  # Listen on all interfaces
    port: int = 8080
    model: SwitcherModel = SwitcherModel.V60HD
    
    # V-160HD authentication
    username: str = "user"
    password: str = "0000"
    
    # Channel configuration
    max_channels_v60hd: int = 8
    max_hdmi_channels_v160hd: int = 16
    max_sdi_channels_v160hd: int = 12
    
    # Tally state control
    auto_cycle_enabled: bool = True
    cycle_interval_sec: float = 5.0
    
    # Per-STAC random state mode
    per_stac_random_enabled: bool = False  # When enabled, each STAC IP gets its own random state
    per_stac_states: Dict[str, TallyState] = field(default_factory=dict)  # IP -> State mapping
    per_stac_cycle_enabled: bool = False  # Cycle per-STAC states independently
    
    # Error injection
    response_delay_ms: int = 0  # 0 = no delay
    junk_probability: float = 0.0  # 0.0-1.0 (0% to 100%)
    ignore_count: int = 0  # Number of consecutive requests to ignore
    ignore_triggered: bool = False  # Keyboard trigger for ignore mode
    
    # Channel states (keyed by channel number)
    channel_states: Dict[int, TallyState] = field(default_factory=dict)
    
    def __post_init__(self):
        """Initialize all channels to UNSELECTED"""
        if not self.channel_states:
            max_ch = self.max_channels_v60hd if self.model == SwitcherModel.V60HD else max(
                self.max_hdmi_channels_v160hd, self.max_sdi_channels_v160hd
            )
            self.channel_states = {ch: TallyState.UNSELECTED for ch in range(1, max_ch + 1)}


@dataclass
class RequestStats:
    """Statistics for a single STAC connection"""
    stac_ip: str
    total_requests: int = 0
    ignored_requests: int = 0
    delayed_responses: int = 0
    junk_responses: int = 0
    normal_responses: int = 0
    first_request: Optional[datetime] = None
    last_request: Optional[datetime] = None


class STSEmulator:
    """Main STS Emulator class"""
    
    def __init__(self, config: EmulatorConfig):
        self.config = config
        self.running = False
        self.server_socket: Optional[socket.socket] = None
        self.client_threads: List[threading.Thread] = []
        self.stats: Dict[str, RequestStats] = {}  # Key: STAC IP
        self.stats_lock = threading.Lock()
        self.cycle_thread: Optional[threading.Thread] = None
        self.per_stac_cycle_thread: Optional[threading.Thread] = None
        self.ignore_request_queue: List[tuple] = []  # (timestamp, stac_ip, request)
    
    def _get_state_for_stac(self, stac_ip: str) -> TallyState:
        """Get or assign a consistent random state for a STAC IP"""
        if stac_ip not in self.config.per_stac_states:
            # Use hash of IP to get deterministic but varied assignment
            ip_hash = hash(stac_ip)
            states = list(TallyState)
            assigned_state = states[ip_hash % len(states)]
            self.config.per_stac_states[stac_ip] = assigned_state
            self.log(f"Assigned {assigned_state.value.upper()} to new STAC {stac_ip}", prefix="[RANDOM]")
        return self.config.per_stac_states[stac_ip]
    
    def _cycle_per_stac_states(self):
        """Cycle per-STAC states independently (background thread)"""
        cycle_sequence = [TallyState.UNSELECTED, TallyState.SELECTED, TallyState.ONAIR]
        
        while self.running:
            time.sleep(self.config.cycle_interval_sec)
            
            if not self.running or not self.config.per_stac_cycle_enabled:
                continue
            
            # Cycle each STAC's state
            for stac_ip in list(self.config.per_stac_states.keys()):
                current_state = self.config.per_stac_states[stac_ip]
                current_idx = cycle_sequence.index(current_state)
                next_idx = (current_idx + 1) % len(cycle_sequence)
                next_state = cycle_sequence[next_idx]
                self.config.per_stac_states[stac_ip] = next_state
        
    def timestamp(self) -> str:
        """Get formatted timestamp with milliseconds"""
        now = datetime.now()
        return now.strftime("[%H:%M:%S.%f")[:-3] + "]"
    
    def log(self, message: str, prefix: str = ""):
        """Print timestamped log message"""
        ts = self.timestamp()
        if prefix:
            print(f"{ts} {prefix} {message}", flush=True)
        else:
            print(f"{ts} {message}", flush=True)
    
    def get_local_ip(self) -> str:
        """Get the local IP address"""
        try:
            # Create a socket to determine local IP
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            local_ip = s.getsockname()[0]
            s.close()
            return local_ip
        except:
            return "0.0.0.0"
    
    def start(self):
        """Start the emulator server"""
        self.running = True
        
        # Create server socket
        self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        try:
            self.server_socket.bind((self.config.host, self.config.port))
            self.server_socket.listen(5)
            
            local_ip = self.get_local_ip()
            self.log(f"═══════════════════════════════════════════════════════════")
            self.log(f"STS Emulator Started")
            self.log(f"Model: {self.config.model.value}")
            self.log(f"Listening on: {local_ip}:{self.config.port}")
            if self.config.model == SwitcherModel.V160HD:
                self.log(f"Auth: {self.config.username}:{'*' * len(self.config.password)}")
            if self.config.per_stac_random_enabled:
                self.log(f"Per-STAC Random: ENABLED (cycle: {'ON' if self.config.per_stac_cycle_enabled else 'OFF'})")
            elif self.config.auto_cycle_enabled:
                self.log(f"Auto-cycle: {self.config.cycle_interval_sec}s interval")
            self.log(f"═══════════════════════════════════════════════════════════")
            
            # Start auto-cycle thread if enabled (for channel-based mode)
            if self.config.auto_cycle_enabled and not self.config.per_stac_random_enabled:
                self.cycle_thread = threading.Thread(target=self._auto_cycle_channels, daemon=True)
                self.cycle_thread.start()
            
            # Start per-STAC cycle thread if enabled
            if self.config.per_stac_random_enabled and self.config.per_stac_cycle_enabled:
                self.per_stac_cycle_thread = threading.Thread(target=self._cycle_per_stac_states, daemon=True)
                self.per_stac_cycle_thread.start()
            
            # Accept connections
            self.server_socket.settimeout(1.0)  # Timeout to allow checking self.running
            while self.running:
                try:
                    client_sock, client_addr = self.server_socket.accept()
                    thread = threading.Thread(
                        target=self._handle_client,
                        args=(client_sock, client_addr),
                        daemon=True
                    )
                    thread.start()
                    self.client_threads.append(thread)
                except socket.timeout:
                    continue
                except Exception as e:
                    if self.running:  # Only log if not intentionally shutting down
                        self.log(f"Error accepting connection: {e}")
        
        except Exception as e:
            self.log(f"Error starting server: {e}")
        finally:
            self.stop()
    
    def stop(self):
        """Stop the emulator server"""
        self.running = False
        
        if self.server_socket:
            self.server_socket.close()
        
        self.log("═══════════════════════════════════════════════════════════")
        self.log("STS Emulator Stopped")
        self.log("═══════════════════════════════════════════════════════════")
    
    def _auto_cycle_channels(self):
        """Auto-cycle channel states (background thread)"""
        cycle_sequence = [TallyState.UNSELECTED, TallyState.SELECTED, TallyState.ONAIR]
        
        while self.running:
            time.sleep(self.config.cycle_interval_sec)
            
            if not self.running:
                break
            
            # Cycle each channel through states
            for channel in self.config.channel_states:
                current_state = self.config.channel_states[channel]
                current_idx = cycle_sequence.index(current_state)
                next_idx = (current_idx + 1) % len(cycle_sequence)
                next_state = cycle_sequence[next_idx]
                
                self.config.channel_states[channel] = next_state
                # Auto-cycle state change (not logged to reduce clutter)
    
    def _handle_client(self, client_sock: socket.socket, client_addr: tuple):
        """Handle a single STAC client connection"""
        stac_ip = client_addr[0]
        
        # Initialize stats for this STAC if new
        with self.stats_lock:
            if stac_ip not in self.stats:
                self.stats[stac_ip] = RequestStats(stac_ip=stac_ip)
        
        try:
            # Set socket timeout
            client_sock.settimeout(5.0)
            
            # Read the HTTP request
            request_data = b""
            while b"\r\n\r\n" not in request_data:
                chunk = client_sock.recv(1024)
                if not chunk:
                    break
                request_data += chunk
            
            if not request_data:
                client_sock.close()
                return
            
            # Parse request
            request_str = request_data.decode('utf-8', errors='ignore')
            lines = request_str.split('\r\n')
            request_line = lines[0] if lines else ""
            
            # Update stats
            with self.stats_lock:
                stats = self.stats[stac_ip]
                stats.total_requests += 1
                if stats.first_request is None:
                    stats.first_request = datetime.now()
                stats.last_request = datetime.now()
            
            # Log request
            self.log(f"{stac_ip}: {request_line}", prefix="<--")
            
            # Process request and generate response
            response = self._process_request(request_line, stac_ip)
            
            # Send response
            client_sock.sendall(response.encode('utf-8'))
            
        except socket.timeout:
            self.log(f"Connection timeout: {stac_ip}")
        except Exception as e:
            self.log(f"Error handling client {stac_ip}: {e}")
        finally:
            client_sock.close()
    
    def _process_request(self, request_line: str, stac_ip: str) -> str:
        """Process HTTP request and return response"""
        
        # Check for ignore mode
        if self.config.ignore_count > 0 and self.config.ignore_triggered:
            with self.stats_lock:
                stats = self.stats[stac_ip]
                stats.ignored_requests += 1
                ignore_num = stats.ignored_requests
            
            request_time = self.timestamp()
            self.ignore_request_queue.append((request_time, stac_ip, request_line))
            self.log(f"Ignoring request #{ignore_num} from {stac_ip}: {request_line}", prefix="-->")
            
            # Check if we've reached ignore count
            if ignore_num >= self.config.ignore_count:
                self.config.ignore_triggered = False
                self.log(f"Ignore mode complete ({self.config.ignore_count} requests ignored) - resuming normal operation")
                self.ignore_request_queue.clear()
            
            # Return empty string to simulate network congestion/packet loss
            # No response sent = simulates packets lost in transit
            return ""
        
        # Check for junk data injection
        if self.config.junk_probability > 0 and random.random() < self.config.junk_probability:
            with self.stats_lock:
                self.stats[stac_ip].junk_responses += 1
            
            junk_length = random.randint(0, 80)
            junk_data = ''.join(random.choices(string.ascii_letters + string.digits + string.punctuation, k=junk_length))
            
            self.log(f"Sending junk data to {stac_ip} (length={junk_length})", prefix="-->")
            
            # Return junk based on model
            if self.config.model == SwitcherModel.V60HD:
                # V-60HD: Raw junk data (no HTTP headers)
                return junk_data
            else:
                # V-160HD: Junk wrapped in HTTP 200 OK response matching Roland switch format
                return f"HTTP/1.0 200 OK\r\nServer: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\nContent-type: text/plain\r\n\r\n{junk_data}"
        
        # Check for response delay
        if self.config.response_delay_ms > 0:
            with self.stats_lock:
                self.stats[stac_ip].delayed_responses += 1
            
            request_time = self.timestamp()
            self.log(f"Delaying response by {self.config.response_delay_ms} ms to {stac_ip}", prefix="-->")
            time.sleep(self.config.response_delay_ms / 1000.0)
            
            # Check if server was stopped during delay
            if not self.running:
                self.log(f"Server stopped during delay - dropping response to {stac_ip}", prefix="-->")
                return ""
            
            self.log(f"Replying to request received at {request_time} from {stac_ip}", prefix="-->")
        
        # Normal processing - parse channel from request
        try:
            # Request format: GET /tally/[channel]/status
            # or for V-160HD: GET /tally/[bank]_[channel]/status (e.g., /tally/hdmi_1/status)
            parts = request_line.split()
            if len(parts) < 2:
                return "HTTP/1.1 400 Bad Request\r\n\r\n"
            
            path = parts[1]  # /tally/1/status or /tally/hdmi_1/status
            path_parts = path.split('/')
            
            if len(path_parts) < 4 or path_parts[1] != 'tally':
                return "HTTP/1.1 400 Bad Request\r\n\r\n"
            
            channel_str = path_parts[2]
            
            # Parse channel (may include bank prefix for V-160HD)
            if '_' in channel_str:
                # V-160HD format: hdmi_5 or sdi_3
                bank, ch_num = channel_str.split('_')
                channel = int(ch_num)
            else:
                # V-60HD format: just channel number
                channel = int(channel_str)
            
            # Get state based on mode
            if self.config.per_stac_random_enabled:
                # Per-STAC random mode: each STAC IP gets its own state
                state = self._get_state_for_stac(stac_ip)
                response_text = state.value
            elif channel in self.config.channel_states:
                # Normal channel-based mode
                state = self.config.channel_states[channel]
                response_text = state.value
            else:
                response_text = TallyState.UNSELECTED.value
            
            # Log response
            with self.stats_lock:
                self.stats[stac_ip].normal_responses += 1
            
            self.log(f"{stac_ip}: {response_text}", prefix="   -->")
            
            # Return response based on model
            # V-60HD uses raw text response (no HTTP headers)
            # V-160HD uses full HTTP response matching Roland switch format
            if self.config.model == SwitcherModel.V60HD:
                return response_text
            else:
                return f"HTTP/1.0 200 OK\r\nServer: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\nContent-type: text/plain\r\n\r\n{response_text}"
        
        except (ValueError, IndexError) as e:
            self.log(f"Error parsing request from {stac_ip}: {e}")
            return "HTTP/1.1 400 Bad Request\r\n\r\n"
    
    def print_stats(self):
        """Print connection statistics"""
        print("\n" + "="*70)
        print("CONNECTION STATISTICS")
        print("="*70)
        
        with self.stats_lock:
            if not self.stats:
                print("No connections received yet.")
                return
            
            for stac_ip, stats in sorted(self.stats.items()):
                print(f"\nSTAC: {stac_ip}")
                # Show assigned state if per-STAC random mode
                if self.config.per_stac_random_enabled and stac_ip in self.config.per_stac_states:
                    print(f"  Assigned State:    {self.config.per_stac_states[stac_ip].value.upper()}")
                print(f"  Total Requests:    {stats.total_requests}")
                print(f"  Normal Responses:  {stats.normal_responses}")
                print(f"  Delayed Responses: {stats.delayed_responses}")
                print(f"  Junk Responses:    {stats.junk_responses}")
                print(f"  Ignored Requests:  {stats.ignored_requests}")
                if stats.first_request:
                    print(f"  First Request:     {stats.first_request.strftime('%H:%M:%S.%f')[:-3]}")
                if stats.last_request:
                    print(f"  Last Request:      {stats.last_request.strftime('%H:%M:%S.%f')[:-3]}")
        
        print("="*70 + "\n")


def flush_input(prompt: str) -> str:
    """Helper function for input() that flushes stdout first"""
    sys.stdout.flush()
    return input(prompt)


def show_menu(config: EmulatorConfig):
    """Display interactive menu"""
    print("\n" + "="*70)
    print(" STS EMULATOR - MAIN MENU")
    print("="*70)
    print(f" 1. Configuration")
    print(f" 2. Tally State Control")
    print(f" 3. Error Injection")
    print(f" 4. Start Server")
    print(f" 5. View Statistics")
    print(f" 0. Exit")
    print("="*70, flush=True)
    choice = flush_input("Select option (1-5, 0=Exit): ").strip()
    return choice


def config_menu(config: EmulatorConfig):
    """Configuration submenu"""
    while True:
        print("\n" + "-"*70)
        print(" CONFIGURATION")
        print("-"*70)
        print(f" Current Settings:")
        print(f"   Host IP:    {get_local_ip_simple()} (auto-detected)")
        print(f"   Port:       {config.port}")
        print(f"   Model:      {config.model.value}")
        if config.model == SwitcherModel.V160HD:
            print(f"   Username:   {config.username}")
            print(f"   Password:   {'*' * len(config.password)}")
        print("-"*70)
        print(f" 1. Change Port (current: {config.port})")
        print(f" 2. Change Model (current: {config.model.value})")
        if config.model == SwitcherModel.V160HD:
            print(f" 3. Change V-160HD Username/Password")
        print(f" 0. Back to Main Menu")
        print("-"*70, flush=True)
        
        choice = flush_input("Select option: ").strip()
        
        if choice == '1':
            try:
                port = int(flush_input(f"Enter port number (current: {config.port}): ").strip())
                if 1 <= port <= 65535:
                    config.port = port
                    print(f"✓ Port set to {port}")
                else:
                    print("✗ Invalid port number (must be 1-65535)")
            except ValueError:
                print("✗ Invalid input")
        
        elif choice == '2':
            print("\nSelect model:")
            print("  1. V-60HD")
            print("  2. V-160HD")
            model_choice = flush_input("Choice: ").strip()
            if model_choice == '1':
                config.model = SwitcherModel.V60HD
                print("✓ Model set to V-60HD")
            elif model_choice == '2':
                config.model = SwitcherModel.V160HD
                print("✓ Model set to V-160HD")
            else:
                print("✗ Invalid choice")
        
        elif choice == '3' and config.model == SwitcherModel.V160HD:
            username = flush_input(f"Enter username (current: {config.username}): ").strip()
            password = flush_input("Enter password: ").strip()
            if username:
                config.username = username
            if password:
                config.password = password
            print("✓ Credentials updated")
        
        elif choice == '0':
            break


def tally_control_menu(config: EmulatorConfig):
    """Tally state control submenu"""
    while True:
        print("\n" + "-"*70)
        print(" TALLY STATE CONTROL")
        print("-"*70)
        
        # Show per-STAC random mode status
        if config.per_stac_random_enabled:
            print(f" Mode: PER-STAC RANDOM (each STAC IP gets unique state)")
            print(f" Per-STAC Cycle: {'ENABLED' if config.per_stac_cycle_enabled else 'DISABLED'}")
            if config.per_stac_states:
                print(f"\n Current per-STAC states:")
                for ip, state in sorted(config.per_stac_states.items()):
                    print(f"   {ip}: {state.value.upper()}")
            else:
                print(f"\n No STACs connected yet (states assigned on first request)")
        else:
            print(f" Mode: CHANNEL-BASED (all STACs see same channel states)")
            print(f" Auto-cycle: {'ENABLED' if config.auto_cycle_enabled else 'DISABLED'}")
            if config.auto_cycle_enabled:
                print(f" Cycle interval: {config.cycle_interval_sec} seconds")
            print(f"\n Current channel states:")
            for ch, state in sorted(config.channel_states.items())[:8]:  # Show first 8
                print(f"   Channel {ch:2d}: {state.value.upper()}")
            if len(config.channel_states) > 8:
                print(f"   ... ({len(config.channel_states)} total channels)")
        
        print("-"*70)
        print(f" 1. Toggle Per-STAC Random Mode (currently: {'ON' if config.per_stac_random_enabled else 'OFF'})")
        if config.per_stac_random_enabled:
            print(f" 2. Toggle Per-STAC Cycle (currently: {'ON' if config.per_stac_cycle_enabled else 'OFF'})")
            print(f" 3. Set STAC State Manually")
            print(f" 4. Clear Per-STAC Assignments")
        else:
            print(f" 2. Toggle Auto-cycle (currently: {'ON' if config.auto_cycle_enabled else 'OFF'})")
            print(f" 3. Set Channel State Manually")
            print(f" 4. Reset All Channels to UNSELECTED")
        print(f" 5. Change Cycle Interval (current: {config.cycle_interval_sec}s)")
        print(f" 0. Back to Main Menu")
        print("-"*70, flush=True)
        
        choice = flush_input("Select option: ").strip()
        
        if choice == '1':
            config.per_stac_random_enabled = not config.per_stac_random_enabled
            if config.per_stac_random_enabled:
                print("✓ Per-STAC Random Mode ENABLED")
                print("  Each STAC IP will receive a unique random tally state")
            else:
                print("✓ Per-STAC Random Mode DISABLED")
                print("  All STACs will see the same channel-based states")
        
        elif choice == '2':
            if config.per_stac_random_enabled:
                config.per_stac_cycle_enabled = not config.per_stac_cycle_enabled
                print(f"✓ Per-STAC cycle {'enabled' if config.per_stac_cycle_enabled else 'disabled'}")
            else:
                config.auto_cycle_enabled = not config.auto_cycle_enabled
                print(f"✓ Auto-cycle {'enabled' if config.auto_cycle_enabled else 'disabled'}")
        
        elif choice == '3':
            if config.per_stac_random_enabled:
                # Set state for specific STAC IP
                if not config.per_stac_states:
                    print("✗ No STACs connected yet")
                    continue
                print("\nConnected STACs:")
                ips = sorted(config.per_stac_states.keys())
                for i, ip in enumerate(ips, 1):
                    print(f"  {i}. {ip} ({config.per_stac_states[ip].value.upper()})")
                try:
                    idx = int(flush_input("Select STAC number: ").strip()) - 1
                    if 0 <= idx < len(ips):
                        stac_ip = ips[idx]
                        print("\nSelect state:")
                        print("  1. UNSELECTED")
                        print("  2. SELECTED (Preview)")
                        print("  3. ONAIR (Program)")
                        state_choice = flush_input("Choice: ").strip()
                        state_map = {'1': TallyState.UNSELECTED, '2': TallyState.SELECTED, '3': TallyState.ONAIR}
                        if state_choice in state_map:
                            config.per_stac_states[stac_ip] = state_map[state_choice]
                            print(f"✓ {stac_ip} set to {state_map[state_choice].value.upper()}")
                        else:
                            print("✗ Invalid choice")
                    else:
                        print("✗ Invalid selection")
                except (ValueError, IndexError):
                    print("✗ Invalid input")
            else:
                # Original channel state setting
                try:
                    ch = int(flush_input("Enter channel number: ").strip())
                    if ch not in config.channel_states:
                        print(f"✗ Invalid channel (range: 1-{len(config.channel_states)})")
                        continue
                    
                    print("\nSelect state:")
                    print("  1. UNSELECTED")
                    print("  2. SELECTED")
                    print("  3. ONAIR")
                    state_choice = flush_input("Choice: ").strip()
                    
                    state_map = {'1': TallyState.UNSELECTED, '2': TallyState.SELECTED, '3': TallyState.ONAIR}
                    if state_choice in state_map:
                        config.channel_states[ch] = state_map[state_choice]
                        print(f"✓ Channel {ch} set to {state_map[state_choice].value.upper()}")
                    else:
                        print("✗ Invalid choice")
                except ValueError:
                    print("✗ Invalid input")
        
        elif choice == '4':
            if config.per_stac_random_enabled:
                config.per_stac_states.clear()
                print("✓ All per-STAC assignments cleared (new states will be assigned on next request)")
            else:
                for ch in config.channel_states:
                    config.channel_states[ch] = TallyState.UNSELECTED
                print("✓ All channels reset to UNSELECTED")
        
        elif choice == '5':
            try:
                interval = float(flush_input(f"Enter cycle interval in seconds (current: {config.cycle_interval_sec}): ").strip())
                if interval > 0:
                    config.cycle_interval_sec = interval
                    print(f"✓ Cycle interval set to {interval}s")
                else:
                    print("✗ Interval must be positive")
            except ValueError:
                print("✗ Invalid input")
        
        elif choice == '0':
            break


def error_injection_menu(config: EmulatorConfig):
    """Error injection submenu"""
    while True:
        print("\n" + "-"*70)
        print(" ERROR INJECTION")
        print("-"*70)
        print(f" Current Settings:")
        print(f"   Response Delay:    {config.response_delay_ms} ms")
        print(f"   Junk Probability:  {config.junk_probability * 100:.1f}%")
        print(f"   Ignore Count:      {config.ignore_count} requests")
        print(f"   Ignore Triggered:  {'YES (press key during test)' if config.ignore_triggered else 'NO'}")
        print("-"*70)
        print(f" 1. Set Response Delay (0-30000 ms)")
        print(f" 2. Set Junk Data Probability (0-100%)")
        print(f" 3. Set Ignore Request Count")
        print(f" 4. Arm Ignore Trigger (press key during test to start)")
        print(f" 5. Reset All Error Injection")
        print(f" 0. Back to Main Menu")
        print("-"*70, flush=True)
        
        choice = flush_input("Select option: ").strip()
        
        if choice == '1':
            try:
                delay = int(flush_input(f"Enter response delay in milliseconds (0-30000, current: {config.response_delay_ms}): ").strip())
                if 0 <= delay <= 30000:
                    config.response_delay_ms = delay
                    print(f"✓ Response delay set to {delay} ms")
                else:
                    print("✗ Delay must be 0-30000 ms")
            except ValueError:
                print("✗ Invalid input")
        
        elif choice == '2':
            try:
                prob = float(flush_input(f"Enter junk probability (0-100%, current: {config.junk_probability * 100:.1f}): ").strip())
                if 0 <= prob <= 100:
                    config.junk_probability = prob / 100.0
                    print(f"✓ Junk probability set to {prob}%")
                else:
                    print("✗ Probability must be 0-100")
            except ValueError:
                print("✗ Invalid input")
        
        elif choice == '3':
            try:
                count = int(flush_input(f"Enter number of requests to ignore (0=disabled, current: {config.ignore_count}): ").strip())
                if count >= 0:
                    config.ignore_count = count
                    config.ignore_triggered = False
                    print(f"✓ Ignore count set to {count}")
                    if count > 0:
                        print("  Use option 4 to arm the trigger, then press any key during test to start ignoring")
                else:
                    print("✗ Count must be >= 0")
            except ValueError:
                print("✗ Invalid input")
        
        elif choice == '4':
            if config.ignore_count > 0:
                config.ignore_triggered = True
                print(f"✓ Ignore trigger ARMED - will ignore next {config.ignore_count} requests when you press a key during test")
            else:
                print("✗ Set ignore count first (option 3)")
        
        elif choice == '5':
            config.response_delay_ms = 0
            config.junk_probability = 0.0
            config.ignore_count = 0
            config.ignore_triggered = False
            print("✓ All error injection settings reset")
        
        elif choice == '0':
            break


def get_local_ip_simple() -> str:
    """Simple local IP detection"""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except:
        return "127.0.0.1"


def main():
    """Main program entry point"""
    print("\n" + "="*70)
    print("  ROLAND STS EMULATOR v1.0")
    print("  Smart Tally Server Emulator for STAC Testing")
    print("="*70)
    
    # Create default configuration
    config = EmulatorConfig()
    emulator: Optional[STSEmulator] = None
    server_thread: Optional[threading.Thread] = None
    
    # Terminal cleanup function
    def cleanup_terminal():
        """Reset terminal to normal mode"""
        try:
            os.system('stty sane')
        except:
            pass
    
    # Handle Ctrl+C gracefully
    def signal_handler(sig, frame):
        print("\n\nShutting down...")
        if emulator:
            emulator.stop()
        cleanup_terminal()
        sys.exit(0)
    
    signal.signal(signal.SIGINT, signal_handler)
    
    while True:
        choice = show_menu(config)
        
        if choice == '1':
            config_menu(config)
        
        elif choice == '2':
            tally_control_menu(config)
        
        elif choice == '3':
            error_injection_menu(config)
        
        elif choice == '4':
            print("\nStarting server...")
            emulator = STSEmulator(config)
            server_thread = threading.Thread(target=emulator.start, daemon=True)
            server_thread.start()
            
            # Monitor keyboard for spacebar (trigger ignore) and Enter (stop server)
            print("\nServer running.")
            if config.ignore_count > 0:
                print("Press SPACEBAR to trigger ignore mode, ENTER to stop server...")
            else:
                print("Press ENTER to stop server...")
            
            # Set terminal to raw mode for immediate key detection
            fd = sys.stdin.fileno()
            old_settings = termios.tcgetattr(fd)
            try:
                tty.setcbreak(fd)  # Use cbreak mode (not raw) to get immediate input without echo
                
                running = True
                while running:
                    # Check if input is available (non-blocking)
                    if select.select([sys.stdin], [], [], 0.1)[0]:
                        key = sys.stdin.read(1)
                        
                        if key == '\n' or key == '\r':  # Enter key
                            running = False
                        elif key == ' ' and config.ignore_count > 0:  # Spacebar
                            if not config.ignore_triggered:
                                config.ignore_triggered = True
                                # Reset ignore counters for all STACs
                                with emulator.stats_lock:
                                    for stats in emulator.stats.values():
                                        stats.ignored_requests = 0
                                print(f"\n[SPACEBAR] Ignore mode triggered - will ignore next {config.ignore_count} requests")
                            else:
                                print("\n[SPACEBAR] Ignore mode already active")
            finally:
                termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
            
            print()  # Newline after key press
            
            if emulator:
                emulator.stop()
                emulator.print_stats()
                emulator = None
            
            # Reset terminal after stopping
            cleanup_terminal()
        
        elif choice == '5':
            if emulator and emulator.running:
                emulator.print_stats()
            else:
                print("\n✗ No server running - statistics unavailable")
        
        elif choice == '0':
            print("\nExiting...")
            if emulator:
                emulator.stop()
            cleanup_terminal()
            break
        
        else:
            print("\n✗ Invalid option")


if __name__ == "__main__":
    main()
