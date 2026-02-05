/**
 * @file DisplayTFT.cpp
 * @brief TFT display implementation using Arduino_GFX
 */

#include "Hardware/Display/TFT/DisplayTFT.h"
#include "Hardware/Display/TFT/ArduinoGFX_STAC.h"
#include "Hardware/Display/TFT/GlyphsTFT.h"
#include <cmath>

// Use STACSansBold24pt7b for smooth font rendering
// Custom contiguous font (0x20-0x34) with all needed characters: space, *, +, -, 0-9, ?, A, C, N, P, S, T
// Includes digits for brightness display - no separate font needed
#include "Hardware/Display/TFT/STACSansBold24pt7b.h"

// Backlight control method selection based on board config
#if defined(DISPLAY_BACKLIGHT_PMU)
    #define USE_AXP192_PMU
#elif defined(DISPLAY_BACKLIGHT_PWM)
    #define USE_LGFX_BACKLIGHT
#endif
// If neither defined, no software backlight control (DISPLAY_BACKLIGHT_NONE)

// Default rotation can be overridden in board config
#ifndef TFT_DEFAULT_ROTATION
    #define TFT_DEFAULT_ROTATION 0
#endif

// Rotation offset for boards that need rotation adjustment
#ifndef TFT_ROTATION_OFFSET
    #define TFT_ROTATION_OFFSET 0
#endif

namespace Display {

    DisplayTFT::DisplayTFT( uint16_t width, uint16_t height )
        : _gfx( nullptr )
        , _canvas( nullptr )
          #if defined(USE_AXP192_PMU)
        , _pmu()
          #endif
        , _width( width )
        , _height( height )
        , _brightness( 128 )
        , _rotation( TFT_DEFAULT_ROTATION ) {
    }

    DisplayTFT::~DisplayTFT() {
        if ( _canvas ) {
            delete _canvas;
        }
        if ( _gfx ) {
            delete _gfx;
        }
    }

    void DisplayTFT::setInitialRotation( uint8_t rotation ) {
        _rotation = rotation % 4;
        log_i( "Initial rotation set to %d (before display init)", _rotation );
    }

    bool DisplayTFT::begin() {
        log_i( "DisplayTFT::begin() - starting initialization..." );
        // Note: Backlight is already OFF - turned off in main.cpp at boot

        #if defined(USE_AXP192_PMU)
        // Initialize the AXP192 PMU first (controls LCD power and backlight)
        log_i( "Initializing AXP192 PMU..." );
        if ( !_pmu.begin() ) {
            log_e( "Failed to initialize AXP192 PMU" );
            return false;
        }
        // Ensure backlight is off via PMU during init
        _pmu.setBacklight( 0 );
        log_i( "AXP192 PMU initialized" );

        // Small delay for power rail stabilization
        delay( 50 );
        #endif

        log_i( "Creating Arduino_GFX display..." );

        // Create and initialize the display using Arduino_GFX
        _gfx = createArduinoGFXDisplay( _rotation );
        if ( !_gfx ) {
            return false;
        }

        // Note: Hardware reset was already performed in STACApp::initializeHardware()
        // before IMU initialization to keep LCD quiet during sensor readings.
        // This ensures proper reset timing and electrical silence during IMU operation.

        log_i( "Calling _gfx->begin()..." );

        _gfx->begin();

        // Clear entire display controller memory (not just visible window)
        // Do this in all 4 rotations to ensure full memory is cleared
        for ( uint8_t r = 0; r < 4; r++ ) {
            _gfx->setRotation( r );
            _gfx->fillScreen( 0x0000 );
        }
        // Restore desired rotation
        _gfx->setRotation( _rotation );
        delay( 20 ); // Give LCD time to process

        log_i( "Display init complete, rotation set to %d", _rotation );

        // Create canvas for double-buffering (flicker-free updates)
        // Canvas dimensions MUST match the rotated display dimensions
        int16_t canvas_w = _gfx->width();   // Rotated width
        int16_t canvas_h = _gfx->height();  // Rotated height

        log_i( "Creating canvas: %dx%d to match display rotation %d",
               canvas_w, canvas_h, _rotation );

        _canvas = new Arduino_Canvas( canvas_w, canvas_h, _gfx, 0, 0, 0 );
        if ( !_canvas ) {
            return false;
        }

        // Begin canvas
        _canvas->begin();

        // Clear canvas and push to display (backlight is still OFF at this point)
        clear( true );

        // NOTE: Backlight remains OFF here - it will be turned on later when
        // STACApp calls setBrightness() after all initialization is complete
        // and the first real content is ready to display. This prevents showing
        // any startup artifacts from stale LCD memory.
        _brightness = 0;

        log_i( "TFT Display initialized: %dx%d (canvas), rotation %d", canvas_w, canvas_h, _rotation );
        return true;
    }

    void DisplayTFT::clear( bool doShow ) {
        if ( _canvas ) {
            _canvas->fillScreen( 0x0000 ); // 0x0000
            if ( doShow ) {
                show();
            }
        }
    }

    void DisplayTFT::setPixel( uint8_t position, color_t color, bool doShow ) {
        // For TFT, we don't typically use single-pixel operations
        // Map position to x,y for compatibility
        uint16_t w = _canvas ? _canvas->width() : currentWidth();
        uint8_t x = position % w;
        uint8_t y = position / w;
        setPixelXY( x, y, color, doShow );
    }

