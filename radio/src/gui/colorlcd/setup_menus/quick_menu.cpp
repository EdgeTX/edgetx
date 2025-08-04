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

#include "model_select.h"
#include "edgetx.h"
#include "quick_menu_group.h"
#include "etx_lv_theme.h"
#include "view_text.h"
#include "view_main.h"
#include "screen_setup.h"
#include "theme_manager.h"
#include "view_channels.h"

//-----------------------------------------------------------------------------

ButtonBase* QuickSubMenu::addButton()
{
  menuButton = quickMenu->getTopMenu()->addButton(icon, title,
                      [=]() -> uint8_t {
                        quickMenu->getTopMenu()->setCurrent(menuButton);
                        quickMenu->getTopMenu()->setDisabled(false);
                        quickMenu->getTopMenu()->clearFocus();
                        if (!subMenu) buildSubMenu();
                        enableSubMenu();
                        return 0;
                      });

  menuButton->setFocusHandler([=](bool focus) {
    if (!quickMenu->deleted()) {
      if (focus) {
        quickMenu->getTopMenu()->setCurrent(menuButton);
        if (!subMenu) buildSubMenu();
      }
      if (subMenu)
        subMenu->show(focus);
      if (!focus && quickMenu->getTopMenu())
        quickMenu->getTopMenu()->setGroup();
    }
  });

  return menuButton;
}

bool QuickSubMenu::isSubMenu(QuickMenu::SubMenu n)
{
  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
    if (items[i].subMenu == n) return true;
  return false;
}

int QuickSubMenu::getIndex(QuickMenu::SubMenu n)
{
  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
    if (items[i].subMenu == n) return i;
  return -1;
}

void QuickSubMenu::enableSubMenu()
{
  subMenu->setGroup();
  subMenu->setFocus();
  subMenu->setEnabled();
  subMenu->show();
  quickMenu->enableSubMenu();
}

void QuickSubMenu::setDisabled(bool all)
{
  if (subMenu)
    subMenu->setDisabled(all);
}

void QuickSubMenu::setCurrent(QuickMenu::SubMenu n)
{
  quickMenu->getTopMenu()->setCurrent(menuButton);
  quickMenu->getTopMenu()->setDisabled(false);
  if (!subMenu) buildSubMenu();
  subMenu->setCurrent(getIndex(n));
  enableSubMenu();
}

void QuickSubMenu::buildSubMenu()
{
  subMenu = new QuickMenuGroup(parent, LV_FLEX_FLOW_ROW_WRAP);

  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1) {
    subMenu->addButton(items[i].icon, items[i].qmTitle,
        std::bind(&QuickSubMenu::onPress, this, i),
        items[i].enabled ? items[i].enabled() : true);
  }

  subMenu->hide();
  subMenu->setDisabled(true);
}

int QuickSubMenu::getPageNumber(int iconNumber)
{
  int pageNumber = 0;
  for (int i = 0; i < iconNumber; i += 1)
    if (items[i].pageAction == PAGE_CREATE)
      pageNumber += 1;
  return pageNumber;
}

uint8_t QuickSubMenu::onPress(int n)
{
  if (items[n].pageAction == PAGE_CREATE) {
    n = getPageNumber(n);
    if (quickMenu->getPageGroup() && isSubMenu(quickMenu->currentPage())) {
      quickMenu->onSelect(false);
      quickMenu->getPageGroup()->setCurrentTab(n);
    } else {
      quickMenu->onSelect(true);
      auto pg = new PageGroup(icon, items);
      pg->setCurrentTab(n);
    }
  } else {
    items[n].action(this);
  }
  return 0;
}

void QuickSubMenu::onSelect(bool close)
{
  quickMenu->onSelect(close);
}

//-----------------------------------------------------------------------------

QuickMenu* QuickMenu::instance = nullptr;

QuickMenu* QuickMenu::openQuickMenu(std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler,
            PageGroupBase* pageGroup, SubMenu curPage)
{
  if (!instance) {
    instance = new QuickMenu(cancelHandler, selectHandler, pageGroup, curPage);
  } else {
    instance->openQM(cancelHandler, selectHandler, pageGroup, curPage);
  }
  return instance;
}

