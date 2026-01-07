/**
 * @file WebPortalServer.cpp
 * @brief Implementation of unified web portal server
 * 
 * @author Rob Lake (@Xylopyrographer)
 * @date 2026-01-07
 */

#include "Network/WebPortalServer.h"
#include "Network/WebPortalPages.h"
#include <esp_wifi.h>
#include <esp_mac.h>


namespace Net {

    WebPortalServer::WebPortalServer(const String& deviceID)
        : apIP(192, 168, 6, 14)
        , apGateway(0, 0, 0, 0)
        , apNetmask(255, 255, 255, 0)
        , server(nullptr)
        , deviceID(deviceID)
        , serverRunning(false)
        , operationComplete(false)
        , displayCallback(nullptr)
        , resetCheckCallback(nullptr)
        , preRestartCallback(nullptr)
        , lastDisplayUpdate(0)
    {
        // Get MAC address and format it for display
        uint8_t mac[6];
        esp_efuse_mac_get_default(mac);
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        macAddress = String(macStr);
    }

    WebPortalServer::~WebPortalServer() {
        end();
    }

    bool WebPortalServer::begin() {
        log_i("Starting web portal server");

        // Turn off WiFi radio first
        WiFi.mode(WIFI_MODE_NULL);
        while (WiFi.getMode() != WIFI_MODE_NULL) {
            delay(10);
        }

        // Configure WiFi mode to AP
        WiFi.mode(WIFI_AP);
        while (WiFi.getMode() != WIFI_MODE_AP) {
            delay(10);
        }

        // Start Access Point
        bool apStarted = WiFi.softAP(
            deviceID.c_str(),
            AP_PASSWORD,
            AP_CHANNEL,
            AP_HIDE_SSID,
            AP_MAX_CONNECTIONS
        );

        if (!apStarted) {
            log_e("Failed to start WiFi AP");
            return false;
        }

        // Configure AP hostname and networking
        WiFi.softAPsetHostname(AP_HOSTNAME);
        WiFi.softAPConfig(apIP, apIP, apNetmask);

        // Start mDNS responder
        if (!MDNS.begin(AP_HOSTNAME)) {
            log_e("Failed to start mDNS responder");
            return false;
        }

        log_i("AP started - SSID: %s, IP: %s", deviceID.c_str(), apIP.toString().c_str());

        // Create and configure web server
        server = new WebServer(80);
        registerEndpoints();
        server->begin();
        server->enableDelay(false); // Disable 1ms delay in handleClient()

        serverRunning = true;
        log_i("Web portal server started on port 80");

        return true;
    }

    WebPortalServer::PortalResult WebPortalServer::waitForCompletion() {
        if (!serverRunning) {
            log_e("Server not running - call begin() first");
            return result;
        }

        log_i("Waiting for user action via web portal");

        // Reset state
        operationComplete = false;
        result.type = PortalResultType::NONE;
        lastDisplayUpdate = millis();

        // Handle client requests until operation completes
        while (!operationComplete) {
            server->handleClient();
            
            // Check for reset button via callback
            if (resetCheckCallback && resetCheckCallback()) {
                log_i("Reset requested during portal session - restarting");
                if (preRestartCallback) {
                    preRestartCallback();  // Call cleanup (e.g., turn off backlight)
                }
                ESP.restart();
            }
            
            // Update pulsing display via callback
            unsigned long now = millis();
            if (displayCallback && (now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL)) {
                lastDisplayUpdate = now;
                displayCallback();
            }
            
            yield();
        }

        log_i("Portal operation complete: type=%d", static_cast<int>(result.type));
        return result;
    }

    void WebPortalServer::end() {
        if (!serverRunning) {
            return;
        }

        log_i("Stopping web portal server");

        // Stop server
        if (server) {
            server->stop();
            server->close();
            delete server;
            server = nullptr;
        }

        // Stop mDNS
        MDNS.end();

        // Shutdown sequence with delays
        shutdownSequence();

        serverRunning = false;
        log_i("Web portal server stopped");
    }

    void WebPortalServer::registerEndpoints() {
        // GET / - Serve tabbed portal index page
        server->on("/", HTTP_GET, [this]() {
            handleRoot();
        });

        // POST /config - Process configuration submission
        server->on("/config", HTTP_POST, [this]() {
            handleConfigSubmit();
        });

        // POST /update - Handle firmware upload and flashing
        server->on("/update", HTTP_POST,
            [this]() {
                // This lambda runs after upload completes
                handleUpdateComplete();
            },
            [this]() {
                // This lambda handles the actual file upload chunks
                handleFileUpload();
            }
        );

        // 404 handler
        server->onNotFound([this]() {
            handleNotFound();
        });
    }

