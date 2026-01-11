#ifndef STAC_QMI8658_IMU_H
#define STAC_QMI8658_IMU_H

#include "IIMU.h"
#include <SensorQMI8658.hpp>
#include <Wire.h>


    namespace Hardware {

        /**
         * @brief QMI8658 IMU implementation for Waveshare ESP32-S3-Matrix
         *
         * Implements IIMU interface for the QMI8658 6-axis IMU.
         * Uses I2C communication to detect device orientation.
         */
        class QMI8658_IMU : public IIMU {
          public:
            /**
             * @brief Construct a new QMI8658_IMU object
             * @param sclPin I2C SCL pin
             * @param sdaPin I2C SDA pin
             * @param address I2C device address
             */
            QMI8658_IMU( uint8_t sclPin, uint8_t sdaPin, uint8_t address );

            ~QMI8658_IMU() override = default;

            // IIMU interface implementation
            bool begin() override;
            Orientation getOrientation() override;
            bool getRawAcceleration(float &accX, float &accY, float &accZ) override;
            bool isAvailable() const override;
            const char *getType() const override;

          private:
            SensorQMI8658 sensor;
            uint8_t sclPin;
            uint8_t sdaPin;
            uint8_t address;
            bool initialized;

            // Accelerometer thresholds for orientation detection
            static constexpr float LOW_TOL = 100.0f;
            static constexpr float HIGH_TOL = 900.0f;
            static constexpr float MID_TOL = LOW_TOL + ( HIGH_TOL - LOW_TOL ) / 2.0f;
            static constexpr float ACCL_SCALE = 1000.0f;
            static constexpr uint32_t DATA_WAIT_TIMEOUT_MS = 1000;
        };

    } // namespace Hardware


#endif // STAC_QMI8658_IMU_H


//  --- EOF --- //
