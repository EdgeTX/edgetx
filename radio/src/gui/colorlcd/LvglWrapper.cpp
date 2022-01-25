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

lv_disp_t * disp;
static lv_disp_draw_buf_t disp_buf;

static lv_indev_drv_t touchDriver;
static lv_indev_drv_t keyboard_drv;


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

void newLcdRefresh(uint16_t* buffer);
static void flushLcd(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
  newLcdRefresh((uint16_t*)color_p);
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

lv_obj_t * canvas=nullptr;
static lv_color_t cbuf[LV_CANVAS_BUF_SIZE_TRUE_COLOR(LCD_W, LCD_H)] __SDRAM;
BitmapBuffer canBuf(BMP_RGB565, LCD_W, LCD_H, (uint16_t *)cbuf);

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
}

LvglWrapper::LvglWrapper()
{
  lv_init();
  init_lvgl_drivers();

  // Create a canvas as the drawing target for libopenui as a hack.
  // This is going to be removed
  //
  canvas = lv_canvas_create(lv_scr_act());
  lv_canvas_set_buffer(canvas, cbuf, LCD_W, LCD_H, LV_IMG_CF_TRUE_COLOR);
  lv_obj_set_style_bg_opa(canvas, LV_OPA_MAX, LV_PART_MAIN);

  auto window = MainWindow::instance();
  lv_obj_set_parent(canvas, window->getLvObj());
  lv_obj_move_background(canvas);
  lv_scr_load(window->getLvObj());
  lv_obj_invalidate(canvas);
}

void LvglWrapper::run()
{
    tmr10ms_t tick = get_tmr10ms();
    lv_tick_inc((tick - lastTick) * 10);
    lastTick = tick;
    lv_timer_handler();
}
