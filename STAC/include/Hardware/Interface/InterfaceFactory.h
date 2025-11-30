#ifndef STAC_INTERFACE_FACTORY_H
#define STAC_INTERFACE_FACTORY_H

#include "Device_Config.h"
#include "Config/Constants.h"
#include <memory>

#if HAS_PERIPHERAL_MODE_CAPABILITY
    #include "GrovePort.h"
    #include "PeripheralMode.h"
#endif


    namespace Hardware {

        /**
         * @brief Factory for creating hardware interface instances
         */
        class InterfaceFactory {
          public:
        #if HAS_PERIPHERAL_MODE_CAPABILITY
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
                // M5StickC Plus: GPIO 25 and 26 share the same physical pin on HAT connector
                // Must float GPIO 25 before using GPIO 26 for peripheral mode detection
                #if defined(PIN_PM_FLOAT_FIRST)
                    pinMode(PIN_PM_FLOAT_FIRST, INPUT);  // Set to high-impedance (floating)
                    log_i("Set GPIO %d to floating for peripheral mode detection", PIN_PM_FLOAT_FIRST);
                #endif

                auto detector = std::make_unique<PeripheralMode>(
                                    Config::Pins::PM_CHECK_OUT,
                                    Config::Pins::PM_CHECK_IN,
                                    Config::Peripheral::PM_CHECK_COUNT
                                );
                detector->begin();
                return detector;
            }
        #endif // HAS_PERIPHERAL_MODE_CAPABILITY
        };

    } // namespace Hardware


#endif // STAC_INTERFACE_FACTORY_H


//  --- EOF --- //
