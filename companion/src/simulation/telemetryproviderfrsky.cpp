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
#include "telemetryproviderfrsky.h"
#include "ui_telemetryproviderfrsky.h"
#include "telem_data.h"
#include "radio/src/telemetry/frsky_defs.h"

template<class t> t LIMIT(t mi, t x, t ma) { return std::min(std::max(mi, x), ma); }

TelemetryProviderFrSky::TelemetryProviderFrSky(QWidget * parent):
  QWidget(parent),
  ui(new Ui::TelemetryProviderFrSky)
{
  sendLat = true;
  sendDate = true;

  ui->setupUi(this);

  ui->A1->setSpecialValueText(" ");
  ui->A2->setSpecialValueText(" ");
  ui->A3->setSpecialValueText(" ");
  ui->A4->setSpecialValueText(" ");
  ui->rpm->setSpecialValueText(" ");
  ui->fuel->setSpecialValueText(" ");

  ui->rxbt_ratio->setEnabled(false);
  ui->A1_ratio->setEnabled(false);
  ui->A2_ratio->setEnabled(false);

  gps.setLatLon(ui->gps_latlon->text());
  gps.setCourseDegrees(ui->gps_course->value());
  gps.setSpeedKMH(ui->gps_speed->value());
  gps.setAltitude(ui->gps_alt->value());
  setGPSDateTime(ui->gps_time->text()); // Have to convert from a non-ISO format

  connect(ui->gps_latlon, &QLineEdit::textChanged,                              &gps, &SimulatedGPS::setLatLon);
  connect(ui->gps_course, QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setCourseDegrees);
  connect(ui->gps_speed,  QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setSpeedKMH);
  connect(ui->gps_alt,    QOverload<double>::of(&QDoubleSpinBox::valueChanged), &gps, &SimulatedGPS::setAltitude);
  connect(ui->gps_time,   &QLineEdit::textChanged,                              this, &TelemetryProviderFrSky::setGPSDateTime);

  connect(&gps, &SimulatedGPS::positionChanged,                           ui->gps_latlon, &QLineEdit::setText);
  connect(&gps, &SimulatedGPS::courseDegreesChanged,                      ui->gps_course, &QDoubleSpinBox::setValue);
  connect(&gps, QOverload<QDateTime>::of(&SimulatedGPS::dateTimeChanged), this,           &TelemetryProviderFrSky::setDateTimeFromGPS);

  // Create this once
  supportedLogItems.clear();
  supportedLogItems.insert("RxBt", "V");
  supportedLogItems.insert("RSSI", "dB");
  supportedLogItems.insert("A1", "V");
  supportedLogItems.insert("A2", "V");
  supportedLogItems.insert("A3", "V");
  supportedLogItems.insert("A4", "V");
  supportedLogItems.insert("Tmp1", "°C");
  supportedLogItems.insert("Tmp2", "°C");
  supportedLogItems.insert("RPM", "rpm");
  supportedLogItems.insert("Fuel", "%");
  supportedLogItems.insert("Fuel", "ml");
  supportedLogItems.insert("VSpd", "m/s");
  supportedLogItems.insert("Alt", "m");
  supportedLogItems.insert("VFAS", "V");
  supportedLogItems.insert("Curr", "A");
  supportedLogItems.insert("Cels", "V");
  supportedLogItems.insert("ASpd", "kmh");
  supportedLogItems.insert("GAlt", "m");
  supportedLogItems.insert("GSpd", "kmh");
  supportedLogItems.insert("Hdg", "°");
  supportedLogItems.insert("Date", "");
  supportedLogItems.insert("GPS", "");
  supportedLogItems.insert("AccX", "g");
  supportedLogItems.insert("AccY", "g");
  supportedLogItems.insert("AccZ", "g");
}

TelemetryProviderFrSky::~TelemetryProviderFrSky()
{
  delete ui;
}


