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

#if defined(LIBOPENUI)
  #include "libopenui.h"
#endif

FIL g_oLogFile __DMA;
uint8_t logDelay100ms;
static tmr10ms_t lastLogTime = 0;

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

void initLoggingTimer() {                                       // called cyclically by main.cpp:perMain()
  static uint8_t logDelay100msOld = 0;

  if(loggingTimer == nullptr) {                                 // log Timer not running
    if(isFunctionActive(FUNCTION_LOGS) && logDelay100ms > 0) {  // if SF Logging is active and log rate is valid
      loggingTimerStart();                                      // start log timer
    }  
  } else {                                                      // log timer is already running
    if(logDelay100msOld != logDelay100ms) {                     // if log rate was changed
      logDelay100msOld = logDelay100ms;                         // memorize new log rate

      if(logDelay100ms > 0) {
        if(xTimerChangePeriod( loggingTimer, logDelay100ms*100, 0 ) != pdPASS ) {  // and restart timer with new log rate
          /* The timer period could not be changed */
        }
      }
    }
  }
}
#endif

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

void logsWrite()
{
  static const char * error_displayed = nullptr;

  if (!sdMounted()) {
    return;
  }

  if (isFunctionActive(FUNCTION_LOGS) && logDelay100ms > 0 && !usbPlugged()) {
    #if defined(SIMU) || !defined(RTCLOCK)
    tmr10ms_t tmr10ms = get_tmr10ms();                                        // tmr10ms works in 10ms increments
    if (lastLogTime == 0 || (tmr10ms_t)(tmr10ms - lastLogTime) >= (tmr10ms_t)(logDelay100ms*10)-1) {
      lastLogTime = tmr10ms;
    #else
    {
    #endif

      bool sdCardFull = sdIsFull();

      // check if file needs to be opened
      if (!g_oLogFile.obj.fs) {
        const char *result = sdCardFull ? STR_SDCARD_FULL_EXT : logsOpen();

        // SD card is full or file open failed
        if (result) {
          if (result != error_displayed) {
            error_displayed = result;
            POPUP_WARNING_ON_UI_TASK(result, nullptr, false);
          }
          return;
        }
      }

      // check at every write cycle
      if (sdCardFull) {
        logsClose();  // timer is still running and code above will try to
                      // open the file again but will fail with error
                      // which will trigger the warning popup
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

      if (result<0 && !error_displayed) {
        error_displayed = STR_SDCARD_ERROR;
        POPUP_WARNING_ON_UI_TASK(STR_SDCARD_ERROR, nullptr, false);
        logsClose();
      }
    }
  }
  else {
    error_displayed = nullptr;
    logsClose();
    
    #if !defined(SIMU)
    loggingTimerStop();
    #endif
  }
}
