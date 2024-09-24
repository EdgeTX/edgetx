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

#include "translations.h"
#include "view_main.h"
#include "dma2d.h"
#include "lua/lua_event.h"

lua_State *lsStandalone = nullptr;

#if defined(LUA_ALLOCATOR_TRACER)
LuaMemTracer lsStandaloneTrace;

static void luaStandaloneHook(lua_State * L, lua_Debug *ar)
{
  else if (ar->event == LUA_HOOKLINE) {
    lua_getinfo(L, "nSl", ar);
    LuaMemTracer * tracer = &lsStandaloneTrace;
    if (tracer->alloc || tracer->free) {
      TRACE("LT: [+%u,-%u] %s:%d", tracer->alloc, tracer->free, tracer->script, tracer->lineno);
    }
    tracer->script = ar->source;
    tracer->lineno = ar->currentline;
    tracer->alloc = 0;
    tracer->free = 0;
  }
}
#endif // #if defined(LUA_ALLOCATOR_TRACER)

static void luaStandaloneInit()
{
  luaClose(&lsStandalone);

#if defined(USE_BIN_ALLOCATOR)
  lsStandalone = lua_newstate(bin_l_alloc, nullptr);   //we use our own allocator!
#elif defined(LUA_ALLOCATOR_TRACER)
  memclear(&lsStandaloneTrace, sizeof(lsStandaloneTrace));
  lsStandaloneTrace.script = "lua_newstate(scripts)";
  lsStandalone = lua_newstate(tracer_alloc, &lsStandaloneTrace);   //we use tracer allocator
#else
  lsStandalone = lua_newstate(l_alloc, nullptr);   //we use Lua default allocator
#endif
  if (lsStandalone) {
    // install our panic handler
    extern int custom_lua_atpanic(lua_State *L);
    lua_atpanic(lsStandalone, &custom_lua_atpanic);

#if defined(LUA_ALLOCATOR_TRACER)
    lua_sethook(L, luaStandaloneHook, LUA_MASKLINE);
#endif

    // protect libs and constants registration
    PROTECT_LUA() {
      luaRegisterLibraries(lsStandalone);
    }
    else {
      // if we got panic during registration
      // we disable Lua for this session
      luaClose(&lsStandalone);
      lsStandalone = 0;
    }
    UNPROTECT_LUA();
  }
}

void luaExecStandalone(const char * filename)
{
  if (lsStandalone == NULL)
    luaStandaloneInit();

  PROTECT_LUA() {
    if (luaLoadScriptFileToState(lsStandalone, filename, LUA_SCRIPT_LOAD_MODE) == SCRIPT_OK) {
      if (lua_pcall(lsStandalone, 0, 1, 0) == LUA_OK && lua_istable(lsStandalone, -1)) {
        int initFunction = LUA_REFNIL, runFunction = LUA_REFNIL;
        bool lvglLayout = false;

        for (lua_pushnil(lsStandalone); lua_next(lsStandalone, -2); lua_pop(lsStandalone, 1)) {
          const char * key = lua_tostring(lsStandalone, -2);
          if (!strcmp(key, "init")) {
            initFunction = luaL_ref(lsStandalone, LUA_REGISTRYINDEX);
            lua_pushnil(lsStandalone);
          } else if (!strcmp(key, "run")) {
            runFunction = luaL_ref(lsStandalone, LUA_REGISTRYINDEX);
            lua_pushnil(lsStandalone);
          } else if (!strcasecmp(key, "useLvgl")) {
            lvglLayout = lua_toboolean(lsStandalone, -1);
          }
        }

        StandaloneLuaWindow::setup(lvglLayout, initFunction, runFunction);
      }
      else {
        TRACE("luaLoadFile(%s): Error parsing script: %s", filename, lua_tostring(lsStandalone, -1));
      }
    }
  }
  else {
    // error while loading Lua widget/theme,
    // do not disable whole Lua state, just ingnore bad script
    return;
  }
  UNPROTECT_LUA();
}

// singleton instance
StandaloneLuaWindow* StandaloneLuaWindow::_instance;

