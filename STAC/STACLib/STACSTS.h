
TallyState getTallyState(TState tally) {
/*  Establishes the connection to the Smart Tally server and returns the response after the server is sent a GET request.
    The long form of the GET request returns an HTTP header, followed by the payload (status state).
    The short form of the GET request returns only the status state.
    This function only sends and responds to the short form.
     
        stClient.print("GET /tally/" +  TALLY_CHAN + "/status HTTP/1.0\r\n\r\n");   // Uses the long form of the tally status request
        stClient.print("GET /tally/" +  TALLY_CHAN + "/status\r\n\r\n");            // Uses the short form of the tally status request

    This version requires arduino-esp32 core 2.0.3 or greater.
    Breaking changes w.r.t WiFi timeouts were introduced from core 2.0.2 to core 2.0.3
    Timeout values that were in ms are now in seconds, which broke the pre-2.0.3 code completely.
*/

    int maxloops = 0;
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
        // try to connect to the ST server
        maxloops = 0;
        while ( !breakFlag && maxloops < 1 ) {
            if ( stClient.connect(stIP, stPort, (uint32_t)1000 ) ) {   // try to connect to the server, waiting up to 1000 ms
                if ( stClient.available() > 0 ) {
                    stClient.flush();
                }
                tally.tConnect = true;
                breakFlag = true;
            }
            else {
                // ST server connect attempt failed; wait 1 ms before trying again.
                maxloops++;
                delay(1);
            }
        }
        if ( maxloops >= 1 ) {
            tally.tState = "NO_STS";         
            return tally;               // unable to get a connection to the ST server
        }
    }
    tally.tConnect = true;
    stClient.setTimeout(1);         // set the timeout to 1 second for WiFiClient '.read'
    
    // we have a connection to the ST server so send a status request
    stClient.print("GET /tally/");
    stClient.print(tally.tChannel);
    stClient.print("/status\r\n\r\n");

    maxloops = 0;
    while ( stClient.available() <= 0 && maxloops < 80 ) {  // Wait a maximum of *** second for the ST server's reply to become available
        maxloops++;
        delay(1);                                           // wait a tick before checking again if there is a response from the ST Server
    }
    if ( maxloops >= 80 ) {         // response from the ST Server timed out
        tally.tState = "NO_REPLY";
        stClient.stop();
        return tally;
    }
    
    replen = 0;                     // reply length to zero   
    breakFlag = false;

    while ( stClient.available() > 0 && breakFlag == false ) {
        reply += (char)stClient.read();        // we have a response from the server
        replen++;
        if ( replen >= 12 ) {
            reply = "INVALID_REPLY";
            breakFlag = true;
        }
    }   
    reply.trim();                           // knock any junk off the front and back of the reply
    if ( reply.length() == 0 ) {
        reply = "EMPTY_REPLY";
    }
    if ( reply == "None") {
        // when the STS emulator "takes a nap", "None" is returned as a response by WiFiClient.cpp
        //  - should let this go and deal with it in STAC loop(), but it's a one-off weirdo. I think.
        reply = "NONE_REPLY";
    }
    else {
        tally.tNoReply = false;
    }
    tally.tTimeout = false; 
    tally.tState = reply;

    return tally;                           // and head back to the barn

}   // end getTallyState()

// --- EOF ---