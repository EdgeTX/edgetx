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
#include <algorithm>
#include "screen_setup.h"
#include "opentx.h"
#include "view_main.h"
#include "widget_settings.h"
#include "topbar.h"
#include "libopenui.h"
#include "layouts/layout_factory_impl.h"
#include "widgets_setup.h"

#define SET_DIRTY()   storageDirty(EE_MODEL)
#define BUTTON_HEIGHT 30
#define BUTTON_WIDTH  75

class LayoutChoice: public FormField
{
  public:

    typedef std::function<const LayoutFactory *()> LayoutFactoryGetter;
    typedef std::function<void(const LayoutFactory *)> LayoutFactorySetter;

    LayoutChoice(Window * parent, const rect_t & rect, LayoutFactoryGetter getValue, LayoutFactorySetter setValue):
      FormField(parent, {rect.x, rect.y, 59, 33}),
      getValue(std::move(getValue)),
      setValue(std::move(setValue))
    {
    }

    void paint(BitmapBuffer * dc) override
    {
      FormField::paint(dc);
      auto layout = getValue();
      if (layout) {
        layout->drawThumb(dc, 4, 4, editMode ? COLOR_THEME_PRIMARY2 : COLOR_THEME_PRIMARY3);
      }
    }

#if defined(HARDWARE_KEYS)
    void onEvent(event_t event) override
    {
      TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

      if (event == EVT_KEY_BREAK(KEY_ENTER)) {
        editMode = true;
        invalidate();
        openMenu();
      }
      else {
        FormField::onEvent(event);
      }
    }
#endif

#if defined(HARDWARE_TOUCH)
    bool onTouchEnd(coord_t, coord_t) override
    {
      if (enabled) {
        if (!hasFocus()) {
          setFocus(SET_FOCUS_DEFAULT);
        }
        onKeyPress();
        openMenu();
      }
      return true;
    }
#endif

    void openMenu()
    {
      auto menu = new Menu(parent);

      for (auto layout : getRegisteredLayouts()) {
        menu->addLine(layout->getBitmap(), layout->getName(),
                      [=]() { setValue(layout); });
      }

      auto it = std::find(getRegisteredLayouts().begin(),
                          getRegisteredLayouts().end(), getValue());
      menu->select(std::distance(getRegisteredLayouts().begin(), it));

      menu->setCloseHandler([=]() {
        editMode = false;
        setFocus(SET_FOCUS_DEFAULT);
      });
    }

  protected:
    std::function<const LayoutFactory *()> getValue;
    std::function<void(const LayoutFactory *)> setValue;
};

ScreenAddPage::ScreenAddPage(ScreenMenu * menu, uint8_t pageIndex):
  PageTab(),
  menu(menu),
  pageIndex(pageIndex)
{
  setTitle(STR_ADD_MAIN_VIEW);
  setIcon(ICON_THEME_ADD_VIEW);
}

extern const LayoutFactory * defaultLayout;

void ScreenAddPage::build(FormWindow * window)
{
  rect_t buttonRect = {LCD_W / 2 - 100, (window->height() - 24) / 2, 200, 24};
  auto button = new TextButton(window, buttonRect, STR_ADD_MAIN_VIEW);

  auto pageIndex = this->pageIndex;
  auto menu      = this->menu;

  button->setPressHandler([menu, pageIndex]() -> uint8_t {

      // First page is "User interface", subtract it
      auto  newIdx     = pageIndex - 1; 
      TRACE("ScreenAddPage: add screen: newIdx = %d", newIdx);

      auto& screen     = customScreens[newIdx];
      auto& screenData = g_model.screenData[newIdx];

      TRACE("ScreenAddPage: add screen: screen = %p", screen);

      const LayoutFactory * factory = defaultLayout;
      if (factory) {
        TRACE("ScreenAddPage: add screen: factory = %p", factory);

        auto viewMain = ViewMain::instance();
        screen = factory->create(viewMain, &screenData.layoutData);
        viewMain->addMainView(screen, newIdx);

        strncpy(screenData.LayoutId, factory->getId(), sizeof(screenData.LayoutId));
        TRACE("ScreenAddPage: add screen: LayoutId = %s", screenData.LayoutId);

        auto tab = new ScreenSetupPage(menu, pageIndex, newIdx);
        std::string title(STR_MAIN_VIEW_X);
        title.back() = newIdx + '1';
        tab->setTitle(title);
        tab->setIcon(ICON_THEME_VIEW1 + newIdx);

        // remove current tab first
        menu->setCurrentTab(0);
        menu->removeTab(pageIndex);

        // add the new one
        menu->addTab(tab);
        menu->setCurrentTab(pageIndex);

        if (menu->getTabs() < MAX_CUSTOM_SCREENS) {
          menu->addTab(new ScreenAddPage(menu, menu->getTabs()));
        }
      }
      else {
        TRACE("Add main view: factory is NULL");
      }
      return 0;
  });
}

