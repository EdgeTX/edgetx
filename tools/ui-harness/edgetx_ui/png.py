import struct
import zlib
from pathlib import Path


def _chunk(kind: bytes, data: bytes) -> bytes:
    return (
        struct.pack(">I", len(data))
        + kind
        + data
        + struct.pack(">I", zlib.crc32(kind + data) & 0xFFFFFFFF)
    )


def write_png_rgb(path: Path, width: int, height: int, rgb: bytes) -> None:
    if len(rgb) != width * height * 3:
        raise ValueError("RGB buffer size does not match image dimensions")

    rows = []
    stride = width * 3
    for y in range(height):
        start = y * stride
        rows.append(b"\x00" + rgb[start : start + stride])

    data = b"".join(rows)
    payload = [
        b"\x89PNG\r\n\x1a\n",
        _chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0)),
        _chunk(b"IDAT", zlib.compress(data, level=6)),
        _chunk(b"IEND", b""),
    ]
    path.write_bytes(b"".join(payload))


def convert_ppm_to_png(ppm_path: Path, png_path: Path) -> tuple[int, int]:
    data = ppm_path.read_bytes()
    if not data.startswith(b"P6\n"):
        raise ValueError(f"{ppm_path} is not a binary PPM file")

    cursor = 3

    def read_token() -> bytes:
        nonlocal cursor
        while cursor < len(data) and data[cursor] in b" \t\r\n":
            cursor += 1
        start = cursor
        while cursor < len(data) and data[cursor] not in b" \t\r\n":
            cursor += 1
        return data[start:cursor]

    width = int(read_token())
    height = int(read_token())
    max_value = int(read_token())
    if max_value != 255:
        raise ValueError(f"unsupported PPM max value: {max_value}")
    if cursor < len(data) and data[cursor] in b" \t\r\n":
        cursor += 1

    rgb = data[cursor:]
    write_png_rgb(png_path, width, height, rgb)
    return width, height
