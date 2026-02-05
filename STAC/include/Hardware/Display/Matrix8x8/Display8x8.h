#ifndef STAC_DISPLAY8X8_H
#define STAC_DISPLAY8X8_H

#include "../DisplayBase.h"


namespace Display {

    /**
     * @brief 8x8 LED Matrix Display Implementation
     *
     * Inherits from DisplayBase and only implements size-specific methods.
     * Uses unpacked glyph format (64 bytes per glyph, 1 byte per pixel).
     */
    class Display8x8 : public DisplayBase {
      public:
        /**
         * @brief Construct a new Display8x8 object
         * @param pin GPIO pin connected to LED data line
         * @param numLeds Number of LEDs (should be 64)
         * @param ledType Type of LEDs (e.g., LED_STRIP_WS2812)
         */
        Display8x8( uint8_t pin, uint8_t numLeds, uint8_t ledType );

        /**
         * @brief Destroy the Display8x8 object
         */
        ~Display8x8() override = default;

        // Size-specific overrides
        uint8_t getWidth() const override {
            return 8;
        }
        uint8_t getHeight() const override {
            return 8;
        }
        uint8_t getPixelCount() const override {
            return 64;
        }

      protected:
        /**
         * @brief Convert X,Y to linear position for 8x8 matrix
         * @param x X coordinate (0-7)
         * @param y Y coordinate (0-7)
         * @return Linear pixel position
         */
        uint8_t xyToPosition( uint8_t x, uint8_t y ) const override;
    };

} // namespace Display


#endif // STAC_DISPLAY8X8_H


//  --- EOF --- //
