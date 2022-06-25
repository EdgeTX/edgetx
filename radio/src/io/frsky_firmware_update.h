/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _FRSKY_FIRMWARE_UPDATE_H_
#define _FRSKY_FIRMWARE_UPDATE_H_

#include "dataconstants.h"
#include "definitions.h"

enum FrskyFirmwareProductFamily {
  FIRMWARE_FAMILY_INTERNAL_MODULE,
  FIRMWARE_FAMILY_EXTERNAL_MODULE,
  FIRMWARE_FAMILY_RECEIVER,
  FIRMWARE_FAMILY_SENSOR,
  FIRMWARE_FAMILY_BLUETOOTH_CHIP,
  FIRMWARE_FAMILY_POWER_MANAGEMENT_UNIT,
  FIRMWARE_FAMILY_FLIGHT_CONTROLLER,
};

enum FrskyFirmwareModuleProductId {
  FIRMWARE_ID_MODULE_NONE,
  FIRMWARE_ID_MODULE_XJT = 0x01,
  FIRMWARE_ID_MODULE_ISRM = 0x02,
};

enum FrskyFirmwareReceiverProductId {
  FIRMWARE_ID_RECEIVER_NONE,
  FIRMWARE_ID_RECEIVER_X8R = 0x01,
  FIRMWARE_ID_RECEIVER_RX8R = 0x02,
  FIRMWARE_ID_RECEIVER_RX8R_PRO = 0x03,
  FIRMWARE_ID_RECEIVER_RX6R = 0x04,
  FIRMWARE_ID_RECEIVER_RX4R = 0x05,
  FIRMWARE_ID_RECEIVER_G_RX8 = 0x06,
  FIRMWARE_ID_RECEIVER_G_RX6 = 0x07,
  FIRMWARE_ID_RECEIVER_X6R = 0x08,
  FIRMWARE_ID_RECEIVER_X4R = 0x09,
  FIRMWARE_ID_RECEIVER_X4R_SB = 0x0A,
  FIRMWARE_ID_RECEIVER_XSR = 0x0B,
  FIRMWARE_ID_RECEIVER_XSR_M = 0x0C,
  FIRMWARE_ID_RECEIVER_RXSR = 0x0D,
  FIRMWARE_ID_RECEIVER_S6R = 0x0E,
  FIRMWARE_ID_RECEIVER_S8R = 0x0F,
  FIRMWARE_ID_RECEIVER_XM = 0x10,
  FIRMWARE_ID_RECEIVER_XMP = 0x11,
  FIRMWARE_ID_RECEIVER_XMR = 0x12,
  FIRMWARE_ID_RECEIVER_R9 = 0x13,
  FIRMWARE_ID_RECEIVER_R9_SLIM = 0x14,
  FIRMWARE_ID_RECEIVER_R9_SLIMP = 0x15,
  FIRMWARE_ID_RECEIVER_R9_MINI = 0x16,
  FIRMWARE_ID_RECEIVER_R9_MM = 0x17,
  FIRMWARE_ID_RECEIVER_R9_STAB = 0x18, // R9_STAB has OTA
  FIRMWARE_ID_RECEIVER_R9_MINI_OTA = 0x19, // this one has OTA (different bootloader)
  FIRMWARE_ID_RECEIVER_R9_MM_OTA = 0x1A, // this one has OTA (different bootloader)
  FIRMWARE_ID_RECEIVER_R9_SLIMP_OTA = 0x1B, // this one has OTA (different bootloader)
  FIRMWARE_ID_RECEIVER_ARCHER_X = 0x1C, // this one has OTA (internal module)
  FIRMWARE_ID_RECEIVER_R9MX = 0x1D, // this one has OTA
  FIRMWARE_ID_RECEIVER_R9SX = 0x1E, // this one has OTA
};

inline bool isReceiverOTAEnabledFromModule(uint8_t moduleIdx, uint8_t productId)
{
  switch (productId) {
    case FIRMWARE_ID_RECEIVER_ARCHER_X:
      return isModuleISRM(moduleIdx);

    case FIRMWARE_ID_RECEIVER_R9_STAB:
    case FIRMWARE_ID_RECEIVER_R9_MINI_OTA:
    case FIRMWARE_ID_RECEIVER_R9_MM_OTA:
    case FIRMWARE_ID_RECEIVER_R9_SLIMP_OTA:
    case FIRMWARE_ID_RECEIVER_R9MX:
    case FIRMWARE_ID_RECEIVER_R9SX:
      return isModuleR9M(moduleIdx);

    default:
      return false;
  }
}

PACK(struct FrSkyFirmwareInformation {
  uint32_t fourcc;
  uint8_t headerVersion;
  uint8_t firmwareVersionMajor;
  uint8_t firmwareVersionMinor;
  uint8_t firmwareVersionRevision;
  uint32_t size;
  uint8_t productFamily;
  uint8_t productId;
  uint16_t crc;
});

const char * readFrSkyFirmwareInformation(const char * filename, FrSkyFirmwareInformation & data);

class FrskyDeviceFirmwareUpdate {
    enum State {
      SPORT_IDLE,
      SPORT_POWERUP_REQ,
      SPORT_POWERUP_ACK,
      SPORT_VERSION_REQ,
      SPORT_VERSION_ACK,
      SPORT_DATA_TRANSFER,
      SPORT_DATA_REQ,
      SPORT_COMPLETE,
      SPORT_FAIL
    };

  public:
    explicit FrskyDeviceFirmwareUpdate(ModuleIndex module):
      module(module) {
    }

    const char * flashFirmware(const char * filename, ProgressHandler progressHandler);

  protected:
    uint8_t state = SPORT_IDLE;
    uint32_t address = 0;
    ModuleIndex module;
    uint8_t frame[12];
    void* uart_ctx = nullptr;

    void startFrame(uint8_t command);
    void sendFrame();

    bool readBuffer(uint8_t * buffer, uint8_t count, uint32_t timeout);
    const uint8_t * readFullDuplexFrame(uint32_t timeout);
    const uint8_t * readHalfDuplexFrame(uint32_t timeout);
    const uint8_t * readFrame(uint32_t timeout);
    bool waitState(State state, uint32_t timeout);
    void processFrame(const uint8_t * frame);

    const char * doFlashFirmware(const char * filename, ProgressHandler progressHandler);
    const char * sendPowerOn();
    const char * sendReqVersion();
    const char * uploadFileNormal(const char * filename, FIL * file, ProgressHandler progressHandler);
    const char * uploadFileToHorusXJT(const char * filename, FIL * file, ProgressHandler progressHandler);
    const char * endTransfer();
};

class FrskyChipFirmwareUpdate {
  public:
    FrskyChipFirmwareUpdate()
    {
    }

    const char * flashFirmware(const char * filename, ProgressHandler progressHandler, bool wait = true);

  protected:
    uint8_t crc;

    void sendByte(uint8_t byte, bool crc = true);
    const char * waitAnswer(uint8_t & status);
    const char * startBootloader();
    const char * sendUpgradeCommand(char command, uint32_t packetsCount);
    const char * sendUpgradeData(uint32_t index, uint8_t * data);

    const char * doFlashFirmware(const char * filename, ProgressHandler progressHandler);
};

#endif // _FRSKY_FIRMWARE_UPDATE_H_
