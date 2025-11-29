# TFT Display Support Context

**Branch:** `feature/tft-display-support`  
**Created:** November 27, 2025  
**Updated:** November 28, 2025  
**Status:** In Development - LED Matrix Tested ✅

## Overview

This branch adds TFT display support to STAC v3, extending the existing LED matrix display system with full-color TFT implementations using the LovyanGFX library. The architecture uses a **unified configuration approach** where all TFT-specific settings are defined in board configuration files.

## Recent Changes (November 28, 2025)

### Glyph System Updates
- **Added GLF_FR glyph** - New factory reset icon (top/bottom bars with X in center)
- **Marked unused glyphs as reserved** - `GLF_RA`, `GLF_LA`, `GLF_HF` (5x5/TFT) and `GLF_SF`, `GLF_IMUX` (8x8)
- **Factory reset display** - Changed from yellow-on-red to red-on-black for clarity

### Button Configuration Fix
- **Fixed GPIO pullup error** - Input-only GPIOs (like GPIO 39 on ATOM Matrix) were incorrectly requesting internal pullup
- **Added `NEEDS_EXTERNAL_PULLUP` to Config::Button** - XP_Button now correctly uses INPUT mode for input-only pins
- **No more `E (xxxx) gpio: gpio_pullup_en` errors** on boot

### Build System Updates
- **Fixed waveshare-s3 build filter** - Added missing TFT and AXP192 file exclusions
- **All LED matrix builds verified** - atom-matrix, waveshare-s3 compile and run correctly

## Supported Hardware

### M5StickC Plus
- **MCU:** ESP32-PICO-D4 (same as ATOM Matrix)
- **Display:** ST7789V2 TFT, 135×240 pixels, 16-bit color (RGB565)
- **PMU:** AXP192 (controls power rails and backlight)
- **IMU:** MPU6886 (same as ATOM Matrix)
- **Primary Button:** GPIO 37 (active low)
- **Reset Button:** GPIO 39 (active low, side button)
- **LED:** GPIO 10 (active low, used for debug)
- **I2C:** GPIO 21 (SDA), GPIO 22 (SCL)
- **Backlight Control:** Via AXP192 LDO2

| TFT Signal | GPIO |
|------------|------|
| MOSI       | 15   |
| CLK        | 13   |
| CS         | 5    |
| DC         | 23   |
| RST        | 18   |

### LilyGo T-Display (TTGO T-Display)
- **MCU:** ESP32 (ESP32-D0WDQ6)
- **Display:** ST7789V TFT, 135×240 pixels, 16-bit color (RGB565)
- **PMU:** None (direct GPIO control)
- **IMU:** None (external IMU required if orientation detection needed)
- **Primary Button:** GPIO 35 (input-only, needs external pullup, active low)
- **Boot Button:** GPIO 0 (for boot mode selection, not used as software input)
- **Hardware Reset:** Dedicated EN button (physical reset, not software-triggered)
- **Backlight Control:** GPIO 4 (PWM via LGFX Light_PWM)

| TFT Signal | GPIO |
|------------|------|
| MOSI       | 19   |
| CLK        | 18   |
| CS         | 5    |
| DC         | 16   |
| RST        | 23   |
| BL         | 4    |

**Note:** The T-Display has a dedicated hardware reset button tied to the ESP32 EN pin, so no software reset button (Button B) is needed.

## Files Created/Modified

### New Files

#### `include/BoardConfigs/M5StickCPlus_Config.h`
Board configuration with:
- `DISPLAY_TYPE_TFT` and `USE_AXP192_PMU` defines
- TFT pin assignments (`TFT_SCLK`, `TFT_MOSI`, `TFT_CS`, `TFT_DC`, `TFT_RST`)
- `TFT_PANEL_ST7789` panel type selection
- `DISPLAY_BACKLIGHT_PMU` for AXP192-controlled backlight
- Brightness map optimized for TFT: `{ 0, 200, 210, 220, 230, 240, 250, 255, 255 }`
- Button B (GPIO 39) for software reset

