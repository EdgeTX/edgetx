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

#include "edgetx.h"
#include "tasks/mixer_task.h"
#include "hal/adc_driver.h"
#include "hal/usb_driver.h"
#include "input_mapping.h"

const unsigned char sticks[]  = {
#include "sticks.lbm"
};

#define RADIO_SETUP_2ND_COLUMN  79

int8_t slider_5pos(coord_t y, int8_t value, event_t event, uint8_t attr, const char* title)
{
  drawSlider(RADIO_SETUP_2ND_COLUMN, y, LCD_W - 2 - RADIO_SETUP_2ND_COLUMN, 2+value, 4, attr);
  return editChoice(RADIO_SETUP_2ND_COLUMN, y, title, nullptr, value, -2, +2, attr, event, INDENT_WIDTH);
}

#if defined(BATTGRAPH)
  #define CASE_BATTGRAPH(x) x,
#else
  #define CASE_BATTGRAPH(x)
#endif

#if !defined(SURFACE_RADIO)
#define CASE_TX_MODE(x) x,
#else
#define CASE_TX_MODE(x)
#endif

enum {
  CASE_RTCLOCK(ITEM_RADIO_SETUP_DATE)
  CASE_RTCLOCK(ITEM_RADIO_SETUP_TIME)
  CASE_BATTGRAPH(ITEM_RADIO_SETUP_BATT_RANGE)
  ITEM_RADIO_SETUP_SOUND_LABEL,
  CASE_AUDIO(ITEM_RADIO_SETUP_BEEP_MODE)
  CASE_BUZZER(ITEM_RADIO_SETUP_BUZZER_MODE)
  ITEM_RADIO_SETUP_SPEAKER_VOLUME,
  ITEM_RADIO_SETUP_BEEP_VOLUME,
  ITEM_RADIO_SETUP_BEEP_LENGTH,
  CASE_AUDIO(ITEM_RADIO_SETUP_SPEAKER_PITCH)
  ITEM_RADIO_SETUP_WAV_VOLUME,
  ITEM_RADIO_SETUP_BACKGROUND_VOLUME,
  ITEM_RADIO_SETUP_START_SOUND,
  CASE_VARIO(ITEM_RADIO_SETUP_VARIO_LABEL)
  CASE_VARIO(ITEM_RADIO_SETUP_VARIO_VOLUME)
  CASE_VARIO(ITEM_RADIO_SETUP_VARIO_PITCH)
  CASE_VARIO(ITEM_RADIO_SETUP_VARIO_RANGE)
  CASE_VARIO(ITEM_RADIO_SETUP_VARIO_REPEAT)
  CASE_HAPTIC(ITEM_RADIO_SETUP_HAPTIC_LABEL)
  CASE_HAPTIC(ITEM_RADIO_SETUP_HAPTIC_MODE)
  CASE_HAPTIC(ITEM_RADIO_SETUP_HAPTIC_LENGTH)
  CASE_HAPTIC(ITEM_RADIO_SETUP_HAPTIC_STRENGTH)
  CASE_IMU(ITEM_RADIO_SETUP_IMU_LABEL)
  CASE_IMU(ITEM_RADIO_SETUP_IMU_MAX)
  CASE_IMU(ITEM_RADIO_SETUP_IMU_OFFSET)
  ITEM_RADIO_SETUP_ALARMS_LABEL,
  ITEM_RADIO_SETUP_BATTERY_WARNING,
  ITEM_RADIO_SETUP_INACTIVITY_ALARM,
  ITEM_RADIO_SETUP_MEMORY_WARNING,
  ITEM_RADIO_SETUP_ALARM_WARNING,
  ITEM_RADIO_SETUP_RSSI_POWEROFF_ALARM,
  ITEM_RADIO_SETUP_TRAINER_POWEROFF_ALARM,
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_LABEL)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_MODE)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_DELAY)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BRIGHTNESS)
  CASE_CONTRAST(ITEM_RADIO_SETUP_CONTRAST)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_FLASH_BEEP)
  CASE_SPLASH_PARAM(ITEM_RADIO_SETUP_DISABLE_SPLASH)
  CASE_PWR_BUTTON_PRESS(ITEM_RADIO_SETUP_PWR_ON_SPEED)
  CASE_PWR_BUTTON_PRESS(ITEM_RADIO_SETUP_PWR_OFF_SPEED)
  CASE_PWR_BUTTON_PRESS(ITEM_RADIO_SETUP_PWR_AUTO_OFF)
  CASE_HAPTIC(ITEM_RADIO_SETUP_PWR_ON_OFF_HAPTIC)
  CASE_PXX2(ITEM_RADIO_SETUP_OWNER_ID)
  CASE_GPS(ITEM_RADIO_SETUP_LABEL_GPS)
  CASE_GPS(ITEM_RADIO_SETUP_TIMEZONE)
  CASE_GPS(ITEM_RADIO_SETUP_ADJUST_RTC)
  CASE_GPS(ITEM_RADIO_SETUP_GPSFORMAT)
  CASE_PXX1(ITEM_RADIO_SETUP_COUNTRYCODE)
  ITEM_RADIO_SETUP_LANGUAGE,
  ITEM_RADIO_SETUP_IMPERIAL,
  ITEM_RADIO_SETUP_PPM,
  IF_FAI_CHOICE(ITEM_RADIO_SETUP_FAI)
  ITEM_RADIO_SETUP_SWITCHES_DELAY,
  ITEM_RADIO_SETUP_USB_MODE,
  CASE_JACK_DETECT(ITEM_RADIO_SETUP_JACK_MODE)
  ITEM_RADIO_SETUP_RX_CHANNEL_ORD,
  CASE_ROTARY_ENCODER(ITEM_RADIO_SETUP_ROTARY_ENC_MODE)
  CASE_TX_MODE(ITEM_RADIO_SETUP_STICK_MODE_LABELS)
  CASE_TX_MODE(ITEM_RADIO_SETUP_STICK_MODE)
  ITEM_VIEW_OPTIONS_LABEL,
  ITEM_VIEW_OPTIONS_RADIO_TAB,
  ITEM_VIEW_OPTIONS_GF,
  ITEM_VIEW_OPTIONS_TRAINER,
  ITEM_VIEW_OPTIONS_MODEL_TAB,
  CASE_HELI(ITEM_VIEW_OPTIONS_HELI)
  CASE_FLIGHT_MODES(ITEM_VIEW_OPTIONS_FM)
  ITEM_VIEW_OPTIONS_CURVES,
  ITEM_VIEW_OPTIONS_LS,
  ITEM_VIEW_OPTIONS_SF,
  CASE_LUA_MODEL_SCRIPTS(ITEM_VIEW_OPTIONS_CUSTOM_SCRIPTS)
  ITEM_VIEW_OPTIONS_TELEMETRY,
  ITEM_RADIO_SETUP_MAX
};

