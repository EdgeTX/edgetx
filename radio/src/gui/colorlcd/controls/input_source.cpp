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
#include "getset_helpers.h"
#include "numberedit.h"
#include "sourcechoice.h"
#include "static.h"
#include "switchchoice.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

// Inverted sources are stored as negative srcRaw values
static bool isTelemetrySource(int16_t srcRaw)
{
  auto src = abs(srcRaw);
  return src >= MIXSRC_FIRST_TELEM && src <= MIXSRC_LAST_TELEM;
}

static LcdFlags getSensorPrec(int16_t srcRaw)
{
  LcdFlags prec = 0;
  if (isTelemetrySource(srcRaw)) {
    uint8_t sensorIndex = (abs(srcRaw) - MIXSRC_FIRST_TELEM) / 3;
    const TelemetrySensor& sensor = g_model.telemetrySensors[sensorIndex];
    if (sensor.prec > 0) {
      prec |= (sensor.prec == 1 ? PREC1 : PREC2);
    }
  }
  return prec;
}

static int getMaxScale(int16_t srcRaw)
{
  if (!isTelemetrySource(srcRaw)) return 0;
  return maxTelemValue(abs(srcRaw) - MIXSRC_FIRST_TELEM + 1);
}

class SensorValue : public StaticText
{
 public:
  SensorValue(Window *parent, const rect_t &rect, ExpoData *input) :
      StaticText(parent, rect), input(input)
  {
  }

  void checkEvents() override
  {
    if (lv_obj_has_flag(lvobj, LV_OBJ_FLAG_HIDDEN)) return;

    // TODO: check for telemetry available
    if (isTelemetrySource(input->srcRaw)) {
      // Signed source: show the (possibly inverted) value fed to the mixer
      getvalue_t sensorVal = getValue(input->srcRaw);
      if (lastSensorVal != sensorVal) {
        lastSensorVal = sensorVal;
        setText(std::to_string(lastSensorVal));
      }
    } else {
      setText("---");
    }
  }

 protected:
  getvalue_t lastSensorVal = INT32_MAX;
  ExpoData *input;
};

static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
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

  // Value
  auto line = sensor_form->newLine(grid);
  line->padAll(PAD_ZERO);
  line->setWidth(SENSOR_W);

  new StaticText(line, rect_t{}, STR_VALUE);
  new SensorValue(line, rect_t{}, input);

  // Scale
  line = sensor_form->newLine(grid);
  line->padAll(PAD_OUTLINE);
  line->setWidth(SENSOR_W);

  new StaticText(line, rect_t{}, STR_SCALE);
  scale_edit = new NumberEdit(line, rect_t{}, 0, getMaxScale(input->srcRaw),
                              GET_SET_DEFAULT(input->scale),
                              getSensorPrec(input->srcRaw));
  // Fill the column so values with a unit suffix fit on all screen sizes
  lv_obj_set_style_grid_cell_x_align(scale_edit->getLvObj(),
                                     LV_GRID_ALIGN_STRETCH, 0);
  // Show the scale with the selected sensor's precision and unit
  scale_edit->setDisplayHandler([=](int value) -> std::string {
    if (!isTelemetrySource(input->srcRaw)) return std::to_string(value);
    mixsrc_t src = abs(input->srcRaw);
    return getSourceCustomValueString(
        src, convertTelemValue(src - MIXSRC_FIRST_TELEM + 1, value), 0);
  });

  update();
}

void InputSource::update()
{
  if (input->srcRaw > MIXSRC_LAST_STICK && input->trimSource == TRIM_ON) {
    input->trimSource = TRIM_OFF;
  }

  bool isTelem = isTelemetrySource(input->srcRaw);

  if (scale_edit && isTelem) {
    // Range and precision depend on the selected sensor
    scale_edit->setMax(getMaxScale(input->srcRaw));
    scale_edit->clearTextFlag(PREC1 | PREC2);
    scale_edit->setTextFlag(getSensorPrec(input->srcRaw));
    scale_edit->update();
  }

  if (sensor_form) sensor_form->show(isTelem);
}
