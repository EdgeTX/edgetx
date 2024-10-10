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

void onModelCustomScriptMenu(const char *result)
{
  ScriptData &sd = g_model.scriptsData[s_currIdx];

  if (result == STR_UPDATE_LIST) {
    if (!sdListFiles(SCRIPTS_MIXES_PATH, SCRIPTS_EXT, sizeof(sd.file), nullptr)) {
      POPUP_WARNING(STR_NO_SCRIPTS_ON_SD);
    }
  }
  else if (result != STR_EXIT) {
    // The user choosed a lua file in the list
    copySelection(sd.file, result, sizeof(sd.file));
    memset(sd.inputs, 0, sizeof(sd.inputs));
    storageDirty(EE_MODEL);
    LUA_LOAD_MODEL_SCRIPT(s_currIdx);
  }
}

enum menuModelCustomScriptItems {
  ITEM_MODEL_CUSTOMSCRIPT_FILE,
  ITEM_MODEL_CUSTOMSCRIPT_NAME,
  ITEM_MODEL_CUSTOMSCRIPT_PARAMS_LABEL,
};

#define SCRIPT_ONE_2ND_COLUMN_POS  (12*FW)
#define SCRIPT_ONE_3RD_COLUMN_POS  (23*FW)

void menuModelCustomScriptOne(event_t event)
{
  ScriptData & sd = g_model.scriptsData[s_currIdx];

  drawStringWithIndex(PSIZE(TR_MENUCUSTOMSCRIPTS)*FW+FW, 0, "LUA", s_currIdx+1, 0);
  lcdDrawFilledRect(0, 0, LCD_W, FH, SOLID, FILL_WHITE|GREY_DEFAULT);

  uint8_t old_editMode = s_editMode;

  SUBMENU(STR_MENUCUSTOMSCRIPTS, 3+scriptInputsOutputs[s_currIdx].inputsCount, { 0, 0, LABEL(inputs), 0/*repeated*/ });

  int8_t sub = menuVerticalPosition;

  for (int k=0; k<LCD_LINES-1; k++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + k*FH;
    int i = k + menuVerticalOffset;
    LcdFlags attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

    if (i == ITEM_MODEL_CUSTOMSCRIPT_FILE) {
      lcdDrawTextAlignedLeft(y, STR_SCRIPT);
      if (ZEXIST(sd.file))
        lcdDrawSizedText(SCRIPT_ONE_2ND_COLUMN_POS, y, sd.file, sizeof(sd.file), attr);
      else
        lcdDrawTextAtIndex(SCRIPT_ONE_2ND_COLUMN_POS, y, STR_VCSWFUNC, 0, attr);
      if (attr && event==EVT_KEY_BREAK(KEY_ENTER)) {
        s_editMode = 0;
        if (sdListFiles(SCRIPTS_MIXES_PATH, SCRIPTS_EXT, sizeof(sd.file), sd.file, LIST_NONE_SD_FILE)) {
          POPUP_MENU_START(onModelCustomScriptMenu);
        }
        else {
          POPUP_WARNING(STR_NO_SCRIPTS_ON_SD);
        }
      }
    }
    else if (i == ITEM_MODEL_CUSTOMSCRIPT_NAME) {
      lcdDrawTextAlignedLeft(y, TR_NAME);
      editName(SCRIPT_ONE_2ND_COLUMN_POS, y, sd.name, sizeof(sd.name), event,
               (attr != 0), attr, old_editMode);
    } else if (i == ITEM_MODEL_CUSTOMSCRIPT_PARAMS_LABEL) {
      lcdDrawTextAlignedLeft(y, STR_INPUTS);
    } else if (i <= ITEM_MODEL_CUSTOMSCRIPT_PARAMS_LABEL +
                        scriptInputsOutputs[s_currIdx].inputsCount) {
      int inputIdx = i-ITEM_MODEL_CUSTOMSCRIPT_PARAMS_LABEL-1;
      lcdDrawSizedText(INDENT_WIDTH, y, scriptInputsOutputs[s_currIdx].inputs[inputIdx].name, 10, 0);
      if (scriptInputsOutputs[s_currIdx].inputs[inputIdx].type == INPUT_TYPE_VALUE) {
        lcdDrawNumber(SCRIPT_ONE_2ND_COLUMN_POS, y, g_model.scriptsData[s_currIdx].inputs[inputIdx].value+ \
                                                    scriptInputsOutputs[s_currIdx].inputs[inputIdx].def, attr|LEFT);
        if (attr) {
          CHECK_INCDEC_MODELVAR(event, g_model.scriptsData[s_currIdx].inputs[inputIdx].value, \
                                scriptInputsOutputs[s_currIdx].inputs[inputIdx].min-scriptInputsOutputs[s_currIdx].inputs[inputIdx].def, \
                                scriptInputsOutputs[s_currIdx].inputs[inputIdx].max-scriptInputsOutputs[s_currIdx].inputs[inputIdx].def);
        }
      }
      else {
        drawSource(SCRIPT_ONE_2ND_COLUMN_POS, y, g_model.scriptsData[s_currIdx].inputs[inputIdx].source, attr);
        if (attr) {
          CHECK_INCDEC_MODELSOURCE(event, g_model.scriptsData[s_currIdx].inputs[inputIdx].source, 0, MIXSRC_LAST_TELEM);
        }
      }
    }
  }

  if (scriptInputsOutputs[s_currIdx].outputsCount > 0) {
    lcdDrawSolidVerticalLine(SCRIPT_ONE_3RD_COLUMN_POS-4, FH+1, LCD_H-FH-1);
    lcdDrawText(SCRIPT_ONE_3RD_COLUMN_POS, FH+1, STR_OUTPUTS);

    for (int i=0; i<scriptInputsOutputs[s_currIdx].outputsCount; i++) {
      drawSource(SCRIPT_ONE_3RD_COLUMN_POS+INDENT_WIDTH, FH+1+FH+i*FH, MIXSRC_FIRST_LUA+(s_currIdx*MAX_SCRIPT_OUTPUTS)+i, 0);
      lcdDrawNumber(SCRIPT_ONE_3RD_COLUMN_POS+11*FW+3, FH+1+FH+i*FH, calcRESXto1000(scriptInputsOutputs[s_currIdx].outputs[i].value), PREC1|RIGHT);
    }
  }
}

