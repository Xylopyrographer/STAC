# ATEM Emulator User's Guide

## Overview

The ATEM Emulator is a testing tool for STAC devices that simulates a Blackmagic Design ATEM video switcher. It provides an interactive, menu-driven interface that implements the server side of the ATEM UDP tally protocol, allowing full end-to-end testing of STAC's ATEM integration without a physical switcher.

**Version:** 1.0.0  
**Python:** 3.13.x  
**Dependencies:** None (uses Python standard library only)

---

## Features

### Core Functionality
- **ATEM Protocol**: Full server-side ATEM UDP tally protocol (HELLO → ACK → TlIn handshake)
- **Multi-STAC Support**: Handle up to 8 simultaneous STAC connections
- **Auto-Cycling**: Automatically rotate the PROGRAM input through all channels on a timer
- **Manual Control**: Set individual input states on demand via menu or live keyboard shortcuts
- **Configurable Inputs**: Emulate 1–40 ATEM inputs

### Error Injection
- **Packet Drop Probability**: Silently drop incoming packets with configurable probability (0–100%)
- **Reject New Connections**: Force-reject all new HELLO connection attempts to test STAC's reconnect behavior

### Logging & Monitoring
- **Timestamped Logs**: Millisecond-precision timestamps on all events
- **Live Status**: Print current client connections and tally state on demand (`s` key)
- **Session Statistics**: Cumulative counters after each server run

---

## Quick Start

### Installation

1. Ensure Python 3.13.x is installed:
   ```bash
   python3 --version
   ```

2. Navigate to the emulator directory:
   ```bash
   cd /path/to/STAC3/STAC/utility/ATEM_Server
   ```

3. Run the emulator:
   ```bash
   python3 atem_emulator.py
   ```

### Basic Usage

1. **Configure Settings** (Menu option 1) — optional, defaults are usually fine
   - Number of inputs defaults to 20 (must be ≥ the highest channel number configured on your STAC)
   - Port is fixed at 9910 (the ATEM UDP port)

2. **Start Server** (Menu option 4)
   - Server begins listening for STAC connections on UDP port 9910
   - Press **ENTER** or **q** to stop the server

3. **Configure the STAC Device**
   - In STAC web config, set Switch Model to **ATEM**
   - Set ATEM Switcher IP Address to the emulator's displayed IP address
   - STAC will connect and receive tally state

---

## Menu System

### Main Menu

```
=================================================================
  ATEM EMULATOR — MAIN MENU
=================================================================
  1. Configuration
  2. Tally State Control
  3. Error Injection
  4. Start Server
  5. View Statistics
  0. Exit
=================================================================
```

Navigation:
- Enter the number of your choice and press **ENTER**
- **0** returns to main menu from submenus, or exits from main menu

---

## Configuration Menu

### Settings Overview

**Host IP**: Auto-detected local network IP address  
**Port**: Fixed at 9910 (standard ATEM UDP port — no root privileges required on macOS)  
**Inputs**: Number of ATEM inputs to advertise (1–40, default 20)  
**Max Clients**: Maximum simultaneous STAC connections (1–8, default 8)  
**Keepalive**: Interval between keepalive pings to each client (100–5000 ms, default 1500 ms)  
**Timeout**: Client disconnect timeout (1000–30000 ms, default 5000 ms)  
**Auto-Cycle**: Automatically rotate PROGRAM through all inputs on a timer

### Options

**1. Change Number of Inputs**
- Set how many ATEM inputs to advertise in the `TlIn` tally command (1–40)
- Must be ≥ the highest channel number you want to test on STAC
- STAC channels beyond `num_inputs` will show as UNSELECTED (no tally flag byte)

**2. Change Max Clients**
- Maximum simultaneous STAC connections (1–8)
- New HELLOs beyond the limit are automatically rejected

**3. Change Keepalive Interval**
- How often to send ACK_REQ keepalive pings to each client (100–5000 ms)
- Default 1500 ms; STAC times out at ~5000 ms

