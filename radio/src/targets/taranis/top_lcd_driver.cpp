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

#include "edgetx.h"
#include "board.h"
#include "lcd.h"

#include <stdlib.h>

#define CS1_HIGH()     gpio_set(TOPLCD_GPIO_CS1)
#define CS1_LOW()      gpio_clear(TOPLCD_GPIO_CS1)
#define CS2_HIGH()     gpio_set(TOPLCD_GPIO_CS2)
#define CS2_LOW()      gpio_clear(TOPLCD_GPIO_CS2)
#define WR_HIGH()      gpio_set(TOPLCD_GPIO_WR)
#define WR_LOW()       gpio_clear(TOPLCD_GPIO_WR)
#define DATA_HIGH()    gpio_set(TOPLCD_GPIO_DATA)
#define DATA_LOW()     gpio_clear(TOPLCD_GPIO_DATA)
#define BL_ON()        gpio_set(TOPLCD_GPIO_BL)
#define BL_OFF()       gpio_clear(TOPLCD_GPIO_BL)

const uint8_t TimeLCDsegs[] = { 0xAF, 0x06, 0x6D, 0x4F, 0xC6, 0xCB, 0xEB, 0x0E, 0xEF, 0xCF };
const uint8_t RssiLCDsegs[] = { 0xFA, 0x60, 0xBC, 0xF4, 0x66, 0xD6, 0xDE, 0x70, 0xFE, 0xF6 };
const uint8_t OpTimeLCDsegs[] = { 0x5F, 0x06, 0x6B, 0x2F, 0x36, 0x3D, 0x7D, 0x07, 0x7F, 0x3F };

uint8_t Ht1621Data1[11] = { 0 };
uint8_t Ht1621Data2[7] = { 0 };

void delay1_7us()
{
  delay_01us(17);
}

void ht1621SendCommand(uint8_t chip, unsigned char command)
{
  if (chip)
    CS2_LOW();
  else
    CS1_LOW();

  delay1_7us();
  WR_LOW();      //PRESENT 100 COMMAND CODE
  delay1_7us();
  DATA_HIGH();
  delay1_7us();
  WR_HIGH();
  delay1_7us();
  WR_LOW();
  delay1_7us();
  DATA_LOW();
  delay1_7us();
  WR_HIGH();
  delay1_7us();

  WR_LOW();
  delay1_7us();
  DATA_LOW();
  delay1_7us();
  WR_HIGH();
  delay1_7us();

  for (int i=0; i<8; i++) {
    WR_LOW();
    delay1_7us();
    if ((command & 0x80) !=0) {
      DATA_HIGH();
      delay1_7us();
    }
    else {
      DATA_LOW();
      delay1_7us();
    }
    WR_HIGH();
    delay1_7us();
    command = command << 1;
  }
  WR_LOW();
  delay1_7us();
  WR_HIGH();
  delay1_7us();

  if (chip)
    CS2_HIGH();
  else
    CS1_HIGH();

  delay1_7us();
}

void ht1621WrData(uint8_t data, uint8_t count)
{
  while (count) {
    WR_LOW();
    if (data & 0x80)
      DATA_HIGH();
    else
      DATA_LOW();
    delay1_7us();
    WR_HIGH();
    delay1_7us();
    data <<= 1;
    count -= 1;
  }
}

void ht1621WrAllData(uint8_t chip, uint8_t *pData)
{
  int len;
  if (chip) {
    len = sizeof(Ht1621Data2);
    CS2_LOW();
  }
  else {
    len = sizeof(Ht1621Data1);
    CS1_LOW();
  }
  delay1_7us();
  ht1621WrData(0xa0, 3);
  ht1621WrData(0, 6); // HT1621 6 bit,left 2 bit;
  for (int i=0; i<len; i++) {
    ht1621WrData(*pData++, 8);
  }
  if (chip)
    CS2_HIGH();
  else
    CS1_HIGH();
  delay1_7us();
}

#if 0
void initTimerTopLcd()
{
  // Timer12
  RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;           // Enable clock
  TIM12->ARR = 17;       // 1.7uS
  TIM12->PSC = (PERI1_FREQUENCY * TIMER_MULT_APB1) / 10000000 - 1;             // 0.1uS from 30MHz
  TIM12->CCER = 0;
  TIM12->CCMR1 = 0;
  TIM12->EGR = 0;
  TIM12->CR1 = 5;
  // TIM12->DIER |= 1;
  NVIC_SetPriority(TIM8_BRK_TIM12_IRQn, 2 );
  NVIC_EnableIRQ(TIM8_BRK_TIM12_IRQn);
}

