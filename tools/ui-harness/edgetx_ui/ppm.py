"""Strict PPM handling and dependency-free deterministic PNG output."""

from __future__ import annotations

import hashlib
import json
import os
import struct
import zlib
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Dict, Tuple

PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"


@dataclass(frozen=True)
class RgbImage:
    width: int
    height: int
    rgb: bytes


@dataclass(frozen=True)
class ArtifactDigest:
    path: Path
    byte_count: int
    sha256: str


def read_ppm(path: Path) -> RgbImage:
    """Read the exact one-image P6 subset emitted by the simulator."""

    data = path.read_bytes()
    if not data.startswith(b"P6\n"):
        raise ValueError("PPM must start with the canonical P6 header")
    third_newline = _nth_index(data, b"\n", 3)
    if third_newline < 0:
        raise ValueError("PPM header is incomplete")
    header = data[: third_newline + 1]
    lines = header.split(b"\n")
    if len(lines) != 4 or lines[0] != b"P6" or lines[2] != b"255" or lines[3]:
        raise ValueError("PPM header is not the canonical max-value-255 form")
    dimensions = lines[1].split(b" ")
    if len(dimensions) != 2 or not all(
        value.isdigit() and str(int(value, 10)).encode("ascii") == value
        for value in dimensions
    ):
        raise ValueError("PPM dimensions are invalid")
    width, height = (int(value, 10) for value in dimensions)
    if width <= 0 or height <= 0 or width > 65535 or height > 65535:
        raise ValueError("PPM dimensions are outside the supported range")
    rgb = data[len(header) :]
    if len(rgb) != width * height * 3:
        raise ValueError("PPM raster length does not match its dimensions")
    return RgbImage(width=width, height=height, rgb=rgb)


def write_png(path: Path, image: RgbImage) -> ArtifactDigest:
    """Write a minimal non-interlaced RGB PNG without replacing a file."""

    _validate_image(image)
    scanlines = bytearray()
    stride = image.width * 3
    for start in range(0, len(image.rgb), stride):
        scanlines.append(0)
        scanlines.extend(image.rgb[start : start + stride])
    payload = b"".join(
        (
            PNG_SIGNATURE,
            _png_chunk(
                b"IHDR",
                struct.pack(">IIBBBBB", image.width, image.height, 8, 2, 0, 0, 0),
            ),
            _png_chunk(b"IDAT", zlib.compress(bytes(scanlines), level=9)),
            _png_chunk(b"IEND", b""),
        )
    )
    _write_new_bytes(path, payload)
    try:
        return digest_file(path)
    except BaseException:
        _remove_owned_file(path)
        raise


def read_png(path: Path) -> RgbImage:
    """Decode and CRC-check the narrow PNG subset produced by write_png."""

    data = path.read_bytes()
    if not data.startswith(PNG_SIGNATURE):
        raise ValueError("PNG signature is invalid")
    cursor = len(PNG_SIGNATURE)
    chunks = []
    while cursor < len(data):
        if len(data) - cursor < 12:
            raise ValueError("PNG chunk is truncated")
        length = struct.unpack(">I", data[cursor : cursor + 4])[0]
        kind = data[cursor + 4 : cursor + 8]
        end = cursor + 12 + length
        if end > len(data):
            raise ValueError("PNG chunk length exceeds the file")
        payload = data[cursor + 8 : cursor + 8 + length]
        expected_crc = struct.unpack(">I", data[cursor + 8 + length : end])[0]
        if (zlib.crc32(kind + payload) & 0xFFFFFFFF) != expected_crc:
            raise ValueError("PNG chunk CRC is invalid")
        chunks.append((kind, payload))
        cursor = end
        if kind == b"IEND":
            break
    if cursor != len(data):
        raise ValueError("PNG contains trailing data")
    if [kind for kind, _ in chunks] != [b"IHDR", b"IDAT", b"IEND"] or chunks[-1][1]:
        raise ValueError("PNG critical chunk order is invalid")

    header = chunks[0][1]
    if len(header) != 13:
        raise ValueError("PNG IHDR length is invalid")
    width, height, depth, color, compression, filtering, interlace = struct.unpack(
        ">IIBBBBB", header
    )
    if (
        width == 0
        or height == 0
        or width > 65535
        or height > 65535
        or (depth, color, compression, filtering, interlace) != (8, 2, 0, 0, 0)
    ):
        raise ValueError("PNG IHDR is outside the supported RGB subset")
    idat = chunks[1][1]
    if not idat:
        raise ValueError("PNG has no IDAT payload")
    try:
        decompressor = zlib.decompressobj()
        filtered = decompressor.decompress(idat) + decompressor.flush()
    except zlib.error as error:
        raise ValueError("PNG zlib stream is invalid") from error
    if not decompressor.eof or decompressor.unused_data:
        raise ValueError("PNG zlib stream has trailing or incomplete data")
    stride = width * 3
    if len(filtered) != height * (stride + 1):
        raise ValueError("PNG scanline length does not match its dimensions")
    rgb = bytearray()
    for y in range(height):
        start = y * (stride + 1)
        if filtered[start] != 0:
            raise ValueError("PNG uses an unsupported scanline filter")
        rgb.extend(filtered[start + 1 : start + stride + 1])
    return RgbImage(width=width, height=height, rgb=bytes(rgb))


