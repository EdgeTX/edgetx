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
#include "ui_simulateduiwidgetNV14.h"

SimulatedUIWidgetNV14::SimulatedUIWidgetNV14(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetNV14)
{
  RadioUiAction * act;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  act = new RadioUiAction(KEY_SYS, QList<int>() << Qt::Key_Up, SIMU_STR_HLP_KEY_UP, SIMU_STR_HLP_ACT_SYS);
  addRadioWidget(ui->leftbuttons->addArea(QRect(QPoint(65, 15), QSize(40, 40)), "NV14/lhs_hat_up.png", act));

  // unused
  //act = new RadioUiAction(13, QList<int>() << Qt::Key_Left, SIMU_STR_HLP_KEY_LFT, SIMU_STR_HLP_ACT_EXIT);
  //addRadioWidget(ui->leftbuttons->addArea(QRect(QPoint(5, 75), QSize(40, 40)), "NV14/lhs_hat_left.png", act));

  act = new RadioUiAction(KEY_MODEL, QList<int>() << Qt::Key_Right, SIMU_STR_HLP_KEY_RGT, SIMU_STR_HLP_ACT_MDL);
  addRadioWidget(ui->leftbuttons->addArea(QRect(QPoint(130, 75), QSize(40, 40)), "NV14/lhs_hat_right.png", act));

  act = new RadioUiAction(KEY_TELE, QList<int>() << Qt::Key_Down, SIMU_STR_HLP_KEY_DN, SIMU_STR_HLP_ACT_TELE);
  addRadioWidget(ui->leftbuttons->addArea(QRect(QPoint(65, 140), QSize(40, 40)), "NV14/lhs_hat_down.png", act));

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(QPoint(60, 70), QSize(50, 50)), "NV14/lhs_hat_press.png", act));

  // non-functional
  // act = new RadioUiAction(KEY_UP, QList<int>() << Qt::Key_Plus, SIMU_STR_HLP_KEY_PLS, SIMU_STR_HLP_ACT_PREVINC);
  // addRadioWidget(ui->rightbuttons->addArea(QRect(QPoint(95, 15), QSize(40, 40)), "NV14/rhs_hat_up.png", act));

  act = new RadioUiAction(KEY_PAGEUP, QList<int>() << Qt::Key_PageUp, SIMU_STR_HLP_KEY_PGDN, SIMU_STR_HLP_ACT_PGDN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(QPoint(35, 75), QSize(40, 40)), "NV14/rhs_hat_left.png", act));

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_PageDown, SIMU_STR_HLP_KEY_PGUP, SIMU_STR_HLP_ACT_PGUP);
  addRadioWidget(ui->rightbuttons->addArea(QRect(QPoint(160, 75), QSize(40, 40)), "NV14/rhs_hat_right.png", act));

  // non-functional
  // act = new RadioUiAction(KEY_DOWN, QList<int>() << Qt::Key_Minus, SIMU_STR_HLP_KEY_MIN, SIMU_STR_HLP_ACT_NEXTDEC);
  // addRadioWidget(ui->rightbuttons->addArea(QRect(QPoint(95, 140), QSize(40, 40)), "NV14/rhs_hat_down.png", act));

  act = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ENT);
  addRadioWidget(ui->rightbuttons->addArea(QRect(QPoint(90, 70), QSize(50, 50)), "NV14/rhs_hat_press.png", act));

  m_scrollUpAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Minus, SIMU_STR_HLP_KEY_MIN % "|" % SIMU_STR_HLP_MOUSE_UP, SIMU_STR_HLP_ACT_ROT_LFT);
  m_scrollDnAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Plus << Qt::Key_Equal, SIMU_STR_HLP_KEY_PLS % "|" % SIMU_STR_HLP_MOUSE_DN, SIMU_STR_HLP_ACT_ROT_RGT);
  connectScrollActions();

  m_backlightColors << QColor(47, 123, 227);

  setLcd(ui->lcd);
}

SimulatedUIWidgetNV14::~SimulatedUIWidgetNV14()
{
  delete ui;
}
