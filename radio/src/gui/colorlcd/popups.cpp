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

#include "opentx.h"
#include "popups.h"
#include "libopenui.h"
#include "pwr.h"

#include "watchdog_driver.h"

static void _run_popup_dialog(const char* title, const char* msg,
                              const char* info = nullptr)
{
  bool running = true;

  resetBacklightTimeout();

  // reset input devices to avoid
  // RELEASED/CLICKED to be called in a loop
  lv_indev_reset(nullptr, nullptr);

  auto md = new MessageDialog(MainWindow::instance(), title, msg);
  md->setCloseHandler([&]() { running = false; });
  if (info) {
    md->setInfoText(std::string(info));
  }
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

void POPUP_INFORMATION(const char * message)
{
  _run_popup_dialog("Message", message);
}

void POPUP_WARNING(const char * message, const char * info)
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

void POPUP_WARNING_ON_UI_TASK(const char * message, const char * info, bool waitForClose)
{
  // if already in a popup, and we don't want to wait, ignore call
  if (!waitForClose && ui_popup_active)
    return;

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
