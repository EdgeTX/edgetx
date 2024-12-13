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
#elif defined(TRANSLATIONS_UA)
#include "translations/ua.h"
#define LEN_SPECIAL_CHARS 0
#else
#include "translations/en.h"
#define LEN_SPECIAL_CHARS 0
#endif

#define PSIZE(x) ( sizeof(x) - 1 )

#if LCD_W < LCD_H    // Portrait mode
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
#else
  #define BUTTON(x)    "[" x "]"
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

extern const char STR_MINUTE_SINGULAR[];
extern const char STR_MINUTE_PLURAL1[];
extern const char STR_MINUTE_PLURAL2[];

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
extern const char STR_START[];
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
extern const char STR_FS_ON_COLOR[];
extern const char STR_FS_OFF_COLOR[];
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
extern const char STR_CAL[];
extern const char STR_CALIB_DONE[];
extern const char STR_CALIBRATION[];
extern const char STR_VTRIM[];
extern const char STR_MENUTOSTART[];
extern const char STR_SETMIDPOINT[];
extern const char STR_MOVESTICKSPOTS[];
extern const char STR_TX[];
#define STR_RX (STR_TX+OFS_RX)
extern const char STR_NODATA[];
extern const char STR_US[];
extern const char STR_HZ[];
extern const char STR_TMIXMAXMS[];
extern const char STR_FREE_STACK[];
extern const char STR_INT_GPS_LABEL[];
extern const char STR_HEARTBEAT_LABEL[];
extern const char STR_LUA_SCRIPTS_LABEL[];
extern const char STR_FREE_MEM_LABEL[];
extern const char STR_DURATION_MS[];
extern const char STR_INTERVAL_MS[];
extern const char STR_MEM_USED_SCRIPT[];
extern const char STR_MEM_USED_WIDGET[];
extern const char STR_MEM_USED_EXTRA[];
extern const char STR_STACK_MIX[];
extern const char STR_STACK_AUDIO[];
extern const char STR_GPS_FIX_YES[];
extern const char STR_GPS_FIX_NO[];
extern const char STR_GPS_SATS[];
extern const char STR_GPS_HDOP[];
extern const char STR_STACK_MENU[];
extern const char STR_TIMER_LABEL[];
extern const char STR_THROTTLE_PERCENT_LABEL[];
extern const char STR_BATT_LABEL[];
extern const char STR_SESSION[];
extern const char STR_MENUTORESET[];
extern const char STR_PPM_TRAINER[];
extern const char STR_CH[];
extern const char STR_MODEL[];
extern const char STR_FM[];
extern const char STR_EEPROMLOWMEM[];
extern const char STR_PRESS_ANY_KEY_TO_SKIP[];
extern const char STR_THROTTLE_NOT_IDLE[];
extern const char STR_ALARMSDISABLED[];
extern const char STR_PRESSANYKEY[];
extern const char STR_BAD_RADIO_DATA[];
extern const char STR_RADIO_DATA_UNRECOVERABLE[];
extern const char STR_RADIO_DATA_RECOVERED[];
extern const char STR_STORAGE_FORMAT[];
extern const char STR_EEPROMOVERFLOW[];
extern const char STR_TRIMS2OFFSETS[];
extern const char STR_CHANNELS2FAILSAFE[];
extern const char STR_CHANNEL2FAILSAFE[];
extern const char STR_RADIO_SETUP[];
extern const char STR_MENUTRAINER[];
extern const char STR_MENUSPECIALFUNCS[];
extern const char STR_MENUVERSION[];
extern const char STR_MENU_RADIO_SWITCHES[];
extern const char STR_MENU_RADIO_ANALOGS[];
extern const char STR_MENU_RADIO_ANALOGS_CALIB[];
extern const char STR_MENU_RADIO_ANALOGS_RAWLOWFPS[];
extern const char STR_MENUCALIBRATION[];
extern const char STR_MENUMODELSEL[];
extern const char STR_MENU_MODEL_SETUP[];
extern const char STR_MENUFLIGHTMODE[];
extern const char STR_MENUFLIGHTMODES[];
extern const char STR_MENU_FSWITCH[];
extern const char STR_MENUHELISETUP[];
extern const char STR_MENUINPUTS[];
extern const char STR_MENULIMITS[];
extern const char STR_MENUCURVES[];
extern const char STR_MENUCURVE[];
extern const char STR_MENULOGICALSWITCH[];
extern const char STR_MENULOGICALSWITCHES[];
extern const char STR_MENUCUSTOMFUNC[];
extern const char STR_MENUCUSTOMSCRIPTS[];
extern const char STR_MENUTELEMETRY[];
extern const char STR_MENUSTAT[];
extern const char STR_MENUDEBUG[];
extern const char * const STR_MONITOR_CHANNELS[];
extern const char STR_MONITOR_SWITCHES[];
extern const char STR_MONITOR_OUTPUT_DESC[];
extern const char STR_MONITOR_MIXER_DESC[];
extern const char STR_MENU_GLOBAL_VARS[];
extern const char STR_AND_SWITCH[];
extern const char STR_SF[];
extern const char STR_GF[];
extern const char STR_ANADIAGS_CALIB[];
extern const char STR_ANADIAGS_FILTRAWDEV[];
extern const char STR_ANADIAGS_UNFILTRAW[];
extern const char STR_ANADIAGS_MINMAX[];
extern const char STR_ANADIAGS_MOVE[];
extern const char STR_MULTI_CUSTOM[];
extern const char STR_MULTI_FIXEDID[];
extern const char STR_MULTI_OPTION[];
extern const char STR_MULTI_VIDFREQ[];
extern const char STR_MULTI_RFTUNE[];
extern const char STR_MULTI_RFPOWER[];
extern const char STR_MULTI_WBUS[];
extern const char STR_MULTI_TELEMETRY[];
extern const char STR_MULTI_AUTOBIND[];
extern const char STR_DISABLE_CH_MAP[];
extern const char STR_DISABLE_TELEM[];
extern const char STR_MULTI_LOWPOWER[];
extern const char STR_MULTI_LNA_DISABLE[];
extern const char STR_MODULE_NO_SERIAL_MODE[];
extern const char STR_MODULE_NO_INPUT[];
extern const char STR_MODULE_WAITFORBIND[];
extern const char STR_MODULE_NO_TELEMETRY[];
extern const char STR_MODULE_BINDING[];
extern const char STR_MODULE_UPGRADE_ALERT[];
extern const char STR_MODULE_UPGRADE[];
extern const char STR_PROTOCOL_INVALID[];
extern const char STR_MODULE_STATUS[];
extern const char STR_MODULE_SYNC[];
extern const char STR_MULTI_SERVOFREQ[];
extern const char STR_MULTI_MAX_THROW[];
extern const char STR_MULTI_RFCHAN[];
extern const char STR_SUBTYPE[];

