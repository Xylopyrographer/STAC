#ifndef STAC_BUTTON_HANDLER_H
#define STAC_BUTTON_HANDLER_H

#include "IButton.h"
#include "Device_Config.h"

namespace STAC {
    namespace Hardware {

        /**
         * @brief Button handler with debouncing and event detection
         *
         * Implements IButton interface with hardware debouncing,
         * long press detection, and event callbacks.
         */
        class ButtonHandler : public IButton {
          public:
            /**
             * @brief Construct a new Button Handler
             * @param pin GPIO pin for button
             * @param activeLow true if button is active low (default)
             * @param debounceMs Debounce time in milliseconds
             * @param longPressMs Long press threshold in milliseconds
             */
            ButtonHandler( uint8_t pin, bool activeLow = true,
                           unsigned long debounceMs = 25,
                           unsigned long longPressMs = 1500 );

            ~ButtonHandler() override = default;

            // IButton interface implementation
            bool begin() override;
            void update() override;
            bool isPressed() const override;
            bool wasClicked() override;
            bool isLongPress() const override;
            unsigned long getPressedDuration() const override;
            void setEventCallback( std::function<void( ButtonEvent )> callback ) override;

          private:
            uint8_t pin;
            bool activeLow;
            unsigned long debounceMs;
            unsigned long longPressMs;

            // State tracking
            bool currentState;          ///< Current debounced state
            bool lastState;             ///< Previous state
            bool clickFlag;             ///< Click detected flag
            bool longPressFlag;         ///< Long press detected flag
            unsigned long pressStartTime; ///< When button was pressed
            unsigned long lastChangeTime; ///< Last state change time

            // Event callback
            std::function<void( ButtonEvent )> eventCallback;

            /**
             * @brief Read raw button state from GPIO
             * @return true if button is pressed
             */
            bool readRawState() const;
        };

    } // namespace Hardware
} // namespace STAC

#endif // STAC_BUTTON_HANDLER_H


//  --- EOF --- //
