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

#include "edgetx.h"
#include "ff.h"

#include "analogs.h"
#include "switches.h"
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"
#include "hal/usb_driver.h"

FIL g_oLogFile __DMA;
uint8_t logDelay100ms;
static tmr10ms_t lastLogTime = 0;

// Last error reported to the user, used only to avoid repeating the popup.
// Written from both the UI task (logsHandle) and the higher-priority timer
// task (logsWrite); the two never run concurrently on the target (the UI task
// cannot preempt the timer task) and a pointer store is atomic, so no lock is
// needed.
static const char* error_displayed = nullptr;

static void logsWrite();
const char* logsOpen();

static void displayLogError(const char* err)
{
  if (err != error_displayed) {
    error_displayed = err;
    POPUP_WARNING_ON_UI_TASK(err, nullptr);
  }
}

#if !defined(SIMU)
#include <FreeRTOS/include/FreeRTOS.h>
#include <FreeRTOS/include/timers.h>

#include "tasks/mixer_task.h"

static TimerHandle_t loggingTimer = nullptr;
static StaticTimer_t loggingTimerBuffer;

static void loggingTimerCb(TimerHandle_t xTimer)
{
  (void)xTimer;
  if (mixerTaskRunning()) {
    DEBUG_TIMER_START(debugTimerLoggingWakeup);
    logsWrite();
    DEBUG_TIMER_STOP(debugTimerLoggingWakeup);
  }
}

void loggingTimerStart()
{
  if (!loggingTimer) {
    loggingTimer =
        xTimerCreateStatic("Logging", logDelay100ms*100 / RTOS_MS_PER_TICK, pdTRUE, (void*)0,
                           loggingTimerCb, &loggingTimerBuffer);
  }

  if (loggingTimer) {
    if( xTimerStart( loggingTimer, 0 ) != pdPASS ) {
      /* The timer could not be set into the Active state. */
    }
  }
}

void loggingTimerStop()
{
  if (loggingTimer) {
    if( xTimerStop( loggingTimer, 120 / RTOS_MS_PER_TICK ) != pdPASS ) {
      /* The timer could not be stopped. */
    }
    loggingTimer = nullptr;
  }
}

#endif

void logsHandle()
{  // called cyclically by main.cpp:perMain(), i.e. from the UI task
  static uint8_t logDelay100msOld = 0;

  bool logsActive = sdMounted() && isFunctionActive(FUNCTION_LOGS) &&
                    logDelay100ms > 0 && !usbPlugged();

  if (!logsActive) {
#if !defined(SIMU)
    loggingTimerStop();
#endif
    logsClose();
    error_displayed = nullptr;
    return;
  }

  // After an error was reported (by logsWrite or a previous open), tear the
  // logging down and retry a fresh open below. This releases the file and
  // stops the no-op timer callbacks; a successful (re)open clears the error
  // and resumes logging without the user having to toggle it off and on.
  if (error_displayed) {
#if !defined(SIMU)
    loggingTimerStop();
#endif
    logsClose();
  }

  // Open the log file here, in the UI task. f_open() may have to scan a large
  // LOGS directory and can block for a long time. On firmware logsWrite() runs
  // on the timer daemon task, which is shared with telemetry frame polling, so
  // opening there would starve telemetry and cause a brief "telemetry lost".
  // See issue #7513.
  if (!g_oLogFile.obj.fs) {
    const char* result = sdIsFull() ? STR_SDCARD_FULL_EXT : logsOpen();
    if (result) {
      displayLogError(result);  // deduplicated: no repeated popup
      return;  // keep logging stopped until we have a valid file
    }
    error_displayed = nullptr;  // (re)opened successfully
  }

#if !defined(SIMU)
  if (loggingTimer == nullptr) {                                // log timer not running
    logDelay100msOld = logDelay100ms;
    loggingTimerStart();                                        // start log timer
  } else if (logDelay100msOld != logDelay100ms) {              // log rate changed
    logDelay100msOld = logDelay100ms;                           // memorize new log rate
    if (xTimerChangePeriod(loggingTimer, logDelay100ms * 100, 0) != pdPASS) {
      /* The timer period could not be changed */
    }
  }
#else
  (void)logDelay100msOld;
  logsWrite();  // SIMU: write directly from the UI task (self rate-limited)
#endif
}

void writeHeader();

int getSwitchState(uint8_t swtch) {
  int value = getValue(MIXSRC_FIRST_SWITCH + swtch);
  return (value == 0) ? 0 : (value < 0) ? -1 : +1;
}

void logsInit()
{
  memset(&g_oLogFile, 0, sizeof(g_oLogFile));
}

