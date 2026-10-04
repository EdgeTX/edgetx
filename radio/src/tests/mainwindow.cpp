/*
 * Copyright (C) EdgeTX
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

#include "mainwindow.h"
#include "view_main.h"

// MainWindow::blockUntilClose() runs a nested UI loop, typically from inside
// an event handler or checkEvents() call further up the stack. Windows closed
// during that loop must not be destroyed until control returns to the
// top-level loop, as callers poll e.g. dialog->deleted() in their close
// condition, and outer stack frames may still reference the closed windows.

// Dialogs are always shown over the main view
static void initMainView()
{
  ViewMain::instance();
  MainWindow::instance()->run();
}

static bool selfClosingDestroyed = false;

class SelfClosingWindow : public Window
{
 public:
  SelfClosingWindow() : Window(MainWindow::instance(), {0, 0, LCD_W, LCD_H})
  {
    setWindowFlag(OPAQUE);
    pushLayer();
  }

  ~SelfClosingWindow() override { selfClosingDestroyed = true; }

  void checkEvents() override { closeWindow(); }
};

TEST(MainWindow, blockUntilCloseDefersWindowDestruction)
{
  initMainView();

  selfClosingDestroyed = false;
  auto w = new SelfClosingWindow();

  bool destroyedWhileBlocked = false;
  MainWindow::instance()->blockUntilClose(false, [&]() {
    // Must not dereference 'w' if it has already been destroyed
    if (selfClosingDestroyed) {
      destroyedWhileBlocked = true;
      return true;
    }
    return w->deleted();
  });

  EXPECT_FALSE(destroyedWhileBlocked);

  // Top-level loop is now free to destroy it
  MainWindow::instance()->run();
  EXPECT_TRUE(selfClosingDestroyed);
}

// Moves a switch from within the blocking loop, simulating the user
// correcting the switch position while the switch warning is displayed.
class SwitchMover : public Window
{
 public:
  SwitchMover(uint8_t sw, int8_t state) :
      Window(MainWindow::instance(), {0, 0, 0, 0}), sw(sw), state(state)
  {
  }

  bool isBubblePopup() override { return true; }

  void checkEvents() override
  {
    if (++count == 3) {
      simuSetSwitch(sw, state);
      closeWindow();
    }
  }

 protected:
  uint8_t sw;
  int8_t state;
  int count = 0;
};

// Real-world path: the switch warning dialog closes itself once the switch
// is corrected, while checkSwitches() polls dialog->deleted(). Relies on
// ASAN (enabled by default for gtests-radio) to report a use-after-free.
TEST(MainWindow, switchWarningClosesSafely)
{
  RADIO_RESET();
  MODEL_RESET();
  initMainView();

  // Warn unless switch 0 is up; start with it down
  g_model.setSwitchWarning(0, 1);
  simuSetSwitch(0, 1);

  uint16_t bad_pots;
  ASSERT_TRUE(isSwitchWarningRequired(bad_pots));

  new SwitchMover(0, -1);
  checkSwitches();

  EXPECT_FALSE(isSwitchWarningRequired(bad_pots));

  MainWindow::instance()->run();
  g_model.setSwitchWarning(0, 0);
}

#endif
