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

#include "telemetry_customscreens.h"
#include "ui_telemetry_customscreen.h"
#include "helpers.h"
#include "appdata.h"

#include <TimerEdit>

constexpr char FIM_RAWSOURCE[]       {"Raw Source"};

CustomScreen::CustomScreen(QWidget *parent, ModelData & model, FrSkyScreenData & screen, GeneralSettings & generalSettings,
                                             Firmware * firmware, const bool & parentLock, FilteredItemModelFactory * panelFilteredItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::TelemetryCustomScreen),
  screen(screen),
  modelsUpdateCnt(0),
  parentLock(parentLock)
{
  ui->setupUi(this);

  FilteredItemModel * rawSourceFilteredModel = panelFilteredItemModels->getItemModel(FIM_RAWSOURCE);
  connectItemModelEvents(rawSourceFilteredModel);

  for (int l = 0; l < firmware->getCapability(TelemetryCustomScreensLines); l++) {
    for (int c = 0; c < firmware->getCapability(TelemetryCustomScreensFieldsPerLine); c++) {
      fieldsCB[l][c] = new QComboBox(this);
      fieldsCB[l][c]->setProperty("index", c + (l << 8));
      fieldsCB[l][c]->setModel(rawSourceFilteredModel);
      ui->screenNumsLayout->addWidget(fieldsCB[l][c], l, c, 1, 1);
      connect(fieldsCB[l][c], SIGNAL(activated(int)), this, SLOT(customFieldChanged(int)));
    }
  }

  for (int l = 0; l < firmware->getCapability(TelemetryCustomScreensBars); l++) {
    barsCB[l] = new QComboBox(this);
    barsCB[l]->setProperty("index", l);
    barsCB[l]->setModel(rawSourceFilteredModel);
    connect(barsCB[l], SIGNAL(activated(int)), this, SLOT(barSourceChanged(int)));
    ui->screenBarsLayout->addWidget(barsCB[l], l+1, 0, 1, 1);

    minSB[l] = new QDoubleSpinBox(this);
    minSB[l]->setProperty("index", l);
    connect(minSB[l], SIGNAL(valueChanged(double)), this, SLOT(barMinChanged(double)));
    ui->screenBarsLayout->addWidget(minSB[l], l+1, 1, 1, 1);

    minTime[l] = new TimerEdit(this);
    minTime[l]->setProperty("index", l);
    minTime[l]->setProperty("type", "min");
    connect(minTime[l], SIGNAL(editingFinished()), this, SLOT(barTimeChanged()));
    ui->screenBarsLayout->addWidget(minTime[l], l+1, 1, 1, 1);
    minTime[l]->hide();

    QLabel * label = new QLabel(this);
    label->setAutoFillBackground(false);
    label->setStyleSheet(QString::fromUtf8("Background:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(0, 0, 128, 255), stop:0.339795 rgba(0, 0, 128, 255), stop:0.339799 rgba(255, 255, 255, 255), stop:0.662444 rgba(255, 255, 255, 255),)\n"""));
    label->setFrameShape(QFrame::Panel);
    label->setFrameShadow(QFrame::Raised);
    label->setAlignment(Qt::AlignCenter);
    ui->screenBarsLayout->addWidget(label, l+1, 2, 1, 1);

    maxSB[l] = new QDoubleSpinBox(this);
    maxSB[l]->setProperty("index", l);
    connect(maxSB[l], SIGNAL(valueChanged(double)), this, SLOT(barMaxChanged(double)));
    ui->screenBarsLayout->addWidget(maxSB[l], l+1, 3, 1, 1);

    maxTime[l] = new TimerEdit(this);
    maxTime[l]->setProperty("index", l);
    maxTime[l]->setProperty("type", "max");
    connect(maxTime[l], SIGNAL(editingFinished()), this, SLOT(barTimeChanged()));
    ui->screenBarsLayout->addWidget(maxTime[l], l+1, 3, 1, 1);
    maxTime[l]->hide();
  }

  disableMouseScrolling();

  lock = true;
  ui->screenType->addItem(tr("None"), TELEMETRY_SCREEN_NONE);
  ui->screenType->addItem(tr("Numbers"), TELEMETRY_SCREEN_NUMBERS);
  ui->screenType->addItem(tr("Bars"), TELEMETRY_SCREEN_BARS);
  if (IS_TARANIS(firmware->getBoard()))
    ui->screenType->addItem(tr("Script"), TELEMETRY_SCREEN_SCRIPT);
  ui->screenType->setField(screen.type, this);
  lock = false;

  if (IS_TARANIS(firmware->getBoard())) {
    QSet<QString> scriptsSet = getFilesSet(g.profile[g.id()].sdPath() + "/SCRIPTS/TELEMETRY", QStringList() << "*.lua", 6);
    Helpers::populateFileComboBox(ui->scriptName, scriptsSet, screen.body.script.filename);
    connect(ui->scriptName, SIGNAL(currentIndexChanged(int)), this, SLOT(scriptNameEdited()));
    connect(ui->scriptName, SIGNAL(editTextChanged ( const QString)), this, SLOT(scriptNameEdited()));
  }

  update();
}

