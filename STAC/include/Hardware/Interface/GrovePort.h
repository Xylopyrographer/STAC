#ifndef STAC_GROVE_PORT_H
#define STAC_GROVE_PORT_H

#include <cstdint>
#include "Config/Types.h"


    namespace Hardware {

        /**
         * @brief GROVE port interface for tally status output
         *
         * Controls two GPIO pins to output tally status:
         * - TS_0 and TS_1 combined create 4 possible states
         * - Used for both normal output and peripheral mode input
         */
        class GrovePort {
          public:
            /**
             * @brief Construct a new Grove Port
             * @param pin0 GPIO pin for TS_0
             * @param pin1 GPIO pin for TS_1
             */
            GrovePort( uint8_t pin0, uint8_t pin1 );

            ~GrovePort() = default;

            /**
             * @brief Initialize the GROVE port
             * @param asOutput true for output mode, false for input mode
             * @return true if initialization succeeded
             */
            bool begin( bool asOutput = true );

            /**
             * @brief Set tally status output
             * @param state Tally state to output
             */
            void setTallyState( TallyState state );

            /**
             * @brief Read tally status input (peripheral mode)
             * @return Current tally state
             */
            TallyState readTallyState();

            /**
             * @brief Set both pins to specific states
             * @param pin0State State for TS_0 (true = HIGH)
             * @param pin1State State for TS_1 (true = HIGH)
             */
            void setPins( bool pin0State, bool pin1State );

            /**
             * @brief Read both pin states
             * @param pin0State Output: state of TS_0
             * @param pin1State Output: state of TS_1
             */
            void readPins( bool& pin0State, bool& pin1State );

          private:
            uint8_t pin0;           ///< TS_0 pin
            uint8_t pin1;           ///< TS_1 pin
            bool isOutputMode;      ///< true = output, false = input
        };

    } // namespace Hardware


#endif // STAC_GROVE_PORT_H


//  --- EOF --- //
