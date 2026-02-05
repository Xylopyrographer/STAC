/**
 * @file TEMPLATE_NewBoard_Config.h
 * @brief Template for creating a new board configuration file
 * @version 1.1
 * @date 2026-01-23
 *
 * REVISION HISTORY:
 * v1.1 (2026-01-23): Added peripheral mode pullup configuration, backlight polarity
 * v1.0 (Initial): Base template with all board configuration options
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
// DISPLAY CONFIGURATION - LED MATRIX
// ============================================================================
// Use this section for LED matrix displays (ATOM Matrix, Waveshare S3, etc.)
// Comment out this entire section if using TFT display instead.

/*
// Include the appropriate glyph header for your display size
// The glyph header automatically provides dimension constants and type aliases
// Choose ONE of the following:
#include "Hardware/Display/Glyphs5x5.h"  // For 5×5 displays (ATOM Matrix)
// #include "Hardware/Display/Glyphs8x8.h"  // For 8×8 displays (Waveshare S3)

// Display type
#define DISPLAY_TYPE_LED_MATRIX

// Display dimensions (must match your physical LED matrix and glyph header)
#define DISPLAY_MATRIX_WIDTH 5     // Example: 5 for 5×5, 8 for 8×8
#define DISPLAY_MATRIX_HEIGHT 5    // Example: 5 for 5×5, 8 for 8×8

// LED type and color order configuration
// Common options:
//   LED_STRIP_WS2812     - GRB color order (most WS2812 LEDs)
//   LED_STRIP_WS2812_RGB - RGB color order (some variants)
//   LED_STRIP_SK6812     - GRBW for RGBW LEDs
#define DISPLAY_LED_TYPE LED_STRIP_WS2812
#define DISPLAY_LED_IS_RGBW false          // Set to true for RGBW (4-color) LEDs

// Display wiring pattern
// Uncomment ONE of the following based on your LED matrix physical wiring:
#define DISPLAY_WIRING_ROW_BY_ROW    // Standard row-by-row (most common)
// #define DISPLAY_WIRING_SERPENTINE  // Snake/zigzag pattern (alternate rows reverse)

// Display data pin (GPIO connected to LED matrix DIN)
#define PIN_DISPLAY_DATA 27  // Example: 27 for ATOM Matrix, 14 for Waveshare S3

// Brightness configuration for LED matrix
// Map: index 0 unused, indices 1-N are user-selectable levels (0-255 range)
// Default brightness is always map[1] (second entry)
// IMPORTANT: Keep maximum around 60 or less to prevent LED heat damage
#define BOARD_BRIGHTNESS_MAP { 0, 10, 20, 30, 40, 50, 60 }
*/

// ============================================================================
// DISPLAY CONFIGURATION - TFT
// ============================================================================
// Use this section for TFT displays (M5StickC Plus, LilyGo T-Display, etc.)
// Comment out this entire section if using LED matrix instead.

// Display type
#define DISPLAY_TYPE_TFT

// -------------------------------------------------------------------------
// TFT Panel Driver - Uncomment ONE that matches your display controller
// -------------------------------------------------------------------------
// Small/Medium TFT panels (most common):
#define TFT_PANEL_ST7789       // 135x240, 240x240, 240x320 (M5StickC Plus, LilyGo T-Display)
// #define TFT_PANEL_ST7735    // 128x128, 128x160, 80x160 (many small displays)
// #define TFT_PANEL_ILI9341   // 240x320 (very common, Adafruit, many shields)
// #define TFT_PANEL_ILI9342   // 320x240 (landscape variant of ILI9341)
// #define TFT_PANEL_ILI9163   // 128x128 (small TFTs)
// #define TFT_PANEL_GC9A01    // 240x240 round TFT displays (also 128x128 variant on T-QT)
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
#define DISPLAY_WIDTH 135      // Panel width in pixels
#define DISPLAY_HEIGHT 240     // Panel height in pixels

// TFT Memory Dimensions (for controllers with larger memory than physical display)
// Example: ST7789 135x240 has 240x320 memory, requires offsets for centering
// Most displays: set equal to DISPLAY_WIDTH/HEIGHT
// #define TFT_MEMORY_WIDTH 240   // Uncomment if controller memory > physical width
// #define TFT_MEMORY_HEIGHT 320  // Uncomment if controller memory > physical height

// Physical display rotation (independent of IMU-based orientation)
// Set this if the display is physically mounted rotated on the board
// Values: 0, 1, 2, 3 (corresponding to 0°, 90°, 180°, 270°)
// #define DISPLAY_PHYSICAL_ROTATION 3  // Uncomment if display is physically rotated

// TFT doesn't use matrix dimensions, but some code expects them
#define DISPLAY_MATRIX_WIDTH 1
#define DISPLAY_MATRIX_HEIGHT 1