    void DisplayTFT::setPixelXY( uint8_t x, uint8_t y, color_t color, bool doShow ) {
        if ( _canvas && x < _canvas->width() && y < _canvas->height() ) {
            _canvas->drawPixel( x, y, colorToRGB565( color ) );
            if ( doShow ) {
                show();
            }
        }
    }

    void DisplayTFT::fill( color_t color, bool doShow ) {
        if ( _canvas ) {
            _canvas->fillScreen( colorToRGB565( color ) );
            if ( doShow ) {
                show();
            }
        }
    }

    void DisplayTFT::drawGlyph( const uint8_t* glyph, color_t foreground, color_t background, bool doShow ) {
        if ( !_canvas || !glyph ) {
            return;
        }

        // Extract glyph index from the stub glyph data
        // Each TFT "glyph" is a 1-byte array containing its index
        uint8_t glyphIndex = glyph[ 0 ];

        log_i( "drawGlyph: index=%d, fg=0x%06X, bg=0x%06X, Sprite: %dx%d, LCD: %dx%d",
               glyphIndex, foreground, background,
               _canvas->width(), _canvas->height(), _gfx->width(), _gfx->height() );

        // Fill background first
        fill( background, false );

        // Center coordinates for icon drawing (use sprite dimensions)
        int16_t cx = _canvas->width() / 2;
        int16_t cy = _canvas->height() / 2;

        // Render based on glyph index
        switch ( glyphIndex ) {
            case Display::GLF_0:
            case Display::GLF_1:
            case Display::GLF_2:
            case Display::GLF_3:
            case Display::GLF_4:
            case Display::GLF_5:
            case Display::GLF_6:
            case Display::GLF_7:
            case Display::GLF_8:
            case Display::GLF_9:
                // Numeric digits
                drawLargeDigit( glyphIndex, foreground, background );
                return;  // drawLargeDigit calls show()

            case Display::GLF_WIFI:
                drawWiFiIcon( cx, cy, foreground, true );
                break;

            case Display::GLF_CFG:
                drawConfigIcon( cx, cy, foreground );
                break;

            case Display::GLF_UD:
                drawUpdateIcon( cx, cy, foreground );
                break;

            case Display::GLF_CK:
                drawCheckIcon( cx, cy, foreground );
                break;

            case Display::GLF_BX:
            case Display::GLF_X:
                drawErrorIcon( cx, cy, foreground );
                break;

            case Display::GLF_QM:
                drawQuestionIcon( cx, cy, foreground );
                break;

            case Display::GLF_FM:
                // Frame glyph - draw a border
                drawTallyFrame( foreground, 8 );
                return;  // drawTallyFrame calls show()

            case Display::GLF_DF:
                // Dotted frame (unselected tally) - purple on black checkerboard
                // Dynamically sized to fit display without clipping
            {
                uint16_t fg = colorToRGB565( foreground );
                uint16_t bg = colorToRGB565( background );
                uint16_t w = _canvas->width();
                uint16_t h = _canvas->height();

                // Calculate block size for perfect fit (target 5 blocks per smallest dimension)
                const uint8_t TARGET_BLOCKS = 5;
                uint16_t minDim = min( w, h );
                uint8_t blockSize = minDim / TARGET_BLOCKS;

                // Constrain to reasonable range for visibility
                blockSize = constrain( blockSize, 10, 32 );

                // Calculate grid dimensions
                uint8_t cols = w / blockSize;
                uint8_t rows = h / blockSize;

                // Center the grid
                int xOffset = ( w - ( cols * blockSize ) ) / 2;
                int yOffset = ( h - ( rows * blockSize ) ) / 2;

                // Fill background first
                _canvas->fillScreen( bg );

                // Draw centered checkerboard grid
                for ( int row = 0; row < rows; row++ ) {
                    for ( int col = 0; col < cols; col++ ) {
                        bool isForeground = ( col + row ) % 2 == 0;
                        if ( isForeground ) {
                            _canvas->fillRect( xOffset + col * blockSize, yOffset + row * blockSize, blockSize, blockSize, fg );
                        }
                    }
                }
            }
            break;

            case Display::GLF_ST:
                // Smart Tally icon - TEST with drawChar instead of print
            {
                _canvas->fillScreen( colorToRGB565( background ) );

                // Try drawChar directly - lower level than print()
                _canvas->drawChar( 10, 10, 'S', 0xFFFF, 0x0000 );
                _canvas->drawChar( 22, 10, 'T', 0xFFFF, 0x0000 );

                // Try different positions and sizes
                _canvas->setTextSize( 2 );
                _canvas->drawChar( 10, 40, 'A', 0xF800, 0x0000 ); // Red
                _canvas->drawChar( 34, 40, 'B', 0x07E0, 0x0000 ); // Green
                _canvas->drawChar( 58, 40, 'C', 0x001F, 0x0000 ); // Blue

                log_w( "GLF_ST: Testing drawChar directly" );
            }
            break;

            case Display::GLF_P:
            case Display::GLF_N:
            case Display::GLF_C:
            case Display::GLF_T:
            case Display::GLF_A:
            case Display::GLF_S:
                // Letter glyphs (P, N, C, T, A, S) - draw using text
            {
                char letter = ' ';
                if ( glyphIndex == Display::GLF_P ) {
                    letter = 'P';
                }
                else if ( glyphIndex == Display::GLF_N ) {
                    letter = 'N';
                }
                else if ( glyphIndex == Display::GLF_C ) {
                    letter = 'C';
                }
                else if ( glyphIndex == Display::GLF_T ) {
                    letter = 'T';
                }
                else if ( glyphIndex == Display::GLF_A ) {
                    letter = 'A';
                }
                else if ( glyphIndex == Display::GLF_S ) {
                    letter = 'S';
                }

                // Use FreeSansBold24pt - scale down for smoother rendering
                uint8_t scale = ( _rotation == 1 || _rotation == 3 ) ? 2 : 3;

                _canvas->setFont( &STACSansBold24pt7b );
                _canvas->setTextColor( colorToRGB565( foreground ) );
                _canvas->setTextSize( scale );

                // Remap letter to font position
                char remappedLetter = remapCharToSTACSansFont( letter );

                // Get text bounds for centering
                int16_t x1, y1;
                uint16_t w, h;
                char letterStr[ 2 ] = {remappedLetter, '\0'};
                _canvas->getTextBounds( letterStr, 0, 0, &x1, &y1, &w, &h );

                // Account for x1 offset (important for narrow chars like "1")
                int16_t x = ( currentWidth() - w ) / 2 - x1;
                int16_t y = ( currentHeight() - h ) / 2 + h; // Baseline adjustment

                _canvas->setCursor( x, y );
                _canvas->print( remappedLetter );
            }
            break;

            case Display::GLF_RA:
                // Right arrow
                _canvas->fillTriangle( cx + 30, cy, cx - 20, cy - 25, cx - 20, cy + 25,
                                       colorToRGB565( foreground ) );
                break;

            case Display::GLF_LA:
                // Left arrow
                _canvas->fillTriangle( cx - 30, cy, cx + 20, cy - 25, cx + 20, cy + 25,
                                       colorToRGB565( foreground ) );
                break;

            case Display::GLF_HF:
                // Happy face - simple smiley
                _canvas->drawCircle( cx, cy, 30, colorToRGB565( foreground ) );
                _canvas->fillCircle( cx - 12, cy - 10, 4, colorToRGB565( foreground ) );
                _canvas->fillCircle( cx + 12, cy - 10, 4, colorToRGB565( foreground ) );
                _canvas->drawArc( cx, cy, 20, 15, 30, 150, colorToRGB565( foreground ) );
                break;

            case Display::GLF_DOT:
            case Display::GLF_PO:
                // Center dot
                _canvas->fillCircle( cx, cy, 8, colorToRGB565( foreground ) );
                break;

            case Display::GLF_CORNERS:
                // Four corner dots (handled by pulseCorners, but provide basic render)
            {
                uint8_t sz = 10;
                uint16_t fg = colorToRGB565( foreground );
                uint16_t w = currentWidth();
                uint16_t h = currentHeight();
                _canvas->fillRect( 0, 0, sz, sz, fg );
                _canvas->fillRect( w - sz, 0, sz, sz, fg );
                _canvas->fillRect( 0, h - sz, sz, sz, fg );
                _canvas->fillRect( w - sz, h - sz, sz, sz, fg );
            }
            break;

            case Display::GLF_CBD:
                // Checkerboard pattern (brightness display)
                // Dynamically sized to fit display without clipping
            {
                uint16_t fg = colorToRGB565( foreground );
                uint16_t bg = colorToRGB565( background );
                uint16_t w = currentWidth();
                uint16_t h = currentHeight();

                // Calculate block size for perfect fit (target 5 blocks per smallest dimension)
                const uint8_t TARGET_BLOCKS = 5;
                uint16_t minDim = min( w, h );
                uint8_t blockSize = minDim / TARGET_BLOCKS;

                // Constrain to reasonable range for visibility
                blockSize = constrain( blockSize, 10, 32 );

                // Calculate grid dimensions
                uint8_t cols = w / blockSize;
                uint8_t rows = h / blockSize;

                // Center the grid
                int xOffset = ( w - ( cols * blockSize ) ) / 2;
                int yOffset = ( h - ( rows * blockSize ) ) / 2;

                // Draw centered checkerboard grid
                for ( int row = 0; row < rows; row++ ) {
                    for ( int col = 0; col < cols; col++ ) {
                        bool isForeground = ( col + row ) % 2 == 0;
                        _canvas->fillRect( xOffset + col * blockSize, yOffset + row * blockSize, blockSize, blockSize, isForeground ? fg : bg );
                    }
                }
            }
            break;

            case Display::GLF_FR:
                // Factory reset icon - circular arrow
                drawResetIcon( cx, cy, foreground );
                break;

            case Display::GLF_EN:
            case Display::GLF_EM:
            default:
                // Empty/space or unknown - just show background (already filled)
                break;
        }

        if ( doShow ) {
            show();
        }
    }

