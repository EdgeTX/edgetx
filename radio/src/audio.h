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

#include <stddef.h>
#include <string.h>

#include "ff.h"
#include "edgetx_types.h"
#include "dataconstants.h"

#include "hal/audio_driver.h"

/*
  Implements a bit field, number of bits is set by the template,
  each bit can be modified and read by the provided methods.
*/
template <unsigned int NUM_BITS> class BitField
{
  private:
    uint8_t bits[(NUM_BITS + 7) / 8];
  public:
    BitField()
    {
      reset();
    }

    void reset()
    {
      memset(bits, 0, sizeof(bits));
    }

    void setBit(unsigned int bitNo)
    {
      if (bitNo >= NUM_BITS) return;
      bits[bitNo >> 3] = bits[bitNo >> 3] | (1 << (bitNo & 0x07));
    }

    bool getBit(unsigned int bitNo) const
    {
      if (bitNo >= NUM_BITS) return false;
      return bits[bitNo >> 3] & (1 << (bitNo & 0x07));
    }

    unsigned int getSize() const
    {
      return NUM_BITS;
    }
};

#define INDEX_LOGICAL_SWITCH_AUDIO_FILE(index, event) (2*(index)+(event))
#define INDEX_PHASE_AUDIO_FILE(index, event)          (2*(index)+(event))

// max length (example: /SOUNDS/fr/123456789012/1234567890-off.wav)
constexpr uint8_t AUDIO_MODEL_FILENAME_MAXLEN = (sizeof("/SOUNDS/fr/") - 1) + LEN_MODEL_NAME + 1 + LEN_FLIGHT_MODE_NAME + (sizeof("-off.wav") - 1);
constexpr uint8_t AUDIO_LUA_FILENAME_MAXLEN = 42; // Some scripts use long audio paths, even on 128x64 boards
constexpr uint8_t AUDIO_FILENAME_MAXLEN = (AUDIO_LUA_FILENAME_MAXLEN > AUDIO_MODEL_FILENAME_MAXLEN ? AUDIO_LUA_FILENAME_MAXLEN : AUDIO_MODEL_FILENAME_MAXLEN);

#define AUDIO_QUEUE_LENGTH             (16) // must be a power of 2!

#define AUDIO_BUFFER_DURATION          (10)
#define AUDIO_BUFFER_SIZE              (AUDIO_SAMPLE_RATE*AUDIO_BUFFER_DURATION/1000)

#if !defined(AUDIO_SAMPLE_FMT)
  #if defined(SIMU) || defined(AUDIO_SPI)
    #define AUDIO_SAMPLE_FMT AUDIO_SAMPLE_FMT_S16
  #else
    #define AUDIO_SAMPLE_FMT AUDIO_SAMPLE_FMT_U16
  #endif
#endif

#if defined(SIMU) && defined(SIMU_AUDIO)
  #define AUDIO_BUFFER_COUNT           (10) // simulator needs more buffers for smooth audio
#elif defined(AUDIO_SPI)
  #define AUDIO_BUFFER_COUNT           (2)  // smaller than Taranis since there is also a buffer on the ADC chip
#elif defined(STORAGE_USE_SPI_FLASH)
  #define AUDIO_BUFFER_COUNT           (10) // SPI Flash need more buffer for smooth audio
#else
  #define AUDIO_BUFFER_COUNT           (3)
#endif

#define BEEP_MIN_FREQ                  (150)
#define BEEP_MAX_FREQ                  (15000)
#define BEEP_DEFAULT_FREQ              (2250)

#define USE_SETTINGS_VOLUME            (127)

#if AUDIO_SAMPLE_FMT == AUDIO_SAMPLE_FMT_S16
  typedef int16_t audio_data_t;
  #define AUDIO_DATA_SILENCE 0
#elif AUDIO_SAMPLE_FMT == AUDIO_SAMPLE_FMT_U16
  typedef uint16_t audio_data_t;
  #define AUDIO_DATA_SILENCE 0x8000 
#else
  #error "Unknown audio sample format"
#endif

struct AudioBuffer {
  audio_data_t data[AUDIO_BUFFER_SIZE];
  uint16_t size;
};

extern AudioBuffer audioBuffers[AUDIO_BUFFER_COUNT];

enum FragmentTypes {
  FRAGMENT_EMPTY,
  FRAGMENT_TONE,
  FRAGMENT_FILE,
};

