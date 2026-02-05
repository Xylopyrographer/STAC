#include "Network/Protocol/RolandClientBase.h"


namespace Net {

    RolandClientBase::RolandClientBase()
        : initialized( false ) {
    }

    bool RolandClientBase::begin( const RolandConfig& cfg ) {
        config = cfg;
        initialized = true;
        return true;
    }

    void RolandClientBase::end() {
        initialized = false;
    }

    bool RolandClientBase::isInitialized() const {
        return initialized;
    }

    TallyStatus RolandClientBase::parseResponse( const String &response, bool checkLength ) const {
        // Trim response
        String trimmedResponse = response;
        trimmedResponse.trim();

        // Optional length check (used by V-160HD)
        if ( checkLength ) {
            // Valid responses are "onair", "selected", or "unselected"
            // Maximum valid length is 10 ("unselected"), anything longer is junk
            if ( trimmedResponse.length() > 12 || trimmedResponse.length() == 0 ) {
                return TallyStatus::INVALID_REPLY;
            }
        }

        // Parse response
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

    bool RolandClientBase::handleSpecialCases( const String &response, TallyQueryResult& result ) const {
        // Check for empty response
        if ( response.length() == 0 ) {
            result.status = TallyStatus::NO_REPLY;
            return true;
        }

        // Check for Python emulator quirk when it "takes a nap"
        if ( response == "None" ) {
            result.status = TallyStatus::NO_REPLY;
            return true;
        }

        return false;
    }

} // namespace Net


//  --- EOF --- //
