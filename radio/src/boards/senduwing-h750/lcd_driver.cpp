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

#include "stm32_hal_ll.h"
#include "stm32_hal.h"
#include "edgetx_types.h"
#include "dma2d.h"
#include "hal.h"
#include "delays_driver.h"
#include "debug.h"
#include "lcd.h"
#include "lcd_driver.h"
#include "board.h"

#include <lvgl/lvgl.h>

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_qspi.h"

uint8_t TouchControllerType = 0;  // 0: other; 1: CST836U
static volatile uint16_t lcd_phys_w = LCD_PHYS_W;
static volatile uint16_t lcd_phys_h = LCD_PHYS_H;

static LTDC_HandleTypeDef hltdc;
static void* initialFrameBuffer = nullptr;

static volatile uint8_t _frame_addr_reloaded = 0;

#if defined(LCD_VERTICAL_INVERT)
// The H17 panel is mounted upside-down, exactly like the tx16s/horus panel.
// LTDC cannot rotate, so we software-rotate every refreshed area 180 into a
// dedicated back buffer and swap it in on the last flush of a frame. Mirrors
// radio/src/targets/horus/lcd_driver.cpp.
typedef uint16_t pixel_t;
static pixel_t _LCD_BUF_1[DISPLAY_BUFFER_SIZE] __SDRAM;
static pixel_t _LCD_BUF_2[DISPLAY_BUFFER_SIZE] __SDRAM;

static pixel_t _line_buffer[LCD_W];

static uint16_t* _front_buffer = _LCD_BUF_1;
static uint16_t* _back_buffer = _LCD_BUF_2;

// Copy 2 pixels at once to speed up a little
static void _copy_rotate_180(uint16_t* dst, uint16_t* src, const rect_t& copy_area)
{
  coord_t x1 = LCD_W - copy_area.w - copy_area.x;
  coord_t y1 = LCD_H - copy_area.h - copy_area.y;

  src += copy_area.w - 2;
  dst += (y1 + copy_area.h - 1) * LCD_W + x1;

  for (auto line = 0; line < copy_area.h; line++) {

    // invert line into _line_buffer first (SRAM)
    auto px_dst = _line_buffer;

    auto line_end = px_dst + (copy_area.w & ~1);
    while (px_dst != line_end) {
      uint32_t* px2_src = (uint32_t*)src;

      *((uint32_t*)px_dst) = ((*px2_src & 0xFFFF0000) >> 16) | ((*px2_src & 0xFFFF) << 16);

      src -= 2;
      px_dst += 2;
    }

    if (copy_area.w & 1) {
      *px_dst = *(src+1);
      src--;
    }

    // ... and DMA back into SDRAM
    DMACopyBitmap(dst, copy_area.w, 1, 0, 0,
                  _line_buffer, copy_area.w, 1, 0, 0,
                  copy_area.w, 1);

    src += copy_area.w * 2;
    dst -= LCD_W;
  }
}

static void _rotate_area_180(lv_area_t& area)
{
  lv_coord_t tmp_coord;
  tmp_coord = area.y2;
  area.y2 = LCD_H - area.y1 - 1;
  area.y1 = LCD_H - tmp_coord - 1;
  tmp_coord = area.x2;
  area.x2 = LCD_W - area.x1 - 1;
  area.x1 = LCD_W - tmp_coord - 1;
}
#endif

