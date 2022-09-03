/* STAC (Smart Tally ATOM Matrix Client)
 *
 *  Version: 1.11_ß5
 *      - Integrates 'BPX' filtering.
 *          - additions to getTallyStatus():
 *            - .tStatus now returns error conditions as well as STS replies.
 *          - rewite of the loop() tally display code to better facillitate error handling.
 *      - Adds user configurable polling interval via the web config page.
 *        - adds polling interval as an NVS item.
 *        - increments the NVS NOM_PREFS_VERSION.
 *      - Corrects improper use of the Preferences library.
 *      - No longer uses the M5 Atom libraries:
 *        - uses JC_Button library for the display button;
 *        - uses I2C_MPU6886 library for the IMU;
 *        - uses FastLED directly for the display;
 *        - directly initializes Serial.    
 *      - Replaces "M5.dis.X" functions with bespoke display drawing routines.
 *      - No longer reformats the NVS when doing a factory reset.
 *      - Modified the layout of the startup data dump.
 *        - displays the polling interval.
 *        - items above the "-----" line are hard coded.
 *        - items above the "=-=-=" line are web config items.
 *        - items below the "=-=-=" line are run-time configurable.
 *      - Adds macros for setting the tally state on the GROVE connector.
 *        - have yet to verify this didn't break this for this build.
 *
 *  Version: 1.11_ß6
 *      - Sets WiFi hostname to the STAC id.
 *      - Sets the default polling interval to 300 ms.
 *      - Bug fix. Re-enables outputs to the GROVE pins during normal operating mode.
 *      - Sets the Peripheral Mode polling interval to 0 ms.
 *      - Comments out left over debug code in parseForm().
 *      - Places the base glyph array into flash memory. 
 *      - Places the color pair definitions into flash memory.
 *      - Modifies the drawGlyph() 'colors[]' parameter data type to 'const int'.
 *
 *   Version: 1.11_ß7
 *      - chunking up the sketch into separate .h files.
 *      - no functional changes from ß6
 *      
 *   Version: 1.11_ß8
 *      - removed the buttonClicked() function in favour of native JC_Button lib calls.
 *      - revert the use of PROGMEM and the F() macro in favour of native ESP32 'const' declarations.
 *        - PROGMEM and the F() macro don't do anything on the ESP32. Managed autmaticaly by the build system.
 *      
 *   Version: 1.11_ß9
 *      - compiles and runs under arduino-esp32 core v1.0.6 and v2.0.3-rc1
 *        - when using core v2.0.3 & greater, the WiFi authentication/encryption 
 *          mode of the STAC when it is being provisioned is now WAP2-PSK (AES).
 *      - change 'stIP' to use data type 'IPAddress' instead of 'char[]'.
 *      - fixed the "set" order in STACWiFi.h so setting the hostname works with core 1.0.6 and 2.0.3-rc1.
 *      - fixed a coding error in 'getCreds()' in STACWeb.h that greatly improves error handling.
 *      - added 'fetchInfo()' function to 'STACUtil.h'.
 *      - removed all references to btnWas & btnNow.
 *      - house cleaning.
 *      
 *    Version: 1.11-RC1
 *      - compiles and runs under arduino-esp32 core v1.0.6
 *      - compiles under core v2.0.3 but functionally not equivalent due to incompatible changes made in core v2 series
 *      - all changes noted above are implemented in this version (excepting the "runs under core 2.x" remarks)
 *      - see also the "README_1.11RC1.txt" file
 
 *
 *  2022-06-07
 *  
 *  Authors: Team STAC
 *  
 *  A Roland Smart Tally Client
 *     An Arduino sketch designed to run on an M5Stack ATOM Matrix board.
 *     Its purpose is to monitor the tally status of a single video input channel 
 *     of a Roland device that implements their Smart Tally protocol.
 *     The sketch uses WiFi to connect to the same network as the Roland device.
 *     For the Roland video input channel being monitored, STAC will set
 *     the colour of the display on the ATOM:
 *      + when in "Camera Operator" mode, to:
 *          - RED if the channel is in PGM (Progam or onair)
 *          - GREEN if the channel is in PVW (Preview or selected)
 *          - "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected)
 *      + when in "Talent" or "Peripheral" mode, to:
 *          - RED if the channel is in PGM (Progam or onair)
 *          - GREEN otherwise
 *     
 *  Configuration of the STAC for the WiFi credentials and IP address, port number  
 *  and number of tally channels of the Roland switch is done using a web browser.
 *
 *  More at: https://github.com/Xylopyrographer/STAC
 *  
*/


