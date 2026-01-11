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
            : led(DISPLAY_LED_TYPE, false)
            , width(DISPLAY_MATRIX_WIDTH)
            , height(DISPLAY_MATRIX_HEIGHT)
            , numLeds(width * height)
            , topCenterX(width / 2)
            , topCenterY(0) {
        }

        bool begin() override {
            // Initialize LED matrix
            led.begin(PIN_DISPLAY_DATA, numLeds);
            led.brightness(128); // Medium brightness
            clear();
            return true;
        }

        void showTopMarker() override {
            clear();
            
            // Calculate linear index for top-center pixel
            // Matrix layout: row-major, top-left is 0
            uint8_t pixelIndex = topCenterY * width + topCenterX;
            
            // Light up top-center pixel in bright white
            // crgb_t is uint32_t with format 0x00RRGGBB
            crgb_t white = 0x00FFFFFF;
            led.setPixel(pixelIndex, white, false);
            led.show();
        }

        void clear() override {
            led.clear();
            led.show();
        }

        void showMessage(const char* message, uint32_t color) override {
            // LED matrix can't show text - just fill with color
            uint8_t r = (color >> 16) & 0xFF;
            uint8_t g = (color >> 8) & 0xFF;
            uint8_t b = color & 0xFF;
            
            // crgb_t is uint32_t with format 0x00RRGGBB
            crgb_t pixelColor = (r << 16) | (g << 8) | b;
            
            for (uint8_t i = 0; i < numLeds; i++) {
                led.setPixel(i, pixelColor, false);
            }
            led.show();
        }

        void setBrightness(uint8_t brightness) override {
            led.brightness(brightness);
        }
    };

} // namespace Calibration

#endif // CALIBRATION_DISPLAY_LED_H


//  --- EOF --- //
