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

#define MODEL_SPECIAL_FUNC_1ST_COLUMN           (0)
#define MODEL_SPECIAL_FUNC_2ND_COLUMN           (4*FW-1)
#define MODEL_SPECIAL_FUNC_3RD_COLUMN           (15*FW-3)
#define MODEL_SPECIAL_FUNC_4TH_COLUMN           (19 * FW - 3)
#define MODEL_SPECIAL_FUNC_4TH_COLUMN_ONOFF     (19 * FW - 3)
#define MODEL_SPECIAL_FUNC_5TH_COLUMN_ONOFF     (20 * FW + 1)

#define SD_LOGS_PERIOD_MIN      1     // 0.1s  fastest period 
#define SD_LOGS_PERIOD_MAX      255   // 25.5s slowest period 
#define SD_LOGS_PERIOD_DEFAULT  10    // 1s    default period for newly created SF

#define PUSH_CS_DURATION_MIN 0       // 0     no duration : as long as switch is true
#define PUSH_CS_DURATION_MAX 255     // 25.5s longest duration

void onCustomFunctionsFileSelectionMenu(const char * result)
{
  int  sub = menuVerticalPosition - HEADER_LINE;
  CustomFunctionData * cfn;
  uint8_t eeFlags;

  if (menuHandlers[menuLevel] == menuModelSpecialFunctions) {
    cfn = &g_model.customFn[sub];
    eeFlags = EE_MODEL;
  }
  else {
    cfn = &g_eeGeneral.customFn[sub];
    eeFlags = EE_GENERAL;
  }

  uint8_t func = CFN_FUNC(cfn);

  if (result == STR_UPDATE_LIST) {
    char directory[256];
    if (func == FUNC_PLAY_SCRIPT) {
      strcpy(directory, SCRIPTS_FUNCS_PATH);
    }
    else if (func == FUNC_RGB_LED) {
      strcpy(directory, SCRIPTS_RGB_PATH);
    }
    else {
      strcpy(directory, SOUNDS_PATH);
      strncpy(directory+SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
    }
    if (!sdListFiles(directory, func==FUNC_PLAY_SCRIPT || func==FUNC_RGB_LED ? SCRIPTS_EXT : SOUNDS_EXT, sizeof(cfn->play.name), nullptr)) {
      POPUP_WARNING(func==FUNC_PLAY_SCRIPT  || func==FUNC_RGB_LED ? STR_NO_SCRIPTS_ON_SD : STR_NO_SOUNDS_ON_SD);
    }
  }
  else if (result != STR_EXIT) {
    // The user choosed a file in the list
    memcpy(cfn->play.name, result, sizeof(cfn->play.name));
    storageDirty(eeFlags);
    if (CFN_ACTIVE(cfn)  && (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED)) {
      LUA_LOAD_MODEL_SCRIPTS();
    }
  }
}

#if defined(PCBTARANIS)
void onAdjustGvarSourceLongEnterPress(const char * result)
{
  CustomFunctionData * cfn = &g_model.customFn[menuVerticalPosition];

  if (result == STR_CONSTANT) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_CONSTANT;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_MIXSOURCE) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_SOURCE;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_MIXSOURCERAW) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_SOURCERAW;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_GLOBALVAR) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_GVAR;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else if (result == STR_INCDEC) {
    CFN_GVAR_MODE(cfn) = FUNC_ADJUST_GVAR_INCDEC;
    CFN_PARAM(cfn) = 0;
    storageDirty(EE_MODEL);
  }
  else if (result != STR_EXIT) {
    onSourceLongEnterPress(result);
  }
}

void onCustomFunctionsMenu(const char * result)
{
  int sub = menuVerticalPosition - HEADER_LINE;
  CustomFunctionData * cfn;
  uint8_t eeFlags;

  if (menuHandlers[menuLevel] == menuModelSpecialFunctions) {
    cfn = &g_model.customFn[sub];
    eeFlags = EE_MODEL;
  }
  else {
    cfn = &g_eeGeneral.customFn[sub];
    eeFlags = EE_GENERAL;
  }

  if (result == STR_COPY) {
    clipboard.type = CLIPBOARD_TYPE_CUSTOM_FUNCTION;
    clipboard.data.cfn = *cfn;
  }
  else if (result == STR_PASTE) {
    *cfn = clipboard.data.cfn;
    storageDirty(eeFlags);
  }
  else if (result == STR_CLEAR) {
    memset(cfn, 0, sizeof(CustomFunctionData));
    storageDirty(eeFlags);
  }
  else if (result == STR_INSERT) {
    memmove(cfn+1, cfn, (MAX_SPECIAL_FUNCTIONS-sub-1)*sizeof(CustomFunctionData));
    memset(cfn, 0, sizeof(CustomFunctionData));
    storageDirty(eeFlags);
  }
  else if (result == STR_DELETE) {
    memmove(cfn, cfn+1, (MAX_SPECIAL_FUNCTIONS-sub-1)*sizeof(CustomFunctionData));
    memset(&g_model.customFn[MAX_SPECIAL_FUNCTIONS-1], 0, sizeof(CustomFunctionData));
    storageDirty(eeFlags);
  }
}
#endif // PCBTARANIS

