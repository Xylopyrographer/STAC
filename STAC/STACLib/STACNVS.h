/*
Informational file for the NVS space used by the STAC software.
This file is not used directly by the software, but is provided for reference.

Stuff we're putting into NVS for retention across power cycles. Uses the 'Preferences' library to manage.

=====================================================================================================================================
  Name space: STCPrefs - used when the STAC is in its normal operating mode

    NOTE:
        Any change to the table below requires that the NOM_PREFS_VERSION #define in file "STACDefine.h" be incremented.
     
    Keys:      
        NVS Key         NVS type        app type        comment
        -------         --------        --------        -------------------------------------------------
        aStart          Bool            bool            true if the STAC should autostart on power up or reset
        ctMde           Bool            bool            true for "camera operator" mode, false for "talent" mode
        curBright       UChar           uint8_t         display brightness level when in normal operating state
        lastModel       String          String          the model of the switch the STAC was configured for at last powerup
        pPoll           ULong           unsigned long   # of ms between polling the Smart Tally server for a tally status change
        prefVer         UShort          uint16_t        The version number of the NVS Preferences layout the STAC had at boot time.
        pVis            Bool            bool            true if the WiFi credentials in NVS are non-zero
        stLANpw         String          String          V-160HD LAN password
        stLANuser       String          String          V-160HD LAN user id
        stnPass         String          String          password of the WiFi network to connect to
        stnSSID         String          String          SSID of the WiFi network to connect to     
        stswIP          String          IPAddress       IP address of the Roland Smart Tally device being monitored
        stswPort        UShort          uint16_t        Port number of the Roland Smart Tally device
        swVersion       String          String          The STAC software version it was operating with before its last power down
        talChan         UChar           uint8_t         tally channel being monitored
        talMax          UChar           uint8_t         max tally channel that can be polled (V-60HD)
        talMaxHDMI      UChar           uint8_t         max HDMI tally channel that can be polled (V-160HD)
        talMaxSDI       UChar           uint8_t         max SDI tally channel that can be polled (V-160HD)

=====================================================================================================================================
  Name space: PModePrefs - used when the STAC is operating in Peripheral Mode
  
    NOTE:
        Any change to the table below requires that the PM_PREFS_VERSION #define in file "STACDefine.h" be incremented.

    Keys:
        NVS Key         NVS type        app type        comment
        -------         --------        --------        -------------------------------------------------
        pmbrightness    UChar           uint8_t         display brightness when operating in Peripheral Mode
        pmCtMode        Bool            bool            true for "camera operator" mode, false for "talent" mode
        pmPrefVer       UShort          uint16_t        the version number of the NVS Preferences layout the STAC had at boot time.

*/

// --- EOF ---
