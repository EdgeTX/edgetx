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

// 4 KB of static buffer rather than a stack one: a pass over the card costs
// far more in transfers than in bytes.
static int16_t ioBuf[2048];
static constexpr uint32_t CHUNK = sizeof(ioBuf) / sizeof(ioBuf[0]);

// Silence is judged against the take's own voice peak, not an absolute level:
// capture gain is fixed, so a quiet talker never crossed a fixed threshold and
// the trim either did nothing or kept only the loudest syllables.
// Levels below were calibrated at PDM_POST_GAIN_SHIFT == 5; scaling them off
// it keeps the trim calibrated when the capture gain is changed.
#define PDM_LEVEL(x) (((x) * (1 << PDM_POST_GAIN_SHIFT)) / 32)

static constexpr int32_t SILENCE_RATIO = 8;      // -18 dB below the voice peak
static constexpr int32_t SILENCE_FLOOR = PDM_LEVEL(200);
static constexpr uint32_t COARSE_MAX = 512;      // whole-file level map
static uint16_t coarseLevel[COARSE_MAX];
// Asymmetric on purpose: speech tails off below the threshold (trailing
// consonants, breath), so cutting 10 ms past the last loud sample clips it.
// Stop-press detection. The press is a short burst, much louder than the
// speech before it, sitting at the very end of the take. Anything that is
// long, or not near the end, is treated as speech and left alone.
static constexpr uint32_t SUB_SAMPLES = 256;                        // 16 ms
static constexpr uint32_t TAIL_RING = 128;                          // ~2.0 s
static constexpr uint32_t CLICK_WINDOW_BLOCKS = 94;                 // ~1.5 s
static constexpr uint32_t CLICK_MAX_BLOCKS = 25;                    // ~0.4 s
static constexpr uint32_t CLICK_GUARD_BLOCKS = 2;                   // ~32 ms
static constexpr uint32_t CLICK_LOOKBACK = 8;                       // ~128 ms
static constexpr int32_t CLICK_MIN_LEVEL = PDM_LEVEL(300);           // ~-40 dBFS at shift 5
static constexpr uint32_t CLICK_GAP_BLOCKS = 38;                    // ~0.6 s hold
static constexpr uint32_t CLICK_MAX_BURSTS = 4;
static constexpr uint32_t CLICK_PAUSE_BLOCKS = 10;                  // ~0.16 s

static constexpr uint32_t PAD_HEAD_SAMPLES = PdmWavRecorder::DST_RATE / 10;   // 100 ms
static constexpr uint32_t PAD_TAIL_SAMPLES = PdmWavRecorder::DST_RATE / 5;    // 200 ms

static constexpr float PP_NORM_TARGET = 29500.0f;  // ~-0.9 dBFS
static constexpr float PP_MAX_GAIN = 4.0f;

// Gain is also capped so it never lifts the take's own noise floor past this
// RMS (~-38 dBFS). The stop click used to be the loudest thing in every file
// and quietly held the gain near 1; with it gone, a quiet take would other-
// wise be boosted until room tone and CIC hiss become the loudest thing.
static constexpr float PP_NOISE_CEILING = 400.0f;   // output level, not capture
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

// Reads the 44-byte header and returns the sample count, or 0 on any problem.
static uint32_t readWavTotal(FIL* f)
{
  uint8_t hdr[44];
  UINT br = 0;
  if (f_lseek(f, 0) != FR_OK) return 0;
  if (f_read(f, hdr, sizeof(hdr), &br) != FR_OK || br != sizeof(hdr)) return 0;
  const uint32_t dataBytes =
      (uint32_t)hdr[40] | ((uint32_t)hdr[41] << 8) |
      ((uint32_t)hdr[42] << 16) | ((uint32_t)hdr[43] << 24);
  return dataBytes / 2;
}