String swVer = ("1.11-RC1");      // version of this software. Shows up on the web config page & is stored in NVS
String idPrefix = ("STAC-");      // prefix to use for naming the STAC AP SSID & STA hostname
#define NOM_PREFS_VERSION 2       // version of the normal operating mode (NOM) Preferences information layout in NVS

#define DB_MODE 0                 // define to 1 for debug statements to be printed to the serial monitor, 0 otherwise

#include <WiFi.h>
#include <Preferences.h>
#include <Wire.h> 
#include <FastLED.h>            // for driving the ATOM matrix display
#include <JC_Button.h>          // for driving the ATOM display button.
#include <I2C_MPU6886.h>        // for driving the ATOM IMU.

#define DIS_BUTTON 39           // GPIO pin attached to the display button
#define DB_PULLUP 1             // 1 to use internal GPIO pullup, 0 to use an external pullup on the DIS_BUTTON pin.
                                //  - this is here as work is starting on porting STAC to platforms other than the ATOM MATRIX.
                                //  - if the GPIO pin chosen to emulate the MATRIX display button does not have an internal
                                //    pullup resistor, an external one should be used and DB_PULLUP set to 0.

// Create the objects we need to talk to the ATOM hardware.
Button dButt(DIS_BUTTON, 20, DB_PULLUP, true);          // display Button(pin, dbTime, puEnable, invert) instantiates a button object.
I2C_MPU6886 imu(I2C_MPU6886_DEFAULT_ADDRESS, Wire1);    // IMU (default address is 0x68, Wire1 is an I2C bus on user defined pins - done in setup() )
CRGB leds[25];                                          // buffer for the 25 LEDs of the display
WiFiClient stClient;		    // initiate the WiFi library and create a WiFi client object
Preferences stcPrefs;           // holds the operational parameters in NVS for retention across restarts and power cycles.

#define WIFI_CONNECT_TIMEOUT 60000      // # of ms to wait in the "connect to WiFi" routine without a successful connection before returning
#define ST_ATTEMPTS 10                  // # of times we try to connect to the Smart Tally server before giving up

#define PO_PIXEL 12                     // the pixel position # of the display to use as the Power On indicator

#define PM_CK_OUT 22                    // ATOM GPIO pin # used for ouput to check to see if the Peripheral Mode jumper is installed
#define PM_CK_IN 33                     // ATOM GPIO pin # used for input to check to see if the Peripheral Mode jumper is installed
#define PM_CK_CNT 5                     // # of times to toggle PM_CK_OUT to test that the Peripheral Mode jumper is installed
#define TS_0 32                         // ATOM GPIO pin # used as an input if in Peripheral Mode, output otherwise
#define TS_1 26                         // ATOM GPIO pin # used as an input if in Peripheral Mode, output otherwise

#define PREFS_RO true                   // NVS Preferences namespace is Read Only if true
#define PREFS_RW false                  // NVS Preferences namespace is Read-Write if false


// ***** Global Variables *****

String stacID;                  // generated in setup(). The unique STAC ID used for the AP SSID & STA hostname
char networkSSID[33]{};         // ST server WiFi SSID. Configured via the user's web browser; max length of a WiFi SSID is 32 char
char networkPass[64]{};         // ST server WiFI password. Configured via the user's web browser; max length of a password is 63 char
IPAddress stIP;                 // IP address of the ST server. Configured via the user's web browser
uint16_t stPort;                // HTTP port of the actual or emulated Roland Smart Tally server (switcher). Configured
                                //  via the user's web browser; 0 to 65353; default is 80

bool ctMode;                    // initialzed in setup(). "Camera Operator" or "Talent" mode. True for camera operator mode, false for talent mode.
bool autoStart;                 // initialzed in setup(). true to bypass the normal "click through to confirm start" sequence.
uint8_t currentBrightness;      // Atom display brightness. Initialzed in setup()
String lastTallyState;          // tally state before the call to getTallyState().
unsigned long stsPollInt;       // # of ms between polling the Smart Tally server for a tally status change. Configured via the user's web browser, then set from NVS.
unsigned long nextPollTime;                 // holds a millis() counter value used to determine when the ST Server is next polled for the tally status.
//unsigned long lastPollTime;                 // holds the millis() counter value for the last time the counter was updated.
//unsigned long connectionLossCount = 0;      // used for ST Server stats
uint8_t tNoReplyCount;                      // # of consecutive "no reply from ST server" counts
bool junkReply;                             // true iff we received a reply from the ST server but it was garbage

