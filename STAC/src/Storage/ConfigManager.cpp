#include "Storage/ConfigManager.h"
#include <Arduino.h>
#include <esp_mac.h>

namespace STAC {
    namespace Storage {

        ConfigManager::ConfigManager() {
        }

        bool ConfigManager::begin() {
            log_i( "Config Manager initialized" );

            // Check and migrate configuration if needed
            return checkAndMigrateConfig();
        }

        bool ConfigManager::saveWiFiCredentials( const String &ssid, const String &password ) {
            if ( !prefs.begin( NS_WIFI, false ) ) { // Read-write mode
                log_e( "Failed to open WiFi preferences" );
                return false;
            }

            prefs.putString( KEY_SSID, ssid );
            prefs.putString( KEY_PASSWORD, password );
            prefs.putUChar( KEY_VERSION, Config::NVS::NOM_PREFS_VERSION );
            prefs.end();

            log_i( "WiFi credentials saved" );
            return true;
        }

        bool ConfigManager::loadWiFiCredentials( String &ssid, String &password ) {
            if ( !prefs.begin( NS_WIFI, true ) ) { // Read-only mode
                log_w( "No WiFi preferences found" );
                return false;
            }

            ssid = prefs.getString( KEY_SSID, "" );
            password = prefs.getString( KEY_PASSWORD, "" );
            prefs.end();

            if ( ssid.isEmpty() ) {
                log_w( "No WiFi credentials stored" );
                return false;
            }

            log_i( "WiFi credentials loaded: %s", ssid.c_str() );
            return true;
        }

        bool ConfigManager::hasWiFiCredentials() {
            if ( !prefs.begin( NS_WIFI, true ) ) {
                return false;
            }

            String ssid = prefs.getString( KEY_SSID, "" );
            prefs.end();

            return !ssid.isEmpty();
        }

        void ConfigManager::clearWiFiCredentials() {
            prefs.begin( NS_WIFI, false );
            prefs.clear();
            prefs.end();
            log_i( "WiFi credentials cleared" );
        }

        bool ConfigManager::saveSwitchConfig( const String &model, const IPAddress& ipAddress, uint16_t port ) {
            if ( !prefs.begin( NS_SWITCH, false ) ) {
                log_e( "Failed to open switch preferences" );
                return false;
            }

            prefs.putString( KEY_MODEL, model );
            prefs.putUInt( KEY_IP, ( uint32_t )ipAddress );
            prefs.putUShort( KEY_PORT, port );
            prefs.putUChar( KEY_VERSION, Config::NVS::NOM_PREFS_VERSION );
            prefs.end();

            log_i( "Switch config saved: %s @ %s:%d", model.c_str(), ipAddress.toString().c_str(), port );
            return true;
        }

        bool ConfigManager::loadSwitchConfig( String &model, IPAddress& ipAddress, uint16_t &port ) {
            if ( !prefs.begin( NS_SWITCH, true ) ) {
                log_w( "No switch preferences found" );
                return false;
            }

            model = prefs.getString( KEY_MODEL, "" );
            uint32_t ip = prefs.getUInt( KEY_IP, 0 );
            port = prefs.getUShort( KEY_PORT, 80 );
            prefs.end();

            if ( model.isEmpty() || ip == 0 ) {
                log_w( "No switch configuration stored" );
                return false;
            }

            ipAddress = IPAddress( ip );
            log_i( "Switch config loaded: %s @ %s:%d", model.c_str(), ipAddress.toString().c_str(), port );
            return true;
        }

        bool ConfigManager::saveOperations( const StacOperations& ops ) {
            if ( !prefs.begin( NS_OPERATIONS, false ) ) {
                log_e( "Failed to open operations preferences" );
                return false;
            }

            prefs.putString( "switchModel", ops.switchModel );
            prefs.putUChar( "tallyChannel", ops.tallyChannel );
            prefs.putUChar( "maxChannelCount", ops.maxChannelCount );
            prefs.putString( "channelBank", ops.channelBank );
            prefs.putUChar( "maxHDMI", ops.maxHDMIChannel );
            prefs.putUChar( "maxSDI", ops.maxSDIChannel );
            prefs.putBool( "autoStart", ops.autoStartEnabled );
            prefs.putBool( "camOpMode", ops.cameraOperatorMode );
            prefs.putUChar( "brightness", ops.displayBrightnessLevel );
            prefs.putULong( "pollInterval", ops.statusPollInterval );
            prefs.putUChar( KEY_VERSION, Config::NVS::NOM_PREFS_VERSION );
            prefs.end();

            log_i( "Operations saved" );
            return true;
        }

