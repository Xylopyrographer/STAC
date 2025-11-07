#include "Hardware/Display/Matrix5x5/Display5x5.h"
#include <Arduino.h>

namespace STAC {
    namespace Display {

        Display5x5::Display5x5( uint8_t pin, uint8_t numLeds, uint8_t ledType )
            : display( static_cast<led_strip_type_t>( ledType ), 0 ) // Cast to enum
            , pin( pin )
            , numLeds( numLeds )
            , currentBrightness( 20 ) {
        }

        bool Display5x5::begin() {
            if ( display.begin( pin, numLeds ) != 0 ) {
                log_e( "Failed to initialize 5x5 LED display on pin %d", pin );
                return false;
            }

            clear( false );
            setBrightness( currentBrightness, false );
            show();

            log_i( "5x5 LED Matrix initialized: %d LEDs on pin %d", numLeds, pin );
            return true;
        }

        void Display5x5::clear( bool show ) {
            display.clear( show );
        }

        void Display5x5::setPixel( uint8_t position, rgb_t color, bool show ) {
            if ( !isValidPosition( position ) ) {
                log_w( "Invalid pixel position: %d (valid: 0-%d)", position, numLeds - 1 );
                return;
            }

            // Convert color to board-specific order
            rgb_t boardSpecificColor = boardColor( color );
            display.setPixel( position, boardSpecificColor, show );
        }

        void Display5x5::setPixelXY( uint8_t x, uint8_t y, rgb_t color, bool show ) {
            if ( x >= 5 || y >= 5 ) {
                log_w( "Invalid coordinates: (%d, %d) (valid: 0-4)", x, y );
                return;
            }

            uint8_t position = xyToPosition( x, y );
            setPixel( position, color, show );
        }

        void Display5x5::fill( rgb_t color, bool show ) {
            rgb_t boardSpecificColor = boardColor( color );

            for ( uint8_t i = 0; i < numLeds; i++ ) {
                display.setPixel( i, boardSpecificColor, false );
            }

            if ( show ) {
                this->show();
            }
        }

        void Display5x5::drawGlyph( const uint8_t* glyph, rgb_t foreground, rgb_t background, bool show ) {
            if ( glyph == nullptr ) {
                log_e( "Null glyph pointer" );
                return;
            }

            // Convert colors to board-specific order
            rgb_t fgColor = boardColor( foreground );
            rgb_t bgColor = boardColor( background );

            // Draw glyph (unpacked format: 25 bytes, 1 byte per pixel)
            for ( uint8_t i = 0; i < numLeds; i++ ) {
                rgb_t color = ( glyph[ i ] != 0 ) ? fgColor : bgColor;
                display.setPixel( i, color, false );
            }

            if ( show ) {
                this->show();
            }
        }

        void Display5x5::setBrightness( uint8_t brightness, bool show ) {
            currentBrightness = brightness;
            display.brightness( brightness, show );
        }

        uint8_t Display5x5::getBrightness() const {
            return currentBrightness;
        }

        void Display5x5::show() {
            display.show();
            delayMicroseconds( 3000 ); // ~3ms flush time for 5x5 display
        }

        void Display5x5::flash( uint8_t times, uint16_t interval, uint8_t brightness ) {
            uint8_t savedBrightness = currentBrightness;

            for ( uint8_t i = 0; i < times; i++ ) {
                setBrightness( brightness, true );
                delay( interval );
                setBrightness( 0, true );
                delay( interval );
            }

            setBrightness( savedBrightness, true );
        }

        bool Display5x5::isValidPosition( uint8_t position ) const {
            return position < numLeds;
        }

        uint8_t Display5x5::xyToPosition( uint8_t x, uint8_t y ) const {
            // For 5x5 matrix, assuming row-major order
            // This may need adjustment based on actual wiring
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
} // namespace STAC


//  --- EOF --- //
