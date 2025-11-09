#include <Arduino.h>
#include "Device_Config.h"
#include "Config/Constants.h"
#include "Config/Types.h"
#include "Hardware/Display/DisplayFactory.h"
#include "Hardware/Display/Colors.h"
#include "Hardware/Sensors/IMUFactory.h"
#include "Hardware/Input/ButtonFactory.h"
#include "Hardware/Interface/InterfaceFactory.h"
#include "Network/WiFiManager.h"
#include "Storage/ConfigManager.h"
#include "State/SystemState.h"

using namespace STAC;
using namespace STAC::Display;
using namespace STAC::Hardware;
using namespace STAC::Network;
using namespace STAC::Storage;
using namespace STAC::State;

// Hardware objects
std::unique_ptr<IDisplay> display;
std::unique_ptr<IIMU> imu;
std::unique_ptr<IButton> button;
std::unique_ptr<GrovePort> grovePort;
std::unique_ptr<PeripheralMode> peripheralDetector;

// Network & Storage
std::unique_ptr<WiFiManager> wifiManager;
std::unique_ptr<ConfigManager> configManager;

// State Management
std::unique_ptr<SystemState> systemState;

// Demo state cycling
int demoStateIndex = 0;
TallyState demoStates[] = {
    TallyState::NO_TALLY,
    TallyState::PREVIEW,
    TallyState::PROGRAM,
    TallyState::UNSELECTED,
    TallyState::ERROR
};

