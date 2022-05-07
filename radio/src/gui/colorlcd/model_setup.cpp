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

#include "model_setup.h"

#include "opentx.h"
#include "libopenui.h"
#include "storage/modelslist.h"
#include "trainer_setup.h"
#include "module_setup.h"
#include "timer_setup.h"

#include <algorithm>

#define SET_DIRTY()     storageDirty(EE_MODEL)

std::string switchWarninglabel(swsrc_t index)
{
  static const char *switchPositions[] = {
      " ",
      STR_CHAR_UP,
      "-",
      STR_CHAR_DOWN,
  };

  return TEXT_AT_INDEX(STR_VSRCRAW,
                       (index + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1)) +
         std::string(
             switchPositions[g_model.switchWarningState >> (3 * index) & 0x07]);
}

ModelSetupPage::ModelSetupPage() :
  PageTab(STR_MENU_MODEL_SETUP, ICON_MODEL_SETUP)
{
}

const char MODEL_NAME_EXTRA_CHARS[] = "_-.,:;<=>";

static void onModelNameChanged()
{
  modelslist.load();
  auto model = modelslist.getCurrentModel();
  if (model) {
    model->setModelName(g_model.header.name);
    modelslist.save();
  }
  SET_DIRTY();
}

struct ModelNameEdit : public ModelTextEdit {
  ModelNameEdit(Window *parent, const rect_t &rect) :
      ModelTextEdit(parent, rect, g_model.header.name,
                    sizeof(g_model.header.name), 0, MODEL_NAME_EXTRA_CHARS)
  {
    setChangeHandler(onModelNameChanged);
  }
};

static std::string getModelBitmap()
{
  return std::string(g_model.header.bitmap, sizeof(g_model.header.bitmap));
}

static void setModelBitmap(std::string newValue)
{
  strncpy(g_model.header.bitmap, newValue.c_str(),
          sizeof(g_model.header.bitmap));
  SET_DIRTY();
}

struct ModelBitmapEdit : public FileChoice {
  ModelBitmapEdit(FormGroup *parent, const rect_t &rect) :
      FileChoice(parent, rect, BITMAPS_PATH, BITMAPS_EXT,
                 sizeof(g_model.header.bitmap), getModelBitmap, setModelBitmap)
  {
  }
};

void ModelSetupPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);

  // Model name
  new StaticText(window, grid.getLabelSlot(), STR_MODELNAME, 0, COLOR_THEME_PRIMARY1);
  new ModelNameEdit(window, grid.getFieldSlot());
  grid.nextLine();

  // Bitmap
  new StaticText(window, grid.getLabelSlot(), STR_BITMAP, 0, COLOR_THEME_PRIMARY1);
  new ModelBitmapEdit(window, grid.getFieldSlot());
  grid.nextLine();

  for (uint8_t i = 0; i < TIMERS; i++)
    new TimerButton(window, grid.getSlot(3, i), i);
  grid.nextLine();

  // Extended limits
  new StaticText(window, grid.getLabelSlot(), STR_ELIMITS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.extendedLimits));
  grid.nextLine();

  // Extended trims
  new StaticText(window, grid.getLabelSlot(), STR_ETRIMS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(window, grid.getFieldSlot(2, 0), GET_SET_DEFAULT(g_model.extendedTrims));
  new TextButton(window, grid.getFieldSlot(2, 1), STR_RESET_BTN,
                 []() -> uint8_t {
                   for (auto &flightModeData : g_model.flightModeData) {
                     memclear(&flightModeData, TRIMS_ARRAY_SIZE);
                   }
                   SET_DIRTY();
                   AUDIO_WARNING1();
                   return 0;
                 });
  grid.nextLine();

  // Display trims
  new StaticText(window, grid.getLabelSlot(), STR_DISPLAY_TRIMS, 0, COLOR_THEME_PRIMARY1);
  new Choice(window, grid.getFieldSlot(), "\006No\0   ChangeYes", 0, 2, GET_SET_DEFAULT(g_model.displayTrims));
  grid.nextLine();

  // Trim step
  new StaticText(window, grid.getLabelSlot(), STR_TRIMINC, 0, COLOR_THEME_PRIMARY1);
  new Choice(window, grid.getFieldSlot(), STR_VTRIMINC, -2, 2, GET_SET_DEFAULT(g_model.trimInc));
  grid.nextLine();

  // Throttle parameters
  {
    new Subtitle(window, grid.getLineSlot(), STR_THROTTLE_LABEL, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();

    // Throttle reversed
    new StaticText(window, grid.getLabelSlot(true), STR_THROTTLEREVERSE, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.throttleReversed));
    grid.nextLine();

    // Throttle source
    new StaticText(window, grid.getLabelSlot(true), STR_TTRACE, 0, COLOR_THEME_PRIMARY1);
    auto sc = new SourceChoice(
        window, grid.getFieldSlot(), 0, MIXSRC_LAST_CH,
        [=]() { return throttleSource2Source(g_model.thrTraceSrc); },
        [=](int16_t src) {
          int16_t val = source2ThrottleSource(src);
          if (val >= 0) {
            g_model.thrTraceSrc = val;
            SET_DIRTY();
          }
        }, 0, COLOR_THEME_PRIMARY1);
    sc->setAvailableHandler(isThrottleSourceAvailable);
    grid.nextLine();

    // Throttle trim
    new StaticText(window, grid.getLabelSlot(true), STR_TTRIM, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.thrTrim));
    grid.nextLine();

    // Throttle trim source
    new StaticText(window, grid.getLabelSlot(true), STR_TTRIM_SW, 0, COLOR_THEME_PRIMARY1);
    new SourceChoice(
        window, grid.getFieldSlot(), MIXSRC_FIRST_TRIM, MIXSRC_LAST_TRIM,
        [=]() { return g_model.getThrottleStickTrimSource(); },
        [=](int16_t src) { g_model.setThrottleStickTrimSource(src); }, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
  }

  // Preflight parameters
  {
    new Subtitle(window, grid.getLineSlot(), STR_PREFLIGHT, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();

    // Display checklist
    new StaticText(window, grid.getLabelSlot(true), STR_CHECKLIST, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.displayChecklist));
    grid.nextLine();

    // Throttle warning
    new StaticText(window, grid.getLabelSlot(true), STR_THROTTLE_WARNING, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_model.disableThrottleWarning));
    grid.nextLine();

    // Custom Throttle warning
    new StaticText(window, grid.getLabelSlot(true), STR_CUSTOM_THROTTLE_WARNING, 0, COLOR_THEME_PRIMARY1);
    new CheckBox(window, grid.getFieldSlot(), GET_SET_DEFAULT(g_model.enableCustomThrottleWarning));
    grid.nextLine();
    // Custom Throttle warning value
    new StaticText(window, grid.getLabelSlot(true), STR_CUSTOM_THROTTLE_WARNING_VAL, 0, COLOR_THEME_PRIMARY1);
    new NumberEdit(window, grid.getFieldSlot(), -100, 100, GET_SET_DEFAULT(g_model.customThrottleWarningPosition));
    grid.nextLine();

    // Switches warning
    new StaticText(window, grid.getLabelSlot(true), STR_SWITCHWARNING, 0, COLOR_THEME_PRIMARY1);
    auto group = new FormGroup(window, grid.getFieldSlot(), FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
    GridLayout switchesGrid(group);
    for (int i = 0, j = 0; i < NUM_SWITCHES; i++) {
      if (SWITCH_EXISTS(i)) {
        if (j > 0 && (j % 3) == 0)
          switchesGrid.nextLine();
        auto button = new TextButton(group, switchesGrid.getSlot(3, j % 3), switchWarninglabel(i), nullptr,
                                     OPAQUE | (bfGet(g_model.switchWarningState, 3 * i, 3) == 0 ? 0 : BUTTON_CHECKED));
        button->setPressHandler([button, i] {
            swarnstate_t newstate = bfGet(g_model.switchWarningState, 3 * i, 3);
            if (newstate == 1 && SWITCH_CONFIG(i) != SWITCH_3POS)
              newstate = 3;
            else
              newstate = (newstate + 1) % 4;
            g_model.switchWarningState = bfSet(g_model.switchWarningState, newstate, 3 * i, 3);
            SET_DIRTY();
            button->setText(switchWarninglabel(i));
            return newstate > 0;
        });
        j++;
      }
    }
    grid.addWindow(group);

    // Pots and sliders warning
#if NUM_POTS + NUM_SLIDERS
    {
      new StaticText(window, grid.getLabelSlot(true), STR_POTWARNINGSTATE, 0, COLOR_THEME_PRIMARY1);
      new Choice(window, grid.getFieldSlot(), {"OFF", "ON", "AUTO"}, 0, 2,
                 GET_SET_DEFAULT(g_model.potsWarnMode));
      grid.nextLine();

#if (NUM_POTS)
      {
        new StaticText(window, grid.getLabelSlot(true), STR_POTWARNING, 0, COLOR_THEME_PRIMARY1);
        auto group =
            new FormGroup(window, grid.getFieldSlot(),
                          FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
        GridLayout centerGrid(group);
        for (int i = POT_FIRST, j = 0, k = 0; i <= POT_LAST; i++, k++) {
          char s[8];
          if ((IS_POT(i) || IS_POT_MULTIPOS(i)) && IS_POT_AVAILABLE(i)) {
            if (j > 0 && ((j % 4) == 0)) centerGrid.nextLine();

            auto button = new TextButton(
                group, centerGrid.getSlot(4, j % 4),
                getStringAtIndex(s, STR_VSRCRAW, i + 1), nullptr,
                OPAQUE | ((g_model.potsWarnEnabled & (1 << k)) ? BUTTON_CHECKED
                                                               : 0));
            button->setPressHandler([button, k] {
              g_model.potsWarnEnabled ^= (1 << k);
              if ((g_model.potsWarnMode == POTS_WARN_MANUAL) &&
                  (g_model.potsWarnEnabled & (1 << k))) {
                SAVE_POT_POSITION(k);
              }
              button->check(g_model.potsWarnEnabled & (1 << k) ? true : false);
              SET_DIRTY();
              return (g_model.potsWarnEnabled & (1 << k) ? 1 : 0);
            });
            j++;
          } else {
            g_model.potsWarnEnabled &= ~(1 << k);
          }
        }
        grid.addWindow(group);
      }
#endif

#if (NUM_SLIDERS)
      {
        new StaticText(window, grid.getLabelSlot(true), STR_SLIDERWARNING, 0, COLOR_THEME_PRIMARY1);
        auto group =
            new FormGroup(window, grid.getFieldSlot(),
                          FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
        GridLayout centerGrid(group);
        for (int i = SLIDER_FIRST, j = 0, k = NUM_POTS; i <= SLIDER_LAST; i++, k++) {
          char s[8];
          if ((IS_SLIDER(i))) {
            if (j > 0 && ((j % 4) == 0)) centerGrid.nextLine();

            auto *button = new TextButton(
                group, centerGrid.getSlot(4, j % 4),
                getStringAtIndex(s, STR_VSRCRAW, i + 1), nullptr,
                OPAQUE | ((g_model.potsWarnEnabled & (1 << k)) ? BUTTON_CHECKED
                                                               : 0));
            button->setPressHandler([button, k] {
              g_model.potsWarnEnabled ^= (1 << (k));
              if ((g_model.potsWarnMode == POTS_WARN_MANUAL) &&
                  (g_model.potsWarnEnabled & (1 << k))) {
                SAVE_POT_POSITION(k);
              }
              button->check(g_model.potsWarnEnabled & (1 << k) ? true : false);
              SET_DIRTY();
              return (g_model.potsWarnEnabled & (1 << k) ? 1 : 0);
            });
            j++;
          }
        }
        grid.addWindow(group);
      }
#endif
    }
  #endif
  }

  grid.nextLine();

  // Center beeps
  {
    new StaticText(window, grid.getLabelSlot(false), STR_BEEPCTR, 0, COLOR_THEME_PRIMARY1);
    auto group = new FormGroup(window, grid.getFieldSlot(), FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
    GridLayout centerGrid(group);
    for (int i = 0, j = 0; i < NUM_STICKS + NUM_POTS + NUM_SLIDERS; i++) {
      char s[2];
      if (i < NUM_STICKS ||  (IS_POT_SLIDER_AVAILABLE(i) && !IS_POT_MULTIPOS(i))) { // multipos cannot be centered
        if (j > 0 && (j % 6) == 0)
          centerGrid.nextLine();

        new TextButton(
            group, centerGrid.getSlot(6, j % 6),
            getStringAtIndex(s, STR_RETA123, i),
            [=]() -> uint8_t {
              BFBIT_FLIP(g_model.beepANACenter, bfBit<BeepANACenter>(i));
              SET_DIRTY();
              return (bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i) ? 1 : 0);
            },
            OPAQUE | (bfSingleBitGet<BeepANACenter>(g_model.beepANACenter, i)
                          ? BUTTON_CHECKED
                          : 0));
        j++;
      }
    }
    grid.addWindow(group);
  }

  // Global functions
  new StaticText(window, grid.getLabelSlot(), STR_USE_GLOBAL_FUNCS, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(window, grid.getFieldSlot(), GET_SET_INVERTED(g_model.noGlobalFunctions));
  grid.nextLine();

  {
     // Model ADC jitter filter
    new StaticText(window, grid.getLabelSlot(), STR_JITTER_FILTER, 0, COLOR_THEME_PRIMARY1);
    new Choice(window, grid.getFieldSlot(), STR_ADCFILTERVALUES, 0, 2, GET_SET_DEFAULT(g_model.jitterFilter));
    grid.nextLine();
  }

  // Internal module
  {
    new Subtitle(window, grid.getLineSlot(), TR_INTERNALRF, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    grid.addWindow(new ModuleWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}, INTERNAL_MODULE));
  }

  // External module
  {
    new Subtitle(window, grid.getLineSlot(), TR_EXTERNALRF, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    grid.addWindow(new ModuleWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}, EXTERNAL_MODULE));
  }

  // Trainer
  {
    new Subtitle(window, grid.getLineSlot(), STR_TRAINER, 0, COLOR_THEME_PRIMARY1);
    grid.nextLine();
    grid.addWindow(new TrainerModuleWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}));
  }

}