void TelemetryProviderFrSky::resetRssi()
{
  if (!(ui && ui->rssi_inst))
    return;

  bool ok = false;

  const int id = ui->rssi_inst->text().toInt(&ok, 0);
  if (!ok)
    return;

  uint8_t buffer[FRSKY_SPORT_PACKET_SIZE] = {0};
  generateSportPacket(buffer, id, DATA_FRAME, RSSI_ID, 0);
  emit telemetryDataChanged(SIMU_TELEMETRY_PROTOCOL_FRSKY_SPORT, QByteArray((char *)buffer, FRSKY_SPORT_PACKET_SIZE));
}

#define SET_INSTANCE(control, id, def)  ui->control->setText(QString::number(simulator->getSensorInstance(id, ((def) & 0x1F))))

void TelemetryProviderFrSky::loadUiFromSimulator(SimulatorInterface * simulator)
{
  SET_INSTANCE(rxbt_inst,     BATT_ID,                0);
  SET_INSTANCE(rssi_inst,     RSSI_ID,                24);
  SET_INSTANCE(swr_inst,      RAS_ID,                 24);
  SET_INSTANCE(a1_inst,       ADC1_ID,                0);
  SET_INSTANCE(a2_inst,       ADC2_ID,                0);
  SET_INSTANCE(a3_inst,       A3_FIRST_ID,            0);
  SET_INSTANCE(a4_inst,       A4_FIRST_ID,            0);
  SET_INSTANCE(t1_inst,       T1_FIRST_ID,            0);
  SET_INSTANCE(t2_inst,       T2_FIRST_ID,            0);
  SET_INSTANCE(rpm_inst,      RPM_FIRST_ID,           DATA_ID_RPM);
  SET_INSTANCE(fuel_inst,     FUEL_FIRST_ID,          0);
  SET_INSTANCE(fuel_qty_inst, FUEL_QTY_FIRST_ID,      0);
  SET_INSTANCE(aspd_inst,     AIR_SPEED_FIRST_ID,     0);
  SET_INSTANCE(vvspd_inst,    VARIO_FIRST_ID,         DATA_ID_VARIO);
  SET_INSTANCE(valt_inst,     ALT_FIRST_ID,           DATA_ID_VARIO);
  SET_INSTANCE(fasv_inst,     VFAS_FIRST_ID,          DATA_ID_FAS);
  SET_INSTANCE(fasc_inst,     CURR_FIRST_ID,          DATA_ID_FAS);
  SET_INSTANCE(cells_inst,    CELLS_FIRST_ID,         DATA_ID_FLVSS);
  SET_INSTANCE(gpsa_inst,     GPS_ALT_FIRST_ID,       DATA_ID_GPS);
  SET_INSTANCE(gpss_inst,     GPS_SPEED_FIRST_ID,     DATA_ID_GPS);
  SET_INSTANCE(gpsc_inst,     GPS_COURS_FIRST_ID,     DATA_ID_GPS);
  SET_INSTANCE(gpst_inst,     GPS_TIME_DATE_FIRST_ID, DATA_ID_GPS);
  SET_INSTANCE(gpsll_inst,    GPS_LONG_LATI_FIRST_ID, DATA_ID_GPS);
  SET_INSTANCE(accx_inst,     ACCX_FIRST_ID,          0);
  SET_INSTANCE(accy_inst,     ACCY_FIRST_ID,          0);
  SET_INSTANCE(accz_inst,     ACCZ_FIRST_ID,          0);

  refreshSensorRatios(simulator);
}

void TelemetryProviderFrSky::refreshSensorRatios(SimulatorInterface * simulator)
{
  ui->rxbt_ratio->setValue(simulator->getSensorRatio(BATT_ID) / 10.0);
  ui->A1_ratio->setValue(simulator->getSensorRatio(ADC1_ID) / 10.0);
  ui->A2_ratio->setValue(simulator->getSensorRatio(ADC2_ID) / 10.0);
}