StandaloneLuaWindow::StandaloneLuaWindow(bool useLvgl, int initFn, int runFn) :
    Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}),
    useLvgl(useLvgl), initFunction(initFn), runFunction(runFn)
{
  setWindowFlag(OPAQUE);

  etx_solid_bg(lvobj);

  if (useLvglLayout()) {
    padAll(PAD_ZERO);
    etx_scrollbar(lvobj);

    lv_obj_t* lbl = lv_label_create(lvobj);
    lv_obj_set_pos(lbl, 0, 0);
    lv_obj_set_size(lbl, LCD_W, LCD_H);
    etx_solid_bg(lbl, COLOR_THEME_PRIMARY1_INDEX);
    etx_txt_color(lbl, COLOR_THEME_PRIMARY2_INDEX);
    etx_font(lbl, FONT_XL_INDEX);
    lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_pad_top(lbl, (LCD_H - EdgeTxStyles::PAGE_LINE_HEIGHT) / 2, LV_PART_MAIN);
    lv_label_set_text(lbl, STR_LOADING);

    luaLvglManager = this;
  } else {
    lcdBuffer = new BitmapBuffer(BMP_RGB565, LCD_W, LCD_H);

    lcdBuffer->clear();
    lcdBuffer->drawText(LCD_W / 2, LCD_H / 2 - 20, STR_LOADING,
                      FONT(L) | COLOR_THEME_PRIMARY2 | CENTERED);
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    auto canvas = lv_canvas_create(lvobj);
    lv_obj_center(canvas);
    lv_canvas_set_buffer(canvas, lcdBuffer->getData(),
                         lcdBuffer->width(), lcdBuffer->height(), LV_IMG_CF_TRUE_COLOR);
  }

  // setup LUA event handler
  setupHandler(this);

  attach();

  lua_gc(lsStandalone, LUA_GCCOLLECT, 0);

  // Pause function and mixer scripts
  prevLuaState = luaState;
  luaState = INTERPRETER_PAUSED;

#if defined(USE_HATS_AS_KEYS)
  setTransposeHatsForLUA(true);
#endif
}

void StandaloneLuaWindow::setup(bool useLvgl, int initFn, int runFn)
{
  if (_instance == nullptr)
    _instance = new StandaloneLuaWindow(useLvgl, initFn, runFn);
}

StandaloneLuaWindow* StandaloneLuaWindow::instance()
{
  return _instance;
}

void StandaloneLuaWindow::attach()
{
  if (!prevScreen) {
    // backup previous screen
    prevScreen = lv_scr_act();

    Layer::back()->hide();
    Layer::push(this);

    if (!useLvglLayout()) {
      lv_group_add_obj(lv_group_get_default(), lvobj);
      lv_group_set_editing(lv_group_get_default(), true);
    }
  }
}

void StandaloneLuaWindow::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;

  if (initFunction != LUA_REFNIL) luaL_unref(lsStandalone, LUA_REGISTRYINDEX, initFunction);
  if (runFunction != LUA_REFNIL) luaL_unref(lsStandalone, LUA_REGISTRYINDEX, runFunction);
  lua_settop(lsStandalone, 0);
  luaLcdBuffer = nullptr;

  if (lcdBuffer) delete lcdBuffer;
  lcdBuffer = nullptr;

  luaLvglManager = nullptr;

  Layer::pop(this);
  Layer::back()->show();

  if (prevScreen) {
    prevScreen = nullptr;
  }

  if (trash) {
    _instance = nullptr;
  }

#if defined(USE_HATS_AS_KEYS)
  setTransposeHatsForLUA(false);
#endif

  luaState = prevLuaState;

  Window::deleteLater(detach, trash);
}

void StandaloneLuaWindow::checkEvents()
{
  Window::checkEvents();

  if (initFunction != LUA_REFNIL) {
    lua_rawgeti(lsStandalone, LUA_REGISTRYINDEX, initFunction);
    if (lua_pcall(lsStandalone, 0, 0, 0) != LUA_OK) {
      luaShowError();
    }
    luaL_unref(lsStandalone, LUA_REGISTRYINDEX, initFunction);
    initFunction = LUA_REFNIL;
    return;
  }

  // Set global LUA LCD buffer
  luaLcdBuffer = lcdBuffer;
  luaLcdAllowed = !useLvglLayout();

  LuaEventData evt;
  // Pull a new event from the buffer
  luaNextEvent(&evt);
  if (evt.event == EVT_KEY_LONG(KEY_EXIT)) {
    killEvents(evt.event);
    deleteLater();
  } else {
    if (runFunction != LUA_REFNIL) {
      lua_rawgeti(lsStandalone, LUA_REGISTRYINDEX, runFunction);

      lua_pushunsigned(lsStandalone, evt.event);
      int inputsCount = 1;

#if defined(HARDWARE_TOUCH)
      if (IS_TOUCH_EVENT(evt.event)) {
        luaPushTouchEventTable(lsStandalone, &evt);
        inputsCount += 1;
      }
#endif

      if (lua_pcall(lsStandalone, inputsCount, 1, 0) != LUA_OK) {
        luaShowError();
      } else {
        if (lua_isnumber(lsStandalone, -1)) {
          int scriptResult = lua_tointeger(lsStandalone, -1);
          lua_pop(lsStandalone, 1);  /* pop returned value */
          if (scriptResult != 0) {
            deleteLater();
          } else {
            if (useLvglLayout() && !hasError) {
              PROTECT_LUA() {
                if (!callRefs(lsStandalone)) {
                  luaShowError();
                }
              } else {
                luaShowError();
              }
              UNPROTECT_LUA();
            } else {
              invalidate();
            }
          }
        } else if (lua_isstring(lsStandalone, -1)) {
          char nextScript[FF_MAX_LFN+1];
          strncpy(nextScript, lua_tostring(lsStandalone, -1), FF_MAX_LFN);
          nextScript[FF_MAX_LFN] = '\0';
          _instance = nullptr;
          lua_settop(lsStandalone, 0);
          deleteLater();
          luaExecStandalone(nextScript);
        }
      }
    }
  }

  // Kill global LUA LCD buffer
  luaLcdBuffer = nullptr;
  luaLcdAllowed = false;
}

