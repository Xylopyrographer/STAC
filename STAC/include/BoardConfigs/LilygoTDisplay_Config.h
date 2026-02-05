/**
 * @file LilygoTDisplay_Config.h
 * @brief Board configuration for LilyGo T-Display (ESP32 + ST7789 TFT)
 * @version 1.1
 * @date 2026-01-23
 *
 * Revision History:
 * - v1.1 (2026-01-23): Removed jumper-based peripheral mode detection, added TFT_BACKLIGHT_ON
 * - v1.0: Initial configuration
 *
 * Hardware Specifications:
 * - MCU: ESP32 (standard, not PICO or S3)
 * - Display: 1.14" ST7789 TFT, 135×240 pixels (SPI interface)
 * - Backlight: PWM controlled via GPIO 4
 * - Buttons: Button 1 (GPIO 35), Button 2 (GPIO 0)
 * - No IMU
 * - No PMU (direct GPIO/PWM control)
 *
 * Pin Reference (V1.1):
 * | Function   | GPIO |
 * | ---------- | ---- |
 * | TFT_MOSI   | 19   |
 * | TFT_SCLK   | 18   |
 * | TFT_CS     | 5    |
 * | TFT_DC     | 16   |
 * | TFT_RST    | 23   |
 * | TFT_BL     | 4    |
 * | I2C_SDA    | 21   |
 * | I2C_SCL    | 22   |
 * | BUTTON1    | 35   |
 * | BUTTON2    | 0    |
 * | ADC_IN     | 34   |
 * | ADC Power  | 14   |
 */

#ifndef LILYGO_T_DISPLAY_CONFIG_H
#define LILYGO_T_DISPLAY_CONFIG_H

// ============================================================================
// BOARD IDENTIFICATION
// ============================================================================

#define STAC_BOARD_NAME "LilyGo T-Display"

// ============================================================================
// DISPLAY CONFIGURATION - TFT
// ============================================================================

/**
 * Display type selection
 * TFT displays use Arduino_GFX library
 */
#define DISPLAY_TYPE_TFT

// Physical display rotation (independent of glyph rotation)
// LilyGo T-Display default orientation - adjust if display appears rotated
#define DISPLAY_PHYSICAL_ROTATION 0  // 0° - default orientation

// ============================================================================
// TFT Panel Configuration
// ============================================================================

// Panel driver type
#define TFT_PANEL_ST7789

// Display dimensions (ST7789, 135×240)
#define DISPLAY_WIDTH 135
#define DISPLAY_HEIGHT 240

// Panel offsets (for ST7789 135x240 centering in 240x320 memory)
// Single offset values used when TFT_ROTATION_OFFSETS not defined
#define TFT_OFFSET_X 52
#define TFT_OFFSET_Y 40

// Rotation-specific offset handling for ST7789 135x240
// The panel memory is 240x320, so we need different offsets per rotation
// Same as M5StickC Plus (identical display controller and panel size)
#define TFT_ROTATION_OFFSETS { \
        {52, 40},  /* Rotation 0 (portrait  135x240): X=(240-135)/2=52, Y=(320-240)/2=40 */ \
        {40, 53},  /* Rotation 1 (landscape 240x135): X=(320-240)/2=40, Y=(240-135)/2=53 */ \
        {53, 40},  /* Rotation 2 (portrait  135x240): X=(240-135)/2=53, Y=(320-240)/2=40 */ \
        {40, 52}   /* Rotation 3 (landscape 240x135): X=(320-240)/2=40, Y=(240-135)/2=52 */ \
    }

// Color settings
#define TFT_INVERT true       // ST7789 needs color inversion
#define TFT_RGB_ORDER false   // BGR order

// SPI pins for TFT
#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS   5
#define TFT_DC   16
#define TFT_RST  23      // Some versions use 23, some don't connect it (-1)
#define TFT_BL   4       // Backlight PWM pin

// TFT doesn't use matrix dimensions, but some code expects them
#define DISPLAY_MATRIX_WIDTH 1
#define DISPLAY_MATRIX_HEIGHT 1

// ============================================================================
// BACKLIGHT CONFIGURATION - PWM
// ============================================================================

/**
 * Backlight control method
 * LilyGo T-Display uses direct PWM on GPIO 4 via LovyanGFX Light_PWM
 * (Unlike M5StickC Plus which uses AXP192 PMU)
 */
#define DISPLAY_BACKLIGHT_PWM

/**
 * Brightness mapping for TFT backlight (PWM duty cycle)
 * Values 0-255 map to PWM duty cycle
 * Index 0 = off, indices 1-4 = user selectable levels
 * Using 50% minimum to maintain visibility
 */
#define BOARD_BRIGHTNESS_MAP { 0, 128, 170, 212, 255 }

/**
 * Backlight polarity (active HIGH = normal, active LOW = inverted PWM)
 * LilyGo T-Display uses active-high backlight (default)
 */
#define TFT_BACKLIGHT_ON HIGH

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================

// Primary button (top button on right side)
#define PIN_BUTTON 35
#define BUTTON_ACTIVE_LOW true
#define BUTTON_NEEDS_EXTERNAL_PULLUP true  // GPIO 35 is input-only, no internal pullup
#define BUTTON_DEBOUNCE_MS 25

// Note: GPIO 0 button is for boot mode selection, not used as software input
// Hardware reset is via dedicated EN button - no software reset button needed

// ============================================================================
// IMU CONFIGURATION
// ============================================================================

// LilyGo T-Display has no IMU
#define IMU_HAS_IMU false

// ============================================================================
// PERIPHERAL MODE CONFIGURATION
// ============================================================================
// Mode selection is done via button in software (v3 architecture)
// GPIO pins drive the peripheral connector with 2-bit tally state

#define HAS_PERIPHERAL_MODE_CAPABILITY true

#if HAS_PERIPHERAL_MODE_CAPABILITY
    // Tally output pins (directly to GPIO header)
    #define PIN_TALLY_STATUS_0 25   // LSB of tally state (exposed on header)
    #define PIN_TALLY_STATUS_1 26   // MSB of tally state (exposed on header)
#endif

// ============================================================================
// STATUS LED CONFIGURATION
// ============================================================================
// LilyGo T-Display doesn't have a separate status LED (uses TFT display)

#define HAS_STATUS_LED false

#if HAS_STATUS_LED
    #define PIN_STATUS_LED 0

    // Uncomment ONE LED type:
    #define STATUS_LED_TYPE_GPIO
    // #define STATUS_LED_TYPE_ADDRESSABLE

    #if defined(STATUS_LED_TYPE_GPIO)
        #define STATUS_LED_ACTIVE_LOW true
    #endif

    #if defined(STATUS_LED_TYPE_ADDRESSABLE)
        #define STATUS_LED_STRIP_TYPE LED_STRIP_WS2812
        #define STATUS_LED_IS_RGBW false
    #endif
#endif // HAS_STATUS_LED

// ============================================================================
// TIMING CONSTANTS (milliseconds)
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
// NETWORK CONFIGURATION
// ============================================================================

#define NETWORK_MAX_POLL_ERRORS 8

// ============================================================================
// GLYPH CONFIGURATION
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

#endif // LILYGO_T_DISPLAY_CONFIG_H

//  --- EOF --- //
