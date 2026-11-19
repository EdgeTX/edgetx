#!/usr/bin/env python3
# /// script
# requires-python = ">=3.9"
# dependencies = ["pyserial>=3.5", "esptool>=4.5"]
# ///
"""
Flash the Bluetooth module (ESP32-C3 running BTWifiModule) through an EdgeTX
radio's USB serial port, using the CLI `serialpassthrough bt <baud> hangup`
command (EdgeTX PR for BT passthrough).

Flow:
  1. Open the radio's CDC port and send the passthrough command.
  2. The radio power-cycles the module, sends `AT+DOWNLOAD`, and bridges
     USB <-> BT UART. The module enters the ROM download bootloader.
  3. Flash the firmware in-process with esptool (same open port).
  4. Close the port (`hangup`) -> the radio power-cycles the module so it
     runs the newly-flashed app.

The module must be plugged into the radio's Bluetooth connector and the radio
must be in USB "Serial (VCP)" mode with the CLI on the VCP port.

Usage:
  uv run tools/bt_flash.py --port COM15 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
  uv run tools/bt_flash.py --port /dev/ttyACM0 firmware-merged.bin

Address/file pairs are flashed at the given addresses; a bare file (no leading
"0x") is flashed at 0x0. A merged single-image binary can be produced with:
  esptool.py --chip esp32c3 merge_bin -o merged.bin 0x0 bootloader.bin 0x8000 partitions.bin 0x10000 firmware.bin
"""

from __future__ import annotations

import argparse
import sys
import time

import serial
from serial.tools import list_ports

DEFAULT_BAUD = 115200
# The byte-by-byte bridge cannot reliably forward data at higher baud rates
# (the radio's UART re-baud races esptool's change), so flash at the ROM's
# default 115200.
FLASH_BAUD = 115200


class FlashError(Exception):
    pass


def log(msg: str) -> None:
    print(msg, flush=True)


def read_for(ser: serial.Serial, duration: float) -> bytes:
    buf = bytearray()
    deadline = time.monotonic() + duration
    while time.monotonic() < deadline:
        buf += ser.read(256)
        time.sleep(0.005)
    return bytes(buf)


def start_passthrough(ser: serial.Serial, baud: int, skip_download: bool) -> None:
    """Send the CLI passthrough command and wait for the radio to echo it."""
    cmd = f"serialpassthrough bt {baud} hangup"
    log(f"Starting passthrough: '{cmd}'")
    ser.reset_input_buffer()
    # LF executes the line (CR is ignored by the EdgeTX CLI).
    ser.write(cmd.encode() + b"\n")

    expect = cmd.encode() + b"\r\n"
    buf = bytearray()
    deadline = time.monotonic() + 2.0
    while time.monotonic() < deadline:
        buf += ser.read(256)
        if expect in buf:
            break
        time.sleep(0.001)
    else:
        raise FlashError(
            "No CLI echo from the radio. Check USB mode is 'Serial (VCP)' with "
            "the CLI on the VCP port, and that the radio is not already in "
            "passthrough from a previous run."
        )

    tail = bytes(buf[buf.index(expect) + len(expect):]) + read_for(ser, 0.25)
    text = tail.decode("ascii", "replace")
    # Only treat a CLI *rejection* as an error. The module may already be in
    # the ROM bootloader and emit "invalid header: ..." over the bridge; that
    # is harmless (esptool re-syncs) and must not be mistaken for a rejection.
    if "> " in text or "not supported" in text.lower():
        raise FlashError(
            f"Radio rejected the passthrough command: {text.strip()!r}. "
            "Is BLUETOOTH enabled in this firmware?"
        )

    # The radio has powered the module and started the bridge. Give it a moment
    # to settle, then ask the module to reboot into the ROM bootloader (unless
    # it is already there, e.g. forced in via BOOT+RESET during recovery).
    time.sleep(1.0)
    if skip_download:
        log("Skipping AT+DOWNLOAD (module already in ROM download mode)...")
    else:
        log("Sending AT+DOWNLOAD to enter the ROM bootloader...")
        ser.write(b"AT+DOWNLOAD\r\n")
        time.sleep(1.5)
    # Discard the ACK / boot log so esptool sees a clean link.
    ser.reset_input_buffer()


