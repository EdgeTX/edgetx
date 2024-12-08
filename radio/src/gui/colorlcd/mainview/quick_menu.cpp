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

  w = (QuickMenuGroup::FAB_BUTTON_WIDTH + PAD_TINY) * QM_COLS - PAD_TINY + PAD_MEDIUM * 2;
  h = (QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY) * QM_ROWS - PAD_TINY + PAD_SMALL + PAD_MEDIUM * 2;

#if !PORTRAIT_LCD
  if (modelHasNotes())
    w += QuickMenuGroup::FAB_BUTTON_WIDTH + PAD_TINY;
#endif

  box = new Window(this, {(LCD_W - w) / 2, (LCD_H - h) / 2, w, h}, etx_modal_dialog_create);
  box->padAll(PAD_SMALL);

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

  channelsBtn = mainMenu->addButton(ICON_MONITOR, STR_MAIN_MENU_CHANNEL_MONITOR,
                      [=]() -> uint8_t {
                        mainMenu->setCurrent(channelsBtn);
                        mainMenu->setDisabled(false);
                        mainMenu->clearFocus();
                        if (!channelsSubMenu) buildChannelsMenu();
                        enableSubMenu(channelsSubMenu);
                        return 0;
                      });
  channelsBtn->setFocusHandler([=](bool focus) {
    if (!deleted()) {
      if (focus) mainMenu->setCurrent(channelsBtn);
      if (!channelsSubMenu) buildChannelsMenu();
      channelsSubMenu->show(focus);
      if (!focus && mainMenu)
        mainMenu->setGroup();
    }
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

  screensBtn = mainMenu->addButton(ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS,
                      [=]() -> uint8_t {
                        mainMenu->setCurrent(screensBtn);
                        mainMenu->setDisabled(false);
                        mainMenu->clearFocus();
                        if (!screensSubMenu) buildScreensMenu();
                        enableSubMenu(screensSubMenu);
                        return 0;
                      });
  screensBtn->setFocusHandler([=](bool focus) {
    if (!deleted()) {
      if (focus) mainMenu->setCurrent(screensBtn);
      if (!screensSubMenu) buildScreensMenu();
      screensSubMenu->show(focus);
      if (!focus && mainMenu)
        mainMenu->setGroup();
    }
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

  statsBtn = mainMenu->addButton(ICON_STATS, STR_MAIN_MENU_STATISTICS, [=]() -> uint8_t {
                        mainMenu->setCurrent(statsBtn);
                        mainMenu->setDisabled(false);
                        mainMenu->clearFocus();
                        if (!statsSubMenu) buildStatsMenu();
                        enableSubMenu(statsSubMenu);
    return 0;
  });
  statsBtn->setFocusHandler([=](bool focus) {
    if (!deleted()) {
      if (focus) mainMenu->setCurrent(statsBtn);
      if (!statsSubMenu) buildStatsMenu();
      statsSubMenu->show(focus);
      if (!focus && mainMenu)
        mainMenu->setGroup();
    }
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
    modelSubMenu->setDisabled(true);
    radioSubMenu->setDisabled(true);
    channelsSubMenu->setDisabled(true);
    screensSubMenu->setDisabled(true);
    statsSubMenu->setDisabled(true);
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
    modelSubMenu->setCurrent(selection - MODEL_FIRST);
    enableSubMenu(modelSubMenu);
  } else if (subMenuGroup(selection) == RADIO_GROUP) {
    mainMenu->setCurrent(radioBtn);
    mainMenu->setDisabled(false);
    if (!radioSubMenu) buildRadioMenu();
    radioSubMenu->setCurrent(selection - RADIO_FIRST);
    enableSubMenu(radioSubMenu);
  } else if (subMenuGroup(selection) == CHANNELS_GROUP) {
    mainMenu->setCurrent(channelsBtn);
    mainMenu->setDisabled(false);
    if (!channelsSubMenu) buildChannelsMenu();
    channelsSubMenu->setCurrent(selection - CHANNELS_FIRST);
    enableSubMenu(channelsSubMenu);
  } else if (subMenuGroup(selection) == SCREENS_GROUP) {
    mainMenu->setCurrent(screensBtn);
    mainMenu->setDisabled(false);
    if (!screensSubMenu) buildScreensMenu();
    screensSubMenu->setCurrent(selection - SCREENS_FIRST);
    enableSubMenu(screensSubMenu);
  } else if (subMenuGroup(selection) == STATS_GROUP) {
    mainMenu->setCurrent(statsBtn);
    mainMenu->setDisabled(false);
    if (!statsSubMenu) buildStatsMenu();
    statsSubMenu->setCurrent(selection - STATS_FIRST);
    enableSubMenu(statsSubMenu);
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
  if (subMenu >= MODEL_FIRST && subMenu <= MODEL_LAST)
    return MODEL_GROUP;
  if (subMenu >= RADIO_FIRST && subMenu <= RADIO_LAST)
    return RADIO_GROUP;
  if (subMenu >= CHANNELS_FIRST && subMenu <= CHANNELS_LAST)
    return CHANNELS_GROUP;
  if (subMenu >= SCREENS_FIRST && subMenu <= SCREENS_LAST)
    return SCREENS_GROUP;
  if (subMenu >= STATS_FIRST && subMenu <= STATS_LAST)
    return STATS_GROUP;
  return NO_GROUP;
}

struct SubMenuItem {
  EdgeTxIcon icon;
  const char* title;
  QuickMenu::SubMenu subMenu;
  std::function<bool()> enabled;
};

static SubMenuItem modelMenuItems[] = {
  { ICON_MODEL_SETUP, STR_MENU_MODEL_SETUP, QuickMenu::MODEL_SETUP},
  { ICON_MODEL_HELI, STR_MENUHELISETUP, QuickMenu::MODEL_HELI, modelHeliEnabled},
  { ICON_MODEL_FLIGHT_MODES, STR_MENUFLIGHTMODES, QuickMenu::MODEL_FLIGHTMODES, modelFMEnabled},
  { ICON_MODEL_INPUTS, STR_MENUINPUTS, QuickMenu::MODEL_INPUTS},
  { ICON_MODEL_MIXER, STR_MIXES, QuickMenu::MODEL_MIXES},
  { ICON_MODEL_OUTPUTS, STR_MENULIMITS, QuickMenu::MODEL_OUTPUTS},
  { ICON_MODEL_CURVES, STR_MENUCURVES, QuickMenu::MODEL_CURVES, modelCurvesEnabled},
  { ICON_MODEL_GVARS, STR_MENU_GLOBAL_VARS, QuickMenu::MODEL_GVARS, modelGVEnabled},
  { ICON_MODEL_LOGICAL_SWITCHES, STR_MENULOGICALSWITCHES, QuickMenu::MODEL_LS, modelSFEnabled},
  { ICON_MODEL_SPECIAL_FUNCTIONS, STR_MENUCUSTOMFUNC, QuickMenu::MODEL_SF, modelSFEnabled},
  { ICON_MODEL_LUA_SCRIPTS, STR_MENUCUSTOMSCRIPTS, QuickMenu::MODEL_SCRIPTS, modelCustomScriptsEnabled},
  { ICON_MODEL_TELEMETRY, STR_MENUTELEMETRY, QuickMenu::MODEL_TELEMETRY, modelTelemetryEnabled},
};

static SubMenuItem radioMenuItems[] = {
  { ICON_RADIO_TOOLS, STR_MENUTOOLS, QuickMenu::RADIO_TOOLSCRIPTS},
  { ICON_RADIO_SD_MANAGER, STR_SD_CARD, QuickMenu::RADIO_SD},
  { ICON_RADIO_SETUP, STR_RADIO_SETUP, QuickMenu::RADIO_SETUP},
  { ICON_RADIO_EDIT_THEME, STR_THEME_EDITOR, QuickMenu::RADIO_THEMES, radioThemesEnabled},
  { ICON_RADIO_GLOBAL_FUNCTIONS, STR_MENUSPECIALFUNCS, QuickMenu::RADIO_GF, radioGFEnabled},
  { ICON_RADIO_TRAINER, STR_MENUTRAINER, QuickMenu::RADIO_TRAINER, radioTrainerEnabled},
  { ICON_RADIO_HARDWARE, STR_HARDWARE, QuickMenu::RADIO_HARDWARE},
  { ICON_RADIO_VERSION, STR_MENUVERSION, QuickMenu::RADIO_VERSION},
};

static SubMenuItem channelsMenuItems[] = {
  { ICON_MONITOR_CHANNELS1, STR_MONITOR_CHANNELS[0], QuickMenu::CHANNELS_PG1},
  { ICON_MONITOR_CHANNELS2, STR_MONITOR_CHANNELS[1], QuickMenu::CHANNELS_PG2},
  { ICON_MONITOR_CHANNELS3, STR_MONITOR_CHANNELS[2], QuickMenu::CHANNELS_PG3},
  { ICON_MONITOR_CHANNELS3, STR_MONITOR_CHANNELS[3], QuickMenu::CHANNELS_PG4},
  { ICON_MONITOR_LOGICAL_SWITCHES, STR_MONITOR_SWITCHES, QuickMenu::CHANNELS_LS},
};

static SubMenuItem statsMenuItems[] = {
  { ICON_STATS_THROTTLE_GRAPH, STR_STATISTICS, QuickMenu::STATS_STATS},
  { ICON_STATS_DEBUG, STR_DEBUG, QuickMenu::STATS_DEBUG},
};

QuickMenuGroup* QuickMenu::buildSubMenu(struct SubMenuItem* items, int count, std::function<PageGroup*()> create, SubMenu first)
{
  auto subMenu = new QuickMenuGroup(box,
          {0, (QuickMenuGroup::FAB_BUTTON_HEIGHT * QMMAIN_ROWS) + PAD_MEDIUM, w - PAD_MEDIUM * 2,
           (QM_ROWS - QMMAIN_ROWS) * QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY * 2},
          true);

  for (int i = 0; i < count; i += 1) {
    subMenu->addButton(items[i].icon, items[i].title, [=]() -> uint8_t {
      if (pageGroup && (subMenuGroup(items[i].subMenu) == subMenuGroup(curPage))) {
        deleteLater();
        pageGroup->setCurrentTab(items[i].subMenu - first);
      } else {
        onSelect();
        create()->setCurrentTab(items[i].subMenu - first);
      }
      return 0;
    }, items[i].enabled ? items[i].enabled() : true);
  }

  subMenu->hide();
  subMenu->setDisabled(true);

  return subMenu;
}

void QuickMenu::buildModelMenu()
{
  modelSubMenu = buildSubMenu(modelMenuItems, DIM(modelMenuItems), [=]() { return new ModelMenu(); }, MODEL_FIRST);
}

void QuickMenu::buildRadioMenu()
{
  radioSubMenu = buildSubMenu(radioMenuItems, DIM(radioMenuItems), [=]() { return new RadioMenu(); }, RADIO_FIRST);
}

void QuickMenu::buildChannelsMenu()
{
  channelsSubMenu = buildSubMenu(channelsMenuItems, DIM(channelsMenuItems), [=]() { return new ChannelsViewMenu(); }, CHANNELS_FIRST);
}

void QuickMenu::buildStatsMenu()
{
  statsSubMenu = buildSubMenu(statsMenuItems, DIM(statsMenuItems), [=]() { return new StatisticsViewPageGroup(); }, STATS_FIRST);
}

void QuickMenu::buildScreensMenu()
{
  screensSubMenu = new QuickMenuGroup(box,
          {0, (QuickMenuGroup::FAB_BUTTON_HEIGHT * QMMAIN_ROWS) + PAD_MEDIUM, w - PAD_MEDIUM * 2,
           (QM_ROWS - QMMAIN_ROWS) * QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY * 2},
          true);

  screensSubMenu->addButton(ICON_THEME_SETUP, STR_USER_INTERFACE, [=]() -> uint8_t {
    if (pageGroup && (SCREENS_GROUP == subMenuGroup(curPage))) {
      deleteLater();
      pageGroup->setCurrentTab(0);
    } else {
      onSelect();
      new ScreenMenu(0);
    }
    return 0;
  });

  int viewCount = 0;

  for (int i = 0; i < MAX_CUSTOM_SCREENS; i++) {
    std::string title(STR_MAIN_VIEW_X);
    if (i >= 9) {
      title[title.size() - 2] = '1';
      title.back() = (i - 9) + '0';
    } else {
      title[title.size() - 2] = i + '1';
      title.back() = ' ';
    }
    screensSubMenu->addButton((EdgeTxIcon)(ICON_THEME_VIEW1 + i), title.c_str(), [=]() -> uint8_t {
      if (pageGroup && (SCREENS_GROUP == subMenuGroup(curPage))) {
        deleteLater();
        pageGroup->setCurrentTab(i +1);
      } else {
        onSelect();
        new ScreenMenu(i + 1);
      }
      return 0;
    }, customScreens[i] != nullptr);
    if (customScreens[i]) viewCount += 1;
  }

  screensSubMenu->addButton(ICON_THEME_ADD_VIEW, STR_ADD_MAIN_VIEW, [=]() -> uint8_t {
    if (pageGroup && (SCREENS_GROUP == subMenuGroup(curPage))) {
      deleteLater();
      pageGroup->setCurrentTab(viewCount + 1);
    } else {
      onSelect();
      new ScreenMenu(viewCount + 1);
    }
    return 0;
  });
}

void QuickMenu::onEvent(event_t event)
{
  if (event == EVT_KEY_LONG(KEY_EXIT)) {
    inSubMenu = false;
    onCancel();
  } else {
    Window::onEvent(event);
  }
}