static bool isAssignableFunctionAvailableSorted(int value)
{
  return isAssignableFunctionAvailable(cfn_sorted[value]);
}

void menuSpecialFunctions(event_t event, CustomFunctionData * functions, CustomFunctionsContext * functionsContext)
{
  int8_t sub = menuVerticalPosition - HEADER_LINE;

  uint8_t eeFlags = (functions == g_model.customFn) ? EE_MODEL : EE_GENERAL;

#if defined(PCBTARANIS)
#if defined(PCBXLITE)
  // ENT LONG on xlite brings up switch type menu, so this menu is activated with SHIFT + ENT LONG
  if (menuHorizontalPosition==0 && event==EVT_KEY_LONG(KEY_ENTER) && keysGetState(KEY_SHIFT)) {
#else
  if (menuHorizontalPosition<0 && event==EVT_KEY_LONG(KEY_ENTER)) {
#endif
    CustomFunctionData *cfn = &functions[sub];
    if (!CFN_EMPTY(cfn))
      POPUP_MENU_ADD_ITEM(STR_COPY);
    if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION && isAssignableFunctionAvailable(clipboard.data.cfn.func))
      POPUP_MENU_ADD_ITEM(STR_PASTE);
    if (!CFN_EMPTY(cfn) && CFN_EMPTY(&functions[MAX_SPECIAL_FUNCTIONS-1]))
      POPUP_MENU_ADD_ITEM(STR_INSERT);
    if (!CFN_EMPTY(cfn))
      POPUP_MENU_ADD_ITEM(STR_CLEAR);
    for (int i=sub+1; i<MAX_SPECIAL_FUNCTIONS; i++) {
      if (!CFN_EMPTY(&functions[i])) {
        POPUP_MENU_ADD_ITEM(STR_DELETE);
        break;
      }
    }
    POPUP_MENU_START(onCustomFunctionsMenu);
  }
#endif // PCBTARANIS

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i+menuVerticalOffset;

    CustomFunctionData * cfn = &functions[k];
    uint8_t func = CFN_FUNC(cfn);
    for (uint8_t j=0; j<6; j++) {
      uint8_t attr = ((sub==k && menuHorizontalPosition==j) ? ((s_editMode>0) ? BLINK|INVERS : INVERS) : 0);
      uint8_t active = (attr && s_editMode > 0);
      switch (j) {
        case 0:
          if (sub==k && menuHorizontalPosition < 1 && CFN_SWITCH(cfn) == SWSRC_NONE) {
            CFN_ACTIVE(cfn) = 0; // Default is disabled
            drawSwitch(MODEL_SPECIAL_FUNC_1ST_COLUMN, y, CFN_SWITCH(cfn), attr | INVERS | ((functionsContext->activeSwitches & ((MASK_CFN_TYPE)1 << k)) ? BOLD : 0));
            if (active) CHECK_INCDEC_SWITCH(event, CFN_SWITCH(cfn), SWSRC_FIRST, SWSRC_LAST, eeFlags, isSwitchAvailableInCustomFunctions);
          }
          else {
            drawSwitch(MODEL_SPECIAL_FUNC_1ST_COLUMN, y, CFN_SWITCH(cfn), attr | ((functionsContext->activeSwitches & ((MASK_CFN_TYPE)1 << k)) ? BOLD : 0));
            if (active || AUTOSWITCH_ENTER_LONG()) {
              if (event == EVT_KEY_LONG(KEY_ENTER))
                killEvents(event);
              CHECK_INCDEC_SWITCH(event, CFN_SWITCH(cfn), SWSRC_FIRST, SWSRC_LAST, eeFlags, isSwitchAvailableInCustomFunctions);
            }
          }
          if (func == FUNC_OVERRIDE_CHANNEL && functions != g_model.customFn) {
            func = CFN_FUNC(cfn) = func+1;
          }
          break;

        case 1:
          if (CFN_SWITCH(cfn)) {
            lcdDrawText(MODEL_SPECIAL_FUNC_2ND_COLUMN, y, funcGetLabel(func), attr);
            if (active) {
              Functions newFunc = cfn_sorted[checkIncDec(event, getFuncSortIdx(CFN_FUNC(cfn)), 0, FUNC_MAX-1, eeFlags, isAssignableFunctionAvailableSorted)];
              // If changing from Lua script then reload to remove old reference
              if ((CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT || CFN_FUNC(cfn) == FUNC_RGB_LED) && newFunc != FUNC_PLAY_SCRIPT && newFunc != FUNC_RGB_LED)
                LUA_LOAD_MODEL_SCRIPTS();
              CFN_FUNC(cfn) = newFunc;
              if (checkIncDec_Ret) CFN_RESET(cfn);
            }
          }
          else {
            j = 5; // skip other fields
            if (sub==k && menuHorizontalPosition > 0) {
              repeatLastCursorHorMove(event);
            }
          }
          break;

        case 2:
        {
          int8_t maxParam = MAX_OUTPUT_CHANNELS-1;
#if defined(OVERRIDE_CHANNEL_FUNCTION)
          if (func == FUNC_OVERRIDE_CHANNEL) {
            putsChn(lcdNextPos, y, CFN_CH_INDEX(cfn)+1, attr);
          }
          else
#endif
          if (func == FUNC_TRAINER) {
            maxParam = MAX_STICKS + 1;
            uint8_t param = CFN_CH_INDEX(cfn);
            if (param == 0)
              lcdDrawText(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_STICKS, attr);
            else if (param == MAX_STICKS + 1)
              lcdDrawText(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_CHANS, attr);
            else
              drawSource(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, MIXSRC_FIRST_STICK + param - 1, attr);
            if (active) CHECK_INCDEC_MODELVAR_ZERO(event, CFN_CH_INDEX(cfn), maxParam);
          }
#if defined(GVARS)
          else if (func == FUNC_ADJUST_GVAR) {
            maxParam = MAX_GVARS - 1;
            drawStringWithIndex(lcdNextPos + 2, y, STR_GV, CFN_GVAR_INDEX(cfn)+1, attr);
            if (active) CFN_GVAR_INDEX(cfn) = checkIncDec(event, CFN_GVAR_INDEX(cfn), 0, maxParam, eeFlags);
          }
#endif // GVARS
          else if (func == FUNC_SET_TIMER) {
            if (timersSetupCount()> 0) {
              maxParam = MAX_TIMERS - 1;
              lcdDrawTextAtIndex(lcdNextPos, y, STR_VFSWRESET, CFN_TIMER_INDEX(cfn), attr);
              if (active) CFN_TIMER_INDEX(cfn) = checkIncDec(event, CFN_TIMER_INDEX(cfn), 0, maxParam, eeFlags, isTimerSourceAvailable);
            } else {
              lcdDrawText(lcdNextPos + FW, y, STR_NO_TIMERS, 0);
              if (attr)
                repeatLastCursorHorMove(event);
            }
          }
#if defined(FUNCTION_SWITCHES)
          else if (func == FUNC_PUSH_CUST_SWITCH) {
            uint8_t sw = switchGetSwitchFromCustomIdx(CFN_CS_INDEX(cfn));
            lcdDrawText(lcdNextPos + 5, y, switchGetDefaultName(sw), attr);
            if (active) CFN_CS_INDEX(cfn) = switchGetCustomSwitchIdx(checkIncDec(event, sw, 0, switchGetMaxSwitches() - 1, eeFlags, switchIsCustomSwitch));
          }
#endif          
          else if (attr) {
            repeatLastCursorHorMove(event);
          }
          break;
        }

        case 3:
        {
          INCDEC_DECLARE_VARS(eeFlags);
          int16_t val_displayed = CFN_PARAM(cfn);
          int16_t val_min = 0;
          int16_t val_max = 255;
          if (func == FUNC_RESET) {
            val_max = FUNC_RESET_PARAM_FIRST_TELEM+lastUsedTelemetryIndex();
            int param = CFN_PARAM(cfn);
            if (param < FUNC_RESET_PARAM_FIRST_TELEM) {
              lcdDrawTextAtIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_VFSWRESET, param, attr);
            }
            else {
              TelemetrySensor * sensor = & g_model.telemetrySensors[param-FUNC_RESET_PARAM_FIRST_TELEM];
              lcdDrawSizedText(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, sensor->label, TELEM_LABEL_LEN, attr);
            }
            if (active) INCDEC_ENABLE_CHECK(functionsContext == &globalFunctionsContext ? isSourceAvailableInGlobalResetSpecialFunction : isSourceAvailableInResetSpecialFunction);
          }
#if defined(OVERRIDE_CHANNEL_FUNCTION)
          else if (func == FUNC_OVERRIDE_CHANNEL) {
            getMixSrcRange(MIXSRC_FIRST_CH, val_min, val_max);
            lcdDrawNumber(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT);
          }
#endif // OVERRIDE_CHANNEL_FUNCTION
#if defined(DANGEROUS_MODULE_FUNCTIONS)
          else if (func >= FUNC_RANGECHECK && func <= FUNC_BIND) {
            val_max = NUM_MODULES-1;
            const char *text[] = {"Int.", "Ext."};
            lcdDrawTextAtIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, text, CFN_PARAM(cfn), attr);
          }
#endif
          else if (func == FUNC_SET_TIMER) {
            if (timersSetupCount() > 0) {
              getMixSrcRange(MIXSRC_FIRST_TIMER, val_min, val_max);
              drawTimer(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT, attr);
            } else if (attr) {
              repeatLastCursorHorMove(event);
            }
          }
#if defined(AUDIO)
          else if (func == FUNC_PLAY_SOUND) {
            val_max = AU_SPECIAL_SOUND_LAST-AU_SPECIAL_SOUND_FIRST-1;
            lcdDrawTextAtIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_FUNCSOUNDS, val_displayed, attr);
          }
#endif
          else if (func == FUNC_SET_SCREEN) {
            val_min = 0;
            val_max = 4;
            lcdDrawNumber(MODEL_SPECIAL_FUNC_3RD_COLUMN + 3*FW, y, val_displayed, attr|LEFT);
          }
#if defined(HAPTIC)
          else if (func == FUNC_HAPTIC) {
            val_max = 3;
            lcdDrawNumber(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|LEFT);
          }
#endif
          else if (func == FUNC_PLAY_TRACK || func == FUNC_BACKGND_MUSIC || func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED) {
            coord_t x = MODEL_SPECIAL_FUNC_3RD_COLUMN - 6;
            if (func == FUNC_PLAY_SCRIPT)
              x = x - 5 * FW;
            else if (func == FUNC_PLAY_TRACK)
              x = x - 3 * FW;
            else if (func == FUNC_BACKGND_MUSIC || func == FUNC_RGB_LED)
              x = x - 2 * FW;
            if (ZEXIST(cfn->play.name))
              lcdDrawSizedText(x, y, cfn->play.name, sizeof(cfn->play.name), attr);
            else
              lcdDrawTextAtIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_VCSWFUNC, 0, attr);
            if (active && event==EVT_KEY_BREAK(KEY_ENTER)) {
              s_editMode = 0;
              char directory[256];
              if (func==FUNC_PLAY_SCRIPT) {
                strcpy(directory, SCRIPTS_FUNCS_PATH);
              }
              else if (func==FUNC_RGB_LED) {
                strcpy(directory, SCRIPTS_RGB_PATH);
              }
              else {
                strcpy(directory, SOUNDS_PATH);
                strncpy(directory+SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
              }
              if (sdListFiles(directory, func==FUNC_PLAY_SCRIPT || func==FUNC_RGB_LED ? SCRIPTS_EXT : SOUNDS_EXT, sizeof(cfn->play.name), cfn->play.name)) {
                POPUP_MENU_START(onCustomFunctionsFileSelectionMenu);
              }
              else {
                POPUP_WARNING(func==FUNC_PLAY_SCRIPT || func==FUNC_RGB_LED ? STR_NO_SCRIPTS_ON_SD : STR_NO_SOUNDS_ON_SD);
              }
            }
            break;
          }
          else if (func == FUNC_PLAY_VALUE) {
            val_max = MIXSRC_LAST_TELEM;
            drawSource(MODEL_SPECIAL_FUNC_3RD_COLUMN - (val_displayed == 0 ? 0 : 2 * FW), y, val_displayed, attr);
            if (active) {
              INCDEC_SET_FLAG(eeFlags | INCDEC_SOURCE | INCDEC_SOURCE_INVERT);
              INCDEC_ENABLE_CHECK(functionsContext == &globalFunctionsContext ? isSourceAvailableInGlobalFunctions : isSourceAvailable);
            }
          }
          else if (func == FUNC_VOLUME) {
            val_max = MIXSRC_LAST_CH;
            drawSource(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr);
            if (active) {
              INCDEC_SET_FLAG(eeFlags | INCDEC_SOURCE | INCDEC_SOURCE_INVERT);
              INCDEC_ENABLE_CHECK(isSourceAvailable);
            }
          }
          else if (func == FUNC_BACKLIGHT) {
            val_max = MIXSRC_LAST_CH;
            drawSource(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr);
            if (active) {
              INCDEC_SET_FLAG(eeFlags | INCDEC_SOURCE | INCDEC_SOURCE_INVERT);
              INCDEC_ENABLE_CHECK(isSourceAvailable);
            }
          }
#if defined(FUNCTION_SWITCHES)
          else if (func == FUNC_PUSH_CUST_SWITCH) {
            val_min = PUSH_CS_DURATION_MIN;
            val_max = PUSH_CS_DURATION_MAX;

            lcdDrawNumber(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|PREC1|LEFT);
            lcdDrawChar(lcdLastRightPos, y, 's');
          }
#endif
          else if (func == FUNC_LOGS) {
            val_min = SD_LOGS_PERIOD_MIN; 
            val_max = SD_LOGS_PERIOD_MAX;

            if (!val_displayed) {
              val_displayed = CFN_PARAM(cfn) = SD_LOGS_PERIOD_DEFAULT;
            }

            lcdDrawNumber(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr|PREC1|LEFT);
            lcdDrawChar(lcdLastRightPos, y, 's');
          }
#if defined(GVARS)
          else if (func == FUNC_ADJUST_GVAR) {
            switch (CFN_GVAR_MODE(cfn)) {
              case FUNC_ADJUST_GVAR_CONSTANT:
                val_displayed = (int16_t)CFN_PARAM(cfn);
                getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min, val_max);
                drawGVarValue(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, CFN_GVAR_INDEX(cfn), val_displayed, attr|LEFT);
                break;
              case FUNC_ADJUST_GVAR_SOURCE:
              case FUNC_ADJUST_GVAR_SOURCERAW:
                val_max = MIXSRC_LAST_CH;
                drawSource(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, val_displayed, attr);
                if (active) {
                  INCDEC_SET_FLAG(eeFlags | INCDEC_SOURCE | INCDEC_SOURCE_INVERT);
                  INCDEC_ENABLE_CHECK(isSourceAvailable);
                }
                break;
              case FUNC_ADJUST_GVAR_GVAR:
                val_max = MAX_GVARS-1;
                drawStringWithIndex(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, STR_GV, val_displayed+1, attr);
                break;
              default: // FUNC_ADJUST_GVAR_INC
                getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min, val_max);
                getGVarIncDecRange(val_min, val_max);
                lcdDrawText(MODEL_SPECIAL_FUNC_3RD_COLUMN, y, (val_displayed < 0 ? "-= " : "+= "), attr);
                drawGVarValue(lcdNextPos, y, CFN_GVAR_INDEX(cfn), abs(val_displayed), attr|LEFT);
                break;
            }

