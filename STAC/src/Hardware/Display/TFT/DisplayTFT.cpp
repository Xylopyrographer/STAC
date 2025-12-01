/**
 * @file DisplayTFT.cpp
 * @brief TFT display implementation using LovyanGFX
 */

#include "Hardware/Display/TFT/DisplayTFT.h"
#include "Hardware/Display/TFT/LGFX_STAC.h"
#include "Hardware/Display/TFT/GlyphsTFT.h"
#include <cmath>

// Backlight control method selection based on board config
#if defined(DISPLAY_BACKLIGHT_PMU)
    #define USE_AXP192_PMU
#elif defined(DISPLAY_BACKLIGHT_PWM)
    #define USE_LGFX_BACKLIGHT
#endif
// If neither defined, no software backlight control (DISPLAY_BACKLIGHT_NONE)

// Debug LED helper - uses GPIO status LED if configured
// Note: Only supports GPIO LEDs for simple debug blinking, not addressable LEDs
#if HAS_STATUS_LED && defined(PIN_STATUS_LED) && defined(STATUS_LED_TYPE_GPIO)
    #if defined(STATUS_LED_ACTIVE_LOW) && STATUS_LED_ACTIVE_LOW
        #define DBG_LED_ON  LOW
        #define DBG_LED_OFF HIGH
    #else
        #define DBG_LED_ON  HIGH
        #define DBG_LED_OFF LOW
    #endif
    inline void dbgBlink(int count, int onMs = 100, int offMs = 100) {
        for (int i = 0; i < count; i++) {
            digitalWrite(PIN_STATUS_LED, DBG_LED_ON);
            delay(onMs);
            digitalWrite(PIN_STATUS_LED, DBG_LED_OFF);
            delay(offMs);
        }
    }
#else
    inline void dbgBlink(int, int = 100, int = 100) {}
#endif

// Default rotation can be overridden in board config
#ifndef TFT_DEFAULT_ROTATION
    #define TFT_DEFAULT_ROTATION 0
#endif

// Rotation offset for boards that need rotation adjustment
#ifndef TFT_ROTATION_OFFSET
    #define TFT_ROTATION_OFFSET 0
#endif

namespace Display {

    DisplayTFT::DisplayTFT(uint16_t width, uint16_t height)
        : _lcd(nullptr)
        , _sprite(nullptr)
        #if defined(USE_AXP192_PMU)
        , _pmu()
        #endif
        , _width(width)
        , _height(height)
        , _brightness(128)
        , _rotation(TFT_DEFAULT_ROTATION)
    {
    }

    DisplayTFT::~DisplayTFT() {
        if (_sprite) {
            _sprite->deleteSprite();
            delete _sprite;
        }
        if (_lcd) {
            delete _lcd;
        }
    }

    bool DisplayTFT::begin() {
        log_i("DisplayTFT::begin() - starting initialization...");
        dbgBlink(1);  // 1 blink = starting init
        
        #if defined(USE_AXP192_PMU)
            // Initialize the AXP192 PMU first (controls LCD power and backlight)
            log_i("Initializing AXP192 PMU...");
            if (!_pmu.begin()) {
                log_e("Failed to initialize AXP192 PMU");
                return false;
            }
            log_i("AXP192 PMU initialized");
            dbgBlink(2);  // 2 blinks = PMU done
            
            // Small delay for power rail stabilization
            delay(50);
        #endif
        
        log_i("Creating LGFX display...");
        dbgBlink(3);  // 3 blinks = creating LCD
        
        // Create and initialize the display (unified LGFX class configured via board defines)
        _lcd = new LGFX_STAC();
        if (!_lcd) {
            return false;
        }

        log_i("Calling _lcd->init()...");
        dbgBlink(4);  // 4 blinks = about to init LCD
        
        _lcd->init();
        _lcd->setSwapBytes(true);  // Required for correct RGB565 byte order
        
        dbgBlink(5);  // 5 blinks = LCD init done
        log_i("LCD init complete, setting rotation...");
        
        _lcd->setRotation(_rotation);
        
        // Create sprite for double-buffering (flicker-free updates)
        _sprite = new LGFX_Sprite(_lcd);
        if (!_sprite) {
            return false;
        }
        
        dbgBlink(6);  // 6 blinks = sprite created
        
        // Create sprite with display dimensions
        _sprite->createSprite(_width, _height);
        _sprite->setSwapBytes(true);  // For correct RGB565 byte order
        
        // Initialize backlight (full brightness for now)
        _brightness = 255;
        updateBacklight();
        
        // Clear display
        clear(true);
        
        log_i("TFT Display initialized: %dx%d", _width, _height);
        return true;
    }

