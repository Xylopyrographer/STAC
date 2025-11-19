/**
 * @file WebConfigServer.cpp
 * @brief Implementation of web-based configuration server
 * 
 * @author Rob Lake (@Xylopyrographer)
 * @date 2025-11-15
 */

#include "Network/WebConfigServer.h"
#include "Network/WebConfigPages.h"
#include <esp_wifi.h>
#include <esp_mac.h>


namespace Net {

    WebConfigServer::WebConfigServer(const String& deviceID)
        : apIP(192, 168, 6, 14)
        , apGateway(0, 0, 0, 0)
        , apNetmask(255, 255, 255, 0)
        , server(nullptr)
        , deviceID(deviceID)
        , configReceived(false)
        , serverRunning(false)
        , displayCallback(nullptr)
    {
        // Get MAC address and format it for display
        uint8_t mac[6];
        esp_efuse_mac_get_default(mac);
        char macStr[18];
        snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        macAddress = String(macStr);
    }

    WebConfigServer::~WebConfigServer() {
        end();
    }

    bool WebConfigServer::begin() {
        log_i("Starting web configuration server");

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
        log_i("Web server started on port 80");

        return true;
    }

    ProvisioningData WebConfigServer::waitForConfiguration() {
        if (!serverRunning) {
            log_e("Server not running - call begin() first");
            return ProvisioningData();
        }

        log_i("Waiting for configuration from web client");

        // Pulsing animation state
        unsigned long lastPulse = 0;
        const unsigned long pulseInterval = 1000; // 1 second pulse

        // Handle client requests until configuration is received
        while (!configReceived) {
            server->handleClient();
            
            // Update pulsing display via callback
            unsigned long now = millis();
            if (displayCallback && (now - lastPulse >= pulseInterval)) {
                lastPulse = now;
                displayCallback();
            }
            
            yield();
        }

        log_i("Configuration received from web client");
        return configData;
    }

    void WebConfigServer::end() {
        if (!serverRunning) {
            return;
        }

        log_i("Stopping web configuration server");

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
        log_i("Web configuration server stopped");
    }

    void WebConfigServer::registerEndpoints() {
        // GET / - Serve model selection page
        server->on("/", HTTP_GET, [this]() {
            handleRoot();
        });

        // POST / - Process model selection
        server->on("/", HTTP_POST, [this]() {
            handleModelSelection();
        });

        // POST /parse - Process V-60HD configuration
        server->on("/parse", HTTP_POST, [this]() {
            handleV60HDConfig();
        });

        // POST /parse2 - Process V-160HD configuration
        server->on("/parse2", HTTP_POST, [this]() {
            handleV160HDConfig();
        });

        // 404 handler
        server->onNotFound([this]() {
            handleNotFound();
        });
    }

    void WebConfigServer::handleRoot() {
        String indexPage = buildIndexPage();
        server->send(200, "text/html", indexPage);
        log_v("Served index page");
    }

    void WebConfigServer::handleModelSelection() {
        String model = server->arg("stModel");
        log_i("Model selected: %s", model.c_str());

        if (model == "V-60HD") {
            server->send(200, "text/html", WebConfig::CONFIG_V60HD);
        } else if (model == "V-160HD") {
            server->send(200, "text/html", WebConfig::CONFIG_V160HD);
        } else {
            log_e("Invalid model selection: %s", model.c_str());
            server->send(400, "text/plain", "Invalid model selection");
        }
    }

    void WebConfigServer::handleV60HDConfig() {
        log_i("Processing V-60HD configuration");

        // Send confirmation page immediately
        server->send(200, "text/html", WebConfig::RECEIVED);
        
        // Ensure response is sent before continuing
        delay(100);

        // Extract form data
        configData.switchModel = "V-60HD";
        configData.wifiSSID = server->arg("SSID");
        configData.wifiPassword = server->arg("pwd");
        configData.switchIPString = server->arg("stIP");
        configData.switchPort = static_cast<uint16_t>(server->arg("stPort").toInt());
        configData.maxChannel = static_cast<uint8_t>(server->arg("stChan").toInt());
        configData.pollInterval = static_cast<unsigned long>(server->arg("pollTime").toInt());

        // Set defaults for unused V-160HD fields
        configData.lanUserID = "";
        configData.lanPassword = "";
        configData.maxHDMIChannel = 0;
        configData.maxSDIChannel = 0;

        log_i("  WiFi SSID: %s", configData.wifiSSID.c_str());
        log_i("  Switch IP: %s:%d", configData.switchIPString.c_str(), configData.switchPort);
        log_i("  Max Channel: %d", configData.maxChannel);
        log_i("  Poll Interval: %lu ms", configData.pollInterval);

        configReceived = true;
    }

    void WebConfigServer::handleV160HDConfig() {
        log_i("Processing V-160HD configuration");

        // Send confirmation page immediately
        server->send(200, "text/html", WebConfig::RECEIVED);
        
        // Ensure response is sent before continuing
        delay(100);

        // Extract form data
        configData.switchModel = "V-160HD";
        configData.wifiSSID = server->arg("SSID");
        configData.wifiPassword = server->arg("pwd");
        configData.switchIPString = server->arg("stIP");
        configData.switchPort = static_cast<uint16_t>(server->arg("stPort").toInt());
        configData.lanUserID = server->arg("stnetUser");
        configData.lanPassword = server->arg("stnetPW");
        configData.maxHDMIChannel = static_cast<uint8_t>(server->arg("stChanHDMI").toInt());
        configData.maxSDIChannel = static_cast<uint8_t>(server->arg("stChanSDI").toInt());
        configData.pollInterval = static_cast<unsigned long>(server->arg("pollTime").toInt());

        // Set default for unused V-60HD field
        configData.maxChannel = 0;

        log_i("  WiFi SSID: %s", configData.wifiSSID.c_str());
        log_i("  Switch IP: %s:%d", configData.switchIPString.c_str(), configData.switchPort);
        log_i("  LAN User: %s", configData.lanUserID.c_str());
        log_i("  Max HDMI: %d, Max SDI: %d", configData.maxHDMIChannel, configData.maxSDIChannel);
        log_i("  Poll Interval: %lu ms", configData.pollInterval);

        configReceived = true;
    }

    void WebConfigServer::handleNotFound() {
        server->send(404, "text/html", WebConfig::NOT_FOUND);
        log_v("404 served for: %s", server->uri().c_str());
    }

    String WebConfigServer::buildIndexPage() const {
        // Get firmware version info
        // TODO: Replace with actual version strings from build system
        String fwVersion = "3.0.0-dev";
        String coreVersion = String(ESP_ARDUINO_VERSION_MAJOR) + "." +
                             String(ESP_ARDUINO_VERSION_MINOR) + "." +
                             String(ESP_ARDUINO_VERSION_PATCH);
        String sdkVersion = esp_get_idf_version();

        // Build complete page
        String page = String(WebConfig::FORM_OPEN) +
                      "Unit: " + deviceID + "<br>" +
                      "MAC: " + macAddress + "<br><br>" +
                      "Version: " + fwVersion + "<br>" +
                      "Core: " + coreVersion + "<br>" +
                      "SDK: " + sdkVersion + "<br>" +
                      String(WebConfig::FORM_CLOSE);

        return page;
    }

    void WebConfigServer::shutdownSequence() {
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