static void patchWavHeader(FIL* f, uint32_t samples)
{
  uint8_t tmp[4];
  UINT bw = 0;
  writeLE32(tmp, 36 + samples * 2);
  f_lseek(f, 4);
  f_write(f, tmp, 4, &bw);
  writeLE32(tmp, samples * 2);
  f_lseek(f, 40);
  f_write(f, tmp, 4, &bw);
}

FRESULT PdmWavRecorder::silenceBounds(const char* path, uint32_t* from,
                                      uint32_t* to)
{
  if (!from || !to) return FR_INVALID_PARAMETER;

  FIL f;
  FRESULT res = f_open(&f, path, FA_READ);
  if (res != FR_OK) return res;

  const uint32_t total = readWavTotal(&f);
  if (total == 0) { f_close(&f); return FR_DISK_ERR; }

  uint32_t firstActive = UINT32_MAX, lastActive = 0;
  UINT br = 0;
  int32_t silence = SILENCE_FLOOR;

  for (int pass = 0; pass < 2; pass++) {
    int32_t peak = 0;
    f_lseek(&f, 44);
    for (uint32_t base = 0; base < total; base += CHUNK) {
      const uint32_t n = (base + CHUNK <= total) ? CHUNK : total - base;
      res = f_read(&f, ioBuf, n * 2, &br);
      if (res != FR_OK) { f_close(&f); return res; }
      const uint32_t got = br / 2;
      for (uint32_t i = 0; i < got; i++) {
        const int32_t a = ioBuf[i] < 0 ? -(int32_t)ioBuf[i] : (int32_t)ioBuf[i];
        if (pass == 0) {
          if (a > peak) peak = a;
        } else if (a > silence) {
          if (firstActive == UINT32_MAX) firstActive = base + i;
          lastActive = base + i;
        }
      }
      if (got < n) break;
    }
    if (pass == 0) {
      silence = peak / SILENCE_RATIO;
      if (silence < SILENCE_FLOOR) silence = SILENCE_FLOOR;
    }
  }

  if (firstActive == UINT32_MAX) {          // nothing above the threshold
    *from = 0;
    *to = total - 1;
  } else {
    *from = firstActive > PAD_HEAD_SAMPLES ? firstActive - PAD_HEAD_SAMPLES : 0;
    *to = (lastActive + PAD_TAIL_SAMPLES < total) ? lastActive + PAD_TAIL_SAMPLES
                                                  : total - 1;
  }
  return f_close(&f);
}

