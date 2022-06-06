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

#define SET_DIRTY()     storageDirty(EE_GENERAL)

class DateTimeWindow : public FormGroup {
  public:
    DateTimeWindow(FormGroup * parent, const rect_t & rect) :
      FormGroup(parent, rect, FORWARD_SCROLL | FORM_FORWARD_FOCUS)
    {
      build();
    }

    void checkEvents() override
    {
      FormGroup::checkEvents();

      if (get_tmr10ms() - lastRefresh > 100) {
        invalidate();
        lastRefresh = get_tmr10ms();
      }
    }

  protected:
    tmr10ms_t lastRefresh = 0;

    void build()
    {
      FormGridLayout grid;

      // Date
      new StaticText(this, grid.getLabelSlot(), STR_DATE, 0, COLOR_THEME_PRIMARY1);
      new NumberEdit(this, grid.getFieldSlot(3, 0), 2018, 2100,
                     [=]() -> int32_t {
                       struct gtm t;
                       gettime(&t);
                       return TM_YEAR_BASE + t.tm_year;
                     },
                     [=](int32_t newValue) {
                       struct gtm t;
                       gettime(&t);
                       t.tm_year = newValue - TM_YEAR_BASE;
                       SET_LOAD_DATETIME(&t);
                     });
      auto month = new NumberEdit(this, grid.getFieldSlot(3, 1), 1, 12,
                                  [=]() -> int32_t {
                                    struct gtm t;
                                    gettime(&t);
                                    return 1 + t.tm_mon;
                                  },
                                  [=](int32_t newValue) {
                                    struct gtm t;
                                    gettime(&t);
                                    t.tm_mon = newValue - 1;
                                    SET_LOAD_DATETIME(&t);
                                  });
      month->setDisplayHandler([](int32_t value) {
        return formatNumberAsString(value, LEADING0);
      });

      /* TODO dynamic max instead of 31 ...
      int16_t year = TM_YEAR_BASE + t.tm_year;
      int8_t dlim = (((((year%4==0) && (year%100!=0)) || (year%400==0)) && (t.tm_mon==1)) ? 1 : 0);
      static const pm_uint8_t dmon[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
      dlim += *(&dmon[t.tm_mon]);*/
      int8_t dlim = 31;
      auto day = new NumberEdit(this, grid.getFieldSlot(3, 2), 1, dlim,
                                [=]() -> int32_t {
                                  struct gtm t;
                                  gettime(&t);
                                  return t.tm_mday;
                                },
                                [=](int32_t newValue) {
                                  struct gtm t;
                                  gettime(&t);
                                  t.tm_mday = newValue;
                                  SET_LOAD_DATETIME(&t);
                                });
      day->setDisplayHandler([](int32_t value) {
        return formatNumberAsString(value, LEADING0, 2);
      });
      grid.nextLine();

      // Time
      new StaticText(this, grid.getLabelSlot(), STR_TIME, 0, COLOR_THEME_PRIMARY1);
      auto hour = new NumberEdit(this, grid.getFieldSlot(3, 0), 0, 24,
                                 [=]() -> int32_t {
                                   struct gtm t;
                                   gettime(&t);
                                   return t.tm_hour;
                                 },
                                 [=](int32_t newValue) {
                                   struct gtm t;
                                   gettime(&t);
                                   t.tm_hour = newValue;
                                   SET_LOAD_DATETIME(&t);
                                 });
      hour->setDisplayHandler([](int32_t value) {
        static char s[50];
        BitmapBuffer::formatNumberAsString(s, 49, value, LEADING0, 2);
        return std::string(s);
        // dc->drawNumber(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, value, flags | LEADING0, 2);
      });

      auto minutes = new NumberEdit(this, grid.getFieldSlot(3, 1), 0, 59,
                                    [=]() -> int32_t {
                                      struct gtm t;
                                      gettime(&t);
                                      return t.tm_min;
                                    },
                                    [=](int32_t newValue) {
                                      struct gtm t;
                                      gettime(&t);
                                      t.tm_min = newValue;
                                      SET_LOAD_DATETIME(&t);
                                    });
      minutes->setDisplayHandler([](int32_t value) {
        return formatNumberAsString(value, LEADING0, 2);
      });

      auto seconds = new NumberEdit(this, grid.getFieldSlot(3, 2), 0, 59,
                                    [=]() -> int32_t {
                                      struct gtm t;
                                      gettime(&t);
                                      return t.tm_sec;
                                    },
                                    [=](int32_t newValue) {
                                      struct gtm t;
                                      gettime(&t);
                                      t.tm_sec = newValue;
                                      SET_LOAD_DATETIME(&t);
                                    });
      seconds->setDisplayHandler([](int value) {
        return std::to_string(value);
      });
    }
};