    void DisplayTFT::clear(bool doShow) {
        if (_sprite) {
            _sprite->fillSprite(TFT_BLACK);
            if (doShow) {
                show();
            }
        }
    }

    void DisplayTFT::setPixel(uint8_t position, color_t color, bool doShow) {
        // For TFT, we don't typically use single-pixel operations
        // Map position to x,y for compatibility
        uint16_t w = currentWidth();
        uint8_t x = position % w;
        uint8_t y = position / w;
        setPixelXY(x, y, color, doShow);
    }

    void DisplayTFT::setPixelXY(uint8_t x, uint8_t y, color_t color, bool doShow) {
        if (_sprite && x < currentWidth() && y < currentHeight()) {
            _sprite->drawPixel(x, y, colorToRGB565(color));
            if (doShow) {
                show();
            }
        }
    }

    void DisplayTFT::fill(color_t color, bool doShow) {
        if (_sprite) {
            _sprite->fillSprite(colorToRGB565(color));
            if (doShow) {
                show();
            }
        }
    }

    void DisplayTFT::drawGlyph(const uint8_t* glyph, color_t foreground, color_t background, bool doShow) {
        if (!_sprite || !glyph) return;
        
        // Extract glyph index from the stub glyph data
        // Each TFT "glyph" is a 1-byte array containing its index
        uint8_t glyphIndex = glyph[0];
        
        log_i("drawGlyph: index=%d, fg=0x%06X, bg=0x%06X", glyphIndex, foreground, background);
        
        // Fill background first
        fill(background, false);
        
        // Center coordinates for icon drawing (use rotated dimensions)
        int16_t cx = currentWidth() / 2;
        int16_t cy = currentHeight() / 2;
        
        // Render based on glyph index
        switch (glyphIndex) {
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
                drawLargeDigit(glyphIndex, foreground, background);
                return;  // drawLargeDigit calls show()
                
            case Display::GLF_WIFI:
                drawWiFiIcon(cx, cy, foreground, true);
                break;
                
            case Display::GLF_CFG:
                drawConfigIcon(cx, cy, foreground);
                break;
                
            case Display::GLF_UD:
                drawUpdateIcon(cx, cy, foreground);
                break;
                
            case Display::GLF_CK:
                drawCheckIcon(cx, cy, foreground);
                break;
                
            case Display::GLF_BX:
            case Display::GLF_X:
                drawErrorIcon(cx, cy, foreground);
                break;
                
            case Display::GLF_QM:
                drawQuestionIcon(cx, cy, foreground);
                break;
                
            case Display::GLF_FM:
                // Frame glyph - draw a border
                drawTallyFrame(foreground, 8);
                return;  // drawTallyFrame calls show()
                
            case Display::GLF_DF:
                // Dotted frame (unselected tally) - purple on black checkerboard
                // Dynamically sized based on current orientation
                {
                    uint16_t fg = colorToRGB565(foreground);
                    uint16_t bg = colorToRGB565(background);
                    const uint8_t blockSize = 24;
                    uint16_t w = currentWidth();
                    uint16_t h = currentHeight();
                    // Calculate grid dimensions to fill display
                    uint8_t cols = w / blockSize;  // e.g., 135/24=5 or 240/24=10
                    uint8_t rows = h / blockSize;  // e.g., 240/24=10 or 135/24=5
                    // Center the grid
                    int xOffset = (w - (cols * blockSize)) / 2;
                    int yOffset = (h - (rows * blockSize)) / 2;
                    // Fill background first
                    _sprite->fillSprite(bg);
                    // Draw centered checkerboard grid
                    for (int row = 0; row < rows; row++) {
                        for (int col = 0; col < cols; col++) {
                            bool isForeground = (col + row) % 2 == 0;
                            if (isForeground) {
                                _sprite->fillRect(xOffset + col * blockSize, yOffset + row * blockSize, blockSize, blockSize, fg);
                            }
                        }
                    }
                }
                break;
                
            case Display::GLF_ST:
                // Smart Tally icon - draw "ST" text large
                _sprite->setTextColor(colorToRGB565(foreground), colorToRGB565(background));
                _sprite->setTextDatum(MC_DATUM);
                _sprite->setFont(&fonts::Font4);
                _sprite->setTextSize(3);
                _sprite->drawString("ST", cx, cy);
                break;
                
            case Display::GLF_C:
            case Display::GLF_T:
            case Display::GLF_A:
            case Display::GLF_S:
            case Display::GLF_P:
                // Single letter glyphs - draw large using a proper font (not 7-segment)
                {
                    char letter = ' ';
                    if (glyphIndex == Display::GLF_C) letter = 'C';
                    else if (glyphIndex == Display::GLF_T) letter = 'T';
                    else if (glyphIndex == Display::GLF_A) letter = 'A';
                    else if (glyphIndex == Display::GLF_S) letter = 'S';
                    else if (glyphIndex == Display::GLF_P) letter = 'P';
                    
                    _sprite->setTextColor(colorToRGB565(foreground), colorToRGB565(background));
                    _sprite->setTextDatum(MC_DATUM);
                    // Use FreeSansBold for proper letter rendering (Font7 is digits-only)
                    _sprite->setFont(&fonts::FreeSansBold24pt7b);
                    _sprite->setTextSize(2);
                    char str[2] = {letter, '\0'};
                    _sprite->drawString(str, cx, cy);
                }
                break;
                
            case Display::GLF_RA:
                // Right arrow
                _sprite->fillTriangle(cx + 30, cy, cx - 20, cy - 25, cx - 20, cy + 25, 
                                      colorToRGB565(foreground));
                break;
                
            case Display::GLF_LA:
                // Left arrow
                _sprite->fillTriangle(cx - 30, cy, cx + 20, cy - 25, cx + 20, cy + 25,
                                      colorToRGB565(foreground));
                break;
                
            case Display::GLF_HF:
                // Happy face - simple smiley
                _sprite->drawCircle(cx, cy, 30, colorToRGB565(foreground));
                _sprite->fillCircle(cx - 12, cy - 10, 4, colorToRGB565(foreground));
                _sprite->fillCircle(cx + 12, cy - 10, 4, colorToRGB565(foreground));
                _sprite->drawArc(cx, cy, 20, 15, 30, 150, colorToRGB565(foreground));
                break;
                
            case Display::GLF_DOT:
            case Display::GLF_PO:
                // Center dot
                _sprite->fillCircle(cx, cy, 8, colorToRGB565(foreground));
                break;
                
            case Display::GLF_CORNERS:
                // Four corner dots (handled by pulseCorners, but provide basic render)
                {
                    uint8_t sz = 10;
                    uint16_t fg = colorToRGB565(foreground);
                    uint16_t w = currentWidth();
                    uint16_t h = currentHeight();
                    _sprite->fillRect(0, 0, sz, sz, fg);
                    _sprite->fillRect(w - sz, 0, sz, sz, fg);
                    _sprite->fillRect(0, h - sz, sz, sz, fg);
                    _sprite->fillRect(w - sz, h - sz, sz, sz, fg);
                }
                break;
                
            case Display::GLF_CBD:
                // Checkerboard pattern
                {
                    uint16_t fg = colorToRGB565(foreground);
                    uint16_t bg = colorToRGB565(background);
                    uint8_t blockSize = 20;
                    uint16_t w = currentWidth();
                    uint16_t h = currentHeight();
                    for (int y = 0; y < h; y += blockSize) {
                        for (int x = 0; x < w; x += blockSize) {
                            bool isWhite = ((x / blockSize) + (y / blockSize)) % 2 == 0;
                            _sprite->fillRect(x, y, blockSize, blockSize, isWhite ? fg : bg);
                        }
                    }
                }
                break;
                
            case Display::GLF_FR:
                // Factory reset icon - circular arrow
                drawResetIcon(cx, cy, foreground);
                break;
                
            case Display::GLF_P_CANCEL:
                // P with cancel slash - PMode cancel indicator
                {
                    _sprite->setTextColor(colorToRGB565(foreground), colorToRGB565(background));
                    _sprite->setTextDatum(MC_DATUM);
                    _sprite->setFont(&fonts::FreeSansBold24pt7b);
                    _sprite->setTextSize(2);
                    _sprite->drawString("P", cx, cy);
                    // Draw diagonal slash through the P
                    uint16_t slashColor = colorToRGB565(foreground);
                    int16_t offset = 40;  // Half-width of slash area
                    for (int t = -3; t <= 3; t++) {  // Line thickness
                        _sprite->drawLine(cx - offset + t, cy + offset, cx + offset + t, cy - offset, slashColor);
                    }
                }
                break;
                
            case Display::GLF_EN:
            case Display::GLF_EM:
            default:
                // Empty/space or unknown - just show background (already filled)
                break;
        }
        
        if (doShow) {
            show();
        }
    }

