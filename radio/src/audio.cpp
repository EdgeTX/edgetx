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

#include <math.h>

#include "os/sleep.h"
#include "os/task.h"

#include "edgetx.h"
#include "strhelpers.h"
#include "switches.h"

#if defined(LIBOPENUI)
#include "libopenui.h"
#endif

#include "model_audio.h"
#include "hal/audio_driver.h"

extern mutex_handle_t audioMutex;

// Only first quadrant values - other quadrants calulated taking advantage of symmetry in sine wave.
const int16_t sineValues[] =
{
    0, 196, 392, 588, 784, 980, 1175, 1370, 1564, 1758,
    1951, 2143, 2335, 2525, 2715, 2904, 3091, 3278, 3463, 3647,
    3829, 4011, 4190, 4369, 4545, 4720, 4894, 5065, 5235, 5403,
    5569, 5733, 5895, 6055, 6213, 6369, 6522, 6673, 6822, 6969,
    7113, 7255, 7395, 7532, 7667, 7799, 7929, 8056, 8180, 8302,
    8422, 8539, 8653, 8765, 8874, 8980, 9084, 9185, 9283, 9379,
    9472, 9562, 9650, 9735, 9818, 9898, 9975, 10050, 10123, 10192,
    10260, 10324, 10387, 10447, 10504, 10559, 10612, 10663, 10711, 10757,
    10801, 10843, 10882, 10920, 10955, 10989, 11020, 11050, 11078, 11104,
    11128, 11151, 11172, 11191, 11209, 11225, 11240, 11254, 11266, 11277,
    11287, 11296, 11303, 11310, 11316, 11320, 11324, 11327, 11330, 11331,
    11332, 11333, 11333, 11333, 11332, 11331, 11329, 11328, 11326, 11324,
    11323, 11321, 11319, 11318, 11316, 11315, 11314, 11313, 11313, 11313,
    11314, 11315, 11317, 11319, 11323, 11326, 11331, 11336, 11342, 11349,
    11356, 11365, 11375, 11385, 11397, 11409, 11423, 11437, 11453, 11470,
    11488, 11507, 11527, 11548, 11571, 11595, 11620, 11646, 11673, 11702,
    11732, 11763, 11795, 11828, 11863, 11899, 11936, 11974, 12013, 12054,
    12095, 12138, 12182, 12227, 12273, 12320, 12368, 12417, 12467, 12518,
    12570, 12623, 12676, 12731, 12786, 12842, 12898, 12956, 13014, 13072,
    13131, 13191, 13251, 13311, 13372, 13433, 13495, 13556, 13618, 13680,
    13743, 13805, 13867, 13929, 13991, 14053, 14115, 14177, 14238, 14299,
    14359, 14419, 14479, 14538, 14597, 14655, 14712, 14768, 14824, 14879,
    14933, 14986, 15039, 15090, 15140, 15189, 15237, 15284, 15330, 15375,
    15418, 15460, 15500, 15539, 15577, 15614, 15648, 15682, 15714, 15744,
    15772, 15799, 15825, 15849, 15871, 15891, 15910, 15927, 15942, 15955,
    15967, 15977, 15985, 15991, 15996, 15999, 16000,
};

#define SINE_INDEX_Q1 256
#define SINE_INDEX_Q2 512
#define SINE_INDEX_Q3 768
#define MAX_SINE_INDEX 1024

const char * const unitsFilenames[] = {
  "",
  "volt",
  "amp",
  "mamp",
  "knot",
  "mps",
  "fps",
  "kph",
  "mph",
  "meter",
  "foot",
  "celsius",
  "fahr",
  "percent",
  "mamph",
  "watt",
  "mwatt",
  "db",
  "rpm",
  "g",
  "degree",
  "rad",
  "ml",
  "founce",
  "mlpm",
  "hertz",
  "ms",
  "us",
  "km",
  "dbm",
  "spare6",
  "spare7",
  "spare8",
  "spare9",
  "spare10",
  "hour",
  "minute",
  "second",
};

const char * const audioFilenames[] = {
  "hello",
  "bye",
  "thralert",
  "swalert",
  "baddata",
  "lowbatt",
  "inactiv",
  "rssi_org",
  "rssi_red",
  "swr_red",
  "telemco",
  "telemko",
  "telemok",
  "trainco",
  "trainko",
  "trainok",
  "sensorko",
  "servoko",
  "rxko",
  "modelpwr",
  "error",
  "warning1",
  "warning2",
  "warning3",
  "midtrim",
  "mintrim",
  "maxtrim",
  "midstck1",
  "midstck2",
  "midstck3",
  "midstck4",
  "midpot1",
  "midpot2",
#if defined(PCBX9E)
  "midpot3",
  "midpot4",
#endif
#if defined(PCBX10)
  "midpot4",
  "midpot5",
  "midpot6",
  "midpot7",
#endif
  "midslid1",
  "midslid2",
#if defined(PCBX9E)
  "midslid3",
  "midslid4",
#endif
  "mixwarn1",
  "mixwarn2",
  "mixwarn3",
  "timovr1",
  "timovr2",
  "timovr3"
};

