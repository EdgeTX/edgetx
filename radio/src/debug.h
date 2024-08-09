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

#include <float.h>
#include "definitions.h"
#include "rtc.h"
#include "dump.h"

#define CRLF "\r\n"

#include "serial.h"

EXTERN_C(extern volatile uint32_t g_tmr10ms);

#if defined(SIMU)
  typedef void (*traceCallbackFunc)(const char * text);
  extern traceCallbackFunc traceCallback;
  EXTERN_C(void debugPrintf(const char * format, ...));
#elif defined(SEMIHOSTING)
  #include <stdio.h>
  #define debugPrintf(...) printf(__VA_ARGS__)
#elif defined(DEBUG)
  #define debugPrintf(...) do { dbgSerialPrintf(__VA_ARGS__); } while(0)
#else
  #define debugPrintf(...)
#endif

#define TRACE_TIME_FORMAT     "%dms: "
#define TRACE_TIME_VALUE      (g_tmr10ms * 10)

#define TRACE_NOCRLF(...)     debugPrintf(__VA_ARGS__)
#define TRACE(f_, ...)        debugPrintf((TRACE_TIME_FORMAT f_ CRLF), TRACE_TIME_VALUE, ##__VA_ARGS__)
#define DUMP(data, size)      dump(data, size)
#define TRACE_DEBUG(...)      debugPrintf("-D- " __VA_ARGS__)
#define TRACE_DEBUG_WP(...)   debugPrintf(__VA_ARGS__)
#define TRACE_INFO(...)       debugPrintf("-I- " __VA_ARGS__)
#define TRACE_INFO_WP(...)    debugPrintf(__VA_ARGS__)
#define TRACE_WARNING(...)    debugPrintf("-W- " __VA_ARGS__)
#define TRACE_WARNING_WP(...) debugPrintf(__VA_ARGS__)
#define TRACE_ERROR(...)      debugPrintf("-E- " __VA_ARGS__)

#if defined(DEBUG_WINDOWS)
#define TRACE_WINDOWS(f_, ...) TRACE(f_, ##__VA_ARGS__)
#define TRACE_WINDOWS_INDENT(f_, ...) debugPrintf((TRACE_TIME_FORMAT "%s" f_ CRLF), TRACE_TIME_VALUE, getIndentString().c_str(), ##__VA_ARGS__)
#else
#define TRACE_WINDOWS(...)
#define TRACE_WINDOWS_INDENT(...)
#endif

#if defined(TRACE_LUA_INTERNALS_ENABLED)
  #define TRACE_LUA_INTERNALS(f_, ...)     debugPrintf(("[LUA INT] " f_ CRLF), ##__VA_ARGS__)

  #define TRACE_LUA_INTERNALS_WITH_LINEINFO(L, f_, ...)   do { \
                                                            lua_Debug ar; \
                                                            if (lua_getstack(L, 1, &ar)) {  \
                                                              lua_getinfo(L, ">Sl", &ar); \
                                                              debugPrintf("%s:%d: ", ar.short_src, ar.currentline); \
                                                            } \
                                                            debugPrintf(("[LUA INT] " f_ CRLF), ##__VA_ARGS__); \
                                                          } while(0)
#else
  #define TRACE_LUA_INTERNALS(...)
  #define TRACE_LUA_INTERNALS_WITH_LINEINFO(L, f_, ...)
#endif

#if defined(DEBUG_YAML)
#define TRACE_YAML(f_, ...) TRACE(f_, ##__VA_ARGS__)
#else
#define TRACE_YAML(f_, ...)
#endif

#if defined(DEBUG) && !defined(SIMU)
#define TIME_MEASURE_START(id) uint16_t t0 ## id = getTmr2MHz()
#define TIME_MEASURE_STOP(id)  TRACE("Measure(" # id ") = %.1fus", float((uint16_t)(getTmr2MHz() - t0 ## id))/2)
#else
#define TIME_MEASURE_START(id)
#define TIME_MEASURE_STOP(id)
#endif

#if defined(DEBUG_TRACE_BUFFER)

#define TRACE_BUFFER_LEN  50

enum TraceEvent {
  trace_start = 1,

  sd_wait_ready = 10,
  sd_rcvr_datablock,
  sd_xmit_datablock_wait_ready,
  sd_xmit_datablock_rcvr_spi,
  sd_send_cmd_wait_ready,
  sd_send_cmd_rcvr_spi,

  sd_SD_ReadSectors = 16,
  sd_disk_read,
  sd_SD_WriteSectors,
  sd_disk_write,