ScreenSetupPage::ScreenSetupPage(ScreenMenu * menu, unsigned pageIndex, unsigned customScreenIndex):
  PageTab(),
  menu(menu),
  pageIndex(pageIndex),
  customScreenIndex(customScreenIndex)
{
}

static void updateLayoutOptions(FormGroup* optionsWindow, unsigned customScreenIndex)
{
  FormGridLayout grid;
  optionsWindow->clear();

  // Layout options...
  int index = 0;
  auto layout = dynamic_cast<Layout*>(customScreens[customScreenIndex]);
  if (!layout) {
    return;
  }
  auto factory = layout->getFactory();
  if (!factory) {
    return;
  }

  for (auto * option = factory->getOptions(); option->name; option++, index++) {

    auto layoutData = &g_model.screenData[customScreenIndex].layoutData;
    ZoneOptionValue * value = &layoutData->options[index].value;

    // Option label
    new StaticText(optionsWindow, grid.getLabelSlot(false), option->name, 0, COLOR_THEME_PRIMARY1);

    // Option value
    switch (option->type) {
    case ZoneOption::Bool:
      new CheckBox(optionsWindow, grid.getFieldSlot(), GET_SET_DEFAULT(value->boolValue));
      break;

    case ZoneOption::Color:
      new ColorEdit(optionsWindow, grid.getFieldSlot(), GET_SET_DEFAULT(value->unsignedValue));
      break;

    default:
      break;
    }
    grid.nextLine();
  }

  optionsWindow->adjustHeight();
}

void ScreenSetupPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);

  // Layout choice...
  new StaticText(window, grid.getLabelSlot(false), STR_LAYOUT, 0, COLOR_THEME_PRIMARY1);

  auto layoutSlot = grid.getFieldSlot();
  layoutSlot.h = 2 * PAGE_LINE_HEIGHT - 1;

  // Dynamic options window...
  auto idx = customScreenIndex;
  rect_t optRect = {0, grid.getWindowHeight(), window->width(), 0};
  auto optionsWindow = new FormGroup(window, optRect, FORWARD_SCROLL | FORM_FORWARD_FOCUS);

  LayoutChoice::LayoutFactoryGetter getFactory = [idx] () -> const LayoutFactory * {
    auto layout = dynamic_cast<Layout*>(customScreens[idx]);
    if (!layout) return nullptr;
    return layout->getFactory();
  };
  LayoutChoice::LayoutFactorySetter setLayout =
      [=](const LayoutFactory* factory) {
        // delete any options potentially accessing
        // the old custom screen
        window->clear();
        createCustomScreen(factory, idx);
        build(window);
      };

  auto layoutChoice = new LayoutChoice(window, layoutSlot, getFactory, setLayout);
  grid.nextLine(layoutChoice->height());

  // Setup widgets button...
  auto setupWidgetsButton = new TextButton(window, grid.getFieldSlot(), STR_SETUP_WIDGETS);

  auto menu = this->menu;
  setupWidgetsButton->setPressHandler([idx, menu]() -> uint8_t {
      new SetupWidgetsPage(menu, idx);
      return 0;
  });
  grid.nextLine();

  // place the options window at the right spot
  optionsWindow->setTop(grid.getWindowHeight());
  updateLayoutOptions(optionsWindow, idx);
  grid.addWindow(optionsWindow);

  // fix focus order due to early insertion
  FormField::link(setupWidgetsButton, optionsWindow);
  window->setLastField(optionsWindow);
  
  // Prevent removing the last page
  if (customScreens[1] != nullptr) {
    auto button = new TextButton(window, grid.getFieldSlot(), STR_REMOVE_SCREEN);
    auto menu = this->menu;
    button->setPressHandler([menu, idx]() -> uint8_t {

      // Set the current tab to "User interface" to trigger body clearing
      menu->setCurrentTab(0);

      // Remove this screen from the model
      disposeCustomScreen(idx);

      // Delete all custom screens
      deleteCustomScreens();

      // ... and reload
      loadCustomScreens();
      menu->updateTabs();

      // Let's try to stay on the same page
      // (first tab is "User interface")
      auto pageIdx = idx + 1;

      // Subtract one more as the last one is "New main screen"
      if (pageIdx > menu->getTabs() - 2) {
        pageIdx = menu->getTabs() - 2;
      }      
      menu->setCurrentTab(pageIdx);
      return 0;
    });

    // fix focus order due to early insertion
    FormField::link(button, layoutChoice);
  }

  window->adjustInnerHeight();
}
