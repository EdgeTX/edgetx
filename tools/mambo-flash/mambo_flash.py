# SPDX-License-Identifier: GPL-2.0-only
"""Mambo controller-application uploader over the original TBS HID bootloader.

Default mode validates and prints an offline plan. --probe sends discovery only.
--execute requires an approved SHA256 and explicit controller serial. No RF or
bootloader updates, raw flash operations, resets, resume, or automatic re-erase.
"""
import argparse
import ctypes as c
import ctypes.util
from datetime import datetime, timezone
import hashlib
import json
import math
from pathlib import Path
import re
import shutil
import struct
import sys
import tempfile
import time

HARDWARE = 0x00041001
FIRMWARE = 0x0300  # Observed TBS version encoding for this EdgeTX 3.0 bench port.
TARGET = 0xEA  # Controller application; never the internal RF device.
ORIGIN = 0x10
CHUNK = 48
MAX_RANGE = 128 * CHUNK
REPORT_DESCRIPTOR = bytes.fromhex("0600ff0901a1011901294015012540750895408100190129409100c0")


def legacy_crc32(data):
    # Historical updater checksum: OR, not XOR, in the polynomial condition.
    value = 0xFFFFFFFF
    for byte in data:
        reflected = int(f"{byte:08b}"[::-1], 2)
        for _ in range(8):
            condition = (value & 0x80000000) or (reflected & 0x80)
            value = ((value << 1) ^ (0x04C11DB7 if condition else 0)) & 0xFFFFFFFF
            reflected <<= 1
    return int(f"{value:032b}"[::-1], 2) ^ 0xFFFFFFFF


def crc8(data):
    value = 0
    for byte in data:
        value ^= byte
        for _ in range(8):
            value = ((value << 1) ^ (0xD5 if value & 0x80 else 0)) & 255
    return value


def frame(payload, destination=TARGET, origin=ORIGIN, kind=0x38):
    body = bytes([kind, destination, origin]) + payload
    if len(body) > 61:
        raise ValueError("CRSF frame exceeds 64 bytes")
    return bytes([0xC8, len(body) + 1]) + body + bytes([crc8(body)])


def parse_report(raw):
    if len(raw) < 4 or raw[0] != 0xC8 or not 2 <= raw[1] <= 62:
        raise ValueError("invalid CRSF report header")
    size = raw[1] + 2
    if len(raw) < size:
        raise ValueError("truncated CRSF report")
    packet = raw[:size]
    # The captured bootloader emits DownloadFinished with an invalid outer CRC;
    # Agent explicitly exempts this one message. It is usable only after the
    # independent terminal AskForBytes checksum has matched the image.
    legacy_finish = len(packet) == 7 and packet[2] == 0x38 and packet[5] == 6
    if not legacy_finish and crc8(packet[2:-1]) != packet[-1]:
        raise ValueError("invalid CRSF CRC8")
    return packet


def load_image(path, elf_path, expected_sha=None):
    if sys.flags.optimize:
        raise ValueError("ELF verification requires Python assertions enabled")
    image = path.read_bytes()
    if not 8 <= len(image) <= 0xB4000:
        raise ValueError("image exceeds the application-only flash allocation")
    digest = hashlib.sha256(image).hexdigest()
    if expected_sha is not None:
        if not re.fullmatch(r"[0-9a-fA-F]{64}", expected_sha):
            raise ValueError("expected SHA256 must contain exactly 64 hexadecimal characters")
        if digest != expected_sha.lower():
            raise ValueError("image differs from the approved SHA256; run flash-plan again")
    # Lazy import: protocol tests and discovery need no ELF/Python packages.
    from verify_firmware import verify
    try:
        size, verified_digest = verify(elf_path, path)
    except Exception as error:
        # Malformed ELF files can raise parser-specific errors as well as
        # contract assertions. All must fail closed before opening USB.
        raise ValueError(f"application ELF/bin verification failed: {error}") from error
    if size != len(image) or verified_digest != digest:
        raise ValueError("artifacts changed during verification")
    return image


def header(image):
    return frame(struct.pack(">BBIHII", 1, 0, HARDWARE, FIRMWARE,
                             len(image), legacy_crc32(image)))


