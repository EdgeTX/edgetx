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

#include "lua_widget.h"

#include "color_picker.h"
#include "dialog.h"
#include "edgetx.h"
#include "filechoice.h"
#include "keyboard_base.h"
#include "lua_event.h"
#include "menu.h"
#include "numberedit.h"
#include "page.h"
#include "pagegroup.h"
#include "slider.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "textedit.h"
#include "theme_manager.h"
#include "toggleswitch.h"

//-----------------------------------------------------------------------------

static void clearRef(lua_State *L, int& ref)
{
  if (ref != LUA_REFNIL)
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
  ref = LUA_REFNIL;
}

static bool getLuaBool(lua_State *L)
{
  if (lua_isboolean(L, -1))
    return lua_toboolean(L, -1);
  return luaL_checkunsigned(L, -1);
}

//-----------------------------------------------------------------------------

void LvglParamFuncOrValue::parse(lua_State *L)
{
  if (lua_isfunction(L, -1)) {
    function = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (lua_isboolean(L, -1)) {
    value = lua_toboolean(L, -1);
  } else {
    value = luaL_checkunsigned(L, -1);
  }
}

bool LvglParamFuncOrValue::changedColor(LcdFlags newFlags)
{
  flags = newFlags;
  if (flags != currVal) {
    currVal = flags;
    return true;
  }
  return false;
}

bool LvglParamFuncOrValue::changedValue(uint32_t v)
{
  value = v;
  if (v != currVal) {
    currVal = v;
    return true;
  }
  return false;
}

void LvglParamFuncOrValue::clearRef(lua_State *L)
{
  ::clearRef(L, function);
}

void LvglParamFuncOrString::parse(lua_State *L)
{
  if (lua_isfunction(L, -1)) {
    function = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    txt = luaL_checkstring(L, -1);
  }
}

bool LvglParamFuncOrString::changedText(const char* s)
{
  txt = s;
  uint32_t h = hash(chars(), txt.size());
  if (h != txtHash) {
    txtHash = h;
    return true;
  }
  return false;
}

void LvglParamFuncOrString::clearRef(lua_State *L)
{
  ::clearRef(L, function);
}

//-----------------------------------------------------------------------------

bool LvglGetSetParams::parseGetSetParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "get")) {
    getFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    return true;
  }
  if (!strcmp(key, "set")) {
    setFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    return true;
  }
  return false;
}

void LvglGetSetParams::clearGetSetRefs(lua_State *L)
{
  clearRef(L, getFunction);
  clearRef(L, setFunction);
}

bool LvglTextParams::parseTextParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "text")) {
    txt.parse(L);
    return true;
  }
  if (!strcmp(key, "font")) {
    font.parse(L);
    return true;
  }
  return false;
}

void LvglTextParams::clearTextRefs(lua_State *L)
{
  txt.clearRef(L);
  font.clearRef(L);
}

bool LvglMinMaxParams::parseMinMaxParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "min")) {
    min = luaL_checkinteger(L, -1);
    return true;
  }
  if (!strcmp(key, "max")) {
    max = luaL_checkinteger(L, -1);
    return true;
  }
  return false;
}

bool LvglTitleParam::parseTitleParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "title")) {
    title.parse(L);
    return true;
  }
  return false;
}

void LvglTitleParam::clearTitleRefs(lua_State *L)
{
  title.clearRef(L);
}

bool LvglMessageParam::parseMessageParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "message")) {
    message = luaL_checkstring(L, -1);
    return true;
  }
  return false;
}

bool LvglRoundedParam::parseRoundedParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "rounded")) {
    rounded = getLuaBool(L);
    return true;
  }
  return false;
}

bool LvglAlignParam::parseAlignParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "align")) {
    align.parse(L);
    return true;
  }
  return false;
}

void LvglAlignParam::clearAlignRefs(lua_State *L)
{
  align.clearRef(L);
}

bool LvglThicknessParam::parseThicknessParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "thickness")) {
    thickness = luaL_checkunsigned(L, -1);
    return true;
  }
  return false;
}

bool LvglValuesParam::parseValuesParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "values")) {
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      values.push_back(lua_tostring(L, -1));
    }
    return true;
  }
  return false;
}

bool LvglScrollableParams::parseScrollableParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "scrollBar")) {
    showScrollBar = getLuaBool(L);
    return true;
  }
  if (!strcmp(key, "scrollDir")) {
    scrollDir = (lv_dir_t)luaL_checkunsigned(L, -1);
    return true;
  }
  if (!strcmp(key, "scrollTo")) {
    scrollToFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    return true;
  }
  if (!strcmp(key, "scrolled")) {
    scrolledFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------

static int pcallcont (lua_State *L, int status, lua_KContext extra) {
  return status == LUA_OK;
}

static bool pcallFunc(lua_State *L, int funcRef, int nretval)
{
  if (funcRef != LUA_REFNIL) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
    return lua_pcallk(L, 0, nretval, 0, 0, pcallcont) == LUA_OK;
  }
  return false;
}

static bool pcallFuncWithInt(lua_State *L, int funcRef, int nretval, int val)
{
  if (funcRef != LUA_REFNIL) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
    lua_pushinteger(L, val);
    return lua_pcallk(L, 1, nretval, 0, 0, pcallcont) == LUA_OK;
  }
  return false;
}

static bool pcallFuncWith2Int(lua_State *L, int funcRef, int nretval, int val1, int val2)
{
  if (funcRef != LUA_REFNIL) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
    lua_pushinteger(L, val1);
    lua_pushinteger(L, val2);
    return lua_pcallk(L, 2, nretval, 0, 0, pcallcont) == LUA_OK;
  }
  return false;
}

static bool pcallFuncWithString(lua_State *L, int funcRef, int nretval, const char* val)
{
  if (funcRef != LUA_REFNIL) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
    lua_pushstring(L, val);
    return lua_pcallk(L, 1, nretval, 0, 0, pcallcont) == LUA_OK;
  }
  return false;
}

//-----------------------------------------------------------------------------

LvglWidgetObjectBase *LvglWidgetObjectBase::checkLvgl(lua_State *L, int index, bool required)
{
  LvglWidgetObjectBase **p;
  
  p = (LvglWidgetObjectBase **)luaL_testudata(L, index, LVGL_METATABLE);
  if (p && *p) return *p;

  p = (LvglWidgetObjectBase **)luaL_testudata(L, index, LVGL_SIMPLEMETATABLE);
  if (p && *p) return *p;

  if (required) {
    luaL_error(L, "Invalid lvgl object (it has been probably been cleared).");
  }

  return nullptr;
}

//-----------------------------------------------------------------------------

LvglWidgetObjectBase::LvglWidgetObjectBase(const char* meta) :
    metatable(meta),
    lvglManager(luaScriptManager)
{
}

int LvglWidgetObjectBase::getRef(lua_State *L)
{
  LvglWidgetObjectBase **p =
      (LvglWidgetObjectBase **)lua_newuserdata(L, sizeof(LvglWidgetObjectBase *));
  *p = this;
  luaL_getmetatable(L, metatable);
  lua_setmetatable(L, -2);

  // Save reference
  luaRef = luaL_ref(L, LUA_REGISTRYINDEX);
  lvglManager->saveLvglObjectRef(luaRef);

  return luaRef;
}

void LvglWidgetObjectBase::push(lua_State *L)
{
  // Save reference
  auto ref = getRef(L);
  // Push userdata back into Lua stack (return object)
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
}

void LvglWidgetObjectBase::getParams(lua_State *L, int index)
{
  luaL_checktype(L, index, LUA_TTABLE);
  for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
    int top = lua_gettop(L);
    parseParam(L, lua_tostring(L, -2));
    // Replace value on Lua stack if consumed by parseParam
    if (top != lua_gettop(L)) lua_pushnil(L);
  }
}