constexpr unsigned int MAX_SWITCH_POSITIONS =
    MAX_SWITCHES * 3 + MAX_POTS * XPOTS_MULTIPOS_COUNT;

BitField<(AU_SPECIAL_SOUND_FIRST)> sdAvailableSystemAudioFiles;
BitField<(MAX_FLIGHT_MODES * 2/*on, off*/)> sdAvailableFlightmodeAudioFiles;
BitField<MAX_SWITCH_POSITIONS> sdAvailableSwitchAudioFiles;
BitField<(MAX_LOGICAL_SWITCHES * 2/*on, off*/)> sdAvailableLogicalSwitchAudioFiles;

char * getAudioPath(char * path)
{
  strcpy(path, SOUNDS_PATH "/");
  strncpy(path+SOUNDS_PATH_LNG_OFS, currentLanguagePack->id, 2);
  return path + sizeof(SOUNDS_PATH);
}

char * strAppendSystemAudioPath(char * path)
{
  char * str = getAudioPath(path);
  strcpy(str, SYSTEM_SUBDIR "/");
  return str + sizeof(SYSTEM_SUBDIR);
}

void getSystemAudioFile(char * filename, int index)
{
  char * str = strAppendSystemAudioPath(filename);
  strcpy(str, audioFilenames[index]);
  strcat(str, SOUNDS_EXT);
}

void referenceSystemAudioFiles()
{
  static_assert(sizeof(audioFilenames)==AU_SPECIAL_SOUND_FIRST*sizeof(char *), "Invalid audioFilenames size");
  char path[AUDIO_FILENAME_MAXLEN+1];
  FILINFO fno;
  DIR dir;

  sdAvailableSystemAudioFiles.reset();

#if defined(SIMU)
  // f_readdir does an f_stat call on every file when running in the simulator
  // so it is faster to just call f_stat on the files we are interested in
  for (int i=0; i<AU_SPECIAL_SOUND_FIRST; i++) {
    getSystemAudioFile(path, i);
    if (f_stat(path, nullptr) == FR_OK)
      sdAvailableSystemAudioFiles.setBit(i);
  }
#else
  // On the radio f_readdir on the whole SYSTEM folder is faster than
  // calling f_stat for each audio file name
  char * filename = strAppendSystemAudioPath(path);
  *(filename-1) = '\0';

  FRESULT res = f_opendir(&dir, path);        /* Open the directory */
  if (res == FR_OK) {
    for (;;) {
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      uint8_t len = strlen(fno.fname);

      // Eliminates directories / non wav files
      if (len < 5 || strcasecmp(fno.fname+len-4, SOUNDS_EXT) || (fno.fattrib & AM_DIR)) continue;

      for (int i=0; i<AU_SPECIAL_SOUND_FIRST; i++) {
        strcpy(path, audioFilenames[i]);
        strcat(path, SOUNDS_EXT);
        if (!strcasecmp(path, fno.fname)) {
          sdAvailableSystemAudioFiles.setBit(i);
          break;
        }
      }
    }
    f_closedir(&dir);
  }
#endif
}

void referenceModelAudioFiles()
{
  DIR dir;
  FILINFO fno;
  char path[AUDIO_FILENAME_MAXLEN + 1];

  sdAvailableFlightmodeAudioFiles.reset();
  sdAvailableSwitchAudioFiles.reset();
  sdAvailableLogicalSwitchAudioFiles.reset();

  getModelAudioPath(path, false);

  FRESULT res = f_opendir(&dir, path); /* Open the directory */
  if (res == FR_OK) {
    for (;;) {
      res = f_readdir(&dir, &fno); /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0)
        break; /* Break on error or end of dir */
      uint8_t len = strlen(fno.fname);

      // Eliminates directories / non wav files
      if (fno.fattrib & AM_DIR) continue;
      if (len < sizeof(SOUNDS_EXT)) continue;

      char *ext = fno.fname + len - (sizeof(SOUNDS_EXT) - 1);
      if (strcasecmp(ext, SOUNDS_EXT)) continue;

      TRACE("referenceModelAudioFiles(): using file: %s", fno.fname);

      int idx, event;
      if (matchModeAudioFile(fno.fname, idx, event)) {
        sdAvailableFlightmodeAudioFiles.setBit(
            INDEX_PHASE_AUDIO_FILE(idx, event));
        continue;
      }

      if (matchSwitchAudioFile(fno.fname, idx)) {
        sdAvailableSwitchAudioFiles.setBit(idx);
        continue;
      }

      if (matchLogicalSwitchAudioFile(fno.fname, idx, event)) {
        sdAvailableLogicalSwitchAudioFiles.setBit(
            INDEX_LOGICAL_SWITCH_AUDIO_FILE(idx, event));
        continue;
      }
    }
    f_closedir(&dir);
  }
}

