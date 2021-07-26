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

#ifndef _CRSF_WRITE_H  /* Guard against multiple inclusion */
#define _CRSF_WRITE_H

#include "crsf.h"

/* Provide C++ Compatibility */
#ifdef __cplusplus
extern "C" {
#endif

#ifdef LIBCRSF_ENABLE_COMMAND
/* LIBCRSF_CMD_FRAME********************************************************* */
typedef enum {
  LIBCRSF_FC_CMD                        = 0x01,
  LIBCRSF_BT_CMD                        = 0x03,
  LIBCRSF_OSD_CMD                       = 0x05,
  LIBCRSF_VTX_CMD                       = 0x08,
  LIBCRSF_LED_CMD                       = 0x09,
  LIBCRSF_GENERAL_CMD                   = 0x0A,
  LIBCRSF_RC_RX_CMD                     = 0x10,
  LIBCRSF_WIFI_MODULE                   = 0x12,
  LIBCRSF_ACK                           = 0xFF,
} libCrsfCommands;

typedef enum {
  LIBCRSF_FC_FORCE_DISARM_SUBCMD        = 0x01,
  LIBCRSF_FC_SCALE_CHANNEL_SUBCMD       = 0x02,
} libCrsfFCSubCommands;

typedef enum {
  LIBCRSF_BT_RESET_SUBCMD               = 0x01,
  LIBCRSF_BT_ENABLE_SUBCMD              = 0x02,
  LIBCRSF_BT_ECHO_SUBCMD                = 0x64,
} libCrsfBTSubCommands;

typedef enum {
  LIBCRSF_OSD_SEND_BUTTON_SUBCMD        = 0x01,
} libCrsfOSDSubCommands;

typedef enum {
  LIBCRSF_VTX_CHANGE_CHANNEL_SUBCMD         = 0x01,
  LIBCRSF_VTX_CHANGE_FREQ_SUBCMD            = 0x02,
  LIBCRSF_VTX_CHANGE_POWER_SUBCMD           = 0x03,
  LIBCRSF_VTX_CHANGE_PITMODE_SUBCMD         = 0x04,
  LIBCRSF_VTX_POWER_UP_FROM_PITMODE_SUBCMD  = 0x05,
} libCrsfVTXSubCommands;

typedef enum {
  LIBCRSF_LED_SET_DEFAULT_SUBCMD        = 0x01,
  LIBCRSF_LED_OVERRIDE_COLOR_SUBCMD     = 0x02,
  LIBCRSF_LED_OVERRIDE_PULSE_SUBCMD     = 0x03,
  LIBCRSF_LED_OVERRIDE_BLINK_SUBCMD     = 0x04,
  LIBCRSF_LED_OVERRIDE_SHIFT_SUBCMD     = 0x05,
} libCrsfLEDSubcommands;

typedef enum {
  LIBCRSF_RC_RX_SET_TO_BIND_MODE_SUBCMD         = 0x01,
  LIBCRSF_RC_RX_CANCEL_BIND_MODE_SUBCMD         = 0x02,
  LIBCRSF_RC_RX_MODEL_SELECTION_SUBCMD          = 0x05,
  LIBCRSF_RC_RX_CURRENT_MODEL_SELECTION_SUBCMD  = 0x06,
  LIBCRSF_RC_RX_REPLY_CURRENT_MODEL_SUBCMD      = 0x07,
} libCrsfRCRxSubCommands;

typedef enum {
  LIBCRSF_GENERAL_START_BOOTLOADER_SUBCMD     = 0x0A,
  LIBCRSF_GENERAL_ERASE_MEMORY_SUBCMD         = 0x0B,
  LIBCRSF_GENERALSOFTWARE_PRODUCT_KEY_SUBCMD  = 0x60,
  LIBCRSF_GENERALPRODUCT_FEEDBACK_SUBCMD      = 0x61,
} libCrsfGENERALSubCommands;

typedef enum {
  LIBCRSF_WIFI_FIRMWARE_FILE_URL_SUBCMD       = 0x01,
} libCrsfWIFISubCommands;

typedef struct {
  libCrsfCommands commandId;
  uint8_t subCommandId;
  uint8_t *payload;
} libCrsfCommandS;
#endif

#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
#define LIBCRSF_REMOTE_RELATED_SUBCOMMAND_START 5
#define LIBCRSF_SD_DATA_START                   6
#define LIBCRSF_MAX_SD_PATH_SIZE                53
#define LIBCRSF_MAX_SD_PAYLOAD_START            19
#define LIBCRSF_MAX_SD_PAYLOAD_SIZE             44

typedef enum {
  LIBCRSF_REMOTE_SD_OPEN                    = 0x01,
  LIBCRSF_REMOTE_SD_CLOSE                   = 0x02,
  LIBCRSF_REMOTE_SD_READ_ACCESS             = 0x03,
  LIBCRSF_REMOTE_SD_WRITE_ACCESS            = 0x04,
  LIBCRSF_REMOTE_SD_WRITE_ACK               = 0x05,
  LIBCRSF_REMOTE_SD_ERASE_FILE              = 0x06,
  LIBCRSF_REMOTE_SD_MOUNT_STATUS            = 0x07,
  LIBCRSF_REMOTE_CRSF_TIMING_CORRECTION     = 0x10,
} libCrsfRemoteFrame;

typedef union {
  struct {
    char path[LIBCRSF_MAX_SD_PATH_SIZE];
    uint32_t size;
  } info;
  struct {
    uint32_t addr;
    uint32_t size;
    uint32_t chunkAddr;
    uint8_t isReply;
    uint8_t payload[LIBCRSF_MAX_SD_PAYLOAD_SIZE];
  } data;
  struct {
    uint32_t chunkAddr;
  } ack;
  struct {
    uint8_t isMounted;
  } mountStatus;
  struct {
    uint32_t interval;
    int32_t offset;
  } syncTimeData;
} libCrsfRemoteDataU;
#endif

/* Checking and Setup Function*********************************************** */
bool libCrsfCheckIfDeviceCalled(uint8_t * pArr, bool generalCall);

/* Write Command************************************************************* */
void libCrsfWrite(uint8_t frameType, uint8_t * pArr, ... );

/* Extended Header Frames**************************************************** */
void libCrsfPackPingCommand(uint8_t * pArr, uint32_t * i);
void libCrsfPackDeviceInfo(uint8_t * pArr, uint32_t * i);

#ifdef LIBCRSF_ENABLE_COMMAND
/* CRSF Command************************************************************** */
void libCrsfPackCommandFrame(uint8_t * pArr, uint32_t * i, uint8_t target_address, libCrsfCommandS * command);

void libCrsfPackFcSubCommand(uint8_t * pArr, uint32_t * i, libCrsfFCSubCommands sub_command_id, uint8_t * payload);
void libCrsfPackBtSubCommand(uint8_t * pArr, uint32_t * i, libCrsfBTSubCommands sub_command_id, uint8_t * payload);
void libCrsfPackOsdSubCommand(uint8_t * pArr, uint32_t * i, libCrsfOSDSubCommands sub_command_id, uint8_t * payload);
void libCrsfPackVtxSubCommand(uint8_t * pArr, uint32_t * i, libCrsfVTXSubCommands sub_command_id, uint8_t * payload);
void libCrsfPackLedSubCommand(uint8_t * pArr, uint32_t * i, libCrsfLEDSubcommands sub_command_id, uint8_t * payload);
void libCrsfPackGeneralSubCommand(uint8_t * pArr, uint32_t * i, libCrsfGENERALSubCommands sub_command_id, uint8_t * payload);
void libCrsfPackRcRxSubCommand(uint8_t * pArr, uint32_t * i, libCrsfRCRxSubCommands sub_command_id, uint8_t * payload);
void libCrsfPackWifiSubCommand(uint8_t * pArr, uint32_t * i, libCrsfWIFISubCommands sub_command_id, uint8_t * payload);
void libCrsfPackAckSubCommand(uint8_t * pArr, uint32_t * i, uint8_t * payload);
#endif

#ifdef LIBCRSF_ENABLE_OPENTX_RELATED
void libCrsfPackRemote(uint8_t * pArr, uint32_t * i, uint8_t targetDevice, uint8_t remoteCommandId, libCrsfRemoteDataU * remoteData);
#endif

/* ************************************************************************** */

  /* Provide C++ Compatibility */
#ifdef __cplusplus
}
#endif

#endif /* _CRSF_WRITE_H */

/* *****************************************************************************
 End of File
 */