def chunks(image, start, end, indexes=None):
    if not 0 <= start < end <= len(image) or end - start > MAX_RANGE or start % CHUNK or \
            (end != len(image) and end % CHUNK):
        raise ValueError("bootloader requested an unsupported image range")
    count = math.ceil((end - start) / CHUNK)
    for index in range(count) if indexes is None else indexes:
        if not 0 <= index < count:
            raise ValueError("chunk index outside requested range")
        begin = start + index * CHUNK
        yield frame(bytes([3, index, count]) + image[begin:min(begin + CHUNK, end)])


def device_info(packet):
    if len(packet) < 7 or packet[2] != 0x29:
        return None
    payload = packet[5:-1]
    end = payload.find(b"\0")
    if end < 0 or len(payload) < end + 15:
        raise ValueError("malformed device-information reply")
    serial, hardware, firmware, parameters, protocol = struct.unpack(">IIIBB", payload[end+1:end+15])
    return {"name": payload[:end].decode("ascii"), "address": packet[4],
            "serial": serial, "hardware_id": hardware, "firmware_id": firmware,
            "parameter_count": parameters, "protocol": protocol}


class HidInfo(c.Structure):
    pass


HidInfo._fields_ = [("path", c.c_char_p), ("vendor_id", c.c_ushort),
                   ("product_id", c.c_ushort), ("serial_number", c.c_wchar_p),
                   ("release_number", c.c_ushort), ("manufacturer_string", c.c_wchar_p),
                   ("product_string", c.c_wchar_p), ("usage_page", c.c_ushort),
                   ("usage", c.c_ushort), ("interface_number", c.c_int),
                   ("next", c.POINTER(HidInfo)), ("bus_type", c.c_int)]


class NativeHid:
    def __init__(self, library=None):
        library = library or ctypes_library()
        self.lib = c.CDLL(library)
        if not hasattr(self.lib, "hid_get_report_descriptor"):
            raise OSError("HIDAPI 0.14+ is required (missing report descriptor API)")
        signatures = {
            "hid_init": ([], c.c_int), "hid_exit": ([], c.c_int),
            "hid_enumerate": ([c.c_ushort, c.c_ushort], c.POINTER(HidInfo)),
            "hid_free_enumeration": ([c.POINTER(HidInfo)], None),
            "hid_open_path": ([c.c_char_p], c.c_void_p),
            "hid_close": ([c.c_void_p], None),
            "hid_error": ([c.c_void_p], c.c_wchar_p),
            "hid_get_report_descriptor": ([c.c_void_p, c.c_void_p, c.c_size_t], c.c_int),
            "hid_write": ([c.c_void_p, c.c_void_p, c.c_size_t], c.c_int),
            "hid_read_timeout": ([c.c_void_p, c.c_void_p, c.c_size_t, c.c_int], c.c_int),
        }
        for name, (arguments, result) in signatures.items():
            function = getattr(self.lib, name, None)
            if function is None:
                raise OSError(f"unsupported HIDAPI library: missing {name}")
            function.argtypes, function.restype = arguments, result
        self.handle = None
        self.initialized = False
        if self.lib.hid_init():
            self.lib.hid_exit()
            raise OSError("HID initialization failed")
        self.initialized = True
        # macOS native HIDAPI supports exclusive opening without a USB reset
        # or kernel-driver detach. Make that behavior explicit for live tests.
        if hasattr(self.lib,"hid_darwin_set_open_exclusive"):
            self.lib.hid_darwin_set_open_exclusive.argtypes = [c.c_int]
            self.lib.hid_darwin_set_open_exclusive.restype = None
            self.lib.hid_darwin_set_open_exclusive(1)
        listing = self.lib.hid_enumerate(0x04D8, 0xF94C)
        try:
            paths = []
            item = listing
            while item:
                info = item.contents
                if info.interface_number == 0 and info.usage_page == 0xFF00 and info.usage == 1:
                    paths.append(info.path)
                item = info.next
            if len(paths) != 1:
                raise OSError(f"expected one Mambo HID interface; found {len(paths)}")
            self.handle = self.lib.hid_open_path(paths[0])
            if not self.handle:
                raise OSError(self.lib.hid_error(None) or "cannot open Mambo HID")
            descriptor = c.create_string_buffer(1024)
            size = self.lib.hid_get_report_descriptor(self.handle, descriptor, 1024)
            if size <= 0 or descriptor.raw[:size] != REPORT_DESCRIPTOR:
                raise OSError("report descriptor differs from the verified bootloader")
        except Exception:
            self.close()
            raise
        finally:
            self.lib.hid_free_enumeration(listing)

    def send(self, packet):
        data = bytes([0]) + packet
        buffer = c.create_string_buffer(data, len(data))
        size = self.lib.hid_write(self.handle, buffer, len(data))
        if size != len(data):
            raise OSError(self.lib.hid_error(self.handle) or "short HID write")

    def receive(self, timeout_ms):
        buffer = c.create_string_buffer(64)
        size = self.lib.hid_read_timeout(self.handle, buffer, 64, timeout_ms)
        if size < 0:
            raise OSError(self.lib.hid_error(self.handle) or "HID read failed")
        return buffer.raw[:size] if size else None

    def close(self):
        if self.handle:
            self.lib.hid_close(self.handle)
            self.handle = None
        if self.initialized:
            self.lib.hid_exit()
            self.initialized = False


