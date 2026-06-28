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

#include "edgetx.h"

#if defined(PDM_CLOCK)

#include <stdint.h>
#include "ff.h"
#include "os/task.h"

// 16 kHz: PDM_CLOCK_FREQ(1.6 MHz)/PDM_PCM_DECIMATION(100) = 16000 Hz directly.
// Must also divide AUDIO_SAMPLE_RATE(32 kHz) evenly for the WAV player.
class PdmWavRecorder
{
 public:
  static constexpr uint32_t DST_RATE = 16000;

  // expectedSeconds == 0 means open-ended; header is patched on stop().
  // At most one recorder active at a time; concurrent start() returns FR_LOCKED.
  FRESULT start(const char* path, uint32_t expectedSeconds);
  FRESULT stop();

  // Called from the audio task every ~4 ms.
  static void audioTick();

  // Trim leading/trailing silence in-place and patch the WAV header.
  static FRESULT trimSilence(const char* path);

  bool isRecording() const { return recording; }
  uint32_t getSamplesWritten() const { return samplesWritten; }
  uint32_t getBytesWritten() const { return samplesWritten * 2U; }
  uint32_t getElapsedSeconds() const { return samplesWritten / DST_RATE; }

 private:
  static constexpr uint32_t PCM_MAX = 256;

  // Guards the global "which recorder is active" selection (s_active) and its
  // file against concurrent access from the audio task and the owner thread.
  // Shared (static), not per-instance: audioTick() must take the lock before it
  // can even safely read s_active to find the active instance.
  static PdmWavRecorder* s_active;
  static mutex_handle_t s_mutex;
  static bool s_mutexInited;
  static void ensureMutex();

  FRESULT startLocked(const char* path, uint32_t expectedSeconds);
  bool tickLocked();

  FIL file;
  int16_t pcm[PCM_MAX];
  // 32-bit aligned reads on Cortex-M7 are atomic — safe to read without mutex.
  volatile uint32_t samplesWritten = 0;
  uint32_t maxSamples = 0;  // 0 = open-ended
  volatile bool recording = false;
};

#endif  // PDM_CLOCK
