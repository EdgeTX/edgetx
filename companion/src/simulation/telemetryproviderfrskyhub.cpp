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
#include "telemetryproviderfrskyhub.h"
#include "ui_telemetryproviderfrskyhub.h"
#include "telem_data.h"
#include "radio/src/telemetry/frsky_defs.h"

template<class t> t LIMIT(t mi, t x, t ma) { return std::min(std::max(mi, x), ma); }

TelemetryProviderFrSkyHub::TelemetryProviderFrSkyHub(QWidget * parent):
  QWidget(parent),
  ui(new Ui::TelemetryProviderFrSkyHub)
{
  ui->setupUi(this);

  // Set default values from UI definition into GPS
  gps.setLatLon(ui->input_gps->text());
  gps.setCourseDegrees(ui->input_hdg->value());
  gps.setSpeedKMH(ui->input_gspd->value());
  gps.setAltitude(ui->input_galt->value());
  gps.setDateTime(ui->input_date->dateTime());

  connect(ui->input_hdg,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setCourseDegrees);
  connect(ui->input_gps,  &QLineEdit::textChanged,                              &gps, &SimulatedGPS::setLatLon);
  connect(ui->input_gspd, QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setSpeedKMH);
  connect(ui->input_galt, QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setAltitude);

  connect(&gps, &SimulatedGPS::positionChanged,                           ui->input_gps,  &QLineEdit::setText);
  connect(&gps, &SimulatedGPS::courseDegreesChanged,                      ui->input_hdg,  QOverload<double>::of(&QDoubleSpinBox::setValue));
  connect(&gps, QOverload<QDateTime>::of(&SimulatedGPS::dateTimeChanged), ui->input_date, &QDateTimeEdit::setDateTime);

  // Create this once
  supportedLogItems.clear();
  supportedLogItems.insert("RSSI", "");
  supportedLogItems.insert("A1", "V");
  supportedLogItems.insert("A2", "V");
  supportedLogItems.insert("TRSS", "dB");
  supportedLogItems.insert("RQly", "");
  supportedLogItems.insert("TQly", "");
  supportedLogItems.insert("RPM", "rpm");
  supportedLogItems.insert("Fuel", "%");
  supportedLogItems.insert("Tmp1", "°C");
  supportedLogItems.insert("Tmp2", "°C");
  supportedLogItems.insert("VFAS", "V");
  supportedLogItems.insert("Curr", "A");
  supportedLogItems.insert("VSpd", "m/s");
  supportedLogItems.insert("Alt", "m");
  supportedLogItems.insert("GPS", "");
  supportedLogItems.insert("GSpd", "kts");
  supportedLogItems.insert("Hdg", "°");
  supportedLogItems.insert("GAlt", "m");
  supportedLogItems.insert("Date", "");
  supportedLogItems.insert("AccX", "g");
  supportedLogItems.insert("AccY", "g");
  supportedLogItems.insert("AccZ", "g");

}

TelemetryProviderFrSkyHub::~TelemetryProviderFrSkyHub()
{
  delete ui;
}

QHash<QString, QString> * TelemetryProviderFrSkyHub::getSupportedLogItems()
{
  return &supportedLogItems;
}

QString TelemetryProviderFrSkyHub::getLogfileIdentifier()
{
  return QString("TELEMETRY_DATA: FrSky Hub");
}

void TelemetryProviderFrSkyHub::loadItemFromLog(QString item, QString value)
{
  if (item == "RSSI") ui->input_rssi->setValue(value.toInt());
  if (item == "A1") ui->input_a1->setValue(value.toDouble());
  if (item == "A2") ui->input_a2->setValue(value.toDouble());
  if (item == "TRSS") ui->input_trss->setValue(value.toInt());
  if (item == "RQly") ui->input_rqly->setValue(value.toInt());
  if (item == "TQly") ui->input_tqly->setValue(value.toInt());
  if (item == "RPM") ui->input_rpm->setValue(value.toInt());
  if (item == "Fuel") ui->input_fuel->setValue(value.toInt());
  if (item == "Tmp1") ui->input_tmp1->setValue(value.toInt());
  if (item == "Tmp2") ui->input_tmp2->setValue(value.toInt());
  if (item == "VFAS") ui->input_vfas->setValue(value.toDouble());
  if (item == "Curr") ui->input_curr->setValue(value.toDouble());
  if (item == "VSpd") ui->input_vspd->setValue(value.toDouble());
  if (item == "Alt") ui->input_alt->setValue(value.toDouble());
  if (item == "GPS") ui->input_gps->setText(value);
  if (item == "GSpd") ui->input_gspd->setValue(value.toDouble());
  if (item == "Hdg") ui->input_hdg->setValue(value.toInt());
  if (item == "GAlt") ui->input_galt->setValue(value.toDouble());
  if (item == "Date") ui->input_date->setDateTime(QDateTime::fromString(value));
  if (item == "AccX") ui->input_accx->setValue(value.toDouble());
  if (item == "AccY") ui->input_accy->setValue(value.toDouble());
  if (item == "AccZ") ui->input_accz->setValue(value.toDouble());
}