    void DisplayTFT::setBrightness( uint8_t brightness, bool doShow ) {
        log_i( "setBrightness: %d", brightness );
        if ( brightness != _brightness ) {
            _brightness = brightness;
            updateBacklight();
        }
        if ( doShow ) {
            show();
        }
    }

    uint8_t DisplayTFT::getBrightness() const {
        return _brightness;
    }

    void DisplayTFT::show() {
        if ( _canvas ) {
            static int flushCount = 0;
            flushCount++;

            uint16_t w = _canvas->width();
            uint16_t h = _canvas->height();

            // Flush canvas to display
            _canvas->flush();
        }
    }

    void DisplayTFT::flash( uint8_t times, uint16_t interval, uint8_t brightness ) {
        // Flash implementation per documentation:
        // - Remove content (OFF) for interval ms
        // - Return content (ON at brightness) for interval ms
        // - Repeat 'times'
        // - Caller manages initial display and final brightness restoration

        for ( uint8_t i = 0; i < times; i++ ) {
            setBrightness( 0, true );        // OFF: Remove content (backlight off)
            delay( interval );
            setBrightness( brightness, true ); // ON: Return content
            delay( interval );
        }
    }

    void DisplayTFT::drawGlyphOverlay( const uint8_t* glyph, color_t color, bool doShow ) {
        if ( !_canvas || !glyph ) {
            if ( doShow ) {
                show();
            }
            return;
        }

        // Extract glyph index from stub glyph data
        uint8_t glyphIndex = glyph[ 0 ];

        // Center coordinates (use sprite dimensions)
        int16_t cx = _canvas->width() / 2;
        int16_t cy = _canvas->height() / 2;

        // For overlay, we draw ON TOP of existing content without clearing
        switch ( glyphIndex ) {
            case Display::GLF_EN:
            case Display::GLF_EM:
                // En/Em space - blank out a rectangle in the center for number overlay
                // This creates a "window" for the brightness number
                // Dynamically sized to match checkerboard block grid
            {
                uint16_t w = _canvas->width();
                uint16_t h = _canvas->height();

                // Calculate block size matching checkerboard (5 blocks per smallest dimension)
                const uint8_t TARGET_BLOCKS = 5;
                uint16_t minDim = min( w, h );
                uint8_t blockSize = minDim / TARGET_BLOCKS;
                blockSize = constrain( blockSize, 10, 32 );

                // Black square as 2×3 checkerboard blocks (aligned to grid)
                int16_t boxW = blockSize * 2;
                int16_t boxH = blockSize * 3;

                _canvas->fillRect( cx - boxW / 2, cy - boxH / 2, boxW, boxH, colorToRGB565( color ) );
            }
            break;

            case Display::GLF_0:
            case Display::GLF_1:
            case Display::GLF_2:
            case Display::GLF_3:
            case Display::GLF_4:
            case Display::GLF_5:
            case Display::GLF_6:
            case Display::GLF_7:
            case Display::GLF_8:
            case Display::GLF_9:
                // Draw digit overlay (for brightness display)
            {
                // Extract digit from glyph index
                uint8_t digit = glyphIndex - Display::GLF_0;

                // Use STACSansBold24pt for digit overlay - scale=1 to fit properly in black rectangle
                uint8_t scale = 1;

                _canvas->setFont( &STACSansBold24pt7b );
                _canvas->setTextColor( colorToRGB565( color ) );
                _canvas->setTextSize( scale );

                // Get text bounds for centering
                char digitStr[ 2 ] = {static_cast<char>( '0' + digit ), '\0'};
                int16_t x1, y1;
                uint16_t w, h;
                _canvas->getTextBounds( digitStr, 0, 0, &x1, &y1, &w, &h );

                // Use same centering formula as drawLargeDigit for consistency
                uint16_t canvasW = _canvas->width();
                uint16_t canvasH = _canvas->height();
                int16_t x = ( canvasW - w ) / 2 - x1;   // Account for x1 offset (important for narrow chars like "1")
                int16_t y = ( canvasH - h ) / 2 + h;    // Baseline adjustment

                _canvas->setCursor( x, y );
                char remappedDigit = remapCharToSTACSansFont( digitStr[ 0 ] );
                _canvas->print( remappedDigit );
            }
            break;

            case Display::GLF_DOT:
            case Display::GLF_PO:
                // Orange square overlay (center power indicator)
                // Dynamically sized as percentage of display
            {
                uint16_t w = _canvas->width();
                uint16_t h = _canvas->height();

                // Size square as 12% of smallest dimension
                const float SQUARE_SIZE_RATIO = 0.12f;
                int16_t minDim = min( w, h );
                int16_t squareSize = minDim * SQUARE_SIZE_RATIO;

                // Ensure minimum size for visibility
                squareSize = max( squareSize, ( int16_t )12 );

                // Make odd for perfect centering
                if ( squareSize % 2 == 0 ) {
                    squareSize++;
                }

                _canvas->fillRect( cx - squareSize / 2, cy - squareSize / 2, squareSize, squareSize, colorToRGB565( color ) );
            }
            break;

            default:
                // Other glyphs - no overlay action
                break;
        }

        if ( doShow ) {
            show();
        }
    }

