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
#include "ui_simulateduiwidgetCommando8.h"

// NOTE: RadioUiAction(NUMBER,...): NUMBER relates to enum EnumKeys in the specific board.h

SimulatedUIWidgetCommando8::SimulatedUIWidgetCommando8(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetCommando8)
{
  RadioUiAction * act;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  act = new RadioUiAction(KEY_MINUS, QList<int>() << Qt::Key_Up, SIMU_STR_HLP_KEY_UP, SIMU_STR_HLP_ACT_MDL);
  addRadioWidget(ui->bottom->addArea(QRect(50, 44, 18, 18), "Commando8/left-mdl.png", act));

  act = new RadioUiAction(KEY_PLUS, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_GO_UP, SIMU_STR_HLP_ACT_UP);
  addRadioWidget(ui->bottom->addArea(QRect(248, 25, 18, 18), "Commando8/right-up.png", act));

  act = new RadioUiAction(KEY_MINUS, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_GO_DN, SIMU_STR_HLP_ACT_DN);
   addRadioWidget(ui->bottom->addArea(QRect(248, 62, 18, 18), "Commando8/right-dn.png", act));

  act = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ENT);
  addRadioWidget(ui->bottom->addArea(QRect(250, 45, 15, 15), "Commando8/right-ent.png", act));

  act = new RadioUiAction(KEY_SYS, QList<int>() << Qt::Key_Left, SIMU_STR_HLP_KEY_LFT, SIMU_STR_HLP_ACT_SYS);
  addRadioWidget(ui->bottom->addArea(QRect(34, 46, 15, 15), "Commando8/left-sys.png", act));

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Down << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace, SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_EXIT);
  addRadioWidget(ui->bottom->addArea(QRect(14, 44, 18, 18), "Commando8/left-rtn.png", act));

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_PageDown, SIMU_STR_HLP_KEY_PGDN, SIMU_STR_HLP_ACT_PGDN);
  addRadioWidget(ui->bottom->addArea(QRect(267, 43, 18, 18), "Commando8/right-pagedn.png", act));

  act = new RadioUiAction(KEY_PAGEUP, QList<int>() << Qt::Key_PageUp, SIMU_STR_HLP_KEY_PGUP, SIMU_STR_HLP_ACT_PGUP);
  addRadioWidget(ui->bottom->addArea(QRect(230, 43, 18, 18), "Commando8/right-pageup.png", act));


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

SimulatedUIWidgetCommando8::~SimulatedUIWidgetCommando8()
{
  delete ui;
}
