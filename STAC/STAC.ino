
// STAC.ino

//  STAC (Smart Tally Atom Client)
/*  For the M5Stack ATOM MATRIX device
 *
 *  Authors: Team STAC
 *
 *  A Roland Smart Tally Client
 *     Its purpose is to monitor via WiFi the tally status of a single video
 *     input channel of a Roland device that implements their Smart Tally protocol.
 *
 *     For the Roland video input channel being monitored,
 *     the STAC will set the colour of the display:
 *      + when in "Camera Operator" mode, to:
 *          - RED if the channel is in PGM (Progam or onair)
 *          - GREEN if the channel is in PVW (Preview or selected)
 *          - "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected)
 *      + when in "Talent" or "Peripheral" mode, to:
 *          - RED if the channel is in PGM (Progam or onair)
 *          - GREEN otherwise
 *
 *  Configuration of the STAC for the WiFi credentials, switch IP address, port number, number of
 *  tally channels, the polling interval and the Roland switch model is done using a web browser.
 *
 *  More at: https://github.com/Xylopyrographer/STAC
*/

const String swVer = "2.2.0 (2f1c36)";   // version and (build number) of this software

#include <WiFi.h>                   // does the magic for connecting to WiFi network
#include <WiFiClient.h>             // sets up the WiFi network communications
#include <HTTPClient.h>             // so we can create a client for certain Roland switch models
#include <WebServer.h>              // lets us create a webserver to configure the STAC
#include <Update.h>                 // lets us do OTA software updates
#include <ESPmDNS.h>                // enables the mDNS service
#include <Esp.h>                    // to get the ESP-IDF SDK version
#include <esp_arduino_version.h>    // to get the arduino-esp32 core version
#include <Preferences.h>            // for getting and setting stuff in NVS memory
#include <Wire.h>                   // for using I2C peripherals
#include <LiteLED.h>                // for driving the ATOM matrix display
#include <XP_Button.h>              // for driving the ATOM display button
#include <I2C_MPU6886.h>            // for driving the ATOM IMU

// bring in the STAC routines
#include "./STACLib/STACInclude.h"

// And so it begins...

