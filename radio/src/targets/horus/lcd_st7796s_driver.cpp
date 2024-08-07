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

#include "stm32_hal.h"
#include "stm32_hal_ll.h"
#include "stm32_gpio_driver.h"

#include "lcd_st7796s_driver.h"

#include "board.h"
#include "debug.h"
#include "delays_driver.h"
#include "dma2d.h"
#include "hal.h"
#include "lcd.h"
#include "edgetx_types.h"

static LTDC_HandleTypeDef hltdc;
static void* initialFrameBuffer = nullptr;

static volatile uint8_t _frame_addr_reloaded = 0;

static void startLcdRefresh(lv_disp_drv_t* disp_drv, uint16_t* buffer,
                            const rect_t& copy_area)
{
  (void)disp_drv;
  (void)copy_area;

  LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
  LTDC_Layer1->CFBAR = (uint32_t)buffer;
  // reload shadow registers on vertical blank
  _frame_addr_reloaded = 0;
  LTDC->SRCR = LTDC_SRCR_VBR;
  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);

  // wait for reload
  // TODO: replace through some smarter mechanism without busy wait
  while (_frame_addr_reloaded == 0);
}

uint32_t lcdPixelClock;

static void LCD_AF_GPIOConfig(void)
{
  /*
   -----------------------------------------------------------------------------
   LCD_CLK <-> PG.07 | LCD_HSYNC <-> PI.12 | LCD_R3 <-> PJ.02 | LCD_G5 <-> PK.00
   | LCD VSYNC <-> PI.13 | LCD_R4 <-> PJ.03 | LCD_G6 <-> PK.01
   |                     | LCD_R5 <-> PJ.04 | LCD_G7 <-> PK.02
   |                     | LCD_R6 <-> PJ.05 | LCD_B4 <-> PK.03
   |                     | LCD_R7 <-> PJ.06 | LCD_B5 <-> PK.04
   |                     | LCD_G2 <-> PJ.09 | LCD_B6 <-> PK.05
   |                     | LCD_G3 <-> PJ.10 | LCD_B7 <-> PK.06
   |                     | LCD_G4 <-> PJ.11 | LCD_DE <-> PK.07
   |                     | LCD_B3 <-> PJ.15 |
   */
  stm32_gpio_enable_clock(GPIOG);
  stm32_gpio_enable_clock(GPIOI);
  stm32_gpio_enable_clock(GPIOJ);
  stm32_gpio_enable_clock(GPIOK);

  LL_GPIO_InitTypeDef GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  // GPIOG configuration
  GPIO_InitStructure.Pin = LL_GPIO_PIN_7;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_ALTERNATE;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  GPIO_InitStructure.Alternate = LL_GPIO_AF_14;  // AF LTDC
  LL_GPIO_Init(GPIOG, &GPIO_InitStructure);

  // GPIOI configuration
  GPIO_InitStructure.Pin = LL_GPIO_PIN_12 | LL_GPIO_PIN_13;
  LL_GPIO_Init(GPIOI, &GPIO_InitStructure);

  // GPIOJ configuration
  GPIO_InitStructure.Pin = LL_GPIO_PIN_2 | LL_GPIO_PIN_3 | LL_GPIO_PIN_4 |
                           LL_GPIO_PIN_5 | LL_GPIO_PIN_6 | LL_GPIO_PIN_9 |
                           LL_GPIO_PIN_10 | LL_GPIO_PIN_11 | LL_GPIO_PIN_15;
  LL_GPIO_Init(GPIOJ, &GPIO_InitStructure);

  // GPIOK configuration
  GPIO_InitStructure.Pin = LL_GPIO_PIN_0 | LL_GPIO_PIN_1 | LL_GPIO_PIN_2 |
                           LL_GPIO_PIN_3 | LL_GPIO_PIN_4 | LL_GPIO_PIN_5 |
                           LL_GPIO_PIN_6 | LL_GPIO_PIN_7;
  LL_GPIO_Init(GPIOK, &GPIO_InitStructure);
}

