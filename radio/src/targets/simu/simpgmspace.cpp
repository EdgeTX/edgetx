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

#include "board.h"
#define SIMPGMSPC_USE_QT    0

#if defined(SIMU_AUDIO)
  #include <SDL.h>
#endif

#include "edgetx.h"
#include "simulcd.h"

#include "hal/adc_driver.h"
#include "hal/rotary_encoder.h"
#include "hal/usb_driver.h"

#include <errno.h>
#include <stdarg.h>
#include <string>

#if !defined (_MSC_VER) || defined (__GNUC__)
  #include <chrono>
  #include <sys/time.h>
#endif

int g_snapshot_idx = 0;

extern uint8_t startOptions;

char * main_thread_error = nullptr;

bool simu_shutdown = false;
bool simu_running = false;


volatile rotenc_t rotencValue = 0;
volatile uint32_t rotencDt = 0;

rotenc_t rotaryEncoderGetValue()
{
  return rotencValue / ROTARY_ENCODER_GRANULARITY;
}

// TODO: remove all STM32 defs

extern const etx_hal_adc_driver_t simu_adc_driver;

void lcdCopy(void * dest, void * src);

uint64_t simuTimerMicros(void)
{
#if SIMPGMSPC_USE_QT
  static QElapsedTimer ticker;
  if (!ticker.isValid())
    ticker.start();
  return ticker.nsecsElapsed() / 1000;

#elif defined(_MSC_VER)
  static double freqScale = 0.0;
  static LARGE_INTEGER firstTick;
  LARGE_INTEGER newTick;

  if (!freqScale) {
    LARGE_INTEGER frequency;
    // get ticks per second
    QueryPerformanceFrequency(&frequency);
    // 1us resolution
    freqScale = 1e6 / frequency.QuadPart;
    // init timer
    QueryPerformanceCounter(&firstTick);
    TRACE_SIMPGMSPACE("microsTimer() init: first tick = %llu @ %llu Hz", firstTick.QuadPart, frequency.QuadPart);
  }
  // read the timer
  QueryPerformanceCounter(&newTick);
  // compute the elapsed time
  return (newTick.QuadPart - firstTick.QuadPart) * freqScale;
#else  // GNUC
  auto now = std::chrono::steady_clock::now();
  return (uint64_t) std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
#endif
}

uint16_t getTmr16KHz()
{
  return simuTimerMicros() * 2 / 125;
}

uint16_t getTmr2MHz()
{
  return simuTimerMicros() * 2;
}

uint32_t timersGetMsTick(void)
{
  return simuTimerMicros() / 1000;
}

void simuInit()
{
#if defined(ROTARY_ENCODER_NAVIGATION)
  rotencValue = 0;
#endif

  // Init ADC driver callback
  adcInit(&simu_adc_driver);
}

bool keysStates[MAX_KEYS] = { false };
void simuSetKey(uint8_t key, bool state)
{
  // TRACE("simuSetKey(%d, %d)", key, state);
  assert(key < DIM(keysStates));
  keysStates[key] = state;
}

bool trimsStates[MAX_TRIMS * 2] = { false };
void simuSetTrim(uint8_t trim, bool state)
{
  // TRACE("simuSetTrim(%d, %d)", trim, state);
  assert(trim < DIM(trimsStates));
  trimsStates[trim] = state;
}

#if defined(SIMU_BOOTLOADER)
int bootloaderMain();
static void* bootloaderThread(void*)
{
  bootloaderMain();
  return nullptr;
}
#endif

