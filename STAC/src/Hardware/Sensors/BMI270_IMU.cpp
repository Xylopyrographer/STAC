#include "Device_Config.h"
#if defined(IMU_TYPE_ATOM_MATRIX)

#include "Hardware/Sensors/BMI270_IMU.h"
#include <Arduino.h>


namespace Hardware {

    // ============================================================================
    // Static Bosch driver callback implementations
    // ============================================================================

    BMI2_INTF_RETURN_TYPE BMI270_IMU::i2c_read( uint8_t reg_addr, uint8_t *reg_data,
            uint32_t len, void *intf_ptr ) {
        TwoWire *wire = reinterpret_cast<TwoWire *>( intf_ptr );

        wire->beginTransmission( BMI2_I2C_PRIM_ADDR );
        wire->write( reg_addr );
        if ( wire->endTransmission( false ) != 0 ) {
            return static_cast<BMI2_INTF_RETURN_TYPE>( 1 );  // communication error
        }

        wire->requestFrom( static_cast<uint8_t>( BMI2_I2C_PRIM_ADDR ),
                           static_cast<uint8_t>( len ) );
        for ( uint32_t i = 0; i < len; i++ ) {
            reg_data[ i ] = wire->available() ? wire->read() : 0;
        }

        return BMI2_INTF_RET_SUCCESS;
    }

    BMI2_INTF_RETURN_TYPE BMI270_IMU::i2c_write( uint8_t reg_addr, const uint8_t *reg_data,
            uint32_t len, void *intf_ptr ) {
        TwoWire *wire = reinterpret_cast<TwoWire *>( intf_ptr );

        wire->beginTransmission( BMI2_I2C_PRIM_ADDR );
        wire->write( reg_addr );
        for ( uint32_t i = 0; i < len; i++ ) {
            wire->write( reg_data[ i ] );
        }
        if ( wire->endTransmission() != 0 ) {
            return static_cast<BMI2_INTF_RETURN_TYPE>( 1 );  // communication error
        }

        return BMI2_INTF_RET_SUCCESS;
    }

    void BMI270_IMU::delay_us_cb( uint32_t period, void *intf_ptr ) {
        ( void )intf_ptr;
        delayMicroseconds( period );
    }

    // ============================================================================
    // Constructor
    // ============================================================================

    BMI270_IMU::BMI270_IMU( uint8_t sclPin, uint8_t sdaPin, uint32_t clock )
        : dev{}
        , sclPin( sclPin )
        , sdaPin( sdaPin )
        , clockHz( clock )
        , initialized( false ) {
    }

    // ============================================================================
    // Public interface
    // ============================================================================

    bool BMI270_IMU::begin() {
        Wire.begin( sdaPin, sclPin, clockHz );

        // Configure Bosch driver interface
        dev.intf           = BMI2_I2C_INTF;
        dev.intf_ptr       = &Wire;
        dev.read           = i2c_read;
        dev.write          = i2c_write;
        dev.delay_us       = delay_us_cb;
        dev.read_write_len = 32;  // safe chunk size for ESP32 I2C buffer

        // Initialize BMI270 — this uploads the firmware configuration blob
        int8_t result = bmi270_init( &dev );
        if ( result != BMI2_OK ) {
            log_e( "Failed to initialize BMI270 (err=%d)", result );
            initialized = false;
            return false;
        }

        // Enable accelerometer
        uint8_t sens_list[ 1 ] = { BMI2_ACCEL };
        result                  = bmi270_sensor_enable( sens_list, 1, &dev );
        if ( result != BMI2_OK ) {
            log_e( "Failed to enable BMI270 accelerometer (err=%d)", result );
            initialized = false;
            return false;
        }

        initialized = true;
        log_i( "BMI270 IMU initialized on I2C (SCL=%d, SDA=%d)", sclPin, sdaPin );
        return true;
    }

    Orientation BMI270_IMU::getOrientation() {
        if ( !initialized ) {
            log_w( "BMI270 not initialized, returning UNKNOWN" );
            return Orientation::UNKNOWN;
        }

        struct bmi2_sens_data sensor_data = {};
        if ( bmi2_get_sensor_data( &sensor_data, &dev ) != BMI2_OK ) {
            return Orientation::UNKNOWN;
        }

        // Convert raw int16_t ADC counts → g values (default ±2G range: 16384 LSB/g)
        float acc_x = static_cast<float>( sensor_data.acc.x ) / 16384.0f;
        float acc_y = static_cast<float>( sensor_data.acc.y ) / 16384.0f;
        float acc_z = static_cast<float>( sensor_data.acc.z ) / 16384.0f;

        // Apply axis remapping from board config.
        // The macros expand using a local struct named 'acc', e.g. IMU_AXIS_REMAP_X → (acc.x)
        struct {
            float x, y, z;
        } acc = { acc_x, acc_y, acc_z };
        float boardX = IMU_AXIS_REMAP_X;
        float boardY = IMU_AXIS_REMAP_Y;
        float boardZ = IMU_AXIS_REMAP_Z;

        // Scale remapped values for the pattern detection algorithm
        float scaledAccX = boardX * ACCL_SCALE;
        float scaledAccY = boardY * ACCL_SCALE;
        float scaledAccZ = boardZ * ACCL_SCALE;

        // Use shared pattern detection method from base class
        Orientation rawOrientation = detectOrientationFromPattern( scaledAccX, scaledAccY, scaledAccZ );

        const char *rawStr = ( rawOrientation == Orientation::ROTATE_0 )   ? "0°" :
                             ( rawOrientation == Orientation::ROTATE_90 )  ? "90°" :
                             ( rawOrientation == Orientation::ROTATE_180 ) ? "180°" :
                             ( rawOrientation == Orientation::ROTATE_270 ) ? "270°" :
                             ( rawOrientation == Orientation::FLAT )       ? "FLAT" : "UNKNOWN";
        log_d( "Physical orientation detected: %s", rawStr );

        return rawOrientation;
    }

    bool BMI270_IMU::getRawAcceleration( float &accX, float &accY, float &accZ ) {
        if ( !initialized ) {
            return false;
        }

        struct bmi2_sens_data sensor_data = {};
        if ( bmi2_get_sensor_data( &sensor_data, &dev ) != BMI2_OK ) {
            return false;
        }

        // Convert raw int16_t ADC counts → g values (default ±2G range: 16384 LSB/g)
        accX = static_cast<float>( sensor_data.acc.x ) / 16384.0f;
        accY = static_cast<float>( sensor_data.acc.y ) / 16384.0f;
        accZ = static_cast<float>( sensor_data.acc.z ) / 16384.0f;

        return true;
    }

    bool BMI270_IMU::isAvailable() const {
        return initialized;
    }

    const char *BMI270_IMU::getType() const {
        return "BMI270";
    }

} // namespace Hardware


#endif  // IMU_TYPE_ATOM_MATRIX

//  --- EOF --- //