#if defined(DSM2) || defined(PXX)
extern const char STR_RECEIVER_NUM[];
extern const char STR_RECEIVER[];
extern const char STR_REBIND[];
extern const char STR_REG_OK[];
extern const char STR_BIND_OK[];
#endif

extern const char STR_INTERNALRF[];
extern const char STR_INTERNAL_MODULE[];
extern const char STR_EXTERNAL_MODULE[];
extern const char STR_EDGETX_UPGRADE_REQUIRED[];
extern const char STR_TELEMETRY_DISABLED[];
extern const char STR_MORE_OPTIONS_AVAILABLE[];
extern const char STR_NO_MODULE_INFORMATION[];
extern const char STR_EXTERNALRF[];
extern const char STR_MODULE_TELEMETRY[];
extern const char STR_MODULE_TELEM_ON[];
extern const char STR_FAILSAFE[];
extern const char STR_FAILSAFESET[];
extern const char STR_RECEIVER[];
extern const char STR_REG_ID[];
extern const char STR_OWNER_ID[];
extern const char STR_HOLD[];
extern const char STR_HOLD_UPPERCASE[];
extern const char STR_NONE[];
extern const char STR_NONE_UPPERCASE[];
extern const char STR_MENUSENSOR[];
extern const char STR_POWERMETER_PEAK[];
extern const char STR_POWERMETER_POWER[];
extern const char STR_POWERMETER_ATTN[];
extern const char STR_POWERMETER_FREQ[];
extern const char STR_MENUTOOLS[];
extern const char STR_TURN_OFF_RECEIVER          [];
extern const char STR_STOPPING              [];
extern const char STR_MENU_SPECTRUM_ANALYSER[];
extern const char STR_MENU_POWER_METER[];
extern const char STR_SENSOR[];
extern const char STR_COUNTRY_CODE[];
extern const char STR_USBMODE[];
extern const char STR_JACK_MODE[];
extern const char STR_DISABLE_INTERNAL[];
extern const char STR_BATT_CALIB[];
extern const char STR_VOLTAGE[];
extern const char STR_CURRENT_CALIB[];
extern const char STR_UNITS_SYSTEM[];
extern const char STR_UNITS_PPM[];
extern const char STR_VOICE_LANGUAGE[];
extern const char STR_MODELIDUSED[];
extern const char STR_MODELIDUNIQUE[];
extern const char STR_BEEP_VOLUME[];
extern const char STR_WAV_VOLUME[];
extern const char STR_BG_VOLUME[];
extern const char STR_SELECT_MODEL[];
extern const char STR_MODELS[];
extern const char STR_SELECT_MODE[];
extern const char STR_CREATE_MODEL[];
extern const char STR_FAVORITE_LABEL[];
extern const char STR_MODELS_MOVED[];
extern const char STR_NEW_MODEL[];
extern const char STR_INVALID_MODEL[];
extern const char STR_MOVE_UP[];
extern const char STR_MOVE_DOWN[];
extern const char STR_DUPLICATE_MODEL[];
extern const char STR_COPY_MODEL[];
extern const char STR_MOVE_MODEL[];
extern const char STR_DELETE_MODEL[];
extern const char STR_EDIT[];
extern const char STR_INSERT_BEFORE[];
extern const char STR_INSERT_AFTER[];
extern const char STR_COPY[];
extern const char STR_MOVE[];
extern const char STR_PASTE[];
extern const char STR_PASTE_AFTER[];
extern const char STR_PASTE_BEFORE[];
extern const char STR_INSERT[];
extern const char STR_DELETE[];
extern const char STR_RESET_FLIGHT[];
extern const char STR_RESET_TIMER1[];
extern const char STR_RESET_TIMER2[];
extern const char STR_RESET_TIMER3[];
extern const char STR_RESET_TELEMETRY[];
extern const char STR_STATISTICS[];
extern const char STR_ABOUT_US[];
extern const char STR_USB_JOYSTICK[];
extern const char STR_USB_MASS_STORAGE[];
extern const char STR_USB_SERIAL[];
extern const char STR_SETUP_SCREENS[];
extern const char STR_MONITOR_SCREENS[];
extern const char STR_RESET_BTN[];
extern const char STR_DEBUG[];
extern const char STR_KEYS_BTN[];
extern const char STR_ANALOGS_BTN[];
extern const char STR_FS_BTN[];
extern const char STR_TOUCH_NOTFOUND[];
extern const char STR_TOUCH_EXIT[];
extern const char STR_CALIBRATION[];
extern const char STR_NO_TOOLS[];
extern const char STR_NORMAL[];
extern const char STR_NOT_INVERTED[];
extern const char STR_NOT_CONNECTED[];
extern const char STR_CONNECTED[];
extern const char STR_FLEX_915[];
extern const char STR_FLEX_868[];
extern const char STR_16CH_WITHOUT_TELEMETRY[];
extern const char STR_16CH_WITH_TELEMETRY[];
extern const char STR_8CH_WITH_TELEMETRY[];
extern const char STR_EXT_ANTENNA[];
extern const char STR_PIN[];
extern const char STR_UPDATE_RX_OPTIONS[];
extern const char STR_UPDATE_TX_OPTIONS[];
extern const char STR_MENU_MODULES_RX_VERSION[];
extern const char STR_MENU_FIRM_OPTIONS[];
extern const char STR_IMU[];
extern const char STR_STICKS_POTS_SLIDERS[];
extern const char STR_PWM_STICKS_POTS_SLIDERS[];
extern const char STR_RF_PROTOCOL[];
extern const char STR_MODULE_OPTIONS[];
extern const char STR_POWER[];
extern const char STR_NO_TX_OPTIONS[];
extern const char STR_RTC_BATT[];
extern const char STR_POWER_METER_EXT[];
extern const char STR_POWER_METER_INT[];
extern const char STR_SPECTRUM_ANALYSER_EXT[];
extern const char STR_SPECTRUM_ANALYSER_INT[];
extern const char STR_WAITING_FOR_RX[];
extern const char STR_WAITING_FOR_TX[];
extern const char STR_WAITING_FOR_MODULE[];
extern const char STR_WARN_5VOLTS[];