class WindowButtonGroup : public FormGroup
{
 public:
  WindowButtonGroup(
      Window* parent, const rect_t& rect,
      std::vector<std::pair<const char*, std::function<void()>>> windows) :
      FormGroup(parent, rect, FORWARD_SCROLL | FORM_FORWARD_FOCUS),
      windows(windows)
  {
    build();
  }

 protected:
  std::vector<std::pair<const char*, std::function<void()>>> windows;

  void build()
  {
    FormGridLayout grid;

    for (auto entry : windows) {
      new TextButton(this, grid.getLabelSlot(), entry.first, [&, entry]() {
        entry.second();
        return 0;
      });
      grid.nextLine();
    }
  }
};

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};
static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

class SoundPage : public Page {
  public:
  SoundPage() :
      Page(ICON_RADIO_SETUP)
    {
      build();
    }

  protected:

    void build()
    {
      new StaticText(&header,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                      STR_SOUND_LABEL, 0, COLOR_THEME_PRIMARY2);

      body.setFlexLayout();
      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

      auto line = body.newLine(&grid);

      // Beeps mode
      new StaticText(line, rect_t{}, STR_SPEAKER, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_VBEEPMODE, -2, 1, GET_SET_DEFAULT(g_eeGeneral.beepMode));
      line = body.newLine(&grid);

      // Main volume
      new StaticText(line, rect_t{}, STR_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, -VOLUME_LEVEL_DEF, VOLUME_LEVEL_MAX-VOLUME_LEVEL_DEF, GET_SET_DEFAULT(g_eeGeneral.speakerVolume));

      line = body.newLine(&grid);

      // Beeps volume
      new StaticText(line, rect_t{}, STR_BEEP_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, -2, +2, GET_SET_DEFAULT(g_eeGeneral.beepVolume));
      line = body.newLine(&grid);

      // Beeps length
      new StaticText(line, rect_t{}, STR_BEEP_LENGTH, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, -2, +2, GET_SET_DEFAULT(g_eeGeneral.beepLength));
      line = body.newLine(&grid);

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
      line = body.newLine(&grid);

      // Wav volume
      new StaticText(line, rect_t{}, STR_WAV_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, -2, +2, GET_SET_DEFAULT(g_eeGeneral.wavVolume));
      line = body.newLine(&grid);

      // Background volume
      new StaticText(line, rect_t{}, STR_BG_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, -2, +2, GET_SET_DEFAULT(g_eeGeneral.backgroundVolume));

    }
};

#if defined(VARIO)
class VarioPage : public Page {
  public:
  VarioPage() :
      Page(ICON_RADIO_SETUP)
    {
      build();
    }

  protected:

    void build()
    {
      new StaticText(&header,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                      STR_VARIO, 0, COLOR_THEME_PRIMARY2);

      body.setFlexLayout();
      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

      auto line = body.newLine(&grid);

      // Vario volume
      new StaticText(line, rect_t{}, TR_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, -2, +2, GET_SET_DEFAULT(g_eeGeneral.varioVolume));
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
class HapticPage : public Page {
  public:
	HapticPage() :
      Page(ICON_RADIO_SETUP)
    {
      build();
    }

  protected:

    void build()
    {
      new StaticText(&header,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
					  STR_HAPTIC_LABEL, 0, COLOR_THEME_PRIMARY2);

      body.setFlexLayout();
      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

      auto line = body.newLine(&grid);

      // Haptic mode
      new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_VBEEPMODE, -2, 1, GET_SET_DEFAULT(g_eeGeneral.hapticMode));
      line = body.newLine(&grid);

      // Haptic duration
      new StaticText(line, rect_t{}, STR_LENGTH, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, -2, +2, GET_SET_DEFAULT(g_eeGeneral.hapticLength));
      line = body.newLine(&grid);

      // Haptic strength
      new StaticText(line, rect_t{}, STR_STRENGTH, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, -2, +2, GET_SET_DEFAULT(g_eeGeneral.hapticStrength));
      line = body.newLine(&grid);
    }
};
#endif

class AlarmsPage : public Page {
  public:
	AlarmsPage() :
      Page(ICON_RADIO_SETUP)
    {
      build();
    }

  protected:

    void build()
    {
      new StaticText(&header,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
					  STR_ALARMS_LABEL, 0, COLOR_THEME_PRIMARY2);

      body.setFlexLayout();
      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

      auto line = body.newLine(&grid);
      // Battery warning
      new StaticText(line, rect_t{}, STR_BATTERYWARNING, 0, COLOR_THEME_PRIMARY1);
      auto edit = new NumberEdit(line, rect_t{}, 30, 120, GET_SET_DEFAULT(g_eeGeneral.vBatWarn), 0, PREC1);
      edit->setSuffix("V");
      line = body.newLine(&grid);

      // Inactivity alarm
      new StaticText(line, rect_t{}, STR_INACTIVITYALARM, 0, COLOR_THEME_PRIMARY1);
      edit = new NumberEdit(line, rect_t{}, 0, 250, GET_SET_DEFAULT(g_eeGeneral.inactivityTimer));
      edit->setSuffix("minutes");
      line = body.newLine(&grid);

      // Alarms warning
      new StaticText(line, rect_t{}, STR_ALARMWARNING, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{}, GET_SET_INVERTED(g_eeGeneral.disableAlarmWarning));
      line = body.newLine(&grid);

      // RSSI shutdown alarm
      new StaticText(line, rect_t{}, STR_RSSI_SHUTDOWN_ALARM, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{}, GET_SET_INVERTED(g_eeGeneral.disableRssiPoweroffAlarm));
      line = body.newLine(&grid);

    }
};

class BacklightPage : public Page {
  public:
	BacklightPage() :
      Page(ICON_RADIO_SETUP)
    {
      build();
    }

  protected:
    FormField* backlightTimeout = nullptr;
    FormField* backlightOnBright = nullptr;
    FormField* backlightOffBright = nullptr;

    void build()
    {
      new StaticText(&header,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
					  STR_BACKLIGHT_LABEL, 0, COLOR_THEME_PRIMARY2);

      body.setFlexLayout();
      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

      auto line = body.newLine(&grid);
      new Subtitle(line, rect_t{}, STR_BACKLIGHT_LABEL, 0, COLOR_THEME_PRIMARY1);
      line = body.newLine(&grid);

      // Backlight mode
      new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);

      auto blMode = new Choice(line, rect_t{}, STR_VBLMODE,
                               e_backlight_mode_off, e_backlight_mode_on,
                               GET_DEFAULT(g_eeGeneral.backlightMode),
                               [=](int32_t newValue) {
                                 g_eeGeneral.backlightMode = newValue;
                                 updateBacklightControls();
                               });

      blMode->setAvailableHandler(
          [=](int newValue) { return newValue != e_backlight_mode_off; });

