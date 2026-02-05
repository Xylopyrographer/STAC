#ifndef STAC_OPERATING_MODE_MANAGER_H
#define STAC_OPERATING_MODE_MANAGER_H

#include <cstdint>
#include <functional>
#include "Config/Types.h"
#include "StateManagerBase.h"


namespace State {

    /**
     * @brief Manages STAC operating modes
     *
     * Handles transitions between Normal, Peripheral, and Provisioning modes.
     * Coordinates mode-specific behavior and state management.
     */
    class OperatingModeManager : public StateManagerBase<OperatingMode> {
      public:
        OperatingModeManager();
        ~OperatingModeManager() = default;

        /**
         * @brief Set new operating mode
         * @param newMode Mode to transition to
         * @return true if mode changed
         */
        bool setMode( OperatingMode newMode );

        /**
         * @brief Get current operating mode (wrapper)
         * @return Current OperatingMode
         */
        OperatingMode getCurrentMode() const {
            return currentState;
        }

        /**
         * @brief Get previous operating mode (wrapper)
         * @return Previous OperatingMode
         */
        OperatingMode getPreviousMode() const {
            return previousState;
        }

        /**
         * @brief Check if in normal operating mode
         * @return true if mode is NORMAL
         */
        bool isNormalMode() const {
            return currentState == OperatingMode::NORMAL;
        }

        /**
         * @brief Check if in peripheral mode
         * @return true if mode is PERIPHERAL
         */
        bool isPeripheralMode() const {
            return currentState == OperatingMode::PERIPHERAL;
        }

        /**
         * @brief Check if in provisioning mode
         * @return true if mode is PROVISIONING
         */
        bool isProvisioningMode() const {
            return currentState == OperatingMode::PROVISIONING;
        }

        /**
         * @brief Get string representation of current mode
         * @return Mode name as string
         */
        const char *getModeString() const;

      private:
        /**
         * @brief Convert OperatingMode to string
         * @param mode Mode to convert
         * @return String representation
         */
        static const char *modeToString( OperatingMode mode );
    };

} // namespace State


#endif // STAC_OPERATING_MODE_MANAGER_H


//  --- EOF --- //
