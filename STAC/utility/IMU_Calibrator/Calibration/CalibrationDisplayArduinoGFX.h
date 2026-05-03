// CalibrationDisplayArduinoGFX.h
// Arduino_GFX implementation of calibration display
//
// Used for boards that drive their TFT via the moononournation Arduino_GFX
// library (e.g. M5Stack ATOM S3R with GC9107 panel).  Boards using
// LovyanGFX should continue to use CalibrationDisplayTFT.h.

#ifndef CALIBRATION_DISPLAY_ARDUINO_GFX_H
#define CALIBRATION_DISPLAY_ARDUINO_GFX_H

#include "Device_Config.h"
#include "Calibration/CalibrationDisplay.h"
#include "Hardware/Display/TFT/ArduinoGFX_STAC.h"

#if defined(DISPLAY_BACKLIGHT_LP5562)
    #include "Hardware/Power/LP5562.h"
#endif

namespace Calibration {

    /**
     * @brief Arduino_GFX TFT calibration display
     *
     * Mirrors the interface of CalibrationDisplayTFT (LovyanGFX) but uses
     * the moononournation Arduino_GFX library.  Backlight is handled
     * conditionally: LP5562 RGBW driver (ATOM S3R) or GPIO PWM pin.
     */
    class CalibrationDisplayArduinoGFX : public CalibrationDisplay {
      private:
        Arduino_GFX *gfx;
        uint16_t displayWidth;
        uint16_t displayHeight;

        #if defined(DISPLAY_BACKLIGHT_LP5562)
        // Wire bus will be initialised by LP5562::begin() because we supply
        // the actual SDA/SCL pins to the constructor (sda >= 0 check).
        Hardware::LP5562 _lp5562;
        #endif

        // Convert a packed RGB888 (uint32_t) colour to RGB565 (uint16_t)
        static uint16_t toRGB565( uint32_t rgb888 ) {
            uint8_t r = ( rgb888 >> 16 ) & 0xFF;
            uint8_t g = ( rgb888 >> 8  ) & 0xFF;
            uint8_t b =   rgb888         & 0xFF;
            return static_cast<uint16_t>(
                       ( ( r & 0xF8 ) << 8 ) |
                       ( ( g & 0xFC ) << 3 ) |
                       (   b          >> 3 )
                   );
        }

      public:
        CalibrationDisplayArduinoGFX()
            : gfx( nullptr )
            , displayWidth( DISPLAY_WIDTH )
            , displayHeight( DISPLAY_HEIGHT )
              #if defined(DISPLAY_BACKLIGHT_LP5562)
              // Supply real SDA/SCL so LP5562::begin() initialises Wire
            , _lp5562( LED_DRVR_ADDR, Wire, LED_DRVR_SDA, LED_DRVR_SCL, 400000 )
              #endif
        {}

        ~CalibrationDisplayArduinoGFX() {
            if ( gfx ) {
                delete gfx;
            }
        }

        bool begin() override {
            // --- Backlight / I2C init -------------------------------------------
            // LP5562 init also calls Wire.begin(), which the BMI270 IMU (initialised
            // after the display) will then re-use on the same bus.
            #if defined(DISPLAY_BACKLIGHT_LP5562)
            if ( !_lp5562.begin() ) {
                return false;
            }
            _lp5562.setW( 0 );  // Keep backlight off until display is ready
            #elif defined(TFT_BACKLIGHT_PIN)
            pinMode( TFT_BACKLIGHT_PIN, OUTPUT );
            analogWrite( TFT_BACKLIGHT_PIN, 0 );
            #endif

            // --- Display init ---------------------------------------------------
            gfx = Display::createArduinoGFXDisplay( 0 ); // rotation 0 for calibration
            if ( !gfx ) {
                return false;
            }
            gfx->begin();

            // Clear display memory in all rotations (matches main firmware init)
            for ( uint8_t r = 0; r < 4; r++ ) {
                gfx->setRotation( r );
                gfx->fillScreen( 0x0000 );  // BLACK
            }
            gfx->setRotation( 0 );

            // --- Enable backlight -----------------------------------------------
            #if defined(DISPLAY_BACKLIGHT_LP5562)
            _lp5562.setW( 128 );  // Medium brightness
            #elif defined(TFT_BACKLIGHT_PIN)
            analogWrite( TFT_BACKLIGHT_PIN, 128 );
            #endif

            return true;
        }

        /** Draw upward-pointing arrow at top-centre of display */
        void showTopMarker() override {
            clear();

            int16_t centerX    = displayWidth  / 2;
            int16_t arrowTop   = 10;
            int16_t arrowHeight = 40;
            int16_t arrowWidth  = 30;

            // Arrow head (triangle)
            gfx->fillTriangle(
                centerX,                        arrowTop,
                centerX - arrowWidth / 2,       arrowTop + arrowHeight / 2,
                centerX + arrowWidth / 2,       arrowTop + arrowHeight / 2,
                0xFFFF  // WHITE
            );

            // Arrow shaft (rectangle)
            int16_t shaftWidth = 10;
            gfx->fillRect(
                centerX - shaftWidth / 2,
                arrowTop + arrowHeight / 2,
                shaftWidth,
                arrowHeight,
                0xFFFF  // WHITE
            );
        }

        /**
         * Show a coloured dot at one of the four screen corners.
         * Index: 0=top-left, 1=top-right, 2=bottom-right, 3=bottom-left
         */
        void showPixelAtIndex( uint8_t index ) {
            clear();

            const int16_t dotSize = 20;
            int16_t x, y;

            switch ( index ) {
                case 0:  // Top-left
                    x = dotSize;
                    y = dotSize;
                    break;
                case 1:  // Top-right
                    x = displayWidth  - dotSize * 2;
                    y = dotSize;
                    break;
                case 2:  // Bottom-right
                    x = displayWidth  - dotSize * 2;
                    y = displayHeight - dotSize * 2;
                    break;
                case 3:  // Bottom-left
                    x = dotSize;
                    y = displayHeight - dotSize * 2;
                    break;
                default:
                    return;
            }

            gfx->fillCircle( x + dotSize / 2, y + dotSize / 2, dotSize, 0xF800 );  // RED
        }

        void clear() override {
            if ( gfx ) {
                gfx->fillScreen( 0x0000 );  // BLACK
            }
        }

        void showMessage( const char* message, uint32_t color ) override {
            if ( !gfx ) {
                return;
            }
            clear();
            gfx->setTextColor( toRGB565( color ) );
            gfx->setTextSize( 2 );
            gfx->setCursor( 5, displayHeight / 2 - 10 );
            gfx->println( message );
        }

        void setBrightness( uint8_t brightness ) override {
            #if defined(DISPLAY_BACKLIGHT_LP5562)
            _lp5562.setW( brightness );
            #elif defined(TFT_BACKLIGHT_PIN)
            analogWrite( TFT_BACKLIGHT_PIN, brightness );
            #endif
        }
    };

} // namespace Calibration

#endif // CALIBRATION_DISPLAY_ARDUINO_GFX_H

// --- EOF --- //
