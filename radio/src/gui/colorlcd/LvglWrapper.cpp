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

static lv_indev_drv_t touchDriver;
static lv_indev_drv_t keyboard_drv;

static TouchState lastState;

#if defined(HARDWARE_TOUCH)
TouchState getLastTochState()
{
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

extern "C" void touchDriverRead(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
#if defined(HARDWARE_TOUCH)
  TouchState st = touchPanelRead();
  lastState = st; // hack for now

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

extern BitmapBuffer * lcdFront;
extern BitmapBuffer * lcd;
LvglWrapper::LvglWrapper()
{
  lv_init();
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

  /*Register the driver in LVGL and save the created input device object*/


  disp = lv_disp_drv_register(&disp_drv); /*Register the driver and save the created display objects*/

  // create a canvas as the drawing target for libopenui as a hack. This is going to be removed
  canvas = lv_canvas_create(lv_scr_act());
  lv_canvas_set_buffer(canvas, cbuf, LCD_W, LCD_H, LV_IMG_CF_TRUE_COLOR);
  
  lv_indev_drv_init(&touchDriver);          /*Basic initialization*/
  touchDriver.type = LV_INDEV_TYPE_POINTER; /*See below.*/
  touchDriver.read_cb = touchDriverRead;      /*See below.*/
  lv_indev_drv_register(&touchDriver);

  lv_indev_drv_init(&keyboard_drv);
  keyboard_drv.type = LV_INDEV_TYPE_KEYPAD;
  keyboard_drv.read_cb = keyboardDriverRead;
  lv_indev_drv_register(&keyboard_drv);
}

void LvglWrapper::run()
{
    tmr10ms_t tick = get_tmr10ms();
    lv_tick_inc((tick - lastTick) * 10);
    lastTick = tick;
    lv_timer_handler();
}
