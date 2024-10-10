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

enum SwitchConfig {
  SWITCH_NONE,
  SWITCH_TOGGLE,
  SWITCH_2POS,
  SWITCH_3POS,
};

enum FlexAnalogConfig {
  FLEX_NONE=0,
  FLEX_POT, // without detent
  FLEX_POT_CENTER, // with detent
  FLEX_SLIDER,
  FLEX_MULTIPOS,
  FLEX_AXIS_X,
  FLEX_AXIS_Y,
  FLEX_SWITCH,
};

enum fsStartPositionType {
  FS_START_ON,
  FS_START_OFF,
  FS_START_PREVIOUS
};

enum CalibrationState {
  CALIB_START,
  CALIB_SET_MIDPOINT,
  CALIB_MOVE_STICKS,
  CALIB_STORE,
  CALIB_FINISHED
};
