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

class SelectedTabIcon : public StaticIcon
{
 public:
  SelectedTabIcon(Window* parent) :
      StaticIcon(parent, 0, 0, ICON_CURRENTMENU_SHADOW, COLOR_THEME_PRIMARY1_INDEX)
  {
    new StaticIcon(this, 0, 0, ICON_CURRENTMENU_BG, COLOR_THEME_FOCUS_INDEX);
    new StaticIcon(this, SEL_DOT_X, SEL_DOT_Y, ICON_CURRENTMENU_DOT, COLOR_THEME_PRIMARY2_INDEX);
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SelectedTabIcon"; }
#endif

  static LAYOUT_VAL(SEL_DOT_X, 10, 10)
  static LAYOUT_VAL(SEL_DOT_Y, 39, 39)
};

class TabsGroupButton : public ButtonBase
{
 public:
  TabsGroupButton(Window* parent, const rect_t& rect, PageTab* page, int idx) :
      ButtonBase(parent, rect, nullptr, window_create), pageTab(page), index(idx)
  {
    lastIcon = pageTab->getIcon();
    icon = new StaticIcon(this, 2, ICON_Y, lastIcon, COLOR_THEME_PRIMARY2_INDEX);

    show(isVisible());
  }

  ~TabsGroupButton()
  {
    delete pageTab;
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override
  {
    return "TabsGroupButton(" + std::to_string(lastIcon) + ")";
  }
#endif

  bool isVisible() const { return pageTab->isVisible(); }

  PageTab* page() const { return pageTab; }
  int getIndex() const { return index; }
  void setIndex(int idx)
  {
    index = idx;
    pageTab->update(index);
  }

  static LAYOUT_VAL(ICON_Y, 7, 7)

 protected:
  PageTab* pageTab;
  EdgeTxIcon lastIcon;
  StaticIcon* icon = nullptr;
  int index;

  void checkEvents() override
  {
    show(isVisible());
    if (lastIcon != pageTab->getIcon()) {
      lastIcon = pageTab->getIcon();
      icon->setIcon(lastIcon);
    }
    ButtonBase::checkEvents();
  }
};

class TabsGroupHeader : public Window
{
 public:
  TabsGroupHeader(TabsGroup* menu, EdgeTxIcon icon) :
      Window(menu, {0, 0, LCD_W, TabsGroup::MENU_BODY_TOP}),
      menu(menu)
  {
    setWindowFlag(NO_FOCUS | OPAQUE);

    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

    new HeaderIcon(this, icon);

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

    carousel = new Window(this, 
                          {TopBar::MENU_HEADER_BUTTONS_LEFT, 0,
                           LCD_W - HDR_DATE_FULL_WIDTH - TopBar::MENU_HEADER_BUTTONS_LEFT, EdgeTxStyles::MENU_HEADER_HEIGHT + 10});
    carousel->padAll(PAD_ZERO);
    carousel->setWindowFlag(NO_FOCUS);

    selectedIcon = new SelectedTabIcon(carousel);

    new HeaderDateTime(this, LCD_W - DATE_XO, PAD_MEDIUM);
  }

  void setTitle(const char* title) { lv_label_set_text(titleLabel, title); }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TabsGroupHeader"; }
#endif

  void setCurrentIndex(uint8_t index)
  {
    if (index < buttons.size()) {
      buttons[currentIndex]->check(false);
      currentIndex = index;
      buttons[currentIndex]->check(true);
      coord_t x = getX(currentIndex);
      selectedIcon->setPos(x, 0);
      coord_t sx = lv_obj_get_scroll_x(carousel->getLvObj());
      if (x + MENU_HEADER_BUTTON_WIDTH - sx > carousel->width()) {
        lv_obj_scroll_to(carousel->getLvObj(), x + MENU_HEADER_BUTTON_WIDTH - carousel->width(), 0, LV_ANIM_OFF);
      } else if (x < sx) {
        lv_obj_scroll_to(carousel->getLvObj(), x, 0, LV_ANIM_OFF);
      }
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
    TabsGroupButton* btn = new TabsGroupButton(
        carousel, {getX(idx), 0, MENU_HEADER_BUTTON_WIDTH + 3, TabsGroup::MENU_TITLE_TOP + 5}, page, idx);
    btn->setPressHandler([=]() {
      menu->setCurrentTab(btn->getIndex());
      return true;
    });
    btn->show(btn->isVisible());
    buttons.emplace_back(btn);
  }

  void removeTab(unsigned index)
  {
    auto btn = buttons[index];
    buttons.erase(buttons.begin() + index);
    btn->deleteLater();
    updateLayout();
  }

  void updateLayout()
  {
    for (uint8_t i = 0; i < buttons.size(); i += 1) {
      buttons[i]->setIndex(i);
      buttons[i]->setPos(getX(i), 0);
    }
  }

  PageTab* pageTab(uint8_t idx) const { return buttons[idx]->page(); }
  bool isCurrent(uint8_t idx) const { return currentIndex == idx; }
  uint8_t tabCount() const { return buttons.size(); }

  static LAYOUT_VAL(DATE_XO, 48, 48)
  static LAYOUT_VAL(MENU_HEADER_BUTTON_WIDTH, 33, 33)
  static LAYOUT_VAL(HDR_DATE_FULL_WIDTH, 51, 51)

 protected:
  uint8_t currentIndex = 0;
  TabsGroup* menu;
  lv_obj_t* titleLabel = nullptr;
  SelectedTabIcon* selectedIcon = nullptr;
  Window* carousel = nullptr;
  std::vector<TabsGroupButton*> buttons;

  coord_t getX(uint8_t idx)
  {
    coord_t x = 0;
    for (uint8_t i = 0; i < idx; i += 1)
      if (buttons[i]->isVisible())
        x += MENU_HEADER_BUTTON_WIDTH;
    return x;
  }

  void checkEvents() override
  {
    updateLayout();
    Window::checkEvents();
  }
};

TabsGroup::TabsGroup(EdgeTxIcon icon) :
    NavWindow(MainWindow::instance(), {0, 0, LCD_W, LCD_H})
{
  etx_solid_bg(lvobj);

  header = new TabsGroupHeader(this, icon);

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
  addBackButton();
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

void TabsGroup::deleteLater(bool detach, bool trash)
{
  if (_deleted) return;

  Layer::pop(this);
  Layer::back()->show();

  Window::deleteLater(detach, trash);
}

void TabsGroup::addTab(PageTab* page)
{
  header->addTab(page);
  if (!currentTab) {
    setCurrentTab(0);
  }
}

void TabsGroup::removeTab(unsigned index)
{
  if (header->isCurrent(index))
    setCurrentTab(max<unsigned>(0, index - 1));
  header->removeTab(index);
}

void TabsGroup::checkEvents()
{
  Window::checkEvents();
  if (currentTab) {
    currentTab->checkEvents();
  }
  ViewMain::instance()->runBackground();
}

#if defined(HARDWARE_KEYS)
void TabsGroup::onPressPGUP() { header->prevTab(); }
void TabsGroup::onPressPGDN() { header->nextTab(); }
#endif

void TabsGroup::onClicked() { Keyboard::hide(false); }

void TabsGroup::onCancel() { deleteLater(); }
