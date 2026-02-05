#ifndef STAC_IROLAND_CLIENT_H
#define STAC_IROLAND_CLIENT_H

#include <Arduino.h>
#include <IPAddress.h>
#include "Config/Types.h"


namespace Net {

    /**
     * @brief Tally status returned from Roland switch
     */
    enum class TallyStatus {
        ONAIR,          ///< Camera is on air (live)
        SELECTED,       ///< Camera is in preview (selected)
        UNSELECTED,     ///< Camera is offline (unselected)
        NO_CONNECTION,  ///< Could not connect to switch
        NO_REPLY,       ///< Connected but no response received
        TIMEOUT,        ///< Request timed out
        INVALID_REPLY,  ///< Received malformed response
        AUTH_FAILED,    ///< Authentication failed (V-160HD)
        NOT_INITIALIZED ///< Client not initialized
    };

    /**
     * @brief Result of a tally status query
     */
    struct TallyQueryResult {
        TallyStatus status;     ///< Status of the query
        bool connected;         ///< Was connection to switch established?
        bool timedOut;          ///< Did the request time out?
        bool gotReply;          ///< Did we receive any reply?
        String rawResponse;     ///< Raw response from switch (for debugging)

        TallyQueryResult()
            : status( TallyStatus::NOT_INITIALIZED )
            , connected( false )
            , timedOut( true )
            , gotReply( false )
            , rawResponse( "" ) {
        }
    };

    /**
     * @brief Configuration for Roland switch connection
     */
    struct RolandConfig {
        IPAddress switchIP;     ///< Switch IP address
        uint16_t switchPort;    ///< Switch port (typically 80 or 8080)
        uint8_t tallyChannel;   ///< Tally channel number (1-based)
        String username;        ///< Username for authentication (V-160HD only)
        String password;        ///< Password for authentication (V-160HD only)
        String channelBank;     ///< Channel bank ("bankA" or "bankB" for V-160HD)
        String stacID;          ///< STAC device ID (used as User-Agent)

        RolandConfig()
            : switchIP( 0, 0, 0, 0 )
            , switchPort( 80 )
            , tallyChannel( 1 )
            , username( "" )
            , password( "" )
            , channelBank( "bankA" )
            , stacID( "" ) {
        }
    };

    /**
     * @brief Interface for Roland video switch tally clients
     *
     * This interface abstracts the differences between Roland V-60HD and V-160HD
     * switches, allowing runtime selection of the appropriate protocol.
     */
    class IRolandClient {
      public:
        virtual ~IRolandClient() = default;

        /**
         * @brief Initialize the Roland client with configuration
         * @param config Roland switch configuration
         * @return true if initialization succeeded
         */
        virtual bool begin( const RolandConfig& config ) = 0;

        /**
         * @brief Query the current tally status from the switch
         * @param result Output parameter containing query results
         * @return true if query completed (check result.status for actual state)
         */
        virtual bool queryTallyStatus( TallyQueryResult& result ) = 0;

        /**
         * @brief Stop the client and release resources
         */
        virtual void end() = 0;

        /**
         * @brief Check if client is initialized
         * @return true if begin() was successful
         */
        virtual bool isInitialized() const = 0;

        /**
         * @brief Get the switch type this client implements
         * @return String identifier ("V-60HD" or "V-160HD")
         */
        virtual String getSwitchType() const = 0;

      protected:
        IRolandClient() = default;
    };

    /**
     * @brief Convert TallyStatus enum to human-readable string
     * @param status TallyStatus value
     * @return String representation
     */
    inline String tallyStatusToString( TallyStatus status ) {
        switch ( status ) {
            case TallyStatus::ONAIR:
                return "onair";
            case TallyStatus::SELECTED:
                return "selected";
            case TallyStatus::UNSELECTED:
                return "unselected";
            case TallyStatus::NO_CONNECTION:
                return "no_connection";
            case TallyStatus::NO_REPLY:
                return "no_reply";
            case TallyStatus::TIMEOUT:
                return "timeout";
            case TallyStatus::INVALID_REPLY:
                return "invalid_reply";
            case TallyStatus::AUTH_FAILED:
                return "auth_failed";
            case TallyStatus::NOT_INITIALIZED:
                return "not_initialized";
            default:
                return "unknown";
        }
    }

} // namespace Net


#endif // STAC_IROLAND_CLIENT_H


//  --- EOF --- //
