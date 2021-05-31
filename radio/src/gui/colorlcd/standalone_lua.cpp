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

#include "standalone_lua.h"
#include "view_main.h"
#include "lua/lua_api.h"

using std::string;

constexpr uint32_t STANDALONE_LUA_REFRESH = 1000 / 50; // 5Hz

void LuaPopup::paint(BitmapBuffer* dc, uint8_t type, const char* text, const char* info)
{
  // popup background
  dc->drawSolidFilledRect(0, 0, rect.w, POPUP_HEADER_HEIGHT,
                          FOCUS_BGCOLOR);

  const char* title = text;//TODO: based on 'type'

  // title bar
  dc->drawText(FIELD_PADDING_LEFT,
               (POPUP_HEADER_HEIGHT - getFontHeight(FONT(STD))) / 2,
               title, FOCUS_COLOR);

  dc->drawSolidFilledRect(0, POPUP_HEADER_HEIGHT, rect.w,
                          rect.h - POPUP_HEADER_HEIGHT,
                          DEFAULT_BGCOLOR);

  dc->drawText(FIELD_PADDING_LEFT,
               POPUP_HEADER_HEIGHT + PAGE_LINE_HEIGHT,
               info, DEFAULT_COLOR);
}

// singleton instance
StandaloneLuaWindow* StandaloneLuaWindow::_instance;

// LUA lcd buffer
uint16_t* lcdGetBackupBuffer();

StandaloneLuaWindow::StandaloneLuaWindow() :
    Window(nullptr, {0, 0, LCD_W, LCD_H}, OPAQUE),
    lcdBuffer(BMP_RGB565, LCD_W, LCD_H, lcdGetBackupBuffer()),
    popup({50, 73, LCD_W - 100, LCD_H - 146})
{
}

StandaloneLuaWindow* StandaloneLuaWindow::instance()
{
  if (!_instance) {
    _instance = new StandaloneLuaWindow();
  }

  return _instance;
}

void StandaloneLuaWindow::attach(Window* newParent)
{
  Window::attach(newParent->getFullScreenWindow());
  Layer::push(this);
  setFocus();
}

void StandaloneLuaWindow::deleteLater(bool detach, bool /*trash*/)
{
  Layer::pop(this);

  // do not trash: we are a singleton
  if (static_cast<Window*>(focusWindow) == static_cast<Window*>(this)) {
    focusWindow = nullptr;
  }

  // detach from parent
  if (detach)
    this->detach();

  if (closeHandler) {
    closeHandler();
  }
}

void StandaloneLuaWindow::paint(BitmapBuffer* dc)
{
  dc->drawSolidFilledRect(0, 0, width(), height(), DEFAULT_BGCOLOR);
  dc->drawBitmap(0 - dc->getOffsetX(), 0 - dc->getOffsetY(), &lcdBuffer);
}

void StandaloneLuaWindow::checkEvents()
{
  // Execute first in case onEvent() is called.
  // (would trigger refresh)
  Window::checkEvents();
    
  uint32_t now = RTOS_GET_MS();
  if (now - lastRefresh >= STANDALONE_LUA_REFRESH) {
    lastRefresh = now;
    runLua(0);
  }
}

#if defined(HARDWARE_KEYS)
void StandaloneLuaWindow::onEvent(event_t evt)
{
  lastRefresh = RTOS_GET_MS(); // mark as 'refreshed'
  runLua(evt);
}
#endif

bool StandaloneLuaWindow::displayPopup(event_t event, uint8_t type, const char* text,
                                       const char* info, bool& result)
{
  // transparent background
  lcdBuffer.drawFilledRect(0, 0, LCD_W, LCD_H, SOLID,
                           OVERLAY_COLOR, OPACITY(5));

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

extern BitmapBuffer* luaLcdBuffer;

void StandaloneLuaWindow::runLua(event_t evt)
{
  // Set global LUA LCD buffer
  luaLcdBuffer = &lcdBuffer;

  TRACE("evt = 0x%x", evt);
  
  bool hasRun = luaTask(evt, RUN_STNDAL_SCRIPT, true);
  if (hasRun) {
#if defined(DEBUG_WINDOWS)
    TRACE("# StandaloneLuaWindow::invalidate()");
#endif
    invalidate();
  } else {
    // Script does not run anymore...
    TRACE("LUA standalone script exited: deleting window!");
    deleteLater();
  }

  // Kill global LUA LCD buffer
  luaLcdBuffer = nullptr;
}
