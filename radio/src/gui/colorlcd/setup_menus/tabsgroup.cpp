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

#include "tabsgroup.h"

#include "theme_manager.h"
#include "etx_lv_theme.h"
#include "view_main.h"
#include "topbar_impl.h"
#include "page.h"

#if defined(HARDWARE_TOUCH)
#include "keyboard_base.h"
#endif

#if defined(DEBUG)
static uint32_t dsms, dems, end_ms, start_ms;
static bool timepg = false;

static void on_draw_begin(lv_event_t* e)
{
  if (timepg) {
    dsms = lv_tick_get();
  }
}
static void on_draw_end(lv_event_t* e)
{
  timepg = false;
  dems = lv_tick_get();
  TRACE("tab time: build %ld layout %ld draw %ld total %ld",
        end_ms - start_ms, dsms - end_ms, dems - dsms, dems - start_ms);
}
#endif

class TabsGroupHeader : public Window
{
 public:
  TabsGroupHeader(TabsGroup* menu, EdgeTxIcon icon, const char* name) :
      Window(menu, {0, 0, LCD_W, TabsGroup::MENU_BODY_TOP}),
      menu(menu)
  {
    setWindowFlag(NO_FOCUS | OPAQUE);

    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

    new HeaderIcon(this, icon);

#if PORTRAIT
    nameLabel = lv_label_create(lvobj);
    etx_txt_color(nameLabel, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_pos(nameLabel, PageGroup::MENU_TITLE_TOP + PAD_LARGE, PAD_MEDIUM * 2);
    lv_obj_set_size(nameLabel, LCD_W - PageGroup::MENU_TITLE_TOP * 2 - PAD_LARGE * 2, PageGroup::MENU_TITLE_TOP - PAD_MEDIUM * 2);
    lv_label_set_text(nameLabel, name);

    auto sep = lv_obj_create(lvobj);
    etx_solid_bg(sep);
    lv_obj_set_pos(sep, 0, EdgeTxStyles::MENU_HEADER_HEIGHT);
    lv_obj_set_size(sep, LCD_W, TabsGroup::MENU_TITLE_TOP - EdgeTxStyles::MENU_HEADER_HEIGHT);

    titleLabel = lv_label_create(lvobj);
    etx_txt_color(titleLabel, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_style_pad_left(titleLabel, PAD_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_pad_top(titleLabel, 1, LV_PART_MAIN);
    lv_obj_set_pos(titleLabel, 0, TabsGroup::MENU_TITLE_TOP);
    lv_obj_set_size(titleLabel, LCD_W, TabsGroup::MENU_TITLE_HEIGHT);
    setTitle("");
#else
    nameLabel = lv_label_create(lvobj);
    etx_txt_color(nameLabel, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_pos(nameLabel, PageGroup::MENU_TITLE_TOP + PAD_LARGE, PageHeader::PAGE_TITLE_TOP);
    lv_obj_set_size(nameLabel, LCD_W - PageGroup::MENU_TITLE_TOP * 4 - PAD_LARGE * 2, EdgeTxStyles::STD_FONT_HEIGHT);
    lv_label_set_text(nameLabel, name);

    titleLabel = lv_label_create(lvobj);
    etx_txt_color(titleLabel, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_pos(titleLabel, PageGroup::MENU_TITLE_TOP + PAD_LARGE, PageHeader::PAGE_TITLE_TOP + EdgeTxStyles::STD_FONT_HEIGHT);
    lv_obj_set_size(titleLabel, LCD_W - PageGroup::MENU_TITLE_TOP * 4 - PAD_LARGE * 2, EdgeTxStyles::STD_FONT_HEIGHT);
    setTitle("");
#endif

    new HeaderBackIcon(this);

    new IconButton(this, ICON_BTN_PREV, LCD_W - PageGroup::MENU_TITLE_TOP * 3, PAD_MEDIUM, [=]() {
      prevTab();
      return 0;
    });

    new IconButton(this, ICON_BTN_NEXT, LCD_W - PageGroup::MENU_TITLE_TOP * 2, PAD_MEDIUM, [=]() {
      nextTab();
      return 0;
    });
  }

  void setTitle(const char* title) { lv_label_set_text(titleLabel, title); }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TabsGroupHeader"; }
#endif

  void setCurrentIndex(uint8_t index)
  {
    currentIndex = index;
  }

  void chgTab(int dir)
  {
    int idx = currentIndex;
    idx += dir;
    if (idx < 0) idx = pages.size() - 1;
    else if (idx >= (int)pages.size()) idx = 0;
    menu->setCurrentTab(idx);
  }

  void nextTab() { chgTab(1); }
  void prevTab() { chgTab(-1); }

  void addTab(PageTab* page)
  {
    pages.emplace_back(page);
  }

  PageTab* pageTab(uint8_t idx) const { return pages[idx]; }
  bool isCurrent(uint8_t idx) const { return currentIndex == idx; }
  uint8_t tabCount() const { return pages.size(); }

 protected:
  uint8_t currentIndex = 0;
  TabsGroup* menu;
  lv_obj_t* nameLabel = nullptr;
  lv_obj_t* titleLabel = nullptr;
  std::vector<PageTab*> pages;
};

TabsGroup::TabsGroup(EdgeTxIcon icon, const char* name) :
    PageGroupBase()
{
  etx_solid_bg(lvobj);

  header = new TabsGroupHeader(this, icon, name);

  body = new Window(this, {0, MENU_BODY_TOP, LCD_W, MENU_BODY_HEIGHT});
  body->setWindowFlag(NO_FOCUS);
  lv_obj_set_style_max_height(body->getLvObj(), MENU_BODY_HEIGHT, LV_PART_MAIN);
  etx_scrollbar(body->getLvObj());

  Layer::back()->hide();
  Layer::push(this);

#if defined(DEBUG)
  lv_obj_add_event_cb(lvobj, on_draw_begin, LV_EVENT_COVER_CHECK, nullptr);
  lv_obj_add_event_cb(lvobj, on_draw_end, LV_EVENT_DRAW_POST_END, nullptr);
#endif

#if defined(HARDWARE_TOUCH)
  addCustomButton(0, 0, [=]() { openMenu(); });
  addCustomButton(LCD_W - EdgeTxStyles::MENU_HEADER_HEIGHT, 0, [=]() { onCancel(); });
#endif
}

uint8_t TabsGroup::tabCount() const { return header->tabCount(); }

void TabsGroup::setCurrentTab(unsigned index)
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
    start_ms = lv_tick_get();
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
    end_ms = lv_tick_get();
#endif
  }
}

void TabsGroup::addTab(PageTab* page)
{
  header->addTab(page);
  if (!currentTab) {
    setCurrentTab(0);
  }
}

void TabsGroup::openMenu()
{
  PageGroup* p = nullptr;
  QuickMenu::SubMenu subMenu = QuickMenu::NONE;
  Window* w = Layer::walk([=](Window *w) mutable -> bool {
    return w->isPageGroup();
  });
  if (w) {
    p = (PageGroup*)w;
    subMenu = p->getCurrentTab()->subMenu();
  }
  quickMenu = new QuickMenu(this, [=]() { quickMenu = nullptr; },
    [=](bool close) {
      onCancel();
      if (p) {
        while (!Layer::back()->isPageGroup()) {
          Layer::back()->deleteLater();
        }
        if (close)
          Layer::back()->onCancel();
      }
    }, p, subMenu);
  quickMenu->setFocus(subMenu);
}

#if defined(HARDWARE_KEYS)
void TabsGroup::onPressPGUP() { header->prevTab(); }
void TabsGroup::onPressPGDN() { header->nextTab(); }
void TabsGroup::onLongPressPGUP() { header->prevTab(); }
void TabsGroup::onLongPressPGDN() { header->nextTab(); }
void TabsGroup::onLongPressRTN() { onCancel(); }
#endif