void menuModelCustomScripts(event_t event)
{
  lcdDrawNumber(19*FW, 0, luaGetMemUsed(lsScripts), RIGHT);
  lcdDrawText(19*FW+1, 0, STR_BYTES);

  MENU(STR_MENUCUSTOMSCRIPTS, menuTabModel, MENU_MODEL_CUSTOM_SCRIPTS, MAX_SCRIPTS, { NAVIGATION_LINE_BY_LINE|3/*repeated*/ });

  coord_t y;
  int8_t  sub = menuVerticalPosition;

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    s_currIdx = sub;
    pushMenu(menuModelCustomScriptOne);
  }

  for (int i=0, scriptIndex=0; i<MAX_SCRIPTS; i++) {
    y = 1 + (i+1)*FH;

    ScriptData &sd = g_model.scriptsData[i];

    // LUAx header
    drawStringWithIndex(0, y, "LUA", i+1, sub==i ? INVERS : 0);

    // LUA script
    if (ZEXIST(sd.file)) {
      lcdDrawSizedText(5*FW, y, sd.file, sizeof(sd.file), 0);
      switch (scriptInternalData[scriptIndex].state) {
        case SCRIPT_SYNTAX_ERROR:
          lcdDrawText(30*FW+2, y, "(error)");
          break;
        default:
          lcdDrawNumber(34*FW, y, luaGetCpuUsed(scriptIndex), RIGHT);
          lcdDrawChar(34*FW, y, '%');
          break;
      }
      scriptIndex++;
    }
    else {
      lcdDrawTextAtIndex(5*FW, y, STR_VCSWFUNC, 0, 0);
    }

    // Script name
    lcdDrawSizedText(16*FW, y, sd.name, sizeof(sd.name), ZCHAR);
  }
}
