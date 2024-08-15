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

#pragma once

#include "hal/serial_port.h"
 
#define HALLSTICK_BUFF_SIZE             ( 512 )
#define FLYSKY_HALL_BAUDRATE            ( 921600 )
#define FLYSKY_HALL_CHANNEL_COUNT       ( 4 )

// This value has been chosen arbitrarily to allow
// for 13-bit precision.
//
// Note: Flysky gimbals provide signed 16-bit values, whereby
//       ADC sampling uses unsigned 16-bit values.
//
#define FLYSKY_OFFSET_VALUE             ( 1 << 12 )

#define FLYSKY_HALL_PROTOLO_HEAD        0x55
#define FLYSKY_HALL_RESP_TYPE_VALUES    0x0c

typedef  struct
{
  signed short channel[4];
  unsigned char  stickState;
  unsigned short CRC16;
} STRUCT_CHANNEL_PACK;

typedef  struct
{
  unsigned char start;
  unsigned char senderID:2;
  unsigned char receiverID:2;
  unsigned char packetID:4;
  unsigned char length;
  STRUCT_CHANNEL_PACK payload;
} STRUCT_HALLDATA;

typedef  struct
{
  unsigned char senderID:2;
  unsigned char receiverID:2;
  unsigned char packetID:4;
} STRUCT_HALLID;

typedef union
{
  STRUCT_HALLID hall_Id;
  unsigned char ID;
} STRUCT_ID;


typedef union
{
  STRUCT_HALLDATA halldat;
  unsigned char buffer[30];
} UNION_HALLDATA;


typedef  struct
{
  unsigned char head;
  STRUCT_ID hallID;
  unsigned char length;
  unsigned char data[HALLSTICK_BUFF_SIZE];
  unsigned char reserved[15];
  unsigned short checkSum;
  unsigned char stickState;
  unsigned char startIndex;
  unsigned char endIndex;
  unsigned char index;
  unsigned char dataIndex;
  unsigned char deindex;
  unsigned char completeFlg;
  unsigned char status;
  unsigned char recevied;
  unsigned char msg_OK;
} STRUCT_HALL;

enum
{
  GET_START = 0,
  GET_ID,
  GET_LENGTH,
  GET_DATA,
  GET_STATE,
  GET_CHECKSUM,
  CHECKSUM,
};

enum HALLSTICK_SEND_STATE_E {
  HALLSTICK_SEND_STATE_IDLE,
  HALLSTICK_STATE_SEND_RESET,
  HALLSTICK_STATE_GET_CONFIG,
  HALLSTICK_STATE_GET_FIRMWARE,
  HALLSTICK_STATE_UPDATE_FW
};

enum TRANSFER_DIR_E {
  TRANSFER_DIR_HALLSTICK,
  TRANSFER_DIR_TXMCU,
  TRANSFER_DIR_HOSTPC,
  TRANSFER_DIR_RFMODULE,
};

extern signed short hall_raw_values[FLYSKY_HALL_CHANNEL_COUNT];
extern unsigned short hall_adc_values[FLYSKY_HALL_CHANNEL_COUNT];

// returns true if the gimbals were detected properly
bool flysky_gimbal_init();

void flysky_gimbal_deinit();
const etx_serial_port_t* flysky_gimbal_get_port();
