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

#include <stdio.h>
#include "edgetx.h"
#include "frsky_firmware_update.h"
#include "debug.h"
#include "timers_driver.h"
#include "tasks/mixer_task.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#else
  #include "lib_file.h"
#endif

#include "hal/module_port.h"

#define PRIM_REQ_POWERUP    0
#define PRIM_REQ_VERSION    1
#define PRIM_CMD_DOWNLOAD   3
#define PRIM_DATA_WORD      4
#define PRIM_DATA_EOF       5

#define PRIM_ACK_POWERUP    0x80
#define PRIM_ACK_VERSION    0x81
#define PRIM_REQ_DATA_ADDR  0x82
#define PRIM_END_DOWNLOAD   0x83
#define PRIM_DATA_CRC_ERR   0x84

const char * readFrSkyFirmwareInformation(const char * filename, FrSkyFirmwareInformation & data)
{
  FIL file;
  UINT count;

  if (f_open(&file, filename, FA_READ) != FR_OK) {
    return STR_NEEDS_FILE;
  }

  if (f_read(&file, &data, sizeof(data), &count) != FR_OK || count != sizeof(data)) {
    f_close(&file);
    return STR_DEVICE_FILE_ERROR;
  }

  uint32_t size = f_size(&file);
  f_close(&file);

  if (data.headerVersion != 1 && data.fourcc != 0x4B535246) {
    return STR_DEVICE_FILE_ERROR;
  }

  if (size != sizeof(data) + data.size) {
    return STR_DEVICE_FILE_ERROR;
  }

  return nullptr;
}

void FrskyDeviceFirmwareUpdate::processFrame(const uint8_t * frame)
{
  if (frame[0] == 0x5E && frame[1] == 0x50) {
    switch (frame[2]) {
      case PRIM_ACK_POWERUP :
        if (state == SPORT_POWERUP_REQ) {
          state = SPORT_POWERUP_ACK;
        }
        break;

      case PRIM_ACK_VERSION:
        if (state == SPORT_VERSION_REQ) {
          state = SPORT_VERSION_ACK;
          // here we could display the version
        }
        break;

      case PRIM_REQ_DATA_ADDR:
        if (state == SPORT_DATA_TRANSFER) {
          address = *((uint32_t *)(&frame[3]));
          state = SPORT_DATA_REQ;
        }
        break;

      case PRIM_END_DOWNLOAD :
        state = SPORT_COMPLETE ;
        break;

      case PRIM_DATA_CRC_ERR :
        state = SPORT_FAIL ;
        break;
    }
  }
}

bool FrskyDeviceFirmwareUpdate::readBuffer(uint8_t * buffer, uint8_t count, uint32_t timeout)
{
  watchdogSuspend(timeout);

  uint32_t elapsed = 0;
  uint8_t index = 0;
  while (index < count && elapsed < timeout) {
    if (uart_drv->getByte(uart_ctx, &(buffer[index]))) {
      ++index;
    }
    else {
      RTOS_WAIT_MS(1);
      if (++elapsed == timeout)
        return false;
    }
  }

  return true;
}

const uint8_t * FrskyDeviceFirmwareUpdate::readFrame(uint32_t timeout)
{
  RTOS_WAIT_MS(1);

  uint8_t len = 0;
  bool bytestuff = false;

  while (len <= FRSKY_SPORT_PACKET_SIZE) {

    uint32_t elapsed = 0;
    uint8_t byte = 0;

    while (uart_drv->getByte(uart_ctx, &byte) == 0) {
      RTOS_WAIT_MS(1);
      if (elapsed++ >= timeout) {
        TRACE("timeout in frame (len=%d)",len);
        return nullptr;
      }
    }

    if (byte == BYTE_STUFF) {
      bytestuff = true;
      continue;
    }

    if (bytestuff) {
      frame[len] = STUFF_MASK ^ byte;
      bytestuff = false;
    }
    else {
      frame[len] = byte;
    }

    if (len > 0 || byte == START_STOP) {
      ++len;
    }
  }

  return &frame[1];
}

