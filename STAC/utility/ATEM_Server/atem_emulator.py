#!/usr/bin/env python3
"""
ATEM Switcher Emulator
Version: 1.0.0
Python: 3.13.x (latest stable 3.13 release)

Emulates a Blackmagic Design ATEM video switcher for testing STAC devices
configured for ATEM tally integration.  Implements the server side of the ATEM
UDP tally protocol as documented in TallyServer.cpp (AronHetLam).

A single UDP socket multiplexes up to 8 simultaneous STAC connections.
Tally state (off / program / preview) is pushed to all connected clients on
change and maintained alive with periodic keepalive pings.

Protocol reference: TallyServer.cpp (AronHetLam/ATEM_tally_light_with_ESP8266)
STAC reference:     STAC/include/Network/Protocol/ATEMClient.h

Packet structure (every ATEM packet starts with a 12-byte header):
  byte  0  : [flags 7-3][pkt_len MSB 2-0]
  byte  1  : pkt_len LSB          (total length including header)
  bytes 2-3: session_id           (uint16 big-endian; echoed from client HELLO)
  bytes 4-5: ack_target           (packet ID being ACK'd)
  bytes 6-7: resend_id            (packet ID requested for retransmission)
  bytes 8-9: reserved (0x00)
  bytes 10-11: local_packet_id   (sender's outgoing counter, uint16 big-endian)
"""

import copy
import os
import random
import select
import signal
import socket
import sys
import termios
import threading
import time
import tty
from dataclasses import dataclass, field
from datetime import datetime
from enum import Enum
from typing import Dict, List, Optional, Tuple

# Force unbuffered I/O for all output
sys.stdout.reconfigure(line_buffering=False)
sys.stderr.reconfigure(line_buffering=False)

# ─── Protocol constants ────────────────────────────────────────────────────────

ATEM_PORT       = 9910   # Fixed ATEM UDP port (no root required on macOS)
HEADER_LEN      = 12     # ATEM packet header size in bytes

# Flag bits — derived from ATEMbase.h headerCmd enum shifted left 3 into byte[0]
# ATEMbase: byte[0] = (headerCmd << 3) | (lengthMSB & 0x07)
# Extraction: flags = data[0] & 0b11111000  (keeps the 5 flag bits in place)
FLAG_ACK_REQ    = 0x08   # ATEM_headerCmd_AckRequest (0x1 << 3) — please ACK this packet
FLAG_HELLO      = 0x10   # ATEM_headerCmd_HelloPacket (0x2 << 3) — connection request/response
FLAG_RESEND     = 0x20   # ATEM_headerCmd_Resend      (0x4 << 3) — this packet is a resend
FLAG_RESEND_REQ = 0x40   # ATEM_headerCmd_RequestNextAfter (0x8 << 3) — requesting retransmit
FLAG_ACK        = 0x80   # ATEM_headerCmd_Ack         (0x10 << 3) — acknowledgement

HELLO_ACCEPT    = 0x02   # byte[12] in HELLO response — accept
HELLO_REJECT    = 0x03   # byte[12] in HELLO response — reject (no slot)

# ─── Tally state ──────────────────────────────────────────────────────────────

class TallyState(Enum):
    """
    ATEM per-input tally state (matches ATEMClient.h tally flag bits).
      bit 0 = program (ONAIR)
      bit 1 = preview (SELECTED)
    """
    OFF     = 0x00
    PROGRAM = 0x01
    PREVIEW = 0x02

    @property
    def label(self) -> str:
        return {TallyState.OFF: "---",
                TallyState.PROGRAM: "PGM",
                TallyState.PREVIEW: "PVW"}[self]


# ─── Configuration ────────────────────────────────────────────────────────────

@dataclass
class ATEMEmulatorConfig:
    """Runtime configuration for the ATEM emulator.  All fields are mutable
    from the menu system; changes take effect on the next server run."""

    host:               str   = "0.0.0.0"
    port:               int   = ATEM_PORT
    num_inputs:         int   = 20         # ATEM inputs to advertise (1–40)
    max_clients:        int   = 8          # Max simultaneous connections
    keepalive_ms:       int   = 1500       # Keepalive ping interval
    timeout_ms:         int   = 5000       # Client disconnect timeout

    # tally[i] is the state for ATEM input (i + 1); resized when num_inputs changes
    tally: List[TallyState]   = field(default_factory=list)

    # Auto-cycle: rotate PROGRAM through all inputs; next input always gets PREVIEW
    auto_cycle:         bool  = False
    cycle_interval_sec: float = 5.0

    # Error injection
    drop_probability:   float = 0.0        # 0.0–1.0 chance of silently dropping a received packet
    reject_new:         bool  = False      # Force-reject all new HELLO connection attempts

    def __post_init__(self):
        if not self.tally:
            self.tally = [TallyState.OFF] * self.num_inputs

    @property
    def keepalive_sec(self) -> float:
        return self.keepalive_ms / 1000.0

    @property
    def timeout_sec(self) -> float:
        return self.timeout_ms / 1000.0


