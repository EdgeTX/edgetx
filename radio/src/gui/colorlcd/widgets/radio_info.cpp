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

#include "hal/usb_driver.h"
#include "edgetx.h"
#include "theme_manager.h"
#include "widget.h"
#include "layout.h"

class TopBarWidget : public Widget
{
 public:
  TopBarWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
               int screenNum, int zoneNum) :
      Widget(factory, parent, rect, screenNum, zoneNum)
  {
  }
};

class RadioInfoWidget : public TopBarWidget
{
 public:
  RadioInfoWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
                  int screenNum, int zoneNum) :
      TopBarWidget(factory, parent, rect, screenNum, zoneNum)
  {
    // Logs
    logsIcon = new StaticIcon(this, W_LOG_X, PAD_THREE, ICON_DOT,
                              COLOR_THEME_PRIMARY2_INDEX);
    logsIcon->hide();

    usbIcon =
        new StaticIcon(this, W_USB_X, W_USB_Y, ICON_TOPMENU_USB,
                       COLOR_THEME_PRIMARY2_INDEX);
    usbIcon->hide();

#if defined(AUDIO)
    audioScale = new StaticIcon(this, W_AUDIO_SCALE_X, PAD_TINY,
                                ICON_TOPMENU_VOLUME_SCALE,
                                COLOR_THEME_PRIMARY3_INDEX);

    for (int i = 0; i < 5; i += 1) {
      audioVol[i] = new StaticIcon(
          this, W_AUDIO_X, PAD_TINY,
         (EdgeTxIcon)(ICON_TOPMENU_VOLUME_0 + i),
          COLOR_THEME_PRIMARY2_INDEX);
      audioVol[i]->hide();
    }
    audioVol[0]->show();
#endif

    batteryIcon = new StaticIcon(this, W_AUDIO_X, W_BATT_Y,
                                 ICON_TOPMENU_TXBATT,
                                 COLOR_THEME_PRIMARY2_INDEX);
#if defined(USB_CHARGER)
    batteryChargeIcon = new StaticIcon(
        this, W_BATT_CHG_X, W_BATT_CHG_Y,
        ICON_TOPMENU_TXBATT_CHARGE, COLOR_THEME_PRIMARY2_INDEX);
    batteryChargeIcon->hide();
#endif

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
    extAntenna = new StaticIcon(this, W_RSSI_X - PAD_SMALL, 1,
                                ICON_TOPMENU_ANTENNA,
                                COLOR_THEME_PRIMARY2_INDEX);
    extAntenna->hide();
#endif

    batteryFill = lv_obj_create(lvobj);
    lv_obj_set_pos(batteryFill, W_AUDIO_X + 1, W_BATT_Y + 1);
    lv_obj_set_size(batteryFill, W_BATT_FILL_W, W_BATT_FILL_H);
    lv_obj_set_style_bg_opa(batteryFill, LV_OPA_COVER, LV_PART_MAIN);
    update();

    // RSSI bars
    LAYOUT_VAL_SCALED(RSSI_BH, 31)
    const uint8_t rssiBarsHeight[] = {
      (RSSI_BH * 5 + 15) / 31,
      (RSSI_BH * 10 + 15) / 31,
      (RSSI_BH * 15 + 15) / 31,
      (RSSI_BH * 21 + 15) / 31,
      RSSI_BH};
    for (unsigned int i = 0; i < DIM(rssiBarsHeight); i++) {
      uint8_t height = rssiBarsHeight[i];
      rssiBars[i] = lv_obj_create(lvobj);
      lv_obj_set_pos(rssiBars[i], W_RSSI_X + i * W_RSSI_BAR_SZ,
                     W_RSSI_BAR_H - height);
      lv_obj_set_size(rssiBars[i], W_RSSI_BAR_W, height);
      etx_solid_bg(rssiBars[i], COLOR_THEME_PRIMARY3_INDEX);
      etx_bg_color(rssiBars[i], COLOR_THEME_PRIMARY2_INDEX, LV_STATE_USER_1);
    }

    checkEvents();
  }

  void update() override
  {
    auto widgetData = getPersistentData();

    // get colors from options
    etx_bg_color_from_flags(batteryFill, widgetData->options[2].value.unsignedValue, LV_PART_MAIN);
    etx_bg_color_from_flags(batteryFill, widgetData->options[1].value.unsignedValue, LV_STATE_USER_1);
    etx_bg_color_from_flags(batteryFill, widgetData->options[0].value.unsignedValue, LV_STATE_USER_2);
  }

  void checkEvents() override
  {
    TopBarWidget::checkEvents();

    usbIcon->show(usbPlugged());
    if (getSelectedUsbMode() == USB_UNSELECTED_MODE)
      usbIcon->setColor(COLOR_THEME_PRIMARY3_INDEX);
    else
      usbIcon->setColor(COLOR_THEME_PRIMARY2_INDEX);

    logsIcon->show(!usbPlugged() && isFunctionActive(FUNCTION_LOGS) &&
                   BLINK_ON_PHASE);

#if defined(AUDIO)
    /* Audio volume */
    uint8_t vol = 4;
    if (requiredSpeakerVolume == 0 || g_eeGeneral.beepMode == e_mode_quiet)
      vol = 0;
    else if (requiredSpeakerVolume < 7)
      vol = 1;
    else if (requiredSpeakerVolume < 13)
      vol = 2;
    else if (requiredSpeakerVolume < 19)
      vol = 3;
    if (vol != lastVol) {
      audioVol[vol]->show();
      audioVol[lastVol]->hide();
      lastVol = vol;
    }
#endif

#if defined(USB_CHARGER)
    batteryChargeIcon->show(usbChargerLed());
#endif

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
    extAntenna->show(isModuleXJT(INTERNAL_MODULE) &&
                     isExternalAntennaEnabled());
#endif

    // Battery level
    uint8_t bars = GET_TXBATT_BARS(W_BATT_FILL_W);
    if (bars != lastBatt) {
      lastBatt = bars;
      lv_obj_set_size(batteryFill, bars, W_BATT_FILL_H);
      if (bars >= W_BATT_FILL_GRN) {
        lv_obj_clear_state(batteryFill, LV_STATE_USER_1 | LV_STATE_USER_2);
      } else if (bars >= W_BATT_FILL_ORA) {
        lv_obj_add_state(batteryFill, LV_STATE_USER_1);
        lv_obj_clear_state(batteryFill, LV_STATE_USER_2);
      } else {
        lv_obj_clear_state(batteryFill, LV_STATE_USER_1);
        lv_obj_add_state(batteryFill, LV_STATE_USER_2);
      }
    }

    // RSSI
    const uint8_t rssiBarsValue[] = {30, 40, 50, 60, 80};
    uint8_t rssi = TELEMETRY_RSSI();
    if (rssi != lastRSSI) {
      lastRSSI = rssi;
      for (unsigned int i = 0; i < DIM(rssiBarsValue); i++) {
        if (rssi >= rssiBarsValue[i])
          lv_obj_add_state(rssiBars[i], LV_STATE_USER_1);
        else
          lv_obj_clear_state(rssiBars[i], LV_STATE_USER_1);
      }
    }
  }

  static const WidgetOption options[];

  static constexpr coord_t W_AUDIO_X = 0;
  static LAYOUT_VAL_SCALED(W_AUDIO_SCALE_X, 15)
  static LAYOUT_VAL_SCALED(W_USB_X, 32)
  static LAYOUT_VAL_SCALED(W_USB_Y, 5)
  static constexpr coord_t W_LOG_X = W_USB_X;
  static LAYOUT_VAL_SCALED(W_RSSI_X, 40)
  static LAYOUT_VAL_SCALED(W_RSSI_BAR_W, 4)
  static LAYOUT_VAL_SCALED(W_RSSI_BAR_H, 35)
  static LAYOUT_VAL_SCALED(W_RSSI_BAR_SZ, 6)
  static LAYOUT_VAL_SCALED(W_BATT_Y, 25)
  static LAYOUT_VAL_SCALED(W_BATT_FILL_W, 20)
  static LAYOUT_VAL_SCALED(W_BATT_FILL_H, 10)
  static LAYOUT_VAL_SCALED(W_BATT_FILL_GRN, 12)
  static LAYOUT_VAL_SCALED(W_BATT_FILL_ORA, 5)
  static LAYOUT_VAL_SCALED(W_BATT_CHG_X, 25)
  static LAYOUT_VAL_SCALED(W_BATT_CHG_Y, 23)

 protected:
  uint8_t lastVol = 0;
  uint8_t lastBatt = 0;
  uint8_t lastRSSI = 0;
  StaticIcon* logsIcon;
  StaticIcon* usbIcon;
