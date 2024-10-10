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

/**
 ******************************************************************************
 * @file    ft6x06.h
 * @author  MCD Application Team
  * @version V1.0.0
  * @date    03-August-2015
 * @brief   This file contains all the functions prototypes for the
 *          ft6x06.c IO expander driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
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
typedef struct
{
  /* field holding the current number of simultaneous active touches */
  uint8_t currActiveTouchNb;

  /* field holding the touch index currently managed */
  uint8_t currActiveTouchIdx;

} tc_handle_TypeDef;



#define TOUCH_FT6236_MAX_TOUCH_POINTS     2

#define TOUCH_FT6236_REG_TH_GROUP         0x80
#define TOUCH_FT6236_REG_PERIODACTIVE     0x88
#define TOUCH_FT6236_REG_LIB_VER_H        0xa1
#define TOUCH_FT6236_REG_LIB_VER_L        0xa2
#define TOUCH_FT6236_REG_CIPHER           0xa3
#define TOUCH_FT6236_REG_FIRMID           0xa6
#define TOUCH_FT6236_REG_FOCALTECH_ID     0xa8
#define TOUCH_FT6236_REG_RELEASE_CODE_ID  0xaf

#define TOUCH_FT6236_EVENT_PRESS_DOWN     0
#define TOUCH_FT6236_EVENT_LIFT_UP        1
#define TOUCH_FT6236_EVENT_CONTACT        2
#define TOUCH_FT6236_EVENT_NO_EVENT       3

#define TOUCH_FT6236_GESTURE_MOVE_FLAG   0x10
#define TOUCH_FT6236_GESTURE_MOVE_UP     0x10
#define TOUCH_FT6236_GESTURE_MOVE_RIGHT  0x14
#define TOUCH_FT6236_GESTURE_MOVE_DOWN   0x18
#define TOUCH_FT6236_GESTURE_MOVE_LEFT   0x1C
#define TOUCH_FT6236_GESTURE_ZOOM_IN     0x48
#define TOUCH_FT6236_GESTURE_ZOOM_OUT    0x49
#define TOUCH_FT6236_GESTURE_NONE        0x00

#define TOUCH_GESTURE_UP    ((TOUCH_FT6236_GESTURE_MOVE_UP    & 0x0F)+1)
#define TOUCH_GESTURE_DOWN  ((TOUCH_FT6236_GESTURE_MOVE_DOWN  & 0x0F)+1)
#define TOUCH_GESTURE_LEFT  ((TOUCH_FT6236_GESTURE_MOVE_LEFT  & 0x0F)+1)
#define TOUCH_GESTURE_RIGHT ((TOUCH_FT6236_GESTURE_MOVE_RIGHT & 0x0F)+1)
#define TOUCH_GESTURE_MOUSE_DOWN (0x80+TOUCH_FT6236_EVENT_PRESS_DOWN)
#define TOUCH_GESTURE_MOUSE_UP   (0x80+TOUCH_FT6236_EVENT_LIFT_UP)
#define TOUCH_GESTURE_MOUSE_MOVE (0x80+TOUCH_FT6236_EVENT_CONTACT)
#define TOUCH_GESTURE_MOUSE_NONE (0x80+TOUCH_FT6236_EVENT_NO_EVENT)



  /* Maximum border values of the touchscreen pad */
#define  FT_6206_MAX_WIDTH              ((uint16_t)800)     /* Touchscreen pad max width   */
#define  FT_6206_MAX_HEIGHT             ((uint16_t)480)     /* Touchscreen pad max height  */

  /* Possible values of driver functions return status */
#define FT6206_STATUS_OK                0
#define FT6206_STATUS_NOT_OK            1

  /* Max detectable simultaneous touches */
#define FT6206_MAX_DETECTABLE_TOUCH     2

  /**
   * @brief : Definitions for FT6206 I2C register addresses on 8 bit
   **/

  /* Current mode register of the FT6206 (R/W) */
#define FT6206_DEV_MODE_REG             0x00

  /* Possible values of FT6206_DEV_MODE_REG */
#define FT6206_DEV_MODE_WORKING         0x00
#define FT6206_DEV_MODE_FACTORY         0x04

#define FT6206_DEV_MODE_MASK            0x7
#define FT6206_DEV_MODE_SHIFT           4

  /* Gesture ID register */
#define FT6206_GEST_ID_REG              0x01

  /* Possible values of FT6206_GEST_ID_REG */
#define FT6206_GEST_ID_NO_GESTURE       0x00
#define FT6206_GEST_ID_MOVE_UP          0x10
#define FT6206_GEST_ID_MOVE_RIGHT       0x14
#define FT6206_GEST_ID_MOVE_DOWN        0x18
#define FT6206_GEST_ID_MOVE_LEFT        0x1C
#define FT6206_GEST_ID_ZOOM_IN          0x48
#define FT6206_GEST_ID_ZOOM_OUT         0x49

  /* Touch Data Status register : gives number of active touch points (0..2) */
#define FT6206_TD_STAT_REG              0x02

  /* Values related to FT6206_TD_STAT_REG */
#define FT6206_TD_STAT_MASK             0x0F
#define FT6206_TD_STAT_SHIFT            0x00

  /* Values Pn_XH and Pn_YH related */
#define FT6206_TOUCH_EVT_FLAG_PRESS_DOWN 0x00
#define FT6206_TOUCH_EVT_FLAG_LIFT_UP    0x01
#define FT6206_TOUCH_EVT_FLAG_CONTACT    0x02
#define FT6206_TOUCH_EVT_FLAG_NO_EVENT   0x03

