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

#include "view_channels.h"

#include "channel_bar.h"
#include "libopenui.h"
#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "model_select.h"
#include "opentx.h"
#include "view_logical_switches.h"

constexpr coord_t LEG_COLORBOX = 14;

//-----------------------------------------------------------------------------

class ChannelsViewFooter : public Window
{
 public:
  explicit ChannelsViewFooter(Window* parent) :
      Window(parent, {0, parent->height() - LEG_COLORBOX - 10, LCD_W,
                      LEG_COLORBOX + 10})
  {
    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

    auto w =
        new Window(this, {6, 4, LEG_COLORBOX + 2, LEG_COLORBOX + 2});
    w->setWindowFlag(NO_FOCUS);
    etx_solid_bg(w->getLvObj(), COLOR_THEME_SECONDARY3_INDEX);
    w = new Window(w, {1, 1, LEG_COLORBOX, LEG_COLORBOX});
    w->setWindowFlag(NO_FOCUS);
    etx_solid_bg(w->getLvObj(), COLOR_THEME_ACTIVE_INDEX);

    new StaticText(this, {24, 2, LV_SIZE_CONTENT, 16}, STR_MONITOR_OUTPUT_DESC, COLOR_THEME_PRIMARY2 | LEFT);

    int x = getTextWidth(STR_MONITOR_OUTPUT_DESC) + 32;

    w = new Window(this, {x + 6, 4, LEG_COLORBOX + 2, LEG_COLORBOX + 2});
    w->setWindowFlag(NO_FOCUS);
    etx_solid_bg(w->getLvObj(), COLOR_THEME_SECONDARY3_INDEX);
    w = new Window(w, {1, 1, LEG_COLORBOX, LEG_COLORBOX});
    w->setWindowFlag(NO_FOCUS);
    etx_solid_bg(w->getLvObj(), COLOR_THEME_FOCUS_INDEX);

    new StaticText(this, {x + 24, 2, LV_SIZE_CONTENT, 16},
                   STR_MONITOR_MIXER_DESC, COLOR_THEME_PRIMARY2);
  }
};

//-----------------------------------------------------------------------------

class ChannelsViewPage : public PageTab
{
 public:
  explicit ChannelsViewPage(uint8_t pageIndex = 0) :
      PageTab(STR_MONITOR_CHANNELS[pageIndex],
              (EdgeTxIcon)(ICON_MONITOR_CHANNELS1 + pageIndex)),
      pageIndex(pageIndex)
  {
  }

 protected:
  uint8_t pageIndex = 0;

  void build(Window* window) override
  {
    constexpr coord_t hmargin = 5;
    window->padAll(PAD_ZERO);

    // Channels bars
    for (uint8_t chan = pageIndex * 8; chan < 8 + pageIndex * 8; chan++) {
#if LCD_H > LCD_W
      coord_t width = window->width() - (hmargin * 2);
      coord_t xPos = hmargin;
      coord_t yPos = (chan % 8) *
                     ((window->height() - 24) / 8);
#else
      coord_t width = window->width() / 2 - (hmargin * 2);
      coord_t xPos = (chan % 8) >= 4 ? width + (hmargin * 2) : hmargin;
      coord_t yPos = (chan % 4) *
                     ((window->height() - 23) / 4);
#endif
      new ComboChannelBar(window, {xPos, yPos, width, 3 * BAR_HEIGHT + 3},
                          chan);
    }

    // Footer
    new ChannelsViewFooter(window);
  }
};

//-----------------------------------------------------------------------------

ChannelsViewMenu::ChannelsViewMenu(ModelMenu* parent) :
    TabsGroup(ICON_MONITOR), parentMenu(parent)
{
  addTab(new ChannelsViewPage(0));
  addTab(new ChannelsViewPage(1));
  addTab(new ChannelsViewPage(2));
  addTab(new ChannelsViewPage(3));
  addTab(new LogicalSwitchesViewPage());
}

#if defined(HARDWARE_KEYS)
void ChannelsViewMenu::onPressSYS()
{
  onCancel();
  if (parentMenu) parentMenu->onCancel();
  new RadioMenu();
}
void ChannelsViewMenu::onLongPressSYS()
{
  onCancel();
  if (parentMenu) parentMenu->onCancel();
  // Radio setup
  (new RadioMenu())->setCurrentTab(2);
}
void ChannelsViewMenu::onPressMDL()
{
  onCancel();
  if (!parentMenu) {
    new ModelMenu();
  }
}
void ChannelsViewMenu::onLongPressMDL()
{
  onCancel();
  if (parentMenu) parentMenu->onCancel();
  new ModelLabelsWindow();
}
void ChannelsViewMenu::onPressTELE()
{
  onCancel();
  if (parentMenu) parentMenu->onCancel();
  new ScreenMenu();
}
#endif
