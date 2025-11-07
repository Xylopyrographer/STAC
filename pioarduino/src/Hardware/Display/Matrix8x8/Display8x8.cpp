// #include "Display8x8.h"
#include "Hardware/Display/Matrix8x8/Display8x8.h"
#include <Arduino.h>

namespace STAC {
    namespace Display {

        Display8x8::Display8x8( uint8_t pin, uint8_t numLeds, uint8_t ledType )
            : display( static_cast<led_strip_type_t>( ledType ), 0 ) // Cast to enum
            , pin( pin )
            , numLeds( numLeds )
            , currentBrightness( 20 ) {
        }

        bool Display8x8::begin() {
            if ( display.begin( pin, numLeds ) != 0 ) {
                log_e( "Failed to initialize 8x8 LED display on pin %d", pin );
                return false;
            }

            clear( false );
            setBrightness( currentBrightness, false );
            show();

            log_i( "8x8 LED Matrix initialized: %d LEDs on pin %d", numLeds, pin );
            return true;
        }

        void Display8x8::clear( bool show ) {
            display.clear( show );
        }

        void Display8x8::setPixel( uint8_t position, rgb_t color, bool show ) {
            if ( !isValidPosition( position ) ) {
                log_w( "Invalid pixel position: %d (valid: 0-%d)", position, numLeds - 1 );
                return;
            }

            // Convert color to board-specific order
            rgb_t boardSpecificColor = boardColor( color );
            display.setPixel( position, boardSpecificColor, show );
        }

        void Display8x8::setPixelXY( uint8_t x, uint8_t y, rgb_t color, bool show ) {
            if ( x >= 8 || y >= 8 ) {
                log_w( "Invalid coordinates: (%d, %d) (valid: 0-7)", x, y );
                return;
            }

            uint8_t position = xyToPosition( x, y );
            setPixel( position, color, show );
        }

        void Display8x8::fill( rgb_t color, bool show ) {
            rgb_t boardSpecificColor = boardColor( color );

            for ( uint8_t i = 0; i < numLeds; i++ ) {
                display.setPixel( i, boardSpecificColor, false );
            }

            if ( show ) {
                this->show();
            }
        }

        void Display8x8::drawGlyph( const uint8_t* glyph, rgb_t foreground, rgb_t background, bool show ) {
            if ( glyph == nullptr ) {
                log_e( "Null glyph pointer" );
                return;
            }

            // Convert colors to board-specific order
            rgb_t fgColor = boardColor( foreground );
            rgb_t bgColor = boardColor( background );

            // Unpack and draw bit-packed glyph (8 bytes, 1 byte per row)
            for ( uint8_t row = 0; row < 8; row++ ) {
                unpackGlyphRow( glyph[ row ], row, fgColor, bgColor );
            }

            if ( show ) {
                this->show();
            }
        }

        void Display8x8::setBrightness( uint8_t brightness, bool show ) {
            currentBrightness = brightness;
            display.brightness( brightness, show );
        }

        uint8_t Display8x8::getBrightness() const {
            return currentBrightness;
        }

        void Display8x8::show() {
            display.show();
            delayMicroseconds( 8000 ); // ~8ms flush time for 8x8 display
        }

        void Display8x8::flash( uint8_t times, uint16_t interval, uint8_t brightness ) {
            uint8_t savedBrightness = currentBrightness;

            for ( uint8_t i = 0; i < times; i++ ) {
                setBrightness( brightness, true );
                delay( interval );
                setBrightness( 0, true );
                delay( interval );
            }

            setBrightness( savedBrightness, true );
        }

        bool Display8x8::isValidPosition( uint8_t position ) const {
            return position < numLeds;
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

        void Display8x8::unpackGlyphRow( uint8_t packedByte, uint8_t row, rgb_t foreground, rgb_t background ) {
            // Unpack a single row (8 bits) of the glyph
            // MSB is leftmost pixel
            for ( uint8_t col = 0; col < 8; col++ ) {
                bool pixelOn = ( packedByte & 0x80 ) != 0; // Test MSB
                rgb_t color = pixelOn ? foreground : background;

                uint8_t position = xyToPosition( col, row );
                display.setPixel( position, color, false );

                packedByte <<= 1;  // Shift to next bit
            }
        }

    } // namespace Display
} // namespace STAC


//  --- EOF --- //
