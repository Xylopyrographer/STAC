#include "Hardware/Display/Matrix5x5/Display5x5.h"
#include <Arduino.h>


    namespace Display {

        Display5x5::Display5x5( uint8_t pin, uint8_t numLeds, uint8_t ledType )
            : DisplayBase( pin, numLeds, ledType ) {
        }

        uint8_t Display5x5::xyToPosition( uint8_t x, uint8_t y ) const {
            // For 5x5 matrix, assuming row-major order
#ifdef DISPLAY_WIRING_SERPENTINE
            // Serpentine wiring (zigzag pattern)
            if ( y % 2 == 0 ) {
                return y * 5 + x;
            }
            else {
                return y * 5 + ( 4 - x );
            }
#else
            // Row-by-row wiring
            return y * 5 + x;
#endif
        }

    } // namespace Display


//  --- EOF --- //
