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

#include "field_edit.h"

#define MY_CLASS &field_edit_class

static void field_edit_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
  /* Call the ancestor's event handler */
  lv_obj_event_base(MY_CLASS, e);
}

const lv_obj_class_t field_edit_class = {
    .base_class = &lv_textarea_class,
    .event_cb = field_edit_event,
    .width_def = LV_DPI_DEF / 2,
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_textarea_t)
};

lv_obj_t* field_edit_create(lv_obj_t* parent)
{
  lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
  lv_obj_class_init_obj(obj);
  return obj;
}