const char * logsOpen()
{
  if (!sdMounted())
    return STR_NO_SDCARD;

  // Determine and set log file filename
  FRESULT result;

  // /LOGS/modelnamexxxxxx_YYYY-MM-DD-HHMMSS.log
  char filename[sizeof(LOGS_PATH) + LEN_MODEL_NAME + 18 + 4 + 1];

  // check and create folder here
  char* tmp = strAppend(filename, STR_LOGS_PATH);
  const char * error = sdCheckAndCreateDirectory(filename);
  if (error) {
    return error;
  }

  tmp = strAppend(tmp, "/");
  if (g_model.header.name[0]) {
    tmp = strAppend(tmp, sanitizeForFilename(g_model.header.name, LEN_MODEL_NAME));
  } else {
    // TODO
    uint8_t num = 1;
    tmp = strAppend(tmp, STR_MODEL);
    tmp = strAppendUnsigned(tmp, num, 2);
  }

#if defined(RTCLOCK)
  if (g_eeGeneral.oneLogPerDay)
    tmp = strAppendDate(tmp);
  else
    tmp = strAppendDate(tmp, true);
#endif

  strAppend(tmp, STR_LOGS_EXT);

  result = f_open(&g_oLogFile, filename, FA_OPEN_ALWAYS | FA_WRITE | FA_OPEN_APPEND);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  if (f_size(&g_oLogFile) == 0) {
    writeHeader();
  }

  return nullptr;
}

void logsClose()
{
  if (g_oLogFile.obj.fs && sdMounted()) {
    if (f_close(&g_oLogFile) != FR_OK) {
      // close failed, forget file
      g_oLogFile.obj.fs = nullptr;
    }
    lastLogTime = 0;
  }

}

void writeHeader()
{
#if defined(RTCLOCK)
  f_puts("Date,Time,", &g_oLogFile);
#else
  f_puts("Time,", &g_oLogFile);
#endif

  char label[TELEM_LABEL_LEN+7];
  for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      TelemetrySensor & sensor = g_model.telemetrySensors[i];
      if (sensor.logs) {
        memset(label, 0, sizeof(label));
        strncpy(label, sensor.label, TELEM_LABEL_LEN);
        uint8_t unit = sensor.unit;
        if (unit == UNIT_CELLS ) unit = UNIT_VOLTS;
        if (UNIT_RAW < unit && unit < UNIT_FIRST_VIRTUAL) {
          strcat(label, "(");
          strncat(label, STR_VTELEMUNIT[unit], 3);
          strcat(label, ")");
        }
        strcat(label, ",");
        f_puts(label, &g_oLogFile);
      }
    }
  }

  auto n_inputs = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (uint8_t i = 0; i < n_inputs; i++) {
    const char* p = analogGetCanonicalName(ADC_INPUT_MAIN, i);
    f_puts(p, &g_oLogFile);
    f_puts(",", &g_oLogFile);
  }

  n_inputs = adcGetMaxInputs(ADC_INPUT_FLEX);
  for (uint8_t i = 0; i < n_inputs; i++) {
    if (!IS_POT_AVAILABLE(i)) continue;
    const char* p = analogGetCanonicalName(ADC_INPUT_FLEX, i);
    f_puts(p, &g_oLogFile);
    f_puts(",", &g_oLogFile);
  }

  for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
    if (SWITCH_EXISTS(i)) {
      char s[LEN_SWITCH_NAME + 2];
      char * temp;
      temp = getSwitchName(s, i);
      *temp++ = ',';
      *temp = '\0';
      f_puts(s, &g_oLogFile);
    }
  }
  f_puts("LSW,", &g_oLogFile);
  
  for (uint8_t channel = 0; channel < MAX_OUTPUT_CHANNELS; channel++) {
    f_printf(&g_oLogFile, "CH%d(us),", channel+1);
  }

  f_puts("TxBat(V)\n", &g_oLogFile);
}

uint32_t getLogicalSwitchesStates(uint8_t first)
{
  uint32_t result = 0;
  for (uint8_t i=0; i<32; i++) {
    result |= (getSwitch(SWSRC_FIRST_LOGICAL_SWITCH+first+i) << i);
  }
  return result;
}

