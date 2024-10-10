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
#include <math.h>

#define RECEIVER_OPTIONS_2ND_COLUMN  (11*FW)

extern uint8_t g_moduleIdx;

enum {
  MODULE_SETTINGS_OK = 0,
  MODULE_SETTINGS_DIRTY = 1,
  MODULE_SETTINGS_REBIND = 2,
  MODULE_SETTINGS_WRITING = 4,
};

void onTxOptionsUpdateConfirm(const char * result)
{
  if (result == STR_OK) {
    reusableBuffer.hardwareAndSettings.moduleSettings.dirty = MODULE_SETTINGS_WRITING;
    moduleState[g_moduleIdx].writeModuleSettings(&reusableBuffer.hardwareAndSettings.moduleSettings);
  }
  else {
    popMenu();
  }
}

bool isTelemetryAvailable()
{
  return reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].information.variant != PXX2_VARIANT_EU ||
         reusableBuffer.hardwareAndSettings.moduleSettings.txPower <= 14;
}

enum {
  ITEM_MODULE_SETTINGS_EXTERNAL_ANTENNA,
  ITEM_MODULE_SETTINGS_POWER,
  ITEM_MODULE_SETTINGS_TELEMETRY,
  ITEM_MODULE_SETTINGS_COUNT
};

#define IF_MODULE_OPTIONS(option, count) uint8_t(isPXX2ModuleOptionAvailable(modelId, option) ? count : HIDDEN_ROW)

bool isPowerAvailable(int value)
{
  return isPXX2PowerAvailable(reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].information, value);
}

