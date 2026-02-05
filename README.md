# STAC  
**Smart Tally Atom Client**

A wireless tally light system for Roland video switches.

Its purpose is to monitor via WiFi the tally status of a single video input channel of a Roland video device that implements their Smart Tally protocol.

It supports the following Roland video switches:

* V-60HD
* V-160HD

For the Roland video input channel being monitored, STAC will set the colour of the display:

+ when in "Camera Operator" mode, to:  
     - RED if the channel is in PGM (Program or onair)
     - GREEN if the channel is in PVW (Preview or selected)
     - "PURPLE DOTTED" if the channel is not in either PGM or PVW (unselected).  
+ when in "Talent" mode, to:
     - RED if the channel is in PGM (Program or onair)
     - GREEN otherwise

Configuration of the STAC for the WiFi credentials and IP address, port number and number of tally channels of the Roland switch is done using a web browser.

There is also a Peripheral Mode where one STAC can be connected to another via a cable and operate without the need for set up or to establish a WiFi connection.

Lots of good information in the *STAC User Guide*, located in the `Documentation` folder. Other interesting bits are there as well.

## Supported Devices

This release will run on the following devices:

- LilyGo T-Display
- LilyGo T-QT
- M5Stack ATOM Matrix
- M5Stack StickC Plus
- Waveshare ESP32-S3 Matrix
- Xorigin AI PI-Lite


## Installation

### Using a Pre-Built Version

Pre-built binaries are provided for the devices listed above. For information on installing, jump over to the `bin` folder and run through `BIN_README.md`.

Once you've flashed your device it is now a STAC 👍. Next steps are in the *STAC User Guide* in the `Documentation` folder of the Release archive.

### Using an IDE

This release is a complete rewrite of the codebase. 

The `STAC` folder contains all the source files. In there as well is a `doc` folder that holds a good measure of information on how to modify and build the software. There are also some very nifty things in the `utility` folder.

Developing requires VS Code with the pioarduino extension installed. With this release, STAC has outgrown the Arduino IDE.

The Arduino framework is used, and this release is was done using ESP-arduino core version 3.3.2. It should be compatible with newer versions but would require testing.

One (of many) new architectural features is the ability to port the software to new hardware via a configuration file. Good chance that's all you'll need to do to add a device. Details in the `STAC/doc` folder.

To download the files:

1. Here in GitHub, on the right side of the window, click the green "Latest" button under "Releases".
1. Click "Source code" choosing either `zip` or `tar.gz` as you prefer.
1. Unpack the downloaded `STAC-X.Y.Z` file and have at 'er.


## Compatibility

This release is a complete rewrite of the STAC software. If you are new to STAC, follow the installation instructions above to get up and running and you can skip the rest of this section.

If you already have ATOM Matrix STAC devices, the fundamental purpose of the STAC&mdash;to relay tally information&mdash;has not changed. At the end of the day, that is what this release still does very well.

From a users point of view, this release brings:
- significant improvements in configuring and managing multiple devices with the ability to create, save and load from a configuration file and use copy and paste functions as well;
- ability to perform maintenance tasks via the web interface;
- a single web access point for all functions setup and maintenance.

Depending on your environment, these may be useful. Check out the *STAC User Guide*.

**Do note that upgrading to this version will require that the:**<br>
- **firmware is flashed as per the installation instructions as if this was the very first time using the ATOM as a STAC**<br>
- **setup be redone;** however, the new setup features makes this much much easier.

**You cannot do an OTA update to install this release.** Too much has changed under the hood to have made that possible.

All that said, if you're happy as a clam with what you've got that's great. STAC's running this or any other release are fully compatible with their main purpose&mdash;keeping everyone in the know  as to the tally status of the camera you're running!

<br><br>

---
### Revision History

**2026-02-05:** Revise for STAC v3.0.0.<br>
**2024-05-09:** Revise for STAC v2.2.0, adding V-160HD support.<br>
**2023-09-09:** Revise required libraries, Build System Compatibility.<br>
**2023-02-21:** Include info on using an ESP flasher.<br>
**2023-02-04:** Revised for software v2.1.<br>
**2022-07-09:** Revised arduino-esp32 core compatibility information. Added required libraries.<br>
**2022-01-04:** Added information on Peripheral Mode. Minor format changes.<br>
**2021-05-20:** Direction to use the `Release` version in place of the `main` branch.<br>
**2021-04-07:** Delete config info. Minor format changes.<br>