bool FrskyDeviceFirmwareUpdate::waitState(State newState, uint32_t timeout)
{
#if defined(SIMU)
  UNUSED(state);
  UNUSED(timeout);
  static uint8_t pass = 0;
  if (++pass == 10) {
    pass = 0;
    RTOS_WAIT_MS(1);
  }
  return true;
#else
  watchdogSuspend(timeout / 10);

  const uint8_t * frame = readFrame(timeout);
  if (!frame) {
    return false;
  }

  processFrame(frame);
  return state == newState;
#endif
}

void FrskyDeviceFirmwareUpdate::startFrame(uint8_t command)
{
  frame[0] = 0x50;
  frame[1] = command;
  memset(&frame[2], 0, 6);
}

// TODO merge this function
void FrskyDeviceFirmwareUpdate::sendFrame()
{
  uint8_t * ptr = outputTelemetryBuffer.data;
  *ptr++ = START_STOP;
  *ptr++ = 0xFF;
  frame[7] = crc16(CRC_1021, frame, 7);
  for (int i = 0; i < 8; i++) {
    if (frame[i] == START_STOP || frame[i] == BYTE_STUFF) {
      *ptr++ = BYTE_STUFF;
      *ptr++ = STUFF_MASK ^ frame[i];
    } else {
      *ptr++ = frame[i];
    }
  }

  uart_drv->sendBuffer(uart_ctx, outputTelemetryBuffer.data,
                       ptr - outputTelemetryBuffer.data);
  return;
}

const char * FrskyDeviceFirmwareUpdate::sendPowerOn()
{
  state = SPORT_POWERUP_REQ;

  RTOS_WAIT_MS(50);
  uart_drv->clearRxBuffer(uart_ctx);

  for (int i=0; i<10; i++) {
    // max 10 attempts
    startFrame(PRIM_REQ_POWERUP);
    sendFrame();
    if (waitState(SPORT_POWERUP_ACK, 100))
      return nullptr;
  }

  return STR_DEVICE_NO_RESPONSE;
}

const char * FrskyDeviceFirmwareUpdate::sendReqVersion()
{
  RTOS_WAIT_MS(20);
  uart_drv->clearRxBuffer(uart_ctx);

  state = SPORT_VERSION_REQ;
  for (int i=0; i<10; i++) {
    // max 10 attempts
    startFrame(PRIM_REQ_VERSION) ;
    sendFrame();
    if (waitState(SPORT_VERSION_ACK, 100))
      return nullptr;
  }
  return "Version request failed";
}

// X12S / X10 IXJT = use TX + RX @ 38400 bauds with BOOTCMD pin inverted
// X10 / X10 ISRM = use TX + RX @ 57600 bauds (no BOOTCMD)
// X9D / X9D+ / X9E / XLite IXJT = use S.PORT @ 57600 bauds
// XLite PRO / X9Lite / X9D+ 2019 ISRM = use TX + RX @ 57600 bauds

static const etx_serial_init serialInitParams = {
  .baudrate = 0,
  .encoding = ETX_Encoding_8N1,
  .direction = ETX_Dir_TX_RX,
  .polarity = ETX_Pol_Normal,
};

