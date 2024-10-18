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
#include "edgetx.h"
#include "lua_event.h"
#include "page.h"
#include "slider.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "toggleswitch.h"

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

LvglWidgetObjectBase *LvglWidgetObjectBase::checkLvgl(lua_State *L, int index)
{
  LvglWidgetObjectBase **p;
  
  p = (LvglWidgetObjectBase **)luaL_testudata(L, index, LVGL_METATABLE);
  if (p) return *p;

  p = (LvglWidgetObjectBase **)luaL_testudata(L, index, LVGL_SIMPLEMETATABLE);
  if (p) return *p;

  return nullptr;
}

//-----------------------------------------------------------------------------

LvglWidgetObjectBase::LvglWidgetObjectBase(const char* meta) :
    metatable(meta),
    lvglManager(luaLvglManager)
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
    PROTECT_LUA()
    {
      if (!pcallFunc(L, funcRef, 0)) {
        lvglManager->luaShowError();
      }
    }
    UNPROTECT_LUA();
  }
}

bool LvglWidgetObjectBase::pcallUpdateBool(lua_State *L, int getFuncRef,
                                           std::function<void(bool)> update)
{
  if (getFuncRef != LUA_REFNIL) {
    int t = lua_gettop(L);
    if (pcallFunc(L, getFuncRef, 1)) {
      bool val = lua_toboolean(L, -1);
      update(val);
      lua_settop(L, t);
    } else {
      return false;
    }
  }
  return true;
}

bool LvglWidgetObjectBase::pcallUpdate1Int(lua_State *L, int getFuncRef,
                                           std::function<void(int)> update)
{
  if (getFuncRef != LUA_REFNIL) {
    int t = lua_gettop(L);
    if (pcallFunc(L, getFuncRef, 1)) {
      int val = luaL_checkunsigned(L, -1);
      update(val);
      lua_settop(L, t);
    } else {
      return false;
    }
  }
  return true;
}

bool LvglWidgetObjectBase::pcallUpdate2Int(lua_State *L, int getFuncRef,
                                           std::function<void(int, int)> update)
{
  if (getFuncRef != LUA_REFNIL) {
    int t = lua_gettop(L);
    if (pcallFunc(L, getFuncRef, 2)) {
      int v1 = luaL_checkunsigned(L, -2);
      int v2 = luaL_checkunsigned(L, -1);
      update(v1, v2);
      lua_settop(L, t);
    } else {
      return false;
    }
  }
  return true;
}

int LvglWidgetObjectBase::pcallGetIntVal(lua_State *L, int getFuncRef)
{
  int val = 0;
  if (getFuncRef != LUA_REFNIL) {
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (pcallFunc(L, getFuncRef, 1)) {
        val = luaL_checkinteger(L, -1);
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
  }
  return val;
}

void LvglWidgetObjectBase::pcallSetIntVal(lua_State *L, int setFuncRef, int val)
{
  if (setFuncRef != LUA_REFNIL) {
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      if (!pcallFuncWithInt(L, setFuncRef, 0, val)) {
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
    if (lua_isfunction(L, -1)) {
      getColorFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    } else {
      color = luaL_checkunsigned(L, -1);
    }
  } else if (!strcmp(key, "visible")) {
    getVisibleFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "size")) {
    getSizeFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "pos")) {
    getPosFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  }
}

bool LvglWidgetObjectBase::callRefs(lua_State *L)
{
  if (clearRequest) {
    clearRequest = false;
    if (getWindow()) {
      getWindow()->clear();
      clearChildRefs(L);
    }
    return true;
  }

  if (!pcallUpdate1Int(L, getColorFunction,
                       [=](int color) { setColor(color); }))
    return false;
  if (!pcallUpdateBool(L, getVisibleFunction,
                       [=](bool visible) { if (visible) show(); else hide(); }))
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
  }

  return true;
}

void LvglWidgetObjectBase::saveLvglObjectRef(int ref)
{
  lvglObjectRefs.push_back(ref);
}

