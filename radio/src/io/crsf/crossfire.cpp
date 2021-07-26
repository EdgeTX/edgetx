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


#include "opentx.h"
#include "stamp.h"

#include "crsf.h"
#include "crossfire.h"
#include "crsf_write.h"
#include "crsf_utilities.h"

extern void crsfToUsbHid(uint8_t * pArr);
extern void usbAgentWrite(uint8_t * pArr);

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

libCrsfPort CrsfPorts[] = {
  {DEVICE_INTERNAL,   &crsfThisDevice},
  {USB_HID,           &crsfToUsbHid},
  {CRSF_SHARED_FIFO,  &crsfToSharedFIFO},
};

uint8_t libCrsfMySlaveAddress = 0;
char * libCrsfMyDeviceName = nullptr;
uint32_t libCrsfMyHwID = 0;
uint32_t libCrsfMySerialNo = 0;
uint32_t libCrsfMyFwID = 0;

uint8_t currentCrsfModelId = 0;

#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
void crsfRemoteRelatedHandler(uint8_t * pArr);
#endif

void crsfPackParam(uint8_t * pArr)
{
  uint32_t count = 0;

  libUtilWrite8(pArr, &count, LIBCRSF_UART_SYNC); /* device address */
  libUtilWrite8(pArr, &count, 0);                 /* frame length */
  libUtilWrite8(pArr, &count, LIBCRSF_EX_PARAM_SETTING_ENTRY); /* cmd type */
  libUtilWrite8(pArr, &count, LIBCRSF_USB_HOST_ADD);     /* Destination Address */
  libUtilWrite8(pArr, &count, LIBCRSF_REMOTE_ADD);/* Origin Address */
  libUtilWrite8(pArr, &count, 0x0);              /* param number */

  uint8_t crc1 = libCRC8GetCRCArr(&pArr[2], count-2, POLYNOM_1);
  libUtilWrite8(pArr, &count, crc1);

  pArr[LIBCRSF_LENGTH_ADD] = count - 2;
}

void crsfInit(void)
{
  uint32_t fw_id;
  uint32_t hw_id;
  uint32_t serial_num;

  memset(&crossfireSharedData, 0, sizeof(CrossfireSharedData));

  fw_id = (VERSION_MAJOR << 8 | (VERSION_MINOR * 16)) + VERSION_REVISION;
  hw_id = readBackupReg(BKREG_HW_ID_RADIO);
  serial_num = readBackupReg(BKREG_SERIAL_NO_RADIO);
  writeBackupReg(BKREG_HW_ID_RADIO, 0);
  writeBackupReg(BKREG_SERIAL_NO_RADIO, 0);

  libCrsfInit(LIBCRSF_REMOTE_ADD, (char *)MY_DEVICE_NAME, serial_num, hw_id, fw_id);

  libCrsfAddDeviceFunctionList( &CrsfPorts[0], ARRAY_SIZE(CrsfPorts));

  crossfireSharedData.rtosApiVersion = RTOS_API_VERSION;

  trampolineInit();
}

