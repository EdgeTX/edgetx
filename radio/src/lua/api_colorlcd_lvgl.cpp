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

#define LUA_LIB

#include "libopenui.h"
#include "lua_api.h"
#include "lua_widget.h"
#include "edgetx.h"

#include "api_colorlcd.h"

LuaLvglManager *luaLvglManager = nullptr;

static int luaLvglObj(lua_State *L, std::function<LvglWidgetObject*()> create, bool standalone = false)
{
  if (luaLvglManager && (!standalone || !luaLvglManager->isWidget())) {
    auto obj = create();
    obj->getParams(L, 1);
    obj->build(L);
    obj->push(L);
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int luaLvglObjEx(lua_State *L, std::function<LvglWidgetObjectBase*()> create, bool standalone = false)
{
  if (luaLvglManager && (!standalone || !luaLvglManager->isWidget())) {
    LvglWidgetObjectBase* p = nullptr;
    LvglWidgetObjectBase* prevParent = nullptr;
    if (lua_gettop(L) == 2) {
      p = LvglWidgetObjectBase::checkLvgl(L, 1);
      if (p) {
        prevParent = luaLvglManager->getTempParent();
        luaLvglManager->setTempParent(p);
      }
    }

    auto obj = create();
    obj->getParams(L, -1);
    obj->build(L);
    obj->push(L);

    if (p)
      luaLvglManager->setTempParent((prevParent));
  } else {
    lua_pushnil(L);
  }

  return 1;
}

static int luaLvglPopup(lua_State *L, std::function<LvglWidgetObjectBase*()> create)
{
  auto obj = create();
  obj->getParams(L, 1);
  obj->build(L);
  return 0;
}

static int luaDestroyLvglWidget(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1);
  if (p) {
    p->clearRefs(L);
    delete p;
  }
  return 0;
}

static int luaLvglSet(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1);
  if (p) {
    p->update(L);
  }
  return 0;
}

static int luaLvglClear(lua_State *L)
{
  if (luaLvglManager) {
    if (lua_gettop(L) == 1) {
      auto p = LvglWidgetObjectBase::checkLvgl(L, 1);
      if (p) {
        p->clear();
      }
    } else {
      luaLvglManager->clear();
    }
  }

  return 0;
}

static int luaLvglShow(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1);
  if (p) {
    p->show();
  }
  return 0;
}

static int luaLvglHide(lua_State *L)
{
  auto p = LvglWidgetObjectBase::checkLvgl(L, 1);
  if (p) {
    p->hide();
  }
  return 0;
}

class LvglWidgetParams
{
 public:
  LvglWidgetParams(lua_State *L, int index = 1)
  {
    luaL_checktype(L, index, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      const char *key = lua_tostring(L, -2);
      if (!strcmp(key, "type")) {
        type = luaL_checkstring(L, -1);
      } else if (!strcmp(key, "name")) {
        name = luaL_checkstring(L, -1);
      } else if (!strcmp(key, "children")) {
        hasChildren = true;
      }
    }
  }

  const char *type = nullptr;
  const char *name = nullptr;
  bool hasChildren = false;
};

