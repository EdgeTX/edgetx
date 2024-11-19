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

#include "filechoice.h"
#include "hal/adc_driver.h"
#include "libopenui.h"
#include "page.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "timeedit.h"
#include "view_main.h"

#define SET_DIRTY() setDirty()

//-----------------------------------------------------------------------------

static const char *_failsafe_module[] = {
    "Int.",
    "Ext.",
};

static const lv_style_const_prop_t sf_enable_state_style_props[] = {
    LV_STYLE_CONST_OUTLINE_WIDTH(3),
    LV_STYLE_CONST_OUTLINE_OPA(LV_OPA_COVER),
    LV_STYLE_CONST_OUTLINE_PAD(1),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(sf_enable_state_style, sf_enable_state_style_props);

static void sf_enable_state_constructor(const lv_obj_class_t *class_p,
                                        lv_obj_t *obj)
{
  etx_obj_add_style(obj, sf_enable_state_style, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->outline_color_light, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->border, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->border_color[COLOR_THEME_SECONDARY1_INDEX], LV_PART_MAIN);
  etx_obj_add_style(obj, styles->bg_opacity_cover, LV_PART_MAIN);
  etx_bg_color(obj, COLOR_THEME_ACTIVE_INDEX, LV_STATE_CHECKED);
}

static const lv_obj_class_t sf_enable_state = {
    .base_class = &lv_obj_class,
    .constructor_cb = sf_enable_state_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FunctionLineButton::EN_SZ,
    .height_def = FunctionLineButton::EN_SZ,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t),
};

static lv_obj_t *sf_enable_state_create(lv_obj_t *parent)
{
  return etx_create(&sf_enable_state, parent);
}

