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
#include "layout.h"
#include "edgetx.h"
#include "topbar.h"
#include "view_main.h"
#include "widget_settings.h"
#include "widgets_setup.h"

#define SET_DIRTY() storageDirty(EE_MODEL)
#define BUTTON_HEIGHT 30
#define BUTTON_WIDTH 75

extern PageDef screensMenuItems[];

class LayoutChoice : public Button
{
 public:
  typedef std::function<const LayoutFactory*()> LayoutFactoryGetter;
  typedef std::function<void(const LayoutFactory*)> LayoutFactorySetter;

  LayoutChoice(Window* parent, LayoutFactoryGetter getValue,
               LayoutFactorySetter setValue) :
      Button(parent, {0, 0, LayoutFactory::BM_W + PAD_LARGE + PAD_SMALL, LayoutFactory::BM_H + PAD_LARGE + PAD_SMALL}),
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

    const MaskBitmap* bitmap = layout->getBitmap();
    if (!bitmap) return;

    lv_coord_t w = bitmap->width;
    lv_coord_t h = bitmap->height;
    lv_canvas_set_buffer(canvas, (void*)&bitmap->data[0], w, h, LV_IMG_CF_ALPHA_8BIT);
  }

  void setValue(const LayoutFactory* layout)
  {
    if (_setValue) _setValue(layout);
    update();
  }
};

#if LANDSCAPE
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1),
                                          LV_GRID_FR(2), LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
#endif
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

ScreenSetupPage::ScreenSetupPage(unsigned index, PageDef& pageDef) :
    PageGroupItem(pageDef)
{
  update(index + FIRST_SCREEN_OFFSET);
}

void ScreenSetupPage::update(uint8_t index)
{
  customScreenIndex = index - FIRST_SCREEN_OFFSET;
}

void ScreenSetupPage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid(line_col_dsc, line_row_dsc);

  // Layout choice...
  auto line = window->newLine(grid);
  auto label = new StaticText(line, rect_t{}, STR_LAYOUT);

  lv_obj_set_style_grid_cell_y_align(label->getLvObj(), LV_GRID_ALIGN_CENTER, 0);

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
        auto layoutData = g_model.getScreenLayoutData(customScreenIndex);
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

#if PORTRAIT
  line = window->newLine(grid);
  grid.nextCell();
#endif
  btn = new TextButton(line, rect_t{}, STR_SETUP_WIDGETS,
                       [=]() -> uint8_t {
    window->getParent()->deleteLater();
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
          g_model.removeScreenLayout(customScreenIndex);

          // Delete all custom screens
          LayoutFactory::deleteCustomScreens();

          // ... and reload
          LayoutFactory::loadCustomScreens();

          // Reset to first screen so user knows something has happened
          PageGroup* menu = (PageGroup*)window->getParent();
          menu->setCurrentTab(FIRST_SCREEN_OFFSET);

          storageDirty(EE_MODEL);
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
  for (auto* option = factory->getLayoutOptions(); option->name; option++, index++) {
    auto layoutData = g_model.getScreenLayoutData(customScreenIndex);
    LayoutOptionValue* value = &layoutData->options[index].value;

    // Option label
    auto line = layoutOptions->newLine(grid);
    new StaticText(line, rect_t{}, STR_VAL(option->name));

    // Option value
    switch (option->type) {
      case LayoutOption::Bool:
        new ToggleSwitch(line, rect_t{},
                         GET_DEFAULT(value->boolValue),
                         [=](int newValue) {
                           value->boolValue = newValue;
                           SET_DIRTY();
                         });
        break;

      case LayoutOption::Color:
        new ColorPicker(line, rect_t{}, GET_SET_DEFAULT(value->unsignedValue));
        break;

      default:
        break;
    }
  }
}