void crsfThisDevice(uint8_t * pArr)
{

  uint8_t arr[LIBCRSF_MAX_BUFFER_SIZE];
  uint8_t i = 0;

  /* handle parameter and command frames */
  switch (*(pArr + LIBCRSF_TYPE_ADD))
  {
    case LIBCRSF_EX_PARAM_PING_DEVICE:
      if (libCrsfCheckIfDeviceCalled(pArr, true)) {
        // Parameter_Pack_Device_Information( &arr[LIBCRSF_LENGTH_ADD] );
        libCrsfWrite(LIBCRSF_EX_PARAM_DEVICE_INFO, &arr[LIBCRSF_LENGTH_ADD]);
        libCrsfRouting(DEVICE_INTERNAL, &arr[0]);
      }
      break;

    case LIBCRSF_EX_PARAM_SETTING_READ:
      crsfPackParam(pArr);
      libCrsfRouting(DEVICE_INTERNAL, &pArr[0]);
      break;

#ifdef LIBCRSF_ENABLE_COMMAND
    case LIBCRSF_CMD_FRAME:
      if ((*(pArr + *(pArr + LIBCRSF_LENGTH_ADD) + LIBCRSF_HEADER_OFFSET - 1))
          == libCRC8GetCRCArr(( pArr + LIBCRSF_TYPE_ADD), *(pArr + LIBCRSF_LENGTH_ADD) - 2, POLYNOM_2)) {
        if (*(pArr + LIBCRSF_PAYLOAD_START_ADD + 2) == LIBCRSF_GENERAL_CMD) {
          if (*(pArr + LIBCRSF_PAYLOAD_START_ADD + 3) == LIBCRSF_GENERAL_START_BOOTLOADER_SUBCMD) {
#if defined(RADIO_FAMILY_TBS)
            // avoid updating the screen
            vTaskSuspendAll();
            lcdOn();
            drawDownload();
            storageDirty(EE_GENERAL | EE_MODEL);
            storageCheck(true);
            sdDone();
            volatile uint32_t delay_cnt = get_tmr10ms();
            while (get_tmr10ms() - delay_cnt <= 200);
            boardReboot2bootloader(1, libCrsfMyHwID, libCrsfMySerialNo);
#endif
          }
        }
        else if (*(pArr + LIBCRSF_EXT_PAYLOAD_START_ADD) == LIBCRSF_RC_RX_CMD) {
          if (*(pArr + LIBCRSF_EXT_PAYLOAD_START_ADD + 1) == LIBCRSF_RC_RX_REPLY_CURRENT_MODEL_SUBCMD) {
            currentCrsfModelId = *(pArr + LIBCRSF_EXT_PAYLOAD_START_ADD + 2);
          }
        }
      }
      break;
#endif

#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
    case LIBCRSF_OPENTX_RELATED:
      crsfRemoteRelatedHandler(pArr);
      break;
#endif

    default:
      // Buffer telemetry data inside a FIFO to let telemetryWakeup read from it and keep the
      // compatibility with the existing telemetry infrastructure.
      for (i = 0; i < *(pArr + LIBCRSF_LENGTH_ADD) + 2; i++) {
        intCrsfTelemetryFifo.push(*(pArr + i));
      }
      break;
  }
}

void crsfToSharedFIFO(uint8_t * pArr)
{
  *pArr = LIBCRSF_UART_SYNC;
  for (uint8_t i = 0; i < (*(pArr + LIBCRSF_LENGTH_ADD) + LIBCRSF_HEADER_OFFSET + LIBCRSF_CRC_SIZE); i++) {
    crossfireSharedData.crsf_rx.push(*(pArr + i));
  }
}

void crsfSharedFifoHandler(void)
{
  uint8_t byte;
  static libCrsfParseData crsfData;
  if (crossfireSharedData.crsf_tx.pop(byte)) {
    if (libCrsfParse(&crsfData, byte)) {
      libCrsfRouting(CRSF_SHARED_FIFO, crsfData.payload);
    }
  }
}

void crsfSetModelID(void)
{
  uint32_t count = 0;
  BYTE txBuf[LIBCRSF_MAX_BUFFER_SIZE];

  libUtilWrite8(txBuf, &count, LIBCRSF_UART_SYNC); /* device address */
  libUtilWrite8(txBuf, &count, 0);                 /* frame length */
  libUtilWrite8(txBuf, &count, LIBCRSF_CMD_FRAME); /* cmd type */
  libUtilWrite8(txBuf, &count, LIBCRSF_RC_TX);     /* Destination Address */
  libUtilWrite8(txBuf, &count, LIBCRSF_REMOTE_ADD);/* Origin Address */
  libUtilWrite8(txBuf, &count, LIBCRSF_RC_RX_CMD); /* sub command */
  libUtilWrite8(txBuf, &count, LIBCRSF_RC_RX_MODEL_SELECTION_SUBCMD);    /* command of set model/receiver id */
  libUtilWrite8(txBuf, &count, g_model.header.modelId[INTERNAL_MODULE]); /* model ID */

  uint8_t crc2 = libCRC8GetCRCArr(&txBuf[2], count-2, POLYNOM_2);
  libUtilWrite8(txBuf, &count, crc2);
  uint8_t crc1 = libCRC8GetCRCArr(&txBuf[2], count-2, POLYNOM_1);
  libUtilWrite8(txBuf, &count, crc1);

  txBuf[LIBCRSF_LENGTH_ADD] = count - 2;

  crsfToSharedFIFO(txBuf);
}

