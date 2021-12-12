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

#include "opentx.h"

#include "LvglWrapper.h"

LvglWrapper* LvglWrapper::_instance = nullptr;

static lv_disp_drv_t disp_drv;          /*A variable to hold the drivers. Must be static or global.*/
lv_disp_t * disp;

static lv_disp_draw_buf_t disp_buf;
static lv_indev_drv_t indev_drv;

void my_flush_cb(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
  uint16_t width = area->x2-area->x1+1;
  uint16_t height = area->y2-area->y1+1;
#if defined (LCD_VERTICAL_INVERT)
  uint16_t x = LCD_W - area->x2 -1;
  uint16_t y = LCD_H - area->y2 -1;
#else
  uint16_t x = area->x1;
  uint16_t y = area->y1;
#endif

  lcdCopy(lcd->getData(), lcdFront->getData());
  DMACopyBitmap(lcd->getData(), LCD_W, LCD_H, x, y, (uint16_t*)color_p, width, height, 0, 0, width, height);
  lv_disp_flush_ready(disp_drv);
  lcdRefresh();
}
  /*Initialize `disp_buf` with the buffer(s). With only one buffer use NULL instead buf_2 */
static void btn_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * btn = lv_event_get_target(e);
  if(code == LV_EVENT_CLICKED) {
    static uint8_t cnt = 0;
    cnt++;

    /*Get the first child of the button which is the label and change its text*/
    lv_obj_t * label = lv_obj_get_child(btn, 0);
    lv_label_set_text_fmt(label, "Button: %d", cnt);
  }
}


void lv_example_get_started_1(void)
{
  lv_obj_t * btn = lv_btn_create(lv_scr_act());     /*Add a button the current screen*/
  lv_obj_set_pos(btn, 10, 10);                            /*Set its position*/
  lv_obj_set_size(btn, 120, 50);                          /*Set its size*/
  lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);           /*Assign a callback to the button*/

  lv_obj_t * label = lv_label_create(btn);          /*Add a label to the button*/
  lv_label_set_text(label, "Button");                     /*Set the labels text*/
  lv_obj_center(label);
}
void lv_example_label_1(void)
{
  lv_obj_t * label1 = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(label1, LV_LABEL_LONG_WRAP);     /*Break the long lines*/
  lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
  lv_label_set_text(label1, "#0000ff Re-color# #ff00ff words# #ff0000 of a# label, align the lines to the center "
            "and wrap long text automatically.");
  lv_obj_set_width(label1, 150);  /*Set smaller width to make the lines wrap*/
  lv_obj_set_style_text_align(label1, LV_TEXT_ALIGN_CENTER, 0);
  lv_obj_align(label1, LV_ALIGN_CENTER, 0, -40);


  lv_obj_t * label2 = lv_label_create(lv_scr_act());
  lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);     /*Circular scroll*/
  lv_obj_set_width(label2, 150);
  lv_label_set_text(label2, "It is a circularly scrolling text. ");
  lv_obj_align(label2, LV_ALIGN_CENTER, 0, 40);
}

