/**
 * @file AIPI_Lite_Config.h
 * @brief Board configuration for AIPI-Lite (ESP32-S3 + ST7789 TFT)
 * @version 1.1
 * @date 2026-01-23
 *
 * Revision History:
 * - v1.1 (2026-01-23): Added revision history, TFT_BACKLIGHT_ON, removed jumper detection template
 * - v1.0: Initial configuration
 *
 * Hardware Specifications:
 * - MCU: ESP32-S3 (16MB Flash, 8MB PSRAM)
 * - Display: 1.3" ST7789 TFT, 128×128 pixels (SPI interface)
 * - Backlight: PWM controlled via GPIO 3 (active-high)
 * - Buttons: Button (GPIO 42), Button B (GPIO 1)
 * - Status LED: WS2812 on GPIO 46
 * - No IMU
 * - No PMU (direct GPIO/PWM control)
 */

#ifndef AIPI_LITE_CONFIG_H
#define AIPI_LITE_CONFIG_H

// ============================================================================
// BOARD IDENTIFICATION
// ============================================================================

// Human-readable board name (shown in serial output)
#define STAC_BOARD_NAME "AIPI-Lite"

// ============================================================================
// DISPLAY CONFIGURATION - TFT
// ============================================================================

// Display type
#define DISPLAY_TYPE_TFT

// Physical display rotation (independent of glyph rotation)
// AIPI-Lite display is mounted 90° CCW - use rotation 3 (270°) for upright orientation
#define DISPLAY_PHYSICAL_ROTATION 3

// -------------------------------------------------------------------------
// TFT Panel Driver - Uncomment ONE that matches your display controller
// -------------------------------------------------------------------------
// Small/Medium TFT panels (most common):
// #define TFT_PANEL_ST7789    // 135x240, 240x240, 240x320 (M5StickC Plus, LilyGo T-Display)
#define TFT_PANEL_ST7735S      // AIPI-Lite 128x128 (uses ST7735S driver despite ST7789 label)
// #define TFT_PANEL_ILI9341   // 240x320 (very common, Adafruit, many shields)
// #define TFT_PANEL_ILI9342   // 320x240 (landscape variant of ILI9341)
// #define TFT_PANEL_ILI9163   // 128x128 (small TFTs)
// #define TFT_PANEL_GC9A01    // 240x240 round TFT displays
//
// Larger TFT panels:
// #define TFT_PANEL_ST7796    // 320x480
// #define TFT_PANEL_ILI9481   // 320x480
// #define TFT_PANEL_ILI9486   // 320x480
// #define TFT_PANEL_ILI9488   // 320x480
// #define TFT_PANEL_R61529    // 320x480
// #define TFT_PANEL_HX8357D   // 320x480

// -------------------------------------------------------------------------
// Display Dimensions (pixels)
// -------------------------------------------------------------------------
#define DISPLAY_WIDTH 128      // Panel width in pixels
#define DISPLAY_HEIGHT 128     // Panel height in pixels

// TFT doesn't use matrix dimensions, but some code expects them
#define DISPLAY_MATRIX_WIDTH 1
#define DISPLAY_MATRIX_HEIGHT 1

// -------------------------------------------------------------------------
// SPI Pins (board-specific - check your schematic)
// -------------------------------------------------------------------------
#define TFT_SCLK    16  // SPI clock pin
#define TFT_MOSI    17  // SPI data pin (MOSI)
#define TFT_CS      15  // Chip select pin
#define TFT_DC       7  // Data/Command pin
#define TFT_RST     18  // Reset pin (-1 if not connected)
#define TFT_BL       3  // Backlight pin (-1 if not controllable)
#define TFT_BACKLIGHT_ON HIGH  // Active-high backlight (HIGH = ON)

// SPI frequency (AIPI-Lite works best at 27MHz)
#define TFT_SPI_FREQ_WRITE 27000000
#define TFT_SPI_FREQ_READ 16000000

// -------------------------------------------------------------------------
// Panel Offset (for displays smaller than controller memory)
// -------------------------------------------------------------------------
#define TFT_OFFSET_X 0   // No column offset
#define TFT_OFFSET_Y 0   // No row offset
#define TFT_OFFSET_ROTATION 3  // 270° rotation - display is rotated 90° CCW from default
#define TFT_ROTATION_OFFSET 3  // Added to orientation rotation at runtime

