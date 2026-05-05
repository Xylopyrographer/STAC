/**
 * @file BMI270_IMU.cpp
 * @brief BMI270 IMU implementation for M5Stack ATOM S3R
 *
 * Uses the TinyuZhao/BMI270_Sensor raw Bosch C driver.
 * Full-stop I2C reads (endTransmission(true)) are required — repeated-start
 * causes hardware timeouts on ESP32-S3 with OPI PSRAM.
 */

// Device_Config.h must be included before the compile guard so that the
// board config (e.g. AtomS3R_Config.h) is visible and IMU_TYPE_BMI270 is defined.
#include "Device_Config.h"

// Compile for boards that have a BMI270 directly, or the ATOM Matrix which
// detects BMI270 at runtime alongside MPU6886 (both share I2C address 0x68).
#if defined(IMU_TYPE_BMI270) || defined(IMU_TYPE_ATOM_MATRIX)

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

        // Configure accelerometer before enabling.
        // The BMI270 power-on default range is ±8G (register 0x41 = 0x02).
        // Explicitly setting ±2G (highest resolution for 1g detection) with
        // 100 Hz ODR ensures consistent sensitivity and correct divisor usage.
        {
            struct bmi2_sens_config accel_cfg;
            accel_cfg.type              = BMI2_ACCEL;
            accel_cfg.cfg.acc.odr       = BMI2_ACC_ODR_100HZ;
            accel_cfg.cfg.acc.bwp       = BMI2_ACC_OSR2_AVG2;
            accel_cfg.cfg.acc.filter_perf = BMI2_PERF_OPT_MODE;
            accel_cfg.cfg.acc.range     = BMI2_ACC_RANGE_2G;
            rslt = bmi2_set_sensor_config( &accel_cfg, 1, &_dev );
            if ( rslt != BMI2_OK ) {
                log_e( "BMI270 set accel config failed: %d", rslt );
                _initialized = false;
                return false;
            }
        }

        // Enable accelerometer
        uint8_t sens_list[] = { BMI2_ACCEL };
        rslt = bmi270_sensor_enable( sens_list, 1, &_dev );
        if ( rslt != BMI2_OK ) {
            log_e( "BMI270 sensor enable failed: %d", rslt );
            _initialized = false;
            return false;
        }

        // Wait for the first accelerometer sample to be ready.
        // Poll the STATUS register (0x03) bit 7 (BMI2_DRDY_ACC_MASK = 0x80).
        // Default accel ODR is 100 Hz → first sample within ~10 ms, but the
        // sensor can take longer on cold power-up, so cap at 500 ms.
        {
            uint8_t status = 0;
            const uint32_t timeoutMs = 500;
            const uint32_t t0 = millis();
            while ( millis() - t0 < timeoutMs ) {
                if ( bmi2_get_status( &status, &_dev ) == BMI2_OK &&
                        ( status & BMI2_DRDY_ACC_MASK ) ) {
                    log_d( "BMI270 accel data ready after %lu ms", millis() - t0 );
                    break;
                }
                delay( 5 );
            }
            if ( !( status & BMI2_DRDY_ACC_MASK ) ) {
                log_w( "BMI270 accel data-ready timeout — proceeding anyway" );
            }
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

        log_i( "BMI270 orientation: %s (raw X=%.0f Y=%.0f Z=%.0f)", rawStr, scaledAccX, scaledAccY, scaledAccZ );

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