**4. Change Client Timeout**
- How long to wait without receiving a packet before dropping a client (1000–30000 ms)

**5. Toggle Auto-Cycle**
- Enables/disables the auto-cycle feature (see below)

**6. Change Auto-Cycle Interval** *(only shown when auto-cycle is enabled)*
- Seconds between PROGRAM advances (> 0, default 5.0 s)

---

## Tally State Control

### Auto-Cycle Mode

When enabled, a single PROGRAM input automatically advances through all inputs on the configured interval:

```
Input 1 → PROGRAM (all others OFF)
   ↓  (after interval)
Input 2 → PROGRAM (all others OFF)
   ↓
...
Input N → PROGRAM (all others OFF)
   ↓
Input 1 → PROGRAM (wraps around)
```

**Behavior:**
- Only one input is PROGRAM at a time; all others are OFF
- PREVIEW is not used in auto-cycle
- Each advance is logged with the `CYC` prefix
- Tally state is pushed to all connected clients on each advance

**Use Case:** Continuous hands-free testing — watch STAC cycle through each channel number displaying ONAIR (red).

### Pre-Run Tally Setup (Menu Option 2)

Set the initial tally state before starting the server. Changes here take effect when the server starts and is pushed on first connection.

| Option | Action |
|--------|--------|
| 1. Set individual input | Choose input number (1–N) and state |
| 2. All → PROGRAM | All inputs to PROGRAM (red) |
| 3. All → PREVIEW | All inputs to PREVIEW (green) |
| 4. All → OFF | All inputs to OFF (unselected) |
| 5. Pattern | Input 1=PGM, 2=PVW, rest=OFF — useful baseline test |

**Tally states:**
- **PROGRAM (PGM)**: Input is on air → STAC shows ONAIR (red)
- **PREVIEW (PVW)**: Input is in preview → STAC shows SELECTED (green)
- **OFF (---)**: Input not selected → STAC shows UNSELECTED (blue/dark)

---

## Error Injection

Simulate error conditions to test STAC resilience.

### Packet Drop Probability

**Purpose**: Test STAC keepalive/reconnect behavior when UDP packets are lost  
**Range**: 0–100%  
**Behavior**: The emulator silently ignores received packets with the given probability

**What STAC Shows:**
- Continues displaying last known tally state while packets are dropped
- If no keepalive ping is received within the timeout window, STAC reconnects

**Testing Strategy:**
- Set to 25% for intermittent loss simulation
- Set to 75–100% to force keepalive timeout

### Reject New Connections

**Purpose**: Simulate a full switcher (no connection slots available)  
**How It Works**: All incoming HELLO packets receive a REJECT response  
**What STAC Shows**: STAC will retry the connection handshake

**Use Case:** Verify STAC gracefully retries after a rejected HELLO.

### Reset All Error Injection

Sets all error injection to defaults:
- Drop probability: 0%
- Reject new connections: disabled

---

## Running the Server

### Starting the Server

Select option **4** from the main menu. The server displays its configuration and begins listening:

```
═════════════════════════════════════════════════════════════════
ATEM Emulator  started
Listening:    192.168.2.58:9910  (UDP)
Inputs:       20   Max clients: 8
Keepalive:    1500 ms   Timeout: 5000 ms
Auto-cycle:   every 5.0s
Shortcuts:    [num]p=PGM  [num]v=PVW  [num]o=OFF  A=all PGM  a=all OFF
              s=status  ?=help  q or Enter=stop
═════════════════════════════════════════════════════════════════
```

### Live Keyboard Controls

While the server is running, keystrokes control the emulator in real time. **Key presses are not echoed** — this is intentional to keep the log output clean.

#### Single-Input State Control

Type one or two digits (the input number) followed by a state letter:

| Keystroke | Action | Example |
|-----------|--------|---------|
| `[num]p` | Set input to PROGRAM | `1p`, `12p` |
| `[num]v` | Set input to PREVIEW | `3v`, `15v` |
| `[num]o` | Set input to OFF | `2o`, `8o` |