CustomScreen::~CustomScreen()
{
  delete ui;
}

void CustomScreen::update()
{
  lock = true;

  ui->scriptName->setVisible(screen.type == TELEMETRY_SCREEN_SCRIPT);
  ui->screenNums->setVisible(screen.type == TELEMETRY_SCREEN_NUMBERS);
  ui->screenBars->setVisible(screen.type == TELEMETRY_SCREEN_BARS);

  for (int l = 0; l < firmware->getCapability(TelemetryCustomScreensLines); l++) {
    for (int c = 0; c < firmware->getCapability(TelemetryCustomScreensFieldsPerLine); c++) {
      fieldsCB[l][c]->setCurrentIndex(fieldsCB[l][c]->findData(screen.body.lines[l].source[c].toValue()));
    }
  }

  for (int l = 0; l < firmware->getCapability(TelemetryCustomScreensBars); l++) {
    barsCB[l]->setCurrentIndex(barsCB[l]->findData(screen.body.bars[l].source.toValue()));
  }

  if (screen.type == TELEMETRY_SCREEN_BARS) {
    for (int i = 0; i < firmware->getCapability(TelemetryCustomScreensBars); i++) {
      updateBar(i);
    }
  }

  lock = false;
}

void CustomScreen::updateBar(int line)
{
  lock = true;

  RawSource source = screen.body.bars[line].source;

  minTime[line]->setVisible(false);
  maxTime[line]->setVisible(false);

  if (source.type != SOURCE_TYPE_NONE) {
    RawSourceRange range = source.getRange(model, generalSettings);
    float minVal = range.getValue(screen.body.bars[line].barMin);
    float maxVal = screen.body.bars[line].barMax;
    maxVal = range.getValue(maxVal);

    if (source.type == SOURCE_TYPE_TIMER) {
      minTime[line]->setVisible(true);
      minTime[line]->setTimeRange(range.min, range.max);
      minTime[line]->setSingleStep(range.step);
      minTime[line]->setPageStep(range.step * 60);
      minTime[line]->setShowSeconds(range.step != 60);
      minTime[line]->setTime((int)minVal);

      maxTime[line]->setVisible(true);
      maxTime[line]->setTimeRange(range.min, range.max);
      maxTime[line]->setSingleStep(range.step);
      maxTime[line]->setPageStep(range.step * 60);
      maxTime[line]->setShowSeconds(range.step != 60);
      maxTime[line]->setTime((int)maxVal);

      minSB[line]->setVisible(false);
      maxSB[line]->setVisible(false);
    }
    else {
      minSB[line]->setVisible(true);
      minSB[line]->setEnabled(true);
      minSB[line]->setDecimals(range.decimals);
      minSB[line]->setMinimum(range.min);
      minSB[line]->setMaximum(range.max);
      minSB[line]->setSingleStep(range.step);
      minSB[line]->setSuffix(" " + range.unit);
      minSB[line]->setValue(minVal);

      maxSB[line]->setVisible(true);
      maxSB[line]->setEnabled(true);
      maxSB[line]->setDecimals(range.decimals);
      maxSB[line]->setMinimum(range.min);
      maxSB[line]->setMaximum(range.max);
      maxSB[line]->setSingleStep(range.step);
      maxSB[line]->setSuffix(" " + range.unit);
      maxSB[line]->setValue(maxVal);
    }
  }
  else {
    minSB[line]->setDisabled(true);
    maxSB[line]->setDisabled(true);
  }

  lock = false;
}

void CustomScreen::on_screenType_currentIndexChanged(int index)
{
  if (!isLocked()) {
    memset(reinterpret_cast<void *>(&screen.body), 0, sizeof(screen.body));
    update();
    emit modified();
  }
}

void CustomScreen::scriptNameEdited()
{
  if (!isLocked()) {
    lock = true;
    Helpers::getFileComboBoxValue(ui->scriptName, screen.body.script.filename, 8);
    emit modified();
    lock = false;
  }
}

