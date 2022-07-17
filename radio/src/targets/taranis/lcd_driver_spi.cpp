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

#include "board.h"
#include "debug.h"
#include "lcd.h"

#if !defined(BOOT)
  #include "opentx.h"
#endif

#if defined(RADIO_FAMILY_JUMPER_T12) || defined(RADIO_TX12) || defined(RADIO_TX12MK2) || defined(RADIO_ZORRO) || defined(RADIO_T8) || defined(RADIO_TPRO)
  #define LCD_CONTRAST_OFFSET            -10
#else
  #define LCD_CONTRAST_OFFSET            160
#endif
#define RESET_WAIT_DELAY_MS            300 // Wait time after LCD reset before first command
#define WAIT_FOR_DMA_END()             do { } while (lcd_busy)

#define LCD_NCS_HIGH()                 LCD_NCS_GPIO->BSRRL = LCD_NCS_GPIO_PIN
#define LCD_NCS_LOW()                  LCD_NCS_GPIO->BSRRH = LCD_NCS_GPIO_PIN

#define LCD_A0_HIGH()                  LCD_SPI_GPIO->BSRRL = LCD_A0_GPIO_PIN
#define LCD_A0_LOW()                   LCD_SPI_GPIO->BSRRH = LCD_A0_GPIO_PIN

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

  // APB1 clock / 2 = 133nS per clock
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
  GPIO_Init(LCD_SPI_GPIO, &GPIO_InitStructure);

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

#if LCD_W == 128
void lcdStart()
{
#if defined(LCD_VERTICAL_INVERT)
  // T12 and TX12 have the screen inverted.
  lcdWriteCommand(0xe2); // (14) Soft reset
  lcdWriteCommand(0xa0); // Set seg
  lcdWriteCommand(0xc8); // Set com
  lcdWriteCommand(0xf8); // Set booster
  lcdWriteCommand(0x00); // 5x
  lcdWriteCommand(0xa3); // Set bias=1/6
  lcdWriteCommand(0x22); // Set internal rb/ra=5.0
  lcdWriteCommand(0x2f); // All built-in power circuits on
  lcdWriteCommand(0x24); // Power control set
  lcdWriteCommand(0x81); // Set contrast
  lcdWriteCommand(0x0A); // Set Vop
  lcdWriteCommand(0xa6); // Set display mode
#else
  lcdWriteCommand(0xe2); // (14) Soft reset
  lcdWriteCommand(0xa1); // Set seg
  lcdWriteCommand(0xc0); // Set com
  lcdWriteCommand(0xf8); // Set booster
  lcdWriteCommand(0x00); // 5x
  lcdWriteCommand(0xa3); // Set bias=1/6
  lcdWriteCommand(0x22); // Set internal rb/ra=5.0
  lcdWriteCommand(0x2f); // All built-in power circuits on
  lcdWriteCommand(0x81); // Set contrast
  lcdWriteCommand(0x36); // Set Vop
  lcdWriteCommand(0xa6); // Set display mode
#endif
#if defined(BOOT)
  lcdSetRefVolt(LCD_CONTRAST_DEFAULT);
#else
  lcdSetRefVolt(g_eeGeneral.contrast);
#endif
}
#else
void lcdStart()
{
  lcdWriteCommand(0x2F); // Internal pump control
  delay_ms(20);
  lcdWriteCommand(0x24); // Temperature compensation
  lcdWriteCommand(0xE9); // Set bias=1/10
  lcdWriteCommand(0x81); // Set Vop
#if defined(BOOT)
  lcdWriteCommand(LCD_CONTRAST_OFFSET + LCD_CONTRAST_DEFAULT);
#else
  lcdWriteCommand(LCD_CONTRAST_OFFSET + g_eeGeneral.contrast);
#endif
  lcdWriteCommand(0xA2); // Set line rate: 28KLPS
  lcdWriteCommand(0x28); // Set panel loading
  lcdWriteCommand(0x40); // Scroll line LSB
  lcdWriteCommand(0x50); // Scroll line MSB
  lcdWriteCommand(0x89); // RAM address control
  lcdWriteCommand(0xC0); // LCD mapping control
  lcdWriteCommand(0x04); // MX=0, MY=1
  lcdWriteCommand(0xD0); // Display pattern = 16-SCALE GRAY
  lcdWriteCommand(0xF1); // Set COM end
  lcdWriteCommand(0x3F); // 64

  lcdWriteCommand(0xF8); // Set Window Program Disable.

  lcdWriteCommand(0xF5); // Start row address of RAM program window. PAGE1
  lcdWriteCommand(0x00);
  lcdWriteCommand(0xF7); // End row address of RAM program window. PAGE32
  lcdWriteCommand(0x1F);
  lcdWriteCommand(0xF4); // Start column address of RAM program window.
  lcdWriteCommand(0x00);
  lcdWriteCommand(0xF6); // End column address of RAM program window. SEG212
  lcdWriteCommand(0xD3);
}

void lcdWriteAddress(uint8_t x, uint8_t y)
{
  lcdWriteCommand(x & 0x0Fu); // Set Column Address LSB CA[3:0]
  lcdWriteCommand((x >> 4u) | 0x10u); // Set Column Address MSB CA[7:4]

  lcdWriteCommand((y & 0x0Fu) | 0x60u); // Set Row Address LSB RA [3:0]
  lcdWriteCommand(((y >> 4u) & 0x0Fu) | 0x70u); // Set Row Address MSB RA [7:4]
}
#endif

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

#if LCD_W == 128
  uint8_t * p = displayBuf;
  for (uint8_t y=0; y < 8; y++, p+=LCD_W) {
    lcdWriteCommand(0x10); // Column addr 0
    lcdWriteCommand(0xB0 | y); // Page addr y
#if !defined(LCD_VERTICAL_INVERT)
    lcdWriteCommand(0x04);
#endif

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
#else
  // Wait if previous DMA transfer still active
  WAIT_FOR_DMA_END();
  lcd_busy = true;

  lcdWriteAddress(0, 0);

  LCD_NCS_LOW();
  LCD_A0_HIGH();

  LCD_DMA_Stream->CR &= ~DMA_SxCR_EN; // Disable DMA
  LCD_DMA->HIFCR = LCD_DMA_FLAGS; // Write ones to clear bits

#if defined(LCD_DUAL_BUFFER)
  // Switch LCD buffer
  LCD_DMA_Stream->M0AR = (uint32_t)displayBuf;
  displayBuf = (displayBuf == displayBuf1) ? displayBuf2 : displayBuf1;
#endif

  LCD_DMA_Stream->CR |= DMA_SxCR_EN | DMA_SxCR_TCIE; // Enable DMA & TC interrupts
  LCD_SPI->CR2 |= SPI_CR2_TXDMAEN;
#endif
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
#if LCD_W == 128
  delay_ms(150);
#else
  delay_ms(1); // Only 3 us needed according to data-sheet, we use 1 ms
#endif
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

#if defined(RADIO_X9DP2019) || defined(RADIO_X7ACCESS)
  #define LCD_DELAY_NEEDED() true
#else
  #define LCD_DELAY_NEEDED() (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
#endif

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

  if (LCD_DELAY_NEEDED()) {
#if !defined(BOOT)
    while (g_tmr10ms < (RESET_WAIT_DELAY_MS / 10)); // wait measured from the power-on
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

#if LCD_W != 128
  WAIT_FOR_DMA_END();
#endif

  lcdWriteCommand(0x81); // Set Vop
  lcdWriteCommand(val+LCD_CONTRAST_OFFSET); // 0-255
}
