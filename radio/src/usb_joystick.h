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

#include <stdint.h>

#include "storage/yaml/yaml_defs.h"

enum USBJoystickIfMode {
  USBJOYS_JOYSTICK,
  USBJOYS_GAMEPAD,
  USBJOYS_MULTIAXIS,
  USBJOYS_LAST SKIP = USBJOYS_MULTIAXIS
};

enum USBJoystickCh {
  USBJOYS_CH_NONE,
  USBJOYS_CH_BUTTON,
  USBJOYS_CH_AXIS,
  USBJOYS_CH_SIM,
  USBJOYS_CH_LAST SKIP = USBJOYS_CH_SIM
};

enum USBJoystickBtnMode {
  USBJOYS_BTN_MODE_NORMAL,
  USBJOYS_BTN_MODE_ON_PULSE,
  USBJOYS_BTN_MODE_SW_EMU,
  USBJOYS_BTN_MODE_DELTA,
  USBJOYS_BTN_MODE_COMPANION,
  USBJOYS_BTN_MODE_LAST SKIP = USBJOYS_BTN_MODE_COMPANION
};

enum USBJoystickAxis {
  USBJOYS_AXIS_X,
  USBJOYS_AXIS_Y,
  USBJOYS_AXIS_Z,
  USBJOYS_AXIS_RX,
  USBJOYS_AXIS_RY,
  USBJOYS_AXIS_RZ,
  USBJOYS_AXIS_SLIDER,
  USBJOYS_AXIS_DIAL,
  USBJOYS_AXIS_WHEEL,
  USBJOYS_AXIS_LAST SKIP = USBJOYS_AXIS_WHEEL
};

enum USBJoystickSim {
  USBJOYS_SIM_AILERON,
  USBJOYS_SIM_ELEVATOR,
  USBJOYS_SIM_RUDDER,
  USBJOYS_SIM_THROTTLE,
  USBJOYS_SIM_ACCELERATOR,
  USBJOYS_SIM_BREAK,
  USBJOYS_SIM_STEERING,
  USBJOYS_SIM_HAT,
  USBJOYS_SIM_LAST SKIP = USBJOYS_SIM_HAT
};

enum USBJoystickCC {
  USBJOYS_CC_NONE,
  USBJOYS_CC_XYZrX,
  USBJOYS_CC_XYrXrY,
  USBJOYS_CC_XYZrZ,
  USBJOYS_CC_LAST SKIP = USBJOYS_CC_XYZrZ
};

#define USBJ_BUTTON_SIZE 32
#define USBJ_MAX_JOYSTICK_CHANNELS 26

#ifdef __cplusplus
extern "C" {
#endif
struct usbReport_t {
  uint8_t* ptr;
  uint8_t size;
};
#ifdef __cplusplus
}
#endif

int usbJoystickExtMode();
int usbJoystickActive();
int usbJoystickSettingsChanged();

// Initializes the data based on the model settings
// Return value (true = description changed, false = desc. update not needed)
int setupUSBJoystick();

void onUSBJoystickModelChanged();

#ifdef __cplusplus
extern "C" {
#endif
struct usbReport_t usbReportDesc();
uint8_t usbReportSize();
#ifdef __cplusplus
}
#endif

struct usbReport_t usbReport();

int isUSBAxisCollision(uint8_t chIdx);
int isUSBSimCollision(uint8_t chIdx);
int isUSBBtnNumCollision(uint8_t chIdx);