    void DisplayTFT::pulseCorners( const uint8_t* cornersGlyph, bool state, color_t color ) {
        // Draw corner indicators for autostart mode
        uint16_t rgb565 = state ? colorToRGB565( color ) : 0x0000;
        uint8_t cornerSize = 15;  // Size of corner indicators

        // Draw directly to display in landscape mode, bypass canvas
        if ( _rotation == 1 || _rotation == 3 ) {
            uint16_t w = _gfx->width();
            uint16_t h = _gfx->height();
            // Top-left corner
            _gfx->fillRect( 0, 0, cornerSize, cornerSize, rgb565 );
            // Top-right corner
            _gfx->fillRect( w - cornerSize, 0, cornerSize, cornerSize, rgb565 );
            // Bottom-left corner
            _gfx->fillRect( 0, h - cornerSize, cornerSize, cornerSize, rgb565 );
            // Bottom-right corner
            _gfx->fillRect( w - cornerSize, h - cornerSize, cornerSize, cornerSize, rgb565 );

            return;  // Skip canvas path
        }

        // Portrait mode: use canvas as normal
        if ( _canvas ) {
            uint16_t w = _canvas->width();
            uint16_t h = _canvas->height();
            // Top-left corner
            _canvas->fillRect( 0, 0, cornerSize, cornerSize, rgb565 );
            // Top-right corner
            _canvas->fillRect( w - cornerSize, 0, cornerSize, cornerSize, rgb565 );
            // Bottom-left corner
            _canvas->fillRect( 0, h - cornerSize, cornerSize, cornerSize, rgb565 );
            // Bottom-right corner
            _canvas->fillRect( w - cornerSize, h - cornerSize, cornerSize, cornerSize, rgb565 );

            show();
        }
    }