void crsfGetModelID(void)
{
  uint32_t count = 0;
  BYTE txBuf[LIBCRSF_MAX_BUFFER_SIZE];

  libUtilWrite8(txBuf, &count, LIBCRSF_UART_SYNC); /* device address */
  libUtilWrite8(txBuf, &count, 0);                 /* frame length */
  libUtilWrite8(txBuf, &count, LIBCRSF_CMD_FRAME); /* cmd type */
  libUtilWrite8(txBuf, &count, LIBCRSF_RC_TX);     /* Destination Address */
  libUtilWrite8(txBuf, &count, LIBCRSF_REMOTE_ADD);/* Origin Address */
  libUtilWrite8(txBuf, &count, LIBCRSF_RC_RX_CMD); /* sub command */
  libUtilWrite8(txBuf, &count, LIBCRSF_RC_RX_CURRENT_MODEL_SELECTION_SUBCMD);  /* command of set model/receiver id */
  libUtilWrite8(txBuf, &count, 0);                 /* the dummy byte of model ID */

  uint8_t crc2 = libCRC8GetCRCArr(&txBuf[2], count-2, POLYNOM_2);
  libUtilWrite8(txBuf, &count, crc2);
  uint8_t crc1 = libCRC8GetCRCArr(&txBuf[2], count-2, POLYNOM_1);
  libUtilWrite8(txBuf, &count, crc1);

  txBuf[LIBCRSF_LENGTH_ADD] = count - 2;

  crsfToSharedFIFO(txBuf);
}

uint32_t crsfGetHWID(void)
{
  return libCrsfMyHwID;
}

void updateIntCrossfireChannels(void)
{
  uint8_t i;
  for (i = 0; i < CROSSFIRE_CHANNELS_COUNT; ++i)
    crossfireSharedData.channels[i] = channelOutputs[i];
}

#if !defined(SIMU)
uint32_t readBackupReg(uint8_t index) {
  return *(__IO uint32_t *) (BKPSRAM_BASE + index * 4);
}

void writeBackupReg(uint8_t index, uint32_t data)
{
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
  PWR_BackupRegulatorCmd(ENABLE);
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_BKPSRAM, ENABLE);
  PWR_BackupAccessCmd(ENABLE);
  while (PWR_GetFlagStatus(PWR_FLAG_BRR) == RESET);
  *(__IO uint32_t *) (BKPSRAM_BASE + index*4) = data;
}

uint8_t bkregGetStatusFlag(uint32_t flag)
{
  uint32_t value = (uint32_t)readBackupReg(BKREG_STATUS_FLAG);
  return ((value & (1 << flag)) ? 1 : 0);
}

void bkregSetStatusFlag(uint32_t flag)
{
  uint32_t value = (uint32_t)readBackupReg(BKREG_STATUS_FLAG);
  value |= (1 << flag);
  writeBackupReg(BKREG_STATUS_FLAG, value);
}

void bkregClrStatusFlag(uint32_t flag)
{
  uint32_t value = (uint32_t)readBackupReg(BKREG_STATUS_FLAG);
  value &= ~(1 << flag);
  writeBackupReg(BKREG_STATUS_FLAG, value);
}

void boardSetSkipWarning()
{
  bkregSetStatusFlag(DEVICE_RESTART_WITHOUT_WARN_FLAG);
}

void crossfirePowerOff()
{
  volatile uint32_t offTimeout;
  if (isCrossfireRfOn()) {
    setCrsfFlag(CRSF_FLAG_POWER_OFF);
    offTimeout = get_tmr10ms();
    while (getCrsfFlag(CRSF_FLAG_POWER_OFF)) {
      WDG_RESET();
      if (get_tmr10ms() - offTimeout >= 200)
        break;
    }
  }
}

bool isCrossfireRfOn()
{
  return !getCrsfFlag(CRSF_FLAG_RF_OFF);
}

void crossfireTurnOffRf(bool ask)
{
  while (!getCrsfFlag(CRSF_FLAG_RF_OFF)) {
    if (ask) {
      lcdRefreshWait();
      lcdClear();
      POPUP_CONFIRMATION(STR_JOYSTICK_RF, nullptr);
      SET_WARNING_INFO(STR_TURN_OFF_RF, sizeof(TR_TURN_OFF_RF), 0);
      event_t evt = getEvent(false);
      DISPLAY_WARNING(evt);
      lcdRefresh();
    }
    WDG_RESET();

    if (warningResult || !ask) {
      warningResult = 0;
      setCrsfFlag(CRSF_FLAG_RF_OFF);
      break;
    }
    else if (!warningText) {
      break;
    }
  }
}

