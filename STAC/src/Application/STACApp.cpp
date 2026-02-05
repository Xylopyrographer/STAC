#include <Arduino.h>
#include "Application/STACApp.h"
#include "Hardware/Display/DisplayFactory.h"
#include "Hardware/Display/GlyphManager.h"
#include "Hardware/Display/StatusLED.h"
// Glyph definitions included via Device_Config.h -> board config
#include "Hardware/Sensors/IMUFactory.h"
#include "Hardware/Input/ButtonFactory.h"
#include "Hardware/Interface/InterfaceFactory.h"
#include "Network/Protocol/RolandClientFactory.h"
#include "Network/WebConfigServer.h"
#include "Utils/InfoPrinter.h"

// Add these 'using' declarations
using Display::DisplayFactory;
using Hardware::IMUFactory;
using Hardware::ButtonFactory;
using Hardware::InterfaceFactory;

namespace Application {

    STACApp::STACApp()
        : initialized( false )
        , stacID( "" )
        , provisioningFromBootButton( false )
        , lastRolandPoll( 0 )
        , rolandPollInterval( 300 )
        , rolandClientInitialized( false )
        , buttonPollTimer( nullptr ) {
        // unique_ptr members default to nullptr
    }

    bool STACApp::setup() {
        // Initialize hardware
        if ( !initializeHardware() ) {
            log_e( "Hardware initialization failed" );
            return false;
        }

        // Initialize network and storage
        if ( !initializeNetworkAndStorage() ) {
            log_e( "Network/Storage initialization failed" );
            return false;
        }

        // Show green power glyph to indicate successful initialization
        // (orange was shown during hardware init, green = all systems ready)
        const uint8_t *powerGlyph = Display::BASE_GLYPHS[ Display::GLF_PO ];
        display->drawGlyph( powerGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
        delay( 750 );  // Hold green power glyph for 750ms

        // Initialize system state
        systemState = std::make_unique<State::SystemState>();
        if ( !systemState->begin() ) {
            log_e( "System state initialization failed" );
            return false;
        }

        // Determine operating mode (after showing green = successful init)
        OperatingMode mode = determineOperatingMode();
        systemState->getOperatingMode().setMode( mode );

        log_i( "Operating Mode: %s", systemState->getOperatingMode().getModeString() );
        log_i( "STAC ID: %s", stacID.c_str() );

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Configure GROVE port based on mode
        bool isOutput = !systemState->getOperatingMode().isPeripheralMode();
        // grovePort = Hardware::InterfaceFactory::createGrovePort( isOutput );
        grovePort = InterfaceFactory::createGrovePort( isOutput );
        #endif

        // Set up state change callback
        systemState->getTallyState().setStateChangeCallback(
        [ this ]( TallyState oldState, TallyState newState ) {
            log_i( "Tally: %s -> %s",
                   State::TallyStateManager::stateToString( oldState ),
                   State::TallyStateManager::stateToString( newState ) );
            updateDisplay();

            #if HAS_PERIPHERAL_MODE_CAPABILITY
            // Update GROVE output if in normal mode
            if ( systemState->getOperatingMode().isNormalMode() ) {
                grovePort->setTallyState( newState );
            }
            #endif
        }
        );

        initialized = true;

        // Handle provisioning mode if needed (blocking call)
        if ( systemState->getOperatingMode().getCurrentMode() == OperatingMode::PROVISIONING ) {
            handleProvisioningMode( provisioningFromBootButton );
            // After provisioning completes, device will restart
        }

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Initialize GROVE port GPIO pins (only in normal mode - peripheral mode is already handled)
        // This prevents floating pins from causing artifacts on peripheral mode devices
        if ( systemState->getOperatingMode().isNormalMode() ) {
            // GROVE port is already configured above, but we need to set initial state
            grovePort->setTallyState( TallyState::ERROR );  // Set to ERROR (both pins LOW)
            log_i( "GROVE port initialized to UNKNOWN state" );
        }
        #endif

        // Create startup config handler (dimension-agnostic using type alias)
        #if defined(BUTTON_B_PIN)
        startupConfig = std::make_unique<Application::StartupConfigType>(
                            button,
                            display.get(),
                            glyphManager.get(),
                            configManager.get(),
                            buttonB
                        );
        #else
        startupConfig = std::make_unique<Application::StartupConfigType>(
                            button,
                            display.get(),
                            glyphManager.get(),
                            configManager.get()
                        );
        #endif

        return true;
    }

    void STACApp::loop() {
        if ( !initialized ) {
            return;
        }

        // Button polling is now handled by esp_timer (2ms interval)
        // Just handle button events here
        #if defined(BUTTON_B_PIN)
        handleButtonB();
        #endif

        // Handle button input
        handleButton();

        // Update managers
        wifiManager->update();
        systemState->update();

        // Mode-specific handling
        switch ( systemState->getOperatingMode().getCurrentMode() ) {
            case OperatingMode::NORMAL:
                handleNormalMode();
                break;

                #if HAS_PERIPHERAL_MODE_CAPABILITY
            case OperatingMode::PERIPHERAL:
                handlePeripheralMode();
                break;
                #endif

            case OperatingMode::PROVISIONING:
                // Provisioning is handled once in setup(), not in loop
                break;
        }
    }

    bool STACApp::initializeHardware() {
        // Turn on status LED during hardware initialization
        statusLedOn();

        // For TFT displays: Assert LCD reset BEFORE IMU initialization
        // This keeps the LCD electrically quiet during sensitive IMU readings
        // and provides the required reset hold time during IMU init
        #if defined(TFT_RST) && (TFT_RST >= 0)
        unsigned long tftResetStartTime = millis();
        pinMode( TFT_RST, OUTPUT );
        digitalWrite( TFT_RST, HIGH );
        delay( 10 );  // Brief HIGH before reset
        digitalWrite( TFT_RST, LOW );  // Assert reset (active low)
        log_i( "TFT reset asserted (LCD silent during IMU init)" );
        #endif

        // IMU - Initialize FIRST to detect orientation before display init
        // This allows display to be created with correct rotation from the start
        imu = IMUFactory::create();
        Orientation displayOrientation = Orientation::ROTATE_0;  // Default if IMU unavailable

        if ( imu->begin() ) {
            #if IMU_HAS_IMU
            // Board has IMU - detect orientation and apply LUT mapping
            log_i( "✓ IMU (%s)", imu->getType() );
            delay( 100 );  // Allow IMU readings to stabilize after power-on

            Orientation detectedOrientation = imu->getOrientation();

            if ( detectedOrientation != Orientation::UNKNOWN ) {
                // Apply board-specific LUT mapping: physical orientation → display rotation
                // LUT includes all orientations (0-3) plus FLAT(4) and UNKNOWN(5)
                static const Orientation lutMap[] = DEVICE_ORIENTATION_TO_LUT_MAP;
                displayOrientation = lutMap[ static_cast<int>( detectedOrientation ) ];

                // Log orientation info
                const char *lutNames[] = {"LUT_ROTATE_0", "LUT_ROTATE_90", "LUT_ROTATE_180", "LUT_ROTATE_270", "LUT_FLAT", "LUT_UNKNOWN"};

                log_i( "  LUT being used: %s", lutNames[ static_cast<int>( displayOrientation ) ] );

                // Log physical orientation
                if ( detectedOrientation == Orientation::FLAT ) {
                    log_i( "  Physical device orientation: FLAT" );
                }
                else if ( detectedOrientation == Orientation::UNKNOWN ) {
                    log_i( "  Physical device orientation: UNKNOWN" );
                }
                else {
                    static const int enumToPhysical[] = ORIENTATION_ENUM_TO_PHYSICAL_ANGLE;
                    int physicalAngle = enumToPhysical[ static_cast<int>( detectedOrientation ) ];
                    log_i( "  Physical device orientation: %d°", physicalAngle );
                }
            }
            #else
            // Board has no IMU - just log and use default orientation
            log_i( "✓ IMU (%s)", imu->getType() );
            log_i( "  Initial orientation: UP" );
            #endif
        }
        else {
            log_w( "⚠ IMU unavailable" );
        }

        // For TFT displays: Ensure minimum reset hold time, then release reset
        // Conservative 200ms total (safe for all controllers: ST7735=50ms, ST7789=120ms, GC9A01=200ms)
        #if defined(TFT_RST) && (TFT_RST >= 0)
        unsigned long elapsedResetTime = millis() - tftResetStartTime;
        const unsigned long MIN_RESET_HOLD_MS = 200;  // Conservative value for all TFT controllers

        if ( elapsedResetTime < MIN_RESET_HOLD_MS ) {
            delay( MIN_RESET_HOLD_MS - elapsedResetTime );
        }

        // Release reset and allow LCD to stabilize
        digitalWrite( TFT_RST, HIGH );
        delay( 200 );  // Post-reset stabilization time
        log_i( "TFT reset released and stabilized" );
        #endif

        // Display - Initialize with correct configuration from board config
        display = DisplayFactory::create();

        // Set initial rotation: use IMU-detected orientation for boards with IMU,
        // or board-specific physical rotation for boards without IMU
        #ifdef DISPLAY_PHYSICAL_ROTATION
        display->setInitialRotation( DISPLAY_PHYSICAL_ROTATION );
        #else
        display->setInitialRotation( static_cast<uint8_t>( displayOrientation ) );
        #endif

        if ( !display->begin() ) {
            log_e( "Display initialization failed" );
            return false;
        }

        // Immediately show orange power glyph (display is already on from begin())
        const uint8_t *earlyPowerGlyph = Display::BASE_GLYPHS[ Display::GLF_PO ];
        display->drawGlyph( earlyPowerGlyph, Display::StandardColors::ORANGE, Display::StandardColors::BLACK, Config::Display::SHOW );

        // Turn on backlight for TFT displays (LED displays ignore this)
        // Use default brightness level 1 for initial power-on display
        uint8_t initialBrightness = Config::Display::BRIGHTNESS_MAP[ 1 ];
        display->setBrightness( initialBrightness, Config::Display::SHOW );

        log_i( "✓ Display (%s)", DisplayFactory::getDisplayType() );

        // Button - Create XP_Button directly
        // puEnable must be false for input-only GPIOs that can't use internal pullup
        bool enableInternalPullup = Config::Button::ACTIVE_LOW && !Config::Button::NEEDS_EXTERNAL_PULLUP;
        button = new Button(
            Config::Pins::BUTTON,
            Config::Button::DEBOUNCE_MS,
            enableInternalPullup,          // puEnable: only if active low AND has internal pullup capability
            Config::Button::ACTIVE_LOW     // invert: true for active low
        );
        button->begin();
        log_i( "✓ Button" );

        // Button B (secondary) - for devices without hardware reset button
        #if defined(BUTTON_B_PIN)
        buttonB = new Button(
            BUTTON_B_PIN,
            Config::Button::DEBOUNCE_MS,
            BUTTON_B_ACTIVE_LOW,    // puEnable: true for active low (needs pullup)
            BUTTON_B_ACTIVE_LOW     // invert: true for active low
        );
        buttonB->begin();
        log_i( "✓ Button B (reset)" );
        #endif

        // Start automatic button polling via esp_timer (2ms interval)
        // Then wait for button state to stabilize (1.5× debounce time)
        // After this delay, all XP_Button methods return valid state
        startButtonPolling();
        delay( Config::Button::DEBOUNCE_MS + ( Config::Button::DEBOUNCE_MS / 2 ) );
        log_i( "✓ Button polling timer (stabilized)" );

        // GlyphManager - initialize with mapped display orientation (dimension-agnostic using type alias)
        glyphManager = std::make_unique<Display::GlyphManagerType>( displayOrientation );
        log_i( "✓ GlyphManager" );

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Peripheral mode: Button-based selection (jumper detection removed in v3)
        log_i( "✓ Peripheral mode capability available" );
        #endif

        // Turn off status LED - hardware initialization complete
        statusLedOff();

        return true;
    }

    // Static callback function for esp_timer - must be at namespace scope
    static Button *s_button = nullptr;
#if defined(BUTTON_B_PIN)
    static Button *s_buttonB = nullptr;
#endif

    static void buttonPollCallback( void* arg ) {
        if ( s_button ) {
            s_button->read();
        }
        #if defined(BUTTON_B_PIN)
        if ( s_buttonB ) {
            s_buttonB->read();
        }
        #endif
    }

    void STACApp::startButtonPolling() {
        // Store button pointers for static callback
        s_button = button;
        #if defined(BUTTON_B_PIN)
        s_buttonB = buttonB;
        #endif

        // Create esp_timer for button polling
        esp_timer_create_args_t timerArgs = {
            .callback = buttonPollCallback,
            .arg = nullptr,
            .dispatch_method = ESP_TIMER_TASK,
            .name = "button_poll",
            .skip_unhandled_events = true
        };

        esp_err_t err = esp_timer_create( &timerArgs, &buttonPollTimer );
        if ( err != ESP_OK ) {
            log_e( "Failed to create button poll timer: %s", esp_err_to_name( err ) );
            return;
        }

        // Start periodic timer at 2ms (2000 microseconds)
        err = esp_timer_start_periodic( buttonPollTimer, 2000 );
        if ( err != ESP_OK ) {
            log_e( "Failed to start button poll timer: %s", esp_err_to_name( err ) );
        }
    }

    bool STACApp::initializeNetworkAndStorage() {
        // Config Manager
        configManager = std::make_unique<Storage::ConfigManager>();
        if ( !configManager->begin() ) {
            log_e( "Config manager initialization failed" );
            return false;
        }
        log_i( "✓ Config Manager" );

        // Load or generate STAC ID
        if ( !configManager->loadStacID( stacID ) ) {
            stacID = configManager->generateAndSaveStacID();
            log_i( "  Generated STAC ID: %s", stacID.c_str() );
        }
        else {
            // Validate loaded ID format (should contain hyphen)
            if ( stacID.indexOf( '-' ) == -1 ) {
                log_w( "  Invalid STAC ID format detected: %s", stacID.c_str() );
                log_w( "  Regenerating STAC ID..." );
                stacID = configManager->generateAndSaveStacID();
                log_i( "  Generated STAC ID: %s", stacID.c_str() );
            }
            else {
                log_i( "  STAC ID: %s", stacID.c_str() );
            }
        }

        // Print startup header to serial
        Utils::InfoPrinter::printHeader( stacID );

        // WiFi Manager
        wifiManager = std::make_unique<Net::WiFiManager>();
        if ( !wifiManager->begin() ) {
            log_e( "WiFi manager initialization failed" );
            return false;
        }
        wifiManager->setHostname( stacID );
        log_i( "✓ WiFi Manager" );

        return true;
    }

    OperatingMode STACApp::determineOperatingMode() {
        // Check for button hold at boot FIRST (PMode toggle/provisioning/factory reset/OTA)
        // This allows peripheral mode entry even when device is not configured
        OperatingMode bootMode = checkBootButtonSequence();
        if ( bootMode == OperatingMode::PROVISIONING ) {
            log_i( "Boot button: Forced provisioning mode" );
            provisioningFromBootButton = true;
            return OperatingMode::PROVISIONING;
        }
        if ( bootMode == OperatingMode::PERIPHERAL ) {
            log_i( "Boot button: Peripheral mode selected" );
            return OperatingMode::PERIPHERAL;
        }
        // Note: Factory reset and OTA modes restart the device, so we never return from them

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Check NVS for peripheral mode setting (replaces hardware jumper detection)
        if ( configManager->loadPModeEnabled() ) {
            log_i( "Peripheral mode enabled in NVS" );
            return OperatingMode::PERIPHERAL;
        }
        #endif

        // Check if device is provisioned - if not, enter provisioning mode
        // This only affects normal mode path (P mode can operate without configuration)
        if ( !configManager->isProvisioned() ) {
            log_i( "Device not provisioned, entering provisioning mode" );
            Serial.println( "      ***** STAC not configured *****" );
            return OperatingMode::PROVISIONING;
        }

        log_i( "Configuration found, starting in NORMAL mode" );
        return OperatingMode::NORMAL;
    }

    void STACApp::handleButton() {
        // Long press during normal operation: Adjust brightness
        static bool longPressHandled = false;

        if ( button->pressedFor( Config::Timing::BUTTON_SELECT_MS ) ) {
            if ( !longPressHandled ) {
                longPressHandled = true;

                #if HAS_PERIPHERAL_MODE_CAPABILITY
                // Set GROVE port to UNKNOWN state while adjusting settings
                // This prevents peripheral devices from showing false tally information
                grovePort->setTallyState( TallyState::ERROR );
                #endif

                // Call brightness adjustment
                StacOperations ops = systemState->getOperations();
                uint8_t oldBrightness = ops.displayBrightnessLevel;

                // Run brightness change (will show checkerboard and allow selection)
                if ( startupConfig ) {
                    // Use a temporary flag to indicate we're in runtime brightness adjust
                    startupConfig->changeBrightness( ops );

                    // Save if changed
                    if ( ops.displayBrightnessLevel != oldBrightness ) {
                        systemState->setOperations( ops );
                        // Save to protocol-specific namespace
                        bool saved = false;
                        if ( ops.isV60HD() ) {
                            saved = configManager->saveV60HDConfig( ops );
                        }
                        else if ( ops.isV160HD() ) {
                            saved = configManager->saveV160HDConfig( ops );
                        }
                        if ( !saved ) {
                            log_e( "Failed to save brightness level" );
                        }
                    }

                    // Restore the display
                    updateDisplay();
                }
            }
            return;  // Don't process short press while long press is active
        }

        // Reset long press flag when button is released
        if ( button->isReleased() && longPressHandled ) {
            longPressHandled = false;
        }
        // Note: Short press not used in normal operation

    }

    /**
     * @brief Handle Button B events - reset on press (devices without hardware reset button)
     * Note: Button state is updated automatically by esp_timer
     */
    void STACApp::handleButtonB() {
        #if defined(BUTTON_B_PIN)
        // Button B press triggers immediate restart (level-triggered for reliability)
        if ( buttonB->isPressed() ) {
            log_i( "Button B pressed - Restarting..." );
            Serial.println( "\n*** Button B pressed - Restarting... ***" );
            display->setBrightness( 0 );  // Turn off backlight before restart
            delay( 100 );  // Allow serial to flush
            ESP.restart();
        }
        #endif
    }

    /**
     * @brief Update display based on current tally state
     */
    void STACApp::updateDisplay() {
        TallyState currentState = systemState->getTallyState().getCurrentState();
        StacOperations ops = systemState->getOperations();

        // Draw the tally state (without showing yet)
        if ( currentState == TallyState::UNSELECTED ) {
            if ( ops.cameraOperatorMode ) {
                // Camera Operator mode: Show dotted frame glyph
                using namespace Display;
                const uint8_t *glyphData = glyphManager->getGlyph( Display::GLF_DF );
                display->drawGlyph( glyphData,
                                    Display::StandardColors::PURPLE,
                                    Display::StandardColors::BLACK,
                                    false );  // Don't show yet
            }
            else {
                // Talent mode: Show solid green
                display->fill( Display::StandardColors::GREEN, Config::Display::NO_SHOW );  // Don't show yet
            }
        }
        else {
            // All other states: Fill with state color
            Display::color_t color = systemState->getTallyState().getStateColor();
            display->fill( color, Config::Display::NO_SHOW );  // Don't show yet
        }

        // Overlay power-on indicator glyph
        // After orientation is determined, use rotated glyphs from GlyphManager
        const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
        display->drawGlyphOverlay( powerGlyph, Display::StandardColors::ORANGE, Config::Display::NO_SHOW );

        // Now show the complete display
        display->show();
    }

    void STACApp::displayWiFiStatus( Net::WiFiState state ) {
        using namespace Config::Timing;
        using namespace Display;

        const uint8_t *wifiGlyph = glyphManager->getGlyph( Display::GLF_WIFI );

        switch ( state ) {
            case Net::WiFiState::CONNECTING: {
                // Show orange WiFi glyph while attempting connection
                display->drawGlyph( wifiGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::SHOW );
                log_i( "WiFi: Attempting connection (orange glyph displayed)" );
                break;
            }

            case Net::WiFiState::CONNECTED: {
                // Show green WiFi glyph on successful connection
                display->drawGlyph( wifiGlyph, StandardColors::GREEN, StandardColors::BLACK, Config::Display::SHOW );
                log_i( "WiFi: Connected (green glyph displayed)" );

                // Print WiFi connected status to serial
                Utils::InfoPrinter::printWiFiConnected();

                delay( GUI_PAUSE_MS );  // Pause to show success

                // Clear display and show power pixel
                // After orientation is determined, use rotated glyphs from GlyphManager
                display->fill( StandardColors::BLACK, Config::Display::NO_SHOW );

                const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::NO_SHOW );
                display->show();
                break;
            }

            case Net::WiFiState::FAILED: {
                // Flash red WiFi glyph on timeout
                display->drawGlyph( wifiGlyph, StandardColors::RED, StandardColors::BLACK, Config::Display::SHOW );
                log_e( "WiFi: Connection timeout (flashing red glyph)" );
                display->flash( 8, 300, display->getBrightness() );  // Flash 8 times at 300ms intervals
                delay( GUI_PAUSE_MS );

                // Show orange glyph again for retry attempt
                display->drawGlyph( wifiGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::SHOW );
                break;
            }

            default:
                break;
        }
    }

