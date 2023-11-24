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

#include "bitmapbuffer.h"

enum EdgeTxIcon {
  ICON_EDGETX,
  ICON_RADIO,
  ICON_RADIO_SETUP,
  ICON_RADIO_SD_MANAGER,
  ICON_RADIO_TOOLS,
  ICON_RADIO_GLOBAL_FUNCTIONS,
  ICON_RADIO_TRAINER,
  ICON_RADIO_HARDWARE,
  ICON_RADIO_CALIBRATION,
  ICON_RADIO_EDIT_THEME,
  ICON_RADIO_VERSION,
  ICON_MODEL,
  ICON_MODEL_SETUP,
  ICON_MODEL_HELI,
  ICON_MODEL_FLIGHT_MODES,
  ICON_MODEL_INPUTS,
  ICON_MODEL_MIXER,
  ICON_MODEL_NOTES,
  ICON_MODEL_OUTPUTS,
  ICON_MODEL_CURVES,
  ICON_MODEL_GVARS,
  ICON_MODEL_LOGICAL_SWITCHES,
  ICON_MODEL_SPECIAL_FUNCTIONS,
  ICON_MODEL_LUA_SCRIPTS,
  ICON_MODEL_TELEMETRY,
  ICON_MODEL_USB,
  ICON_MODEL_SELECT,
  ICON_THEME,
  ICON_THEME_SETUP,
  ICON_THEME_VIEW1,
  ICON_THEME_VIEW2,
  ICON_THEME_VIEW3,
  ICON_THEME_VIEW4,
  ICON_THEME_VIEW5,
  ICON_THEME_VIEW6,
  ICON_THEME_VIEW7,
  ICON_THEME_VIEW8,
  ICON_THEME_VIEW9,
  ICON_THEME_VIEW10,
  ICON_THEME_ADD_VIEW,
  ICON_STATS,
  ICON_STATS_THROTTLE_GRAPH,
  ICON_STATS_TIMERS,
  ICON_STATS_ANALOGS,
  ICON_STATS_DEBUG,
  ICON_MONITOR,
  ICON_MONITOR_CHANNELS1,
  ICON_MONITOR_CHANNELS2,
  ICON_MONITOR_CHANNELS3,
  ICON_MONITOR_CHANNELS4,
  ICON_MONITOR_LOGICAL_SWITCHES,

  ICON_CHAN_MONITOR_LOCKED,
  ICON_CHAN_MONITOR_INVERTED,

  ICON_SHUTDOWN_CIRCLE0,
  ICON_SHUTDOWN_CIRCLE1,
  ICON_SHUTDOWN_CIRCLE2,
  ICON_SHUTDOWN_CIRCLE3,
  ICON_SHUTDOWN,

  ICON_TOPLEFT_BG,
  ICON_CURRENTMENU_BG,
  ICON_CURRENTMENU_SHADOW,
  ICON_CURRENTMENU_DOT,

  ICON_DOT,

  ICON_TOPMENU_USB,
  ICON_TOPMENU_VOLUME_0,
  ICON_TOPMENU_VOLUME_1,
  ICON_TOPMENU_VOLUME_2,
  ICON_TOPMENU_VOLUME_3,
  ICON_TOPMENU_VOLUME_4,
  ICON_TOPMENU_VOLUME_SCALE,
  ICON_TOPMENU_TXBATT,
#if defined(USB_CHARGER)
  ICON_TOPMENU_TXBATT_CHARGE,
#endif
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  ICON_TOPMENU_ANTENNA,
#endif
#if defined(INTERNAL_GPS)
  ICON_TOPMENU_GPS,
#endif

  ICON_ERROR,
  ICON_BUSY,

  ICON_USB_PLUGGED,

  ICON_TIMER_BG,
  ICON_TIMER,

  ICON_TEXTLINE_CURVE,
  ICON_TEXTLINE_FM,

  ICON_MPLEX_ADD,
  ICON_MPLEX_MULTIPLY,
  ICON_MPLEX_REPLACE,

  ICON_ROUND_TITLE_LEFT,
  ICON_ROUND_TITLE_RIGHT,

  ICON_MODEL_GRID_LARGE,
  ICON_MODEL_GRID_SMALL,
  ICON_MODEL_LIST_TWO,
  ICON_MODEL_LIST_ONE,

  EDGETX_ICONS_COUNT
};

struct MaskBitmap {
  uint16_t width;
  uint16_t height;
  uint8_t data[];
};

const MaskBitmap* getBuiltinIcon(EdgeTxIcon id);

struct LZ4Bitmap {
  uint16_t width;
  uint16_t height;
  uint32_t compressedSize;
  uint8_t data[];
};

class LZ4BitmapBuffer : public BitmapBuffer
{
 public:
  LZ4BitmapBuffer(uint8_t format, const LZ4Bitmap* compressed_data);
  ~LZ4BitmapBuffer();
};
