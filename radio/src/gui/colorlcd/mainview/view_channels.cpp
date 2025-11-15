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
#include "model_select.h"
#include "edgetx.h"

//-----------------------------------------------------------------------------

class ChannelsViewFooter : public Window
{
 public:
  explicit ChannelsViewFooter(Window* parent) :
      Window(parent, {0, parent->height() - FOOTER_H, LCD_W, FOOTER_H})
  {
    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

    auto w =
        new Window(this, {PAD_MEDIUM, PAD_SMALL, LEG_COLORBOX + PAD_TINY, LEG_COLORBOX + PAD_TINY});
    w->setWindowFlag(NO_FOCUS);
    etx_solid_bg(w->getLvObj(), COLOR_THEME_SECONDARY3_INDEX);
    w = new Window(w, {1, 1, LEG_COLORBOX, LEG_COLORBOX});
    w->setWindowFlag(NO_FOCUS);
    etx_solid_bg(w->getLvObj(), COLOR_THEME_ACTIVE_INDEX);

    new StaticText(this, {LEG_COLORBOX + PAD_MEDIUM + PAD_SMALL, PAD_TINY, LV_SIZE_CONTENT, TXT_H}, STR_MONITOR_OUTPUT_DESC, COLOR_THEME_PRIMARY2_INDEX);

    int x = getTextWidth(STR_MONITOR_OUTPUT_DESC) + LEG_COLORBOX + PAD_MEDIUM * 2;

    w = new Window(this, {x + PAD_MEDIUM, PAD_SMALL, LEG_COLORBOX + PAD_TINY, LEG_COLORBOX + PAD_TINY});
    w->setWindowFlag(NO_FOCUS);
    etx_solid_bg(w->getLvObj(), COLOR_THEME_SECONDARY3_INDEX);
    w = new Window(w, {1, 1, LEG_COLORBOX, LEG_COLORBOX});
    w->setWindowFlag(NO_FOCUS);
    etx_solid_bg(w->getLvObj(), COLOR_THEME_FOCUS_INDEX);

    new StaticText(this, {x + LEG_COLORBOX + PAD_MEDIUM + PAD_SMALL, PAD_TINY, LV_SIZE_CONTENT, TXT_H},
                   STR_MONITOR_MIXER_DESC, COLOR_THEME_PRIMARY2_INDEX);
  }

  static LAYOUT_VAL_SCALED(LEG_COLORBOX, 14)
  static LAYOUT_VAL_SCALED(TXT_H, 18)
  static constexpr coord_t FOOTER_H = LEG_COLORBOX + PAD_SMALL * 2 + PAD_TINY;
};

//-----------------------------------------------------------------------------

class ChannelsViewPage : public PageGroupItem
{
 public:
  explicit ChannelsViewPage(uint8_t startChan, int rows, int cols, const char* title) :
      PageGroupItem(title), startChan(startChan), rows(rows), cols(cols)
  {
    icon = ICON_MONITOR;
  }

  static constexpr coord_t CHANS_H = 3 * ChannelBar::BAR_HEIGHT + PAD_THREE;

 protected:
  uint8_t startChan;
  int rows;
  int cols;

  void build(Window* window) override
  {
    window->padAll(PAD_ZERO);

#if PORTRAIT
    coord_t w = window->width() - (PAD_SMALL * 2);
#else
    coord_t w = window->width() / 2 - (PAD_SMALL * 2);
#endif

    // Channels bars
    for (uint8_t i = 0, j = 0; j < rows * cols; i += 1) {
      uint8_t chan = startChan + i;
      if (chan >= MAX_OUTPUT_CHANNELS) break;
      if (isChannelUsed(chan)) {
#if PORTRAIT
        coord_t xPos = PAD_SMALL;
        coord_t yPos = j * ((window->height() - PAD_LARGE * 3) / rows);
#else
        coord_t xPos = (j & 1) ? w + (PAD_SMALL * 2) : PAD_SMALL;
        coord_t yPos = (j / cols) * ((window->height() - ChannelsViewFooter::FOOTER_H) / rows);
#endif
        new ComboChannelBar(window, {xPos, yPos, w, CHANS_H}, chan);

        j += 1;
      }
    }

    // Footer
    new ChannelsViewFooter(window);
  }
};

//-----------------------------------------------------------------------------

ChannelsViewMenu::ChannelsViewMenu() :
    TabsGroup(ICON_MONITOR, STR_MAIN_MENU_CHANNEL_MONITOR)
{
  QuickMenu::setCurrentPage(QM_TOOLS_CHAN_MON);

#if PORTRAIT
    int cols = 1;
    int rows = 8;
#else
    int cols = 2;
    int rows = (LCD_H - EdgeTxStyles::MENU_HEADER_HEIGHT - ChannelsViewFooter::FOOTER_H) / ChannelsViewPage::CHANS_H;
#endif

  int pages = 0;
  int chansPerPage = rows * cols;

  char s[50];

  for (int i = 0; i < MAX_OUTPUT_CHANNELS;) {
    int start = i;
    while (!isChannelUsed(start)) {
      start += 1;
      if (start >= MAX_OUTPUT_CHANNELS) break;
    }
    if (start >= MAX_OUTPUT_CHANNELS) break;
    int count = 1;
    int last = start;
    int end = start + 1;
    while (end < MAX_OUTPUT_CHANNELS && count < chansPerPage) {
      if (isChannelUsed(end)) {
        count += 1;
        last = end;
      }
      end += 1;
    }
    sprintf(s, STR_MONITOR_CHANNELS, start + 1, last + 1);
    addTab(new ChannelsViewPage(start, rows, cols, s));
    pages += 1;
    i = end;
  }

  if (pages < 2) hidePageButtons();
}
