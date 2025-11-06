#include <Arduino.h>
#include "Device_Config.h"
#include "Config/Constants.h"
#include "Config/Types.h"

void setup() {
    Serial.begin(115200);
    delay(1000);  // Give time for USB CDC
    
    // Test that our configuration is working
    Serial.println("\n=== STAC Configuration Test ===");
    Serial.printf("Board: %s\n", STAC::Config::Strings::BOARD_NAME);
    Serial.printf("ID Prefix: %s\n", STAC::Config::Strings::ID_PREFIX);
    Serial.printf("Display: %dx%d (%d LEDs)\n", 
                  STAC::Config::Display::MATRIX_WIDTH,
                  STAC::Config::Display::MATRIX_HEIGHT,
                  STAC::Config::Display::MATRIX_SIZE);
    Serial.printf("Display Pin: %d\n", STAC::Config::Pins::DISPLAY_DATA);
    Serial.printf("Button Pin: %d\n", STAC::Config::Pins::BUTTON);
    
    #ifdef IMU_HAS_IMU
        Serial.printf("IMU: Present (SCL=%d, SDA=%d)\n", 
                      STAC::Config::Pins::IMU_SCL,
                      STAC::Config::Pins::IMU_SDA);
    #else
        Serial.println("IMU: Not present");
    #endif
    
    Serial.printf("Brightness: Min=%d, Max=%d, Default=%d\n",
                  STAC::Config::Display::BRIGHTNESS_MIN,
                  STAC::Config::Display::BRIGHTNESS_MAX,
                  STAC::Config::Display::BRIGHTNESS_DEFAULT);
    
    // Test creating a struct
    STAC::StacOperations ops;
    Serial.printf("\nDefault operations:\n");
    Serial.printf("  Model: %s\n", ops.switchModel.c_str());
    Serial.printf("  Channel: %d\n", ops.tallyChannel);
    Serial.printf("  Poll Interval: %lu ms\n", ops.statusPollInterval);
    
    Serial.println("\n=== Configuration Test Complete ===");
    Serial.println("Phase 1 foundation is working!");
}

void loop() {
    // Nothing to do yet
    delay(1000);
}


//  --- EOF --- //
