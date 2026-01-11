#ifndef STAC_TYPES_H
#define STAC_TYPES_H

#include <cstdint>
#include <IPAddress.h>
#include <WString.h>



    // ============================================================================
    // ENUMERATIONS
    // ============================================================================

    /**
     * @brief Display orientation - which edge of the display should be "up" for correct character display
     * 
     * Values represent the display rotation needed to show characters upright:
     * - UP (0°): Display upright, no rotation needed
     * - DOWN (180°): Display inverted 180°
     * - LEFT (270°): Display rotated 270° CW (or 90° CCW)
     * - RIGHT (90°): Display rotated 90° CW
     * - FLAT: Device lying flat (face up or down)
     * - UNKNOWN: Unable to determine orientation
     * 
     * Note: This represents display orientation after applying board config offset,
     * not the raw physical device orientation.
     */
    enum class Orientation : uint8_t {
        UP = 0,      // Display upright (0° rotation)
        DOWN,        // Display inverted (180° rotation)
        LEFT,        // Display rotated 270° CW
        RIGHT,       // Display rotated 90° CW
        FLAT,        // Device lying flat
        UNKNOWN      // Unable to determine
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
        // @Claude: switchModel should be an enum instead of a string for better type safety and performance.
        // @Claude: we discussd detangling V-60HD and V-160HD specific parameters. Is this a case where we should consider an alternate implementation?
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

        // Helper methods for switch model type checking
        /**
         * @brief Check if switch is V-60HD model
         * @return true if switchModel is "V-60HD"
         */
        bool isV60HD() const { return switchModel == "V-60HD"; }

        /**
         * @brief Check if switch is V-160HD model
         * @return true if switchModel is "V-160HD"
         */
        bool isV160HD() const { return switchModel == "V-160HD"; }
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
        // @Claude: we discussd detangling V-60HD and V-160HD specific parameters. Is this a case where we should consider an alternate implementation? lan credentials are specific to certain protocols/switch models and happen at the network link level, not the application level.
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
        // @Claude: if we make the switch model an enum, we'll need to change the HTML provisioning page to match.
        // @Claude: is there a way to detangle the V-60 and V-160 specific parameters here? Woud mean a big revamp in the HTML provisioning pages?
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