// -------------------------------------------------------------------------
// Panel Color Settings
// -------------------------------------------------------------------------
#define TFT_INVERT false      // No color inversion needed
#define TFT_RGB_ORDER false   // false = BGR (required for this display)
#define TFT_READABLE false    // Display doesn't support read-back
#define TFT_BUS_SHARED false  // SPI bus not shared with other devices

// ============================================================================
// TFT BACKLIGHT CONTROL
// ============================================================================
// Choose ONE of the following backlight control methods:

// Option 1: PWM-controlled backlight (most common for standalone TFTs)
// Use when backlight is directly connected to a GPIO pin
#define DISPLAY_BACKLIGHT_PWM

// Option 2: PMU-controlled backlight (M5StickC Plus with AXP192)
// Use when backlight is controlled via power management unit
// #define DISPLAY_BACKLIGHT_PMU

// Option 3: No backlight control (always on, or hardware-only control)
// Use when backlight cannot be software controlled
// #define DISPLAY_BACKLIGHT_NONE

// Brightness configuration for TFT backlight
// Values 0-255 map to PWM duty cycle or PMU voltage level
// Index 0 = off, indices 1-N = user selectable levels
#define BOARD_BRIGHTNESS_MAP { 0, 128, 170, 212, 255 }

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================

// Button GPIO pin
#define PIN_BUTTON 42  // Example: 39 for ATOM Matrix, 7 for Waveshare S3

// Button debounce time (milliseconds)
#define BUTTON_DEBOUNCE_MS 25  // Default works for most buttons

// Button logic (true = button connects to GND when pressed)
#define BUTTON_ACTIVE_LOW true  // Most buttons are active-low

// External pullup requirement (true if GPIO is input-only, like ESP32 GPIO39)
#define BUTTON_NEEDS_EXTERNAL_PULLUP false  // <OPTIONAL: Set to true for input-only pins>

// ============================================================================
// SOFTWARE RESET BUTTON (Optional)
// ============================================================================
// If your board has a button that should trigger an immediate software
// reset when pressed, define BUTTON_B_PIN. When pressed, the device will
// restart - useful for M5StickC Plus side button or similar hardware.
// If not defined, no reset button functionality is compiled in.

#define BUTTON_B_PIN 1           // Uncomment and set GPIO if reset button exists
#define BUTTON_B_ACTIVE_LOW true             // true = button connects to GND when pressed
// #define BUTTON_B_NEEDS_EXTERNAL_PULLUP false // true if GPIO is input-only (no internal pullup)

// ============================================================================
// IMU CONFIGURATION (Optional - for orientation detection)
// ============================================================================

// Set to true if your board has an IMU, false otherwise
#define IMU_HAS_IMU false

#if IMU_HAS_IMU
    // IMU type - uncomment the ONE that matches your hardware:
    // #define IMU_TYPE_MPU6886    // M5Stack ATOM Matrix
    // #define IMU_TYPE_QMI8658    // Waveshare ESP32-S3-Matrixnabled)
    // #define PIN_IMU_SCL 22     // I2C clock pin
    // #define PIN_IMU_SDA 21     // I2C data pin

    // I2C clock speed (optional - some IMUs have fixed speed)
    #define IMU_I2C_CLOCK 100000L      // 100kHz is standard

    // IMU I2C address (if configurable on your IMU)
    // #define IMU_I2C_ADDRESS 0x6B    // Example for QMI8658

    // Orientation offset correction
    // Compensates for how the IMU is physically mounted on your board
    // Use OrientationOffset enum values from Config/Types.h:
    //   OrientationOffset::OFFSET_0   - No rotation
    //   OrientationOffset::OFFSET_90  - 90° clockwise
    //   OrientationOffset::OFFSET_180 - 180°
    //   OrientationOffset::OFFSET_270 - 270° clockwise (90° counter-clockwise)
    #define IMU_ORIENTATION_OFFSET OrientationOffset::OFFSET_90  // <OPTIONAL: Adjust based on IMU mounting>
#endif // IMU_HAS_IMU

// ============================================================================
// PERIPHERAL MODE CONFIGURATION (Optional)
// ============================================================================
// Mode selection is done via button in software (v3 architecture)
// GPIO pins drive the peripheral connector with 2-bit tally state