    void DisplayTFT::setBrightness(uint8_t brightness, bool doShow) {
        log_i("setBrightness: %d", brightness);
        _brightness = brightness;
        updateBacklight();
        if (doShow) {
            show();
        }
    }

    uint8_t DisplayTFT::getBrightness() const {
        return _brightness;
    }

    void DisplayTFT::show() {
        if (_sprite && _lcd) {
            _sprite->pushSprite(0, 0);
        }
    }

    void DisplayTFT::flash(uint8_t times, uint16_t interval, uint8_t brightness) {
        uint8_t originalBrightness = _brightness;
        
        for (uint8_t i = 0; i < times; i++) {
            setBrightness(brightness, true);
            delay(interval);
            setBrightness(0, true);
            delay(interval);
        }
        
        setBrightness(originalBrightness, true);
    }

    void DisplayTFT::drawGlyphOverlay(const uint8_t* glyph, color_t color, bool doShow) {
        if (!_sprite || !glyph) {
            if (doShow) show();
            return;
        }
        
        // Extract glyph index from stub glyph data
        uint8_t glyphIndex = glyph[0];
        
        // Center coordinates (use rotated dimensions)
        int16_t cx = currentWidth() / 2;
        int16_t cy = currentHeight() / 2;
        
        // For overlay, we draw ON TOP of existing content without clearing
        switch (glyphIndex) {
            case Display::GLF_EN:
            case Display::GLF_EM:
                // En/Em space - blank out a rectangle in the center for number overlay
                // This creates a "window" for the brightness number
                {
                    int16_t boxW = 70;  // Width of center blank area
                    int16_t boxH = 80;  // Height of center blank area
                    _sprite->fillRect(cx - boxW/2, cy - boxH/2, boxW, boxH, colorToRGB565(color));
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
                    _sprite->setTextColor(colorToRGB565(color));
                    _sprite->setTextDatum(MC_DATUM);
                    _sprite->setFont(&fonts::FreeSansBold24pt7b);
                    _sprite->setTextSize(2);
                    char digitStr[2] = {static_cast<char>('0' + glyphIndex), '\0'};
                    _sprite->drawString(digitStr, cx, cy);
                }
                break;
                
            default:
                // Other glyphs - no overlay action
                break;
        }
        
        if (doShow) {
            show();
        }
    }

