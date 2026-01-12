## IMU Calibration

### Current State

We have developed a calibration tool that lets us map physical orientation of the four cardinal positions of the device relative to the display.

To be complete, the tool should also map the acceleration vectors when the device is flat, with the display facing up.

### Further Analysis

This solves one part of the puzzle but to plot the display glyphs properly we have to take into consideration that the base glyph map assumes the upper left corner of the display is at index 0 of the display buffer.

Meaning, if the calibration tool shows that at the home orientation of the device pixel 0 is at the bottom left corner, then after mapping the physical display orientations with the X and Y acceleration vectors by rotating the display we need indeed use an `OFFSET_` to then rotate the display to match "UP" when the device is at its home position.

Example; pixel at index 0 is lit. With the display in its home position, that pixel is at the bottom left of the display. That means (to simplify) we need to use an `OFFSET_180` as the home rotation of the glyph map. Then the rotated glyph map would match the physical "UP" orientation of the device.

Another way of saying it is the `OFFSET_` is the number of clockwise degrees we have to rotate the base glyph map so that it is in alignment with the device when the device is at its vertical home orientation.