extern const char STR_BACKUP_MODEL[];
extern const char STR_RESTORE_MODEL[];
extern const char STR_DELETE_ERROR[];
extern const char STR_SDCARD_ERROR[];
extern const char STR_SDCARD[];
extern const char STR_NO_FILES_ON_SD[];
extern const char STR_NO_SDCARD[];
extern const char STR_SDCARD_FULL[];
extern const char STR_SDCARD_FULL_EXT[];
extern const char STR_INCOMPATIBLE[];
extern const char STR_LOGS_PATH[];
extern const char STR_LOGS_EXT[];
extern const char STR_MODELS_PATH[];
extern const char STR_MODELS_EXT[];
extern const char STR_BACKUP_PATH[];
extern const char STR_YAML_EXT[];
#define STR_UPDATE_LIST STR_DELAYDOWN

extern const char STR_NEEDS_FILE[];
extern const char STR_EXT_MULTI_SPEC[];
extern const char STR_INT_MULTI_SPEC[];
extern const char STR_WARNING[];
extern const char STR_STORAGE_WARNING[];
extern const char STR_THROTTLE_UPPERCASE[];
extern const char STR_ALARMSWARN[];
extern const char STR_SWITCHWARN[];
extern const char STR_FAILSAFEWARN[];
extern const char STR_TEST_WARNING[];
extern const char STR_TEST_NOTSAFE[];
extern const char STR_WRONG_SDCARDVERSION[];
extern const char STR_WARN_RTC_BATTERY_LOW[];
extern const char STR_WARN_MULTI_LOWPOWER[];
extern const char STR_BATTERY[];
extern const char STR_WRONG_PCBREV[];
extern const char STR_EMERGENCY_MODE[];
extern const char STR_NO_FAILSAFE[];
extern const char STR_KEYSTUCK[];

