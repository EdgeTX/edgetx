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

#include "stm32_pulse_driver.h"
#include "hal/trainer_driver.h"
#include "hal.h"

#include "opentx.h"

#define __IS_TRAINER_TIMER_OUT_CHANNEL_SUPPORTED(ch)             \
  (((ch) == LL_TIM_CHANNEL_CH1 || (ch) == LL_TIM_CHANNEL_CH2 ||  \
    (ch) == LL_TIM_CHANNEL_CH3 || (ch) == LL_TIM_CHANNEL_CH4) && \
   __STM32_PULSE_IS_TIMER_CHANNEL_SUPPORTED(ch))

#define __IS_TRAINER_TIMER_IN_CHANNEL_SUPPORTED(ch)            \
  ((ch) == LL_TIM_CHANNEL_CH1 || (ch) == LL_TIM_CHANNEL_CH2 || \
   (ch) == LL_TIM_CHANNEL_CH3 || (ch) == LL_TIM_CHANNEL_CH4)

#if defined(TRAINER_GPIO)

static_assert(__IS_TRAINER_TIMER_OUT_CHANNEL_SUPPORTED(TRAINER_OUT_TIMER_Channel),
              "Unsupported trainer timer output channel");

static_assert(__IS_TRAINER_TIMER_IN_CHANNEL_SUPPORTED(TRAINER_IN_TIMER_Channel),
              "Unsupported trainer timer input channel");
#else
void init_trainer_ppm() {}
void stop_trainer_ppm() {}
void init_trainer_capture() {}
void stop_trainer_capture() {}
#endif

static void (*_trainer_timer_isr)();
static const stm32_pulse_timer_t* _trainer_timer;

void init_trainer()
{
#if defined(TRAINER_DETECT_GPIO_PIN)
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);

  pinInit.Pin = TRAINER_DETECT_GPIO_PIN;
  pinInit.Mode = LL_GPIO_MODE_INPUT;
  pinInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(TRAINER_DETECT_GPIO, &pinInit);
#endif  

  _trainer_timer = nullptr;
  _trainer_timer_isr = nullptr;
}

static inline bool trainer_check_isr_flag(const stm32_pulse_timer_t* tim)
{
  switch (tim->TIM_Channel) {
    case LL_TIM_CHANNEL_CH1:
      if (LL_TIM_IsEnabledIT_CC1(tim->TIMx) &&
          LL_TIM_IsActiveFlag_CC1(tim->TIMx)) {
        LL_TIM_ClearFlag_CC1(tim->TIMx);
        return true;
      }
      break;
    case LL_TIM_CHANNEL_CH2:
      if (LL_TIM_IsEnabledIT_CC2(tim->TIMx) &&
          LL_TIM_IsActiveFlag_CC2(tim->TIMx)) {
        LL_TIM_ClearFlag_CC2(tim->TIMx);
        return true;
      }
      break;
    case LL_TIM_CHANNEL_CH3:
      if (LL_TIM_IsEnabledIT_CC3(tim->TIMx) &&
          LL_TIM_IsActiveFlag_CC3(tim->TIMx)) {
        LL_TIM_ClearFlag_CC3(tim->TIMx);
        return true;
      }
      break;
    case LL_TIM_CHANNEL_CH4:
      if (LL_TIM_IsEnabledIT_CC4(tim->TIMx) &&
          LL_TIM_IsActiveFlag_CC4(tim->TIMx)) {
        LL_TIM_ClearFlag_CC4(tim->TIMx);
        return true;
      }
      break;
  }
  return false;
}