bool isAudioFileReferenced(uint32_t i, char * filename)
{
  uint8_t category = (i >> 24);
  uint8_t index = (i >> 16) & 0xFF;
  event_t event = i & 0xFF;

  // TRACE("isAudioFileReferenced(%08x)", i);

  if (category == SYSTEM_AUDIO_CATEGORY) {
    if (sdAvailableSystemAudioFiles.getBit(event)) {
      getSystemAudioFile(filename, event);
      return true;
    }
  }
  else if (category == PHASE_AUDIO_CATEGORY) {
    if (sdAvailableFlightmodeAudioFiles.getBit(INDEX_PHASE_AUDIO_FILE(index, event))) {
      getFlightmodeAudioFile(filename, index, event);
      return true;
    }
  }
  else if (category == SWITCH_AUDIO_CATEGORY) {
    if (sdAvailableSwitchAudioFiles.getBit(index)) {
      getSwitchAudioFile(filename, SWSRC_FIRST_SWITCH + index);
      return true;
    }
  }
  else if (category == LOGICAL_SWITCH_AUDIO_CATEGORY) {
    if (sdAvailableLogicalSwitchAudioFiles.getBit(INDEX_LOGICAL_SWITCH_AUDIO_FILE(index, event))) {
      getLogicalSwitchAudioFile(filename, index, event);
      return true;
    }
  }

  return false;
}

tmr10ms_t timeAutomaticPromptsSilence = 0;

void playModelEvent(uint8_t category, uint8_t index, event_t event)
{
  char filename[AUDIO_FILENAME_MAXLEN+1];
  // TRACE("playModelEvent(): cat: %u, idx: %u, evt:%u", (uint32_t)category, (uint32_t)index, (uint32_t)event);
  if (IS_SILENCE_PERIOD_ELAPSED() && isAudioFileReferenced((category << 24) + (index << 16) + event, filename)) {
    audioQueue.playFile(filename);
  }
}

void playModelName()
{
  char filename[AUDIO_FILENAME_MAXLEN + 1];
  char *str = getModelAudioPath(filename);
  strcpy(str, "name.wav");
  audioQueue.playFile(filename);
}

AudioQueue audioQueue __DMA;      // to place it in the RAM section on Horus, to have file buffers in RAM for DMA access
AudioBuffer audioBuffers[AUDIO_BUFFER_COUNT] __DMA;

AudioQueue::AudioQueue()
  : buffersFifo(),
  _started(false),
  normalContext(),
  backgroundContext(),
  priorityContext(),
  varioContext(),
  fragmentsFifo()
{
}

#define CODEC_ID_PCM_S16LE  1


static void _audio_lock()
{
  mutex_lock(&audioMutex);
}

static void _audio_unlock()
{
  mutex_unlock(&audioMutex);
}

#if !defined(__SSAT)
  #define _sat_s16(x) ((int16_t)limit<int32_t>(INT16_MIN, (x), INT16_MAX))
#else
  #define _sat_s16(x) __SSAT((x), 16)
#endif

#if !defined(__USAT)
  #define _sat_u16(x) ((uint16_t)limit<uint32_t>(UINT16_MIN, (x), UINT16_MAX))
#else
  #define _sat_u16(x) __USAT((x), 16)
#endif

inline void mixSample(audio_data_t * result, int16_t sample, unsigned int fade)
{
  int32_t tmp = (int32_t)*result + ((int32_t)sample >> fade);
#if AUDIO_SAMPLE_FMT == AUDIO_SAMPLE_FMT_S16
  *result = (audio_data_t)_sat_s16(tmp);
#elif AUDIO_SAMPLE_FMT == AUDIO_SAMPLE_FMT_U16
  *result = (audio_data_t)_sat_u16((uint32_t)tmp);
#endif
}

#define RIFF_CHUNK_SIZE 12
uint8_t wavBuffer[AUDIO_BUFFER_SIZE * 2] __DMA;

