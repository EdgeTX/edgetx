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

#include "edgetx.h"
#include "stm32_ws2812.h"


#if defined(LED_STRIP_GPIO)
// Common LED driver
extern const stm32_pulse_timer_t _led_timer;

void ledStripOff()
{
  for (uint8_t i = 0; i < LED_STRIP_LENGTH; i++) {
    ws2812_set_color(i, 0, 0, 0);
  }
  ws2812_update(&_led_timer);
}
void ws2812update(void)
{
  ws2812_update(&_led_timer);
}
#endif


#define SIXPOS_INDEX	5

uint16_t getAnalogValue(uint8_t index);

static uint8_t sixpostkey=1;
static uint16_t oldadcValue=0;

uint8_t sixpostkeyflag=0;

uint16_t getMultiPosAnalogValue(uint16_t adcValue)
{
	int16_t adc=0;	//3909 3227 2437 1638 1107 525
	
  oldadcValue=adcValue;

  if(sixpostkey)
	  adc=(4096/5)*(sixpostkey-1);

	return adc;
}

void MultiposKey(void)
{//50ms
  static uint8_t delayct=0,updataled=1;

  uint8_t r[6]={0}; 
  uint8_t g[6]={0}; 
  uint8_t b[6]={0}; 

	if(sixpostkey==0)
	{//led loop test...
    if(++delayct>=(6*9+1))
		{
      updataled=1;
      sixpostkey=1;
			delayct=1;
      r[0]=160;
		}
    else
    {
      updataled=delayct%6;
      if((delayct/6)==0)      r[updataled]  =3;
      else if((delayct/6)==1) g[5-updataled]=3;
      else if((delayct/6)==2) b[updataled]  =3;
      else if((delayct/6)==3) r[updataled]  =10;
      else if((delayct/6)==4) g[5-updataled]=10;
      else if((delayct/6)==5) b[updataled]  =10;
      else if((delayct/6)==6) r[updataled]  =50;
      else if((delayct/6)==7) g[5-updataled]=50;
      else if((delayct/6)==8) b[updataled]  =50;

      updataled=1;
    }
	}
	else
	{//key function
    sixpostkeyflag=1;
		int16_t adc=oldadcValue; //getAnalogValue(SIXPOS_INDEX);	//3909 3227 2437 1638 1107 525
		sixpostkeyflag=0;

    if(delayct<2&&adc>200)
    {
      updataled=1;
      if(adc>3800){if(++delayct>1)sixpostkey=6;}
      else if(adc>3100){if(++delayct>1)sixpostkey=5;}
      else if(adc>2300){if(++delayct>1)sixpostkey=4;}
      else if(adc>1500){if(++delayct>1)sixpostkey=3;}
      else if(adc>1000){if(++delayct>1)sixpostkey=2;}
      else if(adc>400) {if(++delayct>1)sixpostkey=1;}
      else 
      {
        delayct=0;
        updataled=0;
      }
    }
    else if(delayct&&adc<100) 
    {
      delayct--;
    }
    r[sixpostkey-1]=160;
	}
	if(updataled)
	{
		updataled--;

    for(int i=0;i<6;i++)
    {
      if(sixpostkey)b[i]=4;
    }
    ws2812_set_color(0,r[0],g[0],b[0]);
    ws2812_set_color(1,r[1],g[1],b[1]);
    ws2812_set_color(2,r[2],g[2],b[2]);
    ws2812_set_color(3,r[3],g[3],b[3]);
    ws2812_set_color(4,r[4],g[4],b[4]);
    ws2812_set_color(5,r[5],g[5],b[5]);
	}
}

