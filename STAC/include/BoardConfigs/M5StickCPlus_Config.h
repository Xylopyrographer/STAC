/**
 * @file M5StickCPlus_Config.h
 * @brief Hardware configuration for M5StickC Plus
 *
 * M5StickC Plus specifications:
 * - MCU: ESP32-PICO-D4 (same as ATOM Matrix)
 * - Display: 135×240 ST7789V2 TFT (SPI interface)
 * - IMU: MPU6886 (I2C, address 0x68)
 * - PMU: AXP192 (I2C, address 0x34) - handles power and backlight
 * - Buttons: A (GPIO 37), B (GPIO 39)
 * - GROVE Port: GPIO 32/33 (bottom)
 * - HAT Header: GPIO 0/25/26/36 (top)
 * - LED: Red on GPIO 10
 * - Buzzer: Passive on GPIO 2
 * - IR TX: GPIO 9
 */

#pragma once

#include <stdint.h>
#include "Config/Types.h"

// ============================================================================
// Board Identification
// ============================================================================
#define STAC_BOARD_NAME "M5StickC Plus"

// ============================================================================
// Display Configuration - TFT
// ============================================================================

/**
 * Display type selection
 * For TFT displays, we use LovyanGFX library instead of LED matrix
 */
#define DISPLAY_TYPE_TFT

// ============================================================================
// TFT Panel Configuration
// ============================================================================

// Panel driver type (used by LGFX_STAC.h)
#define TFT_PANEL_ST7789

// Display dimensions
#define DISPLAY_WIDTH 135
#define DISPLAY_HEIGHT 240

// Panel offsets (for ST7789 135x240 centering in 240x320 memory)
#define TFT_OFFSET_X 52
#define TFT_OFFSET_Y 40

// Color settings
#define TFT_INVERT true       // ST7789 needs color inversion
#define TFT_RGB_ORDER false   // BGR order

// SPI pins for TFT
#define TFT_MOSI 15
#define TFT_SCLK 13
#define TFT_CS   5
#define TFT_DC   23
#define TFT_RST  18

// TFT doesn't use matrix dimensions, but some code expects them
#define DISPLAY_MATRIX_WIDTH 1
#define DISPLAY_MATRIX_HEIGHT 1

// TFT doesn't use display data pin like LED matrix
#define PIN_DISPLAY_DATA 0  // Not used for TFT

// ============================================================================
// Backlight Configuration
// ============================================================================

// Backlight is controlled via AXP192 PMU, not direct GPIO PWM
#define DISPLAY_BACKLIGHT_PMU

/**
 * Brightness mapping for TFT backlight
 * AXP192 LDO2 voltage controls backlight brightness
 * Values 0-255 map to LDO2 voltage (higher = brighter)
 * 4 levels: Min=100, Max=255, evenly distributed
 * Index 0 = off, indices 1-4 = user selectable levels
 */
#define BOARD_BRIGHTNESS_MAP { 0, 100, 152, 203, 255 }

// ============================================================================
// Button Configuration
// ============================================================================

// Primary button (Button A - front large button)
#define PIN_BUTTON 37
#define BUTTON_ACTIVE_LOW true
#define BUTTON_NEEDS_EXTERNAL_PULLUP true  // GPIO 37 is input-only, no internal pullup
#define BUTTON_DEBOUNCE_MS 20

// Secondary button (Button B - side button)
#define BUTTON_B_PIN 39
#define BUTTON_B_ACTIVE_LOW true
#define BUTTON_B_NEEDS_EXTERNAL_PULLUP true  // GPIO 39 is input-only

// ============================================================================
// IMU Configuration
// ============================================================================

#define IMU_HAS_IMU true

#if IMU_HAS_IMU
    #define IMU_TYPE_MPU6886

    // I2C configuration (shared system I2C bus)
    #define PIN_IMU_SDA 21
    #define PIN_IMU_SCL 22
    #define IMU_I2C_CLOCK 400000

    /**
     * Orientation offset for this board
     * M5StickC Plus IMU is mounted such that raw orientation needs +90 degree correction
     */
    #define IMU_ORIENTATION_OFFSET OrientationOffset::OFFSET_90
#endif // IMU_HAS_IMU

// ============================================================================
// Power Management Unit (AXP192)
// ============================================================================

#define HAS_PMU 1
#define PMU_TYPE_AXP192
#define PMU_I2C_SDA 21
#define PMU_I2C_SCL 22
#define PMU_I2C_ADDR 0x34