void LvglWidgetObjectBase::clearRef(lua_State *L, int& ref)
{
  if (ref != LUA_REFNIL)
    luaL_unref(L, LUA_REGISTRYINDEX, ref);
  ref = LUA_REFNIL;
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
  clearRef(L, luaRef);
  clearRef(L, getColorFunction);
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
  setColor(color);
}

void LvglWidgetObjectBase::update(lua_State *L)
{
  getParams(L, 2);
  refresh();
}

bool LvglWidgetObjectBase::colorChanged(LcdFlags newColor)
{
  color = newColor;
  LcdFlags c = color;
  if (!(c & RGB_FLAG))
    c = COLOR(COLOR_VAL(c)) | RGB_FLAG; // Convert index to RGB
  if (currentColor != c) {
    currentColor = c;
    return true;
  }
  return false;
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
  if (lvobj) lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
}

void LvglSimpleWidgetObject::hide()
{
  if (lvobj) lv_obj_add_flag(lvobj, LV_OBJ_FLAG_HIDDEN);
}

//-----------------------------------------------------------------------------

void LvglWidgetLabel::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "text")) {
    if (lua_isfunction(L, -1))
      getTextFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    else
      txt = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "font")) {
    if (lua_isfunction(L, -1))
      getFontFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    else
      font = luaL_checkunsigned(L, -1);
  } else {
    LvglSimpleWidgetObject::parseParam(L, key);
  }
}

bool LvglWidgetLabel::callRefs(lua_State *L)
{
  int t = lua_gettop(L);
  if (getTextFunction != LUA_REFNIL) {
    if (pcallFunc(L, getTextFunction, 1)) {
      const char *s = luaL_checkstring(L, -1);
      setText(s);
      lua_settop(L, t);
    } else {
      return false;
    }
  }
  if (!pcallUpdate1Int(L, getFontFunction, [=](int val) { setFont(val); }))
    return false;
  return LvglSimpleWidgetObject::callRefs(L);
}

void LvglWidgetLabel::clearRefs(lua_State *L)
{
  clearRef(L, getTextFunction);
  clearRef(L, getFontFunction);
  LvglSimpleWidgetObject::clearRefs(L);
}

void LvglWidgetLabel::setText(const char *s)
{
  uint32_t h = hash(s, strlen(s));
  if (h != textHash) {
    txt = s;
    textHash = h;
    if (lvobj) lv_label_set_text(lvobj, s);
  }
}

void LvglWidgetLabel::setColor(LcdFlags newColor)
{
  if (lvobj && colorChanged(newColor)) {
    if (color & RGB_FLAG) {
      etx_remove_txt_color(lvobj);
      lv_obj_set_style_text_color(lvobj, makeLvColor(color), LV_PART_MAIN);
    } else {
      lv_obj_remove_local_style_prop(lvobj, LV_STYLE_TEXT_COLOR, LV_PART_MAIN);
      etx_txt_color(lvobj, (LcdColorIndex)COLOR_VAL(color));
    }
  }
}

void LvglWidgetLabel::setFont(LcdFlags font)
{
  if (lvobj) {
    this->font = font;
    if (font & VCENTERED) {
      lv_obj_align(lvobj, LV_ALIGN_LEFT_MID, 0, 0);
    }
    lv_obj_set_style_text_align(lvobj,
                                (font & RIGHT)      ? LV_TEXT_ALIGN_RIGHT
                                : (font & CENTERED) ? LV_TEXT_ALIGN_CENTER
                                                    : LV_TEXT_ALIGN_LEFT,
                                LV_PART_MAIN);
    lv_obj_set_style_text_font(lvobj, getFont(font), LV_PART_MAIN);
  }
}

void LvglWidgetLabel::build(lua_State *L)
{
  lvobj = lv_label_create(lvglManager->getCurrentParent()->getLvObj());
  setPos(x, y);
  setSize(w, h);
  setText(txt);
  setColor(color);
  setFont(font);
  callRefs(L);
}

