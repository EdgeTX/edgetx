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

#include "telemetry.h"
#include "ui_telemetry.h"
#include "ui_telemetry_sensor.h"
#include "helpers.h"
#include "appdata.h"
#include "namevalidator.h"

constexpr char FIM_RAWSOURCE[]       {"Raw Source"};
constexpr char FIM_TELEALLSRC[]      {"Tele All Source"};
constexpr char FIM_TELEPOSSRC[]      {"Tele Pos Source"};
constexpr char FIM_SENSORTYPE[]      {"Sensor.Type"};
constexpr char FIM_SENSORFORMULA[]   {"Sensor.Formula"};
constexpr char FIM_SENSORCELLINDEX[] {"Sensor.CellIndex"};
constexpr char FIM_SENSORUNIT[]      {"Sensor.Unit"};
constexpr char FIM_SENSORPRECISION[] {"Sensor.Precision"};
constexpr char FIM_TELEVARIOSRC[]    {"Tele Vario Source"};

TelemetrySensorPanel::TelemetrySensorPanel(QWidget *parent, SensorData & sensor, int sensorIndex, int sensorCapability, ModelData & model,
                                           GeneralSettings & generalSettings, Firmware * firmware, const bool & parentLock,
                                           FilteredItemModelFactory * panelFilteredItemModels) :
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::TelemetrySensor),
  sensor(sensor),
  sensorIndex(sensorIndex),
  selectedIndex(0),
  sensorCapability(sensorCapability),
  modelsUpdateCnt(0),
  parentLock(parentLock)
{
  ui->setupUi(this);
  FilteredItemModel * fltmdl;

  connectItemModelEvents(panelFilteredItemModels->getItemModel(FIM_TELEALLSRC));
  connectItemModelEvents(panelFilteredItemModels->getItemModel(FIM_TELEPOSSRC));

  lock = true;
  ui->numLabel->setText(tr("TELE%1").arg(sensorIndex + 1));
  ui->numLabel->setProperty("index", sensorIndex);
  ui->numLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  QFontMetrics *f = new QFontMetrics(QFont());
  QSize sz;
  sz = f->size(Qt::TextSingleLine, "TELE00");
  delete f;
  ui->numLabel->setMinimumWidth(sz.width());
  ui->numLabel->setContextMenuPolicy(Qt::CustomContextMenu);
  ui->numLabel->setToolTip(tr("Popup menu available"));
  ui->numLabel->setMouseTracking(true);
  connect(ui->numLabel, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(on_customContextMenuRequested(QPoint)));
  ui->name->setValidator(new NameValidator(firmware->getBoard(), this));
  ui->name->setField(sensor.label, SENSOR_LABEL_LEN);
  connect(ui->name, SIGNAL(currentDataChanged()), this, SLOT(on_nameDataChanged()));
  ui->id->setField(sensor.id, this);
  ui->instance->setField(sensor.instance, this);
  ui->type->setModel(panelFilteredItemModels->getItemModel(FIM_SENSORTYPE));
  ui->type->setField(sensor.type);
  connect(ui->type, SIGNAL(currentDataChanged(int)), this, SLOT(update()));
  ui->formula->setModel(panelFilteredItemModels->getItemModel(FIM_SENSORFORMULA));
  ui->formula->setField(sensor.formula);
  connect(ui->formula, SIGNAL(currentDataChanged(int)), this, SLOT(on_formulaDataChanged()));
  ui->unit->setModel(panelFilteredItemModels->getItemModel(FIM_SENSORUNIT));
  ui->unit->setField(sensor.unit);
  connect(ui->unit, SIGNAL(currentDataChanged(int)), this, SLOT(on_unitDataChanged()));
  ui->ratio->setField(sensor.ratio, this);
  ui->offset->setField(sensor.offset, this);
  ui->autoOffset->setField(sensor.autoOffset, this);
  ui->filter->setField(sensor.filter, this);
  ui->logs->setField(sensor.logs, this);
  ui->persistent->setField(sensor.persistent, this);
  ui->onlyPositive->setField(sensor.onlyPositive, this);
  fltmdl = panelFilteredItemModels->getItemModel(FIM_TELEPOSSRC);
  ui->gpsSensor->setModel(fltmdl);
  ui->gpsSensor->setField(sensor.gps, this);
  ui->altSensor->setModel(fltmdl);
  ui->altSensor->setField(sensor.alt, this);
  ui->ampsSensor->setModel(fltmdl);
  ui->ampsSensor->setField(sensor.amps, this);
  ui->cellsSensor->setModel(fltmdl);
  ui->cellsSensor->setField(sensor.source, this);
  ui->cellsIndex->setModel(panelFilteredItemModels->getItemModel(FIM_SENSORCELLINDEX));
  ui->cellsIndex->setField(sensor.index);
  fltmdl = panelFilteredItemModels->getItemModel(FIM_TELEALLSRC);
  ui->source1->setModel(fltmdl);
  ui->source1->setField(sensor.sources[0], this);
  ui->source2->setModel(fltmdl);
  ui->source2->setField(sensor.sources[1], this);
  ui->source3->setModel(fltmdl);
  ui->source3->setField(sensor.sources[2], this);
  ui->source4->setModel(fltmdl);
  ui->source4->setField(sensor.sources[3], this);
  ui->prec->setModel(panelFilteredItemModels->getItemModel(FIM_SENSORPRECISION));
  ui->prec->setField(sensor.prec);
  connect(ui->prec, SIGNAL(currentDataChanged(int)), this, SLOT(on_precDataChanged()));
  lock = false;

  update();
}

