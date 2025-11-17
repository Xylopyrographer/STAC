#ifndef STAC_IMU_FACTORY_H
#define STAC_IMU_FACTORY_H

#include "IIMU.h"
#include "Device_Config.h"
#include "Config/Constants.h"
#include <memory>

// Include the appropriate IMU implementation based on configuration
#if defined(IMU_TYPE_MPU6886)
    #include "MPU6886_IMU.h"
#elif defined(IMU_TYPE_QMI8658)
    #include "QMI8658_IMU.h"
#elif defined(IMU_TYPE_NONE)
    #include "NullIMU.h"
#else
    #include "NullIMU.h"  // Default to NullIMU if nothing defined
#endif


    namespace Hardware {

        /**
         * @brief Factory for creating IMU instances
         *
         * Automatically selects the correct IMU implementation
         * based on the board configuration in Device_Config.h
         */
        class IMUFactory {
          public:
            /**
             * @brief Create an IMU instance for the configured board
             * @return Unique pointer to IIMU implementation
             */
            static std::unique_ptr<IIMU> create() {
#if defined(IMU_TYPE_MPU6886)
                return std::make_unique<MPU6886_IMU>(
                           Config::Pins::IMU_SCL,
                           Config::Pins::IMU_SDA,
                           IMU_I2C_CLOCK
                       );
#elif defined(IMU_TYPE_QMI8658)
                return std::make_unique<QMI8658_IMU>(
                           Config::Pins::IMU_SCL,
                           Config::Pins::IMU_SDA,
                           IMU_I2C_ADDRESS
                       );
#else
                return std::make_unique<NullIMU>();
#endif
            }

            /**
             * @brief Get IMU type name as string
             * @return IMU type description
             */
            static const char *getIMUType() {
#if defined(IMU_TYPE_MPU6886)
                return "MPU6886";
#elif defined(IMU_TYPE_QMI8658)
                return "QMI8658";
#else
                return "None";
#endif
            }

            /**
             * @brief Check if board has IMU
             * @return true if IMU is present
             */
            static bool hasIMU() {
#if defined(IMU_HAS_IMU)
                return IMU_HAS_IMU;
#else
                return false;
#endif
            }
        };

    } // namespace Hardware


#endif // STAC_IMU_FACTORY_H


//  --- EOF --- //
