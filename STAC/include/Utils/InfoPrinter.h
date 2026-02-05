#ifndef STAC_INFO_PRINTER_H
#define STAC_INFO_PRINTER_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_arduino_version.h>
#include <esp_mac.h>
#include "../Config/Types.h"
#include "../Config/Constants.h"
#include "../build_info.h"
#include "../Device_Config.h"

namespace Utils {

    /**
     * @brief Utility class for printing formatted information to Serial
     *
     * Provides methods for outputting STAC status and configuration information
     * to the serial monitor in a formatted, human-readable way.
     */
    class InfoPrinter {
      public:
        /**
         * @brief Print the startup header with version information
         * @param stacID The STAC identifier (MAC-based ID)
         */
        static void printHeader( const String &stacID ) {
            // Ensure WiFi is initialized to get MAC address
            WiFi.mode( WIFI_STA );

            Serial.print( "\r\n\r\n" );
            Serial.println( "==========================================" );
            Serial.println( "                STAC" );
            Serial.println( "     A Roland Smart Tally Client" );
            Serial.println( "           by: Team STAC" );
            Serial.println( "    github.com/Xylopyrographer/STAC" );
            Serial.println();
            Serial.print( "    Device: " );
            Serial.println( STAC_BOARD_NAME );
            Serial.print( "    SSID: " );
            Serial.println( stacID );
            Serial.println( "    Access: http://stac.local" );
            Serial.println( "        or: http://192.168.6.14" );

            // Get MAC address directly from eFuse (WiFi not initialized yet)
            uint8_t mac[ 6 ];
            esp_efuse_mac_get_default( mac );
            char macStr[ 18 ];
            snprintf( macStr, sizeof( macStr ), "%02X:%02X:%02X:%02X:%02X:%02X",
                      mac[ 0 ], mac[ 1 ], mac[ 2 ], mac[ 3 ], mac[ 4 ], mac[ 5 ] );
            Serial.print( "    MAC: " );
            Serial.println( macStr );

            Serial.print( "    Version: " );
            Serial.println( BUILD_FULL_VERSION );
            Serial.println( "  --------------------------------------" );
            Serial.flush();
        }

        /**
         * @brief Print WiFi connection success message
         */
        static void printWiFiConnected() {
            Serial.print( "    WiFi connected. IP: " );
            Serial.println( WiFi.localIP() );
            Serial.println( "==========================================" );
            Serial.flush();
        }

        /**
         * @brief Print switch model change notification
         */
        static void printModelChange() {
            Serial.println( " ***       Switch Model changed       ***" );
            Serial.println( " ***  User run-time parameters reset  ***" );
            Serial.println( "      ------------------------------" );
            Serial.flush();
        }

        /**
         * @brief Print configuration complete message
         */
        static void printConfigDone() {
            Serial.println( "   ***** Configuration complete *****" );
            Serial.println( "==========================================" );
            Serial.flush();
        }

        /**
         * @brief Print factory reset notification
         */
        static void printReset() {
            Serial.println( "   ***** Performing factory reset *****" );
            Serial.println( "==========================================\r\n\r\n" );
            Serial.flush();
        }

        /**
         * @brief Print new preferences layout notification
         */
        static void printNewPrefs() {
            Serial.println( "    ****** New preferences layout ******" );
            Serial.println( "    ****** Configuration required ******" );
            Serial.flush();
        }