PACK(struct ExpandState {
  uint8_t sound:1;
  uint8_t alarms:1;
  uint8_t viewOpt:1;
});

static struct ExpandState expandState;

static uint8_t SOUND_ROW(uint8_t value) { return expandState.sound ? value : HIDDEN_ROW; }

static uint8_t ALARMS_ROW(uint8_t value) { return expandState.alarms ? value : HIDDEN_ROW; }

static uint8_t VIEWOPT_ROW(uint8_t value) { return expandState.viewOpt ? value : HIDDEN_ROW; }

uint8_t viewOptCheckBox(coord_t y, const char* title, uint8_t value, uint8_t attr, event_t event, uint8_t modelOption)
{
  if (modelOption != OVERRIDE_GLOBAL) {
    std::string s(STR_ADCFILTERVALUES[modelOption]);
    lcdDrawText(LCD_W-3*FW, y, s.c_str());
  }
  return !editCheckBox(!value, LCD_W-4*FW-3, y, title, attr, event, INDENT_WIDTH) ;
}

void menuRadioSetup(event_t event)
{
#if defined(RTCLOCK)
  struct gtm t;
  gettime(&t);

  if ((menuVerticalPosition == ITEM_RADIO_SETUP_DATE + HEADER_LINE ||
       menuVerticalPosition == ITEM_RADIO_SETUP_TIME + HEADER_LINE) &&
      (s_editMode > 0) &&
      (event == EVT_KEY_BREAK(KEY_ENTER) || event == EVT_KEY_BREAK(KEY_EXIT))) {
    // set the date and time into RTC chip
    rtcSetTime(&t);
  }
#endif

#if defined(FAI_CHOICE)
  if (warningResult) {
    warningResult = 0;
    g_eeGeneral.fai = true;
    storageDirty(EE_GENERAL);
  }
#endif

#if defined(PXX2)
  uint8_t old_editMode = s_editMode;
#endif

  MENU(STR_RADIO_SETUP, menuTabGeneral, MENU_RADIO_SETUP, ITEM_RADIO_SETUP_MAX, {
    HEADER_LINE_COLUMNS
    CASE_RTCLOCK(2) CASE_RTCLOCK(2) CASE_BATTGRAPH(1)
    // Sound
    0, 
     CASE_AUDIO(SOUND_ROW(0))
     CASE_BUZZER(SOUND_ROW(0))
     SOUND_ROW(0),
     SOUND_ROW(0),
     SOUND_ROW(0),
     CASE_AUDIO(SOUND_ROW(0))
     SOUND_ROW(0),
     SOUND_ROW(0),
     SOUND_ROW(0),
    // Vario
    CASE_VARIO(LABEL(VARIO))
     CASE_VARIO(0)
     CASE_VARIO(0)
     CASE_VARIO(0)
     CASE_VARIO(0)
    // Haptic
    CASE_HAPTIC(LABEL(VARIO))
     CASE_HAPTIC(0)
     CASE_HAPTIC(0)
     CASE_HAPTIC(0)
    // IMU
    CASE_IMU(LABEL(IMU))
     CASE_IMU(0)
     CASE_IMU(0)
    // Alarms
    0,
     ALARMS_ROW(0),
     ALARMS_ROW(0),
     ALARMS_ROW(0),
     ALARMS_ROW(0),
     ALARMS_ROW(0),
     ALARMS_ROW(0),
    // Backlight
    CASE_BACKLIGHT(LABEL(BACKLIGHT))
     CASE_BACKLIGHT(0)
     CASE_BACKLIGHT(0)
     CASE_BACKLIGHT(0)
     CASE_CONTRAST(0)
     CASE_BACKLIGHT(0)
    CASE_SPLASH_PARAM(0)
    CASE_PWR_BUTTON_PRESS(0)
    CASE_PWR_BUTTON_PRESS(0)
    CASE_PWR_BUTTON_PRESS(0)
    CASE_HAPTIC(0) // power on/off haptic
    CASE_PXX2(0) /* owner registration ID */
    // GPS
    CASE_GPS(LABEL(GPS))
     CASE_GPS(0)
     CASE_GPS(0)
     CASE_GPS(0)
    CASE_PXX1(0)
    0, 0, 0,
    IF_FAI_CHOICE(0)
    0,
    0, // USB mode
    CASE_JACK_DETECT(0) // Jack mode
    0, // Channel order
    CASE_ROTARY_ENCODER(0)
    CASE_TX_MODE(LABEL(TX_MODE))
    CASE_TX_MODE(0)
    // View options
    0,
     VIEWOPT_ROW(LABEL(RadioMenuTabs)),
      VIEWOPT_ROW(0),
      VIEWOPT_ROW(0),
     VIEWOPT_ROW(LABEL(ModelMenuTabs)),
      CASE_HELI(VIEWOPT_ROW(0))
      CASE_FLIGHT_MODES(VIEWOPT_ROW(0))
      VIEWOPT_ROW(0),
      VIEWOPT_ROW(0),
      VIEWOPT_ROW(0),
      CASE_LUA_MODEL_SCRIPTS(VIEWOPT_ROW(0))
      VIEWOPT_ROW(0),
  });

  if (event == EVT_ENTRY) {
    reusableBuffer.generalSettings.stickMode = g_eeGeneral.stickMode;
#if defined(ROTARY_ENCODER_NAVIGATION)
    reusableBuffer.generalSettings.rotaryEncoderMode = g_eeGeneral.rotEncMode;
#endif
  }

  uint8_t sub = menuVerticalPosition - HEADER_LINE;

  for (uint8_t i=0; i<LCD_LINES-1; i++) {
    coord_t y = MENU_HEADER_HEIGHT + 1 + i*FH;
    uint8_t k = i + menuVerticalOffset;
    for (int j=0; j<=k; j++) {
      if (mstate_tab[j+HEADER_LINE] == HIDDEN_ROW) {
        if (++k >= (int)DIM(mstate_tab)) {
          return;
        }
      }
    }

    LcdFlags blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    LcdFlags attr = (sub == k ? blink : 0);

    switch (k) {
#if defined(RTCLOCK)
      case ITEM_RADIO_SETUP_DATE:
        lcdDrawTextAlignedLeft(y, STR_DATE);
        for (uint8_t j=0; j<3; j++) {
          uint8_t rowattr = (menuHorizontalPosition==j ? attr : 0);
          switch (j) {
            case 0:
              lcdDrawNumber(RADIO_SETUP_2ND_COLUMN-2, y, t.tm_year+TM_YEAR_BASE, rowattr);
              lcdDrawChar(lcdNextPos, y, '-');
              if (rowattr && s_editMode > 0) t.tm_year = checkIncDec(event, t.tm_year, 123, 137, 0);
              break;
            case 1:
              lcdDrawNumber(lcdNextPos, y, t.tm_mon+1, rowattr|LEADING0, 2);
              lcdDrawChar(lcdNextPos, y, '-');
              if (rowattr && s_editMode > 0) t.tm_mon = checkIncDec(event, t.tm_mon, 0, 11, 0);
              break;
            case 2:
            {
              int16_t year = TM_YEAR_BASE + t.tm_year;
              int8_t dlim = (((((year%4==0) && (year%100!=0)) || (year%400==0)) && (t.tm_mon==1)) ? 1 : 0);
              static const uint8_t dmon[]  = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
              dlim += dmon[t.tm_mon];
              lcdDrawNumber(lcdNextPos, y, t.tm_mday, rowattr|LEADING0, 2);
              if (rowattr && s_editMode > 0) t.tm_mday = checkIncDec(event, t.tm_mday, 1, dlim, 0);
              break;
            }
          }
        }
        if (attr && checkIncDec_Ret) {
          g_rtcTime = gmktime(&t); // update local timestamp and get wday calculated
        }
        break;

      case ITEM_RADIO_SETUP_TIME:
        lcdDrawTextAlignedLeft(y, STR_TIME);
        for (uint8_t j=0; j<3; j++) {
          uint8_t rowattr = (menuHorizontalPosition==j ? attr : 0);
          switch (j) {
            case 0:
              lcdDrawNumber(LCD_W-6*FW-5, y, t.tm_hour, rowattr|LEADING0, 2);
              lcdDrawChar(lcdNextPos + 1, y, ':');
              if (rowattr && s_editMode > 0) t.tm_hour = checkIncDec(event, t.tm_hour, 0, 23, 0);
              break;
            case 1:
              lcdDrawNumber(lcdNextPos + 1, y, t.tm_min, rowattr|LEADING0, 2);
              lcdDrawChar(lcdNextPos + 1, y, ':');
              if (rowattr && s_editMode > 0) t.tm_min = checkIncDec(event, t.tm_min, 0, 59, 0);
              break;
            case 2:
              lcdDrawNumber(lcdNextPos + 1, y, t.tm_sec, rowattr|LEADING0, 2);
              if (rowattr && s_editMode > 0) t.tm_sec = checkIncDec(event, t.tm_sec, 0, 59, 0);
              break;
          }
        }
        if (attr && checkIncDec_Ret) {
          g_rtcTime = gmktime(&t); // update local timestamp and get wday calculated
        }
        break;
#endif

#if defined(BATTGRAPH)
      case ITEM_RADIO_SETUP_BATT_RANGE:
        lcdDrawTextAlignedLeft(y, STR_BATTERY_RANGE);
        putsVolts(LCD_W-1, y, 120+g_eeGeneral.vBatMax, (menuHorizontalPosition>0 ? attr : 0)|RIGHT|NO_UNIT);
        lcdDrawChar(lcdLastLeftPos - FW, y, '-');
        putsVolts(lcdLastLeftPos - FW, y,  90+g_eeGeneral.vBatMin, (menuHorizontalPosition==0 ? attr : 0)|RIGHT|NO_UNIT);
        if (attr && s_editMode>0) {
          if (menuHorizontalPosition==0)
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatMin, -60, g_eeGeneral.vBatMax+29); // min=3.0V
          else
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatMax, g_eeGeneral.vBatMin-29, +40); // max=16.0V
        }
        break;
#endif

      case ITEM_RADIO_SETUP_SOUND_LABEL:
        expandState.sound = expandableSection(y, STR_SOUND_LABEL, expandState.sound, attr, event);
        break;

#if defined(AUDIO)
      case ITEM_RADIO_SETUP_BEEP_MODE:
        g_eeGeneral.beepMode = editChoice(LCD_W-2, y, STR_SPEAKER, STR_VBEEPMODE, g_eeGeneral.beepMode, -2, 1, attr|RIGHT, event, INDENT_WIDTH);
        break;

#if defined(BUZZER) // AUDIO + BUZZER
      case ITEM_RADIO_SETUP_BUZZER_MODE:
        g_eeGeneral.buzzerMode = editChoice(LCD_W-2, y, STR_BUZZER, STR_VBEEPMODE, g_eeGeneral.buzzerMode, -2, 1, attr|RIGHT, event, INDENT_WIDTH);
        break;
#endif
#elif defined(BUZZER) // BUZZER only
      case ITEM_RADIO_SETUP_BUZZER_MODE:
        g_eeGeneral.beepMode = editChoice(LCD_W-2, y, STR_SPEAKER, STR_VBEEPMODE, g_eeGeneral.beepMode, -2, 1, attr|RIGHT, event, INDENT_WIDTH);
        break;
#endif

#if defined(VOICE)
      case ITEM_RADIO_SETUP_SPEAKER_VOLUME:
      {
        lcdDrawTextIndented(y, STR_VOLUME);
        uint8_t b = g_eeGeneral.speakerVolume+VOLUME_LEVEL_DEF;
        drawSlider(RADIO_SETUP_2ND_COLUMN, y, LCD_W - 2 - RADIO_SETUP_2ND_COLUMN, b, VOLUME_LEVEL_MAX, attr);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, b, 0, VOLUME_LEVEL_MAX);
          if (checkIncDec_Ret) {
            g_eeGeneral.speakerVolume = (int8_t)b-VOLUME_LEVEL_DEF;
          }
        }
        break;
      }
