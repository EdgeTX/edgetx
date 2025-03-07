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

#pragma once

#define LVGL_METATABLE "LVGL*"
#define LVGL_SIMPLEMETATABLE "LVGLSIMPLE*"

class LuaScriptManager;
class LvglDialog;

//-----------------------------------------------------------------------------

class LvglWidgetObjectBase
{
 public:
  LvglWidgetObjectBase(const char* meta);
  virtual ~LvglWidgetObjectBase() {}

  int getRef(lua_State *L);
  void push(lua_State *L);
  void saveLvglObjectRef(int ref);

  virtual bool callRefs(lua_State *L);
  virtual void clearRefs(lua_State *L);
  void clearChildRefs(lua_State *L);

  virtual void show() = 0;
  virtual void hide() = 0;
  virtual void enable() {};
  virtual void disable() {};
  virtual void close() {};

  virtual void setColor(LcdFlags newColor) {}
  virtual void setOpacity(uint8_t newOpa) {}
  virtual void setPos(coord_t x, coord_t y) {}
  virtual void setSize(coord_t w, coord_t h) {}

  void create(lua_State *L, int index);
  void update(lua_State *L);

  virtual Window *getWindow() const = 0;

  static LvglWidgetObjectBase *checkLvgl(lua_State *L, int index);

  void clear() { clearRequest = true; }

 protected:
  int luaRef = LUA_REFNIL;
  std::vector<int> lvglObjectRefs;
  const char* metatable = nullptr;
  bool clearRequest = false;
  LuaScriptManager *lvglManager = nullptr;
  coord_t x = 0, y = 0, w = LV_SIZE_CONTENT, h = LV_SIZE_CONTENT;
  LcdFlags color = COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX);
  LcdFlags currentColor = -1;
  uint8_t opacity = LV_OPA_COVER;
  int getColorFunction = LUA_REFNIL;
  int getOpacityFunction = LUA_REFNIL;
  int getVisibleFunction = LUA_REFNIL;
  int getSizeFunction = LUA_REFNIL;
  int getPosFunction = LUA_REFNIL;

  virtual void build(lua_State *L);
  virtual void refresh();

  virtual void parseParam(lua_State *L, const char *key);

  void getParams(lua_State *L, int index);

  bool colorChanged(LcdFlags newColor);

  void clearRef(lua_State *L, int& ref);

  void pcallSimpleFunc(lua_State *L, int funcRef);
  bool pcallUpdateBool(lua_State *L, int getFuncRef,
                       std::function<void(bool)> update);
  bool pcallUpdate1Int(lua_State *L, int getFuncRef,
                       std::function<void(int)> update);
  bool pcallUpdate2Int(lua_State *L, int getFuncRef,
                       std::function<void(int, int)> update);
  int pcallGetIntVal(lua_State *L, int getFuncRef);
  int pcallGetOptIntVal(lua_State *L, int getFuncRef, int defVal);
  void pcallSetIntVal(lua_State *L, int setFuncRef, int val);
};

//-----------------------------------------------------------------------------

class LvglSimpleWidgetObject : public LvglWidgetObjectBase
{
 public:
  LvglSimpleWidgetObject() : LvglWidgetObjectBase(LVGL_SIMPLEMETATABLE) {}

  void show() override;
  void hide() override;

  void setPos(coord_t x, coord_t y) override;
  void setSize(coord_t w, coord_t h) override;

  Window *getWindow() const override { return nullptr; }

 protected:
  lv_obj_t* lvobj = nullptr;
};

//-----------------------------------------------------------------------------

class LvglWidgetLabel : public LvglSimpleWidgetObject
{
 public:
  LvglWidgetLabel() : LvglSimpleWidgetObject() {}

  void setText(const char *s);
  void setColor(LcdFlags newColor) override;
  void setFont(LcdFlags font);
  void setAlign(LcdFlags align);

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  uint32_t textHash = -1;

  const char *txt = "";
  LcdFlags font = FONT(STD);
  LcdFlags align = LEFT;
  int getTextFunction = LUA_REFNIL;
  int getFontFunction = LUA_REFNIL;
  int getAlignFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setText(txt);
    setFont(font);
    setAlign(align);
    LvglSimpleWidgetObject::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetLineBase : public LvglSimpleWidgetObject
{
 public:
  LvglWidgetLineBase() : LvglSimpleWidgetObject() {}

  void setColor(LcdFlags newColor) override;
  void setOpacity(uint8_t newOpa) override;
  void setPos(coord_t x, coord_t y) override;
  void setSize(coord_t w, coord_t h) override;

 protected:
  bool rounded = false;
  int dashGap = 0;
  int dashWidth = 0;
  lv_point_t pts[2];

  virtual void setLine() = 0;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
  void refresh() override;
};

//-----------------------------------------------------------------------------

class LvglWidgetHLine : public LvglWidgetLineBase
{
 public:
  LvglWidgetHLine() : LvglWidgetLineBase() {}

 protected:
  void setLine() override;
};

//-----------------------------------------------------------------------------

class LvglWidgetVLine : public LvglWidgetLineBase
{
 public:
  LvglWidgetVLine() : LvglWidgetLineBase() {}