    void DisplayTFT::pulseCorners(const uint8_t* cornersGlyph, bool state, color_t color) {
        // Draw corner indicators for autostart mode
        uint16_t rgb565 = state ? colorToRGB565(color) : TFT_BLACK;
        uint8_t cornerSize = 15;  // Size of corner indicators
        
        if (_sprite) {
            uint16_t w = currentWidth();
            uint16_t h = currentHeight();
            // Top-left corner
            _sprite->fillRect(0, 0, cornerSize, cornerSize, rgb565);
            // Top-right corner
            _sprite->fillRect(w - cornerSize, 0, cornerSize, cornerSize, rgb565);
            // Bottom-left corner
            _sprite->fillRect(0, h - cornerSize, cornerSize, cornerSize, rgb565);
            // Bottom-right corner
            _sprite->fillRect(w - cornerSize, h - cornerSize, cornerSize, cornerSize, rgb565);
            
            show();
        }
    }

    void DisplayTFT::pulseDisplay(const uint8_t* glyph, color_t foreground, color_t background,
                                   bool& pulseState, uint8_t normalBrightness, uint8_t dimBrightness) {
        pulseState = !pulseState;
        
        // For TFT displays with PWM backlight, use a more dramatic dim level (1/3 of normal)
        // The passed dimBrightness is based on adjacent brightness map levels which are
        // too close together for a noticeable pulse effect on TFT displays
        #if defined(DISPLAY_BACKLIGHT_PWM)
            uint8_t effectiveDim = normalBrightness / 3;
            if (effectiveDim < 20) effectiveDim = 20;  // Minimum visible level
            setBrightness(pulseState ? normalBrightness : effectiveDim, true);
        #else
            setBrightness(pulseState ? normalBrightness : dimBrightness, true);
        #endif
    }

