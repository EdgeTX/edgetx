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
  EXPECT_TRUE(checkSportPacket(pkt1+1));
  // Packet upstream
  uint8_t pkt2[] = { 0x7E, 0x1C, 0x31, 0x00, 0x10, 0x85, 0x64, 0x00, 0x00, 0xD4 };
  EXPECT_TRUE(checkSportPacket(pkt2+1));
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
  EXPECT_TRUE(checkSportPacket(pkt1+1));
  sportProcessTelemetryPacket(0, pkt1+1, sizeof(pkt1) - 1);
  uint8_t pkt2[] = { 0x7E, 0x98, 0x10, 0x06, 0x00, 0x17, 0xD0, 0x00, 0x00, 0x02 };
  EXPECT_TRUE(checkSportPacket(pkt2+1));
  sportProcessTelemetryPacket(0, pkt2+1, sizeof(pkt2) - 1);
  uint8_t pkt3[] = { 0x7E, 0x98, 0x10, 0x06, 0x00, 0x27, 0xD0, 0x00, 0x00, 0xF1 };
  EXPECT_TRUE(checkSportPacket(pkt3+1));
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

  sensorAllocAt(2)->type = TELEM_TYPE_CALCULATED;
  sensorAddress(2)->formula = TELEM_FORMULA_ADD;
  sensorAddress(2)->prec = 1;
  sensorAddress(2)->calc.sources[0] = 1;
  sensorAddress(2)->calc.sources[1] = 2;

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
  sensorAddress(0)->custom.offset = -5;  /* unit: 1/10 amps */
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
  sensorAddress(0)->custom.offset = +5;  /* unit: 1/10 amps */

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

// --- SensorMap tests ---

TEST(SensorMap, BasicInsertAndLookup)
{
  MODEL_RESET();
  TELEMETRY_RESET();

  // Manually set up two sensors with different (id, subId)
  auto& s0 = *sensorAllocAt(0);
  s0.id = 0x0210;
  s0.subId = 0;
  s0.instance = 0;
  s0.protocol = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  s0.init("VFAS", UNIT_VOLTS, 2);


  auto& s1 = *sensorAllocAt(1);
  s1.id = 0x0200;
  s1.subId = 0;
  s1.instance = 0;
  s1.protocol = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  s1.init("Curr", UNIT_AMPS, 1);


  sensorMap.rebuild();

  // Verify lookup via the hash chain
  uint8_t h0 = SensorMap::hash(0x0210, 0);
  bool found0 = false;
  for (int8_t slot = sensorMap.buckets[h0]; slot >= 0; slot = sensorMap.chain[slot]) {
    if (slot == 0) found0 = true;
  }
  EXPECT_TRUE(found0);

  uint8_t h1 = SensorMap::hash(0x0200, 0);
  bool found1 = false;
  for (int8_t slot = sensorMap.buckets[h1]; slot >= 0; slot = sensorMap.chain[slot]) {
    if (slot == 1) found1 = true;
  }
  EXPECT_TRUE(found1);
}

TEST(SensorMap, HashCollisionChain)
{
  MODEL_RESET();
  TELEMETRY_RESET();

  // Find two different IDs that hash to the same bucket
  uint16_t id1 = 0x0210;
  uint8_t h1 = SensorMap::hash(id1, 0);
  uint16_t id2 = 0;
  for (uint16_t candidate = 1; candidate < 0xFFFF; candidate++) {
    if (candidate != id1 && SensorMap::hash(candidate, 0) == h1) {
      id2 = candidate;
      break;
    }
  }
  ASSERT_NE(id2, 0) << "Could not find a colliding ID";

  // Set up two sensors that collide in the hash table
  auto& s0 = *sensorAllocAt(0);
  s0.id = id1;
  s0.subId = 0;
  s0.instance = 0;
  s0.protocol = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  s0.init("Sns1", UNIT_VOLTS, 2);


  auto& s1 = *sensorAllocAt(1);
  s1.id = id2;
  s1.subId = 0;
  s1.instance = 0;
  s1.protocol = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  s1.init("Sns2", UNIT_AMPS, 1);


  sensorMap.rebuild();

  // Both should be in the same bucket chain
  int count = 0;
  bool found0 = false, found1 = false;
  for (int8_t slot = sensorMap.buckets[h1]; slot >= 0; slot = sensorMap.chain[slot]) {
    if (slot == 0) found0 = true;
    if (slot == 1) found1 = true;
    count++;
  }
  EXPECT_TRUE(found0);
  EXPECT_TRUE(found1);
  EXPECT_GE(count, 2);

  // setTelemetryValue should route to the correct slot despite collision
  setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, id1, 0, 0, (int32_t)4200, UNIT_VOLTS, 2);
  EXPECT_EQ(telemetryItems[0].value, 4200);

  setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, id2, 0, 0, (int32_t)1500, UNIT_AMPS, 1);
  EXPECT_EQ(telemetryItems[1].value, 1500);
}