// -------------------------------------------------------------------------
// SPI Pins (board-specific - check your schematic)
// -------------------------------------------------------------------------
#define TFT_SCLK <REQUIRED>    // SPI clock pin
#define TFT_MOSI <REQUIRED>    // SPI data pin (MOSI)
#define TFT_CS   <REQUIRED>    // Chip select pin
#define TFT_DC   <REQUIRED>    // Data/Command pin
#define TFT_RST  <REQUIRED>    // Reset pin (-1 if not connected)
#define TFT_BL   <REQUIRED>    // Backlight pin (-1 if not controllable)

// SPI Frequency (Hz) - adjust based on your display's maximum speed
// Typical values: 27000000 (27 MHz), 40000000 (40 MHz), 80000000 (80 MHz)
// #define TFT_SPI_FREQ_WRITE 27000000  // Write frequency (uncomment to override default)
// #define TFT_SPI_FREQ_READ  16000000  // Read frequency (uncomment to override default)

// -------------------------------------------------------------------------
// Panel Offset (for displays smaller than controller memory)
// -------------------------------------------------------------------------
// ST7789 135x240 typically needs offset_x=52, offset_y=40
// Most other panels use 0,0
#define TFT_OFFSET_X 52        // X offset for centering (default 0)
#define TFT_OFFSET_Y 40        // Y offset for centering (default 0)

// Rotation-specific offsets (for displays that need different offsets per rotation)
// Example: ST7789 135x240 in 240x320 memory - offsets change with rotation
// Format: {{x0,y0}, {x1,y1}, {x2,y2}, {x3,y3}} for rotations 0°, 90°, 180°, 270°
// Uncomment if needed:
// #define TFT_ROTATION_OFFSETS { \
//     {52, 40},  /* Rotation 0 */ \
//     {40, 53},  /* Rotation 1 */ \
//     {53, 40},  /* Rotation 2 */ \
//     {40, 52}   /* Rotation 3 */ \
// }

// Rotation offset applied when IMU orientation changes
// This is added to the IMU-detected orientation to get correct display rotation
// #define TFT_ROTATION_OFFSET 0  // Uncomment to set (0, 1, 2, or 3)

// Static rotation offset (independent of orientation detection)
// Used when display is mounted at a fixed angle relative to the board
// #define TFT_OFFSET_ROTATION 3  // Uncomment to set (0, 1, 2, or 3)

// -------------------------------------------------------------------------
// Panel Color Settings
// -------------------------------------------------------------------------
// #define TFT_INVERT true        // Color inversion (ST7789 default: true)
// #define TFT_RGB_ORDER false    // true=RGB, false=BGR (default: false/BGR)
// #define TFT_READABLE false     // Display supports read-back operations
// #define TFT_BUS_SHARED false   // SPI bus shared with other devices

// TFT doesn't use display data pin like LED matrix
#define PIN_DISPLAY_DATA 0     // Not used for TFT, placeholder

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

// -------------------------------------------------------------------------
// Backlight Polarity (PWM boards only)
// -------------------------------------------------------------------------
// Define backlight active level:
//   HIGH = Active-high backlight (HIGH = ON, typical for most displays)
//   LOW  = Active-low backlight (LOW = ON, used by some LilyGo boards)
// When active-low, PWM duty cycle is automatically inverted
// #define TFT_BACKLIGHT_ON HIGH  // Uncomment and set to HIGH or LOW if needed

// -------------------------------------------------------------------------
// Brightness Levels
// -------------------------------------------------------------------------
// Brightness configuration for TFT backlight
// Values 0-255 map to PWM duty cycle or PMU voltage level
// Index 0 = off, indices 1-N = user selectable levels
// Note: For active-low backlights, these values are automatically inverted
#define BOARD_BRIGHTNESS_MAP { 0, 128, 170, 212, 255 }

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
// SECONDARY RESET BUTTON (Optional)
// ============================================================================
// If your board has a second button that should trigger an immediate software
// reset when pressed, define BUTTON_B_PIN. When pressed, the device will
// restart - useful for M5StickC Plus side button or similar hardware.
// If not defined, no reset button functionality is compiled in.

