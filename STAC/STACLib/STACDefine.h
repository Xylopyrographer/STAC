
// STACDefine.h

#define ID_PREFIX "STAC"        // prefix to use for naming the STAC AP SSID & STA hostname

#define NOM_PREFS_VERSION 4     // version of the normal operating mode (NOM) Preferences information layout in NVS
#define PM_PREFS_VERSION  2     // version of the peripheral operating mode (PM) Preferences information layout in NVS

// defines for the Atom Matrix hardware
#define LED_TYPE    LED_STRIP_WS2812    // the type of RGB LED of the ATOM display
#define DIS_DATA    27      // GPIO pin connected to the display data line - output
#define PO_PIXEL    12      // the pixel position # of the display to use as the Power On indicator
#define DIS_BUTTON  39      // GPIO pin attached to the display button - input
#define DB_TIME     25UL    // display button debounce time (ms)
#define I_SCL       21      // GPIO pin for SCL for IMU - output
#define I_SDA       25      // GPIO pin for SDA for IMU - I/O
#define PM_CK_OUT   22      // GPIO pin # used for ouput to check if the Peripheral Mode jumper is installed
#define PM_CK_IN    33      // GPIO pin # used for input to check if the Peripheral Mode jumper is installed
#define PM_CK_CNT    5      // # of times to toggle PM_CK_OUT to test if the Peripheral Mode jumper is installed
#define TS_0        32      // GROVE connector GPIO pin # to send/receive tally status - output in NOM, input in PM
#define TS_1        26      // GROVE connector GPIO pin # to send/receive tally status - output in NOM, input in PM


// a bunch of timing defines
#define AS_PULSE_TIME           1000UL      // autostart blink pixels period in ms
#define AS_TIMEOUT             20000UL      // autostart timeout in ms
#define GUI_PAUSE_TIME          1500UL      // pause # of ms for the user to see the display
#define GUI_PAUSE_SHORT          500UL      // short pause # of ms for the user to see the display
#define NEXT_STATE_TIME          750UL      // btn down for this # of ms on reset = move to next reset mode
#define OP_MODE_TIMEOUT        30000UL      // # of ms to wait before timing out on an op mode change

// const unsigned long PM_POLL_INT = 2UL;      // # of ms between tally status checks when operating in Peripheral Mode

#define PM_POLL_INT             2UL         // # of ms between checking for tally change when operating in peripheral mode
#define SELECT_TIME             1500UL      // if button down for this # of ms, change the parameter value (operating modes & brightness)
#define WIFI_CONNECT_TIMEOUT    60000UL     // # of ms to wait for a WiFi connection

// defines for the Smart Tally communications
#define ERROR_REPOLL_TIME   50UL        // # of ms to wait before re-polling the switch after a poll error
#define MAX_POLL_ERRORS     8           // # of consecutive switch polling errors before displaying a fault

// convenience defines for Preferences namespaces
#define PREFS_RO true   // NVS Preferences namespace is Read Only if true
#define PREFS_RW false  // NVS Preferences namespace is Read-Write if false

// convenience defines for the display drawing routines
#define SHOW    true    // push the display buffer to the display
#define NO_SHOW false   // do not push the display buffer to the display

// GROVE port macros - sends the tally status to the ATOM GROVE GPIO pins
#define GROVE_PGM digitalWrite( TS_1, HIGH ); digitalWrite( TS_0, HIGH )
#define GROVE_PVW digitalWrite( TS_1, HIGH ); digitalWrite( TS_0, LOW )
#define GROVE_NO_SEL digitalWrite( TS_1, LOW ); digitalWrite( TS_0, HIGH )
#define GROVE_UNKNOWN digitalWrite( TS_1, LOW ); digitalWrite( TS_0, LOW )

// enum and struct type definitions
enum orientation_t { UP = 0, DOWN, LEFT, RIGHT, FLAT, UNKNOWN };    // for the IMU to set orientation of the STAC

enum pvMode_t { UNDEFINED = 0, CFG_PEND, FR_PEND, DFU_PEND };       // for the STACPRovision.h state machine

struct stac_ops_t {             /* holds the operating parameters for the STAC */
    String tModel;              /* model name of the Roland switch */
    uint8_t tChannel;           /* tally channel being monitored */
    uint8_t tChanMax;           /* maximun channel of the switch that can be queried (V-60HD) */
    String tChanBank;           /* channel bank being monitored (V-160HD) */
    uint8_t tChanHDMIMax;       /* maximun HDMI channel that can be queried (V-160HD) */
    uint8_t tChanSDIMax;        /* maximun SDI channel that can be queried (V-160HD) */
    bool autoStart;             /* true to bypass the normal "click through to confirm start" sequence */
    bool ctMode;                /* "camera operator" or "talent" mode - true for camera operator, false for talent*/
    uint8_t disLevel;           /* current operating brightness level - index into the display brightness map */
    unsigned long stsPollInt;   /* the switch is polled every this many ms for a tally status change. */
};

struct tState_t {               /* holds the video switch info */
    bool tConnect;              /* true iff there is a connection to the switch */
    bool tTimeout;              /* true iff an attempt to connect to the switch timed out */
    bool tNoReply;              /* true iff (connected to the switch) AND (a GET request was sent)
                                     AND (the response was not received within the timeout period) */
    bool junkReply;             /* true iff we received a reply from the switch but it was garbage */
    uint8_t tJunkCount;         /* junk reply error accumulator */
    uint8_t tNoReplyCount;      /* "no reply from the switch" error accumulator */
    String tLanUID;             /* switch LAN control user ID (V-160HD) */
    String tLanPW;              /* switch LAN control password (V-160HD) */
    String lastTallyState;      /* current tally state */
    String tState;              /* the state of the channel being monitored as returned by the querry to the switch */
};

struct wifi_info_t {            /* holds the WiFi network & connections status info */
    String stacID;              /* the unique STAC ID used for the AP SSID & STA hostname */
    char networkSSID[33];       /* WiFi SSID. Configured via the user's browser; max length: 32 char */
    char networkPass[64];       /* WiFi password. Configured via the user's browser; max length: 63 char */
    IPAddress stIP;             /* IP address of the switch. Configured via the user's web browser */
    uint16_t stPort;            /* HTTP port of the switch */
    bool wfconnect;             /* true iff we are connected to the WiFi network */
    bool timeout;               /* true iff it we timed out trying to establish a WiFi connection */
};

struct prov_info_t {            /* holds the provisioning data from the STAC config routines as set by a web browser */
    String pModel;              /* Roland switch model */
    String pSSID;               /* SSID of the WiFi network that the switch is connected to */
    String pPass;               /* password for the above WiFi network */
    String pSwitchIP;           /* IP address of the switch */
    uint16_t pPort;             /* port # of the switch */
    String pLanUID;             /* switch LAN user ID (V-160HD) */
    String pLanPW;              /* switch LAN password (V-160HD) */
    uint8_t ptChanMax;          /* maximun tally channel of the switch that can be queried (V-60HD) */
    uint8_t ptChanHDMIMax;      /* maximun HDMI tally channel of the switch that can be queried (V-160HD) */
    uint8_t ptChanSDIMax;       /* maximun SDI tally channel of the switch that can be queried (V-160HD) */
    unsigned long pPollInt;     /* # of ms between tally status polls by the STAC */
};

//  --- EOF ---

