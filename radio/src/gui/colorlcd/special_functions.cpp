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

#define SET_DIRTY()     storageDirty(functions == g_model.customFn ? EE_MODEL : EE_GENERAL)

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
      invalidate();
      headerSF->setTextFlags(isActive() ? FONT(BOLD) | COLOR_THEME_ACTIVE
                                        : COLOR_THEME_PRIMARY2);
      active = !active;
    }
  }

  void buildHeader(Window *window)
  {
    new StaticText(
        window, {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT, 20},
        functions == g_model.customFn ? STR_MENUCUSTOMFUNC
                                      : STR_MENUSPECIALFUNCS,
        0, COLOR_THEME_PRIMARY2);
    headerSF = new StaticText(
        window,
        {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
         LCD_W - PAGE_TITLE_LEFT, 20},
        (functions == g_model.customFn ? "SF" : "GF") + std::to_string(index+1),
        0, COLOR_THEME_PRIMARY2);
  }

  void updateSpecialFunctionOneWindow()
  {
    FormGridLayout grid;
    specialFunctionOneWindow->clear();

    CustomFunctionData *cfn = &functions[index];
    uint8_t func = CFN_FUNC(cfn);

    // Func param
    switch (func) {
      case FUNC_OVERRIDE_CHANNEL: {
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_CH, 0, COLOR_THEME_PRIMARY1);
        new NumberEdit(specialFunctionOneWindow, grid.getFieldSlot(), 1,
                       MAX_OUTPUT_CHANNELS,
                       GET_SET_VALUE_WITH_OFFSET(CFN_CH_INDEX(cfn), 1));
        grid.nextLine();

        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        int limit = (g_model.extendedLimits ? LIMIT_EXT_PERCENT : LIMIT_STD_PERCENT);
        new NumberEdit(specialFunctionOneWindow, grid.getFieldSlot(), -limit, limit,
                       GET_SET_DEFAULT(CFN_PARAM(cfn)));
        grid.nextLine();
        break;
    }

      case FUNC_TRAINER: {
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        auto choice =
            new Choice(specialFunctionOneWindow, grid.getFieldSlot(), 0,
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
        grid.nextLine();
        break;
      }

      case FUNC_RESET:
        if (CFN_PARAM(cfn) < FUNC_RESET_PARAM_FIRST_TELEM) {
          new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_RESET, 0, COLOR_THEME_PRIMARY1);
          auto choice = new Choice(
              specialFunctionOneWindow, grid.getFieldSlot(), 0,
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
          grid.nextLine();
        }
        break;

      case FUNC_VOLUME:
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VOLUME, 0, COLOR_THEME_PRIMARY1);
        new SourceChoice(specialFunctionOneWindow, grid.getFieldSlot(), 0,
                         MIXSRC_LAST_CH, GET_SET_DEFAULT(CFN_PARAM(cfn)));
        grid.nextLine();
        break;

      case FUNC_PLAY_SOUND:
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new Choice(specialFunctionOneWindow, grid.getFieldSlot(),
                   STR_FUNCSOUNDS, 0,
                   AU_SPECIAL_SOUND_LAST - AU_SPECIAL_SOUND_FIRST - 1,
                   GET_SET_DEFAULT(CFN_PARAM(cfn)));
        grid.nextLine();
        break;

      case FUNC_PLAY_TRACK:
      case FUNC_BACKGND_MUSIC:
      case FUNC_PLAY_SCRIPT:
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new FileChoice(
            specialFunctionOneWindow, grid.getFieldSlot(),
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
        grid.nextLine();
        break;

      case FUNC_SET_TIMER: {
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_TIMER, 0, COLOR_THEME_PRIMARY1);
        auto timerchoice =
            new Choice(specialFunctionOneWindow, grid.getFieldSlot(), 0,
                       TIMERS - 1, GET_SET_DEFAULT(CFN_TIMER_INDEX(cfn)));
        timerchoice->setTextHandler([](int32_t value) {
          return std::string(STR_TIMER) + std::to_string(value + 1);
        });
        grid.nextLine();

        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new TimeEdit(specialFunctionOneWindow, grid.getFieldSlot(), 0,
                     9 * 60 * 60 - 1, GET_SET_DEFAULT(CFN_PARAM(cfn)));
        grid.nextLine();
        break;
      }

      case FUNC_SET_FAILSAFE:
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_MODULE, 0, COLOR_THEME_PRIMARY1);
        new Choice(specialFunctionOneWindow, grid.getFieldSlot(),
                   "\004Int.Ext.", 0, NUM_MODULES - 1,
                   GET_SET_DEFAULT(CFN_PARAM(cfn)));
        grid.nextLine();
        break;

      case FUNC_PLAY_VALUE:
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new SourceChoice(specialFunctionOneWindow, grid.getFieldSlot(), 0,
                         MIXSRC_LAST_TELEM, GET_SET_DEFAULT(CFN_PARAM(cfn)));
        grid.nextLine();
        break;

      case FUNC_HAPTIC:
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        new NumberEdit(specialFunctionOneWindow, grid.getFieldSlot(), 0, 3,
                       GET_SET_DEFAULT(CFN_PARAM(cfn)));
        grid.nextLine();
        break;

      case FUNC_LOGS: {
        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_VALUE, 0, COLOR_THEME_PRIMARY1);
        auto edit =
            new NumberEdit(specialFunctionOneWindow, grid.getFieldSlot(), 0,
                           255, GET_SET_DEFAULT(CFN_PARAM(cfn)));
        edit->setDisplayHandler(
            [=](int32_t value) {
              return formatNumberAsString(CFN_PARAM(cfn), PREC1, sizeof(CFN_PARAM(cfn)), nullptr, "s");
            });
        break;
      }

      case FUNC_ADJUST_GVAR: {

        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_GLOBALVAR, 0, COLOR_THEME_PRIMARY1);
        auto gvarchoice =
            new Choice(specialFunctionOneWindow, grid.getFieldSlot(), 0,
                       MAX_GVARS - 1, GET_SET_DEFAULT(CFN_GVAR_INDEX(cfn)));
        gvarchoice->setTextHandler([](int32_t value) {
          return std::string(STR_GV) + std::to_string(value + 1);
        });
        grid.nextLine();

        new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_MODE, 0, COLOR_THEME_PRIMARY1);
        auto modechoice =
          new Choice(specialFunctionOneWindow, grid.getFieldSlot(),
                     FUNC_ADJUST_GVAR_CONSTANT, FUNC_ADJUST_GVAR_INCDEC,
                     GET_DEFAULT(CFN_GVAR_MODE(cfn)), nullptr);
        grid.nextLine();

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
            new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_CONSTANT, 0, COLOR_THEME_PRIMARY1);
            int16_t val_min, val_max;
            getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min,
                           val_max);
            new NumberEdit(specialFunctionOneWindow, grid.getFieldSlot(),
                           val_min, val_max, GET_SET_DEFAULT(CFN_PARAM(cfn)));
            break;
          }
          case FUNC_ADJUST_GVAR_SOURCE:
            new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_MIXSOURCE, 0, COLOR_THEME_PRIMARY1);
            new SourceChoice(specialFunctionOneWindow, grid.getFieldSlot(),
                             0, MIXSRC_LAST_CH, GET_SET_DEFAULT(CFN_PARAM(cfn)));
            break;
          case FUNC_ADJUST_GVAR_GVAR: {
            new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_GLOBALVAR, 0, COLOR_THEME_PRIMARY1);
            auto gvarchoice =
                new Choice(specialFunctionOneWindow, grid.getFieldSlot(), 0,
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
            new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_INCDEC, 0, COLOR_THEME_PRIMARY1);
            int16_t val_min, val_max;
            getMixSrcRange(CFN_GVAR_INDEX(cfn) + MIXSRC_FIRST_GVAR, val_min, val_max);
            getGVarIncDecRange(val_min, val_max);
            auto numedit = new NumberEdit(specialFunctionOneWindow,
                                          grid.getFieldSlot(), val_min, val_max,
                                          GET_SET_DEFAULT(CFN_PARAM(cfn)));
            numedit->setDisplayHandler(
                [](int value) {
                  return formatNumberAsString(abs(value), 0, 0, value >= 0 ? "+=" : "--", nullptr);
                });
            break;
          }
        }
        grid.nextLine();
      }
    }

    if (HAS_ENABLE_PARAM(func)) {
      new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_ENABLE, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(specialFunctionOneWindow, grid.getFieldSlot(),
                   GET_SET_DEFAULT(CFN_ACTIVE(cfn)));
      grid.nextLine();
    } else if (HAS_REPEAT_PARAM(func)) {  // !1x 1x 1s 2s 3s ...
      new StaticText(specialFunctionOneWindow, grid.getLabelSlot(), STR_REPEAT,
                     0, COLOR_THEME_PRIMARY1);
      auto repeat = new NumberEdit(
          specialFunctionOneWindow, grid.getFieldSlot(2, 1), -1,
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
      grid.nextLine();
    }

    specialFunctionOneWindow->adjustHeight();
  }

  void buildBody(FormWindow *window)
  {
    // SF.one
    FormGridLayout grid;
    grid.spacer(PAGE_PADDING);

    CustomFunctionData *cfn = &functions[index];

    // Switch
    new StaticText(window, grid.getLabelSlot(), STR_SWITCH, 0, COLOR_THEME_PRIMARY1);
    auto switchChoice =
        new SwitchChoice(window, grid.getFieldSlot(), SWSRC_FIRST, SWSRC_LAST,
                         GET_SET_DEFAULT(CFN_SWITCH(cfn)));
    switchChoice->setAvailableHandler([=](int value) {
      return (functions == g_model.customFn
                  ? isSwitchAvailable(value, ModelCustomFunctionsContext)
                  : isSwitchAvailable(value, GeneralCustomFunctionsContext));
    });
    grid.nextLine();

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
    new StaticText(window, grid.getLabelSlot(), STR_FUNC, 0, COLOR_THEME_PRIMARY1);
    auto functionChoice =
        new Choice(window, grid.getFieldSlot(), STR_VFSWFUNC,
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
    grid.nextLine();

    specialFunctionOneWindow = new FormGroup(
        window, {0, grid.getWindowHeight(), LCD_W, 0}// , FORM_FORWARD_FOCUS
                                             );
    updateSpecialFunctionOneWindow();
    grid.addWindow(specialFunctionOneWindow);
  }
};

