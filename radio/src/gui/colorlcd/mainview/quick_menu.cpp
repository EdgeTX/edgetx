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

#include "quick_menu.h"

#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "model_select.h"
#include "edgetx.h"
#include "quick_menu_group.h"
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

QuickMenu::QuickMenu(Window* parent, std::function<void()> cancelHandler, std::function<void()> selectHandler,
                     PageGroup* pageGroup, SubMenu curPage) :
    Window(parent, {0, 0, LCD_W, LCD_H}),
    cancelHandler(std::move(cancelHandler)),
    selectHandler(std::move(selectHandler)),
    pageGroup(pageGroup), curPage(curPage)
{
  setWindowFlag(OPAQUE);

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

  buildMainMenu();

  mainMenu->setGroup();
}

void QuickMenu::buildMainMenu()
{
  mainMenu->addButton(ICON_MODEL_SELECT, STR_MAIN_MENU_MANAGE_MODELS,
                      [=]() -> uint8_t {
                        onSelect();
                        new ModelLabelsWindow();
                        return 0;
                      });

  mainMenu->addButton(ICON_MONITOR, STR_MAIN_MENU_CHANNEL_MONITOR,
                      [=]() -> uint8_t {
                        onSelect();
                        new ChannelsViewMenu();
                        return 0;
                      });

  modelBtn = mainMenu->addButton(ICON_MODEL, STR_MAIN_MENU_MODEL_SETTINGS,
                      [=]() -> uint8_t {
                        mainMenu->setCurrent(modelBtn);
                        mainMenu->setDisabled(false);
                        mainMenu->clearFocus();
                        if (!modelSubMenu) buildModelMenu();
                        enableSubMenu(modelSubMenu);
                        return 0;
                      });
  modelBtn->setFocusHandler([=](bool focus) {
    if (!deleted()) {
      if (focus) mainMenu->setCurrent(modelBtn);
      if (!modelSubMenu) buildModelMenu();
      modelSubMenu->show(focus);
      if (!focus && mainMenu)
        mainMenu->setGroup();
    }
  });

  radioBtn = mainMenu->addButton(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS,
                      [=]() -> uint8_t {
                        mainMenu->setCurrent(radioBtn);
                        mainMenu->setDisabled(false);
                        mainMenu->clearFocus();
                        if (!radioSubMenu) buildRadioMenu();
                        enableSubMenu(radioSubMenu);
                        return 0;
                      });
  radioBtn->setFocusHandler([=](bool focus) {
    if (!deleted()) {
      if (focus) mainMenu->setCurrent(radioBtn);
      if (!radioSubMenu) buildRadioMenu();
      radioSubMenu->show(focus);
      if (!focus && mainMenu)
        mainMenu->setGroup();
    }
  });

  mainMenu->addButton(ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS,
                      [=]() -> uint8_t {
                        onSelect();
                        new ScreenMenu();
                        return 0;
                      });

  mainMenu->addButton(
      ICON_MODEL_TELEMETRY, STR_MAIN_MENU_RESET_TELEMETRY, [=]() -> uint8_t {
        onSelect();
        Menu* resetMenu = new Menu();
        resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
        resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
        resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
        resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
        resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
        return 0;
      });

  mainMenu->addButton(ICON_STATS, STR_MAIN_MENU_STATISTICS, [=]() -> uint8_t {
    onSelect();
    new StatisticsViewPageGroup();
    return 0;
  });

  mainMenu->addButton(ICON_EDGETX, STR_MAIN_MENU_ABOUT_EDGETX,
                      [=]() -> uint8_t {
                        onSelect();
                        new AboutUs();
                        return 0;
                      });

  if (modelHasNotes())
    mainMenu->addButton(ICON_MODEL_NOTES, STR_MAIN_MENU_MODEL_NOTES,
                        [=]() -> uint8_t {
                          onSelect();
                          readModelNotes(true);
                          return 0;
                        });
}

