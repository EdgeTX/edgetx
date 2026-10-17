# SPDX-License-Identifier: GPL-2.0-only
"""Uploader state-machine tests. Synthetic transport; never opens USB."""
from pathlib import Path
import contextlib
import ctypes as c
import hashlib
import io
import json
import os
import shutil
import subprocess
import types
from unittest import mock
import struct
import sys
import tempfile
import unittest

TOOLS = Path(__file__).resolve().parents[1]
ROOT = TOOLS.parent
sys.path.insert(0, str(TOOLS / "mambo-flash"))
import mambo_flash as flash


def reply(payload, kind=0x38, origin=flash.TARGET, destination=flash.ORIGIN):
    return flash.frame(payload,destination,origin,kind)


class MockTransport:
    def __init__(self, replies):
        self.replies = list(replies)
        self.sent = []
        self.closed = False

    def send(self, data):
        self.sent.append(data)

    def receive(self, _timeout):
        if not self.replies:
            raise AssertionError("unexpected receive beyond the mock transcript")
        return self.replies.pop(0)
    def close(self):
        self.closed = True


def ask(start,end,checksum=0):
    return reply(bytes([2])+struct.pack(">III",start,end,checksum))


def report(start,end,bits=255,status=0):
    return reply(bytes([5,bits])+bytes([255])*15+bytes([status])+struct.pack(">II",start,end))


