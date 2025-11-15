#include "Network/Protocol/V160HDClient.h"

namespace STAC {
    namespace Network {

        V160HDClient::V160HDClient()
            : initialized( false ) {
        }

        V160HDClient::~V160HDClient() {
            end();
        }

        bool V160HDClient::begin( const RolandConfig& cfg ) {
            config = cfg;
            initialized = true;
            return true;
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
                result.connected = true;
                result.timedOut = false;

                if ( httpCode == HTTP_CODE_OK ) {
                    // Success - got valid response
                    String response = httpClient.getString();
                    result.rawResponse = response;
                    result.gotReply = true;
                    result.status = parseResponse( response );
                    httpClient.end();
                    return true;
                }
                else if ( httpCode == 401 ) {
                    // Authentication failed
                    result.rawResponse = httpClient.getString();
                    result.gotReply = true;
                    result.status = TallyStatus::AUTH_FAILED;
                    httpClient.end();
                    return false;
                }
                else {
                    // Other HTTP error
                    result.rawResponse = httpClient.getString();
                    result.gotReply = true;
                    result.status = TallyStatus::INVALID_REPLY;
                    httpClient.end();
                    return false;
                }
            }
            else {
                // HTTP request failed (connection or other error)
                result.connected = false;
                result.timedOut = true;
                result.status = TallyStatus::NO_CONNECTION;
                httpClient.end();
                return false;
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

        TallyStatus V160HDClient::parseResponse( const String &response ) const {
            // Trim response
            String trimmedResponse = response;
            trimmedResponse.trim();

            if ( trimmedResponse == "onair" ) {
                return TallyStatus::ONAIR;
            }
            else if ( trimmedResponse == "selected" ) {
                return TallyStatus::SELECTED;
            }
            else if ( trimmedResponse == "unselected" ) {
                return TallyStatus::UNSELECTED;
            }
            else {
                return TallyStatus::INVALID_REPLY;
            }
        }

        void V160HDClient::end() {
            httpClient.end();
            initialized = false;
        }

        bool V160HDClient::isInitialized() const {
            return initialized;
        }

        String V160HDClient::getSwitchType() const {
            return "V-160HD";
        }

    } // namespace Network
} // namespace STAC


//  --- EOF --- //
