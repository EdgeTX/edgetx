/*
 * Copyright (C) OpenTX
 *
 * Source:
 *  https://github.com/opentx/libopenui
 *
 * This file is a part of libopenui library.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 */

#pragma once

#include "window.h"
#include "flexlayout.h"

constexpr WindowFlags FORM_FORWARD_FOCUS = WINDOW_FLAGS_LAST << 1;
constexpr WindowFlags FORM_DETACHED = WINDOW_FLAGS_LAST << 2;
constexpr WindowFlags FORM_BORDER_FOCUS_ONLY = WINDOW_FLAGS_LAST << 3;
constexpr WindowFlags FORM_NO_BORDER = WINDOW_FLAGS_LAST << 4;
constexpr WindowFlags FORM_FLAGS_LAST = FORM_NO_BORDER;

class FormField : public Window
{
 public:
  FormField(Window *parent, const rect_t &rect, WindowFlags windowFlags = 0,
            LcdFlags textFlags = 0, LvglCreate objConstruct = nullptr);

  virtual void changeEnd(bool forceChanged = false)
  {
    if (changeHandler) { changeHandler(); }
  }

  void setChangeHandler(std::function<void()> handler)
  {
    changeHandler = std::move(handler);
  }

  void setBackgroundHandler(std::function<uint32_t(FormField *field)> handler)
  {
    backgroundHandler = std::move(handler);
  }

  inline bool isEditMode() const { return editMode; }
  virtual void setEditMode(bool newEditMode);

  // TODO: replace with LVGL flags
  void enable(bool value = true)
  {
    enabled = value;
    invalidate();
  }

  bool isEnabled() const { return enabled; }
  void disable() { enable(false); }

  void onFocusLost() override
  {
    setEditMode(false);
    Window::onFocusLost();
  }

  void onClicked() override;
  void onCancel() override;
  
 protected:
  bool editMode = false;
  bool enabled = true;
  std::function<void()> changeHandler = nullptr;
  std::function<uint32_t(FormField *)> backgroundHandler = nullptr;
};

class FormGroup : public Window
{
 public:
  class Line : public Window
  {
    FlexGridLayout *layout;
    void construct();

   public:
    Line(Window *parent, lv_obj_t *obj, FlexGridLayout *layout = nullptr);

   protected:
    void addChild(Window *window) override;
  };

  FormGroup(Window *parent, const rect_t &rect, WindowFlags windowflags = 0,
            LvglCreate objConstruct = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "FormGroup"; }
#endif

  void setFlexLayout(lv_flex_flow_t flow = LV_FLEX_FLOW_COLUMN, lv_coord_t padding = 0);
  Line* newLine(FlexGridLayout* layout = nullptr);
};

class FormWindow : public FormGroup
{
 public:
  FormWindow(Window *parent, const rect_t &rect, WindowFlags windowFlags = 0,
             LvglCreate objConstruct = nullptr) :
      FormGroup(parent, rect, windowFlags, objConstruct)
  {
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "FormWindow"; }
#endif

 protected:
  void paint(BitmapBuffer *) override {}
};
