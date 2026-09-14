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

#include <algorithm>

#include "choice.h"
#include "edgetx.h"
#include "fonts.h"

// Labels shown in the dropdown, indexed by RadioGpsTool::UriType.
// App names, so left untranslated.
static const char* const uriTypeNames[RadioGpsTool::URI_TYPE_COUNT] = {
    "Standard",
    "Android",
    "Google Maps (web)",
    "Google Maps (app)",
    "CoMaps",
    "Guru Maps",
};

// Every supported URI is "<prefix>lat,lon"
static const char* const uriTypePrefixes[RadioGpsTool::URI_TYPE_COUNT] = {
    "geo:",                       // RFC 5870, understood by iOS and Android
    "geo:0,0?q=",                 // Android-only "search" form
    "https://maps.google.com/?q=",
    "comgooglemaps://?q=",        // Google Maps app URL scheme
    "cm://map?ll=",               // CoMaps
    "guru://show?place=",         // Guru Maps
};

uint8_t RadioGpsTool::uriType = RadioGpsTool::URI_GEO_RFC5870;

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

  // Three bands, top to bottom: coordinates label, QR code, controls.
  // The QR code takes whatever is left between the other two (capped at
  // QR_SZ), so it can never overlap them whatever the screen size.
  coord_t labelY = PAD_LARGE;
  coord_t labelH = getFontHeight(FONT(L));
  coord_t bottomY = window->height() - EdgeTxStyles::UI_ELEMENT_HEIGHT - PAD_LARGE * 2;

  coord_t qrAreaY = labelY + labelH + PAD_LARGE;
  coord_t qrAreaH = bottomY - PAD_LARGE - qrAreaY;
  coord_t qrSz = std::min({QR_SZ, qrAreaH, (coord_t)(window->width() - PAD_LARGE * 2)});
  if (qrSz < 0) qrSz = 0;

  gpsLabel = new StaticText(window, {PAD_LARGE, labelY, LV_SIZE_CONTENT, 0}, "", COLOR_THEME_PRIMARY1_INDEX, FONT(L));
  gpsQR = new QRCode(window, (window->width() - qrSz) / 2, qrAreaY + (qrAreaH - qrSz) / 2, qrSz, "");

  new Choice(window, {PAD_LARGE, bottomY, CHOICE_W, 0},
             uriTypeNames, 0, URI_TYPE_COUNT - 1,
             [=]() { return uriType; },
             [=](int value) {
               uriType = value;
               refresh();
             });

  new TextButton(window,
                {window->width() - BTN_SZ - PAD_LARGE * 2, bottomY, BTN_SZ, 0},
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

// Format a coordinate stored in micro-degrees as a decimal string.
// Integer arithmetic keeps the full 6 decimals (a float only has ~7
// significant digits) and avoids depending on printf float support.
static void formatCoord(char* buf, size_t len, int32_t value)
{
  div_t d = div((int)value, 1000000);
  snprintf(buf, len, "%s%d.%06d", value < 0 ? "-" : "", abs(d.quot), abs(d.rem));
}

void RadioGpsTool::refresh()
{
  if (gpsSensorID >= 0) {
    TelemetryItem& gpsItem = telemetryItems[gpsSensorID];
    char lat[16], lon[16], gps_uri[64];
    formatCoord(lat, sizeof(lat), gpsItem.gps.latitude);
    formatCoord(lon, sizeof(lon), gpsItem.gps.longitude);
    if (uriType >= URI_TYPE_COUNT) uriType = URI_GEO_RFC5870;
    snprintf(gps_uri, sizeof(gps_uri), "%s%s,%s", uriTypePrefixes[uriType], lat, lon);
    gpsQR->setData(gps_uri);
    gpsQR->show();
    gpsLabel->setText(getGPSSensorValue(gpsItem, 0));
  } else {
    gpsQR->hide();
    gpsLabel->setText(STR_NODATA);
  }
}