void TelemetryProviderFrSky::generateTelemetryFrame(SimulatorInterface *simulator)
{
  static int item = 0;
  bool ok = true;
  uint8_t buffer[FRSKY_SPORT_PACKET_SIZE] = {0};
  static FlvssEmulator *flvss = new FlvssEmulator();

  switch (item++) {
    case 0:
#if defined(XJT_VERSION_ID)
      generateSportPacket(buffer, 1, DATA_FRAME, XJT_VERSION_ID, 11);
#endif
      refreshSensorRatios(simulator);    // placed here in order to call this less often
    break;

    case 1:
      if (ui->rxbt->text().length()) {
        generateSportPacket(buffer, ui->rxbt_inst->text().toInt(&ok, 0), DATA_FRAME, BATT_ID, LIMIT<uint32_t>(0, ui->rxbt->value() * 255.0 / ui->rxbt_ratio->value(), 0xFFFFFFFF));
      }
      break;

    case 2:
      if (ui->Rssi->text().length())
        generateSportPacket(buffer, ui->rssi_inst->text().toInt(&ok, 0), DATA_FRAME, RSSI_ID, LIMIT<uint32_t>(0, ui->Rssi->text().toInt(&ok, 0), 0xFF));
      break;

    case 3:
      if (ui->Swr->text().length())
        generateSportPacket(buffer, ui->swr_inst->text().toInt(&ok, 0), DATA_FRAME, RAS_ID, LIMIT<uint32_t>(0, ui->Swr->text().toInt(&ok, 0), 0xFFFF));
      break;

    case 4:
      if (ui->A1->value() > 0)
        generateSportPacket(buffer, ui->a1_inst->text().toInt(&ok, 0), DATA_FRAME, ADC1_ID, LIMIT<uint32_t>(0, ui->A1->value() * 255.0 / ui->A1_ratio->value(), 0xFF));
      break;

    case 5:
      if (ui->A2->value() > 0)
        generateSportPacket(buffer, ui->a2_inst->text().toInt(&ok, 0), DATA_FRAME, ADC2_ID, LIMIT<uint32_t>(0, ui->A2->value() * 255.0 / ui->A2_ratio->value(), 0xFF));
      break;

    case 6:
      if (ui->A3->value() != 0)
        generateSportPacket(buffer, ui->a3_inst->text().toInt(&ok, 0), DATA_FRAME, A3_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->A3->value() * 100.0, 0x7FFFFFFF));
      break;

    case 7:
      if (ui->A4->value() != 0)
        generateSportPacket(buffer, ui->a4_inst->text().toInt(&ok, 0), DATA_FRAME, A4_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->A4->value() * 100.0, 0x7FFFFFFF));
      break;

    case 8:
      if (ui->T1->value() != 0)
        generateSportPacket(buffer, ui->t1_inst->text().toInt(&ok, 0), DATA_FRAME, T1_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->T1->value(), 0x7FFFFFFF));
      break;

    case 9:
      if (ui->T2->value() != 0)
        generateSportPacket(buffer, ui->t2_inst->text().toInt(&ok, 0), DATA_FRAME, T2_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->T2->value(), 0x7FFFFFFF));
      break;

    case 10:
      if (ui->rpm->value() > 0)
        generateSportPacket(buffer, ui->rpm_inst->text().toInt(&ok, 0), DATA_FRAME, RPM_FIRST_ID, LIMIT<uint32_t>(0, ui->rpm->value(), 0x7FFFFFFF));
      break;

    case 11:
      if (ui->fuel->value() > 0)
        generateSportPacket(buffer, ui->fuel_inst->text().toInt(&ok, 0), DATA_FRAME, FUEL_FIRST_ID, LIMIT<uint32_t>(0, ui->fuel->value(), 0xFFFF));
      break;

    case 12:
      if (ui->vspeed->value() != 0)
        generateSportPacket(buffer, ui->vvspd_inst->text().toInt(&ok, 0), DATA_FRAME, VARIO_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->vspeed->value() * 100.0, 0x7FFFFFFF));
      break;

    case 13:
      if (ui->valt->value() != 0)
        generateSportPacket(buffer, ui->valt_inst->text().toInt(&ok, 0), DATA_FRAME, ALT_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->valt->value() * 100.0, 0x7FFFFFFF));
      break;

    case 14:
      if (ui->vfas->value() != 0)
        generateSportPacket(buffer, ui->fasv_inst->text().toInt(&ok, 0), DATA_FRAME, VFAS_FIRST_ID, LIMIT<uint32_t>(0, ui->vfas->value() * 100.0, 0xFFFFFFFF));
      break;

    case 15:
      if (ui->curr->value() != 0)
        generateSportPacket(buffer, ui->fasc_inst->text().toInt(&ok, 0), DATA_FRAME, CURR_FIRST_ID, LIMIT<uint32_t>(0, ui->curr->value() * 10.0, 0xFFFFFFFF));
      break;

    case 16:
      double cellValues[FlvssEmulator::MAXCELLS];
      if (ui->cell1->value() > 0.009) { // ??? cell1 returning non-zero value when spin box is zero!
        cellValues[0] = ui->cell1->value();
        cellValues[1] = ui->cell2->value();
        cellValues[2] = ui->cell3->value();
        cellValues[3] = ui->cell4->value();
        cellValues[4] = ui->cell5->value();
        cellValues[5] = ui->cell6->value();
        cellValues[6] = ui->cell7->value();
        cellValues[7] = ui->cell8->value();
        generateSportPacket(buffer, ui->cells_inst->text().toInt(&ok, 0), DATA_FRAME, CELLS_FIRST_ID, flvss->setAllCells_GetNextPair(cellValues));
      }
      else {
        cellValues[0] = 0;
        flvss->setAllCells_GetNextPair(cellValues);
      }
      break;

    case 17:
      if (ui->aspeed->value() > 0)
        generateSportPacket(buffer, ui->aspd_inst->text().toInt(&ok, 0), DATA_FRAME, AIR_SPEED_FIRST_ID, LIMIT<uint32_t>(0, ui->aspeed->value() * 5.39957, 0xFFFFFFFF));
      break;

    case 18:
      if (ui->gps_alt->value() != 0) {
        generateSportPacket(buffer, ui->gpsa_inst->text().toInt(&ok, 0), DATA_FRAME, GPS_ALT_FIRST_ID, getNextGPSPacketData(GPS_ALT_FIRST_ID));
      }
      break;

    case 19:
      if (ui->gps_speed->value() > 0) {
        generateSportPacket(buffer, ui->gpss_inst->text().toInt(&ok, 0), DATA_FRAME, GPS_SPEED_FIRST_ID, getNextGPSPacketData(GPS_SPEED_FIRST_ID));
      }
      break;

    case 20:
      if (ui->gps_course->value() != 0) {
        generateSportPacket(buffer, ui->gpsc_inst->text().toInt(&ok, 0), DATA_FRAME, GPS_COURS_FIRST_ID, getNextGPSPacketData(GPS_COURS_FIRST_ID));
      }
      break;

    case 21:
      if (ui->gps_time->text().length()) {
        generateSportPacket(buffer, ui->gpst_inst->text().toInt(&ok, 0), DATA_FRAME, GPS_TIME_DATE_FIRST_ID, getNextGPSPacketData(GPS_TIME_DATE_FIRST_ID));
      }
      break;

    case 22:
      if (ui->gps_latlon->text().length()) {
        generateSportPacket(buffer, ui->gpsll_inst->text().toInt(&ok, 0), DATA_FRAME, GPS_LONG_LATI_FIRST_ID, getNextGPSPacketData(GPS_LONG_LATI_FIRST_ID));
      }
      break;

    case 23:
        if (ui->accx->value() != 0)
          generateSportPacket(buffer, ui->accx_inst->text().toInt(&ok, 0), DATA_FRAME, ACCX_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->accx->value() * 100.0, 0x7FFFFFFF));
        break;

    case 24:
      if (ui->accy->value() != 0)
        generateSportPacket(buffer, ui->accy_inst->text().toInt(&ok, 0), DATA_FRAME, ACCY_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->accy->value() * 100.0, 0x7FFFFFFF));
      break;

    case 25:
      if (ui->accz->value() != 0)
        generateSportPacket(buffer, ui->accz_inst->text().toInt(&ok, 0), DATA_FRAME, ACCZ_FIRST_ID, LIMIT<int32_t>(-0x7FFFFFFF, ui->accz->value() * 100.0, 0x7FFFFFFF));
      break;

    case 26:
      if (ui->fuel_qty->value() > 0)
        generateSportPacket(buffer, ui->fuel_qty_inst->text().toInt(&ok, 0), DATA_FRAME, FUEL_QTY_FIRST_ID, LIMIT<uint32_t>(0, ui->fuel_qty->value() * 100.0, 0xFFFFFF));
      break;

    default:
      item = 0;
      return;
  }

  if (ok && (buffer[2] || buffer[3])) {
    QByteArray ba((char *)buffer, FRSKY_SPORT_PACKET_SIZE);
    emit telemetryDataChanged(SIMU_TELEMETRY_PROTOCOL_FRSKY_SPORT, ba);
    //qDebug("%02X %02X %02X %02X %02X %02X %02X %02X %02X", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
  }
  else {
    generateTelemetryFrame(simulator);
  }
}