static void trainer_in_isr()
{
  // proceed only if the channel flag was set
  // and the IRQ was enabled
  if (!trainer_check_isr_flag(_trainer_timer))
    return;

  uint16_t capture = 0;
  switch(_trainer_timer->TIM_Channel) {
  case LL_TIM_CHANNEL_CH1:
    capture = LL_TIM_IC_GetCaptureCH1(_trainer_timer->TIMx);
    break;
  case LL_TIM_CHANNEL_CH2:
    capture = LL_TIM_IC_GetCaptureCH2(_trainer_timer->TIMx);
    break;
  case LL_TIM_CHANNEL_CH3:
    capture = LL_TIM_IC_GetCaptureCH3(_trainer_timer->TIMx);
    break;
  case LL_TIM_CHANNEL_CH4:
    capture = LL_TIM_IC_GetCaptureCH4(_trainer_timer->TIMx);
    break;
  default:
    return;
  }

  captureTrainerPulses(capture);
}

static void _stop_trainer(const stm32_pulse_timer_t* tim)
{
  stm32_pulse_deinit(tim);
  _trainer_timer_isr = nullptr;
  _trainer_timer = nullptr;
}

static void _init_trainer_capture(const stm32_pulse_timer_t* tim)
{
  // set proper ISR handler first
  _trainer_timer = tim;
  _trainer_timer_isr = trainer_in_isr;

  stm32_pulse_init(tim, 0);
  stm32_pulse_config_input(tim);

  switch (tim->TIM_Channel) {
  case LL_TIM_CHANNEL_CH1:
    LL_TIM_EnableIT_CC1(tim->TIMx);
    break;
  case LL_TIM_CHANNEL_CH2:
    LL_TIM_EnableIT_CC2(tim->TIMx);
    break;
  case LL_TIM_CHANNEL_CH3:
    LL_TIM_EnableIT_CC3(tim->TIMx);
    break;
  case LL_TIM_CHANNEL_CH4:
    LL_TIM_EnableIT_CC4(tim->TIMx);
    break;
  }

  LL_TIM_EnableCounter(tim->TIMx);
}

#if defined(TRAINER_GPIO)

static const stm32_pulse_timer_t trainerOutputTimer = {
  .GPIOx = TRAINER_GPIO,
  .GPIO_Pin = TRAINER_OUT_GPIO_PIN,
  .GPIO_Alternate = TRAINER_GPIO_AF,
  .TIMx = TRAINER_TIMER,
  .TIM_Freq = TRAINER_TIMER_FREQ,
  .TIM_Channel = TRAINER_OUT_TIMER_Channel,
  .TIM_IRQn = TRAINER_TIMER_IRQn,
  .DMAx = nullptr,
  .DMA_Stream = 0,
  .DMA_Channel = 0,
  .DMA_IRQn = (IRQn_Type)0,
  .DMA_TC_CallbackPtr = nullptr,
};

static void trainerSendNextFrame()
{
  stm32_pulse_set_polarity(&trainerOutputTimer, GET_TRAINER_PPM_POLARITY());
  stm32_pulse_set_cmp_val(&trainerOutputTimer, GET_TRAINER_PPM_DELAY() * 2);
  
  // load the first period: next reload when CC compare event triggers
  trainerPulsesData.ppm.ptr = trainerPulsesData.ppm.pulses;
  TRAINER_TIMER->ARR = *(trainerPulsesData.ppm.ptr++);

  switch (trainerOutputTimer.TIM_Channel) {
  case LL_TIM_CHANNEL_CH1:
    LL_TIM_EnableIT_CC1(trainerOutputTimer.TIMx);
    break;
  case LL_TIM_CHANNEL_CH2:
    LL_TIM_EnableIT_CC2(trainerOutputTimer.TIMx);
    break;
  case LL_TIM_CHANNEL_CH3:
    LL_TIM_EnableIT_CC3(trainerOutputTimer.TIMx);
    break;
  case LL_TIM_CHANNEL_CH4:
    LL_TIM_EnableIT_CC4(trainerOutputTimer.TIMx);
    break;
  }
}

static void trainer_out_isr()
{
  // proceed only if the channel flag was set
  // and the IRQ was enabled
  if (!trainer_check_isr_flag(_trainer_timer))
    return;

  if (*trainerPulsesData.ppm.ptr) {
    // load next period
    LL_TIM_SetAutoReload(_trainer_timer->TIMx,
                         *(trainerPulsesData.ppm.ptr++));
  } else {
    setupPulsesPPMTrainer();
    trainerSendNextFrame();
  }  
}

