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

#include "widgets_setup.h"

#include "layer.h"
#include "menu.h"
#include "menu_screen.h"
#include "myeeprom.h"
#include "storage/storage.h"
#include "etx_lv_theme.h"
#include "view_main.h"
#include "widget_settings.h"

SetupWidgetsPageSlot::SetupWidgetsPageSlot(Window* parent, const rect_t& rect,
                                           WidgetsContainer* container,
                                           uint8_t slotIndex) :
    ButtonBase(parent, rect)
{
  setPressHandler([=]() -> uint8_t {
    if (container->getWidget(slotIndex)) {
      Menu* menu = new Menu();
      menu->addLine(STR_SELECT_WIDGET,
                    [=]() { addNewWidget(container, slotIndex); });
      auto widget = container->getWidget(slotIndex);
      if (widget->getOptions() && widget->getOptions()->name)
        menu->addLine(STR_WIDGET_SETTINGS,
                      [=]() { new WidgetSettings(widget); });
      menu->addLine(STR_REMOVE_WIDGET,
                    [=]() { container->removeWidget(slotIndex); });
    } else {
      addNewWidget(container, slotIndex);
    }

    return 0;
  });

  etx_obj_add_style(lvobj, styles->border, LV_STATE_FOCUSED);
  etx_obj_add_style(lvobj, styles->border_color[COLOR_THEME_FOCUS_INDEX], LV_STATE_FOCUSED);

  lv_style_init(&borderStyle);
  lv_style_set_line_width(&borderStyle, 2);
  lv_style_set_line_opa(&borderStyle, LV_OPA_COVER);
  lv_style_set_line_dash_width(&borderStyle, 2);
  lv_style_set_line_dash_gap(&borderStyle, 2);
  lv_style_set_line_color(&borderStyle, makeLvColor(COLOR_THEME_SECONDARY2));

  borderPts[0] = {1, 1};
  borderPts[1] = {(lv_coord_t)(width() - 1), 1};
  borderPts[2] = {(lv_coord_t)(width() - 1), (lv_coord_t)(height() - 1)};
  borderPts[3] = {1, (lv_coord_t)(height() - 1)};
  borderPts[4] = {1, 1};

  border = lv_line_create(lvobj);
  lv_obj_add_style(border, &borderStyle, LV_PART_MAIN);
  lv_line_set_points(border, borderPts, 5);

  setFocusState();

  setFocusHandler([=](bool) { setFocusState(); });
}

void SetupWidgetsPageSlot::setFocusState()
{
  if (hasFocus()) {
    bringToTop();
    lv_obj_add_flag(border, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_clear_flag(border, LV_OBJ_FLAG_HIDDEN);
  }
}

void SetupWidgetsPageSlot::addNewWidget(WidgetsContainer* container,
                                        uint8_t slotIndex)
{
  const char* cur = nullptr;
  auto w = container->getWidget((slotIndex));
  if (w) cur = w->getFactory()->getDisplayName();

  Menu* menu = new Menu();
  menu->setTitle(STR_SELECT_WIDGET);
  int selected = -1;
  int index = 0;
  for (auto factory : WidgetFactory::getRegisteredWidgets()) {
    menu->addLine(factory->getDisplayName(), [=]() {
      container->createWidget(slotIndex, factory);
      auto widget = container->getWidget(slotIndex);
      if (widget->getOptions() && widget->getOptions()->name)
        new WidgetSettings(widget);
    });
    if (cur && strcmp(cur, factory->getDisplayName()) == 0)
      selected = index;
    index += 1;
  }

  if (selected >= 0)
    menu->select(selected);
}

SetupWidgetsPage::SetupWidgetsPage(uint8_t customScreenIdx) :
    Window(ViewMain::instance(), rect_t{}), customScreenIdx(customScreenIdx)
{
  Layer::push(this);

  // attach this custom screen here so we can display it
  auto screen = customScreens[customScreenIdx];
  if (screen) {
    setRect(screen->getRect());
    auto viewMain = ViewMain::instance();
    savedView = viewMain->getCurrentMainView();
    viewMain->setCurrentMainView(customScreenIdx);
    if (!viewMain->hasTopbar()) viewMain->hideTopBarEdgeTxButton();
  }

  SetupWidgetsPageSlot* firstSlot = nullptr;
  for (unsigned i = 0; i < screen->getZonesCount(); i++) {
    auto rect = screen->getZone(i);
    auto widget_container = customScreens[customScreenIdx];
    auto slot = new SetupWidgetsPageSlot(this, rect, widget_container, i);
    if (i == 0) firstSlot = slot;
  }
  if (firstSlot) lv_group_focus_obj(firstSlot->getLvObj());

#if defined(HARDWARE_TOUCH)
  addBackButton();
#endif

  screen->show();
}

void SetupWidgetsPage::onClicked()
{
  // block event forwarding (window is transparent)
}

void SetupWidgetsPage::onCancel() { deleteLater(); }

void SetupWidgetsPage::deleteLater(bool detach, bool trash)
{
  // restore screen setting tab on top
  Layer::pop(this);

  // and continue async deletion...
  auto screen = customScreens[customScreenIdx];
  if (screen) {
    auto viewMain = ViewMain::instance();
    viewMain->setCurrentMainView(savedView);
    viewMain->showTopBarEdgeTxButton();
  }
  Window::deleteLater(detach, trash);
  new ScreenMenu(customScreenIdx + 1);

  storageDirty(EE_MODEL);
}

void SetupWidgetsPage::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  if (event == EVT_KEY_FIRST(KEY_PAGEUP) ||
      event == EVT_KEY_FIRST(KEY_PAGEDN) || event == EVT_KEY_FIRST(KEY_SYS) ||
      event == EVT_KEY_FIRST(KEY_MODEL)) {
    killEvents(event);
  } else if (event == EVT_KEY_FIRST(KEY_TELE)) {
    onCancel();
  } else {
    Window::onEvent(event);
  }
#else
  Window::onEvent(event);
#endif
}