void TelemetryProviderFrSkyHub::resetRssi()
{
  sendLinkFrame(0, 0, 0, 0, 0, 0);
}

void TelemetryProviderFrSkyHub::loadUiFromSimulator(SimulatorInterface * simulator)
{
  // Nothing to do for FrSky Hub
}

void TelemetryProviderFrSkyHub::generateTelemetryFrame(SimulatorInterface *simulator)
{
  static int item = 0;

  switch (item++) {
  case 0:
    // Always generate link stats
    if (true) {
      // just want a block here to put these variables in
      int rssi = ui->input_rssi->value();
      double a1 = ui->input_a1->value();
      double a2 = ui->input_a2->value();
      int trss = ui->input_trss->value();
      int rqly = ui->input_rqly->value();
      int tqly = ui->input_tqly->value();

      sendLinkFrame(rssi, a1, a2, trss, rqly, tqly);
    }
    break;
  case 1:
    if (ui->enabled_rpm->isChecked()) {
      sendRPMFrame(ui->input_rpm->value());
    }
  case 2:
    if (ui->enabled_fuel->isChecked()) {
      sendFuelFrame(ui->input_fuel->value());
    }
  case 3:
    if (ui->enabled_temps->isChecked()) {
      sendTempsFrame(ui->input_tmp1->value(), ui->input_tmp2->value());
    }
  case 4:
    if (ui->enabled_battery->isChecked()) {
      sendBatteryFrame(ui->input_vfas->value(), ui->input_curr->value());
    }
  case 5:
    if (ui->enabled_baro->isChecked()) {
      sendBaroFrame(ui->input_vspd->value(), ui->input_alt->value());
    }
  case 6:
    if (ui->enabled_accel->isChecked()) {
      sendAccelFrame(ui->input_accx->value(), ui->input_accy->value(), ui->input_accz->value());
    }
  case 7:
    if (ui->enabled_gps->isChecked()) {
      sendGPSFrame(gps);
    }
  default:
    item = 0;
    return;
  }
}

void TelemetryProviderFrSkyHub::sendLongLinkFrame(int rssi, double a1, double a2, int trss, int rqly, int tqly)
{
  // A1 & A2 are an unsigned byte measuring 0V (0) to 13.2V (255)
  uint8_t a1_encoded = static_cast<uint8_t>((a1 / 13.2) * 255);
  uint8_t a2_encoded = static_cast<uint8_t>((a2 / 13.2) * 255);
  uint8_t rssi_encoded = static_cast<uint8_t>(rssi);
  uint8_t trss_encoded = static_cast<uint8_t>(trss);
  uint8_t rqly_encoded = static_cast<uint8_t>(rqly);
  uint8_t tqly_encoded = static_cast<uint8_t>(tqly);

  uint8_t packet[7] = { 0xfe, a1_encoded, a2_encoded, rssi_encoded, trss_encoded, rqly_encoded, tqly_encoded};
  QByteArray ba((char *)packet, 7);
  emit telemetryDataChanged(SIMU_TELEMETRY_PROTOCOL_FRSKY_HUB, ba);  
}

void TelemetryProviderFrSkyHub::sendShortLinkFrame(int rssi, double a1, double a2)
{
  // A1 & A2 are an unsigned byte measuring 0V (0) to 13.2V (255)
  uint8_t a1_encoded = static_cast<uint8_t>((a1 / 13.2) * 255);
  uint8_t a2_encoded = static_cast<uint8_t>((a2 / 13.2) * 255);
  uint8_t rssi_encoded = static_cast<uint8_t>(rssi);

  uint8_t packet[4] = { 0xfe, a1_encoded, a2_encoded, rssi_encoded};
  QByteArray ba((char *)packet, 4);
  emit telemetryDataChanged(SIMU_TELEMETRY_PROTOCOL_FRSKY_HUB, ba);  
}