void simuStart(bool tests, const char * sdPath, const char * settingsPath)
{
  if (simu_running)
    return;

#if !defined(COLORLCD)
  menuLevel = 0;
#endif

  startOptions = (tests ? 0 : OPENTX_START_NO_SPLASH | OPENTX_START_NO_CALIBRATION | OPENTX_START_NO_CHECKS);
  simu_shutdown = false;

  simuFatfsSetPaths(sdPath, settingsPath);

  /*
    g_tmr10ms must be non-zero otherwise some SF functions (that use this timer as a marker when it was last executed)
    will be executed twice on startup. Normal radio does not see this issue because g_tmr10ms is already a big number
    before the first call to the Special Functions. Not so in the simulator.

    There is another issue, some other function static variables depend on this value. If simulator is started
    multiple times in one Companion session, they are set to their initial values only first time the simulator
    is started. Therefore g_tmr10ms must also be set to non-zero value only the first time, then it must be left
    alone to continue from the previous simulator session value. See the issue #2446

  */
  if (g_tmr10ms == 0) {
    g_tmr10ms = 1;
  }

#if defined(RTCLOCK)
  time_t rawtime;
  struct tm * timeinfo;
  time (&rawtime);
  timeinfo = localtime (&rawtime);

  if (timeinfo != nullptr) {
    struct gtm gti;
    gti.tm_sec  = timeinfo->tm_sec;
    gti.tm_min  = timeinfo->tm_min;
    gti.tm_hour = timeinfo->tm_hour;
    gti.tm_mday = timeinfo->tm_mday;
    gti.tm_mon  = timeinfo->tm_mon;
    gti.tm_year = timeinfo->tm_year;
    gti.tm_wday = timeinfo->tm_wday;
    gti.tm_yday = timeinfo->tm_yday;
    g_rtcTime = gmktime(&gti);
  } else {
    g_rtcTime = rawtime;
  }
#endif

#if defined(SIMU_EXCEPTIONS)
  signal(SIGFPE, sig);
  signal(SIGSEGV, sig);
  try {
#endif

  // Init LCD callbacks
  lcdInit();

#if !defined(SIMU_BOOTLOADER)
  simuMain();
#else
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  struct sched_param sp;
  sp.sched_priority = SCHED_RR;
  pthread_attr_setschedparam(&attr, &sp);

  pthread_t bl_pid;
  pthread_create(&bl_pid, &attr, &bootloaderThread, nullptr);
#endif

  simu_running = true;

#if defined(SIMU_EXCEPTIONS)
  }
  catch (...) {
  }
#endif
}

extern RTOS_TASK_HANDLE mixerTaskId;
extern RTOS_TASK_HANDLE menusTaskId;

void simuStop()
{
  if (!simu_running)
    return;

  simu_shutdown = true;

  pthread_join(mixerTaskId, nullptr);
  pthread_join(menusTaskId, nullptr);

  simu_running = false;
}

struct SimulatorAudio {
  int volumeGain;
  int currentVolume;
  uint16_t leftoverData[AUDIO_BUFFER_SIZE];
  int leftoverLen;
  bool threadRunning;
  pthread_t threadPid;
} simuAudio;

bool simuIsRunning()
{
  return simu_running;
}

uint8_t simuSleep(uint32_t ms)
{
  for (uint32_t i = 0; i < ms; ++i){
    if (simu_shutdown || !simu_running)
      return 1;
    sleep(1);
  }
  return 0;
}

void audioConsumeCurrentBuffer()
{
}

void setScaledVolume(uint8_t volume)
{
  simuAudio.currentVolume = 127 * volume * simuAudio.volumeGain / VOLUME_LEVEL_MAX / 10;
  // TRACE_SIMPGMSPACE("setVolume(): in: %u, out: %u", volume, simuAudio.currentVolume);
}

void setVolume(uint8_t volume)
{
}

int32_t getVolume()
{
  return 0;
}

#if defined(SIMU_AUDIO)
void copyBuffer(uint8_t * dest, const uint16_t * buff, unsigned int samples)
{
  for(unsigned int i=0; i<samples; i++) {
    int sample = ((int32_t)(uint32_t)(buff[i]) - 0x8000);  // conversion from uint16_t
    *((uint16_t*)dest) = (int16_t)((sample * simuAudio.currentVolume)/127);
    dest += 2;
  }
}

void fillAudioBuffer(void *udata, Uint8 *stream, int len)
{
  SDL_memset(stream, 0, len);

  if (simuAudio.leftoverLen) {
    int len1 = min(len/2, simuAudio.leftoverLen);
    copyBuffer(stream, simuAudio.leftoverData, len1);
    len -= len1*2;
    stream += len1*2;
    simuAudio.leftoverLen -= len1;
    // putchar('l');
    if (simuAudio.leftoverLen) return;		// buffer fully filled
  }

  if (audioQueue.buffersFifo.filledAtleast(len/(AUDIO_BUFFER_SIZE*2)+1) ) {
    while(true) {
      const AudioBuffer * nextBuffer = audioQueue.buffersFifo.getNextFilledBuffer();
      if (nextBuffer) {
        if (len >= nextBuffer->size*2) {
          copyBuffer(stream, nextBuffer->data, nextBuffer->size);
          stream += nextBuffer->size*2;
          len -= nextBuffer->size*2;
          // putchar('+');
          audioQueue.buffersFifo.freeNextFilledBuffer();
        }
        else {
          //partial
          copyBuffer(stream, nextBuffer->data, len/2);
          simuAudio.leftoverLen = (nextBuffer->size-len/2);
          memcpy(simuAudio.leftoverData, &nextBuffer->data[len/2], simuAudio.leftoverLen*2);
          len = 0;
          // putchar('p');
          audioQueue.buffersFifo.freeNextFilledBuffer();
          break;
        }
      }
      else {
        break;
      }
    }
  }

  //fill the rest of buffer with silence
  if (len > 0) {
    SDL_memset(stream, 0x8000, len);  // make sure this is silence.
    // putchar('.');
  }
}