#define FT6206_TOUCH_EVT_FLAG_SHIFT     6
#define FT6206_TOUCH_EVT_FLAG_MASK      (3 << FT6206_TOUCH_EVT_FLAG_SHIFT)

#define FT6206_MSB_MASK                 0x0F
#define FT6206_MSB_SHIFT                0

  /* Values Pn_XL and Pn_YL related */
#define FT6206_LSB_MASK                 0xFF
#define FT6206_LSB_SHIFT                0

#define FT6206_P1_XH_REG                0x03
#define FT6206_P1_XL_REG                0x04
#define FT6206_P1_YH_REG                0x05
#define FT6206_P1_YL_REG                0x06

  /* Touch Pressure register value (R) */
#define FT6206_P1_WEIGHT_REG            0x07

  /* Values Pn_WEIGHT related  */
#define FT6206_TOUCH_WEIGHT_MASK        0xFF
#define FT6206_TOUCH_WEIGHT_SHIFT       0

  /* Touch area register */
#define FT6206_P1_MISC_REG              0x08

  /* Values related to FT6206_Pn_MISC_REG */
#define FT6206_TOUCH_AREA_MASK         (0x04 << 4)
#define FT6206_TOUCH_AREA_SHIFT        0x04

#define FT6206_P2_XH_REG               0x09
#define FT6206_P2_XL_REG               0x0A
#define FT6206_P2_YH_REG               0x0B
#define FT6206_P2_YL_REG               0x0C
#define FT6206_P2_WEIGHT_REG           0x0D
#define FT6206_P2_MISC_REG             0x0E

  /* Threshold for touch detection */
#define FT6206_TH_GROUP_REG            0x80

  /* Values FT6206_TH_GROUP_REG : threshold related  */
#define FT6206_THRESHOLD_MASK          0xFF
#define FT6206_THRESHOLD_SHIFT         0

  /* Filter function coefficients */
#define FT6206_TH_DIFF_REG             0x85

  /* Control register */
#define FT6206_CTRL_REG                0x86

  /* Values related to FT6206_CTRL_REG */

  /* Will keep the Active mode when there is no touching */
#define FT6206_CTRL_KEEP_ACTIVE_MODE    0x00

  /* Switching from Active mode to Monitor mode automatically when there is no touching */
#define FT6206_CTRL_KEEP_AUTO_SWITCH_MONITOR_MODE  0x01

  /* The time period of switching from Active mode to Monitor mode when there is no touching */
#define FT6206_TIMEENTERMONITOR_REG     0x87

  /* Report rate in Active mode */
#define FT6206_PERIODACTIVE_REG         0x88

  /* Report rate in Monitor mode */
#define FT6206_PERIODMONITOR_REG        0x89

  /* The value of the minimum allowed angle while Rotating gesture mode */
#define FT6206_RADIAN_VALUE_REG         0x91

  /* Maximum offset while Moving Left and Moving Right gesture */
#define FT6206_OFFSET_LEFT_RIGHT_REG    0x92

  /* Maximum offset while Moving Up and Moving Down gesture */
#define FT6206_OFFSET_UP_DOWN_REG       0x93

  /* Minimum distance while Moving Left and Moving Right gesture */
#define FT6206_DISTANCE_LEFT_RIGHT_REG  0x94

  /* Minimum distance while Moving Up and Moving Down gesture */
#define FT6206_DISTANCE_UP_DOWN_REG     0x95

  /* Maximum distance while Zoom In and Zoom Out gesture */
#define FT6206_DISTANCE_ZOOM_REG        0x96

  /* High 8-bit of LIB Version info */
#define FT6206_LIB_VER_H_REG            0xA1

  /* Low 8-bit of LIB Version info */
#define FT6206_LIB_VER_L_REG            0xA2

  /* Chip Selecting */
#define FT6206_CIPHER_REG               0xA3

  /* Interrupt mode register (used when in interrupt mode) */
#define FT6206_GMODE_REG                0xA4

#define FT6206_G_MODE_INTERRUPT_MASK    0x03
#define FT6206_G_MODE_INTERRUPT_SHIFT   0x00

  /* Possible values of FT6206_GMODE_REG */
#define FT6206_G_MODE_INTERRUPT_POLLING 0x00
#define FT6206_G_MODE_INTERRUPT_TRIGGER 0x01

  /* Current power mode the FT6206 system is in (R) */
#define FT6206_PWR_MODE_REG             0xA5

  /* FT6206 firmware version */
#define FT6206_FIRMID_REG               0xA6

  /* FT6206 Chip identification register */
#define FT6206_CHIP_ID_REG              0xA8

  /*  Possible values of FT6206_CHIP_ID_REG */
#define FT6206_ID_VALUE                 0x11

  /* Release code version */
#define FT6206_RELEASE_CODE_ID_REG      0xAF

  /* Current operating mode the FT6206 system is in (R) */
#define FT6206_STATE_REG                0xBC


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

#define CST836U_TOUCH_EVT_FLAG_SHIFT     6
#define CST836U_TOUCH_EVT_FLAG_MASK      (3 << CST836U_TOUCH_EVT_FLAG_SHIFT)
#define CST836U_TOUCH_EVT_FLAG_CONTACT   0x02

#define LCD_VERTICAL                ( 0x00 )
#define LCD_HORIZONTAL              ( 0x01 )
#define LCD_DIRECTION               ( LCD_VERTICAL )

#define LCD_WIDTH                         ( 320 )
#define LCD_HEIGHT                        ( 480 )

extern void TouchInit( void );
extern void TouchDriver( void );


#ifdef __cplusplus
}
#endif
