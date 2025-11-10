#ifndef STAC_WIFI_MANAGER_H
#define STAC_WIFI_MANAGER_H

#include <WiFi.h>
#include <cstdint>
#include <WString.h>
#include "Config/Constants.h"

namespace STAC {
    namespace Network {

        /**
         * @brief WiFi connection state
         */
        enum class WiFiState : uint8_t {
            DISCONNECTED,       ///< Not connected
            CONNECTING,         ///< Connection in progress
            CONNECTED,          ///< Connected to network
            FAILED,             ///< Connection failed
            AP_MODE             ///< Running as access point
        };

        /**
         * @brief WiFi connection manager
         *
         * Handles WiFi station and access point modes,
         * connection management, and reconnection logic.
         */
        class WiFiManager {
          public:
            WiFiManager();
            ~WiFiManager() = default;

            /**
             * @brief Initialize WiFi subsystem
             * @return true if initialization succeeded
             */
            bool begin();

            /**
             * @brief Connect to WiFi network (station mode)
             * @param ssid Network SSID
             * @param password Network password
             * @param timeoutMs Connection timeout in milliseconds
             * @return true if connected successfully
             */
            bool connect( const String &ssid, const String &password,
                          unsigned long timeoutMs = Config::Timing::WIFI_CONNECT_TIMEOUT_MS );

            /**
             * @brief Start access point mode
             * @param ssid AP SSID
             * @param password AP password (empty for open network)
             * @return true if AP started successfully
             */
            bool startAP( const String &ssid, const String &password = "" );

            /**
             * @brief Disconnect from WiFi
             */
            void disconnect();

            /**
             * @brief Stop access point
             */
            void stopAP();

            /**
             * @brief Check if connected to WiFi
             * @return true if connected
             */
            bool isConnected() const;

            /**
             * @brief Check if running as access point
             * @return true if AP is active
             */
            bool isAPMode() const;

            /**
             * @brief Get current WiFi state
             * @return WiFiState enum value
             */
            WiFiState getState() const;

            /**
             * @brief Get local IP address
             * @return IP address as string
             */
            String getLocalIP() const;

            /**
             * @brief Get MAC address
             * @return MAC address as string
             */
            String getMacAddress() const;

            /**
             * @brief Get signal strength (RSSI)
             * @return RSSI in dBm
             */
            int32_t getRSSI() const;

            /**
             * @brief Set hostname for mDNS
             * @param hostname Hostname to set
             * @return true if successful
             */
            bool setHostname( const String &hostname );

            /**
             * @brief Get current hostname
             * @return Hostname string
             */
            String getHostname() const;

            /**
             * @brief Handle WiFi events and maintain connection
             * Call this regularly in loop
             */
            void update();

          private:
            WiFiState state;
            String currentSSID;
            String currentPassword;
            String hostname;
            bool apMode;
            unsigned long lastConnectionAttempt;
            static constexpr unsigned long RECONNECT_INTERVAL_MS = 30000;  // 30 seconds

            /**
             * @brief Attempt to reconnect if disconnected
             */
            void attemptReconnect();
        };

    } // namespace Network
} // namespace STAC

#endif // STAC_WIFI_MANAGER_H


//  --- EOF --- //
