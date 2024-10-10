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

#include "popups.h"

#include "libopenui.h"
#include "edgetx.h"
#include "pwr.h"
#include "hal/watchdog_driver.h"
#include "etx_lv_theme.h"

static void _run_popup_dialog(const char* title, const char* msg,
                              const char* info = nullptr)
{
  bool running = true;

  resetBacklightTimeout();

  // reset input devices to avoid
  // RELEASED/CLICKED to be called in a loop
  lv_indev_reset(nullptr, nullptr);

  auto md = new MessageDialog(title, msg, info);
  md->setCloseHandler([&]() { running = false; });
  while (running) {
    // Allow power off while showing popup
    auto check = pwrCheck();
    if (check == e_power_off) {
      boardOff();
#if defined(SIMU)
      // Required so simulator exits cleanly when window closed
      return;
#endif
    } else if (check == e_power_press) {
      WDG_RESET();
      RTOS_WAIT_MS(1);
      continue;
    }

    checkBacklight();
    WDG_RESET();
    MainWindow::instance()->run();
    LvglWrapper::runNested();
    RTOS_WAIT_MS(20);
  }
}

void POPUP_INFORMATION(const char* message)
{
  _run_popup_dialog("Message", message);
}

void POPUP_WARNING(const char* message, const char* info)
{
  _run_popup_dialog("Warning", message, info);
}

static const char* ui_popup_title = nullptr;
static const char* ui_popup_msg = nullptr;
static const char* ui_popup_info = nullptr;
static bool ui_popup_active = false;

// Allow UI task to show a popup deferred from another task.
void show_ui_popup()
{
  if (ui_popup_active) {
    _run_popup_dialog(ui_popup_title, ui_popup_msg, ui_popup_info);
    ui_popup_active = false;
  }
}

void POPUP_WARNING_ON_UI_TASK(const char* message, const char* info,
                              bool waitForClose)
{
  // if already in a popup, and we don't want to wait, ignore call
  if (!waitForClose && ui_popup_active) return;

  // Wait in case already in popup.
  while (ui_popup_active) {
    RTOS_WAIT_MS(20);
  }
  ui_popup_title = "Warning";
  ui_popup_msg = message;
  ui_popup_info = info;
  ui_popup_active = true;

  // Wait until closed
  if (waitForClose) {
    while (ui_popup_active) {
      RTOS_WAIT_MS(20);
    }
  }
}

// Bubble popup style
static const lv_style_const_prop_t bubble_popup_props[] = {
    LV_STYLE_CONST_BORDER_OPA(LV_OPA_COVER),
    LV_STYLE_CONST_BORDER_WIDTH(3),
    LV_STYLE_CONST_RADIUS(10),
    LV_STYLE_PROP_INV,
};
LV_STYLE_CONST_MULTI_INIT(bubble_popup, bubble_popup_props);

static void bubble_popup_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->bg_opacity_cover, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->pad_small, LV_PART_MAIN);
  etx_obj_add_style(obj, bubble_popup, LV_PART_MAIN);
  etx_bg_color(obj, COLOR_WHITE_INDEX, LV_PART_MAIN);
  etx_txt_color(obj, COLOR_BLACK_INDEX, LV_PART_MAIN);
  etx_obj_add_style(obj, styles->border_color[COLOR_BLACK_INDEX], LV_PART_MAIN);
}

static const lv_obj_class_t bubble_popup_class = {
    .base_class = &window_base_class,
    .constructor_cb = bubble_popup_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_DPI_DEF,
    .height_def = LV_DPI_DEF,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t)};

static lv_obj_t* bubble_popup_create(lv_obj_t* parent)
{
  return etx_create(&bubble_popup_class, parent);
}

class BubbleDialog : public Window
{
 public:
  BubbleDialog(const char* message, int timeout, coord_t width) :
      Window(MainWindow::instance(), rect_t{(LCD_W - width) / 2, LCD_H - 100, width, 50},
             bubble_popup_create)
  {
    setWindowFlag(OPAQUE);

    lv_obj_set_parent(lvobj, lv_layer_top());

    auto label = lv_label_create(lvobj);
    lv_label_set_text(label, message);
    lv_obj_center(label);
    lv_obj_set_width(label, lv_pct(100));
    etx_obj_add_style(label, styles->text_align_center, LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);

    endTime = RTOS_GET_MS() + timeout;
  }

  bool isBubblePopup() override { return true; }

  void checkEvents() override
  {
    if (RTOS_GET_MS() >= endTime) {
      deleteLater();
    }
  }

 protected:
  uint32_t endTime;
};

void POPUP_BUBBLE(const char* message, uint32_t timeout, coord_t width)
{
  new BubbleDialog(message, timeout, width);
}
