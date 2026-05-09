# M5Stack ATOM S3R Users Guide Supplement

This document outlines the differences in the STAC software for the M5Stack ATOM S3R.

---

## Button

<center><img src="./supp-pics/atoms3rButtonConfig.png"></center>

The ATOM S3R display,like the ATOM MATRIX is also the User button. Button interaction follows the same single-button conventions as the M5Stack ATOM Matrix — refer to the *STAC Users Guide* for the full interaction model.

### Reset

The ATOM S3R uses a dedicated hardware reset MCU (PMS150G-U6). A brief press of the reset button (the smaller button on the side) will restart the device normally. Holding the reset button for approximately 2 seconds places the device into firmware download mode — a small status LED by the reset button will illuminate green to confirm this state. That is not usually what we want to do when using the S3R as a STAC. Releasing and then a short click of the reset button will do a normal reset of the S3R.

If the device becomes unresponsive and the reset button fails to recover it, remove power (USB cable), wait a couple of seconds, then reconnect.

---

## Display

The ATOM S3R has a 0.85" IPS LCD display (128×128 px) which brings some changes to the icons used.

### Orientation

The ATOM S3R has an IMU and does respect the physical orientation of the device — the display will rotate to match whichever edge is facing down.

### Configuration

The setup or configuration icon appears as a gear.

<center><img src="./supp-pics/lcdConfigIconO.png">    <img src="./supp-pics/lcdConfigIconR.png"></center>

### Factory Reset

The factory reset icon appears as a gear "go back" arrow arc.
<center><img src="./supp-pics/lcdFactoryreset.png"></center>

### Brightness Setting

When setting the display brightness, the background appears as a red and green checkerboard with an overlay of the selected brightness on a black square.
<center><img src="./supp-pics/lcdBrightnessSq.png"></center>

### Unselected Tally Status

An unselected tally state appears as a black and purple checkerboard with an overlay of the orange "power on" indicator.
<center><img src="./supp-pics/lcdTallySq.png"></center>

---

## Peripheral Mode

### Operating in Peripheral Mode

The ATOM S3R supports Peripheral Mode by monitoring the state of GPIO 1 and GPIO 2 on the GROVE Port A connector as per the table below.

<table>
	<tbody><center>
		<tr>
			<td rowspan="2"><center><b>GPIO 2<br>(TS_1)</b></center></td>
			<td rowspan="2"><center><b>GPIO 1<br>(TS_0)</b></center></td>
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

When operating in **normal state** (not Peripheral Mode), the ATOM S3R outputs its current tally state to the GROVE Port A connector on GPIO 1 and GPIO 2 as per the table below.

| GPIO 2<br>(TS_1) | GPIO 1<br>(TS_0) | ROLAND SWITCH<br>TALLY STATE |
|:---:|:---:|:---:|
| 0 | 0 | UNKNOWN |
| 0 | 1 | UNSELECTED |
| 1 | 0 | PVW |
| 1 | 1 | PGM |

**Where:**<br>
&nbsp;&nbsp;&nbsp;**1** = logic high (3.3V output)<br>
&nbsp;&nbsp;&nbsp;**0** = logic low (0V output)


<!-- EOF -->