static void _update_frame_buffer_addr(uint16_t* buffer)
{
  // given the data cache size, this is probably
  // faster than cleaning by address
  SCB_CleanDCache();

  LTDC_Layer1->CFBAR = (uint32_t)buffer;
  // reload shadow registers on vertical blank
  _frame_addr_reloaded = 0;
  LTDC->SRCR = LTDC_SRCR_VBR;

  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);

  // wait for reload
  // TODO: replace through some smarter mechanism without busy wait
  while(_frame_addr_reloaded == 0);
}

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
#if defined(LCD_VERTICAL_INVERT)
  _copy_rotate_180(_back_buffer, buffer, copy_area);

  if (lv_disp_flush_is_last(disp_drv)) {

    // swap back/front
    if (_front_buffer == _LCD_BUF_1) {
      _front_buffer = _LCD_BUF_2;
      _back_buffer = _LCD_BUF_1;
    } else {
      _front_buffer = _LCD_BUF_1;
      _back_buffer = _LCD_BUF_2;
    }

    // Trigger async refresh
    _update_frame_buffer_addr(_front_buffer);

    // Copy refreshed & rotated areas into new back buffer
    uint16_t* src = _front_buffer;
    uint16_t* dst = _back_buffer;

    lv_disp_t* disp = _lv_refr_get_disp_refreshing();
    for(int i = 0; i < disp->inv_p; i++) {
      if(disp->inv_area_joined[i]) continue;

      lv_area_t refr_area;
      lv_area_copy(&refr_area, &disp->inv_areas[i]);

      _rotate_area_180(refr_area);

      auto area_w = refr_area.x2 - refr_area.x1 + 1;
      auto area_h = refr_area.y2 - refr_area.y1 + 1;

      DMACopyBitmap(dst, LCD_W, LCD_H, refr_area.x1, refr_area.y1,
                    src, LCD_W, LCD_H, refr_area.x1, refr_area.y1,
                    area_w, area_h);
    }
  }
#else
  (void)disp_drv;
  (void)copy_area;

  _update_frame_buffer_addr(buffer);
#endif

  // Since #7483 the shared LVGL flush path (gui/colorlcd/lcd.cpp) no longer
  // signals completion after invoking the flush callback; the driver must call
  // lcdFlushed() itself, otherwise LVGL's flush stays pending forever and the
  // UI freezes on the first frame (e.g. stuck on the boot logo).
  lcdFlushed();
}

// LTDC RGB666 data + sync lines (all on AF14). Layout matches the tx15/rm-h750
// panel wiring: PI12-14 (HSYNC/VSYNC/CLK), PJ1-6 (R2-R7), PJ9-11 + PK0-2
// (G2-G7), PJ14-15 + PK3-6 (B2-B7), PK7 (DE).
gpio_t _lcd_af_gpios[] = {
    GPIO_PIN(GPIOI, 12), GPIO_PIN(GPIOI, 13), GPIO_PIN(GPIOI, 14),

    GPIO_PIN(GPIOJ, 1),  GPIO_PIN(GPIOJ, 2),  GPIO_PIN(GPIOJ, 3),
    GPIO_PIN(GPIOJ, 4),  GPIO_PIN(GPIOJ, 5),  GPIO_PIN(GPIOJ, 6),
    GPIO_PIN(GPIOJ, 9),  GPIO_PIN(GPIOJ, 10), GPIO_PIN(GPIOJ, 11),
    GPIO_PIN(GPIOJ, 14), GPIO_PIN(GPIOJ, 15),

    GPIO_PIN(GPIOK, 0),  GPIO_PIN(GPIOK, 1),  GPIO_PIN(GPIOK, 2),
    GPIO_PIN(GPIOK, 3),  GPIO_PIN(GPIOK, 4),  GPIO_PIN(GPIOK, 5),
    GPIO_PIN(GPIOK, 6),  GPIO_PIN(GPIOK, 7),
};

static void LCD_AF_GPIOConfig(void)
{
  for (unsigned i = 0; i < sizeof(_lcd_af_gpios) / sizeof(_lcd_af_gpios[0]); i++) {
    gpio_init_af(_lcd_af_gpios[i], GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  }
}

static void lcdResetConfig(void)
{
  LL_GPIO_InitTypeDef GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_RESET_GPIO_PIN;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LCD_RESET_GPIO, &GPIO_InitStructure);
}

static void lcdReset() {
  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW(); // RESET();
  delay_ms(10);

  LCD_NRST_HIGH();
  delay_ms(100);
}

