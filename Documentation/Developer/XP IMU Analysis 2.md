## IMU oriented with +Z-axis pointing away from me.

Examine the X and Y patterns as a couple.

That pattern repeats as:

| Pattern number | X value | Y value |
|----------------|---------|---------|
| 1 | +1 |  0 |
| 2 |  0 | +1 |
| 3 | -1 |  0 |
| 4 |  0 | -1 |

No matter at what pattern number we start, because of the right hand rule, the next pattern is always `(current pattern number + 1)`.

So if we see pattern number 2, we must next see pattern number 3 on the next 90 degree clockwise rotation.

If current pattern is 4, next pattern will be 1 and so on. (We wrap around.)

Thus no matter where we start, we know what the next pattern will be. So if the initial pattern at the home position is 2, that becomes our baseline home pattern that maps to physical `0 degrees`. Thus after doing that, if on startup we see pattern 1, we know the physical rotation is 270 degrees, as that pattern is 3 positions away. `0 + ( 3 * 90 ) = 270`. We count down the table vertically, wrapping at the end until we find a match.

If we count backwards through the table we have the reverse map should we need it.

## IMU oriented with +Z-axis pointing toward me.

The pattern is:

| Pattern number | Y value | X value |
|----------------|---------|---------|
| 1 |  0 | +1 |
| 2 | -1 |  0 |
| 3 |  0 | -1 |
| 4 | +1 |  0 |

The X and Y columns are reversed because of the Z-axis flip.

The same analysis holds true as for the above case. We know the pattern wraps, so once we assign a home pattern number, that becomes physical 0 degrees. We count through the table to find the match to find how far that is away from the home patten and again multiply that out, and we have our physical rotation.

## LUT Analysis

Starting from a home pixel position at the top-left, as we rotate the display at 90 degree increments the LUT we need to use to keep the characters upright is:

| LUT Number | LUT Table |
|------------|-----------|
| 1 | `LUT_ROTATE_0`   |
| 2 | `LUT_ROTATE_270` |
| 3 | `LUT_ROTATE_180` |
| 4 | `LUT_ROTATE_90`  |

Here we see the pattern. No matter where we start on the next 90 degree rotation, the next pattern we need is the next LUT table number.

We also know that the LUT's were developed assuming "up" is with pixel 0 in the top-left corner.

If we put the device in it's home position and ask the user in which corner pixel 0 lights up, we can determine what the baseline LUT is because "top-left" moves to "top-right" then 
"bottom-right" and finally "bottom-left" as the display rotates clockwise in 90 degree increments.

Thus if the pixel lights at the bottom-right, during calibration, we know we need to use `LUT_ROTATE_180` as the baseline LUT (LUT number 3). And for every 90 degree rotation we find what LUT to use by incrementing the LUT table number, wrapping from 4 to 1.

## Bringing it together

From the IMU tables and the baseline or home rotation set during calibration we know the baseline rotation of the device.

From the pixel calibration we know the baseline LUT to use.

So now during calibration, all we need do is generate a table that matches two at the home position determined during calibration and use the IMU pattern found during startup to pick the corresponding LUT to use against that pattern.

A few lookup tables and we're done.

I think.



