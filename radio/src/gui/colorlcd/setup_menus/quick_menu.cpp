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

#include "edgetx.h"
#include "etx_lv_theme.h"
#include "mainwindow.h"
#include "model_select.h"
#include "quick_menu_group.h"
#include "screen_setup.h"
#include "theme_manager.h"
#include "view_channels.h"
#include "view_main.h"

//-----------------------------------------------------------------------------

#if VERSION_MAJOR > 2
class QuickSubMenu
{
 public:
  QuickSubMenu(Window* parent, QuickMenu* quickMenu,
               EdgeTxIcon icon, const char* title, const char* parentTitle,
               const PageDef* items):
    parent(parent), quickMenu(quickMenu),
    icon(icon), title(title), parentTitle(parentTitle), items(items)
  {}

  bool isSubMenu(QMPage n)
  {
    for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
      if (items[i].qmPage == n) return true;
    return false;
  }

  bool isSubMenu(QMPage n, EdgeTxIcon curIcon)
  {
    if ((icon != EDGETX_ICONS_COUNT) && (icon != curIcon)) return false;
    return isSubMenu(n);
  }

  bool isSubMenu(ButtonBase* b)
  {
    for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
      if (menuButton == b) return true;
    return false;
  }

  int getIndex(QMPage n)
  {
    for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1)
      if (items[i].qmPage == n) return i;
    return -1;
  }

  ButtonBase* addButton()
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

  void enableSubMenu()
  {
    subMenu->activate();
    quickMenu->enableSubMenu();
  }

  void setDisabled(bool all)
  {
    if (subMenu)
      subMenu->setDisabled(all);
  }

  void setCurrent(QMPage n)
  {
    if (!subMenu) buildSubMenu();
    quickMenu->getTopMenu()->setCurrent(menuButton);
    quickMenu->getTopMenu()->setDisabled(false);
    subMenu->setCurrent(getIndex(n));
    enableSubMenu();
  }

  void activate()
  {
    if (!subMenu) buildSubMenu();
    quickMenu->getTopMenu()->setCurrent(menuButton);
    quickMenu->getTopMenu()->setDisabled(false);
    quickMenu->getTopMenu()->clearFocus();
    enableSubMenu();
  }

  void buildSubMenu()
  {
    subMenu = new QuickMenuGroup(parent);

    for (int i = 0; items[i].icon < EDGETX_ICONS_COUNT; i += 1) {
      subMenu->addButton(items[i].icon, STR_VAL(items[i].qmTitle),
          std::bind(&QuickSubMenu::onPress, this, i),
          items[i].enabled,
          [=]() { QuickMenu::setCurrentPage(items[i].qmPage, icon); });
    }

    doLayout();
    subMenu->hide();
    subMenu->setDisabled(true);
  }

  uint8_t onPress(int n)
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

  void onSelect(bool close)
  {
    quickMenu->onSelect(close);
  }

  int getPageNumber(int iconNumber)
  {
    int pageNumber = 0;
    for (int i = 0; i < iconNumber; i += 1)
      if (items[i].pageAction == PAGE_CREATE)
        pageNumber += 1;
    return pageNumber;
  }

  void doLayout()
  {
    if (subMenu)
      subMenu->doLayout(QuickMenu::QM_SUB_COLS);
  }

 protected:
  Window* parent;
  QuickMenu* quickMenu;
  EdgeTxIcon icon;
  const char* title;
  const char* parentTitle;
  const PageDef* items;
  QuickMenuGroup* subMenu = nullptr;
  ButtonBase* menuButton = nullptr;
};
#endif

//-----------------------------------------------------------------------------

QuickMenu* QuickMenu::instance = nullptr;
QMPage QuickMenu::curPage = QM_NONE;
EdgeTxIcon QuickMenu::curIcon = EDGETX_ICONS_COUNT;

QuickMenu* QuickMenu::openQuickMenu(std::function<void(bool close)> selectHandler,
            PageGroupBase* pageGroup, QMPage curPage)
{
  if (!instance) {
    instance = new QuickMenu();
  }
  instance->openQM(selectHandler, pageGroup, curPage);
  return instance;
}

void QuickMenu::closeQuickMenu()
{
  if (instance)
    instance->closeQM();
}

bool QuickMenu::isOpen()
{
  return instance && instance->isVisible();
}

void QuickMenu::shutdownQuickMenu()
{
  if (instance) instance->deleteLater();
  instance = nullptr;
}