static void lcdSpiConfig(void)
{
  stm32_gpio_enable_clock(LCD_SPI_GPIO);
  stm32_gpio_enable_clock(LCD_NRST_GPIO);

  LL_GPIO_InitTypeDef GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_SPI_SCK_GPIO_PIN | LCD_SPI_MOSI_GPIO_PIN;
  GPIO_InitStructure.Speed = LL_GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStructure.Mode = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_SPI_CS_GPIO_PIN;
  GPIO_InitStructure.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin = LCD_NRST_GPIO_PIN;
  LL_GPIO_Init(LCD_NRST_GPIO, &GPIO_InitStructure);

  /* Set the chip select pin always low */
  LCD_CS_LOW();
}

void lcdDelay() { delay_01us(1); }

static void lcdReset()
{
  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW();  // RESET();
  delay_ms(100);

  LCD_NRST_HIGH();
  delay_ms(100);
}

static void lcdWriteByte(uint8_t data_enable, uint8_t byte)
{
  LCD_SCK_LOW();

  if (data_enable) {
    LCD_MOSI_HIGH();
  } else {
    LCD_MOSI_LOW();
  }

  lcdDelay();
  LCD_SCK_HIGH();

  for (int i = 0; i < 8; i++) {
    lcdDelay();
    LCD_SCK_LOW();

    if (byte & 0x80) {
      LCD_MOSI_HIGH();
    } else {
      LCD_MOSI_LOW();
    }

    lcdDelay();
    LCD_SCK_HIGH();
    byte <<= 1;
  }

  lcdDelay();
  LCD_SCK_LOW();
}

void lcdWriteCommand(uint8_t command)
{
  LCD_CS_LOW();
  lcdWriteByte(0, command);
  LCD_CS_HIGH();
  lcdDelay();
}

void lcdWriteData(uint8_t data)
{
  LCD_CS_LOW();
  lcdWriteByte(1, data);
  LCD_CS_HIGH();
  lcdDelay();
}

void LCD_ST7796S_On(void) { lcdWriteCommand(0x29); }

void LCD_ST7796S_Init(void)
{
  delay_ms(120);

  lcdWriteCommand(0x11);
  delay_ms(120);

  lcdWriteCommand(0xF0);
  lcdWriteData(0xC3);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x96);

  lcdWriteCommand(0x36);
  lcdWriteData(0x28);

  lcdWriteCommand(0x2A);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0xDF);
  lcdWriteCommand(0x2B);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x01);
  lcdWriteData(0x3F);

  lcdWriteCommand(0x3A);
#if defined(RADIO_T15)
  lcdWriteData(0x55);
#else
  lcdWriteData(0x66);
#endif

  lcdWriteCommand(0x34);

  // SET RGB STRAT
  lcdWriteCommand(0xB0);  // SET HS VS DE CLK 上升还是下降有效
  lcdWriteData(0x80);

  lcdWriteCommand(0xB4);
  lcdWriteData(0x01);

  lcdWriteCommand(0xB6);
  //  lcdWriteData( 0x20 );
  //  lcdWriteData( 0x02 );
  //  lcdWriteData( 0x3B );
  lcdWriteData(0x20);
  lcdWriteData(0x02);
  lcdWriteData(0x3B);
  // SET RGB END

  lcdWriteCommand(0xB7);
  lcdWriteData(0xC6);

  lcdWriteCommand(0xB9);
  lcdWriteData(0x02);
  lcdWriteData(0xE0);

  lcdWriteCommand(0xC0);
  lcdWriteData(0x80);
  lcdWriteData(0x65);

  lcdWriteCommand(0xC1);
  lcdWriteData(0x0D);

  lcdWriteCommand(0xC2);
  lcdWriteData(0xA7);

  lcdWriteCommand(0xC5);
  lcdWriteData(0x14);

  lcdWriteCommand(0xE8);
  lcdWriteData(0x40);
  lcdWriteData(0x8A);
  lcdWriteData(0x00);
  lcdWriteData(0x00);
  lcdWriteData(0x29);
  lcdWriteData(0x19);
  lcdWriteData(0xA5);
  lcdWriteData(0x33);

  lcdWriteCommand(0xE0);
  lcdWriteData(0xD0);
  lcdWriteData(0x00);
  lcdWriteData(0x04);
  lcdWriteData(0x05);
  lcdWriteData(0x04);
  lcdWriteData(0x21);
  lcdWriteData(0x25);
  lcdWriteData(0x43);
  lcdWriteData(0x3F);
  lcdWriteData(0x37);
  lcdWriteData(0x13);
  lcdWriteData(0x13);
  lcdWriteData(0x29);
  lcdWriteData(0x32);

  lcdWriteCommand(0xE1);
  lcdWriteData(0xD0);
  lcdWriteData(0x04);
  lcdWriteData(0x06);
  lcdWriteData(0x09);
  lcdWriteData(0x06);
  lcdWriteData(0x03);
  lcdWriteData(0x25);
  lcdWriteData(0x32);
  lcdWriteData(0x3E);
  lcdWriteData(0x18);
  lcdWriteData(0x15);
  lcdWriteData(0x15);
  lcdWriteData(0x2B);
  lcdWriteData(0x30);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x3C);

  lcdWriteCommand(0xF0);
  lcdWriteData(0x69);

  delay_ms(120);

