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
#include "curveedit.h"
#include "curve_param.h"
#include "gvar_numberedit.h"
#include "edgetx.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define ETX_STATE_MINMAX_HIGHLIGHT LV_STATE_USER_1

class OutputEditStatusBar : public Window
{
 public:
  OutputEditStatusBar(Window *parent, const rect_t &rect, int8_t channel) :
      Window(parent, rect), _channel(channel)
  {
    channelBar = new ComboChannelBar(
        this,
        {OUTPUT_EDIT_STATUS_BAR_MARGIN, 0,
         rect.w - (OUTPUT_EDIT_STATUS_BAR_MARGIN * 2), rect.h},
        channel, true);
  }

  static LAYOUT_VAL(OUTPUT_EDIT_STATUS_BAR_MARGIN, 3, 0)

 protected:
  ComboChannelBar *channelBar;
  int8_t _channel;
};

OutputEditWindow::OutputEditWindow(uint8_t channel) :
    Page(ICON_MODEL_OUTPUTS), channel(channel)
{
  std::string title2(getSourceString(MIXSRC_FIRST_CH + channel));
  header->setTitle(STR_MENULIMITS);
  header->setTitle2(title2);

  buildHeader(header);
  buildBody(body);
}

void OutputEditWindow::checkEvents()
{
  int newValue = channelOutputs[channel];
  if (value != newValue) {
    value = newValue;

    int chanVal = calcRESXto100(ex_chans[channel]);

    if (chanVal < -DEADBAND) {
      lv_obj_add_state(minText->getLvObj(), ETX_STATE_MINMAX_HIGHLIGHT);
      lv_obj_add_state(minEdit->getLvObj(), ETX_STATE_MINMAX_HIGHLIGHT);
    } else {
      lv_obj_clear_state(minText->getLvObj(), ETX_STATE_MINMAX_HIGHLIGHT);
      lv_obj_clear_state(minEdit->getLvObj(), ETX_STATE_MINMAX_HIGHLIGHT);
    }

    if (chanVal > DEADBAND) {
      lv_obj_add_state(maxText->getLvObj(), ETX_STATE_MINMAX_HIGHLIGHT);
      lv_obj_add_state(maxEdit->getLvObj(), ETX_STATE_MINMAX_HIGHLIGHT);
    } else {
      lv_obj_clear_state(maxText->getLvObj(), ETX_STATE_MINMAX_HIGHLIGHT);
      lv_obj_clear_state(maxEdit->getLvObj(), ETX_STATE_MINMAX_HIGHLIGHT);
    }
  }

  Window::checkEvents();
}

void OutputEditWindow::buildHeader(Window *window)
{
  statusBar = new OutputEditStatusBar(
      window,
      {window->getRect().w - OUTPUT_EDIT_STATUS_BAR_WIDTH -
           OUTPUT_EDIT_RIGHT_MARGIN,
       0, OUTPUT_EDIT_STATUS_BAR_WIDTH, EdgeTxStyles::MENU_HEADER_HEIGHT},
      channel);
}

#if !PORTRAIT_LCD
static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};
#endif

void OutputEditWindow::buildBody(Window *form)
{
  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
  form->setFlexLayout();

  int limit = (g_model.extendedLimits ? LIMIT_EXT_MAX : LIMIT_STD_MAX);
  LimitData *output = limitAddress(channel);

  // Name
  auto line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_NAME);
  new ModelTextEdit(line, rect_t{}, output->name, sizeof(output->name));

  // Offset
  new StaticText(line, rect_t{}, TR_LIMITS_HEADERS_SUBTRIM);
  auto off = new GVarNumberEdit(line, -LIMIT_STD_MAX, +LIMIT_STD_MAX,
                                GET_SET_DEFAULT(output->offset), PREC1);
  off->setFastStep(20);
  off->setAccelFactor(8);

  // Min
  line = form->newLine(grid);
  minText = new StaticText(line, rect_t{}, TR_MIN);
  etx_solid_bg(minText->getLvObj(), COLOR_THEME_ACTIVE_INDEX, ETX_STATE_MINMAX_HIGHLIGHT);
  etx_font(minText->getLvObj(), FONT_BOLD_INDEX, ETX_STATE_MINMAX_HIGHLIGHT);
  minEdit = new GVarNumberEdit(line, -limit, 0,
                               GET_SET_DEFAULT(output->min), PREC1,
                               -LIMIT_STD_MAX, -limit);
  etx_font(minEdit->getLvObj(), FONT_BOLD_INDEX, ETX_STATE_MINMAX_HIGHLIGHT);
  minEdit->setFastStep(20);
  minEdit->setAccelFactor(16);
  minEdit->setDisplayHandler([=](int value) {
    if (g_eeGeneral.ppmunit == PPM_US)
      value = value * 128 / 25;
    return formatNumberAsString(value, PREC1);
  });

  // Max
  maxText = new StaticText(line, rect_t{}, TR_MAX);
  etx_solid_bg(maxText->getLvObj(), COLOR_THEME_ACTIVE_INDEX, ETX_STATE_MINMAX_HIGHLIGHT);
  etx_font(maxText->getLvObj(), FONT_BOLD_INDEX, ETX_STATE_MINMAX_HIGHLIGHT);
  maxEdit = new GVarNumberEdit(line, 0, +limit,
                               GET_SET_DEFAULT(output->max), PREC1,
                               +LIMIT_STD_MAX, limit);
  etx_font(maxEdit->getLvObj(), FONT_BOLD_INDEX, ETX_STATE_MINMAX_HIGHLIGHT);
  maxEdit->setFastStep(20);
  maxEdit->setAccelFactor(16);
  maxEdit->setDisplayHandler([=](int value) {
    if (g_eeGeneral.ppmunit == PPM_US)
      value = value * 128 / 25;
    return formatNumberAsString(value, PREC1);
  });

  // Direction
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_INVERTED);
  new ToggleSwitch(line, rect_t{}, GET_DEFAULT(output->revert),
                   [output, this](uint8_t newValue) {
                     output->revert = newValue;
                     SET_DIRTY();
                   });

  // Curve
  new StaticText(line, rect_t{}, TR_CURVE);
  new CurveChoice(line, GET_SET_DEFAULT(output->curve), nullptr, channel + MIXSRC_FIRST_CH);

  // PPM center
  line = form->newLine(grid);
  auto label = new StaticText(line, rect_t{}, TR_LIMITS_HEADERS_PPMCENTER);
  lv_label_set_long_mode(label->getLvObj(), LV_LABEL_LONG_WRAP);
  lv_obj_set_style_grid_cell_x_align(label->getLvObj(), LV_GRID_ALIGN_STRETCH,
                                     0);

  auto center = new NumberEdit(
      line, rect_t{}, PPM_CENTER - PPM_CENTER_MAX, PPM_CENTER + PPM_CENTER_MAX,
      GET_VALUE(output->ppmCenter + PPM_CENTER),
      SET_VALUE(output->ppmCenter, newValue - PPM_CENTER));
  center->setFastStep(20);
  center->setAccelFactor(8);
  center->setDefault(PPM_CENTER);

  // Subtrims mode
  label = new StaticText(line, rect_t{}, TR_LIMITS_HEADERS_SUBTRIMMODE);
  lv_label_set_long_mode(label->getLvObj(), LV_LABEL_LONG_WRAP);
  lv_obj_set_style_grid_cell_x_align(label->getLvObj(), LV_GRID_ALIGN_STRETCH,
                                     0);

  new Choice(line, rect_t{}, STR_SUBTRIMMODES, 0, 1,
             GET_SET_DEFAULT(output->symetrical));
}
