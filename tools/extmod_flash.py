#!/usr/bin/env python3
# /// script
# requires-python = ">=3.9"
# dependencies = ["pyserial>=3.5", "pyusb>=1.2", "libusb-package>=1.0"]
# ///
"""
Flash an external RF module through an EdgeTX radio's USB serial port,
using the CLI `serialpassthrough rfmod 1 <baud> <sport|ppm> hangup` command
(EdgeTX PR #7927).

The flashing protocols mirror what the radio does natively when flashing
from the SD card:

  * MULTI (MPM)   -> radio/src/io/multi_firmware_update.cpp
                     STK500v1 (optiboot) @ 57600, TX on PPM, RX on S.PORT
  * ELRS (STM32)  -> radio/src/io/multi_firmware_update.cpp (MULTI_TYPE_ELRS)
                     same STK500v1 protocol, half-duplex S.PORT @ 57600
  * FrSky (XJT..) -> radio/src/io/frsky_firmware_update.cpp
                     FrSky S.PORT bootloader @ 57600, half-duplex S.PORT

Radio setup: USB mode "Serial (VCP)" with the CLI assigned to the VCP
port (or choose "Serial" when plugging in). The passthrough ends when the
script closes the port (`hangup`: the host drops DTR).

Usage:
  uv run extmod_flash.py --list
  uv run extmod_flash.py --probe COM5
  uv run extmod_flash.py multi /dev/ttyACM0 multi-stm-serial-aetr-v1.3.4.0.bin
  uv run extmod_flash.py frsky COM5 XJT_build151.frk
  uv run extmod_flash.py elrs /dev/cu.usbmodem1 firmware.elrs
"""

from __future__ import annotations

import argparse
import struct
import sys
import time
from pathlib import Path

import serial
from serial.tools import list_ports

DEFAULT_BAUD = 57600


class FlashError(Exception):
    pass


def log(msg: str) -> None:
    print(msg, flush=True)


VERBOSE = False


def debug(msg: str) -> None:
    if VERBOSE:
        print(f"  [debug] {msg}", flush=True)