#### `include/BoardConfigs/LilygoTDisplay_Config.h`
Board configuration with:
- `DISPLAY_TYPE_TFT` define (no PMU)
- TFT pin assignments including `TFT_BL` (GPIO 4) for backlight
- `TFT_PANEL_ST7789` panel type selection
- `DISPLAY_BACKLIGHT_PWM` for GPIO PWM backlight control
- Brightness map optimized for TFT: `{ 0, 200, 210, 220, 230, 240, 250, 255, 255 }`
- No Button B (hardware reset via EN button)

#### `include/Hardware/Display/TFT/DisplayTFT.h`
TFT display class header implementing `IDisplay` interface.

#### `src/Hardware/Display/TFT/DisplayTFT.cpp`
Full TFT display implementation with:
- LovyanGFX sprite-based double buffering
- Conditional AXP192 PMU integration (when `USE_AXP192_PMU` defined)
- PWM backlight fallback (via LGFX Light_PWM)
- Vector graphics rendering for all glyphs
- Font rendering using FreeSansBold24pt7b

#### `include/Hardware/Display/TFT/LGFX_STAC.h`
**Unified** LovyanGFX display configuration class that:
- Uses `TFT_*` defines from board config for all pin assignments
- Selects panel driver via `TFT_PANEL_*` defines
- Configures backlight via `DISPLAY_BACKLIGHT_PWM` or `DISPLAY_BACKLIGHT_PMU`
- Supports all common TFT panels: ST7789, ST7735, ILI9341, ILI9342, ILI9163, GC9A01, ST7796, ILI9481/86/88, R61529, HX8357D

#### `include/Hardware/Display/TFT/GlyphsTFT.h`
Glyph index definitions compatible with LED matrix system:
- 34 glyph indices (GLF_0 through GLF_CORNERS)
- Stub glyph data arrays for GlyphManager compatibility

#### `src/Hardware/Power/AXP192.cpp` and `include/Hardware/Power/AXP192.h`
AXP192 PMU driver for M5StickC Plus:
- Power rail control (LDO2 for backlight, LDO3 for LCD)
- Backlight brightness control (0-255)
- Development mode: Battery power output disabled for USB-only operation

### Deprecated Files (can be removed)

#### `include/Hardware/Display/TFT/LGFX_M5StickCPlus.h`
Per-board LGFX config, superseded by unified `LGFX_STAC.h`.

#### `include/Hardware/Display/TFT/LGFX_LilygoTDisplay.h`
Per-board LGFX config, superseded by unified `LGFX_STAC.h`.

### Modified Files

#### `platformio.ini`
Added environments with LovyanGFX library dependency:
- `m5stickc-plus` - M5StickC Plus with AXP192 PMU
- `lilygo-t-display` - LilyGo T-Display with PWM backlight

#### `include/Device_Config.h`
Added board detection for `BOARD_LILYGO_T_DISPLAY`.

#### `src/main.cpp`
Added debug LED blinks for boot tracing (M5StickC Plus specific).

## Implementation Details

### Unified TFT Architecture

All TFT configuration is defined in board config files (`*_Config.h`), with a single unified LGFX configuration class:

```
Board Config (M5StickCPlus_Config.h, LilygoTDisplay_Config.h)
    │
    ├── TFT Pin Defines (TFT_SCLK, TFT_MOSI, TFT_CS, TFT_DC, TFT_RST, TFT_BL)
    ├── Panel Type (TFT_PANEL_ST7789, TFT_PANEL_ILI9341, etc.)
    ├── Backlight Control (DISPLAY_BACKLIGHT_PWM or DISPLAY_BACKLIGHT_PMU)
    └── Display Dimensions (TFT_WIDTH, TFT_HEIGHT, TFT_OFFSET_X, TFT_OFFSET_Y)
            │
            ▼
LGFX_STAC (unified configuration class)
    │
    └── Uses preprocessor defines to configure:
        - Panel driver selection
        - Pin assignments
        - Backlight control method
        - Display geometry
            │
            ▼
DisplayTFT (display implementation)
    │
    ├── Uses LGFX_STAC for display
    ├── Conditional AXP192 PMU (when USE_AXP192_PMU defined)
    └── Falls back to LGFX backlight control otherwise
```