struct Tone {
  uint16_t freq;
  uint16_t duration;
  uint16_t pause;
  int8_t   freqIncr;
  uint8_t  reset;
  Tone() {};
  Tone(uint16_t freq, uint16_t duration, uint16_t pause, int8_t freqIncr, bool reset):
    freq(freq),
    duration(duration),
    pause(pause),
    freqIncr(freqIncr),
    reset(reset)
  {};
};

struct AudioFragment {
  uint8_t type;
  uint8_t id;
  uint8_t repeat;
  int8_t fragmentVolume;
  union {
    Tone tone;
    char file[AUDIO_FILENAME_MAXLEN+1];
  };

  AudioFragment() { clear(); };

  AudioFragment(uint16_t freq, uint16_t duration, uint16_t pause, uint8_t repeat, int8_t freqIncr, bool reset, int8_t fragmentVolume, uint8_t id=0 ):
    type(FRAGMENT_TONE),
    id(id),
    repeat(repeat),
    fragmentVolume(fragmentVolume),
    tone(freq, duration, pause, freqIncr, reset)
  {};

  AudioFragment(const char * filename, uint8_t repeat, int8_t fragmentVolume, uint8_t id = 0):
    type(FRAGMENT_FILE),
    id(id),
    repeat(repeat),
    fragmentVolume(fragmentVolume)
  {
    strcpy(file, filename);
  }

  void clear()
  {
    memset(reinterpret_cast<void*>(this), 0, sizeof(AudioFragment));

    this->fragmentVolume = USE_SETTINGS_VOLUME;
  }
};

class ToneContext {
  public:

    inline void clear()
    {
      memset(reinterpret_cast<void*>(this), 0, sizeof(ToneContext));

      fragment.fragmentVolume = USE_SETTINGS_VOLUME;
    }

    bool isFree() const
    {
      return fragment.type == FRAGMENT_EMPTY;
    }

    int mixBuffer(AudioBuffer *buffer, int volume, unsigned int fade);

    void setFragment(uint16_t freq, uint16_t duration, uint16_t pause, uint8_t repeat, int8_t freqIncr, bool reset, int8_t fragmentVolume, uint8_t id = 0)
    {
      fragment = AudioFragment(freq, duration, pause, repeat, freqIncr, reset, fragmentVolume, id);
    }

  private:
    AudioFragment fragment;

    struct {
      float step;
      float idx;
      float  volume;
      uint16_t freq;
      uint16_t duration;
      uint16_t pause;
    } state;

};

class WavContext {
  public:

    inline void clear() { fragment.clear(); };

    int mixBuffer(AudioBuffer *buffer, int volume, unsigned int fade);
    bool hasPromptId(uint8_t id) const { return fragment.id == id; };

    void setFragment(const char * filename, uint8_t repeat, int8_t fragmentVolume, uint8_t id)
    {
      fragment = AudioFragment(filename, repeat, fragmentVolume, id);
    }

    void stop(uint8_t id)
    {
      if (fragment.id == id) {
        fragment.clear();
      }
    }

  private:
    AudioFragment fragment;

    struct {
      FIL      file;
      uint8_t  codec;
      uint32_t freq;
      uint32_t size;
      uint8_t  resampleRatio;
      uint16_t readSize;
    } state;
};

class MixedContext {
#if defined(CLI)
  friend void printAudioVars();
#endif
  public:

    MixedContext()
    {
      clear();
    }

    void setFragment(const AudioFragment * frag)
    {
      if (frag) {
        fragment = *frag;
      }
    }

    inline void clear()
    {
      tone.clear();   // the biggest member of the uninon
    }

    bool isEmpty() const { return fragment.type == FRAGMENT_EMPTY; };
    bool isTone() const { return fragment.type == FRAGMENT_TONE; };
    bool isFile() const { return fragment.type == FRAGMENT_FILE; };
    bool hasPromptId(uint8_t id) const { return fragment.id == id; };

    int mixBuffer(AudioBuffer *buffer, int toneVolume, int wavVolume, unsigned int fade)
    {
      if (isTone())
        return tone.mixBuffer(buffer, toneVolume, fade);
      else if (isFile())
        return wav.mixBuffer(buffer, wavVolume, fade);
      return 0;
    }

  private:
    union {
      AudioFragment fragment;   // a hack: fragment is used to access the fragment members of tone and wav
      ToneContext tone;
      WavContext wav;
    };
};

class AudioBufferFifo {
#if defined(CLI)
  friend void printAudioVars();
#endif

  private:
    volatile uint8_t readIdx;
    volatile uint8_t writeIdx;

    inline uint8_t nextBufferIdx(uint8_t idx) const
    {
      return (idx >= AUDIO_BUFFER_COUNT - 1 ? 0 : idx + 1);
    }

