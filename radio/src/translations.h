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

#pragma once

#include <limits.h>
#include "edgetx_types.h"
#include "translations/untranslated.h"
#include "audio.h"

#if defined(TRANSLATIONS_FR)
#include "translations/fr.h"
#define LEN_SPECIAL_CHARS 3
#elif defined(TRANSLATIONS_IT)
#include "translations/it.h"
#define LEN_SPECIAL_CHARS 1
#elif defined(TRANSLATIONS_DA)
#include "translations/da.h"
#define LEN_SPECIAL_CHARS 6
#elif defined(TRANSLATIONS_SE)
#include "translations/se.h"
#define LEN_SPECIAL_CHARS 6
#elif defined(TRANSLATIONS_FI)
#include "translations/fi.h"
#define LEN_SPECIAL_CHARS 6
#elif defined(TRANSLATIONS_DE)
#include "translations/de.h"
#define LEN_SPECIAL_CHARS 6
#elif defined(TRANSLATIONS_CZ)
#include "translations/cz.h"
#define LEN_SPECIAL_CHARS 17
#elif defined(TRANSLATIONS_ES)
#include "translations/es.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_PL)
#include "translations/pl.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_PT)
#include "translations/pt.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_NL)
#include "translations/nl.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_CN)
#include "translations/cn.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_TW)
#include "translations/tw.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_JP)
#include "translations/jp.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_RU)
#include "translations/ru.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_HE)
#include "translations/he.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_KO)
#include "translations/ko.h"
#define LEN_SPECIAL_CHARS 0
#elif defined(TRANSLATIONS_UA)
#include "translations/ua.h"
#define LEN_SPECIAL_CHARS 0
#else
#include "translations/en.h"
#define LEN_SPECIAL_CHARS 0
#endif

#define PSIZE(x) ( sizeof(x) - 1 )

#if NARROW_LAYOUT
  #define TR3(x, y, z) z
  #define TR(x, y) y
#elif LCD_W >= 480
  #define TR3(x, y, z) z
  #define TR(x, y) y
#elif LCD_W >= 212
  #define TR3(x, y, z) y
  #define TR(x, y) y
#else
  #define TR3(x, y, z) x
  #define TR(x, y) x
#endif

#if defined(COLORLCD)
  #define BUTTON(x) x
  #define TRC(x, y) y
#else
  #define BUTTON(x)    "[" x "]"
  #define TRC(x, y) x
#endif

#if defined(SURFACE_RADIO)
  #define TRSA(x, y) x
#else
  #define TRSA(x, y) y
#endif

#if (LCD_W == 212) || defined(COLORLCD)
 #define LCDW_128_LINEBREAK
#else
 #define LCDW_128_LINEBREAK        "\036"
#endif

constexpr int g_max_plural2 = TR_MAX_PLURAL2;
constexpr int g_min_plural2 = TR_MIN_PLURAL2;
constexpr int g_use_singular_in_plural = TR_USE_SINGULAR_IN_PLURAL;
constexpr int g_use_plural2_special_case = TR_USE_PLURAL2_SPECIAL_CASE;
constexpr int g_use_plural2 = TR_USE_PLURAL2;

extern const char CHR_HOUR;
extern const char CHR_INPUT;

extern const char* const STR_OFFON[];
extern const char* const STR_MMMINV[];
extern const char* const STR_VBEEPMODE[];
extern const char* const STR_TRNMODE[];
extern const char* const STR_TRNCHN[];
extern const char* const STR_AUX_SERIAL_MODES[];
extern const char* const STR_SWTYPES[];
extern const char* const STR_POTTYPES[];
extern const char* const STR_VTRIMINC[];
extern const char* const STR_VDISPLAYTRIMS[];

extern const char* const STR_TIMER_DIR[];

extern const char* const STR_VBLMODE[];
extern const char* const STR_VCURVETYPE[];
extern const char* const STR_VCURVEFUNC[];
extern const char* const STR_VMLTPX[];
extern const char* const STR_VMLTPX2[];
extern const char* const STR_VCSWFUNC[];
extern const char* const STR_VFSWRESET[];
extern const char* const STR_FUNCSOUNDS[];
extern const char* const STR_VTELEMUNIT[];
extern const char* const STR_GPSFORMAT[];
extern const char* const STR_VTELEMSCREENTYPE[];
extern const char* const STR_VSWASHTYPE[];

