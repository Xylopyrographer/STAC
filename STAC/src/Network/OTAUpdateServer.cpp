/**
 * @file OTAUpdateServer.cpp
 * @brief Implementation of OTA firmware update server
 * 
 * @author Rob Lake (@Xylopyrographer)
 * @date 2025-11-15
 */

#include "Network/OTAUpdateServer.h"
#include "Network/OTAUpdatePages.h"
#include <esp_wifi.h>


namespace Net {

    OTAUpdateServer::OTAUpdateServer(const String& deviceID)
        : apIP(192, 168, 6, 14)
        , apGateway(0, 0, 0, 0)
        , apNetmask(255, 255, 255, 0)
        , server(nullptr)
        , deviceID(deviceID)
        , updateComplete(false)
        , serverRunning(false)
        , displayUpdateCallback(nullptr)
        , lastDisplayUpdate(0)
    {
    }

    OTAUpdateServer::~OTAUpdateServer() {
        end();
    }

    bool OTAUpdateServer::begin() {
        log_i("Starting OTA update server");

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

        log_i("AP started - SSID: %s, IP: %s, mDNS: %s.local", 
              deviceID.c_str(), apIP.toString().c_str(), AP_HOSTNAME);

        // Create and configure web server
        server = new WebServer(80);
        registerEndpoints();
        server->begin();
        server->enableDelay(false); // Disable 1ms delay in handleClient()

        serverRunning = true;
        log_i("OTA update server started on port 80");

        return true;
    }

    OTAUpdateResult OTAUpdateServer::waitForUpdate() {
        if (!serverRunning) {
            log_e("Server not running - call begin() first");
            return updateResult;
        }

        log_i("Waiting for firmware upload from web client");

        // Handle client requests until update completes
        while (!updateComplete) {
            server->handleClient();
            
            // Check for reset button via callback
            if (resetCheckCallback && resetCheckCallback()) {
                log_i("Reset requested during OTA wait - restarting");
                if (preRestartCallback) {
                    preRestartCallback();  // Call cleanup (e.g., turn off backlight)
                }
                ESP.restart();
            }
            
            // Call display update callback if set and interval elapsed
            if (displayUpdateCallback && (millis() - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL)) {
                displayUpdateCallback();
                lastDisplayUpdate = millis();
            }
            
            yield();
        }

        log_i("Update complete: %s", updateResult.success ? "SUCCESS" : "FAILED");

        // If update succeeded, we'll restart (this function won't return)
        if (updateResult.success) {
            log_i("Restarting ESP32 to apply new firmware...");
            shutdownSequence();
            ESP.restart();
            // Never reaches here
        }

        // Only returns if update failed
        return updateResult;
    }

    void OTAUpdateServer::end() {
        if (!serverRunning) {
            return;
        }

        log_i("Stopping OTA update server");

        // Stop server
        if (server) {
            server->stop();
            server->close();
            delete server;
            server = nullptr;
        }

        // Stop mDNS
        MDNS.end();

        // Shutdown WiFi
        WiFi.mode(WIFI_MODE_NULL);
        while (WiFi.getMode() != WIFI_MODE_NULL) {
            delay(10);
        }

        serverRunning = false;
        log_i("OTA update server stopped");
    }

    void OTAUpdateServer::setDisplayUpdateCallback(std::function<void()> callback) {
        displayUpdateCallback = callback;
    }

    void OTAUpdateServer::setResetCheckCallback(std::function<bool()> callback) {
        resetCheckCallback = callback;
    }

    void OTAUpdateServer::setPreRestartCallback(std::function<void()> callback) {
        preRestartCallback = callback;
    }

    void OTAUpdateServer::registerEndpoints() {
        // GET / - Serve firmware upload page
        server->on("/", HTTP_GET, [this]() {
            handleRoot();
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

    void OTAUpdateServer::handleRoot() {
        server->send(200, "text/html", OTAPages::INDEX);
        log_v("Served OTA index page");
    }

    void OTAUpdateServer::handleUpdateComplete() {
        // Build and send result page
        String resultPage = buildResultPage();
        server->send(200, "text/html", resultPage);

        log_i("Update result page sent to client");
        log_i("  Filename: %s", updateResult.filename.c_str());
        log_i("  Bytes: %d", updateResult.bytesWritten);
        log_i("  Status: %s", updateResult.statusMessage.c_str());

        // Mark update as complete (will trigger restart or return from waitForUpdate)
        updateComplete = true;
    }

    void OTAUpdateServer::handleFileUpload() {
        HTTPUpload& upload = server->upload();

        if (upload.status == UPLOAD_FILE_START) {
            // Start of upload
            updateResult.filename = upload.filename;
            updateResult.success = false;
            updateResult.bytesWritten = 0;
            updateResult.statusMessage = "Starting upload...";

            log_i("Firmware upload started: %s", upload.filename.c_str());
            log_i("Expected file size: %d bytes", upload.totalSize);

            // Begin OTA update with UPDATE_SIZE_UNKNOWN to auto-detect
            // This allows the Update library to use the entire OTA partition
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                updateResult.statusMessage = "Failed to begin OTA update";
                updateResult.success = false;
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
            updateResult.bytesWritten += written;
        }
        else if (upload.status == UPLOAD_FILE_END) {
            // Upload complete
            if (Update.end(true)) {
                // Success!
                updateResult.success = true;
                updateResult.bytesWritten = upload.totalSize;
                updateResult.statusMessage = "Update successful";
                log_i("Firmware upload complete: %d bytes", upload.totalSize);
            } else {
                // Failure
                updateResult.success = false;
                updateResult.statusMessage = Update.errorString();
                log_e("Update.end() failed: %s", updateResult.statusMessage.c_str());
                Update.printError(Serial);
            }
        }

        // Check for errors during any phase
        if (Update.hasError()) {
            updateResult.success = false;
            updateResult.statusMessage = Update.errorString();
            log_e("Update error: %s", updateResult.statusMessage.c_str());
            Update.abort();
        }
    }

    void OTAUpdateServer::handleNotFound() {
        server->send(404, "text/html", OTAPages::NOT_FOUND);
        log_v("404 served for: %s", server->uri().c_str());
    }

    String OTAUpdateServer::buildResultPage() const {
        String page = String(OTAPages::PAGE_OPEN);

        if (updateResult.success) {
            // Success page
            page += String(OTAPages::SUCCESS);
            page += updateResult.filename;
            page += "<br><br>Bytes written: ";
            page += String(updateResult.bytesWritten);
            page += "<br>Status: ";
            page += updateResult.statusMessage;
        } else {
            // Failure page
            page += String(OTAPages::FAILURE);
            page += updateResult.statusMessage;
            page += "<br><br>Ensure the correct<br>\"<strong>STAC_xxxx.bin</strong>\"<br>file was selected.<br>";
        }

        page += String(OTAPages::PAGE_CLOSE);

        return page;
    }

    void OTAUpdateServer::shutdownSequence() {
        // Stop server
        if (server) {
            server->stop();
            server->close();
        }

        // Stop mDNS
        MDNS.end();

        // Pause to allow HTTP response to be sent
        unsigned long pauseTime = millis() + 500UL;
        while (millis() < pauseTime) {
            delay(10);
        }

        // Turn off WiFi
        WiFi.mode(WIFI_MODE_NULL);
        while (WiFi.getMode() != WIFI_MODE_NULL) {
            delay(10);
        }

        // Final pause before restart
        pauseTime = millis() + 500UL;
        while (millis() < pauseTime) {
            delay(10);
        }
    }

} // namespace Net


//  --- EOF --- //