//-----------------------------------------------------------------------------

void LvglWidgetLineBase::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "rounded")) {
    rounded = lua_toboolean(L, -1);
  } else {
    LvglSimpleWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetLineBase::setColor(LcdFlags newColor)
{
  if (lvobj && colorChanged(newColor)) {
    if (color & RGB_FLAG) {
      etx_remove_line_color(lvobj);
      lv_obj_set_style_line_color(lvobj, makeLvColor(color), LV_PART_MAIN);
    } else {
      lv_obj_remove_local_style_prop(lvobj, LV_STYLE_LINE_COLOR, LV_PART_MAIN);
      etx_line_color(lvobj, (LcdColorIndex)COLOR_VAL(color));
    }
  }
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

void LvglWidgetLineBase::build(lua_State* L)
{
  lvobj = lv_line_create(lvglManager->getCurrentParent()->getLvObj());
  lv_obj_set_style_line_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);
  refresh();
}

void LvglWidgetLineBase::refresh()
{
  setColor(color);
  setLine();
  lv_obj_set_style_line_rounded(lvobj, rounded, LV_PART_MAIN);
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

void LvglWidgetLine::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "thickness")) {
    thickness = luaL_checkunsigned(L, -1);
  } else if (!strcmp(key, "rounded")) {
    rounded = lua_toboolean(L, -1);
  } else if (!strcmp(key, "pts")) {
    luaL_checktype(L, -1, LUA_TTABLE);
    ptCnt = lua_rawlen(L, -1);
    if (pts) delete pts;
    if (ptCnt > 1) {
      pts = new lv_point_t[ptCnt];
      for (size_t i = 0; i < ptCnt; i += 1)
        getPt(L, i);
    }
  } else {
    LvglSimpleWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetLine::setColor(LcdFlags newColor)
{
  if (lvobj && colorChanged(newColor)) {
    if (color & RGB_FLAG) {
      etx_remove_line_color(lvobj);
      lv_obj_set_style_line_color(lvobj, makeLvColor(color), LV_PART_MAIN);
    } else {
      lv_obj_remove_local_style_prop(lvobj, LV_STYLE_LINE_COLOR, LV_PART_MAIN);
      etx_line_color(lvobj, (LcdColorIndex)COLOR_VAL(color));
    }
  }
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
  if (lvobj && pts) {
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
  lvobj = lv_line_create(lvglManager->getCurrentParent()->getLvObj());
  lv_obj_set_style_line_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);
  refresh();
  callRefs(L);
}

void LvglWidgetLine::refresh()
{
  setColor(color);
  setLine();
}

//-----------------------------------------------------------------------------

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
  px[n] = luaL_checkunsigned(L, -1);
  lua_pop(L, 1);
  lua_rawgeti(L, -1, 2);
  py[n] = luaL_checkunsigned(L, -1);
  lua_pop(L, 2);
}

void LvglWidgetTriangle::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "pts")) {
    luaL_checktype(L, -1, LUA_TTABLE);
    getPt(L, 0);
    getPt(L, 1);
    getPt(L, 2);
  } else {
    LvglSimpleWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetTriangle::setColor(LcdFlags newColor)
{
  if (lvobj && colorChanged(newColor)) {
    if (color & RGB_FLAG) {
      etx_remove_img_color(lvobj);
      lv_obj_set_style_img_recolor(lvobj, makeLvColor(color), LV_PART_MAIN);
      lv_obj_set_style_img_recolor_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);
    } else {
      lv_obj_remove_local_style_prop(lvobj, LV_STYLE_IMG_RECOLOR, LV_PART_MAIN);
      etx_img_color(lvobj, (LcdColorIndex)COLOR_VAL(color));
    }
  }
}

void LvglWidgetTriangle::setSize(coord_t w, coord_t h)
{
  // TODO: Scale triangle?
}