    uint8_t DisplayTFT::getWidth() const {
        return static_cast<uint8_t>(currentWidth());
    }

    uint8_t DisplayTFT::getHeight() const {
        return static_cast<uint8_t>(currentHeight());
    }

    uint8_t DisplayTFT::getPixelCount() const {
        // Return a reasonable value for interface compatibility
        // Actual pixel count would overflow uint8_t (135*240 = 32400)
        return 255;
    }

    // ========================================================================
    // TFT-Specific Methods
    // ========================================================================

    void DisplayTFT::drawLargeDigit(uint8_t digit, color_t color, color_t bgColor) {
        if (!_sprite || digit > 9) return;
        
        uint16_t fg = colorToRGB565(color);
        uint16_t bg = colorToRGB565(bgColor);
        
        _sprite->fillSprite(bg);
        
        // Use FreeSansBold for consistent look with letters
        _sprite->setTextColor(fg, bg);
        _sprite->setTextDatum(MC_DATUM);  // Middle-center alignment
        _sprite->setFont(&fonts::FreeSansBold24pt7b);
        _sprite->setTextSize(2);
        
        char digitStr[2] = {static_cast<char>('0' + digit), '\0'};
        _sprite->drawString(digitStr, currentWidth() / 2, currentHeight() / 2);
        
        show();
    }

    void DisplayTFT::drawChannelNumber(uint8_t channel, color_t color, color_t bgColor) {
        if (!_sprite) return;
        
        uint16_t fg = colorToRGB565(color);
        uint16_t bg = colorToRGB565(bgColor);
        
        _sprite->fillSprite(bg);
        
        _sprite->setTextColor(fg, bg);
        _sprite->setTextDatum(MC_DATUM);
        _sprite->setTextSize(1);
        
        if (channel < 10) {
            // Single digit - use larger font
            _sprite->setFont(&fonts::Font7);
        } else {
            // Two digits - use slightly smaller font
            _sprite->setFont(&fonts::Font6);
        }
        
        char numStr[4];
        snprintf(numStr, sizeof(numStr), "%d", channel);
        _sprite->drawString(numStr, currentWidth() / 2, currentHeight() / 2);
        
        show();
    }

    void DisplayTFT::drawWiFiIcon(int16_t cx, int16_t cy, color_t color, bool connected) {
        if (!_sprite) return;
        
        uint16_t rgb565 = colorToRGB565(color);
        
        // Draw WiFi arcs (signal strength indicator)
        // Scale based on rotation: portrait (0,2) = large, landscape (1,3) = smaller to fit 135px
        bool isLandscape = (_rotation == 1 || _rotation == 3);
        float scale = isLandscape ? 0.55f : 1.0f;
        
        int16_t dotY = cy + static_cast<int16_t>(40 * scale);  // Base dot position
        int16_t dotRadius = static_cast<int16_t>(10 * scale);
        int16_t baseRadius = static_cast<int16_t>(25 * scale);
        int16_t radiusStep = static_cast<int16_t>(20 * scale);
        int16_t arcOffset = static_cast<int16_t>(15 * scale);
        int16_t lineThickness = isLandscape ? 2 : 2;
        
        // Draw the base dot
        _sprite->fillCircle(cx, dotY, dotRadius, rgb565);
        
        if (connected) {
            // Draw three arcs for signal strength
            for (int i = 0; i < 3; i++) {
                int16_t radius = baseRadius + (i * radiusStep);
                int16_t arcY = dotY - arcOffset;
                
                // Draw arc using multiple line segments
                for (float angle = -60; angle <= 60; angle += 3) {
                    float rad1 = angle * M_PI / 180.0f;
                    float rad2 = (angle + 3) * M_PI / 180.0f;
                    
                    int16_t x1 = cx + static_cast<int16_t>(radius * sin(rad1));
                    int16_t y1 = arcY - static_cast<int16_t>(radius * cos(rad1));
                    int16_t x2 = cx + static_cast<int16_t>(radius * sin(rad2));
                    int16_t y2 = arcY - static_cast<int16_t>(radius * cos(rad2));
                    
                    // Draw thick arcs (multiple parallel lines)
                    for (int t = -lineThickness; t <= lineThickness; t++) {
                        _sprite->drawLine(x1, y1 + t, x2, y2 + t, rgb565);
                    }
                }
            }
        } else {
            // Searching animation - just show dot pulsing
            // (handled by caller with brightness changes)
        }
    }

