// CalibrationDisplay.h
// Display abstraction for IMU calibration tool
// Supports both LED matrix and TFT displays

#ifndef CALIBRATION_DISPLAY_H
#define CALIBRATION_DISPLAY_H

#include <Arduino.h>
#include "Device_Config.h"

namespace Calibration {

    /**
     * @brief Abstract base class for calibration display markers
     *
     * Provides display-agnostic interface for showing orientation reference markers
     * during IMU calibration procedure.
     */
    class CalibrationDisplay {
      public:
        virtual ~CalibrationDisplay() = default;

        /**
         * @brief Initialize the display
         * @return true if successful, false otherwise
         */
        virtual bool begin() = 0;

        /**
         * @brief Show reference marker at top edge of display
         *
         * For LED matrix: Lights pixel at top-center
         * For TFT: Draws arrow or symbol at top edge
         */
        virtual void showTopMarker() = 0;

        /**
         * @brief Clear the display
         */
        virtual void clear() = 0;

        /**
         * @brief Show text message (TFT only, LED shows color pattern)
         * @param message Text to display
         * @param color Display color (interpretation depends on display type)
         */
        virtual void showMessage( const char* message, uint32_t color ) = 0;

        /**
         * @brief Set display brightness
         * @param brightness 0-255
         */
        virtual void setBrightness( uint8_t brightness ) = 0;
    };

} // namespace Calibration

#endif // CALIBRATION_DISPLAY_H


//  --- EOF --- //