      // Delay
      auto edit = new NumberEdit(line, rect_t{}, 5, 600,
                                 GET_DEFAULT(g_eeGeneral.lightAutoOff * 5),
                                 SET_VALUE(g_eeGeneral.lightAutoOff, newValue / 5));
      edit->setStep(5);
      edit->setSuffix("s");
      backlightTimeout = edit;

      line = body.newLine(&grid);

      // Backlight ON bright
      new StaticText(line, rect_t{}, STR_BLONBRIGHTNESS, 0, COLOR_THEME_PRIMARY1);
      backlightOnBright = new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, BACKLIGHT_LEVEL_MIN, BACKLIGHT_LEVEL_MAX,
                 [=]() -> int32_t {
                   return BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
                 },
                 [=](int32_t newValue) {
                   if(newValue >= g_eeGeneral.blOffBright || g_eeGeneral.backlightMode == e_backlight_mode_on)
                     g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - newValue;
                   else
                     g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.blOffBright;
                 });
      line = body.newLine(&grid);

      // Backlight OFF bright
      new StaticText(line, rect_t{}, STR_BLOFFBRIGHTNESS, 0, COLOR_THEME_PRIMARY1);
      backlightOffBright = new Slider(line, rect_t{0,0,lv_pct(50),PAGE_LINE_HEIGHT}, BACKLIGHT_LEVEL_MIN, BACKLIGHT_LEVEL_MAX, GET_DEFAULT(g_eeGeneral.blOffBright),
          [=](int32_t newValue) {
            int32_t onBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
            if(newValue <= onBright || g_eeGeneral.backlightMode == e_backlight_mode_off)
              g_eeGeneral.blOffBright = newValue;
            else
              g_eeGeneral.blOffBright = onBright;
          });
      line = body.newLine(&grid);

  #if defined(KEYS_BACKLIGHT_GPIO)
      // Keys backlight
      new StaticText(line, rect_t{}, STR_KEYS_BACKLIGHT, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.keysBacklight));
      line = body.newLine(&grid)();
  #endif

      // Flash beep
      new StaticText(line, rect_t{}, STR_ALARM, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.alarmsFlash));
      line = body.newLine(&grid);

      updateBacklightControls();
    }

    void updateBacklightControls()
    {
      switch(g_eeGeneral.backlightMode)
      {
      case e_backlight_mode_off:
        backlightTimeout->enable(false);
        backlightOnBright->enable(false);
        backlightOffBright->enable(true);
        break;
      case e_backlight_mode_keys:
      case e_backlight_mode_sticks:
      case e_backlight_mode_all:
      default:
      {
        backlightTimeout->enable(true);
        backlightOnBright->enable(true);
        backlightOffBright->enable(true);
        int32_t onBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
        if(onBright < g_eeGeneral.blOffBright)
          g_eeGeneral.backlightBright = BACKLIGHT_LEVEL_MAX - g_eeGeneral.blOffBright;
        break;
      }
      case e_backlight_mode_on:
        backlightTimeout->enable(false);
        backlightOnBright->enable(true);
        backlightOffBright->enable(false);
        break;
      }
      resetBacklightTimeout();
    }
};

#if defined(INTERNAL_GPS)
class GpsPage : public Page {
  public:
	GpsPage() :
      Page(ICON_RADIO_SETUP)
    {
      build();
    }

  protected:

    void build()
    {
      new StaticText(&header,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
					  STR_GPS, 0, COLOR_THEME_PRIMARY2);

      body.setFlexLayout();
      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);

      auto line = body.newLine(&grid);
      // Timezone
      new StaticText(line, rect_t{}, STR_TIMEZONE, 0, COLOR_THEME_PRIMARY1);
      new NumberEdit(line, rect_t{}, -12, 12, GET_SET_DEFAULT(g_eeGeneral.timezone));
      line = body.newLine(&grid);

