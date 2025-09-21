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

#include "hal/gpio.h"
#include "stm32_gpio.h"
#include "stm32_spi.h"
#include "stm32_dma.h"

#include "board.h"
#include "debug.h"
#include "lcd.h"

#include "hal/abnormal_reboot.h"
#include "timers_driver.h"

#if !defined(BOOT)
  #include "edgetx.h"
#endif

#if defined(OLED_SCREEN)
  #define LCD_CONTRAST_OFFSET            0
#elif defined(RADIO_FAMILY_JUMPER_T12) || defined(MANUFACTURER_RADIOMASTER) || defined(RADIO_COMMANDO8) || defined(RADIO_TPRO) || defined(RADIO_T12MAX) || defined(RADIO_V12) || defined(RADIO_V14)
  #define LCD_CONTRAST_OFFSET            -10
#else
  #define LCD_CONTRAST_OFFSET            160
#endif
#define RESET_WAIT_DELAY_MS            300 // Wait time after LCD reset before first command
#define WAIT_FOR_DMA_END()             do { } while (lcd_busy)

#define LCD_NCS_HIGH()  gpio_set(LCD_NCS_GPIO)
#define LCD_NCS_LOW()   gpio_clear(LCD_NCS_GPIO)

#define LCD_A0_HIGH()   gpio_set(LCD_A0_GPIO)
#define LCD_A0_LOW()    gpio_clear(LCD_A0_GPIO)

#define LCD_RST_HIGH()  gpio_set(LCD_RST_GPIO)
#define LCD_RST_LOW()   gpio_clear(LCD_RST_GPIO)

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
  stm32_spi_enable_clock(LCD_SPI);
  gpio_init_af(LCD_MOSI_GPIO, LCD_GPIO_AF, GPIO_PIN_SPEED_HIGH);
  gpio_init_af(LCD_CLK_GPIO, LCD_GPIO_AF, GPIO_PIN_SPEED_HIGH);

  // APB1 clock / 2 = 133nS per clock
  LCD_SPI->CR1 = 0; // Clear any mode error
  LCD_SPI->CR1 = SPI_CR1_SSM | SPI_CR1_SSI | SPI_CR1_CPOL | SPI_CR1_CPHA | LCD_SPI_PRESCALER;
  LCD_SPI->CR2 = 0;
  LCD_SPI->CR1 |= SPI_CR1_MSTR;	// Make sure in case SSM/SSI needed to be set first
  LCD_SPI->CR1 |= SPI_CR1_SPE;

  gpio_init(LCD_NCS_GPIO, GPIO_OUT, GPIO_PIN_SPEED_MEDIUM);
  LCD_NCS_HIGH();

  gpio_init(LCD_RST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_MEDIUM);
  gpio_init(LCD_A0_GPIO, GPIO_OUT, GPIO_PIN_SPEED_HIGH);

  stm32_dma_enable_clock(LCD_DMA);
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

  NVIC_SetPriority(LCD_DMA_Stream_IRQn, 7);
  NVIC_EnableIRQ(LCD_DMA_Stream_IRQn);

#if defined(OLED_VCC_CS)
  gpio_init(OLED_VCC_CS, GPIO_OUT, GPIO_PIN_SPEED_LOW);

  // Coming from bootloader or EM, screen is already on
  if (WAS_RESET_BY_WATCHDOG_OR_SOFTWARE())
    gpio_set(OLED_VCC_CS);
#endif
}

#if defined(SSD1309_LCD)
void lcdColumnSet(unsigned char column)
{
  lcdWriteCommand(0x10|(column >> 4));
  lcdWriteCommand(0x00|(column & 0x0f));
}

void lcdPageSet(unsigned char page)
{
  lcdWriteCommand(0xb0 + page);
}
#endif