class FlashTests(unittest.TestCase):
    def test_capture_discovery_golden_crc(self):
        self.assertEqual(flash.frame(b"",0,0x0e,0x28),bytes.fromhex("c80428000e7c"))

    def test_complete_transfer_and_terminal_verification(self):
        image=bytes(range(96))
        crc=flash.legacy_crc32(image)
        # Actual bootloader's legacy DownloadFinished has a zero/invalid CRC.
        finished=bytes.fromhex("c8053810ea0600")
        transport=MockTransport([reply(bytes([7,0])),reply(bytes([7,100])),
                                 ask(0,96),report(0,96),ask(0,0,crc),finished])
        events=[]
        flash.upload(transport,image,event=lambda **kw:events.append(kw),pause=lambda _:None)
        self.assertEqual(transport.sent[0],flash.header(image))
        self.assertEqual(transport.sent[1:3],list(flash.chunks(image,0,96)))
        self.assertEqual(transport.sent[3],flash.frame(bytes([4])))
        self.assertEqual([e['action'] for e in events][-2:],
                         ['image_checksum_verified','download_finished'])

    def test_missing_chunk_is_retried_and_duplicate_request_polled(self):
        image=bytes(range(96))
        transport=MockTransport([ask(0,96),ask(0,96),report(0,96,bits=1),
                                 report(0,96),ask(0,0,flash.legacy_crc32(image)),reply(bytes([6]))])
        flash.upload(transport,image,pause=lambda _:None)
        data=[p for p in transport.sent if p[5]==3]
        self.assertEqual([p[6] for p in data],[0,1,1])
        self.assertEqual(len([p for p in transport.sent if p[5]==1]),1)

    def test_invalid_ranges_and_early_completion_stop(self):
        image=bytes(range(96))
        for incoming in (ask(48,96),ask(0,97),ask(0,6145),ask(1,96),reply(bytes([6])),
                         ask(0,0,flash.legacy_crc32(image))):
            with self.subTest(incoming=incoming.hex()):
                transport=MockTransport([incoming])
                with self.assertRaises(ValueError):
                    flash.upload(transport,image,pause=lambda _:None)
                self.assertEqual(transport.sent,[flash.header(image)])

    def test_bad_terminal_crc_is_not_success(self):
        image=bytes(range(96))
        transport=MockTransport([ask(0,96),ask(0,0,flash.legacy_crc32(image)^1)])
        with self.assertRaisesRegex(ValueError,'terminal'):
            flash.upload(transport,image,pause=lambda _:None)

    def test_report_range_status_and_retry_limit(self):
        image=bytes(range(96))
        for responses in ([report(0,48)], [report(0,96,status=1)],
                          [report(0,96,bits=0)]*4):
            transport=MockTransport([ask(0,96)]+responses)
            with self.assertRaises(ValueError):
                flash.upload(transport,image,pause=lambda _:None)
            self.assertEqual(len([p for p in transport.sent if p[5]==1]),1)

    def test_frame_validation_and_hid_padding(self):
        packet=ask(0,96)
        self.assertEqual(flash.parse_report(packet+bytes(64-len(packet))),packet)
        for raw in (b"", b"\xc8\xff\x38\x00",packet[:-1],packet[:-1]+bytes([packet[-1]^1])):
            with self.assertRaises(ValueError):
                flash.parse_report(raw)

    def test_bootloader_identity_before_erase(self):
        body=b"TBS BOOTLOADER\0"+struct.pack('>IIIBB',0x12345,flash.HARDWARE,0,0,1)
        packet=reply(body,kind=0x29,destination=0x0e)
        transport=MockTransport([packet])
        info=flash.discover(transport,0x12345)
        self.assertEqual(info['hardware_id'],flash.HARDWARE)
        self.assertEqual(transport.sent,[bytes.fromhex('c80428000e7c')])
        transport=MockTransport([packet])
        with self.assertRaisesRegex(ValueError,'identity'):
            flash.discover(transport,0x54321)
        self.assertTrue(all(p[2]==0x28 for p in transport.sent))

    def test_last_short_chunk_not_padded_into_firmware(self):
        image=bytes(range(100))
        packets=list(flash.chunks(image,0,100))
        self.assertEqual(len(packets),3)
        self.assertEqual(packets[-1][8:-1],image[96:])
        self.assertEqual(packets[-1][6:8],bytes([2,3]))

    def test_default_upload_pacing_matches_successful_full_transfer(self):
        image = bytes(range(96))
        transport = MockTransport([ask(0, 96),
            ask(0, 0, flash.legacy_crc32(image)), reply(bytes([6]))])
        delays = []
        flash.upload(transport, image, pause=delays.append)
        self.assertEqual(delays, [0.015, 0.015])

    def test_bench_header_version_and_upload_metadata(self):
        image=bytes(range(96))
        transport=MockTransport([ask(0,96),ask(0,0,flash.legacy_crc32(image)),reply(bytes([6]))])
        flash.upload(transport,image,pause=lambda _:None)
        values=struct.unpack('>BBIHII',transport.sent[0][5:-1])
        self.assertEqual(values[:4],(1,0,flash.HARDWARE,0x0300))
        self.assertEqual(values[4:],(96,flash.legacy_crc32(image)))
        with self.assertRaises(TypeError):
            flash.header(image, 0xFFFF)

    def test_checksum_vectors(self):
        self.assertEqual(flash.legacy_crc32(b"123456789"), 0xA47AA28F)
        self.assertEqual(flash.legacy_crc32(bytes(600)), 0x77EDA223)
        self.assertEqual(flash.legacy_crc32(bytes(range(256))), 0x492ED94E)

    def test_multirange_transfer_and_stale_bitmap(self):
        image = bytes(range(256)) * 25
        transport = MockTransport([ask(0, 6144), report(0, 6144),
            ask(6144, len(image)), report(0, 6144),
            ask(0, 0, flash.legacy_crc32(image)), reply(bytes([6]))])
        flash.upload(transport, image, pause=lambda _: None)
        data = [p[8:-1] for p in transport.sent if p[5] == 3]
        self.assertEqual(b"".join(data), image)
        self.assertEqual(sum(p[5] == 1 for p in transport.sent), 1)

    def test_pacing_range_and_timeouts_never_reerase(self):
        image = bytes(range(96))
        for delay in (0.009, 0.101):
            transport = MockTransport([])
            with self.assertRaises(ValueError):
                flash.upload(transport, image, chunk_delay=delay)
            self.assertEqual(transport.sent, [])
        for clock, message in (([0, 1, 46], "progress"), ([0, 601], "overall")):
            transport = MockTransport([])
            with mock.patch.object(flash.time, "monotonic", side_effect=clock):
                with self.assertRaisesRegex(TimeoutError, message):
                    flash.upload(transport, image)
            self.assertEqual(transport.sent, [flash.header(image)])
        for start, end in ((0, 95), (1, 96), (0, 6145), (0, 6401)):
            with self.assertRaises(ValueError):
                list(flash.chunks(bytes(6400), start, end))

    def test_malformed_commands_and_data_after_terminal_are_rejected(self):
        image = bytes(range(96))
        for responses in ([reply(b"\x07\x65")], [reply(b"\x02")],
                          [reply(b"\x05")], [reply(b"\x09")],
                          [report(0, 96)], [ask(0, 96)] * 5,
                          [ask(0, 96), ask(0, 0, flash.legacy_crc32(image)), ask(0, 96)]):
            with self.assertRaises(ValueError):
                flash.upload(MockTransport(responses), image, pause=lambda _: None)


