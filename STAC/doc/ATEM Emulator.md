# ATEM Switcher Emulator — Feasibility & Design

**Date:** April 12, 2026  
**Status:** Planning  
**Reference:** `STAC/utility/SmartTally_Server/sts_emulator.py`, `TallyServer.cpp` (AronHetLam/ATEM_tally_light_with_ESP8266)

---

## Overview

This document analyses the feasibility of creating a Python ATEM switcher emulator — analagous to the existing `sts_emulator.py` (Roland STS emulator) — that allows STAC devices running ATEM client firmware to be tested without a physical Blackmagic Design ATEM switcher present.

The ESP32 `ATEM_tally_test_server` in the AronHetLam repository was studied as a reference but is not a standalone ATEM simulator — it is the regular tally light firmware compiled with `#define TALLY_TEST_SERVER` which disables the upstream ATEM connection and instead allows keyboard-driven state changes to be relayed to downstream tally clients via the `TallyServer` library. It does not speak the ATEM protocol as a server. The `TallyServer.cpp` library is the actual reference implementation of the server side of the ATEM protocol, and is the primary source for this analysis.

---

## Feasibility Assessment

**Overall: High.** The server side of the ATEM protocol (as implemented in `TallyServer.cpp`) is straightforward UDP packet handling with a well-defined state machine. Python's `socket` library provides everything needed. The ATEM protocol involves no encryption, no certificates, no complex binary encoding beyond big-endian 16-bit integers, and no authentication. The complete server protocol is extractable from `TallyServer.cpp` (~300 lines of C++).

The Roland STS emulator uses TCP sockets + HTTP. The ATEM emulator uses UDP sockets — a simpler transport with no connection management at the socket level (only at the application-protocol level). Python handles UDP identically simply to TCP from an implementation standpoint.

---

## The ATEM Server-Side Protocol

All packet parsing and construction details below are derived directly from `TallyServer.cpp`.

### Packet Structure

Every ATEM packet begins with a 12-byte header:

```
Byte  0:   [flags: 5 bits][packet_length MSB: 3 bits]
Byte  1:   packet_length LSB
Bytes 2-3: session_id (big-endian uint16)
Bytes 4-5: remote_packet_id / ACK target (big-endian uint16)
Bytes 6-7: resend_packet_id (big-endian uint16, used for resend requests only)
Bytes 8-9: reserved (0x00 0x00)
Bytes 10-11: local_packet_id (big-endian uint16)
```

`packet_length` is the **total** length of the packet including the 12-byte header.

### Flag Values (byte 0 upper 5 bits)

| Flag | Value (byte 0, pre-masked) | Meaning |
|------|---------------------------|---------|
| `HELLO` | `0x08` | New client connecting / server accepting |
| `ACK_REQUEST` | `0x10` | Packet requires acknowledgement |
| `ACK` | `0x80` | Acknowledgement of a received packet |
| `RESEND_REQUEST` | `0x20` | Requesting retransmission of a missed packet |
| `RESENT_PACKAGE` | `0x40` | This packet is a retransmission |

Extracting flags from byte 0: `flags = packet[0] & 0b11111000`

### Connection State Machine

The server (emulator) must track each client through three states:

```
                 ┌─────────────────────────────────┐
                 │  DISCONNECTED                   │
                 │  (no record of this client IP)  │
                 └───────────┬─────────────────────┘
                             │ Receive HELLO packet
                             ▼
                 ┌─────────────────────────────────┐
                 │  CONNECTED                      │  ◄── Send HELLO response (accept/reject)
                 │  (hello exchanged, awaiting ACK)│
                 └───────────┬─────────────────────┘
                             │ Receive ACK
                             │ (server sends TlIn tally command + ACK_REQUEST ping)
                             ▼
                 ┌─────────────────────────────────┐
                 │  INITIALIZED                    │  ◄── Normal operation
                 │  (ready for tally data)         │
                 └─────────────────────────────────┘
```

**Timeout:** Client is dropped if no packet received in 5000ms.

**Keepalive:** Server sends an `ACK_REQUEST` ping every `TALLY_SERVER_KEEP_ALIVE_MSG_INTERVAL` (nominally 1500ms) when no tally data has changed.

### Packet Handling Per State

#### DISCONNECTED → CONNECTED

1. Receive HELLO packet (`flags & 0x08`)
2. If client slots available: store client (IP, port, session_id); send HELLO accept (20 bytes)
3. If no slots: send HELLO reject (20 bytes) — `packet[12] = 0x02` for accept, `0x03` for reject

HELLO response (20 bytes):
```python
packet = bytearray(20)
packet[0] = 0x08 | (20 >> 8 & 0x07)  # HELLO flag + length MSB
packet[1] = 20                          # length LSB
packet[2] = session_id >> 8
packet[3] = session_id & 0xFF
packet[12] = 0x02  # accept (or 0x03 to reject)
```

#### CONNECTED → INITIALIZED

