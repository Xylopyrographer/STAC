#ifndef ATOM_MATRIX_CONFIG_H
    #define ATOM_MATRIX_CONFIG_H

    // ============================================================================
    // M5STACK ATOM MATRIX CONFIGURATION
    // ============================================================================

    // Board identification
    #define STAC_BOARD_NAME "M5Stack ATOM Matrix"   // @Claude: is this used anywhere? Remove if not.
    #define STAC_ID_PREFIX "STAC"                   // @Claude: this is a global prefix for all instances of the STAC. Doesn't need to be defined board by board.
    // #define BOARD_M5STACK_ATOM_MATRIX // @Claude: put this here for each board?

    // ============================================================================
    // DISPLAY CONFIGURATION
    // ============================================================================
    // Include glyph definitions for this display size
    #include "Hardware/Display/Glyphs5x5.h"

    // ============================================================================
    // DISPLAY SIZE (for DisplayFactory)
    // ============================================================================

    #define GLYPH_SIZE_5X5      // @Claude: Is there a way to make this automatic based on the board config?

    #define DISPLAY_TYPE_LED_MATRIX
    #define DISPLAY_MATRIX_WIDTH 5
    #define DISPLAY_MATRIX_HEIGHT 5
    #define DISPLAY_TOTAL_LEDS (DISPLAY_MATRIX_WIDTH * DISPLAY_MATRIX_HEIGHT)   // @Claude: this can be moved out of here and calculated elsewhere. One less thing for the user to mess up.

    // LED type configuration
    #define DISPLAY_LED_TYPE LED_STRIP_WS2812  // GRB color order (default WS2812)
    #define DISPLAY_LED_IS_RGBW false

    // Display wiring pattern
    // Uncomment the one that matches your panel wiring
    #define DISPLAY_WIRING_ROW_BY_ROW
    // #define DISPLAY_WIRING_SERPENTINE

    // Display pin - The GPIO number connected to the DIN pin on the LED matrix
    #define PIN_DISPLAY_DATA 27

    // Power indicator pixel (center of 5x5 = pixel 12)
    #define DISPLAY_POWER_LED_PIXEL 12      // @Claude: No longer needed as we're using a glyph now for the power on pixel

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
    // @Claude: How is this next line used in the code base? Nothing the software can do about a hardware issue.
    #define BUTTON_NEEDS_EXTERNAL_PULLUP true  // Add this line - GPIO39 is input-only

    // ============================================================================
    // IMU CONFIGURATION
    // ============================================================================

    #define IMU_HAS_IMU true    // change to false if no IMU present
    // @Claude: If there is no IMU, should exclude all the following IMU config lines
    #define IMU_TYPE_MPU6886

    // I2C pins
    #define PIN_IMU_SCL 21
    #define PIN_IMU_SDA 25
    #define IMU_I2C_CLOCK 100000L  // 100kHz

    // IMU orientation offset
    //  Defines which physical direction corresponds to "UP" orientation
    //  Options: 0, 1, 2, 3 (representing 0°, 90°, 180°, 270° rotation)
    #define IMU_ORIENTATION_OFFSET 1    // @Claude: Shouldn't this be "OFFSET_90", etc. to match the enum defined in Types.h?

    // ============================================================================
    // PERIPHERAL MODE INTERFACE PINS
    // ============================================================================

    // Peripheral mode detection
    // @Claude: I added the following line as as conditional to include the peripheral mode pins only if needed. Make the following lines conditional on this
    #define HAS_PERIPHERAL_MODE_CAPABILITY true
    #if HAS_PERIPHERAL_MODE_CAPABILITY
        #define PIN_PM_CHECK_OUT 22
        #define PIN_PM_CHECK_IN 33
        #define PM_CHECK_TOGGLE_COUNT 5

        // GROVE/Tally output pins
        #define PIN_TALLY_STATUS_0 32
        #define PIN_TALLY_STATUS_1 26

    #endif // HAS_PERIPHERAL_MODE_CAPABILITY

    // Status LED (ATOM doesn't have one)
    #define HAS_STATUS_LED false
    #if HAS_STATUS_LED
        #define PIN_STATUS_LED 13
        #define STATUS_LED_TYPE LED_STRIP_WS2812  // GRB color order (default WS2812)
        #define STATUS_LED_IS_RGBW false
    #endif // HAS_STATUS_LED

    /**
    * @Claude: Not sure on the place or organization of the TIMING and NETWORK constants.
    * What if the device connects via Ethernet or Bluetooth and has no WiFi for example?
    * Probably not much to do now but your input is welcome.
    */

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
    #define TIMING_NEXT_STATE_MS 750        // @Claude: Where is this used?

    // ============================================================================
    // NETWORK CONFIGURATION
    // ============================================================================

    #define NETWORK_MAX_POLL_ERRORS 8

    // ============================================================================
    // NVS (NON-VOLATILE STORAGE)
    // ============================================================================
    // @Claude: These next two items should be defined elsewhere in the codebase. They are not user definable. Are they used at the moment?
    #define NVS_NOM_PREFS_VERSION 4    // Normal operating mode version
    #define NVS_PM_PREFS_VERSION 2     // Peripheral mode version


#endif // ATOM_MATRIX_CONFIG_H

// @Claude: The changes above also need to be made in the Waveshare config file.

//  --- EOF --- //
