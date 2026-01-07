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
2. Hold STAC button 4-6 seconds (down arrow icon)
3. Connect to STAC-XXXXX WiFi (password: 1234567890)
4. Open browser: http://192.168.6.14
5. Select the .ota.bin file and click Update
6. Wait for update to complete and STAC to reboot

### First Time Setup (After Fresh Flash)
1. Hold STAC button for 2+ seconds
2. Connect to STAC-XXXXX WiFi network
3. Browser should auto-open to setup page (captive portal)
4. Click "Setup" tab
5. Enter WiFi and Roland switch details
6. Click Submit
7. STAC will reboot and connect to your network
