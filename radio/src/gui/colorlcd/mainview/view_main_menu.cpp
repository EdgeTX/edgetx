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

#include "view_main_menu.h"

#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "model_select.h"
#include "edgetx.h"
#include "quick_menu.h"
#include "etx_lv_theme.h"
#include "view_about.h"
#include "view_channels.h"
#include "view_statistics.h"
#include "view_text.h"

static void modal_dialog_constructor(const lv_obj_class_t* class_p,
                                     lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->rounded, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->bg_opacity_75, LV_PART_MAIN);
  etx_bg_color(obj, COLOR_BLACK_INDEX, LV_PART_MAIN);
}

static const lv_obj_class_t etx_modal_dialog_class = {
    .base_class = &window_base_class,
    .constructor_cb = modal_dialog_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .instance_size = sizeof(lv_obj_t)};

static lv_obj_t* etx_modal_dialog_create(lv_obj_t* parent)
{
  return etx_create(&etx_modal_dialog_class, parent);
}

ViewMainMenu::ViewMainMenu(Window* parent, std::function<void()> closeHandler) :
    Window(parent, {0, 0, LCD_W, LCD_H}),
    closeHandler(std::move(closeHandler))
{
  // Save focus
  Layer::push(this);

  w = QuickMenuGroup::FAB_BUTTON_WIDTH * QM_COLS + PAD_LARGE * 2;
  h = QuickMenuGroup::FAB_BUTTON_HEIGHT * QM_ROWS + PAD_LARGE * 3;

#if !PORTRAIT_LCD
  if (modelHasNotes())
    w += QuickMenuGroup::FAB_BUTTON_WIDTH;
#endif

  box = new Window(this, {(LCD_W - w) / 2, (LCD_H - h) / 2, w, h}, etx_modal_dialog_create);
  box->padAll(PAD_MEDIUM);

  mainMenu = new QuickMenuGroup(box,
          {0, 0, w - PAD_MEDIUM * 2, QMMAIN_ROWS * QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY * 2},
          true);

  mainMenu->setGroup();

  buildMainMenu();
}

void ViewMainMenu::buildMainMenu()
{
  mainMenu->addButton(ICON_MODEL_SELECT, STR_MAIN_MENU_MANAGE_MODELS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ModelLabelsWindow();
                        return 0;
                      });

  if (modelHasNotes())
    mainMenu->addButton(ICON_MODEL_NOTES, STR_MAIN_MENU_MODEL_NOTES,
                        [=]() -> uint8_t {
                          deleteLater();
                          readModelNotes(true);
                          return 0;
                        });

  mainMenu->addButton(ICON_MONITOR, STR_MAIN_MENU_CHANNEL_MONITOR,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ChannelsViewMenu();
                        return 0;
                      });

  modelBtn = mainMenu->addButton(ICON_MODEL, STR_MAIN_MENU_MODEL_SETTINGS,
                      [=]() -> uint8_t {
                        mainMenu->setCurrent(modelBtn);
                        mainMenu->setDisabled(false);
                        mainMenu->defocus();
                        lv_event_send(modelBtn->getLvObj(), LV_EVENT_FOCUSED, nullptr);
                        if (!modelSubMenu) buildModelMenu();
                        modelSubMenu->setGroup();
                        modelSubMenu->setFocus();
                        modelSubMenu->setEnabled();
                        inSubMenu = true;
                        return 0;
                      });
  modelBtn->setFocusHandler([=](bool focus) {
    if (focus) mainMenu->setCurrent(modelBtn);
    if (!modelSubMenu) buildModelMenu();
    modelSubMenu->show(focus);
    if (!focus && mainMenu)
      mainMenu->setGroup();
  });

  radioBtn = mainMenu->addButton(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS,
                      [=]() -> uint8_t {
                        mainMenu->setCurrent(radioBtn);
                        mainMenu->setDisabled(false);
                        mainMenu->defocus();
                        lv_event_send(radioBtn->getLvObj(), LV_EVENT_FOCUSED, nullptr);
                        if (!radioSubMenu) buildRadioMenu();
                        radioSubMenu->setGroup();
                        radioSubMenu->setFocus();
                        radioSubMenu->setEnabled();
                        inSubMenu = true;
                        return 0;
                      });
  radioBtn->setFocusHandler([=](bool focus) {
    if (focus) mainMenu->setCurrent(radioBtn);
    if (!radioSubMenu) buildRadioMenu();
    radioSubMenu->show(focus);
    if (!focus && mainMenu)
      mainMenu->setGroup();
  });

  mainMenu->addButton(ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ScreenMenu();
                        return 0;
                      });

  mainMenu->addButton(
      ICON_MODEL_TELEMETRY, STR_MAIN_MENU_RESET_TELEMETRY, [=]() -> uint8_t {
        deleteLater();
        Menu* resetMenu = new Menu();
        resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
        resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
        resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
        resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
        resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
        return 0;
      });

  mainMenu->addButton(ICON_STATS, STR_MAIN_MENU_STATISTICS, [=]() -> uint8_t {
    deleteLater();
    new StatisticsViewPageGroup();
    return 0;
  });

  mainMenu->addButton(ICON_EDGETX, STR_MAIN_MENU_ABOUT_EDGETX,
                      [=]() -> uint8_t {
                        deleteLater();
                        new AboutUs();
                        return 0;
                      });
}

