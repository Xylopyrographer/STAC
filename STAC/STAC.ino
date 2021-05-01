/* STAC (Smart Tally ATOM Matrix Client) 
 *
 *  Version: 1.8
 *  2021-04-23
 *  
 *  Author: Xylopyrographer
 *  
 *  A Roland Smart Tally Client
 *     An Arduino sketch designed to run on an M5Stack ATOM Matrix board.
 *     Its purpose is to monitor the tally status of a single video input channel 
 *     of a Roland device that implements their proprietary Smart Tally protocol.
 *     The sketch uses WiFi to connect to the same network as the Roland device.
 *     For the Roland video input channel being monitored, STAC will set
 *     the colour of the display on the ATOM:
 *      + when in "Camera Operator" mode, to:
 *          - RED if the channel is in PGM (Progam or onair)
 *          - GREEN if the channel is in PVW (Preview or selected)
 *          - "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected)
 *      + when in "Talent" mode, to:
 *          - RED if the channel is in PGM (Progam or onair)
 *          - GREEN otherwise
 *     
 *  Configuration of the STAC for the WiFi credentials and IP address, port number  
 *  and number of tally channels of the Roland switch is done using a web browser.
 *
 *  When in its Configuration state the:
 *
 *      STAC WiFi SSID is:  
 *          STAC_XXXXXXXX  
 *      where the X's are a set of 8 alphanumeric characters unique to every ATOM unit.
 *      
 *      Password is:
 *          1234567890
 *
 *      STAC configuration page is at:  
 *          192.168.6.14
 *          
 *  More at: https://github.com/Xylopyrographer/STAC
 *  
*/

#include <WiFi.h>
#include <M5Atom.h>             // this is a modified M5Atom library. See the docs in the GitHub repository for info.
#include <Preferences.h>        // suggest using the modified Preferences library. See the docs in the repository for info.

String swVer = "1.8";           // shows up on the web config page
String apPrefix = "STAC_";      // prefix to use for naming the STAC AP SSID when being configured

char networkSSID[33]{};         // ST server WiFi SSID. Configured via the user's web browser; max length of a WiFi SSID is 32 char
char networkPass[64]{};         // ST server WiFI password. Configured via the user's web browser; max length of a password is 63 char
char stIP[16]{};                // IP address of the ST server. Configured via the user's web browser; max length of an IP address is 15 char
uint16_t stPort;                // HTTP port of the actual or emulated Roland Smart Tally server (switcher). Configured via the user's web browser; 0 to 65353; default is 80

bool ctMode;                    // initialzed in setup(). "Camera Operator" or "Talent" mode. True for camera operator mode, false for talent mode.
bool autoStart;                 // initialzed in setup(). true to bypass the normal "click through to confirm start" sequence.
bool debug = false;			        // will send any debugging stuff out the on-board USB port of the ATOM if true.
bool Accelerometer = false;     // State to determine if an accelerometer is supported by this hardware.

// ***** Global Variables *****
// ~~~~~ State Machine Event Tansition Variables ~~~~~

#define ST_POLL_INTERVAL 150                // # of ms between polling the Smart Tally server for a tally status change
#define ST_ATTEMPTS 10                      // # of times we try to connect to the Smart Tally server before giving up
#define ST_CONNECT_TIMEOUT 500              // # of ms to wait before reattempting to connect to the ST Server after a failed connection attempt
#define WIFI_CONNECT_TIMEOUT 60000          // # of ms to wait in the "connect to WiFi" routine without a successful connection before returning
#define WIFI_ATTEMPTS 6                     // # of times to try to connect to the WiFi network before giving up

enum class ORIENTATION { UP, DOWN, LEFT, RIGHT } ;              // Enumeration for orientation postions
float LOW_TOL = 100;                                            // Accelerometer parameters
float HIGH_TOL = 900;                                           // Accelerometer parameters
float MID_TOL = LOW_TOL + ( HIGH_TOL - LOW_TOL ) / 2.0 ;        // Accelerometer parameters

Preferences stcPrefs;                       // holds the operational parameters in NVS for retention across power cycles.
String lastTallyState = "--nullll--";
uint8_t btnWas, btnNow;                     // used in the button click detector buttonClicked() and the control logic for this fn
bool escapeFlag = false;                    // used for getting out of the settings loops in setup()
unsigned long nextPollTime;                 // holds a millis() counter value used to determine when the ST Server is next polled for the tally status

// ===== Define data strcutures used by the Control loop() =====
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
} tallyStatus;

typedef struct WfState WiFiState;       // going to need a function that returns a WfState structure
typedef struct TState TallyState;       // going to need a function that returns a TState structure

struct provision {                      // structure that holds the credentials data from the WiFi config routines
    String pSSID;
    String pPass;
    String pSwitchIP;
    uint16_t pPort;
    uint8_t ptChanMax;
};

typedef struct provision provData_t;

// ~~~~~ End State Machine Event Tansition Variables ~~~~~

/* ~~~~~ NVS Items ~~~~~

  Stuff we're putting into NVS for retention across power cycles
  Use [.put | .get] "NVS type" to access
  Name space: STCPrefs
  Keys:
  
    NVS Key         NVS type    --> app identifier              app type        comment
    -------         --------    --------------------            --------        -------------------------------------------------
    nvsInit         Bool        --> tpInit                      bool            true if the STCPrefs NVS namespace and its key:value pairs have been created
    curBright       UChar       --> currentBrightness           uint8_t         display brightness
    talChan         UChar       --> tallyStatus.tChannel        uint8_t         tally channel being monitored.
    talMax          UChar       --> tallyStatus.tMax            uint8_t         max tally channel #
    ctMde           Bool        --> ctMode                      bool            "camera operator" or "talent mode"
    pVis            Bool        --> provisioned                 bool            true if the WiFi credentials in NVS are non-zero
    aStart          Bool        --> autoStart                   bool            true if the STAC should autostart on power up or reset
    stnSSID         String      --> stSSID                      String          SSID of the WiFi network to connect to     
    stnPass         String      --> stPass                      String          password of the WiFi network to connect to
    stswIP          String      --> stswIP                      String          IP address of the Roland Smart Tally device being monitored
    stswPort        UShort      --> stswPort                    uint16_t        Port number of the actual or emulated Roland Smart Tally device
    swVersion       String      --> bootVer                     String          The software version the STAC thinks it has from NVS storage at boot time 
*/

WiFiClient stClient;				    // initiate the WiFi library and create a WiFi client object

#define TOTAL_GLYPHS 30         // Maxmimum number of Glyphs in memory

// This is the base set of Glyphs before rotation
uint8_t baseGlyphMap[TOTAL_GLYPHS][25] = {
    {0,0,1,0,0,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,0,0,1,0,0},    // 0
    {0,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0,0,0,1,1,1,0},    // 1
    {0,1,1,0,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,1,1,0},    // 2
    {0,1,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,0,1,1,0,0},    // 3
    {0,1,0,1,0,0,1,0,1,0,0,0,1,1,0,0,0,0,1,0,0,0,0,1,0},    // 4
    {0,1,1,1,0,0,1,0,0,0,0,0,1,1,0,0,0,0,1,0,0,1,1,1,0},    // 5
    {0,0,1,1,0,0,1,0,0,0,0,1,1,1,0,0,1,0,1,0,0,1,1,1,0},    // 6
    {0,1,1,1,0,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,1,0,0,0},    // 7
    {0,1,1,1,0,0,1,0,1,0,0,1,1,1,0,0,1,0,1,0,0,1,1,1,0},    // 8
    {0,1,1,1,0,0,1,0,1,0,0,1,1,1,0,0,0,0,1,0,0,1,1,0,0},    // 9
    {0,1,0,1,0,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,0,1,0,1,0},    // X
    {0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,1,0,0,0,1,0,1,0},    // WiFi logo
    {1,1,0,0,1,1,1,0,1,1,1,1,1,1,1,1,1,0,1,1,1,1,0,0,1},    // ST
    {0,0,1,1,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,0,1,1,0},    // C
    {0,1,1,1,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1,0,0},    // T
    {0,0,1,0,0,0,0,0,1,0,1,1,1,1,1,0,0,0,1,0,0,0,1,0,0},    // RA - right arrow
    {0,0,1,0,0,0,1,0,0,0,1,1,1,1,1,0,1,0,0,0,0,0,1,0,0},    // LA - left arrow
    {0,1,0,1,0,0,0,0,0,0,1,0,0,0,1,1,0,0,0,1,0,1,1,1,0},    // HF - smiley face
    {1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1},    // BX - big X
    {1,1,1,1,1,1,0,0,0,1,1,0,0,0,1,1,0,0,0,1,1,1,1,1,1},    // FM - frame
    {1,0,1,0,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,1,0,1,0,1},    // DF - dotted frame
    {0,1,1,0,0,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,1,0,0},    // QM - question mark
    {1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1,0,1},    // CBD - checkerboard
    {0,0,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,0,0,0,0,0},    // CK - checkmark
    {0,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,1,1,1,0},    // EN space - fills the innermost three columns
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},    // EM space - fills the entire display
    {0,0,0,0,0,0,1,1,1,0,0,1,1,1,0,0,1,1,1,0,0,0,1,1,0},    // dot
    {0,0,1,1,1,0,0,0,0,1,1,0,1,0,1,1,0,0,0,0,1,1,1,0,0},    // WiFi congig
    {0,0,1,0,0,0,1,0,1,0,0,1,1,1,0,0,1,0,1,0,0,1,0,1,0},    // A
    {0,0,1,1,0,0,1,0,0,0,0,0,1,0,0,0,0,0,1,0,0,1,1,0,0},    // S
    
};

