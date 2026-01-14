# STAC v3 Project Context

**Version:** v3.0.0-RC.23  
**Branch:** `v3-config-import-export`  
**Updated:** January 14, 2026  
**Status:** IMU Calibration Tool - Display Corner Identification (Work in Progress)

---

## Quick Reference

### Supported Hardware

| Board | Display | IMU | MCU | Status |
|-------|---------|-----|-----|--------|
| M5Stack ATOM Matrix | 5×5 LED | MPU6886 | ESP32-PICO-D4 | ✅ Tested |
| Waveshare ESP32-S3-Matrix | 8×8 LED | QMI8658 | ESP32-S3 | ✅ Tested |
| M5StickC Plus | 135×240 TFT | MPU6886 | ESP32-PICO-D4 | ✅ Tested |
| LilyGO T-Display | 135×240 TFT | None | ESP32 | ✅ Tested |
| AIPI-Lite | 128×128 TFT | None | ESP32-S3 | ✅ Tested |

### Key Files to Edit

| Purpose | File |
|---------|------|
| Select board | `include/Device_Config.h` |
| Board settings | `include/BoardConfigs/<Board>_Config.h` |
| Version number | `include/Device_Config.h` (FIRMWARE_VERSION) |
| Build config | `platformio.ini` |

### Common Commands

```bash
cd /Users/robl/Documents/PlatformIO/Projects/STAC3/STAC

# Build
pio run -e atom-matrix              # Debug build
pio run -e atom-matrix-release      # Release build

# Upload & Monitor
pio run -e atom-matrix -t upload && pio device monitor -b 115200

# Generate release binaries
pio run -e atom-matrix-release -t merged -t ota
```

---

## Project Overview

**STAC** = **S**mart **T**ally **At**om **C**lient

A WiFi-enabled tally light system for Roland video switchers (V-60HD, V-160HD) using ESP32-based hardware.

### Operating Modes

1. **Normal Mode** - WiFi client polling Roland switch for tally state
2. **Peripheral Mode** - Receives tally via GROVE port from master STAC
3. **Provisioning Mode** - Unified web portal with captive portal for WiFi/switch configuration and OTA updates

---

## Architecture

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
- `IDisplay` interface → `Display5x5`, `Display8x8`, `DisplayTFT`
- `IIMU` interface → `MPU6886_IMU`, `QMI8658_IMU`
- `GlyphManager` → Orientation-aware glyph rotation
- Factory pattern for hardware creation

**Network Layer:**
- `WiFiManager` → Connection management
- `RolandClientBase` → `V60HDClient`, `V160HDClient`
- `WebConfigServer` → Web-based configuration (no captive portal)
- `DNSServer` → Local hostname resolution (stac.local)

**State Management:**
- `SystemState` → Central state coordinator
- `TallyStateManager` → PROGRAM, PREVIEW, UNSELECTED, NO_TALLY
- `OperatingModeManager` → Mode handling

**Storage:**
- `ConfigManager` → NVS-based configuration
- Namespaces: `stac`, `wifi`, `switch`, `v60hd`, `v160hd`, `peripheral`

---

## File Structure

```
STAC3/
├── STAC/
│   ├── platformio.ini
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Application/STACApp.cpp
│   │   ├── Hardware/
│   │   │   ├── Display/
│   │   │   │   ├── Matrix5x5/Display5x5.cpp
│   │   │   │   ├── Matrix8x8/Display8x8.cpp
│   │   │   │   └── TFT/DisplayTFT.cpp
│   │   │   ├── Sensors/MPU6886_IMU.cpp, QMI8658_IMU.cpp
│   │   │   └── Power/AXP192.cpp
│   │   ├── Network/
│   │   │   ├── WiFiManager.cpp
│   │   │   ├── Protocol/V60HDClient.cpp, V160HDClient.cpp
│   │   │   └── Web/WebConfigServer.cpp, OTAUpdateServer.cpp
│   │   ├── State/SystemState.cpp, TallyStateManager.cpp
│   │   └── Storage/ConfigManager.cpp
│   └── include/
│       ├── Device_Config.h           # Board selection
│       ├── BoardConfigs/             # Per-board settings
│       ├── Config/Constants.h        # Global constants
│       └── Hardware/Display/
│           ├── Glyphs5x5.h           # 5×5 glyphs + rotation LUTs
│           ├── Glyphs8x8.h           # 8×8 glyphs + rotation LUTs
│           └── TFT/                  # TFT display support
└── Documentation/
    ├── Developer/
    │   ├── DEVELOPER_GUIDE.md
    │   ├── HARDWARE_CONFIG.md
    │   ├── Detailed Change Log.md
    │   └── TFT_Display_Support_Context.md
    └── User/
        └── STAC Users Guide.md
```

---

## Adding New Hardware

### New LED Matrix Board

1. Create `include/BoardConfigs/NewBoard_Config.h` (copy from TEMPLATE)
2. Define display pins, LED type, IMU type, button GPIO
3. Include appropriate glyph header (`Glyphs5x5.h` or `Glyphs8x8.h`)
4. Add environment to `platformio.ini`
5. Add `#elif` in `Device_Config.h`

### New TFT Board

1. Create board config with `DISPLAY_TYPE_TFT` define
2. Create LovyanGFX config in `include/Hardware/Display/TFT/`
3. Configure backlight (PMU or PWM)
4. See `TFT_Display_Support_Context.md` for details

---

## Key Patterns

### Display Independence

```cpp
// Board config includes size-appropriate glyph header
#include "Hardware/Display/Glyphs5x5.h"  // Defines GLYPH_WIDTH=5

// Type aliases are auto-defined
using GlyphManagerType = GlyphManager<5>;
using StartupConfigType = StartupConfig<5>;

// Application code is display-agnostic
display->showGlyph(Display::GLF_CHECK);
```

### Configuration Flow

```cpp
// 1. Startup: Load from NVS into SystemState
configManager->loadSwitchConfig(...);
systemState->setOperations(ops);

// 2. Runtime: Access from SystemState (not NVS)
auto ops = systemState->getOperations();

// 3. Changes: Update both
systemState->setOperations(newOps);
configManager->saveSwitchConfig(newOps);
```

### Button B (Reset) Pattern (TFT boards)

```cpp
// STACApp.h
std::unique_ptr<XP_Button> buttonB;

// STACApp.cpp - init
buttonB = std::make_unique<XP_Button>(BUTTON_B_PIN, ACTIVE_LOW, enablePullup);

// STACApp.cpp - loop
buttonB->read();
if (buttonB->wasPressed()) {
    ESP.restart();
}
```

---

## Build Configuration

### Environments

