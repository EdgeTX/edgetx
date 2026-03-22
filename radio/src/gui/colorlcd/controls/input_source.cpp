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

// Defined in mixer.cpp
extern mixsrc_t sourceRefToMixSrc(const SourceRef& ref);

// Convert legacy mixsrc_t back to SourceRef
static SourceRef mixSrcToSourceRef(mixsrc_t src)
{
  SourceRef ref = {};
  if (src == MIXSRC_NONE) return ref;

  bool inverted = (src < 0);
  mixsrc_t absSrc = inverted ? -src : src;

  struct Range { mixsrc_t first; mixsrc_t last; uint8_t type; };
  static const Range ranges[] = {
    {MIXSRC_FIRST_INPUT, MIXSRC_LAST_INPUT, SOURCE_TYPE_INPUT},
    {MIXSRC_FIRST_LUA, MIXSRC_LAST_LUA, SOURCE_TYPE_LUA},
    {MIXSRC_FIRST_STICK, MIXSRC_LAST_STICK, SOURCE_TYPE_STICK},
    {MIXSRC_FIRST_POT, MIXSRC_LAST_POT, SOURCE_TYPE_POT},
    {MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM, SOURCE_TYPE_TRIM},
    {MIXSRC_FIRST_SWITCH, MIXSRC_LAST_SWITCH, SOURCE_TYPE_SWITCH},
    {MIXSRC_FIRST_LOGICAL_SWITCH, MIXSRC_LAST_LOGICAL_SWITCH, SOURCE_TYPE_LOGICAL_SWITCH},
    {MIXSRC_FIRST_TRAINER, MIXSRC_LAST_TRAINER, SOURCE_TYPE_TRAINER},
    {MIXSRC_FIRST_CH, MIXSRC_LAST_CH, SOURCE_TYPE_CHANNEL},
    {MIXSRC_FIRST_GVAR, MIXSRC_LAST_GVAR, SOURCE_TYPE_GVAR},
    {MIXSRC_FIRST_TIMER, MIXSRC_LAST_TIMER, SOURCE_TYPE_TIMER},
    {MIXSRC_FIRST_TELEM, MIXSRC_LAST_TELEM, SOURCE_TYPE_TELEMETRY},
    {MIXSRC_FIRST_HELI, MIXSRC_LAST_HELI, SOURCE_TYPE_HELI},
  };

  if (absSrc == MIXSRC_MIN) { ref.type = SOURCE_TYPE_MIN; }
  else if (absSrc == MIXSRC_MAX) { ref.type = SOURCE_TYPE_MAX; }
  else if (absSrc == MIXSRC_TX_VOLTAGE) { ref.type = SOURCE_TYPE_TX_VOLTAGE; }
  else if (absSrc == MIXSRC_TX_TIME) { ref.type = SOURCE_TYPE_TX_TIME; }
  else {
    for (const auto& r : ranges) {
      if (absSrc >= r.first && absSrc <= r.last) {
        ref.type = r.type;
        ref.index = absSrc - r.first;
        break;
      }
    }
  }

  if (inverted) ref.flags = SOURCE_FLAG_INVERTED;
  return ref;
}

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
    mixsrc_t src = sourceRefToMixSrc(input->srcRaw);
    return src >= MIXSRC_FIRST_TELEM &&
           src <= MIXSRC_LAST_TELEM;
  }

  void checkEvents() override
  {
    if (lv_obj_has_flag(lvobj, LV_OBJ_FLAG_HIDDEN)) return;

    // TODO: check for telemetry available
    if (isTelemetryValue()) {
      getvalue_t sensorVal = getValue(sourceRefToMixSrc(input->srcRaw));
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
      mixsrc_t src = sourceRefToMixSrc(input->srcRaw);
      uint8_t sensorIndex = (src - MIXSRC_FIRST_TELEM) / 3;
      TelemetrySensor sensor = g_model.telemetrySensors[sensorIndex];
      if (sensor.prec > 0) {
        prec |= (sensor.prec == 1 ? PREC1 : PREC2);
      }
    }
    return prec;
  }

 protected:
  getvalue_t lastSensorVal = INT32_MAX;
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
      this, rect_t{}, INPUTSRC_FIRST, INPUTSRC_LAST,
      [=]() -> int16_t { return (int16_t)sourceRefToMixSrc(input->srcRaw); },
      [=](int16_t newValue) {
        input->srcRaw = mixSrcToSourceRef((mixsrc_t)newValue);
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
  auto sensor = new SensorValue(line, rect_t{}, input);

  // Scale
  line = sensor_form->newLine(grid);
  line->padAll(PAD_OUTLINE);
  line->setWidth(SENSOR_W);

  new StaticText(line, rect_t{}, STR_SCALE);
  mixsrc_t srcRawLegacy = sourceRefToMixSrc(input->srcRaw);
  new NumberEdit(line, {0, 0, EdgeTxStyles::EDIT_FLD_WIDTH_NARROW, 0}, 0,
                 maxTelemValue(srcRawLegacy - MIXSRC_FIRST_TELEM + 1),
                 GET_SET_DEFAULT(input->scale), sensor->getSensorPrec());

  update();
}

void InputSource::update()
{
  mixsrc_t src = sourceRefToMixSrc(input->srcRaw);
  if (src > MIXSRC_LAST_STICK && input->trimSource == TRIM_ON) {
    input->trimSource = TRIM_OFF;
  }

  if (sensor_form)
    sensor_form->show(src >= MIXSRC_FIRST_TELEM &&
                      src <= MIXSRC_LAST_TELEM);
}
