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
#include "menu_tools.h"
#include "model_select.h"
#include "edgetx.h"
#include "quick_menu_group.h"
#include "etx_lv_theme.h"
#include "view_text.h"
#include "view_main.h"
#include "screen_setup.h"

//-----------------------------------------------------------------------------

extern PageDef modelMenuItems[];
extern PageDef radioMenuItems[];
extern PageDef channelsMenuItems[];
extern PageDef statsMenuItems[];
extern PageDef screensMenuItems[];
extern PageDef toolsMenuItems[];

//-----------------------------------------------------------------------------

ButtonBase* QuickSubMenu::addButton()
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
      if (focus) {
        topMenu->setCurrent(menuButton);
        if (!subMenu) buildSubMenu();
      }
      if (subMenu)
        subMenu->show(focus);
      if (!focus && topMenu)
        topMenu->setGroup();
    }
  });

  return menuButton;
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

void QuickSubMenu::setCurrent(int b)
{
  topMenu->setCurrent(menuButton);
  topMenu->setDisabled(false);
  if (!subMenu) buildSubMenu();
  subMenu->setCurrent(b - first);
  enableSubMenu();
}

void QuickSubMenu::buildSubMenu()
{
  subMenu = new QuickMenuGroup(parent,
          {
            GRP_W(viewSubX, 1), GRP_H(viewSubY, 1),
            GRP_W(viewSubCols, 2), GRP_H(viewSubRows, 2)
          }, LV_FLEX_FLOW_ROW_WRAP);

  for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1) {
    subMenu->addButton(items[i].icon, items[i].title,
        std::bind(&QuickSubMenu::onPress, this, items[i].subMenu - first),
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
    if (pageGroup && isSubMenu(quickMenu->currentPage())) {
      quickMenu->onSelect(false);
      pageGroup->setCurrentTab(n);
    } else {
      quickMenu->onSelect(true);
      create()->setCurrentTab(n);
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

QuickMenu::QuickMenu(Window* parent, std::function<void()> cancelHandler, std::function<void(bool close)> selectHandler,
                     PageGroupBase* pageGroup, SubMenu curPage) :
    Window(parent, {0, 0, LCD_W, LCD_H}),
    cancelHandler(std::move(cancelHandler)),
    selectHandler(std::move(selectHandler)),
    pageGroup(pageGroup), curPage(curPage)
{
  setWindowFlag(OPAQUE);

  // Save focus
  Layer::push(this);

  constexpr int maxMainBtns = 6;
  constexpr int maxSubBtns = 12;
#if LANDSCAPE
  int viewMainRows = 1;
  int viewMainCols = maxMainBtns;
  int viewSubRows = 2;
  int viewSubCols = maxMainBtns;
  int viewSubX = 0;
  int viewSubY = viewMainRows;
  int viewCols = maxMainBtns;
  int viewRows = viewMainRows + viewSubRows;
  lv_flex_flow_t mainFlow = LV_FLEX_FLOW_ROW;
#else
  int viewMainRows = maxMainBtns;
  int viewMainCols = 1;
  int viewSubRows = 4;
  int viewSubCols = 3;
  int viewSubX = viewMainCols;
  int viewSubY = 0;
  int viewCols = 4;
  int viewRows = maxMainBtns;
  lv_flex_flow_t mainFlow = LV_FLEX_FLOW_COLUMN;
#endif

  coord_t w = GRP_W(viewCols, 4);
  coord_t h = GRP_H(viewRows, 4);

  box = new Window(this, {(LCD_W - w) / 2, (LCD_H - h) / 2, w, h}, etx_modal_dialog_create);
  box->padAll(PAD_OUTLINE);

  mainMenu = new QuickMenuGroup(box,
          {0, 0, GRP_W(viewMainCols, 2), GRP_H(viewMainRows, 2)}, mainFlow);

  buildMainMenu(viewSubX, viewSubY, viewSubCols, viewSubRows);

  mainMenu->setGroup();
}

void QuickMenu::buildMainMenu(int viewSubX, int viewSubY, int viewSubCols, int viewSubRows)
{
  mainMenu->addButton(ICON_MODEL_SELECT, STR_MAIN_MENU_MANAGE_MODELS,
                      [=]() -> uint8_t {
                        onSelect(true);
                        new ModelLabelsWindow();
                        return 0;
                      });

  QuickSubMenu* sub;

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_MODEL, STR_MAIN_MENU_MODEL_SETTINGS,
                         QuickMenu::MODEL_FIRST, QuickMenu::MODEL_LAST, []() { return new ModelMenu(); },
                         modelMenuItems, viewSubX, viewSubY, viewSubCols, viewSubRows);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS,
                         QuickMenu::RADIO_FIRST, QuickMenu::RADIO_LAST, []() { return new RadioMenu(); },
                         radioMenuItems, viewSubX, viewSubY, viewSubCols, viewSubRows);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS,
                         QuickMenu::SCREENS_FIRST, QuickMenu::SCREENS_LAST, []() { return new ScreenMenu(); },
                         screensMenuItems, viewSubX, viewSubY, viewSubCols, viewSubRows);
  sub->addButton();
  subMenus.emplace_back(sub);

  sub = new QuickSubMenu(box, pageGroup, this, mainMenu, ICON_RADIO_TOOLS, STR_MAIN_MENU_TOOLS,
                         QuickMenu::TOOLS_FIRST, QuickMenu::TOOLS_LAST, []() { return new ToolsMenu(); },
                         toolsMenuItems, viewSubX, viewSubY, viewSubCols, viewSubRows);
  sub->addButton();
  subMenus.emplace_back(sub);

  // if (modelHasNotes())
  //   mainMenu->addButton(ICON_MODEL_NOTES, STR_MAIN_MENU_MODEL_NOTES,
  //                       [=]() -> uint8_t {
  //                         onSelect(true);
  //                         readModelNotes(true);
  //                         return 0;
  //                       });
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
