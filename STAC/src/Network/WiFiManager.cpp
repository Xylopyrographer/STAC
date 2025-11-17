#include "Network/WiFiManager.h"
#include <Arduino.h>

namespace Net {

        WiFiManager::WiFiManager()
            : state( WiFiState::DISCONNECTED )
            , currentSSID( "" )
            , currentPassword( "" )
            , hostname( "STAC" )
            , apMode( false )
            , lastConnectionAttempt( 0 ) {
        }

        bool WiFiManager::begin() {
            // Set WiFi mode to off initially
            WiFi.mode( WIFI_OFF );
            delay( 100 );

            log_i( "WiFi Manager initialized" );
            return true;
        }

        bool WiFiManager::connect( const String &ssid, const String &password, unsigned long timeoutMs ) {
            if ( ssid.isEmpty() ) {
                log_e( "Cannot connect: SSID is empty" );
                state = WiFiState::FAILED;
                return false;
            }

            log_i( "Connecting to WiFi: %s", ssid.c_str() );

            // Stop AP if running
            if ( apMode ) {
                stopAP();
            }

            // Set to station mode
            WiFi.mode( WIFI_STA );
            delay( 100 );

            // Store credentials for reconnection
            currentSSID = ssid;
            currentPassword = password;

            // Set hostname if configured
            if ( !hostname.isEmpty() ) {
                WiFi.setHostname( hostname.c_str() );
            }

            // Start connection
            state = WiFiState::CONNECTING;
            if ( stateCallback ) {
                stateCallback( state );
            }
            WiFi.begin( ssid.c_str(), password.c_str() );

            // Wait for connection with timeout
            unsigned long startTime = millis();
            while ( WiFi.status() != WL_CONNECTED ) {
                if ( millis() - startTime > timeoutMs ) {
                    log_e( "WiFi connection timeout" );
                    state = WiFiState::FAILED;
                    if ( stateCallback ) {
                        stateCallback( state );
                    }
                    WiFi.disconnect();
                    return false;
                }
                delay( 100 );
            }

            state = WiFiState::CONNECTED;
            if ( stateCallback ) {
                stateCallback( state );
            }
            lastConnectionAttempt = millis();

            log_i( "WiFi connected!" );
            log_i( "  IP address: %s", WiFi.localIP().toString().c_str() );
            log_i( "  RSSI: %d dBm", WiFi.RSSI() );

            return true;
        }

        bool WiFiManager::startAP( const String &ssid, const String &password ) {
            if ( ssid.isEmpty() ) {
                log_e( "Cannot start AP: SSID is empty" );
                return false;
            }

            log_i( "Starting Access Point: %s", ssid.c_str() );

            // Disconnect from station if connected
            if ( WiFi.status() == WL_CONNECTED ) {
                disconnect();
            }

            // Set to AP mode
            WiFi.mode( WIFI_AP );
            delay( 100 );

            // Start AP
            bool success;
            if ( password.isEmpty() ) {
                success = WiFi.softAP( ssid.c_str() );
                log_i( "  Open network (no password)" );
            }
            else {
                success = WiFi.softAP( ssid.c_str(), password.c_str() );
                log_i( "  Password protected" );
            }

            if ( success ) {
                apMode = true;
                state = WiFiState::AP_MODE;
                log_i( "  AP IP address: %s", WiFi.softAPIP().toString().c_str() );
                return true;
            }
            else {
                log_e( "Failed to start Access Point" );
                state = WiFiState::FAILED;
                return false;
            }
        }

        void WiFiManager::disconnect() {
            log_i( "Disconnecting from WiFi" );
            WiFi.disconnect( true );
            state = WiFiState::DISCONNECTED;
            currentSSID = "";
            currentPassword = "";
        }

        void WiFiManager::stopAP() {
            if ( apMode ) {
                log_i( "Stopping Access Point" );
                WiFi.softAPdisconnect( true );
                apMode = false;
                state = WiFiState::DISCONNECTED;
            }
        }

        bool WiFiManager::isConnected() const {
            return ( state == WiFiState::CONNECTED && WiFi.status() == WL_CONNECTED );
        }

        bool WiFiManager::isAPMode() const {
            return apMode;
        }

        WiFiState WiFiManager::getState() const {
            return state;
        }

        String WiFiManager::getLocalIP() const {
            if ( apMode ) {
                return WiFi.softAPIP().toString();
            }
            else {
                return WiFi.localIP().toString();
            }
        }

        String WiFiManager::getMacAddress() const {
            return WiFi.macAddress();
        }

        int32_t WiFiManager::getRSSI() const {
            if ( isConnected() ) {
                return WiFi.RSSI();
            }
            return 0;
        }

        bool WiFiManager::setHostname( const String &newHostname ) {
            hostname = newHostname;

            if ( WiFi.status() == WL_CONNECTED ) {
                return WiFi.setHostname( hostname.c_str() );
            }

            return true;  // Will be set on next connection
        }

        String WiFiManager::getHostname() const {
            return hostname;
        }

        void WiFiManager::setStateCallback( WiFiStateCallback callback ) {
            stateCallback = callback;
        }

        void WiFiManager::update() {
            // Check if we should attempt reconnection
            if ( state == WiFiState::CONNECTED && WiFi.status() != WL_CONNECTED ) {
                log_w( "WiFi connection lost" );
                state = WiFiState::DISCONNECTED;
            }

            // Attempt reconnection if we have credentials
            if ( state == WiFiState::DISCONNECTED && !currentSSID.isEmpty() ) {
                attemptReconnect();
            }
        }

        void WiFiManager::attemptReconnect() {
            unsigned long now = millis();

            // Don't retry too frequently
            if ( now - lastConnectionAttempt < RECONNECT_INTERVAL_MS ) {
                return;
            }

            log_i( "Attempting to reconnect to WiFi..." );
            lastConnectionAttempt = now;

            connect( currentSSID, currentPassword, 10000 ); // 10 second timeout for reconnect
        }

} // namespace Net


//  --- EOF --- //