# ─── Per-client state ─────────────────────────────────────────────────────────

@dataclass
class _Client:
    """Connection state for a single STAC client."""
    addr:            Tuple[str, int]
    session_id:      int
    initialized:     bool  = False
    local_pid:       int   = 0          # Server's outgoing packet-ID counter (per client)
    last_remote_pid: int   = 0          # Most-recent packet ID received from this client
    last_acked_pid:  int   = 0          # Most-recent of our pids that this client ACK'd
    last_recv:       float = field(default_factory=time.time)
    last_send:       float = 0.0
    pkts_sent:       int   = 0
    pkts_recv:       int   = 0
    connect_time:    float = field(default_factory=time.time)


# ─── Session statistics ───────────────────────────────────────────────────────

@dataclass
class _Stats:
    """Cumulative counters for a single emulator.run() session."""
    connections:  int = 0
    rejects:      int = 0
    timeouts:     int = 0
    pkts_sent:    int = 0
    pkts_recv:    int = 0
    tally_pushes: int = 0
    keepalives:   int = 0
    drops:        int = 0


# ─── Packet builders ──────────────────────────────────────────────────────────

def _hello_response(session_id: int, accept: bool) -> bytes:
    """Build a 20-byte HELLO response (accept or reject)."""
    buf = bytearray(20)
    buf[0]  = FLAG_HELLO          # length MSB = 0 (20 < 256)
    buf[1]  = 20
    buf[2]  = (session_id >> 8) & 0xFF
    buf[3]  =  session_id       & 0xFF
    buf[12] = HELLO_ACCEPT if accept else HELLO_REJECT
    return bytes(buf)


def _tally_cmd(flags: List[int]) -> bytes:
    """Build a TlIn command payload (appended after the 12-byte header).

    Format:
      bytes 0-1: command length (10 + num_sources)
      bytes 2-3: 0x00  (reserved)
      bytes 4-7: 'TlIn'
      bytes 8-9: num_sources (uint16 big-endian)
      bytes 10+: one flag byte per input (bit0=pgm, bit1=pvw)
    """
    n = len(flags)
    cmd_len = 10 + n
    cmd = bytearray(cmd_len)
    cmd[0] = (cmd_len >> 8) & 0xFF
    cmd[1] =  cmd_len       & 0xFF
    # bytes 2–3 reserved (0x00)
    cmd[4] = ord('T')
    cmd[5] = ord('l')
    cmd[6] = ord('I')
    cmd[7] = ord('n')
    cmd[8] = (n >> 8) & 0xFF
    cmd[9] =  n       & 0xFF
    for i, f in enumerate(flags):
        cmd[10 + i] = f & 0xFF
    return bytes(cmd)


def _tally_pkt(session_id: int, pid: int, flags: List[int],
               extra_flags: int = 0) -> bytes:
    """Build a complete TlIn packet (12-byte header + TlIn command)
    with the ACK_REQ flag set."""
    cmd   = _tally_cmd(flags)
    total = HEADER_LEN + len(cmd)
    hdr   = bytearray(HEADER_LEN)
    hdr[0]  = (FLAG_ACK_REQ | extra_flags) | ((total >> 8) & 0x07)
    hdr[1]  =  total & 0xFF
    hdr[2]  = (session_id >> 8) & 0xFF
    hdr[3]  =  session_id       & 0xFF
    hdr[10] = (pid >> 8) & 0xFF
    hdr[11] =  pid       & 0xFF
    return bytes(hdr) + cmd


def _ack(session_id: int, ack_id: int) -> bytes:
    """Build a 12-byte ACK packet acknowledging ack_id."""
    buf    = bytearray(HEADER_LEN)
    buf[0] = FLAG_ACK             # length MSB = 0
    buf[1] = HEADER_LEN
    buf[2] = (session_id >> 8) & 0xFF
    buf[3] =  session_id       & 0xFF
    buf[4] = (ack_id >> 8) & 0xFF
    buf[5] =  ack_id       & 0xFF
    return bytes(buf)