def convert_ppm_to_png(
    ppm_path: Path, png_path: Path
) -> Tuple[RgbImage, ArtifactDigest]:
    image = read_ppm(ppm_path)
    png = write_png(png_path, image)
    try:
        decoded = read_png(png_path)
        if decoded != image:
            raise ValueError("PNG decode does not match the PPM raster")
    except BaseException:
        _remove_owned_file(png_path)
        raise
    return image, png


def digest_file(path: Path) -> ArtifactDigest:
    digest = hashlib.sha256()
    byte_count = 0
    with path.open("rb") as stream:
        while True:
            chunk = stream.read(64 * 1024)
            if not chunk:
                break
            byte_count += len(chunk)
            digest.update(chunk)
    return ArtifactDigest(path=path, byte_count=byte_count, sha256=digest.hexdigest())


def write_json_sidecar(path: Path, payload: Dict[str, Any]) -> ArtifactDigest:
    encoded = (
        json.dumps(payload, ensure_ascii=False, sort_keys=True, separators=(",", ":"))
        + "\n"
    ).encode("utf-8")
    _write_new_bytes(path, encoded)
    try:
        return digest_file(path)
    except BaseException:
        _remove_owned_file(path)
        raise


def _validate_image(image: RgbImage) -> None:
    if image.width <= 0 or image.height <= 0:
        raise ValueError("image dimensions must be positive")
    if image.width > 65535 or image.height > 65535:
        raise ValueError("image dimensions exceed the supported range")
    if len(image.rgb) != image.width * image.height * 3:
        raise ValueError("RGB raster length does not match its dimensions")


def _png_chunk(kind: bytes, payload: bytes) -> bytes:
    if len(kind) != 4:
        raise ValueError("PNG chunk names must be four bytes")
    return (
        struct.pack(">I", len(payload))
        + kind
        + payload
        + struct.pack(">I", zlib.crc32(kind + payload) & 0xFFFFFFFF)
    )


def _write_new_bytes(path: Path, payload: bytes) -> None:
    if not path.parent.is_dir():
        raise ValueError("artifact parent directory does not exist")
    temporary = path.with_name("." + path.name + ".tmp-ui-harness")
    created = False
    try:
        with temporary.open("xb") as stream:
            created = True
            stream.write(payload)
            stream.flush()
            os.fsync(stream.fileno())
        if os.name == "nt":
            os.rename(temporary, path)
            created = False
        else:
            os.link(temporary, path)
            try:
                temporary.unlink()
            except OSError:
                try:
                    path.unlink()
                finally:
                    raise
            created = False
    finally:
        if created:
            try:
                temporary.unlink()
            except FileNotFoundError:
                pass


def _remove_owned_file(path: Path) -> None:
    try:
        path.unlink()
    except FileNotFoundError:
        pass


def _nth_index(data: bytes, token: bytes, count: int) -> int:
    position = -1
    for _ in range(count):
        position = data.find(token, position + 1)
        if position < 0:
            return -1
    return position
