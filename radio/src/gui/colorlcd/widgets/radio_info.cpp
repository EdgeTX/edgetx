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

#include "opentx.h"
#include "hal/usb_driver.h"

#include "widgets_container_impl.h"
#include "theme.h"

constexpr uint32_t WIDGET_REFRESH = 1000 / 10; // 10 Hz

#define W_AUDIO_X 0
#define W_USB_X 32
#define W_LOG_X 32
#define W_RSSI_X 40

static const uint8_t _LBM_DOT[] = {
#include "mask_dot.lbm"
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

class TopBarWidget : public Widget
{
  public:
    TopBarWidget(const WidgetFactory* factory, Window* parent,
               const rect_t& rect, Widget::PersistentData* persistentData) :
      Widget(factory, parent, rect, persistentData)
    {
    }

    void checkEvents() override
    {
      Widget::checkEvents();
      uint32_t now = RTOS_GET_MS();
      if (now - lastRefresh >= WIDGET_REFRESH) {
        lastRefresh = now;
        invalidate();
      }
    }

    uint32_t lastRefresh = 0;
};

class RadioInfoWidget: public TopBarWidget
{
  public:
    RadioInfoWidget(const WidgetFactory* factory, Window* parent,
               const rect_t& rect, Widget::PersistentData* persistentData) :
      TopBarWidget(factory, parent, rect, persistentData)
    {
    }

    void refresh(BitmapBuffer * dc) override
    {
      // USB icon
      if (usbPlugged()) {

        LcdFlags flags = COLOR_THEME_PRIMARY2;
        if (getSelectedUsbMode() == USB_UNSELECTED_MODE) {
          flags = COLOR_THEME_PRIMARY3;
        }

        dc->drawBitmapPattern(W_USB_X, 5, LBM_TOPMENU_USB, flags);
      }

      // Logs
      else if (isFunctionActive(FUNCTION_LOGS) && BLINK_ON_PHASE) {
        dc->drawBitmapPattern(W_LOG_X, 3, LBM_DOT, COLOR_THEME_PRIMARY2);
      }

      // RSSI
      const uint8_t rssiBarsValue[] = {30, 40, 50, 60, 80};
      const uint8_t rssiBarsHeight[] = {5, 10, 15, 21, 31};
      for (unsigned int i = 0; i < DIM(rssiBarsHeight); i++) {
        uint8_t height = rssiBarsHeight[i];
        dc->drawSolidFilledRect(W_RSSI_X + i * 6, 35 - height, 4, height,
                                TELEMETRY_RSSI() >= rssiBarsValue[i]
                                    ? COLOR_THEME_PRIMARY2
                                    : COLOR_THEME_PRIMARY3);
      }

#if defined(INTERNAL_MODULE_PXX1) && defined(EXTERNAL_ANTENNA)
      if (isModuleXJT(INTERNAL_MODULE) && isExternalAntennaEnabled()) {
        dc->drawBitmapPattern(W_RSSI_X - 4, 1, LBM_TOPMENU_ANTENNA, COLOR_THEME_PRIMARY2);
      }
#endif

      /* Audio volume */
      dc->drawBitmapPattern(W_AUDIO_X, 1, LBM_TOPMENU_VOLUME_SCALE, COLOR_THEME_PRIMARY3);
      if (requiredSpeakerVolume == 0 || g_eeGeneral.beepMode == e_mode_quiet)
        dc->drawBitmapPattern(W_AUDIO_X, 1, LBM_TOPMENU_VOLUME_0, COLOR_THEME_PRIMARY2);
      else if (requiredSpeakerVolume < 7)
        dc->drawBitmapPattern(W_AUDIO_X, 1, LBM_TOPMENU_VOLUME_1, COLOR_THEME_PRIMARY2);
      else if (requiredSpeakerVolume < 13)
        dc->drawBitmapPattern(W_AUDIO_X, 1, LBM_TOPMENU_VOLUME_2, COLOR_THEME_PRIMARY2);
      else if (requiredSpeakerVolume < 19)
        dc->drawBitmapPattern(W_AUDIO_X, 1, LBM_TOPMENU_VOLUME_3, COLOR_THEME_PRIMARY2);
      else
        dc->drawBitmapPattern(W_AUDIO_X, 1, LBM_TOPMENU_VOLUME_4, COLOR_THEME_PRIMARY2);

      /* Tx battery */
      uint8_t bars = GET_TXBATT_BARS(5);
#if defined(USB_CHARGER)
      if (usbChargerLed()) {
        dc->drawBitmapPattern(W_AUDIO_X, 22, LBM_TOPMENU_TXBATT_CHARGING, COLOR_THEME_PRIMARY2);
      }
      else {
        dc->drawBitmapPattern(W_AUDIO_X, 22, LBM_TOPMENU_TXBATT, COLOR_THEME_PRIMARY2);
      }
#else
      dc->drawBitmapPattern(W_AUDIO_X, 22, LBM_TOPMENU_TXBATT, COLOR_THEME_PRIMARY2);
#endif
      for (unsigned int i = 0; i < 5; i++) {
        dc->drawSolidFilledRect(W_AUDIO_X + 2 + 4 * i, 27, 2, 8, i >= bars ? COLOR_THEME_PRIMARY3 : COLOR_THEME_PRIMARY2);
      }
    }
};

BaseWidgetFactory<RadioInfoWidget> RadioInfoWidget("Radio Info", nullptr, "Radio Info");

// Adjustment to make main view date/time align with model/radio settings views
#if LCD_W > LCD_H
#define DT_OFFSET 8
#else
#define DT_OFFSET 1
#endif

class DateTimeWidget: public TopBarWidget
{
  public:
    DateTimeWidget(const WidgetFactory* factory, Window* parent,
               const rect_t& rect, Widget::PersistentData* persistentData) :
      TopBarWidget(factory, parent, rect, persistentData)
    {
    }

    void refresh(BitmapBuffer * dc) override
    {
      // get color from options
      LcdFlags color = COLOR2FLAGS(persistentData->options[0].value.unsignedValue);
      EdgeTxTheme::instance()->drawMenuDatetime(dc, width()/2+DT_OFFSET, 3, color);
    }

    void checkEvents() override
    {
      Widget::checkEvents();
      // Only update if minute value has changed
      struct gtm t;
      gettime(&t);
      if (t.tm_min != lastMinute) {
        lastMinute = t.tm_min;
        invalidate();
      }
    }

    int8_t lastMinute = 0;

    static const ZoneOption options[];
};

const ZoneOption DateTimeWidget::options[] = {
  { STR_COLOR, ZoneOption::Color, OPTION_VALUE_UNSIGNED(COLOR_THEME_PRIMARY2 >> 16) },
  { nullptr, ZoneOption::Bool }
};

BaseWidgetFactory<DateTimeWidget> DateTimeWidget("Date Time", DateTimeWidget::options, "Date Time");

#if defined(INTERNAL_GPS)

static const uint8_t _LBM_TOPMENU_GPS[] = {
#include "mask_topmenu_gps_18.lbm"
};

STATIC_LZ4_BITMAP(LBM_TOPMENU_GPS);

class InternalGPSWidget: public TopBarWidget
{
  public:
    InternalGPSWidget(const WidgetFactory* factory, Window* parent,
               const rect_t& rect, Widget::PersistentData* persistentData) :
      TopBarWidget(factory, parent, rect, persistentData)
    {
    }

    void refresh(BitmapBuffer * dc) override
    {
      if (serialGetModePort(UART_MODE_GPS) >= 0) {
        if (gpsData.fix) {
          char s[10];
          sprintf(s, "%d", gpsData.numSat);
          dc->drawText(width() / 2, 1, s, FONT(XS) | CENTERED | COLOR_THEME_PRIMARY2);
        }
        dc->drawBitmapPattern(
            width() / 2 - 10, 19, LBM_TOPMENU_GPS,
            (gpsData.fix) ? COLOR_THEME_PRIMARY2 : COLOR_THEME_PRIMARY3);
      }
    }
};

BaseWidgetFactory<InternalGPSWidget> InternalGPSWidget("Internal GPS", nullptr, "Internal GPS");

#endif