FunctionLineButton::FunctionLineButton(Window *parent, const rect_t &rect,
                                       const CustomFunctionData *cfn,
                                       uint8_t index, const char *prefix) :
    ListLineButton(parent, index), cfn(cfn), prefix(prefix)
{
  setHeight(FunctionsPage::SF_BUTTON_H);
  padAll(PAD_ZERO);

  lv_obj_add_event_cb(lvobj, FunctionLineButton::on_draw,
                      LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
}

void FunctionLineButton::on_draw(lv_event_t *e)
{
  lv_obj_t *target = lv_event_get_target(e);
  auto line = (FunctionLineButton *)lv_obj_get_user_data(target);
  if (line) {
    if (!line->init)
      line->delayed_init();
    line->refresh();
  }
}

void FunctionLineButton::delayed_init()
{
  init = true;

  lv_obj_enable_style_refresh(false);

  sfName = lv_label_create(lvobj);
  lv_obj_set_pos(sfName, NM_X, NM_Y);
  lv_obj_set_size(sfName, NM_W, NM_H);

  sfSwitch = lv_label_create(lvobj);
  lv_obj_set_pos(sfSwitch, SW_X, SW_Y);
  lv_obj_set_size(sfSwitch, SW_W, SW_H);

  sfFunc = lv_label_create(lvobj);
  lv_obj_set_pos(sfFunc, FN_X, FN_Y);
  lv_obj_set_size(sfFunc, FN_W, FN_H);

  sfRepeat = lv_label_create(lvobj);
  lv_obj_set_pos(sfRepeat, RP_X, RP_Y);
  lv_obj_set_size(sfRepeat, RP_W, RP_H);

  sfEnable = sf_enable_state_create(lvobj);
  lv_obj_clear_flag(sfEnable, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_set_user_data(sfEnable, this);
  lv_obj_set_pos(sfEnable, EN_X, EN_Y);

  lv_obj_update_layout(lvobj);

  lv_obj_enable_style_refresh(true);
  lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
}

void FunctionLineButton::refresh()
{
  if (!init) return;

  check(isActive());

  uint8_t func = CFN_FUNC(cfn);

  char s[64];

  lv_label_set_text(sfName, (prefix + std::to_string(index + 1)).c_str());
  lv_label_set_text(sfSwitch, getSwitchPositionName(CFN_SWITCH(cfn)));

  strcpy(s, funcGetLabel(func));
  strcat(s, " - ");

  switch (func) {
    case FUNC_OVERRIDE_CHANNEL:
      sprintf(s + strlen(s), "%s = %s",
              getSourceString(MIXSRC_FIRST_CH + CFN_CH_INDEX(cfn)),
              formatNumberAsString(CFN_PARAM(cfn)).c_str());
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
        TelemetrySensor *sensor =
            &g_model.telemetrySensors[CFN_PARAM(cfn) -
                                      FUNC_RESET_PARAM_FIRST_TELEM];
        strAppend(s + strlen(s), sensor->label, TELEM_LABEL_LEN);
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
    case FUNC_RGB_LED:
      if (ZEXIST(cfn->play.name)) {
        strAppend(s + strlen(s), cfn->play.name, LEN_FUNCTION_NAME);
      } else {
        strcat(s, STR_VCSWFUNC[0]);
      }
      break;

    case FUNC_SET_TIMER:
      sprintf(s + strlen(s), "%s%d = %s", STR_SRC_TIMER,
              CFN_TIMER_INDEX(cfn) + 1,
              getTimerString(CFN_PARAM(cfn), {.options = SHOW_TIME}));
      break;

    case FUNC_SET_FAILSAFE:
      strcat(s, _failsafe_module[CFN_PARAM(cfn)]);
      break;

    case FUNC_HAPTIC:
    case FUNC_SET_SCREEN:
      strcat(s, formatNumberAsString(CFN_PARAM(cfn)).c_str());
      break;

#if defined(FUNCTION_SWITCHES)
    case FUNC_PUSH_CUST_SWITCH:
      sprintf(s + strlen(s), "%s%d", STR_SWITCH, CFN_CS_INDEX(cfn) + 1);
      break;
#endif

    case FUNC_LOGS:
      strcat(
          s,
          formatNumberAsString(CFN_PARAM(cfn), PREC1, 0, nullptr, "s").c_str());
      break;

    case FUNC_ADJUST_GVAR:
      strcat(s, getSourceString(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR));
      switch (CFN_GVAR_MODE(cfn)) {
        case FUNC_ADJUST_GVAR_CONSTANT:
          sprintf(s + strlen(s), " = %s",
                  formatNumberAsString(CFN_PARAM(cfn)).c_str());
          break;
        case FUNC_ADJUST_GVAR_SOURCE:
        case FUNC_ADJUST_GVAR_SOURCERAW:
          sprintf(s + strlen(s), " = %s", getSourceString(CFN_PARAM(cfn)));
          break;
        case FUNC_ADJUST_GVAR_GVAR:
          sprintf(s + strlen(s), " = %s",
                  getSourceString(CFN_PARAM(cfn) + MIXSRC_FIRST_GVAR));
          break;
        case FUNC_ADJUST_GVAR_INCDEC: {
          int16_t value = CFN_PARAM(cfn);
          sprintf(s + strlen(s), " %s= %d", (value >= 0) ? "+" : "-",
                  abs(value));
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
    if (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED) {
      sprintf(s, "(%s)", (CFN_PLAY_REPEAT(cfn) == 0) ? "On" : "1x");
    } else {
      sprintf(
          s, "(%s)",
          (CFN_PLAY_REPEAT(cfn) == 0) ? "1x"
          : (CFN_PLAY_REPEAT(cfn) == CFN_PLAY_REPEAT_NOSTART)
              ? "!1x"
              : formatNumberAsString(CFN_PLAY_REPEAT(cfn) * CFN_PLAY_REPEAT_MUL,
                                     0, 0, nullptr, "s")
                    .c_str());
    }
  }

  lv_label_set_text(sfRepeat, s);
}

//-----------------------------------------------------------------------------

static const lv_coord_t col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(3),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

#define ETX_STATE_SF_ACTIVE LV_STATE_USER_1

#define SD_LOGS_PERIOD_MIN 1       // 0.1s  fastest period
#define SD_LOGS_PERIOD_MAX 255     // 25.5s slowest period
#define SD_LOGS_PERIOD_DEFAULT 10  // 1s    default period for newly created SF

#define PUSH_CS_DURATION_MIN 0       // 0     no duration : as long as switch is true
#define PUSH_CS_DURATION_MAX 255     // 25.5s longest duration

FunctionEditPage::FunctionEditPage(uint8_t index, EdgeTxIcon icon,
                                   const char *title, const char *prefix) :
    Page(icon), index(index)
{
  buildHeader(header, title, prefix);

  lv_obj_add_event_cb(lvobj, FunctionEditPage::on_draw,
                      LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
}

void FunctionEditPage::on_draw(lv_event_t *e)
{
  lv_obj_t *target = lv_event_get_target(e);
  auto page = (FunctionEditPage *)lv_obj_get_user_data(target);
  if (page) page->delayed_init();
}

void FunctionEditPage::delayed_init()
{
  if (!init) {
    init = true;
    buildBody(body);
  }
}

void FunctionEditPage::checkEvents()
{
  Page::checkEvents();
  if (active != isActive()) {
    if (isActive()) {
      lv_obj_add_state(headerSF->getLvObj(), ETX_STATE_SF_ACTIVE);
    } else {
      lv_obj_clear_state(headerSF->getLvObj(), ETX_STATE_SF_ACTIVE);
    }
    active = isActive();
  }
}

void FunctionEditPage::buildHeader(Window *window, const char *title,
                                   const char *prefix)
{
  header->setTitle(title);
  headerSF = header->setTitle2(prefix + std::to_string(index + 1));

  etx_txt_color(headerSF->getLvObj(), COLOR_THEME_ACTIVE_INDEX,
                ETX_STATE_SF_ACTIVE);
  etx_font(headerSF->getLvObj(), FONT_BOLD_INDEX, ETX_STATE_SF_ACTIVE);
}

void FunctionEditPage::addSourceChoice(FormLine *line, const char *title,
                                       CustomFunctionData *cfn, int16_t vmax)
{
  new StaticText(line, rect_t{}, title);
  new SourceChoice(line, rect_t{}, 0, vmax, GET_SET_DEFAULT(CFN_PARAM(cfn)), true);
}

NumberEdit *FunctionEditPage::addNumberEdit(FormLine *line, const char *title,
                                            CustomFunctionData *cfn,
                                            int16_t vmin, int16_t vmax)
{
  new StaticText(line, rect_t{}, title);
  return new NumberEdit(line, rect_t{}, vmin, vmax,
                        GET_SET_DEFAULT(CFN_PARAM(cfn)));
}

void FunctionEditPage::updateSpecialFunctionOneWindow()
{
  specialFunctionOneWindow->clear();
  specialFunctionOneWindow->setFlexLayout();
  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);
  auto line = specialFunctionOneWindow->newLine(grid);

  CustomFunctionData *cfn = customFunctionData();
  uint8_t func = CFN_FUNC(cfn);

  // Func param
  switch (func) {
    case FUNC_OVERRIDE_CHANNEL: {
      new StaticText(line, rect_t{}, STR_CH);
      new NumberEdit(line, rect_t{}, 1, MAX_OUTPUT_CHANNELS,
                     GET_SET_VALUE_WITH_OFFSET(CFN_CH_INDEX(cfn), 1));
      line = specialFunctionOneWindow->newLine(grid);

      int limit =
          (g_model.extendedLimits ? LIMIT_EXT_PERCENT : LIMIT_STD_PERCENT);
      addNumberEdit(line, STR_VALUE, cfn, -limit, limit);
      break;
    }

    case FUNC_TRAINER: {
      new StaticText(line, rect_t{}, STR_VALUE);
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
        new StaticText(line, rect_t{}, STR_RESET);
        auto choice =
            new Choice(line, rect_t{}, 0,
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
      new StaticText(line, rect_t{}, STR_VALUE);
      new Choice(line, rect_t{}, STR_FUNCSOUNDS, 0,
                 AU_SPECIAL_SOUND_LAST - AU_SPECIAL_SOUND_FIRST - 1,
                 GET_SET_DEFAULT(CFN_PARAM(cfn)));
      break;

    case FUNC_PLAY_TRACK:
    case FUNC_BACKGND_MUSIC:
    case FUNC_PLAY_SCRIPT:
    case FUNC_RGB_LED:
      new StaticText(line, rect_t{}, STR_VALUE);
      new FileChoice(
          line, rect_t{},
          func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED
              ? (func == FUNC_PLAY_SCRIPT ? SCRIPTS_FUNCS_PATH
                                          : SCRIPTS_RGB_PATH)
              : std::string(SOUNDS_PATH, SOUNDS_PATH_LNG_OFS) +
                    std::string(currentLanguagePack->id, 2),
          (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED) ? SCRIPTS_EXT
                                                             : SOUNDS_EXT,
          sizeof(cfn->play.name),
          [=]() { return std::string(cfn->play.name, ZLEN(cfn->play.name)); },
          [=](std::string newValue) {
            strncpy(cfn->play.name, newValue.c_str(), sizeof(cfn->play.name));
            SET_DIRTY();
            if (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED)
              LUA_LOAD_MODEL_SCRIPTS();
          },
          true, funcGetLabel(func));
      break;

    case FUNC_SET_TIMER: {
      new StaticText(line, rect_t{}, STR_TIMER);
      auto timerchoice = new Choice(line, rect_t{}, 0, TIMERS - 1,
                                    GET_SET_DEFAULT(CFN_TIMER_INDEX(cfn)));
      timerchoice->setTextHandler([](int32_t value) {
        return std::string(STR_TIMER) + std::to_string(value + 1);
      });
      timerchoice->setAvailableHandler(
          [=](int value) { return isTimerSourceAvailable(value); });

      line = specialFunctionOneWindow->newLine(grid);
      new StaticText(line, rect_t{}, STR_VALUE);
      new TimeEdit(line, rect_t{}, 0, 9 * 60 * 60 - 1,
                   GET_SET_DEFAULT(CFN_PARAM(cfn)));
      break;
    }

    static const char* const strModules[] = { "Int.", "Ext." };
    case FUNC_SET_FAILSAFE:
      new StaticText(line, rect_t{}, STR_MODULE);
      new Choice(line, rect_t{}, strModules, 0, NUM_MODULES - 1,
                 GET_SET_DEFAULT(CFN_PARAM(cfn)));
      break;

    case FUNC_PLAY_VALUE:
      addSourceChoice(line, STR_VALUE, cfn, MIXSRC_LAST_TELEM);
      break;

    case FUNC_HAPTIC:
      addNumberEdit(line, STR_VALUE, cfn, 0, 3);
      break;

#if defined(FUNCTION_SWITCHES)
    case FUNC_PUSH_CUST_SWITCH: {
        new StaticText(line, rect_t{}, STR_SWITCH);
        auto choice = new Choice(line, rect_t{}, 0, NUM_FUNCTIONS_SWITCHES - 1, GET_SET_DEFAULT(CFN_CS_INDEX(cfn)), STR_SWITCH);
        choice->setTextHandler([=](int n) {
          return std::string(STR_SWITCH) + std::to_string(n + 1);
        });
        line = specialFunctionOneWindow->newLine(grid);

        auto edit = addNumberEdit(line, STR_INTERVAL, cfn, PUSH_CS_DURATION_MIN,
                                PUSH_CS_DURATION_MAX);

        edit->setDisplayHandler([=](int32_t value) {
          return formatNumberAsString(CFN_PARAM(cfn), PREC1, 0, nullptr, "s");
        });
      }
      break;
#endif

    case FUNC_LOGS: {
      if (CFN_PARAM(cfn) == 0)  // use stored value if SF exists
        CFN_PARAM(cfn) = SD_LOGS_PERIOD_DEFAULT;  // otherwise initialize with
                                                  // default value

      auto edit = addNumberEdit(line, STR_INTERVAL, cfn, SD_LOGS_PERIOD_MIN,
                                SD_LOGS_PERIOD_MAX);
      edit->setDefault(
          SD_LOGS_PERIOD_DEFAULT);  // set default period for DEF button
      edit->setDisplayHandler([=](int32_t value) {
        return formatNumberAsString(CFN_PARAM(cfn), PREC1, 0, nullptr, "s");
      });
      break;
    }

    case FUNC_SET_SCREEN:
      CFN_PARAM(cfn) = (int16_t)max(CFN_PARAM(cfn), (int16_t)1);
      CFN_PARAM(cfn) = (int16_t)min(
          CFN_PARAM(cfn), (int16_t)ViewMain::instance()->getMainViewsCount());
      addNumberEdit(line, STR_VALUE, cfn, 1,
                    ViewMain::instance()->getMainViewsCount());
      break;

    case FUNC_ADJUST_GVAR: {
      new StaticText(line, rect_t{}, STR_GLOBALVAR);
      auto gvarchoice = new Choice(line, rect_t{}, 0, MAX_GVARS - 1,
                                   GET_SET_DEFAULT(CFN_GVAR_INDEX(cfn)));
      gvarchoice->setTextHandler([](int32_t value) {
        return std::string(getSourceString(value + MIXSRC_FIRST_GVAR));
      });
      line = specialFunctionOneWindow->newLine(grid);

      new StaticText(line, rect_t{}, STR_MODE);
      auto modechoice = new Choice(line, rect_t{}, FUNC_ADJUST_GVAR_CONSTANT,
                                   FUNC_ADJUST_GVAR_INCDEC,
                                   GET_DEFAULT(CFN_GVAR_MODE(cfn)),
                                   [=](int32_t newValue) {
                                     CFN_GVAR_MODE(cfn) = newValue;
                                     CFN_PARAM(cfn) = 0;
                                     SET_DIRTY();
                                     updateSpecialFunctionOneWindow();
                                   });
      line = specialFunctionOneWindow->newLine(grid);

      modechoice->setTextHandler([](int32_t value) {
        switch (value) {
          case FUNC_ADJUST_GVAR_CONSTANT:
            return std::string(STR_CONSTANT);
          case FUNC_ADJUST_GVAR_SOURCE:
            return std::string(STR_MIXSOURCE);
          case FUNC_ADJUST_GVAR_SOURCERAW:
            return std::string(STR_MIXSOURCERAW);
          case FUNC_ADJUST_GVAR_GVAR:
            return std::string(STR_GLOBALVAR);
          case FUNC_ADJUST_GVAR_INCDEC:
            return std::string(STR_INCDEC);
        }
        return std::string("---");
      });

      switch (CFN_GVAR_MODE(cfn)) {
        case FUNC_ADJUST_GVAR_CONSTANT: {
          int16_t val_min, val_max;
          getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min,
                         val_max);
          addNumberEdit(line, STR_CONSTANT, cfn, val_min, val_max);
          break;
        }
        case FUNC_ADJUST_GVAR_SOURCE:
        case FUNC_ADJUST_GVAR_SOURCERAW:
          addSourceChoice(line, STR_MIXSOURCE, cfn, MIXSRC_LAST_CH);
          break;
        case FUNC_ADJUST_GVAR_GVAR: {
          new StaticText(line, rect_t{}, STR_GLOBALVAR);
          auto gvarchoice = new Choice(line, rect_t{}, 0, MAX_GVARS - 1,
                                       GET_SET_DEFAULT(CFN_PARAM(cfn)));
          gvarchoice->setTextHandler([](int32_t value) {
            return std::string(getSourceString(value + MIXSRC_FIRST_GVAR));
          });
          gvarchoice->setAvailableHandler(
              [=](int value) { return CFN_GVAR_INDEX(cfn) != value; });
          break;
        }
        case FUNC_ADJUST_GVAR_INCDEC: {
          int16_t val_min, val_max;
          getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min,
                         val_max);
          getGVarIncDecRange(val_min, val_max);
          auto numedit = addNumberEdit(line, STR_INCDEC, cfn, val_min, val_max);
          numedit->setDisplayHandler([](int value) {
            return formatNumberAsString(abs(value), 0, 0,
                                        value >= 0 ? "+=" : "-=", nullptr);
          });
          break;
        }
      }
    }
  }

  if (HAS_REPEAT_PARAM(func)) {  // !1x 1x 1s 2s 3s ...
    line = specialFunctionOneWindow->newLine(grid);
    new StaticText(line, rect_t{}, STR_REPEAT);
    if (func == FUNC_PLAY_SCRIPT || func == FUNC_RGB_LED) {
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
          return formatNumberAsString(value * CFN_PLAY_REPEAT_MUL, 0, 0,
                                      nullptr, "s");
        }
      });
    }
  }

  line = specialFunctionOneWindow->newLine(grid);
  new StaticText(line, rect_t{}, STR_ENABLE);
  new ToggleSwitch(line, rect_t{}, GET_DEFAULT(CFN_ACTIVE(cfn)),
            [=](int newValue) {
              CFN_ACTIVE(cfn) = newValue;
              SET_DIRTY();
              if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT || CFN_FUNC(cfn) == FUNC_RGB_LED)
                LUA_LOAD_MODEL_SCRIPTS();
            });
}

