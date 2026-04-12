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

#define TELEM_COL2                    (16*FW)
#define TELEM_COL3                    (30*FW+2)

static void _init_menu_tab_array(uint8_t* tab, size_t len)
{
  memset((void*)tab, HIDDEN_ROW, sizeof(MENU_TAB_ARRAY_TYPE) * len);
  tab += HEADER_LINE;

  tab[ITEM_TELEMETRY_SENSORS_LABEL] = 0;
  for (int i = 0; i < (int)getSensorCount(); i++)
    tab[ITEM_TELEMETRY_SENSOR_FIRST + i] =
        SENSOR_ROW(isTelemetryFieldAvailable(i) ? (uint8_t)0 : HIDDEN_ROW);
  tab[ITEM_TELEMETRY_NEW_SENSOR] = 0;
  tab[ITEM_TELEMETRY_DELETE_ALL_SENSORS] = 0;
  tab[ITEM_TELEMETRY_IGNORE_SENSOR_INSTANCE] = 0;
  tab[ITEM_TELEMETRY_RSSI_LABEL] = READONLY_ROW;
  tab[ITEM_TELEMETRY_RSSI_ALARM1] = 0;
  tab[ITEM_TELEMETRY_RSSI_ALARM2] = 0;
  tab[ITEM_TELEMETRY_DISABLE_ALARMS] = 0;
#if defined(VARIO)
  tab[ITEM_TELEMETRY_VARIO_LABEL] = READONLY_ROW;
  tab[ITEM_TELEMETRY_VARIO_SOURCE] = 0;
  tab[ITEM_TELEMETRY_VARIO_RANGE] = 1;
  tab[ITEM_TELEMETRY_VARIO_CENTER] = 2;
#endif
}

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
        TelemetrySensor & sourceSensor = *sensorAddress(index);
        TelemetrySensor & newSensor = *sensorAllocAt(newIndex);
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
    for (int i=0; i<(int)getSensorCount(); i++) {
      delTelemetryIndex(i);
    }
  }
}


void menuModelTelemetry(event_t event)
{
  uint8_t MENU_TAB_ARRAY_NAME[HEADER_LINE + ITEM_TELEMETRY_MAX];
  _init_menu_tab_array(MENU_TAB_ARRAY_NAME, HEADER_LINE + ITEM_TELEMETRY_MAX);
  MENU_CHECK(menuTabModel, MENU_MODEL_TELEMETRY, HEADER_LINE + ITEM_TELEMETRY_MAX);
  title(STR_MENUTELEMETRY);

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
      lcdDrawSizedText(3*FW, y, sensorAddress(index)->label, TELEM_LABEL_LEN);
      if (telemetryItems[index].isFresh()) {
        lcdDrawChar(10*FW, y, '*');
      }
      TelemetryItem & telemetryItem = telemetryItems[index];
      if (telemetryItem.isAvailable()) {
        bool isOld = telemetryItem.isOld();
        lcdNextPos = TELEM_COL2;
        if (isOld)
          lcdDrawChar(lcdNextPos, y, '[');
        drawSensorCustomValue(lcdNextPos, y, index, getValue(SourceRef_(SOURCE_TYPE_TELEMETRY, 3*index)), LEFT);
        if (isOld)
          lcdDrawChar(lcdLastRightPos, y, ']');
      }
      else {
        lcdDrawText(TELEM_COL2, y, "---", 0); // TODO shortcut
      }
      TelemetrySensor * sensor = sensorAddress(index);
      if (sensor->type == TELEM_TYPE_CUSTOM && !g_model.ignoreSensorIds) {
        lcdDrawNumber(TELEM_COL3, y, sensor->instance, LEFT);
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
        uint8_t sensorCount = getTelemetrySensorsCount();
        if (sensorCount && !telemExpandState.sensors) {
          lcdDrawChar(TELEM_COL3, y, '(', 0);
          lcdDrawNumber(lcdNextPos, y, sensorCount, 0);
          lcdDrawChar(lcdNextPos, y, ')', 0);
        } else if (telemExpandState.sensors) {
          lcdDrawText(TELEM_COL2, y, STR_VALUE, 0);
          if (!g_model.ignoreSensorIds /*&& !IS_SPEKTRUM_PROTOCOL()*/) {
            lcdDrawText(TELEM_COL3, y, STR_ID, 0);
          }
        }
        break;
      }
      
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
        lcdDrawNumber(TELEM_COL2, y, warning? g_model.rfAlarms.warning : g_model.rfAlarms.critical, LEFT|attr, 3);
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
        drawSource(TELEM_COL2, y, g_model.varioData.source ? SourceRef_(SOURCE_TYPE_TELEMETRY, (uint16_t)(3*(g_model.varioData.source-1))) : SourceRef{}, attr);
        if (attr) {
          g_model.varioData.source = checkIncDec(event, g_model.varioData.source, 0, MAX_TELEMETRY_SENSORS, EE_MODEL|NO_INCDEC_MARKS, isVarioSensorAvailable);
        }
        break;

      case ITEM_TELEMETRY_VARIO_RANGE:
        lcdDrawTextIndented(y, STR_RANGE);
        lcdDrawNumber(TELEM_COL2, y, -10+g_model.varioData.min, (menuHorizontalPosition==0 ? attr : 0)|LEFT);
        lcdDrawNumber(TELEM_COL2+7*FW, y, 10+g_model.varioData.max, (menuHorizontalPosition==1 ? attr : 0)|LEFT);
        if (attr && s_editMode>0) {
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
        lcdDrawNumber(TELEM_COL2, y, -5+g_model.varioData.centerMin, (menuHorizontalPosition==0 ? attr : 0)|PREC1|LEFT);
        lcdDrawNumber(TELEM_COL2+7*FW, y, 5+g_model.varioData.centerMax, (menuHorizontalPosition==1 ? attr : 0)|PREC1|LEFT);
        lcdDrawTextAtIndex(TELEM_COL3, y, STR_VVARIOCENTER, g_model.varioData.centerSilent, (menuHorizontalPosition==2 ? attr : 0));
        if (attr && s_editMode>0) {
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