void LvglWidgetTriangle::fillLine(coord_t x1, coord_t x2, coord_t y)
{
  memset(&mask->data[y * w + x1], 255, x2 - x1 + 1);
}

// Swap two bytes
#define SWAP(x,y) (x)=(x)^(y); (y)=(x)^(y); (x)=(x)^(y);

void LvglWidgetTriangle::fillTriangle()
{
  if (!mask) return;

  coord_t x1 = px[0], y1 = py[0], x2 = px[1], y2 = py[1], x3 = px[2], y3 = py[2];

  coord_t t1x, t2x, y, minx, maxx, t1xp, t2xp;
  bool changed1 = false;
  bool changed2 = false;
  coord_t signx1, signx2, dx1, dy1, dx2, dy2;
  coord_t e1, e2;

  // Sort vertices
  if (y1 > y2) { SWAP(y1, y2); SWAP(x1, x2); }
  if (y1 > y3) { SWAP(y1, y3); SWAP(x1, x3); }
  if (y2 > y3) { SWAP(y2, y3); SWAP(x2, x3); }

  t1x = t2x = x1; y = y1;   // Starting points

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
    // process first line until y value is about to change
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
    // process second line until y value is about to change
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
    fillLine(minx, maxx, y); // Draw line from min to max points found on the y
    // Now increase y
    if (!changed1) t1x += signx1;
    t1x += t1xp;
    if (!changed2) t2x += signx2;
    t2x += t2xp;
    y += 1;
    if (y == y2) break;
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
    // process first line until y value is about to change
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
    // process second line until y value is about to change
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
    fillLine(minx, maxx, y); // Draw line from min to max points found on the y
    // Now increase y
    if (!changed1) t1x += signx1;
    t1x += t1xp;
    if (!changed2) t2x += signx2;
    t2x += t2xp;
    y += 1;
    if (y > y3) return;
  }
}

void LvglWidgetTriangle::build(lua_State *L)
{
  // Bounds
  x = min(min(px[0], px[1]), px[2]);
  y = min(min(py[0], py[1]), py[2]);
  w = max(max(px[0], px[1]), px[2]) - x + 1;
  h = max(max(py[0], py[1]), py[2]) - y + 1;

  // Convert to relative coords
  px[0] -= x; px[1] -= x; px[2] -= x;
  py[0] -= y; py[1] -= y; py[2] -= y;

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
      lvobj = lv_canvas_create(lvglManager->getCurrentParent()->getLvObj());
    lv_canvas_set_buffer(lvobj, (void*)mask->data, mask->width, mask->height,
                        LV_IMG_CF_ALPHA_8BIT);

    // Set position, size and color
    setPos(x, y);
    LvglSimpleWidgetObject::setSize(w,h);
    setColor(color);
  }
  if (L) callRefs(L);
}

void LvglWidgetTriangle::refresh()
{
  if (mask) free(mask);
  mask = nullptr;
  build(nullptr);
}

//-----------------------------------------------------------------------------

void LvglWidgetObject::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "flexFlow")) {
    flexFlow = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "flexPad")) {
    flexPad = luaL_checkinteger(L, -1);
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
  if (flexFlow >= 0) {
    window->padAll(PAD_TINY);
    window->setFlexLayout((lv_flex_flow_t)flexFlow, flexPad, w, h);
    return true;
  } else {
    window->padAll(PAD_ZERO);
    return false;
  }
}

//-----------------------------------------------------------------------------

