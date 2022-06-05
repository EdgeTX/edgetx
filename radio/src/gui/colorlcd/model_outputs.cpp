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

#include "model_outputs.h"
#include "output_edit.h"
#include "input_mix_button.h"
#include "channel_bar.h"

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#if LCD_W > LCD_H
static const coord_t btn_cols[] = {
  90, 158, 208, 262, 306, 326
};
#else
static const coord_t btn_cols[] = {
  264, 66, 123, 180, 232, 264
};
#endif

class OutputLineButton : public InputMixButton
{
 public:
  OutputLineButton(Window* parent, uint8_t channel) :
    InputMixButton(parent, channel)
  {
#if LCD_H > LCD_W
    auto font_h = getFontHeight(FONT(STD));
    rect_t r{ 150, (font_h - 14) / 2, 100, 14 };
#else
    lv_obj_update_layout(lvobj);
    auto font_h = getFontHeight(FONT(STD));
    auto x = lv_obj_get_content_width(lvobj) - 92;
    auto y = (font_h - 14) / 2;

    rect_t r{ x, y, 92, 14 };
#endif
    auto bar = new OutputChannelBar(this, r, channel);
    bar->setDrawLimits(false);
  }

  void paint(BitmapBuffer *dc) override
  {
    LcdFlags textColor = COLOR_THEME_SECONDARY1;

    const LimitData* output = limitAddress(index);
    int chanZero = output->ppmCenter;

    coord_t border = lv_obj_get_style_border_width(lvobj, LV_PART_MAIN);
    coord_t pad_left = lv_obj_get_style_pad_left(lvobj, LV_PART_MAIN);
    coord_t left = pad_left + border;

    // first line ...
    coord_t x = left;
    coord_t y = border + lv_obj_get_style_pad_top(lvobj, LV_PART_MAIN);

    x = left;
    dc->drawText(x, y, getSourceString(MIXSRC_CH1 + index), FONT(BOLD) | textColor);

    if (output->revert) {
      x = left + btn_cols[0];
      dc->drawText(x, y, LV_SYMBOL_SHUFFLE, textColor);
    }

#if LCD_H > LCD_W      
    y += lv_obj_get_style_text_line_space(lvobj, LV_PART_MAIN)
      + getFontHeight(FONT(STD));
#endif
    
    // Min
    LcdFlags txtFlags = (value < chanZero - 5) ? FONT(BOLD) | textColor : textColor;
    x = left + btn_cols[1];
    drawValueOrGVar(dc, x, y, output->min, -GV_RANGELARGE, 0, RIGHT | PREC1 | txtFlags,
                    nullptr, -LIMITS_MIN_MAX_OFFSET);

    // Max
    txtFlags = (value > chanZero + 5) ? FONT(BOLD) | textColor : textColor;
    x = left + btn_cols[2];
    drawValueOrGVar(dc, x, y, output->max, 0, GV_RANGELARGE,
                    RIGHT | PREC1 | txtFlags, nullptr, +LIMITS_MIN_MAX_OFFSET);

    // Offset
    x = left + btn_cols[3];
    drawValueOrGVar(dc, x, y, output->offset, -LIMIT_STD_MAX,
                    +LIMIT_STD_MAX, RIGHT | PREC1 | textColor, nullptr);

    // PPM center
    x = left + btn_cols[4];
    dc->drawNumber(x, y, PPM_CENTER + output->ppmCenter, RIGHT | textColor);
    dc->drawText(x, y, output->symetrical ? "=" : STR_CHAR_DELTA, textColor);

    if (output->curve) {
      x = left + btn_cols[5];
      // dc->drawText(x, y, getCurveString(output->curve), RIGHT | textColor);
      dc->drawMask(x, y, mixerSetupCurveIcon, textColor);
    }
  }

 protected:
  int value = 0;

  bool isActive() const override { return false; }
  size_t getLines() const override
  {
#if LCD_H > LCD_W      
    return 2;
#else
    return 1;
#endif
  }

  void checkEvents() override
  {
    Window::checkEvents();
    int newValue = channelOutputs[index];
    if (value != newValue) {
      value = newValue;
      invalidate();
    }
  }
};

ModelOutputsPage::ModelOutputsPage() :
  PageTab(STR_MENULIMITS, ICON_MODEL_OUTPUTS)
{
}

void ModelOutputsPage::build(FormWindow *window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 8);
  lv_obj_set_style_pad_all(window->getLvObj(), 8, 0);

  auto btn = new TextButton(window, rect_t{}, STR_ADD_ALL_TRIMS_TO_SUBTRIMS,
      [=]() {
        moveTrimsToOffsets();
        // TODO: mass invalidate
        return 0;
      });
  auto btn_obj = btn->getLvObj();
  lv_obj_set_width(btn_obj, lv_pct(100));

  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {

    // Channel settings
    auto btn = new OutputLineButton(window, ch);

    LimitData* output = limitAddress(ch);
    btn->setPressHandler([=]() -> uint8_t {
      Menu *menu = new Menu(window);
      menu->addLine(STR_EDIT, [=]() {
        editOutput(btn, ch);
      });
      menu->addLine(STR_RESET, [=]() {
        output->min = 0;
        output->max = 0;
        output->offset = 0;
        output->ppmCenter = 0;
        output->revert = false;
        output->curve = 0;
        output->symetrical = 0;
        storageDirty(EE_MODEL);
        btn->invalidate();
      });
      menu->addLine(STR_COPY_STICKS_TO_OFS, [=]() {
        copySticksToOffset(ch);
        storageDirty(EE_MODEL);
        btn->invalidate();
      });
      menu->addLine(STR_COPY_TRIMS_TO_OFS, [=]() {
        copyTrimsToOffset(ch);
        storageDirty(EE_MODEL);
        btn->invalidate();
      });
      return 0;
    });
  }
}

void ModelOutputsPage::editOutput(OutputLineButton *btn, uint8_t channel)
{
  // auto btn_obj = btn->getLvObj();
  // Window *edit = 
  new OutputEditWindow(channel);
  // edit->setCloseHandler(
  //     [=]() { lv_event_send(btn_obj, LV_EVENT_VALUE_CHANGED, nullptr); });
}