void ViewMainMenu::buildModelMenu()
{
  modelSubMenu = new QuickMenuGroup(box,
          {0, (QuickMenuGroup::FAB_BUTTON_HEIGHT * QMMAIN_ROWS) + PAD_MEDIUM, w - PAD_MEDIUM * 2,
           (QM_ROWS - QMMAIN_ROWS) * QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY * 2},
          true);
  modelSubMenu->hide();

  modelSubMenu->addButton(ICON_MODEL_SETUP, STR_MENU_MODEL_SETUP, [=]() -> uint8_t {
    deleteLater();
    (new ModelMenu())->setCurrentTab(0);
    return 0;
  });
  if (modelHeliEnabled())
    modelSubMenu->addButton(ICON_MODEL_HELI, STR_MENUHELISETUP, [=]() -> uint8_t {
      deleteLater();
      (new ModelMenu())->setCurrentTab(1);
      return 0;
    });
  if (modelFMEnabled())
    modelSubMenu->addButton(ICON_MODEL_FLIGHT_MODES, STR_MENUFLIGHTMODES, [=]() -> uint8_t {
      deleteLater();
      (new ModelMenu())->setCurrentTab(2);
      return 0;
    });
  modelSubMenu->addButton(ICON_MODEL_INPUTS, STR_MENUINPUTS, [=]() -> uint8_t {
    deleteLater();
    (new ModelMenu())->setCurrentTab(3);
    return 0;
  });
  modelSubMenu->addButton(ICON_MODEL_MIXER, STR_MIXES, [=]() -> uint8_t {
    deleteLater();
    (new ModelMenu())->setCurrentTab(4);
    return 0;
  });
  modelSubMenu->addButton(ICON_MODEL_OUTPUTS, STR_MENULIMITS, [=]() -> uint8_t {
    deleteLater();
    (new ModelMenu())->setCurrentTab(5);
    return 0;
  });
  if (modelCurvesEnabled())
    modelSubMenu->addButton(ICON_MODEL_CURVES, STR_MENUCURVES, [=]() -> uint8_t {
      deleteLater();
      (new ModelMenu())->setCurrentTab(6);
      return 0;
    });
  if (modelGVEnabled())
    modelSubMenu->addButton(ICON_MODEL_GVARS, STR_MENU_GLOBAL_VARS, [=]() -> uint8_t {
      deleteLater();
      (new ModelMenu())->setCurrentTab(7);
      return 0;
    });
  if (modelLSEnabled())
    modelSubMenu->addButton(ICON_MODEL_LOGICAL_SWITCHES, STR_MENULOGICALSWITCHES, [=]() -> uint8_t {
      deleteLater();
      (new ModelMenu())->setCurrentTab(8);
      return 0;
    });
  if (modelSFEnabled())
    modelSubMenu->addButton(ICON_MODEL_SPECIAL_FUNCTIONS, STR_MENUCUSTOMFUNC, [=]() -> uint8_t {
      deleteLater();
      (new ModelMenu())->setCurrentTab(9);
      return 0;
    });
  if (modelCustomScriptsEnabled())
    modelSubMenu->addButton(ICON_MODEL_LUA_SCRIPTS, STR_MENUCUSTOMSCRIPTS, [=]() -> uint8_t {
      deleteLater();
      (new ModelMenu())->setCurrentTab(10);
      return 0;
    });
  if (modelTelemetryEnabled())
    modelSubMenu->addButton(ICON_MODEL_TELEMETRY, STR_MENUTELEMETRY, [=]() -> uint8_t {
      deleteLater();
      (new ModelMenu())->setCurrentTab(11);
      return 0;
    });
  modelSubMenu->defocus();
  modelSubMenu->setDisabled(true);
}

