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

#include <iostream>
#include <sstream>
#include "model_telemetry.h"
#include "opentx.h"
#include "libopenui.h"
#include "lvgl_widgets/input_mix_line.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

std::string getSensorCustomValue(uint8_t sensor, int32_t value, LcdFlags flags);

#if (LCD_H > LCD_W) || defined(TRANSLATIONS_CZ)
#define TWOCOLBUTTONS   1
#else
#define TWOCOLBUTTONS   0
#endif

// Overview grid variants
static const lv_coord_t col_dsc[] = {LV_GRID_FR(14), LV_GRID_FR(6),
                                     LV_GRID_TEMPLATE_LAST};
#if LCD_W > LCD_H
static const lv_coord_t col_dsc2[] = {LV_GRID_FR(3), LV_GRID_FR(3), 8, LV_GRID_FR(8), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
#else                                     
static const lv_coord_t col_dsc2[] = {28, 56, 8, LV_GRID_FR(1), 32,
                                     LV_GRID_TEMPLATE_LAST};
#endif
static const lv_coord_t col_dsc3[] = {LCD_W - 12,
                                     LV_GRID_TEMPLATE_LAST};
#if TWOCOLBUTTONS
static const lv_coord_t col_dsc4[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc4[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
#endif
static const lv_coord_t col_dsc5[] = {LV_GRID_FR(5), LV_GRID_FR(4), LV_GRID_FR(4), LV_GRID_FR(4),
                                     LV_GRID_TEMPLATE_LAST};

// Edit grid variants
static const lv_coord_t e_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                       LV_GRID_TEMPLATE_LAST};
static const lv_coord_t e_col_dsc2[] = {LV_GRID_FR(4), LV_GRID_FR(3), LV_GRID_FR(3),
                                       LV_GRID_TEMPLATE_LAST};

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

// Sensor value label that updates current value display
class SensorValueLabel : public StaticText {
  public:
    SensorValueLabel(Window * parent, const rect_t &rect, uint8_t index) :
      StaticText(parent, rect, ""),
      index(index)
    {
      padLeft(10);
    }

    void checkEvents() override
    {
      uint32_t now = RTOS_GET_MS();
      TelemetryItem & telemetryItem = telemetryItems[index];

      if ((now - lastRefresh >= 200) || telemetryItem.isFresh()) {
        // update at least every 200ms
        lastRefresh = now;

        std::string s;
        LcdFlags color = COLOR_THEME_SECONDARY1;

        if (telemetryItem.isAvailable()) {
          color = telemetryItem.isOld() ? COLOR_THEME_WARNING : color;
          s = getSensorCustomValue(index, getValue(MIXSRC_FIRST_TELEM + 3 * index), LEFT);
        } else {
          s = "---";
        }

#if LCD_H > LCD_W
        if (s.size() >= 20) {
          setFont(FONT(XS));
        } else {
          setFont(FONT(STD));
        }
#endif
        lv_obj_set_style_text_color(lvobj, makeLvColor(color), 0);
        setText(s);
      }
    }

    void paint(BitmapBuffer * dc) override
    {
      TelemetryItem &telemetryItem = telemetryItems[index];

      // Draw a 'fresh' marker
      if (telemetryItem.isFresh())
        dc->drawFilledCircle(5, 10, 4, COLOR_THEME_SECONDARY1);
    }

  protected:
    uint8_t index;
    uint32_t lastRefresh = 0;
};

class SensorButton : public Button {
  public:
    SensorButton(Window * parent, const rect_t &rect, uint8_t index) :
      Button(parent, rect, nullptr, 0, 0, input_mix_line_create),
      index(index)
    {
      padAll(0);

      lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
      lv_obj_set_grid_dsc_array(lvobj, col_dsc2, row_dsc);
      lv_obj_set_style_pad_row(lvobj, 0, 0);
      lv_obj_set_style_pad_column(lvobj, 4, 0);

      check(isActive());

      lv_obj_update_layout(parent->getLvObj());
      if(lv_obj_is_visible(lvobj)) delayed_init(nullptr);

      lv_obj_add_event_cb(lvobj, SensorButton::on_draw, LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
    }

  protected:
    bool init = false;
    uint8_t index;
    uint32_t lastRefresh = 0;

    static void on_draw(lv_event_t * e)
    {
      lv_obj_t* target = lv_event_get_target(e);
      auto line = (SensorButton*)lv_obj_get_user_data(target);
      if (line) {
        if (!line->init)
          line->delayed_init(e);
        else
          line->refresh();
      }
    }
  
    bool isActive() const
    {
       TelemetryItem & telemetryItem = telemetryItems[index];
       return telemetryItem.isAvailable();
    }

    void checkEvents() override
    {
      Button::checkEvents();
      check(isActive());
    }

    void delayed_init(lv_event_t* e)
    {
      char s[20];

      auto lbl = lv_label_create(lvobj);
      lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_grid_cell(lbl, LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      lv_label_set_text(lbl, std::to_string(index).c_str());

      lbl = lv_label_create(lvobj);
      lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_LEFT, 0);
      lv_obj_set_grid_cell(lbl, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      strAppend(s, g_model.telemetrySensors[index].label, TELEM_LABEL_LEN);
      lv_label_set_text(lbl, s);

      auto v = new SensorValueLabel(this, rect_t{}, index);
      lv_obj_set_grid_cell(v->getLvObj(), LV_GRID_ALIGN_STRETCH, 2, 2, LV_GRID_ALIGN_CENTER, 0, 1);

      TelemetrySensor * sensor = & g_model.telemetrySensors[index];
      if (sensor->type == TELEM_TYPE_CUSTOM && !g_model.ignoreSensorIds) {
        sprintf(s, "%d", sensor->instance);
      }

      lbl = lv_label_create(lvobj);
      lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_LEFT, 0);
      lv_obj_set_grid_cell(lbl, LV_GRID_ALIGN_START, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      lv_label_set_text(lbl, s);

      init = true;
      refresh();
      lv_obj_update_layout(lvobj);

      if(e) {
        auto param = lv_event_get_param(e);
        lv_event_send(lvobj, LV_EVENT_DRAW_MAIN, param);
      }
    }

    void refresh()
    {
    }
};

class SensorSourceChoice : public SourceChoice
{
 public:
  SensorSourceChoice(Window *window, const rect_t &rect, uint8_t *source,
                     IsValueAvailable isValueAvailable) :
      SourceChoice(window, rect, MIXSRC_NONE, MIXSRC_LAST_TELEM,
                   GET_DEFAULT(*source ? MIXSRC_FIRST_TELEM + 3 * (*source - 1)
                                       : MIXSRC_NONE),
                   [=](uint16_t newValue) {
                     *source = newValue == MIXSRC_NONE
                                   ? 0
                                   : (newValue - MIXSRC_FIRST_TELEM) / 3 + 1;
                   })
  {
    setAvailableHandler([=](int16_t value) {
      if (value == MIXSRC_NONE) return true;
      if (value < MIXSRC_FIRST_TELEM) return false;
      auto qr = div(value - MIXSRC_FIRST_TELEM, 3);
      return qr.rem == 0 && isValueAvailable(qr.quot + 1);
    });
  }
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
    uint32_t lastRefresh = 0;
    FormWindow * sensorParametersWindow = nullptr;
    StaticText * headerValue = nullptr;

    void buildHeader(Window * window)
    {
      std::string title2 = STR_SENSOR + std::to_string(index + 1) + " = " + STR_NA;
      header.setTitle(STR_MENUTELEMETRY);

      headerValue = header.setTitle2(title2);

      lv_obj_set_style_text_color(headerValue->getLvObj(), makeLvColor(COLOR_THEME_WARNING), LV_STATE_USER_1);
    }

    void checkEvents() override
    {
      uint32_t now = RTOS_GET_MS();
      TelemetryItem & telemetryItem = telemetryItems[index];

      if ((now - lastRefresh >= 200) || telemetryItem.isFresh()) {
        // update at least every 200ms
        lastRefresh = now;

        lv_obj_clear_state(headerValue->getLvObj(), LV_STATE_USER_1);

        if (telemetryItem.isAvailable()) {
          if (telemetryItem.isOld())
            lv_obj_add_state(headerValue->getLvObj(), LV_STATE_USER_1);
          std::string title2 =
              STR_SENSOR + std::to_string(index + 1) + " = " +
               getSensorCustomValue(
                   index, getValue(MIXSRC_FIRST_TELEM + 3 * index), LEFT);
          headerValue->setText(title2);
        } else {
          headerValue->setText(STR_SENSOR + std::to_string(index + 1) + " = " +
                               STR_NA);
        }
      }
    }

    void updateSensorParametersWindow()
    {
      // Sensor variable part
      sensorParametersWindow->clear();
      sensorParametersWindow->setFlexLayout();
      FlexGridLayout grid(e_col_dsc, row_dsc, 2);
      FlexGridLayout grid2(e_col_dsc2, row_dsc, 2);

      TelemetrySensor * sensor = &g_model.telemetrySensors[index];

      FormWindow::Line* line;

      if (sensor->type == TELEM_TYPE_CALCULATED) {
        // Formula
        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_FORMULA, 0, COLOR_THEME_PRIMARY1);
        new Choice(line, rect_t{}, STR_VFORMULAS, 0, TELEM_FORMULA_LAST, GET_DEFAULT(sensor->formula),
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
      }
      else {
        line = sensorParametersWindow->newLine(&grid2);
        new StaticText(line, rect_t{}, STR_ID, 0, COLOR_THEME_PRIMARY1);
        auto hex = new NumberEdit(line, rect_t{}, 0, 0xFFFF, GET_SET_DEFAULT(sensor->id));
        hex->setDisplayHandler([](int32_t value) {
          std::stringstream stream;
          stream << std::hex << value;
          return stream.str();
        });
        new NumberEdit(line, rect_t{}, 0, 0xff, GET_SET_DEFAULT(sensor->instance));
      }

      // Unit
      if ((sensor->type == TELEM_TYPE_CALCULATED && (sensor->formula == TELEM_FORMULA_DIST)) || sensor->isConfigurable()) {
        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_UNIT, 0, COLOR_THEME_PRIMARY1);
        new Choice(line, rect_t{}, STR_VTELEMUNIT, 0, UNIT_MAX, GET_DEFAULT(sensor->unit),
                   [=](uint8_t newValue) {
                     sensor->unit = newValue;
                     if (sensor->unit == UNIT_FAHRENHEIT) {
                       sensor->prec = 0;
                     }
                     SET_DIRTY();
                     telemetryItems[index].clear();
                     updateSensorParametersWindow();
                   });
      }

      // Precision
      if (sensor->isPrecConfigurable() && sensor->unit != UNIT_FAHRENHEIT) {
        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_PRECISION, 0, COLOR_THEME_PRIMARY1);
        new Choice(line, rect_t{}, STR_VPREC, 0, 2, GET_DEFAULT(sensor->prec),
                   [=](uint8_t newValue) {
                     sensor->prec = newValue;
                     SET_DIRTY();
                     telemetryItems[index].clear();
                     updateSensorParametersWindow();
                   });
      }

      // Params
      if (sensor->unit < UNIT_FIRST_VIRTUAL) {
        line = sensorParametersWindow->newLine(&grid);
        if (sensor->type == TELEM_TYPE_CALCULATED) {
          if (sensor->formula == TELEM_FORMULA_CELL) {
            new StaticText(line, rect_t{}, STR_CELLSENSOR, 0, COLOR_THEME_PRIMARY1);
            new SensorSourceChoice(line, rect_t{}, &sensor->cell.source, isCellsSensor);
          }
          else if (sensor->formula == TELEM_FORMULA_DIST) {
            new StaticText(line, rect_t{}, STR_GPSSENSOR, 0, COLOR_THEME_PRIMARY1);
            new SensorSourceChoice(line, rect_t{}, &sensor->dist.gps, isGPSSensor);
          }
          else if (sensor->formula == TELEM_FORMULA_CONSUMPTION) {
            new StaticText(line, rect_t{}, STR_CURRENTSENSOR, 0, COLOR_THEME_PRIMARY1);
            new SensorSourceChoice(line, rect_t{}, &sensor->consumption.source, isSensorAvailable);
          }
          else if (sensor->formula == TELEM_FORMULA_TOTALIZE) {
            new StaticText(line, rect_t{}, STR_SOURCE, 0, COLOR_THEME_PRIMARY1);
            new SensorSourceChoice(line, rect_t{}, &sensor->consumption.source, isSensorAvailable);
          }
          else {
            new StaticText(line, rect_t{}, STR_SOURCE + std::to_string(1), 0, COLOR_THEME_PRIMARY1);
            new SensorSourceChoice(line, rect_t{}, (uint8_t *) &sensor->calc.sources[0], isSensorAvailable);
          }
        }
        else {
          if (sensor->unit == UNIT_RPMS) {
            new StaticText(line, rect_t{}, STR_BLADES, 0, COLOR_THEME_PRIMARY1);
            new NumberEdit(line, rect_t{}, 1, 30000, GET_SET_DEFAULT(sensor->custom.ratio));
          }
          else {
            new StaticText(line, rect_t{}, STR_RATIO, 0, COLOR_THEME_PRIMARY1);
            auto edit = new NumberEdit(line, rect_t{}, 0, 30000, GET_SET_DEFAULT(sensor->custom.ratio));
            edit->setZeroText("-");
          }
        }
      }

      if (!(sensor->unit == UNIT_GPS || sensor->unit == UNIT_DATETIME || sensor->unit == UNIT_CELLS ||
            (sensor->type == TELEM_TYPE_CALCULATED && (sensor->formula == TELEM_FORMULA_CONSUMPTION || sensor->formula == TELEM_FORMULA_TOTALIZE)))) {
        line = sensorParametersWindow->newLine(&grid);
        if (sensor->type == TELEM_TYPE_CALCULATED) {
          if (sensor->formula == TELEM_FORMULA_CELL) {
            new StaticText(line, rect_t{}, STR_CELLINDEX, 0, COLOR_THEME_PRIMARY1);
            new Choice(line, rect_t{}, STR_VCELLINDEX, TELEM_CELL_INDEX_LOWEST, TELEM_CELL_INDEX_LAST, GET_SET_DEFAULT(sensor->cell.index));
          }
          else if (sensor->formula == TELEM_FORMULA_DIST) {
            new StaticText(line, rect_t{}, STR_ALTSENSOR, 0, COLOR_THEME_PRIMARY1);
            new SensorSourceChoice(line, rect_t{}, &sensor->dist.alt, isAltSensor);
          }
          else {
            new StaticText(line, rect_t{}, STR_SOURCE + std::to_string(2), 0, COLOR_THEME_PRIMARY1);
            new SensorSourceChoice(line, rect_t{}, (uint8_t *) &sensor->calc.sources[1], isSensorAvailable);
          }
        }
        else if (sensor->unit == UNIT_RPMS) {
          new StaticText(line, rect_t{}, STR_MULTIPLIER, 0, COLOR_THEME_PRIMARY1);
          new NumberEdit(line, rect_t{}, 1, 30000, GET_SET_DEFAULT(sensor->custom.offset));
        }
        else {
          new StaticText(line, rect_t{}, STR_OFFSET, 0, COLOR_THEME_PRIMARY1);
          new NumberEdit(line, rect_t{}, -30000, 30000, GET_SET_DEFAULT(sensor->custom.offset),
                         0, (sensor->prec > 0) ? (sensor->prec == 2 ? PREC2 : PREC1) : 0);
        }
      }

      if ((sensor->type == TELEM_TYPE_CALCULATED && sensor->formula < TELEM_FORMULA_MULTIPLY)) {
        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_SOURCE + std::to_string(3), 0, COLOR_THEME_PRIMARY1);
        new SensorSourceChoice(line, rect_t{}, (uint8_t *) &sensor->calc.sources[2], isSensorAvailable);

        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_SOURCE + std::to_string(4), 0, COLOR_THEME_PRIMARY1);
        new SensorSourceChoice(line, rect_t{}, (uint8_t *) &sensor->calc.sources[3], isSensorAvailable);
      }

      // Auto Offset
      if (sensor->unit != UNIT_RPMS && sensor->isConfigurable()) {
        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_AUTOOFFSET, 0, COLOR_THEME_PRIMARY1);
        new CheckBox(line, rect_t{}, GET_SET_DEFAULT(sensor->autoOffset));
      }

      if (sensor->isConfigurable()) {
        // Only positive
        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_ONLYPOSITIVE, 0, COLOR_THEME_PRIMARY1);
        new CheckBox(line, rect_t{}, GET_SET_DEFAULT(sensor->onlyPositive));

        // Filter
        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_FILTER, 0, COLOR_THEME_PRIMARY1);
        new CheckBox(line, rect_t{}, GET_SET_DEFAULT(sensor->filter));
      }

      if (sensor->type == TELEM_TYPE_CALCULATED) {
        line = sensorParametersWindow->newLine(&grid);
        new StaticText(line, rect_t{}, STR_PERSISTENT, 0, COLOR_THEME_PRIMARY1);
        new CheckBox(line, rect_t{}, GET_DEFAULT(sensor->persistent), [=](int32_t newValue) {
          sensor->persistent = newValue;
          if (!sensor->persistent)
            sensor->persistentValue = 0;
          SET_DIRTY();
        });
      }

      line = sensorParametersWindow->newLine(&grid);
      new StaticText(line, rect_t{}, STR_LOGS, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{}, GET_DEFAULT(sensor->logs), [=](int32_t newValue) {
        sensor->logs = newValue;
        logsClose();
        SET_DIRTY();
      });
    }

    void buildBody(FormWindow * window)
    {
      window->padAll(0);
      lv_obj_set_scrollbar_mode(window->getLvObj(), LV_SCROLLBAR_MODE_AUTO);

      // Sensor one
      auto form = new FormWindow(window, rect_t{});
      form->setFlexLayout();
      form->padAll(4);

      FlexGridLayout grid(e_col_dsc, row_dsc, 2);

      TelemetrySensor * sensor = &g_model.telemetrySensors[index];

      // Sensor name
      auto line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
      new ModelTextEdit(line, rect_t{}, sensor->label, sizeof(sensor->label));

      // Type
      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_TYPE, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_VSENSORTYPES, 0, 1, GET_DEFAULT(sensor->type),
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

      sensorParametersWindow = new FormWindow(form, rect_t{});
      updateSensorParametersWindow();
    }
};

