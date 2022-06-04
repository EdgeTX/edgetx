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

#include "output_edit.h"
#include "channel_bar.h"
#include "gvar_numberedit.h"

#include "opentx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#if (LCD_W > LCD_H)
  #define OUTPUT_EDIT_STATUS_BAR_WIDTH 250
  #define OUTPUT_EDIT_STATUS_BAR_MARGIN 3
  #define OUTPUT_EDIT_RIGHT_MARGIN 0
#else
  #define OUTPUT_EDIT_STATUS_BAR_WIDTH 180
  #define OUTPUT_EDIT_STATUS_BAR_MARGIN 0
  #define OUTPUT_EDIT_RIGHT_MARGIN 3
#endif

class OutputEditStatusBar : public Window
{
 public:
  OutputEditStatusBar(Window *parent, const rect_t &rect, int8_t channel) :
      Window(parent, rect), _channel(channel)
  {
    channelBar = new ComboChannelBar(this, {OUTPUT_EDIT_STATUS_BAR_MARGIN, 0, rect.w - (OUTPUT_EDIT_STATUS_BAR_MARGIN * 2), rect.h}, channel);
    channelBar->setLeftMargin(0);
    channelBar->setTextColor(COLOR_THEME_PRIMARY2);
    channelBar->setOutputChannelBarLimitColor(COLOR_THEME_EDIT);
  }

  void paint(BitmapBuffer *dc) override
  {
    // dc->clear(COLOR_THEME_SECONDARY2);
  }

 protected:
  ComboChannelBar *channelBar;
  int8_t _channel;
};

OutputEditWindow::OutputEditWindow(uint8_t channel) :
    Page(ICON_MODEL_OUTPUTS), channel(channel)
{
  chanZero = calcRESXto100(channelOutputs[channel]);
  buildBody(&body);
  buildHeader(&header);
}

void OutputEditWindow::checkEvents()
{
  int newValue = channelOutputs[channel];
  if (value != newValue) {
    value = newValue;

    int chanVal = calcRESXto100(channelOutputs[channel]);
    minText->setBackgroundColor(chanVal < chanZero - 1 ? COLOR_THEME_ACTIVE
                                                       : 0U);
    minText->invalidate();
    maxText->setBackgroundColor(chanVal > chanZero + 1 ? COLOR_THEME_ACTIVE
                                                       : 0U);
    maxText->invalidate();
  }
  Window::checkEvents();
}

void OutputEditWindow::buildHeader(Window *window)
{
  new StaticText(window,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                  PAGE_LINE_HEIGHT},
                 STR_MENULIMITS, 0, COLOR_THEME_PRIMARY2);
  new StaticText(window,
                 {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                  LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                 getSourceString(MIXSRC_CH1 + channel), 0,
                 COLOR_THEME_PRIMARY2);

  statusBar = new OutputEditStatusBar(
      window,
      {window->getRect().w - OUTPUT_EDIT_STATUS_BAR_WIDTH -
           OUTPUT_EDIT_RIGHT_MARGIN,
       0, OUTPUT_EDIT_STATUS_BAR_WIDTH, MENU_HEADER_HEIGHT + 3},
      channel);
}

void OutputEditWindow::buildBody(FormWindow *window)
{
  FormGridLayout grid;
  grid.spacer(8);

  int limit = (g_model.extendedLimits ? LIMIT_EXT_MAX : LIMIT_STD_MAX);

  LimitData *output = limitAddress(channel);

  // Name
  new StaticText(window, grid.getLabelSlot(), STR_NAME, 0,
                 COLOR_THEME_PRIMARY1);
  new ModelTextEdit(window, grid.getFieldSlot(), output->name,
                    sizeof(output->name));
  grid.nextLine();

  // Offset
  new StaticText(window, grid.getLabelSlot(), TR_LIMITS_HEADERS_SUBTRIM, 0,
                 COLOR_THEME_PRIMARY1);
  new GVarNumberEdit(window, grid.getFieldSlot(), -LIMIT_STD_MAX,
                     +LIMIT_STD_MAX, GET_SET_DEFAULT(output->offset), 0, PREC1);
  grid.nextLine();

  // TRACE("ch=%d  cV=%d  zero=%d", channel, chanVal, chanZero);

  // Min
  minText = new StaticText(window, grid.getLabelSlot(), TR_MIN, 0,
                           COLOR_THEME_PRIMARY1);
  new GVarNumberEdit(window, grid.getFieldSlot(), -limit, 0,
                     GET_SET_DEFAULT(output->min), 0, PREC1, -LIMIT_STD_MAX);
  grid.nextLine();

  // Max
  maxText = new StaticText(window, grid.getLabelSlot(), TR_MAX, 0,
                           COLOR_THEME_PRIMARY1);
  new GVarNumberEdit(window, grid.getFieldSlot(), 0, +limit,
                     GET_SET_DEFAULT(output->max), 0, PREC1, +LIMIT_STD_MAX);
  grid.nextLine();

  // Direction
  new StaticText(window, grid.getLabelSlot(), STR_INVERTED, 0,
                 COLOR_THEME_PRIMARY1);
  new CheckBox(window, grid.getFieldSlot(), GET_DEFAULT(output->revert),
               [output, this](uint8_t newValue) {
                 if (newValue != output->revert) chanZero = -chanZero;
                 output->revert = newValue;
                 SET_DIRTY();
               });
  grid.nextLine();

  // Curve
  new StaticText(window, grid.getLabelSlot(), TR_CURVE, 0,
                 COLOR_THEME_PRIMARY1);
  auto edit = new NumberEdit(window, grid.getFieldSlot(), -MAX_CURVES,
                             +MAX_CURVES, GET_SET_DEFAULT(output->curve));
  edit->setDisplayHandler(
      [](int32_t value) { return std::string(getCurveString(value)); });
  grid.nextLine();

  // PPM center
  new StaticText(window, grid.getLabelSlot(), TR_LIMITS_HEADERS_PPMCENTER, 0,
                 COLOR_THEME_PRIMARY1);
  new NumberEdit(window, grid.getFieldSlot(), PPM_CENTER - PPM_CENTER_MAX,
                 PPM_CENTER + PPM_CENTER_MAX,
                 GET_VALUE(output->ppmCenter + PPM_CENTER),
                 SET_VALUE(output->ppmCenter, newValue - PPM_CENTER));
  grid.nextLine();

  // Subtrims mode
  new StaticText(window, grid.getLabelSlot(), TR_LIMITS_HEADERS_SUBTRIMMODE, 0,
                 COLOR_THEME_PRIMARY1);
  new Choice(window, grid.getFieldSlot(), STR_SUBTRIMMODES, 0, 1,
             GET_SET_DEFAULT(output->symetrical));
  grid.nextLine();
}
