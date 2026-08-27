#!/usr/bin/env python
# -*- coding: utf-8 -*-

# This program parses CRSF capture data

from __future__ import division, print_function

import sys, struct
import math
import argparse

lineNumber = 0
timeData = 0
prevTimeData = 0

crossfire_types = [
    "UINT8",
    "INT8",
    "UINT16",
    "INT16",
    "UINT32",
    "INT32",
    "UINT64",
    "INT64",
    "FLOAT",
    "TEXT_SELECTION",
    "STRING",
    "FOLDER",
    "INFO",
    "COMMAND",
    "VTX",
]

cmd_status = [
    "READY",
    "START",
    "PROGRESS",
    "CONFIRMATION_NEEDED",
    "CONFIRM",
    "CANCEL",
    "POLL",
]


def dump(data, maxLen=None):
    if maxLen and len(data) > maxLen:
        data = data[:maxLen]
    return " ".join("{:02x}".format(c) for c in data)


# CRC8 implementation with polynom = x^8+x^7+x^6+x^4+x^2+1 (0xD5)
crc8tab = [
    0x00, 0xD5, 0x7F, 0xAA, 0xFE, 0x2B, 0x81, 0x54,
    0x29, 0xFC, 0x56, 0x83, 0xD7, 0x02, 0xA8, 0x7D,
    0x52, 0x87, 0x2D, 0xF8, 0xAC, 0x79, 0xD3, 0x06,
    0x7B, 0xAE, 0x04, 0xD1, 0x85, 0x50, 0xFA, 0x2F,
    0xA4, 0x71, 0xDB, 0x0E, 0x5A, 0x8F, 0x25, 0xF0,
    0x8D, 0x58, 0xF2, 0x27, 0x73, 0xA6, 0x0C, 0xD9,
    0xF6, 0x23, 0x89, 0x5C, 0x08, 0xDD, 0x77, 0xA2,
    0xDF, 0x0A, 0xA0, 0x75, 0x21, 0xF4, 0x5E, 0x8B,
    0x9D, 0x48, 0xE2, 0x37, 0x63, 0xB6, 0x1C, 0xC9,
    0xB4, 0x61, 0xCB, 0x1E, 0x4A, 0x9F, 0x35, 0xE0,
    0xCF, 0x1A, 0xB0, 0x65, 0x31, 0xE4, 0x4E, 0x9B,
    0xE6, 0x33, 0x99, 0x4C, 0x18, 0xCD, 0x67, 0xB2,
    0x39, 0xEC, 0x46, 0x93, 0xC7, 0x12, 0xB8, 0x6D,
    0x10, 0xC5, 0x6F, 0xBA, 0xEE, 0x3B, 0x91, 0x44,
    0x6B, 0xBE, 0x14, 0xC1, 0x95, 0x40, 0xEA, 0x3F,
    0x42, 0x97, 0x3D, 0xE8, 0xBC, 0x69, 0xC3, 0x16,
    0xEF, 0x3A, 0x90, 0x45, 0x11, 0xC4, 0x6E, 0xBB,
    0xC6, 0x13, 0xB9, 0x6C, 0x38, 0xED, 0x47, 0x92,
    0xBD, 0x68, 0xC2, 0x17, 0x43, 0x96, 0x3C, 0xE9,
    0x94, 0x41, 0xEB, 0x3E, 0x6A, 0xBF, 0x15, 0xC0,
    0x4B, 0x9E, 0x34, 0xE1, 0xB5, 0x60, 0xCA, 0x1F,
    0x62, 0xB7, 0x1D, 0xC8, 0x9C, 0x49, 0xE3, 0x36,
    0x19, 0xCC, 0x66, 0xB3, 0xE7, 0x32, 0x98, 0x4D,
    0x30, 0xE5, 0x4F, 0x9A, 0xCE, 0x1B, 0xB1, 0x64,
    0x72, 0xA7, 0x0D, 0xD8, 0x8C, 0x59, 0xF3, 0x26,
    0x5B, 0x8E, 0x24, 0xF1, 0xA5, 0x70, 0xDA, 0x0F,
    0x20, 0xF5, 0x5F, 0x8A, 0xDE, 0x0B, 0xA1, 0x74,
    0x09, 0xDC, 0x76, 0xA3, 0xF7, 0x22, 0x88, 0x5D,
    0xD6, 0x03, 0xA9, 0x7C, 0x28, 0xFD, 0x57, 0x82,
    0xFF, 0x2A, 0x80, 0x55, 0x01, 0xD4, 0x7E, 0xAB,
    0x84, 0x51, 0xFB, 0x2E, 0x7A, 0xAF, 0x05, 0xD0,
    0xAD, 0x78, 0xD2, 0x07, 0x53, 0x86, 0x2C, 0xF9
]