void QuickMenu::buildModelMenu()
{
  modelSubMenu = new QuickMenuGroup(box,
          {0, (QuickMenuGroup::FAB_BUTTON_HEIGHT * QMMAIN_ROWS) + PAD_MEDIUM, w - PAD_MEDIUM * 2,
           (QM_ROWS - QMMAIN_ROWS) * QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY * 2},
          true);

  ButtonBase* btn;

  modelSubMenu->addButton(ICON_MODEL_SETUP, STR_MENU_MODEL_SETUP, [=]() -> uint8_t {
    openModelPage(MODEL_SETUP);
    return 0;
  });
  btn = modelSubMenu->addButton(ICON_MODEL_HELI, STR_MENUHELISETUP, [=]() -> uint8_t {
    openModelPage(MODEL_HELI);
    return 0;
  }, modelHeliEnabled());
  btn = modelSubMenu->addButton(ICON_MODEL_FLIGHT_MODES, STR_MENUFLIGHTMODES, [=]() -> uint8_t {
    openModelPage(MODEL_FLIGHTMODES);
    return 0;
  }, modelFMEnabled());
  modelSubMenu->addButton(ICON_MODEL_INPUTS, STR_MENUINPUTS, [=]() -> uint8_t {
    openModelPage(MODEL_INPUTS);
    return 0;
  });
  modelSubMenu->addButton(ICON_MODEL_MIXER, STR_MIXES, [=]() -> uint8_t {
    openModelPage(MODEL_MIXES);
    return 0;
  });
  modelSubMenu->addButton(ICON_MODEL_OUTPUTS, STR_MENULIMITS, [=]() -> uint8_t {
    openModelPage(MODEL_OUTPUTS);
    return 0;
  });
  btn = modelSubMenu->addButton(ICON_MODEL_CURVES, STR_MENUCURVES, [=]() -> uint8_t {
    openModelPage(MODEL_CURVES);
    return 0;
  }, modelCurvesEnabled());
  btn = modelSubMenu->addButton(ICON_MODEL_GVARS, STR_MENU_GLOBAL_VARS, [=]() -> uint8_t {
    openModelPage(MODEL_GVARS);
    return 0;
  }, modelGVEnabled());
  btn = modelSubMenu->addButton(ICON_MODEL_LOGICAL_SWITCHES, STR_MENULOGICALSWITCHES, [=]() -> uint8_t {
    openModelPage(MODEL_LS);
    return 0;
  }, modelLSEnabled());
  btn = modelSubMenu->addButton(ICON_MODEL_SPECIAL_FUNCTIONS, STR_MENUCUSTOMFUNC, [=]() -> uint8_t {
    openModelPage(MODEL_SF);
    return 0;
  }, modelSFEnabled());
  btn = modelSubMenu->addButton(ICON_MODEL_LUA_SCRIPTS, STR_MENUCUSTOMSCRIPTS, [=]() -> uint8_t {
    openModelPage(MODEL_SCRIPTS);
    return 0;
  }, modelCustomScriptsEnabled());
  btn = modelSubMenu->addButton(ICON_MODEL_TELEMETRY, STR_MENUTELEMETRY, [=]() -> uint8_t {
    openModelPage(MODEL_TELEMETRY);
    return 0;
  }, modelTelemetryEnabled());

  modelSubMenu->hide();
  modelSubMenu->setDisabled(true);
}