def _ping(session_id: int, pid: int) -> bytes:
    """Build a 12-byte ACK_REQ keepalive ping."""
    buf     = bytearray(HEADER_LEN)
    buf[0]  = FLAG_ACK_REQ        # length MSB = 0
    buf[1]  = HEADER_LEN
    buf[2]  = (session_id >> 8) & 0xFF
    buf[3]  =  session_id       & 0xFF
    buf[10] = (pid >> 8) & 0xFF
    buf[11] =  pid       & 0xFF
    return bytes(buf)


# ─── Emulator core ────────────────────────────────────────────────────────────

class ATEMEmulator:
    """
    Single-threaded ATEM UDP server.

    run() blocks until the user presses Enter or 'q'.  Keyboard input is
    multiplexed with UDP receives using select() so no background threads
    are needed for the core server logic.

    Thread safety: _stats is protected by _lock.  _clients and config.tally
    are only accessed from the main (run) thread.
    """

    def __init__(self, config: ATEMEmulatorConfig):
        self.config    = config
        self._clients: Dict[Tuple[str, int], _Client] = {}
        self._sock:    Optional[socket.socket] = None
        self._running  = False
        self._stats    = _Stats()
        self._lock     = threading.Lock()
        self._cycle_idx: int   = 0        # 0-based input index for auto-cycle
        self._last_cycle: float = 0.0

    # ── Logging ───────────────────────────────────────────────────────────────

    @staticmethod
    def _ts() -> str:
        n = datetime.now()
        return n.strftime("[%H:%M:%S.") + f"{n.microsecond // 1000:03d}]"

    def _log(self, msg: str, pfx: str = "") -> None:
        ts = self._ts()
        print(f"{ts} {(pfx + ' ') if pfx else ''}{msg}", flush=True)

    # ── Internal helpers ──────────────────────────────────────────────────────

    def _flag_list(self) -> List[int]:
        """Return the current tally state as a list of flag bytes."""
        return [s.value for s in self.config.tally]

    def _send(self, data: bytes, addr: Tuple[str, int],
              client: Optional[_Client] = None) -> None:
        try:
            self._sock.sendto(data, addr)
            if client:
                client.pkts_sent += 1
                client.last_send  = time.time()
            with self._lock:
                self._stats.pkts_sent += 1
        except OSError as e:
            self._log(f"Send error to {addr}: {e}")

    def _next_pid(self, client: _Client) -> int:
        """Increment and return the server's outgoing packet ID for this client."""
        client.local_pid = (client.local_pid + 1) & 0xFFFF
        return client.local_pid

    # ── Protocol actions ──────────────────────────────────────────────────────

    def _push_tally(self, client: _Client, extra: int = 0) -> None:
        """Send the current tally state to one client as an ACK_REQ TlIn packet."""
        pid = self._next_pid(client)
        self._send(_tally_pkt(client.session_id, pid, self._flag_list(), extra),
                   client.addr, client)

    def _send_ping(self, client: _Client) -> None:
        """Send a 12-byte ACK_REQ keepalive ping to one client."""
        pid = self._next_pid(client)
        self._send(_ping(client.session_id, pid), client.addr, client)
        with self._lock:
            self._stats.keepalives += 1

    def _send_ack(self, client: _Client, remote_pid: int) -> None:
        """Send an ACK acknowledging the client's remote_pid."""
        self._send(_ack(client.session_id, remote_pid), client.addr, client)

    # ── Packet dispatcher ─────────────────────────────────────────────────────

    def _handle_packet(self, data: bytes, addr: Tuple[str, int], now: float) -> None:
        if len(data) < HEADER_LEN:
            return

        # Error injection: random packet drop
        if self.config.drop_probability > 0.0 and \
                random.random() < self.config.drop_probability:
            with self._lock:
                self._stats.drops += 1
            self._log(f"[DROPPED] pkt from {addr[0]}:{addr[1]}", pfx="ERR")
            return

        flags      = data[0] & 0b11111000
        session_id = (data[2] << 8) | data[3]
        ack_target = (data[4] << 8) | data[5]
        resend_id  = (data[6] << 8) | data[7]
        remote_pid = (data[10] << 8) | data[11]

        with self._lock:
            self._stats.pkts_recv += 1

        client = self._clients.get(addr)
        if client:
            client.pkts_recv += 1
            client.last_recv  = now

        if flags & FLAG_HELLO:
            self._on_hello(addr, session_id, now)

        elif client is None:
            return                           # unknown client, not a HELLO — ignore

        elif flags & FLAG_RESEND_REQ:
            if client.initialized:
                self._on_resend(client, resend_id)

        elif flags & FLAG_ACK_REQ:
            if client.initialized:
                client.last_remote_pid = remote_pid
                self._send_ack(client, remote_pid)

        elif flags & FLAG_ACK:
            if not client.initialized:
                self._on_init_ack(client)
            else:
                client.last_acked_pid = ack_target

    def _on_hello(self, addr: Tuple[str, int], session_id: int, now: float) -> None:
        """Handle an incoming HELLO packet from a new or reconnecting client."""
        existing = self._clients.get(addr)
        if existing:
            self._log(f"Re-HELLO from {addr[0]}:{addr[1]} — resetting session")
            del self._clients[addr]

        active = len(self._clients)
        should_reject = self.config.reject_new or active >= self.config.max_clients

        if should_reject:
            self._send(_hello_response(session_id, accept=False), addr)
            with self._lock:
                self._stats.rejects += 1
            reason = "reject_new enabled" if self.config.reject_new \
                     else f"max {self.config.max_clients} clients"
            self._log(f"Rejected    {addr[0]}:{addr[1]}  ({reason})", pfx="<<<")
            return

        client = _Client(addr=addr, session_id=session_id,
                         last_recv=now, connect_time=now)
        self._clients[addr] = client
        self._send(_hello_response(session_id, accept=True), addr, client)
        with self._lock:
            self._stats.connections += 1
        self._log(f"Connected   {addr[0]}:{addr[1]}  session={session_id:#06x}", pfx="<<<")

    def _on_init_ack(self, client: _Client) -> None:
        """First ACK received after HELLO — complete init by sending TlIn + ping.

        TallyServer.cpp reference: after the HELLO exchange the server sends
        the initial TlIn tally data (ACK_REQ) and a bare ACK_REQ ping.  The
        client considers itself initialised when it receives the TlIn.
        """
        client.initialized = True
        self._push_tally(client)
        self._send_ping(client)
        with self._lock:
            self._stats.tally_pushes += 1
        self._log(f"Initialized {client.addr[0]}:{client.addr[1]}"
                  f"  ({self.config.num_inputs} inputs sent)", pfx="   ")

    def _on_resend(self, client: _Client, resend_id: int) -> None:
        """Client requested retransmission — resend TlIn with the original pid."""
        pkt = _tally_pkt(client.session_id, resend_id, self._flag_list(),
                          extra_flags=FLAG_RESEND)
        self._send(pkt, client.addr, client)
        self._log(f"Resent      pid={resend_id}  {client.addr[0]}:{client.addr[1]}",
                  pfx="   ")

    # ── Maintenance: keepalive, timeout, auto-cycle ───────────────────────────

    def _maintenance(self, now: float) -> None:
        """Called every main-loop iteration (~50 ms); handles per-client housekeeping
        and the optional auto-cycle feature."""
        to_drop: List[Tuple[Tuple[str, int], str]] = []

        for addr, client in self._clients.items():
            elapsed = now - client.last_recv

            # Timeout applies to both connecting and initialized clients
            if elapsed > self.config.timeout_sec:
                to_drop.append((addr, f"timeout {elapsed:.1f}s"))
                continue

            if not client.initialized:
                continue

            # Keepalive ping when no packet has been sent recently
            if now - client.last_send > self.config.keepalive_sec:
                self._send_ping(client)

        for addr, reason in to_drop:
            with self._lock:
                self._stats.timeouts += 1
            c = self._clients.pop(addr, None)
            if c:
                self._log(f"Disconnected {addr[0]}:{addr[1]} — {reason}")

        # Auto-cycle: advance PROGRAM each tick; next input gets PREVIEW
        if self.config.auto_cycle and self.config.tally and \
                now - self._last_cycle > self.config.cycle_interval_sec:
            for i in range(len(self.config.tally)):
                self.config.tally[i] = TallyState.OFF
            self._cycle_idx = (self._cycle_idx + 1) % self.config.num_inputs
            pvw_idx = (self._cycle_idx + 1) % self.config.num_inputs
            self.config.tally[self._cycle_idx] = TallyState.PROGRAM
            self.config.tally[pvw_idx] = TallyState.PREVIEW
            self._push_all()
            self._last_cycle = now
            self._log(
                f"Auto-cycle: input {self._cycle_idx + 1} → PROGRAM, "
                f"input {pvw_idx + 1} → PREVIEW",
                pfx="CYC",
            )

    def _push_all(self) -> None:
        """Push the current tally state to all initialized clients."""
        pushed = 0
        for client in list(self._clients.values()):
            if client.initialized:
                self._push_tally(client)
                pushed += 1
        if pushed:
            with self._lock:
                self._stats.tally_pushes += pushed

    # ── Main server loop ──────────────────────────────────────────────────────

    def run(self) -> None:
        """Bind the UDP socket and enter the main select() loop.
        Blocks until the user presses Enter or 'q'."""
        self._sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self._sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        try:
            self._sock.bind((self.config.host, self.config.port))
        except OSError as e:
            self._log(f"Cannot bind to {self.config.host}:{self.config.port}: {e}")
            self._sock.close()
            return

        self._running    = True
        self._cycle_idx  = 0
        self._last_cycle = time.time()
        local_ip         = _local_ip()

        self._log("═" * 65)
        self._log("ATEM Emulator  started")
        self._log(f"Listening:    {local_ip}:{self.config.port}  (UDP)")
        self._log(f"Inputs:       {self.config.num_inputs}   "
                  f"Max clients: {self.config.max_clients}")
        self._log(f"Keepalive:    {self.config.keepalive_ms} ms   "
                  f"Timeout: {self.config.timeout_ms} ms")
        if self.config.auto_cycle:
            self._log(f"Auto-cycle:   every {self.config.cycle_interval_sec:.1f}s")
        if self.config.drop_probability > 0.0:
            self._log(f"Drop prob:    {self.config.drop_probability * 100:.1f}%",
                      pfx="ERR")
        if self.config.reject_new:
            self._log("Reject new:   ENABLED  (new HELLOs will be rejected)", pfx="ERR")
        self._log("Shortcuts:    [num]p=PGM  [num]v=PVW  [num]u=UNSEL  "
                  "P=all PGM  V=all PVW  U=all UNSEL")
        self._log("              s=status  ?=help  q or Enter=stop")
        self._log("═" * 65)

        key_buf = ""
        fd      = sys.stdin.fileno()
        old_tty = termios.tcgetattr(fd)

        try:
            tty.setcbreak(fd)
            while self._running:
                ready, _, _ = select.select([self._sock, sys.stdin], [], [], 0.05)
                now = time.time()

                for src in ready:
                    if src is self._sock:
                        try:
                            data, addr = self._sock.recvfrom(1024)
                            self._handle_packet(data, addr, now)
                        except OSError as e:
                            if self._running:
                                self._log(f"Recv error: {e}")
                    else:
                        ch     = sys.stdin.read(1)
                        result = self._on_key(ch, key_buf)
                        if result is None:
                            self._running = False
                        else:
                            key_buf = result

                self._maintenance(now)

        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_tty)
            self._sock.close()
            self._log("═" * 65)
            self._log("ATEM Emulator  stopped")
            self._log("═" * 65)

    # ── Keyboard handler ──────────────────────────────────────────────────────

    def _on_key(self, ch: str, buf: str) -> Optional[str]:
        """Handle one keypress during run().

        Returns the updated digit-accumulation buffer, or None to signal stop.

        Two-character input scheme for setting individual inputs:
          Type one or two digits (the input number) followed by a state key.
          Examples:  1p → input 1 PROGRAM   12p → input 12 PROGRAM
                     3v → input 3 PREVIEW    2u → input 2 UNSELECTED
        """
        if ch in ('\n', '\r') or ch.lower() == 'q':
            return None                         # stop server

        if ch.isdigit():
            combined = buf + ch
            return combined if len(combined) <= 2 else ch   # cap at 2 digits (max input 40)

        if ch.lower() == 's':
            self._print_status()
            return buf

        if ch == '?':
            self._print_help()
            return buf

        # Bulk state keys: P (all PROGRAM)  V (all PREVIEW)  U (all UNSELECTED)
        # Must be checked BEFORE the single-input handler (which uses ch.lower()
        # and would swallow e.g. uppercase 'P' as a no-op 'p' with empty buffer).
        if ch == 'P':
            for i in range(len(self.config.tally)):
                self.config.tally[i] = TallyState.PROGRAM
            self._push_all()
            self._log("All inputs → PROGRAM", pfx="KEY")
            return ""

        if ch == 'V':
            for i in range(len(self.config.tally)):
                self.config.tally[i] = TallyState.PREVIEW
            self._push_all()
            self._log("All inputs → PREVIEW", pfx="KEY")
            return ""

        if ch == 'U':
            for i in range(len(self.config.tally)):
                self.config.tally[i] = TallyState.OFF
            self._push_all()
            self._log("All inputs → UNSELECTED", pfx="KEY")
            return ""

        lower = ch.lower()

        # Single-input state keys: p / v / o
        if lower in ('p', 'v', 'u'):
            if buf:
                try:
                    inp = int(buf)
                    if 1 <= inp <= self.config.num_inputs:
                        new_state = {'p': TallyState.PROGRAM,
                                     'v': TallyState.PREVIEW,
                                     'u': TallyState.OFF}[lower]
                        self.config.tally[inp - 1] = new_state
                        self._push_all()
                        self._log(f"Input {inp} → {new_state.label}", pfx="KEY")
                    else:
                        self._log(f"Input {inp} out of range "
                                  f"(1–{self.config.num_inputs})", pfx="KEY")
                except ValueError:
                    pass
            return ""

        return buf      # unknown key — keep buffer unchanged

    # ── Display helpers ───────────────────────────────────────────────────────

    def _print_status(self) -> None:
        clients = list(self._clients.values())
        print(f"\n┌─ Status  {len(clients)} client(s) {'─' * 43}")
        for c in clients:
            state   = "INIT" if c.initialized else "CONN"
            elapsed = time.time() - c.connect_time
            print(f"│  {c.addr[0]:<15}:{c.addr[1]:<5d}  [{state}]  "
                  f"session={c.session_id:#06x}  "
                  f"t={elapsed:4.0f}s  tx={c.pkts_sent}  rx={c.pkts_recv}")
        print("│  Tally input states:")
        non_off = [(i + 1, s) for i, s in enumerate(self.config.tally)
                   if s != TallyState.OFF]
        if non_off:
            for inp, s in non_off:
                print(f"│    Input {inp:2d}: {s.label}")
        else:
            print("│    All inputs  OFF")
        print("└" + "─" * 58 + "\n", flush=True)

    @staticmethod
    def _print_help() -> None:
        print("\n┌─ Keyboard shortcuts (while server is running) " + "─" * 16)
        print("│  [num]p   — Set input to PROGRAM      e.g. 1p   12p")
        print("│  [num]v   — Set input to PREVIEW      e.g. 3v")
        print("│  [num]u   — Set input to UNSELECTED   e.g. 2u")
        print("│  P        — All inputs to PROGRAM")
        print("│  V        — All inputs to PREVIEW")
        print("│  U        — All inputs to UNSELECTED")
        print("│  s        — Print live client/tally status")
        print("│  ?        — Show this help")
        print("│  q / Enter — Stop server")
        print("└" + "─" * 63 + "\n", flush=True)

    # ── Public API ────────────────────────────────────────────────────────────

    def get_stats(self) -> _Stats:
        with self._lock:
            return copy.copy(self._stats)


