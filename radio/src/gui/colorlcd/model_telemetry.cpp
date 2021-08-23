/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "model_telemetry.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static constexpr coord_t SENSOR_COL1 = 30;
static constexpr coord_t SENSOR_COL2 = SENSOR_COL1 + 70;
static constexpr coord_t SENSOR_COL3 = LCD_W - 50;

class SensorSourceChoice : public SourceChoice {
  public:
    SensorSourceChoice(Window * window, const rect_t &rect, uint8_t * source, IsValueAvailable isValueAvailable) :
      SourceChoice(window, rect, MIXSRC_NONE, MIXSRC_LAST_TELEM,
                   GET_DEFAULT(*source ? MIXSRC_FIRST_TELEM + 3 * (*source - 1) : MIXSRC_NONE),
                   [=](uint8_t newValue) {
                     *source = newValue == MIXSRC_NONE ? 0 : (newValue - MIXSRC_FIRST_TELEM) / 3 + 1;
                   })
    {
      setAvailableHandler([=](int16_t value) {
        if (value == MIXSRC_NONE)
          return true;
        if (value < MIXSRC_FIRST_TELEM)
          return false;
        auto qr = div(value - MIXSRC_FIRST_TELEM, 3);
        return qr.rem == 0 && isValueAvailable(qr.quot + 1);
      });
    }
};

class SensorButton : public Button {
  public:
    SensorButton(FormGroup * parent, const rect_t &rect, uint8_t index, uint8_t number) :
      Button(parent, rect),
      index(index),
      number(number)
    {
    }

    static constexpr coord_t line1 = 1;

    void checkEvents() override
    {
      uint32_t now = RTOS_GET_MS();
      if (now - lastRefresh >= 200) {
        // update at least every 200ms
        invalidate();
      }

      TelemetryItem & telemetryItem = telemetryItems[index];
      if (telemetryItem.isFresh()) {
        invalidate();
      }

      Button::checkEvents();
    }

    void paint(BitmapBuffer * dc) override
    {
      TelemetryItem &telemetryItem = telemetryItems[index];

      if (telemetryItem.isAvailable())
        dc->drawSolidFilledRect(2, 2, rect.w - 4, rect.h - 4, COLOR_THEME_ACTIVE);
      else
        dc->drawSolidFilledRect(2, 2, rect.w - 4, rect.h - 4, COLOR_THEME_PRIMARY2);

      if (telemetryItem.isFresh())
        dc->drawFilledCircle(24, (2 + rect.h - 4)/2, 4, COLOR_THEME_SECONDARY1);

      dc->drawNumber(2, 1, number, LEFT, 0, nullptr, ":");

      dc->drawSizedText(SENSOR_COL1, line1, g_model.telemetrySensors[index].label, TELEM_LABEL_LEN);

      if (telemetryItem.isAvailable()) {
        LcdFlags color = telemetryItem.isOld() ? COLOR_THEME_WARNING : COLOR_THEME_SECONDARY1;
        drawSensorCustomValue(dc, SENSOR_COL2, line1, index, getValue(MIXSRC_FIRST_TELEM + 3 * index), LEFT | color);
      }
      else {
        dc->drawText(SENSOR_COL2, line1, "---", COLOR_THEME_SECONDARY1);
      }

      TelemetrySensor * sensor = & g_model.telemetrySensors[index];
      if (IS_SPEKTRUM_PROTOCOL()) {
        drawHexNumber(dc, SENSOR_COL3, line1, sensor->id, LEFT);
      }
      else if (sensor->type == TELEM_TYPE_CUSTOM && !g_model.ignoreSensorIds) {
        dc->drawNumber(SENSOR_COL3, line1, sensor->instance, LEFT);
      }

      if (hasFocus())
        dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_FOCUS);
      else
        dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR_THEME_SECONDARY2);
    }

  protected:
    uint8_t index;
    uint8_t number;
    uint32_t lastRefresh = 0;
};

