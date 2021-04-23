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

constexpr uint32_t STANDALONE_LUA_REFRESH = 1000 / 5; // 5Hz

uint16_t* lcdGetBackupBuffer();

// #if !defined(SIMU)

// // LUA script draw into the backup buffer
// extern uint8_t * LCD_BACKUP_FRAME_BUFFER;
// #define LUA_LCD_BUFFER ((uint16_t *)LCD_BACKUP_FRAME_BUFFER)

// #else

// extern pixel_t* simuLcdBuf;
// #define LUA_LCD_BUFFER ((uint16_t *)simuLcdBuf)

// #endif

StandaloneLuaWindow::StandaloneLuaWindow(Window* parent) :
    Window(parent->getFullScreenWindow(), {0, 0, LCD_W, LCD_H}, OPAQUE),
    lcdBuffer(BMP_RGB565, LCD_W, LCD_H, lcdGetBackupBuffer())
{
  TRACE("backup frame buffer = %p", lcdGetBackupBuffer());
  TRACE("lcdBuffer.getData() = %p", lcdBuffer.getData());
  assert(lcdBuffer.getData());
  Layer::push(this);
  setFocus();
}

void StandaloneLuaWindow::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);
  Window::deleteLater(detach, trash);
}

void StandaloneLuaWindow::paint(BitmapBuffer* dc)
{
  dc->drawSolidFilledRect(0, 0, width(), height(), DEFAULT_BGCOLOR);

  // TODO:
  // -> grab coords from 'dc'
  // -> copy from backup buffer into 'dc'
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
