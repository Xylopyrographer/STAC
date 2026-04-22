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

        _initialized      = true;
        _connectStartTime = millis();

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

        // Not yet connected — either initial handshake or reconnect after connection loss.
        if ( !_atemSwitcher.isConnected() ) {
            if ( _everInitialized ) {
                // Previously had a working connection — signal loss so the UI can show an error
                result.status    = TallyStatus::NO_CONNECTION;
                result.connected = false;
                result.gotReply  = false;
                result.timedOut  = true;
            }
            else if ( ( millis() - _connectStartTime ) >= INITIAL_CONNECT_TIMEOUT_MS ) {
                // Never connected and initial timeout elapsed — ATEM is not responding
                result.status    = TallyStatus::NO_CONNECTION;
                result.connected = false;
                result.gotReply  = false;
                result.timedOut  = true;
            }
            else {
                // Initial handshake in progress, still within timeout window — stay silent
                result.status    = TallyStatus::CONNECTING;
                result.connected = false;
                result.gotReply  = false;
                result.timedOut  = false;
            }
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
        _everInitialized = true;   // Record that we have reached a fully-working connection
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
        _initialized      = false;
        _everInitialized  = false;  // Reset so a fresh begin() starts silent again
        _connectStartTime = 0;
        log_i( "ATEMClient: end()" );
    }

    bool ATEMClient::isInitialized() const {
        return _initialized;
    }

} // namespace Net


//  --- EOF --- //
