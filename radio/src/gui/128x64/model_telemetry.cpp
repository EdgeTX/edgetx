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

enum MenuModelTelemetryFrskyItems {
  ITEM_TELEMETRY_SENSORS_LABEL,
  ITEM_TELEMETRY_SENSOR_FIRST,
  ITEM_TELEMETRY_SENSOR_LAST = ITEM_TELEMETRY_SENSOR_FIRST + MAX_TELEMETRY_SENSORS - 1,
  ITEM_TELEMETRY_DISCOVER_SENSORS,
  ITEM_TELEMETRY_NEW_SENSOR,
  ITEM_TELEMETRY_DELETE_ALL_SENSORS,
  ITEM_TELEMETRY_IGNORE_SENSOR_INSTANCE,
  ITEM_TELEMETRY_RSSI_LABEL,
  ITEM_TELEMETRY_RSSI_ALARM1,
  ITEM_TELEMETRY_RSSI_ALARM2,
  ITEM_TELEMETRY_DISABLE_ALARMS,
#if defined(VARIO)
  ITEM_TELEMETRY_VARIO_LABEL,
  ITEM_TELEMETRY_VARIO_SOURCE,
  ITEM_TELEMETRY_VARIO_RANGE,
  ITEM_TELEMETRY_VARIO_CENTER,
#endif
  ITEM_TELEMETRY_MAX
};

PACK(struct ExpandState {
  uint8_t sensors:1;
});

struct ExpandState telemExpandState;

uint8_t SENSOR_ROW(uint8_t value)
{
  if (telemExpandState.sensors)
    return value;
  return HIDDEN_ROW;
}

#if defined(TRANSLATIONS_CZ)
  #define TELEM_COL2                 (9*FW)
#else
  #define TELEM_COL2                 (8*FW)
#endif

#define TELEM_COL3                   (17 * FW)

#define RSSI_ROWS                     LABEL(RSSI), 0, 0, 0,
#define SENSOR_ROWS(x)                SENSOR_ROW((isTelemetryFieldAvailable(x) ? (uint8_t)0 : HIDDEN_ROW))
#define SENSORS_ROWS                  0, SENSOR_ROWS(0), SENSOR_ROWS(1), SENSOR_ROWS(2), SENSOR_ROWS(3), SENSOR_ROWS(4), SENSOR_ROWS(5), SENSOR_ROWS(6), SENSOR_ROWS(7), SENSOR_ROWS(8), SENSOR_ROWS(9), SENSOR_ROWS(10), SENSOR_ROWS(11), SENSOR_ROWS(12), SENSOR_ROWS(13), SENSOR_ROWS(14), SENSOR_ROWS(15), SENSOR_ROWS(16), SENSOR_ROWS(17), SENSOR_ROWS(18), SENSOR_ROWS(19), SENSOR_ROWS(20), SENSOR_ROWS(21), SENSOR_ROWS(22), SENSOR_ROWS(23), SENSOR_ROWS(24), SENSOR_ROWS(25), SENSOR_ROWS(26), SENSOR_ROWS(27), SENSOR_ROWS(28), SENSOR_ROWS(29), SENSOR_ROWS(30), SENSOR_ROWS(31), SENSOR_ROWS(32), SENSOR_ROWS(33), SENSOR_ROWS(34), SENSOR_ROWS(35), SENSOR_ROWS(36), SENSOR_ROWS(37), SENSOR_ROWS(38), SENSOR_ROWS(39), 0, 0, 0, 0,
#if defined(VARIO)
  #define VARIO_ROWS                  LABEL(Vario), 0, 1, 2,
#else
  #define VARIO_ROWS
#endif

void onSensorMenu(const char * result)
{
  uint8_t index = menuVerticalPosition - HEADER_LINE - ITEM_TELEMETRY_SENSOR_FIRST;

  if (index < MAX_TELEMETRY_SENSORS) {
    if (result == STR_EDIT) {
      pushMenu(menuModelSensor);
    }
    else if (result == STR_DELETE) {
      delTelemetryIndex(index);
      index += 1;
      if (index<MAX_TELEMETRY_SENSORS && isTelemetryFieldAvailable(index))
        menuVerticalPosition += 1;
      else
        menuVerticalPosition = HEADER_LINE + ITEM_TELEMETRY_NEW_SENSOR;
    }
    else if (result == STR_COPY) {
      int newIndex = availableTelemetryIndex();
      if (newIndex >= 0) {
        TelemetrySensor & sourceSensor = g_model.telemetrySensors[index];
        TelemetrySensor & newSensor = g_model.telemetrySensors[newIndex];
        newSensor = sourceSensor;
        TelemetryItem & sourceItem = telemetryItems[index];
        TelemetryItem & newItem = telemetryItems[newIndex];
        newItem = sourceItem;
        storageDirty(EE_MODEL);
      }
      else {
        POPUP_WARNING(STR_TELEMETRYFULL);
      }
    }
  }
}