#endif

      case ITEM_RADIO_SETUP_BEEP_VOLUME:
        g_eeGeneral.beepVolume = slider_5pos(y, g_eeGeneral.beepVolume, event, attr, STR_BEEP_VOLUME);
        break;

      case ITEM_RADIO_SETUP_WAV_VOLUME:
        g_eeGeneral.wavVolume = slider_5pos(y, g_eeGeneral.wavVolume, event, attr, STR_WAV_VOLUME);
        break;

      case ITEM_RADIO_SETUP_BACKGROUND_VOLUME:
        g_eeGeneral.backgroundVolume = slider_5pos(y, g_eeGeneral.backgroundVolume, event, attr, STR_BG_VOLUME);
        break;

      case ITEM_RADIO_SETUP_BEEP_LENGTH:
        g_eeGeneral.beepLength = slider_5pos(y, g_eeGeneral.beepLength, event, attr, STR_BEEP_LENGTH);
        break;

#if defined(AUDIO)
      case ITEM_RADIO_SETUP_SPEAKER_PITCH:
        {
          lcdDrawTextIndented(y, STR_BEEP_PITCH);
          lcdDrawNumber(LCD_W-14, y, g_eeGeneral.speakerPitch*15, attr|RIGHT);
          coord_t lp = lcdLastLeftPos - FW;
          lcdDrawText(lcdLastRightPos, y, "Hz", attr);
          lcdDrawChar(lp, y, '+', attr);
          if (attr) {
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.speakerPitch, 0, 20);
          }
        }
        break;