    void DisplayTFT::drawConfigIcon(int16_t cx, int16_t cy, color_t color) {
        if (!_sprite) return;
        
        uint16_t rgb565 = colorToRGB565(color);
        
        // Draw a larger gear icon for provisioning mode
        int16_t outerR = 50;   // Larger outer radius
        int16_t innerR = 35;   // Larger inner radius
        int16_t holeR = 15;    // Larger center hole
        int16_t teethCount = 8;
        int16_t toothHeight = 15;
        
        // Draw the main gear body (filled circle)
        _sprite->fillCircle(cx, cy, innerR, rgb565);
        
        // Draw the gear teeth as rectangles extending outward
        for (int i = 0; i < teethCount; i++) {
            float angle = i * (360.0f / teethCount) * M_PI / 180.0f;
            
            // Calculate tooth position
            int16_t tx = cx + static_cast<int16_t>((innerR - 5) * cos(angle));
            int16_t ty = cy + static_cast<int16_t>((innerR - 5) * sin(angle));
            int16_t tx2 = cx + static_cast<int16_t>(outerR * cos(angle));
            int16_t ty2 = cy + static_cast<int16_t>(outerR * sin(angle));
            
            // Draw thick tooth line
            for (int t = -6; t <= 6; t++) {
                float perpAngle = angle + M_PI/2;
                int16_t ox = static_cast<int16_t>(t * cos(perpAngle));
                int16_t oy = static_cast<int16_t>(t * sin(perpAngle));
                _sprite->drawLine(tx + ox, ty + oy, tx2 + ox, ty2 + oy, rgb565);
            }
        }
        
        // Draw the center hole (black)
        _sprite->fillCircle(cx, cy, holeR, TFT_BLACK);
    }

    void DisplayTFT::drawUpdateIcon(int16_t cx, int16_t cy, color_t color) {
        if (!_sprite) return;
        
        uint16_t rgb565 = colorToRGB565(color);
        
        // Draw a larger download arrow icon for OTA updates
        int16_t arrowWidth = 60;   // Wider arrow head
        int16_t arrowHeight = 70;  // Taller arrow
        int16_t stemWidth = 24;    // Wider stem
        
        // Arrow stem (vertical rectangle) - starts higher
        _sprite->fillRect(cx - stemWidth/2, cy - arrowHeight/2 - 10, stemWidth, arrowHeight * 2/3 + 10, rgb565);
        
        // Arrow head (triangle pointing down) - larger
        int16_t headTop = cy + arrowHeight/6 - 5;
        _sprite->fillTriangle(
            cx, cy + arrowHeight/2 + 5,        // Bottom point
            cx - arrowWidth/2, headTop,        // Top left
            cx + arrowWidth/2, headTop,        // Top right
            rgb565
        );
        
        // Base line - thicker
        _sprite->fillRect(cx - arrowWidth/2 - 5, cy + arrowHeight/2 + 15, arrowWidth + 10, 8, rgb565);
    }

    void DisplayTFT::drawCheckIcon(int16_t cx, int16_t cy, color_t color) {
        if (!_sprite) return;
        
        uint16_t rgb565 = colorToRGB565(color);
        
        // Scale based on rotation: portrait (0,2) = large, landscape (1,3) = smaller to fit 135px
        bool isLandscape = (_rotation == 1 || _rotation == 3);
        float scale = isLandscape ? 0.55f : 1.0f;
        
        int16_t size = static_cast<int16_t>(50 * scale);       // Overall size
        int16_t thickness = static_cast<int16_t>(12 * scale);  // Line thickness
        int16_t offset = static_cast<int16_t>(10 * scale);     // Long leg offset
        
        // Short leg of checkmark (going down-right)
        int16_t x1 = cx - size/2;
        int16_t y1 = cy;
        int16_t x2 = cx - size/6;
        int16_t y2 = cy + size/2;
        
        // Long leg of checkmark (going up-right)
        int16_t x3 = cx + size/2 + offset;
        int16_t y3 = cy - size/2;
        
        // Draw thick lines using multiple parallel lines
        for (int t = -thickness/2; t <= thickness/2; t++) {
            _sprite->drawLine(x1 + t, y1, x2 + t, y2, rgb565);
            _sprite->drawLine(x2, y2 + t, x3, y3 + t, rgb565);
        }
    }

