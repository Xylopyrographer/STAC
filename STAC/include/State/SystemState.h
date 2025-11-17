#ifndef STAC_SYSTEM_STATE_H
#define STAC_SYSTEM_STATE_H

#include "TallyStateManager.h"
#include "OperatingModeManager.h"
#include "Config/Types.h"


    namespace State {

        /**
         * @brief Central system state manager
         *
         * Coordinates all state management including tally state,
         * operating mode, and system configuration.
         */
        class SystemState {
          public:
            SystemState();
            ~SystemState() = default;

            /**
             * @brief Initialize system state
             * @return true if initialization succeeded
             */
            bool begin();

            /**
             * @brief Get tally state manager
             * @return Reference to TallyStateManager
             */
            TallyStateManager &getTallyState() {
                return tallyState;
            }

            /**
             * @brief Get operating mode manager
             * @return Reference to OperatingModeManager
             */
            OperatingModeManager &getOperatingMode() {
                return operatingMode;
            }

            /**
             * @brief Get current operations configuration
             * @return Reference to StacOperations
             */
            StacOperations &getOperations() {
                return operations;
            }

            /**
             * @brief Get current WiFi info
             * @return Reference to WiFiInfo
             */
            WiFiInfo &getWiFiInfo() {
                return wifiInfo;
            }

            /**
             * @brief Get current switch state
             * @return Reference to SwitchState
             */
            SwitchState &getSwitchState() {
                return switchState;
            }

            /**
             * @brief Check if system is ready for normal operation
             * @return true if configured and ready
             */
            bool isReady() const;

            /**
             * @brief Update system state (call regularly)
             */
            void update();

          private:
            TallyStateManager tallyState;           ///< Tally state management
            OperatingModeManager operatingMode;     ///< Operating mode management
            StacOperations operations;              ///< Current operations config
            WiFiInfo wifiInfo;                      ///< WiFi connection info
            SwitchState switchState;                ///< Video switch state

            bool initialized;                       ///< Initialization flag
        };

    } // namespace State


#endif // STAC_SYSTEM_STATE_H


//  --- EOF --- //
