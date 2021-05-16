/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "opentx.h"
#include "multi_firmware_update.h"
#include "stk500.h"
#include "debug.h"

#define UPDATE_MULTI_EXT_BIN ".bin"

class MultiFirmwareUpdateDriver
{
  public:
    MultiFirmwareUpdateDriver() {}
    const char * flashFirmware(FIL * file, const char * label) const;

  protected:
    virtual void moduleOn() const = 0;
    virtual void init(bool inverted) const = 0;
    virtual bool getByte(uint8_t & byte) const = 0;
    virtual void sendByte(uint8_t byte) const = 0;
    virtual void clear() const = 0;
    virtual void deinit(bool inverted) const {}

  private:
    bool getRxByte(uint8_t & byte) const;
    bool checkRxByte(uint8_t byte) const;
    const char * waitForInitialSync(bool& inverted) const;
    const char * getDeviceSignature(uint8_t * signature) const;
    const char * loadAddress(uint32_t offset) const;
    const char * progPage(uint8_t * buffer, uint16_t size) const;
    void leaveProgMode(bool inverted) const;
};

#if defined(INTERNAL_MODULE_MULTI)
class MultiInternalUpdateDriver: public MultiFirmwareUpdateDriver
{
  public:
    MultiInternalUpdateDriver() {}

  protected:
    void moduleOn() const override
    {
      INTERNAL_MODULE_ON();
    }

    void init(bool inverted) const override
    {
      intmoduleSerialStart(57600, true, USART_Parity_No, USART_StopBits_1, USART_WordLength_8b);
    }

    bool getByte(uint8_t & byte) const override
    {
      return intmoduleFifo.pop(byte);
    }

    void sendByte(uint8_t byte) const override
    {
      intmoduleSendByte(byte);
    }

    void clear() const override
    {
      intmoduleFifo.clear();
    }

    void deinit(bool inverted) const override
    {
      clear();
    }
};

static const MultiInternalUpdateDriver multiInternalUpdateDriver;
#endif

class MultiExternalUpdateDriver: public MultiFirmwareUpdateDriver
{
  public:
    MultiExternalUpdateDriver() {}

  protected:
    void moduleOn() const override
    {
      EXTERNAL_MODULE_ON();
    }

    void init(bool inverted) const override
    {
#if !defined(EXTMODULE_USART)
      GPIO_InitTypeDef GPIO_InitStructure;
      GPIO_InitStructure.GPIO_Pin = EXTMODULE_TX_GPIO_PIN;
      GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
      GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
      GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
      GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
      GPIO_Init(EXTMODULE_TX_GPIO, &GPIO_InitStructure);
#endif

      if (inverted)
        telemetryPortInvertedInit(57600);
      else
        telemetryPortInit(57600, TELEMETRY_SERIAL_WITHOUT_DMA);
    }

    bool getByte(uint8_t & byte) const override
    {
      return telemetryGetByte(&byte);
    }

    void sendByte(uint8_t byte) const override
    {
#if defined(HARDWARE_EXTERNAL_MODULE)
      extmoduleSendInvertedByte(byte);
#endif
    }

    void clear() const override
    {
      telemetryClearFifo();
    }

    void deinit(bool inverted) const override
    {
      if (inverted)
        telemetryPortInvertedInit(0);
      else
        telemetryPortInit(0, 0);

      clear();
    }
};

static const MultiExternalUpdateDriver multiExternalUpdateDriver;

class MultiExtSportUpdateDriver: public MultiFirmwareUpdateDriver
{
  public:
    MultiExtSportUpdateDriver(): MultiFirmwareUpdateDriver() {}

  protected:
    void moduleOn() const override
    {
      EXTERNAL_MODULE_ON();
    }

    void init(bool inverted) const override
    {
      telemetryPortInit(57600, TELEMETRY_SERIAL_WITHOUT_DMA);
    }

    bool getByte(uint8_t & byte) const override
    {
      return telemetryGetByte(&byte);
    }

    void sendByte(uint8_t byte) const override
    {
      sportSendByte(byte);
      telemetryPortSetDirectionInput();
    }

    void clear() const override
    {
      telemetryClearFifo();
    }

    void deinit(bool inverted) const override
    {
      telemetryPortInit(0, 0);
      clear();
    }
};

static const MultiExtSportUpdateDriver multiExtSportUpdateDriver;