void LvglWidgetBox::build(lua_State* L)
{
  window =
      new Window(lvglManager->getCurrentParent(), {x, y, w, h}, lv_obj_create);
  if (setFlex())
    lv_obj_set_flex_align(window->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
}

//-----------------------------------------------------------------------------

void LvglWidgetSetting::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "title")) {
    txt = luaL_checkstring(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetSetting::build(lua_State* L)
{
  window =
      new Window(lvglManager->getCurrentParent(), {x, y, w, h}, lv_obj_create);
  window->padAll(PAD_TINY);
  auto lbl = lv_label_create(window->getLvObj());
  lv_obj_align(lbl, LV_ALIGN_LEFT_MID, 0, 0);
  etx_txt_color(lbl, COLOR_THEME_PRIMARY1_INDEX);
  lv_label_set_text(lbl, txt);
}

//-----------------------------------------------------------------------------

void LvglWidgetBorderedObject::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "thickness")) {
    thickness = luaL_checkunsigned(L, -1);
  } else if (!strcmp(key, "filled")) {
    filled = lua_toboolean(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetBorderedObject::setColor(LcdFlags newColor)
{
  if (colorChanged(newColor)) {
    if (filled) {
      etx_bg_color_from_flags(window->getLvObj(), color);
    } else {
      if (color & RGB_FLAG) {
        etx_remove_border_color(window->getLvObj());
        lv_obj_set_style_border_color(window->getLvObj(),
                                      makeLvColor(color), LV_PART_MAIN);
      } else {
        lv_obj_remove_local_style_prop(window->getLvObj(), LV_STYLE_BORDER_COLOR, LV_PART_MAIN);
        etx_border_color(window->getLvObj(), (LcdColorIndex)COLOR_VAL(color));
      }
    }
  }
}

void LvglWidgetBorderedObject::build(lua_State *L)
{
  window =
      new Window(lvglManager->getCurrentParent(), {x, y, w, h}, lv_obj_create);
  lv_obj_add_flag(window->getLvObj(), LV_OBJ_FLAG_EVENT_BUBBLE);
  if (luaLvglManager->isWidget()) {
    lv_obj_clear_flag(window->getLvObj(), LV_OBJ_FLAG_CLICKABLE);
  } else {
    etx_scrollbar(window->getLvObj());
  }
  setColor(color);
  if (filled) {
    lv_obj_set_style_bg_opa(window->getLvObj(), LV_OPA_COVER, LV_PART_MAIN);
  } else {
    lv_obj_set_style_border_opa(window->getLvObj(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_width(window->getLvObj(), thickness, LV_PART_MAIN);
  }
}

//-----------------------------------------------------------------------------

void LvglWidgetRoundObject::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "radius")) {
    if (lua_isfunction(L, -1))
      getRadiusFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    else
      radius = luaL_checkunsigned(L, -1);
  } else {
    LvglWidgetBorderedObject::parseParam(L, key);
  }
}

bool LvglWidgetRoundObject::callRefs(lua_State *L)
{
  if (!pcallUpdate1Int(L, getRadiusFunction, [=](int val) { setRadius(val); }))
    return false;
  return LvglWidgetObject::callRefs(L);
}

void LvglWidgetRoundObject::clearRefs(lua_State *L)
{
  clearRef(L, getRadiusFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetRoundObject::setPos(coord_t x, coord_t y)
{
  LvglWidgetObject::setPos(x - radius, y - radius);
}

void LvglWidgetRoundObject::setRadius(coord_t r)
{
  // Set position to center
  x += radius;
  y += radius;
  radius = r;
  w = radius * 2;
  h = radius * 2;
  setPos(x, y);
  setSize(w, h);
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
  callRefs(L);
}

//-----------------------------------------------------------------------------

void LvglWidgetCircle::build(lua_State *L)
{
  // Reset position to corner
  setPos(x, y);
  // Set width & height
  setRadius(radius);
  LvglWidgetBorderedObject::build(L);
  lv_obj_set_style_radius(window->getLvObj(), LV_RADIUS_CIRCLE, LV_PART_MAIN);
  callRefs(L);
}

//-----------------------------------------------------------------------------

void LvglWidgetArc::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "startAngle")) {
    if (lua_isfunction(L, -1))
      getStartAngleFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    else
      startAngle = luaL_checkunsigned(L, -1);
  } else if (!strcmp(key, "endAngle")) {
    if (lua_isfunction(L, -1))
      getEndAngleFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    else
      endAngle = luaL_checkunsigned(L, -1);
  } else {
    LvglWidgetRoundObject::parseParam(L, key);
  }
}