static void buildLvgl(lua_State *L, int srcIndex, int refIndex)
{
  luaL_checktype(L, srcIndex, LUA_TTABLE);
  for (lua_pushnil(L); lua_next(L, srcIndex - 1); lua_pop(L, 1)) {
    LvglWidgetParams p(L, -1);
    LvglWidgetObjectBase *obj = nullptr;
    if (strcasecmp(p.type, "label") == 0)
      obj = new LvglWidgetLabel();
    else if (strcasecmp(p.type, "rectangle") == 0)
      obj = new LvglWidgetRectangle();
    else if (strcasecmp(p.type, "circle") == 0)
      obj = new LvglWidgetCircle();
    else if (strcasecmp(p.type, "arc") == 0)
      obj = new LvglWidgetArc();
    else if (strcasecmp(p.type, "hline") == 0)
      obj = new LvglWidgetHLine();
    else if (strcasecmp(p.type, "vline") == 0)
      obj = new LvglWidgetVLine();
    else if (strcasecmp(p.type, "line") == 0)
      obj = new LvglWidgetLine();
    else if (strcasecmp(p.type, "triangle") == 0)
      obj = new LvglWidgetTriangle();
    else if (strcasecmp(p.type, "image") == 0)
      obj = new LvglWidgetImage();
    else if (strcasecmp(p.type, "qrcode") == 0)
      obj = new LvglWidgetQRCode();
    else if (strcasecmp(p.type, "box") == 0)
      obj = new LvglWidgetBox();
    else if (strcasecmp(p.type, "setting") == 0)
      obj = new LvglWidgetSetting();
    else if (!luaLvglManager->isWidget()) {
      if (strcasecmp(p.type, "button") == 0)
        obj = new LvglWidgetTextButton();
      else if (strcasecmp(p.type, "toggle") == 0)
        obj = new LvglWidgetToggleSwitch();
      else if (strcasecmp(p.type, "textEdit") == 0)
        obj = new LvglWidgetTextEdit();
      else if (strcasecmp(p.type, "numberEdit") == 0)
        obj = new LvglWidgetNumberEdit();
      else if (strcasecmp(p.type, "choice") == 0)
        obj = new LvglWidgetChoice();
      else if (strcasecmp(p.type, "slider") == 0)
        obj = new LvglWidgetSlider();
      else if (strcasecmp(p.type, "page") == 0)
        obj = new LvglWidgetPage();
      else if (strcasecmp(p.type, "font") == 0)
        obj = new LvglWidgetFontPicker();
      else if (strcasecmp(p.type, "align") == 0)
        obj = new LvglWidgetAlignPicker();
      else if (strcasecmp(p.type, "color") == 0)
        obj = new LvglWidgetColorPicker();
      else if (strcasecmp(p.type, "timer") == 0)
        obj = new LvglWidgetTimerPicker();
      else if (strcasecmp(p.type, "switch") == 0)
        obj = new LvglWidgetSwitchPicker();
      else if (strcasecmp(p.type, "source") == 0)
        obj = new LvglWidgetSourcePicker();
    }
    if (obj) {
      obj->getParams(L, -1);
      obj->build(L);
      auto ref = obj->getRef(L);
      if (p.name) {
        lua_pushstring(L, p.name);
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        lua_settable(L, refIndex - 4);
      }
      if (p.hasChildren && obj->getWindow()) {
        lua_getfield(L, -1, "children");
        auto prevParent = luaLvglManager->getTempParent();
        luaLvglManager->setTempParent(obj);
        buildLvgl(L, -1, refIndex - 3);
        lua_pop(L, 1);
        luaLvglManager->setTempParent(prevParent);
      }
    }
  }
}

static int luaLvglBuild(lua_State *L)
{
  if (luaLvglManager) {
    LvglWidgetObjectBase* p = nullptr;
    LvglWidgetObjectBase* prevParent = nullptr;
    if (lua_gettop(L) == 2) {
      p = LvglWidgetObjectBase::checkLvgl(L, 1);
      if (p) {
        prevParent = luaLvglManager->getTempParent();
        luaLvglManager->setTempParent(p);
      }
    }

    // Return array of lvgl object references
    lua_newtable(L);
    buildLvgl(L, -2, -1);

    if (p)
      luaLvglManager->setTempParent((prevParent));
  } else {
    lua_pushnil(L);
  }
  return 1;
}

static int luaLvglIsAppMode(lua_State *L)
{
  if (luaLvglManager) {
    lua_pushboolean(L, luaLvglManager->isAppMode());
  } else {
    lua_pushboolean(L, false);
  }
  return 1;
}

// lvgl functions
LROT_BEGIN(lvgllib, NULL, 0)
  LROT_FUNCENTRY(clear, luaLvglClear)
  LROT_FUNCENTRY(build, luaLvglBuild)
  LROT_FUNCENTRY(isAppMode, luaLvglIsAppMode)
  // Objects - widgets and standalone
  LROT_FUNCENTRY(label, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLabel(); }); })
  LROT_FUNCENTRY(rectangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetRectangle(); }); })
  LROT_FUNCENTRY(hline, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetHLine(); }); })
  LROT_FUNCENTRY(vline, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetVLine(); }); })
  LROT_FUNCENTRY(line, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLine(); }); })
  LROT_FUNCENTRY(triangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTriangle(); }); })
  LROT_FUNCENTRY(circle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetCircle(); }); })
  LROT_FUNCENTRY(arc, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetArc(); }); })
  LROT_FUNCENTRY(image, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetImage(); }); })
  LROT_FUNCENTRY(qrcode, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetQRCode(); }); })
  // Objects - standalone scripts only
  LROT_FUNCENTRY(button, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextButton(); }, true); })
  LROT_FUNCENTRY(toggle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetToggleSwitch(); }, true); })
  LROT_FUNCENTRY(textEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextEdit(); }, true); })
  LROT_FUNCENTRY(numberEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetNumberEdit(); }, true); })
  LROT_FUNCENTRY(choice, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetChoice(); }, true); })
  LROT_FUNCENTRY(slider, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSlider(); }, true); })
  LROT_FUNCENTRY(font, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetFontPicker(); }, true); })
  LROT_FUNCENTRY(align, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetAlignPicker(); }, true); })
  LROT_FUNCENTRY(color, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetColorPicker(); }, true); })
  LROT_FUNCENTRY(timer, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTimerPicker(); }, true); })
  LROT_FUNCENTRY(switch, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSwitchPicker(); }, true); })
  LROT_FUNCENTRY(source, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSourcePicker(); }, true); })
  // Containers
  LROT_FUNCENTRY(box, [](lua_State* L) { return luaLvglObj(L, []() { return new LvglWidgetBox(); }, true); })
  LROT_FUNCENTRY(setting, [](lua_State* L) { return luaLvglObj(L, []() { return new LvglWidgetSetting(); }, true); })
  LROT_FUNCENTRY(page, [](lua_State* L) { return luaLvglObj(L, []() { return new LvglWidgetPage(); }, true); })
  LROT_FUNCENTRY(dialog, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetDialog(); }, true); })
  // Dialogs
  LROT_FUNCENTRY(confirm, [](lua_State* L) { return luaLvglPopup(L, []() { return new LvglWidgetConfirmDialog(); }); })
  LROT_FUNCENTRY(message, [](lua_State* L) { return luaLvglPopup(L, []() { return new LvglWidgetMessageDialog(); }); })
  // Object manipulation functions
  LROT_FUNCENTRY(set, luaLvglSet)
  LROT_FUNCENTRY(show, luaLvglShow)
  LROT_FUNCENTRY(hide, luaLvglHide)
  LROT_NUMENTRY(FLOW_ROW, LV_FLEX_FLOW_ROW)
  LROT_NUMENTRY(FLOW_COLUMN, LV_FLEX_FLOW_COLUMN)
  LROT_NUMENTRY(PAD_TINY, PAD_TINY)
  LROT_NUMENTRY(PAD_SMALL, PAD_SMALL)
  LROT_NUMENTRY(PAD_MEDIUM, PAD_MEDIUM)
  LROT_NUMENTRY(PAD_LARGE, PAD_LARGE)