#if LCD_W == 128
void lcdStart()
{
#if defined(SSD1309_LCD)
   lcdWriteCommand(0xD5);  // Set Display Clock Divide Ratio / OSC Frequency
   lcdWriteCommand(0x80);  // Display Clock Divide Ratio / OSC Frequency
   lcdWriteCommand(0xA8);  // Set Multiplex Ratio
   lcdWriteCommand(0x3F);  // Multiplex Ratio for 128x64 (LCD_HEIGHT - 1)
   lcdWriteCommand(0xD3);  // Set Display Offset
   lcdWriteCommand(0x00);  // Display Offset (0)
   lcdWriteCommand(0x40);  // Set Display Start Line (0)
   lcdWriteCommand(0x8D);  // Set Charge Pump
   lcdWriteCommand(0x10);  // Charge Pump (0x10 External, 0x14 Internal DC/DC)
   lcdWriteCommand(0xA1);  // Set Segment Re-Map (Reversed)
   lcdWriteCommand(0xC8);  // Set Com Output Scan Direction (Reversed)
   lcdWriteCommand(0xDA);  // Set COM Hardware Configuration
   lcdWriteCommand(0x12);  // COM Hardware Configuration
   lcdWriteCommand(0xD9);  // Set Pre-Charge Period
   lcdWriteCommand(0x4F);  // Set Pre-Charge Period (A[7:4]:Phase 2, A[3:0]:Phase 1)
   lcdWriteCommand(0xDB);  // Set VCOMH Deselect Level
   lcdWriteCommand(0x20);  // VCOMH Deselect Level (0x00 ~ 0.65 x VCC, 0x10 ~ 0.71 x VCC, 0x20 ~ 0.77 x VCC, 0x30 ~ 0.83 x VCC)
   lcdWriteCommand(0xA4);  // Disable Entire Display On
   lcdWriteCommand(0xA6);  // Set Normal Display (not inverted)
   lcdWriteCommand(0x2E);  // Deactivate scroll
#if defined(OLED_VCC_CS)
   delay_ms(100);
   gpio_set(OLED_VCC_CS);
#endif
#else
#if defined(LCD_VERTICAL_INVERT)
  // T12 and TX12 have the screen inverted.
  #if defined(RADIO_V12)
    lcdWriteCommand(0xe2); // (14) Soft reset
    lcdWriteCommand(0xa0);  // Set seg
    lcdWriteCommand(0xc8);  // Set com
    lcdWriteCommand(0xf8);  // Set booster
    lcdWriteCommand(0x00);  // 5x
    lcdWriteCommand(0xa2); // Set bias=1/6
    lcdWriteCommand(0x26);  // Set internal rb/ra=5.0
    lcdWriteCommand(0x2f);  // All built-in power circuits on
    lcdWriteCommand(0x81);  // Set contrast
    lcdWriteCommand(0x1F);  // Set Vop
    lcdWriteCommand(0xa6);  // Set display mode
  #else  
    lcdWriteCommand(0xe2); // (14) Soft reset
#if defined(LCD_HORIZONTAL_INVERT)
    lcdWriteCommand(0xa1); // Set seg
#else 
    lcdWriteCommand(0xa0); // Set seg
#endif
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
  #endif
#else
  #if defined(RADIO_V14)
    lcdWriteCommand(0xe2); // (14) Soft reset
    lcdWriteCommand(0xa1); // Set seg
    lcdWriteCommand(0xc0);  // Set com
    lcdWriteCommand(0xf8);  // Set booster
    lcdWriteCommand(0x00);  // 5x
    lcdWriteCommand(0xa2); // Set bias=1/6
    lcdWriteCommand(0x26);  // Set internal rb/ra=5.0
    lcdWriteCommand(0x2f);  // All built-in power circuits on
    lcdWriteCommand(0x81);  // Set contrast
    lcdWriteCommand(0x1F);  // Set Vop
    lcdWriteCommand(0xa6);  // Set display mode
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
#endif
#if defined(BOOT)
  lcdSetRefVolt(LCD_CONTRAST_DEFAULT);
#else
  lcdSetRefVolt(g_eeGeneral.contrast);
#endif
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

void lcdRefreshWait()
{
  WAIT_FOR_DMA_END();
}

void lcdRefresh(bool wait)
{
  if (!lcdInitFinished) {
    lcdInitFinish();
  }

#if LCD_W == 128
  uint8_t * p = displayBuf;
#if defined(LCD_W_OFFSET)
  lcdWriteCommand(LCD_W_OFFSET);
#endif
  for (uint8_t y=0; y < 8; y++, p+=LCD_W) {
#if defined(SSD1309_LCD)
    lcdPageSet(y);
    lcdColumnSet(0);
#else
    lcdWriteCommand(0x10); // Column addr 0
    lcdWriteCommand(0xB0 | y); // Page addr y
#if !defined(LCD_VERTICAL_INVERT)
    lcdWriteCommand(0x04);
#endif
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
#if defined(OLED_VCC_CS)
  gpio_clear(OLED_VCC_CS);
  delay_ms(100);
#else
  delay_ms(3); // Wait for caps to drain
#endif
}

void lcdReset()
{
  LCD_NCS_HIGH();
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
    uint32_t end = timersGetMsTick() + RESET_WAIT_DELAY_MS;
    while (timersGetMsTick() < end);
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

#if defined(RADIO_V12) || defined(RADIO_V14)
  lcdWriteCommand(0x81);                      // Set Vop
  lcdWriteCommand(val+LCD_CONTRAST_OFFSET+20);// 0-255
#else
  lcdWriteCommand(0x81); // Set Vop
  lcdWriteCommand(val+LCD_CONTRAST_OFFSET); // 0-255
#endif
}

#if LCD_W == 128
void lcdSetInvert(bool invert)
{
   lcdWriteCommand(invert ? 0xA7 : 0xA6);
}
#endif
