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

#include "theme.h"
#include "themes/etx_lv_theme.h"
#include "view_main.h"

#if defined(HARDWARE_TOUCH)
#include "keyboard_base.h"
#endif

class SelectedTabIcon : public StaticIcon
{
 public:
  SelectedTabIcon(Window* parent) :
      StaticIcon(parent, 0, 0, ICON_CURRENTMENU_SHADOW, COLOR_THEME_PRIMARY1)
  {
    new StaticIcon(this, 0, 0, ICON_CURRENTMENU_BG, COLOR_THEME_FOCUS);
    new StaticIcon(this, 10, 39, ICON_CURRENTMENU_DOT, COLOR_THEME_PRIMARY2);
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "SelectedTabIcon"; }
#endif
};

class TabCarouselButton : public ButtonBase
{
 public:
  TabCarouselButton(Window* parent, const rect_t& rect, PageTab* page) :
      ButtonBase(parent, rect, nullptr, window_create), pageTab(page)
  {
    selected = new SelectedTabIcon(this);
    selected->hide();

    lastIcon = getIcon();
    icon = new StaticIcon(this, 2, 7, lastIcon, COLOR_THEME_PRIMARY2);

    show(isVisible());
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override
  {
    return "TabCarouselButton(" + std::to_string(getIcon()) + ")";
  }
#endif

  bool isVisible() const { return pageTab->isVisible(); }

  EdgeTxIcon getIcon() const { return pageTab->getIcon(); }
  PageTab* page() const { return pageTab; }

 protected:
  PageTab* pageTab;
  EdgeTxIcon lastIcon;
  SelectedTabIcon* selected = nullptr;
  StaticIcon* icon = nullptr;

  void checkEvents() override
  {
    selected->show(checked());
    show(isVisible());
    if (lastIcon != getIcon()) {
      lastIcon = getIcon();
      icon->deleteLater();
      icon = new StaticIcon(this, 2, 7, lastIcon, COLOR_THEME_PRIMARY2);
    }
    ButtonBase::checkEvents();
  }
};

class TabsCarousel : public Window
{
 public:
  TabsCarousel(Window* parent, TabsGroup* menu) :
      Window(parent,
             {MENU_HEADER_BUTTONS_LEFT, 0,
              LCD_W - 51 - MENU_HEADER_BUTTONS_LEFT, MENU_HEADER_HEIGHT + 10}),
      menu(menu)
  {
    setWindowFlag(NO_FOCUS);

    lv_obj_set_style_max_width(lvobj, LCD_W - 51 - MENU_HEADER_BUTTONS_LEFT,
                               LV_PART_MAIN);

    padAll(PAD_ZERO);
    setFlexLayout(LV_FLEX_FLOW_ROW, PAD_ZERO);
    lv_obj_set_flex_align(lvobj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER,
                          LV_FLEX_ALIGN_CENTER);

    setCurrentIndex(0);
  }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TabsCarousel"; }
#endif

  void setCurrentIndex(uint8_t index)
  {
    if (index < buttons.size()) {
      buttons[currentIndex]->check(false);
      currentIndex = index;
      buttons[currentIndex]->check(true);
    }
  }

  void nextTab()
  {
    uint8_t idx = currentIndex;
    do {
      idx += 1;
      if (idx >= buttons.size()) idx = 0;
    } while (!buttons[idx]->isVisible());
    menu->setCurrentTab(idx);
  }

  void prevTab()
  {
    uint8_t idx = currentIndex;
    do {
      if (idx == 0) idx = buttons.size();
      idx -= 1;
    } while (!buttons[idx]->isVisible());
    menu->setCurrentTab(idx);
  }

