#ifndef STAC_NULL_IMU_H
#define STAC_NULL_IMU_H

#include "IIMU.h"

namespace STAC {
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
                return Orientation::UP;  // Always return UP
            }

            bool isAvailable() const override {
                return false;  // No IMU present
            }

            const char *getType() const override {
                return "None";
            }
        };

    } // namespace Hardware
} // namespace STAC

#endif // STAC_NULL_IMU_H


//  --- EOF --- //
