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

// Macros for B&W menus

#if defined(VARIO)
  #define CASE_VARIO(x) x,
#else
  #define CASE_VARIO(x)
#endif

#if defined(IMU)
#define CASE_IMU(x) x,
#else
#define CASE_IMU(x)
#endif

#if defined(OLED_SCREEN)
#define CASE_CONTRAST(x)
#else
#define CASE_CONTRAST(x) x,
#endif

#if defined(BACKLIGHT_GPIO) || defined(OLED_SCREEN)
#define CASE_BACKLIGHT(x) x,
#else
#define CASE_BACKLIGHT(x)
#endif

#if defined(RTCLOCK)
  #define CASE_RTCLOCK(x) x,
#else
  #define CASE_RTCLOCK(x)
#endif

#if defined(BUZZER)
  #define CASE_BUZZER(x) x,
#else
  #define CASE_BUZZER(x)
#endif

#if defined(AUDIO)
  #define CASE_AUDIO(x) x,
#else
  #define CASE_AUDIO(x)
#endif

#if defined(GPS)
  #define CASE_GPS(x) x,
#else
  #define CASE_GPS(x)
#endif

#if defined(HAPTIC)
  #define CASE_HAPTIC(x) x,
#else
  #define CASE_HAPTIC(x)
#endif

#if defined(PWR_BUTTON_PRESS)
  #define CASE_PWR_BUTTON_PRESS(x) x,
#else
  #define CASE_PWR_BUTTON_PRESS(x)
#endif

#if defined(PXX1)
  #define CASE_PXX1(x) x,
#else
  #define CASE_PXX1(x)
#endif

#if defined(PXX2)
  #define CASE_PXX2(x) x,
#else
  #define CASE_PXX2(x)
#endif

#if defined(BLUETOOTH)
  #define CASE_BLUETOOTH(x) x,
#else
  #define CASE_BLUETOOTH(x)
#endif

#if defined(HELI)
  #define CASE_HELI(x) x,
#else
  #define CASE_HELI(x)
#endif

#if defined(FLIGHT_MODES)
  #define CASE_FLIGHT_MODES(x) x,
#else
  #define CASE_FLIGHT_MODES(x)
#endif

#if defined(GVARS)
  #define CASE_GVARS(x) x,
#else
  #define CASE_GVARS(x)
#endif

#if defined(LUA_MODEL_SCRIPTS)
  #define CASE_LUA_MODEL_SCRIPTS(x) x,
#else
  #define CASE_LUA_MODEL_SCRIPTS(x)
#endif

#if defined(PCBX9DP) || defined(PCBX9E)
  #define CASE_PCBX9E_PCBX9DP(x) x,
#else
  #define CASE_PCBX9E_PCBX9DP(x)
#endif

#define CASE_SPLASH_PARAM(x) x,
