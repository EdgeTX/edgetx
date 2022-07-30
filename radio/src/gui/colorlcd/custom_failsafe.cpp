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
#include "opentx.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

class ChannelFailsafeBargraph : public Window
{
 public:
  ChannelFailsafeBargraph(Window* parent, const rect_t& rect, uint8_t moduleIdx,
                          uint8_t channel) :
      Window(parent, rect), moduleIdx(moduleIdx), channel(channel)
  {
  }

  void checkEvents() override
  {
    invalidate();
    Window::checkEvents();
  }

  void paint(BitmapBuffer* dc) override
  {
    int32_t failsafeValue = g_model.failsafeChannels[channel];
    int32_t channelValue = channelOutputs[channel];

    const int lim = g_model.extendedLimits ? 1024 * LIMIT_EXT_PERCENT / 100 : 1024;

    coord_t x = 0;
    dc->drawRect(x, 0, width(), height());

    if (failsafeValue == FAILSAFE_CHANNEL_HOLD ||
        failsafeValue == FAILSAFE_CHANNEL_NOPULSE)
      return;
    
    const coord_t lenChannel = limit(
        (uint8_t)1, uint8_t((abs(channelValue) * width() / 2 + lim / 2) / lim),
        uint8_t(width() / 2));

    const coord_t lenFailsafe = limit(
        (uint8_t)1, uint8_t((abs(failsafeValue) * width() / 2 + lim / 2) / lim),
        uint8_t(width() / 2));

    x += width() / 2;

    const coord_t xChannel = (channelValue > 0) ? x : x + 1 - lenChannel;
    const coord_t xFailsafe = (failsafeValue > 0) ? x : x + 1 - lenFailsafe;

    dc->drawSolidFilledRect(xChannel, +2, lenChannel, (height() / 2) - 3,
                            COLOR_THEME_SECONDARY1);

    dc->drawSolidFilledRect(xFailsafe, (height() / 2) + 1, lenFailsafe,
                            (height() / 2) - 3, COLOR_THEME_WARNING);
  }

 protected:
  uint8_t moduleIdx;
  uint8_t channel;
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
      value = calcRESXto1000(value);
      return formatNumberAsString(value, PREC1, 0, "", "%");
    }
  }
  
public:
  ChannelFailsafeEdit(Window* parent, uint8_t ch, int vmin, int vmax) :
    NumberEdit(parent, rect_t{}, vmin, vmax, nullptr), channel(ch)
  {
    setGetValueHandler([=]() { return calcRESXto1000(g_model.failsafeChannels[ch]); });
    setDisplayHandler([=](int) -> std::string { return getString(); });
    update();
  }

  void update() override
  {
    auto value = g_model.failsafeChannels[channel];
    if (value != FAILSAFE_CHANNEL_HOLD && value != FAILSAFE_CHANNEL_NOPULSE) {
      setSetValueHandler([=](int value) {
        g_model.failsafeChannels[channel] = calc1000toRESX(value);
      });
      lv_obj_clear_state(lvobj, LV_STATE_DISABLED);
    } else {
      // disable setter to avoid overwritting the value limited by vmin/vmax
      setSetValueHandler(nullptr);
      lv_obj_add_state(lvobj, LV_STATE_DISABLED);
    }
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
};

class ChannelFSCombo : public FormGroup
{
  ChannelFailsafeEdit* edit = nullptr;
  
public:
  ChannelFSCombo(Window* parent, uint8_t ch, int vmin, int vmax) :
    FormGroup(parent, rect_t{})
  {
    setFlexLayout(LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_column(lvobj, lv_dpx(4), 0);
    lv_obj_set_style_flex_cross_place(lvobj, LV_FLEX_ALIGN_CENTER, 0);

    edit = new ChannelFailsafeEdit(this, ch, vmin, vmax);
    auto btn = new TextButton(this, rect_t{}, LV_SYMBOL_SETTINGS, [=]() {
      edit->toggle();
      return 0;
    });
    btn->padTop(lv_dpx(4));
    btn->padBottom(lv_dpx(4));
  }

  void update() { edit->update(); }
};


static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2), LV_GRID_FR(3),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static void set_failsafe(lv_event_t* e)
{
  auto combo = (ChannelFSCombo*)lv_event_get_user_data(e);
  if (combo) combo->update();
}

FailSafePage::FailSafePage(uint8_t moduleIdx) :
    Page(ICON_STATS_ANALOGS)
{
  header.setTitle(STR_FAILSAFESET);

  auto form = new FormGroup(&body, rect_t{});
  form->setFlexLayout();
  form->padAll(lv_dpx(8));
  form->padRow(lv_dpx(8));

  FlexGridLayout grid(line_col_dsc, line_row_dsc, 0);

  auto btn = new TextButton(form, rect_t{}, STR_CHANNELS2FAILSAFE);
  lv_obj_set_width(btn->getLvObj(), lv_pct(100));
  
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
    auto line = form->newLine(&grid);
    const char* ch_label = getSourceString(MIXSRC_CH1 + ch);
    new StaticText(line, rect_t{}, ch_label, 0, COLOR_THEME_PRIMARY1);

    // Channel value
    auto combo = new ChannelFSCombo(line, ch, -lim, lim);
    lv_obj_add_event_cb(btn->getLvObj(), set_failsafe, LV_EVENT_CLICKED, combo);
    
    // Channel bargraph
    auto bar = new ChannelFailsafeBargraph(line, rect_t{}, moduleIdx, ch);
    bar->setWidth(LV_DPI_DEF);
    bar->setHeight(LV_DPI_DEF / 5);
  }
}
