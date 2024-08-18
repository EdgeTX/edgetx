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

#if !defined(DISABLE_MULTI_UPDATE)

#include "hal.h"
#include "hal/module_port.h"

#include <stdio.h>
#include "edgetx.h"
#include "multi_firmware_update.h"
#include "stk500.h"
#include "debug.h"

#include "timers_driver.h"
#include "hal/watchdog_driver.h"

#include <memory>

#if defined(LIBOPENUI)
  #include "libopenui.h"
#else
  #include "lib_file.h"
#endif

#if defined(MULTI_PROTOLIST)
  #include "io/multi_protolist.h"
#endif

#define UPDATE_MULTI_EXT_BIN ".bin"

class MultiFirmwareUpdateDriver
{
  ModuleIndex module;
  MultiModuleType type;

  etx_module_state_t* mod_st = nullptr;

  bool init();
  bool getByte(uint8_t& byte) const;
  void sendByte(uint8_t byte) const;
  void sendBuffer(uint8_t* buffer, uint16_t size) const;
  void clear() const;
  void deinit();

  bool getRxByte(uint8_t& byte) const;
  bool checkRxByte(uint8_t byte) const;
  const char* waitForInitialSync();
  const char* getDeviceSignature(uint8_t* signature) const;
  const char* loadAddress(uint32_t offset) const;
  const char* progPage(uint8_t* buffer, uint16_t size) const;
  void leaveProgMode();

 public:
  MultiFirmwareUpdateDriver(ModuleIndex module, MultiModuleType type) :
      module(module), type(type)
  {
  }

  const char* flashFirmware(FIL* file, const char* label,
                            ProgressHandler progressHandler);
};

static const etx_serial_init serialInitParams = {
  .baudrate = 57600,
  .encoding = ETX_Encoding_8N1,
  .direction = ETX_Dir_TX_RX,
  .polarity = ETX_Pol_Normal,
};

bool MultiFirmwareUpdateDriver::init()
{
  if (type == MULTI_TYPE_MULTIMODULE && module == INTERNAL_MODULE) {
    // duplex internal USART
    mod_st = modulePortInitSerial(INTERNAL_MODULE, ETX_MOD_PORT_UART, &serialInitParams, false);
  } else if (type == MULTI_TYPE_MULTIMODULE && module == EXTERNAL_MODULE) {
    // TX on PPM (inverted softserial)
    etx_serial_init params(serialInitParams);
    params.direction = ETX_Dir_TX;
    params.polarity = ETX_Pol_Inverted;
    mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_UART, &params, true);
    if (!mod_st) return false;

    // RX on S.PORT (inverted softserial)
    params.direction = ETX_Dir_RX;
    params.polarity = ETX_Pol_Inverted;
    if (!modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_SPORT, &params, true)) {
      modulePortDeInit(mod_st);
      return false;
    }
  } else if (type == MULTI_TYPE_ELRS && module == EXTERNAL_MODULE) {
    // half-duplex on S.PORT USART
    mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_SPORT, &serialInitParams, false);
  }

  if (!mod_st) return false;

  modulePortSetPower(module, true);
  return true;
}

bool MultiFirmwareUpdateDriver::getByte(uint8_t & byte) const
{
  auto drv = modulePortGetSerialDrv(mod_st->rx);
  auto ctx = modulePortGetCtx(mod_st->rx);
  return drv->getByte(ctx, &byte) > 0;
}

void MultiFirmwareUpdateDriver::sendByte(uint8_t byte) const
{
  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto ctx = modulePortGetCtx(mod_st->tx);
  drv->sendByte(ctx, byte);
}

void MultiFirmwareUpdateDriver::sendBuffer(uint8_t* buffer, uint16_t size) const
{
  auto drv = modulePortGetSerialDrv(mod_st->tx);
  auto ctx = modulePortGetCtx(mod_st->tx);

  drv->waitForTxCompleted(ctx);
  drv->sendBuffer(ctx, buffer, size);
  drv->waitForTxCompleted(ctx);
}

void MultiFirmwareUpdateDriver::clear() const
{
  auto drv = modulePortGetSerialDrv(mod_st->rx);
  auto ctx = modulePortGetCtx(mod_st->rx);
  drv->clearRxBuffer(ctx);
}