The tally state is immediately pushed to all connected clients.

#### Bulk State Keys

| Key | Action |
|-----|--------|
| `A` | All inputs → PROGRAM |
| `a` | All inputs → OFF |

#### Other Keys

| Key | Action |
|-----|--------|
| `s` | Print live client/tally status |
| `?` | Show keyboard shortcut help |
| `q` or **ENTER** | Stop server and return to main menu |

### Live Status Display (`s` key)

```
┌─ Status  2 client(s) ───────────────────────────────────
│  192.168.2.27 :56321  [INIT]  session=0x1234  t=  45s  tx=63  rx=120
│  192.168.2.31 :44812  [INIT]  session=0x5678  t=   8s  tx=11  rx=21
│  Tally input states:
│    Input  1: PGM
│    Input  3: PVW
└──────────────────────────────────────────────────────────
```

**Fields:**
- `[INIT]` — fully initialized client (has received TlIn);
	- `[CONN]` — HELLO received but init not yet complete
- `session` — ATEM session ID echoed from client's HELLO
- `t` — seconds since connection
- `tx` / `rx` — packets sent/received per client

### Log Output

The emulator displays real-time activity:

```
[14:32:01.412] <<< Connected   192.168.2.27:56321  session=0x1234
[14:32:01.414]     Initialized 192.168.2.27:56321  (20 inputs sent)
[14:32:06.415] CYC Auto-cycle: input 2 → PROGRAM
[14:32:11.416] CYC Auto-cycle: input 3 → PROGRAM
[14:32:15.201] KEY Input 5 → PGM
```

**Log Format:**
- `[HH:MM:SS.mmm]` — Timestamp with milliseconds
- `<<<` — Incoming connection event
- `   ` — Server-initiated event (tally push, keepalive, etc.)
- `CYC` — Auto-cycle advance
- `KEY` — Live keyboard input
- `ERR` — Error injection event (drop, reject)

---

## Session Statistics

After stopping the server (option 5 or shown automatically after option 4):

```
=================================================================
  SESSION STATISTICS
=================================================================
  Total connections:   3
  Rejected:            0
  Timeouts:            1
  Packets sent:        847
  Packets received:    512
  Tally state pushes:  62
  Keepalives sent:     312
  Packets dropped:     0
=================================================================
```

**Metrics:**
- **Total Connections**: HELLO handshakes accepted
- **Rejected**: HELLOs refused (max clients exceeded or reject-new enabled)
- **Timeouts**: Clients dropped due to no traffic exceeding timeout window
- **Packets Sent**: Total UDP datagrams sent (data + keepalives + ACKs)
- **Packets Received**: Total UDP datagrams received
- **Tally State Pushes**: `TlIn` commands sent (initial + updates)
- **Keepalives Sent**: Bare ACK_REQ ping packets sent
- **Packets Dropped**: Packets silently discarded by error injection

---

## Protocol Details

### ATEM UDP Protocol Overview

STAC uses the ATEM server-side UDP tally protocol documented in TallyServer.cpp (AronHetLam). All communication uses a single UDP socket on **port 9910**.

**Packet Header (12 bytes):**

| Bytes | Field | Description |
|-------|-------|-------------|
| 0 | `[flags 7-3][len MSB 2-0]` | Flag bits in upper 5 bits; packet length MSB in lower 3 bits |
| 1 | `len LSB` | Packet total length including header |
| 2–3 | `session_id` | uint16 big-endian; assigned on HELLO, echoed by both sides |
| 4–5 | `ack_target` | Packet ID being acknowledged |
| 6–7 | `resend_id` | Packet ID requested for retransmission |
| 8–9 | reserved | Always 0x00 |
| 10–11 | `local_packet_id` | Sender's outgoing counter, uint16 big-endian |

**Flag bits** (derived from `ATEMbase.h` `headerCmd << 3`):

