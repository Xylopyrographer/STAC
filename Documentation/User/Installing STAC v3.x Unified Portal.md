## Installing STAC v3.x Unified Portal

### Method 1: Web-Based Flash (Recommended for New Devices)
**For:**
- Brand new M5Stack ATOM/ATOM S3/StickC Plus devices
- LilyGO T-Display, AIPI-Lite boards
- Pre-OTA STAC firmware (very old versions)

**Steps:**
1. Visit: https://stac.yourdomain.com/flash
2. Connect STAC to computer via USB
3. Click the flash button for your board type
4. Select the USB port when prompted
5. Wait 30-60 seconds for flashing to complete
6. Done! Proceed to "First Time Setup" below

**Requirements:**
- Chrome, Edge, or Opera browser (WebSerial support)
- USB cable (data-capable, not charge-only)
- Windows, macOS, Linux, or ChromeOS

### Method 2: OTA Upgrade (For Existing v2.x STACs)
**For:**
- STACs already running v2.x firmware with OTA support

**Steps:**
1. Download: `STAC-v3.x.x-unified-[board].ota.bin`
2. Hold STAC button 2-4 seconds (orange config icon)
3. Connect to STAC-XXXXX WiFi (password: 1234567890)
4. Browser should auto-open to portal page (or visit http://192.168.6.14)
5. Click "Maintenance" tab
6. Select the .ota.bin file and click Update Firmware
7. Wait for update to complete and STAC to reboot

### First Time Setup (After Fresh Flash)
1. Hold STAC button for 2-4 seconds (orange config icon)
2. Connect to STAC-XXXXX WiFi network (password: 1234567890)
3. Browser should auto-open to portal page (captive portal)
   - Works on iOS, Android, Windows, macOS
   - If not automatic: visit http://192.168.6.14
4. Click "Setup" tab (default view)
5. Select your Roland switcher model (V-60HD or V-160HD)
6. Enter WiFi credentials and switcher IP address
7. Configure channel assignments and poll rate
8. Click Submit Configuration
9. STAC will reboot and connect to your network

### Maintenance Operations

**Firmware Updates:**
1. Hold button 2-4 seconds to enter portal mode
2. Connect to STAC-XXXXX WiFi
3. Click "Maintenance" tab
4. Click "Choose File" under Firmware Update
5. Select .bin file and click Update Firmware
6. Wait for completion and automatic reboot

**Factory Reset:**
1. Hold button 2-4 seconds to enter portal mode
2. Connect to STAC-XXXXX WiFi
3. Click "Maintenance" tab
4. Click "Factory Reset" button
5. Confirm when prompted
6. STAC will erase all settings and reboot

**Alternative Factory Reset (Hardware):**
- Hold button for 4-6 seconds (red frame, then green check)
- All settings erased, STAC reboots to defaults
