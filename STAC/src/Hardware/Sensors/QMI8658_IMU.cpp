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

        // Configure accelerometer (only - gyroscope not needed for orientation detection)
        sensor.configAccelerometer(
            SensorQMI8658::ACC_RANGE_2G,
            SensorQMI8658::ACC_ODR_1000Hz,
            SensorQMI8658::LPF_MODE_0
        );

        // Enable accelerometer only
        sensor.enableAccelerometer();

        // QMI8658 is ready immediately after enable (no polling needed like MPU6886)
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

        // Read raw accelerometer data from sensor (immediate read like MPU6886)
        IMUdata acc;
        if ( !sensor.getAccelerometer( acc.x, acc.y, acc.z ) ) {
            log_e( "Failed to read QMI8658 accelerometer data" );
            return Orientation::UNKNOWN;
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

    bool QMI8658_IMU::getRawAcceleration( float &accX, float &accY, float &accZ ) {
        if ( !initialized ) {
            return false;
        }

        IMUdata acc;

        // Read accelerometer data directly (immediate read like MPU6886)
        if ( !sensor.getAccelerometer( acc.x, acc.y, acc.z ) ) {
            return false;
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
