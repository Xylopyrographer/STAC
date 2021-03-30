# STAC
## (Smart Tally Atom Client)
**A Roland Smart Tally Client**

This is an Arduino sketch designed to run on an [M5Stack ATOM Matrix](https://docs.m5stack.com/#/en/core/atom_matrix) board.

Its purpose is to monitor the tally status of a single video input channel 
of a Roland video device that implements their proprietary Smart Tally protocol.

The sketch uses WiFi to connect to the same network as that of the Roland device.

For the Roland video input channel being monitored, STAC will set
the colour of the display on the ATOM:

+ when in "Camera Operator" mode, to:  
     - RED if the channel is in PGM (Progam or onair)
     - GREEN if the channel is in PVW (Preview or selected)
     - "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected).  
+ when in "Talent" mode, to:
     - RED if the channel is in PGM (Progam or onair)
     - GREEN otherwise

Configuration of the STAC for the WiFi credentials and number of tally 
channels of the Roland switch is done using a web browser.

When in its Configuration state the:

* STAC WiFi SSID is:  
    - **STAC_XXXXXXXX**  
where the X's are a set of characters unique to every ATOM unit.
* STAC configuration page is at:  
    - **192.168.6.14**

* Password is:  
    - **1234567890**

To use this sketch:
* download the zip file from the **Code** tab
* unzip the STAC-main.zip file
* rename the unzipped folder from  *STAC-main* to just *STAC*
* move the *STAC* folder to your Arduino sketch folder.

Use of this sketch requires a modified set of libraries. See the *library_README.md* file in the *library* folder.

Additional information is in the User Manual, located in the *manual* folder.

---
