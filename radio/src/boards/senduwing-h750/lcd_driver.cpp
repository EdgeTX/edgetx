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
#include "hal.h"
#include "delays_driver.h"
#include "debug.h"
#include "lcd.h"
#include "stm32_lcd_flush.h"
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

#if LCD_VERTICAL_INVERT
// The H17 panel is mounted upside-down, exactly like the tx16s/horus panel.
// LTDC cannot rotate: each refreshed area is rotated by 180 degrees into the
// frame buffers (see lcd_flush.h).
#define LCD_ROTATION LCD_ROTATE_180
static pixel_t _rotate_tiles[LCD_FLUSH_TILES_SIZE] __DMA;
#else
#define LCD_ROTATION LCD_ROTATE_NONE
#define _rotate_tiles nullptr
#endif

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
  layer.FBStartAdress =
      (intptr_t)lcdFlusherInit(LCD_ROTATION, _rotate_tiles);

  /* Initialize LTDC layer 1 */
  HAL_LTDC_ConfigLayer(&hltdc, &hltdc.LayerCfg[0], 0);

  /* dithering activation */
  HAL_LTDC_EnableDither(&hltdc);
}

extern "C"
void lcdSetInitalFrameBuffer(void* fbAddress)
{
  // LVGL's buffer is never scanned out
  (void)fbAddress;
}

const char* boardLcdType = "";

extern "C"
void lcdInit(void)
{
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

  // LVGL renders into a draw buffer of its own, copied into the frame
  // buffers (see lcd_flush.h)
  lcdSetFlushCb(lcdFlusherFlush);
}
