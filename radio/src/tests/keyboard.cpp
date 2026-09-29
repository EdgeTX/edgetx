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
#include "textedit.h"

static int changeEndCalls = 0;

class KeyboardTestField : public FormField
{
 public:
  KeyboardTestField(Window* parent) :
      FormField(parent, {0, 0, 100, 0}, etx_textarea_create)
  {
  }

  void changeEnd() override { changeEndCalls += 1; }
};

class KeyboardTextEditField : public TextEdit
{
 public:
  KeyboardTextEditField(Window* parent) :
      TextEdit(parent, {0, 50, 100, 0}, text, 10)
  {
  }

  FormField* textArea() const { return (FormField*)edit; }

  char text[10] = "abc";
};

class KeyboardTestPage : public Page
{
 public:
  KeyboardTestPage() : Page(ICON_MODEL) {
    field = new KeyboardTestField(body);
    textEdit = new KeyboardTextEditField(body);
  }

  void clickTextEdit()
  {
    lv_event_send(textEdit->getLvObj(), LV_EVENT_CLICKED, nullptr);
  }

  void simulateCancelButtonClick()
  {
  // Send clicked event to page cancel button to close window
    Window *btn = nullptr;
    for (auto c : header->getChildren())
      if (c->left() == LCD_W - EdgeTxStyles::MENU_HEADER_HEIGHT && c->top() == 0) {
        btn = c;
        break;
      }
    EXPECT_NE(btn, nullptr);
    lv_event_send(btn->getLvObj(), LV_EVENT_CLICKED, nullptr);
  }

  FormField* textArea() const { return textEdit->textArea(); }

  KeyboardTestField* field;
  KeyboardTextEditField* textEdit;
};

// Keyboard is attached to the MainWindow::instnace().
// Closing a window with the keyboard open should hide the keyboard.
// The field edit must still be completed, and the keyboard must
// not be left as the active keyboard once hidden.
// Keyboad instance is not deleted, instance is re-used as needed.
TEST(Keyboard, closePageWithKeyboardOpen)
{
  // Dialogs and pages are always shown over the main view
  ViewMain::instance();
  MainWindow::instance()->run();

  auto page = new KeyboardTestPage();
  MainWindow::instance()->run();
  TextKeyboard::open(page->field);
  MainWindow::instance()->run();

  auto kbInstance = Keyboard::keyboardWindow();
  ASSERT_NE(kbInstance, nullptr);

  changeEndCalls = 0;
  page->simulateCancelButtonClick();

  EXPECT_EQ(changeEndCalls, 1);
  // Must not continue if the hiddem keyboard is still active
  ASSERT_EQ(Keyboard::keyboardWindow(), nullptr);

  MainWindow::instance()->run();

  // Keyboard can be opened again on a new page
  auto page2 = new KeyboardTestPage();
  MainWindow::instance()->run();
  page2->clickTextEdit();
  MainWindow::instance()->run();
  ASSERT_NE(Keyboard::keyboardWindow(), nullptr);
  ASSERT_EQ(Keyboard::keyboardWindow(), kbInstance);
  EXPECT_EQ(Keyboard::keyboardWindow()->getParent(), MainWindow::instance());
  EXPECT_TRUE(page2->textArea()->isEditMode());

  page2->simulateCancelButtonClick();
  MainWindow::instance()->run();
  EXPECT_EQ(Keyboard::keyboardWindow(), nullptr);
}

#endif
