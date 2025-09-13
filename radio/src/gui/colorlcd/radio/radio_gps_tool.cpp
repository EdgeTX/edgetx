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

#include "radio_gps_tool.h"

#include "edgetx.h"

RadioGpsTool::RadioGpsTool() :
    Page(ICON_RADIO_TOOLS)
{
  init();
  buildHeader(header);
  buildBody(body);
}

void RadioGpsTool::buildHeader(Window* window)
{
  header->setTitle(STR_MENUTOOLS);
  header->setTitle2(STR_GPS_MODEL_LOCATOR);
}

void RadioGpsTool::buildBody(Window* window)
{
  window->padAll(PAD_ZERO);
  gpsLabel = new StaticText(window, {PAD_LARGE, PAD_LARGE, LV_SIZE_CONTENT, 0}, "", COLOR_THEME_PRIMARY1_INDEX, FONT(L));
  gpsQR = new QRCode(window, (window->width() - QR_SZ) / 2, (window->height() - QR_SZ) / 2, QR_SZ, "");
  new TextButton(window, 
                {window->width() - BTN_SZ - PAD_LARGE * 2, window->height() - EdgeTxStyles::UI_ELEMENT_HEIGHT - PAD_LARGE * 2, BTN_SZ, 0},
                STR_REFRESH, [=]() {
                  refresh();
                  return 0;
                });
  refresh();
}

void RadioGpsTool::init()
{
  gpsSensorID = -1;
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (isGPSSensor(i+1)) {
      gpsSensorID = i;
      return;
    }
  }
}

void RadioGpsTool::refresh()
{
  if (gpsSensorID >= 0) {
    static TelemetryItem& gpsItem = telemetryItems[gpsSensorID];
    char gps_uri[64];
    snprintf(gps_uri, sizeof(gps_uri), "geo:%f,%f", (float)gpsItem.gps.latitude / 1000000, (float)gpsItem.gps.longitude / 1000000);
    gpsQR->setData(gps_uri);
    gpsQR->show();
    gpsLabel->setText(getGPSSensorValue(gpsItem, 0));
  } else {
    gpsQR->hide();
    gpsLabel->setText(STR_NODATA);
  }
}
