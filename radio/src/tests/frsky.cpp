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

#include "gtests.h"

void frskyDProcessPacket(const uint8_t *packet);
bool checkSportPacket(const uint8_t *packet);
bool checkSportPacket(const uint8_t *packet);

TEST(FrSky, TelemetryValueWithMinAveraging)
{
  /*
    The following expected[] array is filled
    with values that correspond to 4 elements
    long averaging buffer.
    If length of averaging buffer is changed, this
    values must be adjusted
  */
  uint8_t expected[] = { 10, 12, 17, 25, 35, 45, 55, 65, 75, 85, 92, 97, 100, 100, 100, 100, 100};
  int testPos = 0;
  //test of averaging
  TelemetryMinDecorator<TelemetryFilterDecorator<TelemetryValue>> testVal;
  testVal.reset();
  testVal.set(10);
  EXPECT_EQ(testVal.value(), expected[testPos++]);
  for (int n = 2; n < 10; ++n) {
    testVal.set(n * 10);
    EXPECT_EQ(testVal.value(), expected[testPos++]);
  }
  for (int n = 2; n < 10; ++n) {
    testVal.set(100);
    EXPECT_EQ(testVal.value(), expected[testPos++]);
  }
}

TEST(FrSky, Vfas_0x39_HiPrecision)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  EXPECT_EQ(telemetryItems[0].value, 0);
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  // normal precision, resolution 0.1V
  processHubPacket(VFAS_ID, 1234);  // set value of 123.4V
  EXPECT_EQ(telemetryItems[0].value, 12340);      // stored value has resolution of 0.01V

  // now high precision, resolution 0.01V
  processHubPacket(VFAS_ID, VFAS_D_HIPREC_OFFSET);  // set value of 0V
  EXPECT_EQ(telemetryItems[0].value, 0);
  processHubPacket(VFAS_ID, VFAS_D_HIPREC_OFFSET + 12345);  // set value of 123.45V
  EXPECT_EQ(telemetryItems[0].value, 12345);
  processHubPacket(VFAS_ID, VFAS_D_HIPREC_OFFSET + 30012);  // set value of 300.12V
  EXPECT_EQ(telemetryItems[0].value, 30012);
}

TEST(FrSky, HubAltNegative)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  EXPECT_EQ(telemetryItems[0].value, 0);
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  // altimeter auto offset
  processHubPacket(BARO_ALT_BP_ID, 0);
  processHubPacket(BARO_ALT_AP_ID, 0);
  EXPECT_EQ(telemetryItems[0].value, 0);

  // low precision altimeter, bp always less than 10
  processHubPacket(BARO_ALT_BP_ID, 12);  // set value of 12.3m
  processHubPacket(BARO_ALT_AP_ID, 3);
  EXPECT_EQ(telemetryItems[0].value, 123);      // altitude stored has resolution of 0.1m

  processHubPacket(BARO_ALT_BP_ID, -12);  // set value of -12.3m
  processHubPacket(BARO_ALT_AP_ID, 3);
  EXPECT_EQ(telemetryItems[0].value, -123);

  // hi precision altimeter, bp can be two decimals
  MODEL_RESET();
  TELEMETRY_RESET();

  // altimeter auto offset
  processHubPacket(BARO_ALT_BP_ID, 0);
  processHubPacket(BARO_ALT_AP_ID, 0);
  EXPECT_EQ(telemetryItems[0].value, 0);

  // first trigger hi precision, by setting AP above 9
  processHubPacket(BARO_ALT_BP_ID, -1);  // set value of -1.35m
  processHubPacket(BARO_ALT_AP_ID, 35);
  EXPECT_EQ(telemetryItems[0].value, -13);

  processHubPacket(BARO_ALT_BP_ID, 12);  // set value of 12.35m
  processHubPacket(BARO_ALT_AP_ID, 35);
  EXPECT_EQ(telemetryItems[0].value, 123);

  // now test with the AP less than 10 to check if hiprecision is still active
  processHubPacket(BARO_ALT_BP_ID, 12);  // set value of 12.05m
  processHubPacket(BARO_ALT_AP_ID, 05);
  EXPECT_EQ(telemetryItems[0].value, 120);
}

