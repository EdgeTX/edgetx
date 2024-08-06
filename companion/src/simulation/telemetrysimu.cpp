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

#include <QRegularExpression>
#include <stdint.h>

#include <QDebug>

TelemetrySimulator::TelemetrySimulator(QWidget * parent, SimulatorInterface * simulator):
  QWidget(parent),
  ui(new Ui::TelemetrySimulator),
  simulator(simulator),
  m_simuStarted(false),
  m_logReplayEnable(false),
  logPlayback(new LogPlaybackController(ui))
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

  connect(ui->internalTelemetrySelector, &QComboBox::currentTextChanged,
          this,                          &TelemetrySimulator::onInternalTelemetrySelectorChanged);
  connect(ui->externalTelemetrySelector, &QComboBox::currentTextChanged,
          this,                          &TelemetrySimulator::onExternalTelemetrySelectorChanged);

  connect(this,                &TelemetrySimulator::internalTelemetryDataChanged,
          simulator,           &SimulatorInterface::sendInternalModuleTelemetry);
  connect(this,                &TelemetrySimulator::externalTelemetryDataChanged,
          simulator,           &SimulatorInterface::sendExternalModuleTelemetry);

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
  if (internalProvider)
    internalProvider->loadUiFromSimulator(simulator);
  if (externalProvider)
    externalProvider->loadUiFromSimulator(simulator);
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

TelemetryProvider * TelemetrySimulator::newTelemetryProviderFromDropdownChoice(const QString &text, QScrollArea * parent)
{
  if (text == "None") {
    return NULL;
  }
  if (text == "FrSky S.Port") {
    TelemetryProviderFrSky * newProvider =  new TelemetryProviderFrSky(parent);
    parent->setWidget(newProvider);
    connect(newProvider, &TelemetryProviderFrSky::telemetryDataChanged,
            this,        &TelemetrySimulator::onInternalTelemetryProviderDataChanged);
    return newProvider;
  }
  qDebug() << "Unimplemented telemetry provider " << text;
  return NULL;
}

void TelemetrySimulator::onInternalTelemetrySelectorChanged(const QString &text)
{
  qDebug() << "internal changed: " << text;
  if (internalProvider) {
    ui->internalScrollArea->setWidget(NULL);
    delete internalProvider;
  }
  internalProvider = newTelemetryProviderFromDropdownChoice(text, ui->internalScrollArea);
}

void TelemetrySimulator::onExternalTelemetrySelectorChanged(const QString &text)
{
  qDebug() << "external changed: " << text;
  if (externalProvider) {
    ui->externalScrollArea->setWidget(NULL);
    delete externalProvider;
  }
  externalProvider = newTelemetryProviderFromDropdownChoice(text, ui->externalScrollArea);
}

void TelemetrySimulator::onInternalTelemetryProviderDataChanged(const quint8 protocol, const QByteArray data)
{
  qDebug() << "internal telemetry data " << protocol << ": " << data;
  emit internalTelemetryDataChanged(protocol, data);
}

