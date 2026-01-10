#include "Hardware/Sensors/QMI8658_IMU.h"
#include "Device_Config.h"  // Add this line to get IMU_ORIENTATION_OFFSET
#include <Arduino.h>


    namespace Hardware {

        QMI8658_IMU::QMI8658_IMU( uint8_t sclPin, uint8_t sdaPin, uint8_t address )
            : sensor()
            , sclPin( sclPin )
            , sdaPin( sdaPin )
            , address( address )
            , initialized( false ) {
        }

        bool QMI8658_IMU::begin() {
            if ( !sensor.begin( Wire, address, sdaPin, sclPin ) ) {
                log_e( "Failed to initialize QMI8658 IMU" );
                Wire.flush();
                initialized = false;
                return false;
            }

            // Configure accelerometer
            sensor.configAccelerometer(
                SensorQMI8658::ACC_RANGE_2G,
                SensorQMI8658::ACC_ODR_1000Hz,
                SensorQMI8658::LPF_MODE_0
            );

            // Configure gyroscope
            sensor.configGyroscope(
                SensorQMI8658::GYR_RANGE_64DPS,
                SensorQMI8658::GYR_ODR_896_8Hz,
                SensorQMI8658::LPF_MODE_3
            );

            // Enable sensors
            sensor.enableGyroscope();
            sensor.enableAccelerometer();

            // Wait for data to be ready
            uint32_t timeout = millis() + DATA_WAIT_TIMEOUT_MS;
            while ( !sensor.getDataReady() ) {
                if ( millis() > timeout ) {
                    log_e( "Timeout waiting for QMI8658 data ready" );
                    sensor.powerDown();
                    Wire.flush();
                    initialized = false;
                    return false;
                }
                delay( 10 );
            }

            initialized = true;
            log_i( "QMI8658 IMU initialized on I2C (SCL=%d, SDA=%d, Addr=0x%02X)",
                   sclPin, sdaPin, address );
            return true;
        }

        Orientation QMI8658_IMU::getOrientation() {
            if ( !initialized ) {
                log_w( "QMI8658 not initialized, returning UNKNOWN" );
                return Orientation::UNKNOWN;
            }

            IMUdata acc;

            // Read accelerometer data
            uint32_t timeout = millis() + DATA_WAIT_TIMEOUT_MS;
            while ( !sensor.getAccelerometer( acc.x, acc.y, acc.z ) ) {
                if ( millis() > timeout ) {
                    log_e( "Timeout retrieving QMI8658 accelerometer data" );
                    return Orientation::UNKNOWN;
                }
                delay( 10 );
            }

            // Scale accelerometer values
            // QMI8658 orientation on Waveshare (rear view, home position):
            // Sensor +X = UP (away from USB), Sensor +Y = LEFT
            // Board +Y = UP (away from USB), Board +X = RIGHT
            float scaledAccX = -(acc.y * ACCL_SCALE);     // Board X (RIGHT) = -Sensor Y (LEFT)
            float scaledAccY = acc.x * ACCL_SCALE;        // Board Y (UP) = Sensor X (UP)
            float scaledAccZ = acc.z * ACCL_SCALE;        // Board Z = Sensor Z

            // Determine raw orientation based on accelerometer readings
            // The USB port is the reference point (home = vertical, USB down)
            Orientation rawOrientation = Orientation::UNKNOWN;

            if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) > MID_TOL && abs( scaledAccZ ) < HIGH_TOL ) {
                if ( scaledAccY > 0 ) {
                    rawOrientation = Orientation::LEFT;  // USB port to the left
                }
                else {
                    rawOrientation = Orientation::RIGHT; // USB port to the right
                }
            }
            else if ( abs( scaledAccX ) > MID_TOL && abs( scaledAccY ) < HIGH_TOL && abs( scaledAccZ ) < HIGH_TOL ) {
                if ( scaledAccX > 0 ) {
                    rawOrientation = Orientation::DOWN;  // USB port at the top
                }
                else {
                    rawOrientation = Orientation::UP;    // USB port at the bottom
                }
            }
            else if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) < HIGH_TOL && abs( scaledAccZ ) > MID_TOL ) {
                rawOrientation = Orientation::FLAT;      // Device is horizontal
            }

            // Apply orientation offset correction
            OrientationOffset offset = static_cast<OrientationOffset>( IMU_ORIENTATION_OFFSET );
            Orientation corrected = applyOrientationOffset( rawOrientation, offset );

            return corrected;
        }

        bool QMI8658_IMU::isAvailable() const {
            return initialized;
        }

        const char *QMI8658_IMU::getType() const {
            return "QMI8658";
        }

    } // namespace Hardware



//  --- EOF --- //
