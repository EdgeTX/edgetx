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

#if defined (LCD_VERTICAL_INVERT)
static void buf_rotate_180(lv_area_t * area, uint16_t* color_p)
{
    lv_coord_t area_w = lv_area_get_width(area);
    lv_coord_t area_h = lv_area_get_height(area);
    uint32_t total = area_w * area_h;
    /*Swap the beginning and end values*/

    uint32_t tmp;
    uint32_t* c_tail = (uint32_t*)(color_p + (total & 1));
    uint32_t* c_head = (uint32_t*)color_p;
    uint32_t i = total/2 - 1, j = 0;
    while(i > j) {
        tmp = c_tail[i];
        c_tail[i] = ((c_head[j] & 0xFFFF0000) >> 16) | ((c_head[j] & 0xFFFF) << 16);
        c_head[j] = ((tmp & 0xFFFF0000) >> 16) | ((tmp & 0xFFFF) << 16);
        i--;
        j++;
    }
    if (total & 1) {
        uint16_t tmp = color_p[i * 2 + 1];
        color_p[j * 2] = color_p[i * 2 + 1];
        color_p[i * 2 + 1] = tmp;
    }

    lv_coord_t tmp_coord;
    tmp_coord = area->y2;
    area->y2 = LCD_H - area->y1 - 1;
    area->y1 = LCD_H - tmp_coord - 1;
    tmp_coord = area->x2;
    area->x2 = LCD_W - area->x1 - 1;
    area->x1 = LCD_W - tmp_coord - 1;
}
#endif

static void flushLcd(lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
  lv_area_t refr_area;
  lv_area_copy(&refr_area, area);

#if defined(DEBUG_WINDOWS)
  if (refr_area.x1 != 0 || refr_area.x2 != LCD_W-1 || refr_area.y1 != 0 ||
      refr_area.y2 != LCD_H-1) {
    TRACE("partial refresh @ 0x%p {%d,%d,%d,%d}", color_p, refr_area.x1,
          refr_area.y1, refr_area.x2, refr_area.y2);
  } else {
    TRACE("full refresh @ 0x%p", color_p);
  }
#endif

  if (lcd_flush_cb) {
    refr_disp = disp_drv;

#if defined (LCD_VERTICAL_INVERT)
    buf_rotate_180(&refr_area, (uint16_t*)color_p);
#endif

    rect_t copy_area = {refr_area.x1, refr_area.y1,
                        refr_area.x2 - refr_area.x1 + 1,
                        refr_area.y2 - refr_area.y1 + 1};

    lcd_flush_cb(disp_drv, (uint16_t*)color_p, copy_area);
  } else {
    lv_disp_flush_ready(disp_drv);
  }
}

void lcdInitDisplayDriver()
{
  lv_init();

  // Clear buffers first (is that really needed?)
  memset(LCD_FIRST_FRAME_BUFFER, 0, sizeof(LCD_FIRST_FRAME_BUFFER));
  memset(LCD_SECOND_FRAME_BUFFER, 0, sizeof(LCD_SECOND_FRAME_BUFFER));
  
  lv_disp_draw_buf_init(&disp_buf, lcdFront->getData(), lcd->getData(), LCD_W*LCD_H);
  lv_disp_drv_init(&disp_drv);            /*Basic initialization*/

  disp_drv.draw_buf = &disp_buf;          /*Set an initialized buffer*/
  disp_drv.flush_cb = flushLcd;           /*Set a flush callback to draw to the display*/
  disp_drv.wait_cb = lcd_wait_cb;         /*Set a wait callback*/

  disp_drv.hor_res = LCD_W;               /*Set the horizontal resolution in pixels*/
  disp_drv.ver_res = LCD_H;               /*Set the vertical resolution in pixels*/
  disp_drv.full_refresh = 0;
  disp_drv.direct_mode = 0;

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
}

void lcdRefresh()
{
  _lv_inv_area(nullptr, &screen_area);
  lv_refr_now(nullptr);
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
