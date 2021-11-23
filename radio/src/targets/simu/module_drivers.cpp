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

#include "hal/serial_driver.h"
#include "intmodule_serial_driver.h"
#include "extmodule_serial_driver.h"

#include "io/frsky_pxx2.h"
ModuleFifo intmoduleFifo;
ModuleFifo extmoduleFifo;

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
void extmodulePxx1SerialStart() {}
void extmodulePxx1PulsesStart() {}
void extmoduleSendInvertedByte(uint8_t) {}
void extmoduleSendNextFramePxx1(void const*, unsigned short) {}
void extmoduleSendNextFrameAFHDS3(void const*, unsigned short) {}
void extmoduleSendNextFrameSoftSerial100kbit(void const*, unsigned short, bool) {}
void extmoduleSendNextFramePpm(void*, unsigned short, unsigned short, bool) {}

void init_trainer_ppm() {}
void stop_trainer_ppm() {}

void init_trainer_capture() {}
void stop_trainer_capture() {}

void init_trainer_module_cppm() {}
void stop_trainer_module_cppm() {}

void init_trainer_module_sbus() {}
void stop_trainer_module_sbus() {}

void init_intmodule_heartbeat() {}
void stop_intmodule_heartbeat() {}

static void init(const etx_serial_init*) {}
static void deinit() {}
static void sendByte(uint8_t) {}
static void sendBuffer(const uint8_t*, uint8_t) {}
static void waitForTxCompleted() {}

const etx_serial_driver_t IntmoduleSerialDriver = {
    .init = init,
    .deinit = deinit,
    .sendByte = sendByte,
    .sendBuffer = sendBuffer,
    .waitForTxCompleted = waitForTxCompleted,
    .setReceiveCb = nullptr,
    .setOnErrorCb = nullptr,
};

const etx_serial_driver_t ExtmoduleSerialDriver = {
    .init = init,
    .deinit = deinit,
    .sendByte = sendByte,
    .sendBuffer = sendBuffer,
    .waitForTxCompleted = waitForTxCompleted,
    .setReceiveCb = nullptr,
    .setOnErrorCb = nullptr,
};