void FunctionEditPage::buildBody(Window *form)
{
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_LARGE);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  CustomFunctionData *cfn = customFunctionData();

  // Switch
  auto line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_SF_SWITCH);
  auto switchChoice = new SwitchChoice(line, rect_t{}, SWSRC_FIRST, SWSRC_LAST,
                                       GET_SET_DEFAULT(CFN_SWITCH(cfn)));
  switchChoice->setAvailableHandler(
      [=](int value) { return isSwitchAvailable(value); });

  // Patch function in case not available
  if (!isAssignableFunctionAvailable(CFN_FUNC(cfn))) {
    auto func = 0;
    while (!isAssignableFunctionAvailable(func) && (func < FUNC_MAX - 1)) {
      func++;
    }
    if (func < FUNC_MAX - 1) {
      CFN_FUNC(cfn) = func;
    }
  }

  // Function
  line = form->newLine(grid);
  new StaticText(line, rect_t{}, STR_FUNC);
  auto functionChoice =
      new Choice(line, rect_t{}, 0, FUNC_MAX - 1, GET_DEFAULT(getFuncSortIdx(CFN_FUNC(cfn))),
                  [=](int32_t newValue) {
                    Functions newFunc = cfn_sorted[newValue];
                    // If changing from Lua script then reload to remove old reference
                    if ((CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT || CFN_FUNC(cfn) == FUNC_RGB_LED) && newFunc != FUNC_PLAY_SCRIPT && newFunc != FUNC_RGB_LED)
                      LUA_LOAD_MODEL_SCRIPTS();
                    CFN_FUNC(cfn) = newFunc;
                    CFN_RESET(cfn);
                    SET_DIRTY();
                    updateSpecialFunctionOneWindow();
                  });
  functionChoice->setTextHandler([=](int val) { return funcGetLabel(cfn_sorted[val]); });
  functionChoice->setAvailableHandler(
      [=](int value) { return isAssignableFunctionAvailable(cfn_sorted[value]); });

  specialFunctionOneWindow = new Window(form, rect_t{});
  updateSpecialFunctionOneWindow();
}

