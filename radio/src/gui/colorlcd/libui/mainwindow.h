/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   libopenui - https://github.com/opentx/libopenui
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

#pragma once

#include "window.h"

class BitmapBuffer;

class MainWindow: public Window
{
 protected:
  // singleton
  MainWindow();

 public:
  static MainWindow * instance();

#if defined(TESTS)
  static void create()
  {
    _instance = new MainWindow();
  }
#endif

#if defined(DEBUG_WINDOWS)
  std::string getName() const override
  {
    return "MainWindow";
  }
#endif

  void run(bool trash = true);

  bool setBackgroundImage(std::string& fileName);

  void shutdown();

  void enableWidgetRefresh(bool state) { widgetRefreshEnable = state; }

  void blockUntilClose(bool checkPwr, std::function<bool(void)> closeCondition, bool isError = false);

 protected:
  lv_obj_t* background = nullptr;
  const BitmapBuffer *backgroundBitmap = nullptr;
  bool widgetRefreshEnable = true;

  static MainWindow * _instance;

  void emptyTrash();
};
