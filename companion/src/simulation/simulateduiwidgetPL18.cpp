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

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetPL18.h"

SimulatedUIWidgetPL18::SimulatedUIWidgetPL18(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetPL18)
{
  RadioUiAction * act;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  //  Note: the PL18 has no physical buttons though at some point the trim joystick is repurposed
  //        allow for colorlcd key events and see what works
  //        the mouse click areas do not map to visual buttons on the background images

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_Up, SIMU_STR_HLP_KEY_UP, SIMU_STR_HLP_ACT_MDL);
  addRadioWidget(ui->rightbuttons->addArea(QRect(10, 1, 80, 35), "PL18/left.png", act));

  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(10, 40, 80, 35), "PL18/left.png", m_mouseMidClickAction));

  act = new RadioUiAction(KEY_UP, QList<int>() << Qt::Key_Left, SIMU_STR_HLP_KEY_LFT, SIMU_STR_HLP_ACT_SYS);
  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 80, 80, 35), "PL18/left.png", act));

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_Right, SIMU_STR_HLP_KEY_RGT, SIMU_STR_HLP_ACT_TELE);
  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 120, 80, 35), "PL18/left.png", act));

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_PageDown, SIMU_STR_HLP_KEY_PGDN, SIMU_STR_HLP_ACT_PGDN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 160, 80, 35), "PL18/left.png", act));

  act = new RadioUiAction(KEY_MENU, QList<int>() << Qt::Key_PageUp, SIMU_STR_HLP_KEY_PGUP, SIMU_STR_HLP_ACT_PGUP);
  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 200, 80, 35), "PL18/left.png", act));

  act = new RadioUiAction(KEY_PAGEUP, QList<int>() << Qt::Key_Down << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace,
                          SIMU_STR_HLP_KEY_DN % "<br>" % SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_RTN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 240, 80, 35), "PL18/left.png", act));

  m_scrollUpAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Minus, SIMU_STR_HLP_KEY_MIN % "|" % SIMU_STR_HLP_MOUSE_UP, SIMU_STR_HLP_ACT_ROT_LFT);
  m_scrollDnAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Plus << Qt::Key_Equal, SIMU_STR_HLP_KEY_PLS % "|" % SIMU_STR_HLP_MOUSE_DN, SIMU_STR_HLP_ACT_ROT_RGT);
  connectScrollActions();

  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 280, 30, 30), "PL18/left.png", m_screenshotAction));

  m_backlightColors << QColor(47, 123, 227);

  setLcd(ui->lcd);
}

SimulatedUIWidgetPL18::~SimulatedUIWidgetPL18()
{
  delete ui;
}