TelemetrySensorPanel::~TelemetrySensorPanel()
{
  delete ui;
}

void TelemetrySensorPanel::update()
{
  lock = true;
  int mask = sensor.getMask();

  ui->name->updateValue();
  ui->type->updateValue();
  ui->formula->updateValue();
  ui->unit->updateValue();
  ui->id->updateValue();
  ui->instance->updateValue();
  ui->ratio->updateValue();
  ui->offset->updateValue();
  ui->autoOffset->updateValue();
  ui->filter->updateValue();
  ui->logs->updateValue();
  ui->persistent->updateValue();
  ui->onlyPositive->updateValue();
  ui->gpsSensor->updateValue();
  ui->altSensor->updateValue();
  ui->ampsSensor->updateValue();
  ui->cellsSensor->updateValue();
  ui->cellsIndex->updateValue();
  ui->prec->updateValue();

  if (sensor.type == SensorData::TELEM_TYPE_CALCULATED) {
    sensor.updateUnit();
    ui->idLabel->hide();
    ui->id->hide();
    ui->instanceLabel->hide();
    ui->instance->hide();
    ui->originLabel->hide();
    ui->origin->hide();
    ui->formula->show();
    ui->formula->setCurrentIndex(ui->formula->findData(sensor.formula));
    ui->source1->updateValue();
    ui->source2->updateValue();
    ui->source3->updateValue();
    ui->source4->updateValue();
  }
  else {
    ui->idLabel->show();
    ui->id->show();
    ui->instanceLabel->show();
    ui->instance->show();
    QString origin = sensor.getOrigin(model);
    ui->originLabel->setVisible(!origin.isEmpty());
    ui->origin->setVisible(!origin.isEmpty());
    ui->origin->setText(origin);
    ui->formula->hide();
    FieldRange rng = sensor.getRatioRange();
    ui->ratio->setDecimals(rng.decimals);
    ui->ratio->setMaximum(rng.max);
    ui->ratio->setMinimum(rng.min);
    ui->ratio->setSingleStep(rng.step);
    rng = sensor.getOffsetRange();
    ui->offset->setDecimals(rng.decimals);
    ui->offset->setMaximum(rng.max);
    ui->offset->setMinimum(rng.min);
    ui->offset->setSingleStep(rng.step);
  }

  ui->ratioLabel->setVisible(mask & SENSOR_HAS_RATIO && sensor.unit != SensorData::UNIT_RPMS);
  ui->bladesLabel->setVisible(mask & SENSOR_HAS_RATIO && sensor.unit == SensorData::UNIT_RPMS);
  ui->ratio->setVisible(mask & SENSOR_HAS_RATIO);

  ui->offsetLabel->setVisible(mask & SENSOR_HAS_RATIO && sensor.unit != SensorData::UNIT_RPMS);
  ui->multiplierLabel->setVisible(mask & SENSOR_HAS_RATIO && sensor.unit == SensorData::UNIT_RPMS);
  ui->offset->setVisible(mask & SENSOR_HAS_RATIO);

  ui->precLabel->setVisible(mask & SENSOR_HAS_PRECISION);
  ui->prec->setVisible(mask & SENSOR_HAS_PRECISION);

  ui->unit->setVisible(mask & SENSOR_ISCONFIGURABLE);

  ui->gpsSensorLabel->setVisible(mask & SENSOR_HAS_GPS);
  ui->gpsSensor->setVisible(mask & SENSOR_HAS_GPS);

  ui->altSensorLabel->setVisible(mask & SENSOR_HAS_GPS);
  ui->altSensor->setVisible(mask & SENSOR_HAS_GPS);

  ui->ampsSensorLabel->setVisible(mask & SENSOR_HAS_CONSUMPTION || mask & SENSOR_HAS_TOTALIZE);
  ui->ampsSensor->setVisible(mask & SENSOR_HAS_CONSUMPTION || mask & SENSOR_HAS_TOTALIZE);

  ui->cellsSensorLabel->setVisible(mask & SENSOR_HAS_CELLS);
  ui->cellsSensor->setVisible(mask & SENSOR_HAS_CELLS);
  ui->cellsIndex->setVisible(mask & SENSOR_HAS_CELLS);

  ui->source1->setVisible(mask & SENSOR_HAS_SOURCES_12);
  ui->source2->setVisible(mask & SENSOR_HAS_SOURCES_12);
  ui->source3->setVisible(mask & SENSOR_HAS_SOURCES_34);
  ui->source4->setVisible(mask & SENSOR_HAS_SOURCES_34);

  ui->autoOffset->setEnabled(mask & SENSOR_HAS_RATIO && sensor.unit != SensorData::UNIT_RPMS);
  ui->onlyPositive->setEnabled(mask & SENSOR_HAS_POSITIVE);
  ui->filter->setEnabled(mask & SENSOR_ISCONFIGURABLE);
  ui->persistent->setEnabled(sensor.type == SensorData::TELEM_TYPE_CALCULATED);

  lock = false;
}

