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

#include "simulateduiwidget.h"
#include "ui_simulateduiwidgetJumperTLITE.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetJumperTLITE::SimulatedUIWidgetJumperTLITE(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetJumperTLITE)
{
  RadioUiAction * act;

  ui->setupUi(this);

  act = new RadioUiAction(KEY_UP, QList<int>() << Qt::Key_Up << Qt::Key_PageUp, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_UP);
  addRadioWidget(ui->rightbuttons->addArea(QRect(40, 15, 70, 50), "JumperTLITE/right_top.png", act));

  act = new RadioUiAction(KEY_DOWN, QList<int>() << Qt::Key_Down << Qt::Key_PageDown, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_DN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(47, 65, 70, 50), "JumperTLITE/right_bottom.png", act));

  act = new RadioUiAction(KEY_RIGHT, QList<int>() << Qt::Key_Right << Qt::Key_Plus, SIMU_STR_HLP_KEY_RGT % "|" % SIMU_STR_HLP_KEY_PLS, SIMU_STR_HLP_ACT_PLS);
  addRadioWidget(ui->bottombuttons->addArea(QRect(150, 73, 80, 30), "JumperTLITE/bottom_right.png", act));

  act = new RadioUiAction(KEY_LEFT, QList<int>() << Qt::Key_Left << Qt::Key_Minus, SIMU_STR_HLP_KEY_LFT % "|" % SIMU_STR_HLP_KEY_MIN, SIMU_STR_HLP_ACT_MIN);
  addRadioWidget(ui->bottombuttons->addArea(QRect(24, 73, 80, 30), "JumperTLITE/bottom_left.png", act));

  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ENT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(5, 15, 70, 50), "JumperTLITE/left_top.png", m_mouseMidClickAction));

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 65, 70, 50), "JumperTLITE/left_bottom.png", act));

  //addRadioWidget(ui->leftbuttons->addArea(QRect(10, 65, 70, 50), "JumperTLITE/left_scrnshot.png", m_screenshotAction));

  m_backlightColors << QColor(215, 243, 255);  // X7 Blue
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

  setLcd(ui->lcd);

  QString css = "#radioUiWidget {"
                "background-color: qlineargradient(spread:reflect, x1:0, y1:0, x2:0, y2:1,"
                "stop:0 rgba(255, 255, 255, 255),"
                "stop:0.757062 rgba(241, 238, 238, 255),"
                "stop:1 rgba(247, 245, 245, 255));"
                "}";

  QTimer * tim = new QTimer(this);
  tim->setSingleShot(true);
  connect(tim, &QTimer::timeout, [this, css]() {
      emit customStyleRequest(css);
  });
  tim->start(100);
}

SimulatedUIWidgetJumperTLITE::~SimulatedUIWidgetJumperTLITE()
{
  delete ui;
}
