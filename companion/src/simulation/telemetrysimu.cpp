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

#include "telemetrysimu.h"
#include "ui_telemetrysimu.h"
#include "appdata.h"
#include "simulatorinterface.h"
#include "telem_data.h"
#include "radio/src/telemetry/frsky_defs.h"
#include "telemetryproviderfrsky.h"
#include "telemetryproviderfrskyhub.h"
#include "telemetryprovidercrossfire.h"

#include <QRegularExpression>
#include <stdint.h>

#include <QDebug>

TelemetrySimulator::TelemetrySimulator(QWidget * parent, SimulatorInterface * simulator):
  QWidget(parent),
  ui(new Ui::TelemetrySimulator),
  simulator(simulator),
  m_simuStarted(false),
  m_logReplayEnable(false),
  logPlayback(new LogPlaybackController(ui, this))
{
  ui->setupUi(this);

  ui->Simulate->setChecked(g.currentProfile().telemSimEnabled());
  ui->cbPauseOnHide->setChecked(g.currentProfile().telemSimPauseOnHide());
  ui->cbResetRssiOnStop->setChecked(g.currentProfile().telemSimResetRssiOnStop());

  internalProvider = NULL;
  externalProvider = NULL;

  timer.setInterval(10);
  connect(&timer,    &QTimer::timeout, this, &TelemetrySimulator::generateTelemetryFrame);
  connect(&logTimer, &QTimer::timeout, this, &TelemetrySimulator::onLogTimerEvent);

  connect(ui->Simulate,          &QCheckBox::toggled, [&](bool on) { g.currentProfile().telemSimEnabled(on);         });
  connect(ui->cbPauseOnHide,     &QCheckBox::toggled, [&](bool on) { g.currentProfile().telemSimPauseOnHide(on);     });
  connect(ui->cbResetRssiOnStop, &QCheckBox::toggled, [&](bool on) { g.currentProfile().telemSimResetRssiOnStop(on); });

  connect(ui->loadLogFile,       &QPushButton::released,    this, &TelemetrySimulator::onLoadLogFile);
  connect(ui->play,              &QPushButton::released,    this, &TelemetrySimulator::onPlay);
  connect(ui->rewind,            &QPushButton::clicked,     this, &TelemetrySimulator::onRewind);
  connect(ui->stepForward,       &QPushButton::clicked,     this, &TelemetrySimulator::onStepForward);
  connect(ui->stepBack,          &QPushButton::clicked,     this, &TelemetrySimulator::onStepBack);
  connect(ui->stop,              &QPushButton::clicked,     this, &TelemetrySimulator::onStop);
  connect(ui->positionIndicator, &QScrollBar::valueChanged, this, &TelemetrySimulator::onPositionIndicatorChanged);
  connect(ui->replayRate,        &QSlider::valueChanged,    this, &TelemetrySimulator::onReplayRateChanged);

  connect(ui->internalTelemetrySelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TelemetrySimulator::onInternalTelemetrySelectorChanged);
  connect(ui->externalTelemetrySelector, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &TelemetrySimulator::onExternalTelemetrySelectorChanged);

  connect(this, &TelemetrySimulator::internalTelemetryDataChanged, simulator, &SimulatorInterface::sendInternalModuleTelemetry);
  connect(this, &TelemetrySimulator::externalTelemetryDataChanged, simulator, &SimulatorInterface::sendExternalModuleTelemetry);

  connect(simulator,           &SimulatorInterface::started,              this,      &TelemetrySimulator::onSimulatorStarted);
  connect(simulator,           &SimulatorInterface::stopped,              this,      &TelemetrySimulator::onSimulatorStopped);
  connect(&g.currentProfile(), &Profile::telemSimEnabledChanged,          this,      &TelemetrySimulator::onSimulateToggled);
}

TelemetrySimulator::~TelemetrySimulator()
{
  stopTelemetry();
  delete logPlayback;
  delete ui;
}

TelemetryProvider * TelemetrySimulator::getInternalTelemetryProvider()
{
  return internalProvider;
}

TelemetryProvider * TelemetrySimulator::getExternalTelemetryProvider()
{
  return externalProvider;
}

