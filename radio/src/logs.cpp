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

#include "opentx.h"
#include "VirtualFS.h"

#include "analogs.h"
#include "switches.h"
#include "hal/adc_driver.h"
#include "hal/switch_driver.h"

#include "logs.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#endif

VfsFile g_oLogFile __DMA;
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
  g_oLogFile.close();
}

const char * logsOpen()
{
  // Determine and set log file filename
  VfsError result;
  VirtualFS& vfs = VirtualFS::instance();

  // /LOGS/modelnamexxxxxx_YYYY-MM-DD-HHMMSS.log
  char filename[sizeof(LOGS_PATH) + LEN_MODEL_NAME + 18 + 4 + 1];

  if (!VirtualFS::instance().sdCardMounted())
    return STR_NO_SDCARD;

  // check and create folder here
  strcpy(filename, STR_LOGS_PATH);
  const char * error = vfs.checkAndCreateDirectory(filename);
  if (error) {
    return error;
  }

  filename[sizeof(LOGS_PATH) - 1] = '/';
  memcpy(&filename[sizeof(LOGS_PATH)], g_model.header.name, sizeof(g_model.header.name));
  filename[sizeof(LOGS_PATH) + LEN_MODEL_NAME] = '\0';

  uint8_t i = sizeof(LOGS_PATH) + LEN_MODEL_NAME - 1;
  uint8_t len = 0;
  while (i > sizeof(LOGS_PATH) - 1) {
    if (!len && filename[i])
      len = i+1;
    if (len) {
      if (!filename[i])
        filename[i] = '_';
    }
    i--;
  }

  if (len == 0) {
#if defined(EEPROM)
    uint8_t num = g_eeGeneral.currModel + 1;
#else
    // TODO
    uint8_t num = 1;
#endif
    strcpy(&filename[sizeof(LOGS_PATH)], STR_MODEL);
    filename[sizeof(LOGS_PATH) + PSIZE(TR_MODEL)] = (char)((num / 10) + '0');
    filename[sizeof(LOGS_PATH) + PSIZE(TR_MODEL) + 1] = (char)((num % 10) + '0');
    len = sizeof(LOGS_PATH) + PSIZE(TR_MODEL) + 2;
  }

  char * tmp = &filename[len];

#if defined(RTCLOCK)
  tmp = strAppendDate(tmp, true);
#endif

  strcpy(tmp, STR_LOGS_EXT);

  result = vfs.openFile(g_oLogFile, filename, VfsOpenFlags::OPEN_ALWAYS | VfsOpenFlags::WRITE | VfsOpenFlags::OPEN_APPEND);
  if (result != VfsError::OK) {
    return STORAGE_ERROR(result);
  }

  if (g_oLogFile.size() == 0) {
    writeHeader();
  }

  return nullptr;
}

void logsClose()
{
  if (g_oLogFile.isOpen() && VirtualFS::instance().sdCardMounted()) {
    g_oLogFile.close();
    lastLogTime = 0;
  }

}

