
// STACGlyph5.h

// Display glyph and colour definitions
// For a 5 x 5 RGB LED colour matrix arranged in row by row order

#define TOTAL_GLYPHS 32     // number of glyphs in the baseGlyphMap array below
#define MATRIX_LEDS 25      // # of RGB LED's in the display matrix

// This is the base set of Glyphs before rotation
uint8_t baseGlyphMap[ TOTAL_GLYPHS ][ MATRIX_LEDS ] = {
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
    {1,0,0,0,1,0,1,0,1,0,0,0,1,0,0,0,1,1,1,0,0,1,1,1,0}     // firmware update icon
};

// glyphMap is the baseGlyphMap rotated according to the vertical orientation of the STAC
uint8_t glyphMap[ TOTAL_GLYPHS ][ MATRIX_LEDS ] = { 0 };

/* ----- mnemonic define table for the glyphMap[] ----- */
/*  Need to keep the numbers 0 to 9 at the start of the array and in the order below as latter
 *      functions that index into the glyphMap[] directly depend on this order to retrieve the
 *      correct bitmap corresponding to that number.
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
#define GLF_HF      glyphMap[ 17 ]      // happy face
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

/* ~~~~~ Colour value definitions for  ~~~~~ */
/* ~~~~~ the Atom display in RGB order ~~~~~ */
#define RGB_COLOR_BLACK     0x000000
#define RGB_COLOR_BLUE      0x0000ff
#define RGB_COLOR_BLUEDK    0x00007f    /* dark blue */
#define RGB_COLOR_BLUEBR    0x005cff    /* bright blue */
#define RGB_COLOR_GREEN     0x00ff00
#define RGB_COLOR_GREENLT   0x1a800d
#define RGB_COLOR_GREENDK   0x0d4007
#define RGB_COLOR_ORANGE    0xff6500
#define RGB_COLOR_ORNGDK    0x2b2100
#define RGB_COLOR_PRPLEDK   0x380070
#define RGB_COLOR_PURPLE    0x800080
#define RGB_COLOR_RED       0xff0000
#define RGB_COLOR_TEAL      0x00e662
#define RGB_COLOR_TEALDK    0x003a21    /* dark teal (-ish) */
#define RGB_COLOR_WHITE     0xffffff
#define RGB_COLOR_YELLOW    0xffff00
#define RGB_AS_PULSE_COLOR  0x00ee00    /* flashing color for the four corner pixels when in auto start mode */


#define PVW         RGB_COLOR_GREEN
#define PGM         RGB_COLOR_RED
#define PO_COLOR    RGB_COLOR_ORANGE    /* color to use for the Power On indicator pixel(s) */

// ----- Colour pairs for drawing glyphs with drawGlyph() -----
//          - the first colour in the pair is the background colour; the second, the foreground colour.
const crgb_t programcolor[]        = { RGB_COLOR_RED, RGB_COLOR_RED };
const crgb_t previewcolor[]        = { RGB_COLOR_GREEN, RGB_COLOR_GREEN };
const crgb_t unselectedcolor[]     = { RGB_COLOR_BLACK, RGB_COLOR_PURPLE };
const crgb_t gtgcolor[]            = { RGB_COLOR_BLACK, RGB_COLOR_GREEN };
const crgb_t warningcolor[]        = { RGB_COLOR_BLACK, RGB_COLOR_ORANGE };
const crgb_t alertcolor[]          = { RGB_COLOR_BLACK, RGB_COLOR_RED };
const crgb_t bluecolor[]           = { RGB_COLOR_BLACK, RGB_COLOR_BLUE };
const crgb_t bluebrcolor[]         = { RGB_COLOR_BLACK, RGB_COLOR_BLUEBR };
const crgb_t purplecolor[]         = { RGB_COLOR_BLACK, RGB_COLOR_PURPLE };
const crgb_t tealcolor[]           = { RGB_COLOR_BLACK, RGB_COLOR_TEAL };
const crgb_t greenltcolor[]        = { RGB_COLOR_BLACK, RGB_COLOR_GREENLT };
const crgb_t orangecolor[]         = { RGB_COLOR_BLACK, RGB_COLOR_ORANGE };
const crgb_t brightnessset[]       = { RGB_COLOR_GREEN, RGB_COLOR_RED };        // colors when displaying the brightness level
const crgb_t brightnesschange[]    = { RGB_COLOR_WHITE, RGB_COLOR_WHITE };      // colors when changing the brightness level
const crgb_t tHDMIvalueColor[]     = { RGB_COLOR_BLACK, RGB_COLOR_BLUE };       // colors when showing the # of the active (HDMI) tally channel (V-60HD & V-160HD)
const crgb_t tSDIvalueColor[]      = { RGB_COLOR_BLACK, RGB_COLOR_GREENLT };    // colors when showing the # of the active SDI tally channel (V-160HD)
const crgb_t tallychangecolor[]    = { RGB_COLOR_BLUEDK, RGB_COLOR_ORANGE };    // colors when changing the HDMI tally channel
const crgb_t tallychangecolorSDI[] = { RGB_COLOR_GREENDK,  RGB_COLOR_ORANGE };  // colors when changing the SDI tally channel
const crgb_t tallymodecolor[]      = { RGB_COLOR_PRPLEDK, RGB_COLOR_ORANGE };   // colors when changing the tally mode
const crgb_t startchangecolor[]    = { RGB_COLOR_TEALDK, RGB_COLOR_ORANGE };    // colors when changing the startup mode
const crgb_t perifmodecolor[]      = { RGB_COLOR_ORNGDK, RGB_COLOR_GREEN };     // colors when starting in Peripheral Mode

//  end glyph and colour definitions
