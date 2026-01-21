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

#include "simulateduiwidgetGeneric.h"
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
  Board::Type board = getCurrentBoard();

  if (Boards::getCapability(board, Board::HasBacklightColor)) {
    m_backlightColors << QColor(215, 243, 255);   //  blue
    m_backlightColors << QColor(166, 247, 159);   //  green
    m_backlightColors << QColor(247, 159, 166);   //  red
    m_backlightColors << QColor(255, 195, 151);   //  orange
    m_backlightColors << QColor(247, 242, 159);   //  yellow
  }
  else
    m_backlightColors << QColor(198, 208, 199);

  if (Boards::getCapability(board, Board::LcdOLED)) {
    ui->lcd->setBgDefaultColor(QColor(0, 0, 0));        //  black
    ui->lcd->setFgDefaultColor(QColor(255, 255, 255));  //  white
  }

  setLcd(ui->lcd);

  //  TODO: move to setLcd when all radios using generic
  auto lcdDepth = Boards::getCapability(board, Board::LcdDepth);
  auto lcdWidth = Boards::getCapability(board, Board::LcdWidth) * (lcdDepth < 12 ? 2 : 1);
  auto lcdHeight = Boards::getCapability(board, Board::LcdHeight) * (lcdDepth < 12 ? 2 : 1);

  ui->lcd->setFixedSize(lcdWidth, lcdHeight);
  //  end TODO

  addPushButtons(ui->leftbuttons, ui->rightbuttons);
  addScrollActions();

  int widgetHeight = lcdHeight;

  if (ui->leftbuttons->height() > widgetHeight)
    widgetHeight = ui->leftbuttons->height();

  if (ui->rightbuttons->height() > widgetHeight)
    widgetHeight = ui->rightbuttons->height();

  int widgetWidth = lcdWidth + ui->leftbuttons->width() + ui->rightbuttons->width();

  setFixedSize(50 + widgetWidth, 50 + widgetHeight);

  //  workaround to delay resize until after parent has been resized
  QTimer * t1 = new QTimer(this);
  t1->setSingleShot(true);
  connect(t1, &QTimer::timeout, [this]() {
      emit resizeRequest();
  });
  t1->start(100);

  //  workaround to delay changing until after parent has been initialised
  QString css = "#radioUiWidget { background-color: " + QVariant(g.currentProfile().radioSimCaseColor()).toString() + "; }";

  QTimer * t2 = new QTimer(this);
  t2->setSingleShot(true);
  connect(t2, &QTimer::timeout, [this, css]() {
      emit customStyleRequest(css);
  });
  t2->start(100);
}

SimulatedUIWidgetGeneric::~SimulatedUIWidgetGeneric()
{
  delete ui;
}

void SimulatedUIWidgetGeneric::shrink()
{
  //  adjust parent before child to ensure parent does not constrain child
  adjustSize();
  ui->lcd->adjustSize();
}
