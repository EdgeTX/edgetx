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

#if defined(LIBOPENUI)
  #include "libopenui.h"
#endif

uint8_t logDelay100ms;

static FIL _log_file __DMA;
static const char* _error_msg = nullptr;

static const char* openLog();
static void writeLog();
static void writeHeader();

// static bool logFileIsValid() { return _logFile.obj.fs; }
static void logFileReset() { memset(&_log_file, 0, sizeof(_log_file)); }

static uint32_t logDelayMs() { return (uint32_t)logDelay100ms * 100; }

static void displayErrorMsg(const char* err)
{
  if (_error_msg != err) {
    _error_msg = err;
    POPUP_WARNING_ON_UI_TASK(err, nullptr, false);
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
  writeLog();
}

static int loggingTimerStart(uint32_t period)
{
  if (!loggingTimer) {
    loggingTimer = xTimerCreateStatic(
        "Logging", period / RTOS_MS_PER_TICK, pdTRUE, (void*)0,
        loggingTimerCb, &loggingTimerBuffer);
  }

  if (!loggingTimer || (xTimerStart(loggingTimer, 0) != pdPASS)) {
    return -1;
  }

  return 0;
}

void loggingTimerStop()
{
  if (loggingTimer) {
    if (xTimerStop(loggingTimer, 120 / RTOS_MS_PER_TICK) != pdPASS) {
      /* The timer could not be stopped. */
    }
  }
}

static bool loggingTimerRunning()
{
  return loggingTimer && xTimerIsTimerActive(loggingTimer);
}

static void loggingTimerSetPeriod(uint32_t period)
{
  if (xTimerChangePeriod(loggingTimer, period / RTOS_MS_PER_TICK, 0) !=
      pdPASS) {
    /* The timer period could not be changed */
  }
}

static bool loggingTimerExpired() { return true; }

#else // !SIMU

static bool logging_running = false;

static int loggingTimerStart(uint32_t period) { logging_running = true; }
static int loggingTimerStop() { logging_running = false; }
static bool loggingTimerRunning() { return logging_running; }

static void loggingTimerSetPeriod(uint32_t period) {}

static bool loggingTimerExpired()
{
  static uint32_t last_log = 0;
  uint32_t now_ms = RTOS_GET_MS();

  if (now_ms - last_log < logDelayMs()) {
    return false;
  }

  last_log = now_ms;
  return true;
}

#endif // !SIMU


static int getSwitchState(uint8_t swtch)
{
  int value = getValue(MIXSRC_FIRST_SWITCH + swtch);
  return (value == 0) ? 0 : (value < 0) ? -1 : +1;
}

static uint32_t getLogicalSwitchesStates(uint8_t first)
{
  uint32_t result = 0;
  for (uint8_t i = 0; i < 32; i++) {
    result |= (getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + first + i) << i);
  }
  return result;
}

static void writeHeader()
{
#if defined(RTCLOCK)
  f_puts("Date,Time,", &_log_file);
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
        f_puts(label, &_log_file);
      }
    }
  }

  auto n_inputs = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (uint8_t i = 0; i < n_inputs; i++) {
    const char* p = analogGetCanonicalName(ADC_INPUT_MAIN, i);
    f_puts(p, &_log_file);
    f_puts(",", &_log_file);
  }

  n_inputs = adcGetMaxInputs(ADC_INPUT_FLEX);
  for (uint8_t i = 0; i < n_inputs; i++) {
    if (!IS_POT_AVAILABLE(i)) continue;
    const char* p = analogGetCanonicalName(ADC_INPUT_FLEX, i);
    f_puts(p, &_log_file);
    f_puts(",", &_log_file);
  }

  for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
    if (SWITCH_EXISTS(i)) {
      char s[LEN_SWITCH_NAME + 2];
      char * temp;
      temp = getSwitchName(s, i);
      *temp++ = ',';
      *temp = '\0';
      f_puts(s, &_log_file);
    }
  }
  f_puts("LSW,", &_log_file);
  
  for (uint8_t channel = 0; channel < MAX_OUTPUT_CHANNELS; channel++) {
    f_printf(&_log_file, "CH%d(us),", channel+1);
  }

  f_puts("TxBat(V)\n", &_log_file);
}