class SensorEditWindow : public Page {
  public:
    explicit SensorEditWindow(uint8_t index) :
      Page(ICON_MODEL_TELEMETRY),
      index(index)
    {
      buildHeader(&header);
      buildBody(&body);
    }

  protected:
    uint8_t index;
    FormGroup * sensorParametersWindow = nullptr;

    void buildHeader(Window * window)
    {
      new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                     STR_SENSOR + std::to_string(index + 1), 0,
                     COLOR_THEME_PRIMARY2);
      // dynamic display of sensor value ?
      //new StaticText(window, {70, 28, 100, 20}, "SF" + std::to_string(index), 0, COLOR_THEME_SECONDARY1);
    }

    void updateSensorParametersWindow()
    {
      // Sensor variable part
      FormGridLayout grid;
      sensorParametersWindow->clear();
      TelemetrySensor * sensor = &g_model.telemetrySensors[index];

      if (sensor->type == TELEM_TYPE_CALCULATED) {
        // Formula
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_FORMULA);
        new Choice(sensorParametersWindow, grid.getFieldSlot(), STR_VFORMULAS, 0, TELEM_FORMULA_LAST, GET_DEFAULT(sensor->formula),
                   [=](uint8_t newValue) {
                       sensor->formula = newValue;
                       sensor->param = 0;
                       if (sensor->formula == TELEM_FORMULA_CELL) {
                         sensor->unit = UNIT_VOLTS;
                         sensor->prec = 2;
                       }
                       else if (sensor->formula == TELEM_FORMULA_DIST) {
                         sensor->unit = UNIT_DIST;
                         sensor->prec = 0;
                       }
                       else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
                         sensor->unit = UNIT_MAH;
                         sensor->prec = 0;
                       }
                       SET_DIRTY();
                       telemetryItems[index].clear();
                       updateSensorParametersWindow();
                   });
        grid.nextLine();
      }
      else {
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_ID);
        auto hex = new NumberEdit(sensorParametersWindow, grid.getFieldSlot(2, 0), 0, 0xFFFF, GET_SET_DEFAULT(sensor->id));
        hex->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
          drawHexNumber(dc, FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, 0);
        });
        new NumberEdit(sensorParametersWindow, grid.getFieldSlot(2, 1), 0, 0xff, GET_SET_DEFAULT(sensor->instance));
        grid.nextLine();
      }

      // Unit
      if ((sensor->type == TELEM_TYPE_CALCULATED && (sensor->formula == TELEM_FORMULA_DIST)) || sensor->isConfigurable()) {
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_UNIT);
        new Choice(sensorParametersWindow, grid.getFieldSlot(), STR_VTELEMUNIT, 0, UNIT_MAX, GET_DEFAULT(sensor->unit),
                   [=](uint8_t newValue) {
                     sensor->unit = newValue;
                     if (sensor->unit == UNIT_FAHRENHEIT) {
                       sensor->prec = 0;
                     }
                     SET_DIRTY();
                     telemetryItems[index].clear();
                     updateSensorParametersWindow();
                   });
        grid.nextLine();
      }

      // Precision
      if (sensor->isPrecConfigurable() && sensor->unit != UNIT_FAHRENHEIT) {
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_PRECISION);
        new Choice(sensorParametersWindow, grid.getFieldSlot(), STR_VPREC, 0, 2, GET_DEFAULT(sensor->prec),
                   [=](uint8_t newValue) {
                     sensor->prec = newValue;
                     SET_DIRTY();
                     telemetryItems[index].clear();
                     updateSensorParametersWindow();
                   });
        grid.nextLine();
      }

      // Params
      if (sensor->unit < UNIT_FIRST_VIRTUAL) {
        if (sensor->type == TELEM_TYPE_CALCULATED) {
          if (sensor->formula == TELEM_FORMULA_CELL) {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_CELLSENSOR);
            new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), &sensor->cell.source, isCellsSensor);
          }
          else if (sensor->formula == TELEM_FORMULA_DIST) {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_GPSSENSOR);
            new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), &sensor->dist.gps, isGPSSensor);
          }
          else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_CURRENTSENSOR);
            new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), &sensor->consumption.source, isSensorAvailable);
          }
          else if (sensor->formula == TELEM_FORMULA_TOTALIZE) {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_SOURCE);
            new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), &sensor->consumption.source, isSensorAvailable);
          }
          else {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_SOURCE + std::to_string(1));
            new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), (uint8_t *) &sensor->calc.sources[0], isSensorAvailable);
          }
        }
        else {
          if (sensor->unit == UNIT_RPMS) {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_BLADES);
            new NumberEdit(sensorParametersWindow, grid.getFieldSlot(), 1, 30000, GET_SET_DEFAULT(sensor->custom.ratio));
          }
          else {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_RATIO);
            auto edit = new NumberEdit(sensorParametersWindow, grid.getFieldSlot(), 0, 30000, GET_SET_DEFAULT(sensor->custom.ratio));
            edit->setZeroText("-");
          }
        }
        grid.nextLine();
      }

      if (!(sensor->unit == UNIT_GPS || sensor->unit == UNIT_DATETIME || sensor->unit == UNIT_CELLS ||
            (sensor->type == TELEM_TYPE_CALCULATED && (sensor->formula == TELEM_FORMULA_CONSUMPTION || sensor->formula == TELEM_FORMULA_TOTALIZE)))) {
        if (sensor->type == TELEM_TYPE_CALCULATED) {
          if (sensor->formula == TELEM_FORMULA_CELL) {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_CELLINDEX);
            new Choice(sensorParametersWindow, grid.getFieldSlot(), STR_VCELLINDEX, 0, 8, GET_SET_DEFAULT(sensor->cell.index));
          }
          else if (sensor->formula == TELEM_FORMULA_DIST) {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_ALTSENSOR);
            new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), &sensor->dist.alt, isAltSensor);
          }
          else {
            new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_SOURCE + std::to_string(2));
            new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), (uint8_t *) &sensor->calc.sources[1], isSensorAvailable);
          }
        }
        else if (sensor->unit == UNIT_RPMS) {
          new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_MULTIPLIER);
          new NumberEdit(sensorParametersWindow, grid.getFieldSlot(), 1, 30000, GET_SET_DEFAULT(sensor->custom.offset));
        }
        else {
          new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_OFFSET);
          new NumberEdit(sensorParametersWindow, grid.getFieldSlot(), -30000, 30000, GET_SET_DEFAULT(sensor->custom.offset),
                         0, (sensor->prec > 0) ? (sensor->prec == 2 ? PREC2 : PREC1) : 0);
        }
        grid.nextLine();
      }

      if ((sensor->type == TELEM_TYPE_CALCULATED && sensor->formula < TELEM_FORMULA_MULTIPLY)) {
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_SOURCE + std::to_string(3));
        new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), (uint8_t *) &sensor->calc.sources[2], isSensorAvailable);
        grid.nextLine();

        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_SOURCE + std::to_string(4));
        new SensorSourceChoice(sensorParametersWindow, grid.getFieldSlot(), (uint8_t *) &sensor->calc.sources[3], isSensorAvailable);
        grid.nextLine();
      }

      // Auto Offset
      if (sensor->unit != UNIT_RPMS && sensor->isConfigurable()) {
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_AUTOOFFSET);
        new CheckBox(sensorParametersWindow, grid.getFieldSlot(), GET_SET_DEFAULT(sensor->autoOffset));
        grid.nextLine();
      }

      if (sensor->isConfigurable()) {
        // Only positive
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_ONLYPOSITIVE);
        new CheckBox(sensorParametersWindow, grid.getFieldSlot(), GET_SET_DEFAULT(sensor->onlyPositive));
        grid.nextLine();

        // Filter
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_FILTER);
        new CheckBox(sensorParametersWindow, grid.getFieldSlot(), GET_SET_DEFAULT(sensor->filter));
        grid.nextLine();
      }

      if (sensor->type == TELEM_TYPE_CALCULATED) {
        new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_PERSISTENT);
        new CheckBox(sensorParametersWindow, grid.getFieldSlot(), GET_DEFAULT(sensor->persistent), [=](int32_t newValue) {
          sensor->persistent = newValue;
          if (!sensor->persistent)
            sensor->persistentValue = 0;
          SET_DIRTY();
        });
        grid.nextLine();
      }

      new StaticText(sensorParametersWindow, grid.getLabelSlot(), STR_LOGS);
      new CheckBox(sensorParametersWindow, grid.getFieldSlot(), GET_DEFAULT(sensor->logs), [=](int32_t newValue) {
        sensor->logs = newValue;
        logsClose();
        SET_DIRTY();
      });

      sensorParametersWindow->adjustHeight();
    }

    void buildBody(FormWindow * window)
    {
      // Sensor one
      FormGridLayout grid;
      grid.spacer(PAGE_PADDING);

      TelemetrySensor * sensor = &g_model.telemetrySensors[index];

      // Sensor name
      new StaticText(window, grid.getLabelSlot(), STR_NAME);
      new RadioTextEdit(window, grid.getFieldSlot(), sensor->label, sizeof(sensor->label));
      grid.nextLine();

      // Type
      new StaticText(window, grid.getLabelSlot(), STR_TYPE);
      new Choice(window, grid.getFieldSlot(), STR_VSENSORTYPES, 0, 1, GET_DEFAULT(sensor->type),
                 [=](uint8_t newValue) {
                   sensor->type = newValue;
                   sensor->instance = 0;
                   if (sensor->type == TELEM_TYPE_CALCULATED) {
                     sensor->param = 0;
                     sensor->filter = 0;
                     sensor->autoOffset = 0;
                   }
                   SET_DIRTY();
                   updateSensorParametersWindow();
                 });
      grid.nextLine();

      sensorParametersWindow = new FormGroup(window, {0, grid.getWindowHeight(), LCD_W, 0},
                                             FORM_FORWARD_FOCUS | FORM_NO_BORDER
                                             | FORWARD_SCROLL);
      updateSensorParametersWindow();
      grid.addWindow(sensorParametersWindow);

      window->setInnerHeight(grid.getWindowHeight());
    }
};