        bool ConfigManager::loadOperations( StacOperations& ops ) {
            if ( !prefs.begin( NS_OPERATIONS, true ) ) {
                log_w( "No operations preferences found" );
                return false;
            }

            ops.switchModel = prefs.getString( "switchModel", "NO_MODEL" );
            ops.tallyChannel = prefs.getUChar( "tallyChannel", 1 );
            ops.maxChannelCount = prefs.getUChar( "maxChannelCount", 6 );
            ops.channelBank = prefs.getString( "channelBank", "NO_BANK" );
            ops.maxHDMIChannel = prefs.getUChar( "maxHDMI", 8 );
            ops.maxSDIChannel = prefs.getUChar( "maxSDI", 8 );
            ops.autoStartEnabled = prefs.getBool( "autoStart", false );
            ops.cameraOperatorMode = prefs.getBool( "camOpMode", true );
            ops.displayBrightnessLevel = prefs.getUChar( "brightness", 1 );
            ops.statusPollInterval = prefs.getULong( "pollInterval", 300 );
            prefs.end();

            log_i( "Operations loaded" );
            return true;
        }

        bool ConfigManager::saveStacID( const String &stacID ) {
            if ( !prefs.begin( NS_IDENTITY, false ) ) {
                log_e( "Failed to open identity preferences" );
                return false;
            }

            prefs.putString( KEY_STAC_ID, stacID );
            prefs.end();

            log_i( "STAC ID saved: %s", stacID.c_str() );
            return true;
        }

        bool ConfigManager::loadStacID( String &stacID ) {
            if ( !prefs.begin( NS_IDENTITY, true ) ) {
                return false;
            }

            stacID = prefs.getString( KEY_STAC_ID, "" );
            prefs.end();

            return !stacID.isEmpty();
        }

        String ConfigManager::generateAndSaveStacID() {
            // Generate STAC ID from MAC address
            uint8_t mac[ 6 ];
            esp_read_mac( mac, ESP_MAC_WIFI_STA );

            String stacID = String( Config::Strings::ID_PREFIX ) + "_" +
                            String( mac[ 3 ], HEX ) + String( mac[ 4 ], HEX ) + String( mac[ 5 ], HEX );
            stacID.toUpperCase();

            saveStacID( stacID );

            log_i( "Generated STAC ID: %s", stacID.c_str() );
            return stacID;
        }

        bool ConfigManager::isConfigured() {
            return hasWiFiCredentials();
        }

        bool ConfigManager::clearAll() {
            log_i( "Clearing all configuration" );

            prefs.begin( NS_WIFI, false );
            prefs.clear();
            prefs.end();

            prefs.begin( NS_SWITCH, false );
            prefs.clear();
            prefs.end();

            prefs.begin( NS_OPERATIONS, false );
            prefs.clear();
            prefs.end();

            prefs.begin( NS_IDENTITY, false );
            prefs.clear();
            prefs.end();

            log_i( "All configuration cleared" );
            return true;
        }

        uint8_t ConfigManager::getConfigVersion() {
            if ( !prefs.begin( NS_OPERATIONS, true ) ) {
                return 0;
            }

            uint8_t version = prefs.getUChar( KEY_VERSION, 0 );
            prefs.end();

            return version;
        }

        bool ConfigManager::checkAndMigrateConfig() {
            uint8_t currentVersion = getConfigVersion();

            if ( currentVersion == 0 ) {
                log_i( "No existing configuration found" );
                return true;
            }

            if ( currentVersion < Config::NVS::NOM_PREFS_VERSION ) {
                log_i( "Configuration version %d is older than current %d",
                       currentVersion, Config::NVS::NOM_PREFS_VERSION );
                log_i( "Migration may be needed (not implemented yet)" );
                // TODO: Implement migration logic if needed
            }

            return true;
        }

    } // namespace Storage
} // namespace STAC


//  --- EOF --- //
