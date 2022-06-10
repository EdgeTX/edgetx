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

#include "standalone_lua.h"
#include "view_main.h"
// #include "touch.h"

using std::string;
extern BitmapBuffer* luaLcdBuffer;

void LuaPopup::paint(BitmapBuffer* dc, uint8_t type, const char* text, const char* info)
{
  // popup background
  dc->drawSolidFilledRect(0, 0, rect.w, POPUP_HEADER_HEIGHT,
                          COLOR_THEME_FOCUS);

  const char* title = text;//TODO: based on 'type'

  // title bar
  dc->drawText(FIELD_PADDING_LEFT,
               (POPUP_HEADER_HEIGHT - getFontHeight(FONT(STD))) / 2,
               title, COLOR_THEME_PRIMARY2);

  dc->drawSolidFilledRect(0, POPUP_HEADER_HEIGHT, rect.w,
                          rect.h - POPUP_HEADER_HEIGHT,
                          COLOR_THEME_SECONDARY3);

  dc->drawText(FIELD_PADDING_LEFT,
               POPUP_HEADER_HEIGHT + PAGE_LINE_HEIGHT,
               info, COLOR_THEME_SECONDARY1);
}

// singleton instance
StandaloneLuaWindow* StandaloneLuaWindow::_instance;

StandaloneLuaWindow::StandaloneLuaWindow() :
    Window(nullptr, {0, 0, LCD_W, LCD_H}, OPAQUE),
    lcdBuffer(BMP_RGB565, LCD_W, LCD_H),
    popup({50, 73, LCD_W - 100, LCD_H - 146})
{
  // setup LUA event handler
  setupHandler(this);
}

StandaloneLuaWindow* StandaloneLuaWindow::instance()
{
  if (!_instance) {
    _instance = new StandaloneLuaWindow();
  }

  return _instance;
}

void StandaloneLuaWindow::attach()
{
  if (!prevScreen) {

    // backup previous screen
    prevScreen = lv_scr_act();

    // and load new one
    lv_scr_load(lvobj);

    Layer::push(this);

    lv_group_add_obj(lv_group_get_default(), lvobj);
    lv_group_set_editing(lv_group_get_default(), true);
  }
}

void StandaloneLuaWindow::deleteLater(bool detach, bool trash)
{
  if (_deleted)
    return;

  Layer::pop(this);

  if (prevScreen) {
    lv_scr_load(prevScreen);
    prevScreen = nullptr;
  }

  if (trash) {
    _instance = nullptr;
  }
  
  Window::deleteLater(detach, trash);
}

void StandaloneLuaWindow::paint(BitmapBuffer* dc)
{
  dc->drawSolidFilledRect(0, 0, width(), height(), COLOR_THEME_SECONDARY3);
  dc->drawBitmap(0 - dc->getOffsetX(), 0 - dc->getOffsetY(), &lcdBuffer);
}

void StandaloneLuaWindow::checkEvents()
{
  Window::checkEvents();

  // Set global LUA LCD buffer
  luaLcdBuffer = &lcdBuffer;

  if (luaState != INTERPRETER_RELOAD_PERMANENT_SCRIPTS) {
    // if LUA finished a full cycle,
    // invalidate to display the screen buffer
    if (luaTask(0, true)) { invalidate(); }
  }

  if (luaState == INTERPRETER_RELOAD_PERMANENT_SCRIPTS) {
    // Script does not run anymore...
    TRACE("LUA standalone script exited: deleting window!");
    deleteLater();
  }
  
  // Kill global LUA LCD buffer
  luaLcdBuffer = nullptr;
}

void StandaloneLuaWindow::onClicked()
{
  LuaEventHandler::onClicked();
}

void StandaloneLuaWindow::onCancel()
{
  LuaEventHandler::onCancel();
}

void StandaloneLuaWindow::onEvent(event_t evt)
{
  LuaEventHandler::onEvent(evt);
}

bool StandaloneLuaWindow::displayPopup(event_t event, uint8_t type, const char* text,
                                       const char* info, bool& result)
{
  // transparent background
  lcdBuffer.drawFilledRect(0, 0, LCD_W, LCD_H, SOLID,
                           COLOR_THEME_PRIMARY1, OPACITY(5));

  // center pop-up
  lcdBuffer.setOffset(LCD_W/2 - popup.rect.w/2, LCD_H/2 - popup.rect.h/2);

  // draw it, then clear the offset
  popup.paint(&lcdBuffer, type, text, info);
  lcdBuffer.clearOffset();

  TRACE("displayPopup(event = 0x%x)", event);
  if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    result = false;
    return true;
  } else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    result = true;
    return true;
  }

  return false;
}