#endif

      case ITEM_RADIO_SETUP_START_SOUND:
        g_eeGeneral.dontPlayHello = !editCheckBox(!g_eeGeneral.dontPlayHello, LCD_W-9, y, STR_PLAY_HELLO, attr, event, INDENT_WIDTH) ;
        break;

#if defined(VARIO)
      case ITEM_RADIO_SETUP_VARIO_LABEL:
        lcdDrawTextAlignedLeft(y, STR_VARIO);
        break;

      case ITEM_RADIO_SETUP_VARIO_VOLUME:
        g_eeGeneral.varioVolume = slider_5pos(y, g_eeGeneral.varioVolume, event, attr, STR_VOLUME);
        break;

      case ITEM_RADIO_SETUP_VARIO_PITCH:
        lcdDrawTextIndented(y, STR_PITCH_AT_ZERO);
        lcdDrawNumber(LCD_W-14, y, VARIO_FREQUENCY_ZERO+(g_eeGeneral.varioPitch*10), attr|RIGHT);
        lcdDrawText(lcdLastRightPos, y, "Hz", attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.varioPitch, -40, 40);
        break;

      case ITEM_RADIO_SETUP_VARIO_RANGE:
        lcdDrawTextIndented(y, STR_PITCH_AT_MAX);
        lcdDrawNumber(LCD_W-14, y, VARIO_FREQUENCY_ZERO+(g_eeGeneral.varioPitch*10)+VARIO_FREQUENCY_RANGE+(g_eeGeneral.varioRange*10), attr|RIGHT);
        lcdDrawText(lcdLastRightPos, y, "Hz", attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.varioRange, -80, 80);
        break;

      case ITEM_RADIO_SETUP_VARIO_REPEAT:
        lcdDrawTextIndented(y, STR_REPEAT_AT_ZERO);
        lcdDrawNumber(LCD_W-14, y, VARIO_REPEAT_ZERO+(g_eeGeneral.varioRepeat*10), attr|RIGHT);
        lcdDrawText(lcdLastRightPos, y, STR_MS, attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.varioRepeat, -30, 50);
        break;
