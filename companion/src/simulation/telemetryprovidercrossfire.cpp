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

  gpsTimer.setInterval(250);
  connect(&gpsTimer, &QTimer::timeout, this, &TelemetryProviderCrossfire::updateGps);
}

TelemetryProviderCrossfire::~TelemetryProviderCrossfire()
{
  delete ui;
}


void TelemetryProviderCrossfire::resetRssi()
{
  if (!(ui && ui->input_1rss && ui->input_2rss))
    return;

  uint8_t buffer[CROSSFIRE_PACKET_SIZE] = {0};
  generateTelemetryLinkStatisticsFrame(buffer, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
  emit telemetryDataChanged(SIMU_TELEMETRY_PROTOCOL_CROSSFIRE, QByteArray((char *)buffer, CROSSFIRE_PACKET_SIZE));
}

void TelemetryProviderCrossfire::updateGps()
{
  int a = ui->input_gps->text().contains(",");
  if (!a) {
    QMessageBox::information(this, tr("Bad GPS Format"), tr("Must be decimal latitude,longitude"));
    ui->input_gps->setText("000.00000000,000.00000000");
    //ui->button_gps_run->click();
  }
  else
  {
    QStringList gpsLatLon = (ui->input_gps->text()).split(",");

    double b2 = gpsLatLon[0].toDouble();
    double c2 = gpsLatLon[1].toDouble();
    double d3 = ui->input_gspd->value() / 14400;
    double f3 = ui->input_hdg->value();
    double j2 = 6378.1;
    double b3 = qRadiansToDegrees(qAsin( qSin(qDegreesToRadians(b2))*qCos(d3/j2) + qCos(qDegreesToRadians(b2))*qSin(d3/j2)*qCos(qDegreesToRadians(f3))));
    double bb3 = b3;
    if (bb3 < 0) {
      bb3 = bb3 * -1;
    }
    if (bb3 > 89.99) {
      f3 = f3 + 180;
      if (f3 > 360) {
        f3 = f3 - 360;
      }
      ui->input_hdg->setValue(f3);
    }
    double c3 = qRadiansToDegrees(qDegreesToRadians(c2) + qAtan2(qSin(qDegreesToRadians(f3))*qSin(d3/j2)*qCos(qDegreesToRadians(b2)),qCos(d3/j2)-qSin(qDegreesToRadians(b2))*qSin(qDegreesToRadians(b3))));
    if (c3 > 180) {
      c3 = c3 - 360;
    }
    if (c3 < -180) {
      c3 = c3 + 360;
    }
    QString lats = QString::number(b3, 'f', 8);
    QString lons = QString::number(c3, 'f', 8);
    QString qs = lats + "," + lons;
    ui->input_gps->setText(qs);
  }
}

void TelemetryProviderCrossfire::loadUiFromSimulator(SimulatorInterface * simulator)
{
  // Do nothing.
}

void TelemetryProviderCrossfire::generateTelemetryFrame(SimulatorInterface *simulator)
{
  static int item = 0;
  uint8_t buffer[CROSSFIRE_PACKET_SIZE] = {0};
  static GPSEmulator *gps = new GPSEmulator();

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
      uint8_t tpwr = 2; // TODO: make this a dropdown and find the index
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
      // generateTelemetryGPSFrame(buffer);
    }
    break;
  case 3:
    if (ui->enabled_attitude->isChecked()) {
      double pitch = ui->input_pitch->value();
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
    //qDebug("%02X %02X %02X %02X %02X %02X %02X %02X %02X", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
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
}

void TelemetryProviderCrossfire::generateTelemetryBarometerFrame(uint8_t *packet, double altitude, double vspeed)
{
  uint16_t tmp;

  packet[0] = 0xc8; // SYNC
  packet[1] = 6; // LEN (4 + 2 CRC)
  packet[2] = 0x09; // CRSF_FRAMETYPE_BARO_ALTITUDE

  if (altitude > 2276.7) {
    tmp = altitude;
    tmp |= 0x8000; // set high bit to indicate it's integer meters
  } else {
    tmp = (altitude * 10) + 10000;
    tmp &= 0x7fff; // clear high bit to indicate it's decimeters above -1000m
  }
  packet[3] = (tmp & 0xff00) >> 8;
  packet[4] = tmp & 0xff;

  tmp = vspeed * 100; // cm/sec
  packet[5] = (tmp & 0xff00) >> 8;
  packet[6] = tmp & 0xff;
}

TelemetryProviderCrossfire::GPSEmulator::GPSEmulator()
{
  lat = 0;
  lon = 0;
  dt = QDateTime::currentDateTime();
  sendLat = true;
  sendDate = true;
}


uint32_t TelemetryProviderCrossfire::GPSEmulator::encodeLatLon(double latLon, bool isLat)
{
  uint32_t data = (uint32_t)((latLon < 0 ? -latLon : latLon) * 60 * 10000) & 0x3FFFFFFF;
  if (isLat == false) {
    data |= 0x80000000;
  }
  if (latLon < 0) {
    data |= 0x40000000;
  }
  return data;
}

uint32_t TelemetryProviderCrossfire::GPSEmulator::encodeDateTime(uint8_t yearOrHour, uint8_t monthOrMinute, uint8_t dayOrSecond, bool isDate)
{
  uint32_t data = yearOrHour;
  data <<= 8;
  data |= monthOrMinute;
  data <<= 8;
  data |= dayOrSecond;
  data <<= 8;
  if (isDate == true) {
    data |= 0xFF;
  }
  return data;
}

void TelemetryProviderCrossfire::GPSEmulator::setGPSDateTime(QString dateTime)
{
  dt = QDateTime::currentDateTime().toTimeSpec(Qt::UTC); // default to current systemtime
  if (!dateTime.startsWith('*')) {
    QString format("dd-MM-yyyy hh:mm:ss");
    dt = QDateTime::fromString(dateTime, format);
  }
}

void TelemetryProviderCrossfire::GPSEmulator::setGPSLatLon(QString latLon)
{
  QStringList coords = latLon.split(",");
  lat = 0.0;
  lon = 0.0;
  if (coords.length() > 1)
  {
    lat = coords[0].toDouble();
    lon = coords[1].toDouble();
  }
}

void TelemetryProviderCrossfire::GPSEmulator::setGPSCourse(double course)
{
  this->course = course;
}

void TelemetryProviderCrossfire::GPSEmulator::setGPSSpeedKMH(double speedKMH)
{
  this->speedKNTS = speedKMH * 0.539957;
}

void TelemetryProviderCrossfire::GPSEmulator::setGPSAltitude(double altitude)
{
  this->altitude = altitude;
}



void TelemetryProviderCrossfire::on_GPSpushButton_clicked()
{
  /*
  if (ui->GPSpushButton->text() == tr("Run")) {
    ui->GPSpushButton->setText(tr("Stop"));
    gpsTimer.start();
  }
  else
  {
    ui->GPSpushButton->setText(tr("Run"));
    gpsTimer.stop();
  }
  */
}

void TelemetryProviderCrossfire::on_gps_course_valueChanged(double arg1)
{
  /*
  if (ui->gps_course->value() > 360) {
    ui->gps_course->setValue(1);
  }
  if (ui->gps_course->value() < 1) {
    ui->gps_course->setValue(360);
  }
  */
}