    void STACApp::handleNormalMode() {
        // Ensure WiFi is connected
        static bool wifiAttempted = false;
        static bool interactiveConfigDone = false;

        // Switch configuration - loaded once and reused for Roland client initialization
        static IPAddress switchIP;
        static uint16_t switchPort = 0;
        static String username;
        static String passwordSwitch;
        static bool switchConfigLoaded = false;

        // Run startup configuration sequence (once)
        if ( !interactiveConfigDone ) {
            interactiveConfigDone = true;

            // Load protocol-specific operations from NVS
            StacOperations ops;
            String protocol = configManager->getActiveProtocol();
            bool opsLoaded = false;

            // @Claude: another place where we can simplify if we define an enum for the switch models
            if ( protocol == "V-60HD" ) {
                opsLoaded = configManager->loadV60HDConfig( ops );
            }
            else if ( protocol == "V-160HD" ) {
                opsLoaded = configManager->loadV160HDConfig( ops );
            }

            // @Claude: Again, should only be one place where we check if we're provisioned or not. Defaults should be loaded int ops parameters based on the configured switch at startup
            if ( !opsLoaded ) {
                log_e( "Failed to load protocol configuration from NVS for %s", protocol.c_str() );
                // This should never happen if device is properly provisioned
                // Use system default but log critical error
                ops = StacOperations();  // Use explicit default construction

                // Set switchModel from active protocol if available
                if ( !protocol.isEmpty() ) {
                    ops.switchModel = protocol;
                }
                else {
                    log_e( "CRITICAL: No active protocol found and config load failed!" );
                }
            }
            else {
                log_i( "Loaded configuration: channel=%d, model=%s, autoStart=%s",
                       ops.tallyChannel, ops.switchModel.c_str(), ops.autoStartEnabled ? "YES" : "NO" );
            }

            // Update system state with loaded operations IMMEDIATELY to preserve them
            systemState->setOperations( ops );

            // Apply the stored brightness level to the display hardware
            {
                uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ ops.displayBrightnessLevel ];
                display->setBrightness( absoluteBrightness, Config::Display::SHOW );
                log_i( "Applied brightness level %d", ops.displayBrightnessLevel );
            }

            // Load switch configuration for printing and Roland client initialization
            // @Claude: I guess using an enum for the switch models can be problematic if we want to print the model name. Maybe we can have a function that converts from enum to string for printing purposes?
            String ssid, password;
            switchConfigLoaded = configManager->loadSwitchConfig( ops.switchModel, switchIP, switchPort, username, passwordSwitch );

            // Print configuration summary to serial (always, before autostart check)
            if ( switchConfigLoaded && configManager->loadWiFiCredentials( ssid, password ) ) {
                Utils::InfoPrinter::printFooter( ops, switchIP, switchPort, ssid );
            }

            // Display the active tally channel (always, regardless of autostart setting)
            // For V-160HD SDI channels (9-20), display the channel within bank (1-8)
            uint8_t displayChannel = ops.tallyChannel;
            if ( ops.switchModel != "V-60HD" && ops.tallyChannel > 8 ) {
                displayChannel = ops.tallyChannel - 8;  // SDI 9→1, 10→2, etc.
            }
            const uint8_t *channelGlyph = glyphManager->getDigitGlyph( displayChannel );

            // Channel and autostart colors depend on switch model and channel bank
            Display::color_t channelColor;
            Display::color_t autostartColor;

            if ( ops.switchModel != "V-60HD" && ops.tallyChannel > 8 ) {
                // V-160HD second bank (SDI channels 9-20)
                channelColor = Display::StandardColors::LIGHT_GREEN;
                autostartColor = Display::StandardColors::BLUE;
            }
            else {
                // V-60HD or V-160HD first bank (HDMI channels 1-8)
                channelColor = Display::StandardColors::BLUE;
                autostartColor = Display::StandardColors::BRIGHT_GREEN;
            }

            // Clear any previous display state before showing channel glyph
            display->clear( Config::Display::NO_SHOW );
            display->drawGlyph( channelGlyph, channelColor, Display::StandardColors::BLACK, Config::Display::SHOW );

            // Wait for button release before proceeding
            while ( button->isPressed() ) {
                // Button state updated by esp_timer, just wait
                delay( 1 );
            }

            // Check if autostart is enabled
            bool autoStartBypass = false;

            if ( ops.autoStartEnabled ) {
                // Autostart mode: Add blinking corners and wait for timeout or button press
                log_i( "Autostart mode active - waiting for timeout or button press" );

                using namespace Config::Timing;
                using namespace Display;

                // Get corners glyph for pulsing
                const uint8_t *cornersGlyph = glyphManager->getGlyph( Display::GLF_CORNERS );
                display->pulseCorners( cornersGlyph, true, autostartColor );

                unsigned long autostartTimeout = millis() + AUTOSTART_TIMEOUT_MS;
                unsigned long nextFlash = millis() + AUTOSTART_PULSE_MS;
                bool cornersOn = true;

                while ( millis() < autostartTimeout ) {
                    // Button state updated by esp_timer

                    // Check Button B for reset
                    #if defined(BUTTON_B_PIN)
                    handleButtonB();  // Triggers ESP.restart() if pressed
                    #endif

                    // Button pressed: Cancel autostart
                    if ( button->isPressed() ) {
                        log_i( "Button pressed - cancelling autostart" );
                        autoStartBypass = false;
                        break;
                    }

                    // Flash corner pixels
                    if ( millis() >= nextFlash ) {
                        nextFlash = millis() + AUTOSTART_PULSE_MS;
                        cornersOn = !cornersOn;

                        // Only pulse corners - channel glyph already drawn
                        display->pulseCorners( cornersGlyph, cornersOn, autostartColor );
                    }
                }

                // If we timed out (no button press), bypass startup config
                if ( millis() >= autostartTimeout ) {
                    autoStartBypass = true;
                    log_i( "Autostart timeout - bypassing startup config" );
                }
            }

            // Run startup configuration (unless autostart bypassed)
            if ( startupConfig->runStartupSequence( ops, autoStartBypass ) ) {
                // Update operations in system state
                systemState->setOperations( ops );

                // Save to protocol-specific namespace
                bool saved = false;
                if ( ops.isV60HD() ) {
                    saved = configManager->saveV60HDConfig( ops );
                }
                else if ( ops.isV160HD() ) {
                    saved = configManager->saveV160HDConfig( ops );
                }
                if ( !saved ) {
                    log_e( "Failed to save protocol configuration after startup" );
                }
            }
        }