TEST(FrSky, Gps)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  EXPECT_EQ(telemetryItems[0].value, 0);

  // latitude 15 degrees north, 30.5000 minutes = 15.508333333333333 degrees
  processHubPacket(GPS_LAT_BP_ID, 1530);  // DDDMM.
  processHubPacket(GPS_LAT_AP_ID, 5000);  // .MMMM
  processHubPacket(GPS_LAT_NS_ID, 'N');

  // longitude 45 degrees west, 20.5000 minutes = 45.34166666666667 degrees
  processHubPacket(GPS_LONG_BP_ID, 4520);
  processHubPacket(GPS_LONG_AP_ID, 5000);
  processHubPacket(GPS_LONG_EW_ID, 'E');

  EXPECT_EQ(telemetryItems[0].gps.latitude, 15508333);
  EXPECT_EQ(telemetryItems[0].gps.longitude, 45341666);
}

TEST(FrSkySPORT, checkCrc)
{
  // Packet downstream
  uint8_t pkt1[] = { 0x7E, 0x98, 0x10, 0x10, 0x00, 0x7E, 0x02, 0x00, 0x00, 0x5F };
  EXPECT_EQ(checkSportPacket(pkt1+1), true);
  // Packet upstream
  uint8_t pkt2[] = { 0x7E, 0x1C, 0x31, 0x00, 0x10, 0x85, 0x64, 0x00, 0x00, 0xD4 };
  EXPECT_EQ(checkSportPacket(pkt2+1), true);
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
  //TRACE("crc set: %x", packet[FRSKY_SPORT_PACKET_SIZE-1]);
}

void generateSportCellPacket(uint8_t * packet, uint8_t cells, uint8_t battnumber, uint16_t cell1, uint16_t cell2, uint8_t sensorId=DATA_ID_FLVSS)
{
  packet[0] = sensorId;
  packet[1] = 0x10; //DATA_FRAME
  *((uint16_t *)(packet+2)) = 0x0300; //CELLS_FIRST_ID
  uint32_t data = 0;
  data += (cells << 4) + battnumber;
  data += ((cell1 * 5) & 0xFFF) << 8;
  data += ((cell2 * 5) & 0xFFF) << 20;
  *((int32_t *)(packet+4)) = data;
  setSportPacketCrc(packet);
}

TEST(FrSkySPORT, FrSkyDCells)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  uint8_t pkt1[] = { 0x7E, 0x98, 0x10, 0x06, 0x00, 0x07, 0xD0, 0x00, 0x00, 0x12 };
  EXPECT_EQ(checkSportPacket(pkt1+1), true);
  sportProcessTelemetryPacket(0, pkt1+1, sizeof(pkt1) - 1);
  uint8_t pkt2[] = { 0x7E, 0x98, 0x10, 0x06, 0x00, 0x17, 0xD0, 0x00, 0x00, 0x02 };
  EXPECT_EQ(checkSportPacket(pkt2+1), true);
  sportProcessTelemetryPacket(0, pkt2+1, sizeof(pkt2) - 1);
  uint8_t pkt3[] = { 0x7E, 0x98, 0x10, 0x06, 0x00, 0x27, 0xD0, 0x00, 0x00, 0xF1 };
  EXPECT_EQ(checkSportPacket(pkt3+1), true);
  sportProcessTelemetryPacket(0, pkt3+1, sizeof(pkt3) - 1);
  sportProcessTelemetryPacket(0, pkt1+1, sizeof(pkt1) - 1);
  sportProcessTelemetryPacket(0, pkt2+1, sizeof(pkt2) - 1);
  sportProcessTelemetryPacket(0, pkt3+1, sizeof(pkt3) - 1);
  EXPECT_EQ(telemetryItems[0].cells.count, 3);
  EXPECT_EQ(telemetryItems[0].value, 1200);
  for (int i=0; i<3; i++) {
    EXPECT_EQ(telemetryItems[0].cells.values[i].state, 1);
    EXPECT_EQ(telemetryItems[0].cells.values[i].value, 400);
  }
}