    bool full() const { return readIdx == nextBufferIdx(writeIdx); }
    bool empty() const { return readIdx == writeIdx; }
    uint8_t used() const { return (writeIdx - readIdx) % AUDIO_BUFFER_COUNT; }

   public:
    AudioBufferFifo() : readIdx(0), writeIdx(0)
    {
      memset(audioBuffers, 0, sizeof(audioBuffers));
    }

    // returns an empty buffer to be filled with data and put back into FIFO
    // with audioPushBuffer()
    AudioBuffer *getEmptyBuffer() const
    {
      return full() ? nullptr : &audioBuffers[writeIdx];
    }

    // puts filled buffer into FIFO
    void audioPushBuffer() { writeIdx = nextBufferIdx(writeIdx); }

    // frees the last played buffer
    void freeNextFilledBuffer() { readIdx = nextBufferIdx(readIdx); }

    // returns a pointer to the audio buffer to be played
    const AudioBuffer *getNextFilledBuffer()
    {
      return empty() ? nullptr : &audioBuffers[readIdx];
    }

    bool filledAtleast(int noBuffers) const { return used() >= noBuffers; }
};

class AudioFragmentFifo
{
#if defined(CLI)
  friend void printAudioVars();
#endif
  private:
    volatile uint8_t ridx;
    volatile uint8_t widx;
    AudioFragment fragments[AUDIO_QUEUE_LENGTH];

    uint8_t nextIdx(uint8_t idx) const
    {
      return (idx + 1) & (AUDIO_QUEUE_LENGTH - 1);
    }

  public:
    AudioFragmentFifo() : ridx(0), widx(0), fragments() {};

    bool hasPromptId(uint8_t id)
    {
      uint8_t i = ridx;
      while (i != widx) {
        AudioFragment & fragment = fragments[i];
        if (fragment.id == id) return true;
        i = nextIdx(i);
      }
      return false;
    }

    bool removePromptById(uint8_t id)
    {
      uint8_t i = ridx;
      while (i != widx) {
        AudioFragment & fragment = fragments[i];
        if (fragment.id == id) fragment.clear();
        i = nextIdx(i);
      }
      return false;
    }

    bool empty() const
    {
      return ridx == widx;
    }

    bool full() const
    {
      return ridx == nextIdx(widx);
    }

    void clear()
    {
      widx = ridx;                      // clean the queue
    }

    const AudioFragment * get()
    {
      if (!empty()) {
        const AudioFragment * result = &fragments[ridx];
        if (!fragments[ridx].repeat--) {
          // repeat is done, move to the next fragment
          ridx = nextIdx(ridx);
        }
        return result;
      }
      return nullptr;
    }

    void push(const AudioFragment & fragment)
    {
      if (!full()) {
        // TRACE("fragment %d at %d", fragment.type, widx);
        fragments[widx] = fragment;
        widx = nextIdx(widx);
      }
    }

};

class AudioQueue {

#if defined(SIMU_AUDIO)
  friend void fillAudioBuffer(void *, uint8_t *, int);
#endif
#if defined(CLI)
  friend void printAudioVars();
#endif
  public:
    AudioQueue();
    void start() { _started = true; };
    void playTone(uint16_t freq, uint16_t len, uint16_t pause=0, uint8_t flags=0, int8_t freqIncr=0, int8_t fragmentVolume = USE_SETTINGS_VOLUME);
    void playFile(const char *filename, uint8_t flags=0, uint8_t id=0, int8_t fragmentVolume = USE_SETTINGS_VOLUME);
    void stopPlay(uint8_t id);
    void stopAll();
    void flush();
    void pause(uint16_t tLen);
    void stopSD();
    bool isPlaying(uint8_t id);
    bool isEmpty() const { return fragmentsFifo.empty(); };
    void wakeup();
    bool started() const { return _started; };
#if defined(AUDIO_UNMUTE_DELAY)
    tmr10ms_t lastAudioPlayTime = 0;
#endif

    AudioBufferFifo buffersFifo;

  private:
    volatile bool _started;
    MixedContext normalContext;
    WavContext   backgroundContext;
    ToneContext  priorityContext;
    ToneContext  varioContext;
    AudioFragmentFifo fragmentsFifo;
};

extern uint8_t currentSpeakerVolume;
extern AudioQueue audioQueue;

enum {
  // IDs for special functions [0:64]
  // IDs for global functions [64:128]
  ID_PLAY_PROMPT_BASE = 128,
  ID_PLAY_FROM_SD_MANAGER = 255,
};

void codecsInit();
void audioEvent(unsigned int index);
void audioPlay(unsigned int index, uint8_t id=0);

