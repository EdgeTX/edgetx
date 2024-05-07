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

#define LANGUAGE_PACKS_DEFINITION

#include "radio_setup.h"

#include "hal/adc_driver.h"
#include "hal/usb_driver.h"
#include "input_mapping.h"
#include "libopenui.h"
#include "opentx.h"
#include "page.h"
#include "storage/modelslist.h"
#include "tasks/mixer_task.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_two_dsc[] = {LV_GRID_FR(19), LV_GRID_FR(21),
                                         LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class DateTimeWindow : public Window
{
 public:
  DateTimeWindow(Window* parent, const rect_t& rect) :
      Window(parent, rect)
  {
    padAll(PAD_ZERO);
    build();
  }

  void checkEvents() override
  {
    Window::checkEvents();

    if (seconds && (get_tmr10ms() - lastRefresh >= 10)) {
      lastRefresh = get_tmr10ms();

      gettime(&m_tm);
      if (m_tm.tm_year != m_last_tm.tm_year)
        year->update();
      if (m_tm.tm_mon != m_last_tm.tm_mon)
        month->update();
      if (m_tm.tm_mday != m_last_tm.tm_mday)
        day->update();
      if (m_tm.tm_hour != m_last_tm.tm_hour)
        hour->update();
      if (m_tm.tm_min != m_last_tm.tm_min)
        minutes->update();
      if (m_tm.tm_sec != m_last_tm.tm_sec)
        seconds->update();
      m_last_tm = m_tm;
    }
  }

  // Absolute layout for date/time setion due to slow performance
  // of lv_textarea in a flex layout.
  static LAYOUT_VAL(DT_EDT_W, 80, 52)
  static LAYOUT_VAL(DT_Y1, PAD_TINY, PAD_TINY)
  static LAYOUT_VAL(DT_Y2, DT_Y1 + EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_MEDIUM, DT_Y1 + EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_MEDIUM)

 protected:
  bool init = false;
  struct gtm m_tm;
  struct gtm m_last_tm;
  tmr10ms_t lastRefresh = 0;
  NumberEdit* year = nullptr;
  NumberEdit* month = nullptr;
  NumberEdit* day = nullptr;
  NumberEdit* hour = nullptr;
  NumberEdit* minutes = nullptr;
  NumberEdit* seconds = nullptr;

  int8_t daysInMonth()
  {
    static const int8_t dmon[] = {31, 28, 31, 30, 31, 30,
                                  31, 31, 30, 31, 30, 31};
    int16_t year = TM_YEAR_BASE + m_tm.tm_year;
    int8_t days_in_month = dmon[m_tm.tm_mon];
    if ((m_tm.tm_mon == 1) &&
        (((year % 4 == 0) && (year % 100 != 0)) || (year % 400 == 0)))
      days_in_month += 1;
    return days_in_month;
  }

  void setDaysInMonth()
  {
    if (day) {
      day->setMax(daysInMonth());
      if (m_tm.tm_mday > day->getMax()) {
        // Update stored day value if > actual days in month
        // Will be written to RTC via SET_LOAD_DATETIME call after returning
        // UI will update on next iteration of checkEvents
        m_tm.tm_mday = day->getMax();
      }
    }
  }

  void build()
  {
    gettime(&m_tm);
    m_last_tm = m_tm;

    // Date
    new StaticText(this, rect_t{PAD_TINY, DT_Y1 + PAD_MEDIUM, RadioSetupPage::LBL_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, STR_DATE);
    year = new NumberEdit(
        this, rect_t{RadioSetupPage::EDT_X, DT_Y1, DT_EDT_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 2023, 2037,
        [=]() -> int32_t { return TM_YEAR_BASE + m_tm.tm_year; },
        [=](int32_t newValue) {
          m_last_tm.tm_year = m_tm.tm_year = newValue - TM_YEAR_BASE;
          setDaysInMonth();
          SET_LOAD_DATETIME(&m_tm);
        });

    month = new NumberEdit(
        this, rect_t{RadioSetupPage::EDT_X + DT_EDT_W + PAD_TINY, DT_Y1, DT_EDT_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 1, 12,
        [=]() -> int32_t { return 1 + m_tm.tm_mon; },
        [=](int32_t newValue) {
          m_last_tm.tm_mon = m_tm.tm_mon = newValue - 1;
          setDaysInMonth();
          SET_LOAD_DATETIME(&m_tm);
        });
    month->setDisplayHandler(
        [](int32_t value) { return formatNumberAsString(value, LEADING0); });

    day = new NumberEdit(
        this, rect_t{RadioSetupPage::EDT_X + 2 * DT_EDT_W + PAD_SMALL, DT_Y1, DT_EDT_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 1,
        daysInMonth(), [=]() -> int32_t { return m_tm.tm_mday; },
        [=](int32_t newValue) {
          m_last_tm.tm_mday = m_tm.tm_mday = newValue;
          SET_LOAD_DATETIME(&m_tm);
        });
    day->setDisplayHandler(
        [](int32_t value) { return formatNumberAsString(value, LEADING0, 2); });

    // Time
    new StaticText(this, rect_t{PAD_TINY, DT_Y2 + PAD_MEDIUM, RadioSetupPage::LBL_W, EdgeTxStyles::PAGE_LINE_HEIGHT}, STR_TIME);
    hour = new NumberEdit(
        this, rect_t{RadioSetupPage::EDT_X, DT_Y2, DT_EDT_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 0, 23,
        [=]() -> int32_t { return m_tm.tm_hour; },
        [=](int32_t newValue) {
          m_last_tm.tm_hour = m_tm.tm_hour = newValue;
          SET_LOAD_DATETIME(&m_tm);
        });
    hour->setDisplayHandler(
        [](int32_t value) { return formatNumberAsString(value, LEADING0, 2); });

    minutes = new NumberEdit(
        this, rect_t{RadioSetupPage::EDT_X + DT_EDT_W + PAD_TINY, DT_Y2, DT_EDT_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 0, 59,
        [=]() -> int32_t { return m_tm.tm_min; },
        [=](int32_t newValue) {
          m_last_tm.tm_min = m_tm.tm_min = newValue;
          SET_LOAD_DATETIME(&m_tm);
        });
    minutes->setDisplayHandler(
        [](int32_t value) { return formatNumberAsString(value, LEADING0, 2); });

    seconds = new NumberEdit(
        this, rect_t{RadioSetupPage::EDT_X + DT_EDT_W * 2 + PAD_SMALL, DT_Y2, DT_EDT_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 0, 59,
        [=]() -> int32_t { return m_tm.tm_sec; },
        [=](int32_t newValue) {
          m_last_tm.tm_sec = m_tm.tm_sec = newValue;
          SET_LOAD_DATETIME(&m_tm);
        });
    seconds->setDisplayHandler(
        [](int value) { return formatNumberAsString(value, LEADING0, 2); });
  }
};

SetupButtonGroup::SetupButtonGroup(Window* parent, const rect_t& rect, const char* title, int cols, PaddingSize padding, PageDefs pages) :
    Window(parent, rect)
{
  padAll(padding);

  coord_t buttonWidth = (LCD_W - PAD_MEDIUM * (cols + 1) - PAD_TINY * 2) / cols;

  int rows = (pages.size() + cols - 1) / cols;
  int height = rows * EdgeTxStyles::UI_ELEMENT_HEIGHT + (rows - 1) * PAD_MEDIUM + PAD_TINY * 2;
  if (title) {
    height += EdgeTxStyles::PAGE_LINE_HEIGHT + PAD_TINY;
  }
  setHeight(height);

  if (title)
    new Subtitle(this, title);

  int n = 0;
  int remaining = pages.size();
  coord_t xo = 0;
  coord_t yo = title ? EdgeTxStyles::PAGE_LINE_HEIGHT + PAD_TINY : 0;
  coord_t xw = buttonWidth + PAD_MEDIUM;
  coord_t x, y;
  for (auto& entry : pages) {
    if (remaining < cols && (n % cols == 0)) {
      coord_t space = ((cols - remaining) * xw) / (remaining + 1);
      xw += space;
      xo = space;
    }
    x = xo + (n % cols) * xw;
    y = yo + (n / cols) * (EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_MEDIUM);

    // TODO: sort out all caps title strings VS quick menu strings
    std::string title(entry.first);
    std::replace(title.begin(), title.end(), '\n', ' ');

    new TextButton(this, rect_t{x, y, buttonWidth, EdgeTxStyles::UI_ELEMENT_HEIGHT}, title, [&, entry]() {
      entry.second();
      return 0;
    });
    n += 1;
    remaining -= 1;
  }
}

class SubPage : public Page
{
 public:
  SubPage(EdgeTxIcon icon, const char* title) : Page(icon)
  {
    header->setTitle(STR_RADIO_SETUP);
    header->setTitle2(title);

    body->setFlexLayout();
  }
};

class SoundPage : public SubPage
{
 public:
  SoundPage() : SubPage(ICON_RADIO_SETUP, STR_SOUND_LABEL)
  {
    FlexGridLayout grid(col_two_dsc, row_dsc, PAD_TINY);

    auto line = body->newLine(grid);

    // Beeps mode
    new StaticText(line, rect_t{}, STR_SPEAKER);
    new Choice(line, rect_t{}, STR_VBEEPMODE, -2, 1,
               GET_SET_DEFAULT(g_eeGeneral.beepMode));
    line = body->newLine(grid);

    // Main volume
    new StaticText(line, rect_t{}, STR_VOLUME);
    new Slider(line, lv_pct(50), -VOLUME_LEVEL_DEF,
               VOLUME_LEVEL_MAX - VOLUME_LEVEL_DEF,
               GET_SET_DEFAULT(g_eeGeneral.speakerVolume));
    line = body->newLine(grid);

    // Beeps volume
    new StaticText(line, rect_t{}, STR_BEEP_VOLUME);
    new Slider(line, lv_pct(50), -2, +2,
               GET_SET_DEFAULT(g_eeGeneral.beepVolume));
    line = body->newLine(grid);

    // Beeps length
    new StaticText(line, rect_t{}, STR_BEEP_LENGTH);
    new Slider(line, lv_pct(50), -2, +2,
               GET_SET_DEFAULT(g_eeGeneral.beepLength));
    line = body->newLine(grid);

    // Beeps pitch
    new StaticText(line, rect_t{}, STR_BEEP_PITCH);
    auto edit = new NumberEdit(line, rect_t{}, 0, 300,
                               GET_DEFAULT(15 * g_eeGeneral.speakerPitch),
                               [=](int32_t newValue) {
                                 g_eeGeneral.speakerPitch = newValue / 15;
                                 SET_DIRTY();
                               });
    edit->setStep(15);
    edit->setPrefix("+");
    edit->setSuffix("Hz");
    line = body->newLine(grid);

    // Wav volume
    new StaticText(line, rect_t{}, STR_WAV_VOLUME);
    new Slider(line, lv_pct(50), -2, +2,
               GET_SET_DEFAULT(g_eeGeneral.wavVolume));
    line = body->newLine(grid);

    // Background volume
    new StaticText(line, rect_t{}, STR_BG_VOLUME);
    new Slider(line, lv_pct(50), -2, +2,
               GET_SET_DEFAULT(g_eeGeneral.backgroundVolume));
  }
};

#if defined(VARIO)
class VarioPage : public SubPage
{
 public:
  VarioPage() : SubPage(ICON_RADIO_SETUP, STR_VARIO)
  {
    FlexGridLayout grid(col_two_dsc, row_dsc);

    auto line = body->newLine(grid);

    // Vario volume
    new StaticText(line, rect_t{}, STR_VOLUME);
    new Slider(line, lv_pct(50), -2, +2,
               GET_SET_DEFAULT(g_eeGeneral.varioVolume));
    line = body->newLine(grid);

    new StaticText(line, rect_t{}, STR_PITCH_AT_ZERO);
    auto edit = new NumberEdit(
        line, rect_t{}, VARIO_FREQUENCY_ZERO - 400, VARIO_FREQUENCY_ZERO + 400,
        GET_DEFAULT(VARIO_FREQUENCY_ZERO + (g_eeGeneral.varioPitch * 10)),
        SET_VALUE(g_eeGeneral.varioPitch,
                  (newValue - VARIO_FREQUENCY_ZERO) / 10));
    edit->setStep(10);
    edit->setSuffix("Hz");
    line = body->newLine(grid);

    new StaticText(line, rect_t{}, STR_PITCH_AT_MAX);
    edit = new NumberEdit(
        line, rect_t{}, 900, 2500,
        GET_DEFAULT(VARIO_FREQUENCY_ZERO + (g_eeGeneral.varioPitch * 10) +
                    VARIO_FREQUENCY_RANGE + (g_eeGeneral.varioRange * 10)),
        SET_VALUE(
            g_eeGeneral.varioRange,
            (newValue - VARIO_FREQUENCY_ZERO - VARIO_FREQUENCY_RANGE) / 10 -
                g_eeGeneral.varioPitch));
    edit->setStep(10);
    edit->setSuffix("Hz");
    line = body->newLine(grid);

    new StaticText(line, rect_t{}, STR_REPEAT_AT_ZERO);
    edit = new NumberEdit(
        line, rect_t{}, 200, 1000,
        GET_DEFAULT(VARIO_REPEAT_ZERO + (g_eeGeneral.varioRepeat * 10)),
        SET_VALUE(g_eeGeneral.varioRepeat,
                  (newValue - VARIO_REPEAT_ZERO) / 10));
    edit->setStep(10);
    edit->setSuffix("ms");
    line = body->newLine(grid);
  }
};
#endif

#if defined(HAPTIC)
class HapticPage : public SubPage
{
 public:
  HapticPage() : SubPage(ICON_RADIO_SETUP, STR_HAPTIC_LABEL)
  {
    FlexGridLayout grid(col_two_dsc, row_dsc);

    auto line = body->newLine(grid);

    // Haptic mode
    new StaticText(line, rect_t{}, STR_MODE);
    new Choice(line, rect_t{}, STR_VBEEPMODE, -2, 1,
               GET_SET_DEFAULT(g_eeGeneral.hapticMode));
    line = body->newLine(grid);

    // Haptic duration
    new StaticText(line, rect_t{}, STR_LENGTH);
    new Slider(line, lv_pct(50), -2, +2,
               GET_SET_DEFAULT(g_eeGeneral.hapticLength));
    line = body->newLine(grid);

    // Haptic strength
    new StaticText(line, rect_t{}, STR_STRENGTH);
    new Slider(line, lv_pct(50), -2, +2,
               GET_SET_DEFAULT(g_eeGeneral.hapticStrength));
    line = body->newLine(grid);
  }
};
#endif

class AlarmsPage : public SubPage
{
 public:
  AlarmsPage() : SubPage(ICON_RADIO_SETUP, STR_ALARMS_LABEL)
  {
    FlexGridLayout grid(col_two_dsc, row_dsc);

    auto line = body->newLine(grid);
    // Battery warning
    new StaticText(line, rect_t{}, STR_BATTERYWARNING);
    auto edit = new NumberEdit(line, rect_t{}, 30, 120,
                               GET_SET_DEFAULT(g_eeGeneral.vBatWarn), PREC1);
    edit->setSuffix("V");
    line = body->newLine(grid);

    // Inactivity alarm
    new StaticText(line, rect_t{}, STR_INACTIVITYALARM);
    edit = new NumberEdit(line, rect_t{}, 0, 250,
                          GET_SET_DEFAULT(g_eeGeneral.inactivityTimer));

    edit->setDisplayHandler([=](int value) -> std::string {
      std::string suffix(STR_MINUTE_PLURAL2);
      if (value == 1) {
        suffix = std::string(STR_MINUTE_SINGULAR);
      } else if (value < g_use_plural2) {
        const int secondDecimal = (value / 10) % 10;
        if (secondDecimal != 1) {
          const int firstDecimal = value % 10;
          if (firstDecimal) {
            if (firstDecimal < g_min_plural2 &&
                firstDecimal == g_use_singular_in_plural) {
              suffix = std::string(STR_MINUTE_SINGULAR);
            } else if (firstDecimal <= g_max_plural2 &&
                       firstDecimal != g_use_plural2_special_case) {
              suffix = std::string(STR_MINUTE_PLURAL1);
            }
          }
        }
      }
      suffix = " " + suffix;
      return formatNumberAsString(value, 0, 0, nullptr, suffix.c_str());
    });
    line = body->newLine(grid);

    // Alarms warning
    new StaticText(line, rect_t{}, STR_ALARMWARNING);
    new ToggleSwitch(line, rect_t{},
                     GET_SET_INVERTED(g_eeGeneral.disableAlarmWarning));
    line = body->newLine(grid);

    // RSSI shutdown alarm
    new StaticText(line, rect_t{}, STR_RSSI_SHUTDOWN_ALARM);
    new ToggleSwitch(line, rect_t{},
                     GET_SET_INVERTED(g_eeGeneral.disableRssiPoweroffAlarm));
    line = body->newLine(grid);

    // Trainer shutdown alarm
    new StaticText(line, rect_t{}, STR_TRAINER_SHUTDOWN_ALARM);
    new ToggleSwitch(line, rect_t{},
                     GET_SET_INVERTED(g_eeGeneral.disableTrainerPoweroffAlarm));
    line = body->newLine(grid);
  }
};

class BacklightPage : public SubPage
{
 public:
  BacklightPage() : SubPage(ICON_RADIO_SETUP, STR_BACKLIGHT_LABEL)
  {
    FlexGridLayout grid(col_two_dsc, row_dsc, PAD_TINY);

    auto line = body->newLine(grid);

    // Backlight mode
    new StaticText(line, rect_t{}, STR_MODE);

    auto blMode = new Choice(
        line, rect_t{}, STR_VBLMODE, e_backlight_mode_off, e_backlight_mode_on,
        GET_DEFAULT(g_eeGeneral.backlightMode), [=](int32_t newValue) {
          g_eeGeneral.backlightMode = newValue;
          updateBacklightControls();
          SET_DIRTY();
        });

    blMode->setAvailableHandler(
        [=](int newValue) { return newValue != e_backlight_mode_off; });

    backlightTimeout = body->newLine(grid);

    // Delay
    new StaticText(backlightTimeout, rect_t{}, STR_BACKLIGHT_TIMER);
    auto edit =
        new NumberEdit(backlightTimeout, rect_t{}, 5, 600,
                       GET_DEFAULT(g_eeGeneral.lightAutoOff * 5),
                       SET_VALUE(g_eeGeneral.lightAutoOff, newValue / 5));
    edit->setStep(5);
    edit->setSuffix("s");

    backlightOnBright = body->newLine(grid);

    // Backlight ON bright
    new StaticText(backlightOnBright, rect_t{}, STR_BLONBRIGHTNESS);
    backlightOnSlider = new Slider(
        backlightOnBright, lv_pct(50), BACKLIGHT_LEVEL_MIN, BACKLIGHT_LEVEL_MAX,
        [=]() -> int32_t {
          return BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
        },
        [=](int32_t newValue) {
          if (newValue >= g_eeGeneral.blOffBright ||
              g_eeGeneral.backlightMode == e_backlight_mode_on) {
            g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - newValue;
          } else {
            g_eeGeneral.backlightBright =
                BACKLIGHT_LEVEL_MAX - g_eeGeneral.blOffBright;
            backlightOnSlider->update();
          }
          SET_DIRTY();
        });

    backlightOffBright = body->newLine(grid);

    // Backlight OFF bright
    new StaticText(backlightOffBright, rect_t{}, STR_BLOFFBRIGHTNESS);
    backlightOffSlider = new Slider(
        backlightOffBright, lv_pct(50), BACKLIGHT_LEVEL_MIN,
        BACKLIGHT_LEVEL_MAX, GET_DEFAULT(g_eeGeneral.blOffBright),
        [=](int32_t newValue) {
          int32_t onBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
          if (newValue <= onBright ||
              g_eeGeneral.backlightMode == e_backlight_mode_off) {
            g_eeGeneral.blOffBright = newValue;
          } else {
            g_eeGeneral.blOffBright = onBright;
            backlightOffSlider->update();
          }
          SET_DIRTY();
        });

    line = body->newLine(grid);

#if defined(KEYS_BACKLIGHT_GPIO)
    // Keys backlight
    new StaticText(line, rect_t{}, STR_KEYS_BACKLIGHT);
    new ToggleSwitch(line, rect_t{},
                     GET_SET_DEFAULT(g_eeGeneral.keysBacklight));
    line = body->newLine(grid);
#endif

    // Flash beep
    new StaticText(line, rect_t{}, STR_ALARM);
    new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.alarmsFlash));
    line = body->newLine(grid);

    updateBacklightControls();
  }

 protected:
  Window* backlightTimeout = nullptr;
  Window* backlightOnBright = nullptr;
  Window* backlightOffBright = nullptr;
  Slider* backlightOffSlider = nullptr;
  Slider* backlightOnSlider = nullptr;

  void updateBacklightControls()
  {
    switch (g_eeGeneral.backlightMode) {
      case e_backlight_mode_off:
        backlightTimeout->hide();
        backlightOnBright->hide();
        backlightOffBright->show();
        break;
      case e_backlight_mode_keys:
      case e_backlight_mode_sticks:
      case e_backlight_mode_all:
      default: {
        backlightTimeout->show();
        backlightOnBright->show();
        backlightOffBright->show();
        int32_t onBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
        if (onBright < g_eeGeneral.blOffBright)
          g_eeGeneral.backlightBright =
              BACKLIGHT_LEVEL_MAX - g_eeGeneral.blOffBright;
        break;
      }
      case e_backlight_mode_on:
        backlightTimeout->hide();
        backlightOnBright->show();
        backlightOffBright->hide();
        break;
    }
    resetBacklightTimeout();
  }
};

class GpsPage : public SubPage
{
 public:
  GpsPage() : SubPage(ICON_RADIO_SETUP, STR_GPS)
  {
    FlexGridLayout grid(col_two_dsc, row_dsc);

    tzIndex = timezoneIndex(g_eeGeneral.timezone, g_eeGeneral.timezoneMinutes);

    auto line = body->newLine(grid);
    // Timezone
    new StaticText(line, rect_t{}, STR_TIMEZONE);
    auto tz = new NumberEdit(line, rect_t{}, minTimezone(), maxTimezone(),
                             GET_DEFAULT(tzIndex), [=](int newTz) {
                               tzIndex = newTz;
                               g_eeGeneral.timezone = timezoneHour(newTz);
                               g_eeGeneral.timezoneMinutes =
                                   timezoneMinute(newTz);
                               SET_DIRTY();
                             });
    tz->setDisplayHandler([](int32_t tz) { return timezoneDisplay(tz); });
    line = body->newLine(grid);

    // Adjust RTC (from telemetry)
    new StaticText(line, rect_t{}, STR_ADJUST_RTC);
    new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.adjustRTC));
    line = body->newLine(grid);

    // GPS format
    new StaticText(line, rect_t{}, STR_GPS_COORDS_FORMAT);
    new Choice(line, rect_t{}, STR_GPSFORMAT, 0, 1,
               GET_SET_DEFAULT(g_eeGeneral.gpsFormat));
    line = body->newLine(grid);
  }

 protected:
  int tzIndex;
};

class ViewOptionsPage : public SubPage
{
 private:
  const lv_coord_t opt_col_dsc[4] = {LV_GRID_FR(5), LV_GRID_FR(2),
                                     LV_GRID_FR(4), LV_GRID_TEMPLATE_LAST};

  void viewOption(FormLine* line, const char* name,
                  std::function<uint8_t()> getValue,
                  std::function<void(uint8_t)> setValue, uint8_t modelOption)
  {
    line->padLeft(10);
    new StaticText(line, rect_t{}, name);
    new ToggleSwitch(line, rect_t{}, getValue, setValue);
    if (modelOption != OVERRIDE_GLOBAL) {
      std::string s(STR_MODEL);
      s += " - ";
      s += STR_ADCFILTERVALUES[modelOption];
      new StaticText(line, rect_t{}, s.c_str(), COLOR_THEME_SECONDARY1);
    }
  }

 public:
  ViewOptionsPage() : SubPage(ICON_RADIO_SETUP, STR_ENABLED_FEATURES)
  {
    FlexGridLayout grid(opt_col_dsc, row_dsc, PAD_TINY);

    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_RADIO_MENU_TABS);

    line = body->newLine(grid);
    viewOption(line, STR_THEME_EDITOR,
               GET_SET_INVERTED(g_eeGeneral.radioThemesDisabled),
               g_model.radioThemesDisabled);

    line = body->newLine(grid);
    viewOption(line, STR_MENUSPECIALFUNCS,
               GET_SET_INVERTED(g_eeGeneral.radioGFDisabled),
               g_model.radioGFDisabled);

    line = body->newLine(grid);
    viewOption(line, STR_MENUTRAINER,
               GET_SET_INVERTED(g_eeGeneral.radioTrainerDisabled),
               g_model.radioTrainerDisabled);

    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_MODEL_MENU_TABS);

#if defined(HELI)
    line = body->newLine(grid);
    viewOption(line, STR_MENUHELISETUP,
               GET_SET_INVERTED(g_eeGeneral.modelHeliDisabled),
               g_model.modelHeliDisabled);
#endif

#if defined(FLIGHT_MODES)
    line = body->newLine(grid);
    viewOption(line, STR_MENUFLIGHTMODES,
               GET_SET_INVERTED(g_eeGeneral.modelFMDisabled),
               g_model.modelFMDisabled);
#endif

#if defined(GVARS)
    line = body->newLine(grid);
    viewOption(line, STR_MENU_GLOBAL_VARS,
               GET_SET_INVERTED(g_eeGeneral.modelGVDisabled),
               g_model.modelGVDisabled);
#endif

    line = body->newLine(grid);
    viewOption(line, STR_MENUCURVES,
               GET_SET_INVERTED(g_eeGeneral.modelCurvesDisabled),
               g_model.modelCurvesDisabled);

    line = body->newLine(grid);
    viewOption(line, STR_MENULOGICALSWITCHES,
               GET_SET_INVERTED(g_eeGeneral.modelLSDisabled),
               g_model.modelLSDisabled);

    line = body->newLine(grid);
    viewOption(line, STR_MENUCUSTOMFUNC,
               GET_SET_INVERTED(g_eeGeneral.modelSFDisabled),
               g_model.modelSFDisabled);

#if defined(LUA_MODEL_SCRIPTS)
    line = body->newLine(grid);
    viewOption(line, STR_MENUCUSTOMSCRIPTS,
               GET_SET_INVERTED(g_eeGeneral.modelCustomScriptsDisabled),
               g_model.modelCustomScriptsDisabled);
#endif

    line = body->newLine(grid);
    viewOption(line, STR_MENUTELEMETRY,
               GET_SET_INVERTED(g_eeGeneral.modelTelemetryDisabled),
               g_model.modelTelemetryDisabled);
  }
};

class ManageModelsSetupPage : public SubPage
{
 public:
  ManageModelsSetupPage() : SubPage(ICON_MODEL, STR_MANAGE_MODELS)
  {
    FlexGridLayout grid(col_two_dsc, row_dsc, PAD_TINY);

    // Model quick select
    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_MODEL_QUICK_SELECT);
    new ToggleSwitch(line, rect_t{},
                     GET_SET_DEFAULT(g_eeGeneral.modelQuickSelect));

    // Label single/multi select
    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_LABELS_SELECT);
    new Choice(line, rect_t{}, STR_LABELS_SELECT_MODE, 0, 1,
               GET_DEFAULT(g_eeGeneral.labelSingleSelect),
               [=](int newValue) {
                 g_eeGeneral.labelSingleSelect = newValue;
                 modelslabels.clearFilter();
                 SET_DIRTY();
               });

    // Label multi select matching mode
    multiSelectMatch = body->newLine(grid);
    new StaticText(multiSelectMatch, rect_t{}, STR_LABELS_MATCH);
    new Choice(multiSelectMatch, rect_t{}, STR_LABELS_MATCH_MODE, 0, 1,
               GET_SET_DEFAULT(g_eeGeneral.labelMultiMode));

    // Favorites multi select matching mode
    favSelectMatch = body->newLine(grid);
    new StaticText(favSelectMatch, rect_t{}, STR_FAV_MATCH);
    new Choice(favSelectMatch, rect_t{}, STR_FAV_MATCH_MODE, 0, 1,
               GET_SET_DEFAULT(g_eeGeneral.favMultiMode));

    checkEvents();
  }

  void checkEvents() override
  {
    if (g_eeGeneral.labelSingleSelect) {
      lv_obj_add_flag(multiSelectMatch->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(favSelectMatch->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_clear_flag(multiSelectMatch->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      if (g_eeGeneral.labelMultiMode == 0)
        lv_obj_add_flag(favSelectMatch->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      else
        lv_obj_clear_flag(favSelectMatch->getLvObj(), LV_OBJ_FLAG_HIDDEN);
    }
  }

 protected:
  Window* multiSelectMatch = nullptr;
  Window* favSelectMatch = nullptr;
};

SetupLine::SetupLine(Window* parent, const rect_t& rect, const char* title, std::function<void(Window*, coord_t, coord_t)> createEdit, coord_t col2) :
    Window(parent, rect)
{
  padAll(PAD_ZERO);
  coord_t titleY = PAD_MEDIUM + 1;
  coord_t titleH = EdgeTxStyles::PAGE_LINE_HEIGHT;
  if (createEdit) {
    coord_t editY = PAD_TINY;
    if (title) {
      if (getTextWidth(title) >= RadioSetupPage::LBL_W) {
        setHeight(EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY * 2 + PAD_MEDIUM);
        titleY = 0;
        titleH = EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY;
        editY = PAD_SMALL + 1;
      } else {
        setHeight(EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY * 2);
      }
      new StaticText(this, {PAD_TINY, titleY, RadioSetupPage::LBL_W, titleH}, title);
    }
    createEdit(this, col2, editY);
  } else {
    new StaticText(this, {0, titleY, 0, titleH}, title, COLOR_THEME_PRIMARY1 | FONT(BOLD));
    setHeight(EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY * 2);
  }
}

static SetupLineDef setupLines[] = {
  {
    // Splash screen
    STR_SPLASHSCREEN,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(
          parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_SPLASHSCREEN_DELAYS, 0, 7,
          [=]() -> int32_t { return 3 - g_eeGeneral.splashMode; },
          [=](int32_t newValue) {
            g_eeGeneral.splashMode = 3 - newValue;
            SET_DIRTY();
          });
    }
  },
  {
    // PPM units
    STR_UNITS_PPM,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_PPMUNIT, PPM_PERCENT_PREC0, PPM_PERCENT_PREC1,
                GET_SET_DEFAULT(g_eeGeneral.ppmunit));
    }
  },
  {
    // Play startup sound
    STR_PLAY_HELLO,
    [](Window* parent, coord_t x, coord_t y) {
      new ToggleSwitch(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, GET_SET_INVERTED(g_eeGeneral.dontPlayHello));
    }
  },
#if defined(PWR_BUTTON_PRESS)
  {
    // Pwr Off Delay
    STR_PWR_OFF_DELAY,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(
          parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_PWR_OFF_DELAYS, 0, 3,
          [=]() -> int32_t { return 2 - g_eeGeneral.pwrOffSpeed; },
          [=](int32_t newValue) {
            g_eeGeneral.pwrOffSpeed = 2 - newValue;
            SET_DIRTY();
          });
    }
  },