TEST(FrSkySPORT, frskySetCellVoltage)
{
  uint8_t packet[FRSKY_SPORT_PACKET_SIZE];

  MODEL_RESET();
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  // test that simulates 3 cell battery
  generateSportCellPacket(packet, 3, 0, 410, 420);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(checkSportPacket(packet), true) << "Bad CRC generation in setSportPacketCrc()";
  generateSportCellPacket(packet, 3, 2, 430,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  generateSportCellPacket(packet, 3, 0, 405, 300);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 3, 2, 430,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  EXPECT_EQ(telemetryItems[0].cells.count, 3);
  EXPECT_EQ(telemetryItems[0].cells.values[0].value, 405);
  EXPECT_EQ(telemetryItems[0].cells.values[1].value, 300);
  EXPECT_EQ(telemetryItems[0].cells.values[2].value, 430);
  EXPECT_EQ(telemetryItems[0].cells.values[4].value, 0);
  EXPECT_EQ(telemetryItems[0].value, 1135);
  EXPECT_EQ(telemetryItems[0].valueMin, 1135);
  EXPECT_EQ(telemetryItems[0].valueMax, 1260);

  generateSportCellPacket(packet, 3, 0, 405, 250);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  generateSportCellPacket(packet, 3, 2, 430,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  generateSportCellPacket(packet, 3, 0, 410, 420);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  generateSportCellPacket(packet, 3, 2, 430,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  EXPECT_EQ(telemetryItems[0].cells.count, 3);
  EXPECT_EQ(telemetryItems[0].cells.values[0].value, 410);
  EXPECT_EQ(telemetryItems[0].cells.values[1].value, 420);
  EXPECT_EQ(telemetryItems[0].cells.values[2].value, 430);
  EXPECT_EQ(telemetryItems[0].cells.values[4].value, 0);
  EXPECT_EQ(telemetryItems[0].value, 1260);
  EXPECT_EQ(telemetryItems[0].valueMin, 1085);
  EXPECT_EQ(telemetryItems[0].valueMax, 1260);

  //add another two cells - 5 cell battery
  generateSportCellPacket(packet, 5, 0, 418, 408);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 5, 2, 415, 420);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 5, 4, 410,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  EXPECT_EQ(telemetryItems[0].cells.count, 5);
  EXPECT_EQ(telemetryItems[0].cells.values[0].value, 418);
  EXPECT_EQ(telemetryItems[0].cells.values[1].value, 408);
  EXPECT_EQ(telemetryItems[0].cells.values[2].value, 415);
  EXPECT_EQ(telemetryItems[0].cells.values[3].value, 420);
  EXPECT_EQ(telemetryItems[0].cells.values[4].value, 410);
  EXPECT_EQ(telemetryItems[0].cells.values[5].value, 0);
  EXPECT_EQ(telemetryItems[0].value, 2071);
  EXPECT_EQ(telemetryItems[0].valueMin, 2071);
  EXPECT_EQ(telemetryItems[0].valueMax, 2071);

  //simulate very low voltage for cell 3
  generateSportCellPacket(packet, 5, 0, 418, 408);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 5, 2, 100, 420);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 5, 4, 410,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  EXPECT_EQ(telemetryItems[0].cells.count, 5);
  EXPECT_EQ(telemetryItems[0].cells.values[0].value, 418);
  EXPECT_EQ(telemetryItems[0].cells.values[1].value, 408);
  EXPECT_EQ(telemetryItems[0].cells.values[2].value, 100);
  EXPECT_EQ(telemetryItems[0].cells.values[3].value, 420);
  EXPECT_EQ(telemetryItems[0].cells.values[4].value, 410);
  EXPECT_EQ(telemetryItems[0].cells.values[5].value, 0);
  EXPECT_EQ(telemetryItems[0].value, 1756);
  EXPECT_EQ(telemetryItems[0].valueMin, 1756);
  EXPECT_EQ(telemetryItems[0].valueMax, 2071);

  //back to normal (but with reversed order of packets)
  generateSportCellPacket(packet, 5, 4, 410,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 5, 0, 418, 408);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 5, 2, 412, 420);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 5, 4, 410,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  EXPECT_EQ(telemetryItems[0].cells.count, 5);
  EXPECT_EQ(telemetryItems[0].cells.values[0].value, 418);
  EXPECT_EQ(telemetryItems[0].cells.values[1].value, 408);
  EXPECT_EQ(telemetryItems[0].cells.values[2].value, 412);
  EXPECT_EQ(telemetryItems[0].cells.values[3].value, 420);
  EXPECT_EQ(telemetryItems[0].cells.values[4].value, 410);
  EXPECT_EQ(telemetryItems[0].cells.values[5].value, 0);
  EXPECT_EQ(telemetryItems[0].value, 2068);
  EXPECT_EQ(telemetryItems[0].valueMin, 1756);
  EXPECT_EQ(telemetryItems[0].valueMax, 2071);
}