| Environment | Board | Build Type | Debug Level |
|-------------|-------|------------|-------------|
| `atom-matrix` | ATOM Matrix | debug | 3 (INFO) |
| `atom-matrix-release` | ATOM Matrix | release | 0 (NONE) |
| `waveshare-s3` | Waveshare S3 | debug | 3 (INFO) |
| `waveshare-s3-release` | Waveshare S3 | release | 0 (NONE) |
| `m5stickc-plus` | M5StickC Plus | debug | 3 (INFO) |
| `lilygo-t-display` | T-Display | debug | 3 (INFO) |
| `aipi-lite` | AIPI-Lite | debug | 3 (INFO) |

### Version String Format

- **Release:** `3.0.0-RC.9 (b26d99)`
- **Debug:** `3.0.0-RC.9 (6928a8) D3`

---

## Testing Checklist

### Core Functionality
- [ ] Display: Colors correct, rotation follows IMU
- [ ] Button: Short press, long press (1.5s), boot sequences
- [ ] WiFi: Connect, reconnect, timeout handling
- [ ] Tally: PROGRAM (red), PREVIEW (green), UNSELECTED

### Operating Modes
- [ ] Normal: Poll Roland switch, display tally
- [ ] Peripheral: GROVE input, mirror master
- [ ] Provisioning: Captive portal, config save
- [ ] OTA: Upload firmware, restart
- [ ] Factory Reset: Clear NVS, restart

### Boot Button Sequence (HAS_PERIPHERAL_MODE_CAPABILITY)
| Hold Time | Action | Display | Color |
|-----------|--------|---------|-------|
| 0-2 sec | Toggle Peripheral Mode | P (enable) or N (disable) | Green |
| 2-4 sec | Unified Portal (Setup/OTA) | Gear icon | Orange (provisioned) / Red (not provisioned) |
| 4-6 sec | Factory Reset | Factory reset icon | Red |

### Boot Button Sequence (Non-Peripheral Boards)
| Hold Time | Action | Display | Color |
|-----------|--------|---------|-------|
| 0-2 sec | Unified Portal (Setup/OTA) | Gear icon | Orange (provisioned) / Red (not provisioned) |
| 2-4 sec | Factory Reset | Factory reset icon | Red |

---

## Recent Changes

### January 13, 2026 - IMU Calibration Tool - Arbitrary Home Position Support Complete

**Issue:** Calibration tool required USB DOWN as home position, failed with other orientations
- Validation test with USB LEFT as home revealed three bugs:
  1. Reverse mapping showed home at 90° instead of 0°
  2. Step 6 prompt required double ENTER press
  3. Display rotation LUT values wrong for FLAT/UNKNOWN orientations

**Root Cause Analysis:**
- deviceHome offset not applied to reverse mapping calculation
- Step 6 instruction case used `break` instead of `return`, falling through to waitForEnter()
- FLAT/UNKNOWN incorrectly used home vertical orientation instead of lut[0]
- Key insight: When device is physically flat, getOrientation() always returns ROTATE_0, regardless of which vertical position was chosen as calibration home

**Fixes Applied:**
- Reverse mapping: `physicalAngle = ((physPos - deviceHome) * 90 + 360) % 360`
- Step 6 prompt: Changed to `return` to skip waitForEnter()
- Instructions: Made hardware-agnostic (removed "USB port" references)
- FLAT/UNKNOWN: Use `lut[0]` instead of `lut[orientationEnums[deviceHome]]`

**Testing:**
- USB LEFT as home with Bottom-Left pixel corner: All orientations correct ✅
- Generates LUT matching working manual config ✅
- Physical angle logging shows 0° for chosen home ✅

**Files Modified:**
- `src/main_calibrate.cpp`: Fixed reverse mapping, double ENTER, FLAT calculation
- All three bugs resolved, calibration tool now works for any home position choice

**Commit:** be14e79

---

### January 13-14, 2026 - Calibration Tool Redesign: Breaking Rotational Symmetry - Work in Progress

**Problem Discovery:**
- Testing calibration tool with USB RIGHT as home (instead of USB DOWN) revealed fundamental issue
- Generated DIFFERENT axis remap: `X=(acc.x), Y=((-acc.y))` vs expected `X=((-acc.y)), Y=((-acc.x))`
- Axis remap should be physical property of mounting (invariant), not dependent on which orientation chosen as home
- Root cause: Accelerometer pattern has 4-fold rotational symmetry
  - Pattern: `X:(-1,0,+1,0), Y:(0,-1,0,+1)` looks identical when rotated 90°, 180°, 270°
  - Different measurement sequences can match the same expected pattern with different axis remaps

