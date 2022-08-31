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

#include "opentx.h"

const unsigned char sticks[]  = {
#include "sticks.lbm"
};

#define RADIO_SETUP_2ND_COLUMN  79

#define SLIDER_5POS(y, value, event, attr) { \
  int8_t tmp = value; \
  drawSlider(RADIO_SETUP_2ND_COLUMN, y, LCD_W - 2 - RADIO_SETUP_2ND_COLUMN, 2+tmp, 4, attr); \
  value = editChoice(RADIO_SETUP_2ND_COLUMN, y, nullptr, nullptr, tmp, -2, +2, attr, event); \
}

#if defined(SPLASH)
  #define CASE_SPLASH_PARAM(x) x,
#else
  #define CASE_SPLASH_PARAM(x)
#endif

#if defined(BATTGRAPH)
  #define CASE_BATTGRAPH(x) x,
#else
  #define CASE_BATTGRAPH(x)
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
  ITEM_RADIO_SETUP_CONTRAST,
  ITEM_RADIO_SETUP_ALARMS_LABEL,
  ITEM_RADIO_SETUP_BATTERY_WARNING,
  CASE_CAPACITY(ITEM_RADIO_SETUP_CAPACITY_WARNING)
  ITEM_RADIO_SETUP_INACTIVITY_ALARM,
  ITEM_RADIO_SETUP_MEMORY_WARNING,
  ITEM_RADIO_SETUP_ALARM_WARNING,
  ITEM_RADIO_SETUP_RSSI_POWEROFF_ALARM,
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_LABEL)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_MODE)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_DELAY)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_SOURCE)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_BRIGHTNESS)
  CASE_PWM_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_BRIGHTNESS_OFF)
  CASE_PWM_BACKLIGHT(ITEM_RADIO_SETUP_BACKLIGHT_BRIGHTNESS_ON)
  CASE_BACKLIGHT(ITEM_RADIO_SETUP_FLASH_BEEP)
  CASE_SPLASH_PARAM(ITEM_RADIO_SETUP_DISABLE_SPLASH)
  CASE_PWR_BUTTON_PRESS(ITEM_RADIO_SETUP_PWR_ON_SPEED)
  CASE_PWR_BUTTON_PRESS(ITEM_RADIO_SETUP_PWR_OFF_SPEED)
  CASE_PXX2(ITEM_RADIO_SETUP_OWNER_ID)
  CASE_GPS(ITEM_RADIO_SETUP_TIMEZONE)
  ITEM_RADIO_SETUP_ADJUST_RTC,
  CASE_GPS(ITEM_RADIO_SETUP_GPSFORMAT)
  CASE_PXX1(ITEM_RADIO_SETUP_COUNTRYCODE)
  ITEM_RADIO_SETUP_LANGUAGE,
  ITEM_RADIO_SETUP_IMPERIAL,
  IF_FAI_CHOICE(ITEM_RADIO_SETUP_FAI)
  ITEM_RADIO_SETUP_SWITCHES_DELAY,
  ITEM_RADIO_SETUP_USB_MODE,
  CASE_JACK_DETECT(ITEM_RADIO_SETUP_JACK_MODE)
  ITEM_RADIO_SETUP_RX_CHANNEL_ORD,
  CASE_ROTARY_ENCODER(ITEM_RADIO_SETUP_ROTARY_ENC_MODE)
  ITEM_RADIO_SETUP_STICK_MODE_LABELS,
  ITEM_RADIO_SETUP_STICK_MODE,
  ITEM_RADIO_SETUP_MAX
};

#if defined(FRSKY_STICKS) && !defined(PCBTARANIS)
  #define COL_TX_MODE 0
#else
  #define COL_TX_MODE LABEL(TX_MODE)
#endif