// AXP192 power rail assignments
#define PMU_TFT_BACKLIGHT_LDO  2   // LDO2 controls TFT backlight
#define PMU_TFT_POWER_LDO      3   // LDO3 controls TFT IC power
#define PMU_ESP32_DCDC         1   // DC-DC1 for ESP32 and MPU6886

// ============================================================================
// Peripheral Mode Configuration (GROVE Port)
// ============================================================================

#define HAS_PERIPHERAL_MODE_CAPABILITY true

#if HAS_PERIPHERAL_MODE_CAPABILITY

    // Peripheral mode detection using HAT header pins
    // Note: GPIO 25 and 26 share the same physical pin on the HAT connector
    // GPIO 25 must be set to floating (INPUT with no pullup) before using GPIO 26
    #define PIN_PM_FLOAT_FIRST 25 // Must float this pin before using GPIO 26
    #define PIN_PM_CHECK_OUT 26   // HAT header pin 6 (GPIO 26)
    #define PIN_PM_CHECK_IN  36   // HAT header pin 5B (GPIO 36, input-only)
    #define PM_CHECK_TOGGLE_COUNT 5

    // GROVE port pins for tally signals
    // TS_0 and TS_1 form the 2-bit tally encoding
    #define PIN_TALLY_STATUS_0 32   // GROVE pin 0
    #define PIN_TALLY_STATUS_1 33   // GROVE pin 1

#endif // HAS_PERIPHERAL_MODE_CAPABILITY

// ============================================================================
// Status LED
// ============================================================================

#define HAS_STATUS_LED true
#if HAS_STATUS_LED
    #define PIN_STATUS_LED 10
    
    // M5StickC Plus has a standard red GPIO LED (active low)
    #define STATUS_LED_TYPE_GPIO
    // #define STATUS_LED_TYPE_ADDRESSABLE
    
    #if defined(STATUS_LED_TYPE_GPIO)
        #define STATUS_LED_ACTIVE_LOW true
    #endif
    
    #if defined(STATUS_LED_TYPE_ADDRESSABLE)
        #define STATUS_LED_STRIP_TYPE LED_STRIP_WS2812
        #define STATUS_LED_IS_RGBW false
    #endif
#endif

// ============================================================================
// Additional Hardware
// ============================================================================

// Passive Buzzer
#define BUZZER_PIN 2

// IR Transmitter
#define IR_TX_PIN 9

// System interrupt (from AXP192)
#define SYSTEM_IRQ_PIN 35

// ============================================================================
// Timing Constants (milliseconds)
// ============================================================================

#define TIMING_AUTOSTART_PULSE_MS 1000
#define TIMING_AUTOSTART_TIMEOUT_MS 20000
#define TIMING_GUI_PAUSE_MS 1500
#define TIMING_GUI_PAUSE_SHORT_MS 500
#define TIMING_BUTTON_SELECT_MS 1500
#define TIMING_WIFI_CONNECT_TIMEOUT_MS 60000
#define TIMING_ERROR_REPOLL_MS 50
#define TIMING_PM_POLL_INTERVAL_MS 2
#define TIMING_OP_MODE_TIMEOUT_MS 30000

// ============================================================================
// Network Configuration
// ============================================================================

#define NETWORK_MAX_POLL_ERRORS 8

// ============================================================================
// HAT Header Pins (for reference/future use)
// ============================================================================
// Pin 4: GPIO 0
// Pin 5: GPIO 25 (also connected to GPIO 36 - use one, float the other)
// Pin 6: GPIO 26

// ============================================================================
// Glyph Configuration
// ============================================================================
// TFT displays use a compatibility glyph layer that maps glyph indices
// to graphics primitives. The GlyphManager template is instantiated with
// GLYPH_WIDTH=1 to satisfy the template, but rendering is handled specially.

#include "Hardware/Display/TFT/GlyphsTFT.h"

namespace Display {
    // Type alias for GlyphManager using TFT stub size (1x1)
    // The actual GlyphManager<1> returns pointers to 1-byte glyph "data"
    // which contains the glyph index. DisplayTFT interprets this index
    // to render the appropriate graphics primitive.
    template<uint8_t SIZE> class GlyphManager;
    using GlyphManagerType = GlyphManager<Display::GLYPH_WIDTH>;
}

namespace Application {
    // StartupConfig is also templated on glyph width for NVS compatibility
    template<uint8_t W> class StartupConfig;
    using StartupConfigType = StartupConfig<Display::GLYPH_WIDTH>;
}

// --- EOF ---
