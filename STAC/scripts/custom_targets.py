# Register a custom PlatformIO/SCons target: `-t merged`
# - Ensures normal build completes (depends on firmware.bin)
# - Creates merged-<env>.bin in the build dir (using esptool if available)
# - Copies the merged image to <project>/bin/ as merged-<env>.bin

import os
import shutil
import subprocess
from SCons.Script import Import  # type: ignore

Import("env")


def _ensure_dir(path: str):
    os.makedirs(path, exist_ok=True)


def _read(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()


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
    board = env.BoardConfig()
    chip = (board.get("build.mcu") or "esp32").lower()

    # Offsets (Arduino-ESP32 defaults)
    OFF_BOOTLOADER = 0x1000
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

    merged_src = os.path.join(build_dir, f"merged-{env_name}.bin")
    out_dir = os.path.join(project_dir, "bin")
    _ensure_dir(out_dir)
    merged_dst = os.path.join(out_dir, f"merged-{env_name}.bin")

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
                f"0x{OFF_BOOTLOADER:X}", bootloader_path,
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
    size = os.path.getsize(merged_dst)
    print(f"[merged target] Copied: {merged_src} -> {merged_dst} ({size} bytes)")
    return 0


# Create a human-friendly custom target visible to `pio run -t merged`
env.AddCustomTarget(
    name="merged",
    dependencies=["$BUILD_DIR/${PROGNAME}.bin"],  # triggers normal build, then post action runs
    actions=[_do_merge_and_copy_action],
    title="Export merged firmware",
    description="Create & copy merged-<env>.bin into ./bin for Web Serial flashing",
)