void LvglWidgetObjectBase::pcallSimpleFunc(lua_State *L, int funcRef)
{
  if (funcRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    PROTECT_LUA()
    {
      if (!pcallFunc(L, funcRef, 0)) {
        lvglManager->luaShowError();
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    luaScriptManager = save;
  }
}

bool LvglWidgetObjectBase::pcallUpdateBool(lua_State *L, int getFuncRef,
                                           std::function<void(bool)> update)
{
  bool res = true;
  if (getFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (pcallFunc(L, getFuncRef, 1)) {
        bool val = false;
        if (lua_isboolean(L, -1))
          val = lua_toboolean(L, -1);
        else
          val = luaL_optinteger(L, -1, 0) != 0;
        update(val);
      } else {
        res = false;
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
  return res;
}

bool LvglWidgetObjectBase::pcallUpdate1Int(lua_State *L, int getFuncRef,
                                           std::function<void(int)> update)
{
  bool res = true;
  if (getFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (pcallFunc(L, getFuncRef, 1)) {
        int val = luaL_checkunsigned(L, -1);
        update(val);
      } else {
        res = false;
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
  return res;
}

bool LvglWidgetObjectBase::pcallUpdate2Int(lua_State *L, int getFuncRef,
                                           std::function<void(int, int)> update)
{
  bool res = true;
  if (getFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (pcallFunc(L, getFuncRef, 2)) {
        int v1 = luaL_checkunsigned(L, -2);
        int v2 = luaL_checkunsigned(L, -1);
        update(v1, v2);
      } else {
        res = false;
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
  return res;
}

bool LvglWidgetObjectBase::pcallUpdateStringVal(lua_State *L, int getFuncRef,
                                           std::function<void(const char*)> update)
{
  bool res = true;
  if (getFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (pcallFunc(L, getFuncRef, 1)) {
        const char* val = luaL_checkstring(L, -1);
        update(val);
      } else {
        res = false;
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
  return res;
}

int LvglWidgetObjectBase::pcallGetIntVal(lua_State *L, int getFuncRef)
{
  int val = 0;
  if (getFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (pcallFunc(L, getFuncRef, 1)) {
        if (lua_isboolean(L, -1))
          val = lua_toboolean(L, -1);
        else
          val = luaL_checkinteger(L, -1);
      } else {
        lvglManager->luaShowError();
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
  return val;
}

int LvglWidgetObjectBase::pcallGetOptIntVal(lua_State *L, int getFuncRef, int defVal)
{
  int val = defVal;
  if (getFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (pcallFunc(L, getFuncRef, 1)) {
        if (lua_isboolean(L, -1))
          val = lua_toboolean(L, -1);
        else
          val = luaL_optinteger(L, -1, defVal);
      } else {
        lvglManager->luaShowError();
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
  return val;
}

void LvglWidgetObjectBase::pcallSetIntVal(lua_State *L, int setFuncRef, int val)
{
  if (setFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (!pcallFuncWithInt(L, setFuncRef, 0, val)) {
        lvglManager->luaShowError();
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
}

const char* LvglWidgetObjectBase::pcallGetStringVal(lua_State *L, int getFuncRef)
{
  const char* val = "";
  if (getFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (pcallFunc(L, getFuncRef, 1)) {
        val = luaL_checkstring(L, -1);
      } else {
        lvglManager->luaShowError();
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
  return val;
}

void LvglWidgetObjectBase::pcallSetStringVal(lua_State *L, int setFuncRef, const char* val)
{
  if (setFuncRef != LUA_REFNIL) {
    auto save = luaScriptManager;
    luaScriptManager = lvglManager;
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (!pcallFuncWithString(L, setFuncRef, 0, val)) {
        lvglManager->luaShowError();
      }
    } else {
      lvglManager->luaShowError();
    }
    UNPROTECT_LUA();
    lua_settop(L, t);
    luaScriptManager = save;
  }
}

void LvglWidgetObjectBase::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "x")) {
    x = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "y")) {
    y = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "w")) {
    w = luaL_checkinteger(L, -1);
    if (w == 0) w = LV_SIZE_CONTENT;
  } else if (!strcmp(key, "h")) {
    h = luaL_checkinteger(L, -1);
    if (h == 0) h = LV_SIZE_CONTENT;
  } else if (!strcmp(key, "color")) {
    color.parse(L);
  } else if (!strcmp(key, "opacity")) {
    opacity.parse(L);
  } else if (!strcmp(key, "visible")) {
    getVisibleFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "size")) {
    getSizeFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "pos")) {
    getPosFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  }
}

void LvglWidgetObjectBase::clear()
{
  clearRequest = true;
  if (getWindow())
    getWindow()->clear();
}

bool LvglWidgetObjectBase::callRefs(lua_State *L)
{
  if (clearRequest) {
    clearRequest = false;
    clearChildRefs(L);
    return true;
  }

  if (!pcallUpdateBool(L, getVisibleFunction,
                       [=](bool visible) { if (visible) show(); else hide(); }))
    return false;

  if (isVisible()) {
    if (!pcallUpdate1Int(L, color.function,
                        [=](int color) { setColor(color); }))
      return false;
    if (!pcallUpdate1Int(L, opacity.function,
                        [=](int opa) { setOpacity(opa); }))
      return false;
    if (!pcallUpdate2Int(L, getSizeFunction,
                        [=](int w, int h) { setSize(w, h); }))
      return false;
    if (!pcallUpdate2Int(L, getPosFunction, [=](int x, int y) { setPos(x, y); }))
      return false;

    for (size_t i = 0; i < lvglObjectRefs.size(); i += 1) {
      lua_rawgeti(L, LUA_REGISTRYINDEX, lvglObjectRefs[i]);
      auto p = LvglWidgetObjectBase::checkLvgl(L, -1);
      lua_pop(L, 1);
      if (p) if (!p->callRefs(L)) return false;
      if (clearRequest) {
        clearRequest = false;
        clearChildRefs(L);
        return true;
      }
    }
  }

  return true;
}

void LvglWidgetObjectBase::saveLvglObjectRef(int ref)
{
  lvglObjectRefs.push_back(ref);
}

void LvglWidgetObjectBase::clearChildRefs(lua_State *L)
{
  for (size_t i = 0; i < lvglObjectRefs.size(); i += 1) {
    lua_rawgeti(L, LUA_REGISTRYINDEX, lvglObjectRefs[i]);
    auto p = LvglWidgetObjectBase::checkLvgl(L, -1);
    lua_pop(L, 1);
    if (p) p->clearRefs(L);
  }
  lvglObjectRefs.clear();
}

void LvglWidgetObjectBase::clearRefs(lua_State *L)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, luaRef);
  auto p = (LvglWidgetObjectBase **)luaL_testudata(L, -1, metatable);
  if (p) *p = nullptr;
  lua_pop(L, 1);

  clearRef(L, luaRef);
  color.clearRef(L);
  opacity.clearRef(L);
  clearRef(L, getVisibleFunction);
  clearRef(L, getSizeFunction);
  clearRef(L, getPosFunction);

  clearChildRefs(L);
}

void LvglWidgetObjectBase::build(lua_State *L) {}

void LvglWidgetObjectBase::refresh()
{
  setPos(x, y);
  setSize(w, h);
  setColor(color.flags);
  setOpacity(opacity.value);
}

void LvglWidgetObjectBase::create(lua_State *L, int index)
{
  getParams(L, index);
  build(L);
  callRefs(L);
}

void LvglWidgetObjectBase::update(lua_State *L)
{
  getParams(L, 2);
  refresh();
}

//-----------------------------------------------------------------------------

void LvglSimpleWidgetObject::setPos(coord_t x, coord_t y)
{
  this->x = x;
  this->y = y;
  if (lvobj) lv_obj_set_pos(lvobj, x, y);
}

void LvglSimpleWidgetObject::setSize(coord_t w, coord_t h)
{
  this->w = w;
  this->h = h;
  if (lvobj) lv_obj_set_size(lvobj, w, h);
}

void LvglSimpleWidgetObject::show()
{
  if (!isVisible()) lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
}

void LvglSimpleWidgetObject::hide()
{
  if (isVisible()) lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
}

bool LvglSimpleWidgetObject::isVisible()
{
  return lvobj && !lv_obj_has_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
}

//-----------------------------------------------------------------------------

void LvglWidgetLabel::parseParam(lua_State *L, const char *key)
{
  if (parseAlignParam(L, key)) return;
  if (parseTextParam(L, key)) return;
  LvglSimpleWidgetObject::parseParam(L, key);
}

bool LvglWidgetLabel::callRefs(lua_State *L)
{
  if (!LvglSimpleWidgetObject::callRefs(L)) return false;

  if (isVisible()) {
    if (!pcallUpdateStringVal(L, txt.function, [=](const char* s) { setText(s); }))
      return false;
    if (!pcallUpdate1Int(L, font.function, [=](int val) { setFont(val); }))
      return false;
    if (!pcallUpdate1Int(L, align.function, [=](int val) { setAlign(val); }))
      return false;
  }

  return true;
}

void LvglWidgetLabel::clearRefs(lua_State *L)
{
  clearAlignRefs(L);
  clearTextRefs(L);
  LvglSimpleWidgetObject::clearRefs(L);
}

void LvglWidgetLabel::setText(const char *s)
{
  if (lvobj && txt.changedText(s))
    lv_label_set_text(lvobj, txt.chars());
}

void LvglWidgetLabel::setColor(LcdFlags newColor)
{
  if (lvobj && color.changedColor(newColor))
    etx_txt_color_from_flags(lvobj, color.flags);
}

void LvglWidgetLabel::setFont(LcdFlags newFont)
{
  if (lvobj && font.changedFont(newFont))
    etx_font(lvobj, FONT_INDEX(font.flags));
}

void LvglWidgetLabel::setAlign(LcdFlags newAlign)
{
  if (lvobj) {
    align.flags = newAlign;
    if (align.flags & VCENTERED) {
      lv_obj_align(lvobj, LV_ALIGN_LEFT_MID, 0, 0);
    }
    lv_obj_set_style_text_align(lvobj,
                                (align.flags & RIGHT)      ? LV_TEXT_ALIGN_RIGHT
                                : (align.flags & CENTERED) ? LV_TEXT_ALIGN_CENTER
                                                    : LV_TEXT_ALIGN_LEFT,
                                LV_PART_MAIN);
  }
}

void LvglWidgetLabel::build(lua_State *L)
{
  lvobj = etx_label_create(lvglManager->getCurrentParent()->getLvObj());
  setPos(x, y);
  setSize(w, h);
  setText(txt.chars());
  setColor(color.flags);
  setOpacity(opacity.value);
  setFont(font.flags);
  setAlign(align.flags);
}

//-----------------------------------------------------------------------------

void LvglWidgetLineBase::parseParam(lua_State *L, const char *key)
{
  if (parseRoundedParam(L, key)) return;
  if (!strcmp(key, "dashGap")) {
    dashGap = luaL_checkinteger(L, -1);
    if (dashGap < 0) dashGap = 0;
  } else if (!strcmp(key, "dashWidth")) {
    dashWidth = luaL_checkinteger(L, -1);
    if (dashWidth < 0) dashWidth = 0;
  } else {
    LvglSimpleWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetLineBase::setColor(LcdFlags newColor)
{
  if (lvobj && color.changedColor(newColor)) {
    etx_line_color_from_flags(lvobj, color.flags);
  }
}

void LvglWidgetLineBase::setOpacity(uint8_t newOpa)
{
  if (opacity.changedValue(newOpa))
    lv_obj_set_style_line_opa(lvobj, opacity.value, LV_PART_MAIN);
}

void LvglWidgetLineBase::setPos(coord_t x, coord_t y)
{
  this->x = x;
  this->y = y;
  setLine();
}

void LvglWidgetLineBase::setSize(coord_t w, coord_t h)
{
  this->w = w;
  this->h = h;
  setLine();
}

void LvglWidgetLineBase::build(lua_State *L)
{
  lvobj = lv_line_create(lvglManager->getCurrentParent()->getLvObj());
  refresh();
}

void LvglWidgetLineBase::refresh()
{
  setColor(color.flags);
  setOpacity(opacity.value);
  setLine();
  lv_obj_set_style_line_rounded(lvobj, rounded, LV_PART_MAIN);
  if (dashGap > 0 && dashWidth > 0) {
    lv_obj_set_style_line_dash_gap(lvobj, dashGap, LV_PART_MAIN);
    lv_obj_set_style_line_dash_width(lvobj, dashWidth, LV_PART_MAIN);
  }
}

//-----------------------------------------------------------------------------

void LvglWidgetHLine::setLine()
{
  if (lvobj) {
    pts[0].x = x;
    pts[1].x = x + w;
    pts[0].y = y;
    pts[1].y = y;
    lv_line_set_points(lvobj, pts, 2);
    lv_obj_set_style_line_width(lvobj, abs(h), LV_PART_MAIN);
  }
}

//-----------------------------------------------------------------------------

void LvglWidgetVLine::setLine()
{
  if (lvobj) {
    pts[0].x = x;
    pts[1].x = x;
    pts[0].y = y;
    pts[1].y = y + h;
    lv_line_set_points(lvobj, pts, 2);
    lv_obj_set_style_line_width(lvobj, abs(w), LV_PART_MAIN);
  }
}

//-----------------------------------------------------------------------------

LvglWidgetLine::LvglWidgetLine() : LvglSimpleWidgetObject()
{
  parent = lvglManager->getCurrentParent()->getLvObj();
}

LvglWidgetLine::~LvglWidgetLine()
{
  if (pts)
    delete pts;
  pts = nullptr;
}

void LvglWidgetLine::getPt(lua_State* L, int n)
{
  lua_rawgeti(L, -1, n + 1);
  luaL_checktype(L, -1, LUA_TTABLE);
  lua_rawgeti(L, -1, 1);
  pts[n].x = luaL_checkunsigned(L, -1);
  lua_pop(L, 1);
  lua_rawgeti(L, -1, 2);
  pts[n].y = luaL_checkunsigned(L, -1);
  lua_pop(L, 2);
}

uint32_t LvglWidgetLine::getPts(lua_State* L)
{
  luaL_checktype(L, -1, LUA_TTABLE);
  size_t newPtCnt = lua_rawlen(L, -1);
  if (newPtCnt > 1) {
    ptCnt = newPtCnt;
    if (pts && ptCnt > ptAlloc) {
      delete pts;
      pts = nullptr;
    }
    if (!pts) {
      pts = new lv_point_t[ptCnt];
      ptAlloc = ptCnt;
    }
    for (size_t i = 0; i < ptCnt; i += 1)
      getPt(L, i);
    return hash(pts, ptCnt * sizeof(lv_point_t));
  } else {
    ptCnt = 0;
    return -1;
  }
}

void LvglWidgetLine::parseParam(lua_State *L, const char *key)
{
  if (parseRoundedParam(L, key)) return;
  if (parseThicknessParam(L, key)) return;
  if (!strcmp(key, "pts")) {
    if (lua_isfunction(L, -1)) {
      getPointsFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
      ptsHash = getPts(L);
    }
  } else {
    LvglSimpleWidgetObject::parseParam(L, key);
  }
}

bool LvglWidgetLine::callRefs(lua_State *L)
{
  if (!LvglSimpleWidgetObject::callRefs(L)) return false;

  if (isVisible()) {
    int t = lua_gettop(L);
    if (getPointsFunction != LUA_REFNIL) {
      if (pcallFunc(L, getPointsFunction, 1)) {
        bool firstPts = (pts == nullptr);
        uint32_t h = getPts(L);
        if (h != ptsHash) {
          ptsHash = h;
          setLine();
        }
        if (firstPts) {
          setColor(color.flags);
          setOpacity(opacity.value);
        }
        lua_settop(L, t);
      } else {
        return false;
      }
    }
  }

  return true;
}

void LvglWidgetLine::clearRefs(lua_State *L)
{
  clearRef(L, getPointsFunction);
  LvglSimpleWidgetObject::clearRefs(L);
}

void LvglWidgetLine::setColor(LcdFlags newColor)
{
  if (lvobj && color.changedColor(newColor)) {
    etx_line_color_from_flags(lvobj, color.flags);
  }
}

void LvglWidgetLine::setOpacity(uint8_t newOpa)
{
  if (lvobj)
    if (opacity.changedValue(newOpa))
      lv_obj_set_style_line_opa(lvobj, opacity.value, LV_PART_MAIN);
}

void LvglWidgetLine::setPos(coord_t x, coord_t y)
{
  if (pts) {
    coord_t dx = this->x - x;
    coord_t dy = this->y - y;
    for (size_t i = 0; i < ptCnt; i += 1) {
      pts[i].x += dx;
      pts[i].y += dy;
    }
    setLine();
  }
}

void LvglWidgetLine::setSize(coord_t w, coord_t h)
{
}

void LvglWidgetLine::setLine()
{
  if (pts) {
    if (!lvobj)
      lvobj = lv_line_create(parent);

    x = pts[0].x;
    y = pts[0].y;
    for (size_t i = 1; i < ptCnt; i += 1) {
      if (pts[i].x < x) x = pts[i].x;
      if (pts[i].y < y) y = pts[i].y;
    }

    lv_line_set_points(lvobj, pts, ptCnt);
    lv_obj_set_style_line_width(lvobj, thickness, LV_PART_MAIN);
    lv_obj_set_style_line_rounded(lvobj, rounded, LV_PART_MAIN);
  }
}

void LvglWidgetLine::build(lua_State *L)
{
  if (pts) {
    setLine();
    setColor(color.flags);
    setOpacity(opacity.value);
  }
}

void LvglWidgetLine::refresh()
{
  color.currVal = -1;
  build(nullptr);
}

bool LvglWidgetLine::isVisible()
{
  return !lvobj || !lv_obj_has_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
}

//-----------------------------------------------------------------------------

LvglWidgetTriangle::LvglWidgetTriangle() : LvglSimpleWidgetObject()
{
  parent = lvglManager->getCurrentParent()->getLvObj();
}

LvglWidgetTriangle::~LvglWidgetTriangle()
{
  if (mask)
    free(mask);
  mask = nullptr;
}

void LvglWidgetTriangle::getPt(lua_State* L, int n)
{
  lua_rawgeti(L, -1, n + 1);
  luaL_checktype(L, -1, LUA_TTABLE);
  lua_rawgeti(L, -1, 1);
  pts[n].x = luaL_checkunsigned(L, -1);
  lua_pop(L, 1);
  lua_rawgeti(L, -1, 2);
  pts[n].y = luaL_checkunsigned(L, -1);
  lua_pop(L, 2);
}

void LvglWidgetTriangle::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "pts")) {
    if (lua_isfunction(L, -1)) {
      getPointsFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
      luaL_checktype(L, -1, LUA_TTABLE);
      getPt(L, 0);
      getPt(L, 1);
      getPt(L, 2);
    }
  } else {
    LvglSimpleWidgetObject::parseParam(L, key);
  }
}

bool LvglWidgetTriangle::callRefs(lua_State *L)
{
  if (!LvglSimpleWidgetObject::callRefs(L)) return false;

  if (isVisible()) {
    int t = lua_gettop(L);
    if (getPointsFunction != LUA_REFNIL) {
      if (pcallFunc(L, getPointsFunction, 1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        getPt(L, 0);
        getPt(L, 1);
        getPt(L, 2);
        lua_settop(L, t);
        uint32_t h = hash(&pts, sizeof(pts));
        if (h != ptsHash) {
          ptsHash = h;
          refresh();
        }
      } else {
        return false;
      }
    }
  }

  return true;
}

void LvglWidgetTriangle::clearRefs(lua_State *L)
{
  clearRef(L, getPointsFunction);
  LvglSimpleWidgetObject::clearRefs(L);
}

void LvglWidgetTriangle::setColor(LcdFlags newColor)
{
  if (lvobj && color.changedColor(newColor)) {
    if (color.flags & RGB_FLAG) {
      etx_remove_img_color(lvobj);
      lv_obj_set_style_img_recolor(lvobj, makeLvColor(color.flags), LV_PART_MAIN);
      lv_obj_set_style_img_recolor_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);
    } else {
      lv_obj_remove_local_style_prop(lvobj, LV_STYLE_IMG_RECOLOR, LV_PART_MAIN);
      etx_img_color(lvobj, (LcdColorIndex)COLOR_VAL(color.flags));
    }
  }
}

void LvglWidgetTriangle::setSize(coord_t w, coord_t h)
{
  // TODO: Scale triangle?
}

void LvglWidgetTriangle::fillLine(coord_t x1, coord_t x2, coord_t y)
{
  memset(&mask->data[y * w + x1], opacity.value, x2 - x1 + 1);
}

// Swap two bytes
#define SWAP(x,y) (x)=(x)^(y); (y)=(x)^(y); (x)=(x)^(y);

void LvglWidgetTriangle::fillTriangle()
{
  if (!mask) return;

  // Convert to relative coords
  coord_t x1 = pts[0].x - x, y1 = pts[0].y - y;
  coord_t x2 = pts[1].x - x, y2 = pts[1].y - y;
  coord_t x3 = pts[2].x - x, y3 = pts[2].y - y;

  coord_t t1x, t2x, ty, minx, maxx, t1xp, t2xp;
  bool changed1 = false;
  bool changed2 = false;
  coord_t signx1, signx2, dx1, dy1, dx2, dy2;
  coord_t e1, e2;

  // Sort vertices
  if (y1 > y2) { SWAP(y1, y2); SWAP(x1, x2); }
  if (y1 > y3) { SWAP(y1, y3); SWAP(x1, x3); }
  if (y2 > y3) { SWAP(y2, y3); SWAP(x2, x3); }

  t1x = t2x = x1; ty = y1;   // Starting points

  dx1 = (coord_t)(x2 - x1); if(dx1 < 0) { dx1 = -dx1; signx1 = -1; } else signx1 = 1;
  dy1 = (coord_t)(y2 - y1);
 
  dx2 = (coord_t)(x3 - x1); if(dx2 < 0) { dx2 = -dx2; signx2 = -1; } else signx2 = 1;
  dy2 = (coord_t)(y3 - y1);

  if (dy1 > dx1) {   // swap values
    SWAP(dx1,dy1);
    changed1 = true;
  }
  if (dy2 > dx2) {   // swap values
    SWAP(dy2,dx2);
    changed2 = true;
  }

  e2 = (coord_t)(dx2 >> 1);
  // Flat top, just process the second half
  if (y1 == y2) goto next;
  e1 = (coord_t)(dx1 >> 1);

  for (coord_t i = 0; i < dx1;) {
    t1xp = 0; t2xp = 0;
    if (t1x < t2x) { minx = t1x; maxx = t2x; }
    else { minx = t2x; maxx = t1x; }
    // process first line until ty value is about to change
    while (i < dx1) {
      i++;
      e1 += dy1;
      while (e1 >= dx1) {
        e1 -= dx1;
        if (changed1) t1xp = signx1;
        else goto next1;
      }
      if (changed1) break;
      else t1x += signx1;
    }
    // Move line
next1:
    // process second line until ty value is about to change
    while (1) {
      e2 += dy2;
      while (e2 >= dx2) {
        e2 -= dx2;
        if (changed2) t2xp = signx2;
        else goto next2;
      }
      if (changed2) break;
      else t2x += signx2;
    }
next2:
    if (minx > t1x) minx = t1x;
    if (minx > t2x) minx = t2x;
    if (maxx < t1x) maxx = t1x;
    if (maxx < t2x) maxx = t2x;
    fillLine(minx, maxx, ty); // Draw line from min to max points found on the y
    // Now increase y
    if (!changed1) t1x += signx1;
    t1x += t1xp;
    if (!changed2) t2x += signx2;
    t2x += t2xp;
    ty += 1;
    if (ty == y2) break;
  }
next:
  // Second half
  dx1 = (int8_t)(x3 - x2); if(dx1<0) { dx1 = -dx1; signx1 = -1; } else signx1 = 1;
  dy1 = (int8_t)(y3 - y2);
  t1x = x2;

  if (dy1 > dx1) { // swap values
    SWAP(dy1,dx1);
    changed1 = true;
  } else changed1 = false;

  e1 = (coord_t)(dx1 >> 1);

  for (coord_t i = 0; i <= dx1; i++) {
    t1xp = 0; t2xp = 0;
    if (t1x < t2x) { minx = t1x; maxx = t2x; }
    else { minx = t2x; maxx = t1x; }
    // process first line until ty value is about to change
    while (i < dx1) {
      e1 += dy1;
      while (e1 >= dx1) {
        e1 -= dx1;
        if (changed1) { t1xp = signx1; break; }
        else goto next3;
      }
      if (changed1) break;
      else t1x += signx1;
      if (i < dx1) i++;
    }
next3:
    // process second line until ty value is about to change
    while (t2x != x3) {
      e2 += dy2;
      while (e2 >= dx2) {
        e2 -= dx2;
        if(changed2) t2xp = signx2;
        else goto next4;
      }
      if (changed2) break;
      else t2x += signx2;
    }
next4:
    if (minx > t1x) minx = t1x;
    if (minx > t2x) minx = t2x;
    if (maxx < t1x) maxx = t1x;
    if (maxx < t2x) maxx = t2x;
    fillLine(minx, maxx, ty); // Draw line from min to max points found on the y
    // Now increase y
    if (!changed1) t1x += signx1;
    t1x += t1xp;
    if (!changed2) t2x += signx2;
    t2x += t2xp;
    ty += 1;
    if (ty > y3) return;
  }
}

void LvglWidgetTriangle::build(lua_State *L)
{
  // Bounds
  x = min(min(pts[0].x, pts[1].x), pts[2].x);
  y = min(min(pts[0].y, pts[1].y), pts[2].y);
  w = max(max(pts[0].x, pts[1].x), pts[2].x) - x + 1;
  h = max(max(pts[0].y, pts[1].y), pts[2].y) - y + 1;
  
  // Allocate mask
  size_t size = w * h;
  mask = (MaskBitmap*)malloc(size + 4);
  if (mask) {
    mask->width = w;
    mask->height = h;
    memset(mask->data, 0, size);

    // Draw triangle
    fillTriangle();

    // Create canvas from mask buffer
    if (lvobj == nullptr)
      lvobj = lv_canvas_create(parent);
    lv_canvas_set_buffer(lvobj, (void*)mask->data, mask->width, mask->height,
                        LV_IMG_CF_ALPHA_8BIT);

    // Set position and, size
    setPos(x, y);
    LvglSimpleWidgetObject::setSize(w,h);

    // Set color
    setColor(color.flags);
  }
}

void LvglWidgetTriangle::refresh()
{
  if (mask) {
    free(mask);
    mask = nullptr;
  }
  if (lvobj) {
    // May render incorrectly when trying to reuse previous canvas
    lv_obj_del(lvobj);
    lvobj = nullptr;
  }
  color.forceUpdate();
  build(nullptr);
}

//-----------------------------------------------------------------------------

void LvglWidgetObject::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "flexFlow")) {
    flexFlow = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "flexPad")) {
    flexPad = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "borderPad")) {
    customPad = true;
    if (lua_isinteger(L, -1)) {
      int8_t pad = luaL_checkinteger(L, -1);
      borderPadLeft = borderPadRight = borderPadTop = borderPadBottom = pad;
    } else {
      luaL_checktype(L, -1, LUA_TTABLE);
      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        const char *key = lua_tostring(L, -2);
        if (!strcmp(key, "left")) {
          borderPadLeft = luaL_checkinteger(L, -1);
        } else if (!strcmp(key, "right")) {
          borderPadRight = luaL_checkinteger(L, -1);
        } else if (!strcmp(key, "top")) {
          borderPadTop = luaL_checkinteger(L, -1);
        } else if (!strcmp(key, "bottom")) {
          borderPadBottom = luaL_checkinteger(L, -1);
        }
      }
    }
  } else if (!strcmp(key, "active")) {
    getActiveFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObjectBase::parseParam(L, key);
  }
}

void LvglWidgetObject::setPos(coord_t x, coord_t y)
{
  this->x = x;
  this->y = y;
  if (window) window->setPos(x, y);
}

void LvglWidgetObject::setSize(coord_t w, coord_t h)
{
  this->w = w;
  this->h = h;
  if (window) window->setSize(w, h);
}

bool LvglWidgetObject::setFlex()
{
  if (customPad) {
    lv_obj_set_style_pad_left(window->getLvObj(), borderPadLeft, 0);
    lv_obj_set_style_pad_right(window->getLvObj(), borderPadRight, 0);
    lv_obj_set_style_pad_top(window->getLvObj(), borderPadTop, 0);
    lv_obj_set_style_pad_bottom(window->getLvObj(), borderPadBottom, 0);
  } else {
    window->padAll(flexFlow >= 0 ? PAD_OUTLINE : PAD_ZERO);
  }

  if (flexFlow >= 0) {
    window->setFlexLayout((lv_flex_flow_t)flexFlow, flexPad, w, h);
    return true;
  }

  return false;
}

bool LvglWidgetObject::callRefs(lua_State *L)
{
  if (!LvglWidgetObjectBase::callRefs(L)) return false;

  if (isVisible()) {
    if (!pcallUpdateBool(L, getActiveFunction,
                         [=](bool active) { if (active) enable(); else disable(); }))
      return false;
  }

  return true;
}

void LvglWidgetObject::clearRefs(lua_State *L)
{
  clearRef(L, getActiveFunction);
  LvglWidgetObjectBase::clearRefs(L);
}

//-----------------------------------------------------------------------------

void LvglWidgetBox::parseParam(lua_State *L, const char *key)
{
  if (parseAlignParam(L, key)) return;
  if (parseScrollableParam(L, key)) return;
  LvglWidgetObject::parseParam(L, key);
}

coord_t LvglWidgetBox::getScrollX()
{
  return lv_obj_get_scroll_x(window->getLvObj());
}

coord_t LvglWidgetBox::getScrollY()
{
  return lv_obj_get_scroll_y(window->getLvObj());
}

bool LvglWidgetBox::callRefs(lua_State *L)
{
  if (!LvglWidgetObject::callRefs(L)) return false;

  if (isVisible()) {
    if (!pcallUpdate2Int(L, scrollToFunction,
        [=](int x, int y) { if (x != getScrollX() || y != getScrollY()) lv_obj_scroll_to(window->getLvObj(), x, y, LV_ANIM_OFF); })) {
      return false;
    }
  }

  return true;
}

void LvglWidgetBox::clearRefs(lua_State *L)
{
  clearAlignRefs(L);
  clearRef(L, scrollToFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetBox::build(lua_State *L)
{
  window =
      new Window(lvglManager->getCurrentParent(), {x, y, w, h}, lv_obj_create);
  window->setWindowFlag(NO_FORCED_SCROLL);
  window->setScrollHandler([=](coord_t x, coord_t y) { pcallFuncWith2Int(L, scrolledFunction, 0, x, y); });
  lv_obj_add_flag(window->getLvObj(), LV_OBJ_FLAG_EVENT_BUBBLE);
  if (luaScriptManager->isWidget() && !luaScriptManager->isFullscreen()) {
    window->setWindowFlag(NO_CLICK);
  } else {
    lv_obj_set_scroll_dir(window->getLvObj(), scrollDir);
    if (showScrollBar)
      etx_scrollbar(window->getLvObj());
  }
  if (setFlex()) {
    lv_flex_align_t align1 = (align.flags & RIGHT) ? LV_FLEX_ALIGN_END : (align.flags & CENTERED) ? LV_FLEX_ALIGN_CENTER : LV_FLEX_ALIGN_START;
    lv_flex_align_t align2 = (align.flags & VCENTERED) ? LV_FLEX_ALIGN_CENTER : (align.flags & VBOTTOM) ? LV_FLEX_ALIGN_END : LV_FLEX_ALIGN_START;
    if (flexFlow & LV_FLEX_FLOW_COLUMN)
      lv_obj_set_flex_align(window->getLvObj(), align2, align2, align1);
    else
      lv_obj_set_flex_align(window->getLvObj(), align1, align1, align2);
  }
  setColor(color.flags);
  setOpacity(opacity.value);
}

//-----------------------------------------------------------------------------

void LvglWidgetSetting::parseParam(lua_State *L, const char *key)
{
  if (parseTitleParam(L, key)) return;
  LvglWidgetObject::parseParam(L, key);
}

void LvglWidgetSetting::clearRefs(lua_State *L)
{
  clearTitleRefs(L);
  LvglWidgetObjectBase::clearRefs(L);
}

void LvglWidgetSetting::build(lua_State *L)
{
  window =
      new Window(lvglManager->getCurrentParent(), {x, y, w, h}, lv_obj_create);
  window->padAll(PAD_OUTLINE);
  auto lbl = etx_label_create(window->getLvObj());
  lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
  etx_txt_color(lbl, COLOR_THEME_PRIMARY1_INDEX);
  lv_label_set_text(lbl, title.txt.c_str());
}

//-----------------------------------------------------------------------------

void LvglWidgetBorderedObject::parseParam(lua_State *L, const char *key)
{
  if (parseThicknessParam(L, key)) return;
  if (!strcmp(key, "filled")) {
    filled.parse(L);
  } else {
    LvglWidgetBox::parseParam(L, key);
  }
}

bool LvglWidgetBorderedObject::callRefs(lua_State *L)
{
  if (!LvglWidgetBox::callRefs(L)) return false;

  if (isVisible()) {
    if (!pcallUpdateBool(L, filled.function, [=](bool val) { setFilled(val); }))
      return false;
  }

  return true;
}

void LvglWidgetBorderedObject::clearRefs(lua_State *L)
{
  filled.clearRef(L);
  LvglWidgetBox::clearRefs(L);
}

void LvglWidgetBorderedObject::setFilled(int newFilled)
{
  if (filled.changedValue(newFilled)) {
    color.forceUpdate();
    opacity.forceUpdate();
    setColor(color.value);
    setOpacity(opacity.value);
  }
}

void LvglWidgetBorderedObject::setColor(LcdFlags newColor)
{
  if (color.changedColor(newColor)) {
    if (filled.value) {
      etx_bg_color_from_flags(window->getLvObj(), color.flags);
    } else {
      if (color.flags & RGB_FLAG) {
        etx_remove_border_color(window->getLvObj());
        lv_obj_set_style_border_color(window->getLvObj(),
                                      makeLvColor(color.flags), LV_PART_MAIN);
      } else {
        lv_obj_remove_local_style_prop(window->getLvObj(), LV_STYLE_BORDER_COLOR, LV_PART_MAIN);
        etx_border_color(window->getLvObj(), (LcdColorIndex)COLOR_VAL(color.flags));
      }
    }
  }
}

void LvglWidgetBorderedObject::setOpacity(uint8_t newOpa)
{
  if (opacity.changedValue(newOpa)) {
    if (filled.value) {
      lv_obj_set_style_bg_opa(window->getLvObj(), opacity.value, LV_PART_MAIN);
    } else {
      lv_obj_set_style_bg_opa(window->getLvObj(), 0, LV_PART_MAIN);
      lv_obj_set_style_border_opa(window->getLvObj(), opacity.value, LV_PART_MAIN);
      lv_obj_set_style_border_width(window->getLvObj(), thickness, LV_PART_MAIN);
    }
  }
}

void LvglWidgetBorderedObject::refresh()
{
  setFilled(filled.value);
  LvglWidgetBox::refresh();
}

//-----------------------------------------------------------------------------

void LvglWidgetRoundObject::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "radius")) {
    radius.parse(L);
  } else {
    LvglWidgetBorderedObject::parseParam(L, key);
  }
}