#endif

#if defined(HAPTIC)
      case ITEM_RADIO_SETUP_HAPTIC_LABEL:
        lcdDrawTextAlignedLeft(y, STR_HAPTIC_LABEL);
        break;

      case ITEM_RADIO_SETUP_HAPTIC_MODE:
        g_eeGeneral.hapticMode = editChoice(LCD_W-2, y, STR_MODE, STR_VBEEPMODE, g_eeGeneral.hapticMode, -2, 1, attr|RIGHT, event, INDENT_WIDTH);
        break;

      case ITEM_RADIO_SETUP_HAPTIC_LENGTH:
        g_eeGeneral.hapticLength = slider_5pos(y, g_eeGeneral.hapticLength, event, attr, STR_LENGTH);
        break;

      case ITEM_RADIO_SETUP_HAPTIC_STRENGTH:
        g_eeGeneral.hapticStrength = slider_5pos(y, g_eeGeneral.hapticStrength, event, attr, STR_STRENGTH);
        break;
#endif

#if defined(IMU)
      case ITEM_RADIO_SETUP_IMU_LABEL:
        lcdDrawTextAlignedLeft(y, STR_IMU_LABEL);
        break;

      case ITEM_RADIO_SETUP_IMU_MAX:
        {
          lcdDrawTextIndented(y, STR_IMU_MAX);
          lcdDrawNumber(LCD_W-7, y, IMU_MAX_DEFAULT + g_eeGeneral.imuMax, attr|RIGHT);
          coord_t lp = lcdLastLeftPos - 2;
          lcdDrawChar(lcdLastRightPos, y, STR_CHAR_BW_DEGREE, attr);
          if (attr) {
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.imuMax, IMU_MAX_DEFAULT - IMU_MAX_RANGE, IMU_MAX_DEFAULT + IMU_MAX_RANGE);
            lcdDrawText(lp, y, ")", RIGHT);
            lcdDrawNumber(lcdLastLeftPos, y, max(abs(gyro.outputs[0]), abs(gyro.outputs[1])) * 180 / 1024, RIGHT);
            lcdDrawText(lcdLastLeftPos, y, "(", RIGHT);
          }
        }
        break;

      case ITEM_RADIO_SETUP_IMU_OFFSET:
        {
          lcdDrawTextIndented(y, STR_IMU_OFFSET);
          lcdDrawNumber(LCD_W-7, y, g_eeGeneral.imuOffset, attr|RIGHT);
          coord_t lp = lcdLastLeftPos - 2;
          lcdDrawChar(lcdLastRightPos, y, STR_CHAR_BW_DEGREE, attr);
          if (attr) {
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.imuOffset, IMU_OFFSET_MIN, IMU_OFFSET_MAX);
            lcdDrawText(lp, y, ")", RIGHT);
            lcdDrawNumber(lcdLastLeftPos, y, gyro.outputs[0] * 180 / 1024, RIGHT);
            lcdDrawText(lcdLastLeftPos, y, "(", RIGHT);
          }
        }
        break;
#endif

      case ITEM_RADIO_SETUP_ALARMS_LABEL:
        expandState.alarms = expandableSection(y, STR_ALARMS_LABEL, expandState.alarms, attr, event);
        break;

      case ITEM_RADIO_SETUP_BATTERY_WARNING:
        lcdDrawTextIndented(y, STR_BATTERYWARNING);
        putsVolts(LCD_W-7, y, g_eeGeneral.vBatWarn, attr|RIGHT);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatWarn, 30, 120); //3-12V
        break;

      case ITEM_RADIO_SETUP_MEMORY_WARNING:
      {
        uint8_t b = 1 - g_eeGeneral.disableMemoryWarning;
        g_eeGeneral.disableMemoryWarning = 1 - editCheckBox(b, LCD_W-9, y, STR_MEMORYWARNING, attr, event, INDENT_WIDTH);
        break;
      }

      case ITEM_RADIO_SETUP_ALARM_WARNING:
      {
        uint8_t b = 1 - g_eeGeneral.disableAlarmWarning;
        g_eeGeneral.disableAlarmWarning = 1 - editCheckBox(b, LCD_W-9, y, STR_ALARMWARNING, attr, event, INDENT_WIDTH);
        break;
      }

      case ITEM_RADIO_SETUP_RSSI_POWEROFF_ALARM:
      {
        uint8_t b = 1 - g_eeGeneral.disableRssiPoweroffAlarm;
        g_eeGeneral.disableRssiPoweroffAlarm = 1 - editCheckBox(b, LCD_W-9, y, STR_RSSI_SHUTDOWN_ALARM, attr, event, INDENT_WIDTH);
        break;
      }

      case ITEM_RADIO_SETUP_TRAINER_POWEROFF_ALARM:
      {
        lcdDrawText(INDENT_WIDTH, y, STR_TRAINER_SHUTDOWN_ALARM);
        uint8_t b = 1 - g_eeGeneral.disableTrainerPoweroffAlarm;
        g_eeGeneral.disableTrainerPoweroffAlarm = 1 - editCheckBox(b, LCD_W-9, y, nullptr, attr, event);
        break;
      }

      case ITEM_RADIO_SETUP_INACTIVITY_ALARM:
        lcdDrawTextIndented(y, STR_INACTIVITYALARM);
        lcdDrawNumber(LCD_W-7, y, g_eeGeneral.inactivityTimer, attr|RIGHT);
        lcdDrawChar(lcdLastRightPos, y, 'm');
        if(attr) g_eeGeneral.inactivityTimer = checkIncDec(event, g_eeGeneral.inactivityTimer, 0, 250, EE_GENERAL); //0..250minutes
        break;

