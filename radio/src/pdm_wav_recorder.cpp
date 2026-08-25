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

#include <math.h>
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

  uint16_t peak = peakSinceRead;
  for (uint32_t i = 0; i < n; i++) {
    const int32_t a = pcm[i] < 0 ? -(int32_t)pcm[i] : (int32_t)pcm[i];
    if (a > (int32_t)peak) peak = (uint16_t)a;
  }
  peakSinceRead = peak;

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

  // Drop the tail the stop press lands in. Only the header count changes;
  // finalise() truncates the stray bytes when it rewrites the file.
  static constexpr uint32_t TAIL_CUT = (DST_RATE * TAIL_CUT_MS) / 1000;
  if (samplesWritten > TAIL_CUT + DST_RATE / 10) samplesWritten -= TAIL_CUT;

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

uint8_t PdmWavRecorder::takePeakLevel()
{
  const uint16_t peak = peakSinceRead;
  peakSinceRead = 0;
  return (uint8_t)(peak >> 7);
}

// ---------------------------------------------------------------------------
// Post-recording clean-up.
//
// The CIC decimator leaves two artefacts worth fixing offline, where cost does
// not matter: a residual DC/rumble term (the mic's ones-density is never
// exactly 50%), and sinc^3 passband droop -- about -2.7 dB at 4 kHz and
// -11.8 dB at 8 kHz for R=100, N=3, which is what makes takes sound muffled.
// Level is then set from the measured peak instead of the fixed capture gain.
// ---------------------------------------------------------------------------

// One-pole high-pass, -3 dB at ~80 Hz for a 16 kHz rate: exp(-2*pi*80/16000).
static constexpr float PP_HPF_A = 0.969f;

// Symmetric 3-tap droop compensator (-a, 1+2a, -a): +3.5 dB at 4 kHz,
// +6 dB at 8 kHz, unity at DC.
static constexpr float PP_COMP_A = 0.25f;

static constexpr float PP_NORM_TARGET = 29500.0f;  // ~-0.9 dBFS
static constexpr float PP_MAX_GAIN = 4.0f;

// Gain is also capped so it never lifts the take's own noise floor past this
// RMS (~-38 dBFS). The stop click used to be the loudest thing in every file
// and quietly held the gain near 1; with it gone, a quiet take would other-
// wise be boosted until room tone and CIC hiss become the loudest thing.
static constexpr float PP_NOISE_CEILING = 400.0f;
static constexpr uint32_t PP_FADE_SAMPLES = PdmWavRecorder::DST_RATE / 125;  // 8 ms

namespace {
struct PostFilter
{
  float hpX = 0.0f, hpY = 0.0f;
  float d1 = 0.0f, d2 = 0.0f;
  bool primed = false;

  float process(int16_t sample)
  {
    const float x = (float)sample;
    if (!primed) { hpX = x; primed = true; }   // no step at the first sample
    const float y = PP_HPF_A * (hpY + x - hpX);
    hpX = x;
    hpY = y;
    const float out = (1.0f + 2.0f * PP_COMP_A) * d1 - PP_COMP_A * (y + d2);
    d2 = d1;
    d1 = y;
    return out;
  }
};
}  // namespace

