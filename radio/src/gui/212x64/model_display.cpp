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

enum MenuModelDisplayItems {
  ITEM_DISPLAY_TOP_BAR_LABEL,
  ITEM_DISPLAY_TOP_BAR_VOLTAGE,
  ITEM_DISPLAY_TOP_BAR_ALTITUDE,
  ITEM_DISPLAY_SCREEN_LABEL1,
  ITEM_DISPLAY_SCREEN_LINE1,
  ITEM_DISPLAY_SCREEN_LINE2,
  ITEM_DISPLAY_SCREEN_LINE3,
  ITEM_DISPLAY_SCREEN_LINE4,
  ITEM_DISPLAY_SCREEN_LABEL2,
  ITEM_DISPLAY_SCREEN_LINE5,
  ITEM_DISPLAY_SCREEN_LINE6,
  ITEM_DISPLAY_SCREEN_LINE7,
  ITEM_DISPLAY_SCREEN_LINE8,
  ITEM_DISPLAY_SCREEN_LABEL3,
  ITEM_DISPLAY_SCREEN_LINE9,
  ITEM_DISPLAY_SCREEN_LINE10,
  ITEM_DISPLAY_SCREEN_LINE11,
  ITEM_DISPLAY_SCREEN_LINE12,
  ITEM_DISPLAY_SCREEN_LABEL4,
  ITEM_DISPLAY_SCREEN_LINE13,
  ITEM_DISPLAY_SCREEN_LINE14,
  ITEM_DISPLAY_SCREEN_LINE15,
  ITEM_DISPLAY_SCREEN_LINE16,
  ITEM_DISPLAY_MAX
};

#define DISPLAY_COL1                  (1*FW)
#define DISPLAY_COL2                  (16*FW)
#define DISPLAY_COL3                  (28*FW+2)

inline uint8_t SCREEN_TYPE_COLUMNS(uint8_t screenIndex)
{
  if (TELEMETRY_SCREEN_TYPE(screenIndex) == TELEMETRY_SCREEN_TYPE_SCRIPT)
    return 1;
  else
    return 0;
}

inline uint8_t SCREEN_LINE_COLUMNS(uint8_t screenIndex, uint8_t lineIndex)
{
  switch (TELEMETRY_SCREEN_TYPE(screenIndex)) {
    case TELEMETRY_SCREEN_TYPE_VALUES:
      return 2;
    case TELEMETRY_SCREEN_TYPE_BARS:
      return g_model.screens[screenIndex].bars[lineIndex].source ? 2 : 0;
    default:
      return HIDDEN_ROW;
    }
}

#define TELEMETRY_SCREEN_ROWS(x)      SCREEN_TYPE_COLUMNS(x), SCREEN_LINE_COLUMNS(x, 0), SCREEN_LINE_COLUMNS(x, 1), SCREEN_LINE_COLUMNS(x, 2), SCREEN_LINE_COLUMNS(x, 3)

inline uint8_t DISPLAY_CURRENT_SCREEN(uint8_t line)
{
  if (line < ITEM_DISPLAY_SCREEN_LABEL2)
    return 0;
  else if (line < ITEM_DISPLAY_SCREEN_LABEL3)
    return 1;
  else if (line < ITEM_DISPLAY_SCREEN_LABEL4)
    return 2;
  else
    return 3;
}

#if defined(LUA)
void onTelemetryScriptFileSelectionMenu(const char *result)
{
  int screenIndex = DISPLAY_CURRENT_SCREEN(menuVerticalPosition);

  if (result == STR_UPDATE_LIST) {
    if (!sdListFiles(SCRIPTS_TELEM_PATH, SCRIPTS_EXT, sizeof(g_model.screens[screenIndex].script.file), nullptr)) {
      POPUP_WARNING(STR_NO_SCRIPTS_ON_SD);
    }
  }
  else if (result != STR_EXIT) {
    // The user choosed a file in the list
    memcpy(g_model.screens[screenIndex].script.file, result, sizeof(g_model.screens[screenIndex].script.file));
    storageDirty(EE_MODEL);
    LUA_LOAD_MODEL_SCRIPTS();
  }
}
#endif

