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

#include "libopenui.h"
#include "list_line_button.h"
#include "edgetx.h"
#include "page.h"
#include "tabsgroup.h"

struct CustomFunctionData;
class FunctionEditPage;
class FunctionLineButton;

//-----------------------------------------------------------------------------

class FunctionLineButton : public ListLineButton
{
 public:
  FunctionLineButton(Window *parent, const rect_t &rect,
                     const CustomFunctionData *cfn, uint8_t index,
                     const char *prefix);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "FunctionButton"; }
#endif

  static void on_draw(lv_event_t *e);

  void delayed_init();

  void refresh() override;

  static constexpr coord_t NM_X = PAD_TINY;
  static LAYOUT_VAL(NM_Y, 4, 10)
  static LAYOUT_VAL(NM_W, 43, 40)
  static LAYOUT_VAL(NM_H, 20, 20)
  static constexpr coord_t SW_X = NM_X + NM_W + PAD_TINY;
  static LAYOUT_VAL(SW_Y, NM_Y, 0)
  static LAYOUT_VAL(SW_W, 70, 198)
  static constexpr coord_t SW_H = NM_H;
  static LAYOUT_VAL(FN_X, SW_X + SW_W + PAD_TINY, NM_X + NM_W + PAD_TINY)
  static LAYOUT_VAL(FN_Y, NM_Y, 20)
  static LAYOUT_VAL(FN_W, 278, SW_W)
  static LAYOUT_VAL(RP_W, 40, 34)
  static constexpr coord_t FN_H = NM_H;
  static constexpr coord_t RP_X = FN_X + FN_W + PAD_TINY;
  static constexpr coord_t RP_Y = NM_Y;
  static constexpr coord_t RP_H = NM_H;
  static constexpr coord_t EN_X = RP_X + RP_W + PAD_TINY;
  static constexpr coord_t EN_Y = NM_Y + PAD_TINY_GAP;
  static LAYOUT_VAL(EN_SZ, 16, 16)

 protected:
  bool init = false;
  const CustomFunctionData *cfn;
  const char *prefix;

  lv_obj_t *sfName = nullptr;
  lv_obj_t *sfSwitch = nullptr;
  lv_obj_t *sfFunc = nullptr;
  lv_obj_t *sfRepeat = nullptr;
  lv_obj_t *sfEnable = nullptr;

  virtual bool isActive() const override = 0;
};

//-----------------------------------------------------------------------------

class FunctionEditPage : public Page
{
 public:
  FunctionEditPage(uint8_t index, EdgeTxIcon icon, const char *title,
                   const char *prefix);

  static void on_draw(lv_event_t *e);

  void delayed_init();

 protected:
  bool init = false;
  uint8_t index;
  Window *specialFunctionOneWindow = nullptr;
  StaticText *headerSF = nullptr;
  bool active = false;

  virtual bool isActive() const = 0;
  virtual bool isSwitchAvailable(int value) const = 0;
  virtual CustomFunctionData *customFunctionData() const = 0;
  virtual bool isAssignableFunctionAvailable(int function) const = 0;
  virtual void setDirty() const = 0;

  void checkEvents() override;

  void buildHeader(Window *window, const char *title, const char *prefix);

  void addSourceChoice(FormLine *line, const char *title,
                       CustomFunctionData *cfn, int16_t vmax);

  NumberEdit *addNumberEdit(FormLine *line, const char *title,
                            CustomFunctionData *cfn, int16_t vmin, int16_t vmax);

  void updateSpecialFunctionOneWindow();

  void buildBody(Window *form);
};

//-----------------------------------------------------------------------------

class FunctionsPage : public PageTab
{
 public:
  FunctionsPage(CustomFunctionData* functions, const char* title,
                const char* prefix, EdgeTxIcon icon);

  void build(Window* window) override;

  static LAYOUT_VAL(SF_BUTTON_H, 32, 44)

 protected:
  int8_t focusIndex = -1;
  int8_t prevFocusIndex = -1;
  bool isRebuilding = false;
  CustomFunctionData* functions;
  ButtonBase* addButton = nullptr;
  const char* title = nullptr;
  const char* prefix = nullptr;

  void rebuild(Window* window);
  void newSF(Window* window, bool pasteSF);
  void editSpecialFunction(Window* window, uint8_t index,
                           ButtonBase* button);
  void pasteSpecialFunction(Window* window, uint8_t index,
                            ButtonBase* button);
  void plusPopup(Window* window);

  virtual CustomFunctionData* customFunctionData(uint8_t index) const = 0;
  virtual FunctionEditPage* editPage(uint8_t index) const = 0;
  virtual FunctionLineButton* functionButton(Window* parent, const rect_t& rect,
                                         uint8_t index) const = 0;
  virtual void setDirty() const = 0;
};

//-----------------------------------------------------------------------------

class SpecialFunctionsPage : public FunctionsPage
{
 public:
  SpecialFunctionsPage();

  bool isVisible() const override { return modelSFEnabled(); }

 protected:
  CustomFunctionData* customFunctionData(uint8_t index) const override;
  FunctionEditPage* editPage(uint8_t index) const override;
  FunctionLineButton* functionButton(Window* parent, const rect_t& rect,
                                 uint8_t index) const override;
  void setDirty() const override;
};

//-----------------------------------------------------------------------------

class GlobalFunctionsPage : public FunctionsPage
{
 public:
  GlobalFunctionsPage();

  bool isVisible() const override { return radioGFEnabled(); }

 protected:
  CustomFunctionData* customFunctionData(uint8_t index) const override;
  FunctionEditPage* editPage(uint8_t index) const override;
  FunctionLineButton* functionButton(Window* parent, const rect_t& rect,
                                 uint8_t index) const override;
  void setDirty() const override;
};
