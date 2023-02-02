//  STAC (Smart Tally Atom Client)
/*
 *  Version: 2.1
 *  For the M5Stack ATOM MATRIX device
 *
 *  Authors: Team STAC
 *  
 *  A Roland Smart Tally Client
 *     An Arduino sketch designed to run on an ESP32 development board.
 *
 *     Its purpose is to monitor the tally status of a single video input channel 
 *     of a Roland device that implements their Smart Tally protocol.
 *     The sketch uses WiFi to connect to the same network as the Roland device.
 *
 *     For the Roland video input channel being monitored, 
 *     STAC will set the colour of the display:
 *      + when in "Camera Operator" mode, to:
 *          - RED if the channel is in PGM (Progam or onair)
 *          - GREEN if the channel is in PVW (Preview or selected)
 *          - "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected)
 *      + when in "Talent" or "Peripheral" mode, to:
 *          - RED if the channel is in PGM (Progam or onair)
 *          - GREEN otherwise
 *     
 *  Configuration of the STAC for the WiFi credentials and IP address, port number, number of
 *  tally channels and the polling interval of the Roland switch is done using a web browser.
 *
 *  More at: https://github.com/Xylopyrographer/STAC
 *  
*/

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Update.h>
#include <ESPmDNS.h>
#include <Esp.h>                    // to get the ESP-IDF SDK version
#include <esp_arduino_version.h>    // to get the arduino-esp32 core version
#include <Preferences.h>
#include <Wire.h>                   // for using the I2C peripheral
#include <FastLED.h>                // for driving the external matrix display
#include <JC_Button.h>              // for driving the "select" button.
#include <I2C_MPU6886.h>            // for driving the ATOM IMU.

String swVer = "2.1 (GM1)";         // version and (build number) of this software. Shows up on the web  
                                    //   config page, serial monitor startup data dump & is stored in NVS
String idPrefix = "STAC-";          // prefix to use for naming the STAC AP SSID & STA hostname
#define NOM_PREFS_VERSION 3         // version of the normal operating mode (NOM) Preferences information layout in NVS
#define PM_PREFS_VERSION 2          // version of the peripheral operating mode Preferences information layout in NVS

// defines for the Atom Matrix hardware
#define DIS_BUTTON  39      // GPIO pin attached to the display select button - input
#define DB_TIME     25      // display button debounce time (ms)
#define DIS_DATA    27      // GPIO pin connected to the display data line - output
#define I_SCL       21      // GPIO pin for SCL for IMU - output
#define I_SDA       25      // GPIO pin for SDA for IMU - I/O
#define MATRIX_LEDS 25      // # of RGB LED's in the display matrix
#define PM_CK_CNT    5      // # of times to toggle PM_CK_OUT to test if the Peripheral Mode jumper is installed
#define PM_CK_IN    33      // ATOM GPIO pin # used for input to check if the Peripheral Mode jumper is installed
#define PM_CK_OUT   22      // ATOM GPIO pin # used for ouput to check if the Peripheral Mode jumper is installed
#define PO_PIXEL    12      // the pixel position # of the display to use as the Power On indicator
#define TS_0        32      // GPIO pin # to send tally status to an attached device - output
#define TS_1        26      // GPIO pin # to send tally status to an attached device - output
// end defines for the Atom Matrix hardware

// a bunch of timing defines
#define AS_PULSE_TIME         1000UL    // autostart on/off display time in ms
#define AS_TIMEOUT           20000UL    // autostart timeout in ms
#define GUI_PAUSE_TIME        1500UL    // # of ms to pause for the user to see the display
#define NEXT_STATE_TIME        750UL    // btn down for this # of ms on reset = move to next reset mode
#define OP_MODE_TIMEOUT      30000UL    // # of ms to wait before timing out on an op mode change
#define PM_POLL_INT            100UL    // # of ms between checking for tally change when operating in peripheral mode
#define SELECT_TIME           1500UL    // if button down for this # of ms, change the parameter value (operating modes & brightness)
#define WIFI_CONNECT_TIMEOUT 60000UL    // # of ms to wait in the "connect to WiFi" routine without a successful connection before returning

