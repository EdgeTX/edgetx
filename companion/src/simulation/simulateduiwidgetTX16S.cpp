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
#include "ui_simulateduiwidgetTX16S.h"

SimulatedUIWidgetTX16S::SimulatedUIWidgetTX16S(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetTX16S)
{
  RadioUiAction * act;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  act = new RadioUiAction(KEY_MODEL, QList<int>() << Qt::Key_Up, SIMU_STR_HLP_KEY_UP, SIMU_STR_HLP_ACT_MDL);
  addRadioWidget(ui->rightbuttons->addArea(QRect(40, 0, 110, 35), "TX16S/right_mdl.png", act));

  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(QRect(45, 70, 100, 160), "TX16S/right_ent.png", m_mouseMidClickAction));

  act = new RadioUiAction(KEY_SYS, QList<int>() << Qt::Key_Left, SIMU_STR_HLP_KEY_LFT, SIMU_STR_HLP_ACT_SYS);
  addRadioWidget(ui->leftbuttons->addArea(QRect(32, 1, 80, 35), "TX16S/left_sys.png", act));

  act = new RadioUiAction(KEY_TELE, QList<int>() << Qt::Key_Right, SIMU_STR_HLP_KEY_RGT, SIMU_STR_HLP_ACT_TELE);
  addRadioWidget(ui->leftbuttons->addArea(QRect(55, 220, 80, 35), "TX16S/left_tele.png", act));

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_PageDown, SIMU_STR_HLP_KEY_PGDN, SIMU_STR_HLP_ACT_PGDN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(55, 115, 80, 35), "TX16S/left_page.png", act));

  act = new RadioUiAction(KEY_PAGEUP, QList<int>() << Qt::Key_PageUp, SIMU_STR_HLP_KEY_PGUP, SIMU_STR_HLP_ACT_PGUP);
  addRadioWidget(ui->leftbuttons->addArea(QRect(55, 160, 80, 35), "TX16S/left_page2.png", act));

  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Down << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace,
                          SIMU_STR_HLP_KEY_DN % "<br>" % SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_RTN);
  addRadioWidget(ui->leftbuttons->addArea(QRect(55, 55, 80, 35), "TX16S/left_rtn.png", act));

  m_scrollUpAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Minus, SIMU_STR_HLP_KEY_MIN % "|" % SIMU_STR_HLP_MOUSE_UP, SIMU_STR_HLP_ACT_ROT_LFT);
  m_scrollDnAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Plus << Qt::Key_Equal, SIMU_STR_HLP_KEY_PLS % "|" % SIMU_STR_HLP_MOUSE_DN, SIMU_STR_HLP_ACT_ROT_RGT);
  connectScrollActions();

  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 245, 30, 30), "TX16S/left_scrnsht.png", m_screenshotAction));

  m_backlightColors << QColor(47, 123, 227);

  setLcd(ui->lcd);
}

SimulatedUIWidgetTX16S::~SimulatedUIWidgetTX16S()
{
  delete ui;
}
