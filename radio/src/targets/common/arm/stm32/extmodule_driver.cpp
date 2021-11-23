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

extern "C" {
  #define USE_FULL_LL_DRIVER
  #if defined(STM32F4)
    #include "CMSIS/Device/ST/STM32F4xx/Include/stm32f4xx.h"
    #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_gpio.h"
    #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_tim.h"
    #include "STM32F4xx_HAL_Driver/Inc/stm32f4xx_ll_dma.h"
  #elif defined(STM32F2)
    #include "CMSIS/Device/ST/STM32F2xx/Include/stm32f2xx.h"
    #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_gpio.h"
    #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_tim.h"
    #include "STM32F2xx_HAL_Driver/Inc/stm32f2xx_ll_dma.h"
  #endif
}

#include "hal.h"
#include "extmodule_driver.h"
#include "timers_driver.h"

#if defined(CROSSFIRE)
#include "pulses/crossfire.h"
#endif

void extmoduleStop()
{
  EXTERNAL_MODULE_OFF();

  // De-init DMA & timer
  LL_DMA_DeInit(EXTMODULE_TIMER_DMA, EXTMODULE_TIMER_DMA_STREAM_LL);
  LL_TIM_DeInit(EXTMODULE_TIMER);

  // Reconfigure pin as output
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = EXTMODULE_TX_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  LL_GPIO_Init(EXTMODULE_TX_GPIO, &pinInit);
}

static void extmoduleInitTxTimerPin()
{
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = EXTMODULE_TX_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_ALTERNATE;
  pinInit.Alternate = EXTMODULE_TIMER_TX_GPIO_AF;
  LL_GPIO_Init(EXTMODULE_TX_GPIO, &pinInit);
}

static void extmoduleInitBaseTimer()
{
  LL_TIM_InitTypeDef timInit;
  LL_TIM_StructInit(&timInit);

  // 0.5uS (2Mhz)
  timInit.Prescaler = __LL_TIM_CALC_PSC(EXTMODULE_TIMER_FREQ, 2000000);
  timInit.Autoreload = 65535;
  LL_TIM_Init(EXTMODULE_TIMER, &timInit);
}

static void extmoduleStartOutput(LL_TIM_OC_InitTypeDef* p_ocInit)
{
  LL_TIM_DisableCounter(EXTMODULE_TIMER);
  LL_TIM_OC_Init(EXTMODULE_TIMER, EXTMODULE_TIMER_Channel, p_ocInit);
  LL_TIM_OC_EnablePreload(EXTMODULE_TIMER, EXTMODULE_TIMER_Channel);

  if (IS_TIM_BREAK_INSTANCE(EXTMODULE_TIMER)) {
    LL_TIM_EnableAllOutputs(EXTMODULE_TIMER);
  }

  LL_TIM_EnableDMAReq_UPDATE(EXTMODULE_TIMER);
  LL_TIM_EnableCounter(EXTMODULE_TIMER);
}