void LvglWidgetArc::setColor(LcdFlags newColor)
{
  if (colorChanged(newColor)) {
    if (color & RGB_FLAG) {
      etx_remove_arc_color(window->getLvObj());
      lv_obj_set_style_arc_color(window->getLvObj(), makeLvColor(color), LV_PART_INDICATOR);
    } else {
      lv_obj_remove_local_style_prop(window->getLvObj(), LV_STYLE_ARC_COLOR, LV_PART_MAIN);
      etx_arc_color(window->getLvObj(), (LcdColorIndex)COLOR_VAL(color), LV_PART_INDICATOR);
    }
  }
}

void LvglWidgetArc::setStartAngle(coord_t angle)
{
  lv_arc_set_start_angle(window->getLvObj(), angle);
}

void LvglWidgetArc::setEndAngle(coord_t angle)
{
  lv_arc_set_end_angle(window->getLvObj(), angle);
}

bool LvglWidgetArc::callRefs(lua_State *L)
{
  if (!pcallUpdate1Int(L, getStartAngleFunction,
                       [=](int val) { setStartAngle(val); }))
    return false;
  if (!pcallUpdate1Int(L, getEndAngleFunction,
                       [=](int val) { setEndAngle(val); }))
    return false;
  return LvglWidgetRoundObject::callRefs(L);
}

void LvglWidgetArc::clearRefs(lua_State *L)
{
  clearRef(L, getStartAngleFunction);
  clearRef(L, getEndAngleFunction);
  LvglWidgetRoundObject::clearRefs(L);
}

void LvglWidgetArc::build(lua_State *L)
{
  // Reset position to corner
  setPos(x, y);
  // Set width & height
  setRadius(radius);
  window =
      new Window(lvglManager->getCurrentParent(), {x, y, w, h}, lv_arc_create);
  lv_obj_add_flag(window->getLvObj(), LV_OBJ_FLAG_EVENT_BUBBLE);
  lv_obj_clear_flag(window->getLvObj(), LV_OBJ_FLAG_CLICKABLE);
  setColor(color);
  lv_arc_set_bg_angles(window->getLvObj(), 0, 360);
  lv_arc_set_range(window->getLvObj(), 0, 360);
  lv_arc_set_angles(window->getLvObj(), 0, 360);
  setStartAngle(startAngle);
  setEndAngle(endAngle);
  lv_obj_remove_style(window->getLvObj(), NULL, LV_PART_KNOB);
  lv_obj_set_style_arc_opa(window->getLvObj(), LV_OPA_TRANSP, LV_PART_MAIN);
  lv_obj_set_style_arc_width(window->getLvObj(), thickness, LV_PART_MAIN);
  lv_obj_set_style_arc_opa(window->getLvObj(), LV_OPA_COVER, LV_PART_INDICATOR);
  lv_obj_set_style_arc_width(window->getLvObj(), thickness, LV_PART_INDICATOR);
  callRefs(L);
}

//-----------------------------------------------------------------------------