void TelemetrySensorPanel::on_nameDataChanged()
{
  emit dataModified();
}

void TelemetrySensorPanel::on_formulaDataChanged()
{
  sensor.formulaChanged();
  update();
}

void TelemetrySensorPanel::on_unitDataChanged()
{
  sensor.unitChanged();
  update();
  emit dataModified();
}

void TelemetrySensorPanel::on_precDataChanged()
{
  update();
}

void TelemetrySensorPanel::on_customContextMenuRequested(QPoint pos)
{
  QLabel *label = (QLabel *)sender();
  selectedIndex = label->property("index").toInt();
  QPoint globalPos = label->mapToGlobal(pos);

  QMenu contextMenu;
  contextMenu.addAction(CompanionIcon("copy.png"), tr("Copy"),this,SLOT(cmCopy()));
  contextMenu.addAction(CompanionIcon("cut.png"), tr("Cut"),this,SLOT(cmCut()));
  contextMenu.addAction(CompanionIcon("paste.png"), tr("Paste"),this,SLOT(cmPaste()))->setEnabled(hasClipboardData());
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear"),this,SLOT(cmClear()));
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("arrow-right.png"), tr("Insert"),this,SLOT(cmInsert()))->setEnabled(insertAllowed());
  contextMenu.addAction(CompanionIcon("arrow-left.png"), tr("Delete"),this,SLOT(cmDelete()));
  contextMenu.addAction(CompanionIcon("moveup.png"), tr("Move Up"),this,SLOT(cmMoveUp()))->setEnabled(moveUpAllowed());
  contextMenu.addAction(CompanionIcon("movedown.png"), tr("Move Down"),this,SLOT(cmMoveDown()))->setEnabled(moveDownAllowed());
  contextMenu.addSeparator();
  contextMenu.addAction(CompanionIcon("clear.png"), tr("Clear All"),this,SLOT(cmClearAll()));

  contextMenu.exec(globalPos);
}