void MultiFirmwareUpdateDriver::deinit()
{
  clear();
  modulePortSetPower(module, false);
  modulePortDeInit(mod_st);
}

bool MultiFirmwareUpdateDriver::getRxByte(uint8_t & byte) const
{
  uint32_t time = RTOS_GET_MS();
  
  while ((RTOS_GET_MS() - time) < 100) {              // 100ms
    if (getByte(byte)) {
#if defined(DEBUG_EXT_MODULE_FLASH)
      TRACE("[RX] 0x%X", byte);
#endif
      return true;
    }
  }

  byte = 0;
  return false;
}

bool MultiFirmwareUpdateDriver::checkRxByte(uint8_t byte) const
{
  uint8_t rxchar;
  return getRxByte(rxchar) ? rxchar == byte : false;
}

const char * MultiFirmwareUpdateDriver::waitForInitialSync()
{
  uint8_t byte;
  tmr10ms_t now = get_tmr10ms();;

#if defined(DEBUG_EXT_MODULE_FLASH)
  TRACE("[Wait for Sync]");
#endif

  clear();
  do {

    // Send sync request
    sendByte(STK_GET_SYNC);
    sendByte(CRC_EOP);

    getRxByte(byte);
    WDG_RESET();

  } while ((byte != STK_INSYNC) && ((get_tmr10ms() - now) < 500)); // 5secs

  if ((get_tmr10ms() - now) > 500) {
    return STR_DEVICE_NO_RESPONSE;
  }

  if (byte != STK_INSYNC) {
#if defined(DEBUG_EXT_MODULE_FLASH)
    TRACE("[byte != STK_INSYNC]");
#endif
    return STR_DEVICE_NO_RESPONSE;
  }

  if (!checkRxByte(STK_OK)) {
#if defined(DEBUG_EXT_MODULE_FLASH)
    TRACE("[!checkRxByte(STK_OK)]");
#endif
    return STR_DEVICE_NO_RESPONSE;
  }

  // avoids sending STK_READ_SIGN with STK_OK
  // in case the receiver is too slow changing
  // to RX mode (half-duplex).
  RTOS_WAIT_TICKS(1);

  return nullptr;
}

const char * MultiFirmwareUpdateDriver::getDeviceSignature(uint8_t * signature) const
{
  clear();

  // Read signature
  sendByte(STK_READ_SIGN);
  sendByte(CRC_EOP);

  if (!checkRxByte(STK_INSYNC))
    return STR_DEVICE_NO_RESPONSE;

  for (uint8_t i = 0; i < 4; i++) {
    if (!getRxByte(signature[i])) {
      return STR_DEVICE_FILE_WRONG_SIG;
    }
  }

  return nullptr;
}

const char * MultiFirmwareUpdateDriver::loadAddress(uint32_t offset) const
{
  sendByte(STK_LOAD_ADDRESS);
  sendByte(offset & 0xFF); // low  byte
  sendByte(offset >> 8);   // high byte
  sendByte(CRC_EOP);

  if (!checkRxByte(STK_INSYNC) || !checkRxByte(STK_OK)) {
    return STR_DEVICE_NO_RESPONSE;
  }

  // avoids sending next page back-to-back with STK_OK
  // in case the receiver is to slow changing to RX mode (half-duplex).
  RTOS_WAIT_TICKS(1);

  return nullptr;
}

const char * MultiFirmwareUpdateDriver::progPage(uint8_t * buffer, uint16_t size) const
{
  sendByte(STK_PROG_PAGE);

  // page size
  sendByte(size >> 8);
  sendByte(size & 0xFF);

  // flash/eeprom flag
  sendByte(0);

  sendBuffer(buffer, size);

  sendByte(CRC_EOP);

  if (!checkRxByte(STK_INSYNC))
    return STR_DEVICE_NO_RESPONSE;

  uint8_t byte;
  uint8_t retries = 4;
  do {
    getRxByte(byte);
    WDG_RESET();
  } while (!byte && --retries);

  if (!retries || (byte != STK_OK))
    return STR_DEVICE_WRONG_REQUEST;

  return nullptr;
}

void MultiFirmwareUpdateDriver::leaveProgMode()
{
  sendByte(STK_LEAVE_PROGMODE);
  sendByte(CRC_EOP);

  // eat last sync byte
  checkRxByte(STK_INSYNC);
  deinit();
}

