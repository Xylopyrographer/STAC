/**
 * @file OTAUpdateServer.h
 * @brief Web server for STAC firmware OTA updates
 * 
 * Provides a web-based interface for uploading and flashing new firmware.
 * Creates an Access Point, serves upload form, and handles firmware updates.
 * 
 * Ported and refactored from STACUtil.h STACupdate() in baseline STAC implementation.
 * 
 * @author Rob Lake (@Xylopyrographer)
 * @date 2025-11-15
 */

#ifndef STAC_OTA_UPDATE_SERVER_H
#define STAC_OTA_UPDATE_SERVER_H

#include <WebServer.h>
#include <ESPmDNS.h>
#include <WiFi.h>
#include <Update.h>


namespace Net {

    /**
     * @brief Result of an OTA update attempt
     */
    struct OTAUpdateResult {
        bool success;           ///< true if update succeeded
        String filename;        ///< Name of uploaded firmware file
        size_t bytesWritten;    ///< Number of bytes written to flash
        String statusMessage;   ///< Status or error message

        OTAUpdateResult()
            : success(false)
            , filename("")
            , bytesWritten(0)
            , statusMessage("Not started")
        {}
    };

    /**
     * @brief Web server for OTA firmware updates
     * 
     * This class manages the complete OTA update workflow:
     * 1. Creates WiFi Access Point with STAC device ID as SSID
     * 2. Serves HTML form for firmware file selection
     * 3. Receives uploaded .bin file via HTTP POST
     * 4. Writes firmware to OTA partition using Update library
     * 5. Displays success/failure page
     * 6. Automatically restarts ESP32 on completion
     * 
     * Usage:
     * @code
     * OTAUpdateServer otaServer(stacID);
     * otaServer.begin();
     * otaServer.waitForUpdate(); // Blocks until update complete, then restarts
     * @endcode
     */
    class OTAUpdateServer {
    public:
        /**
         * @brief Constructor
         * @param deviceID Unique STAC device identifier (used as AP SSID)
         */
        explicit OTAUpdateServer(const String& deviceID);

        /**
         * @brief Destructor - ensures clean shutdown
         */
        ~OTAUpdateServer();

        /**
         * @brief Start the OTA update web server
         * 
         * - Creates WiFi Access Point
         * - Configures IP addressing (192.168.6.14)
         * - Starts mDNS responder (hostname: "update")
         * - Registers HTTP endpoint handlers
         * - Begins listening for client connections
         * 
         * @return true if server started successfully, false otherwise
         */
        bool begin();

        /**
         * @brief Wait for firmware upload and perform update
         * 
         * Blocks until user uploads firmware file. Processes the upload,
         * flashes firmware, displays result page, and restarts ESP32.
         * 
         * This function does NOT return if update is successful (ESP32 restarts).
         * Returns only on failure or if cancelled.
         * 
         * @return OTAUpdateResult containing update status (only on failure)
         */
        OTAUpdateResult waitForUpdate();

        /**
         * @brief Stop the OTA update web server
         * 
         * - Stops web server
         * - Shuts down mDNS
         * - Turns off WiFi AP
         * - Returns WiFi to OFF mode
         */
        void end();

    private:
        // WiFi Access Point configuration
        static constexpr const char* AP_HOSTNAME = "update";
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
        OTAUpdateResult updateResult;
        bool updateComplete;
        bool serverRunning;

        /**
         * @brief Register all HTTP endpoint handlers
         * 
         * Endpoints:
         * - GET  / : Serve firmware upload page
         * - POST /update : Handle firmware file upload and flashing
         * - * (not found) : Serve 404 page
         */
        void registerEndpoints();

        /**
         * @brief Handler for GET /
         * Serves the firmware upload page
         */
        void handleRoot();

        /**
         * @brief Handler for POST /update (after upload completes)
         * Sends result page to client and triggers restart
         */
        void handleUpdateComplete();

        /**
         * @brief Handler for file upload during POST /update
         * Receives firmware file chunks and writes to flash
         */
        void handleFileUpload();

        /**
         * @brief Handler for 404 Not Found
         */
        void handleNotFound();

        /**
         * @brief Build the update result HTML page
         * @return Complete HTML page with update status
         */
        String buildResultPage() const;

        /**
         * @brief Shutdown sequence helper
         * 
         * Provides delays to ensure HTTP responses are sent before
         * shutting down server, WiFi, and restarting.
         */
        void shutdownSequence();
    };

} // namespace Net


#endif // STAC_OTA_UPDATE_SERVER_H

//  --- EOF --- //