bool LvglWidgetRoundObject::callRefs(lua_State *L)
{
  if (!LvglWidgetBorderedObject::callRefs(L)) return false;

  if (isVisible()) {
    if (!pcallUpdate1Int(L, radius.function, [=](int val) { setRadius(val); }))
      return false;
  }

  return true;
}

void LvglWidgetRoundObject::clearRefs(lua_State *L)
{
  radius.clearRef(L);
  LvglWidgetBorderedObject::clearRefs(L);
}

void LvglWidgetRoundObject::setPos(coord_t x, coord_t y)
{
  LvglWidgetObject::setPos(x - radius.coord, y - radius.coord);
}

void LvglWidgetRoundObject::setRadius(coord_t r)
{
  // Set position to center
  x += radius.coord;
  y += radius.coord;
  radius.coord = r;
  w = radius.coord * 2;
  h = radius.coord * 2;
  setPos(x, y);
  setSize(w, h);
}

void LvglWidgetRoundObject::refresh()
{
  setRadius(radius.coord);
  LvglWidgetBorderedObject::refresh();
}

//-----------------------------------------------------------------------------

void LvglWidgetRectangle::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "rounded")) {
    rounded = luaL_checkunsigned(L, -1);
  } else {
    LvglWidgetBorderedObject::parseParam(L, key);
  }
}

