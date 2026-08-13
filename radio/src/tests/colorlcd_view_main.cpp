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

#include "gtests.h"

#if defined(COLORLCD)

#include "gui/colorlcd/libui/mainwindow.h"
#include "gui/colorlcd/mainview/view_main.h"

namespace {

lv_timer_t* findTimerFor(const void* userData)
{
  for (lv_timer_t* timer = lv_timer_get_next(nullptr); timer;
       timer = lv_timer_get_next(timer)) {
    if (timer->user_data == userData) return timer;
  }

  return nullptr;
}

}  // namespace

TEST(ViewMain, deletionCancelsWidgetSelectionTimerBeforeTrashCollection)
{
  ViewMain* view = ViewMain::instance();
  const void* timerOwner = view;
  ASSERT_EQ(findTimerFor(timerOwner), nullptr);

  view->refreshWidgetSelectTimer();
  ASSERT_NE(findTimerFor(timerOwner), nullptr);

  view->deleteLater();
  view->deleteLater();
  EXPECT_EQ(findTimerFor(timerOwner), nullptr);

  // Flush the deleted ViewMain from the Window trash list before cleaning up a
  // stale timer left by the code under test.
  MainWindow::instance()->shutdown();

  if (lv_timer_t* staleTimer = findTimerFor(timerOwner)) {
    lv_timer_del(staleTimer);
  }
}

#endif
