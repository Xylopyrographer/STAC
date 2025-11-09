#ifndef STAC_APP_H
#define STAC_APP_H

#include <memory>
#include "Hardware/Display/IDisplay.h"
#include "Hardware/Sensors/IIMU.h"
#include "Hardware/Input/IButton.h"
#include "Hardware/Interface/GrovePort.h"
#include "Hardware/Interface/PeripheralMode.h"
#include "Network/WiFiManager.h"
#include "Storage/ConfigManager.h"
#include "State/SystemState.h"

namespace STAC {
    namespace Application {

        /**
         * @brief Main STAC application controller
         *
         * Coordinates all subsystems and manages the main application logic.
         * Handles mode switching, state updates, and user interaction.
         */
        class STACApp {
          public:
            /**
             * @brief Construct STAC application
             */
            STACApp();

            ~STACApp() = default;

            /**
             * @brief Initialize the application
             * @return true if initialization succeeded
             */
            bool setup();

            /**
             * @brief Main application loop (call repeatedly)
             */
            void loop();

            /**
             * @brief Get current system state
             * @return Reference to SystemState
             */
            State::SystemState &getSystemState() {
                return *systemState;
            }

          private:
            // Hardware - use correct namespaces!
            std::unique_ptr<Display::IDisplay> display;              // Display namespace
            std::unique_ptr<Hardware::IIMU> imu;                     // Hardware namespace
            std::unique_ptr<Hardware::IButton> button;               // Hardware namespace
            std::unique_ptr<Hardware::GrovePort> grovePort;          // Hardware namespace
            std::unique_ptr<Hardware::PeripheralMode> peripheralDetector;  // Hardware namespace

            // Network & Storage
            std::unique_ptr<Network::WiFiManager> wifiManager;
            std::unique_ptr<Storage::ConfigManager> configManager;

            // State
            std::unique_ptr<State::SystemState> systemState;

            // Application state
            bool initialized;
            String stacID;
            Orientation lastOrientation;

            /**
             * @brief Initialize hardware subsystems
             * @return true if successful
             */
            bool initializeHardware();

            /**
             * @brief Initialize network and storage
             * @return true if successful
             */
            bool initializeNetworkAndStorage();

            /**
             * @brief Determine initial operating mode
             * @return Operating mode to start in
             */
            OperatingMode determineOperatingMode();

            /**
             * @brief Handle button events
             */
            void handleButton();

            /**
             * @brief Handle IMU orientation changes
             */
            void handleOrientation();

            /**
             * @brief Update display based on current state
             */
            void updateDisplay();

            /**
             * @brief Handle normal operating mode
             */
            void handleNormalMode();

            /**
             * @brief Handle peripheral operating mode
             */
            void handlePeripheralMode();

            /**
             * @brief Handle provisioning mode
             */
            void handleProvisioningMode();

            /**
             * @brief Show startup animation
             */
            void showStartupAnimation();

            /**
             * @brief Show error on display
             * @param errorCode Error code to display
             */
            void showError( uint8_t errorCode );
        };

    } // namespace Application
} // namespace STAC

#endif // STAC_APP_H


//  --- EOF --- //
