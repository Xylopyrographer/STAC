/**
 * @file WebConfigServer.h
 * @brief Web server for STAC configuration during provisioning mode
 * 
 * Provides a web-based interface for configuring STAC WiFi and Roland switch settings.
 * Creates an Access Point, serves HTML forms, and processes configuration data.
 * 
 * Ported and refactored from STACUtil.h getCreds() in baseline STAC implementation.
 * 
 * @author Rob Lake (@Xylopyrographer)
 * @date 2025-11-15
 */

#ifndef STAC_WEB_CONFIG_SERVER_H
#define STAC_WEB_CONFIG_SERVER_H

#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include "Config/Types.h"


namespace Net {

    /**
     * @brief Web server for provisioning mode configuration
     * 
     * This class manages the complete provisioning workflow:
     * 1. Creates WiFi Access Point with STAC device ID as SSID
     * 2. Serves HTML forms for switch model selection
     * 3. Serves model-specific configuration forms (V-60HD or V-160HD)
     * 4. Processes submitted form data
     * 5. Returns configuration data to caller
     * 6. Shuts down AP when complete
     * 
     * Usage:
     * @code
     * WebConfigServer configServer(stacID);
     * configServer.begin();
     * ProvisioningData data = configServer.waitForConfiguration();
     * configServer.end();
     * @endcode
     */
    class WebConfigServer {
    public:
        /**
         * @brief Callback function type for display updates during wait
         */
        using DisplayUpdateCallback = std::function<void()>;

        /**
         * @brief Constructor
         * @param deviceID Unique STAC device identifier (used as AP SSID)
         */
        explicit WebConfigServer(const String& deviceID);

        /**
         * @brief Destructor - ensures clean shutdown
         */
        ~WebConfigServer();

        /**
         * @brief Start the configuration web server
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
         * @param callback Function to call periodically during waitForConfiguration
         */
        void setDisplayUpdateCallback(DisplayUpdateCallback callback) {
            displayCallback = callback;
        }

        /**
         * @brief Wait for user to submit configuration via web interface
         * 
         * Blocks until user completes and submits the configuration form.
         * Call this after begin() to retrieve configuration data.
         * 
         * @return ProvisioningData structure containing all configuration parameters
         */
        ProvisioningData waitForConfiguration();

        /**
         * @brief Stop the configuration web server
         * 
         * - Stops web server
         * - Shuts down mDNS
         * - Turns off WiFi AP
         * - Returns WiFi to OFF mode
         */
        void end();

        /**
         * @brief Check if configuration has been received
         * @return true if user has submitted configuration, false otherwise
         */
        bool hasConfiguration() const {
            return configReceived;
        }

    private:
        // WiFi Access Point configuration
        static constexpr const char* AP_HOSTNAME = "setup";
        static constexpr const char* AP_PASSWORD = "1234567890";
        static constexpr uint8_t AP_CHANNEL = 1;
        static constexpr bool AP_HIDE_SSID = false;
        static constexpr uint8_t AP_MAX_CONNECTIONS = 1;

        // IP configuration for AP
        IPAddress apIP;
        IPAddress apGateway;
        IPAddress apNetmask;

        // Server and state
        WebServer* server;
        String deviceID;
        ProvisioningData configData;
        bool configReceived;
        bool serverRunning;
        DisplayUpdateCallback displayCallback;

        /**
         * @brief Register all HTTP endpoint handlers
         * 
         * Endpoints:
         * - GET  / : Serve model selection page
         * - POST / : Process model selection, return appropriate config form
         * - POST /parse : Process V-60HD configuration
         * - POST /parse2 : Process V-160HD configuration
         * - * (not found) : Serve 404 page
         */
        void registerEndpoints();

        /**
         * @brief Handler for GET /
         * Serves the initial model selection page with device info
         */
        void handleRoot();

        /**
         * @brief Handler for POST /
         * Processes model selection and returns appropriate config form
         */
        void handleModelSelection();

        /**
         * @brief Handler for POST /parse
         * Processes V-60HD configuration form submission
         */
        void handleV60HDConfig();

        /**
         * @brief Handler for POST /parse2
         * Processes V-160HD configuration form submission
         */
        void handleV160HDConfig();

        /**
         * @brief Handler for 404 Not Found
         */
        void handleNotFound();

        /**
         * @brief Build the complete index page with device information
         * @return Complete HTML for model selection page
         */
        String buildIndexPage() const;

        /**
         * @brief Shutdown sequence helper
         * 
         * Provides delays to ensure HTTP responses are sent before
         * shutting down server and WiFi.
         */
        void shutdownSequence();
    };

} // namespace Net


#endif // STAC_WEB_CONFIG_SERVER_H

//  --- EOF --- //
