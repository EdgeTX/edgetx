/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#pragma once

#include <lvgl/lvgl.h>

lv_obj_t* window_create(lv_obj_t* parent);
lv_obj_t* etx_form_window_create(lv_obj_t* parent);

lv_obj_t* etx_text_edit_create(lv_obj_t* parent);
lv_obj_t* etx_number_edit_create(lv_obj_t* parent);

lv_obj_t* etx_table_create(lv_obj_t* parent);
lv_obj_t* etx_keyboard_create(lv_obj_t* parent);
lv_obj_t* etx_switch_create(lv_obj_t* parent);
lv_obj_t* etx_slider_create(lv_obj_t* parent);
lv_obj_t* etx_btnmatrix_create(lv_obj_t* parent);
lv_obj_t* etx_button_create(lv_obj_t* parent);
lv_obj_t* etx_choice_create(lv_obj_t* parent);
lv_obj_t* etx_modal_create(lv_obj_t* parent);
lv_obj_t* etx_modal_content_create(lv_obj_t* parent);
lv_obj_t* etx_modal_title_create(lv_obj_t* parent);
lv_obj_t* etx_bar_create(lv_obj_t* parent);

lv_obj_t* input_mix_line_create(lv_obj_t* parent);
lv_obj_t* input_mix_group_create(lv_obj_t* parent);
