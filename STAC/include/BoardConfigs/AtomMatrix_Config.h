#ifndef ATOM_MATRIX_CONFIG_H
    #define ATOM_MATRIX_CONFIG_H

    // ============================================================================
    // M5STACK ATOM MATRIX CONFIGURATION
    // ============================================================================

    // Board identification
    #define STAC_BOARD_NAME "M5Stack ATOM Matrix"
    #define STAC_ID_PREFIX "STAC"

    // ============================================================================
    // DISPLAY CONFIGURATION
    // ============================================================================

    #define DISPLAY_TYPE_LED_MATRIX
    #define DISPLAY_MATRIX_WIDTH 5
    #define DISPLAY_MATRIX_HEIGHT 5
    #define DISPLAY_TOTAL_LEDS (DISPLAY_MATRIX_WIDTH * DISPLAY_MATRIX_HEIGHT)

    // LED configuration
    #define DISPLAY_LED_TYPE LED_STRIP_WS2812
    #define DISPLAY_LED_IS_RGBW false
    #define DISPLAY_COLOR_ORDER_RGB

    // Display wiring pattern
    // Uncomment the one that matches your panel wiring
    #define DISPLAY_WIRING_ROW_BY_ROW
    // #define DISPLAY_WIRING_SERPENTINE

    // Display pin
    #define PIN_DISPLAY_DATA 27

    // Power indicator pixel (center of 5x5 = pixel 12)
    #define DISPLAY_POWER_LED_PIXEL 12

    // Brightness limits (0-255, keep max around 60 to prevent heat damage)
    #define DISPLAY_BRIGHTNESS_MIN 0
    #define DISPLAY_BRIGHTNESS_MAX 60
    #define DISPLAY_BRIGHTNESS_DEFAULT 20

    // ============================================================================
    // BUTTON CONFIGURATION
    // ============================================================================

    #define PIN_BUTTON 39
    #define BUTTON_DEBOUNCE_MS 25
    #define BUTTON_ACTIVE_LOW true
    #define BUTTON_NEEDS_EXTERNAL_PULLUP true  // Add this line - GPIO39 is input-only

    // ============================================================================
    // IMU CONFIGURATION
    // ============================================================================

    #define IMU_TYPE_MPU6886
    #define IMU_HAS_IMU true

    // I2C pins
    #define PIN_IMU_SCL 21
    #define PIN_IMU_SDA 25
    #define IMU_I2C_CLOCK 100000L  // 100kHz

    // IMU orientation offset
    //  Defines which physical direction corresponds to "UP" orientation
    //  Options: 0, 1, 2, 3 (representing 0°, 90°, 180°, 270° rotation)
    #define IMU_ORIENTATION_OFFSET 1

    // ============================================================================
    // INTERFACE PINS
    // ============================================================================

    // Peripheral mode detection
    #define PIN_PM_CHECK_OUT 22
    #define PIN_PM_CHECK_IN 33
    #define PM_CHECK_TOGGLE_COUNT 5

    // GROVE/Tally output pins
    #define PIN_TALLY_STATUS_0 32
    #define PIN_TALLY_STATUS_1 26

    // Status LED (ATOM doesn't have one)
    #define HAS_STATUS_LED false

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
    // DISPLAY SIZE (for DisplayFactory)
    // ============================================================================

    #define GLYPH_SIZE_5X5

#endif // ATOM_MATRIX_CONFIG_H


//  --- EOF --- //
