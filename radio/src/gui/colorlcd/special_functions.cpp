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

#include "special_functions.h"
#include "opentx.h"
#include "libopenui.h"
#include "view_main.h"

#define SET_DIRTY()     storageDirty(functions == g_model.customFn ? EE_MODEL : EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

class SpecialFunctionEditPage : public Page
{
 public:
  SpecialFunctionEditPage(CustomFunctionData *functions, uint8_t index) :
      Page(functions == g_model.customFn ? ICON_MODEL_SPECIAL_FUNCTIONS
                                         : ICON_RADIO_GLOBAL_FUNCTIONS),
      functions(functions),
      index(index)
  {
    buildHeader(&header);
    buildBody(&body);
    lv_obj_set_style_max_height(body.getLvObj(), LCD_H - header.height(), 0);
    lv_obj_set_style_max_width(body.getLvObj(), LCD_W, 0);
  }

 protected:
  CustomFunctionData *functions;
  uint8_t index;
  FormGroup *specialFunctionOneWindow = nullptr;
  StaticText *headerSF = nullptr;
  bool active = false;

  bool isActive() const
  {
    return ((functions == g_model.customFn
                 ? modelFunctionsContext.activeSwitches
                 : globalFunctionsContext.activeSwitches) &
                    ((MASK_CFN_TYPE)1 << index)
                ? 1
                : 0);
  }

  void checkEvents() override
  {
    Page::checkEvents();
    if (active != isActive()) {
      if(isActive()) {
        lv_obj_add_state(headerSF->getLvObj(), LV_STATE_USER_1);
      } else {
        lv_obj_clear_state(headerSF->getLvObj(), LV_STATE_USER_1);
      }
      active = isActive();
      invalidate();
    }
  }

  void buildHeader(Window *window)
  {
    header.setTitle(functions == g_model.customFn ? STR_MENUCUSTOMFUNC : STR_MENUSPECIALFUNCS);
    headerSF = new StaticText(
        window,
        {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
         LCD_W - PAGE_TITLE_LEFT, 20},
        (functions == g_model.customFn ? "SF" : "GF") + std::to_string(index+1),
        0, COLOR_THEME_PRIMARY2);

    lv_obj_set_style_text_color(headerSF->getLvObj(), makeLvColor(COLOR_THEME_ACTIVE), LV_STATE_USER_1);
    lv_obj_set_style_text_font(headerSF->getLvObj(), getFont(FONT(BOLD)), LV_STATE_USER_1);
  }

  void addSourceChoice(FormGroup::Line* line, const char* title, CustomFunctionData* cfn, int16_t vmax)
  {
    new StaticText(line, rect_t{}, title, 0, COLOR_THEME_PRIMARY1);
    new SourceChoice(line, rect_t{}, 0, vmax, GET_SET_DEFAULT(CFN_PARAM(cfn)));
  }

  NumberEdit* addNumberEdit(FormGroup::Line* line, const char* title, CustomFunctionData* cfn, int16_t vmin, int16_t vmax)
  {
    new StaticText(line, rect_t{}, title, 0, COLOR_THEME_PRIMARY1);
    return new NumberEdit(line, rect_t{}, vmin, vmax, GET_SET_DEFAULT(CFN_PARAM(cfn)));
  }