// #define BUTTON_B_PIN <GPIO_NUMBER>           // Uncomment and set GPIO if reset button exists
// #define BUTTON_B_ACTIVE_LOW true             // true = button connects to GND when pressed
// #define BUTTON_B_NEEDS_EXTERNAL_PULLUP false // true if GPIO is input-only (no internal pullup)

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

    // Advanced IMU Configuration (Optional - for fine-tuned orientation detection)
    // -----------------------------------------------------------------------

    // IMU axis remapping (if IMU axes don't align with board coordinate system)
    // Uncomment and adjust if needed. 'acc' is the raw accelerometer reading.
    // #define IMU_AXIS_REMAP_X    (acc.x)
    // #define IMU_AXIS_REMAP_Y    (acc.y)
    // #define IMU_AXIS_REMAP_Z    (acc.z)

    // Device orientation to display rotation mapping
    // Maps IMU-detected orientation enums to display rotation LUT indices
    // Calibrate using the IMU calibration tool (see Developer Guide)
    // #define DEVICE_ORIENTATION_TO_LUT_MAP { \
    //     Orientation::ROTATE_270,  /* enum 0 → LUT_270 */ \
    //     Orientation::ROTATE_180,  /* enum 1 → LUT_180 */ \
    //     Orientation::ROTATE_90,   /* enum 2 → LUT_90 */ \
    //     Orientation::ROTATE_0,    /* enum 3 → LUT_0 */ \
    //     Orientation::ROTATE_0,    /* FLAT → same as home */ \
    //     Orientation::ROTATE_0     /* UNKNOWN → same as home */ \
    // }

    // Reverse mapping for debug logging (orientation enum → physical angle in degrees)
    // #define ORIENTATION_ENUM_TO_PHYSICAL_ANGLE { \
    //     90,   /* Orientation::ROTATE_0 → Physical 90° */ \
    //     180,  /* Orientation::ROTATE_90 → Physical 180° */ \
    //     270,  /* Orientation::ROTATE_180 → Physical 270° */ \
    //     0,    /* Orientation::ROTATE_270 → Physical 0° */ \
    //     -1,   /* FLAT */ \
    //     -1    /* UNKNOWN */ \
    // }
#endif // IMU_HAS_IMU

// ============================================================================
// POWER MANAGEMENT UNIT (Optional)
// ============================================================================
// Set to true if your board has a PMU (like AXP192 on M5StickC Plus)

// #define HAS_PMU true

// #if HAS_PMU
//     #define PMU_TYPE_AXP192         // PMU chip type
//     #define PMU_I2C_SDA 21          // PMU I2C data pin
//     #define PMU_I2C_SCL 22          // PMU I2C clock pin
//     #define PMU_I2C_ADDR 0x34       // PMU I2C address
//
//     // AXP192 power rail assignments (example for M5StickC Plus)
//     #define PMU_TFT_BACKLIGHT_LDO 2   // LDO2 controls TFT backlight
//     #define PMU_TFT_POWER_LDO 3       // LDO3 controls TFT IC power
//     #define PMU_ESP32_DCDC 1          // DC-DC1 for ESP32 and sensors
// #endif

// ============================================================================
// PERIPHERAL MODE CONFIGURATION (Optional)
// ============================================================================

// Set to true if your board supports peripheral mode
#define HAS_PERIPHERAL_MODE_CAPABILITY <REQUIRED: true or false>

#if HAS_PERIPHERAL_MODE_CAPABILITY
    // Peripheral connector output pins (2-bit tally state encoding)
    // Mode selection is now done via button, not jumper detection
    #define PIN_TALLY_STATUS_0 <REQUIRED>   // LSB of tally state
    #define PIN_TALLY_STATUS_1 <REQUIRED>   // MSB of tally state
#endif // HAS_PERIPHERAL_MODE_CAPABILITY

// ============================================================================
// STATUS LED CONFIGURATION (Optional)
// ============================================================================
// A separate status LED independent of the main display.
// Supports both standard GPIO LEDs and addressable RGB LEDs (WS2812, etc.)

// Set to true if your board has a separate status LED
#define HAS_STATUS_LED false  // <OPTIONAL: Usually false, main display used instead>

#if HAS_STATUS_LED
    #define PIN_STATUS_LED <REQUIRED>       // GPIO pin for status LED

    // LED Type - uncomment ONE:
    #define STATUS_LED_TYPE_GPIO            // Standard single-color GPIO LED
    // #define STATUS_LED_TYPE_ADDRESSABLE  // WS2812, SK6812, or similar addressable LED

    // GPIO LED settings (when STATUS_LED_TYPE_GPIO is defined)
    #if defined(STATUS_LED_TYPE_GPIO)
        #define STATUS_LED_ACTIVE_LOW true  // true = LED on when pin LOW, false = LED on when pin HIGH
    #endif

    // Addressable LED settings (when STATUS_LED_TYPE_ADDRESSABLE is defined)
    #if defined(STATUS_LED_TYPE_ADDRESSABLE)
        #define STATUS_LED_STRIP_TYPE LED_STRIP_WS2812  // LED type (LED_STRIP_WS2812, LED_STRIP_SK6812, etc.)
        #define STATUS_LED_IS_RGBW false                // true for RGBW LEDs (SK6812), false for RGB
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
// GLYPH CONFIGURATION
// ============================================================================
// TFT displays use a compatibility glyph layer that maps glyph indices
// to graphics primitives. LED matrix displays use actual pixel bitmaps.
// This section is typically the same for all boards of the same display type.

#if defined(DISPLAY_TYPE_TFT)
// TFT glyph configuration
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
#elif defined(DISPLAY_TYPE_LED_MATRIX)
// LED matrix glyph configuration is handled by the included Glyphs header
// No additional configuration needed here
#endif

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