bool TelemetrySensorPanel::hasClipboardData(QByteArray * data) const
{
  const QClipboard * clipboard = QApplication::clipboard();
  const QMimeData * mimeData = clipboard->mimeData();
  if (mimeData->hasFormat(MIMETYPE_TELE_SENSOR)) {
    if (data)
      data->append(mimeData->data(MIMETYPE_TELE_SENSOR));
    return true;
  }
  return false;
}

bool TelemetrySensorPanel::insertAllowed() const
{
  return ((selectedIndex < sensorCapability - 1) && (model->sensorData[sensorCapability - 1].isEmpty()));
}

bool TelemetrySensorPanel::moveDownAllowed() const
{
  return selectedIndex < sensorCapability - 1;
}

bool TelemetrySensorPanel::moveUpAllowed() const
{
  return selectedIndex > 0;
}

void TelemetrySensorPanel::cmCopy()
{
  QByteArray data;
  data.append((char*)&sensor, sizeof(SensorData));
  QMimeData *mimeData = new QMimeData;
  mimeData->setData(MIMETYPE_TELE_SENSOR, data);
  QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);
}

void TelemetrySensorPanel::cmCut()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Cut Telemetry Sensor. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  cmCopy();
  cmClear(false);
}

void TelemetrySensorPanel::cmPaste()
{
  QByteArray data;
  if (hasClipboardData(&data)) {
    memcpy(&sensor, data.constData(), sizeof(SensorData));
    emit dataModified();
  }
}

void TelemetrySensorPanel::cmClear(bool prompt)
{
  if (prompt) {
    if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear Telemetry Sensor. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
      return;
  }

  sensor.clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_SENSOR, ModelData::REF_UPD_ACT_CLEAR, selectedIndex);
  emit dataModified();
}

void TelemetrySensorPanel::cmClearAll()
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Clear all Telemetry Sensors. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  emit clearAllSensors();
}

void TelemetrySensorPanel::cmInsert()
{
  emit insertSensor(selectedIndex);
}

void TelemetrySensorPanel::cmDelete()
{
  emit deleteSensor(selectedIndex);
}

void TelemetrySensorPanel::cmMoveUp()
{
  emit moveUpSensor(selectedIndex);
}

void TelemetrySensorPanel::cmMoveDown()
{
  emit moveDownSensor(selectedIndex);
}

void TelemetrySensorPanel::connectItemModelEvents(const FilteredItemModel * itemModel)
{
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &TelemetrySensorPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &TelemetrySensorPanel::onItemModelUpdateComplete);
}

void TelemetrySensorPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void TelemetrySensorPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    //  leave updating to parent
    lock = false;
  }
}

/******************************************************/

