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

#include "prefs_panel.h"

PrefsPanel::PrefsPanel(QWidget * parent, Firmware * firmware, Board::Type & board, Profile & profile):
  AbstractPanel(parent),
  firmware(firmware),
  board(board),
  profile(profile),
  row(0),
  col(0)
{
}

PrefsPanel::~PrefsPanel()
{
}

AutoLineEdit * PrefsPanel::newPathWidget(QWidget * parent)
{
  AutoLineEditPath *wgt = new AutoLineEditPath(parent, true);
  setPathWidget(wgt);
  return wgt;
}

void PrefsPanel::setPathWidget(AutoLineEdit * wgt)
{
  wgt->setMinimumWidth(PATH_MIN_WIDTH);
  wgt->setSizePolicy(PATH_SIZE_POLICY);
  wgt->setEditSignal(true);
  wgt->setClearButtonEnabled(true);
}

void PrefsPanel::onRadioChanged(Firmware * firmware)
{
  this->firmware = firmware;
  board = this->firmware->getBoard();
  update();
}

/*
    PrefsScrollArea
*/

PrefsScrollArea::PrefsScrollArea(QWidget * parent, PrefsPanel * panel):
  QScrollArea(parent),
  panel(panel)
{
  setWidgetResizable(true);
  setWidget(panel);
  panel->installEventFilter(this);
}

bool PrefsScrollArea::eventFilter(QObject * o, QEvent * e)
{
  if (o == panel && e->type() == QEvent::Resize)
    setMinimumWidth(panel->minimumSizeHint().width());

  return false;
}