QuickMenu::QuickMenu() :
    NavWindow(MainWindow::instance(), {QM_X, QM_Y, QM_W, QM_H})
{
  setWindowFlag(OPAQUE);

  etx_obj_add_style(lvobj, styles->bg_opacity_90, LV_PART_MAIN);
  etx_bg_color(lvobj, COLOR_THEME_QM_BG_INDEX);

  auto sep = lv_obj_create(lvobj);
  etx_solid_bg(sep, COLOR_THEME_QM_FG_INDEX);
  lv_obj_set_size(sep, QM_W, PAD_THREE);

  auto mask = getBuiltinIcon(ICON_TOP_LOGO);
  new StaticIcon(this, (QM_W - mask->width) / 2, 0, ICON_TOP_LOGO, COLOR_THEME_QM_FG_INDEX);

  new ButtonBase(
    this, {0, 0, QM_W, EdgeTxStyles::UI_ELEMENT_HEIGHT},
    [=]() -> uint8_t {
      inSubMenu = false;
      onCancel();
      return 0;
    },
    window_create);

  auto box = new Window(this, {QM_MAIN_X, QM_MAIN_Y, QM_MAIN_W, QM_MAIN_H});

  mainMenu = new QuickMenuGroup(box);

#if VERSION_MAJOR > 2
  box = new Window(this, {QM_SUB_X, QM_SUB_Y, QM_SUB_W, QM_SUB_H});

  int f = 0;
  for (int i = 0; i < MAX_QM_FAVORITES; i += 1) {
    if (g_eeGeneral.qmFavorites[i].shortcut != QM_NONE) {
      setupFavorite((QMPage)g_eeGeneral.qmFavorites[i].shortcut, f);
      f += 1;
    }
  }
  favoritesMenuItems[f].icon = EDGETX_ICONS_COUNT;
#endif

  for (int i = 0; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if ((qmTopItems[i].enabled == nullptr) || qmTopItems[i].enabled()) {
      if (qmTopItems[i].pageAction == QM_ACTION) {
        mainMenu->addButton(qmTopItems[i].icon, STR_VAL(qmTopItems[i].qmTitle), [=]() { onSelect(true); qmTopItems[i].action(); });
#if VERSION_MAJOR > 2
      } else {
        auto sub = new QuickSubMenu(box, this, qmTopItems[i].icon, STR_VAL(qmTopItems[i].qmTitle), STR_VAL(qmTopItems[i].title), qmTopItems[i].subMenuItems);
        sub->addButton();
        subMenus.emplace_back(sub);
#endif
      }
    }
  }
}

void QuickMenu::deleteLater()
{
  if (_deleted) return;

  instance = nullptr;

  NavWindow::deleteLater();
}

void QuickMenu::openQM(std::function<void(bool close)> selectHandler,
            PageGroupBase* newPageGroup, QMPage newCurPage)
{
  pushLayer();

#if VERSION_MAJOR == 2
  curPage = QM_NONE;
#endif

  mainMenu->doLayout(QM_MAIN_COLS);
#if VERSION_MAJOR > 2
  for (size_t i = 0; i < subMenus.size(); i += 1)
    subMenus[i]->doLayout();
#endif

  this->selectHandler = std::move(selectHandler);
  show();
  lv_obj_move_foreground(lvobj);
  if (newPageGroup) {
    pageGroup = newPageGroup;
    curPage = newCurPage;
    curIcon = pageGroup->getIcon();
    mainMenu->setDisabled(false);
    mainMenu->clearFocus();
    setFocus(curPage);
  } else {
    pageGroup = nullptr;
    mainMenu->clearFocus();
    if (curPage != QM_MANAGE_MODELS && curPage != QM_NONE) {
      mainMenu->setDisabled(false);
      setFocus(curPage);
    } else {
#if VERSION_MAJOR > 2
      if (curPage == QM_MANAGE_MODELS)
        mainMenu->setCurrent(favoritesMenuItems[0].icon == EDGETX_ICONS_COUNT ? 0 : 1);
#endif
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
  for (int i = FIRST_SEARCH_IDX; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      if (qmTopItems[i].qmPage == page) {
        QuickMenu::selected();
        setCurrentPage(page, qmTopItems[i].icon);
        qmTopItems[i].action();
        return;
        }
    } else {
      const PageDef* sub = qmTopItems[i].subMenuItems;
      for (int j = 0, k = 0; sub[j].icon != EDGETX_ICONS_COUNT; j += 1) {
        if (sub[j].qmPage == page) {
          if (sub[j].pageAction == PAGE_ACTION) {
            QuickMenu::selected();
            setCurrentPage(page, qmTopItems[i].icon);
            sub[j].action();
          } else {
            QuickMenu::selected();
            auto pg = new PageGroup(qmTopItems[i].icon, STR_VAL(qmTopItems[i].title), sub);
            pg->setCurrentTab(k);
            return;
          }
        } else if (sub[j].pageAction == PAGE_CREATE) {
          k += 1;
        }
      }
    }
  }
}

EdgeTxIcon QuickMenu::subMenuIcon(QMPage page)
{
  for (int i = FIRST_SEARCH_IDX; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      if (qmTopItems[i].qmPage == page) {
        return qmTopItems[i].icon;
        }
    } else {
      const PageDef* sub = qmTopItems[i].subMenuItems;
      for (int j = 0; sub[j].icon != EDGETX_ICONS_COUNT; j += 1) {
        if (sub[j].qmPage == page) {
          return qmTopItems[i].icon;
        }
      }
    }
  }
  return EDGETX_ICONS_COUNT;
}

