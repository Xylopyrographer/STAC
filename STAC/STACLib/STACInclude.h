
// STACInclude.h
//  - brings in all the STAC routines, defines, constants
//  - like a loaded pizza
//  - the include order is significant
//  - "STACDefine.h" must be first
//  - "STACGlobal.h" must be second
//  - "STACInfoHeader.h" must appear before "STACUtil.h"
//  - "STACOpModes.h" must appear before "STACPeripheral.h"
#include "./STACDefine.h"       // hardware, timing & other #defines - must include this one first
#include "./STACGlobal.h"       // global variables & constants - must include this one immediately after STACDefine.h
#include "./STACDis5.h"         // routines for manipulating & drawing on a 5x5 display - #includes glyphs and colours
#include "./STACInfoHeader.h"   // info dump bits to send to the serial port
#include "./STACUtil.h"         // routines that fire up WiFi AP points for config & updates, a few other utility functions
#include "./STACIMU.h"          // creates the rotated glyphs for a 5x5 display (depends on "STACGlyph5.h")
#include "./STACWiFi.h"         // connect to & manage the WiFi in station mode
#include "./STACOpModes.h"      // routines to change the run-time operating parameters at startup
#include "./STACPeripheral.h"   // Peripheral Mode checks and PM operating code
#include "./STACProvision.h"    // startup and provisioning checks
#include "./STACSTS.h"          // function to retreive status from the Smart Tally Server (STS)


//  --- EOF ---
