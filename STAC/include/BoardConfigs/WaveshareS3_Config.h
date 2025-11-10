#ifndef WAVESHARE_S3_CONFIG_H
    #define WAVESHARE_S3_CONFIG_H

    // ============================================================================
    // WAVESHARE ESP32-S3-MATRIX CONFIGURATION
    // ============================================================================

    // Board identification
    #define STAC_BOARD_NAME "Waveshare ESP32-S3-Matrix"
    #define STAC_ID_PREFIX "STAC_WS"

    // ============================================================================
    // DISPLAY CONFIGURATION
    // ============================================================================

    #define DISPLAY_TYPE_LED_MATRIX
    #define DISPLAY_MATRIX_WIDTH 8
    #define DISPLAY_MATRIX_HEIGHT 8
    #define DISPLAY_TOTAL_LEDS (DISPLAY_MATRIX_WIDTH * DISPLAY_MATRIX_HEIGHT)

    // LED configuration
    #define DISPLAY_LED_TYPE LED_STRIP_WS2812
    #define DISPLAY_LED_IS_RGBW false
    #define DISPLAY_COLOR_ORDER_GRB  // GRB order for Waveshare board

    // Display wiring pattern
    // Uncomment the one that matches your panel wiring
    #define DISPLAY_WIRING_ROW_BY_ROW
    // #define DISPLAY_WIRING_SERPENTINE

    // Display pin (Waveshare board-specific)
    #define PIN_DISPLAY_DATA 47

    // Power indicator pixel (top-left of center 4 pixels in 8x8)
    #define DISPLAY_POWER_LED_PIXEL 27

    // Brightness limits
    #define DISPLAY_BRIGHTNESS_MIN 0
    #define DISPLAY_BRIGHTNESS_MAX 60
    #define DISPLAY_BRIGHTNESS_DEFAULT 20

    // ============================================================================
    // BUTTON CONFIGURATION
    // ============================================================================

    #define PIN_BUTTON 7
    #define BUTTON_DEBOUNCE_MS 25
    #define BUTTON_ACTIVE_LOW true

    // ============================================================================
    // IMU CONFIGURATION
    // ============================================================================

    #define IMU_TYPE_QMI8658
    #define IMU_HAS_IMU true

    // I2C pins (defined in Waveshare board variant)
    #define PIN_IMU_SCL 6
    #define PIN_IMU_SDA 5
    #define IMU_I2C_ADDRESS 0x6B

    // IMU orientation offset
    // Defines which physical direction corresponds to "UP" orientation
    #define IMU_ORIENTATION_OFFSET OFFSET_0  // No offset (adjust as needed)

    // Note: QMI8658 library doesn't allow setting I2C clock
    // Uses default 100kHz

    // ============================================================================
    // INTERFACE PINS
    // ============================================================================

    // Peripheral mode detection
    #define PIN_PM_CHECK_OUT 3
    #define PIN_PM_CHECK_IN 4
    #define PM_CHECK_TOGGLE_COUNT 5

    // GROVE/Tally output pins
    #define PIN_TALLY_STATUS_0 5
    #define PIN_TALLY_STATUS_1 6

    // Status LED (Waveshare board doesn't have built-in LED)
    #define HAS_STATUS_LED true

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
    #define TIMING_NEXT_STATE_MS 750

    // ============================================================================
    // NETWORK CONFIGURATION
    // ============================================================================

    #define NETWORK_MAX_POLL_ERRORS 8

    // ============================================================================
    // NVS (NON-VOLATILE STORAGE)
    // ============================================================================

    #define NVS_NOM_PREFS_VERSION 4    // Normal operating mode version
    #define NVS_PM_PREFS_VERSION 2     // Peripheral mode version

    // ============================================================================
    // GLYPH CONFIGURATION
    // ============================================================================

    #define GLYPH_SIZE_8X8
    #define GLYPH_FORMAT_BITPACKED  // 8 bytes per glyph, bit-packed rows

#endif // WAVESHARE_S3_CONFIG_H


//  --- EOF --- //
