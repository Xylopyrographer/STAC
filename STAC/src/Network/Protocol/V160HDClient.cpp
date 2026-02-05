#include "Network/Protocol/V160HDClient.h"


namespace Net {

    V160HDClient::V160HDClient()
        : RolandClientBase() {
    }

    V160HDClient::~V160HDClient() {
        end();
    }

    bool V160HDClient::queryTallyStatus( TallyQueryResult& result ) {
        // Initialize result to default state
        result = TallyQueryResult();

        if ( !initialized ) {
            result.status = TallyStatus::NOT_INITIALIZED;
            return false;
        }

        // Build the request URL
        String requestURL = buildRequestURL();
        String fullURL = "http://" + config.switchIP.toString() + ":" + String( config.switchPort ) + requestURL;

        // Begin HTTP request
        if ( !httpClient.begin( fullURL ) ) {
            result.status = TallyStatus::NO_CONNECTION;
            return false;
        }

        // Configure HTTP client
        httpClient.setReuse( true );  // Enable keep-alive
        httpClient.setTimeout( 1000 );  // 1 second timeout for fast error recovery

        // Set authentication credentials
        if ( config.username.length() > 0 ) {
            httpClient.setAuthorization( config.username.c_str(), config.password.c_str() );
        }

        // Set User-Agent to STAC ID
        if ( config.stacID.length() > 0 ) {
            httpClient.setUserAgent( config.stacID );
        }

        // Perform GET request
        int httpCode = httpClient.GET();

        // Check return code
        if ( httpCode > 0 ) {
            // Got some response from server (even if error code)
            result.connected = true;
            result.timedOut = false;

            if ( httpCode == HTTP_CODE_OK ) {
                // Success - got valid HTTP 200 response
                String response = httpClient.getString();

                // Trim and store raw response
                response.trim();
                result.rawResponse = response;
                result.gotReply = true;

                // Handle special cases (empty response, "None" quirk)
                if ( handleSpecialCases( response, result ) ) {
                    httpClient.end();
                    return false;
                }

                // Parse the response (with length check for V-160HD)
                result.status = parseResponse( response, true );
                httpClient.end();
                return true;
            }
            else if ( httpCode == 401 ) {
                // Authentication failed - got reply but auth error
                result.rawResponse = httpClient.getString();
                result.gotReply = false;  // Not a valid tally reply
                result.status = TallyStatus::AUTH_FAILED;
                httpClient.end();
                return false;
            }
            else {
                // Other HTTP error (4xx, 5xx) - connected but not valid tally reply
                result.rawResponse = httpClient.getString();
                result.gotReply = false;  // Not a valid tally reply
                result.status = TallyStatus::NO_REPLY;
                httpClient.end();
                return false;
            }
        }
        else {
            // HTTP request failed (httpCode <= 0)
            // Distinguish between connection refused (switch offline) vs timeout (congestion)

            if ( httpCode == HTTPC_ERROR_CONNECTION_REFUSED ) {
                // Connection actively refused - switch is offline/unreachable
                // Show orange X immediately (don't accumulate)
                result.connected = false;
                result.timedOut = true;
                result.gotReply = false;
                result.status = TallyStatus::NO_CONNECTION;
                httpClient.end();
                return false;
            }
            else {
                // Timeout or other error - likely network congestion
                // Treat as "connected but no response" to allow error accumulation
                result.connected = true;  // WiFi is up, we attempted connection
                result.timedOut = true;   // But the HTTP request timed out
                result.gotReply = false;  // No valid reply received
                result.status = TallyStatus::NO_CONNECTION;
                httpClient.end();
                return false;
            }
        }
    }

    String V160HDClient::buildRequestURL() const {
        // Build URL: /tally/{bank}{channel}/status
        String url = "/tally/";
        url += config.channelBank;
        url += getBankChannel();
        url += "/status";
        return url;
    }

    uint8_t V160HDClient::getBankChannel() const {
        // Map channel to bank channel (1-8)
        if ( config.tallyChannel < 9 ) {
            return config.tallyChannel;
        }
        else {
            return config.tallyChannel - 8;
        }
    }

    void V160HDClient::end() {
        httpClient.end();
        RolandClientBase::end();
    }

    String V160HDClient::getSwitchType() const {
        return "V-160HD";
    }

} // namespace Net


//  --- EOF --- //