static constexpr coord_t line1 = FIELD_PADDING_TOP;
static constexpr coord_t line2 = line1 + PAGE_LINE_HEIGHT;
static constexpr coord_t col1 = 20;
static constexpr coord_t col2 = (LCD_W - 100) / 3 + col1;
static constexpr coord_t col3 = ((LCD_W - 100) / 3) * 2 + col1 + 20;

static const char* _failsafe_module[] = {
  "Ext.", "Int.",
};

class SpecialFunctionButton : public Button
{
 public:
  SpecialFunctionButton(FormWindow *parent, const rect_t &rect,
                        CustomFunctionData *functions, uint8_t index) :
      Button(parent, rect), functions(functions), index(index)
  {
    const CustomFunctionData *cfn = &functions[index];
    uint8_t func = CFN_FUNC(cfn);
    if (!cfn->isEmpty() &&
        (HAS_ENABLE_PARAM(func) || HAS_REPEAT_PARAM(func) ||
         (func == FUNC_PLAY_TRACK || func == FUNC_BACKGND_MUSIC ||
          func == FUNC_PLAY_SCRIPT))) {
      setHeight(line2 + PAGE_LINE_HEIGHT);
    }
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

      case FUNC_PLAY_VALUE:
        drawSource(dc, col1, line2, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        break;

      case FUNC_HAPTIC:
        dc->drawNumber(col1, line2, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1);
        break;

      case FUNC_LOGS:
        dc->drawNumber(col3, line1, CFN_PARAM(cfn), COLOR_THEME_SECONDARY1 | PREC1, sizeof(CFN_PARAM(cfn)), nullptr, "s");
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
    if (active)
      dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR_THEME_ACTIVE);
    else
      dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR_THEME_PRIMARY2);

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

SpecialFunctionsPage::SpecialFunctionsPage(CustomFunctionData *functions) :
    PageTab(functions == g_model.customFn ? STR_MENUCUSTOMFUNC
                                          : STR_MENUSPECIALFUNCS,
            functions == g_model.customFn ? ICON_MODEL_SPECIAL_FUNCTIONS
                                          : ICON_RADIO_GLOBAL_FUNCTIONS),
    functions(functions)
{
}

void SpecialFunctionsPage::rebuild(FormWindow *window,
                                   int8_t focusSpecialFunctionIndex)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());  
  window->clear();
  build(window, focusSpecialFunctionIndex);
  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void SpecialFunctionsPage::editSpecialFunction(FormWindow *window,
                                               uint8_t index)
{
  auto editPage = new SpecialFunctionEditPage(functions, index);
  editPage->setCloseHandler([=]() { rebuild(window, index); });
}