| Flag | Value | Meaning |
|------|-------|---------|
| `FLAG_ACK_REQ` | `0x08` | Please acknowledge this packet (`AckRequest`, `0x1 << 3`) |
| `FLAG_HELLO` | `0x10` | Connection request or response (`HelloPacket`, `0x2 << 3`) |
| `FLAG_RESEND` | `0x20` | This packet is a retransmission |
| `FLAG_RESEND_REQ` | `0x40` | Requesting retransmission of a packet |
| `FLAG_ACK` | `0x80` | Acknowledgement (no payload) |

### Connection Handshake

```
STAC                            Emulator
  │── HELLO (FLAG_HELLO) ──────────▶│  Client requests connection
  │◀─ HELLO + ACCEPT ───────────────│  Server accepts (or rejects)
  │── ACK (FLAG_ACK) ──────────────▶│  Client acknowledges
  │◀─ TlIn (FLAG_ACK_REQ) ──────────│  Server sends initial tally data
  │◀─ PING (FLAG_ACK_REQ) ──────────│  Server sends keepalive ping
  │── ACK (FLAG_ACK) ──────────────▶│  Client acknowledges TlIn
  │── ACK (FLAG_ACK) ──────────────▶│  Client acknowledges ping
  │          [connected]            │
```

### Tally Command (`TlIn`)

The `TlIn` command is appended after the 12-byte header:

| Bytes | Content |
|-------|---------|
| 0–1 | Command length (10 + num_inputs) |
| 2–3 | Reserved (0x00) |
| 4–7 | `'T' 'l' 'I' 'n'` (ASCII) |
| 8–9 | num_sources (uint16 big-endian) |
| 10+ | One flag byte per input (bit 0 = PROGRAM, bit 1 = PREVIEW) |

**Tally flag byte values:**

| Value | Meaning |
|-------|---------|
| `0x00` | Not selected (OFF) |
| `0x01` | PROGRAM (on air) |
| `0x02` | PREVIEW (selected) |
| `0x03` | Both PROGRAM and PREVIEW |

---

## Testing Scenarios

### Scenario 1: Normal Operation — Single Channel

**Goal**: Verify STAC correctly displays ONAIR/PREVIEW/UNSELECTED for a specific channel

**Setup:**
1. Tally State Control → All → OFF
2. Start server
3. Press `5p` to set input 5 to PROGRAM

**Expected Behavior:**
- STAC channel 5 shows ONAIR (red)
- Press `5v` → STAC shows SELECTED (green)
- Press `5o` → STAC shows UNSELECTED (blue)

---

### Scenario 2: Auto-Cycle — All Channels

**Goal**: Verify tally display for every input number

**Setup:**
1. Configuration → Set inputs to match your test count (e.g. 20)
2. Configuration → Enable auto-cycle, interval 3–5 s
3. Start server

**Expected Behavior:**
- STAC cycles through each channel number displaying ONAIR
- All other channels display UNSELECTED

**Pass Criteria:**
- Each channel renders correctly for its bank/digit display
- Bank indicator corners (purple) display correctly during bank selection

---

### Scenario 3: Multi-STAC Test

**Goal**: Verify emulator handles multiple STACs simultaneously

**Setup:**
1. Configure 2+ STAC devices to the same emulator IP
2. Enable auto-cycle
3. Start server

**Expected Behavior:**
- Both STACs appear in the log with different IP addresses
- All STACs receive the same tally pushes simultaneously

**Pass Criteria:**
- All STACs display identical tally states
- `s` key shows all connections as `[INIT]`

---

### Scenario 4: Connection Loss Test

**Goal**: Test STAC behavior when the switcher stops responding

**Setup:**
1. Error Injection → Set drop probability to 100%
2. Start server, wait for STAC to connect

**Expected Behavior:**
- STAC stops receiving keepalive pings
- STAC attempts to reconnect after its timeout window
- Log shows re-HELLO from the client

