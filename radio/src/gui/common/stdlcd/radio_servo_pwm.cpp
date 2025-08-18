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

#include "edgetx.h"
#include "os/sleep.h"

#if defined(SERVO_PWM)

#if LCD_W >= 212
  #define HW_SETTINGS_COLUMN1            12*FW
  #define HW_SETTINGS_COLUMN2            (20*FW - 3)
  #define HW_SETTINGS_COLUMN3            HW_SETTINGS_COLUMN2
#else
  #define HW_SETTINGS_COLUMN1            30
  #define HW_SETTINGS_COLUMN2            (HW_SETTINGS_COLUMN1 + 5*FW)
  #define HW_SETTINGS_COLUMN3            HW_SETTINGS_COLUMN2 + FW
#endif

#if !defined(SIMU)
extern void pwmOutputDisable();
extern void pwmOutputInit();
extern void pwmOutputEnable(uint16_t pulse_width_us);
#else
void pwmOutputDisable() {}
void pwmOutputInit() {}
void pwmOutputEnable(uint16_t) {}
#endif

static uint8_t pwmOutputSource;

enum {
  ITEM_PWM_SOURCE,
  ITEM_SERVO_PWM_MAX,
};

void menuServoPwmTool(event_t event)
{
  SUBMENU(STR_PWM_OUTPUT, 0, {0});

  if (menuEvent == EVT_ENTRY_UP) {
    TRACE("stopping servo pwm...");
    pwmOutputDisable();
    return;
  }

  if (event == EVT_ENTRY) {
    TRACE("starting servo pwm...");
    pwmOutputSource = MIXSRC_NONE;
    pwmOutputInit();
  }

  if (pwmOutputSource > MIXSRC_NONE) {
    mixsrc_t chan = pwmOutputSource - MIXSRC_FIRST;
    pwmOutputEnable(PPM_CH_CENTER(chan) + channelOutputs[chan] / 2);
  } else {
    pwmOutputDisable();
  }

  for (int i = 0; i < ITEM_SERVO_PWM_MAX; i++) {
    LcdFlags attr = (menuVerticalPosition == i ? (s_editMode > 0 ? INVERS | BLINK : INVERS) : 0);
    coord_t y = MENU_HEADER_HEIGHT + 12 + i * FH;

    switch (i) {
      case ITEM_PWM_SOURCE:
        lcdDrawTextAlignedLeft(y, STR_SOURCE);
        drawSource(HW_SETTINGS_COLUMN2, y, pwmOutputSource, attr);
        if (attr & BLINK)
          CHECK_INCDEC_GENVAR(event, pwmOutputSource, 0, MAX_OUTPUT_CHANNELS);
        break;
    }
  }
}

#endif

