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

//  LTDC_Layer1->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
  LTDC_Layer1->CFBAR = (uint32_t)buffer;
  // reload shadow registers on vertical blank
  _frame_addr_reloaded = 0;
  LTDC->SRCR = LTDC_SRCR_VBR;
  __HAL_LTDC_ENABLE_IT(&hltdc, LTDC_IT_LI);

  // wait for reload
  // TODO: replace through some smarter mechanism without busy wait
  while(_frame_addr_reloaded == 0);
}

lcdSpiInitFucPtr lcdInitFunction;
lcdSpiInitFucPtr lcdOffFunction;
lcdSpiInitFucPtr lcdOnFunction;
uint32_t lcdPixelClock;

volatile uint8_t LCD_ReadBuffer[24] = { 0, 0 };

static void LCD_Delay(void) {
  volatile unsigned int i;

  for (i = 0; i < 20; i++) {
    ;
  }
}

static void LCD_AF_GPIOConfig(void) {
  //HAL_RCCEx_PeriphCLKConfig();

  gpio_init_af(GPIO_PIN(GPIOI, 12), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 13), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOI, 14), GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(GPIO_PIN(GPIOJ, 1), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 2), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 3), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 4), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 5), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 6), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 9), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 10), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 11), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 14), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOJ, 15), GPIO_AF14, GPIO_SPEED_FREQ_LOW);

  gpio_init_af(GPIO_PIN(GPIOK, 0), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 1), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 2), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 3), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 4), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 5), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 6), GPIO_AF14, GPIO_SPEED_FREQ_LOW);
  gpio_init_af(GPIO_PIN(GPIOK, 7), GPIO_AF14, GPIO_SPEED_FREQ_LOW);

}

