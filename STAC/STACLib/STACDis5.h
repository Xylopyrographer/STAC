/*~~~~ Display drawing routines ~~~~*/
// back porting the STAC-LR stuff to ATOM Matrix
// These routines are for an 5x5 matrix arranged in row by row order
// Requires the FastLED library

void disSetBright( uint8_t brightness ) {
    // "brightness" is the absolute value to set he display to, not a brightness level mapped through brightMap[].

    FastLED.setBrightness( brightness );
    FastLED.show();
    return;
}

void disDrawPixXY( uint8_t xpos, uint8_t ypos, CRGB color, bool show = true ) {
    if ( ( xpos >= 5 ) || ( ypos >= 5 ) ) {
        return;
    }
    leds[ xpos + ypos * 5 ] = color;
    if ( show ) {
        FastLED.show();
    }
    return;
}

void disDrawPixXY( int xpos, int ypos, CRGB color, bool show = true ) {
    // overloaded version of disDrawPixXY that casts int arguments.
    // fwiw, sometimes C++ data typing is really annoying.
    disDrawPixXY( (uint8_t)xpos, (uint8_t)ypos, color, show );
    return;
}

void disDrawPix( uint8_t dot, CRGB color, bool show = true ) {
    if ( dot >= MATRIX_LEDS ) {
        return;
    }
    leds[ dot ] = color;
    if ( show ) {
        FastLED.show();
    }
    return;
}

void disDrawPix( int dot, CRGB color, bool show = true ) {
    // overloaded version of disDrawPix that casts int arguments.
    disDrawPix( (uint8_t)dot, color, show );
    return;
}

void disClear( bool show = true ) {

    FastLED.clear();
    if ( show ) {
        FastLED.show();
    }
    return;
}

void disFillPix( CRGB color, bool show = true ) {

    for ( int8_t i = 0; i < MATRIX_LEDS; i++ ) {
        leds[ i ] = color;
    }
    if ( show ) {
        FastLED.show();
    }
    return;
}

void drawGlyph( uint8_t glyph[], const CRGB colors[], bool show = true ) {
/*  Draws a bit mapped image onto the Atom display.
      - glyph[] is the bitmap array of the glyph to draw.
      - in the pair of colors[]; a "0" at glyph[i] will draw color[0]; a "1", color[1]
      - or if you like, "0" and "1" in glyph[i] are the background/foreground color selectors at that pixel location.
*/

    for ( uint8_t i = 0; i < MATRIX_LEDS; i++ ) {
        disDrawPix( i, colors[ glyph[ i ] ] );
    }
    if ( show ) {
        FastLED.show();
    }
    return;
}   // end drawGlyph()

void drawGlyph( const uint8_t glyph[], const CRGB colors[], bool show = true ) {
/*  Version of drawGlyph to draw a glyph directly from the 
*   baseGlyphMap which has the glyphs typed as "const".
*/

    uint8_t currentGlyph[ MATRIX_LEDS ] = { };

    for ( uint8_t g = 0; g < MATRIX_LEDS; g++) {
        currentGlyph[ g ] = glyph[ g ];
    }
    drawGlyph( currentGlyph, colors, show );
    return;

}   // end drawGlyph()

void drawOverlay( uint8_t glyph[], CRGB ovColor, bool show = true ) {
/*  Overlays a bit mapped image onto the Atom display.
      - glyph[] is the bitmap array of the glyph to draw.
      - a "1" at glyph[ i ] will draw ovColor at that pixel location; a "0" will leave that pixel as it is
      - ovColor is a CGRB type
*/
    for (int i = 0; i < MATRIX_LEDS; i++) {
        if (glyph[ i ] == 1) disDrawPix( i, ovColor );
    }
    if ( show ) {
        FastLED.show();
    }
    return;  
}   // end drawOverlay()

void flashDisplay( uint8_t count, uint16_t rate, uint8_t brightness ) {
/*  Flashes the display brightness between 0 and the brightness value passed.
    - "count" is the number of times to flash the display
    - "rate" is the speed at which the display flashes in ms
    - on exit, the display brightness remains at the brightness level passed
    - function is blocking for ( count * rate ) ms
*/
    unsigned long nextChange;
    
    for ( uint8_t i = 0; i < count; i++ ) {
        nextChange = millis() + rate / 2;
        disSetBright( 0 );
        while ( nextChange >= millis() );
        nextChange = millis() + rate / 2;
        disSetBright( brightness );
        while ( nextChange >= millis() );
    }
    return;    
}   // end flashDisplay()

void pulsePix( bool state, CRGB colour ) {
/*  turns the four outermost pixels of the display off or on
 *  depending on the state of "state" using the colour "colour"
 */
    if ( state ) {
        disDrawPix( 0, colour, false );
        disDrawPix( 4, colour, false );
        disDrawPix( 20, colour, false );
        disDrawPix( 24, colour, false );
    }
    else {
        disDrawPix( 0, GRB_COLOR_BLACK, false );
        disDrawPix( 4, GRB_COLOR_BLACK, false );
        disDrawPix( 20, GRB_COLOR_BLACK, false );
        disDrawPix( 24, GRB_COLOR_BLACK, false );
    }
    FastLED.show();
    return;
    
}   // end pulsePix()

/*~~~~ End Display drawing routines ~~~~*/


//  ---- EOF ----