TelemetryPanel::TelemetryPanel(QWidget *parent, ModelData & model, GeneralSettings & generalSettings, Firmware * firmware,
                               CompoundItemModelFactory * sharedItemModels):
  ModelPanel(parent, model, generalSettings, firmware),
  ui(new Ui::Telemetry),
  sharedItemModels(sharedItemModels),
  modelsUpdateCnt(0)
{
  ui->setupUi(this);

  panelItemModels = new CompoundItemModelFactory(&generalSettings, &model);
  panelFilteredItemModels = new FilteredItemModelFactory();
  int id;

  id = panelFilteredItemModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_RawSource)),
                                                  FIM_RAWSOURCE);
  connectItemModelEvents(id);

  id = panelFilteredItemModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_TeleSource)),
                                                  FIM_TELEALLSRC);
  connectItemModelEvents(id);

  id = panelFilteredItemModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_TeleSource),
                                                                        SensorData::SensorTypeContextPos),
                                                  FIM_TELEPOSSRC);
  connectItemModelEvents(id);

  id = panelFilteredItemModels->registerItemModel(new FilteredItemModel(sharedItemModels->getItemModel(AbstractItemModel::IMID_TeleSource),
                                                                        SensorData::SensorTypeContextVario),
                                                  FIM_TELEVARIOSRC);
  connectItemModelEvents(id);

  id = panelItemModels->registerItemModel(SensorData::typeItemModel());
  panelFilteredItemModels->registerItemModel(new FilteredItemModel(panelItemModels->getItemModel(id)), FIM_SENSORTYPE);

  id = panelItemModels->registerItemModel(SensorData::formulaItemModel());
  panelFilteredItemModels->registerItemModel(new FilteredItemModel(panelItemModels->getItemModel(id)), FIM_SENSORFORMULA);

  id = panelItemModels->registerItemModel(SensorData::cellIndexItemModel());
  panelFilteredItemModels->registerItemModel(new FilteredItemModel(panelItemModels->getItemModel(id)), FIM_SENSORCELLINDEX);

  id = panelItemModels->registerItemModel(SensorData::unitItemModel());
  panelFilteredItemModels->registerItemModel(new FilteredItemModel(panelItemModels->getItemModel(id)), FIM_SENSORUNIT);

  id = panelItemModels->registerItemModel(SensorData::precisionItemModel());
  panelFilteredItemModels->registerItemModel(new FilteredItemModel(panelItemModels->getItemModel(id)), FIM_SENSORPRECISION);

  sensorCapability = firmware->getCapability(Sensors);
  if (sensorCapability > CPN_MAX_SENSORS) //  TODO should be role of getCapability
    sensorCapability = CPN_MAX_SENSORS;

  if (firmware->getCapability(NoTelemetryProtocol)) {
    model.frsky.usrProto = 1;
  }

  ui->varioSource->setModel(panelFilteredItemModels->getItemModel(FIM_TELEVARIOSRC));
  ui->varioSource->setField(model.frsky.varioSource, this);
  ui->varioCenterSilent->setField(model.frsky.varioCenterSilent, this);
  ui->A1GB->hide();
  ui->A2GB->hide();

  for (int i = 0; i < sensorCapability; ++i) {
    TelemetrySensorPanel * panel = new TelemetrySensorPanel(this, model.sensorData[i], i, sensorCapability, model, generalSettings,
                                                            firmware, lock, panelFilteredItemModels);
    ui->sensorsLayout->addWidget(panel);
    sensorPanels[i] = panel;
    connect(panel, SIGNAL(dataModified()), this, SLOT(on_dataModifiedSensor()));
    connect(panel, SIGNAL(modified()), this, SLOT(onModified()));
    connect(panel, SIGNAL(clearAllSensors()), this, SLOT(on_clearAllSensors()));
    connect(panel, SIGNAL(insertSensor(int)), this, SLOT(on_insertSensor(int)));
    connect(panel, SIGNAL(deleteSensor(int)), this, SLOT(on_deleteSensor(int)));
    connect(panel, SIGNAL(moveUpSensor(int)), this, SLOT(on_moveUpSensor(int)));
    connect(panel, SIGNAL(moveDownSensor(int)), this, SLOT(on_moveDownSensor(int)));
  }

  if (IS_TARANIS_X9(firmware->getBoard())) {
    ui->voltsSource->setModel(panelFilteredItemModels->getItemModel(FIM_TELEPOSSRC));
    ui->voltsSource->setField(model.frsky.voltsSource, this);
    ui->altitudeSource->setModel(panelFilteredItemModels->getItemModel(FIM_TELEPOSSRC));
    ui->altitudeSource->setField(model.frsky.altitudeSource, this);
  }
  else {
    ui->topbarGB->hide();
  }

  disableMouseScrolling();

  setup();
}

TelemetryPanel::~TelemetryPanel()
{
  delete ui;
  delete panelFilteredItemModels;
  delete panelItemModels;
}

void TelemetryPanel::update()
{
  lock = true;

  if (IS_HORUS_OR_TARANIS(firmware->getBoard())) {
    ui->voltsSource->updateValue();
    ui->altitudeSource->updateValue();
    ui->varioSource->updateValue();
  }

  for (int i = 0; i < sensorCapability; ++i) {
    sensorPanels[i]->update();
  }

  lock = false;
}

