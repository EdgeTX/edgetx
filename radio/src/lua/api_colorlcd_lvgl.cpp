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

static int luaLvglObjEx(lua_State *L, std::function<LvglWidgetObject*()> create, bool standalone = false)
{
  if (luaLvglManager && (!standalone || !luaLvglManager->isWidget())) {
    LvglWidgetObject* p = nullptr;
    Window* prevParent = nullptr;
    if (lua_gettop(L) == 2) {
      p = LvglWidgetObject::checkLvgl(L, 1);
      if (p) {
        prevParent = luaLvglManager->getTempParent();
        luaLvglManager->setTempParent(p->getWindow());
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

static int luaLvglConfirm(lua_State *L)
{
  auto obj = new LvglWidgetConfirmDialog();
  obj->getParams(L, 1);
  obj->build(L);
  return 0;
}

LvglWidgetObject *LvglWidgetObject::checkLvgl(lua_State *L, int index)
{
  LvglWidgetObject **p =
      (LvglWidgetObject **)luaL_checkudata(L, index, LVGL_METATABLE);
  if (p) return *p;
  return nullptr;
}

static int luaDestroyLvglWidget(lua_State *L)
{
  auto p = LvglWidgetObject::checkLvgl(L, 1);
  if (p) delete p;
  return 0;
}

static int luaLvglSet(lua_State *L)
{
  auto p = LvglWidgetObject::checkLvgl(L, 1);
  if (p) {
    p->update(L);
  }
  return 0;
}

static int luaLvglClear(lua_State *L)
{
  if (luaLvglManager) {
    if (lua_gettop(L) == 1) {
      auto p = LvglWidgetObject::checkLvgl(L, 1);
      if (p)
        p->getWindow()->clear();
    } else {
      luaLvglManager->clear();
    }
  }

  return 0;
}

static int luaLvglShow(lua_State *L)
{
  auto p = LvglWidgetObject::checkLvgl(L, 1);
  if (p) {
    p->show();
  }
  return 0;
}

static int luaLvglHide(lua_State *L)
{
  auto p = LvglWidgetObject::checkLvgl(L, 1);
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
    LvglWidgetObject *lvobj = nullptr;
    if (strcasecmp(p.type, "label") == 0)
      lvobj = new LvglWidgetLabel();
    else if (strcasecmp(p.type, "rectangle") == 0)
      lvobj = new LvglWidgetRectangle();
    else if (strcasecmp(p.type, "circle") == 0)
      lvobj = new LvglWidgetCircle();
    else if (strcasecmp(p.type, "arc") == 0)
      lvobj = new LvglWidgetArc();
    else if (strcasecmp(p.type, "image") == 0)
      lvobj = new LvglWidgetImage();
    else if (strcasecmp(p.type, "qrcode") == 0)
      lvobj = new LvglWidgetQRCode();
    // else if (strcasecmp(p.type, "meter") == 0)
    //   lvobj = new LvglWidgetMeter();
    else if (!luaLvglManager->isWidget()) {
      if (strcasecmp(p.type, "button") == 0)
        lvobj = new LvglWidgetTextButton();
      else if (strcasecmp(p.type, "toggle") == 0)
        lvobj = new LvglWidgetToggleSwitch();
      else if (strcasecmp(p.type, "textEdit") == 0)
        lvobj = new LvglWidgetTextEdit();
      else if (strcasecmp(p.type, "numberEdit") == 0)
        lvobj = new LvglWidgetNumberEdit();
      else if (strcasecmp(p.type, "choice") == 0)
        lvobj = new LvglWidgetChoice();
      else if (strcasecmp(p.type, "slider") == 0)
        lvobj = new LvglWidgetSlider();
      else if (strcasecmp(p.type, "page") == 0)
        lvobj = new LvglWidgetPage();
    }
    if (lvobj) {
      lvobj->getParams(L, -1);
      lvobj->build(L);
      auto ref = lvobj->getRef(L);
      if (p.name) {
        lua_pushstring(L, p.name);
        lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
        lua_settable(L, refIndex - 4);
      }
      if (p.hasChildren) {
        lua_getfield(L, -1, "children");
        auto prevParent = luaLvglManager->getTempParent();
        luaLvglManager->setTempParent(lvobj->getWindow());
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
    LvglWidgetObject* p = nullptr;
    Window* prevParent = nullptr;
    if (lua_gettop(L) == 2) {
      p = LvglWidgetObject::checkLvgl(L, 1);
      if (p) {
        prevParent = luaLvglManager->getTempParent();
        luaLvglManager->setTempParent(p->getWindow());
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

LROT_BEGIN(lvgllib, NULL, LROT_MASK_GC_INDEX)
LROT_FUNCENTRY(__gc, luaDestroyLvglWidget)
LROT_TABENTRY(__index, lvgllib)
LROT_FUNCENTRY(clear, luaLvglClear)
LROT_FUNCENTRY(build, luaLvglBuild)
// Objects - widgets and standalone
LROT_FUNCENTRY(label, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetLabel(); }); })
LROT_FUNCENTRY(rectangle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetRectangle(); }); })
LROT_FUNCENTRY(circle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetCircle(); }); })
LROT_FUNCENTRY(arc, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetArc(); }); })
LROT_FUNCENTRY(image, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetImage(); }); })
LROT_FUNCENTRY(qrcode, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetQRCode(); }); })
// LROT_FUNCENTRY(meter, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetMeter(); }); })
// Objects - standalone scripts only
LROT_FUNCENTRY(button, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextButton(); }, true); })
LROT_FUNCENTRY(toggle, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetToggleSwitch(); }, true); })
LROT_FUNCENTRY(textEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetTextEdit(); }, true); })
LROT_FUNCENTRY(numberEdit, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetNumberEdit(); }, true); })
LROT_FUNCENTRY(choice, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetChoice(); }, true); })
LROT_FUNCENTRY(slider, [](lua_State* L) { return luaLvglObjEx(L, []() { return new LvglWidgetSlider(); }, true); })
// Containers
LROT_FUNCENTRY(page, [](lua_State* L) { return luaLvglObj(L, []() { return new LvglWidgetPage(); }, true); })
// Dialogs
LROT_FUNCENTRY(confirm, luaLvglConfirm)
// Manipulation functions
LROT_FUNCENTRY(set, luaLvglSet)
LROT_FUNCENTRY(show, luaLvglShow)
LROT_FUNCENTRY(hide, luaLvglHide)
LROT_END(lvgllib, NULL, LROT_MASK_GC_INDEX)

extern "C" {
LUALIB_API int luaopen_lvgl(lua_State *L)
{
  luaL_rometatable(L, LVGL_METATABLE, LROT_TABLEREF(lvgllib));
  return 0;
}
}