    void WebPortalServer::handleRoot() {
        String page = buildIndexPage();
        server->send(200, "text/html", page);
        log_v("Served portal index page");
    }

    void WebPortalServer::handleConfigSubmit() {
        log_i("Processing configuration submission");

        // Send confirmation page immediately
        server->send(200, "text/html", WebPortal::CONFIG_RECEIVED);
        
        // Ensure response is sent before continuing
        delay(100);

        // Extract form data
        String model = server->arg("stModel");
        result.configData.switchModel = model;
        result.configData.wifiSSID = server->arg("SSID");
        result.configData.wifiPassword = server->arg("pwd");
        result.configData.switchIPString = server->arg("stIP");
        result.configData.switchPort = static_cast<uint16_t>(server->arg("stPort").toInt());
        result.configData.pollInterval = static_cast<unsigned long>(server->arg("pollTime").toInt());

        if (model == "V-60HD") {
            result.configData.maxChannel = static_cast<uint8_t>(server->arg("stChan").toInt());
            // Set defaults for unused V-160HD fields
            result.configData.lanUserID = "";
            result.configData.lanPassword = "";
            result.configData.maxHDMIChannel = 0;
            result.configData.maxSDIChannel = 0;
            
            log_i("  V-60HD Config:");
            log_i("    WiFi SSID: %s", result.configData.wifiSSID.c_str());
            log_i("    Switch IP: %s:%d", result.configData.switchIPString.c_str(), result.configData.switchPort);
            log_i("    Max Channel: %d", result.configData.maxChannel);
            log_i("    Poll Interval: %lu ms", result.configData.pollInterval);
        }
        else if (model == "V-160HD") {
            result.configData.lanUserID = server->arg("stnetUser");
            result.configData.lanPassword = server->arg("stnetPW");
            result.configData.maxHDMIChannel = static_cast<uint8_t>(server->arg("stChanHDMI").toInt());
            result.configData.maxSDIChannel = static_cast<uint8_t>(server->arg("stChanSDI").toInt());
            // Set default for unused V-60HD field
            result.configData.maxChannel = 0;
            
            log_i("  V-160HD Config:");
            log_i("    WiFi SSID: %s", result.configData.wifiSSID.c_str());
            log_i("    Switch IP: %s:%d", result.configData.switchIPString.c_str(), result.configData.switchPort);
            log_i("    LAN User: %s", result.configData.lanUserID.c_str());
            log_i("    Max HDMI: %d, Max SDI: %d", result.configData.maxHDMIChannel, result.configData.maxSDIChannel);
            log_i("    Poll Interval: %lu ms", result.configData.pollInterval);
        }

        result.type = PortalResultType::CONFIG_RECEIVED;
        operationComplete = true;
    }

    void WebPortalServer::handleUpdateComplete() {
        // Build and send result page
        String resultPage = buildOTAResultPage();
        server->send(200, "text/html", resultPage);

        log_i("Update result page sent to client");
        log_i("  Filename: %s", result.otaResult.filename.c_str());
        log_i("  Bytes: %d", result.otaResult.bytesWritten);
        log_i("  Status: %s", result.otaResult.statusMessage.c_str());

        // Set result type based on success
        result.type = result.otaResult.success ? PortalResultType::OTA_SUCCESS : PortalResultType::OTA_FAILED;
        operationComplete = true;
    }

