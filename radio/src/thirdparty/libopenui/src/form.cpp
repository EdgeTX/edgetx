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

FormField * FormField::current = nullptr;

FormField::FormField(Window *parent, const rect_t &rect, uint8_t flags) :
  Window(parent, rect, flags)
{
  if (current) {
    setPreviousField(current);
    current->setNextField(this);
  }

  FormWindow * form = dynamic_cast<FormWindow *>(parent);
  if (form && !form->getFirstField())
    form->setFirstField(this);

  current = this;
}

#if defined(HARDWARE_KEYS)
void FormWindow::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_EXIT) && first && getFocus() != first) {
    first->setFocus();
  }
  else {
    Window::onKeyEvent(event);
  }
}

void FormField::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", FormField::getWindowDebugString().c_str(), event);

  if (event == EVT_ROTARY_RIGHT/*EVT_KEY_BREAK(KEY_DOWN)*/) {
    if (next) {
      next->setFocus();
    }
  }
  else if (event == EVT_ROTARY_LEFT/*EVT_KEY_BREAK(KEY_UP)*/) {
    if (previous) {
      previous->setFocus();
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
    Window::onKeyEvent(event);
  }
}

void FormGroup::onKeyEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  if (event == EVT_KEY_BREAK(KEY_ENTER)) {
    editMode = true;
    first->setFocus();
  }
  else if (event == EVT_KEY_BREAK(KEY_EXIT) && editMode) {
    editMode = false;
    setFocus();
  }
  else {
    FormField::onKeyEvent(event);
  }
}
#endif

void FormField::paint(BitmapBuffer * dc)
{
  if (editMode) {
    dc->drawSolidFilledRect(0, 0, rect.w, rect.h, TEXT_INVERTED_BGCOLOR);
  }
  else if (hasFocus()) {
    dc->drawSolidRect(0, 0, rect.w, rect.h, 2, TEXT_INVERTED_BGCOLOR);
  }
  else if (!(windowFlags & BORDER_FOCUS_ONLY)) {
    dc->drawSolidRect(0, 0, rect.w, rect.h, 1, CURVE_AXIS_COLOR);
  }
}

void FormGroup::paint(BitmapBuffer * dc)
{
  if (!editMode && hasFocus()) {
    dc->drawSolidRect(0, 0, rect.w, rect.h, 2, TEXT_INVERTED_BGCOLOR);
  }
  else if (!(windowFlags & BORDER_FOCUS_ONLY)) {
    dc->drawSolidRect(0, 0, rect.w, rect.h, 1, CURVE_AXIS_COLOR);
  }
}
