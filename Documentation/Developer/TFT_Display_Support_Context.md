# TFT Display Support Context

**Branch:** `feature/tft-display-support`  
**Created:** November 27, 2025  
**Status:** In Development

## Overview

This branch adds TFT display support for the M5StickC Plus to STAC v3, extending the existing LED matrix display system with a full-color TFT implementation using the LovyanGFX library.

## Hardware Target

### M5StickC Plus Specifications
- **MCU:** ESP32-PICO-D4 (same as ATOM Matrix)
- **Display:** ST7789V2 TFT, 135×240 pixels, 16-bit color (RGB565)
- **PMU:** AXP192 (controls power rails and backlight)
- **IMU:** MPU6886 (same as ATOM Matrix)
- **Button:** GPIO 37 (active low)
- **LED:** GPIO 10 (active low, used for debug)
- **I2C:** GPIO 21 (SDA), GPIO 22 (SCL)

### Display Interface (SPI)
| Signal | GPIO |
|--------|------|
| MOSI   | 15   |
| CLK    | 13   |
| CS     | 5    |
| DC     | 23   |
| RST    | 18   |

## Files Created/Modified

### New Files

#### `include/BoardConfigs/M5StickCPlus_Config.h`
Board configuration with:
- `DISPLAY_TYPE_TFT` define
- Pin assignments for TFT display
- PMU configuration (AXP192)
- Brightness map optimized for TFT: `{ 0, 200, 210, 220, 230, 240, 250, 255, 255 }`

#### `include/Hardware/Display/TFT/DisplayTFT.h`
TFT display class header implementing `IDisplay` interface.

#### `src/Hardware/Display/TFT/DisplayTFT.cpp`
Full TFT display implementation with:
- LovyanGFX sprite-based double buffering
- AXP192 PMU integration for backlight control
- Vector graphics rendering for all glyphs
- Font rendering using FreeSansBold24pt7b

#### `include/Hardware/Display/TFT/LGFX_M5StickCPlus.h`
LovyanGFX display configuration class for ST7789V2.

#### `include/Hardware/Display/TFT/GlyphsTFT.h`
Glyph index definitions compatible with LED matrix system:
- 34 glyph indices (GLF_0 through GLF_CORNERS)
- Stub glyph data arrays for GlyphManager compatibility

#### `src/Hardware/Power/AXP192.cpp` and `include/Hardware/Power/AXP192.h`
AXP192 PMU driver for M5StickC Plus:
- Power rail control (LDO2 for backlight, LDO3 for LCD)
- Backlight brightness control (0-255)
- Development mode: Battery power output disabled for USB-only operation

### Modified Files

#### `platformio.ini`
Added `m5stickc-plus` environment with:
- LovyanGFX library dependency
- Build flags for TFT display type

#### `src/main.cpp`
Added debug LED blinks for boot tracing (M5StickC Plus specific).

## Implementation Details

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
- [ ] Factory reset display sequence
- [ ] OTA update mode full flow
- [ ] Error condition displays (question mark, big X)
- [ ] Display rotation with IMU orientation changes

### Remaining Work 📋
- [ ] Clean up debug code (LED blinks in main.cpp, log statements)
- [ ] Test all error condition displays
- [ ] Verify peripheral mode display
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
# Build only
pio run -e m5stickc-plus

# Build and upload
pio run -e m5stickc-plus -t upload

# Monitor serial output
pio device monitor -e m5stickc-plus
```

## Notes

- The main project context document (`STAC_v3_PROJECT_CONTEXT.md`) should be kept as the primary reference for the overall STAC v3 project
- This document focuses specifically on the TFT display branch work
- Once merged to main, relevant sections should be integrated into the main context document