FRESULT PdmWavRecorder::cut(const char* path, uint32_t from, uint32_t to,
                            uint8_t* env, uint16_t cols,
                            uint32_t* totalSamples)
{
  if (env && cols) memset(env, 0, cols);

  FIL f;
  FRESULT res = f_open(&f, path, FA_READ | FA_WRITE);
  if (res != FR_OK) return res;

  const uint32_t total = readWavTotal(&f);
  if (total == 0) { f_close(&f); return FR_DISK_ERR; }

  if (to >= total) to = total - 1;
  if (to <= from) { f_close(&f); return FR_INVALID_PARAMETER; }

  const uint32_t newTotal = to - from + 1;
  if (newTotal * 1000U < MIN_TAKE_MS * DST_RATE) {
    f_close(&f);
    return FR_INVALID_PARAMETER;
  }
  if (totalSamples) *totalSamples = newTotal;

  const uint32_t fade =
      newTotal / 4 < PP_FADE_SAMPLES ? newTotal / 4 : PP_FADE_SAMPLES;

  uint32_t src = from, dst = 0;
  UINT br = 0;

  while (src <= to) {
    const uint32_t n = (src + CHUNK - 1 <= to) ? CHUNK : to - src + 1;
    f_lseek(&f, 44 + src * 2);
    res = f_read(&f, ioBuf, n * 2, &br);
    if (res != FR_OK) { f_close(&f); return res; }
    const uint32_t got = br / 2;

    for (uint32_t i = 0; i < got; i++) {
      const uint32_t idx = dst + i;
      int32_t v = ioBuf[i];

      if (fade) {
        if (idx < fade) {
          v = (int32_t)((int64_t)v * (int32_t)idx / (int32_t)fade);
        } else if (idx >= newTotal - fade) {
          v = (int32_t)((int64_t)v * (int32_t)(newTotal - idx) / (int32_t)fade);
        }
      }
      ioBuf[i] = (int16_t)v;

      if (env && cols) {
        int32_t a = v < 0 ? -v : v;
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
  patchWavHeader(&f, newTotal);

  return f_close(&f);
}

FRESULT PdmWavRecorder::finalise(const char* path, uint8_t* env, uint16_t cols,
                                 uint32_t* totalSamples,
                                 uint32_t* clippedPermille,
                                 PressDiag* diag)
{
  if (env && cols) memset(env, 0, cols);
  if (totalSamples) *totalSamples = 0;
  if (clippedPermille) *clippedPermille = 0;
  if (diag) *diag = PressDiag{0, 0, 0, 0};

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
  uint32_t clipped = 0;
  float peak = 0.0f;
  float quietestRms = 0.0f;
  bool haveRms = false;

  const uint32_t totalBlocks = (total + SUB_SAMPLES - 1) / SUB_SAMPLES;
  // A take shorter than the window is searched whole rather than skipped.
  const uint32_t firstTailBlock =
      totalBlocks > CLICK_WINDOW_BLOCKS ? totalBlocks - CLICK_WINDOW_BLOCKS : 0;

  const uint32_t coarseSize =
      total / COARSE_MAX + 1 > SUB_SAMPLES ? total / COARSE_MAX + 1 : SUB_SAMPLES;
  const uint32_t coarseCount = (total + coarseSize - 1) / coarseSize;
  memset(coarseLevel, 0, sizeof(coarseLevel));
  int32_t coarsePeak = 0;

  int32_t blockPeak[TAIL_RING] = {0};
  int32_t peakBeforeWindow = 0;
  int32_t roomLevel = INT32_MAX;   // quietest block: the take's own noise floor
  int32_t subPeak = 0;

  for (uint32_t base = 0; base < total; base += CHUNK) {
    const uint32_t n = (base + CHUNK <= total) ? CHUNK : total - base;
    res = f_read(&f, ioBuf, n * 2, &br);
    if (res != FR_OK) { f_close(&f); return res; }
    const uint32_t got = br / 2;
    float sumSq = 0.0f;
    for (uint32_t i = 0; i < got; i++) {
      const uint32_t idx = base + i;
      const int16_t s = ioBuf[i];
      const int32_t a = s < 0 ? -(int32_t)s : (int32_t)s;
      if (a >= 32767) clipped++;
      if (a > subPeak) subPeak = a;
      if ((idx + 1) % SUB_SAMPLES == 0 || idx + 1 == total) {
        const uint32_t b = idx / SUB_SAMPLES;
        blockPeak[b % TAIL_RING] = subPeak;
        if (b < firstTailBlock && subPeak > peakBeforeWindow)
          peakBeforeWindow = subPeak;
        if (subPeak < roomLevel) roomLevel = subPeak;
        subPeak = 0;
      }
      const float v = flt.process(s);
      const float m = fabsf(v);
      if (m > peak) peak = m;
      sumSq += v * v;

      int32_t mi = (int32_t)m;
      if (mi > 32767) mi = 32767;
      if (mi > coarsePeak) coarsePeak = mi;
      if ((idx + 1) % coarseSize == 0 || idx + 1 == total) {
        coarseLevel[idx / coarseSize] = (uint16_t)coarsePeak;
        coarsePeak = 0;
      }
    }
    if (got == CHUNK) {   // quietest full block is the noise floor
      const float rms = sqrtf(sumSq / (float)CHUNK);
      if (!haveRms || rms < quietestRms) { quietestRms = rms; haveRms = true; }
    }
    if (got < n) break;
  }

  // Locate the stop press: the earliest block in the trailing window that
  // stands clear of everything before it, provided the burst it starts is
  // short enough to be a press and not the last word.
  // Pressing the screen makes two bursts, not one: the finger landing and
  // the finger lifting, with a decay dip between them. Walk back from the end
  // over as many adjacent bursts as qualify, judging each against the voice
  // level rather than against the other burst.
  uint32_t clickStart = 0;
  if (totalBlocks > CLICK_MAX_BLOCKS + CLICK_GUARD_BLOCKS + 2) {
    if (roomLevel == INT32_MAX) roomLevel = 0;
    const int32_t audible = roomLevel * 3 > CLICK_MIN_LEVEL ? roomLevel * 3
                                                            : CLICK_MIN_LEVEL;
    const int32_t voiceLevel =
        peakBeforeWindow > audible ? peakBeforeWindow : audible;
    if (diag) diag->voiceLevel = voiceLevel;

    uint32_t searchEnd = totalBlocks;
    uint32_t cutBlock = 0;
    int32_t burstPeak = 0;
    bool haveCut = false;

    for (uint32_t iter = 0; iter < CLICK_MAX_BURSTS; iter++) {
      // Between the landing and the lift there is decay, not silence. Once a
      // burst is known, only something of comparable size counts as the next
      // one, so that decay is crossed as a gap instead of ending the walk.
      const int32_t seek =
          haveCut && burstPeak / 8 > audible ? burstPeak / 8 : audible;

      uint32_t last = 0;
      bool found = false;
      for (uint32_t b = searchEnd; b-- > firstTailBlock;) {
        if (blockPeak[b % TAIL_RING] > seek) { last = b; found = true; break; }
      }
      if (!found) break;
      if (haveCut && cutBlock - last > CLICK_GAP_BLOCKS) break;

      int32_t eventPeak = blockPeak[last % TAIL_RING];
      uint32_t onset = last;
      while (onset > firstTailBlock) {
        const int32_t half = eventPeak / 2;
        const int32_t limit = half > audible ? half : audible;
        const int32_t prev = blockPeak[(onset - 1) % TAIL_RING];
        if (prev <= limit) break;
        onset--;
        if (prev > eventPeak) eventPeak = prev;
      }

      if (diag && iter == 0) {
        diag->burstPeak = eventPeak;
        diag->burstMs = ((last - onset + 1) * SUB_SAMPLES * 1000U) / DST_RATE;
      }

      if (onset <= firstTailBlock) break;              // start not in view
      if (last - onset + 1 > CLICK_MAX_BLOCKS) break;  // too long to be a press
      // No loudness test here on purpose: it would depend on where the window
      // boundary happens to fall relative to the speech. Whether this really
      // is a press is decided once, below, against the voice measured ahead
      // of the whole run of bursts.

      cutBlock = onset;
      if (eventPeak > burstPeak) burstPeak = eventPeak;
      haveCut = true;
      searchEnd = onset;
    }

    if (haveCut) {
      // The window is wide enough to hold a slow press-and-hold, which means
      // it can also hold the tail of the voice. Re-measure the voice over
      // everything ahead of the press before accepting it.
      int32_t voiceCheck = peakBeforeWindow;
      for (uint32_t b = firstTailBlock; b < cutBlock; b++) {
        const int32_t l = blockPeak[b % TAIL_RING];
        if (l > voiceCheck) voiceCheck = l;
      }
      if (voiceCheck < audible) voiceCheck = audible;
      if (diag) diag->voiceLevel = voiceCheck;

      // How much quiet sits ahead of the press.
      const int32_t quiet =
          voiceCheck / 8 > audible ? voiceCheck / 8 : audible;
      uint32_t gapBlocks = 0;
      for (uint32_t b = cutBlock; b-- > firstTailBlock;) {
        if (blockPeak[b % TAIL_RING] > quiet) break;
        gapBlocks++;
      }

      // Twice the voice settles it outright. When capture clips, both the
      // press and the voice sit at full scale and no ratio can separate
      // them, so a burst merely as loud as the voice counts too - provided
      // it follows a pause, which a word carried on from speech does not.
      const bool louder = burstPeak >= voiceCheck * 2;
      // Comparable, not greater: when both saturate, which of them reads a
      // few counts higher is arbitrary.
      const bool afterPause =
          burstPeak * 4 >= voiceCheck * 3 && gapBlocks >= CLICK_PAUSE_BLOCKS;

      if (louder || afterPause) {
        const uint32_t guard = cutBlock > firstTailBlock + CLICK_GUARD_BLOCKS
                                   ? CLICK_GUARD_BLOCKS
                                   : 0;
        clickStart = (cutBlock - guard) * SUB_SAMPLES;
      }
    }
  }

  if (diag && clickStart > 0) diag->cutSamples = total - clickStart;

  // Silence bounds, from the level map and a threshold set by the take's own
  // voice peak. The press is excluded from both, so neither the threshold nor
  // the padding is measured from it.
  const uint32_t pressBlock =
      clickStart > 0 ? clickStart / coarseSize : coarseCount;

  int32_t voicePeak = 0;
  for (uint32_t b = 0; b < pressBlock && b < coarseCount; b++) {
    if (coarseLevel[b] > voicePeak) voicePeak = coarseLevel[b];
  }

  int32_t silence = voicePeak / SILENCE_RATIO;
  if (silence < roomLevel * 4) silence = roomLevel * 4;
  if (silence < SILENCE_FLOOR) silence = SILENCE_FLOOR;

  uint32_t firstActive = UINT32_MAX, lastActive = 0;
  for (uint32_t b = 0; b < pressBlock && b < coarseCount; b++) {
    if (coarseLevel[b] <= silence) continue;
    if (firstActive == UINT32_MAX) firstActive = b * coarseSize;
    lastActive = (b + 1) * coarseSize - 1;
  }
  if (lastActive >= total) lastActive = total - 1;

  uint32_t trimStart = 0, trimEnd = total - 1;
  if (firstActive != UINT32_MAX) {
    trimStart = firstActive > PAD_HEAD_SAMPLES ? firstActive - PAD_HEAD_SAMPLES : 0;
    trimEnd = (lastActive + PAD_TAIL_SAMPLES < total) ? lastActive + PAD_TAIL_SAMPLES
                                                      : total - 1;
  }
  if (clickStart > 0 && trimEnd >= clickStart) trimEnd = clickStart - 1;

  // If nothing cleared the threshold, the bounds can cross. Keep everything ahead of the press instead.
  if (trimEnd <= trimStart) {
    trimStart = 0;
    trimEnd = clickStart > 0 ? clickStart - 1 : total - 1;
  }
  if (trimEnd <= trimStart) {
    trimStart = 0;
    trimEnd = total - 1;
  }
  const uint32_t newTotal = trimEnd - trimStart + 1;

  if (clippedPermille) *clippedPermille = (clipped * 1000U) / total;
  if (totalSamples) *totalSamples = newTotal;

  const bool doFilter = newTotal >= 2 * PP_FADE_SAMPLES;

  // Measured over what survives the trim: the press is about to go, and
  // normalising to it would leave the voice quiet.
  float keptPeak = 0.0f;
  for (uint32_t b = trimStart / coarseSize;
       b <= trimEnd / coarseSize && b < coarseCount; b++) {
    if ((float)coarseLevel[b] > keptPeak) keptPeak = (float)coarseLevel[b];
  }
  if (keptPeak <= 1.0f) keptPeak = peak;

  float gain = 1.0f;
  if (doFilter && keptPeak > 1.0f) {
    gain = PP_NORM_TARGET / keptPeak;
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
