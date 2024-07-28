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
#include "ui_simulateduiwidgetX10.h"

SimulatedUIWidgetX10::SimulatedUIWidgetX10(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetX10)
{
  RadioUiAction * act;
  QPolygon polygon;

  ui->setupUi(this);

  // add actions in order of appearance on the help menu

  int x = 74, y = 148, oR = 63, iR = 40;

  polygon.clear();
  polygon << polyArc(x, y, oR, -45, 45) << polyArc(x, y, iR, -45, 45);
  act = new RadioUiAction(KEY_MODEL, QList<int>() << Qt::Key_Up, SIMU_STR_HLP_KEY_UP, SIMU_STR_HLP_ACT_MDL);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "X10/left_mdl.png", act));

  polygon.clear();
  polygon << polyArc(x, y, oR, 225, 315) << polyArc(x, y, iR, 225, 315);
  act = new RadioUiAction(KEY_SYS, QList<int>() << Qt::Key_Left, SIMU_STR_HLP_KEY_LFT, SIMU_STR_HLP_ACT_SYS);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "X10/left_sys.png", act));

  polygon.clear();
  polygon << polyArc(x, y, oR, 45, 135) << polyArc(x, y, iR, 45, 135);
  act = new RadioUiAction(KEY_TELE, QList<int>() << Qt::Key_Right, SIMU_STR_HLP_KEY_RGT, SIMU_STR_HLP_ACT_TELE);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "X10/left_tele.png", act));

  polygon.clear();
  polygon << polyArc(x, y, oR, 135, 225) << polyArc(x, y, iR, 135, 225);
  act = new RadioUiAction(KEY_EXIT, QList<int>() << Qt::Key_Down << Qt::Key_Delete << Qt::Key_Escape << Qt::Key_Backspace,
                         SIMU_STR_HLP_KEY_DN % "<br>" % SIMU_STR_HLP_KEYS_EXIT, SIMU_STR_HLP_ACT_RTN);
  addRadioWidget(ui->leftbuttons->addArea(polygon, "X10/left_rtn.png", act));

  act = new RadioUiAction(KEY_PAGEDN, QList<int>() << Qt::Key_PageDown, SIMU_STR_HLP_KEY_PGDN, SIMU_STR_HLP_ACT_PGDN);
  addRadioWidget(ui->leftbuttons->addArea(polyArc(x, y, iR), "X10/left_page.png", act));

  m_scrollUpAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Minus, SIMU_STR_HLP_KEY_MIN % "|" % SIMU_STR_HLP_MOUSE_UP, SIMU_STR_HLP_ACT_ROT_LFT);
  m_scrollDnAction = new RadioUiAction(-1, QList<int>() << Qt::Key_Plus << Qt::Key_Equal, SIMU_STR_HLP_KEY_PLS % "|" % SIMU_STR_HLP_MOUSE_DN, SIMU_STR_HLP_ACT_ROT_RGT);
  connectScrollActions();

  m_mouseMidClickAction = new RadioUiAction(KEY_ENTER, QList<int>() << Qt::Key_Enter << Qt::Key_Return, SIMU_STR_HLP_KEYS_ACTIVATE, SIMU_STR_HLP_ACT_ROT_DN);
  addRadioWidget(ui->rightbuttons->addArea(polyArc(98, y, iR), "X10/right_ent.png", m_mouseMidClickAction));

  addRadioWidget(ui->leftbuttons->addArea(QRect(10, 252, 30, 30), "X10/left_scrnsht.png", m_screenshotAction));

  m_backlightColors << QColor(47, 123, 227);

  setLcd(ui->lcd);
}

SimulatedUIWidgetX10::~SimulatedUIWidgetX10()
{
  delete ui;
}
