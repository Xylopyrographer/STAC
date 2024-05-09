
// STACGlobal.h

// ***** Global Variables *****

// objects we need to talk to the ATOM hardware
Button dButt( DIS_BUTTON, DB_TIME, 1, true );   // make a Button object
LiteLED theDisplay( LED_TYPE, 0 );              // create a LiteLED strip object
WiFiClient stClient;		                    // initiate the WiFi library and create a WiFi client object
Preferences stcPrefs;                           // NVS space holding the normal operating parameters across restarts

unsigned long nextPollTime = 0UL;               // the switch is next polled at this many ms

// get the techie IDE info stuff
String ardesp32Ver =
    String( ESP_ARDUINO_VERSION_MAJOR ) + "." +
    String( ESP_ARDUINO_VERSION_MINOR ) + "." +
    String( ESP_ARDUINO_VERSION_PATCH );        // arduino-esp32 core version
String espidfsdk = ESP.getSdkVersion();         // ESP-IDF SDK version

// declare the normal operating mode stuctures & initialize them
stac_ops_t stacOps = { "NO_MODEL", 1, 6, "NO_BANK", 8, 8, false, true, 1, 300UL };                  // operating paramaters
tState_t tallyStat = { false, true, true, false, 0, 0, "NO_UID", "NO_PW", "NO_INIT", "NO_TALLY" };  // tally status
wifi_info_t wifiStat = { "NO_STAC", "NO_SSID", "NO_PW", ( 0, 0, 0, 0 ), 80, false, false };         // WiFi connection status

// HTML pages
#include "./STACsuPages.h"      // HTML config/setup pages
#include "./STACotaPages.h"     // HTML OTA pages

// ***** End Global Variables *****
