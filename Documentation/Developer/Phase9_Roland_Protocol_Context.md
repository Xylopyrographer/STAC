# Phase 9 Part 2: Roland Protocol Implementation - Context Document

**Date:** November 15, 2025  
**Session:** Roland V-60HD/V-160HD Protocol Implementation and Testing  
**AI Assistant:** Claude Sonnet 4.5  
**Developer:** Rob Lake (@Xylopyrographer)  
**Branch:** `refactor/phase1-foundation`

---

## Session Summary

Successfully implemented and tested the Roland Smart Tally protocol for both V-60HD and V-160HD video switchers. The STAC device now polls Roland switches over WiFi and automatically updates the tally display based on the camera's status (PROGRAM/PREVIEW/UNSELECTED).

---

## What Was Accomplished

### 1. Roland Protocol Implementation ✅

**Created Files:**
- `include/Network/Protocol/IRolandClient.h` - Interface for Roland clients
- `include/Network/Protocol/V60HDClient.h` - V-60HD header
- `src/Network/Protocol/V60HDClient.cpp` - V-60HD implementation (WiFiClient, no auth)
- `include/Network/Protocol/V160HDClient.h` - V-160HD header  
- `src/Network/Protocol/V160HDClient.cpp` - V-160HD implementation (HTTPClient with Basic Auth)
- `include/Network/Protocol/RolandClientFactory.h` - Runtime switch model selection

**Key Features:**
- **Runtime selection** between V-60HD and V-160HD based on stored configuration
- **V-60HD Protocol:**
  - Simple WiFiClient with short-form GET requests
  - No authentication required
  - GET /tally/{channel}/status
  - Returns: "onair", "selected", or "unselected"
  
- **V-160HD Protocol:**
  - HTTPClient with Basic Authentication
  - Keep-alive connections
  - Bank-based channels (bankA/bankB)
  - GET /tally/{bank}{channel}/status
  - Username/password authentication

### 2. Configuration Management Updates ✅

**Updated Files:**
- `include/Storage/ConfigManager.h` - Added username/password parameters
- `src/Storage/ConfigManager.cpp` - Updated save/load methods for V-160HD auth

**New Storage:**
- Switch username (for V-160HD)
- Switch password (for V-160HD)
- Both optional, stored in NVS "switch" namespace

### 3. Application Integration ✅

**Updated Files:**
- `include/Application/STACApp.h` - Added Roland client and polling state
- `src/Application/STACApp.cpp` - Integrated polling into normal mode

**Integration Points:**
- `handleNormalMode()` - WiFi connection, Roland client init, polling
- `initializeRolandClient()` - Create and configure client from stored config
- `pollRolandSwitch()` - Query switch and update tally state
- Automatic tally state updates based on Roland response

### 4. Test Infrastructure ✅

**Created Files:**
- `include/Utils/TestConfig.h` - Compile-time test configuration

**Test Mode Features:**
- Enable with `#define ENABLE_TEST_CONFIG`
- Hardcoded WiFi credentials (bypasses provisioning)
- Hardcoded switch configuration
- Hardcoded operating parameters
- Saves test config to NVS on startup
- **ADDED TO .gitignore** to keep credentials private

### 5. Performance Optimizations ✅

**Problem:** Serial output flooded with "Operations loaded" messages

**Solution:**
- Added `rolandPollInterval` cache to STACApp
- Load poll interval once during initialization
- Use cached value instead of repeated NVS reads
- Changed `ConfigManager::loadOperations()` log from `log_i` to `log_v`

**Result:**
- Clean serial output with only state changes
- Improved performance (no NVS reads every 300ms)
- Reduced flash wear

---

## Testing Results

### Test Environment
- **Hardware:** M5Stack ATOM Matrix (ESP32-PICO-D4)
- **Simulator:** Python script `sts_norm_8080.py` (V-60HD emulator)
- **Network:** Local WiFi (192.168.2.x)
- **Switch IP:** 192.168.2.58:8080
- **Poll Interval:** 300ms

### Test Observations ✅

**From Python Simulator:**
```
Request: 29:/tally/1/status
    --> onair
Request: 29:/tally/1/status
    --> unselected
Request: 29:/tally/1/status
    --> selected
```
- STAC connecting from IP ending in .29
- Requesting channel 1 every 300ms
- Receiving correct state responses
- Cycling through states every 10 seconds as programmed

**From STAC Serial Monitor:**
```
[118302][I][TallyStateManager.cpp:26] setState(): Tally state: UNSELECTED -> PREVIEW
[118310][I][STACApp.cpp:83] operator()(): Tally: UNSELECTED -> PREVIEW
[118320][I][STACApp.cpp:644] pollRolandSwitch(): Tally updated from Roland: PREVIEW
```
- Clean output (no log spam after optimization)
- Only state change messages
- Correct state mapping (ONAIR→PROGRAM, SELECTED→PREVIEW, UNSELECTED→UNSELECTED)

