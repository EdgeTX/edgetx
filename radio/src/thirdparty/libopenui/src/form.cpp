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

// FormField styles
static lv_style_t bg_style;
static lv_style_t focus_style;
static lv_style_t edit_style;

// FormGroup styles
static lv_style_t border_style;
static lv_style_t focus_border_style;

static void init_styles()
{
  static bool inited = false;
  if (inited) return;

  inited = true;

  // FormField styles
  lv_style_init(&bg_style);
  lv_style_set_bg_color(&bg_style, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_bg_opa(&bg_style, LV_OPA_COVER);
  
  lv_style_init(&focus_style);
  lv_style_set_bg_color(&focus_style, makeLvColor(COLOR_THEME_FOCUS));

  lv_style_init(&edit_style);
  lv_style_set_bg_color(&edit_style, makeLvColor(COLOR_THEME_EDIT));

  // FormGroup styles
  lv_style_init(&border_style);
  lv_style_set_border_width(&border_style, 2);
  lv_style_set_border_color(&border_style, makeLvColor(COLOR_THEME_FOCUS));

  lv_style_init(&focus_border_style);
  lv_style_set_border_width(&focus_border_style, 1);
  lv_style_set_border_color(&focus_border_style, makeLvColor(COLOR_THEME_SECONDARY2));
}

FormField::FormField(Window* parent, const rect_t& rect,
                     WindowFlags windowFlags, LcdFlags textFlags,
                     LvglCreate objConstruct) :
    Window(parent, rect, windowFlags, textFlags, objConstruct)
{
  if (!(windowFlags & NO_FOCUS) || (windowFlags & FORM_FORWARD_FOCUS)) {
    auto cont = dynamic_cast<FieldContainer*>(parent);
    if (cont) {
      cont->addField(this);
    }
  }

  lv_obj_enable_style_refresh(false);
  init_styles();
  
  lv_obj_add_style(lvobj, &bg_style, LV_PART_MAIN);
  lv_obj_add_style(lvobj, &focus_style, LV_PART_MAIN | LV_STATE_FOCUSED);
  lv_obj_add_style(lvobj, &edit_style, LV_PART_MAIN | LV_STATE_EDITED);

  lv_obj_enable_style_refresh(true);
  lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
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
extern lv_group_t* inputGroup;
void FormField::setFocus(uint8_t flag, Window * from)
{
  if (enabled) {
    Window::setFocus(flag, from);
    if(lvobj != nullptr) {
      lv_group_focus_obj(lvobj);
    }
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

FormGroup::Line::Line(FormGroup* parent, lv_obj_t* obj,
                      FlexGridLayout* layout) :
    Window(parent, obj), layout(layout), group(parent)
{
  construct();
}

FormGroup::Line::Line(Window* parent, lv_obj_t* obj, FlexGridLayout* layout,
                      FormGroup* group) :
    Window(parent, obj), layout(layout), group(group)
{
  construct();
}

void FormGroup::Line::construct()
{
  // forward scroll and focus
  windowFlags |= FORWARD_SCROLL | NO_FOCUS | FORM_FORWARD_FOCUS;
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICK_FOCUSABLE);

  if (layout) {
    layout->apply(lvobj);
  }

  lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
  lv_obj_set_width(lvobj, lv_pct(100));
}

void FormGroup::Line::addChild(Window* window)
{
  Window::addChild(window);
  if (layout) {
    layout->add(window->getLvObj());
    layout->nextCell();
  }
}

void FormGroup::Line::addField(FormField *field)
{
  group->addField(field);
}

void FormGroup::Line::removeField(FormField *field)
{
  auto form = static_cast<FormGroup*>(parent);
  form->removeField(field);
}

FormGroup::FormGroup(Window* parent, const rect_t& rect,
                     WindowFlags windowflags, LvglCreate objConstruct) :
   FormField(parent, rect, windowflags, 0, objConstruct)
{
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_TRANSP, LV_PART_MAIN);

  if (!(windowFlags & (FORM_NO_BORDER | FORM_FORWARD_FOCUS))) {

    init_styles();
    lv_obj_add_style(lvobj, &border_style, LV_PART_MAIN | LV_STATE_FOCUSED);

    if (!(windowFlags & FORM_BORDER_FOCUS_ONLY)) {
      lv_obj_add_style(lvobj, &focus_border_style, LV_PART_MAIN);
    }
  }
}

void FormGroup::clear()
{
  Window::clear();
  first = nullptr;
  last = nullptr;
  if (previous && (windowFlags & FORM_FORWARD_FOCUS)) {
    previous->setNextField(this);
  }
}

void FormGroup::setFlexLayout(lv_flex_flow_t flow, lv_coord_t padding)
{
  lv_obj_set_flex_flow(lvobj, flow);
  if (_LV_FLEX_COLUMN & flow) {
    lv_obj_set_style_pad_row(lvobj, padding, LV_PART_MAIN);
  } else {
    lv_obj_set_style_pad_column(lvobj, padding, LV_PART_MAIN);
  }
  lv_obj_set_width(lvobj, lv_pct(100));
  lv_obj_set_height(lvobj, LV_SIZE_CONTENT);
}

FormGroup::Line* FormGroup::newLine(FlexGridLayout* layout)
{
  if (layout) layout->resetPos();
  auto lv_line = lv_obj_create(lvobj);
  return new Line(this, lv_line, layout);
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

void FormGroup::addField(FormField * field)
{
  if (field->getWindowFlags() & FORM_DETACHED)
    return;

  if (last)
    link(last, field);
  last = field;
  if (!first)
    first = field;

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
            if (first)
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

  FormField::onEvent(event);
}
#endif