extern "C" void TIM8_BRK_TIM12_IRQHandler()
{
  struct t_top_lcd_control *pc;
  TIM12->SR &= ~TIM_SR_UIF;
  pc = &TopLcdControl;

  if ( pc->state == TOP_LCD_CKLOW ) {
    WR_LOW();      //PRESENT 100 COMMAND CODE
    if ((*pc->data & 0x80) !=0) {
      DATA_HIGH();
    }
    else {
      DATA_LOW();
    }
    *pc->data <<= 1;
    pc->state = TOP_LCD_CKHI;
  }
  else if ( pc->state == TOP_LCD_CKHI ) {
    WR_HIGH();
    if ( --pc->count == 0 ) {
      pc->state = TOP_LCD_END;
    }
    else {
      pc->state = TOP_LCD_CKLOW;
    }
  }
  else if (pc->state == TOP_LCD_END) {
    if (pc->chip) {
      CS2_HIGH();
      pc->state = TOP_LCD_IDLE;
      TIM12->DIER &= ~1;
    }
    else {
      CS1_HIGH();
      pc->chip = 1;
      pc->data = pc->data2;
      pc->count = pc->count2;
      pc->state = TOP_LCD_IDLE;
    }
  }
  else if ( pc->state == TOP_LCD_IDLE ) {
    WR_HIGH();
    if ( pc->chip   ) CS2_LOW(); else CS1_LOW();
    pc->state = TOP_LCD_1CKLOW;
  }
  else if ( pc->state == TOP_LCD_1CKLOW ) {
    WR_LOW();
    DATA_HIGH();             // First 1 bit
    pc->state = TOP_LCD_1CKHI;
  }
  else if ( pc->state == TOP_LCD_1CKHI ) {
    WR_HIGH();
    if ( --pc->count == 0 ) {
      pc->state = TOP_LCD_END;
    }
    else {
      pc->state = TOP_LCD_CKLOW;
    }
  }
}
#endif

