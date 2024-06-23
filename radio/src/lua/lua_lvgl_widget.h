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

//-----------------------------------------------------------------------------

class LvglWidgetObjectBase
{
 public:
  LvglWidgetObjectBase() { lvglManager = luaLvglManager; }
  virtual ~LvglWidgetObjectBase() {}

  void getParams(lua_State *L, int index);

  virtual void build(lua_State *L);
  virtual bool callRefs(lua_State *L) = 0;
  virtual void clearRefs(lua_State *L) = 0;

 protected:
  LuaLvglManager *lvglManager = nullptr;

  virtual void parseParam(lua_State *L, const char *key) = 0;

  void clearRef(lua_State *L, int ref);

  bool pcallFunc(lua_State *L, int getFuncRef, int nret);
  void pcallSimpleFunc(lua_State *L, int funcRef);
  bool pcallUpdateBool(lua_State *L, int getFuncRef,
                       std::function<void(bool)> update);
  bool pcallUpdate1Int(lua_State *L, int getFuncRef,
                       std::function<void(int)> update);
  bool pcallUpdate2Int(lua_State *L, int getFuncRef,
                       std::function<void(int, int)> update);
  int pcallGetIntVal(lua_State *L, int getFuncRef);
  void pcallSetIntVal(lua_State *L, int setFuncRef, int val);
};

//-----------------------------------------------------------------------------

class LvglWidgetObject : public LvglWidgetObjectBase
{
 public:
  LvglWidgetObject() : LvglWidgetObjectBase() {}

  int getRef(lua_State *L);
  void push(lua_State *L);

  void update(lua_State *L);

  virtual void setColor(LcdFlags color) {}
  virtual void setPos(coord_t x, coord_t y);
  void setSize(coord_t w, coord_t h);

  void show() { window->show(); }
  void hide() { window->hide(); }

  Window *getWindow() const { return window; }

  static LvglWidgetObject *checkLvgl(lua_State *L, int index);

  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  Window *window = nullptr;
  LcdFlags currentColor = -1;

  coord_t x = 0, y = 0, w = LV_SIZE_CONTENT, h = LV_SIZE_CONTENT;
  LcdFlags color = COLOR2FLAGS(COLOR_THEME_SECONDARY1_INDEX);
  int getColorFunction = 0;
  int getVisibleFunction = 0;
  int getSizeFunction = 0;
  int getPosFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
  virtual void refresh();
};

//-----------------------------------------------------------------------------

class LvglWidgetLabel : public LvglWidgetObject
{
 public:
  LvglWidgetLabel() : LvglWidgetObject() {}

  void setText(const char *s);
  void setColor(LcdFlags color) override;
  void setFont(LcdFlags font);

  void build(lua_State *L) override;
  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  uint32_t textHash = -1;

  const char *txt = "";
  LcdFlags font = FONT(STD);
  int getTextFunction = 0;
  int getFontFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
  void refresh() override
  {
    setText(txt);
    setFont(font);
    LvglWidgetObject::refresh();
  }
};

//-----------------------------------------------------------------------------

class LvglWidgetBorderedObject : public LvglWidgetObject
{
 public:
  LvglWidgetBorderedObject() : LvglWidgetObject() {}

  void setColor(LcdFlags color) override;

  void build(lua_State *L) override;

 protected:
  coord_t thickness = 1;
  bool filled = false;

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

  int getRadiusFunction = 0;

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

  void build(lua_State *L) override;

 protected:
  coord_t rounded = 0;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetCircle : public LvglWidgetRoundObject
{
 public:
  LvglWidgetCircle() : LvglWidgetRoundObject() {}

  void build(lua_State *L) override;

 protected:
};

//-----------------------------------------------------------------------------

class LvglWidgetArc : public LvglWidgetRoundObject
{
 public:
  LvglWidgetArc() : LvglWidgetRoundObject() {}

  void setColor(LcdFlags color) override;
  void setStartAngle(coord_t angle);
  void setEndAngle(coord_t angle);

  void build(lua_State *L) override;
  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  coord_t startAngle = 0, endAngle = 0;

  int getStartAngleFunction = 0;
  int getEndAngleFunction = 0;

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
  LvglWidgetImage() : LvglWidgetObject() {}

  void build(lua_State *L) override;

 protected:
  std::string filename;
  bool fillFrame = false;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetQRCode : public LvglWidgetObject
{
 public:
  LvglWidgetQRCode() : LvglWidgetObject() {}

  void build(lua_State *L) override;

 protected:
  std::string data;
  LcdFlags bgColor = COLOR2FLAGS(COLOR_THEME_SECONDARY3_INDEX);

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetMeterScale;

class LvglWidgetMeter : public LvglWidgetRoundObject
{
 public:
  LvglWidgetMeter() : LvglWidgetRoundObject() {}

  void build(lua_State *L) override;
  bool callRefs(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  std::vector<LvglWidgetMeterScale*> scales;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetTextButton : public LvglWidgetObject
{
 public:
  LvglWidgetTextButton() : LvglWidgetObject() {}

  void setText(const char *s);

  void build(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  uint32_t textHash = -1;

  const char *txt = "";
  int pressFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetToggleSwitch : public LvglWidgetObject
{
 public:
  LvglWidgetToggleSwitch() : LvglWidgetObject() {}

  void build(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  int getStateFunction = 0;
  int setStateFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetConfirmDialog : public LvglWidgetObject
{
 public:
  LvglWidgetConfirmDialog() : LvglWidgetObject() {}

  void build(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  const char *title = nullptr;
  const char *message = nullptr;

  int confirmFunction = 0;
  int cancelFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetTextEdit : public LvglWidgetObject
{
 public:
  LvglWidgetTextEdit() : LvglWidgetObject() {}

  void build(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  const char *txt = "";
  char value[129];
  int maxLen = 32;

  int setFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetNumberEdit : public LvglWidgetObject
{
 public:
  LvglWidgetNumberEdit() : LvglWidgetObject() {}

  void build(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  int min = -1024, max = 1024;

  int getFunction = 0;
  int setFunction = 0;
  int dispFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetChoice : public LvglWidgetObject
{
 public:
  LvglWidgetChoice() : LvglWidgetObject() {}

  void build(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  std::string title;
  std::vector<std::string> values;

  int getFunction = 0;
  int setFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetSlider : public LvglWidgetObject
{
 public:
  LvglWidgetSlider() : LvglWidgetObject() {}

  void build(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  int32_t vmin = 0;
  int32_t vmax = 100;
  int getValueFunction = 0;
  int setValueFunction = 0;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------

class LvglWidgetPage : public LvglWidgetObject
{
 public:
  LvglWidgetPage() : LvglWidgetObject() {}

  void build(lua_State *L) override;
  void clearRefs(lua_State *L) override;

 protected:
  int backActionFunction = 0;
  std::string title;
  std::string subtitle;
  std::string iconFile;

  void parseParam(lua_State *L, const char *key) override;
};

//-----------------------------------------------------------------------------
