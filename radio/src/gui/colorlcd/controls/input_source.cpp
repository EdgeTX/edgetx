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

#include "input_source.h"

#include "edgetx.h"
#include "sourcechoice.h"
#include "switchchoice.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

class SensorValue : public StaticText
{
 public:
  SensorValue(Window *parent, const rect_t &rect, ExpoData *input) :
      StaticText(parent, rect), input(input)
  {
  }

  bool isTelemetryValue()
  {
    return input->srcRaw >= MIXSRC_FIRST_TELEM &&
           input->srcRaw <= MIXSRC_LAST_TELEM;
  }

  void checkEvents() override
  {
    if (lv_obj_has_flag(lvobj, LV_OBJ_FLAG_HIDDEN)) return;

    // TODO: check for telemetry available
    if (isTelemetryValue()) {
      getvalue_t sensorVal = getValue(input->srcRaw);
      if (lastSensorVal != sensorVal) {
        lastSensorVal = sensorVal;
        setText(std::to_string(lastSensorVal));
      }
    } else {
      setText("---");
    }
  }

  LcdFlags getSensorPrec()
  {
    LcdFlags prec = 0;
    if (isTelemetryValue()) {
      uint8_t sensorIndex = (input->srcRaw - MIXSRC_FIRST_TELEM) / 3;
      TelemetrySensor sensor = g_model.telemetrySensors[sensorIndex];
      if (sensor.prec > 0) {
        prec |= (sensor.prec == 1 ? PREC1 : PREC2);
      }
    }
    return prec;
  }

 protected:
  getvalue_t lastSensorVal;
  ExpoData *input;
};

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

InputSource::InputSource(Window *parent, ExpoData *input) :
    Window(parent, rect_t{}), input(input)
{
  padAll(PAD_TINY);
  lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_size(lvobj, lv_pct(100), LV_SIZE_CONTENT);

  new SourceChoice(
      this, rect_t{}, INPUTSRC_FIRST, INPUTSRC_LAST, GET_DEFAULT(input->srcRaw),
      [=](int32_t newValue) {
        input->srcRaw = newValue;
        update();
        SET_DIRTY();
      }, true);

  sensor_form = new Window(this, rect_t{});
  sensor_form->padAll(PAD_TINY);
  sensor_form->setFlexLayout();

  FlexGridLayout grid(col_dsc, row_dsc);
  auto line = sensor_form->newLine(grid);
  line->padAll(PAD_ZERO);

  // Value
  new StaticText(line, rect_t{}, STR_VALUE);
  auto sensor = new SensorValue(line, rect_t{}, input);

  // Scale
  line = sensor_form->newLine(grid);
  line->padAll(PAD_TINY);
  new StaticText(line, rect_t{}, STR_SCALE);
  new NumberEdit(line, rect_t{0, 0, 60, 0}, 0,
                 maxTelemValue(input->srcRaw - MIXSRC_FIRST_TELEM + 1),
                 GET_SET_DEFAULT(input->scale), sensor->getSensorPrec());

  update();
}

void InputSource::update()
{
  if (input->srcRaw > MIXSRC_LAST_STICK && input->trimSource == TRIM_ON) {
    input->trimSource = TRIM_OFF;
  }

  if (sensor_form)
    sensor_form->show(input->srcRaw >= MIXSRC_FIRST_TELEM &&
                      input->srcRaw <= MIXSRC_LAST_TELEM);
}