bool Accelerometer = false;     // state to determine if an accelerometer is supported by this hardware.

enum class ORIENTATION { UP, DOWN, LEFT, RIGHT } ;              // Enumeration for orientation postions
float LOW_TOL = 100;                                            // Accelerometer parameters
float HIGH_TOL = 900;                                           // Accelerometer parameters
float MID_TOL = LOW_TOL + ( HIGH_TOL - LOW_TOL ) / 2.0 ;        // Accelerometer parameters

// ===== Define data structures used by the Control loop() =====
//          - using structures to pass the state conditions from the state functions back to the control loop

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
                        //     received within the timeout period) OR (tTimeout is true)
    String tState;      // the state of the tally channel being monitored as returned by the querry to the ST Server
    uint8_t tHistory;   // keeps track of the last 8 server requests
} tallyStatus;

typedef struct WfState WiFiState;       // going to need a function that returns a WfState structure
typedef struct TState TallyState;       // going to need a function that returns a TState structure

struct provision {                      // structure that holds the provisioning data from the WiFi config routines
    String pSSID;
    String pPass;
    String pSwitchIP;
    uint16_t pPort;
    uint8_t ptChanMax;
    unsigned long pPollInt;
};
typedef struct provision provData_t;

// ***** End Global Variables *****

/* ~~~~~~ NVS Items ~~~~~~ */
/*

  Stuff we're putting into NVS for retention across power cycles. Uses the 'Preferences' library to manage.

  Name space: STCPrefs - used when the STAC is in its normal operating state
   
   This defines the layout and structure for the NOM_PREFS_VERSION

    NOTE:
        Any change to the table below requires that the NOM_PREFS_VERSION #define above be incremented.
     
    Keys:
      
        NVS Key         NVS type    --> app identifier              app type        comment
        -------         --------    --------------------            --------        -------------------------------------------------
        nvsInit         Bool        --> tpInit                      bool            true if the STCPrefs NVS namespace and its key:value pairs have been created
        curBright       UChar       --> currentBrightness           uint8_t         display brightness when in normal operating state
        talChan         UChar       --> tallyStatus.tChannel        uint8_t         tally channel being monitored.
        talMax          UChar       --> tallyStatus.tMax            uint8_t         max tally channel #
        ctMde           Bool        --> ctMode                      bool            "camera operator" or "talent mode"
        pPoll           ULong       --> stsPollInt                  unsigned long   # of ms between polling the Smart Tally server for a tally status change
        pVis            Bool        --> provisioned                 bool            true if the WiFi credentials in NVS are non-zero
        aStart          Bool        --> autoStart                   bool            true if the STAC should autostart on power up or reset
        stnSSID         String      --> stSSID                      String          SSID of the WiFi network to connect to     
        stnPass         String      --> stPass                      String          password of the WiFi network to connect to
        stswIP          String      --> stswIP                      String          IP address of the Roland Smart Tally device being monitored
        stswPort        UShort      --> stswPort                    uint16_t        Port number of the actual or emulated Roland Smart Tally device
        swVersion       String      --> bootVer                     String          The STAC software version it was operating with before its last power down
        prefVer         UShort      --> NOM_PREFS_VERSION           uint16_t        The version number of the NVS Preferences layout the STAC had at boot time.
                                                                                     - Used to decide if we can reuse the existing NVS preferences values after a softwate update
                                                                                     - Means the user doesn't have to reconfigure the STAC if the NVS layout hasn't changed 
                                                                                       across software versions
 
  Name space: PModePrefs - used when the STAC is operating in Peripheral Mode
  
   Keys:

    NVS Key         NVS type    --> app identifier              app type        comment
    -------         --------    --------------------            --------        -------------------------------------------------
    pmbrightness    UChar       --> pmBright                    uint8_t         display brightness when operating in Peripheral Mode

*/
/*~~~~~ End NVS Items ~~~~~*/