      // Adjust RTC (from telemetry)
      new StaticText(line, rect_t{}, STR_ADJUST_RTC, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.adjustRTC));
      line = body.newLine(&grid);

      // GPS format
      new StaticText(line, rect_t{}, STR_GPS_COORDS_FORMAT, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_GPSFORMAT, 0, 1, GET_SET_DEFAULT(g_eeGeneral.gpsFormat));
      line = body.newLine(&grid);
    }
};
#endif

RadioSetupPage::RadioSetupPage():
  PageTab(STR_RADIO_SETUP, ICON_RADIO_SETUP)
{
}

void RadioSetupPage::build(FormWindow * window)
{
  window->setFlexLayout();
  FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);
//  FormGridLayout grid;
//  grid.spacer(PAGE_PADDING);

  auto line = window->newLine(&grid);
  // Date and Time
//  auto timeWindow = new DateTimeWindow(line, {0, 0, LCD_W, 0});
//  grid.addWindow(timeWindow);


  // Batt meter range - Range 3.0v to 16v
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_BATTERY_RANGE, 0, COLOR_THEME_PRIMARY1);
  auto batMinEdit = new NumberEdit(line, rect_t{}, -60 + 90, g_eeGeneral.vBatMax + 29 + 90, GET_SET_WITH_OFFSET(g_eeGeneral.vBatMin, 90), 0, PREC1);
  batMinEdit->setSuffix("V");
  auto batMaxEdit = new NumberEdit(line, rect_t{}, g_eeGeneral.vBatMin - 29 + 120, 40 + 120, GET_SET_WITH_OFFSET(g_eeGeneral.vBatMax, 120), 0, PREC1);
  batMaxEdit->setSuffix("V");
  batMinEdit->setSetValueHandler([=](int32_t newValue) {
    g_eeGeneral.vBatMin= newValue - 90;
    SET_DIRTY();
    batMaxEdit->setMin(g_eeGeneral.vBatMin - 29 + 120);
    batMaxEdit->invalidate();
  });
  batMaxEdit->setSetValueHandler([=](int32_t newValue) {
    g_eeGeneral.vBatMax= newValue - 120;
    SET_DIRTY();
    batMinEdit->setMax(g_eeGeneral.vBatMax + 29 + 90);
    batMinEdit->invalidate();
  });

//  grid.nextLine();
  line = window->newLine(&grid);
  std::vector<std::pair<const char*, std::function<void()> >> windows;
  windows.push_back(std::make_pair(STR_SOUND_LABEL, [window](){new SoundPage();}));
#if defined(VARIO)
  windows.push_back(std::make_pair(STR_VARIO, [window](){new VarioPage();}));
#endif
#if defined(HAPTIC)
  windows.push_back(std::make_pair(STR_HAPTIC_LABEL, [window](){new HapticPage();}));
#endif
  windows.push_back(std::make_pair(STR_ALARM, [window](){new AlarmsPage();}));
  windows.push_back(std::make_pair(STR_BACKLIGHT_LABEL, [window](){new BacklightPage();}));
#if defined(INTERNAL_GPS)
  windows.push_back(std::make_pair(STR_GPS, [window](){new GpsPage();}));
#endif
  auto buttons = new WindowButtonGroup(line, {0,0, LCD_W, 0}, windows);
  buttons->adjustHeight();
 // grid.addWindow(buttons);


#if defined(PWR_BUTTON_PRESS)
  // Pwr Off Delay
  {
    new StaticText(line, rect_t{}, STR_PWR_OFF_DELAY, 0, COLOR_THEME_PRIMARY1);
    new Choice(line, rect_t{}, STR_PWR_OFF_DELAYS, 0, 3,
               [=]() -> int32_t {
               return 2 - g_eeGeneral.pwrOffSpeed;
               },
               [=](int32_t newValue) {
                   g_eeGeneral.pwrOffSpeed = 2 - newValue;
                   SET_DIRTY();
               });
//    grid.nextLine();
    line = window->newLine(&grid);
  }
#endif
  
