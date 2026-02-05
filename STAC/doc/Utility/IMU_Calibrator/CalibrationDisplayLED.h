// CalibrationDisplayLED.h
// LED Matrix implementation of calibration display

#ifndef CALIBRATION_DISPLAY_LED_H
#define CALIBRATION_DISPLAY_LED_H

#include "Device_Config.h"
#include "Calibration/CalibrationDisplay.h"
#include <LiteLED.h>

namespace Calibration {

    /**
     * @brief LED Matrix calibration display (5x5 or 8x8)
     *
     * Shows reference pixel at top-center position during calibration.
     */
    class CalibrationDisplayLED : public CalibrationDisplay {
      private:
        LiteLED led;
        uint8_t width;
        uint8_t height;
        uint8_t numLeds;

        // Top-center pixel position (varies by matrix size)
        uint8_t topCenterX;
        uint8_t topCenterY;

      public:
        CalibrationDisplayLED()
            : led( DISPLAY_LED_TYPE, false )
            , width( DISPLAY_MATRIX_WIDTH )
            , height( DISPLAY_MATRIX_HEIGHT )
            , numLeds( width * height )
            , topCenterX( width / 2 )
            , topCenterY( 0 ) {
        }

        bool begin() override {
            // Initialize LED matrix
            led.begin( PIN_DISPLAY_DATA, numLeds );
            led.brightness( 10 ); // Low brightness to avoid heat
            led.clear();
            led.show();
            return true;
        }

        void showTopMarker() override {
            showPixelAtIndex( 0 ); // Show pixel at buffer index 0
        }

        void showPixelAtIndex( uint8_t index ) {
            led.clear();
            led.show();
            delay( 100 ); // Ensure clear is visible

            Serial.printf( "DEBUG: Showing pixel at buffer index %d\n", index );

            // Light up pixel in bright green
            crgb_t green = 0x00FF00;  // Green: 0x00RRGGBB format
            led.setPixel( index, green, false );
            led.show();
        }

        void showPixelAtSide( int side ) {
            // side: 0=TOP, 1=RIGHT, 2=BOTTOM, 3=LEFT
            led.clear();
            led.show();
            delay( 100 ); // Ensure clear is visible

            uint8_t pixelX, pixelY;

            if ( side == 0 ) {
                // TOP: middle of top row
                pixelX = width / 2;
                pixelY = 0;
            }
            else if ( side == 1 ) {
                // RIGHT: middle of right column
                pixelX = width - 1;
                pixelY = height / 2;
            }
            else if ( side == 2 ) {
                // BOTTOM: middle of bottom row
                pixelX = width / 2;
                pixelY = height - 1;
            }
            else {
                // LEFT: middle of left column
                pixelX = 0;
                pixelY = height / 2;
            }

            // Calculate linear index for pixel
            uint8_t pixelIndex = pixelY * width + pixelX;

            Serial.printf( "DEBUG: Showing pixel at side %d, index %d (x=%d, y=%d)\n",
                           side, pixelIndex, pixelX, pixelY );

            // Light up pixel in bright green
            crgb_t green = 0x00FF00;  // Green: 0x00RRGGBB format
            led.setPixel( pixelIndex, green, false );
            led.show();
        }

        void clear() override {
            led.clear();
            led.show();
        }

        void showMessage( const char* message, uint32_t color ) override {
            // LED matrix can't show text - just fill with color
            uint8_t r = ( color >> 16 ) & 0xFF;
            uint8_t g = ( color >> 8 ) & 0xFF;
            uint8_t b = color & 0xFF;

            // crgb_t is uint32_t with format 0x00RRGGBB
            crgb_t pixelColor = ( r << 16 ) | ( g << 8 ) | b;

            for ( uint8_t i = 0; i < numLeds; i++ ) {
                led.setPixel( i, pixelColor, false );
            }
            led.show();
        }

        void setBrightness( uint8_t brightness ) override {
            led.brightness( brightness );
        }
    };

} // namespace Calibration

#endif // CALIBRATION_DISPLAY_LED_H


//  --- EOF --- //
