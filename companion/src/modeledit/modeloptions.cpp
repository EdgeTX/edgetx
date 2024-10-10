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

#include "modeloptions.h"
#include "autocombobox.h"
#include "compounditemmodels.h"

#include <QLabel>
#include <QGridLayout>

ModelOptionsPanel::ModelOptionsPanel(QWidget * parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware):
  ModelPanel(parent, model, generalSettings, firmware),
  board(firmware->getBoard()),
  params(new QList<QWidget *>),
  row(0),
  col(0)
{
  // grid within a grid to compact the inner grid layout more than a single grid
  QGridLayout* panelgrid = new QGridLayout(this);
  grid = new QGridLayout();
  panelgrid->addLayout(grid, 0, 0);

  AbstractStaticItemModel * mdl = new AbstractStaticItemModel();
  mdl->appendToItemList(tr("Global"), 0);
  mdl->appendToItemList(tr("Off"), 1);
  mdl->appendToItemList(tr("On"), 2);
  mdl->loadItemList();

  addSection(tr("Radio Menus"));

  if (Boards::getCapability(board, Board::HasColorLcd)) {
    addLabel(tr("Themes"));
    AutoComboBox *themesDisable = new AutoComboBox(this);
    themesDisable->setModel(mdl);
    themesDisable->setField(model.radioThemesDisabled, this);
    params->append(themesDisable);
    addParams();
  }

  addLabel(tr("Global Functions"));
  AutoComboBox *globalFuncsDisable = new AutoComboBox(this);
  globalFuncsDisable->setModel(mdl);
  globalFuncsDisable->setField(model.radioGFDisabled, this);
  params->append(globalFuncsDisable);
  addParams();

  addLabel(tr("Trainer"));
  AutoComboBox *trainerDisable = new AutoComboBox(this);
  trainerDisable->setModel(mdl);
  trainerDisable->setField(model.radioTrainerDisabled, this);
  params->append(trainerDisable);
  addParams();

  row = 0;
  col = 2;
  addSection(tr("Model Menus"));

  if (firmware->getCapability(Heli)) {
    addLabel(tr("Heli"));
    AutoComboBox *heliDisable = new AutoComboBox(this);
    heliDisable->setModel(mdl);
    heliDisable->setField(model.modelHeliDisabled, this);
    params->append(heliDisable);
    addParams();
  }

  addLabel(tr("%1 Modes").arg(Boards::getRadioTypeString(firmware->getBoard())));
  AutoComboBox *fmDisable = new AutoComboBox(this);
  fmDisable->setModel(mdl);
  fmDisable->setField(model.modelFMDisabled, this);
  params->append(fmDisable);
  addParams();

  addLabel(tr("Curves"));
  AutoComboBox *curvesDisable = new AutoComboBox(this);
  curvesDisable->setModel(mdl);
  curvesDisable->setField(model.modelCurvesDisabled, this);
  params->append(curvesDisable);
  addParams();

  if (firmware->getCapability(Gvars)) {
    addLabel(tr("Global Variables"));
    AutoComboBox *gvDisable = new AutoComboBox(this);
    gvDisable->setModel(mdl);
    gvDisable->setField(model.modelGVDisabled, this);
    params->append(gvDisable);
    addParams();
  }

  addLabel(tr("Logical Switches"));
  AutoComboBox *lsDisable = new AutoComboBox(this);
  lsDisable->setModel(mdl);
  lsDisable->setField(model.modelLSDisabled, this);
  params->append(lsDisable);
  addParams();

  addLabel(tr("Special Functions"));
  AutoComboBox *sfDisable = new AutoComboBox(this);
  sfDisable->setModel(mdl);
  sfDisable->setField(model.modelSFDisabled, this);
  params->append(sfDisable);
  addParams();

  addLabel(tr("Custom Mix Scripts"));
  AutoComboBox *cmsDisable = new AutoComboBox(this);
  cmsDisable->setModel(mdl);
  cmsDisable->setField(model.modelCustomScriptsDisabled, this);
  params->append(cmsDisable);
  addParams();

  addLabel(tr("Telemetry"));
  AutoComboBox *teleDisable = new AutoComboBox(this);
  teleDisable->setModel(mdl);
  teleDisable->setField(model.modelTelemetryDisabled, this);
  params->append(teleDisable);
  addParams();

  addVSpring(panelgrid, 0, panelgrid->rowCount());
  addHSpring(panelgrid, panelgrid->columnCount(), 0);
  disableMouseScrolling();
}

ModelOptionsPanel::~ModelOptionsPanel()
{

}

void ModelOptionsPanel::addLabel(QString text)
{
  QLabel *label = new QLabel(this);
  label->setText(text);
  grid->addWidget(label, row, col);
}

void ModelOptionsPanel::addLine()
{
  QFrame *line = new QFrame(this);
  line->setFrameShape(QFrame::HLine);
  line->setFrameShadow(QFrame::Sunken);
  line->setLineWidth(1);
  line->setMidLineWidth(0);
  grid->addWidget(line, row++, 0, 1, grid->columnCount());
}

void ModelOptionsPanel::addParams()
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

void ModelOptionsPanel::addSection(QString text)
{
  addLabel(QString("<b>%1</b>").arg(text));
  row++;
}