// This is the set of Glyphs that will be used by drawGlyph()
uint8_t glyphMap[TOTAL_GLYPHS][25] = {0} ;

/* ----- mnemonic define table for the glyphMap[] -----
 *  Need to keep the numbers 0 to 9 at the start of the array and in the order below as latter 
 *  functions that index into the glyphMap[] directly depend on this order to retrieve the 
 *  correct bitmap corresponding to that number.
*/

#define GLF_0 glyphMap[0]       // the number 0
#define GLF_1 glyphMap[1]       // the number 1
#define GLF_2 glyphMap[2]       // the number 2
#define GLF_3 glyphMap[3]       // the number 3
#define GLF_4 glyphMap[4]       // the number 4
#define GLF_5 glyphMap[5]       // the number 5
#define GLF_6 glyphMap[6]       // the number 6
#define GLF_7 glyphMap[7]       // the number 7
#define GLF_8 glyphMap[8]       // the number 8
#define GLF_9 glyphMap[9]       // the number 9
#define GLF_X glyphMap[10]      // the letter X
#define GLF_WIFI glyphMap[11]   // WiFi icon
#define GLF_ST glyphMap[12]     // Smart Tally icon
#define GLF_C glyphMap[13]      // the letter C
#define GLF_T glyphMap[14]      // the letter T
#define GLF_RA glyphMap[15]     // right arrow
#define GLF_LA glyphMap[16]     // left arrow
#define GLF_HF glyphMap[17]     // smimley face
#define GLF_BX glyphMap[18]     // big X
#define GLF_FM glyphMap[19]     // frame
#define GLF_DF glyphMap[20]     // dotted frame
#define GLF_QM glyphMap[21]     // question mark
#define GLF_CBD glyphMap[22]    // checkerboard pattern
#define GLF_CK glyphMap[23]     // checkmark
#define GLF_EN glyphMap[24]     // en space
#define GLF_EM glyphMap[25]     // em space
#define GLF_DOT glyphMap[26]    // dot
#define GLF_CFG glyphMap[27]    // WiFi config icon
#define GLF_A glyphMap[28]      // the letter A
#define GLF_S glyphMap[29]      // the letter S

// ~~~~~ Colour value definitions for the Atom display ~~~~~
const int GRB_COLOR_WHITE = 0xffffff;
const int GRB_COLOR_BLACK = 0x000000;
const int GRB_COLOR_GREY = 0x050505;      // in practice, this colour doesn't really work :(
const int GRB_COLOR_RED = 0x00ff00;
const int GRB_COLOR_ORANGE = 0x65ff00;    // was 0xa5ff00
const int GRB_COLOR_YELLOW = 0xffff00;
const int GRB_COLOR_GREEN = 0xff0000;
const int GRB_COLOR_BLUE = 0x0000ff;
const int GRB_COLOR_DKBLUE = 0x00007f;
const int GRB_COLOR_PURPLE = 0x008080;
const int GRB_COLOR_DKPRPLE = 0x003838;
const int GRB_COLOR_TEAL = 0xe60062;        // was ;
const int GRB_COLOR_DKTEAL = 0x600026;      // was 0xa8059d;


// ----- Colour pairs for drawing glyphs with drawGlyph() -----
//          - the first colour in the pair is the background colour; the second, the foreground colour.
int programcolor[] = {GRB_COLOR_RED, GRB_COLOR_RED};
int previewcolor[] = {GRB_COLOR_GREEN, GRB_COLOR_GREEN};
int unselectedcolor[] = {GRB_COLOR_BLACK, GRB_COLOR_PURPLE};
int gtgcolor[] = {GRB_COLOR_BLACK, GRB_COLOR_GREEN};
int warningcolor[] = {GRB_COLOR_BLACK, GRB_COLOR_ORANGE};
int alertcolor[] = {GRB_COLOR_BLACK, GRB_COLOR_RED};
int bluecolor[] = {GRB_COLOR_BLACK, GRB_COLOR_BLUE};
int purplecolor[] = {GRB_COLOR_BLACK, GRB_COLOR_PURPLE};
int tealcolor[] = {GRB_COLOR_BLACK, GRB_COLOR_TEAL};
int brightnessset[] = {GRB_COLOR_GREEN, GRB_COLOR_RED};             // colors when changing the brightness
int brightnesschange[] = {GRB_COLOR_WHITE, GRB_COLOR_WHITE};        // colors when changing the brightness
int tallychangecolor[] = {GRB_COLOR_DKBLUE, GRB_COLOR_ORANGE};      // colors when changing the tally channel
int tallymodecolor[] = {GRB_COLOR_DKPRPLE, GRB_COLOR_ORANGE};       // colors when changing the tally mode
int startchangecolor[] = {GRB_COLOR_DKTEAL, GRB_COLOR_ORANGE};      // colors when changing the startup mode
int poColor = GRB_COLOR_ORANGE;                                     // color to use for the Power On indicator pixel(s)

const int poPixel = 12;                 // the pixel position # of the display to use as the Power On indicator
uint8_t currentBrightness;              // Atom display brightness. Initialzed in setup()

// ***** End Global Variables *****


// ***** Function Definitions *****

WiFiState connectToWifi(WfState wifistate) {
/*  This is the "Connect to WiFi" state function
*/
    unsigned long wfTimeout;
    int wfStatus = WiFi.status();
    wifistate.wfconnect = false;
    wifistate.timeout = false;
    
    WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);      //set WiFi station mode
    WiFi.setSleep(false);   
    wfTimeout = millis() + WIFI_CONNECT_TIMEOUT;
   
    while (wfStatus != WL_CONNECTED && wfTimeout >= millis()) {
        wfStatus = WiFi.begin(networkSSID, networkPass);       
        delay(5000);    // it takes some time for the WiFi routines to do their thing in the backgroud. So take a pause
    }
    if (WiFi.status() == WL_CONNECTED) wifistate.wfconnect = true;
    else wifistate.timeout = true;
    return wifistate;
    
}	// closing brace for connectToWifi()

