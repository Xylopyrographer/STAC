# Auto-merge ESP32 binaries into one flashable image at offset 0x0
# Works for Arduino-ESP32 defaults: bootloader@0x1000, partitions@0x8000,
# boot_app0@0xe000 (if present), app@0x10000

import os
import subprocess
import shutil
from SCons.Script import Import  # type: ignore

Import("env")


def _read(path):
    with open(path, "rb") as f:
        return f.read()


def _merge_segments(segments):
    # segments: list of tuples (offset:int, data:bytes)
    if not segments:
        return b""
    end = 0
    for off, data in segments:
        end = max(end, off + len(data))
    image = bytearray(b"\xFF" * end)
    for off, data in segments:
        image[off : off + len(data)] = data
    return bytes(image)


def _try_path(*parts):
    p = os.path.join(*parts)
    return p if os.path.exists(p) else None


def _merge_action(source, target, env):
    build_dir = env.subst("$BUILD_DIR")
    env_name = env.subst("$PIOENV")
    board = env.BoardConfig()
    chip = (board.get("build.mcu") or "esp32").lower()

    # Typical Arduino-ESP32 offsets
    OFF_BOOTLOADER = 0x1000
    OFF_PARTITIONS = 0x8000
    OFF_BOOT_APP0 = 0xE000
    OFF_APP = 0x10000

    firmware_path = _try_path(build_dir, env.subst("${PROGNAME}.bin"))
    bootloader_path = _try_path(build_dir, "bootloader.bin")
    partitions_path = _try_path(build_dir, "partitions.bin")
    boot_app0_path = _try_path(build_dir, "boot_app0.bin")

    missing = []
    if not firmware_path:
        missing.append("firmware.bin")
    if not bootloader_path:
        missing.append("bootloader.bin")
    if not partitions_path:
        missing.append("partitions.bin")

    if missing:
        print("[merge_bin] Skipping: missing required files:", ", ".join(missing))
        return

    segments = []
    # Required segments
    segments.append((OFF_BOOTLOADER, _read(bootloader_path)))
    segments.append((OFF_PARTITIONS, _read(partitions_path)))
    segments.append((OFF_APP, _read(firmware_path)))
    # Optional boot_app0 (present on many boards for OTA)
    if boot_app0_path:
        segments.append((OFF_BOOT_APP0, _read(boot_app0_path)))

    out_name = f"merged-{env_name}.bin"
    out_path = os.path.join(build_dir, out_name)

    # Try to use system 'esptool' CLI first (newer syntax uses hyphenated subcommands)
    try:
        esptool_exe = shutil.which("esptool")
        if esptool_exe:
            cmd = [
                esptool_exe,
                "--chip",
                chip,
                "merge-bin",
                "-o",
                out_path,
                f"0x{OFF_BOOTLOADER:X}",
                bootloader_path,
                f"0x{OFF_PARTITIONS:X}",
                partitions_path,
            ]
            if boot_app0_path:
                cmd += [f"0x{OFF_BOOT_APP0:X}", boot_app0_path]
            cmd += [f"0x{OFF_APP:X}", firmware_path]
            print("[merge_bin] Using system esptool:", " ".join(cmd))
            subprocess.run(cmd, check=True)
            size = os.path.getsize(out_path)
            print(f"[merge_bin] Created merged image via esptool: {out_path} ({size} bytes)")
            return
        
        # Fallback to PlatformIO's bundled esptool.py with underscore subcommand
        platform = env.PioPlatform()
        esptool_dir = platform.get_package_dir("tool-esptoolpy")
        if esptool_dir:
            pyexe = env.subst("$PYTHONEXE") or "python3"
            esptool_py = os.path.join(esptool_dir, "esptool.py")
            cmd = [
                pyexe,
                esptool_py,
                "--chip",
                chip,
                "merge_bin",
                "-o",
                out_path,
                f"0x{OFF_BOOTLOADER:X}",
                bootloader_path,
                f"0x{OFF_PARTITIONS:X}",
                partitions_path,
            ]
            if boot_app0_path:
                cmd += [f"0x{OFF_BOOT_APP0:X}", boot_app0_path]
            cmd += [f"0x{OFF_APP:X}", firmware_path]
            print("[merge_bin] Using bundled esptool.py:", " ".join(cmd))
            subprocess.run(cmd, check=True)
            size = os.path.getsize(out_path)
            print(f"[merge_bin] Created merged image via esptool.py: {out_path} ({size} bytes)")
            return
    except Exception as e:
        print(f"[merge_bin] esptool merge failed ({e}); falling back to Python merge")

    # Fallback: do an in-Python merge
    merged = _merge_segments(segments)
    with open(out_path, "wb") as f:
        f.write(merged)
    print(f"[merge_bin] Created merged image (fallback): {out_path} ({len(merged)} bytes)")


# Run after the main firmware binary is produced
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", _merge_action)
