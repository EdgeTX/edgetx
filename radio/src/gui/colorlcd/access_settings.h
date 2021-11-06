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

#include "menu.h"
#include "static.h"
#include "button.h"
#include "dialog.h"
#include "textedit.h"
#include "numberedit.h"

class BindRxChoiceMenu : public Menu
{
 public:
  BindRxChoiceMenu(Window* parent, uint8_t moduleIdx, uint8_t receiverIdx);

 protected:
  uint8_t moduleIdx;
  uint8_t receiverIdx;
};

class BindWaitDialog : public Dialog
{
 public:
  BindWaitDialog(Window* parent, uint8_t moduleIdx, uint8_t receiverIdx);

  void checkEvents() override;
#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif

 protected:
  uint8_t moduleIdx;
  uint8_t receiverIdx;
};

class ReceiverButton : public TextButton
{
 public:
  ReceiverButton(FormGroup* parent, rect_t rect, uint8_t moduleIdx,
                 uint8_t receiverIdx);

  uint8_t pressBind();
  void startBind();
  void checkEvents() override;

 protected:
  uint8_t moduleIdx;
  uint8_t receiverIdx;
};

class RegisterDialog : public Dialog
{
 public:
  RegisterDialog(Window* parent, uint8_t moduleIdx);

  void start();
  void checkEvents() override;
#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif

 protected:
  uint8_t moduleIdx;
  NumberEdit* uid;
  StaticText* waiting;
  TextEdit* rxName = nullptr;
  TextButton* exitButton;
};
