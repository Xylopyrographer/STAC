#ifndef STAC_DISPLAY8X8_H
#define STAC_DISPLAY8X8_H

#include "../IDisplay.h"
#include <LiteLED.h>


    namespace Display {

        /**
         * @brief 8x8 LED Matrix Display Implementation
         *
         * Implements IDisplay for 8x8 LED matrix panels.
         * Uses unpacked glyph format (64 bytes per glyph, 1 byte per pixel).
         */
        class Display8x8 : public IDisplay {
          public:
            /**
             * @brief Construct a new Display8x8 object
             * @param pin GPIO pin connected to LED data line
             * @param numLeds Number of LEDs (should be 64)
             * @param ledType Type of LEDs (e.g., LED_STRIP_WS2812)
             */
            Display8x8( uint8_t pin, uint8_t numLeds, uint8_t ledType ); // Just declaration, no body!

            /**
             * @brief Destroy the Display8x8 object
             */
            ~Display8x8() override = default;

            // IDisplay interface implementation
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
            void pulseCorners( bool state, color_t color ) override;

            uint8_t getWidth() const override {
                return 8;
            }
            uint8_t getHeight() const override {
                return 8;
            }
            uint8_t getPixelCount() const override {
                return 64;
            }

          private:
            LiteLED display;            ///< Underlying LiteLED object
            uint8_t pin;                ///< GPIO pin for LED data
            uint8_t numLeds;            ///< Number of LEDs in matrix
            uint8_t currentBrightness;  ///< Current brightness level

            /**
             * @brief Validate pixel position
             * @param position Pixel position to check
             * @return true if position is valid
             */
            bool isValidPosition( uint8_t position ) const;

            /**
             * @brief Convert X,Y to linear position
             * @param x X coordinate
             * @param y Y coordinate
             * @return Linear pixel position
             */
            uint8_t xyToPosition( uint8_t x, uint8_t y ) const;
        };

    } // namespace Display


#endif // STAC_DISPLAY8X8_H


//  --- EOF --- //