def ctypes_library():
    # Prefer native HID/hidraw, not a raw libusb claim or driver detach.
    names = ("hidapi-hidraw", "hidapi") if sys.platform.startswith("linux") else ("hidapi",)
    for name in names:
        path = c.util.find_library(name)
        if path:
            return path
    for candidate in ("/opt/homebrew/lib/libhidapi.dylib",
                      "/usr/local/lib/libhidapi.dylib"):
        if Path(candidate).is_file():
            return candidate
    raise OSError("native HIDAPI 0.14+ unavailable; install it or use --hid-library")


class LoggedTransport:
    def __init__(self, transport, log):
        self.transport, self.log = transport, log
        self.header_attempted = False

    def record(self, direction, packet):
        self.log.write(json.dumps({"monotonic":time.monotonic(),
                                   "direction":direction,"hex":packet.hex()})+"\n")
        self.log.flush()

    def send(self, packet):
        # Set before the write: an error can leave delivery uncertain.
        if packet[2] == 0x38 and packet[5] == 1:
            self.header_attempted = True
        self.transport.send(packet)
        self.record("OUT",packet)

    def receive(self, timeout_ms):
        packet = self.transport.receive(timeout_ms)
        if packet:
            self.record("IN",packet)
        return packet


def discover(transport, expected_serial=None, event=lambda **kw: None):
    transport.send(bytes.fromhex("c80428000e7c"))
    stop = time.monotonic() + 3
    while time.monotonic() < stop:
        raw = transport.receive(200)
        if not raw:
            continue
        packet = parse_report(raw)
        info = device_info(packet)
        if info and info["address"] == TARGET:
            event(action="device_info", info=info)
            if info["name"] != "TBS BOOTLOADER" or info["hardware_id"] != HARDWARE or \
                    (expected_serial is not None and info["serial"] != expected_serial) or info["protocol"] != 1:
                raise ValueError("target identity/mode differs from the verified Mambo bootloader")
            return info
    raise TimeoutError("no controller bootloader device-information reply")


