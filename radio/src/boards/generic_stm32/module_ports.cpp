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

#include "hal/module_port.h"
#include "stm32_serial_driver.h"
#include "stm32_softserial_driver.h"

#include "board.h"
#include "dataconstants.h"


#define AUX_SERIAL_TX_BUFFER 512
#if defined(SDRAM)
  #define AUX_SERIAL_RX_BUFFER 512
#else
  #define AUX_SERIAL_RX_BUFFER 32
#endif

#if defined(AUX_SERIAL_PWR_GPIO) || defined(AUX2_SERIAL_PWR_GPIO)
static void _aux_pwr(GPIO_TypeDef *GPIOx, uint32_t pin, uint8_t on)
{
  LL_GPIO_InitTypeDef pinInit;
  LL_GPIO_StructInit(&pinInit);
  pinInit.Pin = pin;
  pinInit.Mode = LL_GPIO_MODE_OUTPUT;
  pinInit.Pull = LL_GPIO_PULL_UP;
  LL_GPIO_Init(GPIOx, &pinInit);

  if (on) {
    LL_GPIO_SetOutputPin(GPIOx, pin);
  } else {
    LL_GPIO_ResetOutputPin(GPIOx, pin);
  }
}
#endif

#if defined(AUX_SERIAL)

static const LL_GPIO_InitTypeDef auxUSARTPinInit = {
  .Pin = AUX_SERIAL_GPIO_PIN_TX | AUX_SERIAL_GPIO_PIN_RX,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = AUX_SERIAL_GPIO_AF_LL,
};

#if !defined(AUX_SERIAL_DMA_TX)
  #define AUX_SERIAL_DMA_TX                   nullptr
  #define AUX_SERIAL_DMA_Stream_TX            nullptr
  #define AUX_SERIAL_DMA_Stream_TX_LL         0
  #define AUX_SERIAL_DMA_Channel_TX           0
#endif

static const stm32_usart_t auxUSART = {
  .USARTx = AUX_SERIAL_USART,
  .GPIOx = AUX_SERIAL_GPIO,
  .pinInit = &auxUSARTPinInit,
  .IRQn = AUX_SERIAL_USART_IRQn,
  .IRQ_Prio = 7, // TODO: define constant
  .txDMA = AUX_SERIAL_DMA_TX,
  .txDMA_Stream = AUX_SERIAL_DMA_Stream_TX_LL,
  .txDMA_Channel = AUX_SERIAL_DMA_Channel_TX,
  .rxDMA = AUX_SERIAL_DMA_RX,
  .rxDMA_Stream = AUX_SERIAL_DMA_Stream_RX_LL,
  .rxDMA_Channel = AUX_SERIAL_DMA_Channel_RX,
};

DEFINE_STM32_SERIAL_PORT(Aux, auxUSART, AUX_SERIAL_RX_BUFFER, AUX_SERIAL_TX_BUFFER);

#if defined(AUX_SERIAL_PWR_GPIO)
void set_aux_pwr(uint8_t on)
{
  _aux_pwr(AUX_SERIAL_PWR_GPIO, AUX_SERIAL_PWR_GPIO_PIN, on);
}
#define AUX_PWR_FCT set_aux_pwr
#else
#define AUX_PWR_FCT nullptr
#endif

const etx_serial_port_t auxSerialPort = {
  .name = "AUX1",
  .uart = &STM32SerialDriver,
  .hw_def = REF_STM32_SERIAL_PORT(Aux),
  AUX_PWR_FCT
};
#define AUX_SERIAL_PORT &auxSerialPort
#else
#define AUX_SERIAL_PORT nullptr
#endif

#if defined(AUX2_SERIAL)

static const LL_GPIO_InitTypeDef aux2USARTPinInit = {
  .Pin = AUX2_SERIAL_GPIO_PIN_TX | AUX2_SERIAL_GPIO_PIN_RX,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = AUX2_SERIAL_GPIO_AF_LL,
};

static const stm32_usart_t aux2USART = {
  .USARTx = AUX2_SERIAL_USART,
  .GPIOx = AUX2_SERIAL_GPIO,
  .pinInit = &aux2USARTPinInit,
  .IRQn = AUX2_SERIAL_USART_IRQn,
  .IRQ_Prio = 7, // TODO: define constant
  .txDMA = nullptr,
  .txDMA_Stream = 0,
  .txDMA_Channel = 0,
  .rxDMA = AUX2_SERIAL_DMA_RX,
  .rxDMA_Stream = AUX2_SERIAL_DMA_Stream_RX_LL,
  .rxDMA_Channel = AUX2_SERIAL_DMA_Channel_RX,
};

