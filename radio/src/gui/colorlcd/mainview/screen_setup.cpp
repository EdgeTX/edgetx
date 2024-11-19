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

#include "screen_setup.h"

#include <algorithm>

#include "color_picker.h"
#include "layout_factory_impl.h"
#include "libopenui.h"
#include "edgetx.h"
#include "topbar.h"
#include "view_main.h"
#include "widget_settings.h"
#include "widgets_setup.h"

#define SET_DIRTY() storageDirty(EE_MODEL)
#define BUTTON_HEIGHT 30
#define BUTTON_WIDTH 75

class LayoutChoice : public Button
{
 public:
  typedef std::function<const LayoutFactory*()> LayoutFactoryGetter;
  typedef std::function<void(const LayoutFactory*)> LayoutFactorySetter;

  LayoutChoice(Window* parent, LayoutFactoryGetter getValue,
               LayoutFactorySetter setValue) :
      Button(parent, {0, 0, LayoutFactory::BM_W + 12, LayoutFactory::BM_H + 12}),
      getValue(std::move(getValue)),
      _setValue(std::move(setValue))
  {
    padAll(PAD_ZERO);
    canvas = lv_canvas_create(lvobj);
    lv_obj_center(canvas);
    update();
  }

  void onPress() override
  {
    auto menu = new Menu();
    for (auto layout : LayoutFactory::getRegisteredLayouts()) {
      menu->addLine(layout->getBitmap(), layout->getName(),
                    [=]() { setValue(layout); });
    }

    auto it =
        std::find(LayoutFactory::getRegisteredLayouts().begin(),
                  LayoutFactory::getRegisteredLayouts().end(), getValue());
    menu->select(
        std::distance(LayoutFactory::getRegisteredLayouts().begin(), it));

    menu->setCloseHandler([=]() {
      if (!menu->deleted())
        update();
    });
  }

 protected:
  lv_obj_t* canvas = nullptr;
  std::function<const LayoutFactory*()> getValue;
  std::function<void(const LayoutFactory*)> _setValue;

  void update()
  {
    if (!getValue || !canvas) return;

    auto layout = getValue();
    if (!layout) return;

    const uint8_t* bitmap = layout->getBitmap();
    if (!bitmap) return;

    lv_coord_t w = *((uint16_t*)bitmap);
    lv_coord_t h = *(((uint16_t*)bitmap) + 1);
    void* buf = (void*)(bitmap + 4);
    lv_canvas_set_buffer(canvas, buf, w, h, LV_IMG_CF_ALPHA_8BIT);
  }

  void setValue(const LayoutFactory* layout)
  {
    if (_setValue) _setValue(layout);
    update();
  }
};

ScreenAddPage::ScreenAddPage(ScreenMenu* menu, uint8_t pageIndex) :
    PageTab(STR_ADD_MAIN_VIEW, ICON_THEME_ADD_VIEW),
    menu(menu),
    pageIndex(pageIndex)
{
}

void ScreenAddPage::update(uint8_t index)
{
  pageIndex = index;
}

void ScreenAddPage::build(Window* window)
{
  new TextButton(window,
                 rect_t{LCD_W / 2 - 100, window->height() / 2 - 32, 200, 32},
                 STR_ADD_MAIN_VIEW, [this]() -> uint8_t {
                   // First page is "User interface", subtract it
                   auto newIdx = pageIndex - 1;
                   TRACE("ScreenAddPage: add screen: newIdx = %d", newIdx);

                   auto& screen = customScreens[newIdx];
                   auto& screenData = g_model.screenData[newIdx];

                   TRACE("ScreenAddPage: add screen: screen = %p", screen);

                   const LayoutFactory* factory = defaultLayout;
                   if (factory) {
                     TRACE("ScreenAddPage: add screen: factory = %p", factory);

                     auto viewMain = ViewMain::instance();
                     screen = factory->create(viewMain, &screenData.layoutData);
                     viewMain->addMainView(screen, newIdx);

                     strncpy(screenData.LayoutId, factory->getId(),
                             sizeof(screenData.LayoutId));
                     TRACE("ScreenAddPage: add screen: LayoutId = %s",
                           screenData.LayoutId);

                     auto tab = new ScreenSetupPage(menu, newIdx);
                     std::string title(STR_MAIN_VIEW_X);
                     if (newIdx >= 9) {
                       title[title.size() - 2] = '1';
                       title.back() = (newIdx - 9) + '0';
                     } else {
                       title[title.size() - 2] = newIdx + '1';
                       title.back() = ' ';
                     }
                     tab->setTitle(title);
                     tab->setIcon((EdgeTxIcon)(ICON_THEME_VIEW1 + newIdx));

                     // remove current tab first
                     menu->setCurrentTab(0);
                     menu->removeTab(pageIndex);

                     // add the new one
                     menu->addTab(tab);

                     if (menu->tabCount() <= MAX_CUSTOM_SCREENS) {
                       menu->addTab(new ScreenAddPage(menu, menu->tabCount()));
                     }

                     menu->setCurrentTab(pageIndex);
                   } else {
                     TRACE("Add main view: factory is NULL");
                   }
                   return 0;
                 });
}

#if !PORTRAIT_LCD
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1),
                                          LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
#endif
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

ScreenSetupPage::ScreenSetupPage(ScreenMenu* menu, unsigned index) :
    PageTab(),
    menu(menu)
{
  update(index + 1);
}

void ScreenSetupPage::update(uint8_t index)
{
  customScreenIndex = index - 1;

  std::string title(STR_MAIN_VIEW_X);
  if (customScreenIndex >= 9) {
    title[title.size() - 2] = '1';
    title.back() = (customScreenIndex - 9) + '0';
  } else {
    title[title.size() - 2] = customScreenIndex + '1';
    title.back() = ' ';
  }
  setTitle(title);
  setIcon((EdgeTxIcon)(ICON_THEME_VIEW1 + customScreenIndex));
}

