#!/usr/bin/env python3
"""Drive an EdgeTX radio's trainer channels with SBUS frames over USB-VCP.

Requires the radio to be set up as:

  SYS -> Radio Setup  -> USB Mode      = Serial
  SYS -> Hardware     -> Serial Port   -> USB-VCP = SBUS Trainer
  MDL -> Setup        -> Trainer Mode  = Master/Serial

Watch the result in MDL -> Channels.

Do this with props off and RF disabled.

Examples:
  ./sbus_vcp_test.py --port /dev/ttyACM0 --sweep
  ./sbus_vcp_test.py --port /dev/ttyACM0 --sweep --rate 150
  ./sbus_vcp_test.py --port /dev/ttyACM0 --constant --stop-after 200
  ./sbus_vcp_test.py --port /dev/ttyACM0 --sweep --garbage
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

SBUS_FRAME_SIZE = 25
SBUS_START_BYTE = 0x0F
SBUS_END_BYTE = 0x00

# Raw 11 bit channel values. EdgeTX decodes these as (raw - 992) * 5 / 8, so
# these are the values that give the trainer input full -512..+512 travel.
SBUS_MIN = 172
SBUS_CENTER = 992
SBUS_MAX = 1811

NUM_CHANNELS = 16


def encode_frame(channels, flags=0x00, end_byte=SBUS_END_BYTE):
    """Pack 16 channels x 11 bits into a standard 25 byte SBUS frame."""
    if len(channels) != NUM_CHANNELS:
        raise ValueError(f"expected {NUM_CHANNELS} channels, got {len(channels)}")

    frame = bytearray([SBUS_START_BYTE])

    bits = 0
    bits_available = 0
    for value in channels:
        bits |= (int(value) & 0x7FF) << bits_available
        bits_available += 11
        while bits_available >= 8:
            frame.append(bits & 0xFF)
            bits >>= 8
            bits_available -= 8

    frame.append(flags)
    frame.append(end_byte)

    assert len(frame) == SBUS_FRAME_SIZE, len(frame)
    return bytes(frame)


def triangle(phase):
    """Triangle wave in 0.0 .. 1.0, phase in 0.0 .. 1.0."""
    return 2 * phase if phase < 0.5 else 2 * (1.0 - phase)


def sweep_channels(elapsed, period):
    """Slow triangle on ch1-4, staggered so channel order is visible."""
    channels = [SBUS_CENTER] * NUM_CHANNELS
    for ch in range(4):
        phase = ((elapsed / period) + ch * 0.25) % 1.0
        channels[ch] = int(SBUS_MIN + (SBUS_MAX - SBUS_MIN) * triangle(phase))
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

    # Baud/parity are meaningless over USB CDC and are ignored by the radio.
    # They are set to the nominal SBUS values only so the host driver is happy.
    with serial.Serial(port, baudrate=100000, parity=serial.PARITY_EVEN,
                       stopbits=serial.STOPBITS_TWO, timeout=0) as ser:
        print(f"{port}: sending SBUS at {args.rate:g} Hz, ctrl-c to stop")

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
                    channels = [SBUS_CENTER] * NUM_CHANNELS
                else:
                    channels = sweep_channels(now - start, args.period)

                if args.garbage and count and count % args.garbage_every == 0:
                    noise = bytes(random.randrange(256) for _ in range(random.randrange(1, 12)))
                    ser.write(noise)
                    print(f"frame {count}: injected {len(noise)} garbage bytes")

                ser.write(encode_frame(channels))
                count += 1

                next_send += interval
                delay = next_send - time.monotonic()
                if delay > 0:
                    time.sleep(delay)
                else:
                    # fell behind; resynchronise the schedule
                    next_send = time.monotonic()
        except KeyboardInterrupt:
            print(f"\nstopped after {count} frames")


if __name__ == "__main__":
    main()