#if defined(RADIO_T15)
  if (hardwareOptions.pcbrev == PCBREV_T15_IPS) lcdWriteCommand(0x21);
#endif

  LCD_ST7796S_On();
}

void LCD_ST7796S_Off(void) { lcdWriteCommand(0x28); }

void LCD_Init_LTDC()
{
  __HAL_RCC_LTDC_CLK_ENABLE();
  hltdc.Instance = LTDC;

  /* Configure PLLSAI prescalers for LCD */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * lcdPixelclock * 16 = XX Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLL_LTDC = PLLSAI_VCO/4 = YY Mhz */
  /* LTDC clock frequency = PLLLCDCLK / RCC_PLLSAIDivR = YY/4 = lcdPixelClock
   * Mhz */
  uint32_t clock = (lcdPixelClock * 16) / 1000000;  // clock*16 in MHz
  RCC_PeriphCLKInitTypeDef clkConfig;
  clkConfig.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
  clkConfig.PLLSAI.PLLSAIN = clock;
  clkConfig.PLLSAI.PLLSAIR = 4;
  clkConfig.PLLSAIDivQ = 6;
  clkConfig.PLLSAIDivR = RCC_PLLSAIDIVR_4;
  HAL_RCCEx_PeriphCLKConfig(&clkConfig);

  /* LTDC Configuration
   * *********************************************************/
  /* Polarity configuration */
  /* Initialize the horizontal synchronization polarity as active low */
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  /* Initialize the vertical synchronization polarity as active low */
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  /* Initialize the data enable polarity as active low */
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  /* Initialize the pixel clock polarity as input pixel clock */
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IIPC;

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
  hltdc.Init.AccumulatedActiveW = LCD_H + HBP;
  /* Configure accumulated active height */
  hltdc.Init.AccumulatedActiveH = LCD_W + VBP;
  /* Configure total width */
  hltdc.Init.TotalWidth = LCD_H + HBP + HFP;
  /* Configure total height */
  hltdc.Init.TotalHeigh = LCD_W + VBP + VFP;

  HAL_LTDC_Init(&hltdc);

  // Configure IRQ (line)
  NVIC_SetPriority(LTDC_IRQn, LTDC_IRQ_PRIO);
  NVIC_EnableIRQ(LTDC_IRQn);

  // Trigger on last line
  HAL_LTDC_ProgramLineEvent(&hltdc, LCD_W);
  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);
}

void LCD_LayerInit()
{
  auto& layer = hltdc.LayerCfg[0];

  /* Windowing configuration */
  layer.WindowX0 = 0;
  layer.WindowX1 = LCD_H;
  layer.WindowY0 = 0;
  layer.WindowY1 = LCD_W;

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

  layer.ImageWidth = LCD_H;
  layer.ImageHeight = LCD_W;

  /* Start Address configuration : the LCD Frame buffer is defined on SDRAM w/
   * Offset */
  layer.FBStartAdress = (intptr_t)initialFrameBuffer;

  /* Initialize LTDC layer 1 */
  HAL_LTDC_ConfigLayer(&hltdc, &hltdc.LayerCfg[0], 0);

  /* dithering activation */
  HAL_LTDC_EnableDither(&hltdc);
}

extern "C" void lcdSetInitalFrameBuffer(void* fbAddress)
{
  initialFrameBuffer = fbAddress;
}

extern "C" void lcdInit(void)
{
  /* Configure the LCD SPI+RESET pins */
  lcdSpiConfig();

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();

  /* Send LCD initialization commands */
  lcdPixelClock = 12000000;

  LCD_ST7796S_Init();

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