    void DisplayTFT::drawErrorIcon(int16_t cx, int16_t cy, color_t color) {
        if (!_sprite) return;
        
        uint16_t rgb565 = colorToRGB565(color);
        
        // Draw a large, bold X for errors
        int16_t size = 45;       // Large X
        int16_t thickness = 12;  // Thick lines
        
        // Draw the X with thick lines
        for (int t = -thickness/2; t <= thickness/2; t++) {
            // Main diagonal (\)
            _sprite->drawLine(cx - size + t, cy - size, cx + size + t, cy + size, rgb565);
            _sprite->drawLine(cx - size, cy - size + t, cx + size, cy + size + t, rgb565);
            // Anti-diagonal (/)
            _sprite->drawLine(cx + size + t, cy - size, cx - size + t, cy + size, rgb565);
            _sprite->drawLine(cx + size, cy - size + t, cx - size, cy + size + t, rgb565);
        }
    }

    void DisplayTFT::drawQuestionIcon(int16_t cx, int16_t cy, color_t color) {
        if (!_sprite) return;
        
        uint16_t rgb565 = colorToRGB565(color);
        
        // Scale based on rotation: portrait (0,2) = large, landscape (1,3) = smaller to fit 135px
        bool isLandscape = (_rotation == 1 || _rotation == 3);
        
        // Draw a large question mark using FreeSansBold font
        _sprite->setTextColor(rgb565);
        _sprite->setTextDatum(MC_DATUM);
        _sprite->setFont(&fonts::FreeSansBold24pt7b);
        _sprite->setTextSize(isLandscape ? 2 : 3);  // Smaller for landscape
        
        int16_t textOffset = isLandscape ? -5 : -10;
        int16_t dotOffset = isLandscape ? 30 : 50;
        int16_t dotRadius = isLandscape ? 5 : 8;
        
        _sprite->drawString("?", cx, cy + textOffset);
        
        // Draw the dot at the bottom
        _sprite->fillCircle(cx, cy + dotOffset, dotRadius, rgb565);
    }

    void DisplayTFT::drawResetIcon(int16_t cx, int16_t cy, color_t color) {
        if (!_sprite) return;
        
        uint16_t rgb565 = colorToRGB565(color);
        
        // Scale based on rotation: portrait (0,2) = large, landscape (1,3) = smaller to fit 135px
        bool isLandscape = (_rotation == 1 || _rotation == 3);
        float scale = isLandscape ? 0.65f : 1.0f;
        
        // Circular arrow dimensions
        int16_t outerR = static_cast<int16_t>(45 * scale);   // Outer radius of arc
        int16_t innerR = static_cast<int16_t>(35 * scale);   // Inner radius - thinner arc (was 25)
        int16_t arrowLen = static_cast<int16_t>(28 * scale); // Arrow length (slightly longer)
        int16_t arrowWid = static_cast<int16_t>(26 * scale); // Arrow width - wider base for visibility
        
        // Draw the circular arc with gap on the LEFT side
        // LovyanGFX: angles go clockwise from 3 o'clock (0°)
        // Gap on left: arc from 225° to 495° (135° + 360°)
        _sprite->fillArc(cx, cy, outerR, innerR, 225, 495, rgb565);
        
        // Arrow at the END of arc (135°), with BASE aligned to arc end
        // Tangent at 135° points toward 225° (down-left, clockwise along arc)
        float endAngle = 135.0f * M_PI / 180.0f;
        int16_t midR = (outerR + innerR) / 2;
        
        // Arc end point (center of arc thickness)
        int16_t arcEndX = cx + static_cast<int16_t>(midR * cos(endAngle));
        int16_t arcEndY = cy + static_cast<int16_t>(midR * sin(endAngle));
        
        // Tangent direction (225 degrees - pointing down-left along arc flow)
        float tangentAngle = (135.0f + 90.0f) * M_PI / 180.0f;  // 225 degrees
        
        // Perpendicular to tangent for base spread (135° and 315°)
        float perpAngle1 = (225.0f - 90.0f) * M_PI / 180.0f;  // 135 degrees
        float perpAngle2 = (225.0f + 90.0f) * M_PI / 180.0f;  // 315 degrees
        
        // BASE of arrow at arc end point, spread perpendicular to tangent
        int16_t base1X = arcEndX + static_cast<int16_t>(arrowWid * 0.5f * cos(perpAngle1));
        int16_t base1Y = arcEndY + static_cast<int16_t>(arrowWid * 0.5f * sin(perpAngle1));
        int16_t base2X = arcEndX + static_cast<int16_t>(arrowWid * 0.5f * cos(perpAngle2));
        int16_t base2Y = arcEndY + static_cast<int16_t>(arrowWid * 0.5f * sin(perpAngle2));
        
        // TIP of arrow extends from arc end in tangent direction (down-left)
        int16_t tipX = arcEndX + static_cast<int16_t>(arrowLen * cos(tangentAngle));
        int16_t tipY = arcEndY + static_cast<int16_t>(arrowLen * sin(tangentAngle));
        
        _sprite->fillTriangle(tipX, tipY, base1X, base1Y, base2X, base2Y, rgb565);
    }

