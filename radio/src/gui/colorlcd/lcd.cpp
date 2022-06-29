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

pixel_t LCD_FIRST_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM;
pixel_t LCD_SECOND_FRAME_BUFFER[DISPLAY_BUFFER_SIZE] __SDRAM;

BitmapBuffer lcdBuffer1(BMP_RGB565, LCD_W, LCD_H, (uint16_t *)LCD_FIRST_FRAME_BUFFER);
BitmapBuffer lcdBuffer2(BMP_RGB565, LCD_W, LCD_H, (uint16_t *)LCD_SECOND_FRAME_BUFFER);

BitmapBuffer * lcdFront = &lcdBuffer1;
BitmapBuffer * lcd = &lcdBuffer2;

extern BitmapBuffer * lcdFront;
extern BitmapBuffer * lcd;

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;
static lv_disp_t* disp = nullptr;

static lv_area_t screen_area = {
    0, 0, LCD_W-1, LCD_H-1
};

// Call backs
static void (*lcd_wait_cb)(lv_disp_drv_t *) = nullptr;
static void (*lcd_flush_cb)(lv_disp_drv_t *, uint16_t* buffer, const rect_t& area) = nullptr;

void lcdSetWaitCb(void (*cb)(lv_disp_drv_t *))
{
  lcd_wait_cb = cb;
}

void lcdSetFlushCb(void (*cb)(lv_disp_drv_t *, uint16_t*, const rect_t&))
{
  lcd_flush_cb = cb;
}

static lv_disp_drv_t* refr_disp = nullptr;

#if !defined(LCD_VERTICAL_INVERT)
// TODO: DMA copy would be possible (use function from draw_ctx???
static void _copy_screen_area(uint16_t* dst, uint16_t* src, const lv_area_t& copy_area)
{
  lv_coord_t x1 = copy_area.x1;
  lv_coord_t y1 = copy_area.y1;
  lv_coord_t area_w = copy_area.x2 - copy_area.x1 + 1;

  auto offset = y1 * LCD_W + x1;
  auto px_src = src + offset;
  auto px_dst = dst + offset;

  for (auto line = copy_area.y1; line <= copy_area.y2; line++) {
    memcpy(px_dst, px_src, area_w * sizeof(uint16_t));
    px_dst += LCD_W;
    px_src += LCD_W;
  }
}
#endif

static void flushLcd(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
#if !defined(LCD_VERTICAL_INVERT)
  // we're only interested in the last flush in direct mode
  if (!lv_disp_flush_is_last(disp_drv)) {
    lv_disp_flush_ready(disp_drv);
    return;
  }
#endif
  
#if defined(DEBUG_WINDOWS)
  if (area->x1 != 0 || area->x2 != LCD_W-1 || area->y1 != 0 ||
      area->y2 != LCD_H-1) {
    TRACE("partial refresh @ 0x%p {%d,%d,%d,%d}", color_p, area->x1,
          area->y1, area->x2, area->y2);
  } else {
    TRACE("full refresh @ 0x%p", color_p);
  }
#endif

  if (lcd_flush_cb) {
    refr_disp = disp_drv;


    rect_t copy_area = {area->x1, area->y1,
                        area->x2 - area->x1 + 1,
                        area->y2 - area->y1 + 1};

    lcd_flush_cb(disp_drv, (uint16_t*)color_p, copy_area);

#if !defined(LCD_VERTICAL_INVERT)
    uint16_t* src = (uint16_t*)color_p;
    uint16_t* dst = nullptr;
    if ((uint16_t*)color_p == LCD_FIRST_FRAME_BUFFER)
      dst = LCD_SECOND_FRAME_BUFFER;
    else
      dst = LCD_FIRST_FRAME_BUFFER;

    lv_disp_t* disp = _lv_refr_get_disp_refreshing();
    for(int i = 0; i < disp->inv_p; i++) {
      if(disp->inv_area_joined[i]) continue;

      const lv_area_t& refr_area = disp->inv_areas[i];

      auto area_w = refr_area.x2 - refr_area.x1 + 1;
      auto area_h = refr_area.y2 - refr_area.y1 + 1;

      DMACopyBitmap(dst, LCD_W, LCD_H, refr_area.x1, refr_area.y1,
                    src, LCD_W, LCD_H, refr_area.x1, refr_area.y1,
                    area_w, area_h);      
    }
    
    lv_disp_flush_ready(disp_drv);
#endif
  } else {
    lv_disp_flush_ready(disp_drv);
  }
}

