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

#include "screen_user_interface.h"
#include "theme_manager.h"
#include "file_preview.h"

struct ThemeDetails : public Window {
  ThemeDetails(Window* parent, ThemeFile* theme) : Window(parent, rect_t{})
  {
    // vertical flow layout
    lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(lvobj, 2, LV_PART_MAIN);

    // make the object fill the grid cell
    lv_obj_set_style_grid_cell_x_align(lvobj, LV_GRID_ALIGN_STRETCH, 0);

    // TODO: translation
    new StaticText(this, rect_t{}, "Author", 0,
                   COLOR_THEME_PRIMARY1 | FONT(BOLD));
    auto txt = new StaticText(this, rect_t{}, theme->getAuthor(), 0,
                              COLOR_THEME_PRIMARY1);

    // labels default to LV_SIZE_CONTENT,
    // which could overflow the width avail
    auto obj = txt->getLvObj();
    lv_obj_set_width(obj, lv_pct(100));

    // TODO: translation
    new StaticText(this, rect_t{}, "Description", 0,
                   COLOR_THEME_PRIMARY1 | FONT(BOLD));
    txt = new StaticText(this, rect_t{}, theme->getInfo(), 0,
                         COLOR_THEME_PRIMARY1);
    obj = txt->getLvObj();
    lv_obj_set_width(obj, lv_pct(100));
  }
};

#if LCD_W > LCD_H // landscape

// form grid
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

// theme details grid
static const lv_coord_t* theme_col_dsc = line_col_dsc;
static const lv_coord_t* theme_row_dsc = line_row_dsc;

#else // portrait

// form grid
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

// theme details grid
static const lv_coord_t theme_col_dsc[] = {LV_GRID_FR(1),
                                           LV_GRID_TEMPLATE_LAST};
static const lv_coord_t theme_row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                           LV_GRID_TEMPLATE_LAST};
#endif

constexpr coord_t MAX_PREVIEW_WIDTH = (100 * LV_DPI_DEF) / 57;
constexpr coord_t MAX_PREVIEW_HEIGHT = LV_DPI_DEF;

ScreenUserInterfacePage::ScreenUserInterfacePage(ScreenMenu* menu):
  PageTab(STR_USER_INTERFACE, ICON_THEME_SETUP),
  menu(menu)
{
}
void ScreenUserInterfacePage::build(FormWindow* form)
{
  FlexGridLayout grid(line_col_dsc, line_row_dsc);
  form->setFlexLayout();

  // Top Bar
  auto line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_TOP_BAR, 0, COLOR_THEME_PRIMARY1);

  auto menu = this->menu;
  auto setupTopbarWidgets = new TextButton(line, rect_t{}, STR_SETUP_WIDGETS);
  setupTopbarWidgets->setPressHandler([menu]() -> uint8_t {
      new SetupTopBarWidgetsPage(menu);
      return 0;
  });

  // Theme choice
  line = form->newLine(&grid);
  new StaticText(line, rect_t{}, STR_THEME, 0, COLOR_THEME_PRIMARY1);

  auto tp = ThemePersistance::instance();
  tp->refresh();
  std::vector<std::string> names = tp->getNames();

  new Choice(line, rect_t{}, names, 0, names.size() - 1,
    [=] () {
      return tp->getThemeIndex();
    }, 
    [=] (int value) {
      tp->setThemeIndex(value);
      tp->applyTheme(value);
      tp->setDefaultTheme(value);

      // TODO: shouldn't be necessary, would be better to send LV_EVENT_CHANGED
      form->clear();
      build(form);
  });

  auto theme = tp->getCurrentTheme();
  FilePreview* preview = nullptr;

  if (theme) {
    FlexGridLayout theme_grid(theme_col_dsc, theme_row_dsc);
    line = form->newLine(&theme_grid);

    new ThemeDetails(line, theme);

    auto themeImage = theme->getThemeImageFileNames();
    if (themeImage.size() > 0) {
      preview = new FilePreview(line, rect_t{}, false);
      preview->setFile(themeImage[0].c_str());

      // adjust width according to max
      preview->setWidth(min(MAX_PREVIEW_WIDTH, preview->getBitmapWidth()));
      preview->setHeight(min(MAX_PREVIEW_HEIGHT, preview->getBitmapHeight()));

      // center within cell
      auto obj = preview->getLvObj();
      lv_obj_set_style_grid_cell_x_align(obj, LV_GRID_ALIGN_CENTER, 0);
    }
  }

  form->updateSize();
}

