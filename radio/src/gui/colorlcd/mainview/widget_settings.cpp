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
#include "libopenui.h"
#include "edgetx.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "view_main.h"
#include "filechoice.h"

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
  auto opt = widget->getOptions();
  while (opt && opt->name != nullptr) {
    auto line = form->newLine(grid);

    new StaticText(line, rect_t{},
                   opt->displayName ? opt->displayName : opt->name);

    auto optVal = widget->getOptionValue(optIdx);

    switch (opt->type) {
      case ZoneOption::Integer:
        (new NumberEdit(
             line, rect_t{0, 0, 96, 0}, opt->min.signedValue,
             opt->max.signedValue,
             [=]() -> int {
               return optVal->signedValue;
             },
             [=](int32_t newValue) {
               optVal->signedValue = newValue;
               SET_DIRTY();
             }))
            ->setDefault(opt->deflt.signedValue);
        break;

      case ZoneOption::Source:
        new SourceChoice(
            line, rect_t{}, 0, MIXSRC_LAST_TELEM,
            [=]() -> int16_t {
              return (int16_t)optVal->unsignedValue;
            },
            [=](int16_t newValue) {
              optVal->unsignedValue =
                  (uint32_t)newValue;
              SET_DIRTY();
            });
        break;

      case ZoneOption::Bool:
        new ToggleSwitch(
            line, rect_t{},
            [=]() -> uint8_t {
              return (uint8_t)optVal->boolValue;
            },
            [=](int8_t newValue) {
              optVal->boolValue = newValue;
              SET_DIRTY();
            });
        break;

      case ZoneOption::String:
        new ModelTextEdit(line, rect_t{0, 0, 96, 0},
                          optVal->stringValue,
                          sizeof(optVal->stringValue));
        break;

      case ZoneOption::TextSize:
        new Choice(
            line, rect_t{}, STR_FONT_SIZES, 0, FONTS_COUNT - 1,
            [=]() -> int {  // getValue
              return (int)optVal->unsignedValue;
            },
            [=](int newValue) {  // setValue
              optVal->unsignedValue =
                  (uint32_t)newValue;
              SET_DIRTY();
            });
        break;

      case ZoneOption::Align:
        new Choice(
            line, rect_t{}, STR_ALIGN_OPTS, 0, ALIGN_COUNT - 1,
            [=]() -> int {  // getValue
              return (int)optVal->unsignedValue;
            },
            [=](int newValue) {  // setValue
              optVal->unsignedValue =
                  (uint32_t)newValue;
              SET_DIRTY();
            });
        break;

      case ZoneOption::Timer:  // Unsigned
      {
        auto tmChoice = new Choice(
            line, rect_t{}, 0, TIMERS - 1,
            [=]() -> int {  // getValue
              return (int)optVal->unsignedValue;
            },
            [=](int newValue) {  // setValue
              optVal->unsignedValue =
                  (uint32_t)newValue;
              SET_DIRTY();
            });

        tmChoice->setTextHandler([](int value) {
          return std::string(STR_TIMER) + std::to_string(value + 1);
        });
      } break;

      case ZoneOption::Switch:
        new SwitchChoice(
            line, rect_t{},
            opt->min.signedValue,  // min
            opt->max.signedValue,  // max
            [=]() -> int16_t {       // getValue
              return optVal->signedValue;
            },
            [=](int16_t newValue) {  // setValue
              optVal->signedValue = newValue;
              SET_DIRTY();
            });
        break;

      case ZoneOption::Color:
        new ColorPicker(
            line, rect_t{},
            [=]() -> uint32_t {  // getValue
              return optVal->unsignedValue;
            },
            [=](uint32_t newValue) {  // setValue
              optVal->unsignedValue = newValue;
              SET_DIRTY();
            });
        break;

      case ZoneOption::Slider:
        new Slider(
            line, SLIDER_W, opt->min.signedValue, opt->max.signedValue,
            [=]() { return optVal->unsignedValue; },
            [=](uint32_t newValue) {
              optVal->unsignedValue = newValue;
              SET_DIRTY();
            });
        break;

      case ZoneOption::Choice:
        new Choice(line, rect_t{}, opt->choiceValues, 0, opt->choiceValues.size() - 1,
            [=]() { return optVal->unsignedValue - 1; },
            [=](uint32_t newValue) {
              optVal->unsignedValue = newValue + 1;
              SET_DIRTY();
            });
        break;

      case ZoneOption::File:
        new FileChoice(line, rect_t{}, opt->fileSelectPath, nullptr, LEN_ZONE_OPTION_STRING,
                        [=]() {
                          char s[LEN_ZONE_OPTION_STRING + 1];
                          strncpy(s, optVal->stringValue, LEN_ZONE_OPTION_STRING);
                          s[LEN_ZONE_OPTION_STRING] = 0;
                          return std::string(s);
                        },
                        [=](std::string s) {
                          strncpy(optVal->stringValue, s.c_str(), LEN_ZONE_OPTION_STRING);
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
