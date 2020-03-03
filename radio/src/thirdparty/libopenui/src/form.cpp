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

FormField::FormField(Window *parent, const rect_t & rect, WindowFlags windowFlags) :
  Window(parent, rect, windowFlags)
{
  if (!(windowFlags & NO_FOCUS)) {
    FormGroup * form = dynamic_cast<FormGroup *>(parent);
    if (form) {
      form->addField(this);
    }
  }
}

#if defined(HARDWARE_KEYS)
void FormField::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", FormField::getWindowDebugString("FormField").c_str(), event);

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
    editMode = !editMode;
    invalidate();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && editMode) {
    editMode = false;
    invalidate();
  }
  else {
    Window::onEvent(event);
  }
}
#endif

void FormField::paint(BitmapBuffer * dc)
{
  if (editMode) {
    dc->drawSolidFilledRect(0, 0, rect.w, rect.h, FOCUS_BGCOLOR);
  }
  else if (hasFocus()) {
    dc->drawSolidRect(0, 0, rect.w, rect.h, 2, FOCUS_BGCOLOR);
  }
  else if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
    dc->drawSolidRect(0, 0, rect.w, rect.h, 1, DISABLE_COLOR);
  }
}

void FormGroup::addField(FormField * field)
{
  if (field->getWindowFlags() & FORM_DETACHED)
    return;

  if (!first) {
    first = field;
    if (windowFlags & FORM_FORWARD_FOCUS)
      link(previous, field);
  }
  if (last)
    link(last, field);
  last = field;
  if (!(windowFlags & FORM_FORWARD_FOCUS))
    link(field, first);
  else
    field->setNextField(this);
  if (!focusWindow && !(field->getWindowFlags() & FORM_FORWARD_FOCUS))
    field->setFocus();
}

void FormGroup::setFocus(uint8_t flag)
{
  TRACE_WINDOWS("FormGroup::setFocus(%d)", flag);

  if (windowFlags & FORM_FORWARD_FOCUS) {
    switch (flag) {
      case SET_FOCUS_FIRST:
        if (first)
          first->setFocus(SET_FOCUS_FIRST);
        break;
      case SET_FOCUS_BACKWARD:
        // TODO this test should be recursive
        if (focusWindow == first || focusWindow->getParent() == first) {
          if (previous == this)
            last->setFocus(SET_FOCUS_BACKWARD);
          else if (previous)
            previous->setFocus(SET_FOCUS_BACKWARD);
        }
        else {
          if (last)
            last->setFocus(SET_FOCUS_BACKWARD);
        }
        break;
      case SET_FOCUS_FORWARD:
        // TODO this test should be recursive
        if (focusWindow == last || focusWindow->getParent() == last) {
          if (next == this)
            first->setFocus(SET_FOCUS_FORWARD);
          else if (next)
            next->setFocus(SET_FOCUS_FORWARD);
        }
        else {
          if (first)
            first->setFocus(SET_FOCUS_FORWARD);
        }
        break;
      default:
        if (focusWindow == previous) {
          if (first)
            first->setFocus();
        }
        else {
          if (next)
            next->setFocus(SET_FOCUS_FORWARD);
        }
        break;
    }
  }
  else {
    FormField::setFocus();
  }
}

#if defined(HARDWARE_KEYS)
void FormGroup::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString("FormGroup").c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    first->setFocus();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && !hasFocus()) {
    setFocus(SET_FOCUS_DEFAULT);
  }
  else if (event == EVT_ROTARY_RIGHT && !next) {
    first->setFocus();
  }
  else if (event == EVT_ROTARY_LEFT && !previous) {
    last->setFocus();
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

  if (event == EVT_KEY_BREAK(KEY_EXIT) && first) {
    Window * currentFocus = getFocus();
    first->setFocus(SET_FOCUS_FIRST);
    if (getFocus() != currentFocus)
      return;
  }

  Window::onEvent(event);
}
#endif
