# STAC v3.0.0-RC.9 Project Context

**Date:** November 21, 2025  
**Branch:** `v3_RC`  
**Version:** v3.0.0-RC.9  
**Status:** Ready for testing - Protocol-specific namespace refactoring complete  
**Last Session:** NVS namespace architecture refactoring, factory reset fixes, documentation updates

---

## Quick Status

### ✅ Completed
- **Phase 9 (All Parts)**: Glyph Management, Roland Protocol, Web Config, OTA Updates
- **Boot Button Sequence**: Visual feedback with glyph displays and pulsing
- **Performance Optimizations**: Server startup timing improvements
- **Documentation**: DEVELOPER_GUIDE.md and HARDWARE_CONFIG.md updated for v3.0
- **Code Quality**: MAC address fix, STAC ID validation, visual parity with baseline
- **v3.0.0-RC.1**: Critical error recovery fix, STS Emulator, web UI enhancements
- **v3.0.0-RC.2 to RC.8**: V-160HD startup sequence bug fixes (channel display, colors, serial output)
- **v3.0.0-RC.9**: V-160HD error handling fixes, HTTP timeout optimization, polling improvements
- **Code Refactoring**: Display, Network, State management - ~330+ lines of duplication eliminated
- **API Updates**: NetworkClient.flush() → clear() for arduino-esp32 v3.3.2+ compatibility
- **Build System**: Release binary generation with standardized naming convention
- **Factory Reset**: Fixed to match baseline v2.x behavior (single flash, infinite park)
- **Button Sequence**: Skip factory reset when unconfigured (matches User's Guide)
- **NVS Architecture Refactoring**: Protocol-specific namespaces (v60hd, v160hd)
  - Migrated from unified 'operations' namespace to protocol-specific namespaces
  - Automatic migration logic preserves existing configurations
  - Improved extensibility for future protocols (ATEM, VMix, etc.)
  - Complete separation of protocol parameters (no "dead" keys)
- **Documentation**: NVS_Namespace_Structure.md v2.0 with complete architecture details

### 🎯 Current State
- All code compiles cleanly (one expected warning: RMT DMA on ESP32-PICO)
- **v3.0.0-RC.9** tested on all hardware:
  - ATOM Matrix #1 (94:b9:7e:a8:f8:00) - Normal mode
  - ATOM Matrix #2 (4c:75:25:c5:53:c4) - Peripheral mode
  - Waveshare ESP32-S3 (f0:f5:bd:6e:31:fc) - Normal mode with 8x8 display
- Major refactoring complete:
  - DisplayBase class eliminates ~180 lines per display type
  - RolandClientBase class eliminates ~100 lines between protocols
  - StateManagerBase<T> template eliminates ~140 lines of state management code
- API updates for future compatibility:
  - NetworkClient.flush() → clear() (arduino-esp32 v3.3.2+)
- Release binaries generated in `bin/` with naming convention
- Branch `v3_RC` ready for final testing and merge

### 📋 Next Steps
1. Final testing on real hardware (both ATOM Matrix and Waveshare if available)
2. Test with Roland V-60HD (already tested)
3. Optional: Test with Roland V-160HD on real hardware
4. When ready: Merge `v3_RC` → `main` and tag as `v3.0.0`

---

## Project Overview

**STAC** = **S**mart **T**ally **At**om **C**lient

A WiFi-enabled tally light system for Roland video switchers (V-60HD, V-160HD) using ESP32-based hardware with LED matrix displays.

**Supported Hardware:**
- M5Stack ATOM Matrix (5×5 LED, MPU6886 IMU, ESP32-PICO-D4)
- Waveshare ESP32-S3-Matrix (8×8 LED, QMI8658 IMU, ESP32-S3)

---

## Architecture Summary

### Layered Design

```
┌─────────────────────────────────────────┐
│        Application Layer                │
│  STACApp - Main application logic       │
├─────────────────────────────────────────┤
│        State Management                 │
│  SystemState, TallyState, OpMode        │
├─────────────────────────────────────────┤
│     Network & Storage Layer             │
│  WiFi, Roland Protocol, Web, OTA, NVS   │
├─────────────────────────────────────────┤
│   Hardware Abstraction Layer (HAL)      │
│  Display, IMU, Button, GROVE Port       │
├─────────────────────────────────────────┤
│        Configuration Layer              │
│  Device_Config.h, Board Configs         │
└─────────────────────────────────────────┘
```

### Key Components

**Hardware Layer:**
- `IDisplay` interface → `Display5x5`, `Display8x8` implementations
- `IIMU` interface → `MPU6886_IMU`, `QMI8658_IMU` implementations
- `IButton` interface → `ButtonHandler` implementation
- `GlyphManager` → Orientation-aware glyph rotation (5×5 and 8×8)
- Factory pattern for hardware creation

**Network Layer:**
- `WiFiManager` → Connection management with visual feedback
- `RolandClient` base class → `V60HDClient`, `V160HDClient` implementations
- `WebConfigServer` → Captive portal for initial setup
- `OTAUpdateServer` → Over-the-air firmware updates

**State Management:**
- `SystemState` → Central state coordinator
- `TallyStateManager` → Tally state (PROGRAM, PREVIEW, UNSELECTED, NO_TALLY)
- `OperatingModeManager` → Mode handling (NORMAL, PERIPHERAL, PROVISIONING)

**Storage:**
- `ConfigManager` → NVS-based configuration persistence
- Stores: WiFi credentials, switch config, operations, peripheral settings, STAC ID

**Application:**
- `STACApp` → Main application controller
- `StartupConfig` → Interactive startup configuration (channel, brightness, mode)
- Mode handlers: `handleNormalMode()`, `handlePeripheralMode()`, `handleProvisioningMode()`

**Utilities:**
- `InfoPrinter` → Consistent serial output formatting
- Test configurations for development

---

## Recent Changes (v3.0 Development)

### Code Refactoring and API Updates (November 21, 2025)

**Display Refactoring** (Commit: d05809b)
- **Problem**: Display5x5 and Display8x8 had ~180 lines of duplicate code each
- **Solution**: Created DisplayBase abstract base class with common functionality
- **Impact**: 
  - Display5x5.cpp: 179→41 lines (77% reduction)
  - Display8x8.cpp: ~170→41 lines (76% reduction)
  - Net reduction: ~93 lines of duplicate code
- **Files**:
  - Created: `include/Hardware/Display/DisplayBase.h`, `src/Hardware/Display/DisplayBase.cpp`
  - Modified: `src/Hardware/Display/Matrix5x5/Display5x5.cpp`, `src/Hardware/Display/Matrix8x8/Display8x8.cpp`

**Network Client Refactoring** (Commit: f9f431b)
- **Problem**: V60HDClient and V160HDClient had ~100 lines of duplicate code
- **Solution**: Created RolandClientBase abstract base class
- **Common Methods**: begin(), end(), isInitialized(), parseResponse(), handleSpecialCases()
- **Impact**:
  - V60HDClient: Removed ~40 lines of duplicate code
  - V160HDClient: Removed ~60 lines of duplicate code
  - Net reduction: ~100 lines of duplicate code
- **Files**:
  - Created: `include/Network/Protocol/RolandClientBase.h`, `src/Network/Protocol/RolandClientBase.cpp`
  - Modified: `include/Network/Protocol/V60HDClient.h`, `src/Network/Protocol/V60HDClient.cpp`
  - Modified: `include/Network/Protocol/V160HDClient.h`, `src/Network/Protocol/V160HDClient.cpp`

**State Management Refactoring** (Commit: b9265d2)
- **Problem**: TallyStateManager and OperatingModeManager had ~50 lines of duplicate state management logic
- **Solution**: Created StateManagerBase<T> template class (header-only, ~110 lines)
- **Template**: Generic state manager for any enum type (TallyState, OperatingMode)
- **Common Methods**: setState(), getTimeSinceChange(), setStateChangeCallback(), getCurrentState(), getPreviousState()
- **Impact**:
  - TallyStateManager: Removed duplicate constructor, setState logic, callbacks, accessors
  - OperatingModeManager: Removed duplicate logic, added getCurrentMode()/getPreviousMode() wrappers
  - Net reduction: ~50 lines core logic + ~90 lines boilerplate
- **Integration**: Updated SystemState.cpp to use setStateChangeCallback() instead of setModeChangeCallback()
- **Files**:
  - Created: `include/State/StateManagerBase.h` (template class)
  - Modified: `include/State/TallyStateManager.h`, `src/State/TallyStateManager.cpp`
  - Modified: `include/State/OperatingModeManager.h`, `src/State/OperatingModeManager.cpp`
  - Modified: `src/State/SystemState.cpp`

**NetworkClient API Update** (Commit: 4953018)
- **Problem**: arduino-esp32 v3.3.2+ deprecated NetworkClient.flush(), recommends clear() instead
- **Solution**: Changed client.flush() to client.clear() in V60HDClient
- **Impact**: Eliminates deprecation warning, future-proofs code for arduino-esp32 core updates
- **Files**: `src/Network/Protocol/V60HDClient.cpp` line 32

**Git Cleanup** (Commit: b0d29ad)
- **Problem**: Auto-generated build_info.h was tracked by git despite being in .gitignore
- **Solution**: Removed build_info.h from git tracking (file already in .gitignore)
- **Impact**: Clean git status, build_info.h regenerated automatically on each build
- **Files**: `include/build_info.h` (removed from tracking)

**Release Binary Generation** (November 21, 2025)
- **Naming Convention**: `STAC-{version}-{hardware}-{type}.bin`
  - version: From BUILD_VERSION (e.g., "3.0.0-RC.9")
  - hardware: `atom-matrix` or `waveshare-s3`
  - type: `firmware` (OTA updates) or `full` (complete flash image)
- **Binaries Created**:
  - `STAC-3.0.0-RC.9-atom-matrix-firmware.bin` (1.2M) - OTA app only
  - `STAC-3.0.0-RC.9-atom-matrix-full.bin` (1.3M) - Bootloader + partitions + app
  - `STAC-3.0.0-RC.9-waveshare-s3-firmware.bin` (1.2M) - OTA app only
  - `STAC-3.0.0-RC.9-waveshare-s3-full.bin` (1.3M) - Bootloader + partitions + app
- **Location**: `/Users/robl/Documents/PlatformIO/Projects/STAC3/STAC/bin/`
- **Full Image Layout**:
  - ATOM Matrix: 0x1000=bootloader, 0x8000=partitions, 0x10000=firmware
  - Waveshare S3: 0x0=bootloader, 0x8000=partitions, 0x10000=firmware

**Testing Results:**
- ✅ All refactoring validated on three hardware devices
- ✅ ATOM Matrix #1 (94:b9:7e:a8:f8:00): Normal mode, V-160HD client, DisplayBase, StateManagerBase
- ✅ ATOM Matrix #2 (4c:75:25:c5:53:c4): Peripheral mode, state transitions logged from StateManagerBase.h:69
- ✅ Waveshare ESP32-S3 (f0:f5:bd:6e:31:fc): Normal mode, V-60HD client, 8x8 display with DisplayBase
- ✅ No deprecation warnings in STAC code (only upstream WebServer library warning)
- ✅ Clean builds, all functionality working correctly

**Total Code Reduction:**
- Display refactoring: ~93 lines removed
- Network refactoring: ~100 lines removed  
- State refactoring: ~50 core + ~90 boilerplate = ~140 lines removed
- **Total: ~330+ lines of duplicate code eliminated**

**Maintainability Improvements:**
- Adding new displays: Only implement size-specific methods, inherit from DisplayBase
- Adding new Roland protocols: Only implement protocol-specific transport, inherit from RolandClientBase
- Adding new state types: Use StateManagerBase<T> template with any enum type
- Future API changes: Centralized in base classes, minimal changes to derived classes

### Automatic Build Versioning (November 21, 2025)

**Build Version Automation** (Commit: 9f10899)
- **Problem**: Baseline v2.x required 7 manual steps to generate build number using external hashdir tool
- **Solution**: Fully automated build versioning system using Python script
- **Features**:
  - Automatic version extraction from `Device_Config.h`
  - MD5 hash of all source files (src/ and include/) for build number
  - Git integration (commit hash, branch name, dirty status detection)
  - Timestamp generation
  - Auto-generated `build_info.h` header with macros
- **Integration**: Registered as PlatformIO pre-action, runs before every build
- **Macros**:
  - `BUILD_VERSION`: "3.0.0-RC.9" (from Device_Config.h)
  - `BUILD_NUMBER`: "a34bd6" (last 6 chars of source MD5)
  - `BUILD_GIT_COMMIT`: "9f10899+" (+ if uncommitted changes)
  - `BUILD_GIT_BRANCH`: "v3_RC"
  - `BUILD_TIMESTAMP`: "2025-11-21 01:00:25"
  - `BUILD_FULL_VERSION`: "3.0.0-RC.9 (a34bd6)"
  - `BUILD_INFO_STRING`: Complete build information
- **Documentation**: New file `Documentation/Developer/Automatic Build Versioning.md`
- **Old Method**: Renamed to `Creating the build number (v2-manual-obsolete).md`
- **Files**:
  - `scripts/build_version.py` (165 lines)
  - `platformio.ini` (registered pre-action)
  - `.gitignore` (excludes build_info.h)

**Build Number Display** (Commit: 0b7b71e)
- **Integration**: Updated `InfoPrinter.h` to display build information at startup
- **Serial Output**:
  ```
  Version: 3.0.0-RC.9 (a34bd6)
  Build: 0b7b71e+ @ 2025-11-21
  ```
- **Changes**:
  - Added `#include "../build_info.h"`
  - Version line uses `BUILD_FULL_VERSION` instead of `Config::Strings::SOFTWARE_VERSION`
  - New Build line shows `BUILD_GIT_COMMIT` and `BUILD_DATE`
- **Impact**: Users can easily identify which exact build is running on a device
- **Files**: `include/Utils/InfoPrinter.h`

### Code Cleanup (November 21, 2025)

**Refactoring Artifact Removal** (Commit: 286e690)
- **Removed**: Unused `#include "Utils/TestConfig.h"` from STACApp.cpp
  - Test mode (`ENABLE_TEST_CONFIG`) is disabled and not being used
  - Include was leftover from early refactoring development
- **Removed**: Commented-out `handleNormalMode()` stub (lines 404-420)
  - Old TODO placeholder from before Roland polling was implemented
  - Fully replaced by current working implementation
- **Impact**: Cleaner code, no functional changes
- **Files**: `src/Application/STACApp.cpp`

### v3.0.0-RC.9 Session (November 20-21, 2025)

**V-160HD Error Handling and Performance Optimizations:**

**HTTP Timeout Optimization**
- **Problem**: HTTPClient default 5-second timeout caused slow error recovery - polls 5+ seconds apart during errors
- **Solution**: Set `httpClient.setTimeout(1000)` for 1-second timeout (matches baseline V-60HD WiFiClient timeout)
- **Impact**: Fast error recovery - approximately 1-second retry rate during timeout conditions
- **Files**: `src/Network/Protocol/V160HDClient.cpp` lines 40-41

**Error Flag Logic Fix**
- **Problem**: V-160HD showed wrong error glyphs - orange X for junk/timeout instead of purple ?/X
- **Root Cause**: `gotReply` flag set incorrectly - was true for HTTP 401/4xx/5xx errors (not valid tally replies)
- **Solution**: Set `gotReply=true` only for HTTP 200 responses, `false` for all error codes
- **Baseline Match**: Now matches `STAC_220_rel/STAC/STACLib/STACSTS.h` getTallyState2() semantics
- **Files**: `src/Network/Protocol/V160HDClient.cpp` lines 61-90

**Connection Error Distinction**
- **Problem**: Couldn't distinguish between switch offline (connection refused) vs network congestion (timeout)
- **Solution**: Check `httpCode == HTTPC_ERROR_CONNECTION_REFUSED (-1)` separately from other errors
- **Behavior**:
  - Connection refused (errno 104): `connected=false`, orange X immediate display
  - Timeout/other errors: `connected=true`, purple X after 8-error threshold
- **Files**: `src/Network/Protocol/V160HDClient.cpp` lines 115-133

**Display Behavior Fix**
- **Problem**: Display showed purple fill on startup before any valid tally response
- **Root Cause**: `TallyState::NO_TALLY` mapped to purple color, `updateDisplay()` called prematurely after Roland client init
- **Solution**: 
  - Changed NO_TALLY color from PURPLE to BLACK
  - Removed `updateDisplay()` call after Roland client initialization
- **Result**: Display stays black (with power pixel) until first valid tally response or error threshold
- **Files**: 
  - `src/State/TallyStateManager.cpp` line 87
  - `src/Application/STACApp.cpp` lines 677-683

**Polling Timing Fix**
- **Problem**: Fast error polling (50ms) didn't work - still 5 seconds between polls during errors
- **Root Cause**: `lastRolandPoll = millis()` set BEFORE blocking HTTP call, counted request time as part of interval
- **Solution**: Moved `lastRolandPoll = millis()` to AFTER `queryTallyStatus()` completes
- **Result**: Proper 50ms fast polling during errors (within 1-second timeout blocks)
- **Files**: `src/Application/STACApp.cpp` lines 1150-1158

**WiFi Status Check**
- **Addition**: Added WiFi connection check before polling to prevent unnecessary switch queries when WiFi down
- **Files**: `src/Application/STACApp.cpp` lines 1143-1148

**Error Threshold Handling**
- **Fix**: Added `rolandPollInterval = ERROR_REPOLL_MS` to timeout/no-reply error path
- **Fix**: Don't update display or tally state until error threshold reached (keep showing last valid state)
- **Fix**: Set `currentTallyState` and `lastTallyState` when threshold hit
- **Files**: `src/Application/STACApp.cpp` lines 1272-1295

**STS Emulator Improvements**

**Default Credentials Update**
- **Change**: Default username from "admin" to "user", password from "admin" to "0000"
- **Reason**: Better matches typical Roland switch defaults for testing
- **Files**: `Documentation/Developer/Utility/SmartTally Server/sts_emulator.py` lines 53-54

**V-160HD HTTP Response Format**
- **Problem**: Emulator sent `HTTP/1.1` responses, real Roland V-160HD sends `HTTP/1.0` with lwIP server header
- **Fix**: Changed to `HTTP/1.0 200 OK\r\nServer: lwIP/1.3.1\r\nContent-type: text/plain\r\n\r\n{response}`
- **Impact**: Emulator now accurately simulates Roland switch HTTP responses
- **Files**: `Documentation/Developer/Utility/SmartTally Server/sts_emulator.py` lines 372-376

**Channel Format Auto-Detection**
- **Problem**: Emulator couldn't parse V-160HD channel format (hdmi_5) when not configured for V-160HD mode
- **Solution**: Auto-detect channel format by checking for underscore - handles both V-60HD (5) and V-160HD (hdmi_5)
- **Files**: `Documentation/Developer/Utility/SmartTally Server/sts_emulator.py` lines 351-359

**Junk Response HTTP Wrapping**
- **Problem**: Emulator sent raw junk data for V-160HD instead of HTTP-wrapped
- **Fix**: Wrap junk in HTTP 200 OK response for V-160HD mode, raw for V-60HD
- **Files**: `Documentation/Developer/Utility/SmartTally Server/sts_emulator.py` lines 305-310

**Immediate Shutdown**
- **Problem**: Delayed responses (5s sleep) continued sending after hitting return to stop emulator
- **Solution**: Check `self.running` flag after delay, drop response if server stopped
- **Result**: Hitting return immediately stops all output (no queued delayed responses)
- **Files**: `Documentation/Developer/Utility/SmartTally Server/sts_emulator.py` lines 330-337

**Error Semantics Summary:**
- Orange X: Connection refused (switch offline/unreachable), immediate display
- Purple X: Timeout or no-reply after 8 consecutive errors (~8-10 seconds)
- Purple ?: Junk responses after 8 consecutive junk replies
- Black display: On startup until first valid response or error threshold
- Fast recovery: 1-second timeout enables ~1-second retry rate during errors

**Testing Results:**
- ✅ Normal V-160HD polling works correctly
- ✅ Connection refused shows orange X immediately
- ✅ Display stays black on startup until valid tally response
- ✅ Fast error recovery verified (~1 second between timeout retries)
- ✅ Error glyphs correct (orange X for offline, purple X for timeout threshold)
- ✅ Emulator delay mode shutdown works (no queued responses after stop)

### v3.0.0-RC.2 to RC.8 Session (November 20, 2025)

**V-160HD Startup Sequence Bug Fixes:**

**Channel Display Bug** (RC.2)
- **Problem**: V-160HD SDI channels (9-20) showed wrong glyphs during startup config - displayed glyph indices 9-20 instead of digits 1-8
- **Root Cause**: `StartupConfig.tpp` used `getGlyph()` which expects index 0-9, but was passing channel numbers 9-20
- **Solution**: Calculate `displayChannel = tallyChannel - 8` for SDI channels, use `getDigitGlyph(displayChannel)` instead
- **Files**: `include/Application/StartupConfig.tpp` lines 132-280

**Channel Bank Initialization** (RC.3)
- **Problem**: When loading V-160HD config from NVS, channelBank wasn't being set correctly
- **Solution**: Added logic in `loadOperations()` to set `channelBank = "sdi_"` if `tallyChannel > 8`, else `"hdmi_"`
- **Files**: `src/Storage/ConfigManager.cpp` lines 136-170

**Serial Output Missing** (RC.4-RC.7)
- **Problem**: `printFooter()` wasn't being called consistently during startup
- **Root Cause**: Footer was only printing inside startup sequence block, which could be bypassed by autostart
- **Solution**: Moved `printFooter()` to execute BEFORE autostart logic, after brightness is applied (matches baseline STACProvision.h line 163)
- **Files**: `src/Application/STACApp.cpp` lines 537-547

**Channel Color Logic** (RC.5-RC.6)
- **Problem**: V-160HD SDI channels showed blue instead of light green during startup
- **Solution**: Added color logic - SDI channels (>8) use light green (0x1a800d), HDMI channels use blue
- **Files**: `src/Application/STACApp.cpp` lines 549-561

**Autostart Corner Color** (RC.7-RC.8)
- **Problem**: Autostart corner LEDs were always blue, but should vary by channel bank (baseline behavior)
- **Root Cause**: Baseline uses different colors: first bank (HDMI 1-8) = green corners (0x00ee00), second bank (SDI 9-20) = blue corners
- **Solution**: Set `autostartColor` based on channel bank - HDMI/V-60HD use 0x00ee00 (green), V-160HD SDI uses blue
- **Baseline Reference**: `STAC_220_rel/STAC/STACLib/STACGlyph5.h` line 106 (`RGB_AS_PULSE_COLOR 0x00ee00`)
- **Files**: `src/Application/STACApp.cpp` lines 548-566, 571-597

**Summary of Fixes:**
- ✅ SDI channels display as digits 1-8 (not indices 9-20)
- ✅ Channel bank correctly initialized from NVS
- ✅ Serial output prints before autostart (always visible)
- ✅ Channel colors: blue for HDMI/V-60HD, light green (0x1a800d) for V-160HD SDI
- ✅ Autostart colors: green (0x00ee00) for first bank, blue for second bank
- ✅ Startup workflow matches baseline for both V-60HD and V-160HD

### v3.0.0-RC.1 Session (November 20, 2025)

**Critical Bug Fixes:**

**Error Recovery Fix** (Commit: 0878c0a)
- **Problem**: Orange X error glyph persisted for ~15 poll cycles (~4.5 seconds) when recovering from connection errors
- **Root Cause**: When tally state was unchanged during error recovery (e.g., UNSELECTED→UNSELECTED), `setState()` returned false without calling callbacks, so display and GROVE port never updated
- **Solution**: Added `else` branch in `pollRolandSwitch()` to force `updateDisplay()` and `grovePort->setTallyState()` even when state unchanged
- **Result**: Display and GROVE now update **immediately** on first valid response after error
- **Files**: `src/Application/STACApp.cpp` lines 1270-1289

**Provisioning Display Fix**
- **Problem**: After provisioning submit, STAC display showed green fill instead of green checkmark
- **Solution**: Changed line 1023 to `drawGlyph(GLF_CK, GREEN, BLACK)` instead of `fill(GREEN)`
- **Additional**: Removed duplicate green fill at line 1098
- **Files**: `src/Application/STACApp.cpp`

**New Tools:**

**STS Emulator** (754 lines)
- Unified Roland V-60HD and V-160HD protocol emulator
- Replaced separate test scripts with comprehensive testing tool
- Features:
  - Live keyboard error injection (spacebar trigger for ignore mode)
  - Auto-cycle tally states with configurable intervals
  - Multi-STAC support with per-device statistics
  - Response delay, junk data, and connection loss simulation
  - Menu navigation uses '0' for Exit/Back consistently
- Shell alias: `sts-emulator` (bypasses subprocess buffering issues)
- PlatformIO target removed (buffering problems with subprocess)
- Files: `Documentation/Developer/Utility/SmartTally Server/sts_emulator.py`
- Documentation: `Documentation/Developer/Utility/SmartTally Server/STS_EMULATOR_GUIDE.md`

**Web UI Enhancements:**
- Larger touch targets: Buttons 18px font, 44px min-height
- Larger dropdowns: 18px font, 44px height
- Centered page layout with max-width container
- Applied to both provisioning and OTA pages
- Files: `include/Network/WebConfigPages.h`, `include/Network/OTAUpdatePages.h`

**Other Changes:**
- Version updated to "3.0.0-RC.1" in `include/Device_Config.h`
- Removed emulator target from `scripts/custom_targets.py` (shell alias preferred)

### Performance Optimizations (November 19, 2025)

**Server Startup Timing** (Committed: 576c56e)
- **OTA Mode**: Server starts immediately after 6-second flash sequence (don't wait for button release)
- **Provisioning Mode**: Server starts before 4-second flash sequence (saves 4 seconds)
- **Impact**: Network services (WiFi AP, mDNS, web server) available immediately
- **User Experience**: Can release button anytime, faster access to services

**Files Modified:**
- `src/Application/STACApp.cpp`:
  - Line 975-1025: `handleProvisioningMode()` - Server start moved before flash
  - Line 1568-1590: Boot button sequence - OTA server start moved to immediate execution

### Boot Button Sequence

**Hold Times:**
- 0-2 sec: Provisioning mode (orange config glyph)
- 2-4 sec: Factory reset (red frame + green check glyph)
- 4-6 sec: OTA update mode (red update glyph)

**Visual Feedback:**
- Glyph displays for each state
- Flash sequences on state transitions (4×125ms)
- Pulsing displays during provisioning/OTA (brightness modulation)

**Implementation:**
- State machine in `checkBootButtonSequence()`
- Handlers: `handleProvisioningMode()`, `handleFactoryReset()`, `handleOTAUpdateMode()`
- Visual parity with v2.x baseline maintained

### Roland Protocol Support

**V-60HD Client** (`V60HDClient`)
- 8 channels (numbered 1-8)
- Query format: `QST` command
- Response parsing for ONAIR/SELECTED/UNSELECTED states
- Error handling: junk replies, timeouts, no connection

**V-160HD Client** (`V160HDClient`)
- HDMI channels (1-16) and SDI channels (1-12)
- Channel bank selection (hdmi_, sdi_, hdmi, sdi)
- Query format: `QTL` command with bank prefix
- Response parsing for bank-specific queries

**Factory Pattern:**
- `RolandClientFactory::createFromString(model)` selects correct implementation
- Easy to extend for new switcher models

**Error Management:**
- Normal polling: User-configured interval (default 300ms)
- Error conditions: Fast re-poll at 50ms (configurable)
- Error counter threshold: 8 consecutive errors before display update
- Visual feedback: Purple/orange X glyphs in camera mode, preview in talent mode

### Web Configuration

**WebConfigServer Features:**
- Captive portal (auto-redirect to config page)
- Mobile-friendly responsive interface
- Configures: WiFi SSID/password, switch model/IP/port, tally channel, poll interval
- Model-specific fields (V-60HD max channel, V-160HD HDMI/SDI max channels)
- Automatic restart after configuration save

**Implementation:**
- DNS server for captive portal
- mDNS advertisement (`stac-xxxxx.local`)
- HTML/CSS embedded in PROGMEM
- Callback for display updates (pulsing glyph)

### OTA Updates

**OTAUpdateServer Features:**
- Web-based firmware upload
- Upload `.bin` files via browser
- Progress indication on display (pulsing red update glyph)
- Automatic verification and restart
- Rollback on failure

**Access:**
- Boot button hold 4-6 seconds
- mDNS: `http://stac-xxxxx.local`
- Direct IP: `http://192.168.4.1`

### GlyphManager

**Features:**
- Automatic glyph rotation based on IMU orientation
- Separate implementations for 5×5 and 8×8 displays
- Thread-safe glyph access
- Power-on indicator glyph management

**Glyph Types:**
- Numbers 0-9 (for channel display)
- Icons: WiFi, config, update, checkmark, frame, power-on, etc.
- Orientation-aware (rotates with device)

### StartupConfig

**Interactive Configuration:**
- Channel selection (visual number display)
- Brightness adjustment (checkerboard + level number)
- Camera/Talent mode selection (C/T glyphs)
- Autostart bypass (blinking corners + timeout)

**Shared Implementation:**
- Used in both normal mode and peripheral mode
- Callbacks for saving settings (normal vs peripheral NVS namespaces)
- Consistent UX across modes

---

## File Structure

```
STAC3/
├── STAC/                           # PlatformIO project
│   ├── platformio.ini              # Build configuration
│   ├── src/
│   │   ├── main.cpp                # Entry point
│   │   ├── Application/
│   │   │   └── STACApp.cpp         # Main application (1600+ lines)
│   │   ├── Hardware/
│   │   │   ├── Display/
│   │   │   │   ├── Matrix5x5/Display5x5.cpp
│   │   │   │   ├── Matrix8x8/Display8x8.cpp
│   │   │   │   ├── GlyphManager5x5.cpp
│   │   │   │   └── GlyphManager8x8.cpp
│   │   │   ├── Sensors/
│   │   │   │   ├── MPU6886_IMU.cpp
│   │   │   │   └── QMI8658_IMU.cpp
│   │   │   ├── Input/
│   │   │   │   └── ButtonHandler.cpp
│   │   │   └── Interface/
│   │   │       ├── GrovePort.cpp
│   │   │       └── PeripheralDetector.cpp
│   │   ├── Network/
│   │   │   ├── WiFiManager.cpp
│   │   │   ├── Protocol/
│   │   │   │   ├── RolandClient.cpp
│   │   │   │   ├── V60HDClient.cpp
│   │   │   │   └── V160HDClient.cpp
│   │   │   └── Web/
│   │   │       ├── WebConfigServer.cpp
│   │   │       └── OTAUpdateServer.cpp
│   │   ├── State/
│   │   │   ├── SystemState.cpp
│   │   │   ├── TallyStateManager.cpp
│   │   │   └── OperatingModeManager.cpp
│   │   ├── Storage/
│   │   │   └── ConfigManager.cpp
│   │   └── Utils/
│   │       ├── InfoPrinter.cpp
│   │       └── StartupConfig*.cpp (5x5 and 8x8)
│   └── include/
│       ├── Device_Config.h         # USER EDITS THIS - Board selection
│       ├── BoardConfigs/           # Board-specific configs
│       │   ├── AtomMatrix_Config.h
│       │   └── WaveshareS3_Config.h
│       ├── Config/
│       │   ├── Constants.h         # System constants
│       │   └── Types.h             # Common types/enums
│       ├── Hardware/               # HAL interfaces
│       ├── Network/                # Network headers
│       ├── State/                  # State management headers
│       ├── Storage/                # Storage headers
│       └── Application/            # Application headers
└── Documentation/
    ├── Developer/
    │   ├── DEVELOPER_GUIDE.md      # ✅ Updated for v3.0.0-RC
    │   └── HARDWARE_CONFIG.md      # ✅ Updated for v3.0.0-RC
    └── User/
        ├── Release Notes.md
        └── STAC Users Guide.md
```

---

## Key Code Locations

### Application Logic
**File:** `src/Application/STACApp.cpp`

```cpp
// Initialization
setup()                          // Lines 37-117
initializeHardware()             // Lines 119-227
initializeNetworkAndStorage()    // Lines 229-318
determineOperatingMode()         // Lines 320-358

// Main loop
loop()                           // Lines 360-389
handleButton()                   // Lines 391-438
handleOrientation()              // Lines 440-454
updateDisplay()                  // Lines 456-486

// Mode handlers
handleNormalMode()               // Lines 488-663 (WiFi, Roland polling)
handlePeripheralMode()           // Lines 665-873 (Read GROVE, display tally)
handleProvisioningMode()         // Lines 975-1089 (Web config server)
handleOTAUpdateMode()            // Lines 1334-1378 (OTA server)
handleFactoryReset()             // Lines 1380-1414 (Clear NVS, restart)

// Boot button sequence
checkBootButtonSequence()        // Lines 1416-1609 (State machine)

// Roland polling
initializeRolandClient()         // Lines 1091-1143
pollRolandSwitch()               // Lines 1145-1332 (Query, parse, display)
```

### Configuration Management
**File:** `src/Storage/ConfigManager.cpp`

```cpp
// WiFi
saveWiFiCredentials()
loadWiFiCredentials()
hasWiFiCredentials()

// Switch
saveSwitchConfig()
loadSwitchConfig()

// Operations (channel, poll rate, brightness, etc.)
saveOperations()
loadOperations()

// Peripheral mode settings
savePeripheralSettings()
loadPeripheralSettings()

// STAC ID
generateAndSaveStacID()
loadStacID()
```

### Roland Protocol
**Files:** `src/Network/Protocol/V60HDClient.cpp`, `V160HDClient.cpp`

```cpp
// V-60HD
queryTallyStatus()              // Query "QST", parse response
parseTallyResponse()            // Parse "ONAIR"/"SELECTED"/etc.

// V-160HD
queryTallyStatus()              // Query "QTL:hdmi_1" etc.
parseTallyResponse()            // Parse bank-specific responses
```

### Display Management
**Files:** `src/Hardware/Display/GlyphManager*.cpp`

```cpp
updateOrientation()             // Set rotation based on IMU
getGlyph(index)                 // Get rotated glyph data
getDigitGlyph(number)           // Get number glyph (0-9)
```

**Files:** `src/Hardware/Display/Matrix*/Display*.cpp`

```cpp
begin()                         // Initialize LED strip
fill()                          // Fill with color
drawGlyph()                     // Draw glyph at rotation
drawGlyphOverlay()              // Overlay glyph (non-black pixels)
pulseDisplay()                  // Brightness modulation pulse
flash()                         // Flash display N times
```

---

## Configuration System

### User Configuration
**File:** `include/Device_Config.h`

```cpp
// Uncomment ONE board
#define BOARD_M5STACK_ATOM_MATRIX
// #define BOARD_WAVESHARE_ESP32_S3_MATRIX
```

This automatically includes the correct board config from `include/BoardConfigs/`.

### Board Configurations
**Files:** `include/BoardConfigs/AtomMatrix_Config.h`, `WaveshareS3_Config.h`

Define:
- Display: Width, height, LED type, color order, wiring pattern, data pin, brightness
- IMU: Type, I2C pins, clock speed, orientation offset
- Button: GPIO pin, debounce, active low/high, pullup needs
- Pins: Peripheral mode detection, GROVE/tally pins
- Timing: Button hold times, WiFi timeout, poll intervals
- Glyphs: Size (5×5 or 8×8), format (packed/unpacked)

### Runtime Configuration (NVS)
**Namespaces:**
- `stac` - STAC ID
- `wifi` - SSID, password
- `switch` - Model, IP, port, username, password
- `operations` - Channel, poll rate, brightness, camera mode, autostart, max channels
- `peripheral` - Camera mode, brightness (for peripheral mode)

---

## Build & Upload

### PlatformIO

```bash
# Navigate to project
cd /Users/robl/Documents/PlatformIO/Projects/STAC3/STAC

# Build
pio run

# Upload
pio run -t upload

# Monitor serial
pio device monitor -b 115200

# Combined (upload and monitor)
pio run -t upload && pio device monitor -b 115200
```

### Expected Output

```
Compile: ✅ Success (1 warning: RMT DMA on ESP32-PICO - EXPECTED, ignore)
Flash Size: ~1.29 MB (66% of 1.9 MB available)
RAM Usage: 15.6% (51220 bytes)
```

---

## Testing Checklist

### Hardware Tests
- [ ] Display: All colors correct (RED, GREEN, ORANGE, PURPLE)
- [ ] Display: Glyph rotation follows IMU orientation
- [ ] Button: Short press, long press (1.5s), boot hold sequences
- [ ] IMU: Orientation detection (UP, DOWN, LEFT, RIGHT)
- [ ] WiFi: Connection, reconnection, timeout handling
- [ ] GROVE port: Output (normal mode), Input (peripheral mode)

### Operating Mode Tests
**Normal Mode:**
- [ ] Startup config sequence (channel, brightness, camera/talent)
- [ ] Autostart mode (blinking corners, timeout/button cancel)
- [ ] WiFi connection with visual feedback (orange→green→power pixel)
- [ ] Roland V-60HD polling (tested ✅)
- [ ] Roland V-160HD polling (tested with emulator ✅)
- [ ] Tally states: PROGRAM (red), PREVIEW (green), UNSELECTED (green or purple frame)
- [ ] Error displays: Orange X (timeout), Purple X (no reply), Purple ? (junk)
- [ ] GROVE output: Correct 2-bit tally encoding

**Peripheral Mode:**
- [ ] Jumper detection at boot
- [ ] Startup animation (green P glyph, flash 4×, checkmark)
- [ ] GROVE input: Read tally from master device
- [ ] Display mirrors master (PROGRAM=red, PREVIEW=green, UNSELECTED=green/frame)
- [ ] Settings: Long-press button for brightness/mode change
- [ ] Settings persistence after restart

**Provisioning Mode:**
- [ ] Entry: No config at boot OR boot button 0-2sec hold
- [ ] Captive portal redirect works
- [ ] Web interface mobile-friendly
- [ ] Configuration save and restart
- [ ] Switch model dropdown (V-60HD, V-160HD)
- [ ] Model-specific fields (V-60HD max channel, V-160HD HDMI/SDI)

**OTA Mode:**
- [ ] Entry: Boot button 4-6sec hold
- [ ] mDNS: `http://stac-xxxxx.local` accessible
- [ ] Direct IP: `http://192.168.4.1` accessible
- [ ] Upload .bin file via browser
- [ ] Progress indication (pulsing red glyph)
- [ ] Successful update and restart
- [ ] Failure handling (rollback)

**Factory Reset:**
- [ ] Entry: Boot button 2-4sec hold
- [ ] Visual confirmation (red flashing)
- [ ] All NVS cleared
- [ ] Device restarts in provisioning mode

### Roland Protocol Tests
**V-60HD:**
- [ ] Channel 1-8 query and response
- [ ] ONAIR → PROGRAM (red)
- [ ] SELECTED → PREVIEW (green)
- [ ] UNSELECTED → Green or purple frame (camera/talent)
- [ ] Connection errors handled
- [ ] Junk replies handled
- [ ] Fast re-poll on errors

**V-160HD:**
- [ ] HDMI bank channels 1-16
- [ ] SDI bank channels 1-12
- [ ] Bank switching (hdmi_, sdi_, hdmi, sdi)
- [ ] All above tally states
- [ ] Error handling

### Performance Tests
- [ ] Provisioning server starts before flash (4sec improvement)
- [ ] OTA server starts at 6sec threshold (no button wait)
- [ ] Normal poll rate: 300ms (or user-configured)
- [ ] Error poll rate: 50ms
- [ ] Peripheral mode poll: 2ms (< 2ms latency)

---

## Git Workflow

### Current Branches

```
main (or master)                # Production/release branch
  │
  └─ refactor/phase1-foundation # Development branch (8 commits ahead)
       │
       └─ v3_RC                 # Release candidate (current) ✅
```

### Recent Commits (v3_RC)

```
0b7b71e (HEAD -> v3_RC) feat: Display build number in serial startup output
9f10899 feat: Add automatic build versioning system
8a8f1e9 docs: Update context for code cleanup commit
286e690 refactor: Remove refactoring artifacts from STACApp
0cb6394 v3.0.0-RC.9: Fix V-160HD error handling and improve polling performance
xxxxxxx v3.0.0-RC.8: Fix V-160HD startup color logic (channel and autostart colors)
xxxxxxx v3.0.0-RC.2-RC.7: Fix V-160HD channel display, serial output, and bank initialization
0878c0a v3.0.0-RC.1: Critical error recovery fix + STS Emulator + Web UI enhancements
ad1b956 docs: update developer documentation for v3.0.0-RC
576c56e (refactor/phase1-foundation) perf: optimize OTA and provisioning server startup timing
f3ba8ac feat: Implement boot button sequence with glyph-based displays and pulsing
```

### Release Workflow (When Ready)

```bash
# 1. Final testing on v3_RC branch (you are here)
git checkout v3_RC
# ... test everything ...

# 2. Merge to main when ready
git checkout main
git merge v3_RC

# 3. Tag the release
git tag -a v3.0.0 -m "Release v3.0.0 - Major refactoring with Roland protocol support"

# 4. Push everything
git push origin main --tags

# 5. Optional: Keep branches for history
# refactor/phase1-foundation: Development history
# v3_RC: Release candidate testing
```

---

## Recent Changes (November 21, 2025)

### Factory Reset and Button Sequence Fixes
- **Factory Reset Behavior**: Fixed to match baseline v2.x
  - Single flash confirmation (was 5x flash + green checkmark)
  - Infinite park with factory reset glyph displayed (was auto-restart)
  - Clears all NVS namespaces: stac, wifi, switch, v60hd, v160hd, peripheral
- **Button Sequence Logic**: Updated to skip factory reset when unconfigured
  - Unconfigured state starts at OTA_UPDATE_PENDING
  - Configured state starts at PROVISIONING_PENDING → FACTORY_RESET_PENDING → OTA_UPDATE_PENDING
  - Added serial output "***** STAC not configured *****"
  - Matches User's Guide: "You cannot do a Factory Reset if the red Configuration Required icon is displayed"

### NVS Architecture Refactoring
**Major architectural improvement for better extensibility and maintainability.**

**Changes:**
- Replaced unified `operations` namespace with protocol-specific namespaces:
  - `v60hd`: Roland V-60HD operational parameters
  - `v160hd`: Roland V-160HD operational parameters
- Updated ConfigManager API with protocol-specific methods:
  - `saveV60HDConfig()`, `loadV60HDConfig()` for V-60HD
  - `saveV160HDConfig()`, `loadV160HDConfig()` for V-160HD
  - `getActiveProtocol()` for protocol detection
  - `hasProtocolConfig()` for namespace validation
- Added automatic migration from legacy `operations` namespace:
  - Detects old namespace on first boot after update
  - Migrates all parameters to appropriate protocol namespace
  - Clears old namespace after successful migration
- Updated all save/load call sites (8 locations):
  - STACApp.cpp: startup sequence, provisioning, info printer, Roland client init
  - StartupConfig.tpp: tally channel, camera/talent mode, startup mode, brightness
- Updated factory reset to clear new namespaces (v60hd, v160hd)
- Updated NVS documentation (v2.0) with new architecture

**Benefits:**
- Clean separation: each protocol fully self-contained
- No dead keys: no unused parameters for inactive protocols
- Extensibility: easy to add new protocols (ATEM, VMix, etc.)
- Protocol independence: no parameter conflicts when switching models
- Better maintainability: clearer code organization

**Migration:**
- Transparent on first boot after update
- Preserves all existing configuration data
- Serial output: "Found old operations namespace, migrating to V-60HD protocol namespace"

**Testing:**
- Build: Successful compilation, all warnings resolved
- Hardware: Tested on ATOM Matrix, migration successful from RC.8 configuration
- Operational: V-60HD client functioning correctly with migrated settings

**Commits:**
- `4890891`: Fix factory reset behavior to match baseline v2.x
- `60e61b0`: Add NVS namespace structure documentation
- `b085803`: Refactor to protocol-specific NVS namespaces
- `a48d83b`: Update NVS documentation: clarify WiFi namespace usage

---

## Known Issues & Notes

### Expected Warnings
```
LiteLED: Selected ESP32 model does not support RMT DMA access.
```
**Status:** EXPECTED - ATOM Matrix uses ESP32-PICO-D4, ignore this warning

### GPIO Considerations
- **ATOM Matrix**: GPIO 39 is input-only, needs `BUTTON_NEEDS_EXTERNAL_PULLUP true`
- **Peripheral Mode**: Jumper must connect `PM_CHECK_OUT` to `PM_CHECK_IN` before boot

### V-160HD Notes
- Tested with emulator only (STS junk delay emulator)
- Real hardware testing recommended but not critical (protocol identical to V-60HD)
- Channel banks: `hdmi_`, `sdi_`, `hdmi`, `sdi` (configurable in web UI)

### Power Considerations
- 5×5 display at max brightness: ~1.5W (300mA @ 5V)
- 8×8 display at max brightness: ~3.8W (760mA @ 5V)
- USB may not provide enough current at full white/full brightness
- Default brightness limited to safe levels (BRIGHTNESS_MAX = 60)

---

## Development Notes

### Code Style
- **Classes:** PascalCase (`DisplayFactory`, `TallyStateManager`)
- **Functions:** camelCase (`updateDisplay`, `isConnected`)
- **Constants:** UPPER_SNAKE_CASE or constexpr (`MAX_RETRIES`, `BUFFER_SIZE`)
- **Namespaces:** PascalCase (`Hardware`, `Net`, `Display`)
- **Indentation:** 4 spaces
- **Braces:** Opening on same line

### Debugging Tips

**Serial Logging:**
```cpp
log_e("Error: %s", msg);        // ERROR
log_w("Warning: %d", val);      // WARNING
log_i("Info: connected");       // INFO
log_d("Debug: x=%d", x);        // DEBUG
log_v("Verbose: entering");     // VERBOSE
```

**Set log level in platformio.ini:**
```ini
-DCORE_DEBUG_LEVEL=3  ; 3=INFO, 4=DEBUG, 5=VERBOSE
```

**Exception Decoder:**
- Enabled automatically in platformio.ini
- Decodes crash backtraces to function names and line numbers

### Common Tasks

**Change default brightness:**
```cpp
// BoardConfigs/YourBoard_Config.h
#define DISPLAY_BRIGHTNESS_DEFAULT 30  // Was 20
```

**Add new configuration parameter:**
1. Add to `StacOperations` struct in `include/Config/Types.h`
2. Add save/load in `src/Storage/ConfigManager.cpp`
3. Use in application via `systemState->getOperations().yourParam`

**Add support for new Roland switcher:**
1. Create `VxxxClient.h` and `.cpp` in `src/Network/Protocol/`
2. Inherit from `RolandClient`
3. Implement `queryTallyStatus()` and `parseTallyResponse()`
4. Add to `RolandClientFactory::createFromString()`
5. Add to web config dropdown options

---

## Resources

### Documentation
- **Developer Guide:** `/Documentation/Developer/DEVELOPER_GUIDE.md` ✅ Updated
- **Hardware Config:** `/Documentation/Developer/HARDWARE_CONFIG.md` ✅ Updated
- **Detailed Changelog:** `/Documentation/Developer/Detailed Change Log.md`
- **User Guide:** `/Documentation/User/STAC Users Guide.md`

### External Links
- **ESP32 Arduino Core:** https://docs.espressif.com/projects/arduino-esp32/
- **PlatformIO Docs:** https://docs.platformio.org/
- **LiteLED Library:** https://github.com/Xylopyrographer/LiteLED
- **XP_Button Library:** https://github.com/Xylopyrographer/XP_Button
- **M5Stack ATOM:** https://docs.m5stack.com/en/core/atom_matrix
- **Waveshare ESP32-S3:** https://www.waveshare.com/wiki/ESP32-S3-Matrix

### Repository
- **GitHub:** https://github.com/Xylopyrographer/STAC
- **Issues:** https://github.com/Xylopyrographer/STAC/issues
- **Discussions:** https://github.com/Xylopyrographer/STAC/discussions

---

## Quick Reference Commands

```bash
# Build and upload
cd /Users/robl/Documents/PlatformIO/Projects/STAC3/STAC
pio run -t upload && pio device monitor -b 115200

# Git status
git status
git log --oneline -5
git branch -v

# Check for errors (after edit)
pio run

# Clean build
pio run -t clean
pio run
```

---

## Contact Points for Questions

### Architecture Questions
- Why factory pattern? → See DEVELOPER_GUIDE.md "Design Patterns"
- How to add hardware? → See HARDWARE_CONFIG.md "Adding Custom Hardware"
- State management? → See `include/State/` headers and implementations

### Protocol Questions
- Roland protocol details → See `/Documentation/Developer/STAC Communications.md`
- V-60HD specifics → See `src/Network/Protocol/V60HDClient.cpp`
- V-160HD specifics → See `src/Network/Protocol/V160HDClient.cpp`

### Configuration Questions
- Board selection → `include/Device_Config.h`
- Pin assignments → `include/BoardConfigs/*.h`
- Runtime config → `src/Storage/ConfigManager.cpp`

---

## Session Continuation Checklist

When picking up this project again:

1. **Read this document** to refresh context
2. **Check current branch:** `git branch` (should be `v3_RC`)
3. **Review recent commits:** `git log --oneline -5`
4. **Check build status:** `pio run` (should compile cleanly)
5. **Review any new issues/PRs** on GitHub
6. **Test key functionality** if making changes
7. **Update this document** if project direction changes

---

**Project Status:** ✅ **READY FOR RELEASE CANDIDATE TESTING**

All major features complete. Code refactored and optimized. Tested on all hardware platforms.
Next step: Final testing, then merge to main and tag v3.0.0.

---

*Last Updated: November 21, 2025*  
*Context Document Version: 2.4*  
*Project Version: v3.0.0-RC.9*

<!-- End of Context Document -->