#endif
#if defined(PXX2)
  {
    // Owner ID
    STR_OWNER_ID,
    [](Window* parent, coord_t x, coord_t y) {
      new RadioTextEdit(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, g_eeGeneral.ownerRegistrationID,
                        PXX2_LEN_REGISTRATION_ID);
    }
  },
#endif
  {
    // Country code
    STR_COUNTRY_CODE,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_COUNTRY_CODES, 0, 2,
                GET_SET_DEFAULT(g_eeGeneral.countryCode));
    }
  },
  {
    // Audio language
    STR_VOICE_LANGUAGE,
    [](Window* parent, coord_t x, coord_t y) {
      auto choice =
          new Choice(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 0, DIM(languagePacks) - 2,
                    GET_VALUE(currentLanguagePackIdx), [](uint8_t newValue) {
                      currentLanguagePackIdx = newValue;
                      currentLanguagePack = languagePacks[currentLanguagePackIdx];
                      strncpy(g_eeGeneral.ttsLanguage, currentLanguagePack->id, 2);
                      SET_DIRTY();
                    });
      choice->setTextHandler(
          [](uint8_t value) { return languagePacks[value]->name; });
    }
  },
  {
    // Imperial units
    STR_UNITS_SYSTEM,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_VUNITSSYSTEM, 0, 1,
                GET_SET_DEFAULT(g_eeGeneral.imperial));
    }
  },
  {
    // Switches delay
    STR_SWITCHES_DELAY,
    [](Window* parent, coord_t x, coord_t y) {
      auto edit =
          new NumberEdit(parent, {x, y, RadioSetupPage::NUM_W, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 0, 100,
                        GET_SET_VALUE_WITH_OFFSET(g_eeGeneral.switchesDelay, 15));
      edit->setDisplayHandler([](int32_t value) {
        return formatNumberAsString(value * 10, 0, 0, nullptr, STR_MS);
      });
    }
  },
  {
    // USB mode
    STR_USBMODE,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_USBMODES, USB_UNSELECTED_MODE, USB_MAX_MODE,
                GET_SET_DEFAULT(g_eeGeneral.USBMode));
    }
  },
