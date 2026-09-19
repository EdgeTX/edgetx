#!/usr/bin/env python3
"""Drive an EdgeTX radio's trainer channels with CRSF frames over USB-VCP.

Requires the radio to be set up as:

  SYS -> Radio Setup  -> USB Mode      = Serial
  SYS -> Hardware     -> Serial Port   -> USB-VCP = CRSF Trainer
  MDL -> Setup        -> Trainer Mode  = CRSF

A Trainer special function also has to be active for the channels to reach the
mixer (Function: Trainer, Value: Axis).

Watch the result in Quick Menu -> Tools -> Channel Monitor, or Model -> Outputs.

Do this with props off and RF disabled.

Examples:
  ./crsf_vcp_test.py --port /dev/ttyACM0 --sweep
  ./crsf_vcp_test.py --port /dev/ttyACM0 --constant --stop-after 200
  ./crsf_vcp_test.py --port /dev/ttyACM0 --sweep --garbage
"""

import argparse
import glob
import random
import sys
import time

try:
    import serial
except ImportError:
    sys.exit("pyserial is required: pip install pyserial")

UART_SYNC = 0xC8
CHANNELS_ID = 0x16

NUM_CHANNELS = 16
CH_BITS = 11

# Raw 11-bit channel values. EdgeTX decodes these as (raw - 0x3E0) * 5 / 8,
# so these give the trainer input its full -512..+512 range.
CRSF_MIN = 172
CRSF_CENTER = 992
CRSF_MAX = 1811

# CRC8 with polynomial 0xD5, as used by CRSF
_CRC8_POLY = 0xD5
_CRC8_TAB = []
for _i in range(256):
    _c = _i
    for _ in range(8):
        _c = ((_c << 1) ^ _CRC8_POLY) & 0xFF if _c & 0x80 else (_c << 1) & 0xFF
    _CRC8_TAB.append(_c)


def crc8(data):
    crc = 0
    for b in data:
        crc = _CRC8_TAB[crc ^ b]
    return crc


def encode_channels_frame(channels):
    """Build a CRSF RC channels packed frame: [addr][len][type][22 bytes][crc]."""
    if len(channels) != NUM_CHANNELS:
        raise ValueError(f"expected {NUM_CHANNELS} channels, got {len(channels)}")

    payload = bytearray()
    bits = 0
    bits_available = 0
    for value in channels:
        bits |= (int(value) & 0x7FF) << bits_available
        bits_available += CH_BITS
        while bits_available >= 8:
            payload.append(bits & 0xFF)
            bits >>= 8
            bits_available -= 8

    assert len(payload) == 22, len(payload)

    frame = bytearray([UART_SYNC, len(payload) + 2, CHANNELS_ID])
    frame += payload
    frame.append(crc8(frame[2:]))
    return bytes(frame)


def triangle(phase):
    return 2 * phase if phase < 0.5 else 2 * (1.0 - phase)


def sweep_channels(elapsed, period):
    """Slow triangle on ch1-4, staggered so channel order is visible."""
    channels = [CRSF_CENTER] * NUM_CHANNELS
    for ch in range(4):
        phase = ((elapsed / period) + ch * 0.25) % 1.0
        channels[ch] = int(CRSF_MIN + (CRSF_MAX - CRSF_MIN) * triangle(phase))
    return channels


def autodetect_port():
    candidates = sorted(glob.glob("/dev/ttyACM*") + glob.glob("/dev/ttyUSB*"))
    if not candidates:
        sys.exit("no /dev/ttyACM* or /dev/ttyUSB* found; pass --port explicitly")
    if len(candidates) > 1:
        print(f"multiple ports found {candidates}, using {candidates[0]}", file=sys.stderr)
    return candidates[0]


def main():
    parser = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter
    )
    parser.add_argument("--port", help="CDC device (default: autodetect)")
    parser.add_argument("--rate", type=float, default=100.0,
                        help="frames per second (default: 100)")
    parser.add_argument("--period", type=float, default=4.0,
                        help="--sweep period in seconds (default: 4)")
    parser.add_argument("--stop-after", type=int, metavar="N",
                        help="send N frames then stop, holding the port open. "
                             "The radio must fall back to the sticks within 1s.")
    parser.add_argument("--garbage", action="store_true",
                        help="inject random bytes mid-stream to test re-sync")
    parser.add_argument("--garbage-every", type=int, default=50, metavar="N",
                        help="inject garbage every N frames (default: 50)")

    mode = parser.add_mutually_exclusive_group()
    mode.add_argument("--sweep", action="store_true",
                      help="triangle wave on ch1-4 (default)")
    mode.add_argument("--constant", action="store_true",
                      help="hold all channels centered")

    args = parser.parse_args()

    port = args.port or autodetect_port()
    interval = 1.0 / args.rate

    # Baud rate is not carried over USB CDC and is ignored by the radio.
    with serial.Serial(port, baudrate=400000, timeout=0) as ser:
        print(f"{port}: sending CRSF at {args.rate:g} Hz, ctrl-c to stop")

        start = time.monotonic()
        next_send = start
        count = 0

        try:
            while True:
                now = time.monotonic()

                if args.stop_after is not None and count >= args.stop_after:
                    print(f"sent {count} frames, stopping. Port stays open; "
                          f"the radio should fall back to the sticks within 1s.")
                    while True:
                        time.sleep(1)

                if args.constant:
                    channels = [CRSF_CENTER] * NUM_CHANNELS
                else:
                    channels = sweep_channels(now - start, args.period)

                if args.garbage and count and count % args.garbage_every == 0:
                    noise = bytes(random.randrange(256) for _ in range(random.randrange(1, 12)))
                    ser.write(noise)
                    print(f"frame {count}: injected {len(noise)} garbage bytes")

                ser.write(encode_channels_frame(channels))
                count += 1

                next_send += interval
                delay = next_send - time.monotonic()
                if delay > 0:
                    time.sleep(delay)
                else:
                    next_send = time.monotonic()
        except KeyboardInterrupt:
            print(f"\nstopped after {count} frames")


if __name__ == "__main__":
    main()