int WavContext::mixBuffer(AudioBuffer *buffer, int volume, unsigned int fade)
{
  FRESULT result = FR_OK;
  UINT read = 0;

  if(fragment.fragmentVolume != USE_SETTINGS_VOLUME)
    volume = fragment.fragmentVolume;

  if (fragment.file[1]) {
    result = f_open(&state.file, fragment.file, FA_OPEN_EXISTING | FA_READ);
    fragment.file[1] = 0;
    if (result == FR_OK) {
      result = f_read(&state.file, wavBuffer, RIFF_CHUNK_SIZE+8, &read);
      if (result == FR_OK && read == RIFF_CHUNK_SIZE+8 && !memcmp(wavBuffer, "RIFF", 4) && !memcmp(wavBuffer+8, "WAVEfmt ", 8)) {
        uint32_t size = *((uint32_t *)(wavBuffer+16));
        result = (size < 256 ? f_read(&state.file, wavBuffer, size+8, &read) : FR_DENIED);
        if (result == FR_OK && read == size+8) {
          state.codec = ((uint16_t *)wavBuffer)[0];
          state.freq = ((uint16_t *)wavBuffer)[2];
          uint32_t *wavSamplesPtr = (uint32_t *)(wavBuffer + size);
          uint32_t size = wavSamplesPtr[1];
          if (state.freq != 0 && state.freq * (AUDIO_SAMPLE_RATE / state.freq) == AUDIO_SAMPLE_RATE) {
            state.resampleRatio = (AUDIO_SAMPLE_RATE / state.freq);
            state.readSize = (state.codec == CODEC_ID_PCM_S16LE ? 2*AUDIO_BUFFER_SIZE : AUDIO_BUFFER_SIZE) / state.resampleRatio;
          }
          else {
            result = FR_DENIED;
          }
          while (result == FR_OK && memcmp(wavSamplesPtr, "data", 4) != 0) {
            result = f_lseek(&state.file, f_tell(&state.file)+size);
            if (result == FR_OK) {
              result = f_read(&state.file, wavBuffer, 8, &read);
              if (read != 8) result = FR_DENIED;
              wavSamplesPtr = (uint32_t *)wavBuffer;
              size = wavSamplesPtr[1];
            }
          }
          state.size = size;
        }
        else {
          result = FR_DENIED;
        }
      }
      else {
        result = FR_DENIED;
      }
    }
  }

  if (result == FR_OK) {
    read = 0;
    result = f_read(&state.file, wavBuffer, state.readSize, &read);
    if (result == FR_OK) {
      if (read > state.size) {
        read = state.size;
      }
      state.size -= read;

      if (read != state.readSize) {
        f_close(&state.file);
        fragment.clear();
      }

      audio_data_t * samples = buffer->data;
      if (state.codec == CODEC_ID_PCM_S16LE) {
        read /= 2;
        for (uint32_t i=0; i<read; i++) {
          for (uint8_t j=0; j<state.resampleRatio; j++) {
            mixSample(samples++, ((int16_t *)wavBuffer)[i], fade+2-volume);
          }
        }
      }

      return samples - buffer->data;
    }
  }

  if (result != FR_OK) {
    f_close(&state.file);
    clear();
  }
  return 0;
}

const uint8_t toneVolumes[] = { 10, 8, 6, 4, 2 };
inline float evalVolumeRatio(int freq, int volume)
{
  float result = toneVolumes[2+volume];
  if (freq < 330) {
    result = (result * freq * freq) / (330 * 330);
  }
  return result;
}

int ToneContext::mixBuffer(AudioBuffer * buffer, int volume, unsigned int fade)
{
  int duration = 0;
  int result = 0;

  if(fragment.fragmentVolume != USE_SETTINGS_VOLUME)
    volume = fragment.fragmentVolume;

  int remainingDuration = fragment.tone.duration - state.duration;
  if (remainingDuration > 0) {
    int points;
    float toneIdx = state.idx;

    if (fragment.tone.reset) {
      fragment.tone.reset = 0;
      state.duration = 0;
      state.pause = 0;
    }

    if (fragment.tone.freq != state.freq) {
      state.freq = fragment.tone.freq;
      state.step = limit<float>(1, float(fragment.tone.freq) * (float(MAX_SINE_INDEX)/float(AUDIO_SAMPLE_RATE)), 512);
      state.volume = 1.0f / evalVolumeRatio(fragment.tone.freq, volume);
    }

    if (fragment.tone.freqIncr) {
      int freqChange = AUDIO_BUFFER_DURATION * fragment.tone.freqIncr;
      if (freqChange > 0) {
        fragment.tone.freq += freqChange;
        if (fragment.tone.freq > BEEP_MAX_FREQ) {
          fragment.tone.freq = BEEP_MAX_FREQ;
        }
      }
      else {
        if (fragment.tone.freq > BEEP_MIN_FREQ - freqChange) {
          fragment.tone.freq += freqChange;
        }
        else {
          fragment.tone.freq = BEEP_MIN_FREQ;
        }
      }
    }

    if (remainingDuration > AUDIO_BUFFER_DURATION) {
      duration = AUDIO_BUFFER_DURATION;
      points = AUDIO_BUFFER_SIZE;
    }
    else {
      duration = remainingDuration;
      points = (duration * AUDIO_BUFFER_SIZE) / AUDIO_BUFFER_DURATION;
      unsigned int end = toneIdx + (state.step * points);
      if (end > MAX_SINE_INDEX)
        end -= (end % MAX_SINE_INDEX);
      else
        end = MAX_SINE_INDEX;
      points = (float(end) - toneIdx) / state.step;
    }

    for (int i=0; i<points; i++) {
      int16_t sineIdx = ((int)toneIdx) % MAX_SINE_INDEX;
      int16_t sineVal;
      if (sineIdx <= SINE_INDEX_Q1)
        sineVal = sineValues[sineIdx];
      else if (sineIdx <= SINE_INDEX_Q2)
        sineVal = sineValues[SINE_INDEX_Q2 - sineIdx];
      else if (sineIdx <= SINE_INDEX_Q3)
        sineVal = -sineValues[sineIdx - SINE_INDEX_Q2];
      else
        sineVal = -sineValues[MAX_SINE_INDEX - sineIdx];
      int16_t sample = sineVal * state.volume;
      mixSample(&buffer->data[i], sample, fade);
      toneIdx += state.step;
      if ((unsigned int)toneIdx >= MAX_SINE_INDEX)
        toneIdx -= MAX_SINE_INDEX;
    }

    if (remainingDuration > AUDIO_BUFFER_DURATION) {
      state.duration += AUDIO_BUFFER_DURATION;
      state.idx = toneIdx;
      return AUDIO_BUFFER_SIZE;
    }
    else {
      state.duration = 32000; // once the tone is finished, it's not possible to update its frequency and duration
    }
  }

  remainingDuration = fragment.tone.pause - state.pause;
  if (remainingDuration > 0) {
    result = AUDIO_BUFFER_SIZE;
    state.pause += min<unsigned int>(AUDIO_BUFFER_DURATION-duration, fragment.tone.pause);
    if (fragment.tone.pause > state.pause)
      return result;
  }

  clear();
  return result;
}