#if defined(BACKLIGHT_GPIO) || defined(OLED_SCREEN)
      case ITEM_RADIO_SETUP_BACKLIGHT_LABEL:
#if defined(OLED_SCREEN)
        lcdDrawTextAlignedLeft(y, STR_BRIGHTNESS);
#else
        lcdDrawTextAlignedLeft(y, STR_BACKLIGHT_LABEL);
#endif
        break;

      case ITEM_RADIO_SETUP_BACKLIGHT_MODE:
        g_eeGeneral.backlightMode = editChoice(LCD_W-2, y, STR_MODE, STR_VBLMODE, g_eeGeneral.backlightMode, e_backlight_mode_off, e_backlight_mode_on, attr|RIGHT, event, INDENT_WIDTH);
        break;

      case ITEM_RADIO_SETUP_FLASH_BEEP:
        g_eeGeneral.alarmsFlash = editCheckBox(g_eeGeneral.alarmsFlash, LCD_W-9, y, STR_ALARM, attr, event, INDENT_WIDTH) ;
        break;

      case ITEM_RADIO_SETUP_BACKLIGHT_DELAY:
        lcdDrawTextIndented(y, STR_DURATION);
        lcdDrawNumber(LCD_W-7, y, g_eeGeneral.lightAutoOff*5, attr|RIGHT);
        lcdDrawChar(lcdLastRightPos, y, 's');
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.lightAutoOff, 0, 600/5);
        break;

      case ITEM_RADIO_SETUP_BRIGHTNESS:
        lcdDrawTextIndented(y, STR_BRIGHTNESS);
#if defined(OLED_SCREEN)
        lcdDrawNumber(LCD_W-2, y, g_eeGeneral.contrast, attr|RIGHT);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.contrast, LCD_CONTRAST_MIN, LCD_CONTRAST_MAX);
          lcdSetContrast();
#else
        lcdDrawNumber(LCD_W-2, y, 100-g_eeGeneral.backlightBright, attr|RIGHT) ;
        if (attr) {
          uint8_t b = 100 - g_eeGeneral.backlightBright;
          CHECK_INCDEC_GENVAR(event, b, 0, 100);
          g_eeGeneral.backlightBright = 100 - b;
#endif
        }
        break;
#endif

#if !defined(OLED_SCREEN)
      case ITEM_RADIO_SETUP_CONTRAST:
        lcdDrawTextIndented(y, STR_CONTRAST);
        lcdDrawNumber(LCD_W-2, y, g_eeGeneral.contrast, attr|RIGHT);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.contrast, LCD_CONTRAST_MIN, LCD_CONTRAST_MAX);
          lcdSetContrast();
        }
        break;
#endif

      case ITEM_RADIO_SETUP_DISABLE_SPLASH:
      {
        lcdDrawTextAlignedLeft(y, STR_SPLASHSCREEN);
        if (SPLASH_NEEDED()) {
          lcdDrawNumber(LCD_W-7, y, SPLASH_TIMEOUT/100, attr|RIGHT);
          lcdDrawChar(lcdLastRightPos, y, 's');
        }
        else {
          lcdDrawMMM(LCD_W-2, y, attr|RIGHT);
        }
        if (attr) g_eeGeneral.splashMode = -checkIncDecGen(event, -g_eeGeneral.splashMode, -3, 4);
        break;
      }

#if defined(PWR_BUTTON_PRESS)
      case ITEM_RADIO_SETUP_PWR_ON_SPEED:
        g_eeGeneral.pwrOnSpeed = pwrDelayToYaml(editChoice(LCD_W-2, y, STR_PWR_ON_DELAY, STR_PWR_OFF_DELAYS, pwrDelayFromYaml(g_eeGeneral.pwrOnSpeed), 0, 4, attr|RIGHT, event));
        break;

      case ITEM_RADIO_SETUP_PWR_OFF_SPEED:
        g_eeGeneral.pwrOffSpeed = pwrDelayToYaml(editChoice(LCD_W-2, y, STR_PWR_OFF_DELAY, STR_PWR_OFF_DELAYS, pwrDelayFromYaml(g_eeGeneral.pwrOffSpeed), 0, 4, attr|RIGHT, event));
        break;

      case ITEM_RADIO_SETUP_PWR_AUTO_OFF:
        lcdDrawTextAlignedLeft(y, STR_PWR_AUTO_OFF);
        lcdDrawNumber(LCD_W-7, y, g_eeGeneral.pwrOffIfInactive, attr|RIGHT);
        lcdDrawChar(lcdLastRightPos, y, 'm');
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.pwrOffIfInactive, 0, 255);
        break;
#endif

#if defined(HAPTIC)
      case ITEM_RADIO_SETUP_PWR_ON_OFF_HAPTIC: {
        lcdDrawTextAlignedLeft(y, STR_PWR_ON_OFF_HAPTIC);
        g_eeGeneral.disablePwrOnOffHaptic =
            !editCheckBox(!g_eeGeneral.disablePwrOnOffHaptic, LCD_W - 9, y,
                          nullptr, attr, event);
        break;
      }
#endif

#if defined(PXX2)
      case ITEM_RADIO_SETUP_OWNER_ID:
        lcdDrawTextAlignedLeft(y, STR_OWNER_ID);
        if (attr)
          editName(RADIO_SETUP_2ND_COLUMN, y,
                   g_eeGeneral.ownerRegistrationID,
                   PXX2_LEN_REGISTRATION_ID, event, attr, 0,
                   old_editMode);
        else
          lcdDrawSizedText(LCD_W-2, y, g_eeGeneral.ownerRegistrationID, PXX2_LEN_REGISTRATION_ID, RIGHT);
        break;