extern "C" void my_input_read(lv_indev_drv_t * drv, lv_indev_data_t*data)
{
  TouchState st = touchPanelRead();
  if(st.event == TE_NONE)
  return;
  if(st.event == TE_DOWN || st.event == TE_SLIDE)
  {
#if defined (LCD_VERTICAL_INVERT)
  data->point.x = LCD_W - st.x;
  data->point.y = LCD_H - st.y;
#else
  data->point.x = st.x;
  data->point.y = st.y;
#endif
  data->state = LV_INDEV_STATE_PRESSED;
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static void slider_event_cb(lv_event_t * e);
static lv_obj_t * slider_label;

/**
 * A default slider with a label displaying the current value
 */
void lv_example_slider_1(void)
{
  /*Create a slider in the center of the display*/
  lv_obj_t * slider = lv_slider_create(lv_scr_act());
  lv_obj_center(slider);
  lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
  lv_obj_align(slider, LV_ALIGN_BOTTOM_LEFT, 10, -10);

  /*Create a label below the slider*/
  slider_label = lv_label_create(lv_scr_act());
  lv_label_set_text(slider_label, "0%");

  lv_obj_align_to(slider_label, slider, LV_ALIGN_CENTER, 0, 0);
}

static void slider_event_cb(lv_event_t * e)
{
  lv_obj_t * slider = lv_event_get_target(e);
  char buf[8];
  lv_snprintf(buf, sizeof(buf), "%d%%", (int)lv_slider_get_value(slider));
  lv_label_set_text(slider_label, buf);
  lv_obj_align_to(slider_label, slider, LV_ALIGN_CENTER, 0, 0);
}

static void sw_event_cb(lv_event_t * e)
{
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * sw = lv_event_get_target(e);

  if(code == LV_EVENT_VALUE_CHANGED) {
    lv_obj_t * list = (lv_obj_t*)lv_event_get_user_data(e);

    if(lv_obj_has_state(sw, LV_STATE_CHECKED))
      lv_obj_add_flag(list, LV_OBJ_FLAG_SCROLL_ONE);
    else
      lv_obj_clear_flag(list, LV_OBJ_FLAG_SCROLL_ONE);
  }
}

/**
 * Show an example to scroll snap
 */
void lv_example_scroll_2(void)
{
  lv_obj_t * panel = lv_obj_create(lv_scr_act());
  lv_obj_set_size(panel, LCD_W/3, LCD_H/3);
  lv_obj_set_scroll_snap_x(panel, LV_SCROLL_SNAP_CENTER);
  lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_ROW);
  lv_obj_align(panel, LV_ALIGN_BOTTOM_RIGHT, 0, 0);
//    lv_obj_align(panel, LV_ALIGN_BOTTOM_RIGHT, 0, 20);

  unsigned int i;
  for(i = 0; i < 10; i++) {
    lv_obj_t * btn = lv_btn_create(panel);
    lv_obj_set_size(btn, lv_pct(60), lv_pct(80));

    lv_obj_t * label = lv_label_create(btn);
    if(i == 3) {
        lv_label_set_text_fmt(label, "Panel %u\nno snap", i);
        lv_obj_clear_flag(btn, LV_OBJ_FLAG_SNAPPABLE);
    } else {
        lv_label_set_text_fmt(label, "Panel %u", i);
    }

    lv_obj_center(label);
  }
  lv_obj_update_snap(panel, LV_ANIM_ON);

  /*Switch between "One scroll" and "Normal scroll" mode*/
  lv_obj_t * sw = lv_switch_create(lv_scr_act());
  lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, -20, 10);
  lv_obj_add_event_cb(sw, sw_event_cb, LV_EVENT_ALL, panel);
  lv_obj_t * label = lv_label_create(lv_scr_act());
  lv_label_set_text(label, "One scroll");
  lv_obj_align_to(label, sw, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
}
/**
 * Create a transparent canvas with Chroma keying and indexed color format (palette).
 */
#define CANVAS_WIDTH LCD_W
#define CANVAS_HEIGHT LCD_H
lv_obj_t * canvas=nullptr;
static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(CANVAS_WIDTH, CANVAS_HEIGHT)] __SDRAM;
void lv_example_canvas_2(void)
{
  /*Create a button to better see the transparency*/
  //lv_btn_create(lv_scr_act());

  /*Create a buffer for the canvas*/

  /*Create a canvas and initialize its palette*/
  /*lv_obj_t * */canvas = lv_canvas_create(lv_scr_act());
  lv_canvas_set_buffer(canvas, cbuf, CANVAS_WIDTH, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);
#if 0
  lv_canvas_set_palette(canvas, 0, LV_COLOR_CHROMA_KEY);
  lv_canvas_set_palette(canvas, 1, lv_palette_main(LV_PALETTE_RED));

  /*Create colors with the indices of the palette*/
  lv_color_t c0;
  lv_color_t c1;

  c0.full = 0;
  c1.full = 1;

  /*Red background (There is no dedicated alpha channel in indexed images so LV_OPA_COVER is ignored)*/
  lv_canvas_fill_bg(canvas, c1, LV_OPA_COVER);

  /*Create hole on the canvas*/
  uint32_t x;
  uint32_t y;
  for( y = 10; y < 30; y++) {
      for( x = 5; x < 20; x++) {
          lv_canvas_set_px_color(canvas, x, y, c0);
      }
  }
#endif
}

LvglWrapper::LvglWrapper()
{
  lv_init();
  lv_disp_draw_buf_init(&disp_buf, lcdGetScratchBuffer(),NULL, LCD_W*LCD_H);
  lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
  disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
  disp_drv.flush_cb = my_flush_cb;        /*Set a flush callback to draw to the display*/
  disp_drv.hor_res = LCD_W;                 /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = LCD_H;                 /*Set the vertical resolution in pixels*/
  disp_drv.full_refresh = 0;
  disp_drv.direct_mode = 0;
  #if defined (LCD_VERTICAL_INVERT)
  disp_drv.rotated = LV_DISP_ROT_180;
  #endif
  disp_drv.sw_rotate = 1;

  lv_indev_drv_init(&indev_drv);      /*Basic initialization*/
  indev_drv.type =LV_INDEV_TYPE_POINTER;                 /*See below.*/
  indev_drv.read_cb =my_input_read;              /*See below.*/
  /*Register the driver in LVGL and save the created input device object*/


  disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/
  lv_example_canvas_2();


    
#if 0
  /*lv_indev_t * my_indev = */lv_indev_drv_register(&indev_drv);
  lv_example_get_started_1();
  lv_example_label_1();
  lv_example_slider_1();
  lv_example_scroll_2();
#endif
}
extern const lv_obj_class_t GuiWidgetClass;

extern "C"
void GuiWidgetConstructor(const struct _lv_obj_class_t * class_p, struct _lv_obj_t * obj)
{}

extern "C"
void GuiWidgetEvent(const struct _lv_obj_class_t * class_p, struct _lv_event_t * e)
{
  LV_UNUSED(class_p);

  lv_res_t res;

  /*Call the ancestor's event handler*/
  res = lv_obj_event_base(&GuiWidgetClass, e);
  if(res != LV_RES_OK) return;

  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t * obj = lv_event_get_target(e);

  switch(code)
  {
  case LV_EVENT_DRAW_MAIN:
    break;
  }
}
const lv_obj_class_t GuiWidgetClass = {
  .base_class = nullptr,
  .constructor_cb = GuiWidgetConstructor,
  .event_cb = GuiWidgetEvent,
  .editable = LV_OBJ_CLASS_EDITABLE_TRUE,
  .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
  .instance_size = sizeof(GuiWidget)
};
