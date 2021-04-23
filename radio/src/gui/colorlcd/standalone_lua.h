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

#pragma once

#include "libopenui.h"

class StandaloneLuaWindow : public Window
{
  public:
    explicit StandaloneLuaWindow(Window* parent);

    void deleteLater(bool detach=true, bool trash=true) override;
    void paint(BitmapBuffer* dc) override;
    void checkEvents() override;

#if defined(DEBUG_WINDOWS)
    std::string getName() const override { return "StandaloneLuaWindow"; }
#endif

#if defined(HARDWARE_KEYS)
    void onEvent(event_t evt) override;
#endif

  protected:
    BitmapBuffer lcdBuffer;
    uint32_t lastRefresh = 0;

    void runLua(event_t evt);
};