void TelemetrySimulator::hideEvent(QHideEvent *event)
{
  if (g.currentProfile().telemSimPauseOnHide())
    stopTelemetry();
  event->accept();
}

void TelemetrySimulator::showEvent(QShowEvent * event)
{
  if (g.currentProfile().telemSimEnabled())
    startTelemetry();
}

void TelemetrySimulator::startTelemetry()
{
  if (!m_simuStarted)
    return;

  timer.start();
  if (m_logReplayEnable)
    onPlay();
}

void TelemetrySimulator::stopTelemetry()
{
  timer.stop();
  m_logReplayEnable = logTimer.isActive();
  onStop();

  if (!g.currentProfile().telemSimResetRssiOnStop())
    return;
  if (internalProvider)
    internalProvider->resetRssi();
  if (externalProvider)
    externalProvider->resetRssi();
}

void TelemetrySimulator::onSimulatorStarted()
{
  m_simuStarted = true;
  if (isVisible() && g.currentProfile().telemSimEnabled())
    startTelemetry();
}

void TelemetrySimulator::onSimulatorStopped()
{
  m_simuStarted = false;
  stopTelemetry();
}

void TelemetrySimulator::onSimulateToggled(bool isChecked)
{
  if (isChecked)
    startTelemetry();
  else
    stopTelemetry();
}

TelemetryProvider * TelemetrySimulator::newTelemetryProviderFromDropdownChoice(int selectedIndex, QScrollArea * parent, bool isExternal)
{
  switch(selectedIndex) {
  case 0:
    return NULL;
  case 1:
    {
      TelemetryProviderCrossfire * newProvider =  new TelemetryProviderCrossfire(parent);
      parent->setWidget(newProvider);
      if (isExternal) {
        connect(newProvider, &TelemetryProviderCrossfire::telemetryDataChanged, this, &TelemetrySimulator::onExternalTelemetryProviderDataChanged);
      } else {
        connect(newProvider, &TelemetryProviderCrossfire::telemetryDataChanged, this, &TelemetrySimulator::onInternalTelemetryProviderDataChanged);
      }
      return newProvider;
    }
  case 2:
    {
      TelemetryProviderFrSkyHub * newProvider =  new TelemetryProviderFrSkyHub(parent);
      parent->setWidget(newProvider);
      if (isExternal) {
        connect(newProvider, &TelemetryProviderFrSkyHub::telemetryDataChanged, this, &TelemetrySimulator::onExternalTelemetryProviderDataChanged);
      } else {
        connect(newProvider, &TelemetryProviderFrSkyHub::telemetryDataChanged, this, &TelemetrySimulator::onInternalTelemetryProviderDataChanged);
      }
      return newProvider;
    }
  case 3:
    {
      TelemetryProviderFrSky * newProvider =  new TelemetryProviderFrSky(parent);
      parent->setWidget(newProvider);
      if (isExternal) {
        connect(newProvider, &TelemetryProviderFrSky::telemetryDataChanged, this, &TelemetrySimulator::onExternalTelemetryProviderDataChanged);
      } else {
        connect(newProvider, &TelemetryProviderFrSky::telemetryDataChanged, this, &TelemetrySimulator::onInternalTelemetryProviderDataChanged);
      }
      // FrSky provider needs to set up sensor instances from the model's telemetry configuration
      newProvider->loadUiFromSimulator(simulator);
      return newProvider;
    }
  default:
    qDebug() << "Unimplemented telemetry provider " << selectedIndex;
  }
  return NULL;
}

void TelemetrySimulator::onInternalTelemetrySelectorChanged(int selectedIndex)
{
  if (internalProvider) {
    delete internalProvider;
  }
  internalProvider = newTelemetryProviderFromDropdownChoice(selectedIndex, ui->internalScrollArea, false);
}

void TelemetrySimulator::onExternalTelemetrySelectorChanged(int selectedIndex)
{
  if (externalProvider) {
    delete externalProvider;
  }
  externalProvider = newTelemetryProviderFromDropdownChoice(selectedIndex, ui->externalScrollArea, true);
}

void TelemetrySimulator::onInternalTelemetryProviderDataChanged(const quint8 protocol, const QByteArray data)
{
  emit internalTelemetryDataChanged(protocol, data);
}

