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

FormField::FormField(Window * parent, const rect_t & rect, WindowFlags windowFlags, LcdFlags textFlags) :
  Window(parent, rect, windowFlags, textFlags)
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
      next->setFocus(SET_FOCUS_FORWARD);
    }
  }
  else if (event == EVT_ROTARY_LEFT/*EVT_KEY_BREAK(KEY_UP)*/) {
    if (previous) {
      previous->setFocus(SET_FOCUS_BACKWARD);
    }
  }
  else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    setEditMode(!editMode);
    invalidate();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && editMode) {
    setEditMode(false);
    invalidate();
  }
  else {
    Window::onEvent(event);
  }
}
#endif

void FormField::setFocus(uint8_t flag)
{
  if (enabled) {
    Window::setFocus(flag);
  }
  else {
    clearFocus();
    focusWindow = this;
    if (flag == SET_FOCUS_BACKWARD) {
      if (previous) {
        previous->setFocus(flag);
      }
    }
    else {
      if (next) {
        next->setFocus(flag);
      }
    }
  }
}

void FormField::paint(BitmapBuffer * dc)
{
  if (editMode) {
    dc->drawSolidFilledRect(0, 0, rect.w, rect.h, FOCUS_BGCOLOR);
  }
  else if (!(windowFlags & FORM_NO_BORDER)) {
    if (hasFocus()) {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, FOCUS_BGCOLOR);
    }
    else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, DISABLE_COLOR);
    }
  }
}

void FormGroup::addField(FormField * field, bool front)
{
  if (field->getWindowFlags() & FORM_DETACHED)
    return;

  if (!first) {
    first = field;
    last = field;
  }
  if (front) {
    if (first)
      link(field, first);
    first = field;
  }
  else {
    if (last)
      link(last, field);
    last = field;
  }
  if (previous && (windowFlags & FORM_FORWARD_FOCUS)) {
    last->setNextField(this);
    link(previous, first);
  }
  else {
    link(last, first);
  }
  if (!focusWindow && !(field->getWindowFlags() & FORM_FORWARD_FOCUS)) {
    field->setFocus(SET_FOCUS_DEFAULT);
  }
  else if (focusWindow == this && (windowFlags & FORM_FORWARD_FOCUS)) {
    field->setFocus(SET_FOCUS_DEFAULT);
  }
}

void FormGroup::setFocus(uint8_t flag)
{
  TRACE_WINDOWS("%s setFocus(%d)", getWindowDebugString("FormGroup").c_str(), flag);

  if (windowFlags & FORM_FORWARD_FOCUS) {
    switch (flag) {
      case SET_FOCUS_BACKWARD:
        if (focusWindow->isChild(first)) {
          if (previous == this) {
            last->setFocus(SET_FOCUS_BACKWARD);
          }
          else if (previous) {
            previous->setFocus(SET_FOCUS_BACKWARD);
          }
        }
        else {
          if (last) {
            last->setFocus(SET_FOCUS_BACKWARD);
          }
          else if (previous) {
            clearFocus();
            focusWindow = this;
            previous->setFocus(SET_FOCUS_BACKWARD);
          }
        }
        break;

      case SET_FOCUS_FIRST:
        clearFocus();
        // no break;

      case SET_FOCUS_FORWARD:
        if (focusWindow && focusWindow->isChild(this)) {
          if (next == this) {
            first->setFocus(SET_FOCUS_FORWARD);
          }
          else if (next) {
            next->setFocus(SET_FOCUS_FORWARD);
          }
        }
        else {
          if (first) {
            first->setFocus(SET_FOCUS_FORWARD);
          }
          else if (next) {
            clearFocus();
            focusWindow = this;
            next->setFocus(SET_FOCUS_FORWARD);
          }
        }
        break;

      default:
        if (focusWindow == previous) {
          if (first) {
            first->setFocus(SET_FOCUS_DEFAULT);
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
  else {
    FormField::setFocus(SET_FOCUS_DEFAULT);
  }
}

#if defined(HARDWARE_KEYS)
void FormGroup::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("FormGroup").c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    setFocusOnFirstVisibleField(SET_FOCUS_FIRST);
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && !hasFocus() && !(windowFlags & FORM_FORWARD_FOCUS)) {
    setFocus(SET_FOCUS_DEFAULT); // opentx - model - timers settings
  }
  else if (event == EVT_ROTARY_RIGHT && !next) {
    setFocusOnFirstVisibleField(SET_FOCUS_FIRST);
  }
  else if (event == EVT_ROTARY_LEFT && !previous) {
    setFocusOnLastVisibleField(SET_FOCUS_BACKWARD);
  }
  else {
    FormField::onEvent(event);
  }
}
#endif

void FormGroup::paint(BitmapBuffer * dc)
{
  if (!(windowFlags & (FORM_NO_BORDER | FORM_FORWARD_FOCUS))) {
    if (!editMode && hasFocus()) {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, FOCUS_BGCOLOR);
    }
    else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, DISABLE_COLOR);
    }
  }
}

#if defined(HARDWARE_KEYS)
void FormWindow::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("FormWindow").c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_EXIT) && (windowFlags & FORM_FORWARD_FOCUS) && first) {
    Window * currentFocus = getFocus();
    first->setFocus(SET_FOCUS_FIRST);
    if (getFocus() != currentFocus)
      return;
  }

  FormGroup::onEvent(event);
}
#endif
