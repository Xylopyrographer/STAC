#ifndef STAC_DISPLAY_BASE_H
#define STAC_DISPLAY_BASE_H

#include "IDisplay.h"
#include <LiteLED.h>
#include <Arduino.h>


namespace Display {

    /**
     * @brief Helper function to convert X,Y to position for square matrices
     * @param x X coordinate
     * @param y Y coordinate
     * @param matrixSize Size of square matrix (5 for 5x5, 8 for 8x8)
     * @param serpentine true for serpentine wiring, false for row-by-row
     * @return Linear position in LED strip
     */
    inline uint8_t xyToPositionHelper( uint8_t x, uint8_t y, uint8_t matrixSize, bool serpentine ) {
        if ( serpentine ) {
            // Serpentine wiring (zigzag pattern)
            if ( y % 2 == 0 ) {
                return y * matrixSize + x;
            }
            else {
                return y * matrixSize + ( matrixSize - 1 - x );
            }
        }
        else {
            // Row-by-row wiring
            return y * matrixSize + x;
        }
    }

    /**
     * @brief Base class for LED matrix displays
     *
     * Implements all size-independent display methods. Derived classes
     * only need to implement size-specific methods like pulseCorners()
     * and provide constructor with appropriate dimensions.
     */
    class DisplayBase : public IDisplay {
      protected:
        LiteLED display;
        uint8_t pin;
        uint8_t numLeds;
        uint8_t currentBrightness;

        /**
         * @brief Constructor for derived classes
         * @param pin GPIO pin for LED data
         * @param numLeds Total number of LEDs
         * @param ledType LED strip type
         */
        DisplayBase( uint8_t pin, uint8_t numLeds, uint8_t ledType );

        /**
         * @brief Convert X,Y coordinates to linear position
         * @param x X coordinate
         * @param y Y coordinate
         * @return Linear position in LED strip
         * @note Must be implemented by derived class based on matrix size and wiring
         */
        virtual uint8_t xyToPosition( uint8_t x, uint8_t y ) const = 0;

        /**
         * @brief Check if position is valid
         * @param position Position to check
         * @return true if position is within valid range
         */
        bool isValidPosition( uint8_t position ) const;

      public:
        virtual ~DisplayBase() = default;

        // Implemented common methods
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
        void pulseDisplay( const uint8_t* glyph, color_t foreground, color_t background,
                           bool& pulseState, uint8_t normalBrightness, uint8_t dimBrightness ) override;
        void pulseCorners( const uint8_t* cornersGlyph, bool state, color_t color ) override;

        // Size-specific methods must be implemented by derived classes
        // getWidth(), getHeight(), getPixelCount() - dimension-specific
    };

} // namespace Display


#endif // STAC_DISPLAY_BASE_H


//  --- EOF --- //
