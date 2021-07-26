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

#include "crsf.h"
#include "crsf_write.h"
#include "crsf_utilities.h"

uint8_t libCrsfMyparamListSize = 0;
uint8_t libCrsfSenderAddress;

/* Checking and Setup Function*********************************************** */
bool libCrsfCheckIfDeviceCalled(uint8_t * pArr, bool generalCall)
{
  bool feedback = false;

  if ((generalCall
     && *(pArr + LIBCRSF_EXT_HEAD_DST_ADD) == LIBCRSF_BROADCAST_ADD)
     || *(pArr + LIBCRSF_EXT_HEAD_DST_ADD) == libCrsfMySlaveAddress) {
    feedback = true;
    libCrsfSenderAddress = *(pArr + LIBCRSF_EXT_HEAD_ORG_ADD);
  }
  return feedback;
}

/* Write Command************************************************************* */
void libCrsfWrite( uint8_t frameType, uint8_t * pArr, ... )
{
  uint32_t i = 1; /* one byte space for length */

  va_list argp;
  va_start(argp, pArr);

  libUtilWrite8(pArr, &i, frameType);
  switch (frameType) {
    case LIBCRSF_EX_PARAM_PING_DEVICE:
      libCrsfPackPingCommand(pArr, &i);
      break;
    case LIBCRSF_EX_PARAM_DEVICE_INFO:
      libCrsfPackDeviceInfo(pArr, &i);
      break;
#ifdef LIBCRSF_ENABLE_COMMAND
    case LIBCRSF_CMD_FRAME:
      libCrsfPackCommandFrame(pArr, &i
          , (uint8_t) va_arg(argp, int), (libCrsfCommandS *) va_arg(argp, int *));
      break;
#endif
#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
    case LIBCRSF_OPENTX_RELATED:
      libCrsfPackRemote(pArr, &i
              , (uint8_t)va_arg(argp, int), (libCrsfRemoteFrame) va_arg(argp, int), (libCrsfRemoteDataU *) va_arg(argp, int *));
      break;
#endif
    default:
      break;
  }
  va_end(argp);
  libUtilWriteEnd8(pArr, 0, i, POLYNOM_1);
}


/* Extended Header Frames**************************************************** */
void libCrsfPackPingCommand(uint8_t * pArr, uint32_t * i)
{
  libUtilWrite8(pArr, i, LIBCRSF_BROADCAST_ADD);
  libUtilWrite8(pArr, i, libCrsfMySlaveAddress);
}

void libCrsfPackDeviceInfo(uint8_t * pArr, uint32_t * i)
{
  libUtilWrite8(pArr, i, libCrsfSenderAddress);
  libUtilWrite8(pArr, i, libCrsfMySlaveAddress);
  libUtilWriteString(pArr, i, libCrsfMyDeviceName, true);
  libUtilWrite32(pArr, i, libCrsfMySerialNo);
  libUtilWrite32(pArr, i, libCrsfMyHwID);
  libUtilWrite32(pArr, i, libCrsfMyFwID);
  if (libCrsfMyparamListSize <= 2) {
    libUtilWrite8(pArr, i, 0x00);
  }
  else {
    libUtilWrite8(pArr, i, libCrsfMyparamListSize - 2);
  }
  libUtilWrite8(pArr, i, LIBCRSF_PARAM_VERSION_NUMBER);
}

#ifdef LIBCRSF_ENABLE_COMMAND
/* CRSF Command************************************************************** */
void libCrsfPackCommandFrame( uint8_t * pArr, uint32_t * i, uint8_t target_address, libCrsfCommandS * command)
{
  libUtilWrite8( pArr, i, target_address);
  libUtilWrite8( pArr, i, libCrsfMySlaveAddress);
  libUtilWrite8( pArr, i, command->commandId);
  libUtilWrite8( pArr, i, command->subCommandId);
  if (command->payload != NULL) {
    switch(command->commandId) {
      case LIBCRSF_ACK:
        libCrsfPackAckSubCommand(pArr, i, command->payload);
        break;
      default:
        break;
    }
  }
  libUtilWriteEnd8(pArr, 0, *i, POLYNOM_2);
  (*i)++;
}

void libCrsfPackAckSubCommand(uint8_t * pArr, uint32_t * i, uint8_t * payload)
{

}

/* ************************************************************************** */
#endif

#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
/* CRSF OPENTX RELATED************************************************************** */
void libCrsfPackRemote(uint8_t * pArr, uint32_t * i, uint8_t target_device, uint8_t remoteCommandId, libCrsfRemoteDataU * remoteData)
{

  libUtilWrite8(pArr, i, target_device);
  libUtilWrite8(pArr, i, libCrsfMySlaveAddress);
  libUtilWrite8(pArr, i, remoteCommandId);

  switch (remoteCommandId) {
#ifdef LIBCRSF_ENABLE_SD
    case LIBCRSF_REMOTE_SD_OPEN:
      libUtilWriteBytes(pArr, i, (uint8_t*)remoteData->info.path, LIBCRSF_MAX_SD_PATH_SIZE);
      libUtilWrite32(pArr, i, remoteData->info.size);
      break;
    case LIBCRSF_REMOTE_SD_CLOSE:
      break;
    case LIBCRSF_REMOTE_SD_READ_ACCESS:
      libUtilWrite32(pArr, i, remoteData->data.addr);
      libUtilWrite32(pArr, i, remoteData->data.size);
      libUtilWrite32(pArr, i, remoteData->data.chunkAddr);
      libUtilWrite8(pArr, i, remoteData->data.isReply);
      libUtilWriteBytes(pArr, i, remoteData->data.payload, LIBCRSF_MAX_SD_PAYLOAD_SIZE);
      break;
    case LIBCRSF_REMOTE_SD_WRITE_ACCESS:
      break;
    case LIBCRSF_REMOTE_SD_WRITE_ACK:
      libUtilWrite32(pArr, i, remoteData->ack.chunkAddr);
      break;
    case LIBCRSF_REMOTE_SD_ERASE_FILE:
      libUtilWriteBytes(pArr, i, (uint8_t *)remoteData->info.path, LIBCRSF_MAX_SD_PATH_SIZE);
      break;
    case LIBCRSF_REMOTE_SD_MOUNT_STATUS:
      libUtilWrite8(pArr, i, remoteData->mountStatus.isMounted);
      break;
#endif
    default:
      break;
  }
}
#endif

/* *****************************************************************************
 End of File
 */
