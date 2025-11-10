# STAC Hardware Configuration Guide

This guide explains how to configure STAC for different hardware platforms and how to add support for custom hardware.

## Table of Contents

- [Overview](#overview)
- [Supported Boards](#supported-boards)
- [Configuration System](#configuration-system)
- [Board-Specific Settings](#board-specific-settings)
- [Adding Custom Hardware](#adding-custom-hardware)
- [Pin Reference](#pin-reference)
- [Troubleshooting](#troubleshooting)

---

## Overview

STAC uses a compile-time configuration system based on `Device_Config.h`. This approach:

- ✅ **Zero runtime overhead** - All decisions made at compile time
- ✅ **Type-safe** - Compiler catches configuration errors
- ✅ **Easy to use** - Edit one file to change boards
- ✅ **Extensible** - Add new boards without modifying core code

## Supported Boards

### M5Stack ATOM Matrix

**Specifications:**
- **Display:** 5×5 RGB LED matrix (WS2812, GRB color order, serpentine wiring)
- **IMU:** MPU6886 (6-axis accelerometer/gyroscope)
- **Button:** GPIO 39 (active low, input-only pin - needs external pullup)
- **Processor:** ESP32-PICO-D4
- **Flash:** 4MB
- **Pins Available:** Limited (many used by display/IMU)

**Configuration:** `BoardConfigs/AtomMatrix_Config.h`

**Purchase:** [M5Stack Store](https://shop.m5stack.com/products/atom-matrix-esp32-development-kit)

---

### Waveshare ESP32-S3-Matrix

**Specifications:**
- **Display:** 8×8 RGB LED matrix (WS2812, GRB color order, serpentine wiring)
- **IMU:** QMI8658 (6-axis accelerometer/gyroscope)
- **Button:** GPIO 7 (active low, internal pullup available)
- **Processor:** ESP32-S3-WROOM-1
- **Flash:** 4MB
- **PSRAM:** 2MB
- **Pins Available:** GROVE connector exposes I2C and GPIO

**Configuration:** `BoardConfigs/WaveshareS3_Config.h`

**Purchase:** [Waveshare Wiki](https://www.waveshare.com/wiki/ESP32-S3-Matrix)

---

## Configuration System

### Step 1: Select Your Board

Edit `pioarduino/include/Device_Config.h`:
```cpp
// Uncomment ONE board definition
#define BOARD_M5STACK_ATOM_MATRIX
// #define BOARD_WAVESHARE_ESP32_S3_MATRIX
// #define BOARD_CUSTOM_5X5
// #define BOARD_CUSTOM_8X8
```

### Step 2: Compile

The build system automatically includes the correct board configuration:
```bash
# PlatformIO
pio run -e atom-matrix

# Arduino IDE
# Just select your board and compile
```

### Step 3: Upload
```bash
pio run -e atom-matrix -t upload
```

That's it! The correct drivers, pin assignments, and settings are automatically selected.

---

## Board-Specific Settings

Each board configuration file (`BoardConfigs/XYZ_Config.h`) defines:

### Display Configuration
```cpp
#define DISPLAY_MATRIX_WIDTH 5          // Matrix width (5 or 8)
#define DISPLAY_MATRIX_HEIGHT 5         // Matrix height (5 or 8)
#define DISPLAY_LED_TYPE LED_STRIP_WS2812  // LED type
#define DISPLAY_LED_IS_RGBW false       // RGB or RGBW LEDs
#define DISPLAY_COLOR_ORDER_RGB         // RGB or GRB order
#define DISPLAY_WIRING_SERPENTINE       // Serpentine or row-by-row
#define PIN_DISPLAY_DATA 27             // Data pin
#define DISPLAY_BRIGHTNESS_MAX 60       // Max brightness (heat limit)
```

**Display Wiring Patterns:**

**Serpentine** (zigzag):
```
→ → → → →
        ↓
← ← ← ← ←
↓
→ → → → →
```

**Row-by-row** (all left-to-right):
```
→ → → → →
→ → → → →
→ → → → →
```

### IMU Configuration
```cpp
#define IMU_TYPE_MPU6886               // IMU chip type
#define IMU_HAS_IMU true               // Board has IMU
#define PIN_IMU_SCL 21                 // I2C clock pin
#define PIN_IMU_SDA 25                 // I2C data pin
#define IMU_I2C_CLOCK 100000L          // I2C speed (100kHz)
#define IMU_ORIENTATION_OFFSET 3       // Rotation offset (0-3)
```

**IMU Orientation Offsets:**

The IMU may be mounted rotated relative to your expected "UP" orientation. Use the offset to compensate:

- `0` = No rotation (0°)
- `1` = 90° clockwise
- `2` = 180°
- `3` = 270° clockwise (or 90° counter-clockwise)

**How to determine correct offset:**
1. Hold device with USB port at bottom (your expected "UP")
2. Note what orientation is reported
3. Adjust offset until "UP" is correct

### Button Configuration
```cpp
#define PIN_BUTTON 39                  // Button GPIO pin
#define BUTTON_DEBOUNCE_MS 25          // Debounce time
#define BUTTON_ACTIVE_LOW true         // Active low/high
#define BUTTON_NEEDS_EXTERNAL_PULLUP true  // For input-only pins
```

**Note on GPIO 39 (ATOM Matrix):**
- GPIO 39 is input-only on ESP32 (no internal pullup/pulldown)
- ATOM Matrix has external pullup resistor
- Must set `BUTTON_NEEDS_EXTERNAL_PULLUP true`

### Interface Pins
```cpp
// Peripheral mode detection
#define PIN_PM_CHECK_OUT 3             // Output toggle pin
#define PIN_PM_CHECK_IN 4              // Input sense pin
#define PM_CHECK_TOGGLE_COUNT 5        // Test cycles

// GROVE/Tally pins
#define PIN_TALLY_STATUS_0 32          // Tally bit 0
#define PIN_TALLY_STATUS_1 26          // Tally bit 1
```

**Peripheral Mode Detection:**
- Install jumper between `PM_CHECK_OUT` and `PM_CHECK_IN`
- Device toggles output and checks if input follows
- Multiple tests confirm solid connection

**Tally Encoding** (2-bit binary):
| TS_1 | TS_0 | State |
|------|------|-------|
| LOW  | LOW  | NO_TALLY |
| LOW  | HIGH | UNSELECTED |
| HIGH | LOW  | PREVIEW |
| HIGH | HIGH | PROGRAM |

### Timing Constants
```cpp
#define TIMING_BUTTON_SELECT_MS 1500   // Long press threshold
#define TIMING_WIFI_CONNECT_TIMEOUT_MS 60000  // WiFi timeout
#define TIMING_PM_POLL_INTERVAL_MS 2   // Peripheral mode poll rate
```

### Glyph Configuration
```cpp
#define GLYPH_SIZE_5X5                 // 5×5 or 8×8 glyphs
#define GLYPH_FORMAT_UNPACKED          // Unpacked or bit-packed
```

**Glyph Formats:**
- **5×5 Unpacked:** 25 bytes per glyph (1 byte = 1 pixel)
- **8×8 Bit-packed:** 8 bytes per glyph (8 bits = 8 pixels per row)

---

## Adding Custom Hardware

### Step 1: Create Board Configuration

Copy an existing config as a template:
```bash
cd pioarduino/include/BoardConfigs
cp Custom5x5_Config.h MyBoard_Config.h
```

### Step 2: Edit Configuration

Open `MyBoard_Config.h` and update all settings:
```cpp
#ifndef MY_BOARD_CONFIG_H
#define MY_BOARD_CONFIG_H

// Board identification
#define STAC_BOARD_NAME "My Custom Board"
#define STAC_ID_PREFIX "STAC_CUSTOM"

// Display Configuration
#define DISPLAY_TYPE_LED_MATRIX
#define DISPLAY_MATRIX_WIDTH 8
#define DISPLAY_MATRIX_HEIGHT 8
#define DISPLAY_TOTAL_LEDS (DISPLAY_MATRIX_WIDTH * DISPLAY_MATRIX_HEIGHT)

#define DISPLAY_LED_TYPE LED_STRIP_WS2812
#define DISPLAY_LED_IS_RGBW false
#define DISPLAY_COLOR_ORDER_RGB  // Test both RGB and GRB

#define DISPLAY_WIRING_SERPENTINE  // Or ROW_BY_ROW

#define PIN_DISPLAY_DATA 27  // Your data pin

#define DISPLAY_POWER_LED_PIXEL 32  // Center pixel for power indicator

#define DISPLAY_BRIGHTNESS_MIN 0
#define DISPLAY_BRIGHTNESS_MAX 60
#define DISPLAY_BRIGHTNESS_DEFAULT 20

// Button Configuration
#define PIN_BUTTON 39  // Your button pin
#define BUTTON_DEBOUNCE_MS 25
#define BUTTON_ACTIVE_LOW true

// IMU Configuration - Choose one or none
// #define IMU_TYPE_MPU6886
// #define IMU_TYPE_QMI8658
#define IMU_TYPE_NONE

#ifdef IMU_TYPE_NONE
    #define IMU_HAS_IMU false
#else
    #define IMU_HAS_IMU true
    #define PIN_IMU_SCL 21  // Your I2C pins
    #define PIN_IMU_SDA 25
    #define IMU_I2C_CLOCK 100000L
    #define IMU_ORIENTATION_OFFSET 0
#endif

// Interface Pins
#define PIN_PM_CHECK_OUT 22  // Your GPIO pins
#define PIN_PM_CHECK_IN 33
#define PM_CHECK_TOGGLE_COUNT 5

#define PIN_TALLY_STATUS_0 32
#define PIN_TALLY_STATUS_1 26

// Status LED (optional)
#define HAS_STATUS_LED false

// Timing Constants (use defaults from ATOM config)
#define TIMING_AUTOSTART_PULSE_MS 1000
#define TIMING_AUTOSTART_TIMEOUT_MS 20000
#define TIMING_GUI_PAUSE_MS 1500
#define TIMING_GUI_PAUSE_SHORT_MS 500
#define TIMING_BUTTON_SELECT_MS 1500
#define TIMING_WIFI_CONNECT_TIMEOUT_MS 60000
#define TIMING_ERROR_REPOLL_MS 50
#define TIMING_PM_POLL_INTERVAL_MS 2
#define TIMING_OP_MODE_TIMEOUT_MS 30000
#define TIMING_NEXT_STATE_MS 750

// Network Configuration
#define NETWORK_MAX_POLL_ERRORS 8

// NVS
#define NVS_NOM_PREFS_VERSION 4
#define NVS_PM_PREFS_VERSION 2

// Glyph Configuration
#define GLYPH_SIZE_8X8  // Match your display size
#define GLYPH_FORMAT_BITPACKED

#endif // MY_BOARD_CONFIG_H
```

### Step 3: Add to Device_Config.h

Edit `include/Device_Config.h`:
```cpp
// Add your board option
#define BOARD_M5STACK_ATOM_MATRIX
// #define BOARD_WAVESHARE_ESP32_S3_MATRIX
// #define BOARD_MY_CUSTOM_BOARD  // <-- Add this

// Add include case
#if defined(BOARD_M5STACK_ATOM_MATRIX)
    #include "BoardConfigs/AtomMatrix_Config.h"
#elif defined(BOARD_WAVESHARE_ESP32_S3_MATRIX)
    #include "BoardConfigs/WaveshareS3_Config.h"
#elif defined(BOARD_MY_CUSTOM_BOARD)
    #include "BoardConfigs/MyBoard_Config.h"  // <-- Add this
#else
    #error "No board selected!"
#endif
```

### Step 4: Add PlatformIO Environment (Optional)

Edit `platformio.ini`:
```ini
[env:my-custom-board]
extends = common_settings
platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.32/platform-espressif32.zip
board = esp32dev  # Or your specific board

build_flags =
    ${debug_level.build_flags}
    -DBOARD_MY_CUSTOM_BOARD

lib_deps = 
    ${core_libs.lib_deps}
    # Add your IMU library if needed
```

### Step 5: Test
```bash
pio run -e my-custom-board -t upload
pio device monitor
```

### Step 6: Adjust Settings

Common issues and fixes:

**Wrong colors?**
```cpp
// Try the other color order
#define DISPLAY_COLOR_ORDER_GRB  // Was RGB
```

**Display upside down?**
```cpp
// Adjust orientation offset
#define IMU_ORIENTATION_OFFSET 2  // 180° rotation
```

**Wrong pixel order?**
```cpp
// Try other wiring pattern
#define DISPLAY_WIRING_ROW_BY_ROW  // Was SERPENTINE
```

---

## Pin Reference

### M5Stack ATOM Matrix

| Function | GPIO | Notes |
|----------|------|-------|
| Display Data | 27 | WS2812 data |
| Button | 39 | Active low, input-only (needs external pullup) |
| IMU SCL | 21 | I2C clock |
| IMU SDA | 25 | I2C data |
| PM Check Out | 22 | Peripheral mode detection output |
| PM Check In | 33 | Peripheral mode detection input |
| Tally Status 0 | 32 | GROVE port / Tally bit 0 |
| Tally Status 1 | 26 | GROVE port / Tally bit 1 |

**Reserved Pins:**
- GPIO 0: Boot mode select (don't use)
- GPIO 2: Boot mode select (don't use)
- GPIO 12: Flash voltage (don't use)
- GPIO 15: Flash timing (don't use)

### Waveshare ESP32-S3-Matrix

| Function | GPIO | Notes |
|----------|------|-------|
| Display Data | 47 | WS2812 data |
| Button | 7 | Active low, internal pullup available |
| IMU SCL | 6 | I2C clock |
| IMU SDA | 5 | I2C data |
| PM Check Out | 3 | Peripheral mode detection output |
| PM Check In | 4 | Peripheral mode detection input |
| Tally Status 0 | 5 | GROVE port / Tally bit 0 (shared with IMU SDA) |
| Tally Status 1 | 6 | GROVE port / Tally bit 1 (shared with IMU SCL) |

**GROVE Connector:**
- Pin 1: GND
- Pin 2: VCC (5V or 3.3V selectable)
- Pin 3: GPIO 5 (SDA / TS_0)
- Pin 4: GPIO 6 (SCL / TS_1)

---

## Troubleshooting

### Compilation Errors

**Error:** `No board selected in Device_Config.h`
- **Fix:** Uncomment exactly ONE `#define BOARD_XXX` line in `Device_Config.h`

**Error:** `namespace STAC::Hardware has no member IDisplay`
- **Fix:** Check that all namespaces are correct in STACApp.h

**Error:** `GPIO number error (input-only pad has no internal PU)`
- **Fix:** Set `#define BUTTON_NEEDS_EXTERNAL_PULLUP true` for GPIO 34-39

### Display Issues

**Problem:** Display shows wrong colors (red appears green, etc.)
- **Fix:** Change `DISPLAY_COLOR_ORDER_RGB` to `DISPLAY_COLOR_ORDER_GRB` or vice versa

**Problem:** Display pattern is wrong (pixels in wrong locations)
- **Fix:** Try `DISPLAY_WIRING_ROW_BY_ROW` instead of `DISPLAY_WIRING_SERPENTINE`

**Problem:** Display too bright/dim
- **Fix:** Adjust `DISPLAY_BRIGHTNESS_DEFAULT` and `DISPLAY_BRIGHTNESS_MAX`

### IMU Issues

**Problem:** Orientation detection incorrect
- **Fix:** Adjust `IMU_ORIENTATION_OFFSET` (try 0, 1, 2, or 3)

**Problem:** IMU not detected
- **Fix:** Check I2C pins and `IMU_I2C_ADDRESS` (if applicable)
- **Fix:** Ensure IMU library is installed

### Button Issues

**Problem:** Button presses not detected
- **Fix:** Check `BUTTON_ACTIVE_LOW` setting matches your hardware
- **Fix:** Adjust `BUTTON_DEBOUNCE_MS` if getting multiple triggers

**Problem:** GPIO 39 pullup error (ATOM Matrix)
- **Fix:** Set `BUTTON_NEEDS_EXTERNAL_PULLUP true`

### Peripheral Mode Issues

**Problem:** Peripheral mode not detected with jumper installed
- **Fix:** Verify jumper connects `PM_CHECK_OUT` to `PM_CHECK_IN`
- **Fix:** Check pin numbers in board config
- **Fix:** Reset device after installing jumper

**Problem:** Tally not received in peripheral mode
- **Fix:** Check GROVE port wiring (pin-to-pin: TS_0 to TS_0, TS_1 to TS_1, GND to GND)
- **Fix:** Ensure one device in Normal mode, other in Peripheral mode

---

## Best Practices

### Power Considerations

- **Max Brightness:** Keep ≤ 60 to prevent overheating
- **USB Power:** Most USB ports provide 500mA (5V = 2.5W)
- **LED Power:** At full white, full brightness:
  - 5×5 matrix: ~1.5W (300mA @ 5V)
  - 8×8 matrix: ~3.8W (760mA @ 5V)
- **Recommendation:** Use powered USB hub for 8×8 at high brightness

### GPIO Selection

**Good practices:**
- Avoid GPIO 0, 2, 12, 15 (boot mode and flash)
- Avoid GPIO 6-11 on ESP32 (flash interface)
- Use input-only GPIOs (34-39) only for inputs
- Check board schematic for conflicts

**Pin conflicts:**
- ATOM: GPIO 27 is display, don't use for other purposes
- Waveshare: GPIO 47 is display, don't use for other purposes
- Always check if pins are already assigned

### Testing New Boards

1. **Display first:** Test with solid colors
2. **Button next:** Test with serial logging
3. **IMU last:** Test orientation changes
4. **Peripheral mode:** Test with two devices

---

## Support

**Need help?**
- Check [Troubleshooting](#troubleshooting) section above
- Open an issue: [GitHub Issues](https://github.com/Xylopyrographer/STAC/issues)
- Join discussion: [GitHub Discussions](https://github.com/Xylopyrographer/STAC/discussions)

**Contributing:**
- Submit custom board configs via pull request
- Document any quirks or special requirements
- Test thoroughly before submitting

---

**Last Updated:** December 2024  
**Version:** 2.3.0


<!-- //  --- EOF --- // -->
