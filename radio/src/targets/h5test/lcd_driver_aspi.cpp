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

#include "board.h"
#include "lcd.h"

#include "hal/abnormal_reboot.h"
#include "timers_driver.h"

#if !defined(BOOT)
#include "myeeprom.h"
#include "debug.h"
#endif

#define CONTRAST_OFS                   5
#define RESET_WAIT_DELAY_MS            1300 // Wait time after LCD reset before first command

bool lcdInitFinished = false;
void lcdInitFinish();

#define LCD_NCS_HIGH()  gpio_set(LCD_NCS_GPIO)
#define LCD_NCS_LOW()   gpio_clear(LCD_NCS_GPIO)

#define LCD_A0_HIGH()   gpio_set(LCD_A0_GPIO)
#define LCD_A0_LOW()    gpio_clear(LCD_A0_GPIO)

#define LCD_RST_HIGH()  gpio_set(LCD_RST_GPIO)
#define LCD_RST_LOW()   gpio_clear(LCD_RST_GPIO)

#define LCD_CLK_HIGH()  gpio_set(LCD_CLK_GPIO)
#define LCD_CLK_LOW()   gpio_clear(LCD_CLK_GPIO)

#define LCD_MOSI_HIGH() gpio_set(LCD_MOSI_GPIO)
#define LCD_MOSI_LOW()  gpio_clear(LCD_MOSI_GPIO)

void lcdWriteCommand(uint8_t command)
{
  int i = 8;
  LCD_A0_LOW();

  LCD_CLK_HIGH();
  LCD_CLK_HIGH();
  LCD_NCS_LOW();

  while (i--) {
    LCD_CLK_LOW();

    if (command & 0x80)
      LCD_MOSI_HIGH();
    else
      LCD_MOSI_LOW();

    command <<= 1;

    LCD_CLK_LOW();
    LCD_CLK_LOW();

    LCD_CLK_HIGH();
    LCD_CLK_HIGH();
  }

  LCD_NCS_HIGH();
  LCD_A0_HIGH();
}

void lcdWriteData(uint8_t data)
{
  int i = 8;

  LCD_CLK_HIGH();
  LCD_CLK_HIGH();
  LCD_A0_HIGH();
  LCD_NCS_LOW();

  while (i--) {
    LCD_CLK_LOW();
    if (data & 0x80)
      LCD_MOSI_HIGH();
    else
      LCD_MOSI_LOW();

    data <<= 1;

    LCD_CLK_LOW();
    LCD_CLK_LOW();

    LCD_CLK_HIGH();
    LCD_CLK_HIGH();
  }

  LCD_NCS_HIGH();
  LCD_A0_HIGH();
}

void lcdStart()
{	
  lcdWriteCommand(0x2B); // Panel loading set, Internal VLCD.
  delay_ms(20);
  lcdWriteCommand(0x25); // Temperature compensation curve definition: 0x25 = -0.05%/oC
  lcdWriteCommand(0xEA); // Set bias=1/10: Command table NO.27
  lcdWriteCommand(0x81); // Set Vop
#if defined(BOOT)
  lcdWriteCommand(CONTRAST_OFS+25);
#else
  lcdWriteCommand(CONTRAST_OFS+g_eeGeneral.contrast);
#endif
  lcdWriteCommand(0xA6); // Inverse display off
  lcdWriteCommand(0xD1); // Set RGB: Command table NO.21, D1=RGB
  lcdWriteCommand(0xD5); // Set color mode 4K and 12bits: Command table NO.22
  lcdWriteCommand(0xA0); // Line rates, 25.2 Klps
  lcdWriteCommand(0xC8); // Set N-LINE inversion
  lcdWriteCommand(0x1D); // Disable NIV
  lcdWriteCommand(0xF1); // Set CEN
  lcdWriteCommand(0x3F); // 1/64 DUTY
  lcdWriteCommand(0x84); // Disable Partial Display
  lcdWriteCommand(0xC4); // MY=1, MX=0
  lcdWriteCommand(0x89); // WA=1, column (CA) increment (+1) first until CA reaches CA boundary, then RA will increment by (+1)

  lcdWriteCommand(0xF8); // Set Window Program Enable, inside modle
  lcdWriteCommand(0xF4); // Start column address of RAM program window.
  lcdWriteCommand(0x00);
  lcdWriteCommand(0xF5); // Start row address of RAM program window.
  lcdWriteCommand(0x60);
  lcdWriteCommand(0xF6); // End column address of RAM program window.
  lcdWriteCommand(0x47);
  lcdWriteCommand(0xF7); // End row address of RAM program window.
  lcdWriteCommand(0x9F);
}