#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  {
    STR_ROTARY_ENC_MODE,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_ROTARY_ENC_OPT, ROTARY_ENCODER_MODE_NORMAL,
                ROTARY_ENCODER_MODE_INVERT_BOTH,
                GET_SET_DEFAULT(g_eeGeneral.rotEncMode));
    }
  },
#endif
#if defined(USE_HATS_AS_KEYS)
  {
    STR_HATSMODE,
    [](Window* parent, coord_t x, coord_t y) {
      new Choice(parent, {x, y, 120, EdgeTxStyles::UI_ELEMENT_HEIGHT}, STR_HATSOPT, HATSMODE_TRIMS_ONLY,
                HATSMODE_SWITCHABLE, GET_SET_DEFAULT(g_eeGeneral.hatsMode));
      new TextButton(parent, {x + 120 + PAD_MEDIUM, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, "?", [=]() {
        new MessageDialog(parent, STR_HATSMODE_KEYS, STR_HATSMODE_KEYS_HELP, "",
                          LEFT);
        return 0;
      });
    }
  },
#endif
  {
    // RX channel order
    STR_DEF_CHAN_ORD,
    [](Window* parent, coord_t x, coord_t y) {
      uint8_t mains = adcGetMaxInputs(ADC_INPUT_MAIN);
      auto max_order = inputMappingGetMaxChannelOrder() - 1;
      auto choice = new Choice(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 0, max_order,
                          GET_SET_DEFAULT(g_eeGeneral.templateSetup));

      choice->setTextHandler([=](uint8_t value) {
        std::string s;
        for (uint8_t i = 0; i < mains; i++) {
          s += getAnalogShortLabel(inputMappingChannelOrder(value, i));
        }
        return s;
      });
    }
  },
  {
    // Stick mode
    STR_MODE,
    [](Window* parent, coord_t x, coord_t y) {
      auto choice = new Choice(parent, {x, y, 0, EdgeTxStyles::UI_ELEMENT_HEIGHT}, 0, 3, GET_DEFAULT(g_eeGeneral.stickMode),
                          [=](uint8_t newValue) {
                            mixerTaskStop();
                            g_eeGeneral.stickMode = newValue;
                            SET_DIRTY();
                            checkThrottleStick();
                            mixerTaskStart();
                          });
      choice->setTextHandler([](uint8_t value) {
        auto stick0 = inputMappingConvertMode(value, 0);
        auto stick1 = inputMappingConvertMode(value, 1);
        return std::to_string(1 + value) + ": " + STR_LEFT_STICK + " = " +
              std::string(getMainControlLabel(stick0)) + "+" +
              std::string(getMainControlLabel(stick1));
      });
    }
  },
};