void onDeleteAllSensorsConfirm(const char * result)
{
  if (result == STR_OK) {
    for (int i=0; i<MAX_TELEMETRY_SENSORS; i++) {
      delTelemetryIndex(i);
    }
  }
}

uint8_t getTelemetrySensorCount()
{
  uint8_t count = 0;

  for (uint8_t index = 0; index <= MAX_TELEMETRY_SENSORS; index++) {
    if (isTelemetryFieldAvailable(index)) {
      count++;
    }
  }
  return count;
}

void menuModelTelemetry(event_t event)
{
  MENU(STR_MENUTELEMETRY, menuTabModel, MENU_MODEL_TELEMETRY, HEADER_LINE+ITEM_TELEMETRY_MAX, { HEADER_LINE_COLUMNS SENSORS_ROWS RSSI_ROWS VARIO_ROWS });

  uint8_t sub = menuVerticalPosition - HEADER_LINE;

  for (uint8_t i=0; i<NUM_BODY_LINES; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i + menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j+HEADER_LINE] == HIDDEN_ROW) {
        if (++k >= (int)DIM(mstate_tab)) {
          return;
        }
      }
    }

    LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);


    if (k >= ITEM_TELEMETRY_SENSOR_FIRST && k <= ITEM_TELEMETRY_SENSOR_LAST) {
      int index = k - ITEM_TELEMETRY_SENSOR_FIRST;
      lcdDrawNumber(INDENT_WIDTH, y, index+1, LEFT|attr);
      lcdDrawChar(lcdLastRightPos, y, ':', attr);
      lcdDrawSizedText(3*FW, y, g_model.telemetrySensors[index].label, TELEM_LABEL_LEN, 0);
      if (telemetryItems[index].isFresh()) {
        lcdDrawChar(TELEM_COL3, y, '*');
      }
      TelemetryItem & telemetryItem = telemetryItems[index];
      if (telemetryItem.isAvailable()) {
        bool isOld = telemetryItem.isOld();
        lcdNextPos = TELEM_COL2;
        if (isOld)
          lcdDrawChar(lcdNextPos, y, '[');
        drawSensorCustomValue(lcdNextPos, y, index, getValue(MIXSRC_FIRST_TELEM+3*index), LEFT);
        if (isOld)
          lcdDrawChar(lcdLastRightPos, y, ']');
      }
      else {
        lcdDrawText(TELEM_COL2, y, "---", 0); // TODO shortcut
      }
      if (attr) {
        s_editMode = 0;
        s_currIdx = index;
        if (event == EVT_KEY_LONG(KEY_ENTER)) {
          POPUP_MENU_START(onSensorMenu, 3, STR_EDIT, STR_COPY, STR_DELETE);
        }
        else if (event == EVT_KEY_BREAK(KEY_ENTER)) {
          pushMenu(menuModelSensor);
        }
      }
    }
    else

    switch (k) {
      case ITEM_TELEMETRY_SENSORS_LABEL:
      {
        telemExpandState.sensors = expandableSection(y, STR_TELEMETRY_SENSORS, telemExpandState.sensors, attr, event);
        uint8_t sensorCount = getTelemetrySensorCount();
        if (sensorCount && !telemExpandState.sensors) {
          lcdDrawChar(TELEM_COL3, y, '(', 0);
          lcdDrawNumber(lcdNextPos, y, sensorCount, 0);
          lcdDrawChar(lcdNextPos, y, ')', 0);
        }
        break;
      }

      case ITEM_TELEMETRY_DISCOVER_SENSORS:
        lcdDrawText(INDENT_WIDTH, y, allowNewSensors ? STR_STOP_DISCOVER_SENSORS : STR_DISCOVER_SENSORS, attr);
        if (attr && event==EVT_KEY_BREAK(KEY_ENTER)) {
          s_editMode = 0;
          allowNewSensors = !allowNewSensors;
          telemExpandState.sensors = true;
        }
        break;

      case ITEM_TELEMETRY_NEW_SENSOR:
        lcdDrawText(INDENT_WIDTH, y, STR_TELEMETRY_NEWSENSOR, attr);
        if (attr && event==EVT_KEY_BREAK(KEY_ENTER)) {
          s_editMode = 0;
          int res = availableTelemetryIndex();
          if (res >= 0) {
            s_currIdx = res;
            pushMenu(menuModelSensor);
          }
          else {
            allowNewSensors = 0;
            POPUP_WARNING(STR_TELEMETRYFULL);
          }
        }
        break;

      case ITEM_TELEMETRY_DELETE_ALL_SENSORS:
        lcdDrawText(INDENT_WIDTH, y, STR_DELETE_ALL_SENSORS, attr);
        if (attr) {
          s_editMode = 0;
          if (event == EVT_KEY_BREAK(KEY_ENTER)) {
            POPUP_CONFIRMATION(STR_CONFIRMDELETE, onDeleteAllSensorsConfirm);
          }
        }
        break;

      case ITEM_TELEMETRY_IGNORE_SENSOR_INSTANCE:
        g_model.ignoreSensorIds = editCheckBox(g_model.ignoreSensorIds, TELEM_COL2, y, STR_IGNORE_INSTANCE, attr, event, INDENT_WIDTH);
        break;

      case ITEM_TELEMETRY_RSSI_LABEL:
        lcdDrawTextAlignedLeft(y, getRxStatLabels()->label);
        break;

      case ITEM_TELEMETRY_RSSI_ALARM1:
      case ITEM_TELEMETRY_RSSI_ALARM2:
      {
        bool warning = (k==ITEM_TELEMETRY_RSSI_ALARM1);
        lcdDrawTextIndented(y, (warning ? STR_LOWALARM : STR_CRITICALALARM));
        lcdDrawNumber(TELEM_COL3, y, warning? g_model.rfAlarms.warning : g_model.rfAlarms.critical, attr, 3);
        if (attr && s_editMode>0) {
          if (warning)
            CHECK_INCDEC_MODELVAR(event, g_model.rfAlarms.warning, 0, 100);
          else
            CHECK_INCDEC_MODELVAR(event, g_model.rfAlarms.critical, 0, 100);
        }
        break;
      }

      case ITEM_TELEMETRY_DISABLE_ALARMS:
        g_model.disableTelemetryWarning = editCheckBox(g_model.disableTelemetryWarning, TELEM_COL3, y, STR_DISABLE_ALARM, attr, event, INDENT_WIDTH);
        break;

#if defined(VARIO)
      case ITEM_TELEMETRY_VARIO_LABEL:
        lcdDrawTextAlignedLeft(y, STR_VARIO);
        break;

      case ITEM_TELEMETRY_VARIO_SOURCE:
        lcdDrawTextIndented(y, STR_SOURCE);
        drawSource(TELEM_COL2, y, g_model.varioData.source ? MIXSRC_FIRST_TELEM+3*(g_model.varioData.source-1) : 0, attr);
        if (attr) {
          g_model.varioData.source = checkIncDec(event, g_model.varioData.source, 0, MAX_TELEMETRY_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isVarioSensorAvailable);
        }
        break;

      case ITEM_TELEMETRY_VARIO_RANGE:
        lcdDrawTextIndented(y, STR_RANGE);
        if (attr && CURSOR_ON_LINE()) {
          lcdDrawSolidFilledRect(TELEM_COL2-1, y-1, LCD_W-TELEM_COL2+1, FH+1);
        }
        lcdDrawNumber(TELEM_COL2, y, -10+g_model.varioData.min, ((CURSOR_ON_LINE() || menuHorizontalPosition==0) ? attr : 0)|LEFT);
        lcdDrawNumber(TELEM_COL2+4*FW, y, 10+g_model.varioData.max, ((CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0)|LEFT);
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.varioData.min, -7, 7);
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.varioData.max, -7, 7);
              break;
          }
        }
        break;

      case ITEM_TELEMETRY_VARIO_CENTER:
        lcdDrawTextIndented(y, STR_CENTER);
        lcdDrawNumber(TELEM_COL2, y, -5+g_model.varioData.centerMin, ((CURSOR_ON_LINE() || menuHorizontalPosition==0) ? attr : 0)|PREC1|LEFT);
        lcdDrawNumber(TELEM_COL2+4*FW, y, 5+g_model.varioData.centerMax, ((CURSOR_ON_LINE() || menuHorizontalPosition==1) ? attr : 0)|PREC1|LEFT);
        lcdDrawTextAtIndex(TELEM_COL2+8*FW, y, STR_VVARIOCENTER, g_model.varioData.centerSilent, (menuHorizontalPosition==2 ? attr : 0));
        if (attr && s_editMode > 0) {
          switch (menuHorizontalPosition) {
            case 0:
              CHECK_INCDEC_MODELVAR(event, g_model.varioData.centerMin, -16, 5+min<int8_t>(10, g_model.varioData.centerMax+5));
              break;
            case 1:
              CHECK_INCDEC_MODELVAR(event, g_model.varioData.centerMax, -5+max<int8_t>(-10, g_model.varioData.centerMin-5), +15);
              break;
            case 2:
              CHECK_INCDEC_MODELVAR_ZERO(event, g_model.varioData.centerSilent, 1);
              break;
          }
        }
        break;
#endif
    }
  }
}