const char *FrskyDeviceFirmwareUpdate::doFlashFirmware(
    const char *filename, ProgressHandler progressHandler)
{
  FIL file;
  const char *result;
  FrSkyFirmwareInformation information;
  UINT count;

  if (f_open(&file, filename, FA_READ) != FR_OK) {
    return STR_NEEDS_FILE;
  }

  // Start with checking if we have a firmware description:
  // this mainly covers "newer" modules (ISRM, ACCESS based)

  // default to S.PORT update...
  uint8_t port = ETX_MOD_PORT_SPORT;
  void (*set_pwr)(uint8_t) = nullptr;
  void (*set_bootcmd)(uint8_t) = nullptr;

  // TODO: what about S.PORT update power control? (RX?)
  auto mod_desc = modulePortGetModuleDescription(module);
  set_pwr = mod_desc->set_pwr;

  // ... and 57600 bps
  etx_serial_init cfg(serialInitParams);
  cfg.baudrate = 57600;
  
  const char *ext = getFileExtension(filename);
  if (ext && !strcasecmp(ext, FRSKY_FIRMWARE_EXT)) {
    auto ret =
        f_read(&file, &information, sizeof(FrSkyFirmwareInformation), &count);
    if (ret != FR_OK || count != sizeof(FrSkyFirmwareInformation)) {
      f_close(&file);
      return STR_DEVICE_FILE_ERROR;
    }    

    // probably some ISRM or ACCESS module...
    // TODO: what are we supposed to start with the firmware header file?
    if ((information.productFamily == FIRMWARE_FAMILY_INTERNAL_MODULE &&
         module != INTERNAL_MODULE) ||
        (information.productFamily == FIRMWARE_FAMILY_EXTERNAL_MODULE &&
         module != EXTERNAL_MODULE)) {

      return STR_DEVICE_FILE_WRONG_SIG;
    }

    if (information.productFamily == FIRMWARE_FAMILY_INTERNAL_MODULE) {
      port = ETX_MOD_PORT_UART;
    }

  } else if (module == INTERNAL_MODULE) {
    // For the older modules we need some guess work:
    //
    // - X12S / X10 IXJT = use TX + RX @ 38400 bauds with BOOTCMD pin
    //   -> internal USART + boot pin
    //
    // - X9D / X9D+ / X9E / XLite IXJT = use S.PORT @ 57600 bauds
    //   -> no internal USART OR no boot pin (XLite has an internal USART)

    // X12S / X10 iXJT: TX + RX @ 38400 bauds with BOOTCMD pin inverted
    if (mod_desc->set_bootcmd) {
      cfg.baudrate = 38400;
      port = ETX_MOD_PORT_UART;
      set_bootcmd = mod_desc->set_bootcmd;
    } else {
      // keep the above defaults (S.PORT @ 57600 bps)
    }
  }

  // SPORT_MODULE is only defined for power control
  // but has not ports declared on it.
  //
  uint8_t serial_module = module != SPORT_MODULE ? module : EXTERNAL_MODULE;

  mod_st = modulePortInitSerial(serial_module, port, &cfg, false);
  if (!mod_st) return "Communication port error";

  // assume RX port is the same as TX
  uart_drv = modulePortGetSerialDrv(mod_st->tx);
  uart_ctx = modulePortGetCtx(mod_st->tx);

  if (set_bootcmd) set_bootcmd(true);
  if (set_pwr) set_pwr(true);

  // wait a bit for PWR to settle
  RTOS_WAIT_MS(1);

  // Special update method for X12S / X10 iXJT
  if (module == INTERNAL_MODULE && port == ETX_MOD_PORT_UART && set_bootcmd != nullptr) {
    result = uploadFileToHorusXJT(filename, &file, progressHandler);
  } else {
    result = uploadFileNormal(filename, &file, progressHandler);
  }

  if (set_pwr) set_pwr(false);
  if (set_bootcmd) set_bootcmd(false);

  modulePortDeInit(mod_st);
  return result;
}

const char *FrskyDeviceFirmwareUpdate::uploadFileToHorusXJT(
    const char *filename, FIL *file, ProgressHandler progressHandler)
{
  uint32_t buffer[1024 / sizeof(uint32_t)];
  UINT count;
  uint8_t frame[8];

  uart_drv->clearRxBuffer(uart_ctx);
  if (!readBuffer(frame, 8, 100) || frame[0] != 0x01) {
    return STR_DEVICE_NO_RESPONSE;
  }

  uart_drv->sendByte(uart_ctx, 0x81);
  readBuffer(frame, 1, 100);

  if (!readBuffer(frame, 8, 100) || frame[0] != 0x02) {
      return STR_DEVICE_NO_RESPONSE;
  }

  uart_drv->sendByte(uart_ctx, 0x82);
  readBuffer(frame, 1, 100);

  uint8_t index = 0;
  while (true) {
    progressHandler(getBasename(filename), STR_WRITING, file->fptr, file->obj.objsize);

    if (f_read(file, buffer, 1024, &count) != FR_OK) {
      return STR_DEVICE_FILE_ERROR;
    }

    if (!readBuffer(frame, 2, 100))
        return STR_DEVICE_DATA_REFUSED;

    if (frame[0] != 0x11 || frame[1] != index)
        return STR_DEVICE_WRONG_REQUEST;

    if (count == 0) {
      uart_drv->sendByte(uart_ctx, 0xA1);
      RTOS_WAIT_MS(50);
      return nullptr;
    }

    if (count < 1024)
      memset(((uint8_t *)buffer) + count, 0, 1024 - count);

    uart_drv->sendByte(uart_ctx, frame[0] + 0x80);
    uart_drv->sendByte(uart_ctx, frame[1]);

    uint16_t crc_16 = crc16(CRC_1189, (uint8_t *)buffer, 1024, crc16(CRC_1189, &frame[1], 1));
    for (size_t i = 0; i < sizeof(buffer); i++) {
      uart_drv->sendByte(uart_ctx, ((uint8_t *)buffer)[i]);
    }
    uart_drv->sendByte(uart_ctx, crc_16 >> 8);
    uart_drv->sendByte(uart_ctx, crc_16);

    index++;
  }
}