void StandaloneLuaWindow::onClicked() { Keyboard::hide(false); LuaEventHandler::onClicked(); }

void StandaloneLuaWindow::onCancel() { LuaEventHandler::onCancel(); }

void StandaloneLuaWindow::onEvent(event_t evt)
{
  LuaEventHandler::onEvent(evt);
}

void StandaloneLuaWindow::popupPaint(BitmapBuffer* dc, coord_t x, coord_t y, coord_t w, coord_t h,
                                     const char* text, const char* info)
{
  // popup background
  dc->drawSolidFilledRect(x, y, w, POPUP_HEADER_HEIGHT, COLOR_THEME_FOCUS);

  // title bar
  dc->drawText(x + PAD_SMALL,
               y + (POPUP_HEADER_HEIGHT - getFontHeight(FONT(STD))) / 2, text,
               COLOR_THEME_PRIMARY2);

  dc->drawSolidFilledRect(x, y + POPUP_HEADER_HEIGHT, w,
                          h - POPUP_HEADER_HEIGHT, COLOR_THEME_SECONDARY3);

  dc->drawText(x + PAD_SMALL, y + POPUP_HEADER_HEIGHT + EdgeTxStyles::PAGE_LINE_HEIGHT, info,
               COLOR_THEME_SECONDARY1);
}

bool StandaloneLuaWindow::displayPopup(event_t event, uint8_t type,
                                       const char* text, const char* info,
                                       bool& result)
{
  if (useLvgl) return true;

  // transparent background
  lcdBuffer->drawFilledRect(0, 0, LCD_W, LCD_H, SOLID, COLOR_THEME_PRIMARY1,
                            OPACITY(5));

  popupPaint(lcdBuffer, POPUP_X, POPUP_Y, LCD_W - POPUP_X * 2, LCD_H - POPUP_Y * 2, text, info);

  // TRACE("displayPopup(event = 0x%x)", event);
  if (event == EVT_KEY_BREAK(KEY_EXIT)) {
    result = false;
    return true;
  } else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    result = true;
    return true;
  }

  return false;
}

void StandaloneLuaWindow::clear()
{
  clearRefs(lsStandalone);
  Window::clear();
}

void StandaloneLuaWindow::luaShowError()
{
  if (runFunction != LUA_REFNIL) luaL_unref(lsStandalone, LUA_REGISTRYINDEX, runFunction);
  runFunction = LUA_REFNIL;
  hasError = true;
  extern void luaError(lua_State *L, uint8_t error);
  luaError(lsStandalone, SCRIPT_SYNTAX_ERROR);
}

void StandaloneLuaWindow::showError(bool firstCall, const char* title, const char* msg)
{
  runFunction = LUA_REFNIL;
  hasError = true;
  if (!errorModal) {
    lv_obj_set_scroll_dir(lvobj, LV_DIR_NONE);
    errorModal = lv_obj_create(lvobj);
    lv_obj_set_pos(errorModal, lv_obj_get_scroll_x(lvobj), lv_obj_get_scroll_y(lvobj));
    lv_obj_set_size(errorModal, LCD_W, LCD_H);
    etx_bg_color(errorModal, COLOR_BLACK_INDEX);
    etx_obj_add_style(errorModal, styles->bg_opacity_75, LV_PART_MAIN);
    errorTitle = lv_label_create(errorModal);
    lv_obj_set_pos(errorTitle, 50, 30);
    lv_obj_set_size(errorTitle, LCD_W - 100, 32);
    etx_txt_color(errorTitle, COLOR_THEME_PRIMARY2_INDEX);
    etx_solid_bg(errorTitle, COLOR_THEME_SECONDARY1_INDEX);
    etx_font(errorTitle, FONT_L_INDEX);
    etx_obj_add_style(errorTitle, styles->text_align_center, LV_PART_MAIN);
    errorMsg = lv_label_create(errorModal);
    lv_obj_set_pos(errorMsg, 50, 62);
    lv_obj_set_size(errorMsg, LCD_W - 100, LCD_H - 92);
    lv_obj_set_style_pad_all(errorMsg, 4, LV_PART_MAIN);
    etx_txt_color(errorMsg, COLOR_THEME_PRIMARY1_INDEX);
    etx_solid_bg(errorMsg, COLOR_THEME_SECONDARY3_INDEX);
    etx_font(errorMsg, FONT_STD_INDEX);
    etx_obj_add_style(errorMsg, styles->text_align_center, LV_PART_MAIN);
  }

  lv_label_set_text(errorTitle, title);
  lv_label_set_text(errorMsg, msg);
}