void crossfireTurnOnRf()
{
  clearCrsfFlag(CRSF_FLAG_RF_OFF);
}
#endif

#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
void libCrsfUnpackRemote(uint8_t * pArr, libCrsfRemoteDataU * remoteData)
{
  uint32_t j = LIBCRSF_EXT_PAYLOAD_START_ADD;
  uint32_t *i = &j;
  libCrsfRemoteFrame remote_command_id = (libCrsfRemoteFrame)libUtilRead8(pArr, i);

  switch (remote_command_id) {
#ifdef LIBCRSF_ENABLE_SD
    case LIBCRSF_REMOTE_SD_OPEN:
      for (uint8_t j = 0; j < LIBCRSF_MAX_SD_PATH_SIZE; j++) {
        remoteData->info.path[j] = libUtilReadInt8(pArr, i);
      }
      remoteData->info.size = libUtilReadInt32(pArr, i);
      break;
    case LIBCRSF_REMOTE_SD_CLOSE:
      break;
    case LIBCRSF_REMOTE_SD_READ_ACCESS:
      remoteData->data.addr = libUtilReadInt32(pArr, i);
      remoteData->data.size = libUtilReadInt32(pArr, i);
      remoteData->data.chunkAddr = libUtilReadInt32(pArr, i);
      remoteData->data.isReply = libUtilReadInt8(pArr, i);
      break;
    case LIBCRSF_REMOTE_SD_WRITE_ACCESS:
      remoteData->data.addr = libUtilReadInt32(pArr, i);
      remoteData->data.size = libUtilReadInt32(pArr, i);
      remoteData->data.chunkAddr = libUtilReadInt32(pArr, i);
      remoteData->data.isReply = libUtilReadInt8(pArr, i);
      for (uint8_t j = 0; j < LIBCRSF_MAX_SD_PAYLOAD_SIZE; j++) {
        remoteData->data.payload[j] = libUtilReadInt8(pArr, i);
      }
      break;
    case LIBCRSF_REMOTE_SD_WRITE_ACK:
      break;
    case LIBCRSF_REMOTE_SD_ERASE_FILE:
      for (uint8_t j = 0; j < LIBCRSF_MAX_SD_PATH_SIZE; j++) {
        remoteData->info.path[j] = libUtilReadInt8(pArr, i);
      }
      break;
    case LIBCRSF_REMOTE_SD_MOUNT_STATUS:
      remoteData->mountStatus.isMounted = libUtilReadInt8(pArr, i);
      break;
#endif // LIBCRSF_ENABLE_SD
    default:
      break;
  }
}