void ScreenSetupPage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid(line_col_dsc, line_row_dsc);

  // Layout choice...
  auto line = window->newLine(grid);
  auto label = new StaticText(line, rect_t{}, STR_LAYOUT);

  lv_obj_set_style_grid_cell_y_align(label->getLvObj(), LV_GRID_ALIGN_CENTER,
                                     0);

  // Dynamic options window...
  LayoutChoice::LayoutFactoryGetter getFactory =
      [=]() -> const LayoutFactory* {
    auto layout = customScreens[customScreenIndex];
    if (!layout->isLayout()) return nullptr;
    return ((Layout*)layout)->getFactory();
  };

  LayoutChoice::LayoutFactorySetter setLayout =
      [=](const LayoutFactory* factory) {
        // delete any options potentially accessing
        // the old custom screen before re-creating it
        clearLayoutOptions();

        // If screen is not App Mode then save option values
        auto layoutData = &g_model.screenData[customScreenIndex].layoutData;
        auto layout = (Layout*)customScreens[customScreenIndex];
        bool restoreOptions = false;
        bool hasTopbar = true, hasFM = true, hasSliders = true, hasTrims = true, isMirrored = false;
        if (!layout->isAppMode()) {
          hasTopbar = layoutData->options[LAYOUT_OPTION_TOPBAR].value.boolValue;
          hasFM = layoutData->options[LAYOUT_OPTION_FM].value.boolValue;
          hasSliders = layoutData->options[LAYOUT_OPTION_SLIDERS].value.boolValue;
          hasTrims = layoutData->options[LAYOUT_OPTION_TRIMS].value.boolValue;
          isMirrored = layoutData->options[LAYOUT_OPTION_MIRRORED].value.boolValue;
          restoreOptions = true;
        }

        factory->createCustomScreen(customScreenIndex);

        // If new screen is not App Mode then restore saved option values
        layout = (Layout*)customScreens[customScreenIndex];
        if (restoreOptions && !layout->isAppMode()) {
          layoutData->options[LAYOUT_OPTION_TOPBAR].value.boolValue = hasTopbar;
          layoutData->options[LAYOUT_OPTION_FM].value.boolValue = hasFM;
          layoutData->options[LAYOUT_OPTION_SLIDERS].value.boolValue = hasSliders;
          layoutData->options[LAYOUT_OPTION_TRIMS].value.boolValue = hasTrims;
          layoutData->options[LAYOUT_OPTION_MIRRORED].value.boolValue = isMirrored;
        }

        buildLayoutOptions();
      };

  Window* btn = new LayoutChoice(line, getFactory, setLayout);

#if PORTRAIT_LCD
  line = window->newLine(grid);
  grid.nextCell();
#endif
  btn = new TextButton(line, rect_t{}, STR_SETUP_WIDGETS,
                       [=]() -> uint8_t {
    menu->deleteLater();
    new SetupWidgetsPage(customScreenIndex);
    return 0;
  });
  lv_obj_set_style_grid_cell_y_align(btn->getLvObj(), LV_GRID_ALIGN_CENTER, 0);
  lv_group_focus_obj(btn->getLvObj());

  line = window->newLine(grid);
  layoutOptions = new Window(line, rect_t{});
  buildLayoutOptions();

  // Prevent removing the last page
  if (customScreens[1] != nullptr) {
    grid.setColSpan(2);
    line = window->newLine(grid);
    Window* btn =
        new TextButton(line, rect_t{}, STR_REMOVE_SCREEN, [=]() -> uint8_t {
          // Remove this screen from the model
          LayoutFactory::disposeCustomScreen(customScreenIndex);

          // Delete all custom screens
          LayoutFactory::deleteCustomScreens();

          // ... and reload
          LayoutFactory::loadCustomScreens();

          // Let's try to stay on the same page
          menu->removeTab(customScreenIndex + 1);
          menu->setCurrentTab(customScreenIndex);
          return 0;
        });
    auto obj = btn->getLvObj();
    lv_obj_set_width(obj, lv_pct(100));
    lv_obj_center(obj);
  }
}

void ScreenSetupPage::clearLayoutOptions()
{
  if (!layoutOptions) return;
  layoutOptions->clear();
}

void ScreenSetupPage::buildLayoutOptions()
{
  if (!layoutOptions) return;

  FlexGridLayout grid(line_col_dsc, line_row_dsc);
  layoutOptions->setFlexLayout();

  // Layout options...
  auto layout = customScreens[customScreenIndex];
  if (!layout->isLayout()) return;

  auto factory = ((Layout*)layout)->getFactory();
  if (!factory) return;

  int index = 0;
  for (auto* option = factory->getOptions(); option->name; option++, index++) {
    auto layoutData = &g_model.screenData[customScreenIndex].layoutData;
    ZoneOptionValue* value = &layoutData->options[index].value;

    // Option label
    auto line = layoutOptions->newLine(grid);
    new StaticText(line, rect_t{}, option->name);

    // Option value
    switch (option->type) {
      case ZoneOption::Bool:
        new ToggleSwitch(line, rect_t{},
                         GET_DEFAULT(value->boolValue),
                         [=](int newValue) {
                           value->boolValue = newValue;
                           customScreens[customScreenIndex]->show();
                           SET_DIRTY();
                         });
        break;

      case ZoneOption::Color:
        new ColorPicker(line, rect_t{}, GET_SET_DEFAULT(value->unsignedValue));
        break;

      default:
        break;
    }
  }
}
