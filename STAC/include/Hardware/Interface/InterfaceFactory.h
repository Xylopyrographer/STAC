#ifndef STAC_INTERFACE_FACTORY_H
#define STAC_INTERFACE_FACTORY_H

#include "GrovePort.h"
#include "PeripheralMode.h"
#include "Device_Config.h"
#include "Config/Constants.h"
#include <memory>


    namespace Hardware {

        /**
         * @brief Factory for creating hardware interface instances
         */
        class InterfaceFactory {
          public:
            /**
             * @brief Create a GROVE port instance
             * @param asOutput true for output mode, false for input mode
             * @return Unique pointer to GrovePort
             */
            static std::unique_ptr<GrovePort> createGrovePort( bool asOutput = true ) {
                auto grove = std::make_unique<GrovePort>(
                                 Config::Pins::TALLY_STATUS_0,
                                 Config::Pins::TALLY_STATUS_1
                             );
                grove->begin( asOutput );
                return grove;
            }

            /**
             * @brief Create a peripheral mode detector
             * @return Unique pointer to PeripheralMode
             */
            static std::unique_ptr<PeripheralMode> createPeripheralDetector() {
                auto detector = std::make_unique<PeripheralMode>(
                                    Config::Pins::PM_CHECK_OUT,
                                    Config::Pins::PM_CHECK_IN,
                                    Config::Peripheral::PM_CHECK_COUNT
                                );
                detector->begin();
                return detector;
            }
        };

    } // namespace Hardware


#endif // STAC_INTERFACE_FACTORY_H


//  --- EOF --- //
