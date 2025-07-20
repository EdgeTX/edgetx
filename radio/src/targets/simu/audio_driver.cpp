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

#include "audio.h"
#include "simuaudio.h"

#if !defined(SOFTWARE_VOLUME)
static int _simu_volume = 0;

void audioSetVolume(uint8_t volume)
{
  _simu_volume = volume;
}
#endif

int simuAudioGetVolume()
{
#if !defined(SOFTWARE_VOLUME)
  return _simu_volume;
#else
  return VOLUME_LEVEL_MAX;
#endif
}

void simuQueueAudio(const uint8_t* data, uint32_t len);

void audioConsumeCurrentBuffer()
{
  auto& fifo = audioQueue.buffersFifo;
  while(true) {
    auto nextBuffer = fifo.getNextFilledBuffer();
    if (!nextBuffer) return;

    auto data = (const uint8_t*)nextBuffer->data;
    uint32_t len = nextBuffer->size * sizeof(audio_data_t);
    simuQueueAudio(data, len);
    fifo.freeNextFilledBuffer();
  }
}
