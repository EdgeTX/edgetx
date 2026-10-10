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

#include "touch.h"
#include "definitions.h"

#define HAS_TOUCH_PANEL()     (touchGT911Flag == true)

extern bool touchGT911Flag;
extern uint16_t touchGT911fwver;
extern uint32_t touchGT911hiccups;

extern void touchPanelDeInit();
extern bool touchPanelInit();

struct TouchState touchPanelRead();
bool touchPanelEventOccured();
struct TouchState getInternalTouchState();

#define GT911_TIMEOUT           3 // 3ms
#define GT911_TAP_TIME          250 // 250 ms

#define GT911_MAX_TP            5

#define GT911_CFG_NUMBER        0x6D

//I2C
#define GT911_I2C_ADDR          0x14

//#define GT_CMD_WR             0x28
//#define GT_CMD_RD             0x29
//#define I2C_TIMEOUT_MAX       1000

#define GT911_COMMAND_REG               0x8040
#define GT911_CONFIG_REG                0x8047
#define GT911_COORDINATE_REG            0x804D
#define GT911_CONFIG_CHECKSUM_REG       0x80FF

#define GT911_PRODUCT_ID_REG            0x8140
#define GT911_FIRMWARE_VERSION_REG      0x8144
#define GT911_VENDOR_ID_REG             0x814A
#define GT911_READ_XY_REG               0x814E
#define GT911_TP1_REG                   0x8150
#define GT911_TP2_REG                   0x8158
#define GT911_TP3_REG                   0x8160
#define GT911_TP4_REG                   0x8168
#define GT911_TP5_REG                   0x8170

PACK(typedef struct {
  uint8_t track;
  uint16_t x;
  uint16_t y;
  uint16_t size;
  uint8_t reserved;
}) TouchPoint;

PACK(struct TouchData {
  union
  {
    TouchPoint points[GT911_MAX_TP];
    uint8_t data[GT911_MAX_TP * sizeof(TouchPoint)];
  };
});
