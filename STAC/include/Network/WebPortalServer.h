/**
 * @file WebPortalServer.h
 * @brief Unified web portal for STAC configuration and OTA updates
 * 
 * Provides a single web-based interface with tabbed navigation for:
 * - Device commissioning (WiFi and Roland switch configuration)
 * - OTA firmware updates
 * 
 * Replaces separate WebConfigServer and OTAUpdateServer with unified approach.
 * 
 * @author Rob Lake (@Xylopyrographer)
 * @date 2026-01-07
 */

#ifndef STAC_WEB_PORTAL_SERVER_H
#define STAC_WEB_PORTAL_SERVER_H

#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <Update.h>
#include <DNSServer.h>
#include "Config/Types.h"


namespace Net {

    /**
     * @brief Result data from OTA update operation
     */
    struct OTAUpdateResult {
        bool success;                  ///< Whether update succeeded
        String filename;               ///< Name of uploaded file
        size_t bytesWritten;          ///< Total bytes written to flash
        String statusMessage;          ///< Human-readable status message
        
        OTAUpdateResult() 
            : success(false)
            , filename("")
            , bytesWritten(0)
            , statusMessage("")
        {}
    };

    /**
     * @brief Unified web portal server for provisioning and OTA updates
     * 
     * This class manages both device commissioning and firmware updates through
     * a single tabbed web interface with captive portal support:
     * 
     * 1. Creates WiFi Access Point with STAC device ID as SSID
     * 2. Starts DNS server to capture all requests (captive portal)
     * 3. Serves tabbed HTML interface with Setup and Maintenance tabs
     * 4. Handles configuration form submission (WiFi + Roland switch settings)
     * 5. Handles firmware upload and flashing
     * 6. Handles factory reset requests
     * 7. Provides automatic browser popup on connection (iOS, Android, Windows, macOS)
     * 
     * Usage:
     * @code
     * WebPortalServer portal(stacID);
     * portal.begin();
     * 
     * // Wait for either config or OTA to complete
     * PortalResult result = portal.waitForCompletion();
     * 
     * if (result.type == PortalResultType::CONFIG_RECEIVED) {
     *     // Handle configuration
     *     applyConfig(result.configData);
     * } else if (result.type == PortalResultType::OTA_SUCCESS) {
     *     // OTA success - will auto-restart
     * }
     * 
     * portal.end();
     * @endcode
     */
    class WebPortalServer {
    public:
        /**
         * @brief Type of result from portal operation
         */
        enum class PortalResultType {
            NONE,              ///< No action taken yet
            CONFIG_RECEIVED,   ///< Configuration submitted
            OTA_SUCCESS,       ///< OTA update completed successfully
            OTA_FAILED,        ///< OTA update failed
            FACTORY_RESET      ///< Factory reset requested
        };

        /**
         * @brief Result from portal wait operation
         */
        struct PortalResult {
            PortalResultType type;
            ProvisioningData configData;
            OTAUpdateResult otaResult;
            
            PortalResult() : type(PortalResultType::NONE) {}
        };

        /**
         * @brief Callback function type for display updates during wait
         */
        using DisplayUpdateCallback = std::function<void()>;

        /**
         * @brief Callback function type for polling reset button during wait
         * Should return true if reset was requested
         */
        using ResetCheckCallback = std::function<bool()>;

        /**
         * @brief Callback function type for pre-restart cleanup (e.g., turn off backlight)
         */
        using PreRestartCallback = std::function<void()>;

        /**
         * @brief Constructor
         * @param deviceID Unique STAC device identifier (used as AP SSID)
         */
        explicit WebPortalServer(const String& deviceID);

        /**
         * @brief Destructor - ensures clean shutdown
         */
        ~WebPortalServer();

        /**
         * @brief Start the web portal server
         * 
         * - Creates WiFi Access Point
         * - Configures IP addressing (192.168.6.14)
         * - Starts mDNS responder (hostname: "setup")
         * - Registers HTTP endpoint handlers
         * - Begins listening for client connections
         * 
         * @return true if server started successfully, false otherwise
         */
        bool begin();

