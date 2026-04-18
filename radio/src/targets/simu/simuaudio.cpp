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

#include <stdint.h>
#include <SDL.h>

#define AUDIO_FMT AUDIO_S16SYS

static SDL_AudioDeviceID _sdl_audio_device = 0;

// Companion-side volume gain, stored as user-facing gain (0.5..3.0x) * 10.
// Default 10 = unity.
static int _volume_gain = 10;

void simuSetVolumeGain(int value)
{
  _volume_gain = value;
}

void simuQueueAudio(const uint8_t* data, uint32_t len)
{
#if !defined(SOFTWARE_VOLUME)
  int volume = (simuAudioGetVolume() * SDL_MIX_MAXVOLUME ) / VOLUME_LEVEL_MAX;
  SDL_MixAudioFormat((uint8_t*)data, data, AUDIO_FMT, len, volume);
#endif

  int gain = _volume_gain;
  if (gain != 10) {
    // SDL_MixAudioFormat cannot amplify above unity, so apply the
    // Companion gain manually with saturation to support the full
    // 0.5..3.0x UI range.
    auto* samples = (int16_t*)data;
    uint32_t n = len / sizeof(int16_t);
    for (uint32_t i = 0; i < n; ++i) {
      int32_t s = ((int32_t)samples[i] * gain) / 10;
      if (s > INT16_MAX) s = INT16_MAX;
      else if (s < INT16_MIN) s = INT16_MIN;
      samples[i] = (int16_t)s;
    }
  }

  SDL_QueueAudio(_sdl_audio_device, data, len);
}

bool simuAudioInit()
{
  SDL_AudioSpec wanted = {
    .freq = AUDIO_SAMPLE_RATE,
    .format = AUDIO_FMT,
    .channels = 1,
    .silence = 0,
    .samples = 1024,
  };

  SDL_AudioSpec have;
  _sdl_audio_device = SDL_OpenAudioDevice(0, 0, &wanted, &have, 0);
  if (!_sdl_audio_device) {
    SDL_Log("Couldn't open audio: %s\n", SDL_GetError());
    return false;
  }

  SDL_PauseAudioDevice(_sdl_audio_device, 0);
  return true;
}

void simuAudioDeInit()
{
  if(_sdl_audio_device > 0) {
    SDL_CloseAudioDevice(_sdl_audio_device);
    _sdl_audio_device = 0;
  }
}