void TelemetrySimulator::onExternalTelemetryProviderDataChanged(const quint8 protocol, const QByteArray data)
{
  emit externalTelemetryDataChanged(protocol, data);
}

void TelemetrySimulator::generateTelemetryFrame()
{
  if (internalProvider)
    internalProvider->generateTelemetryFrame(simulator);
  if (externalProvider)
    externalProvider->generateTelemetryFrame(simulator);
}

void TelemetrySimulator::onLogTimerEvent()
{
  logPlayback->stepForward(false);
}

void TelemetrySimulator::onLoadLogFile()
{
  onStop(); // in case we are in playback mode
  logPlayback->loadLogFile();
}

void TelemetrySimulator::onPlay()
{
  ui->Simulate->setChecked(true);
  if (logPlayback->isReady()) {
    logTimer.start(logPlayback->logFrequency * 1000 / SPEEDS[ui->replayRate->value()]);
    logPlayback->play();
  }
}

void TelemetrySimulator::onRewind()
{
  if (logPlayback->isReady()) {
    logTimer.stop();
    logPlayback->rewind();
  }
}

void TelemetrySimulator::onStepForward()
{
  if (logPlayback->isReady()) {
    logTimer.stop();
    logPlayback->stepForward(true);
  }
}

void TelemetrySimulator::onStepBack()
{
  if (logPlayback->isReady()) {
    logTimer.stop();
    logPlayback->stepBack();
  }
}

void TelemetrySimulator::onStop()
{
  if (logPlayback->isReady()) {
    logTimer.stop();
    logPlayback->stop();
  }
}

void TelemetrySimulator::onPositionIndicatorChanged(int value)
{
  if (logPlayback->isReady()) {
    logPlayback->updatePositionLabel(value);
    logPlayback->setUiDataValues();
  }
}

void TelemetrySimulator::onReplayRateChanged(int value)
{
  if (logTimer.isActive()) {
    logTimer.setInterval(logPlayback->logFrequency * 1000 / SPEEDS[ui->replayRate->value()]);
  }
}


TelemetrySimulator::LogPlaybackController::LogPlaybackController(Ui::TelemetrySimulator * ui, TelemetrySimulator * sim)
{
  TelemetrySimulator::LogPlaybackController::sim = sim;
  TelemetrySimulator::LogPlaybackController::ui = ui;
  stepping = false;
}

QString convertFeetToMeters(QString input)
{
  double meters = input.toDouble() * 0.3048;
  return QString::number(meters);
}

QString convertFahrenheitToCelsius(QString input)
{
  double celsius = (input.toDouble() - 32.0) * 0.5556;
  return QString::number(celsius);
}

QString convertKnotsToKPH(QString input)
{
  double kph = input.toDouble() * 1.852;
  return QString::number(kph);
}

QString convertMilesPerHourToKPH(QString input)
{
  double kph = input.toDouble() * 1.60934;
  return QString::number(kph);
}

QString convertMetersPerSecondToKPH(QString input)
{
  double kph = input.toDouble() * 3.6;
  return QString::number(kph);
}

QString convertFeetPerSecondToKPH(QString input)
{
  double kph = input.toDouble() * 1.09728;
  return QString::number(kph);
}

QString convertDegreesToRadians(QString input)
{
  double rad = input.toDouble() * 0.0174533;
  return QString::number(rad);
}

QString convertRadiansToDegrees(QString input)
{
  double deg = input.toDouble() * 57.2958;
  return QString::number(deg);
}

QString convertWattToMilliwatt(QString input)
{
  double mw = input.toDouble() * 1000;
  return QString::number(mw);
}

QString convertFluidOuncesToMilliliters(QString input)
{
  double ml = input.toDouble() * 29.5735;
  return QString::number(ml);
}

QString convertDBMToMilliwatts(QString input)
{
  double dbm = input.toDouble();
  double mw = qPow(10, dbm/10);
  return QString::number(mw);
}

