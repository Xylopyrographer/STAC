/**
 * @file LilygoTQT_Config.h
 * @brief Board configuration for LilyGo T-QT
 *
 * Hardware Specifications:
 * - MCU: ESP32-S3 (FN4R2: 4MB Flash, 2MB PSRAM or N8: 8MB Flash, No PSRAM)
 * - Display: 0.85" GC9A01 TFT, 128×128 pixels (SPI interface)
 * - Backlight: PWM controlled via GPIO 10
 * - Button: Right button (GPIO 47) - primary user input
 * - Reset: Hardware reset button on side (do not use GPIO 0 - boot strapping pin)
 * - Battery Voltage: GPIO 4 (ADC)
 * - No IMU
 * - No PMU (direct GPIO/PWM control)
 * - Same layout as LilyGo T-Display
 */

#ifndef TQT_CONFIG_H
#define TQT_CONFIG_H

// ============================================================================
// BOARD IDENTIFICATION
// ============================================================================

#define STAC_BOARD_NAME "LilyGo T-QT"

// ============================================================================
// DISPLAY CONFIGURATION - TFT
// ============================================================================

#define DISPLAY_TYPE_TFT

// -------------------------------------------------------------------------
// TFT Panel Driver
// -------------------------------------------------------------------------
#define TFT_PANEL_GC9A01

// -------------------------------------------------------------------------
// Display Dimensions (pixels)
// -------------------------------------------------------------------------
// T-QT has native 128x128 GC9A01 (not windowed from 240x240)
#define DISPLAY_WIDTH 128      // Panel width in pixels
#define DISPLAY_HEIGHT 128     // Panel height in pixels

// TFT doesn't use matrix dimensions, but some code expects them
#define DISPLAY_MATRIX_WIDTH 1
#define DISPLAY_MATRIX_HEIGHT 1

// Physical display rotation (display is mounted 180° rotated)
#define DISPLAY_PHYSICAL_ROTATION 2

// -------------------------------------------------------------------------
// SPI Pins
// -------------------------------------------------------------------------
#define TFT_SCLK    3    // SPI clock pin
#define TFT_MOSI    2    // SPI data pin (MOSI)
#define TFT_CS      5    // Chip select pin
#define TFT_DC      6    // Data/Command pin
#define TFT_RST     1    // Reset pin
#define TFT_BL      10   // Backlight pin

// SPI Frequency - GC9A01 supports up to 80 MHz
#define TFT_SPI_FREQ_WRITE 80000000
#define TFT_SPI_FREQ_READ  16000000

// -------------------------------------------------------------------------
// Panel Offset
// -------------------------------------------------------------------------
// GC9A01 128x128 variant - no offset needed
#define TFT_OFFSET_X 0
#define TFT_OFFSET_Y 0

// -------------------------------------------------------------------------
// Panel Color Settings
// -------------------------------------------------------------------------
#define TFT_INVERT false      // No color inversion needed
#define TFT_RGB_ORDER false   // BGR order
#define TFT_READABLE false    // Display doesn't support read-back
#define TFT_BUS_SHARED false  // SPI bus not shared

// ============================================================================
// TFT BACKLIGHT CONTROL
// ============================================================================

#define DISPLAY_BACKLIGHT_PWM

// Brightness configuration for TFT backlight
// Values 0-255 map to PWM duty cycle
// Index 0 = off, indices 1-4 = user selectable levels
// Note: GC9A01 backlight is active-low (LOW = ON)
#define TFT_BACKLIGHT_ON LOW
#define BOARD_BRIGHTNESS_MAP { 0, 128, 170, 212, 255 }

// ============================================================================
// BUTTON CONFIGURATION
// ============================================================================

#define PIN_BUTTON 47  // Right button (primary)

#define BUTTON_DEBOUNCE_MS 25
#define BUTTON_ACTIVE_LOW true
#define BUTTON_NEEDS_EXTERNAL_PULLUP false

// Note: GPIO 0 (left button) is NOT used as it's the boot strapping pin.
// Using GPIO 0 as a button interferes with bootloader entry (hold during reset).
// The T-QT has a dedicated hardware reset button on the side.

// ============================================================================
// IMU CONFIGURATION
// ============================================================================

#define IMU_HAS_IMU false

// ============================================================================
// PERIPHERAL MODE CONFIGURATION
// ============================================================================
// T-QT exposes GPIO42/43 on connector for peripheral communication
// These are safe to use as ESP32-S3 uses USB-Serial/JTAG, not UART0

#define HAS_PERIPHERAL_MODE_CAPABILITY true

// Peripheral connector output pins (2-bit tally state encoding)
#define PIN_TALLY_STATUS_0 43         // LSB of tally state (exposed on connector)
#define PIN_TALLY_STATUS_1 42         // MSB of tally state (exposed on connector)

// ============================================================================
// STATUS LED CONFIGURATION
// ============================================================================

#define HAS_STATUS_LED false

// ============================================================================
// BATTERY MONITORING
// ============================================================================
// T-QT has battery voltage monitoring on GPIO 4
// This is optional and not currently used by STAC

#define PIN_BAT_VOLT 4

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

#include "Hardware/Display/TFT/GlyphsTFT.h"

namespace Display {
    template<uint8_t SIZE> class GlyphManager;
    using GlyphManagerType = GlyphManager<Display::GLYPH_WIDTH>;
}

namespace Application {
    template<uint8_t W> class StartupConfig;
    using StartupConfigType = StartupConfig<Display::GLYPH_WIDTH>;
}

#endif // TQT_CONFIG_H

// --- EOF ---
