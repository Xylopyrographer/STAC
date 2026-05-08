/**
 * @file main.cpp
 * @brief BMI270 IMU sanity-check sketch for M5Stack ATOM S3R
 *
 * Uses the Arduino_BMI270_BMM150 library (BoschSensorClass) to initialise the
 * BMI270 over I2C and print raw accelerometer counts + converted g-values to
 * the serial monitor once per second.
 *
 * ATOM S3R I2C pins: SCL=0, SDA=45, address 0x68.
 *
 * The library already defines a global IMU_BMI270_BMM150(Wire).
 * Call Wire.begin(SDA, SCL) first, then IMU_BMI270_BMM150.begin().
 */

#include <Arduino.h>
#include <Wire.h>
#include <Arduino_BMI270_BMM150.h>

// ATOM S3R pin assignments
static constexpr uint8_t IMU_SDA = 45;
static constexpr uint8_t IMU_SCL =  0;

// Use the library's built-in global — do NOT declare another BoschSensorClass,
// as the library already defines IMU_BMI270_BMM150(Wire) in BMI270.cpp.

void setup() {
    Serial.begin( 115200 );
    delay( 1500 ); // let USB CDC enumerate
    Serial.println( "\n=== BMI270 IMU test (Arduino_BMI270_BMM150) ===" );

    Wire.begin( IMU_SDA, IMU_SCL, 100000UL );

    if ( !IMU_BMI270_BMM150.begin( BOSCH_ACCELEROMETER_ONLY ) ) {
        Serial.println( "ERROR: IMU.begin() failed — check wiring/address" );
        while ( true ) {
            delay( 1000 );
        }
    }

    Serial.print( "Accel sample rate: " );
    Serial.print( IMU_BMI270_BMM150.accelerationSampleRate() );
    Serial.println( " Hz" );
    Serial.println( "Printing X, Y, Z  [g]  every second...\n" );
}

void loop() {
    float x, y, z;
    if ( IMU_BMI270_BMM150.accelerationAvailable() ) {
        IMU_BMI270_BMM150.readAcceleration( x, y, z );
        Serial.printf( "Accel  X=%7.4f  Y=%7.4f  Z=%7.4f  g\n", x, y, z );
    }
    else {
        Serial.println( "(no sample ready)" );
    }
    delay( 1000 );
}