void crsfRemoteRelatedHandler(uint8_t * pArr)
{

#ifdef LIBCRSF_ENABLE_SD
  static FIL file;
  static char filePath[LIBCRSF_MAX_SD_PATH_SIZE];
  static uint32_t offset = 0;
  static uint32_t fileSize = 0;

  FRESULT result;
  static libCrsfRemoteDataU data;
  libCrsfUnpackRemote(pArr, &data);
  static libCrsfRemoteDataU replyData;
#endif

  switch (pArr[LIBCRSF_EXT_PAYLOAD_START_ADD])
  {
#ifdef LIBCRSF_ENABLE_SD
    case LIBCRSF_REMOTE_SD_OPEN:
    {
      // if opened, close it first
      if (file.obj.fs != NULL) {
        result = f_close(&file);
        CRSF_SD_PRINTF("open:close\r\n");
        if (result != FR_OK) {
          CRSF_SD_PRINTF("open:close failed\r\n");
          return;
        }
      }

      // create path if does not exist
      char str[LIBCRSF_MAX_SD_PATH_SIZE];
      strcpy(filePath, (const char*)data.info.path);
      strcpy(str, (const char*)data.info.path);
      uint8_t dirCount = 0;
      char *dir = strtok(str, "/");
      while (dir != NULL) {
        dirCount++;
        dir = strtok(NULL, "/");
      }
      char path[LIBCRSF_MAX_SD_PATH_SIZE];
      memset(path, 0, LIBCRSF_MAX_SD_PATH_SIZE);
      strcpy(str, (const char *)data.info.path);
      dir = strtok(str, "/");
      for (uint8_t i = 0; i < dirCount - 1; i++) {
        strcat(path, "/");
        strcat(path, dir);
        result = f_mkdir(path);
        dir = strtok(NULL, "/");
      }

      result = f_open(&file, data.info.path, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
      if(result == FR_OK) {
        FILINFO info;
        result = f_stat(data.info.path, &info);
        if (result == FR_OK) {
          memcpy(&replyData.info.path, data.info.path, LIBCRSF_MAX_SD_PATH_SIZE);
          fileSize = replyData.info.size = info.fsize;
          libCrsfWrite( LIBCRSF_OPENTX_RELATED, &pArr[LIBCRSF_LENGTH_ADD], pArr[LIBCRSF_EXT_HEAD_ORG_ADD], LIBCRSF_REMOTE_SD_OPEN, &replyData);
          libCrsfRouting(DEVICE_INTERNAL, &pArr[0]);
          offset = 0;
          CRSF_SD_PRINTF("%s opened, fileLen: %ld\r\n", reply_data.info.path, reply_data.info.size);
        }
      }
      else{
        CRSF_SD_PRINTF("open:failed\r\n");
      }
      break;
    }
    case LIBCRSF_REMOTE_SD_CLOSE:
      if (file.obj.fs != NULL) {
        result = f_close(&file);
        if (result == FR_OK) {
          file.obj.fs = 0;
          CRSF_SD_PRINTF("%s closed\r\n", filePath);
        }
      }
      break;
    case LIBCRSF_REMOTE_SD_READ_ACCESS:
      // check if the range to read is valided
      if (fileSize >= (data.data.addr + data.data.size)) {
        // check if the address is previous
        if (offset > data.data.chunkAddr) {
          CRSF_SD_PRINTF( "read:addr:chunk_addr: %ld ofs: %ld\r\n", data.data.chunk_addr, offset );
          // re-open file to load the previous address
          if (file.obj.fs != NULL) {
            result = f_close(&file);
            if (result == FR_OK ) {
              CRSF_SD_PRINTF("read:close\r\n");
              file.obj.fs = 0;
            }
            else{
              CRSF_SD_PRINTF("read:close failed\r\n");
              return;
            }
          }
        }

        if (file.obj.fs == NULL) {
          result = f_open(&file, filePath, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
          if (result == FR_OK) {
            CRSF_SD_PRINTF("read:re-open\r\n");
            offset = 0;
          }
          else {
            CRSF_SD_PRINTF("read:re-open failed\r\n");
            return;
          }
        }

        // seek the specific address
        if (data.data.chunkAddr > offset) {
          result = f_lseek(&file, data.data.chunkAddr);
          if (result == FR_OK) {
            CRSF_SD_PRINTF("read:lseek: %ld\r\n", data.data.chunk_addr);
            offset = data.data.chunkAddr;
          }
          else {
            CRSF_SD_PRINTF("read:lseek failed\r\n");
            return;
          }
        }

        UINT byteRead;
        UINT size = (data.data.size - data.data.chunkAddr + data.data.addr) >= LIBCRSF_MAX_SD_PAYLOAD_SIZE ? LIBCRSF_MAX_SD_PAYLOAD_SIZE : (data.data.size - data.data.chunkAddr + data.data.addr);
        result = f_read(&file, replyData.data.payload, size, &byteRead);
        if (result == FR_OK) {
          CRSF_SD_PRINTF( "read: ofs: %ld size: %ld data_size: %ld ofs - addr: %ld\r\n", offset, size, data.data.size, offset - data.data.addr );
          replyData.data.addr = data.data.addr;
          replyData.data.size = data.data.size;
          replyData.data.chunkAddr = data.data.chunkAddr;
          offset += byteRead;
          replyData.data.isReply = 1;
          libCrsfWrite(LIBCRSF_OPENTX_RELATED, &pArr[LIBCRSF_LENGTH_ADD], pArr[LIBCRSF_EXT_HEAD_ORG_ADD], LIBCRSF_REMOTE_SD_READ_ACCESS, &replyData );
          libCrsfRouting(DEVICE_INTERNAL, &pArr[0]);
          CRSF_SD_PRINTF("read: ofs_inc: %ld\r\n", offset);
        }
        else {
          CRSF_SD_PRINTF( "read:falied\r\n");
          return;
        }
      }
      break;
    case LIBCRSF_REMOTE_SD_WRITE_ACCESS:
      // check if the range to write is valided
      if (data.data.size > 0) {
        // check if the address is previous
        if (offset > data.data.chunkAddr) {
          // re-open file to load the previous address
          if (file.obj.fs != NULL) {
            result = f_close(&file);
            CRSF_SD_PRINTF("write:close\r\n");
            if (result == FR_OK) {
              CRSF_SD_PRINTF("write:close\r\n");
              file.obj.fs = 0;
            }
            else{
              CRSF_SD_PRINTF("write:close failed\r\n");
              return;
            }
          }
        }

        if (file.obj.fs == NULL) {
          result = f_open(&file, filePath, FA_OPEN_ALWAYS | FA_WRITE | FA_READ);
          if (result == FR_OK) {
            CRSF_SD_PRINTF( "write:re-open\r\n");
            offset = 0;
          }
          else {
            CRSF_SD_PRINTF( "write:re-open failed\r\n");
            return;
          }
        }

        // seek the specific address
        if (data.data.chunkAddr > offset) {
          result = f_lseek(&file, data.data.chunkAddr);
          if (result == FR_OK) {
            offset = data.data.chunkAddr;
            CRSF_SD_PRINTF( "write:lseek: %ld\r\n", data.data.chunk_addr);
          }
          else {
            CRSF_SD_PRINTF( "write:lseek failed\r\n");
            return;
          }
        }

        UINT byteWrite;
        UINT size = (data.data.size - data.data.chunkAddr + data.data.addr) >= LIBCRSF_MAX_SD_PAYLOAD_SIZE ? LIBCRSF_MAX_SD_PAYLOAD_SIZE : (data.data.size - data.data.chunkAddr + data.data.addr);
        result = f_write(&file, data.data.payload, size, &byteWrite);
        if (result == FR_OK) {
          CRSF_SD_PRINTF( "\r\nwrite:bytes: ofs:%ld size:%ld\r\n", offset, size);
          for (uint8_t i = 0; i < byteWrite; i++) {
            CRSF_SD_PRINTF("%02X ", data.data.payload[i]);
          }
          CRSF_SD_PRINTF("\r\n");
          CRSF_SD_PRINTF("write: ofs: %ld\r\n", offset);
          replyData.ack.chunkAddr = data.data.chunkAddr;
          offset += byteWrite;
          libCrsfWrite(LIBCRSF_OPENTX_RELATED, &pArr[LIBCRSF_LENGTH_ADD], pArr[LIBCRSF_EXT_HEAD_ORG_ADD], LIBCRSF_REMOTE_SD_WRITE_ACK, &replyData);
          libCrsfRouting(DEVICE_INTERNAL, &pArr[0]);
          CRSF_SD_PRINTF("write: ofs_inc: %ld\r\n", offset);
        }
        else {
          CRSF_SD_PRINTF("write:falied\r\n");
          return;
        }
      }
      break;
    case LIBCRSF_REMOTE_SD_WRITE_ACK:
      break;
    case LIBCRSF_REMOTE_SD_ERASE_FILE:
      result = f_unlink(data.info.path);
      if (result == FR_OK) {
        CRSF_SD_PRINTF("erase:success: %s\r\n", data.info.path);
      }
      else {
        CRSF_SD_PRINTF("erase:falied\r\n");
      }
      break;
    case LIBCRSF_REMOTE_SD_MOUNT_STATUS:
      replyData.mountStatus.isMounted = (uint8_t)sdMounted();
      libCrsfWrite(LIBCRSF_OPENTX_RELATED, &pArr[LIBCRSF_LENGTH_ADD], pArr[LIBCRSF_EXT_HEAD_ORG_ADD], LIBCRSF_REMOTE_SD_MOUNT_STATUS, &replyData);
      libCrsfRouting(DEVICE_INTERNAL, &pArr[0]);
      CRSF_SD_PRINTF("mount:%d\r\n", reply_data.mount_status.is_mounted);
      break;
#endif
    default:
      break;
  }

  RTOS_GIVE_SEM(get_task_sem(CRSF_SD_TASK_SEM));
}
#endif // LIBCRSF_ENABLE_OPENTX_RELATED