# ─── Utilities ────────────────────────────────────────────────────────────────

def _local_ip() -> str:
    """Best-effort detection of the machine's outbound IP address."""
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        ip = s.getsockname()[0]
        s.close()
        return ip
    except Exception:
        return "127.0.0.1"


def _ask(prompt: str) -> str:
    """Prompt for input with flushed stdout."""
    sys.stdout.flush()
    return input(prompt)


# ─── Menu: configuration ──────────────────────────────────────────────────────

def _config_menu(config: ATEMEmulatorConfig) -> None:
    while True:
        print("\n" + "─" * 65)
        print("  CONFIGURATION")
        print("─" * 65)
        print(f"  Host IP:        {_local_ip()}  (auto-detected)")
        print(f"  Port:           {config.port}  (fixed ATEM UDP port)")
        print(f"  Inputs:         {config.num_inputs}")
        print(f"  Max clients:    {config.max_clients}")
        print(f"  Keepalive:      {config.keepalive_ms} ms")
        print(f"  Timeout:        {config.timeout_ms} ms")
        auto_str = (f"ENABLED — every {config.cycle_interval_sec:.1f}s"
                    if config.auto_cycle else "DISABLED")
        print(f"  Auto-cycle:     {auto_str}")
        print("─" * 65)
        print("  1. Change number of inputs   (1–40)")
        print("  2. Change max clients        (1–8)")
        print("  3. Change keepalive interval")
        print("  4. Change client timeout")
        print("  5. Toggle auto-cycle")
        if config.auto_cycle:
            print("  6. Change auto-cycle interval")
        print("  0. Back")
        print("─" * 65, flush=True)

        choice = _ask("Select option: ").strip()

        if choice == '1':
            try:
                n = int(_ask(f"Number of inputs (1–40, current: {config.num_inputs}): "))
                if 1 <= n <= 40:
                    if n > config.num_inputs:
                        config.tally.extend([TallyState.OFF] * (n - config.num_inputs))
                    else:
                        config.tally = config.tally[:n]
                    config.num_inputs = n
                    print(f"✓ Inputs set to {n}")
                else:
                    print("✗ Must be 1–40")
            except ValueError:
                print("✗ Invalid input")

        elif choice == '2':
            try:
                n = int(_ask(f"Max clients (1–8, current: {config.max_clients}): "))
                if 1 <= n <= 8:
                    config.max_clients = n
                    print(f"✓ Max clients set to {n}")
                else:
                    print("✗ Must be 1–8")
            except ValueError:
                print("✗ Invalid input")

        elif choice == '3':
            try:
                ms = int(_ask(f"Keepalive ms (100–5000, current: {config.keepalive_ms}): "))
                if 100 <= ms <= 5000:
                    config.keepalive_ms = ms
                    print(f"✓ Keepalive set to {ms} ms")
                else:
                    print("✗ Must be 100–5000")
            except ValueError:
                print("✗ Invalid input")

        elif choice == '4':
            try:
                ms = int(_ask(f"Timeout ms (1000–30000, current: {config.timeout_ms}): "))
                if 1000 <= ms <= 30000:
                    config.timeout_ms = ms
                    print(f"✓ Timeout set to {ms} ms")
                else:
                    print("✗ Must be 1000–30000")
            except ValueError:
                print("✗ Invalid input")

        elif choice == '5':
            config.auto_cycle = not config.auto_cycle
            print(f"✓ Auto-cycle {'ENABLED' if config.auto_cycle else 'DISABLED'}")

        elif choice == '6' and config.auto_cycle:
            try:
                s = float(_ask(f"Cycle interval seconds (>0, current: {config.cycle_interval_sec}): "))
                if s > 0:
                    config.cycle_interval_sec = s
                    print(f"✓ Cycle interval set to {s:.1f}s")
                else:
                    print("✗ Must be > 0")
            except ValueError:
                print("✗ Invalid input")

        elif choice == '0':
            break