struct unitConversion {
  QString source;
  QString destination;
  QString (*converter)(QString);
} conversions[] = {
    {QString("ft"), QString("m"), convertFeetToMeters},
    {QString("°F"), QString("°C"), convertFahrenheitToCelsius},
    {QString("kts"), QString("kmh"), convertKnotsToKPH},
    {QString("mph"), QString("kmh"), convertMilesPerHourToKPH},
    {QString("m/s"), QString("kmh"), convertMetersPerSecondToKPH},
    {QString("f/s"), QString("kmh"), convertFeetPerSecondToKPH},
    {QString("°"), QString("rad"), convertDegreesToRadians},
    {QString("rad"), QString("°"), convertRadiansToDegrees},
    {QString("W"), QString("mW"), convertWattToMilliwatt},
    {QString("fOz"), QString("ml"), convertFluidOuncesToMilliliters},
    {QString("dBm"), QString("mW"), convertDBMToMilliwatts},
    {NULL, NULL, NULL},
  };

QString convertItemValue(QString sourceUnit, QString destUnit, QString value)
{
  if (sourceUnit == destUnit)
    return value;

  int i = 0;
  while (conversions[i].source != NULL) {
    if (conversions[i].source == sourceUnit && conversions[i].destination == destUnit) {
      return ((conversions[i].converter)(value));
    }
    i++;
  }
  qDebug() << "TelemetrySimulator::LogPlaybackController: failed to convert " << sourceUnit << " to " << destUnit;
  return value;
}

QDateTime TelemetrySimulator::LogPlaybackController::parseTransmitterTimestamp(QString row)
{
  QStringList rowParts = row.simplified().split(',');
  if (rowParts.size() < 2) {
    return QDateTime();
  }
  QString datePart = rowParts[0];
  QString timePart = rowParts[1];
  QDateTime result;
  QString format("yyyy-MM-dd hh:mm:ss.zzz"); // assume this format
  // hour can be 'missing'
  if (timePart.count(":") < 2) {
    timePart = "00:" + timePart;
  }
  if (datePart.contains("/")) { // happens when csv is edited by Excel
    format = "M/d/yyyy hh:mm:ss.z";
  }
  return QDateTime::fromString(datePart + " " + timePart, format);
}

void TelemetrySimulator::LogPlaybackController::calcLogFrequency()
{
  // examine up to 20 rows to determine log frequency in seconds
  // Skip the first entry which contains the file open time
  logFrequency = 25.5; // default value
  QDateTime lastTime;
  for (int i = 2; (i < 21) && (i < csvRecords.count()); i++)
  {
    QDateTime logTime = parseTransmitterTimestamp(csvRecords[i]);
    // ugh - no timespan in this Qt version
    double timeDiff = (logTime.toMSecsSinceEpoch() - lastTime.toMSecsSinceEpoch()) / 1000.0;
    if ((timeDiff > 0.09) && (timeDiff < logFrequency)) {
      logFrequency = timeDiff;
    }
    lastTime = logTime;
  }
}

bool TelemetrySimulator::LogPlaybackController::isReady()
{
  return csvRecords.count() > 1;
}

void TelemetrySimulator::LogPlaybackController::loadLogFile()
{
  QString logFileNameAndPath = QFileDialog::getOpenFileName(NULL, tr("Log File"), g.logDir(), tr("LOG Files (*.csv)"));
  if (logFileNameAndPath.isEmpty())
    return;

  g.logDir(logFileNameAndPath);

  // reset the playback ui
  ui->play->setEnabled(false);
  ui->rewind->setEnabled(false);
  ui->stepBack->setEnabled(false);
  ui->stepForward->setEnabled(false);
  ui->stop->setEnabled(false);
  ui->positionIndicator->setEnabled(false);
  ui->replayRate->setEnabled(false);
  ui->positionLabel->setText("Row #\nTimestamp");

  // clear existing data
  csvRecords.clear();

  QFile file(logFileNameAndPath);
  if (!file.open(QIODevice::ReadOnly)) {
    ui->logFileLabel->setText(tr("ERROR - invalid file"));
    return;
  }
  while (!file.atEnd()) {
    QByteArray line = file.readLine();
    csvRecords.append(line.simplified());
  }
  file.close();
  if (csvRecords.count() > 1) {
    columnNames.clear();
    QStringList keys = csvRecords[0].split(',');
    // override the first two column names
    keys[0] = "LogDate";
    keys[1] = "LogTime";
    Q_FOREACH(QString key, keys) {
      columnNames.append(key.simplified());
    }
    ui->play->setEnabled(true);
    ui->rewind->setEnabled(true);
    ui->stepBack->setEnabled(true);
    ui->stepForward->setEnabled(true);
    ui->stop->setEnabled(true);
    ui->positionIndicator->setEnabled(true);
    ui->replayRate->setEnabled(true);
    recordIndex = 1;
    calcLogFrequency();
  }
  ui->logFileLabel->setText(QFileInfo(logFileNameAndPath).fileName());
  rewind();
  return;
}

