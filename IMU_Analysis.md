## IMU Error Analysis

### Assumptions

1. We are discussing the acceleration vectors only of the IMU.
1. The IMU chip is has its Z-axis perpendicular to the face of the IMU package.
1. The display is perpendicular to the Z-axis of the IMU.
1. The IMU is flat if the magnitude of the Z-axis vector is dominant.


### The Issue

When manufactured, the IMU could be affixed to the board:

- with the positive Z-axis facing toward the display - call this forward;
- with the positive Z-axis facing away from the display - call this aft;
- where the display is vertical and facing the user and relative to a user defined "Home" or "0" orientation; rotated at either 0, 90, 180 or 270 degrees clockwise.

Thus there are eight possible orientations to consider. Forward at one of 0, 90, 180 or 270 and aft at one of 0, 90, 180 or 270.

The current implementation only considers four of these eight possibilities.

We have, with the IMU’s in the ATOM and the Waveshare come against the problem where one IMU is forward and one is aft and so we cannot resolve both concurrently.

### Proposed Solution

We need to define forward and aft as well as orientation in the IMU section board configuration file and expand the implementation to include all eight cases.

In essence, decouple board orientation, display orientation and forward and aft.

If the users fines display rotation relative to their arbitrary choice of a Home position then all they need specify is display rotation and either forward or aft.

The issue is also obfuscated as I choose an ambiguous nomenclature for positioning.

Recommend we define display rotation as 0, 90, 280 or 270 rather than UP, DOWN, RIGHT or LEFT. In that way the user can decide which way is "up" by specifying in the board config file, one display rotation and one of either forward or aft. Let’s say IMU face can be either FORWARD or AFT.

The debug display would then report IMU face: and Device rotation: where the device rotation is the offset relative to the display rotation picked in the config file.

For example, device Home could be with the USB port facing down, but the device could be manufactured such that the IMU if facing AFT and the display "up" is rotated 90 degrees clockwise from Home. So the user would specify IMU AFT and Display rotation 270. The debug log would report IMU AFT, Device rotation 0. The STAC IMU code would create the rotated glyph maps accordingly.

