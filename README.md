# STAC  
## (Smart Tally Atom Client)  
**A Roland Smart Tally Client**  

An Arduino sketch designed to run on an [M5Stack ATOM Matrix](https://docs.m5stack.com/#/en/core/atom_matrix) board.

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

Configuration of the STAC for the WiFi credentials and IP address, port number  
and number of tally channels of the Roland switch is done using a web browser.

Lots of good information in the *STAC User Guide*, located in the `manual` folder.

There are other good bits you'll want to read in the `STAC` folder.

## To use this sketch:

* download the zip file from the **Code** tab
* unzip the `STAC-main.zip` file
* rename the unzipped folder from  `STAC-main` to just `STAC`
* move the `STAC` folder to your Arduino sketch folder.

|Use of this sketch requires a modified set of libraries. <br>See the`libraries_README.md` file in the `libraries` folder.|  
:---:
<br><br>
---

### Document Revision History

**2021-04-07:** Delete config info. Minor format changes.  
**2021-04-05:** Revise for user configurable port number.
