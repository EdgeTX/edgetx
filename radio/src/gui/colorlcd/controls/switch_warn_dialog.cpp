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

#include "switch_warn_dialog.h"

#include "static.h"
#include "switches.h"

SwitchWarnDialog::SwitchWarnDialog() :
    FullScreenDialog(WARNING_TYPE_ALERT, STR_SWITCHWARN, "",
                     STR_PRESS_ANY_KEY_TO_SKIP)
{
  lv_label_set_long_mode(messageLabel->getLvObj(), LV_LABEL_LONG_WRAP);
  AUDIO_ERROR_MESSAGE(AU_SWITCH_ALERT);
}

void SwitchWarnDialog::checkEvents()
{
  FullScreenDialog::checkEvents();

  uint16_t bad_pots;
  if (!isSwitchWarningRequired(bad_pots)) {
    deleteLater();
    return;
  };

  std::string warn_txt;
  for (int i = 0; i < switchGetMaxAllSwitches(); ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      uint8_t warnState = g_model.getSwitchWarning(i);
      if (warnState) {
        swarnstate_t swState = g_model.getSwitchStateForWarning(i);
        if (warnState != swState) {
          warn_txt +=
              getSwitchPositionName(SWSRC_FIRST_SWITCH + i * 3 + warnState - 1);
          warn_txt += " ";
        }
      }
    }
  }

  if (g_model.potsWarnMode) {
    if (!warn_txt.empty()) {
      warn_txt += '\n';
    }
    for (int i = 0; i < MAX_POTS; i++) {
      if (!IS_POT_SLIDER_AVAILABLE(i)) {
        continue;
      }
      if ((g_model.potsWarnEnabled & (1 << i))) {
        if (abs(g_model.potsWarnPosition[i] - GET_LOWRES_POT_POSITION(i)) > 1) {
          warn_txt += CHAR_POT;
          warn_txt += getPotLabel(i);
          warn_txt += " ";
        }
      }
    }
  }

  messageLabel->setText(warn_txt);
}

ThrottleWarnDialog::ThrottleWarnDialog(const char* msg) :
    FullScreenDialog(WARNING_TYPE_ALERT, STR_THROTTLE_UPPERCASE, msg,
                     STR_PRESS_ANY_KEY_TO_SKIP)
{
  lv_label_set_long_mode(messageLabel->getLvObj(), LV_LABEL_LONG_WRAP);
  AUDIO_ERROR_MESSAGE(AU_THROTTLE_ALERT);
}

void ThrottleWarnDialog::checkEvents()
{
  FullScreenDialog::checkEvents();

  extern bool isThrottleWarningAlertNeeded();
  if (!isThrottleWarningAlertNeeded())
    deleteLater();
}