static int writeLogLine()
{
#if defined(RTCLOCK)
  {
    static struct gtm utm;
    static gtime_t lastRtcTime = 0;
    if (g_rtcTime != lastRtcTime) {
      lastRtcTime = g_rtcTime;
      gettime(&utm);
    }
    f_printf(&_log_file, "%4d-%02d-%02d,%02d:%02d:%02d.%02d0,",
             utm.tm_year + TM_YEAR_BASE, utm.tm_mon + 1, utm.tm_mday,
             utm.tm_hour, utm.tm_min, utm.tm_sec, g_ms100);
  }
#else
  f_printf(&g_oLogFile, "%d,", tmr10ms);
#endif

  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isTelemetryFieldAvailable(i)) {
      TelemetrySensor& sensor = g_model.telemetrySensors[i];
      TelemetryItem telemetryItem;

      if (sensor.logs) {
        if (TELEMETRY_STREAMING() && !telemetryItems[i].isOld())
          telemetryItem = telemetryItems[i];

        if (sensor.unit == UNIT_GPS) {
          if (telemetryItem.gps.longitude && telemetryItem.gps.latitude) {
            div_t qr = div((int)telemetryItem.gps.latitude, 1000000);
            if (telemetryItem.gps.latitude < 0) f_printf(&_log_file, "-");
            f_printf(&_log_file, "%d.%06d ", abs(qr.quot), abs(qr.rem));
            qr = div((int)telemetryItem.gps.longitude, 1000000);
            if (telemetryItem.gps.longitude < 0) f_printf(&_log_file, "-");
            f_printf(&_log_file, "%d.%06d,", abs(qr.quot), abs(qr.rem));
          } else {
            f_printf(&_log_file, ",");
          }
        } else if (sensor.unit == UNIT_DATETIME) {
          f_printf(&_log_file, "%4d-%02d-%02d %02d:%02d:%02d,",
                   telemetryItem.datetime.year, telemetryItem.datetime.month,
                   telemetryItem.datetime.day, telemetryItem.datetime.hour,
                   telemetryItem.datetime.min, telemetryItem.datetime.sec);
        } else if (sensor.unit == UNIT_TEXT) {
          f_printf(&_log_file, "\"%s\",", telemetryItem.text);
        } else if (sensor.prec == 2) {
          div_t qr = div((int)telemetryItem.value, 100);
          if (telemetryItem.value < 0) f_printf(&_log_file, "-");
          f_printf(&_log_file, "%d.%02d,", abs(qr.quot), abs(qr.rem));
        } else if (sensor.prec == 1) {
          div_t qr = div((int)telemetryItem.value, 10);
          if (telemetryItem.value < 0) f_printf(&_log_file, "-");
          f_printf(&_log_file, "%d.%d,", abs(qr.quot), abs(qr.rem));
        } else {
          f_printf(&_log_file, "%d,", telemetryItem.value);
        }
      }
    }
  }

  auto n_inputs = adcGetMaxInputs(ADC_INPUT_MAIN);
  auto offset = adcGetInputOffset(ADC_INPUT_MAIN);

  for (uint8_t i = 0; i < n_inputs; i++) {
    f_printf(&_log_file, "%d,",
             calibratedAnalogs[inputMappingConvertMode(offset + i)]);
  }

  n_inputs = adcGetMaxInputs(ADC_INPUT_FLEX);
  offset = adcGetInputOffset(ADC_INPUT_FLEX);

  for (uint8_t i = 0; i < n_inputs; i++) {
    if (IS_POT_AVAILABLE(i))
      f_printf(&_log_file, "%d,", calibratedAnalogs[offset + i]);
  }

  for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
    if (SWITCH_EXISTS(i)) {
      f_printf(&_log_file, "%d,", getSwitchState(i));
    }
  }
  f_printf(&_log_file, "0x%08X%08X,", getLogicalSwitchesStates(32),
           getLogicalSwitchesStates(0));

  for (uint8_t channel = 0; channel < MAX_OUTPUT_CHANNELS; channel++) {
    f_printf(&_log_file, "%d,",
             PPM_CENTER + channelOutputs[channel] / 2);  // in us
  }

  div_t qr = div(g_vbat100mV, 10);
  return f_printf(&_log_file, "%d.%d\n", abs(qr.quot), abs(qr.rem));
}

static void writeLog()
{
  const char* err = nullptr;

  if (_error_msg || !sdMounted() || !mixerTaskRunning() ||
      !loggingTimerExpired())
    return;

  // TODO: check how long this really takes
  if (sdIsFull()) {
    err = STR_SDCARD_FULL_EXT;
  } else if (writeLogLine() != 0) {
    err = STR_SDCARD_ERROR;
  }

  // timer is left running even if openLog()
  // returned an error as we use the "running"
  // state to detect intentional start/stop
  if (err) displayErrorMsg(err);
}

static const char* openLog()
{
  if (!sdMounted()) return STR_NO_SDCARD;

  // Determine and set log file filename
  FRESULT result;

  // /LOGS/modelnamexxxxxx_YYYY-MM-DD-HHMMSS.log
  char filename[sizeof(LOGS_PATH) + LEN_MODEL_NAME +
                sizeof("_YYYY-MM-DD-HHMMSS.log") + 1];

  // check and create folder here
  char* tmp = strAppend(filename, STR_LOGS_PATH);
  const char * error = sdCheckAndCreateDirectory(filename);
  if (error) return error;

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
  tmp = strAppendDate(tmp, true);
#endif

  strAppend(tmp, STR_LOGS_EXT);

  result = f_open(&_log_file, filename, FA_OPEN_ALWAYS | FA_WRITE | FA_OPEN_APPEND);
  if (result != FR_OK) {
    return SDCARD_ERROR(result);
  }

  if (f_size(&_log_file) == 0) writeHeader();

  return nullptr;
}

static void loggingStart(uint32_t period)
{
  const char* err = nullptr;

  if (sdIsFull()) {
    err = STR_SDCARD_FULL_EXT;
  } else {
    err = openLog();
  }

  // timer is left running even if openLog()
  // returned an error as we use the "running"
  // state to detect intentional start/stop
  if (err) displayErrorMsg(err);

  // start timer anyway
  loggingTimerStart(period);
}

void logsInit() { logFileReset(); }

void logsClose()
{
  if (loggingTimerRunning()) loggingTimerStop();

  f_close(&_log_file);
  logFileReset();

  _error_msg = nullptr;
}

void logsHandle()
{
  static uint8_t current_delay = 0;

  if (sdMounted() && isFunctionActive(FUNCTION_LOGS) && (logDelay100ms > 0)) {
    if (!loggingTimerRunning()) {
      current_delay = logDelay100ms;
      loggingStart(logDelayMs());
    } else if (current_delay != logDelay100ms) {
      current_delay = logDelay100ms;
      loggingTimerSetPeriod(logDelayMs());
    }
  } else {
    logsClose();
  }
}