#if !defined(NAVIGATION_X7)
            // For X7 type navigation the ENTER long press is handled below
            if (attr && event==EVT_KEY_LONG(KEY_ENTER)) {
              killEvents(event);
              s_editMode = !s_editMode;
              active = true;
              CFN_GVAR_MODE(cfn) += 1;
              CFN_GVAR_MODE(cfn) &= 0x03;
              val_displayed = 0;
            }
#endif
          }
#endif // GVARS
          else if (attr) {
            repeatLastCursorHorMove(event);
          }
#if defined(NAVIGATION_X7)
          if (active || event==EVT_KEY_LONG(KEY_ENTER)) {
            CFN_PARAM(cfn) = CHECK_INCDEC_PARAM(event, val_displayed, val_min, val_max);
            if (func == FUNC_ADJUST_GVAR && attr && event==EVT_KEY_LONG(KEY_ENTER)) {
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_CONSTANT)
                POPUP_MENU_ADD_ITEM(STR_CONSTANT);
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_SOURCE)
                POPUP_MENU_ADD_ITEM(STR_MIXSOURCE);
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_SOURCERAW)
                POPUP_MENU_ADD_ITEM(STR_MIXSOURCERAW);
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_GVAR)
                POPUP_MENU_ADD_ITEM(STR_GLOBALVAR);
              if (CFN_GVAR_MODE(cfn) != FUNC_ADJUST_GVAR_INCDEC)
                POPUP_MENU_ADD_ITEM(STR_INCDEC);
              POPUP_MENU_START(onAdjustGvarSourceLongEnterPress);
              s_editMode = EDIT_MODIFY_FIELD;
            }
