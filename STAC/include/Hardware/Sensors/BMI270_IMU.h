#ifndef STAC_BMI270_IMU_H
#define STAC_BMI270_IMU_H

#include "IIMU.h"
#include "Device_Config.h"
#include <bmi270.h>
#include <Wire.h>

namespace Hardware {

    /**
     * @brief IIMU implementation for the Bosch BMI270 accelerometer/gyroscope.
     *
     * Used on the revised M5Stack ATOM Matrix hardware where the BMI270 replaces
     * the original MPU6886. The IMUFactory probes the I2C bus at runtime to
     * distinguish the two chips and constructs the appropriate subclass.
     *
     * The BMI270 uses the raw Bosch C driver (no Arduino wrapper), so Wire I/O
     * is handled via static callback functions registered with the bmi2_dev struct.
     */
    class BMI270_IMU : public IIMU {
      public:
        /**
         * @brief Construct with I2C pins and clock rate.
         * @param sclPin  I2C clock pin number
         * @param sdaPin  I2C data pin number
         * @param clock   I2C clock frequency in Hz (e.g. 100000L)
         */
        BMI270_IMU( uint8_t sclPin, uint8_t sdaPin, uint32_t clock );

        bool        begin() override;
        Orientation getOrientation() override;
        bool        getRawAcceleration( float &x, float &y, float &z ) override;
        bool        isAvailable() const override;
        const char *getType() const override;

      private:
        struct bmi2_dev dev;
        uint8_t         sclPin;
        uint8_t         sdaPin;
        uint32_t        clockHz;
        bool            initialized;

        // Bosch driver I2C callback functions.
        // intf_ptr is a TwoWire* (set to &Wire during begin()).
        static BMI2_INTF_RETURN_TYPE i2c_read( uint8_t reg_addr, uint8_t *reg_data,
                                               uint32_t len, void *intf_ptr );
        static BMI2_INTF_RETURN_TYPE i2c_write( uint8_t reg_addr, const uint8_t *reg_data,
                                                uint32_t len, void *intf_ptr );
        static void delay_us_cb( uint32_t period, void *intf_ptr );
    };

} // namespace Hardware

#endif // STAC_BMI270_IMU_H

//  --- EOF --- //