static void lcdSpiConfig(void) {
  LL_GPIO_InitTypeDef GPIO_InitStructure;
  LL_GPIO_StructInit(&GPIO_InitStructure);

  GPIO_InitStructure.Pin        = LCD_SPI_SCK_GPIO_PIN | LCD_SPI_MOSI_GPIO_PIN;
  GPIO_InitStructure.Speed      = LL_GPIO_SPEED_FREQ_LOW;
  GPIO_InitStructure.Mode       = LL_GPIO_MODE_OUTPUT;
  GPIO_InitStructure.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
  GPIO_InitStructure.Pull       = LL_GPIO_PULL_NO;
  LL_GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin        = LCD_SPI_CS_GPIO_PIN;
  LL_GPIO_Init(LCD_SPI_CS_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.Pin        = LCD_RESET_GPIO_PIN;
  LL_GPIO_Init(LCD_RESET_GPIO, &GPIO_InitStructure);

  LCD_CS_HIGH();
}

void lcdDelay() {
  delay_01us(1);
}

static void lcdReset() {
  LCD_CS_HIGH();

  LCD_NRST_HIGH();
  delay_ms(1);

  LCD_NRST_LOW(); // RESET();
  delay_ms(100);

  LCD_NRST_HIGH();
  delay_ms(100);
}

unsigned char LCD_ReadByteOnFallingEdge(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  LCD_MOSI_HIGH();
  LCD_MOSI_AS_INPUT();

  for (i = 0; i < 8; i++) {
    LCD_DELAY();
    LCD_SCK_HIGH();
    LCD_DELAY();
    LCD_DELAY();
    ReceiveData <<= 1;

    LCD_SCK_LOW();
    LCD_DELAY();
    LCD_DELAY();
    if (LCD_READ_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }

  LCD_MOSI_AS_OUTPUT();

  return (ReceiveData);
}

static void lcdWriteByte(uint8_t data_enable, uint8_t byte) {

  LCD_SCK_LOW();
  lcdDelay();

  if (data_enable) {
    LCD_MOSI_HIGH();
  } else {
    LCD_MOSI_LOW();
  }

  LCD_SCK_HIGH();
  lcdDelay();

  for (int i = 0; i < 8; i++) {
    LCD_SCK_LOW();
    lcdDelay();

    if (byte & 0x80) {
      LCD_MOSI_HIGH();
    } else {
      LCD_MOSI_LOW();
    }

    LCD_SCK_HIGH();
    byte <<= 1;

    lcdDelay();
  }

  LCD_SCK_LOW();
}

unsigned char LCD_ReadByte(void) {
  unsigned int i;
  unsigned char ReceiveData = 0;

  LCD_MOSI_HIGH();
  LCD_MOSI_AS_INPUT();
  for (i = 0; i < 8; i++) {
    LCD_SCK_LOW();
    lcdDelay();
    ReceiveData <<= 1;
    LCD_SCK_HIGH();
    lcdDelay();
    if (LCD_READ_DATA_PIN()) {
      ReceiveData |= 0x01;
    }
  }
  LCD_SCK_LOW();
  LCD_MOSI_AS_OUTPUT();
  return (ReceiveData);
}

unsigned char LCD_ReadRegister(unsigned char Register) {
  unsigned char ReadData = 0;
  LCD_CS_LOW();
  lcdDelay();

  lcdWriteByte(0, Register);
  lcdDelay();
  lcdDelay();
  ReadData = LCD_ReadByte();
  LCD_CS_HIGH();
  return (ReadData);
}

void lcdWriteCommand(uint8_t command) {
  lcdWriteByte(0, command);
}

void lcdWriteData(uint8_t data) {
  lcdWriteByte(1, data);
}

void LCD_ST7365_On(void) {
  LCD_CS_LOW();
  delay_ms(1);
  lcdWriteCommand(0x29);
  LCD_CS_HIGH();
}

#define SPI_WriteComm lcdWriteCommand
#define SPI_WriteData lcdWriteData

void LCD_ST7365_Init(void) {

  delay_ms(120);
  LCD_CS_LOW();
  delay_ms(1);
  lcdWriteCommand( 0x11 );
  delay_ms(120);

  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0xC3 );

  lcdWriteCommand( 0xF0 );
  lcdWriteData( 0x96 );


  SPI_WriteComm(0x11);

  delay_ms(120);

  SPI_WriteComm(0xF0);     // Command Set Control
  SPI_WriteData(0xC3);

  SPI_WriteComm(0xF0);
  SPI_WriteData(0x96);

  SPI_WriteComm(0x36);     // Memory Data Access Control MY,MX~~
  SPI_WriteData(0xE8);

  SPI_WriteComm(0x2A);//480-1
  SPI_WriteData(0x00);
  SPI_WriteData(0x00);
  SPI_WriteData(0x01);
  SPI_WriteData(0xDF);

  SPI_WriteComm(0x2B);
  SPI_WriteData(0x00);
  SPI_WriteData(0x00);
  SPI_WriteData(0x01);
  SPI_WriteData(0x3F); //320-1


  SPI_WriteComm(0x3A);
  SPI_WriteData(0x66);   //55:RGB565 66:RGB666 77:RGB888

  SPI_WriteComm(0xB4);
  SPI_WriteData(0x01);

  SPI_WriteComm(0xB6);    //RGB
  SPI_WriteData(0x20);
  SPI_WriteData(0x02);     //02
  SPI_WriteData(0x3B);

  SPI_WriteComm(0xB7);
  SPI_WriteData(0xC6);

  SPI_WriteComm(0xC0);
  SPI_WriteData(0x80);
  SPI_WriteData(0x45);

  SPI_WriteComm(0xC1);
  SPI_WriteData(0x0F );   //18  //00

  SPI_WriteComm(0xC2);
  SPI_WriteData(0xA7);   //A7

  SPI_WriteComm(0xC5);
  SPI_WriteData(0x0A);

  SPI_WriteComm(0xE8);
  SPI_WriteData(0x40);
  SPI_WriteData(0x8A);
  SPI_WriteData(0x00);
  SPI_WriteData(0x00);
  SPI_WriteData(0x29);
  SPI_WriteData(0x19);
  SPI_WriteData(0xA5);
  SPI_WriteData(0x33);

  SPI_WriteComm(0xE0);
  SPI_WriteData(0xD0);
  SPI_WriteData(0x08);
  SPI_WriteData(0x0F);
  SPI_WriteData(0x06);
  SPI_WriteData(0x06);
  SPI_WriteData(0x33);
  SPI_WriteData(0x30);
  SPI_WriteData(0x33);
  SPI_WriteData(0x47);
  SPI_WriteData(0x17);
  SPI_WriteData(0x13);
  SPI_WriteData(0x13);
  SPI_WriteData(0x2B);
  SPI_WriteData(0x31);

  SPI_WriteComm(0xE1);
  SPI_WriteData(0xD0);
  SPI_WriteData(0x0A);
  SPI_WriteData(0x11);
  SPI_WriteData(0x0B);
  SPI_WriteData(0x09);
  SPI_WriteData(0x07);
  SPI_WriteData(0x2F);
  SPI_WriteData(0x33);
  SPI_WriteData(0x47);
  SPI_WriteData(0x38);
  SPI_WriteData(0x15);
  SPI_WriteData(0x16);
  SPI_WriteData(0x2C);
  SPI_WriteData(0x32);


  SPI_WriteComm(0xF0);
  SPI_WriteData(0x3C);

  SPI_WriteComm(0xF0);
  SPI_WriteData(0x69);

  delay_ms(120);

  SPI_WriteComm(0x21);
  LCD_CS_HIGH();
  delay_ms(1);
  LCD_ST7365_On();
}

void LCD_ST7365_Off(void) {
  LCD_CS_LOW();
  delay_ms(1);
  lcdWriteCommand(0x28);
  LCD_CS_HIGH();
}

unsigned int LCD_ST7365_ReadID(void) {
//  lcdReset();
  unsigned int ID = 0;
  LCD_CS_LOW();
  lcdDelay();

  lcdWriteCommand( 0x0a );
//  lcdWriteData( 0x00);

  LCD_MOSI_AS_INPUT();
  LCD_SCK_LOW();
  lcdDelay();
  lcdDelay();
  LCD_SCK_HIGH();
  lcdDelay();
  lcdDelay();
  /*
  LCD_ReadByte();
  ID += (uint32_t)(LCD_ReadByte())<<8;
  */
  ID = LCD_ReadByte();
  ID |= ((uint32_t)LCD_ReadByte())<<8;
  ID |= ((uint32_t)LCD_ReadByte())<<16;
  ID |= ((uint32_t)LCD_ReadByte())<<24;

  LCD_CS_HIGH();
  lcdDelay();
  lcdDelay();


   return (ID);
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
  /* Configure the LCD SPI+RESET pins */
  lcdSpiConfig();

  /* Reset the LCD --------------------------------------------------------*/
  lcdReset();

  /* Configure the LCD Control pins */
  LCD_AF_GPIOConfig();
  LCD_ST7365_ReadID();
  /* Send LCD initialization commands */
  TRACE("LCD INIT (default): ST7365");
  boardLcdType = "ST7365 (Default)";
  lcdInitFunction = LCD_ST7365_Init;
  lcdOffFunction = LCD_ST7365_Off;
  lcdOnFunction = LCD_ST7365_On;

  __HAL_RCC_LTDC_CLK_ENABLE();
  lcdInitFunction();

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