void menuRadioSetup(event_t event)
{
#if defined(RTCLOCK)
  struct gtm t;
  gettime(&t);

  if ((menuVerticalPosition==ITEM_RADIO_SETUP_DATE+HEADER_LINE || menuVerticalPosition==ITEM_RADIO_SETUP_TIME+HEADER_LINE) &&
      (s_editMode>0) &&
      (event==EVT_KEY_FIRST(KEY_ENTER) || event==EVT_KEY_FIRST(KEY_EXIT) || IS_ROTARY_BREAK(event) || IS_ROTARY_LONG(event))) {
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

  uint8_t old_editMode = s_editMode;

  MENU(STR_RADIO_SETUP, menuTabGeneral, MENU_RADIO_SETUP, HEADER_LINE+ITEM_RADIO_SETUP_MAX, {
    HEADER_LINE_COLUMNS CASE_RTCLOCK(2) CASE_RTCLOCK(2) CASE_BATTGRAPH(1)
    LABEL(SOUND), CASE_AUDIO(0)
    CASE_BUZZER(0)
    0, 0, 0, 0, 0, CASE_AUDIO(0)
    CASE_VARIO(LABEL(VARIO))
    CASE_VARIO(0)
    CASE_VARIO(0)
    CASE_VARIO(0)
    CASE_VARIO(0)
    CASE_HAPTIC(LABEL(HAPTIC))
    CASE_HAPTIC(0)
    CASE_HAPTIC(0)
    CASE_HAPTIC(0)
    CASE_IMU(LABEL(IMU))
    CASE_IMU(0)
    CASE_IMU(0)
    0, LABEL(ALARMS), 0, CASE_CAPACITY(0)
    0, 0, 0, 0, /* ITEM_RADIO_SETUP_INACTIVITY_ALARM ITEM_RADIO_SETUP_MEMORY_WARNING ITEM_RADIO_SETUP_ALARM_WARNING ITEM_RADIO_SETUP_RSSI_POWEROFF_ALARM */
    CASE_BACKLIGHT(LABEL(BACKLIGHT))
    CASE_BACKLIGHT(0)
    CASE_BACKLIGHT(0)
    CASE_BACKLIGHT(0)
    CASE_PWM_BACKLIGHT(0)
    CASE_PWM_BACKLIGHT(0)
    CASE_BACKLIGHT(0)
    CASE_SPLASH_PARAM(0)
    CASE_PWR_BUTTON_PRESS(0)
    CASE_PWR_BUTTON_PRESS(0)
    CASE_PXX2(0) /* owner registration ID */

    CASE_GPS(0)
    0, CASE_GPS(0)
    CASE_PXX1(0)
    0, 0, IF_FAI_CHOICE(0)
    0,
    0, // USB mode
    CASE_JACK_DETECT(0) // Jack mode
    CASE_ROTARY_ENCODER(0)
    0, COL_TX_MODE, 0, 1/*to force edit mode*/});

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
    uint8_t blink = ((s_editMode>0) ? BLINK|INVERS : INVERS);
    uint8_t attr = (sub == k ? blink : 0);

    switch (k) {
#if defined(RTCLOCK)
      case ITEM_RADIO_SETUP_DATE:
        lcdDrawTextAlignedLeft(y, STR_DATE);
        for (uint8_t j=0; j<3; j++) {
          uint8_t rowattr = (menuHorizontalPosition==j ? attr : 0);
          switch (j) {
            case 0:
              lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, t.tm_year+TM_YEAR_BASE, rowattr);
              lcdDrawChar(lcdNextPos, y, '-');
              if (rowattr && s_editMode > 0) t.tm_year = checkIncDec(event, t.tm_year, 112, 200, 0);
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
              lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, t.tm_hour, rowattr|LEADING0, 2);
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
        putsVolts(RADIO_SETUP_2ND_COLUMN, y,  90+g_eeGeneral.vBatMin, (menuHorizontalPosition==0 ? attr : 0)|LEFT|NO_UNIT);
        lcdDrawChar(lcdLastRightPos, y, '-');
        putsVolts(lcdLastRightPos+FW, y, 120+g_eeGeneral.vBatMax, (menuHorizontalPosition>0 ? attr : 0)|LEFT|NO_UNIT);
        if (attr && s_editMode>0) {
          if (menuHorizontalPosition==0)
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatMin, -60, g_eeGeneral.vBatMax+29); // min=3.0V
          else
            CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatMax, g_eeGeneral.vBatMin-29, +40); // max=16.0V
        }
        break;
#endif

      case ITEM_RADIO_SETUP_SOUND_LABEL:
        lcdDrawTextAlignedLeft(y, STR_SOUND_LABEL);
        break;

#if defined(AUDIO)
      case ITEM_RADIO_SETUP_BEEP_MODE:
        g_eeGeneral.beepMode = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_SPEAKER, STR_VBEEPMODE, g_eeGeneral.beepMode, -2, 1, attr, event);
        break;