# ─── Menu: tally state control ────────────────────────────────────────────────

def _tally_menu(config: ATEMEmulatorConfig) -> None:
    while True:
        print("\n" + "─" * 65)
        print("  TALLY STATE CONTROL")
        print("─" * 65)
        print("  Current state  (PGM=program  PVW=preview  ---=off):")

        # Compact grid: 5 inputs per row
        row: List[str] = []
        for i, s in enumerate(config.tally, 1):
            row.append(f"  {i:2d}:{s.label}")
            if len(row) == 5 or i == len(config.tally):
                print("   " + "  ".join(row))
                row = []

        print("─" * 65)
        print("  1. Set individual input")
        print("  2. All → PROGRAM")
        print("  3. All → PREVIEW")
        print("  4. All → OFF")
        print("  5. Pattern: input 1=PGM, 2=PVW, rest=OFF")
        print("  0. Back")
        print("─" * 65, flush=True)

        choice = _ask("Select option: ").strip()

        if choice == '1':
            try:
                inp = int(_ask(f"Input number (1–{config.num_inputs}): "))
                if 1 <= inp <= config.num_inputs:
                    print("  1=PROGRAM  2=PREVIEW  3=OFF")
                    sc = _ask("State: ").strip()
                    m = {'1': TallyState.PROGRAM,
                         '2': TallyState.PREVIEW,
                         '3': TallyState.OFF}
                    if sc in m:
                        config.tally[inp - 1] = m[sc]
                        print(f"✓ Input {inp} → {m[sc].label}")
                    else:
                        print("✗ Invalid choice")
                else:
                    print(f"✗ Must be 1–{config.num_inputs}")
            except ValueError:
                print("✗ Invalid input")

        elif choice == '2':
            for i in range(len(config.tally)):
                config.tally[i] = TallyState.PROGRAM
            print("✓ All inputs → PROGRAM")

        elif choice == '3':
            for i in range(len(config.tally)):
                config.tally[i] = TallyState.PREVIEW
            print("✓ All inputs → PREVIEW")

        elif choice == '4':
            for i in range(len(config.tally)):
                config.tally[i] = TallyState.OFF
            print("✓ All inputs → OFF")

        elif choice == '5':
            for i in range(len(config.tally)):
                config.tally[i] = TallyState.OFF
            if config.num_inputs >= 1:
                config.tally[0] = TallyState.PROGRAM
            if config.num_inputs >= 2:
                config.tally[1] = TallyState.PREVIEW
            print("✓ Pattern set: input 1=PGM  2=PVW  rest=OFF")

        elif choice == '0':
            break


