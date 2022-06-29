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



#define HAS_TOUCH_PANEL()           touchCST340Flag == true

#define CST340_MODE_DEBUG_INFO      0xD101      // To read out chip ID and firmware version
#define CST340_MODE_NORMAL          0xD109      // Normal mode
#define CST340_FINGER1_REG          0xD000		// Touch info register
#define CST340_CHIPTYPE_REG         0xD204		// uint16_t chip IC type & uint16_t project ID register
#define CST340_FWVER_REG            0xD208		// Firmware version register(uint8_t major, uint8_t minor, uint16_t build)
#define CST340_TOUCH_EVT_FLAG_CONTACT 6

#define CST340_CHIP_ID              0x011C      // Expected answer to CST340_CHIPTYPE_REG query for the ChipID field

#define TOUCH_POINTS_MAX            5	        // Max touch points

#define TOUCH_ACK                   0  			       
#define TOUCH_NACK                  1	

#define CST340_I2C_ADDR             0x1A

extern bool touchCST340Flag;
extern uint32_t touchI2Chiccups;
extern uint16_t touchICfwver;
void TouchInit();

struct TouchState touchPanelRead();
bool touchPanelEventOccured();

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
    TouchPoint points[TOUCH_POINTS_MAX];
    uint8_t data[TOUCH_POINTS_MAX * sizeof(TouchPoint)];
  };
});

#define TPRST_LOW()   do { TOUCH_RST_GPIO->BSRRH = TOUCH_RST_GPIO_PIN; } while(0)
#define TPRST_HIGH()  do { TOUCH_RST_GPIO->BSRRL = TOUCH_RST_GPIO_PIN; } while(0)
