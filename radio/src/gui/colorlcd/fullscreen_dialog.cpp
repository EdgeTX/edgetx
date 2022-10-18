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

#include "fullscreen_dialog.h"
#include "LvglWrapper.h"
#include "mainwindow.h"
#include "opentx.h"
#include "libopenui.h"

#include "watchdog_driver.h"

static Window* _get_parent()
{
  Window* p = Layer::back();
  if (!p) p = MainWindow::instance();
  return p;
}

FullScreenDialog::FullScreenDialog(
    uint8_t type, std::string title, std::string message, std::string action,
    const std::function<void(void)>& confirmHandler) :
    Window(_get_parent(), {0, 0, LCD_W, LCD_H}, OPAQUE),
    type(type),
    title(std::move(title)),
    message(std::move(message)),
    action(std::move(action)),
    confirmHandler(confirmHandler)
{
  Layer::push(this);

  bringToTop();
  lv_obj_add_event_cb(lvobj, FullScreenDialog::long_pressed, LV_EVENT_LONG_PRESSED, nullptr);
}

void FullScreenDialog::init()
{
  if (!loaded) {
    loaded = true;

    std::string t;
    if (type == WARNING_TYPE_ALERT) {
#if defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_IT) || defined(TRANSLATIONS_CZ)
      t = std::string(STR_WARNING) + "\n" + title;
#else
      t = title + "\n" + STR_WARNING;
#endif
    } else if (!title.empty()) {
      t = title;
    }
    new StaticText(this, 
                   rect_t{ALERT_TITLE_LEFT, ALERT_TITLE_TOP, LCD_W - ALERT_TITLE_LEFT - PAGE_PADDING, LCD_H - ALERT_TITLE_TOP - PAGE_PADDING},
                   t.c_str(), 0, COLOR_THEME_WARNING | FONT(XL));

    messageLabel = new StaticText(this, 
                   rect_t{ALERT_MESSAGE_LEFT, ALERT_MESSAGE_TOP, LCD_W - ALERT_MESSAGE_LEFT - PAGE_PADDING, LCD_H - ALERT_MESSAGE_TOP - PAGE_PADDING},
                   message.c_str(), 0, COLOR_THEME_PRIMARY1 | FONT(BOLD));

    if (!action.empty()) {
      auto btn = new TextButton(this, { (LCD_W - 280) / 2, LCD_H - 48, 280, 40 }, action.c_str(),
                     [=]() {
                       closeDialog();
                       return 0;
                     }, COLOR_THEME_PRIMARY1 | FONT(BOLD));
      lv_obj_set_style_bg_color(btn->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), 0);
      lv_obj_set_style_bg_opa(btn->getLvObj(), LV_OPA_COVER, 0);
      lv_obj_set_style_text_color(btn->getLvObj(), makeLvColor(COLOR_THEME_PRIMARY1), 0);
    } else {
      if (type == WARNING_TYPE_CONFIRM) {
        auto btn = new TextButton(this, { LCD_W / 3 - 50, LCD_H - 48, 100, 40 }, STR_EXIT,
                       [=]() {
                         deleteLater();
                         return 0;
                       }, COLOR_THEME_PRIMARY1 | FONT(BOLD));
        lv_obj_set_style_bg_color(btn->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), 0);
        lv_obj_set_style_bg_opa(btn->getLvObj(), LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(btn->getLvObj(), makeLvColor(COLOR_THEME_PRIMARY1), 0);
        btn = new TextButton(this, { LCD_W * 2 / 3 - 50, LCD_H - 48, 100, 40 }, STR_OK,
                       [=]() {
                         closeDialog();
                         return 0;
                       }, COLOR_THEME_PRIMARY1 | FONT(BOLD));
        lv_obj_set_style_bg_color(btn->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), 0);
        lv_obj_set_style_bg_opa(btn->getLvObj(), LV_OPA_COVER, 0);
        lv_obj_set_style_text_color(btn->getLvObj(), makeLvColor(COLOR_THEME_PRIMARY1), 0);
      }
    }
  }
}