void AudioQueue::wakeup()
{
  DEBUG_TIMER_START(debugTimerAudioConsume);
  audioConsumeCurrentBuffer();
  DEBUG_TIMER_STOP(debugTimerAudioConsume);

  AudioBuffer * buffer;
  while ((buffer = buffersFifo.getEmptyBuffer()) != nullptr) {
    int result;
    unsigned int fade = 0;
    int size = 0;

    // write silence in the buffer
    for (uint32_t i=0; i<AUDIO_BUFFER_SIZE; i++) {
      buffer->data[i] = AUDIO_DATA_SILENCE; /* silence */
    }

    // mix the priority context (only tones)
    result = priorityContext.mixBuffer(buffer, g_eeGeneral.beepVolume, fade);
    if (result > 0) {
      size = result;
      fade += 1;
    }

    // mix the normal context (tones and wavs)
    if (normalContext.isEmpty() && !fragmentsFifo.empty()) {
      _audio_lock();
      normalContext.setFragment(fragmentsFifo.get());
      _audio_unlock();
    }
    result = normalContext.mixBuffer(buffer, g_eeGeneral.beepVolume, g_eeGeneral.wavVolume, fade);
    if (result > 0) {
      size = max(size, result);
      fade += 1;
    }

    // mix the vario context
    result = varioContext.mixBuffer(buffer, g_eeGeneral.varioVolume, fade);
    if (result > 0) {
      size = max(size, result);
      fade += 1;
    }

    // mix the background context
    if (isFunctionActive(FUNCTION_BACKGND_MUSIC) && !isFunctionActive(FUNCTION_BACKGND_MUSIC_PAUSE)) {
      result = backgroundContext.mixBuffer(buffer, g_eeGeneral.backgroundVolume, fade);
      if (result > 0) {
        size = max(size, result);
      }
    }

    // push the buffer if needed
    if (size > 0) {
      // TRACE("pushing buffer %p", buffer);
      buffer->size = size;

#if defined(SOFTWARE_VOLUME)
      if (currentSpeakerVolume > 0) {
        for (uint32_t i=0; i<buffer->size; ++i) {
          int32_t tmpSample =
              (int32_t)((uint32_t)(buffer->data[i]) - AUDIO_DATA_SILENCE);
          buffer->data[i] = (int16_t)(((tmpSample * currentSpeakerVolume) /
                                       VOLUME_LEVEL_MAX) +
                                      AUDIO_DATA_SILENCE);
        }
        buffersFifo.audioPushBuffer();
      } else {
        break;
      }
#else
      buffersFifo.audioPushBuffer();
#endif
    } else {
      // break the endless loop
      break;
    }
    DEBUG_TIMER_START(debugTimerAudioConsume);
    audioConsumeCurrentBuffer();
    DEBUG_TIMER_STOP(debugTimerAudioConsume);
  }
}

inline unsigned int getToneLength(uint16_t len)
{
  unsigned int result = len; // default
  if (g_eeGeneral.beepLength < 0) {
    result /= (1-g_eeGeneral.beepLength);
  }
  else if (g_eeGeneral.beepLength > 0) {
    result *= (1+g_eeGeneral.beepLength);
  }
  return result;
}

void AudioQueue::pause(uint16_t len)
{
  playTone(0, 0, len);
}

bool AudioQueue::isPlaying(uint8_t id)
{
  return normalContext.hasPromptId(id) ||
         (isFunctionActive(FUNCTION_BACKGND_MUSIC) && backgroundContext.hasPromptId(id)) ||
         fragmentsFifo.hasPromptId(id);
}

void AudioQueue::playTone(uint16_t freq, uint16_t len, uint16_t pause, uint8_t flags, int8_t freqIncr, int8_t fragmentVolume)
{
  _audio_lock();

  freq = limit<uint16_t>(BEEP_MIN_FREQ, freq, BEEP_MAX_FREQ);

  if (flags & PLAY_BACKGROUND) {
    varioContext.setFragment(freq, len, pause, 0, 0, (flags & PLAY_NOW), fragmentVolume);
  }
  else {
    // adjust frequency and length according to the user preferences
    freq += g_eeGeneral.speakerPitch * 15;
    len = getToneLength(len);

    if (flags & PLAY_NOW) {
      if (priorityContext.isFree()) {
        priorityContext.clear();
        priorityContext.setFragment(freq, len, pause, flags & 0x0f, freqIncr, false, fragmentVolume);
      }
    }
    else {
      fragmentsFifo.push(AudioFragment(freq, len, pause, flags & 0x0f, freqIncr, false, fragmentVolume));
    }
  }

  _audio_unlock();
}

