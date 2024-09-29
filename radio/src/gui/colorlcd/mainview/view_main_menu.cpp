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

#include "view_main_menu.h"

#include "menu_model.h"
#include "menu_radio.h"
#include "menu_screen.h"
#include "model_select.h"
#include "edgetx.h"
#include "select_fab_carousel.h"
#include "etx_lv_theme.h"
#include "view_about.h"
#include "view_channels.h"
#include "view_statistics.h"
#include "view_text.h"

static void modal_dialog_constructor(const lv_obj_class_t* class_p,
                                     lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->rounded, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->bg_opacity_75, LV_PART_MAIN);
  etx_bg_color(obj, COLOR_BLACK_INDEX, LV_PART_MAIN);
}

static const lv_obj_class_t etx_modal_dialog_class = {
    .base_class = &window_base_class,
    .constructor_cb = modal_dialog_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = 0,
    .height_def = 0,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .instance_size = sizeof(lv_obj_t)};

static lv_obj_t* etx_modal_dialog_create(lv_obj_t* parent)
{
  return etx_create(&etx_modal_dialog_class, parent);
}

ViewMainMenu::ViewMainMenu(Window* parent, std::function<void()> closeHandler) :
    Window(parent, {0, 0, LCD_W, LCD_H}),
    closeHandler(std::move(closeHandler))
{
  // Save focus
  Layer::push(this);

  coord_t w = SelectFabCarousel::FAB_BUTTON_WIDTH * QM_COLS + PAD_LARGE * 2;
  coord_t h = SelectFabCarousel::FAB_BUTTON_HEIGHT * QM_ROWS + PAD_LARGE * 2;

  bool hasNotes = modelHasNotes();

#if !PORTRAIT_LCD
  if (hasNotes)
    w += SelectFabCarousel::FAB_BUTTON_WIDTH;
#endif

  auto box =
      new Window(this, {(LCD_W - w) / 2, (LCD_H - h) / 2, w, h},
                 etx_modal_dialog_create);
  box->padAll(PAD_LARGE);

  auto carousel = new SelectFabCarousel(box);
  carousel->addButton(ICON_MODEL_SELECT, STR_MAIN_MENU_MANAGE_MODELS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ModelLabelsWindow();
                        return 0;
                      });

  if (hasNotes) {
    carousel->addButton(ICON_MODEL_NOTES, STR_MAIN_MENU_MODEL_NOTES,
                        [=]() -> uint8_t {
                          deleteLater();
                          readModelNotes(true);
                          return 0;
                        });
  }

  carousel->addButton(ICON_MONITOR, STR_MAIN_MENU_CHANNEL_MONITOR,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ChannelsViewMenu();
                        return 0;
                      });

  carousel->addButton(ICON_MODEL, STR_MAIN_MENU_MODEL_SETTINGS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ModelMenu();
                        return 0;
                      });

  carousel->addButton(ICON_RADIO, STR_MAIN_MENU_RADIO_SETTINGS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new RadioMenu();
                        return 0;
                      });

  carousel->addButton(ICON_THEME, STR_MAIN_MENU_SCREEN_SETTINGS,
                      [=]() -> uint8_t {
                        deleteLater();
                        new ScreenMenu();
                        return 0;
                      });

  carousel->addButton(
      ICON_MODEL_TELEMETRY, STR_MAIN_MENU_RESET_TELEMETRY, [=]() -> uint8_t {
        deleteLater();
        Menu* resetMenu = new Menu();
        resetMenu->addLine(STR_RESET_FLIGHT, []() { flightReset(); });
        resetMenu->addLine(STR_RESET_TIMER1, []() { timerReset(0); });
        resetMenu->addLine(STR_RESET_TIMER2, []() { timerReset(1); });
        resetMenu->addLine(STR_RESET_TIMER3, []() { timerReset(2); });
        resetMenu->addLine(STR_RESET_TELEMETRY, []() { telemetryReset(); });
        return 0;
      });

  carousel->addButton(ICON_STATS, STR_MAIN_MENU_STATISTICS, [=]() -> uint8_t {
    deleteLater();
    new StatisticsViewPageGroup();
    return 0;
  });

  carousel->addButton(ICON_EDGETX, STR_MAIN_MENU_ABOUT_EDGETX,
                      [=]() -> uint8_t {
                        deleteLater();
                        new AboutUs();
                        return 0;
                      });
}

void ViewMainMenu::deleteLater(bool detach, bool trash)
{
  if (closeHandler) closeHandler();
  Layer::pop(this);
  Window::deleteLater(detach, trash);
}

void ViewMainMenu::onClicked() { deleteLater(); }

void ViewMainMenu::onCancel() { deleteLater(); }
