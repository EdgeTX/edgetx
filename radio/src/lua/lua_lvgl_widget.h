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

struct LvglParamFuncOrValue
{
 public:
  enum ParamType {
    PVALUE,
    PTXT,
  };

  int function;
  union {
    LcdFlags flags;
    coord_t coord;
    const char* txt;
    uint32_t value;
  };

  uint32_t currVal  = -1;

  void parse(lua_State *L, ParamType typ = PVALUE);
  void forceUpdate() { currVal = -1; }
  bool changedColor(LcdFlags newFlags);
  bool changedFont(LcdFlags newFlags) { return changedColor(newFlags);}
  bool changedText(const char* s);
  bool changedValue(uint32_t v);
  void clearRef(lua_State *L);
};

//-----------------------------------------------------------------------------

class LvglGetSetParams
{
 public:
  LvglGetSetParams() {}

 protected:
  int getFunction = LUA_REFNIL;
  int setFunction = LUA_REFNIL;

  bool parseGetSetParam(lua_State *L, const char *key);
  void clearGetSetRefs(lua_State *L);
};

class LvglTextParams
{
 public:
  LvglTextParams() {}

 protected:
  LvglParamFuncOrValue txt = { .function = LUA_REFNIL, .txt = ""};
  LvglParamFuncOrValue font = { .function = LUA_REFNIL, .flags = FONT(STD)};

  bool parseTextParam(lua_State *L, const char *key);
  void clearTextRefs(lua_State *L);
};

class LvglMinMaxParams
{
 public:
  LvglMinMaxParams(int min, int max) : min(min), max(max) {}

 protected:
  int min;
  int max;

  bool parseMinMaxParam(lua_State *L, const char *key);
};

class LvglTitleParam
{
 public:
  LvglTitleParam() {}

 protected:
  std::string title;

  bool parseTitleParam(lua_State *L, const char *key);
};

class LvglMessageParam
{
 public:
  LvglMessageParam() {}

 protected:
  std::string message;

  bool parseMessageParam(lua_State *L, const char *key);
};

class LvglRoundedParam
{
 public:
  LvglRoundedParam() {}

 protected:
  bool rounded = false;

  bool parseRoundedParam(lua_State *L, const char *key);
};

class LvglThicknessParam
{
 public:
  LvglThicknessParam() {}

 protected:
  coord_t thickness = 1;

  bool parseThicknessParam(lua_State *L, const char *key);
};

class LvglValuesParam
{
 public:
  LvglValuesParam() {}

 protected:
  std::vector<std::string> values;

  bool parseValuesParam(lua_State *L, const char *key);
};

class LvglScrollableParams
{
 public:
  LvglScrollableParams() {}

 protected:
  bool showScrollBar = true;
  lv_dir_t scrollDir = LV_DIR_ALL;
  int scrollToFunction = LUA_REFNIL;
  int scrolledFunction = LUA_REFNIL;

  bool parseScrollableParam(lua_State *L, const char *key);
};

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

  virtual coord_t getScrollX() { return 0; }
  virtual coord_t getScrollY() { return 0; }

  virtual void setColor(LcdFlags newColor) {}
  virtual void setOpacity(uint8_t newOpa) {}
  virtual void setPos(coord_t x, coord_t y) {}
  virtual void setSize(coord_t w, coord_t h) {}

  void create(lua_State *L, int index);
  void update(lua_State *L);

  virtual Window *getWindow() const = 0;

  static LvglWidgetObjectBase *checkLvgl(lua_State *L, int index, bool required = false);

  void clear();

 protected:
  int luaRef = LUA_REFNIL;
  std::vector<int> lvglObjectRefs;
  const char* metatable = nullptr;
  bool clearRequest = false;
  LuaScriptManager *lvglManager = nullptr;
  coord_t x = 0, y = 0, w = LV_SIZE_CONTENT, h = LV_SIZE_CONTENT;
  int getVisibleFunction = LUA_REFNIL;
  int getSizeFunction = LUA_REFNIL;
  int getPosFunction = LUA_REFNIL;
  LvglParamFuncOrValue color = { .function = LUA_REFNIL, .flags = COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX)};
  LvglParamFuncOrValue opacity = { .function = LUA_REFNIL, .value = LV_OPA_COVER};

  virtual void build(lua_State *L);
  virtual void refresh();

  virtual void parseParam(lua_State *L, const char *key);

  void getParams(lua_State *L, int index);

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
  const char* pcallGetStringVal(lua_State *L, int getFuncRef);
  void pcallSetStringVal(lua_State *L, int setFuncRef, const char* val);
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

