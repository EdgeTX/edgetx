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
#include "keyboard_base.h"

void Keyboard::hide()
{
  if (activeKeyboard) {
    activeKeyboard->clearField();
    activeKeyboard = nullptr;
  }
}

void Keyboard::clearField()
{
  detach();
  if (fields) {
    fields->setHeight(oldHeight);
    fields = nullptr;
  }
  if (fieldContainer) {
    fieldContainer->setHeight(LCD_H - 0 - fieldContainer->top());
    fieldContainer = nullptr;
  }
  if (field) {
    field->setEditMode(false);
    field = nullptr;
  }
}

void Keyboard::setField(FormField* newField)
{
  if (activeKeyboard) {
    if (activeKeyboard == this) return;
    activeKeyboard->clearField();
  }
  activeKeyboard = this;
  attach(MainWindow::instance());

  fieldContainer = getFieldContainer(newField);
  if (fieldContainer) {
    coord_t newWindowHeight = LCD_H - height();

    fieldContainer->setHeight(newWindowHeight);

    std::list<Window*> children = fieldContainer->getChildren();
    fields = *std::next(children.begin());

    rect_t scrollRect = {
      fieldContainer->left(), 
      fields->top(),  // this will remove the title from the screen to provide extra visible space.
      fieldContainer->width(),
      fieldContainer->height()
    };
    // both should be the same height as we are scrolled to the right position 
    fieldContainer->scrollTo(scrollRect);
    oldHeight = fields->height();
    fields->setHeight(newWindowHeight);

    // now we need to calculate the offset of the field in the fields scroll container
    coord_t offsetX = 0, offsetY = 0;
    Window* parentWindow = newField->getParent();
    while (parentWindow && parentWindow != fieldContainer) {
      offsetX += parentWindow->left();
      offsetY += parentWindow->top();
      parentWindow = parentWindow->getParent();
    }

    scrollRect = {
      offsetX, 
      offsetY + 69, 
      fieldContainer->width(),
      fieldContainer->height()
    };
    fields->scrollTo(scrollRect);

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
