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
#include "view_main.h"
#include "screen_setup.h"
#include "theme_manager.h"
#include "view_channels.h"

//-----------------------------------------------------------------------------

ButtonBase* QuickSubMenu::addButton()
{
  menuButton = quickMenu->getTopMenu()->addButton(icon, title,
                      [=]() -> uint8_t {
                        activate();
                        return 0;
                      });

  menuButton->setFocusHandler([=](bool focus) {
    if (!quickMenu->deleted()) {
      if (focus) {
        if (!subMenu) buildSubMenu();
        quickMenu->getTopMenu()->setCurrent(menuButton);
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

bool QuickSubMenu::isSubMenu(ButtonBase* b)
{
  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
    if (menuButton == b) return true;
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
  if (!subMenu) buildSubMenu();
  quickMenu->getTopMenu()->setCurrent(menuButton);
  quickMenu->getTopMenu()->setDisabled(false);
  subMenu->setCurrent(getIndex(n));
  enableSubMenu();
}

void QuickSubMenu::activate()
{
  if (!subMenu) buildSubMenu();
  quickMenu->getTopMenu()->setCurrent(menuButton);
  quickMenu->getTopMenu()->setDisabled(false);
  quickMenu->getTopMenu()->clearFocus();
  enableSubMenu();
}

void QuickSubMenu::doLayout()
{
  if (subMenu)
    subMenu->doLayout(QuickMenu::QM_SUB_COLS);
}

void QuickSubMenu::buildSubMenu()
{
  subMenu = new QuickMenuGroup(parent);

  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1) {
    subMenu->addButton(items[i].icon, STR_VAL(items[i].qmTitle),
        std::bind(&QuickSubMenu::onPress, this, i),
        [=]() { return items[i].enabled ? items[i].enabled() : true; },
        [=]() { QuickMenu::setCurrentPage(items[i].subMenu); });
  }

  doLayout();
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
    quickMenu->getTopMenu()->clearFocus();
    int pgIdx = getPageNumber(n);
    if (quickMenu->getPageGroup() && quickMenu->getPageGroup()->hasSubMenu(items[n].subMenu)) {
      quickMenu->onSelect(false);
      quickMenu->getPageGroup()->setCurrentTab(pgIdx);
    } else {
      quickMenu->onSelect(true);
      auto pg = new PageGroup(icon, parentTitle, items);
      pg->setCurrentTab(pgIdx);
    }
  } else {
    quickMenu->getTopMenu()->setCurrent(menuButton);
    quickMenu->getTopMenu()->setDisabled(false);
    quickMenu->getTopMenu()->clearFocus();
    enableSubMenu();
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
QuickMenu::SubMenu QuickMenu::curPage = QuickMenu::NONE;

QuickMenu* QuickMenu::openQuickMenu(std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler,
            PageGroupBase* pageGroup, SubMenu curPage)
{
  if (!instance) {
    instance = new QuickMenu();
  }
  instance->openQM(cancelHandler, selectHandler, pageGroup, curPage);
  return instance;
}

void QuickMenu::shutdownQuickMenu()
{
  if (instance) instance->deleteLater();
}

QuickMenu::QuickMenu() :
    NavWindow(MainWindow::instance(), {0, 0, LCD_W, LCD_H})
{
  setWindowFlag(OPAQUE);

  etx_obj_add_style(lvobj, styles->bg_opacity_90, LV_PART_MAIN);
  etx_bg_color(lvobj, COLOR_THEME_QM_BG_INDEX);

  auto sep = lv_obj_create(lvobj);
  etx_solid_bg(sep, COLOR_THEME_QM_FG_INDEX);
  lv_obj_set_size(sep, LCD_W, PAD_THREE);

  auto mask = getBuiltinIcon(ICON_TOP_LOGO);
  new StaticIcon(this, (LCD_W - mask->width) / 2, 0, ICON_TOP_LOGO, COLOR_THEME_QM_FG_INDEX);

  new ButtonBase(
    this, {0, 0, LCD_W, EdgeTxStyles::UI_ELEMENT_HEIGHT},
    [=]() -> uint8_t {
      inSubMenu = false;
      onCancel();
      return 0;
    },
    window_create);

  auto box = new Window(this, {QM_MAIN_X, QM_MAIN_Y, QM_MAIN_W, QM_MAIN_H});

  mainMenu = new QuickMenuGroup(box);

  mainMenu->addButton(ICON_MODEL_SELECT, STR_QM_MANAGE_MODELS,
                      [=]() -> uint8_t {
                        onSelect(true);
                        new ModelLabelsWindow();
                        return 0;
                      });

  box = new Window(this, {QM_SUB_X, QM_SUB_Y, QM_SUB_W, QM_SUB_H});

  QuickSubMenu* sub;

  sub = new QuickSubMenu(box, this, ICON_MODEL, STR_QM_MODEL_SETUP, STR_MAIN_MENU_MODEL_SETTINGS, modelMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_RADIO, STR_QM_RADIO_SETUP, STR_MAIN_MENU_RADIO_SETTINGS, radioMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_THEME, STR_QM_UI_SETUP, STR_MAIN_MENU_SCREEN_SETTINGS, screensMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_RADIO_TOOLS, STR_QM_TOOLS, STR_QM_TOOLS, toolsMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);
}

void QuickMenu::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;

  instance = nullptr;

  NavWindow::deleteLater(detach, trash);
}

void QuickMenu::openQM(std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler,
            PageGroupBase* newPageGroup, SubMenu newCurPage)
{
  Layer::push(this);

  mainMenu->doLayout(QM_MAIN_COLS);
  for (size_t i = 0; i < subMenus.size(); i += 1)
    subMenus[i]->doLayout();

  this->cancelHandler = std::move(cancelHandler);
  this->selectHandler = std::move(selectHandler);
  show();
  lv_obj_move_foreground(lvobj);
  if (newPageGroup) {
    pageGroup = newPageGroup;
    curPage = newCurPage;
    mainMenu->setDisabled(false);
    mainMenu->clearFocus();
    setFocus(curPage);
  } else {
    pageGroup = nullptr;
    if (curPage >= QuickMenu::FIRST_SUB_MENU_ITEM) {
      mainMenu->setDisabled(false);
      mainMenu->clearFocus();
      setFocus(curPage);
    } else {
      if (curPage == QuickMenu::MANAGE_MODELS)
        mainMenu->setCurrent(0);
      focusMainMenu();
    }
  }
}

void QuickMenu::focusMainMenu()
{
  inSubMenu = false;
  mainMenu->setFocus();
  mainMenu->setEnabled();
  mainMenu->setGroup();
  for(auto sub : subMenus)
    sub->setDisabled(true);
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
    focusMainMenu();
    curPage = QuickMenu::NONE;
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

#if defined(HARDWARE_KEYS)
void QuickMenu::onPressSYS() { closeMenu(); }
void QuickMenu::onLongPressSYS() { subMenus[3]->onPress(0); }
void QuickMenu::onPressMDL() { subMenus[0]->onPress(0); }
void QuickMenu::onLongPressMDL() { onSelect(true); new ModelLabelsWindow(); }
void QuickMenu::onPressTELE() { subMenus[2]->onPress(ScreenSetupPage::FIRST_SCREEN_OFFSET); }
void QuickMenu::onLongPressTELE() { onSelect(true); new ChannelsViewMenu(); }
void QuickMenu::onLongPressRTN() { closeMenu(); }

void QuickMenu::afterPG()
{
  auto b = mainMenu->getFocusedButton();
  if (b) {
    for(auto sub : subMenus) {
      if (sub->isSubMenu(b)) {
        sub->activate();
        return;
      }
    }
    focusMainMenu();
    curPage = QuickMenu::NONE;
  }
}

void QuickMenu::onPressPGDN()
{
  mainMenu->nextEntry();
  afterPG();
}

void QuickMenu::onPressPGUP()
{
  mainMenu->prevEntry();
  afterPG();
}
#endif
