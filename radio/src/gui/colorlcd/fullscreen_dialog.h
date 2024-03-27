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

#include "dialog.h"

class StaticText;

class FullScreenDialog : public Window
{
  public:
    FullScreenDialog(uint8_t type, std::string title, std::string message = "",
                     std::string action = "",
                     const std::function<void(void)>& confirmHandler = nullptr);

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "FullScreenDialog";
    }
#endif

    void setMessage(std::string text);

    void onEvent(event_t event) override;
    void onCancel() override;

    void deleteLater(bool detach = true, bool trash = true) override;

    void checkEvents() override;

    void setCloseCondition(std::function<bool(void)> handler)
    {
      closeCondition = std::move(handler);
    }

    void runForever(bool checkPwr = true);

    void closeDialog();

  protected:
    uint8_t type;
    std::string title;
    std::string message;
    std::string action;
    bool running = false;
    std::function<bool(void)> closeCondition;
    std::function<void(void)> confirmHandler;
    bool loaded = false;
    StaticText* messageLabel;

    virtual void delayedInit() {}
    void build();

    static void long_pressed(lv_event_t* e);
    static void on_draw(lv_event_t* e);
};
