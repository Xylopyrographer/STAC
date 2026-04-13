#include "Network/Protocol/ATEMClient.h"
#include <Arduino.h>


namespace Net {

    bool ATEMClient::begin( const RolandConfig& config ) {
        // Store 0-based input index (UI presents 1-based channel numbers)
        _inputIndex = ( config.tallyChannel > 0 ) ? ( config.tallyChannel - 1 ) : 0;

        // Initialise the ATEM library — sets IP and picks a random local UDP port
        _atemSwitcher.begin( config.switchIP );

        // Send the initial hello packet (connect() is also called automatically
        // on the first runLoop() if neverConnected, but calling it here ensures
        // the packet goes out immediately)
        _atemSwitcher.connect();

        _initialized = true;

        log_i( "ATEMClient: begin() — IP=%s  input_index=%d",
               config.switchIP.toString().c_str(), _inputIndex );

        return true;
    }

    bool ATEMClient::queryTallyStatus( TallyQueryResult& result ) {
        if ( !_initialized ) {
            result.status    = TallyStatus::NOT_INITIALIZED;
            result.connected = false;
            return true;
        }

        // Drive the ATEM UDP state machine — non-blocking (0 ms delay)
        _atemSwitcher.runLoop( 0 );

        // Connection rejected (switcher client slots full)
        if ( _atemSwitcher.isRejected() ) {
            log_e( "ATEMClient: connection rejected — switcher client slots full" );
            result.status    = TallyStatus::NO_CONNECTION;
            result.connected = false;
            result.gotReply  = false;
            return true;
        }

        // Not yet connected (hello-packet handshake in progress)
        if ( !_atemSwitcher.isConnected() ) {
            result.status    = TallyStatus::NO_CONNECTION;
            result.connected = false;
            result.gotReply  = false;
            return true;
        }

        // Connected but initial state payload still arriving
        if ( !_atemSwitcher.hasInitialized() ) {
            result.status    = TallyStatus::NOT_INITIALIZED;
            result.connected = true;
            result.gotReply  = false;
            return true;
        }

        // Fully connected and initialised — read tally flags
        uint8_t flags = _atemSwitcher.getTallyByIndexTallyFlags( _inputIndex );

        if ( flags & 0x01 ) {
            result.status = TallyStatus::ONAIR;
        }
        else if ( flags & 0x02 ) {
            result.status = TallyStatus::SELECTED;
        }
        else {
            result.status = TallyStatus::UNSELECTED;
        }

        result.connected = true;
        result.gotReply  = true;
        result.timedOut  = false;

        return true;
    }

    void ATEMClient::end() {
        // ATEMmin has no explicit disconnect or close; the UDP socket lifetime
        // is managed by the ATEMmin object itself.
        _initialized = false;
        log_i( "ATEMClient: end()" );
    }

    bool ATEMClient::isInitialized() const {
        return _initialized;
    }

} // namespace Net


//  --- EOF --- //
