#include "Storage/ConfigManager.h"
#include <Arduino.h>
#include <esp_mac.h>


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

        bool ConfigManager::saveSwitchConfig( const String &model, const IPAddress& ipAddress, uint16_t port,
                                                       const String &username, const String &password ) {
            if ( !prefs.begin( NS_SWITCH, false ) ) {
                log_e( "Failed to open switch preferences" );
                return false;
            }

            prefs.putString( KEY_MODEL, model );
            prefs.putUInt( KEY_IP, ( uint32_t )ipAddress );
            prefs.putUShort( KEY_PORT, port );
            prefs.putString( KEY_USERNAME, username );
            prefs.putString( KEY_PASSWORD, password );
            prefs.putUChar( KEY_VERSION, Config::NVS::NOM_PREFS_VERSION );
            prefs.end();

            log_i( "Switch config saved: %s @ %s:%d", model.c_str(), ipAddress.toString().c_str(), port );
            return true;
        }

        bool ConfigManager::loadSwitchConfig( String &model, IPAddress& ipAddress, uint16_t &port,
                                                       String &username, String &password ) {
            if ( !prefs.begin( NS_SWITCH, true ) ) {
                log_w( "No switch preferences found" );
                return false;
            }

            model = prefs.getString( KEY_MODEL, "" );
            uint32_t ip = prefs.getUInt( KEY_IP, 0 );
            port = prefs.getUShort( KEY_PORT, 80 );
            username = prefs.getString( KEY_USERNAME, "" );
            password = prefs.getString( KEY_PASSWORD, "" );
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

            ops.switchModel = prefs.getString( "switchModel", "V-60HD" );
            ops.tallyChannel = prefs.getUChar( "tallyChannel", 1 );
            ops.maxChannelCount = prefs.getUChar( "maxChannelCount", 8 );
            ops.channelBank = prefs.getString( "channelBank", "hdmi_" );
            ops.maxHDMIChannel = prefs.getUChar( "maxHDMI", 8 );
            ops.maxSDIChannel = prefs.getUChar( "maxSDI", 8 );
            
            // Validate and correct channelBank based on tallyChannel for V-160HD
            if (ops.switchModel == "V-160HD") {
                if (ops.tallyChannel > 8) {
                    ops.channelBank = "sdi_";
                } else {
                    ops.channelBank = "hdmi_";
                }
            }
            
            // Validate maxChannelCount based on switch model
            if (ops.switchModel == "V-60HD") {
                if (ops.maxChannelCount == 0 || ops.maxChannelCount > 8) {
                    ops.maxChannelCount = 8;
                    log_w("Invalid maxChannelCount for V-60HD, set to 8");
                }
            } else if (ops.switchModel == "V-160HD") {
                // V-160HD uses maxHDMIChannel and maxSDIChannel instead
                ops.maxChannelCount = 0;
            }
            
            ops.autoStartEnabled = prefs.getBool( "autoStart", false );
            ops.cameraOperatorMode = prefs.getBool( "camOpMode", true );
            ops.displayBrightnessLevel = prefs.getUChar( "brightness", 1 );
            ops.statusPollInterval = prefs.getULong( "pollInterval", 300 );
            prefs.end();

            log_v( "Operations loaded" );  // Changed to verbose to reduce log spam
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
            // Generate STAC ID from MAC address (last 3 bytes, reversed)
            uint8_t mac[ 6 ];
            esp_efuse_mac_get_default( mac );

            // Use snprintf to ensure zero-padding for single-digit hex values
            char macSuffix[ 7 ];  // 6 hex chars + null terminator
            snprintf( macSuffix, sizeof( macSuffix ), "%02X%02X%02X", 
                     mac[ 5 ], mac[ 4 ], mac[ 3 ] );

            log_d( "MAC bytes: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5] );
            log_d( "MAC suffix (reversed last 3): %s", macSuffix );

            String stacID = String( Config::Strings::ID_PREFIX ) + "-" + String( macSuffix );

            log_d( "Generated STAC ID: %s", stacID.c_str() );

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

        bool ConfigManager::savePeripheralSettings( bool cameraMode, uint8_t brightnessLevel ) {
            if ( !prefs.begin( "PModePrefs", false ) ) {
                log_e( "Failed to open PModePrefs namespace" );
                return false;
            }

            prefs.putUShort( "pmPrefVer", Config::NVS::PM_PREFS_VERSION );
            prefs.putBool( "pmct", cameraMode );
            prefs.putUChar( "pmbrightness", brightnessLevel );
            prefs.end();

            log_i( "Peripheral settings saved: mode=%s, brightness=%d",
                   cameraMode ? "Camera" : "Talent", brightnessLevel );
            return true;
        }

        bool ConfigManager::loadPeripheralSettings( bool& cameraMode, uint8_t& brightnessLevel ) {
            if ( !prefs.begin( "PModePrefs", true ) ) {
                log_w( "PModePrefs namespace not found" );
                return false;
            }

            // Check version
            if ( !prefs.isKey( "pmPrefVer" ) ||
                 prefs.getUShort( "pmPrefVer" ) != Config::NVS::PM_PREFS_VERSION ) {
                prefs.end();
                log_w( "Peripheral prefs version mismatch or missing" );
                return false;
            }

            cameraMode = prefs.getBool( "pmct", false );
            brightnessLevel = prefs.getUChar( "pmbrightness", 1 );
            prefs.end();

            log_v( "Peripheral settings loaded: mode=%s, brightness=%d",
                   cameraMode ? "Camera" : "Talent", brightnessLevel );
            return true;
        }

    } // namespace Storage



//  --- EOF --- //