void LCD_Init_LTDC() {
  hltdc.Instance = LTDC;

  /* LTDC pixel clock is driven from PLL3R (see system_clock.c) */

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  /* Initialize the vertical synchronization polarity as active low */
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /* Initialize the data enable polarity as active low */
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  /* Initialize the pixel clock polarity as input pixel clock.
     tx16s/horus drives this panel with IPC (non-inverted); jumper-h750 used
     IIPC for a different panel. */
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

  /* Configure R,G,B component values for LCD background color */
  hltdc.Init.Backcolor.Red = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Blue = 0;

  /* Configure horizontal synchronization width */
  hltdc.Init.HorizontalSync = HSW-1;
  /* Configure vertical synchronization height */
  hltdc.Init.VerticalSync = VSH-1;
  /* Configure accumulated horizontal back porch */
  hltdc.Init.AccumulatedHBP = HSW+HBP-1;
  /* Configure accumulated vertical back porch */
  hltdc.Init.AccumulatedVBP = VSH+VBP-1;
  /* Configure accumulated active width */
  hltdc.Init.AccumulatedActiveW = lcd_phys_w + HBP+HSW-1;
  /* Configure accumulated active height */
  hltdc.Init.AccumulatedActiveH = lcd_phys_h + VBP+VSH-1;
  /* Configure total width */
  hltdc.Init.TotalWidth = lcd_phys_w + HBP + HFP + HSW -1;
  /* Configure total height */
  hltdc.Init.TotalHeigh = lcd_phys_h + VBP + VFP+VSH-1;

  HAL_LTDC_Init(&hltdc);

  // Configure IRQ (line)
  NVIC_SetPriority(LTDC_IRQn, LTDC_IRQ_PRIO);
  NVIC_EnableIRQ(LTDC_IRQn);

  // Trigger on last line
  HAL_LTDC_ProgramLineEvent(&hltdc, lcd_phys_h);
  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);
}

void LCD_LayerInit() {
  auto& layer = hltdc.LayerCfg[0];

  /* Windowing configuration */
  layer.WindowX0 = 0;
  layer.WindowX1 = lcd_phys_w;
  layer.WindowY0 = 0;
  layer.WindowY1 = lcd_phys_h;

  /* Pixel Format configuration*/
  layer.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;

  /* Alpha constant (255 totally opaque) */
  layer.Alpha = 255;

  /* Default Color configuration (configure A,R,G,B component values) */
  layer.Backcolor.Blue = 0;
  layer.Backcolor.Green = 0;
  layer.Backcolor.Red = 0;
  layer.Alpha0 = 0;

  /* Configure blending factors */
  layer.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  layer.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;

  layer.ImageWidth = lcd_phys_w;
  layer.ImageHeight = lcd_phys_h;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/ Offset */
#if defined(LCD_VERTICAL_INVERT)
  layer.FBStartAdress = (intptr_t)_LCD_BUF_1;
#else
  layer.FBStartAdress = (intptr_t)initialFrameBuffer;
#endif

  /* Initialize LTDC layer 1 */
  HAL_LTDC_ConfigLayer(&hltdc, &hltdc.LayerCfg[0], 0);

  /* dithering activation */
  HAL_LTDC_EnableDither(&hltdc);
}

extern "C"
void lcdSetInitalFrameBuffer(void* fbAddress)
{
  initialFrameBuffer = fbAddress;
}

const char* boardLcdType = "";

extern "C"
void lcdInit(void)
{
#if defined(LCD_VERTICAL_INVERT)
  // Clear buffers first
  memset(_LCD_BUF_1, 0, sizeof(_LCD_BUF_1));
  memset(_LCD_BUF_2, 0, sizeof(_LCD_BUF_2));
#endif

  /* Configure the LCD reset pin */
  lcdResetConfig();

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LTDC RGB control pins */
  LCD_AF_GPIOConfig();

  TRACE("LCD INIT: parallel RGB (LTDC)");
  boardLcdType = "RGB (LTDC)";

  __HAL_RCC_LTDC_CLK_ENABLE();

  LCD_Init_LTDC();
  LCD_LayerInit();

  // Enable LCD display
  __HAL_LTDC_ENABLE(&hltdc);

  lcdSetFlushCb(startLcdRefresh);
}

extern "C" void LTDC_IRQHandler(void)
{
  __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_LI);
  __HAL_LTDC_DISABLE_IT(&hltdc, LTDC_IT_LI);
  _frame_addr_reloaded = 1;
}