#if defined(BUZZER) // AUDIO + BUZZER
      case ITEM_RADIO_SETUP_BUZZER_MODE:
        g_eeGeneral.buzzerMode = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_BUZZER, STR_VBEEPMODE, g_eeGeneral.buzzerMode, -2, 1, attr, event);
        break;
#endif
#elif defined(BUZZER) // BUZZER only
      case ITEM_RADIO_SETUP_BUZZER_MODE:
        g_eeGeneral.beepMode = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_SPEAKER, STR_VBEEPMODE, g_eeGeneral.beepMode, -2, 1, attr, event);
        break;
#endif

#if defined(VOICE)
      case ITEM_RADIO_SETUP_SPEAKER_VOLUME:
      {
        lcdDrawText(INDENT_WIDTH, y, STR_VOLUME);
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
        lcdDrawTextAlignedLeft(y, STR_BEEP_VOLUME);
        SLIDER_5POS(y, g_eeGeneral.beepVolume, event, attr);
        break;

      case ITEM_RADIO_SETUP_WAV_VOLUME:
        lcdDrawTextAlignedLeft(y, STR_WAV_VOLUME);
        SLIDER_5POS(y, g_eeGeneral.wavVolume, event, attr);
        break;

      case ITEM_RADIO_SETUP_BACKGROUND_VOLUME:
        lcdDrawTextAlignedLeft(y, STR_BG_VOLUME);
        SLIDER_5POS(y, g_eeGeneral.backgroundVolume, event, attr);
        break;

      case ITEM_RADIO_SETUP_BEEP_LENGTH:
        lcdDrawText(INDENT_WIDTH, y, STR_BEEP_LENGTH);
        SLIDER_5POS(y, g_eeGeneral.beepLength, event, attr);
        break;

#if defined(AUDIO)
      case ITEM_RADIO_SETUP_SPEAKER_PITCH:
        lcdDrawText(INDENT_WIDTH, y, STR_BEEP_PITCH);
        lcdDrawChar(RADIO_SETUP_2ND_COLUMN, y, '+', attr);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN+FW, y, g_eeGeneral.speakerPitch*15, attr|LEFT);
        lcdDrawText(lcdLastRightPos, y, "Hz", attr);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.speakerPitch, 0, 20);
        }
        break;
#endif

#if defined(VARIO)
      case ITEM_RADIO_SETUP_VARIO_LABEL:
        lcdDrawTextAlignedLeft(y, STR_VARIO);
        break;

      case ITEM_RADIO_SETUP_VARIO_VOLUME:
        lcdDrawText(INDENT_WIDTH, y, STR_VOLUME);
        SLIDER_5POS(y, g_eeGeneral.varioVolume, event, attr);
        break;

      case ITEM_RADIO_SETUP_VARIO_PITCH:
        lcdDrawText(INDENT_WIDTH, y, STR_PITCH_AT_ZERO);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, VARIO_FREQUENCY_ZERO+(g_eeGeneral.varioPitch*10), attr|LEFT);
        lcdDrawText(lcdLastRightPos, y, "Hz", attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.varioPitch, -40, 40);
        break;

      case ITEM_RADIO_SETUP_VARIO_RANGE:
        lcdDrawText(INDENT_WIDTH, y, STR_PITCH_AT_MAX);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, VARIO_FREQUENCY_ZERO+(g_eeGeneral.varioPitch*10)+VARIO_FREQUENCY_RANGE+(g_eeGeneral.varioRange*10), attr|LEFT);
        lcdDrawText(lcdLastRightPos, y, "Hz", attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.varioRange, -80, 80);
        break;

      case ITEM_RADIO_SETUP_VARIO_REPEAT:
        lcdDrawText(INDENT_WIDTH, y, STR_REPEAT_AT_ZERO);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, VARIO_REPEAT_ZERO+(g_eeGeneral.varioRepeat*10), attr|LEFT);
        lcdDrawText(lcdLastRightPos, y, STR_MS, attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.varioRepeat, -30, 50);
        break;
#endif

