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
#include "opentx.h"
#include "libopenui.h"
#include "input_mapping.h"

#include "tasks/mixer_task.h"
#include "hal/adc_driver.h"
#include "hal/usb_driver.h"

#define SET_DIRTY()     storageDirty(EE_GENERAL)

static const lv_coord_t col_two_dsc[] = {LV_GRID_FR(19), LV_GRID_FR(21),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t col_four_dsc[] = {LV_GRID_FR(19), LV_GRID_FR(7), LV_GRID_FR(7), LV_GRID_FR(7),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

class DateTimeWindow : public FormWindow {
  public:
    DateTimeWindow(Window* parent, const rect_t & rect) :
      FormWindow(parent, rect)
    {
      build();
    }

    void checkEvents() override
    {
      FormWindow::checkEvents();

      if (seconds && (get_tmr10ms() - lastRefresh >= 10)) {
        lastRefresh = get_tmr10ms();

        gettime(&m_tm);
        if (m_tm.tm_year != m_last_tm.tm_year) {
          m_last_tm.tm_year = m_tm.tm_year;
          year->update();
        }
        if (m_tm.tm_mon != m_last_tm.tm_mon) {
          m_last_tm.tm_mon = m_tm.tm_mon;
          month->update();
        }
        if (m_tm.tm_mday != m_last_tm.tm_mday) {
          m_last_tm.tm_mday = m_tm.tm_mday;
          day->update();
        }
        if (m_tm.tm_hour != m_last_tm.tm_hour) {
          m_last_tm.tm_hour = m_tm.tm_hour;
          hour->update();
        }
        if (m_tm.tm_min != m_last_tm.tm_min) {
          m_last_tm.tm_min = m_tm.tm_min;
          minutes->update();
        }
        if (m_tm.tm_sec != m_last_tm.tm_sec) {
          m_last_tm.tm_sec = m_tm.tm_sec;
          seconds->update();
        }
      }
    }

  protected:
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
      static const int8_t dmon[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
      int16_t year = TM_YEAR_BASE + m_tm.tm_year;
      int8_t days_in_month = dmon[m_tm.tm_mon];
      if ((m_tm.tm_mon == 1) && (((year % 4 == 0) && (year % 100 != 0)) || (year%400==0)))
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
      setFlexLayout();
      FlexGridLayout grid(col_four_dsc, row_dsc, 2);

      gettime(&m_tm);
      m_last_tm = m_tm;

      // Date
      auto line = newLine(&grid);
      new StaticText(line, rect_t{}, STR_DATE, 0, COLOR_THEME_PRIMARY1);
      year = new NumberEdit(line, rect_t{}, 2023, 2037,
                     [=]() -> int32_t {
                       return TM_YEAR_BASE + m_tm.tm_year;
                     },
                     [=](int32_t newValue) {
                       m_last_tm.tm_year = m_tm.tm_year = newValue - TM_YEAR_BASE;
                       setDaysInMonth();
                       SET_LOAD_DATETIME(&m_tm);
                     });
      lv_obj_set_style_grid_cell_x_align(year->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      month = new NumberEdit(line, rect_t{}, 1, 12,
                     [=]() -> int32_t {
                       return 1 + m_tm.tm_mon;
                     },
                     [=](int32_t newValue) {
                       m_last_tm.tm_mon = m_tm.tm_mon = newValue - 1;
                       setDaysInMonth();
                       SET_LOAD_DATETIME(&m_tm);
                     });
      month->setDisplayHandler([](int32_t value) {
        return formatNumberAsString(value, LEADING0);
      });
      lv_obj_set_style_grid_cell_x_align(month->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      day = new NumberEdit(line, rect_t{}, 1, daysInMonth(),
                     [=]() -> int32_t {
                       return m_tm.tm_mday;
                     },
                     [=](int32_t newValue) {
                       m_last_tm.tm_mday = m_tm.tm_mday = newValue;
                       SET_LOAD_DATETIME(&m_tm);
                     });
      day->setDisplayHandler([](int32_t value) {
        return formatNumberAsString(value, LEADING0, 2);
      });
      lv_obj_set_style_grid_cell_x_align(day->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      // Time
      line = newLine(&grid);
      new StaticText(line, rect_t{}, STR_TIME, 0, COLOR_THEME_PRIMARY1);
      hour = new NumberEdit(line, rect_t{}, 0, 23,
                     [=]() -> int32_t {
                       return m_tm.tm_hour;
                     },
                     [=](int32_t newValue) {
                       m_last_tm.tm_hour = m_tm.tm_hour = newValue;
                       SET_LOAD_DATETIME(&m_tm);
                     });
      hour->setDisplayHandler([](int32_t value) {
        return formatNumberAsString(value, LEADING0, 2);
      });
      lv_obj_set_style_grid_cell_x_align(hour->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      minutes = new NumberEdit(line, rect_t{}, 0, 59,
                     [=]() -> int32_t {
                       return m_tm.tm_min;
                     },
                     [=](int32_t newValue) {
                       m_last_tm.tm_min = m_tm.tm_min = newValue;
                       SET_LOAD_DATETIME(&m_tm);
                     });
      minutes->setDisplayHandler([](int32_t value) {
        return formatNumberAsString(value, LEADING0, 2);
      });
      lv_obj_set_style_grid_cell_x_align(minutes->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      seconds = new NumberEdit(line, rect_t{}, 0, 59,
                     [=]() -> int32_t {
                       return m_tm.tm_sec;
                     },
                     [=](int32_t newValue) {
                       m_last_tm.tm_sec = m_tm.tm_sec = newValue;
                       SET_LOAD_DATETIME(&m_tm);
                     });
      seconds->setDisplayHandler([](int value) {
        return formatNumberAsString(value, LEADING0, 2);
      });
      lv_obj_set_style_grid_cell_x_align(seconds->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      // Spacer
      line = newLine(&grid);
    }
};

class WindowButtonGroup : public FormWindow
{
 public:
  typedef std::function<void()>           PageFct;
  typedef std::pair<const char*, PageFct> PageDef;
  typedef std::list<PageDef>              PageDefs;

  WindowButtonGroup(
      Window* parent, const rect_t& rect, PageDefs pages) :
      FormWindow(parent, rect),
      pages(pages)
  {
    padBottom(4);
    setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
    lv_obj_set_style_flex_main_place(lvobj, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    padRow(lv_dpx(8));
    padBottom(4);

    for (auto& entry : pages) {
      auto btn = new TextButton(this, rect_t{}, entry.first, [&, entry]() {
        entry.second();
        return 0;
      });
      lv_obj_set_style_min_width(btn->getLvObj(), LV_DPI_DEF, 0);
    }
  }

 protected:
  PageDefs pages;
};

class SubPage : public Page
{
  public:
    SubPage(MenuIcons icon, const char* title, bool isFlex = true) : Page(icon)
    {
      header.setTitle(STR_RADIO_SETUP);
      header.setTitle2(title);

      if (isFlex)
        body.setFlexLayout();

      body.padAll(8);
    }
};

class SoundPage : public SubPage {
  public:
    SoundPage() : SubPage(ICON_RADIO_SETUP, STR_SOUND_LABEL, false)
    {
      FlexGridLayout grid(col_two_dsc, row_dsc, 2);

      auto form = new FormWindow(&body, rect_t{});
      form->setFlexLayout();
      form->padAll(0);

      auto line = form->newLine(&grid);

      // Beeps mode
      new StaticText(line, rect_t{}, STR_SPEAKER, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_VBEEPMODE, -2, 1, GET_SET_DEFAULT(g_eeGeneral.beepMode));
      line = form->newLine(&grid);

      // Main volume
      new StaticText(line, rect_t{}, STR_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, lv_pct(50), -VOLUME_LEVEL_DEF, VOLUME_LEVEL_MAX-VOLUME_LEVEL_DEF, GET_SET_DEFAULT(g_eeGeneral.speakerVolume));
      line = form->newLine(&grid);

      // Beeps volume
      new StaticText(line, rect_t{}, STR_BEEP_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, lv_pct(50), -2, +2, GET_SET_DEFAULT(g_eeGeneral.beepVolume));
      line = form->newLine(&grid);

      // Beeps length
      new StaticText(line, rect_t{}, STR_BEEP_LENGTH, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, lv_pct(50), -2, +2, GET_SET_DEFAULT(g_eeGeneral.beepLength));
      line = form->newLine(&grid);

      // Beeps pitch
      new StaticText(line, rect_t{}, STR_BEEP_PITCH, 0, COLOR_THEME_PRIMARY1);
      auto edit = new NumberEdit(line, rect_t{}, 0, 300,
                                 GET_DEFAULT(15 * g_eeGeneral.speakerPitch),
                                 [=](int32_t newValue) {
                                     g_eeGeneral.speakerPitch = newValue / 15;
                                     SET_DIRTY();
                                 });
      edit->setStep(15);
      edit->setPrefix("+");
      edit->setSuffix("Hz");
      line = form->newLine(&grid);

      // Wav volume
      new StaticText(line, rect_t{}, STR_WAV_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, lv_pct(50), -2, +2, GET_SET_DEFAULT(g_eeGeneral.wavVolume));
      line = form->newLine(&grid);

      // Background volume
      new StaticText(line, rect_t{}, STR_BG_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, lv_pct(50), -2, +2, GET_SET_DEFAULT(g_eeGeneral.backgroundVolume));

    }
};

#if defined(VARIO)
class VarioPage : public SubPage {
  public:
    VarioPage() : SubPage(ICON_RADIO_SETUP, STR_VARIO)
    {
      FlexGridLayout grid(col_two_dsc, row_dsc, 4);

      auto line = body.newLine(&grid);

      // Vario volume
      new StaticText(line, rect_t{}, STR_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, lv_pct(50), -2, +2, GET_SET_DEFAULT(g_eeGeneral.varioVolume));
      line = body.newLine(&grid);

      new StaticText(line, rect_t{}, STR_PITCH_AT_ZERO, 0, COLOR_THEME_PRIMARY1);
      auto edit = new NumberEdit(line, rect_t{}, VARIO_FREQUENCY_ZERO - 400, VARIO_FREQUENCY_ZERO + 400,
                            GET_DEFAULT(VARIO_FREQUENCY_ZERO + (g_eeGeneral.varioPitch * 10)),
                            SET_VALUE(g_eeGeneral.varioPitch, (newValue - VARIO_FREQUENCY_ZERO) / 10));
      edit->setStep(10);
      edit->setSuffix("Hz");
      line = body.newLine(&grid);

      new StaticText(line, rect_t{}, STR_PITCH_AT_MAX, 0, COLOR_THEME_PRIMARY1);
      edit = new NumberEdit(line, rect_t{}, 900, 2500,
                            GET_DEFAULT(VARIO_FREQUENCY_ZERO + (g_eeGeneral.varioPitch * 10) + VARIO_FREQUENCY_RANGE + (g_eeGeneral.varioRange * 10)),
                            SET_VALUE(g_eeGeneral.varioRange, (newValue - VARIO_FREQUENCY_ZERO - VARIO_FREQUENCY_RANGE) / 10 - g_eeGeneral.varioPitch ));
      edit->setStep(10);
      edit->setSuffix("Hz");
      line = body.newLine(&grid);

      new StaticText(line, rect_t{}, STR_REPEAT_AT_ZERO, 0, COLOR_THEME_PRIMARY1);
      edit = new NumberEdit(line, rect_t{}, 200, 1000,
                            GET_DEFAULT(VARIO_REPEAT_ZERO + (g_eeGeneral.varioRepeat * 10)),
                            SET_VALUE(g_eeGeneral.varioRepeat, (newValue - VARIO_REPEAT_ZERO) / 10));
      edit->setStep(10);
      edit->setSuffix("ms");
      line = body.newLine(&grid);
    }
};
#endif

#if defined(HAPTIC)
class HapticPage : public SubPage {
  public:
    HapticPage() : SubPage(ICON_RADIO_SETUP, STR_HAPTIC_LABEL)
    {
      FlexGridLayout grid(col_two_dsc, row_dsc, 4);

      auto line = body.newLine(&grid);

      // Haptic mode
      new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_VBEEPMODE, -2, 1, GET_SET_DEFAULT(g_eeGeneral.hapticMode));
      line = body.newLine(&grid);

      // Haptic duration
      new StaticText(line, rect_t{}, STR_LENGTH, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, lv_pct(50), -2, +2, GET_SET_DEFAULT(g_eeGeneral.hapticLength));
      line = body.newLine(&grid);

      // Haptic strength
      new StaticText(line, rect_t{}, STR_STRENGTH, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, lv_pct(50), -2, +2, GET_SET_DEFAULT(g_eeGeneral.hapticStrength));
      line = body.newLine(&grid);
    }
};
#endif

class AlarmsPage : public SubPage {
  public:
    AlarmsPage() : SubPage(ICON_RADIO_SETUP, STR_ALARMS_LABEL)
    {
      FlexGridLayout grid(col_two_dsc, row_dsc, 4);

      auto line = body.newLine(&grid);
      // Battery warning
      new StaticText(line, rect_t{}, STR_BATTERYWARNING, 0, COLOR_THEME_PRIMARY1);
      auto edit = new NumberEdit(line, rect_t{}, 30, 120, GET_SET_DEFAULT(g_eeGeneral.vBatWarn), 0, PREC1);
      edit->setSuffix("V");
      line = body.newLine(&grid);

      // Inactivity alarm
      new StaticText(line, rect_t{}, STR_INACTIVITYALARM, 0, COLOR_THEME_PRIMARY1);
      edit = new NumberEdit(line, rect_t{}, 0, 250, GET_SET_DEFAULT(g_eeGeneral.inactivityTimer));

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
      line = body.newLine(&grid);

      // Alarms warning
      new StaticText(line, rect_t{}, STR_ALARMWARNING, 0, COLOR_THEME_PRIMARY1);
      new ToggleSwitch(line, rect_t{},
                   GET_SET_INVERTED(g_eeGeneral.disableAlarmWarning));
      line = body.newLine(&grid);

      // RSSI shutdown alarm
      new StaticText(line, rect_t{}, STR_RSSI_SHUTDOWN_ALARM, 0,
                     COLOR_THEME_PRIMARY1);
      new ToggleSwitch(line, rect_t{},
                   GET_SET_INVERTED(g_eeGeneral.disableRssiPoweroffAlarm));
      line = body.newLine(&grid);
    }
};

class BacklightPage : public SubPage {
  public:
    BacklightPage() : SubPage(ICON_RADIO_SETUP, STR_BACKLIGHT_LABEL)
    {
      FlexGridLayout grid(col_two_dsc, row_dsc, 4);

      auto line = body.newLine(&grid);

      // Backlight mode
      new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);

      auto blMode = new Choice(line, rect_t{}, STR_VBLMODE,
                               e_backlight_mode_off, e_backlight_mode_on,
                               GET_DEFAULT(g_eeGeneral.backlightMode),
                               [=](int32_t newValue) {
                                 g_eeGeneral.backlightMode = newValue;
                                 updateBacklightControls();
                                 SET_DIRTY();
                               });

      blMode->setAvailableHandler(
          [=](int newValue) { return newValue != e_backlight_mode_off; });

      backlightTimeout = body.newLine(&grid);

      // Delay
      new StaticText(backlightTimeout, rect_t{}, STR_BACKLIGHT_TIMER, 0, COLOR_THEME_PRIMARY1);
      auto edit = new NumberEdit(backlightTimeout, rect_t{}, 5, 600,
                                 GET_DEFAULT(g_eeGeneral.lightAutoOff * 5),
                                 SET_VALUE(g_eeGeneral.lightAutoOff, newValue / 5));
      edit->setStep(5);
      edit->setSuffix("s");

      backlightOnBright = body.newLine(&grid);

      // Backlight ON bright
      new StaticText(backlightOnBright, rect_t{}, STR_BLONBRIGHTNESS, 0, COLOR_THEME_PRIMARY1);
      backlightOnSlider = new Slider(backlightOnBright, lv_pct(50), BACKLIGHT_LEVEL_MIN, BACKLIGHT_LEVEL_MAX,
                 [=]() -> int32_t {
                   return BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
                 },
                 [=](int32_t newValue) {
                   if(newValue >= g_eeGeneral.blOffBright || g_eeGeneral.backlightMode == e_backlight_mode_on) {
                     g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - newValue;
                   } else {
                     g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.blOffBright;
                     backlightOnSlider->update();
                   }
                   SET_DIRTY();
                 });

      backlightOffBright = body.newLine(&grid);

      // Backlight OFF bright
      new StaticText(backlightOffBright, rect_t{}, STR_BLOFFBRIGHTNESS, 0, COLOR_THEME_PRIMARY1);
      backlightOffSlider = new Slider(backlightOffBright, lv_pct(50), BACKLIGHT_LEVEL_MIN, BACKLIGHT_LEVEL_MAX, GET_DEFAULT(g_eeGeneral.blOffBright),
                 [=](int32_t newValue) {
                   int32_t onBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
                   if(newValue <= onBright || g_eeGeneral.backlightMode == e_backlight_mode_off) {
                     g_eeGeneral.blOffBright = newValue;
                   } else {
                     g_eeGeneral.blOffBright = onBright;
                     backlightOffSlider->update();
                   }
                   SET_DIRTY();
                 });

      line = body.newLine(&grid);

  #if defined(KEYS_BACKLIGHT_GPIO)
      // Keys backlight
      new StaticText(line, rect_t{}, STR_KEYS_BACKLIGHT, 0, COLOR_THEME_PRIMARY1);
      new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.keysBacklight));
      line = body.newLine(&grid);
  #endif

      // Flash beep
      new StaticText(line, rect_t{}, STR_ALARM, 0, COLOR_THEME_PRIMARY1);
      new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.alarmsFlash));
      line = body.newLine(&grid);

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
      switch(g_eeGeneral.backlightMode)
      {
      case e_backlight_mode_off:
        lv_obj_add_flag(backlightTimeout->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(backlightOnBright->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(backlightOffBright->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        break;
      case e_backlight_mode_keys:
      case e_backlight_mode_sticks:
      case e_backlight_mode_all:
      default:
      {
        lv_obj_clear_flag(backlightTimeout->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(backlightOnBright->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(backlightOffBright->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        int32_t onBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
        if(onBright < g_eeGeneral.blOffBright)
          g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.blOffBright;
        break;
      }
      case e_backlight_mode_on:
        lv_obj_add_flag(backlightTimeout->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(backlightOnBright->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(backlightOffBright->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        break;
      }
      resetBacklightTimeout();
    }
};

class GpsPage : public SubPage {
  public:
    GpsPage() : SubPage(ICON_RADIO_SETUP, STR_GPS)
    {
      FlexGridLayout grid(col_two_dsc, row_dsc, 4);

      tzIndex = timezoneIndex(g_eeGeneral.timezone, g_eeGeneral.timezoneMinutes);

      auto line = body.newLine(&grid);
      // Timezone
      new StaticText(line, rect_t{}, STR_TIMEZONE, 0, COLOR_THEME_PRIMARY1);
      auto tz = new NumberEdit(line, rect_t{}, minTimezone(), maxTimezone(),
                               GET_DEFAULT(tzIndex),
                               [=](int newTz) {
                                 tzIndex = newTz;
                                 g_eeGeneral.timezone = timezoneHour(newTz);
                                 g_eeGeneral.timezoneMinutes = timezoneMinute(newTz);
                                 SET_DIRTY();
                               });
      tz->setDisplayHandler([](int32_t tz) {
        return timezoneDisplay(tz);
      });
      line = body.newLine(&grid);

      // Adjust RTC (from telemetry)
      new StaticText(line, rect_t{}, STR_ADJUST_RTC, 0, COLOR_THEME_PRIMARY1);
      new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.adjustRTC));
      line = body.newLine(&grid);

      // GPS format
      new StaticText(line, rect_t{}, STR_GPS_COORDS_FORMAT, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_GPSFORMAT, 0, 1, GET_SET_DEFAULT(g_eeGeneral.gpsFormat));
      line = body.newLine(&grid);
    }

  protected:
    int tzIndex;
};

class ViewOptionsPage : public SubPage
{
   private:
    const lv_coord_t opt_col_dsc[4] = {LV_GRID_FR(5), LV_GRID_FR(2), LV_GRID_FR(4), LV_GRID_TEMPLATE_LAST};

    void viewOption(FormWindow::Line *line, const char* name, std::function<uint8_t()> getValue, std::function<void(uint8_t)> setValue, uint8_t modelOption)
    {
      line->padLeft(10);
      new StaticText(line, rect_t{}, name, 0, COLOR_THEME_PRIMARY1);
      new ToggleSwitch(line, rect_t{}, getValue, setValue);
      if (modelOption != OVERRIDE_GLOBAL) {
        std::string s(STR_MODEL);
        s += " - ";
        s += STR_ADCFILTERVALUES[modelOption];
        new StaticText(line, rect_t{}, s.c_str(), 0, COLOR_THEME_SECONDARY1);
      }
    }

  public:
    ViewOptionsPage() : SubPage(ICON_RADIO_SETUP, STR_ENABLED_FEATURES, false)
    {
      FlexGridLayout grid(opt_col_dsc, row_dsc, 2);

      auto form = new FormWindow(&body, rect_t{});
      form->setFlexLayout();
      form->padAll(0);

      auto line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_RADIO_MENU_TABS, 0, COLOR_THEME_PRIMARY1);

      line = form->newLine(&grid);
      viewOption(line, STR_THEME_EDITOR, GET_SET_INVERTED(g_eeGeneral.radioThemesDisabled), g_model.radioThemesDisabled);

      line = form->newLine(&grid);
      viewOption(line, STR_MENUSPECIALFUNCS, GET_SET_INVERTED(g_eeGeneral.radioGFDisabled), g_model.radioGFDisabled);

      line = form->newLine(&grid);
      viewOption(line, STR_MENUTRAINER, GET_SET_INVERTED(g_eeGeneral.radioTrainerDisabled), g_model.radioTrainerDisabled);

      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_MODEL_MENU_TABS, 0, COLOR_THEME_PRIMARY1);

#if defined(HELI)
      line = form->newLine(&grid);
      viewOption(line, STR_MENUHELISETUP, GET_SET_INVERTED(g_eeGeneral.modelHeliDisabled), g_model.modelHeliDisabled);
#endif

#if defined(FLIGHT_MODES)
      line = form->newLine(&grid);
      viewOption(line, STR_MENUFLIGHTMODES, GET_SET_INVERTED(g_eeGeneral.modelFMDisabled), g_model.modelFMDisabled);
#endif

#if defined(GVARS)
      line = form->newLine(&grid);
      viewOption(line, STR_MENU_GLOBAL_VARS, GET_SET_INVERTED(g_eeGeneral.modelGVDisabled), g_model.modelGVDisabled);
#endif

      line = form->newLine(&grid);
      viewOption(line, STR_MENUCURVES, GET_SET_INVERTED(g_eeGeneral.modelCurvesDisabled), g_model.modelCurvesDisabled);

      line = form->newLine(&grid);
      viewOption(line, STR_MENULOGICALSWITCHES, GET_SET_INVERTED(g_eeGeneral.modelLSDisabled), g_model.modelLSDisabled);

      line = form->newLine(&grid);
      viewOption(line, STR_MENUCUSTOMFUNC, GET_SET_INVERTED(g_eeGeneral.modelSFDisabled), g_model.modelSFDisabled);

#if defined(LUA_MODEL_SCRIPTS)
      line = form->newLine(&grid);
      viewOption(line, STR_MENUCUSTOMSCRIPTS, GET_SET_INVERTED(g_eeGeneral.modelCustomScriptsDisabled), g_model.modelCustomScriptsDisabled);
#endif

      line = form->newLine(&grid);
      viewOption(line, STR_MENUTELEMETRY, GET_SET_INVERTED(g_eeGeneral.modelTelemetryDisabled), g_model.modelTelemetryDisabled);
    }
};

RadioSetupPage::RadioSetupPage():
  PageTab(STR_RADIO_SETUP, ICON_RADIO_SETUP)
{
}

void RadioSetupPage::build(FormWindow * window)
{
  FlexGridLayout grid(col_two_dsc, row_dsc, 2);
  window->setFlexLayout();

  // Date & time picker including labels
  new DateTimeWindow(window, rect_t{});

  // Sub-pages
  new WindowButtonGroup(window, rect_t{}, {
      {STR_SOUND_LABEL, []() { new SoundPage(); }},
#if defined(VARIO)
      {STR_VARIO, []() { new VarioPage(); }},
#endif
#if defined(HAPTIC)
      {STR_HAPTIC_LABEL, []() { new HapticPage(); }},
#endif
      {STR_ALARMS_LABEL, []() { new AlarmsPage(); }},
      {STR_BACKLIGHT_LABEL, []() { new BacklightPage(); }},
      {STR_GPS, [](){new GpsPage();}},
      {STR_ENABLED_FEATURES, [](){new ViewOptionsPage();}},
});

  // Splash screen
    auto line = window->newLine(&grid);
    new StaticText(line, rect_t{}, STR_SPLASHSCREEN, 0, COLOR_THEME_PRIMARY1);
    new Choice(line, rect_t{}, STR_SPLASHSCREEN_DELAYS, 0, 7,
               [=]() -> int32_t {
               return 3 - g_eeGeneral.splashMode;
               },
               [=](int32_t newValue) {
                   g_eeGeneral.splashMode = 3 - newValue;
                   SET_DIRTY();
               });

  // Play startup sound
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_PLAY_HELLO, 0, COLOR_THEME_PRIMARY1);
  new ToggleSwitch(line, rect_t{}, GET_SET_INVERTED(g_eeGeneral.dontPlayHello));

#if defined(PWR_BUTTON_PRESS)
  // Pwr Off Delay
  {
    line = window->newLine(&grid);
    new StaticText(line, rect_t{}, STR_PWR_OFF_DELAY, 0, COLOR_THEME_PRIMARY1);
    new Choice(line, rect_t{}, STR_PWR_OFF_DELAYS, 0, 3,
               [=]() -> int32_t {
               return 2 - g_eeGeneral.pwrOffSpeed;
               },
               [=](int32_t newValue) {
                   g_eeGeneral.pwrOffSpeed = 2 - newValue;
                   SET_DIRTY();
               });
  }
#endif

#if defined(PXX2)
  // Owner ID
  {
    line = window->newLine(&grid);
    new StaticText(line, rect_t{}, STR_OWNER_ID, 0, COLOR_THEME_PRIMARY1);
    new RadioTextEdit(line, rect_t{}, g_eeGeneral.ownerRegistrationID,
                      PXX2_LEN_REGISTRATION_ID);
  }
#endif

  // Country code
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_COUNTRY_CODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_COUNTRY_CODES, 0, 2, GET_SET_DEFAULT(g_eeGeneral.countryCode));

  // Audio language
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_VOICE_LANGUAGE, 0, COLOR_THEME_PRIMARY1);
  auto choice =
      new Choice(line, rect_t{}, 0, DIM(languagePacks) - 2,
                 GET_VALUE(currentLanguagePackIdx), [](uint8_t newValue) {
                   currentLanguagePackIdx = newValue;
                   currentLanguagePack = languagePacks[currentLanguagePackIdx];
                   strncpy(g_eeGeneral.ttsLanguage, currentLanguagePack->id, 2);
                   SET_DIRTY();
                 });
  choice->setTextHandler(
      [](uint8_t value) { return languagePacks[value]->name; });

  // Imperial units
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_UNITS_SYSTEM, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VUNITSSYSTEM, 0, 1, GET_SET_DEFAULT(g_eeGeneral.imperial));

  // PPM units
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_UNITS_PPM, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_PPMUNIT, PPM_PERCENT_PREC0, PPM_PERCENT_PREC1, GET_SET_DEFAULT(g_eeGeneral.ppmunit));

#if defined(FAI_CHOICE)
/*  case ITEM_SETUP_FAI:
    lcdDrawText(MENUS_MARGIN_LEFT, y, "FAI Mode");
    if (g_eeGeneral.fai) {
      lcdDrawText(RADIO_SETUP_2ND_COLUMN, y, "Locked in FAI Mode");
    }
    else {
      g_eeGeneral.fai = editCheckBox(g_eeGeneral.fai, RADIO_SETUP_2ND_COLUMN, y, attr, event);
      if (attr && checkIncDec_Ret) {
          g_eeGeneral.fai = false;
          POPUP_CONFIRMATION("FAI mode?");
      }
    }
    break;*/
#endif

  // Switches delay
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_SWITCHES_DELAY, 0, COLOR_THEME_PRIMARY1);
  auto edit =
      new NumberEdit(line, rect_t{}, 0, 100,
                     GET_SET_VALUE_WITH_OFFSET(g_eeGeneral.switchesDelay, 15));
  edit->setDisplayHandler([](int32_t value) {
    return formatNumberAsString(value * 10, 0, 0, nullptr, STR_MS);
  });

  // USB mode
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_USBMODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_USBMODES, USB_UNSELECTED_MODE, USB_MAX_MODE,
             GET_SET_DEFAULT(g_eeGeneral.USBMode));

#if defined(ROTARY_ENCODER_NAVIGATION) && !defined(USE_HATS_AS_KEYS)
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_ROTARY_ENC_MODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ROTARY_ENC_OPT, ROTARY_ENCODER_MODE_NORMAL,
             ROTARY_ENCODER_MODE_INVERT_BOTH,
             GET_SET_DEFAULT(g_eeGeneral.rotEncMode));
#endif

#if defined(USE_HATS_AS_KEYS)
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_HATSMODE, 0, COLOR_THEME_PRIMARY1);
  auto box = new FormWindow(line, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, 4);
  new Choice(box, rect_t{}, STR_HATSOPT, HATSMODE_TRIMS_ONLY,
             HATSMODE_SWITCHABLE, GET_SET_DEFAULT(g_eeGeneral.hatsMode));
  new TextButton(box, rect_t{}, "?", [=]() {
    new HelpDialog(window, {50, 100, LCD_W - 100, LCD_H - 200},
                   STR_HATSMODE_KEYS, STR_HATSMODE_KEYS_HELP, LEFT);
    return 0;
  });
#endif

  // RX channel order
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_DEF_CHAN_ORD, 0,
                 COLOR_THEME_PRIMARY1);  // RAET->AETR

  uint8_t mains = adcGetMaxInputs(ADC_INPUT_MAIN);
  auto max_order = inputMappingGetMaxChannelOrder() - 1;
  choice = new Choice(line, rect_t{}, 0, max_order,
                      GET_SET_DEFAULT(g_eeGeneral.templateSetup));

  choice->setTextHandler([=](uint8_t value) {
    std::string s;
    for (uint8_t i = 0; i < mains; i++) {
      s += getAnalogShortLabel(inputMappingChannelOrder(value, i));
    }
    return s;
  });

  // Stick mode
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);
  choice = new Choice(line, rect_t{}, 0, 3, GET_DEFAULT(g_eeGeneral.stickMode),
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

  // Model quick select
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODEL_QUICK_SELECT, 0, COLOR_THEME_PRIMARY1);
  new ToggleSwitch(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.modelQuickSelect));
}