void TelemetrySimulator::onExternalTelemetryProviderDataChanged(const quint8 protocol, const QByteArray data)
{
  qDebug() << "external telemetry data " << protocol << ": " << data;
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



TelemetrySimulator::LogPlaybackController::LogPlaybackController(Ui::TelemetrySimulator * ui)
{
  TelemetrySimulator::LogPlaybackController::ui = ui;
  stepping = false;
  logFileGpsCordsInDecimalFormat = false;
  // initialize the map - TODO: how should this be localized?
  colToFuncMap.clear();
  colToFuncMap.insert("RxBt(V)", RXBT_V);
  colToFuncMap.insert("RSSI(dB)", RSSI);
  colToFuncMap.insert("RAS", RAS);
  colToFuncMap.insert("A1", A1);
  colToFuncMap.insert("A1(V)", A1);
  colToFuncMap.insert("A2", A2);
  colToFuncMap.insert("A2(V)", A2);
  colToFuncMap.insert("A3", A3);
  colToFuncMap.insert("A3(V)", A3);
  colToFuncMap.insert("A4", A4);
  colToFuncMap.insert("A4(V)", A4);
  colToFuncMap.insert("Tmp1(@C)", T1_DEGC);
  colToFuncMap.insert("Tmp1(@F)", T1_DEGF);
  colToFuncMap.insert("Tmp2(@C)", T2_DEGC);
  colToFuncMap.insert("Tmp2(@F)", T2_DEGF);
  colToFuncMap.insert("RPM(rpm)", RPM);
  colToFuncMap.insert("Fuel(%)", FUEL);
  colToFuncMap.insert("Fuel(ml)", FUEL_QTY);
  colToFuncMap.insert("VSpd(m/s)", VSPD_MS);
  colToFuncMap.insert("VSpd(f/s)", VSPD_FS);
  colToFuncMap.insert("Alt(ft)", ALT_FEET);
  colToFuncMap.insert("Alt(m)", ALT_METERS);
  colToFuncMap.insert("VFAS(V)", FASV);
  colToFuncMap.insert("Curr(A)", FASC);
  colToFuncMap.insert("Cels(gRe)", CELS_GRE);
  colToFuncMap.insert("Cels(V)", CELS_GRE);
  colToFuncMap.insert("ASpd(kts)", ASPD_KTS);
  colToFuncMap.insert("ASpd(kmh)", ASPD_KMH);
  colToFuncMap.insert("ASpd(mph)", ASPD_MPH);
  colToFuncMap.insert("GAlt(ft)", GALT_FEET);
  colToFuncMap.insert("GAlt(m)", GALT_METERS);
  colToFuncMap.insert("GSpd(kts)", GSPD_KNTS);
  colToFuncMap.insert("GSpd(kmh)", GSPD_KMH);
  colToFuncMap.insert("GSpd(mph)", GSPD_MPH);
  colToFuncMap.insert("Hdg(@)", GHDG_DEG);
  colToFuncMap.insert("Date", GDATE);
  colToFuncMap.insert("GPS", G_LATLON);
  colToFuncMap.insert("AccX(g)", ACCX);
  colToFuncMap.insert("AccY(g)", ACCY);
  colToFuncMap.insert("AccZ(g)", ACCZ);

  // ACCX Y and Z
}

QDateTime TelemetrySimulator::LogPlaybackController::parseTransmittterTimestamp(QString row)
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

void TelemetrySimulator::LogPlaybackController::checkGpsFormat()
{
  // sample the first record to check if cords are in decimal format
  logFileGpsCordsInDecimalFormat = false;
  if(csvRecords.count() > 1) {
    QStringList keys = csvRecords[0].split(',');
    if(keys.contains("GPS")) {
      int gpsColIndex = keys.indexOf("GPS");
      QStringList firstRowVlues = csvRecords[1].split(',');
      QString gpsSample = firstRowVlues[gpsColIndex];
      QStringList cords = gpsSample.simplified().split(' ');
      if (cords.count() == 2) {
        // frsky and TBS crossfire GPS sensor logs cords in decimal format with a precision of 6 places
        // if this format is met there is no need to call convertDegMin later on when processing the file
        QRegularExpression decimalCoordinateFormatRegex("^[-+]?\\d{1,2}[.]\\d{6}$");
        QRegularExpressionMatch latFormatMatch = decimalCoordinateFormatRegex.match(cords[0]);
        QRegularExpressionMatch lonFormatMatch = decimalCoordinateFormatRegex.match(cords[1]);
        if (lonFormatMatch.hasMatch() && latFormatMatch.hasMatch()) {
          logFileGpsCordsInDecimalFormat = true;
        }
      }
    }
  }
}

void TelemetrySimulator::LogPlaybackController::calcLogFrequency()
{
  // examine up to 20 rows to determine log frequency in seconds
  // Skip the first entry which contains the file open time
  logFrequency = 25.5; // default value
  QDateTime lastTime;
  for (int i = 2; (i < 21) && (i < csvRecords.count()); i++)
  {
    QDateTime logTime = parseTransmittterTimestamp(csvRecords[i]);
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
    supportedCols.clear();
    recordIndex = 1;
    calcLogFrequency();
    checkGpsFormat();
  }
  ui->logFileLabel->setText(QFileInfo(logFileNameAndPath).fileName());
  // iterate through all known mappings and add those that are used
  QMapIterator<QString, CONVERT_TYPE> it(colToFuncMap);
  while (it.hasNext()) {
    it.next();
    addColumnHash(it.key(), it.value());
  }
  rewind();
  return;
}

void TelemetrySimulator::LogPlaybackController::addColumnHash(QString key, CONVERT_TYPE functionIndex)
{
  DATA_TO_FUNC_XREF dfx;
  if (columnNames.contains(key)) {
    dfx.functionIndex = functionIndex;
    dfx.dataIndex = columnNames.indexOf(key);
    supportedCols.append(dfx);
  }
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

double TelemetrySimulator::LogPlaybackController::convertFeetToMeters(QString input)
{
  double meters100 = input.toDouble() * 0.3048;
  return qFloor(meters100 + .005);
}

QString TelemetrySimulator::LogPlaybackController::convertGPSDate(QString input)
{
  QStringList dateTime = input.simplified().split(' ');
  if (dateTime.size() < 2) {
    return ""; // invalid format
  }
  QStringList dateParts = dateTime[0].split('-'); // input as yy-mm-dd
  if (dateParts.size() < 3) {
    return ""; // invalid format
  }
  // output is dd-MM-yyyy hh:mm:ss
  QString localDateString = dateParts[2] + "-" + dateParts[1] + "-20" + dateParts[0] + " " + dateTime[1];
  QString format("dd-MM-yyyy hh:mm:ss");
  QDateTime utcDate = QDateTime::fromString(localDateString, format).toTimeSpec(Qt::UTC);
  return utcDate.toString(format);
}

double TelemetrySimulator::LogPlaybackController::convertDegMin(QString input)
{
  double fInput = input.mid(0, input.length() - 1).toDouble();
  double degrees = qFloor(fInput / 100.0);
  double minutes = fInput - (degrees * 100);
  int32_t sign = ((input.endsWith('E')) || (input.endsWith('N'))) ? 1 : -1;
  return (degrees + (minutes / 60)) * sign;
}

QString TelemetrySimulator::LogPlaybackController::convertGPS(QString input)
{
  // input format is DDmm.mmmmH DDDmm.mmmmH (longitude latitude - degrees (2 places) minutes (2 places) decimal minutes (4 places))
  QStringList lonLat = input.simplified().split(' ');
  if (lonLat.count() < 2) {
    return ""; // invalid format
  }
  double lon, lat;
  if (logFileGpsCordsInDecimalFormat) {
    lat = lonLat[0].toDouble();
    lon = lonLat[1].toDouble();
  }
  else {
    lon = convertDegMin(lonLat[0]);
    lat = convertDegMin(lonLat[1]);
  }
  return QString::number(lat, 'f', 6) + ", " + QString::number(lon, 'f', 6);
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
  QDateTime transmitterTimestamp = parseTransmittterTimestamp(csvRecords[recordIndex]);
  QString format("yyyy-MM-dd hh:mm:ss.z");
  ui->positionLabel->setText("Row " + QString::number(recordIndex) + " of " + QString::number(csvRecords.count() - 1)
              + "\n" + transmitterTimestamp.toString(format));
  if (percentage < 0) { // did we step past a threshold?
    uint32_t posPercent = (recordIndex / (double)(csvRecords.count() - 1)) * 100;
    ui->positionIndicator->setValue(posPercent);
  }
}

double TelemetrySimulator::LogPlaybackController::convertFahrenheitToCelsius(QString input)
{
  return (input.toDouble() - 32.0) * 0.5556;
}

void TelemetrySimulator::LogPlaybackController::setUiDataValues()
{
  QStringList columnData = csvRecords[recordIndex].split(',');
  Q_FOREACH(DATA_TO_FUNC_XREF info, supportedCols) {
    if (info.dataIndex < columnData.size()) {
      switch (info.functionIndex) {
        /* For now just break this and move on
      case RXBT_V:
        ui->rxbt->setValue(columnData[info.dataIndex].toDouble());
        break;
      case RSSI:
        ui->Rssi->setValue(columnData[info.dataIndex].toDouble());
        break;
      case RAS:
        ui->Swr->setValue(columnData[info.dataIndex].toDouble());
        break;
      case A1:
        ui->A1->setValue(columnData[info.dataIndex].toDouble());
        break;
      case A2:
        ui->A2->setValue(columnData[info.dataIndex].toDouble());
        break;
      case A3:
        ui->A3->setValue(columnData[info.dataIndex].toDouble());
        break;
      case A4:
        ui->A4->setValue(columnData[info.dataIndex].toDouble());
        break;
      case T1_DEGC:
        ui->T1->setValue(columnData[info.dataIndex].toDouble());
        break;
      case T2_DEGC:
        ui->T2->setValue(columnData[info.dataIndex].toDouble());
        break;
      case T1_DEGF:
        ui->T1->setValue(convertFahrenheitToCelsius(columnData[info.dataIndex]));
        break;
      case T2_DEGF:
        ui->T2->setValue(convertFahrenheitToCelsius(columnData[info.dataIndex]));
        break;
      case RPM:
        ui->rpm->setValue(columnData[info.dataIndex].toDouble());
        break;
      case FUEL:
        ui->fuel->setValue(columnData[info.dataIndex].toDouble());
        break;
      case FUEL_QTY:
        ui->fuel_qty->setValue(columnData[info.dataIndex].toDouble());
        break;
      case VSPD_MS:
        ui->vspeed->setValue(columnData[info.dataIndex].toDouble());
        break;
      case VSPD_FS:
        ui->vspeed->setValue(columnData[info.dataIndex].toDouble() * 0.3048);
        break;
      case ALT_FEET:
        ui->valt->setValue(convertFeetToMeters(columnData[info.dataIndex]));
        break;
      case ALT_METERS:
        ui->valt->setValue(columnData[info.dataIndex].toDouble());
        break;
      case FASV:
        ui->vfas->setValue(columnData[info.dataIndex].toDouble());
        break;
      case FASC:
        ui->curr->setValue(columnData[info.dataIndex].toDouble());
        break;
      case CELS_GRE:
        ui->cell1->setValue(columnData[info.dataIndex].toDouble());
        break;
      case ASPD_KTS:
        ui->aspeed->setValue(columnData[info.dataIndex].toDouble() * 1.8520008892119);
        break;
      case ASPD_KMH:
        ui->aspeed->setValue(columnData[info.dataIndex].toDouble());
        break;
      case ASPD_MPH:
        ui->aspeed->setValue(columnData[info.dataIndex].toDouble() * 1.60934);
        break;
      case GALT_FEET:
        ui->gps_alt->setValue(convertFeetToMeters(columnData[info.dataIndex]));
        break;
      case GALT_METERS:
        ui->gps_alt->setValue(columnData[info.dataIndex].toDouble());
        break;
      case GSPD_KNTS:
        ui->gps_speed->setValue(columnData[info.dataIndex].toDouble() * 1.852);
        break;
      case GSPD_KMH:
        ui->gps_speed->setValue(columnData[info.dataIndex].toDouble());
        break;
      case GSPD_MPH:
        ui->gps_speed->setValue(columnData[info.dataIndex].toDouble() * 1.60934);
        break;
      case GHDG_DEG:
        ui->gps_course->setValue(columnData[info.dataIndex].toDouble());
        break;
      case GDATE:
        ui->gps_time->setText(convertGPSDate(columnData[info.dataIndex]));
        break;
      case G_LATLON:
        ui->gps_latlon->setText(convertGPS(columnData[info.dataIndex]));
        break;
      case ACCX:
        ui->accx->setValue(columnData[info.dataIndex].toDouble());
        break;
      case ACCY:
        ui->accy->setValue(columnData[info.dataIndex].toDouble());
        break;
      case ACCZ:
        ui->accz->setValue(columnData[info.dataIndex].toDouble());
        break;
        */
      }
    }
    else {
      // file is corrupt - shut down with open logs, or log format changed mid-day
    }
  }
}
