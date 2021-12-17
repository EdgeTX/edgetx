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

bool SwitchWarnDialog::warningInactive()
{
  if (!isSwitchWarningRequired(bad_pots))
    return true;

  if (last_bad_switches != switches_states || last_bad_pots != bad_pots) {
    invalidate();
    if (last_bad_switches == 0xff || last_bad_pots == 0xff) {
      AUDIO_ERROR_MESSAGE(AU_SWITCH_ALERT);
    }
  }

  last_bad_pots = bad_pots;
  last_bad_switches = switches_states;

  return false;
}

void SwitchWarnDialog::paint(BitmapBuffer * dc)
{
  if (!running)
    return;

  FullScreenDialog::paint(dc);

  coord_t x = ALERT_MESSAGE_LEFT;
  coord_t y = ALERT_MESSAGE_TOP;
  swarnstate_t states = g_model.switchWarningState;

  for (int i = 0; i < NUM_SWITCHES; ++i) {
    if (SWITCH_WARNING_ALLOWED(i)) {
      swarnstate_t mask = ((swarnstate_t)0x07 << (i*3));
      if (states & mask) {
        if ((switches_states & mask) != (states & mask)) {
          swarnstate_t state = (states >> (i*3)) & 0x07;
          if (x < LCD_W) {
            x = drawSwitch(dc, x, y, SWSRC_FIRST_SWITCH + i * 3 + state - 1,
                           COLOR_THEME_PRIMARY1 | FONT(BOLD));
            x += 5;
          }
          else {
            dc->drawText(x, y, "...", COLOR_THEME_PRIMARY1 | FONT(BOLD));
            break;
          }
        }
      }
    }
  }

  if (g_model.potsWarnMode) {
    x = ALERT_MESSAGE_LEFT;
    y += 20;
    for (int i = 0; i < NUM_POTS + NUM_SLIDERS; i++) {
      if (!IS_POT_SLIDER_AVAILABLE(POT1 + i)) {
        continue;
      }
      if ( (g_model.potsWarnEnabled & (1 << i))) {
        if (abs(g_model.potsWarnPosition[i] - GET_LOWRES_POT_POSITION(i)) > 1) {
          if (y < LCD_H) {
            char s[8];
            // TODO add an helper
            strncpy(s, &STR_VSRCRAW[1 + (NUM_STICKS + 1 + i) * STR_VSRCRAW[0]], STR_VSRCRAW[0]);
            s[int(STR_VSRCRAW[0])] = '\0';
            dc->drawText(x, y, s, COLOR_THEME_PRIMARY1 | FONT(BOLD));
            x += 40;
          }
          else {
            dc->drawText(x, y, "...", COLOR_THEME_PRIMARY1 | FONT(BOLD));
          }
        }
      }
    }
  }
}
