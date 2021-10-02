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

#include "menu_screen.h"
#include "layout.h"
#include "datastructs.h"

class ScreenUserInterfacePage: public PageTab {
  public:
    ScreenUserInterfacePage(ScreenMenu* menu);

    void build(FormWindow * window) override;

  protected:
    ScreenMenu * menu;
};

class ScreenAddPage: public PageTab {
  public:
    ScreenAddPage(ScreenMenu * menu, uint8_t pageIndex);

    void build(FormWindow * window) override;

  protected:
    ScreenMenu * menu;
    uint8_t pageIndex;
};

class ScreenSetupPage: public PageTab {
  public:
    ScreenSetupPage(ScreenMenu * menu, unsigned pageIndex, unsigned customScreenIndex);

    void build(FormWindow * window) override;

  protected:
    ScreenMenu * menu;
    unsigned pageIndex;
    unsigned customScreenIndex;
};

class SetupWidgetsPageSlot: public Button
{
  public:
    SetupWidgetsPageSlot(FormGroup * parent, const rect_t & rect, WidgetsContainer* container, uint8_t slotIndex);

    void paint(BitmapBuffer * dc) override;
};