    void DisplayTFT::pulseDisplay( const uint8_t* glyph, color_t foreground, color_t background,
                                   bool& pulseState, uint8_t normalBrightness, uint8_t dimBrightness ) {
        pulseState = !pulseState;

        // For TFT displays with PWM backlight, use a more dramatic dim level (1/3 of normal)
        // The passed dimBrightness is based on adjacent brightness map levels which are
        // too close together for a noticeable pulse effect on TFT displays
        #if defined(DISPLAY_BACKLIGHT_PWM)
        uint8_t effectiveDim = normalBrightness / 3;
        if ( effectiveDim < 20 ) {
            effectiveDim = 20;    // Minimum visible level
        }
        setBrightness( pulseState ? normalBrightness : effectiveDim, true );
        #else
        setBrightness( pulseState ? normalBrightness : dimBrightness, true );
        #endif
    }

    uint8_t DisplayTFT::getWidth() const {
        return static_cast<uint8_t>( currentWidth() );
    }

    uint8_t DisplayTFT::getHeight() const {
        return static_cast<uint8_t>( currentHeight() );
    }

    uint8_t DisplayTFT::getPixelCount() const {
        // Return a reasonable value for interface compatibility
        // Actual pixel count would overflow uint8_t (135*240 = 32400)
        return 255;
    }

    // ========================================================================
    // TFT-Specific Methods
    // ========================================================================

    void DisplayTFT::drawLargeDigit( uint8_t digit, color_t color, color_t bgColor ) {
        if ( !_canvas || digit > 9 ) {
            return;
        }

        uint16_t fg = colorToRGB565( color );
        uint16_t bg = colorToRGB565( bgColor );

        _canvas->fillScreen( bg );

        // Use STACSansBold24pt scaled down for smooth digit rendering
        uint8_t scale = ( _rotation == 1 || _rotation == 3 ) ? 2 : 3;

        _canvas->setFont( &STACSansBold24pt7b );
        _canvas->setTextColor( fg, bg );
        _canvas->setTextSize( scale );

        // Remap digit character for our contiguous font
        char remappedDigit = remapCharToSTACSansFont( '0' + digit );
        char digitStr[ 2 ] = {remappedDigit, '\0'};

        // Get text bounds for centering (using remapped character)
        int16_t x1, y1;
        uint16_t w, h;
        _canvas->getTextBounds( digitStr, 0, 0, &x1, &y1, &w, &h );

        uint16_t displayW = _canvas->width();
        uint16_t displayH = _canvas->height();
        // Account for x1 offset (important for narrow chars like "1")
        int16_t x = ( displayW - w ) / 2 - x1;
        int16_t y = ( displayH - h ) / 2 + h; // Baseline adjustment

        _canvas->setCursor( x, y );
        _canvas->print( remappedDigit );

        show();
    }

    void DisplayTFT::drawChannelNumber( uint8_t channel, color_t color, color_t bgColor ) {
        if ( !_canvas ) {
            return;
        }

        uint16_t fg = colorToRGB565( color );
        uint16_t bg = colorToRGB565( bgColor );

        _canvas->fillScreen( bg );

        // Format channel number
        char numStr[ 4 ];
        snprintf( numStr, sizeof( numStr ), "%d", channel );
        uint8_t numDigits = ( channel < 10 ) ? 1 : 2;

        // Use STACSansBold24pt - larger size for single digit, smaller for double digit
        uint8_t scale = ( numDigits == 1 ) ? 3 : 2;
        if ( _rotation == 1 || _rotation == 3 ) {
            scale = ( numDigits == 1 ) ? 2 : 1; // Landscape is narrower
        }

        _canvas->setFont( &STACSansBold24pt7b );
        _canvas->setTextColor( fg, bg );
        _canvas->setTextSize( scale );

        // Remap the number string to our contiguous font
        char remappedStr[ 4 ];
        for ( int i = 0; numStr[ i ] != '\0'; i++ ) {
            remappedStr[ i ] = remapCharToSTACSansFont( numStr[ i ] );
        }
        remappedStr[ strlen( numStr ) ] = '\0';

        // Get text bounds for centering (using remapped string)
        int16_t x1, y1;
        uint16_t w, h;
        _canvas->getTextBounds( remappedStr, 0, 0, &x1, &y1, &w, &h );

        uint16_t displayW = _canvas->width();
        uint16_t displayH = _canvas->height();
        // Account for x1 offset (important for narrow chars like "1")
        int16_t x = ( displayW - w ) / 2 - x1;
        int16_t y = ( displayH - h ) / 2 + h; // Baseline adjustment

        _canvas->setCursor( x, y );
        _canvas->print( remappedStr );

        show();
    }

