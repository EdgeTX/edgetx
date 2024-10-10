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
#include "hal/module_port.h"

#include "stm32_gpio.h"
#include "stm32_serial_driver.h"
#include "stm32_softserial_driver.h"
#include "stm32_dma.h"
#include "trainer_driver.h"

#include "module_ports.h"
#include "board.h"
#include "dataconstants.h"

#if defined (HARDWARE_INTERNAL_MODULE)
#if defined(INTMODULE_USART)

#define INTMODULE_USART_IRQ_PRIORITY 5

static const stm32_usart_t intmoduleUSART = {
  .USARTx = INTMODULE_USART,
  .txGPIO = INTMODULE_TX_GPIO,
  .rxGPIO = INTMODULE_RX_GPIO,
  .IRQn = INTMODULE_USART_IRQn,
  .IRQ_Prio = INTMODULE_USART_IRQ_PRIORITY,
#if defined(INTMODULE_DMA)
  .txDMA = INTMODULE_DMA,
  .txDMA_Stream = INTMODULE_DMA_STREAM,
  .txDMA_Channel = INTMODULE_DMA_CHANNEL,
#else
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
#endif
#if defined(INTMODULE_RX_DMA)
  .rxDMA = INTMODULE_RX_DMA,
  .rxDMA_Stream = INTMODULE_RX_DMA_STREAM,
  .rxDMA_Channel = INTMODULE_RX_DMA_CHANNEL,
#else
  .rxDMA = nullptr,
  .rxDMA_Stream = 0,
  .rxDMA_Channel = 0,
#endif
};

DEFINE_STM32_SERIAL_PORT(InternalModule, intmoduleUSART, INTMODULE_FIFO_SIZE, 0);

#else // INTMODULE_USART

#include "stm32_pulse_driver.h"
#include "timers_driver.h"

static stm32_pulse_dma_tc_cb_t _int_timer_DMA_TC_Callback;

static const stm32_pulse_timer_t intmoduleTimer = {
  .GPIO = INTMODULE_TX_GPIO,
  .GPIO_Alternate = INTMODULE_TX_GPIO_AF,
  .TIMx = INTMODULE_TIMER,
  .TIM_Freq = INTMODULE_TIMER_FREQ,
  .TIM_Channel = INTMODULE_TIMER_Channel,
  .TIM_IRQn = INTMODULE_TIMER_IRQn,
  .DMAx = INTMODULE_TIMER_DMA,
  .DMA_Stream = INTMODULE_TIMER_DMA_STREAM,
  .DMA_Channel = INTMODULE_TIMER_DMA_CHANNEL,
  .DMA_IRQn = INTMODULE_TIMER_DMA_STREAM_IRQn,
  .DMA_TC_CallbackPtr = &_int_timer_DMA_TC_Callback,
};

// Make sure the timer channel is supported
static_assert(__STM32_PULSE_IS_TIMER_CHANNEL_SUPPORTED(INTMODULE_TIMER_Channel),
              "Unsupported timer channel");

// Make sure the DMA channel is supported
static_assert(__STM32_DMA_IS_STREAM_SUPPORTED(INTMODULE_TIMER_DMA_STREAM),
              "Unsupported DMA stream");

#if !defined(INTMODULE_TIMER_DMA_IRQHandler)
#error "Missing INTMODULE_TIMER_DMA_IRQHandler definition"
#endif

extern "C" void INTMODULE_TIMER_DMA_IRQHandler()
{
  stm32_pulse_dma_tc_isr(&intmoduleTimer);
}

#if !defined(INTMODULE_TIMER_IRQHandler)
#error "Missing INTMODULE_TIMER_IRQHandler definition"
#endif

extern "C" void INTMODULE_TIMER_IRQHandler()
{
  stm32_pulse_tim_update_isr(&intmoduleTimer);
}

DEFINE_STM32_SOFTSERIAL_PORT(InternalModule, intmoduleTimer);

#endif // INTMODULE_USART
#endif // HARDWARE_INTERNAL_MODULE

#include "module_timer_driver.h"

#if defined(HARDWARE_EXTERNAL_MODULE)
#if defined(EXTMODULE_USART)

#define EXTMODULE_USART_IRQ_PRIORITY 6

