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
#include "hal/adc_driver.h"
#if defined(RADIO_MAMBO)
#include "targets/mambo/diagnostics.h"
#include "tasks.h"
#include "tasks/mixer_task.h"
#include "timers_driver.h"
#if defined(MAMBO_EXTERNAL_CRSF)
#include "hal/module_port.h"
#include "targets/mambo/external_rf.h"
#endif

static const char* mamboCalibrationLabel(MamboCalibrationStatus status)
{
  switch (status) {
    case MamboCalibrationStatus::Loaded:
      return "SD OK";
    case MamboCalibrationStatus::Invalid:
      return "INVALID";
    default:
      return "MISSING";
  }
}

static void drawMamboHall(unsigned sensor)
{
  auto data = mamboBenchDiagnostics();
  const auto& sample = data.sensors[sensor];
  const char* health = "WAIT";
  switch (mambo::hallHealth(sample, timersGetUsTick())) {
    case mambo::HallHealth::Fresh:
      health = "OK";
      break;
    case mambo::HallHealth::Stale:
      health = "OLD";
      break;
    case mambo::HallHealth::Error:
      health = "ERR";
      break;
    default:
      break;
  }
  coord_t y = MENU_HEADER_HEIGHT + 1;
  lcdDrawText(0, y, "Cal:");
  lcdDrawText(24, y, mamboCalibrationLabel(data.calibration));
  lcdDrawText(96, y, health);
  const int32_t values[3] = {sample.x, sample.y, sample.z};
  const char* labels[3] = {"X:", "Y:", "Z:"};
  for (unsigned i = 0; i < 3; ++i) {
    y += FH;
    lcdDrawText(0, y, labels[i]);
    lcdDrawNumber(60, y, values[i], RIGHT);
    if (i == 0) {
      lcdDrawText(66, y, "Age");
      uint32_t age = (sample.count || sample.valid)
                         ? uint32_t(timersGetUsTick() - sample.timestamp) / 1000
                         : 0;
      lcdDrawNumber(128, y, age > 9999 ? 9999 : age, RIGHT);
    } else if (i == 1) {
      lcdDrawText(66, y, "Err");
      lcdDrawNumber(128, y, sample.errors > 9999 ? 9999 : sample.errors, RIGHT);
    } else {
      lcdDrawText(66, y, "St");
      lcdDrawHexNumber(99, y, sample.status);
    }
  }
  y += FH;
  lcdDrawText(0, y, "S:");
  lcdDrawNumber(60, y, sample.count > 999999 ? 999999 : sample.count, RIGHT);
  lcdDrawText(66, y, "F:");
  lcdDrawHexNumber(99, y,
                   (uint16_t(sample.lastFault) << 8) | sample.faultStatus);
  y += FH;
  lcdDrawText(0, y, "H:");
  lcdDrawNumber(60, y, data.sticks[sensor ? 3 : 0], RIGHT);
  lcdDrawText(66, y, "V:");
  lcdDrawNumber(128, y, data.sticks[sensor ? 2 : 1], RIGHT);
}

