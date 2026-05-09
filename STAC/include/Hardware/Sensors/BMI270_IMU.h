/**
 * @file BMI270_IMU.h
 * @brief BMI270 IMU implementation for M5Stack ATOM S3R
 *
 * Implements the IIMU interface for the Bosch BMI270 6-axis IMU.
 * Uses the TinyuZhao/BMI270_Sensor raw Bosch C driver directly —
 * no Arduino wrapper class exists for this library.
 *
 * I2C address: 0x68 (SDO pulled low)
 *
 * IMPORTANT — I2C read callback:
 *   Use full stop (endTransmission(true)) between the register write and the
 *   subsequent requestFrom().  The ESP32 ng I2C driver produces a hardware
 *   timeout with repeated-start (endTransmission(false)) on certain ESP32-S3
 *   hardware configurations (OPI PSRAM).
 *
 * Library: https://github.com/TinyuZhao/BMI270_Sensor
 */

#ifndef STAC_BMI270_IMU_H
#define STAC_BMI270_IMU_H

#include "IIMU.h"
#include "Device_Config.h"
#include <Wire.h>

// Include BMI270 Bosch C driver headers (provided by TinyuZhao/BMI270_Sensor)
#include <bmi270.h>

namespace Hardware {

    /**
     * @brief BMI270 IMU for M5Stack ATOM S3R
     *
     * Provides 6-axis accelerometer/gyroscope data used for display
     * orientation detection.  The BMM150 magnetometer connected through
     * the BMI270 sensor hub is not used by STAC.
     */
    class BMI270_IMU : public IIMU {
      public:
        /**
         * @brief Construct a new BMI270_IMU object
         * @param sclPin I2C SCL pin
         * @param sdaPin I2C SDA pin
         * @param clock  I2C clock frequency (Hz)
         * @param i2cAddr BMI270 I2C address (default 0x68)
         */
        BMI270_IMU( uint8_t sclPin, uint8_t sdaPin, uint32_t clock,
                    uint8_t i2cAddr = 0x68 );

        ~BMI270_IMU() override = default;

        // IIMU interface implementation
        bool begin() override;
        Orientation getOrientation() override;
        bool getRawAcceleration( float &accX, float &accY, float &accZ ) override;
        bool isAvailable() const override;
        const char *getType() const override;

      private:
        uint8_t  _sclPin;
        uint8_t  _sdaPin;
        uint32_t _clock;
        uint8_t  _i2cAddr;
        bool     _initialized;

        struct bmi2_dev _dev;  // Bosch driver device descriptor

        // -----------------------------------------------------------------------
        // Static I2C callbacks registered with the Bosch driver.
        // intf_ptr is set to `this` so callbacks can access _i2cAddr.
        // -----------------------------------------------------------------------
        static int8_t i2c_read( uint8_t reg_addr, uint8_t *reg_data,
                                uint32_t len, void *intf_ptr );
        static int8_t i2c_write( uint8_t reg_addr, const uint8_t *reg_data,
                                 uint32_t len, void *intf_ptr );
        static void delay_us( uint32_t period_us, void *intf_ptr );
    };

} // namespace Hardware

#endif // STAC_BMI270_IMU_H

//  --- EOF --- //
