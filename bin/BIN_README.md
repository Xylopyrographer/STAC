# Contents

This `bin` folder contains device-specific folders, each holding:

1. A *Users Guide Supplement* for that device
2. Supporting images and documentation

**Note:** The actual binary firmware files (`.bin` files) are distributed via [GitHub Releases](https://github.com/Xylopyrographer/STAC/releases) and are not stored in this repository.

## Downloading Firmware

To download STAC firmware for your device:

1. Go to the [STAC Releases page](https://github.com/Xylopyrographer/STAC/releases)
2. Select the latest release (e.g., `v3.0.0`)
3. Scroll to the **Assets** section at the bottom
4. Download the appropriate `.bin` files for your device:
   - `STAC_vX.X.X_<DEVICE>_FULL.bin` - For first-time installation
   - `STAC_vX.X.X_<DEVICE>_OTA.bin` - For over-the-air updates

## Users Guide Supplement

Jump into the folder named for the device you're looking to use as a STAC. Open the *Users Guide Supplement*. It describes differences in functionality or operation from the *STAC Users Guide*, which is the main file on using the STAC (you have read that first, right?).

## Understanding `.bin` File Names

When you download firmware from the Releases page, the files are named like:

**`STAC_v3.0.0_ATOM_Matrix_FULL.bin`** and<br>
**`STAC_v3.0.0_ATOM_Matrix_OTA.bin`**

**`STAC`** identifies it as a STAC binary file.<br>
**`v3.0.0`** is the version number of the software.<br>
**`ATOM_Matrix`** is the physical device or target for this firmware.<br>
**`FULL`** or **`OTA`** indicates the file type (explained below).

> **Important**
> 
> It is *very* important to pick the right `.bin` file for the device being programmed. While you can always re-flash a device with the correct file if an error is made, best to double-check before flashing!


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
