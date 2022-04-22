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
    if (changeHandler) {
      changeHandler();
    }
  }

  void setChangeHandler(std::function<void()> handler)
  {
    changeHandler = std::move(handler);
  }

  void setBackgroundHandler(std::function<uint32_t(FormField *field)> handler)
  {
    backgroundHandler = std::move(handler);
  }

  inline void setNextField(FormField *field) { next = field; }

  inline void setPreviousField(FormField *field) { previous = field; }

  inline FormField *getPreviousField() { return previous; }

  inline FormField *getNextField() { return next; }

  inline bool isEditMode() const { return editMode; }

  virtual void setEditMode(bool newEditMode)
  {
    editMode = newEditMode;

    // if (lvobj != nullptr) {
    //   lv_group_t *grp = (lv_group_t *)lv_obj_get_group(lvobj);
    //   if (grp != nullptr) lv_group_set_editing(grp, editMode);
    // }

    if (editMode && lvobj != nullptr) {
      lv_obj_add_state(lvobj, LV_STATE_EDITED);
    } else if (lvobj != nullptr) {
      lv_obj_clear_state(lvobj, LV_STATE_EDITED);
    }
    invalidate();
  }

  void enable(bool value = true)
  {
    enabled = value;
    invalidate();
  }

  bool isEnabled() const { return enabled; }
  void disable() { enable(false); }

  void setFocus(uint8_t flag = SET_FOCUS_DEFAULT,
                Window *from = nullptr) override;

  void onFocusLost() override
  {
    setEditMode(false);
    Window::onFocusLost();
  }

  inline static void link(FormField *previous, FormField *next)
  {
    previous->setNextField(next);
    next->setPreviousField(previous);
  }

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif

 protected:
  FormField *next = nullptr;
  FormField *previous = nullptr;
  bool editMode = false;
  bool enabled = true;
  std::function<void()> changeHandler = nullptr;
  std::function<uint32_t(FormField *)> backgroundHandler = nullptr;
};

class FieldContainer
{
 public:
  virtual void addField(FormField *field, bool front = false) = 0;
  virtual void removeField(FormField *field) = 0;
};

class FormGroup : public FormField, public FieldContainer
{
 public:
  class Line : public Window, public FieldContainer
  {
    FlexGridLayout* layout;

   public:
    Line(FormGroup *parent, lv_obj_t *lvobj, FlexGridLayout* layout);

   protected:
    void addChild(Window* window, bool front = false) override;
    void addField(FormField *field, bool front = false) override;
    void removeField(FormField *field) override;
  };

  FormGroup(Window *parent, const rect_t &rect, WindowFlags windowflags = 0,
            LvglCreate objConstruct = nullptr);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "FormGroup"; }
#endif

  void clear();

  void setFlexLayout(lv_flex_flow_t flow = LV_FLEX_FLOW_COLUMN, lv_coord_t padding = 0);
  Line* newLine(FlexGridLayout* layout);
  
  void setFocus(uint8_t flag = SET_FOCUS_DEFAULT,
                Window *from = nullptr) override;

  void addField(FormField *field, bool front = false) override;
  void removeField(FormField *field) override;

  void setFirstField(FormField *field) { first = field; }
  void setLastField(FormField *field) { last = field; }
  FormField *getFirstField() const { return first; }
  FormField *getLastField() const { return last; }

  void setFocusOnFirstVisibleField(uint8_t flag = SET_FOCUS_DEFAULT) const
  {
    auto field = getFirstField();
    while (field && !field->isInsideParentScrollingArea()) {
      field = field->getNextField();
    }
    if (field) {
      field->setFocus(flag);
    }
  }

  void setFocusOnLastVisibleField(uint8_t flag = SET_FOCUS_DEFAULT) const
  {
    auto field = getLastField();
    while (field && !field->isInsideParentScrollingArea()) {
      field = field->getPreviousField();
    }
    if (field) {
      field->setFocus(flag);
    }
  }

 protected:
  FormField *first = nullptr;
  FormField *last = nullptr;

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif
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

#if defined(HARDWARE_KEYS)
  void onEvent(event_t event) override;
#endif
};