// ***** Function Definitions *****

// The include order is significant
#include "./STACLib/STACUtil.h"             // utility functions
#include "./STACLib/STACGlyph.h"            // definition of display glyphs and colors
#include "./STACLib/STACDis.h"              // routines for manupulating & drawing on the display
#include "./STACLib/STACWeb.h"              // the embeded web server funtions - for provisioning
#include "./STACLib/STACIMU.h"              // on-board IMU routines
#include "./STACLib/STACSTS.h"              // functions to retreive status from the Smart Tally Server (STS)
#include "./STACLib/STACWiFi.h"             // connect to & manage the WiFi
#include "./STACLib/STACOpModes.h"          // routines to change the run-time operating modes at startup 

// ***** End Function Definitions *****

// And so it begins...

void setup() {

    provData_t sConfigData;                 // structure to hold the WiFi provisioning data from user's web browser
    bool provisioned;                       // true if the WiFi provisioning has been done
    String bootVer;                         // the version of software the STAC thinks it has - from NVS storage at boot time
    uint16_t bootPrefs;                     // the version of the normal operating mode preferences layout the STAC thinks it has - from NVS storage at boot time
    bool escapeFlag = false;                // used for getting out of the settings loops.

    dButt.begin();
    Wire1.begin( 25, 21, 100000L );             // enable the I2C bus (SDA pin, SCL pin, Clock frequency)
    FastLED.addLeds<WS2812B, 27>(leds, 25);     // matrix display of 25 RGB LED's on pin 27
    Serial.begin(115200);
    delay(250);

    #if ( DB_MODE )
        #include <Esp.h>
        fetchInfo();
    #endif
    
    dButt.read();                         // initialize the Btn class
    disClear();
    disSetBright(10);
    disDrawPix(PO_PIXEL, PO_COLOR);       // turn on the power LED

    /*
     * Setting up to figure out the orientation of the STAC & then create a copy
     * of the glyph matrix rotated to match
    */
    
    ORIENTATION stacOrientation;                     // Pull data from the accelerometer and determine the orientation
    Accelerometer = imu.begin() != -1;
    if ( !Accelerometer ) {
        log_e( "Could not initalize the IMU." );
        stacOrientation = ORIENTATION::UP;           // Set the default orientation of the STAC
    }
    else {
        stacOrientation = getOrientation();          // Go check the orientation of the STAC    
    }
    rotateGlyphs(stacOrientation) ;                  // create the rotated glyph matrix
    Wire1.flush();                                   // done with the IMU; release all I2C bus resources, power down peripheral
    
    // create the unique STAC ID
    uint32_t chiptID = (uint32_t)(ESP.getEfuseMac() >> 16);     // grab the top four bytes of the chip id and  
    stacID = idPrefix + String(chiptID, HEX);                   //  use that to create the last part of the unique STAC ID
    stacID.toUpperCase();                                       // flip any hex alphas to upper case

    // THE FOLLOWING 3 HEADER FILE MUST STAY HERE AND IN THIS ORDER - relative position to each other and the code above and below is important!
    #include "./STACLib/STACInfoHeader.h"           // send the serial port info dump header
    #include "./STACLib/STACPeripheral.h"           // do all the Peripheral Mode checks and run in PM if set
    #include "./STACLib/STACProvision.h"            // do all the checks for provisioning & initialization for Normal Operating Mode

    drawGlyph(glyphMap[tallyStatus.tChannel], bluecolor);       // do this here as setting the tally channel is the first thing we do in the user setup stuff.
                                                                //   - also gives the user some feedback so they can see we've transitioned out of doing all the setup bits
    while ( dButt.read() );                                     // wait for button to be released

    bool asBypass = false;                                      // get set up for the auto start detect and control stuff
    if (autoStart) {                                            // if we're in auto start mode...
        asBypass = true;
        pulsePix(true, GRB_AS_PULSE_COLOR);                         // ...turn on the four corner LEDs
        bool nextonstate = false;
        unsigned long asTimeOut =  millis() + 20000;                // autostart timeout is 20 seconds
        unsigned long nextFlash = millis() + 1000;                  // flash the corners every second while waiting for autostart to time out.
        while ( asTimeOut >= millis() && dButt.read() == 0 ) {      // pause until we time out (continue with auto start)
            if ( nextFlash <= millis() ) {                          // have some fun & flash the four corner LED's while waiting :)
                pulsePix(nextonstate, GRB_AS_PULSE_COLOR);
                nextonstate = !nextonstate;
                nextFlash = millis() + 1000;
            }
        }
        if ( dButt.read() ) {                                  // unless the button is pressed, in which case...
            asBypass = false;                                  //   ...cancel auto start
        }
    }

    /* Setting up to display & change the current (active) tally channel
     *  
     * If the button is clicked, drop out
     * If the button is long pressed, call changeTallyChannel() and return to the start of
     *  "Setting up to display the current tally channel"
    */
    if (!asBypass) {                                              // skip everything here if autostart kicked in
        escapeFlag = false;
        
        do  {
            dButt.read();                                               // read & refresh the state of the button
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor(1500) ) {
                changeTallyChannel();
                drawGlyph(glyphMap[tallyStatus.tChannel], bluecolor);   // display the (new) current tally channel
            }
        } while ( !escapeFlag );
        dButt.read();
    }

    /* Setting up to display & change the current tally mode
     * 
     * If the button is clicked, drop out.
     * If the button is long pressed, call changeTallyMode() and return to the start of
     *  "setting up to display the current tally mode"
    */
    if (!asBypass) {                                            // skip everything here if autostart kicked in
        if (ctMode) drawGlyph(GLF_C, purplecolor);              // display the current tally mode...
        else drawGlyph(GLF_T, purplecolor);
        
        while ( dButt.read() );                                 // wait for button to be released
        escapeFlag = false;
        
        do  {
            dButt.read();                      // read & refresh the state of the button
            if ( dButt.wasReleased() ) {
                escapeFlag = true;             // if button was clicked, exit
                }
            if ( dButt.pressedFor(1500) ) {
                changeTallyMode();
                if (ctMode) drawGlyph(GLF_C, purplecolor);      // display the (new) current tally mode...
                else drawGlyph(GLF_T, purplecolor);
            }
        } while ( !escapeFlag );
        dButt.read();
    }

    /* Setting up to display & change the startup mode
     *  
     * If the button is clicked, drop out.
     * If the button is long pressed, call changeStartupMode() and return to the start of
     *  "Setting up to display & change the startup mode"
    */
    if (!asBypass) {                                            // skip everything here if autostart kicked in
        if (autoStart) drawGlyph(GLF_A, tealcolor);             // display the current startup mode
        else drawGlyph(GLF_S, tealcolor);
        
        while ( dButt.read() );                             // wait for button to be released
        escapeFlag = false;
        
        do  {
            dButt.read();                                   // read & refresh the state of the button
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor(1500) ) {
                changeStartupMode();
                if (autoStart) drawGlyph(GLF_A, tealcolor);     // display the current startup mode              
                else drawGlyph(GLF_S, tealcolor);
            }
        } while (!escapeFlag);
        dButt.read();
    }

    /* Setting up to display & change the current display brightness level
     * 
     * If the button is clicked, drop out.
     * If the button is long pressed, call updateBrightness() and return to the start of
     *  "Setting up to display the current brightness level"
    */
    if (!asBypass) {                                                        // skip everything here if autostart kicked in
        drawGlyph(GLF_CBD, brightnessset);                                  // draw the checkerboard test pattern...
        drawOverlay(GLF_EN, GRB_COLOR_BLACK);                               // blank out the inside three columns...
        drawOverlay(glyphMap[currentBrightness / 10], GRB_COLOR_WHITE);     // and overlay the brightness setting number
    
        while ( dButt.read() );                                         // wait for button to be released
        escapeFlag = false;
        
        do  {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor(1500) ) {
                currentBrightness = updateBrightness(stcPrefs, "STCPrefs", "curBright", currentBrightness);          
                drawGlyph(GLF_CBD, brightnessset);                                  // draw the checkerboard test pattern...
                drawOverlay(GLF_EN, GRB_COLOR_BLACK);                               // blank out the inside three columns...
                drawOverlay(glyphMap[currentBrightness / 10], GRB_COLOR_WHITE);     // and overlay the (new) brightness setting number
           }
        } while ( !escapeFlag );
        dButt.read();
    }
    
    disClear();
    disDrawPix(PO_PIXEL, GRB_COLOR_GREEN);
    delay (1000);                // chill for a second for the sake of the "GUI"
 
    nextPollTime = millis();     // set the initial value for the ST Server poll timer to now
                    
    if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.println(": In setup(). nextPoll set to NOW "); }
                

}   // closing brace for setup()


