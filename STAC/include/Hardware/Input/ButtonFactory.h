#ifndef STAC_BUTTON_FACTORY_H
#define STAC_BUTTON_FACTORY_H

#include "IButton.h"
#include "ButtonHandler.h"
#include "Device_Config.h"
#include "Config/Constants.h"
#include <memory>

namespace STAC {
    namespace Hardware {

        /**
         * @brief Factory for creating button instances
         *
         * Creates button handler configured for the current board
         */
        class ButtonFactory {
          public:
            /**
             * @brief Create a button instance for the configured board
             * @return Unique pointer to IButton implementation
             */
            static std::unique_ptr<IButton> create() {
                return std::make_unique<ButtonHandler>(
                           Config::Pins::BUTTON,
                           BUTTON_ACTIVE_LOW,
                           Config::Button::DEBOUNCE_MS,
                           Config::Timing::BUTTON_SELECT_MS
                       );
            }
        };

    } // namespace Hardware
} // namespace STAC

#endif // STAC_BUTTON_FACTORY_H


//  --- EOF --- //
