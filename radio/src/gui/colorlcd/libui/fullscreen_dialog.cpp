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

#include "edgetx.h"
#include "etx_lv_theme.h"
#include "hal/watchdog_driver.h"
#include "mainwindow.h"
#include "os/sleep.h"
#include "static.h"
#include "view_main.h"

FullScreenDialog::FullScreenDialog(
    uint8_t type, std::string title, std::string message, std::string action,
    const std::function<void(void)>& confirmHandler) :
    Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}),
    type(type),
    title(std::move(title)),
    message(std::move(message)),
    action(std::move(action)),
    confirmHandler(confirmHandler)
{
  setWindowFlag(OPAQUE);
  etx_solid_bg(lvobj, (type == WARNING_TYPE_ALERT) ? COLOR_THEME_WARNING_INDEX : COLOR_THEME_SECONDARY1_INDEX);

  // In case alert raised while splash screen is showing.
  cancelSplash();

  pushLayer();

  build();
}

void FullScreenDialog::build()
{
  auto div = new Window(this, {0, ALERT_FRAME_TOP, LCD_W, ALERT_FRAME_HEIGHT});
  div->setWindowFlag(NO_FOCUS);
  etx_solid_bg(div->getLvObj(), COLOR_THEME_PRIMARY2_INDEX);

  new StaticIcon(
      this, ALERT_BITMAP_LEFT, ALERT_BITMAP_TOP,
      (type == WARNING_TYPE_INFO) ? ICON_BUSY : ICON_ERROR,
      COLOR_THEME_WARNING_INDEX);

  std::string t;
  if (type == WARNING_TYPE_ALERT) {
#if defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_IT) || \
    defined(TRANSLATIONS_CZ)
    t = std::string(STR_WARNING) + "\n" + title;
#else
    t = title + "\n" + STR_WARNING;
#endif
  } else if (!title.empty()) {
    t = title;
  }
  new StaticText(this,
                 rect_t{ALERT_TITLE_LEFT, ALERT_TITLE_TOP,
                        LCD_W - ALERT_TITLE_LEFT - PAD_MEDIUM,
                        LCD_H - ALERT_TITLE_TOP - PAD_MEDIUM},
                 t.c_str(), COLOR_THEME_WARNING_INDEX, FONT(XL));

  messageLabel =
      new StaticText(this,
                     rect_t{ALERT_MESSAGE_LEFT, ALERT_MESSAGE_TOP,
                            LCD_W - ALERT_MESSAGE_LEFT - PAD_MEDIUM,
                            LCD_H - ALERT_MESSAGE_TOP - PAD_MEDIUM},
                     message.c_str(), COLOR_THEME_PRIMARY1_INDEX, FONT(BOLD));

  if (!action.empty()) {
    auto btn = new TextButton(
        this, {(LCD_W - ONEBTN_W) / 2, LCD_H - ONEBTN_H - PAD_LARGE, ONEBTN_W, ONEBTN_H}, action.c_str(),
        [=]() {
          closeDialog();
          return 0;
        });
    etx_bg_color(btn->getLvObj(), COLOR_THEME_SECONDARY3_INDEX);
    etx_txt_color(btn->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);
  } else {
    if (type == WARNING_TYPE_CONFIRM) {
      auto btn = new TextButton(
          this, {LCD_W / 3 - TWOBTN_W / 2, LCD_H - TWOBTN_H - PAD_LARGE, TWOBTN_W, TWOBTN_H}, STR_CANCEL,
          [=]() {
            deleteLater();
            return 0;
          });
      etx_bg_color(btn->getLvObj(), COLOR_THEME_SECONDARY3_INDEX);
      etx_txt_color(btn->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);
      btn = new TextButton(
          this, {LCD_W * 2 / 3 - TWOBTN_W / 2, LCD_H - TWOBTN_H - PAD_LARGE, TWOBTN_W, TWOBTN_H}, STR_OK,
          [=]() {
            closeDialog();
            return 0;
          });
      etx_bg_color(btn->getLvObj(), COLOR_THEME_SECONDARY3_INDEX);
      etx_txt_color(btn->getLvObj(), COLOR_THEME_PRIMARY1_INDEX);
    }
  }
}

void FullScreenDialog::closeDialog()
{
  if (confirmHandler) confirmHandler();
  deleteLater();
}

bool FullScreenDialog::onLongPress()
{
  closeDialog();
  lv_indev_wait_release(lv_indev_get_act());
  return false;
}

void FullScreenDialog::onEvent(event_t event)
{
  // Buttons other than RTN or ENTER
  if (type == WARNING_TYPE_ALERT) {
    closeDialog();
    killEvents(event);
  }
}

void FullScreenDialog::onCancel() { deleteLater(); }

void FullScreenDialog::checkEvents()
{
  Window::checkEvents();
  if (closeCondition && closeCondition()) {
    deleteLater();
  }
}

void FullScreenDialog::deleteLater()
{
  if (_deleted) return;

  if (running) {
    running = false;
  } else {
    Window::deleteLater();
  }
}

void FullScreenDialog::setMessage(const char* text)
{
  if (messageLabel) messageLabel->setText(text);
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
        sleep_ms(1);
        continue;
      }
    }

    checkBacklight();
    WDG_RESET();

    MainWindow::instance()->run();
    sleep_ms(10);
  }

  deleteLater();
}

void raiseAlert(const char* title, const char* msg, const char* action,
                uint8_t sound)
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
