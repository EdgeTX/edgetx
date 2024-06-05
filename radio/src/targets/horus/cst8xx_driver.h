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

/*
    Copyright 2016 fishpepper <AT> gmail.com
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
    author: fishpepper <AT> gmail.com
*/

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <touch.h>

/* Set Multi-touch as non supported */
#ifndef TS_MULTI_TOUCH_SUPPORTED
#define TS_MULTI_TOUCH_SUPPORTED 0
#endif

/* Set Auto-calibration as non supported */
#ifndef TS_AUTO_CALIBRATION_SUPPORTED
#define TS_AUTO_CALIBRATION_SUPPORTED 0
#endif

/* Macros --------------------------------------------------------------------*/

/** @typedef ft6x06_handle_TypeDef
 *  ft6x06 Handle definition.
 */
typedef struct {
  /* field holding the current number of simultaneous active touches */
  uint8_t currActiveTouchNb;

  /* field holding the touch index currently managed */
  uint8_t currActiveTouchIdx;

} tc_handle_TypeDef;
/* clang-format off */
#define CST836U_WORK_MODE_REG           0x00
#define CST836U_PROXIMITY_ID_REG        0x01
#define CST836U_TOUCH_NUM_REG           0x02
#define CST836U_TOUCH1_XH_REG           0x03
#define CST836U_TOUCH1_XL_REG           0x04
#define CST836U_TOUCH1_YH_REG           0x05
#define CST836U_TOUCH1_YL_REG           0x06

#define CST836U_TOUCH2_XH_REG           0x09
#define CST836U_TOUCH2_XL_REG           0x0A
#define CST836U_TOUCH2_YH_REG           0x0B
#define CST836U_TOUCH2_YL_REG           0x0C

#define CST836U_FW_VERSION_L_REG        0xA6
#define CST836U_FW_VERSION_H_REG        0xA7
#define CST836U_MODULE_VERSION_REG      0xA8
#define CST836U_PROJECT_NAME_REG        0xA9
#define CST836U_CHIP_TYPE_L_REG         0xAA
#define CST836U_CHIP_TYPE_H_REG         0xAB
#define CST836U_CHECKSUM_L_REG          0xAC
#define CST836U_CHECKSUM_H_REG          0xAD

#define CST836U_PROX_STATE_REG          0xB0
#define CST836U_GES_STATE_REG           0xD0
#define CST836U_GES_ID_REG_REG          0xD3

#define CST836U_MAX_DETECTABLE_TOUCH    2

#define CST836U_MSB_MASK                0x0F

#define CST836U_TOUCH_EVT_FLAG_SHIFT    6
#define CST836U_TOUCH_EVT_FLAG_MASK     (3 << CST836U_TOUCH_EVT_FLAG_SHIFT)
#define CST836U_TOUCH_EVT_FLAG_CONTACT  0x02
/* clang-format on */

extern void TouchInit(void);
extern void TouchDriver(void);

#ifdef __cplusplus
}
#endif
