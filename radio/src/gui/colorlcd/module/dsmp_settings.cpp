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

#include "dsmp_settings.h"

#include "choice.h"
#include "edgetx.h"
#include "getset_helpers.h"
#include "toggleswitch.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

struct DSMPEnableAETR : public FormLine {
  DSMPEnableAETR(Window* form, FlexGridLayout& layout, uint8_t moduleIdx) :
      FormLine(form, layout)
  {
    new StaticText(this, rect_t{}, STR_DSMP_ENABLE_AETR);

    auto md = &g_model.moduleData[moduleIdx];

    cb = new ToggleSwitch(this, rect_t{}, GET_SET_DEFAULT(md->dsmp.enableAETR));
  }

  void update()
  {
    show();
    cb->update();
  }

 private:
  ToggleSwitch* cb;
};

DSMPSettings::DSMPSettings(Window* parent,
                           const FlexGridLayout& g,
                           uint8_t moduleIdx) :
    Window(parent, rect_t{}),
    md(&g_model.moduleData[moduleIdx]),
    moduleIdx(moduleIdx)
{
  FlexGridLayout grid(g);
  setFlexLayout();

  // DSMP status
  auto line = newLine(grid);
  new StaticText(line, rect_t{}, STR_MODULE_STATUS);
  new DynamicText(
      line, rect_t{},
      [=] {
        char msg[64] = "";
        getModuleStatusString(moduleIdx, msg);
        return std::string(msg);
      });

  enableAETR_line = new DSMPEnableAETR(this, grid, moduleIdx);

  // Ensure elements properly initalised
  update();
}

void DSMPSettings::update() { 
    enableAETR_line->update(); 
}
