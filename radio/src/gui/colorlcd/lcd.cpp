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

#include "lcd.h"

#include <lvgl/lvgl.h>

#include "bitmapbuffer.h"
#include "board.h"
#include "etx_lv_theme.h"
#if !LV_USE_GPU_STM32_DMA2D && !defined(SIMU)
#include "dma2d.h"
#endif

pixel_t LCD_FIRST_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM;
pixel_t LCD_SECOND_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM;

BitmapBuffer lcdBuffer1(BMP_RGB565, LCD_W, LCD_H,
                        (uint16_t*)LCD_FIRST_FRAME_BUFFER);
BitmapBuffer lcdBuffer2(BMP_RGB565, LCD_W, LCD_H,
                        (uint16_t*)LCD_SECOND_FRAME_BUFFER);

static BitmapBuffer* lcdFront = &lcdBuffer1;
static BitmapBuffer* lcd = &lcdBuffer2;

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;

// Call backs
static void (*lcd_wait_cb)(lv_disp_drv_t*) = nullptr;
static void (*lcd_flush_cb)(lv_disp_drv_t*, uint16_t* buffer,
                            const rect_t& area) = nullptr;

void lcdSetWaitCb(void (*cb)(lv_disp_drv_t*)) { lcd_wait_cb = cb; }

void lcdSetFlushCb(void (*cb)(lv_disp_drv_t*, uint16_t*, const rect_t&))
{
  lcd_flush_cb = cb;
}

static lv_disp_drv_t* refr_disp = nullptr;

static void flushLcd(lv_disp_drv_t* disp_drv, const lv_area_t* area,
                     lv_color_t* color_p)
{
#if !defined(LCD_VERTICAL_INVERT) || defined(RADIO_F16)
#if defined(RADIO_F16)
  if (hardwareOptions.pcbrev > 0)
#endif
  {
    // we're only interested in the last flush in direct mode
    if (!lv_disp_flush_is_last(disp_drv)) {
      lv_disp_flush_ready(disp_drv);
      return;
    }
  }
#endif

#if defined(DEBUG_WINDOWS)
  if (area->x1 != 0 || area->x2 != LCD_W - 1 || area->y1 != 0 ||
      area->y2 != LCD_H - 1) {
    TRACE("partial refresh @ 0x%p {%d,%d,%d,%d}", color_p, area->x1, area->y1,
          area->x2, area->y2);
  } else {
    TRACE("full refresh @ 0x%p", color_p);
  }
#endif

  if (lcd_flush_cb) {
    refr_disp = disp_drv;

    rect_t copy_area = {area->x1, area->y1, area->x2 - area->x1 + 1,
                        area->y2 - area->y1 + 1};

    lcd_flush_cb(disp_drv, (uint16_t*)color_p, copy_area);
  }

  lv_disp_flush_ready(disp_drv);
}

static void clear_frame_buffers()
{
  memset(LCD_FIRST_FRAME_BUFFER, 0, sizeof(LCD_FIRST_FRAME_BUFFER));
  memset(LCD_SECOND_FRAME_BUFFER, 0, sizeof(LCD_SECOND_FRAME_BUFFER));
}

static void init_lvgl_disp_drv()
{
  lv_disp_draw_buf_init(&disp_buf, lcdFront->getData(), lcd->getData(),
                        LCD_W * LCD_H);
  lv_disp_drv_init(&disp_drv); /*Basic initialization*/

  disp_drv.draw_buf = &disp_buf; /*Set an initialized buffer*/
  disp_drv.flush_cb = flushLcd;  /*Set a flush callback to draw to the display*/
  disp_drv.wait_cb = lcd_wait_cb; /*Set a wait callback*/

  disp_drv.hor_res = LCD_W; /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = LCD_H; /*Set the vertical resolution in pixels*/
  disp_drv.full_refresh = 0;

#if !defined(LCD_VERTICAL_INVERT)
  disp_drv.direct_mode = 1;
#elif defined(RADIO_F16)
  disp_drv.direct_mode = (hardwareOptions.pcbrev > 0) ? 1 : 0;
#else
  disp_drv.direct_mode = 0;
#endif
}

void lcdInitDisplayDriver()
{
  static bool lcdDriverStarted = false;
  // we already have a display: exit
  if (lcdDriverStarted) return;
  lcdDriverStarted = true;

#if !LV_USE_GPU_STM32_DMA2D && !defined(SIMU)
  DMAInit();
#endif

  // Full LVGL init in firmware mode
  lv_init();
  // Initialise styles
  useMainStyle();

  // Clear buffers first
  clear_frame_buffers();
  lcdSetInitalFrameBuffer(lcdFront->getData());

  // Init hardware LCD driver
  lcdInit();
  backlightInit();

  init_lvgl_disp_drv();

  // Register the driver and save the created display object
  lv_disp_drv_register(&disp_drv);

  // remove all styles on default screen (makes it transparent as well)
  lv_obj_remove_style_all(lv_scr_act());
}

void lcdFlushed()
{
  // Only used in simulator

  // its possible to get here before flushLcd is ever called.
  // so check for nullptr first. (Race condition if you put breakpoints in
  // startup code)
  if (refr_disp != nullptr) lv_disp_flush_ready(refr_disp);
}

#if defined(SIMU)
void lcdRefresh() {}
#endif
