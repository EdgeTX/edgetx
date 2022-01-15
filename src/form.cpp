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

#include "form.h"
#include "bitmapbuffer.h"
#include "libopenui_config.h"

FormField::FormField(Window * parent, const rect_t & rect, WindowFlags windowFlags, LcdFlags textFlags, LvglWidgetFactory *factory) :
  Window(parent, rect, windowFlags, textFlags, factory)
{
  if (!(windowFlags & NO_FOCUS)) {
    auto * form = dynamic_cast<FormGroup *>(parent);
    if (form) {
      form->addField(this, windowFlags & PUSH_FRONT);
    }
  }
}

#if defined(HARDWARE_KEYS)
void FormField::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("FormField").c_str(), event);

  if (event == EVT_ROTARY_RIGHT/*EVT_KEY_BREAK(KEY_DOWN)*/) {
    if (next) {
      onKeyPress();
      next->setFocus(SET_FOCUS_FORWARD, this);
    }
  }
  else if (event == EVT_ROTARY_LEFT/*EVT_KEY_BREAK(KEY_UP)*/) {
    if (previous) {
      onKeyPress();
      previous->setFocus(SET_FOCUS_BACKWARD, this);
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
    setEditMode(!editMode);
    invalidate();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && editMode) {
    onKeyPress();
    setEditMode(false);
    invalidate();
  }
  else {
    Window::onEvent(event);
  }
}
#endif

void FormField::setFocus(uint8_t flag, Window * from)
{
  if (enabled) {
    Window::setFocus(flag, from);
  }
  else {
    if (flag == SET_FOCUS_BACKWARD) {
      if (previous) {
        previous->setFocus(flag, this);
      }
    }
    else {
      if (next) {
        next->setFocus(flag, this);
      }
    }
  }
}

void FormField::paint(BitmapBuffer * dc)
{
  uint32_t bgColor = 0;
  if (backgroundHandler) {
    bgColor = backgroundHandler(this);
  } else {
    if (editMode) {
      bgColor = COLOR_THEME_EDIT;
    } else if (hasFocus()) {
        bgColor = COLOR_THEME_FOCUS;
    } else {
      bgColor = COLOR_THEME_PRIMARY2;
    }
  }

  if (editMode || ! (windowFlags & FORM_NO_BORDER)) {
    dc->drawSolidFilledRect(0, 0, rect.w, rect.h, bgColor);
    if (!editMode)
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR_THEME_SECONDARY2);
  }
}

void FormGroup::removeField(FormField * field)
{
  FormField* prev = field->getPreviousField();
  FormField* next = field->getNextField();

  if (prev) { prev->setNextField(next); }
  if (next) { next->setPreviousField(prev); }

  if (first == field) {
    if (prev && (prev != field)) first = prev;
    else if (next && (next != field)) first = next;
    else first = nullptr;
  }

  if (last == field) {
    if (next && (next != field)) last = next;
    else if (prev && (prev != field)) last = prev;
    else last = nullptr;
  }
}

void FormGroup::addField(FormField * field, bool front)
{
  if (field->getWindowFlags() & FORM_DETACHED)
    return;

  if (front) {
    if (first)
      link(field, first);
    first = field;
    if (!last)
      last = field;
  }
  else {
    if (last)
      link(last, field);
    last = field;
    if (!first)
      first = field;
  }

  if (WRAP_FORM_FIELDS_WITHIN_PAGE) {
    if (previous && (windowFlags & FORM_FORWARD_FOCUS)) {
      last->setNextField(this);
      link(previous, first);
    }
    else {
      link(last, first);
    }
  }
  else {
    if (windowFlags & FORM_FORWARD_FOCUS) {
      last->setNextField(this);
      if (previous)
        link(previous, first);
    }
  }

  if (!focusWindow && !(field->getWindowFlags() & FORM_FORWARD_FOCUS)) {
    field->setFocus(SET_FOCUS_DEFAULT);
  }
  else if (focusWindow == this && (windowFlags & FORM_FORWARD_FOCUS)) {
    field->setFocus(SET_FOCUS_DEFAULT);
  }
}

void FormGroup::setFocus(uint8_t flag, Window * from)
{
  TRACE_WINDOWS("%s setFocus(%d)", getWindowDebugString("FormGroup").c_str(), flag);

  if (windowFlags & FORM_FORWARD_FOCUS) {
    switch (flag) {
      case SET_FOCUS_BACKWARD:
        if (from && from->isChild(first)) {
          if (previous == this) {
            last->setFocus(SET_FOCUS_BACKWARD, this);
          }
          else if (previous) {
            previous->setFocus(SET_FOCUS_BACKWARD, this);
          }
        }
        else {
          if (last) {
            last->setFocus(SET_FOCUS_BACKWARD, this);
          }
          else if (previous) {
            previous->setFocus(SET_FOCUS_BACKWARD, this);
          }
        }
        break;

      case SET_FOCUS_FIRST:
        clearFocus();
        // no break;

      case SET_FOCUS_FORWARD:
        if (from && from->isChild(this)) {
          if (next == this) {
            first->setFocus(SET_FOCUS_FORWARD, this);
          }
          else if (next) {
            next->setFocus(SET_FOCUS_FORWARD, this);
          }
        }
        else {
          if (first) {
            first->setFocus(SET_FOCUS_FORWARD, this);
          }
          else if (next) {
            next->setFocus(SET_FOCUS_FORWARD, this);
          }
        }
        break;

      default:
        if (from == previous) {
          if (first) {
            first->setFocus(SET_FOCUS_DEFAULT);
          }
          else {
            clearFocus();
            focusWindow = this;
          }
        }
        else if (next) {
          next->setFocus(SET_FOCUS_FORWARD);
        }
        else {
          clearFocus();
          focusWindow = this;
        }
        break;
    }
  }
  else if (!(windowFlags & NO_FOCUS)) {
    FormField::setFocus(flag, from);
  }
}

#if defined(HARDWARE_KEYS)
void FormGroup::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("FormGroup").c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    onKeyPress();
    setFocusOnFirstVisibleField(SET_FOCUS_FIRST);
  } else if (event == EVT_KEY_FIRST(KEY_EXIT) && !hasFocus() &&
             !(windowFlags & FORM_FORWARD_FOCUS)) {
    killEvents(event);
    onKeyPress();
    setFocus(SET_FOCUS_DEFAULT);  // opentx - model - timers settings
  } else if (event == EVT_ROTARY_RIGHT && !next) {
    onKeyPress();
    setFocusOnFirstVisibleField(SET_FOCUS_FIRST);
  } else if (event == EVT_ROTARY_LEFT && !previous) {
    onKeyPress();
    setFocusOnLastVisibleField(SET_FOCUS_BACKWARD);
  } else {
    FormField::onEvent(event);
  }
}
#endif

void FormGroup::paint(BitmapBuffer * dc)
{
  if (!(windowFlags & (FORM_NO_BORDER | FORM_FORWARD_FOCUS))) {
    if (!editMode && hasFocus()) {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_FOCUS);
    }
    else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR_THEME_SECONDARY2);
    }
  }
}

#if defined(HARDWARE_KEYS)
void FormWindow::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("FormWindow").c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_EXIT) && (windowFlags & FORM_FORWARD_FOCUS) && first) {
    onKeyPress();
    Window * currentFocus = getFocus();
    first->setFocus(SET_FOCUS_FIRST);
    if (getFocus() != currentFocus) {
      return;
    }
  }

  FormGroup::onEvent(event);
}
#endif