#if defined(AUDIO) && defined(BUZZER)
  #define AUDIO_BUZZER(a, b)  do { a; b; } while(0)
#elif defined(AUDIO)
  #define AUDIO_BUZZER(a, b)  a
#else
  #define AUDIO_BUZZER(a, b)  b
#endif

void onKeyError();

void audioKeyPress();
void audioKeyError();
void audioTrimPress(int value);
void audioTimerCountdown(uint8_t timer, int value);

#if defined(AUDIO)

#define AUDIO_ERROR_MESSAGE(e)   audioEvent(e)
#define AUDIO_TIMER_MINUTE(t)    playDuration(t, 0, 0)

#define AUDIO_KEY_PRESS()        audioKeyPress()
#define AUDIO_KEY_ERROR()        audioKeyError()

#define AUDIO_HELLO()            audioPlay(AUDIO_HELLO)
#define AUDIO_BYE()              audioPlay(AU_BYE, ID_PLAY_PROMPT_BASE + AU_BYE)
#define AUDIO_WARNING1()         AUDIO_BUZZER(audioEvent(AU_WARNING1), beep(3))
#define AUDIO_WARNING2()         AUDIO_BUZZER(audioEvent(AU_WARNING2), beep(2))
#define AUDIO_TX_BATTERY_LOW()   AUDIO_BUZZER(audioEvent(AU_TX_BATTERY_LOW), beep(4))
#define AUDIO_ERROR()            AUDIO_BUZZER(audioEvent(AU_ERROR), beep(4))
#define AUDIO_TIMER_COUNTDOWN(idx, val) audioTimerCountdown(idx, val)
#define AUDIO_TIMER_ELAPSED(idx) AUDIO_BUZZER(audioEvent(AU_TIMER1_ELAPSED+idx), beep(3))
#define AUDIO_INACTIVITY()       AUDIO_BUZZER(audioEvent(AU_INACTIVITY), beep(3))
#define AUDIO_MIX_WARNING(x)     AUDIO_BUZZER(audioEvent(AU_MIX_WARNING_1+x-1), beep(1))
#define AUDIO_POT_MIDDLE(x)      AUDIO_BUZZER(audioEvent(AU_STICK1_MIDDLE+x), beep(2))
#define AUDIO_TRIM_MIDDLE()      AUDIO_BUZZER(audioEvent(AU_TRIM_MIDDLE), beep(2))
#define AUDIO_TRIM_MIN()         AUDIO_BUZZER(audioEvent(AU_TRIM_MIN), beep(2))
#define AUDIO_TRIM_MAX()         AUDIO_BUZZER(audioEvent(AU_TRIM_MAX), beep(2))
#define AUDIO_TRIM_PRESS(val)    audioTrimPress(val)
#define AUDIO_PLAY(p)            audioEvent(p)
#define AUDIO_VARIO(fq, t, p, f) audioQueue.playTone(fq, t, p, f)
#define AUDIO_RSSI_ORANGE()      audioEvent(AU_RSSI_ORANGE)
#define AUDIO_RSSI_RED()         audioEvent(AU_RSSI_RED)
#define AUDIO_RAS_RED()          audioEvent(AU_RAS_RED)
#define AUDIO_TELEMETRY_CONNECTED() audioEvent(AU_TELEMETRY_CONNECTED)
#define AUDIO_TELEMETRY_LOST()   audioEvent(AU_TELEMETRY_LOST)
#define AUDIO_TELEMETRY_BACK()   audioEvent(AU_TELEMETRY_BACK)
#define AUDIO_TRAINER_CONNECTED() audioEvent(AU_TRAINER_CONNECTED)
#define AUDIO_TRAINER_LOST()     audioEvent(AU_TRAINER_LOST)
#define AUDIO_TRAINER_BACK()     audioEvent(AU_TRAINER_BACK)

#else // AUDIO

#include "buzzer.h"

#define AUDIO_TIMER_COUNTDOWN(idx, val) 
#define AUDIO_TIMER_ELAPSED(idx) 
#define AUDIO_TRIM_MIN()
#define AUDIO_TRIM_MAX()
#define AUDIO_TRIM_PRESS(val)
#define AUDIO_VARIO(fq, t, p, f) 
#define AUDIO_RSSI_ORANGE()
#define AUDIO_RSSI_RED()
#define AUDIO_RAS_RED()
#define AUDIO_TELEMETRY_CONNECTED()
#define AUDIO_TELEMETRY_LOST()
#define AUDIO_TELEMETRY_BACK()
#define AUDIO_TRAINER_CONNECTED()
#define AUDIO_TRAINER_LOST()
#define AUDIO_TRAINER_BACK()

