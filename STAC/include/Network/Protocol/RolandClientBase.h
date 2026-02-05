#ifndef STAC_ROLAND_CLIENT_BASE_H
#define STAC_ROLAND_CLIENT_BASE_H

#include "IRolandClient.h"


namespace Net {

    /**
     * @brief Base class for Roland video switch clients
     *
     * Implements common functionality shared between V-60HD and V-160HD clients.
     * Derived classes only need to implement protocol-specific communication.
     */
    class RolandClientBase : public IRolandClient {
      protected:
        RolandConfig config;       ///< Switch configuration
        bool initialized;          ///< Initialization state

        /**
         * @brief Constructor for derived classes
         */
        RolandClientBase();

        /**
         * @brief Parse tally response string to TallyStatus
         * @param response Response string from switch (trimmed)
         * @param checkLength If true, validate response length (max 12 chars)
         * @return Parsed TallyStatus
         */
        TallyStatus parseResponse( const String &response, bool checkLength = false ) const;

        /**
         * @brief Check for special case responses (empty, "None")
         * @param response Response string to check
         * @param result TallyQueryResult to update
         * @return true if special case detected (result updated), false otherwise
         */
        bool handleSpecialCases( const String &response, TallyQueryResult& result ) const;

      public:
        virtual ~RolandClientBase() = default;

        // Implemented common methods
        bool begin( const RolandConfig& config ) override;
        void end() override;
        bool isInitialized() const override;

        // Protocol-specific methods remain pure virtual
        // queryTallyStatus() - must be implemented by derived classes
        // getSwitchType() - must be implemented by derived classes
    };

} // namespace Net


#endif // STAC_ROLAND_CLIENT_BASE_H


//  --- EOF --- //
