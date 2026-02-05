#ifndef STAC_NULL_IMU_H
#define STAC_NULL_IMU_H

#include "IIMU.h"


namespace Hardware {

    /**
     * @brief Null IMU implementation for boards without IMU
     *
     * Provides a no-op implementation that always returns UP orientation.
     * Used when IMU_HAS_IMU is false in board configuration.
     */
    class NullIMU : public IIMU {
      public:
        NullIMU() = default;
        ~NullIMU() override = default;

        bool begin() override {
            return true;  // Always succeeds
        }

        Orientation getOrientation() override {
            return Orientation::ROTATE_0;  // Always return default (no glyph rotation)
        }

        bool getRawAcceleration( float &accX, float &accY, float &accZ ) override {
            // No IMU, return zero acceleration
            accX = 0.0f;
            accY = 0.0f;
            accZ = 0.0f;
            return false;  // No valid data
        }

        bool isAvailable() const override {
            return false;  // No IMU present
        }

        const char *getType() const override {
            return "None";
        }
    };

} // namespace Hardware


#endif // STAC_NULL_IMU_H


//  --- EOF --- //