ModelTelemetryPage::ModelTelemetryPage() :
  PageTab(STR_MENUTELEMETRY, ICON_MODEL_TELEMETRY)
{
}

void ModelTelemetryPage::checkEvents()
{
  int _lastKnownIndex = availableTelemetryIndex();

  if (lastKnownIndex >= 0 && lastKnownIndex != _lastKnownIndex) {
    rebuild(window);
    lastKnownIndex = _lastKnownIndex;
  }

  PageTab::checkEvents();
}

void ModelTelemetryPage::rebuild(FormWindow * window, int8_t focusSensorIndex)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());  
  window->clear();
  build(window, focusSensorIndex);
  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
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

#if LCD_W > LCD_H
#define B_HEIGHT    32
#else
#define B_HEIGHT    44
#endif

void ModelTelemetryPage::build(FormWindow * window, int8_t focusSensorIndex)
{
  window->padAll(4);
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);

  this->window = window;

  FlexGridLayout grid(col_dsc, row_dsc, 2);
  FlexGridLayout grid2(col_dsc2, row_dsc, 2);

  // Sensors
  auto line = window->newLine(&grid2);
  lv_obj_set_style_pad_row(line->getLvObj(), 0, 0);
  lv_obj_set_style_pad_column(line->getLvObj(), 4, 0);
  auto subttl = new Subtitle(line, rect_t{}, STR_TELEMETRY_SENSORS, 0, COLOR_THEME_PRIMARY1);