 protected:
  void setLine() override;
};

//-----------------------------------------------------------------------------

class LvglWidgetLine : public LvglSimpleWidgetObject
{
 public:
  LvglWidgetLine();
  ~LvglWidgetLine();

  void setColor(LcdFlags newColor) override;
  void setOpacity(uint8_t newOpa) override;
  void setPos(coord_t x, coord_t y) override;
  void setSize(coord_t w, coord_t h) override;

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  coord_t thickness = 1;
  bool rounded = false;
  size_t ptCnt = 0;
  lv_point_t* pts = nullptr;
  uint32_t ptsHash = -1;
  lv_obj_t* parent = nullptr;
  int getPointsFunction = LUA_REFNIL;

  void setLine();

  void getPt(lua_State* L, int n);
  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
  void refresh() override;
};

//-----------------------------------------------------------------------------

class LvglWidgetTriangle : public LvglSimpleWidgetObject
{
 public:
  LvglWidgetTriangle();
  ~LvglWidgetTriangle();

  void setColor(LcdFlags newColor) override;
  void setSize(coord_t w, coord_t h) override;

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  lv_point_t pts[3] = {0};
  uint32_t ptsHash = -1;
  MaskBitmap* mask = nullptr;
  lv_obj_t* parent = nullptr;
  int getPointsFunction = LUA_REFNIL;

  void fillTriangle();
  void fillLine(coord_t x1, coord_t x2, coord_t y);

  void getPt(lua_State* L, int n);
  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
  void refresh() override;
};

//-----------------------------------------------------------------------------

class LvglWidgetObject : public LvglWidgetObjectBase
{
 public:
  LvglWidgetObject(const char* meta = LVGL_METATABLE) : LvglWidgetObjectBase(meta) {}

  void show() override { window->show(); }
  void hide() override { window->hide(); }
  void enable() override { window->enable(); }
  void disable() override { window->disable(); }

  void setPos(coord_t x, coord_t y) override;
  void setSize(coord_t w, coord_t h) override;

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

  Window *getWindow() const override { return window; }

 protected:
  Window *window = nullptr;
  int8_t flexFlow = -1;
  int8_t flexPad = PAD_TINY;
  int getActiveFunction = LUA_REFNIL;

  void parseParam(lua_State *L, const char *key) override;

  bool setFlex();
};

//-----------------------------------------------------------------------------

class LvglWidgetBox : public LvglWidgetObject
{
 public:
  LvglWidgetBox() : LvglWidgetObject() {}

