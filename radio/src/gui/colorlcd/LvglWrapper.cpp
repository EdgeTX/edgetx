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
#include "view_main.h"

LvglWrapper* LvglWrapper::_instance = nullptr;

static lv_disp_t* disp;
static lv_disp_draw_buf_t disp_buf;
static lv_indev_drv_t touchDriver;
static lv_indev_drv_t keyboard_drv;
static lv_indev_drv_t rotaryDriver;
static lv_indev_t* rotaryDevice = nullptr;

lv_group_t* inputGroup;

#if defined(HARDWARE_TOUCH)
static bool touchOccured = false;
bool getTouchOccured()
{
  return touchOccured;
}

static TouchState lastState;
TouchState getLastTochState()
{
  touchOccured = false;
  return lastState;
}
#endif

void newLcdRefresh(uint16_t* buffer, const rect_t& copy_area);
static void flushLcd(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
  lv_area_t refr_area;
  lv_area_copy(&refr_area, area);

#if defined(LCD_VERTICAL_INVERT)
  lv_coord_t tmp_coord;
  tmp_coord = refr_area.y2;
  refr_area.y2 = LCD_H - refr_area.y1 - 1;
  refr_area.y1 = LCD_H - tmp_coord - 1;
  tmp_coord = refr_area.x2;
  refr_area.x2 = LCD_W - refr_area.x1 - 1;
  refr_area.x1 = LCD_W - tmp_coord - 1;
#endif

  if (refr_area.x1 != 0 || refr_area.x2 != 479 || refr_area.y1 != 0 ||
      refr_area.y2 != 271) {
    TRACE("partial refresh @ 0x%p {%d,%d,%d,%d}", color_p, refr_area.x1,
          refr_area.y1, refr_area.x2, refr_area.y2);
  } else {
    TRACE("full refresh @ 0x%p", color_p);
  }

  rect_t copy_area = {refr_area.x1, refr_area.y1,
                      refr_area.x2 - refr_area.x1 + 1,
                      refr_area.y2 - refr_area.y1 + 1};

  newLcdRefresh((uint16_t*)color_p, copy_area);
  lv_disp_flush_ready(disp_drv);
}

extern "C" void keyboardDriverRead(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  // if there is a keyboard event then call checkevents
  if (isEvent())
    MainWindow::instance()->checkEvents();
}

static void copy_ts_to_indev_data(const TouchState &st, lv_indev_data_t *data)
{
#if defined(LCD_VERTICAL_INVERT)
  data->point.x = LCD_W - st.x;
  data->point.y = LCD_H - st.y;
#else
  data->point.x = st.x;
  data->point.y = st.y;
#endif
}

static lv_indev_data_t indev_data_backup;

static void backup_indev_data(lv_indev_data_t* data)
{
  memcpy(&indev_data_backup, data, sizeof(lv_indev_data_t));
}

static void copy_indev_data_backup(lv_indev_data_t* data)
{
  memcpy(data, &indev_data_backup, sizeof(lv_indev_data_t));
}

extern "C" void touchDriverRead(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
#if defined(HARDWARE_TOUCH)
  if(!touchPanelEventOccured()) {
    copy_indev_data_backup(data);
    return;
  }
  touchOccured = true;

  TouchState st = touchPanelRead();
  lastState = st; // hack for now

  if(st.event == TE_NONE) {
    TRACE("TE_NONE");
  } else if(st.event == TE_DOWN || st.event == TE_SLIDE) {
    TRACE("INDEV_STATE_PRESSED");
    data->state = LV_INDEV_STATE_PRESSED;
    copy_ts_to_indev_data(st, data);
  } else {
    TRACE("INDEV_STATE_RELEASED");
    data->state = LV_INDEV_STATE_RELEASED;
    copy_ts_to_indev_data(st, data);
  }

  backup_indev_data(data);
#endif
}

event_t encoderEvent = 0;
bool newEncoderEvent = false;
lv_indev_state_t encoderState = LV_INDEV_STATE_RELEASED;

void lvglPushEncoderEvent(event_t evt)
{
  if(evt != EVT_KEY_FIRST(KEY_ENTER) && evt != EVT_KEY_BREAK(KEY_ENTER) && evt != EVT_ROTARY_LEFT && evt !=  EVT_ROTARY_RIGHT)
    return;

  encoderEvent = evt;
  newEncoderEvent = true;
}

extern "C" void rotaryDriverRead(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
  data->enc_diff = 0;
  if(newEncoderEvent)
  {
    switch(encoderEvent)
    {
    case EVT_KEY_FIRST(KEY_ENTER):
//      encoderState = LV_INDEV_STATE_PRESSED;
      break;
    case EVT_KEY_BREAK(KEY_ENTER):
//      encoderState = LV_INDEV_STATE_RELEASED;
      break;
    case EVT_KEY_FIRST(KEY_EXIT):
//      encoderState = LV_INDEV_STATE_PRESSED;
      break;
    case EVT_KEY_BREAK(KEY_EXIT):
//      encoderState = LV_INDEV_STATE_RELEASED;
      break;
    case EVT_ROTARY_LEFT:
//      data->enc_diff = -1;
      break;
    case EVT_ROTARY_RIGHT:
//      data->enc_diff = 1;
      break;
    }
  }
  newEncoderEvent = false;
  data->state = encoderState;
}

/**
 * Helper function to translate a colorFlags value to a lv_color_t suitable
 * for passing to an lv_obj function
 * @param colorFlags a textFlags value.  This value will contain the color shifted by 16 bits.
 */
