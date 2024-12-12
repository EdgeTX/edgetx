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

#include "pagegroup.h"

#include "tabsgroup.h"
#include "theme_manager.h"
#include "etx_lv_theme.h"
#include "view_main.h"
#include "topbar_impl.h"
#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "menu_channels.h"
#include "model_select.h"

#if defined(HARDWARE_TOUCH)
#include "keyboard_base.h"
#endif

#if defined(DEBUG)
static uint32_t dsms, dems, end_ms, start_ms;
static bool timepg = false;

static void on_draw_begin(lv_event_t* e)
{
  if (timepg) {
    dsms = RTOS_GET_MS();
  }
}
static void on_draw_end(lv_event_t* e)
{
  timepg = false;
  dems = RTOS_GET_MS();
  TRACE("tab time: build %ld layout %ld draw %ld total %ld",
        end_ms - start_ms, dsms - end_ms, dems - dsms, dems - start_ms);
}
#endif

class PageGroupButton
{
 public:
  PageGroupButton(PageTab* page, int idx) : pageTab(page), index(idx)
  {
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "PageGroupButton"; }
#endif

  bool isVisible() const { return pageTab->isVisible(); }

  PageTab* page() const { return pageTab; }
  int getIndex() const { return index; }
  void setIndex(int idx)
  {
    index = idx;
    pageTab->update(index);
  }

 protected:
  PageTab* pageTab;
  int index;
};

class PageGroupHeader : public Window
{
 public:
  PageGroupHeader(PageGroup* menu, EdgeTxIcon icon) :
      Window(menu, {0, 0, LCD_W, PageGroup::MENU_TITLE_TOP}),
      menu(menu)
  {
    padAll(PAD_ZERO);

    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

    new HeaderIcon(this, icon, [=]() { menu->openMenu(); });

    titleLabel = lv_label_create(lvobj);
    etx_txt_color(titleLabel, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_pos(titleLabel, PageGroup::MENU_TITLE_TOP + PAD_LARGE, PAD_MEDIUM * 2);
    lv_obj_set_size(titleLabel, LCD_W - PageGroup::MENU_TITLE_TOP * 2 - PAD_LARGE * 2, PageGroup::MENU_TITLE_TOP);
    setTitle("");

    new HeaderBackIcon(this, [=]() { menu->onCancel(); });
  }

  void setTitle(const char* title) { if (titleLabel) lv_label_set_text(titleLabel, title); }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "PageGroupHeader"; }
#endif

  void setCurrentIndex(uint8_t index)
  {
    if (index < buttons.size()) {
      currentIndex = index;
    }
  }

  void chgTab(int dir)
  {
    int idx = currentIndex;
    do {
      idx += dir;
      if (idx < 0) idx = buttons.size() - 1;
      if (idx >= (int)buttons.size()) idx = 0;
    } while (!buttons[idx]->isVisible());
    menu->setCurrentTab(idx);
  }

  void nextTab() { chgTab(1); }
  void prevTab() { chgTab(-1); }

  void addTab(PageTab* page)
  {
    uint8_t idx = buttons.size();
    PageGroupButton* btn = new PageGroupButton(page, idx);
    buttons.emplace_back(btn);
  }

  void removeTab(unsigned index)
  {
    auto btn = buttons[index];
    buttons.erase(buttons.begin() + index);
    delete btn;
    updateLayout();
  }

  void updateLayout()
  {
    for (uint8_t i = 0; i < buttons.size(); i += 1) {
      buttons[i]->setIndex(i);
    }
  }

  PageTab* pageTab(uint8_t idx) const { return buttons[idx]->page(); }
  bool isCurrent(uint8_t idx) const { return currentIndex == idx; }
  uint8_t tabCount() const { return buttons.size(); }

 protected:
  uint8_t currentIndex = 0;
  PageGroup* menu;
  lv_obj_t* titleLabel = nullptr;
  std::vector<PageGroupButton*> buttons;

  void checkEvents() override
  {
    updateLayout();
    Window::checkEvents();
  }
};

