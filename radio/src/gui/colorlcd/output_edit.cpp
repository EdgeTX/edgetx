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
    channelBar->setLeftMargin(15);
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
  std::string title(STR_MENULIMITS);
  title += "\n";
  title += getSourceString(MIXSRC_FIRST_CH + channel);

  auto form = new FormWindow(&body, rect_t{});
  auto form_obj = form->getLvObj();
  lv_obj_set_style_pad_all(form_obj, lv_dpx(8), 0);
  buildBody(form);

  buildHeader(&header);
}

void OutputEditWindow::checkEvents()
{
  int newValue = channelOutputs[channel];
  if (value != newValue) {
    value = newValue;

    int chanVal = calcRESXto100(ex_chans[channel]);

    if(chanVal < -DEADBAND) {
      lv_obj_set_style_text_font(minEdit->getLvObj(), getFont(FONT(BOLD)), 0);   
      minText->setBackgroudOpacity(LV_OPA_COVER);
      minText->setFont(FONT(BOLD));

      lv_obj_set_style_text_font(maxEdit->getLvObj(), getFont(FONT(STD)), 0);  
      maxText->setBackgroudOpacity(LV_OPA_TRANSP);
      maxText->setFont(FONT(STD));    
    } else {
        if(chanVal > DEADBAND) {
          lv_obj_set_style_text_font(minEdit->getLvObj(), getFont(FONT(STD)), 0);  
          minText->setBackgroudOpacity(LV_OPA_TRANSP);
          minText->setFont(FONT(STD));

          lv_obj_set_style_text_font(maxEdit->getLvObj(), getFont(FONT(BOLD)), 0);  
          maxText->setBackgroudOpacity(LV_OPA_COVER);
          maxText->setFont(FONT(BOLD));
        } else {
            lv_obj_set_style_text_font(minEdit->getLvObj(), getFont(FONT(STD)), 0);
            minText->setBackgroudOpacity(LV_OPA_TRANSP);
            minText->setFont(FONT(STD));

            lv_obj_set_style_text_font(maxEdit->getLvObj(), getFont(FONT(STD)), 0); 
            maxText->setBackgroudOpacity(LV_OPA_TRANSP);
            maxText->setFont(FONT(STD)); 
        }
    }

    minText->invalidate();
    maxText->invalidate();
    minEdit->invalidate(); 
    maxEdit->invalidate();
  }

  Window::checkEvents();
}

void OutputEditWindow::buildHeader(Window *window)
{
  statusBar = new OutputEditStatusBar(
      window,
      {window->getRect().w - OUTPUT_EDIT_STATUS_BAR_WIDTH -
           OUTPUT_EDIT_RIGHT_MARGIN,
       0, OUTPUT_EDIT_STATUS_BAR_WIDTH, MENU_HEADER_HEIGHT + 3},
      channel);
}

#if LCD_W > LCD_H
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

void OutputEditWindow::buildBody(FormWindow* form)
{
  FlexGridLayout grid(col_dsc, row_dsc, 2);
  form->setFlexLayout();

  int limit = (g_model.extendedLimits ? LIMIT_EXT_MAX : LIMIT_STD_MAX);
  LimitData *output = limitAddress(channel);

  // Name
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
  new ModelTextEdit(line, rect_t{}, output->name, sizeof(output->name));

  // Offset
  new StaticText(line, rect_t{}, TR_LIMITS_HEADERS_SUBTRIM, 0,
                 COLOR_THEME_PRIMARY1);
  auto off = new GVarNumberEdit(line, rect_t{}, -LIMIT_STD_MAX, +LIMIT_STD_MAX,
                                 GET_SET_DEFAULT(output->offset), PREC1);
  off->setFastStep(20);
  off->setAccelFactor(8);

  // Min
  line = form->newLine(&grid);
  minText = new StaticText(line, rect_t{}, TR_MIN, 0, COLOR_THEME_PRIMARY1);
  minEdit = new GVarNumberEdit(line, rect_t{}, -limit, 0, GET_SET_DEFAULT(output->min),
                     PREC1, -LIMIT_STD_MAX);
  minText->setBackgroundColor(COLOR_THEME_ACTIVE);
  minEdit->setFastStep(20);
  minEdit->setAccelFactor(16);

  // Max
  maxText = new StaticText(line, rect_t{}, TR_MAX, 0, COLOR_THEME_PRIMARY1);
  maxEdit = new GVarNumberEdit(line, rect_t{}, 0, +limit, GET_SET_DEFAULT(output->max),
                     PREC1, +LIMIT_STD_MAX);
  maxText->setBackgroundColor(COLOR_THEME_ACTIVE);
  maxEdit->setFastStep(20);
  maxEdit->setAccelFactor(16);

  // Direction
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_INVERTED, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_DEFAULT(output->revert),
               [output, this](uint8_t newValue) {
                 output->revert = newValue;
                 SET_DIRTY();
               });

  // Curve
  new StaticText(line, rect_t{}, TR_CURVE, 0, COLOR_THEME_PRIMARY1);
  auto edit = new NumberEdit(line, rect_t{}, -MAX_CURVES, +MAX_CURVES,
                             GET_SET_DEFAULT(output->curve));
  edit->setDisplayHandler(
      [](int32_t value) { return std::string(getCurveString(value)); });

  // PPM center
  line = form->newLine(&grid);
  auto label = new StaticText(line, rect_t{}, TR_LIMITS_HEADERS_PPMCENTER, 0,
                              COLOR_THEME_PRIMARY1);
  lv_label_set_long_mode(label->getLvObj(), LV_LABEL_LONG_WRAP);
  lv_obj_set_style_grid_cell_x_align(label->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  auto center = new NumberEdit(line, rect_t{}, PPM_CENTER - PPM_CENTER_MAX,
                               PPM_CENTER + PPM_CENTER_MAX,
                               GET_VALUE(output->ppmCenter + PPM_CENTER),
                               SET_VALUE(output->ppmCenter, newValue - PPM_CENTER));
  center->setFastStep(20);
  center->setAccelFactor(8);
  center->setDefault(PPM_CENTER);

  // Subtrims mode
  label = new StaticText(line, rect_t{}, TR_LIMITS_HEADERS_SUBTRIMMODE, 0,
                         COLOR_THEME_PRIMARY1);
  lv_label_set_long_mode(label->getLvObj(), LV_LABEL_LONG_WRAP);
  lv_obj_set_style_grid_cell_x_align(label->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

  new Choice(line, rect_t{}, STR_SUBTRIMMODES, 0, 1,
             GET_SET_DEFAULT(output->symetrical));
}