// Set to false - AIPI-Lite does not have peripheral mode capability
#define HAS_PERIPHERAL_MODE_CAPABILITY false

#if HAS_PERIPHERAL_MODE_CAPABILITY
    // GROVE/Tally output pins (2-bit tally state encoding)
    #define PIN_TALLY_STATUS_0 <REQUIRED>   // LSB of tally state
    #define PIN_TALLY_STATUS_1 <REQUIRED>   // MSB of tally state
#endif // HAS_PERIPHERAL_MODE_CAPABILITY

// ============================================================================
// STATUS LED CONFIGURATION (Optional)
// ============================================================================

// Set to true if your board has a separate status LED
#define HAS_STATUS_LED true

#if HAS_STATUS_LED
    #define PIN_STATUS_LED 46

    // AIPI-Lite has a WS2812 addressable RGB LED
    // #define STATUS_LED_TYPE_GPIO
    #define STATUS_LED_TYPE_ADDRESSABLE

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
// These values typically work well for all boards - only adjust if needed

#define TIMING_AUTOSTART_PULSE_MS 1000      // Autostart corner blink rate
#define TIMING_AUTOSTART_TIMEOUT_MS 20000   // Autostart wait timeout (20 seconds)
#define TIMING_GUI_PAUSE_MS 1500            // Standard GUI pause duration
#define TIMING_GUI_PAUSE_SHORT_MS 500       // Short GUI pause
#define TIMING_BUTTON_SELECT_MS 1500        // Long press threshold (1.5 seconds)
#define TIMING_WIFI_CONNECT_TIMEOUT_MS 60000 // WiFi connection timeout (60 seconds)
#define TIMING_ERROR_REPOLL_MS 50           // Fast polling rate during errors
#define TIMING_PM_POLL_INTERVAL_MS 2        // Peripheral mode polling (2ms = <2ms latency)
#define TIMING_OP_MODE_TIMEOUT_MS 30000     // Operating mode timeout (30 seconds)

// ============================================================================
// NETWORK CONFIGURATION
// ============================================================================
// Network error handling - typically same for all boards

#define NETWORK_MAX_POLL_ERRORS 8  // Number of consecutive errors before display update

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

#endif // AIPI_LITE_CONFIG_H

/**
 * CHECKLIST BEFORE USE:
 *
 * Required Configuration:
 * [ ] Set STAC_BOARD_NAME to your board's name
 * [ ] Include correct GlyphsMxN.h header for your display size
 * [ ] Set DISPLAY_MATRIX_WIDTH and DISPLAY_MATRIX_HEIGHT to match glyph header
 * [ ] Set DISPLAY_LED_TYPE to match your LED type (WS2812, WS2812_RGB, etc.)
 * [ ] Set PIN_DISPLAY_DATA to correct GPIO
 * [ ] Set PIN_BUTTON to correct GPIO
 * [ ] Set IMU_HAS_IMU (true/false)
 * [ ] If IMU_HAS_IMU: Configure IMU_TYPE and I2C pins
 * [ ] Set HAS_PERIPHERAL_MODE_CAPABILITY (true/false)
 * [ ] If HAS_PERIPHERAL_MODE_CAPABILITY: Configure peripheral pins
 *
 * Optional Adjustments:
 * [ ] Customize BOARD_BRIGHTNESS_MAP for your display
 * [ ] Adjust IMU_ORIENTATION_OFFSET if IMU is rotated on board
 * [ ] Set BUTTON_NEEDS_EXTERNAL_PULLUP if using input-only GPIO
 * [ ] Configure BUTTON_B_PIN if board has a secondary reset button
 * [ ] Configure HAS_STATUS_LED if separate status LED present
 *
 * Testing:
 * [ ] Verify compilation succeeds with no errors
 * [ ] Test all display orientations (if IMU present)
 * [ ] Test button functionality (short press, long press, boot sequences)
 * [ ] Test reset button if configured (BUTTON_B_PIN)
 * [ ] Test peripheral mode (if enabled)
 * [ ] Verify brightness levels are safe (no LED overheating)
 * [ ] Verify all glyphs render correctly at all orientations
 */

//  --- EOF --- //