PageGroup::PageGroup(EdgeTxIcon icon, PageDef* pages) :
    NavWindow(MainWindow::instance(), {0, 0, LCD_W, LCD_H}),
    icon(icon)
{
  etx_solid_bg(lvobj);

  Layer::back()->hide();
  Layer::push(this);

  header = new PageGroupHeader(this, icon);

  body = new Window(this, {0, MENU_TITLE_TOP, LCD_W, MENU_BODY_HEIGHT});
  body->setWindowFlag(NO_FOCUS);
  lv_obj_set_style_max_height(body->getLvObj(), MENU_BODY_HEIGHT, LV_PART_MAIN);
  etx_scrollbar(body->getLvObj());

#if defined(DEBUG)
  lv_obj_add_event_cb(lvobj, on_draw_begin, LV_EVENT_COVER_CHECK, nullptr);
  lv_obj_add_event_cb(lvobj, on_draw_end, LV_EVENT_DRAW_POST_END, nullptr);
#endif

  for (int i = 0; pages[i].icon < EDGETX_ICONS_COUNT; i += 1) {
    if (!pages[i].enabled || pages[i].enabled())
      addTab(pages[i].createPage(pages[i]));
  }
}

uint8_t PageGroup::tabCount() const { return header->tabCount(); }

void PageGroup::setCurrentTab(unsigned index)
{
  if (deleted()) return;

  header->setCurrentIndex(index);

  PageTab* tab = header->pageTab(index);

  if (tab != currentTab && !deleted()) {
    header->setTitle(tab->getTitle().c_str());

    lv_obj_enable_style_refresh(false);

    body->clear();
    if (currentTab)
      currentTab->cleanup();
    currentTab = tab;

#if defined(DEBUG)
    start_ms = RTOS_GET_MS();
    timepg = true;
#endif

    static lv_style_prop_t remStyles[] = {
        LV_STYLE_FLEX_FLOW,  LV_STYLE_LAYOUT,    LV_STYLE_PAD_ROW,
        LV_STYLE_PAD_COLUMN, LV_STYLE_PAD_LEFT,  LV_STYLE_PAD_RIGHT,
        LV_STYLE_PAD_TOP,    LV_STYLE_PAD_BOTTOM};
    for (uint8_t i = 0; i < DIM(remStyles); i += 1)
      lv_obj_remove_local_style_prop(body->getLvObj(), remStyles[i],
                                     LV_PART_MAIN);

    body->padAll(tab->getPadding());

    tab->build(body);

    lv_obj_enable_style_refresh(true);
    lv_obj_refresh_style(body->getLvObj(), LV_PART_ANY, LV_STYLE_PROP_ANY);

#if defined(DEBUG)
    end_ms = RTOS_GET_MS();
#endif
  }
}

void PageGroup::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;

  Layer::pop(this);
  Layer::back()->show();

  Window::deleteLater(detach, trash);
}

void PageGroup::addTab(PageTab* page)
{
  header->addTab(page);
  if (!currentTab) {
    setCurrentTab(0);
  }
}

void PageGroup::removeTab(unsigned index)
{
  if (header->isCurrent(index))
    setCurrentTab(max<unsigned>(0, index - 1));
  header->removeTab(index);
}

void PageGroup::checkEvents()
{
  Window::checkEvents();
  if (currentTab) {
    currentTab->checkEvents();
  }
  ViewMain::instance()->runBackground();
}

void PageGroup::openMenu()
{
  quickMenu = new QuickMenu(this, [=]() { quickMenu = nullptr; },
    [=](bool close) {
      if (close)
        onCancel();
    }, this, currentTab->subMenu());
  quickMenu->setFocus(currentTab->subMenu());
}

void PageGroup::onClicked() { Keyboard::hide(false); }

void PageGroup::onCancel()
{
  if (quickMenu) quickMenu->closeMenu();
  quickMenu = nullptr;
  deleteLater();
}

#if defined(HARDWARE_KEYS)
void PageGroup::onPressSYS()
{
  if (!quickMenu) openMenu();
}

void PageGroup::onLongPressSYS()
{
  if (icon == ICON_RADIO) {
    setCurrentTab(2);
  } else {
    onCancel();
    (new RadioMenu())->setCurrentTab(2);
  }
}

void PageGroup::onPressMDL()
{
  if (icon != ICON_MODEL) {
    onCancel();
    new ModelMenu();
  }
}

void PageGroup::onLongPressMDL()
{
  onCancel();
  new ModelLabelsWindow();
}

void PageGroup::onPressTELE()
{
  if (icon != ICON_THEME) {
    onCancel();
    (new ScreenMenu())->setCurrentTab(ViewMain::instance()->getCurrentMainView() + 1);
  }
}

void PageGroup::onLongPressTELE()
{
  if (icon != ICON_MONITOR) {
    onCancel();
    new ChannelsViewMenu();
  }
}

void PageGroup::onPressPGUP()
{
  header->prevTab();
}

void PageGroup::onPressPGDN()
{
  header->nextTab();
}
#endif