**Display Behavior:**
- RED when simulator returns "onair" (PROGRAM state)
- GREEN when simulator returns "selected" (PREVIEW state)
- GRAY/DIM when simulator returns "unselected" (UNSELECTED state)
- Automatic color changes every 10 seconds
- No manual button presses needed

### Verification ✅
- ✅ WiFi connection successful
- ✅ Roland client initialization successful
- ✅ Polling every 300ms as configured
- ✅ Correct protocol implementation (V-60HD tested)
- ✅ Tally state updates working
- ✅ Display color changes automatic
- ✅ No log spam after optimization

---

## Current Project State

### Completed Phases
- ✅ Phase 1: Configuration System
- ✅ Phase 2: Display Abstraction
- ✅ Phase 3: IMU Abstraction
- ✅ Phase 4: Hardware Interfaces
- ✅ Phase 5: Network & Storage
- ✅ Phase 6: State Management
- ✅ Phase 7: Application Layer
- ✅ Phase 8: Documentation & Polish
- ✅ **Phase 9 Part 1:** Glyph Management
- ✅ **Phase 9 Part 2:** Roland Protocol Implementation (V-60HD tested, V-160HD implemented)

### Still To Do
- ⏳ **Phase 9 Part 3:** Web Configuration Interface (provisioning mode)
- ⏳ **Phase 9 Part 4:** OTA Firmware Updates
- ⏳ Test V-160HD protocol with simulator
- ⏳ Arduino IDE compatibility testing

---

## Important Files Modified

### Protocol Implementation
```
include/Network/Protocol/
├── IRolandClient.h           (NEW - interface)
├── V60HDClient.h             (NEW - V-60HD header)
├── V160HDClient.h            (NEW - V-160HD header)
└── RolandClientFactory.h     (NEW - factory)

src/Network/Protocol/
├── V60HDClient.cpp           (NEW - V-60HD implementation)
└── V160HDClient.cpp          (NEW - V-160HD implementation)
```

### Configuration & Application
```
include/Storage/ConfigManager.h        (MODIFIED - added auth params)
src/Storage/ConfigManager.cpp          (MODIFIED - save/load auth)
include/Application/STACApp.h          (MODIFIED - Roland client member)
src/Application/STACApp.cpp            (MODIFIED - polling integration)
include/Utils/TestConfig.h             (NEW - test configuration)
```

---

## Key Code Patterns

### Runtime Switch Selection
```cpp
// Create client based on stored switch model
rolandClient = Network::RolandClientFactory::createFromString(model);
```

### Polling Pattern
```cpp
void STACApp::pollRolandSwitch() {
    // Use cached poll interval (loaded once at init)
    if (now - lastRolandPoll < rolandPollInterval) {
        return;
    }
    
    // Query switch
    Network::TallyQueryResult result;
    rolandClient->queryTallyStatus(result);
    
    // Map to TallyState and update display
    // Only log when state actually changes
}
```

### Test Mode Pattern
```cpp
#ifdef ENABLE_TEST_CONFIG
    // Apply hardcoded test configuration at startup
    configManager->saveWiFiCredentials(Test::TEST_WIFI_SSID, Test::TEST_WIFI_PASSWORD);
    configManager->saveSwitchConfig(Test::TEST_SWITCH_MODEL, ...);
    configManager->saveOperations(testOps);
#endif
```

---

## Build & Flash Commands

### Standard Build
```bash
cd /Users/robl/Documents/PlatformIO/Projects/STAC3/STAC
pio run -e atom-matrix
```

### Upload to Hardware
```bash
pio run -e atom-matrix -t upload
```

### Monitor Serial Output
```bash
pio device monitor
# or use CoolTerm, screen, etc.
```

### Run Roland Simulator (V-60HD)
```bash
cd /Users/robl/Documents/STAC-SmartTallyAtomClient/STSEmulator/v60_python_scripts
python3 sts_norm_8080.py
# Server starts on http://192.168.2.58:8080
# Cycles through states every 10 seconds
```

---

## Configuration Details

### Test Configuration (TestConfig.h)
```cpp
#define ENABLE_TEST_CONFIG  // Comment out to disable test mode

// WiFi
TEST_WIFI_SSID = "YourNetwork"
TEST_WIFI_PASSWORD = "YourPassword"

// Switch
TEST_SWITCH_MODEL = "V-60HD"
TEST_SWITCH_IP = "192.168.2.58"
TEST_SWITCH_PORT = 8080
TEST_TALLY_CHANNEL = 1
TEST_POLL_INTERVAL_MS = 300
```

**IMPORTANT:** `TestConfig.h` is in `.gitignore` - do not commit with real credentials!

### NVS Storage Structure
```
Namespace: "wifi"
- ssid: String
- password: String

Namespace: "switch"
- model: String ("V-60HD" or "V-160HD")
- ip: uint32_t (IPAddress as uint)
- port: uint16_t
- username: String (V-160HD only)
- password: String (V-160HD only)

Namespace: "operations"
- switchModel: String
- tallyChannel: uint8_t (1-based)
- channelBank: String ("bankA"/"bankB" for V-160HD)
- statusPollInterval: uint32_t (milliseconds)
- (other operating parameters)
```