#else
          if (active) {
            CFN_PARAM(cfn) = CHECK_INCDEC_PARAM(event, val_displayed, val_min, val_max);
#endif
          }
          break;
        }

        case 4:
          if (HAS_REPEAT_PARAM(func)) {
            if (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED) {
              lcdDrawText(MODEL_SPECIAL_FUNC_4TH_COLUMN_ONOFF-3, y, (CFN_PLAY_REPEAT(cfn) == 0) ? "On" : "1x", attr);
              if (active) CFN_PLAY_REPEAT(cfn) = checkIncDec(event, CFN_PLAY_REPEAT(cfn), 0, 1, eeFlags);
            }
            else {
              if (CFN_PLAY_REPEAT(cfn) == 0) {
                lcdDrawChar(MODEL_SPECIAL_FUNC_4TH_COLUMN_ONOFF+3, y, '-', attr);
              }
              else if (CFN_PLAY_REPEAT(cfn) == CFN_PLAY_REPEAT_NOSTART) {
                lcdDrawText(MODEL_SPECIAL_FUNC_4TH_COLUMN_ONOFF+1, y, "!-", attr);
              }
              else {
                lcdDrawNumber(MODEL_SPECIAL_FUNC_4TH_COLUMN+2+FW, y, CFN_PLAY_REPEAT(cfn)*CFN_PLAY_REPEAT_MUL, RIGHT | attr);
              }
              if (active)
                CFN_PLAY_REPEAT(cfn) = checkIncDec(event, CFN_PLAY_REPEAT(cfn)==CFN_PLAY_REPEAT_NOSTART?-1:CFN_PLAY_REPEAT(cfn),
                                                   -1,
                                                   (func == FUNC_SET_SCREEN ? 0 : 60/CFN_PLAY_REPEAT_MUL),
                                                   eeFlags);
            }
          }
          else if (attr) {
            repeatLastCursorHorMove(event);
          }
          break;
        
        case 5:
            drawCheckBox(MODEL_SPECIAL_FUNC_5TH_COLUMN_ONOFF, y, CFN_ACTIVE(cfn), attr);
            if (active) {
              CFN_ACTIVE(cfn) = checkIncDec(event, CFN_ACTIVE(cfn), 0, 1, eeFlags);
              if (checkIncDec_Ret && (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED)) {
                LUA_LOAD_MODEL_SCRIPTS();
              }
            }
            break;
      }
    }
#if defined(NAVIGATION_X7)
    if (sub==k && menuHorizontalPosition<0 && CFN_SWITCH(cfn)) {
      lcdInvertLine(i+1);
    }
#endif
  }
}

void menuModelSpecialFunctions(event_t event)
{
#if defined(NAVIGATION_X7)
  const CustomFunctionData * cfn = &g_model.customFn[menuVerticalPosition];
  if (!CFN_SWITCH(cfn) && menuHorizontalPosition < 0 && event==EVT_KEY_BREAK(KEY_ENTER)) {
    menuHorizontalPosition = 0;
  }
#endif
  MENU(STR_MENUCUSTOMFUNC, menuTabModel, MENU_MODEL_SPECIAL_FUNCTIONS, HEADER_LINE+MAX_SPECIAL_FUNCTIONS, { HEADER_LINE_COLUMNS NAVIGATION_LINE_BY_LINE|5/*repeated*/ });

  menuSpecialFunctions(event, g_model.customFn, &modelFunctionsContext);

#if defined(NAVIGATION_X7)
  if (!CFN_SWITCH(cfn) && menuHorizontalPosition == 0 && s_editMode <= 0) {
    menuHorizontalPosition = -1;
  }
#endif
}