TEST(SensorMap, RemoveAndReinsert)
{
  MODEL_RESET();
  TELEMETRY_RESET();

  auto& s0 = *sensorAllocAt(0);
  s0.id = 0x0100;
  s0.subId = 0;
  s0.instance = 0;
  s0.protocol = PROTOCOL_TELEMETRY_FRSKY_SPORT;
  s0.init("Alt", UNIT_METERS, 2);


  sensorMap.rebuild();

  uint8_t h = SensorMap::hash(0x0100, 0);
  EXPECT_GE(sensorMap.buckets[h], 0);

  sensorMap.remove(0);

  // After remove, slot 0 should no longer be in the chain
  bool found = false;
  for (int8_t slot = sensorMap.buckets[h]; slot >= 0; slot = sensorMap.chain[slot]) {
    if (slot == 0) found = true;
  }
  EXPECT_FALSE(found);

  // Re-insert
  sensorMap.insert(0);
  found = false;
  for (int8_t slot = sensorMap.buckets[h]; slot >= 0; slot = sensorMap.chain[slot]) {
    if (slot == 0) found = true;
  }
  EXPECT_TRUE(found);
}

TEST(SensorMap, GhostSlotInMap)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  allowNewSensors = true;

  // Create a sensor via telemetry discovery
  sensorMap.rebuild();
  setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, (uint16_t)0x0210, (uint8_t)0, (uint8_t)0,
                    (int32_t)4200, (uint32_t)UNIT_VOLTS, (uint32_t)2);
  EXPECT_TRUE(sensorAddress(0)->isAvailable());
  EXPECT_EQ(sensorAddress(0)->protocol, PROTOCOL_TELEMETRY_FRSKY_SPORT);

  // Delete it (ghost)
  delTelemetryIndex(0);
  EXPECT_FALSE(sensorAddress(0)->isAvailable());
  EXPECT_EQ(sensorAddress(0)->id, 0x0210);  // identity preserved

  // Ghost should still be in the map (for reactivation)
  uint8_t h = SensorMap::hash(0x0210, 0);
  bool found = false;
  for (int8_t slot = sensorMap.buckets[h]; slot >= 0; slot = sensorMap.chain[slot]) {
    if (slot == 0) found = true;
  }
  EXPECT_TRUE(found);

  // Re-discover same sensor — should reactivate in slot 0
  setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, (uint16_t)0x0210, (uint8_t)0, (uint8_t)0,
                    (int32_t)3800, (uint32_t)UNIT_VOLTS, (uint32_t)2);
  EXPECT_TRUE(sensorAddress(0)->isAvailable());
  EXPECT_EQ(telemetryItems[0].value, 3800);
}

TEST(SensorMap, ProtocolFieldSetOnDiscovery)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  allowNewSensors = true;

  sensorMap.rebuild();
  setTelemetryValue(PROTOCOL_TELEMETRY_FRSKY_SPORT, (uint16_t)0x0210, (uint8_t)0, (uint8_t)0,
                    (int32_t)4200, (uint32_t)UNIT_VOLTS, (uint32_t)2);
  EXPECT_EQ(sensorAddress(0)->protocol, PROTOCOL_TELEMETRY_FRSKY_SPORT);

  // Protocol short name round-trip
  const char* name = telemetryProtocolShortName(PROTOCOL_TELEMETRY_FRSKY_SPORT);
  EXPECT_STREQ(name, "Sport");
  EXPECT_EQ(telemetryProtocolFromShortName("Sport", 5), PROTOCOL_TELEMETRY_FRSKY_SPORT);
  EXPECT_EQ(telemetryProtocolFromShortName("CRSF", 4), PROTOCOL_TELEMETRY_CROSSFIRE);
}

