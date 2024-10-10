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

#include <QMessageBox>
#include <QFileDialog>

#include "appdata.h"
#include "telemetryprovidercrossfire.h"
#include "ui_telemetryprovidercrossfire.h"
#include "telem_data.h"

template<class t> t LIMIT(t mi, t x, t ma) { return std::min(std::max(mi, x), ma); }

TelemetryProviderCrossfire::TelemetryProviderCrossfire(QWidget * parent):
  QWidget(parent),
  ui(new Ui::TelemetryProviderCrossfire)
{
  ui->setupUi(this);

  // Set default values from UI definition into GPS
  gps.setLatLon(ui->input_gps->text());
  gps.setCourseDegrees(ui->input_hdg->value());
  gps.setSpeedKMH(ui->input_gspd->value());
  gps.setSatelliteCount(ui->input_sats->value());
  gps.setAltitude(ui->input_alt->value());

  connect(ui->input_hdg,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setCourseDegrees);
  connect(ui->input_gps,  &QLineEdit::textChanged,                              &gps, &SimulatedGPS::setLatLon);
  connect(ui->input_gspd, QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setSpeedKMH);
  connect(ui->input_alt,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setAltitude);
  connect(ui->input_sats, QOverload<int>::of(&QSpinBox::valueChanged),          &gps, &SimulatedGPS::setSatelliteCount);

  connect(&gps, &SimulatedGPS::positionChanged,      ui->input_gps, &QLineEdit::setText);
  connect(&gps, &SimulatedGPS::courseDegreesChanged, ui->input_hdg, QOverload<double>::of(&QDoubleSpinBox::setValue));

  // Create this once
  supportedLogItems.clear();
  supportedLogItems.insert("1RSS", "dB");
  supportedLogItems.insert("2RSS", "dB");
  supportedLogItems.insert("RQly", "%");
  supportedLogItems.insert("RSNR", "dB");
  supportedLogItems.insert("ANT", "");
  supportedLogItems.insert("RFMD", "");
  supportedLogItems.insert("TPWR", "mW");
  supportedLogItems.insert("TRSS", "dB");
  supportedLogItems.insert("TQly", "%");
  supportedLogItems.insert("TSNR", "dB");
  supportedLogItems.insert("RxBt", "V");
  supportedLogItems.insert("Curr", "A");
  supportedLogItems.insert("Capa", "mAh");
  supportedLogItems.insert("Bat%", "%");
  supportedLogItems.insert("GPS", "");
  supportedLogItems.insert("GSpd", "kmh");
  supportedLogItems.insert("Hdg", "°");
  supportedLogItems.insert("Alt", "m");
  supportedLogItems.insert("Sats", "");
  supportedLogItems.insert("Ptch", "rad");
  supportedLogItems.insert("Roll", "rad");
  supportedLogItems.insert("Yaw", "rad");
  supportedLogItems.insert("FM", "");
  supportedLogItems.insert("Alt", "m");
  supportedLogItems.insert("VSpd", "m/s");
}

TelemetryProviderCrossfire::~TelemetryProviderCrossfire()
{
  delete ui;
}

QHash<QString, QString> * TelemetryProviderCrossfire::getSupportedLogItems()
{
  return &supportedLogItems;
}

QString TelemetryProviderCrossfire::getLogfileIdentifier()
{
  return QString("TELEMETRY_DATA: CRSF");
}

