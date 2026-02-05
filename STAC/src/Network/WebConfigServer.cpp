/**
 * @file WebPortalServer.cpp
 * @brief Implementation of unified web portal server
 *
 * @author Rob Lake (@Xylopyrographer)
 * @date 2026-01-07
 */

#include "Network/WebConfigServer.h"
#include "Network/WebConfigPages.h"
#include "Storage/ConfigManager.h"
#include "Device_Config.h"
#include "build_info.h"
#include <esp_wifi.h>
#include <esp_mac.h>


namespace Net {

    WebConfigServer::WebConfigServer( const String &deviceID )
        : apIP( 192, 168, 6, 14 )
        , apGateway( 0, 0, 0, 0 )
        , apNetmask( 255, 255, 255, 0 )
        , server( nullptr )
        , dnsServer( nullptr )
        , deviceID( deviceID )
        , serverRunning( false )
        , operationComplete( false )
        , displayCallback( nullptr )
        , resetCheckCallback( nullptr )
        , preRestartCallback( nullptr )
        , lastDisplayUpdate( 0 ) {
        // Get MAC address and format it for display
        uint8_t mac[ 6 ];
        esp_efuse_mac_get_default( mac );
        char macStr[ 18 ];
        snprintf( macStr, sizeof( macStr ), "%02X:%02X:%02X:%02X:%02X:%02X",
                  mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );
        macAddress = String( macStr );
    }

    WebConfigServer::~WebConfigServer() {
        end();
    }

    bool WebConfigServer::begin() {
        log_i( "Starting web portal server" );

        // Turn off WiFi radio first
        WiFi.mode( WIFI_MODE_NULL );
        while ( WiFi.getMode() != WIFI_MODE_NULL ) {
            delay( 10 );
        }

        // Configure WiFi mode to AP
        WiFi.mode( WIFI_AP );
        while ( WiFi.getMode() != WIFI_MODE_AP ) {
            delay( 10 );
        }

        // Start Access Point
        bool apStarted = WiFi.softAP(
                             deviceID.c_str(),
                             AP_PASSWORD,
                             AP_CHANNEL,
                             AP_HIDE_SSID,
                             AP_MAX_CONNECTIONS
                         );

        if ( !apStarted ) {
            log_e( "Failed to start WiFi AP" );
            return false;
        }

        // Configure AP hostname and networking
        WiFi.softAPsetHostname( AP_HOSTNAME );
        WiFi.softAPConfig( apIP, apIP, apNetmask );

        // Start mDNS responder
        if ( !MDNS.begin( AP_HOSTNAME ) ) {
            log_e( "Failed to start mDNS responder" );
            return false;
        }

        log_i( "AP started - SSID: %s, IP: %s", deviceID.c_str(), apIP.toString().c_str() );

        // Start DNS server for local hostname resolution (NOT captive portal)
        // Only responds to stac.local queries, doesn't intercept all DNS
        dnsServer = new DNSServer();
        dnsServer->start( DNS_PORT, AP_HOSTNAME, apIP );
        log_i( "DNS server started for local hostname resolution" );

        // Create and configure web server
        server = new WebServer( 80 );
        registerEndpoints();
        server->begin();
        server->enableDelay( false ); // Disable 1ms delay in handleClient()

        serverRunning = true;
        log_i( "Web portal server started on port 80" );

        return true;
    }

    WebConfigServer::PortalResult WebConfigServer::waitForCompletion() {
        if ( !serverRunning ) {
            log_e( "Server not running - call begin() first" );
            return result;
        }

        log_i( "Waiting for user action via web portal" );

        // Reset state
        operationComplete = false;
        result.type = PortalResultType::NONE;
        lastDisplayUpdate = millis();

        // Handle client requests until operation completes
        while ( !operationComplete ) {
            dnsServer->processNextRequest();  // Process DNS requests for captive portal
            server->handleClient();

            // Check for reset button via callback
            if ( resetCheckCallback && resetCheckCallback() ) {
                log_i( "Reset requested during portal session - restarting" );
                if ( preRestartCallback ) {
                    preRestartCallback();  // Call cleanup (e.g., turn off backlight)
                }
                ESP.restart();
            }

            // Update pulsing display via callback
            unsigned long now = millis();
            if ( displayCallback && ( now - lastDisplayUpdate >= DISPLAY_UPDATE_INTERVAL ) ) {
                lastDisplayUpdate = now;
                displayCallback();
            }

            yield();
        }

        log_i( "Portal operation complete: type=%d", static_cast<int>( result.type ) );
        return result;
    }

