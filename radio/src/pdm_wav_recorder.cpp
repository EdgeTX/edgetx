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

#include "pdm_wav_recorder.h"

#if defined(PDM_CLOCK)

#include <string.h>

#include "os/task.h"

// Guards against concurrent access from the audio task and the owner thread.
static PdmWavRecorder* s_active = nullptr;
static mutex_handle_t s_mutex;
static bool s_mutexInited = false;

static void ensureMutex()
{
  if (!s_mutexInited) {
    mutex_create(&s_mutex);
    s_mutexInited = true;
  }
}

static void writeLE16(uint8_t* p, uint16_t v)
{
  p[0] = v & 0xFF;
  p[1] = (v >> 8) & 0xFF;
}

static void writeLE32(uint8_t* p, uint32_t v)
{
  p[0] = v & 0xFF;
  p[1] = (v >> 8) & 0xFF;
  p[2] = (v >> 16) & 0xFF;
  p[3] = (v >> 24) & 0xFF;
}

FRESULT PdmWavRecorder::start(const char* path, uint32_t expectedSeconds)
{
  ensureMutex();
  MutexLock lock = MutexLock::MakeInstance(&s_mutex);

  if (s_active != nullptr) return FR_LOCKED;

  samplesWritten = 0;
  maxSamples = expectedSeconds * DST_RATE;
  recording = false;

  FRESULT res = f_open(&file, path, FA_CREATE_ALWAYS | FA_WRITE);
  if (res != FR_OK) return res;

  const uint32_t dataBytes = maxSamples * 2U;
  uint8_t header[44];
  memcpy(&header[0],  "RIFF", 4);
  writeLE32(&header[4],  36U + dataBytes);
  memcpy(&header[8],  "WAVE", 4);
  memcpy(&header[12], "fmt ", 4);
  writeLE32(&header[16], 16);
  writeLE16(&header[20], 1);              // PCM
  writeLE16(&header[22], 1);              // mono
  writeLE32(&header[24], DST_RATE);
  writeLE32(&header[28], DST_RATE * 2U);  // byte rate
  writeLE16(&header[32], 2);              // block align
  writeLE16(&header[34], 16);             // bits per sample
  memcpy(&header[36], "data", 4);
  writeLE32(&header[40], dataBytes);

  UINT written = 0;
  res = f_write(&file, header, sizeof(header), &written);
  if (res != FR_OK || written != sizeof(header)) {
    f_close(&file);
    return res != FR_OK ? res : FR_DISK_ERR;
  }

  recording = true;
  s_active = this;
  return FR_OK;
}

bool PdmWavRecorder::tickLocked()
{
  if (!recording) return false;
  if (!pdmCapture()) return false;

  uint32_t n = pdmConvertToPCM(pcm, PCM_MAX);
  if (n == 0) return false;

  if (maxSamples != 0) {
    const uint32_t remaining = maxSamples - samplesWritten;
    if (n > remaining) n = remaining;
  }

  UINT w = 0;
  FRESULT res = f_write(&file, pcm, n * sizeof(int16_t), &w);
  if (res != FR_OK || w != n * sizeof(int16_t)) {
    recording = false;
    return false;
  }
  samplesWritten += n;
  if (maxSamples != 0 && samplesWritten >= maxSamples) {
    recording = false;
  }
  return true;
}

void PdmWavRecorder::audioTick()
{
  if (s_active == nullptr) return;
  ensureMutex();
  MutexLock lock = MutexLock::MakeInstance(&s_mutex);
  if (s_active != nullptr) s_active->tickLocked();
}

FRESULT PdmWavRecorder::stop()
{
  ensureMutex();
  MutexLock lock = MutexLock::MakeInstance(&s_mutex);

  if (s_active != this) return FR_OK;

  s_active = nullptr;
  recording = false;

  const uint32_t dataBytes = samplesWritten * 2U;
  uint8_t buf[4];
  UINT written = 0;

  writeLE32(buf, 36U + dataBytes);
  f_lseek(&file, 4);
  f_write(&file, buf, 4, &written);

  writeLE32(buf, dataBytes);
  f_lseek(&file, 40);
  f_write(&file, buf, 4, &written);

  return f_close(&file);
}