void setup() {

    Serial.begin( 115200 );
    while ( !Serial ) delay( 50 );              // primarily here for ESP32 variants with USB CDC

    theDisplay.begin( DIS_DATA, MATRIX_LEDS );  // install the RMT driver and initialize the display
    disClear( NO_SHOW );                        // make sure the display buffer is zeroed out
    disSetBright( brightMap[ 1 ], NO_SHOW );    // set the initial display brightness
    disDrawPix( PO_PIXEL, PO_COLOR, SHOW );     // turn on the power LED
    delay( GUI_PAUSE_TIME );

    dButt.begin();                              // initialize the Btn class
    do {                                        /* initialize & debounce the display button */
        dButt.read();
    } while ( !dButt.isStable() );

    orientation_t stacOrientation = getOrientation();   // check the orientation of the STAC
    rotateGlyphs( stacOrientation );                    // create the display glyphMap[] in memory
    wifiStat.stacID = genSTACid();                      // generate the STAC id
    sendHeader( wifiStat );                             // send the serial port info dump header
    if ( STACpmCheck() ) {                              // check if the PM jumper is in place and...
        STACPeripheral();                               //  run in PM if so (we never return from this function)
    }
    pinMode( TS_0, OUTPUT );                            // set the GROVE GPIO
    pinMode( TS_1, OUTPUT );                            //  pins as outputs
    GROVE_UNKNOWN;                                      // set the initial tally state of the GROVE pins

    STACProvision( stcPrefs, stacOps, tallyStat, wifiStat );    // do all the checks for provisioning & initialization for Normal Operating Mode

    disDrawPix( PO_PIXEL, RGB_COLOR_GREEN, SHOW );      // turn on the gtg LED
    delay( GUI_PAUSE_TIME );                            // all the background setup is done. Whew. Almost there... Pause for the "GUI"

    if ( stacOps.tChannel < 9 ) {                       /* display the activce tally channel */
        drawGlyph( glyphMap[ stacOps.tChannel ], tHDMIvalueColor, 1 );
    }
    else {
        drawGlyph( glyphMap[ stacOps.tChannel - 8 ], tSDIvalueColor, 1 );
    }
    while ( dButt.read() );                 // wait for button to be released

    bool asBypass = false;                  // get set up for the auto start detect and control stuff
    bool nextonstate = true;
    if ( stacOps.autoStart ) {              // if we're in auto start mode...
        asBypass = true;
        pulsePix( nextonstate, RGB_AS_PULSE_COLOR );            // ...turn on the four corner LEDs
        unsigned long asTimeOut = millis() + AS_TIMEOUT;
        unsigned long nextFlash = millis() + AS_PULSE_TIME;
        while ( asTimeOut >= millis() && dButt.read() == 0 ) {  /* pause until we time out (continue with auto start) */
            if ( nextFlash <= millis() ) {                      /* have some fun & flash the four corner LED's while waiting :) */
                nextonstate = !nextonstate;
                nextFlash = millis() + AS_PULSE_TIME;
                pulsePix( nextonstate, RGB_AS_PULSE_COLOR );
            }
        }
        if ( dButt.read() ) {                                   /* unless the button is pressed, in which case... */
            asBypass = false;                                   /* cancel auto start */
        }
    }

    /* Setting up to display & change the current (active) tally channel
     * If the button is clicked, drop out
     * If the button is long pressed, call changeTallyChannel()
    */
    if ( !asBypass ) {              /* skip everything here if autostart kicked in */
        bool escapeFlag = false;
        do  {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor( SELECT_TIME ) ) {
                changeTallyChannel( stcPrefs, "STCPrefs", "talChan", stacOps );
                if ( stacOps.tChannel < 9 ) {
                    drawGlyph( glyphMap[ stacOps.tChannel ], tHDMIvalueColor, SHOW );
                }
                else {
                    drawGlyph( glyphMap[ stacOps.tChannel - 8 ], tSDIvalueColor, SHOW );
                }
            }
        } while ( !escapeFlag );
        dButt.read();
    }

    /* Setting up to display & change the current tally mode
     * If the button is clicked, drop out.
     * If the button is long pressed, call changeTallyMode()
    */
    if ( !asBypass ) {                  /* skip everything here if autostart kicked in */
        drawGlyph( ( stacOps.ctMode ? GLF_C : GLF_T ), purplecolor, SHOW ); // display the current tally mode...
        while ( dButt.read() );             // wait for button to be released
        bool escapeFlag = false;
        do {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;          // if button was clicked, exit
            }
            if ( dButt.pressedFor( SELECT_TIME ) ) {
                changeTallyMode( stcPrefs, "STCPrefs", "ctMde", stacOps );
                drawGlyph( ( stacOps.ctMode ? GLF_C : GLF_T ), purplecolor, SHOW );
            }
        } while ( !escapeFlag );
        dButt.read();
    }

    /* Setting up to display & change the startup mode
     * If the button is clicked, drop out.
     * If the button is long pressed, call changeStartupMode()
    */
    if ( !asBypass ) {                  /* skip everything here if autostart kicked in */
        drawGlyph( ( stacOps.autoStart ? GLF_A : GLF_S ), tealcolor, SHOW );
        while ( dButt.read() );                         // wait for button to be released
        bool escapeFlag = false;
        do  {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor( SELECT_TIME ) ) {
                changeStartupMode( stcPrefs, "STCPrefs", "aStart", stacOps );
                drawGlyph( ( stacOps.autoStart ? GLF_A : GLF_S ), tealcolor, SHOW );
            }
        } while ( !escapeFlag );
        dButt.read();
    }

    /* Setting up to display & change the current display brightness level
     * If the button is clicked, drop out.
     * If the button is long pressed, call updateBrightness()
    */
    if ( !asBypass ) {                                               /* skip everything here if autostart kicked in */
        drawGlyph( GLF_CBD, brightnessset, NO_SHOW );                           // draw the checkerboard test pattern...
        drawOverlay( GLF_EN, RGB_COLOR_BLACK, NO_SHOW );                        // blank out the inside three columns...
        drawOverlay( glyphMap[ stacOps.disLevel ], RGB_COLOR_WHITE, SHOW );     // and overlay the brightness setting number
        while ( dButt.read() );
        bool escapeFlag = false;
        do  {
            dButt.read();
            if ( dButt.wasReleased() ) {
                escapeFlag = true;
            }
            if ( dButt.pressedFor( SELECT_TIME ) ) {
                stacOps.disLevel = updateBrightness( stcPrefs, "STCPrefs", "curBright", stacOps.disLevel );
                drawGlyph( GLF_CBD, brightnessset, NO_SHOW );                           // draw the checkerboard test pattern...
                drawOverlay( GLF_EN, RGB_COLOR_BLACK, NO_SHOW );                        // blank out the inside three columns...
                drawOverlay( glyphMap[ stacOps.disLevel ], RGB_COLOR_WHITE, SHOW );     // overlay the (new) brightness level number
           }
        } while ( !escapeFlag );
        dButt.read();
    }

    // initial user run time operational changes done (or autostart kicked in) so let's get this party started...
    disClear( NO_SHOW );
    disDrawPix( PO_PIXEL, RGB_COLOR_GREEN, SHOW );  // turn on the GTG light
    delay ( GUI_PAUSE_TIME );                       // chill for a second for the sake of the "GUI"
    nextPollTime = millis();                        // set the initial value for the switch poll timer to now

}   // end setup()

