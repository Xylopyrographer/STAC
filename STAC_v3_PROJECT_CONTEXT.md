# STAC v3 Project Context

**Version:** v3.0.0-RC.9  
**Branch:** `v3_RC`  
**Updated:** December 1, 2025  
**Status:** TFT Display Startup Artifact Fix, Multi-Board TFT Support

---

## Quick Reference

### Supported Hardware

| Board | Display | IMU | MCU | Status |
|-------|---------|-----|-----|--------|
| M5Stack ATOM Matrix | 5×5 LED | MPU6886 | ESP32-PICO-D4 | ✅ Tested |
| Waveshare ESP32-S3-Matrix | 8×8 LED | QMI8658 | ESP32-S3 | ✅ Tested |
| M5StickC Plus | 135×240 TFT | MPU6886 | ESP32-PICO-D4 | 🔄 In Development |
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
3. **Provisioning Mode** - Captive portal for WiFi/switch configuration
4. **OTA Mode** - Over-the-air firmware updates

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
- `WebConfigServer` → Captive portal
- `OTAUpdateServer` → Firmware updates

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

### Boot Button Sequence
| Hold Time | Action | Display |
|-----------|--------|---------|
| 0-2 sec | Provisioning | Gear icon |
| 2-4 sec | Factory Reset | Red flash |
| 4-6 sec | OTA Update | Down arrow |
| >6 sec | Normal boot | Channel digit |

---

## Recent Changes

### December 1, 2025 - TFT Display Startup Artifact Fix
- Fixed display artifacts (stale pixels) showing during soft reset on TFT displays
- **M5StickC Plus (ST7789):** Disabled LDO2 (backlight) in AXP192 init - let display code turn it on
- **AIPI-Lite (ST7735S):** Hardware LCD reset + direct write to full 132×162 driver memory via `setWindow()`
- Common fixes for all TFT boards:
  - Early backlight OFF in `main.cpp` before any other init
  - Backlight OFF before all `ESP.restart()` calls
  - Removed LGFX Light_PWM backlight management - control directly via LEDC/analogWrite or PMU
  - Added `PreRestartCallback` to WebConfigServer and OTAUpdateServer
- Key insight: ST7735S has 132×162 memory but only 128×128 visible; hidden "offset" regions retain stale data through soft reset
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