TallyState getTallyState(TState tally) {
/*  Returns the response returned from the Smart Tally server after it is sent a GET request.
    The long form of the GET request returns an HTTP header, followed by the payload (status state).
    The short form of the GET request returns only the status state.
    This function only sends and responds to the short form.
     
        stClient.print("GET /tally/" +  TALLY_CHAN + "/status HTTP/1.0\r\n\r\n");   // Uses the long form of the tally status request
        stClient.print("GET /tally/" +  TALLY_CHAN + "/status\r\n\r\n");            // Uses the short form of the tally status request
*/

    String reply = "\0\0\0\0\0\0\0\0\0\0\0";
    // unsigned long stTimeout;
    int maxloops = 0;
    tally.tState = "nojoy";
    tally.tConnect = false;
    tally.tTimeout = false;
    tally.tNoReply = true;

    stClient.setTimeout(0);                                 // stClient.readString() is a 1 sec blocking function by default!! This sets the 'wait for' timeout to 0.
    
    if (!stClient.connected()) {
        if(stClient.connect(stIP, stPort)) {
            tally.tConnect = true;
        }
        else {
            return tally;
        }
    }
    stClient.print("GET /tally/");      // send the status request sent to the ST server
    stClient.print(tally.tChannel);
    stClient.print("/status\r\n\r\n");

    while (stClient.available() == 0 && maxloops < 1000) {    // Wait a maximum of 1 second for the ST server's reply to become available
        maxloops++;
        delay(1);                                             // wait a tick before checking again if there is a response from the ST Server.
    }
    if (maxloops == 1000) {         // response from the ST Server timed out
        tally.tTimeout = true;     
        return tally;
    }

    while (stClient.available() > 0) reply += stClient.readString();        // we have a response from the server
    reply.trim();                                                           // knock any junk off the front and back of the reply
    tally.tState = reply;                                                   // store the response
    tally.tNoReply = false;                                                 // set our control flags
    return tally;                                                           // and head back to the barn

}   //closing brace for getTallyState()

void changeTallyChannel() {
/*  Allows the user to select the tally chanel being monitored. 
 *      - function will return after a period of inactivity, 
 *        restoring the monitored channel to what it was prior to the call
*/

    unsigned long updateTimeout;
    uint8_t tallyChanNow = tallyStatus.tChannel;     // keep the tally channel state we had on entry

    drawGlyph(glyphMap[tallyStatus.tChannel], tallychangecolor);    // display the current tally channel
    
    while (M5.Btn.read() == 1);                     // don't proeeed until the button is released.
    btnWas = 0;                                     // initialize the click detector
    updateTimeout = millis() + 30000;               // delete this line if you don't want to leave after a period of inactivity
     
    do {
        if (millis() >= updateTimeout) {            // remove this if clause if you don't want to leave after a period of inactivity
            tallyStatus.tChannel = tallyChanNow;    // we timed out, user didn't confirm the change, so restore the tally channel as it was on entry...
            return;                                 // and head back to the barn
        }
        btnNow = M5.Btn.read();                     // read & refresh the state of the button
        if (buttonClicked()) {
            updateTimeout = millis() + 30000;      // kick the timeout timer (delete this line if you don't want to leave after a period of inactivity)
            if (tallyStatus.tChannel >= tallyStatus.tChanMax) tallyStatus.tChannel = 1; // loop the channel number back to 1 if it would otherwise exceed the max channel #
            else tallyStatus.tChannel++;
            drawGlyph(glyphMap[tallyStatus.tChannel], tallychangecolor);
        }
        if (M5.Btn.pressedFor(1500)) {                              // user wants to confirm the change and exit so...
            drawGlyph(GLF_CK, gtgcolor);
            if (tallyChanNow != tallyStatus.tChannel) {                 // if the startup mode changed...
                stcPrefs.begin("STCPrefs", false);                      //  open up the pref namespace for R/W
                stcPrefs.putUChar("talChan", tallyStatus.tChannel);     //  save the new tally channel in NVS
                stcPrefs.end();                                         //  close the prefs namespace
            }    
            while (M5.Btn.read() == 1);                             // don't proeeed until the button is released.
            delay(500);
            return;
        }
    } while (true);
    return;
    
}   // closing brace for changeTallyChannel()

void changeTallyMode() {
/*  Allows the user to flip the STAC operaing mode
    between "camera operator" and "talent"
 *      - function will return after a period of inactivity, 
 *        restoring the operating mode to what it was prior to the call
*/
    unsigned long updateTimeout;
    bool ctModeNow = ctMode;

    if (ctMode) drawGlyph(GLF_C, tallymodecolor);   // display the current tally mode...
    else drawGlyph(GLF_T, tallymodecolor);
 
    while (M5.Btn.read() == 1);                     // don't proeeed until the button is released
    btnWas = 0;                                     // initialize the click detector
    updateTimeout = millis() + 30000;               // initialize the timeout timer 
     
    do {
        if (millis() >= updateTimeout) {
            ctMode = ctModeNow;                     // we timed out, user didn't confirm the change, restore the ctMode as it was on entry...
            return;                                 // and head back to the barn
        }

        btnNow = M5.Btn.read();                             // start the click detector. Read & refresh the state of the button
        if (buttonClicked()) {
            updateTimeout = millis() + 30000;               // kick the timeout timer
            ctMode = !ctMode;                               // btn clicked so we want to flip the current ctMode...
            if (ctMode) drawGlyph(GLF_C, tallymodecolor);   // and display the new operating mode
            else drawGlyph(GLF_T, tallymodecolor);
        }
        if (M5.Btn.pressedFor(1500)) {              // user wants to confirm the change and exit so...
            drawGlyph(GLF_CK, gtgcolor);
            if (ctModeNow != ctMode) {                  // if the startup mode changed...
                stcPrefs.begin("STCPrefs", false);      //  open up the pref namespace for R/W
                stcPrefs.putBool("ctMde", ctMode);      //  save the new ctMode in NVS
                stcPrefs.end();                         //  close the prefs namespace
            }
            while (M5.Btn.read() == 1);             // don't proeeed until the button is released
            delay(500);
            return;
        }
    } while (true);
    // return;

} //closing brace for changeTallyMode()

void changeStartupMode() {
/*  Allows the user to flip the STAC startup mode
 *  between "Auto" and "Standard"
 *      - function will return after a period of inactivity, 
 *        restoring the startup mode to what it was prior to the call
*/
    unsigned long updateTimeout;
    bool suModeNow = autoStart;
    if (autoStart) drawGlyph(GLF_A, startchangecolor);     // display the current startup mode
    else drawGlyph(GLF_S, startchangecolor);

    while (M5.Btn.read() == 1);                     // don't proeeed until the button is released
    btnWas = 0;                                     // initialize the click detector
    updateTimeout = millis() + 30000;               // initialize the timeout timer 
     
    do {
        if (millis() >= updateTimeout) {
            autoStart = suModeNow;                  // we timed out, user didn't confirm the change, restore the startup mode as it was on entry...
            return;                                 // and head back to the barn
        }

        btnNow = M5.Btn.read();                             // start the click detector. Read & refresh the state of the button
        if (buttonClicked()) {
            updateTimeout = millis() + 30000;               // kick the timeout timer
            autoStart = !autoStart;                         // btn clicked so we want to flip the current startup mode...
            if (autoStart) drawGlyph(GLF_A, startchangecolor);     // and display the new operating mode
            else drawGlyph(GLF_S, startchangecolor);
        }
        if (M5.Btn.pressedFor(1500)) {              // user wants to confirm the change and exit so...
            drawGlyph(GLF_CK, gtgcolor);
            if (suModeNow != autoStart) {               // if the startup mode changed...
                stcPrefs.begin("STCPrefs", false);      // ...open up the pref namespace for R/W              
                stcPrefs.putBool("aStart", autoStart);  // ...save the new autoStart mode in NVS
                stcPrefs.end();                         // ...close the prefs namespace
            }
            while (M5.Btn.read() == 1);             // don't proeeed until the button is released
            delay(500);
            return;
        }
    } while (true);
//    return;
}   //closing brace for changeStartupMode()

