from __future__ import annotations

import hashlib
import json
import sys
import tempfile
import unittest
from pathlib import Path

HARNESS_ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(HARNESS_ROOT))

from edgetx_ui.ppm import (  # noqa: E402
    PNG_SIGNATURE,
    RgbImage,
    convert_ppm_to_png,
    digest_file,
    read_png,
    read_ppm,
    write_json_sidecar,
    write_png,
)


class CaptureImageFormatTests(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary_directory = tempfile.TemporaryDirectory()
        self.addCleanup(self.temporary_directory.cleanup)
        self.root = Path(self.temporary_directory.name)

    def write(self, name: str, payload: bytes) -> Path:
        path = self.root / name
        path.write_bytes(payload)
        return path

    def test_reads_only_the_canonical_ppm_subset(self) -> None:
        payload = b"P6\n2 1\n255\n" + bytes((0, 1, 2, 253, 254, 255))
        image = read_ppm(self.write("valid.ppm", payload))

        self.assertEqual((image.width, image.height), (2, 1))
        self.assertEqual(image.rgb, payload[-6:])

        invalid_payloads = (
            b"P3\n2 1\n255\n" + payload[-6:],
            b"P6\r\n2 1\r\n255\r\n" + payload[-6:],
            b"P6\n02 1\n255\n" + payload[-6:],
            b"P6\n2 1\n254\n" + payload[-6:],
            b"P6\n2 1\n255\n" + payload[-5:],
            payload + b"trailing",
        )
        for index, invalid in enumerate(invalid_payloads):
            with self.subTest(index=index):
                with self.assertRaises(ValueError):
                    read_ppm(self.write(f"invalid-{index}.ppm", invalid))

    def test_png_is_deterministic_and_independently_decoded(self) -> None:
        image = RgbImage(
            width=2,
            height=2,
            rgb=bytes(
                (
                    0,
                    0,
                    0,
                    255,
                    255,
                    255,
                    255,
                    0,
                    0,
                    0,
                    255,
                    0,
                )
            ),
        )
        first = self.root / "first.png"
        second = self.root / "second.png"

        first_digest = write_png(first, image)
        second_digest = write_png(second, image)

        self.assertTrue(first.read_bytes().startswith(PNG_SIGNATURE))
        self.assertEqual(first.read_bytes(), second.read_bytes())
        self.assertEqual(first_digest.sha256, second_digest.sha256)
        self.assertEqual(read_png(first), image)

        corrupted = bytearray(first.read_bytes())
        corrupted[16] ^= 0x01
        with self.assertRaisesRegex(ValueError, "CRC"):
            read_png(self.write("corrupted.png", bytes(corrupted)))

    def test_ppm_to_png_round_trip_preserves_every_rgb_byte(self) -> None:
        ppm = self.write(
            "source.ppm",
            b"P6\n3 1\n255\n" + bytes((1, 2, 3, 4, 5, 6, 7, 8, 9)),
        )
        png = self.root / "converted.png"

        image, digest = convert_ppm_to_png(ppm, png)

        self.assertEqual(read_png(png), image)
        self.assertEqual(digest, digest_file(png))

    def test_artifact_writers_never_replace_an_existing_name(self) -> None:
        final = self.write("existing.png", b"keep-me")
        image = RgbImage(width=1, height=1, rgb=b"\x01\x02\x03")

        with self.assertRaises(FileExistsError):
            write_png(final, image)

        self.assertEqual(final.read_bytes(), b"keep-me")
        self.assertFalse((self.root / ".existing.png.tmp-ui-harness").exists())

    def test_sidecar_is_utf8_sorted_stable_and_hashed(self) -> None:
        payload = {
            "target": "écran",
            "schema_version": 1,
            "artifacts": {"ppm": {"sha256": "abc", "bytes": 7}},
        }
        first = self.root / "first.capture.json"
        second = self.root / "second.capture.json"

        first_digest = write_json_sidecar(first, payload)
        second_digest = write_json_sidecar(second, payload)

        expected = (
            json.dumps(
                payload,
                ensure_ascii=False,
                sort_keys=True,
                separators=(",", ":"),
            )
            + "\n"
        ).encode("utf-8")
        self.assertEqual(first.read_bytes(), expected)
        self.assertEqual(first.read_bytes(), second.read_bytes())
        self.assertEqual(first_digest.sha256, hashlib.sha256(expected).hexdigest())
        self.assertEqual(first_digest.sha256, second_digest.sha256)


if __name__ == "__main__":
    unittest.main()