void loop() {

    dButt.read();                               // put this at the top of loop() so it always gets hit

    // update brightness check
    if ( dButt.pressedFor( SELECT_TIME ) ) {    // user wants to change the display brightness
        GROVE_UNKNOWN;                          // output the tally state to the GROVE pins
        stacOps.disLevel = updateBrightness( stcPrefs, "STCPrefs", "curBright", stacOps.disLevel );
        if ( !stacOps.ctMode ) {                // if in talent mode
            disFillPix( PVW, NO_SHOW );         //   set the display to PVW
        }
        else {
            disClear( NO_SHOW );
        }
        disDrawPix( PO_PIXEL, PO_COLOR, SHOW );

        // reset the tally state varialbles and flags
        tallyStat.lastTallyState = "NO_TALLY";
        tallyStat.tState = "NO_INIT";
        tallyStat.tConnect = false;
        tallyStat.tTimeout = true;
        tallyStat.tNoReply = true;
        tallyStat.tNoReplyCount = 0;
        tallyStat.tJunkCount = 0;
        nextPollTime = millis();                // force a repoll of the tally state
    }
    // end update brightness check

    /* ~~~~~ Connect to WiFi control logic ~~~~~ */
    if ( WiFi.status() != WL_CONNECTED ) {
        log_e( "No WiFi connection." );

        bool leaveDisplayGreen = false;

        if (wifiStat.wfconnect) {                   // if we had a previous good WiFi connection...
            GROVE_UNKNOWN;                          //  output the tally state to the GROVE pins
            stClient.stop();                        //  stop the stClient
            WiFi.disconnect();                      //  kill the WiFi
            log_e( "WiFi connection lost :(" );

            if (!stacOps.ctMode) {                      // if in talent mode...
                disFillPix( PVW, NO_SHOW );             // set the display to PVW
                disDrawPix( PO_PIXEL, PO_COLOR, SHOW );
                leaveDisplayGreen = true;               //  let the downstream code know to not futz with the display
            }
            else {
                drawGlyph( GLF_WIFI, alertcolor, SHOW );            // let the user know we lost WiFi...
                flashDisplay(8, 300, brightMap[ stacOps.disLevel ] );
                delay( GUI_PAUSE_TIME );
            }
            // reset the WiFi control flags
            wifiStat.wfconnect = false;
            wifiStat.timeout = false;
            // reset the tally state varialbles and flags
            tallyStat.lastTallyState = "NO_TALLY";
            tallyStat.tState = "NO_INIT";
            tallyStat.tConnect = false;
            tallyStat.tTimeout = true;
            tallyStat.tNoReply = true;
            tallyStat.tNoReplyCount = 0;
            tallyStat.tJunkCount = 0;
        }

        if ( !leaveDisplayGreen )
            drawGlyph( GLF_WIFI, warningcolor, SHOW );          // draw the "attempting to connect to WiFi" thing on the display
        while ( !wifiStat.wfconnect ) {
            connectToWifi( wifiStat );                          // connect to the WiFi network
            if ( wifiStat.timeout ) {
                log_e( "WiFi connect attempt timed out." );
            }
            if ( wifiStat.timeout && !leaveDisplayGreen ) {
                drawGlyph( GLF_WIFI, alertcolor, SHOW );            // let the user know we timed out trying to connect to WiFi...
                flashDisplay( 8, 300, brightMap[ stacOps.disLevel ] );
                delay( GUI_PAUSE_TIME );
                drawGlyph( GLF_WIFI, warningcolor, SHOW );          // draw the "attempting to connect to WiFi" thing on the display.
            }
        }

        sendWifiGood();                 // add to the serial monitor info dump
        log_e( "WiFi connected." );

        if ( !leaveDisplayGreen ) {
            drawGlyph( GLF_WIFI, gtgcolor, SHOW );  // draw the "connected to WiFi" thing on the display.
            delay( GUI_PAUSE_TIME );
            disClear( NO_SHOW );
            disDrawPix( PO_PIXEL, PO_COLOR, SHOW );
        }
        nextPollTime = millis();                    // force a repoll of the switch

    }   /* ~~~~~ end Connect to WiFi control logic ~~~~~ */

    /* ~~~~~ Update Tally Display control logic ~~~~~ */
    if ( millis() >= nextPollTime ) {
        nextPollTime = millis() + stacOps.stsPollInt;
        if ( stacOps.tModel == "V-60HD" ) {
            getTallyState( tallyStat, wifiStat );
        }
        else {
            getTallyState2( stacOps, tallyStat, wifiStat );         // tallyStatus.tModel is "V-160HD"
        }

        // handle the normal operating conditions first...
        if ( tallyStat.tConnect && !tallyStat.tNoReply ) {
            // we have a reply from the switch, lets go figure out what to do with it
            nextPollTime = millis() + stacOps.stsPollInt;
            tallyStat.junkReply = false;

            if ( tallyStat.tState != tallyStat.lastTallyState ) {
                if ( tallyStat.tState == "onair" ) {
                    GROVE_PGM;                                          // output the tally state to the GROVE pins
                    disFillPix( PGM, NO_SHOW );                         // change the display to the PGM colour;
                }
                else if ( tallyStat.tState == "selected" ) {
                    GROVE_PVW;                                          // output the tally state to the GROVE pins
                    disFillPix( PVW, NO_SHOW );                         // change the display to the PVW colour
                }
                else if ( tallyStat.tState == "unselected" ) {
                    GROVE_NO_SEL;                                       // output the tally state to the GROVE pins
                    if ( stacOps.ctMode ) {                             /* if in camera operator mode */
                        drawGlyph( GLF_DF, unselectedcolor, NO_SHOW );  //  change the display to the unselected glyph and colour
                    }
                    else {
                        disFillPix( PVW, NO_SHOW );                     //  else change the display to the PVW colour
                    }
                }
                else {
                    // catchall code block only executed if we get a junk reply from the switch
                    nextPollTime = millis() + ERROR_REPOLL_TIME;        // set the "re-poll on error" time
                    tallyStat.junkReply = true;                         // we got a reply from the ST server, but it's garbage
                    tallyStat.tJunkCount++;                             // increment the error accumulator
                    tallyStat.lastTallyState = "JUNK";
                    tallyStat.tState = "NO_TALLY";

                    if ( tallyStat.tJunkCount >= MAX_POLL_ERRORS ) {    /* we've hit the error threshold */
                        GROVE_UNKNOWN;                                  // output the tally state to the GROVE pins
                        if ( stacOps.ctMode ) {                         // if in camera operator mode
                            drawGlyph( GLF_QM, purplecolor, SHOW );     //  display unknown response glyph
                        }
                        else {
                            disFillPix( PVW, NO_SHOW  );                //  else change the display to the PVW colour
                            disDrawPix( PO_PIXEL, PO_COLOR, SHOW );
                        }
                        tallyStat.tJunkCount = 0;                       // clear the error accumulator
                    }
                }   // closing brace for catchall code block

                if ( !tallyStat.junkReply ) {                           /* valid status state returned */
                    disDrawPix( PO_PIXEL, PO_COLOR, SHOW );
                    tallyStat.tNoReplyCount = 0;                        // clear the error accumulators
                    tallyStat.tJunkCount = 0;
                    tallyStat.lastTallyState = tallyStat.tState;        // save the current tally state
                }
            }   // end "if ( tallyStat.tState != tallyStat.lastTallyState )"
        }   // end "if ( tallyStat.tConnect && !tallyStat.tNoReply )"
        // the block below handles all the error conditions (except a junk STS reply) reported by the last tally check
        else {
            // error occurred when trying get a tally status update
            tallyStat.tState = "NO_INIT";
            tallyStat.lastTallyState = "NO_TALLY";
            tallyStat.tJunkCount = 0;                                   // clear the error accumulator

            if ( !tallyStat.tConnect && tallyStat.tTimeout ) {
                // could not connect to the STS & timed out trying
                nextPollTime = millis() + ERROR_REPOLL_TIME;            // set the "re-poll on error" time
                tallyStat.tNoReplyCount = 0;                            //  clear the error accumulator
                GROVE_UNKNOWN;                                          //  output the tally state to the GROVE pins
                log_e( "\"OrangeX\" error." );
                if ( stacOps.ctMode ) {
                    drawGlyph( GLF_BX, warningcolor, SHOW );            //  throw a warning X to the display...
                }
                else {
                    disFillPix( PVW, NO_SHOW );                         // else change the display to the PVW colour...
                    disDrawPix( PO_PIXEL, PO_COLOR, SHOW );
                }
            }
            else if ( tallyStat.tConnect && ( tallyStat.tTimeout || tallyStat.tNoReply ) ) {
                // we connected to the STS & sent a tally status request but either (the response timed out) or (an empty reply was received)
                nextPollTime = millis() + ERROR_REPOLL_TIME;            // set the "re-poll on error" time
                tallyStat.tNoReplyCount++;                              // increment the error accumulator
                if ( tallyStat.tNoReplyCount >= MAX_POLL_ERRORS ) {
                    tallyStat.tNoReplyCount = 0;                        // clear the error accumulator
                    GROVE_UNKNOWN;                                      // output the tally state to the GROVE pins
                    log_e( "\"Big Purple X\" error." );
                    if ( stacOps.ctMode ) {
                        drawGlyph( GLF_BX, purplecolor, SHOW );         // throw up a BPX
                    }
                    else {
                        disFillPix( PVW, NO_SHOW );                     // else change the display to the PVW colour...
                        disDrawPix( PO_PIXEL, PO_COLOR, SHOW );
                    }
                }
            }
            else {
                // some other error condition has occurred
                nextPollTime = millis() + ERROR_REPOLL_TIME;    // set the "re-poll on error" time
                tallyStat.tNoReplyCount = 0;                    // clear the error accumulator
                GROVE_UNKNOWN;                                  // output the tally state to the GROVE pins
                log_e( "\"Big Red X\" error." );
                if (stacOps.ctMode) {
                    drawGlyph( GLF_BX, alertcolor, SHOW );      // throw up a big red X
                }
                else {
                    disFillPix( PVW, NO_SHOW );                 // else change the display to the PVW colour...
                    disDrawPix( PO_PIXEL, PO_COLOR, SHOW );
                }
            }
        }   // end "else error block"
    }   // end if ( millis() >= nextPollTime )

}   // end loop()


// --- EOF ---