    void DisplayTFT::drawWiFiIcon( int16_t cx, int16_t cy, color_t color, bool connected ) {
        if ( !_canvas ) {
            return;
        }

        uint16_t rgb565 = colorToRGB565( color );

        // Draw WiFi arcs (signal strength indicator)
        // Scale based on rotation: portrait (0,2) = large, landscape (1,3) = smaller to fit 135px
        bool isLandscape = ( _rotation == 1 || _rotation == 3 );
        float scale = isLandscape ? 0.55f : 1.0f;

        int16_t dotY = cy + static_cast<int16_t>( 40 * scale ); // Base dot position
        int16_t dotRadius = static_cast<int16_t>( 10 * scale );
        int16_t baseRadius = static_cast<int16_t>( 25 * scale );
        int16_t radiusStep = static_cast<int16_t>( 20 * scale );
        int16_t arcOffset = static_cast<int16_t>( 15 * scale );
        int16_t lineThickness = isLandscape ? 2 : 2;

        // Draw the base dot
        _canvas->fillCircle( cx, dotY, dotRadius, rgb565 );

        if ( connected ) {
            // Draw three arcs for signal strength
            for ( int i = 0; i < 3; i++ ) {
                int16_t radius = baseRadius + ( i * radiusStep );
                int16_t arcY = dotY - arcOffset;

                // Draw arc using multiple line segments
                for ( float angle = -60; angle <= 60; angle += 3 ) {
                    float rad1 = angle * M_PI / 180.0f;
                    float rad2 = ( angle + 3 ) * M_PI / 180.0f;

                    int16_t x1 = cx + static_cast<int16_t>( radius * sin( rad1 ) );
                    int16_t y1 = arcY - static_cast<int16_t>( radius * cos( rad1 ) );
                    int16_t x2 = cx + static_cast<int16_t>( radius * sin( rad2 ) );
                    int16_t y2 = arcY - static_cast<int16_t>( radius * cos( rad2 ) );

                    // Draw thick arcs (multiple parallel lines)
                    for ( int t = -lineThickness; t <= lineThickness; t++ ) {
                        _canvas->drawLine( x1, y1 + t, x2, y2 + t, rgb565 );
                    }
                }
            }
        }
        else {
            // Searching animation - just show dot pulsing
            // (handled by caller with brightness changes)
        }
    }

    void DisplayTFT::drawConfigIcon( int16_t cx, int16_t cy, color_t color ) {
        if ( !_canvas ) {
            return;
        }

        uint16_t rgb565 = colorToRGB565( color );

        // Draw a larger gear icon for provisioning mode
        int16_t outerR = 50;   // Larger outer radius
        int16_t innerR = 35;   // Larger inner radius
        int16_t holeR = 15;    // Larger center hole
        int16_t teethCount = 8;
        int16_t toothHeight = 15;

        // Draw the main gear body (filled circle)
        _canvas->fillCircle( cx, cy, innerR, rgb565 );

        // Draw the gear teeth as rectangles extending outward
        for ( int i = 0; i < teethCount; i++ ) {
            float angle = i * ( 360.0f / teethCount ) * M_PI / 180.0f;

            // Calculate tooth position
            int16_t tx = cx + static_cast<int16_t>( ( innerR - 5 ) * cos( angle ) );
            int16_t ty = cy + static_cast<int16_t>( ( innerR - 5 ) * sin( angle ) );
            int16_t tx2 = cx + static_cast<int16_t>( outerR * cos( angle ) );
            int16_t ty2 = cy + static_cast<int16_t>( outerR * sin( angle ) );

            // Draw thick tooth line
            for ( int t = -6; t <= 6; t++ ) {
                float perpAngle = angle + M_PI / 2;
                int16_t ox = static_cast<int16_t>( t * cos( perpAngle ) );
                int16_t oy = static_cast<int16_t>( t * sin( perpAngle ) );
                _canvas->drawLine( tx + ox, ty + oy, tx2 + ox, ty2 + oy, rgb565 );
            }
        }

        // Draw the center hole (black)
        _canvas->fillCircle( cx, cy, holeR, 0x0000 ); // 0x0000
    }

    void DisplayTFT::drawUpdateIcon( int16_t cx, int16_t cy, color_t color ) {
        if ( !_canvas ) {
            return;
        }

        uint16_t rgb565 = colorToRGB565( color );

        // Draw a larger download arrow icon for OTA updates
        int16_t arrowWidth = 60;   // Wider arrow head
        int16_t arrowHeight = 70;  // Taller arrow
        int16_t stemWidth = 24;    // Wider stem

        // Arrow stem (vertical rectangle) - starts higher
        _canvas->fillRect( cx - stemWidth / 2, cy - arrowHeight / 2 - 10, stemWidth, arrowHeight * 2 / 3 + 10, rgb565 );

        // Arrow head (triangle pointing down) - larger
        int16_t headTop = cy + arrowHeight / 6 - 5;
        _canvas->fillTriangle(
            cx, cy + arrowHeight / 2 + 5,      // Bottom point
            cx - arrowWidth / 2, headTop,      // Top left
            cx + arrowWidth / 2, headTop,      // Top right
            rgb565
        );

        // Base line - thicker
        _canvas->fillRect( cx - arrowWidth / 2 - 5, cy + arrowHeight / 2 + 15, arrowWidth + 10, 8, rgb565 );
    }

