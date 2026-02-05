#ifndef STAC_INTERFACE_FACTORY_H
#define STAC_INTERFACE_FACTORY_H

#include "Device_Config.h"
#include "Config/Constants.h"
#include <memory>

#if HAS_PERIPHERAL_MODE_CAPABILITY
    #include "GrovePort.h"
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


        #endif // HAS_PERIPHERAL_MODE_CAPABILITY
    };

} // namespace Hardware


#endif // STAC_INTERFACE_FACTORY_H


//  --- EOF --- //
