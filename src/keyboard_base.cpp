/*
 * Copyright (C) EdgeTX
 *
 * Source:
 *  https://github.com/edgetx/libopenui
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
#include "keyboard_base.h"

coord_t calcScrollOffsetForField(FormField *newField, Window *topWindow)
{
  // now we need to calculate the offset of the field in the fields scroll container
  // start with the current fields top and walk the hierarchy to calculate positon
  coord_t offsetY = newField->top();

  Window* parentWindow = newField->getParent();
  while (parentWindow && parentWindow != topWindow) {
    offsetY += parentWindow->top();
    parentWindow = parentWindow->getParent();
  }

  // try and place it in the middle of the screen.  The containing window MUST have 
  // already been resized
  return offsetY - topWindow->height() / 2;
}

void Keyboard::attachKeyboard()
{
  if (activeKeyboard) {
    if (activeKeyboard == this) return;
    activeKeyboard->clearField();
  }
  activeKeyboard = this;
  attach(MainWindow::instance());
}


// this routine finds the window that is a FormWindow.  This is the window
// that contains all of the editable fields.  This is the window that needs
// to be scrolled into view.
FormWindow *Keyboard::findFormWindow()
{
  if (fieldContainer) {
    auto children = fieldContainer->getChildren();
    auto k = children.begin();
    while (k != children.end())
    {
      FormWindow *a = dynamic_cast<FormWindow*>(*k);
      if (a)
        return a;
      k++;
    }
  }

  return nullptr;
}

void Keyboard::setField(FormField* newField)
{
  attachKeyboard();

  fieldContainer = getFieldContainer(newField);
  if (fieldContainer) {
    coord_t newHeight = LCD_H - height();
    fieldContainer->setHeight(newHeight);

    fields = findFormWindow();
    if (fields) {
      // scroll the header of the window out of view to get more space to
      // see the field being edited
      fieldContainer->setScrollPositionY(fields->top());
      oldHeight = fields->height();
      fields->setHeight(newHeight);

      coord_t offsetY = calcScrollOffsetForField(newField, fields);
      fields->setScrollPositionY(offsetY);
    }

    invalidate();
    newField->setEditMode(true);
    field = newField;
  } else {
    clearField();
  }
}

Window* Keyboard::getFieldContainer(FormField* field)
{
  return field->getFullScreenWindow();
}
