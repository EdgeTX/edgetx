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
#include "ui_simulateduiwidgetGeneric.h"
#include "eeprominterface.h"

/*
    Note: This class is not expected to be instantiated directly but as the base for each firmware that uses it
          due to the methods and functions surrounding the building and loading of radio simulators
*/

SimulatedUIWidgetGeneric::SimulatedUIWidgetGeneric(SimulatorInterface *simulator, QWidget * parent):
  SimulatedUIWidget(simulator, parent),
  ui(new Ui::SimulatedUIWidgetGeneric)
{
  ui->setupUi(this);
  setLcd(ui->lcd);

  Board::Type board = getCurrentBoard();
  auto lcdDepth = Boards::getCapability(board, Board::LcdDepth);
  auto lcdWidth = Boards::getCapability(board, Board::LcdWidth) * (lcdDepth < 12 ? 2 : 1);
  auto lcdHeight = Boards::getCapability(board, Board::LcdHeight) * (lcdDepth < 12 ? 2 : 1);

  ui->lcd->setMaximumSize(lcdWidth, lcdHeight);
  ui->lcd->setMinimumSize(lcdWidth, lcdHeight);
  ui->lcd->setFixedSize(lcdWidth, lcdHeight);

  addGenericPushButtons(ui->leftbuttons, ui->rightbuttons);
  addScrollActions();

  int widgetHeight = lcdHeight;

  if (ui->leftbuttons->height() > widgetHeight)
    widgetHeight = ui->leftbuttons->height();

  if (ui->rightbuttons->height() > widgetHeight)
    widgetHeight = ui->rightbuttons->height();

  int widgetWidth = lcdWidth + ui->leftbuttons->width() + ui->rightbuttons->width();

  setMaximumSize(widgetWidth, 50 + widgetHeight);
  setMinimumSize(maximumSize());

  QTimer * t1 = new QTimer(this);
  t1->setSingleShot(true);
  connect(t1, &QTimer::timeout, [this]() {
      emit resizeRequest();
  });
  t1->start(100);

  m_backlightColors << QColor(47, 123, 227);

  QString css = "#radioUiWidget { background-color: rgb(0, 0, 0); }";

  QTimer * tim = new QTimer(this);
  tim->setSingleShot(true);
  connect(tim, &QTimer::timeout, [this, css]() {
      emit customStyleRequest(css);
  });
  tim->start(100);
}

SimulatedUIWidgetGeneric::~SimulatedUIWidgetGeneric()
{
  delete ui;
}

void SimulatedUIWidgetGeneric::shrink()
{
  adjustSize();
  //resize(0, 0);

  ui->lcd->adjustSize();
  ui->lcd->resize(0, 0);
}
