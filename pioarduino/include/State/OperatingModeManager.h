#ifndef STAC_OPERATING_MODE_MANAGER_H
#define STAC_OPERATING_MODE_MANAGER_H

#include <cstdint>
#include <functional>
#include "Config/Types.h"

namespace STAC {
    namespace State {

        /**
         * @brief Manages STAC operating modes
         *
         * Handles transitions between Normal, Peripheral, and Provisioning modes.
         * Coordinates mode-specific behavior and state management.
         */
        class OperatingModeManager {
          public:
            /**
             * @brief Callback function type for mode changes
             * @param oldMode Previous operating mode
             * @param newMode New operating mode
             */
            using ModeChangeCallback = std::function<void( OperatingMode oldMode, OperatingMode newMode )>;

            OperatingModeManager();
            ~OperatingModeManager() = default;

            /**
             * @brief Get current operating mode
             * @return Current OperatingMode
             */
            OperatingMode getCurrentMode() const {
                return currentMode;
            }

            /**
             * @brief Set new operating mode
             * @param newMode Mode to transition to
             * @return true if mode changed
             */
            bool setMode( OperatingMode newMode );

            /**
             * @brief Get previous operating mode
             * @return Previous OperatingMode
             */
            OperatingMode getPreviousMode() const {
                return previousMode;
            }

            /**
             * @brief Check if in normal operating mode
             * @return true if mode is NORMAL
             */
            bool isNormalMode() const {
                return currentMode == OperatingMode::NORMAL;
            }

            /**
             * @brief Check if in peripheral mode
             * @return true if mode is PERIPHERAL
             */
            bool isPeripheralMode() const {
                return currentMode == OperatingMode::PERIPHERAL;
            }

            /**
             * @brief Check if in provisioning mode
             * @return true if mode is PROVISIONING
             */
            bool isProvisioningMode() const {
                return currentMode == OperatingMode::PROVISIONING;
            }

            /**
             * @brief Get string representation of current mode
             * @return Mode name as string
             */
            const char *getModeString() const;

            /**
             * @brief Set callback for mode changes
             * @param callback Function to call on mode change
             */
            void setModeChangeCallback( ModeChangeCallback callback );

            /**
             * @brief Get time since last mode change
             * @return Milliseconds since last change
             */
            unsigned long getTimeSinceChange() const;

          private:
            OperatingMode currentMode;      ///< Current operating mode
            OperatingMode previousMode;     ///< Previous operating mode
            unsigned long lastChangeTime;   ///< Time of last mode change
            ModeChangeCallback callback;    ///< Mode change callback function

            /**
             * @brief Convert OperatingMode to string
             * @param mode Mode to convert
             * @return String representation
             */
            static const char *modeToString( OperatingMode mode );
        };

    } // namespace State
} // namespace STAC

#endif // STAC_OPERATING_MODE_MANAGER_H


//  --- EOF --- //