QuickMenu::QuickMenu(std::function<void()> cancelHandler, std::function<void(bool close)> selectHandler,
                     PageGroupBase* pageGroup, SubMenu curPage) :
    NavWindow(MainWindow::instance(), {0, 0, LCD_W, LCD_H}),
    cancelHandler(std::move(cancelHandler)),
    selectHandler(std::move(selectHandler)),
    pageGroup(pageGroup), curPage(curPage)
{
  setWindowFlag(OPAQUE);

  etx_obj_add_style(lvobj, styles->bg_opacity_90, LV_PART_MAIN);
  etx_bg_color(lvobj, COLOR_DARKGREY_INDEX);

  auto mask = getBuiltinIcon(ICON_BTN_CLOSE);
  coord_t w = mask->width + 2;
  coord_t h = mask->height + 2;
  auto cb = new Window(this, {LCD_W - w, 0, w, h});
  etx_solid_bg(cb->getLvObj(), COLOR_BLACK_INDEX);
  new StaticIcon(cb, 1, 1, ICON_BTN_CLOSE, COLOR_WHITE_INDEX);

  Layer::push(this);

  auto body = new Window(this, {(LCD_W - QM_POPUP_W) / 2, (LCD_H - QM_POPUP_H) / 2 + PAD_LARGE + PAD_SMALL, QM_POPUP_W, QM_POPUP_H});
  body->padAll(PAD_OUTLINE);

  auto box = new Window(body, {0, 0, QM_MAIN_W, QM_MAIN_H});

  mainMenu = new QuickMenuGroup(box, (lv_flex_flow_t)QM_MAIN_FLOW);

  mainMenu->addButton(ICON_MODEL_SELECT, STR_QM_MANAGE_MODELS,
                      [=]() -> uint8_t {
                        onSelect(true);
                        new ModelLabelsWindow();
                        return 0;
                      });

  box = new Window(body, {QM_SUB_X, QM_SUB_Y + PAD_SMALL, QM_SUB_W, QM_SUB_H});

  QuickSubMenu* sub;

  sub = new QuickSubMenu(box, this, ICON_MODEL, STR_QM_MODEL_SETUP, modelMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_RADIO, STR_QM_RADIO_SETUP, radioMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_THEME, STR_QM_UI_SETUP, screensMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_RADIO_TOOLS, STR_QM_TOOLS, toolsMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  // if (modelHasNotes())
  //   mainMenu->addButton(ICON_MODEL_NOTES, STR_MAIN_MENU_MODEL_NOTES,
  //                       [=]() -> uint8_t {
  //                         onSelect(true);
  //                         readModelNotes(true);
  //                         return 0;
  //                       });

  mainMenu->setGroup();
}

void QuickMenu::openQM(std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler,
            PageGroupBase* pageGroup, SubMenu curPage)
{
  Layer::push(this);

  this->cancelHandler = std::move(cancelHandler);
  this->selectHandler = std::move(selectHandler);
  this->pageGroup = pageGroup;
  this->curPage = curPage;
  show();
  lv_obj_move_foreground(lvobj);
  mainMenu->setGroup();
}

void QuickMenu::onSelect(bool close)
{
  closeMenu();
  if (selectHandler) selectHandler(close);
}

void QuickMenu::closeMenu()
{
  Layer::pop(this);
  hide();
  if (cancelHandler) cancelHandler();
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
    closeMenu();
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

void QuickMenu::onClicked()
{
  inSubMenu = false;
  onCancel();
}

#if defined(HARDWARE_KEYS)
void QuickMenu::onPressSYS() { subMenus[3]->onPress(0); }
void QuickMenu::onLongPressSYS() { subMenus[1]->onPress(0); }
void QuickMenu::onPressMDL() { subMenus[0]->onPress(0); }
void QuickMenu::onLongPressMDL() { onSelect(true); new ModelLabelsWindow(); }
void QuickMenu::onPressTELE() { subMenus[2]->onPress(ScreenSetupPage::FIRST_SCREEN_OFFSET); }
void QuickMenu::onLongPressTELE() { onSelect(true); new ChannelsViewMenu(); }
void QuickMenu::onLongPressRTN() { onClicked(); }
#endif