void TelemetryProviderFrSkyHub::sendLinkFrame(int rssi, double a1, double a2, int trss, int rqly, int tqly)
{
  if (ui->enabled_multi->isChecked())
    sendLongLinkFrame(rssi, a1, a2, trss << 1, rqly, tqly);
  else
    sendShortLinkFrame(rssi, a1, a2);
}

void TelemetryProviderFrSkyHub::sendSensorPacket(uint8_t id, uint16_t value)
{
  uint8_t packet[3] = { id, static_cast<uint8_t>(value & 0xff), static_cast<uint8_t>((value >> 8) & 0xff) };
  QByteArray ba((char *)packet, 3);
  emit telemetryDataChanged(SIMU_TELEMETRY_PROTOCOL_FRSKY_HUB_OOB, ba);  
}

void TelemetryProviderFrSkyHub::sendRPMFrame(int rpm)
{
  uint16_t rpm_60 = rpm / 60;
  sendSensorPacket(RPM_ID, rpm_60);
}

void TelemetryProviderFrSkyHub::sendFuelFrame(int fuelpercent)
{
  sendSensorPacket(FUEL_ID, fuelpercent);
}

void TelemetryProviderFrSkyHub::sendTempsFrame(int temp1, int temp2)
{
  sendSensorPacket(TEMP1_ID, temp1);
  sendSensorPacket(TEMP2_ID, temp2);
}

void TelemetryProviderFrSkyHub::sendBatteryFrame(double vfas, double curr)
{
  /*
  // There are a couple of voltage sensors that could be used in a
  // FrSky Hub setup. We will use the high precision one as that's
  // easier, but if you _really_ want to simulate VFAS from the sensor
  // that reports in units of 0.52381 volts, go ahead and use this
  // snippet instead.

  double scaled_vfas = vfas * (110.0 / 210.0);
  int16_t bp = scaled_vfas;
  int16_t ap = (scaled_vfas - bp) * 10;

  sendSensorPacket(VOLTS_BP_ID, bp);
  sendSensorPacket(VOLTS_AP_ID, ap);
  */

  sendSensorPacket(VFAS_ID, 2000 + (vfas * 100));
  sendSensorPacket(CURRENT_ID, curr * 10);
}

void TelemetryProviderFrSkyHub::sendBaroFrame(double vspd, double alt)
{
  sendSensorPacket(VARIO_ID, vspd * 100);
  sendSensorPacket(BARO_ALT_BP_ID, alt);
  sendSensorPacket(BARO_ALT_AP_ID, (alt - floor(alt)) * 10);
}

void TelemetryProviderFrSkyHub::sendAccelFrame(double accx, double accy, double accz)
{
  sendSensorPacket(ACCEL_X_ID, accx * 1000);
  sendSensorPacket(ACCEL_Y_ID, accy * 1000);
  sendSensorPacket(ACCEL_Z_ID, accz * 1000);
}

void TelemetryProviderFrSkyHub::sendGPSFrame(SimulatedGPS & gps)
{
  double speedKnots = gps.speedKMH * 0.539957;
  uint16_t bp, ap;

  uint16_t degrees;
  double decimal_minutes;

  degrees = floor(abs(gps.lat));
  decimal_minutes = (abs(gps.lat) - degrees) * 60;
  bp = degrees * 100 + floor(decimal_minutes);
  ap = (decimal_minutes - floor(decimal_minutes)) * 10000;

  // MUST send these in BP, AP, NS order
  sendSensorPacket(GPS_LAT_BP_ID, bp);
  sendSensorPacket(GPS_LAT_AP_ID, ap);
  sendSensorPacket(GPS_LAT_NS_ID, gps.lat < 0 ? 'S' : 'N');

  degrees = floor(abs(gps.lon));
  decimal_minutes = (abs(gps.lon) - degrees) * 60;
  bp = degrees * 100 + floor(decimal_minutes);
  ap = (decimal_minutes - floor(decimal_minutes)) * 10000;

  // MUST send these in BP, AP, EW order
  sendSensorPacket(GPS_LONG_BP_ID, bp);
  sendSensorPacket(GPS_LONG_AP_ID, ap);
  sendSensorPacket(GPS_LONG_EW_ID, gps.lon < 0 ? 'W' : 'E');

  // The _after decimal point_ packets are ignored by the telemetry parser, so don't bother sending them.
  sendSensorPacket(GPS_ALT_BP_ID, gps.altitude);
  sendSensorPacket(GPS_SPEED_BP_ID, speedKnots);
  sendSensorPacket(GPS_COURS_BP_ID, gps.courseDegrees);

  // Date and Time
  QDate date = gps.dt.date();
  QTime time = gps.dt.time();
  sendSensorPacket(GPS_YEAR_ID, date.year());
  sendSensorPacket(GPS_DAY_MONTH_ID, (date.month() << 8) + date.day());
  sendSensorPacket(GPS_HOUR_MIN_ID, (time.minute() << 8) + time.hour());
  sendSensorPacket(GPS_SEC_ID, time.second());
}