extern const char* const STR_ON_ONE_SWITCHES[];
#if defined(IMU)
extern const char* const STR_IMU_VSRCRAW[];
#endif
#if defined(PCBHORUS)
extern const char* const STR_SM_VSRCRAW[];
#endif
extern const char* const STR_CYC_VSRCRAW[];

extern const char* const STR_ROTARY_ENC_OPT[];

#if defined(USE_HATS_AS_KEYS)
extern const char STR_HATSMODE[];
extern const char* const STR_HATSOPT[];
extern const char STR_HATSMODE_TRIMS[];
extern const char STR_HATSMODE_KEYS[];
extern const char STR_HATSMODE_KEYS_HELP[];
#endif

extern const char* const STR_VTMRMODES[];
extern const char* const STR_VPERSISTENT[];

extern const char* const STR_VUNITSSYSTEM[];
extern const char* const STR_VBEEPCOUNTDOWN[];
extern const char* const STR_COUNTDOWNVALUES[];
extern const char* const STR_VVARIOCENTER[];

extern const char* const STR_COUNTRY_CODES[];
extern const char* const STR_USBMODES[];
extern const char* const STR_JACK_MODES[];
extern const char* const STR_VFAILSAFE[];

extern const char* const STR_FONT_SIZES[];
extern const char* const STR_VTRAINERMODES[];
extern const char* const STR_MODULE_PROTOCOLS[];
extern const char* const STR_R9M_REGION[];
extern const char* const STR_R9M_FCC_POWER_VALUES[];
extern const char* const STR_R9M_LBT_POWER_VALUES[];
extern const char* const STR_R9M_LITE_FCC_POWER_VALUES[];
extern const char* const STR_R9M_LITE_LBT_POWER_VALUES[];
extern const char* const STR_XJT_ACCST_RF_PROTOCOLS[];
extern const char* const STR_ISRM_RF_PROTOCOLS[];
extern const char* const STR_R9M_PXX2_RF_PROTOCOLS[];
extern const char* const STR_PPM_PROTOCOLS[];
extern const char* const STR_SBUS_PROTOCOLS[];
extern const char* const STR_DSM_PROTOCOLS[];
extern const char* const STR_CRSF_BAUDRATE[];
extern const char* const STR_PPM_POL[];
extern const char* const STR_SBUS_INVERSION_VALUES[];

extern const char* const STR_MULTI_PROTOCOLS[];
extern const char* const STR_MULTI_POWER[];
extern const char* const STR_MULTI_BAYANG_OPTIONS[];
extern const char* const STR_MULTI_DSM_OPTIONS[];
extern const char* const STR_MULTI_DSM_CLONE[];
extern const char* const STR_MULTI_WBUS_MODE[];
extern const char* const STR_SPORT_MODES[];
extern const char* const STR_FSGROUPS[];
extern const char* const STR_FSSWITCHES[];

extern const char STR_AFHDS3_ONE_TO_ONE_TELEMETRY[];
extern const char STR_AFHDS3_ONE_TO_MANY[];
extern const char* const STR_AFHDS3_PROTOCOLS[];
extern const char* const STR_AFHDS3_POWERS[];
extern const char STR_AFHDS3_RX_FREQ[];
extern const char STR_AFHDS3_ACTUAL_POWER[];
extern const char STR_AFHDS3_POWER_SOURCE[];

extern const char* const STR_FLYSKY_PULSE_PROTO[];
extern const char* const STR_FLYSKY_SERIAL_PROTO[];

extern const char* const STR_CURVE_TYPES[];
extern const char* const STR_VSENSORTYPES[];
extern const char* const STR_VFORMULAS[];
extern const char* const STR_VPREC[];
extern const char* const STR_VCELLINDEX[];
extern const char STR_TOUCH_PANEL[];
extern const char* const STR_ADCFILTERVALUES[];
extern const char STR_FILE_SIZE[];
extern const char STR_FILE_OPEN[];
extern const char* const STR_TIMER_MODES[];
extern const char* const STR_MONTHS[];
extern const char* const STR_PPMUNIT[];

extern const char STR_BLUETOOTH[];
extern const char STR_BLUETOOTH_DISC[];
extern const char STR_BLUETOOTH_INIT[];
extern const char STR_BLUETOOTH_DIST_ADDR[];
extern const char STR_BLUETOOTH_LOCAL_ADDR[];
extern const char STR_BLUETOOTH_PIN_CODE[];
extern const char STR_BLUETOOTH_NODEVICES[];
extern const char STR_BLUETOOTH_SCANNING[];
extern const char* const STR_BLUETOOTH_MODES[];