#if defined(HAPTIC)
      case ITEM_RADIO_SETUP_HAPTIC_LABEL:
        lcdDrawTextAlignedLeft(y, STR_HAPTIC_LABEL);
        break;

      case ITEM_RADIO_SETUP_HAPTIC_MODE:
        g_eeGeneral.hapticMode = editChoice(RADIO_SETUP_2ND_COLUMN, y, INDENT TR_MODE, STR_VBEEPMODE, g_eeGeneral.hapticMode, -2, 1, attr, event);
        break;

      case ITEM_RADIO_SETUP_HAPTIC_LENGTH:
        lcdDrawText(INDENT_WIDTH, y, STR_LENGTH);
        SLIDER_5POS(y, g_eeGeneral.hapticLength, event, attr);
        break;

      case ITEM_RADIO_SETUP_HAPTIC_STRENGTH:
        lcdDrawText(INDENT_WIDTH, y, STR_STRENGTH);
        SLIDER_5POS(y, g_eeGeneral.hapticStrength, event, attr);
        break;
#endif

#if defined(IMU)
      case ITEM_RADIO_SETUP_IMU_LABEL:
        lcdDrawTextAlignedLeft(y, STR_IMU_LABEL);
        break;

      case ITEM_RADIO_SETUP_IMU_MAX:
        lcdDrawText(INDENT_WIDTH, y, STR_IMU_MAX);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, IMU_MAX_DEFAULT + g_eeGeneral.imuMax, attr|LEFT);
        lcdDrawChar(lcdLastRightPos, y, '@', attr);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.imuMax, IMU_MAX_DEFAULT - IMU_MAX_RANGE, IMU_MAX_DEFAULT + IMU_MAX_RANGE);
          lcdDrawText(LCD_W-4*FW, y, "(");
          lcdDrawNumber(lcdLastRightPos, y, max(abs(gyro.outputs[0]), abs(gyro.outputs[1])) * 180 / 1024);
          lcdDrawText(lcdLastRightPos, y, ")");
        }
        break;

      case ITEM_RADIO_SETUP_IMU_OFFSET:
        lcdDrawText(INDENT_WIDTH, y, STR_IMU_OFFSET);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.imuOffset, attr|LEFT);
        lcdDrawChar(lcdLastRightPos, y, '@', attr);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.imuOffset, IMU_OFFSET_MIN, IMU_OFFSET_MAX);
          lcdDrawText(LCD_W-4*FW, y, "(");
          lcdDrawNumber(lcdLastRightPos, y, gyro.outputs[0] * 180 / 1024);
          lcdDrawText(lcdLastRightPos, y, ")");
        }
        break;
#endif

      case ITEM_RADIO_SETUP_CONTRAST:
        lcdDrawTextAlignedLeft(y, STR_CONTRAST);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.contrast, attr|LEFT);
        if (attr) {
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.contrast, LCD_CONTRAST_MIN, LCD_CONTRAST_MAX);
          lcdSetContrast();
        }
        break;

      case ITEM_RADIO_SETUP_ALARMS_LABEL:
        lcdDrawTextAlignedLeft(y, STR_ALARMS_LABEL);
        break;

      case ITEM_RADIO_SETUP_BATTERY_WARNING:
        lcdDrawText(INDENT_WIDTH, y, STR_BATTERYWARNING);
        putsVolts(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.vBatWarn, attr|LEFT);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.vBatWarn, 30, 120); //3-12V
        break;

      case ITEM_RADIO_SETUP_MEMORY_WARNING:
      {
        lcdDrawText(INDENT_WIDTH, y, STR_MEMORYWARNING);
        uint8_t b = 1 - g_eeGeneral.disableMemoryWarning;
        g_eeGeneral.disableMemoryWarning = 1 - editCheckBox(b, RADIO_SETUP_2ND_COLUMN, y, nullptr, attr, event);
        break;
      }

      case ITEM_RADIO_SETUP_ALARM_WARNING:
      {
        lcdDrawText(INDENT_WIDTH, y, STR_ALARMWARNING);
        uint8_t b = 1 - g_eeGeneral.disableAlarmWarning;
        g_eeGeneral.disableAlarmWarning = 1 - editCheckBox(b, RADIO_SETUP_2ND_COLUMN, y, nullptr, attr, event);
        break;
      }

      case ITEM_RADIO_SETUP_RSSI_POWEROFF_ALARM:
      {
        lcdDrawText(INDENT_WIDTH, y, STR_RSSI_SHUTDOWN_ALARM);
        uint8_t b = 1 - g_eeGeneral.disableRssiPoweroffAlarm;
        g_eeGeneral.disableRssiPoweroffAlarm = 1 - editCheckBox(b, RADIO_SETUP_2ND_COLUMN, y, nullptr, attr, event);
        break;
      }