extern const char STR_VOLUME[];
extern const char STR_LCD[];
extern const char STR_BRIGHTNESS[];
extern const char STR_CPU_TEMP[];
extern const char STR_COPROC[];
extern const char STR_COPROC_TEMP[];
extern const char STR_TTL_WARNING[];
extern const char STR_FUNC[];
extern const char STR_V1[];
extern const char STR_V2[];
extern const char STR_DURATION[];
extern const char STR_DELAY[];
extern const char STR_SD_CARD[];
extern const char STR_SDHC_CARD[];
extern const char STR_NO_SOUNDS_ON_SD[];
extern const char STR_NO_MODELS_ON_SD[];
extern const char STR_NO_BITMAPS_ON_SD[];
extern const char STR_NO_SCRIPTS_ON_SD[];
extern const char STR_SCRIPT_SYNTAX_ERROR[];
extern const char STR_SCRIPT_PANIC[];
extern const char STR_SCRIPT_KILLED[];
extern const char STR_SCRIPT_ERROR[];
extern const char STR_PLAY_FILE[];
extern const char STR_ASSIGN_BITMAP[];
extern const char STR_LIMITS_HEADERS_DIRECTION[];
#if defined(COLORLCD)
extern const char STR_ASSIGN_SPLASH[];
extern const char STR_FIRST_CHANNEL[];
extern const char STR_FILL_BACKGROUND[];
extern const char STR_BG_COLOR[];
extern const char STR_SLIDERS_TRIMS[];
extern const char STR_SLIDERS[];
extern const char STR_FLIGHT_MODE[];
extern const char STR_INVALID_FILE[];
extern const char STR_TIMER_SOURCE[];
extern const char* const STR_SUBTRIMMODES[];
extern const char STR_SIZE[];
extern const char STR_SHADOW[];
extern const char STR_ALIGNMENT[];
extern const char STR_ALIGN_LABEL[];
extern const char STR_ALIGN_VALUE[];
extern const char* const STR_ALIGN_OPTS[];
extern const char STR_TEXT[];
extern const char STR_COLOR[];
extern const char STR_MAIN_VIEW_X[];
extern const char STR_PANEL1_BACKGROUND[];
extern const char STR_PANEL2_BACKGROUND[];
extern const char STR_WIDGET_GAUGE[];
extern const char STR_WIDGET_MODELBMP[];
extern const char STR_WIDGET_OUTPUTS[];
extern const char STR_WIDGET_TEXT[];
extern const char STR_WIDGET_TIMER[];
extern const char STR_WIDGET_VALUE[];
extern const char STR_MODULES_RX_VERSION[];
extern const char STR_LEFT_STICK[];
extern const char STR_SHOW_MIXER_MONITORS[];
extern const char STR_MODEL_QUICK_SELECT[];
extern const char STR_LOADING[];
extern const char STR_DELETE_THEME[];
extern const char STR_SAVE_THEME[];
extern const char STR_EDIT_COLOR[];
extern const char STR_NO_THEME_IMAGE[];
extern const char STR_BACKLIGHT_TIMER[];
extern const char STR_LABELS_SELECT[];
extern const char STR_LABELS_MATCH[];
extern const char STR_FAV_MATCH[];
extern const char* const STR_LABELS_SELECT_MODE[];
extern const char* const STR_LABELS_MATCH_MODE[];
extern const char* const STR_FAV_MATCH_MODE[];
#endif
extern const char STR_EXECUTE_FILE[];
extern const char STR_DELETE_FILE[];
extern const char STR_COPY_FILE[];
extern const char STR_RENAME_FILE[];
extern const char STR_REMOVED[];
extern const char STR_SD_INFO[];
extern const char STR_NA[];
extern const char STR_HARDWARE[];
extern const char STR_FORMATTING[];
extern const char STR_TEMP_CALIB[];
extern const char STR_TIME[];
extern const char STR_MAXBAUDRATE[];
extern const char STR_BAUDRATE[];
extern const char STR_CRSF_ARMING_MODE[];
extern const char* const STR_CRSF_ARMING_MODES[];
extern const char STR_SAMPLE_MODE[];
extern const char* const STR_SAMPLE_MODES[];
extern const char STR_BLUETOOTH_BAUDRATE[];
extern const char STR_SD_INFO_TITLE[];
extern const char STR_SD_SPEED[];
extern const char STR_SD_SECTORS[];
extern const char STR_SD_SIZE[];
extern const char STR_TYPE[];
extern const char STR_GLOBAL_VARS[];
extern const char STR_GVARS[];
extern const char STR_GLOBAL_VAR[];
extern const char STR_OWN[];
extern const char STR_DATE[];
extern const char STR_ROTARY_ENCODER[];
extern const char STR_ROTARY_ENC_MODE[];
extern const char STR_CHANNELS_MONITOR[];
extern const char STR_MIXERS_MONITOR[];
extern const char STR_PATH_TOO_LONG[];
extern const char STR_VIEW_TEXT[];
extern const char STR_FLASH_BOOTLOADER[];
extern const char STR_FLASH_DEVICE[];
extern const char STR_FLASH_EXTERNAL_DEVICE[];
extern const char STR_FLASH_RECEIVER_OTA[];
extern const char STR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA[];
extern const char STR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA[];
extern const char STR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA[];
extern const char STR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA[];
extern const char STR_FLASH_BLUETOOTH_MODULE[];
extern const char STR_FLASH_POWER_MANAGEMENT_UNIT[];
extern const char STR_DEVICE_NO_RESPONSE[];
extern const char STR_DEVICE_FILE_ERROR[];
extern const char STR_DEVICE_DATA_REFUSED[];
extern const char STR_DEVICE_WRONG_REQUEST[];
extern const char STR_DEVICE_FILE_REJECTED[];
extern const char STR_DEVICE_FILE_WRONG_SIG[];
extern const char STR_CURRENT_VERSION[];
extern const char STR_FLASH_INTERNAL_MODULE[];
extern const char STR_FLASH_INTERNAL_MULTI[];
extern const char STR_FLASH_EXTERNAL_MODULE[];
extern const char STR_FLASH_EXTERNAL_MULTI[];
extern const char STR_FLASH_EXTERNAL_ELRS[];
extern const char STR_FIRMWARE_UPDATE_ERROR[];
extern const char STR_FIRMWARE_UPDATE_SUCCESS[];
extern const char STR_WRITING[];
extern const char STR_CONFIRM_FORMAT[];
extern const char STR_CONFIRMRESET[];
extern const char STR_TOO_MANY_LUA_SCRIPTS[];
extern const char STR_BLCOLOR[];

