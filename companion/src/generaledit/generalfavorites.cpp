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

#include "generalfavorites.h"
#include "autocombobox.h"
#include "compounditemmodels.h"

#include <QLabel>
#include <QGridLayout>

GeneralFavsPanel::GeneralFavsPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware):
  GeneralPanel(parent, generalSettings, firmware),
  board(firmware->getBoard()),
  params(new QList<QWidget *>),
  row(0),
  col(0)
{
  AbstractItemModel *mdl = generalSettings.quickMenuItemModel(false);
  const int cnt = firmware->getCapability(QMFavourites);
  grid = new QGridLayout(this);

  for (int i = 0; i < cnt; i++) {
    addLabel(tr("Favorite %1").arg(i + 1));
    AutoComboBox *cbo = new AutoComboBox(this);
    cbo->setModel(mdl);
    cbo->setField(generalSettings.qmFavorites[i], this);
    params->append(cbo);
    addParams();
  }

  addLine();

  QPushButton *reset = new QPushButton(tr("Reset"));
  connect(reset, &QPushButton::clicked, [&] ()
  {
    generalSettings.setDefaultFavorites();

    foreach(AutoComboBox *cb, findChildren<AutoComboBox*>())
      cb->updateValue();
  });
  params->append(reset);
  addParams();

  addVSpring(grid, 0, grid->rowCount());
  addHSpring(grid, grid->columnCount(), 0);
  disableMouseScrolling();
}

GeneralFavsPanel::~GeneralFavsPanel()
{

}

void GeneralFavsPanel::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  grid->addWidget(label, row, col);
}

void GeneralFavsPanel::addLine()
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row++, 0, 1, grid->columnCount());
}

void GeneralFavsPanel::addParams()
{
  int col = 0;
  QGridLayout *subgrid = new QGridLayout();

  for (int i = 0; i < params->size(); i++) {
    subgrid->addWidget(params->at(i), 0, col++);
  }

  addHSpring(subgrid, col, 0);
  grid->addLayout(subgrid, row++, this->col + 1);
  params->clear();
}

void GeneralFavsPanel::addSection(QString text)
{
  addLabel(QString("<b>%1</b>").arg(text));
  row++;
}
