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

void StandaloneLuaWindow::redraw_cb(lv_event_t* e)
{
  lv_obj_t* target = lv_event_get_target(e);
  if (lv_obj_has_flag(target, LV_OBJ_FLAG_HIDDEN)) return;

  StandaloneLuaWindow* widget = (StandaloneLuaWindow*)lv_obj_get_user_data(target);

  if (widget) {
    lv_draw_ctx_t* draw_ctx = lv_event_get_draw_ctx(e);

    auto w = draw_ctx->buf_area->x2 - draw_ctx->buf_area->x1 + 1;
    auto h = draw_ctx->buf_area->y2 - draw_ctx->buf_area->y1 + 1;

    TRACE_WINDOWS("Draw %s", widget->getWindowDebugString().c_str());

    // Standalone script always redraws full screen
    DMAWait();
    DMACopyBitmap((uint16_t*)draw_ctx->buf, w, h, 0, 0,
                  widget->lcdBuffer->getData(), w, h, 0, 0, w, h);
  }
}

// singleton instance
StandaloneLuaWindow* StandaloneLuaWindow::_instance;

StandaloneLuaWindow::StandaloneLuaWindow(bool useLvgl) :
    Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}),
    useLvgl(useLvgl)
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

    lv_obj_add_event_cb(lvobj, StandaloneLuaWindow::redraw_cb, LV_EVENT_DRAW_MAIN, nullptr);
  }

  // setup LUA event handler
  setupHandler(this);

  attach();
}

void StandaloneLuaWindow::setup(bool useLvgl)
{
  if (_instance == nullptr)
    _instance = new StandaloneLuaWindow(useLvgl);
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

  Window::deleteLater(detach, trash);
}

void StandaloneLuaWindow::checkEvents()
{
  Window::checkEvents();

  // Set global LUA LCD buffer
  luaLcdBuffer = lcdBuffer;

  if (luaState != INTERPRETER_RELOAD_PERMANENT_SCRIPTS) {
    if (luaTask(true)) {
      if (useLvglLayout() && !hasError) {
        // if LUA finished a full cycle,
        // call update functions
        PROTECT_LUA() {
          if (!callRefs(lsScripts)) {
            luaShowError();
          }
        } else {
          luaShowError();
        }
        UNPROTECT_LUA();
      } else {
        // if LUA finished a full cycle,
        // invalidate to display the screen buffer
        invalidate();
      }
    }
  }

  if (luaState == INTERPRETER_RELOAD_PERMANENT_SCRIPTS) {
    // Script does not run anymore...
    TRACE("LUA standalone script exited: deleting window!");
    deleteLater();
  }

  // Kill global LUA LCD buffer
  luaLcdBuffer = nullptr;
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
  clearRefs(lsScripts);
  Window::clear();
}

void StandaloneLuaWindow::luaShowError()
{
  hasError = true;
  extern void luaError(lua_State *L, uint8_t error);
  luaError(lsScripts, SCRIPT_SYNTAX_ERROR);
}

void StandaloneLuaWindow::showError(bool firstCall, const char* title, const char* msg)
{
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
