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

#include "etx_lv_theme.h"
#include "keyboard_text.h"
#include "mainwindow.h"
#include "page.h"
#include "view_main.h"

static int changeEndCalls = 0;

class KeyboardTestField : public FormField
{
 public:
  KeyboardTestField(Window* parent) :
      FormField(parent, {0, 0, 100, 0}, etx_textarea_create)
  {
  }

  void changeEnd(bool forceChanged = false) override { changeEndCalls += 1; }
};

class KeyboardTestPage : public Page
{
 public:
  KeyboardTestPage() : Page(ICON_MODEL) { field = new KeyboardTestField(body); }

  KeyboardTestField* field;
};

// While open, the keyboard attaches itself to the field's full screen window,
// so closing that window (e.g. touching the back button) also closes the
// keyboard. The field edit must still be completed, and the keyboard must
// not be left as the active keyboard once destroyed.
TEST(Keyboard, closePageWithKeyboardOpen)
{
  // Dialogs and pages are always shown over the main view
  ViewMain::instance();
  MainWindow::instance()->run();

  auto page = new KeyboardTestPage();
  MainWindow::instance()->run();
  TextKeyboard::open(page->field);
  MainWindow::instance()->run();
  ASSERT_NE(Keyboard::keyboardWindow(), nullptr);

  changeEndCalls = 0;
  page->closeWindow();

  EXPECT_EQ(changeEndCalls, 1);
  // Must not continue if the (soon to be destroyed) keyboard is still active
  ASSERT_EQ(Keyboard::keyboardWindow(), nullptr);

  MainWindow::instance()->run();

  // Keyboard can be opened again on a new page
  auto page2 = new KeyboardTestPage();
  MainWindow::instance()->run();
  TextKeyboard::open(page2->field);
  MainWindow::instance()->run();
  ASSERT_NE(Keyboard::keyboardWindow(), nullptr);
  EXPECT_EQ(Keyboard::keyboardWindow()->getParent(), page2);
  EXPECT_TRUE(page2->field->isEditMode());

  page2->closeWindow();
  MainWindow::instance()->run();
  EXPECT_EQ(Keyboard::keyboardWindow(), nullptr);
}

#endif
