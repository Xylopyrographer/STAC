/**
 * @file AtomS3R_Config.h
 * @brief Board configuration for M5Stack ATOM S3R
 * @version 1.0
 * @date 2026-05-02
 *
 * Hardware Specifications:
 *   SoC:     ESP32-S3-PICO-1-N8R8
 *   Flash:   8 MB (QIO)
 *   PSRAM:   8 MB OPI (Octal)
 *   Display: 0.85" IPS GC9107, 128×128 px (SPI)
 *   Backlight: LP5562 RGBW LED driver, W channel (I2C 0x60)
 *   IMU:     BMI270 + BMM150 (via BMI270 sensor hub), I2C 0x68
 *   Button:  User button GPIO 41 (active-low)
 *   IR TX:   GPIO 47
 *   GROVE (Port A): GPIO 1 / GPIO 2 — peripheral tally output
 */

#ifndef ATOM_S3R_CONFIG_H
#define ATOM_S3R_CONFIG_H

// ============================================================================
// BOARD IDENTIFICATION
// ============================================================================

#define STAC_BOARD_NAME "M5Stack ATOM S3R"

// ============================================================================
// DISPLAY CONFIGURATION - TFT (GC9107 128×128 IPS)
// ============================================================================

#define DISPLAY_TYPE_TFT

// -------------------------------------------------------------------------
// TFT Panel Driver
// -------------------------------------------------------------------------
#define TFT_PANEL_GC9107

// -------------------------------------------------------------------------
// Display Dimensions
// -------------------------------------------------------------------------
#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 128

// TFT doesn't use matrix dimensions, but some code expects them
#define DISPLAY_MATRIX_WIDTH  1
#define DISPLAY_MATRIX_HEIGHT 1

// Physical display rotation — set to match how the panel is mounted.
// Adjust after first boot: 0, 1, 2, or 3.
#define DISPLAY_PHYSICAL_ROTATION 0

// -------------------------------------------------------------------------
// SPI Pins (system SPI bus shared with display)
// -------------------------------------------------------------------------
#define TFT_SCLK  15   // SPI_SCK
#define TFT_MOSI  21   // SPI_MOSI
#define TFT_CS    14   // DIS_CS
#define TFT_DC    42   // DIS_RS / DIS_DC
#define TFT_RST   48   // DIS_RST

// SPI frequency — GC9107 supports up to 80 MHz
#define TFT_SPI_FREQ_WRITE 80000000
#define TFT_SPI_FREQ_READ  16000000

// -------------------------------------------------------------------------
// Panel Offset (GC9107 native 128×128, no window offset)
// -------------------------------------------------------------------------
#define TFT_OFFSET_X 0
#define TFT_OFFSET_Y 0

// -------------------------------------------------------------------------
// Panel Colour Settings
// -------------------------------------------------------------------------
#define TFT_INVERT     false   // No colour inversion
#define TFT_RGB_ORDER  false   // BGR order
#define TFT_READABLE   false
#define TFT_BUS_SHARED false

// ============================================================================
// TFT BACKLIGHT CONTROL — LP5562 W channel (I2C)
// ============================================================================
// The display backlight is driven by the W channel of the LP5562 RGBW LED
// driver IC (I2C address 0x60).  This is NOT a GPIO/PWM backlight.
// The DisplayTFT code uses DISPLAY_BACKLIGHT_LP5562 to select the LP5562
// backlight path rather than the standard GPIO-PWM or AXP192-PMU paths.

#define DISPLAY_BACKLIGHT_LP5562

// LP5562 driver IC — I2C address and bus pins
// (shared I2C bus with IMU)
// NOTE: LP5562 7-bit I2C address is 0x30 (0x60 is the 8-bit write address — Wire uses 7-bit)
#define LED_DRVR_ADDR  0x30
#define LED_DRVR_SDA   45   // SYS_SDA
#define LED_DRVR_SCL    0   // SYS_SCL

// Backlight brightness map: index 0 = off, indices 1–N = user levels
// Values are LP5562 PWM counts (0–255)
#define BOARD_BRIGHTNESS_MAP { 0, 64, 128, 192, 255 }

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================
// The ATOM S3R uses a PMS150G-U6 micro-MCU to handle the hardware reset
// button.  When the reset button is held for ~2 seconds the PMS150G-U6
// pulls GPIO0 LOW, which causes the ESP32-S3 to enter ROM download mode
// (same effect as holding BOOT while pressing RESET on a standard devkit).
// A status LED on the PMS150G-U6 illuminates during download mode.
// GPIO0 is therefore NOT available as a general-purpose input; do not use
// it as a user button (see T-QT note in LilygoTQT_Config.h for same reason).

