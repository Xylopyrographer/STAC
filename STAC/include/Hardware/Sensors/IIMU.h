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

                // Map orientations to rotation indices (UP=0, RIGHT=1, DOWN=2, LEFT=3)
                int rawIdx = 0;
                switch ( raw ) {
                    case Orientation::UP:
                        rawIdx = 0;
                        break;
                    case Orientation::RIGHT:
                        rawIdx = 1;
                        break;
                    case Orientation::DOWN:
                        rawIdx = 2;
                        break;
                    case Orientation::LEFT:
                        rawIdx = 3;
                        break;
                    default:
                        return raw;
                }

                // Apply offset (rotate clockwise)
                int correctedIdx = ( rawIdx + static_cast<int>( offset ) ) % 4;

                // Map back to orientation
                switch ( correctedIdx ) {
                    case 0:
                        return Orientation::UP;
                    case 1:
                        return Orientation::RIGHT;
                    case 2:
                        return Orientation::DOWN;
                    case 3:
                        return Orientation::LEFT;
                    default:
                        return Orientation::UNKNOWN;
                }
            }
        };  // ← This is the closing brace of the class

    } // namespace Hardware


#endif // STAC_IIMU_H


//  --- EOF --- //
