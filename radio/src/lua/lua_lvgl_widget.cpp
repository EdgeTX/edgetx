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
#include "opentx.h"
#include "page.h"
#include "slider.h"
#include "toggleswitch.h"
#include "lua_event.h"

//-----------------------------------------------------------------------------

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

bool LvglWidgetObjectBase::pcallFunc(lua_State *L, int getFuncRef, int nretval)
{
  lua_rawgeti(L, LUA_REGISTRYINDEX, getFuncRef);
  return lua_pcall(L, 0, nretval, 0) == 0;
}

void LvglWidgetObjectBase::pcallSimpleFunc(lua_State *L, int funcRef)
{
  if (funcRef) {
    PROTECT_LUA()
    {
      if (!pcallFunc(L, funcRef, 1)) {
        lvglManager->luaShowError();
      }
    }
    UNPROTECT_LUA();
  }
}

bool LvglWidgetObjectBase::pcallUpdateBool(lua_State *L, int getFuncRef,
                                           std::function<void(bool)> update)
{
  if (getFuncRef) {
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
  if (getFuncRef) {
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
  if (getFuncRef) {
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
  if (getFuncRef) {
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
  if (setFuncRef) {
    int t = lua_gettop(L);
    PROTECT_LUA()
    {
      lua_rawgeti(L, LUA_REGISTRYINDEX, setFuncRef);
      lua_pushinteger(L, val);
      bool rv = lua_pcall(L, 1, 0, 0) == 0;
      if (!rv) {
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

void LvglWidgetObjectBase::clearRef(lua_State *L, int ref)
{
  if (ref) luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

void LvglWidgetObjectBase::build(lua_State *L) {}

//-----------------------------------------------------------------------------

bool LvglWidgetObject::callRefs(lua_State *L)
{
  if (!pcallUpdate1Int(L, getColorFunction,
                       [=](int color) { setColor(color); }))
    return false;
  if (!pcallUpdateBool(L, getVisibleFunction,
                       [=](bool visible) { window->show(visible); }))
    return false;
  if (!pcallUpdate2Int(L, getSizeFunction,
                       [=](int w, int h) { setSize(w, h); }))
    return false;
  if (!pcallUpdate2Int(L, getPosFunction, [=](int x, int y) { setPos(x, y); }))
    return false;
  return true;
}

void LvglWidgetObject::clearRefs(lua_State *L)
{
  clearRef(L, getColorFunction);
  clearRef(L, getVisibleFunction);
  clearRef(L, getSizeFunction);
  clearRef(L, getPosFunction);
}

void LvglWidgetObject::parseParam(lua_State *L, const char *key)
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

int LvglWidgetObject::getRef(lua_State *L)
{
  LvglWidgetObject **p =
      (LvglWidgetObject **)lua_newuserdata(L, sizeof(LvglWidgetObject *));
  *p = this;
  luaL_getmetatable(L, LVGL_METATABLE);
  lua_setmetatable(L, -2);

  // Save reference
  auto ref = luaL_ref(L, LUA_REGISTRYINDEX);
  lvglManager->saveLvglObjectRef(ref);

  return ref;
}

void LvglWidgetObject::push(lua_State *L)
{
  // Save reference
  auto ref = getRef(L);
  // Push userdata back into Lua stack (return object)
  lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
}

void LvglWidgetObject::update(lua_State *L)
{
  getParams(L, 2);
  refresh();
}

void LvglWidgetObject::refresh()
{
  setPos(x, y);
  setSize(w, h);
  setColor(color);
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

void LvglWidgetBorderedObject::setColor(LcdFlags color)
{
  if (color != currentColor) {
    currentColor = color;
    if (filled) {
      etx_bg_color_from_flags(window->getLvObj(), color);
    } else {
      if (color & RGB_FLAG) {
        etx_remove_border_color(window->getLvObj());
        lv_obj_set_style_border_color(window->getLvObj(),
                                      makeLvColor(colorToRGB(color)), LV_PART_MAIN);
      } else {
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
    LvglWidgetObject::parseParam(L, key);
  }
}

bool LvglWidgetLabel::callRefs(lua_State *L)
{
  int t = lua_gettop(L);
  if (getTextFunction) {
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
  return LvglWidgetObject::callRefs(L);
}

void LvglWidgetLabel::clearRefs(lua_State *L)
{
  clearRef(L, getTextFunction);
  clearRef(L, getFontFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetLabel::setText(const char *s)
{
  uint32_t h = hash(s, strlen(s));
  if (h != textHash) {
    txt = s;
    textHash = h;
    lv_label_set_text(window->getLvObj(), s);
  }
}

void LvglWidgetLabel::setColor(LcdFlags color)
{
  if (color != currentColor) {
    currentColor = color;
    if (color & RGB_FLAG) {
      etx_remove_txt_color(window->getLvObj());
      lv_obj_set_style_text_color(window->getLvObj(),
                                  makeLvColor(colorToRGB(color)), LV_PART_MAIN);
    } else {
      etx_txt_color(window->getLvObj(), (LcdColorIndex)COLOR_VAL(color));
    }
  }
}

void LvglWidgetLabel::setFont(LcdFlags font)
{
  this->font = font;
  lv_obj_set_style_text_align(window->getLvObj(),
                              (font & RIGHT)      ? LV_TEXT_ALIGN_RIGHT
                              : (font & CENTERED) ? LV_TEXT_ALIGN_CENTER
                                                  : LV_TEXT_ALIGN_LEFT,
                              LV_PART_MAIN);
  lv_obj_set_style_text_font(window->getLvObj(), getFont(font), LV_PART_MAIN);
}

void LvglWidgetLabel::build(lua_State *L)
{
  window = new Window(lvglManager->getCurrentParent(), {x, y, w, h},
                      lv_label_create);
  setText(txt);
  setColor(color);
  setFont(font);
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
  setRadius(radius);
  LvglWidgetBorderedObject::build(L);
  lv_obj_set_style_radius(window->getLvObj(), LV_RADIUS_CIRCLE, LV_PART_MAIN);
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

void LvglWidgetArc::setColor(LcdFlags color)
{
  if (color != currentColor) {
    currentColor = color;
    if (color & RGB_FLAG) {
      etx_remove_arc_color(window->getLvObj());
      lv_obj_set_style_arc_color(window->getLvObj(), makeLvColor(colorToRGB(color)),
                                 LV_PART_INDICATOR);
    } else {
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

class LvglWidgetScaleIndicator : public LvglWidgetObjectBase
{
 public:
  LvglWidgetScaleIndicator() {}

  virtual void buildIndicator(lv_obj_t *parent, lv_meter_scale_t *scale) = 0;

 protected:
  lv_obj_t *meter = nullptr;
  lv_meter_indicator_t *indic = nullptr;
};

class LvglWidgetScaleArc : public LvglWidgetScaleIndicator
{
 public:
  LvglWidgetScaleArc(lua_State *L) { getParams(L, -1); }

  bool callRefs(lua_State *L) override
  {
    if (!pcallUpdate1Int(L, getStartPosFunction, [=](int val) {
          lv_meter_set_indicator_start_value(meter, indic, val);
        }))
      return false;
    if (!pcallUpdate1Int(L, getEndPosFunction, [=](int val) {
          lv_meter_set_indicator_end_value(meter, indic, val);
        }))
      return false;
    return true;
  }

  void clearRefs(lua_State *L) override
  {
    clearRef(L, getStartPosFunction);
    clearRef(L, getEndPosFunction);
  }

  void buildIndicator(lv_obj_t *parent, lv_meter_scale_t *scale) override
  {
    meter = parent;

    indic =
        lv_meter_add_arc(meter, scale, w, makeLvColor(colorToRGB(color)), rmod);
    if (hasStart) lv_meter_set_indicator_start_value(meter, indic, startPos);
    if (hasEnd) lv_meter_set_indicator_end_value(meter, indic, endPos);
  }

 protected:
  uint8_t w = 0;
  int8_t rmod = 0;
  LcdFlags color = COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX);
  bool hasStart = false, hasEnd = false;
  int16_t startPos = 0, endPos = 0;

  int getStartPosFunction = 0;
  int getEndPosFunction = 0;

  void parseParam(lua_State *L, const char *key) override
  {
    if (!strcmp(key, "w")) {
      w = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "rmod")) {
      rmod = luaL_checkinteger(L, -1);
    } else if (!strcmp(key, "startPos")) {
      if (lua_isfunction(L, -1))
        getStartPosFunction = luaL_ref(L, LUA_REGISTRYINDEX);
      else {
        hasStart = true;
        startPos = luaL_checkinteger(L, -1);
      }
    } else if (!strcmp(key, "endPos")) {
      if (lua_isfunction(L, -1))
        getEndPosFunction = luaL_ref(L, LUA_REGISTRYINDEX);
      else {
        hasEnd = true;
        endPos = luaL_checkinteger(L, -1);
      }
    } else if (!strcmp(key, "color")) {
      color = luaL_checkunsigned(L, -1);
    }
  }
};

class LvglWidgetScaleLines : public LvglWidgetScaleIndicator
{
 public:
  LvglWidgetScaleLines(lua_State *L) { getParams(L, -1); }

  bool callRefs(lua_State *L) override
  {
    if (!pcallUpdate1Int(L, getStartPosFunction, [=](int val) {
          lv_meter_set_indicator_start_value(meter, indic, val);
        }))
      return false;
    if (!pcallUpdate1Int(L, getEndPosFunction, [=](int val) {
          lv_meter_set_indicator_end_value(meter, indic, val);
        }))
      return false;
    return true;
  }

  void clearRefs(lua_State *L) override
  {
    clearRef(L, getStartPosFunction);
    clearRef(L, getEndPosFunction);
  }

  void buildIndicator(lv_obj_t *parent, lv_meter_scale_t *scale) override
  {
    meter = parent;

    indic = lv_meter_add_scale_lines(
        meter, scale, makeLvColor(colorToRGB(startColor)),
        makeLvColor(colorToRGB(endColor)), localFade, wmod);
    if (hasStart) lv_meter_set_indicator_start_value(meter, indic, startPos);
    if (hasEnd) lv_meter_set_indicator_end_value(meter, indic, endPos);
  }

 protected:
  int8_t wmod = 0;
  LcdFlags startColor = COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX);
  LcdFlags endColor = COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX);
  bool hasStart = false, hasEnd = false;
  int16_t startPos = 0, endPos = 0;
  bool localFade = false;

  int getStartPosFunction = 0;
  int getEndPosFunction = 0;

  void parseParam(lua_State *L, const char *key) override
  {
    if (!strcmp(key, "wmod")) {
      wmod = luaL_checkinteger(L, -1);
    } else if (!strcmp(key, "startPos")) {
      if (lua_isfunction(L, -1)) {
        getStartPosFunction = luaL_ref(L, LUA_REGISTRYINDEX);
      } else {
        hasStart = true;
        startPos = luaL_checkinteger(L, -1);
      }
    } else if (!strcmp(key, "endPos")) {
      if (lua_isfunction(L, -1)) {
        getEndPosFunction = luaL_ref(L, LUA_REGISTRYINDEX);
      } else {
        hasEnd = true;
        endPos = luaL_checkinteger(L, -1);
      }
    } else if (!strcmp(key, "startColor")) {
      startColor = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "endColor")) {
      endColor = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "localFade")) {
      localFade = lua_toboolean(L, -1);
    }
  }
};

class LvglWidgetScaleNeedle : public LvglWidgetScaleIndicator
{
 public:
  LvglWidgetScaleNeedle(lua_State *L) { getParams(L, -1); }

  bool callRefs(lua_State *L) override
  {
    if (!pcallUpdate1Int(L, getPosFunction, [=](int val) {
          lv_meter_set_indicator_value(meter, indic, val);
        }))
      return false;
    return true;
  }

  void clearRefs(lua_State *L) override { clearRef(L, getPosFunction); }

  void buildIndicator(lv_obj_t *parent, lv_meter_scale_t *scale) override
  {
    meter = parent;

    indic = lv_meter_add_needle_line(meter, scale, w,
                                     makeLvColor(colorToRGB(color)), rmod);
  }

 protected:
  uint8_t w = 0;
  int8_t rmod = 0;
  LcdFlags color = COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX);

  int getPosFunction = 0;

  void parseParam(lua_State *L, const char *key) override
  {
    if (!strcmp(key, "w")) {
      w = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "rmod")) {
      rmod = luaL_checkinteger(L, -1);
    } else if (!strcmp(key, "color")) {
      color = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "pos")) {
      getPosFunction = luaL_ref(L, LUA_REGISTRYINDEX);
    }
  }
};

class LvglWidgetMeterScale : public LvglWidgetObjectBase
{
 public:
  LvglWidgetMeterScale(lua_State *L) { getParams(L, -1); }

  bool callRefs(lua_State *L) override
  {
    for (auto it = indicators.cbegin(); it != indicators.cend(); ++it) {
      if (!(*it)->callRefs(L)) return false;
    }
    return true;
  }

  void clearRefs(lua_State *L) override
  {
    for (auto it = indicators.cbegin(); it != indicators.cend(); ++it) {
      (*it)->clearRefs(L);
    }
  }

  void build(lv_obj_t *parent)
  {
    meter = parent;
    lv_meter_scale_t *scale = lv_meter_add_scale(meter);

    if (ticks) {
      lv_meter_set_scale_ticks(meter, scale, ticks, tickWidth, tickLen,
                               makeLvColor(colorToRGB(tickColor)));
      if (majorNth)
        lv_meter_set_scale_major_ticks(
            meter, scale, majorNth, majorWidth, majorLen,
            makeLvColor(colorToRGB(majorColor)), labelGap);
    }

    lv_meter_set_scale_range(meter, scale, scaleMin, scaleMax, scaleAngle,
                             scaleRotate);

    if (centerDotSize > 0) {
      lv_obj_set_style_size(meter, centerDotSize, LV_PART_INDICATOR);
      etx_bg_color_from_flags(meter, centerDotColor, LV_PART_INDICATOR);
      lv_obj_set_style_bg_opa(meter, LV_OPA_COVER, LV_PART_INDICATOR);
      lv_obj_set_style_radius(meter, LV_RADIUS_CIRCLE, LV_PART_INDICATOR);
    }

    for (auto it = indicators.cbegin(); it != indicators.cend(); ++it) {
      (*it)->buildIndicator(meter, scale);
    }
  }

 protected:
  lv_obj_t *meter;
  std::vector<LvglWidgetScaleIndicator *> indicators;

  int16_t scaleMin = 0, scaleMax = 100, scaleAngle = 360, scaleRotate = 0;
  uint8_t ticks = 0, majorNth = 0;
  uint8_t tickWidth = 0, majorWidth = 0;
  uint8_t tickLen = 0, majorLen = 0;
  LcdFlags tickColor = COLOR2FLAGS(COLOR_THEME_PRIMARY1);
  LcdFlags majorColor = COLOR2FLAGS(COLOR_THEME_PRIMARY1);
  uint8_t labelGap = 0, centerDotSize = 0;
  LcdFlags centerDotColor = COLOR2FLAGS(COLOR_THEME_PRIMARY1_INDEX);

  void parseParam(lua_State *L, const char *key) override
  {
    if (!strcmp(key, "ticks")) {
      ticks = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "tickWidth")) {
      tickWidth = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "tickLen")) {
      tickLen = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "tickColor")) {
      tickColor = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "majorNth")) {
      majorNth = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "majorWidth")) {
      majorWidth = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "majorLen")) {
      majorLen = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "majorColor")) {
      majorColor = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "labelGap")) {
      labelGap = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "min")) {
      scaleMin = luaL_checkinteger(L, -1);
    } else if (!strcmp(key, "max")) {
      scaleMax = luaL_checkinteger(L, -1);
    } else if (!strcmp(key, "angle")) {
      scaleAngle = luaL_checkinteger(L, -1);
    } else if (!strcmp(key, "rotate")) {
      scaleRotate = luaL_checkinteger(L, -1);
    } else if (!strcmp(key, "dotSize")) {
      centerDotSize = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "dotColor")) {
      centerDotColor = luaL_checkunsigned(L, -1);
    } else if (!strcmp(key, "indicators")) {
      luaL_checktype(L, -1, LUA_TTABLE);
      for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        lua_getfield(L, -1, "type");
        const char *key = lua_tostring(L, -1);
        lua_pop(L, 1);
        if (!strcmp(key, "arc")) {
          indicators.push_back(new LvglWidgetScaleArc(L));
        } else if (!strcmp(key, "lines")) {
          indicators.push_back(new LvglWidgetScaleLines(L));
        } else if (!strcmp(key, "needle")) {
          indicators.push_back(new LvglWidgetScaleNeedle(L));
        }
      }
    }
  }
};

