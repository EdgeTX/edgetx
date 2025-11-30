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

#include "widget_settings.h"

#include "color_picker.h"
#include "edgetx.h"
#include "filechoice.h"
#include "numberedit.h"
#include "slider.h"
#include "sourcechoice.h"
#include "static.h"
#include "switchchoice.h"
#include "textedit.h"
#include "toggleswitch.h"
#include "view_main.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static const rect_t widgetSettingsDialogRect = {
    LCD_W / 10,              // x
    LCD_H / 5,               // y
    LCD_W - 2 * LCD_W / 10,  // width
    LCD_H - 2 * LCD_H / 5    // height
};

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

WidgetSettings::WidgetSettings(Widget* w) :
    BaseDialog(w->getFactory()->getDisplayName(), true), widget(w)
{
  FlexGridLayout grid(line_col_dsc, line_row_dsc, PAD_TINY);

  uint8_t optIdx = 0;

  widget->getFactory()->parseOptionDefaults();
  auto opt = widget->getOptionDefinitions();

  auto* widgetData = widget->getPersistentData();

  while (opt && opt->name != nullptr) {
    auto line = form->newLine(grid);

    new StaticText(line, rect_t{},
                   opt->displayName ? opt->displayName : opt->name);

    switch (opt->type) {
      case WidgetOption::Integer:
        (new NumberEdit(
             line, rect_t{}, opt->min.signedValue,
             opt->max.signedValue,
             [=]() -> int {
               return widgetData->getSignedValue(optIdx);
             },
             [=](int32_t newValue) {
               widgetData->setSignedValue(optIdx, newValue);
               SET_DIRTY();
             }))
            ->setDefault(opt->deflt.signedValue);
        break;

      case WidgetOption::Source:
        new SourceChoice(
            line, rect_t{}, 0, MIXSRC_LAST_TELEM,
            [=]() -> int16_t {
              return widgetData->getUnsignedValue(optIdx);
            },
            [=](int16_t newValue) {
              widgetData->setUnsignedValue(optIdx, (uint32_t)newValue);
              SET_DIRTY();
            });
        break;

      case WidgetOption::Bool:
        new ToggleSwitch(
            line, rect_t{},
            [=]() -> uint8_t {
              return (uint8_t)widgetData->getBoolValue(optIdx);
            },
            [=](int8_t newValue) {
              widgetData->setBoolValue(optIdx, newValue);
              SET_DIRTY();
            });
        break;

      case WidgetOption::String:
        new ModelStringEdit(line, rect_t{}, widgetData->getString(optIdx),
                            [=](const char* s) {
                              widgetData->setString(optIdx, s);
                            });
        break;

      case WidgetOption::TextSize:
        new Choice(
            line, rect_t{}, STR_FONT_SIZES, 0, FONTS_COUNT - 1,
            [=]() -> int {  // getValue
              return widgetData->getUnsignedValue(optIdx);
            },
            [=](int newValue) {  // setValue
              widgetData->setUnsignedValue(optIdx, (uint32_t)newValue);
              SET_DIRTY();
            });
        break;

      case WidgetOption::Align:
        new Choice(
            line, rect_t{}, STR_ALIGN_OPTS, 0, ALIGN_COUNT - 1,
            [=]() -> int {  // getValue
              return widgetData->getUnsignedValue(optIdx);
            },
            [=](int newValue) {  // setValue
              widgetData->setUnsignedValue(optIdx, (uint32_t)newValue);
              SET_DIRTY();
            });
        break;

      case WidgetOption::Timer:  // Unsigned
      {
        auto tmChoice = new Choice(
            line, rect_t{}, 0, TIMERS - 1,
            [=]() -> int {  // getValue
              return widgetData->getUnsignedValue(optIdx);
            },
            [=](int newValue) {  // setValue
              widgetData->setUnsignedValue(optIdx, (uint32_t)newValue);
              SET_DIRTY();
            });

        tmChoice->setTextHandler([](int value) {
          return std::string(STR_TIMER) + std::to_string(value + 1);
        });
      } break;

      case WidgetOption::Switch:
        new SwitchChoice(
            line, rect_t{},
            opt->min.signedValue,  // min
            opt->max.signedValue,  // max
            [=]() -> int16_t {       // getValue
              return widgetData->getSignedValue(optIdx);
            },
            [=](int16_t newValue) {  // setValue
              widgetData->setSignedValue(optIdx, newValue);
              SET_DIRTY();
            });
        break;

      case WidgetOption::Color:
        new ColorPicker(
            line, rect_t{},
            [=]() -> uint32_t {  // getValue
              return widgetData->getUnsignedValue(optIdx);
            },
            [=](uint32_t newValue) {  // setValue
              widgetData->setUnsignedValue(optIdx, newValue);
              SET_DIRTY();
            });
        break;

      case WidgetOption::Slider:
        new Slider(
            line, SLIDER_W, opt->min.signedValue, opt->max.signedValue,
            [=]() {
              return widgetData->getUnsignedValue(optIdx);
            },
            [=](uint32_t newValue) {
              widgetData->setUnsignedValue(optIdx, newValue);
              SET_DIRTY();
            });
        break;

      case WidgetOption::Choice:
        new Choice(line, rect_t{}, opt->choiceValues, 0, opt->choiceValues.size() - 1,
            [=]() {
              return widgetData->getUnsignedValue(optIdx) - 1;
            },
            [=](uint32_t newValue) {
              widgetData->setUnsignedValue(optIdx, newValue + 1);
              SET_DIRTY();
            });
        break;

      case WidgetOption::File:
        new FileChoice(line, rect_t{}, opt->fileSelectPath, "", FF_MAX_LFN,
                        [=]() {
                          return widgetData->getString(optIdx);
                        },
                        [=](std::string s) {
                          widgetData->setString(optIdx, s.c_str());
                          SET_DIRTY();
                        });
        break;
    }

    optIdx++;
    opt++;
  }
}

void WidgetSettings::onCancel()
{
  widget->update();
  deleteLater();
}