#endif

#if defined(GPS)
      case ITEM_RADIO_SETUP_LABEL_GPS:
        lcdDrawTextAlignedLeft(y, STR_GPS);
        break;

      case ITEM_RADIO_SETUP_TIMEZONE:
        {
          lcdDrawTextIndented(y, STR_TIMEZONE);
          int tzIndex = timezoneIndex(g_eeGeneral.timezone, g_eeGeneral.timezoneMinutes);
          lcdDrawText(LCD_W-2, y, timezoneDisplay(tzIndex).c_str(), attr|RIGHT);
          if (attr) {
            tzIndex = checkIncDec(event, tzIndex, minTimezone(), maxTimezone(), EE_GENERAL);
            if (checkIncDec_Ret) {
              g_eeGeneral.timezone = timezoneHour(tzIndex);
              g_eeGeneral.timezoneMinutes = timezoneMinute(tzIndex);
            }
          }
        }
        break;

      case ITEM_RADIO_SETUP_ADJUST_RTC:
        g_eeGeneral.adjustRTC = editCheckBox(g_eeGeneral.adjustRTC, LCD_W-9, y, STR_ADJUST_RTC, attr, event, INDENT_WIDTH);
        break;

      case ITEM_RADIO_SETUP_GPSFORMAT:
        g_eeGeneral.gpsFormat = editChoice(LCD_W-2, y, STR_GPS_COORDS_FORMAT, STR_GPSFORMAT, g_eeGeneral.gpsFormat, 0, 1, attr|RIGHT, event, INDENT_WIDTH);
        break;
#endif

#if defined(PXX1)
      case ITEM_RADIO_SETUP_COUNTRYCODE:
        g_eeGeneral.countryCode = editChoice(LCD_W-2, y, STR_COUNTRY_CODE, STR_COUNTRY_CODES, g_eeGeneral.countryCode, 0, 2, attr|RIGHT, event);
        break;
#endif

      case ITEM_RADIO_SETUP_LANGUAGE:
        lcdDrawTextAlignedLeft(y, STR_VOICE_LANGUAGE);
        lcdDrawText(LCD_W-2, y, currentLanguagePack->name, attr|RIGHT);
        if (attr) {
          currentLanguagePackIdx = checkIncDec(event, currentLanguagePackIdx, 0, DIM(languagePacks)-2, EE_GENERAL);
          if (checkIncDec_Ret) {
            currentLanguagePack = languagePacks[currentLanguagePackIdx];
            strncpy(g_eeGeneral.ttsLanguage, currentLanguagePack->id, 2);
          }
        }
        break;

      case ITEM_RADIO_SETUP_IMPERIAL:
        g_eeGeneral.imperial = editChoice(LCD_W-2, y, STR_UNITS_SYSTEM, STR_VUNITSSYSTEM, g_eeGeneral.imperial, 0, 1, attr|RIGHT, event);
        break;

      case ITEM_RADIO_SETUP_PPM:
        g_eeGeneral.ppmunit = editChoice(LCD_W-2, y, STR_UNITS_PPM, STR_PPMUNIT, g_eeGeneral.ppmunit, PPM_PERCENT_PREC0, PPM_US, attr|RIGHT, event);
        break;

#if defined(FAI_CHOICE)
      case ITEM_RADIO_SETUP_FAI:
        editCheckBox(g_eeGeneral.fai, LCD_W-9, y, "FAI Mode", attr, event);
        if (attr && checkIncDec_Ret) {
          if (g_eeGeneral.fai)
            POPUP_WARNING("FAI\001mode blocked!");
          else
            POPUP_CONFIRMATION("FAI mode?", nullptr);
        }
        break;
#endif


      case ITEM_RADIO_SETUP_SWITCHES_DELAY:
        lcdDrawTextAlignedLeft(y, STR_SWITCHES_DELAY);
        lcdDrawNumber(LCD_W-14, y, 10*SWITCHES_DELAY(), attr|RIGHT);
        lcdDrawText(lcdLastRightPos, y, STR_MS, attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.switchesDelay, -15, 100-15);
        break;

      case ITEM_RADIO_SETUP_USB_MODE:
        g_eeGeneral.USBMode = editChoice(LCD_W-2, y, STR_USBMODE, STR_USBMODES, g_eeGeneral.USBMode, USB_UNSELECTED_MODE, USB_MAX_MODE, attr|RIGHT, event);
        break;

#if defined(JACK_DETECT_GPIO)
      case ITEM_RADIO_SETUP_JACK_MODE:
        g_eeGeneral.jackMode = editChoice(LCD_W-2, y, STR_JACK_MODE, STR_JACK_MODES, g_eeGeneral.jackMode, JACK_UNSELECTED_MODE, JACK_MAX_MODE, attr|RIGHT, event);
        break;
#endif

      case ITEM_RADIO_SETUP_RX_CHANNEL_ORD:
        lcdDrawTextAlignedLeft(y, STR_DEF_CHAN_ORD); // RAET->AETR
        {
          for (uint8_t i = 0; i < adcGetMaxInputs(ADC_INPUT_MAIN); i++) {
            putsChnLetter(LCD_W - 2 - (adcGetMaxInputs(ADC_INPUT_MAIN)-i)*FW, y, inputMappingChannelOrder(i), attr);
          }
          if (attr) {
            auto max_order = inputMappingGetMaxChannelOrder() - 1;
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.templateSetup, 0, max_order);
          }
        }
        break;