#if defined(TX_CAPACITY_MEASUREMENT)
      case ITEM_RADIO_SETUP_CAPACITY_WARNING:
        lcdDrawTextAlignedLeft(y, STR_CAPAWARNING);
        drawValueWithUnit(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.mAhWarn*50, UNIT_MAH, attr|LEFT) ;
        if(attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.mAhWarn, 0, 100);
        break;
#endif

      case ITEM_RADIO_SETUP_INACTIVITY_ALARM:
        lcdDrawText(INDENT_WIDTH, y, STR_INACTIVITYALARM);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.inactivityTimer, attr|LEFT);
        lcdDrawChar(lcdLastRightPos, y, 'm');
        if(attr) g_eeGeneral.inactivityTimer = checkIncDec(event, g_eeGeneral.inactivityTimer, 0, 250, EE_GENERAL); //0..250minutes
        break;

#if defined(BACKLIGHT_GPIO)
      case ITEM_RADIO_SETUP_BACKLIGHT_LABEL:
        lcdDrawTextAlignedLeft(y, STR_BACKLIGHT_LABEL);
        break;

      case ITEM_RADIO_SETUP_BACKLIGHT_MODE:
        lcdDrawText(INDENT_WIDTH, y, STR_MODE);
        g_eeGeneral.backlightMode = editChoice(RADIO_SETUP_2ND_COLUMN, y, nullptr, STR_VBLMODE, g_eeGeneral.backlightMode, e_backlight_mode_off, e_backlight_mode_on, attr, event);
        break;

      case ITEM_RADIO_SETUP_FLASH_BEEP:
        lcdDrawText(INDENT_WIDTH, y, STR_ALARM);
        g_eeGeneral.alarmsFlash = editCheckBox(g_eeGeneral.alarmsFlash, RADIO_SETUP_2ND_COLUMN, y, nullptr, attr, event ) ;
        break;

      case ITEM_RADIO_SETUP_BACKLIGHT_DELAY:
        lcdDrawText(INDENT_WIDTH, y, STR_DURATION);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.lightAutoOff*5, attr|LEFT);
        lcdDrawChar(lcdLastRightPos, y, 's');
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.lightAutoOff, 0, 600/5);
        break;

      case ITEM_RADIO_SETUP_BACKLIGHT_SOURCE:
        lcdDrawText(INDENT_WIDTH, y, STR_SOURCE);
        drawSource(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.backlightSrc, STREXPANDED|attr);
        if (attr) {
          g_eeGeneral.backlightSrc = checkIncDec(
              event, g_eeGeneral.backlightSrc, 0, MIXSRC_LAST_SWITCH,
              EE_MODEL | INCDEC_SOURCE | NO_INCDEC_MARKS,
              isBacklightSourceAvailable);
        }
        break;
   
      case ITEM_RADIO_SETUP_BRIGHTNESS:
        lcdDrawText(INDENT_WIDTH, y, STR_BRIGHTNESS);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, 100-g_eeGeneral.backlightBright, attr|LEFT) ;
        if (attr) {
          uint8_t b = 100 - g_eeGeneral.backlightBright;
          CHECK_INCDEC_GENVAR(event, b, 0, 100);
          g_eeGeneral.backlightBright = 100 - b;
        }
        break;
#endif

#if defined(PWM_BACKLIGHT)
      case ITEM_RADIO_SETUP_BACKLIGHT_BRIGHTNESS_OFF:
        lcdDrawText(INDENT_WIDTH, y, STR_BLOFFBRIGHTNESS);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.blOffBright, attr|LEFT);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.blOffBright, 0, 15);
        break;

      case ITEM_RADIO_SETUP_BACKLIGHT_BRIGHTNESS_ON:
        lcdDrawText(INDENT_WIDTH, y, STR_BLONBRIGHTNESS);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, 15-g_eeGeneral.blOnBright, attr|LEFT);
        if (attr) g_eeGeneral.blOnBright = 15 - checkIncDecGen(event, 15-g_eeGeneral.blOnBright, 0, 15);
        break;
#endif