def crc8(buffer):
    crc = 0
    for c in buffer:
        crc = crc8tab[crc ^ c]
    return crc

def ParseGPS(payload):
    lat, long, speed, head, alt, numsat = struct.unpack('>iiHHHB', bytes(bytearray(payload)))          # bytes(bytearray) casting is required for python 2.7.3 compatibility
    return "[GPS] lat:%f long:%f speed:%d heading:%d alt:%d numsat:%d" % (lat / 1e7, long / 1e7, speed / 100, head / 100, alt - 1000, numsat)

def ParseBattery(payload):
    voltage = float((payload[0] << 8) + payload[1]) / 10
    current = float((payload[2] << 8) + payload[3]) / 10
    consumption = (payload[4] << 16) + (payload[5] << 8) + payload[6]
    return "[Battery] %.1fV %.1fA %dmAh" % (voltage, current, consumption)

def ParseBaroAltVario(payload):
    alt_packed = (payload[0] << 8) + payload[1]
    altitude = float(alt_packed & 0x7fff) if (alt_packed & 0x8000) else (float(alt_packed - 10000) / 10)
    if len(payload) == 3:
        # per CRSF spec
        vario_packed = float(payload[2])
        vario = (math.exp(abs(vario_packed) * 0.026) - 1) * 100 * math.copysign(1, vario_packed)
    else:
        # ELRS and Rotorflight
        vario = float(struct.unpack('>h', bytes(payload[2:4]))[0] / 100)

    return "[Baro Altitude Vario] Altitude %.1f m, Vertical Speed %.2f m/s" % (altitude, vario)

def ParseVtxTelem(payload):
    return "[VTX Telemetry] freq=%d, power=%d, pitmode=%d" % (int.from_bytes(payload[1:3], 'big'), int(payload[3]), int(payload[4]))

def ParseLinkStatistics(payload):
    return "[Link Statistics] "

def ParseChannels(payload):
    return "[Channel Data] "

def ParseAttitude(payload):
    pitch = float((payload[0] << 8) + payload[1]) / 1000
    roll = float((payload[2] << 8) + payload[3]) / 1000
    yaw = float((payload[4] << 8) + payload[5]) / 1000
    return "[Attitude] pitch=%.3f roll=%.3f yaw=%.3f" % (pitch, roll, yaw)

def ParseFlightMode(payload):
    return '[Flight Mode] "%s"' % "".join([chr(c) for c in payload[:-1]])

def ParsePingDevices(_):
    return '[Ping Devices]'

def ParseDevice(payload):
    return '[Device] 0x%02x "%s" %d parameters' % (payload[1], "".join([chr(c) for c in payload[2:-14]]), payload[-2])

def ParseRadioId(payload):
    return '[RadioId] subcmd 0x%02x, interval %d, correction %d' % \
            (payload[2], int.from_bytes(payload[3:7], 'big')/10, int.from_bytes(payload[7:11], 'big', signed=True)/10)

def ParseLinkStatisticsRX(payload):
    return "[RX Link Stats] rssi=%d dBm rssi=%d%% lqi=%d snr=%d dB rf_power=%d dBm" % (payload[0], payload[1], payload[2], payload[3], payload[4])

def ParseCommand(payload):
    if payload[2]==0x10 and payload[3]==5:
        return '[Command] subcmd Crossfire, set ModelId, data %d' % (payload[4])
    if payload[2]==0x0a and payload[3]==0x70:
        return '[Command] subcmd General, Speed Proposal, port_id %d, bitrate %d' % (payload[4], int.from_bytes(payload[5:9], 'big'))
    if payload[2]==0x0a and payload[3]==0x71:
        return '[Command] subcmd General, Speed Response, port_id %d, %s' % (payload[4], 'accepted' if payload[5] else 'rejected')
    return '[Command] subcmd 0x%02x, datatype 0x%02x, data %d' % \
            (payload[2], payload[3], payload[4])

def ParseFieldsRequest(payload):
    return '[Fields request]'

