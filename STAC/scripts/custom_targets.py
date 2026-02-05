# Register custom PlatformIO/SCons targets
# - `-t merged`: Creates STAC_v<version>_<board>_<build>_FULL.bin for Web Serial flashing (includes bootloader + partitions + app)
# - `-t ota`: Creates STAC_v<version>_<board>_<build>.bin for OTA updates (app only)
#
# Binaries are collected in STAC/bin/ (flat structure for GitHub Releases):
#   STAC/bin/STAC_v3.0.0_ATOM_Matrix_FULL.bin
#   STAC/bin/STAC_v3.0.0_ATOM_Matrix_OTA.bin
#   STAC/bin/STAC_v3.0.0_Waveshare_S3_FULL.bin
#   ...
#
# When creating a binary, any existing binary of the same type is removed first.

import os
import re
import glob
import shutil
import subprocess
from SCons.Script import Import  # type: ignore

Import("env")


def _ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)


def _read(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()


def _get_version_from_header(project_dir: str) -> str:
    """Extract STAC_SOFTWARE_VERSION from Device_Config.h"""
    header_path = os.path.join(project_dir, "include", "Device_Config.h")
    try:
        with open(header_path, "r") as f:
            for line in f:
                match = re.search(r'#define\s+STAC_SOFTWARE_VERSION\s+"([^"]+)"', line)
                if match:
                    return match.group(1)
    except Exception as e:
        print(f"[custom_targets] Warning: Could not read version from {header_path}: {e}")
    return "UNKNOWN"


def _get_build_suffix(env_name: str, build_type: str) -> str:
    """Generate board suffix for filename
    
    Returns:
        str: board_suffix with build type
    
    Examples:
        atom-matrix + debug -> "ATOM_Matrix_D3"
        atom-matrix-release + release -> "ATOM_Matrix"
        waveshare-s3 + release -> "Waveshare_S3"
        m5stickc-plus + release -> "StickC_Plus"
        lilygo-t-display + release -> "LilyGo_T_Display"
        lilygo-t-qt + release -> "LilyGo_T_QT"
        aipi-lite + release -> "AIPI_Lite"
    """
    # Determine board name
    env_lower = env_name.lower()
    if "atom" in env_lower:
        board = "ATOM_Matrix"
    elif "waveshare" in env_lower:
        board = "Waveshare_S3"
    elif "m5stickc" in env_lower or "m5sp" in env_lower:
        board = "StickC_Plus"
    elif "t-qt" in env_lower or "tqt" in env_lower:
        board = "LilyGo_T_QT"
    elif "t-display" in env_lower or ("lilygo" in env_lower and "qt" not in env_lower):
        board = "LilyGo_T_Display"
    elif "aipi" in env_lower:
        board = "AIPI_Lite"
    else:
        board = "Unknown_Device"
    
    # Append build type for debug builds
    if "release" in env_lower or build_type == "release":
        return board
    else:
        return f"{board}_D3"  # Debug with LOG_LEVEL_DEBUG


def _merge_segments(segments):
    if not segments:
        return b""
    end = 0
    for off, data in segments:
        end = max(end, off + len(data))
    image = bytearray(b"\xFF" * end)
    for off, data in segments:
        image[off : off + len(data)] = data
    return bytes(image)



def _do_merge_and_copy_action(target, source, env):
    project_dir = env.subst("$PROJECT_DIR")
    build_dir = env.subst("$BUILD_DIR")
    env_name = env.subst("$PIOENV")
    build_type = env.GetProjectOption("build_type", "debug")
    board = env.BoardConfig()
    chip = (board.get("build.mcu") or "esp32").lower()

    # Get version and generate filename
    version = _get_version_from_header(project_dir)
    suffix = _get_build_suffix(env_name, build_type)
    merged_filename = f"STAC_v{version}_{suffix}_FULL.bin"

    # Bootloader offsets vary by chip family
    # ESP32 classic and S2 use 0x1000, newer chips (S3, C-series, H-series) use 0x0
    if chip in ("esp32s3", "esp32c3", "esp32c6", "esp32h2"):
        OFF_BOOTLOADER = 0x0     # Newer generation chips
    else:
        OFF_BOOTLOADER = 0x1000  # ESP32 classic, ESP32-S2
    
    # Partition table and app offsets are standard across all variants
    OFF_PARTITIONS = 0x8000
    OFF_BOOT_APP0 = 0xE000
    OFF_APP = 0x10000

    firmware_path = os.path.join(build_dir, env.subst("${PROGNAME}.bin"))
    bootloader_path = os.path.join(build_dir, "bootloader.bin")
    partitions_path = os.path.join(build_dir, "partitions.bin")
    boot_app0_path = os.path.join(build_dir, "boot_app0.bin")

    missing = []
    if not os.path.exists(firmware_path):
        missing.append("firmware.bin")
    if not os.path.exists(bootloader_path):
        missing.append("bootloader.bin")
    if not os.path.exists(partitions_path):
        missing.append("partitions.bin")
    if missing:
        print(f"[merged target] ERROR: missing required files: {', '.join(missing)}")
        return 1

    merged_src = os.path.join(build_dir, merged_filename)
    
    # Create flat output directory (STAC/bin/)
    out_dir = os.path.join(project_dir, "bin")
    _ensure_dir(out_dir)
    merged_dst = os.path.join(out_dir, merged_filename)
    
    # Remove any existing FULL binaries for this board/build type before creating new one
    pattern = os.path.join(out_dir, f"STAC_*_{suffix}_FULL.bin")
    for old_bin in glob.glob(pattern):
        try:
            os.remove(old_bin)
            print(f"[merged target] Removed old binary: {os.path.basename(old_bin)}")
        except Exception as e:
            print(f"[merged target] Warning: Could not remove {old_bin}: {e}")

    # Prefer system esptool (hyphen commands)
    try:
        esptool_exe = shutil.which("esptool")
        if esptool_exe:
            cmd = [
                esptool_exe,
                "--chip",
                chip,
                "merge-bin",
                "-o",
                merged_src,
                f"0x{OFF_BOOTLOADER:X}", bootloader_path,  # Bootloader at 0x1000
                f"0x{OFF_PARTITIONS:X}", partitions_path,
            ]
            if os.path.exists(boot_app0_path):
                cmd += [f"0x{OFF_BOOT_APP0:X}", boot_app0_path]
            cmd += [f"0x{OFF_APP:X}", firmware_path]
            print("[merged target] Using system esptool:", " ".join(cmd))
            subprocess.run(cmd, check=True)
        else:
            # Fall back to PlatformIO's bundled esptool.py (underscore commands)
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
                    merged_src,
                    f"0x{OFF_BOOTLOADER:X}", bootloader_path,
                    f"0x{OFF_PARTITIONS:X}", partitions_path,
                ]
                if os.path.exists(boot_app0_path):
                    cmd += [f"0x{OFF_BOOT_APP0:X}", boot_app0_path]
                cmd += [f"0x{OFF_APP:X}", firmware_path]
                print("[merged target] Using bundled esptool.py:", " ".join(cmd))
                subprocess.run(cmd, check=True)
            else:
                raise RuntimeError("esptool not available")
    except Exception as e:
        print(f"[merged target] esptool merge failed ({e}); falling back to Python merge")
        segments = [
            (OFF_BOOTLOADER, _read(bootloader_path)),
            (OFF_PARTITIONS, _read(partitions_path)),
            (OFF_APP, _read(firmware_path)),
        ]
        if os.path.exists(boot_app0_path):
            segments.append((OFF_BOOT_APP0, _read(boot_app0_path)))
        merged = _merge_segments(segments)
        with open(merged_src, "wb") as f:
            f.write(merged)

    shutil.copy2(merged_src, merged_dst)
    
    # Note: firmware was anonymized before merging, so checksum is already correct
    
    size = os.path.getsize(merged_dst)
    print(f"[merged target] Created: {merged_dst} ({size} bytes)")
    return 0


