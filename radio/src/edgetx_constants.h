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

#ifndef _OPENTX_CONSTANTS_H_
#define _OPENTX_CONSTANTS_H_

enum SwitchConfig {
  SWITCH_NONE,
  SWITCH_TOGGLE,
  SWITCH_2POS,
  SWITCH_3POS,
};

enum PotConfig {
  POT_NONE,
  POT_WITH_DETENT,
  POT_MULTIPOS_SWITCH,
  POT_WITHOUT_DETENT
};

enum SliderConfig {
  SLIDER_NONE,
  SLIDER_WITH_DETENT,
};

enum CalibrationState {
  CALIB_START,
  CALIB_SET_MIDPOINT,
  CALIB_MOVE_STICKS,
  CALIB_STORE,
  CALIB_FINISHED
};

#endif // _OPENTX_CONSTANTS_H_