#if defined(SPLASH)
      case ITEM_RADIO_SETUP_DISABLE_SPLASH:
      {
        lcdDrawTextAlignedLeft(y, STR_SPLASHSCREEN);
        if (SPLASH_NEEDED()) {
          lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, SPLASH_TIMEOUT/100, attr|LEFT);
          lcdDrawChar(lcdLastRightPos, y, 's');
        }
        else {
          lcdDrawMMM(RADIO_SETUP_2ND_COLUMN, y, attr);
        }
        if (attr) g_eeGeneral.splashMode = -checkIncDecGen(event, -g_eeGeneral.splashMode, -3, 4);
        break;
      }
#endif

#if defined(PWR_BUTTON_PRESS)
      case ITEM_RADIO_SETUP_PWR_ON_SPEED:
        lcdDrawTextAlignedLeft(y, STR_PWR_ON_DELAY);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, 2 - g_eeGeneral.pwrOnSpeed, attr|LEFT);
        lcdDrawChar(lcdLastRightPos, y, 's');
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.pwrOnSpeed, -1, 2);
        break;

      case ITEM_RADIO_SETUP_PWR_OFF_SPEED:
        lcdDrawTextAlignedLeft(y, STR_PWR_OFF_DELAY);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, 2 - g_eeGeneral.pwrOffSpeed, attr|LEFT);
        lcdDrawChar(lcdLastRightPos, y, 's');
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.pwrOffSpeed, -1, 2);
        break;
#endif

#if defined(PXX2)
      case ITEM_RADIO_SETUP_OWNER_ID:
        editSingleName(RADIO_SETUP_2ND_COLUMN, y, STR_OWNER_ID,
                       g_eeGeneral.ownerRegistrationID,
                       PXX2_LEN_REGISTRATION_ID, event, attr,
                       old_editMode);
        break;
#endif

#if defined(GPS)
      case ITEM_RADIO_SETUP_TIMEZONE:
        lcdDrawTextAlignedLeft(y, STR_TIMEZONE);
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, g_eeGeneral.timezone, attr|LEFT);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.timezone, -12, 12);
        break;

      case ITEM_RADIO_SETUP_ADJUST_RTC:
        lcdDrawTextAlignedLeft(y, STR_ADJUST_RTC);
        g_eeGeneral.adjustRTC = editCheckBox(g_eeGeneral.adjustRTC, RADIO_SETUP_2ND_COLUMN, y, nullptr, attr, event);
        break;

      case ITEM_RADIO_SETUP_GPSFORMAT:
        lcdDrawTextAlignedLeft(y, STR_GPS_COORDS_FORMAT);
        g_eeGeneral.gpsFormat = editChoice(RADIO_SETUP_2ND_COLUMN, y, nullptr, STR_GPSFORMAT, g_eeGeneral.gpsFormat, 0, 1, attr, event);
        break;
#endif

#if defined(PXX1)
      case ITEM_RADIO_SETUP_COUNTRYCODE:
        g_eeGeneral.countryCode = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_COUNTRY_CODE, STR_COUNTRY_CODES, g_eeGeneral.countryCode, 0, 2, attr, event);
        break;
#endif

      case ITEM_RADIO_SETUP_LANGUAGE:
        lcdDrawTextAlignedLeft(y, STR_VOICE_LANGUAGE);
        lcdDrawText(RADIO_SETUP_2ND_COLUMN, y, currentLanguagePack->name, attr);
        if (attr) {
          currentLanguagePackIdx = checkIncDec(event, currentLanguagePackIdx, 0, DIM(languagePacks)-2, EE_GENERAL);
          if (checkIncDec_Ret) {
            currentLanguagePack = languagePacks[currentLanguagePackIdx];
            strncpy(g_eeGeneral.ttsLanguage, currentLanguagePack->id, 2);
          }
        }
        break;

      case ITEM_RADIO_SETUP_IMPERIAL:
        g_eeGeneral.imperial = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_UNITS_SYSTEM, STR_VUNITSSYSTEM, g_eeGeneral.imperial, 0, 1, attr, event);
        break;

