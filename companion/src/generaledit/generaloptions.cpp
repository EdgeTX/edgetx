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

#include "generaloptions.h"
#include "autocheckbox.h"

#include <QLabel>
#include <QGridLayout>

GeneralOptionsPanel::GeneralOptionsPanel(QWidget * parent, GeneralSettings & generalSettings, Firmware * firmware):
  GeneralPanel(parent, generalSettings, firmware),
  board(firmware->getBoard()),
  params(new QList<QWidget *>),
  row(0),
  col(0)
{
  grid = new QGridLayout(this);

  addSection(tr("Radio Menus"));

  if (Boards::getCapability(board, Board::HasColorLcd)) {
    addLabel(tr("Themes"));
    AutoCheckBox *themesDisable = new AutoCheckBox(this);
    themesDisable->setField(generalSettings.radioThemesDisabled, this, true);
    params->append(themesDisable);
    addParams();
  }

  addLabel(tr("Global Functions"));
  AutoCheckBox *globalFuncsDisable = new AutoCheckBox(this);
  globalFuncsDisable->setField(generalSettings.radioGFDisabled, this, true);
  params->append(globalFuncsDisable);
  addParams();

  addLabel(tr("Trainer"));
  AutoCheckBox *trainerDisable = new AutoCheckBox(this);
  trainerDisable->setField(generalSettings.radioTrainerDisabled, this, true);
  params->append(trainerDisable);
  addParams();

  row = 0;
  col = 2;
  addSection(tr("Model Menus"));

  if (firmware->getCapability(Heli)) {
    addLabel(tr("Heli"));
    AutoCheckBox *heliDisable = new AutoCheckBox(this);
    heliDisable->setField(generalSettings.modelHeliDisabled, this, true);
    params->append(heliDisable);
    addParams();
  }

  addLabel(tr("%1 Modes").arg(Boards::getRadioTypeString(board)));
  AutoCheckBox *fmDisable = new AutoCheckBox(this);
  fmDisable->setField(generalSettings.modelFMDisabled, this, true);
  params->append(fmDisable);
  addParams();

  addLabel(tr("Curves"));
  AutoCheckBox *curvesDisable = new AutoCheckBox(this);
  curvesDisable->setField(generalSettings.modelCurvesDisabled, this, true);
  params->append(curvesDisable);
  addParams();

  if (firmware->getCapability(Gvars)) {
    addLabel(tr("Global Variables"));
    AutoCheckBox *gvDisable = new AutoCheckBox(this);
    gvDisable->setField(generalSettings.modelGVDisabled, this, true);
    params->append(gvDisable);
    addParams();
  }

  addLabel(tr("Logical Switches"));
  AutoCheckBox *lsDisable = new AutoCheckBox(this);
  lsDisable->setField(generalSettings.modelLSDisabled, this, true);
  params->append(lsDisable);
  addParams();

  addLabel(tr("Special Functions"));
  AutoCheckBox *sfDisable = new AutoCheckBox(this);
  sfDisable->setField(generalSettings.modelSFDisabled, this, true);
  params->append(sfDisable);
  addParams();

  addLabel(tr("Custom Mix Scripts"));
  AutoCheckBox *cmsDisable = new AutoCheckBox(this);
  cmsDisable->setField(generalSettings.modelCustomScriptsDisabled, this, true);
  params->append(cmsDisable);
  addParams();

  addLabel(tr("Telemetry"));
  AutoCheckBox *teleDisable = new AutoCheckBox(this);
  teleDisable->setField(generalSettings.modelTelemetryDisabled, this, true);
  params->append(teleDisable);
  addParams();

  addVSpring(grid, 0, grid->rowCount());
  addHSpring(grid, grid->columnCount(), 0);
  disableMouseScrolling();
}

GeneralOptionsPanel::~GeneralOptionsPanel()
{

}

void GeneralOptionsPanel::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  grid->addWidget(label, row, col);
}

void GeneralOptionsPanel::addLine()
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row++, 0, 1, grid->columnCount());
}

void GeneralOptionsPanel::addParams()
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

void GeneralOptionsPanel::addSection(QString text)
{
  addLabel(QString("<b>%1</b>").arg(text));
  row++;
}