void FullScreenDialog::paint(BitmapBuffer * dc)
{
  init();

  OpenTxTheme::instance()->drawBackground(dc);

  dc->drawFilledRect(0, ALERT_FRAME_TOP, LCD_W, ALERT_FRAME_HEIGHT, SOLID,
                     COLOR_THEME_PRIMARY2, OPACITY(8));

  if (type == WARNING_TYPE_ALERT || type == WARNING_TYPE_ASTERISK) {
    dc->drawMask(ALERT_BITMAP_LEFT, ALERT_BITMAP_TOP,
                 OpenTxTheme::instance()->error, COLOR_THEME_WARNING);
  } else if (type == WARNING_TYPE_INFO) {
    dc->drawMask(ALERT_BITMAP_LEFT, ALERT_BITMAP_TOP,
                 OpenTxTheme::instance()->busy, COLOR_THEME_WARNING);
  } else { // confirmation
    dc->drawMask(ALERT_BITMAP_LEFT, ALERT_BITMAP_TOP,
                 OpenTxTheme::instance()->error, COLOR_THEME_WARNING);
  }
}

void FullScreenDialog::closeDialog()
{
 if (confirmHandler)
   confirmHandler();
 deleteLater();
}

void FullScreenDialog::long_pressed(lv_event_t* e)
{
  auto obj = lv_event_get_target(e);
  auto fs = (FullScreenDialog*)lv_obj_get_user_data(obj);

  if (fs) {
    fs->closeDialog();
    lv_indev_wait_release(lv_indev_get_act());
  }
}

void FullScreenDialog::onEvent(event_t event)
{
  // Buttons other than RTN or ENTER
  if (type == WARNING_TYPE_ALERT) {
    closeDialog();
    killEvents(event);
  }
}

void FullScreenDialog::onCancel()
{
  deleteLater();
}

void FullScreenDialog::checkEvents()
{
  Window::checkEvents();
  if (closeCondition && closeCondition()) {
    deleteLater();
  }
}

void FullScreenDialog::deleteLater(bool detach, bool trash)
{
  if (running) {
    running = false;
  }
  else {
    Layer::pop(this);
    Window::deleteLater(detach, trash);
  }
}

static void run_ui_manually()
{
  checkBacklight();
  WDG_RESET();

  RTOS_WAIT_MS(10);
  LvglWrapper::runNested();
  MainWindow::instance()->run(false);
}

void FullScreenDialog::runForever(bool checkPwr)
{
  running = true;

  // reset input devices to avoid
  // RELEASED/CLICKED to be called in a loop
  lv_indev_reset(nullptr, nullptr);
  
  while (running) {
    resetBacklightTimeout();

    if (checkPwr) {
      auto check = pwrCheck();
      if (check == e_power_off) {
        boardOff();
#if defined(SIMU)
        return;
#endif
      } else if (check == e_power_press) {
        WDG_RESET();
        RTOS_WAIT_MS(1);
        continue;
      }
    }

    run_ui_manually();
  }

  deleteLater();
}

void raiseAlert(const char * title, const char * msg, const char * action, uint8_t sound)
{
  TRACE("raiseAlert('%s')", msg);
  AUDIO_ERROR_MESSAGE(sound);
  LED_ERROR_BEGIN();
  auto dialog = new FullScreenDialog(WARNING_TYPE_ALERT, title ? title : "",
                                     msg ? msg : "", action ? action : "");
  dialog->runForever();
  LED_ERROR_END();
}

// POPUP_CONFIRMATION
bool confirmationDialog(const char* title, const char* msg, bool checkPwr,
                        const std::function<bool(void)>& closeCondition)
{
  bool confirmed = false;
  auto dialog = new FullScreenDialog(WARNING_TYPE_CONFIRM, title ? title : "",
                                     msg ? msg : "", "",
                                     [&confirmed]() { confirmed = true; });
  if (closeCondition) {
    dialog->setCloseCondition([&confirmed, &closeCondition]() {
      if (closeCondition()) {
        confirmed = true;
        return true;
      } else {
        return false;
      }
    });
  }

  dialog->runForever(checkPwr);

  return confirmed;
}
