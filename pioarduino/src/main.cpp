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

using namespace STAC;
using namespace STAC::Display;
using namespace STAC::Hardware;
using namespace STAC::Network;
using namespace STAC::Storage;

// Create hardware objects
std::unique_ptr<IDisplay> display;
std::unique_ptr<IIMU> imu;
std::unique_ptr<IButton> button;
std::unique_ptr<GrovePort> grovePort;
std::unique_ptr<PeripheralMode> peripheralDetector;

// Create network & storage objects
std::unique_ptr<WiFiManager> wifiManager;
std::unique_ptr<ConfigManager> configManager;

// State
bool isPeripheralMode = false;
String stacID;

void setup() {
    Serial.begin( 115200 );
    delay( 1000 );

    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║  STAC Phase 5: Network & Storage Test     ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.printf( "\nBoard: %s\n", Config::Strings::BOARD_NAME );

    // ========================================================================
    // DISPLAY
    // ========================================================================
    Serial.println( "\n--- Display Initialization ---" );
    display = DisplayFactory::create();

    if ( !display->begin() ) {
        Serial.println( "❌ Display initialization failed!" );
        while ( true ) {
            delay( 1000 );
        }
    }
    Serial.println( "✓ Display initialized" );

    // Show startup animation
    display->fill( StandardColors::BLUE, true );
    delay( 200 );
    display->clear( false );
    display->setPixel( Config::Display::POWER_LED_PIXEL, STACColors::POWER_ON, true );

    // ========================================================================
    // IMU
    // ========================================================================
    Serial.println( "\n--- IMU Initialization ---" );
    imu = IMUFactory::create();

    if ( !imu->begin() ) {
        Serial.println( "⚠ IMU not initialized" );
    }
    else {
        Serial.printf( "✓ IMU initialized (%s)\n", imu->getType() );
    }

    // ========================================================================
    // PERIPHERAL MODE DETECTION
    // ========================================================================
    Serial.println( "\n--- Peripheral Mode Detection ---" );
    peripheralDetector = InterfaceFactory::createPeripheralDetector();
    isPeripheralMode = peripheralDetector->detect();

    if ( isPeripheralMode ) {
        Serial.println( "✓ PERIPHERAL MODE detected" );
        display->fill( StandardColors::ORANGE, true );
        delay( 500 );
    }
    else {
        Serial.println( "✓ NORMAL MODE" );
    }

    // ========================================================================
    // GROVE PORT
    // ========================================================================
    Serial.println( "\n--- GROVE Port Initialization ---" );
    grovePort = InterfaceFactory::createGrovePort( !isPeripheralMode );
    Serial.printf( "✓ GROVE port initialized (%s mode)\n",
                   isPeripheralMode ? "INPUT" : "OUTPUT" );

    // ========================================================================
    // BUTTON
    // ========================================================================
    Serial.println( "\n--- Button Initialization ---" );
    button = ButtonFactory::create();

    if ( !button->begin() ) {
        Serial.println( "❌ Button initialization failed!" );
    }
    else {
        Serial.println( "✓ Button initialized" );
    }

    // ========================================================================
    // CONFIG MANAGER
    // ========================================================================
    Serial.println( "\n--- Configuration Manager ---" );
    configManager = std::make_unique<ConfigManager>();

    if ( !configManager->begin() ) {
        Serial.println( "❌ Config manager initialization failed!" );
    }
    else {
        Serial.println( "✓ Config manager initialized" );
    }

    // Load or generate STAC ID
    if ( !configManager->loadStacID( stacID ) ) {
        Serial.println( "  No STAC ID found, generating..." );
        stacID = configManager->generateAndSaveStacID();
    }
    Serial.printf( "  STAC ID: %s\n", stacID.c_str() );

    // Check for existing configuration
    if ( configManager->hasWiFiCredentials() ) {
        String ssid, password;
        configManager->loadWiFiCredentials( ssid, password );
        Serial.printf( "  Stored WiFi: %s\n", ssid.c_str() );
    }
    else {
        Serial.println( "  No WiFi credentials stored" );
    }

    // ========================================================================
    // WIFI MANAGER
    // ========================================================================
    Serial.println( "\n--- WiFi Manager ---" );
    wifiManager = std::make_unique<WiFiManager>();

    if ( !wifiManager->begin() ) {
        Serial.println( "❌ WiFi manager initialization failed!" );
    }
    else {
        Serial.println( "✓ WiFi manager initialized" );
    }

    // Set hostname
    wifiManager->setHostname( stacID );
    Serial.printf( "  Hostname: %s\n", wifiManager->getHostname().c_str() );
    Serial.printf( "  MAC Address: %s\n", wifiManager->getMacAddress().c_str() );

    // Start in AP mode for testing
    String apSSID = stacID;
    Serial.printf( "\n  Starting AP mode: %s\n", apSSID.c_str() );

    if ( wifiManager->startAP( apSSID, "" ) ) {
        Serial.println( "  ✓ Access Point started" );
        Serial.printf( "  AP IP: %s\n", wifiManager->getLocalIP().c_str() );

        // Show AP mode on display
        display->fill( StandardColors::TEAL, true );
        delay( 500 );
    }
    else {
        Serial.println( "  ❌ Failed to start Access Point" );
    }

    // ========================================================================
    // SETUP COMPLETE
    // ========================================================================
    Serial.println( "\n╔════════════════════════════════════════════╗" );
    Serial.println( "║  Setup Complete!                           ║" );
    Serial.println( "╚════════════════════════════════════════════╝" );
    Serial.println( "\n--- Test Instructions ---" );
    Serial.println( "Button Actions:" );
    Serial.println( "  • Short press: Save test WiFi credentials" );
    Serial.println( "  • Long press:  Clear all configuration" );
    Serial.println( "\nConnect to WiFi:" );
    Serial.printf( "  SSID: %s (open network)\n", apSSID.c_str() );
    Serial.printf( "  IP:   %s\n", wifiManager->getLocalIP().c_str() );
    Serial.println( "\n" );

    // Ready state
    display->clear( false );
    display->setPixel( Config::Display::POWER_LED_PIXEL,
                       StandardColors::TEAL, true );
}

