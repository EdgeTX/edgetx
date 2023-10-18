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

constexpr coord_t CHANNEL_VIEW_FOOTER_HEIGHT = 16;

ChannelsViewMenu::ChannelsViewMenu(ModelMenu* parent) :
    TabsGroup(ICON_MONITOR), parentMenu(parent)
{
  addTab(new ChannelsViewPage(0));
  addTab(new ChannelsViewPage(1));
  addTab(new ChannelsViewPage(2));
  addTab(new ChannelsViewPage(3));
  addTab(new LogicalSwitchesViewPage());
}

void ChannelsViewMenu::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (event == EVT_KEY_BREAK(KEY_MODEL)) {
    killEvents(event);
    onCancel();
    if (!parentMenu) {
      new ModelMenu();
    }
  } else if (event == EVT_KEY_LONG(KEY_MODEL)) {
    killEvents(KEY_MODEL);
    onCancel();
    if (parentMenu) parentMenu->onCancel();
    new ModelLabelsWindow();
  } else if (event == EVT_KEY_BREAK(KEY_SYS)) {
    onCancel();
    if (parentMenu) parentMenu->onCancel();
    new RadioMenu();
  } else if (event == EVT_KEY_LONG(KEY_SYS)) {
    killEvents(KEY_SYS);
    onCancel();
    if (parentMenu) parentMenu->onCancel();
    // Radio setup
    (new RadioMenu())->setCurrentTab(2);
  } else if (event == EVT_KEY_BREAK(KEY_TELE)) {
    onCancel();
    if (parentMenu) parentMenu->onCancel();
    new ScreenMenu();
  } else if (event == EVT_KEY_LONG(KEY_TELE)) {
    killEvents(KEY_TELE);
  } else {
    TabsGroup::onEvent(event);
  }
#endif
}

class ChannelsViewFooter : public Window
{
 public:
  explicit ChannelsViewFooter(Window* parent) :
      Window(parent,
             {0, parent->height() - MODEL_SELECT_FOOTER_HEIGHT, LCD_W,
              MODEL_SELECT_FOOTER_HEIGHT},
             OPAQUE)
  {
  }

  // Draw single legend
  coord_t drawChannelsMonitorLegend(BitmapBuffer* dc, coord_t x, const char* s,
                                    int color)
  {
    dc->drawSolidFilledRect(x, 4, LEG_COLORBOX + 2, LEG_COLORBOX + 2,
                            COLOR_THEME_SECONDARY3);
    dc->drawSolidFilledRect(x + 1, 5, LEG_COLORBOX, LEG_COLORBOX, color);
    dc->drawText(x + 20, 4, s, COLOR_THEME_PRIMARY2);
    return x + 25 + getTextWidth(s);
  }

  void paint(BitmapBuffer* dc) override
  {
    // Draw legend bar
    coord_t x = 10;
    dc->drawSolidFilledRect(0, 0, width(), height(), COLOR_THEME_SECONDARY1);
    x = drawChannelsMonitorLegend(dc, MENUS_MARGIN_LEFT,
                                  STR_MONITOR_OUTPUT_DESC, COLOR_THEME_ACTIVE);
    drawChannelsMonitorLegend(dc, x, STR_MONITOR_MIXER_DESC, COLOR_THEME_FOCUS);
  }
};

void ChannelsViewPage::build(FormWindow* window)
{
  constexpr coord_t hmargin = 5;
  window->padAll(0);

  // Channels bars
  for (uint8_t chan = pageIndex * 8; chan < 8 + pageIndex * 8; chan++) {
#if LCD_H > LCD_W
    coord_t width = window->width() - (hmargin * 2);
    coord_t xPos = hmargin;
    coord_t yPos =
        (chan % 8) * ((window->height() - CHANNEL_VIEW_FOOTER_HEIGHT - 8) / 8);
#else
    coord_t width = window->width() / 2 - (hmargin * 2);
    coord_t xPos = (chan % 8) >= 4 ? width + (hmargin * 2) : hmargin;
    coord_t yPos =
        (chan % 4) * ((window->height() - CHANNEL_VIEW_FOOTER_HEIGHT - 4) / 4);
#endif
    new ComboChannelBar(window, {xPos, yPos, width, 3 * BAR_HEIGHT + 3}, chan);
  }

  // Footer
  new ChannelsViewFooter(window);
}
