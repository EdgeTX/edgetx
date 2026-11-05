# SPDX-License-Identifier: GPL-2.0-only
"""Compare the ARM-portable C++ backend with independent double-precision math.

The reference solves a full eight-variable homography, rather than copying
the C++ closed-form map construction. All inputs are synthetic.
"""
import math
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
import zlib

from calibration_fixtures import fixture, crc16, legacy_crc32, reference_parameters

TARGETS = [(0,0), (10000,0), (10000,10000), (0,10000), (-10000,10000),
           (-10000,0), (-10000,-10000), (0,-10000), (10000,-10000)]
checks = 0


def check(condition, message):
    global checks
    if not condition:
        raise AssertionError(message)
    checks += 1


def solve(rows):
    n = len(rows)
    for column in range(n):
        pivot = max(range(column, n), key=lambda i: abs(rows[i][column]))
        rows[pivot], rows[column] = rows[column], rows[pivot]
        divisor = rows[column][column]
        if abs(divisor) < 1e-12:
            raise ValueError("singular reference homography")
        rows[column] = [value / divisor for value in rows[column]]
        for i in range(n):
            if i != column:
                multiple = rows[i][column]
                rows[i] = [a - multiple*b for a, b in zip(rows[i], rows[column])]
    return [row[-1] for row in rows]


def map_for(points, quadrant):
    indices = [(0,1,2,3), (0,5,4,3), (0,5,6,7), (0,1,8,7)][quadrant]
    rows = []
    for index, (u, v) in zip(indices, [(0,0), (1,0), (1,1), (0,1)]):
        x, y = points[index]
        sx = -1 if quadrant in (1,2) else 1
        sy = -1 if quadrant in (2,3) else 1
        x = (x - points[0][0]) * sx
        y = (y - points[0][1]) * sy
        rows.extend([[x,y,1,0,0,0,-u*x,-u*y,u],
                     [0,0,0,x,y,1,-v*x,-v*y,v]])
    return solve(rows)


def plane_reference(points, x, y):
    x -= points[0][0]
    y -= points[0][1]
    def cross(index):
        bx, by = points[index]
        return y * (bx - points[0][0]) - x * (by - points[0][1])
    if cross(1) >= 0:
        q = 0 if cross(3) <= 0 else (1 if cross(5) <= 0 else 2)
    else:
        q = 3 if cross(7) >= 0 else (2 if cross(5) >= 0 else 1)
    sx = -1 if q in (1,2) else 1
    sy = -1 if q in (2,3) else 1
    m = map_for(points, q)
    x *= sx
    y *= sy
    denominator = m[6]*x + m[7]*y + 1
    return (10000*sx*(m[0]*x + m[1]*y + m[2])/denominator,
            10000*sy*(m[3]*x + m[4]*y + m[5])/denominator)


def raw_reference(sensor, px, py):
    bx, by, bz = math.trunc(px*1.573), math.trunc(py*1.573), math.trunc(4000*2.534)
    plane = []
    for axis, primary, secondary in zip(sensor["axes"], (bx,by), (by,bx)):
        denominator = (secondary - axis["denominator_cross_z"]*bz) * axis["ratio"]
        numerator = primary - axis["numerator_cross_z"]*bz
        angle = math.degrees(math.atan2(math.sqrt(bz*bz + denominator*denominator), numerator))
        delta = angle - axis["centre_degrees"]
        gain = axis["positive_gain"] if delta > 0 else axis["negative_gain"]
        plane.append(math.trunc(delta * gain * 100))
    corrected = plane_reference(sensor["corner_points"], *plane)
    return [math.floor(2047.5 + math.trunc(max(-10000,min(10000,v)))*4095/20000 + 0.5)
            for v in corrected]


def verify(probe, path):
    report = reference_parameters()
    result = subprocess.run([str(probe), str(path)], capture_output=True, text=True)
    check(result.returncode == 0, f"probe rejected valid fixture: {result.returncode}, {result.stdout}")
    points, samples = 0, 0
    for line in result.stdout.splitlines():
        fields = line.split()
        sensor = report["sensors"][int(fields[1])]
        if fields[0] == "point":
            target = TARGETS[int(fields[2])]
            values = [float(v) for v in fields[3:5]]
            check(all(abs(v-t) <= 2 for v,t in zip(values,target)), f"point mismatch: {line}")
            points += 1
        else:
            target = raw_reference(sensor, int(fields[2]), int(fields[3]))
            values = [int(v) for v in fields[4:6]]
            # Float/FMA/truncation differences can shift a few stock units.
            check(all(abs(v-t) <= 3 for v,t in zip(values,target)), f"sample mismatch: {line}, {target}")
            samples += 1
    check(points == 18 and samples == 50, "probe output incomplete")


def mutate(data, index, value, fix_inner=False):
    words = list(struct.unpack("<300H", data[:600]))
    words[index] = value
    if fix_inner:
        for base in (0x78, 0x83):
            words[base + 10] = crc16(struct.pack(">10H", *words[base:base + 10]))
        for base in (0xA0, 0xB4):
            checksum = zlib.crc32(struct.pack("<18H", *words[base:base + 18])) ^ 0xFFFFFFFF
            words[base + 18], words[base + 19] = checksum >> 16, checksum & 0xFFFF
    body = struct.pack("<300H", *words)
    return body + struct.pack("<I", zlib.crc32(body))


def main():
    check(crc16(b"123456789") == 0x6F91, "angle CRC check vector")
    check(legacy_crc32(b"123456789") == 0xA47AA28F, "legacy CRC check vector")
    probe = Path(sys.argv[1]).resolve()
    with tempfile.TemporaryDirectory(prefix="mambo-calibration-") as directory:
        path = Path(directory)/"fixture.bin"
        for legacy in (False,True):
            path.write_bytes(fixture(legacy_backup=legacy))
            verify(probe, path)
        damaged = bytearray(fixture())
        damaged[-1] ^= 1
        invalid = [bytes(603), bytes(damaged),
                   fixture(additive_angles=True),
                   mutate(fixture(),0x78,1201), # invalid angle CRC
                   mutate(fixture(),0xB4,1), # invalid right corner CRC
                   mutate(fixture(),0x78,0,True), # zero ratio
                   mutate(fixture(),0x80,18001,True), # invalid centre
                   mutate(fixture(),0xA2,0,True)] # degenerate first quadrant
        for data in invalid:
            path.write_bytes(data)
            result = subprocess.run([str(probe),str(path)], capture_output=True, text=True)
            check(result.returncode == 1 and result.stdout.strip() == "rejected",
                  f"bad fixture was accepted: {result.returncode}")
    print(f"Factory calibration: {checks} independent checks passed")


if __name__ == "__main__":
    main()
