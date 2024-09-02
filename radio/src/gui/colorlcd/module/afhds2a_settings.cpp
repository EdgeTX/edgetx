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

#include "afhds2a_settings.h"
#include "edgetx.h"

#include "pulses/flysky.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

class FSProtoOpts : public Window
{
  std::function<uint8_t()> _getMode;
  std::function<void(uint8_t)> _setMode;
  
public:
  FSProtoOpts(Window* parent, std::function<uint8_t()> getMode,
              std::function<void(uint8_t)> setMode);
};

FSProtoOpts::FSProtoOpts(Window* parent, std::function<uint8_t()> getMode,
                         std::function<void(uint8_t)> setMode) :
  Window(parent, rect_t{}),
  _getMode(std::move(getMode)),
  _setMode(std::move(setMode))
{
  setFlexLayout(LV_FLEX_FLOW_ROW);

  // PPM / PWM
  new Choice(
      this, rect_t{}, STR_FLYSKY_PULSE_PROTO, 0, 1,
      [=]() -> int { return _getMode() >> 1; },
      [=](int v) {
        _setMode((_getMode() & 1) | ((v & 1) << 1));
        SET_DIRTY();
      });

  // SBUS / iBUS
  new Choice(
      this, rect_t{}, STR_FLYSKY_SERIAL_PROTO, 0, 1,
      [=]() -> int { return _getMode() & 1; },
      [=](int v) {
        _setMode((_getMode() & 2) | (v & 1));
        SET_DIRTY();
      });
}

AFHDS2ASettings::AFHDS2ASettings(Window* parent, const FlexGridLayout& g,
                               uint8_t moduleIdx) :
    Window(parent, rect_t{}),
    moduleIdx(moduleIdx),
    md(&g_model.moduleData[moduleIdx]),
    grid(g)
{
  setFlexLayout();

  FormLine* line;

  // RX options:
  line = newLine(grid);
  afhds2OptionsLabel = new StaticText(line, rect_t{}, STR_OPTIONS);

  afhds2ProtoOpts = new FSProtoOpts(
                                    line, [=]() { return md->flysky.mode; },
                                    [=](uint8_t v) { md->flysky.mode = v; });

#if defined(PCBNV14)
  line = newLine(grid);
  static const char* _rf_power[] = {"Default", "High"};
  afhds2RFPowerText = new StaticText(line, rect_t{}, STR_MULTI_RFPOWER);
  afhds2RFPowerChoice = new Choice(line, rect_t{}, _rf_power, 0, 1,
                                   GET_DEFAULT(md->flysky.rfPower),
                                   [=](int32_t newValue) -> void {
                                     md->flysky.rfPower = newValue;
                                     resetPulsesAFHDS2();
                                   });
#endif

  hideAFHDS2Options();
}

void AFHDS2ASettings::hideAFHDS2Options()
{
  afhds2OptionsLabel->hide();
  afhds2ProtoOpts->hide();
#if defined(PCBNV14)
  afhds2RFPowerText->hide();
  afhds2RFPowerChoice->hide();
#endif
}

void AFHDS2ASettings::showAFHDS2Options()
{
  afhds2OptionsLabel->show();
  afhds2ProtoOpts->show();
#if defined(PCBNV14)
  bool showRFPower = (getNV14RfFwVersion() >= 0x1000E);
  afhds2RFPowerText->show(showRFPower);
  afhds2RFPowerChoice->show(showRFPower);
  if (showRFPower && (showRFPower != hasRFPower)) {
    hasRFPower = showRFPower;
    afhds2RFPowerChoice->update();
  }
#endif
}

void AFHDS2ASettings::checkEvents()
{
  Window::checkEvents();
  update();
}

void AFHDS2ASettings::update()
{
  if (isModuleAFHDS2A(moduleIdx)) {
    showAFHDS2Options();
  } else {
    hideAFHDS2Options();
  }
}
