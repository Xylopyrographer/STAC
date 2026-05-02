/**
 * @file BMI270_IMU.cpp
 * @brief BMI270 IMU implementation for M5Stack ATOM S3R
 *
 * Uses the TinyuZhao/BMI270_Sensor raw Bosch C driver.
 * Full-stop I2C reads (endTransmission(true)) are required — repeated-start
 * causes hardware timeouts on ESP32-S3 with OPI PSRAM.
 */

// Compile only for boards that have a BMI270
#if defined(IMU_TYPE_BMI270)

#include "Hardware/Sensors/BMI270_IMU.h"
#include "Config/Constants.h"
#include <Arduino.h>

namespace Hardware {

    // =========================================================================
    // Construction
    // =========================================================================

    BMI270_IMU::BMI270_IMU( uint8_t sclPin, uint8_t sdaPin, uint32_t clock,
                            uint8_t i2cAddr )
        : _sclPin( sclPin )
        , _sdaPin( sdaPin )
        , _clock( clock )
        , _i2cAddr( i2cAddr )
        , _initialized( false )
        , _dev{} {
    }

    // =========================================================================
    // IIMU interface
    // =========================================================================

    bool BMI270_IMU::begin() {
        // Initialize I2C bus (SDA, SCL ordering matches Wire.begin signature)
        Wire.begin( _sdaPin, _sclPin, _clock );

        // Fill Bosch driver device descriptor
        _dev.intf     = BMI2_I2C_INTF;
        _dev.intf_ptr = this;               // passed to callbacks as intf_ptr
        _dev.read     = BMI270_IMU::i2c_read;
        _dev.write    = BMI270_IMU::i2c_write;
        _dev.delay_us = BMI270_IMU::delay_us;
        _dev.read_write_len = 32;
        _dev.config_file_ptr = nullptr;

        // Initialize BMI270 chip
        int8_t rslt = bmi270_init( &_dev );
        if ( rslt != BMI2_OK ) {
            log_e( "BMI270 init failed: %d", rslt );
            _initialized = false;
            return false;
        }

        // Enable accelerometer
        uint8_t sens_list[] = { BMI2_ACCEL };
        rslt = bmi270_sensor_enable( sens_list, 1, &_dev );
        if ( rslt != BMI2_OK ) {
            log_e( "BMI270 sensor enable failed: %d", rslt );
            _initialized = false;
            return false;
        }

        _initialized = true;
        log_i( "BMI270 IMU initialized (SCL=%d, SDA=%d)", _sclPin, _sdaPin );
        return true;
    }

    Orientation BMI270_IMU::getOrientation() {
        if ( !_initialized ) {
            log_w( "BMI270 not initialized, returning UNKNOWN" );
            return Orientation::UNKNOWN;
        }

        struct bmi2_sens_data sensor_data = {};
        int8_t rslt = bmi2_get_sensor_data( &sensor_data, &_dev );
        if ( rslt != BMI2_OK ) {
            log_w( "BMI270 get sensor data failed: %d", rslt );
            return Orientation::UNKNOWN;
        }

        // Convert raw int16_t counts to g values (±2 g range → 16384 counts/g)
        struct {
            float x, y, z;
        } acc = {
            sensor_data.acc.x / 16384.0f,
            sensor_data.acc.y / 16384.0f,
            sensor_data.acc.z / 16384.0f
        };

        // Apply board-specific axis remapping from board config
        float boardX = IMU_AXIS_REMAP_X;
        float boardY = IMU_AXIS_REMAP_Y;
        float boardZ = IMU_AXIS_REMAP_Z;

        // Scale and detect orientation using shared pattern from base class
        float scaledAccX = boardX * ACCL_SCALE;
        float scaledAccY = boardY * ACCL_SCALE;
        float scaledAccZ = boardZ * ACCL_SCALE;

        Orientation rawOrientation = detectOrientationFromPattern( scaledAccX, scaledAccY, scaledAccZ );

        const char *rawStr = ( rawOrientation == Orientation::ROTATE_0   ) ? "0°"     :
                             ( rawOrientation == Orientation::ROTATE_90  ) ? "90°"    :
                             ( rawOrientation == Orientation::ROTATE_180 ) ? "180°"   :
                             ( rawOrientation == Orientation::ROTATE_270 ) ? "270°"   :
                             ( rawOrientation == Orientation::FLAT       ) ? "FLAT"   : "UNKNOWN";

        log_d( "BMI270 physical orientation: %s", rawStr );

        return rawOrientation;
    }

    bool BMI270_IMU::getRawAcceleration( float &accX, float &accY, float &accZ ) {
        if ( !_initialized ) {
            return false;
        }

        struct bmi2_sens_data sensor_data = {};
        if ( bmi2_get_sensor_data( &sensor_data, &_dev ) != BMI2_OK ) {
            return false;
        }

        accX = sensor_data.acc.x / 16384.0f;
        accY = sensor_data.acc.y / 16384.0f;
        accZ = sensor_data.acc.z / 16384.0f;
        return true;
    }

    bool BMI270_IMU::isAvailable() const {
        return _initialized;
    }

    const char *BMI270_IMU::getType() const {
        return "BMI270";
    }

    // =========================================================================
    // Static I2C callbacks for Bosch driver
    // =========================================================================

    int8_t BMI270_IMU::i2c_read( uint8_t reg_addr, uint8_t *reg_data,
                                 uint32_t len, void *intf_ptr ) {
        BMI270_IMU *self = static_cast<BMI270_IMU *>( intf_ptr );

        Wire.beginTransmission( self->_i2cAddr );
        Wire.write( reg_addr );
        // Full stop — NOT repeated-start.  Repeated-start (false) causes hardware
        // timeouts on ESP32-S3 with OPI PSRAM via the ESP32 ng I2C driver.
        if ( Wire.endTransmission( true ) != 0 ) {
            return BMI2_E_COM_FAIL;
        }

        uint32_t received = Wire.requestFrom( self->_i2cAddr, ( uint8_t )len );
        if ( received != len ) {
            return BMI2_E_COM_FAIL;
        }

        for ( uint32_t i = 0; i < len; i++ ) {
            reg_data[ i ] = Wire.read();
        }
        return BMI2_OK;
    }

    int8_t BMI270_IMU::i2c_write( uint8_t reg_addr, const uint8_t *reg_data,
                                  uint32_t len, void *intf_ptr ) {
        BMI270_IMU *self = static_cast<BMI270_IMU *>( intf_ptr );

        Wire.beginTransmission( self->_i2cAddr );
        Wire.write( reg_addr );
        for ( uint32_t i = 0; i < len; i++ ) {
            Wire.write( reg_data[ i ] );
        }
        return ( Wire.endTransmission() == 0 ) ? BMI2_OK : BMI2_E_COM_FAIL;
    }

    void BMI270_IMU::delay_us( uint32_t period_us, void * /*intf_ptr*/ ) {
        delayMicroseconds( period_us );
    }

} // namespace Hardware

#endif // IMU_TYPE_BMI270

//  --- EOF --- //