static void drawMamboBench()
{
  auto data = mamboBenchDiagnostics();
  coord_t y = MENU_HEADER_HEIGHT + 1;
#if defined(MAMBO_EXTERNAL_CRSF)
  lcdDrawText(
      0, y,
      modulePortPowered(EXTERNAL_MODULE) ? "Ext ON Cal:" : "Ext OFF Cal:");
#else
  lcdDrawText(0, y, "RF OFF  Cal:");
#endif
  lcdDrawText(78, y, mamboCalibrationLabel(data.calibration));
  y += FH;
  lcdDrawText(0, y, "Hall us:");
  lcdDrawNumber(128, y, data.lastDurationUs, RIGHT);
  y += FH;
  lcdDrawText(0, y, "Max us:");
  lcdDrawNumber(128, y, data.maxDurationUs, RIGHT);
  y += FH;
  lcdDrawText(0, y, "Frames:");
  lcdDrawNumber(128, y, data.frames, RIGHT);
  y += FH;
  lcdDrawText(0, y, "Free M/U:");
  lcdDrawNumber(87, y, task_get_stack_usage(&mixerTaskId), RIGHT);
  lcdDrawText(89, y, "/");
  lcdDrawNumber(128, y, task_get_stack_usage(&menusTaskId), RIGHT);
  y += FH;
  lcdDrawText(0, y, "Reset mask:");
  lcdDrawHexNumber(99, y, data.resetFlags >> 24);
}
#if defined(MAMBO_EXTERNAL_CRSF)
static void drawMamboExternal()
{
  auto data = mamboExternalDiagnostics();
  coord_t y = MENU_HEADER_HEIGHT + 1;
  lcdDrawText(0, y, data.powered ? "Ext ON" : "Ext OFF");
  lcdDrawText(66, y, (data.pins & 4) ? "DIR OUT" : "DIR IN");
  const uint32_t left[] = {data.txFrames, data.uartErrors, data.parseEvents,
                           data.crcErrors};
  const uint32_t right[] = {data.rxBytes, data.idleEvents, data.validFrames,
                            data.headerErrors};
  const char* leftLabels[] = {"TX:", "Err:", "Parse:", "CRC:"};
  const char* rightLabels[] = {"RX:", "Idle:", "Good:", "Hdr:"};
  for (unsigned i = 0; i < 4; ++i) {
    y += FH;
    lcdDrawText(0, y, leftLabels[i]);
    lcdDrawNumber(60, y, left[i] > 99999 ? 99999 : left[i], RIGHT);
    lcdDrawText(66, y, rightLabels[i]);
    lcdDrawNumber(128, y, right[i] > 99999 ? 99999 : right[i], RIGHT);
  }
  y += FH;
  lcdDrawText(0, y, "T/R:");
  lcdDrawHexNumber(30, y, (uint16_t(data.lastTxType) << 8) | data.lastRxType);
  lcdDrawText(66, y, "Pins:");
  lcdDrawNumber(128, y, data.pins, RIGHT);
}
#endif
#endif

#define HOLDANAVALUEFRAMES 4 /* 4* 50ms = 200 ms update rate */

