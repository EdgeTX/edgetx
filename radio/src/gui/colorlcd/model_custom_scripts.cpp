/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
  else {
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

#define SCRIPT_ONE_2ND_COLUMN_POS  (120)
#define SCRIPT_ONE_3RD_COLUMN_POS  (240)

bool menuModelCustomScriptOne(event_t event)
{
  ScriptData &sd = g_model.scriptsData[s_currIdx];

  // drawStringWithIndex(lcdLastRightPos+FW, 0, "LUA", s_currIdx+1, 0);

  SUBMENU(STR_MENUCUSTOMSCRIPTS, ICON_MODEL_LUA_SCRIPTS, 3+scriptInputsOutputs[s_currIdx].inputsCount, { 0, 0, LABEL(inputs), 0/*repeated*/ });

  int8_t sub = menuVerticalPosition;

  for (int k=0; k<NUM_BODY_LINES; k++) {
    coord_t y = MENU_CONTENT_TOP + k*FH;
    int i = k + menuVerticalOffset;
    LcdFlags attr = (sub==i ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

    if (i == ITEM_MODEL_CUSTOMSCRIPT_FILE) {
      lcdDrawText(MENUS_MARGIN_LEFT, y, STR_SCRIPT);
      if (ZEXIST(sd.file))
        lcdDrawSizedText(SCRIPT_ONE_2ND_COLUMN_POS, y, sd.file, sizeof(sd.file), attr);
      else
        lcdDrawTextAtIndex(SCRIPT_ONE_2ND_COLUMN_POS, y, STR_VCSWFUNC, 0, attr);
      if (attr) s_editMode = 0;
      if (attr && event==EVT_KEY_FIRST(KEY_ENTER) && !READ_ONLY()) {
        killEvents(KEY_ENTER);
        if (sdListFiles(SCRIPTS_MIXES_PATH, SCRIPTS_EXT, sizeof(sd.file), sd.file, LIST_NONE_SD_FILE)) {
          POPUP_MENU_START(onModelCustomScriptMenu);
        }
        else {
          POPUP_WARNING(STR_NO_SCRIPTS_ON_SD);
        }
      }
    }
    else if (i == ITEM_MODEL_CUSTOMSCRIPT_NAME) {
      lcdDrawText(MENUS_MARGIN_LEFT, y, TR_NAME);
      editName(SCRIPT_ONE_2ND_COLUMN_POS, y, sd.name, sizeof(sd.name), event, attr);
    }
    else if (i == ITEM_MODEL_CUSTOMSCRIPT_PARAMS_LABEL) {
      lcdDrawText(MENUS_MARGIN_LEFT, y, STR_INPUTS);
    }
    else if (i <= ITEM_MODEL_CUSTOMSCRIPT_PARAMS_LABEL+scriptInputsOutputs[s_currIdx].inputsCount) {
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
    lcdDrawSolidVerticalLine(SCRIPT_ONE_3RD_COLUMN_POS-4, DEFAULT_SCROLLBAR_Y, DEFAULT_SCROLLBAR_H+5, DEFAULT_COLOR);
    // lcdDrawText(SCRIPT_ONE_3RD_COLUMN_POS, FH+1, STR_OUTPUTS);

    for (int i=0; i<scriptInputsOutputs[s_currIdx].outputsCount; i++) {
      drawSource(SCRIPT_ONE_3RD_COLUMN_POS+INDENT_WIDTH, MENU_CONTENT_TOP+i*FH, MIXSRC_FIRST_LUA+(s_currIdx*MAX_SCRIPT_OUTPUTS)+i, 0);
      lcdDrawNumber(SCRIPT_ONE_3RD_COLUMN_POS+130, MENU_CONTENT_TOP+i*FH, calcRESXto1000(scriptInputsOutputs[s_currIdx].outputs[i].value), PREC1);
    }
  }

  return true;
}

#define SCRIPTS_COLUMN_FILE  70
#define SCRIPTS_COLUMN_NAME  160
#define SCRIPTS_COLUMN_STATE 300

bool menuModelCustomScripts(event_t event)
{
  // lcdDrawNumber(19*FW, 0, luaGetMemUsed(lsScripts), 0);
  // lcdDrawText(19*FW+1, 0, STR_BYTES);

  MENU(STR_MENUCUSTOMSCRIPTS, MODEL_ICONS, menuTabModel, MENU_MODEL_CUSTOM_SCRIPTS, MAX_SCRIPTS, { NAVIGATION_LINE_BY_LINE|3/*repeated*/ });

  int8_t  sub = menuVerticalPosition;

  if (event == EVT_KEY_FIRST(KEY_ENTER) && sub >= 0) {
    s_currIdx = sub;
    pushMenu(menuModelCustomScriptOne);
  }

  for (int i=0, scriptIndex=0; i<MAX_SCRIPTS; i++) {
    coord_t y = MENU_CONTENT_TOP + i*FH;

    ScriptData &sd = g_model.scriptsData[i];

    // LUAx header
    drawStringWithIndex(MENUS_MARGIN_LEFT, y, "LUA", i+1, sub==i ? INVERS : 0);

    // LUA script
    if (ZEXIST(sd.file)) {
      lcdDrawSizedText(SCRIPTS_COLUMN_FILE, y, sd.file, sizeof(sd.file), 0);
      switch (scriptInternalData[scriptIndex].state) {
        case SCRIPT_SYNTAX_ERROR:
          lcdDrawText(SCRIPTS_COLUMN_STATE, y, "(error)");
          break;
        case SCRIPT_KILLED:
          lcdDrawText(SCRIPTS_COLUMN_STATE, y, "(killed)");
          break;
        default:
          lcdDrawNumber(SCRIPTS_COLUMN_STATE, y, luaGetCpuUsed(scriptIndex), LEFT|DEFAULT_COLOR, 0, nullptr, "%");
          break;
      }
      scriptIndex++;
    }
    else {
      lcdDrawTextAtIndex(SCRIPTS_COLUMN_FILE, y, STR_VCSWFUNC, 0, 0);
    }

    // Script name
    lcdDrawSizedText(SCRIPTS_COLUMN_NAME, y, sd.name, sizeof(sd.name), ZCHAR);
  }

  return true;
}
