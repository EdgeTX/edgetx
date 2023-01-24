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

#include "hal.h"
#include "hal/serial_driver.h"
#include "hal/module_port.h"

void intmoduleStop() {}
void intmoduleFifoError() {}
void intmoduleFifoReceive(uint8_t){}

void intmodulePxx1PulsesStart() {}
void intmoduleSendNextFramePxx1(unsigned short const*, unsigned char) {}

void extmoduleStop() {}
void extmoduleFifoError() {}
void extmoduleFifoReceive(uint8_t){}

void extmodulePpmStart(unsigned short, bool) {}
void extmoduleSerialStart() {}
void extmodulePxx1PulsesStart() {}
void extmoduleInitTxPin() {}
void extmoduleSendInvertedByte(uint8_t) {}
void extmoduleSendNextFramePxx1(void const*, unsigned short) {}
void extmoduleSendNextFrameSoftSerial(void const*, unsigned short, bool) {}
void extmoduleSendNextFramePpm(void*, unsigned short, unsigned short, bool) {}

void init_trainer_ppm() {}
void stop_trainer_ppm() {}
void init_trainer_capture() {}
void stop_trainer_capture() {}

bool is_trainer_connected() { return false; }

void init_trainer_module_cppm() {}
void stop_trainer_module_cppm() {}

void init_trainer_module_sbus() {}
void stop_trainer_module_sbus() {}

void init_intmodule_heartbeat() {}
void stop_intmodule_heartbeat() {}

#if defined(INTMODULE_USART) ||  defined(EXTMODULE_USART)
static void* init(void*, const etx_serial_init*) { return (void*)1; }
static void deinit(void*) {}
static void sendByte(void*, uint8_t) {}
static void sendBuffer(void*, const uint8_t*, uint8_t) {}
static void waitForTxCompleted(void*) {}
static int getByte(void*,uint8_t*) { return -1; }

const etx_serial_driver_t _fakeSerialDriver = {
    .init = init,
    .deinit = deinit,
    .sendByte = sendByte,
    .sendBuffer = sendBuffer,
    .waitForTxCompleted = waitForTxCompleted,
    .enableRx = nullptr,
    .getByte = getByte,
    .clearRxBuffer = nullptr,
    .getBaudrate = nullptr,
    .setReceiveCb = nullptr,
    .setBaudrateCb = nullptr,
};
#endif

static void* module_timer_init(void* hw_def, const etx_timer_config_t* cfg)
{ return (void*)1; }

static void module_timer_deinit(void* ctx) {}

static void module_timer_send(void* ctx, const etx_timer_config_t* cfg,
                              const void* pulses, uint16_t length)
{}

const etx_timer_driver_t _fakeTimerDriver = {
  .init = module_timer_init,
  .deinit = module_timer_deinit,
  .send = module_timer_send,
};

BEGIN_MODULE_PORTS()
#if defined(INTMODULE_USART)
  {
    .port = ETX_MOD_PORT_INTERNAL_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = nullptr,
  },
#else // INTMODULE_USART
  {
    .port = ETX_MOD_PORT_INTERNAL_TIMER,
    .type = ETX_MOD_TYPE_TIMER,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .timer = &_fakeTimerDriver },
    .hw_def = nullptr,
  },
#endif
  {
    .port = ETX_MOD_PORT_EXTERNAL_TIMER,
    .type = ETX_MOD_TYPE_TIMER,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .timer = &_fakeTimerDriver },
    .hw_def = nullptr,
  },
#if defined(EXTMODULE_USART)
  {
    .port = ETX_MOD_PORT_EXTERNAL_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = nullptr,
  },
#endif
END_MODULE_PORTS()
