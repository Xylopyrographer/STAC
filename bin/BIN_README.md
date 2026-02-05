# Contents

This `bin` folder contains a number of folders holding:

1. for each device, a *Users Guide Supplement*.
1. binary files for flashing to that device.

## Users Guide Supplement

Jump into the folder named for the device you're looking to use as a STAC. Open the *Users Guide Supplement*. It describes differences in functionality or operation from the *STAC Users Guide*, which is the main file on using the STAC (you have read that first, right?).

## `.bin` Files

The `.bin` files are named something like:

**`STAC_v3.0.0_ATOM_R.bin`** and<br>
**`STAC_v3.0.0_ATOM_R_FULL.bin`**

**`STAC`** identifies it as a STAC binary file.<br>
**`v3.0.0`** is the version number of the software.<br>
**`ATOM`** is the physical device or target for this software file.<br>
**`R`** is a nerdy part meaning this is a release version.

One file will also end with **`FULL`**. It is described next.

> **Note**
> 
> It is *very* important to pick the right `.bin` file for the device being programmed. While you can always program a device with the correct file if an error is made, best to check this a couple of times beforehand.


## The `...FULL` File

There are two `.bin` files. The file that ends with `...FULL` is the file you need to flash, download, or program into a device to turn a "never-used-as-a-STAC-before" device into a STAC.

In other words, it's the file you need for first time use of a device as a STAC.

### First Time Flashing

Best way to flash a `...FULL` file into a device is to use a web-based flashing tool. There are a number out there. Some recommendations (in no particular order):

- ESP Connect: https://thelastoutpostworkshop.github.io/ESPConnect/
- Adafruit WebSerial ESPTool: https://adafruit.github.io/Adafruit_WebSerial_ESPTool/
- ESPTool: https://esptool.spacehuhn.com/
- Espressif ESP Tool: https://espressif.github.io/esptool-js/

Connect your device to the computer and follow the instructions to connect and flash the software.

If prompted for a "Starting address" or "Flash address", set that to `0x0000`. That is ***very*** important!

If prompted to add another file, there is no need. Everything is in the STAC `...FULL` file.

## OTA File

The other `.bin.` file is the one used when updating the software in a device already running as a STAC.

The method to do this is described in the *STAC Users Guide*.

Do not use a web based flasher to do this.

<!-- EOF -->
