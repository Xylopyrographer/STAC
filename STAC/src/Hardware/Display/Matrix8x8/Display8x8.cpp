#include "Hardware/Display/Matrix8x8/Display8x8.h"
#include <Arduino.h>


namespace Display {

    Display8x8::Display8x8( uint8_t pin, uint8_t numLeds, uint8_t ledType )
        : DisplayBase( pin, numLeds, ledType ) {
    }

    uint8_t Display8x8::xyToPosition( uint8_t x, uint8_t y ) const {
        #ifdef DISPLAY_WIRING_SERPENTINE
        return xyToPositionHelper( x, y, 8, true );
        #else
        return xyToPositionHelper( x, y, 8, false );
        #endif
    }

} // namespace Display


//  --- EOF --- //