void AudioQueue::playFile(const char * filename, uint8_t flags, uint8_t id, int8_t fragmentVolume)
{
#if defined(SIMU)
  TRACE("playFile(\"%s\", flags=%x, id=%d fragmentVolume=%d ee_general=%d)", filename, flags, id, fragmentVolume, g_eeGeneral.wavVolume);
  if (strlen(filename) > AUDIO_FILENAME_MAXLEN) {
    TRACE("file name too long! maximum length is %d characters", AUDIO_FILENAME_MAXLEN);
  }
#endif

  if (!sdMounted())
    return;

  if (g_eeGeneral.beepMode == e_mode_quiet)
    return;

  if (strlen(filename) > AUDIO_FILENAME_MAXLEN) {
    POPUP_WARNING(STR_PATH_TOO_LONG);
    return;
  }

  _audio_lock();

  if (flags & PLAY_BACKGROUND) {
    backgroundContext.clear();
    backgroundContext.setFragment(filename, 0, fragmentVolume, id);
  }
  else {
    fragmentsFifo.push(AudioFragment(filename, flags & 0x0f, fragmentVolume, id));
  }

  _audio_unlock();
}

void AudioQueue::stopPlay(uint8_t id)
{
  _audio_lock();

  fragmentsFifo.removePromptById(id);
  backgroundContext.stop(id);

  _audio_unlock();
}

void AudioQueue::stopSD()
{
  sdAvailableSystemAudioFiles.reset();
  stopAll();
  playTone(0, 0, 100, PLAY_NOW);        // insert a 100ms pause
}

void AudioQueue::stopAll()
{
  flush();
  _audio_lock();
  priorityContext.clear();
  normalContext.clear();
  _audio_unlock();
}

void AudioQueue::flush()
{
  _audio_lock();
  fragmentsFifo.clear();
  varioContext.clear();
  backgroundContext.clear();
  _audio_unlock();
}

void audioPlay(unsigned int index, uint8_t id)
{
  if (g_eeGeneral.beepMode >= -1) {
    char filename[AUDIO_FILENAME_MAXLEN+1];
    if (isAudioFileReferenced(index, filename)) {
      audioQueue.playFile(filename, 0, id);
    }
  }
}

void audioKeyPress()
{
  if (g_eeGeneral.beepMode == e_mode_all) {
    audioQueue.playTone(BEEP_DEFAULT_FREQ, 40, 20, PLAY_NOW);
  }
#if defined(HAPTIC)
  if (g_eeGeneral.hapticMode == e_mode_all) {
    haptic.play(5, 0, PLAY_NOW);
  }
#endif
}

void audioKeyError()
{
  if (g_eeGeneral.beepMode >= e_mode_nokeys) {
    audioQueue.playTone(BEEP_DEFAULT_FREQ, 160, 20, PLAY_NOW);
  }

#if defined(HAPTIC)
  if (g_eeGeneral.hapticMode >= e_mode_nokeys) {
    haptic.play(15, 3, PLAY_NOW);
  }
#endif
}

void audioTrimPress(int value)
{
  if (g_eeGeneral.beepMode >= e_mode_nokeys) {
    value = limit(TRIM_MIN, value, TRIM_MAX) * 8 + 120*16;
    audioQueue.playTone(value, 40, 20, PLAY_NOW);
  }
}

void audioTimerCountdown(uint8_t timer, int value)
{
  if (g_model.timers[timer].countdownBeep == COUNTDOWN_VOICE) {
    int announceValue = value;
    if (g_model.timers[timer].showElapsed) {
      announceValue = g_model.timers[timer].start - value;
    }
    if (value >= 0 && value <= TIMER_COUNTDOWN_START(timer)) {
      if (announceValue > 60 && !(announceValue % 2) && (announceValue % 30) &&
          (announceValue % 30))
        playNumber(announceValue / 60, 0, 0, 0);
      if (announceValue < 60 ||
          (announceValue > 60 && !(announceValue % 2) && (announceValue % 60)))
        playNumber(announceValue % 60, 0, 0, 0);
    } else if ((!(announceValue % 30) || !(announceValue % 20)) && value < 31) {
      playDuration(announceValue, 0, 0);
    }
  } else if (g_model.timers[timer].countdownBeep == COUNTDOWN_BEEPS) {
    if (value == 0) {
      audioQueue.playTone(BEEP_DEFAULT_FREQ + 150, 300, 20, PLAY_NOW);
    } else if (value > 0 && value <= TIMER_COUNTDOWN_START(timer)) {
      audioQueue.playTone(BEEP_DEFAULT_FREQ + 150, 100, 20, PLAY_NOW);
    } else if (value == 30) {
      audioQueue.playTone(BEEP_DEFAULT_FREQ + 150, 120, 20, PLAY_REPEAT(2));
    } else if (value == 20) {
      audioQueue.playTone(BEEP_DEFAULT_FREQ + 150, 120, 20, PLAY_REPEAT(1));
    } else if (value == 10) {
      audioQueue.playTone(BEEP_DEFAULT_FREQ + 150, 120, 20, PLAY_NOW);
    }
  }
#if defined(HAPTIC)
  if ( (g_model.timers[timer].countdownBeep == COUNTDOWN_HAPTIC) ||
       (g_model.timers[timer].extraHaptic) ) {
    if (value == 0) {
      haptic.play(15, 3, PLAY_NOW);
    }
    else if (value > 0 && value <= TIMER_COUNTDOWN_START(timer)) {
      haptic.play(10, 0, PLAY_NOW);
    }
    else if (value == 30) {
      haptic.play(10, 3, PLAY_REPEAT(2) | PLAY_NOW);
    }
    else if (value == 20) {
      haptic.play(10, 3, PLAY_REPEAT(1) | PLAY_NOW);
    }
    else if (value == 10) {
      haptic.play(10, 3, PLAY_NOW);
    }
  }
#endif
}