def progress(label: str, done: int, total: int) -> None:
    pct = 100 * done // total if total else 100
    bar = "#" * (pct // 4)
    end = "\n" if done >= total else ""
    print(f"\r{label}: [{bar:<25}] {pct:3d}% ({done}/{total})", end=end, flush=True)


# --------------------------------------------------------------------------
# Serial helpers
# --------------------------------------------------------------------------


class Port:
    def __init__(self, name: str, baud: int):
        # timeout=0: non-blocking reads, we do our own deadlines
        self.ser = serial.Serial(name, baud, timeout=0, write_timeout=2)

    def close(self) -> None:
        self.ser.close()

    def write(self, data: bytes) -> None:
        if VERBOSE:
            debug(f"TX {data.hex(' ')}" if len(data) <= 32 else f"TX {len(data)} bytes")
        self.ser.write(data)
        self.ser.flush()

    def clear(self) -> None:
        self.ser.reset_input_buffer()

    def read_byte(self, timeout: float) -> int | None:
        deadline = time.monotonic() + timeout
        while True:
            b = self.ser.read(1)
            if b:
                return b[0]
            if time.monotonic() >= deadline:
                return None
            time.sleep(0.0005)

    def read_for(self, duration: float) -> bytes:
        buf = bytearray()
        deadline = time.monotonic() + duration
        while time.monotonic() < deadline:
            buf += self.ser.read(256)
            time.sleep(0.005)
        return bytes(buf)


# --------------------------------------------------------------------------
# EdgeTX CLI passthrough
# --------------------------------------------------------------------------


def start_passthrough(port: Port, baud: int, mode: str) -> float:
    """Send the passthrough command; return the time the radio started
    executing it (the module is powered on ~500 ms later)."""
    cmd = f"serialpassthrough rfmod 1 {baud} {mode} hangup"
    log(f"Starting passthrough: '{cmd}'")
    port.clear()
    # LF executes the line (CR is ignored by the EdgeTX CLI). Never send a bare
    # LF: an empty line repeats the last command.
    port.write(cmd.encode() + b"\n")

    # The CLI echoes every character, then CRLF, then runs the command.
    expect = cmd.encode() + b"\r\n"
    buf = bytearray()
    deadline = time.monotonic() + 2.0
    while time.monotonic() < deadline:
        buf += port.ser.read(256)
        if expect in buf:
            break
        time.sleep(0.001)
    else:
        debug(f"RX {bytes(buf)!r}")
        raise FlashError(
            "No CLI echo from the radio. Check USB mode is 'Serial (VCP)' with "
            "the CLI on the VCP port, and that the radio is not still in "
            "passthrough from a previous run (unplug/replug USB)."
        )
    started = time.monotonic()

    # If the command failed, the CLI prints an error and a new "> " prompt.
    # On success it prints nothing and powers the module off for ~500 ms.
    tail = bytes(buf[buf.index(expect) + len(expect):]) + port.read_for(0.25)
    if tail:
        debug(f"RX after echo {tail!r}")
    text = tail.decode("ascii", "replace")
    if "> " in text or "invalid" in text.lower() or "not supported" in text.lower():
        raise FlashError(
            f"Radio rejected the passthrough command: {text.strip()!r}. "
            "Does this firmware include external module passthrough (PR #7927)?"
        )
    return started


def wait_until(t: float) -> None:
    delay = t - time.monotonic()
    if delay > 0:
        time.sleep(delay)


# --------------------------------------------------------------------------
# MULTI (STK500v1 / optiboot), as multi_firmware_update.cpp
# --------------------------------------------------------------------------

STK_OK = 0x10
STK_INSYNC = 0x14
CRC_EOP = 0x20
STK_GET_SYNC = 0x30
STK_LEAVE_PROGMODE = 0x51
STK_LOAD_ADDRESS = 0x55
STK_PROG_PAGE = 0x64
STK_READ_SIGN = 0x75

MULTI_SIGN_SIZE = 24


def check_multi_signature(data: bytes) -> str:
    """Parse the trailing signature, like MultiFirmwareInformation.
    Returns a description; raises if the file is not an external MULTI image."""
    if len(data) < MULTI_SIGN_SIZE:
        raise FlashError("File too small to be a MULTI firmware")
    sig = data[-MULTI_SIGN_SIZE:]

    if sig.startswith(b"multi-x"):
        hexpart = sig[7:15]
        try:
            options = int(hexpart.decode("ascii"), 16)
        except ValueError:
            raise FlashError(f"Bad MULTI v2 signature: {sig!r}")
        board = options & 0x3
        optiboot = bool(options & 0x80)
        bl_check = bool(options & 0x100)
        inversion = bool(options & 0x200)
        telem = "multi_telemetry" if options & 0x800 else (
            "multi_status" if options & 0x400 else "none")
    else:
        boards = {b"multi-stm": 1, b"multi-avr": 0, b"multi-orx": 2}
        if sig[:9] not in boards:
            raise FlashError("No MULTI firmware signature found at end of file")
        board = boards[sig[:9]]
        optiboot = sig[10:11] == b"b"
        bl_check = sig[11:12] == b"c"
        telem = {b"t": "multi_status", b"s": "multi_telemetry"}.get(sig[12:13], "none")
        inversion = sig[13:14] == b"i"

    board_name = {0: "AVR", 1: "STM32", 2: "OrangeRX"}.get(board, "?")
    desc = (f"board={board_name} optiboot={optiboot} bootloader_check={bl_check} "
            f"telemetry={telem} inversion={inversion}")

    # isMultiExternalFirmware()
    if not ((inversion or board == 1) and optiboot and bl_check
            and telem == "multi_telemetry"):
        raise FlashError(
            f"Not a firmware for an external MULTI module ({desc}). "
            "It must be built with bootloader support and MULTI_TELEMETRY.")
    return desc


class MultiFlasher:
    def __init__(self, port: Port):
        self.port = port

    def expect(self, value: int, timeout: float = 0.1) -> bool:
        b = self.port.read_byte(timeout)
        return b == value

    def sync(self, timeout: float = 5.0) -> None:
        self.port.clear()
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            self.port.write(bytes([STK_GET_SYNC, CRC_EOP]))
            b = self.port.read_byte(0.1)
            if b == STK_INSYNC:
                if self.expect(STK_OK):
                    time.sleep(0.001)
                    return
        raise FlashError("Bootloader did not respond to sync")

    def read_signature(self) -> bytes:
        self.port.clear()
        self.port.write(bytes([STK_READ_SIGN, CRC_EOP]))
        if not self.expect(STK_INSYNC):
            raise FlashError("No response to signature request")
        sig = bytearray()
        for _ in range(4):  # 3 signature bytes + STK_OK
            b = self.port.read_byte(0.1)
            if b is None:
                raise FlashError("Short signature response")
            sig.append(b)
        return bytes(sig)

    def load_address(self, word_addr: int) -> None:
        self.port.write(bytes([STK_LOAD_ADDRESS, word_addr & 0xFF,
                               (word_addr >> 8) & 0xFF, CRC_EOP]))
        if not (self.expect(STK_INSYNC) and self.expect(STK_OK)):
            raise FlashError(f"No response to load address 0x{word_addr:04X}")
        time.sleep(0.001)

    def prog_page(self, page: bytes) -> None:
        size = len(page)
        self.port.write(bytes([STK_PROG_PAGE, size >> 8, size & 0xFF, 0])
                        + page + bytes([CRC_EOP]))
        # page transfer over softserial takes ~45 ms for 256 bytes
        if not self.expect(STK_INSYNC, timeout=0.5):
            raise FlashError("No response to program page")
        # flash write time: native retries 4 x 100 ms
        b = None
        for _ in range(4):
            b = self.port.read_byte(0.1)
            if b:
                break
        if b != STK_OK:
            raise FlashError("Program page rejected")

    def leave(self) -> None:
        try:
            self.port.write(bytes([STK_LEAVE_PROGMODE, CRC_EOP]))
            self.expect(STK_INSYNC)
        except serial.SerialException:
            pass

    def flash(self, data: bytes) -> None:
        log("Waiting for STK500 bootloader...")
        self.sync()
        sig = self.read_signature()
        log(f"Device signature: {sig[:3].hex(' ')}")
        if sig[0] != 0x1E:
            raise FlashError(f"Unexpected device signature {sig.hex(' ')}")

        page_size, word_addr = 128, 0
        if sig[1] == 0x55 and sig[2] == 0xAA:
            # STM32 MULTI bootloader: 256-byte pages, app at word 0x1000
            page_size, word_addr = 256, 0x1000

        total = len(data)
        for off in range(0, total, page_size):
            page = data[off:off + page_size].ljust(page_size, b"\x00")
            self.port.clear()
            self.load_address(word_addr)
            self.prog_page(page)
            word_addr += page_size // 2
            progress("Writing", min(off + page_size, total), total)


# --------------------------------------------------------------------------
# FrSky S.PORT bootloader, as frsky_firmware_update.cpp
# --------------------------------------------------------------------------

START_STOP = 0x7E
BYTE_STUFF = 0x7D
STUFF_MASK = 0x20

PRIM_REQ_POWERUP = 0x00
PRIM_REQ_VERSION = 0x01
PRIM_CMD_DOWNLOAD = 0x03
PRIM_DATA_WORD = 0x04
PRIM_DATA_EOF = 0x05

PRIM_ACK_POWERUP = 0x80
PRIM_ACK_VERSION = 0x81
PRIM_REQ_DATA_ADDR = 0x82
PRIM_END_DOWNLOAD = 0x83
PRIM_DATA_CRC_ERR = 0x84

FRSK_HEADER = struct.Struct("<IBBBBIBBH")  # FrSkyFirmwareInformation
FRSK_FOURCC = 0x4B535246  # "FRSK"
FAMILY_EXTERNAL_MODULE = 1


def crc16_1021(data: bytes, crc: int = 0) -> int:
    # CRC-16/CCITT, MSB first, init 0 (crc16(CRC_1021, ...) in radio/src/crc.cpp)
    for b in data:
        crc ^= b << 8
        for _ in range(8):
            crc = ((crc << 1) ^ 0x1021) if crc & 0x8000 else (crc << 1)
            crc &= 0xFFFF
    return crc


def frsky_payload(path: Path, data: bytes, force: bool) -> bytes:
    if len(data) >= FRSK_HEADER.size:
        (fourcc, hdr_ver, vmaj, vmin, vrev, size, family, product,
         _crc) = FRSK_HEADER.unpack_from(data)
        if fourcc == FRSK_FOURCC:
            log(f"FRSK header: v{vmaj}.{vmin}.{vrev} family={family} "
                f"product=0x{product:02X} size={size}")
            if len(data) != FRSK_HEADER.size + size:
                raise FlashError("FRSK header size does not match file size")
            if family != FAMILY_EXTERNAL_MODULE and not force:
                raise FlashError(
                    f"Firmware is not for an external module (family={family}); "
                    "use --force to flash anyway")
            return data[FRSK_HEADER.size:]
    if path.suffix.lower() == ".frsk":
        raise FlashError(".frsk file without a valid FRSK header")
    return data


class FrskyFlasher:
    def __init__(self, port: Port):
        self.port = port
        self.rx = bytearray()
        self.last_frame: bytes | None = None

    def send_frame(self, prim: int, payload: bytes = b"") -> None:
        frame = bytearray([0x50, prim]) + bytearray(payload.ljust(5, b"\x00")[:5])
        frame.append(crc16_1021(bytes(frame)) & 0xFF)
        out = bytearray([START_STOP, 0xFF])
        for b in frame:
            if b in (START_STOP, BYTE_STUFF):
                out += bytes([BYTE_STUFF, b ^ STUFF_MASK])
            else:
                out.append(b)
        self.last_frame = bytes(out)
        self.port.write(self.last_frame)

    def resend(self) -> None:
        if self.last_frame:
            self.port.write(self.last_frame)

    def read_frame(self, timeout: float) -> bytes | None:
        """Return the 9 unstuffed bytes following 0x7E of the next frame
        coming from the module (physical id 0x5E). Our own frames may be
        echoed back on half-duplex S.PORT; they start with 0xFF and are
        skipped."""
        deadline = time.monotonic() + timeout
        frame: bytearray | None = None
        stuffed = False
        while True:
            b = self.port.read_byte(max(0.0, deadline - time.monotonic()))
            if b is None:
                return None
            if b == START_STOP:
                frame, stuffed = bytearray(), False
                continue
            if frame is None:
                continue
            if b == BYTE_STUFF:
                stuffed = True
                continue
            frame.append(b ^ STUFF_MASK if stuffed else b)
            stuffed = False
            if len(frame) == 9:
                f, frame = bytes(frame), None
                debug(f"RX frame {f.hex(' ')}")
                if f[0] == 0x5E and f[1] == 0x50:
                    return f

    def wait_prim(self, prims: set[int], timeout: float) -> bytes | None:
        deadline = time.monotonic() + timeout
        while True:
            f = self.read_frame(max(0.0, deadline - time.monotonic()))
            if f is None:
                return None
            if f[2] == PRIM_DATA_CRC_ERR:
                raise FlashError("Module reported a CRC error")
            if f[2] in prims:
                return f

    def request(self, prim: int, ack: int, attempts: int, what: str) -> bytes:
        for _ in range(attempts):
            self.send_frame(prim)
            f = self.wait_prim({ack}, 0.1)
            if f:
                return f
        raise FlashError(f"No response to {what}")

    def flash(self, data: bytes, attempts: int) -> None:
        if len(data) % 4:
            log("Warning: firmware size is not a multiple of 4, padding with 0xFF")
            data = data + b"\xFF" * (4 - len(data) % 4)

        log("Waiting for FrSky bootloader...")
        self.port.clear()
        self.request(PRIM_REQ_POWERUP, PRIM_ACK_POWERUP, attempts, "power-up request")

        time.sleep(0.02)
        self.port.clear()
        f = self.request(PRIM_REQ_VERSION, PRIM_ACK_VERSION, 10, "version request")
        log(f"Bootloader version reply: {f[3:7].hex(' ')}")

        time.sleep(0.2)
        self.port.clear()
        self.send_frame(PRIM_CMD_DOWNLOAD)

        total = len(data)
        base: int | None = None
        retries = 0
        timeout = 2.0  # first request may take a while (erase)
        while True:
            f = self.wait_prim({PRIM_REQ_DATA_ADDR, PRIM_END_DOWNLOAD}, timeout)
            if f is None:
                if retries <= 0:
                    raise FlashError("Module stopped requesting data")
                retries -= 1
                debug("timeout, resending last frame")
                self.resend()
                continue
            if f[2] == PRIM_END_DOWNLOAD:
                raise FlashError("Module ended the download early")

            retries, timeout = 4, 0.5
            address = struct.unpack_from("<I", f, 3)[0]
            if base is None:
                base = address & ~1023
            offset = address - base
            if offset < 0 or offset % 4:
                raise FlashError(f"Unexpected address request 0x{address:08X}")

            if offset >= total:
                break
            self.send_frame(PRIM_DATA_WORD,
                            data[offset:offset + 4] + bytes([address & 0xFF]))
            if offset % 1024 == 0 or offset + 4 >= total:
                progress("Writing", offset + 4, total)

        self.send_frame(PRIM_DATA_EOF)
        if not self.wait_prim({PRIM_END_DOWNLOAD}, 2.0):
            raise FlashError("Module did not confirm the end of download")


# --------------------------------------------------------------------------


def looks_like_other_firmware(data: bytes) -> str | None:
    """Catch an obviously wrong file for an ELRS module, which has no
    signature of its own."""
    if data[-MULTI_SIGN_SIZE:].startswith(b"multi-"):
        return "MULTI"
    if len(data) >= FRSK_HEADER.size and \
            FRSK_HEADER.unpack_from(data)[0] == FRSK_FOURCC:
        return "FrSky"
    return None


def list_serial_ports() -> None:
    ports = sorted(list_ports.comports(), key=lambda p: p.device)
    if not ports:
        log("No serial ports found")
    for p in ports:
        ids = f" [{p.vid:04X}:{p.pid:04X}]" if p.vid is not None else ""
        log(f"{p.device:20} {p.description}{ids}")


# --------------------------------------------------------------------------
# USB descriptor check (CDC ACM bmCapabilities)
# --------------------------------------------------------------------------

EDGETX_VCP_ID = (0x0483, 0x5740)  # USBD_CDC_VID/PID, the same on every radio

CS_INTERFACE = 0x24
ACM_FUNCTIONAL = 0x02
ACM_CAP_SEND_BREAK = 0x04

ACM_CAP_BITS = {
    0x01: "Comm_Feature",
    0x02: "Line_Coding/Control_Line_State/Serial_State",
    0x04: "Send_Break",
    0x08: "Network_Connection",
}


def acm_capabilities(dev) -> int | None:
    """Return bmCapabilities of the first ACM functional descriptor."""
    for cfg in dev:
        for intf in cfg:
            extra = bytes(intf.extra_descriptors)
            i = 0
            while i + 2 < len(extra):
                length = extra[i]
                if length < 2:
                    break
                if (extra[i + 1] == CS_INTERFACE and extra[i + 2] == ACM_FUNCTIONAL
                        and length >= 4):
                    return extra[i + 3]
                i += length
    return None


def parse_usb_location(location: str | None) -> tuple[int, tuple[int, ...]] | None:
    """pyserial location '1-1.1.1' (Linux adds ':1.0') -> (bus, port path)."""
    if not location:
        return None
    try:
        bus, _, ports = location.split(":")[0].partition("-")
        return int(bus), tuple(int(p) for p in ports.split("."))
    except ValueError:
        return None


def probe_radios(port_name: str | None) -> int:
    """Read the CDC ACM descriptor of the radio's USB serial port, telling
    whether a break ends the passthrough (EdgeTX PR #7927) without sending
    anything on the port. Works even while the radio is in passthrough."""
    try:
        import libusb_package
        import usb.core
    except ImportError as e:
        log(f"Error: {e} (run with 'uv run' to get pyusb and libusb-package)")
        return 1

    target = None
    if port_name:
        target = next((p for p in list_ports.comports()
                       if p.device.lower() == port_name.lower()), None)
        if target is None:
            log(f"Error: {port_name} not found")
            return 1
        if (target.vid, target.pid) != EDGETX_VCP_ID:
            log(f"Error: {port_name} is not an EdgeTX radio serial port (0483:5740)")
            return 1

    backend = libusb_package.get_libusb1_backend()
    devices = list(usb.core.find(find_all=True, backend=backend,
                                 idVendor=EDGETX_VCP_ID[0], idProduct=EDGETX_VCP_ID[1]))
    if target:
        # Every radio has the same VID:PID and serial number, so match the USB
        # port the radio is plugged into.
        loc = parse_usb_location(target.location)
        by_loc = [d for d in devices if loc and (d.bus, d.port_numbers) == loc]
        if by_loc:
            devices = by_loc
        elif len(devices) > 1:
            log(f"Warning: can't match {port_name} by USB location "
                f"({target.location!r}); showing every radio")

    found = 0
    for dev in devices:
        try:
            caps = acm_capabilities(dev)
        except usb.core.USBError as e:
            debug(f"{dev.idVendor:04X}:{dev.idProduct:04X}: {e}")
            continue
        if caps is None:
            continue
        found += 1
        names = [n for bit, n in ACM_CAP_BITS.items() if caps & bit] or ["none"]
        ids = f"{dev.idVendor:04X}:{dev.idProduct:04X}"
        path = ".".join(map(str, dev.port_numbers or ()))
        log(f"{ids} (USB location {dev.bus}-{path})")
        log(f"  ACM bmCapabilities: 0x{caps:02X} ({', '.join(names)})")
        if caps & ACM_CAP_SEND_BREAK:
            log("  Send_Break advertised: a break from the host ends the passthrough")
        else:
            log("  No Send_Break: older firmware, unplug the cable to end the passthrough")

    if not found:
        log("No EdgeTX radio (0483:5740) found" + (f" for {port_name}" if port_name else ""))
        return 1
    return 0


def main() -> int:
    global VERBOSE
    ap = argparse.ArgumentParser(
        description="Flash an external MULTI, ELRS (STM32) or FrSky module via EdgeTX "
                    "CLI serial passthrough.")
    ap.add_argument("module", nargs="?", choices=["multi", "elrs", "frsky"],
                    help="external module type: multi (MPM), elrs (STM32 based) "
                         "or frsky (XJT, R9M, ...)")
    ap.add_argument("port", nargs="?", help="radio serial port (COM5, /dev/ttyACM0, ...)")
    ap.add_argument("firmware", nargs="?", type=Path, help="firmware file (.bin, .elrs, .frk, .frsk)")
    ap.add_argument("--list", action="store_true", help="list serial ports and exit")
    ap.add_argument("--probe", nargs="?", const="", metavar="PORT",
                    help="show the passthrough capabilities of all radios, or PORT "
                         "(from the USB descriptor: whether a break ends it), and exit")
    ap.add_argument("--baud", type=int, default=DEFAULT_BAUD,
                    help=f"module baud rate (default {DEFAULT_BAUD})")
    ap.add_argument("--no-passthrough", action="store_true",
                    help="don't send the CLI command (passthrough already running, "
                         "or a plain USB-serial adapter); power the module on yourself")
    ap.add_argument("--boot-delay", type=float, default=0.5,
                    help="seconds between the CLI command and the module power-on "
                         "(default 0.5, matching the radio's power-off time)")
    ap.add_argument("--attempts", type=int, default=30,
                    help="FrSky power-up requests to send, 100 ms apart (default 30)")
    ap.add_argument("--force", action="store_true", help="skip firmware file checks")
    ap.add_argument("-v", "--verbose", action="store_true")
    args = ap.parse_args()
    VERBOSE = args.verbose

    if args.list:
        list_serial_ports()
        return 0
    if args.probe is not None:
        return probe_radios(args.probe or None)
    if not args.module or not args.port or not args.firmware:
        ap.error("module, port and firmware are required (or use --list)")

    try:
        data = args.firmware.read_bytes()
        kind = args.module

        if kind == "multi":
            mode = "ppm"
            if args.force:
                log("Skipping MULTI signature check (--force)")
            else:
                log(f"MULTI firmware: {check_multi_signature(data)}")
            payload = data
        elif kind == "elrs":
            # the radio does no file checks for ELRS either
            mode = "sport"
            other = looks_like_other_firmware(data)
            if other and not args.force:
                raise FlashError(f"This looks like a {other} firmware, not ELRS "
                                 "(use --force to flash anyway)")
            payload = data
        else:
            mode = "sport"
            payload = frsky_payload(args.firmware, data, args.force)
        log(f"{args.firmware.name}: {len(payload)} bytes, {kind} via {mode}")

        port = Port(args.port, args.baud)
    except (OSError, serial.SerialException, FlashError) as e:
        log(f"Error: {e}")
        return 1

    ok = False
    t0 = time.monotonic()
    try:
        if args.no_passthrough:
            log("Power the module on now...")
        else:
            started = start_passthrough(port, args.baud, mode)
            # The module is powered on boot-delay after the command starts;
            # bytes sent earlier would end up in the CLI's input buffer.
            wait_until(started + args.boot_delay)

        if kind in ("multi", "elrs"):
            flasher = MultiFlasher(port)
            try:
                flasher.flash(payload)
            finally:
                flasher.leave()
        else:
            FrskyFlasher(port).flash(payload, args.attempts)
        ok = True
        log(f"Firmware update successful ({time.monotonic() - t0:.1f} s)")
    except FlashError as e:
        print()
        log(f"Firmware update failed: {e}")
    except serial.SerialException as e:
        print()
        log(f"Serial error: {e}")
    except KeyboardInterrupt:
        print()
        log("Aborted")
    finally:
        # ends the passthrough on the radio (hangup)
        port.close()
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main())
