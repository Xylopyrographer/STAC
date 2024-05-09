
// STACDis5.h

#include "./STACGlyph5.h"

//  Display brightness mapping table
//  - maps the display brightness level to the brightness
//      value set when calling the "disSetBright" function
//  - first entry must be 0, max size is 10 items, each
//      entry should be greater than the previous
//  - empirically, highest value should be ~60 to prevent heat damage
const uint8_t brightMap[] =  { 0, 10, 20, 30, 40, 50, 60 };
const uint8_t brightLevels = sizeof( brightMap ) - 1;

const unsigned long __flushDelay = 3UL;     // it takes ~ this many ms to flush the data to the ATOM MATRIX display

/*~~~~ Display drawing routines ~~~~*/
// These routines are for an 5x5 matrix arranged in row by row order
// Requires the LiteLED library

void disSetBright( uint8_t brightness, bool show = true ) {
    // "brightness" is the absolute value to set he display to,
    // not a brightness level mapped through brightMap[].
    theDisplay.brightness( brightness, show );
    if ( show )  delay( __flushDelay );
    return;
}

void disDrawPix( uint8_t dot, crgb_t color, bool show = true ) {
    theDisplay.setPixel( (size_t)dot, color, show );
    if ( show ) delay( __flushDelay );
    return;
}

void disDrawPix( int dot, crgb_t color, bool show = true ) {
    // overloaded version of disDrawPix that casts int arguments.
    disDrawPix( (uint8_t)dot, color, show );
    return;
}

void disDrawPixXY( uint8_t xpos, uint8_t ypos, crgb_t color, bool show = true ) {
    theDisplay.setPixel( (size_t)(xpos + ypos * 5), color, show );
    if ( show ) delay( __flushDelay );
    return;
}

void disDrawPixXY( int xpos, int ypos, crgb_t color, bool show = true ) {
    // overloaded version of disDrawPixXY that casts int arguments.
    //  fwiw, sometimes C++ data typing is really annoying.
    disDrawPixXY( (uint8_t)xpos, (uint8_t)ypos, color, show );
    return;
}

void disFillPix( crgb_t color, bool show = true ) {
    theDisplay.fill( rgb_from_code( color ), show );
    if ( show ) delay( __flushDelay );
    return;
}

void disShow() {
    theDisplay.show();
    delay( __flushDelay );
    return;
}

void disClear( bool show = true ) {
    theDisplay.clear( show );
    if ( show ) delay( __flushDelay );
    return;
}

void drawGlyph( uint8_t glyph[], const crgb_t colors[], bool show = true ) {
    /*  Draws a two colour bit mapped image onto the display.
        - glyph[] is the bitmap array of the glyph to draw.
        - in the pair of colors[]; a "0" at glyph[i] will draw color[0]; a "1", color[1]
        - or if you like, "0" and "1" in glyph[i] are the background/foreground
            color selectors at that pixel location.
    */

    for ( uint8_t i = 0; i < MATRIX_LEDS; i++ ) {
        theDisplay.setPixel( (size_t)i, rgb_from_code( colors[ glyph[ i ] ] ), 0 );
    }
    if ( show )
        theDisplay.show();
        delay( __flushDelay );
    return;
}   // end drawGlyph()

void drawGlyph( const uint8_t glyph[], const crgb_t colors[], bool show = true ) {
    /*  Version of drawGlyph to draw a glyph directly from the
    *   baseGlyphMap which has the glyphs typed as "const".
    */
    uint8_t currentGlyph[ MATRIX_LEDS ] = { };

    for ( uint8_t g = 0; g < MATRIX_LEDS; g++) {
        currentGlyph[ g ] = glyph[ g ];
    }
    drawGlyph( currentGlyph, colors, show );
    if ( show ) delay( __flushDelay );
    return;

}   // end drawGlyph()

void drawOverlay( uint8_t glyph[], crgb_t ovColor, bool show = true ) {
    /*  Overlays a bit mapped image onto the Atom display.
          - glyph[] is the bitmap array of the glyph to draw.
          - a "1" at glyph[ i ] will draw ovColor at that pixel location; a "0" will leave that pixel as it is
    */
    for ( uint8_t i = 0; i < MATRIX_LEDS; i++ ) {
        if ( glyph[ i ] == 1 )
            disDrawPix( i, ovColor, 0 );
    }
    if ( show )
        theDisplay.show();
        delay( __flushDelay );
    return;
}   // end drawOverlay()

void flashDisplay( uint8_t count, unsigned long rate, uint8_t brightness ) {
    /*  Flashes the display brightness between 0 and the brightness value passed.
        - "count" is the number of times to flash the display
        - "rate" is the speed at which the display flashes in ms
        - "brightness" is the absolute value to set he display to, not a brightness level mapped through brightMap[].
        - on exit, the display brightness remains at the brightness level passed
        - function is blocking for ( count * rate ) ms
    */
    unsigned long nextChange;

    // while ( led_strip_busy( &theDisplay) );

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

void pulsePix( bool state, crgb_t colour ) {
    /*  turns the four outermost pixels of the display off or on
     *  depending on the state of "state" using the colour "colour"
     */
    disDrawPix( 0, ( state ? colour : RGB_COLOR_BLACK ), NO_SHOW );
    disDrawPix( 4, ( state ? colour : RGB_COLOR_BLACK ), NO_SHOW );
    disDrawPix( 20, ( state ? colour : RGB_COLOR_BLACK ), NO_SHOW );
    disDrawPix( 24, ( state ? colour : RGB_COLOR_BLACK ), NO_SHOW );
    disShow();
    return;
}   // end pulsePix()

void rotateGlyphs( orientation_t stacOrientation ) {
    /*  copies the entire glyph matrix into memory, rotated
         according to the physical orientation of the STAC
    */

    // Initalize the rotation vectors
    uint8_t rotateUp[ MATRIX_LEDS ] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24 };
    uint8_t rotateLeft[ MATRIX_LEDS ] = { 20,15,10,5,0,21,16,11,6,1,22,17,12,7,2,23,18,13,8,3,24,19,14,9,4 };
    uint8_t rotateDown[ MATRIX_LEDS ] = { 24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0 };
    uint8_t rotateRight[ MATRIX_LEDS ] = { 4,9,14,19,24,3,8,13,18,23,2,7,12,17,22,1,6,11,16,21,0,5,10,15,20 };

    // Initalize the rotation LUT
    uint8_t *rotation_LUT = NULL;

    // Determine which rotation to use
    if ( stacOrientation == DOWN )
        rotation_LUT = rotateDown;
    else if ( stacOrientation == LEFT )
        rotation_LUT = rotateLeft;
    else if ( stacOrientation == RIGHT )
        rotation_LUT = rotateRight;
    else
        rotation_LUT = rotateUp;

    // Perform the rotation
    for ( uint8_t glyphID = 0 ; glyphID < TOTAL_GLYPHS; glyphID++ ) {
        for ( uint8_t loop_pix = 0; loop_pix < MATRIX_LEDS; loop_pix++ ) {
            glyphMap[ glyphID ][ loop_pix ] = baseGlyphMap[ glyphID ][ rotation_LUT[ loop_pix ] ];
        }
    }

    return;

}   // end rotateGlyphs()


/*~~~~ End Display drawing routines ~~~~*/

