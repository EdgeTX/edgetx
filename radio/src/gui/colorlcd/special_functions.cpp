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

#include "hal/adc_driver.h"
#include "strhelpers.h"

#define SET_DIRTY()     storageDirty(functions == g_model.customFn ? EE_MODEL : EE_GENERAL)

static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
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
  FormWindow *specialFunctionOneWindow = nullptr;
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
    headerSF = header.setTitle2((functions == g_model.customFn ? "SF" : "GF") + std::to_string(index+1));

    lv_obj_set_style_text_color(headerSF->getLvObj(), makeLvColor(COLOR_THEME_ACTIVE), LV_STATE_USER_1);
    lv_obj_set_style_text_font(headerSF->getLvObj(), getFont(FONT(BOLD)), LV_STATE_USER_1);
  }

  void addSourceChoice(FormWindow::Line* line, const char* title, CustomFunctionData* cfn, int16_t vmax)
  {
    new StaticText(line, rect_t{}, title, 0, COLOR_THEME_PRIMARY1);
    new SourceChoice(line, rect_t{}, 0, vmax, GET_SET_DEFAULT(CFN_PARAM(cfn)));
  }

  NumberEdit* addNumberEdit(FormWindow::Line* line, const char* title, CustomFunctionData* cfn, int16_t vmin, int16_t vmax)
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
        auto max_sticks = adcGetMaxInputs(ADC_INPUT_MAIN);
        auto choice = new Choice(line, rect_t{}, 0, max_sticks + 1,
                                 GET_SET_DEFAULT(CFN_CH_INDEX(cfn)));
        choice->setTextHandler([=](int32_t value) {
          if (value == 0)
            return std::string(STR_STICKS);
          else if (value == MAX_STICKS + 1)
            return std::string(STR_CHANS);

          return std::string(getMainControlLabel(value - 1));
        });
        break;
      }

      case FUNC_RESET:
        if (CFN_PARAM(cfn) <= FUNC_RESET_PARAM_LAST) {
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
      case FUNC_RGB_LED:
        new StaticText(line, rect_t{}, STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new FileChoice(
            line, rect_t{},
            func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED
                ? (func == FUNC_PLAY_SCRIPT ? SCRIPTS_FUNCS_PATH : SCRIPTS_RGB_PATH)
                : std::string(SOUNDS_PATH, SOUNDS_PATH_LNG_OFS) +
                      std::string(currentLanguagePack->id, 2),
            (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED) ? SCRIPTS_EXT : SOUNDS_EXT,
            sizeof(cfn->play.name),
            [=]() { return std::string(cfn->play.name, ZLEN(cfn->play.name)); },
            [=](std::string newValue) {
              strncpy(cfn->play.name, newValue.c_str(), sizeof(cfn->play.name));
              SET_DIRTY();
              if (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED)
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
        timerchoice->setAvailableHandler([=](int value) {
          return isTimerSourceAvailable(value);
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
        addSourceChoice(line, STR_VALUE, cfn, MIXSRC_LAST_TELEM);
        break;

      case FUNC_HAPTIC:
        addNumberEdit(line, STR_VALUE, cfn, 0, 3);
        break;

      case FUNC_LOGS: {
        if(CFN_PARAM(cfn) == 0)                           // use stored value if SF exists
          CFN_PARAM(cfn) = SD_LOGS_PERIOD_DEFAULT;        // otherwise initialize with default value

        auto edit = addNumberEdit(line, STR_INTERVAL, cfn, SD_LOGS_PERIOD_MIN, SD_LOGS_PERIOD_MAX);
        edit->setDefault(SD_LOGS_PERIOD_DEFAULT);         // set default period for DEF button
        edit->setDisplayHandler(
            [=](int32_t value) {
              return formatNumberAsString(CFN_PARAM(cfn), PREC1, 0, nullptr, "s");
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
                  return formatNumberAsString(abs(value), 0, 0, value >= 0 ? "+=" : "-=", nullptr);
                });
            break;
          }
        }
      }
    }

    if (HAS_REPEAT_PARAM(func)) {  // !1x 1x 1s 2s 3s ...
      line = specialFunctionOneWindow->newLine(&grid);
      new StaticText(line, rect_t{}, STR_REPEAT,
                     0, COLOR_THEME_PRIMARY1);
      if (func == FUNC_PLAY_SCRIPT) {
        auto repeat = new Choice(line, rect_t{}, 0, 1, 
                                 GET_DEFAULT((int8_t)CFN_PLAY_REPEAT(cfn)),
                                 SET_DEFAULT(CFN_PLAY_REPEAT(cfn)));
        repeat->setTextHandler([](int32_t value) {
            // 0 == repeat at 50ms interval for backward compatibility
            return (value == 0) ? std::string("On") : std::string("1x");
        });
      } else {
        auto repeat = new NumberEdit(line, rect_t{}, -1, 60 / CFN_PLAY_REPEAT_MUL,
                                     GET_DEFAULT((int8_t)CFN_PLAY_REPEAT(cfn)),
                                     SET_DEFAULT(CFN_PLAY_REPEAT(cfn)));
        repeat->setDisplayHandler([](int32_t value) {
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

    line = specialFunctionOneWindow->newLine(&grid);
    new StaticText(line, rect_t{}, STR_ENABLE, 0, COLOR_THEME_PRIMARY1);
    new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(CFN_ACTIVE(cfn)));

  }

  void buildBody(FormWindow *window)
  {
    window->padAll(0);
    lv_obj_set_scrollbar_mode(window->getLvObj(), LV_SCROLLBAR_MODE_AUTO);

    auto form = new FormWindow(window, rect_t{});
    form->setFlexLayout();
    form->padAll(8);

    FlexGridLayout grid(col_dsc, row_dsc, 2);

    CustomFunctionData *cfn = &functions[index];

    // Set new function to "disabled" by default
    if (!CFN_SWITCH(cfn))
      CFN_ACTIVE(cfn) = false;

    // Switch
    auto line = form->newLine(&grid);
    new StaticText(line, rect_t{}, STR_SF_SWITCH, 0, COLOR_THEME_PRIMARY1);
    auto switchChoice =
        new SwitchChoice(line, rect_t{}, SWSRC_FIRST, SWSRC_LAST,
                         GET_SET_DEFAULT(CFN_SWITCH(cfn)));
    switchChoice->setAvailableHandler([=](int value) {
      return (functions == g_model.customFn
                  ? isSwitchAvailable(value, ModelCustomFunctionsContext)
                  : isSwitchAvailable(value, GeneralCustomFunctionsContext));
    });

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
    line = form->newLine(&grid);
    new StaticText(line, rect_t{}, STR_FUNC, 0, COLOR_THEME_PRIMARY1);
    auto functionChoice =
        new Choice(line, rect_t{}, 0, FUNC_MAX - 1,
                   GET_DEFAULT(CFN_FUNC(cfn)));
    functionChoice->setTextHandler([=](int val) {
      return funcGetLabel(val);
    });
    functionChoice->setSetValueHandler([=](int32_t newValue) {
      CFN_FUNC(cfn) = newValue;
      CFN_RESET(cfn);
      SET_DIRTY();
      updateSpecialFunctionOneWindow();
    });
    functionChoice->setAvailableHandler([=](int value) {
      return isAssignableFunctionAvailable(value, functions);
    });

    specialFunctionOneWindow = new FormWindow(form, rect_t{});
    updateSpecialFunctionOneWindow();
  }
};

#if LCD_W > LCD_H

static const lv_coord_t b_col_dsc[] = {
  43, 70, LV_GRID_FR(1), 40, 30,
  LV_GRID_TEMPLATE_LAST
};

static const lv_coord_t b_row_dsc[] = {
  LV_GRID_CONTENT,
  LV_GRID_TEMPLATE_LAST};

#define NM_ROW_CNT  1
#define FUNC_COL    2
#define FUNC_ROW    0

#else

static const lv_coord_t b_col_dsc[] = {
  40, LV_GRID_FR(1), 34, 24,
  LV_GRID_TEMPLATE_LAST
};

static const lv_coord_t b_row_dsc[] = {
  LV_GRID_CONTENT, LV_GRID_CONTENT,
  LV_GRID_TEMPLATE_LAST};

#define NM_ROW_CNT  2
#define FUNC_COL    1
#define FUNC_ROW    1

#endif

static const char* _failsafe_module[] = {
  "Int.", "Ext.",
};

class SpecialFunctionButton : public Button
{
 public:
  SpecialFunctionButton(Window *parent, const rect_t &rect,
                        CustomFunctionData *functions, uint8_t index) :
      Button(parent, rect, nullptr, 0, 0, input_mix_line_create), functions(functions), index(index)
  {
#if LCD_H > LCD_W
    padTop(0);
#endif
    padLeft(4);
    lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(lvobj, b_col_dsc, b_row_dsc);
    lv_obj_set_style_pad_row(lvobj, 0, 0);
    lv_obj_set_style_pad_column(lvobj, 4, 0);

    check(isActive());

    lv_obj_add_event_cb(lvobj, SpecialFunctionButton::on_draw, LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SpecialFunctionButton"; }
#endif

  static void on_draw(lv_event_t * e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (SpecialFunctionButton*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init(e);
      else
        line->refresh();
    }
  }
  
  void delayed_init(lv_event_t* e)
  {
    sfName = lv_label_create(lvobj);
    lv_obj_set_style_text_align(sfName, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(sfName, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, NM_ROW_CNT);

    sfSwitch = lv_label_create(lvobj);
    lv_obj_set_style_text_align(sfSwitch, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_grid_cell(sfSwitch, LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

    sfFunc = lv_label_create(lvobj);
    lv_obj_set_style_text_align(sfFunc, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(sfFunc, LV_GRID_ALIGN_START, FUNC_COL, 1, LV_GRID_ALIGN_CENTER, FUNC_ROW, 1);

    sfRepeat = lv_label_create(lvobj);
    lv_obj_set_style_text_align(sfRepeat, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(sfRepeat, LV_GRID_ALIGN_CENTER, FUNC_COL+1, 1, LV_GRID_ALIGN_CENTER, 0, NM_ROW_CNT);

    sfEnable = lv_obj_create(lvobj);
    lv_obj_set_size(sfEnable, 22, 22);
    lv_obj_set_style_border_width(sfEnable, 3, 0);
    lv_obj_set_style_border_color(sfEnable, makeLvColor(COLOR_THEME_PRIMARY2), 0);
    lv_obj_set_style_border_opa(sfEnable, LV_OPA_100, 0);
    lv_obj_set_style_bg_color(sfEnable, makeLvColor(COLOR_THEME_ACTIVE), LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(sfEnable, LV_OPA_100, 0);
    auto sfEnableInner = lv_obj_create(sfEnable);
    lv_obj_set_size(sfEnableInner, 16, 16);
    lv_obj_set_style_border_width(sfEnableInner, 2, 0);
    lv_obj_set_style_border_color(sfEnableInner, makeLvColor(COLOR_THEME_SECONDARY1), 0);
    lv_obj_set_style_border_opa(sfEnableInner, LV_OPA_100, 0);
    lv_obj_set_style_bg_opa(sfEnableInner, LV_OPA_0, 0);
    lv_obj_set_grid_cell(sfEnable, LV_GRID_ALIGN_CENTER, FUNC_COL+2, 1, LV_GRID_ALIGN_CENTER, 0, NM_ROW_CNT);

    init = true;
    refresh();

    if(e) {
      auto param = lv_event_get_param(e);
      lv_event_send(lvobj, LV_EVENT_DRAW_MAIN, param);
    }
  }

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
    check(isActive());
  }

  void refresh()
  {
    if (!init) return;
    const CustomFunctionData *cfn = &functions[index];
    // TODO: Is this necessary?
    if (functions[index].func == FUNC_OVERRIDE_CHANNEL &&
        functions != g_model.customFn) {
      functions[index].func = FUNC_OVERRIDE_CHANNEL + 1;
    }
    uint8_t func = CFN_FUNC(cfn);

    char s[32] = "SFxx";
    if (functions == g_eeGeneral.customFn) s[0] = 'G';
    strAppendUnsigned(&s[2], index+1);

    lv_label_set_text(sfName, s);
    lv_label_set_text(sfSwitch, getSwitchPositionName(CFN_SWITCH(cfn)));

    strcpy(s, funcGetLabel(func));
    strcat(s, " - ");

    switch (func) {
      case FUNC_OVERRIDE_CHANNEL:
        sprintf(s+strlen(s), "%s = %s", getSourceString(MIXSRC_FIRST_CH + CFN_CH_INDEX(cfn)), formatNumberAsString(CFN_PARAM(cfn)).c_str());
        break;

      case FUNC_TRAINER: {
        int16_t value = CFN_CH_INDEX(cfn);
        if (value == 0)
          strcat(s, STR_STICKS);
        else if (value == MAX_STICKS + 1)
          strcat(s, STR_CHANS);
        else
          strcat(s, getMainControlLabel(value - 1));
        break;
      }

      case FUNC_RESET:
        if (CFN_PARAM(cfn) < FUNC_RESET_PARAM_FIRST_TELEM) {
          strcat(s, STR_VFSWRESET[CFN_PARAM(cfn)]);
        } else {
          TelemetrySensor *sensor = &g_model.telemetrySensors[CFN_PARAM(cfn) - FUNC_RESET_PARAM_FIRST_TELEM];
          strAppend(s+strlen(s), sensor->label, TELEM_LABEL_LEN);
        }
        break;

      case FUNC_VOLUME:
      case FUNC_BACKLIGHT:
      case FUNC_PLAY_VALUE:
        strcat(s, getSourceString(CFN_PARAM(cfn)));
        break;

      case FUNC_PLAY_SOUND:
        strcat(s, STR_FUNCSOUNDS[CFN_PARAM(cfn)]);
        break;

      case FUNC_PLAY_TRACK:
      case FUNC_BACKGND_MUSIC:
      case FUNC_PLAY_SCRIPT:
        if (ZEXIST(cfn->play.name)) {
          strAppend(s+strlen(s), cfn->play.name, LEN_FUNCTION_NAME);
        } else {
          strcat(s, STR_VCSWFUNC[0]);
        }
        break;

      case FUNC_SET_TIMER:
        sprintf(s+strlen(s), "%s%d = %s", STR_SRC_TIMER, CFN_TIMER_INDEX(cfn) + 1, getTimerString(CFN_PARAM(cfn), {.options = SHOW_TIME}));
        break;

      case FUNC_SET_FAILSAFE:
        strcat(s, _failsafe_module[CFN_PARAM(cfn)]);
        break;

      case FUNC_HAPTIC:
      case FUNC_SET_SCREEN:
        strcat(s, formatNumberAsString(CFN_PARAM(cfn)).c_str());
        break;

      case FUNC_LOGS:
        strcat(s, formatNumberAsString(CFN_PARAM(cfn), PREC1, 0, nullptr, "s").c_str());
        break;

      case FUNC_ADJUST_GVAR:
        strcat(s, getSourceString(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR));
        switch(CFN_GVAR_MODE(cfn)) {
          case FUNC_ADJUST_GVAR_CONSTANT:
            sprintf(s+strlen(s), " = %s", formatNumberAsString(CFN_PARAM(cfn)).c_str());
            break;
          case FUNC_ADJUST_GVAR_SOURCE:
            sprintf(s+strlen(s), " = %s", getSourceString(CFN_PARAM(cfn)));
            break;
          case FUNC_ADJUST_GVAR_GVAR:
            sprintf(s+strlen(s), " = %s", getSourceString(CFN_PARAM(cfn) + MIXSRC_FIRST_GVAR));
            break;
          case FUNC_ADJUST_GVAR_INCDEC: {
            int16_t value = CFN_PARAM(cfn);
            sprintf(s+strlen(s), " %s= %d", (value >= 0) ? "+" : "-", abs(value));
            break;
          }
        }
        break;
        
      default:
        strcpy(s, funcGetLabel(func));
        break;
    }

    lv_label_set_text(sfFunc, s);

    s[0] = 0;

    if (CFN_ACTIVE(cfn))
      lv_obj_add_state(sfEnable, LV_STATE_CHECKED);
    else
      lv_obj_clear_state(sfEnable, LV_STATE_CHECKED);

    if (HAS_REPEAT_PARAM(func)) {
      if (func == FUNC_PLAY_SCRIPT) {
        sprintf(s, "(%s)", (CFN_PLAY_REPEAT(cfn) == 0) ? "On" : "1x");
      } else {
        sprintf(s, "(%s)",
          (CFN_PLAY_REPEAT(cfn) == 0) ? "1x" :
          (CFN_PLAY_REPEAT(cfn) == CFN_PLAY_REPEAT_NOSTART) ? "!1x" :
          formatNumberAsString(CFN_PLAY_REPEAT(cfn) * CFN_PLAY_REPEAT_MUL, 0, 0, nullptr, "s").c_str()
        );
      }
    }

    lv_label_set_text(sfRepeat, s);
  }

 protected:
  bool init = false;
  CustomFunctionData *functions;
  uint8_t index;

  lv_obj_t* sfName = nullptr;
  lv_obj_t* sfSwitch = nullptr;
  lv_obj_t* sfFunc = nullptr;
  lv_obj_t* sfRepeat = nullptr;
  lv_obj_t* sfEnable = nullptr;
};

#if LCD_W > LCD_H
#define SF_BUTTON_H 34
#else
#define SF_BUTTON_H 45
#endif

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
  // When window.clear() is called the last button on screen is given focus (???)
  // This causes the page to jump to the end when rebuilt.
  // Set flag to bypass the button focus handler and reset focusIndex when rebuilding
  isRebuilding = true;
  window->clear();
  build(window);
  isRebuilding = false;
}

void SpecialFunctionsPage::newSF(FormWindow * window, bool pasteSF)
{
  Menu* menu = new Menu(Layer::back());
  menu->setTitle(functions == g_model.customFn ? STR_MENUCUSTOMFUNC : STR_MENUSPECIALFUNCS);
  char s[6] = "SFxx";
  if (functions == g_eeGeneral.customFn) s[0] = 'G';

  // search for unused switches
  for (uint8_t i = 0; i < MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData *cfn = &functions[i];
    if (cfn->swtch == 0) {
      strAppendUnsigned(&s[2], i+1);
      menu->addLineBuffered(s, [=]() {
        if (pasteSF) {
          pasteSpecialFunction(window, i, nullptr);
        } else {
          editSpecialFunction(window, i, nullptr);
        }
      });
    }
  }
  menu->updateLines();
}

void SpecialFunctionsPage::pasteSpecialFunction(FormWindow * window, uint8_t index, Button* button)
{
  CustomFunctionData *cfn = &functions[index];
  if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
    LUA_LOAD_MODEL_SCRIPTS();
  *cfn = clipboard.data.cfn;
  if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
    LUA_LOAD_MODEL_SCRIPTS();
  storageDirty(EE_MODEL);
  focusIndex = index;
  if (button)
    lv_event_send(button->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
  else
    rebuild(window);
}

void SpecialFunctionsPage::editSpecialFunction(FormWindow * window, uint8_t index, Button* button)
{
  auto editPage = new SpecialFunctionEditPage(functions, index);
  editPage->setCloseHandler([=]() {
    CustomFunctionData *cfn = &functions[index];
    if (cfn->swtch != 0) {
      focusIndex = index;
      if (button)
        lv_event_send(button->getLvObj(), LV_EVENT_VALUE_CHANGED, nullptr);
      else
        rebuild(window);
    }
  });
}

void SpecialFunctionsPage::plusPopup(FormWindow * window)
{
  if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION) {
    Menu* menu = new Menu(window);
    menu->addLine(STR_NEW, [=]() {
      newSF(window, false);
    });
    menu->addLine(STR_PASTE, [=]() {
      newSF(window, true);
    });
  } else {
    newSF(window, false);
  }
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
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);

  FlexGridLayout grid(l_col_dsc, row_dsc, 2);

  FormWindow::Line* line;
  bool hasEmptyFunction = false;
  Button* button;

  // Reset focusIndex after switching tabs
  if (!isRebuilding)
    focusIndex = prevFocusIndex;

  char s[] = "SFxx";
  if (functions == g_eeGeneral.customFn) s[0] = 'G';

  for (uint8_t i = 0; i < MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData *cfn = &functions[i];
    strAppendUnsigned(&s[2], i+1);

    bool isActive = (cfn->swtch != 0);

    if (isActive) {
      line = window->newLine(&grid);

      button = new SpecialFunctionButton(line, rect_t{0, 0, window->width() - 12, SF_BUTTON_H}, functions, i);

      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER, 0, PER_ROW, LV_GRID_ALIGN_CENTER, 0, 1);

      if (focusIndex == i) {
        lv_group_focus_obj(button->getLvObj());
      }

      button->setFocusHandler([=](bool hasFocus) {
        if (hasFocus && !isRebuilding) {
          prevFocusIndex = focusIndex;
          focusIndex = i;
        }
      });

      button->setPressHandler([=]() {
        Menu *menu = new Menu(window);
        menu->addLine(STR_EDIT, [=]() {
          editSpecialFunction(window, i, button);
        });
        if (isActive) {
          menu->addLine(STR_COPY, [=]() {
            clipboard.type = CLIPBOARD_TYPE_CUSTOM_FUNCTION;
            clipboard.data.cfn = *cfn;
          });
        }
        if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION) {
          menu->addLine(STR_PASTE, [=]() {
            pasteSpecialFunction(window, i, button);
          });
        }
        CustomFunctionData *cfn = &functions[i];
        uint8_t func = CFN_FUNC(cfn);
        if (CFN_ACTIVE(cfn)) {
          menu->addLine(STR_DISABLE, [=]() {
            CFN_ACTIVE(cfn) = 0;
            SET_DIRTY();
            rebuild(window);
          });
        } else {
          menu->addLine(STR_ENABLE, [=]() {
            CFN_ACTIVE(cfn) = 1;
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
                editSpecialFunction(window, i, nullptr);
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
              rebuild(window);
            });
            break;
          }
        }
        return 0;
      });

      button->setLongPressHandler([=]() -> uint8_t {
        if (addButton) {
          lv_group_focus_obj(addButton->getLvObj());
          plusPopup(window);
        }
        return 0;
      });
    } else {
      hasEmptyFunction = true;
    }
  }

  if (hasEmptyFunction)
  {
    line = window->newLine(&grid);
    addButton = new TextButton(line, rect_t{0, 0, window->width() - 12, SF_BUTTON_H}, LV_SYMBOL_PLUS, [=]() {
      plusPopup(window);
      return 0;
    });

    addButton->setLongPressHandler([=]() -> uint8_t {
      plusPopup(window);
      return 0;
    });

    addButton->setFocusHandler([=](bool hasFocus) {
      if (hasFocus && !isRebuilding) {
        prevFocusIndex = focusIndex;
      }
    });
  }
  else
  {
    addButton = nullptr;
  }
}
