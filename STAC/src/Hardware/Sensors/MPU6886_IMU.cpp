#include "Hardware/Sensors/MPU6886_IMU.h"
#include <Arduino.h>

namespace STAC {
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

            float accX, accY, accZ;
            float gyroX, gyroY, gyroZ;

            // Read accelerometer data
            sensor.getAccel( &accX, &accY, &accZ );
            sensor.getGyro( &gyroX, &gyroY, &gyroZ );

            // Scale accelerometer values
            float scaledAccX = accX * ACCL_SCALE;
            float scaledAccY = accY * ACCL_SCALE;
            float scaledAccZ = accZ * ACCL_SCALE;

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

            // Apply orientation offset correction
            OrientationOffset offset = static_cast<OrientationOffset>( IMU_ORIENTATION_OFFSET );
            Orientation corrected = applyOrientationOffset( rawOrientation, offset );

            // Debug output to verify correction is applied
            const char *rawStr = ( rawOrientation == Orientation::UP ) ? "UP" :
                                 ( rawOrientation == Orientation::DOWN ) ? "DOWN" :
                                 ( rawOrientation == Orientation::LEFT ) ? "LEFT" :
                                 ( rawOrientation == Orientation::RIGHT ) ? "RIGHT" :
                                 ( rawOrientation == Orientation::FLAT ) ? "FLAT" : "UNKNOWN";
            const char *corrStr = ( corrected == Orientation::UP ) ? "UP" :
                                  ( corrected == Orientation::DOWN ) ? "DOWN" :
                                  ( corrected == Orientation::LEFT ) ? "LEFT" :
                                  ( corrected == Orientation::RIGHT ) ? "RIGHT" :
                                  ( corrected == Orientation::FLAT ) ? "FLAT" : "UNKNOWN";

            log_d( "Orientation: raw=%s, offset=%d, corrected=%s", rawStr, IMU_ORIENTATION_OFFSET, corrStr );

            return corrected;
        }

        bool MPU6886_IMU::isAvailable() const {
            return initialized;
        }

        const char *MPU6886_IMU::getType() const {
            return "MPU6886";
        }

    } // namespace Hardware
} // namespace STAC


//  --- EOF --- //