void LvglWidgetRectangle::build(lua_State *L)
{
  LvglWidgetBorderedObject::build(L);
  if (rounded) {
    lv_obj_set_style_radius(window->getLvObj(),
                            (rounded >= thickness) ? rounded : thickness,
                            LV_PART_MAIN);
  }
}

//-----------------------------------------------------------------------------

void LvglWidgetCircle::build(lua_State *L)
{
  // Reset position to corner
  setPos(x, y);
  // Set width & height
  setRadius(radius.coord);
  LvglWidgetRoundObject::build(L);
  lv_obj_set_style_radius(window->getLvObj(), LV_RADIUS_CIRCLE, LV_PART_MAIN);
}

//-----------------------------------------------------------------------------

void LvglWidgetArc::parseParam(lua_State *L, const char *key)
{
  if (parseRoundedParam(L, key)) return;
  if (!strcmp(key, "startAngle")) {
    startAngle.parse(L);
  } else if (!strcmp(key, "endAngle")) {
    endAngle.parse(L);
  } else if (!strcmp(key, "bgColor")) {
    bgColor.parse(L);
  } else if (!strcmp(key, "bgOpacity")) {
    bgOpacity.parse(L);
  } else if (!strcmp(key, "bgStartAngle")) {
    bgStartAngle.parse(L);
  } else if (!strcmp(key, "bgEndAngle")) {
    bgEndAngle.parse(L);
  } else {
    LvglWidgetRoundObject::parseParam(L, key);
  }
}

