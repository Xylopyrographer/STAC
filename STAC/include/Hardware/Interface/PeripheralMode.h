#ifndef STAC_PERIPHERAL_MODE_H
#define STAC_PERIPHERAL_MODE_H

#include <cstdint>


    namespace Hardware {

        /**
         * @brief Peripheral mode detection
         *
         * Detects if a jumper is installed between two pins to enable
         * peripheral mode. Uses a toggle/sense technique to detect connection.
         */
        class PeripheralMode {
          public:
            /**
             * @brief Construct a new Peripheral Mode detector
             * @param outPin GPIO pin for output (toggling)
             * @param inPin GPIO pin for input (sensing)
             * @param toggleCount Number of toggle cycles for detection
             */
            PeripheralMode( uint8_t outPin, uint8_t inPin, uint8_t toggleCount = 5 );

            ~PeripheralMode() = default;

            /**
             * @brief Initialize peripheral mode detection
             * @return true if initialization succeeded
             */
            bool begin();

            /**
             * @brief Detect if peripheral mode jumper is installed
             * @return true if jumper detected (peripheral mode enabled)
             */
            bool detect();

            /**
             * @brief Get last detection result
             * @return true if peripheral mode was detected
             */
            bool isPeripheralMode() const {
                return peripheralModeDetected;
            }

          private:
            uint8_t outPin;                     ///< Output pin for toggling
            uint8_t inPin;                      ///< Input pin for sensing
            uint8_t toggleCount;                ///< Number of toggle cycles
            bool peripheralModeDetected;        ///< Last detection result

            /**
             * @brief Perform a single toggle/sense test
             * @param outputState State to write to output pin
             * @return true if input pin matches output state
             */
            bool testConnection( bool outputState );
        };

    } // namespace Hardware


#endif // STAC_PERIPHERAL_MODE_H


//  --- EOF --- //