        /**
         * @brief Print complete configuration footer with all settings
         * @param ops Current operations configuration
         * @param switchIP IP address of the Roland switch
         * @param switchPort Port number of the Roland switch
         * @param networkSSID WiFi network SSID
         */
        static void printFooter( const StacOperations& ops,
                                 const IPAddress& switchIP,
                                 uint16_t switchPort,
                                 const String &networkSSID ) {
            Serial.print( "    WiFi Network SSID: " );
            Serial.println( networkSSID );
            Serial.print( "    Switch IP: " );
            Serial.println( switchIP.toString() );
            Serial.print( "    Switch Port #: " );
            Serial.println( switchPort );
            Serial.println( "  --------------------------------------" );
            Serial.print( "    Configured for Model: " );
            Serial.println( ops.switchModel );
            Serial.print( "    Active Tally Channel: " );

            if ( ops.isV60HD() ) {
                Serial.println( ops.tallyChannel );
                Serial.print( "    Max Tally Channel: " );
                Serial.println( ops.maxChannelCount );
            }
            else {
                // V-160HD
                if ( ops.tallyChannel > 8 ) {
                    Serial.print( "SDI " );
                    Serial.println( ops.tallyChannel - 8 );
                }
                else {
                    Serial.print( "HDMI " );
                    Serial.println( ops.tallyChannel );
                }
                Serial.print( "    Max HDMI Tally Channel: " );
                Serial.println( ops.maxHDMIChannel );
                Serial.print( "    Max SDI Tally Channel: " );
                Serial.println( ops.maxSDIChannel );
            }

            Serial.print( "    Tally Mode: " );
            Serial.println( ops.cameraOperatorMode ? "Camera Operator" : "Talent" );
            Serial.print( "    Auto start: " );
            Serial.println( ops.autoStartEnabled ? "Enabled" : "Disabled" );
            Serial.print( "    Brightness Level: " );
            Serial.println( ops.displayBrightnessLevel );
            Serial.print( "    Polling Interval: " );
            Serial.print( ops.statusPollInterval );
            Serial.println( " ms" );
            Serial.println( "    Operating mode: Normal" );
            Serial.println( "  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=" );
            Serial.print( "    Git: " );
            Serial.print( BUILD_GIT_COMMIT );
            Serial.print( " @ " );
            Serial.println( BUILD_DATE );
            Serial.print( "    Core: " );
            Serial.print( ESP_ARDUINO_VERSION_MAJOR );
            Serial.print( "." );
            Serial.print( ESP_ARDUINO_VERSION_MINOR );
            Serial.print( "." );
            Serial.println( ESP_ARDUINO_VERSION_PATCH );
            Serial.print( "    SDK: " );
            Serial.println( ESP.getSdkVersion() );
            Serial.println( "==========================================" );
            Serial.flush();
        }

        /**
         * @brief Print peripheral mode status
         * @param cameraMode True for camera operator mode, false for talent mode
         * @param brightnessLevel Display brightness level (1-6 for 5x5, 1-8 for 8x8)
         * @param isConfigured True if device is provisioned, false otherwise
         */
        static void printPeripheral( bool cameraMode, uint8_t brightnessLevel, bool isConfigured = true ) {
            if ( isConfigured ) {
                Serial.println( ">>>> OPERATING IN PERIPHERAL MODE <<<<" );
            }
            else {
                Serial.println( "    >>> DEVICE NOT CONFIGURED <<<" );
                Serial.println( "     Operating in Peripheral Mode" );
                Serial.println( "    Receiving tally via GROVE port" );
            }
            Serial.print( "    Tally Mode: " );
            Serial.println( cameraMode ? "Camera Operator" : "Talent" );
            Serial.print( "    Brightness Level: " );
            Serial.println( brightnessLevel );
            Serial.println( "=======================================" );
            Serial.flush();
        }

        /**
         * @brief Print OTA update mode notification
         */
        static void printOTA() {
            Serial.println( "    ***** Updating STAC firmware *****" );
            Serial.println( "    Connect to the STAC SSID WiFi AP," );
            Serial.println( "    then browse to http://update.local" );
            Serial.println( "===========================================" );
            Serial.flush();
        }

        /**
         * @brief Print OTA update result
         * @param success True if update succeeded
         * @param fileName Name of the firmware file
         * @param bytesWritten Number of bytes written (if successful)
         * @param status Status message
         */
        static void printOTAResult( bool success,
                                    const String &fileName,
                                    size_t bytesWritten,
                                    const String &status ) {
            if ( success ) {
                Serial.println( "  ******* Firmware update done *******" );
                Serial.print( " File: " );
                Serial.println( fileName );
                Serial.print( " Bytes written: " );
                Serial.println( bytesWritten );
                Serial.print( " Status: " );
                Serial.println( status );
            }
            else {
                Serial.println( " ******* FIRMWARE UPDATE FAILED *******" );
                Serial.print( " Tried with file: " );
                Serial.println( fileName );
                Serial.print( " Reason: " );
                Serial.println( status );
                Serial.println( " Ensure the correct \"STAC_XXXXX.BIN\"" );
                Serial.println( " file was selected" );
            }
            Serial.println( "              Restarting..." );
            Serial.println( "=========================================\r\n\r\n" );
            Serial.flush();
        }
    };

} // namespace Utils

#endif // STAC_INFO_PRINTER_H

//  --- EOF ---