---
---
---

# STAC - Smart Tally Atom Client

A professional tally light system for Roland video switchers, built for ESP32 microcontrollers with LED matrix displays.

## Features

✨ **Multi-Hardware Support**

- M5Stack ATOM Matrix (5×5 RGB LED)
- Waveshare ESP32-S3-Matrix (8×8 RGB LED)
- Easy to add custom hardware configurations

🎯 **Operating Modes**

- **Normal Mode**: WiFi connection to Roland switcher
- **Peripheral Mode**: Wired tally reception via GROVE port
- **Provisioning Mode**: Web-based configuration interface

🎨 **Visual Feedback**

- Color-coded tally states (Red=Program, Green=Preview, Blue=Unselected)
- IMU-based automatic display rotation
- Configurable brightness levels

🔧 **Professional Architecture**

- Hardware abstraction layer (HAL)
- Compile-time board selection (zero runtime overhead)
- Modular, testable design
- Clean separation of concerns


## Supported Hardware

### M5Stack ATOM Matrix

- 5×5 RGB LED matrix
- MPU6886 IMU (6-axis)
- Built-in button
- USB-C programming/power


### Waveshare ESP32-S3-Matrix

- 8×8 RGB LED matrix
- QMI8658 IMU (6-axis)
- Button on GPIO 7
- USB-C programming/power

### Custom Hardware

Template configurations provided for custom builds with:

- 5×5 or 8×8 LED matrices
- Various IMU options (MPU6886, QMI8658, or none)
- Flexible GPIO assignments

## Quick Start

### Prerequisites

**Software:**