void LvglWidgetMeter::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "scales")) {
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      scales.push_back(new LvglWidgetMeterScale(L));
    }
  } else {
    LvglWidgetRoundObject::parseParam(L, key);
  }
}

bool LvglWidgetMeter::callRefs(lua_State *L)
{
  for (auto it = scales.cbegin(); it != scales.cend(); ++it) {
    if (!(*it)->callRefs(L)) return false;
  }
  return LvglWidgetRoundObject::callRefs(L);
}

void LvglWidgetMeter::clearRefs(lua_State *L)
{
  for (auto it = scales.cbegin(); it != scales.cend(); ++it) {
    (*it)->clearRefs(L);
  }
  LvglWidgetRoundObject::clearRefs(L);
}

void LvglWidgetMeter::build(lua_State *L)
{
  window = new Window(lvglManager->getCurrentParent(),
                      {x - radius, y - radius, radius * 2, radius * 2},
                      lv_meter_create);

  lv_obj_t *meter = window->getLvObj();

  lv_obj_add_flag(meter, LV_OBJ_FLAG_EVENT_BUBBLE);
  lv_obj_clear_flag(meter, LV_OBJ_FLAG_CLICKABLE);

  for (auto it = scales.cbegin(); it != scales.cend(); ++it) {
    (*it)->build(meter);
  }
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
  window = new ConfirmDialog(
      MainWindow::instance(), title, message,
      [=]() { pcallSimpleFunc(L, confirmFunction); },
      [=]() { pcallSimpleFunc(L, cancelFunction); });
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
                          if (setFunction) {
                            int t = lua_gettop(L);
                            PROTECT_LUA()
                            {
                              lua_rawgeti(L, LUA_REGISTRYINDEX, setFunction);
                              lua_pushstring(L, value);
                              bool rv = lua_pcall(L, 1, 0, 0) == 0;
                              if (!rv) {
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
  if (dispFunction) {
    ((NumberEdit *)window)->setDisplayHandler([=](int val) {
      const char *s = "???";
      int t = lua_gettop(L);
      PROTECT_LUA()
      {
        lua_rawgeti(L, LUA_REGISTRYINDEX, dispFunction);
        lua_pushinteger(L, val);
        bool rv = lua_pcall(L, 1, 1, 0) == 0;
        if (rv) {
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

void LvglWidgetChoice::parseParam(lua_State *L, const char *key)
{
  if (!strcmp(key, "title")) {
    title = luaL_checkstring(L, -1);
  } else if (!strcmp(key, "values")) {
    luaL_checktype(L, -1, LUA_TTABLE);
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
      values.push_back(lua_tostring(L, -1));
    }
  } else if (!strcmp(key, "get")) {
    getFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else if (!strcmp(key, "set")) {
    setFunction = luaL_ref(L, LUA_REGISTRYINDEX);
  } else {
    LvglWidgetObject::parseParam(L, key);
  }
}

void LvglWidgetChoice::clearRefs(lua_State *L)
{
  clearRef(L, getFunction);
  clearRef(L, setFunction);
  LvglWidgetObject::clearRefs(L);
}

void LvglWidgetChoice::build(lua_State *L)
{
  if (h == LV_SIZE_CONTENT) h = 0;
  window = new Choice(
      lvglManager->getCurrentParent(), {x, y, w, h}, values, 0,
      values.size() - 1, [=]() { return pcallGetIntVal(L, getFunction) - 1; },
      [=](int val) { pcallSetIntVal(L, setFunction, val + 1); }, title.c_str());
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

    body->padAll(PAD_ZERO);

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
}

//-----------------------------------------------------------------------------
