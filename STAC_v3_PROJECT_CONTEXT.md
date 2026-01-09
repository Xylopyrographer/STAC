# STAC v3 Project Context

**Version:** v3.0.0-RC.14  
**Branch:** `v3-unified-portal`  
**Updated:** January 8, 2026  
**Status:** Unified Web Portal - Cross-Platform Validated (iOS, macOS, Android, Windows, Linux)

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
- `WebPortalServer` → Unified captive portal with Setup/Maintenance tabs
- `DNSServer` → Captive portal DNS redirect

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