LROT_END(lvgllib, NULL, 0)

// Metatable for simple objects (line, arc, label)
LROT_BEGIN(lvgl_base_mt, NULL, LROT_MASK_GC_INDEX)
  LROT_FUNCENTRY(__gc, luaDestroyLvglWidget)
  LROT_TABENTRY(__index, lvgl_base_mt)
  // Object manipulation functions
  LROT_FUNCENTRY(set, luaLvglSet)
  LROT_FUNCENTRY(show, luaLvglShow)
  LROT_FUNCENTRY(hide, luaLvglHide)
LROT_END(lvgl_base_mt, NULL, LROT_MASK_GC_INDEX)

// Metatable for complex objects
LROT_BEGIN(lvgl_mt, NULL, LROT_MASK_GC_INDEX)
  LROT_FUNCENTRY(__gc, luaDestroyLvglWidget)
  LROT_TABENTRY(__index, lvgl_mt)
  LROT_FUNCENTRY(clear, luaLvglClear)
  LROT_FUNCENTRY(build, luaLvglBuild)
  // Objects - widgets and standalone
  LROT_FUNCENTRY(label, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLabel(); }); })
  LROT_FUNCENTRY(rectangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetRectangle(); }); })
  LROT_FUNCENTRY(hline, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetHLine(); }); })
  LROT_FUNCENTRY(vline, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetVLine(); }); })
  LROT_FUNCENTRY(line, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLine(); }); })
  LROT_FUNCENTRY(triangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTriangle(); }); })
  LROT_FUNCENTRY(circle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetCircle(); }); })
  LROT_FUNCENTRY(arc, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetArc(); }); })
  LROT_FUNCENTRY(image, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetImage(); }); })
  LROT_FUNCENTRY(qrcode, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetQRCode(); }); })
  // Objects - standalone scripts only
  LROT_FUNCENTRY(button, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextButton(); }, true); })
  LROT_FUNCENTRY(toggle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetToggleSwitch(); }, true); })
  LROT_FUNCENTRY(textEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextEdit(); }, true); })
  LROT_FUNCENTRY(numberEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetNumberEdit(); }, true); })
  LROT_FUNCENTRY(choice, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetChoice(); }, true); })
  LROT_FUNCENTRY(slider, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSlider(); }, true); })
  LROT_FUNCENTRY(font, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetFontPicker(); }, true); })
  LROT_FUNCENTRY(align, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetAlignPicker(); }, true); })
  LROT_FUNCENTRY(color, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetColorPicker(); }, true); })
  LROT_FUNCENTRY(timer, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTimerPicker(); }, true); })
  LROT_FUNCENTRY(switch, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSwitchPicker(); }, true); })
  LROT_FUNCENTRY(source, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSourcePicker(); }, true); })
  // Object manipulation functions
  LROT_FUNCENTRY(set, luaLvglSet)
  LROT_FUNCENTRY(show, luaLvglShow)
  LROT_FUNCENTRY(hide, luaLvglHide)
LROT_END(lvgl_mt, NULL, LROT_MASK_GC_INDEX)

extern "C" {
LUALIB_API int luaopen_lvgl(lua_State *L)
{
  luaL_rometatable(L, LVGL_SIMPLEMETATABLE, LROT_TABLEREF(lvgl_base_mt));
  luaL_rometatable(L, LVGL_METATABLE, LROT_TABLEREF(lvgl_mt));
  return 0;
}
}