#if defined(AUDIO)
  StaticIcon* audioScale;
  StaticIcon* audioVol[5];
#endif
  StaticIcon* batteryIcon;
  lv_obj_t* batteryFill = nullptr;
  lv_obj_t* rssiBars[5] = {nullptr};
#if defined(USB_CHARGER)
  StaticIcon* batteryChargeIcon;
#endif
#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
  StaticIcon* extAntenna;
#endif
};

const WidgetOption RadioInfoWidget::options[] = {
    {STR_LOW_BATT_COLOR, WidgetOption::Color, RGB2FLAGS(0xF4, 0x43, 0x36)},
    {STR_MID_BATT_COLOR, WidgetOption::Color, RGB2FLAGS(0xFF, 0xC1, 0x07)},
    {STR_HIGH_BATT_COLOR, WidgetOption::Color, RGB2FLAGS(0x4C, 0xAF, 0x50)},
    {nullptr, WidgetOption::Bool}};

BaseWidgetFactory<RadioInfoWidget> RadioInfoWidget("Radio Info", RadioInfoWidget::options,
                                                   STR_RADIO_INFO_WIDGET);

class DateTimeWidget : public TopBarWidget
{
 public:
  DateTimeWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
                 int screenNum, int zoneNum) :
      TopBarWidget(factory, parent, rect, screenNum, zoneNum)
  {
    coord_t x = rect.w - HeaderDateTime::HDR_DATE_WIDTH - DT_XO;
    dateTime = new HeaderDateTime(this, x, PAD_THREE);
    update();
  }

  void update() override
  {
    auto widgetData = getPersistentData();

    // get color from options
    uint32_t color;
    memcpy(&color, &widgetData->options[0].value.unsignedValue, sizeof(color));
    dateTime->setColor(color);
    coord_t x = width() - HeaderDateTime::HDR_DATE_WIDTH - DT_XO;
    dateTime->setPos(x, PAD_THREE);
  }

  HeaderDateTime* dateTime = nullptr;
  int8_t lastMinute = -1;

  static const WidgetOption options[];

  // Adjustment to make main view date/time align with model/radio settings views
  static LAYOUT_VAL_SCALED(DT_XO, 1)
};