#if defined(EXTMODULE_TX_INVERT_GPIO) && defined(EXTMODULE_RX_INVERT_GPIO)

#define HAS_EXTMODULE_INVERTERS

static void _extmod_set_inverted(uint8_t enable)
{
  // In EdgeTX the external module is normally always
  // inverted, so invert the logic here
  gpio_write(EXTMODULE_TX_INVERT_GPIO, !enable);
  gpio_write(EXTMODULE_RX_INVERT_GPIO, !enable);
}

static void _extmod_init_inverter()
{
  gpio_init(EXTMODULE_TX_INVERT_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(EXTMODULE_RX_INVERT_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);

  // this sets the output to idle-low as is
  // the default in EdgeTX on external module
  // (historically, most radios work this way)
  _extmod_set_inverted(false);
}
#endif

static const stm32_usart_t extmoduleUSART = {
  .USARTx = EXTMODULE_USART,
  .txGPIO = EXTMODULE_TX_GPIO,
  .rxGPIO = EXTMODULE_RX_GPIO,
  .IRQn = EXTMODULE_USART_IRQn,
  .IRQ_Prio = EXTMODULE_USART_IRQ_PRIORITY,
  .txDMA = EXTMODULE_USART_TX_DMA,
  .txDMA_Stream = EXTMODULE_USART_TX_DMA_STREAM,
  .txDMA_Channel = EXTMODULE_USART_TX_DMA_CHANNEL,
  .rxDMA = EXTMODULE_USART_TX_DMA,
  .rxDMA_Stream = EXTMODULE_USART_RX_DMA_STREAM,
  .rxDMA_Channel = EXTMODULE_USART_RX_DMA_CHANNEL,
  .set_input = nullptr,
  .txDMA_IRQn = (IRQn_Type)0,
  .txDMA_IRQ_Prio = 0,
};

DEFINE_STM32_SERIAL_PORT(ExternalModule, extmoduleUSART, INTMODULE_FIFO_SIZE, 0);

#elif defined(TRAINER_MODULE_SBUS_USART)

static const stm32_usart_t sbus_trainer_USART = {
  .USARTx = TRAINER_MODULE_SBUS_USART,
  .rxGPIO = TRAINER_MODULE_SBUS_GPIO,
  .IRQn = (IRQn_Type)-1,
  .IRQ_Prio = 0,
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
  .rxDMA = TRAINER_MODULE_SBUS_DMA,
  .rxDMA_Stream = TRAINER_MODULE_SBUS_DMA_STREAM_LL,
  .rxDMA_Channel = TRAINER_MODULE_SBUS_DMA_CHANNEL,
};

DEFINE_STM32_SERIAL_PORT(SbusTrainer, sbus_trainer_USART, 32, 0);

#endif

static stm32_pulse_dma_tc_cb_t _ext_timer_DMA_TC_Callback;

static const stm32_pulse_timer_t extmoduleTimer = {
  .GPIO = EXTMODULE_TX_GPIO,
  .GPIO_Alternate = EXTMODULE_TIMER_TX_GPIO_AF,
  .TIMx = EXTMODULE_TIMER,
  .TIM_Freq = EXTMODULE_TIMER_FREQ,
  .TIM_Channel = EXTMODULE_TIMER_Channel,
  .TIM_IRQn = EXTMODULE_TIMER_IRQn,
  .DMAx = EXTMODULE_TIMER_DMA,
  .DMA_Stream = EXTMODULE_TIMER_DMA_STREAM,
  .DMA_Channel = EXTMODULE_TIMER_DMA_CHANNEL,
  .DMA_IRQn = EXTMODULE_TIMER_DMA_STREAM_IRQn,
  .DMA_TC_CallbackPtr = &_ext_timer_DMA_TC_Callback,
};

// Make sure the timer channel is supported
static_assert(__STM32_PULSE_IS_TIMER_CHANNEL_SUPPORTED(EXTMODULE_TIMER_Channel),
              "Unsupported timer channel");

// Make sure the DMA channel is supported
static_assert(__STM32_DMA_IS_STREAM_SUPPORTED(EXTMODULE_TIMER_DMA_STREAM),
              "Unsupported DMA stream");

#if !defined(EXTMODULE_TIMER_DMA_IRQHandler)
#error "Missing EXTMODULE_TIMER_DMA_IRQHandler definition"
#endif

extern "C" void EXTMODULE_TIMER_DMA_IRQHandler()
{
  stm32_pulse_dma_tc_isr(&extmoduleTimer);
}

#if !defined(EXTMODULE_TIMER_IRQHandler)
#error "Missing EXTMODULE_TIMER_IRQHandler definition"
#endif

extern "C" void EXTMODULE_TIMER_IRQHandler()
{
  stm32_pulse_tim_update_isr(&extmoduleTimer);
}

DEFINE_STM32_SOFTSERIAL_PORT(ExternalModule, extmoduleTimer);

#if defined(TRAINER_MODULE_CPPM_TIMER)

static_assert(__IS_TRAINER_TIMER_IN_CHANNEL_SUPPORTED(TRAINER_MODULE_CPPM_TIMER_Channel),
              "Unsupported trainer timer input channel");

static const stm32_pulse_timer_t trainerModuleTimer = {
  .GPIO = TRAINER_MODULE_CPPM_GPIO,
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

#endif // TRAINER_MODULE_CPPM_TIMER

#endif // HARDWARE_EXTERNAL_MODULE

#define TELEMETRY_USART_IRQ_PRIORITY 0
#define TELEMETRY_DMA_IRQ_PRIORITY   0

static void _set_sport_input(uint8_t enable)
{
#if defined(TELEMETRY_DIR_GPIO)
  if (TELEMETRY_SET_INPUT) {
    gpio_write(TELEMETRY_DIR_GPIO, enable);
  } else {
    gpio_write(TELEMETRY_DIR_GPIO, !enable);
  }
#else
  (void)enable;
#endif
}

#if defined(TELEMETRY_TX_REV_GPIO) && defined(TELEMETRY_RX_REV_GPIO)

#define HAS_SPORT_INVERTER

static void _sport_set_inverted(uint8_t enable)
{
  gpio_write(TELEMETRY_TX_REV_GPIO, !enable);
  gpio_write(TELEMETRY_RX_REV_GPIO, !enable);
}

static void _sport_init_inverter()
{
  gpio_init(TELEMETRY_TX_REV_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  gpio_init(TELEMETRY_RX_REV_GPIO, GPIO_OUT, GPIO_PIN_SPEED_LOW);
  _sport_set_inverted(false);
}
#endif

#if defined(TELEMETRY_USART)
static const stm32_usart_t sportUSART = {
  .USARTx = TELEMETRY_USART,
  .txGPIO = TELEMETRY_TX_GPIO,
  .rxGPIO = TELEMETRY_RX_GPIO,
  .IRQn = TELEMETRY_USART_IRQn,
  .IRQ_Prio = TELEMETRY_USART_IRQ_PRIORITY,
  .txDMA = TELEMETRY_DMA,
  .txDMA_Stream = TELEMETRY_DMA_Stream_TX,
  .txDMA_Channel = TELEMETRY_DMA_Channel_TX,
  .rxDMA = nullptr,
  .rxDMA_Stream = 0,
  .rxDMA_Channel = 0,
  .set_input = _set_sport_input,
  .txDMA_IRQn = TELEMETRY_DMA_TX_Stream_IRQ,
  .txDMA_IRQ_Prio = TELEMETRY_DMA_IRQ_PRIORITY,
};

extern "C" void TELEMETRY_DMA_TX_IRQHandler(void)
{
  stm32_usart_tx_dma_isr(&sportUSART);
}

DEFINE_STM32_SERIAL_PORT(SportModule, sportUSART, TELEMETRY_FIFO_SIZE, 0);

#if defined(TELEMETRY_DIR_GPIO)
static void _sport_direction_init()
{
  gpio_init(TELEMETRY_DIR_GPIO, GPIO_OUT, GPIO_PIN_SPEED_MEDIUM);
}
#endif
#endif

#if defined(TELEMETRY_TIMER)
static const stm32_softserial_rx_port sportSoftRX = {
  .GPIO = TELEMETRY_RX_GPIO,
  .TIMx = TELEMETRY_TIMER,
  .TIM_Freq = PERI2_FREQUENCY * TIMER_MULT_APB2,
  .TIM_IRQn = TELEMETRY_TIMER_IRQn,
  .EXTI_Port = TELEMETRY_EXTI_PORT,
  .EXTI_SysLine = TELEMETRY_EXTI_SYS_LINE,
  .EXTI_Line = TELEMETRY_EXTI_LINE,
  .dir_GPIO = TELEMETRY_DIR_GPIO,
  .dir_Input = TELEMETRY_SET_INPUT,
  // re-use S.PORT serial RX buffer
  .buffer = { SportModule_RXBuffer, TELEMETRY_FIFO_SIZE },
};

extern "C" void TELEMETRY_TIMER_IRQHandler()
{
  stm32_softserial_rx_timer_isr(&sportSoftRX);
}
#endif

#if defined(HARDWARE_INTERNAL_MODULE)

static void _internal_module_set_pwr(uint8_t enable)
{
  if (enable) {
    INTERNAL_MODULE_ON();
  } else {
    INTERNAL_MODULE_OFF();
  }
}

#if defined(INTMODULE_BOOTCMD_GPIO)
static void _internal_module_set_bootcmd(uint8_t enable)
{
  // If default state is SET, invert the logic
  if (INTMODULE_BOOTCMD_DEFAULT) {
    enable = !enable;
  }

  gpio_write(INTMODULE_BOOTCMD_GPIO, enable);
}
#endif

static const etx_module_port_t _internal_ports[] = {
#if defined(INTMODULE_USART)
  {
    .port = ETX_MOD_PORT_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX,
    .drv = { .serial = &STM32SerialDriver },
    .hw_def = REF_STM32_SERIAL_PORT(InternalModule),
  },
#else // INTMODULE_USART
  {
    .port = ETX_MOD_PORT_SOFT_INV,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .serial = &STM32SoftSerialTxDriver },
    .hw_def = REF_STM32_SOFTSERIAL_PORT(InternalModule),
  },
#endif
#if defined(INTERNAL_MODULE_PXX1)
  {
    .port = ETX_MOD_PORT_SPORT,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX | ETX_MOD_DIR_RX,
    .drv = { .serial = &STM32SerialDriver },
    .hw_def = REF_STM32_SERIAL_PORT(SportModule),
  },
#endif
};

static const etx_module_t _internal_module = {
  .ports = _internal_ports,
  .set_pwr = _internal_module_set_pwr,
#if defined(INTMODULE_BOOTCMD_GPIO)
  .set_bootcmd = _internal_module_set_bootcmd,
#else
  .set_bootcmd = nullptr,
#endif
  .n_ports = DIM(_internal_ports),
};
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)

static void _external_module_set_pwr(uint8_t enable)
{
  if (enable) {
    EXTERNAL_MODULE_ON();
#if defined(PCBNV14)
    if (hardwareOptions.pcbrev == PCBREV_NV14) {
      gpio_clear(EXTMODULE_PWR_FIX_GPIO);
    }
#endif
  } else {
    EXTERNAL_MODULE_OFF();
#if defined(PCBNV14)
    if (hardwareOptions.pcbrev == PCBREV_NV14) {
      gpio_set(EXTMODULE_PWR_FIX_GPIO);
    }
#endif
  }
}

static const etx_module_port_t _external_ports[] = {
#if defined(EXTMODULE_USART)
  // TX on PPM, RX on HEARTBEAT
  {
    .port = ETX_MOD_PORT_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX,
    .drv = { .serial = &STM32SerialDriver },
    .hw_def = REF_STM32_SERIAL_PORT(ExternalModule),
#if defined(HAS_EXTMODULE_INVERTERS)
    .set_inverted = _extmod_set_inverted,
#else
    .set_inverted = nullptr,
#endif
  },
#endif
  // Timer output on PPM
  {
    .port = ETX_MOD_PORT_TIMER,
    .type = ETX_MOD_TYPE_TIMER,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .timer = &STM32ModuleTimerDriver },
    .hw_def = (void*)&extmoduleTimer,
#if defined(HAS_EXTMODULE_INVERTERS)
    .set_inverted = _extmod_set_inverted,
#else
    .set_inverted = nullptr,
#endif
  },
  // TX inverted DMA pulse train on PPM
  {
    .port = ETX_MOD_PORT_SOFT_INV,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .serial = &STM32SoftSerialTxDriver },
    .hw_def = REF_STM32_SOFTSERIAL_PORT(ExternalModule),
#if defined(HAS_EXTMODULE_INVERTERS)
    .set_inverted = _extmod_set_inverted,
#else
    .set_inverted = nullptr,
#endif
  },
