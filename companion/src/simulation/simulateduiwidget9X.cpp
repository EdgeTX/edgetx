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
#include "ui_simulateduiwidget9X.h"
#include "eeprominterface.h"

SimulatedUIWidget9X::SimulatedUIWidget9X(SimulatorInterface * simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidget9X)
{
  RadioUiAction * act;
  QPolygon polygon;

  ui->setupUi(this);

  bool hasRotEnc = getCurrentFirmware()->getCapability(Capability::RotaryEncoders);

  // add actions in order of appearance on the help menu

  int x = 68, y = 91, oR = 63;

  polygon << QPoint(x, y) << polyArc(x, y, oR, -45, 45);
  act = new RadioUiAction(KEY_PLUS, QList<int>() << Qt::Key_Up << Qt::Key_PageUp, SIMU_STR_HLP_KEYS_GO_UP % (hasRotEnc ? QString("") :  "|" % SIMU_STR_HLP_MOUSE_UP), SIMU_STR_HLP_ACT_UP);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "9X/9xcursup.png", act));

  polygon.clear();
  polygon << QPoint(x, y) << polyArc(x, y, oR, 135, 225);
  act = new RadioUiAction(KEY_MINUS, QList<int>() << Qt::Key_Down << Qt::Key_PageDown, SIMU_STR_HLP_KEYS_GO_DN % (hasRotEnc ? QString("") :  "|" % SIMU_STR_HLP_MOUSE_DN), SIMU_STR_HLP_ACT_DN);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "9X/9xcursdown.png", act));

  polygon.clear();
  polygon << QPoint(x, y) << polyArc(x, y, oR, 45, 135);
  act = new RadioUiAction(KEY_MINUS, QList<int>() << Qt::Key_Right << Qt::Key_Minus, SIMU_STR_HLP_KEY_RGT % "|" % SIMU_STR_HLP_KEY_MIN, SIMU_STR_HLP_ACT_MIN);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "9X/9xcursmin.png", act));

  polygon.clear();
  polygon << QPoint(x, y) << polyArc(x, y, oR, 225, 315);
  act = new RadioUiAction(KEY_PLUS, QList<int>() << Qt::Key_Left << Qt::Key_Plus << Qt::Key_Equal, SIMU_STR_HLP_KEY_LFT % "|" % SIMU_STR_HLP_KEY_PLS, SIMU_STR_HLP_ACT_PLS);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "9X/9xcursplus.png", act));

  act = new RadioUiAction(KEY_MENU, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEY_ENTER, SIMU_STR_HLP_ACT_MENU);
  addRadioWidget(ui->rightbuttons->addArea(QRect(16, 54, 60, 34), "9X/9xmenumenu.png", act));

  if (!hasRotEnc) {
    m_mouseMidClickAction = act;
    m_mouseMidClickAction->setText(act->getText() % "|" % SIMU_STR_HLP_MOUSE_MID);
  }

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->rightbuttons->addArea(QRect(16, 114, 60, 34), "9X/9xmenuexit.png", act));

  addRadioWidget(ui->leftbuttons->addArea(QRect(6, 149, 30, 30), "9X/9xcursphoto.png", m_screenshotAction));

  if (hasRotEnc) {
    addRadioAction(new RadioUiAction(-1, 0, SIMU_STR_HLP_MOUSE_SCRL, SIMU_STR_HLP_ROTENC % "|" % SIMU_STR_HLP_ROTENC_LR));
    m_mouseMidClickAction = new RadioUiAction(KEY_SYS, Qt::Key_Insert, SIMU_STR_HLP_KEY_INS % "|" % SIMU_STR_HLP_MOUSE_MID,  SIMU_STR_HLP_ACT_ROT_DN);
    addRadioWidget(ui->leftbuttons->addArea(QRect(0, 0, 0, 0), "9X/9xcurs.png", m_mouseMidClickAction));
  }

  m_backlightColors << QColor(159,165,247);
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

  setLcd(ui->lcd);
}

SimulatedUIWidget9X::~SimulatedUIWidget9X()
{
  delete ui;
}

void SimulatedUIWidget9X::setLightOn(bool enable)
{
  static QStringList list = QStringList() << "bl" << "gr" << "rd" << "or" << "yl";
  static QString bgfmt = "background:url(:/images/simulator/9X/9xd%1%2.png);";
  QString bg = "";
  if (enable && (int)m_backLight < list.size()) {
    bg = "-" + list[m_backLight];
  }
  ui->top->setStyleSheet(bgfmt.arg("t").arg(bg));
  ui->bottom->setStyleSheet(bgfmt.arg("b").arg(bg));
  ui->left->setStyleSheet(bgfmt.arg("l").arg(bg));
  ui->right->setStyleSheet(bgfmt.arg("r").arg(bg));
}