void TelemetryPanel::setup()
{
  lock = true;

  ui->ignoreSensorIds->setField(model->frsky.ignoreSensorIds, this);
  ui->showInstanceIds->setField(model->showInstanceIds, this);
  ui->disableTelemetryAlarms->setField(model->rssiAlarms.disabled);

  ui->rssiAlarmWarningSB->setRange(0, 127);
  ui->rssiAlarmWarningSB->setValue(model->rssiAlarms.warning);
  ui->rssiAlarmCriticalSB->setRange(0, 127);
  ui->rssiAlarmCriticalSB->setValue(model->rssiAlarms.critical);

  ui->rssiAlarmWarningCB->hide();
  ui->rssiAlarmCriticalCB->hide();
  ui->rssiAlarmWarningLabel->setText(tr("Low Alarm"));
  ui->rssiAlarmCriticalLabel->setText(tr("Critical Alarm"));

  if (!firmware->getCapability(HasVario)) {
    ui->varioLimitMax_DSB->hide();
    ui->varioLimitMin_DSB->hide();
    ui->varioLimitCenterMin_DSB->hide();
    ui->varioLimitCenterMax_DSB->hide();
    ui->varioLimit_label->hide();
    ui->VarioLabel_1->hide();
    ui->VarioLabel_2->hide();
    ui->VarioLabel_3->hide();
    ui->VarioLabel_4->hide();
    ui->varioSource->hide();
    ui->varioSource_label->hide();
  }
  else {
    if (!firmware->getCapability(HasVarioSink)) {
      ui->varioLimitMin_DSB->hide();
      ui->varioLimitCenterMin_DSB->hide();
      ui->VarioLabel_1->hide();
      ui->VarioLabel_2->hide();
    }
    ui->varioLimitMin_DSB->setValue(model->frsky.varioMin - 10);
    ui->varioLimitMax_DSB->setValue(model->frsky.varioMax + 10);
    ui->varioLimitCenterMax_DSB->setValue((model->frsky.varioCenterMax / 10.0) + 0.5);
    ui->varioLimitCenterMin_DSB->setValue((model->frsky.varioCenterMin / 10.0) - 0.5);
  }

  ui->altimetryGB->setVisible(firmware->getCapability(HasVario)),
  ui->frskyProtoCB->setDisabled(firmware->getCapability(NoTelemetryProtocol));

  if (firmware->getCapability(Telemetry)) {
    ui->frskyProtoCB->addItem(tr("Winged Shadow How High"));
  }
  else {
    ui->frskyProtoCB->addItem(tr("Winged Shadow How High (not supported)"));
  }

  ui->variousGB->hide();

  lock = false;
}

void TelemetryPanel::onModified()
{
  emit modified();
}

void TelemetryPanel::on_bladesCount_editingFinished()
{
  if (!isLocked()) {
    model->frsky.blades = ui->bladesCount->value();
    emit modified();
  }
}

void TelemetryPanel::on_frskyProtoCB_currentIndexChanged(int index)
{
  if (!isLocked()) {
    model->frsky.usrProto = index;
    emit modified();
  }
}

void TelemetryPanel::on_rssiAlarmWarningSB_editingFinished()
{
  if (!isLocked()) {
    model->rssiAlarms.warning= ui->rssiAlarmWarningSB->value();
    emit modified();
  }
}

void TelemetryPanel::on_rssiAlarmCriticalSB_editingFinished()
{
  if (!isLocked()) {
    model->rssiAlarms.critical = ui->rssiAlarmCriticalSB->value();
    emit modified();
  }
}

void TelemetryPanel::on_varioLimitMin_DSB_editingFinished()
{
  if (!isLocked()) {
    model->frsky.varioMin = round(ui->varioLimitMin_DSB->value() + 10);
    emit modified();
  }
}

void TelemetryPanel::on_varioLimitMax_DSB_editingFinished()
{
  if (!isLocked()) {
    model->frsky.varioMax = round(ui->varioLimitMax_DSB->value() - 10);
    emit modified();
  }
}

void TelemetryPanel::on_varioLimitCenterMin_DSB_editingFinished()
{
  if (!isLocked()) {
    if (ui->varioLimitCenterMin_DSB->value() > ui->varioLimitCenterMax_DSB->value()) {
      ui->varioLimitCenterMax_DSB->setValue(ui->varioLimitCenterMin_DSB->value());
    }
    model->frsky.varioCenterMin = round((ui->varioLimitCenterMin_DSB->value() + 0.5) * 10);
    emit modified();
  }
}

void TelemetryPanel::on_varioLimitCenterMax_DSB_editingFinished()
{
  if (!isLocked()) {
    if (ui->varioLimitCenterMin_DSB->value() > ui->varioLimitCenterMax_DSB->value()) {
      ui->varioLimitCenterMax_DSB->setValue(ui->varioLimitCenterMin_DSB->value());
    }
    model->frsky.varioCenterMax = round((ui->varioLimitCenterMax_DSB->value() - 0.5) * 10);
    emit modified();
  }
}

