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
#include "intmodule_serial_driver.h"
#include "extmodule_serial_driver.h"

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

#if defined(TRAINER_GPIO)
void init_trainer_ppm() {}
void stop_trainer_ppm() {}
void init_trainer_capture() {}
void stop_trainer_capture() {}
#endif

void init_trainer_module_cppm() {}
void stop_trainer_module_cppm() {}

void init_trainer_module_sbus() {}
void stop_trainer_module_sbus() {}

void init_intmodule_heartbeat() {}
void stop_intmodule_heartbeat() {}

static void* init(const etx_serial_init*) { return nullptr; }
static void deinit(void*) {}
static void sendByte(void*, uint8_t) {}
static void sendBuffer(void*, const uint8_t*, uint8_t) {}
static void waitForTxCompleted(void*) {}
static int getByte(void*,uint8_t*) { return -1; }

const etx_serial_driver_t IntmoduleSerialDriver = {
    .init = init,
    .deinit = deinit,
    .sendByte = sendByte,
    .sendBuffer = sendBuffer,
    .waitForTxCompleted = waitForTxCompleted,
    .getByte = getByte,
    .clearRxBuffer = nullptr,
    .getBaudrate = nullptr,
    .setReceiveCb = nullptr,
    .setBaudrateCb = nullptr,
};

const etx_serial_driver_t ExtmoduleSerialDriver = {
    .init = init,
    .deinit = deinit,
    .sendByte = sendByte,
    .sendBuffer = sendBuffer,
    .waitForTxCompleted = waitForTxCompleted,
    .getByte = getByte,
    .clearRxBuffer = nullptr,
    .getBaudrate = nullptr,
    .setReceiveCb = nullptr,
    .setBaudrateCb = nullptr,
};
