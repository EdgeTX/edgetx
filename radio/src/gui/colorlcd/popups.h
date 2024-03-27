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

#include <functional>

#include "libopenui.h"

enum WarningType
{
  WARNING_TYPE_WAIT,
  WARNING_TYPE_INFO,
  WARNING_TYPE_ASTERISK,
  WARNING_TYPE_CONFIRM,
  WARNING_TYPE_INPUT,
  WARNING_TYPE_ALERT
};

typedef std::function<void(const char *, const char *, int, int)>
    ProgressHandler;

void POPUP_INFORMATION(const char *message);
void POPUP_WARNING(const char *message, const char *info = nullptr);
void POPUP_WARNING_ON_UI_TASK(const char *message, const char *info = nullptr,
                              bool waitForClose = true);
void POPUP_BUBBLE(const char *message, uint32_t timeout, coord_t width = LCD_W - 100);

void show_ui_popup();