void TelemetrySimulator::LogPlaybackController::play()
{
}

void TelemetrySimulator::LogPlaybackController::stop()
{
}

void TelemetrySimulator::LogPlaybackController::rewind()
{
  stepping = true;
  recordIndex = 1;
  ui->stop->setChecked(true);
  updatePositionLabel(-1);
  setUiDataValues();
  stepping = false;
}

void TelemetrySimulator::LogPlaybackController::stepForward(bool focusOnStop)
{
  stepping = true;
  if (recordIndex < (csvRecords.count() - 1)) {
    recordIndex++;
    if (focusOnStop) {
      ui->stop->setChecked(true);
    }
    updatePositionLabel(-1);
    setUiDataValues();
  }
  else {
    rewind(); // always loop at the end
  }
  stepping = false;
}

void TelemetrySimulator::LogPlaybackController::stepBack()
{
  stepping = true;
  if (recordIndex > 1) {
    recordIndex--;
    ui->stop->setChecked(true);
    updatePositionLabel(-1);
    setUiDataValues();
  }
  stepping = false;
}

void TelemetrySimulator::LogPlaybackController::updatePositionLabel(int32_t percentage)
{
  if ((percentage > 0) && (!stepping)) {
    recordIndex = qFloor((double)csvRecords.count() / 100.0 * percentage);
    if (recordIndex == 0) {
      recordIndex = 1; // record 0 is column labels
    }
  }
  // format the transmitter date info
  QDateTime transmitterTimestamp = parseTransmitterTimestamp(csvRecords[recordIndex]);
  QString format("yyyy-MM-dd hh:mm:ss.z");
  ui->positionLabel->setText("Row " + QString::number(recordIndex) + " of " + QString::number(csvRecords.count() - 1)
              + "\n" + transmitterTimestamp.toString(format));
  if (percentage < 0) { // did we step past a threshold?
    uint32_t posPercent = (recordIndex / (double)(csvRecords.count() - 1)) * 100;
    ui->positionIndicator->setValue(posPercent);
  }
}

QString unitFromColumnName(QString columnName)
{
  QStringList parts = columnName.split('(');
  if (parts.count() == 1) {
    return QString("");
  }
  parts = parts[1].split(')');
  return parts[0];
}

QString itemFromColumnName(QString columnName)
{
  QStringList parts = columnName.split('(');
  return parts[0];
}

void TelemetrySimulator::LogPlaybackController::setUiDataValues()
{
  QStringList columnData = csvRecords[recordIndex].split(',');

  TelemetryProvider *internalProvider = sim->getInternalTelemetryProvider();
  TelemetryProvider *externalProvider = sim->getExternalTelemetryProvider();

  QHash<QString, QString> * internalSupportedItems = NULL;
  if (internalProvider)
    internalSupportedItems = internalProvider->getSupportedLogItems();
  QHash<QString, QString> * externalSupportedItems = NULL;
  if (externalProvider)
    externalSupportedItems = externalProvider->getSupportedLogItems();

  for (int col = 0; col < columnData.count(); col++) {
    QString columnName = columnNames[col];
    QString columnValue = columnData[col];

    QString item = itemFromColumnName(columnName);
    QString suppliedUnit = unitFromColumnName(columnName);

    if (internalSupportedItems && internalSupportedItems->contains(item)) {
      QString expectedUnit = internalSupportedItems->value(item);

      internalProvider->loadItemFromLog(item, convertItemValue(suppliedUnit, expectedUnit, columnValue));
    }
    if (externalSupportedItems && externalSupportedItems->contains(item)) {
      QString expectedUnit = externalSupportedItems->value(item);

      externalProvider->loadItemFromLog(item, convertItemValue(suppliedUnit, expectedUnit, columnValue));
    }
  }
}