void setSportPacketCrc(uint8_t * packet)
{
  short crc = 0;
  for (int i=1; i<FRSKY_SPORT_PACKET_SIZE-1; i++) {
    crc += packet[i]; //0-1FF
    crc += crc >> 8; //0-100
    crc &= 0x00ff;
    crc += crc >> 8; //0-0FF
    crc &= 0x00ff;
  }
  packet[FRSKY_SPORT_PACKET_SIZE-1] = 0xFF - (crc & 0x00ff);
}

uint8_t getBit(uint8_t position, uint8_t value)
{
  return (value & (uint8_t)(1 << position)) ? 1 : 0;
}

bool TelemetryProviderFrSky::generateSportPacket(uint8_t * packet, uint8_t dataId, uint8_t prim, uint16_t appId, uint32_t data)
{
  if (dataId > 0x1B ) return false;

  // generate Data ID field
  uint8_t bit5 = getBit(0, dataId) ^ getBit(1, dataId) ^ getBit(2, dataId);
  uint8_t bit6 = getBit(2, dataId) ^ getBit(3, dataId) ^ getBit(4, dataId);
  uint8_t bit7 = getBit(0, dataId) ^ getBit(2, dataId) ^ getBit(4, dataId);

  packet[0] = (bit7 << 7) + (bit6 << 6) + (bit5 << 5) + dataId;
  // qDebug("dataID: 0x%02x (%d)", packet[0], dataId);
  packet[1] = prim;
  *((uint16_t *)(packet+2)) = appId;
  *((int32_t *)(packet+4)) = data;
  setSportPacketCrc(packet);
  return true;
}