# ─── Menu: error injection ────────────────────────────────────────────────────

def _error_menu(config: ATEMEmulatorConfig) -> None:
    while True:
        print("\n" + "─" * 65)
        print("  ERROR INJECTION")
        print("─" * 65)
        print(f"  Packet drop probability:  {config.drop_probability * 100:.1f}%")
        rej_str = "YES — new HELLOs refused" if config.reject_new else "NO"
        print(f"  Reject new connections:   {rej_str}")
        print("─" * 65)
        print("  1. Set packet drop probability  (0–100%)")
        print("  2. Toggle reject new connections")
        print("  3. Reset all error injection")
        print("  0. Back")
        print("─" * 65, flush=True)

        choice = _ask("Select option: ").strip()

        if choice == '1':
            try:
                pct = float(_ask(
                    f"Drop probability 0–100%  "
                    f"(current: {config.drop_probability * 100:.1f}%): "))
                if 0 <= pct <= 100:
                    config.drop_probability = pct / 100.0
                    print(f"✓ Drop probability set to {pct:.1f}%")
                else:
                    print("✗ Must be 0–100")
            except ValueError:
                print("✗ Invalid input")

        elif choice == '2':
            config.reject_new = not config.reject_new
            state_str = ("ENABLED — all new HELLOs will be rejected"
                         if config.reject_new else "DISABLED")
            print(f"✓ Reject new connections: {state_str}")

        elif choice == '3':
            config.drop_probability = 0.0
            config.reject_new       = False
            print("✓ All error injection reset")

        elif choice == '0':
            break