void menuModelDisplay(event_t event)
{
  MENU(STR_MENU_DISPLAY, menuTabModel, MENU_MODEL_DISPLAY, ITEM_DISPLAY_MAX, { LABEL(TopBar), 0, 0, TELEMETRY_SCREEN_ROWS(0), TELEMETRY_SCREEN_ROWS(1), TELEMETRY_SCREEN_ROWS(2), TELEMETRY_SCREEN_ROWS(3) });

  for (int i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    int k = i + menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j] == HIDDEN_ROW)
        k++;
    }

    LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (menuVerticalPosition == k ? blink : 0);

    switch (k) {
      case ITEM_DISPLAY_TOP_BAR_LABEL:
        lcdDrawTextAlignedLeft(y, STR_TOP_BAR);
        break;

      case ITEM_DISPLAY_TOP_BAR_VOLTAGE:
        lcdDrawTextIndented(y, STR_VOLTAGE);
        drawSource(DISPLAY_COL2, y, g_model.voltsSource ? MIXSRC_FIRST_TELEM+3*(g_model.voltsSource-1) : 0, attr);
        if (attr) {
          g_model.voltsSource = checkIncDec(event, g_model.voltsSource, 0, MAX_TELEMETRY_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isVoltsSensor);
        }
        break;

      case ITEM_DISPLAY_TOP_BAR_ALTITUDE:
        lcdDrawTextIndented(y, STR_ALTITUDE);
        drawSource(DISPLAY_COL2, y, g_model.altitudeSource ? MIXSRC_FIRST_TELEM+3*(g_model.altitudeSource-1) : 0, attr);
        if (attr) {
          g_model.altitudeSource = checkIncDec(event, g_model.altitudeSource, 0, MAX_TELEMETRY_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isAltSensor);
        }
        break;

      case ITEM_DISPLAY_SCREEN_LABEL1:
      case ITEM_DISPLAY_SCREEN_LABEL2:
      case ITEM_DISPLAY_SCREEN_LABEL3:
      case ITEM_DISPLAY_SCREEN_LABEL4:
      {
        uint8_t screenIndex = DISPLAY_CURRENT_SCREEN(k);
        drawStringWithIndex(0*FW, y, STR_SCREEN, screenIndex+1);
        TelemetryScreenType oldScreenType = TELEMETRY_SCREEN_TYPE(screenIndex);
        TelemetryScreenType newScreenType = (TelemetryScreenType)editChoice(DISPLAY_COL2, y, "", STR_VTELEMSCREENTYPE, oldScreenType, 0, TELEMETRY_SCREEN_TYPE_MAX, (menuHorizontalPosition==0 ? attr : 0), event);
        if (newScreenType != oldScreenType) {
          g_model.screensType = (g_model.screensType & (~(0x03 << (2*screenIndex)))) | (newScreenType << (2*screenIndex));
          memset(&g_model.screens[screenIndex], 0, sizeof(g_model.screens[screenIndex]));
        }
#if defined(LUA)
        if (newScreenType == TELEMETRY_SCREEN_TYPE_SCRIPT) {
          TelemetryScriptData & scriptData = g_model.screens[screenIndex].script;

          // TODO better function name for ---
          // TODO function for these lines
          if (ZEXIST(scriptData.file))
            lcdDrawSizedText(DISPLAY_COL2+7*FW, y, scriptData.file, sizeof(scriptData.file), (menuHorizontalPosition==1 ? attr : 0));
          else
            lcdDrawTextAtIndex(DISPLAY_COL2+7*FW, y, STR_VCSWFUNC, 0, (menuHorizontalPosition==1 ? attr : 0));

          if (menuHorizontalPosition==1 && attr && event==EVT_KEY_BREAK(KEY_ENTER)) {
            s_editMode = 0;
            if (sdListFiles(SCRIPTS_TELEM_PATH, SCRIPTS_EXT, sizeof(g_model.screens[screenIndex].script.file), g_model.screens[screenIndex].script.file)) {
              POPUP_MENU_START(onTelemetryScriptFileSelectionMenu);
            }
            else {
              POPUP_WARNING(STR_NO_SCRIPTS_ON_SD);
            }
          }
        }
#endif
        break;
      }

      case ITEM_DISPLAY_SCREEN_LINE1:
      case ITEM_DISPLAY_SCREEN_LINE2:
      case ITEM_DISPLAY_SCREEN_LINE3:
      case ITEM_DISPLAY_SCREEN_LINE4:
      case ITEM_DISPLAY_SCREEN_LINE5:
      case ITEM_DISPLAY_SCREEN_LINE6:
      case ITEM_DISPLAY_SCREEN_LINE7:
      case ITEM_DISPLAY_SCREEN_LINE8:
      case ITEM_DISPLAY_SCREEN_LINE9:
      case ITEM_DISPLAY_SCREEN_LINE10:
      case ITEM_DISPLAY_SCREEN_LINE11:
      case ITEM_DISPLAY_SCREEN_LINE12:
      case ITEM_DISPLAY_SCREEN_LINE13:
      case ITEM_DISPLAY_SCREEN_LINE14:
      case ITEM_DISPLAY_SCREEN_LINE15:
      case ITEM_DISPLAY_SCREEN_LINE16:
      {
        uint8_t screenIndex, lineIndex;
        if (k < ITEM_DISPLAY_SCREEN_LABEL2) {
          screenIndex = 0;
          lineIndex = k-ITEM_DISPLAY_SCREEN_LINE1;
        }
        else if (k >= ITEM_DISPLAY_SCREEN_LABEL4) {
          screenIndex = 3;
          lineIndex = k-ITEM_DISPLAY_SCREEN_LINE13;
        }
        else if (k >= ITEM_DISPLAY_SCREEN_LABEL3) {
          screenIndex = 2;
          lineIndex = k-ITEM_DISPLAY_SCREEN_LINE9;
        }
        else {
          screenIndex = 1;
          lineIndex = k-ITEM_DISPLAY_SCREEN_LINE5;
        }

        if (IS_BARS_SCREEN(screenIndex)) {
          FrSkyBarData & bar = g_model.screens[screenIndex].bars[lineIndex];
          drawSource(DISPLAY_COL1, y, bar.source, menuHorizontalPosition == 0 ? attr : 0);
          int16_t barMax, barMin;
          LcdFlags lf = LEFT;
          getMixSrcRange(bar.source, barMin, barMax, &lf);
          if (bar.source) {
            if (bar.source <= MIXSRC_LAST_CH) {
              drawSourceCustomValue(DISPLAY_COL2, y, bar.source, calc100toRESX(bar.barMin), (menuHorizontalPosition==1 ? attr : 0) | lf);
              drawSourceCustomValue(DISPLAY_COL3, y, bar.source, calc100toRESX(bar.barMax), (menuHorizontalPosition==2 ? attr : 0) | lf);
            }
            else {
              drawSourceCustomValue(DISPLAY_COL2, y, bar.source, bar.barMin, (menuHorizontalPosition==1 ? attr : 0) | lf);
              drawSourceCustomValue(DISPLAY_COL3, y, bar.source, bar.barMax, (menuHorizontalPosition==2 ? attr : 0) | lf);
            }
          }
          if (attr && s_editMode>0) {
            switch (menuHorizontalPosition) {
              case 0:
                bar.source = checkIncDec(event, bar.source, 0, MIXSRC_LAST_TELEM, EE_MODEL|INCDEC_SOURCE|NO_INCDEC_MARKS, isSourceAvailable);
                if (checkIncDec_Ret) {
                  if (bar.source <= MIXSRC_LAST_CH) {
                    bar.barMin = -100;
                    bar.barMax = 100;
                  }
                  else {
                    bar.barMin = 0;
                    bar.barMax = 0;
                  }
                }
                break;
              case 1:
                bar.barMin = checkIncDec(event, bar.barMin, barMin, barMax, EE_MODEL|NO_INCDEC_MARKS);
                break;
              case 2:
                bar.barMax = checkIncDec(event, bar.barMax, barMin, barMax, EE_MODEL|NO_INCDEC_MARKS);
                break;
            }
          }
        }
        else {
          for (int c=0; c<NUM_LINE_ITEMS; c++) {
            LcdFlags cellAttr = (menuHorizontalPosition==c ? attr : 0);
            source_t * value = &g_model.screens[screenIndex].lines[lineIndex].sources[c];
            const coord_t pos[] = {DISPLAY_COL1, DISPLAY_COL2, DISPLAY_COL3};
            drawSource(pos[c], y, *value, cellAttr);
            if (cellAttr && s_editMode>0) {
              *value = checkIncDec(event, *value, 0, MIXSRC_LAST_TELEM, EE_MODEL|INCDEC_SOURCE|NO_INCDEC_MARKS, isSourceAvailable);
            }
          }
          if (attr && menuHorizontalPosition == NUM_LINE_ITEMS) {
            repeatLastCursorMove(event);
          }
        }
        break;
      }
    }
  }
}
