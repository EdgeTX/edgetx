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

#include "gtest/gtest.h"
#include "gtests.h"
#include "telemetry/telemetry.h"
#include "telemetry/ghost.h"
#include "telemetry/sensor_names.h"
#include "crc.h"

#if defined(GHOST)

// GHST_DL_GPS_SECONDARY as sent by Betaflight (ghstFrameGpsSecondaryTelemetry):
// ground speed in cm/s, ground course in 0.1 degree, number of satellites.
static void sendGhostGpsSecondary(uint16_t speed, uint16_t course, uint8_t sats)
{
  uint8_t buffer[14] = {0};
  buffer[0] = GHST_ADDR_RADIO;
  buffer[1] = 12;  // type + 10 bytes payload + crc
  uint8_t* frame = buffer + 2;
  frame[0] = GHST_DL_GPS_SECONDARY;
  frame[1] = speed & 0xFF;
  frame[2] = speed >> 8;
  frame[3] = course & 0xFF;
  frame[4] = course >> 8;
  frame[5] = sats;
  frame[11] = crc8(frame, 11);
  processGhostTelemetryFrame(EXTERNAL_MODULE, buffer, sizeof(buffer));
}

static int findSensor(const char* label)
{
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (g_model.telemetrySensors[i].isAvailable() &&
        !strncmp(g_model.telemetrySensors[i].label, label, TELEM_LABEL_LEN))
      return i;
  }
  return -1;
}

// sensor value in whole units, as shown on the radio
static int32_t sensorValue(int index)
{
  int32_t value = telemetryItems[index].value;
  for (uint8_t prec = g_model.telemetrySensors[index].prec; prec > 0; prec--)
    value /= 10;
  return value;
}

TEST(Ghost, gpsSecondaryHeading)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  telemetryStreaming = TELEMETRY_TIMEOUT10ms;
  allowNewSensors = true;

  // 10 m/s, 185.0 deg, 12 sats
  sendGhostGpsSecondary(1000, 1850, 12);

  int hdg = findSensor(STR_SENSOR_HDG);
  ASSERT_GE(hdg, 0);
  EXPECT_EQ(sensorValue(hdg), 185);
  EXPECT_EQ(telemetryItems[hdg].value, 1850);
  EXPECT_EQ(g_model.telemetrySensors[hdg].prec, 1);

  sendGhostGpsSecondary(1000, 3599, 12);
  EXPECT_EQ(telemetryItems[hdg].value, 3599);

  // sensor discovered by an older firmware (stored with precision 2)
  g_model.telemetrySensors[hdg].prec = 2;
  sendGhostGpsSecondary(1000, 1850, 12);
  EXPECT_EQ(telemetryItems[hdg].value, 18500);
  EXPECT_EQ(sensorValue(hdg), 185);

  // ground speed and satellites were already right
  int gspd = findSensor(STR_SENSOR_GSPD);
  ASSERT_GE(gspd, 0);
  EXPECT_EQ(sensorValue(gspd), 36);
  int sats = findSensor(STR_SENSOR_SATELLITES);
  ASSERT_GE(sats, 0);
  EXPECT_EQ(sensorValue(sats), 12);

  allowNewSensors = false;
}

#endif  // GHOST
