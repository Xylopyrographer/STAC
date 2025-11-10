#ifndef STAC_CONFIG_MANAGER_H
#define STAC_CONFIG_MANAGER_H

#include <Preferences.h>
#include <IPAddress.h>
#include "Config/Types.h"
#include "Config/Constants.h"

namespace STAC {
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
             * @brief Clear WiFi credentials
             */
            void clearWiFiCredentials();

            // ========================================================================
            // Switch Configuration
            // ========================================================================

            /**
             * @brief Save switch configuration
             * @param model Switch model ("V-60HD" or "V-160HD")
             * @param ipAddress Switch IP address
             * @param port Switch HTTP port
             * @return true if saved successfully
             */
            bool saveSwitchConfig( const String &model, const IPAddress& ipAddress, uint16_t port );

            /**
             * @brief Load switch configuration
             * @param model Output: Switch model
             * @param ipAddress Output: Switch IP address
             * @param port Output: Switch HTTP port
             * @return true if configuration exists
             */
            bool loadSwitchConfig( String &model, IPAddress& ipAddress, uint16_t &port );

            // ========================================================================
            // Operating Parameters
            // ========================================================================

            /**
             * @brief Save operating parameters
             * @param ops StacOperations structure
             * @return true if saved successfully
             */
            bool saveOperations( const StacOperations& ops );

            /**
             * @brief Load operating parameters
             * @param ops Output: StacOperations structure
             * @return true if parameters exist
             */
            bool loadOperations( StacOperations& ops );

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
             * @return true if cleared successfully
             */
            bool clearAll();

            /**
             * @brief Get configuration version
             * @return Version number
             */
            uint8_t getConfigVersion();

          private:
            Preferences prefs;

            // Namespace names for different config areas
            static constexpr const char *NS_WIFI = "wifi";
            static constexpr const char *NS_SWITCH = "switch";
            static constexpr const char *NS_OPERATIONS = "operations";
            static constexpr const char *NS_IDENTITY = "identity";

            // Key names
            static constexpr const char *KEY_VERSION = "version";
            static constexpr const char *KEY_SSID = "ssid";
            static constexpr const char *KEY_PASSWORD = "password";
            static constexpr const char *KEY_MODEL = "model";
            static constexpr const char *KEY_IP = "ip";
            static constexpr const char *KEY_PORT = "port";
            static constexpr const char *KEY_STAC_ID = "stacid";

            /**
             * @brief Check and migrate configuration if needed
             * @return true if migration successful or not needed
             */
            bool checkAndMigrateConfig();
        };

    } // namespace Storage
} // namespace STAC

#endif // STAC_CONFIG_MANAGER_H


//  --- EOF --- //