QHash<QString, QString> * TelemetryProviderFrSky::getSupportedLogItems()
{
  return &supportedLogItems;
}

QString TelemetryProviderFrSky::getLogfileIdentifier()
{
  return QString("TELEMETRY_DATA: FrSky S.Port");
}

QString convertGPSDate(QString input)
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

void TelemetryProviderFrSky::loadItemFromLog(QString item, QString value)
{
  if (item == "RxBt") ui->rxbt->setValue(value.toDouble());
  if (item == "RSSI") ui->Rssi->setValue(value.toInt());
  // SWR doesn't get set as a telemetry item, it appears to just be
  // used to alert when the transmitter / module antenna's broken. So
  // we won't be setting it from the log.
  if (item == "A1") ui->A1->setValue(value.toDouble());
  if (item == "A2") ui->A2->setValue(value.toDouble());
  if (item == "A3") ui->A3->setValue(value.toDouble());
  if (item == "A4") ui->A4->setValue(value.toDouble());
  if (item == "Tmp1") ui->T1->setValue(value.toInt());
  if (item == "Tmp2") ui->T2->setValue(value.toInt());
  if (item == "RPM") ui->rpm->setValue(value.toInt());
  if (item == "Fuel") {
    // Both of these end up as "Fuel", so set them both and hope that
    // nobody has both Fuel(%) and Fuel(ml) set up on the same
    // vehicle.
    ui->fuel->setValue(value.toInt());
    ui->fuel_qty->setValue(value.toInt());
  }
  if (item == "VSpd") ui->vspeed->setValue(value.toDouble());
  if (item == "Alt") ui->valt->setValue(value.toDouble());
  if (item == "VFAS") ui->vfas->setValue(value.toDouble());
  if (item == "Curr") ui->curr->setValue(value.toDouble());
  if (item == "Cels") {
    // Individual cell voltages don't get logged, just the total. So
    // set each cell to be equal and add up to the value supplied
    double cellV = value.toDouble() / 8;
    ui->cell1->setValue(cellV);
    ui->cell2->setValue(cellV);
    ui->cell3->setValue(cellV);
    ui->cell4->setValue(cellV);
    ui->cell5->setValue(cellV);
    ui->cell6->setValue(cellV);
    ui->cell7->setValue(cellV);
    ui->cell8->setValue(cellV);
  }
  if (item == "ASpd") ui->aspeed->setValue(value.toDouble());
  if (item == "GAlt") ui->gps_alt->setValue(value.toDouble());
  if (item == "GSpd") ui->gps_speed->setValue(value.toDouble());
  if (item == "Hdg") ui->gps_course->setValue(value.toDouble());
  if (item == "Date") {
    // This is logged as YYYY-mm-dd hh:mm:ss, but we want dd-MM-yyyy
    // hh:mm:ss in the UI
    ui->gps_time->setText(convertGPSDate(value));
  }
  if (item == "GPS") ui->gps_latlon->setText(value);
  if (item == "AccX") ui->accx->setValue(value.toDouble());
  if (item == "AccY") ui->accy->setValue(value.toDouble());
  if (item == "AccZ") ui->accz->setValue(value.toDouble());
}

