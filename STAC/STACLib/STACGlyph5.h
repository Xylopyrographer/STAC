// Display glyph and colour definitions
// For a 5 x 5 RGB LED colour matrix arranged in row by row order
// Requires the FastLED library

#define TOTAL_GLYPHS 32         // number of glyphs in the baseGlyphMap array below

// This is the base set of Glyphs before rotation
const uint8_t baseGlyphMap[TOTAL_GLYPHS][25] = {
    {0,0,1,0,0,0,1,0,1,0,0,1,0,1,0,0,1,0,1,0,0,0,1,0,0},    // 0
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
    {0,1,1,0,0,0,1,0,1,0,0,1,1,0,0,0,1,0,0,0,0,1,0,0,0},    // P
    {1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,1,1,0,0,1,1,1,0},    // firmware update icon
};

// glyphMap is the baseGlyphMap rotated according to the vertical orientation of the STAC
uint8_t glyphMap[ TOTAL_GLYPHS ][ 25 ] = { 0 };

/* ----- mnemonic define table for the glyphMap[] ----- */
/* *  Need to keep the numbers 0 to 9 at the start of the array and in the order below as latter 
 *  functions that index into the glyphMap[] directly depend on this order to retrieve the 
 *  correct bitmap corresponding to that number.
*/
#define GLF_0       glyphMap[ 0 ]       // the number 0
#define GLF_1       glyphMap[ 1 ]       // the number 1
#define GLF_2       glyphMap[ 2 ]       // the number 2
#define GLF_3       glyphMap[ 3 ]       // the number 3
#define GLF_4       glyphMap[ 4 ]       // the number 4
#define GLF_5       glyphMap[ 5 ]       // the number 5
#define GLF_6       glyphMap[ 6 ]       // the number 6
#define GLF_7       glyphMap[ 7 ]       // the number 7
#define GLF_8       glyphMap[ 8 ]       // the number 8
#define GLF_9       glyphMap[ 9 ]       // the number 9
#define GLF_X       glyphMap[ 10 ]      // the letter X
#define GLF_WIFI    glyphMap[ 11 ]      // WiFi icon
#define GLF_ST      glyphMap[ 12 ]      // Smart Tally icon
#define GLF_C       glyphMap[ 13 ]      // the letter C
#define GLF_T       glyphMap[ 14 ]      // the letter T
#define GLF_RA      glyphMap[ 15 ]      // right arrow
#define GLF_LA      glyphMap[ 16 ]      // left arrow
#define GLF_HF      glyphMap[ 17 ]      // smimley face
#define GLF_BX      glyphMap[ 18 ]      // big X
#define GLF_FM      glyphMap[ 19 ]      // frame
#define GLF_DF      glyphMap[ 20 ]      // dotted frame
#define GLF_QM      glyphMap[ 21 ]      // question mark
#define GLF_CBD     glyphMap[ 22 ]      // checkerboard pattern
#define GLF_CK      glyphMap[ 23 ]      // checkmark
#define GLF_EN      glyphMap[ 24 ]      // en space
#define GLF_EM      glyphMap[ 25 ]      // em space
#define GLF_DOT     glyphMap[ 26 ]      // dot
#define GLF_CFG     glyphMap[ 27 ]      // WiFi config icon
#define GLF_A       glyphMap[ 28 ]      // the letter A
#define GLF_S       glyphMap[ 29 ]      // the letter S
#define GLF_P       glyphMap[ 30 ]      // the letter P
#define GLF_UD      glyphMap[ 31 ]      // firmware update icon

/* ~~~~~ Colour value definitions for the Atom display ~~~~~ */
#define GRB_COLOR_WHITE     0xffffff
#define GRB_COLOR_BLACK     0x000000
#define GRB_COLOR_RED       0x00ff00
#define GRB_COLOR_ORANGE    0x65ff00
#define GRB_COLOR_YELLOW    0xffff00
#define GRB_COLOR_GREEN     0xff0000
#define GRB_COLOR_BLUE      0x0000ff
#define GRB_COLOR_DKBLUE    0x00007f
#define GRB_COLOR_PURPLE    0x008080
#define GRB_COLOR_DKPRPLE   0x003838
#define GRB_COLOR_TEAL      0xe60062
#define GRB_COLOR_DKTEAL    0x600026
#define GRB_AS_PULSE_COLOR  0xee0000            // flashing color for the four corner pixels when in auto start mode
#define PVW                 GRB_COLOR_GREEN
#define PGM                 GRB_COLOR_RED
#define PO_COLOR            GRB_COLOR_ORANGE    // color to use for the Power On indicator pixel(s)

// ----- Colour pairs for drawing glyphs with drawGlyph() -----
//          - the first colour in the pair is the background colour; the second, the foreground colour.
const CRGB programcolor[]        = { GRB_COLOR_RED, GRB_COLOR_RED };
const CRGB previewcolor[]        = { GRB_COLOR_GREEN, GRB_COLOR_GREEN };
const CRGB unselectedcolor[]     = { GRB_COLOR_BLACK, GRB_COLOR_PURPLE };
const CRGB gtgcolor[]            = { GRB_COLOR_BLACK, GRB_COLOR_GREEN };
const CRGB warningcolor[]        = { GRB_COLOR_BLACK, GRB_COLOR_ORANGE };
const CRGB alertcolor[]          = { GRB_COLOR_BLACK, GRB_COLOR_RED };
const CRGB bluecolor[]           = { GRB_COLOR_BLACK, GRB_COLOR_BLUE };
const CRGB purplecolor[]         = { GRB_COLOR_BLACK, GRB_COLOR_PURPLE };
const CRGB tealcolor[]           = { GRB_COLOR_BLACK, GRB_COLOR_TEAL };
const CRGB brightnessset[]       = { GRB_COLOR_GREEN, GRB_COLOR_RED };         // colors when displaying the brightness level
const CRGB brightnesschange[]    = { GRB_COLOR_WHITE, GRB_COLOR_WHITE };       // colors when changing the brightness level
const CRGB tallychangecolor[]    = { GRB_COLOR_DKBLUE, GRB_COLOR_ORANGE };     // colors when changing the tally channel
const CRGB tallymodecolor[]      = { GRB_COLOR_DKPRPLE, GRB_COLOR_ORANGE };    // colors when changing the tally mode
const CRGB startchangecolor[]    = { GRB_COLOR_DKTEAL, GRB_COLOR_ORANGE };     // colors when changing the startup mode
const CRGB perifmodecolor[]      = { GRB_COLOR_ORANGE, GRB_COLOR_GREEN };      // colors when starting in Peripheral Mode