class LvglWidgetLabel : public LvglSimpleWidgetObject, public LvglTextParams
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
  LvglParamFuncOrValue align = { .function = LUA_REFNIL, .flags = LEFT};

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setText(txt.txt);
    setFont(font.flags);
    setAlign(align.flags);
    LvglSimpleWidgetObject::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetLineBase : public LvglSimpleWidgetObject, public LvglRoundedParam
{
 public:
  LvglWidgetLineBase() : LvglSimpleWidgetObject() {}

  void setColor(LcdFlags newColor) override;
  void setOpacity(uint8_t newOpa) override;
  void setPos(coord_t x, coord_t y) override;
  void setSize(coord_t w, coord_t h) override;

 protected:
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

class LvglWidgetLine : public LvglSimpleWidgetObject, public LvglRoundedParam, public LvglThicknessParam
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
  size_t ptCnt = 0;
  size_t ptAlloc = 0;
  lv_point_t* pts = nullptr;
  uint32_t ptsHash = -1;
  lv_obj_t* parent = nullptr;
  int getPointsFunction = LUA_REFNIL;

  void setLine();

  uint32_t getPts(lua_State* L);
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

class LvglWidgetBox : public LvglWidgetObject, public LvglScrollableParams
{
 public:
  LvglWidgetBox() : LvglWidgetObject() {}

  coord_t getScrollX() override;
  coord_t getScrollY() override;

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetSetting : public LvglWidgetObject, public LvglTitleParam
{
 public:
  LvglWidgetSetting() : LvglWidgetObject() {}

 protected:

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetBorderedObject : public LvglWidgetBox, public LvglThicknessParam
{
 public:
  LvglWidgetBorderedObject() : LvglWidgetBox() {}

  void setColor(LcdFlags newColor) override;
  void setOpacity(uint8_t newOpa) override;

 protected:
  bool filled = false;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetRoundObject : public LvglWidgetBorderedObject
{
 public:
  LvglWidgetRoundObject() : LvglWidgetBorderedObject()
  {
    scrollDir = LV_DIR_NONE;
  }

  void setPos(coord_t x, coord_t y) override;
  void setRadius(coord_t r);

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  LvglParamFuncOrValue radius = { .function = LUA_REFNIL, .coord = 0};

  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setRadius(radius.coord);
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

class LvglWidgetArc : public LvglWidgetRoundObject, public LvglRoundedParam
{
 public:
  LvglWidgetArc() : LvglWidgetRoundObject() {}

  void setColor(LcdFlags newColor) override;
  void setBgColor(LcdFlags newColor);
  void setOpacity(uint8_t newOpa) override;
  void setBgOpacity(uint8_t newOpa);
  void setStartAngle(coord_t angle);
  void setEndAngle(coord_t angle);
  void setBgStartAngle(coord_t angle);
  void setBgEndAngle(coord_t angle);

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  LvglParamFuncOrValue startAngle = { .function = LUA_REFNIL, .coord = 0};
  LvglParamFuncOrValue endAngle = { .function = LUA_REFNIL, .coord = 360};
  LvglParamFuncOrValue bgColor = { .function = LUA_REFNIL, .flags = (LcdFlags)-1};
  LvglParamFuncOrValue bgOpacity = { .function = LUA_REFNIL, .value = LV_OPA_TRANSP};
  LvglParamFuncOrValue bgStartAngle = { .function = LUA_REFNIL, .coord = 0};
  LvglParamFuncOrValue bgEndAngle = { .function = LUA_REFNIL, .coord = 360};

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setStartAngle(startAngle.coord);
    setEndAngle(endAngle.coord);
    setBgStartAngle(bgStartAngle.coord);
    setBgEndAngle(bgEndAngle.coord);
    setBgColor(bgColor.flags);
    setBgOpacity(bgOpacity.value);
    LvglWidgetRoundObject::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetImage : public LvglWidgetObject
{
 public:
  LvglWidgetImage() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  LvglParamFuncOrValue filename = { .function = LUA_REFNIL, .txt = ""};
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

class LvglWidgetTextButtonBase : public LvglWidgetObject, public LvglTextParams
{
 public:
  LvglWidgetTextButtonBase() : LvglWidgetObject(LVGL_SIMPLEMETATABLE)
  {
    color.flags = -1; // ignore unless overridden
  }

  void setText(const char *s);
  void setFont(LcdFlags font);
  void setSize(coord_t w, coord_t h) override;
  void setColor(LcdFlags newColor) override;
  void setTextColor(LcdFlags newColor);
  void setRounded();

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  coord_t rounded = -1;
  LvglParamFuncOrValue textColor = { .function = LUA_REFNIL, .flags = (LcdFlags)-1};
  int pressFunction = LUA_REFNIL;

  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setText(txt.txt);
    setFont(font.flags);
    setTextColor(textColor.flags);
    LvglWidgetObject::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetTextButton : public LvglWidgetTextButtonBase
{
 public:
  LvglWidgetTextButton() : LvglWidgetTextButtonBase() {}

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
    LvglWidgetTextButtonBase::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetMomentaryButton : public LvglWidgetTextButtonBase
{
 public:
  LvglWidgetMomentaryButton() : LvglWidgetTextButtonBase() {}

  void clearRefs(lua_State *L) override;

 protected:
  int releaseFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetToggleSwitch : public LvglWidgetObject, public LvglGetSetParams
{
 public:
  LvglWidgetToggleSwitch() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  void clearRefs(lua_State *L) override;

 protected:
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

class LvglWidgetNumberEdit : public LvglWidgetObject, public LvglGetSetParams, public LvglMinMaxParams
{
 public:
  LvglWidgetNumberEdit() : LvglWidgetObject(LVGL_SIMPLEMETATABLE), LvglMinMaxParams(-1024, 1024) {}

  void clearRefs(lua_State *L) override;

 protected:
  int dispFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetSliderBase : public LvglWidgetObject, public LvglGetSetParams, public LvglMinMaxParams
{
 public:
  LvglWidgetSliderBase() : LvglWidgetObject(LVGL_SIMPLEMETATABLE), LvglMinMaxParams(0, 100)
  {
    color.flags = -1;
  }

  void setColor(LcdFlags newColor) override;

  void clearRefs(lua_State *L) override;

 protected:
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

class LvglWidgetPage : public LvglWidgetObject, public LvglTitleParam, public LvglScrollableParams
{
 public:
  LvglWidgetPage() : LvglWidgetObject() {}

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

  coord_t getScrollX() override;
  coord_t getScrollY() override;

 protected:
  std::string subtitle;
  std::string iconFile;

  int backActionFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetDialog : public LvglWidgetObject, public LvglTitleParam
{
 public:
  LvglWidgetDialog() : LvglWidgetObject() {}

  void clearRefs(lua_State *L) override;
  void close() override;

 protected:
  LvglDialog* dialog = nullptr;

  int closeFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetConfirmDialog : public LvglWidgetObject, public LvglTitleParam, public LvglMessageParam
{
 public:
  LvglWidgetConfirmDialog() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  void clearRefs(lua_State *L) override;

 protected:
  int confirmFunction = LUA_REFNIL;
  int cancelFunction = LUA_REFNIL;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetMessageDialog : public LvglWidgetObject, public LvglTitleParam, public LvglMessageParam
{
 public:
  LvglWidgetMessageDialog() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

 protected:
  const char *details = nullptr;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetPicker : public LvglWidgetObject, public LvglGetSetParams
{
 public:
  LvglWidgetPicker() : LvglWidgetObject(LVGL_SIMPLEMETATABLE) {}

  void clearRefs(lua_State *L) override;

 protected:
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetChoice : public LvglWidgetPicker, public LvglTitleParam, public LvglValuesParam
{
 public:
  LvglWidgetChoice() : LvglWidgetPicker() {}

  void clearRefs(lua_State *L) override;

 protected:
  int filterFunction = LUA_REFNIL;
  coord_t popupWidth = 0;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetMenu : public LvglWidgetPicker, public LvglTitleParam, public LvglValuesParam
{
 public:
  LvglWidgetMenu() : LvglWidgetPicker() {}

 protected:
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

class LvglWidgetFilePicker : public LvglWidgetPicker, public LvglTitleParam
{
 public:
   LvglWidgetFilePicker() : LvglWidgetPicker() {}

 protected:
  const char* folder = nullptr;
  const char* extension = nullptr;
  int maxLen = 255;
  bool hideExtension = false;

  void build(lua_State *L) override;
  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------
