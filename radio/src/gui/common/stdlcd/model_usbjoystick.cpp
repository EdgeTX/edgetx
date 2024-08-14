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
#include "usb_joystick.h"

#define _STR_MAX(x)                     "/" #x
#define STR_MAX(x)                     _STR_MAX(x)

#if LCD_W >= 212
  #define MODE_POS          5*FW
  #define SUBMODE_POS       MODE_POS+2*FW
  #define USBJ_ONE_2ND_COLUMN (12*FW)
  #define BTNMODE_POS       SUBMODE_POS+5*FW
  #define BTNNUM_POS        BTNMODE_POS+12*FW
#else
  #define MODE_POS          5*FW
  #define SUBMODE_POS       MODE_POS+2*FW
  #define USBJ_ONE_2ND_COLUMN (12*FW)
  #define BTNMODE_POS       SUBMODE_POS+5*FW
  #define BTNNUM_POS        LCD_W
#endif

enum USBJFields {
  USBJ_FIELD_MODE,
  USBJ_FIELD_INVERSION,
  USBJ_FIELD_SUBMODE,
  USBJ_FIELD_BTNPOS,
  USBJ_FIELD_BTNNUM,
  USBJ_FIELD_MAX
};

void menuModelUSBJoystickOne(event_t event)
{
#if defined(NAVIGATION_XLITE)
  if (event == EVT_KEY_FIRST(KEY_ENTER) && keysGetState(KEY_SHIFT)) {
    pushMenu(menuChannelsView);
  }
#else
  if (event == EVT_KEY_BREAK(KEY_MODEL) || event == EVT_KEY_BREAK(KEY_MENU)) {
    pushMenu(menuChannelsView);
  }
#endif
  USBJoystickChData * cch = usbJChAddress(s_currIdx);
  putsChn(12*FW, 0, s_currIdx+1, 0);
  lcdDrawNumber(20*FW, 0, channelOutputs[s_currIdx], RIGHT);

  SUBMENU(STR_USBJOYSTICK_LABEL, USBJ_FIELD_MAX, {
            0,
            (cch->mode != USBJOYS_CH_NONE) ? (uint8_t)0 : (uint8_t)HIDDEN_ROW,
            (cch->mode != USBJOYS_CH_NONE) ? (uint8_t)0 : (uint8_t)HIDDEN_ROW,
            (cch->mode == USBJOYS_CH_BUTTON) ?
              (((cch->param == USBJOYS_BTN_MODE_SW_EMU) || (cch->param == USBJOYS_BTN_MODE_DELTA)) ? (uint8_t)READONLY_ROW : (uint8_t)0)
              : (uint8_t)HIDDEN_ROW,
            (cch->mode == USBJOYS_CH_BUTTON) ? (uint8_t)0 : (uint8_t)HIDDEN_ROW });

  int8_t sub = menuVerticalPosition;

  char str[20];

  for (uint8_t i=0; i<(NUM_BODY_LINES-1); i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i + menuVerticalOffset;
    for (int j=0; j<=k; ++j) {
      if (j<(int)DIM(mstate_tab) && mstate_tab[j] == HIDDEN_ROW) {
        ++k;
      }
    }
    LcdFlags attr = (sub==k ? (s_editMode>0 ? BLINK|INVERS : INVERS) : 0);

    switch (k) {
      case USBJ_FIELD_MODE:
        cch->mode = editChoice(USBJ_ONE_2ND_COLUMN, y, STR_USBJOYSTICK_CH_MODE, STR_VUSBJOYSTICK_CH_MODE, cch->mode, 0, USBJOYS_CH_LAST, attr, event);
        break;

      case USBJ_FIELD_INVERSION:
        lcdDrawTextAlignedLeft(y, TR_USBJOYSTICK_CH_INVERSION);
        drawCheckBox(USBJ_ONE_2ND_COLUMN, y, cch->inversion, attr);
        if (attr) cch->inversion = checkIncDecModel(event, cch->inversion, 0, 1);
        break;

      case USBJ_FIELD_SUBMODE:
        if (cch->mode == USBJOYS_CH_BUTTON) {
          if (cch->param > USBJOYS_BTN_MODE_LAST) cch->param = 0;
          cch->param = editChoice(USBJ_ONE_2ND_COLUMN, y, STR_USBJOYSTICK_CH_BTNMODE, STR_VUSBJOYSTICK_CH_BTNMODE, cch->param, 0, USBJOYS_BTN_MODE_LAST, attr, event);
          if (cch->param == USBJOYS_BTN_MODE_SW_EMU) cch->switch_npos = 0;
          else if (cch->param == USBJOYS_BTN_MODE_DELTA) cch->switch_npos = 1;
        }
        else if (cch->mode == USBJOYS_CH_AXIS) {
          cch->param = editChoice(USBJ_ONE_2ND_COLUMN, y, STR_USBJOYSTICK_CH_AXIS, STR_VUSBJOYSTICK_CH_AXIS, cch->param, 0, USBJOYS_AXIS_LAST, attr, event);
        }
        else if (cch->mode == USBJOYS_CH_SIM) {
          if (cch->param > USBJOYS_SIM_LAST) cch->param = 0;
          cch->param = editChoice(USBJ_ONE_2ND_COLUMN, y, STR_USBJOYSTICK_CH_SIM, STR_VUSBJOYSTICK_CH_SIM, cch->param, 0, USBJOYS_SIM_LAST, attr, event);
        }
        break;

      case USBJ_FIELD_BTNPOS:
        if (cch->param == USBJOYS_BTN_MODE_SW_EMU) {
          lcdDrawTextAlignedLeft(y, STR_USBJOYSTICK_CH_SWPOS);
          lcdDrawText(USBJ_ONE_2ND_COLUMN, y, STR_VUSBJOYSTICK_CH_SWPOS[0], attr);
        }
        else if (cch->param == USBJOYS_BTN_MODE_DELTA) {
          lcdDrawTextAlignedLeft(y, STR_USBJOYSTICK_CH_SWPOS);
          lcdDrawText(USBJ_ONE_2ND_COLUMN, y, STR_VUSBJOYSTICK_CH_SWPOS[1], attr);
        }
        else {
          cch->switch_npos = editChoice(USBJ_ONE_2ND_COLUMN, y, STR_USBJOYSTICK_CH_SWPOS, STR_VUSBJOYSTICK_CH_SWPOS, cch->switch_npos, 0, 7, attr, event);
        }
        break;

      case USBJ_FIELD_BTNNUM:
        lcdDrawTextAlignedLeft(y, TR_USBJOYSTICK_CH_BTNNUM);
        uint8_t last = cch->lastBtnNumNoCLip();
        uint8_t limited = 0;
        if(last >= USBJ_BUTTON_SIZE) {
          limited = 1;
          last = USBJ_BUTTON_SIZE-1;
        }
        if (last > cch->btn_num)
          snprintf(str, 20, "%u..%u%c", cch->btn_num, last, limited ? '!' : ' ');
        else
          snprintf(str, 20, "%u%c", cch->btn_num, limited ? '!' : ' ');
        lcdDrawText(USBJ_ONE_2ND_COLUMN, y, str, attr);
        if (attr) {
          CHECK_INCDEC_MODELVAR(event, cch->btn_num, 0, USBJ_BUTTON_SIZE);
        }
        break;
    }
  }

  if (cch->mode == USBJOYS_CH_BUTTON) {
    if (isUSBBtnNumCollision(s_currIdx)) {
      lcdDrawText(LCD_W/2, LCD_H-FH, STR_USBJOYSTICK_BTN_COLLISION, INVERS | CENTERED);
    }
  }
  else if (cch->mode == USBJOYS_CH_AXIS) {
    if (isUSBAxisCollision(s_currIdx)) {
      lcdDrawText(LCD_W/2, LCD_H-FH, STR_USBJOYSTICK_AXIS_COLLISION, INVERS | CENTERED);
    }
  }
  else if (cch->mode == USBJOYS_CH_SIM) {
    if (isUSBSimCollision(s_currIdx)) {
      lcdDrawText(LCD_W/2, LCD_H-FH, STR_USBJOYSTICK_AXIS_COLLISION, INVERS | CENTERED);
    }
  }
}