lastChunkRequested = 0
chunkOutOfOrder = False
def ParseFieldRequest(payload):
    global lastChunkRequested, chunkOutOfOrder
    s = '[Field request] device=0x%02x field=%d chunk=%d' % (payload[1], payload[2], payload[3])
    if lastChunkRequested > 0 and payload[3] != lastChunkRequested+1:
        s += ' (OUT OF ORDER REQUEST!)'
        chunkOutOfOrder = True
    lastChunkRequested = payload[3]
    return s

def ParseFieldUpdate(payload):
    return '[Field update] device=0x%02x field=%d value=%s' % (payload[1], payload[2], ' '.join(f'{byte:02x}' for byte in payload[3:]))

def ParseELRSInfo(payload):
    return '[ELRS info] device=0x%02x bad=%d, good=%d, flags=0x%02x, flag_str=%s' % (payload[1], payload[2],
            int.from_bytes(payload[3:5], 'big'), payload[5], ''.join(map(chr, payload[6:-2])))

fieldBuff = []
lastChunk = 0
def ParseField(payload):
    global fieldBuff, lastChunk, lastChunkRequested, chunkOutOfOrder

    fieldBuff += payload[4:]
    if lastChunk > 0 and payload[3] != lastChunk-1:
        s = '[Field] device=0x%02x field=%d chunk=%d (OUT OF ORDER CHUNK!)' % (payload[1], payload[2], payload[3])
        lastChunk = 0
        fieldBuff = []
        return s
    if payload[3] != 0:
        lastChunk = payload[3]
        return '[Field] device=0x%02x field=%d chunk=%d' % (payload[1], payload[2], payload[3])

    print("fieldBuff: ", " ".join(f'{byte:02x}' for byte in fieldBuff))
    name = ""
    options = []
    i = 2
    try:
        while fieldBuff[i] != 0:
            name += chr(fieldBuff[i])
            i += 1
        i += 1
        ftype = crossfire_types[fieldBuff[1] & 0x7f]
        retstr = '[Field] %s device=0x%02x field=%d parent=%d type=%s' % (name, payload[1], payload[2], fieldBuff[0], ftype)
        if ftype == "TEXT_SELECTION":
            name = ""
            while fieldBuff[i] != 0:
                if fieldBuff[i] == 0x3b:
                    options.append(name)
                    name = ""
                    i += 1
                    continue
                name += chr(fieldBuff[i])
                i += 1
            options.append(name)
            i += 1
            retstr += ' selection=(%d) %s' % (fieldBuff[i], options[fieldBuff[i]])
            retstr += ' options %s' % (", ".join(options))
        if ftype == "INFO":
            value = ""
            while fieldBuff[i] != 0:
                value += chr(fieldBuff[i])
                i += 1
            retstr += ' name=%s value=%s' % (name, value)
        if ftype == "COMMAND":
            retstr += ' status=%s, timeout=%d ms, info=%s' % (cmd_status[fieldBuff[i]], fieldBuff[i+1], ''.join(map(chr, fieldBuff[i+2:])))
        fieldBuff = []
        lastChunk = 0
        lastChunkRequested = 0
        return retstr
    except Exception as inst:
        print(type(inst))    # the exception instance
        print(inst.args)     # arguments stored in .args
        print(inst)          # __str__ allows args to be printed directly,
#...                          # but may be overridden in exception subclasses
        print("i: ", i)
        print("len(payload): ", len(payload))
        if i < len(payload):
            print("payload[i-1]: ", payload[i-1])
        fieldBuff = []
        lastChunk = 0
        return '[Exception]'

parsers = {
    0x02: ParseGPS,
    0x08: ParseBattery,
    0x09: ParseBaroAltVario,
    0x10: ParseVtxTelem,
    0x14: ParseLinkStatistics,
    0x16: ParseChannels,
    0x1c: ParseLinkStatisticsRX,
    0x1E: ParseAttitude,
    0x21: ParseFlightMode,
    0x28: ParsePingDevices,
    0x29: ParseDevice,
    0x2a: ParseFieldsRequest,
    0x2b: ParseField,
    0x2c: ParseFieldRequest,
    0x2d: ParseFieldUpdate,
    0x2e: ParseELRSInfo,
    0x32: ParseCommand,
    0x3a: ParseRadioId,
}