        if ( !wifiAttempted && !wifiManager->isConnected() && configManager->hasWiFiCredentials() ) {
            wifiAttempted = true;

            String ssid, password;
            if ( configManager->loadWiFiCredentials( ssid, password ) ) {
                log_i( "Attempting to connect to WiFi: %s", ssid.c_str() );

                // Set callback for visual feedback
                wifiManager->setStateCallback(
                [ this ]( Net::WiFiState state ) {
                    displayWiFiStatus( state );
                }
                );

                wifiManager->connect( ssid, password );
            }
        }

        // Initialize Roland client if WiFi connected and not yet initialized
        if ( wifiManager->isConnected() && !rolandClientInitialized && switchConfigLoaded ) {
            if ( initializeRolandClient( switchIP, switchPort, username, passwordSwitch ) ) {
                rolandClientInitialized = true;
                log_i( "Roland client initialized" );
                // Display will be updated by first tally response
            }
        }

        // Poll Roland switch if initialized
        if ( rolandClientInitialized ) {
            pollRolandSwitch();
        }
    }

    #if HAS_PERIPHERAL_MODE_CAPABILITY
    void STACApp::handlePeripheralMode() {
        using namespace Display;
        using namespace Config::Timing;

        // Get glyph indices based on display size
        using namespace Display;

        log_i( "Entering Peripheral Mode" );

        // Configure GROVE GPIO pins as inputs for reading tally state
        grovePort->configurePinsAsInputs();
        log_i( "GROVE port configured as inputs for peripheral mode" );

        // ===== Load or initialize peripheral mode settings =====
        bool cameraMode = false;  // Default: talent mode
        uint8_t brightnessLevel = 1;  // Default: lowest brightness

        if ( !configManager->loadPeripheralSettings( cameraMode, brightnessLevel ) ) {
            // First time in peripheral mode - save defaults
            log_i( "First time in peripheral mode - using defaults" );
            configManager->savePeripheralSettings( cameraMode, brightnessLevel );
        }

        // Apply brightness
        uint8_t absoluteBrightness = Config::Display::BRIGHTNESS_MAP[ brightnessLevel ];
        display->setBrightness( absoluteBrightness, Config::Display::NO_SHOW );

        // Print peripheral mode status to serial (include configuration state)
        bool isProvisioned = configManager->isProvisioned();
        Utils::InfoPrinter::printPeripheral( cameraMode, brightnessLevel, isProvisioned );

        // ===== Startup animation =====
        // Show "P" glyph in green (perifmodecolor)
        const uint8_t *pGlyph = glyphManager->getGlyph( Display::GLF_P );
        display->drawGlyph( pGlyph, StandardColors::GREEN, StandardColors::BLACK, Config::Display::SHOW );

        // Flash display 4 times
        for ( int i = 0; i < 4; i++ ) {
            delay( 250 );
            display->clear( Config::Display::SHOW );
            delay( 250 );
            display->drawGlyph( pGlyph, StandardColors::GREEN, StandardColors::BLACK, Config::Display::SHOW );
        }

        delay( GUI_PAUSE_MS );

        // Show power-on glyph as orange pixel on green background
        const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
        display->clear( Config::Display::NO_SHOW );
        display->fill( StandardColors::GREEN, Config::Display::NO_SHOW );
        display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::NO_SHOW );
        display->show();

        // Wait for button release
        while ( button->isPressed() ) {
            delay( 1 );
        }

        log_i( "Peripheral mode initialized: camera=%s, brightness=%d",
               cameraMode ? "true" : "false", brightnessLevel );

        // ===== Main peripheral mode loop =====
        uint8_t lastTallyState = Config::Peripheral::INVALID_STATE;
        unsigned long nextCheck = 0;

        while ( true ) {
            // Read tally state from Grove port
            if ( millis() >= nextCheck ) {
                nextCheck = millis() + PM_POLL_INTERVAL_MS;

                TallyState receivedState = grovePort->readTallyState();

                // Convert to numeric state for comparison
                uint8_t currentState;
                switch ( receivedState ) {
                    case TallyState::PROGRAM:
                        currentState = 3;
                        break;
                    case TallyState::PREVIEW:
                        currentState = 2;
                        break;
                    case TallyState::UNSELECTED:
                        currentState = 1;
                        break;
                    default:
                        currentState = 0;
                        break;
                }

                // Update display if state changed
                if ( currentState != lastTallyState ) {
                    lastTallyState = currentState;

                    switch ( receivedState ) {
                        case TallyState::PROGRAM: {
                            // Red (program) with orange square overlay
                            display->fill( StandardColors::RED, Config::Display::NO_SHOW );
                            const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                            display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            break;
                        }

                        case TallyState::PREVIEW: {
                            // Green (preview) with orange square overlay
                            display->fill( StandardColors::GREEN, Config::Display::NO_SHOW );
                            const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                            display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            break;
                        }

                        case TallyState::UNSELECTED: {
                            if ( cameraMode ) {
                                // Camera mode: Show dark frame glyph in purple with orange square
                                const uint8_t *dfGlyph = glyphManager->getGlyph( Display::GLF_DF );
                                display->drawGlyph( dfGlyph, StandardColors::PURPLE, StandardColors::BLACK, Config::Display::NO_SHOW );
                            }
                            else {
                                // Talent mode: Show green with orange square
                                display->fill( StandardColors::GREEN, Config::Display::NO_SHOW );
                            }
                            const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                            display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            break;
                        }

                        default: {
                            // Error/unknown state - no orange square (ATOM behavior)
                            if ( cameraMode ) {
                                // Camera mode: Show orange X
                                const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_BX );
                                display->drawGlyph( xGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::SHOW );
                            }
                            else {
                                // Talent mode: Show green with orange power pixel
                                display->fill( StandardColors::GREEN, Config::Display::NO_SHOW );
                                const uint8_t *powerGlyph = glyphManager->getGlyph( Display::GLF_PO );
                                display->drawGlyphOverlay( powerGlyph, StandardColors::ORANGE, Config::Display::SHOW );
                            }
                            break;
                        }
                    }
                }
            }

            // Handle button for settings adjustment
            // Button state updated by esp_timer
            handleButtonB();  // Check for Button B reset

            if ( button->pressedFor( BUTTON_SELECT_MS ) ) {
                // User wants to change peripheral mode settings

                // Show brightness selection screen
                display->fill( StandardColors::WHITE, Config::Display::NO_SHOW );

                // Blank center columns
                const uint8_t *centerBlank = glyphManager->getGlyph( Display::GLF_EN );
                display->drawGlyphOverlay( centerBlank, StandardColors::BLACK, Config::Display::NO_SHOW );

                // Show current brightness level
                const uint8_t *levelGlyph = glyphManager->getDigitGlyph( brightnessLevel );
                display->drawGlyphOverlay( levelGlyph, StandardColors::ORANGE, Config::Display::SHOW );

                // State machine: brightness adjustment or mode change
                // Wait for release (brightness) or keep holding (mode change)
                bool exitSettings = false;
                unsigned long modeChangeTimeout = millis() + BUTTON_SELECT_MS;

                do {
                    // Button state updated by esp_timer
                    handleButtonB();  // Check for Button B reset

                    // Released before timeout: Enter brightness adjustment
                    if ( button->isReleased() && ( modeChangeTimeout >= millis() ) ) {
                        // Use shared changeBrightness with peripheral save callback

                        brightnessLevel = startupConfig->changeBrightness( brightnessLevel,
                        [ this, &cameraMode ]( uint8_t newBrightness ) {
                            configManager->savePeripheralSettings( cameraMode, newBrightness );
                        } );
                        exitSettings = true;
                    }
                    // Still pressed after timeout: Enter mode change
                    else if ( button->isPressed() && ( modeChangeTimeout < millis() ) ) {
                        // Use shared changeCameraTalentMode with peripheral save callback
                        cameraMode = startupConfig->changeCameraTalentMode( cameraMode,
                        [ this, &brightnessLevel ]( bool newMode ) {
                            configManager->savePeripheralSettings( newMode, brightnessLevel );
                        } );
                        exitSettings = true;
                    }

                } while ( !exitSettings );

                // Force immediate tally state refresh by setting impossible value
                lastTallyState = Config::Peripheral::INVALID_STATE;
                nextCheck = 0;          // Force immediate check on next loop iteration
            }
        }
    }
    #endif // HAS_PERIPHERAL_MODE_CAPABILITY

    void STACApp::handleProvisioningMode( bool fromBootButton ) {
        log_i( "Entering unified portal mode (provisioning/OTA)" );

        // Check if device was already provisioned - affects display color
        // Provisioned: ORANGE (warning - proceeding will modify existing config)
        // Not provisioned: RED (alert - must provision to use device)
        bool wasProvisioned = configManager->isProvisioned();
        Display::color_t provisionColor = wasProvisioned
                                          ? Display::StandardColors::ORANGE
                                          : Display::StandardColors::RED;
        log_i( "Provisioning color: %s", wasProvisioned ? "ORANGE (already provisioned)" : "RED (not provisioned)" );

        // Create and start unified web portal server immediately
        Net::WebConfigServer configServer( stacID );

        if ( !configServer.begin() ) {
            log_e( "Failed to start portal server" );
            return;
        }

        // Get glyph indices
        using namespace Display;

        // Show GLF_CFG in appropriate color based on provisioned state
        const uint8_t *cfgGlyph = glyphManager->getGlyph( Display::GLF_CFG );
        const uint8_t normalBrightness = display->getBrightness();

        // Calculate dim brightness using adjacent brightness levels from the map
        // Find current level index in brightness map
        uint8_t currentLevel = 1;
        for ( uint8_t i = 1; i <= Config::Display::BRIGHTNESS_LEVELS; i++ ) {
            if ( Config::Display::BRIGHTNESS_MAP[ i ] == normalBrightness ) {
                currentLevel = i;
                break;
            }
        }
        // Pulse to adjacent level: down one level, or up one if already at minimum
        const uint8_t dimBrightness = ( currentLevel > 1 )
                                      ? Config::Display::BRIGHTNESS_MAP[ currentLevel - 1 ]
                                      : Config::Display::BRIGHTNESS_MAP[ 2 ];

        // Clear display and show config glyph
        // (for unprogrammed devices, this transitions from green power glyph)
        // (for programmed devices entering via boot button, this redraws after flash sequence)
        display->clear( Config::Display::SHOW );
        display->drawGlyph( cfgGlyph, provisionColor, Display::StandardColors::BLACK, Config::Display::SHOW );

        // For unprogrammed devices NOT from boot button, flash the config glyph before pulsing
        // (if from boot button, the boot sequence already did the flashing)
        if ( !wasProvisioned && !fromBootButton ) {
            delay( 500 );  // Hold static glyph for 500ms
            display->flash( 4, 250, normalBrightness );  // Flash 4 times at 250ms intervals
            display->setBrightness( normalBrightness );  // Restore brightness after flash
        }

        // Set up pulsing config glyph display callback using brightness modulation
        bool pulseState = false;
        configServer.setDisplayUpdateCallback( [ this, cfgGlyph, provisionColor, normalBrightness, dimBrightness, &pulseState ]() {
            display->pulseDisplay( cfgGlyph, provisionColor, Display::StandardColors::BLACK,
                                   pulseState, normalBrightness, dimBrightness );
        } );

        // Set up Button B reset check callback (for M5StickC Plus)
        #if defined(BUTTON_B_PIN)
        configServer.setResetCheckCallback( [ this ]() {
            // Button state updated by esp_timer
            return buttonB->isPressed();
        } );

        // Set up pre-restart callback to turn off backlight on TFT displays
        configServer.setPreRestartCallback( [ this ]() {
            display->setBrightness( 0 );
        } );
        #endif

        // Ensure brightness is set to normal before pulsing starts
        display->setBrightness( normalBrightness, Config::Display::NO_SHOW );

        // Wait for either configuration or OTA to complete
        Net::WebConfigServer::PortalResult result = configServer.waitForCompletion();

        // Handle result based on type
        if ( result.type == Net::WebConfigServer::PortalResultType::OTA_SUCCESS ) {
            // OTA succeeded - server will restart automatically
            log_i( "OTA update successful - restarting..." );
            const uint8_t *checkmarkGlyph = glyphManager->getGlyph( Display::GLF_CK );
            display->drawGlyph( checkmarkGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
            configServer.end();
            restartDevice( 1000 );
            // Never returns
        }
        else if ( result.type == Net::WebConfigServer::PortalResultType::OTA_FAILED ) {
            // OTA failed - show error and restart
            log_e( "OTA update failed: %s", result.otaResult.statusMessage.c_str() );
            const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_X );
            display->drawGlyph( xGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, Config::Display::SHOW );
            configServer.end();
            restartDevice( 3000 );
            // Never returns
        }
        else if ( result.type == Net::WebConfigServer::PortalResultType::FACTORY_RESET ) {
            // Factory reset requested from web portal
            log_i( "Factory reset requested from web portal" );
            configServer.end();

            // Show factory reset glyph (matching button-initiated behavior)
            const uint8_t *frGlyph = glyphManager->getGlyph( Display::GLF_FR );
            display->drawGlyph( frGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, Config::Display::SHOW );

            // Print factory reset notification to serial
            Utils::InfoPrinter::printReset();

            // Clear all NVS data using ConfigManager
            if ( !configManager->clearAll() ) {
                log_e( "Factory reset failed - NVS clear unsuccessful" );
            }
            else {
                log_i( "Factory reset complete" );
            }

            // Brief pause
            delay( Config::Timing::GUI_PAUSE_MS );

            // Flash display once to confirm (baseline behavior)
            // Use brightness level 1 for the flash
            uint8_t brightness = Config::Display::BRIGHTNESS_MAP[ 1 ];
            display->flash( 1, 500, brightness );

            // Park here forever showing the factory reset glyph (matching button behavior)
            // User must power cycle or press reset button to restart
            while ( true ) {
                #if defined(BUTTON_B_PIN)
                // Button state updated by esp_timer
                if ( buttonB->isPressed() ) {
                    log_i( "Button B pressed after factory reset - restarting" );
                    display->setBrightness( 0 );  // Turn off backlight before restart
                    ESP.restart();
                }
                #endif
                yield();
            }
            // Never returns
        }
        // else CONFIG_RECEIVED - continue with provisioning flow

        ProvisioningData provData = result.configData;

        // Show green checkmark to confirm receipt (matching baseline)
        const uint8_t *checkmarkGlyph = glyphManager->getGlyph( Display::GLF_CK );
        display->drawGlyph( checkmarkGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
        delay( 1000 );

        // Stop the web server
        configServer.end();

        // Save configuration to NVS
        log_i( "Saving configuration to NVS" );

        // Save WiFi credentials
        if ( !configManager->saveWiFiCredentials( provData.wifiSSID, provData.wifiPassword ) ) {
            log_e( "Failed to save WiFi credentials" );
            return;
        }

        // Convert IP string to IPAddress
        IPAddress switchIP;
        if ( !switchIP.fromString( provData.switchIPString ) ) {
            log_e( "Invalid IP address: %s", provData.switchIPString.c_str() );
            return;
        }

        // Save switch configuration
        String username = provData.lanUserID;
        String password = provData.lanPassword;

        if ( !configManager->saveSwitchConfig(
                    provData.switchModel,
                    switchIP,
                    provData.switchPort,
                    username,
                    password ) ) {
            log_e( "Failed to save switch configuration" );
            return;
        }

        // Create and save operations configuration
        StacOperations ops;
        ops.switchModel = provData.switchModel;
        ops.tallyChannel = 1; // Default to channel 1
        ops.statusPollInterval = provData.pollInterval;
        ops.displayBrightnessLevel = 1; // Default brightness
        ops.cameraOperatorMode = true; // Camera operator mode
        ops.autoStartEnabled = false;

        // Set model-specific parameters
        if ( provData.switchModel == "V-60HD" ) {
            ops.maxChannelCount = provData.maxChannel;
            ops.maxHDMIChannel = 0;
            ops.maxSDIChannel = 0;
            ops.channelBank = "";
        }
        else {   // V-160HD
            ops.maxChannelCount = 0;
            ops.maxHDMIChannel = provData.maxHDMIChannel;
            ops.maxSDIChannel = provData.maxSDIChannel;
            ops.channelBank = "hdmi_"; // Default to HDMI bank
        }

        // Save to protocol-specific namespace
        bool saved = false;
        if ( ops.isV60HD() ) {
            saved = configManager->saveV60HDConfig( ops );
        }
        else if ( ops.isV160HD() ) {
            saved = configManager->saveV160HDConfig( ops );
        }
        if ( !saved ) {
            log_e( "Failed to save protocol configuration" );
            return;
        }

        log_i( "Configuration saved successfully" );

        // Print configuration complete message to serial
        Utils::InfoPrinter::printConfigDone();

        // Checkmark already shown above - just delay before restart
        // Restart to apply new configuration
        log_i( "Restarting to apply configuration" );
        restartDevice( 1000 );
    }

    bool STACApp::initializeRolandClient( const IPAddress& switchIP, uint16_t switchPort,
                                          const String &username, const String &password ) {
        // Get operations from system state (already loaded during startup)
        StacOperations ops = systemState->getOperations();

        // Cache poll interval to avoid repeated NVS reads
        rolandPollInterval = ops.statusPollInterval;

        // Create Roland client based on switch model from operations
        rolandClient = Net::RolandClientFactory::createFromString( ops.switchModel );
        if ( !rolandClient ) {
            log_e( "Failed to create Roland client for model: %s", ops.switchModel.c_str() );
            return false;
        }

        // Build configuration
        Net::RolandConfig rolandConfig;
        rolandConfig.switchIP = switchIP;
        rolandConfig.switchPort = switchPort;
        rolandConfig.tallyChannel = ops.tallyChannel;
        rolandConfig.username = username;
        rolandConfig.password = password;
        rolandConfig.channelBank = ops.channelBank;
        rolandConfig.stacID = stacID;

        // Initialize the client
        if ( !rolandClient->begin( rolandConfig ) ) {
            log_e( "Failed to initialize Roland client" );
            rolandClient.reset();
            return false;
        }

        log_i( "Roland client ready: %s @ %s:%d (ch %d)",
               ops.switchModel.c_str(), switchIP.toString().c_str(), switchPort, ops.tallyChannel );

        return true;
    }

    void STACApp::pollRolandSwitch() {
        using namespace Display;
        using namespace Config::Timing;
        using namespace Config::Net;

        // Get glyph indices based on display size
        using namespace Display;

        // Check if it's time to poll
        unsigned long now = millis();
        if ( now - lastRolandPoll < rolandPollInterval ) {
            return;
        }

        // Don't poll if WiFi is not connected
        if ( !wifiManager->isConnected() ) {
            return;
        }

        // Get references to state
        SwitchState& switchState = systemState->getSwitchState();
        StacOperations& ops = systemState->getOperations();

        // Query tally status
        Net::TallyQueryResult result;
        rolandClient->queryTallyStatus( result );

        // Update last poll time AFTER query completes
        // This ensures we don't count the blocking HTTP request time as part of the interval
        lastRolandPoll = millis();

        // Update switch state from query result
        switchState.connected = result.connected;
        switchState.timeout = result.timedOut;
        switchState.noReply = !result.gotReply;
        switchState.currentTallyState = result.rawResponse;

        // ===== NORMAL OPERATION: Valid tally response =====
        if ( result.connected && result.gotReply ) {
            // Check if response is a valid tally state
            bool validResponse = false;
            TallyState newState = TallyState::UNSELECTED;

            switch ( result.status ) {
                case Net::TallyStatus::ONAIR:
                    newState = TallyState::PROGRAM;
                    validResponse = true;
                    break;
                case Net::TallyStatus::SELECTED:
                    newState = TallyState::PREVIEW;
                    validResponse = true;
                    break;
                case Net::TallyStatus::UNSELECTED:
                    newState = TallyState::UNSELECTED;
                    validResponse = true;
                    break;
                default:
                    // Junk/invalid reply
                    validResponse = false;
                    break;
            }

            if ( validResponse ) {
                // ===== Valid response - update tally state =====
                rolandPollInterval = ops.statusPollInterval;  // Use normal poll interval
                switchState.junkReply = false;
                switchState.junkReplyCount = 0;  // Clear error counters
                switchState.noReplyCount = 0;
                switchState.lastTallyState = switchState.currentTallyState;

                // Update tally state if changed
                if ( newState != systemState->getTallyState().getCurrentState() ) {
                    systemState->getTallyState().setState( newState );
                    log_i( "Tally: %s", State::TallyStateManager::stateToString( newState ) );
                }
                else {
                    // State unchanged, but we need to update display and GROVE in case we're recovering from error
                    updateDisplay();
                    #if HAS_PERIPHERAL_MODE_CAPABILITY
                    if ( systemState->getOperatingMode().isNormalMode() && grovePort ) {
                        grovePort->setTallyState( newState );
                    }
                    #endif
                }

                // Grove port will be updated by tally state change callback
            }
            else {
                // ===== Junk reply received =====
                rolandPollInterval = ERROR_REPOLL_MS;  // Use faster error polling
                switchState.junkReply = true;
                switchState.junkReplyCount++;
                switchState.lastTallyState = "JUNK";
                switchState.currentTallyState = "NO_TALLY";

                if ( switchState.junkReplyCount >= MAX_POLL_ERRORS ) {
                    // Hit error threshold - display error
                    switchState.junkReplyCount = 0;  // Reset counter

                    #if HAS_PERIPHERAL_MODE_CAPABILITY
                    // Set Grove to unknown state
                    if ( grovePort ) {
                        grovePort->setTallyState( TallyState::ERROR );
                    }
                    #endif

                    if ( ops.cameraOperatorMode ) {
                        // Camera operator mode: Show purple question mark
                        const uint8_t *qmGlyph = glyphManager->getGlyph( Display::GLF_QM );
                        display->drawGlyph( qmGlyph, StandardColors::PURPLE, StandardColors::BLACK, Config::Display::SHOW );
                        log_e( "Junk reply error - showing purple '?'" );
                    }
                    else {
                        // Talent mode: Show preview with power pixel
                        systemState->getTallyState().setState( TallyState::PREVIEW );
                    }
                }
            }
        }
        // ===== ERROR CONDITIONS =====
        else {
            switchState.currentTallyState = "NO_INIT";
            switchState.lastTallyState = "NO_TALLY";
            switchState.junkReplyCount = 0;  // Clear junk counter (not a junk reply error)
            rolandPollInterval = ERROR_REPOLL_MS;  // Use faster error polling

            if ( !result.connected && result.timedOut ) {
                // ===== Connection failed and timed out =====
                switchState.noReplyCount = 0;  // Clear no-reply counter

                #if HAS_PERIPHERAL_MODE_CAPABILITY
                // Set Grove to error state (immediate - connection timeout)
                if ( grovePort ) {
                    grovePort->setTallyState( TallyState::ERROR );
                }
                #endif

                if ( ops.cameraOperatorMode ) {
                    // Camera operator mode: Show orange X
                    const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_BX );
                    display->drawGlyph( xGlyph, StandardColors::ORANGE, StandardColors::BLACK, Config::Display::SHOW );
                    log_e( "Connection timeout - showing orange 'X'" );
                }
                else {
                    // Talent mode: Show preview with power pixel
                    systemState->getTallyState().setState( TallyState::PREVIEW );
                }
                handleButtonB();  // Allow reset while stuck in connection timeout
            }
            else if ( result.connected && ( result.timedOut || !result.gotReply ) ) {
                // ===== Connected but no reply or timed out =====
                rolandPollInterval = ERROR_REPOLL_MS;  // Use faster error polling
                switchState.noReplyCount++;

                // Don't update display or tally state until threshold is reached
                // Keep showing last valid state (or blank on first connection)

                if ( switchState.noReplyCount >= MAX_POLL_ERRORS ) {
                    // Hit error threshold
                    switchState.noReplyCount = 0;  // Reset counter
                    switchState.currentTallyState = "NO_INIT";
                    switchState.lastTallyState = "NO_TALLY";

                    #if HAS_PERIPHERAL_MODE_CAPABILITY
                    // Set Grove to error state (threshold reached)
                    if ( grovePort ) {
                        grovePort->setTallyState( TallyState::ERROR );
                    }
                    #endif

                    if ( ops.cameraOperatorMode ) {
                        // Camera operator mode: Show purple X (big purple X)
                        const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_BX );
                        display->drawGlyph( xGlyph, StandardColors::PURPLE, StandardColors::BLACK, Config::Display::SHOW );
                        log_e( "No reply error - showing purple 'X'" );

                        // Check for Button B reset while stuck in error state
                        handleButtonB();
                    }
                    else {
                        // Talent mode: Show preview with power pixel
                        systemState->getTallyState().setState( TallyState::PREVIEW );
                    }
                }
            }
            else {
                // ===== Some other error condition =====
                switchState.noReplyCount = 0;  // Clear counter

                #if HAS_PERIPHERAL_MODE_CAPABILITY
                // Set Grove to error state (unknown error)
                if ( grovePort ) {
                    grovePort->setTallyState( TallyState::ERROR );
                }
                #endif

                if ( ops.cameraOperatorMode ) {
                    // Camera operator mode: Show red X
                    const uint8_t *xGlyph = glyphManager->getGlyph( Display::GLF_BX );
                    display->drawGlyph( xGlyph, StandardColors::RED, StandardColors::BLACK, Config::Display::SHOW );
                    log_e( "Unknown error - showing red 'X'" );

                    // Check for Button B reset while stuck in error state
                    handleButtonB();
                }
                else {
                    // Talent mode: Show preview with power pixel
                    systemState->getTallyState().setState( TallyState::PREVIEW );
                }
            }
        }
    }

    void STACApp::handleFactoryReset() {
        log_i( "Performing factory reset" );

        // Print factory reset notification to serial
        Utils::InfoPrinter::printReset();

        // Clear all NVS data using ConfigManager
        if ( !configManager->clearAll() ) {
            log_e( "Factory reset failed - NVS clear unsuccessful" );
            return;
        }

        log_i( "Factory reset complete" );

        // Brief pause
        delay( Config::Timing::GUI_PAUSE_MS );

        // Flash display once to confirm (baseline behavior)
        // Use brightness level 1 for the flash
        uint8_t brightness = Config::Display::BRIGHTNESS_MAP[ 1 ];
        display->flash( 1, 500, brightness );

        // Park here forever showing the factory reset glyph (baseline behavior)
        // User must power cycle or press reset button to restart
        while ( true ) {
            #if defined(BUTTON_B_PIN)
            // Button state updated by esp_timer
            if ( buttonB->isPressed() ) {
                log_i( "Button B pressed after factory reset - restarting" );
                display->setBrightness( 0 );  // Turn off backlight before restart
                ESP.restart();
            }
            #endif
            yield();
        }
    }

    void STACApp::restartDevice( uint16_t delayMs ) {
        delay( delayMs );
        display->setBrightness( 0 );  // Turn off display before restart
        ESP.restart();
        // Never returns
    }

    void STACApp::showConfirmationCheckmark() {
        // Clear display, show green checkmark, pause, then clear
        display->clear( Config::Display::SHOW );
        // delay( Config::Timing::GUI_PAUSE_MS );
        const uint8_t *checkGlyph = glyphManager->getGlyph( Display::GLF_CK );
        display->drawGlyph( checkGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
        delay( Config::Timing::GUI_PAUSE_MS );
        display->clear( Config::Display::SHOW );
    }

    OperatingMode STACApp::checkBootButtonSequence() {
        // Button state is maintained by esp_timer - no manual read() needed
        bool buttonPressed = button->isPressed();
        log_i( "checkBootButtonSequence: button pressed = %s", buttonPressed ? "TRUE" : "FALSE" );

        // If button not pressed at boot, return NORMAL (will check config later)
        if ( !buttonPressed ) {
            return OperatingMode::NORMAL;
        }

        log_i( "Button held at boot - entering button sequence state machine" );

        // Get brightness for boot sequence flashing
        uint8_t nvsBrightness = display->getBrightness();

        // Check if device is provisioned - affects state machine behavior
        // If not provisioned: skip factory reset (device already in factory default state)
        //                     show RED glyph (user cannot proceed without provisioning)
        // If provisioned: show ORANGE glyph (warning that proceeding will modify config)
        //                 include factory reset option
        bool isProvisioned = configManager->isProvisioned();
        log_i( "Device provisioned: %s", isProvisioned ? "YES" : "NO" );

        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // Check current PMode setting to determine glyph/color for PMODE_PENDING state
        bool pmodeCurrentlyEnabled = configManager->loadPModeEnabled();
        log_i( "PMode currently enabled: %s", pmodeCurrentlyEnabled ? "YES" : "NO" );
        #endif

        // Button state machine timing (in milliseconds)
        static constexpr unsigned long STATE_HOLD_TIME = 2000;  // 2 seconds per state

        enum class BootButtonState {
            #if HAS_PERIPHERAL_MODE_CAPABILITY
            PMODE_PENDING,         // First state: Toggle peripheral mode (0-2 sec)
            #endif
            PROVISIONING_PENDING,  // Short hold -> unified portal (2-4 sec)
            FACTORY_RESET_PENDING  // Medium hold -> factory reset (4-6 sec)
        };

        // Determine starting state based on PMode capability
        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // PMode capable: Always start at PMODE_PENDING
        BootButtonState state = BootButtonState::PMODE_PENDING;
        #else
        // No PMode: Start at PROVISIONING_PENDING
        BootButtonState state = BootButtonState::PROVISIONING_PENDING;
        #endif

        unsigned long stateArmTime = millis() + STATE_HOLD_TIME;
        bool sequenceExit = false;
        OperatingMode resultMode = OperatingMode::NORMAL;

        // Get glyph indices
        using namespace Display;

        // Get all glyphs we'll need
        const uint8_t *cfgGlyph = glyphManager->getGlyph( Display::GLF_CFG );
        const uint8_t *udGlyph = glyphManager->getGlyph( Display::GLF_UD );
        #if HAS_PERIPHERAL_MODE_CAPABILITY
        const uint8_t *pGlyph = glyphManager->getGlyph( Display::GLF_P );
        const uint8_t *nGlyph = glyphManager->getGlyph( Display::GLF_N );
        #endif

        // Show initial glyph based on starting state
        // Display static glyph for 500ms, then flash 4 times (rate=500ms, interval=250ms)
        #if HAS_PERIPHERAL_MODE_CAPABILITY
        // PMode capable: Show P or N glyph
        // - If PMode disabled: Show [P] in GREEN (entering P mode - always works)
        // - If PMode enabled: Show [N] in RED if unconfigured (warning), GREEN if configured
        if ( pmodeCurrentlyEnabled ) {
            // Exiting P mode → Normal mode
            // Color indicates if device is configured for normal mode operation
            Display::color_t nColor = isProvisioned
                                      ? Display::StandardColors::GREEN  // Configured: ready for Normal
                                      : Display::StandardColors::RED;   // Not configured: warning
            display->drawGlyph( nGlyph, nColor, Display::StandardColors::BLACK, Config::Display::SHOW );
            delay( 500 );  // Static glyph visible for 500ms (0.5 * rate)
            display->flash( 4, 250, nvsBrightness );  // Flash 4 times: OFF/ON at 250ms intervals
            display->setBrightness( nvsBrightness );  // Restore brightness after flash
        }
        else {
            // Entering P mode - always GREEN (P mode works without configuration)
            display->drawGlyph( pGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::SHOW );
            delay( 500 );  // Static glyph visible for 500ms (0.5 * rate)
            display->flash( 4, 250, nvsBrightness );  // Flash 4 times: OFF/ON at 250ms intervals
            display->setBrightness( nvsBrightness );  // Restore brightness after flash
        }
        #else
        // No PMode capability: Show provisioning glyph based on provisioned state
        auto color = isProvisioned ? Display::StandardColors::ORANGE : Display::StandardColors::RED;
        display->drawGlyph( cfgGlyph, color, Display::StandardColors::BLACK, Config::Display::SHOW );
        delay( 500 );  // Static glyph visible for 500ms (0.5 * rate)
        display->flash( 4, 250, nvsBrightness );  // Flash 4 times: OFF/ON at 250ms intervals
        display->setBrightness( nvsBrightness );  // Restore brightness after flash
        #endif

        // Button state machine loop
        while ( !sequenceExit ) {
            // Button state updated by esp_timer

            switch ( state ) {
                    #if HAS_PERIPHERAL_MODE_CAPABILITY
                case BootButtonState::PMODE_PENDING:
                    if ( !button->isPressed() ) {
                        // Released - toggle PMode setting
                        bool newPModeState = !pmodeCurrentlyEnabled;
                        configManager->savePModeEnabled( newPModeState );
                        log_i( "Boot button sequence: PMODE toggled to %s", newPModeState ? "ENABLED" : "DISABLED" );

                        // Show confirmation checkmark (stays visible until restart)
                        display->clear( Config::Display::NO_SHOW );
                        const uint8_t *checkGlyph = glyphManager->getGlyph( Display::GLF_CK );
                        display->drawGlyph( checkGlyph, Display::StandardColors::GREEN, Display::StandardColors::BLACK, Config::Display::NO_SHOW );
                        display->show();  // Single show() call to update display

                        // Restart device to boot with new mode and correct hardware initialization
                        log_i( "Restarting device to apply new operating mode" );
                        restartDevice();  // Uses default GUI_PAUSE_MS delay
                    }
                    else if ( millis() >= stateArmTime ) {
                        // Held long enough - advance to provisioning state
                        log_v( "Advancing from PMODE_PENDING to PROVISIONING_PENDING state" );

                        // Show provisioning glyph (gear icon)
                        // Use ORANGE if provisioned, RED if not
                        auto color = isProvisioned ? Display::StandardColors::ORANGE : Display::StandardColors::RED;
                        display->drawGlyph( cfgGlyph, color, Display::StandardColors::BLACK, Config::Display::SHOW );
                        delay( 500 );  // Static glyph visible for 500ms
                        display->flash( 4, 250, nvsBrightness );  // Flash to indicate state armed
                        display->setBrightness( nvsBrightness );  // Restore brightness

                        state = BootButtonState::PROVISIONING_PENDING;
                        stateArmTime = millis() + STATE_HOLD_TIME;
                    }
                    break;
                    #endif

                case BootButtonState::PROVISIONING_PENDING:
                    if ( !button->isPressed() ) {
                        // Released - enter unified portal mode
                        log_i( "Boot button sequence: UNIFIED PORTAL selected (provisioning/OTA)" );
                        resultMode = OperatingMode::PROVISIONING;
                        sequenceExit = true;
                    }
                    else if ( millis() >= stateArmTime ) {
                        // Only advance to factory reset if device is provisioned
                        // No point doing factory reset on unconfigured device (already in factory state)
                        if ( isProvisioned ) {
                            // Held long enough - advance to factory reset state
                            log_v( "Advancing to FACTORY_RESET_PENDING state" );

                            // GLF_FR (factory reset icon) in red
                            const uint8_t *frGlyph = glyphManager->getGlyph( Display::GLF_FR );
                            display->drawGlyph( frGlyph, Display::StandardColors::RED, Display::StandardColors::BLACK, Config::Display::SHOW );
                            delay( 500 );  // Static glyph visible for 500ms
                            display->flash( 4, 250, nvsBrightness );  // Flash to indicate state armed
                            display->setBrightness( nvsBrightness );  // Restore brightness

                            state = BootButtonState::FACTORY_RESET_PENDING;
                            stateArmTime = millis() + STATE_HOLD_TIME;
                        }
                        else {
                            // Device not provisioned - stay in provisioning state showing red glyph
                            // User must release button to enter provisioning portal
                            log_v( "Device not provisioned - staying in PROVISIONING_PENDING (no factory reset needed)" );
                        }
                    }
                    break;

                case BootButtonState::FACTORY_RESET_PENDING:
                    if ( !button->isPressed() ) {
                        // Released - perform factory reset
                        log_i( "Boot button sequence: FACTORY RESET selected" );
                        handleFactoryReset();
                        // Never returns - ESP32 restarts
                    }
                    // If button stays pressed beyond this state, just wait for release
                    // (no more states after factory reset)
                    break;
            }
        }

        return resultMode;
    }

} // namespace Application


//  --- EOF --- //
