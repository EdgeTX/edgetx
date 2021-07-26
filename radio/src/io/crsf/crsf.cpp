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

#define LIBCRSF_DEVICE_LIST_ENTRY_EMPTY  0

libCrsfPort   *libCrsfPorts;
unsigned int  libCrsfPortsSize = 0;
bool (* libCrsfFilterFunction)(uint8_t inputPort, uint8_t * pArr);
static bool libCrsfSyncOnBoradcastAddress = false;

void libCrsfInit(uint8_t thisDeviceAddress, char * thisDeviceName, uint32_t serialNumber, uint32_t hwId, uint32_t fwId)
{
  libCrsfMySlaveAddress = thisDeviceAddress;
  libCrsfMyDeviceName = thisDeviceName;
  libCrsfMySerialNo = serialNumber;
  libCrsfMyHwID = hwId;
  libCrsfMyFwID = fwId;
}

void libCrsfAddDeviceFunctionList(libCrsfPort * functionList, uint8_t listSize)
{
  libCrsfPorts = functionList;
  libCrsfPortsSize = listSize;
}

void libCrsfAddRouterFilter(bool (* pFilterFunction) (uint8_t inputPort, uint8_t * pArr))
{
  libCrsfFilterFunction = pFilterFunction;
}

void libCrsfEnableSyncOnBoradcastAddress(void)
{
  libCrsfSyncOnBoradcastAddress = true;
}

void libCrsfRouting(uint8_t inputPort, uint8_t * pArr)
{
  uint8_t portCnt;
  uint8_t deviceBuffCnt;
  uint8_t routeToPort = 0;
  bool deviceFound = false;

  /* call filter function and return if filter function triggers */
  if (libCrsfFilterFunction != NULL && libCrsfFilterFunction(inputPort, pArr)) {
    return;
  }

  if (*(pArr + LIBCRSF_TYPE_ADD) >= LIBCRSF_EXT_HEADER_RANGE_START
      && *(pArr + LIBCRSF_TYPE_ADD) < LIBCRSF_EXT_HEADER_RANGE_STOP) {
    for (portCnt = 0; portCnt < libCrsfPortsSize; portCnt++) {
      if (inputPort == libCrsfPorts[portCnt].portName) {
        /* store source device address for future routing */
        for (deviceBuffCnt = 0; deviceBuffCnt < LIBCRSF_DEVICE_LIST_SIZE; deviceBuffCnt++) {
          if (libCrsfPorts[portCnt].deviceList[deviceBuffCnt] == *(pArr + LIBCRSF_EXT_HEAD_ORG_ADD)) {
            break;   /* device already listed */
          }

          if (libCrsfPorts[portCnt].deviceList[deviceBuffCnt] == LIBCRSF_DEVICE_LIST_ENTRY_EMPTY) {
            libCrsfPorts[portCnt].deviceList[deviceBuffCnt] = *(pArr + LIBCRSF_EXT_HEAD_ORG_ADD); /* new device found */
            break;
          }
        }
      } else if (*(pArr + LIBCRSF_EXT_HEAD_DST_ADD) != LIBCRSF_BROADCAST_ADD) {
        /* if frame is not sent to broadcast address try to route it */
        for (deviceBuffCnt = 0; deviceBuffCnt < LIBCRSF_DEVICE_LIST_SIZE; deviceBuffCnt++) {
          if (libCrsfPorts[portCnt].deviceList[deviceBuffCnt] == LIBCRSF_BROADCAST_ADD) {
            /* area we are searching never got written so we abort searching */
            break;
          }
          if (libCrsfPorts[portCnt].deviceList[deviceBuffCnt] == *(pArr + LIBCRSF_EXT_HEAD_DST_ADD)) {
            deviceFound = true;
            routeToPort = portCnt;
          }
        }
      }
    }
  }

  /* distribute frame */
  if (deviceFound) {
    if (libCrsfPorts[routeToPort].gateway != NULL) {
      libCrsfPorts[routeToPort].gateway(pArr);
    }
  } else {
    for (portCnt = 0; portCnt < libCrsfPortsSize; portCnt++) {
      if (inputPort != libCrsfPorts[portCnt].portName
          && libCrsfPorts[portCnt].gateway != NULL) {
        libCrsfPorts[portCnt].gateway(pArr);
      }
    }
  }
}

bool libCrsfParse(libCrsfParseData * pParseData, uint8_t newData)
{
  switch (pParseData->status) {
    default:
    case CRSF_PARSE_SYNC:
      pParseData->payload[LIBCRSF_ADDRESS_ADD] = newData;
#ifdef LIBCRSF_SYNC_PASS_ONLY
      if( pParse_Data->Payload[ LIBCRSF_ADDRESS_ADD ] == LIBCRSF_UART_SYNC ) {
#else
      if ((pParseData->payload[LIBCRSF_ADDRESS_ADD] == LIBCRSF_BROADCAST_ADD
            && libCrsfSyncOnBoradcastAddress)
         || pParseData->payload[LIBCRSF_ADDRESS_ADD] == libCrsfMySlaveAddress
         || pParseData->payload[LIBCRSF_ADDRESS_ADD] == LIBCRSF_UART_SYNC) {
#endif
        pParseData->cnt = LIBCRSF_HEADER_OFFSET + LIBCRSF_HEADER_OFFSET;
        pParseData->status = CRSF_PARSE_RD_LENGTH;
      }
      break;

    case CRSF_PARSE_RD_LENGTH:
      pParseData->payload[LIBCRSF_LENGTH_ADD] = newData;
      if (pParseData->payload[LIBCRSF_LENGTH_ADD] < LIBCRSF_PAYLOAD_SIZE
          && pParseData->payload[LIBCRSF_LENGTH_ADD] != 0) {
        pParseData->status = CRSF_PARSE_RD_FRAME;
      } else {
        pParseData->status = CRSF_PARSE_SYNC;
      }
      break;

    case CRSF_PARSE_RD_FRAME:
      if (pParseData->cnt < LIBCRSF_PAYLOAD_SIZE) {
        pParseData->payload[pParseData->cnt++] = newData;

        if (pParseData->cnt > pParseData->payload[LIBCRSF_LENGTH_ADD] + LIBCRSF_HEADER_OFFSET) {
          if (pParseData->payload[pParseData->payload[LIBCRSF_LENGTH_ADD] + LIBCRSF_HEADER_OFFSET]
              == libCRC8GetCRCArr(&pParseData->payload[LIBCRSF_TYPE_ADD]
              ,pParseData->payload[LIBCRSF_LENGTH_ADD] - LIBCRSF_CRC_SIZE, POLYNOM_1)) {
            pParseData->status = CRSF_PARSE_SYNC;
            return true;
          }
          pParseData->status = CRSF_PARSE_SYNC;
        }
      } else {
        pParseData->status = CRSF_PARSE_SYNC;
      }
      break;
  }
  return  false;
}

/* *****************************************************************************
 End of File
 */