    void DisplayTFT::drawTallyFrame(color_t color, uint8_t thickness) {
        if (!_sprite) return;
        
        uint16_t rgb565 = colorToRGB565(color);
        uint16_t w = currentWidth();
        uint16_t h = currentHeight();
        
        // Draw frame around the display edge
        for (uint8_t i = 0; i < thickness; i++) {
            _sprite->drawRect(i, i, w - (2 * i), h - (2 * i), rgb565);
        }
        
        show();
    }

    void DisplayTFT::setRotation(uint8_t rotation) {
        _rotation = rotation % 4;
        if (_lcd) {
            _lcd->setRotation(_rotation);
            
            // Recreate sprite with new dimensions to match rotated LCD
            // After rotation, _lcd->width() and _lcd->height() return the rotated dimensions
            if (_sprite) {
                _sprite->deleteSprite();
                _sprite->createSprite(_lcd->width(), _lcd->height());
                _sprite->setSwapBytes(true);
                log_i("Sprite recreated for rotation %d: %dx%d", _rotation, _lcd->width(), _lcd->height());
            }
        }
    }

    uint8_t DisplayTFT::getRotation() const {
        return _rotation;
    }

    void DisplayTFT::setOrientationRotation(Orientation orientation) {
        // Map IMU orientation to TFT rotation values
        // M5StickC Plus default orientation: USB port at bottom = UP
        // LovyanGFX rotation: 0=portrait(USB bottom), 1=landscape(USB right), 
        //                     2=portrait inverted(USB top), 3=landscape(USB left)
        uint8_t rotation;
        switch (orientation) {
            case Orientation::UP:
                rotation = 0;  // Portrait, USB at bottom
                break;
            case Orientation::RIGHT:
                rotation = 3;  // Landscape, USB at right (was 1, swapped)
                break;
            case Orientation::DOWN:
                rotation = 2;  // Portrait inverted, USB at top
                break;
            case Orientation::LEFT:
                rotation = 1;  // Landscape, USB at left (was 3, swapped)
                break;
            case Orientation::FLAT:
            case Orientation::UNKNOWN:
            default:
                rotation = 0;  // Default to portrait
                break;
        }
        // Apply board-specific rotation offset (modulo 4 to keep in valid range)
        rotation = (rotation + TFT_ROTATION_OFFSET) % 4;
        setRotation(rotation);
        log_i("Display rotation set to %d for orientation %d (offset %d)", rotation, static_cast<int>(orientation), TFT_ROTATION_OFFSET);
    }

    // ========================================================================
    // Private Helper Methods
    // ========================================================================

    uint16_t DisplayTFT::colorToRGB565(color_t color) const {
        // Convert 24-bit RGB to 16-bit RGB565
        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;
        
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }

    // Helper to get current width accounting for rotation
    inline uint16_t DisplayTFT::currentWidth() const {
        return _lcd ? _lcd->width() : _width;
    }

    // Helper to get current height accounting for rotation
    inline uint16_t DisplayTFT::currentHeight() const {
        return _lcd ? _lcd->height() : _height;
    }

    void DisplayTFT::updateBacklight() {
        #if defined(USE_AXP192_PMU)
            // Use AXP192 PMU to control backlight brightness via LDO2
            _pmu.setBacklight(_brightness);
        #elif defined(USE_LGFX_BACKLIGHT)
            // Use LovyanGFX built-in backlight control (PWM)
            if (_lcd) {
                _lcd->setBrightness(_brightness);
            }
        #endif
        log_d("Backlight set to: %d", _brightness);
    }

} // namespace Display