void LvglWidgetArc::setColor(LcdFlags newColor)
{
  if (color.changedColor(newColor)) {
    etx_arc_color_from_flags(window->getLvObj(), color.flags, LV_PART_INDICATOR);
  }
}

void LvglWidgetArc::setBgColor(LcdFlags newColor)
{
  if (bgColor.changedColor(newColor)) {
    etx_arc_color_from_flags(window->getLvObj(), bgColor.flags, LV_PART_MAIN);
  }
}

void LvglWidgetArc::setOpacity(uint8_t newOpa)
{
  if (opacity.changedValue(newOpa)) {
    lv_obj_set_style_arc_opa(window->getLvObj(), opacity.value, LV_PART_INDICATOR);
  }
}

void LvglWidgetArc::setBgOpacity(uint8_t newOpa)
{
  if (bgOpacity.changedValue(newOpa)) {
    lv_obj_set_style_arc_opa(window->getLvObj(), bgOpacity.value, LV_PART_MAIN);
  }
}

void LvglWidgetArc::setStartAngle(coord_t angle)
{
  if (startAngle.changedValue(angle)) {
    lv_arc_set_start_angle(window->getLvObj(), startAngle.coord);
  }
}

void LvglWidgetArc::setBgStartAngle(coord_t angle)
{
  if (bgStartAngle.changedValue(angle)) {
    lv_arc_set_bg_start_angle(window->getLvObj(), bgStartAngle.coord);
  }
}

void LvglWidgetArc::setEndAngle(coord_t angle)
{
  if (endAngle.changedValue(angle)) {
    lv_arc_set_end_angle(window->getLvObj(), endAngle.coord);
  }
}

void LvglWidgetArc::setBgEndAngle(coord_t angle)
{
  if (bgEndAngle.changedValue(angle)) {
    lv_arc_set_bg_end_angle(window->getLvObj(), bgEndAngle.coord);
  }
}

bool LvglWidgetArc::callRefs(lua_State *L)
{
  if (!LvglWidgetRoundObject::callRefs(L)) return false;

  if (isVisible()) {
    if (!pcallUpdate1Int(L, startAngle.function,
                        [=](int val) { setStartAngle(val); }))
      return false;
    if (!pcallUpdate1Int(L, endAngle.function,
                        [=](int val) { setEndAngle(val); }))
      return false;
    if (!pcallUpdate1Int(L, bgStartAngle.function,
                        [=](int val) { setBgStartAngle(val); }))
      return false;
    if (!pcallUpdate1Int(L, bgEndAngle.function,
                        [=](int val) { setBgEndAngle(val); }))
      return false;
  }

  return true;
}

void LvglWidgetArc::clearRefs(lua_State *L)
{
  startAngle.clearRef(L);
  endAngle.clearRef(L);
  bgColor.clearRef(L);
  bgOpacity.clearRef(L);
  bgStartAngle.clearRef(L);
  bgEndAngle.clearRef(L);
  LvglWidgetRoundObject::clearRefs(L);
}

