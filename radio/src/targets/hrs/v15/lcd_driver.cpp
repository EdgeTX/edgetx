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
#include "stm32_gpio_driver.h"
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
static volatile uint16_t lcd_phys_w = LCD_W;
static volatile uint16_t lcd_phys_h = LCD_H;

static LTDC_HandleTypeDef hltdc;
static void* initialFrameBuffer = nullptr;

static void startLcdRefresh(lv_disp_drv_t *disp_drv, uint16_t *buffer,
                            const rect_t &copy_area)
{
  (void)disp_drv;
  (void)copy_area;

  // given the data cache size, this is probably
  // faster than cleaning by address
  SCB_CleanDCache();

//  LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
  LTDC_Layer1->CFBAR = (uint32_t)buffer;
  // reload shadow registers on vertical blank
  LTDC->SRCR = LTDC_SRCR_VBR;
  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);
}

lcdSpiInitFucPtr lcdInitFunction;
lcdSpiInitFucPtr lcdOffFunction;
lcdSpiInitFucPtr lcdOnFunction;
uint32_t lcdPixelClock;

#define LTDC_HSYNC        GPIO_PIN(GPIOI, 10)
#define LTDC_VSYNC        GPIO_PIN(GPIOI, 9)
#define LTDC_CLK          GPIO_PIN(GPIOG, 7)
#define LTDC_DE           GPIO_PIN(GPIOK, 7)

#define LTDC_R3           GPIO_PIN(GPIOH, 9)
#define LTDC_R4           GPIO_PIN(GPIOH, 10)
#define LTDC_R5           GPIO_PIN(GPIOH, 11)
#define LTDC_R6           GPIO_PIN(GPIOJ, 5)
#define LTDC_R7           GPIO_PIN(GPIOG, 6)

#define LTDC_G2           GPIO_PIN(GPIOI, 15)
#define LTDC_G3           GPIO_PIN(GPIOJ, 10)
#define LTDC_G4           GPIO_PIN(GPIOH, 15)
#define LTDC_G5           GPIO_PIN(GPIOK, 0)
#define LTDC_G6           GPIO_PIN(GPIOK, 1)
#define LTDC_G7           GPIO_PIN(GPIOK, 2)

#define LTDC_B3           GPIO_PIN(GPIOJ, 15)
#define LTDC_B4           GPIO_PIN(GPIOK, 3)
#define LTDC_B5           GPIO_PIN(GPIOK, 4)
#define LTDC_B6           GPIO_PIN(GPIOK, 5)
#define LTDC_B7           GPIO_PIN(GPIOK, 6)

static void LCD_AF_GPIOConfig(void) {
  gpio_init_af(LTDC_HSYNC, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_VSYNC, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_CLK, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_DE, GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(LTDC_R3, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_R4, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_R5, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_R6, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_R7, GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(LTDC_G2, GPIO_AF9, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G3, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G4, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G5, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G6, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_G7, GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(LTDC_B3, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_B4, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_B5, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_B6, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(LTDC_B7, GPIO_AF14, GPIO_SPEED_FREQ_LOW);
}

static void LCD_NRSTConfig(void)
{
  stm32_gpio_enable_clock(LCD_NRST_GPIO);

  LL_GPIO_InitTypeDef GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_NRST_GPIO_PIN;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull       = LL_GPIO_PULL_NO;

  LL_GPIO_Init(LCD_NRST_GPIO, &GPIO_InitStructure);

}

static void lcdReset(void)
{
  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW(); //  RESET();
  delay_ms(30);

  LCD_NRST_HIGH();
  delay_ms(50);
}

void LCD_Init_LTDC() {
  hltdc.Instance = LTDC;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * lcdPixelclock * 16 = XX Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = PLLSAI_VCO/4 = YY Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = YY/4 = lcdPixelClock Mhz */
//  uint32_t clock = (lcdPixelClock*16) / 1000000; // clock*16 in MHz
//  RCC_PeriphCLKInitTypeDef clkConfig;
//  clkConfig.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
//  clkConfig.PLLI2S
//  clkConfig.PLLSAI.PLLSAIN = clock;
//  clkConfig.PLLSAI.PLLSAIR = 4;
//  clkConfig.PLLSAIDivQ = 6;
//  clkConfig.PLLSAIDivR = RCC_PLLSAIDIVR_4;
//  HAL_RCCEx_PeriphCLKConfig(&clkConfig);

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
  hltdc.Init.VerticalSync = VSW;
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

extern "C"
void lcdInit(void)
{
  LCD_NRSTConfig();

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

  __HAL_RCC_LTDC_CLK_ENABLE();

  LCD_Init_LTDC();
  LCD_LayerInit();

  // Ensure the cleared SDRAM frame buffer is visible to LTDC on first scan.
  SCB_CleanDCache();

  // Enable LCD display
  __HAL_LTDC_ENABLE(&hltdc);

  lcdSetFlushCb(startLcdRefresh);
}

extern "C" void LTDC_IRQHandler(void)
{
  __HAL_LTDC_CLEAR_FLAG(&hltdc, LTDC_FLAG_LI);
  __HAL_LTDC_DISABLE_IT(&hltdc, LTDC_IT_LI);

  lcdFlushed();
}