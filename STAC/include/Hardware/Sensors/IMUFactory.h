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
#elif defined(IMU_TYPE_ATOM_MATRIX)
    #include "MPU6886_IMU.h"
    #include "BMI270_IMU.h"
    #include "NullIMU.h"
    #include <Wire.h>
#elif defined(IMU_TYPE_BMI270)
    #include "BMI270_IMU.h"
    #include "NullIMU.h"
    #include <Wire.h>
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
            #elif defined(IMU_TYPE_ATOM_MATRIX)
            {
                // Probe I2C bus to determine which IMU is physically present.
                // Both MPU6886 and BMI270 share address 0x68.
                //
                // Strategy:
                //   1. Presence check — ping 0x68; no ACK → no IMU, use NullIMU.
                //   2. Read MPU6886 WHO_AM_I (reg 0x75); expect 0x19.
                //   3. Read BMI270 CHIP_ID (reg 0x00); expect 0x24.
                //   4. No match → fall back to NullIMU.
                //
                // IMPORTANT: use endTransmission() (full stop) between write and
                // requestFrom — the ESP32 ng I2C driver times out on repeated-start
                // (endTransmission(false)) on this hardware.
                Wire.begin( Config::Pins::IMU_SDA, Config::Pins::IMU_SCL,
                            static_cast<uint32_t>( IMU_I2C_CLOCK ) );

                // Step 1: presence check — just ping the address
                Wire.beginTransmission( 0x68 );
                if ( Wire.endTransmission() != 0 ) {
                    log_w( "IMU probe: no device found at I2C address 0x68 — using NullIMU" );
                    return std::make_unique<NullIMU>();
                }
                log_d( "IMU probe: device present at 0x68, identifying..." );

                // Step 2: MPU6886 — WHO_AM_I register 0x75 → 0x19
                Wire.beginTransmission( 0x68 );
                Wire.write( 0x75 );
                Wire.endTransmission();
                Wire.requestFrom( static_cast<uint8_t>( 0x68 ),
                                  static_cast<uint8_t>( 1 ) );
                if ( Wire.available() ) {
                    uint8_t whoami = Wire.read();
                    if ( whoami == 0x19 ) {
                        log_i( "IMU probe: MPU6886 detected (WHO_AM_I=0x%02X)", whoami );
                        return std::make_unique<MPU6886_IMU>(
                                   Config::Pins::IMU_SCL,
                                   Config::Pins::IMU_SDA,
                                   IMU_I2C_CLOCK
                               );
                    }
                    log_d( "IMU probe: reg 0x75 = 0x%02X (not MPU6886)", whoami );
                }

                // Step 3: BMI270 — CHIP_ID register 0x00 → 0x24
                Wire.beginTransmission( 0x68 );
                Wire.write( 0x00 );
                Wire.endTransmission();
                Wire.requestFrom( static_cast<uint8_t>( 0x68 ),
                                  static_cast<uint8_t>( 1 ) );
                if ( Wire.available() ) {
                    uint8_t chipid = Wire.read();
                    if ( chipid == BMI270_CHIP_ID ) {
                        log_i( "IMU probe: BMI270 detected (CHIP_ID=0x%02X)", chipid );
                        return std::make_unique<BMI270_IMU>(
                                   Config::Pins::IMU_SCL,
                                   Config::Pins::IMU_SDA,
                                   IMU_I2C_CLOCK
                               );
                    }
                    log_d( "IMU probe: reg 0x00 = 0x%02X (not BMI270)", chipid );
                }

                log_e( "IMU probe: device at 0x68 not recognised — using NullIMU" );
                return std::make_unique<NullIMU>();
            }
            #elif defined(IMU_TYPE_BMI270)
            // ATOM S3R: BMI270 is the only IMU variant — no runtime detection needed.
            // Probe presence before constructing the full driver.
            {
                Wire.begin( Config::Pins::IMU_SDA, Config::Pins::IMU_SCL, IMU_I2C_CLOCK );
                Wire.beginTransmission( 0x68 );
                if ( Wire.endTransmission() != 0 ) {
                    log_w( "BMI270 not found at 0x68 — using NullIMU" );
                    return std::make_unique<NullIMU>();
                }
            }
            return std::make_unique<BMI270_IMU>(
                       Config::Pins::IMU_SCL,
                       Config::Pins::IMU_SDA,
                       IMU_I2C_CLOCK
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
            #elif defined(IMU_TYPE_ATOM_MATRIX)
            return "Auto";
            #elif defined(IMU_TYPE_BMI270)
            return "BMI270";
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