void FrskyDeviceFirmwareUpdate::sendDataTransfer(uint32_t* buffer)
{
  startFrame(PRIM_DATA_WORD);
  uint32_t offset = (address & 1023) >> 2; // 32 bit word offset into buffer
  *((uint32_t *)(frame + 2)) = buffer[offset];
  frame[6] = address & 0x000000FF;
  state = SPORT_DATA_TRANSFER;
  sendFrame();
}

const char *FrskyDeviceFirmwareUpdate::uploadFileNormal(
    const char *filename, FIL *file, ProgressHandler progressHandler)
{
  uint32_t buffer[1024 / sizeof(uint32_t)];
  UINT count;

  const char * result = sendPowerOn();
  if (result)
    return result;

  result = sendReqVersion();
  if (result)
    return result;

  RTOS_WAIT_MS(200);
  uart_drv->clearRxBuffer(uart_ctx);

  state = SPORT_DATA_TRANSFER;
  startFrame(PRIM_CMD_DOWNLOAD);
  sendFrame();

  uint8_t retries = 0;

  while (true) {
    if (f_read(file, buffer, 1024, &count) != FR_OK) {
        return STR_DEVICE_FILE_ERROR;
    }

    count >>= 2;

    for (uint32_t i = 0; i < count; i++) {

      if (waitState(SPORT_DATA_REQ, 80)) {
        retries = 4; // reset retries
      } else if (retries > 0){
        --retries;
        TRACE("retrying last transfer (address=0x%04X)", address);
      } else {
        return STR_DEVICE_DATA_REFUSED;
      }

      sendDataTransfer(buffer);

      if (i == 0) {
        progressHandler(getBasename(filename), STR_WRITING, file->fptr, file->obj.objsize);
      }
    }

    if (count < 256) {
      break;
    }
  }

  return endTransfer();
}

const char * FrskyDeviceFirmwareUpdate::endTransfer()
{
  if (!waitState(SPORT_DATA_REQ, 2000))
      return STR_DEVICE_DATA_REFUSED;
  startFrame(PRIM_DATA_EOF);
  sendFrame();
  if (!waitState(SPORT_COMPLETE, 2000)) {
    return STR_DEVICE_FILE_REJECTED;
  }
  return nullptr;
}

const char *FrskyDeviceFirmwareUpdate::flashFirmware(
    const char *filename, ProgressHandler progressHandler)
{
  pulsesStop();

  // switch S.PORT power OFF if supported
  modulePortSetPower(SPORT_MODULE, false);

  progressHandler(getBasename(filename), STR_DEVICE_RESET, 0, 0);

  /* wait 2s off */
  watchdogSuspend(1000 /*10s*/);
  RTOS_WAIT_MS(2000);

  const char * result = doFlashFirmware(filename, progressHandler);

  AUDIO_PLAY(AU_SPECIAL_SOUND_BEEP1 );
  BACKLIGHT_ENABLE();

  if (result) {
    POPUP_WARNING(STR_FIRMWARE_UPDATE_ERROR, result);
  } else {
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);
  }

  watchdogSuspend(50 /*500ms*/);
  pulsesStart();

// TODO: S.PORT power control
//       -> where is it actually turned ON normally?
//
// #if defined(SPORT_UPDATE_PWR_GPIO)
//   if (spuPwr) {
//     SPORT_UPDATE_POWER_ON();
//   }
// #endif

  state = SPORT_IDLE;
  return result;
}
