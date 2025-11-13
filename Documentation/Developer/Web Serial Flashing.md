# Flashing STAC via Web Serial

This guide explains where to find the build artifacts, how the merged image is created, and how to flash it using a Web Serial capable browser.

## Build the firmware

From the `STAC` project folder:

```zsh
# M5Stack ATOM Matrix
pio run -e atom-matrix

# Waveshare ESP32-S3-Matrix
pio run -e waveshare-s3
```

Artifacts are generated under:
- `.pio/build/atom-matrix/`
- `.pio/build/waveshare-s3/`

Typical files:
- `bootloader.bin`
- `partitions.bin`
- `boot_app0.bin` (optional; only if generated for your board)
- `firmware.bin` (the application)
- `merged-<env>.bin` (single image produced post-build)

## The merged image

After each successful build, a post-build script creates a single flash image starting at offset `0x0`:

Offsets used (Arduino-ESP32 defaults):
- `0x1000` → `bootloader.bin`
- `0x8000` → `partitions.bin`
- `0xE000` → `boot_app0.bin` (if present)
- `0x10000` → `firmware.bin`

Output files:
- `.pio/build/atom-matrix/merged-atom-matrix.bin`
- `.pio/build/waveshare-s3/merged-waveshare-s3.bin`

The script prefers the system `esptool` with hyphenated subcommands (e.g., `merge-bin`), falls back to PlatformIO’s bundled `esptool.py` with underscore subcommands, and finally to an internal Python merge if needed.

### Manual merge (optional)

You can recreate the merged binary yourself using the `esptool` CLI:

```zsh
# ATOM (ESP32)
esptool --chip esp32 merge-bin -o .pio/build/atom-matrix/merged-atom-matrix.bin \
  0x1000 .pio/build/atom-matrix/bootloader.bin \
  0x8000 .pio/build/atom-matrix/partitions.bin \
  0x10000 .pio/build/atom-matrix/firmware.bin

# ESP32-S3 (Waveshare)
esptool --chip esp32s3 merge-bin -o .pio/build/waveshare-s3/merged-waveshare-s3.bin \
  0x1000 .pio/build/waveshare-s3/bootloader.bin \
  0x8000 .pio/build/waveshare-s3/partitions.bin \
  0x10000 .pio/build/waveshare-s3/firmware.bin
```

If `boot_app0.bin` exists, insert `0xE000 <path-to-boot_app0.bin>` before the `0x10000` line.

## Flash with a Web Serial browser

Option A: Single merged image (recommended)
1. Open a Web Serial flasher (e.g., Espressif’s esptool web UI):
   - https://espressif.github.io/esptool/
2. Connect to the device via the browser.
3. Select `merged-<env>.bin` and set the address to `0x0`.
4. Start flashing.

Option B: Multiple files with offsets
1. Use the same Web Serial flasher.
2. Add these file/address pairs:
   - `0x1000` → `bootloader.bin`
   - `0x8000` → `partitions.bin`
   - `0xE000` → `boot_app0.bin` (if present)
   - `0x10000` → `firmware.bin`
3. Flash.

## Notes
- Offsets shown are the Arduino-ESP32 defaults used by this project. If you change partition tables or flash mode/speed in `platformio.ini`, update offsets accordingly.
- The merged image is intended to be written starting at `0x0`.
- If your flasher supports “erase before flash”, leaving it enabled is fine.
