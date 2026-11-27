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

#include "stm32_lcd_flush.h"

#include "stm32_hal_ll.h"
#include "stm32_hal.h"

#include "hal.h"
#include "lcd.h"
#include "board.h"
#include "timers_driver.h"

// The frame buffers are only ever written by the DMA2D: no D-cache
// maintenance on them, and the DMA2D is programmed directly, as
// DMACopyBitmap() would clean and invalidate the whole D-cache for each copy.
// Not LVGL's with LCD_FLUSHER (see lcd.cpp).
extern pixel_t LCD_FIRST_FRAME_BUFFER[DISPLAY_BUFFER_SIZE];
extern pixel_t LCD_SECOND_FRAME_BUFFER[DISPLAY_BUFFER_SIZE];

static LcdFlusher _flusher;

// The LTDC scans a frame in less than 20ms: wait a couple of frames before
// giving up
#define LCD_RELOAD_TIMEOUT_MS 50

static void _dma_wait()
{
  while (DMA2D->CR & DMA2D_CR_START);
}

static void _dma_copy(pixel_t* dst, uint32_t dst_stride, const pixel_t* src,
                      uint32_t src_stride, uint32_t w, uint32_t h)
{
  _dma_wait();
  DMA2D->CR = LL_DMA2D_MODE_M2M;
  DMA2D->FGMAR = (uint32_t)src;
  DMA2D->FGOR = src_stride - w;
  DMA2D->FGPFCCR = LL_DMA2D_INPUT_MODE_RGB565;
  DMA2D->OPFCCR = LL_DMA2D_OUTPUT_MODE_RGB565;
  DMA2D->OMAR = (uint32_t)dst;
  DMA2D->OOR = dst_stride - w;
  DMA2D->NLR = (w << DMA2D_NLR_PL_Pos) | h;
  DMA2D->CR |= DMA2D_CR_START;
}

static void _clean(const void* p, uint32_t len)
{
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
  SCB_CleanDCache_by_Addr((void*)p, (int32_t)len);
#else
  (void)p;
  (void)len;
#endif
}

static void _ltdc_show(pixel_t* buffer)
{
  LTDC_Layer1->CFBAR = (uint32_t)buffer;
  // reload shadow registers on vertical blank
  LTDC->SRCR = LTDC_SRCR_VBR;
}

static void _ltdc_wait_shown()
{
  // VBR clears at the actual reload. Not delay_us(): it stops when a
  // debugger disables the DWT cycle counter.
  uint32_t start = timersGetMsTick();
  while (LTDC->SRCR & LTDC_SRCR_VBR) {
    if (timersGetMsTick() - start > LCD_RELOAD_TIMEOUT_MS) {
      // LTDC stopped? Better a torn frame than a hung UI.
      LTDC->SRCR = LTDC_SRCR_IMR;
      break;
    }
  }
}

static const LcdFlushOps _ops = {
    _dma_copy, _dma_wait, _clean, _ltdc_show, _ltdc_wait_shown,
};

pixel_t* lcdFlusherInit(LcdRotation rotation, pixel_t* tiles)
{
  _flusher.init(&_ops, rotation, LCD_FIRST_FRAME_BUFFER,
                LCD_SECOND_FRAME_BUFFER, tiles);
  return _flusher.frontBuffer();
}

void lcdFlusherFlush(lv_disp_drv_t* disp_drv, uint16_t* buffer,
                     const rect_t& area)
{
  _flusher.flush(disp_drv, buffer, area);

  // LVGL's buffer has been consumed: it can draw the next area
  lcdFlushed();
}
