#include "Network/Protocol/V60HDClient.h"


    namespace Net {

        V60HDClient::V60HDClient()
            : RolandClientBase() {
        }

        V60HDClient::~V60HDClient() {
            end();
        }

        bool V60HDClient::queryTallyStatus( TallyQueryResult& result ) {
            // Initialize result to default state
            result = TallyQueryResult();

            if ( !initialized ) {
                result.status = TallyStatus::NOT_INITIALIZED;
                return false;
            }

            // Connect to switch if not already connected
            if ( !client.connected() ) {
                uint8_t connectionAttempts = 0;
                bool connected = false;

                while ( !connected && connectionAttempts < 1 ) {
                    if ( client.connect( config.switchIP, config.switchPort, CONNECTION_TIMEOUT_MS ) ) {
                        // Clear any stale data from RX buffer
                        if ( client.available() > 0 ) {
                            client.clear();
                        }
                        connected = true;
                        result.connected = true;
                    }
                    else {
                        // Connection failed, wait 1ms before retry
                        connectionAttempts++;
                        delay( 1 );
                    }
                }

                if ( !connected ) {
                    result.status = TallyStatus::NO_CONNECTION;
                    return false;
                }
            }
            else {
                result.connected = true;
            }

            // Set client timeout for response reading
            client.setTimeout( 1 );  // 1 second timeout

            // Send the tally status request
            if ( !sendRequest() ) {
                client.stop();
                result.status = TallyStatus::NO_CONNECTION;
                return false;
            }

            // Read the response
            if ( !readResponse( result ) ) {
                client.stop();
                return false;
            }

            return true;
        }

        bool V60HDClient::sendRequest() {
            // Build request: GET /tally/{channel}/status\r\n\r\n
            client.print( "GET /tally/" );
            client.print( config.tallyChannel );
            client.print( "/status\r\n\r\n" );

            return true;
        }

        bool V60HDClient::readResponse( TallyQueryResult& result ) {
            // Wait for response with timeout
            uint32_t maxLoops = RESPONSE_TIMEOUT_MS;
            while ( client.available() <= 0 && maxLoops > 0 ) {
                maxLoops--;
                delay( 1 );
            }

            if ( maxLoops == 0 ) {
                // Timed out waiting for response
                result.timedOut = true;
                result.status = TallyStatus::TIMEOUT;
                return false;
            }

            // Read response
            String response = "";
            uint8_t responseLength = 0;
            bool breakFlag = false;

            while ( client.available() > 0 && !breakFlag ) {
                char c = (char)client.read();
                response += c;
                responseLength++;

                if ( responseLength >= MAX_RESPONSE_LENGTH ) {
                    // Response too long, invalid
                    result.gotReply = true;
                    result.timedOut = false;
                    result.rawResponse = response;
                    result.status = TallyStatus::INVALID_REPLY;
                    return false;
                }
            }

            // Trim and store raw response
            response.trim();
            result.rawResponse = response;
            result.gotReply = true;
            result.timedOut = false;

            // Handle special cases (empty response, "None" quirk)
            if ( handleSpecialCases( response, result ) ) {
                return false;
            }

            // Parse the response to TallyStatus
            result.status = parseResponse( response );

            return true;
        }

        void V60HDClient::end() {
            if ( client.connected() ) {
                client.stop();
            }
            RolandClientBase::end();
        }

        String V60HDClient::getSwitchType() const {
            return "V-60HD";
        }

    } // namespace Net



//  --- EOF --- //
