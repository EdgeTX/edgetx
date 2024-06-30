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
#include "opentx.h"

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

static int luaLvglLabel(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetLabel(); });
}

static int luaLvglRectangle(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetRectangle(); });
}

static int luaLvglCircle(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetCircle(); });
}

static int luaLvglArc(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetArc(); });
}

static int luaLvglImage(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetImage(); });
}

static int luaLvglQRCode(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetQRCode(); });
}

static int luaLvglMeter(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetMeter(); });
}

static int luaLvglButton(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetTextButton(); }, true);
}

static int luaLvglToggle(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetToggleSwitch(); }, true);
}

static int luaLvglTextEdit(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetTextEdit(); }, true);
}

static int luaLvglNumberEdit(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetNumberEdit(); }, true);
}

static int luaLvglChoice(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetChoice(); }, true);
}

static int luaLvglSlider(lua_State *L)
{
  return luaLvglObj(L, [=]() { return new LvglWidgetSlider(); }, true);
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

static LvglWidgetObject *optLvgl(lua_State *L, int index)
{
  LvglWidgetObject **p =
      (LvglWidgetObject **)luaL_testudata(L, index, LVGL_METATABLE);
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
  if (luaLvglManager) luaLvglManager->clear();

  return 0;
}

static int luaLvglSetParent(lua_State *L)
{
  if (luaLvglManager) {
    auto p = optLvgl(L, 1);
    luaLvglManager->setTempParent(p ? p->getWindow() : nullptr);
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
    else if (strcasecmp(p.type, "meter") == 0)
      lvobj = new LvglWidgetMeter();
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
    // Return array of lvgl object references
    lua_newtable(L);
    buildLvgl(L, -2, -1);
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
LROT_FUNCENTRY(setParent, luaLvglSetParent)
// Objects
LROT_FUNCENTRY(label, luaLvglLabel)
LROT_FUNCENTRY(rectangle, luaLvglRectangle)
LROT_FUNCENTRY(circle, luaLvglCircle)
LROT_FUNCENTRY(arc, luaLvglArc)
LROT_FUNCENTRY(image, luaLvglImage)
LROT_FUNCENTRY(qrcode, luaLvglQRCode)
LROT_FUNCENTRY(meter, luaLvglMeter)
LROT_FUNCENTRY(button, luaLvglButton)
LROT_FUNCENTRY(toggle, luaLvglToggle)
LROT_FUNCENTRY(confirm, luaLvglConfirm)
LROT_FUNCENTRY(textEdit, luaLvglTextEdit)
LROT_FUNCENTRY(numberEdit, luaLvglNumberEdit)
LROT_FUNCENTRY(choice, luaLvglChoice)
LROT_FUNCENTRY(slider, luaLvglSlider)
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