  void addTab(PageTab* page)
  {
    TabCarouselButton* btn = new TabCarouselButton(
        this, {0, 0, MENU_HEADER_BUTTON_WIDTH + 3, MENU_TITLE_TOP + 5}, page);
    btn->setPressHandler([=]() {
      menu->setCurrentTab(getIndex(btn->getIcon()));
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
    for (uint8_t i = 0; i < buttons.size(); i += 1)
      buttons[i]->page()->update(i);
  }

  PageTab* pageTab(uint8_t idx) const { return buttons[idx]->page(); }
  bool isCurrent(uint8_t idx) const { return currentIndex == idx; }
  uint8_t tabCount() const { return buttons.size(); }

 protected:
  std::vector<TabCarouselButton*> buttons;
  TabsGroup* menu;
  uint8_t currentIndex = 0;

  uint8_t getIndex(EdgeTxIcon icon)
  {
    for (uint8_t i = 0; i < buttons.size(); i += 1) {
      if (buttons[i]->getIcon() == icon) return i;
    }
    return -1;
  }
};

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

class TabsGroupHeader : public Window
{
  friend class TabsGroup;

 public:
  TabsGroupHeader(TabsGroup* menu, EdgeTxIcon icon) :
      Window(menu, {0, 0, LCD_W, MENU_BODY_TOP}), icon(icon)
  {
    setWindowFlag(NO_FOCUS | OPAQUE);

    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

    headerIcon = new HeaderIcon(this, icon);

    auto sep = lv_obj_create(lvobj);
    etx_solid_bg(sep);
    lv_obj_set_pos(sep, 0, MENU_HEADER_HEIGHT);
    lv_obj_set_size(sep, LCD_W, MENU_TITLE_TOP - MENU_HEADER_HEIGHT);

    titleLabel = lv_label_create(lvobj);
    etx_txt_color(titleLabel, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_style_pad_left(titleLabel, PAD_MEDIUM, LV_PART_MAIN);
    lv_obj_set_style_pad_top(titleLabel, 1, LV_PART_MAIN);
    lv_obj_set_pos(titleLabel, 0, MENU_TITLE_TOP);
    lv_obj_set_size(titleLabel, LCD_W, MENU_TITLE_HEIGHT);
    setTitle("");

    carousel = new TabsCarousel(this, menu);

    dateTime = new HeaderDateTime(lvobj, LCD_W - 48, 6);
  }

  void setTitle(const char* title) { lv_label_set_text(titleLabel, title); }

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "TabsGroupHeader"; }
#endif

 protected:
  EdgeTxIcon icon;
  TabsCarousel* carousel = nullptr;
  lv_obj_t* titleLabel = nullptr;
  HeaderDateTime* dateTime = nullptr;
  HeaderIcon* headerIcon = nullptr;

  void checkEvents() override
  {
    dateTime->update();
    Window::checkEvents();
  }

  void updateDateTime();
};

TabsGroup::TabsGroup(EdgeTxIcon icon) :
    NavWindow(MainWindow::instance(), {0, 0, LCD_W, LCD_H})
{
  header = new TabsGroupHeader(this, icon);
  body = new Window(this, {0, MENU_BODY_TOP, LCD_W, MENU_BODY_HEIGHT});
  body->setWindowFlag(NO_FOCUS);

  etx_solid_bg(lvobj);
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

uint8_t TabsGroup::tabCount() const { return header->carousel->tabCount(); }

void TabsGroup::setCurrentTab(unsigned index)
{
  header->carousel->setCurrentIndex(index);
  setVisibleTab(header->carousel->pageTab(index));
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
  header->carousel->addTab(page);
  if (!currentTab) {
    setCurrentTab(0);
  }
}

void TabsGroup::removeTab(unsigned index)
{
  if (header->carousel->isCurrent(index))
    setCurrentTab(max<unsigned>(0, index - 1));
  header->carousel->removeTab(index);
}

void TabsGroup::setVisibleTab(PageTab* tab)
{
  if (tab != currentTab && !deleted()) {
    header->setTitle(tab->title.c_str());

    body->clear();
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

    body->padAll(tab->padding);
    tab->build(body);

#if defined(DEBUG)
    end_ms = RTOS_GET_MS();
#endif
  }
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
void TabsGroup::onPressPGUP() { header->carousel->prevTab(); }
void TabsGroup::onPressPGDN() { header->carousel->nextTab(); }
#endif

void TabsGroup::onClicked() { Keyboard::hide(false); }

void TabsGroup::onCancel() { deleteLater(); }

Window* TabsGroup::getHeaderWindow() { return header; }