    void WebConfigServer::end() {
        if ( !serverRunning ) {
            return;
        }

        log_i( "Stopping web portal server" );

        // Stop DNS server
        if ( dnsServer ) {
            dnsServer->stop();
            delete dnsServer;
            dnsServer = nullptr;
        }

        // Stop server
        if ( server ) {
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
        log_i( "Web portal server stopped" );
    }

    void WebConfigServer::registerEndpoints() {
        // GET / - Serve tabbed portal index page
        server->on( "/", HTTP_GET, [ this ]() {
            handleRoot();
        } );

        // Captive portal detection endpoints
        // Apple/iOS/macOS
        server->on( "/hotspot-detect.html", HTTP_GET, [ this ]() {
            handleRoot();
        } );

        // Catch-all for captive portal - redirect to root
        server->onNotFound( [ this ]() {
            // If it looks like a captive portal check, redirect to root
            String host = server->hostHeader();
            if ( host.indexOf( apIP.toString() ) < 0 ) {
                server->sendHeader( "Location", "http://" + apIP.toString(), true );
                server->send( 302, "text/plain", "" );
            }
            else {
                // Regular 404
                server->send( 404, "text/html", WebConfig::NOT_FOUND );
            }
        } );

        // POST /config - Process configuration submission
        server->on( "/config", HTTP_POST, [ this ]() {
            handleConfigSubmit();
        } );

        // POST /factory-reset - Handle factory reset request
        server->on( "/factory-reset", HTTP_POST, [ this ]() {
            handleFactoryReset();
        } );

        // POST /update - Handle firmware upload and flashing
        server->on( "/update", HTTP_POST,
        [ this ]() {
            // This lambda runs after upload completes
            handleUpdateComplete();
        },
        [ this ]() {
            // This lambda handles the actual file upload chunks
            handleFileUpload();
        }
                  );
    }

    void WebConfigServer::handleRoot() {
        String page = buildIndexPage();
        server->send( 200, "text/html", page );
        log_v( "Served portal index page" );
    }

    void WebConfigServer::handleConfigSubmit() {
        log_i( "Processing configuration submission" );

        // Send confirmation page immediately
        server->send( 200, "text/html", WebConfig::CONFIG_RECEIVED );

        // Ensure response is sent before continuing
        delay( 100 );

        // Extract form data
        String model = server->arg( "stModel" );
        result.configData.switchModel = model;
        result.configData.wifiSSID = server->arg( "SSID" );
        result.configData.wifiPassword = server->arg( "pwd" );
        result.configData.switchIPString = server->arg( "stIP" );
        result.configData.switchPort = static_cast<uint16_t>( server->arg( "stPort" ).toInt() );
        result.configData.pollInterval = static_cast<unsigned long>( server->arg( "pollTime" ).toInt() );

        if ( model == "V-60HD" ) {
            result.configData.maxChannel = static_cast<uint8_t>( server->arg( "stChan" ).toInt() );
            // Set defaults for unused V-160HD fields
            result.configData.lanUserID = "";
            result.configData.lanPassword = "";
            result.configData.maxHDMIChannel = 0;
            result.configData.maxSDIChannel = 0;

            log_i( "  V-60HD Config:" );
            log_i( "    WiFi SSID: %s", result.configData.wifiSSID.c_str() );
            log_i( "    Switch IP: %s:%d", result.configData.switchIPString.c_str(), result.configData.switchPort );
            log_i( "    Max Channel: %d", result.configData.maxChannel );
            log_i( "    Poll Interval: %lu ms", result.configData.pollInterval );
        }
        else if ( model == "V-160HD" ) {
            result.configData.lanUserID = server->arg( "stnetUser" );
            result.configData.lanPassword = server->arg( "stnetPW" );
            result.configData.maxHDMIChannel = static_cast<uint8_t>( server->arg( "stChanHDMI" ).toInt() );
            result.configData.maxSDIChannel = static_cast<uint8_t>( server->arg( "stChanSDI" ).toInt() );
            // Set default for unused V-60HD field
            result.configData.maxChannel = 0;

            log_i( "  V-160HD Config:" );
            log_i( "    WiFi SSID: %s", result.configData.wifiSSID.c_str() );
            log_i( "    Switch IP: %s:%d", result.configData.switchIPString.c_str(), result.configData.switchPort );
            log_i( "    LAN User: %s", result.configData.lanUserID.c_str() );
            log_i( "    Max HDMI: %d, Max SDI: %d", result.configData.maxHDMIChannel, result.configData.maxSDIChannel );
            log_i( "    Poll Interval: %lu ms", result.configData.pollInterval );
        }

        result.type = PortalResultType::CONFIG_RECEIVED;
        operationComplete = true;
    }

    void WebConfigServer::handleFactoryReset() {
        log_i( "Processing factory reset request from web portal" );

        // Send confirmation page immediately
        server->send( 200, "text/html", WebConfig::FACTORY_RESET_RECEIVED );

        // Ensure response is sent before continuing
        delay( 100 );

        log_i( "Factory reset confirmed via web portal" );

        result.type = PortalResultType::FACTORY_RESET;
        operationComplete = true;
    }

    void WebConfigServer::handleUpdateComplete() {
        // Build and send result page
        String resultPage = buildOTAResultPage();
        server->send( 200, "text/html", resultPage );

        log_i( "Update result page sent to client" );
        log_i( "  Filename: %s", result.otaResult.filename.c_str() );
        log_i( "  Bytes: %d", result.otaResult.bytesWritten );
        log_i( "  Status: %s", result.otaResult.statusMessage.c_str() );

        // Set result type based on success
        result.type = result.otaResult.success ? PortalResultType::OTA_SUCCESS : PortalResultType::OTA_FAILED;
        operationComplete = true;
    }

    void WebConfigServer::handleFileUpload() {
        HTTPUpload& upload = server->upload();

        if ( upload.status == UPLOAD_FILE_START ) {
            // Start of upload
            result.otaResult.filename = upload.filename;
            result.otaResult.success = false;
            result.otaResult.bytesWritten = 0;
            result.otaResult.statusMessage = "Starting upload...";

            log_i( "Firmware upload started: %s", upload.filename.c_str() );
            log_i( "Expected file size: %d bytes", upload.totalSize );

            // Begin OTA update with UPDATE_SIZE_UNKNOWN to auto-detect
            if ( !Update.begin( UPDATE_SIZE_UNKNOWN ) ) {
                result.otaResult.statusMessage = "Failed to begin OTA update";
                result.otaResult.success = false;
                log_e( "Update.begin() failed" );
                Update.printError( Serial );
            }
            else {
                log_i( "OTA partition ready for writing (size: %d bytes)", Update.size() );
            }
        }
        else if ( upload.status == UPLOAD_FILE_WRITE ) {
            // Write chunk to flash
            size_t written = Update.write( upload.buf, upload.currentSize );
            if ( written != upload.currentSize ) {
                log_e( "Write error: expected %d bytes, wrote %d bytes",
                       upload.currentSize, written );
            }
            result.otaResult.bytesWritten += written;

            // Log progress periodically
            if ( result.otaResult.bytesWritten % 32768 == 0 ) {
                log_v( "Written: %d bytes", result.otaResult.bytesWritten );
            }
        }
        else if ( upload.status == UPLOAD_FILE_END ) {
            // Upload complete
            if ( Update.end( true ) ) {
                // Success!
                result.otaResult.success = true;
                result.otaResult.statusMessage = "Firmware updated successfully";
                log_i( "OTA update successful! Total bytes: %d", result.otaResult.bytesWritten );
            }
            else {
                // Failed
                result.otaResult.success = false;
                result.otaResult.statusMessage = "Update failed: " + String( Update.errorString() );
                log_e( "OTA update failed: %s", Update.errorString() );
                Update.printError( Serial );
            }
        }
        else if ( upload.status == UPLOAD_FILE_ABORTED ) {
            // Upload was cancelled
            Update.abort();
            result.otaResult.success = false;
            result.otaResult.statusMessage = "Upload cancelled by user";
            log_w( "OTA upload aborted" );
        }
    }


    String WebConfigServer::buildIndexPage() const {
        // Get firmware version info
        String fwVersion = BUILD_FULL_VERSION;  // "3.0.0-RC.11 (b826c4)"
        String gitInfo = String( BUILD_GIT_COMMIT ) + " @ " + String( BUILD_DATE );
        String coreVersion = String( ESP_ARDUINO_VERSION_MAJOR ) + "." +
                             String( ESP_ARDUINO_VERSION_MINOR ) + "." +
                             String( ESP_ARDUINO_VERSION_PATCH );
        String sdkVersion = esp_get_idf_version();

        // Load configuration from NVS
        Storage::ConfigManager configMgr;
        configMgr.begin();

        bool isConfigured = configMgr.isProvisioned();

        // Build complete page
        String page;
        page.reserve( 8192 ); // Pre-allocate to reduce fragmentation

        page += WebConfig::PAGE_HEAD;
        page += WebConfig::DEVICE_INFO_OPEN;
        page += "<p><strong>Device:</strong> " STAC_BOARD_NAME "</p>";
        page += "<p><strong>SSID:</strong> " + deviceID + "</p>";
        page += "<p><strong>MAC:</strong> " + macAddress + "</p>";
        page += "<p><strong>Version:</strong> " + fwVersion + "</p>";
        page += WebConfig::DEVICE_INFO_CLOSE;

        // Hidden info for modal (accessed by JavaScript)
        page += "<div id='geek-info' style='display:none;'><pre style='font-family: monospace; font-size: 12px; line-height: 1.4;'>";
        page += "==========================================";
        page += "\n                  STAC";
        page += "\n        A Roland Smart Tally Client";
        page += "\n             by: Team STAC";
        page += "\n      github.com/Xylopyrographer/STAC";
        page += "\n";
        page += "\n    Device: " STAC_BOARD_NAME;
        page += "\n    SSID: " + deviceID;
        page += "\n    Access: http://stac.local";
        page += "\n        or: http://" + apIP.toString();
        page += "\n    MAC: " + macAddress;
        page += "\n    Version: " + fwVersion;

        // Check if device is in peripheral mode (regardless of configuration state)
        bool isPeripheralMode = configMgr.loadPModeEnabled();

        if ( !isConfigured && !isPeripheralMode ) {
            // Not configured and not in peripheral mode
            page += "\n  --------------------------------------";
            page += "\n     >>> DEVICE NOT CONFIGURED <<<";
        }
        else if ( isPeripheralMode && !isConfigured ) {
            // Peripheral mode but not configured for normal mode
            // Load peripheral-specific settings
            bool pmCameraMode;
            uint8_t pmBrightness;
            configMgr.loadPeripheralSettings( pmCameraMode, pmBrightness );

            page += "\n  --------------------------------------";
            page += "\n     >>> DEVICE NOT CONFIGURED <<<";
            page += "\n     Operating in Peripheral Mode";
            page += "\n    Receiving tally via GROVE port";
            page += "\n  --------------------------------------";
            page += "\n    Tally Mode: " + String( pmCameraMode ? "Camera Operator" : "Talent" );
            page += "\n    Brightness Level: " + String( pmBrightness );
        }
        else {
            // Load WiFi config
            String wifiSSID, wifiPassword;
            configMgr.loadWiFiCredentials( wifiSSID, wifiPassword );

            // Load switch config
            String switchModel;
            IPAddress switchIPAddr;
            uint16_t switchPort;
            String username, password;
            configMgr.loadSwitchConfig( switchModel, switchIPAddr, switchPort, username, password );

            // Load protocol-specific operations
            StacOperations ops;
            if ( switchModel == "V-60HD" ) {
                configMgr.loadV60HDConfig( ops );
            }
            else if ( switchModel == "V-160HD" ) {
                configMgr.loadV160HDConfig( ops );
            }

            // Determine tally mode
            String tallyMode = ops.cameraOperatorMode ? "Camera Operator" : "Talent";

            // Determine operating mode
            bool isPeripheralMode = configMgr.loadPModeEnabled();
            String operatingMode = isPeripheralMode ? "Peripheral" : "Normal";

            // For peripheral mode, load peripheral-specific settings
            if ( isPeripheralMode ) {
                bool pmCameraMode;
                uint8_t pmBrightness;
                configMgr.loadPeripheralSettings( pmCameraMode, pmBrightness );
                ops.cameraOperatorMode = pmCameraMode;
                ops.displayBrightnessLevel = pmBrightness;
                tallyMode = pmCameraMode ? "Camera Operator" : "Talent";
            }

            page += "\n  --------------------------------------";
            page += "\n    WiFi Network SSID: " + wifiSSID;
            page += "\n    Switch IP: " + switchIPAddr.toString();
            page += "\n    Switch Port #: " + String( switchPort );
            page += "\n  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
            page += "\n    Configured for Model: " + switchModel;

            // Display channel info based on model
            if ( ops.isV60HD() ) {
                page += "\n    Active Tally Channel: " + String( ops.tallyChannel );
                page += "\n    Max Tally Channel: " + String( ops.maxChannelCount );
            }
            else if ( ops.isV160HD() ) {
                // V-160HD shows HDMI/SDI format
                if ( ops.tallyChannel > 8 ) {
                    page += "\n    Active Tally Channel: SDI " + String( ops.tallyChannel - 8 );
                }
                else {
                    page += "\n    Active Tally Channel: HDMI " + String( ops.tallyChannel );
                }
                page += "\n    Max HDMI Tally Channel: " + String( ops.maxHDMIChannel );
                page += "\n    Max SDI Tally Channel: " + String( ops.maxSDIChannel );
            }

            page += "\n    Tally Mode: " + tallyMode;
            page += "\n    Auto start: " + String( ops.autoStartEnabled ? "Enabled" : "Disabled" );
            page += "\n    Brightness Level: " + String( ops.displayBrightnessLevel );
            page += "\n    Polling Interval: " + String( ops.statusPollInterval ) + " ms";
            page += "\n    Operating mode: " + operatingMode;
        }

        page += "\n  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=";
        page += "\n    Git: " + gitInfo;
        page += "\n    Core: " + coreVersion;
        page += "\n    SDK: " + sdkVersion;
        page += "\n==========================================";
        page += "</pre></div>";

        page += WebConfig::LANDING_PAGE;
        page += WebConfig::TAB_BUTTONS;
        page += WebConfig::TAB_SETUP;
        page += WebConfig::TAB_MAINTENANCE;
        page += WebConfig::PAGE_SCRIPT;
        page += WebConfig::PAGE_FOOTER;

        return page;
    }

    String WebConfigServer::buildOTAResultPage() const {
        String page;
        page.reserve( 1024 );

        page += WebConfig::OTA_PAGE_OPEN;

        if ( result.otaResult.success ) {
            page += "<h1 class=\"success\">&#10004; Update Successful</h1>";
            page += "<p>Firmware file: <span class=\"filename\">" + result.otaResult.filename + "</span></p>";
            page += "<p>Bytes written: " + String( result.otaResult.bytesWritten ) + "</p>";
            page += "<p><strong>STAC is restarting...</strong></p>";
            page += "<p>Please wait for the device to reboot.</p>";
        }
        else {
            page += "<h1 class=\"error\">&#10008; Update Failed</h1>";
            page += "<p class=\"error\">" + result.otaResult.statusMessage + "</p>";
            page += "<p>Firmware file: <span class=\"filename\">" + result.otaResult.filename + "</span></p>";
            page += "<p>Bytes written: " + String( result.otaResult.bytesWritten ) + "</p>";
            page += "<p><a href=\"/\">Return to Portal</a></p>";
        }

        page += WebConfig::OTA_PAGE_CLOSE;

        return page;
    }

    void WebConfigServer::shutdownSequence() {
        // Pause to allow HTTP response to be sent
        unsigned long pauseTime = millis() + 500UL;
        while ( millis() < pauseTime ) {
            yield();
        }

        // Turn off WiFi radio
        WiFi.mode( WIFI_MODE_NULL );
        while ( WiFi.getMode() != WIFI_MODE_NULL ) {
            delay( 10 );
        }

        // Final pause
        pauseTime = millis() + 500UL;
        while ( millis() < pauseTime ) {
            delay( 10 );
        }
    }

} // namespace Net


//  --- EOF --- //
