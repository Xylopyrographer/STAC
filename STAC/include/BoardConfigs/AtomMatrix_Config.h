#ifndef ATOM_MATRIX_CONFIG_H
    #define ATOM_MATRIX_CONFIG_H

    // ============================================================================
    // M5STACK ATOM MATRIX CONFIGURATION
    // ============================================================================

    // Board identification
    #define STAC_BOARD_NAME "M5Stack ATOM Matrix"

    // ============================================================================
    // DISPLAY CONFIGURATION
    // ============================================================================
    // Include glyph definitions for this display size
    #include "Hardware/Display/Glyphs5x5.h"
    #define GLYPH_WIDTH_5  // Signal to DisplayFactory which display implementation to use

    #define DISPLAY_TYPE_LED_MATRIX
    #define DISPLAY_MATRIX_WIDTH 5
    #define DISPLAY_MATRIX_HEIGHT 5

    // LED type configuration
    #define DISPLAY_LED_TYPE LED_STRIP_WS2812  // GRB color order (default WS2812)
    #define DISPLAY_LED_IS_RGBW false

    // Display wiring pattern
    // Uncomment the one that matches your panel wiring
    #define DISPLAY_WIRING_ROW_BY_ROW
    // #define DISPLAY_WIRING_SERPENTINE

    // Display pin - The GPIO number connected to the DIN pin on the LED matrix
    #define PIN_DISPLAY_DATA 27

    // Brightness limits (0-255, keep max around 60 to prevent heat damage)
    // Brightness map: index 0 is unused, indices 1-N are user-selectable levels
    // Default brightness is always map[1] (second entry)
    #define BOARD_BRIGHTNESS_MAP { 0, 10, 20, 30, 40, 50, 60 }

    // ============================================================================
    // BUTTON CONFIGURATION
    // ============================================================================

    #define PIN_BUTTON 39
    #define BUTTON_DEBOUNCE_MS 25
    #define BUTTON_ACTIVE_LOW true
    #define BUTTON_NEEDS_EXTERNAL_PULLUP true  // GPIO39 is input-only, requires external pullup

    // ============================================================================
    // IMU CONFIGURATION
    // ============================================================================

    #define IMU_HAS_IMU true
    
    #if IMU_HAS_IMU
        #define IMU_TYPE_MPU6886

        // I2C pins
        #define PIN_IMU_SCL 21
        #define PIN_IMU_SDA 25
        #define IMU_I2C_CLOCK 100000L  // 100kHz

        // IMU orientation offset
        //  Defines which physical direction corresponds to "UP" orientation
        //  Use OrientationOffset enum values from Types.h
        // #define IMU_ORIENTATION_OFFSET OrientationOffset::OFFSET_90

        // IMU Configuration (from calibration tool - systematic testing)
        #define IMU_AXIS_REMAP_X    ((-acc.y))
        #define IMU_AXIS_REMAP_Y    ((-acc.x))
        #define IMU_AXIS_REMAP_Z    (acc.z)
        
        // Display LUT mapping for physical orientations
        // AtomMatrix: X/Y swapped and inverted, 90°/270° LUTs swapped for correct display orientation
        // This maps: physical orientation → which LUT to use for display
        #define DEVICE_ORIENTATION_TO_LUT_MAP { \
            Orientation::ROTATE_0,    /* Physical 0°   → LUT_0 */ \
            Orientation::ROTATE_270,  /* Physical 90°  → LUT_270 (corrected) */ \
            Orientation::ROTATE_180,  /* Physical 180° → LUT_180 */ \
            Orientation::ROTATE_90,   /* Physical 270° → LUT_90 (corrected) */ \
            Orientation::ROTATE_0,    /* FLAT → same as home (0°) */ \
            Orientation::ROTATE_0     /* UNKNOWN → same as home (0°) */ \
        }
        
        // Reverse mapping for debug logging: enum → physical angle
        // AtomMatrix: getOrientation() enums happen to match physical angles
        #define ORIENTATION_ENUM_TO_PHYSICAL_ANGLE { \
            0,   /* ROTATE_0 → Physical 0° */ \
            90,  /* ROTATE_90 → Physical 90° */ \
            180, /* ROTATE_180 → Physical 180° */ \
            270, /* ROTATE_270 → Physical 270° */ \
            -1,  /* FLAT */ \
            -1   /* UNKNOWN */ \
        }

    #endif // IMU_HAS_IMU

    // ============================================================================
    // PERIPHERAL MODE INTERFACE PINS
    // ============================================================================

    #define HAS_PERIPHERAL_MODE_CAPABILITY true
    #if HAS_PERIPHERAL_MODE_CAPABILITY
        #define PIN_PM_CHECK_OUT 22
        #define PIN_PM_CHECK_IN 33
        #define PM_CHECK_TOGGLE_COUNT 5

        // GROVE/Tally output pins
        #define PIN_TALLY_STATUS_0 32
        #define PIN_TALLY_STATUS_1 26

    #endif // HAS_PERIPHERAL_MODE_CAPABILITY

    // ============================================================================
    // STATUS LED CONFIGURATION
    // ============================================================================
    // ATOM Matrix doesn't have a separate status LED (uses the 5x5 matrix)
    
    #define HAS_STATUS_LED false
    
    #if HAS_STATUS_LED
        #define PIN_STATUS_LED 13
        
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

#endif // ATOM_MATRIX_CONFIG_H

//  --- EOF --- //
