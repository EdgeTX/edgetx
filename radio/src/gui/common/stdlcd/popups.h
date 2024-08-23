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

#pragma once

#include <inttypes.h>
#include "audio.h"
#include "lcd.h"
#include "keys.h"

extern const char * warningText;
extern const char * warningInfoText;
extern uint8_t      warningInfoLength;
extern uint8_t      warningResult;
extern uint8_t      warningType;
extern uint8_t      warningInfoFlags;

typedef void         (* PopupFunc)(event_t event);
extern PopupFunc popupFunc;
extern uint8_t      popupMenuOffsetType;

extern uint16_t     popupMenuOffset;
extern const char * popupMenuItems[POPUP_MENU_MAX_LINES];
extern uint16_t     popupMenuItemsCount;
typedef void         (* PopupMenuHandler)(const char * result);
extern PopupMenuHandler popupMenuHandler;
extern const char * popupMenuTitle;
extern uint8_t      popupMenuSelectedItem;

// Message box
void drawMessageBoxBackground(coord_t top, coord_t height);
void drawMessageBox(const char * title);
void showMessageBox(const char * title);

// Popup menu
const char * runPopupMenu(event_t event);
void runPopupWarning(event_t event);

// Full screen with an alert
void showAlertBox(const char * title, const char * text, const char * action , uint8_t sound);

// Full screen with 2 lines and a progress bar
void drawProgressScreen(const char * title, const char * message, int num, int den);
typedef void (* ProgressHandler)(const char *, const char *, int, int);

enum WarningType
{
  WARNING_TYPE_WAIT,
  WARNING_TYPE_INFO,
  WARNING_TYPE_ASTERISK,
  WARNING_TYPE_CONFIRM,
  WARNING_TYPE_INPUT,
  WARNING_TYPE_ALERT
};

#if !defined(GUI)
  #define DISPLAY_WARNING(...)
  inline void POPUP_WAIT(const char * s) { }
  inline void POPUP_WARNING(const char *, const char * = nullptr, bool waitForClose) { }
  inline void POPUP_CONFIRMATION(const char * s, PopupMenuHandler handler) { }
  inline void POPUP_INPUT(const char * s, PopupFunc func) { }
  inline void SET_WARNING_INFO(const char * info, uint8_t length, uint8_t flags) { }
#else
  #define DISPLAY_WARNING(evt)              (*popupFunc)(evt)
  extern void CLEAR_POPUP();
  extern void POPUP_WAIT(const char * s);
  extern void POPUP_INFORMATION(const char * s);
  extern void POPUP_WARNING(const char * message, const char * info = nullptr, bool waitForClose = true);
  extern void SET_WARNING_INFO(const char * info, uint8_t length, uint8_t flags);
  extern void POPUP_CONFIRMATION(const char * s, PopupMenuHandler handler);
  extern void POPUP_INPUT(const char * s, PopupFunc func);
  extern bool isEventCaughtByPopup();
#endif

extern void POPUP_MENU_ADD_ITEM(const char * s);

extern void POPUP_MENU_SELECT_ITEM(uint8_t index);

extern void POPUP_MENU_TITLE(const char * s);

extern void POPUP_MENU_START(PopupMenuHandler handler);
extern void POPUP_MENU_START(PopupMenuHandler handler, int count, ...);

// For compatability with color LCD code base, not (currently) required for B&W
#define POPUP_WARNING_ON_UI_TASK POPUP_WARNING