  sd_disk_ioctl_CTRL_SYNC = 20,
  sd_disk_ioctl_GET_SECTOR_COUNT,
  sd_disk_ioctl_MMC_GET_CSD,
  sd_disk_ioctl_MMC_GET_CID,
  sd_disk_ioctl_MMC_GET_OCR,
  sd_disk_ioctl_MMC_GET_SDSTAT_1,
  sd_disk_ioctl_MMC_GET_SDSTAT_2,
  sd_spi_reset,
  sd_wait_read,
  sd_wait_write,
  sd_irq,

  ff_f_write_validate = 40,
  ff_f_write_flag,
  ff_f_write_clst,
  ff_f_write_sync_window,
  ff_f_write_disk_write_dirty,
  ff_f_write_clust2sect,
  ff_f_write_disk_write,
  ff_f_write_disk_read,
  ff_f_write_move_window,

  audio_getNextFilledBuffer_skip = 60,
};

struct TraceElement {
  gtime_t time;
  uint8_t time_ms;
  enum TraceEvent event;
  uint32_t data;
};

#if defined(__cplusplus)
extern "C" {
#endif
void trace_event(enum TraceEvent event, uint32_t data);
void trace_event_i(enum TraceEvent event, uint32_t data);
const struct TraceElement * getTraceElement(uint16_t idx);
void dumpTraceBuffer();
#if defined(__cplusplus)
}
#endif

#define TRACE_EVENT(condition, event, data)   if (condition) { trace_event(event, data); }
#define TRACEI_EVENT(condition, event, data)  if (condition) { trace_event_i(event, data); }

#else  // #if defined(DEBUG_TRACE_BUFFER)

#define TRACE_EVENT(condition, event, data)
#define TRACEI_EVENT(condition, event, data)

#endif // #if defined(DEBUG_TRACE_BUFFER)

#if defined(TRACE_SD_CARD)
  #define TRACE_SD_CARD_EVENT(condition, event, data)  TRACE_EVENT(condition, event, data)
#else
  #define TRACE_SD_CARD_EVENT(condition, event, data)
#endif
#if defined(TRACE_FATFS)
  #define TRACE_FATFS_EVENT(condition, event, data)  TRACE_EVENT(condition, event, data)
#else
  #define TRACE_FATFS_EVENT(condition, event, data)
#endif
#if defined(TRACE_AUDIO)
  #define TRACE_AUDIO_EVENT(condition, event, data)  TRACE_EVENT(condition, event, data)
  #define TRACEI_AUDIO_EVENT(condition, event, data) TRACEI_EVENT(condition, event, data)
#else
  #define TRACE_AUDIO_EVENT(condition, event, data)
  #define TRACEI_AUDIO_EVENT(condition, event, data)
#endif


#if defined(JITTER_MEASURE)  && defined(__cplusplus)

template<class T> class JitterMeter {
public:
  T min;
  T max;
  T measured;

  JitterMeter() : min(~(T)0), max(0), measured(0) {};

  void reset() {
    // store mesaurement
    measured = max - min;
    //reset - begin new measurement
    min = ~(T)0;
    max = 0;
  };

  void measure(T value) {
    if (value > max) max = value;
    if (value < min) min = value;
  };

  T get() const {
    return measured;
  };
};

#endif  // defined(JITTER_MEASURE)


#if defined(DEBUG_INTERRUPTS) && !defined(BOOT)

#if defined(PCBHORUS)
enum InterruptNames {
  INT_TICK,
  INT_1MS,
  INT_SER2,
  INT_TELEM_DMA,
  INT_TELEM_USART,
  INT_SDIO,
  INT_SDIO_DMA,
  INT_DMA2S7,
  INT_TIM1CC,
  INT_TIM2,
  INT_TRAINER,
  INT_BLUETOOTH,
  INT_OTG_FS,
#if defined(DEBUG_USB_INTERRUPTS)
  INT_OTG_FS_SPURIOUS,
  INT_OTG_FS_OUT_EP,
  INT_OTG_FS_IN_EP,
  INT_OTG_FS_MODEMISMATCH,
  INT_OTG_FS_WAKEUP,
  INT_OTG_FS_SUSPEND,
  INT_OTG_FS_SOF,
  INT_OTG_FS_RX_STAT,
  INT_OTG_FS_RESET,
  INT_OTG_FS_ENUM,
  INT_OTG_FS_INCOMPLETE_IN,
  INT_OTG_FS_INCOMPLETE_OUT,
  INT_OTG_FS_SESSION,
  INT_OTG_FS_OTG,
  INT_OTG_FS_RX_NOT_DEVICE,
#endif // #if defined(DEBUG_USB_INTERRUPTS)
  INT_LAST
};
#elif defined(PCBTARANIS)
enum InterruptNames {
  INT_TICK,
  INT_5MS,
  INT_AUDIO,
  INT_BLUETOOTH,
  INT_LCD,
  INT_TIM1CC,
  INT_TIM1,
  INT_TIM8,
  INT_SER2,
  INT_TELEM_DMA,
  INT_TELEM_USART,
  INT_TRAINER,
  INT_OTG_FS,
#if defined(DEBUG_USB_INTERRUPTS)
  INT_OTG_FS_SPURIOUS,
  INT_OTG_FS_OUT_EP,
  INT_OTG_FS_IN_EP,
  INT_OTG_FS_MODEMISMATCH,
  INT_OTG_FS_WAKEUP,
  INT_OTG_FS_SUSPEND,
  INT_OTG_FS_SOF,
  INT_OTG_FS_RX_STAT,
  INT_OTG_FS_RESET,
  INT_OTG_FS_ENUM,
  INT_OTG_FS_INCOMPLETE_IN,
  INT_OTG_FS_INCOMPLETE_OUT,
  INT_OTG_FS_SESSION,
  INT_OTG_FS_OTG,
  INT_OTG_FS_RX_NOT_DEVICE,
#endif // #if defined(DEBUG_USB_INTERRUPTS)
  INT_LAST
};
#endif