def _do_ota_copy_action(target, source, env):
    project_dir = env.subst("$PROJECT_DIR")
    build_dir = env.subst("$BUILD_DIR")
    env_name = env.subst("$PIOENV")
    build_type = env.GetProjectOption("build_type", "debug")

    # Get version and generate filename
    version = _get_version_from_header(project_dir)
    suffix = _get_build_suffix(env_name, build_type)
    ota_filename = f"STAC_v{version}_{suffix}_OTA.bin"

    firmware_path = os.path.join(build_dir, env.subst("${PROGNAME}.bin"))

    if not os.path.exists(firmware_path):
        print(f"[ota target] ERROR: firmware.bin not found at {firmware_path}")
        return 1

    # Create flat output directory (STAC/bin/)
    out_dir = os.path.join(project_dir, "bin")
    _ensure_dir(out_dir)
    ota_dst = os.path.join(out_dir, ota_filename)
    
    # Remove any existing OTA binaries for this board/build type before creating new one
    pattern = os.path.join(out_dir, f"STAC_*_{suffix}_OTA.bin")
    for old_bin in glob.glob(pattern):
        try:
            os.remove(old_bin)
            print(f"[ota target] Removed old binary: {os.path.basename(old_bin)}")
        except Exception as e:
            print(f"[ota target] Warning: Could not remove {old_bin}: {e}")

    shutil.copy2(firmware_path, ota_dst)
    
    # Note: firmware was already anonymized before copying
    
    size = os.path.getsize(ota_dst)
    print(f"[ota target] Created: {ota_dst} ({size} bytes)")
    return 0


# Create a human-friendly custom target visible to `pio run -t merged`
env.AddCustomTarget(
    name="merged",
    dependencies=["$BUILD_DIR/${PROGNAME}.bin"],  # triggers normal build, then post action runs
    actions=[_do_merge_and_copy_action],
    title="Export merged firmware",
    description="Create STAC_v<version>_<board>_<build>_FULL.bin in ./bin for Web Serial flashing",
)

# Create a human-friendly custom target visible to `pio run -t ota`
env.AddCustomTarget(
    name="ota",
    dependencies=["$BUILD_DIR/${PROGNAME}.bin"],  # triggers normal build, then post action runs
    actions=[_do_ota_copy_action],
    title="Export OTA firmware",
    description="Create STAC_v<version>_<board>_<build>.bin in ./bin for OTA updates",
)