extern const char STR_DIGITAL_SERVO[];
extern const char STR_ANALOG_SERVO[];
extern const char STR_SIGNAL_OUTPUT[];
extern const char STR_SERIAL_BUS[];
extern const char STR_SYNC[];

extern const char STR_ENABLED_FEATURES[];
extern const char STR_RADIO_MENU_TABS[];
extern const char STR_MODEL_MENU_TABS[];


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

extern const char STR_MODELNAME[];
extern const char STR_PHASENAME[];
extern const char STR_MIXNAME[];
extern const char STR_INPUTNAME[];
extern const char STR_EXPONAME[];

#if LCD_W >= 212
#define TR_PHASES_HEADERS { TR_PHASES_HEADERS_NAME, TR_PHASES_HEADERS_SW, TR_PHASES_HEADERS_RUD_TRIM, TR_PHASES_HEADERS_ELE_TRIM, TR_PHASES_HEADERS_THT_TRIM, TR_PHASES_HEADERS_AIL_TRIM, TR_PHASES_HEADERS_FAD_IN, TR_PHASES_HEADERS_FAD_OUT }
#define TR_LIMITS_HEADERS { TR_LIMITS_HEADERS_NAME, TR_LIMITS_HEADERS_SUBTRIM, TR_LIMITS_HEADERS_MIN, TR_LIMITS_HEADERS_MAX, TR_LIMITS_HEADERS_DIRECTION, TR_LIMITS_HEADERS_CURVE, TR_LIMITS_HEADERS_PPMCENTER, TR_LIMITS_HEADERS_SUBTRIMMODE }
#define TR_LSW_HEADERS    { TR_FUNC, TR_V1, TR_V2, TR_V2, TR_AND_SWITCH, TR_DURATION, TR_DELAY, TR_PERSISTENT }
  extern const char * const STR_PHASES_HEADERS[];
  extern const char * const STR_LIMITS_HEADERS[];
  extern const char * const STR_LSW_HEADERS[];
  extern const char * const STR_LSW_DESCRIPTIONS[];
#endif