### Supported TFT Panels

| Define | Panel Driver | Common Displays |
|--------|-------------|-----------------|
| `TFT_PANEL_ST7789` | ST7789 | M5StickC Plus, T-Display |
| `TFT_PANEL_ST7735` | ST7735 | Many small TFTs |
| `TFT_PANEL_ILI9341` | ILI9341 | 2.4" TFT modules |
| `TFT_PANEL_ILI9342` | ILI9342 | M5Stack Core |
| `TFT_PANEL_ILI9163` | ILI9163 | 1.44" TFT modules |
| `TFT_PANEL_GC9A01` | GC9A01 | Round TFT displays |
| `TFT_PANEL_ST7796` | ST7796 | 4" TFT modules |

### Backlight Control Methods

| Method | Define | Implementation | Use Case |
|--------|--------|----------------|----------|
| PMU | `DISPLAY_BACKLIGHT_PMU` | AXP192 LDO2 voltage | M5StickC Plus |
| PWM | `DISPLAY_BACKLIGHT_PWM` | LGFX Light_PWM on `TFT_BL` pin | T-Display, most modules |
| None | Neither defined | No backlight control | Always-on displays |

### Display Architecture

```
IDisplay (interface)
    ├── DisplayLED (5x5/8x8 LED matrix)
    └── DisplayTFT (135x240 TFT)
            └── Uses LovyanGFX + AXP192
```

### Glyph Rendering Strategy

Unlike LED matrices which use bitmap data, the TFT display uses vector graphics:

1. **GlyphManager** returns pointers to stub glyph data (1-byte arrays containing glyph index)
2. **DisplayTFT::drawGlyph()** extracts the index and renders using graphics primitives
3. All glyphs are drawn centered on the 135×240 display

### Font Usage

| Content | Font | Size |
|---------|------|------|
| Channel digits (0-9) | FreeSansBold24pt7b | 2x |
| Letters (C, T, A, S, P) | FreeSansBold24pt7b | 2x |
| Question mark | FreeSansBold24pt7b | 3x |
| "ST" logo | Font4 | 3x |

### Icon Implementations

| Glyph | Icon | Size/Details |
|-------|------|--------------|
| GLF_WIFI | WiFi arcs | 3 arcs with base dot, thick strokes |
| GLF_CFG | Gear | 50px radius, 8 teeth |
| GLF_UD | Download arrow | 60×70px, thick stem and base |
| GLF_CK | Checkmark | 50px, 12px thick lines |
| GLF_BX/GLF_X | X mark | 45px, 12px thick lines |
| GLF_QM | Question mark | Large font + bottom dot |
| GLF_FM/GLF_DF | Frame | 8px thick border |
| GLF_CBD | Checkerboard | 20px blocks, red/green |
| GLF_CORNERS | Corner squares | 10px squares at corners |
| GLF_RA/GLF_LA | Arrows | Filled triangles |
| GLF_HF | Happy face | Circle with eyes and smile arc |

### Brightness Control

Brightness is controlled via AXP192 LDO2 voltage (backlight power):
- Range: 0-255 maps to LDO2 voltage
- BRIGHTNESS_MAP in config provides 8 user-selectable levels
- TFT needs higher values than LED (200-255 range vs 0-80)

### Development Mode

For easier debugging, the AXP192 driver includes a "dev mode" that:
- Disables battery power output (REG 0x30, 0x32)
- Device only runs on USB power
- Prevents battery from keeping crashed device alive

