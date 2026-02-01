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
#include "edgetx.h"
#include "getset_helpers.h"
#include "layout.h"
#include "menu.h"
#include "static.h"
#include "toggleswitch.h"
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
      setValue(std::move(setValue))
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
                    [=]() { if (setValue) setValue(layout); update(); });
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
  std::function<void(const LayoutFactory*)> setValue;

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

ScreenSetupPage::ScreenSetupPage(unsigned index, const PageDef& pageDef) :
    PageGroupItem(pageDef), customScreenIndex(index)
{
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
    auto idx = customScreenIndex;
    window->getParent()->deleteLater();
    new SetupWidgetsPage(idx);
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

          // adjust index if last screen deleted
          if (customScreens[customScreenIndex] == nullptr) customScreenIndex -= 1;

          PageGroup* menu = (PageGroup*)window->getParent();
          // Reset to setup page to ensure screen properly updates.
          menu->setCurrentTab(QuickMenu::pageIndex(QM_UI_SETUP));
          // Reset to original (or adjusted screen)
          menu->setCurrentTab(QuickMenu::pageIndex((QMPage)(QM_UI_SCREEN1 + customScreenIndex)));

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
                           Messaging::send(Messaging::DECORATION_UPDATE);
                           SET_DIRTY();
                         });
        break;

      case LayoutOption::Color:
        new ColorPicker(line, rect_t{},
                        GET_DEFAULT(value->unsignedValue),
                        [=](int newValue) {
                          value->unsignedValue = newValue;
                           Messaging::send(Messaging::DECORATION_UPDATE);
                           SET_DIRTY();
                        });
        break;

      default:
        break;
    }
  }
}

void ScreenSetupPage::addScreen()
{
  int newIdx = 1;
  for (; newIdx < MAX_CUSTOM_SCREENS; newIdx += 1)
    if (customScreens[newIdx] == nullptr)
      break;

  TRACE("Add screen: add screen: newIdx = %d", newIdx);

  auto& screen = customScreens[newIdx];

  const LayoutFactory* factory = defaultLayout;
  if (factory) {
    TRACE("Add screen: add screen: factory = %p", factory);

    auto viewMain = ViewMain::instance();
    screen = factory->create(viewMain, newIdx);
    viewMain->addMainView(screen, newIdx);

    g_model.setScreenLayoutId(newIdx, factory->getId());
    TRACE("Add screen: add screen: LayoutId = %s", g_model.getScreenLayoutId(newIdx));

#if VERSION_MAJOR == 2
    Window::pageGroup()->deleteLater();
#endif
    QuickMenu::openPage((QMPage)(QM_UI_SCREEN1 + newIdx));

    storageDirty(EE_MODEL);
  } else {
    TRACE("Add screen: factory is NULL");
  }
}

#if VERSION_MAJOR == 2
ScreenAddPage::ScreenAddPage(const PageDef& pageDef) : PageGroupItem(pageDef)
{
}

void ScreenAddPage::build(Window* window)
{
  std::string s = replaceAll(STR_QM_ADD_SCREEN, "\n", " ");

  new TextButton(window,
                 rect_t{LCD_W / 2 - ADD_TXT_W / 2, window->height() / 2 - EdgeTxStyles::UI_ELEMENT_HEIGHT, ADD_TXT_W, EdgeTxStyles::UI_ELEMENT_HEIGHT},
                 s, [this]() -> uint8_t {
                    ScreenSetupPage::addScreen();
                    return 0;
                 });
}
#endif