void onUSBJoystickMenu(const char *result)
{
  int8_t sub = menuVerticalPosition;
  USBJoystickChData * cs = usbJChAddress(sub);

  if (result == STR_EDIT) {
    s_currIdx = sub;
    pushMenu(menuModelUSBJoystickOne);
  }
  else if (result == STR_CLEAR) {
    memset(cs, 0, sizeof(USBJoystickChData));
    storageDirty(EE_MODEL);
  }
}

void menuModelUSBJoystick(event_t event)
{
  check_submenu_simple(event, USBJ_MAX_JOYSTICK_CHANNELS);
  title(STR_USBJOYSTICK_LABEL);

  if (s_editMode > 0) s_editMode = 0;

  int8_t sub = menuVerticalPosition;

  coord_t y = 0;
  uint8_t k = 0;

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    USBJoystickChData * cch = usbJChAddress(sub);
    s_currIdx = sub;
    if (sub >= 0)
      POPUP_MENU_ADD_ITEM(STR_EDIT);
    if (cch->mode != USBJOYS_CH_NONE)
      POPUP_MENU_ADD_ITEM(STR_CLEAR);
    if (popupMenuItemsCount == 1) {
      popupMenuItemsCount = 0;
      s_currIdx = sub;
      pushMenu(menuModelUSBJoystickOne);
    }
    else {
      POPUP_MENU_START(onUSBJoystickMenu);
    }
  }

  char str[20];

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    y = MENU_HEADER_HEIGHT + 1 + i*FH;
    k = i + menuVerticalOffset;
    LcdFlags attr = (sub==k) ? INVERS : 0;

    USBJoystickChData * cch = usbJChAddress(k);

    putsChn(0, y, k+1, attr);
    lcdDrawChar(4*FW, y, cch->inversion ? '{' : ' ', 0);

    lcdDrawTextAtIndex(MODE_POS, y, STR_VUSBJOYSTICK_CH_MODE_S, cch->mode, 0);
    if (cch->mode == USBJOYS_CH_BUTTON) {
      lcdDrawTextAtIndex(SUBMODE_POS, y, STR_VUSBJOYSTICK_CH_SWPOS, cch->switch_npos, 0);
      lcdDrawTextAtIndex(BTNMODE_POS, y, STR_VUSBJOYSTICK_CH_BTNMODE_S, cch->param, 0);

      uint8_t last = cch->lastBtnNum();
      if (last > cch->btn_num)
        sprintf(str, "%u..%u", cch->btn_num, last);
      else
        sprintf(str, "%u", cch->btn_num);
      LcdFlags warn = 0;
      if (isUSBBtnNumCollision(k)) warn = INVERS;
      lcdDrawText(BTNNUM_POS, y, str, RIGHT|warn);
    }
    else if (cch->mode == USBJOYS_CH_AXIS) {
      LcdFlags warn = 0;
      if (isUSBAxisCollision(k)) warn = INVERS;
      lcdDrawTextAtIndex(SUBMODE_POS, y, STR_VUSBJOYSTICK_CH_AXIS, cch->param, warn);
    }
    else if (cch->mode == USBJOYS_CH_SIM) {
      LcdFlags warn = 0;
      if (isUSBSimCollision(k)) warn = INVERS;
      lcdDrawTextAtIndex(SUBMODE_POS, y, STR_VUSBJOYSTICK_CH_SIM, cch->param, warn);
    }
  }

}