1. Receive ACK packet (`flags & 0x80`)
2. Send tally data command (TlIn) + ACK_REQUEST header (full tally state)
3. Send a second bare ACK_REQUEST ping (12 bytes)
4. Mark client as INITIALIZED

#### INITIALIZED — Normal Operation

| Received | Response |
|----------|----------|
| `ACK` | Record last acked packet ID; no reply |
| `ACK_REQUEST` | Send 12-byte ACK back immediately |
| `RESEND_REQUEST` | Re-send current tally data with the requested packet ID |

**When tally state changes:** Send new TlIn command to all initialized clients.

**Keepalive (periodic):** Send 12-byte ACK_REQUEST ping if `last_sent > keepalive_interval` and client has not acked the most recent tally data.

**Disconnect on timeout:** If `last_recv > 5000ms`, drop client.

### TlIn Tally Data Command

The `TlIn` command carries the tally state of all inputs. It is appended after the 12-byte header:

```python
def build_tally_cmd(tally_flags: list[int]) -> bytes:
    """
    tally_flags: list of per-input flag bytes (bit 0 = program, bit 1 = preview)
    Returns the command payload (to be appended after the 12-byte header).
    """
    num_sources = len(tally_flags)
    cmd_len = 10 + num_sources  # command length field covers cmd header + data
    
    cmd = bytearray(cmd_len)
    cmd[0] = cmd_len >> 8         # cmd length MSB
    cmd[1] = cmd_len & 0xFF       # cmd length LSB
    # cmd[2] and cmd[3] are 0x00 (purpose unknown, not used by client)
    cmd[4] = ord('T')             # command name: "TlIn"
    cmd[5] = ord('l')
    cmd[6] = ord('I')
    cmd[7] = ord('n')
    cmd[8] = num_sources >> 8     # source count MSB
    cmd[9] = num_sources & 0xFF   # source count LSB
    for i, flag in enumerate(tally_flags):
        cmd[10 + i] = flag        # one byte per input: bit 0 = program, bit 1 = preview
    
    return bytes(cmd)
```

Tally flag values:
```python
TALLY_OFF     = 0x00
TALLY_PROGRAM = 0x01
TALLY_PREVIEW = 0x02
TALLY_BOTH    = 0x03  # both program and preview (possible on some models)
```

### Session ID

The ATEM client proposes a temporary session ID (typically `0x53AB`) in the HELLO packet. The server echoes this back. The session ID is then used in all subsequent packets from both sides. The Python emulator should extract and store the client's proposed session ID from bytes [2:4] of the HELLO packet.

### Packet ID Counter

Each side maintains its own **local packet ID counter**, incremented on each `ACK_REQUEST` packet sent. The server only increments its counter on new data packets (not on ACKs, pings, or resends). This counter is tracked per client.

---

## Comparison with STS Emulator

| Dimension | STS Emulator (Roland) | ATEM Emulator |
|-----------|----------------------|---------------|
| Transport | TCP | UDP |
| Protocol | HTTP request/response | Custom binary protocol |
| Server role | Respond to incoming polls | Maintain persistent sessions, push state |
| Client model | One thread per connection | One UDP socket, all clients multiplexed |
| State per client | None (stateless) | Session ID, packet counters, initialized flag |
| Tally delivery | On each poll (pull) | On change + keepalive (push) |
| Number of inputs | Per-channel (keyed by channel number) | Per-input-index (0-based array, up to 40) |
| Authentication | V-160HD/V-80HD: HTTP Basic | None |

The fundamental architectural difference is that the ATEM emulator must maintain **persistent per-client state** and actively push updates, rather than just responding to HTTP polls. This is more complex but well within Python's capabilities.

---

## Python Implementation Design

### Core Architecture

Unlike the STS emulator's one-thread-per-client model (driven by TCP connection lifecycle), the ATEM emulator uses a single UDP socket with a **main loop** processing all incoming datagrams and a background thread for keepalive/timeout management:

```python
class ATEMEmulator:
    def __init__(self, config: ATEMEmulatorConfig):
        self._clients: dict[tuple, ATEMClient] = {}  # (ip, port) -> ATEMClient
        self._tally_flags: list[int] = [TALLY_OFF] * config.num_inputs
        self._sock: socket.socket  # UDP socket bound to port 9910
        self._lock: threading.Lock()

    def run(self):
        # Main receive loop (single thread, non-blocking select())
        while self._running:
            ready = select.select([self._sock], [], [], 0.1)
            if ready[0]:
                data, addr = self._sock.recvfrom(1024)
                self._handle_packet(data, addr)
            self._run_keepalive()  # Check all clients for timeouts/keepalive

    def _handle_packet(self, data: bytes, addr: tuple):
        flags = data[0] & 0b11111000
        session_id = (data[2] << 8) | data[3]
        # ... dispatch to client state machine
```

### Config Structure

