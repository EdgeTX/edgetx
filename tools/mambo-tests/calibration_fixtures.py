# SPDX-License-Identifier: GPL-2.0-only
"""Synthetic Mambo factory-calibration records; never use them on a radio."""
import struct
import zlib


def crc16(data):
    value = 0xFFFF
    for byte in data:
        value ^= byte
        for _ in range(8):
            value = (value >> 1) ^ (0x8408 if value & 1 else 0)
    return value


def legacy_crc32(data):
    # This historical file checksum uses OR, not the standard CRC XOR step.
    value = 0xFFFFFFFF
    for byte in data:
        reflected = int(f"{byte:08b}"[::-1], 2)
        for _ in range(8):
            condition = (value & 0x80000000) or (reflected & 0x80)
            value = ((value << 1) ^ (0x04C11DB7 if condition else 0)) & 0xFFFFFFFF
            reflected <<= 1
    return int(f"{value:032b}"[::-1], 2) ^ 0xFFFFFFFF


POINTS = [(0, 0), (10000, 0), (10000, 10000), (0, 10000),
          (-10000, 10000), (-10000, 0), (-10000, -10000),
          (0, -10000), (10000, -10000)]


def fixture(legacy_backup=False, additive_angles=False):
    words = [0] * 300
    for sensor in range(2):
        base = 0x78 + 11 * sensor
        values = [1200, 0xE040, 2500, 3000, 900, 0x2080,
                  2000, 2200, 8900, 9100]
        words[base:base + 10] = values
        words[base + 10] = (sum(values) & 0xFFFF) if additive_angles else \
            crc16(struct.pack(">10H", *values))
        corner = 0xA0 + 20 * sensor
        raw = struct.pack("<18h", *(v for point in POINTS for v in point))
        words[corner:corner + 18] = struct.unpack("<18H", raw)
        checksum = zlib.crc32(raw) ^ 0xFFFFFFFF
        words[corner + 18], words[corner + 19] = checksum >> 16, checksum & 0xFFFF
    body = struct.pack("<300H", *words)
    checksum = legacy_crc32(body) if legacy_backup else zlib.crc32(body)
    return body + struct.pack("<I", checksum)


def reference_parameters():
    # Independent expected values, not decoded using the implementation under test.
    axes = [
        {"ratio": 1.2, "denominator_cross_z": -0.125,
         "numerator_cross_z": 0.25, "positive_gain": 2.5,
         "negative_gain": 3.0, "centre_degrees": 89.0},
        {"ratio": 0.9, "denominator_cross_z": 0.125,
         "numerator_cross_z": -0.5, "positive_gain": 2.0,
         "negative_gain": 2.2, "centre_degrees": 91.0},
    ]
    return {"sensors": [{"axes": axes, "corner_points": POINTS} for _ in range(2)]}
