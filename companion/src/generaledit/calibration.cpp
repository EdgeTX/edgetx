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

#include "calibration.h"
#include "autospinbox.h"
#include "helpers.h"

CalibrationPanel::CalibrationPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware):
  GeneralPanel(parent, generalSettings, firmware)
{
  int maxrows = Boards::getInputsCalibrated();
  int maxinputs = Boards::getCapability(getCurrentBoard(), Board::Inputs);

  QStringList headerLabels;
  headerLabels << "" << tr("Negative span") << tr("Mid value") << tr("Positive span");

  TableLayout * tableLayout = new TableLayout(this, maxrows, headerLabels);

  for (int i = 0; i < maxinputs; i++) {
    if (!Boards::isInputCalibrated(i))
      continue;

    GeneralSettings::InputCalib &calib = generalSettings.inputConfig[i].calib;
    int col = 0;
    QLabel * label = new QLabel(this);
    label->setText(Boards::getInputName(i));
    tableLayout->addWidget(i, col++, label);

    QLineEdit * leNeg = new QLineEdit(this);
    leNeg->setText(QString("%1").arg(calib.spanNeg));
    leNeg->setReadOnly(true);
    tableLayout->addWidget(i, col++, leNeg);

    QLineEdit * leMid = new QLineEdit(this);
    leMid->setText(QString("%1").arg(calib.mid));
    leMid->setReadOnly(true);
    tableLayout->addWidget(i, col++, leMid);

    QLineEdit * lePos = new QLineEdit(this);
    lePos->setText(QString("%1").arg(calib.spanPos));
    lePos->setReadOnly(true);
    tableLayout->addWidget(i, col++, lePos);
  }

  disableMouseScrolling();
  tableLayout->resizeColumnsToContents();
  tableLayout->setColumnWidth(0, QString(15, ' '));
  tableLayout->pushColumnsLeft(headerLabels.count());
  tableLayout->pushRowsUp(maxrows + 1);
}
