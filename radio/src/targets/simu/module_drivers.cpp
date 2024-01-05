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
#include "dataconstants.h"
#include "debug.h"

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

bool trainer_dsc_available() { return true; }
void trainer_init_dsc_out() {}
void trainer_init_dsc_in() {}
void trainer_stop_dsc() {}

bool is_trainer_dsc_connected() { return false; }

void trainer_init_module_cppm() {}
void trainer_stop_module_cppm() {}

void init_intmodule_heartbeat() {}
void stop_intmodule_heartbeat() {}

static bool _sport_used = false;

static void* init(void* ctx, const etx_serial_init*)
{
  if (ctx == nullptr) {
    return (void*)1;
  }

  if (ctx == (void*)&_sport_used) {
    if (_sport_used) {
      return nullptr;
    }

    _sport_used = true;
    return &_sport_used;
  }

  return nullptr;
}

static void deinit(void* ctx)
{
  if (ctx != (void*)&_sport_used) return;
  _sport_used = false;
}

static void sendByte(void*, uint8_t) {}
static void sendBuffer(void*, const uint8_t*, uint32_t) {}
static void waitForTxCompleted(void*) {}
static int getByte(void*,uint8_t*) { return -1; }

const etx_serial_driver_t _fakeSerialDriver = {
    .init = init,
    .deinit = deinit,
    .sendByte = sendByte,
    .sendBuffer = sendBuffer,
    .txCompleted = nullptr,
    .waitForTxCompleted = waitForTxCompleted,
    .enableRx = nullptr,
    .getByte = getByte,
    .getLastByte = nullptr,
    .getBufferedBytes = nullptr,
    .copyRxBuffer = nullptr,
    .clearRxBuffer = nullptr,
    .getBaudrate = nullptr,
    .setBaudrate = nullptr,
    .setPolarity = nullptr,
    .setHWOption = nullptr,
    .setReceiveCb = nullptr,
    .setIdleCb = nullptr,
    .setBaudrateCb = nullptr,
};

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

#if defined(HARDWARE_INTERNAL_MODULE)
const etx_module_port_t _internal_ports[] = {
#if defined(INTMODULE_USART)
  {
    .port = ETX_MOD_PORT_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = nullptr,
  },
#else // INTMODULE_USART
  {
    .port = ETX_MOD_PORT_SOFT_INV,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = nullptr,
  },
#endif
#if defined(INTERNAL_MODULE_PXX1)
  {
    .port = ETX_MOD_PORT_SPORT,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX | ETX_MOD_DIR_RX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = &_sport_used,
  },
#endif
};

static const etx_module_t _internal_module = {
  .ports = _internal_ports,
  .set_pwr = nullptr,
  .set_bootcmd = nullptr,
  .n_ports = DIM(_internal_ports),
};
#endif

#if defined(SPORT_UPDATE_PWR_GPIO)
void _sport_set_pwr(uint8_t) {}
const etx_module_t _sport_module = {
  .ports = nullptr,
  .set_pwr = _sport_set_pwr,
  .set_bootcmd = nullptr,
  .n_ports = 0,
};
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
const etx_module_port_t _external_ports[] = {
#if defined(EXTMODULE_USART)
  // TX on PPM, RX on HEARTBEAT
  {
    .port = ETX_MOD_PORT_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX_RX | ETX_MOD_FULL_DUPLEX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = nullptr,
  },
#elif defined(TRAINER_MODULE_SBUS_USART)
  // RX on HEARTBEAT
  {
    .port = ETX_MOD_PORT_UART,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_RX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = nullptr,
  },  
#endif
  // Timer output on PPM
  {
    .port = ETX_MOD_PORT_TIMER,
    .type = ETX_MOD_TYPE_TIMER,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .timer = &_fakeTimerDriver },
    .hw_def = nullptr,
  },
  // TX inverted DMA pulse train on PPM
  {
    .port = ETX_MOD_PORT_SOFT_INV,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = nullptr,
  },
#if defined(TRAINER_MODULE_CPPM_TIMER)
  // Timer input on HEARTBEAT
  {
    .port = ETX_MOD_PORT_TIMER,
    .type = ETX_MOD_TYPE_TIMER,
    .dir_flags = ETX_MOD_DIR_RX,
    .drv = { .timer = nullptr },
    .hw_def = nullptr,
  },
#endif
  // TX/RX half-duplex on S.PORT
  {
    .port = ETX_MOD_PORT_SPORT,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_TX | ETX_MOD_DIR_RX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = &_sport_used,
  },
#if defined(TELEMETRY_TIMER)
  // RX soft-serial sampled bit-by-bit via timer IRQ on S.PORT
  {
    .port = ETX_MOD_PORT_SPORT_INV,
    .type = ETX_MOD_TYPE_SERIAL,
    .dir_flags = ETX_MOD_DIR_RX,
    .drv = { .serial = &_fakeSerialDriver },
    .hw_def = nullptr,
  },
#endif
};

static const etx_module_t _external_module = {
  .ports = _external_ports,
  .set_pwr = nullptr,
  .set_bootcmd = nullptr,
  .n_ports = DIM(_external_ports),
};
#endif


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