// defines for the Smart Tally communications
#define ERROR_REPOLL_TIME 50UL          // # of ms to wait before re-polling the STS after a poll error
#define MAX_POLL_ERRORS 8               // # of consecutive STS polling errors before changing the display

#define PREFS_RO true                   // NVS Preferences namespace is Read Only if true
#define PREFS_RW false                  // NVS Preferences namespace is Read-Write if false

// Create the objects we need to talk to the LR hardware.
Button dButt( DIS_BUTTON, DB_TIME, 1, true );    // display Button(pin, dbTime, puEnable, invert) make a button object.
I2C_MPU6886 imu(I2C_MPU6886_DEFAULT_ADDRESS, Wire1);    // IMU (default address is 0x68, Wire1 is an I2C bus on user defined pins)
CRGB leds[ MATRIX_LEDS ];               // for FastLED: buffer for the LEDs of the display

// ***** Global Variables *****

WiFiClient stClient;		    // initiate the WiFi library and create a WiFi client object
Preferences stcPrefs;           // holds the operational parameters in NVS for retention across restarts and power cycles.

// get the techie IDE info stuff
String ardesp32Ver = 
    String( ESP_ARDUINO_VERSION_MAJOR ) + "." + 
    String( ESP_ARDUINO_VERSION_MINOR ) + "." +  
    String( ESP_ARDUINO_VERSION_PATCH );            // arduino-esp32 core version
String espidfsdk = ESP.getSdkVersion();             // ESP-IDF SDK version

String stacID;                  // generated in setup(). The unique STAC ID used for the AP SSID & STA hostname
char networkSSID[33]{};         // ST server WiFi SSID. Configured via the user's browser; max length: 32 char
char networkPass[64]{};         // ST server WiFI password. Configured via the user's browser; max length: 63 char
IPAddress stIP;                 // IP address of the ST server. Configured via the user's web browser
uint16_t stPort;                // HTTP port of the Roland Smart Tally server (switcher). Configured
                                //  via the user's web browser; 0 to 65353; default is 80
                                
//  Display brightness mapping table
//  - maps the display brightness level to the 
//    brightness value set when calling FastLED.setBrightness()
//  - first entry must be 0, max size is 10 items, each entry should be greater than the previous
static const uint8_t brightMap[] =  { 0, 10, 20, 30, 40, 50, 60 };  
static const uint8_t brightLevels = sizeof( brightMap ) - 1;

bool ctMode;                    // initialzed in setup(). "Camera Operator" or "Talent" mode. True for camera operator mode, false for talent mode.
bool autoStart;                 // initialzed in setup(). true to bypass the normal "click through to confirm start" sequence.
uint8_t currentBrightness;      // display brightness level. Initialzed in setup()
String lastTallyState;          // tally state before the call to getTallyState().
unsigned long stsPollInt;       // # of ms between polling the Smart Tally server for a tally status change.
unsigned long nextPollTime;     // ms value to when the ST Server is next polled for the tally status.

bool junkReply;                 // true iff we received a reply from the ST server but it was garbage
uint8_t tNoReplyCount;          // STS "no reply from ST server" error accumulator
uint8_t tJunkCount;             // STS junk reply error accumulator

bool Accelerometer = false;     // state to determine if an accelerometer is supported by this hardware.

// Enumeration for orientation postions
enum class ORIENTATION { UP = 0, DOWN, LEFT, RIGHT, FLAT, UNDEFINED };
// Accelerometer parameters
float LOW_TOL = 100.0;
float HIGH_TOL = 900.0;
float MID_TOL = LOW_TOL + ( HIGH_TOL - LOW_TOL ) / 2.0;

// ===== Define data structures used by our various functions =====

struct WfState {
    bool wfconnect;     // true iff we are connected to the WiFi network
    bool timeout;       // true iff it we timed out trying to establish a WiFi connection
} wifiStatus;

