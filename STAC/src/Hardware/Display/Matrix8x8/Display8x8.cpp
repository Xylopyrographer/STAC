#include "Hardware/Display/Matrix8x8/Display8x8.h"
#include <Arduino.h>


    namespace Display {

        Display8x8::Display8x8( uint8_t pin, uint8_t numLeds, uint8_t ledType )
            : DisplayBase( pin, numLeds, ledType ) {
        }

        uint8_t Display8x8::xyToPosition( uint8_t x, uint8_t y ) const {
            // For 8x8 matrix, assuming row-major order
#ifdef DISPLAY_WIRING_SERPENTINE
            // Serpentine wiring (zigzag pattern)
            if ( y % 2 == 0 ) {
                return y * 8 + x;
            }
            else {
                return y * 8 + ( 7 - x );
            }
#else
            // Row-by-row wiring
            return y * 8 + x;
#endif
        }

    } // namespace Display


//  --- EOF --- //