void QuickMenu::buildRadioMenu()
{
  radioSubMenu = new QuickMenuGroup(box,
          {0, (QuickMenuGroup::FAB_BUTTON_HEIGHT * QMMAIN_ROWS) + PAD_MEDIUM, w - PAD_MEDIUM * 2,
           (QM_ROWS - QMMAIN_ROWS) * QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY * 2},
          true);

  ButtonBase* btn;

  radioSubMenu->addButton(ICON_RADIO_TOOLS, STR_MENUTOOLS, [=]() -> uint8_t {
    openRadioPage(RADIO_TOOLSCRIPTS);
    return 0;
  });
  radioSubMenu->addButton(ICON_RADIO_SD_MANAGER, STR_SD_CARD, [=]() -> uint8_t {
    openRadioPage(RADIO_SD);
    return 0;
  });
  radioSubMenu->addButton(ICON_RADIO_SETUP, STR_RADIO_SETUP, [=]() -> uint8_t {
    openRadioPage(RADIO_SETUP);
    return 0;
  });
  btn = radioSubMenu->addButton(ICON_RADIO_EDIT_THEME, STR_THEME_EDITOR, [=]() -> uint8_t {
    openRadioPage(RADIO_THEMES);
    return 0;
  }, radioThemesEnabled());
  btn = radioSubMenu->addButton(ICON_RADIO_GLOBAL_FUNCTIONS, STR_MENUSPECIALFUNCS, [=]() -> uint8_t {
    openRadioPage(RADIO_GF);
    return 0;
  }, radioGFEnabled());
  btn = radioSubMenu->addButton(ICON_RADIO_TRAINER, STR_MENUTRAINER, [=]() -> uint8_t {
    openRadioPage(RADIO_TRAINER);
    return 0;
  }, radioTrainerEnabled());
  radioSubMenu->addButton(ICON_RADIO_HARDWARE, STR_HARDWARE, [=]() -> uint8_t {
    openRadioPage(RADIO_HARDWARE);
    return 0;
  });
  radioSubMenu->addButton(ICON_RADIO_VERSION, STR_MENUVERSION, [=]() -> uint8_t {
    openRadioPage(RADIO_VERSION);
    return 0;
  });

  radioSubMenu->hide();
  radioSubMenu->setDisabled(true);
}

void QuickMenu::openModelPage(SubMenu newPage)
{
  if (pageGroup && (subMenuGroup(newPage) == subMenuGroup(curPage))) {
    deleteLater();
    pageGroup->setCurrentTab(newPage - MODEL_SETUP);
  } else {
    onSelect();
    (new ModelMenu())->setCurrentTab(newPage - MODEL_SETUP);
  }
}

void QuickMenu::openRadioPage(SubMenu newPage)
{
  if (pageGroup && (subMenuGroup(newPage) == subMenuGroup(curPage))) {
    deleteLater();
    pageGroup->setCurrentTab(newPage - MODEL_SETUP);
  } else {
    onSelect();
    (new RadioMenu())->setCurrentTab(newPage - RADIO_TOOLSCRIPTS);
  }
}

void QuickMenu::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);
  if (cancelHandler) cancelHandler();
  Window::deleteLater(detach, trash);
}

void QuickMenu::onSelect()
{
  deleteLater();
  if (selectHandler) selectHandler();
}

void QuickMenu::closeMenu()
{
  deleteLater();
}

void QuickMenu::onCancel()
{
  if (inSubMenu) {
    inSubMenu = false;
    mainMenu->setFocus();
    mainMenu->setEnabled();
    mainMenu->setGroup();
    radioSubMenu->setDisabled(true);
    modelSubMenu->setDisabled(true);
  } else {
    deleteLater();
  }
}

void QuickMenu::setFocus(SubMenu selection)
{
  if (subMenuGroup(selection) == MODEL_GROUP) {
    mainMenu->setCurrent(modelBtn);
    mainMenu->setDisabled(false);
    if (!modelSubMenu) buildModelMenu();
    modelSubMenu->setCurrent(selection - MODEL_SETUP);
    enableSubMenu(modelSubMenu);
  } else if (subMenuGroup(selection) == RADIO_GROUP) {
    mainMenu->setCurrent(radioBtn);
    mainMenu->setDisabled(false);
    if (!radioSubMenu) buildRadioMenu();
    radioSubMenu->setCurrent(selection - RADIO_TOOLSCRIPTS);
    enableSubMenu(radioSubMenu);
  }
}

void QuickMenu::enableSubMenu(QuickMenuGroup* subMenu)
{
  subMenu->show();
  subMenu->setGroup();
  subMenu->setFocus();
  subMenu->setEnabled();
  inSubMenu = true;
}

QuickMenu::SubMenuGroup QuickMenu::subMenuGroup(SubMenu subMenu)
{
  if (subMenu >= MODEL_SETUP && subMenu <= MODEL_TELEMETRY)
    return MODEL_GROUP;
  if (subMenu >= RADIO_TOOLSCRIPTS && subMenu <= RADIO_VERSION)
    return RADIO_GROUP;
  return NO_GROUP;
}