#if !defined(SURFACE_RADIO)
      case ITEM_RADIO_SETUP_STICK_MODE_LABELS:
        lcdDrawTextAlignedLeft(y, STR_MODE);
        for (uint8_t i=0; i<4; i++) {
          lcdDraw1bitBitmap(5*FW+i*(4*FW+2), y, sticks, i, 0);
        }
        break;

      case ITEM_RADIO_SETUP_STICK_MODE:
        lcdDrawChar(2*FW, y, '1'+reusableBuffer.generalSettings.stickMode, attr);
        {
          auto controls = adcGetMaxInputs(ADC_INPUT_MAIN);
          auto mode = reusableBuffer.generalSettings.stickMode;
          for (uint8_t i = 0; i < controls; i++) {
            source_t src = MIXSRC_FIRST_STICK + inputMappingConvertMode(mode, i);
            drawSource((5 * FW - 3) + i * (4 * FW + 2), y, src, 0);
          }
        }
        if (attr && s_editMode > 0) {
          CHECK_INCDEC_GENVAR(event, reusableBuffer.generalSettings.stickMode, 0, 3);
        } else if (reusableBuffer.generalSettings.stickMode !=
                   g_eeGeneral.stickMode) {
          mixerTaskStop();
          g_eeGeneral.stickMode = reusableBuffer.generalSettings.stickMode;
          checkThrottleStick();
          mixerTaskStart();
          waitKeysReleased();
        }
        break;
#endif
#if defined(ROTARY_ENCODER_NAVIGATION)
      case ITEM_RADIO_SETUP_ROTARY_ENC_MODE:
        lcdDrawTextAlignedLeft(y, STR_ROTARY_ENC_MODE);
        lcdDrawTextAtIndex(LCD_W-2, y, STR_ROTARY_ENC_OPT,
                           reusableBuffer.generalSettings.rotaryEncoderMode,
                           attr|RIGHT);
        if (attr && s_editMode > 0) {
          CHECK_INCDEC_GENVAR(event,
                              reusableBuffer.generalSettings.rotaryEncoderMode,
                              ROTARY_ENCODER_MODE_NORMAL,
                              ROTARY_ENCODER_MODE_LAST);
        } else if (reusableBuffer.generalSettings.rotaryEncoderMode !=
                   g_eeGeneral.rotEncMode) {
          g_eeGeneral.rotEncMode =
              reusableBuffer.generalSettings.rotaryEncoderMode;
        }
        break;
#endif

      case ITEM_VIEW_OPTIONS_LABEL:
        expandState.viewOpt = expandableSection(y, STR_ENABLED_FEATURES, expandState.viewOpt, attr, event);
        break;
      case ITEM_VIEW_OPTIONS_RADIO_TAB:
        lcdDrawText(INDENT_WIDTH-2, y, TR_RADIO_MENU_TABS);
        break;
      case ITEM_VIEW_OPTIONS_GF:
        g_eeGeneral.radioGFDisabled = viewOptCheckBox(y, STR_MENUSPECIALFUNCS, g_eeGeneral.radioGFDisabled, attr, event, g_model.radioGFDisabled);
        break;
      case ITEM_VIEW_OPTIONS_TRAINER:
        g_eeGeneral.radioTrainerDisabled = viewOptCheckBox(y, STR_MENUTRAINER, g_eeGeneral.radioTrainerDisabled, attr, event, g_model.radioTrainerDisabled);
        break;
      case ITEM_VIEW_OPTIONS_MODEL_TAB:
        lcdDrawText(INDENT_WIDTH-2, y, TR_MODEL_MENU_TABS);
        break;
#if defined(HELI)
      case ITEM_VIEW_OPTIONS_HELI:
        g_eeGeneral.modelHeliDisabled = viewOptCheckBox(y, STR_MENUHELISETUP, g_eeGeneral.modelHeliDisabled, attr, event, g_model.modelHeliDisabled);
        break;
#endif
#if defined(FLIGHT_MODES)
      case ITEM_VIEW_OPTIONS_FM:
        g_eeGeneral.modelFMDisabled = viewOptCheckBox(y, STR_MENUFLIGHTMODES, g_eeGeneral.modelFMDisabled, attr, event, g_model.modelFMDisabled);
        break;
#endif
      case ITEM_VIEW_OPTIONS_CURVES:
        g_eeGeneral.modelCurvesDisabled = viewOptCheckBox(y, STR_MENUCURVES, g_eeGeneral.modelCurvesDisabled, attr, event, g_model.modelCurvesDisabled);
        break;
      case ITEM_VIEW_OPTIONS_LS:
        g_eeGeneral.modelLSDisabled = viewOptCheckBox(y, STR_MENULOGICALSWITCHES, g_eeGeneral.modelLSDisabled, attr, event, g_model.modelLSDisabled);
        break;
      case ITEM_VIEW_OPTIONS_SF:
        g_eeGeneral.modelSFDisabled = viewOptCheckBox(y, STR_MENUCUSTOMFUNC, g_eeGeneral.modelSFDisabled, attr, event, g_model.modelSFDisabled);
        break;
#if defined(LUA_MODEL_SCRIPTS)
      case ITEM_VIEW_OPTIONS_CUSTOM_SCRIPTS:
        g_eeGeneral.modelCustomScriptsDisabled = viewOptCheckBox(y, STR_MENUCUSTOMSCRIPTS, g_eeGeneral.modelCustomScriptsDisabled, attr, event, g_model.modelCustomScriptsDisabled);
        break;
#endif
      case ITEM_VIEW_OPTIONS_TELEMETRY:
        g_eeGeneral.modelTelemetryDisabled = viewOptCheckBox(y, STR_MENUTELEMETRY, g_eeGeneral.modelTelemetryDisabled, attr, event, g_model.modelTelemetryDisabled);
        break;
    }
  }
}