FRESULT PdmWavRecorder::trimSilence(const char* path)
{
  // Threshold tracks the post-CIC gain so trim stays calibrated when
  // PDM_POST_GAIN_SHIFT is changed. Baseline 4000 was tuned at shift=4.
  static constexpr int32_t _silenceRaw =
      ((int32_t)4000 * (1 << PDM_POST_GAIN_SHIFT)) / 16;
  static constexpr int16_t SILENCE_THRESHOLD =
      _silenceRaw > 32767 ? (int16_t)32767 : (int16_t)_silenceRaw;
  static constexpr uint32_t PAD_SAMPLES = DST_RATE / 100;  // 10 ms
  static constexpr uint32_t CHUNK = 256;

  FIL f;
  FRESULT res = f_open(&f, path, FA_READ | FA_WRITE);
  if (res != FR_OK) return res;

  uint8_t hdr[44];
  UINT br = 0;
  res = f_read(&f, hdr, sizeof(hdr), &br);
  if (res != FR_OK || br != sizeof(hdr)) { f_close(&f); return res; }

  const uint32_t dataBytes =
      (uint32_t)hdr[40] | ((uint32_t)hdr[41] << 8) |
      ((uint32_t)hdr[42] << 16) | ((uint32_t)hdr[43] << 24);
  const uint32_t total = dataBytes / 2;
  if (total == 0) { f_close(&f); return FR_OK; }

  int16_t buf[CHUNK];
  uint32_t firstActive = UINT32_MAX;
  uint32_t lastActive = 0;

  for (uint32_t base = 0; base < total; base += CHUNK) {
    const uint32_t n = (base + CHUNK <= total) ? CHUNK : total - base;
    f_lseek(&f, 44 + base * 2);
    res = f_read(&f, buf, n * 2, &br);
    if (res != FR_OK) { f_close(&f); return res; }
    const uint32_t got = br / 2;
    for (uint32_t i = 0; i < got; i++) {
      const int16_t abs = buf[i] < 0 ? (int16_t)-buf[i] : buf[i];
      if (abs > SILENCE_THRESHOLD) {
        if (firstActive == UINT32_MAX) firstActive = base + i;
        lastActive = base + i;
      }
    }
  }

  if (firstActive == UINT32_MAX) { f_close(&f); return FR_OK; }  // all silent

  const uint32_t trimStart = firstActive > PAD_SAMPLES ? firstActive - PAD_SAMPLES : 0;
  const uint32_t trimEnd   = (lastActive + PAD_SAMPLES < total) ? lastActive + PAD_SAMPLES : total - 1;
  const uint32_t newTotal  = trimEnd - trimStart + 1;

  if (trimStart == 0 && trimEnd == total - 1) { f_close(&f); return FR_OK; }

  if (trimStart > 0) {
    uint32_t src = trimStart, dst = 0;
    while (src <= trimEnd) {
      const uint32_t n = (src + CHUNK - 1 <= trimEnd) ? CHUNK : trimEnd - src + 1;
      f_lseek(&f, 44 + src * 2);
      res = f_read(&f, buf, n * 2, &br);
      if (res != FR_OK) { f_close(&f); return res; }
      UINT bw = 0;
      f_lseek(&f, 44 + dst * 2);
      res = f_write(&f, buf, br, &bw);
      if (res != FR_OK) { f_close(&f); return res; }
      src += n;
      dst += n;
    }
  }

  f_lseek(&f, 44 + newTotal * 2);
  f_truncate(&f);

  const uint32_t newDataBytes = newTotal * 2;
  uint8_t tmp[4];
  UINT bw = 0;
  writeLE32(tmp, 36 + newDataBytes);
  f_lseek(&f, 4);
  f_write(&f, tmp, 4, &bw);
  writeLE32(tmp, newDataBytes);
  f_lseek(&f, 40);
  f_write(&f, tmp, 4, &bw);

  return f_close(&f);
}

#endif  // PDM_CLOCK
