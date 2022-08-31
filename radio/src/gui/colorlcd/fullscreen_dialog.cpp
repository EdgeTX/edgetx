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

FullScreenDialog::FullScreenDialog(
    uint8_t type, std::string title, std::string message, std::string action,
    const std::function<void(void)>& confirmHandler) :
    Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H}, OPAQUE),
    type(type),
    title(std::move(title)),
    message(std::move(message)),
    action(std::move(action)),
    confirmHandler(confirmHandler)
{
  Layer::push(this);
  
//   // TODO: if 'confirmHandler':
//   //       -> add a confirm / cancel button
//   //
// #if defined(HARDWARE_TOUCH) && 0
//   new FabButton(this, LCD_W - (FAB_BUTTON_SIZE + PAGE_PADDING),
//                 LCD_H - (FAB_BUTTON_SIZE + PAGE_PADDING), ICON_NEXT,
//                 [=]() -> uint8_t {
//                   deleteLater();
//                   if (confirmHandler) confirmHandler();
//                   return 0;
//                 });
// #endif

  bringToTop();
}

void FullScreenDialog::paint(BitmapBuffer * dc)
{
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

  if (type == WARNING_TYPE_ALERT) {
#if defined(TRANSLATIONS_FR) || defined(TRANSLATIONS_IT) || \
    defined(TRANSLATIONS_CZ)
    dc->drawText(ALERT_TITLE_LEFT, ALERT_TITLE_TOP, STR_WARNING,
                 COLOR_THEME_WARNING | FONT(XL));
    dc->drawText(ALERT_TITLE_LEFT, ALERT_TITLE_TOP + ALERT_TITLE_LINE_HEIGHT,
                 title.c_str(), COLOR_THEME_WARNING | FONT(XL));
#else
    dc->drawText(ALERT_TITLE_LEFT, ALERT_TITLE_TOP, title.c_str(),
                 COLOR_THEME_WARNING | FONT(XL));
    dc->drawText(ALERT_TITLE_LEFT, ALERT_TITLE_TOP + ALERT_TITLE_LINE_HEIGHT,
                 STR_WARNING, COLOR_THEME_WARNING | FONT(XL));
#endif
  } else if (!title.empty()) {
    dc->drawText(ALERT_TITLE_LEFT, ALERT_TITLE_TOP, title.c_str(),
                 COLOR_THEME_WARNING | FONT(XL));
  }

  if (!message.empty()) {
    dc->drawText(ALERT_MESSAGE_LEFT, ALERT_MESSAGE_TOP, message.c_str(),
                 FONT(BOLD));
  }

  if (!action.empty()) {
    dc->drawText(LCD_W / 2, ALERT_ACTION_TOP, action.c_str(),
                 CENTERED | FONT(BOLD));
  } else if (type == WARNING_TYPE_CONFIRM) {

    LcdFlags flags = FONT(BOLD);
    int w = getTextWidth(STR_OK, 0, flags);
    
    dc->drawText(ALERT_MESSAGE_LEFT, ALERT_ACTION_TOP,
                 STR_OK, flags);
    
    dc->drawText(ALERT_MESSAGE_LEFT + w + 20, ALERT_ACTION_TOP,
                 STR_EXIT, flags);
  }
}

void FullScreenDialog::onClicked()
{
  if (confirmHandler) confirmHandler();
  deleteLater();
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
  MainWindow::instance()->run();
}

void FullScreenDialog::runForever()
{
  running = true;

  // reset input devices to avoid
  // RELEASED/CLICKED to be called in a loop
  lv_indev_reset(nullptr, nullptr);
  
  if (type == WARNING_TYPE_ALERT) {
    forceBacklightOn = true;
  }

  while (running) {
    resetBacklightTimeout();

    auto check = pwrCheck();
    if (check == e_power_off) {
      boardOff();
#if defined(SIMU)
      return;
#endif
    }
    else if (check == e_power_press) {
      WDG_RESET();
      RTOS_WAIT_MS(1);
      continue;
    }
    run_ui_manually();
  }

  forceBacklightOn = false;
  deleteLater();
}

void FullScreenDialog::runForeverNoPwrCheck()
{
  running = true;

  if (type == WARNING_TYPE_ALERT) {
    forceBacklightOn = true;
  }

  while (running) {
    resetBacklightTimeout();
    run_ui_manually();
  }

  forceBacklightOn = false;
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

  if (checkPwr) {
    dialog->runForever();
  } else {
    dialog->runForeverNoPwrCheck();
  }

  return confirmed;
}