struct InterruptCounters
{
  uint32_t cnt[INT_LAST];
  uint32_t resetTime;
};

extern const char * const interruptNames[INT_LAST];
extern struct InterruptCounters interruptCounters;

#define DEBUG_INTERRUPT(int)    (++interruptCounters.cnt[int])

#if defined(DEBUG_USB_INTERRUPTS)
  #define DEBUG_USB_INTERRUPT(int)  DEBUG_INTERRUPT(int)
#else
  #define DEBUG_USB_INTERRUPT(int)
#endif

#else  //#if defined(DEBUG_INTERRUPTS)

#define DEBUG_INTERRUPT(int)
#define DEBUG_USB_INTERRUPT(int)

#endif //#if defined(DEBUG_INTERRUPTS)



#if defined(DEBUG_TIMERS)

#if defined(__cplusplus)
typedef uint32_t debug_timer_t;

class DebugTimer
{
private:
  debug_timer_t min;
  debug_timer_t max;
  // debug_timer_t avg;
  debug_timer_t last;   //unit 1us

  uint16_t _start_hiprec;
  uint32_t _start_loprec;

  void evalStats() {
    if (min > last) min = last;
    if (max < last) max = last;
    //todo avg
  }

public:
  DebugTimer(): min(-1), max(0), /*avg(0),*/ last(0), _start_hiprec(0), _start_loprec(0) {};

  void start();
  void stop();
  void sample() { stop(); start(); }

  void reset() { min = -1;  max = last = 0; }

  debug_timer_t getMin() const { return min; }
  debug_timer_t getMax() const { return max; }
  debug_timer_t getLast() const { return last; }
};

enum DebugTimers {
  debugTimerIntPulses,
  debugTimerIntPulsesDuration,
  debugTimerPer10ms,
  debugTimerPer10msPeriod,
  debugTimerRotEnc,
  debugTimerHaptic,
  debugTimerMixer,
  debugTimerTelemetryWakeup,
  debugTimerPerMain,
  debugTimerPerMain1,
  debugTimerGuiMain,
  debugTimerLua,
  debugTimerLcdRefreshWait,
  debugTimerLcdRefresh,
  debugTimerMenus,
  debugTimerMenuHandlers,
  debugTimerVersion,
  debugTimerSimpleMenu,
  debugTimerDrawText,
  debugTimerDrawText1,

  debugTimerGetAdc,
  debugTimerGetSwitches,
  debugTimerEvalMixes,
  debugTimerMixes10ms,

  debugTimerAdcRead,

  debugTimerMixerCalcToUsage,
  debugTimerMixerIterval,

  debugTimerAudioIterval,
  debugTimerAudioDuration,
  debugTimerAudioConsume,
  debugTimerYamlScan,

#if defined(SPACEMOUSE)
  debugTimerSpacemouseWakeup,
#endif

  DEBUG_TIMERS_COUNT
};

extern DebugTimer debugTimers[DEBUG_TIMERS_COUNT];
extern const char * const debugTimerNames[DEBUG_TIMERS_COUNT];

#endif // #if defined(__cplusplus)

#define DEBUG_TIMER_START(timer)  debugTimers[timer].start()
#define DEBUG_TIMER_STOP(timer)   debugTimers[timer].stop()
#define DEBUG_TIMER_SAMPLE(timer) debugTimers[timer].sample()


#else //#if defined(DEBUG_TIMERS)

#define DEBUG_TIMER_START(timer)
#define DEBUG_TIMER_STOP(timer)
#define DEBUG_TIMER_SAMPLE(timer)

#endif //#if defined(DEBUG_TIMERS)