uint32_t TelemetryProviderFrSky::FlvssEmulator::encodeCellPair(uint8_t cellNum, uint8_t firstCellNo, double cell1, double cell2)
{
  uint16_t cell1Data = cell1 * 500.0;
  uint16_t cell2Data = cell2 * 500.0;
  uint32_t cellData = 0;

  cellData = cell2Data & 0x0FFF;
  cellData <<= 12;
  cellData |= cell1Data & 0x0FFF;
  cellData <<= 4;
  cellData |= cellNum & 0x0F;
  cellData <<= 4;
  cellData |= firstCellNo & 0x0F;

  return cellData;
}

void TelemetryProviderFrSky::FlvssEmulator::encodeAllCells()
{
  cellData1 = encodeCellPair(numCells, 0, cellFloats[0], cellFloats[1]);
  if (numCells > 2) cellData2 = encodeCellPair(numCells, 2, cellFloats[2], cellFloats[3]); else cellData2 = 0;
  if (numCells > 4) cellData3 = encodeCellPair(numCells, 4, cellFloats[4], cellFloats[5]); else cellData3 = 0;
  if (numCells > 6) cellData4 = encodeCellPair(numCells, 6, cellFloats[6], cellFloats[7]); else cellData4 = 0;
}

void TelemetryProviderFrSky::FlvssEmulator::splitIntoCells(double totalVolts)
{
  numCells = qFloor((totalVolts / 3.7) + .5);
  double avgVolts = totalVolts / numCells;
  double remainder = (totalVolts - (avgVolts * numCells));
    for (uint32_t i = 0; (i < numCells) && ( i < MAXCELLS); i++) {
    cellFloats[i] = avgVolts;
  }
  for (uint32_t i = numCells; i < MAXCELLS; i++) {
    cellFloats[i] = 0;
  }
  cellFloats[0] += remainder;
  numCells = numCells > MAXCELLS ? MAXCELLS : numCells; // force into valid cell count in case of input out of range
}