extern const char STR_STICK_NAMES0[];
extern const char STR_STICK_NAMES1[];
extern const char STR_STICK_NAMES2[];
extern const char STR_STICK_NAMES3[];
extern const char STR_SURFACE_NAMES0[];
extern const char STR_SURFACE_NAMES1[];

extern const char STR_EMPTY[];
extern const char STR_POPUPS_ENTER_EXIT[];
extern const char STR_OK[];
extern const char STR_EXIT[];

#if !defined(PCBHORUS)
extern const char STR_MENUWHENDONE[];
extern const char STR_NO_TELEMETRY_SCREENS[];
#endif

extern const char STR_FREE[];
extern const char STR_YES[];
extern const char STR_NO[];
#define LEN_FREE PSIZE(TR_FREE)
extern const char STR_DELETEMODEL[];
extern const char STR_COPYINGMODEL[];
extern const char STR_MOVINGMODEL[];
extern const char STR_LOADINGMODEL[];
extern const char STR_UNLABELEDMODEL[];
extern const char STR_NAME[];
extern const char STR_BITMAP[];
extern const char STR_NO_PICTURE[];
extern const char STR_TIMER[];
extern const char STR_NO_TIMERS[];
extern const char STR_START[];
extern const char STR_NEXT[];
extern const char STR_ELIMITS[];
extern const char STR_ETRIMS[];
extern const char STR_TRIMINC[];
extern const char STR_DISPLAY_TRIMS[];
extern const char STR_TTRACE[];
extern const char STR_TTRIM[];
extern const char STR_TTRIM_SW[];
extern const char STR_BEEPCTR[];
extern const char STR_USE_GLOBAL_FUNCS[];

extern const char STR_PPMFRAME[];
extern const char STR_REFRESHRATE[];
extern const char STR_MS[];
extern const char STR_SWITCH[];
extern const char STR_FUNCTION_SWITCHES[];
extern const char STR_GROUP[];
extern const char STR_LUA_OVERRIDE[];
extern const char* const STR_FS_COLOR_LIST[];
extern const char STR_GROUP_ALWAYS_ON[];
extern const char STR_GROUPS[];
extern const char STR_LAST[];
extern const char STR_MORE_INFO[];
extern const char STR_SWITCH_TYPE[];
extern const char STR_SWITCH_GROUP[];
extern const char STR_SWITCH_STARTUP[];
extern const char* const STR_FUNCTION_SWITCH_GROUPS[];
extern const char STR_ADJUST_GVAR[];
extern const char STR_PLAY_TRACK[];
extern const char STR_PLAY_VALUE[];
extern const char STR_SOUND[];
extern const char STR_SF_BACKLIGHT[];
extern const char STR_SF_BG_MUSIC[];
extern const char STR_SF_BG_MUSIC_PAUSE[];
extern const char STR_SF_DISABLE_TOUCH[];
extern const char STR_SF_DISABLE_AUDIO_AMP[];
extern const char STR_SF_FAILSAFE[];
extern const char STR_SF_HAPTIC[];
extern const char STR_SF_INST_TRIM[];
extern const char STR_SF_LOGS[];
extern const char STR_SF_MOD_BIND[];
extern const char STR_SF_PLAY_SCRIPT[];
extern const char STR_SF_RANGE_CHECK[];
extern const char STR_SF_RESET[];
extern const char STR_SF_SET_TIMER[];
extern const char STR_SF_SAFETY[];
extern const char STR_SF_SET_SCREEN[];
extern const char STR_SF_SWITCH[];
extern const char STR_SF_TRAINER[];
extern const char STR_SF_VARIO[];
extern const char STR_SF_RGBLEDS[];
extern const char STR_SF_VOLUME[];
extern const char STR_SF_RACING_MODE[];
extern const char STR_SF_SCREENSHOT[];
extern const char STR_SF_LCD_TO_VIDEO[];
extern const char STR_SF_PUSH_CUST_SWITCH[];
extern const char STR_SF_TEST[];
extern const char STR_TRIMS[];
extern const char STR_MIRROR[];
extern const char STR_FADEIN[];
extern const char STR_FADEOUT[];
extern const char STR_DEFAULT[];
extern const char STR_CHECKTRIMS[];
extern const char STR_SWASHTYPE[];
extern const char STR_COLLECTIVE[];
extern const char STR_AILERON[];
extern const char STR_ELEVATOR[];
extern const char STR_SWASHRING[];
extern const char STR_MODE[];