void menuRadioDiagAnalogs(event_t event)
{
    static int8_t entryCount = 0;
    static uint16_t lastShownAnalogValue[MAX_ANALOG_INPUTS];

    enum ANAVIEWS{
       ANAVIEW_FIRST,
       ANAVIEW_CALIB=ANAVIEW_FIRST,
       ANAVIEW_RAWLOWFPS,
#if defined(RADIO_MAMBO)
       ANAVIEW_MAMBO_LEFT,
       ANAVIEW_MAMBO_RIGHT,
       ANAVIEW_MAMBO_BENCH,
#if defined(MAMBO_EXTERNAL_CRSF)
       ANAVIEW_MAMBO_EXTERNAL,
       ANAVIEW_LAST=ANAVIEW_MAMBO_EXTERNAL
#else
       ANAVIEW_LAST=ANAVIEW_MAMBO_BENCH
#endif
#else
       ANAVIEW_LAST=ANAVIEW_RAWLOWFPS
#endif
    };

    static int viewpage = ANAVIEW_FIRST;

  switch (event) {
    case EVT_KEY_FIRST(KEY_RIGHT):
    case EVT_KEY_BREAK(KEY_PAGEDN):
    {
     if (viewpage == ANAVIEW_LAST)
       viewpage = ANAVIEW_FIRST;
     else
       viewpage++;
     if (viewpage == ANAVIEW_RAWLOWFPS) entryCount = 0;
     break;
    }

    case EVT_KEY_FIRST(KEY_LEFT):
    case EVT_KEY_BREAK(KEY_PAGEUP):
    {
     if (viewpage == ANAVIEW_FIRST)
       viewpage = ANAVIEW_LAST;
     else
       viewpage--;
     if (viewpage == ANAVIEW_RAWLOWFPS) entryCount = 0;
      break;
    }
  }

  switch (viewpage) {
    case (ANAVIEW_CALIB):
      SIMPLE_SUBMENU(STR_MENU_RADIO_ANALOGS_CALIB,
                     HEADER_LINE+1);
      break;
    case (ANAVIEW_RAWLOWFPS):
      SIMPLE_SUBMENU(STR_MENU_RADIO_ANALOGS_RAWLOWFPS,
                     HEADER_LINE+1);
      break;
#if defined(RADIO_MAMBO)
    case ANAVIEW_MAMBO_LEFT:
    case ANAVIEW_MAMBO_RIGHT:
      SIMPLE_SUBMENU(viewpage == ANAVIEW_MAMBO_LEFT ? "Mambo Hall L" : "Mambo Hall R", HEADER_LINE+1);
      drawMamboHall(viewpage == ANAVIEW_MAMBO_LEFT ? 0 : 1);
      return;
    case ANAVIEW_MAMBO_BENCH:
      SIMPLE_SUBMENU("Mambo Bench", HEADER_LINE+1);
      drawMamboBench();
      return;
#if defined(MAMBO_EXTERNAL_CRSF)
    case ANAVIEW_MAMBO_EXTERNAL:
      SIMPLE_SUBMENU("Mambo External", HEADER_LINE+1);
      drawMamboExternal();
      return;
#endif
#endif
  }

  coord_t y = MENU_HEADER_HEIGHT + 1;

  lcdDrawTextAlignedLeft(y, STR_STICKS_POTS_SLIDERS);

  for (uint8_t i = 0; i < adcGetMaxInputs(ADC_INPUT_ALL); i++) {
    // TODO: if available
    uint8_t x;
    if (i & 1) {
      x = LCD_W / 2 + INDENT_WIDTH;
    } else {
      x = INDENT_WIDTH;
      y += FH;
    }
    if (((adcGetInputMask() & (1 << i)) != 0) && i < adcGetMaxInputs(ADC_INPUT_MAIN)) {
      lcdDrawText(x, y, "D");
      lcdDrawNumber(lcdNextPos, y, i + 1);
    }
    else {
      lcdDrawNumber(x, y, i+1, LEADING0|LEFT, 2);
    }
    lcdDrawChar(lcdNextPos, y, ':');
    switch (viewpage) {
      case (ANAVIEW_RAWLOWFPS):
        if (entryCount == 0) {
          lastShownAnalogValue[i] = getAnalogValue(i); // Update value
        }
        lcdDrawNumber(x+3*FW+1, y, lastShownAnalogValue[i],
                      LEADING0|LEFT, 4);
        break;
      case (ANAVIEW_CALIB):
      default:
        lcdDrawNumber(x+3*FW+1, y, anaIn(i), LEADING0|LEFT, 4);
        break;
    }
    lcdDrawNumber(x+(LCD_W / 2 - INDENT_WIDTH), y,
                  (int16_t)calibratedAnalogs[i]*25/256,
                  RIGHT);
  }

  if (viewpage == ANAVIEW_RAWLOWFPS) {
    if (entryCount > HOLDANAVALUEFRAMES)
      entryCount = 0;
    else
      entryCount++;
  }

#if defined(IMU)
  y += FH;
  lcdDrawTextAlignedLeft(y, STR_IMU);
  y += FH;
  uint8_t x = INDENT_WIDTH;
  lcdDrawText(x, y, "X:");
  lcdDrawNumber(x+3*FW-1, y, gyroOutputs[0] * 180 / 1024);
  lcdDrawChar(lcdNextPos, y, CHAR_BW_DEGREE);
  lcdDrawNumber(x+10*FW-1, y, gyroScaledX(), RIGHT);
  x = LCD_W/2 + INDENT_WIDTH;
  lcdDrawText(x, y, "Y:");
  lcdDrawNumber(x+3*FW-1, y, gyroOutputs[1] * 180 / 1024);
  lcdDrawChar(lcdNextPos, y, CHAR_BW_DEGREE);
  lcdDrawNumber(x+10*FW-1, y, gyroScaledY(), RIGHT);
#endif
}
