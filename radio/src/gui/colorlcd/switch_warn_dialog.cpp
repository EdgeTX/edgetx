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

#include "switches.h"

SwitchWarnDialog::SwitchWarnDialog() :
    FullScreenDialog(WARNING_TYPE_ALERT, STR_SWITCHWARN, "",
                     STR_PRESS_ANY_KEY_TO_SKIP)
{
  last_bad_switches = 0xff;
  last_bad_pots = 0x0;
  setCloseCondition(std::bind(&SwitchWarnDialog::warningInactive, this));
}

void SwitchWarnDialog::delayedInit()
{
  lv_label_set_long_mode(messageLabel->getLvObj(), LV_LABEL_LONG_WRAP);
  AUDIO_ERROR_MESSAGE(AU_SWITCH_ALERT);
}

bool SwitchWarnDialog::warningInactive()
{
  uint16_t bad_pots;

  if (!isSwitchWarningRequired(bad_pots)) return true;

  if (last_bad_switches != switches_states || last_bad_pots != bad_pots) {
    last_bad_pots = bad_pots;
    last_bad_switches = switches_states;
  }

  return false;
}

void SwitchWarnDialog::checkEvents()
{
  if (!running) return;

  FullScreenDialog::checkEvents();

  std::string warn_txt;
  swarnstate_t states = g_model.switchWarning;
  for (int i = 0; i < MAX_SWITCHES; ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      swarnstate_t mask = ((swarnstate_t)0x07 << (i * 3));
      if (states & mask) {
        if ((switches_states & mask) != (states & mask)) {
          swarnstate_t state = (states >> (i * 3)) & 0x07;
          warn_txt +=
              getSwitchPositionName(SWSRC_FIRST_SWITCH + i * 3 + state - 1);
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
          warn_txt += STR_CHAR_POT;
          warn_txt += getPotLabel(i);
          warn_txt += " ";
        }
      }
    }
  }

  messageLabel->setText(warn_txt);
}

ThrottleWarnDialog::ThrottleWarnDialog(const char* msg) :
    FullScreenDialog(WARNING_TYPE_ALERT, TR_THROTTLE_UPPERCASE, msg,
                     STR_PRESS_ANY_KEY_TO_SKIP)
{
  setCloseCondition(std::bind(&ThrottleWarnDialog::warningInactive, this));
}

void ThrottleWarnDialog::delayedInit()
{
  lv_label_set_long_mode(messageLabel->getLvObj(), LV_LABEL_LONG_WRAP);
  AUDIO_ERROR_MESSAGE(AU_THROTTLE_ALERT);
}

bool ThrottleWarnDialog::warningInactive()
{
  extern bool isThrottleWarningAlertNeeded();
  return !isThrottleWarningAlertNeeded();
}
