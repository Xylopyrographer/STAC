#ifndef STAC_MPU6886_IMU_H
#define STAC_MPU6886_IMU_H

#include "IIMU.h"
#include "Device_Config.h"
#include <I2C_MPU6886.h>
#include <Wire.h>


namespace Hardware {

    /**
     * @brief MPU6886 IMU implementation for M5Stack ATOM Matrix
     *
     * Implements IIMU interface for the MPU6886 6-axis IMU.
     * Uses I2C communication to detect device orientation.
     */
    class MPU6886_IMU : public IIMU {
      public:
        /**
         * @brief Construct a new MPU6886_IMU object
         * @param sclPin I2C SCL pin
         * @param sdaPin I2C SDA pin
         * @param clock I2C clock frequency
         */
        MPU6886_IMU( uint8_t sclPin, uint8_t sdaPin, uint32_t clock );

        ~MPU6886_IMU() override = default;

        // IIMU interface implementation
        bool begin() override;
        Orientation getOrientation() override;
        bool getRawAcceleration( float &accX, float &accY, float &accZ ) override;
        bool isAvailable() const override;
        const char *getType() const override;

      private:
        I2C_MPU6886 sensor;
        uint8_t sclPin;
        uint8_t sdaPin;
        uint32_t clock;
        bool initialized;

        static constexpr uint32_t DATA_WAIT_TIMEOUT_MS = 1000;
    };

} // namespace Hardware


#endif // STAC_MPU6886_IMU_H


//  --- EOF --- //