def identity(serial=0x12345, hardware=flash.HARDWARE, protocol=1, name="TBS BOOTLOADER"):
    return reply(name.encode() + b"\0" + struct.pack(">IIIBB", serial, hardware, 0, 0, protocol),
                 kind=0x29, destination=0x0e)


class CliTests(unittest.TestCase):
    def setUp(self):
        self.directory = tempfile.TemporaryDirectory(prefix="mambo-flash-test-")
        self.addCleanup(self.directory.cleanup)
        self.path = Path(self.directory.name)
        self.image = bytes(range(96))
        self.binary = self.path / "firmware.bin"
        self.binary.write_bytes(self.image)
        self.elf = self.path / "firmware.elf"
        self.elf.write_bytes(b"synthetic ELF placeholder")
        self.sha = hashlib.sha256(self.image).hexdigest()
        self.args = ["--image", str(self.binary), "--elf", str(self.elf)]
        self.live = self.args + ["--execute", "--expected-serial", "0x12345",
            "--expected-sha256", self.sha, "--log-dir", str(self.path / "logs")]

    def run_main(self, args, transport=None):
        output, error = io.StringIO(), io.StringIO()
        with contextlib.redirect_stdout(output), contextlib.redirect_stderr(error), \
                mock.patch.object(flash, "NativeHid", return_value=transport) as hid:
            result = flash.main(args)
        return result, output.getvalue(), error.getvalue(), hid

    def test_offline_plan_never_opens_usb_or_creates_logs(self):
        with mock.patch.object(flash, "load_image", return_value=self.image):
            result, output, _, hid = self.run_main(self.args + ["--log-dir", str(self.path / "unused")])
        self.assertEqual(result, 0)
        self.assertEqual(json.loads(output)["sha256"], self.sha)
        self.assertFalse(json.loads(output)["usb_access"])
        hid.assert_not_called()
        self.assertFalse((self.path / "unused").exists())

    def test_probe_is_only_discovery_and_closes_transport(self):
        transport = MockTransport([identity()])
        result, output, _, _ = self.run_main(["--probe"], transport)
        self.assertEqual(result, 0)
        self.assertEqual(json.loads(output)["serial"], 0x12345)
        self.assertEqual(transport.sent, [bytes.fromhex("c80428000e7c")])
        self.assertTrue(transport.closed)

    def test_wrong_identity_is_rejected_before_erase(self):
        for packet in (identity(serial=1), identity(hardware=1), identity(protocol=2),
                       identity(name="Mambo application")):
            transport = MockTransport([packet])
            with mock.patch.object(flash, "load_image", return_value=self.image):
                args = self.live[:-1] + [str(self.path / f"logs-{len(list(self.path.iterdir()))}")]
                result, _, error, _ = self.run_main(args, transport)
            self.assertEqual(result, 1)
            self.assertIn("identity/mode", error)
            self.assertEqual(transport.sent, [bytes.fromhex("c80428000e7c")])
            self.assertTrue(transport.closed)

    def test_execute_requires_pins_and_rejects_resume_stock_and_mixed_modes(self):
        for args in (self.args + ["--execute"], ["--probe", "--execute"],
                     ["--probe", "--image", str(self.binary)], self.args + ["--resume"],
                     self.args + ["--execute-stock"], self.args + ["--chunk-delay-ms", "9"],
                     self.args + ["--expected-serial", "-1"],
                     self.args + ["--expected-serial", "0x100000000"]):
            with self.subTest(args=args), mock.patch.object(flash, "NativeHid") as hid, \
                    contextlib.redirect_stderr(io.StringIO()):
                with self.assertRaises(SystemExit):
                    flash.main(args)
                hid.assert_not_called()

    def test_wrong_hash_and_invalid_image_never_open_usb(self):
        args = list(self.live)
        args[args.index(self.sha)] = "0" * 64
        result, _, error, hid = self.run_main(args)
        self.assertEqual(result, 1)
        self.assertIn("SHA256", error)
        hid.assert_not_called()
        self.assertFalse((self.path / "logs").exists())
        for data in (b"", bytes(0xB4001)):
            self.binary.write_bytes(data)
            result, _, error, hid = self.run_main(self.live)
            self.assertEqual(result, 1)
            self.assertIn("allocation", error)
            hid.assert_not_called()

    def test_image_verification_and_change_detection(self):
        verifier = mock.Mock(return_value=(len(self.image), self.sha))
        with mock.patch.dict(sys.modules, {"verify_firmware": types.SimpleNamespace(verify=verifier)}):
            self.assertEqual(flash.load_image(self.binary, self.elf, self.sha), self.image)
            verifier.assert_called_once_with(self.elf, self.binary)
            verifier.return_value = (len(self.image), "0" * 64)
            with self.assertRaisesRegex(ValueError, "changed"):
                flash.load_image(self.binary, self.elf)
            verifier.side_effect = AssertionError("out of bounds")
            with self.assertRaisesRegex(ValueError, "ELF/bin"):
                flash.load_image(self.binary, self.elf)
        with self.assertRaisesRegex(ValueError, "64 hexadecimal"):
            flash.load_image(self.binary, self.elf, "not a hash")

    def test_completed_transfer_writes_evidence_without_reset(self):
        transport = MockTransport([identity(), ask(0, 96), report(0, 96),
            ask(0, 0, flash.legacy_crc32(self.image)), bytes.fromhex("c8053810ea0600")])
        with mock.patch.object(flash, "load_image", return_value=self.image), \
                mock.patch.object(flash, "upload", wraps=flash.upload) as upload:
            result, _, error, _ = self.run_main(self.live, transport)
        self.assertEqual((result, error), (0, ""))
        self.assertEqual(upload.call_args.kwargs["chunk_delay"], 0.015)
        plan_data = json.loads((self.path / "logs/plan.json").read_text())
        self.assertEqual(plan_data["chunk_delay_ms"], 15)
        result_data = json.loads((self.path / "logs/result.json").read_text())
        self.assertTrue(result_data["terminal_checksum_verified"])
        self.assertFalse(result_data["reset_command_sent"])
        self.assertFalse(result_data["flash_readback_performed"])
        rows = [json.loads(line) for line in (self.path / "logs/usb.jsonl").read_text().splitlines()]
        self.assertEqual(sum(row["direction"] == "OUT" for row in rows), 5)
        self.assertTrue(transport.closed)

    def test_partial_transfer_and_logging_error_warn_without_retry(self):
        for logging_failure in (False, True):
            transport = MockTransport([identity(), ask(0, 97)])
            args = self.live[:-1] + [str(self.path / f"failure-{logging_failure}")]
            original = flash.LoggedTransport.record
            def record(logged, direction, packet):
                if logging_failure and direction == "OUT" and packet[5] == 1:
                    raise OSError("no space left on device")
                return original(logged, direction, packet)
            with mock.patch.object(flash, "load_image", return_value=self.image), \
                    mock.patch.object(flash.LoggedTransport, "record", record):
                result, _, error, _ = self.run_main(args, transport)
            self.assertEqual(result, 1)
            self.assertIn("may be incomplete", error)
            self.assertEqual(sum(p[2] == 0x38 and p[5] == 1 for p in transport.sent), 1)
            self.assertFalse((Path(args[-1]) / "result.json").exists())
            self.assertTrue(transport.closed)

    def test_existing_logs_and_low_disk_fail_before_usb(self):
        (self.path / "logs").mkdir()
        with mock.patch.object(flash, "load_image", return_value=self.image):
            result, _, _, hid = self.run_main(self.live)
            self.assertEqual(result, 1)
            hid.assert_not_called()
            with mock.patch.object(flash.shutil, "disk_usage", return_value=types.SimpleNamespace(free=1)):
                result, _, error, hid = self.run_main(self.live[:-1] + [str(self.path / "low-disk")])
            self.assertEqual(result, 1)
            self.assertIn("64 MiB", error)
            hid.assert_not_called()