void setup() {
    Serial.begin( 115200 );
    delay( 1000 );

    Serial.println();
    Serial.println( "╔════════════════════════════════════════════╗" );
    Serial.println( "║    STAC Phase 6: State Management Test     ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.printf( "\nBoard: %s\n", Config::Strings::BOARD_NAME );

    // ========================================================================
    // HARDWARE INITIALIZATION
    // ========================================================================
    Serial.println( "\n--- Hardware Initialization ---" );

    // Display
    display = DisplayFactory::create();
    if ( !display->begin() ) {
        Serial.println( "❌ Display failed!" );
        while ( true ) {
            delay( 1000 );
        }
    }
    Serial.println( "✓ Display" );

    display->fill( StandardColors::BLUE, true );
    delay( 200 );
    display->clear( true );

    // IMU
    imu = IMUFactory::create();
    if ( imu->begin() ) {
        Serial.printf( "✓ IMU (%s)\n", imu->getType() );
    }
    else {
        Serial.println( "⚠ IMU unavailable" );
    }

    // Peripheral mode detection
    peripheralDetector = InterfaceFactory::createPeripheralDetector();
    bool isPeripheralMode = peripheralDetector->detect();

    // GROVE port
    grovePort = InterfaceFactory::createGrovePort( !isPeripheralMode );
    Serial.printf( "✓ GROVE (%s)\n", isPeripheralMode ? "INPUT" : "OUTPUT" );

    // Button
    button = ButtonFactory::create();
    if ( !button->begin() ) {
        Serial.println( "❌ Button failed!" );
    }
    else {
        Serial.println( "✓ Button" );
    }

    // ========================================================================
    // STORAGE & NETWORK
    // ========================================================================
    Serial.println( "\n--- Storage & Network ---" );

    // Config Manager
    configManager = std::make_unique<ConfigManager>();
    configManager->begin();
    Serial.println( "✓ Config Manager" );

    // WiFi Manager
    wifiManager = std::make_unique<WiFiManager>();
    wifiManager->begin();
    Serial.println( "✓ WiFi Manager" );

    // Load or generate STAC ID
    String stacID;
    if ( !configManager->loadStacID( stacID ) ) {
        stacID = configManager->generateAndSaveStacID();
    }
    Serial.printf( "  STAC ID: %s\n", stacID.c_str() );

    // Start AP for testing
    wifiManager->setHostname( stacID );
    if ( wifiManager->startAP( stacID, "" ) ) {
        Serial.printf( "  AP: %s @ %s\n", stacID.c_str(),
                       wifiManager->getLocalIP().c_str() );
    }

    // ========================================================================
    // STATE MANAGEMENT
    // ========================================================================
    Serial.println( "\n--- State Management ---" );

    systemState = std::make_unique<SystemState>();

    if ( !systemState->begin() ) {
        Serial.println( "❌ System state failed!" );
        while ( true ) {
            delay( 1000 );
        }
    }
    Serial.println( "✓ System State initialized" );

    // Set operating mode based on peripheral detection
    if ( isPeripheralMode ) {
        systemState->getOperatingMode().setMode( OperatingMode::PERIPHERAL );
    }
    else {
        systemState->getOperatingMode().setMode( OperatingMode::NORMAL );
    }

    Serial.printf( "  Operating Mode: %s\n",
                   systemState->getOperatingMode().getModeString() );
    Serial.printf( "  Tally State: %s\n",
                   systemState->getTallyState().getStateString() );

    // Set up tally state change callback
    systemState->getTallyState().setStateChangeCallback(
    []( TallyState oldState, TallyState newState ) {
        Serial.printf( "🔔 Tally changed: %s -> %s\n",
                       TallyStateManager::stateToString( oldState ),
                       TallyStateManager::stateToString( newState ) );
    }
    );

    // Load and display operations
    StacOperations& ops = systemState->getOperations();
    if ( configManager->loadOperations( ops ) ) {
        Serial.println( "  Loaded operations from NVS" );
        Serial.printf( "    Switch: %s\n", ops.switchModel.c_str() );
        Serial.printf( "    Channel: %d\n", ops.tallyChannel );
        Serial.printf( "    Auto-start: %s\n", ops.autoStartEnabled ? "Yes" : "No" );
    }

    // Check if system is ready
    Serial.printf( "  System Ready: %s\n",
                   systemState->isReady() ? "Yes" : "No (needs config)" );

    // ========================================================================
    // SETUP COMPLETE
    // ========================================================================
    Serial.println();
    Serial.println( "╔════════════════════════════════════════════╗" );
    Serial.println( "║              Setup Complete!               ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println( "\n--- Test Instructions ---" );
    Serial.println( "Button Actions:" );
    Serial.println( "  • Short press: Cycle tally states" );
    Serial.println( "  • Long press:  Toggle operating mode" );
    Serial.println( "\nWatch display change color with tally state:" );
    Serial.println( "  NO_TALLY:    Purple" );
    Serial.println( "  PREVIEW:     Green" );
    Serial.println( "  PROGRAM:     Red" );
    Serial.println( "  UNSELECTED:  Blue" );
    Serial.println( "  ERROR:       Red (bright)" );
    Serial.println();

    // Show initial state
    display->fill( systemState->getTallyState().getStateColor(), true );
}

void loop() {
    // Update button
    button->update();

    // Short press: Cycle tally states
    if ( button->wasClicked() ) {
        demoStateIndex = ( demoStateIndex + 1 ) % 5;
        TallyState newState = demoStates[ demoStateIndex ];

        // Update system state
        systemState->getTallyState().setState( newState );

        // Update display
        display->fill( systemState->getTallyState().getStateColor(), true );

        // Update GROVE port (if in normal mode)
        if ( systemState->getOperatingMode().isNormalMode() ) {
            grovePort->setTallyState( newState );
            Serial.printf( "  → GROVE output: %s\n",
                           systemState->getTallyState().getStateString() );
        }
    }

    // Long press: Toggle between Normal and Peripheral modes
    static bool longPressHandled = false;
    if ( button->isLongPress() && !longPressHandled ) {
        longPressHandled = true;

        // Toggle mode
        if ( systemState->getOperatingMode().isNormalMode() ) {
            systemState->getOperatingMode().setMode( OperatingMode::PERIPHERAL );
            Serial.println( "\n→ Switched to PERIPHERAL mode" );

            // Reconfigure GROVE as input
            grovePort = InterfaceFactory::createGrovePort( false ); // Input mode

        }
        else {
            systemState->getOperatingMode().setMode( OperatingMode::NORMAL );
            Serial.println( "\n→ Switched to NORMAL mode" );

            // Reconfigure GROVE as output
            grovePort = InterfaceFactory::createGrovePort( true ); // Output mode
        }

        // Visual feedback
        display->fill( StandardColors::YELLOW, true );
        delay( 500 );
        display->fill( systemState->getTallyState().getStateColor(), true );
    }

    if ( !button->isPressed() ) {
        longPressHandled = false;
    }

    // In peripheral mode, read tally state from GROVE
    if ( systemState->getOperatingMode().isPeripheralMode() ) {
        static unsigned long lastRead = 0;
        if ( millis() - lastRead > 100 ) {
            lastRead = millis();

            TallyState receivedState = grovePort->readTallyState();

            if ( receivedState != systemState->getTallyState().getCurrentState() ) {
                systemState->getTallyState().setState( receivedState );
                display->fill( systemState->getTallyState().getStateColor(), true );
                Serial.printf( "  ← GROVE input: %s\n",
                               systemState->getTallyState().getStateString() );
            }
        }
    }

    // Update managers
    systemState->update();
    wifiManager->update();

    // Status updates
    static unsigned long lastStatus = 0;
    if ( millis() - lastStatus > 30000 ) {
        lastStatus = millis();

        Serial.println( "\n--- Status Update ---" );
        Serial.printf( "Operating Mode: %s\n",
                       systemState->getOperatingMode().getModeString() );
        Serial.printf( "Tally State: %s (for %lu sec)\n",
                       systemState->getTallyState().getStateString(),
                       systemState->getTallyState().getTimeSinceChange() / 1000 );
        Serial.printf( "WiFi: %s\n",
                       wifiManager->isAPMode() ? "AP Mode" :
                       wifiManager->isConnected() ? "Connected" : "Disconnected" );
        Serial.println();
    }

    delay( 10 );
}


//  --- EOF --- //