- [PlatformIO](https://platformio.org/) (VS Code extension recommended)
- OR [Arduino IDE](https://www.arduino.cc/en/software) 2.0+

**Hardware:**

- Supported ESP32 board (see above)
- USB-C cable
- Roland V-60HD or V-160HD video switcher (for normal mode)

### Installation

1. **Clone the repository:**

```bash
   git clone https://github.com/Xylopyrographer/STAC.git
   cd STAC
```

2. **Configure for your hardware:**
 
   - Edit `STAC/include/Device_Config.h`
   - Uncomment the line for your board:
   - 
```cpp
     #define BOARD_M5STACK_ATOM_MATRIX
     // or
     // #define BOARD_WAVESHARE_ESP32_S3_MATRIX
```

3. **Build and upload:**
   
   **PlatformIO:**
   
```zsh
   cd STAC
   pio run -e atom-matrix -t upload
   # or for Waveshare:
   # pio run -e waveshare-s3 -t upload
```

### Merged Binaries + Web Serial Flashing

- After a build, single-file merged images are created automatically:
   - `.pio/build/atom-matrix/merged-atom-matrix.bin`
   - `.pio/build/waveshare-s3/merged-waveshare-s3.bin`
- Flash in a Web Serial browser by selecting the merged file and writing at address `0x0`.
- Alternatively, flash the individual files with offsets: `0x1000` bootloader, `0x8000` partitions, `0xE000` boot_app0 (if present), `0x10000` firmware.
- Full instructions: `Documentation/Developer/Web Serial Flashing.md`.

### First Run

1. Power on the device
2. You'll see a RGB color sweep (startup animation)
3. Device enters Normal mode (purple display = NO_TALLY)
4. Press button to cycle tally states:
   - Purple → Green (Preview)
   - Green → Red (Program)
   - Red → Blue (Unselected)
   - Blue → Purple (No Tally)

## Operating Modes

### Normal Mode

Connect to Roland switcher via WiFi and display real-time tally status.

**Setup:**

1. Long-press button to enter provisioning mode
2. Connect to WiFi AP: `STAC_XXXXX`
3. Navigate to `192.168.4.1` in browser
4. Configure WiFi and switcher settings
5. Device will connect and monitor tally

### Peripheral Mode

Receive tally status from another STAC device via wired connection.

**Setup:**
1. Install jumper between GPIO pins (see board config)
   - ATOM: Connect pins 3-4
   - Waveshare: Connect pins 3-4
2. Reset device
3. Connects to "Normal" STAC via GROVE port
4. Display mirrors the tally state

### Provisioning Mode

Web-based configuration interface for initial setup.

**Access:**

- Long-press button (>1.5 seconds)
- Device creates WiFi AP: `STAC_XXXXX`
- Connect and navigate to `192.168.4.1`

## Configuration

### WiFi Settings

- SSID and password for your network
- Stored in non-volatile memory

### Switcher Settings

- Model (V-60HD or V-160HD)
- IP address
- Port (default: 80)
- Channel to monitor

### Display Settings

- Brightness level (0-8)
- Auto-start enable/disable
- Camera operator vs. Talent mode

## Tally States

| State | Color | Meaning |
|-------|-------|---------|
| **PROGRAM** | Red | Camera is ON-AIR |
| **PREVIEW** | Green | Camera selected for preview |
| **UNSELECTED** | Blue | Camera not selected |
| **NO_TALLY** | Purple | No connection/status |
| **ERROR** | Red (flashing) | Communication error |

## Hardware Configuration

For custom hardware or modifications, see [HARDWARE_CONFIG.md](Documentation/HARDWARE_CONFIG.md)

## Development

### Project Structure

```
STAC/                    # PlatformIO project (primary)             
├── include/             # Header files
│   ├── Device_Config.h  # Hardware selection
│   ├── BoardConfigs/    # Board-specific configs
│   ├── Hardware/        # HAL implementations
│   ├── Network/         # WiFi, web server
│   ├── Storage/         # NVS configuration
│   ├── State/           # State management
│   └── Application/     # Main app logic
├── src/                 # Implementation files
├── platformio.ini       # Build configuration
└── Documentation/       # User guides and docs
```

### Building from Source

**Requirements:**

- C++17 or later
- ESP32 Arduino core 3.x+
- Libraries (auto-installed by PlatformIO):
  - LiteLED v3.0.0
  - XP_Button v1.0.3
  - I2C_MPU6886 (for ATOM)
  - SensorLib (for Waveshare)

**IMPORTANT: Building Firmware Binaries**

Always use the custom build script to generate firmware binaries:

```bash
cd STAC
pio run -e <environment> -t merged -t ota
```

This creates properly named binaries:
- `STAC_v<version>_<board>_<build>.bin` - OTA update
- `STAC_v<version>_<board>_<build>_FULL.bin` - Web Serial flash

For complete build instructions and naming conventions, see:
[Building Firmware Binaries](STAC/Documentation/Developer/Building%20Firmware%20Binaries.md)

### Adding New Hardware

1. Create board config: `include/BoardConfigs/YourBoard_Config.h`
2. Define all pins and settings
3. Add option to `Device_Config.h`
4. Test and submit PR!

See [DEVELOPER_GUIDE.md](Documentation/DEVELOPER_GUIDE.md) for details.

## Troubleshooting

### Device won't connect to WiFi

- Check SSID/password in configuration
- Ensure 2.4GHz WiFi (ESP32 doesn't support 5GHz)
- Check signal strength (move closer to router)

### Display shows wrong colors

- Check `DISPLAY_COLOR_ORDER` in board config
- Try RGB vs GRB setting

### IMU orientation incorrect

- Adjust `IMU_ORIENTATION_OFFSET` in board config
- Values: 0, 1, 2, or 3 (90° increments)

### Peripheral mode not detected

- Verify jumper wire connection
- Check pin numbers in board config
- Reset device after installing jumper

## Contributing

Contributions welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Test thoroughly on hardware
4. Submit a pull request

## License

[Insert your license here - MIT, GPL, etc.]

## Credits

**Author:** Xylopyrographer
**Version:** 2.3.0  
**Last Updated:** December 2024

### Libraries Used

- [LiteLED](https://github.com/Xylopyrographer/LiteLED) - LED control
- [XP_Button](https://github.com/Xylopyrographer/XP_Button) - Button handling
- ESP32 Arduino Core - Base framework

### Special Thanks

- M5Stack for excellent hardware
- Waveshare for the ESP32-S3 Matrix
- The ESP32 community

## Support

- **Issues:** [GitHub Issues](https://github.com/Xylopyrographer/STAC/issues)
- **Discussions:** [GitHub Discussions](https://github.com/Xylopyrographer/STAC/discussions)
- **Documentation:** [Wiki](https://github.com/Xylopyrographer/STAC/wiki)

---

**Made with ❤️ for the video production community**

<!-- //  --- EOF --- // -->