struct TState {
    uint8_t tChannel;   // tally channel being monitored
    uint8_t tChanMax;   // highest tally channel number that can be monitored
    bool ctMode;        // "camera operator" or "talent mode". True to select camera operator mode, false to select talent mode.
    bool tConnect;      // true iff there is a connection to the ST Server
    bool tTimeout;      // true iff an attempt to connect to the ST Server timed out
    bool tNoReply;      // true iff (ST Server is connected AND a GET request was sent AND the response was not
                        //  received within the timeout period)
    String tState;      // the state of the tally channel being monitored as returned by the querry to the ST Server
} tallyStatus;

typedef struct WfState WiFiState;       // going to need a function that returns a WfState structure
typedef struct TState TallyState;       // going to need a function that returns a TState structure

struct provision {              // structure that holds the provisioning data from the WiFi config routines
    String pSSID;               // SSID of the WiFi network that the Smart Tally device is connected to
    String pPass;               // password for the above WiFi network
    String pSwitchIP;           // IP address of the Smart Tally device
    uint16_t pPort;             // port # of the Smart Tally device
    uint8_t ptChanMax;          // maximun tally channel of the Smart Tally device that can be queried
    unsigned long pPollInt;     // # of ms between tally status polls by the STAC
};
typedef struct provision provData_t;

// ***** End Global Variables *****

/* ~~~~~~ NVS Items ~~~~~~ */
/* Stuff we're putting into NVS for retention across power cycles. Uses the 'Preferences' library to manage.

  Name space: STCPrefs - used when the STAC is in its normal operating mode
   
   This defines the layout and structure for the NOM_PREFS_VERSION

    NOTE:
        Any change to the table below requires that the NOM_PREFS_VERSION #define above be incremented.
     
    Keys:      
        NVS Key         NVS type    --> app identifier              app type        comment
        -------         --------    --------------------            --------        -------------------------------------------------
        nvsInit         Bool        --> tpInit                      bool            true if the STCPrefs NVS namespace and its key:value pairs have been created
        curBright       UChar       --> currentBrightness           uint8_t         display brightness level when in normal operating state
        talChan         UChar       --> tallyStatus.tChannel        uint8_t         tally channel being monitored.
        talMax          UChar       --> tallyStatus.tMax            uint8_t         max tally channel #
        ctMde           Bool        --> ctMode                      bool            true for "camera operator" mode, false for "talent" mode
        pPoll           ULong       --> stsPollInt                  unsigned long   # of ms between polling the Smart Tally server for a tally status change
        pVis            Bool        --> provisioned                 bool            true if the WiFi credentials in NVS are non-zero
        aStart          Bool        --> autoStart                   bool            true if the STAC should autostart on power up or reset
        stnSSID         String      --> stSSID                      String          SSID of the WiFi network to connect to     
        stnPass         String      --> stPass                      String          password of the WiFi network to connect to
        stswIP          String      --> stIP                        IPAddress       IP address of the Roland Smart Tally device being monitored
        stswPort        UShort      --> stPort                      uint16_t        Port number of the Roland Smart Tally device
        swVersion       String      --> bootVer                     String          The STAC software version it was operating with before its last power down
        prefVer         UShort      --> NOM_PREFS_VERSION           uint16_t        The version number of the NVS Preferences layout the STAC had at boot time.
                                                                                     - Used to decide if we can reuse the existing NVS preferences values after a software update
                                                                                     - Means the user doesn't have to reconfigure the STAC if the NVS layout hasn't changed across software versions.

  Name space: PModePrefs - used when the STAC is operating in Peripheral Mode
  
    Keys:
        NVS Key         NVS type    --> app identifier              app type        comment
        -------         --------    --------------------            --------        -------------------------------------------------
        pmbrightness    UChar       --> pmBright                    uint8_t         display brightness when operating in Peripheral Mode
        pmCtMode        Bool        --> pmCT                        bool            true for "camera operator" mode, false for "talent" mode
        pmPrefVer       UShort      --> PM_PREFS_VERSION            uint16_t        The version number of the NVS Preferences layout the STAC had at boot time.
                                                                                     - Used to decide if we can reuse the existing NVS preferences values after a software update
                                                                                     - Means the user doesn't have to reconfigure the STAC if the NVS layout hasn't changed across software versions.

*/
/*~~~~~ end NVS Items ~~~~~*/