void init_trainer_ppm()
{
  // set proper ISR handler first
  _trainer_timer = &trainerOutputTimer;
  _trainer_timer_isr = trainer_out_isr;

  stm32_pulse_init(&trainerOutputTimer, 0);
  stm32_pulse_config_output(&trainerOutputTimer, GET_TRAINER_PPM_POLARITY(),
                            LL_TIM_OCMODE_PWM1, GET_TRAINER_PPM_DELAY() * 2);

  setupPulsesPPMTrainer();
  trainerSendNextFrame();

  LL_TIM_EnableCounter(trainerOutputTimer.TIMx);
}

void stop_trainer_ppm()
{
  _stop_trainer(&trainerOutputTimer);
}

static const stm32_pulse_timer_t trainerInputTimer = {
  .GPIOx = TRAINER_GPIO,
  .GPIO_Pin = TRAINER_IN_GPIO_PIN,
  .GPIO_Alternate = TRAINER_GPIO_AF,
  .TIMx = TRAINER_TIMER,
  .TIM_Freq = TRAINER_TIMER_FREQ,
  .TIM_Channel = TRAINER_IN_TIMER_Channel,
  .TIM_IRQn = TRAINER_TIMER_IRQn,
  .DMAx = nullptr,
  .DMA_Stream = 0,
  .DMA_Channel = 0,
  .DMA_IRQn = (IRQn_Type)0,
  .DMA_TC_CallbackPtr = nullptr,
};

void init_trainer_capture()
{
  _init_trainer_capture(&trainerInputTimer);
}

void stop_trainer_capture()
{
  _stop_trainer(&trainerInputTimer);
}

#endif // TRAINER_GPIO

bool is_trainer_connected()
{
#if defined(TRAINER_DETECT_GPIO_PIN)
  bool set = LL_GPIO_IsInputPinSet(TRAINER_DETECT_GPIO, TRAINER_DETECT_GPIO_PIN);
#if defined(TRAINER_DETECT_INVERTED)
  return !set;
#else
  return set;
#endif
#else // TRAINER_DETECT_GPIO_PIN
  return true;
#endif
}

#if defined(TRAINER_GPIO) || (defined(TRAINER_MODULE_CPPM) && \
                              !defined(TRAINER_MODULE_CPPM_TIMER_IRQHandler))

#if !defined(TRAINER_TIMER_IRQHandler)
  #error "Missing TRAINER_TIMER_IRQHandler definition"
#endif

extern "C" void TRAINER_TIMER_IRQHandler()
{
  DEBUG_INTERRUPT(INT_TRAINER);

  if (_trainer_timer && _trainer_timer_isr)
    _trainer_timer_isr();
}
#endif

#if defined(TRAINER_MODULE_CPPM)

static_assert(__IS_TRAINER_TIMER_IN_CHANNEL_SUPPORTED(TRAINER_MODULE_CPPM_TIMER_Channel),
              "Unsupported trainer timer input channel");

static const stm32_pulse_timer_t trainerModuleTimer = {
  .GPIOx = TRAINER_MODULE_CPPM_GPIO,
  .GPIO_Pin = TRAINER_MODULE_CPPM_GPIO_PIN,
  .GPIO_Alternate = TRAINER_MODULE_CPPM_GPIO_AF,
  .TIMx = TRAINER_MODULE_CPPM_TIMER,
  .TIM_Freq = TRAINER_MODULE_CPPM_FREQ,
  .TIM_Channel = TRAINER_MODULE_CPPM_TIMER_Channel,
  .TIM_IRQn = TRAINER_MODULE_CPPM_TIMER_IRQn,
  .DMAx = nullptr,
  .DMA_Stream = 0,
  .DMA_Channel = 0,
  .DMA_IRQn = (IRQn_Type)0,
  .DMA_TC_CallbackPtr = nullptr,
};