#define PIN_BUTTON             41   // USR_BUT — display button
#define BUTTON_DEBOUNCE_MS     25
#define BUTTON_ACTIVE_LOW      true
#define BUTTON_NEEDS_EXTERNAL_PULLUP false

// ============================================================================
// IMU CONFIGURATION — BMI270 (+ BMM150 via sensor hub)
// ============================================================================

#define IMU_HAS_IMU true

#if IMU_HAS_IMU
// Direct BMI270 — no runtime detection needed (only one variant of this board)
#define IMU_TYPE_BMI270

// Shared system I2C bus
#define PIN_IMU_SCL  0   // SYS_SCL
#define PIN_IMU_SDA 45   // SYS_SDA
#define IMU_I2C_CLOCK 100000L  // 100 kHz

// BMI270 INT1 output is connected to ESP32-S3 XTAL_32K_N (GPIO16).
// STAC does not use IMU interrupts (orientation is polled), so this
// connection is informational only and requires no firmware action.

// IMU orientation remapping — calibrate after first hardware build.
// Uncomment and adjust once physical orientation is confirmed.
// #define IMU_ORIENTATION_OFFSET OrientationOffset::OFFSET_0

// Axis remap — adjust signs/swaps to match ATOM S3R PCB orientation.
// Placeholder: identity mapping until calibration is done.
#define IMU_AXIS_REMAP_X (acc.x)
#define IMU_AXIS_REMAP_Y (acc.y)
#define IMU_AXIS_REMAP_Z (acc.z)

// Orientation-to-LUT map — placeholder, mirrors ATOM Matrix pattern.
// Update after physical rotation testing.
#define DEVICE_ORIENTATION_TO_LUT_MAP { \
        Orientation::ROTATE_90,   /* enum 0 → LUT_90  */ \
                    Orientation::ROTATE_180,  /* enum 1 → LUT_180 */ \
                    Orientation::ROTATE_270,  /* enum 2 → LUT_270 */ \
                    Orientation::ROTATE_0,    /* enum 3 → LUT_0   */ \
                    Orientation::ROTATE_0,    /* FLAT → home      */ \
                    Orientation::ROTATE_0     /* UNKNOWN → home   */ \
    }

// Reverse mapping for debug logging: enum → physical angle (placeholder)
#define ORIENTATION_ENUM_TO_PHYSICAL_ANGLE { \
        270,  /* Orientation::ROTATE_0   → Physical 270° */ \
        180,  /* Orientation::ROTATE_90  → Physical 180° */ \
        90,  /* Orientation::ROTATE_180 → Physical  90° */ \
        0,  /* Orientation::ROTATE_270 → Physical   0° */ \
        -1,  /* FLAT    */                                  \
        -1   /* UNKNOWN */                                  \
    }

#endif // IMU_HAS_IMU

// ============================================================================
// PERIPHERAL MODE CONFIGURATION
// ============================================================================
// GROVE connector (Port A) output pins for 2-bit tally state

#define HAS_PERIPHERAL_MODE_CAPABILITY true

#if HAS_PERIPHERAL_MODE_CAPABILITY
    #define PIN_TALLY_STATUS_0  1   // LSB of tally state
    #define PIN_TALLY_STATUS_1  2   // MSB of tally state
#endif

// ============================================================================
// STATUS LED CONFIGURATION
// ============================================================================
// The LP5562 R/G/B channels can drive a status LED.
// Integration not yet implemented — set to false for initial bring-up.

#define HAS_STATUS_LED false

// ============================================================================
// TIMING CONSTANTS (milliseconds)
// ============================================================================

#define TIMING_AUTOSTART_PULSE_MS     1000
#define TIMING_AUTOSTART_TIMEOUT_MS  20000
#define TIMING_GUI_PAUSE_MS           1500
#define TIMING_GUI_PAUSE_SHORT_MS      500
#define TIMING_BUTTON_SELECT_MS       1500
#define TIMING_WIFI_CONNECT_TIMEOUT_MS 60000
#define TIMING_ERROR_REPOLL_MS          50
#define TIMING_PM_POLL_INTERVAL_MS       2
#define TIMING_OP_MODE_TIMEOUT_MS    30000

// ============================================================================
// NETWORK CONFIGURATION
// ============================================================================

#define NETWORK_MAX_POLL_ERRORS 8

// ============================================================================
// GLYPH CONFIGURATION
// ============================================================================

#include "Hardware/Display/TFT/GlyphsTFT.h"

namespace Display {
    template<uint8_t SIZE> class GlyphManager;
    using GlyphManagerType = GlyphManager<Display::GLYPH_WIDTH>;
}

namespace Application {
    template<uint8_t W> class StartupConfig;
    using StartupConfigType = StartupConfig<Display::GLYPH_WIDTH>;
}

#endif // ATOM_S3R_CONFIG_H

//  --- EOF --- //
