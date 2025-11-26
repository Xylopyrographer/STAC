#include "Storage/ConfigManager.h"
#include <Arduino.h>
#include <esp_mac.h>

using namespace Config::Storage;

    namespace Storage {

        ConfigManager::ConfigManager() {
        }

        bool ConfigManager::begin() {
            log_i( "Config Manager initialized" );

            // Check NVS schema version
            checkSchemaVersion();
            
            return true;
        }

        bool ConfigManager::saveWiFiCredentials( const String &ssid, const String &password ) {
            if ( !prefs.begin( NS_WIFI, READ_WRITE ) ) {
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
            if ( !prefs.begin( NS_WIFI, READ_ONLY ) ) {
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
            if ( !prefs.begin( NS_WIFI, READ_ONLY ) ) {
                return false;
            }

            String ssid = prefs.getString( KEY_SSID, "" );
            prefs.end();

            return !ssid.isEmpty();
        }

        bool ConfigManager::isProvisioned() {
            // Check for WiFi credentials
            if ( !prefs.begin( NS_WIFI, true ) ) {
                return false;
            }
            bool hasSSID = prefs.isKey( KEY_SSID );
            prefs.end();

            if ( !hasSSID ) {
                return false;
            }

            // Check for switch configuration
            if ( !prefs.begin( NS_SWITCH, READ_ONLY ) ) {
                return false;
            }
            bool hasSwitch = prefs.isKey( KEY_MODEL );
            prefs.end();

            return hasSwitch;
        }

        void ConfigManager::clearWiFiCredentials() {
            prefs.begin( NS_WIFI, READ_WRITE );
            prefs.clear();
            prefs.end();
            log_i( "WiFi credentials cleared" );
        }

        bool ConfigManager::saveSwitchConfig( const String &model, const IPAddress& ipAddress, uint16_t port,
                                                       const String &username, const String &password ) {
            if ( !prefs.begin( NS_SWITCH, READ_WRITE ) ) {
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
            if ( !prefs.begin( NS_SWITCH, READ_ONLY ) ) {
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

        bool ConfigManager::saveV60HDConfig( const StacOperations& ops ) {
            if ( !prefs.begin( NS_V60HD, READ_WRITE ) ) {
                log_e( "Failed to open V-60HD preferences" );
                return false;
            }

            prefs.putUChar( KEY_TALLY_CHANNEL, ops.tallyChannel );
            prefs.putUChar( KEY_MAX_CHANNEL, ops.maxChannelCount );
            prefs.putUChar( KEY_AUTO_START, ops.autoStartEnabled );
            prefs.putBool( KEY_CAM_OP_MODE, ops.cameraOperatorMode );
            prefs.putUChar( KEY_BRIGHTNESS, ops.displayBrightnessLevel );
            prefs.putULong( KEY_POLL_INTERVAL, ops.statusPollInterval );
            prefs.end();

            log_i( "V-60HD configuration saved" );
            return true;
        }

        bool ConfigManager::loadV60HDConfig( StacOperations& ops ) {
            if ( !prefs.begin( NS_V60HD, READ_ONLY ) ) {
                log_w( "No V-60HD configuration found" );
                return false;
            }

            ops.switchModel = "V-60HD";
            ops.tallyChannel = prefs.getUChar( KEY_TALLY_CHANNEL, 1 );
            ops.maxChannelCount = prefs.getUChar( KEY_MAX_CHANNEL, 8 );
            ops.autoStartEnabled = prefs.getBool( KEY_AUTO_START, false );
            ops.cameraOperatorMode = prefs.getBool( KEY_CAM_OP_MODE, true );
            ops.displayBrightnessLevel = prefs.getUChar( KEY_BRIGHTNESS, 1 );
            ops.statusPollInterval = prefs.getULong( KEY_POLL_INTERVAL, 300 );
            
            // V-60HD doesn't use these fields
            ops.channelBank = "";
            ops.maxHDMIChannel = 0;
            ops.maxSDIChannel = 0;
            
            // Validate maxChannelCount
            if ( ops.maxChannelCount == 0 || ops.maxChannelCount > 8 ) {
                ops.maxChannelCount = 8;
                log_w( "Invalid maxChannelCount for V-60HD, set to 8" );
            }
            
            prefs.end();

            log_i( "V-60HD configuration loaded" );
            return true;
        }

        bool ConfigManager::saveV160HDConfig( const StacOperations& ops ) {
            if ( !prefs.begin( NS_V160HD, READ_WRITE ) ) {
                log_e( "Failed to open V-160HD preferences" );
                return false;
            }

            prefs.putUChar( KEY_TALLY_CHANNEL, ops.tallyChannel );
            prefs.putUChar( KEY_MAX_HDMI, ops.maxHDMIChannel );
            prefs.putUChar( KEY_MAX_SDI, ops.maxSDIChannel );
            prefs.putString( KEY_CHANNEL_BANK, ops.channelBank );
            prefs.putBool( KEY_AUTO_START, ops.autoStartEnabled );
            prefs.putBool( KEY_CAM_OP_MODE, ops.cameraOperatorMode );
            prefs.putUChar( KEY_BRIGHTNESS, ops.displayBrightnessLevel );
            prefs.putULong( KEY_POLL_INTERVAL, ops.statusPollInterval );
            prefs.end();

            log_i( "V-160HD configuration saved" );
            return true;
        }

        bool ConfigManager::loadV160HDConfig( StacOperations& ops ) {
            if ( !prefs.begin( NS_V160HD, READ_ONLY ) ) {
                log_w( "No V-160HD configuration found" );
                return false;
            }

            ops.switchModel = "V-160HD";
            ops.tallyChannel = prefs.getUChar( KEY_TALLY_CHANNEL, 1 );
            ops.maxHDMIChannel = prefs.getUChar( KEY_MAX_HDMI, 8 );
            ops.maxSDIChannel = prefs.getUChar( KEY_MAX_SDI, 8 );
            ops.channelBank = prefs.getString( KEY_CHANNEL_BANK, "hdmi_" );
            ops.autoStartEnabled = prefs.getBool( KEY_AUTO_START, false );
            ops.cameraOperatorMode = prefs.getBool( KEY_CAM_OP_MODE, true );
            ops.displayBrightnessLevel = prefs.getUChar( KEY_BRIGHTNESS, 1 );
            ops.statusPollInterval = prefs.getULong( KEY_POLL_INTERVAL, 300 );
            
            // V-160HD doesn't use maxChannelCount
            ops.maxChannelCount = 0;
            
            // Validate and correct channelBank based on tallyChannel
            if ( ops.tallyChannel > 8 ) {
                ops.channelBank = "sdi_";
            } else {
                ops.channelBank = "hdmi_";
            }
            
            prefs.end();

            log_i( "V-160HD configuration loaded" );
            return true;
        }

        String ConfigManager::getActiveProtocol() {
            String model;
            IPAddress ip;
            uint16_t port;
            String username, password;
            
            if ( loadSwitchConfig( model, ip, port, username, password ) ) {
                return model;
            }
            
            return "";
        }

        bool ConfigManager::hasProtocolConfig( const String& protocol ) {
            bool exists = false;
            if ( protocol == "V-60HD" ) {
                if ( prefs.begin( NS_V60HD, READ_ONLY ) ) {
                    exists = prefs.isKey( KEY_TALLY_CHANNEL );
                    prefs.end();
                }
            } else if ( protocol == "V-160HD" ) {
                if ( prefs.begin( NS_V160HD, READ_ONLY ) ) {
                    exists = prefs.isKey( KEY_TALLY_CHANNEL );
                    prefs.end();
                }
            }
            return exists;
        }

        bool ConfigManager::saveStacID( const String &stacID ) {
            if ( !prefs.begin( NS_IDENTITY, READ_WRITE ) ) {
                log_e( "Failed to open identity preferences" );
                return false;
            }

            prefs.putString( KEY_STAC_ID, stacID );
            prefs.end();

            log_i( "STAC ID saved: %s", stacID.c_str() );
            return true;
        }

        bool ConfigManager::loadStacID( String &stacID ) {
            if ( !prefs.begin( NS_IDENTITY, READ_ONLY ) ) {
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

        bool ConfigManager::savePeripheralSettings( bool cameraMode, uint8_t brightnessLevel ) {
            if ( !prefs.begin( NS_PERIPHERAL, READ_WRITE ) ) {
                log_e( "Failed to open peripheral preferences" );
                return false;
            }

            prefs.putBool( KEY_PM_CAMERA_MODE, cameraMode );
            prefs.putUChar( KEY_PM_BRIGHTNESS, brightnessLevel );
            prefs.putUChar( KEY_VERSION, Config::NVS::PM_PREFS_VERSION );
            prefs.end();

            log_i( "Peripheral settings saved: mode=%s, brightness=%d",
                   cameraMode ? "Camera" : "Talent", brightnessLevel );
            return true;
        }

        bool ConfigManager::loadPeripheralSettings( bool& cameraMode, uint8_t& brightnessLevel ) {
            if ( !prefs.begin( NS_PERIPHERAL, READ_ONLY ) ) {
                log_w( "No peripheral settings found, using defaults" );
                cameraMode = false;  // Default to Talent mode
                brightnessLevel = 1;  // Default to brightness level 1
                return false;
            }

            cameraMode = prefs.getBool( KEY_PM_CAMERA_MODE, false );
            brightnessLevel = prefs.getUChar( KEY_PM_BRIGHTNESS, 1 );
            prefs.end();

            log_i( "Peripheral settings loaded: mode=%s, brightness=%d",
                   cameraMode ? "Camera" : "Talent", brightnessLevel );
            return true;
        }

        bool ConfigManager::isConfigured() {
            return hasWiFiCredentials();
        }

        bool ConfigManager::clearAll() {
            log_i( "Clearing all NVS configuration data" );

            // Use esp-idf functions to erase entire NVS partition
            // This is simpler, faster, and more reliable than clearing each namespace individually
            esp_err_t err = nvs_flash_erase();
            if ( err != ESP_OK ) {
                log_e( "Failed to erase NVS flash: %s", esp_err_to_name( err ) );
                return false;
            }

            err = nvs_flash_init();
            if ( err != ESP_OK ) {
                log_e( "Failed to reinitialize NVS flash: %s", esp_err_to_name( err ) );
                return false;
            }

            log_i( "NVS flash erased and reinitialized" );
            return true;
        }

        uint8_t ConfigManager::getConfigVersion() {
            // Read global NOM version from wifi namespace
            if ( prefs.begin( NS_WIFI, READ_ONLY ) ) {
                uint8_t version = prefs.getUChar( KEY_VERSION, 0 );
                prefs.end();
                return version;
            }
            return 0;  // No config exists
        }

        bool ConfigManager::checkSchemaVersion() {
            uint8_t storedVersion = getConfigVersion();
            
            if ( storedVersion == 0 ) {
                log_i( "No existing NVS configuration found" );
                return true;  // Fresh device, no mismatch
            }
            
            if ( storedVersion != Config::NVS::NOM_PREFS_VERSION ) {
                log_w( "========================================" );
                log_w( "NVS SCHEMA VERSION MISMATCH" );
                log_w( "Stored version: %d", storedVersion );
                log_w( "Expected version: %d", Config::NVS::NOM_PREFS_VERSION );
                log_w( "Configuration may be incompatible" );
                log_w( "Factory reset recommended" );
                log_w( "========================================" );
                return false;  // Version mismatch detected
            }
            
            log_i( "NVS schema version OK: %d", storedVersion );
            return true;  // Version matches
        }

    } // namespace Storage



//  --- EOF --- //