void audioEvent(unsigned int index)
{
  if (index == AU_NONE)
    return;

#if defined(HAPTIC)
  haptic.event(index); // do this before audio to help sync timings
#endif

  if (index <= AU_ERROR) {
    if (g_eeGeneral.alarmsFlash) {
      flashCounter = FLASH_DURATION;
    }
  }

  if (g_eeGeneral.beepMode >= e_mode_nokeys || (g_eeGeneral.beepMode >= e_mode_alarms && index <= AU_ERROR)) {
    char filename[AUDIO_FILENAME_MAXLEN + 1];
    if (index < AU_SPECIAL_SOUND_FIRST && isAudioFileReferenced(index, filename)) {
      audioQueue.stopPlay(ID_PLAY_PROMPT_BASE + index);
      audioQueue.playFile(filename, 0, ID_PLAY_PROMPT_BASE + index);
      return;
    }
    switch (index) {
      case AU_INACTIVITY:
        audioQueue.playTone(2250, 80, 20, PLAY_REPEAT(2));
        break;
      case AU_TX_BATTERY_LOW:
        audioQueue.playTone(1950, 160, 20, PLAY_REPEAT(2), 1);
        audioQueue.playTone(2550, 160, 20, PLAY_REPEAT(2), -1);
        break;
      case AU_THROTTLE_ALERT:
      case AU_SWITCH_ALERT:
      case AU_ERROR:
        audioQueue.playTone(BEEP_DEFAULT_FREQ, 200, 20, PLAY_NOW);
        break;
      case AU_TRIM_MIDDLE:
        audioQueue.playTone(120*16, 80, 20, PLAY_NOW);
        break;
      case AU_TRIM_MIN:
        audioQueue.playTone(TRIM_MIN*8 + 120*16, 80, 20, PLAY_NOW);
        break;
      case AU_TRIM_MAX:
        audioQueue.playTone(TRIM_MAX*8 + 120*16, 80, 20, PLAY_NOW);
        break;
      case AU_WARNING1:
        audioQueue.playTone(BEEP_DEFAULT_FREQ, 80, 20, PLAY_NOW);
        break;
      case AU_WARNING2:
        audioQueue.playTone(BEEP_DEFAULT_FREQ, 160, 20, PLAY_NOW);
        break;
      case AU_WARNING3:
        audioQueue.playTone(BEEP_DEFAULT_FREQ, 200, 20, PLAY_NOW);
        break;
        // TODO remove all these ones
      case AU_STICK1_MIDDLE:
      case AU_STICK2_MIDDLE:
      case AU_STICK3_MIDDLE:
      case AU_STICK4_MIDDLE:
      case AU_POT1_MIDDLE:
      case AU_POT2_MIDDLE:
#if defined(PCBX9E)
      case AU_POT3_MIDDLE:
      case AU_POT4_MIDDLE:
#endif
#if defined(PCBX10)
      case AU_POT4_MIDDLE:
      case AU_POT5_MIDDLE:
      case AU_POT6_MIDDLE:
      case AU_POT7_MIDDLE:
#endif
      case AU_SLIDER1_MIDDLE:
      case AU_SLIDER2_MIDDLE:
#if defined(PCBX9E)
      case AU_SLIDER3_MIDDLE:
      case AU_SLIDER4_MIDDLE:
#endif
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1500, 80, 20, PLAY_NOW);
        break;
      case AU_MIX_WARNING_1:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1440, 48, 32);
        break;
      case AU_MIX_WARNING_2:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1560, 48, 32, PLAY_REPEAT(1));
        break;
      case AU_MIX_WARNING_3:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1680, 48, 32, PLAY_REPEAT(2));
        break;
      case AU_TIMER1_ELAPSED:
      case AU_TIMER2_ELAPSED:
      case AU_TIMER3_ELAPSED:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 150, 300, 20, PLAY_NOW);
        break;
      case AU_RSSI_ORANGE:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1500, 800, 20, PLAY_NOW);
        break;
      case AU_RSSI_RED:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1800, 800, 20, PLAY_REPEAT(1) | PLAY_NOW);
        break;
      case AU_RAS_RED:
        audioQueue.playTone(450, 160, 40, PLAY_REPEAT(2), 1);
        break;
      case AU_SPECIAL_SOUND_BEEP1:
        audioQueue.playTone(BEEP_DEFAULT_FREQ, 60, 20);
        break;
      case AU_SPECIAL_SOUND_BEEP2:
        audioQueue.playTone(BEEP_DEFAULT_FREQ, 120, 20);
        break;
      case AU_SPECIAL_SOUND_BEEP3:
        audioQueue.playTone(BEEP_DEFAULT_FREQ, 200, 20);
        break;
      case AU_SPECIAL_SOUND_WARN1:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 600, 120, 40, PLAY_REPEAT(2));
        break;
      case AU_SPECIAL_SOUND_WARN2:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 900, 120, 40, PLAY_REPEAT(2));
        break;
      case AU_SPECIAL_SOUND_CHEEP:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 900, 80, 20, PLAY_REPEAT(2), 2);
        break;
      case AU_SPECIAL_SOUND_RING:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 750, 40, 20, PLAY_REPEAT(10));
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 750, 40, 80, PLAY_REPEAT(1));
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 750, 40, 20, PLAY_REPEAT(10));
        break;
      case AU_SPECIAL_SOUND_SCIFI:
        audioQueue.playTone(2550, 80, 20, PLAY_REPEAT(2), -1);
        audioQueue.playTone(1950, 80, 20, PLAY_REPEAT(2), 1);
        audioQueue.playTone(2250, 80, 20, 0);
        break;
      case AU_SPECIAL_SOUND_ROBOT:
        audioQueue.playTone(2250, 40, 20, PLAY_REPEAT(1));
        audioQueue.playTone(1650, 120, 20, PLAY_REPEAT(1));
        audioQueue.playTone(2550, 120, 20, PLAY_REPEAT(1));
        break;
      case AU_SPECIAL_SOUND_CHIRP:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1200, 40, 20, PLAY_REPEAT(2));
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1620, 40, 20, PLAY_REPEAT(3));
        break;
      case AU_SPECIAL_SOUND_TADA:
        audioQueue.playTone(1650, 80, 40);
        audioQueue.playTone(2850, 80, 40);
        audioQueue.playTone(3450, 64, 36, PLAY_REPEAT(2));
        break;
      case AU_SPECIAL_SOUND_CRICKET:
        audioQueue.playTone(2550, 40, 80, PLAY_REPEAT(3));
        audioQueue.playTone(2550, 40, 160, PLAY_REPEAT(1));
        audioQueue.playTone(2550, 40, 80, PLAY_REPEAT(3));
        break;
      case AU_SPECIAL_SOUND_SIREN:
        audioQueue.playTone(450, 160, 40, PLAY_REPEAT(2), 2);
        break;
      case AU_SPECIAL_SOUND_ALARMC:
        audioQueue.playTone(1650, 32, 68, PLAY_REPEAT(2));
        audioQueue.playTone(2250, 64, 156, PLAY_REPEAT(1));
        audioQueue.playTone(1650, 64, 76, PLAY_REPEAT(2));
        audioQueue.playTone(2250, 32, 168, PLAY_REPEAT(1));
        break;
      case AU_SPECIAL_SOUND_RATATA:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1500, 40, 80, PLAY_REPEAT(10));
        break;
      case AU_SPECIAL_SOUND_TICK:
        audioQueue.playTone(BEEP_DEFAULT_FREQ + 1500, 40, 400, PLAY_REPEAT(2));
        break;
      default:
        break;
    }
  }
}

void pushUnit(uint8_t unit, uint8_t idx, uint8_t id, uint8_t fragmentVolume)
{
  if (unit < DIM(unitsFilenames)) {
    char path[AUDIO_FILENAME_MAXLEN+1];
    char * tmp = strAppendSystemAudioPath(path);
    tmp = strAppendStringWithIndex(tmp, unitsFilenames[unit], idx);
    strcpy(tmp, SOUNDS_EXT);
    audioQueue.playFile(path, 0, id, fragmentVolume);
  }
  else {
    TRACE("pushUnit: out of bounds unit : %d", unit); // We should never get here, but given the nature of TTS files, this prevent segfault in case of bug there.
  }
}

void pushPrompt(uint16_t prompt, uint8_t id, uint8_t fragmentVolume)
{
  char filename[AUDIO_FILENAME_MAXLEN+1];
  char * str = strAppendSystemAudioPath(filename);
  strcpy(str, "0000" SOUNDS_EXT);
  for (int8_t i=3; i>=0; i--) {
    str[i] = '0' + (prompt%10);
    prompt /= 10;
  }
  audioQueue.playFile(filename, 0, id, fragmentVolume);
}

void onKeyError()
{
  audioKeyError();
}