    void DisplayTFT::drawCheckIcon( int16_t cx, int16_t cy, color_t color ) {
        if ( !_canvas ) {
            return;
        }

        // Vector graphics checkmark using filled rectangles for bold appearance
        // Scales better than lines and matches font weight
        uint16_t rgb = colorToRGB565( color );

        // Checkmark consists of two bold strokes (rotated rectangles):
        // 1. Short stroke: down-left to middle (bottom of check)
        // 2. Long stroke: middle to up-right (ascending part)

        // Short stroke (bottom-left arm) - drawn as thick angled rectangle
        // Width: 14 pixels, Length: ~30 pixels, Angle: ~45 degrees down-right
        for ( int i = 0; i < 14; i++ ) {
            _canvas->drawLine( cx - 36 + i, cy + 6, cx - 10 + i, cy + 30, rgb );
        }

        // Long stroke (ascending arm) - drawn as thick angled rectangle
        // Width: 14 pixels, Length: ~80 pixels, Angle: ~60 degrees up-right
        for ( int i = 0; i < 14; i++ ) {
            _canvas->drawLine( cx - 10 + i, cy + 30, cx + 56 + i, cy - 48, rgb );
        }
    }

    void DisplayTFT::drawErrorIcon( int16_t cx, int16_t cy, color_t color ) {
        if ( !_canvas ) {
            return;
        }

        uint16_t rgb565 = colorToRGB565( color );

        // Draw a large, bold X for errors
        int16_t size = 45;       // Large X
        int16_t thickness = 12;  // Thick lines

        // Draw the X with thick lines
        for ( int t = -thickness / 2; t <= thickness / 2; t++ ) {
            // Main diagonal (\)
            _canvas->drawLine( cx - size + t, cy - size, cx + size + t, cy + size, rgb565 );
            _canvas->drawLine( cx - size, cy - size + t, cx + size, cy + size + t, rgb565 );
            // Anti-diagonal (/)
            _canvas->drawLine( cx + size + t, cy - size, cx - size + t, cy + size, rgb565 );
            _canvas->drawLine( cx + size, cy - size + t, cx - size, cy + size + t, rgb565 );
        }
    }

    void DisplayTFT::drawQuestionIcon( int16_t cx, int16_t cy, color_t color ) {
        if ( !_canvas ) {
            return;
        }

        uint16_t rgb565 = colorToRGB565( color );

        // Use larger scale to match the prominence of error X icon
        // Portrait (0,2): scale 3, Landscape (1,3): scale 2
        uint8_t scale = ( _rotation == 1 || _rotation == 3 ) ? 2 : 3;

        // Draw question mark using STACSansBold24pt font
        _canvas->setFont( &STACSansBold24pt7b );
        _canvas->setTextColor( rgb565 );
        _canvas->setTextSize( scale );

        // Get text bounds for centering
        char qMark = remapCharToSTACSansFont( '?' );
        char qStr[ 2 ] = {qMark, '\0'};
        int16_t x1, y1;
        uint16_t w, h;
        _canvas->getTextBounds( qStr, 0, 0, &x1, &y1, &w, &h );

        // Center the question mark
        int16_t x = cx - ( w / 2 ) - x1;
        int16_t y = cy + ( h / 2 );

        _canvas->setCursor( x, y );
        _canvas->print( qStr );
    }

    void DisplayTFT::drawResetIcon( int16_t cx, int16_t cy, color_t color ) {
        if ( !_canvas ) {
            return;
        }

        uint16_t rgb565 = colorToRGB565( color );

        // Scale based on rotation: portrait (0,2) = large, landscape (1,3) = smaller to fit 135px
        bool isLandscape = ( _rotation == 1 || _rotation == 3 );
        float scale = isLandscape ? 0.65f : 1.0f;

        // Circular arrow dimensions
        int16_t outerR = static_cast<int16_t>( 45 * scale ); // Outer radius of arc
        int16_t innerR = static_cast<int16_t>( 35 * scale ); // Inner radius - thinner arc (was 25)
        int16_t arrowLen = static_cast<int16_t>( 28 * scale ); // Arrow length (slightly longer)
        int16_t arrowWid = static_cast<int16_t>( 26 * scale ); // Arrow width - wider base for visibility

        // Draw the circular arc with gap on the LEFT side
        // LovyanGFX: angles go clockwise from 3 o'clock (0°)
        // Gap on left: arc from 225° to 495° (135° + 360°)
        _canvas->fillArc( cx, cy, outerR, innerR, 225, 495, rgb565 );

        // Arrow at the END of arc (135°), with BASE aligned to arc end
        // Tangent at 135° points toward 225° (down-left, clockwise along arc)
        float endAngle = 135.0f * M_PI / 180.0f;
        int16_t midR = ( outerR + innerR ) / 2;

        // Arc end point (center of arc thickness)
        int16_t arcEndX = cx + static_cast<int16_t>( midR * cos( endAngle ) );
        int16_t arcEndY = cy + static_cast<int16_t>( midR * sin( endAngle ) );

        // Tangent direction (225 degrees - pointing down-left along arc flow)
        float tangentAngle = ( 135.0f + 90.0f ) * M_PI / 180.0f; // 225 degrees

        // Perpendicular to tangent for base spread (135° and 315°)
        float perpAngle1 = ( 225.0f - 90.0f ) * M_PI / 180.0f; // 135 degrees
        float perpAngle2 = ( 225.0f + 90.0f ) * M_PI / 180.0f; // 315 degrees

        // BASE of arrow at arc end point, spread perpendicular to tangent
        int16_t base1X = arcEndX + static_cast<int16_t>( arrowWid * 0.5f * cos( perpAngle1 ) );
        int16_t base1Y = arcEndY + static_cast<int16_t>( arrowWid * 0.5f * sin( perpAngle1 ) );
        int16_t base2X = arcEndX + static_cast<int16_t>( arrowWid * 0.5f * cos( perpAngle2 ) );
        int16_t base2Y = arcEndY + static_cast<int16_t>( arrowWid * 0.5f * sin( perpAngle2 ) );

        // TIP of arrow extends from arc end in tangent direction (down-left)
        int16_t tipX = arcEndX + static_cast<int16_t>( arrowLen * cos( tangentAngle ) );
        int16_t tipY = arcEndY + static_cast<int16_t>( arrowLen * sin( tangentAngle ) );

        _canvas->fillTriangle( tipX, tipY, base1X, base1Y, base2X, base2Y, rgb565 );
    }

