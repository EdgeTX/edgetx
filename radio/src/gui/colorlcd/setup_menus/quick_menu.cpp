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
    subMenu->addButton(items[i].icon, items[i].title,
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

QuickMenu::QuickMenu(std::function<void()> cancelHandler, std::function<void(bool close)> selectHandler,
                     PageGroupBase* pageGroup, SubMenu curPage) :
    Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}),
    cancelHandler(std::move(cancelHandler)),
    selectHandler(std::move(selectHandler)),
    pageGroup(pageGroup), curPage(curPage)
{
  setWindowFlag(OPAQUE);

  etx_obj_add_style(lvobj, styles->bg_opacity_75, LV_PART_MAIN);
  etx_bg_color(lvobj, COLOR_BLACK_INDEX, LV_PART_MAIN);

  new StaticIcon(this, LCD_W - EdgeTxStyles::UI_ELEMENT_HEIGHT, PAD_TINY, ICON_BTN_CLOSE, COLOR_THEME_PRIMARY2_INDEX);

  // Save focus
  Layer::push(this);

  auto body = new Window(this, {(LCD_W - QM_POPUP_W) / 2, (LCD_H - QM_POPUP_H) / 2 + PAD_LARGE + PAD_SMALL, QM_POPUP_W, QM_POPUP_H});
  body->padAll(PAD_OUTLINE);

  auto box = new Window(body, {0, 0, QM_MAIN_W, QM_MAIN_H});

  mainMenu = new QuickMenuGroup(box, (lv_flex_flow_t)QM_MAIN_FLOW);

  mainMenu->addButton(ICON_MODEL_SELECT, STR_MAIN_MENU_MANAGE_MODELS,
                      [=]() -> uint8_t {
                        onSelect(true);
                        new ModelLabelsWindow();
                        return 0;
                      });

  box = new Window(body, {QM_SUB_X, QM_SUB_Y + PAD_SMALL, QM_SUB_W, QM_SUB_H});

  QuickSubMenu* sub;

  sub = new QuickSubMenu(box, this, ICON_MODEL, STR_MAIN_MENU_MODEL_SETTINGS, modelMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS, radioMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS, screensMenuItems);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, this, ICON_RADIO_TOOLS, STR_MAIN_MENU_TOOLS, toolsMenuItems);
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

void QuickMenu::deleteLater(bool detach, bool trash)
{
  Layer::pop(this);
  if (cancelHandler) cancelHandler();
  Window::deleteLater(detach, trash);
}

void QuickMenu::onSelect(bool close)
{
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

void QuickMenu::onClicked()
{
  closeMenu();
}
