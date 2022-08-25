void fetchInfo( void ) {
// dumps a swack of system info to the serial monitor
// - must include
//      #include <Esp.h>
//      #include <esp_arduino_version.h>
// - the Serial system must be initialized prior to use.
    
    Serial.println("\r\n======================================\r\nHey there...");
    Serial.print("    Running sketch: "); Serial.println( __FILE__ );
    Serial.print("    arduino-esp32 core version: "); Serial.print( ESP_ARDUINO_VERSION_MAJOR ); Serial.print("."); Serial.print( ESP_ARDUINO_VERSION_MINOR ); Serial.print("."); Serial.println( ESP_ARDUINO_VERSION_PATCH );
    Serial.print("    ESP-IDF SDK version: "); Serial.println( ESP.getSdkVersion() );
    Serial.print("    Sketch size/free space: 0x"); Serial.print( ESP.getSketchSize(), HEX );
    Serial.print(" / 0x"); Serial.println( ESP.getFreeSketchSpace(), HEX );
    Serial.print("    Sketch space used (%): "); Serial.println( ( 100 / ( 1 + ESP.getFreeSketchSpace() / ESP.getSketchSize() )  ) );
    Serial.print("    Board type: ");
    #ifdef ARDUINO_ESP32_DEV
        Serial.println("ESP32 Dev Module");
    #elif defined ARDUINO_M5Stick_C
        Serial.println("M5Stick_C");
    #else
        Serial.println("Other");
    #endif

    Serial.print("    Chip model: "); Serial.println( ESP.getChipModel() );
    Serial.print("    Chip revision: "); Serial.println( ESP.getChipRevision() );
    Serial.print("    Number of cores: "); Serial.println( ESP.getChipCores() );
    Serial.print("    Chip MAC: 0x"); Serial.println( ESP.getEfuseMac(), HEX );
    Serial.print("    Flash size: 0x"); Serial.println( ESP.getFlashChipSize(), HEX );
    Serial.print("    Flash mode: "); Serial.println( ESP.getFlashChipMode() );
    Serial.print("    Flash speed (MHz): "); Serial.println( ESP.getFlashChipSpeed() / 1000000L);
    Serial.print("    CPU frequency (MHz): "); Serial.println( ESP.getCpuFreqMHz() );
    Serial.print("    Internal RAM total/free heap: 0x"); Serial.print( ESP.getHeapSize(), HEX );
    Serial.print(" / 0x"); Serial.println( ESP.getFreeHeap(), HEX );

    if ( ESP.getPsramSize() > 0 ) {
        Serial.print("    SPI RAM total/free heap: 0x"); Serial.print( ESP.getPsramSize(), HEX );
        Serial.print(" / 0x"); Serial.println( ESP.getFreePsram(), HEX );
    }
    else {
        Serial.println("    No SPI RAM found");
    }
    
    if ( CHIP_FEATURE_WIFI_BGN ) {
        Serial.println("    WiFi supported: b, g, n ");
    }
    else {
        Serial.println("    WiFi: Not supported.");
    }

    Serial.print("    BT Classic: ");
    if ( CHIP_FEATURE_BT ) {
        Serial.println("supported");
    }
    else {
        Serial.println("not supported");
    }
    
    Serial.print("    BLE: ");
    if ( CHIP_FEATURE_BLE ) {
        Serial.println("supported");
    }
    else {
        Serial.println("not supported");
    }
    
    Serial.println("Now, on with the show...");
    Serial.println("======================================\r\n");

    return;

}  // fetchInfo


/*~~~~ Set GROVE Pins Macros ~~~~*/
#define GROVE_PGM digitalWrite(TS_1, HIGH); digitalWrite(TS_0, HIGH)
#define GROVE_PVW digitalWrite(TS_1, HIGH); digitalWrite(TS_0, LOW)
#define GROVE_NO_SEL digitalWrite(TS_1, LOW); digitalWrite(TS_0, HIGH)
#define GROVE_UNKNOWN digitalWrite(TS_1, LOW); digitalWrite(TS_0, LOW)