#endif

enum AutomaticPromptsCategories {
  SYSTEM_AUDIO_CATEGORY,
  MODEL_AUDIO_CATEGORY,
  PHASE_AUDIO_CATEGORY,
  SWITCH_AUDIO_CATEGORY,
  LOGICAL_SWITCH_AUDIO_CATEGORY,
};

enum AutomaticPromptsEvents {
  AUDIO_EVENT_OFF,
  AUDIO_EVENT_ON,
  AUDIO_EVENT_MID,
};

void pushPrompt(uint16_t prompt, uint8_t id=0, uint8_t fragmentVolume = USE_SETTINGS_VOLUME);
void pushUnit(uint8_t unit, uint8_t idx, uint8_t id, uint8_t fragmentVolume = USE_SETTINGS_VOLUME);
void playModelName();

#define I18N_PLAY_FUNCTION(lng, x, ...) void lng ## _ ## x(__VA_ARGS__, uint8_t id, int8_t fragmentVolume = USE_SETTINGS_VOLUME)
#define PUSH_NUMBER_PROMPT(p)    pushPrompt((p), id, fragmentVolume)
#define PUSH_UNIT_PROMPT(p, i)   pushUnit((p), (i), id, fragmentVolume)
#define PLAY_NUMBER(n, u, a)     playNumber((n), (u), (a), id, fragmentVolume)
#define PLAY_DURATION(d, att)    playDuration((d), (att), id, fragmentVolume)
#define PLAY_DURATION_ATT        , uint8_t flags
#define PLAY_TIME                1
#define PLAY_LONG_TIMER          2
#define LONG_TIMER_DURATION      (10 * 60)  // 10 minutes
#define IS_PLAY_TIME()           (flags & PLAY_TIME)
#define IS_PLAY_LONG_TIMER()     (flags & PLAY_LONG_TIMER)
#define IS_PLAYING(id)           audioQueue.isPlaying((id))
#define PLAY_VALUE(v, id)        playValue((v), (id), USE_SETTINGS_VOLUME)
#define PLAY_FILE(f, flags, id)  audioQueue.playFile((f), (flags), (id), USE_SETTINGS_VOLUME)
#define STOP_PLAY(id)            audioQueue.stopPlay((id))

#if defined(AUDIO)
#define AUDIO_RESET()            audioQueue.stopAll()
#define AUDIO_FLUSH()            audioQueue.flush()
#endif

#if defined(AUDIO)
  extern tmr10ms_t timeAutomaticPromptsSilence;
  void playModelEvent(uint8_t category, uint8_t index, event_t event=0);
  #define PLAY_PHASE_OFF(phase)         playModelEvent(PHASE_AUDIO_CATEGORY, phase, AUDIO_EVENT_OFF)
  #define PLAY_PHASE_ON(phase)          playModelEvent(PHASE_AUDIO_CATEGORY, phase, AUDIO_EVENT_ON)
  #define PLAY_SWITCH_MOVED(sw)         playModelEvent(SWITCH_AUDIO_CATEGORY, sw)
  #define PLAY_LOGICAL_SWITCH_OFF(sw)   playModelEvent(LOGICAL_SWITCH_AUDIO_CATEGORY, sw, AUDIO_EVENT_OFF)
  #define PLAY_LOGICAL_SWITCH_ON(sw)    playModelEvent(LOGICAL_SWITCH_AUDIO_CATEGORY, sw, AUDIO_EVENT_ON)
  #define PLAY_MODEL_NAME()             playModelName()
  #define START_SILENCE_PERIOD()        timeAutomaticPromptsSilence = get_tmr10ms()
  #define IS_SILENCE_PERIOD_ELAPSED()   (get_tmr10ms()-timeAutomaticPromptsSilence > 50)
#else
  #define PLAY_PHASE_OFF(phase)
  #define PLAY_PHASE_ON(phase)
  #define PLAY_SWITCH_MOVED(sw)
  #define PLAY_LOGICAL_SWITCH_OFF(sw)
  #define PLAY_LOGICAL_SWITCH_ON(sw)
  #define PLAY_MODEL_NAME()
  #define START_SILENCE_PERIOD()
  #define IS_SILENCE_PERIOD_ELAPSED()   true
#endif

char * getAudioPath(char * path);

void referenceSystemAudioFiles();
void referenceModelAudioFiles();

bool isAudioFileReferenced(uint32_t i, char * filename/*at least AUDIO_FILENAME_MAXLEN+1 long*/);