void TelemetryPanel::on_fasOffset_DSB_editingFinished()
{
  if (!isLocked()) {
    model->frsky.fasOffset = ui->fasOffset_DSB->value() * 10;
    emit modified();
  }
}

void TelemetryPanel::on_mahCount_SB_editingFinished()
{
  if (!isLocked()) {
    model->frsky.storedMah = ui->mahCount_SB->value();
    emit modified();
  }
}

void TelemetryPanel::on_mahCount_ChkB_toggled(bool checked)
{
  if (!isLocked()) {
    model->frsky.mAhPersistent = checked;
    ui->mahCount_SB->setDisabled(!checked);
    emit modified();
  }
}

void TelemetryPanel::on_clearAllSensors()
{
  for (int i = 0; i < CPN_MAX_SENSORS; i++) {
    model->sensorData[i].clear();
    model->updateAllReferences(ModelData::REF_UPD_TYPE_SENSOR, ModelData::REF_UPD_ACT_CLEAR, i);
  }

  on_dataModifiedSensor();
}

void TelemetryPanel::on_insertSensor(int selectedIndex)
{
  memmove(&model->sensorData[selectedIndex + 1], &model->sensorData[selectedIndex], (CPN_MAX_SENSORS - (selectedIndex + 1)) * sizeof(SensorData));
  model->sensorData[selectedIndex].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_SENSOR, ModelData::REF_UPD_ACT_SHIFT, selectedIndex, 0, 1);
  on_dataModifiedSensor();
}

void TelemetryPanel::on_deleteSensor(int selectedIndex)
{
  if (QMessageBox::question(this, CPN_STR_APP_NAME, tr("Delete Sensor. Are you sure?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
    return;

  memmove(&model->sensorData[selectedIndex], &model->sensorData[selectedIndex + 1], (CPN_MAX_SENSORS - (selectedIndex + 1)) * sizeof(SensorData));
  model->sensorData[CPN_MAX_SENSORS - 1].clear();
  model->updateAllReferences(ModelData::REF_UPD_TYPE_SENSOR, ModelData::REF_UPD_ACT_SHIFT, selectedIndex, 0, -1);
  on_dataModifiedSensor();
}

void TelemetryPanel::on_moveUpSensor(int selectedIndex)
{
  swapData(selectedIndex, selectedIndex - 1);
}

void TelemetryPanel::on_moveDownSensor(int selectedIndex)
{
  swapData(selectedIndex, selectedIndex + 1);
}

void TelemetryPanel::swapData(int idx1, int idx2)
{
  if ((idx1 != idx2) && (!model->sensorData[idx1].isEmpty() || !model->sensorData[idx2].isEmpty())) {
    SensorData sdtmp = model->sensorData[idx2];
    SensorData *sd1 = &model->sensorData[idx1];
    SensorData *sd2 = &model->sensorData[idx2];
    memcpy(sd2, sd1, sizeof(SensorData));
    memcpy(sd1, &sdtmp, sizeof(SensorData));
    model->updateAllReferences(ModelData::REF_UPD_TYPE_SENSOR, ModelData::REF_UPD_ACT_SWAP, idx1, idx2);
    on_dataModifiedSensor();
  }
}

void TelemetryPanel::on_dataModifiedSensor()
{
  sharedItemModels->update(AbstractItemModel::IMUE_TeleSensors);
  emit modified();
}

void TelemetryPanel::connectItemModelEvents(const int id)
{
  FilteredItemModel * itemModel = panelFilteredItemModels->getItemModel(id);
  connect(itemModel, &FilteredItemModel::aboutToBeUpdated, this, &TelemetryPanel::onItemModelAboutToBeUpdated);
  connect(itemModel, &FilteredItemModel::updateComplete, this, &TelemetryPanel::onItemModelUpdateComplete);
}

void TelemetryPanel::onItemModelAboutToBeUpdated()
{
  lock = true;
  modelsUpdateCnt++;
}

void TelemetryPanel::onItemModelUpdateComplete()
{
  modelsUpdateCnt--;
  if (modelsUpdateCnt < 1) {
    update();
    lock = false;
  }
}