## Current Status

### Working ✅
- [x] AXP192 PMU initialization and backlight control
- [x] TFT display initialization via LovyanGFX
- [x] All startup config screens (channel, C/T, A/S, brightness)
- [x] Brightness level cycling with digit update
- [x] WiFi connection display (orange/green WiFi icons)
- [x] Tally polling and state display (PREVIEW/PROGRAM/UNSELECTED)
- [x] Autostart corner flashing
- [x] Provisioning mode gear icon
- [x] OTA update download arrow icon
- [x] Error X icon
- [x] Question mark icon
- [x] Checkmark confirmation icon

### To Test 🔄
- [ ] Provisioning mode full flow
- [ ] Factory reset display sequence (TFT)
- [ ] OTA update mode full flow
- [ ] Error condition displays (question mark, big X)
- [ ] Display rotation with IMU orientation changes

### Verified 🧪
- [x] ATOM Matrix - All features working, GPIO pullup fix verified
- [x] Waveshare ESP32-S3 Matrix - Builds successfully  
- [x] M5StickC Plus - Builds successfully
- [x] LilyGO T-Display - Builds successfully

### Remaining Work 📋
- [ ] Clean up debug code (LED blinks in main.cpp, log statements)
- [ ] Test all error condition displays
- [ ] Verify peripheral mode display (TFT)
- [ ] Consider adding status text labels (optional enhancement)

## Key Learnings

1. **Font7 is digits-only** - Use FreeSansBold for letters
2. **setBrightness must call show()** - Fixed bug where brightness changes weren't displayed
3. **I2C must be initialized before AXP192** - Added Wire.begin() in AXP192::begin()
4. **TFT needs higher brightness values** - LED matrix uses 0-80, TFT needs 200-255
5. **Sprite double-buffering prevents flicker** - All drawing to sprite, then pushSprite()

## Testing Checklist

### Startup Sequence
- [ ] Channel number displays correctly (FreeSansBold font)
- [ ] C/T mode letters display correctly
- [ ] A/S autostart letters display correctly
- [ ] Brightness checkerboard with cycling digit
- [ ] Autostart corner flashing

### Boot Button Sequence
- [ ] 0-2 sec: Orange gear (provisioning pending)
- [ ] Release: Red pulsing gear (provisioning mode)
- [ ] 2-4 sec: Red frame + green checkmark (factory reset pending)
- [ ] 4-6 sec: Red download arrow (OTA update pending)

### Normal Operation
- [ ] PREVIEW state: Green fill
- [ ] PROGRAM state: Red fill
- [ ] UNSELECTED (Camera mode): Purple dotted frame
- [ ] UNSELECTED (Talent mode): Green fill
- [ ] WiFi reconnection handling

### Error Conditions
- [ ] Question mark (junk STS reply)
- [ ] Big X (no connection)

## Dependencies

- **LovyanGFX** v1.2.7 - TFT display driver
- **Wire** - I2C for PMU and IMU
- **Existing STAC libraries** - Button, IMU, WiFi, etc.

## Build Commands

```bash
# Build M5StickC Plus
pio run -e m5stickc-plus

# Build LilyGo T-Display
pio run -e lilygo-t-display

# Build both TFT targets
pio run -e m5stickc-plus -e lilygo-t-display

# Build and upload M5StickC Plus
pio run -e m5stickc-plus -t upload

# Build and upload LilyGo T-Display
pio run -e lilygo-t-display -t upload

# Monitor serial output
pio device monitor -e m5stickc-plus
pio device monitor -e lilygo-t-display
```

## Notes

- The main project context document (`STAC_v3_PROJECT_CONTEXT.md`) should be kept as the primary reference for the overall STAC v3 project
- This document focuses specifically on the TFT display branch work
- Once merged to main, relevant sections should be integrated into the main context document