void init_trainer_module_cppm()
{
  _init_trainer_capture(&trainerModuleTimer);
}

void stop_trainer_module_cppm()
{
  _stop_trainer(&trainerModuleTimer);
}

#if defined(TRAINER_MODULE_CPPM_TIMER_IRQHandler)
extern "C" void TRAINER_MODULE_CPPM_TIMER_IRQHandler()
{
  DEBUG_INTERRUPT(INT_TRAINER);

  if (_trainer_timer && _trainer_timer_isr)
    _trainer_timer_isr();
}
#endif

#endif // TRAINER_MODULE_CPPM

#if defined(TRAINER_MODULE_SBUS)

const etx_serial_init sbusTrainerParams = {
    .baudrate = SBUS_BAUDRATE,
    .parity = ETX_Parity_Even,
    .stop_bits = ETX_StopBits_Two,
    .word_length = ETX_WordLength_9,
    .rx_enable = true,
};

// external module may have a full-duplex USART
#if defined(EXTMODULE_USART) || defined(CONFIGURABLE_MODULE_PORT)

#include "hal/module_port.h"

static etx_module_state_t* sbus_trainer_mod_st = nullptr;

void init_trainer_module_sbus()
{
  if (sbus_trainer_mod_st) return;

  sbus_trainer_mod_st = modulePortInitSerial(EXTERNAL_MODULE, ETX_MOD_PORT_EXTERNAL_UART,
                                             ETX_MOD_DIR_RX, &sbusTrainerParams);
}

void stop_trainer_module_sbus()
{
  if (!sbus_trainer_mod_st) return;
  modulePortDeInit(sbus_trainer_mod_st);
}

int trainerModuleSbusGetByte(uint8_t* data)
{
  if (!sbus_trainer_mod_st) return 0;

  auto serial_driver = modulePortGetSerialDrv(sbus_trainer_mod_st->rx);
  auto ctx = modulePortGetCtx(sbus_trainer_mod_st->rx);

  if (ctx) {
    return serial_driver->getByte(ctx, data);
  }

  return 0;
}

#elif defined(TRAINER_MODULE_SBUS_USART)
#include "stm32_usart_driver.h"

static const LL_GPIO_InitTypeDef sbus_trainer_USART_PinDef = {
  .Pin = TRAINER_MODULE_SBUS_GPIO_PIN,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = TRAINER_MODULE_SBUS_GPIO_AF,
};

static const stm32_usart_t sbus_trainer_USART = {
  .USARTx = TRAINER_MODULE_SBUS_USART,
  .GPIOx = TRAINER_MODULE_SBUS_GPIO,
  .pinInit = &sbus_trainer_USART_PinDef,
  .IRQn = (IRQn_Type)-1,
  .IRQ_Prio = 0,
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
  .rxDMA = TRAINER_MODULE_SBUS_DMA,
  .rxDMA_Stream = TRAINER_MODULE_SBUS_DMA_STREAM_LL,
  .rxDMA_Channel = TRAINER_MODULE_SBUS_DMA_CHANNEL,
};

static DMAFifo<32> sbus_trainer_fifo __DMA (TRAINER_MODULE_SBUS_DMA_STREAM);

void init_trainer_module_sbus()
{
  stm32_usart_init(&sbus_trainer_USART, &sbusTrainerParams);
  stm32_usart_init_rx_dma(&sbus_trainer_USART, sbus_trainer_fifo.buffer(),
                          sbus_trainer_fifo.size());
}

void stop_trainer_module_sbus()
{
  stm32_usart_deinit(&sbus_trainer_USART);
}

int trainerModuleSbusGetByte(uint8_t* data)
{
  return sbus_trainer_fifo.pop(*data);
}

#else
  #error "No available SBUS trainer implementation"
#endif
#endif // TRAINER_MODULE_SBUS