#if defined(FAI_CHOICE)
      case ITEM_RADIO_SETUP_FAI:
        editCheckBox(g_eeGeneral.fai, RADIO_SETUP_2ND_COLUMN, y, "FAI Mode", attr, event);
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
        lcdDrawNumber(RADIO_SETUP_2ND_COLUMN, y, 10*SWITCHES_DELAY(), attr|LEFT);
        lcdDrawText(lcdLastRightPos, y, STR_MS, attr);
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.switchesDelay, -15, 100-15);
        break;

      case ITEM_RADIO_SETUP_USB_MODE:
        g_eeGeneral.USBMode = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_USBMODE, STR_USBMODES, g_eeGeneral.USBMode, USB_UNSELECTED_MODE, USB_MAX_MODE, attr, event);
        break;

#if defined(JACK_DETECT_GPIO)
      case ITEM_RADIO_SETUP_JACK_MODE:
        g_eeGeneral.jackMode = editChoice(RADIO_SETUP_2ND_COLUMN, y, STR_JACK_MODE, STR_JACK_MODES, g_eeGeneral.jackMode, JACK_UNSELECTED_MODE, JACK_MAX_MODE, attr, event);
        break;
#endif

      case ITEM_RADIO_SETUP_RX_CHANNEL_ORD:
        lcdDrawTextAlignedLeft(y, STR_RXCHANNELORD); // RAET->AETR
        for (uint8_t i=1; i<=4; i++) {
          putsChnLetter(RADIO_SETUP_2ND_COLUMN - FW + i*FW, y, channelOrder(i), attr);
        }
        if (attr) CHECK_INCDEC_GENVAR(event, g_eeGeneral.templateSetup, 0, 23);
        break;

      case ITEM_RADIO_SETUP_STICK_MODE_LABELS:
        lcdDrawTextAlignedLeft(y, STR_MODE);
        for (uint8_t i=0; i<4; i++) {
          lcdDraw1bitBitmap(5*FW+i*(4*FW+2), y, sticks, i, 0);
#if defined(FRSKY_STICKS) && !defined(PCBTARANIS)
          if (g_eeGeneral.stickReverse & (1<<i)) {
            lcdDrawFilledRect(5*FW+i*(4*FW+2), y, 3*FW, FH-1);
          }
#endif
        }
#if defined(FRSKY_STICKS) && !defined(PCBTARANIS)
        if (attr) {
          s_editMode = 0;
          CHECK_INCDEC_GENVAR(event, g_eeGeneral.stickReverse, 0, 15);
          lcdDrawRect(5*FW-1, y-1, 16*FW+2, 9);
        }
#endif
        break;

#if defined(ROTARY_ENCODER_NAVIGATION)
      case ITEM_RADIO_SETUP_ROTARY_ENC_MODE:
        lcdDrawTextAlignedLeft(y, STR_ROTARY_ENC_MODE);
        lcdDrawTextAtIndex(RADIO_SETUP_2ND_COLUMN, y, STR_ROTARY_ENC_OPT,
                           reusableBuffer.generalSettings.rotaryEncoderMode,
                           attr);
        if (attr && s_editMode > 0) {
          CHECK_INCDEC_GENVAR(event,
                              reusableBuffer.generalSettings.rotaryEncoderMode,
                              ROTARY_ENCODER_MODE_NORMAL,
                              ROTARY_ENCODER_MODE_INVERT_VERT_HORZ_ALT);
        } else if (reusableBuffer.generalSettings.rotaryEncoderMode !=
                   g_eeGeneral.rotEncMode) {
          g_eeGeneral.rotEncMode =
              reusableBuffer.generalSettings.rotaryEncoderMode;
        }
        break;
#endif

      case ITEM_RADIO_SETUP_STICK_MODE:
        lcdDrawChar(2*FW, y, '1'+reusableBuffer.generalSettings.stickMode, attr);
        for (uint8_t i=0; i<NUM_STICKS; i++) {
          drawSource((5*FW-3)+i*(4*FW+2), y, MIXSRC_Rud + *(modn12x3 + 4*reusableBuffer.generalSettings.stickMode + i), 0);
        }
        if (attr && s_editMode>0) {
          CHECK_INCDEC_GENVAR(event, reusableBuffer.generalSettings.stickMode, 0, 3);
        }
        else if (reusableBuffer.generalSettings.stickMode != g_eeGeneral.stickMode) {
          pausePulses();
          g_eeGeneral.stickMode = reusableBuffer.generalSettings.stickMode;
          checkThrottleStick();
          resumePulses();
          waitKeysReleased();
        }
        break;
    }
  }
}