void TelemetryProviderFrSkyHub::on_button_gpsRunStop_clicked()
{
  if (gps.running) {
    gps.stop();
    ui->button_gpsRunStop->setText(tr("Run"));
  } else {
    gps.start();
    ui->button_gpsRunStop->setText(tr("Stop"));
  }
}

void TelemetryProviderFrSkyHub::on_button_saveTelemetryValues_clicked()
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

  out << getLogfileIdentifier() << "\r\n";
  out << ui->input_rssi->text() << "\r\n";
  out << ui->input_a1->text() << "\r\n";
  out << ui->input_a2->text() << "\r\n";
  out << ui->input_trss->text() << "\r\n";
  out << ui->input_rqly->text() << "\r\n";
  out << ui->input_tqly->text() << "\r\n";
  out << ui->input_rpm->text() << "\r\n";
  out << ui->input_fuel->text() << "\r\n";
  out << ui->input_tmp1->text() << "\r\n";
  out << ui->input_tmp2->text() << "\r\n";
  out << ui->input_vfas->text() << "\r\n";
  out << ui->input_curr->text() << "\r\n";
  out << ui->input_vspd->text() << "\r\n";
  out << ui->input_alt->text() << "\r\n";
  out << ui->input_gps->text() << "\r\n";
  out << ui->input_gspd->text() << "\r\n";
  out << ui->input_hdg->text() << "\r\n";
  out << ui->input_galt->text() << "\r\n";
  out << ui->input_date->dateTime().toString(Qt::ISODate) << "\r\n";
  out << ui->input_accx->text() << "\r\n";
  out << ui->input_accy->text() << "\r\n";
  out << ui->input_accz->text() << "\r\n";

  out << ui->enabled_multi->isChecked() << "\r\n";
  out << ui->enabled_rpm->isChecked() << "\r\n";
  out << ui->enabled_fuel->isChecked() << "\r\n";
  out << ui->enabled_temps->isChecked() << "\r\n";
  out << ui->enabled_battery->isChecked() << "\r\n";
  out << ui->enabled_baro->isChecked() << "\r\n";
  out << ui->enabled_gps->isChecked() << "\r\n";
  out << ui->enabled_accel->isChecked() << "\r\n";

  file.flush();
  file.close();
}

void TelemetryProviderFrSkyHub::on_button_loadTelemetryValues_clicked()
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
  QDateTime inputDatetime;
  double inputDouble;
  int inputInt;

  inputText = in.readLine();
  if (inputText != getLogfileIdentifier()) {
    QMessageBox::critical(this, CPN_STR_APP_NAME, tr("Not a FrSky Hub telemetry values file."));
    return;
  }

  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_rssi->setValue(inputInt);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_a1->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_a2->setValue(inputDouble);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_trss->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_rqly->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_tqly->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_rpm->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_fuel->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_tmp1->setValue(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->input_tmp2->setValue(inputInt);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_vfas->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_curr->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_vspd->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_alt->setValue(inputDouble);
  inputText = in.readLine(); ui->input_gps->setText(inputText);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_gspd->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_hdg->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_galt->setValue(inputDouble);
  inputText = in.readLine(); inputDatetime = QDateTime::fromString(inputText, Qt::ISODate); ui->input_date->setDateTime(inputDatetime);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_accx->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_accy->setValue(inputDouble);
  inputText = in.readLine(); inputDouble = inputText.toDouble(); ui->input_accz->setValue(inputDouble);

  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_multi->setChecked(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_rpm->setChecked(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_fuel->setChecked(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_temps->setChecked(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_battery->setChecked(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_baro->setChecked(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_gps->setChecked(inputInt);
  inputText = in.readLine(); inputInt = inputText.toInt(); ui->enabled_accel->setChecked(inputInt);

  file.close();
}