lv_color_t makeLvColor(uint32_t colorFlags)
{
  auto color = COLOR_VAL(colorFlags);
  return lv_color_make(GET_RED(color), GET_GREEN(color), GET_BLUE(color));
}


extern BitmapBuffer * lcdFront;
extern BitmapBuffer * lcd;

static lv_disp_drv_t disp_drv;

static lv_disp_drv_t* init_disp_drv()
{
  lv_disp_draw_buf_init(&disp_buf, lcdFront->getData(), lcd->getData(), LCD_W*LCD_H);
  lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
  disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
  disp_drv.flush_cb = flushLcd;           /*Set a flush callback to draw to the display*/
  disp_drv.hor_res = LCD_W;               /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = LCD_H;               /*Set the vertical resolution in pixels*/
  disp_drv.full_refresh = 0;
  disp_drv.direct_mode = 0;
#if defined (LCD_VERTICAL_INVERT)
  disp_drv.rotated = LV_DISP_ROT_180;
#endif
  disp_drv.sw_rotate = 1;

  return &disp_drv;
}

static void init_lvgl_drivers()
{
  // add all lvgl object automatically to a input handling group
  inputGroup = lv_group_create();
  lv_group_set_default(inputGroup);

  // Register the driver and save the created display objects
  disp = lv_disp_drv_register(init_disp_drv());
 
  // Register the driver in LVGL and save the created input device object
  lv_indev_drv_init(&touchDriver);          /*Basic initialization*/
  touchDriver.type = LV_INDEV_TYPE_POINTER; /*See below.*/
  touchDriver.read_cb = touchDriverRead;      /*See below.*/
  lv_indev_drv_register(&touchDriver);

  lv_indev_drv_init(&keyboard_drv);
  keyboard_drv.type = LV_INDEV_TYPE_KEYPAD;
  keyboard_drv.read_cb = keyboardDriverRead;
  lv_indev_drv_register(&keyboard_drv);

  lv_indev_drv_init(&rotaryDriver);
  rotaryDriver.type = LV_INDEV_TYPE_ENCODER;
  rotaryDriver.read_cb = rotaryDriverRead;
  rotaryDevice = lv_indev_drv_register(&rotaryDriver);

  lv_indev_set_group(rotaryDevice, inputGroup);
}

// The theme code needs to go somewhere else (gui/colorlcd/themes/default.cpp?)
static lv_style_t generic_style;
static lv_style_t btn_style;
static lv_style_t btn_checked_style;
static lv_style_t btn_focused_style;
static lv_style_t btn_checked_focused_style;

static void theme_apply_cb(lv_theme_t * th, lv_obj_t * obj)
{
  LV_UNUSED(th);
  lv_obj_add_style(obj, &generic_style, LV_PART_MAIN);

  lv_obj_t* parent = lv_obj_get_parent(obj);
  if (parent == NULL) {
    // main screen
    return;
  }

  if (lv_obj_check_type(obj, &lv_btn_class)) {
    lv_obj_add_style(obj, &btn_style, 0);
    lv_obj_add_style(obj, &btn_checked_style, LV_STATE_CHECKED);
    lv_obj_add_style(obj, &btn_focused_style, LV_STATE_FOCUSED);
    return;
  }
}

static void init_theme()
{
  /*Initialize the styles*/

  // Generic (applied to all)
  lv_style_init(&generic_style);
  lv_style_set_pad_all(&generic_style, 0);
  lv_style_set_bg_opa(&generic_style, LV_OPA_TRANSP);
  lv_style_set_border_width(&generic_style, 0);
  lv_style_set_radius(&generic_style, 0);

  // Buttons
  lv_style_init(&btn_style);
  lv_style_set_bg_opa(&btn_style, LV_OPA_100);
  lv_style_set_bg_color(&btn_style, makeLvColor(COLOR_THEME_SECONDARY2));

  // LV_STATE_CHECKED
  lv_style_set_bg_color(&btn_checked_style, makeLvColor(COLOR_THEME_ACTIVE));

  // LV_STATE_FOCUSED
  lv_style_set_border_width(&btn_focused_style, 1);
  lv_style_set_border_color(&btn_focused_style, makeLvColor(COLOR_THEME_FOCUS));
  lv_style_set_text_color(&btn_focused_style, makeLvColor(COLOR_THEME_PRIMARY2));
  lv_style_set_bg_color(&btn_focused_style, makeLvColor(COLOR_THEME_FOCUS));
  
  /*Initialize the new theme from the current theme*/
  lv_theme_t * th_act = lv_disp_get_theme(NULL);
  static lv_theme_t th_new;
  th_new = *th_act;

  /*Set the parent theme and the style apply callback for the new theme*/
  lv_theme_set_parent(&th_new, th_act);
  lv_theme_set_apply_cb(&th_new, theme_apply_cb);

  /*Assign the new theme to the current display*/
  lv_disp_set_theme(NULL, &th_new);
}

LvglWrapper::LvglWrapper()
{
  lv_init();
  init_lvgl_drivers();
  init_theme();

  // Create main window and load that screen
  auto window = MainWindow::instance();
  lv_scr_load(window->getLvObj());
}

void LvglWrapper::run()
{
    tmr10ms_t tick = get_tmr10ms();
    lv_tick_inc((tick - lastTick) * 10);
    lastTick = tick;
    lv_timer_handler();
}
