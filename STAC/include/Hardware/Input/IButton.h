#ifndef STAC_IBUTTON_H
#define STAC_IBUTTON_H

#include <cstdint>
#include <functional>


    namespace Hardware {

        /**
         * @brief Button event types
         */
        enum class ButtonEvent : uint8_t {
            NONE,           ///< No event
            PRESSED,        ///< Button was pressed
            RELEASED,       ///< Button was released
            CLICK,          ///< Short press and release
            LONG_PRESS,     ///< Button held for long duration
            DOUBLE_CLICK    ///< Two quick clicks
        };

        /**
         * @brief Abstract interface for button input
         *
         * Provides debounced button handling with event callbacks.
         * Supports click, long press, and double-click detection.
         */
        class IButton {
          public:
            virtual ~IButton() = default;

            /**
             * @brief Initialize the button
             * @return true if initialization succeeded
             */
            virtual bool begin() = 0;

            /**
             * @brief Update button state (call frequently in loop)
             * Must be called regularly to detect state changes
             */
            virtual void update() = 0;

            /**
             * @brief Check if button is currently pressed
             * @return true if button is pressed
             */
            virtual bool isPressed() const = 0;

            /**
             * @brief Check if button was clicked since last check
             * @return true if click detected (resets on read)
             */
            virtual bool wasClicked() = 0;

            /**
             * @brief Check if button is being held
             * @return true if button held longer than long press threshold
             */
            virtual bool isLongPress() const = 0;

            /**
             * @brief Get how long button has been pressed
             * @return Duration in milliseconds (0 if not pressed)
             */
            virtual unsigned long getPressedDuration() const = 0;

            /**
             * @brief Set callback for button events
             * @param callback Function to call when events occur
             */
            virtual void setEventCallback( std::function<void( ButtonEvent )> callback ) = 0;
        };

    } // namespace Hardware


#endif // STAC_IBUTTON_H


//  --- EOF --- //
