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
#include "inactivity_timer.h"

static bool splashStarted = false;

void startSplash()
{
  splashStarted = true;
}

void cancelSplash()
{
  splashStarted = false;
}

#if defined(HR_GIMBAL_RGB)

#include "stm32_ws2812.h"

void ws2812update(void);

void rbgInitLoop(void)
{//10ms
  static uint16_t loopct=0;
  static uint8_t loop=0,rgbloopindex=0,rgbstep=0,r=0,g=0,b=0;
  uint8_t a;

  #define maxrgbvalue (SPLASH_TIMEOUT/4)    //SPLASH_TIMEOUT ms

  if(++loopct>=10)
  {//10ms
    loopct=0;
    if(rgbstep){
      if(rgbstep==1){
        if(++r>=maxrgbvalue)rgbstep=2;
      }
      else if(rgbstep==2){
        r--;
        if(++g>=maxrgbvalue)rgbstep=3;
      }
      else if(rgbstep==3){
        g--;
        if(++b>=maxrgbvalue)rgbstep=4;
      }
      else if(rgbstep==4){
        b--;
        if(b<=0)rgbstep=5;
      }
      else{
        r=g=b=0;
      }
      for(a=0;a<32+6;a++){
        ws2812_set_color(a, r, g, b);    //6-37
      }
    }
    else{
      if(loop==0){
        if(rgbloopindex<6)ws2812_set_color(rgbloopindex, 80, 0, 0);               //0-5
        else if(rgbloopindex<38)ws2812_set_color(rgbloopindex, 80, 0, 0);     //6-37
        else {rgbloopindex=0;loop=1;}
      }
      else if(loop==1){
        if(rgbloopindex<6)ws2812_set_color(rgbloopindex, 0, 80, 0);               //0-5
        else if(rgbloopindex<38)ws2812_set_color(rgbloopindex, 0, 80, 0);     //6-37
        else {rgbloopindex=0;loop=2;}
      }
      else if(loop==2){
        if(rgbloopindex<6)ws2812_set_color(rgbloopindex, 0, 0, 80);               //0-5
        else if(rgbloopindex<38)ws2812_set_color(rgbloopindex, 0, 0, 80);     //6-37
        else {
          rgbstep=1;
          }
      }
      rgbloopindex++;
    }
  }
}
#endif

void waitSplash()
{
  // Handle B&W splash screen

#if defined(PWR_BUTTON_PRESS)
  bool refresh = false;
#endif

  if (SPLASH_NEEDED() && splashStarted) {
    resetBacklightTimeout();
    drawSplash();

    getADC(); // init ADC array

    inactivityCheckInputs();

    tmr10ms_t tgtime = get_tmr10ms() + SPLASH_TIMEOUT;

  #if defined(HR_GIMBAL_RGB)
    if(SPLASH_TIMEOUT<(6000/10))  //time expand GIMBAL_RGB init loop
      tgtime = get_tmr10ms() +6000/10;
  #endif   

    while (tgtime > get_tmr10ms()) {
      RTOS_WAIT_TICKS(1);

      getADC();

      if (getEvent() || inactivityCheckInputs())
        return;
    #if defined(HR_GIMBAL_RGB)
        rbgInitLoop();
        ws2812update();
    #endif  
    #if defined(PWR_BUTTON_PRESS)
      uint32_t pwr_check = pwrCheck();
      if (pwr_check == e_power_off) {
        break;
      }
      else if (pwr_check == e_power_press) {
        refresh = true;
      }
      else if (pwr_check == e_power_on && refresh) {
        drawSplash();
        refresh = false;
      }
    #else
      if (pwrCheck() == e_power_off) {
        return;
      }
    #endif

      checkBacklight();
    }
  }
}
