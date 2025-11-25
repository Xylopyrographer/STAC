#ifndef STAC_APP_H
#define STAC_APP_H

#include <memory>
#include "Hardware/Display/IDisplay.h"
#include "Hardware/Display/GlyphManager.h"
#include "Hardware/Sensors/IIMU.h"
#include "Hardware/Input/IButton.h"
#include "Hardware/Interface/GrovePort.h"
#include "Hardware/Interface/PeripheralMode.h"
#include "Network/WiFiManager.h"
#include "Network/Protocol/IRolandClient.h"
#include "Network/WebConfigServer.h"
#include "Network/OTAUpdateServer.h"
#include "Storage/ConfigManager.h"
#include "State/SystemState.h"
#include "Application/StartupConfig.h"

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

        /**
         * @brief Destructor - cleanup button
         */
        ~STACApp() {
            delete button;
            // @Claude: should the other hardware objects be released too? Display?
        }

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
        Button *button;                                          // XP_Button library
        std::unique_ptr<Hardware::GrovePort> grovePort;          // Hardware namespace
        std::unique_ptr<Hardware::PeripheralMode> peripheralDetector;  // Hardware namespace

// @Claude: No longer need these conditionals since we have GLYPH_SIZE_ defined in board config
        // Glyph management
#ifdef GLYPH_SIZE_5X5
        std::unique_ptr<Display::GlyphManager5x5> glyphManager;
#else
        std::unique_ptr<Display::GlyphManager8x8> glyphManager;
#endif

        // Network & Storage
        std::unique_ptr<Net::WiFiManager> wifiManager;
        std::unique_ptr<Net::IRolandClient> rolandClient;
        std::unique_ptr<Storage::ConfigManager> configManager;

        // State
        std::unique_ptr<State::SystemState> systemState;

 // @Claude: No longer need these conditionals since we have GLYPH_SIZE_ defined in board config
        // Startup configuration
#ifdef GLYPH_SIZE_5X5
        std::unique_ptr<StartupConfig5x5> startupConfig;
#else
        std::unique_ptr<StartupConfig8x8> startupConfig;
#endif

        // Application state
        bool initialized;
        String stacID;

        // Roland polling state
        unsigned long lastRolandPoll;
        uint32_t rolandPollInterval;
        bool rolandClientInitialized;

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
         * @brief Update display based on current state
         */
        void updateDisplay();

        /**
         * @brief Handle normal operating mode
         */
        void handleNormalMode();

        /**
         * @brief Display WiFi connection status with visual feedback
         * @param state Current WiFi state
         */
        void displayWiFiStatus( Net::WiFiState state );

        /**
         * @brief Initialize Roland client with provided configuration
         * @param switchIP IP address of the Roland switch
         * @param switchPort Port number of the Roland switch
         * @param username LAN username for the switch
         * @param password LAN password for the switch
         * @return true if initialization successful
         */
        bool initializeRolandClient( const IPAddress& switchIP, uint16_t switchPort,
                                     const String& username, const String& password );

        /**
         * @brief Poll Roland switch for tally status
         */
        void pollRolandSwitch();

        /**
         * @brief Handle peripheral operating mode
         */
        void handlePeripheralMode();

        /**
         * @brief Handle provisioning mode
         */
        void handleProvisioningMode();

        /**
         * @brief Handle OTA update mode
         *
         * Starts OTA update server, waits for firmware upload,
         * and restarts device with new firmware.
         */
        void handleOTAUpdateMode();

        /**
         * @brief Handle factory reset
         *
         * Clears all NVS configuration and restarts device.
         */
        void handleFactoryReset();

        /**
         * @brief Check for button hold at boot
         *
         * Implements button state machine:
         * - Short hold: Force provisioning mode
         * - Medium hold: Factory reset
         * - Long hold: OTA update mode
         *
         * @return Operating mode to enter (NORMAL, PROVISIONING, or DFU)
         */
        OperatingMode checkBootButtonSequence();

    };

} // namespace Application

#endif // STAC_APP_H


//  --- EOF --- //