DEFINE_STM32_SERIAL_PORT(Aux2, aux2USART, AUX_SERIAL_RX_BUFFER, AUX_SERIAL_TX_BUFFER);

#if defined(AUX2_SERIAL_PWR_GPIO)
void set_aux2_pwr(uint8_t on)
{
  _aux_pwr(AUX2_SERIAL_PWR_GPIO, AUX2_SERIAL_PWR_GPIO_PIN, on);
}
#define AUX2_PWR_FCT set_aux2_pwr
#else
#define AUX2_PWR_FCT nullptr
#endif

const etx_serial_port_t aux2SerialPort = {
  .name = "AUX2",
  .uart = &STM32SerialDriver,
  .hw_def = REF_STM32_SERIAL_PORT(Aux2),
  AUX2_PWR_FCT
};
#define AUX2_SERIAL_PORT &aux2SerialPort
#else
#define AUX2_SERIAL_PORT nullptr
#endif

static const etx_serial_port_t* serialPorts[MAX_AUX_SERIAL] = {
  AUX_SERIAL_PORT,
  AUX2_SERIAL_PORT,
};

const etx_serial_port_t* auxSerialGetPort(int port_nr)
{
  if (port_nr >= MAX_AUX_SERIAL) return nullptr;
  return serialPorts[port_nr];
}


#if defined(INTMODULE_USART)

#define INTMODULE_USART_IRQ_PRIORITY 6

static const LL_GPIO_InitTypeDef intmoduleUSART_PinDef = {
  .Pin = INTMODULE_TX_GPIO_PIN | INTMODULE_RX_GPIO_PIN,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = INTMODULE_GPIO_AF,
};

