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

#include "topbar_impl.h"
#include "opentx.h"

const char * const STR_MONTHS[] = TR_MONTHS;
constexpr uint32_t TOPBAR_REFRESH = 1000 / 10; // 10 Hz

static const uint8_t _LBM_DOT[] = {
#include "mask_dot.lbm"
};

static const uint8_t _LBM_TOPMENU_GPS[] = {
#include "mask_topmenu_gps_18.lbm"
};

static const uint8_t _LBM_TOPMENU_USB[] = {
#include "mask_topmenu_usb.lbm"
};

static const uint8_t _LBM_TOPMENU_VOLUME_0[] = {
#include "mask_volume_0.lbm"
};

static const uint8_t _LBM_TOPMENU_VOLUME_1[] = {
#include "mask_volume_1.lbm"
};

static const uint8_t _LBM_TOPMENU_VOLUME_2[] = {
#include "mask_volume_2.lbm"
};

static const uint8_t _LBM_TOPMENU_VOLUME_3[] = {
#include "mask_volume_3.lbm"
};

static const uint8_t _LBM_TOPMENU_VOLUME_4[] = {
#include "mask_volume_4.lbm"
};

static const uint8_t _LBM_TOPMENU_VOLUME_SCALE[] = {
#include "mask_volume_scale.lbm"
};

static const uint8_t _LBM_TOPMENU_TXBATT[] = {
#include "mask_txbat.lbm"
};

static const uint8_t _LBM_TOPMENU_TXBATT_CHARGING[] = {
#include "mask_txbat_charging.lbm"
};

static const uint8_t _LBM_TOPMENU_ANTENNA[] = {
#include "mask_antenna.lbm"
};

STATIC_LZ4_BITMAP(LBM_DOT);
STATIC_LZ4_BITMAP(LBM_TOPMENU_GPS);
STATIC_LZ4_BITMAP(LBM_TOPMENU_USB);
STATIC_LZ4_BITMAP(LBM_TOPMENU_VOLUME_0);
STATIC_LZ4_BITMAP(LBM_TOPMENU_VOLUME_1);
STATIC_LZ4_BITMAP(LBM_TOPMENU_VOLUME_2);
STATIC_LZ4_BITMAP(LBM_TOPMENU_VOLUME_3);
STATIC_LZ4_BITMAP(LBM_TOPMENU_VOLUME_4);
STATIC_LZ4_BITMAP(LBM_TOPMENU_VOLUME_SCALE);
STATIC_LZ4_BITMAP(LBM_TOPMENU_TXBATT);
STATIC_LZ4_BITMAP(LBM_TOPMENU_TXBATT_CHARGING);
STATIC_LZ4_BITMAP(LBM_TOPMENU_ANTENNA);

TopbarImpl::TopbarImpl(Window * parent) :
  TopbarImplBase(parent, {0, 0, LCD_W, MENU_HEADER_HEIGHT}, &g_model.topbarData)
{
}

unsigned int TopbarImpl::getZonesCount() const
{
#if defined(INTERNAL_GPS)
  if (hasSerialMode(UART_MODE_GPS) != -1) {
    return MAX_TOPBAR_ZONES-1;
  }
#endif
  return MAX_TOPBAR_ZONES;
}

rect_t TopbarImpl::getZone(unsigned int index) const
{
  return {
    coord_t(49 + (TOPBAR_ZONE_WIDTH + 2 * TOPBAR_ZONE_MARGIN) * index),
    TOPBAR_ZONE_MARGIN,
    TOPBAR_ZONE_WIDTH,
    TOPBAR_HEIGHT
  };
}

void TopbarImpl::setVisible(float visible) // 0.0 -> 1.0
{
  if (visible == 0.0) {
    setTop(-(int)MENU_HEADER_HEIGHT);
  }
  else if (visible == 1.0) {
    setTop(0);
  }
  else if (visible > 0.0 && visible < 1.0){
    float top = - (float)MENU_HEADER_HEIGHT * (1.0 - visible);
    setTop((coord_t)top);
  }
}

coord_t TopbarImpl::getVisibleHeight(float visible) const // 0.0 -> 1.0
{
  if (visible == 0.0) {
    return 0;
  }
  else if (visible == 1.0) {
    return MENU_HEADER_HEIGHT;
  }

  float h = (float)MENU_HEADER_HEIGHT * visible;
  return (coord_t)h;
}

