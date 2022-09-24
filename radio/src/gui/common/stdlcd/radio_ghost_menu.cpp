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

#include "opentx.h"

#if defined(GHOST)
  #include "telemetry/ghost.h"
#endif

void menuGhostModuleConfig(event_t event)
{
  switch (event) {
    case EVT_ENTRY:
      memclear(&reusableBuffer.ghostMenu, sizeof(reusableBuffer.ghostMenu));
      strAppend((char *) &reusableBuffer.ghostMenu.line[1].menuText, STR_WAITING_FOR_MODULE, 0);
      reusableBuffer.ghostMenu.line[1].lineFlags = GHST_LINE_FLAGS_VALUE_EDIT;
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_NONE;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_OPEN;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      break;

#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_LEFT:
#elif defined(KEYS_GPIO_REG_UP)
      case EVT_KEY_BREAK(KEY_UP):
#elif defined(KEYS_GPIO_REG_PLUS)
      case EVT_KEY_BREAK(KEY_PLUS):
#endif
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYUP;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      audioKeyPress();
      break;

#if defined(ROTARY_ENCODER_NAVIGATION)
    case EVT_ROTARY_RIGHT:
#elif defined(KEYS_GPIO_REG_DOWN)
      case EVT_KEY_BREAK(KEY_DOWN):
#elif defined(KEYS_GPIO_REG_MINUS)
    case EVT_KEY_BREAK(KEY_MINUS):
#endif
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYDOWN;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      audioKeyPress();
      break;


    case EVT_KEY_FIRST(KEY_ENTER):
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYPRESS;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      audioKeyPress();
      break;

    case EVT_KEY_BREAK(KEY_EXIT):
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_JOYLEFT;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_NONE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      audioKeyPress();
      break;

    case EVT_KEY_LONG(KEY_EXIT):
      menuVerticalOffset = 0;
      memclear(&reusableBuffer.ghostMenu, sizeof(reusableBuffer.ghostMenu));
      reusableBuffer.ghostMenu.buttonAction = GHST_BTN_NONE;
      reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_CLOSE;
      moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
      RTOS_WAIT_MS(10);
      popMenu();
      break;
  }

  if (reusableBuffer.ghostMenu.menuStatus == GHST_MENU_STATUS_UNOPENED) { // Handles situation where module is plugged after tools start
    reusableBuffer.ghostMenu.buttonAction = GHST_BTN_NONE;
    reusableBuffer.ghostMenu.menuAction = GHST_MENU_CTRL_OPEN;
    moduleState[EXTERNAL_MODULE].counter = GHST_MENU_CONTROL;
  }
  else if (reusableBuffer.ghostMenu.menuStatus == GHST_MENU_STATUS_CLOSING) {
    popMenu();
  }

  constexpr coord_t xOffset = LCD_W / 3 - 15;
  constexpr coord_t xOffset2 = LCD_W / 3 + LCD_W / 4;
  constexpr coord_t yOffset = 6;
  LcdFlags flags = 0;

  for (uint8_t line = 0; line < GHST_MENU_LINES; line++) {
    flags = 0;
    if (reusableBuffer.ghostMenu.line[line].splitLine) {
      if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_LABEL_SELECT)
        flags = INVERS;
      lcdDrawText(xOffset, yOffset + line * FH, reusableBuffer.ghostMenu.line[line].menuText, flags);

      flags = 0;
      if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_VALUE_SELECT)
        flags |= INVERS;
      if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_VALUE_EDIT)
        flags |= BLINK;
      lcdDrawText(xOffset2, yOffset + line * FH, &reusableBuffer.ghostMenu.line[line].menuText[reusableBuffer.ghostMenu.line[line].splitLine], flags);
    }
    else {
      if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_LABEL_SELECT)
        flags = INVERS;
      if (reusableBuffer.ghostMenu.line[line].lineFlags & GHST_LINE_FLAGS_VALUE_EDIT)
        flags |= BLINK;
      lcdDrawText(xOffset, yOffset + line * FH, reusableBuffer.ghostMenu.line[line].menuText, flags);
    }
  }
}
