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

            // Read raw accelerometer data from sensor
            IMUdata acc;

            uint32_t timeout = millis() + DATA_WAIT_TIMEOUT_MS;
            while ( !sensor.getAccelerometer( acc.x, acc.y, acc.z ) ) {
                if ( millis() > timeout ) {
                    log_e( "Timeout retrieving QMI8658 accelerometer data" );
                    return Orientation::UNKNOWN;
                }
                delay( 10 );
            }

            // Apply axis remapping from board config
            // This allows the same IMU chip to be mounted in different orientations
            float boardX = IMU_AXIS_REMAP_X;
            float boardY = IMU_AXIS_REMAP_Y;
            float boardZ = IMU_AXIS_REMAP_Z;

            // Scale remapped accelerometer values
            float scaledAccX = boardX * ACCL_SCALE;
            float scaledAccY = boardY * ACCL_SCALE;
            float scaledAccZ = boardZ * ACCL_SCALE;
            
            // Debug: Show raw sensor values and board-mapped values
            log_d( "Raw IMU: acc.x=%.3f, acc.y=%.3f, acc.z=%.3f → boardX=%.3f, boardY=%.3f, boardZ=%.3f", 
                   acc.x * ACCL_SCALE, acc.y * ACCL_SCALE, acc.z * ACCL_SCALE,
                   scaledAccX, scaledAccY, scaledAccZ );

            // Pattern-based orientation detection
            // Match accelerometer readings against expected patterns
            Orientation rawOrientation = Orientation::UNKNOWN;

            // Check if device is flat first
            if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) < HIGH_TOL && abs( scaledAccZ ) > MID_TOL ) {
                rawOrientation = Orientation::FLAT;
            }
            // Check if device is vertical (one axis dominant)
            else if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) > MID_TOL && abs( scaledAccZ ) < HIGH_TOL ) {
                // Y-axis dominant - match against patterns
                // Pattern 3: (0, +1), Pattern 1: (0, -1)
                if ( scaledAccY > 0 ) {
                    rawOrientation = Orientation::ROTATE_270;  // Pattern 3
                }
                else {
                    rawOrientation = Orientation::ROTATE_90;   // Pattern 1
                }
            }
            else if ( abs( scaledAccX ) > MID_TOL && abs( scaledAccY ) < HIGH_TOL && abs( scaledAccZ ) < HIGH_TOL ) {
                // X-axis dominant - match against patterns
                // Pattern 0: (+1, 0), Pattern 2: (-1, 0)
                if ( scaledAccX > 0 ) {
                    rawOrientation = Orientation::ROTATE_180;  // Pattern 0
                }
                else {
                    rawOrientation = Orientation::ROTATE_0;    // Pattern 2
                }
            }

        // Debug output showing raw physical orientation
        const char *rawStr = ( rawOrientation == Orientation::ROTATE_0 ) ? "0°" :
                             ( rawOrientation == Orientation::ROTATE_90 ) ? "90°" :
                             ( rawOrientation == Orientation::ROTATE_180 ) ? "180°" :
                             ( rawOrientation == Orientation::ROTATE_270 ) ? "270°" :
                             ( rawOrientation == Orientation::FLAT ) ? "FLAT" : "UNKNOWN";

        log_d( "Physical orientation detected: %s", rawStr );

        return rawOrientation;  // Return raw physical orientation (offset will be applied at display level)
    }

        bool QMI8658_IMU::getRawAcceleration(float &accX, float &accY, float &accZ) {
            if (!initialized) {
                return false;
            }

            IMUdata acc;
            
            // Read accelerometer data with timeout
            uint32_t timeout = millis() + DATA_WAIT_TIMEOUT_MS;
            while (!sensor.getAccelerometer(acc.x, acc.y, acc.z)) {
                if (millis() > timeout) {
                    return false;
                }
                delay(10);
            }
            
            // Return raw accelerometer values (sensor units)
            accX = acc.x;
            accY = acc.y;
            accZ = acc.z;
            
            return true;
        }

        bool QMI8658_IMU::isAvailable() const {
            return initialized;
        }

        const char *QMI8658_IMU::getType() const {
            return "QMI8658";
        }

    } // namespace Hardware



//  --- EOF --- //