#if LCD_H > LCD_W
  lv_obj_set_grid_cell(subttl->getLvObj(), LV_GRID_ALIGN_START, 0, 3, LV_GRID_ALIGN_CENTER, 0, 1);
  line = window->newLine(&grid2);
  lv_obj_set_style_pad_row(line->getLvObj(), 0, 0);
  lv_obj_set_style_pad_column(line->getLvObj(), 4, 0);
  grid2.nextCell();
#endif

  // Sensors columns titles
  uint8_t sensorsCount = getTelemetrySensorsCount();
  if (sensorsCount > 0) {
    new StaticText(line, rect_t{}, STR_NAME, 0, FONT(XS) | COLOR_THEME_PRIMARY1);
    grid2.nextCell();
    new StaticText(line, rect_t{}, STR_VALUE, 0, FONT(XS) | COLOR_THEME_PRIMARY1);
    if (!g_model.ignoreSensorIds) {
      new StaticText(line, rect_t{}, STR_ID, 0, FONT(XS) | COLOR_THEME_PRIMARY1);
    }
  }

  FlexGridLayout grid3(col_dsc3, row_dsc, 2);
 
  for (uint8_t idx = 0; idx < MAX_TELEMETRY_SENSORS; idx++) {
    if (g_model.telemetrySensors[idx].isAvailable()) {
      line = window->newLine(&grid3);
      auto button = new SensorButton(line, rect_t{0, 0, LCD_W-12, 25}, idx);

      button->setPressHandler([=]() -> uint8_t {
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
        lv_group_focus_obj(button->getLvObj());
      }
    }
  }

  FlexGridLayout grid4(col_dsc4, row_dsc, 4);

  // Autodiscover button
  line = window->newLine(&grid4);
  auto discover = new TextButton(line, rect_t{}, (allowNewSensors) ? STR_STOP_DISCOVER_SENSORS : STR_DISCOVER_SENSORS);
  discover->setPressHandler([=]() {
    allowNewSensors = !allowNewSensors;
    if (allowNewSensors) {
      discover->setText(STR_STOP_DISCOVER_SENSORS);
      return 1;
    } else {
      discover->setText(STR_DISCOVER_SENSORS);
      return 0;
    }
  });
  lv_obj_set_grid_cell(discover->getLvObj(), LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  discover->check(allowNewSensors);

  // New sensor button
  auto b = new TextButton(line, rect_t{}, STR_TELEMETRY_NEWSENSOR,
                 [=]() -> uint8_t {
                   int idx = availableTelemetryIndex();
                   if (idx >= 0)
                     editSensor(window, idx);
                   else
                     new FullScreenDialog(WARNING_TYPE_ALERT, "", STR_TELEMETRYFULL);
                   return 0;
                 });
  lv_obj_set_grid_cell(b->getLvObj(), LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  if (sensorsCount > 0) {
#if TWOCOLBUTTONS
    line = window->newLine(&grid4);
#endif
    // Delete all sensors button
    b = new TextButton(line, rect_t{}, STR_DELETE_ALL_SENSORS,
                   []() -> uint8_t {
                       new FullScreenDialog(WARNING_TYPE_CONFIRM, STR_CONFIRMDELETE, "", "", [=]() {
                           for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
                             delTelemetryIndex(i);
                           }
                       });
                       return 0;
                   });
#if TWOCOLBUTTONS
    b->setWidth((LCD_W - 16) / 2);
    lv_obj_set_grid_cell(b->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_CENTER, 0, 1);
#else
    lv_obj_set_grid_cell(b->getLvObj(), LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);
#endif
  }

  // Ignore instance button
  line = window->newLine(&grid);
  line->padLeft(10);
  new StaticText(line, rect_t{}, STR_IGNORE_INSTANCE, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_model.ignoreSensorIds));

  // RSSI
  line = window->newLine(&grid);
  new Subtitle(line, rect_t{}, getRssiLabel(), 0, COLOR_THEME_PRIMARY1);

  line = window->newLine(&grid);
  line->padLeft(10);
  new StaticText(line, rect_t{}, STR_LOWALARM, 0, COLOR_THEME_PRIMARY1);
  new NumberEdit(line, rect_t{}, 0, 100, GET_SET_DEFAULT(g_model.rfAlarms.warning));

  line = window->newLine(&grid);
  line->padLeft(10);
  new StaticText(line, rect_t{}, STR_CRITICALALARM, 0, COLOR_THEME_PRIMARY1);
  new NumberEdit(line, rect_t{}, 0, 100, GET_SET_DEFAULT(g_model.rfAlarms.critical));

  line = window->newLine(&grid);
  line->padLeft(10);
  new StaticText(line, rect_t{}, STR_DISABLE_ALARM, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_model.disableTelemetryWarning));

  // Vario
  line = window->newLine(&grid);
  new Subtitle(line, rect_t{}, STR_VARIO, 0, COLOR_THEME_PRIMARY1);

  FlexGridLayout grid5(col_dsc5, row_dsc, 4);

  line = window->newLine(&grid5);
  line->padLeft(10);
  new StaticText(line, rect_t{}, STR_SOURCE, 0, COLOR_THEME_PRIMARY1);
  auto choice = new SourceChoice(line, rect_t{}, MIXSRC_NONE, MIXSRC_LAST_TELEM,
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

  line = window->newLine(&grid5);
  line->padLeft(10);
  new StaticText(line, rect_t{}, STR_RANGE, 0, COLOR_THEME_PRIMARY1);

  auto vMin = new NumberEdit(line, rect_t{}, -17, 17, GET_SET_WITH_OFFSET(g_model.varioData.min, -10));
  vMin->setAvailableHandler([](int val) { return val < g_model.varioData.max + 10; });

  auto vMax = new NumberEdit(line, rect_t{}, -17, 17, GET_SET_WITH_OFFSET(g_model.varioData.max, 10));
  vMax->setAvailableHandler([](int val) { return g_model.varioData.min - 10 < val; });

  line = window->newLine(&grid5);
  line->padLeft(10);
  new StaticText(line, rect_t{}, STR_CENTER, 0, COLOR_THEME_PRIMARY1);

  auto cMin = new NumberEdit(line, rect_t{}, -15, 15, GET_SET_WITH_OFFSET(g_model.varioData.centerMin, -5), 0, PREC1);
  cMin->setAvailableHandler([](int val) { return val < g_model.varioData.centerMax + 5; });

  auto cMax = new NumberEdit(line, rect_t{}, -15, 15, GET_SET_WITH_OFFSET(g_model.varioData.centerMax, 5), 0, PREC1);
  cMax->setAvailableHandler([](int val) { return g_model.varioData.centerMin - 5 < val; });

  new Choice(line, rect_t{}, STR_VVARIOCENTER, 0, 1, GET_SET_DEFAULT(g_model.varioData.centerSilent));
}