uint32_t TelemetryProviderFrSky::FlvssEmulator::setAllCells_GetNextPair(double cellValues[MAXCELLS])
{
  numCells = 0;
  for (uint32_t i = 0; i < MAXCELLS; i++) {
    if ((i == 0) && (cellValues[0] > 4.2)) {
      splitIntoCells(cellValues[0]);
      break;
    }
    if (cellValues[i] > 0) {
      cellFloats[i] = cellValues[i];
      numCells++;
    }
    else {
      // zero marks the last cell
      for (uint32_t x = i; x < MAXCELLS; x++) {
        cellFloats[x] = 0;
      }
      break;
    }
  }

  // encode the double values into telemetry format
  encodeAllCells();

  // return the value for the current pair
  uint32_t cellData = 0;
  if (nextCellNum >= numCells) {
    nextCellNum = 0;
  }
  switch (nextCellNum) {
  case 0:
    cellData = cellData1;
    break;
  case 2:
    cellData = cellData2;
    break;
  case 4:
    cellData = cellData3;
    break;
  case 6:
    cellData = cellData4;
    break;
  }
  nextCellNum += 2;
  return cellData;
}

uint32_t encodeLatLon(double latLon, bool isLat)
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

uint32_t encodeDateTime(uint8_t yearOrHour, uint8_t monthOrMinute, uint8_t dayOrSecond, bool isDate)
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

uint32_t TelemetryProviderFrSky::getNextGPSPacketData(uint32_t packetType)
{
  switch (packetType) {
  case GPS_LONG_LATI_FIRST_ID:
    sendLat = !sendLat;
    return sendLat ? encodeLatLon(gps.lat, true) : encodeLatLon(gps.lon, false);
    break;
  case GPS_TIME_DATE_FIRST_ID:
    {
      QDateTime dt = gps.dt;

      sendDate = !sendDate;
      return sendDate ? encodeDateTime(dt.date().year() - 2000, dt.date().month(), dt.date().day(), true) : encodeDateTime(dt.time().hour(), dt.time().minute(), dt.time().second(), false);
    }
    break;
  case GPS_ALT_FIRST_ID:
    return (uint32_t) (gps.altitude * 100);
    break;
  case GPS_SPEED_FIRST_ID:
    {
      double speedKNOTS = gps.speedKMH * 0.539957;

      return speedKNOTS * 1000;
    }
    break;
  case GPS_COURS_FIRST_ID:
    return gps.courseDegrees * 100;
    break;
  }
  return 0;
}

void TelemetryProviderFrSky::setGPSDateTime(QString dateTime)
{
  QDateTime dt = QDateTime::currentDateTime().toTimeSpec(Qt::UTC); // default to current systemtime
  if (!dateTime.startsWith('*')) {
    QString format("dd-MM-yyyy hh:mm:ss");
    dt = QDateTime::fromString(dateTime, format);
  }
  gps.setDateTime(dt);
}

void TelemetryProviderFrSky::setDateTimeFromGPS(QDateTime dateTime)
{
  QString format("dd-MM-yyyy hh:mm:ss");
  ui->gps_time->setText(dateTime.toString(format));
}