const WidgetOption DateTimeWidget::options[] = {
    {STR_COLOR, WidgetOption::Color, COLOR2FLAGS(COLOR_THEME_PRIMARY2_INDEX)},
    {nullptr, WidgetOption::Bool}};

BaseWidgetFactory<DateTimeWidget> DateTimeWidget("Date Time",
                                                 DateTimeWidget::options,
                                                 STR_DATE_TIME_WIDGET);

#if defined(INTERNAL_GPS)

class InternalGPSWidget : public TopBarWidget
{
 public:
  InternalGPSWidget(const WidgetFactory* factory, Window* parent, const rect_t& rect,
                    int screenNum, int zoneNum) :
      TopBarWidget(factory, parent, rect, screenNum, zoneNum)
  {
    icon =
        new StaticIcon(this, width() / 2 - PAD_LARGE - PAD_TINY, ICON_H,
                       ICON_TOPMENU_GPS, COLOR_THEME_PRIMARY3_INDEX);

    numSats = new DynamicNumber<uint16_t>(
        this, {0, 1, width(), SATS_H}, [=] { return gpsData.numSat; },
        COLOR_THEME_PRIMARY2_INDEX, CENTERED | FONT(XS));
  }

  void checkEvents() override
  {
    TopBarWidget::checkEvents();

    bool hasGPS = serialGetModePort(UART_MODE_GPS) >= 0;

    numSats->show(hasGPS && (gpsData.numSat > 0));
    icon->show(hasGPS);

    if (gpsData.fix)
      icon->setColor(COLOR_THEME_PRIMARY2_INDEX);
    else
      icon->setColor(COLOR_THEME_PRIMARY3_INDEX);
  }

 protected:
  StaticIcon* icon;
  DynamicNumber<uint16_t>* numSats;

  static LAYOUT_VAL_SCALED(ICON_H, 19)
  static LAYOUT_VAL_SCALED(SATS_H, 12)
};

BaseWidgetFactory<InternalGPSWidget> InternalGPSWidget("Internal GPS", nullptr,
                                                       STR_INT_GPS_LABEL);

#endif