void lcdWriteAddress(uint8_t x, uint8_t y)
{
  lcdWriteCommand(x&0x0F); // Set Column Address LSB CA[3:0]
  lcdWriteCommand((x>>4) | 0x10); // Set Column Address MSB CA[7:4]

  lcdWriteCommand((y&0x0F) | 0x60); // Set Row Address LSB RA [3:0]
  lcdWriteCommand(((y>>4) & 0x0F) | 0x70); // Set Row Address MSB RA [7:4]
}

#define LCD_WRITE_BIT(bit) \
  if (bit) \
    LCD_MOSI_HIGH(); \
  else \
    LCD_MOSI_LOW(); \
  LCD_CLK_LOW(); \
  LCD_CLK_LOW(); \
  LCD_CLK_LOW(); \
  LCD_CLK_HIGH(); \
  LCD_CLK_HIGH();

void lcdRefresh()
{  
  if (!lcdInitFinished) {
    lcdInitFinish();
  }

  for (uint8_t y=0; y<LCD_H; y++) {
    uint8_t * p = &displayBuf[y/2 * LCD_W];

    lcdWriteAddress(0, y);
    lcdWriteCommand(0xAF);

    LCD_CLK_HIGH();
    LCD_A0_HIGH();
    LCD_NCS_LOW();

    for (uint32_t x=0; x<LCD_W; x++) {
      uint8_t b = p[x];
      if (y & 1)
        b >>= 4;
      LCD_WRITE_BIT(b & 0x08);
      LCD_WRITE_BIT(b & 0x04);
      LCD_WRITE_BIT(b & 0x02);
      LCD_WRITE_BIT(b & 0x01);
    }

    LCD_NCS_HIGH();
    LCD_A0_HIGH();

    lcdWriteData(0);
  }
}

void lcdHardwareInit()
{
  gpio_init(LCD_MOSI_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(LCD_CLK_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(LCD_A0_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  
  gpio_init(LCD_NCS_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  LCD_NCS_HIGH();
  
  gpio_init(LCD_RST_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
}

/*
  Proper method for turning of LCD module. It must be used,
  otherwise we might damage LCD crystals in the long run!
*/
void lcdOff()
{
  /*
  LCD Sleep mode is also good for draining capacitors and enables us
  to re-init LCD without any delay
  */
  lcdWriteCommand(0xAE); // LCD sleep
  delay_ms(3);	         // wait for caps to drain
}

void lcdReset()
{
  LCD_RST_LOW();
  delay_ms(1);    // only 3 us needed according to data-sheet, we use 1 ms
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

  if (!WAS_RESET_BY_WATCHDOG_OR_SOFTWARE()) {
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
    uint32_t end = timersGetMsTick() + RESET_WAIT_DELAY_MS;
    while (timersGetMsTick() < end);
  }
  
  lcdStart();
  lcdWriteCommand(0xAF); // dc2=1, IC into exit SLEEP MODE, dc3=1 gray=ON, dc4=1 Green Enhanc mode disabled
  delay_ms(20); // Needed for internal DC-DC converter startup
}

void lcdSetRefVolt(uint8_t val)
{
  if (!lcdInitFinished) {
    lcdInitFinish();
  }

  lcdWriteCommand(0x81); // Set Vop
  lcdWriteCommand(val+CONTRAST_OFS); // 0-255
}
