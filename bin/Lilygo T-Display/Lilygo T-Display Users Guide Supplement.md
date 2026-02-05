# LilyGo TTGO T-Display Users Guide Supplement

This document outlines the differences in the STAC software for the LilyGo T-Display.

---

## Buttons

The buttons are the mapped as in the diagram below.

<center><img src="./supp-pics/t-displayButtonConfig.png"></center>

The button at bottom left is not used and is ignored by the STAC. However, this is the "Boot" button of the ESP32 which if pressed when power is applied or when the reset button is clicked, will make it appear that the device is unresponsive (which technically it is as it is now  waiting for firmware to be flashed). Should that occur, disconnect the USB cable, wait a tick or two and then reconnect power&mdash;ensuring no buttons are pressed.

---

## Display

The T-Display has an LCD display which brings some changes to the icons used.

### Configuration

The setup or configuration icon appears as a gear.

<center><img src="./supp-pics/lcdConfigIconO.png">    <img src="./supp-pics/lcdConfigIconR.png"></center>

### Factory Reset

The factory reset icon appears as a "go back" arrow arc.
<center><img src="./supp-pics/lcdFactoryreset.png"></center>

### Brightness Setting

When setting the display brightness, the background appears as a red and green checkerboard with an overlay of the selected brightness on a black square.
<center><img src="./supp-pics/lcdBrightnessRtV.png"></center>


### Unselected Tally Status

An unselected tally sate appears as a black and purple checkerboard with an overlay of the orange "power on" indicator.<center><img src="./supp-pics/lcdUnselcTallyRtV.png"></center>


### Orientation

This device does not have an IMU so the display is always oriented with the baseline where the USB port is located.

## Peripheral Mode


## Peripheral Mode
 
### Operating in Peripheral Mode

The T-Display supports Peripheral Mode operation by monitoring the state of GPIO25 and GPIO26 as per the table below.

<table>
	<tbody><center>
		<tr>
			<td rowspan="2"><center><b>GPIO 26<br>(TS_1)</b></center></td>
			<td rowspan="2"><center><b>GPIO 25<br>(TS_0)</b></center></td>
			<td colspan="2"><center><b>Displayed Tally State</center></td>
		</tr>
		<tr>
			<td><center><b>Talent</center></b></td>
			<td><center><b>Camera Operator</center></b></td>
		</tr>
		<tr>
			<td><center>0</center></td>
			<td><center>0</center></td>
			<td><center>PVW</center></td>
			<td><center>X</center></td>
		</tr>
		<tr>
			<td><center>0</center></td>
			<td><center>1</center></td>
			<td><center>PVW</center></td>
			<td><center>UNSELECTED</center></td>
		</tr>
		<tr>
			<td><center>1</center></td>
			<td><center>0</center></td>
			<td><center>PVW</center></td>
			<td><center>PVW</center></td>
		</tr>
		<tr>
			<td><center>1</center></td>
			<td><center>1</center></td>
			<td><center>PGM</center></td>
			<td><center>PGM</center></td>
		</tr>
	</center></tbody>
</table>

**Where:**<br>
&nbsp;&nbsp;&nbsp;**1** = logic high (3.3V)<br>
&nbsp;&nbsp;&nbsp;**0** = logic low (0V)<br>
&nbsp;&nbsp;&nbsp;**PVW** (Preview, Standby, Selected) = GREEN<br>
&nbsp;&nbsp;&nbsp;**PGM** (Program, Live, On Air) = RED<br>
&nbsp;&nbsp;&nbsp;**UNSELECTED** = Purple checkerboard pattern<br>
&nbsp;&nbsp;&nbsp;**X** (Unknown) = Orange X

> **Note:**
> 
> Be aware that the GPIO pins use 3.3V logic levels and are not 5V tolerant. Driving any GPIO pin beyond 3.3V will irreparably damage the device.

### Normal State (Controller) Output

When operating in **normal state** (not Peripheral Mode), the M5Stack StickC Plus outputs its current tally state to the GROVE connector on the bottom of the device on pins G32 and G33 as per the table below.

| GPIO 26<br>(TS_1) | GPIO 25<br>(TS_0) | ROLAND SWITCH<br>TALLY STATE |
|:---:|:---:|:---:|
| 0 | 0 | UNKNOWN |
| 0 | 1 | UNSELECTED |
| 1 | 0 | PVW |
| 1 | 1 | PGM |

**Where:**<br>
&nbsp;&nbsp;&nbsp;**1** = logic high (3.3V output)<br>
&nbsp;&nbsp;&nbsp;**0** = logic low (0V output)

<!-- EOF -->
