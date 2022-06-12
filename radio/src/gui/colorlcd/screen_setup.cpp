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

class LayoutChoice : public Button
{
 public:
  typedef std::function<const LayoutFactory*()> LayoutFactoryGetter;
  typedef std::function<void(const LayoutFactory*)> LayoutFactorySetter;

  LayoutChoice(Window* parent, LayoutFactoryGetter getValue, LayoutFactorySetter setValue) :
    Button(parent, rect_t{}, nullptr, 0, 0, lv_btn_create),
      getValue(std::move(getValue)),
      _setValue(std::move(setValue))
  {
    canvas = lv_canvas_create(lvobj);
    lv_obj_center(canvas);
    update();
  }

  void onPress() override
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

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

static std::function<uint8_t()> startWidgetsSetup(ScreenMenu* menu,
                                                  uint8_t screen_idx)
{
  return [=]() -> uint8_t {
    new SetupWidgetsPage(menu, screen_idx);
    return 0;
  };
}

static std::function<uint8_t()> removeScreen(ScreenMenu* menu,
                                             uint8_t screen_idx)
{
  return [=]() -> uint8_t {
    // Set the current tab to "User interface" to trigger body clearing
    menu->setCurrentTab(0);

    // Remove this screen from the model
    disposeCustomScreen(screen_idx);

    // Delete all custom screens
    deleteCustomScreens();

    // ... and reload
    loadCustomScreens();
    menu->updateTabs();

    // Let's try to stay on the same page
    // (first tab is "User interface")
    auto pageIdx = screen_idx + 1;

    // Subtract one more as the last one is "New main screen"
    if (pageIdx > menu->getTabs() - 2) {
      pageIdx = menu->getTabs() - 2;
    }
    menu->setCurrentTab(pageIdx);
    return 0;
  };
}

ScreenSetupPage::ScreenSetupPage(ScreenMenu* menu, unsigned pageIndex,
                                 unsigned customScreenIndex) :
    PageTab(),
    menu(menu),
    pageIndex(pageIndex),
    customScreenIndex(customScreenIndex)
{
}

void ScreenSetupPage::build(FormWindow * form)
{
  FlexGridLayout grid(line_col_dsc, line_row_dsc);
  form->setFlexLayout();

  // Layout choice...
  auto line = form->newLine(&grid);
  auto label = new StaticText(line, rect_t{}, STR_LAYOUT, 0, COLOR_THEME_PRIMARY1);

  auto obj = label->getLvObj();
  lv_obj_set_style_grid_cell_y_align(obj, LV_GRID_ALIGN_START, 0);

  // Dynamic options window...
  auto idx = customScreenIndex;

  LayoutChoice::LayoutFactoryGetter getFactory = [idx] () -> const LayoutFactory * {
    auto layout = dynamic_cast<Layout*>(customScreens[idx]);
    if (!layout) return nullptr;
    return layout->getFactory();
  };

  LayoutChoice::LayoutFactorySetter setLayout =
      [=](const LayoutFactory* factory) {
        // delete any options potentially accessing
        // the old custom screen before re-creating it
        clearLayoutOptions();
        createCustomScreen(factory, idx);
        buildLayoutOptions();
      };

  auto layoutForm = new FormGroup(line, rect_t{}, FORWARD_SCROLL | FORM_FORWARD_FOCUS);
  layoutForm->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, 8);

  obj = layoutForm->getLvObj();
  lv_obj_set_style_grid_cell_x_align(obj, LV_GRID_ALIGN_STRETCH, 0);

  Window* btn = new LayoutChoice(layoutForm, getFactory, setLayout);
  obj = btn->getLvObj();
  lv_obj_set_style_min_width(obj, LV_DPI_DEF / 2, LV_PART_MAIN);
  lv_obj_set_style_min_height(obj, LV_DPI_DEF / 3, LV_PART_MAIN);
  lv_obj_set_style_pad_all(obj, 8, LV_PART_MAIN);
  lv_obj_set_style_radius(obj, 8, LV_PART_MAIN);

  btn = new TextButton(layoutForm, rect_t{}, STR_SETUP_WIDGETS,
                       startWidgetsSetup(menu, idx));
  obj = btn->getLvObj();
  lv_obj_set_style_min_width(obj, LV_DPI_DEF / 2, LV_PART_MAIN);
  lv_obj_set_style_min_height(obj, LV_DPI_DEF / 3, LV_PART_MAIN);
  lv_obj_set_style_pad_all(obj, 8, LV_PART_MAIN);
  lv_obj_set_style_radius(obj, 8, LV_PART_MAIN);

  line = form->newLine();
  layoutOptions = new FormGroup(line, rect_t{}, FORWARD_SCROLL | FORM_FORWARD_FOCUS);
  buildLayoutOptions();

  // Prevent removing the last page
  if (customScreens[1] != nullptr) {
    line = form->newLine();
    btn = new TextButton(line, rect_t{}, STR_REMOVE_SCREEN, removeScreen(menu, idx));
    obj = btn->getLvObj();
    lv_obj_set_style_min_width(obj, LV_DPI_DEF / 2, LV_PART_MAIN);
    lv_obj_set_style_min_height(obj, LV_DPI_DEF / 3, LV_PART_MAIN);
    lv_obj_set_style_pad_all(obj, 8, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 8, LV_PART_MAIN);

    lv_obj_set_width(obj, lv_pct(50));
    lv_obj_center(obj);
  }

  form->updateSize();
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
  auto layout = dynamic_cast<Layout*>(customScreens[customScreenIndex]);
  if (!layout) return;

  auto factory = layout->getFactory();
  if (!factory) return;

  int index = 0;
  for (auto* option = factory->getOptions(); option->name; option++, index++) {

    auto layoutData = &g_model.screenData[customScreenIndex].layoutData;
    ZoneOptionValue* value = &layoutData->options[index].value;

    // Option label
    auto line = layoutOptions->newLine(&grid);
    new StaticText(line, rect_t{}, option->name, 0, COLOR_THEME_PRIMARY1);

    // Option value
    switch (option->type) {
      case ZoneOption::Bool:
        new CheckBox(line, rect_t{}, GET_SET_DEFAULT(value->boolValue));
        break;

      case ZoneOption::Color:
        new ColorEdit(line, rect_t{}, GET_SET_DEFAULT(value->unsignedValue));
        break;

      default:
        break;
    }
  }

  layoutOptions->updateSize();

  auto parent = layoutOptions->getParent();
  if (parent) parent->updateSize();
}