 protected:
  void build(lua_State *L) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetSetting : public LvglWidgetObject
{
 public:
  LvglWidgetSetting() : LvglWidgetObject() {}

 protected:
  const char *txt = "";

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetBorderedObject : public LvglWidgetObject
{
 public:
  LvglWidgetBorderedObject() : LvglWidgetObject() {}

  void setColor(LcdFlags newColor) override;
  void setOpacity(uint8_t newOpa) override;

 protected:
  coord_t thickness = 1;
  bool filled = false;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetRoundObject : public LvglWidgetBorderedObject
{
 public:
  LvglWidgetRoundObject() : LvglWidgetBorderedObject() {}

  void setPos(coord_t x, coord_t y) override;
  void setRadius(coord_t r);

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  coord_t radius = 0;

  int getRadiusFunction = LUA_REFNIL;

  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setRadius(radius);
    LvglWidgetObject::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetRectangle : public LvglWidgetBorderedObject
{
 public:
  LvglWidgetRectangle() : LvglWidgetBorderedObject() {}

 protected:
  coord_t rounded = 0;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetCircle : public LvglWidgetRoundObject
{
 public:
  LvglWidgetCircle() : LvglWidgetRoundObject() {}

 protected:
  void build(lua_State *L) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetArc : public LvglWidgetRoundObject
{
 public:
  LvglWidgetArc() : LvglWidgetRoundObject() {}

  void setColor(LcdFlags newColor) override;
  void setOpacity(uint8_t newOpa) override;
  void setStartAngle(coord_t angle);
  void setEndAngle(coord_t angle);

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  coord_t startAngle = 0, endAngle = 0;

  int getStartAngleFunction = LUA_REFNIL;
  int getEndAngleFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setStartAngle(startAngle);
    setEndAngle(endAngle);
    LvglWidgetRoundObject::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetImage : public LvglWidgetObject
{
 public:
  LvglWidgetImage() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

 protected:
  std::string filename;
  bool fillFrame = false;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetQRCode : public LvglWidgetObject
{
 public:
  LvglWidgetQRCode() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

 protected:
  std::string data;
  LcdFlags bgColor = COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX);

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetTextButtonBase : public LvglWidgetObject
{
 public:
  LvglWidgetTextButtonBase() : LvglWidgetObject(LVGL_SIMPLEMETATABLE)
  {
    color = -1; // ignore unless overridden
  }

  void setText(const char *s);
  void setSize(coord_t w, coord_t h) override;
  void setColor(LcdFlags newColor) override;
  void setRounded();

  void clearRefs(lua_State *L) override;

 protected:
  uint32_t textHash = -1;

  const char *txt = "";
  LcdFlags font = FONT(STD);
  coord_t rounded = -1;
  LcdFlags textColor = -1;
  LcdFlags currentTextColor = -1;
  int pressFunction = LUA_REFNIL;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetTextButton : public LvglWidgetTextButtonBase
{
 public:
  LvglWidgetTextButton() : LvglWidgetTextButtonBase() {}

  void setFont(LcdFlags font);
  void setChecked(bool check);

  void clearRefs(lua_State *L) override;

 protected:
  bool checked = false;
  int longPressFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setChecked(checked);
    LvglWidgetObject::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetMomentaryButton : public LvglWidgetTextButtonBase
{
 public:
  LvglWidgetMomentaryButton() : LvglWidgetTextButtonBase() {}

  void setFont(LcdFlags font);

  void clearRefs(lua_State *L) override;

 protected:
  int releaseFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetToggleSwitch : public LvglWidgetObject
{
 public:
  LvglWidgetToggleSwitch() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  void clearRefs(lua_State *L) override;

 protected:
  int getStateFunction = LUA_REFNIL;
  int setStateFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetTextEdit : public LvglWidgetObject
{
 public:
  LvglWidgetTextEdit() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  void clearRefs(lua_State *L) override;

 protected:
  const char *txt = "";
  char value[129];
  int maxLen = 32;

  int setFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetNumberEdit : public LvglWidgetObject
{
 public:
  LvglWidgetNumberEdit() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  void clearRefs(lua_State *L) override;

 protected:
  int min = -1024, max = 1024;

  int getFunction = LUA_REFNIL;
  int setFunction = LUA_REFNIL;
  int dispFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetSliderBase : public LvglWidgetObject
{
 public:
  LvglWidgetSliderBase() : LvglWidgetObject(LVGL_SIMPLEMETATABLE)
  {
    color = -1;
  }

  void setColor(LcdFlags newColor) override;

  void clearRefs(lua_State *L) override;

 protected:
  int32_t vmin = 0;
  int32_t vmax = 100;
  int getValueFunction = LUA_REFNIL;
  int setValueFunction = LUA_REFNIL;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetSlider : public LvglWidgetSliderBase
{
 public:
  LvglWidgetSlider() : LvglWidgetSliderBase() {}

 protected:
  void build(lua_State *L) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetVerticalSlider : public LvglWidgetSliderBase
{
 public:
  LvglWidgetVerticalSlider() : LvglWidgetSliderBase() {}

 protected:
  void build(lua_State *L) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetPage : public LvglWidgetObject
{
 public:
  LvglWidgetPage() : LvglWidgetObject() {}

  void clearRefs(lua_State *L) override;

 protected:
  std::string title;
  std::string subtitle;
  std::string iconFile;

  int backActionFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetDialog : public LvglWidgetObject
{
 public:
  LvglWidgetDialog() : LvglWidgetObject() {}

  void clearRefs(lua_State *L) override;
  void close() override;

 protected:
  const char *title = nullptr;
  LvglDialog* dialog = nullptr;

  int closeFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetConfirmDialog : public LvglWidgetObject
{
 public:
  LvglWidgetConfirmDialog() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  void clearRefs(lua_State *L) override;

 protected:
  const char *title = nullptr;
  const char *message = nullptr;

  int confirmFunction = LUA_REFNIL;
  int cancelFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetMessageDialog : public LvglWidgetObject
{
 public:
  LvglWidgetMessageDialog() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

 protected:
  const char *title = nullptr;
  const char *message = nullptr;
  const char *details = nullptr;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetPicker : public LvglWidgetObject
{
 public:
  LvglWidgetPicker() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  void clearRefs(lua_State *L) override;

 protected:
  int getFunction = LUA_REFNIL;
  int setFunction = LUA_REFNIL;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetChoice : public LvglWidgetPicker
{
 public:
  LvglWidgetChoice() : LvglWidgetPicker() {}

 protected:
  std::string title;
  std::vector<std::string> values;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetFontPicker : public LvglWidgetPicker
{
 public:
  LvglWidgetFontPicker() : LvglWidgetPicker() {}

 protected:
  void build(lua_State *L) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetAlignPicker : public LvglWidgetPicker
{
 public:
  LvglWidgetAlignPicker() : LvglWidgetPicker() {}

 protected:
  void build(lua_State *L) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetColorPicker : public LvglWidgetPicker
{
 public:
  LvglWidgetColorPicker() : LvglWidgetPicker() {}

 protected:
  void build(lua_State *L) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetTimerPicker : public LvglWidgetPicker
{
 public:
  LvglWidgetTimerPicker() : LvglWidgetPicker() {}

 protected:
  void build(lua_State *L) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetSwitchPicker : public LvglWidgetPicker
{
 public:
  LvglWidgetSwitchPicker() : LvglWidgetPicker() {}

 protected:
  uint32_t filter = 0xFFFFFFFF;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetSourcePicker : public LvglWidgetPicker
{
 public:
  LvglWidgetSourcePicker() : LvglWidgetPicker() {}

 protected:
  uint32_t filter = 0xFFFFFFFF;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------