// ***** Function Definitions *****

// The include order is significant
#include "./STACLib/STACGlyph5.h"       // definition of display glyphs and colors for an 5 x 5 display
#include "./STACLib/STACDis5.h"         // routines for manupulating & drawing on an 5 x 5 display
#include "./STACLib/STACUtil.h"         // routines that fire up WiFi AP points for config & updates, a few other utility functions
#include "./STACLib/STACIMU.h"          // creates the glyphs for an 5 x 5 display using the LR version of the IMU hardware
#include "./STACLib/STACSTS.h"          // function to retreive status from the Smart Tally Server (STS)
#include "./STACLib/STACWiFi.h"         // connect to & manage the WiFi in station mode
#include "./STACLib/STACOpModes.h"      // routines to change the run-time operating parameters at startup 

// ***** end Function Definitions *****

// And so it begins...

void setup() {
    
    pinMode(TS_0, OUTPUT);              // set the GROVE GPIO 
    pinMode(TS_1, OUTPUT);              //   pins as outputs
    GROVE_UNKNOWN;                      // set the tally state of the GROVE pins
    
    provData_t sConfigData;             // structure to hold the WiFi provisioning data from user's web browser
    bool provisioned;                   // true if the WiFi provisioning has been done
    String bootVer;                     // the version of software the STAC thinks it has - from NVS storage at boot time
    uint16_t bootPrefs;                 // the version of the normal operating mode preferences layout the STAC thinks it has - from NVS storage at boot time
    bool goodPrefs;                     // true if bootPrefs = NOM_PREFS_VERSION
    bool escapeFlag = false;            // used for getting out of the settings loops.

    // initialize the WiFi state flags
    wifiStatus.wfconnect = false;
    wifiStatus.timeout = false;
    
    // initialize the tally state varialbles and flags
    lastTallyState = "NO_TALLY";
    tallyStatus.tState = "NO_INIT";
    tallyStatus.tConnect = false;
    tallyStatus.tTimeout = true;
    tallyStatus.tNoReply = true;
    
    // initialize the STS error accumulators
    tNoReplyCount = 0;
    tJunkCount = 0;

    dButt.begin();
    Wire1.begin( I_SDA, I_SCL, 100000L );   // enable the I2C bus (SDA pin, SCL pin, Clock frequency)
    FastLED.addLeds<WS2812B, DIS_DATA>( leds, MATRIX_LEDS ); // matrix display of "MATRIX_LEDS" on pin "DIS_DATA" using buffer "leds"
    FastLED.clear();
    
    Serial.begin( 115200 );
    while ( !Serial ) delay( 50 );          // primarily here for the single core USB CDC ESP32 variants
    dButt.read();                           // initialize the Btn class
    
    // make sure button state is stable?                                        // DEBUG CODE
    for ( uint8_t i = 0; i < 3; i++ ) {
       dButt.read();
       delay( DB_TIME * 3 );
    }
    
    disClear( 0 );
    disSetBright( brightMap[ 1 ] );         // set the brightness & refresh the display
    disDrawPix( PO_PIXEL, PO_COLOR, 1 );       // turn on the power LED

    /* Setting up to figure out the orientation of the STAC
       & then create a copy of the glyph matrix rotated to match. */
    ORIENTATION stacOrientation;
    Accelerometer = imu.begin() != -1;
    if ( !Accelerometer ) {
        log_e( "Could not initialize the IMU." );
        stacOrientation = ORIENTATION::UP;          // Set the default orientation of the STAC
    }
    else {
        stacOrientation = getOrientation();         // Go check the orientation of the STAC    
    }
    Wire1.flush();                                  // done with the IMU; release all I2C bus resources, power down peripheral
    rotateGlyphs( stacOrientation ) ;               // create the rotated matrix glyphMap[] in memory
    
    // create the unique STAC ID
    uint32_t chiptID = (uint32_t)( ESP.getEfuseMac() >> 16 );   // grab the top four bytes of the chip id and  
    stacID = idPrefix + String( chiptID, HEX );                 //  use that to create the last part of the unique STAC ID
    stacID.toUpperCase();                                       // flip any hex alphas to upper case

    // THE FOLLOWING THREE HEADER FILES MUST STAY HERE AND IN THIS ORDER
    //    - relative position to each other and the code above and below is important!
    #include "./STACLib/STACInfoHeader.h"           // send the serial port info dump header
    #include "./STACLib/STACPeripheral.h"           // do all the Peripheral Mode checks and run in PM if set
    #include "./STACLib/STACProvision.h"           // do all the checks for provisioning & initialization for Normal Operating Mode
    
    drawGlyph( glyphMap[ tallyStatus.tChannel ], bluecolor, 1 ); // do this here as setting the tally channel is the first thing we do in the user setup stuff.
                                                                 //  - also gives the user some feedback so they can see
                                                                 //    we've transitioned out of doing all the setup bits
    while ( dButt.read() );                                     // wait for button to be released

    bool asBypass = false;                                      // get set up for the auto start detect and control stuff
    if (autoStart) {                                            // if we're in auto start mode...
        asBypass = true;
        pulsePix( true, GRB_AS_PULSE_COLOR );                   // ...turn on the four corner LEDs
        bool nextonstate = false;
        unsigned long asTimeOut =  millis() + AS_TIMEOUT;
        unsigned long nextFlash = millis() + AS_PULSE_TIME;     
        while ( asTimeOut >= millis() && dButt.read() == 0 ) {  // pause until we time out (continue with auto start)
            if ( nextFlash <= millis() ) {                      // have some fun & flash the four corner LED's while waiting :)
                pulsePix( nextonstate, GRB_AS_PULSE_COLOR );
                nextonstate = !nextonstate;
                nextFlash = millis() + AS_PULSE_TIME;
            }
        }
        if ( dButt.read() ) {                                   // unless the button is pressed, in which case...
            asBypass = false;                                   //   ...cancel auto start
        }
    }

    /* Setting up to display & change the current (active) tally channel
     *  
     * If the button is clicked, drop out
     * If the button is long pressed, call changeTallyChannel() and return to the start of
     *  "Setting up to display the current tally channel"
    */
    if ( !asBypass ) {                // skip everything here if autostart kicked in
        escapeFlag = false;
        do  {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor( SELECT_TIME ) ) {
                changeTallyChannel();
                drawGlyph( glyphMap[ tallyStatus.tChannel ], bluecolor, 1 );    // display the (new) current tally channel
            }
        } while ( !escapeFlag );
        dButt.read();
    }

    /* Setting up to display & change the current tally mode
     * 
     * If the button is clicked, drop out.
     * If the button is long pressed, call changeTallyMode() and return 
     *  to the start of "setting up to display the current tally mode"
    */
    if (!asBypass) {                                            // skip everything here if autostart kicked in
        if ( ctMode ) drawGlyph( GLF_C, purplecolor, 1 );       // display the current tally mode...
        else drawGlyph( GLF_T, purplecolor, 1 );
        
        while ( dButt.read() );                                 // wait for button to be released
        escapeFlag = false;
        
        do {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;              // if button was clicked, exit
                }
            if ( dButt.pressedFor( SELECT_TIME ) ) {
                changeTallyMode();
                if (ctMode) drawGlyph( GLF_C, purplecolor, 1 );      // display the (new) current tally mode...
                else drawGlyph( GLF_T, purplecolor, 1 );
            }
        } while ( !escapeFlag );
        dButt.read();
    }

    /* Setting up to display & change the startup mode
     * If the button is clicked, drop out.
     * If the button is long pressed, call changeStartupMode() and return 
     *  to the start of "Setting up to display & change the startup mode"
    */
    if (!asBypass) {                                            // skip everything here if autostart kicked in
        if ( autoStart ) drawGlyph( GLF_A, tealcolor, 1 );      // display the current startup mode
        else drawGlyph( GLF_S, tealcolor, 1 );
        
        while ( dButt.read() );
        escapeFlag = false;
        do  {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor( SELECT_TIME ) ) {
                changeStartupMode();
                if (autoStart) drawGlyph( GLF_A, tealcolor, 1 );    // display the current startup mode              
                else drawGlyph( GLF_S, tealcolor, 1 );
            }
        } while (!escapeFlag);
        dButt.read();
    }

    /* Setting up to display & change the current display brightness level
     * If the button is clicked, drop out.
     * If the button is long pressed, call updateBrightness() and return to
     *  the start of "Setting up to display the current brightness level"
    */
    if (!asBypass) {                                                        // skip everything here if autostart kicked in                  
        drawGlyph(GLF_CBD, brightnessset, 0);                               // draw the checkerboard test pattern...
        drawOverlay(GLF_EN, GRB_COLOR_BLACK, 0);                            // blank out the inside three columns...       
        drawOverlay( glyphMap[ currentBrightness ], GRB_COLOR_WHITE, 1 );   // and overlay the brightness setting number
        
        while ( dButt.read() );
        escapeFlag = false;
        do  {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor( SELECT_TIME ) ) {               
                currentBrightness = updateBrightness( stcPrefs, "STCPrefs", "curBright", currentBrightness );          
                //disFillPix( GRB_COLOR_ORANGE, 0 );
                drawGlyph( GLF_CBD, brightnessset, 0 );                               // draw the checkerboard test pattern...
                drawOverlay( GLF_EN, GRB_COLOR_BLACK, 0 );                            // blank out the inside three columns...       
                drawOverlay( glyphMap[ currentBrightness ], GRB_COLOR_WHITE, 1 );   // overlay the (new) brightness level number
           }
        } while ( !escapeFlag );
        dButt.read();
    }

    disClear();
    disDrawPix( PO_PIXEL, GRB_COLOR_GREEN, 1 );
    delay ( GUI_PAUSE_TIME );           // chill for a second for the sake of the "GUI"
 
    nextPollTime = millis();            // set the initial value for the ST Server poll timer to now

}   // closing brace for setup()

