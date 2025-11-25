#ifndef STAC_CONFIG_MANAGER_H
#define STAC_CONFIG_MANAGER_H

#include <Preferences.h>
#include <IPAddress.h>
#include "Config/Types.h"
#include "Config/Constants.h"


    namespace Storage {

        /**
         * @brief Non-volatile storage manager for STAC configuration
         *
         * Manages persistent storage of configuration using ESP32 Preferences (NVS).
         * Handles versioning and migration of stored data.
         */
        class ConfigManager {
          public:
            ConfigManager();
            ~ConfigManager() = default;

            /**
             * @brief Initialize the configuration manager
             * @return true if initialization succeeded
             */
            bool begin();

            // ========================================================================
            // WiFi Configuration
            // ========================================================================

            /**
             * @brief Save WiFi credentials
             * @param ssid Network SSID
             * @param password Network password
             * @return true if saved successfully
             */
            bool saveWiFiCredentials( const String &ssid, const String &password );

            /**
             * @brief Load WiFi credentials
             * @param ssid Output: Network SSID
             * @param password Output: Network password
             * @return true if credentials exist
             */
            bool loadWiFiCredentials( String &ssid, String &password );

            /**
             * @brief Check if WiFi credentials are stored
             * @return true if credentials exist
             */
            bool hasWiFiCredentials();

            /**
             * @brief Check if device is provisioned (has complete configuration)
             * @return true if device has WiFi credentials and switch configuration
             */
            bool isProvisioned();

            /**
             * @brief Clear WiFi credentials
             */
            void clearWiFiCredentials();

            // ========================================================================
            // Switch Configuration
            // ========================================================================

            // @Claude: Should the model be an enum instead of a string for better type safety and performance?
            /**
             * @brief Save switch configuration
             * @param model Switch model ("V-60HD" or "V-160HD")
             * @param ipAddress Switch IP address
             * @param port Switch HTTP port
             * @param username Username for authentication (V-160HD only, optional)
             * @param password Password for authentication (V-160HD only, optional)
             * @return true if saved successfully
             */
            bool saveSwitchConfig( const String &model, const IPAddress& ipAddress, uint16_t port,
                                   const String &username = "", const String &password = "" );

            /**
             * @brief Load switch configuration
             * @param model Output: Switch model
             * @param ipAddress Output: Switch IP address
             * @param port Output: Switch HTTP port
             * @param username Output: Username for authentication (may be empty)
             * @param password Output: Password for authentication (may be empty)
             * @return true if configuration exists
             */
            bool loadSwitchConfig( String &model, IPAddress& ipAddress, uint16_t &port,
                                   String &username, String &password );

            // ========================================================================
            // Protocol-Specific Operating Parameters
            // ========================================================================

            /**
             * @brief Save V-60HD protocol configuration
             * @param ops StacOperations structure containing V-60HD settings
             * @return true if saved successfully
             */
            bool saveV60HDConfig( const StacOperations& ops );

            /**
             * @brief Load V-60HD protocol configuration
             * @param ops Output: StacOperations structure with V-60HD settings
             * @return true if configuration exists
             */
            bool loadV60HDConfig( StacOperations& ops );

            /**
             * @brief Save V-160HD protocol configuration
             * @param ops StacOperations structure containing V-160HD settings
             * @return true if saved successfully
             */
            bool saveV160HDConfig( const StacOperations& ops );

            /**
             * @brief Load V-160HD protocol configuration
             * @param ops Output: StacOperations structure with V-160HD settings
             * @return true if configuration exists
             */
            bool loadV160HDConfig( StacOperations& ops );

            /**
             * @brief Get currently active protocol
             * @return Protocol name ("V-60HD", "V-160HD", or empty if not set)
             */
            String getActiveProtocol();

            /**
             * @brief Check if a specific protocol has configuration stored
             * @param protocol Protocol name ("V-60HD" or "V-160HD")
             * @return true if protocol configuration exists
             */
            bool hasProtocolConfig( const String& protocol );

            // ========================================================================
            // STAC Identity
            // ========================================================================

            /**
             * @brief Save STAC ID
             * @param stacID Unique STAC identifier
             * @return true if saved successfully
             */
            bool saveStacID( const String &stacID );

            /**
             * @brief Load STAC ID
             * @param stacID Output: STAC identifier
             * @return true if ID exists
             */
            bool loadStacID( String &stacID );

            /**
             * @brief Generate and save a new STAC ID
             * @return Generated STAC ID
             */
            String generateAndSaveStacID();

            // ========================================================================
            // Utility
            // ========================================================================

            /**
             * @brief Check if configuration has been initialized
             * @return true if configuration exists
             */
            bool isConfigured();

            /**
             * @brief Clear all stored configuration
             * @return true if successful
             */
            bool clearAll();

            /**
             * @brief Save peripheral mode settings
             * @param cameraMode Camera operator mode (true) or talent mode (false)
             * @param brightnessLevel Brightness level (1-6 for 5x5, 1-8 for 8x8)
             * @return true if successful
             */
            bool savePeripheralSettings( bool cameraMode, uint8_t brightnessLevel );

            /**
             * @brief Load peripheral mode settings
             * @param cameraMode Output: camera operator mode
             * @param brightnessLevel Output: brightness level
             * @return true if settings exist
             */
            bool loadPeripheralSettings( bool& cameraMode, uint8_t& brightnessLevel );

            /**
             * @brief Get configuration version
             * @return Version number
             */
            uint8_t getConfigVersion();

          private:

            /*
            * @Claude: We only have a single Preferences namespace defined, not separate ones for 
            * each config area as we discussed. This setup doesn't give us the separation we wanted.
            * It creates all keys in the same namespace, not dynamically as needed.
            * Managing each makes it a bit more work though for sure.
            * To simplify clearing NVS later for factory reset we should use the esp-idf functions:
            *   nvs_flash_erase(void)
            *   nvs_flash_init(void)
            */

            Preferences prefs;

            // @Claude: Again, these aren't separate namespaces? They all exist in the same 'prefs' instance?
            // Namespace names for different config areas
            static constexpr const char *NS_WIFI = "wifi";
            static constexpr const char *NS_SWITCH = "switch";
            static constexpr const char *NS_V60HD = "v60hd";
            static constexpr const char *NS_V160HD = "v160hd";
            static constexpr const char *NS_IDENTITY = "identity";
            static constexpr const char *NS_PERIPHERAL = "peripheral";

            // Key names - WiFi
            static constexpr const char *KEY_VERSION = "version";
            static constexpr const char *KEY_SSID = "ssid";
            static constexpr const char *KEY_PASSWORD = "password";
            
            // Key names - Switch
            static constexpr const char *KEY_MODEL = "model";
            static constexpr const char *KEY_IP = "ip";
            static constexpr const char *KEY_PORT = "port";
            static constexpr const char *KEY_USERNAME = "username";
            
            // Key names - Identity
            static constexpr const char *KEY_STAC_ID = "stacid";
            
            // Key names - Protocol Operations (common)
            static constexpr const char *KEY_TALLY_CHANNEL = "tallyChannel";
            static constexpr const char *KEY_BRIGHTNESS = "brightness";
            static constexpr const char *KEY_AUTO_START = "autoStart";
            static constexpr const char *KEY_CAM_OP_MODE = "camOpMode";
            static constexpr const char *KEY_POLL_INTERVAL = "pollInterval";
            
            // Key names - V-60HD specific
            static constexpr const char *KEY_MAX_CHANNEL = "maxChannel";
            
            // Key names - V-160HD specific
            static constexpr const char *KEY_MAX_HDMI = "maxHDMI";
            static constexpr const char *KEY_MAX_SDI = "maxSDI";
            static constexpr const char *KEY_CHANNEL_BANK = "channelBank";
            
            // Key names - Peripheral Mode
            static constexpr const char *KEY_PM_CAMERA_MODE = "pmCamMode";
            static constexpr const char *KEY_PM_BRIGHTNESS = "pmBrightness";

            /**
             * @brief Check and migrate configuration if needed
             * @return true if migration successful or not needed
             */
            bool checkAndMigrateConfig();
        };

    } // namespace Storage


#endif // STAC_CONFIG_MANAGER_H


//  --- EOF --- //