---

## Known Issues & Notes

### Resolved During Session
1. ✅ Empty `handleNormalMode()` function - Fixed with proper WiFi and Roland polling code
2. ✅ Log spam from repeated NVS reads - Fixed with cached poll interval and log level change
3. ✅ File editing challenges - Worked around with targeted patches

### Current Limitations
- V-160HD protocol implemented but not tested (no V-160HD simulator run yet)
- Test mode requires manual editing of `TestConfig.h`
- Serial port locking issues when monitor is open during upload

### Future Improvements
- Add ability to change poll interval without reflashing
- Implement web configuration interface (Phase 9 Part 3)
- Add OTA update capability (Phase 9 Part 4)
- Add connection retry logic with exponential backoff
- Add diagnostic display modes (show IP, channel, etc.)

---

## Testing Checklist for Next Session

### V-60HD Protocol (Already Tested) ✅
- [x] WiFi connection
- [x] Roland client initialization
- [x] Polling at correct interval
- [x] Correct state mapping
- [x] Display updates
- [x] Python simulator interaction

### V-160HD Protocol (To Test)
- [ ] Authentication with username/password
- [ ] Bank-based channel requests
- [ ] Keep-alive connections
- [ ] Test with V-160HD simulator script
- [ ] Verify channel mapping (1-8 bankA, 9-16 bankB)

### Edge Cases (To Test)
- [ ] WiFi disconnect/reconnect behavior
- [ ] Switch unavailable/timeout handling
- [ ] Invalid credentials (V-160HD)
- [ ] Channel out of range
- [ ] Malformed responses

---

## Git Status

### Files to Commit
```
Modified:
- include/Storage/ConfigManager.h
- src/Storage/ConfigManager.cpp
- include/Application/STACApp.h
- src/Application/STACApp.cpp

New:
- include/Network/Protocol/IRolandClient.h
- include/Network/Protocol/V60HDClient.h
- src/Network/Protocol/V60HDClient.cpp
- include/Network/Protocol/V160HDClient.h
- src/Network/Protocol/V160HDClient.cpp
- include/Network/Protocol/RolandClientFactory.h
- include/Utils/TestConfig.h

Modified (gitignore):
- .gitignore (added include/Utils/TestConfig.h)
```

### Suggested Commit Message
```
feat: Implement Roland V-60HD/V-160HD protocol support

- Add IRolandClient interface for switch abstraction
- Implement V60HDClient using WiFiClient (no auth)
- Implement V160HDClient using HTTPClient (Basic Auth)
- Add RolandClientFactory for runtime switch selection
- Integrate polling into STACApp normal mode
- Add test configuration infrastructure
- Optimize NVS access (cache poll interval)
- Update ConfigManager for V-160HD authentication
- Successfully tested with V-60HD Python simulator

Tested on: M5Stack ATOM Matrix
Polling: Every 300ms
States: PROGRAM (red), PREVIEW (green), UNSELECTED (gray)
Display: Automatic updates based on Roland switch status
```

---

## Next Session Priorities

1. **Test V-160HD Protocol** - Run V-160HD simulator and verify authentication
2. **Begin Phase 9 Part 3** - Web configuration interface (provisioning mode)
3. **Cleanup Test Mode** - Consider more elegant test configuration approach
4. **Documentation** - Update main README with Roland protocol details

---

## Quick Reference Commands

### Start Development Session
```bash
# Open workspace
cd /Users/robl/Documents/PlatformIO/Projects/STAC3
code STAC3.code-workspace

# Start Roland simulator (separate terminal)
cd /Users/robl/Documents/STAC-SmartTallyAtomClient/STSEmulator/v60_python_scripts
python3 sts_norm_8080.py
```

### Build and Test
```bash
cd /Users/robl/Documents/PlatformIO/Projects/STAC3/STAC
pio run -e atom-matrix -t upload
# Then open serial monitor in VS Code or separate terminal
```

### Disable Test Mode
```cpp
// In include/Utils/TestConfig.h
// #define ENABLE_TEST_CONFIG  // Comment out this line
```

---

## Technical Notes

### Protocol Details

**V-60HD Request:**
```
GET /tally/1/status\r\n\r\n
```

**V-160HD Request:**
```
GET /tally/bankA1/status HTTP/1.1
Authorization: Basic <base64(username:password)>
User-Agent: STAC_XXXXX
Connection: keep-alive
```

**Responses (both):**
```
onair      -> Maps to TallyState::PROGRAM (red)
selected   -> Maps to TallyState::PREVIEW (green)
unselected -> Maps to TallyState::UNSELECTED (gray)
```

### State Machine
```
Normal Mode -> WiFi Connect -> Initialize Roland Client -> Poll Loop
                                                              ↓
                                                         Query Switch
                                                              ↓
                                                      Update TallyState
                                                              ↓
                                                       Update Display
```

---

**Context document created:** November 15, 2025  
**For continuation in:** Next VS Code session with Claude Sonnet 4.5  
**Status:** Phase 9 Part 2 complete and tested ✅