bool MultiFirmwareUpdateDriver::getRxByte(uint8_t & byte) const
{
  uint16_t time;

  time = getTmr2MHz();
  while ((uint16_t) (getTmr2MHz() - time) < 25000) {  // 12.5mS

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

const char * MultiFirmwareUpdateDriver::waitForInitialSync(bool & inverted) const
{
  uint8_t byte;
  int retries = 200;

#if defined(DEBUG_EXT_MODULE_FLASH)
  TRACE("[Wait for Sync]");
#endif

  clear();
  do {

    // Invert at half-time
    if (retries == 100) {
      deinit(inverted);
      inverted = !inverted;
      init(inverted);
    }

    // Send sync request
    sendByte(STK_GET_SYNC);
    sendByte(CRC_EOP);

    getRxByte(byte);
    WDG_RESET();

  } while ((byte != STK_INSYNC) && --retries);

  if (!retries) {
    return "NoSync";
  }

  if (byte != STK_INSYNC) {
#if defined(DEBUG_EXT_MODULE_FLASH)
    TRACE("[byte != STK_INSYNC]");
#endif
    return "NoSync";
  }

  if (!checkRxByte(STK_OK)) {
#if defined(DEBUG_EXT_MODULE_FLASH)
    TRACE("[!checkRxByte(STK_OK)]");
#endif
    return "NoSync";
  }

  // avoids sending STK_READ_SIGN with STK_OK
  // in case the receiver is too slow changing
  // to RX mode (half-duplex).
  RTOS_WAIT_TICKS(1);
  
  return nullptr;
}

const char * MultiFirmwareUpdateDriver::getDeviceSignature(uint8_t * signature) const
{
  // Read signature
  sendByte(STK_READ_SIGN);
  sendByte(CRC_EOP);
  clear();

  if (!checkRxByte(STK_INSYNC))
    return "NoSync";

  for (uint8_t i = 0; i < 4; i++) {
    if (!getRxByte(signature[i])) {
      return "NoSignature";
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
    return "NoSync";
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

  for (uint16_t i = 0; i < size; i++) {
    sendByte(buffer[i]);
  }

  sendByte(CRC_EOP);

  if (!checkRxByte(STK_INSYNC))
    return "NoSync";

  uint8_t byte;
  uint8_t retries = 4;
  do {
    getRxByte(byte);
    WDG_RESET();
  } while (!byte && --retries);

  if (!retries || (byte != STK_OK))
    return "NoPageSync";

  return nullptr;
}

void MultiFirmwareUpdateDriver::leaveProgMode(bool inverted) const
{
  sendByte(STK_LEAVE_PROGMODE);
  sendByte(CRC_EOP);

  // eat last sync byte
  checkRxByte(STK_INSYNC);
  deinit(inverted);
}

const char * MultiFirmwareUpdateDriver::flashFirmware(FIL * file, const char * label) const
{
  const char * result = nullptr;
  moduleOn();

  bool inverted = true; //false; // true
  init(inverted);

  /* wait 500ms for power on */
  watchdogSuspend(500 /*5s*/);
  RTOS_WAIT_MS(500);

  result = waitForInitialSync(inverted);
  if (result) {
    leaveProgMode(inverted);
    return result;
  }

  unsigned char signature[4]; // 3 bytes signature + STK_OK
  result = getDeviceSignature(signature);
  if (result) {
    leaveProgMode(inverted);
    return result;
  }

  uint8_t buffer[256];
  uint16_t pageSize = 128;
  uint32_t writeOffset = 0;

  if (signature[0] != 0x1E) {
    leaveProgMode(inverted);
    return "Wrong signature";
  }

  if (signature[1] == 0x55 && signature[2] == 0xAA) {
    pageSize = 256;
    writeOffset = 0x1000; // start offset (word address)
  }

  while (!f_eof(file)) {

    drawProgressScreen(label, STR_WRITING, file->fptr, file->obj.objsize);

    UINT count = 0;
    memclear(buffer, pageSize);
    if (f_read(file, buffer, pageSize, &count) != FR_OK) {
      result = "Error reading file";
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
    drawProgressScreen(label, STR_WRITING, file->fptr, file->obj.objsize);
  }

  leaveProgMode(inverted);
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
    return "Wrong format";

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
    return "Invalid signature";

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
    return "Error opening file";

  const char * err = readMultiFirmwareInformation(&file);
  f_close(&file);

  return err;
}

const char * MultiFirmwareInformation::readMultiFirmwareInformation(FIL * file)
{
  char buffer[MULTI_SIGN_SIZE];
  UINT count;

  if (f_size(file) < MULTI_SIGN_SIZE)
    return "File too small";

  f_lseek(file, f_size(file) - MULTI_SIGN_SIZE);
  if (f_read(file, buffer, MULTI_SIGN_SIZE, &count) != FR_OK || count != MULTI_SIGN_SIZE) {
    return "Error reading file";
  }

  if (!memcmp(buffer, "multi-x", 7)) {
    return readV2Signature(buffer);
  }

  return readV1Signature(buffer);
}

bool multiFlashFirmware(uint8_t moduleIdx, const char * filename, MultiModuleType type)
{
  FIL file;

  if (f_open(&file, filename, FA_READ) != FR_OK) {
    POPUP_WARNING("Not a valid file");
    return false;
  }

  if (type == MULTI_TYPE_MULTIMODULE) {
    MultiFirmwareInformation firmwareFile;
    if (firmwareFile.readMultiFirmwareInformation(&file)) {
      f_close(&file);
      POPUP_WARNING("Not a valid file");
      return false;
    }
    f_lseek(&file, 0);

    if (moduleIdx == EXTERNAL_MODULE) {
      if (!firmwareFile.isMultiExternalFirmware()) {
        f_close(&file);
        POPUP_WARNING(STR_NEEDS_FILE);
        SET_WARNING_INFO(STR_EXT_MULTI_SPEC, strlen(STR_EXT_MULTI_SPEC), 0);
        return false;
      }
    }
    else {
      if (!firmwareFile.isMultiInternalFirmware()) {
        f_close(&file);
        POPUP_WARNING(STR_NEEDS_FILE);
        SET_WARNING_INFO(STR_INT_MULTI_SPEC, strlen(STR_INT_MULTI_SPEC), 0);
        return false;
      }
    }
  }

  const MultiFirmwareUpdateDriver * driver = &multiExternalUpdateDriver;
#if defined(INTERNAL_MODULE_MULTI)
  if (moduleIdx == INTERNAL_MODULE)
    driver = &multiInternalUpdateDriver;
#endif
  if (type == MULTI_TYPE_ELRS)
    driver = &multiExtSportUpdateDriver;

  pausePulses();

#if defined(HARDWARE_INTERNAL_MODULE)
  uint8_t intPwr = IS_INTERNAL_MODULE_ON();
  INTERNAL_MODULE_OFF();
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  uint8_t extPwr = IS_EXTERNAL_MODULE_ON();
  EXTERNAL_MODULE_OFF();
#endif

  uint8_t spuPwr = IS_SPORT_UPDATE_POWER_ON();
  SPORT_UPDATE_POWER_OFF();

  drawProgressScreen(getBasename(filename), STR_DEVICE_RESET, 0, 0);

  /* wait 2s off */
  watchdogSuspend(500 /*5s*/);
  RTOS_WAIT_MS(3000);

  const char * result = driver->flashFirmware(&file, getBasename(filename));
  f_close(&file);

  AUDIO_PLAY(AU_SPECIAL_SOUND_BEEP1);
  BACKLIGHT_ENABLE();

  if (result) {
    POPUP_WARNING(STR_FIRMWARE_UPDATE_ERROR);
    SET_WARNING_INFO(result, strlen(result), 0);
  }
  else {
    POPUP_INFORMATION(STR_FIRMWARE_UPDATE_SUCCESS);
  }

#if defined(HARDWARE_INTERNAL_MODULE)
  INTERNAL_MODULE_OFF();
#endif
  EXTERNAL_MODULE_OFF();
  SPORT_UPDATE_POWER_OFF();

  /* wait 2s off */
  watchdogSuspend(500 /*5s*/);
  RTOS_WAIT_MS(2000);

  // reset telemetry protocol
  telemetryInit(255);

#if defined(HARDWARE_INTERNAL_MODULE)
  if (intPwr) {
    INTERNAL_MODULE_ON();
    setupPulsesInternalModule();
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (extPwr) {
    EXTERNAL_MODULE_ON();
    setupPulsesExternalModule();
  }
#endif

  if (spuPwr) {
    SPORT_UPDATE_POWER_ON();
  }

  resumePulses();

  return result == nullptr;
}
#endif
