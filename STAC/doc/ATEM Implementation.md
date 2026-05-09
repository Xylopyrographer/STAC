# ATEM Switcher Support — Implementation Analysis

**Date:** April 12, 2026  
**Status:** Planning / Pre-implementation  
**Initial target platform:** M5Stack ATOM Matrix  
**Reference:** [AronHetLam/ATEM_tally_light_with_ESP8266](https://github.com/AronHetLam/ATEM_tally_light_with_ESP8266)

---

## Overview

This document analyses the feasibility and approach for adding support for Blackmagic Design ATEM video switchers to STAC v3, using the ATEMbase/ATEMmin Arduino libraries (originally by SKAARHOJ, extended by Aron N. Het Lam) as the reference implementation for understanding the ATEM protocol.

---

## The Protocol Difference: HTTP vs UDP

This is the central architectural challenge. Roland switchers use **HTTP polling** — stateless, request/response, on a configurable interval. ATEM uses a **custom UDP protocol** (port 9910) that is **stateful and push-based**:

| Aspect | Roland (current) | ATEM |
|--------|------------------|------|
| Transport | HTTP/TCP | Custom UDP |
| Pattern | Poll on interval | Continuous `runLoop()` + pushed updates |
| State | Stateless | Session ID, ACK tracking, packet sequencing |
| Authentication | Username/password | None required |
| Tally data | HTTP response body | `TlIn` command with flags per input index |
| Channel concept | 1–16 (HDMI/SDI banks) | Input index 1–40 |
| Connection limit | Unlimited | 5–8 simultaneous clients (model-dependent) |

---

## ATEM Protocol Details

### Connection Handshake

The ATEM protocol involves a multi-step handshake before tally data is available:

1. Send a "Hello" packet to the switcher at UDP port 9910
2. ATEM responds with a session ID and connection status
   - Response code `2` = accepted
   - Response code `3` = rejected (all client slots full)
   - Response code `4` = reconnect attempt
3. ATEM sends a burst of initial state packets (`_hasInitialized = false` during this phase)
4. Once all init packets are received and acknowledged, `hasInitialized()` becomes `true`
5. Normal operation: ATEM pushes state-change packets; client sends ACKs; keepalive pings exchanged

The init phase can take 1–3 seconds and must complete before tally flags are valid.

### Tally Data (from ATEMmin)

The `TlIn` command carries a tally flags array, one byte per input:

```cpp
// ATEMmin._parseGetCommands() — "TlIn" handler
atemTallyByIndexSources = word(_packetBuffer[0], _packetBuffer[1]);  // count
for (uint8_t a = 0; a < sources; a++) {
    atemTallyByIndexTallyFlags[a] = _packetBuffer[2 + a];  // bit 0 = PROGRAM, bit 1 = PREVIEW
}
```

Accessing tally state for a given input index:

```cpp
uint8_t flags = atemSwitcher.getTallyByIndexTallyFlags(inputIndex);  // 0-based
bool isProgram = (flags & TALLY_FLAG_PROGRAM);  // bit 0
bool isPreview = (flags & TALLY_FLAG_PREVIEW);  // bit 1
```

Mapping to STAC tally states is direct:

| ATEM flag | STAC `TallyStatus` | Display |
|-----------|--------------------|---------|
| `PROGRAM` bit set | `ONAIR` | Red |
| `PREVIEW` bit set | `SELECTED` | Green |
| Neither | `UNSELECTED` | No tally |

### Main Loop Requirement

ATEM requires `runLoop()` to be called continuously — it is non-blocking when passed `delayTime = 0`, draining the UDP receive buffer and sending ACKs. This must be called every main loop iteration, not on a polling timer interval.

---

## Fitting ATEM into STAC's Architecture

### The Interface Adapter

`IRolandClient::queryTallyStatus()` is poll-based. An `ATEMClient` adapter bridges this model cleanly by calling `runLoop(0)` (non-blocking) inside `queryTallyStatus()`:

```cpp
bool ATEMClient::queryTallyStatus(TallyQueryResult& result) {
    _atemSwitcher.runLoop(0);  // Non-blocking: drain UDP buffer, send ACKs

    if (!_atemSwitcher.isConnected()) {
        result.status = TallyStatus::NO_CONNECTION;
        result.connected = false;
        return true;
    }
    if (_atemSwitcher.isRejected()) {
        // Switcher client slots full — treat as NO_CONNECTION with log
        log_e("ATEM connection rejected — switcher client slots full");
        result.status = TallyStatus::NO_CONNECTION;
        result.connected = false;
        return true;
    }
    if (!_atemSwitcher.hasInitialized()) {
        // Connected but still receiving init packets — not ready yet
        result.status = TallyStatus::NOT_INITIALIZED;
        result.connected = true;
        return true;
    }

    uint8_t flags = _atemSwitcher.getTallyByIndexTallyFlags(_inputIndex);
    if (flags & TALLY_FLAG_PROGRAM)       result.status = TallyStatus::ONAIR;
    else if (flags & TALLY_FLAG_PREVIEW)  result.status = TallyStatus::SELECTED;
    else                                  result.status = TallyStatus::UNSELECTED;

    result.connected = true;
    result.gotReply = true;
    return true;
}
```

With `pollingInterval` set to ~0ms in the ATEM operating state, `queryTallyStatus()` is called every main loop iteration — which is exactly what `runLoop()` requires.

### Configuration Structure

ATEM config is simpler than Roland — no credentials, no port, no bank concept:

```cpp
struct AtemConfig {
    IPAddress switcherIP;   // ATEM switcher IP address
    uint8_t inputIndex;     // Tally input (0-based internally; displayed as 1-based)
};
```

This fits into a new `NS_ATEM` NVS namespace alongside existing Roland namespaces (`NS_V60HD`, `NS_V160HD`, `NS_V80HD`).

### Channel / Input Concepts

ATEM inputs are numbered 1–40 (no HDMI/SDI bank separation). The channel cycling logic in `StartupConfig.tpp` simplifies to a single 1–40 range, with no bank offset calculations. The web portal form is correspondingly simpler.

---

## Files to Create / Modify

| File | Change |
|------|--------|
| `include/Network/Protocol/ATEMClient.h` | New — wraps ATEMmin, implements `IRolandClient` |
| `src/Network/Protocol/ATEMClient.cpp` | New — ATEMClient implementation |
| `include/Config/Types.h` | Add `AtemConfig` struct, `isATEM()` helper to `StacOperations` |
| `include/Storage/ConfigManager.h` | Add `saveAtemConfig()` / `loadAtemConfig()` declarations |
| `src/Storage/ConfigManager.cpp` | Implement ATEM NVS save/load |
| `include/Network/WebConfigPages.h` | New `form-atem` HTML form (IP + input index 1–40) |
| `src/Network/WebConfigServer.cpp` | Handle ATEM form submission |
| `include/Application/StartupConfig.tpp` | ATEM channel cycling (1–40), display labels |
| `include/Utils/InfoPrinter.h` | ATEM serial output formatting |
| `src/Application/STACApp.cpp` | `isATEM()` branch in config load/save, client factory |
| `platformio.ini` | Add ATEMmin library dependency |

---

## ATOM Matrix Platform — No Issues

The ATEMmin library explicitly supports ESP32. The ATOM Matrix (ESP32-PICO-D4) runs the Arduino framework and has `WiFiUDP` available.

**Flash/RAM impact:** ATEMmin adds approximately 8–12KB of flash — well within the ATOM Matrix's ~32% free flash headroom. No RAM concerns; the library's packet buffer is approximately 60 bytes.

The 5×5 LED matrix display layer requires zero changes — tally state maps identically to the existing PROGRAM/PREVIEW/UNSELECTED rendering path.

---

## Key Concerns

### 1. GPL v3 License ⚠️

The ATEMbase/ATEMmin library is **GPL v3** (SKAARHOJ origin, modified by AronHetLam). STAC v3 is currently MIT licensed. Incorporating GPL code has implications:

**Options:**
- **Reimplement the ATEM UDP protocol directly.** The protocol is reverse-engineered and documented. The core connection + tally logic is approximately 300 lines of C++. This is roughly 2–3 days of work and eliminates the dependency entirely. Recommended if STAC is to remain MIT.
- **License STAC under GPL v3** if ATEM support is intended as a first-class feature.
- **Keep ATEM support as an optional GPL-licensed add-on module** delivered separately.

The ATEM protocol itself (as a communication standard) is not copyrightable — only the library implementation is GPL. A clean-room reimplementation has no license constraints.

### 2. ATEM Client Connection Limit

ATEM switchers allow only **5–8 simultaneous clients** (model-dependent). For a STAC deployment with one STAC per camera, this limits the system to 5–8 STAC units directly connected to the switcher.

This is a real-world constraint that differs from Roland (unlimited HTTP clients) and must be documented clearly in the User Guide. The AronHetLam project addresses this with a TallyServer relay (one STAC acts as a bridge for others), but that is out of scope for v3.

### 3. "Rejected" Connection State

If all ATEM client slots are occupied, `isRejected()` returns `true`. STAC should:
- Treat this as `NO_CONNECTION` for display purposes
- Log a specific error: `"ATEM connection rejected — switcher client slots full"`
- Continue retrying (the ATEM library handles reconnect attempts automatically)

### 4. Initialization Delay

After connecting, ATEM sends a burst of initial state packets before `hasInitialized()` becomes `true`. During this 1–3 second window, tally flags are not reliable. STAC should hold in a "connecting" visual state during this phase — the existing NO_CONNECTION/connecting display path already handles this naturally if `NOT_INITIALIZED` is treated as not-yet-ready.

### 5. No Polling Interval Needed

Unlike Roland (which has a configurable `pollingInterval` in NVS), ATEM must be driven continuously. The stored polling interval should be ignored (or forced to 0ms) for ATEM mode. Consider whether to expose a polling interval setting in the ATEM web form at all — it is not applicable and could confuse users.

---

## Architecture Notes

### Relationship to Existing String-Based Model Dispatch

The existing codebase already has the documented pattern of `if (isV60HD()) ... else if (isV160HD()) ... else if (isV80HD()) ...` in 7+ files. Adding ATEM extends this to a 4th branch everywhere. This strengthens the case for the `enum class SwitcherModel` refactor documented in the Architecture Observations section of `STAC_v3_PROJECT_CONTEXT.md`. However, per that document's guidance, the refactor should still be deferred unless adding multiple further models.

### Protocol Family Split

Roland and ATEM represent fundamentally different protocol families:
- **Roland:** HTTP request/response, interval-based polling
- **ATEM:** UDP, stateful, event/push-driven

If further non-Roland switchers are added in future (e.g., vMix, Ross Video, Tricaster), a cleaner architecture might split `IRolandClient` into a more generic `ISwitcherClient` interface, with Roland and ATEM as two separate implementation branches. For now, the adapter pattern described above fits ATEM into the existing `IRolandClient` interface without requiring that refactor.

---

## Summary Assessment

| Dimension | Assessment |
|-----------|------------|
| **Feasibility** | High — protocol well-understood, library runs on ESP32 |
| **Complexity** | Moderate — similar effort to V-80HD addition, plus new protocol type |
| **Display layer changes (ATOM Matrix)** | None required |
| **Primary decision point** | GPL license: reimplement or accept GPL dependency |
| **Connection limit caveat** | Max 5–8 STACs per ATEM — must be documented |
| **Estimated implementation effort** | 3–5 days (using ATEMmin); 5–8 days (clean-room reimplementation) |