void LvglWidgetImage::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "file")) {
    filename = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "fill")) {
    fillFrame = lua_toboolean(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetImage::build(lua_State *L)
{
  window = new StaticImage(lvglManager->getCurrentParent(), {x, y, w, h},
                           filename.c_str(), fillFrame);
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
  window = new QRCode(lvglManager->getCurrentParent(), x, y, w, data, colorToRGB(color), colorToRGB(bgColor));
}

//-----------------------------------------------------------------------------

void LvglWidgetTextButton::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "text")) {
    txt = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "press")) {
    pressFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetTextButton::clearRefs(lua_State *L)
{
  clearRef(L, pressFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetTextButton::setText(const char *s)
{
  uint32_t h = hash(s, strlen(s));
  if (h != textHash) {
    txt = s;
    textHash = h;
    ((TextButton *)window)->setText(s);
  }
}

void LvglWidgetTextButton::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  window =
      new TextButton(lvglManager->getCurrentParent(), {x, y, w, h}, txt, [=]() {
        pcallSimpleFunc(L, pressFunction);
        return 0;
      });
}

//-----------------------------------------------------------------------------

void LvglWidgetToggleSwitch::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "get")) {
    getStateFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "set")) {
    setStateFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetToggleSwitch::clearRefs(lua_State *L)
{
  clearRef(L, getStateFunction);
  clearRef(L, setStateFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetToggleSwitch::build(lua_State *L)
{
  window = new ToggleSwitch(
      lvglManager->getCurrentParent(), {x, y, 0, 0},
      [=]() { return pcallGetIntVal(L, getStateFunction); },
      [=](uint8_t val) { pcallSetIntVal(L, setStateFunction, val); });
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
                              if (!pcallFuncWithString(L, setFunction, 0, value)) {
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
  if (!strcmp(key, "min")) {
    min = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "max")) {
    max = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "get")) {
    getFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "set")) {
    setFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "display")) {
    dispFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetNumberEdit::clearRefs(lua_State *L)
{
  clearRef(L, getFunction);
  clearRef(L, setFunction);
  clearRef(L, dispFunction);
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
}

//-----------------------------------------------------------------------------

void LvglWidgetSlider::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "get")) {
    getValueFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "set")) {
    setValueFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "min")) {
    vmin = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "max")) {
    vmax = luaL_checkinteger(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetSlider::clearRefs(lua_State *L)
{
  clearRef(L, getValueFunction);
  clearRef(L, setValueFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetSlider::build(lua_State *L)
{
  window = new Slider(
      lvglManager->getCurrentParent(), w, vmin, vmax,
      [=]() { return pcallGetIntVal(L, getValueFunction); },
      [=](uint8_t val) { pcallSetIntVal(L, setValueFunction, val); });
  window->setPos(x, y);
}

//-----------------------------------------------------------------------------

class WidgetPage : public NavWindow, public LuaEventHandler
{
 public:
  WidgetPage(Window *parent, std::function<void()> backAction,
             std::string title, std::string subtitle, std::string iconFile) :
      NavWindow(parent, {0, 0, LCD_W, LCD_H}), backAction(std::move(backAction))
  {
    if (iconFile.empty())
      header = new PageHeader(this, ICON_EDGETX);
    else
      header = new PageHeader(this, iconFile.c_str());

    body = new Window(
        this, {0, EdgeTxStyles::MENU_HEADER_HEIGHT, LCD_W, LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT});
    body->setWindowFlag(NO_FOCUS);

    header->setTitle(title);
    header->setTitle2(subtitle);

    etx_solid_bg(lvobj);
    lv_obj_set_style_max_height(body->getLvObj(), LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT,
                                LV_PART_MAIN);
    etx_scrollbar(body->getLvObj());

#if defined(HARDWARE_TOUCH)
    addBackButton();
#endif
  }

  Window *getBody() { return body; }

 protected:
  std::function<void()> backAction;
  PageHeader *header = nullptr;
  Window *body = nullptr;

  bool bubbleEvents() override { return true; }

  void onClicked() override { Keyboard::hide(false); LuaEventHandler::onClicked(); }

  void onCancel() override { backAction(); }

  void onEvent(event_t evt) override
  {
    LuaEventHandler::onEvent(evt);
  }

 protected:
};

void LvglWidgetPage::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "back")) {
    backActionFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "title")) {
    title = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "subtitle")) {
    subtitle = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "icon")) {
    iconFile = luaL_checkstring(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetPage::clearRefs(lua_State *L)
{
  clearRef(L, backActionFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetPage::build(lua_State *L)
{
  auto page = new WidgetPage(
      lvglManager->getCurrentParent(),
      [=]() { pcallSimpleFunc(L, backActionFunction); }, title, subtitle, iconFile);

  window = page->getBody();
  if (setFlex())
    lv_obj_set_flex_align(window->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_SPACE_AROUND);
}

//-----------------------------------------------------------------------------

class LvglDialog : public BaseDialog
{
 public:
  LvglDialog(const char* title, coord_t w, coord_t h, std::function<void()> onClose) :
    BaseDialog(title, true, w, h, false),
    onClose(std::move(onClose))
  {
    form->setHeight(h - EdgeTxStyles::UI_ELEMENT_HEIGHT);
  }

  Window *getBody() { return form; }

 protected:
  std::function<void()> onClose;

  void onCancel() override
  {
    onClose();
    BaseDialog::onCancel();
  }
};

void LvglWidgetDialog::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "close")) {
    closeFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "title")) {
    title = luaL_checkstring(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetDialog::clearRefs(lua_State *L)
{
  clearRef(L, closeFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetDialog::build(lua_State *L)
{
  if (w == LV_SIZE_CONTENT) w = DIALOG_DEFAULT_WIDTH;
  if (h == LV_SIZE_CONTENT) h = DIALOG_DEFAULT_HEIGHT;
  auto dlg = new LvglDialog(title, w, h,
      [=]() { pcallSimpleFunc(L, closeFunction); });
  window = dlg->getBody();
  window->setWidth(w);
  setFlex();
}

//-----------------------------------------------------------------------------

void LvglWidgetConfirmDialog::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "title")) {
    title = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "message")) {
    message = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "confirm")) {
    confirmFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "cancel")) {
    cancelFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetConfirmDialog::clearRefs(lua_State *L)
{
  clearRef(L, confirmFunction);
  clearRef(L, cancelFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetConfirmDialog::build(lua_State *L)
{
  window = new ConfirmDialog(title, message,
      [=]() { pcallSimpleFunc(L, confirmFunction); },
      [=]() { pcallSimpleFunc(L, cancelFunction); });
}

//-----------------------------------------------------------------------------

void LvglWidgetMessageDialog::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "title")) {
    title = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "message")) {
    message = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "details")) {
    details = luaL_checkstring(L, -1);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetMessageDialog::build(lua_State *L)
{
  window = new MessageDialog(title, message, details);
}

//-----------------------------------------------------------------------------

void LvglWidgetPicker::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "get")) {
    getFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "set")) {
    setFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetPicker::clearRefs(lua_State *L)
{
  clearRef(L, getFunction);
  clearRef(L, setFunction);
  LvglWidgetObject::clearRefs(L);
}

//-----------------------------------------------------------------------------

void LvglWidgetChoice::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "title")) {
    title = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "values")) {
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      values.push_back(lua_tostring(L, -1));
    }
  } else {
    LvglWidgetPicker::parseParam(L, key);
  }
}