int QuickMenu::pageIndex(QMPage page)
{
  for (int i = FIRST_SEARCH_IDX; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      if (qmTopItems[i].qmPage == page) {
        return 0;
        }
    } else {
      const PageDef* sub = qmTopItems[i].subMenuItems;
      for (int j = 0, k = 0; sub[j].icon != EDGETX_ICONS_COUNT; j += 1) {
        if (sub[j].qmPage == page) {
          if (sub[j].pageAction == PAGE_CREATE)
            return k;
          else
            return 0;
        } else if (sub[j].pageAction == PAGE_CREATE) {
          k += 1;
        }
      }
    }
  }
  return 0;
}

std::string replaceAll(std::string str, const std::string& from, const std::string& to)
{
    auto&& pos = str.find(from, size_t{});
    while (pos != std::string::npos)
    {
        str.replace(pos, from.length(), to);
        // easy to forget to add to.length()
        pos = str.find(from, pos + to.length());
    }
    return str;
}

std::vector<std::string> QuickMenu::menuPageNames(bool forFavorites)
{
  std::vector<std::string> qmPages;

  qmPages.emplace_back(STR_NONE);
  qmPages.emplace_back(STR_OPEN_QUICK_MENU);

  for (int i = FIRST_SEARCH_IDX; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      qmPages.emplace_back(STR_VAL(qmTopItems[i].title));
    } else {
      const PageDef* sub = qmTopItems[i].subMenuItems;
      for (int j = 0; sub[j].icon != EDGETX_ICONS_COUNT; j += 1) {
        std::string s(STR_VAL(qmTopItems[i].title));
        s += " - ";
        if (!forFavorites && sub[j].qmPage >= QM_UI_SCREEN1 && sub[j].qmPage <= QM_UI_SCREEN10)
          s += STR_CURRENT_SCREEN;
        else
          s += STR_VAL(sub[j].title);
        s = replaceAll(s, "\n", " ");
        qmPages.emplace_back(s);
      }
    }
  }

  return qmPages;
}

#if VERSION_MAJOR > 2
void QuickMenu::setupFavorite(QMPage page, int f)
{
  PageDef& fav = favoritesMenuItems[f];

  for (int i = FIRST_SEARCH_IDX; qmTopItems[i].icon != EDGETX_ICONS_COUNT; i += 1) {
    if (qmTopItems[i].pageAction == QM_ACTION) {
      if (qmTopItems[i].qmPage == page) {
          fav.icon = qmTopItems[i].icon;
          fav.qmTitle = qmTopItems[i].qmTitle;
          fav.title = qmTopItems[i].title;
          fav.pageAction = PAGE_ACTION;
          fav.qmPage = qmTopItems[i].qmPage;
          fav.create = nullptr;
          fav.enabled = nullptr;
          fav.action = qmTopItems[i].action;
        return;
        }
    } else {
      const PageDef* sub = qmTopItems[i].subMenuItems;
      for (int j = 0; sub[j].icon != EDGETX_ICONS_COUNT; j += 1) {
        if (sub[j].qmPage == page) {
          fav.icon = sub[j].icon;
          fav.qmTitle = sub[j].qmTitle;
          fav.title = sub[j].title;
          fav.pageAction = sub[j].pageAction;
          fav.qmPage = sub[j].qmPage;
          fav.create = sub[j].create;
          fav.enabled = sub[j].enabled;
          fav.action = sub[j].action;
          return;
        }
      }
    }
  }
}
#endif

void QuickMenu::setCurrentPage(QMPage newPage, EdgeTxIcon newIcon)
{
  curPage = newPage;
  curIcon = newIcon;
}

void QuickMenu::focusMainMenu()
{
  inSubMenu = false;
  mainMenu->activate();
#if VERSION_MAJOR > 2
  for(auto sub : subMenus)
    sub->setDisabled(true);
#endif
}

void QuickMenu::onSelect(bool close)
{
  if (selectHandler) selectHandler(close);
  selectHandler = nullptr;
  closeQM();
}

void QuickMenu::closeQM()
{
  if (isVisible()) {
    popLayer();
    hide();
    selectHandler = nullptr;
  }
}

void QuickMenu::onCancel()
{
  if (inSubMenu) {
    focusMainMenu();
    curPage = QM_NONE;
  } else {
    closeQM();
  }
}

void QuickMenu::setFocus(QMPage selection)
{
#if VERSION_MAJOR > 2
  for(auto sub : subMenus) {
    if (sub->isSubMenu(selection, curIcon)) {
      sub->setCurrent(selection);
      return;
    }
  }
#endif
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
    closeQM();
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
void QuickMenu::onLongPressRTN() { closeQM(); }

void QuickMenu::afterPG()
{
  auto b = mainMenu->getFocusedButton();
  if (b) {
#if VERSION_MAJOR > 3
    for(auto sub : subMenus) {
      if (sub->isSubMenu(b)) {
        sub->activate();
        return;
      }
    }
#endif
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
