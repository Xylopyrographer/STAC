#ifndef STAC_IIMU_H
#define STAC_IIMU_H

#include "Config/Types.h"


    namespace Hardware {

        /**
         * @brief Abstract interface for Inertial Measurement Units (IMU)
         *
         * Defines the contract that all IMU implementations must follow.
         * This allows for different IMU chips (MPU6886, QMI8658, etc.)
         * and boards without IMU.
         */
        class IIMU {
          public:
            virtual ~IIMU() = default;

            /**
             * @brief Initialize the IMU hardware
             * @return true if initialization succeeded
             */
            virtual bool begin() = 0;

            /**
             * @brief Get the current device orientation
             * @return Orientation enum value
             */
            virtual Orientation getOrientation() = 0;

            /**
             * @brief Get raw accelerometer readings
             * @param accX Output: X-axis acceleration (g)
             * @param accY Output: Y-axis acceleration (g)
             * @param accZ Output: Z-axis acceleration (g)
             * @return true if reading succeeded
             */
            virtual bool getRawAcceleration(float &accX, float &accY, float &accZ) = 0;

            /**
             * @brief Check if IMU is available and working
             * @return true if IMU is functional
             */
            virtual bool isAvailable() const = 0;

            /**
             * @brief Get IMU type as string
             * @return IMU type description
             */
            virtual const char *getType() const = 0;

          protected:
            /**
             * @brief Apply orientation offset correction
             * @param raw Raw orientation from sensor
             * @param offset Offset to apply
             * @return Corrected orientation
             */
            static Orientation applyOrientationOffset( Orientation raw, OrientationOffset offset ) {
                // Don't rotate FLAT or UNKNOWN
                if ( raw == Orientation::FLAT || raw == Orientation::UNKNOWN ) {
                    return raw;
                }

                // Map orientations to rotation indices (ROTATE_0=0, ROTATE_90=1, ROTATE_180=2, ROTATE_270=3)
                // Enum is already ordered this way, so can cast directly
                int rawIdx = static_cast<int>( raw );

                // Apply offset (subtract to correct sensor mounting)
                // OFFSET represents how much the sensor coordinate system is rotated from device coordinates
                // If sensor reads 90° when device is at 0°, OFFSET_90 means subtract 90° to get true device position
                int correctedIdx = ( rawIdx - static_cast<int>( offset ) + 4 ) % 4;

                // Map back to orientation
                return static_cast<Orientation>( correctedIdx );
            }
        };  // ← This is the closing brace of the class

    } // namespace Hardware


#endif // STAC_IIMU_H


//  --- EOF --- //
