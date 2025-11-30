#ifndef WAVESHARE_S3_CONFIG_H
    #define WAVESHARE_S3_CONFIG_H

    // Include glyph definitions for this display size
    #include "Hardware/Display/Glyphs8x8.h"
    #define GLYPH_WIDTH_8  // Signal to DisplayFactory which display implementation to use

    // ============================================================================
    // WAVESHARE ESP32-S3-MATRIX CONFIGURATION
    // ============================================================================

    // Board identification
    #define STAC_BOARD_NAME "Waveshare ESP32-S3-Matrix"

    // ============================================================================
    // DISPLAY CONFIGURATION
    // ============================================================================

    #define DISPLAY_TYPE_LED_MATRIX
    #define DISPLAY_MATRIX_WIDTH 8
    #define DISPLAY_MATRIX_HEIGHT 8

    // LED configuration
    #define DISPLAY_LED_TYPE LED_STRIP_WS2812_RGB  // RGB color order for Waveshare
    #define DISPLAY_LED_IS_RGBW false

    // Display wiring pattern
    // Uncomment the one that matches your panel wiring
    #define DISPLAY_WIRING_ROW_BY_ROW
    // #define DISPLAY_WIRING_SERPENTINE

    // Display pin (Waveshare board-specific)
    #define PIN_DISPLAY_DATA 14

    // Brightness limits
    // Brightness map: index 0 is unused, indices 1-N are user-selectable levels
    // Default brightness is always map[1] (second entry)
    #define BOARD_BRIGHTNESS_MAP { 0, 5, 10, 15, 20, 25, 30, 35, 40 }

    // ============================================================================
    // BUTTON CONFIGURATION
    // ============================================================================

    #define PIN_BUTTON 7
    #define BUTTON_DEBOUNCE_MS 25
    #define BUTTON_ACTIVE_LOW true
    #define BUTTON_NEEDS_EXTERNAL_PULLUP false  // GPIO 7 supports internal pullup

    // ============================================================================
    // IMU CONFIGURATION
    // ============================================================================

    #define IMU_HAS_IMU true
    
    #if IMU_HAS_IMU
        #define IMU_TYPE_QMI8658

        // I2C pins for QMI8658 IMU
        #define PIN_IMU_SCL 12
        #define PIN_IMU_SDA 11
        #define IMU_I2C_ADDRESS 0x6B

        // IMU interrupt pins (available but not currently used)
        // #define PIN_IMU_INT1 10
        // #define PIN_IMU_INT2 13

        // IMU orientation offset
        // Defines which physical direction corresponds to "UP" orientation
        #define IMU_ORIENTATION_OFFSET OrientationOffset::OFFSET_180

        // Note: QMI8658 library doesn't allow setting I2C clock (uses default 100kHz)
    #endif // IMU_HAS_IMU

    // ============================================================================
    // PERIPHERAL MODE INTERFACE PINS
    // ============================================================================

    #define HAS_PERIPHERAL_MODE_CAPABILITY true
    #if HAS_PERIPHERAL_MODE_CAPABILITY
        #define PIN_PM_CHECK_OUT 3
        #define PIN_PM_CHECK_IN 4
        #define PM_CHECK_TOGGLE_COUNT 5

        // GROVE/Tally output pins
        #define PIN_TALLY_STATUS_0 5
        #define PIN_TALLY_STATUS_1 6
    #endif // HAS_PERIPHERAL_MODE_CAPABILITY

    // ============================================================================
    // STATUS LED CONFIGURATION
    // ============================================================================
    // Waveshare ESP32-S3-Matrix doesn't have a separate status LED (uses the 8x8 matrix)
    
    #define HAS_STATUS_LED false
    
    #if HAS_STATUS_LED
        #define PIN_STATUS_LED 0
        
        // Uncomment ONE LED type:
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

#endif // WAVESHARE_S3_CONFIG_H


//  --- EOF --- //
