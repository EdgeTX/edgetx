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

#include "bind_menu_d16.h"
#include "opentx.h"

#if defined(MULTIMODULE)
  #include "telemetry/multi.h"
#endif

BindChoiceMenu::BindChoiceMenu(Window *parent, uint8_t moduleIdx,
                               std::function<void()> onPress,
                               std::function<void()> onCancel) :
    Menu(parent), moduleIdx(moduleIdx), onPressHandler(std::move(onPress))
{
  if (isTelemAllowedOnBind(moduleIdx)) {
    addLine(STR_BINDING_1_8_TELEM_ON, [=]() { onSelect(Bind_1_8_TELEM_ON); });
  }
  addLine(STR_BINDING_1_8_TELEM_OFF, [=]() { onSelect(Bind_1_8_TELEM_OFF); });

  if (isBindCh9To16Allowed(moduleIdx)) {
    if (isTelemAllowedOnBind(moduleIdx)) {
      addLine(STR_BINDING_9_16_TELEM_ON,
              [=]() { onSelect(Bind_9_16_TELEM_ON); });
    }
    addLine(STR_BINDING_9_16_TELEM_OFF,
            [=]() { onSelect(Bind_9_16_TELEM_OFF); });
  }
  setTitle(STR_SELECT_MODE);
  setCancelHandler([=] {
    moduleState[moduleIdx].mode = MODULE_MODE_NORMAL;
    onCancel();
  });
}

void BindChoiceMenu::onSelect(BindChanMode mode)
{
  bool receiverTelemetry = false;
  bool receiverHigherChannels = false;

  switch (mode) {
    case Bind_1_8_TELEM_ON:
      receiverTelemetry = true;
      break;
    case Bind_1_8_TELEM_OFF:
      break;
    case Bind_9_16_TELEM_ON:
      receiverTelemetry = true;
      receiverHigherChannels = true;
      break;
    case Bind_9_16_TELEM_OFF:
      receiverHigherChannels = true;
      break;
  }

  ModuleData* md = &g_model.moduleData[moduleIdx];
#if defined(MULTIMODULE)
  if (isModuleMultimodule(moduleIdx)) {
    md->multi.receiverTelemetryOff = !receiverTelemetry;
    md->multi.receiverHigherChannels = receiverHigherChannels;
    setMultiBindStatus(moduleIdx, MULTI_BIND_INITIATED);
  } else
#endif
  {
    md->pxx.receiverTelemetryOff = !receiverTelemetry;
    md->pxx.receiverHigherChannels = receiverHigherChannels;
  }

  moduleState[moduleIdx].mode = MODULE_MODE_BIND;
  if (onPressHandler) onPressHandler();
}