#if !defined(EXTMODULE_USART) && defined(TRAINER_MODULE_SBUS_USART)
  // RX on HEARTBEAT
  {
    .port = ETX_MOD_PORT_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_RX,
    .drv = { .serial = &STM32SerialDriver },
    .hw_def = REF_STM32_SERIAL_PORT(SbusTrainer),
    .set_inverted = nullptr,
  },
#endif
#if defined(TRAINER_MODULE_CPPM_TIMER)
  // Timer input on HEARTBEAT
  {
    .port = ETX_MOD_PORT_TIMER,
    .type = ETX_MOD_TYPE_TIMER,
    .dir_flags = ETX_MOD_DIR_RX,
    .drv = { .timer = nullptr },
    .hw_def = (void*)&trainerModuleTimer,
    .set_inverted = nullptr,
  },
#endif
  // TX/RX half-duplex on S.PORT
  {
    .port = ETX_MOD_PORT_SPORT,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX | ETX_MOD_DIR_RX,
    .drv = { .serial = &STM32SerialDriver },
    .hw_def = REF_STM32_SERIAL_PORT(SportModule),
#if defined(HAS_SPORT_INVERTER)
    .set_inverted = _sport_set_inverted,
#else
    .set_inverted = nullptr,
#endif
  },
#if defined(TELEMETRY_TIMER)
  // RX soft-serial sampled bit-by-bit via timer IRQ on S.PORT
  {
    .port = ETX_MOD_PORT_SPORT_INV,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_RX,
    .drv = { .serial = &STM32SoftSerialRxDriver },
    .hw_def = (void*)&sportSoftRX,
  },
