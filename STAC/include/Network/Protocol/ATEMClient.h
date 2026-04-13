#ifndef STAC_ATEM_CLIENT_H
#define STAC_ATEM_CLIENT_H

/*
 * ATEMClient — wraps the ATEMmin library (AronHetLam's ESP32-compatible fork
 * of the SKAARHOJ ATEMbase/ATEMmin library, GPL v3) to implement IRolandClient.
 *
 * Unlike the Roland HTTP-based clients this client drives the ATEM connection
 * from a persistent UDP state machine.  runLoop(0) is called on every
 * queryTallyStatus() invocation so the caller loop() drives the state machine.
 *
 * begin()             — binds UDP, sends hello packet
 * queryTallyStatus()  — calls runLoop(0), reads tally flags
 * end()               — no-op (UDP socket released when object is destroyed)
 */

#include "IRolandClient.h"
#include <ATEMmin.h>


namespace Net {

    class ATEMClient : public IRolandClient {
      public:
        ATEMClient() = default;
        ~ATEMClient() override = default;

        /**
         * @brief Initialise the ATEM connection.
         *
         * Calls ATEMbase::begin() (sets IP, picks random local port) and then
         * ATEMbase::connect() (sends the initial hello packet to the switcher).
         * The actual protocol handshake completes on the first few runLoop(0)
         * calls inside queryTallyStatus().
         *
         * @param config  Only switchIP and tallyChannel are used.
         *                tallyChannel is 1-based; stored internally as 0-based.
         * @return true always (UDP is connectionless; errors surface later)
         */
        bool begin( const RolandConfig& config ) override;

        /**
         * @brief Run the ATEM state machine and read the current tally.
         *
         * Calls runLoop(0) (non-blocking — drains the UDP receive buffer and
         * sends any pending ACKs) then inspects the tally-by-index flags.
         *
         * result.status values:
         *   NOT_INITIALIZED — runLoop is active but initialisation handshake
         *                     not yet complete (normal for the first ~500 ms)
         *   NO_CONNECTION   — not yet connected or connection was rejected
         *   ONAIR           — tally flag bit 0 set
         *   SELECTED        — tally flag bit 1 set (preview)
         *   UNSELECTED      — connected and initialised, neither flag set
         *
         * @param result  Filled with the query outcome.
         * @return true always
         */
        bool queryTallyStatus( TallyQueryResult& result ) override;

        /**
         * @brief Release resources. No-op for UDP-based client.
         */
        void end() override;

        /**
         * @brief Check if begin() has been called successfully.
         */
        bool isInitialized() const override;

        /**
         * @brief Return human-readable identifier for this client type.
         */
        String getSwitchType() const override { return "ATEM"; }

      private:
        ATEMmin  _atemSwitcher;
        uint16_t _inputIndex   = 0;    ///< 0-based tally input index
        bool     _initialized  = false;
    };

} // namespace Net


#endif // STAC_ATEM_CLIENT_H


//  --- EOF --- //