#if defined(AUDIO) && defined(BUZZER)
extern const char STR_SPEAKER[];
extern const char STR_BUZZER[];
#else
#define STR_SPEAKER STR_MODE
#define STR_BUZZER  STR_MODE
#endif

extern const char STR_NOFREEEXPO[];
extern const char STR_NOFREEMIXER[];
extern const char STR_SOURCE[];
extern const char STR_WEIGHT[];
extern const char STR_SIDE[];
extern const char STR_OFFSET[];
extern const char STR_TRIM[];
extern const char STR_DREX[];
extern const char STR_CURVE[];
extern const char STR_FLMODE[];
extern const char STR_MIXWARNING[];
extern const char STR_OFF[];
extern const char STR_ANTENNA[];
extern const char STR_NO_INFORMATION[];
extern const char STR_MULTPX[];
extern const char STR_DELAYDOWN[];
extern const char STR_DELAYUP[];
extern const char STR_SLOWDOWN[];
extern const char STR_SLOWUP[];
extern const char STR_MIXES[];
extern const char STR_CV[];
extern const char STR_VC[];
extern const char STR_GV[];
extern const char STR_RANGE[];
extern const char STR_CENTER[];
extern const char STR_ALARM[];
extern const char STR_BLADES[];
extern const char STR_SCREEN[];
extern const char STR_SOUND_LABEL[];
extern const char STR_LENGTH[];
extern const char STR_BEEP_LENGTH[];
extern const char STR_BEEP_PITCH[];
extern const char STR_CONTRAST[];
extern const char STR_ALARMS_LABEL[];
extern const char STR_BATTERY_RANGE[];
extern const char STR_BATTERYCHARGING[];
extern const char STR_BATTERYFULL[];
extern const char STR_BATTERYNONE[];
extern const char STR_BATTERYWARNING[];
extern const char STR_INACTIVITYALARM[];
extern const char STR_MEMORYWARNING[];
extern const char STR_ALARMWARNING[];
extern const char STR_RSSI_SHUTDOWN_ALARM[];
extern const char STR_TRAINER_SHUTDOWN_ALARM[];
extern const char STR_MODEL_STILL_POWERED[];
extern const char STR_USB_STILL_CONNECTED[];
extern const char STR_TRAINER_STILL_CONNECTED[];
extern const char STR_MODEL_SHUTDOWN[];
extern const char STR_PRESS_ENTER_TO_CONFIRM[];
extern const char STR_THROTTLEREVERSE[];
extern const char STR_MINUTEBEEP[];
extern const char STR_BEEPCOUNTDOWN[];
extern const char STR_PERSISTENT[];
extern const char STR_BACKLIGHT_LABEL[];
extern const char STR_GHOST_MENU_LABEL[];
extern const char STR_STATUS[];

#if defined(COLORLCD)
extern const char STR_BLONBRIGHTNESS[];
extern const char STR_BLOFFBRIGHTNESS[];
extern const char STR_KEYS_BACKLIGHT[];
#endif

extern const char STR_SPLASHSCREEN[];
extern const char STR_PLAY_HELLO[];
#if defined(COLORLCD)
extern const char* const STR_SPLASHSCREEN_DELAYS[];
#endif
#if defined(PWR_BUTTON_PRESS)
extern const char STR_PWR_ON_DELAY[];
extern const char STR_PWR_OFF_DELAY[];
extern const char STR_PWR_AUTO_OFF[];
extern const char* const STR_PWR_OFF_DELAYS[];
#endif
#if defined(HAPTIC)
extern const char STR_PWR_ON_OFF_HAPTIC[];
extern const char STR_HAPTIC_LABEL[];
extern const char STR_STRENGTH[];
#endif
#if defined(IMU)
extern const char STR_IMU_LABEL[];
extern const char STR_IMU_OFFSET[];
extern const char STR_IMU_MAX[];
#endif
extern const char STR_THROTTLE_WARNING[];
extern const char STR_CUSTOM_THROTTLE_WARNING[];
extern const char STR_CUSTOM_THROTTLE_WARNING_VAL[];
extern const char STR_SWITCHWARNING[];
extern const char STR_POTWARNINGSTATE[];
extern const char STR_POTWARNING[];
extern const char STR_TIMEZONE[];
extern const char STR_ADJUST_RTC[];
extern const char STR_GPS[];
extern const char STR_GPS_COORDS_FORMAT[];
extern const char STR_VARIO[];
extern const char STR_FLYSKY_TELEMETRY[];
extern const char STR_PITCH_AT_ZERO[];
extern const char STR_PITCH_AT_MAX[];
extern const char STR_REPEAT_AT_ZERO[];
extern const char STR_DEF_CHAN_ORD[];
extern const char STR_STICKS[];
extern const char STR_POTS[];
extern const char STR_SWITCHES[];
extern const char STR_SWITCHES_DELAY[];
extern const char STR_SLAVE[];
extern const char STR_MODESRC[];
extern const char STR_MULTIPLIER[];
#define LEN_MULTIPLIER PSIZE(TR_MULTIPLIER)
#define STR_UPDATE_LIST STR_DELAYDOWN

