<a name="stac-hardware-configuration-guide"></a>
# STAC Hardware Configuration Guide

This guide explains how to configure STAC for different hardware platforms and how to add support for custom hardware.

* [Overview](#overview)
* [Supported Boards](#supported-boards)
    + [LED Matrix Boards](#led-matrix-boards)
        - [M5Stack ATOM Matrix](#m5stack-atom-matrix)
        - [Waveshare ESP32-S3-Matrix](#waveshare-esp32-s3-matrix)
    + [TFT Display Boards](#tft-display-boards)
        - [M5StickC Plus](#m5stickc-plus)
        - [LilyGo T-Display](#lilygo-t-display)
        - [LilyGo T-QT (ESP32-S3)](#lilygo-t-qt-esp32-s3)
        - [AIPI-Lite (ESP32-S3)](#aipi-lite-esp32-s3)
* [Configuration System](#configuration-system)
    + [Step 1: Select Your Board](#step-1-select-your-board)
    + [Step 2: Compile](#step-2-compile)
    + [Step 3: Upload](#step-3-upload)
* [Board-Specific Settings](#board-specific-settings)
    + [Display Configuration](#display-configuration)
    + [IMU Configuration](#imu-configuration)
    + [Button Configuration](#button-configuration)
    + [Interface Pins](#interface-pins)
    + [Timing Constants](#timing-constants)
    + [Glyph Configuration](#glyph-configuration)
* [Adding Custom Hardware](#adding-custom-hardware)
    + [LED Matrix Boards (WS2812/SK6812)](#led-matrix-boards-ws2812sk6812)
    + [TFT Display Boards](#tft-display-boards-1)
* [Troubleshooting](#troubleshooting)
    + [Compilation Errors](#compilation-errors)
    + [Display Issues](#display-issues)
    + [IMU Issues](#imu-issues)
    + [Button Issues](#button-issues)
    + [Peripheral Mode Issues](#peripheral-mode-issues)
* [Best Practices](#best-practices)
    + [Power Considerations](#power-considerations)
    + [GPIO Selection](#gpio-selection)
    + [Testing New Boards](#testing-new-boards)
    + [TFT Display Optimization](#tft-display-optimization)
* [Supported Display Types](#supported-display-types)
    + [LED Matrix](#led-matrix)
    + [TFT Displays](#tft-displays)
* [Support](#support)

---

<a name="overview"></a>
## Overview

STAC uses a compile-time configuration system based on `Device_Config.h`. Benefits of this approach:

- **Zero runtime overhead** - All decisions made at compile time
- **Type-safe** - Compiler catches configuration errors
- **Easy to use** - Edit one file to change boards
- **Extensible** - Add new boards without modifying core code

<a name="supported-boards"></a>
## Supported Boards

STAC v3 supports a wide range of ESP32-based hardware platforms with both LED matrix and TFT displays.

<a name="led-matrix-boards"></a>
### LED Matrix Boards

<a name="m5stack-atom-matrix"></a>
#### M5Stack ATOM Matrix

**Specifications:**

- **Display:** 5×5 RGB LED matrix (WS2812, GRB color order, serpentine wiring)
- **IMU:** MPU6886 (accelerometer-only for orientation)
- **Button:** GPIO 39 (active low, input-only pin - external pullup required)
- **Processor:** ESP32-PICO-D4
- **Flash:** 4MB
- **PSRAM:** None
- **Peripheral Mode:** Yes (via GROVE port)
- **Pins Available:** Limited (many used by display/IMU)

**Configuration:** `BoardConfigs/AtomMatrix_Config.h`  

---

<a name="waveshare-esp32-s3-matrix"></a>
#### Waveshare ESP32-S3-Matrix

**Specifications:**

- **Display:** 8×8 RGB LED matrix (WS2812, GRB color order, serpentine wiring)
- **IMU:** QMI8658 (accelerometer-only for performance; gyroscope disabled)
- **Button:** GPIO 7 (active low, internal pullup available)
- **Processor:** ESP32-S3-WROOM-1
- **Flash:** 4MB
- **PSRAM:** 2MB (available but not currently used)
- **Peripheral Mode:** Yes (via GROVE port)
- **Pins Available:** GROVE connector exposes I2C and GPIO

**Configuration:** `BoardConfigs/WaveshareS3_Config.h`  


---

<a name="tft-display-boards"></a>
### TFT Display Boards

<a name="m5stickc-plus"></a>
#### M5StickC Plus

**Specifications:**

- **Display:** 1.14" ST7789 TFT (135×240 pixels, SPI interface)
- **IMU:** MPU6886 (accelerometer-only for orientation)
- **PMU:** AXP192 (power management, backlight control)
- **Buttons:** A (GPIO 37), B (GPIO 39)
- **Processor:** ESP32-PICO-D4
- **Flash:** 4MB
- **PSRAM:** None
- **Peripheral Mode:** Yes (via GROVE port on bottom)
- **Pins Available:** GROVE port (GPIO 32/33), HAT header (GPIO 0/25/26/36)

**Configuration:** `BoardConfigs/M5StickCPlus_Config.h`  

---

<a name="lilygo-t-display"></a>
#### LilyGo T-Display

**Specifications:**

- **Display:** 1.14" ST7789 TFT (135×240 pixels, SPI interface)
- **IMU:** None
- **PMU:** None (direct GPIO backlight control)
- **Buttons:** Button 1 (GPIO 35), Button 2 (GPIO 0)
- **Processor:** ESP32 (standard, not PICO or S3)
- **Flash:** 4MB
- **PSRAM:** None
- **Peripheral Mode:** No (no IMU for orientation detection)
- **Pins Available:** I2C header (GPIO 21/22), various GPIO exposed

**Configuration:** `BoardConfigs/LilygoTDisplay_Config.h`  

---

<a name="lilygo-t-qt-esp32-s3"></a>
#### LilyGo T-QT (ESP32-S3)

**Specifications:**

- **Display:** 0.85" GC9A01 TFT (128×128 pixels, round, SPI interface)
- **IMU:** None
- **PMU:** None (direct GPIO backlight control)
- **Buttons:** Left (GPIO 0), Right (GPIO 47)
- **Processor:** ESP32-S3 (FN4R2: 4MB Flash, 2MB PSRAM or N8: 8MB Flash)
- **Flash:** 4MB or 8MB (model dependent)
- **PSRAM:** 2MB (FN4R2) or None (N8)
- **Peripheral Mode:** No (no IMU for orientation detection)
- **Battery:** ADC on GPIO 4 for voltage monitoring

**Configuration:** `BoardConfigs/LilygoTQT_Config.h`

---

<a name="aipi-lite-esp32-s3"></a>
#### AIPI-Lite (ESP32-S3)

**Specifications:**

- **Display:** 1.3" ST7735S TFT (128×128 pixels, SPI interface)
- **IMU:** None
- **PMU:** None (direct GPIO backlight control)
- **Buttons:** Button (GPIO 42), Button B (GPIO 1)
- **Processor:** ESP32-S3
- **Flash:** 16MB
- **PSRAM:** 8MB
- **Peripheral Mode:** No (no IMU for orientation detection)
- **Status LED:** WS2812 on GPIO 46
- **Pins Available:** Multiple GPIO broken out

**Configuration:** `BoardConfigs/AIPI_Lite_Config.h`

---

<a name="configuration-system"></a>
## Configuration System

<a name="step-1-select-your-board"></a>
### Step 1: Select Your Board

Build for your target board using the appropriate environment.

**Debug vs Release:**
- Debug builds: Full serial logging (`CORE_DEBUG_LEVEL=3`)
- Release builds: Minimal logging (`CORE_DEBUG_LEVEL=0`), smaller binary size

<a name="step-2-compile"></a>
### Step 2: Compile

The build system automatically includes the correct drivers and settings:

```bash
# PlatformIO
pio run -e <environment-name>

```

<a name="step-3-upload"></a>
### Step 3: Upload

```bash
# PlatformIO
pio run -e <environment-name> -t upload

```

That's it! The correct drivers, pin assignments, and settings are automatically selected based on your board configuration.

---

<a name="board-specific-settings"></a>
## Board-Specific Settings

Each board configuration file (`BoardConfigs/XYZ_Config.h`) defines:

<a name="display-configuration"></a>
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

<a name="imu-configuration"></a>
### IMU Configuration

```cpp
#define IMU_TYPE_MPU6886               // IMU chip type
#define IMU_HAS_IMU true               // Board has IMU
#define PIN_IMU_SCL 21                 // I2C clock pin
#define PIN_IMU_SDA 25                 // I2C data pin
#define IMU_I2C_CLOCK 100000L          // I2C speed (100kHz)
#define IMU_ORIENTATION_OFFSET 3       // Rotation offset (0-3)
```

**Note:** STAC uses only the accelerometer for orientation detection. Gyroscope is not initialized for performance optimization (faster startup, lower power consumption).

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

<a name="button-configuration"></a>
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

<a name="interface-pins"></a>
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

**Tally Encoding** (2-bit binary):

| `TS_1` | `TS_0` | State |
|------|------|-------|
| LOW  | LOW  | NO_TALLY |
| LOW  | HIGH | UNSELECTED |
| HIGH | LOW  | PREVIEW |
| HIGH | HIGH | PROGRAM |

<a name="timing-constants"></a>
### Timing Constants

```cpp
#define TIMING_BUTTON_SELECT_MS 1500   // Long press threshold
#define TIMING_WIFI_CONNECT_TIMEOUT_MS 60000  // WiFi timeout
#define TIMING_PM_POLL_INTERVAL_MS 2   // Peripheral mode poll rate
```

<a name="glyph-configuration"></a>
### Glyph Configuration

```cpp
#define GLYPH_SIZE_5X5                 // 5×5 or 8×8 glyphs
#define GLYPH_FORMAT_UNPACKED          // Unpacked or bit-packed
```

**Glyph Formats:**

- **5×5 Unpacked:** 25 bytes per glyph (1 byte = 1 pixel)
- **8×8 Bit-packed:** 8 bytes per glyph (8 bits = 8 pixels per row)

---

<a name="adding-custom-hardware"></a>
## Adding Custom Hardware

STAC v3 supports both LED matrix and TFT display boards.

Make a copy of the template in `include/BoardConfigs/TEMPLATE_NewBoard_Config.h` as a starting point.

<a name="led-matrix-boards-ws2812sk6812"></a>
### LED Matrix Boards (WS2812/SK6812)

**Step 1: Create Board Configuration**

Copy the template and rename:

```bash
cd STAC/include/BoardConfigs
cp TEMPLATE_NewBoard_Config.h MyLEDBoard_Config.h
```

**Step 2: Edit Configuration**

Update all settings in `MyLEDBoard_Config.h`:

```cpp
#ifndef MY_LED_BOARD_CONFIG_H
#define MY_LED_BOARD_CONFIG_H

// Board identification
#define STAC_BOARD_NAME "My LED Board"
#define STAC_ID_PREFIX "STAC_CUSTOM"

// Display Configuration - LED MATRIX
#define DISPLAY_TYPE_LED_MATRIX
#define DISPLAY_MATRIX_WIDTH 8         // 5 or 8
#define DISPLAY_MATRIX_HEIGHT 8        // 5 or 8
#define DISPLAY_TOTAL_LEDS (DISPLAY_MATRIX_WIDTH * DISPLAY_MATRIX_HEIGHT)

#define DISPLAY_LED_TYPE LED_STRIP_WS2812  // or LED_STRIP_SK6812_GRBW
#define DISPLAY_LED_IS_RGBW false          // true for RGBW LEDs
#define DISPLAY_COLOR_ORDER_GRB            // Test RGB vs GRB with your LEDs

#define DISPLAY_WIRING_SERPENTINE          // or ROW_BY_ROW

#define PIN_DISPLAY_DATA 27                // GPIO for LED data

#define DISPLAY_BRIGHTNESS_MIN 0
#define DISPLAY_BRIGHTNESS_MAX 60          // Limit to prevent overheating
#define DISPLAY_BRIGHTNESS_DEFAULT 20

// Button Configuration
#define PIN_BUTTON 39
#define BUTTON_DEBOUNCE_MS 25
#define BUTTON_ACTIVE_LOW true
#define BUTTON_NEEDS_EXTERNAL_PULLUP false  // true for GPIO 34-39

// IMU Configuration
#define IMU_TYPE_MPU6886                   // or IMU_TYPE_QMI8658, or IMU_TYPE_NONE
#define IMU_HAS_IMU true                   // false if no IMU
#define PIN_IMU_SCL 21
#define PIN_IMU_SDA 25
#define IMU_I2C_CLOCK 100000L
#define IMU_ORIENTATION_OFFSET 0           // Adjust for IMU mounting (0-3)

// Peripheral Mode (requires IMU for orientation)
#define HAS_PERIPHERAL_MODE_CAPABILITY true  // false if no IMU
#define PIN_TALLY_STATUS_0 32
#define PIN_TALLY_STATUS_1 26

// Glyph Configuration
#define GLYPH_SIZE_8X8                     // or GLYPH_SIZE_5X5
#define GLYPH_FORMAT_BIT_PACKED            // Efficient storage for LED matrix

// Device Orientation to Display Rotation Lookup
#define DEVICE_ORIENTATION_TO_LUT_MAP { \
        Orientation::ROTATE_0,   /* ROTATE_0   → 0°   */ \
        Orientation::ROTATE_90,  /* ROTATE_90  → 90°  */ \
        Orientation::ROTATE_180, /* ROTATE_180 → 180° */ \
        Orientation::ROTATE_270, /* ROTATE_270 → 270° */ \
        Orientation::ROTATE_0,   /* FLAT → your choice */ \
    }

#endif
```

<a name="tft-display-boards-1"></a>
### TFT Display Boards

**Step 1: Create Board Configuration**

Copy an existing TFT config as template:

```bash
cd STAC/include/BoardConfigs
cp LilygoTDisplay_Config.h MyTFTBoard_Config.h
```

**Step 2: Edit Configuration**

Update all settings in `MyTFTBoard_Config.h`:

```cpp
#ifndef MY_TFT_BOARD_CONFIG_H
#define MY_TFT_BOARD_CONFIG_H

// Board identification
#define STAC_BOARD_NAME "My TFT Board"

// Display Configuration - TFT
#define DISPLAY_TYPE_TFT

// TFT Panel Driver - Choose one:
#define TFT_PANEL_ST7789    // Common: 135x240, 240x240, 240x320
// #define TFT_PANEL_ST7735S   // Small: 128x128
// #define TFT_PANEL_ILI9341   // Common: 240x320
// #define TFT_PANEL_GC9A01    // Round: 240x240

// Display dimensions (actual visible pixels)
#define DISPLAY_WIDTH 135
#define DISPLAY_HEIGHT 240

// Physical rotation (if display is mounted rotated)
#define DISPLAY_PHYSICAL_ROTATION 0  // 0, 1, 2, or 3 (0/90/180/270°)

// SPI Pins (check your board schematic)
#define TFT_SCLK    18
#define TFT_MOSI    19
#define TFT_CS      5
#define TFT_DC      16
#define TFT_RST     23
#define TFT_BL      4      // Backlight pin (-1 if none)
#define TFT_BACKLIGHT_ON HIGH  // HIGH or LOW

// Panel offsets (for ST7789 135x240 in 240x320 memory)
#define TFT_OFFSET_X 52
#define TFT_OFFSET_Y 40

// For panels with rotation-dependent offsets:
#define TFT_ROTATION_OFFSETS { \
        {52, 40},  /* Rotation 0 */ \
        {40, 53},  /* Rotation 1 */ \
        {53, 40},  /* Rotation 2 */ \
        {40, 52}   /* Rotation 3 */ \
    }

// Color settings
#define TFT_INVERT true      // Color inversion (test both)
#define TFT_RGB_ORDER false  // false = BGR, true = RGB

// SPI Speed
#define TFT_SPI_FREQ_WRITE 40000000  // 40 MHz typical
#define TFT_SPI_FREQ_READ  16000000  // 16 MHz typical

// Button Configuration
#define PIN_BUTTON 35
#define BUTTON_DEBOUNCE_MS 25
#define BUTTON_ACTIVE_LOW true

// IMU Configuration (optional for TFT boards)
#define IMU_TYPE_NONE
#define IMU_HAS_IMU false

// Peripheral Mode - requires IMU
#define HAS_PERIPHERAL_MODE_CAPABILITY false

// Glyph Configuration
#define GLYPH_SIZE_RENDER         // TFT uses rendered glyphs
#define GLYPH_FORMAT_UNPACKED     // TFT needs unpacked format

#endif
```

**Step 3: Add to platformio.ini**

Create a new environment in `platformio.ini`:

```ini
[env:my-board]
extends = common_settings
platform = https://github.com/pioarduino/platform-espressif32/releases/download/55.03.32/platform-espressif32.zip
board = esp32dev  ; or your specific board

lib_deps = 
    ${core_libs.lib_deps}
    ; Add IMU library if needed:
    ; https://github.com/tanakamasayuki/I2C_MPU6886.git
    ; lewisxhe/SensorLib@^0.2.5
    ; Add Arduino_GFX for TFT:
    ; moononournation/GFX Library for Arduino@^1.6.3

build_flags =
    ${debug_level.build_flags}
    -DBOARD_CONFIG_FILE=\"BoardConfigs/MyBoard_Config.h\"
    ; Add for ESP32-S3 with USB:
    ; -DARDUINO_USB_MODE=1
    ; -DARDUINO_USB_CDC_ON_BOOT=1
    ; Add for TFT text rendering:
    ; -ULITTLE_FOOT_PRINT

; Build filter - exclude unused files
build_src_filter = 
    +<*>
    ; Exclude unused IMU drivers:
    ; -<Hardware/Sensors/QMI8658_IMU.cpp>
    ; -<Hardware/Sensors/MPU6886_IMU.cpp>
    ; Exclude unused display types:
    ; -<Hardware/Display/TFT/*>         ; For LED matrix boards
    ; -<Hardware/Display/Matrix5x5/*>   ; For TFT boards
    ; -<Hardware/Display/Matrix8x8/*>   ; For TFT boards
    -<Hardware/Power/AXP192.cpp>
    -<main_calibrate.cpp>
```

**Step 4: Test Your Board**

1. **Display test:** Verify colors, orientation, brightness
2. **Button test:** Check debouncing and active level  
3. **IMU test (if present):** Verify orientation changes
4. **WiFi test:** Confirm provisioning and network connection

**Step 6: Adjust Settings**

---

<a name="troubleshooting"></a>
## Troubleshooting

<a name="compilation-errors"></a>
### Compilation Errors

**Error:** `No board selected in Device_Config.h`
- **Fix:** Uncomment exactly ONE `#define BOARD_XXX` line in `Device_Config.h`

**Error:** `namespace STAC::Hardware has no member IDisplay`
- **Fix:** Check that all namespaces are correct in STACApp.h

**Error:** `GPIO number error (input-only pad has no internal PU)`
- **Fix:** Set `#define BUTTON_NEEDS_EXTERNAL_PULLUP true` for GPIO 34-39

<a name="display-issues"></a>
### Display Issues

**Problem:** Display shows wrong colors (red appears green, etc.)
- **Fix:** Change `DISPLAY_COLOR_ORDER_RGB` to `DISPLAY_COLOR_ORDER_GRB` or vice versa

**Problem:** Display pattern is wrong (pixels in wrong locations)
- **Fix:** Try `DISPLAY_WIRING_ROW_BY_ROW` instead of `DISPLAY_WIRING_SERPENTINE`

**Problem:** Display too bright/dim
- **Fix:** Adjust the values in the display brightness mapping table

**Problem:** TFT display shows nothing (backlight on)
- **Fix:** Check SPI pins (SCLK, MOSI, CS, DC, RST)
- **Fix:** Verify TFT panel driver (ST7789 vs ST7735S vs ILI9341)
- **Fix:** Try toggling `TFT_INVERT` (true ↔ false)
- **Fix:** Check `TFT_RGB_ORDER` (RGB vs BGR)

**Problem:** TFT display shows shifted/garbled image
- **Fix:** Adjust `TFT_OFFSET_X` and `TFT_OFFSET_Y`
- **Fix:** For ST7789 135x240, use `TFT_ROTATION_OFFSETS` (rotation-dependent)
- **Fix:** Verify `DISPLAY_WIDTH` and `DISPLAY_HEIGHT` match your panel

**Problem:** TFT backlight not working
- **Fix:** Check `TFT_BL` pin number
- **Fix:** Toggle `TFT_BACKLIGHT_ON` (HIGH ↔ LOW for active-high/low)
- **Fix:** COnfigure the PMU; on M5StickC Plus, AXP192 PMU controls backlight

**Problem:** TFT text not rendering (blank rectangles)
- **Fix:** Add `-ULITTLE_FOOT_PRINT` to build_flags in platformio.ini
- **Cause:** Arduino_GFX disables text rendering by default to save memory

<a name="imu-issues"></a>
### IMU Issues

**Problem:** Orientation detection incorrect
- **Fix:** Adjust `IMU_ORIENTATION_OFFSET` (try 0, 1, 2, or 3)

**Problem:** IMU not detected
- **Fix:** Check I2C pins and `IMU_I2C_ADDRESS` (if applicable)
- **Fix:** Ensure IMU library is installed

<a name="button-issues"></a>
### Button Issues

**Problem:** Button presses not detected
- **Fix:** Check `BUTTON_ACTIVE_LOW` setting matches your hardware
- **Fix:** Adjust `BUTTON_DEBOUNCE_MS` if getting multiple triggers

**Problem:** GPIO 39 pullup error (ATOM Matrix)
- **Fix:** Set `BUTTON_NEEDS_EXTERNAL_PULLUP true`

<a name="peripheral-mode-issues"></a>
### Peripheral Mode Issues

**Problem:** Tally not received in peripheral mode
- **Fix:** Check GROVE port wiring (pin-to-pin: `TS_0` to `TS_0`, `TS_1` to `TS_1`, `GND` to `GND`)
- **Fix:** Ensure one device in Normal mode, other in Peripheral mode

---

<a name="best-practices"></a>
## Best Practices

<a name="power-considerations"></a>
### Power Considerations

- **Max Brightness (LED displays):** Keep ≤ 60 to prevent overheating
- **USB Power:** Most USB ports provide 500mA (5V = 2.5W)
- **LED Power:** At full white, full brightness:
  - 5×5 matrix: ~1.5W (300mA @ 5V)
  - 8×8 matrix: ~3.8W (760mA @ 5V)
- **Recommendation:** Use powered USB hub for 8×8 at high brightness

<a name="gpio-selection"></a>
### GPIO Selection

**Good practices:**

- Avoid boot mode pins
- Check board schematic for conflicts
- Always check if pins are already assigned

<a name="testing-new-boards"></a>
### Testing New Boards

1. **Display first:** Test with solid colours (LED matrix) or screen fill (TFT)
2. **Button next:** Test with serial logging
3. **IMU last (if present):** Test orientation changes
4. **WiFi:** Test provisioning portal
5. **Peripheral mode (if supported):** Test with two devices

<a name="tft-display-optimization"></a>
### TFT Display Optimization

- **Lower SPI speed** if display shows artifacts (try 20 MHz instead of 40 MHz)
- **Disable read-back** if not needed (`TFT_READABLE false`)
- **PWM backlight** provides dimming on supported boards
- **Screen rotation** should match physical device orientation

---

<a name="supported-display-types"></a>
## Supported Display Types

<a name="led-matrix"></a>
### LED Matrix
- **Libraries:** LiteLED (optimized for low overhead)
- **Types:** WS2812, WS2812B, SK6812, SK6812_RGBW
- **Sizes:** 5×5 (ATOM Matrix), 8×8 (Waveshare S3)
- **Features:** Orientation-aware glyph rotation, brightness control

<a name="tft-displays"></a>
### TFT Displays
- **Library:** Arduino_GFX (hardware-accelerated SPI)
- **Supported panels:**
  - ST7789
  - ST7735S
  - ILI9341
  - GC9A01
  - And many more
- **Features:** rendered glyphs, full RGB color

---

<a name="support"></a>
## Support

**Need help?**

- Check [Troubleshooting](#troubleshooting) section above
- Open an issue: [GitHub Issues](https://github.com/Xylopyrographer/STAC/issues)
- Join discussion: [GitHub Discussions](https://github.com/Xylopyrographer/STAC/discussions)


<!-- EOF -->
