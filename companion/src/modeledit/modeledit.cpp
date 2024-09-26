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

#include "modeledit.h"
#include "helpers.h"
#include "ui_modeledit.h"
#include "setup.h"
#include "heli.h"
#include "flightmodes.h"
#include "inputs.h"
#include "mixes.h"
#include "channels.h"
#include "curves.h"
#include "verticalscrollarea.h"
#include "logicalswitches.h"
#include "customfunctions.h"
#include "telemetry.h"
#include "colorcustomscreens.h"
#include "telemetry_customscreens.h"
#include "modeloptions.h"
#include "appdata.h"
#include "compounditemmodels.h"

ModelEdit::ModelEdit(QWidget * parent, RadioData & radioData, int modelId, Firmware * firmware) :
  QDialog(parent),
  ui(new Ui::ModelEdit),
  modelId(modelId),
  radioData(radioData),
  firmware(firmware)
{
  Stopwatch s1("ModelEdit");
  gStopwatch.report("ModelEdit start constructor");

  ui->setupUi(this);
  setWindowIcon(CompanionIcon("edit.png"));
  setAttribute(Qt::WA_DeleteOnClose);
  restoreGeometry(g.modelEditGeo());
  ui->pushButton->setIcon(CompanionIcon("simulate.png"));

  GeneralSettings &generalSettings = radioData.generalSettings;
  ModelData &model = radioData.models[modelId];

  sharedItemModels = new CompoundItemModelFactory(&generalSettings, &model);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_RawSource);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_RawSwitch);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_Curve);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_GVarRef);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_ThrSource);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_CustomFuncAction);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_CustomFuncResetParam);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_TeleSource);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_CurveRefType);
  sharedItemModels->addItemModel(AbstractItemModel::IMID_CurveRefFunc);

  s1.report("Init");

  SetupPanel *setupPanel = new SetupPanel(this, model, generalSettings, firmware, sharedItemModels);
  addTab(setupPanel, tr("Setup"));
  s1.report("Setup");

  if (firmware->getCapability(Heli)) {
    addTab(new HeliPanel(this, model, generalSettings, firmware, sharedItemModels), tr("Heli"));
    s1.report("Heli");
  }

  QString radioType = QString("%1 Modes").arg(Boards::getRadioTypeString(firmware->getBoard()));
  FlightModesPanel *flightModesPanel = new FlightModesPanel(this, model, generalSettings, firmware, sharedItemModels);
  addTab(flightModesPanel, radioType);
  s1.report(radioType);

  addTab(new InputsPanel(this, model, generalSettings, firmware, sharedItemModels), tr("Inputs"));
  s1.report("Inputs");

  addTab(new MixesPanel(this, model, generalSettings, firmware, sharedItemModels), tr("Mixes"));
  s1.report("Mixes");

  ChannelsPanel *channelsPanel = new ChannelsPanel(this, model, generalSettings, firmware, sharedItemModels);
  addTab(channelsPanel, tr("Outputs"));
  s1.report("Outputs");

  addTab(new CurvesPanel(this, model, generalSettings, firmware, sharedItemModels), tr("Curves"));
  s1.report("Curves");

  addTab(new LogicalSwitchesPanel(this, model, generalSettings, firmware, sharedItemModels), tr("Logical Switches"));
  s1.report("Logical Switches");

  addTab(new CustomFunctionsPanel(this, &model, generalSettings, firmware, sharedItemModels), tr("Special Functions"));
  s1.report("Special Functions");

  if (firmware->getCapability(Telemetry)) {
    addTab(new TelemetryPanel(this, model, generalSettings, firmware, sharedItemModels), tr("Telemetry"));
    s1.report("Telemetry");
  }

  if (Boards::getCapability(firmware->getBoard(), Board::HasColorLcd)) {
    addTab(new ColorCustomScreensPanel(this, model, generalSettings, firmware, sharedItemModels), tr("Custom Screens"));
    s1.report("ColorLcd Custom Screens");
  }
  else if (firmware->getCapability(TelemetryCustomScreens)) {
    addTab(new TelemetryCustomScreensPanel(this, model, generalSettings, firmware, sharedItemModels), tr("Custom Screens"));
    s1.report("Telemetry Custom Screens");
  }

  addTab(new ModelOptionsPanel(this, model, generalSettings, firmware), tr("Enabled Features"));
  s1.report("Enabled Features");

  connect(setupPanel, &SetupPanel::extendedLimitsToggled, channelsPanel, &ChannelsPanel::refreshExtendedLimits);
  connect(setupPanel, &SetupPanel::throttleReverseChanged, flightModesPanel, &FlightModesPanel::onThrottleReverseChanged);
  connect(ui->tabWidget, &QTabWidget::currentChanged, this, &ModelEdit::onTabIndexChanged);
  connect(ui->pushButton, &QPushButton::clicked, this, &ModelEdit::launchSimulation);

  onTabIndexChanged(ui->tabWidget->currentIndex());  // make sure to trigger update on default tab panel

  gStopwatch.report("ModelEdit end constructor");
}

ModelEdit::~ModelEdit()
{
  delete ui;
  delete sharedItemModels;
}

void ModelEdit::closeEvent(QCloseEvent *event)
{
  g.modelEditGeo( saveGeometry() );
}

void ModelEdit::addTab(GenericPanel *panel, QString text)
{
  panels << panel;
  QWidget * widget = new QWidget(ui->tabWidget);
  QVBoxLayout *baseLayout = new QVBoxLayout(widget);
  VerticalScrollArea * area = new VerticalScrollArea(widget, panel);
  baseLayout->addWidget(area);
  ui->tabWidget->addTab(widget, text);
  connect(panel, &GenericPanel::modified, this, &ModelEdit::modified);
}

void ModelEdit::onTabIndexChanged(int index)
{
  if (index < panels.size())
    panels.at(index)->update();
}

void ModelEdit::launchSimulation()
{
  startSimulation(this, radioData, modelId);
}