def upload(transport, image, event=lambda **kw: None, pause=time.sleep,
           chunk_delay=0.020):
    if not 0.010 <= chunk_delay <= 0.100:
        raise ValueError("chunk delay must be between 10 and 100 ms")
    # One application header initiates erasure. No automatic restart or resume.
    checksum = legacy_crc32(image)
    transport.send(header(image))
    event(action="header_sent", bytes=len(image), checksum=f"0x{checksum:08x}")
    started = time.monotonic()
    phase_deadline = started + 45
    current = None
    expected_start = 0
    complete_checksum = False
    retries = 0
    while time.monotonic() - started < 600:
        if time.monotonic() > phase_deadline:
            raise TimeoutError("bootloader progress timed out; no automatic re-erase attempted")
        raw = transport.receive(200)
        if not raw:
            continue
        packet = parse_report(raw)
        if packet[2] != 0x38 or len(packet) < 7 or packet[3:5] != bytes([ORIGIN, TARGET]):
            continue
        payload = packet[5:-1]
        command = payload[0]
        if command == 7:
            if len(payload) != 2 or payload[1] > 100 or current is not None:
                raise ValueError("unexpected erase-progress response")
            event(action="erase_progress", percent=payload[1])
        elif command == 2:
            if len(payload) != 13:
                raise ValueError("malformed range request")
            start, end, reported_crc = struct.unpack(">III", payload[1:])
            if start == end == 0:
                if expected_start != len(image) or reported_crc != checksum:
                    raise ValueError("terminal image checksum/length verification failed")
                complete_checksum = True
                phase_deadline = time.monotonic() + 15
                event(action="image_checksum_verified", checksum=f"0x{reported_crc:08x}")
                continue
            if complete_checksum:
                raise ValueError("new data requested after terminal verification")
            if current == (start, end):
                retries += 1
                if retries > 3:
                    raise ValueError("repeated range request retry limit exceeded")
                # The capture contains a duplicate final-range request. Poll
                # for its bitmap instead of blindly sending the range twice.
                transport.send(frame(bytes([4])))
                event(action="duplicate_range_polled",start=start,end=end)
                phase_deadline = time.monotonic() + 15
                continue
            else:
                if start != expected_start:
                    raise ValueError("nonsequential bootloader range request")
                current = (start, end)
                retries = 0
            packets = list(chunks(image, start, end))
            for data_packet in packets:
                transport.send(data_packet)
                pause(chunk_delay)
            transport.send(frame(bytes([4])))
            expected_start = end
            phase_deadline = time.monotonic() + 15
            event(action="range_sent", start=start, end=end, chunks=len(packets),
                  percent=round(100*end/len(image),1))
        elif command == 5:
            if len(payload) != 26:
                raise ValueError("unexpected/malformed range report")
            bitmap, status = payload[1:17], payload[17]
            start, end = struct.unpack(">II", payload[18:26])
            if current is None:
                raise ValueError("unexpected range report without an active transfer")
            if (start, end) != current:
                # Delayed duplicate report for an earlier range is benign.
                if end <= current[0]:
                    continue
                raise ValueError("range report does not match the current transfer")
            if status != 0:
                raise ValueError(f"unsupported range report status {status}")
            count = math.ceil((end - start)/CHUNK)
            missing = [i for i in range(count) if not bitmap[i//8] & (1 << (i%8))]
            if missing:
                retries += 1
                if retries > 3:
                    raise ValueError("missing-chunk retry limit exceeded")
                for data_packet in chunks(image,start,end,missing):
                    transport.send(data_packet)
                    pause(chunk_delay)
                transport.send(frame(bytes([4])))
                event(action="chunks_retried", start=start, count=len(missing))
            phase_deadline = time.monotonic() + 15
        elif command == 6:
            if len(payload) != 1 or not complete_checksum:
                raise ValueError("finished response before image checksum verification")
            event(action="download_finished", elapsed_seconds=round(time.monotonic()-started,2))
            return
        else:
            raise ValueError(f"unsupported firmware response 0x{command:02x}; transfer stopped")
    raise TimeoutError("overall transfer deadline exceeded")


def serial_number(value):
    try:
        number = int(value, 0)
    except ValueError as error:
        raise argparse.ArgumentTypeError("serial must be decimal or 0x-prefixed hex") from error
    if not 0 < number <= 0xFFFFFFFF:
        raise argparse.ArgumentTypeError("serial must be between 1 and 0xffffffff")
    return number


def make_plan(image):
    return {
        "target": "Mambo controller application only; external CRSF included",
        "bytes": len(image),
        "sha256": hashlib.sha256(image).hexdigest(),
        "legacy_checksum": f"0x{legacy_crc32(image):08x}",
        "hardware_id": f"0x{HARDWARE:08x}",
        "firmware_id": f"0x{FIRMWARE:04x}",
        "application_flash": ["0x0800c000", "0x080c0000"],
        "elf_memory_and_vectors_verified": True,
        "bootloader_update": False,
        "internal_rf_update": False,
        "reset_command": False,
        "usb_access": False,
    }


def execute(args, image, plan):
    # Make logs before USB access. Never overwrite an existing evidence folder.
    if args.log_dir:
        log_dir = args.log_dir
        log_dir.mkdir(parents=True, exist_ok=False)
    else:
        args.log_root.mkdir(parents=True, exist_ok=True)
        prefix = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%SZ-")
        log_dir = Path(tempfile.mkdtemp(prefix=prefix, dir=args.log_root))
    if shutil.disk_usage(log_dir).free < 64 * 1024 * 1024:
        raise OSError("less than 64 MiB free for transfer logs; USB not opened")
    plan = dict(plan, expected_serial=args.expected_serial,
                chunk_delay_ms=args.chunk_delay_ms)
    (log_dir / "plan.json").write_text(json.dumps(plan, indent=2) + "\n")
    print(f"Transfer logs: {log_dir.resolve()}", flush=True)
    transport = logged = None
    try:
        with (log_dir / "events.jsonl").open("x") as events, \
                (log_dir / "usb.jsonl").open("x") as packets:
            def event(**data):
                data["utc"] = datetime.now(timezone.utc).isoformat()
                events.write(json.dumps(data) + "\n")
                events.flush()
                print(json.dumps(data), flush=True)

            transport = NativeHid(args.hid_library)
            logged = LoggedTransport(transport, packets)
            info = discover(logged, args.expected_serial, event)
            upload(logged, image, event, chunk_delay=args.chunk_delay_ms / 1000)
            result = {
                "application_transfer_completed": True,
                "image_sha256": plan["sha256"],
                "device": info,
                "terminal_checksum_verified": True,
                "flash_readback_performed": False,
                "application_startup_verified": False,
                "reset_command_sent": False,
            }
            (log_dir / "result.json").write_text(json.dumps(result, indent=2) + "\n")
        print("Application transfer completed; terminal checksum matched. "
              "No reboot command sent. Disconnect USB and bench-test on batteries.")
    except (Exception, KeyboardInterrupt):
        if logged and logged.header_attempted:
            print("Transfer stopped after an erase-triggering write was attempted. "
                  "The application may be incomplete: keep bootloader mode active; "
                  "do not boot it. No automatic retry or re-erase.", file=sys.stderr)
        raise
    finally:
        if transport:
            transport.close()


def main(argv=None):
    parser = argparse.ArgumentParser(description=__doc__)
    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--probe", action="store_true",
                      help="query controller identity over HID; never erase or upload")
    mode.add_argument("--execute", action="store_true",
                      help="flash the verified application; initiates application erase")
    parser.add_argument("--image", type=Path, help="raw application firmware.bin")
    parser.add_argument("--elf", type=Path, help="matching firmware.elf with symbols")
    parser.add_argument("--expected-sha256", help="approved binary SHA256 from the offline plan")
    parser.add_argument("--expected-serial", type=serial_number,
                        help="controller serial from --probe (decimal or 0x-prefixed hex)")
    logs = parser.add_mutually_exclusive_group()
    logs.add_argument("--log-dir", type=Path, help="new transfer log directory; never overwritten")
    logs.add_argument("--log-root", type=Path,
                      help="create a fresh transfer directory under this path")
    parser.add_argument("--hid-library", help="explicit native HIDAPI 0.14+ library path")
    parser.add_argument("--chunk-delay-ms", type=int, default=20,
                        help="data pacing, 10..100 ms (default: 20)")
    args = parser.parse_args(argv)
    if args.probe and (args.image or args.elf or args.expected_sha256 or
                       args.log_dir or args.log_root):
        parser.error("--probe cannot be combined with image or log arguments")
    if not args.probe and (not args.image or not args.elf):
        parser.error("offline plan and live flash require --image and --elf")
    if args.execute and (args.expected_serial is None or not args.expected_sha256 or
                         not (args.log_dir or args.log_root)):
        parser.error("--execute requires --expected-serial, --expected-sha256 and a log location")
    if not 10 <= args.chunk_delay_ms <= 100:
        parser.error("--chunk-delay-ms must be between 10 and 100")
    try:
        if args.probe:
            transport = NativeHid(args.hid_library)
            try:
                info = discover(transport, args.expected_serial)
                print(json.dumps(dict(info, application_erase_sent=False), indent=2))
            finally:
                transport.close()
            return 0
        image = load_image(args.image, args.elf, args.expected_sha256)
        plan = make_plan(image)
        if not args.execute:
            print(json.dumps(plan, indent=2))
            return 0
        execute(args, image, plan)
        return 0
    except (OSError, ValueError, TimeoutError, ImportError, struct.error) as error:
        print(f"Uploader stopped: {error}", file=sys.stderr)
        return 1
    except KeyboardInterrupt:
        print("Uploader interrupted; no automatic retry.", file=sys.stderr)
        return 130


if __name__ == "__main__":
    sys.exit(main())