class NativeHidTests(unittest.TestCase):
    def library(self, count=1, descriptor=flash.REPORT_DESCRIPTOR):
        entries = []
        for i in range(count):
            entry = flash.HidInfo()
            entry.path = f"synthetic-device-{i}".encode()
            entry.interface_number = 0
            entry.usage_page, entry.usage = 0xFF00, 1
            if entries:
                entries[-1].next = c.pointer(entry)
            entries.append(entry)
        lib = types.SimpleNamespace(**{name: mock.Mock() for name in (
            "hid_init", "hid_exit", "hid_enumerate", "hid_free_enumeration",
            "hid_open_path", "hid_close", "hid_error", "hid_get_report_descriptor",
            "hid_write", "hid_read_timeout")})
        lib.hid_init.return_value = 0
        lib.hid_enumerate.return_value = c.pointer(entries[0]) if entries else None
        lib.hid_open_path.return_value = 1
        lib.hid_error.return_value = None
        def get_descriptor(_handle, buffer, _size):
            c.memmove(buffer, descriptor, len(descriptor))
            return len(descriptor)
        lib.hid_get_report_descriptor.side_effect = get_descriptor
        lib.entries = entries  # Keep the fake linked list alive.
        return lib

    def test_descriptor_enumeration_and_unnumbered_reports(self):
        lib = self.library()
        with mock.patch.object(flash.c, "CDLL", return_value=lib):
            transport = flash.NativeHid("synthetic-library")
        lib.hid_enumerate.assert_called_once_with(0x04D8, 0xF94C)
        lib.hid_free_enumeration.assert_called_once()
        packet = flash.frame(b"\x04")
        def write(_handle, buffer, size):
            self.assertEqual(buffer.raw[:size], b"\0" + packet)
            return size
        lib.hid_write.side_effect = write
        transport.send(packet)
        def read(_handle, buffer, _size, _timeout):
            c.memmove(buffer, packet, len(packet))
            return len(packet)
        lib.hid_read_timeout.side_effect = read
        self.assertEqual(transport.receive(200), packet)
        transport.close()
        transport.close()
        lib.hid_close.assert_called_once_with(1)
        lib.hid_exit.assert_called_once()

    def test_refuses_multiple_devices_and_wrong_descriptor(self):
        for count, descriptor in ((0, flash.REPORT_DESCRIPTOR),
                                  (2, flash.REPORT_DESCRIPTOR), (1, b"different")):
            lib = self.library(count, descriptor)
            with mock.patch.object(flash.c, "CDLL", return_value=lib):
                with self.assertRaises(OSError):
                    flash.NativeHid("synthetic-library")
            lib.hid_exit.assert_called_once()
            lib.hid_free_enumeration.assert_called_once()
            if count != 1:
                lib.hid_open_path.assert_not_called()
            else:
                lib.hid_close.assert_called_once()

    def test_missing_api_and_initialization_fail_before_open(self):
        lib = self.library()
        del lib.hid_get_report_descriptor
        with mock.patch.object(flash.c, "CDLL", return_value=lib):
            with self.assertRaisesRegex(OSError, "0.14"):
                flash.NativeHid("synthetic-library")
        lib.hid_init.assert_not_called()
        lib = self.library()
        lib.hid_init.return_value = -1
        with mock.patch.object(flash.c, "CDLL", return_value=lib):
            with self.assertRaisesRegex(OSError, "initialization"):
                flash.NativeHid("synthetic-library")
        lib.hid_enumerate.assert_not_called()
        lib.hid_exit.assert_called_once()

    def test_short_write_and_failed_read_are_errors(self):
        lib = self.library()
        with mock.patch.object(flash.c, "CDLL", return_value=lib):
            transport = flash.NativeHid("synthetic-library")
        lib.hid_write.return_value = 1
        with self.assertRaisesRegex(OSError, "short HID write"):
            transport.send(flash.frame(b"\x04"))
        lib.hid_read_timeout.return_value = -1
        with self.assertRaisesRegex(OSError, "HID read failed"):
            transport.receive(200)
        lib.hid_read_timeout.return_value = 0
        self.assertIsNone(transport.receive(200))
        transport.close()