static const stm32_usart_t intmoduleUSART = {
  .USARTx = INTMODULE_USART,
  .GPIOx = INTMODULE_GPIO,
  .pinInit = &intmoduleUSART_PinDef,
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

extern const stm32_pulse_timer_t intmoduleTimer;

#endif

#include "module_timer_driver.h"
#include "extmodule_driver.h"

#if defined(EXTMODULE_USART)

#define EXTMODULE_USART_IRQ_PRIORITY 6

static const LL_GPIO_InitTypeDef extmoduleUSART_PinDef = {
  .Pin = EXTMODULE_TX_GPIO_PIN | EXTMODULE_RX_GPIO_PIN,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = INTMODULE_GPIO_AF,
};

static const stm32_usart_t extmoduleUSART = {
  .USARTx = EXTMODULE_USART,
  .GPIOx = EXTMODULE_TX_GPIO,
  .pinInit = &extmoduleUSART_PinDef,
  .IRQn = EXTMODULE_USART_IRQn,
  .IRQ_Prio = EXTMODULE_USART_IRQ_PRIORITY,
  .txDMA = EXTMODULE_USART_TX_DMA,
  .txDMA_Stream = EXTMODULE_USART_TX_DMA_STREAM_LL,
  .txDMA_Channel = EXTMODULE_USART_TX_DMA_CHANNEL,
  .rxDMA = EXTMODULE_USART_TX_DMA,
  .rxDMA_Stream = EXTMODULE_USART_RX_DMA_STREAM_LL,
  .rxDMA_Channel = EXTMODULE_USART_RX_DMA_CHANNEL,
};

DEFINE_STM32_SERIAL_PORT(ExternalModule, extmoduleUSART, INTMODULE_FIFO_SIZE, 0);

#endif

DEFINE_STM32_SOFTSERIAL_PORT(ExternalModule, extmoduleTimer);

#define TELEMETRY_USART_IRQ_PRIORITY 6
#define TELEMETRY_DMA_IRQ_PRIORITY   7

static const LL_GPIO_InitTypeDef sportUSART_PinDef = {
  .Pin = TELEMETRY_TX_GPIO_PIN | TELEMETRY_RX_GPIO_PIN,
  .Mode = LL_GPIO_MODE_ALTERNATE,
  .Speed = LL_GPIO_SPEED_FREQ_LOW,
  .OutputType = LL_GPIO_OUTPUT_PUSHPULL,
  .Pull = LL_GPIO_PULL_UP,
  .Alternate = TELEMETRY_GPIO_AF,
};

static const stm32_usart_t sportUSART = {
  .USARTx = TELEMETRY_USART,
  .GPIOx = TELEMETRY_GPIO,
  .pinInit = &sportUSART_PinDef,
  .IRQn = TELEMETRY_USART_IRQn,
  .IRQ_Prio = TELEMETRY_USART_IRQ_PRIORITY,
  .txDMA = TELEMETRY_DMA,
  .txDMA_Stream = TELEMETRY_DMA_Stream_TX,
  .txDMA_Channel = TELEMETRY_DMA_Channel_TX,
  .rxDMA = nullptr,
  .rxDMA_Stream = 0,
  .rxDMA_Channel = 0,
  .dir_GPIOx = TELEMETRY_DIR_GPIO,
  .dir_Pin = TELEMETRY_DIR_GPIO_PIN,
  .dir_Input = TELEMETRY_SET_INPUT,
  .txDMA_IRQn = TELEMETRY_DMA_TX_Stream_IRQ,
  .txDMA_IRQ_Prio = TELEMETRY_DMA_IRQ_PRIORITY,
};

extern "C" void TELEMETRY_DMA_TX_IRQHandler(void)
{
  stm32_usart_tx_dma_isr(&sportUSART);
}

DEFINE_STM32_SERIAL_PORT(SportModule, sportUSART, TELEMETRY_FIFO_SIZE, 0);

// static const stm32_softserial_port sportSoftRX = {
//   .GPIOx = TELEMETRY_GPIO,
//   .GPIO_Pin = TELEMETRY_RX_GPIO_PIN,
//   .TIMx = TELEMETRY_TIMER,
//   .TIM_Freq = PERI2_FREQUENCY * TIMER_MULT_APB2,
//   .TIM_IRQn = TELEMETRY_TIMER_IRQn,
//   .EXTI_Port = TELEMETRY_EXTI_PORT,
//   .EXTI_SysLine = TELEMETRY_EXTI_SYS_LINE,
//   .EXTI_Line = TELEMETRY_EXTI_LINE,
//   // re-use S.PORT serial RX buffer
//   .buffer = { SportModule_RXBuffer, TELEMETRY_FIFO_SIZE },
// };

// extern "C" void TELEMETRY_TIMER_IRQHandler()
// {
//   stm32_softserial_timer_isr(&sportSoftRX);
// }

// static const stm32_softserial_port ppmSoftTX = {
//   .GPIOx = EXTMODULE_TX_GPIO,
//   .GPIO_Pin = EXTMODULE_TX_GPIO_PIN,
//   .TIMx = nullptr,
//   .TIM_Freq = 0,
//   .TIM_IRQn = (IRQn_Type)0,
//   .EXTI_Port = 0,
//   .EXTI_SysLine = 0,
//   .EXTI_Line = 0,
//   .buffer = { nullptr, 0 },
// };

BEGIN_MODULE_PORTS()
#if defined(INTMODULE_USART)
  {
    .port = ETX_MOD_PORT_INTERNAL_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX,
    .drv = { .serial = &STM32SerialDriver },
    .hw_def = REF_STM32_SERIAL_PORT(InternalModule),
  },
#else // INTMODULE_USART
  {
    .port = ETX_MOD_PORT_INTERNAL_TIMER,
    .type = ETX_MOD_TYPE_TIMER,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .timer = &STM32ModuleTimerDriver },
    .hw_def = (void*)&intmoduleTimer,
  },
#endif
  {
    .port = ETX_MOD_PORT_EXTERNAL_TIMER,
    .type = ETX_MOD_TYPE_TIMER,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .timer = &STM32ModuleTimerDriver },
    .hw_def = (void*)&extmoduleTimer,
  },
#if defined(EXTMODULE_USART)
  {
    .port = ETX_MOD_PORT_EXTERNAL_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX,
    .drv = { .serial = &STM32SerialDriver },
    .hw_def = REF_STM32_SERIAL_PORT(ExternalModule),
  },
#endif
  {
    .port = ETX_MOD_PORT_SPORT,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX | ETX_MOD_DIR_RX,
    .drv = { .serial = &STM32SerialDriver },
    .hw_def = REF_STM32_SERIAL_PORT(SportModule),
  },
  // {
  //   .port = ETX_MOD_PORT_SPORT_INV,
  //   .type = ETX_MOD_TYPE_SERIAL,
  //   .dir_flags = ETX_MOD_DIR_RX,
  //   .drv = { .serial = &STM32SoftSerialDriver },
  //   .hw_def = (void*)&sportSoftRX,
  // },
  {
    .port = ETX_MOD_PORT_EXTERNAL_SOFT_INV,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .serial = &STM32SoftSerialTxDriver },
    // .hw_def = (void*)&ppmSoftTX,
    .hw_def = REF_STM32_SOFTSERIAL_PORT(ExternalModule),
  },
END_MODULE_PORTS()