void loop() {

    /* ~~~~~ Update Brightness contol logic ~~~~~ */
    
    dButt.read();                        // put this at the top of loop() instead of the bottom so it always gets hit
    if ( dButt.pressedFor(1500) ) {
        GROVE_UNKNOWN;                           // output the tally state to the GROVE pins
        currentBrightness = updateBrightness(stcPrefs, "STCPrefs", "curBright", currentBrightness);
        if ( !ctMode ) {                        // if in talent mode
            disFillPix(PVW);                    //  set the display to PVW
        }
        else {
            disClear();
        }
        disDrawPix(PO_PIXEL, PO_COLOR);
        
        lastTallyState = "NO_TALLY";            // reset the tally status control logic...
        tallyStatus.tState = "NO_INIT";

        nextPollTime = millis();                // force a repoll of the tally state

        if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.println(": Update Brightness contol logic. nextPoll set to NOW "); }

    }   // closing brace for Update Brightness contol logic

    /* ~~~~~ Connect to WiFi control logic ~~~~~ */

    if ( WiFi.status() != WL_CONNECTED ) {

        log_i("No WiFi connection.");
        
        bool leaveDisplayGreen = false;

        if (wifiStatus.wfconnect) {                 // if we had a previous good WiFi connection...
            GROVE_UNKNOWN;                          //  output the tally state to the GROVE pins
            stClient.stop();                        //  stop the stClient
            WiFi.disconnect();                      //  kill the WiFi
            log_w("WiFi connection lost :(");

            if (!ctMode) {                          // if in talent mode...
                disFillPix(PVW);                    //  set the display to PVW
                disDrawPix(PO_PIXEL, PO_COLOR);     //  turn on the power LED
                leaveDisplayGreen = true;           //  let the downstream code know to not futz with the display 
            }
            else {
                drawGlyph(GLF_WIFI, alertcolor);            // let the user know we lost WiFi...
                flashDisplay(8, 300, currentBrightness);
                delay(1500);
            }
            wifiStatus.wfconnect = false;       // reset the control logic...
            wifiStatus.timeout = false;
            lastTallyState = "NO_TALLY";
            tallyStatus.tState = "NO_INIT";
        }

        if (!leaveDisplayGreen) drawGlyph(GLF_WIFI, warningcolor);          // draw the "attempting to connect to WiFi" thing on the display
        while (!wifiStatus.wfconnect) {
            wifiStatus = connectToWifi(wifiStatus);
            if (wifiStatus.timeout) log_w("WiFi connect attempt timed out");
            if (wifiStatus.timeout && !leaveDisplayGreen) {
                drawGlyph(GLF_WIFI, alertcolor);            // let the user know we timed out trying to connect to WiFi... 
                flashDisplay(8, 300, currentBrightness);
                delay(1500);
                drawGlyph(GLF_WIFI, warningcolor);          // draw the "attempting to connect to WiFi" thing on the display.
            }
        }

        log_i("WiFi connected");

        if (!leaveDisplayGreen) {
            drawGlyph(GLF_WIFI, gtgcolor);          // draw the "connected to WiFi" thing on the display.
            delay(1500);
            disClear();
            disDrawPix(PO_PIXEL, PO_COLOR);
        }
        nextPollTime = millis();                // force a repoll of the ST server

        if ( DB_MODE ) { Serial.print(__LINE__ - 2); Serial.println(": Connect to WiFi control logic. nextPoll set to NOW "); }

    }   // closing brace for "Connect to WiFi control logic"

    /* ~~~~~ Update Tally Display control logic ~~~~~ */

    if ( millis() >= nextPollTime ) {             // we _just_ did a WiFi check so no need to repeat that here

        if ( DB_MODE ) {
            Serial.println("\r\n\r\n");
            Serial.print( __LINE__ - 2 ); Serial.println (": Top of Update Tally Display control logic. Time to poll.");
            Serial.print( __LINE__ - 3 ); Serial.print(": lastTally state is: "); Serial.println(lastTallyState);
            Serial.print( __LINE__ - 4 ); Serial.print(": tallyStatus.tState is: "); Serial.println(tallyStatus.tState);
        }
        
        tallyStatus = getTallyState(tallyStatus);

        if ( DB_MODE ) {
            Serial.print( __LINE__ - 2 ); Serial.println (": Back from call to getTallyStatus.");
            Serial.print( __LINE__ - 3 ); Serial.print(": tallyStatus.tState is: "); Serial.println(tallyStatus.tState);
            Serial.print( __LINE__ - 4 ); Serial.print(": tallyStatus.tConnect is: "); Serial.println(tallyStatus.tConnect);
            Serial.print( __LINE__ - 5 ); Serial.print(": tallyStatus.tTimeout is: "); Serial.println(tallyStatus.tTimeout);
            Serial.print( __LINE__ - 6 ); Serial.print(": tallyStatus.NoReply is: "); Serial.println(tallyStatus.tNoReply);
        }

        // handle the normal operating conditions first...
        if ( tallyStatus.tConnect && !tallyStatus.tNoReply ) {
            // we have a reply from the STS, lets go figure out what to do with it
            
            if ( DB_MODE) { Serial.print( __LINE__ - 2 ); Serial.println( ": Inside if ( !tallyStatus.tNoReply )" ); }

            nextPollTime = millis() + stsPollInt;
                                                
            if ( DB_MODE) { Serial.print( __LINE__ - 2); Serial.println(": Inside if ( !tallyStatus.tNoReply ). nextPollTime set to NOW + stsPollInt"); }

            junkReply = false;
            if ( tallyStatus.tState != lastTallyState ) {
            
                if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.println(": Inside if ( !tallyStatus.tNoReply ). tally state change"); }
            
                if (tallyStatus.tState == "onair") {
                    GROVE_PGM;                                      // output the tally state to the GROVE pins
                    disFillPix(PGM);                                // change the display to the PGM colour;
                }
                else if (tallyStatus.tState == "selected") {
                    GROVE_PVW;                                      // output the tally state to the GROVE pins
                    disFillPix(PVW);                                // change the display to the PVW colour
                }
                else if (tallyStatus.tState == "unselected") {
                    GROVE_NO_SEL;                                   // output the tally state to the GROVE pins
                    if (ctMode) {                                   // if in camera operator mode
                        drawGlyph(GLF_DF, unselectedcolor);         //  change the display to the unselected glyph and colour
                    }
                    else {
                        disFillPix(PVW);                           // else change the display to the PVW colour
                    }
                }
                else {
                    // catchall codeblock only executed if we get a junk reply from the STS
                    
                    if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.println(": Inside if ( !tallyStatus.tNoReply). In the junk drawer."); }
                    
                    junkReply = true;                             // we got a reply from the ST server, but it's garbage
                    lastTallyState = "JUNK";
                    tallyStatus.tState = "NO_TALLY";
                    GROVE_UNKNOWN;                                // output the tally state to the GROVE pins
                    log_e("ts error: junk reply = %s", tallyStatus.tState);
                    if (ctMode) {                                 // if in camera operator mode
                        drawGlyph(GLF_QM, purplecolor);           //  display unknown response glyph

                    if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.println(": Inside if ( !tallyStatus.tNoReply). In the junk drawer. Displaying '?'"); }
                        
                    }
                    else {
                        disFillPix(PVW);                          // else change the display to the PVW colour
                        disDrawPix(PO_PIXEL, PO_COLOR);           // turn on the power LED
                    }
//                     nextPollTime = millis();                      // force a re-poll next time through loop()
                    nextPollTime = millis() + stsPollInt;         // for debugging
            
                    if ( DB_MODE) { Serial.print(__LINE__ - 2); Serial.println(": Inside if ( !tallyStatus.tNoReply. In the junk drawer. nextPoll set to NOW "); }

                }
                if (!junkReply) {
                    tNoReplyCount = 0;                             // clear the BPX error accumulator
                    disDrawPix(PO_PIXEL, PO_COLOR);                // turn on the power LED
                    lastTallyState = tallyStatus.tState;           // save the current tally state
                }
                
            }   // closing brace for "if ( tallyStatus.tState != lastTallyState )"
            
        }   // closing brace for "if ( !tallyState.tNoReply )"
            
        // the block below here handles all the error conditions (except a junk STS reply) reported by the last tally check
        else {
            // error occurred when trying get a tally status update

            if ( DB_MODE ) { 
                Serial.print( __LINE__ - 2 ); Serial.println(": Inside the \"else\" error handling block");
                Serial.print( __LINE__ - 3 ); Serial.print(": tallyStatus.tState is: "); Serial.println(tallyStatus.tState);
                Serial.print( __LINE__ - 4 ); Serial.print(": tallyStatus.tConnect is: "); Serial.println(tallyStatus.tConnect);
                Serial.print( __LINE__ - 5 ); Serial.print(": tallyStatus.tTimeout is: "); Serial.println(tallyStatus.tTimeout);
                Serial.print( __LINE__ - 6 ); Serial.print(": tallyStatus.NoReply is: "); Serial.println(tallyStatus.tNoReply);
            }
            
            lastTallyState = "NO_TALLY";
            GROVE_UNKNOWN;                              // output the tally state to the GROVE pins

//            log_e("ts error: status = %s, connect = %i, timeout = %i, noreply = %i", 
//                tallyStatus.tState, tallyStatus.tConnect, tallyStatus.tTimeout, tallyStatus.tNoReply);

//            nextPollTime = millis();                        // force a re-poll next time through loop()
            nextPollTime = millis() + stsPollInt;           // for debugging
            
            if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.println(": Inside the \"else\" error handling block. nextPoll set to NOW "); }

            if ( !tallyStatus.tConnect && tallyStatus.tTimeout ) {
                if (ctMode) {
                    drawGlyph(GLF_BX, warningcolor);       // throw a BPX to the display... (for debugging, will throw an orange X)

                    if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.print(": Inside the \"else\" error handling block. Displaying orange X"); }
                    
                }
                else {
                    disFillPix(PVW);                    // else change the display to the PVW colour...
                    disDrawPix(PO_PIXEL, PO_COLOR);     //  and turn on the power LED
                }
            }    
            else if ( tallyStatus.tConnect && ( tallyStatus.tTimeout || tallyStatus.tNoReply ) ) {

                if ( DB_MODE ) { 
                    Serial.print( __LINE__ - 2 ); Serial.print(": Inside the \"else\" error handling block. tConnect = "); Serial.println( tallyStatus.tConnect );
                    Serial.print( __LINE__ - 3 ); Serial.print(": Inside the \"else\" error handling block. tTimeOut = "); Serial.println( tallyStatus.tTimeout );
                    Serial.print( __LINE__ - 4 ); Serial.print(": Inside the \"else\" error handling block. tConnect = "); Serial.println( tallyStatus.tNoReply );
                }
                
                tNoReplyCount++;
                
                if ( DB_MODE ) { Serial.print( __LINE__ - 1 ); Serial.print(": Inside the \"else\" error handling block. tNoReplyCount = "); Serial.println( tNoReplyCount ); }

                if ( tNoReplyCount >= 8 ) {

                    if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.print(": Inside the \"else\" error handling block. tNoReplyCount = "); Serial.println( tNoReplyCount ); }

                    tNoReplyCount = 0;                     // only throw another BPX if we get 8 more in a row
                    if (ctMode) {
                        drawGlyph(GLF_BX, purplecolor);    // throw up a BPX

                        if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.print(": Inside the \"else\" error handling block. Displaying purple X"); }
                        
                    }
                    else {
                        disFillPix(PVW);                    // else change the display to the PVW colour...
                        disDrawPix(PO_PIXEL, PO_COLOR);     // and turn on the power LED
                    }
                }
            }
            else {
                // some other error condition has occurred
                if (ctMode) {
                    drawGlyph(GLF_BX, alertcolor);          // throw up a big red X
                    
                    if ( DB_MODE ) { Serial.print( __LINE__ - 2 ); Serial.println(": Inside the \"else\" error handling block. Displaying red X"); }
                    
                }
                else {
                    disFillPix(PVW);                        // else change the display to the PVW colour...
                    disDrawPix(PO_PIXEL, PO_COLOR);         //  and turn on the power LED
                }
            }

        }   // closing brace for "else error block"
                    
    }   // closing brace for: if ( millis() >= nextPollTime )

    // ~~~~~ End of the Control loop

}   // closing  brace for loop()


// - EOF -