**Solution: Display Pixel Corner Identification**
- Use display pixel 0 as absolute reference to break rotational symmetry
- New calibration flow adds corner identification step:
  1. FLAT (reference gravity vector)
  2. HOME (user's chosen home position)
  3. **CORNER_IDENTIFY** (light corners sequentially, user identifies which is top-left)
  4. Measure orientations at 90°, 180°, 270° from home
  5. CALCULATE (use corner position to establish absolute reference frame)

**Implementation Progress:**
- ✅ Corner identification UI: Sequential corner lighting with Y/N prompts
- ✅ Absolute pattern calculation: Rotate expected pattern by corner offset
- ✅ Pattern matching: Match measurements against absolute pattern
- ✅ **Major Milestone: Axis remap now INVARIANT!**
  - Both USB RIGHT and USB DOWN home produce: `X=((-acc.y)), Y=((-acc.x))`
  - Rotational symmetry successfully broken
- ❌ LUT calculation formula still incorrect (display rotation wrong by ~90°)
- ❌ Physical angle logging shows incorrect values

**Documentation Created:**
- `Documentation/Developer/IMU_Display_Reference_Tables.md`: 8 comprehensive rotation tables
  - Demonstrates 4-fold rotational symmetry visually
  - Shows IMU readings for all orientations with Z-away and Z-toward
  - LUT rotation mapping tables
- `Documentation/Developer/XP IMU Analysis 2.md`: User insight document
  - Proposes pattern-based simplification: store pattern numbers, use modular arithmetic
  - May lead to algorithm redesign in next phase

**Files Modified:**
- `src/main_calibrate.cpp`: Major redesign (~300 lines)
  - State machine: Added STATE_CORNER_IDENTIFY, removed old corner selection states
  - New function: `identifyTopLeftCorner()` with sequential corner UI
  - Algorithm: Calculate absolutePattern, match measurements in absolute space
  - LUT calculation: Multiple iterations attempted, still not correct
- `include/BoardConfigs/AtomMatrix_Config.h`: Latest test configuration
- `Documentation/Developer/IMU_Calibration_Methodology.md`: Updated 6-step flow

**Current State:**
- Axis remap detection: **WORKING** (invariant achieved)
- LUT calculation: **BROKEN** (needs formula fix)
- Corner identification: **WORKING** (UI perfect)
- Overall: Partial success - major breakthrough on symmetry, but complete solution in progress

**Next Steps:**
- Fix LUT calculation formula (consider pattern-based approach from XP Analysis)
- Test complete flow on both ATOM and Waveshare
- Validate all orientations display correctly

**Commit:** (pending - save point before continuing)

---

### January 12, 2026 - IMU Orientation System Complete - Waveshare Working, Calibration Tool Updated

**Session 1: Critical Bug Discovery & Architecture Refinement**
- **CRITICAL BUG FIX:** GlyphManager was receiving raw physical orientation instead of mapped display orientation
  - STACApp.cpp was reading IMU orientation twice:
    1. First read: Applied DEVICE_ORIENTATION_TO_LUT_MAP for logging (correct)
    2. Second read: Passed raw orientation to GlyphManager (WRONG - bypassed mapping!)
  - Result: LUT mapping only affected log output, not actual glyph rotation
  - Fix: Consolidated to single IMU read, store mapped orientation in variable, use for both logging and GlyphManager
  - Code duplication eliminated, single source of truth for display orientation
  - Commit: 2ff4679 "Consolidate IMU orientation detection - fix GlyphManager bug"

- **Waveshare ESP32-S3-Matrix Z-axis flip handled:**
  - QMI8658 IMU has Z+ pointing AWAY from display (vs AtomMatrix Z+ toward display)
  - Requires Y-axis rotation LUT swap: Physical 0°→LUT_180, Physical 180°→LUT_0
  - X-axis rotations unchanged: Physical 90°→LUT_90, Physical 270°→LUT_270
  - DEVICE_ORIENTATION_TO_LUT_MAP config:
    ```cpp
    { Orientation::ROTATE_180,   // Physical 0°   → LUT_ROTATE_180
      Orientation::ROTATE_90,    // Physical 90°  → LUT_ROTATE_90  
      Orientation::ROTATE_0,     // Physical 180° → LUT_ROTATE_0   
      Orientation::ROTATE_270,   // Physical 270° → LUT_ROTATE_270 
      Orientation::ROTATE_180,   // FLAT          → same as 0°
      Orientation::ROTATE_180 }  // UNKNOWN       → same as 0°
    ```
  - Insight: IMU_AXIS_REMAP handles coordinate transformation, LUT mapping handles rotation offset

- **Board-specific LUT mapping architecture:**
  - Replaced simple rotation offset with per-board LUT mapping table
  - Each board defines DEVICE_ORIENTATION_TO_LUT_MAP[6] array
  - Maps each physical orientation (0°/90°/180°/270°/FLAT/UNKNOWN) to display LUT
  - Supports both Z-axis orientations (toward/away from display)
  - FLAT and UNKNOWN use home position LUT (typically same as 0°)

- **Architectural separation - Physical vs Display:**
  - IMU layer: Returns RAW physical orientation based on accelerometer only
    * No offset application at IMU level
    * Axis remapping (IMU_AXIS_REMAP_X/Y/Z) handles coordinate transformation
    * Physical orientation logging always accurate
  - Application layer: Applies board-specific LUT mapping
    * STACApp.cpp looks up display orientation using DEVICE_ORIENTATION_TO_LUT_MAP
    * Passes mapped orientation to GlyphManager
    * Logging shows both LUT name and physical orientation

- **Complete nomenclature refactoring:**
  - Eliminated UP/DOWN/LEFT/RIGHT terminology throughout codebase
  - Replaced with rotation degrees: ROTATE_0, ROTATE_90, ROTATE_180, ROTATE_270, FLAT, UNKNOWN
  - Updated: Orientation enum, all LUT names, GlyphManager, DisplayTFT, IMU drivers
  - LUT names: LUT_ROTATE_0/90/180/270 (instead of LUT_UP/RIGHT/DOWN/LEFT)
  - Clearer intent: Rotation angle, not arbitrary direction

- **Enhanced debug logging:**
  - Shows LUT being used: "LUT being used: LUT_ROTATE_180"
  - Shows physical orientation: "Physical device orientation: 0°"
  - Removed confusing "Display rotation applied" log (redundant with LUT name)
  - Raw IMU values logged in QMI8658_IMU.cpp: "Raw IMU: acc.x=..., boardX=..., boardY=..."

- **Testing results (Session 1):**
  - Waveshare ESP32-S3-Matrix: All orientations (0°/90°/180°/270°/FLAT) display correctly ✅
  - Physical orientation detection accurate at all positions ✅
  - Display rotation matches physical device orientation ✅
  - FLAT and UNKNOWN use home orientation LUT ✅

**Session 2: Calibration Tool Update & End-to-End Validation**
- **Calibration tool updated to match new architecture:**
  - Removed obsolete IMU_ROTATION_OFFSET detection logic (~70 lines)
  - Added Z-axis orientation detection based on FLAT_UP measurement
    * If FLAT_UP Z < 0: Z+ points AWAY from display
    * If FLAT_UP Z > 0: Z+ points TOWARD display
  - Generates DEVICE_ORIENTATION_TO_LUT_MAP in proper C++ array format
    * Z+ away: Outputs Y-axis swapped LUTs (0°↔180°)
    * Z+ toward: Outputs direct LUT mapping
  - Retained axis remapping detection (X/Y sensor mapping via delta analysis)
  - Output ready for copy/paste into board config files
  - Commit: 9596562 "Update calibration tool to output DEVICE_ORIENTATION_TO_LUT_MAP"

- **End-to-end validation complete:**
  - Ran calibration tool on Waveshare ESP32-S3-Matrix
  - Tool output matched existing board configuration exactly ✅
  - Uploaded main STAC application
  - Tested all orientations (0°, 90°, 180°, 270°, FLAT)
  - All orientations display glyphs correctly ✅
  - IMU detection, LUT mapping, and display rotation all working perfectly ✅

- **Files modified (both sessions):**
  - src/Application/STACApp.cpp: Consolidated IMU read, removed duplication, cleaner logging
  - include/Config/Types.h: Orientation enum ROTATE_* values
  - include/Hardware/Display/Glyphs8x8.h & Glyphs5x5.h: LUT_ROTATE_* names
  - src/Hardware/Display/GlyphManager.cpp: Updated to ROTATE_* enum
  - src/Hardware/Sensors/QMI8658_IMU.cpp: Returns raw orientation, enhanced logging
  - src/Hardware/Sensors/MPU6886_IMU.cpp: Returns raw orientation
  - include/BoardConfigs/WaveshareS3_Config.h: DEVICE_ORIENTATION_TO_LUT_MAP with Z-flip swap
  - include/BoardConfigs/AtomMatrix_Config.h: DEVICE_ORIENTATION_TO_LUT_MAP (direct mapping)
  - src/Hardware/Display/TFT/DisplayTFT.cpp: Updated to ROTATE_* enum
  - src/main_calibrate.cpp: Z-axis detection, DEVICE_ORIENTATION_TO_LUT_MAP output

- **Key insights from debugging:**
  - System appeared to work in logs but display was wrong
  - Root cause: Logging code path vs execution code path diverged
  - LUT mapping was only applied for logging, not for actual GlyphManager
  - Classic case of code duplication hiding critical bug
  - Consolidated to single code path with single source of truth

- **Production ready:**
  - Complete IMU orientation system working on Waveshare ESP32-S3-Matrix ✅
  - Calibration tool generates correct configuration ✅
  - Board config applies correct LUT mapping ✅
  - End-to-end workflow validated ✅
  - Architecture clean, maintainable, and documented ✅

- **Next steps:**
  - Test AtomMatrix with current architecture (expected to work with existing config)
  - Clean up obsolete code (OrientationOffset enum, applyOrientationOffset() function)
  - Consider removing IMU_ROTATION_OFFSET from board config files (no longer used)

### January 8, 2026 - OTA Update UX Improvements & Build System Enhancements
- **OTA Progress Tracking (RC.11):**
  - Added JavaScript XMLHttpRequest-based upload with real-time progress bar
  - Visual progress: 0-100% animated green bar, percentage text display
  - Works across all browsers including mobile (Android Chrome/Edge tested)
  - Flash impact: +2.7KB for JavaScript code
- **Symbol Encoding Fix (RC.11):**
  - Replaced UTF-8 symbols (✓, ✗, ⚠️) with HTML entities (&#10004;, &#10008;, &#9888;)
  - Fixes rendering issues on Android WebView and captive portals
  - Universal browser compatibility without font dependencies
- **Version Display Enhancement (RC.11):**
  - Changed from hardcoded "3.0.0-unified" to `STAC_SOFTWARE_VERSION` macro
  - Version now updates correctly after OTA firmware updates
  - Single source of truth in `Device_Config.h`
- **Build Number & Git Info Display (RC.12):**
  - Serial console: Renamed "Build:" to "Git:" to avoid confusion with build number
  - Web portal: Now displays `BUILD_FULL_VERSION` (e.g., "3.0.0-RC.12 (7259c9)")
  - Added Git commit info to web portal matching serial console format
  - Build number = MD5 hash of source files (tracks code changes)
  - Git commit = repository SHA (tracks commits)
- **Build System Automation (RC.13):**
  - Fixed `build_version.py` to auto-generate `build_info.h` before compilation
  - Version changes in `Device_Config.h` now automatically update build artifacts
  - Script runs immediately when loaded, ensures build_info.h exists before compilation starts
  - Eliminated manual build_info.h regeneration requirement
- **Captive Portal Platform Detection (RC.14):**
  - Enhanced JavaScript detection for macOS, iOS, Android, Windows, and Linux
  - **iOS captive portal:** Full functionality (Setup + Maintenance tabs) - file uploads work natively
  - **macOS captive portal:** Setup tab only + notice - file uploads don't work in captive portal WebView
  - **Android captive portal:** Setup tab only + notice - file uploads don't work in WebView
  - **Desktop browsers (all platforms):** Full functionality when user manually navigates to portal
  - Updated notice text to mention Safari for macOS users
  - Fixed stuck "Opening in browser..." issue - now always shows appropriate content
  - Detection logic: Uses `CaptiveNetworkSupport` presence + platform detection (iPhone/iPad/iPod vs Mac OS X)
  - Validated on: iOS (iPhone), macOS (Safari captive portal + manual Safari), Android (Chrome), Windows (Edge/Chrome), Debian 13 (Firefox)

### January 9, 2026 - Configuration Import/Export & Captive Portal Removal
- **Configuration Import/Export Feature:**
  - Added JSON-based configuration export/import for multi-device deployment
  - Export options: Copy to clipboard or download as file (`stac-v60hd-config.json` / `stac-v160hd-config.json`)
  - Import options: Upload file or paste from clipboard
  - HTTP-compatible clipboard paste using `execCommand()` (works without HTTPS)
  - Model mismatch validation with user confirmation dialog
  - Auto-updates export JSON as user types in forms
  - Enables quick cloning of settings across multiple STAC devices
- **Captive Portal Removal:**
  - Abandoned captive portal approach - iOS has no intuitive exit method, Android/iOS still show "no internet" warnings
  - DNS server changed from wildcard redirect (`*`) to hostname-only (`stac.local`)
  - Users now manually connect to STAC AP, then navigate to http://stac.local or http://192.168.6.14
  - Updated boot banner: "Access: http://stac.local (or http://192.168.6.14)"
  - Removed captive portal detection endpoints (simplified from 8+ endpoints to just 1 for macOS compatibility)
  - Simpler, more reliable user experience across all platforms
- **Geek Info Modal:**
  - Added "Show Geek Info" button on Maintenance tab
  - Displays Device, MAC, Firmware, Git commit, Core version, SDK version
  - Modal dialog with Copy and OK buttons
  - Line breaks properly rendered in display and preserved when copied
- **WebPortal → WebConfig Rename:**
  - Renamed `WebPortalServer` → `WebConfigServer` (no longer a "portal" since captive portal removed)
  - Renamed `WebPortalPages` → `WebConfigPages`
  - Updated all namespace references: `WebPortal` → `WebConfig`
  - Updated include guards, file headers, comments for consistency
  - Removed dead code: `openInBrowser()` function, portal-notice div
- **UX Improvements:**
  - Page title: "STAC Portal" → "STAC Setup"
  - Import: Replaced textarea with "Paste Settings" button
  - Export: "Copy Settings" and "Save Settings" buttons (below Configure STAC)
  - Button order: Configure STAC, then Reset/Back on second row
  - Factory reset confirmation dialog
  - Landing page simplified (removed captive portal exit instructions)
- **Technical Details:**
  - Version: 3.0.0-RC.22
  - Branch: v3-config-import-export (from v3-unified-portal at commit 1534119)
  - Flash: 67.8% (1,319,735 bytes), RAM: 15.7% (51,348 bytes)
  - Tested on: iOS, Android, macOS
  - Git commits: RC.19-RC.22 progression

### January 8, 2026 - Multi-Platform Glyph Compatibility Fixes & Complete Platform Validation
- **Waveshare ESP32-S3-Matrix (8×8 LED):** Added GLF_X alias to GLF_BX (existing Big X glyph at index 18)
  - Boot button sequence uses GLF_X for error states
  - 8×8 glyph set already had Big X icon, just needed alias for compatibility
  - Build: Flash 65.7% (1,267,347 bytes), RAM 15.0% (49,192 bytes)
  - Tested: Unified portal working, boot button sequence visual feedback confirmed
- **M5StickC Plus (TFT LCD):** Added GLF_N glyph support to TFT display system
  - TFT displays render letter glyphs (C, T, A, S, P) using `drawString()` with FreeSansBold font
  - GLF_N was missing when boot sequence changed from GLF_P_CANCEL to GLF_N in commit 164772e
  - Added GLF_N constant (index 36) to `GlyphsTFT.h` to match LED matrix glyph sets
  - Added GLF_N to BASE_GLYPHS array and drawGlyph() switch case to render 'N'
  - Build: Flash 72.1% (1,402,067 bytes), RAM 15.7% (51,368 bytes)
  - Tested: Boot button sequence shows proper glyphs (N, gear, factory reset icons)
- **AIPI-Lite (TFT LCD):** Validated with GLF_N support
  - Build: Flash 16.7% (1,393,195 bytes), RAM 15.4% (50,376 bytes)
  - Tested: Boot button sequence, unified portal, all features working
- **LilyGO T-Display (TFT LCD):** Validated with GLF_N support
  - Build: Flash 72.4% (1,408,063 bytes), RAM 15.7% (51,572 bytes)
  - Tested: Boot button sequence (P, gear, factory reset glyphs), provisioning mode with captive portal, factory reset, configuration save/load, WiFi connection
  - Serial monitor confirmed: startup config sequence, web portal operation, successful configuration submission, restart and normal mode operation
- **Key insight:** TFT and LED matrix displays have fundamentally different architectures:
  - LED matrix: Include `Glyphs5x5.h` or `Glyphs8x8.h` with bitmap glyph data arrays
  - TFT: Include `GlyphsTFT.h` with stub arrays (just index values), render glyphs as graphics primitives
  - Both use same glyph index constants for application code compatibility
- **All five platforms validated:** ATOM Matrix (5×5 LED), Waveshare S3 (8×8 LED), M5StickC Plus (TFT), AIPI-Lite (TFT), LilyGO T-Display (TFT)
- **Production ready:** Unified portal fully operational across entire hardware ecosystem
- Git commits: a96ab82 (GLF_X alias), c88705c (GLF_N support)

### January 7-8, 2026 - Unified Web Portal with Captive Portal Support & Cross-Platform Testing
- **Major architectural change:** Combined provisioning and OTA into single unified web portal
- Replaced separate `WebConfigServer` and `OTAUpdateServer` with unified `WebPortalServer`
- Deleted 6 obsolete files: WebConfigServer.{h,cpp}, OTAUpdateServer.{h,cpp}, WebConfigPages.h, OTAUpdatePages.h (1,521 lines removed)
- **Key Features:**
  - Tabbed interface: "Setup" tab (WiFi + Roland configuration) and "Maintenance" tab (Firmware Update + Factory Reset)
  - Auto-popup captive portal on iOS/iPadOS/Android/Windows devices
  - DNSServer integration for DNS redirect to 192.168.6.14
  - Captive portal detection endpoints: `/hotspot-detect.html` (iOS), `/generate_204` (Android), `/connecttest.txt` (Windows)
  - mDNS support: Access portal at **http://stac.local** (hostname: "stac")
  - Fallback direct access: `http://192.168.6.14`
- **Platform-Specific Behavior:**
  - **iOS/iPadOS captive portal:** Auto-redirect to Safari after 1.5s (captive portal restrictions bypass), full functionality in Safari
  - **iOS/iPadOS caching:** After first successful setup, iOS caches network as "known good" and won't show captive portal on reconnection (user must use browser or "forget" network)
  - **Android captive portal:** Setup tab only with instructions to open browser for OTA (WebView blocks file input for security, navigation attempts blocked)
  - **Android browser (Chrome/Firefox):** Full functionality, both tabs visible, file upload works
  - **Desktop browsers:** Full functionality with both tabs
  - Browser detection: Identifies Chrome/Firefox/Safari vs WebView (checks User-Agent for browser name + absence of `wv`/`WebView`)
- **iOS/iPadOS Compatibility Fixes:**
  - Removed UTF-8 special characters (displayed incorrectly on iOS)
  - Replaced JavaScript `confirm()` dialogs with HTML5 required checkbox (JavaScript dialogs blocked in iOS captive portal)
  - Added `inputmode="numeric"` and `pattern="[0-9]*"` for numeric keyboards on mobile devices (both attributes required for iOS reliability)
  - Added `inputmode="decimal"` for IP address fields (numeric keyboard with decimal point)
  - Auto-redirect button uses `<a href target="_blank">` instead of JavaScript `window.location` (Android best practice)
- **Form Validation (matching v2.x implementation):**
  - IP address pattern: `^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$`
  - SSID: Required, max 32 characters
  - Password: Optional (allows open WiFi), max 63 characters (corrected from 64)
  - Port: Required, 1-65535
  - Max channels (V-60HD/V-160HD): Required, 1-8
  - Poll interval: Required, 175-2000ms
  - V-160HD LAN credentials: Required, max 32 characters each
- **UX Improvements:**
  - Factory reset: Simplified to single button click (removed checkbox confirmation requirement)
  - Android captive portal: Step-by-step instructions to open browser at http://stac.local or http://192.168.6.14
  - All platforms: Consistent mDNS + IP fallback guidance
- **Form Defaults:**
  - V-60HD: port=80, maxChan=6, poll=300ms
  - V-160HD: port=80, poll=300ms, LAN user="admin", LAN pass="admin", HDMI/SDI channels=8
- **Boot Button Sequence Updates:**
  - 0-2 sec: Toggle Peripheral Mode (if capable) - shows green N glyph when exiting to normal mode
  - 2-4 sec: Unified Portal (Setup/OTA) - shows orange/red gear icon
  - 4-6 sec: Factory Reset - shows red factory reset icon
- **Display Improvements:**
  - Added GLF_N glyph for 5×5 and 8×8 displays (clearer than slash-through-P for normal mode)
  - Peripheral mode exit sequence: green N flash → checkmark → clear → channel number
  - Fixed display artifacts when transitioning from peripheral mode to normal mode (added clear() calls)
  - Web-based factory reset calls `ESP.restart()` directly (no orange glyph artifact)
- **Documentation:**
  - Updated DEVELOPER_GUIDE.md with unified portal architecture and platform-specific captive portal behavior
  - Updated "Installing STAC v3.x Unified Portal.md" with comprehensive platform-specific notes section
  - Documented iOS caching behavior, Android WebView restrictions, mDNS compatibility requirements
  - Serial monitor shows "Portal: Captive (auto-popup)" and "Fallback: http://192.168.6.14"
- **Testing:** Validated on iOS/iPadOS (captive portal + Safari, caching behavior confirmed), Android (captive portal + Chrome browser), desktop browsers
- **Memory:** Flash 67.1% (1,305,735 bytes), RAM 15.7% (51,348 bytes)

### December 1, 2025 - TFT Display Startup Artifact Fix
- Fixed display artifacts (stale pixels) showing during soft reset on TFT displays
- **Root cause:** LCD driver memory persists through ESP32 soft reset; hidden offset regions retain stale data
- **Solution:** Unified approach for all TFT panels - clear full driver memory before normal `fillScreen()`
- **M5StickC Plus (ST7789):** Disabled LDO2 (backlight) in AXP192 init - let display code turn it on
- Common fixes for all TFT boards:
  - Early backlight OFF in `main.cpp` before any other init
  - Backlight OFF before all `ESP.restart()` calls
  - Removed LGFX Light_PWM backlight management - control directly via LEDC/analogWrite or PMU
  - Added `PreRestartCallback` to WebConfigServer and OTAUpdateServer
  - Clear full LCD driver memory via `setWindow()` to address hidden offset regions:
    - ST7735/ST7735S: 132×162 memory for various display sizes
    - ST7789: 240×320 memory for various display sizes
    - ILI9341/ILI9342: 240×320 memory
    - GC9A01: 240×240 memory
- Changed AIPI-Lite backlight control from `ledcAttach/ledcWrite` to simpler `analogWrite()`

### November 30, 2025 - AIPI-Lite TFT Display Working
- Added full support for AIPI-Lite board (ESP32-S3 + 128×128 ST7735S TFT)
- **Key finding:** Display labeled ST7789 but requires ST7735S driver
- Added `Panel_ST7735S` support to unified LGFX_STAC.h configuration
- Added `TFT_ROTATION_OFFSET` for board-specific rotation adjustment
- Added `_lcd->setSwapBytes(true)` after init for correct RGB565 colors
- Added configurable `TFT_READABLE` and `TFT_BUS_SHARED` options
- LovyanGFX requires develop branch (1.2.9+) for ESP32-S3/ESP-IDF 5.5 compatibility
- AIPI-Lite specific settings: offset_rotation=2, rotation_offset=3, 27MHz SPI, BGR

### January 12, 2026 - IMU Calibration Redesign - Corner-Based Rotation Method
- **Redesigned calibration tool with corner-based approach:**
  - Previous method used edge-based pixel placement (TOP/RIGHT/BOTTOM/LEFT) - too confusing
  - New method: Light single pixel at buffer index 0, rotate device to place at 4 corners
  - Clearer instructions: "Rotate device until pixel is at TOP-LEFT corner"
  - Measurements taken at: TOP-LEFT (0°), TOP-RIGHT (90°), BOTTOM-RIGHT (180°), BOTTOM-LEFT (270°)
- **Added automatic OFFSET detection:**
  - Analyzes gravity magnitude at each rotation: |g| = √(accX² + accY² + accZ²)
  - Rotation with strongest gravity = device UP (vertical, home position)
  - Calculates OFFSET automatically - no manual adjustment needed
  - Outputs: `#define IMU_ROTATION_OFFSET OrientationOffset::OFFSET_270`
- **Waveshare ESP32-S3-Matrix calibration completed:**
  - Axis mapping: X=((-acc.y)), Y=((-acc.x)), Z=(acc.z)
  - Detected OFFSET: OFFSET_270 (gravity magnitude 1.399g at 270° measurement)
  - DEVICE_ORIENTATION_MAP: Maps display Orientation enum to physical device degrees
  - Board-specific mapping: { "90°", "270°", "0°", "180°", "FLAT", "UNKNOWN" }
- **Enhanced orientation logging:**
  - Dual logging: "Display rotation applied" vs "Physical device orientation"
  - Display rotation: Glyph rotation (0°/90°/180°/270°) applied to buffer
  - Physical orientation: Actual device position (0°/90°/180°/270°/FLAT)
  - DEVICE_ORIENTATION_MAP bridges Display enum to physical degrees
- **Fixed FLAT orientation detection:**
  - QMI8658_IMU.cpp: Now returns Orientation::FLAT when Z-axis dominant
  - Previously returned RIGHT (causing FLAT to show as 180°)
  - FLAT special handling: Uses OFFSET rotation directly for display
- **Identified fundamental calibration methodology issue:**
  - Current approach has double rotation compensation (OFFSET + runtime adjustment)
  - Calibration measurements show device was not consistently vertical
  - Measurement 0: Z=1.085g (device flat), Measurement 3: Z=0.000g (device vertical)
  - Root cause: Calibration reference frame based on pixel position, not device home
  - OFFSET compensates for sensor mounting relative to calibration frame, not device frame
- **Outstanding issues to resolve:**
  - Display rotation working correctly (90° compensation applied in STACApp.cpp)
  - Physical orientation logging inverted (shows 270° when device at 0°)
  - FLAT orientation 180° out of phase (should match home rotation)
  - End-to-end methodology needs revision to eliminate trial-and-error
  - Suspect Z-axis mapping or sensor coordinate system mismatch
  - Need to verify SensorLib returns axes in expected order
- **Next steps:**
  - Verify raw sensor axis order from QMI8658 via SensorLib
  - Add debug output showing which physical direction each axis points
  - Redesign calibration to explicitly define device home position (vertical, USB down)
  - Ensure device held vertical during all measurements (gravity in XY plane, Z≈0)
  - Validate Z-axis is perpendicular to display after remapping
- **Files modified:**
  - `src/main_calibrate.cpp`: Redesigned state machine for corner-based rotation
  - `include/Calibration/CalibrationDisplayLED.h`: Added `showPixelAtIndex()`
  - `include/BoardConfigs/WaveshareS3_Config.h`: Applied calibration values
  - `src/Application/STACApp.cpp`: Added FLAT handling and dual orientation logging
  - `src/Hardware/Sensors/QMI8658_IMU.cpp`: Fixed FLAT detection
- **Testing status:** Waveshare rotation working, but calibration methodology under review

### January 11, 2026 - IMU Calibration System Complete
- **Identified fundamental IMU orientation issue:**
  - Current implementation assumed 4 possible IMU orientations (0°/90°/180°/270° rotation)
  - Reality: IMU can be mounted in 8 configurations (2 Z-axis directions × 4 rotations)
    * Z+ toward display (FORWARD) or Z+ away from display (AFT)
    * Rotated 0°, 90°, 180°, or 270° relative to board Home position
  - Different boards (ATOM vs Waveshare) have different physical mountings
  - See `IMU_Analysis.md` for detailed analysis
- **Implemented interactive calibration tool:**
  - Development-time offline calibration utility (not runtime NVS-based)
  - New PlatformIO environments: `atom-matrix-calibrate`, `waveshare-s3-calibrate`, `m5stickc-plus-calibrate`, `lilygo-t-display-calibrate`, `aipi-lite-calibrate`
  - Minimal builds: Exclude Network, State, Storage, Application layers (343KB flash, 23KB RAM)
  - Interactive procedure via serial monitor:
    1. Place board in Home position (USB down)
    2. Tool shows reference marker (white pixel on LED / arrow on TFT)
    3. User reports marker location (Top/Bottom/Left/Right)
    4. Rotate board 90° clockwise, repeat 3 more times
  - Algorithm analyzes accelerometer data at each rotation:
    * Identifies dominant sensor axis (X/Y/Z) at each position
    * Determines axis polarity (+ or -)
    * Detects standard vs swapped axis mapping
    * Calculates rotation offset from marker progression
  - Outputs complete board config values with proper syntax:
    ```cpp
    #define IMU_AXIS_REMAP_X    (acc.x)      // or (-acc.y), etc.
    #define IMU_AXIS_REMAP_Y    (acc.y)      // or (acc.x), etc.
    #define IMU_AXIS_REMAP_Z    (acc.z)
    #define IMU_ROTATION_OFFSET OrientationOffset::OFFSET_0     // or OFFSET_90/180/270
    ```
  - Removed unused IMU_FACE_DIRECTION (not implemented in code)
- **Axis remapping integration:**
  - Added `getRawAcceleration()` to IIMU interface (MPU6886, QMI8658)
  - Updated IMU drivers to read and apply `IMU_AXIS_REMAP_X/Y/Z` from board config
  - Axis remapping expressions evaluated at runtime (compile-time defined, runtime evaluated)
  - Supports standard mapping (X→X, Y→Y) and swapped mapping (X→Y, Y→X)
  - Supports polarity inversions (e.g., -acc.y)
  - Changed `IMU_ORIENTATION_OFFSET` → `IMU_ROTATION_OFFSET` for clarity
- **Display rotation validation:**
  - Display now correctly rotates based on physical device orientation
  - ATOM Matrix requires `OFFSET_90` (raw IMU LEFT becomes display UP)
  - Tested all 4 rotations (0°/90°/180°/270°) plus FLAT detection
- **Enhanced debugging and logging:**
  - Updated `Orientation` enum documentation in Types.h
    * Clarifies enum represents which display edge should be "up" for correct character display
    * Documents rotation degrees for each value (UP=0°, DOWN=180°, LEFT=270°, RIGHT=90°)
    * Explains difference between display orientation (after offset) and device orientation
  - Enhanced debug logging in IMU drivers:
    * Shows both direction names AND rotation degrees (e.g., "UP (0°)", "RIGHT (90°)")
    * Format: `Display orientation: raw=LEFT (270°), offset=1, corrected=UP (0°)`
  - User-facing log shows physical device orientation:
    * Reports device rotation in degrees: "Device orientation: 0°" (USB down - home)
    * Mapping: Display UP→Device 0°, DOWN→180°, LEFT→90°, RIGHT→270°, FLAT→FLAT
    * More meaningful than abstract directions (UP/DOWN/LEFT/RIGHT)
- **Implementation details:**
  - Display-agnostic architecture: `CalibrationDisplay` base class
  - `CalibrationDisplayLED`: Shows white pixel at top-center (5×5, 8×8 matrices)
  - `CalibrationDisplayTFT`: Shows upward arrow marker (TFT displays)
  - `main_calibrate.cpp`: 9-state interactive state machine
  - Display clears on completion
  - Build system: Excluded main_calibrate.cpp from normal builds (prevents linker conflicts)
- **Testing and validation:**
  - ATOM Matrix (MPU6886): Standard mapping (acc.x, acc.y, acc.z) with OFFSET_90
  - Display rotation matches device orientation in all positions
  - Device orientation logging verified accurate: 0°/90°/180°/270°/FLAT
  - End-to-end validation: Calibration tool → board config → runtime → correct display rotation
- **Architecture achievement:**
  - Self-discovering IMU calibration system
  - Manufacturing-agnostic solution (same IMU chip, different physical mounting)
  - Solves the 8-orientation problem (2 Z-axis directions × 4 rotations)
  - Ready for Waveshare testing (expected: different axis mapping)
- **Documentation:** `Documentation/Developer/IMU_CALIBRATION_TOOL.md`
- **Benefits:**
  - Self-discovering: No manual sensor orientation analysis
  - Manufacturing-agnostic: Works regardless of PCB assembly variations
  - Comprehensive: Handles both axis remapping and rotation offset
  - Version-controlled: Config values committed to git, survive flash erases
  - One-time: Only needed during hardware bring-up
  - Supports both LED matrix (pixel marker) and TFT displays (visual marker)
- **Benefits:**
  - Self-discovering: No manual sensor orientation analysis
  - Manufacturing-agnostic: Works regardless of PCB assembly variations
  - Version-controlled: Config values committed to git, survive flash erases
  - One-time: Only needed during hardware bring-up

### January 13, 2026 - IMU Calibration Tool LUT Rotation Direction Fix
- **Critical bug discovered in calibration tool LUT calculation:**
  - Display rotation LUTs (e.g., `LUT_ROTATE_90`) rotate pixel content in the **same** direction as their name
  - When device physically rotates clockwise, content must rotate **counter-clockwise** to stay upright
  - Original formula produced direct mapping (Physical 90° → LUT_90) which caused upside-down display at 90°/270°
  - Required manual correction: swapping 90°↔270° in output before use
- **Root cause identified:**
  - Tool was calculating: `LUT[rotation] = (rotation - deviceHome + displayOffset) % 360`
  - This assumes LUTs rotate content opposite to their name (they don't)
  - Git history showed commit 2ff4679 had wrong comment: "AtomMatrix has Z+ toward display" (actually Z+ away)
  - Previous direct mapping worked by accident only for specific Z-axis orientations
- **Solution implemented:**
  - Fixed formula to invert rotation direction: `LUT[rotation] = (360 - rotation + displayOffset + deviceHome) % 360`
  - The `360 - rotation` term inverts rotation so content rotates counter to device
  - Example: Physical 90° CW → `(360 - 90 + 0 + 0) % 360 = 270` → LUT_270 (90° CCW)
  - Now produces correct output matching manually-validated working configurations
- **Files modified:**
  - `src/main_calibrate.cpp`: Fixed LUT calculation formula with rotation inversion
  - `src/main_calibrate.cpp`: Added detailed header comment explaining rotation inversion logic
  - `src/main_calibrate.cpp`: Added runtime note in output explaining LUT inversion
  - `Documentation/Developer/IMU_Calibration_Methodology.md`: Updated formula with critical insight section
- **Testing status:**
  - Formula mathematically validated against working ATOM Matrix configuration
  - Calibration tool built and uploaded successfully
  - Awaiting full end-to-end testing (scheduled for tomorrow)
- **Expected outcome:**
  - Tool will now produce directly copy/paste-able configuration
  - No manual LUT swapping required
  - Same calibration methodology works for all devices regardless of Z-axis direction

### January 13, 2026 - IMU Calibration Tool Complete - Production Ready ✅
- **Major achievement: Calibration tool produces directly usable copy/paste configuration**
  - Both ATOM Matrix and Waveshare ESP32-S3-Matrix fully validated end-to-end
  - No manual adjustments needed - output works immediately in main application
  - Physical orientation logging matches actual device position (0°, 90°, 180°, 270°, FLAT)
  
- **Root cause discovered: LUT indexing paradigm mismatch**
  - **Original (wrong) assumption:** `DEVICE_ORIENTATION_TO_LUT_MAP` indexed by physical rotation angles
    - Array index 0 = physical 0°, index 1 = physical 90°, etc.
  - **Actual runtime behavior:** Indexed by `Orientation` enum values from `getOrientation()`
    - Array index = enum value returned by accelerometer logic, NOT physical angle
  - **Why it matters:** For boards with axis remapping, enum values ≠ physical angles
    - Example: Waveshare at physical 0° (USB DOWN) → boardX=-1g → `getOrientation()` returns `ROTATE_270` (enum 3)
    - Runtime looks up `LUT[3]`, so LUT[3] must contain the rotation for physical 0°, not physical 270°

- **Calibration tool comprehensive fix:**
  - **Simulates `getOrientation()` during calibration** using identical threshold logic (0.5g, 0.7g)
  - **Builds enum mapping:** For each measurement, determines which `Orientation` enum `getOrientation()` will return
  - **Constructs LUT by enum index:** `lut[enumValue] = rotation` instead of `lut[physicalPosition] = rotation`
  - **Generates reverse mapping:** `ORIENTATION_ENUM_TO_PHYSICAL_ANGLE` for debug logging
  - **LUT rotation formula:** `(360 - physicalAngle + displayOffset + deviceHome*90) % 360` (inverted for CCW rotation)

- **Code changes implemented:**
  - `src/main_calibrate.cpp` (lines 495-530): Added `orientationEnums[4]` calculation simulating `getOrientation()`
  - `src/main_calibrate.cpp` (lines 547-615): Rewrote `printConfiguration()` to build enum-indexed LUT
  - `src/main_calibrate.cpp`: Added reverse mapping output for physical angle debugging
  - `src/Application/STACApp.cpp` (lines 190-205): Use reverse mapping to log actual physical orientation
  - `include/BoardConfigs/AtomMatrix_Config.h`: Added `ORIENTATION_ENUM_TO_PHYSICAL_ANGLE` (enums match angles)
  - `include/BoardConfigs/WaveshareS3_Config.h`: Added `ORIENTATION_ENUM_TO_PHYSICAL_ANGLE` (enums offset from angles)

- **Validation results - ATOM Matrix:**
  - Calibration output: enum 0→physical 0°, enum 1→physical 90°, enum 2→physical 180°, enum 3→physical 270°
  - All vertical orientations display correctly upright
  - Logs show correct physical angles: "Physical device orientation: 0°" when USB DOWN
  - FLAT orientation: baseline aligned to home position (USB DOWN)

- **Validation results - Waveshare ESP32-S3-Matrix:**
  - Calibration output: enum 0→physical 90°, enum 1→physical 180°, enum 2→physical 270°, enum 3→physical 0°
  - All vertical orientations display correctly upright  
  - Logs show correct physical angles: "Physical device orientation: 0°" when USB DOWN
  - FLAT orientation: baseline aligned to home position (USB DOWN)
  - **Confirms:** Enum values differ from physical angles due to axis remap, but LUT mapping is now correct

- **Technical insights gained:**
  - LUT is runtime-indexed: `displayRotation = lutMap[detectedOrientation]` where `detectedOrientation` is enum
  - `getOrientation()` returns enum based on which accelerometer axis is dominant (>0.7g threshold)
  - Different axis remappings → different enum-to-physical mappings → different LUT contents (but same visual result)
  - Display offset: Determines relationship between pixel 0 corner and home position
  - Home position: Arbitrary choice - any vertical orientation can serve as reference frame
  - Copy/paste now works perfectly: No understanding of LUT internals required by user

- **Production status:**
  - ✅ Calibration tool validated on 2 platforms with different IMU sensors and axis remappings
  - ✅ Output works immediately without manual corrections
  - ✅ Debug logging shows meaningful physical orientations
  - ✅ Tool is generic - works regardless of home position choice
  - ✅ Ready for use on additional hardware platforms

### January 12, 2026 - IMU Calibration Tool - LUT Rotation Direction Fixed
- **Version bump:** RC.22 → RC.23
- **Enhanced build_all.sh script:**
  - Added ATOM Matrix to release build sequence
  - Added `-t clean` flag to all environments for guaranteed clean builds
  - Ensures no stale artifacts from previous builds
- **Fixed merged binary generation:**
  - Corrected bootloader offset in `custom_targets.py`
  - Changed from 0x1000 to 0x0 (esptool pads to 0x1000 automatically)
  - Prevents 0xFF padding that corrupts ESP32 image header
  - All `_FULL.bin` files now have valid 0xE9 magic byte
- **IMU improvements for Waveshare QMI8658:**
  - Added 100ms settling delay after IMU initialization
  - Clarified debug message: "Initial orientation" → "Display orientation"
  - Fixed FLAT orientation handling: Returns RIGHT raw value (becomes DOWN after OFFSET_90)
  - Matches vertical USB-down home position when board is horizontal

### January 10, 2026 - Waveshare IMU Orientation Fix & Startup LED Enhancements
- **Fixed QMI8658 axis remapping** on Waveshare ESP32-S3-Matrix
  - Root cause: Incorrect sign mapping in axis transformation
  - Physical sensor orientation: +X=UP (away from USB), +Y=LEFT (rear view)
  - Correct mapping: Board X (RIGHT) = -Sensor Y, Board Y (UP) = Sensor X
  - With OFFSET_90, all edge orientations now display correctly
- **Improved startup LED feedback sequence:**
  - Orange POG displays immediately after `display->begin()` 
  - Green POG shows for 750ms at end of successful setup
  - Red POG shows on system halt for error indication
  - Removed early LED clearing (caused RMT channel conflict)
- **Cleaned up debug logging:**
  - Converted startup messages to `log_i()` (debug-only, suppressed in release builds)
  - Removed non-existent `Logger.h` include, uses Arduino ESP32 built-in logging

### November 29, 2025 - Provisioning Mode Color Fix
- Button held at boot: ORANGE if provisioned, RED if not
- Pulsing display during provisioning now uses correct color
- Not provisioned: skips factory reset state (already at defaults)
- Matches baseline v2.x behavior

### November 28, 2025 - TFT Support Merged to v3_RC
- Added M5StickC Plus and LilyGO T-Display support
- TFT display via LovyanGFX with sprite buffering
- AXP192 PMU driver for backlight control
- Button B (reset) support for TFT boards
- GLF_FR glyph (factory reset icon)
- Fixed GPIO pullup error on input-only pins

### November 28, 2025 - STS Emulator v1.2.0
- Added per-STAC sequential state mode
- Each STAC gets unique state (first 3 guaranteed different)
- States are static by default (optional cycling)
- Fixed duplicate class definition bug

### Key Documentation
- **TFT Details:** `Documentation/Developer/TFT_Display_Support_Context.md`
- **Full History:** `Documentation/Developer/Detailed Change Log.md`

---

## Known Issues

### Expected Warnings
- `RMT channel not initialized. Call initRMT first` - Ignore (LiteLED RMT DMA not available on ESP32-PICO)

### GPIO Notes
- GPIO 39 (ATOM Matrix, M5StickC Plus) is input-only, no internal pullup
- Use `BUTTON_NEEDS_EXTERNAL_PULLUP` flag in board config