// Version of 'drawSensorCustomValue' and related functions that returns strings instead of drawing to screen
// TODO: should this be moved somewhere else?

std::string getDate(TelemetryItem & telemetryItem)
{
  return
    formatNumberAsString(telemetryItem.datetime.day, LEADING0|LEFT, 2) + "-" +
    formatNumberAsString(telemetryItem.datetime.month, LEFT, 2) + "-" +
    formatNumberAsString(telemetryItem.datetime.year-2000, LEFT) + " " +
    formatNumberAsString(telemetryItem.datetime.hour, LEADING0|LEFT, 2) + ":" +
    formatNumberAsString(telemetryItem.datetime.min, LEADING0|LEFT, 2) + ":" +
    formatNumberAsString(telemetryItem.datetime.sec, LEADING0|LEFT, 2);
}

std::string getGPSCoord(int32_t value, const char * direction, bool seconds=true)
{
  char s[32] = {};
  uint32_t absvalue = abs(value);
  char * tmp = strAppendUnsigned(s, absvalue / 1000000);
  tmp = strAppend(tmp, "°");
  absvalue = absvalue % 1000000;
  absvalue *= 60;
  if (g_eeGeneral.gpsFormat == 0 || !seconds) {
    tmp = strAppendUnsigned(tmp, absvalue / 1000000, 2);
    *tmp++ = '\'';
    if (seconds) {
      absvalue %= 1000000;
      absvalue *= 60;
      absvalue /= 100000;
      tmp = strAppendUnsigned(tmp, absvalue / 10);
      *tmp++ = '.';
      tmp = strAppendUnsigned(tmp, absvalue % 10);
      *tmp++ = '"';
    }
  }
  else {
    tmp = strAppendUnsigned(tmp, absvalue / 1000000, 2);
    *tmp++ = '.';
    absvalue /= 1000;
    tmp = strAppendUnsigned(tmp, absvalue, 3);
  }
  *tmp++ = direction[value>=0 ? 0 : 1];
  *tmp = '\0';
  return std::string(s);
}