void updateBrightness() {
/*  Allows the user to select the brightness of the display. 
 *      - function will return after a period of inactivity, 
 *        restoring the brightness to what it was prior to the call
*/
    unsigned long updateTimeout;                    // delete this line if you don't want to bug out after a period of inactivity
    uint8_t brightnessNow = currentBrightness;
    
    drawGlyph(GLF_EM, brightnesschange);                                   // fill the display...
    drawOverlay(GLF_EN, GRB_COLOR_BLACK);                                  // blank out the inside three columns...
    drawOverlay(glyphMap[currentBrightness / 10], GRB_COLOR_ORANGE);       // and overlay the brightness setting number
        
    while (M5.Btn.read() == 1);                     // don't proeeed until the button is released.
    btnWas = 0;                                     // initialize the click detector
    updateTimeout = millis() + 30000;               // delete this line if you don't want to bug out after a period of inactivity

    do {
        if (millis() >= updateTimeout) {                // delete this if clause if you don't want to bug out after a period of inactivity
            currentBrightness = brightnessNow;          // we timed out, user didn't confirm the change, restore currentBrightness as it was on entry...
            M5.dis.setBrightness(currentBrightness);    // reset the display to that brightness...            
            return;                                     // and head back to the barn
        }
        btnNow = M5.Btn.read();                         // read & refresh the state of the button
        if (buttonClicked()) {
            updateTimeout = millis() + 30000;           // delete this line if you don't want to bug out after a period of inactivity
            if(currentBrightness >= 60) currentBrightness = 10;
            else currentBrightness = currentBrightness + 10;                    
            M5.dis.setBrightness(currentBrightness);                            // set the display to the new brightness setting
            drawOverlay(GLF_EN, GRB_COLOR_BLACK);                               // blank out the inside three columns...
            drawOverlay(glyphMap[currentBrightness / 10], GRB_COLOR_ORANGE);    // and overlay the new brightness setting number
        }
        if (M5.Btn.pressedFor(1500)) {                              // user wants to confirm the change and exit so...
            drawGlyph(GLF_CK, gtgcolor);                            // let the user know we're good to go
            if (brightnessNow != currentBrightness) {                    // if the brightness level changed...
                stcPrefs.begin("STCPrefs", false);                      //  open up the pref namespace for R/W
                stcPrefs.putUChar("curBright", currentBrightness);      //  save the new brightness in NVS
                stcPrefs.end();                                         //  close the prefs namespace
            }
            while (M5.Btn.read() == 1);                             // don't proeeed until the button is released.
            delay(500);
            return;
        }
    } while (true);
    return;
    
}    // closing brace for updateBrightness()

void drawGlyph(uint8_t glyph[], int colors[]) {
/*  Draws a bit mapped image onto the Atom display.
      - glyph[] is the bitmap array of the glyph to draw.
      - in the pair of colors[]; a "0" at glyph[i] will draw color[0]; a "1", color[1]
      - or if you like, "0" and "1" in glyph[i] are the background/foreground color selectors at that pixel location.
*/

  for (int i = 0; i <= 24; i++) {
    M5.dis.drawpix(i, colors[glyph[i]]);
  }
}   // closing brace for drawGlyph()

void drawOverlay(uint8_t glyph[], int ovColor) {
/*  Overlays a bit mapped image onto the Atom display.
      - glyph[] is the bitmap array of the glyph to draw.
      - a "1" at glyph[i] will draw ovColor at that pixel location; a "0" will leave that pixel as it is
      - ovColor is a CGRB type
*/

    for (int i = 0; i <= 24; i++) {
        if (glyph[i] == 1) M5.dis.drawpix(i, ovColor);
    }

}   // closing brace for drawOverlay()

void flashDisplay(int count, int rate, int brightness) {
/*  Flashes the display brightness between 0 and the brightness level passed.
    - count is the number of times to flash the display
    - rate is the speed at which the display flashes in ms
    - function is blocking
*/

  for (int i = 0; i < count; i++) {
      M5.dis.setBrightness(0);
      delay(rate/2);
      M5.dis.setBrightness(brightness);
      delay(rate/2);
  }
  // return;

}   // closing brace for flashDisplay()

void pulsePix(bool state, CRGB colour) {
/*  turns the four outermost pixels of the display off or on
 *  depending on the state of "state" and using the colour "colour"
 */
    if (state) {
        M5.dis.drawpix(0, colour);
        M5.dis.drawpix(4, colour);
        M5.dis.drawpix(20, colour);
        M5.dis.drawpix(24, colour);
    }
    else {
        M5.dis.drawpix(0, 0x000000);
        M5.dis.drawpix(4, 0x000000);
        M5.dis.drawpix(20, 0x000000);
        M5.dis.drawpix(24, 0x000000);
    }

}   // closing brace for pulsePix()

boolean buttonClicked() {
/* Non-blocking funtion that returns true if the display button 
   has been clicked since the last time the function was called
   (looks for a "last time it was 1" to a "now it is 0" state).
   Returns false otherwise.
 
   Requires two variables to be declared within the same scope from where this function is called:
       - uint8_t btnWas = 0, btnNow;
   The value of "btnWas" must be set to 0 before every use of this fn
       btnWas = 0;
*/ 
    bool bc = false;
    
    if (btnNow == 1) {
        btnWas = btnNow;
        // bc = false;
    }
    else if (btnNow == 0 && btnWas == 1) {
        btnWas = 0;
        bc = true;
    }
    return bc;
    
}    // closing brace for buttonClicked()

void sendForm(WiFiClient theClient, String &swVer) {
/* Sends the HTML configuration form web page to the user's browser
 HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
 and a content-type so the client knows what's coming, then a blank line
 then an optional payload (data) and then a blank line
*/
  
    theClient.println("HTTP/1.1 200 OK");
    theClient.println("Content-type: text/html");
    theClient.println("Accept-Language: en-us");
    theClient.println("Cache-Control: no-store");
    theClient.println("Connection: Keep-Alive");
    theClient.println();
    theClient.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"></head>");
    theClient.println("<link rel=\"icon\" href=\"data:,\">");   // prevent 'favicon' requests
    theClient.println("<body><h1 align=\"center\"><font face=\"Helvetica, Arial, sans-serif\">STAC Configuration</font></h1><div align=\"center\">");
    theClient.println("<form method=\"post\"><font face=\"Helvetica, Arial, sans-serif\">");
    theClient.println("<label for=\"SSID:\">Network SSID:</label><input id=\"SSID\" name=\"SSID\" autofocus=\"\" required=\"\" type=\"text\" maxlength=\"32\"><br><br>");
    theClient.println("<label for=\"Password:\">Password:</label><input id=\"pwd\" name=\"pwd\" type=\"text\" size=\"20\" maxlength=\"63\"><br><br>");
    theClient.println("<label for=\"Smart Tally IP:\">Smart Tally IP:</label><input id=\"stIP\" name=\"stIP\" size=\"15\" ");
    theClient.println("pattern=\"^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$\" required=\"\" type=\"text\"><br><br>");
    theClient.println("<label for=\"stPort\">port #:</label><input id=\"stPort\" name=\"stPort\" size=\"5\" min=\"0\" max=\"65353\" required=\"\" type=\"number\" value=\"80\"><br><br>");
    theClient.println("<label for=\"stChan\"># of channels:</label><input id=\"stChan\" name=\"stChan\" size=\"3\" min=\"1\" max=\"8\" required=\"\" type=\"number\" value=\"6\"><br><br>");
    theClient.println("<input value=\"Submit\" type=\"submit\">&nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp; &nbsp;<input type=\"Reset\"></font></form><br>");
    theClient.print("<p align=\"center\"><font face=\"Helvetica, Arial, sans-serif\">STAC software version: ");
    theClient.print(swVer);
    theClient.println("</font></p></body>");
    theClient.println();
    theClient.println();
     
}   // closing brace for sendForm()

void send404(WiFiClient theClient) {
/*  Sends a "not found" HTML response to the user's web browser
 */
    
    theClient.println("HTTP/1.1 404 Not Found");
    theClient.println("Cache-Control: no-store, max-age=0");
    theClient.println("Connection: Keep-Alive");
    theClient.println();
    theClient.println();

}   // closing brace for send404()

void sendtftf(WiFiClient theClient) {
/*  Sends the HTML page acknowledging receipt of the 
 *  configuration data back to the users web browser.
 */

    theClient.println("HTTP/1.1 200 OK");
    theClient.println("Content-type: text/html");
    theClient.println("Cache-Control: no-store");
    theClient.println("Clear-Site-Data: \"*\"");
    theClient.println("Connection: closed");
    theClient.println();
    theClient.println("<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=0.86, maximum-scale=1.0, minimum-scale=0.86\"></head><body>");
    theClient.println("<div align=\"center\"><h1><font face=\"Helvetica, Arial, sans-serif\">");
    theClient.println("STAC configuration received.<br>");
    theClient.println("</font></h1><font size=\"+2\" face=\"Helvetica, Arial, sans-serif\">");
    theClient.println("Close this window and reconnect <br>");
    theClient.println("to your regular WiFi network.<br><br>");
    theClient.println("Consult the manual <br>");
    theClient.println("if you need to reconfigure this device.");
    theClient.println("</font></div></body></html>");
    theClient.println();
    theClient.println();
    theClient.flush();      // wait and make sure everything has been sent 
                           // (important to do this time as we shut down the AP after sending this response back to the user's browser)
    
}   // closing brace for sendtftf()

