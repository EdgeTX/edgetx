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

#ifndef OPENTX_OPTIONS_H
#define OPENTX_OPTIONS_H

static const char * const options[] = {
#if defined(AUTOUPDATE)
  "autoupdate",
#endif
#if defined(BLUETOOTH)
  "bluetooth",
#endif
#if defined(CROSSFIRE)
  "crossfire",
#endif
#if defined(GHOST)
  "ghost",
#endif
#if !defined(MODULE_PROTOCOL_D8)
  "eu",
#endif
#if defined(FAI)
  "FAImode",
#endif
#if defined(FAI_CHOICE)
  "FAIchoice",
#endif
#if defined(AFHDS3)
  "afhds3",
#endif
#if !defined(GVARS)
  "nogvars",
#endif
#if defined(HAPTIC) && defined(PCBX9D)
  "haptic",
#endif
#if !defined(HELI)
  "noheli",
#endif
#if defined(HORUS_STICKS)
  "horussticks",
#endif
#if defined(INTERNAL_GPS)
  "internalgps",
#endif
#if defined(INTERNAL_MODULE_PPM)
  "internalppm",
#endif
#if defined(INTERNAL_MODULE_MULTI)
  "internalmulti",
#endif
#if defined(INTERNAL_MODULE_PXX2)
  "internalaccess",
#endif
#if defined(MULTIMODULE)
  "multimodule",
#endif
#if defined(LUA_MODEL_SCRIPTS)
  "lua",
#endif
#if defined(LUA_COMPILER)
  "luac",
#endif
#if !defined(OVERRIDE_CHANNEL_FUNCTION)
  "nooverridech",
#endif
#if defined(PPM_UNIT_US)
  "ppmus",
#endif
#if defined(NO_RAS)
  "noras",
#endif
#if defined(SHUTDOWN_CONFIRMATION)
  "shutdownconfirm",
#endif
#if defined(HARDWARE_EXTERNAL_ACCESS_MOD)
  "externalaccessmod",
#endif
#if defined(IMU_LSM6DS33)
  "lsm6ds33",
#endif
#if defined(BIND_KEY)
  "bindkey",
#endif
#if defined(CLI)
    "cli",
#endif
#if defined(ENABLE_SERIAL_PASSTHROUGH)
    "passthrough",
    "cli",
#endif
  nullptr //sentinel
};

#endif //OPENTX_OPTIONS_H