# ─── Statistics display ───────────────────────────────────────────────────────

def _print_stats(stats: Optional[_Stats]) -> None:
    print("\n" + "=" * 65)
    print("  SESSION STATISTICS")
    print("=" * 65)
    if stats is None:
        print("  No server session recorded yet.")
    else:
        print(f"  Total connections:   {stats.connections}")
        print(f"  Rejected:            {stats.rejects}")
        print(f"  Timeouts:            {stats.timeouts}")
        print(f"  Packets sent:        {stats.pkts_sent}")
        print(f"  Packets received:    {stats.pkts_recv}")
        print(f"  Tally state pushes:  {stats.tally_pushes}")
        print(f"  Keepalives sent:     {stats.keepalives}")
        print(f"  Packets dropped:     {stats.drops}")
    print("=" * 65, flush=True)


# ─── Main menu ────────────────────────────────────────────────────────────────

def _show_menu(config: ATEMEmulatorConfig) -> str:
    print("\n" + "=" * 65)
    print("  ATEM EMULATOR — MAIN MENU")
    print("=" * 65)
    print("  1. Configuration")
    print("  2. Tally State Control")
    print("  3. Error Injection")
    print("  4. Start Server")
    print("  5. View Statistics")
    print("  0. Exit")
    print("=" * 65, flush=True)
    return _ask("Select option (1-5, 0=Exit): ").strip()


