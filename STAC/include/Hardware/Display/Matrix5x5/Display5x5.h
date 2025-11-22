#ifndef STAC_DISPLAY5X5_H
#define STAC_DISPLAY5X5_H

#include "../DisplayBase.h"


    namespace Display {

        /**
         * @brief 5x5 LED Matrix Display Implementation
         *
         * Inherits from DisplayBase and only implements size-specific methods.
         * Uses unpacked glyph format (25 bytes per glyph, 1 byte per pixel).
         */
        class Display5x5 : public DisplayBase {
          public:
            /**
             * @brief Construct a new Display5x5 object
             * @param pin GPIO pin connected to LED data line
             * @param numLeds Number of LEDs (should be 25)
             * @param ledType Type of LEDs (e.g., LED_STRIP_WS2812)
             */
            Display5x5( uint8_t pin, uint8_t numLeds, uint8_t ledType );

            /**
             * @brief Destroy the Display5x5 object
             */
            ~Display5x5() override = default;

            // Size-specific overrides
            uint8_t getWidth() const override {
                return 5;
            }
            uint8_t getHeight() const override {
                return 5;
            }
            uint8_t getPixelCount() const override {
                return 25;
            }

          protected:
            /**
             * @brief Convert X,Y to linear position for 5x5 matrix
             * @param x X coordinate (0-4)
             * @param y Y coordinate (0-4)
             * @return Linear pixel position
             */
            uint8_t xyToPosition( uint8_t x, uint8_t y ) const override;
        };

    } // namespace Display


#endif // STAC_DISPLAY5X5_H


//  --- EOF --- //