void menuModelModuleOptions(event_t event)
{
  if (event == EVT_ENTRY) {
    memclear(&reusableBuffer.hardwareAndSettings, sizeof(reusableBuffer.hardwareAndSettings));
#if defined(SIMU)
    reusableBuffer.hardwareAndSettings.moduleSettings.state = PXX2_SETTINGS_OK;
#endif
  }

  uint8_t modelId = reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].information.modelID;
  // uint8_t variant = reusableBuffer.hardwareAndSettings.modules[g_moduleIdx].information.variant;
  uint8_t optionsAvailable = getPXX2ModuleOptions(modelId) & ((1 << MODULE_OPTION_EXTERNAL_ANTENNA) | (1 << MODULE_OPTION_POWER));

  SUBMENU_NOTITLE(ITEM_MODULE_SETTINGS_COUNT, {
    !optionsAvailable ? (uint8_t)0 : IF_MODULE_OPTIONS(MODULE_OPTION_EXTERNAL_ANTENNA, 0),
    IF_MODULE_OPTIONS(MODULE_OPTION_POWER, 0),
    IF_MODULE_OPTIONS(MODULE_OPTION_POWER, isTelemetryAvailable() ? HIDDEN_ROW : READONLY_ROW)
  });

  if (reusableBuffer.hardwareAndSettings.moduleSettings.state == PXX2_HARDWARE_INFO && moduleState[g_moduleIdx].mode == MODULE_MODE_NORMAL) {
    if (!modelId)
      moduleState[g_moduleIdx].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[g_moduleIdx], PXX2_HW_INFO_TX_ID, PXX2_HW_INFO_TX_ID);
    else
      moduleState[g_moduleIdx].readModuleSettings(&reusableBuffer.hardwareAndSettings.moduleSettings);
  }

  if (menuEvent) {
    killEvents(KEY_EXIT); // TODO: Check if needed
    moduleState[g_moduleIdx].mode = MODULE_MODE_NORMAL;
    if (reusableBuffer.hardwareAndSettings.moduleSettings.dirty) {
      abortPopMenu();
      POPUP_CONFIRMATION(STR_UPDATE_TX_OPTIONS, onTxOptionsUpdateConfirm);
    }
    else {
      return;
    }
  }

  if (event == EVT_KEY_LONG(KEY_ENTER) && reusableBuffer.hardwareAndSettings.moduleSettings.dirty) {
    killEvents(event);
    reusableBuffer.hardwareAndSettings.moduleSettings.dirty = MODULE_SETTINGS_OK;
    moduleState[g_moduleIdx].writeModuleSettings(&reusableBuffer.hardwareAndSettings.moduleSettings);
  }

  if (reusableBuffer.hardwareAndSettings.moduleSettings.dirty == MODULE_SETTINGS_WRITING && reusableBuffer.hardwareAndSettings.moduleSettings.state == PXX2_SETTINGS_OK) {
    popMenu();
    return;
  }

  if (modelId != 0 && mstate_tab[menuVerticalPosition] == HIDDEN_ROW) {
    menuVerticalPosition = 0;
    while (menuVerticalPosition < ITEM_MODULE_SETTINGS_COUNT && mstate_tab[menuVerticalPosition] == HIDDEN_ROW) {
      ++menuVerticalPosition;
    }
  }

  int8_t sub = menuVerticalPosition;
  lcdDrawTextAlignedLeft(0, STR_MODULE_OPTIONS);
  lcdDrawText(lcdLastRightPos + 3, 0, getPXX2ModuleName(modelId));
  lcdInvertLine(0);

  if (reusableBuffer.hardwareAndSettings.moduleSettings.state == PXX2_SETTINGS_OK) {
    if (optionsAvailable) {
      for (uint8_t k=0; k<LCD_LINES-1; k++) {
        coord_t y = MENU_HEADER_HEIGHT + 1 + k*FH;
        uint8_t i = k + menuVerticalOffset;
        for (int j=0; j<=i; ++j) {
          if (j<(int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
            ++i;
          }
        }
        LcdFlags attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

        switch (i) {
          case ITEM_MODULE_SETTINGS_EXTERNAL_ANTENNA:
            reusableBuffer.hardwareAndSettings.moduleSettings.externalAntenna = editCheckBox(reusableBuffer.hardwareAndSettings.moduleSettings.externalAntenna, RECEIVER_OPTIONS_2ND_COLUMN, y, STR_EXT_ANTENNA, attr, event);
            if (attr && checkIncDec_Ret) {
              reusableBuffer.hardwareAndSettings.moduleSettings.dirty = MODULE_SETTINGS_DIRTY;
            }
            break;

          case ITEM_MODULE_SETTINGS_POWER:
            lcdDrawText(0, y, STR_POWER);
            lcdDrawNumber(RECEIVER_OPTIONS_2ND_COLUMN, y, reusableBuffer.hardwareAndSettings.moduleSettings.txPower, attr);
            lcdDrawText(lcdNextPos, y, "dBm(");
            drawPower(lcdNextPos, y, reusableBuffer.hardwareAndSettings.moduleSettings.txPower);
            lcdDrawText(lcdNextPos, y, ")");
            if (attr) {
              bool previousTelemetry = isTelemetryAvailable();
              reusableBuffer.hardwareAndSettings.moduleSettings.txPower = checkIncDec(event, reusableBuffer.hardwareAndSettings.moduleSettings.txPower, 0, 30, 0, &isPowerAvailable);
              if (checkIncDec_Ret) {
                reusableBuffer.hardwareAndSettings.moduleSettings.dirty = MODULE_SETTINGS_DIRTY;
                if (previousTelemetry != isTelemetryAvailable()) {
                  reusableBuffer.hardwareAndSettings.moduleSettings.dirty |= MODULE_SETTINGS_REBIND;
                }
              }
              if (s_editMode == 0 && (reusableBuffer.hardwareAndSettings.moduleSettings.dirty & MODULE_SETTINGS_REBIND)) {
                reusableBuffer.hardwareAndSettings.moduleSettings.dirty &= ~MODULE_SETTINGS_REBIND;
                POPUP_WARNING(STR_REBIND);
              }
            }
            break;

          case ITEM_MODULE_SETTINGS_TELEMETRY:
            // only displayed in EU mode when TX power > 25mW
            lcdDrawText(RECEIVER_OPTIONS_2ND_COLUMN, y, "Telem OFF", attr | SMLSIZE);
            break;
        }
      }
    }
    else {
      lcdDrawCenteredText(LCD_H/2, STR_NO_TX_OPTIONS);
      s_editMode = 0;
    }
  }
  else {
    lcdDrawCenteredText(LCD_H/2, STR_WAITING_FOR_TX);
    s_editMode = 0;
  }
}