# ─── Entry point ──────────────────────────────────────────────────────────────

def main() -> None:
    print("\n" + "=" * 65)
    print("  ATEM SWITCHER EMULATOR  v1.0.0")
    print("  Blackmagic ATEM protocol emulator for STAC testing")
    print("=" * 65)

    config     = ATEMEmulatorConfig()
    last_stats: Optional[_Stats] = None

    def _cleanup() -> None:
        try:
            os.system("stty sane")
        except Exception:
            pass

    def _sigint(sig, frame) -> None:
        print("\n\nInterrupted. Exiting.")
        _cleanup()
        sys.exit(0)

    signal.signal(signal.SIGINT, _sigint)

    while True:
        choice = _show_menu(config)

        if choice == '1':
            _config_menu(config)

        elif choice == '2':
            _tally_menu(config)

        elif choice == '3':
            _error_menu(config)

        elif choice == '4':
            emulator   = ATEMEmulator(config)
            emulator.run()                      # blocks until Enter / q
            last_stats = emulator.get_stats()
            _print_stats(last_stats)
            _cleanup()

        elif choice == '5':
            _print_stats(last_stats)

        elif choice == '0':
            print("\nExiting.")
            _cleanup()
            break

        else:
            print("\n✗ Invalid option")


if __name__ == "__main__":
    main()