static void logsWrite()
{
  // Called from the logging timer callback (timer task) on firmware, and
  // directly from logsHandle() (UI task) in the simulator. The log file is
  // opened and closed by logsHandle(), so this function only ever appends to
  // an already-open file and never blocks on f_open().

  // stop writing once an error has been reported; logsHandle() clears
  // error_displayed when logging is toggled off, which resumes logging.
  if (!sdMounted() || !g_oLogFile.obj.fs || error_displayed) {
    return;
  }

  {
    #if defined(SIMU) || !defined(RTCLOCK)
    tmr10ms_t tmr10ms = get_tmr10ms();                                        // tmr10ms works in 10ms increments
    if (lastLogTime == 0 || (tmr10ms_t)(tmr10ms - lastLogTime) >= (tmr10ms_t)(logDelay100ms*10)-1) {
      lastLogTime = tmr10ms;
    #else
    {
    #endif

      // SD card became full: report and stop writing. The file is left open
      // and closed later by logsHandle() from the UI task.
      if (sdIsFull()) {
        displayLogError(STR_SDCARD_FULL_EXT);
        return;
      }

#if defined(RTCLOCK)
      {
        static struct gtm utm;
        static gtime_t lastRtcTime = 0;
        if (g_rtcTime != lastRtcTime) {
          lastRtcTime = g_rtcTime;
          gettime(&utm);
        }
        f_printf(&g_oLogFile, "%4d-%02d-%02d,%02d:%02d:%02d.%02d0,", utm.tm_year+TM_YEAR_BASE, utm.tm_mon+1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec, g_ms100);
      }
#else
      f_printf(&g_oLogFile, "%d,", tmr10ms);
#endif

      for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
        if (isTelemetryFieldAvailable(i)) {
          TelemetrySensor & sensor = g_model.telemetrySensors[i];
          TelemetryItem telemetryItem;
          
          if (sensor.logs) {
            if(TELEMETRY_STREAMING() && !telemetryItems[i].isOld())
              telemetryItem = telemetryItems[i];

            if (sensor.unit == UNIT_GPS) {
              if (telemetryItem.gps.longitude && telemetryItem.gps.latitude) {
                div_t qr = div((int)telemetryItem.gps.latitude, 1000000);
                if (telemetryItem.gps.latitude < 0) f_printf(&g_oLogFile, "-");
                f_printf(&g_oLogFile, "%d.%06d ", abs(qr.quot), abs(qr.rem));
                qr = div((int)telemetryItem.gps.longitude, 1000000);
                if (telemetryItem.gps.longitude < 0) f_printf(&g_oLogFile, "-");
                f_printf(&g_oLogFile, "%d.%06d,", abs(qr.quot), abs(qr.rem));
              }
              else {
                f_printf(&g_oLogFile, ",");
              }
            }
            else if (sensor.unit == UNIT_DATETIME) {
              f_printf(&g_oLogFile, "%4d-%02d-%02d %02d:%02d:%02d,", telemetryItem.datetime.year, telemetryItem.datetime.month, telemetryItem.datetime.day, telemetryItem.datetime.hour, telemetryItem.datetime.min, telemetryItem.datetime.sec);
            }
            else if (sensor.unit == UNIT_TEXT) {
              f_printf(&g_oLogFile, "\"%s\",", telemetryItem.text);
            }
            else if (sensor.prec == 2) {
              div_t qr = div((int)telemetryItem.value, 100);
              if (telemetryItem.value < 0) f_printf(&g_oLogFile, "-");
              f_printf(&g_oLogFile, "%d.%02d,", abs(qr.quot), abs(qr.rem));
            }
            else if (sensor.prec == 1) {
              div_t qr = div((int)telemetryItem.value, 10);
              if (telemetryItem.value < 0) f_printf(&g_oLogFile, "-");
              f_printf(&g_oLogFile, "%d.%d,", abs(qr.quot), abs(qr.rem));
            }
            else {
              f_printf(&g_oLogFile, "%d,", telemetryItem.value);
            }
          }
        }
      }

      auto n_inputs = adcGetMaxInputs(ADC_INPUT_MAIN);
      auto offset = adcGetInputOffset(ADC_INPUT_MAIN);

      for (uint8_t i = 0; i < n_inputs; i++) {
        f_printf(&g_oLogFile, "%d,", calibratedAnalogs[inputMappingConvertMode(offset + i)]);
      }

      n_inputs = adcGetMaxInputs(ADC_INPUT_FLEX);
      offset = adcGetInputOffset(ADC_INPUT_FLEX);

      for (uint8_t i = 0; i < n_inputs; i++) {
        if (IS_POT_AVAILABLE(i))
          f_printf(&g_oLogFile, "%d,", calibratedAnalogs[offset + i]);
      }

      for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
        if (SWITCH_EXISTS(i)) {
          f_printf(&g_oLogFile, "%d,", getSwitchState(i));
        }
      }
      f_printf(&g_oLogFile, "0x%08X%08X,", getLogicalSwitchesStates(32),
               getLogicalSwitchesStates(0));

      for (uint8_t channel = 0; channel < MAX_OUTPUT_CHANNELS; channel++) {
        f_printf(&g_oLogFile, "%d,", PPM_CENTER+channelOutputs[channel]/2); // in us
      }

      div_t qr = div(g_vbat100mV, 10);
      int result = f_printf(&g_oLogFile, "%d.%d\n", abs(qr.quot), abs(qr.rem));

      // Write error: report it and stop writing. The file is left open and
      // closed later by logsHandle() from the UI task.
      if (result < 0) {
        displayLogError(STR_SDCARD_ERROR);
      }
    }
  }
}
