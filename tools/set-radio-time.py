#!/usr/bin/env python3
"""Set an EdgeTX radio's clock from the host, over the CLI serial port.

The radio must be in USB Serial (VCP) mode, running a build with CLI enabled.

The command body is sent ahead of time and only the closing newline is timed, so
what has to land on the second boundary is a single byte rather than a whole
line.  The radio treats a CLI set as host timed and skips the margin it allows
for someone turning the dials by hand, which leaves the drift measurement good
to the USB round trip, a few milliseconds.

That accuracy is in the measurement, not in the resulting clock: the RTC is
written in whole seconds and keeps whatever sub-second phase it already had, so
the radio can still read up to a second away from the host afterwards.

Every setting rewrites the drift reference, and two settings less than a day
apart are discarded, so do not put this on a short schedule or the radio never
learns its crystal error.  Every day or two is enough for it to converge.
"""

import argparse
import sys
import time

try:
    import serial
    from serial.tools import list_ports
except ImportError:
    sys.exit("pyserial is required:  python3 -m pip install pyserial")


# radio/src/targets/common/arm/stm32/usbd_desc.c, fixed for the Windows ST driver
USB_CDC_VID = 0x0483
USB_CDC_PID = 0x5740


def _device(port):
    """macOS lists both tty and cu for one device; only cu opens without DCD."""
    dev = port.device
    if sys.platform == "darwin" and dev.startswith("/dev/tty."):
        return "/dev/cu." + dev[len("/dev/tty."):]
    return dev


def _looks_like_cdc(dev):
    name = dev.upper()
    return name.startswith("COM") or "TTYACM" in name or "USBMODEM" in name


def list_candidates():
    """Best first: exact USB ids, then self-named, then any plausible CDC port."""
    exact, named, generic = [], [], []
    for port in list_ports.comports():
        dev = _device(port)
        if port.vid == USB_CDC_VID and port.pid == USB_CDC_PID:
            bucket = exact
        else:
            blob = " ".join(x for x in (port.description, port.manufacturer,
                                        port.product) if x).lower()
            if "edgetx" in blob or "opentx" in blob:
                bucket = named
            elif _looks_like_cdc(dev):
                bucket = generic
            else:
                continue
        if dev not in bucket:
            bucket.append(dev)
    return exact, named, generic


def find_port():
    exact, named, generic = list_candidates()
    if len(exact) > 1:
        sys.exit("several radios found, pick one with --port:\n  " +
                 "\n  ".join(exact))
    for bucket in (exact, named, generic):
        if bucket:
            return bucket[0]
    return None


def drain(ser, seconds):
    out = bytearray()
    end = time.time() + seconds
    while time.time() < end:
        chunk = ser.read(256)
        if chunk:
            out += chunk
        else:
            time.sleep(0.01)
    return out.decode("ascii", "replace")


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("-p", "--port",
                    help="serial port, e.g. /dev/cu.usbmodem1234 on macOS, "
                         "/dev/ttyACM0 on Linux, COM4 on Windows "
                         "(default: autodetect)")
    ap.add_argument("--list", action="store_true",
                    help="list the ports autodetection can see, then exit")
    ap.add_argument("-u", "--utc", action="store_true",
                    help="send UTC instead of host local time")
    ap.add_argument("-l", "--lead", type=float, default=0.004,
                    help="seconds to send the newline early, to cover USB latency "
                         "(default: 0.004)")
    ap.add_argument("-n", "--dry-run", action="store_true",
                    help="show the command without opening the port")
    ap.add_argument("-c", "--check", action="store_true",
                    help="run 'p rtc' afterwards and show the result")
    args = ap.parse_args()

    if args.list:
        for label, bucket in zip(("usb id match", "named", "possible"),
                                 list_candidates()):
            for dev in bucket:
                print("%-12s  %s" % (label, dev))
        return 0

    port = args.port or find_port()
    if not port and not args.dry_run:
        sys.exit("no radio found; is it plugged in and set to USB Serial (VCP)?")

    # aim at the start of a whole second, far enough out to get the body there
    target = int(time.time()) + 2
    tm = time.gmtime(target) if args.utc else time.localtime(target)
    # the fractional second says the newline is meant to land on the boundary,
    # which lets the radio skip the margin it allows for a hand set
    body = "set rtc %04d %02d %02d %02d %02d %02d.000" % (
        tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec)

    if args.dry_run:
        print("port: %s" % (port or "none found"))
        print("would send: %s" % body)
        return 0

    with serial.Serial(port, 115200, timeout=0.05) as ser:
        ser.write(b"\n")            # wake the prompt
        time.sleep(0.25)
        ser.reset_input_buffer()

        ser.write(body.encode())    # everything except the newline
        ser.flush()

        while time.time() < target - args.lead:
            time.sleep(0.0005)
        ser.write(b"\n")
        ser.flush()

        print("%s  (%s)" % (body, "UTC" if args.utc else "local"))
        print(drain(ser, 1.0).strip())

        if args.check:
            ser.reset_input_buffer()
            ser.write(b"p rtc\n")
            ser.flush()
            print(drain(ser, 1.0).strip())

    return 0


if __name__ == "__main__":
    sys.exit(main())
