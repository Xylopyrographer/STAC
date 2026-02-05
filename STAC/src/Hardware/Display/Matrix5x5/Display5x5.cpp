#include "Hardware/Display/Matrix5x5/Display5x5.h"
#include <Arduino.h>


namespace Display {

    Display5x5::Display5x5( uint8_t pin, uint8_t numLeds, uint8_t ledType )
        : DisplayBase( pin, numLeds, ledType ) {
    }

    uint8_t Display5x5::xyToPosition( uint8_t x, uint8_t y ) const {
        #ifdef DISPLAY_WIRING_SERPENTINE
        return xyToPositionHelper( x, y, 5, true );
        #else
        return xyToPositionHelper( x, y, 5, false );
        #endif
    }

} // namespace Display


//  --- EOF --- //