#if defined(PXX2)
  // Owner ID
  new StaticText(line, rect_t{}, STR_OWNER_ID, 0, COLOR_THEME_PRIMARY1);
  new RadioTextEdit(line, rect_t{}, g_eeGeneral.ownerRegistrationID, PXX2_LEN_REGISTRATION_ID);
  line = window->newLine(&grid);
#endif

  // Country code
  new StaticText(line, rect_t{}, STR_COUNTRY_CODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_COUNTRY_CODES, 0, 2, GET_SET_DEFAULT(g_eeGeneral.countryCode));
  line = window->newLine(&grid);

  // Audio language
  new StaticText(line, rect_t{}, STR_VOICE_LANGUAGE, 0, COLOR_THEME_PRIMARY1);
  auto choice = new Choice(line, rect_t{}, 0, DIM(languagePacks) - 2, GET_VALUE(currentLanguagePackIdx),
                           [](uint8_t newValue) {
                             currentLanguagePackIdx = newValue;
                             currentLanguagePack = languagePacks[currentLanguagePackIdx];
                             strncpy(g_eeGeneral.ttsLanguage, currentLanguagePack->id, 2);
                           });
  choice->setTextHandler([](uint8_t value) {
    return languagePacks[value]->name;
  });
  line = window->newLine(&grid);

  // Imperial units
  new StaticText(line, rect_t{}, STR_UNITS_SYSTEM, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VUNITSSYSTEM, 0, 1, GET_SET_DEFAULT(g_eeGeneral.imperial));
  line = window->newLine(&grid);

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
  new StaticText(line, rect_t{}, STR_SWITCHES_DELAY, 0, COLOR_THEME_PRIMARY1);
  auto edit = new NumberEdit(line, rect_t{}, -15, 100 - 15, GET_SET_VALUE_WITH_OFFSET(g_eeGeneral.switchesDelay, 15));
  edit->setSuffix(std::string("0") + STR_MS);
  line = window->newLine(&grid);

  // USB mode
  new StaticText(line, rect_t{}, STR_USBMODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_USBMODES, USB_UNSELECTED_MODE, USB_MAX_MODE, GET_SET_DEFAULT(g_eeGeneral.USBMode));
  line = window->newLine(&grid);

#if defined(ROTARY_ENCODER_NAVIGATION)
  new StaticText(line, rect_t{}, STR_INVERT_ROTARY, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.rotEncDirection));
  line = window->newLine(&grid);
#endif

  // RX channel order
  new StaticText(line, rect_t{}, STR_RXCHANNELORD, 0, COLOR_THEME_PRIMARY1); // RAET->AETR
  choice = new Choice(line, rect_t{}, 0, 4*3*2 - 1, GET_SET_DEFAULT(g_eeGeneral.templateSetup));
  choice->setTextHandler([](uint8_t value) {
    std::string s;
    for (uint8_t i = 0; i < 4; i++) {
      s += STR_RETA123[channelOrder(value, i + 1) - 1];
    }
    return s;
  });
  line->adjustHeight();
  line = window->newLine(&grid);

  // Stick mode
  new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);
  choice = new Choice(line, rect_t{}, 0, 3, GET_DEFAULT(g_eeGeneral.stickMode),
                      [=](uint8_t newValue) {
                        pausePulses();
                        g_eeGeneral.stickMode = newValue;
                        SET_DIRTY();
                        checkThrottleStick();
                        resumePulses();
                      });
  choice->setTextHandler([](uint8_t value) {
    return std::to_string(1 + value) + ": left=" + std::string(&getSourceString(MIXSRC_Rud + modn12x3[4 * value])[1]) + "+" + std::string(&getSourceString(MIXSRC_Rud + modn12x3[4 * value + 1])[1]);
  });
  line = window->newLine(&grid);

// extra bottom padding if touchscreen
#if defined HARDWARE_TOUCH
  line = window->newLine(&grid);
#endif

}


