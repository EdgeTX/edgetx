/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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
constexpr uint32_t TOPBAR_REFRESH = 1000 / 2; // 2 Hz

TopbarImpl::TopbarImpl(Window * parent) :
  TopbarImplBase({0, 0, LCD_W, MENU_HEADER_HEIGHT}, &g_model.topbarData)
{
  attach(parent);
}

unsigned int TopbarImpl::getZonesCount() const
{
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
  dc->drawSolidFilledRect(0, 0, width(), height(), HEADER_COLOR);
  OpenTxTheme::instance()->drawTopLeftBitmap(dc);

  struct gtm t;
  gettime(&t);
  char str[10];

  sprintf(str, "%d %s", t.tm_mday, STR_MONTHS[t.tm_mon]);
  dc->drawSolidVerticalLine(DATETIME_SEPARATOR_X, 7, 31, MENU_COLOR);
  dc->drawText(DATETIME_MIDDLE, DATETIME_LINE1, str, FONT(XS) | CENTERED| MENU_COLOR);

  getTimerString(str, getValue(MIXSRC_TX_TIME));
  dc->drawText(DATETIME_MIDDLE, DATETIME_LINE2, str, FONT(XS) | CENTERED | MENU_COLOR);

  // USB icon
  if (usbPlugged()) {
    dc->drawBitmapPattern(LCD_W - 98, 8, LBM_TOPMENU_USB, MENU_COLOR);
  }

  // Logs
  if (isFunctionActive(FUNCTION_LOGS) && !usbPlugged() && BLINK_ON_PHASE) {
    dc->drawBitmapPattern(LCD_W - 98, 6, LBM_DOT, MENU_COLOR);
  }

  // RSSI
  const uint8_t rssiBarsValue[] = {30, 40, 50, 60, 80};
  const uint8_t rssiBarsHeight[] = {5, 10, 15, 21, 31};
  for (unsigned int i = 0; i < DIM(rssiBarsHeight); i++) {
    uint8_t height = rssiBarsHeight[i];
    dc->drawSolidFilledRect(LCD_W - 90 + i * 6, 38 - height, 4, height, TELEMETRY_RSSI() >= rssiBarsValue[i] ? MENU_COLOR : MENU_TITLE_DISABLE_COLOR);
  }

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  if (isModuleXJT(INTERNAL_MODULE) && isExternalAntennaEnabled()) {
    dc->drawBitmapPattern(LCD_W-94, 4, LBM_TOPMENU_ANTENNA, MENU_COLOR);
  }
#endif

  /* Audio volume */
  dc->drawBitmapPattern(LCD_W - 130, 4, LBM_TOPMENU_VOLUME_SCALE, MENU_TITLE_DISABLE_COLOR);
  if (requiredSpeakerVolume == 0 || g_eeGeneral.beepMode == e_mode_quiet)
    dc->drawBitmapPattern(LCD_W - 130, 4, LBM_TOPMENU_VOLUME_0, MENU_COLOR);
  else if (requiredSpeakerVolume < 7)
    dc->drawBitmapPattern(LCD_W - 130, 4, LBM_TOPMENU_VOLUME_1, MENU_COLOR);
  else if (requiredSpeakerVolume < 13)
    dc->drawBitmapPattern(LCD_W - 130, 4, LBM_TOPMENU_VOLUME_2, MENU_COLOR);
  else if (requiredSpeakerVolume < 19)
    dc->drawBitmapPattern(LCD_W - 130, 4, LBM_TOPMENU_VOLUME_3, MENU_COLOR);
  else
    dc->drawBitmapPattern(LCD_W - 130, 4, LBM_TOPMENU_VOLUME_4, MENU_COLOR);

  /* Tx battery */
  uint8_t bars = GET_TXBATT_BARS(5);
#if defined(USB_CHARGER)
  if (usbChargerLed()) {
    dc->drawBitmapPattern(LCD_W - 130, 25, LBM_TOPMENU_TXBATT_CHARGING, MENU_COLOR);
  }
  else {
    dc->drawBitmapPattern(LCD_W - 130, 25, LBM_TOPMENU_TXBATT, MENU_COLOR);
  }
#else
  dc->drawBitmapPattern(LCD_W - 130, 25, LBM_TOPMENU_TXBATT, MENU_COLOR);
#endif
  for (unsigned int i = 0; i < 5; i++) {
    dc->drawSolidFilledRect(LCD_W - 128 + 4 * i, 30, 2, 8, i >= bars ? MENU_TITLE_DISABLE_COLOR : MENU_COLOR);
  }

#if 0
  // Radio battery - TODO
  // drawValueWithUnit(370, 8, g_vbat100mV, UNIT_VOLTS, PREC1|FONT(XS)|MENU_COLOR);
  // lcdDrawSolidRect(300, 3, 20, 50, MENU_COLOR);
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
      att |= (item.isOld() ? ALARM_COLOR : DEFAULT_COLOR);
      lcdDrawSolidFilledRect(ALTITUDE_X, VOLTS_Y, ALTITUDE_W, ALTITUDE_H, DEFAULT_BGCOLOR);
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
      LcdFlags att = (item.isOld() ? ALARM_COLOR : DEFAULT_COLOR);
      lcdDrawSolidFilledRect(ALTITUDE_X, ALTITUDE_Y, ALTITUDE_W, ALTITUDE_H, DEFAULT_BGCOLOR);
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