void * audioThread(void *)
{
  /*
    Checking here if SDL audio was initialized is wrong, because
    the SDL_CloseAudio() de-initializes it.

    if ( !SDL_WasInit(SDL_INIT_AUDIO) ) {
      fprintf(stderr, "ERROR: couldn't initialize SDL audio support\n");
      return 0;
    }
  */

  SDL_AudioSpec wanted, have;

  /* Set the audio format */
  wanted.freq = AUDIO_SAMPLE_RATE;
  wanted.format = AUDIO_S16SYS;
  wanted.channels = 1;    /* 1 = mono, 2 = stereo */
  wanted.samples = AUDIO_BUFFER_SIZE*2;  /* Good low-latency value for callback */
  wanted.callback = fillAudioBuffer;
  wanted.userdata = nullptr;

  /*
    SDL_OpenAudio() internally calls SDL_InitSubSystem(SDL_INIT_AUDIO),
    which initializes SDL Audio subsystem if necessary
  */
  if ( SDL_OpenAudio(&wanted, &have) < 0 ) {
    fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
    return nullptr;
  }
  SDL_PauseAudio(0);

  while (simuAudio.threadRunning) {
    audioQueue.wakeup();
    sleep(1);
  }
  SDL_CloseAudio();
  return nullptr;
}

void startAudioThread(int volumeGain)
{
  simuAudio.leftoverLen = 0;
  simuAudio.threadRunning = true;
  simuAudio.volumeGain = volumeGain;
  TRACE_SIMPGMSPACE("startAudioThread(%d)", volumeGain);
  setScaledVolume(VOLUME_LEVEL_DEF);

  pthread_attr_t attr;
  pthread_attr_init(&attr);
  struct sched_param sp;
  sp.sched_priority = SCHED_RR;
  pthread_attr_setschedparam(&attr, &sp);
  pthread_create(&simuAudio.threadPid, &attr, &audioThread, nullptr);
#ifdef __linux__
  pthread_setname_np(simuAudio.threadPid, "audio");
#endif
}

void stopAudioThread()
{
  simuAudio.threadRunning = false;
  pthread_join(simuAudio.threadPid, nullptr);
}
#endif // #if defined(SIMU_AUDIO)

#if !defined(COLORLCD)
void lcdSetRefVolt(uint8_t val)
{
}
#endif

#if LCD_W == 128
void lcdSetInvert(bool invert)
{
}
#endif

void boardInit()
{
}

uint32_t pwrCheck() { return simu_shutdown ? e_power_off : e_power_on; }

bool pwrPressed() { return false; }
bool pwrOffPressed()
{
#if defined(PWR_BUTTON_PRESS)
  return pwrPressed();
#else
  return !pwrPressed();
#endif
}

void pwrInit() {}
void pwrOn() {}
void pwrOff() {}

bool UNEXPECTED_SHUTDOWN() { return false; }
void SET_POWER_REASON(uint32_t value) {}

#if defined(TRIMS_EMULATE_BUTTONS)
bool trimsAsButtons = false;

void setHatsAsKeys(bool val) { trimsAsButtons = val; }

bool getHatsAsKeys()
{
  bool lua = false;
#if defined(LUA)
  lua = isLuaStandaloneRunning();
#endif
  return (trimsAsButtons || lua);
}
#endif

uint32_t readKeys()
{
  uint32_t result = 0;

  for (int i = 0; i < MAX_KEYS; i++) {
    if (keysStates[i]) {
      // TRACE("key pressed %d", i);
      result |= 1 << i;
    }
  }

  return result;
}

uint32_t readTrims()
{
  uint32_t trims = 0;

  for (int i = 0; i < keysGetMaxTrims() * 2; i++) {
    if (trimsStates[i]) {
      // TRACE("trim pressed %d", i);
      trims |= 1 << i;
    }
  }

  return trims;
}

int usbPlugged() { return false; }
int getSelectedUsbMode() { return USB_JOYSTICK_MODE; }
void setSelectedUsbMode(int mode) {}
void delay_ms(uint32_t ms) { }
void delay_us(uint16_t us) { }

void unlockFlash()
{
}

void lockFlash()
{
}

void flashWrite(uint32_t *address, const uint32_t *buffer)
{
  simuSleep(10);
}

uint32_t isBootloaderStart(const uint8_t * block)
{
  return 1;
}

#if defined(PCBXLITES)
bool isJackPlugged()
{
  return false;
}
#endif