void SpecialFunctionsPage::build(FormWindow *window, int8_t focusIndex)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(66);
  window->padAll(0);

  // Window::clearFocus();

  char s[] = "SFxx";
  if (functions == g_eeGeneral.customFn) s[0] = 'G';

  for (uint8_t i = 0; i < MAX_SPECIAL_FUNCTIONS; i++) {
    CustomFunctionData *cfn = &functions[i];
    strAppendUnsigned(&s[2], i+1);

    Button *button;
    if (cfn->swtch == 0) {
      button = new TextButton(window, grid.getLabelSlot(), s);
      button->setPressHandler([=]() {
        if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION) {
          Menu *menu = new Menu(window);
          menu->addLine(STR_EDIT, [=]() { editSpecialFunction(window, i); });
          menu->addLine(STR_PASTE, [=]() {
            *cfn = clipboard.data.cfn;
            SET_DIRTY();
            if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
              LUA_LOAD_MODEL_SCRIPTS();
            rebuild(window, i);
          });
        } else {
          editSpecialFunction(window, i);
        }
        return 0;
      });
      grid.spacer(button->height() + 5);
    } else {
      auto txt = new StaticText(window, grid.getLabelSlot(), s, BUTTON_BACKGROUND, COLOR_THEME_PRIMARY1 | CENTERED);

      button = new SpecialFunctionButton(window, grid.getFieldSlot(), functions, i);
      button->setPressHandler([=]() {
        button->bringToTop();
        Menu *menu = new Menu(window);
        menu->addLine(STR_EDIT, [=]() { editSpecialFunction(window, i); });
        menu->addLine(STR_COPY, [=]() {
          clipboard.type = CLIPBOARD_TYPE_CUSTOM_FUNCTION;
          clipboard.data.cfn = *cfn;
        });
        if (clipboard.type == CLIPBOARD_TYPE_CUSTOM_FUNCTION) {
          menu->addLine(STR_PASTE, [=]() {
            if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
              LUA_LOAD_MODEL_SCRIPTS();
            *cfn = clipboard.data.cfn;
            if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
              LUA_LOAD_MODEL_SCRIPTS();
            SET_DIRTY();
            rebuild(window, i);
          });
        }
        if (functions[MAX_SPECIAL_FUNCTIONS - 1].isEmpty()) {
          menu->addLine(STR_INSERT, [=]() {
            memmove(
                cfn + 1, cfn,
                (MAX_SPECIAL_FUNCTIONS - i - 1) * sizeof(CustomFunctionData));
            memset(cfn, 0, sizeof(CustomFunctionData));
            SET_DIRTY();
            rebuild(window, i);
          });
        }
        menu->addLine(STR_CLEAR, [=]() {
          if (CFN_FUNC(cfn) == FUNC_PLAY_SCRIPT)
            LUA_LOAD_MODEL_SCRIPTS();
          memset(cfn, 0, sizeof(CustomFunctionData));
          SET_DIRTY();
          rebuild(window, i);
        });
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
              rebuild(window, i);
            });
            break;
          }
        }
        return 0;
      });
      button->setFocusHandler([=](bool focus) {
        if (focus) {
          txt->setBackgroundColor(COLOR_THEME_FOCUS);
          txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED);
        } else {
          txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
          txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED);
        }
        txt->invalidate();
      });

      // if (focusIndex == i) {
      //   txt->setBackgroundColor(COLOR_THEME_FOCUS);
      //   txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED);
      //   txt->invalidate();
      // }

      txt->setHeight(button->height());
      grid.spacer(button->height() + 5);
    }

    // if (focusIndex == i) {  // fix focus #303
    //   button->setFocus(SET_FOCUS_DEFAULT);
    // }
  }

  grid.nextLine();

  //  window->setLastField();
}