void TopbarImpl::paint(BitmapBuffer * dc)
{
  dc->drawSolidFilledRect(0, 0, width(), height(), COLOR_THEME_SECONDARY1);
  OpenTxTheme::instance()->drawTopLeftBitmap(dc);
  const TimerOptions timerOptions = {.options = SHOW_TIME}; 
  struct gtm t;
  gettime(&t);
  char str[10];
#if defined(TRANSLATIONS_CN) || defined(TRANSLATIONS_TW) 
      sprintf(str, "%02d-%02d", t.tm_mon + 1, t.tm_mday);
#else
      const char * const STR_MONTHS[] = TR_MONTHS;
      sprintf(str, "%d %s", t.tm_mday, STR_MONTHS[t.tm_mon]);
#endif
  //dc->drawSolidVerticalLine(DATETIME_SEPARATOR_X, 7, 31, COLOR_THEME_SECONDARY1);
  dc->drawText(DATETIME_MIDDLE, DATETIME_LINE1, str, FONT(XS) | CENTERED | COLOR_THEME_PRIMARY2);
  getTimerString(str, getValue(MIXSRC_TX_TIME), timerOptions);
  dc->drawText(DATETIME_MIDDLE, DATETIME_LINE2, str, FONT(XS) | CENTERED | COLOR_THEME_PRIMARY2);

#if defined(INTERNAL_GPS)
  if (hasSerialMode(UART_MODE_GPS) != -1) {
    if (gpsData.fix) {
      char s[10];
      sprintf(s, "%d", gpsData.numSat);
      dc->drawText(GPS_X, 4, s, FONT(XS) | CENTERED | COLOR_THEME_PRIMARY2);
    }
    dc->drawBitmapPattern(
        GPS_X - 10, 22, LBM_TOPMENU_GPS,
        (gpsData.fix) ? COLOR_THEME_PRIMARY2 : COLOR_THEME_PRIMARY3);
  }
#endif

  // USB icon
  if (usbPlugged()) {

    LcdFlags flags = COLOR_THEME_PRIMARY2;
    if (getSelectedUsbMode() == USB_UNSELECTED_MODE) {
      flags = COLOR_THEME_PRIMARY3;
    }

    dc->drawBitmapPattern(USB_X, 8, LBM_TOPMENU_USB, flags);
  }
  // Logs
  else if (isFunctionActive(FUNCTION_LOGS) && BLINK_ON_PHASE) {
    dc->drawBitmapPattern(LOG_X, 6, LBM_DOT, COLOR_THEME_PRIMARY2);
  }

  // RSSI
  const uint8_t rssiBarsValue[] = {30, 40, 50, 60, 80};
  const uint8_t rssiBarsHeight[] = {5, 10, 15, 21, 31};
  for (unsigned int i = 0; i < DIM(rssiBarsHeight); i++) {
    uint8_t height = rssiBarsHeight[i];
    dc->drawSolidFilledRect(RSSI_X + i * 6, 38 - height, 4, height,
                            TELEMETRY_RSSI() >= rssiBarsValue[i]
                                ? COLOR_THEME_PRIMARY2
                                : COLOR_THEME_PRIMARY3);
  }

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  if (isModuleXJT(INTERNAL_MODULE) && isExternalAntennaEnabled()) {
    dc->drawBitmapPattern(RSSI_X - 4, 4, LBM_TOPMENU_ANTENNA, COLOR_THEME_PRIMARY2);
  }
#endif

  /* Audio volume */
  dc->drawBitmapPattern(AUDIO_X, 4, LBM_TOPMENU_VOLUME_SCALE, COLOR_THEME_PRIMARY3);
  if (requiredSpeakerVolume == 0 || g_eeGeneral.beepMode == e_mode_quiet)
    dc->drawBitmapPattern(AUDIO_X, 4, LBM_TOPMENU_VOLUME_0, COLOR_THEME_PRIMARY2);
  else if (requiredSpeakerVolume < 7)
    dc->drawBitmapPattern(AUDIO_X, 4, LBM_TOPMENU_VOLUME_1, COLOR_THEME_PRIMARY2);
  else if (requiredSpeakerVolume < 13)
    dc->drawBitmapPattern(AUDIO_X, 4, LBM_TOPMENU_VOLUME_2, COLOR_THEME_PRIMARY2);
  else if (requiredSpeakerVolume < 19)
    dc->drawBitmapPattern(AUDIO_X, 4, LBM_TOPMENU_VOLUME_3, COLOR_THEME_PRIMARY2);
  else
    dc->drawBitmapPattern(AUDIO_X, 4, LBM_TOPMENU_VOLUME_4, COLOR_THEME_PRIMARY2);

  /* Tx battery */
  uint8_t bars = GET_TXBATT_BARS(5);
#if defined(USB_CHARGER)
  if (usbChargerLed()) {
    dc->drawBitmapPattern(AUDIO_X, 25, LBM_TOPMENU_TXBATT_CHARGING, COLOR_THEME_PRIMARY2);
  }
  else {
    dc->drawBitmapPattern(AUDIO_X, 25, LBM_TOPMENU_TXBATT, COLOR_THEME_PRIMARY2);
  }
#else
  dc->drawBitmapPattern(AUDIO_X, 25, LBM_TOPMENU_TXBATT, COLOR_THEME_PRIMARY2);
#endif
  for (unsigned int i = 0; i < 5; i++) {
    dc->drawSolidFilledRect(AUDIO_X + 2 + 4 * i, 30, 2, 8, i >= bars ? COLOR_THEME_PRIMARY3 : COLOR_THEME_PRIMARY2);
  }

#if 0
  // Radio battery - TODO
  // drawValueWithUnit(370, 8, g_vbat100mV, UNIT_VOLTS, PREC1|FONT(XS)|COLOR_THEME_SECONDARY1);
  // lcdDrawSolidRect(300, 3, 20, 50, COLOR_THEME_SECONDARY1);
  // lcdDrawRect(batt_icon_x+FW, BAR_Y+1, 13, 7);
  // lcdDrawSolidVerticalLine(batt_icon_x+FW+13, BAR_Y+2, 5);

  // Rx battery
  if (g_model.voltsSource) {
    TelemetryItem & item = telemetryItems[g_model.voltsSource-1];
    if (item.isAvailable()) {
      int32_t value = item.value;
      TelemetrySensor & sensor = g_model.telemetrySensors[g_model.frsky.altitudeSource-1];
      LcdFlags att = 0;
      if (sensor.prec == 2) {
        att |= PREC1;
        value /= 10;
      }
      else if (sensor.prec == 1) {
        att |= PREC1;
      }
      att |= (item.isOld() ? COLOR_THEME_WARNING : COLOR_THEME_SECONDARY1);
      lcdDrawSolidFilledRect(ALTITUDE_X, VOLTS_Y, ALTITUDE_W, ALTITUDE_H, COLOR_THEME_SECONDARY3);
      lcdDrawText(ALTITUDE_X+PADDING, VOLTS_Y+2, "Voltage", att);
      drawValueWithUnit(ALTITUDE_X+PADDING, VOLTS_Y+12, value, UNIT_VOLTS, FONT(XL)|LEFT|att);
    }
  }

  // Model altitude
  if (g_model.frsky.altitudeSource) {
    TelemetryItem & item = telemetryItems[g_model.frsky.altitudeSource-1];
    if (item.isAvailable()) {
      int32_t value = item.value;
      TelemetrySensor & sensor = g_model.telemetrySensors[g_model.frsky.altitudeSource-1];
      if (sensor.prec) value /= sensor.prec == 2 ? 100 : 10;
      LcdFlags att = (item.isOld() ? COLOR_THEME_WARNING : COLOR_THEME_SECONDARY1);
      lcdDrawSolidFilledRect(ALTITUDE_X, ALTITUDE_Y, ALTITUDE_W, ALTITUDE_H, COLOR_THEME_SECONDARY3);
      lcdDrawText(ALTITUDE_X+PADDING, ALTITUDE_Y+2, "Alt", att);
      drawValueWithUnit(ALTITUDE_X+PADDING, ALTITUDE_Y+12, value, UNIT_METERS, FONT(XL)|LEFT|att);
    }
  }
#endif
}

void TopbarImpl::checkEvents()
{
  uint32_t now = RTOS_GET_MS();
  if (now - lastRefresh >= TOPBAR_REFRESH) {
    lastRefresh = now;
    invalidate();
  }
}