static void extmoduleStartTimerDMARequest(const void* pulses, uint16_t length)
{
  LL_DMA_DeInit(EXTMODULE_TIMER_DMA, EXTMODULE_TIMER_DMA_STREAM_LL);

  LL_DMA_InitTypeDef dmaInit;
  LL_DMA_StructInit(&dmaInit);

  // Direction
  dmaInit.Direction = LL_DMA_DIRECTION_MEMORY_TO_PERIPH;
  
  // Source
  dmaInit.MemoryOrM2MDstAddress = CONVERT_PTR_UINT(pulses);
  dmaInit.MemoryOrM2MDstIncMode = LL_DMA_MEMORY_INCREMENT;

  // Destination
  dmaInit.PeriphOrM2MSrcAddress = CONVERT_PTR_UINT(&EXTMODULE_TIMER->ARR);
  dmaInit.PeriphOrM2MSrcIncMode = LL_DMA_PERIPH_NOINCREMENT;

  // Data width
  if (IS_TIM_32B_COUNTER_INSTANCE(EXTMODULE_TIMER)) {
    dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_WORD;
    dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_WORD;
  } else {
    dmaInit.MemoryOrM2MDstDataSize = LL_DMA_MDATAALIGN_HALFWORD;
    dmaInit.PeriphOrM2MSrcDataSize = LL_DMA_PDATAALIGN_HALFWORD;
  }

  dmaInit.NbData = length;
  dmaInit.Channel = EXTMODULE_TIMER_DMA_CHANNEL;
  dmaInit.Priority = LL_DMA_PRIORITY_VERYHIGH;
  LL_DMA_Init(EXTMODULE_TIMER_DMA, EXTMODULE_TIMER_DMA_STREAM_LL, &dmaInit);

  // Enable DMA
  LL_DMA_EnableStream(EXTMODULE_TIMER_DMA, EXTMODULE_TIMER_DMA_STREAM_LL);

  // re-init timer
  LL_TIM_GenerateEvent_UPDATE(EXTMODULE_TIMER);
  LL_TIM_EnableCounter(EXTMODULE_TIMER);
}

void extmodulePpmStart(uint16_t ppm_delay, bool polarity)
{
  EXTERNAL_MODULE_ON();
  extmoduleInitTxTimerPin();
  extmoduleInitBaseTimer();

  // PPM generation principle:
  //
  // Hardware timer in PWM mode is used for PPM generation
  // Output is OFF if CNT<CCR1(delay) and ON if bigger
  // CCR1 register defines duration of pulse length and is constant
  // AAR register defines duration of each pulse, it is
  // updated after every pulse in Update interrupt handler.

  LL_TIM_OC_InitTypeDef ocInit;
  LL_TIM_OC_StructInit(&ocInit);

  ocInit.OCMode = LL_TIM_OCMODE_PWM1;
  ocInit.OCState = LL_TIM_OCSTATE_ENABLE;

  if (polarity) {
    ocInit.OCPolarity = LL_TIM_OCPOLARITY_LOW;
  } else {
    ocInit.OCPolarity = LL_TIM_OCPOLARITY_HIGH;
  }

  ocInit.CompareValue = ppm_delay * 2;
  extmoduleStartOutput(&ocInit);
}

#if defined(PXX1)
void extmodulePxx1PulsesStart()
{
  EXTERNAL_MODULE_ON();
  extmoduleInitTxTimerPin();
  extmoduleInitBaseTimer();

  LL_TIM_OC_InitTypeDef ocInit;
  LL_TIM_OC_StructInit(&ocInit);

  ocInit.OCMode = LL_TIM_OCMODE_PWM1;
  ocInit.OCState = LL_TIM_OCSTATE_ENABLE;
  ocInit.OCNState = LL_TIM_OCSTATE_ENABLE;
  ocInit.OCPolarity = LL_TIM_OCPOLARITY_LOW;
  ocInit.CompareValue = 9 * 2; // 9 uS

  extmoduleStartOutput(&ocInit);
}
#endif

void extmoduleSerialStart()
{
  EXTERNAL_MODULE_ON();
  extmoduleInitTxTimerPin();
  extmoduleInitBaseTimer();

  LL_TIM_OC_InitTypeDef ocInit;
  LL_TIM_OC_StructInit(&ocInit);

  ocInit.OCMode = LL_TIM_OCMODE_TOGGLE;
  ocInit.OCState = LL_TIM_OCSTATE_ENABLE;
  ocInit.OCPolarity = LL_TIM_OCPOLARITY_LOW;
  ocInit.CompareValue = 0;

  extmoduleStartOutput(&ocInit);
}