void writeHeader()
{
#if defined(RTCLOCK)
  g_oLogFile.puts("Date,Time,");
#else
  g_oLogFile.puts("Time,");
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
        g_oLogFile.puts(label);
      }
    }
  }

  auto n_inputs = adcGetMaxInputs(ADC_INPUT_MAIN);
  for (uint8_t i = 0; i < n_inputs; i++) {
    const char* p = analogGetCanonicalName(ADC_INPUT_MAIN, i);
    while (*p) { g_oLogFile.putc(*(p++)); }
    g_oLogFile.putc(',');
  }

  n_inputs = adcGetMaxInputs(ADC_INPUT_POT);
  for (uint8_t i = 0; i < n_inputs; i++) {
    if (!IS_POT_AVAILABLE(i)) continue;
    const char* p = analogGetCanonicalName(ADC_INPUT_POT, i);
    while (*p) { f_putc(*(p++), &g_oLogFile); }
    f_putc(',', &g_oLogFile);
  }

  for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
    if (SWITCH_EXISTS(i)) {
      char s[LEN_SWITCH_NAME + 2];
      char * temp;
      temp = getSwitchName(s, i);
      *temp++ = ',';
      *temp = '\0';
      g_oLogFile.puts(s);
    }
  }
  g_oLogFile.puts("LSW,");
  
  for (uint8_t channel = 0; channel < MAX_OUTPUT_CHANNELS; channel++) {
    g_oLogFile.fprintf("CH%d(us),", channel+1);
  }

  g_oLogFile.puts("TxBat(V)\n");
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

  if (!VirtualFS::instance().sdCardMounted()) {
    return;
  }

  if (isFunctionActive(FUNCTION_LOGS) && logDelay100ms > 0) {
    #if defined(SIMU) || !defined(RTCLOCK)
    tmr10ms_t tmr10ms = get_tmr10ms();                                        // tmr10ms works in 10ms increments
    if (lastLogTime == 0 || (tmr10ms_t)(tmr10ms - lastLogTime) >= (tmr10ms_t)(logDelay100ms*10)-1) {
      lastLogTime = tmr10ms;
    #else
    {
    #endif

      bool sdCardFull = sdIsFull();

      // check if file needs to be opened
      if (!g_oLogFile.isOpen()) {
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
        g_oLogFile.fprintf("%4d-%02d-%02d,%02d:%02d:%02d.%02d0,", utm.tm_year+TM_YEAR_BASE, utm.tm_mon+1, utm.tm_mday, utm.tm_hour, utm.tm_min, utm.tm_sec, g_ms100);
      }
#else
      g_oLogFile.fprintf("%d,", tmr10ms);
#endif

      for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
        if (isTelemetryFieldAvailable(i)) {
          TelemetrySensor & sensor = g_model.telemetrySensors[i];
          TelemetryItem & telemetryItem = telemetryItems[i];
          if (sensor.logs) {
            if (sensor.unit == UNIT_GPS) {
              if (telemetryItem.gps.longitude && telemetryItem.gps.latitude) {
                div_t qr = div((int)telemetryItem.gps.latitude, 1000000);
                if (telemetryItem.gps.latitude < 0) g_oLogFile.fprintf("-");
                g_oLogFile.fprintf("%d.%06d ", abs(qr.quot), abs(qr.rem));
                qr = div((int)telemetryItem.gps.longitude, 1000000);
                if (telemetryItem.gps.longitude < 0) g_oLogFile.fprintf("-");
                g_oLogFile.fprintf("%d.%06d,", abs(qr.quot), abs(qr.rem));
              }
              else {
                g_oLogFile.fprintf(",");
              }
            }
            else if (sensor.unit == UNIT_DATETIME) {
              g_oLogFile.fprintf("%4d-%02d-%02d %02d:%02d:%02d,", telemetryItem.datetime.year, telemetryItem.datetime.month, telemetryItem.datetime.day, telemetryItem.datetime.hour, telemetryItem.datetime.min, telemetryItem.datetime.sec);
            }
            else if (sensor.unit == UNIT_TEXT) {
              g_oLogFile.fprintf("\"%s\",", telemetryItem.text);
            }
            else if (sensor.prec == 2) {
              div_t qr = div((int)telemetryItem.value, 100);
              if (telemetryItem.value < 0) g_oLogFile.fprintf("-");
              g_oLogFile.fprintf("%d.%02d,", abs(qr.quot), abs(qr.rem));
            }
            else if (sensor.prec == 1) {
              div_t qr = div((int)telemetryItem.value, 10);
              if (telemetryItem.value < 0) g_oLogFile.fprintf("-");
              g_oLogFile.fprintf("%d.%d,", abs(qr.quot), abs(qr.rem));
            }
            else {
              g_oLogFile.fprintf("%d,", telemetryItem.value);
            }
          }
        }
      }

      auto n_inputs = adcGetMaxInputs(ADC_INPUT_MAIN);
      auto offset = adcGetInputOffset(ADC_INPUT_MAIN);

      for (uint8_t i = 0; i < n_inputs; i++) {
        g_oLogFile.fprint("%d,", calibratedAnalogs[inputMappingConvertMode(offset + i)]);
      }

      n_inputs = adcGetMaxInputs(ADC_INPUT_POT);
      offset = adcGetInputOffset(ADC_INPUT_POT);

      for (uint8_t i = 0; i < n_inputs; i++) {
        if (IS_POT_AVAILABLE(i))
          g_oLogFile.fprint("%d,", calibratedAnalogs[offset + i]);
      }

      for (uint8_t i = 0; i < switchGetMaxSwitches(); i++) {
        if (SWITCH_EXISTS(i)) {
          g_oLogFile.fprintf("%d,", getSwitchState(i));
        }
      }
      g_oLogFile.fprintf("0x%08X%08X,", getLogicalSwitchesStates(32), getLogicalSwitchesStates(0));

      for (uint8_t channel = 0; channel < MAX_OUTPUT_CHANNELS; channel++) {
        g_oLogFile.fprintf("%d,", PPM_CENTER+channelOutputs[channel]/2); // in us
      }

      div_t qr = div(g_vbat100mV, 10);
      int result = g_oLogFile.fprintf("%d.%d\n", abs(qr.quot), abs(qr.rem));

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