void loop() {
    // Update button
    button->update();

    // Handle button events
    if ( button->wasClicked() ) {
        Serial.println( "\n--- Saving Test Configuration ---" );

        // Save test WiFi credentials
        configManager->saveWiFiCredentials( "TestNetwork", "TestPassword123" );
        Serial.println( "✓ WiFi credentials saved" );

        // Save test switch config
        IPAddress testIP( 192, 168, 1, 100 );
        configManager->saveSwitchConfig( "V-60HD", testIP, 80 );
        Serial.println( "✓ Switch config saved" );

        // Save test operations
        StacOperations ops;
        ops.switchModel = "V-60HD";
        ops.tallyChannel = 3;
        ops.autoStartEnabled = true;
        ops.displayBrightnessLevel = 5;
        configManager->saveOperations( ops );
        Serial.println( "✓ Operations saved" );

        // Visual feedback
        display->fill( StandardColors::GREEN, true );
        delay( 500 );
        display->clear( false );
        display->setPixel( Config::Display::POWER_LED_PIXEL,
                           StandardColors::TEAL, true );

        Serial.println( "Configuration saved! Reset to see it loaded.\n" );
    }

    if ( button->isLongPress() ) {
        static bool longPressHandled = false;

        if ( !longPressHandled ) {
            longPressHandled = true;

            Serial.println( "\n--- Clearing All Configuration ---" );
            configManager->clearAll();
            Serial.println( "✓ All configuration cleared" );

            // Visual feedback
            display->fill( StandardColors::RED, true );
            delay( 1000 );
            display->clear( false );
            display->setPixel( Config::Display::POWER_LED_PIXEL,
                               StandardColors::TEAL, true );

            Serial.println( "Configuration cleared!\n" );
        }

        if ( !button->isPressed() ) {
            longPressHandled = false;
        }
    }

    // Update WiFi manager
    wifiManager->update();

    // Show WiFi state on serial periodically
    static unsigned long lastStatusUpdate = 0;
    if ( millis() - lastStatusUpdate > 30000 ) { // Every 30 seconds
        lastStatusUpdate = millis();

        Serial.println( "\n--- Status Update ---" );
        Serial.printf( "WiFi State: %s\n",
                       wifiManager->isAPMode() ? "AP Mode" :
                       wifiManager->isConnected() ? "Connected" : "Disconnected" );
        Serial.printf( "IP: %s\n", wifiManager->getLocalIP().c_str() );

        if ( wifiManager->isConnected() ) {
            Serial.printf( "RSSI: %d dBm\n", wifiManager->getRSSI() );
        }
        Serial.println();
    }

    delay( 10 );
}


//  --- EOF --- //