void LvglWidgetChoice::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  window = new Choice(
      lvglManager->getCurrentParent(), {x, y, w, h}, values, 0, values.size() - 1,
      [=]() { return pcallGetIntVal(L, getFunction) - 1; },
      [=](int val) { pcallSetIntVal(L, setFunction, val + 1); }, title.c_str());
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
  if (!strcmp(key, "min")) {
    vmin = luaL_checkinteger(L, -1);
  } else if (!strcmp(key, "max")) {
    vmax = luaL_checkinteger(L, -1);
  } else {
    LvglWidgetPicker::parseParam(L, key);
  }
}

void LvglWidgetSwitchPicker::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  window = new SwitchChoice(
      lvglManager->getCurrentParent(), {x, y, w, h},
      vmin, vmax,
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](uint32_t val) { pcallSetIntVal(L, setFunction, val); });
}

//-----------------------------------------------------------------------------

void LvglWidgetSourcePicker::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  window = new SourceChoice(
      lvglManager->getCurrentParent(), {x, y, w, h},
      0, MIXSRC_LAST_TELEM,
      [=]() { return pcallGetIntVal(L, getFunction); },
      [=](uint32_t val) { pcallSetIntVal(L, setFunction, val); },
      true);
}

//-----------------------------------------------------------------------------