**Pass Criteria:**
- STAC reconnects automatically after loss
- Log shows `Re-HELLO from ...` when STAC reconnects

---

### Scenario 5: Reject New Connections

**Goal**: Test STAC behavior when HELLO is rejected

**Setup:**
1. Error Injection → Enable reject new connections
2. Start server with an unconfigured STAC

**Expected Behavior:**
- Emulator sends HELLO REJECT to STAC
- STAC retries the connection

**Pass Criteria:**
- Log shows `Rejected ...` for each HELLO attempt
- STAC does not crash or freeze

---

### Scenario 6: ATEM Channel > 20

**Goal**: Verify STAC bank-select display for channels beyond the first bank

**Setup:**
1. Configuration → Set inputs to 40
2. Start server
3. Press `25p` to set input 25 to PROGRAM

**Expected Behavior:**
- STAC channel 25 shows ONAIR (red)
- STAC bank indicator shows bank 3 (purple corners)
- STAC requires 2 button presses to reach channel 25: bank to 20, then select +5

---

## Troubleshooting

### Emulator Won't Start

**Error**: `Cannot bind to 0.0.0.0:9910`

**Solutions:**
- Another process is using UDP port 9910 — check with `lsof -nP -iUDP | grep 9910`
- Stop any actual ATEM switcher software running on the same machine
- A previous crashed emulator instance may still have the port open — wait a few seconds and retry

---

### STAC Not Connecting

**Checklist:**
1. Verify STAC and emulator are on the same network
2. Check the emulator's displayed IP address matches what is configured in STAC
3. Port is always 9910 — confirm STAC is not configured for a different port
4. Ensure firewall is not blocking UDP port 9910
5. Confirm STAC has a valid WiFi connection and is in ATEM mode (not Roland)

**Debug Steps:**
1. Start the emulator
2. Note the IP shown: `Listening: 192.168.2.58:9910  (UDP)`
3. Configure STAC web config with that exact IP, Switch Model = ATEM
4. Watch emulator log for `<<< Connected` within a few seconds of STAC power-on or reconnect

---

### STAC Shows Channel as UNSELECTED When It Should Be ONAIR

**Cause A**: `num_inputs` in the emulator is less than the channel number

- Example: emulator has 10 inputs but STAC is configured for channel 15
- `TlIn` sends only 10 flag bytes; STAC gets no flag byte for input 15 → shows UNSELECTED

**Fix:** Configuration → Set inputs to a value ≥ your highest STAC channel number

**Cause B**: Tally state not set for that input

- Check with `s` key or Tally State Control menu

---

### Auto-Cycle Not Advancing

**Cause**: Auto-cycle may have been left disabled from a previous session

**Fix:** Configuration → Toggle auto-cycle → verify status shows `ENABLED`

---

### Terminal Looks Garbled After Stopping

**Cause**: An unexpected crash may have left the terminal in raw mode

**Fix:**
```bash
stty sane
```

The emulator runs `stty sane` automatically on clean exit, but a hard kill may bypass this.

---

### Keys Not Working While Server Running

**Expected Behavior**: Keys are not echoed to screen in raw (cbreak) mode

**Note**: This is intentional — prevents input characters from cluttering the log output. Type your input and watch the log for confirmation (e.g. `KEY Input 5 → PGM`).

---

## Appendix: Keyboard Shortcut Reference

```
┌─ Keyboard shortcuts (while server is running) ────────────────
│  [num]p   — Set input to PROGRAM   e.g. 1p   12p
│  [num]v   — Set input to PREVIEW   e.g. 3v
│  [num]o   — Set input to OFF        e.g. 2o
│  A        — All inputs to PROGRAM
│  a        — All inputs to OFF
│  s        — Print live client/tally status
│  ?        — Show this help
│  q / Enter — Stop server
└───────────────────────────────────────────────────────────────
```

Input numbers are 1–40 (two digits max). Unrecognized keys are silently ignored.


<!-- EOF -->