void ViewMainMenu::buildRadioMenu()
{
  radioSubMenu = new QuickMenuGroup(box,
          {0, (QuickMenuGroup::FAB_BUTTON_HEIGHT * QMMAIN_ROWS) + PAD_MEDIUM, w - PAD_MEDIUM * 2,
           (QM_ROWS - QMMAIN_ROWS) * QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY * 2},
          true);
  radioSubMenu->hide();

  radioSubMenu->addButton(ICON_RADIO_TOOLS, STR_MENUTOOLS, [=]() -> uint8_t {
    deleteLater();
    (new RadioMenu())->setCurrentTab(0);
    return 0;
  });
  radioSubMenu->addButton(ICON_RADIO_SD_MANAGER, STR_SD_CARD, [=]() -> uint8_t {
    deleteLater();
    (new RadioMenu())->setCurrentTab(1);
    return 0;
  });
  radioSubMenu->addButton(ICON_RADIO_SETUP, STR_RADIO_SETUP, [=]() -> uint8_t {
    deleteLater();
    (new RadioMenu())->setCurrentTab(2);
    return 0;
  });
  if (radioThemesEnabled())
    radioSubMenu->addButton(ICON_RADIO_EDIT_THEME, STR_THEME_EDITOR, [=]() -> uint8_t {
      deleteLater();
      (new RadioMenu())->setCurrentTab(3);
      return 0;
    });
  if (radioGFEnabled())
    radioSubMenu->addButton(ICON_RADIO_GLOBAL_FUNCTIONS, STR_MENUSPECIALFUNCS, [=]() -> uint8_t {
      deleteLater();
      (new RadioMenu())->setCurrentTab(4);
      return 0;
    });
  if (radioTrainerEnabled())
    radioSubMenu->addButton(ICON_RADIO_TRAINER, STR_MENUTRAINER, [=]() -> uint8_t {
      deleteLater();
      (new RadioMenu())->setCurrentTab(5);
      return 0;
    });
  radioSubMenu->addButton(ICON_RADIO_HARDWARE, STR_HARDWARE, [=]() -> uint8_t {
    deleteLater();
    (new RadioMenu())->setCurrentTab(6);
    return 0;
  });
  radioSubMenu->addButton(ICON_RADIO_VERSION, STR_MENUVERSION, [=]() -> uint8_t {
    deleteLater();
    (new RadioMenu())->setCurrentTab(7);
    return 0;
  });
  radioSubMenu->defocus();
  radioSubMenu->setDisabled(true);
}

void ViewMainMenu::deleteLater(bool detach, bool trash)
{
  if (closeHandler) closeHandler();
  Layer::pop(this);
  Window::deleteLater(detach, trash);
}

void ViewMainMenu::onClicked() { deleteLater(); }

void ViewMainMenu::onCancel()
{
  if (inSubMenu) {
    inSubMenu = false;
    mainMenu->setEnabled();
    mainMenu->setGroup();
    radioSubMenu->defocus();
    modelSubMenu->defocus();
    radioSubMenu->setDisabled(true);
    modelSubMenu->setDisabled(true);
  } else {
    deleteLater();
  }
}