extern const char STR_TRAINER[];
extern const char STR_CHANS[];
extern const char STR_MODULE_BIND[];
extern const char STR_MODULE_UNBIND[];
extern const char STR_POWERMETER_ATTN_NEEDED[];
extern const char STR_PXX2_SELECT_RX[];
extern const char STR_PXX2_DEFAULT[];
extern const char STR_BT_SELECT_DEVICE[];
extern const char STR_DISCOVER[];
extern const char STR_BUTTON_INIT[];
extern const char STR_WAITING[];
extern const char STR_RECEIVER_DELETE[];
extern const char STR_RECEIVER_RESET[];
extern const char STR_SHARE[];
extern const char STR_BIND[];
extern const char STR_REGISTER[];
extern const char STR_BINDING_1_8_TELEM_ON[];
extern const char STR_BINDING_1_8_TELEM_OFF[];
extern const char STR_BINDING_9_16_TELEM_ON[];
extern const char STR_BINDING_9_16_TELEM_OFF[];
extern const char STR_CHANNELRANGE[];
extern const char STR_ANTENNACONFIRM1[];
extern const char STR_ANTENNACONFIRM2[];
extern const char* const STR_ANTENNA_MODES[];
extern const char STR_USE_INTERNAL_ANTENNA[];
extern const char STR_USE_EXTERNAL_ANTENNA[];
extern const char STR_MODULE_PROTOCOL_FLEX_WARN_LINE1[];
extern const char STR_MODULE_PROTOCOL_FCC_WARN_LINE1[];
extern const char STR_MODULE_PROTOCOL_EU_WARN_LINE1[];
extern const char STR_MODULE_PROTOCOL_WARN_LINE2[];
extern const char STR_SET[];
extern const char* const STR_PREFLIGHT_POTSLIDER_CHECK[];
extern const char STR_PREFLIGHT[];
extern const char STR_CHECKLIST[];
extern const char STR_CHECKLIST_INTERACTIVE[];
extern const char STR_VIEW_NOTES[];
extern const char STR_MODEL_SELECT[];
extern const char STR_RESET_SUBMENU[];
extern const char STR_LOWALARM[];
extern const char STR_CRITICALALARM[];
extern const char STR_DISABLE_ALARM[];
extern const char STR_TELEMETRY_TYPE[];
extern const char STR_TELEMETRY_SENSORS[];
extern const char STR_VALUE[];
extern const char STR_PERIOD[];
extern const char STR_INTERVAL[];
extern const char STR_REPEAT[];
extern const char STR_ENABLE[];
extern const char STR_DISABLE[];
extern const char STR_TOPLCDTIMER[];
extern const char STR_UNIT[]; ;
extern const char STR_TELEMETRY_NEWSENSOR[];
extern const char STR_ID[];
extern const char STR_PRECISION[];
extern const char STR_RATIO[];
extern const char STR_FORMULA[];
extern const char STR_CELLINDEX[];
extern const char STR_LOGS[];
extern const char STR_OPTIONS[];
extern const char STR_FIRMWARE_OPTIONS[];
extern const char STR_ALTSENSOR[];
extern const char STR_CELLSENSOR[];
extern const char STR_GPSSENSOR[];
extern const char STR_CURRENTSENSOR[];
extern const char STR_AUTOOFFSET[];
extern const char STR_ONLYPOSITIVE[];
extern const char STR_FILTER[];
extern const char STR_TELEMETRYFULL[];
extern const char STR_IGNORE_INSTANCE[];
extern const char STR_SHOW_INSTANCE_ID[];
extern const char STR_DISCOVER_SENSORS[];
extern const char STR_STOP_DISCOVER_SENSORS[];
extern const char STR_DELETE_ALL_SENSORS[];
extern const char STR_CONFIRMDELETE[];
extern const char STR_SELECT_WIDGET[];
extern const char STR_REMOVE_WIDGET[];
extern const char STR_WIDGET_SETTINGS[];
extern const char STR_WIDGET_FULLSCREEN[];
extern const char STR_REMOVE_SCREEN[];
extern const char STR_SETUP_WIDGETS[];
extern const char STR_USER_INTERFACE[];
extern const char STR_THEME[];
extern const char STR_SETUP[];
extern const char STR_LAYOUT[];
extern const char STR_ADD_MAIN_VIEW[];
extern const char STR_TEXT_COLOR[];
extern const char STR_RF_POWER[];

