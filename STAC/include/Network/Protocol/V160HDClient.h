#ifndef STAC_V160HD_CLIENT_H
#define STAC_V160HD_CLIENT_H

#include <HTTPClient.h>
#include "IRolandClient.h"

namespace STAC {
    namespace Network {

        /**
         * @brief Roland V-160HD tally client implementation
         *
         * Implements the HTTP-based tally protocol with Basic Authentication used
         * by the Roland V-160HD video switcher. Uses HTTPClient for full HTTP
         * protocol support including authentication headers.
         *
         * Protocol:
         * - GET /tally/{bank}{channel}/status
         * - Response: "onair", "selected", or "unselected"
         * - Requires Basic Authentication
         * - Uses keep-alive connections
         * - Bank-based channels (bankA/bankB)
         *
         * Channel mapping:
         * - Channels 1-8: bankA + channel (1-8)
         * - Channels 9-16: bankB + channel - 8 (1-8)
         */
        class V160HDClient : public IRolandClient {
          public:
            V160HDClient();
            ~V160HDClient() override;

            bool begin( const RolandConfig& config ) override;
            bool queryTallyStatus( TallyQueryResult& result ) override;
            void end() override;
            bool isInitialized() const override;
            String getSwitchType() const override;

          private:
            HTTPClient httpClient;
            RolandConfig config;
            bool initialized;

            /**
             * @brief Parse response string to TallyStatus
             * @param response Response string from switch
             * @return Parsed TallyStatus
             */
            TallyStatus parseResponse( const String &response ) const;

            /**
             * @brief Build the tally request URL
             * @return Complete URL path for tally request
             */
            String buildRequestURL() const;

            /**
             * @brief Get the channel number within the bank (1-8)
             * @return Channel number for bank request
             */
            uint8_t getBankChannel() const;
        };

    } // namespace Network
} // namespace STAC

#endif // STAC_V160HD_CLIENT_H


//  --- EOF --- //
