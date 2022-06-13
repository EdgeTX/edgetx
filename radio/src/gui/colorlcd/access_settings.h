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

#if defined(HARDWARE_TOUCH)
  bool onTouchEnd(coord_t x, coord_t y) override;
#endif  
};

class BindWaitDialog : public Dialog
{
 public:
  BindWaitDialog(Window* parent, uint8_t moduleIdx, uint8_t receiverIdx);
  void checkEvents() override;

  void deleteLater(bool detach = true, bool trash = true) override;
  
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

 protected:
  uint8_t moduleIdx;
  NumberEdit* uid;
  StaticText* waiting;
  TextEdit* rxName = nullptr;
  TextButton* exitButton;
};

class ModuleOptions : public Dialog
{
 public:
  ModuleOptions(Window* parent, uint8_t moduleIdx);
  void checkEvents() override;

 protected:

  enum {
    MO_Init=0,
    MO_ReadModuleInfo,
    MO_ReadModuleSettings,
    MO_DisplaySettings,
    MO_WriteSettings,
    MO_WritingSettings,
  };
  
  uint8_t moduleIdx;
  uint8_t state = MO_Init;

  std::string statusText;

  uint8_t getModuleSettingsState();

  void update();
  void writeSettings();
};

class RxOptions : public Dialog
{
 public:
  RxOptions(Window* parent, uint8_t moduleIdx, uint8_t rxIdx);
  void checkEvents() override;

 protected:

  enum {
    RO_Init=0,
    RO_ReadModuleInfo,
    RO_ReadModuleSettings,
    RO_ReadReceiverSettings,
    RO_DisplaySettings,
    RO_WriteSettings,
    RO_WritingSettings,
  };
  
  uint8_t moduleIdx;
  uint8_t receiverIdx;
  uint8_t state = RO_Init;

  std::string statusText;

  uint8_t getRxSettingsState();

  void update();
  void writeSettings();
};