provData_t parseForm(String &formData) {
/*  Extracts the data from the POST response returned by the client web browser
 *      - Example last line of a POST response:
 *        SSID=SandyShores222&pwd=flatBUSH%40%26%24%29%28%3B%3A%2F-3546&stIP=192.168.2.132&stPort=80&stChan=7
 */

    String payload = "";
    String tempString;
    unsigned int llIndex;
    provData_t stCred;
 
    llIndex = formData.lastIndexOf("SSID=");                                // the last line of the POST reply is the only thing we want
    if (llIndex != -1) payload = formData.substring(llIndex);               // throw away everything except the last line
    else payload = formData;                                                // should never need this check but just in case...
    payload.trim();                                                         // knock any junk off the front and back of the last line of the reply
    
    // parse the payload from the end of the string to the start
    tempString = payload.substring(payload.lastIndexOf("&") + 8);           // pull out the max # of tally channels; 8 = length of "&stChan="...
    stCred.ptChanMax = (uint8_t)tempString.toInt();                         // convert (cast) it to an 8-bit uint and save it   
    payload = payload.substring(0, payload.lastIndexOf("&"));               // throw away the part of payload that held the tally channel

    tempString = payload.substring(payload.lastIndexOf("&") + 8);           // pull out the port #; 8 = length of "&stPort="...
    stCred.pPort = (uint16_t)tempString.toInt();                            // convert (cast) it to a uint16_t and save it      
    payload = payload.substring(0, payload.lastIndexOf("&"));               // throw away the part of payload that held the port number

    stCred.pSwitchIP = payload.substring(payload.lastIndexOf("&") + 6);     // pull out IP address of the Roland switch and save it; 6 = length of "&stIP=" 
    payload = payload.substring(0, payload.lastIndexOf("&"));               // throw away the part of payload that held the IP address
    
    tempString = payload.substring(payload.lastIndexOf("&") + 5);           // pull out the WiFi password; 5 = length of "&pwd="...
    stCred.pPass = decodeURL(tempString.c_str());                           // decode the password and save it
    payload = payload.substring(0, payload.lastIndexOf("&"));               // throw away the part of payload that held the password
    
    tempString = payload.substring(5);                                      // pull out the SSID of the WiFi network; 5 = length of "SSID="

    stCred.pSSID = decodeURL(tempString.c_str());                           // decode the SSID and save it

    return stCred;

}   // closing brace for parseForm()

String decodeURL(const char* ptr) {
/* Converts a URL encoded string back to its plain text form 
   
   Usage: 
    const char* stringToDecode = "abcd%24%26%40%29%28-987";
    String decodedString = decodeURL(stringToDecode);
     - or, putting it another way -
    String stringToDecode = "abcd%24%26%40%29%28-987";
    String decodedString = decodeURL(stringToDecode.c_str());

   Example:
    Encoded URL: abcd%24%26%40%29%28-987
    Decoded plain text: abcd$&@)(-987

   Modified 2021-04-23 to decode + chars to spaces

Credit to reddit user u/truetofiction
Source: https://www.reddit.com/r/arduino/comments/m2uw0g/replacing_hex_with_ascii/
*/

    const size_t size = strlen(ptr);                // size of the string, in characters
    String buffer;  
    buffer.reserve(size);                           // buffer on heap in String object

     for (size_t i = 0; i < size; i++) {
        char c = ptr[i];                            // get character       
        if (c == '%' && i + 2 < size) {             // if hex follows...
            char tmp[3]{};
            memcpy(tmp, &ptr[i + 1], 2);            // make a temporary copy of the hex data (i+1, i+2)
            c = (char) strtol(tmp, nullptr, 16);    // convert from hex string to long (ASCII)
            i += 2;                                 // skip the characters we just processed for the next loop
        }
        else {
            if (c == '+') {
                c = ' ';                 // decode a + to a space
            }
        }
        buffer.concat(c);                           // save the current character
    }
    return buffer;

}   // closing brace for decodeURL()

provData_t getCreds(String &ssidPrefix, String &swVersion) {
/*  Big honkin' routine that retrevies the configuration/provisiong data for 
 *  the WiFi network that the Roland switch is connected to.
 *      - sets the STAC as a WiFi access point using a device unique SSID
 *      - waits for the user's web browser to connect
 *      - sends the form to the browser for the user to fill out
 *      - waits for the form to be returned
 *      - confirms to the user that the form was received (on the browser and on the STAC)
 *      - shuts down the access point
 *      - returns the configuration/provisiong data to the calling function
 *      - and then goes for a beer.
 *
 *  The IP and password for the access point are set in this function.
 */

    provData_t stProv;                          // structure to hold the data from the web page form from the user

    const char* password = "1234567890";        // the password for our WiFi AP
    IPAddress configIP(192, 168, 6, 14);        // sets the IP...
    IPAddress gateway(192, 168, 6, 14);         // ...gateway...
    IPAddress NMask(255, 255, 255, 0);          // ...and network mask of the AP
    WiFiServer server(80);                      // init a server class & set the AP to listen for inbound connections on port 80
    bool hideSSID = false;                      // false to broadcast the SSID of our AP network, true to hide it
    uint8_t wifiChan = 1;                       // WiFi channel to use. Default is 1, max is 13
    uint8_t maxConnect = 1;                     // maximum # of client connections allowed to our AP
    WiFiClient scClient;                        // we need a WiFi client to talk to the user's web browser
    
    String clData = "";                         // for holdng the input from the user's web browser
    bool formReceived;                          // true when we get the POST form back from the user's web browser

                                                            // create the device unique AP SSID
    uint32_t chiptID = (uint32_t)(ESP.getEfuseMac() >> 16); // grab the top four bytes of the chip id and...
    String tempx = ssidPrefix + String(chiptID, HEX);       // ...use that to create the last part of the unique SSID for our AP
    tempx.toUpperCase();                                    // flip any hex alphas to upper case
    char apssid[tempx.length() + 1];                        // create the C-string style char array to hold the SSID and...
    tempx.toCharArray(apssid, tempx.length() + 1);          // ...copy the String into the apssid char array

    // set up the WiFi access point
    WiFi.mode(WIFI_AP);                                                 // configure the WiFi mode to AP 
    while (WiFi.getMode() != 2) delay(10);                              // best practise to ensure we're configured in AP mode. "2" = "WIFI_AP_CONNECTED" (or something like it).
    WiFi.softAP(apssid, password, wifiChan, hideSSID, maxConnect);      // set the SSID, password, etc. of our AP (all the WiFi stuff)
    WiFi.softAPConfig(configIP, gateway, NMask);                        // set the IP address, gateway and network mask of our AP (all the networking stuff)
    server.begin();                                                     // fire up the AP server

    stClient.setTimeout(0);                     // the Stream.readString() function has a 1s blocking 'wait for' by default. This sets it to X ms.

    // let's go fetch the info from the user's web browser...
    formReceived = false;
    do {
        scClient = server.available();                                                  // listen for incoming clients
        if (scClient) {                                                                 // if a new client connects...
            clData = "";                                                                // clear out any old client data received                        
            while (scClient.connected()) {                                              // loop while the client's connected
                if (scClient.available()) {                                             // if there's something from from the client...
                    while (scClient.available()) clData += scClient.readString();       // suck in the entire response from the client (timeout for the .readString() is set above)
                    if (clData.indexOf("GET / HTTP/") >= 0) {                           // goody! a root request...
                        formReceived = false;                                           // but we're stil waiting on the form...
                        sendForm(scClient, swVersion);                                  // so send the form
                        break;                                                          // break out of while (scClient.connected())
                    }           
                    else if (clData.indexOf("POST /") >= 0) {                           // incoming form response from the user's web browser!!
                        formReceived = true;                                            // this is the data we're looking for!
                        sendtftf(scClient);                                             // acknowledge to the user receipt of the form and send the "so long and thanks for all the fish" page
                        break;                                                          // break out of while (scClient.connected())
                    }
                    else                                                                // the client is asking for somethig we don't have
                    formReceived = false;                                               // this isn't the reply we're looking for.                   
                    send404(scClient);                                                  // send a no can do response
                    break;                                                              // break out of while (scClient.connected())
                }   // closing brace for if (scClient.available())               
            }   // closing brace for while (scClient.connected())
        }   // closing brace for if (scClient)
    } while (!formReceived);                                                            // stay here forever until the POST is received
    
                                            // we got the goods so, shut down the access point
    scClient.stop();                        // stop listening for incoming stuff
    delay(500);                             // seems to be needed for the client browser to accept the final response
    server.end();                           // close the connection
    delay(500);                             // seems to be needed for the client browser to accept the final response
    WiFi.mode(WIFI_MODE_NULL);              // turn off the WiFi AP
    
    stProv = parseForm(clData);             // extract the data from the POSTed form...
    return stProv;                          // and head back to the barn

}   // closing brace for getCreds()