void TelemetryProviderCrossfire::loadItemFromLog(QString item, QString value)
{
  if (item == "1RSS") ui->input_1rss->setValue(value.toInt());
  if (item == "2RSS") ui->input_2rss->setValue(value.toInt());
  if (item == "RQly") ui->input_rqly->setValue(value.toInt());
  if (item == "RSNR") ui->input_rsnr->setValue(value.toInt());
  if (item == "TRSS") ui->input_trss->setValue(value.toInt());
  if (item == "TPWR") ui->input_tpwr->setCurrentText(value + "mW");
  if (item == "RFMD") ui->input_rfmd->setValue(value.toInt());
  if (item == "ANT") ui->input_ant->setValue(value.toInt());
  if (item == "TQly") ui->input_tqly->setValue(value.toInt());
  if (item == "TSNR") ui->input_tsnr->setValue(value.toInt());
  if (item == "RRSP") ui->input_rrsp->setValue(value.toInt());
  if (item == "RPWR") ui->input_rpwr->setValue(value.toInt());
  if (item == "TRSP") ui->input_trsp->setValue(value.toInt());
  if (item == "RxBt") ui->input_rxbt->setValue(value.toDouble());
  if (item == "Curr") ui->input_curr->setValue(value.toDouble());
  if (item == "Capa") ui->input_capa->setValue(value.toInt());
  if (item == "Bat%") ui->input_batpercent->setValue(value.toInt());
  if (item == "GPS") ui->input_gps->setText(value);
  if (item == "GSpd") ui->input_gspd->setValue(value.toDouble());
  if (item == "Hdg") ui->input_hdg->setValue(value.toDouble());
  if (item == "Sats") ui->input_sats->setValue(value.toInt());
  if (item == "Ptch") ui->input_ptch->setValue(value.toDouble());
  if (item == "Roll") ui->input_roll->setValue(value.toDouble());
  if (item == "Yaw") ui->input_yaw->setValue(value.toDouble());
  if (item == "FM") ui->input_fm->setText(value.remove(QChar('"')));
  if (item == "VSpd") ui->input_vspd->setValue(value.toDouble());
  if (item == "Alt") ui->input_alt->setValue(value.toDouble());
}