    void DisplayTFT::drawTallyFrame( color_t color, uint8_t thickness ) {
        if ( !_canvas ) {
            return;
        }

        uint16_t rgb565 = colorToRGB565( color );
        uint16_t w = _canvas->width();
        uint16_t h = _canvas->height();

        // Draw frame around the display edge
        for ( uint8_t i = 0; i < thickness; i++ ) {
            _canvas->drawRect( i, i, w - ( 2 * i ), h - ( 2 * i ), rgb565 );
        }

        show();
    }

    void DisplayTFT::setRotation( uint8_t rotation ) {
        log_w( "setRotation(%d) called! Current rotation: %d", rotation, _rotation );
        _rotation = rotation % 4;
        if ( _gfx ) {
            _rotation = rotation & 3;

            // Set rotation on display
            _gfx->setRotation( _rotation );
            _gfx->fillScreen( 0x0000 );

            // Recreate canvas with new dimensions
            if ( _canvas ) {
                delete _canvas;
                // Canvas must match rotated display dimensions
                _canvas = new Arduino_Canvas( _gfx->width(), _gfx->height(), _gfx, 0, 0, 0 );
                _canvas->begin();
                _canvas->fillScreen( 0x0000 );
                log_i( "Canvas recreated for rotation %d: %dx%d", _rotation, _gfx->width(), _gfx->height() );
            }
        }
    }

    uint8_t DisplayTFT::getRotation() const {
        return _rotation;
    }

    void DisplayTFT::setOrientationRotation( Orientation orientation ) {
        // Map device rotation to TFT rotation values
        // LovyanGFX rotation: 0=0\u00b0, 1=90\u00b0 CW, 2=180\u00b0, 3=270\u00b0 CW
        uint8_t rotation;
        switch ( orientation ) {
            case Orientation::ROTATE_0:
                rotation = 0;  // 0\u00b0 rotation
                break;
            case Orientation::ROTATE_90:
                rotation = 1;  // 90\u00b0 CW rotation
                break;
            case Orientation::ROTATE_180:
                rotation = 2;  // 180\u00b0 rotation
                break;
            case Orientation::ROTATE_270:
                rotation = 3;  // 270\u00b0 CW rotation
                break;
            case Orientation::FLAT:
            case Orientation::UNKNOWN:
            default:
                rotation = 0;  // Default to 0\u00b0
                break;
        }
        // Apply board-specific rotation offset (modulo 4 to keep in valid range)
        rotation = ( rotation + TFT_ROTATION_OFFSET ) % 4;
        setRotation( rotation );
        log_i( "Display rotation set to %d for orientation %d (offset %d)", rotation, static_cast<int>( orientation ), TFT_ROTATION_OFFSET );
    }

    // ========================================================================
    // Private Helper Methods
    // ========================================================================

    uint16_t DisplayTFT::colorToRGB565( color_t color ) const {
        // Convert 24-bit RGB to 16-bit RGB565
        uint8_t r = ( color >> 16 ) & 0xFF;
        uint8_t g = ( color >> 8 ) & 0xFF;
        uint8_t b = color & 0xFF;

        return ( ( r & 0xF8 ) << 8 ) | ( ( g & 0xFC ) << 3 ) | ( b >> 3 );
    }

    // Helper to get current width accounting for rotation
    inline uint16_t DisplayTFT::currentWidth() const {
        return _gfx ? _gfx->width() : _width;
    }

    // Helper to get current height accounting for rotation
    inline uint16_t DisplayTFT::currentHeight() const {
        return _gfx ? _gfx->height() : _height;
    }

    // Note: remapCharToFont() has been replaced by global remapCharToSTACSansFont()
    // defined in STACSansBold24pt7b.h for use anywhere in the codebase

    void DisplayTFT::updateBacklight() {
        #if defined(USE_AXP192_PMU)
        // Use AXP192 PMU to control backlight brightness via LDO2
        _pmu.setBacklight( _brightness );
        #elif defined(DISPLAY_BACKLIGHT_PWM) && defined(TFT_BL)
        // Use analogWrite for backlight PWM control (simpler, more compatible)
        // Invert brightness if backlight is active-low (LOW = ON, HIGH = OFF)
        #if defined(TFT_BACKLIGHT_ON) && (TFT_BACKLIGHT_ON == LOW)
        // Active-low: higher PWM duty = more HIGH time = dimmer
        // So invert: 255 - brightness
        analogWrite( TFT_BL, 255 - _brightness );
        #else
        // Active-high: normal PWM mapping
        analogWrite( TFT_BL, _brightness );
        #endif
        #endif
        log_d( "Backlight set to: %d", _brightness );
    }

} // namespace Display
