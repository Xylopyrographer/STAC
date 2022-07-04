
TallyState getTallyState(TState tally) {
/*  Establishes the connection to the Smart Tally server and returns the response after the server is sent a GET request.
    The long form of the GET request returns an HTTP header, followed by the payload (status state).
    The short form of the GET request returns only the status state.
    This function only sends and responds to the short form.
     
        stClient.print("GET /tally/" +  TALLY_CHAN + "/status HTTP/1.0\r\n\r\n");   // Uses the long form of the tally status request
        stClient.print("GET /tally/" +  TALLY_CHAN + "/status\r\n\r\n");            // Uses the short form of the tally status request

    This version requires with arduino=esp32 core 2.0.3 or greater
*/

    int maxloops;
    bool breakFlag;
    String reply;
    uint8_t replen;
    
    // initialize the tally state, status flags and the reply string
    tally.tState = "NO_INIT";
    tally.tConnect = false;
    tally.tTimeout = true;
    tally.tNoReply = true;
    breakFlag = false;
    reply = ""; 

    if ( !stClient.connected() ) {
        // try to connect to the ST server
        
        log_e( "No STS connection." );
        
        maxloops = 0;
        while ( !breakFlag && maxloops < 1 ) {
            //if ( stClient.connect(stIP, stPort, 100) ) {   // try to connect to the server, waiting up to 100 ms
            if ( stClient.connect(stIP, stPort, (uint32_t)1000 ) ) {   // try to connect to the server, waiting up to 1000 ms
                if ( stClient.available() > 0 ) {
                    stClient.flush();
                }
                
                log_e( "New STS connection");
                
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
            // we tried to get an ST server connection for (1000) ms & failed
            tally.tState = "NO_STS";         
            return tally;               // unable to get a connection to the ST server
        }
    }
    
    // the log_e() below always prints even if we have to establish a new connection above,
    //   but didn't want to rewrite the code for the sake of a debug statement.
    log_e( "Reusing existing STS connection." );
    
    tally.tConnect = true;
    
    // we have a connection to the ST server so send a status request
    stClient.print("GET /tally/");
    stClient.print(tally.tChannel);
    stClient.print("/status\r\n\r\n");

    maxloops = 0;
    while ( stClient.available() <= 0 && maxloops < 10 ) {    // Wait a maximum of *** second for the ST server's reply to become available
        maxloops++;
        delay(1);                                             // wait a tick before checking again if there is a response from the ST Server.
    }
    if ( maxloops >= 10 ) {         // response from the ST Server timed out
        tally.tState = "NO_REPLY";
        return tally;
    }
    
    replen = 0;                     // reply length to zero   
    stClient.setTimeout(1);         // set the timeout to 1 second for WiFiClient '.read'
    breakFlag = false;

//     stClient.setTimeout(0);                    // need to override the default 1 sec blocking of the stClient.readString() function
    while ( stClient.available() > 0 && breakFlag == false ) {
        reply += (char)stClient.read();        // we have a response from the server
        replen++;
        if ( replen >= 12 ) {
            reply = "INVALID_REPLY";
            breakFlag = true;
        }
    }   
    reply.trim();                             // knock any junk off the front and back of the reply (trim is done in-place)
    if ( reply.length() == 0 ) {
        reply = "EMPTY_REPLY";
    }
    if ( reply == "None") {
        // when the STS emulator "takes a nap", "None" is returned as a response by WiFiClient.cpp
        //  - should let this go and deal with it in STAC loop(), but it's a one-off weirdo. I think.
        log_e( "We got a reply = \"None\"!!" );
        reply = "NONE_REPLY";
    }
    else {
        tally.tNoReply = false;
    }
    tally.tTimeout = false; 
    tally.tState = reply;

    return tally;                              // and head back to the barn

}   // closing brace for getTallyState()