//
//  Code to retrieve the current orientation of the STAC
//

ORIENTATION getOrientation( )
{
    ORIENTATION stacState = ORIENTATION::UP;
  
    if(Accelerometer)                               // Make sure the accelerometer is supported by this HW
    {
        float accX=0, accY=0, accZ = 0 ;      
      
        M5.IMU.getAccelData(&accX, &accY, &accZ);   // Call into the accelerometer to get the current values 
        
        float scaledAccX = accX * 1000;
        float scaledAccY = accY * 1000;
        float scaledAccZ = accZ * 1000;

        if( ( abs(scaledAccX) < HIGH_TOL ) && ( abs(scaledAccY) > MID_TOL ) && ( abs(scaledAccZ) < HIGH_TOL ) )
        {
            if ( scaledAccY > 0 )                   // Device is oriented Up            
            {
                stacState = ORIENTATION::UP;
                // Serial.println( "Scaled Y > 0,  Device is oriented Up" ) ;
            }
            else // scaledAccY < 0                  // Device is oriented Down
            {
                stacState = ORIENTATION::DOWN;
                // Serial.println( "Scaled Y < 0,  Device is oriented Down" ) ;
            }
        }
        else if( ( abs(scaledAccX) > MID_TOL ) && ( abs(scaledAccY) < HIGH_TOL ) && ( abs(scaledAccZ) < HIGH_TOL ) )
        {
            if( scaledAccX > 0 )                    // Device is oriented Right
            {
                stacState = ORIENTATION::RIGHT;
                // Serial.println( "Scaled X > 0,  Device is oriented Right" ) ;
            }
            else // scaledAccX < 0                  // Device is oriented Left
            {
                stacState = ORIENTATION::LEFT;
                // Serial.println( "Scaled X < 0,  Device is oriented Left" ) ;
            }
        }
        else
        {
            // Serial.printf( "Orientation is flat or clost to flat" );
        }
    }

    return stacState ;
}

//
//  Code to rotate the Glyphs for STAC
//

void rotateGlyphs( ORIENTATION stacOrientation )
{
    // Initalize the rotation vectors
    int rotate_0[25] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24};
    int rotate_90[25] = {20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4};
    int rotate_180[25] = {24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0};
    int rotate_270[25] = {4, 9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20};
  
    // Initalize the rotation LUT
    int * rotation_LUT = NULL ;
  
    // Determine which rotation to use
    if ( stacOrientation == ORIENTATION::DOWN )                 // Upside Down
      rotation_LUT = rotate_180 ;
    else if ( stacOrientation == ORIENTATION::LEFT )            // Rotated to left 
      rotation_LUT = rotate_90 ;
    else if ( stacOrientation == ORIENTATION::RIGHT )           // Rotated to Right
      rotation_LUT = rotate_270 ;
    else
      rotation_LUT = rotate_0 ; 
  
    // Perform the rotation
    for ( int glyphID = 0 ; glyphID< TOTAL_GLYPHS; glyphID++ )
    {
        for ( int loop_pix = 0; loop_pix < 25; loop_pix++ )
        {   
          glyphMap[glyphID][ loop_pix ] = baseGlyphMap[glyphID][ rotation_LUT[ loop_pix ] ] ;
        }
    }
    
    return ;
}




// ***** End Function Definitions *****

// And so it begins...

