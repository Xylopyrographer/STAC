# ATEM Protocol Reference

> **Note:** Blackmagic Design does not publish official documentation for the ATEM switcher
> protocol. Everything in this document has been derived by reverse engineering done by
> Kasper Skårhøj (SKAARHOJ K/S) and subsequently extended by Aron N. Het Lam.
> The primary reference implementations are:
>
> - [SKAARHOJ Open Engineering – ATEMbase](https://github.com/kasperskaarhoj/SKAARHOJ-Open-Engineering/tree/master/ArduinoLibs/ATEMbase)  
> - [AronHetLam – ATEM Tally Light with ESP8266/ESP32](https://github.com/AronHetLam/ATEM_tally_light_with_ESP8266)
>
> Both are licensed under the **GNU General Public License v3.0**.

---

## 1. Transport Layer

| Property | Value |
|---|---|
| Protocol | UDP |
| Switcher listening port | **9910** |
| Client source port | Random, chosen at connection time (default range: 50100–65300) |
| Direction | Bidirectional; all client packets are sent to the switcher's port 9910 |

The ATEM protocol is **stateful** and **push-based**. Once a client successfully connects
and initializes, the switcher proactively sends state change notifications without polling.
The client must run a receive loop (`runLoop()`) continuously to process incoming packets
and send timely ACKs.

---

## 2. Packet Structure

Every ATEM packet consists of a **12-byte header** optionally followed by one or more
**command segments**.

### 2.1 Header Layout

```
Byte  Field                       Notes
----  --------------------------  ----------------------------------------------------
 0    Flags (bits 7-3) +          flags[4:0] << 3 | (packet_length >> 8 & 0x07)
      Length MSB (bits 2-0)
 1    Length LSB                  Total packet length in bytes, including this header
 2    Session ID (MSB)            16-bit session ID, big-endian
 3    Session ID (LSB)
 4    Remote Packet ID (MSB)      ID of the last remote packet being ACK'd, big-endian
 5    Remote Packet ID (LSB)
 6    Resend Packet ID (MSB)      Used only with the RequestNextAfter flag, big-endian
 7    Resend Packet ID (LSB)        (ATEMbase subtracts 1 from the requested ID when
                                    sending; TallyServer adds 1 back on receipt)
 8    Reserved                    0x00  (ATEMbase places 0x01 here for RequestNextAfter)
 9    Reserved / Hello marker     0x3a in client Hello; 0x03 in client ACK after Hello
10    Local Packet ID (MSB)       Sender's own incrementing counter, big-endian
11    Local Packet ID (LSB)
```

**Packet length** is encoded across bytes 0–1:
```
packet_length = ((buffer[0] & 0x07) << 8) | buffer[1]
```

**Flags** occupy the upper five bits of byte 0 and are read back by shifting right three:
```
headerBitmask = buffer[0] >> 3     (ATEMbase client convention)
```
Or by masking directly when the flag constants are pre-shifted (TallyServer convention):
```
flags = buffer[0] & 0b11111000     (TallyServer convention)
```

Both encodings represent identical on-wire values; the difference is only in how the library
constants are defined (see section 3).

### 2.2 Session ID

- The client proposes a temporary session ID of `0x53AB` in its initial Hello packet.
- The switcher assigns a real session ID in its Hello response and the client updates its
  local copy from bytes 2–3 of every incoming packet.
- The session ID is echoed back in every subsequent packet in both directions.

### 2.3 Local Packet ID Counter

- Each sender maintains its own incrementing 16-bit counter.
- The counter **wraps at 32768** (`1 << 15`, i.e., `ATEM_maxPacketId`), not at 65536.
- The counter is **not incremented** for Hello, ACK-only, or RequestNextAfter packets.

---

## 3. Flag Values

The five flag bits occupy the upper five bits of byte 0 (bits 7–3).

### 3.1 SKAARHOJ / ATEMbase constants (pre-shift, applied when writing with `<< 3`)

| Constant | Pre-shift value | Value in byte 0 | Meaning |
|---|---|---|---|
| `ATEM_headerCmd_AckRequest` | `0x01` | `0x08` | Recipient must ACK this packet |
| `ATEM_headerCmd_HelloPacket` | `0x02` | `0x10` | Connection handshake packet |
| `ATEM_headerCmd_Resend` | `0x04` | `0x20` | This packet is a resent copy |
| `ATEM_headerCmd_RequestNextAfter` | `0x08` | `0x40` | Request a missed packet be resent |
| `ATEM_headerCmd_Ack` | `0x10` | `0x80` | Pure acknowledgement (no payload) |

ATEMbase writes to byte 0 as:
```cpp
_packetBuffer[0] = (headerCmd << 3) | (highByte(lengthOfData) & 0x07);
```
ATEMbase reads flags as:
```cpp
uint8_t headerBitmask = _packetBuffer[0] >> 3;
if (headerBitmask & ATEM_headerCmd_HelloPacket) { … }
```

### 3.2 TallyServer constants (post-shift, placed directly into byte 0)

| Constant | Value in byte 0 | Corresponds to |
|---|---|---|
| `TALLY_SERVER_FLAG_ACK_REQUEST` | `0b00001000` = `0x08` | `ATEM_headerCmd_AckRequest` |
| `TALLY_SERVER_FLAG_HELLO` | `0b00010000` = `0x10` | `ATEM_headerCmd_HelloPacket` |
| `TALLY_SERVER_FLAG_RESENT_PACKAGE` | `0b00100000` = `0x20` | `ATEM_headerCmd_Resend` |
| `TALLY_SERVER_FLAG_RESEND_REQUEST` | `0b01000000` = `0x40` | `ATEM_headerCmd_RequestNextAfter` |
| `TALLY_SERVER_FLAG_ACK` | `0b10000000` = `0x80` | `ATEM_headerCmd_Ack` |

TallyServer writes to byte 0 as:
```cpp
_buffer[0] = (flags) | (lengthOfData >> 8) & 0b00000111;
```
TallyServer reads flags as:
```cpp
uint8_t flags = _buffer[0] & 0b11111000;
if (flags & TALLY_SERVER_FLAG_HELLO) { … }
```

> **Note on naming:** SKAARHOJ `ATEM_headerCmd_Resend` (a packet the switcher is
> re-sending to the client) maps to TallyServer `TALLY_SERVER_FLAG_RESENT_PACKAGE`.
> SKAARHOJ `ATEM_headerCmd_RequestNextAfter` (a client request for a resend) maps to
> TallyServer `TALLY_SERVER_FLAG_RESEND_REQUEST`. The internal naming conventions differ
> between the two implementations.

---

## 4. Connection Handshake

The handshake uses a three-step sequence: **Hello → Server Hello-ACK → Client ACK**.

### Step 1 — Client Hello (client → switcher)

```
Byte  Value   Notes
----  ------  ----------------------------------------
 0    0x10    HelloPacket flag (0x02 << 3), length MSB = 0
 1    0x14    Packet length = 20 (0x14)
 2    0x53    Session ID MSB (proposed: 0x53AB)
 3    0xAB    Session ID LSB
 4    0x00    Remote Packet ID = 0
 5    0x00
 6    0x00    (unused)
 7    0x00
 8    0x00    (unused)
 9    0x3A    Marker byte; present in initial Hello only
10    0x00    Local Packet ID = 0 (not incremented for Hello)
11    0x00
12    0x01    Connection request byte (1 = REQUEST)
13–19 0x00    Padding
```

### Step 2 — Server Hello Response (switcher → client)

The switcher echoes a 20-byte Hello packet back with a new session ID assigned.

```
Byte  Value   Notes
----  ------  ----------------------------------------
 0    0x10    HelloPacket flag
 1    0x14    Packet length = 20
 2    xx      Session ID MSB (assigned by switcher)
 3    xx      Session ID LSB
 4–9  0x00    (varies; byte 9 is not set to a special value server-side)
10    0x00    Local Packet ID = 0
11    0x00
12    0x02    Connection status: 2 = ACCEPTED, 3 = REJECTED (server full)
13–19 0x00    
```

> **Connection byte 12 values:**
> - `1` = Connection request (sent by client)
> - `2` = Connection accepted (sent by switcher)
> - `3` = Connection rejected — switcher has no free client slots
> - `4` = Reconnect attempt (observed when the switcher tries to recover a dropped session)
>
> Byte 15 in the server response reportedly increments by ~3 for each new connection
> attempt made since the switcher booted; its exact semantics are unknown.

### Step 3 — Client Acknowledgement (client → switcher)

After receiving the server Hello, the client sends a plain 12-byte ACK:

```
Byte  Value   Notes
----  ------  ----------------------------------------
 0    0x80    Ack flag (0x10 << 3)
 1    0x0C    Packet length = 12
 2    xx      Session ID (as received from switcher)
 3    xx
 4    0x00    Remote Packet ID (ACK for packet 0)
 5    0x00
 6–8  0x00
 9    0x03    Marker byte; present in this ACK only
10    0x00    Local Packet ID = 0 (not incremented for ACK)
11    0x00
```

After this exchange, `isConnected()` returns `true` and the switcher begins sending the
initialization burst.

---

## 5. Initialization Phase

After the handshake completes, the switcher sends a burst of packets containing the full
current state of the switcher (all input sources, tally states, keyer settings, etc.).
These are numbered with consecutive Local Packet IDs starting from 1.

### 5.1 Client-side tracking

The client maintains a missed-packet bitmap (`_missedInitializationPackages`) with one bit
per possible init packet, initialized to all-1s (all presumed missing). Each packet
received during init clears its bit.

The init burst ends when the client receives a plain 12-byte ping packet from the switcher
(after having received at least one init packet with ID > 1). At that point,
`_initPayloadSent` is set to `true` and `_initPayloadSentAtPacketId` records the count of
init packets sent.

### 5.2 Requesting missed packets

After the burst ends, the client scans its bitmap and requests any missed packets using
`RequestNextAfter`:

```
Byte  Value   Notes
----  ------  ----------------------------------------
 0    0x40    RequestNextAfter flag (0x08 << 3)
 1    0x0C    Packet length = 12
 2    xx      Session ID
 3    xx
 4    0x00    Remote Packet ID = 0
 5    0x00
 6    xx      Missed packet ID - 1 (MSB)  ← Note: one less than wanted ID
 7    xx      Missed packet ID - 1 (LSB)
 8    0x01    Marker
 9–11 0x00
```

The switcher responds by re-sending the requested packet (with the `Resend` flag set).

### 5.3 Initialization complete

Once all bits in the missed-packet bitmap are cleared, `_hasInitialized` is set to `true`
and `hasInitialized()` returns `true`. At this point, the switcher connection is operational
and real-time state updates will arrive as they occur.

**Constants:**
- Maximum init packets tracked: `ATEM_maxInitPackageCount = 40`
- Packet buffer length: `ATEM_packetBufferLength = 96` bytes

### 5.4 TallyServer simplified initialization

The TallyServer (the ATEM-emulating server in AronHetLam's project) uses a simplified
initialization:

1. On receiving the client ACK (Step 3 above), the server immediately sends one TlIn
   command packet followed by a plain 12-byte ACK request.
2. The client marks itself as `_isInitialized` after sending its ACK to the init data
   packet — no missed-packet recovery is needed.

This is a valid subset of the full ATEM init protocol. Clients built on ATEMbase accept it
correctly because `_initPayloadSent` triggers when the 12-byte-only packet is received.

---

## 6. Normal Operation

### 6.1 ACK protocol

- Any packet with `AckRequest` set requires a response from the receiver.
- The ACK is a plain 12-byte packet with the `Ack` flag and the sender's Remote Packet ID
  field set to the ID being acknowledged.
- Packets that are not ACK'd within ~250 ms are resent by the TallyServer.

### 6.2 Keepalive

| Condition | Action |
|---|---|
| `TALLY_SERVER_KEEP_ALIVE_MSG_INTERVAL` (1500 ms) elapsed since last send | Send a 12-byte `AckRequest` ping |
| 5000 ms elapsed since last receive | Disconnect the client and reset state |

The reconnection loop in ATEMbase also triggers a full `connect()` if 5000 ms passes
without any contact from the switcher.

### 6.3 Resend handling

If the switcher sends a `RequestNextAfter` packet asking for a command the client
previously sent, the client responds with an empty `AckRequest` packet stamped with the
requested packet ID (rather than the real resent data). This prevents certain ATEM models
from crashing when they do not receive a response within 63 subsequent commands.

---

## 7. Command Segment Format

Packets longer than 12 bytes contain one or more **command segments** immediately after
the header. Each segment has an 8-byte sub-header:

```
Offset  Size  Field            Notes
------  ----  ---------------  ----------------------------------------
  0      2    Segment length   Total segment length in bytes (includes this 8-byte header)
  2      2    Flags/unknown    Typically 0x00 0x00
  4      4    Command name     4 ASCII characters, e.g. "TlIn", "PrgI"
  8+     N    Payload          Command-specific data
```

Multiple command segments can be concatenated within a single packet.  
After the 12-byte ATEM header, command parsing proceeds:

```python
index = 12
while index < packet_length:
    cmd_length = (buf[index] << 8) | buf[index+1]
    cmd_name   = buf[index+4 : index+8].decode('ascii')
    payload    = buf[index+8 : index+cmd_length]
    index     += cmd_length
```

---

## 8. Commands Reference

### 8.1 Tally by Index — `TlIn` (switcher → client)

The most important command for tally light applications. Sent during initialization and
whenever any tally state changes.

**Absolute byte layout within a TlIn packet:**

```
Byte   Field                Notes
-----  -------------------  -------------------------------------------
12–13  Segment length       = 10 + numSources  (big-endian)
14–15  0x00 0x00            Unknown
16–19  "TlIn"               ASCII command name
20–21  numSources           Number of tally sources (uint16_t, big-endian)
22+    tallyFlags[0..N-1]   One byte per source, index = input number - 1
```

**Tally flag bits:**

| Bit | Value | Meaning |
|---|---|---|
| 0 | `0x01` | Input is on PROGRAM |
| 1 | `0x02` | Input is on PREVIEW |
| — | `0x03` | Input is on both PROGRAM and PREVIEW |
| — | `0x00` | Input is neither program nor preview |

Tally indices are **zero-based** (index 0 = ATEM Input 1).

**Example — 4-source TlIn packet (total 26 bytes):**

```
Byte  Hex   Meaning
----  ----  -------------------------
  0   0x08  AckRequest, length MSB=0
  1   0x1A  Total length = 26
  2   0xAB  Session ID MSB
  3   0xCD  Session ID LSB
  4   0x00  Remote Packet ID = 0
  5   0x00
  6   0x00
  7   0x00
  8   0x00
  9   0x00
 10   0x00  Local Packet ID
 11   0x01
 12   0x00  Segment length MSB
 13   0x0E  Segment length = 14 (= 10 + 4 sources)
 14   0x00
 15   0x00
 16   0x54  'T'
 17   0x6C  'l'
 18   0x49  'I'
 19   0x6E  'n'
 20   0x00  numSources MSB
 21   0x04  numSources = 4
 22   0x03  Input 1: Program + Preview
 23   0x00  Input 2: neither
 24   0x01  Input 3: Program only
 25   0x02  Input 4: Preview only
```

### 8.2 Tally by Source — `TlSr` (switcher → client)

Similar to `TlIn` but uses the switcher's internal source ID rather than input index.
Each entry is 3 bytes: 2-byte source ID (big-endian) + 1-byte tally flag.

This command is less commonly used for simple tally lights; `TlIn` is preferred.

### 8.3 Program Input — `PrgI` (switcher → client)

Sent when the program bus selection changes.

```
Payload offset  Field           Notes
--------------  --------------- ----------------------------------------
 0              mE              Mix/Effects bus (0 = ME1, 1 = ME2)
 1              (padding)       0x00
 2–3            videoSource     Active program input (see Video Source IDs)
```

### 8.4 Preview Input — `PrvI` (switcher → client)

Same structure as `PrgI`; reports the preview bus selection.

```
Payload offset  Field           Notes
--------------  --------------- ----------------------------------------
 0              mE              Mix/Effects bus (0 = ME1, 1 = ME2)
 1              (padding)       0x00
 2–3            videoSource     Active preview input (see Video Source IDs)
```

### 8.5 Product Info — `_pin` (switcher → client, init only)

Identifies the switcher model. Sent once during initialization.

The payload is a null-terminated ASCII product name string (e.g., "ATEM 1 M/E Production
Switcher"). Model detection logic in ATEMmin examines specific character offsets within the
string:

| `payload[5]` | `payload[29]` | `_ATEMmodel` | Switcher type |
|---|---|---|---|
| `'T'` | — | 0 | Television Studio |
| `'1'` | `'4'` | 4 | ATEM 1 M/E 4K |
| `'1'` | other | 1 | ATEM 1 M/E |
| `'2'` | `'4'` | 5 | ATEM 2 M/E 4K |
| `'2'` | other | 2 | ATEM 2 M/E |
| `'P'` | — | 3 | Production Studio 4K |

### 8.6 Transition Position — `TrPs` (switcher → client)

Reports the current state of a mix/effects transition.

```
Payload offset  Field                Notes
--------------  -------------------- ----------------------------------------
 0              mE                   Mix/Effects bus (0 = ME1, 1 = ME2)
 1              inTransition         1 if a transition is in progress
 2              framesRemaining      Frames left in transition
 3              (padding)
 4–5            position             Transition position 0–9999
```

### 8.7 Keyer On Air — `KeOn` (switcher → client)

```
Payload offset  Field      Notes
--------------  ---------- ----------------------------------------
 0              mE         Mix/Effects bus
 1              keyer      Keyer index 0–3
 2              enabled    1 if keyer is on air
```

### 8.8 Downstream Keyer Status — `DskS` (switcher → client)

```
Payload offset  Field                  Notes
--------------  ---------------------- ----------------------------------------
 0              keyer                  0 = DSK1, 1 = DSK2
 1              onAir                  1 if downstream keyer is on air
 2              inTransition           1 if in transition
 3              isAutoTransitioning    1 if auto-transitioning
 4              framesRemaining        Frames remaining in transition
```

### 8.9 Fade-To-Black State — `FtbS` (switcher → client)

```
Payload offset  Field            Notes
--------------  ---------------- ----------------------------------------
 0              mE               Mix/Effects bus
 1              fullyBlack       1 if the output is fully black
 2              inTransition     1 if FtbS transition is in progress
 3              framesRemaining  Frames remaining
```

### 8.10 Aux Source — `AuxS` (switcher → client)

```
Payload offset  Field       Notes
--------------  ----------- ----------------------------------------
 0              auxChannel  Aux bus index 0–5
 1              (unknown)
 2–3            input       Currently routed video source
```

### 8.11 Streaming Status — `StRS` (switcher → client)

Added by AronHetLam; reports ATEM streaming status for models with built-in streaming.

```
Payload offset  Field                Notes
--------------  -------------------- ----------------------------------------
 0–1            statusFlags          Bit field (see below)
```

| Bit | Meaning |
|---|---|
| 0 | Stream idle |
| 1 | Stream connecting |
| 2 | Currently streaming |
| 4 | Invalid state |
| 5 | Stream stopping (still streaming) |
| 15 | Unknown streaming error |

### 8.12 Initialization Complete — `InCm` (switcher → client)

Sent by some ATEM firmware versions to mark the end of the initialization burst. Not
universally reliable as an end marker in all firmware versions; ATEMbase relies on
detecting the plain 12-byte ping packet instead.

---

## 9. Set Commands (client → switcher)

These commands are sent by the client inside `AckRequest` packets. The payload is placed
at offset 12 (the same command segment structure as section 7), constructed by
`_prepareCommandPacket()` and sent by `_finishCommandPacket()`.

| Command | Name string | Description |
|---|---|---|
| `CPgI` | Set Program Input | `mE` (byte 0), `videoSource` (bytes 2–3) |
| `CPvI` | Set Preview Input | Same structure as `CPgI` |
| `DCut` | Take Cut | `mE` (byte 0) |
| `DAut` | Take Auto | `mE` (byte 0) |
| `CTPs` | Set Transition Position | `mE` (byte 0), `position` (bytes 2–3, 0–9999) |
| `CKOn` | Set Keyer On Air | `mE` (byte 0), `keyer` (byte 1), `enabled` (byte 2) |
| `CDsL` | Set Downstream Keyer | `keyer` (byte 0), `onAir` (byte 1) |
| `FtbA` | Perform Fade-To-Black | `mE` (byte 0), `0x02` (byte 1) |
| `CAuS` | Set Aux Source | mask (byte 0, set bit 0), `auxChannel` (byte 1), `input` (bytes 2–3) |

Set command packet structure:
```
Bytes 0–11:   ATEM header (AckRequest flag set, new local packet ID)
Bytes 12–13:  Segment length = 12 (8-byte cmd header + 4-byte payload)
Bytes 14–15:  0x00 0x00
Bytes 16–19:  Command name (e.g., "CPgI")
Bytes 20+:    Command-specific payload
```

---

## 10. Video Source IDs

Standard numeric IDs used in `PrgI`, `PrvI`, `CPgI`, `CPvI`, `AuxS`, `CAuS` commands:

| ID | Source |
|---|---|
| 0 | Black |
| 1–20 | Input 1–20 |
| 1000 | Color Bars |
| 2001–2002 | Color 1–2 |
| 3010, 3011 | Media Player 1, Media Player 1 Key |
| 3020, 3021 | Media Player 2, Media Player 2 Key |
| 4010–4040 | Key 1–4 Mask |
| 5010–5020 | DSK 1–2 Mask |
| 6000 | Super Source |
| 7001–7002 | Clean Feed 1–2 |
| 8001–8006 | Auxiliary 1–6 |
| 10010, 10011 | ME 1 Program, ME 1 Preview |
| 10020, 10021 | ME 2 Program, ME 2 Preview |

---

## 11. Connection Limits and Rejection

Blackmagic ATEM switchers support a limited number of simultaneous UDP clients (typically
5–8, depending on the model). When all slots are occupied, the switcher responds to a new
Hello packet with byte 12 = `3` (REJECTED).

AronHetLam's ATEMbase extension adds an `_isRejected` flag that is set when the server
returns connection status 3. The application can check `isRejected()` to differentiate
between an unreachable switcher (timeout) and a full-capacity rejection.

A rejected client should back off and retry; the recommended approach is to wait for an
existing client to disconnect before retrying the Hello handshake.

---

## 12. Protocol Constants Summary

```cpp
// Port
ATEM_UDP_PORT            = 9910

// ATEMbase header command values (pre-shift; applied with << 3 into byte 0)
ATEM_headerCmd_AckRequest       = 0x01   →  0x08 in byte 0
ATEM_headerCmd_HelloPacket      = 0x02   →  0x10 in byte 0
ATEM_headerCmd_Resend           = 0x04   →  0x20 in byte 0
ATEM_headerCmd_RequestNextAfter = 0x08   →  0x40 in byte 0
ATEM_headerCmd_Ack              = 0x10   →  0x80 in byte 0

// TallyServer flag values (post-shift; placed directly into byte 0)
TALLY_SERVER_FLAG_ACK_REQUEST   = 0x08
TALLY_SERVER_FLAG_HELLO         = 0x10
TALLY_SERVER_FLAG_RESENT_PACKAGE= 0x20
TALLY_SERVER_FLAG_RESEND_REQUEST= 0x40
TALLY_SERVER_FLAG_ACK           = 0x80

// Connection status bytes (byte 12 of Hello packet)
TALLY_SERVER_CONNECTION_REQUEST  = 1
TALLY_SERVER_CONNECTION_ACCEPTED = 2
TALLY_SERVER_CONNECTION_REJECTED = 3
TALLY_SERVER_CONNECTION_LOST     = 4

// Sizing limits
ATEM_maxInitPackageCount        = 40      // Bitmap size for init burst tracking
ATEM_packetBufferLength         = 96      // ATEMbase internal packet buffer (bytes)
ATEM_maxPacketId                = 32768   // (1 << 15) — packet ID wraps here
TALLY_SERVER_MAX_TALLY_FLAGS    = 41      // Max tally inputs TallyServer tracks
TALLY_SERVER_BUFFER_LENGTH      = 62      // TallyServer UDP buffer (bytes)
                                          //  = 12 (hdr) + 8 (cmd hdr) + 2 + 40

// Timing
TALLY_SERVER_KEEP_ALIVE_MSG_INTERVAL = 1500  // ms between keepalive pings
CONNECTION_TIMEOUT                   = 5000  // ms of silence before disconnect
CLIENT_LOCAL_PORT_RANGE              = 50100–65300

// Default client limit
TALLY_SERVER_DEFAULT_MAX_CLIENTS = 5
```

---

## 13. State Machine Summary

```
Client state         Trigger                      Next state
-------------------  ---------------------------  --------------------
DISCONNECTED         connect() called             → Hello sent; waiting

Hello sent           Server Hello received        → CONNECTED
Hello sent           5000 ms timeout              → DISCONNECTED (reconnect)

CONNECTED            Server Hello byte[12] == 2   → ACK sent; Init burst begins
CONNECTED            Server Hello byte[12] == 3   → REJECTED (_isRejected = true)

Init burst active    12-byte ping received from   → Requesting missed packets
                     switcher (rpID > 1)
Init burst active    All packet bits cleared      → INITIALIZED

INITIALIZED          AckRequest received          → Send ACK
INITIALIZED          State change push received   → Parse + update local state
INITIALIZED          Resend request received      → Respond with empty AckRequest
INITIALIZED          1500 ms idle                 → Send keepalive AckRequest
INITIALIZED          5000 ms no receive           → DISCONNECTED (reconnect)
```