TEST(FrSkySPORT, StrangeCellsBug)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  uint8_t pkt[] = { 0x7E, 0x48, 0x10, 0x00, 0x03, 0x30, 0x15, 0x50, 0x81, 0xD5 };
  EXPECT_EQ(checkSportPacket(pkt+1), true);
  sportProcessTelemetryPacket(0, pkt+1, sizeof(pkt) - 1);
  EXPECT_EQ(telemetryItems[0].cells.count, 3);
  EXPECT_EQ(telemetryItems[0].cells.values[0].value, 0); // now we ignore such low values
  EXPECT_EQ(telemetryItems[0].cells.values[1].value, 413);
}

TEST(FrSkySPORT, frskySetCellVoltageTwoSensors)
{
  uint8_t packet[FRSKY_SPORT_PACKET_SIZE];

  MODEL_RESET();
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  //sensor 1: 3 cell battery
  generateSportCellPacket(packet, 3, 0, 418, 416);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 3, 2, 415,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  EXPECT_EQ(telemetryItems[0].cells.count, 3);
  EXPECT_EQ(telemetryItems[0].cells.values[0].value, 418);
  EXPECT_EQ(telemetryItems[0].cells.values[1].value, 416);
  EXPECT_EQ(telemetryItems[0].cells.values[2].value, 415);
  EXPECT_EQ(telemetryItems[0].cells.values[3].value, 0);
  EXPECT_EQ(telemetryItems[0].value, 1249);
  EXPECT_EQ(telemetryItems[0].valueMin, 1249);
  EXPECT_EQ(telemetryItems[0].valueMax, 1249);

  //sensor 2: 4 cell battery
  generateSportCellPacket(packet, 4, 0, 410, 420, DATA_ID_FLVSS+1);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 4, 2, 400, 405, DATA_ID_FLVSS+1);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  EXPECT_EQ(telemetryItems[1].cells.count, 4);
  EXPECT_EQ(telemetryItems[1].cells.values[0].value, 410);
  EXPECT_EQ(telemetryItems[1].cells.values[1].value, 420);
  EXPECT_EQ(telemetryItems[1].cells.values[2].value, 400);
  EXPECT_EQ(telemetryItems[1].cells.values[3].value, 405);
  EXPECT_EQ(telemetryItems[1].value, 1635);
  EXPECT_EQ(telemetryItems[1].valueMin, 1635);
  EXPECT_EQ(telemetryItems[1].valueMax, 1635);

  g_model.telemetrySensors[2].type = TELEM_TYPE_CALCULATED;
  g_model.telemetrySensors[2].formula = TELEM_FORMULA_ADD;
  g_model.telemetrySensors[2].prec = 1;
  g_model.telemetrySensors[2].calc.sources[0] = 1;
  g_model.telemetrySensors[2].calc.sources[1] = 2;

  telemetryWakeup();

  EXPECT_EQ(telemetryItems[2].value, 287);
  EXPECT_EQ(telemetryItems[2].valueMin, 287);
  EXPECT_EQ(telemetryItems[2].valueMax, 287);

  //now change some voltages
  generateSportCellPacket(packet, 3, 2, 415,   0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 4, 2, 390, 370, DATA_ID_FLVSS+1);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 3, 0, 420, 410);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  generateSportCellPacket(packet, 4, 0, 410, 420, DATA_ID_FLVSS+1);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));

  telemetryWakeup();

  EXPECT_EQ(telemetryItems[2].value, 283);
  EXPECT_EQ(telemetryItems[2].valueMin, 283);
  EXPECT_EQ(telemetryItems[2].valueMax, 287);
}