void CustomScreen::customFieldChanged(int value)
{
  if (isLocked() || !sender() || !qobject_cast<QComboBox *>(sender()))
    return;

  bool ok;
  const int index = sender()->property("index").toInt(&ok),
      i = index / 256,
      m = index % 256;
  const RawSource src(qobject_cast<QComboBox *>(sender())->itemData(value).toInt());
  if (ok && screen.body.lines[i].source[m].toValue() != src.toValue()) {
    screen.body.lines[i].source[m] = src;
    emit modified();
  }
}

void CustomScreen::barSourceChanged(int value)
{
  if (isLocked() || !sender() || !qobject_cast<QComboBox *>(sender()))
    return;

  bool ok;
  const int index = sender()->property("index").toInt(&ok);
  const RawSource src(qobject_cast<QComboBox *>(sender())->itemData(value).toInt());
  if (!ok || screen.body.bars[index].source.toValue() == src.toValue())
    return;

  screen.body.bars[index].source = src;
  screen.body.bars[index].barMin = 0;
  screen.body.bars[index].barMax = 0;
  updateBar(index);
  emit modified();
}

void CustomScreen::barMinChanged(double value)
{
  if (!isLocked()) {
    int line = sender()->property("index").toInt();
    screen.body.bars[line].barMin = round(value / minSB[line]->singleStep());
    // TODO set min (maxSB)
    emit modified();
  }
}

void CustomScreen::barMaxChanged(double value)
{
  if (!isLocked()) {
    int line = sender()->property("index").toInt();
    screen.body.bars[line].barMax = round((value) / maxSB[line]->singleStep());
    // TODO set max (minSB)
    emit modified();
  }
}

void CustomScreen::barTimeChanged()
{
  if (!isLocked()) {
    int line = sender()->property("index").toInt();
    int & valRef = (sender()->property("type").toString() == "min" ? screen.body.bars[line].barMin : screen.body.bars[line].barMax);
    TimerEdit * te = qobject_cast<TimerEdit *>(sender());
    if (!te)
      return;

    valRef = round(te->timeInSeconds() / te->singleStep());

    emit modified();
  }
}

void CustomScreen::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &CustomScreen::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &CustomScreen::onItemModelUpdateComplete);
}

void CustomScreen::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void CustomScreen::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    //  leave updating to parent
    lock = false;
  }
}

/******************************************************/

TelemetryCustomScreensPanel::TelemetryCustomScreensPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                               CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  sharedItemModels(sharedItemModels),
  modelsUpdateCnt(0)
{
  panelFilteredItemModels = new FilteredItemModelFactory();
  int id;

  id = panelFilteredItemModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource)),
                                                  FIM_RAWSOURCE);
  connectItemModelEvents(id);

  grid = new QGridLayout(this);
  tabWidget = new QTabWidget(this);

  for (int i = 0; i < firmware->getCapability(TelemetryCustomScreens); i++) {
    CustomScreen * tab = new CustomScreen(this, model, model.frsky.screens[i], generalSettings, firmware, lock,
                                                            panelFilteredItemModels);
    tab->setProperty("index", i + 1);
    tabWidget->addTab(tab, getTabName(i + 1));
    panels << tab;
    connect(tab, &CustomScreen::modified, this, &TelemetryCustomScreensPanel::onModified);
  }

  grid->addWidget(tabWidget, 0, 0, 1, 1);

  addHSpring(grid, grid->columnCount(), 0);
  addVSpring(grid, 0, grid->rowCount());
  disableMouseScrolling();
}

TelemetryCustomScreensPanel::~TelemetryCustomScreensPanel()
{
  delete panelFilteredItemModels;
  foreach(GenericPanel * gp, panels) {
    delete gp;
  }
}

QString TelemetryCustomScreensPanel::getTabName(int index)
{
  return tr("Telemetry screen %1").arg(index);
}

void TelemetryCustomScreensPanel::update()
{
  lock = true;

  foreach(GenericPanel * pnl, panels) {
    pnl->update();
  }

  lock = false;
}

void TelemetryCustomScreensPanel::onModified()
{
  emit modified();
}

void TelemetryCustomScreensPanel::on_dataModifiedSensor()
{
  sharedItemModels->update(AbstractItemModel::IMUE_TeleSensors);
  emit modified();
}

void TelemetryCustomScreensPanel::connectItemModelEvents(const int id)
{
  FilteredItemModel * itemModel = panelFilteredItemModels->getItemModel(id);
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &TelemetryCustomScreensPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &TelemetryCustomScreensPanel::onItemModelUpdateComplete);
}

void TelemetryCustomScreensPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void TelemetryCustomScreensPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    update();
    lock = false;
  }
}
