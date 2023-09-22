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
#include "menu_screen.h"

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

class ThemeView : public FormWindow
{
  public:
    ThemeView(Window* parent) : FormWindow(parent, rect_t{})
    {
      padAll(0);
      setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);

      auto tp = ThemePersistance::instance();
      auto theme = tp->getCurrentTheme();

      if (theme) {
        if (!details) {
          FlexGridLayout theme_grid(theme_col_dsc, theme_row_dsc);
          auto line = newLine(&theme_grid);

          details = new Window(line, rect_t{});
          // vertical flow layout
          lv_obj_set_flex_flow(details->getLvObj(), LV_FLEX_FLOW_COLUMN);
          lv_obj_set_style_pad_row(details->getLvObj(), 2, LV_PART_MAIN);

          // make the object fill the grid cell
          lv_obj_set_style_grid_cell_x_align(details->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

          new StaticText(details, rect_t{}, STR_AUTHOR, 0, COLOR_THEME_PRIMARY1 | FONT(BOLD));
          author = new StaticText(details, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);

          // labels default to LV_SIZE_CONTENT,
          // which could overflow the width avail
          lv_obj_set_width(author->getLvObj(), lv_pct(100));

          new StaticText(details, rect_t{}, STR_DESCRIPTION, 0, COLOR_THEME_PRIMARY1 | FONT(BOLD));
          description = new StaticText(details, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);

          lv_obj_set_width(description->getLvObj(), lv_pct(100));

          preview = new FilePreview(line, rect_t{}, false);

          // center within cell
          lv_obj_set_style_grid_cell_x_align(preview->getLvObj(), LV_GRID_ALIGN_CENTER, 0);
        }

        author->setText(theme->getAuthor());
        description->setText(theme->getInfo());

        auto themeImage = theme->getThemeImageFileNames();
        if (themeImage.size() > 0) {
          preview->setFile(themeImage[0].c_str());

          // adjust width according to max
          preview->setWidth(min(MAX_PREVIEW_WIDTH, preview->getBitmapWidth()));
          preview->setHeight(min(MAX_PREVIEW_HEIGHT, preview->getBitmapHeight()));
        } else {
          preview->setFile("");
        }
      }
    }

  protected:
    Window* details = nullptr;
    FilePreview* preview = nullptr;
    StaticText* author = nullptr;
    StaticText* description = nullptr;
};

ScreenUserInterfacePage::ScreenUserInterfacePage(ScreenMenu* menu):
  PageTab(STR_USER_INTERFACE, ICON_THEME_SETUP),
  menu(menu)
{
}

void ScreenUserInterfacePage::build(FormWindow* window)
{
  window->padAll(4);
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 0);

  FlexGridLayout grid(line_col_dsc, line_row_dsc);

  // Top Bar
  auto line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_TOP_BAR, 0, COLOR_THEME_PRIMARY1);

  auto menu = this->menu;
  auto setupTopbarWidgets = new TextButton(line, rect_t{}, STR_SETUP_WIDGETS);
  setupTopbarWidgets->setPressHandler([menu]() -> uint8_t {
      menu->deleteLater();
      new SetupTopBarWidgetsPage();
      return 0;
  });

  // Theme choice
  line = window->newLine(&grid);
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
      TabsGroup::refreshTheme();

      // Force redraw to ensure all items are updated with new theme colors
      window->clear();
      build(window);
  });

  grid.setColSpan(2);
  line = window->newLine(&grid);
  themeView = new ThemeView(line);
}

