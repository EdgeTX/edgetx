/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "opentx.h"
#if defined(RADIO_TANGO)
#define WAIT_FOR_DMA_END()             do { } while (lcd_busy)

#define LCD_CS_HIGH()                 LCD_NCS_GPIO->BSRRL = LCD_NCS_GPIO_PIN
#define LCD_CS_LOW()                  LCD_NCS_GPIO->BSRRH = LCD_NCS_GPIO_PIN

#define LCD_RST_HIGH()                LCD_RST_GPIO->BSRRL = LCD_RST_GPIO_PIN
#define LCD_RST_LOW()                 LCD_RST_GPIO->BSRRH = LCD_RST_GPIO_PIN

#define LCD_DC_HIGH()                 LCD_DC_GPIO->BSRRL = LCD_DC_GPIO_PIN
#define LCD_DC_LOW()                  LCD_DC_GPIO->BSRRH = LCD_DC_GPIO_PIN

#define SPI_TIMEOUT                   10000000UL

volatile bool lcd_busy = false;
bool lcd_on = false;

static void spiWrite(uint8_t byte)
{
  LCD_DC_LOW();
  LCD_CS_LOW();

  uint32_t timeout = 0;

  while ((LCD_SPI->SR & SPI_SR_TXE) == 0 && timeout++ < SPI_TIMEOUT) {
    // Wait
  }

  if(timeout >= SPI_TIMEOUT ){
    LCD_CS_HIGH();
    return;
  }

  timeout = 0;

  (void)LCD_SPI->DR; // Clear receive
  LCD_SPI->DR = byte;

  while ((LCD_SPI->SR & SPI_SR_RXNE) == 0 && timeout++ < SPI_TIMEOUT) {
    // Wait
  }

  LCD_CS_HIGH();
}


void spiWriteCommand(uint8_t command)
{
  spiWrite(command);
}

void spiWriteArg(uint8_t arg)
{
  spiWrite(arg);
}

void spiWriteCommandWithArg(uint8_t command, uint8_t arg)
{
  spiWriteCommand(command);
  spiWriteArg(arg);
}

void lcdHardwareInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // APB1 clock / 4 = 10.5MHz
  LCD_SPI->CR1 = 0; // Clear any mode error
  LCD_SPI->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL | SPI_CR1_CPHA | SPI_CR1_BR_0;
  LCD_SPI->CR2 = 0;
  LCD_SPI->CR1 |= SPI_CR1_MSTR;  // Make sure in case SSM/SSI needed to be set first
  LCD_SPI->CR1 |= SPI_CR1_SPE;

  LCD_CS_HIGH();

  GPIO_InitStructure.GPIO_Pin = LCD_NCS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_Init(LCD_NCS_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_RST_GPIO_PIN;
  GPIO_Init(LCD_RST_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_DC_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LCD_DC_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_CLK_GPIO_PIN | LCD_MOSI_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(LCD_SPI_GPIO, LCD_MOSI_GPIO_PinSource, LCD_GPIO_AF);
  GPIO_PinAFConfig(LCD_SPI_GPIO, LCD_CLK_GPIO_PinSource, LCD_GPIO_AF);

  LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  LCD_DMA->HIFCR = LCD_DMA_FLAGS; // Write ones to clear bits
  LCD_DMA_Stream->CR =  DMA_SxCR_PL_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0;
  LCD_DMA_Stream->PAR = (uint32_t)&LCD_SPI->DR;
#if LCD_W == 128
  LCD_DMA_Stream->NDTR = LCD_W;
#else
  LCD_DMA_Stream->M0AR = (uint32_t)displayBuf;
  LCD_DMA_Stream->NDTR = LCD_W*LCD_H/8*4;
#endif
  LCD_DMA_Stream->FCR = 0x05; // DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;

  NVIC_EnableIRQ(LCD_DMA_Stream_IRQn);
}


extern "C" void LCD_DMA_Stream_IRQHandler()
{
  DEBUG_INTERRUPT(INT_LCD);

  LCD_DMA_Stream->CR &= ~DMA_SxCR_TCIE; // Stop interrupt
  LCD_DMA->HIFCR |= LCD_DMA_FLAG_INT; // Clear interrupt flag
  LCD_SPI->CR2 &= ~SPI_CR2_TXDMAEN;
  LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA

  while (LCD_SPI->SR & SPI_SR_BSY) {
    /* Wait for SPI to finish sending data
    The DMA TX End interrupt comes two bytes before the end of SPI transmission,
    therefore we have to wait here.
    */
  }

  LCD_DC_LOW(); // command write
  spiWrite(0xAF);
  LCD_CS_HIGH();

  lcd_busy = false;
}

/*
  Get the display On/Off status
*/
bool isLcdOn()
{
  return lcd_on;
}

/*
  Turn on the display
*/
void lcdOn()
{
  if(!lcd_on)
  {
    LCD_CS_LOW();
    LCD_DC_LOW();
    spiWriteCommandWithArg(0x0fd, 0x012);
    spiWriteCommand(0xAF); // LCD on
    LCD_CS_HIGH();
    lcd_on = true;
  }
}

/*
  Proper method for turning of LCD module. It must be used,
  otherwise we might damage LCD crystals in the long run!
*/
void lcdOff()
{
  if(lcd_on)
  {    
    WAIT_FOR_DMA_END();

    /*
    LCD Sleep mode is also good for draining capacitors and enables us
    to re-init LCD without any delay
    */
    LCD_CS_LOW();
    LCD_DC_LOW();
    spiWriteCommandWithArg(0x0fd, 0x012);
    spiWriteCommand(0xAE); // LCD sleep
    delay_ms(3); // Wait for caps to drain
    LCD_CS_HIGH();
    lcd_on = false;
  }
}

/*
  Starts LCD initialization routine. It should be called as
  soon as possible after the reset because LCD takes a lot of
  time to properly power-on.
  Make sure that delay_ms() is functional before calling this function!
*/

void lcdSetRefVolt(uint8_t val)
{
}

/*
  Adjust the contrast of the screen.
*/
void lcdAdjustContrast(uint8_t val)
{
  LCD_CS_LOW();
  LCD_DC_LOW();
  spiWriteCommandWithArg(0x081, val);
  LCD_CS_HIGH();
}

void lcdDisplayInit(void)
{
  LCD_CS_LOW();
  LCD_DC_LOW();

  // command write
  spiWriteCommandWithArg(0x0fd, 0x012);       /* unlock display, usually not required because the display is unlocked after reset */
  spiWriteCommand(0x0ae);                     /* display off */
  spiWriteCommandWithArg(0x0a8, 0x07f);       /* multiplex ratio: 0x05f * 1/128duty */
  spiWriteCommandWithArg(0x0a1, 0x000);       /* display start line */
  spiWriteCommandWithArg(0x0a2, 0x000);       /* display offset, shift mapping ram counter */
  spiWriteCommandWithArg(0x0a0, 0x51);        /* remap configuration */
  spiWriteCommandWithArg(0x0ab, 0x001);       /* Enable internal VDD regulator (RESET) */
  spiWriteCommandWithArg(0x081, 0x80);        /* contrast, brightness, 0..255 */
  spiWriteCommandWithArg(0x0b1, 0x051);       /* phase length */
  spiWriteCommandWithArg(0x0b3, 0x001);       /* set display clock divide ratio/oscillator frequency  */
  spiWriteCommand(0x0b9);                     /* use linear lookup table */
  spiWriteCommandWithArg(0x0bc, 0x008);       /* pre-charge voltage level */
  spiWriteCommandWithArg(0x0be, 0x007);       /* VCOMH voltage */
  spiWriteCommandWithArg(0x0b6, 0x001);       /* second precharge */
  spiWriteCommandWithArg(0x0d5, 0x062);       /* enable second precharge, internal vsl (bit0 = 0) */

  spiWriteCommand(0x0a4);                     /* normal display mode */

  LCD_CS_HIGH();
  lcd_on = true;
}

void lcdReset()
{
  TRACE("LCD reset\r\n");
  LCD_RST_HIGH();
  delay_ms(50);
  LCD_RST_LOW(); // module reset
  delay_ms(50);
  LCD_RST_HIGH();
  delay_ms(50);
}

void lcdInit(void)
{
  lcdHardwareInit();

  if (IS_LCD_RESET_NEEDED()) {
    lcdReset();
    lcdDisplayInit(); // panel configure
  }
}

void lcdRefreshWait()
{
  if(lcd_on)
    WAIT_FOR_DMA_END();
}

void lcdRefresh(bool wait)
{
  uint8_t * p = displayBuf;
  
  if(!lcd_on)
    return;

  LCD_CS_LOW();
  LCD_DC_LOW(); // command write

  spiWriteCommand(0x75);
  spiWriteArg(0);
  spiWriteArg(LCD_H - 1);

  spiWriteCommand(0x15);  // set column address
  spiWriteArg(0);
  spiWriteArg((LCD_W / 2) - 1);

  LCD_DC_HIGH(); // data write


  LCD_CS_LOW();;

  lcd_busy = true;
  LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  LCD_DMA->HIFCR = LCD_DMA_FLAGS; // Write ones to clear bits
  LCD_DMA_Stream->M0AR = (uint32_t)p;
  LCD_DMA_Stream->NDTR = 16*32*12;
  LCD_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA & TC interrupts
  LCD_SPI->CR2 |= SPI_CR2_TXDMAEN;

  WAIT_FOR_DMA_END();
}
#elif defined(RADIO_MAMBO)

#define LCD_CONTRAST_OFFSET            127
#define RESET_WAIT_DELAY_MS            300 // Wait time after LCD reset before first command
#define WAIT_FOR_DMA_END()             do { } while (lcd_busy)

#define LCD_NCS_HIGH()                 LCD_NCS_GPIO->BSRRL = LCD_NCS_GPIO_PIN
#define LCD_NCS_LOW()                  LCD_NCS_GPIO->BSRRH = LCD_NCS_GPIO_PIN

#define LCD_A0_HIGH()                  LCD_A0_GPIO->BSRRL = LCD_A0_GPIO_PIN
#define LCD_A0_LOW()                   LCD_A0_GPIO->BSRRH = LCD_A0_GPIO_PIN

#define LCD_RST_HIGH()                 LCD_RST_GPIO->BSRRL = LCD_RST_GPIO_PIN
#define LCD_RST_LOW()                  LCD_RST_GPIO->BSRRH = LCD_RST_GPIO_PIN

bool lcdInitFinished = false;
void lcdInitFinish();

void lcdWriteCommand(uint8_t byte)
{
  LCD_A0_LOW();
  LCD_NCS_LOW();
  while ((SPI3->SR & SPI_SR_TXE) == 0) {
    // Wait
  }
  (void)SPI3->DR; // Clear receive
  LCD_SPI->DR = byte;
  while ((SPI3->SR & SPI_SR_RXNE) == 0) {
    // Wait
  }
  LCD_NCS_HIGH();
}

void lcdHardwareInit()
{
  GPIO_InitTypeDef GPIO_InitStructure;

  // APB1 clock / 2 = 21MHz
  LCD_SPI->CR1 = 0; // Clear any mode error
  LCD_SPI->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL | SPI_CR1_CPHA;
  LCD_SPI->CR2 = 0;
  LCD_SPI->CR1 |= SPI_CR1_MSTR;	// Make sure in case SSM/SSI needed to be set first
  LCD_SPI->CR1 |= SPI_CR1_SPE;

  LCD_NCS_HIGH();

  GPIO_InitStructure.GPIO_Pin = LCD_NCS_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(LCD_NCS_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_RST_GPIO_PIN;
  GPIO_Init(LCD_RST_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_A0_GPIO_PIN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(LCD_A0_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = LCD_CLK_GPIO_PIN | LCD_MOSI_GPIO_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

  GPIO_PinAFConfig(LCD_SPI_GPIO, LCD_MOSI_GPIO_PinSource, LCD_GPIO_AF);
  GPIO_PinAFConfig(LCD_SPI_GPIO, LCD_CLK_GPIO_PinSource, LCD_GPIO_AF);

  LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  LCD_DMA->HIFCR = LCD_DMA_FLAGS; // Write ones to clear bits
  LCD_DMA_Stream->CR =  DMA_SxCR_PL_0 | DMA_SxCR_MINC | DMA_SxCR_DIR_0;
  LCD_DMA_Stream->PAR = (uint32_t)&LCD_SPI->DR;
  LCD_DMA_Stream->NDTR = LCD_W;
  LCD_DMA_Stream->FCR = 0x05; // DMA_SxFCR_DMDIS | DMA_SxFCR_FTH_0;

  NVIC_EnableIRQ(LCD_DMA_Stream_IRQn);
}

void lcdStart()
{
  lcdWriteCommand(0xAE);  // Display OFF
  lcdWriteCommand(0xA2);  // 1/64 Duty 1/9 Bias
  lcdWriteCommand(0xA0);  // ADC select S0->S131(S1-S128)
  lcdWriteCommand(0xC8);  // com1 --> com64
  lcdWriteCommand(0x24);  // Rb/Ra
  lcdWriteCommand(0x81);  // Sets V0
  lcdWriteCommand(0x1F);  // EV of V0, range：0x00-0x3f
  lcdWriteCommand(0x24);  // Ratio of V0, range：0x20-0x27
  lcdWriteCommand(0x2F);  // voltage follower ON  regulator ON  booster ON
  lcdWriteCommand(0xA6);  // Normal Display (not reverse display)
  lcdWriteCommand(0xA4);  // Entire Display Disable
  lcdWriteCommand(0x40);  // Set Display Start Line = com0
  lcdWriteCommand(0xB0);  // Set Page Address = 0
  lcdWriteCommand(0x10);  // Set Column Address 4 higher bits = 0
  lcdWriteCommand(0x01);  // Set Column Address 4 lower bits = 1 , from IC SEG1 -> SEG128
  lcdWriteCommand(0xAF);  // Display ON
}

volatile bool lcd_busy;

#if !defined(LCD_DUAL_BUFFER)
void lcdRefreshWait()
{
  WAIT_FOR_DMA_END();
}
#endif

void lcdRefresh(bool wait)
{
  if (!lcdInitFinished) {
    lcdInitFinish();
  }

  uint8_t * p = displayBuf;
  for (uint8_t y=0; y < 8; y++, p+=LCD_W) {
    lcdWriteCommand(0x10); // Column addr 0
    lcdWriteCommand(0x00);
    lcdWriteCommand(0xB0 | y); // Page addr y

    LCD_NCS_LOW();
    LCD_A0_HIGH();

    lcd_busy = true;
    LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
    LCD_DMA->HIFCR = LCD_DMA_FLAGS; // Write ones to clear bits
    LCD_DMA_Stream->M0AR = (uint32_t)p;
    LCD_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA & TC interrupts
    LCD_SPI->CR2 |= SPI_CR2_TXDMAEN;

    WAIT_FOR_DMA_END();

    LCD_NCS_HIGH();
    LCD_A0_HIGH();
  }
}

extern "C" void LCD_DMA_Stream_IRQHandler()
{
  DEBUG_INTERRUPT(INT_LCD);

  LCD_DMA_Stream->CR &= ~DMA_SxCR_TCIE; // Stop interrupt
  LCD_DMA->HIFCR |= LCD_DMA_FLAG_INT; // Clear interrupt flag
  LCD_SPI->CR2 &= ~SPI_CR2_TXDMAEN;
  LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA

  while (LCD_SPI->SR & SPI_SR_BSY) {
    /* Wait for SPI to finish sending data
    The DMA TX End interrupt comes two bytes before the end of SPI transmission,
    therefore we have to wait here.
    */
  }
  LCD_NCS_HIGH();
  lcd_busy = false;
}

/*
  Proper method for turning of LCD module. It must be used,
  otherwise we might damage LCD crystals in the long run!
*/
void lcdOff()
{
  WAIT_FOR_DMA_END();

  /*
  LCD Sleep mode is also good for draining capacitors and enables us
  to re-init LCD without any delay
  */
  lcdWriteCommand(0xAE); // LCD sleep
  delay_ms(3); // Wait for caps to drain
}

void lcdReset()
{
  LCD_RST_LOW();
  delay_ms(150);
  LCD_RST_HIGH();
}

/*
  Starts LCD initialization routine. It should be called as
  soon as possible after the reset because LCD takes a lot of
  time to properly power-on.

  Make sure that delay_ms() is functional before calling this function!
*/
void lcdInit()
{
  lcdHardwareInit();

  if (IS_LCD_RESET_NEEDED()) {
    lcdReset();
  }
}

/*
  Finishes LCD initialization. It is called auto-magically when first LCD command is
  issued by the other parts of the code.
*/
void lcdInitFinish()
{
  lcdInitFinished = true;

  /*
    LCD needs longer time to initialize in low temperatures. The data-sheet
    mentions a time of at least 150 ms. The delay of 1300 ms was obtained
    experimentally. It was tested down to -10 deg Celsius.

    The longer initialization time seems to only be needed for regular Taranis,
    the Taranis Plus (9XE) has been tested to work without any problems at -18 deg Celsius.
    Therefore the delay for T+ is lower.

    If radio is reset by watchdog or boot-loader the wait is skipped, but the LCD
    is initialized in any case.

    This initialization is needed in case the user moved power switch to OFF and
    then immediately to ON position, because lcdOff() was called. In any case the LCD
    initialization (without reset) is also recommended by the data sheet.
  */

  if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
#if !defined(BOOT)
    while (g_tmr10ms < (RESET_WAIT_DELAY_MS/10)); // wait measured from the power-on
#else
    delay_ms(RESET_WAIT_DELAY_MS);
#endif
  }

  lcdStart();
  lcdWriteCommand(0xAF); // dc2=1, IC into exit SLEEP MODE, dc3=1 gray=ON, dc4=1 Green Enhanc mode disabled
  delay_ms(20); // needed for internal DC-DC converter startup
}

void lcdSetRefVolt(uint8_t val)
{
  if (!lcdInitFinished) {
    lcdInitFinish();
  }
  uint16_t setVal = val + LCD_CONTRAST_OFFSET;
  lcdWriteCommand(0x81); // Set Vop
  lcdWriteCommand(setVal & 0x3F); // the lower 6 bits for EV
  lcdWriteCommand(0x20 | (setVal >> 6)); // and the higher 3 bits for ratio
}

#endif