void TelemetryProviderFrSky::on_saveTelemetryvalues_clicked()
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

    out<< getLogfileIdentifier();
    out << "\r\n";
    out<< ui -> rxbt -> text();
    out<<"\r\n";
    out<< ui -> Rssi -> text();
    out<<"\r\n";
    out<< ui -> Swr -> text();
    out<<"\r\n";
    out << ui -> A1 -> text();
    out<<"\r\n";
    out<< ui -> A2 -> text();
    out<<"\r\n";
    out<< ui -> A3 -> text();
    out<<"\r\n";
    out << ui -> A4 -> text();
    out<<"\r\n";
    out << ui -> T1 -> text();
    out<<"\r\n";
    out << ui -> T2 -> text();
    out<<"\r\n";
    out << ui -> rpm -> text();
    out<<"\r\n";
    out << ui -> fuel -> text();
    out<<"\r\n";
    out << ui -> fuel_qty -> text();
    out<<"\r\n";
    out << ui -> vspeed -> text();
    out<<"\r\n";
    out << ui -> valt -> text();
    out<<"\r\n";
    out << ui -> vfas -> text();
    out<<"\r\n";
    out << ui -> curr -> text();
    out<<"\r\n";
    out << ui -> cell1 -> text();
    out<<"\r\n";
    out << ui -> cell2 -> text();
    out<<"\r\n";
    out << ui -> cell3 -> text();
    out<<"\r\n";
    out << ui -> cell4 -> text();
    out<<"\r\n";
    out << ui -> cell5 -> text();
    out<<"\r\n";
    out << ui -> cell6 -> text();
    out<<"\r\n";
    out << ui -> cell7 -> text();
    out<<"\r\n";    
    out << ui -> cell8 -> text();
    out<<"\r\n"; 
    out << ui -> aspeed -> text();
    out<<"\r\n";
    out << ui -> gps_alt -> text();
    out<<"\r\n";
    out << ui -> gps_speed -> text();
    out<<"\r\n";
    out << ui -> gps_course -> text();
    out<<"\r\n";
    out << ui -> gps_time -> text();
    out<<"\r\n";
    out << ui -> gps_latlon -> text();
    out<<"\r\n";
    out << ui -> accx -> text();
    out<<"\r\n";
    out << ui -> accy -> text();
    out<<"\r\n";
    out << ui -> accz -> text();
    out<<"\r\n";
    file.flush();

    file.close();

}


void TelemetryProviderFrSky::on_loadTelemetryvalues_clicked()
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

    QString n = in.readLine();
    double ns;

    if (n == getLogfileIdentifier()) {
      // It's not a legacy telemetry values file _AND_ it's one of ours
      n = in.readLine();
      ns = n.toDouble();
    } else if (n.length() > 0 && n.at(0).isDigit()) {
      // Legacy telemetry values file starts with numbers and have no header line
      ns = n.toDouble();
    } else {
      // It's not ours, it's not legacy, error.
      QMessageBox::critical(this, CPN_STR_APP_NAME, tr("Not a FrSky S.Port telemetry values file."));
      return;
    }
    ui -> rxbt -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> Rssi -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> Swr -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> A1 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> A2 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> A3 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> A4 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> T1 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> T2 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> rpm -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> fuel -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> fuel_qty -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> vspeed -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> valt -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> vfas -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> curr -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> cell1 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> cell2 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> cell3 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> cell4 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> cell5 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> cell6 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> cell7 -> setValue(ns);
    
    n = in.readLine();
    ns = n.toDouble();
    ui -> cell8 -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> aspeed -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> gps_alt -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> gps_speed -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> gps_course -> setValue(ns);

    n = in.readLine();
    ui -> gps_time -> setText(n);

    n = in.readLine();
    ui -> gps_latlon -> setText(n);

    n = in.readLine();
    ns = n.toDouble();
    ui -> accx -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> accy -> setValue(ns);

    n = in.readLine();
    ns = n.toDouble();
    ui -> accz -> setValue(ns);

    file.close();

}

void TelemetryProviderFrSky::on_GPSpushButton_clicked()
{
  if (ui->GPSpushButton->text() == tr("Run")) {
    ui->GPSpushButton->setText(tr("Stop"));
    gps.start();
  }
  else
  {
    ui->GPSpushButton->setText(tr("Run"));
    gps.stop();
  }
}

