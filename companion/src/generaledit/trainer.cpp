/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "trainer.h"
#include "compounditemmodels.h"
#include "autocombobox.h"
#include "autolineedit.h"
#include "autospinbox.h"
#include "autodoublespinbox.h"

#include <QFrame>
#include <QLabel>
#include <QStringList>

TrainerPanel::TrainerPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware, CompoundItemModelFactory * editorItemModels):
  GeneralPanel(parent, generalSettings, firmware)
{
  int modeid = editorItemModels->registerItemModel(TrainerMix::modeItemModel());
  int srcid = editorItemModels->registerItemModel(TrainerMix::srcItemModel());

  Board::Type board = getCurrentBoard();
  const int stickcnt = Boards::getCapability(board, Board::Sticks);

  const FieldRange weightrng = TrainerMix::getWeightRange();

  grid = new QGridLayout(this);

  int col = 0;
  int row = 0;

  if (stickcnt) {
    addLabel(tr("Mode"), row, 1);
    addLabel(tr("Weight"), row, 2);
    addLabel(tr("Source"), row++, 3);

    for (int i = 0; i < 4; i++, row++) {  //  TODO constant
      col = 0;
      addLabel(Boards::getAnalogInputName(board, i), row, col++);

      AutoComboBox *mode = new AutoComboBox(this);
      mode->setModel(editorItemModels->getItemModel(modeid));
      mode->setField(generalSettings.trainer.mix[i].mode, this);
      grid->addWidget(mode, row, col++);

      AutoSpinBox *weight = new AutoSpinBox(this);
      weight->setRange(weightrng.min, weightrng.max);
      weight->setField(generalSettings.trainer.mix[i].weight, this);
      grid->addWidget(weight, row, col++);

      AutoComboBox *src = new AutoComboBox(this);
      src->setModel(editorItemModels->getItemModel(srcid));
      src->setField(generalSettings.trainer.mix[i].src, this);
      grid->addWidget(src, row, col++);
    }

    addLine(row++, 0, grid->columnCount());
  }

  col = 0;
  addLabel(tr("Multiplier"), row, col++);

  const FieldRange ppmmrng = GeneralSettings::getPPM_MultiplierRange();

  AutoDoubleSpinBox *multi = new AutoDoubleSpinBox(this);
  multi->setDecimals(ppmmrng.decimals);
  multi->setOffset(ppmmrng.offset);
  multi->setSingleStep(ppmmrng.step);
  multi->setRange(ppmmrng.min, ppmmrng.max);
  multi->setField(generalSettings.PPM_Multiplier, this);
  grid->addWidget(multi, row++, col);

  col = 0;

  addLabel(tr("Calibration"), row, col++);

  for (int i = 0; i < stickcnt; i++, col++) {
    AutoLineEdit *calib = new AutoLineEdit(this);
    calib->setText(QString("%1").arg(generalSettings.trainer.calib[i]));
    calib->setReadOnly(true);
    grid->addWidget(calib, row, col);
  }

  addHSpring(grid, grid->columnCount(), 0);
  addVSpring(grid, 0, grid->rowCount());
  disableMouseScrolling();
}

TrainerPanel::~TrainerPanel()
{
}

void TrainerPanel::addLabel(QString text, int row, int col)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  grid->addWidget(label, row, col);
}

void TrainerPanel::addLine(int row, int col, int colspan)
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row, col, 1, colspan);
}
