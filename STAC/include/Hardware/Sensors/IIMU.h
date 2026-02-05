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
        virtual bool getRawAcceleration( float &accX, float &accY, float &accZ ) = 0;

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
        // Pattern detection thresholds (used by all IMU implementations)
        static constexpr float ACCL_SCALE = 1000.0f;
        static constexpr float LOW_TOL = 100.0f;
        static constexpr float HIGH_TOL = 900.0f;
        static constexpr float MID_TOL = LOW_TOL + ( HIGH_TOL - LOW_TOL ) / 2.0f;

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

        /**
         * @brief Detect orientation using pattern-based matching
         *
         * Uses empirically-validated pattern matching to determine device orientation.
         * This method is used by all IMU implementations to ensure consistent
         * pattern-to-enum mappings between calibration and runtime.
         *
         * Pattern Mapping (validated on MPU6886 and QMI8658):
         * - Pattern 0: (+1, 0) → ROTATE_180
         * - Pattern 1: (0, -1) → ROTATE_90
         * - Pattern 2: (-1, 0) → ROTATE_0
         * - Pattern 3: (0, +1) → ROTATE_270
         *
         * @param scaledAccX Scaled X-axis acceleration (in milli-g)
         * @param scaledAccY Scaled Y-axis acceleration (in milli-g)
         * @param scaledAccZ Scaled Z-axis acceleration (in milli-g)
         * @return Orientation enum value
         */
        static Orientation detectOrientationFromPattern( float scaledAccX, float scaledAccY, float scaledAccZ ) {
            Orientation rawOrientation = Orientation::UNKNOWN;

            // Check if device is flat first (Z-axis dominant)
            if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) < HIGH_TOL && abs( scaledAccZ ) > MID_TOL ) {
                rawOrientation = Orientation::FLAT;
            }
            // Check if device is vertical with Y-axis dominant
            else if ( abs( scaledAccX ) < HIGH_TOL && abs( scaledAccY ) > MID_TOL && abs( scaledAccZ ) < HIGH_TOL ) {
                // Y-axis dominant - match against patterns
                // Pattern 3: (0, +1), Pattern 1: (0, -1)
                if ( scaledAccY > 0 ) {
                    rawOrientation = Orientation::ROTATE_270;  // Pattern 3
                }
                else {
                    rawOrientation = Orientation::ROTATE_90;   // Pattern 1
                }
            }
            // Check if device is vertical with X-axis dominant
            else if ( abs( scaledAccX ) > MID_TOL && abs( scaledAccY ) < HIGH_TOL && abs( scaledAccZ ) < HIGH_TOL ) {
                // X-axis dominant - match against patterns
                // Pattern 0: (+1, 0), Pattern 2: (-1, 0)
                if ( scaledAccX > 0 ) {
                    rawOrientation = Orientation::ROTATE_180;  // Pattern 0
                }
                else {
                    rawOrientation = Orientation::ROTATE_0;    // Pattern 2
                }
            }

            return rawOrientation;
        }
    };  // ← This is the closing brace of the class

} // namespace Hardware


#endif // STAC_IIMU_H


//  --- EOF --- //