void extmoduleSendNextFramePpm(void* pulses, uint16_t length,
                               uint16_t ppm_delay, bool polarity)
{
  if (LL_DMA_IsEnabledStream(EXTMODULE_TIMER_DMA,
                             EXTMODULE_TIMER_DMA_STREAM_LL))
    return;

  // disable timer
  LL_TIM_DisableCounter(EXTMODULE_TIMER);

  switch(EXTMODULE_TIMER_Channel){
  case LL_TIM_CHANNEL_CH1:
    LL_TIM_OC_SetCompareCH1(EXTMODULE_TIMER, ppm_delay * 2);
    break;
  case LL_TIM_CHANNEL_CH3:
    LL_TIM_OC_SetCompareCH3(EXTMODULE_TIMER, ppm_delay * 2);
    break;
  }

  uint32_t ll_polarity;
  if (polarity) {
    ll_polarity = LL_TIM_OCPOLARITY_LOW;
  } else {
    ll_polarity = LL_TIM_OCPOLARITY_HIGH;
  }
  LL_TIM_OC_SetPolarity(EXTMODULE_TIMER, EXTMODULE_TIMER_Channel, ll_polarity);

  // Start DMA request and re-enable timer
  extmoduleStartTimerDMARequest(pulses, length);
}

#if defined(PXX1)
void extmoduleSendNextFramePxx1(const void* pulses, uint16_t length)
{
  if (LL_DMA_IsEnabledStream(EXTMODULE_TIMER_DMA,
                             EXTMODULE_TIMER_DMA_STREAM_LL))
    return;

  // disable timer
  LL_TIM_DisableCounter(EXTMODULE_TIMER);

  // Start DMA request and re-enable timer
  extmoduleStartTimerDMARequest(pulses, length);
}
#endif

#if defined(AFHDS3) && !(defined(EXTMODULE_USART) && defined(EXTMODULE_TX_INVERT_GPIO))
void extmoduleSendNextFrameAFHDS3(const void* pulses, uint16_t length)
{
  if (LL_DMA_IsEnabledStream(EXTMODULE_TIMER_DMA,
                             EXTMODULE_TIMER_DMA_STREAM_LL))
    return;

  // Start DMA request and re-enable timer
  extmoduleStartTimerDMARequest(pulses, length);
}
#endif

void extmoduleSendNextFrameSoftSerial100kbit(const void* pulses, uint16_t length,
                                             bool polarity)
{
  if (LL_DMA_IsEnabledStream(EXTMODULE_TIMER_DMA,
                             EXTMODULE_TIMER_DMA_STREAM_LL))
    return;


  // disable timer
  LL_TIM_DisableCounter(EXTMODULE_TIMER);

  uint32_t ll_polarity;
  if (polarity) {
    ll_polarity = LL_TIM_OCPOLARITY_LOW;
  } else {
    ll_polarity = LL_TIM_OCPOLARITY_HIGH;
  }
  LL_TIM_OC_SetPolarity(EXTMODULE_TIMER, EXTMODULE_TIMER_Channel, ll_polarity);

  // Start DMA request and re-enable timer
  extmoduleStartTimerDMARequest(pulses, length);
}

void extmoduleSendInvertedByte(uint8_t byte)
{
  uint16_t time;
  uint32_t i;

  __disable_irq();
  time = getTmr2MHz();
  LL_GPIO_SetOutputPin(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
  while ((uint16_t) (getTmr2MHz() - time) < 34)	{
    // wait
  }
  time += 34;
  for (i = 0; i < 8; i++) {
    if (byte & 1) {
      LL_GPIO_ResetOutputPin(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
    }
    else {
      LL_GPIO_SetOutputPin(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
    }
    byte >>= 1 ;
    while ((uint16_t) (getTmr2MHz() - time) < 35) {
      // wait
    }
    time += 35 ;
  }
  LL_GPIO_ResetOutputPin(EXTMODULE_TX_GPIO, EXTMODULE_TX_GPIO_PIN);
  __enable_irq();	// No need to wait for the stop bit to complete
  while ((uint16_t) (getTmr2MHz() - time) < 34) {
    // wait
  }
}
