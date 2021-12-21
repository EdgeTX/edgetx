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

#ifndef _SWITCH_WARN_DIALOG_H_
#define _SWITCH_WARN_DIALOG_H_

#include "fullscreen_dialog.h"
#include "mainwindow.h"
#include "opentx.h"
#include "gridlayout.h"

class SwitchWarnDialog: public FullScreenDialog {
  public:
    SwitchWarnDialog():
      FullScreenDialog(WARNING_TYPE_ALERT, STR_SWITCHWARN)
    {
      last_bad_switches = 0xff;
      bad_pots = 0;
      last_bad_pots = 0x0;
      setCloseCondition(std::bind(&SwitchWarnDialog::warningInactive, this));
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "SwitchWarnDialog";
    }
#endif

    void paint(BitmapBuffer * dc) override;

  protected:
    swarnstate_t last_bad_switches;
    uint16_t      bad_pots;
    uint16_t      last_bad_pots;
    bool warningInactive();
};

#endif
