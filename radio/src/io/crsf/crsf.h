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

/* USAGE ******************************
  Please Only include crsf.h
  This library will need to include libCrc, libUtil (No need to include crc8.h, utilities.h in your project)

  You may define the following MACROs in your project if needed.
	LIBCRSF_ENABLE_PARAMETER
	LIBCRSF_ENABLE_COMMAND

  LIBCRSF_SYNC_PASS_ONLY
  
**************************************/

#ifndef _CRSF_H  /* Guard against multiple inclusion */
#define _CRSF_H

#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>



#include "crc8.h"

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

/* ************************************************************************** */
#define LIBCRSF_MAX_BUFFER_SIZE             64
#define LIBCRSF_DEVICE_LIST_SIZE            30
#ifdef LIBCRSF_ENABLE_PARAMETER
  #define LIBCRSF_PARAMETER_LIST_SIZE       50
#endif

#define LIBCRSF_PARAM_VERSION_NUMBER        0x01

/* ************************************************************************** */
#define LIBCRSF_HEADER_OFFSET               1
#define LIBCRSF_ADDRESS_ADD                 0
#define LIBCRSF_LENGTH_ADD                  1
#define LIBCRSF_TYPE_ADD                    2
#define LIBCRSF_PAYLOAD_START_ADD           3

#define LIBCRSF_EXT_HEAD_DST_ADD            3  /* extended header destination address address */
#define LIBCRSF_EXT_HEAD_ORG_ADD            4  /* extended header origin address address */
#define LIBCRSF_EXT_PAYLOAD_START_ADD       5
#define LIBCRSF_DEVICE_NAME_START_ADD       LIBCRSF_EXT_PAYLOAD_START_ADD
#define LIBCRSF_PARAMETER_NUMBER_ADD        LIBCRSF_EXT_PAYLOAD_START_ADD
#define LIBCRSF_REMAINING_CHUNK_ADD         6
#define LIBCRSF_PARAMETER_DATA_START_ADD    7
#define LIBCRSF_PARENT_FOLDER_ADD           LIBCRSF_PARAMETER_DATA_START_ADD
#define LIBCRSF_DATA_TYPE_ADD               8
#define LIBCRSF_NAME_START_ADD              9

#define LIBCRSF_CRC_SIZE                    1
#define LIBCRSF_PAYLOAD_SIZE                LIBCRSF_MAX_BUFFER_SIZE


#define LIBCRSF_UART_SYNC                   0xC8
#define LIBCRSF_AGENT_LEGACY_SYNC           0x0A

/* ************************************************************************** */
#define LIBCRSF_PPM_CENTER                  1500

#define LIBCRSF_RC_MAX_NUMBER_OF_CHANNEL    16
#define LIBCRSF_RC_RESOLUTION               11  /* Resolution in bits */
#define LIBCRSF_RC_CENTER                   992
#define LIBCRSF_RC_MAX                      (2 * LIBCRSF_RC_CENTER)
#define LIBCRSF_RC_MIN                      0

#define LIBCRSF_PPM_TO_CRSF_RC(x)           (( x - LIBCRSF_PPM_CENTER) * 8 / 5 + LIBCRSF_RC_CENTER)
#define LIBCRSF_CRSF_TO_PPM_RC(x)           (( x - LIBCRSF_RC_CENTER) * 5 / 8 + LIBCRSF_PPM_CENTER)

/* ************************************************************************** */
typedef enum {
  LIBCRSF_BROADCAST_ADD                     = 0x00,
  LIBCRSF_USB_HOST_ADD                      = 0x10,
  LIBCRSF_WIFI_ADD                          = 0x12,
  LIBCRSF_OSD_ADD                           = 0x80,
  LIBCRSF_CURR_SENS_ADD                     = 0x8A,
  LIBCRSF_TBS_CUR_SENS_ADD                  = 0xC0,
  LIBCRSF_GPS_ADD                           = 0xC2,
  LIBCRSF_BLACKBOX_ADD                      = 0xC4,
  LIBCRSF_FC_ADD                            = LIBCRSF_UART_SYNC,
  LIBCRSF_VTX_ADD                           = 0xCE,
  LIBCRSF_REMOTE_ADD                        = 0xEA,
  LIBCRSF_RC_RX                             = 0xEC,
  LIBCRSF_RC_TX                             = 0xEE
} _libCrsf_DEVICE_ADDRESS;

/* ************************************************************************** */
typedef enum {
  LIBCRSF_BF_RC_CHANNELS_PACK               = 0x16,
  LIBCRSF_EX_PARAM_PING_DEVICE              = 0x28,
  LIBCRSF_EX_PARAM_DEVICE_INFO              = 0x29,
#ifdef LIBCRSF_ENABLE_PARAMETER
  LIBCRSF_EX_PARAM_SETTING_ENTRY            = 0x2B,
  LIBCRSF_EX_PARAM_SETTING_READ             = 0x2C,
  LIBCRSF_EX_PARAM_VALUE_WRITE              = 0x2D,
#endif
#ifdef LIBCRSF_ENABLE_COMMAND
  LIBCRSF_CMD_FRAME                         = 0x32,
#endif
#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
  LIBCRSF_OPENTX_RELATED						        = 0x3A,
#endif

  LIBCRSF_EX_PARAM_LAST_ADDRESS             = 0x96
} libCrsf_FrameTypesNum;

#define LIBCRSF_EXT_HEADER_RANGE_START      LIBCRSF_EX_PARAM_PING_DEVICE
#define LIBCRSF_EXT_HEADER_RANGE_STOP       LIBCRSF_EX_PARAM_LAST_ADDRESS

/* ************************************************************************** */
typedef enum {
  CRSF_PARSE_SYNC,
  CRSF_PARSE_RD_LENGTH,
  CRSF_PARSE_RD_FRAME
} libCrsfParserStatus;

typedef struct {
  libCrsfParserStatus status;
  uint8_t cnt;
  uint32_t tmr;
  uint8_t payload[LIBCRSF_PAYLOAD_SIZE];
} libCrsfParseData;

typedef struct {
  uint8_t portName;
  void (* gateway)(uint8_t * pArr);
  uint8_t deviceList[ LIBCRSF_DEVICE_LIST_SIZE ];
} libCrsfPort;

extern uint8_t  libCrsfMySlaveAddress;
extern char * libCrsfMyDeviceName;
extern uint32_t libCrsfMySerialNo;
extern uint32_t libCrsfMyHwID;
extern uint32_t libCrsfMyFwID;

void libCrsfInit(uint8_t thisDeviceAddress, char * thisDeviceName, uint32_t serialNumber, uint32_t hwId, uint32_t fwId);
void libCrsfAddDeviceFunctionList(libCrsfPort * functionList, uint8_t listSize);
void libCrsfAddRouterFilter(bool (* p_filter_function) (uint8_t Input_Port, uint8_t * pArr));
void libCrsfEnableSyncOnBoradcastAddress(void);

/* ***** Notes ***** */
/* uint8_t *pArr for CRSF_Routing need to point to <Device address or Sync Byte>
 * of the CRSF frame
 * <Device address or Sync Byte> <Frame length> <Type> <Payload> <CRC> */
void libCrsfRouting(uint8_t inputPort, uint8_t * pArr);

bool libCrsfParse(libCrsfParseData * pParseData, uint8_t newData);

  /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _CRSF_H */

/* *****************************************************************************
 End of File
 */