struct LanguagePack {
  const char * id;
  const char * name;
  void (*playNumber)(getvalue_t number, uint8_t unit, uint8_t flags, uint8_t id, int8_t fragmentVolume);
  void (*playDuration)(int seconds, uint8_t flags, uint8_t id, int8_t fragmentVolume);
};

extern const LanguagePack * currentLanguagePack;
extern uint8_t currentLanguagePackIdx;

extern const LanguagePack czLanguagePack;
extern const LanguagePack daLanguagePack;
extern const LanguagePack deLanguagePack;
extern const LanguagePack enLanguagePack;
extern const LanguagePack esLanguagePack;
extern const LanguagePack frLanguagePack;
extern const LanguagePack huLanguagePack;
extern const LanguagePack itLanguagePack;
extern const LanguagePack nlLanguagePack;
extern const LanguagePack plLanguagePack;
extern const LanguagePack ptLanguagePack;
extern const LanguagePack ruLanguagePack;
extern const LanguagePack seLanguagePack;
extern const LanguagePack skLanguagePack;
extern const LanguagePack cnLanguagePack;
extern const LanguagePack jpLanguagePack;
extern const LanguagePack koLanguagePack;
extern const LanguagePack ruLanguagePack;
extern const LanguagePack heLanguagePack;
extern const LanguagePack uaLanguagePack;
extern const LanguagePack * const languagePacks[];

#if defined(LANGUAGE_PACKS_DEFINITION)
const LanguagePack * const languagePacks[] = {
  // alphabetical order
  &cnLanguagePack,
  &czLanguagePack,
  &daLanguagePack,
  &deLanguagePack,
  &enLanguagePack,
  &esLanguagePack,
  &frLanguagePack,
  &heLanguagePack,
  &huLanguagePack,
  &itLanguagePack,
  &jpLanguagePack,
  &koLanguagePack,
  &nlLanguagePack,
  &plLanguagePack,
  &ptLanguagePack,
  &ruLanguagePack,
  &seLanguagePack,
  &skLanguagePack,
  &uaLanguagePack,
  NULL
};
#endif

#if defined(SIMU)
#define LANGUAGE_PACK_DECLARE(lng, name)                                  \
  const LanguagePack lng##LanguagePack = {#lng, name, lng##_##playNumber, \
                                          lng##_##playDuration}
#else
#define LANGUAGE_PACK_DECLARE(lng, name)          \
  extern const LanguagePack lng##LanguagePack = { \
      #lng, name, lng##_##playNumber, lng##_##playDuration}
#endif

#define LANGUAGE_PACK_DECLARE_DEFAULT(lng, name)                \
  LANGUAGE_PACK_DECLARE(lng, name);                             \
  const LanguagePack* currentLanguagePack = &lng##LanguagePack; \
  uint8_t currentLanguagePackIdx

#define PLAY_FUNCTION(x, ...)    void x(__VA_ARGS__, uint8_t id, int8_t fragmentVolume = USE_SETTINGS_VOLUME)

inline PLAY_FUNCTION(playNumber, getvalue_t number, uint8_t unit, uint8_t flags) {
  currentLanguagePack->playNumber(number, unit, flags, id, fragmentVolume);
}

inline PLAY_FUNCTION(playDuration, int seconds, uint8_t flags) {
   currentLanguagePack->playDuration(seconds, flags, id, fragmentVolume);
}

// Static string
#define STR(x) extern const char STR_##x[];
// Static string array
#define STRARRAY(x) extern const char* const STR_##x[];

#include "string_list.h"

STR(SF_SET_SCREEN);
STR(RX);

#if defined(AUDIO) && defined(BUZZER)
STR(SPEAKER);
STR(BUZZER);
#else
#define STR_SPEAKER STR_MODE
#define STR_BUZZER  STR_MODE
#endif

#undef STR
#undef STRARRAY
