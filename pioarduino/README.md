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
   - Edit `pioarduino/include/Device_Config.h`
   - Uncomment the line for your board:
```cpp
     #define BOARD_M5STACK_ATOM_MATRIX
     // or
     // #define BOARD_WAVESHARE_ESP32_S3_MATRIX
```

3. **Build and upload:**
   
   **PlatformIO:**
```bash
   cd pioarduino
   pio run -e atom-matrix -t upload
   # or for Waveshare:
   # pio run -e waveshare-s3 -t upload
```
   
   **Arduino IDE:**
   - Open `Arduino/STAC/STAC.ino`
   - Select your board from Tools → Board
   - Click Upload

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
STAC/
├── pioarduino/              # PlatformIO project (primary)
│   ├── include/             # Header files
│   │   ├── Device_Config.h  # Hardware selection
│   │   ├── BoardConfigs/    # Board-specific configs
│   │   ├── Hardware/        # HAL implementations
│   │   ├── Network/         # WiFi, web server
│   │   ├── Storage/         # NVS configuration
│   │   ├── State/           # State management
│   │   └── Application/     # Main app logic
│   ├── src/                 # Implementation files
│   └── platformio.ini       # Build configuration
├── Arduino/                 # Arduino IDE compatibility
└── Documentation/           # User guides and docs
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

**Author:** Rob Lake (Xylopyrographer)  
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