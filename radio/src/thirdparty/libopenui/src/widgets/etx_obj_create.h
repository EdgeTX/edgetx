
#ifndef ETX_OBJ_CREATE_H
#define ETX_OBJ_CREATE_H

#include <lvgl/lvgl.h>

lv_obj_t* window_create(lv_obj_t* parent);

lv_obj_t* etx_text_edit_create(lv_obj_t* parent);
lv_obj_t* etx_number_edit_create(lv_obj_t* parent);

lv_obj_t* etx_table_create(lv_obj_t* parent);
lv_obj_t* etx_keyboard_create(lv_obj_t* parent);
lv_obj_t* etx_switch_create(lv_obj_t* parent);
lv_obj_t* etx_slider_create(lv_obj_t* parent);
lv_obj_t* etx_btnmatrix_create(lv_obj_t* parent);
lv_obj_t* etx_button_create(lv_obj_t* parent);
lv_obj_t* etx_choice_create(lv_obj_t* parent);

lv_obj_t* input_mix_line_create(lv_obj_t* parent);
lv_obj_t* input_mix_group_create(lv_obj_t* parent);

#endif