extern const char STR_BYTES[];
extern const char STR_ANTENNAPROBLEM[];
extern const char STR_MODULE[];
extern const char STR_RX_NAME[];
extern const char STR_POPUP[];
extern const char STR_MIN[];
extern const char STR_MAX[];
extern const char STR_INVERTED[];
extern const char STR_CURVE_PRESET[];
extern const char STR_PRESET[];
extern const char STR_CLEAR[];
extern const char STR_RESET[];
extern const char STR_ADD_ALL_TRIMS_TO_SUBTRIMS[];
extern const char STR_OPEN_CHANNEL_MONITORS[];
extern const char STR_COUNT[];
extern const char STR_PT[];
extern const char STR_PTS[];
extern const char STR_SMOOTH[];
extern const char STR_COPY_STICKS_TO_OFS[];
extern const char STR_COPY_MIN_MAX_TO_OUTPUTS[];
extern const char STR_COPY_TRIMS_TO_OFS[];
extern const char STR_INCDEC[];
extern const char STR_GLOBALVAR[];
extern const char STR_MIXSOURCE[];
extern const char STR_MIXSOURCERAW[];
extern const char STR_CONSTANT[];
extern const char STR_TOP_BAR[];
extern const char STR_FLASH_ERASE[];
extern const char STR_FLASH_WRITE[];
extern const char STR_OTA_UPDATE[];
extern const char STR_MODULE_RESET[];
extern const char STR_UNKNOWN_RX[];
extern const char STR_UNSUPPORTED_RX[];
extern const char STR_OTA_UPDATE_ERROR[];
extern const char STR_DEVICE_RESET[];
extern const char STR_ALTITUDE[];
extern const char STR_SCALE[];
extern const char STR_VIEW_CHANNELS[];
extern const char STR_POTWARNING[];
extern const char STR_AUX_SERIAL_MODE[];
extern const char STR_AUX2_SERIAL_MODE[];
extern const char STR_AUX_SERIAL_PORT_POWER[];
extern const char STR_THROTTLE_LABEL[];
extern const char STR_SCRIPT[];
extern const char STR_INPUTS[];
extern const char STR_OUTPUTS[];
extern const char STR_MENU_INPUTS[];
extern const char STR_MENU_LUA[];
extern const char STR_MENU_STICKS[];
extern const char STR_MENU_POTS[];
extern const char STR_MENU_MIN[];
extern const char STR_MENU_MAX[];
extern const char STR_MENU_HELI[];
extern const char STR_MENU_TRIMS[];
extern const char STR_MENU_SWITCHES[];
extern const char STR_MENU_LOGICAL_SWITCHES[];
extern const char STR_MENU_TRAINER[];
extern const char STR_MENU_CHANNELS[];
extern const char STR_MENU_GVARS[];
extern const char STR_MENU_TELEMETRY[];
extern const char STR_MENU_DISPLAY[];
extern const char STR_MENU_OTHER[];
extern const char STR_MENU_INVERT[];
extern const char STR_AUDIO_MUTE[];
extern const char STR_JITTER_FILTER[];
extern const char STR_DEAD_ZONE[];
extern const char STR_RTC_CHECK[];
extern const char STR_SPORT_UPDATE_POWER_MODE[];
extern const char* const STR_SPORT_UPDATE_POWER_MODES[];

extern const char STR_MODULE_RANGE[];
extern const char STR_RANGE_TEST[];
extern const char STR_RECEIVER_OPTIONS[];

extern const char STR_ABOUTUS[];

extern const char STR_AUTH_FAILURE[];
extern const char STR_PROTOCOL[];
extern const char STR_RACING_MODE[];

extern const char STR_SRC_BATT[];
extern const char STR_SRC_TIME[];
extern const char STR_SRC_GPS[];
extern const char STR_SRC_TIMER[];
extern const char STR_FUNC_SW[];

#if defined(COLORLCD)
extern const char STR_SELECT_TEMPLATE[];
extern const char STR_SELECT_TEMPLATE_FOLDER[];
extern const char STR_NO_TEMPLATES[];
extern const char STR_SAVE_TEMPLATE[];
extern const char STR_BLANK_MODEL[];
extern const char STR_BLANK_MODEL_INFO[];
extern const char STR_FILE_EXISTS[];
extern const char STR_ASK_OVERWRITE[];

extern const char STR_USE_THEME_COLOR[];

extern const char STR_DUPLICATE[];
extern const char STR_ACTIVATE[];
extern const char STR_RED[];
extern const char STR_BLUE[];
extern const char STR_GREEN[];
extern const char STR_COLOR_PICKER[];
extern const char STR_EDIT_THEME_DETAILS[];
extern const char STR_THEME_COLOR_DEFAULT[];
extern const char STR_THEME_COLOR_PRIMARY1[];
extern const char STR_THEME_COLOR_PRIMARY2[];
extern const char STR_THEME_COLOR_PRIMARY3[];
extern const char STR_THEME_COLOR_SECONDARY1[];
extern const char STR_THEME_COLOR_SECONDARY2[];
extern const char STR_THEME_COLOR_SECONDARY3[];
extern const char STR_THEME_COLOR_FOCUS[];
extern const char STR_THEME_COLOR_EDIT[];
extern const char STR_THEME_COLOR_ACTIVE[];
extern const char STR_THEME_COLOR_WARNING[];
extern const char STR_THEME_COLOR_DISABLED[];
extern const char STR_THEME_COLOR_CUSTOM[];
extern const char STR_THEME_CHECKBOX[];
extern const char STR_THEME_ACTIVE[];
extern const char STR_THEME_REGULAR[];
extern const char STR_THEME_WARNING[];
extern const char STR_THEME_DISABLED[];
extern const char STR_THEME_EDIT[];
extern const char STR_THEME_FOCUS[];
extern const char STR_AUTHOR[];
extern const char STR_DESCRIPTION[];
extern const char STR_SAVE[];
extern const char STR_CANCEL[];
extern const char STR_EDIT_THEME[];
extern const char STR_DETAILS[];
extern const char STR_THEME_EDITOR[];