const char* MultiFirmwareUpdateDriver::flashFirmware(
    FIL* file, const char* label, ProgressHandler progressHandler)
{
#if defined(SIMU)
  for (uint16_t i = 0; i < 100; i++) {
    progressHandler(label, STR_WRITING, i, 100);
    if (SIMU_SLEEP_OR_EXIT_MS(30))
      break;
  }
  return nullptr;
#endif

  const char * result = nullptr;
  if (!init()) return "Initialisation error";

  /* wait 500ms for power on */
  watchdogSuspend(500 /*5s*/);
  RTOS_WAIT_MS(500);

  result = waitForInitialSync();
  if (result) {
    leaveProgMode();
    return result;
  }

  unsigned char signature[4]; // 3 bytes signature + STK_OK
  result = getDeviceSignature(signature);
  if (result) {
    leaveProgMode();
    return result;
  }

  uint8_t buffer[256];
  uint16_t pageSize = 128;
  uint32_t writeOffset = 0;

  if (signature[0] != 0x1E) {
    leaveProgMode();
    return STR_DEVICE_FILE_WRONG_SIG;
  }

  if (signature[1] == 0x55 && signature[2] == 0xAA) {
    pageSize = 256;
    writeOffset = 0x1000; // start offset (word address)
  }

  while (!f_eof(file)) {
    progressHandler(label, STR_WRITING, file->fptr, file->obj.objsize);

    UINT count = 0;
    memclear(buffer, pageSize);
    if (f_read(file, buffer, pageSize, &count) != FR_OK) {
      result = STR_DEVICE_FILE_ERROR;
      break;
    }

    if (!count)
      break;

    clear();

    result = loadAddress(writeOffset);
    if (result) {
      break;
    }

    result = progPage(buffer, pageSize);
    if (result) {
      break;
    }

    writeOffset += pageSize / 2;
  }

  if (f_eof(file)) {
    progressHandler(label, STR_WRITING, file->fptr, file->obj.objsize);
  }

  leaveProgMode();
  return result;
}

// example :  multi-stm-bcsid-01020176
#define MULTI_SIGN_SIZE                             24
#define MULTI_SIGN_BOOTLOADER_SUPPORT_OFFSET        10
#define MULTI_SIGN_BOOTLOADER_CHECK_OFFSET          11
#define MULTI_SIGN_TELEM_TYPE_OFFSET                12
#define MULTI_SIGN_TELEM_INVERSION_OFFSET           13
#define MULTI_SIGN_VERSION_OFFSET                   15

const char * MultiFirmwareInformation::readV1Signature(const char * buffer)
{
  if (!memcmp(buffer, "multi-stm", 9))
    boardType = FIRMWARE_MULTI_STM;
  else if (!memcmp(buffer, "multi-avr", 9))
    boardType = FIRMWARE_MULTI_AVR;
  else if (!memcmp(buffer, "multi-orx", 9))
    boardType = FIRMWARE_MULTI_ORX;
  else
    return STR_DEVICE_FILE_WRONG_SIG;

  if (buffer[MULTI_SIGN_BOOTLOADER_SUPPORT_OFFSET] == 'b')
    optibootSupport = true;
  else
    optibootSupport = false;

  if (buffer[MULTI_SIGN_BOOTLOADER_CHECK_OFFSET] == 'c')
    bootloaderCheck = true;
  else
    bootloaderCheck = false;

  if (buffer[MULTI_SIGN_TELEM_TYPE_OFFSET] == 't')
    telemetryType = FIRMWARE_MULTI_TELEM_MULTI_STATUS;
  else if (buffer[MULTI_SIGN_TELEM_TYPE_OFFSET] == 's')
    telemetryType = FIRMWARE_MULTI_TELEM_MULTI_TELEMETRY;
  else
    telemetryType = FIRMWARE_MULTI_TELEM_NONE;

  if (buffer[MULTI_SIGN_TELEM_INVERSION_OFFSET] == 'i')
    telemetryInversion = true;
  else
    telemetryInversion = false;

  return nullptr;
}