    void WebPortalServer::handleFileUpload() {
        HTTPUpload& upload = server->upload();

        if (upload.status == UPLOAD_FILE_START) {
            // Start of upload
            result.otaResult.filename = upload.filename;
            result.otaResult.success = false;
            result.otaResult.bytesWritten = 0;
            result.otaResult.statusMessage = "Starting upload...";

            log_i("Firmware upload started: %s", upload.filename.c_str());
            log_i("Expected file size: %d bytes", upload.totalSize);

            // Begin OTA update with UPDATE_SIZE_UNKNOWN to auto-detect
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                result.otaResult.statusMessage = "Failed to begin OTA update";
                result.otaResult.success = false;
                log_e("Update.begin() failed");
                Update.printError(Serial);
            } else {
                log_i("OTA partition ready for writing (size: %d bytes)", Update.size());
            }
        }
        else if (upload.status == UPLOAD_FILE_WRITE) {
            // Write chunk to flash
            size_t written = Update.write(upload.buf, upload.currentSize);
            if (written != upload.currentSize) {
                log_e("Write error: expected %d bytes, wrote %d bytes", 
                      upload.currentSize, written);
            }
            result.otaResult.bytesWritten += written;
            
            // Log progress periodically
            if (result.otaResult.bytesWritten % 32768 == 0) {
                log_v("Written: %d bytes", result.otaResult.bytesWritten);
            }
        }
        else if (upload.status == UPLOAD_FILE_END) {
            // Upload complete
            if (Update.end(true)) {
                // Success!
                result.otaResult.success = true;
                result.otaResult.statusMessage = "Firmware updated successfully";
                log_i("OTA update successful! Total bytes: %d", result.otaResult.bytesWritten);
            } else {
                // Failed
                result.otaResult.success = false;
                result.otaResult.statusMessage = "Update failed: " + String(Update.errorString());
                log_e("OTA update failed: %s", Update.errorString());
                Update.printError(Serial);
            }
        }
        else if (upload.status == UPLOAD_FILE_ABORTED) {
            // Upload was cancelled
            Update.abort();
            result.otaResult.success = false;
            result.otaResult.statusMessage = "Upload cancelled by user";
            log_w("OTA upload aborted");
        }
    }

    void WebPortalServer::handleNotFound() {
        server->send(404, "text/html", WebPortal::NOT_FOUND);
        log_v("404 served for: %s", server->uri().c_str());
    }

    String WebPortalServer::buildIndexPage() const {
        // Get firmware version info
        // TODO: Replace with actual version strings from build system
        String fwVersion = "3.0.0-unified";
        String coreVersion = String(ESP_ARDUINO_VERSION_MAJOR) + "." +
                             String(ESP_ARDUINO_VERSION_MINOR) + "." +
                             String(ESP_ARDUINO_VERSION_PATCH);
        String sdkVersion = esp_get_idf_version();

        // Build complete page
        String page;
        page.reserve(8192);  // Pre-allocate to reduce fragmentation
        
        page += WebPortal::PAGE_HEAD;
        page += WebPortal::DEVICE_INFO_OPEN;
        page += "<p><strong>Device:</strong> " + deviceID + "</p>";
        page += "<p><strong>MAC:</strong> " + macAddress + "</p>";
        page += "<p><strong>Firmware:</strong> " + fwVersion + "</p>";
        page += "<p><strong>Core:</strong> " + coreVersion + " | <strong>SDK:</strong> " + sdkVersion + "</p>";
        page += WebPortal::DEVICE_INFO_CLOSE;
        page += WebPortal::TAB_BUTTONS;
        page += WebPortal::TAB_SETUP;
        page += WebPortal::TAB_UPDATE;
        page += WebPortal::PAGE_SCRIPT;
        page += WebPortal::PAGE_FOOTER;

        return page;
    }

    String WebPortalServer::buildOTAResultPage() const {
        String page;
        page.reserve(1024);
        
        page += WebPortal::OTA_PAGE_OPEN;
        
        if (result.otaResult.success) {
            page += "<h1 class=\"success\">✓ Update Successful</h1>";
            page += "<p>Firmware file: <span class=\"filename\">" + result.otaResult.filename + "</span></p>";
            page += "<p>Bytes written: " + String(result.otaResult.bytesWritten) + "</p>";
            page += "<p><strong>STAC is restarting...</strong></p>";
            page += "<p>Please wait for the device to reboot.</p>";
        } else {
            page += "<h1 class=\"error\">✗ Update Failed</h1>";
            page += "<p class=\"error\">" + result.otaResult.statusMessage + "</p>";
            page += "<p>Firmware file: <span class=\"filename\">" + result.otaResult.filename + "</span></p>";
            page += "<p>Bytes written: " + String(result.otaResult.bytesWritten) + "</p>";
            page += "<p><a href=\"/\">Return to Portal</a></p>";
        }
        
        page += WebPortal::OTA_PAGE_CLOSE;
        
        return page;
    }

    void WebPortalServer::shutdownSequence() {
        // Pause to allow HTTP response to be sent
        unsigned long pauseTime = millis() + 500UL;
        while (millis() < pauseTime) {
            yield();
        }

        // Turn off WiFi radio
        WiFi.mode(WIFI_MODE_NULL);
        while (WiFi.getMode() != WIFI_MODE_NULL) {
            delay(10);
        }

        // Final pause
        pauseTime = millis() + 500UL;
        while (millis() < pauseTime) {
            delay(10);
        }
    }

} // namespace Net


//  --- EOF --- //