void serialPrintf(const char * format, ...) { }
void serialCrlf() { }
void serialPutc(char c) { }

void boardOff()
{
}

void hapticOff() {}

#if defined(PCBFRSKY) || defined(PCBNV14)
HardwareOptions hardwareOptions;
#endif

uint32_t Master_frequency = 0;
uint32_t Current_used = 0;
uint16_t Current_max = 0;

void setSticksGain(uint8_t)
{
}

uint16_t getCurrent()
{
  return 10;
}

void calcConsumption()
{
}

#if defined(HEADPHONE_TRAINER_SWITCH_GPIO)
void enableHeadphone()
{
}

void enableTrainer()
{
}

void enableSpeaker()
{
}

void disableSpeaker()
{
}
#endif

int trainerModuleSbusGetByte(unsigned char*) { return 0; }

void rtcInit()
{
}

void rtcGetTime(struct gtm * t)
{
}

void rtcSetTime(const struct gtm * t)
{
}

#if defined(PCBTARANIS)
void sdPoll10ms() {}
#endif

uint32_t SD_GetCardType() { return 0; }

#if defined(USB_SERIAL)
const etx_serial_port_t UsbSerialPort = { "USB-VCP", nullptr, nullptr };
#endif

#if defined(AUX_SERIAL) || defined(AUX2_SERIAL)
static void* null_drv_init(void* hw_def, const etx_serial_init* dev) { return nullptr; }
static void null_drv_deinit(void* ctx) { }
static void null_drv_send_byte(void* ctx, uint8_t b) { }
static void null_drv_send_buffer(void* ctx, const uint8_t* b, uint32_t l) { }
static int null_drv_get_byte(void* ctx, uint8_t* b) { return 0; }
static void null_drv_set_baudrate(void* ctx, uint32_t baudrate) { }

const etx_serial_driver_t null_drv = {
  .init = null_drv_init,
  .deinit = null_drv_deinit,
  .sendByte = null_drv_send_byte,
  .sendBuffer = null_drv_send_buffer,
  .txCompleted = nullptr,
  .waitForTxCompleted = nullptr,
  .enableRx = nullptr,
  .getByte = null_drv_get_byte,
  .getLastByte = nullptr,
  .getBufferedBytes = nullptr,
  .copyRxBuffer = nullptr,
  .clearRxBuffer = nullptr,
  .getBaudrate = nullptr,
  .setBaudrate = null_drv_set_baudrate,
  .setPolarity = nullptr,
  .setHWOption = nullptr,
  .setReceiveCb = nullptr,
  .setIdleCb = nullptr,
  .setBaudrateCb = nullptr,
};

static void null_pwr_aux(uint8_t) {}
#endif

#if defined(AUX_SERIAL)
#if defined(AUX_SERIAL_PWR_GPIO)
  #define AUX_SERIAL_PWR null_pwr_aux
#else
  #define AUX_SERIAL_PWR nullptr
#endif
static etx_serial_port_t auxSerialPort = {
  "AUX1",
  &null_drv,
  nullptr,
  AUX_SERIAL_PWR
};
#define AUX_SERIAL_PORT &auxSerialPort
#else
#define AUX_SERIAL_PORT nullptr
#endif

#if defined(AUX2_SERIAL)
#if defined(AUX_SERIAL_PWR_GPIO)
  #define AUX2_SERIAL_PWR null_pwr_aux
#else
  #define AUX2_SERIAL_PWR nullptr
#endif
static etx_serial_port_t aux2SerialPort = {
  "AUX2",
  &null_drv,
  nullptr,
  AUX2_SERIAL_PWR
};
#define AUX2_SERIAL_PORT &aux2SerialPort
#else
#define AUX2_SERIAL_PORT nullptr
#endif // AUX2_SERIAL

etx_serial_port_t* serialPorts[MAX_AUX_SERIAL] = {
  AUX_SERIAL_PORT,
  AUX2_SERIAL_PORT,
};

const etx_serial_port_t* auxSerialGetPort(int port_nr)
{
  if (port_nr >= MAX_AUX_SERIAL) return nullptr;
  return serialPorts[port_nr];
}

#if defined(HARDWARE_TOUCH)
struct TouchState simTouchState = {};
bool simTouchOccured = false;

bool touchPanelEventOccured()
{
  if(simTouchOccured)
  {
    simTouchOccured = false;
    return true;
  }
  return false;
}

struct TouchState touchPanelRead()
{
  struct TouchState st = simTouchState;
  simTouchState.deltaX = 0;
  simTouchState.deltaY = 0;
  return st;
}

struct TouchState getInternalTouchState()
{
  return simTouchState;
}
#endif

void telemetryStart() {}
void telemetryStop() {}
