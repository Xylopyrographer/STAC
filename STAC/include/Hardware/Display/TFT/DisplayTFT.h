/**
 * @file DisplayTFT.h
 * @brief TFT display implementation using Arduino_GFX
 *
 * Implements IDisplay interface for TFT LCD screens like the ST7789.
 * Uses Arduino_GFX for sprite buffering and drawing operations.
 *
 * Display Strategy:
 * - Large centered channel numbers (rotates with IMU orientation)
 * - Tally status shown via background color/pattern (symmetric, doesn't rotate)
 * - Icons drawn with graphics primitives (WiFi, config, update, etc.)
 *
 * Power Management:
 * - M5StickC Plus: Backlight controlled via AXP192 PMU (LDO2)
 * - LilyGo T-Display: Backlight controlled via PWM (GPIO 4)
 * - Other boards: May use built-in backlight or none
 */

#pragma once

#include "../IDisplay.h"
#include "Config/Types.h"  // For Orientation enum
#include <Arduino_GFX_Library.h>

// Conditionally include AXP192 PMU for boards that use PMU-controlled backlight
#if defined(DISPLAY_BACKLIGHT_PMU)
    #include "Hardware/Power/AXP192.h"
#endif

namespace Display {

    /**
     * @brief TFT display implementation for M5StickC Plus, LilyGo T-Display, etc.
     *
     * This class adapts the LED matrix-oriented IDisplay interface to work
     * with TFT displays. Key differences from LED matrix:
     *
     * - Glyphs are rendered as large graphics, not individual pixels
     * - Background patterns indicate tally status (fills, gradients, frames)
     * - Brightness controlled via PMU or PWM backlight, not LED current
     * - Uses sprite buffering for flicker-free updates
     */
    class DisplayTFT : public IDisplay {
      public:
        /**
         * @brief Construct a TFT display
         * @param width Display width in pixels (default 135 for M5StickC Plus)
         * @param height Display height in pixels (default 240 for M5StickC Plus)
         */
        DisplayTFT( uint16_t width = 135, uint16_t height = 240 );

        ~DisplayTFT() override;

        // ====================================================================
        // IDisplay Interface Implementation
        // ====================================================================

        bool begin() override;
        void clear( bool show = true ) override;
        void setPixel( uint8_t position, color_t color, bool show = true ) override;
        void setPixelXY( uint8_t x, uint8_t y, color_t color, bool show = true ) override;
        void fill( color_t color, bool show = true ) override;
        void drawGlyph( const uint8_t* glyph, color_t foreground, color_t background, bool show = true ) override;
        void setBrightness( uint8_t brightness, bool show = true ) override;
        uint8_t getBrightness() const override;
        void show() override;
        void flash( uint8_t times, uint16_t interval, uint8_t brightness ) override;
        void drawGlyphOverlay( const uint8_t* glyph, color_t color, bool show = true ) override;
        void pulseCorners( const uint8_t* cornersGlyph, bool state, color_t color ) override;
        void pulseDisplay( const uint8_t* glyph, color_t foreground, color_t background,
                           bool& pulseState, uint8_t normalBrightness, uint8_t dimBrightness ) override;
        uint8_t getWidth() const override;
        uint8_t getHeight() const override;
        uint8_t getPixelCount() const override;

        // ====================================================================
        // TFT-Specific Methods
        // ====================================================================

        /**
         * @brief Draw a large centered digit (0-9)
         * @param digit The digit to draw (0-9)
         * @param color Text color
         * @param bgColor Background color
         */
        void drawLargeDigit( uint8_t digit, color_t color, color_t bgColor );

        /**
         * @brief Draw a two-digit channel number
         * @param channel Channel number (1-99)
         * @param color Text color
         * @param bgColor Background color
         */
        void drawChannelNumber( uint8_t channel, color_t color, color_t bgColor );

        /**
         * @brief Draw WiFi icon using primitives
         * @param x Center X position
         * @param y Center Y position
         * @param color Icon color
         * @param connected If true, show connected state; if false, show searching
         */
        void drawWiFiIcon( int16_t x, int16_t y, color_t color, bool connected = true );

        /**
         * @brief Draw configuration/gear icon using primitives
         * @param x Center X position
         * @param y Center Y position
         * @param color Icon color
         */
        void drawConfigIcon( int16_t x, int16_t y, color_t color );

        /**
         * @brief Draw update/download icon using primitives
         * @param x Center X position
         * @param y Center Y position
         * @param color Icon color
         */
        void drawUpdateIcon( int16_t x, int16_t y, color_t color );

        /**
         * @brief Draw checkmark icon using font character
         * @param x Center X position
         * @param y Center Y position
         * @param color Icon color
         */
        void drawCheckIcon( int16_t x, int16_t y, color_t color );

        /**
         * @brief Draw X/error icon using primitives
         * @param x Center X position
         * @param y Center Y position
         * @param color Icon color
         */
        void drawErrorIcon( int16_t x, int16_t y, color_t color );

        /**
         * @brief Draw question mark icon using primitives
         * @param x Center X position
         * @param y Center Y position
         * @param color Icon color
         */
        void drawQuestionIcon( int16_t x, int16_t y, color_t color );

        /**
         * @brief Draw factory reset icon (circular arrow) using primitives
         * @param x Center X position
         * @param y Center Y position
         * @param color Icon color
         */
        void drawResetIcon( int16_t x, int16_t y, color_t color );

        /**
         * @brief Draw a tally status frame (for UNSELECTED state in camera mode)
         * @param color Frame color
         * @param thickness Frame thickness in pixels
         */
        void drawTallyFrame( color_t color, uint8_t thickness = 8 );

        /**
         * @brief Set display rotation (overrides IDisplay)
         * @param rotation Rotation value (0-3, each step is 90 degrees)
         */
        void setRotation( uint8_t rotation ) override;

        /**
         * @brief Get current rotation (overrides IDisplay)
         * @return Current rotation value (0-3)
         */
        uint8_t getRotation() const override;

        /**
         * @brief Set rotation based on IMU orientation (overrides IDisplay)
         * @param orientation IMU orientation value
         */
        void setOrientationRotation( Orientation orientation ) override;

        /**
         * @brief Set initial rotation before display initialization
         * Must be called BEFORE begin() to ensure panel offsets are configured correctly
         * @param rotation Initial rotation value (0-3)
         */
        void setInitialRotation( uint8_t rotation );

      private:
        // Arduino_GFX display and canvas objects
        Arduino_GFX *_gfx;        // Main display instance
        Arduino_Canvas *_canvas;  // Off-screen buffer for flicker-free updates

        // Power management (backlight control) - only for boards with PMU
        #if defined(DISPLAY_BACKLIGHT_PMU)
        Hardware::AXP192 _pmu;
        #endif

        // Display properties
        uint16_t _width;
        uint16_t _height;
        uint8_t _brightness;
        uint8_t _rotation;

        // Internal helpers
        uint16_t colorToRGB565( color_t color ) const;
        void updateBacklight();

        // Rotation-aware dimension helpers
        inline uint16_t currentWidth() const;
        inline uint16_t currentHeight() const;

        // Note: Font remapping moved to global remapCharToSTACSansFont() in STACSansBold24pt7b.h

        // Icon drawing helpers (primitives)
        void drawArc( int16_t cx, int16_t cy, int16_t r, float startAngle, float endAngle,
                      uint16_t color, uint8_t thickness = 2 );
    };

} // namespace Display