FRESULT PdmWavRecorder::finalise(const char* path, uint8_t* env, uint16_t cols,
                                 uint32_t* totalSamples,
                                 uint32_t* clippedPermille)
{
  // 4 KB of static buffer rather than a stack one: a pass over the card costs
  // far more in transfers than in bytes, and everything that needs to see
  // every sample shares these two passes.
  static int16_t ioBuf[2048];
  static constexpr uint32_t CHUNK = sizeof(ioBuf) / sizeof(ioBuf[0]);

  // Threshold tracks the post-CIC gain so trim stays calibrated when
  // PDM_POST_GAIN_SHIFT is changed. Baseline 4000 was tuned at shift=4.
  static constexpr int32_t _silenceRaw =
      ((int32_t)4000 * (1 << PDM_POST_GAIN_SHIFT)) / 16;
  static constexpr int16_t SILENCE_THRESHOLD =
      _silenceRaw > 32767 ? (int16_t)32767 : (int16_t)_silenceRaw;
  static constexpr uint32_t PAD_SAMPLES = DST_RATE / 100;  // 10 ms

  if (env && cols) memset(env, 0, cols);
  if (totalSamples) *totalSamples = 0;
  if (clippedPermille) *clippedPermille = 0;

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

  // Pass 1 (sequential read): silence bounds, capture clipping, filtered peak.
  PostFilter flt;
  uint32_t firstActive = UINT32_MAX;
  uint32_t lastActive = 0;
  uint32_t clipped = 0;
  float peak = 0.0f;
  float quietestRms = 0.0f;
  bool haveRms = false;

  for (uint32_t base = 0; base < total; base += CHUNK) {
    const uint32_t n = (base + CHUNK <= total) ? CHUNK : total - base;
    res = f_read(&f, ioBuf, n * 2, &br);
    if (res != FR_OK) { f_close(&f); return res; }
    const uint32_t got = br / 2;
    float sumSq = 0.0f;
    for (uint32_t i = 0; i < got; i++) {
      const int16_t s = ioBuf[i];
      const int32_t a = s < 0 ? -(int32_t)s : (int32_t)s;
      if (a >= 32767) clipped++;
      if (a > SILENCE_THRESHOLD) {
        if (firstActive == UINT32_MAX) firstActive = base + i;
        lastActive = base + i;
      }
      const float v = flt.process(s);
      const float m = fabsf(v);
      if (m > peak) peak = m;
      sumSq += v * v;
    }
    if (got == CHUNK) {   // quietest full block is the noise floor
      const float rms = sqrtf(sumSq / (float)CHUNK);
      if (!haveRms || rms < quietestRms) { quietestRms = rms; haveRms = true; }
    }
    if (got < n) break;
  }

  uint32_t trimStart = 0, trimEnd = total - 1;
  if (firstActive != UINT32_MAX) {
    trimStart = firstActive > PAD_SAMPLES ? firstActive - PAD_SAMPLES : 0;
    trimEnd = (lastActive + PAD_SAMPLES < total) ? lastActive + PAD_SAMPLES
                                                 : total - 1;
  }
  const uint32_t newTotal = trimEnd - trimStart + 1;

  if (clippedPermille) *clippedPermille = (clipped * 1000U) / total;
  if (totalSamples) *totalSamples = newTotal;

  const bool doFilter = newTotal >= 2 * PP_FADE_SAMPLES;

  float gain = 1.0f;
  if (doFilter && peak > 1.0f) {
    gain = PP_NORM_TARGET / peak;
    if (haveRms && quietestRms > 1.0f) {
      float noiseLimit = PP_NOISE_CEILING / quietestRms;
      if (noiseLimit < 1.0f) noiseLimit = 1.0f;   // never attenuate for noise
      if (gain > noiseLimit) gain = noiseLimit;
    }
    if (gain > PP_MAX_GAIN) gain = PP_MAX_GAIN;
  }

  // Pass 2: shift to the front, filter, normalise, fade, fill the envelope.
  flt = PostFilter();
  uint32_t src = trimStart, dst = 0;

  while (src <= trimEnd) {
    const uint32_t n = (src + CHUNK - 1 <= trimEnd) ? CHUNK : trimEnd - src + 1;
    f_lseek(&f, 44 + src * 2);
    res = f_read(&f, ioBuf, n * 2, &br);
    if (res != FR_OK) { f_close(&f); return res; }
    const uint32_t got = br / 2;

    for (uint32_t i = 0; i < got; i++) {
      const uint32_t idx = dst + i;
      int32_t out;

      if (doFilter) {
        float v = flt.process(ioBuf[i]) * gain;
        if (idx < PP_FADE_SAMPLES) {
          v *= (float)idx / (float)PP_FADE_SAMPLES;
        } else if (idx >= newTotal - PP_FADE_SAMPLES) {
          v *= (float)(newTotal - idx) / (float)PP_FADE_SAMPLES;
        }
        if (v > 32767.0f) v = 32767.0f;
        if (v < -32768.0f) v = -32768.0f;
        out = (int32_t)lrintf(v);
      } else {
        out = ioBuf[i];
      }
      ioBuf[i] = (int16_t)out;

      if (env && cols) {
        int32_t a = out < 0 ? -out : out;
        if (a > 32767) a = 32767;
        const uint16_t c = (uint16_t)(((uint64_t)idx * cols) / newTotal);
        const uint8_t lvl = (uint8_t)(a >> 7);
        if (lvl > env[c]) env[c] = lvl;
      }
    }

    UINT bw = 0;
    f_lseek(&f, 44 + dst * 2);
    res = f_write(&f, ioBuf, got * 2, &bw);
    if (res != FR_OK || bw != got * 2) {
      f_close(&f);
      return res != FR_OK ? res : FR_DISK_ERR;
    }

    src += got;
    dst += got;
    if (got < n) break;
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
