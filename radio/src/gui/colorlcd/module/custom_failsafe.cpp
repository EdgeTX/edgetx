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

#include "custom_failsafe.h"

#include "channel_bar.h"
#include "edgetx.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

class ChannelFailsafeBargraph : public Window
{
 public:
  ChannelFailsafeBargraph(Window* parent, const rect_t& rect, uint8_t channel) :
      Window(parent, rect), channel(channel)
  {
    etx_obj_add_style(lvobj, styles->border_thin, LV_PART_MAIN);
    etx_obj_add_style(lvobj, styles->border_color[COLOR_BLACK_INDEX], LV_PART_MAIN);

    outputsBar = new OutputChannelBar(this, {0, 1, width() - 2, ChannelBar::BAR_HEIGHT},
                                      channel, false, false);
    outputsBar->hide();

    failsafeBar = new ChannelBar(
        this, {0, ChannelBar::BAR_HEIGHT + 3, width() - 2, ChannelBar::BAR_HEIGHT}, channel,
        [=] { return g_model.failsafeChannels[channel]; },
        COLOR_THEME_WARNING_INDEX);
    failsafeBar->hide();
  }

  void checkEvents() override
  {
    Window::checkEvents();

    outputsBar->show(
        g_model.failsafeChannels[channel] != FAILSAFE_CHANNEL_HOLD &&
        g_model.failsafeChannels[channel] != FAILSAFE_CHANNEL_NOPULSE);

    failsafeBar->show(
        g_model.failsafeChannels[channel] != FAILSAFE_CHANNEL_HOLD &&
        g_model.failsafeChannels[channel] != FAILSAFE_CHANNEL_NOPULSE);
  }

 protected:
  uint8_t channel;

  OutputChannelBar* outputsBar = nullptr;
  ChannelBar* failsafeBar = nullptr;
};

class ChannelFailsafeEdit : public NumberEdit
{
  uint8_t channel;

  std::string getString()
  {
    auto value = g_model.failsafeChannels[channel];
    if (value == FAILSAFE_CHANNEL_HOLD) {
      return STR_HOLD;
    } else if (value == FAILSAFE_CHANNEL_NOPULSE) {
      return STR_NONE;
    } else {
      if (g_eeGeneral.ppmunit == PPM_US) {
        value = PPM_CH_CENTER(channel) + value / 2;
        return formatNumberAsString(value, 0, 0, "", "");
      } else if (g_eeGeneral.ppmunit == PPM_PERCENT_PREC1) {
        value = calcRESXto1000(value);
        return formatNumberAsString(value, PREC1, 0, "", "%");
      } else {
        value = calcRESXto100(value);
        return formatNumberAsString(value, 0, 0, "", "%");
      }
    }
  }

 public:
  ChannelFailsafeEdit(Window* parent, uint8_t ch, int vmin, int vmax) :
      NumberEdit(parent, rect_t{0, 0, NUM_W, 0}, vmin, vmax, nullptr), channel(ch)
  {
    setGetValueHandler(
        [=]() { return calcRESXto1000(g_model.failsafeChannels[ch]); });
    setDisplayHandler([=](int) -> std::string { return getString(); });
    update();
  }

  void update() override
  {
    auto value = g_model.failsafeChannels[channel];
    if (value != FAILSAFE_CHANNEL_HOLD && value != FAILSAFE_CHANNEL_NOPULSE) {
      setSetValueHandler([=](int value) {
        g_model.failsafeChannels[channel] = calc1000toRESX(value);
        SET_DIRTY();
      });
      enable();
    } else {
      // disable setter to avoid overwritting the value limited by vmin/vmax
      setSetValueHandler(nullptr);
      disable();
    }
    SET_DIRTY();
    NumberEdit::update();
  }

  void setHold()
  {
    g_model.failsafeChannels[channel] = FAILSAFE_CHANNEL_HOLD;
    update();
  }

  void setNoPulse()
  {
    g_model.failsafeChannels[channel] = FAILSAFE_CHANNEL_NOPULSE;
    update();
  }

  void toggle()
  {
    auto value = &(g_model.failsafeChannels[channel]);
    if (*value == FAILSAFE_CHANNEL_HOLD) {
      *value = FAILSAFE_CHANNEL_NOPULSE;
    } else if (*value == FAILSAFE_CHANNEL_NOPULSE) {
      *value = 0;
    } else {
      *value = FAILSAFE_CHANNEL_HOLD;
    }
    update();
  }

  void copyChannel()
  {
    g_model.failsafeChannels[channel] = channelOutputs[channel];
    update();
  }

  static LAYOUT_VAL(NUM_W, 70, 70)
};

class ChannelFSCombo : public Window
{
  ChannelFailsafeEdit* edit = nullptr;

 public:
  ChannelFSCombo(Window* parent, uint8_t ch, int vmin, int vmax) :
      Window(parent, rect_t{})
  {
    padAll(PAD_TINY);

    setFlexLayout(LV_FLEX_FLOW_ROW, PAD_TINY, LV_SIZE_CONTENT);

    lv_obj_set_style_pad_column(lvobj, PAD_TINY, 0);
    lv_obj_set_style_flex_cross_place(lvobj, LV_FLEX_ALIGN_CENTER, 0);

    edit = new ChannelFailsafeEdit(this, ch, vmin, vmax);
    new TextButton(this, rect_t{}, LV_SYMBOL_SETTINGS, [=]() {
      edit->toggle();
      return 0;
    });

    new TextButton(this, rect_t{}, LV_SYMBOL_COPY, [=]() {
      edit->copyChannel();
      return 0;
    });
  }

  void update() { edit->update(); }
};

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(5),
                                          LV_GRID_FR(3), LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static void set_failsafe(lv_event_t* e)
{
  auto combo = (ChannelFSCombo*)lv_event_get_user_data(e);
  if (combo) combo->update();
}

FailSafePage::FailSafePage(uint8_t moduleIdx) : Page(ICON_STATS_ANALOGS)
{
  header->setTitle(STR_FAILSAFESET);

  body->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid(line_col_dsc, line_row_dsc, PAD_ZERO);

  auto btn = new TextButton(body, rect_t{0, 0, LV_PCT(100), 0}, STR_CHANNELS2FAILSAFE);

  btn->setPressHandler([=]() {
    setCustomFailsafe(moduleIdx);
    AUDIO_WARNING1();
    SET_DIRTY();
    return 0;
  });

  ModuleData* md = &g_model.moduleData[moduleIdx];
  auto start_ch = md->channelsStart;
  auto end_ch = md->channelsStart + maxModuleChannels(moduleIdx);

  const int lim = calcRESXto1000(
      g_model.extendedLimits ? 1024 * LIMIT_EXT_PERCENT / 100 : 1024);

  for (int ch = start_ch; ch < end_ch; ch++) {
    // Channel name
    auto line = body->newLine(grid);
    if (ch == start_ch) line->padTop(PAD_TINY);
    const char* ch_label = getSourceString(MIXSRC_FIRST_CH + ch);
    new StaticText(line, rect_t{}, ch_label);

    // Channel value
    auto combo = new ChannelFSCombo(line, ch, -lim, lim);
    lv_obj_add_event_cb(btn->getLvObj(), set_failsafe, LV_EVENT_CLICKED, combo);

    // Channel bargraph
    auto bar = new ChannelFailsafeBargraph(
        line, rect_t{0, 0, FS_BARGRAPH_WIDTH, EdgeTxStyles::UI_ELEMENT_HEIGHT}, ch);
    lv_obj_set_style_grid_cell_x_align(bar->getLvObj(), LV_GRID_ALIGN_END, 0);
  }
}