void toplcdInit()
{
  gpio_init(TOPLCD_GPIO_DATA, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(TOPLCD_GPIO_WR, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(TOPLCD_GPIO_CS1, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(TOPLCD_GPIO_CS2, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(TOPLCD_GPIO_BL, GPIO_OUT, GPIO_PIN_SPEED_LOW);

  BL_ON();

  ht1621SendCommand(0, 0x03);
  ht1621SendCommand(0, 0x01);
  ht1621SendCommand(0, 0x29);

  ht1621SendCommand(1, 0x03);
  ht1621SendCommand(1, 0x01);
  ht1621SendCommand(1, 0x29);

  toplcdRefreshStart();
  toplcdRefreshEnd();
}

void toplcdOff()
{
  BL_OFF();
  toplcdRefreshStart();
  toplcdRefreshEnd();
}

void setTopFirstTimer(int32_t value)
{
  Ht1621Data1[2] |= 0x10; // ":"

  if (value < 0) {
    if (BLINK_ON_PHASE) {
      return;
    }
    else {
      value = -value;
    }
  }

  div_t qr = div((int)value, 60);
  uint32_t r = qr.rem;
  qr = div(qr.quot, 10);
  
  // Limit to 99 min 99 sec to avoid breaking display
  if (qr.quot > 9) {
    qr.quot = 9;
    qr.rem = 9;
    r = 99;
  }
  
  Ht1621Data1[0] |= TimeLCDsegs[qr.quot];
  Ht1621Data1[1] |= TimeLCDsegs[qr.rem];
  qr = div(r, 10);
  Ht1621Data1[2] |= TimeLCDsegs[qr.quot];
  Ht1621Data1[3] |= TimeLCDsegs[qr.rem];
}

void setTopRssiValue(uint32_t rssi)
{
  div_t qr = div(rssi, 10);
  uint32_t r = 0;
  if (qr.quot > 9) {
    r = 1;
    qr.quot -= 10;
  }
  Ht1621Data2[2] |= r ? RssiLCDsegs[1] : 0;
  Ht1621Data2[1] |= (r || qr.quot) ? RssiLCDsegs[qr.quot] : 0;
  Ht1621Data2[0] |= RssiLCDsegs[qr.rem];
}

void setTopRssiBar(uint32_t rssi)
{
  if (rssi > 42)
    Ht1621Data2[2] |= 1;
  if (rssi > 45)
    Ht1621Data2[1] |= 1;
  if (rssi > 50)
    Ht1621Data2[0] |= 1;
  if (rssi > 60)
    Ht1621Data2[3] |= 0x10;
  if (rssi > 70)
    Ht1621Data2[3] |= 0x20;
  if (rssi > 80)
    Ht1621Data2[3] |= 0x40;
  if (rssi > 90)
    Ht1621Data2[3] |= 0x80;
}

void setTopRssi(uint32_t rssi)
{
  Ht1621Data1[4] |= 0x10; // "RSSI"
  Ht1621Data2[3] |= 0x01; // "dB"
  Ht1621Data2[4] |= 0x01; // Horizontal line
  setTopRssiValue(rssi);
  setTopRssiBar(rssi);
}

void setTopBatteryState(int state, uint8_t blinking)
{
  if (!blinking || BLINK_ON_PHASE) {
    // since the border can not be turned off,
    // we blink the first bar even if the actual value is zero
    if (blinking && state < 1) {
      state = 1;
    }
    Ht1621Data1[4] |= 0x40; // Battery border  // TODO this is not working for me, the border is ALWAYS on
    if (state > 0)
      Ht1621Data1[7] |= 0x80;
    if (state > 2)
      Ht1621Data1[9] |= 0x80;
    if (state > 4)
      Ht1621Data1[5] |= 0x80;
    if (state > 6)
      Ht1621Data1[10] |= 0x80;
    if (state > 8)
      Ht1621Data1[4] |= 0x80;
  }
}

void setTopBatteryValue(uint32_t volts)
{
  div_t qr;
  uint32_t r = 0;
  uint32_t segs;
  qr = div(volts, 10);
  if (qr.quot > 9) {
    r = 1;
    qr.quot -= 10;
  }
  segs = r ? RssiLCDsegs[1] : 0;
  Ht1621Data2[3] |= ((segs & 7) << 1);
  Ht1621Data2[4] |= segs & 0xF0;
  segs = (r || qr.quot) ? RssiLCDsegs[qr.quot] : 0;
  Ht1621Data2[4] |= (segs & 0x0E);
  Ht1621Data2[5] |= segs & 0xF0;
  segs = RssiLCDsegs[qr.rem];
  Ht1621Data2[5] |= (segs & 0x0E) | 1;   // Decimal point
  Ht1621Data2[6] |= segs & 0xF0;
  Ht1621Data1[4] |= 0x20; // "V"
}

void setTopSecondTimer(uint32_t value)
{
  div_t qr;
  uint32_t segs;
  uint32_t segs2;

  qr = div(value, 60);
  uint32_t secs = qr.rem;
  qr = div(qr.quot, 60);
  uint32_t mins = qr.rem;
  uint32_t hours = min(qr.quot, 99);

  qr = div(secs, 10);
  segs = OpTimeLCDsegs[qr.rem];
  Ht1621Data1[4] |= (segs & 0x0F) | 0x70;
  segs &= 0x70;
  segs2 = OpTimeLCDsegs[qr.quot];
  Ht1621Data1[5] |= segs | (segs2 & 0x0F);

  qr = div(mins, 10);
  segs = OpTimeLCDsegs[qr.rem];
  segs2 &= 0x70;
  Ht1621Data1[6] |= segs2 | (segs & 0x0F) | 0x80; // ":"
  segs &= 0x70;
  segs2 = OpTimeLCDsegs[qr.quot];
  Ht1621Data1[7] |= segs | (segs2 & 0x0F);

  qr = div(hours, 10);
  segs = OpTimeLCDsegs[qr.rem];
  segs2 &= 0x70;
  Ht1621Data1[8] |= segs2 | (segs & 0x0F) | 0x80; // ":"
  segs &= 0x70;
  segs2 = OpTimeLCDsegs[qr.quot];
  Ht1621Data1[9] |= segs | (segs2 & 0x0F);
  Ht1621Data1[10] |= segs2 & 0xF0;

  Ht1621Data1[3] |= 0x10; // "Operation Time"
}

void toplcdRefreshStart()
{
  memset(Ht1621Data1, 0, sizeof(Ht1621Data1));
  memset(Ht1621Data2, 0, sizeof(Ht1621Data2));
}

void toplcdRefreshEnd()
{
  ht1621WrAllData(0, Ht1621Data1);
  ht1621WrAllData(1, Ht1621Data2);
}
