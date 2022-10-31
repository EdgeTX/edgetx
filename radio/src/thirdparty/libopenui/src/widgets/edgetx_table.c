
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
  lv_event_code_t code = lv_event_get_code(e);
  if (code != LV_EVENT_RELEASED) {
    /*Call the ancestor's event handler*/
    res = lv_obj_event_base(MY_CLASS, e);
    if(res != LV_RES_OK) return;
  } else {
    lv_obj_t * obj = lv_event_get_target(e);
    lv_table_t * table = (lv_table_t *)obj;

    /*From lv_table.c: handler for LV_EVENT_RELEASED*/
    lv_obj_invalidate(obj);
    lv_indev_t* indev = lv_indev_get_act();
    lv_obj_t* scroll_obj = lv_indev_get_scroll_obj(indev);
    if (table->col_act != LV_TABLE_CELL_NONE &&
        table->row_act != LV_TABLE_CELL_NONE && scroll_obj == NULL) {
      res = lv_event_send(obj, LV_EVENT_VALUE_CHANGED, NULL);
      if (res != LV_RES_OK) return;
    }
  }
}