class CMakeTests(unittest.TestCase):
    @unittest.skipUnless(shutil.which("cmake"), "CMake unavailable")
    def test_runner_modes_guards_paths_and_error_propagation(self):
        with tempfile.TemporaryDirectory(prefix="mambo cmake ") as directory:
            path = Path(directory)
            script = path / "stub.py"
            script.write_text("import json,sys; print(json.dumps(sys.argv[1:]))\n")
            common = [shutil.which("cmake"), f"-DMAMBO_FLASH_PYTHON={sys.executable}",
                f"-DMAMBO_FLASH_SCRIPT={script}", f"-DMAMBO_FLASH_IMAGE={path / 'firmware.bin'}",
                f"-DMAMBO_FLASH_ELF={path / 'firmware.elf'}", f"-DMAMBO_FLASH_LOG_ROOT={path / 'logs'}"]
            env = dict(os.environ)
            for name in ("MAMBO_FLASH_SERIAL", "MAMBO_FLASH_SHA256", "MAMBO_HID_LIBRARY"):
                env.pop(name, None)
            def run(mode):
                return subprocess.run(common + [f"-DMAMBO_FLASH_MODE={mode}", "-P",
                    str(ROOT / "cmake/RunMamboFlash.cmake")], capture_output=True, text=True, env=env)
            result = run("PLAN")
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(json.loads(result.stdout), ["--image", str(path / "firmware.bin"),
                "--elf", str(path / "firmware.elf")])
            result = run("PROBE")
            self.assertEqual(json.loads(result.stdout), ["--probe"])
            result = run("FLASH")
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("MAMBO_FLASH_SERIAL", result.stderr)
            self.assertEqual(result.stdout, "")
            env["MAMBO_FLASH_SERIAL"] = "0x12345"
            result = run("FLASH")
            self.assertNotEqual(result.returncode, 0)
            self.assertIn("MAMBO_FLASH_SHA256", result.stderr)
            env["MAMBO_FLASH_SHA256"] = "a" * 64
            result = run("FLASH")
            self.assertEqual(result.returncode, 0, result.stderr)
            args = json.loads(result.stdout)
            self.assertIn("--execute", args)
            self.assertEqual(args[args.index("--expected-serial") + 1], "0x12345")
            self.assertEqual(args[args.index("--expected-sha256") + 1], "a" * 64)
            self.assertEqual(json.loads(run("PLAN").stdout)[:1], ["--image"])
            self.assertNotEqual(run("UNKNOWN").returncode, 0)
            script.write_text("import sys; sys.exit(7)\n")
            self.assertNotEqual(run("FLASH").returncode, 0)


if __name__ == "__main__":
    unittest.main()
