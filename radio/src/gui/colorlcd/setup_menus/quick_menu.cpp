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
#include "menu_channels.h"
#include "menu_statistics.h"
#include "model_select.h"
#include "edgetx.h"
#include "quick_menu_group.h"
#include "etx_lv_theme.h"
#include "view_about.h"
#include "view_text.h"

//-----------------------------------------------------------------------------

extern PageDef modelMenuItems[];
extern PageDef radioMenuItems[];
extern PageDef channelsMenuItems[];
extern PageDef statsMenuItems[];
extern PageDef screensMenuItems[];

//-----------------------------------------------------------------------------

class QuickSubMenu
{
 public:
  QuickSubMenu(Window* parent, PageGroup* pageGroup, QuickMenu* quickMenu, QuickMenuGroup* topMenu,
               EdgeTxIcon icon, const char* title, QuickMenu::SubMenu first, QuickMenu::SubMenu last,
               std::function<PageGroup*()> create, PageDef* items) :
    parent(parent), pageGroup(pageGroup), quickMenu(quickMenu), topMenu(topMenu),
    icon(icon), title(title), first(first), last(last),
    create(std::move(create)), items(items)
  {}

  bool isSubMenu(QuickMenu::SubMenu n) { return (n >= first) && (n <= last); }

  ButtonBase* addButton()
  {
    menuButton = topMenu->addButton(icon, title,
                        [=]() -> uint8_t {
                          topMenu->setCurrent(menuButton);
                          topMenu->setDisabled(false);
                          topMenu->clearFocus();
                          if (!subMenu) buildSubMenu();
                          enableSubMenu();
                          return 0;
                        });

    menuButton->setFocusHandler([=](bool focus) {
      if (!quickMenu->deleted()) {
        if (focus) topMenu->setCurrent(menuButton);
        if (!subMenu) buildSubMenu();
        subMenu->show(focus);
        if (!focus && topMenu)
          topMenu->setGroup();
      }
    });

    return menuButton;
  }

  void enableSubMenu()
  {
    subMenu->show();
    subMenu->setGroup();
    subMenu->setFocus();
    subMenu->setEnabled();
    quickMenu->enableSubMenu();
  }

  void setDisabled(bool all) { subMenu->setDisabled(all); }

  void setCurrent(int b)
  {
    topMenu->setCurrent(menuButton);
    topMenu->setDisabled(false);
    if (!subMenu) buildSubMenu();
    subMenu->setCurrent(b - first);
    enableSubMenu();
  }

 protected:
  Window* parent;
  PageGroup* pageGroup;
  QuickMenu* quickMenu;
  QuickMenuGroup* topMenu;
  EdgeTxIcon icon;
  const char* title;
  QuickMenu::SubMenu first;
  QuickMenu::SubMenu last;
  std::function<PageGroup*()> create;
  PageDef* items;
  QuickMenuGroup* subMenu;
  ButtonBase* menuButton = nullptr;

  void buildSubMenu()
  {
    subMenu = new QuickMenuGroup(parent,
            {0, (QuickMenuGroup::FAB_BUTTON_HEIGHT * QuickMenu::QMMAIN_ROWS) + PAD_MEDIUM, parent->width() - PAD_MEDIUM * 2,
            (QuickMenu::QM_ROWS - QuickMenu::QMMAIN_ROWS) * QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY * 2},
            true);

    for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1) {
      subMenu->addButton(items[i].icon, items[i].title,
          std::bind(&QuickSubMenu::onPress, this, items[i].subMenu - first),
          items[i].enabled ? items[i].enabled() : true);
    }

    subMenu->hide();
    subMenu->setDisabled(true);
  }

  uint8_t onPress(int n)
  {
    if (pageGroup && isSubMenu(quickMenu->currentPage())) {
      quickMenu->deleteLater();
      quickMenu->onSelect(false);
      pageGroup->setCurrentTab(n);
    } else {
      quickMenu->onSelect(true);
      create()->setCurrentTab(n);
    }
    return 0;
  }
};

//-----------------------------------------------------------------------------

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

QuickMenu::QuickMenu(Window* parent, std::function<void()> cancelHandler, std::function<void(bool close)> selectHandler,
                     PageGroup* pageGroup, SubMenu curPage) :
    Window(parent, {0, 0, LCD_W, LCD_H}),
    cancelHandler(std::move(cancelHandler)),
    selectHandler(std::move(selectHandler)),
    pageGroup(pageGroup), curPage(curPage)
{
  setWindowFlag(OPAQUE);

  // Save focus
  Layer::push(this);

  coord_t w = (QuickMenuGroup::FAB_BUTTON_WIDTH + PAD_TINY) * QM_COLS - PAD_TINY + PAD_MEDIUM * 2;
  coord_t h = (QuickMenuGroup::FAB_BUTTON_HEIGHT + PAD_TINY) * QM_ROWS - PAD_TINY + PAD_SMALL + PAD_MEDIUM * 2;

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
                        onSelect(true);
                        new ModelLabelsWindow();
                        return 0;
                      });

  QuickSubMenu* sub;

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_MONITOR, STR_MAIN_MENU_CHANNEL_MONITOR,
                         QuickMenu::CHANNELS_FIRST, QuickMenu::CHANNELS_LAST, []() { return new ChannelsViewMenu(); }, channelsMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_MODEL, STR_MAIN_MENU_MODEL_SETTINGS,
                         QuickMenu::MODEL_FIRST, QuickMenu::MODEL_LAST, []() { return new ModelMenu(); }, modelMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS,
                         QuickMenu::RADIO_FIRST, QuickMenu::RADIO_LAST, []() { return new RadioMenu(); }, radioMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS,
                         QuickMenu::SCREENS_FIRST, QuickMenu::SCREENS_LAST, []() { return new ScreenMenu(); }, screensMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  mainMenu->addButton(
      ICON_MODEL_TELEMETRY, STR_MAIN_MENU_RESET_TELEMETRY, [=]() -> uint8_t {
        onSelect(true);
        Menu* resetMenu = new Menu();
        resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
        resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
        resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
        resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
        resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
        return 0;
      });

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_STATS, STR_MAIN_MENU_STATISTICS,
                         QuickMenu::STATS_FIRST, QuickMenu::STATS_LAST, []() { return new StatisticsViewPageGroup(); }, statsMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  mainMenu->addButton(ICON_EDGETX, STR_MAIN_MENU_ABOUT_EDGETX,
                      [=]() -> uint8_t {
                        onSelect(true);
                        new AboutUs();
                        return 0;
                      });

  if (modelHasNotes())
    mainMenu->addButton(ICON_MODEL_NOTES, STR_MAIN_MENU_MODEL_NOTES,
                        [=]() -> uint8_t {
                          onSelect(true);
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

void QuickMenu::onSelect(bool close)
{
  if (close)
    deleteLater();
  if (selectHandler) selectHandler(close);
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
    for(auto sub : subMenus)
      sub->setDisabled(true);
  } else {
    deleteLater();
  }
}

void QuickMenu::setFocus(SubMenu selection)
{
  for(auto sub : subMenus) {
    if (sub->isSubMenu(selection)) {
      sub->setCurrent(selection);
      return;
    }
  }
}

void QuickMenu::enableSubMenu()
{
  inSubMenu = true;
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
