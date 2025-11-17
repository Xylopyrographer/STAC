#ifndef STAC_TYPES_H
#define STAC_TYPES_H

#include <cstdint>
#include <IPAddress.h>
#include <WString.h>



    // ============================================================================
    // ENUMERATIONS
    // ============================================================================

    /**
     * @brief Device orientation as detected by IMU
     */
    enum class Orientation : uint8_t {
        UP = 0,
        DOWN,
        LEFT,
        RIGHT,
        FLAT,
        UNKNOWN
    };

    /**
     * @brief IMU orientation offset for correcting sensor mounting
     */
    enum class OrientationOffset : uint8_t {
        OFFSET_0 = 0,      // No rotation
        OFFSET_90 = 1,     // 90° clockwise
        OFFSET_180 = 2,    // 180°
        OFFSET_270 = 3     // 270° clockwise (or 90° counter-clockwise)
    };

    /**
     * @brief Provisioning state machine states
     */
    enum class ProvisionMode : uint8_t {
        UNDEFINED = 0,
        CONFIG_PENDING,
        FACTORY_RESET_PENDING,
        DFU_PENDING
    };

    /**
     * @brief Tally state returned from Roland switch
     */
    enum class TallyState : uint8_t {
        PROGRAM,        // On-air (PGM)
        PREVIEW,        // Selected (PVW)
        UNSELECTED,     // Not selected
        NO_TALLY,       // No valid tally state
        ERROR           // Error state
    };

    /**
     * @brief Operating mode for STAC
     */
    enum class OperatingMode : uint8_t {
        NORMAL,         // Normal WiFi operation
        PERIPHERAL,     // Peripheral mode (wired connection)
        PROVISIONING    // Configuration mode
    };

    // ============================================================================
    // STRUCTURES
    // ============================================================================

    /**
     * @brief Operating parameters for STAC
     */
    struct StacOperations {
        String switchModel;             ///< Roland switch model ("V-60HD" or "V-160HD")
        uint8_t tallyChannel;           ///< Channel being monitored (1-based)
        uint8_t maxChannelCount;        ///< Max channels for V-60HD
        String channelBank;             ///< Channel bank for V-160HD
        uint8_t maxHDMIChannel;         ///< Max HDMI channels for V-160HD
        uint8_t maxSDIChannel;          ///< Max SDI channels for V-160HD
        bool autoStartEnabled;          ///< Auto-start on boot
        bool cameraOperatorMode;        ///< true=Camera Operator, false=Talent
        uint8_t displayBrightnessLevel; ///< Brightness index into brightness map
        unsigned long statusPollInterval; ///< Polling interval in ms

        // Default constructor
        StacOperations()
            : switchModel( "NO_MODEL" )
            , tallyChannel( 1 )
            , maxChannelCount( 6 )
            , channelBank( "NO_BANK" )
            , maxHDMIChannel( 8 )
            , maxSDIChannel( 8 )
            , autoStartEnabled( false )
            , cameraOperatorMode( true )
            , displayBrightnessLevel( 1 )
            , statusPollInterval( 300 )
        {}
    };

    /**
     * @brief Video switch connection state
     */
    struct SwitchState {
        bool connected;                 ///< Connected to switch
        bool timeout;                   ///< Connection attempt timed out
        bool noReply;                   ///< Connected but no response
        bool junkReply;                 ///< Received garbage response
        uint8_t junkReplyCount;         ///< Consecutive junk replies
        uint8_t noReplyCount;           ///< Consecutive no-replies
        String lanUserID;               ///< LAN control user ID (V-160HD)
        String lanPassword;             ///< LAN control password (V-160HD)
        String lastTallyState;          ///< Previous tally state
        String currentTallyState;       ///< Current tally state string

        // Default constructor
        SwitchState()
            : connected( false )
            , timeout( true )
            , noReply( true )
            , junkReply( false )
            , junkReplyCount( 0 )
            , noReplyCount( 0 )
            , lanUserID( "NO_UID" )
            , lanPassword( "NO_PW" )
            , lastTallyState( "NO_INIT" )
            , currentTallyState( "NO_TALLY" )
        {}
    };

    /**
     * @brief WiFi network and connection information
     */
    struct WiFiInfo {
        String stacID;                  ///< Unique STAC ID for AP SSID and hostname
        char networkSSID[ 33 ];         ///< WiFi SSID (max 32 chars + null)
        char networkPassword[ 64 ];     ///< WiFi password (max 63 chars + null)
        IPAddress switchIPAddress;      ///< IP address of Roland switch
        uint16_t switchPort;            ///< HTTP port of switch
        bool wifiConnected;             ///< Connected to WiFi
        bool connectionTimeout;         ///< WiFi connection timed out

        // Default constructor
        WiFiInfo()
            : stacID( "NO_STAC" )
            , networkSSID{0}
            , networkPassword{0}
            , switchIPAddress( 0, 0, 0, 0 )
            , switchPort( 80 )
            , wifiConnected( false )
            , connectionTimeout( false )
        {}
    };

    /**
     * @brief Provisioning data from web configuration
     */
    struct ProvisioningData {
        String switchModel;             ///< Roland switch model
        String wifiSSID;                ///< WiFi network SSID
        String wifiPassword;            ///< WiFi network password
        String switchIPString;          ///< Switch IP as string
        uint16_t switchPort;            ///< Switch port number
        String lanUserID;               ///< LAN control user ID (V-160HD)
        String lanPassword;             ///< LAN control password (V-160HD)
        uint8_t maxChannel;             ///< Max channel (V-60HD)
        uint8_t maxHDMIChannel;         ///< Max HDMI channel (V-160HD)
        uint8_t maxSDIChannel;          ///< Max SDI channel (V-160HD)
        unsigned long pollInterval;     ///< Status polling interval in ms

        // Default constructor
        ProvisioningData()
            : switchModel( "NO_MODEL" )
            , wifiSSID( "" )
            , wifiPassword( "" )
            , switchIPString( "" )
            , switchPort( 80 )
            , lanUserID( "" )
            , lanPassword( "" )
            , maxChannel( 6 )
            , maxHDMIChannel( 8 )
            , maxSDIChannel( 8 )
            , pollInterval( 300 )
        {}
    };



#endif // STAC_TYPES_H

//  --- EOF --- //
