#ifndef STAC_APP_H
#define STAC_APP_H

#include <memory>
#include <esp_timer.h>
#include "Hardware/Display/IDisplay.h"
#include "Hardware/Display/GlyphManager.h"
#include "Hardware/Sensors/IIMU.h"
#include "Hardware/Input/IButton.h"
#if HAS_PERIPHERAL_MODE_CAPABILITY
    #include "Hardware/Interface/GrovePort.h"
    #include "Hardware/Interface/PeripheralMode.h"
#endif
#include "Network/WiFiManager.h"
#include "Network/Protocol/IRolandClient.h"
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
         * @brief Destructor - cleanup buttons
         */
        ~STACApp() {
            delete button;
            #if defined(BUTTON_B_PIN)
            delete buttonB;
            #endif
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
        Button *button;                                          // XP_Button library - primary button (A)
        #if defined(BUTTON_B_PIN)
        Button *buttonB;                                     // XP_Button library - secondary button (B) for reset
        #endif
        esp_timer_handle_t buttonPollTimer;                         // Timer for automatic button polling

        /**
         * @brief Start the button polling timer
         * Polls button(s) every 2ms via esp_timer callback
         */
        void startButtonPolling();
        #if HAS_PERIPHERAL_MODE_CAPABILITY
        std::unique_ptr<Hardware::GrovePort> grovePort;          // Hardware namespace

        #endif

        // Glyph management - dimension-agnostic using type alias from glyph header
        std::unique_ptr<Display::GlyphManagerType> glyphManager;

        // Network & Storage
        std::unique_ptr<Net::WiFiManager> wifiManager;
        std::unique_ptr<Net::IRolandClient> rolandClient;
        std::unique_ptr<Storage::ConfigManager> configManager;

        // State
        std::unique_ptr<State::SystemState> systemState;

        // Startup configuration - dimension-agnostic using type alias from glyph header
        std::unique_ptr<Application::StartupConfigType> startupConfig;

        // Application state
        bool initialized;
        String stacID;
        bool provisioningFromBootButton;  // Track if provisioning mode was entered via boot button

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
         * @brief Handle Button B events (reset on M5StickC Plus)
         */
        void handleButtonB();

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
                                     const String &username, const String &password );

        /**
         * @brief Poll Roland switch for tally status
         */
        void pollRolandSwitch();

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        /**
         * @brief Handle peripheral operating mode
         */
        void handlePeripheralMode();
        #endif

        /**
         * @brief Handle provisioning mode
         */
        /**
         * @brief Handle provisioning/portal mode
         *
         * Starts unified web portal server with tabbed interface for:
         * - Device commissioning (WiFi + Roland switch config)
         * - OTA firmware updates
         *
         * Waits for user action (config submission or firmware upload),
         * then processes accordingly and restarts device.
         *
         * @param fromBootButton If true, skip initial flash (already done in boot sequence)
         */
        void handleProvisioningMode( bool fromBootButton = false );

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
         * - 0-2 sec (with PMode): Toggle peripheral mode
         * - 2-4 sec: Unified portal mode (provisioning/OTA)
         * - 4-6 sec: Factory reset
         *
         * @return Operating mode to enter (NORMAL, PROVISIONING, or PERIPHERAL)
         */
        OperatingMode checkBootButtonSequence();

        /**
         * @brief Show confirmation checkmark after boot button selection
         *
         * Clears display, shows green checkmark, pauses, then clears again
         */
        void showConfirmationCheckmark();

        /**
         * @brief Restart device with proper display cleanup
         *
         * Delays specified time, turns off display, then restarts.
         * Never returns.
         *
         * @param delayMs Delay before restart (default: GUI_PAUSE_MS = 1500ms)
         */
        void restartDevice( uint16_t delayMs = Config::Timing::GUI_PAUSE_MS );

    };

} // namespace Application

#endif // STAC_APP_H


//  --- EOF --- //