void generateSportFasVoltagePacket(uint8_t * packet, uint32_t voltage)
{
  packet[0] = 0x22; //DATA_ID_FAS
  packet[1] = 0x10; //DATA_FRAME
  *((uint16_t *)(packet+2)) = 0x0210; //VFAS_FIRST_ID
  *((int32_t *)(packet+4)) = voltage;  // unit 10mV
  setSportPacketCrc(packet);
}

TEST(FrSkySPORT, frskyVfas)
{
  uint8_t packet[FRSKY_SPORT_PACKET_SIZE];

  MODEL_RESET();
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  // tests for Vfas
  generateSportFasVoltagePacket(packet, 5000);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 5000);
  EXPECT_EQ(telemetryItems[0].valueMin, 5000);
  EXPECT_EQ(telemetryItems[0].valueMax, 5000);

  generateSportFasVoltagePacket(packet, 6524);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 6524);
  EXPECT_EQ(telemetryItems[0].valueMin, 6524); // the batt was changed (val > old max)
  EXPECT_EQ(telemetryItems[0].valueMax, 6524);

  generateSportFasVoltagePacket(packet, 1248);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 1248);
  EXPECT_EQ(telemetryItems[0].valueMin, 1248);
  EXPECT_EQ(telemetryItems[0].valueMax, 6524);

  generateSportFasVoltagePacket(packet, 2248);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 2248);
  EXPECT_EQ(telemetryItems[0].valueMin, 1248);
  EXPECT_EQ(telemetryItems[0].valueMax, 6524);
}

void generateSportFasCurrentPacket(uint8_t * packet, uint32_t current)
{
  packet[0] = 0x22; //DATA_ID_FAS
  packet[1] = 0x10; //DATA_FRAME
  *((uint16_t *)(packet+2)) = 0x0200; //CURR_FIRST_ID
  *((int32_t *)(packet+4)) = current;
  setSportPacketCrc(packet);
}

TEST(FrSkySPORT, frskyCurrent)
{
  uint8_t packet[FRSKY_SPORT_PACKET_SIZE];

  MODEL_RESET();
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  allowNewSensors = true;

  // tests for Curr
  generateSportFasCurrentPacket(packet, 0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  g_model.telemetrySensors[0].custom.offset = -5;  /* unit: 1/10 amps */
  generateSportFasCurrentPacket(packet, 0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 0);
  EXPECT_EQ(telemetryItems[0].valueMin, 0);
  EXPECT_EQ(telemetryItems[0].valueMax, 0);

  // measured current less then offset - value should be zero
  generateSportFasCurrentPacket(packet, 4);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 0);
  EXPECT_EQ(telemetryItems[0].valueMin, 0);
  EXPECT_EQ(telemetryItems[0].valueMax, 0);

  generateSportFasCurrentPacket(packet, 10);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 5);
  EXPECT_EQ(telemetryItems[0].valueMin, 0);
  EXPECT_EQ(telemetryItems[0].valueMax, 5);

  generateSportFasCurrentPacket(packet, 500);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 495);
  EXPECT_EQ(telemetryItems[0].valueMin, 0);
  EXPECT_EQ(telemetryItems[0].valueMax, 495);

  generateSportFasCurrentPacket(packet, 200);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 195);
  EXPECT_EQ(telemetryItems[0].valueMin, 0);
  EXPECT_EQ(telemetryItems[0].valueMax, 495);

  // test with positive offset
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  telemetryData.telemetryValid = 0x07;
  g_model.telemetrySensors[0].custom.offset = +5;  /* unit: 1/10 amps */

  generateSportFasCurrentPacket(packet, 0);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 5);
  EXPECT_EQ(telemetryItems[0].valueMin, 5);
  EXPECT_EQ(telemetryItems[0].valueMax, 5);

  generateSportFasCurrentPacket(packet, 500);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 505);
  EXPECT_EQ(telemetryItems[0].valueMin, 5);
  EXPECT_EQ(telemetryItems[0].valueMax, 505);

  generateSportFasCurrentPacket(packet, 200);
  sportProcessTelemetryPacket(0, packet, sizeof(packet));
  EXPECT_EQ(telemetryItems[0].value, 205);
  EXPECT_EQ(telemetryItems[0].valueMin, 5);
  EXPECT_EQ(telemetryItems[0].valueMax, 505);
}

