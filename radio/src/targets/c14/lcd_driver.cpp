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

#include "stm32_gpio.h"
#include "hal/gpio.h"

uint8_t TouchControllerType = 0;  // 0: other; 1: CST836U
static volatile uint16_t lcd_phys_w = LCD_PHYS_W;
static volatile uint16_t lcd_phys_h = LCD_PHYS_H;

static LTDC_HandleTypeDef hltdc;
static void* initialFrameBuffer = nullptr;

static volatile uint8_t _frame_addr_reloaded = 0;

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
  (void)disp_drv;
  (void)copy_area;

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


static void LCD_AF_GPIOConfig(void) {
  /**LTDC GPIO Configuration
  PI10     ------> LTDC_HSYNC
  PF10     ------> LTDC_DE
  PA4     ------> LTDC_VSYNC
  PH9     ------> LTDC_R3
  PH10     ------> LTDC_R4
  PH11     ------> LTDC_R5
  PH12     ------> LTDC_R6
  PG6     ------> LTDC_R7
  PG7     ------> LTDC_CLK
  PH13     ------> LTDC_G2
  PH14     ------> LTDC_G3
  PH15     ------> LTDC_G4
  PI0     ------> LTDC_G5
  PI1     ------> LTDC_G6
  PI2     ------> LTDC_G7
  PG11     ------> LTDC_B3
  PI4     ------> LTDC_B4
  PI5     ------> LTDC_B5
  PI6     ------> LTDC_B6
  PI7     ------> LTDC_B7
  */

  gpio_init_af(GPIO_PIN(GPIOI, 0), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 1), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 2), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 4), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 5), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 6), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 7), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 10), GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(GPIO_PIN(GPIOF, 10), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 9), GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(GPIO_PIN(GPIOH,  9), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOH, 10), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOH, 11), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOH, 12), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOH, 13), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOH, 14), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOH, 15), GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(GPIO_PIN(GPIOG,  6), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOG,  7), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOG, 11), GPIO_AF14, GPIO_SPEED_FREQ_LOW);

}

inline void LCD_NRST_LOW()  { gpio_clear(LCD_GPIO_NRST); }
inline void LCD_NRST_HIGH() { gpio_set(LCD_GPIO_NRST);   }

static void lcdReset()
{
  LCD_NRST_HIGH();
  delay_ms(1);
  LCD_NRST_LOW();
  delay_ms(20);
  LCD_NRST_HIGH();
  delay_ms(30);
}

void LCD_Init_LTDC() {
  __HAL_RCC_LTDC_CLK_ENABLE();
  hltdc.Instance = LTDC;

  /* LTDC Configuration *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  /* Initialize the vertical synchronization polarity as active low */
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /* Initialize the data enable polarity as active low */
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;


  /* Configure R,G,B component values for LCD background color */
  hltdc.Init.Backcolor.Red = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Blue = 0;

  /* Configure horizontal synchronization width */
  hltdc.Init.HorizontalSync = HSW;
  /* Configure vertical synchronization height */
  hltdc.Init.VerticalSync = VSH;

  /* Configure accumulated horizontal back porch */
  hltdc.Init.AccumulatedHBP = HBP;
  /* Configure accumulated vertical back porch */
  hltdc.Init.AccumulatedVBP = VBP;
  /* Configure accumulated active width */
  hltdc.Init.AccumulatedActiveW = lcd_phys_w + HBP;
  /* Configure accumulated active height */
  hltdc.Init.AccumulatedActiveH = lcd_phys_h + VBP;
  /* Configure total width */
  hltdc.Init.TotalWidth = lcd_phys_w + HBP + HFP;
  /* Configure total height */
  hltdc.Init.TotalHeigh = lcd_phys_h + VBP + VFP;

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
  layer.FBStartAdress = (intptr_t)initialFrameBuffer;

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

extern "C" void LCD_NRSTConfig(void)
{
  gpio_init(LCD_GPIO_NRST, GPIO_OUT, GPIO_PIN_SPEED_LOW);
}

void lcdInit(void)
{
  LCD_NRSTConfig();
  lcdReset();
  LCD_AF_GPIOConfig();

  LCD_Init_LTDC();
  LCD_LayerInit();

  __HAL_LTDC_ENABLE(&hltdc);

  lcdSetFlushCb(startLcdRefresh);
}

extern "C" void LTDC_IRQHandler(void)
{
  __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_LI);
  __HAL_LTDC_DISABLE_IT(&hltdc, LTDC_IT_LI);
  _frame_addr_reloaded = 1;
}