//-----------------------------------------------------------------------------

FunctionsPage::FunctionsPage(CustomFunctionData *functions, const char *title,
                             const char *prefix, EdgeTxIcon icon) :
    PageTab(title, icon), functions(functions), title(title), prefix(prefix)
{
}

void FunctionsPage::rebuild(Window *window)
{
  // When window.clear() is called the last button on screen is given focus
  // (???) This causes the page to jump to the end when rebuilt. Set flag to
  // bypass the button focus handler and reset focusIndex when rebuilding
  isRebuilding = true;
  window->clear();
  build(window);
  isRebuilding = false;
}

void FunctionsPage::newSF(Window *window, bool pasteSF)
{
  Menu *menu = new Menu();
  menu->setTitle(title);

  // search for unused switches
  for (uint8_t i = 0; i < MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData *cfn = customFunctionData(i);
    if (cfn->swtch == 0) {
      menu->addLineBuffered(prefix + std::to_string(i + 1), [=]() {
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

void FunctionsPage::pasteSpecialFunction(Window *window, uint8_t index,
                                         ButtonBase *button)
{
  CustomFunctionData *cfn = customFunctionData(index);
  if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT) LUA_LOAD_MODEL_SCRIPTS();
  *cfn = clipboard.data.cfn;
  if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT) LUA_LOAD_MODEL_SCRIPTS();
  storageDirty(EE_MODEL);
  focusIndex = index;
  if (!button)
    rebuild(window);
}

void FunctionsPage::editSpecialFunction(Window *window, uint8_t index,
                                        ButtonBase *button)
{
  auto edit = editPage(index);
  edit->setCloseHandler([=]() {
    CustomFunctionData *cfn = customFunctionData(index);
    if (cfn->swtch != 0) {
      focusIndex = index;
      if (!button)
        rebuild(window);
    }
  });
}

void FunctionsPage::plusPopup(Window *window)
{
  if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION) {
    Menu *menu = new Menu();
    menu->addLine(STR_NEW, [=]() { newSF(window, false); });
    menu->addLine(STR_PASTE, [=]() { newSF(window, true); });
  } else {
    newSF(window, false);
  }
}

void FunctionsPage::build(Window *window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  bool hasEmptyFunction = false;

  // Reset focusIndex after switching tabs
  if (!isRebuilding) focusIndex = prevFocusIndex;

  for (uint8_t i = 0; i < MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData *cfn = customFunctionData(i);

    bool isActive = (cfn->swtch != 0);

    if (isActive) {
      auto button = functionButton(
          window, rect_t{0, 0, window->width() - 12, SF_BUTTON_H}, i);

      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 1,
                           LV_GRID_ALIGN_CENTER, 0, 1);

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
        Menu *menu = new Menu();
        menu->addLine(STR_EDIT,
                      [=]() { editSpecialFunction(window, i, button); });
        if (isActive) {
          menu->addLine(STR_COPY, [=]() {
            clipboard.type = CLIPBOARD_TYPE_CUSTOM_FUNCTION;
            clipboard.data.cfn = *cfn;
          });
        }
        if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION) {
          menu->addLine(STR_PASTE,
                        [=]() { pasteSpecialFunction(window, i, button); });
        }
        CustomFunctionData *cfn = customFunctionData(i);
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
            if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT || CFN_FUNC(cfn) == FUNC_RGB_LED)
              LUA_LOAD_MODEL_SCRIPTS();
            rebuild(window);
          });
        }
        if (customFunctionData(MAX_SPECIAL_FUNCTIONS - 1)->isEmpty()) {
          for (int j = i; j < MAX_SPECIAL_FUNCTIONS; j++) {
            if (!customFunctionData(j)->isEmpty()) {
              menu->addLine(STR_INSERT, [=]() {
                memmove(cfn + 1, cfn,
                        (MAX_SPECIAL_FUNCTIONS - i - 1) *
                            sizeof(CustomFunctionData));
                memset(cfn, 0, sizeof(CustomFunctionData));
                editSpecialFunction(window, i, nullptr);
              });
              break;
            }
          }
        }
        if (isActive) {
          menu->addLine(STR_CLEAR, [=]() {
            if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT) LUA_LOAD_MODEL_SCRIPTS();
            memset(cfn, 0, sizeof(CustomFunctionData));
            SET_DIRTY();
            rebuild(window);
          });
        }
        for (int j = i; j < MAX_SPECIAL_FUNCTIONS; j++) {
          if (!customFunctionData(j)->isEmpty()) {
            menu->addLine(STR_DELETE, [=]() {
              if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT) LUA_LOAD_MODEL_SCRIPTS();
              memmove(
                  cfn, cfn + 1,
                  (MAX_SPECIAL_FUNCTIONS - i - 1) * sizeof(CustomFunctionData));
              memset(customFunctionData(MAX_SPECIAL_FUNCTIONS - 1), 0,
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

  if (hasEmptyFunction) {
    addButton =
        new TextButton(window, rect_t{0, 0, window->width() - 12, SF_BUTTON_H},
                       LV_SYMBOL_PLUS, [=]() {
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
  } else {
    addButton = nullptr;
  }
}

//-----------------------------------------------------------------------------

class SpecialFunctionLineButton : public FunctionLineButton
{
 public:
  SpecialFunctionLineButton(Window *parent, const rect_t &rect, uint8_t index) :
      FunctionLineButton(parent, rect, &g_model.customFn[index], index, "SF")
  {
  }

 protected:
  bool isActive() const override
  {
    return modelFunctionsContext.activeSwitches & ((MASK_CFN_TYPE)1 << index);
  }
};

//-----------------------------------------------------------------------------

class SpecialFunctionEditPage : public FunctionEditPage
{
 public:
  SpecialFunctionEditPage(uint8_t index) :
      FunctionEditPage(index, ICON_MODEL_SPECIAL_FUNCTIONS, STR_MENUCUSTOMFUNC,
                       "SF")
  {
  }

 protected:
  bool isActive() const override
  {
    return modelFunctionsContext.activeSwitches & ((MASK_CFN_TYPE)1 << index);
  }

  bool isSwitchAvailable(int value) const override
  {
    return ::isSwitchAvailable(value, ModelCustomFunctionsContext);
  }

  CustomFunctionData *customFunctionData() const override
  {
    return &g_model.customFn[index];
  }

  bool isAssignableFunctionAvailable(int function) const override
  {
    return ::isAssignableFunctionAvailable(function, true);
  }

  void setDirty() const override { storageDirty(EE_MODEL); }
};

//-----------------------------------------------------------------------------

SpecialFunctionsPage::SpecialFunctionsPage() :
    FunctionsPage(g_model.customFn, STR_MENUCUSTOMFUNC, "SF",
                  ICON_MODEL_SPECIAL_FUNCTIONS)
{
}

CustomFunctionData *SpecialFunctionsPage::customFunctionData(
    uint8_t index) const
{
  return &g_model.customFn[index];
}

FunctionEditPage *SpecialFunctionsPage::editPage(uint8_t index) const
{
  return new SpecialFunctionEditPage(index);
}

FunctionLineButton *SpecialFunctionsPage::functionButton(Window *parent,
                                                         const rect_t &rect,
                                                         uint8_t index) const
{
  return new SpecialFunctionLineButton(parent, rect, index);
}

void SpecialFunctionsPage::setDirty() const { storageDirty(EE_MODEL); }

//-----------------------------------------------------------------------------

class GlobalFunctionLineButton : public FunctionLineButton
{
 public:
  GlobalFunctionLineButton(Window *parent, const rect_t &rect, uint8_t index) :
      FunctionLineButton(parent, rect, &g_eeGeneral.customFn[index], index,
                         "GF")
  {
  }

 protected:
  bool isActive() const override
  {
    return globalFunctionsContext.activeSwitches & ((MASK_CFN_TYPE)1 << index);
  }
};

//-----------------------------------------------------------------------------

class GlobalFunctionEditPage : public FunctionEditPage
{
 public:
  GlobalFunctionEditPage(uint8_t index) :
      FunctionEditPage(index, ICON_RADIO_GLOBAL_FUNCTIONS, STR_MENUSPECIALFUNCS,
                       "GF")
  {
  }

 protected:
  bool isActive() const override
  {
    return globalFunctionsContext.activeSwitches & ((MASK_CFN_TYPE)1 << index);
  }

  bool isSwitchAvailable(int value) const override
  {
    return ::isSwitchAvailable(value, GeneralCustomFunctionsContext);
  }

  CustomFunctionData *customFunctionData() const override
  {
    return &g_eeGeneral.customFn[index];
  }

  bool isAssignableFunctionAvailable(int function) const override
  {
    return ::isAssignableFunctionAvailable(function, false);
  }

  void setDirty() const override { storageDirty(EE_GENERAL); }
};

//-----------------------------------------------------------------------------

GlobalFunctionsPage::GlobalFunctionsPage() :
    FunctionsPage(g_eeGeneral.customFn, STR_MENUSPECIALFUNCS, "GF",
                  ICON_RADIO_GLOBAL_FUNCTIONS)
{
}

CustomFunctionData *GlobalFunctionsPage::customFunctionData(uint8_t index) const
{
  return &g_eeGeneral.customFn[index];
}

FunctionEditPage *GlobalFunctionsPage::editPage(uint8_t index) const
{
  return new GlobalFunctionEditPage(index);
}

FunctionLineButton *GlobalFunctionsPage::functionButton(Window *parent,
                                                        const rect_t &rect,
                                                        uint8_t index) const
{
  return new GlobalFunctionLineButton(parent, rect, index);
}

void GlobalFunctionsPage::setDirty() const { storageDirty(EE_GENERAL); }
