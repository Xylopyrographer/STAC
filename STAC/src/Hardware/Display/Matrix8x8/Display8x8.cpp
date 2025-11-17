// #include "Display8x8.h"
#include "Hardware/Display/Matrix8x8/Display8x8.h"
#include <Arduino.h>


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

        void Display8x8::setPixel( uint8_t position, color_t color, bool show ) {
            if ( !isValidPosition( position ) ) {
                log_w( "Invalid pixel position: %d (valid: 0-%d)", position, numLeds - 1 );
                return;
            }

            // Convert color to board-specific order
            color_t boardSpecificColor = boardColor( color );
            display.setPixel( position, boardSpecificColor, show );
        }

        void Display8x8::setPixelXY( uint8_t x, uint8_t y, color_t color, bool show ) {
            if ( x >= 8 || y >= 8 ) {
                log_w( "Invalid coordinates: (%d, %d) (valid: 0-7)", x, y );
                return;
            }

            uint8_t position = xyToPosition( x, y );
            setPixel( position, color, show );
        }

        void Display8x8::fill( color_t color, bool show ) {
            color_t boardSpecificColor = boardColor( color );

            for ( uint8_t i = 0; i < numLeds; i++ ) {
                display.setPixel( i, boardSpecificColor, false );
            }

            if ( show ) {
                this->show();
            }
        }

        void Display8x8::drawGlyph( const uint8_t* glyph, color_t foreground, color_t background, bool show ) {
            if ( glyph == nullptr ) {
                log_e( "Null glyph pointer" );
                return;
            }

            // Convert colors to board-specific order
            color_t fgColor = boardColor( foreground );
            color_t bgColor = boardColor( background );

            // Draw glyph (unpacked format: 64 bytes, 1 byte per pixel)
            for ( uint8_t i = 0; i < numLeds; i++ ) {
                color_t color = ( glyph[ i ] != 0 ) ? fgColor : bgColor;
                display.setPixel( i, color, false );
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

        void Display8x8::drawGlyphOverlay( const uint8_t* glyph, color_t color, bool show ) {
            if ( glyph == nullptr ) {
                log_e( "Null glyph pointer" );
                return;
            }

            // Convert color to board-specific order
            color_t boardSpecificColor = boardColor( color );

            // Overlay glyph: only draw pixels where glyph[i] == 1
            for ( uint8_t i = 0; i < numLeds; i++ ) {
                if ( glyph[ i ] == 1 ) {
                    display.setPixel( i, boardSpecificColor, false );
                }
            }

            if ( show ) {
                this->show();
            }
        }

        void Display8x8::pulseCorners( bool state, color_t color ) {
            // Corner pixels for 8x8 matrix: 0 (top-left), 7 (top-right), 56 (bottom-left), 63 (bottom-right)
            color_t pixelColor = state ? boardColor( color ) : StandardColors::BLACK;

            display.setPixel( 0, pixelColor, false );
            display.setPixel( 7, pixelColor, false );
            display.setPixel( 56, pixelColor, false );
            display.setPixel( 63, pixelColor, false );

            show();
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

    } // namespace Display



//  --- EOF --- //
