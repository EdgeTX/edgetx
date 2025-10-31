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

bool QuickSubMenu::isSubMenu(QMPage n)
{
  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
    if (items[i].qmPage == n) return true;
  return false;
}

bool QuickSubMenu::isSubMenu(ButtonBase* b)
{
  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
    if (menuButton == b) return true;
  return false;
}

int QuickSubMenu::getIndex(QMPage n)
{
  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
    if (items[i].qmPage == n) return i;
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

void QuickSubMenu::setCurrent(QMPage n)
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
        [=]() { QuickMenu::setCurrentPage(items[i].qmPage); });
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
    if (quickMenu->getPageGroup() && quickMenu->getPageGroup()->hasSubMenu(items[n].qmPage)) {
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
    onSelect(true);
    items[n].action();
  }
  return 0;
}

void QuickSubMenu::onSelect(bool close)
{
  quickMenu->onSelect(close);
}

//-----------------------------------------------------------------------------

QuickMenu* QuickMenu::instance = nullptr;
QMPage QuickMenu::curPage = QM_NONE;

QuickMenu* QuickMenu::openQuickMenu(std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler,
            PageGroupBase* pageGroup, QMPage curPage)
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
  etx_bg_color(lvobj, COLOR_BLACK_INDEX);

  auto sep = lv_obj_create(lvobj);
  etx_solid_bg(sep, COLOR_WHITE_INDEX);
  lv_obj_set_size(sep, LCD_W, PAD_THREE);

  auto mask = getBuiltinIcon(ICON_TOP_LOGO);
  new StaticIcon(this, (LCD_W - mask->width) / 2, 0, ICON_TOP_LOGO, COLOR_WHITE_INDEX);

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

  box = new Window(this, {QM_SUB_X, QM_SUB_Y, QM_SUB_W, QM_SUB_H});

  for (int i = 0; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      mainMenu->addButton(qmTopItems[i].icon, qmTopItems[i].qmTitle, qmTopItems[i].action);
    } else {
      auto sub = new QuickSubMenu(box, this, qmTopItems[i].icon, qmTopItems[i].qmTitle, qmTopItems[i].title, qmTopItems[i].subMenuItems);
      sub->addButton();
      subMenus.emplace_back(sub);
    }
  }
}

void QuickMenu::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;

  instance = nullptr;

  Window::deleteLater(detach, trash);
}

void QuickMenu::openQM(std::function<void()> cancelHandler,
            std::function<void(bool close)> selectHandler,
            PageGroupBase* newPageGroup, QMPage newCurPage)
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
    if (curPage > QM_MANAGE_MODELS) {
      mainMenu->setDisabled(false);
      mainMenu->clearFocus();
      setFocus(curPage);
    } else {
      if (curPage == QM_MANAGE_MODELS)
        mainMenu->setCurrent(0);
      focusMainMenu();
    }
  }
}

void QuickMenu::selected()
{
  if (instance)
    instance->onSelect(true);
}

void QuickMenu::openPage(QMPage page)
{
  for (int i = 0; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      if (qmTopItems[i].qmPage == page) {
        qmTopItems[i].action();
        return;
        }
    } else {
      PageDef* sub = qmTopItems[i].subMenuItems;
      for (int j = 0; sub[j].icon != EDGETX_ICONS_COUNT; j += 1) {
        if (sub[j].qmPage == page) {
          if (sub[j].pageAction == PAGE_ACTION) {
            sub[j].action();
          } else {
            auto pg = new PageGroup(qmTopItems[i].icon, qmTopItems[i].title, sub);
            pg->setCurrentTab(j);
            return;
          }
        }
      }
    }
  }
}

EdgeTxIcon QuickMenu::pageIcon(QMPage page)
{
  for (int i = 0; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      if (qmTopItems[i].qmPage == page) {
        return qmTopItems[i].icon;
        }
    } else {
      PageDef* sub = qmTopItems[i].subMenuItems;
      for (int j = 0; sub[j].icon != EDGETX_ICONS_COUNT; j += 1) {
        if (sub[j].qmPage == page) {
          return sub[j].icon;
        }
      }
    }
  }
  return EDGETX_ICONS_COUNT;
}

int QuickMenu::pageIndex(QMPage page)
{
  if (page >= QM_TOOLS_APPS) return page - QM_TOOLS_APPS;
  if (page >= QM_UI_THEMES) return page - QM_UI_THEMES;
  if (page >= QM_RADIO_SETUP) return page - QM_RADIO_SETUP;
  if (page >= QM_MODEL_SETUP) return page - QM_MODEL_SETUP;
  return 0;
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
    curPage = QM_NONE;
  } else {
    closeMenu();
  }
}

void QuickMenu::setFocus(QMPage selection)
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
void QuickMenu::doKeyShortcut(event_t event)
{
  QMPage pg = g_eeGeneral.getKeyShortcut(event);
  if (pg == QM_OPEN_QUICK_MENU) {
    closeMenu();
  } else {
    onSelect(true);
    QuickMenu::openPage(pg);
  }
}
void QuickMenu::onPressSYS() { doKeyShortcut(EVT_KEY_BREAK(KEY_SYS)); }
void QuickMenu::onLongPressSYS() { doKeyShortcut(EVT_KEY_LONG(KEY_SYS)); }
void QuickMenu::onPressMDL() { doKeyShortcut(EVT_KEY_BREAK(KEY_MODEL)); }
void QuickMenu::onLongPressMDL() { doKeyShortcut(EVT_KEY_LONG(KEY_MODEL)); }
void QuickMenu::onPressTELE() { doKeyShortcut(EVT_KEY_BREAK(KEY_TELE)); }
void QuickMenu::onLongPressTELE() { doKeyShortcut(EVT_KEY_LONG(KEY_TELE)); }
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
    curPage = QM_NONE;
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