const char * MultiFirmwareInformation::readV2Signature(const char * buffer)
{
  // new format
  uint32_t options = 0;
  const char * beg = buffer + 7;
  const char * cursor = beg;

  while (cursor - beg < 8) {
    options <<= 4;
    if (*cursor >= '0' && *cursor <= '9')
      options |= *cursor - '0';
    else if (*cursor >= 'a' && *cursor <= 'f')
      options |= *cursor - 'a' + 10;
    else if (*cursor >= 'A' && *cursor <= 'F')
      options |= *cursor - 'A' + 10;
    else
      break; // should be '-'
    cursor++;
  }

  if (cursor - beg < 8)
    return STR_DEVICE_FILE_WRONG_SIG;

  boardType = options & 0x3;
  optibootSupport = options & 0x80 ? true : false;
  telemetryInversion = options & 0x200 ? true : false;
  bootloaderCheck = options & 0x100 ? true : false;

  telemetryType = FIRMWARE_MULTI_TELEM_NONE;
  if (options & 0x400)
    telemetryType = FIRMWARE_MULTI_TELEM_MULTI_STATUS;
  if (options & 0x800)
    telemetryType = FIRMWARE_MULTI_TELEM_MULTI_TELEMETRY;

  return nullptr;
}

const char * MultiFirmwareInformation::readMultiFirmwareInformation(const char * filename)
{
  FIL file;
  if (f_open(&file, filename, FA_READ) != FR_OK)
    return STR_DEVICE_FILE_ERROR;

  const char * err = readMultiFirmwareInformation(&file);
  f_close(&file);

  return err;
}

const char * MultiFirmwareInformation::readMultiFirmwareInformation(FIL * file)
{
  char buffer[MULTI_SIGN_SIZE];
  UINT count;

  if (f_size(file) < MULTI_SIGN_SIZE)
    return STR_DEVICE_FILE_ERROR;

  f_lseek(file, f_size(file) - MULTI_SIGN_SIZE);
  if (f_read(file, buffer, MULTI_SIGN_SIZE, &count) != FR_OK || count != MULTI_SIGN_SIZE) {
    return STR_DEVICE_FILE_ERROR;
  }

  if (!memcmp(buffer, "multi-x", 7)) {
    return readV2Signature(buffer);
  }

  return readV1Signature(buffer);
}

bool MultiDeviceFirmwareUpdate::flashFirmware(const char * filename, ProgressHandler progressHandler)
{
  FIL file;

  if (f_open(&file, filename, FA_READ) != FR_OK) {
    POPUP_WARNING(STR_DEVICE_FILE_ERROR);
    return false;
  }

  if (type == MULTI_TYPE_MULTIMODULE) {
    MultiFirmwareInformation firmwareFile;
    if (firmwareFile.readMultiFirmwareInformation(&file)) {
      f_close(&file);
      POPUP_WARNING(STR_DEVICE_FILE_ERROR);
      return false;
    }
    f_lseek(&file, 0);

#if defined(HARDWARE_EXTERNAL_MODULE)
    if (module == EXTERNAL_MODULE) {
      if (!firmwareFile.isMultiExternalFirmware()) {
        f_close(&file);
        POPUP_WARNING(STR_NEEDS_FILE, STR_EXT_MULTI_SPEC);
        return false;
      }
    }
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
    if (module == INTERNAL_MODULE) {
      if (!firmwareFile.isMultiInternalFirmware()) {
        f_close(&file);
        POPUP_WARNING(STR_NEEDS_FILE, STR_INT_MULTI_SPEC);
        return false;
      }
    }
#endif
  }

  pulsesStop();

#if defined(MULTI_PROTOLIST)
  for (uint8_t i = 0; i < MAX_MODULES; i++) {
    MultiRfProtocols::removeInstance(i);
  }
#endif

  // switch S.PORT power OFF if supported
  modulePortSetPower(SPORT_MODULE, false);

  progressHandler(getBasename(filename), STR_DEVICE_RESET, 0, 0);

  /* wait 2s off */
  watchdogSuspend(500 /*5s*/);
  RTOS_WAIT_MS(3000);

  MultiFirmwareUpdateDriver driver(module, type);
  const char * result = driver.flashFirmware(&file, getBasename(filename), progressHandler);
  f_close(&file);

  AUDIO_PLAY(AU_SPECIAL_SOUND_BEEP1);
  BACKLIGHT_ENABLE();

  if (result) {
    POPUP_WARNING(STR_FIRMWARE_UPDATE_ERROR, result);
  }
  else {
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

  return result == nullptr;
}
#endif
