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

static const lv_coord_t col_two_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t col_three_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t col_four_dsc[] = {LV_GRID_FR(3), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

class DateTimeWindow : public FormGroup {
  public:
    DateTimeWindow(Window* parent, const rect_t & rect) :
      FormGroup(parent, rect)
    {
      build();
    }

    void checkEvents() override
    {
      FormGroup::checkEvents();

      if (get_tmr10ms() - lastRefresh > 100) {
        seconds->setValue(seconds->getValue());
        lastRefresh = get_tmr10ms();
      }
    }

  protected:
    tmr10ms_t lastRefresh = 0;
    NumberEdit* seconds = nullptr;

    void build()
    {
      setFlexLayout();
      FlexGridLayout grid(col_four_dsc, row_dsc, 2);

      auto line = newLine(&grid);
      // Date
      new StaticText(line, rect_t{}, STR_DATE, 0, COLOR_THEME_PRIMARY1);
      auto year = new NumberEdit(line, rect_t{}, 2018, 2100,
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
      lv_obj_set_style_grid_cell_x_align(year->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
      auto month = new NumberEdit(line, rect_t{}, 1, 12,
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
      lv_obj_set_style_grid_cell_x_align(month->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      /* TODO dynamic max instead of 31 ...
      int16_t year = TM_YEAR_BASE + t.tm_year;
      int8_t dlim = (((((year%4==0) && (year%100!=0)) || (year%400==0)) && (t.tm_mon==1)) ? 1 : 0);
      static const pm_uint8_t dmon[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
      dlim += *(&dmon[t.tm_mon]);*/
      int8_t dlim = 31;
      auto day = new NumberEdit(line, rect_t{}, 1, dlim,
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
      lv_obj_set_style_grid_cell_x_align(day->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
      line = newLine(&grid);

      // Time
      new StaticText(line, rect_t{}, STR_TIME, 0, COLOR_THEME_PRIMARY1);
      auto hour = new NumberEdit(line, rect_t{}, 0, 24,
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
      lv_obj_set_style_grid_cell_x_align(hour->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      auto minutes = new NumberEdit(line, rect_t{}, 0, 59,
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
      lv_obj_set_style_grid_cell_x_align(minutes->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);

      seconds = new NumberEdit(line, rect_t{}, 0, 59,
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
        return formatNumberAsString(value, LEADING0, 2);
      });
      lv_obj_set_style_grid_cell_x_align(seconds->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
      line = newLine(&grid);
    }
};

class WindowButtonGroup : public FormGroup
{
 public:
  typedef std::function<void()>           PageFct;
  typedef std::pair<const char*, PageFct> PageDef;
  typedef std::list<PageDef>              PageDefs;
  
  WindowButtonGroup(
      Window* parent, const rect_t& rect, PageDefs pages) :
      FormGroup(parent, rect),
      pages(pages)
  {
    setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
    lv_obj_set_style_flex_main_place(lvobj, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    padRow(lv_dpx(8));

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
      FlexGridLayout grid(col_two_dsc, row_dsc, 2);
      lv_obj_set_style_pad_column(lvobj, 10, 0);

      auto line = body.newLine(&grid);

      // Beeps mode
      new StaticText(line, rect_t{}, STR_SPEAKER, 0, COLOR_THEME_PRIMARY1);
      new Choice(line, rect_t{}, STR_VBEEPMODE, -2, 1, GET_SET_DEFAULT(g_eeGeneral.beepMode));
      line = body.newLine(&grid);

      // Main volume
      new StaticText(line, rect_t{}, STR_VOLUME, 0, COLOR_THEME_PRIMARY1);
      new Slider(line, rect_t{0,0,lv_pct(50), PAGE_LINE_HEIGHT}, -VOLUME_LEVEL_DEF, VOLUME_LEVEL_MAX-VOLUME_LEVEL_DEF, GET_SET_DEFAULT(g_eeGeneral.speakerVolume));
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
      FlexGridLayout grid(col_two_dsc, row_dsc, 2);

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
      FlexGridLayout grid(col_two_dsc, row_dsc, 2);

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
      FlexGridLayout grid(col_two_dsc, row_dsc, 2);

      auto line = body.newLine(&grid);
      // Battery warning
      new StaticText(line, rect_t{}, STR_BATTERYWARNING, 0, COLOR_THEME_PRIMARY1);
      auto edit = new NumberEdit(line, rect_t{}, 30, 120, GET_SET_DEFAULT(g_eeGeneral.vBatWarn), 0, PREC1);
      lv_obj_set_style_grid_cell_x_align(edit->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
      edit->setSuffix("V");
      line = body.newLine(&grid);

      // Inactivity alarm
      new StaticText(line, rect_t{}, STR_INACTIVITYALARM, 0, COLOR_THEME_PRIMARY1);
      edit = new NumberEdit(line, rect_t{}, 0, 250, GET_SET_DEFAULT(g_eeGeneral.inactivityTimer));
      lv_obj_set_style_grid_cell_x_align(edit->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
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

// defined in gui/gui_common.cpp
uint8_t switchToMix(uint8_t source);

class BacklightSourceChoice : public Choice
{
 public:
  BacklightSourceChoice(Window* parent, const rect_t& rect) :
    Choice(parent, rect, 0, MIXSRC_LAST_SWITCH, GET_SET_DEFAULT(g_eeGeneral.backlightSrc))
  {
    setBeforeDisplayMenuHandler([=](Menu* menu) {
#if defined(AUTOSOURCE)
      menu->setWaitHandler([=]() {
        int16_t val = getMovedSource(MIXSRC_FIRST_POT);
        if (val) {
          //fillMenu(menu);
          menu->select(getIndexFromValue(val));
        }
#if defined(AUTOSWITCH)
        else {
          swsrc_t swtch = abs(getMovedSwitch());
          if (swtch && !IS_SWITCH_MULTIPOS(swtch)) {
            val = switchToMix(swtch);
            if (val && (val >= MIXSRC_FIRST_SWITCH) && (val <= MIXSRC_LAST_SWITCH)) {
              // fillMenu(menu);
              menu->select(getIndexFromValue(val));
            }
          }
        }
#endif
      });
#endif
    });

    setTextHandler([=](int value) {
      if (isValueAvailable && !isValueAvailable(value))
        return std::to_string(0);  // we will fix this later

      return std::string(getSourceString(value));
    });

    setAvailableHandler(::isSourceAvailable);
  }

// protected:
//   static bool isSourceAvailable(int source)
//   {
//     if (source == MIXSRC_NONE) return true;
//     if (source >= MIXSRC_FIRST_POT && source <= MIXSRC_LAST_POT) {
//       return IS_POT_SLIDER_AVAILABLE(POT1+source - MIXSRC_FIRST_POT);
//     }
    
//     if (source >= MIXSRC_FIRST_SWITCH && source <= MIXSRC_LAST_SWITCH) {
//       return SWITCH_EXISTS(source - MIXSRC_FIRST_SWITCH);
//     }

//     return false;
//   }
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
      FlexGridLayout grid(col_three_dsc, row_dsc, 2);

      auto line = body.newLine(&grid);

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
      new StaticText(line, rect_t{}, STR_SOURCE, 0, COLOR_THEME_PRIMARY1);
      new BacklightSourceChoice(line, rect_t{});
      
      // Backlight ON bright
      line = body.newLine(&grid);
      new StaticText(line, rect_t{}, STR_BLONBRIGHTNESS, 0, COLOR_THEME_PRIMARY1);
      grid.setColSpan(2);
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
      grid.setColSpan(1);
      line = body.newLine(&grid);

      // Backlight OFF bright
      new StaticText(line, rect_t{}, STR_BLOFFBRIGHTNESS, 0, COLOR_THEME_PRIMARY1);
      grid.setColSpan(2);
      backlightOffBright = new Slider(
          line, rect_t{0, 0, lv_pct(50), PAGE_LINE_HEIGHT}, BACKLIGHT_LEVEL_MIN,
          BACKLIGHT_LEVEL_MAX, GET_DEFAULT(g_eeGeneral.blOffBright),
          [=](int32_t newValue) {
            int32_t onBright =
                BACKLIGHT_LEVEL_MAX - g_eeGeneral.backlightBright;
            if (newValue <= onBright ||
                g_eeGeneral.backlightMode == e_backlight_mode_off)
              g_eeGeneral.blOffBright = newValue;
            else
              g_eeGeneral.blOffBright = onBright;
          });
      grid.setColSpan(1);
      line = body.newLine(&grid);

  #if defined(KEYS_BACKLIGHT_GPIO)
      // Keys backlight
      new StaticText(line, rect_t{}, STR_KEYS_BACKLIGHT, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(line, rect_t{}, GET_SET_DEFAULT(g_eeGeneral.keysBacklight));
      line = body.newLine(&grid);
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
      FlexGridLayout grid(col_two_dsc, row_dsc, 2);

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
  FlexGridLayout grid(col_three_dsc, row_dsc, 2);
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
      {STR_ALARM, []() { new AlarmsPage(); }},
      {STR_BACKLIGHT_LABEL, []() { new BacklightPage(); }},
#if defined(INTERNAL_GPS)
      {STR_GPS, [](){new GpsPage();}},
#endif
  });


#if defined(PWR_BUTTON_PRESS)
  // Pwr Off Delay
  {
    auto line = window->newLine(&grid);
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
    auto line = window->newLine(&grid);
    new StaticText(line, rect_t{}, STR_OWNER_ID, 0, COLOR_THEME_PRIMARY1);
    new RadioTextEdit(line, rect_t{}, g_eeGeneral.ownerRegistrationID,
                      PXX2_LEN_REGISTRATION_ID);
  }
#endif

  // Country code
  auto line = window->newLine(&grid);
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
                 });
  choice->setTextHandler(
      [](uint8_t value) { return languagePacks[value]->name; });

  // Imperial units
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_UNITS_SYSTEM, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_VUNITSSYSTEM, 0, 1, GET_SET_DEFAULT(g_eeGeneral.imperial));

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
  grid.setColSpan(2);
  auto edit =
      new NumberEdit(line, rect_t{}, -15, 100 - 15,
                     GET_SET_VALUE_WITH_OFFSET(g_eeGeneral.switchesDelay, 15));
  edit->setSuffix(std::string("0") + STR_MS);
  grid.setColSpan(1);

  // USB mode
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_USBMODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_USBMODES, USB_UNSELECTED_MODE, USB_MAX_MODE,
             GET_SET_DEFAULT(g_eeGeneral.USBMode));

#if defined(ROTARY_ENCODER_NAVIGATION)
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_ROTARY_ENC_MODE, 0, COLOR_THEME_PRIMARY1);
  new Choice(line, rect_t{}, STR_ROTARY_ENC_OPT, ROTARY_ENCODER_MODE_NORMAL,
             ROTARY_ENCODER_MODE_INVERT_BOTH,
             GET_SET_DEFAULT(g_eeGeneral.rotEncMode));
#endif

  // RX channel order
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_RXCHANNELORD, 0,
                 COLOR_THEME_PRIMARY1);  // RAET->AETR
  grid.setColSpan(2);
  choice = new Choice(line, rect_t{}, 0, 4 * 3 * 2 - 1,
                      GET_SET_DEFAULT(g_eeGeneral.templateSetup));
  choice->setTextHandler([](uint8_t value) {
    std::string s;
    for (uint8_t i = 0; i < 4; i++) {
      s += STR_RETA123[channelOrder(value, i + 1) - 1];
    }
    return s;
  });
  grid.setColSpan(1);

  // Stick mode
  line = window->newLine(&grid);
  new StaticText(line, rect_t{}, STR_MODE, 0, COLOR_THEME_PRIMARY1);
  grid.setColSpan(2);
  choice = new Choice(line, rect_t{}, 0, 3, GET_DEFAULT(g_eeGeneral.stickMode),
                      [=](uint8_t newValue) {
                        pausePulses();
                        g_eeGeneral.stickMode = newValue;
                        SET_DIRTY();
                        checkThrottleStick();
                        resumePulses();
                      });
  choice->setTextHandler([](uint8_t value) {
    return std::to_string(1 + value) + ": left=" +
           std::string(&getSourceString(MIXSRC_Rud + modn12x3[4 * value])[1]) +
           "+" +
           std::string(
               &getSourceString(MIXSRC_Rud + modn12x3[4 * value + 1])[1]);
  });
}


