/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "window_base.h"

void window_event_cb(lv_event_t* e);

#define MY_CLASS &window_base_class

static void window_base_event(const lv_obj_class_t* class_p, lv_event_t* e);

const lv_obj_class_t window_base_class = {
    .base_class = &lv_obj_class,
    .event_cb = window_base_event,
    .width_def = LV_DPI_DEF,
    .height_def = LV_DPI_DEF,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t)
};

static void window_base_event(const lv_obj_class_t* class_p, lv_event_t* e)
{
  /* Call the ancestor's event handler */
  lv_res_t res = lv_obj_event_base(MY_CLASS, e);
  if (res != LV_RES_OK) return;

  window_event_cb(e);
}

lv_obj_t* window_create(lv_obj_t* parent)
{
  lv_obj_t* obj = lv_obj_class_create_obj(MY_CLASS, parent);
  lv_obj_class_init_obj(obj);
  return obj;
}