RadioSetupPage::RadioSetupPage() : PageTab(STR_RADIO_SETUP, ICON_RADIO_SETUP, PAD_TINY) {}

void RadioSetupPage::build(Window* window)
{
  coord_t y = 0;
  Window * w;

  // Date & time picker including labels
  w = new DateTimeWindow(window, {0, y, LCD_W - padding * 2, EdgeTxStyles::UI_ELEMENT_HEIGHT * 2 + PAD_TINY * 2 + PAD_MEDIUM});

  y += w->height() + padding;

  // Sub-pages
  w = new SetupButtonGroup(window, {0, y, LCD_W - padding * 2, 0}, nullptr, BTN_COLS, PAD_TINY, {
    {STR_SOUND_LABEL, []() { new SoundPage(); }},
#if defined(VARIO)
    {STR_VARIO, []() { new VarioPage(); }},
#endif
#if defined(HAPTIC)
    {STR_HAPTIC_LABEL, []() { new HapticPage(); }},
#endif
    {STR_ALARMS_LABEL, []() { new AlarmsPage(); }},
    {STR_BACKLIGHT_LABEL, []() { new BacklightPage(); }},
    {STR_GPS, []() { new GpsPage(); }},
    {STR_ENABLED_FEATURES, []() { new ViewOptionsPage(); }},
    {STR_MAIN_MENU_MANAGE_MODELS, []() { new ManageModelsSetupPage(); }},
  });

  for (size_t i = 0; i < DIM(setupLines); i += 1) {
    y += w->height() + padding;
    w = new SetupLine(window, {0, y, LCD_W - padding * 2, 0}, setupLines[i].title, setupLines[i].createEdit, EDT_X);
  }
}
