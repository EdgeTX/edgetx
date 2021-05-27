/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#ifndef _LIBOPENUI_H_
#define _LIBOPENUI_H_

#include "libopenui_config.h"
#include "libopenui_file.h"
#include "font.h"
#include "window.h"
#include "mainwindow.h"
#include "static.h"
#include "button.h"
#include "fab_button.h"
#include "checkbox.h"
#include "numberedit.h"
#include "timeedit.h"
#include "choice.h"
#include "sourcechoice.h"
#include "switchchoice.h"
#include "filechoice.h"
#include "textedit.h"
#include "slider.h"
#include "progress.h"
#include "keyboard_text.h"
#include "keyboard_number.h"
#include "keyboard_curve.h"
#include "tabsgroup.h"
#include "page.h"
#include "menu.h"
#include "dialog.h"
#include "fullscreen_dialog.h"
#include "message_dialog.h"
#include "confirm_dialog.h"
#include "dialog.h"
#include "gridlayout.h"
#include "getset_helpers.h"
#include "curveedit.h"
#include "coloredit.h"
#include "draw_functions.h"
#include "textedits.h"

inline MessageDialog * createPopupInformation(const char * message)
{
  return new MessageDialog(MainWindow::instance(), "Message", message);
}

inline MessageDialog * createPopupWarning(const char * message)
{
  return new MessageDialog(MainWindow::instance(), "Warning", message);
}

inline void POPUP_INFORMATION(const char * message)
{
  auto popup = createPopupInformation(message);
  while (popup->getParent()) {
    WDG_RESET();
    MainWindow::instance()->run(false);
  }
}

inline void POPUP_WARNING(const char * message, const char * info = nullptr)
{
  auto popup = createPopupWarning(message);
  if (info) {
    popup->setInfoText(std::string(info));
  }
  while (popup->getParent()) {
    WDG_RESET();
    MainWindow::instance()->run(false);
  }
}

#endif // _LIBOPENUI_H_