void loop() {

    /* ~~~~~ Update Brightness contol logic ~~~~~ */
    
    dButt.read();       // put this at the top of loop() instead of the bottom so it always gets hit
    if ( dButt.pressedFor( SELECT_TIME ) ) {    // user wants to change the display brightness
        GROVE_UNKNOWN;                          // output the tally state to the GROVE pins
        currentBrightness = updateBrightness(stcPrefs, "STCPrefs", "curBright", currentBrightness);
        if ( !ctMode ) {                        // if in talent mode
            disFillPix( PVW, 0 );               //   set the display to PVW
        }
        else {
            disClear( 0 );
        }
        disDrawPix( PO_PIXEL, PO_COLOR, 1 );

        // reset the tally state varialbles and flags
        lastTallyState = "NO_TALLY";
        tallyStatus.tState = "NO_INIT";
        tallyStatus.tConnect = false;
        tallyStatus.tTimeout = true;
        tallyStatus.tNoReply = true;
        // reset the STS error accumulators
        tNoReplyCount = 0;
        tJunkCount = 0;
        nextPollTime = millis();                // force a repoll of the tally state
    }   // end update Brightness contol logic

    /* ~~~~~ Connect to WiFi control logic ~~~~~ */

    if ( WiFi.status() != WL_CONNECTED ) {
        log_e( "No WiFi connection." );
        
        bool leaveDisplayGreen = false;

        if (wifiStatus.wfconnect) {                 // if we had a previous good WiFi connection...
            GROVE_UNKNOWN;                          // output the tally state to the GROVE pins
            stClient.stop();                        //  stop the stClient
            WiFi.disconnect();                      //  kill the WiFi
            log_e( "WiFi connection lost :(" );

            if (!ctMode) {                          // if in talent mode...
                disFillPix( PVW, 0 );                   // set the display to PVW
                //drawOverlay( GLF_MID, PO_COLOR, 1 );    // overlay the power LEDs
                disDrawPix( PO_PIXEL, PO_COLOR, 1);
                leaveDisplayGreen = true;           //  let the downstream code know to not futz with the display 
            }
            else {
                drawGlyph( GLF_WIFI, alertcolor, 1 );           // let the user know we lost WiFi...
                flashDisplay(8, 300, brightMap[ currentBrightness ] );
                delay( GUI_PAUSE_TIME );
            }
            // reset the WiFi control flags
            wifiStatus.wfconnect = false;
            wifiStatus.timeout = false;
            
            // reset the tally state varialbles and flags
            lastTallyState = "NO_TALLY";
            tallyStatus.tState = "NO_INIT";
            tallyStatus.tConnect = false;
            tallyStatus.tTimeout = true;
            tallyStatus.tNoReply = true;
    
            // reset the STS error accumulators
            tNoReplyCount = 0;
            tJunkCount = 0;
        }

        if ( !leaveDisplayGreen ) drawGlyph( GLF_WIFI, warningcolor, 1 ); // draw the "attempting to connect to WiFi" thing on the display
        while (!wifiStatus.wfconnect) {
            wifiStatus = connectToWifi(wifiStatus);             // connect to the configured WiFi network
            if (wifiStatus.timeout) {
                log_e( "WiFi connect attempt timed out." );
            }
            if (wifiStatus.timeout && !leaveDisplayGreen) {
                drawGlyph( GLF_WIFI, alertcolor, 1 );           // let the user know we timed out trying to connect to WiFi... 
                flashDisplay( 8, 300, brightMap[ currentBrightness ] );
                delay( GUI_PAUSE_TIME );
                drawGlyph( GLF_WIFI, warningcolor, 1 );         // draw the "attempting to connect to WiFi" thing on the display.
            }
        }

        Serial.print( "    WiFi Connected. IP: " );
        Serial.println( WiFi.localIP() );
        Serial.println( "=========================================" );
        log_e( "WiFi connected." );

        if ( !leaveDisplayGreen ) {
            drawGlyph( GLF_WIFI, gtgcolor, 1 );     // draw the "connected to WiFi" thing on the display.
            delay( GUI_PAUSE_TIME );
            disClear();
            disDrawPix( PO_PIXEL, PO_COLOR, 1);
        }
        nextPollTime = millis();                    // force a repoll of the ST server

    }   // end "Connect to WiFi control logic"

    /* ~~~~~ Update Tally Display control logic ~~~~~ */

    if ( millis() >= nextPollTime ) {           // we just did a WiFi check so no need to repeat that here
        nextPollTime = millis() + stsPollInt;

        tallyStatus = getTallyState(tallyStatus);

        // handle the normal operating conditions first...
        if ( tallyStatus.tConnect && !tallyStatus.tNoReply ) {
            // we have a reply from the STS, lets go figure out what to do with it
            nextPollTime = millis() + stsPollInt;
            junkReply = false;
        
            if ( tallyStatus.tState != lastTallyState ) {
                if ( tallyStatus.tState == "onair" ) {
                    GROVE_PGM;                                      //output the tally state to the GROVE pins
                    disFillPix( PGM, 0 );                           // change the display to the PGM colour;
                }
                else if ( tallyStatus.tState == "selected" ) {
                    GROVE_PVW;                                      // output the tally state to the GROVE pins
                    disFillPix( PVW, 0 );                           // change the display to the PVW colour
                }
                else if ( tallyStatus.tState == "unselected" ) {
                    GROVE_NO_SEL;                                   //output the tally state to the GROVE pins
                    if ( ctMode ) {                                 // if in camera operator mode
                        drawGlyph( GLF_DF, unselectedcolor, 0 );    //  change the display to the unselected glyph and colour
                    }
                    else {
                        disFillPix( PVW, 0 );                           // else change the display to the PVW colour
                    }                  
                }
                else {
                    // catchall code block only executed if we get a junk reply from the STS
                    nextPollTime = millis() + ERROR_REPOLL_TIME;     // set the "re-poll on error" time
                    junkReply = true;                               // we got a reply from the ST server, but it's garbage
                    tJunkCount++;                                   // increment the error accumulator
                    lastTallyState = "JUNK";
                    tallyStatus.tState = "NO_TALLY";
                  
                    if ( tJunkCount >= MAX_POLL_ERRORS ) {          // we've hit the error threshold
                        GROVE_UNKNOWN;                              // output the tally state to the GROVE pins
                        if ( ctMode ) {                             // if in camera operator mode                      
                            drawGlyph( GLF_QM, purplecolor, 1 );    //  display unknown response glyph
                        }
                        else {
                            disFillPix( PVW, 0 );                   // else change the display to the PVW colour
                            disDrawPix( PO_PIXEL, PO_COLOR, 1 );
                        }
                        tJunkCount = 0;                               // clear the error accumulator
                    }                   
                }   // closing brace for catchall code block
                
                if ( !junkReply ) {                             // valid status state returned
                    disDrawPix( PO_PIXEL, PO_COLOR, 1 );
                    tNoReplyCount = 0;                          // clear the error accumulators
                    tJunkCount = 0;
                    lastTallyState = tallyStatus.tState;        // save the current tally state
                }
            }   // end "if ( tallyStatus.tState != lastTallyState )"
        }   // end "if ( tallyStatus.tConnect && !tallyStatus.tNoReply )"
        // the block below here handles all the error conditions (except a junk STS reply) reported by the last tally check
        else {
            // error occurred when trying get a tally status update        
            tallyStatus.tState = "NO_INIT";
            lastTallyState = "NO_TALLY";
            tJunkCount = 0;                                     // clear the error accumulator

            if ( !tallyStatus.tConnect && tallyStatus.tTimeout ) {
                // could not connect to the STS & timed out trying
                nextPollTime = millis() + ERROR_REPOLL_TIME;    // set the "re-poll on error" time
                tNoReplyCount = 0;                              // clear the error accumulator
                GROVE_UNKNOWN;                                  // output the tally state to the GROVE pins
                if (ctMode) {
                    drawGlyph( GLF_BX, warningcolor, 1 );       // throw a warning X to the display...
                    log_e( "Threw up an \"OrangeX\"." );
                }
                else {
                    disFillPix( PVW, 0 );                       // else change the display to the PVW colour...
                    disDrawPix( PO_PIXEL, PO_COLOR, 1 );
                }
            }
            else if ( tallyStatus.tConnect && ( tallyStatus.tTimeout || tallyStatus.tNoReply ) ) {
                // we connected to the STS & sent a tally status request but either (the response timed out) or (an empty reply was received)
                nextPollTime = millis() + ERROR_REPOLL_TIME;    // set the "re-poll on error" time 
                tNoReplyCount++;                                // increment the error accumulator                
                if ( tNoReplyCount >= MAX_POLL_ERRORS ) {
                    tNoReplyCount = 0;                          // clear the error accumulator
                    GROVE_UNKNOWN;                              // output the tally state to the GROVE pins
                    if (ctMode) {
                        drawGlyph(GLF_BX, purplecolor, 1 );    // throw up a BPX                   
                    }
                    else {
                        disFillPix( PVW, 0 );                   // else change the display to the PVW colour...
                        disDrawPix( PO_PIXEL, PO_COLOR, 1 );
                    }
                }
            }
            else {
                // some other error condition has occurred
                nextPollTime = millis() + ERROR_REPOLL_TIME;    // set the "re-poll on error" time
                tNoReplyCount = 0;                              // clear the error accumulator
                GROVE_UNKNOWN;                                  // output the tally state to the GROVE pins
                if (ctMode) {
                    drawGlyph( GLF_BX, alertcolor, 1 );         // throw up a big red X
                }
                else {
                    disFillPix( PVW, 0 );                       // else change the display to the PVW colour...
                    disDrawPix( PO_PIXEL, PO_COLOR, 1 );
                }
            }
        }   // end "else error block"
    }   // end if ( millis() >= nextPollTime )

}   // end loop()


// --- EOF ---
