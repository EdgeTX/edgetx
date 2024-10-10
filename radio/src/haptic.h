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

#pragma once

#define HAPTIC_QUEUE_LENGTH  4

class hapticQueue
{
  public:

    hapticQueue();

    // only difference between these two functions is that one does the
    // interupt queue (Now) and the other queues for playing ASAP.
    void play(uint8_t tLen, uint8_t tPause, uint8_t tRepeat=0);

    inline bool busy() { return (buzzTimeLeft > 0); }

    void event(uint8_t e);

    // heartbeat is responsibile for issueing the haptic buzzs and general square waves
    // it is essentially the life of the class.
    void heartbeat();

    // bool freeslots(uint8_t slots);

    inline bool empty() {
      return (t_queueRidx == t_queueWidx);
    }

  protected:
    inline uint8_t getHapticLength(uint8_t tLen) {
      return ((g_eeGeneral.hapticLength * 2) + tLen) * 2;
    }

  private:
    uint8_t t_queueRidx;
    uint8_t t_queueWidx;

    uint8_t buzzTimeLeft;
    uint8_t buzzPause;

    uint8_t hapticTick;

    // queue arrays
    uint8_t queueHapticLength[HAPTIC_QUEUE_LENGTH];
    uint8_t queueHapticPause[HAPTIC_QUEUE_LENGTH];
    uint8_t queueHapticRepeat[HAPTIC_QUEUE_LENGTH];
};

extern hapticQueue haptic;

#define IS_HAPTIC_BUSY()     haptic.busy()
#define HAPTIC_HEARTBEAT()   haptic.heartbeat()