void lcdInitDisplayDriver()
{
  // we already have a display: exit
  if (disp != nullptr) return;

  lv_init();

  // Clear buffers first
  memset(LCD_FIRST_FRAME_BUFFER, 0, sizeof(LCD_FIRST_FRAME_BUFFER));
  memset(LCD_SECOND_FRAME_BUFFER, 0, sizeof(LCD_SECOND_FRAME_BUFFER));

  // Init hardware LCD driver
  lcdInit();
  backlightInit();
  
  lv_disp_draw_buf_init(&disp_buf, lcdFront->getData(), lcd->getData(), LCD_W*LCD_H);
  lv_disp_drv_init(&disp_drv);            /*Basic initialization*/

  disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
  disp_drv.flush_cb = flushLcd;           /*Set a flush callback to draw to the display*/
  disp_drv.wait_cb = lcd_wait_cb;         /*Set a wait callback*/

  disp_drv.hor_res = LCD_W;               /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = LCD_H;               /*Set the vertical resolution in pixels*/
  disp_drv.full_refresh = 0;

#if !defined(LCD_VERTICAL_INVERT)
  disp_drv.direct_mode = 1;
#else
  disp_drv.direct_mode = 0;
#endif

  // Register the driver and save the created display object
  disp = lv_disp_drv_register(&disp_drv);

  // remove all styles on default screen (makes it transparent as well)
  lv_obj_remove_style_all(lv_scr_act());

  // transparent background:
  //  - this prevents LVGL overwritting things drawn directly into the bitmap buffer
  lv_disp_set_bg_opa(disp, LV_OPA_TRANSP);
  
  // allow drawing at any moment
  _lv_refr_set_disp_refreshing(disp);
  
  lv_draw_ctx_t * draw_ctx = disp->driver->draw_ctx;
  lcd->setDrawCtx(draw_ctx);
  lcdFront->setDrawCtx(draw_ctx);
}

void lcdInitDirectDrawing()
{
  lv_draw_ctx_t* draw_ctx = disp->driver->draw_ctx;
  draw_ctx->buf = disp->driver->draw_buf->buf_act;
  draw_ctx->buf_area = &screen_area;
  draw_ctx->clip_area = &screen_area;
  lcd->setData((pixel_t*)draw_ctx->buf);
  lcd->reset();
}

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

static void _draw_buf_flush(lv_disp_t* disp)
{
  lv_disp_draw_buf_t* draw_buf = lv_disp_get_draw_buf(disp);

  /*Flush the rendered content to the display*/
  lv_draw_ctx_t* draw_ctx = disp->driver->draw_ctx;
  if (draw_ctx->wait_for_finish) draw_ctx->wait_for_finish(draw_ctx);

  /* In double buffered mode wait until the other buffer is freed
   * and driver is ready to receive the new buffer */
  if (draw_buf->buf1 && draw_buf->buf2) {
    while (draw_buf->flushing) {
      if (disp->driver->wait_cb)
        disp->driver->wait_cb(disp->driver);
    }
  }

  draw_buf->flushing = 1;
  draw_buf->flushing_last = 1;

  if (disp->driver->flush_cb) {
    _call_flush_cb(disp->driver, draw_ctx->buf_area,
                   (lv_color_t*)draw_ctx->buf);
  }

  /*If there are 2 buffers swap them. */
  if (draw_buf->buf1 && draw_buf->buf2) {
    if (draw_buf->buf_act == draw_buf->buf1)
      draw_buf->buf_act = draw_buf->buf2;
    else
      draw_buf->buf_act = draw_buf->buf1;
  }
}

void lcdRefresh()
{
  _draw_buf_flush(disp);
}

void lcdFlushed()
{
  // its possible to get here before flushLcd is ever called.
  // so check for nullptr first. (Race condition if you put breakpoints in startup code)
  if (refr_disp != nullptr)
    lv_disp_flush_ready(refr_disp);
}

//TODO: move this somewhere else
event_t getWindowEvent()
{
  return getEvent(false);
}
