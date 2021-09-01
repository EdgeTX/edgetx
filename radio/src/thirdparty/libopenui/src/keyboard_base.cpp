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
FormWindow *Keyboard::findFormWindow(Window *parent)
{
  if (parent) {
    auto children = parent->getChildren();
    auto childIterator = children.begin();
    while (childIterator != children.end())
    {
#if defined(DEBUG_WINDOWS)
      std::string windowName;
      Window *window = dynamic_cast<Window *>(*childIterator);
      if (window)
        windowName = window->getName();
#endif
        
      FormWindow *formWindow = dynamic_cast<FormWindow*>(*childIterator);
      if (formWindow)
        return formWindow;

      childIterator++;
    }
  }

  return nullptr;
}

void Keyboard::setField(FormField* newField)
{
  attachKeyboard();

  fieldContainer = getFieldContainer(newField);
  if (fieldContainer) {
#if defined(DEBUG_WINDOWS)
    auto windowName = fieldContainer->getName();
#endif
    coord_t newHeight = LCD_H - height();
    fieldContainer->setHeight(newHeight);

    fields = findFormWindow(fieldContainer);

    if (fields) {
#if defined(DEBUG_WINDOWS)
      windowName = fields->getName();
#endif      
      // scroll the header of the window out of view to get more space to
      // see the field being edited
      fieldContainer->setScrollPositionY(fields->top());
      oldHeight = fields->height();
      fields->setHeight(newHeight);

      coord_t offsetY = calcScrollOffsetForField(newField, fields);
      fields->setScrollPositionY(offsetY);
    } else {
      // TODO: what happens if field container is null. 
      // we will need to interrogate the fieldContainer for its
      // FormGroup.  That is really how it should work anyway.
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