extern const char STR_EDIT_LABELS[];
extern const char STR_LABEL_MODEL[];
extern const char STR_ENTER_LABEL[];
extern const char STR_LABEL[];
extern const char STR_LABELS[];
extern const char STR_SELECT_MODEL[];
extern const char STR_NONE[];
extern const char STR_ACTIVE[];
extern const char STR_NEW[];
extern const char STR_NEW_LABEL[];
extern const char STR_RENAME_LABEL[];
extern const char STR_DELETE_LABEL[];
extern const char STR_MAIN_MENU_SELECT_MODEL[];
extern const char STR_MAIN_MENU_MANAGE_MODELS[];
extern const char STR_MAIN_MENU_MODEL_NOTES[];
extern const char STR_MAIN_MENU_CHANNEL_MONITOR[];
extern const char STR_MAIN_MENU_MODEL_SETTINGS[];
extern const char STR_MAIN_MENU_RADIO_SETTINGS[];
extern const char STR_MAIN_MENU_SCREEN_SETTINGS[];
extern const char STR_MAIN_MENU_RESET_TELEMETRY[];
extern const char STR_MAIN_MENU_STATISTICS[];
extern const char STR_MAIN_MENU_ABOUT_EDGETX[];
extern const char STR_MANAGE_MODELS[];
#endif

#if defined(USBJ_EX)
extern const char STR_USBJOYSTICK_LABEL[];
extern const char STR_USBJOYSTICK_EXTMODE[];
extern const char* const STR_VUSBJOYSTICK_EXTMODE[];
extern const char STR_USBJOYSTICK_SETTINGS[];
extern const char STR_USBJOYSTICK_IF_MODE[];
extern const char* const STR_VUSBJOYSTICK_IF_MODE[];
extern const char STR_USBJOYSTICK_CH_MODE[];
extern const char* const STR_VUSBJOYSTICK_CH_MODE[];
extern const char* const STR_VUSBJOYSTICK_CH_MODE_S[];
extern const char STR_USBJOYSTICK_CH_BTNMODE[];
extern const char* const STR_VUSBJOYSTICK_CH_BTNMODE[];
extern const char* const STR_VUSBJOYSTICK_CH_BTNMODE_S[];
extern const char STR_USBJOYSTICK_CH_SWPOS[];
extern const char* const STR_VUSBJOYSTICK_CH_SWPOS[];
extern const char STR_USBJOYSTICK_CH_AXIS[];
extern const char* const STR_VUSBJOYSTICK_CH_AXIS[];
extern const char STR_USBJOYSTICK_CH_SIM[];
extern const char* const STR_VUSBJOYSTICK_CH_SIM[];
extern const char STR_USBJOYSTICK_CH_INVERSION[];
extern const char STR_USBJOYSTICK_CH_BTNNUM[];
extern const char STR_USBJOYSTICK_BTN_COLLISION[];
extern const char STR_USBJOYSTICK_AXIS_COLLISION[];
extern const char STR_USBJOYSTICK_CIRC_COUTOUT[];
extern const char* const STR_VUSBJOYSTICK_CIRC_COUTOUT[];
extern const char STR_USBJOYSTICK_APPLY_CHANGES[];
#endif

extern const char STR_SELECT_MENU_ALL[];
extern const char STR_SELECT_MENU_CLR[];
extern const char STR_SELECT_MENU_INV[];

#if defined(COLORLCD)
extern const char* const STR_SORT_ORDERS[];
extern const char STR_SORT_MODELS_BY[];
extern const char STR_CREATE_NEW[];
extern const char STR_THEME_EXISTS[];

extern const char STR_DATE_TIME_WIDGET[];
extern const char STR_RADIO_INFO_WIDGET[];
extern const char STR_LOW_BATT_COLOR[];
extern const char STR_MID_BATT_COLOR[];
extern const char STR_HIGH_BATT_COLOR[];
extern const char STR_WIDGET_SIZE[];
#endif

extern const char STR_MIX_SLOW_PREC[];
extern const char STR_MIX_DELAY_PREC[];

extern const char STR_DEL_DIR_NOT_EMPTY[];