def ParsePacket(packet):
    global timeData, prevTimeData, fieldBuff
    length = packet[1]
    command = packet[2]
    payload = packet[3:-1]
    crc = packet[-1]
    diffTime = timeData - prevTimeData
    prefix = '(%d)' % lineNumber if timeData == 0 else '%10.6f [%9.6f]' % (timeData, diffTime)
    prevTimeData = timeData
    timeData = 0
    if crc != crc8(packet[2:-1]):
        print(prefix, dump(packet), "[CRC error]")
        fieldBuff = []
        lastChunk = 0
        return
    if args.ignore and command == 0x16:
        return
    if args.omit and ((command == 0x2d and payload[2] == 0x00) or \
                      command == 0x2e):
        return
    if args.remove and command == 0x3a:
        return
    parser = parsers.get(command, None)
    if parser != None:
        print(prefix, dump(packet), parser(payload))
    else:
        print(prefix, dump(packet), '[Unknown Command 0x%0x]' % command)

crossfireDataBuff = []
chunkedBuffer = []
def ParseData(data):
    global crossfireDataBuff
    crossfireDataBuff += data
    # build packet for parsing
    # separate buffers for data sources so chunked data handled correctly
    while len(crossfireDataBuff) > 1:
        if crossfireDataBuff[0] != 0xc8 and crossfireDataBuff[0] != 0x00 and crossfireDataBuff[0] != 0xee and crossfireDataBuff[0] != 0xea:
            print("Skipped 1 byte", dump(crossfireDataBuff[:1]))
            crossfireDataBuff = crossfireDataBuff[1:]
            continue
        length = crossfireDataBuff[1]
        if length < 2 or length > 0x40:
            print("Skipped 1 bytex", dump(crossfireDataBuff[:1]))
            crossfireDataBuff = crossfireDataBuff[1:]
            continue
        if len(crossfireDataBuff) < length+2:
            break
        ParsePacket(crossfireDataBuff[:length+2])
        crossfireDataBuff = crossfireDataBuff[length+2:]

def readSport(inp):
    global timeData
    line = inp.readline()
    lineNumber += 1
    if len(line) == 0:
        return
    line = line.strip('\r\n')
    if len(line) == 0:
        return
    parts = line.split(': ')
    if len(parts) < 2:
        print("weird data: \"%s\" at line %d" % (line, lineNumber))
        return
    timeData = float(parts[0].strip())
    crossfireData = parts[1].strip()
    # convert from hex
    parts = crossfireData.split(' ')
    binData = [int(hex, 16) for hex in parts]
    return binData

def readCsv(inp):
    global lineNumber
    global timeData
    crossfireData = []
    line = inp.readline()
    if ('Value' in line):
        line = inp.readline()
    lineNumber += 1
    line = "".join(line.split())
    parts = line.split(',')
    if len(parts) < 2:
        return []
    crossfireData = [int(parts[1][2:4],16)]
    if timeData == 0:
        timeData = float(parts[0].strip())
    return crossfireData

def readHex(inp):
    global lineNumber
    crossfireData = []
    line = inp.readline()
    lineNumber += 1
    line = "".join(line.split())
    crossfireData = [int(hex,16) for hex in [line[i:i+2] for i in range(0, len(line), 2)]]
    return crossfireData

def readBinary(inp):
    crossfireData = []
    while (b := inp.read(1)):
        crossfireData += b
    return crossfireData


# execution starts here
parser = argparse.ArgumentParser()
parser.add_argument('inputFile', default='stdin', help='Input file with capture data (stdin works)')
parser.add_argument('-f', '--format', default='sport',
                    choices=['bin', 'hex', 'csv', 'sport'],
                    help='Type of input file. bin=binary, hex=ascii hex, csv=salae async serial export, sport=s.port log')
parser.add_argument('-i', '--ignore', action="store_true",
                    help='Ignore rc data (stick) packets')
parser.add_argument('-o', '--omit', action="store_true",
                    help='Ignore ELRS status request/response packets')
parser.add_argument('-r', '--remove', action="store_true",
                    help='Ignore RadioId CRSFshot messages')
args = parser.parse_args()

# open input
if args.inputFile == 'stdin':
    if args.format == 'bin':
        inp = sys.stdin.buffer
    else:
        inp = sys.stdin
else:
    if args.format == 'bin':
        inp = open(args.inputFile, 'rb')
    else:
        inp = open(args.inputFile, 'r')

if args.format == 'sport':
    get_data = readSport
elif args.format == 'csv':
    get_data = readCsv
elif args.format == 'bin':
    get_data = readBinary
else:
    get_data = readHex

while (crossfireData := get_data(inp)):
    ParseData(crossfireData)