        /**
         * @brief Set callback for periodic display updates
         * @param callback Function to call periodically during waitForCompletion
         */
        void setDisplayUpdateCallback(DisplayUpdateCallback callback) {
            displayCallback = callback;
        }

        /**
         * @brief Set callback for checking reset button during wait
         * @param callback Function that polls button and returns true if reset requested
         */
        void setResetCheckCallback(ResetCheckCallback callback) {
            resetCheckCallback = callback;
        }

        /**
         * @brief Set callback for pre-restart cleanup (e.g., turn off TFT backlight)
         * @param callback Function to call before ESP.restart()
         */
        void setPreRestartCallback(PreRestartCallback callback) {
            preRestartCallback = callback;
        }

        /**
         * @brief Wait for user action via web interface
         * 
         * Blocks until user either:
         * - Submits configuration form (Setup tab)
         * - Completes firmware upload (Update tab)
         * - Presses reset button (via callback)
         * 
         * @return PortalResult structure indicating what happened
         */
        PortalResult waitForCompletion();

        /**
         * @brief Stop the web portal server
         * 
         * - Stops web server
         * - Shuts down mDNS
         * - Turns off WiFi AP
         * - Returns WiFi to OFF mode
         */
        void end();

    private:
        // WiFi Access Point configuration
        static constexpr const char* AP_HOSTNAME = "setup";
        static constexpr const char* AP_PASSWORD = "1234567890";
        static constexpr uint8_t AP_CHANNEL = 1;
        static constexpr bool AP_HIDE_SSID = false;
        static constexpr uint8_t AP_MAX_CONNECTIONS = 1;

        // DNS Server configuration for captive portal
        static constexpr uint8_t DNS_PORT = 53;

        // Display update interval (ms)
        static constexpr unsigned long DISPLAY_UPDATE_INTERVAL = 1000;

        // IP configuration for AP
        IPAddress apIP;
        IPAddress apGateway;
        IPAddress apNetmask;

        // Server and state
        WebServer* server;
        DNSServer* dnsServer;
        String deviceID;
        String macAddress;
        bool serverRunning;
        
        // Result tracking
        PortalResult result;
        bool operationComplete;

        // Callbacks
        DisplayUpdateCallback displayCallback;
        ResetCheckCallback resetCheckCallback;
        PreRestartCallback preRestartCallback;
        unsigned long lastDisplayUpdate;

        /**
         * @brief Register all HTTP endpoint handlers
         * 
         * Endpoints:
         * - GET  / : Serve tabbed portal index page
         * - POST /config : Process configuration submission
         * - POST /update : Handle firmware upload
         * - POST /factory-reset : Handle factory reset request
         * - * (not found) : Serve 404 page
         */
        void registerEndpoints();

        /**
         * @brief Handler for GET /
         * Serves the tabbed portal page with device info
         */
        void handleRoot();

        /**
         * @brief Handler for POST /config
         * Processes configuration form submission (from Setup tab)
         */
        void handleConfigSubmit();

        /**
         * @brief Handler for POST /factory-reset
         * Processes factory reset request (from Maintenance tab)
         */
        void handleFactoryReset();

        /**
         * @brief Handler for POST /update (completion)
         * Called after firmware upload completes
         */
        void handleUpdateComplete();

        /**
         * @brief Handler for POST /update (file upload chunks)
         * Processes uploaded firmware data
         */
        void handleFileUpload();

        /**
         * @brief Build the main portal index page with tabs
         * @return Complete HTML page as String
         */
        String buildIndexPage() const;

        /**
         * @brief Build OTA result page
         * @return Complete HTML result page as String
         */
        String buildOTAResultPage() const;

        /**
         * @brief Shutdown sequence with delays for clean WiFi shutdown
         */
        void shutdownSequence();
    };

} // namespace Net

#endif // STAC_WEB_PORTAL_SERVER_H


//  --- EOF --- //
