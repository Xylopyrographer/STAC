#include "Hardware/Display/DisplayBase.h"


    namespace Display {

        DisplayBase::DisplayBase( uint8_t pin, uint8_t numLeds, uint8_t ledType )
            : display( static_cast<led_strip_type_t>( ledType ), 0 )
            , pin( pin )
            , numLeds( numLeds )
            , currentBrightness( 20 ) {
        }

        bool DisplayBase::begin() {
            if ( display.begin( pin, numLeds ) != 0 ) {
                log_e( "Failed to initialize LED display on pin %d", pin );
                return false;
            }

            clear( false );
            setBrightness( currentBrightness, false );
            show();

            log_i( "LED Matrix initialized: %d LEDs on pin %d", numLeds, pin );
            return true;
        }

        void DisplayBase::clear( bool show ) {
            display.clear( show );
        }

        void DisplayBase::setPixel( uint8_t position, color_t color, bool show ) {
            if ( !isValidPosition( position ) ) {
                log_w( "Invalid pixel position: %d (valid: 0-%d)", position, numLeds - 1 );
                return;
            }

            // Convert color to board-specific order
            color_t boardSpecificColor = boardColor( color );
            display.setPixel( position, boardSpecificColor, show );
        }

        void DisplayBase::setPixelXY( uint8_t x, uint8_t y, color_t color, bool show ) {
            uint8_t position = xyToPosition( x, y );
            setPixel( position, color, show );
        }

        void DisplayBase::fill( color_t color, bool show ) {
            color_t boardSpecificColor = boardColor( color );

            for ( uint8_t i = 0; i < numLeds; i++ ) {
                display.setPixel( i, boardSpecificColor, false );
            }

            if ( show ) {
                this->show();
            }
        }

        void DisplayBase::drawGlyph( const uint8_t* glyph, color_t foreground, color_t background, bool show ) {
            if ( glyph == nullptr ) {
                log_e( "Null glyph pointer" );
                return;
            }

            // Convert colors to board-specific order
            color_t fgColor = boardColor( foreground );
            color_t bgColor = boardColor( background );

            // Draw glyph (unpacked format: 1 byte per pixel)
            for ( uint8_t i = 0; i < numLeds; i++ ) {
                color_t color = ( glyph[ i ] != 0 ) ? fgColor : bgColor;
                display.setPixel( i, color, false );
            }

            if ( show ) {
                this->show();
            }
        }

        void DisplayBase::setBrightness( uint8_t brightness, bool show ) {
            currentBrightness = brightness;
            display.brightness( brightness, show );
        }

        uint8_t DisplayBase::getBrightness() const {
            return currentBrightness;
        }

        void DisplayBase::show() {
            display.show();
            // Flush time depends on number of LEDs
            // ~0.3ms per LED @ 800kHz, plus some overhead
            delayMicroseconds( numLeds * 320 );
        }

        void DisplayBase::flash( uint8_t times, uint16_t interval, uint8_t brightness ) {
            uint8_t savedBrightness = currentBrightness;

            for ( uint8_t i = 0; i < times; i++ ) {
                setBrightness( brightness, true );
                delay( interval );
                setBrightness( 0, true );
                delay( interval );
            }

            setBrightness( savedBrightness, true );
        }

        void DisplayBase::drawGlyphOverlay( const uint8_t* glyph, color_t color, bool show ) {
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

        void DisplayBase::pulseDisplay( const uint8_t* glyph, color_t foreground, color_t background,
                                       bool& pulseState, uint8_t normalBrightness, uint8_t dimBrightness ) {
            pulseState = !pulseState;
            setBrightness( pulseState ? normalBrightness : dimBrightness, false );
            drawGlyph( glyph, foreground, background, true );
        }

        bool DisplayBase::isValidPosition( uint8_t position ) const {
            return position < numLeds;
        }

    } // namespace Display


//  --- EOF --- //