#endif
};

static const etx_module_t _external_module = {
  .ports = _external_ports,
  .set_pwr = _external_module_set_pwr,
  .set_bootcmd = nullptr,
  .n_ports = DIM(_external_ports),
};
#endif

#if defined(SPORT_UPDATE_PWR_GPIO)
#include "sport_update.h"

// Not declared 'static' to be accessible
// from board.cpp
void _sport_set_pwr(uint8_t enabled)
{
  gpio_write(SPORT_UPDATE_PWR_GPIO, enabled);
}

#if defined(RADIO_X7)
// set_pwr is set at runtime once
// the PCB has been identified
etx_module_t _sport_module = {
  .ports = nullptr,
  .set_pwr = nullptr,
  .set_bootcmd = nullptr,
  .n_ports = 0,
};
#else
const etx_module_t _sport_module = {
  .ports = nullptr,
  .set_pwr = _sport_set_pwr,
  .set_bootcmd = nullptr,
  .n_ports = 0,
};
#endif // RADIO_X7

#endif // SPORT_UPDATE_PWR_GPIO

#if !defined(RADIO_X10E)
uint32_t __pxx1_get_inverter_comp() { return 1; }
#endif

void boardInitModulePorts()
{
#if defined(TELEMETRY_USART) && defined(TELEMETRY_DIR_GPIO)
  _sport_direction_init();
#endif

#if defined(SPORT_UPDATE_PWR_GPIO)
  sportUpdateInit();
#endif

#if defined(HAS_SPORT_INVERTER)
  _sport_init_inverter();
#endif
#if defined(HAS_EXTMODULE_INVERTERS)
  _extmod_init_inverter();
#endif  

#if defined(PCBNV14)
  if (hardwareOptions.pcbrev == PCBREV_NV14) {
    // pin must be pulled to V+ (voltage of board - VCC is not enough to fully close transistor)
    // for additional transistor to ensuring module is completely disabled
    gpio_init(EXTMODULE_PWR_FIX_GPIO, GPIO_OD, GPIO_PIN_SPEED_LOW);
    gpio_set(EXTMODULE_PWR_FIX_GPIO);
  }
#endif
}

BEGIN_MODULES()
#if defined(HARDWARE_INTERNAL_MODULE)
  &_internal_module,
#else
  nullptr,
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
  &_external_module,
#else
  nullptr,
#endif
#if defined(SPORT_UPDATE_PWR_GPIO)
  &_sport_module,
#endif
END_MODULES()