```python
@dataclass
class ATEMEmulatorConfig:
    host: str = "0.0.0.0"
    port: int = 9910          # Fixed ATEM port
    num_inputs: int = 20      # Number of tally inputs to advertise
    max_clients: int = 8      # Max simultaneous connections
    keepalive_interval_ms: int = 1500
    client_timeout_ms: int = 5000
    auto_cycle: bool = False
    cycle_interval_sec: float = 5.0
```

### Per-Client State

```python
@dataclass
class ATEMClient:
    ip: str
    port: int
    session_id: int
    is_connected: bool = False     # Hello exchanged
    is_initialized: bool = False   # Init sequence complete
    local_packet_id: int = 0       # Server's outgoing packet counter
    last_remote_packet_id: int = 0 # Client's last sent packet ID
    last_acked_id: int = 0         # Last packet ID ACKed by client
    last_recv_ms: int = 0          # millis() equivalent of last received packet
    last_send_ms: int = 0          # millis() equivalent of last sent packet
```

### Interactive Control (matching STS Emulator style)

The emulator should follow the same menu-driven pattern as `sts_emulator.py`:

- **Main menu:** Configuration, Tally State Control, Error Injection, Start Server, View Statistics
- **Tally State Control:** Set individual input states (UNSELECTED / PREVIEW / PROGRAM), set multi-input patterns, toggle all, per-client mode
- **Error Injection:** Packet drop probability, response delay, simulate "no empty slot" (reject new connections), simulate client timeout
- **Statistics:** Per-client connection info, packets sent/received, tally state history
- **Keyboard shortcuts during running:** Input number + key to change state (e.g. `1p` = input 1 to PROGRAM, `3v` = input 3 to PREVIEW)

---

## What the Emulator Enables

For STAC ATEM development, a Python emulator provides the same development workflow as the STS emulator provides for Roland development:

| Use case | Without emulator | With emulator |
|----------|-----------------|---------------|
| Basic connection testing | Need physical ATEM | Python script on laptop |
| Tally state verification | Need to switch live inputs | Single keypress |
| Connection loss handling | Hard to reproduce | Keyboard-triggered packet drop |
| "Switcher full" error handling | Need 8 other clients | Config option |
| Reconnection logic | Hard to reproduce | Kill/restart emulator |
| Multiple STAC devices simultaneously | Need physical ATEM | Emulator handles all |
| Automated testing integration | Not possible | Script the emulator config |

Notably, the emulator would support **multiple simultaneous STAC connections** from the same laptop — critical for testing peripheral mode relay scenarios or multi-device deployments.

---

## Key Implementation Notes

### UDP Port Binding on macOS

The ATEM protocol uses port 9910. On macOS, binding to a port below 1024 requires root. Port 9910 is fine, but the emulator process needs no special privileges.

If running the emulator on the same machine as a STAC connection test (e.g. a desktop ATEM client for comparison), only one process can bind port 9910. The emulator should document this limitation.

### No Authentication

Unlike the Roland V-160HD emulator path, the ATEM protocol has no authentication. The emulator does not need to implement HTTP Basic Auth or any credential validation. Connection acceptance is purely slot-based (max clients).

### Init Packet Simulation

Real ATEM switchers send a large burst of init packets (model info `_pin`, program/preview input sources `PrgI`/`PrvI`, etc.) before `hasInitialized()` becomes true on the client. The `TallyServer.cpp` reference implementation skips all of this — it sends only the `TlIn` tally command and the client library accepts this as a complete initialisation sequence. The Python emulator should do the same: send only TlIn during init. This is confirmed working by the many existing TallyServer deployments.

### Keepalive Behaviour

The client will disconnect after 5 seconds without receiving a packet. The emulator must send a 12-byte ACK_REQUEST ping to each initialized client every ~1.5 seconds even if tally state has not changed. This is a background task running continuously while the server is active.

### Resend Requests

The ATEMbase client library tracks missed init packets and requests retransmission. Since the emulator sends only a single TlIn (no multi-packet init burst), resend requests will be rare in practice. When received, the emulator should resend the current TlIn command with the requested packet ID. This is a minor edge case but should be handled to avoid client-side error logging.

---

## Implementation Effort

| Component | Estimate |
|-----------|----------|
| Core UDP server + per-client state machine | 1–2 days |
| TlIn packet builder + header construction | 2–3 hours |
| Keepalive + timeout background thread | 2–3 hours |
| Interactive menu (matching STS style) | 1 day |
| Error injection features | 0.5 days |
| Statistics + logging | 0.5 days |
| Testing against STAC firmware on ATOM Matrix | 0.5 days |
| **Total** | **~4–5 days** |

The protocol implementation itself (state machine + packet construction) is the shortest part. The interactive shell, logging, and testing are the bulk of the work — consistent with the STS emulator's development history.

---

## Suggested File Location

Consistent with the existing utility layout:

```
STAC/utility/
├── SmartTally_Server/
│   ├── sts_emulator.py
│   └── STS_EMULATOR_GUIDE.md
└── ATEM_Server/             ← new
    ├── atem_emulator.py
    └── ATEM_EMULATOR_GUIDE.md
```
