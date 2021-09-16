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

constexpr WindowFlags FORM_FORWARD_FOCUS = WINDOW_FLAGS_LAST << 1;
constexpr WindowFlags FORM_DETACHED = WINDOW_FLAGS_LAST << 2;
constexpr WindowFlags FORM_BORDER_FOCUS_ONLY = WINDOW_FLAGS_LAST << 3;
constexpr WindowFlags FORM_NO_BORDER = WINDOW_FLAGS_LAST << 4;
constexpr WindowFlags FORM_FLAGS_LAST = FORM_NO_BORDER;

class FormField: public Window
{
  public:
    FormField(Window * parent, const rect_t & rect, WindowFlags windowFlags = 0, LcdFlags textFlags = 0);

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

    inline void setNextField(FormField *field)
    {
      next = field;
    }

    inline void setPreviousField(FormField * field)
    {
      previous = field;
    }

    inline FormField * getPreviousField()
    {
      return previous;
    }

    inline FormField * getNextField()
    {
      return next;
    }

    inline bool isEditMode() const
    {
      return editMode;
    }

    virtual void setEditMode(bool newEditMode)
    {
      editMode = newEditMode;
      invalidate();
    }

    void enable(bool value = true)
    {
      enabled = value;
      invalidate();
    }

    bool isEnabled() const
    {
      return enabled;
    }

    void disable()
    {
      enable(false);
    }

    void setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override;

    void onFocusLost() override
    {
      setEditMode(false);
      Window::onFocusLost();
    }

    inline static void link(FormField * previous, FormField * next)
    {
      previous->setNextField(next);
      next->setPreviousField(previous);
    }

    void paint(BitmapBuffer * dc) override;

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

  protected:
    FormField * next = nullptr;
    FormField * previous = nullptr;
    bool editMode = false;
    bool enabled = true;
    std::function<void()> changeHandler = nullptr;
};

class FormGroup: public FormField
{
  public:
    FormGroup(Window * parent, const rect_t & rect, WindowFlags windowflags = 0) :
      FormField(parent, rect, windowflags)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "FormGroup";
    }
#endif

    void clear()
    {
      Window::clear();
      first = nullptr;
      last = nullptr;
      if (previous && (windowFlags & FORM_FORWARD_FOCUS)) {
        previous->setNextField(this);
      }
    }

    void setFocus(uint8_t flag = SET_FOCUS_DEFAULT, Window * from = nullptr) override;

    virtual void addField(FormField * field, bool front = false);

    virtual void removeField(FormField * field);

    void setFirstField(FormField * field)
    {
      first = field;
    }

    void setLastField(FormField * field)
    {
      last = field;
    }

    FormField * getFirstField() const
    {
      return first;
    }

    FormField * getLastField() const
    {
      return last;
    }

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
    FormField * first = nullptr;
    FormField * last = nullptr;

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif

    void paint(BitmapBuffer * dc) override;
};

class FormWindow: public FormGroup
{
  public:
    FormWindow(Window * parent, const rect_t & rect, WindowFlags windowFlags = 0) :
      FormGroup(parent, rect, windowFlags)
    {
    }

#if defined(DEBUG_WINDOWS)
    std::string getName() const override
    {
      return "FormWindow";
    }
#endif

  protected:
    void paint(BitmapBuffer *) override
    {
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override;
#endif
};
