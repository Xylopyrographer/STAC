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
        struct {
            float x, y, z;
        } acc = { acc_x, acc_y, acc_z };
        float boardX = IMU_AXIS_REMAP_X;
        float boardY = IMU_AXIS_REMAP_Y;
        float boardZ = IMU_AXIS_REMAP_Z;

        // Scale remapped accelerometer values
        float scaledAccX = boardX * ACCL_SCALE;
        float scaledAccY = boardY * ACCL_SCALE;
        float scaledAccZ = boardZ * ACCL_SCALE;

        // Use shared pattern detection method from base class
        Orientation rawOrientation = detectOrientationFromPattern( scaledAccX, scaledAccY, scaledAccZ );

        // Debug output showing raw physical orientation
        const char *rawStr = ( rawOrientation == Orientation::ROTATE_0 ) ? "0°" :
                             ( rawOrientation == Orientation::ROTATE_90 ) ? "90°" :
                             ( rawOrientation == Orientation::ROTATE_180 ) ? "180°" :
                             ( rawOrientation == Orientation::ROTATE_270 ) ? "270°" :
                             ( rawOrientation == Orientation::FLAT ) ? "FLAT" : "UNKNOWN";

        log_d( "Physical orientation detected: %s", rawStr );

        return rawOrientation;  // Return raw physical orientation (offset will be applied at display level)
    }

    bool MPU6886_IMU::getRawAcceleration( float &accX, float &accY, float &accZ ) {
        if ( !initialized ) {
            return false;
        }

        float ax, ay, az;
        sensor.getAccel( &ax, &ay, &az );

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
