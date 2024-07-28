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
#include "ui_simulateduiwidgetX9E.h"

SimulatedUIWidgetX9E::SimulatedUIWidgetX9E(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetX9E)
{
  RadioUiAction * act;
  QPolygon polygon;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  QPoint ctr(70, 100);

  polygon << polyArc(ctr.x(), ctr.y(), 60, -90, 0) << ctr;
  act = new RadioUiAction(KEY_MENU, QList<int>() << Qt::Key_Up << Qt::Key_PageUp, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_MENU);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "X9E/left_menu.png", act));

  polygon.clear();
  polygon << polyArc(ctr.x(), ctr.y(), 45, 0, 180);
  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_Down << Qt::Key_PageDown, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_PAGE);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "X9E/left_page.png", act));

  polygon.clear();
  polygon << polyArc(ctr.x(), ctr.y(), 60, 180, 270) << ctr;
  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "X9E/left_exit.png", act));

  m_scrollUpAction = addRadioAction(new RadioUiAction(-1, QList<int>() << Qt::Key_Minus << Qt::Key_Left, SIMU_STR_HLP_KEYS_GO_LFT, SIMU_STR_HLP_ACT_ROT_LFT));
  m_scrollDnAction = addRadioAction(new RadioUiAction(-1, QList<int>() << Qt::Key_Plus << Qt::Key_Equal << Qt::Key_Right, SIMU_STR_HLP_KEYS_GO_RGT, SIMU_STR_HLP_ACT_ROT_RGT));
  connectScrollActions();

  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(polyArc(90, 100, 50), "X9E/right_enter.png", m_mouseMidClickAction));

  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 170, 30, 30), "X9E/left_scrnshot.png", m_screenshotAction));

  m_backlightColors << QColor(47, 123, 227);  // Taranis Blue
  m_backlightColors << QColor(166,247,159);
  m_backlightColors << QColor(247,159,166);
  m_backlightColors << QColor(255,195,151);
  m_backlightColors << QColor(247,242,159);

  setLcd(ui->lcd);

  QString css = "#radioUiWidget {"
                  "background-color: qlineargradient(spread:pad, x1:1, y1:0.9, x2:0, y2:0,"
                    "stop:0 rgba(255, 255, 255, 255),"
                    "stop:0.35 rgba(250, 250, 250, 255), stop:0.5 rgba(242, 241, 241, 255),"
                    "stop:0.61 rgba(241, 241, 241, 255), stop:1.0 rgba(251, 251, 251, 255));"
                "}";

  QTimer * tim = new QTimer(this);
  tim->setSingleShot(true);
  connect(tim, &QTimer::timeout, [this, css]() {
    emit customStyleRequest(css);
  });
  tim->start(100);
}

SimulatedUIWidgetX9E::~SimulatedUIWidgetX9E()
{
  delete ui;
}