void setup() {

    provData_t sConfigData;                 // structure to hold the WiFi provisioning data from user's web browser
    bool provisioned;                       // true if the WiFi provisioning has been done
    String bootVer;                         // the version of software the STAC thinks it has - from NVS storage at boot time

    // M5.begin(SerialEnable = true|false, I2CEnable = true|false, DisplayEnable = true|false);
    M5.begin(true, false, true);
    delay(500);
    M5.Btn.read();                          // initialize the Btn class
    M5.dis.clear();
    M5.dis.setBrightness(10);
    M5.dis.drawpix(poPixel, poColor);                   // turn on the power LED

    // send the serial port info dump header
    Serial.println("\r\n\r\n======================================");
    Serial.println("             STAC");
    Serial.println("A Smart Tally ATOM Matrix Client");
    Serial.println("        by: Xylopyrographer");
    Serial.println("https://github.com/Xylopyrographer/STAC\r\n");
    Serial.print("    Software Version: ");
    Serial.println(swVer);
    // end send the serial port info dump header

    delay(1000);                                            // delay a second for the "GUI"
    
    stcPrefs.begin("STCPrefs", true);                       // open our preferences in R/O mode.
    provisioned = stcPrefs.getBool("pVis", false);          // check to see if we've been provisioned already. provisioned = false if namespace doesn't exist
    bootVer = stcPrefs.getString("swVersion", "-1");        // get the software version stored in NVS
                                                            //  - it'll be -1 if this key doesn't exist (because older versions didn't store it in NVS)
    stcPrefs.end();

    if (provisioned && (bootVer != swVer)) {     // if we've been provisioned before but the current SW load is different than the last...
        
        stcPrefs.begin("STCPrefs", false);      // open the namespace in R/W mode
        stcPrefs.clear();                       // wipe our namespace...
        stcPrefs.end();                         // close the preferences...
        provisioned = false;                    // we are no longer provisioned       
    }

    if (!provisioned) {                         // if not ever provisioned before (or we wiped the namespace because of a new SW load)...
        // add to the serial port info dump
        Serial.println("   ***** STAC NOT PROVISIONED *****");
        Serial.println("======================================");
        // end add to the serial port info dump
                
        stcPrefs.begin("STCPrefs", false);      // create and open the namespace in R/W mode     
                                                // we need to create the namespace key entries first by 'reading' or 'getting' the key.
        stcPrefs.getBool("pVis");
        stcPrefs.getBool("ctMde");
        stcPrefs.getBool("aStart");
        stcPrefs.getUChar("curBright");
        stcPrefs.getUChar("talChan");
        stcPrefs.getUChar("talMax");
        stcPrefs.getUShort("stswPort");
        stcPrefs.getString("swVersion");
        stcPrefs.getString("stnSSID");
        stcPrefs.getString("stnPass");
        stcPrefs.getString("stswIP");           // the namespace key entries are now created.
                                                
        stcPrefs.putBool("pVis", false);        // now store the initial "factory default" values
        stcPrefs.putString("swVersion", swVer);
        stcPrefs.putUChar("curBright", 10);
        stcPrefs.putUChar("talChan", 1);
        stcPrefs.putUChar("talMax", 6);
        stcPrefs.putBool("ctMde", true);
        stcPrefs.putBool("aStart", false);
        stcPrefs.end();                         // "factory default" values are saved
                                                
        drawGlyph(GLF_CFG, alertcolor);                     // let the user know this is a first time run - configuration required
        flashDisplay(4, 500, 20);
        M5.dis.setBrightness(10);
        
        while (M5.Btn.read() == 1);                         // wait for the button to be released
        
    }   // end brace for if (!provisioned)

    if (M5.Btn.read() && provisioned) {                     // if the button is down at restart and we are provisioned...
        drawGlyph(GLF_CFG, warningcolor);                   //      (let the user know they are about to change the provisioning info)
        flashDisplay(4, 500, 20);
        M5.dis.setBrightness(10);
        unsigned long resetTime = millis() + 2000;
        provisioned = false;                                // ... then the user wants to reconfigure the STAC, unless...
        while (M5.Btn.read()) {
            if (millis() >= resetTime) {                    // ... the button is long pressed, in which case...
                drawGlyph(GLF_FM, alertcolor);              // ... the user wants to do a factory reset.
                drawOverlay(GLF_CK, GRB_COLOR_GREEN);       // confirm to the user...
                
                // add to the serial port info dump
                Serial.println(" ***** PERFORMING FACTORY RESET *****");
                Serial.println("======================================");
                // end add to the serial port info dump
                
                stcPrefs.begin("STCPrefs", false);          // open the preferences in R/W mode...
                stcPrefs.clear();                           // wipe our namespace...
                stcPrefs.end();                             // close the preferences...
                while (M5.Btn.read());                      // wait for the button to be released            
                delay(1000);                                // wait a bit for the "GUI"
                M5.dis.clear();                             // clear the display
                ESP.restart();                              // restart the STAC
            }
        }
    }   // closing brace for if button down
        
    if (!provisioned) {
        Serial.println(" ***** WAITING FOR PROVISIONING *****");
        Serial.println("======================================");
        
        sConfigData = getCreds(apPrefix, swVer);            // go get the WiFi provisioning data from the user's web browser
        
        drawGlyph(GLF_CK, gtgcolor);                        // confirm to the user we got the provisioning data
        delay(1000);                                        // park a bit for the "GUI"
        
        stcPrefs.begin("STCPrefs", false);                  // open our preferences in R/W mode
        stcPrefs.putString("stnSSID", sConfigData.pSSID);   // save the provisioning data
        stcPrefs.putString("stnPass", sConfigData.pPass);
        stcPrefs.putString("stswIP", sConfigData.pSwitchIP);
        stcPrefs.putUShort("stswPort", sConfigData.pPort);
        stcPrefs.putUChar("talMax", sConfigData.ptChanMax);
        if (stcPrefs.getUChar("talChan") > sConfigData.ptChanMax) {     // make sure the active tally channel is not > max tally
            stcPrefs.putUChar("talChan", 1);
        }
        stcPrefs.putBool("pVis", true);
        stcPrefs.end();
        provisioned = true;
        Serial.println("  ***** PROVISIONING COMPLETE *****");
        Serial.println("======================================");
    }

    // go get & set the runtime ops parms from NVS :)     
    stcPrefs.begin("STCPrefs", true);                           // open our preferences in R/O mode
    currentBrightness = stcPrefs.getUChar("curBright");         // read and set the operational parameters from NVS
    tallyStatus.tChannel = stcPrefs.getUChar("talChan");
    tallyStatus.tChanMax = stcPrefs.getUChar("talMax");
    ctMode = stcPrefs.getBool("ctMde");
    autoStart = stcPrefs.getBool("aStart");
    
    String tempstring;   
    tempstring = stcPrefs.getString("stnSSID") + '\0';          // ST WiFi SSID. Make it a C-string  
    tempstring.toCharArray(networkSSID, tempstring.length());   

    tempstring = stcPrefs.getString("stnPass") + '\0';          // ST WiFi Password. Make it a C-string       
    tempstring.toCharArray(networkPass, tempstring.length());

    tempstring = stcPrefs.getString("stswIP") + '\0';           // ST switch IP address. Make it a C-string    
    tempstring.toCharArray(stIP, tempstring.length());

    stPort = stcPrefs.getUShort("stswPort");                    // ST port number
 
    stcPrefs.end();                                             // close our preferences namespace
    // runtime ops parms are set

    M5.dis.setBrightness(currentBrightness);
    tallyStatus.tState = "7";                                   // initialize the control flags
    tallyStatus.tConnect = false;
    tallyStatus.tTimeout = true;
    tallyStatus.tNoReply = true;
    wifiStatus.wfconnect = false;
    wifiStatus.timeout = false;

    M5.dis.clear();
    M5.dis.drawpix(poPixel, GRB_COLOR_GREEN);                   // turn the power LED green

    // add to the info dump to the serial port   
    uint32_t chiptID = (uint32_t)(ESP.getEfuseMac() >> 16);     // grab the top four bytes of the chip id and...
    String tempx =  apPrefix + String(chiptID, HEX);            // ...use that to create the last part of the unique SSID for our AP
    tempx.toUpperCase();                                        // flip any hex alphas to upper case

    Serial.print("    WiFi Network SSID: ");
    Serial.println(networkSSID);
    Serial.print("    Smart Tally IP: ");
    Serial.println(stIP);
    Serial.print("    Port #: ");
    Serial.println(stPort);
    Serial.print("    Auto start: ");
        if (autoStart) Serial.println("Enabled");
        else Serial.println("Disabled");
    Serial.print("    Operating Mode: ");
        if (ctMode) Serial.println("Camera Operator");
        else Serial.println("Talent");
    Serial.print("    Active Tally Channel: ");
    Serial.println(tallyStatus.tChannel);
    Serial.print("    Max Tally Channel: ");
    Serial.println(tallyStatus.tChanMax);
    Serial.print("    Brightness Level: ");
    Serial.println(currentBrightness / 10);
    Serial.print("    Configuration SSID: ");
    Serial.println(tempx);
    Serial.println("======================================");
    // end add to the info dump to the serial port

    delay(1000);                                                            // all the background setup is done. Whew. Almost there... Pause for the "GUI"

    // Pull data from the accelerometer and determine the orientation
    Accelerometer = M5.IMU.Init() == 0 ;
    if( !Accelerometer )
      Serial.println( "Could not initalize the accelerometer" ) ; 

    ORIENTATION stacOrientation = getOrientation() ;                        // Go check the orientation of the STAC
    rotateGlyphs( stacOrientation ) ; 

    drawGlyph(glyphMap[tallyStatus.tChannel], bluecolor);       // do this here as setting the tally channel is the first thing we do in the user setup stuff.
                                                                // also gives the user some feedback so they can see we've transitioned out of doing all the setup stuff
    while (M5.Btn.read() == 1);                                 // wait for button to be released

    bool asBypass = false;                                      // get set up for the auto start detect and control stuff
    if (autoStart) {                                                // if we're in auto start mode...
        asBypass = true;
        CRGB asPulseColour = 0xee0000;
        pulsePix( true, asPulseColour);                             // ...turn on the four corner LEDs
        bool nextonstate = false;
        unsigned long asTimeOut =  millis() + 20000;
        unsigned long nextFlash = millis() + 1000;
        while (asTimeOut >= millis() && M5.Btn.read() == 0) {       // ...pause until we time out (continue with auto start)
            if (nextFlash <= millis()) {                            // ...have some fun & flash the four corner LED's while waiting :)
                pulsePix(nextonstate, asPulseColour);
                nextonstate = !nextonstate;
                nextFlash = millis() + 1000;
            }
        }
        if (M5.Btn.read() == 1) {                                   // unless the button is pressed, in which case...
            asBypass = false;                                       // ...cancel auto start
        }
    }

    /* Setting up to display & change the current (active) tally channel
     *  
     * If the button is clicked, drop out
     * If the button is long pressed, call changeTallyChannel() and return to the start of
     *  "Setting up to display the current tally channel"
    */
    if (!asBypass) {                                              // skip everything here if autostart kicked in
//        drawGlyph(glyphMap[tallyStatus.tChannel], bluecolor);   // display the current tally channel and... (put this back if this routine is not the first in the setup sequence)    
//        while (M5.Btn.read() == 1);   // wait for button to be released. If this routine ever gets moved from being the first thing after the autostart check, this has to go back here
        escapeFlag = false;
        btnWas = 0;
        
        do  {
            btnNow = M5.Btn.read();                     // read & refresh the state of the button
            if (buttonClicked()) escapeFlag = true;           
            if (M5.Btn.pressedFor(1500)) {
                changeTallyChannel();
                drawGlyph(glyphMap[tallyStatus.tChannel], bluecolor);   // display the (new) current tally channel
                btnWas = 0;     // reset the click detector & loop back to the top of the do loop. Need to keep this here!
            }
        } while (!escapeFlag);
    }

    /* Setting up to display & change the current tally mode
     * 
     * If the button is clicked, drop out
     * If the button is long pressed, call changeTallyMode() and return to the start of
     *  "setting up to display the current tally mode"
    */
    if (!asBypass) {                                            // skip everything here if autostart kicked in
        if (ctMode) drawGlyph(GLF_C, purplecolor);              // display the current tally mode...
        else drawGlyph(GLF_T, purplecolor);
        
        while (M5.Btn.read() == 1);                             // wait for button to be released
        escapeFlag = false;
        btnWas = 0;
        
        do  {
            btnNow = M5.Btn.read();                             // read & refresh the state of the button
            if (buttonClicked()) escapeFlag = true;
            if (M5.Btn.pressedFor(1500)) {
                changeTallyMode();
                if (ctMode) drawGlyph(GLF_C, purplecolor);      // display the (new) current tally mode...
                else drawGlyph(GLF_T, purplecolor);
                btnWas = 0;     // reset the click detector & loop back to the top of the do loop. Need to keep this here!
            }
        } while (!escapeFlag);
    }

    /* Setting up to display & change the startup mode
     *  
     * If the button is clicked, drop out
     * If the button is long pressed, call changeStartupMode() and return to the start of
     *  "Setting up to display & change the startup mode"
    */
    if (!asBypass) {                                            // skip everything here if autostart kicked in
        if (autoStart) drawGlyph(GLF_A, tealcolor);             // display the current startup mode
        else drawGlyph(GLF_S, tealcolor);
        
        while (M5.Btn.read() == 1);                             // wait for button to be released
        escapeFlag = false;
        btnWas = 0;
        
        do  {
            btnNow = M5.Btn.read();                             // read & refresh the state of the button
            if (buttonClicked()) escapeFlag = true;           
            if (M5.Btn.pressedFor(1500)) {
                changeStartupMode();
                if (autoStart) drawGlyph(GLF_A, tealcolor);     // display the current startup mode              
                else drawGlyph(GLF_S, tealcolor);
                btnWas = 0;     // reset the click detector & loop back to the top of the do loop. Need to keep this here!
            }
        } while (!escapeFlag);
    }

    /* Setting up to display & change the current display brightness level
     * 
     * If the button is clicked, drop out
     * If the button is long pressed, call changeBrightness() and return to the start of
     *  "Setting up to display the current brightness level"
    */
    if (!asBypass) {                                                        // skip everything here if autostart kicked in
        drawGlyph(GLF_CBD, brightnessset);                                  // draw the checkerboard test pattern...
        drawOverlay(GLF_EN, GRB_COLOR_BLACK);                               // blank out the inside three columns...
        drawOverlay(glyphMap[currentBrightness / 10], GRB_COLOR_WHITE);     // and overlay the brightness setting number
    
        while (M5.Btn.read() == 1);                                         // wait for button to be released
        escapeFlag = false;
        btnWas = 0;
        
        do  {
            btnNow = M5.Btn.read();                                         // read & refresh the state of the button
            if (buttonClicked()) escapeFlag = true;
            if (M5.Btn.pressedFor(1500)) {
                updateBrightness();          
                drawGlyph(GLF_CBD, brightnessset);                                  // draw the checkerboard test pattern...
                drawOverlay(GLF_EN, GRB_COLOR_BLACK);                               // blank out the inside three columns...
                drawOverlay(glyphMap[currentBrightness / 10], GRB_COLOR_WHITE);     // and overlay the (new) brightness setting number
                btnWas = 0;     // reset the click detector & loop back to the top of the do loop. Need to keep this here!
           }
        } while (!escapeFlag);
    }
    
    M5.dis.clear();
    M5.dis.drawpix(poPixel, GRB_COLOR_GREEN);
    delay (1000);                // chill for a second for the sake of the "GUI"
    nextPollTime = millis();     // set the initial value for the ST Server poll timer; should cause the ST Server to be polled the first time into loop()

}   // closing brace for setup()

