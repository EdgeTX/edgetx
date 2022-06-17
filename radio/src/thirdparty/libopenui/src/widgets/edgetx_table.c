
#include "edgetx_table.h"


#define MY_CLASS &table_class

static void table_event(const lv_obj_class_t * class_p, lv_event_t * e);

const lv_obj_class_t table_class  = {
    .constructor_cb = NULL,
    .destructor_cb = NULL,
    .event_cb = table_event,
    .width_def = LV_SIZE_CONTENT,
    .height_def = LV_SIZE_CONTENT,
    .base_class = &lv_table_class,
    .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = 0,
};


lv_obj_t * table_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}


static void table_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
  lv_res_t res;

  lv_table_t * obj = (lv_table_t *) lv_event_get_target(e);
  uint16_t col = obj->col_act;
  uint16_t row = obj->row_act;

  /*Call the ancestor's event handler*/
  res = lv_obj_event_base(MY_CLASS, e);
  if(res != LV_RES_OK) return;

  lv_event_code_t code = lv_event_get_code(e);
  if (code == LV_EVENT_RELEASED) {
    obj->col_act = col;
    obj->row_act = row;
    lv_obj_invalidate((lv_obj_t *)obj);
  }
}