void TelemetryProviderCrossfire::resetRssi()
{
  uint8_t buffer[CROSSFIRE_PACKET_SIZE] = {0};
  generateTelemetryLinkStatisticsFrame(buffer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  emit telemetryDataChanged(SIMU_TELEMETRY_PROTOCOL_CROSSFIRE, QByteArray((char *)buffer, CROSSFIRE_PACKET_SIZE));
}

void TelemetryProviderCrossfire::loadUiFromSimulator(SimulatorInterface * simulator)
{
  // Nothing to do for crossfire
}

uint8_t dropdownToTPWRMap[] = { 0, 1, 2, 8, 3, 7, 4, 5, 6, };

void TelemetryProviderCrossfire::generateTelemetryFrame(SimulatorInterface *simulator)
{
  static int item = 0;
  uint8_t buffer[CROSSFIRE_PACKET_SIZE] = {0};

  switch (item++) {
  case 0:
    // Always generate link stats
    if (true) {
      // just want a block here to put these variables in
      uint8_t rssi1 = ui->input_1rss->value();
      uint8_t rssi2 = ui->input_2rss->value();
      uint8_t rqly = ui->input_rqly->value();
      int8_t rsnr = ui->input_rsnr->value();
      uint8_t ant = ui->input_ant->value();
      uint8_t rfmd = ui->input_rfmd->value();
      uint8_t tpwr = dropdownToTPWRMap[ui->input_tpwr->currentIndex()];
      uint8_t trss = ui->input_trss->value();
      uint8_t tqly = ui->input_tqly->value();
      int8_t tsnr = ui->input_tsnr->value();

      generateTelemetryLinkStatisticsFrame(buffer, rssi1, rssi2, rqly, rsnr, ant, rfmd, tpwr, trss, tqly, tsnr);
    }
    break;
  case 1:
    if (ui->enabled_battery->isChecked()) {
      double voltage = ui->input_rxbt->value();
      double current = ui->input_curr->value();
      int32_t used_capacity = ui->input_capa->value();
      int8_t battery_percent = ui->input_batpercent->value();

      generateTelemetryBatterySensorFrame(buffer, voltage, current, used_capacity, battery_percent);
    }
    break;
  case 2:
    if (ui->enabled_gps->isChecked()) {
      generateTelemetryGPSFrame(buffer, gps.lat, gps.lon, gps.speedKMH, gps.courseDegrees, gps.altitude, gps.satellites);
    }
    break;
  case 3:
    if (ui->enabled_attitude->isChecked()) {
      double pitch = ui->input_ptch->value();
      double roll = ui->input_roll->value();
      double yaw = ui->input_yaw->value();

      generateTelemetryAttitudeFrame(buffer, pitch, roll, yaw);
    }
    break;
  case 4:
    if (ui->enabled_flightcontroller->isChecked()) {
      QString mode = ui->input_fm->text();

      generateTelemetryFlightModeFrame(buffer, mode);
    }
    break;
  case 5:
    if (ui->enabled_barometer->isChecked()) {
      double altitude = ui->input_alt->value();
      double vspeed = ui->input_vspd->value();

      generateTelemetryBarometerFrame(buffer, altitude, vspeed);
    }
    break;
  default:
    item = 0;
    return;
  }

  if (buffer[0]) {
    // If we put anything in the buffer, send it
    QByteArray ba((char *)buffer, CROSSFIRE_PACKET_SIZE);
    emit telemetryDataChanged(SIMU_TELEMETRY_PROTOCOL_CROSSFIRE, ba);
  }
}

void TelemetryProviderCrossfire::generateTelemetryLinkStatisticsFrame(uint8_t *packet, uint8_t rss1, uint8_t rss2, uint8_t rqly, int8_t rsnr, uint8_t ant, uint8_t rfmd, uint8_t tpwr, uint8_t trss, uint8_t tqly, int8_t tsnr)
{
  packet[0] = 0xc8; // SYNC
  packet[1] = 13; // LEN (11 + 2 CRC)
  packet[2] = 0x14; // CRSF_FRAMETYPE_LINK_STATISTICS
  packet[3] = rss1;
  packet[4] = rss2;
  packet[5] = rqly;
  packet[6] = rsnr;
  packet[7] = ant;
  packet[8] = rfmd;
  packet[9] = tpwr;
  packet[10] = trss;
  packet[11] = tqly;
  packet[12] = tsnr;
  // Don't bother calculating the CRC, the telemetry consumer doesn't check it
}

void TelemetryProviderCrossfire::generateTelemetryBatterySensorFrame(uint8_t *packet, double voltage, double current, int32_t used_capacity, int8_t battery_percent)
{
  int32_t tmp;

  packet[0] = 0xc8; // SYNC
  packet[1] = 10; // LEN (8 + 2 CRC)
  packet[2] = 0x08; // CRSF_FRAMETYPE_BATTERY_SENSOR

  tmp = (voltage * 10);
  packet[3] = (tmp & 0xff00) >> 8;
  packet[4] = tmp & 0xff;                                                         

  tmp = (current * 10);
  packet[5] = (tmp & 0xff00) >> 8;
  packet[6] = tmp & 0xff;

  packet[7] = (used_capacity & 0xff0000) >> 16;
  packet[8] = (used_capacity & 0xff00) >> 8;
  packet[9] = (used_capacity & 0xff);

  packet[10] = battery_percent;
  // Don't bother calculating the CRC, the telemetry consumer doesn't check it
}

void TelemetryProviderCrossfire::generateTelemetryAttitudeFrame(uint8_t *packet, double pitch, double roll, double yaw)
{
  int16_t tmp;

  packet[0] = 0xc8; // SYNC
  packet[1] = 8; // LEN (6 + 2 CRC)
  packet[2] = 0x1e; // CRSF_FRAMETYPE_ATTITUDE

  tmp = pitch * 10000;
  packet[3] = (tmp & 0xff00) >> 8;
  packet[4] = tmp & 0xff;

  tmp = roll * 10000;
  packet[5] = (tmp & 0xff00) >> 8;
  packet[6] = tmp & 0xff;

  tmp = yaw * 10000;
  packet[7] = (tmp & 0xff00) >> 8;
  packet[8] = tmp & 0xff;
  // Don't bother calculating the CRC, the telemetry consumer doesn't check it
}

void TelemetryProviderCrossfire::generateTelemetryFlightModeFrame(uint8_t *packet, const QString &mode)
{
  QByteArray snipped = mode.toUtf8().left(13);
  int len = snipped.length();

  packet[0] = 0xc8; // SYNC
  packet[1] = 3 + len; // LEN (string + 2 CRC)
  packet[2] = 0x21; // CRSF_FRAMETYPE_FLIGHT_MODE

  int i;
  for (i = 0; i < len; i++) {
    packet[3+i] = snipped[i];
  }
  // Null terminate
  packet[3+i] = 0;
  // Don't bother calculating the CRC, the telemetry consumer doesn't check it
}

uint16_t encodeAltitude(double altitude)
{
  uint16_t res;

  if (altitude > 2276.7) {
    res = altitude;
    res |= 0x8000; // set high bit to indicate it's integer meters
  } else {
    res = (altitude * 10) + 10000;
    res &= 0x7fff; // clear high bit to indicate it's decimeters above -1000m
  }

  return res;
}

void TelemetryProviderCrossfire::generateTelemetryBarometerFrame(uint8_t *packet, double altitude, double vspeed)
{
  uint16_t tmp;

  packet[0] = 0xc8; // SYNC
  packet[1] = 6; // LEN (4 + 2 CRC)
  packet[2] = 0x09; // CRSF_FRAMETYPE_BARO_ALTITUDE

  tmp = encodeAltitude(altitude);
  packet[3] = (tmp & 0xff00) >> 8;
  packet[4] = tmp & 0xff;

  tmp = vspeed * 100; // cm/sec
  packet[5] = (tmp & 0xff00) >> 8;
  packet[6] = tmp & 0xff;
  // Don't bother calculating the CRC, the telemetry consumer doesn't check it
}

void TelemetryProviderCrossfire::generateTelemetryGPSFrame(uint8_t *packet, double latitude, double longitude, double ground_speed, double ground_course, double altitude, int satellite_count)
{
  int32_t tmp;
  int16_t tmp2;
  uint16_t tmp3;

  packet[0] = 0xc8;
  packet[1] = 17; // LEN(15 + 2 CRC)
  packet[2] = 0x02; // CRSF_FRAMETYPE_GPS

  tmp = latitude * 10000000;
  packet[3] = (tmp & 0xff000000) >> 24;
  packet[4] = (tmp & 0xff0000) >> 16;
  packet[5] = (tmp & 0xff00) >> 8;
  packet[6] = tmp & 0xff;

  tmp = longitude * 10000000;
  packet[7] = (tmp & 0xff000000) >> 24;
  packet[8] = (tmp & 0xff0000) >> 16;
  packet[9] = (tmp & 0xff00) >> 8;
  packet[10] = tmp & 0xff;

  tmp2 = ground_speed * 10; // 10ths of a kph
  packet[11] = (tmp2 & 0xff00) >> 8;
  packet[12] = tmp2 & 0xff;

  tmp2 = ground_course;
  if (ground_course > 180) 
    tmp2 = (ground_course - 360); // -180..180, not 0-360
  tmp2 *= 100; // 100ths of a degree

  packet[13] = (tmp2 & 0xff00) >> 8;
  packet[14] = tmp2 & 0xff;
  
  tmp3 = altitude + 1000;
  packet[15] = (tmp3 & 0xff00) >> 8;
  packet[16] = tmp3 & 0xff;

  packet[17] = satellite_count & 0xff;
  // Don't bother calculating the CRC, the telemetry consumer doesn't check it
}

void TelemetryProviderCrossfire::on_button_gpsRunStop_clicked()
{
  if (gps.running) {
    gps.stop();
    ui->button_gpsRunStop->setText(tr("Run"));
  } else {
    gps.start();
    ui->button_gpsRunStop->setText(tr("Stop"));
  }
}

void TelemetryProviderCrossfire::on_button_saveTelemetryValues_clicked()
{
  QString fldr = g.backupDir().trimmed();
  if (fldr.isEmpty())
    fldr = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

  QString idFileNameAndPath = QFileDialog::getSaveFileName(this, tr("Save Telemetry"), fldr % "/telemetry.tlm", tr(".tlm Files (*.tlm)"));
  if (idFileNameAndPath.isEmpty())
    return;

  QFile file(idFileNameAndPath);
  if (!file.open(QIODevice::WriteOnly)){
    QMessageBox::critical(this, CPN_STR_APP_NAME, tr("Unable to open file for writing.\n%1").arg(file.errorString()));
    return;
  }
  QTextStream out(&file);

  out << getLogfileIdentifier();
  out << "\r\n";
  out << ui->input_1rss->text();
  out << "\r\n";
  out << ui->input_2rss->text();
  out << "\r\n";
  out << ui->input_rqly->text();
  out << "\r\n";
  out << ui->input_rsnr->text();
  out << "\r\n";
  out << ui->input_trss->text();
  out << "\r\n";
  out << ui->input_tpwr->currentText();
  out << "\r\n";
  out << ui->input_rfmd->text();
  out << "\r\n";
  out << ui->input_ant->text();
  out << "\r\n";
  out << ui->input_tqly->text();
  out << "\r\n";
  out << ui->input_tsnr->text();
  out << "\r\n";
  out << ui->input_rrsp->text();
  out << "\r\n";
  out << ui->input_rpwr->text();
  out << "\r\n";
  out << ui->input_trsp->text();
  out << "\r\n";
  out << ui->enabled_battery->isChecked();
  out << "\r\n";
  out << ui->input_rxbt->text();
  out << "\r\n";
  out << ui->input_curr->text();
  out << "\r\n";
  out << ui->input_capa->text();
  out << "\r\n";
  out << ui->input_batpercent->text();
  out << "\r\n";
  out << ui->enabled_gps->isChecked();
  out << "\r\n";
  out << ui->input_gps->text();
  out << "\r\n";
  out << ui->input_gspd->text();
  out << "\r\n";
  out << ui->input_hdg->text();
  out << "\r\n";
  out << ui->input_sats->text();
  out << "\r\n";
  out << ui->enabled_attitude->isChecked();
  out << "\r\n";
  out << ui->input_ptch->text();
  out << "\r\n";
  out << ui->input_roll->text();
  out << "\r\n";
  out << ui->input_yaw->text();
  out << "\r\n";
  out << ui->enabled_flightcontroller->isChecked();
  out << "\r\n";
  out << ui->input_fm->text();
  out << "\r\n";
  out << ui->enabled_barometer->isChecked();
  out << "\r\n";
  out << ui->input_vspd->text();
  out << "\r\n";
  out << ui->input_alt->text();
  out << "\r\n";
  file.flush();
  file.close();
}

void TelemetryProviderCrossfire::on_button_loadTelemetryValues_clicked()
{
  QString fldr = g.backupDir().trimmed();
  if (fldr.isEmpty())
    fldr = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);

  QString idFileNameAndPath = QFileDialog::getOpenFileName(this, tr("Open Telemetry File"), fldr % "/telemetry.tlm", tr(".tlm Files (*.tlm)"));
  if (idFileNameAndPath.isEmpty())
    return;

  QFile file(idFileNameAndPath);

  if (!file.open(QIODevice::ReadOnly)){
    QMessageBox::critical(this, CPN_STR_APP_NAME, tr("Unable to open file for reading.\n%1").arg(file.errorString()));
    return;
  }

  QTextStream in(&file);

  QString inputText;
  double inputDouble;
  int inputInt;

  inputText = in.readLine();
  if (inputText != getLogfileIdentifier()) {
    QMessageBox::critical(this, CPN_STR_APP_NAME, tr("Not a CRSF telemetry values file."));
    return;
  }

  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_1rss->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_2rss->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_rqly->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_rsnr->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_trss->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_tpwr->setCurrentText(inputText);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_rfmd->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_ant->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_tqly->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_tsnr->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_rrsp->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_rpwr->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_trsp->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_battery->setChecked(inputInt);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_rxbt->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_curr->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_capa->setValue(inputDouble);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_batpercent->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_gps->setChecked(inputInt);
  inputText = in.readLine(); ui->input_gps->setText(inputText);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_gspd->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_hdg->setValue(inputDouble);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_sats->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_attitude->setChecked(inputInt);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_ptch->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_roll->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_yaw->setValue(inputDouble);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_flightcontroller->setChecked(inputInt);
  inputText = in.readLine(); ui->input_fm->setText(inputText);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_barometer->setChecked(inputInt);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_vspd->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_alt->setValue(inputDouble);

  file.close();
}
