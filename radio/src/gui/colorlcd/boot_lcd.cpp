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

#if LV_USE_GPU_STM32_DMA2D
#include <lvgl/src/draw/stm32_dma2d/lv_gpu_stm32_dma2d.h>
#else
#include "dma2d.h"
#endif

#include "bitmapbuffer.h"
#include "board.h"
#include "etx_lv_theme.h"

pixel_t LCD_FIRST_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM __ALIGNED(64);
pixel_t LCD_SECOND_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM __ALIGNED(64);

BitmapBuffer lcd(BMP_RGB565, LCD_W, LCD_H, (uint16_t*)LCD_FIRST_FRAME_BUFFER);

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_disp_t disp;

// Call backs
static void (*lcd_flush_cb)(lv_disp_drv_t*, uint16_t* buffer,
                            const rect_t& area) = nullptr;

extern "C" void lcdFlushed()
{
  lv_disp_flush_ready(&disp_drv);
}

void lcdSetFlushCb(void (*cb)(lv_disp_drv_t*, uint16_t*, const rect_t&))
{
  lcd_flush_cb = cb;
}

static void flushLcd(lv_disp_drv_t* disp_drv, const lv_area_t* area,
                     lv_color_t* color_p)
{
  if (lcd_flush_cb) {
    rect_t copy_area = {area->x1, area->y1, area->x2 - area->x1 + 1,
                        area->y2 - area->y1 + 1};

    lcd_flush_cb(disp_drv, (uint16_t*)color_p, copy_area);
  } else {
    lcdFlushed();
  }
}

static void clear_frame_buffers()
{
  memset(LCD_FIRST_FRAME_BUFFER, 0, sizeof(LCD_FIRST_FRAME_BUFFER));
  memset(LCD_SECOND_FRAME_BUFFER, 0, sizeof(LCD_SECOND_FRAME_BUFFER));
}

static void init_lvgl_disp_drv()
{
  int direct_mode = 1;
#if LCD_VERTICAL_INVERT
#if defined(RADIO_F16)
  direct_mode = (hardwareOptions.pcbrev > 0) ? 1 : 0;
#else
  direct_mode = 0;
#endif
#endif

  lv_disp_draw_buf_init(&disp_buf,
                        LCD_FIRST_FRAME_BUFFER,
                        direct_mode ? LCD_SECOND_FRAME_BUFFER : nullptr,
                        LCD_W * LCD_H);
  lv_disp_drv_init(&disp_drv); /*Basic initialization*/

  disp_drv.draw_buf = &disp_buf; /*Set an initialized buffer*/
  disp_drv.flush_cb = flushLcd;  /*Set a flush callback to draw to the display*/

  disp_drv.hor_res = LCD_W; /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = LCD_H; /*Set the vertical resolution in pixels*/
  disp_drv.direct_mode = direct_mode;
}

void lcdInitDisplayDriver()
{
  static bool lcdDriverStarted = false;
  // we already have a display: exit
  if (lcdDriverStarted) return;
  lcdDriverStarted = true;

#if LV_USE_GPU_STM32_DMA2D
  // Init only DMA2D
  lv_draw_stm32_dma2d_init();
#else
  DMAInit();
#endif

  // Clear buffers first
  clear_frame_buffers();
  lcdSetInitalFrameBuffer(lcd.getData());

  // Init hardware LCD driver
  lcdInit();

  init_lvgl_disp_drv();

  // allow drawing at any moment
  lv_memset_00(&disp, sizeof(lv_disp_t));
  disp.driver = &disp_drv;
  _lv_refr_set_disp_refreshing(&disp);

  if (disp_drv.draw_ctx == NULL) {
    lv_draw_ctx_t* draw_ctx =
        (lv_draw_ctx_t*)lv_mem_alloc(disp_drv.draw_ctx_size);
    LV_ASSERT_MALLOC(draw_ctx);
    if (draw_ctx == NULL) return;
    disp_drv.draw_ctx_init(&disp_drv, draw_ctx);
    disp_drv.draw_ctx = draw_ctx;
  }

  lv_draw_ctx_t* draw_ctx = disp_drv.draw_ctx;
  lcd.setDrawCtx(draw_ctx);

  static lv_area_t screen_area = {0, 0, LCD_W - 1, LCD_H - 1};
  draw_ctx->buf = disp_drv.draw_buf->buf_act;
  draw_ctx->buf_area = &screen_area;
  draw_ctx->clip_area = &screen_area;
    
#if defined(LCD_BACKLIGHT_INIT_DELAY_MS)
  delay_ms(LCD_BACKLIGHT_INIT_DELAY_MS);
#endif
    
  backlightInit();
}

void lcdClear() { lcd.clear(); }

// Direct drawing - used by boot loader

//
// Private code copied and adapted from LVGL / lv_refr.c
// (_call_flush_cb & _draw_buf_flush)
//
static void _call_flush_cb(lv_disp_drv_t* drv, const lv_area_t* area,
                           lv_color_t* color_p)
{
  lv_area_t offset_area = {.x1 = (lv_coord_t)(area->x1 + drv->offset_x),
                           .y1 = (lv_coord_t)(area->y1 + drv->offset_y),
                           .x2 = (lv_coord_t)(area->x2 + drv->offset_x),
                           .y2 = (lv_coord_t)(area->y2 + drv->offset_y)};

  drv->flush_cb(drv, &offset_area, color_p);
}

void lcdRefresh()
{
  /* Flush the rendered content to the display */
  lv_disp_t* disp = _lv_refr_get_disp_refreshing();
  lv_disp_draw_buf_t* draw_buf = lv_disp_get_draw_buf(disp);
  lv_draw_ctx_t* draw_ctx = disp->driver->draw_ctx;

  draw_buf->flushing = 1;
  draw_buf->flushing_last = 1;

  if (disp->driver->flush_cb) {
    _call_flush_cb(disp->driver, draw_ctx->buf_area,
                   (lv_color_t*)draw_ctx->buf);
  }

  /*If there are 2 buffers swap them. */
  if (draw_buf->buf1 && draw_buf->buf2) {
    /* In double buffered mode wait until the other buffer is freed
     * and driver is ready to receive the new buffer */
    while (draw_buf->flushing);
    if (draw_buf->buf_act == draw_buf->buf1)
      draw_buf->buf_act = draw_buf->buf2;
    else
      draw_buf->buf_act = draw_buf->buf1;
  }

  // Update draw context and lcd bitmap
  draw_ctx->buf = draw_buf->buf_act;
  lcd.setData((pixel_t*)draw_buf->buf_act);
  lcd.reset();
}