void LvglWidgetArc::build(lua_State *L)
{
  // Reset position to corner
  setPos(x, y);
  // Set width & height
  setRadius(radius.coord);
  window =
      new Window(lvglManager->getCurrentParent(), {x, y, w, h}, lv_arc_create);
  window->setWindowFlag(NO_CLICK);
  lv_obj_add_flag(window->getLvObj(), LV_OBJ_FLAG_EVENT_BUBBLE);
  lv_arc_set_range(window->getLvObj(), 0, 360);
  lv_obj_remove_style(window->getLvObj(), NULL, LV_PART_KNOB);
  lv_obj_set_style_arc_width(window->getLvObj(), thickness, LV_PART_MAIN);
  lv_obj_set_style_arc_rounded(window->getLvObj(), rounded, LV_PART_MAIN);
  lv_obj_set_style_arc_width(window->getLvObj(), thickness, LV_PART_INDICATOR);
  lv_obj_set_style_arc_rounded(window->getLvObj(), rounded, LV_PART_INDICATOR);
  setBgStartAngle(bgStartAngle.coord);
  setBgEndAngle(bgEndAngle.coord);
  setStartAngle(startAngle.coord);
  setEndAngle(endAngle.coord);
  setColor(color.flags);
  setBgColor(bgColor.flags);
  setOpacity(opacity.value);
  setBgOpacity(bgOpacity.value);
}

//-----------------------------------------------------------------------------

void LvglWidgetImage::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "file")) {
    filename.parse(L);
  } else if (!strcmp(key, "fill")) {
    fillFrame = getLuaBool(L);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

bool LvglWidgetImage::callRefs(lua_State *L)
{
  if (!LvglWidgetObject::callRefs(L)) return false;

  if (isVisible()) {
    int t = lua_gettop(L);
    if (filename.function != LUA_REFNIL) {
      if (pcallFunc(L, filename.function, 1)) {
        const char *s = luaL_checkstring(L, -1);
        if (filename.changedText(s)) {
          ((StaticImage*)window)->setSource(filename.chars());
        }
        lua_settop(L, t);
      } else {
        return false;
      }
    }
  }

  return true;
}

void LvglWidgetImage::clearRefs(lua_State *L)
{
  filename.clearRef(L);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetImage::build(lua_State *L)
{
  window = new StaticImage(lvglManager->getCurrentParent(), {x, y, w, h},
                           filename.chars(), fillFrame);
}

//-----------------------------------------------------------------------------

void LvglWidgetQRCode::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "data")) {
    data = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "bgColor")) {
    bgColor = luaL_checkunsigned(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetQRCode::build(lua_State *L)
{
  window = new QRCode(lvglManager->getCurrentParent(), x, y, w, data, colorToRGB(color.flags), colorToRGB(bgColor));
}

//-----------------------------------------------------------------------------

void LvglWidgetTextButtonBase::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "cornerRadius")) {
    rounded = luaL_checkunsigned(L, -1);
  } else if (!strcmp(key, "textColor")) {
    textColor.parse(L);
  } else if (!strcmp(key, "press")) {
    pressFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!parseTextParam(L, key)) {
    LvglWidgetObject::parseParam(L, key);
  }
}

bool LvglWidgetTextButtonBase::callRefs(lua_State *L)
{
  if (!LvglWidgetObject::callRefs(L)) return false;

  if (isVisible()) {
    if (!pcallUpdateStringVal(L, txt.function, [=](const char* s) { setText(s); }))
      return false;
    if (!pcallUpdate1Int(L, textColor.function, [=](int color) { setTextColor(color); }))
      return false;
  }

  return true;
}

void LvglWidgetTextButtonBase::clearRefs(lua_State *L)
{
  textColor.clearRef(L);
  clearRef(L, pressFunction);
  clearTextRefs(L);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetTextButtonBase::setSize(coord_t w, coord_t h)
{
  if (w == LV_SIZE_CONTENT || w == 0) w = lv_obj_get_width(window->getLvObj());
  if (h == LV_SIZE_CONTENT || h == 0) h = lv_obj_get_height(window->getLvObj());
  LvglWidgetObject::setSize(w, h);
}

void LvglWidgetTextButtonBase::setColor(LcdFlags newColor)
{
  if (window && color.changedColor(newColor)) {
    etx_bg_color_from_flags(window->getLvObj(), color.flags);
  }
}

void LvglWidgetTextButtonBase::setTextColor(LcdFlags newColor)
{
  if (window && textColor.changedColor(newColor)) {
    etx_txt_color_from_flags(window->getLvObj(), textColor.flags);
  }
}

void LvglWidgetTextButtonBase::setRounded()
{
  if (window && rounded >= 0) {
    lv_obj_remove_style(window->getLvObj(), (lv_style_t*)&styles->rounded, LV_PART_MAIN);
    if (rounded > 0)
      lv_obj_set_style_radius(window->getLvObj(), rounded, LV_PART_MAIN);
  }
}

//-----------------------------------------------------------------------------

void LvglWidgetTextButton::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "checked")) {
    checked = getLuaBool(L);
  } else if (!strcmp(key, "longpress")) {
    longPressFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetTextButtonBase::parseParam(L, key);
  }
}

void LvglWidgetTextButton::clearRefs(lua_State *L)
{
  clearRef(L, longPressFunction);
  LvglWidgetTextButtonBase::clearRefs(L);
}

void LvglWidgetTextButton::setChecked(bool checked)
{
  this->checked = checked;
  ((TextButton*)window)->check(checked);
}

void LvglWidgetTextButton::setText(const char *s)
{
  if (window && txt.changedText(s)) {
    ((TextButton *)window)->setText(s);
  }
}

void LvglWidgetTextButton::setFont(LcdFlags newFont)
{
  if (window && font.changedFont(newFont))
    ((TextButton*)window)->setFont(FONT_INDEX(font.flags));
}

void LvglWidgetTextButton::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  auto btn =
      new TextButton(lvglManager->getCurrentParent(), {x, y, w, h}, txt.chars(), [=]() {
        return pcallGetOptIntVal(L, pressFunction, 0);
      });
  if (longPressFunction != LUA_REFNIL) {
    btn->setLongPressHandler([=]() {
      return pcallGetOptIntVal(L, longPressFunction, 0);
    });
  }
  window = btn;
  setFont(font.flags);
  setChecked(checked);
  setColor(color.flags);
  setTextColor(textColor.flags);
  setRounded();
}

//-----------------------------------------------------------------------------

void LvglWidgetMomentaryButton::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "release")) {
    releaseFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetTextButtonBase::parseParam(L, key);
  }
}

void LvglWidgetMomentaryButton::clearRefs(lua_State *L)
{
  clearRef(L, releaseFunction);
  LvglWidgetTextButtonBase::clearRefs(L);
}

void LvglWidgetMomentaryButton::setText(const char *s)
{
  if (window && txt.changedText(s)) {
    ((MomentaryButton *)window)->setText(s);
  }
}

void LvglWidgetMomentaryButton::setFont(LcdFlags newFont)
{
  if (window && font.changedFont(newFont))
    ((MomentaryButton*)window)->setFont(FONT_INDEX(font.flags));
}

void LvglWidgetMomentaryButton::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  window =
      new MomentaryButton(lvglManager->getCurrentParent(), {x, y, w, h}, txt.chars(),
          [=]() {
            pcallSimpleFunc(L, pressFunction);
          },
          [=]() {
            pcallSimpleFunc(L, releaseFunction);
          });
  setFont(font.flags);
  setColor(color.flags);
  setTextColor(textColor.flags);
  setRounded();
}

//-----------------------------------------------------------------------------

