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

#include "static.h"
#include "button.h"
#include "dialog.h"
#include "textedit.h"
#include "numberedit.h"

namespace pxx2 {

class ReceiverButton : public TextButton
{
 public:
  ReceiverButton(Window* parent, rect_t rect, uint8_t moduleIdx,
                 uint8_t receiverIdx);

  uint8_t pressBind();
  void startBind();
  void checkEvents() override;

 protected:
  uint8_t moduleIdx;
  uint8_t receiverIdx;
};

class RegisterDialog : public BaseDialog
{
 public:
  RegisterDialog(uint8_t moduleIdx);

  void start();
  void checkEvents() override;

 protected:
  uint8_t moduleIdx;
  uint8_t old_registerStep = 0;
  Window* reg_id = nullptr;
  Window* uid = nullptr;
  TextEdit* rx_name = nullptr;
  StaticText* status = nullptr;
  Window* btn_ok = nullptr;
};

class ModuleOptions : public BaseDialog
{
 public:
  ModuleOptions(uint8_t moduleIdx);
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

};
