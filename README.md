# STAC V-160HD Development Version
## (Smart Tally Atom Client)  
**A Roland Smart Tally Client**  

An Arduino sketch designed to run on an [M5Stack ATOM Matrix](https://docs.m5stack.com/#/en/core/atom_matrix) board.

This is a development version to test compatibility with the Roland V-160HD switcher.

Thanks to Reddit user @feta00 for the request and doing the initial legwork.

## Changes from STAC 2.1.1

This test version is specific to the Roland V-160HD switcher.

Operation is as per [STAC version 2.1.1](https://github.com/Xylopyrographer/STAC) with the following changes:

- when configuring the STAC, the channel number has a range of 1 to 16, but this should be left at the default of 16 as I haven't decided on how this should be handled on the V-160HD.
- channel numbers are either blue or orange.
- blue channel numbers correspond to HDMI channels 1 to 8. (i.e.: an blue 3 is HDMI channel 3.)
- orange channel numbers correspond to SDI channels 1 to 8. (i.e.: an orange 6 is SDI channel 6.)
- when changing the channel number to be monitored:
    - if the currently selected channel is an HDMI channel (blue number), the channel number will change to orange on a blue background (no change from the way v2.1.1 works).
    -  if the currently selected channel is an SDI channel (orange number), the channel number will change to blue number on an orange background.
    - to advance the channel being monitored, keep clicking the display button as the *Users Guide* currently describes.
    - if you click while SDI channel 8 is selected, the STAC will wrap around to select HDMI channel 1.
- the serial startup data dump shows the currently selected channel as HDMI X through to SDI X where "X" is 1 to 8.
- everything else should be as per the docs.

---
### Revision History

**2024-02-19:** First release.<br>