void LvglWidgetToggleSwitch::parseParam(lua_State *L, const char *key)
{
  if (!parseGetSetParam(L, key)) {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetToggleSwitch::clearRefs(lua_State *L)
{
  clearGetSetRefs(L);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetToggleSwitch::build(lua_State *L)
{
  window = new ToggleSwitch(
      lvglManager->getCurrentParent(), {x, y, 0, 0},
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](uint8_t val) { pcallSetIntVal(L, setFunction, val); });
}

//-----------------------------------------------------------------------------

void LvglWidgetTextEdit::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "value")) {
    txt = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "length")) {
    maxLen = luaL_checkinteger(L, -1);
    if (maxLen > 128) maxLen = 128;
    if (maxLen <= 0) maxLen = 32;
  } else if (!strcmp(key, "set")) {
    setFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetTextEdit::clearRefs(lua_State *L)
{
  clearRef(L, setFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetTextEdit::build(lua_State *L)
{
  strcpy(value, txt);
  if (h == LV_SIZE_CONTENT) h = 0;
  window = new TextEdit(lvglManager->getCurrentParent(), {x, y, w, h}, value,
                        maxLen, [=]() {
                          if (setFunction != LUA_REFNIL) {
                            int t = lua_gettop(L);
                            PROTECT_LUA()
                            {
                              std::string s(value, maxLen); // Ensure string is terminated
                              if (!pcallFuncWithString(L, setFunction, 0, s.c_str())) {
                                lvglManager->luaShowError();
                              }
                            }
                            else
                            {
                              lvglManager->luaShowError();
                            }
                            UNPROTECT_LUA();
                            lua_settop(L, t);
                          }
                        });
}

//-----------------------------------------------------------------------------

void LvglWidgetNumberEdit::parseParam(lua_State *L, const char *key)
{
  if (parseMinMaxParam(L, key)) return;
  if (parseGetSetParam(L, key)) return;

  if (!strcmp(key, "display")) {
    dispFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "edited")) {
    editedFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetNumberEdit::clearRefs(lua_State *L)
{
  clearRef(L, dispFunction);
  clearRef(L, editedFunction);
  clearGetSetRefs(L);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetNumberEdit::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  window = new NumberEdit(
      lvglManager->getCurrentParent(), {x, y, w, h}, min, max,
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](int val) { pcallSetIntVal(L, setFunction, val); });
  if (dispFunction != LUA_REFNIL) {
    ((NumberEdit *)window)->setDisplayHandler([=](int val) {
      const char *s = "???";
      int t = lua_gettop(L);
      PROTECT_LUA()
      {
        if (pcallFuncWithInt(L, dispFunction, 1, val)) {
          s = luaL_checkstring(L, -1);
        } else {
          lvglManager->luaShowError();
        }
      }
      else
      {
        lvglManager->luaShowError();
      }
      UNPROTECT_LUA();
      lua_settop(L, t);
      return s;
    });
  }
  if (editedFunction != LUA_REFNIL) {
    ((NumberEdit*)window)->setOnEditedHandler([=]( int val) {
      pcallSetIntVal(L, editedFunction, val);
    });
  }
}

//-----------------------------------------------------------------------------

void LvglWidgetSliderBase::parseParam(lua_State *L, const char *key)
{
  if (parseMinMaxParam(L, key)) return;
  if (parseGetSetParam(L, key)) return;
  LvglWidgetObject::parseParam(L, key);
}

void LvglWidgetSliderBase::setColor(LcdFlags newColor)
{
  if (color.changedColor(newColor)) {
    ((SliderBase*)window)->setColor(color.flags);
  }
}

void LvglWidgetSliderBase::clearRefs(lua_State *L)
{
  clearGetSetRefs(L);
  LvglWidgetObject::clearRefs(L);
}

//-----------------------------------------------------------------------------

void LvglWidgetSlider::build(lua_State *L)
{
  window = new Slider(
      lvglManager->getCurrentParent(), w, min, max,
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](int val) { pcallSetIntVal(L, setFunction, val); });
  window->setPos(x, y);
  setColor(color.flags);
}

//-----------------------------------------------------------------------------

void LvglWidgetVerticalSlider::build(lua_State *L)
{
  window = new VerticalSlider(
      lvglManager->getCurrentParent(), h, min, max,
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](int val) { pcallSetIntVal(L, setFunction, val); });
  window->setPos(x, y);
  setColor(color.flags);
}

//-----------------------------------------------------------------------------

class WidgetPage : public NavWindow, public LuaEventHandler
{
 public:
  WidgetPage(Window *parent, 
             std::function<void()> backAction,
             std::function<void()> menuAction,
             std::function<void()> prevAction,
             std::function<void()> nextAction,
             std::function<bool()> prevActive,
             std::function<bool()> nextActive,
             std::string title, std::string subtitle, std::string iconFile,
             lv_dir_t scrollDir, bool showScrollBar,
             bool showBackBtn, bool showPrevBtn, bool showNextBtn) :
      NavWindow(parent, {0, 0, LCD_W, LCD_H}),
      backAction(std::move(backAction)), menuAction(menuAction),
      prevAction(std::move(prevAction)), nextAction(std::move(nextAction)),
      prevActive(std::move(prevActive)), nextActive(std::move(nextActive)),
      showPrev(showPrevBtn), showNext(showNextBtn)
  {
    if (iconFile.empty())
      header = new PageHeader(this, ICON_EDGETX);
    else
      header = new PageHeader(this, iconFile.c_str());

#if defined(HARDWARE_TOUCH)
    if (showBackBtn) {
      new HeaderBackIcon(header);
      addCustomButton(0, 0, this->menuAction);
      addCustomButton(LCD_W - EdgeTxStyles::MENU_HEADER_HEIGHT, 0, this->backAction);
    } else {
      addCustomButton(0, 0, this->backAction);
    }
#endif

    body = new Window(
        this, {0, EdgeTxStyles::MENU_HEADER_HEIGHT, LCD_W, LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT});
    body->setWindowFlag(NO_FOCUS);

    header->setTitle(title);
    header->setTitle2(subtitle);

    etx_solid_bg(lvobj);
    lv_obj_set_style_max_height(body->getLvObj(), LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT,
                                LV_PART_MAIN);
    lv_obj_set_scroll_dir(body->getLvObj(), scrollDir);
    if (showScrollBar)
      etx_scrollbar(body->getLvObj());

    delayLoad();
  }

  void delayedInit() override
  {
#if defined(HARDWARE_TOUCH)
    if (showPrev) {
      prevBtn = new IconButton(this, ICON_BTN_PREV, LCD_W - PageGroup::PAGE_GROUP_BACK_BTN_XO * 3, PAD_MEDIUM, [=]() {
        prevAction();
        return 0;
      });
    }
    if (showNext) {
      nextBtn = new IconButton(this, ICON_BTN_NEXT, LCD_W - PageGroup::PAGE_GROUP_BACK_BTN_XO * 2, PAD_MEDIUM, [=]() {
        nextAction();
        return 0;
      });
    }
#endif
  }

  Window *getBody() { return body; }

  void setTitle(std::string s) { header->setTitle(s); }
  void setSubTitle(std::string s) { header->setTitle2(s); }

 protected:
  std::function<void()> backAction;
  std::function<void()> menuAction;
  std::function<void()> prevAction;
  std::function<void()> nextAction;
  std::function<bool()> prevActive;
  std::function<bool()> nextActive;
  bool showPrev = false;
  bool showNext = false;
  PageHeader *header = nullptr;
  Window *body = nullptr;
  IconButton* prevBtn = nullptr;
  IconButton* nextBtn = nullptr;

  void onClicked() override { Keyboard::hide(false); LuaEventHandler::onClickedEvent(); }

  void onCancel() override { backAction(); }

  void onEvent(event_t evt) override
  {
    LuaEventHandler::onLuaEvent(evt);
    parent->onEvent(evt);
  }

  void checkEvents() override
  {
    if (prevBtn) prevBtn->enable(prevActive());
    if (nextBtn) nextBtn->enable(nextActive());
    NavWindow::checkEvents();
  }
};

void LvglWidgetPage::parseParam(lua_State *L, const char *key)
{
  if (parseAlignParam(L, key)) return;
  if (parseTitleParam(L, key)) return;
  if (parseScrollableParam(L, key)) return;
  if (!strcmp(key, "back")) {
    backActionFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "menu")) {
    menuActionFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "prevButton")) {
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      const char *key = lua_tostring(L, -2);
      if (!strcmp(key, "press")) {
        prevActionFunction = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
      } else if (!strcmp(key, "active")) {
        prevActiveFunction = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
      }
    }
  } else if (!strcmp(key, "nextButton")) {
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      const char *key = lua_tostring(L, -2);
      if (!strcmp(key, "press")) {
        nextActionFunction = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
      } else if (!strcmp(key, "active")) {
        nextActiveFunction = luaL_ref(L, LUA_REGISTRYINDEX);
        lua_pushnil(L);
      }
    }
  } else if (!strcmp(key, "backButton")) {
    showBackButton = getLuaBool(L);
  } else if (!strcmp(key, "subtitle")) {
    subtitle.parse(L);
  } else if (!strcmp(key, "icon")) {
    iconFile = luaL_checkstring(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

coord_t LvglWidgetPage::getScrollX()
{
  return lv_obj_get_scroll_x(window->getLvObj());
}

coord_t LvglWidgetPage::getScrollY()
{
  return lv_obj_get_scroll_y(window->getLvObj());
}

void LvglWidgetPage::setTitle(const char* s)
{
  if (page && title.changedText(s))
    page->setTitle(title.txt);
}

void LvglWidgetPage::setSubTitle(const char* s)
{
  if (page && subtitle.changedText(s))
    page->setSubTitle(subtitle.txt);
}

bool LvglWidgetPage::callRefs(lua_State *L)
{
  if (!LvglWidgetObject::callRefs(L)) return false;

  if (isVisible()) {
    if (!pcallUpdateStringVal(L, title.function, [=](const char* s) { setTitle(s); }))
      return false;
    if (!pcallUpdateStringVal(L, subtitle.function, [=](const char* s) { setSubTitle(s); }))
      return false;
    if (!pcallUpdate2Int(L, scrollToFunction,
        [=](int x, int y) { if (x != getScrollX() || y != getScrollY()) lv_obj_scroll_to(window->getLvObj(), x, y, LV_ANIM_OFF); })) {
      return false;
    }
  }

  return true;
}

void LvglWidgetPage::clearRefs(lua_State *L)
{
  clearAlignRefs(L);
  clearTitleRefs(L);
  subtitle.clearRef(L);
  clearRef(L, backActionFunction);
  clearRef(L, menuActionFunction);
  clearRef(L, prevActionFunction);
  clearRef(L, nextActionFunction);
  clearRef(L, prevActiveFunction);
  clearRef(L, nextActiveFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetPage::build(lua_State *L)
{
  w = LCD_W;
  h = LCD_H;
  page = new WidgetPage(
      lvglManager->getCurrentParent(),
      [=]() { pcallSimpleFunc(L, backActionFunction); },
      [=]() { pcallSimpleFunc(L, menuActionFunction); },
      [=]() { pcallSimpleFunc(L, prevActionFunction); },
      [=]() { pcallSimpleFunc(L, nextActionFunction); },
      [=]() { return pcallGetOptIntVal(L, prevActiveFunction, true); },
      [=]() { return pcallGetOptIntVal(L, nextActiveFunction, true); },
      title.txt, subtitle.txt, iconFile, scrollDir, showScrollBar,
      showBackButton, prevActionFunction != LUA_REFNIL, nextActionFunction != LUA_REFNIL);

  window = page->getBody();
  window->setScrollHandler([=](coord_t x, coord_t y) { pcallFuncWith2Int(L, scrolledFunction, 0, x, y); });
  if (setFlex()) {
    lv_flex_align_t align1 = (align.flags & RIGHT) ? LV_FLEX_ALIGN_END : (align.flags & CENTERED) ? LV_FLEX_ALIGN_CENTER : LV_FLEX_ALIGN_START;
    lv_flex_align_t align2 = (align.flags & VCENTERED) ? LV_FLEX_ALIGN_CENTER : (align.flags & VBOTTOM) ? LV_FLEX_ALIGN_END : LV_FLEX_ALIGN_START;
    if (flexFlow & LV_FLEX_FLOW_COLUMN)
      lv_obj_set_flex_align(window->getLvObj(), align2, align2, align1);
    else
      lv_obj_set_flex_align(window->getLvObj(), align1, align1, align2);
  }
}

//-----------------------------------------------------------------------------

class LvglDialog : public BaseDialog
{
 public:
  LvglDialog(std::string title, coord_t w, coord_t h, std::function<void()> onClose) :
    BaseDialog(title.c_str(), true, w, h, false),
    onClose(std::move(onClose))
  {
    form->setHeight(h - EdgeTxStyles::UI_ELEMENT_HEIGHT);
  }

  Window *getBody() { return form; }

  void onCancel() override
  {
    onClose();
    BaseDialog::onCancel();
  }

 protected:
  std::function<void()> onClose;
};

void LvglWidgetDialog::parseParam(lua_State *L, const char *key)
{
  if (parseTitleParam(L, key)) return;
  if (!strcmp(key, "close")) {
    closeFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetDialog::clearRefs(lua_State *L)
{
  clearTitleRefs(L);
  clearRef(L, closeFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetDialog::build(lua_State *L)
{
  if (w == LV_SIZE_CONTENT) w = DIALOG_DEFAULT_WIDTH;
  if (h == LV_SIZE_CONTENT) h = DIALOG_DEFAULT_HEIGHT;
  auto dlg = new LvglDialog(title.txt, w, h,
      [=]() { pcallSimpleFunc(L, closeFunction); });
  dialog = dlg;
  window = dlg->getBody();
  window->setWidth(w);
  setFlex();
}

void LvglWidgetDialog::close()
{
  dialog->onCancel();
  clear();
}

//-----------------------------------------------------------------------------

void LvglWidgetConfirmDialog::parseParam(lua_State *L, const char *key)
{
  if (parseTitleParam(L, key)) return;
  if (parseMessageParam(L, key)) return;
  if (!strcmp(key, "confirm")) {
    confirmFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "cancel")) {
    cancelFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetConfirmDialog::clearRefs(lua_State *L)
{
  clearTitleRefs(L);
  clearRef(L, confirmFunction);
  clearRef(L, cancelFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetConfirmDialog::build(lua_State *L)
{
  window = new ConfirmDialog(title.txt.c_str(), message.c_str(),
      [=]() { pcallSimpleFunc(L, confirmFunction); },
      [=]() { pcallSimpleFunc(L, cancelFunction); });
}

//-----------------------------------------------------------------------------

void LvglWidgetMessageDialog::parseParam(lua_State *L, const char *key)
{
  if (parseTitleParam(L, key)) return;
  if (parseMessageParam(L, key)) return;
  if (!strcmp(key, "details")) {
    details = luaL_checkstring(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetMessageDialog::clearRefs(lua_State *L)
{
  clearTitleRefs(L);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetMessageDialog::build(lua_State *L)
{
  window = new MessageDialog(title.txt.c_str(), message.c_str(), details.c_str());
}

//-----------------------------------------------------------------------------

void LvglWidgetPicker::parseParam(lua_State *L, const char *key)
{
  if (!parseGetSetParam(L, key)) {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetPicker::clearRefs(lua_State *L)
{
  clearGetSetRefs(L);
  LvglWidgetObject::clearRefs(L);
}

//-----------------------------------------------------------------------------

void LvglWidgetChoice::parseParam(lua_State *L, const char *key)
{
  if (parseTitleParam(L, key)) return;
  if (parseValuesParam(L, key)) return;
  if (!strcmp(key, "filter")) {
    filterFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "popupWidth")) {
    popupWidth = luaL_checkinteger(L, -1);
  } else {
    LvglWidgetPicker::parseParam(L, key);
  }
}

void LvglWidgetChoice::clearRefs(lua_State *L)
{
  clearTitleRefs(L);
  clearRef(L, filterFunction);
  LvglWidgetPicker::clearRefs(L);
}

void LvglWidgetChoice::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  auto c = new Choice(
      lvglManager->getCurrentParent(), {x, y, w, h}, values, 0, values.size() - 1,
      [=]() { return pcallGetIntVal(L, getFunction) - 1; },
      [=](int val) { pcallSetIntVal(L, setFunction, val + 1); }, title.txt.c_str());

  c->setPopupWidth(popupWidth);

  if (filterFunction != LUA_REFNIL)
    c->setAvailableHandler([=](int n) {
      bool rv = true;
      int t = lua_gettop(L);
      PROTECT_LUA()
      {
        if (pcallFuncWithInt(L, filterFunction, 1, n + 1)) {
          rv = getLuaBool(L);
        } else {
          lvglManager->luaShowError();
        }
      }
      else
      {
        lvglManager->luaShowError();
      }
      UNPROTECT_LUA();
      lua_settop(L, t);
      return rv;
    });

  window = c;
}

//-----------------------------------------------------------------------------

void LvglWidgetMenu::parseParam(lua_State *L, const char *key)
{
  if (parseTitleParam(L, key)) return;
  if (parseValuesParam(L, key)) return;
  LvglWidgetPicker::parseParam(L, key);
}

void LvglWidgetMenu::clearRefs(lua_State *L)
{
  clearTitleRefs(L);
  LvglWidgetPicker::clearRefs(L);
}

void LvglWidgetMenu::build(lua_State *L)
{
  auto menu = new Menu();
  if (!title.txt.empty()) menu->setTitle(title.txt);

  for (size_t i = 0; i < values.size(); i += 1) {
    menu->addLine(values[i], [=]() {
      pcallSetIntVal(L, setFunction, i + 1);
    });
  }

  int selected = pcallGetIntVal(L, getFunction) - 1;
  if (selected >= 0) {
    menu->select(selected);
  }

  window = menu;
}

//-----------------------------------------------------------------------------

void LvglWidgetFontPicker::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  window = new Choice(
      lvglManager->getCurrentParent(), {x, y, w, h}, STR_FONT_SIZES, 0, FONTS_COUNT - 1,
      [=]() { return FONT_INDEX(pcallGetIntVal(L, getFunction)); },
      [=](int val) { pcallSetIntVal(L, setFunction, val << 8u); });
}

//-----------------------------------------------------------------------------

void LvglWidgetAlignPicker::build(lua_State *L)
{
  static LcdFlags alignments[3] = { LEFT, CENTERED, RIGHT };

  if (h == LV_SIZE_CONTENT) h = 0;
  window = new Choice(
      lvglManager->getCurrentParent(), {x, y, w, h}, STR_ALIGN_OPTS, 0, ALIGN_COUNT - 1,
      [=]() -> int {
        auto v = (LcdFlags)pcallGetIntVal(L, getFunction);
        for (size_t i = 0; i < DIM(alignments); i += 1)
          if (alignments[i] == v) return i;
        return 0;
      },
      [=](int val) { pcallSetIntVal(L, setFunction, alignments[val]); });
}

//-----------------------------------------------------------------------------

void LvglWidgetColorPicker::build(lua_State *L)
{
  if (w == LV_SIZE_CONTENT) w = 0;
  if (h == LV_SIZE_CONTENT) h = 0;
  window = new ColorPicker(
      lvglManager->getCurrentParent(), {x, y, w, h},
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](uint32_t val) { pcallSetIntVal(L, setFunction, val); });
}

//-----------------------------------------------------------------------------

void LvglWidgetTimerPicker::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  auto tmChoice = new Choice(
      lvglManager->getCurrentParent(), {x, y, w, h}, 0, TIMERS - 1,
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](uint32_t val) { pcallSetIntVal(L, setFunction, val); });
  tmChoice->setTextHandler([](int value) {
    return std::string(STR_TIMER) + std::to_string(value + 1);
  });
  window = tmChoice;
}

//-----------------------------------------------------------------------------

void LvglWidgetSwitchPicker::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "filter")) {
    filter = luaL_checkunsigned(L, -1);
  } else {
    LvglWidgetPicker::parseParam(L, key);
  }
}

void LvglWidgetSwitchPicker::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  auto c = new SwitchChoice(
      lvglManager->getCurrentParent(), {x, y, w, h},
      SWSRC_FIRST, SWSRC_LAST,
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](uint32_t val) { pcallSetIntVal(L, setFunction, val); });
  c->setAvailableHandler([=](int value) {
    return checkSwitchAvailable(value, filter);
  });
  window = c;
}

//-----------------------------------------------------------------------------

void LvglWidgetSourcePicker::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "filter")) {
    filter = luaL_checkunsigned(L, -1);
  } else {
    LvglWidgetPicker::parseParam(L, key);
  }
}

void LvglWidgetSourcePicker::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  auto c = new SourceChoice(
      lvglManager->getCurrentParent(), {x, y, w, h},
      0, MIXSRC_LAST_TELEM,
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](uint32_t val) { pcallSetIntVal(L, setFunction, val); },
      filter & SRC_INVERT);
  c->setAvailableHandler([=](int value) {
    return checkSourceAvailable(value, filter);
  });
  window = c;
}

//-----------------------------------------------------------------------------

void LvglWidgetFilePicker::parseParam(lua_State *L, const char *key)
{
  if (parseTitleParam(L, key)) return;
  if (!strcmp(key, "folder")) {
    folder = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "extension")) {
    extension = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "maxLen")) {
    maxLen = luaL_checkunsigned(L, -1);
  } else if (!strcmp(key, "hideExtension")) {
    hideExtension = getLuaBool(L);
  } else {
    LvglWidgetPicker::parseParam(L, key);
  }
}

void LvglWidgetFilePicker::clearRefs(lua_State *L)
{
  clearTitleRefs(L);
  LvglWidgetPicker::clearRefs(L);
}

void LvglWidgetFilePicker::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  auto c = new FileChoice(
      lvglManager->getCurrentParent(), {x, y, w, h},
      folder, extension, maxLen,
      [=]() { return pcallGetStringVal(L, getFunction); },
      [=](std::string val) { pcallSetStringVal(L, setFunction, val.c_str()); },
      hideExtension, title.txt.c_str());
  window = c;
}

//-----------------------------------------------------------------------------
