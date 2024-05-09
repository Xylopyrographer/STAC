
// STACSTS.h

void getTallyState( tState_t &tally, wifi_info_t &wifist ) {
/*  Establishes the connection to the Smart Tally server and returns the response after the server is sent a GET request.
 *  The long form of the GET request returns an HTTP header, followed by the payload (status state).
 *  The short form of the GET request returns only the status state.
 *  This function only sends and responds to the short form.
 *
 *      stClient.print("GET /tally/" +  TALLY_CHAN + "/status HTTP/1.0\r\n\r\n");   // Uses the long form of the tally status request
 *      stClient.print("GET /tally/" +  TALLY_CHAN + "/status\r\n\r\n");            // Uses the short form of the tally status request
 *
 *  This version requires arduino-esp32 core 2.0.3 or greater.
 *  Breaking changes w.r.t WiFi timeouts were introduced from core 2.0.2 to core 2.0.3
 *  Timeout values that were in ms are now in seconds, which broke the pre-2.0.3 code completely.
 *
 *  in this routine, stClient needs to be a global
*/

    uint8_t maxloops = 0;
    bool breakFlag = false;
    String reply = "";
    uint8_t replen = 0;

    // initialize the tally state, status flags and the reply string
    tally.tState = "NO_INIT";
    tally.tConnect = false;
    tally.tTimeout = true;
    tally.tNoReply = true;
    breakFlag = false;
    reply = "";

    if ( !stClient.connected() ) {
        // try to connect to the switch
        maxloops = 0;
        while ( !breakFlag && maxloops < 1 ) {
            if ( stClient.connect( wifist.stIP, wifist.stPort, (uint32_t)1000 ) ) {     // try to connect to the switch, waiting up to 1000 ms
                if ( stClient.available() > 0 ) {
                    stClient.flush();
                }
                tally.tConnect = true;
                breakFlag = true;
            }
            else {
                // switch connect attempt failed; wait 1 ms before trying again.
                maxloops++;
                delay( 1 );
            }
        }
        if ( maxloops >= 1 ) {
            tally.tState = "NO_STS";
            return;               // unable to get a connection to the switch
        }
    }
    tally.tConnect = true;
    stClient.setTimeout( 1 );       // set the timeout to 1 second for WiFiClient '.read'

    // we have a connection to the switch so send a status request
    stClient.print( "GET /tally/" );
    stClient.print( stacOps.tChannel );
    stClient.print( "/status\r\n\r\n" );

    maxloops = 100;
    while ( stClient.available() <= 0 && maxloops > 0 ) {  /* Wait a maximum of maxloops ms for the switch reply */
        maxloops--;
        delay( 1 );
    }
    if ( maxloops == 0 ) {          // response from the switch timed out
        tally.tState = "NO_REPLY";
        stClient.stop();
        return;
    }

    replen = 0;                     // reply length to zero
    breakFlag = false;

    while ( stClient.available() > 0 && breakFlag == false ) {
        reply += (char)stClient.read();        // we have a response from the switch
        replen++;
        if ( replen >= 12 ) {           /* 12 = len( "unselected" ) + 2 */
            reply = "INVALID_REPLY";
            breakFlag = true;
        }
    }
    reply.trim();                       // knock any junk off the front and back of the reply
    if ( reply.length() == 0 ) {
        reply = "EMPTY_REPLY";
    }
    if ( reply == "None" ) {
        // when the python STS emulator used for debugging "takes a nap", "None" is returned as a response by WiFiClient.cpp
        //  - should let this go and deal with it in STAC loop(), but it's a one-off weirdo. I think,
        //     as the actual video switch does not do this.
        reply = "NONE_REPLY";
    }
    else {
        tally.tNoReply = false;
    }
    tally.tTimeout = false;
    tally.tState = reply;

    return;                         // and head back to the barn

}   // end getTallyState()


void getTallyState2( stac_ops_t &_stacOps, tState_t &tally, wifi_info_t &wifist ) {
/*  For the Roland V-160HD switch
 *  Establishes the connection to the switch and returns the response after a GET request is sent.
 *      For the V-160HD, the form is:
 *          GET /tally/ + bank + BANK_NUM + /status\r\n\r\n
 *      but note Basic Authentication is required.
*/

    HTTPClient _stClient;

    int stReturnCode = 0;
    String stResponse = "NO_TALLY";
    String tallyReq = "";
    uint8_t bankChan = 0;

    if ( _stacOps.tChannel < 9 ) {
        bankChan = _stacOps.tChannel;
    }
    else {
        bankChan = _stacOps.tChannel - 8;
    }

    // initialize the tally state, status flags and the reply string
    tally.tState = "NO_INIT";
    tally.tConnect = false;
    tally.tTimeout = true;
    tally.tNoReply = true;

    tallyReq = "/tally/" + _stacOps.tChanBank + bankChan + "/status";

    _stClient.begin( wifist.stIP.toString(), wifist.stPort, tallyReq );

    _stClient.setReuse( true );                     // keep-alive - use this before .begin
    _stClient.setAuthorization( tally.tLanUID.c_str(), tally.tLanPW.c_str() );  // use this after .begin & before .GET
    _stClient.setUserAgent( wifist.stacID );

    stReturnCode = _stClient.GET();                 // do the tally state request

    if ( stReturnCode > 0 ) {                       /* Check the return code */
        if ( stReturnCode == HTTP_CODE_OK ) {
            stResponse = _stClient.getString();
            tally.tConnect = true;
            tally.tNoReply = false;
            tally.tTimeout = false;
            tally.tState = stResponse;
            _stClient.end();
            return;
        }
        else {
            String stResponse = _stClient.getString();
            tally.tConnect = true;
            tally.tNoReply = true;
            tally.tTimeout = false;
            tally.tState = stResponse;
            _stClient.end();
            return;
        }
        if ( stReturnCode == 401 ) {
            tally.tConnect = true;
            tally.tNoReply = true;
            tally.tTimeout = false;
            tally.tState = stResponse;
            return;
        }
    }
    _stClient.end();
    return;

}   // end getTallyState2()

// --- EOF ---