ModelTelemetryPage::ModelTelemetryPage() :
  PageTab(STR_MENUTELEMETRY, ICON_MODEL_TELEMETRY)
{
}

void ModelTelemetryPage::checkEvents()
{
  if (lastKnownIndex >= 0 && lastKnownIndex != availableTelemetryIndex()) {
    rebuild(window);
  }

  PageTab::checkEvents();
}

void ModelTelemetryPage::rebuild(FormWindow * window, int8_t focusSensorIndex)
{
  coord_t scrollPosition = window->getScrollPositionY();
  window->clear();
  build(window, focusSensorIndex);
  window->setScrollPositionY(scrollPosition);
  lastKnownIndex = availableTelemetryIndex();
}


void ModelTelemetryPage::editSensor(FormWindow * window, uint8_t index)
{
  lastKnownIndex = -1;
  Window * editWindow = new SensorEditWindow(index);
  editWindow->setCloseHandler([=]() {
    rebuild(window, index);
  });
}

void ModelTelemetryPage::build(FormWindow * window, int8_t focusSensorIndex)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);

  this->window = window;

  // RSSI
  new Subtitle(window, grid.getLineSlot(), getRssiLabel());
  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(true), STR_LOWALARM);
  auto edit = new NumberEdit(window, grid.getFieldSlot(), -30, 30, GET_SET_DEFAULT(g_model.rssiAlarms.warning));
  edit->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
    dc->drawNumber(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, g_model.rssiAlarms.getWarningRssi(), flags);
  });
