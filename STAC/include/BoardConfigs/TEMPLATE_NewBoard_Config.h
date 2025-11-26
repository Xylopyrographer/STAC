/**
 * @file TEMPLATE_NewBoard_Config.h
 * @brief Template for creating a new board configuration file
 * 
 * INSTRUCTIONS:
 * 1. Copy this file to a new name (e.g., MyCustomBoard_Config.h)
 * 2. Update the header guard (replace TEMPLATE_NEWBOARD with your board name)
 * 3. Fill in all the configuration values marked with <REQUIRED>
 * 4. Adjust optional values as needed
 * 5. Include in Device_Config.h by uncommenting the appropriate line
 * 
 * IMPORTANT NOTES:
 * - All GPIO pin numbers are specific to your board's hardware
 * - Display size is determined automatically from the included glyph header
 * - Display wiring pattern must match your LED matrix physical layout
 * - IMU configuration is optional (set IMU_HAS_IMU to false if no IMU)
 * - Peripheral mode pins are optional (controlled by HAS_PERIPHERAL_MODE_CAPABILITY)
 * - Timing and network constants typically use the defaults shown here
 */

#ifndef TEMPLATE_NEWBOARD_CONFIG_H
    #define TEMPLATE_NEWBOARD_CONFIG_H

    // ============================================================================
    // BOARD IDENTIFICATION
    // ============================================================================
    
    // Human-readable board name (shown in serial output)
    #define STAC_BOARD_NAME "<REQUIRED: Board Name>"   // Example: "MyCustom ESP32-C3 Board"

    // ============================================================================
    // DISPLAY CONFIGURATION
    // ============================================================================
    
    // Include the appropriate glyph header for your display size
    // The glyph header automatically provides dimension constants and type aliases
    // Choose ONE of the following:
    #include "Hardware/Display/GlyphsMxN.h"   // <REQUIRED: Replace MxN with your size, e.g., Glyphs5x5.h or Glyphs8x8.h>
    // #include "Hardware/Display/Glyphs5x5.h"  // For 5×5 displays (ATOM Matrix)
    // #include "Hardware/Display/Glyphs8x8.h"  // For 8×8 displays (Waveshare S3)
    
    // Display type (currently only LED matrix supported)
    #define DISPLAY_TYPE_LED_MATRIX
    
    // Display dimensions (must match your physical LED matrix and glyph header)
    #define DISPLAY_MATRIX_WIDTH <REQUIRED>    // Example: 5 for 5×5, 8 for 8×8
    #define DISPLAY_MATRIX_HEIGHT <REQUIRED>   // Example: 5 for 5×5, 8 for 8×8
    
    // LED type and color order configuration
    // Common options:
    //   LED_STRIP_WS2812     - GRB color order (most WS2812 LEDs)
    //   LED_STRIP_WS2812_RGB - RGB color order (some variants)
    //   LED_STRIP_SK6812     - GRBW for RGBW LEDs
    #define DISPLAY_LED_TYPE LED_STRIP_WS2812  // <REQUIRED: Choose appropriate type>
    #define DISPLAY_LED_IS_RGBW false          // Set to true for RGBW (4-color) LEDs
    
    // Display wiring pattern
    // Uncomment ONE of the following based on your LED matrix physical wiring:
    #define DISPLAY_WIRING_ROW_BY_ROW    // Standard row-by-row (most common)
    // #define DISPLAY_WIRING_SERPENTINE  // Snake/zigzag pattern (alternate rows reverse)
    
    // Display data pin (GPIO connected to LED matrix DIN)
    #define PIN_DISPLAY_DATA <REQUIRED>  // Example: 27 for ATOM Matrix, 14 for Waveshare S3
    
    // Brightness configuration
    // Map: index 0 unused, indices 1-N are user-selectable levels (0-255 range)
    // Default brightness is always map[1] (second entry)
    // IMPORTANT: Keep maximum around 60 or less to prevent LED heat damage
    // Adjust number of levels and values based on your display size and power supply
    #define BOARD_BRIGHTNESS_MAP { 0, 10, 20, 30, 40, 50, 60 }  // <OPTIONAL: Customize brightness levels>
    // Example for 8×8 (more LEDs, more levels):
    // #define BOARD_BRIGHTNESS_MAP { 0, 5, 10, 15, 20, 25, 30, 35, 40 }

    // ============================================================================
    // BUTTON CONFIGURATION
    // ============================================================================
    
    // Button GPIO pin
    #define PIN_BUTTON <REQUIRED>  // Example: 39 for ATOM Matrix, 7 for Waveshare S3
    
    // Button debounce time (milliseconds)
    #define BUTTON_DEBOUNCE_MS 25  // Default works for most buttons
    
    // Button logic (true = button connects to GND when pressed)
    #define BUTTON_ACTIVE_LOW true  // Most buttons are active-low
    
    // External pullup requirement (true if GPIO is input-only, like ESP32 GPIO39)
    #define BUTTON_NEEDS_EXTERNAL_PULLUP false  // <OPTIONAL: Set to true for input-only pins>

    // ============================================================================
    // IMU CONFIGURATION (Optional - for orientation detection)
    // ============================================================================
    
    // Set to true if your board has an IMU, false otherwise
    #define IMU_HAS_IMU <REQUIRED: true or false>  // Example: true for ATOM Matrix, Waveshare S3
    
    #if IMU_HAS_IMU
        // IMU type - uncomment the ONE that matches your hardware:
        // #define IMU_TYPE_MPU6886    // M5Stack ATOM Matrix
        // #define IMU_TYPE_QMI8658    // Waveshare ESP32-S3-Matrix
        // #define IMU_TYPE_<YOUR_IMU> // Add your IMU type here
        
        #define IMU_TYPE_<REQUIRED>  // <REQUIRED: Specify your IMU type>
        
        // I2C pins for IMU communication
        #define PIN_IMU_SCL <REQUIRED>     // I2C clock pin
        #define PIN_IMU_SDA <REQUIRED>     // I2C data pin
        
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
        #define IMU_ORIENTATION_OFFSET OrientationOffset::OFFSET_0  // <OPTIONAL: Adjust based on IMU mounting>
    #endif // IMU_HAS_IMU

    // ============================================================================
    // PERIPHERAL MODE CONFIGURATION (Optional)
    // ============================================================================
    
    // Set to true if your board supports peripheral mode (daisy-chaining)
    #define HAS_PERIPHERAL_MODE_CAPABILITY <REQUIRED: true or false>
    
    #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Peripheral mode detection pins (for jumper detection)
        #define PIN_PM_CHECK_OUT <REQUIRED>     // Output pin for detection
        #define PIN_PM_CHECK_IN <REQUIRED>      // Input pin for detection
        #define PM_CHECK_TOGGLE_COUNT 5         // Number of toggles for reliable detection
        
        // GROVE/Tally output pins (2-bit tally state encoding)
        #define PIN_TALLY_STATUS_0 <REQUIRED>   // LSB of tally state
        #define PIN_TALLY_STATUS_1 <REQUIRED>   // MSB of tally state
    #endif // HAS_PERIPHERAL_MODE_CAPABILITY

    // ============================================================================
    // STATUS LED CONFIGURATION (Optional)
    // ============================================================================
    
    // Set to true if your board has a separate status LED
    #define HAS_STATUS_LED false  // <OPTIONAL: Usually false, main display used instead>
    
    #if HAS_STATUS_LED
        #define PIN_STATUS_LED <REQUIRED>               // GPIO for status LED
        #define STATUS_LED_TYPE LED_STRIP_WS2812        // LED type (same options as display)
        #define STATUS_LED_IS_RGBW false                // RGBW capability
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

#endif // TEMPLATE_NEWBOARD_CONFIG_H

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
 * [ ] Configure HAS_STATUS_LED if separate status LED present
 * 
 * Testing:
 * [ ] Verify compilation succeeds with no errors
 * [ ] Test all display orientations (if IMU present)
 * [ ] Test button functionality (short press, long press, boot sequences)
 * [ ] Test peripheral mode (if enabled)
 * [ ] Verify brightness levels are safe (no LED overheating)
 * [ ] Verify all glyphs render correctly at all orientations
 */

//  --- EOF --- //