std::string getGPSSensorValue(TelemetryItem & telemetryItem, LcdFlags flags)
{
  if (flags & RIGHT)
    return getGPSCoord(telemetryItem.gps.longitude, "EW", true) + " " + getGPSCoord(telemetryItem.gps.latitude, "NS", true);

  return getGPSCoord(telemetryItem.gps.latitude, "NS", true) + " " + getGPSCoord(telemetryItem.gps.longitude, "EW", true);
}

std::string getSensorCustomValue(uint8_t sensor, int32_t value, LcdFlags flags)
{
  TelemetryItem & telemetryItem = telemetryItems[sensor];
  TelemetrySensor & telemetrySensor = g_model.telemetrySensors[sensor];

  if (telemetrySensor.unit == UNIT_DATETIME) {
    return getDate(telemetryItem);
  }
  else if (telemetrySensor.unit == UNIT_GPS) {
    return getGPSSensorValue(telemetryItem, flags);
  }
  else if (telemetrySensor.unit == UNIT_TEXT) {
    return std::string(telemetryItem.text);
  }
  else {
    if (telemetrySensor.prec > 0) {
      flags |= (telemetrySensor.prec==1 ? PREC1 : PREC2);
    }
    return getValueWithUnit(value, telemetrySensor.unit == UNIT_CELLS ? UNIT_VOLTS : telemetrySensor.unit, flags);
  }

  return std::string("");
}