//  window->setFirstField(edit);
  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(true), STR_CRITICALALARM);
  edit = new NumberEdit(window, grid.getFieldSlot(), -30, 30, GET_SET_DEFAULT(g_model.rssiAlarms.critical));
  edit->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
    dc->drawNumber(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, g_model.rssiAlarms.getCriticalRssi(), flags);
  });
  grid.nextLine();

  new StaticText(window, grid.getLabelSlot(true), STR_DISABLE_ALARM);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.rssiAlarms.disabled));
  grid.nextLine();

  // Sensors
  grid.setLabelWidth(140);
  new Subtitle(window, grid.getLineSlot(), STR_TELEMETRY_SENSORS);

  // Sensors columns titles
  uint8_t sensorsCount = getTelemetrySensorsCount();
  if (sensorsCount > 0) {
    new StaticText(window, {SENSOR_COL2, grid.getWindowHeight() + 3, SENSOR_COL3 - SENSOR_COL2, PAGE_LINE_HEIGHT}, STR_VALUE, 0, FONT(XS) | COLOR_THEME_DISABLED);
    if (!g_model.ignoreSensorIds && !IS_SPEKTRUM_PROTOCOL()) {
      new StaticText(window, {SENSOR_COL3, grid.getWindowHeight() + 3, LCD_W - SENSOR_COL3, PAGE_LINE_HEIGHT}, STR_ID, 0, FONT(XS) | COLOR_THEME_DISABLED);
    }
  }

  grid.nextLine();
  grid.setLabelWidth(PAGE_PADDING + PAGE_INDENT_WIDTH);

  for (uint8_t idx = 0, count = 0; idx < MAX_TELEMETRY_SENSORS; idx++) {
    if (g_model.telemetrySensors[idx].isAvailable()) {
      Button * button = new SensorButton(window, grid.getFieldSlot(), idx, ++count);
      button->setPressHandler([=]() -> uint8_t {
        button->bringToTop();
        Menu * menu = new Menu(window);
        menu->addLine(STR_EDIT, [=]() {
          editSensor(window, idx);
        });
        menu->addLine(STR_COPY, [=]() {
          auto newIndex = availableTelemetryIndex();
          if (newIndex >= 0) {
            TelemetrySensor &sourceSensor = g_model.telemetrySensors[idx];
            TelemetrySensor &newSensor = g_model.telemetrySensors[newIndex];
            newSensor = sourceSensor;
            TelemetryItem &sourceItem = telemetryItems[idx];
            TelemetryItem &newItem = telemetryItems[newIndex];
            newItem = sourceItem;
            SET_DIRTY();
            rebuild(window, newIndex);
          }
          else {
            new FullScreenDialog(WARNING_TYPE_ALERT, "", STR_TELEMETRYFULL);
          }
        });
        menu->addLine(STR_DELETE, [=]() {
          delTelemetryIndex(idx); // calls setDirty internally
          rebuild(window);
        });
        return 0;
      });
      if (focusSensorIndex == idx) {
        button->setFocus(SET_FOCUS_DEFAULT);
      }
      grid.nextLine();
    }
  }

  // Autodiscover button
  auto discover = new TextButton(window, grid.getFieldSlot(2, 0), (allowNewSensors) ? STR_STOP_DISCOVER_SENSORS : STR_DISCOVER_SENSORS);
  discover->setPressHandler([=]() {
    allowNewSensors = !allowNewSensors;
    if (allowNewSensors) {
      discover->setText(STR_STOP_DISCOVER_SENSORS);
      return 1;
    }
    else {
      discover->setText(STR_DISCOVER_SENSORS);
      return 0;
    }
  });

  // New sensor button
  new TextButton(window, grid.getFieldSlot(2, 1), STR_TELEMETRY_NEWSENSOR,
                 [=]() -> uint8_t {
                   int idx = availableTelemetryIndex();
                   if (idx >= 0)
                     editSensor(window, idx);
                   else
                     new FullScreenDialog(WARNING_TYPE_ALERT, "", STR_TELEMETRYFULL);
                   return 0;
                 });
  grid.nextLine();

  if (sensorsCount > 0) {
    // Delete all sensors button
    new TextButton(window, grid.getFieldSlot(), STR_DELETE_ALL_SENSORS,
                   []() -> uint8_t {
                       new FullScreenDialog(WARNING_TYPE_CONFIRM, STR_CONFIRMDELETE, "", "", [=]() {
                           for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
                             delTelemetryIndex(i);
                           }
                       });
                       return 0;
                   });
    grid.nextLine();
  }

  // restore aligned layout
  grid.setLabelWidth(PAGE_LABEL_WIDTH);

  // Ignore instance button
  new StaticText(window, grid.getLabelSlot(true), STR_IGNORE_INSTANCE);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.ignoreSensorIds));
  grid.nextLine();

  // Vario
  new Subtitle(window, grid.getLineSlot(), STR_VARIO);
  grid.nextLine();
  new StaticText(window, grid.getLabelSlot(true), STR_SOURCE);
  auto choice = new SourceChoice(window, grid.getFieldSlot(), MIXSRC_NONE, MIXSRC_LAST_TELEM,
                                 GET_DEFAULT(g_model.varioData.source ? MIXSRC_FIRST_TELEM + 3 * (g_model.varioData.source - 1) : MIXSRC_NONE),
                                 SET_VALUE(g_model.varioData.source, newValue == MIXSRC_NONE ? 0 : (newValue - MIXSRC_FIRST_TELEM) / 3 + 1));
  choice->setAvailableHandler([=](int16_t value) {
    if (value == MIXSRC_NONE)
      return true;
    if (value < MIXSRC_FIRST_TELEM)
      return false;
    auto qr = div(value - MIXSRC_FIRST_TELEM, 3);
    return qr.rem == 0 && isSensorAvailable(qr.quot + 1);
  });
  grid.nextLine();
  new StaticText(window, grid.getLabelSlot(true), STR_RANGE);

  auto vMin = new NumberEdit(window, grid.getFieldSlot(2, 0), -17, 17,
                             GET_SET_WITH_OFFSET(g_model.varioData.min, -10));
  vMin->setAvailableHandler(
      [](int val) { return val < g_model.varioData.max + 10; });

  auto vMax = new NumberEdit(window, grid.getFieldSlot(2, 1), -17, 17,
                             GET_SET_WITH_OFFSET(g_model.varioData.max, 10));
  vMax->setAvailableHandler(
      [](int val) { return g_model.varioData.min - 10 < val; });

  grid.nextLine();
  new StaticText(window, grid.getLabelSlot(true), STR_CENTER);

  auto cMin = new NumberEdit(
      window, grid.getFieldSlot(3, 0), -15, 15,
      GET_SET_WITH_OFFSET(g_model.varioData.centerMin, -5), 0, PREC1);
  cMin->setAvailableHandler(
      [](int val) { return val < g_model.varioData.centerMax + 5; });

  auto cMax = new NumberEdit(
      window, grid.getFieldSlot(3, 1), -15, 15,
      GET_SET_WITH_OFFSET(g_model.varioData.centerMax, 5), 0, PREC1);
  cMax->setAvailableHandler(
      [](int val) { return g_model.varioData.centerMin - 5 < val; });

  new Choice(window, grid.getFieldSlot(3, 2), STR_VVARIOCENTER, 0, 1,
             GET_SET_DEFAULT(g_model.varioData.centerSilent));
  grid.nextLine();

  window->setInnerHeight(grid.getWindowHeight());
}
