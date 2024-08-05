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

#include "file_preview.h"
#include "menu_screen.h"
#include "theme_manager.h"
#include "topbar_impl.h"
#include "view_main.h"

#if !PORTRAIT_LCD  // landscape

// form grid
static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

// theme details grid
static const lv_coord_t* theme_col_dsc = line_col_dsc;
static const lv_coord_t* theme_row_dsc = line_row_dsc;

#else  // portrait

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

class ThemeView : public Window
{
 public:
  ThemeView(Window* parent) : Window(parent, rect_t{})
  {
    padAll(PAD_ZERO);
    setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

    auto tp = ThemePersistance::instance();
    auto theme = tp->getCurrentTheme();

    if (theme) {
      FlexGridLayout theme_grid(theme_col_dsc, theme_row_dsc, PAD_TINY);
      auto line = newLine(theme_grid);

      details = new Window(line, rect_t{0, 0, 0, LV_SIZE_CONTENT});
      // vertical flow layout
      lv_obj_set_flex_flow(details->getLvObj(), LV_FLEX_FLOW_COLUMN);
      lv_obj_set_style_pad_row(details->getLvObj(), PAD_TINY, LV_PART_MAIN);

      // make the object fill the grid cell
      lv_obj_set_style_grid_cell_x_align(details->getLvObj(),
                                          LV_GRID_ALIGN_STRETCH, 0);
      lv_obj_set_style_grid_cell_y_align(details->getLvObj(),
                                          LV_GRID_ALIGN_START, 0);

      new StaticText(details, rect_t{}, STR_AUTHOR, COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));
      author = new StaticText(details, rect_t{}, "");

      // labels default to LV_SIZE_CONTENT,
      // which could overflow the width avail
      lv_obj_set_width(author->getLvObj(), lv_pct(100));

      new StaticText(details, rect_t{}, STR_DESCRIPTION, COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));
      description =
          new StaticText(details, rect_t{}, "");

      lv_obj_set_width(description->getLvObj(), lv_pct(100));

      preview = new FilePreview(line, {0, 0, PREVIEW_W, LV_SIZE_CONTENT});

      // align within cell
      lv_obj_set_style_grid_cell_x_align(preview->getLvObj(),
                                          LV_GRID_ALIGN_START, 0);

      update();
    }
  }

  void update()
  {
    auto tp = ThemePersistance::instance();
    auto theme = tp->getCurrentTheme();
    if (theme) {
      author->setText(theme->getAuthor());
      description->setText(theme->getInfo());

      auto themeImage = theme->getThemeImageFileNames();
      if (themeImage.size() > 0) {
        preview->setFile(themeImage[0].c_str());
      } else {
        preview->setFile("");
      }
    }
  }

  static LAYOUT_VAL(PREVIEW_W, (LCD_W - PAD_MEDIUM * 3) / 2, LCD_W - 12)

 protected:
  Window* details = nullptr;
  FilePreview* preview = nullptr;
  StaticText* author = nullptr;
  StaticText* description = nullptr;
};

ScreenUserInterfacePage::ScreenUserInterfacePage(ScreenMenu* menu) :
    PageTab(STR_USER_INTERFACE, ICON_THEME_SETUP, PAD_TINY), menu(menu)
{
}

void ScreenUserInterfacePage::build(Window* window)
{
  window->padAll(PAD_TINY);
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid(line_col_dsc, line_row_dsc, PAD_TINY);

  // Top Bar
  auto line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_TOP_BAR);

  new TextButton(line, rect_t{}, STR_SETUP_WIDGETS,
            [=]() -> uint8_t {
                menu->deleteLater();
                new SetupTopBarWidgetsPage();
                return 0;
            });

  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_WIDGET_SIZE);
  line = window->newLine(grid);
  auto box = new Window(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_TINY);
  box->padAll(PAD_TINY);
  box->padLeft(PAD_MEDIUM);
  for (int i = 0; i < MAX_TOPBAR_ZONES; i += 1) {
    coord_t w = (g_model.topbarWidgetWidth[i] * (WWBTN_W + PAD_TINY)) - PAD_TINY;
    if (w < WWBTN_W) w = WWBTN_W;
    widths[i] = new Choice(box, {0, 0, w, 0}, 0, MAX_TOPBAR_ZONES,
              [=]() {
                return g_model.topbarWidgetWidth[i];
              },
              [=](int value) {
                g_model.topbarWidgetWidth[i] = value;
                coord_t w = (g_model.topbarWidgetWidth[i] * (WWBTN_W + PAD_TINY)) - PAD_TINY;
                widths[i]->setWidth(w);
                int remaining = MAX_TOPBAR_ZONES;
                for (int n = 0; n < MAX_TOPBAR_ZONES; n += 1) {
                  if (n > i) {
                    if (remaining > 0) {
                      if (g_model.topbarWidgetWidth[n] == 0) {
                        g_model.topbarWidgetWidth[n] = 1;
                        widths[n]->setWidth(WWBTN_W);
                        widths[n]->show();
                        widths[n]->update();
                      }
                    } else {
                      if (g_model.topbarWidgetWidth[n] != 0) {
                        g_model.topbarWidgetWidth[n] = 0;
                        widths[n]->setWidth(WWBTN_W);
                        widths[n]->hide();
                        ViewMain::instance()->getTopbar()->removeWidget(n);
                      }
                    }
                  }
                  ViewMain::instance()->getTopbar()->load();
                  remaining -= g_model.topbarWidgetWidth[n];
                }
                storageDirty(EE_MODEL);
              }, STR_WIDGET_SIZE);
    widths[i]->setAvailableHandler([=](int value) {
      int remaining = MAX_TOPBAR_ZONES;
      for (int n = 0; n < i; n += 1)
        remaining -= g_model.topbarWidgetWidth[n];
      return value > 0 && value <= remaining;
    });
    if (g_model.topbarWidgetWidth[i] == 0)
      widths[i]->hide();
  }

  // Theme choice
  line = window->newLine(grid);
  new StaticText(line, rect_t{}, STR_THEME);

  auto tp = ThemePersistance::instance();
  tp->refresh();
  std::vector<std::string> names = tp->getNames();

  new Choice(
      line, rect_t{}, names, 0, names.size() - 1,
      [=]() { return tp->getThemeIndex(); },
      [=](int value) {
        tp->setThemeIndex(value);
        tp->applyTheme(value);
        tp->setDefaultTheme(value);
        themeView->update();
      });

  grid.setColSpan(2);
  line = window->newLine(grid);
  themeView = new ThemeView(line);
}
