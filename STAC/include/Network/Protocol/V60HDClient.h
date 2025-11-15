#ifndef STAC_V60HD_CLIENT_H
#define STAC_V60HD_CLIENT_H

#include <WiFi.h>
#include "IRolandClient.h"

namespace STAC {
    namespace Network {

        /**
         * @brief Roland V-60HD tally client implementation
         *
         * Implements the simple HTTP-based tally protocol used by the Roland V-60HD
         * video switcher. Uses WiFiClient for direct TCP/HTTP communication without
         * authentication requirements.
         *
         * Protocol:
         * - GET /tally/{channel}/status\r\n\r\n
         * - Response: "onair", "selected", or "unselected"
         * - No authentication required
         * - Short-form GET (no HTTP headers in response)
         */
        class V60HDClient : public IRolandClient {
          public:
            V60HDClient();
            ~V60HDClient() override;

            bool begin( const RolandConfig& config ) override;
            bool queryTallyStatus( TallyQueryResult& result ) override;
            void end() override;
            bool isInitialized() const override;
            String getSwitchType() const override;

          private:
            WiFiClient client;
            RolandConfig config;
            bool initialized;

            static constexpr uint32_t CONNECTION_TIMEOUT_MS = 1000;  ///< Connection timeout
            static constexpr uint32_t RESPONSE_TIMEOUT_MS = 100;     ///< Response wait timeout
            static constexpr uint8_t MAX_RESPONSE_LENGTH = 12;       ///< Max expected response length

            /**
             * @brief Parse response string to TallyStatus
             * @param response Response string from switch
             * @return Parsed TallyStatus
             */
            TallyStatus parseResponse( const String& response ) const;

            /**
             * @brief Send GET request to switch
             * @return true if request sent successfully
             */
            bool sendRequest();

            /**
             * @brief Read response from switch
             * @param result Output parameter for result
             * @return true if response read successfully
             */
            bool readResponse( TallyQueryResult& result );
        };

    } // namespace Network
} // namespace STAC

#endif // STAC_V60HD_CLIENT_H


//  --- EOF --- //
