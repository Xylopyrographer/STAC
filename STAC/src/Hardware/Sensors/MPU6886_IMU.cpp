#include "Hardware/Sensors/MPU6886_IMU.h"
#include <Arduino.h>


    namespace Hardware {

        MPU6886_IMU::MPU6886_IMU( uint8_t sclPin, uint8_t sdaPin, uint32_t clock )
            : sensor()
            , sclPin( sclPin )
            , sdaPin( sdaPin )
            , clock( clock )
            , initialized( false ) {
        }

        bool MPU6886_IMU::begin() {
            Wire.begin( sdaPin, sclPin, clock );

            if ( sensor.begin() != 0 ) { // Remove the &Wire parameter
                log_e( "Failed to initialize MPU6886 IMU" );
                Wire.flush();
                initialized = false;
                return false;
            }

            initialized = true;
            log_i( "MPU6886 IMU initialized on I2C (SCL=%d, SDA=%d)", sclPin, sdaPin );
            return true;
        }

        Orientation MPU6886_IMU::getOrientation() {
            if ( !initialized ) {
                log_w( "MPU6886 not initialized, returning UNKNOWN" );
                return Orientation::UNKNOWN;
            }

            // Read raw accelerometer data from sensor
            float acc_x, acc_y, acc_z;
            float gyroX, gyroY, gyroZ;

            sensor.getAccel( &acc_x, &acc_y, &acc_z );
            sensor.getGyro( &gyroX, &gyroY, &gyroZ );

            // Apply axis remapping from board config
            // This allows the same IMU chip to be mounted in different orientations
            struct { float x, y, z; } acc = { acc_x, acc_y, acc_z };
            float boardX = IMU_AXIS_REMAP_X;
            float boardY = IMU_AXIS_REMAP_Y;
            float boardZ = IMU_AXIS_REMAP_Z;

            // Scale remapped accelerometer values
            float scaledAccX = boardX * ACCL_SCALE;
            float scaledAccY = boardY * ACCL_SCALE;
            float scaledAccZ = boardZ * ACCL_SCALE;

            // Determine raw orientation based on accelerometer readings
            Orientation rawOrientation = Orientation::UNKNOWN;

            if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) > MID_TOL && abs( scaledAccZ ) < HIGH_TOL ) {
                if ( scaledAccY > 0 ) {
                    rawOrientation = Orientation::LEFT;
                }
                else {
                    rawOrientation = Orientation::RIGHT;
                }
            }
            else if ( abs( scaledAccX ) > MID_TOL && abs( scaledAccY ) < HIGH_TOL && abs( scaledAccZ ) < HIGH_TOL ) {
                if ( scaledAccX > 0 ) {
                    rawOrientation = Orientation::DOWN;
                }
                else {
                    rawOrientation = Orientation::UP;
                }
            }
            else if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) < HIGH_TOL && abs( scaledAccZ ) > MID_TOL ) {
                rawOrientation = Orientation::FLAT;
            }

            // Apply orientation offset correction from board config
            OrientationOffset offset = static_cast<OrientationOffset>( IMU_ROTATION_OFFSET );
            Orientation corrected = applyOrientationOffset( rawOrientation, offset );

            // Debug output to verify correction is applied
            const char *rawStr = ( rawOrientation == Orientation::UP ) ? "UP (0°)" :
                                 ( rawOrientation == Orientation::DOWN ) ? "DOWN (180°)" :
                                 ( rawOrientation == Orientation::LEFT ) ? "LEFT (270°)" :
                                 ( rawOrientation == Orientation::RIGHT ) ? "RIGHT (90°)" :
                                 ( rawOrientation == Orientation::FLAT ) ? "FLAT" : "UNKNOWN";
            const char *corrStr = ( corrected == Orientation::UP ) ? "UP (0°)" :
                                  ( corrected == Orientation::DOWN ) ? "DOWN (180°)" :
                                  ( corrected == Orientation::LEFT ) ? "LEFT (270°)" :
                                  ( corrected == Orientation::RIGHT ) ? "RIGHT (90°)" :
                                  ( corrected == Orientation::FLAT ) ? "FLAT" : "UNKNOWN";

            log_d( "Display orientation: raw=%s, offset=%d, corrected=%s", rawStr, IMU_ROTATION_OFFSET, corrStr );

            return corrected;
        }

        bool MPU6886_IMU::getRawAcceleration(float &accX, float &accY, float &accZ) {
            if (!initialized) {
                return false;
            }

            float ax, ay, az;
            sensor.getAccel(&ax, &ay, &az);
            
            // Return raw accelerometer values in g's
            accX = ax;
            accY = ay;
            accZ = az;
            
            return true;
        }

        bool MPU6886_IMU::isAvailable() const {
            return initialized;
        }

        const char *MPU6886_IMU::getType() const {
            return "MPU6886";
        }

    } // namespace Hardware



//  --- EOF --- //