  void updateSpecialFunctionOneWindow()
  {
    specialFunctionOneWindow->clear();
    specialFunctionOneWindow->setFlexLayout();
    FlexGridLayout grid(col_dsc, row_dsc, 2);
    auto line = specialFunctionOneWindow->newLine(&grid);

    CustomFunctionData *cfn = &functions[index];
    uint8_t func = CFN_FUNC(cfn);

    // Func param
    switch (func) {
      case FUNC_OVERRIDE_CHANNEL: {
        new StaticText(line, rect_t{}, STR_CH, 0, COLOR_THEME_PRIMARY1);
        new NumberEdit(line, rect_t{}, 1,
                       MAX_OUTPUT_CHANNELS,
                       GET_SET_VALUE_WITH_OFFSET(CFN_CH_INDEX(cfn), 1));
        line = specialFunctionOneWindow->newLine(&grid);

        int limit = (g_model.extendedLimits ? LIMIT_EXT_PERCENT : LIMIT_STD_PERCENT);
        addNumberEdit(line, STR_VALUE, cfn, -limit, limit);
        break;
      }

      case FUNC_TRAINER: {
        new StaticText(line, rect_t{}, STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        auto choice =
            new Choice(line, rect_t{}, 0,
                       NUM_STICKS + 1, GET_SET_DEFAULT(CFN_CH_INDEX(cfn)));
        choice->setTextHandler([=](int32_t value) {
          if (value == 0)
            return std::string(STR_STICKS);
          else if (value == NUM_STICKS + 1)
            return std::string(STR_CHANS);
          else
            return TEXT_AT_INDEX(STR_VSRCRAW, value);
          ;
        });
        break;
      }

      case FUNC_RESET:
        if (CFN_PARAM(cfn) < FUNC_RESET_PARAM_FIRST_TELEM) {
          new StaticText(line, rect_t{}, STR_RESET, 0, COLOR_THEME_PRIMARY1);
          auto choice = new Choice(
              line, rect_t{}, 0,
              FUNC_RESET_PARAM_FIRST_TELEM + lastUsedTelemetryIndex(),
              GET_SET_DEFAULT(CFN_PARAM(cfn)));
          choice->setAvailableHandler(isSourceAvailableInResetSpecialFunction);
          choice->setTextHandler([=](int32_t value) {
            if (value < FUNC_RESET_PARAM_FIRST_TELEM)
              return TEXT_AT_INDEX(STR_VFSWRESET, value);
            else
              return std::string(
                  g_model.telemetrySensors[value - FUNC_RESET_PARAM_FIRST_TELEM]
                      .label,
                  TELEM_LABEL_LEN);
          });
        }
        break;

      case FUNC_VOLUME:
        addSourceChoice(line, STR_VOLUME, cfn, MIXSRC_LAST_CH);
        break;

      case FUNC_BACKLIGHT:
        addSourceChoice(line, STR_VALUE, cfn, MIXSRC_LAST_CH);
        break;

      case FUNC_PLAY_SOUND:
        new StaticText(line, rect_t{}, STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new Choice(line, rect_t{},
                   STR_FUNCSOUNDS, 0,
                   AU_SPECIAL_SOUND_LAST - AU_SPECIAL_SOUND_FIRST - 1,
                   GET_SET_DEFAULT(CFN_PARAM(cfn)));
        break;

      case FUNC_PLAY_TRACK:
      case FUNC_BACKGND_MUSIC:
      case FUNC_PLAY_SCRIPT:
        new StaticText(line, rect_t{}, STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new FileChoice(
            line, rect_t{},
            func == FUNC_PLAY_SCRIPT
                ? SCRIPTS_FUNCS_PATH
                : std::string(SOUNDS_PATH, SOUNDS_PATH_LNG_OFS) +
                      std::string(currentLanguagePack->id, 2),
            func == FUNC_PLAY_SCRIPT ? SCRIPTS_EXT : SOUNDS_EXT,
            sizeof(cfn->play.name),
            [=]() { return std::string(cfn->play.name, ZLEN(cfn->play.name)); },
            [=](std::string newValue) {
              strncpy(cfn->play.name, newValue.c_str(), sizeof(cfn->play.name));
              SET_DIRTY();
              LUA_LOAD_MODEL_SCRIPTS();
            },
            true);  // strip extension
        break;

      case FUNC_SET_TIMER: {
        new StaticText(line, rect_t{}, STR_TIMER, 0, COLOR_THEME_PRIMARY1);
        auto timerchoice =
            new Choice(line, rect_t{}, 0,
                       TIMERS - 1, GET_SET_DEFAULT(CFN_TIMER_INDEX(cfn)));
        timerchoice->setTextHandler([](int32_t value) {
          return std::string(STR_TIMER) + std::to_string(value + 1);
        });
        line = specialFunctionOneWindow->newLine(&grid);

        new StaticText(line, rect_t{}, STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new TimeEdit(line, rect_t{}, 0,
                     9 * 60 * 60 - 1, GET_SET_DEFAULT(CFN_PARAM(cfn)));
        break;
      }

      case FUNC_SET_FAILSAFE:
        new StaticText(line, rect_t{}, STR_MODULE, 0, COLOR_THEME_PRIMARY1);
        new Choice(line, rect_t{},
                   "\004Int.Ext.", 0, NUM_MODULES - 1,
                   GET_SET_DEFAULT(CFN_PARAM(cfn)));
        break;

      case FUNC_PLAY_VALUE:
        addSourceChoice(line, STR_VOLUME, cfn, MIXSRC_LAST_TELEM);
        break;

      case FUNC_HAPTIC:
        addNumberEdit(line, STR_VALUE, cfn, 0, 3);
        break;

      case FUNC_LOGS: {
        auto edit = addNumberEdit(line, STR_VALUE, cfn, 0, 255);
        edit->setDisplayHandler(
            [=](int32_t value) {
              return formatNumberAsString(CFN_PARAM(cfn), PREC1, sizeof(CFN_PARAM(cfn)), nullptr, "s");
            });
        break;
      }

      case FUNC_SET_SCREEN:
        CFN_PARAM(cfn) = (int16_t)max(CFN_PARAM(cfn), (int16_t)1);
        CFN_PARAM(cfn) = (int16_t)min(CFN_PARAM(cfn), (int16_t)ViewMain::instance()->getMainViewsCount());
        addNumberEdit(line, STR_VALUE, cfn, 1, ViewMain::instance()->getMainViewsCount());
        break;
        
      case FUNC_ADJUST_GVAR: {
        new StaticText(line, rect_t{}, STR_GLOBALVAR, 0, COLOR_THEME_PRIMARY1);
        auto gvarchoice =
            new Choice(line, rect_t{}, 0,
                       MAX_GVARS - 1, GET_SET_DEFAULT(CFN_GVAR_INDEX(cfn)));
        gvarchoice->setTextHandler([](int32_t value) {
          return std::string(STR_GV) + std::to_string(value + 1);
        });
        line = specialFunctionOneWindow->newLine(&grid);

        new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);
        auto modechoice =
          new Choice(line, rect_t{},
                     FUNC_ADJUST_GVAR_CONSTANT, FUNC_ADJUST_GVAR_INCDEC,
                     GET_DEFAULT(CFN_GVAR_MODE(cfn)), nullptr);
        line = specialFunctionOneWindow->newLine(&grid);

        modechoice->setTextHandler([](int32_t value) {
          switch (value) {
            case FUNC_ADJUST_GVAR_CONSTANT:
              return std::string(STR_CONSTANT);
            case FUNC_ADJUST_GVAR_SOURCE:
              return std::string(STR_MIXSOURCE);
            case FUNC_ADJUST_GVAR_GVAR:
              return std::string(STR_GLOBALVAR);
            case FUNC_ADJUST_GVAR_INCDEC:
              return std::string(STR_INCDEC);
          }
          return std::string("---");
        });

        modechoice->setSetValueHandler([=](int32_t newValue) {
            CFN_GVAR_MODE(cfn) = newValue;
            CFN_PARAM(cfn) = 0;
            SET_DIRTY();
            updateSpecialFunctionOneWindow();
          });

        switch (CFN_GVAR_MODE(cfn)) {
          case FUNC_ADJUST_GVAR_CONSTANT: {
            int16_t val_min, val_max;
            getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min, val_max);
            addNumberEdit(line, STR_CONSTANT, cfn, val_min, val_max);
            break;
          }
          case FUNC_ADJUST_GVAR_SOURCE:
            addSourceChoice(line, STR_MIXSOURCE, cfn, MIXSRC_LAST_CH);
            break;
          case FUNC_ADJUST_GVAR_GVAR: {
            new StaticText(line, rect_t{}, STR_GLOBALVAR, 0, COLOR_THEME_PRIMARY1);
            auto gvarchoice =
                new Choice(line, rect_t{}, 0,
                           MAX_GVARS - 1, GET_SET_DEFAULT(CFN_PARAM(cfn)));
            gvarchoice->setTextHandler([](int32_t value) {
              return std::string(STR_GV) + std::to_string(value + 1);
            });
            gvarchoice->setAvailableHandler([=](int value) {
                return CFN_GVAR_INDEX(cfn) != value;
            });
            break;
          }
          case FUNC_ADJUST_GVAR_INCDEC: {
            int16_t val_min, val_max;
            getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min, val_max);
            getGVarIncDecRange(val_min, val_max);
            auto numedit = addNumberEdit(line, STR_INCDEC, cfn, val_min, val_max);
            numedit->setDisplayHandler(
                [](int value) {
                  return formatNumberAsString(abs(value), 0, 0, value >= 0 ? "+=" : "--", nullptr);
                });
            break;
          }
        }
      }
    }

    if (HAS_ENABLE_PARAM(func)) {
      line = specialFunctionOneWindow->newLine(&grid);
      new StaticText(line, rect_t{}, STR_ENABLE, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{},
                   GET_SET_DEFAULT(CFN_ACTIVE(cfn)));
    } else if (HAS_REPEAT_PARAM(func)) {  // !1x 1x 1s 2s 3s ...
      line = specialFunctionOneWindow->newLine(&grid);
      new StaticText(line, rect_t{}, STR_REPEAT,
                     0, COLOR_THEME_PRIMARY1);
      auto repeat = new NumberEdit(
          line, rect_t{}, -1,
          60 / CFN_PLAY_REPEAT_MUL, GET_DEFAULT((int8_t)CFN_PLAY_REPEAT(cfn)),
          SET_DEFAULT(CFN_PLAY_REPEAT(cfn)));
      repeat->setDisplayHandler(
          [](int32_t value) {
            if (value == 0)
              return std::string("1x");
            else if (value == (int8_t)CFN_PLAY_REPEAT_NOSTART)
              return std::string("!1x");
            else {
              return formatNumberAsString(value * CFN_PLAY_REPEAT_MUL, 0, 0, nullptr, "s");
            }
          });
    }
  }

  void buildBody(FormWindow *window)
  {
    window->setFlexLayout();
    FlexGridLayout grid(col_dsc, row_dsc, 2);
    lv_obj_set_style_pad_all(window->getLvObj(), lv_dpx(8), 0);

    CustomFunctionData *cfn = &functions[index];

    // Switch
    auto line = window->newLine(&grid);
    new StaticText(line, rect_t{}, STR_SWITCH, 0, COLOR_THEME_PRIMARY1);
    auto switchChoice =
        new SwitchChoice(line, rect_t{}, SWSRC_FIRST, SWSRC_LAST,
                         GET_SET_DEFAULT(CFN_SWITCH(cfn)));
    switchChoice->setAvailableHandler([=](int value) {
      return (functions == g_model.customFn
                  ? isSwitchAvailable(value, ModelCustomFunctionsContext)
                  : isSwitchAvailable(value, GeneralCustomFunctionsContext));
    });
    line = window->newLine(&grid);

    // Patch function in case not available
    if (!isAssignableFunctionAvailable(CFN_FUNC(cfn), functions)) {
      auto func = 0;
      while(!isAssignableFunctionAvailable(func, functions)
            && (func < FUNC_MAX - 1)) {
        func++;
      }
      if (func < FUNC_MAX - 1) {
        CFN_FUNC(cfn) = func;
      }
    }
    
    // Function
    new StaticText(line, rect_t{}, STR_FUNC, 0, COLOR_THEME_PRIMARY1);
    auto functionChoice =
        new Choice(line, rect_t{}, STR_VFSWFUNC,
                   0, FUNC_MAX - 1,
                   GET_DEFAULT(CFN_FUNC(cfn)));
    functionChoice->setSetValueHandler([=](int32_t newValue) {
      CFN_FUNC(cfn) = newValue;
      CFN_RESET(cfn);
      SET_DIRTY();
      updateSpecialFunctionOneWindow();
    });
    functionChoice->setAvailableHandler([=](int value) {
      return isAssignableFunctionAvailable(value, functions);
    });
    line = window->newLine(&grid);

    specialFunctionOneWindow = new FormGroup(window, rect_t{});
    updateSpecialFunctionOneWindow();
  }
};

static constexpr coord_t line1 = FIELD_PADDING_TOP;
static constexpr coord_t line2 = line1 + PAGE_LINE_HEIGHT;
static constexpr coord_t col0w = 56;
static constexpr coord_t col1 = 10 + col0w;
static constexpr coord_t col2 = (LCD_W - 100) / 3 + col1;
static constexpr coord_t col3 = ((LCD_W - 100) / 3) * 2 + col1 + 20;

static const char* _failsafe_module[] = {
  "Ext.", "Int.",
};

class SpecialFunctionButton : public Button
{
 public:
  SpecialFunctionButton(Window *parent, const rect_t &rect,
                        CustomFunctionData *functions, uint8_t index) :
      Button(parent, rect), functions(functions), index(index)
  {
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SpecialFunctionButton"; }
#endif

  bool isActive() const
  {
    return ((functions == g_model.customFn
                 ? modelFunctionsContext.activeSwitches
                 : globalFunctionsContext.activeSwitches) &
                    ((MASK_CFN_TYPE)1 << index)
                ? 1
                : 0);
  }

  void checkEvents() override
  {
    Button::checkEvents();
    if (active != isActive()) {
      invalidate();
      active = !active;
    }
  }

  void paintSpecialFunctionLine(BitmapBuffer *dc)
  {
    const CustomFunctionData *cfn = &functions[index];
    if (functions[index].func == FUNC_OVERRIDE_CHANNEL &&
        functions != g_model.customFn) {
      functions[index].func = FUNC_OVERRIDE_CHANNEL + 1;
    }
    uint8_t func = CFN_FUNC(cfn);

    drawSwitch(dc, col1, line1, CFN_SWITCH(cfn), COLOR_THEME_SECONDARY1);
    if (cfn->isEmpty()) return;

    dc->drawTextAtIndex(col2, line1, STR_VFSWFUNC, func, COLOR_THEME_SECONDARY1);
    int16_t val_min = 0;
    int16_t val_max = 255;

    switch (func) {
      case FUNC_OVERRIDE_CHANNEL:
        drawChn(dc, col1, line2, CFN_CH_INDEX(cfn) + 1, COLOR_THEME_SECONDARY1);
        getMixSrcRange(MIXSRC_FIRST_CH, val_min, val_max);
        dc->drawNumber(col2, line2, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        break;

      case FUNC_TRAINER: {
        std::string text;
        int16_t value = CFN_CH_INDEX(cfn);
        if (value == 0)
          text = std::string(STR_STICKS);
        else if (value == NUM_STICKS + 1)
          text = std::string(STR_CHANS);
        else
          text = TEXT_AT_INDEX(STR_VSRCRAW, value);
        dc->drawText(col1, line2, text.c_str(), COLOR_THEME_SECONDARY1);
        break;
      }
      case FUNC_RESET:
        if (CFN_PARAM(cfn) < FUNC_RESET_PARAM_FIRST_TELEM) {
          dc->drawTextAtIndex(col1, line2, STR_VFSWRESET, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        } else {
          TelemetrySensor *sensor =
              &g_model.telemetrySensors[CFN_PARAM(cfn) -
                                        FUNC_RESET_PARAM_FIRST_TELEM];
          dc->drawSizedText(col1, line2, sensor->label, TELEM_LABEL_LEN, COLOR_THEME_SECONDARY1);
        }
        break;

      case FUNC_VOLUME:
      case FUNC_BACKLIGHT:
      case FUNC_PLAY_VALUE:
        drawSource(dc, col1, line2, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        break;

      case FUNC_PLAY_SOUND:
        dc->drawTextAtIndex(col1, line2, STR_FUNCSOUNDS, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        break;

      case FUNC_PLAY_TRACK:
      case FUNC_BACKGND_MUSIC:
      case FUNC_PLAY_SCRIPT:
        if (ZEXIST(cfn->play.name))
          dc->drawSizedText(col1, line2, cfn->play.name, sizeof(cfn->play.name), COLOR_THEME_SECONDARY1);
        else
          dc->drawTextAtIndex(col1, line2, STR_VCSWFUNC, 0, COLOR_THEME_SECONDARY1);
        break;

      case FUNC_SET_TIMER:
        drawStringWithIndex(dc, col1, line2, STR_TIMER, CFN_TIMER_INDEX(cfn) + 1, COLOR_THEME_SECONDARY1);
        break;

      case FUNC_SET_FAILSAFE:
        dc->drawTextAtIndex(col1, line2, _failsafe_module, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        break;

      case FUNC_HAPTIC:
        dc->drawNumber(col1, line2, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        break;

      case FUNC_LOGS:
        dc->drawNumber(col3, line1, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1 | PREC1, sizeof(CFN_PARAM(cfn)), nullptr, "s");
        break;

      case FUNC_SET_SCREEN:
        dc->drawNumber(col2, line2, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        break;

      case FUNC_ADJUST_GVAR:
        switch(CFN_GVAR_MODE(cfn)) {
          case FUNC_ADJUST_GVAR_CONSTANT:
            dc->drawNumber(col1, line2, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
            break;
          case FUNC_ADJUST_GVAR_SOURCE:
            drawSource(dc, col1, line2, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
            break;
          case FUNC_ADJUST_GVAR_GVAR:
            drawSource(dc, col1, line2, CFN_PARAM(cfn) + MIXSRC_FIRST_GVAR, COLOR_THEME_SECONDARY1);
            break;
          case FUNC_ADJUST_GVAR_INCDEC: {
            int16_t value = CFN_PARAM(cfn);
            std::string text(value >= 0 ? "+= " : "-= ");
            text += std::to_string(abs(value));
            dc->drawText(col1, line2, text.c_str(), COLOR_THEME_SECONDARY1);
            break;
          }
        }
    }
    if (HAS_ENABLE_PARAM(func)) {
      theme->drawCheckBox(dc, CFN_ACTIVE(cfn), col3, line2);
    } else if (HAS_REPEAT_PARAM(func)) {
      if (CFN_PLAY_REPEAT(cfn) == 0) {
        dc->drawText(col3, line2, "1x", COLOR_THEME_SECONDARY1);
      } else if (CFN_PLAY_REPEAT(cfn) == CFN_PLAY_REPEAT_NOSTART) {
        dc->drawText(col3, line2, "!1x", COLOR_THEME_SECONDARY1);
      } else {
        dc->drawNumber(col3 + 12, line2, CFN_PLAY_REPEAT(cfn) * CFN_PLAY_REPEAT_MUL, COLOR_THEME_SECONDARY1 | RIGHT, 0, nullptr, "s");
      }
    }
  }

  void paint(BitmapBuffer *dc) override
  {
    char s[] = "SFxx";
    if (functions == g_eeGeneral.customFn) s[0] = 'G';
    strAppendUnsigned(&s[2], index+1);

    if (active) {
      dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR_THEME_ACTIVE);
      dc->drawSolidFilledRect(0, 0, col0w, rect.h, COLOR_THEME_FOCUS);
      dc->drawText(8, 12, s, COLOR_THEME_PRIMARY2);
    } else {
      dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR_THEME_PRIMARY2);
      dc->drawSolidFilledRect(0, 0, col0w, rect.h, COLOR_THEME_SECONDARY2);
      dc->drawText(8, 12, s, COLOR_THEME_PRIMARY1);
    }

    paintSpecialFunctionLine(dc);

    // The bounding rect
    if (hasFocus())
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_FOCUS);
    else
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR_THEME_SECONDARY2);
  }

 protected:
  CustomFunctionData *functions;
  uint8_t index;
  bool active = false;
};

#define SF_BUTTON_H 45

SpecialFunctionsPage::SpecialFunctionsPage(CustomFunctionData *functions) :
    PageTab(functions == g_model.customFn ? STR_MENUCUSTOMFUNC
                                          : STR_MENUSPECIALFUNCS,
            functions == g_model.customFn ? ICON_MODEL_SPECIAL_FUNCTIONS
                                          : ICON_RADIO_GLOBAL_FUNCTIONS),
    functions(functions)
{
}

void SpecialFunctionsPage::rebuild(FormWindow *window)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());  

  isRebuilding = true;
  window->clear();
  isRebuilding = false;
  build(window);

  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void SpecialFunctionsPage::editSpecialFunction(FormWindow *window,
                                               uint8_t index)
{
  focusIndex = index;
  auto editPage = new SpecialFunctionEditPage(functions, index);
  editPage->setCloseHandler([=]() { rebuild(window); });
}

void SpecialFunctionsPage::build(FormWindow *window)
{
#if LCD_W > LCD_H
  #define PER_ROW 6
  static const lv_coord_t l_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};
#else
  #define PER_ROW 4
  static const lv_coord_t l_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};
#endif

  window->padAll(4);

  auto form = new FormWindow(window, rect_t{});
  form->setFlexLayout();
  form->padAll(0);

  FlexGridLayout grid(l_col_dsc, row_dsc, 2);

  FormWindow::Line* line;
  firstActiveButton = nullptr;
  firstInactiveButton = nullptr;
  bool hasFocusButton = false;
  Button* button;

  char s[] = "SFxx";
  if (functions == g_eeGeneral.customFn) s[0] = 'G';

  uint8_t scol = 0;

  for (uint8_t i = 0; i < MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData *cfn = &functions[i];
    strAppendUnsigned(&s[2], i+1);

    bool isActive = (cfn->swtch != 0);

    if (isActive) {
      line = form->newLine(&grid);
      scol = 0;

      button = new SpecialFunctionButton(line, rect_t{0, 0, window->width() - 12, SF_BUTTON_H}, functions, i);

      if (firstActiveButton == nullptr)
        firstActiveButton = button;

      button->setLongPressHandler([=]() -> uint8_t {
        if (firstInactiveButton)
          lv_group_focus_obj(firstInactiveButton->getLvObj());
        return 0;
      });

      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER, 0, PER_ROW, LV_GRID_ALIGN_CENTER, 0, 1);
    } else {
      if (scol == 0) {
        line = form->newLine(&grid);
        lv_obj_set_style_pad_column(line->getLvObj(), 4, LV_PART_MAIN);
      }

      button = new TextButton(line, rect_t{0, 0, 0, SF_BUTTON_H}, s);

      if (firstInactiveButton == nullptr)
        firstInactiveButton = button;

      button->setLongPressHandler([=]() -> uint8_t {
        if (firstActiveButton)
          lv_group_focus_obj(firstActiveButton->getLvObj());
        return 0;
      });

      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_STRETCH, scol, 1, LV_GRID_ALIGN_CENTER, 0, 1);
      scol = (scol + 1) % PER_ROW;
    }

    if (focusIndex == i) {
      hasFocusButton = true;
      lv_group_focus_obj(button->getLvObj());
    }

    button->setFocusHandler([=](bool hasFocus) {
      if (hasFocus && !isRebuilding)
        focusIndex = i;
    });

    button->setPressHandler([=]() {
      Menu *menu = new Menu(window);
      menu->addLine(STR_EDIT, [=]() {
        editSpecialFunction(window, i);
       });
      if (isActive) {
        menu->addLine(STR_COPY, [=]() {
          clipboard.type = CLIPBOARD_TYPE_CUSTOM_FUNCTION;
          clipboard.data.cfn = *cfn;
        });
      }
      if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION) {
        menu->addLine(STR_PASTE, [=]() {
          if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
            LUA_LOAD_MODEL_SCRIPTS();
          *cfn = clipboard.data.cfn;
          if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
            LUA_LOAD_MODEL_SCRIPTS();
          SET_DIRTY();
          rebuild(window);
        });
      }
      if (functions[MAX_SPECIAL_FUNCTIONS - 1].isEmpty()) {
        for (int j = i; j < MAX_SPECIAL_FUNCTIONS; j++) {
          if (!functions[j].isEmpty()) {
            menu->addLine(STR_INSERT, [=]() {
              memmove(
                  cfn + 1, cfn,
                  (MAX_SPECIAL_FUNCTIONS - i - 1) * sizeof(CustomFunctionData));
              memset(cfn, 0, sizeof(CustomFunctionData));
              SET_DIRTY();
              focusIndex = i + 1;
              rebuild(window);
            });
            break;
          }
        }
      }
      if (isActive) {
        menu->addLine(STR_CLEAR, [=]() {
          if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
            LUA_LOAD_MODEL_SCRIPTS();
          memset(cfn, 0, sizeof(CustomFunctionData));
          SET_DIRTY();
          focusIndex = i;
          rebuild(window);
        });
      }
      for (int j = i; j < MAX_SPECIAL_FUNCTIONS; j++) {
        if (!functions[j].isEmpty()) {
          menu->addLine(STR_DELETE, [=]() {
            if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
              LUA_LOAD_MODEL_SCRIPTS();
            memmove(
                cfn, cfn + 1,
                (MAX_SPECIAL_FUNCTIONS - i - 1) * sizeof(CustomFunctionData));
            memset(&functions[MAX_SPECIAL_FUNCTIONS - 1], 0,
                   sizeof(CustomFunctionData));
            SET_DIRTY();
            focusIndex = i;
            rebuild(window);
          });
          break;
        }
      }
      return 0;
    });
  }

  if (!hasFocusButton)
  {
    if (firstActiveButton)
      lv_group_focus_obj(firstActiveButton->getLvObj());
    else if (firstInactiveButton)
      lv_group_focus_obj(firstInactiveButton->getLvObj());
  }
}
