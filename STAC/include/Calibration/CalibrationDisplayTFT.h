// CalibrationDisplayTFT.h
// TFT LCD implementation of calibration display

#ifndef CALIBRATION_DISPLAY_TFT_H
#define CALIBRATION_DISPLAY_TFT_H

#include "Calibration/CalibrationDisplay.h"
#include "Hardware/Display/TFT/LGFX_STAC.h"

namespace Calibration {

    /**
     * @brief TFT LCD calibration display
     * 
     * Shows arrow pointing up at top edge during calibration.
     */
    class CalibrationDisplayTFT : public CalibrationDisplay {
    private:
        LGFX_STAC* tft;
        uint16_t displayWidth;
        uint16_t displayHeight;

    public:
        CalibrationDisplayTFT() : tft(nullptr) {
        }

        ~CalibrationDisplayTFT() {
            if (tft) {
                delete tft;
            }
        }

        bool begin() override {
            tft = new LGFX_STAC();
            
            if (!tft->init()) {
                return false;
            }

            tft->setRotation(0); // Portrait mode for calibration
            displayWidth = tft->width();
            displayHeight = tft->height();
            
            // Turn on backlight
            #if defined(USE_PMU)
                // AXP192 backlight control
                #include "Hardware/Power/AXP192.h"
                Hardware::AXP192::enableBacklight();
            #elif defined(TFT_BACKLIGHT_PIN)
                // PWM backlight control
                pinMode(TFT_BACKLIGHT_PIN, OUTPUT);
                analogWrite(TFT_BACKLIGHT_PIN, 128); // Medium brightness
            #endif

            clear();
            return true;
        }

        void showTopMarker() override {
            clear();
            
            // Draw upward-pointing arrow at top-center
            int16_t centerX = displayWidth / 2;
            int16_t arrowTop = 10;
            int16_t arrowHeight = 40;
            int16_t arrowWidth = 30;
            
            // Draw triangle (arrow head)
            tft->fillTriangle(
                centerX, arrowTop,                    // Top point
                centerX - arrowWidth/2, arrowTop + arrowHeight/2, // Bottom left
                centerX + arrowWidth/2, arrowTop + arrowHeight/2, // Bottom right
                TFT_WHITE
            );
            
            // Draw rectangle (arrow shaft)
            int16_t shaftWidth = 10;
            int16_t shaftHeight = arrowHeight;
            tft->fillRect(
                centerX - shaftWidth/2,
                arrowTop + arrowHeight/2,
                shaftWidth,
                shaftHeight,
                TFT_WHITE
            );
        }

        void clear() override {
            if (tft) {
                tft->fillScreen(TFT_BLACK);
            }
        }

        void showMessage(const char* message, uint32_t color) override {
            if (!tft) return;
            
            clear();
            tft->setTextColor(color);
            tft->setTextSize(2);
            tft->setCursor(10, displayHeight / 2 - 10);
            tft->println(message);
        }

        void setBrightness(uint8_t brightness) override {
            #if defined(TFT_BACKLIGHT_PIN) && !defined(USE_PMU)
                analogWrite(TFT_BACKLIGHT_PIN, brightness);
            #endif
            // Note: AXP192 brightness control would need to be added if needed
        }
    };

} // namespace Calibration

#endif // CALIBRATION_DISPLAY_TFT_H


//  --- EOF --- //