void loop() {

    M5.update();        // put this at the top of loop() instead of the bottom so it always gets hit
    
    /* ~~~~~ Update Brightness contol logic ~~~~~ */

    if (M5.Btn.pressedFor(1500)) {
        updateBrightness();
        M5.dis.clear();
        M5.dis.drawpix(poPixel, poColor);
        nextPollTime = millis();                // force a repoll of the tally state
        lastTallyState = "11";
    
    }   // closing brace for Update Brightness contol logic
    
    /* ~~~~~ Connect to WiFi control logic ~~~~~ */

    if (WiFi.status() != WL_CONNECTED) {
        if (wifiStatus.wfconnect) {             // if we had a previous good WiFi connection...
            wifiStatus.wfconnect = false;       // reset the control logic...
            wifiStatus.timeout = false;         
            tallyStatus.tState = "9";
            tallyStatus.tConnect = false;
            tallyStatus.tTimeout = true;
            tallyStatus.tNoReply = true;        // ...flags & varialbles
            if (ctMode) {                                   
                drawGlyph(GLF_WIFI, alertcolor);            // let the user know we lost WiFi... 
                flashDisplay(8, 300, currentBrightness);
                delay(1500);                                // pause and then carry on to try and reconnect.
            }
            else {
                M5.dis.fillpix(GRB_COLOR_GREEN);
                M5.dis.drawpix(poPixel, poColor);         // turn on the power LED
            }
        }
        drawGlyph(GLF_WIFI, warningcolor);          // draw the "attempting to connect to WiFi" thing on the display.
        wifiStatus = connectToWifi(wifiStatus);        
        if (wifiStatus.wfconnect) {
            drawGlyph(GLF_WIFI, gtgcolor);          // draw the "connected to WiFi" thing on the display.
            delay(1500);
            M5.dis.clear();
            M5.dis.drawpix(poPixel, poColor);
        }
        else if (wifiStatus.timeout) {
            drawGlyph(GLF_WIFI, alertcolor);    // flash the "no WiFi" thing on the display.
            flashDisplay(3, 500, currentBrightness);
            delay(1500);
            return;                             // bail to to the top of loop() to try again to connect to WiFi
        }
        else {
            if (ctMode) {
                drawGlyph(GLF_QM, alertcolor);              // something is wrong with the control logic!!!
                flashDisplay(10, 200, currentBrightness);
            }
            else {
                M5.dis.fillpix(GRB_COLOR_GREEN);          // Change the display to the PVW colour
                M5.dis.drawpix(poPixel, poColor);         // turn on the power LED
            }
            delay(10000);
            return;
        }
    }

    /* ~~~~~ Get Tally State Control logic ~~~~~ */

    if (millis() >= nextPollTime && wifiStatus.wfconnect) {
        tallyStatus = getTallyState(tallyStatus);
        if (!tallyStatus.tConnect || tallyStatus.tTimeout || tallyStatus.tNoReply) {
            if (ctMode) {
                drawGlyph(GLF_BX, purplecolor);          // throw up the big purple X...
            }
            else {
                M5.dis.fillpix(GRB_COLOR_GREEN);        // else change the display to the PVW colour
                M5.dis.drawpix(poPixel, poColor);       // turn on the power LED
            }
            lastTallyState = "10";
            nextPollTime = millis();                    // force a re-poll next time through loop()
            return;                                     // assuming this takes you back to the start of loop()?
        }
        nextPollTime = millis() + ST_POLL_INTERVAL;
    }

    /* ~~~~~ Update Tally Display control logic ~~~~~ */

    if (tallyStatus.tState != lastTallyState) {
        lastTallyState = tallyStatus.tState;   
        if (tallyStatus.tState == "onair") {                    // was tallyStatus.tState == String("onair")
            M5.dis.fillpix(GRB_COLOR_RED);                      // Change the display to the PGM colour;
        }
        else if (tallyStatus.tState == "selected") {            // was tallyStatus.tState == String("selected")
            M5.dis.fillpix(GRB_COLOR_GREEN);                    // Change the display to the PVW colour
        }
        else if (tallyStatus.tState == "unselected") {          // was tallyStatus.tState == String("unselected")
            if (ctMode) {                                       // if in camera operator mode
                drawGlyph(GLF_DF, unselectedcolor);             // Change the display to the unselected glyph and colour
            }
            else {
                M5.dis.fillpix(GRB_COLOR_GREEN);                // else change the display to the PVW colour
            }
        }
        else {
            // Things have gone wrong big time.
            
            Serial.println("\r\n!!!!! TALLY CHANNEL IS IN AN *** UNKNOWN *** state. !!!!!\r\n");
           
            if (ctMode) {
                drawGlyph(GLF_QM, purplecolor);      // throw up a purple "?"...
            }
            else {
                M5.dis.fillpix(GRB_COLOR_GREEN);                // else change the display to the PVW colour
            }
            lastTallyState = "";
            return;                              // and bail back to the start of loop() 
        }
        M5.dis.drawpix(poPixel, poColor);         // turn on the power LED
    }

    // ~~~~~ End of the Control loop
    
}   // closing  brace for loop()


// - EOF -