def flash(ser: serial.Serial, files: list[tuple[int, str]], chip: str,
          flash_baud: int, verbose: bool) -> None:
    from esptool.targets import ESP32C3ROM

    if chip != "esp32c3":
        raise FlashError(f"unsupported chip '{chip}' (only esp32c3 for now)")

    # The CLI phase used non-blocking reads (timeout=0); esptool's slip reader
    # blocks on the port read timeout, so switch to a blocking timeout for the
    # (slow) erase/write responses.
    ser.timeout = 3

    rom = ESP32C3ROM(ser, DEFAULT_BAUD, trace_enabled=verbose)
    log("Connecting to the module (ROM download mode)...")
    # The chip is already in download mode (AT+DOWNLOAD); do not reset it.
    rom.connect("no-reset")

    if flash_baud != DEFAULT_BAUD:
        rom.change_baud(flash_baud)

    # esptool erases/writes each file region, then finishes once at the end.
    for addr, path in files:
        with open(path, "rb") as f:
            data = f.read()
        data = data + b"\xff" * ((4 - len(data) % 4) % 4)  # pad to 4 bytes
        log(f"Flashing {path} ({len(data)} bytes) at 0x{addr:x}...")
        rom.flash_begin(len(data), addr)
        seq = 0
        while data:
            block = data[: rom.FLASH_WRITE_SIZE]
            block = block + b"\xff" * (rom.FLASH_WRITE_SIZE - len(block))
            rom.flash_block(block, seq)
            data = data[rom.FLASH_WRITE_SIZE:]
            seq += 1
    try:
        rom.flash_finish(False)  # don't reboot; the radio power-cycles on hangup
    except Exception as e:
        # The ROM's "stay in bootloader" response (0x06) is expected and
        # harmless: the flash content is already written, and the radio
        # power-cycles the module on hangup to run the new app.
        log(f"Note: {e}")
    log("Flash complete.")


def list_serial_ports() -> None:
    ports = sorted(list_ports.comports(), key=lambda p: p.device)
    if not ports:
        log("No serial ports found")
    for p in ports:
        ids = f" [{p.vid:04X}:{p.pid:04X}]" if p.vid is not None else ""
        log(f"{p.device:20} {p.description}{ids}")


def main() -> int:
    ap = argparse.ArgumentParser(
        description="Flash a BTWifiModule (ESP32-C3) through an EdgeTX radio's "
                    "USB serial port."
    )
    ap.add_argument("--port", help="radio serial port (COM15, /dev/ttyACM0, ...)")
    ap.add_argument("--chip", default="esp32c3", help="chip type (default esp32c3)")
    ap.add_argument("--baud", type=int, default=DEFAULT_BAUD,
                    help=f"CLI handshake baud (default {DEFAULT_BAUD})")
    ap.add_argument("--flash-baud", type=int, default=FLASH_BAUD,
                    help=f"esptool flash baud (default {FLASH_BAUD})")
    ap.add_argument("--skip-download", action="store_true",
                    help="skip AT+DOWNLOAD (module already in ROM download mode)")
    ap.add_argument("-v", "--verbose", action="store_true",
                    help="enable esptool byte-level tracing")
    ap.add_argument("--list", action="store_true",
                    help="list serial ports and exit")
    ap.add_argument("files", nargs="*",
                    help="address/file pairs, or a single file (flashed at 0x0)")
    args = ap.parse_args()

    if args.list:
        list_serial_ports()
        return 0

    if not args.port:
        ap.error("--port is required (or use --list)")

    if args.files and (args.files[0].lower().startswith("0x")
                       or args.files[0].isdigit()):
        # address/file pairs
        if len(args.files) % 2 != 0:
            ap.error("address/file arguments must come in pairs")
        pairs = [(int(args.files[i], 0), args.files[i + 1])
                 for i in range(0, len(args.files), 2)]
    else:
        # single (merged) file at 0x0
        pairs = [(0x0, f) for f in args.files]

    if not pairs:
        ap.error("no firmware files given")

    try:
        ser = serial.Serial(args.port, args.baud, timeout=0, write_timeout=2)
    except (OSError, serial.SerialException) as e:
        log(f"Error opening {args.port}: {e}")
        return 1

    ok = False
    try:
        start_passthrough(ser, args.baud, args.skip_download)
        flash(ser, pairs, args.chip, args.flash_baud, args.verbose)
        ok = True
    except FlashError as e:
        log(f"Firmware update failed: {e}")
    except (OSError, serial.SerialException) as e:
        log(f"Serial error: {e}")
    except KeyboardInterrupt:
        log("Aborted")
    except Exception as e:  # esptool raises FatalError and friends
        log(f"Error: {e}")
    finally:
        # closing the port drops DTR, ending the passthrough (hangup) and
        # power-cycling the module so it runs the new app
        ser.close()
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
