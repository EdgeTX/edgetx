/*  translation by adir kahsharo \ Motti Shonak.
 *  make configure system by stav raviv
 *  Ver 1.026
 *
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
 *
 */

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */

// Main menu
#define TR_QM_MANAGE_MODELS             "ניהול\nמודלים"
#define TR_QM_MODEL_SETUP               "הגדרת\nמודל"
#define TR_QM_RADIO_SETUP               "הגדרת\nשלט"
#define TR_QM_UI_SETUP                  "הגדרת\nממשק"
#define TR_QM_TOOLS                     "כלים"
#define TR_QM_MODEL_SETTINGS            "הגדרות\nמודל"
#define TR_QM_RADIO_SETTINGS            "הגדרות\nשלט"
#define TR_QM_FLIGHT_MODES              TR_SFC_AIR("מצבי\nנהיגה", "מצבי\nטיסה")
#define TR_QM_INPUTS                    "כניסות"
#define TR_QM_MIXES                     "מיקסים"
#define TR_QM_OUTPUTS                   "יציאות"
#define TR_QM_CURVES                    "עקומות"
#define TR_QM_GLOBAL_VARS               "משתנים\nגלובליים"
#define TR_QM_LOGICAL_SW                "מתגים\nלוגיים"
#define TR_QM_SPEC_FUNC                 "פונקציות\nמיוחדות"
#define TR_QM_CUSTOM_LUA                "סקריפטים\nמותאמים"
#define TR_QM_TELEM                     "טלמטריה"
#define TR_QM_GLOB_FUNC                 "פונקציות\nגלובליות"
#define TR_QM_TRAINER                   "טריינר"
#define TR_QM_HARDWARE                  "חומרה"
#define TR_QM_ABOUT                     "אודות\nEdgeTX"
#define TR_QM_THEMES                    "ערכות נושא"
#define TR_QM_TOP_BAR                   "בר עליון"
#define TR_QM_SCREEN_1                  "מסך 1"
#define TR_QM_SCREEN_2                  "מסך 2"
#define TR_QM_SCREEN_3                  "מסך 3"
#define TR_QM_SCREEN_4                  "מסך 4"
#define TR_QM_SCREEN_5                  "מסך 5"
#define TR_QM_SCREEN_6                  "מסך 6"
#define TR_QM_SCREEN_7                  "מסך 7"
#define TR_QM_SCREEN_8                  "מסך 8"
#define TR_QM_SCREEN_9                  "מסך 9"
#define TR_QM_SCREEN_10                 "מסך 10"
#define TR_QM_ADD_SCREEN                "הוסף\nמסך"
#define TR_QM_APPS                      "יישומים"
#define TR_QM_STORAGE                   "אחסון"
#define TR_QM_RESET                     TR_SFC_AIR("איפוס\nנהיגה", "איפוס\nטיסה")
#define TR_QM_CHAN_MON                  "תצוגת\nערוצים"
#define TR_QM_LS_MON                    "תצוגת\nמתגים לוגיים"
#define TR_QM_STATS                     "סטטיסטיקה"
#define TR_QM_DEBUG                     "ניפוי שגיאות"
#define TR_MAIN_MODEL_SETTINGS          "הגדרות מודל"
#define TR_MAIN_RADIO_SETTINGS          "הגדרות שלט"
#define TR_MAIN_MENU_MANAGE_MODELS      "ניהול מודלים"
#define TR_MAIN_MENU_MODEL_NOTES        "הערות מודל"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "תצוגת ערוצים"
#define TR_MONITOR_SWITCHES            "מסך מתגים לוגיים"
#define TR_MAIN_MENU_MODEL_SETTINGS     "הגדרות מודל"
#define TR_MAIN_MENU_RADIO_SETTINGS     "הגדרות שלט"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "הגדרות ממשק"
#define TR_MAIN_MENU_STATISTICS         "סטטיסטיקות"
#define TR_MAIN_MENU_ABOUT_EDGETX       "מידע על EdgeTX"
#define TR_MAIN_VIEW_X                  "מסך "
#define TR_MAIN_MENU_THEMES             "ערכות נושא"
#define TR_MAIN_MENU_APPS               "יישומים"
#define TR_MENUHELISETUP               "הגדרות מסוק"
#define TR_MENUFLIGHTMODES               TR_SFC_AIR("DRIVE MODES", "מצבי טיסה")
#define TR_MENUFLIGHTMODE                TR_SFC_AIR("DRIVE MODE", "מצב טיסה")
#define TR_MENUINPUTS                  "כניסות"
#define TR_MENULIMITS                  "יציאות"
#define TR_MENUCURVES                  "עקומות"
#define TR_MIXES                       "מיקסים"
#define TR_MENU_GLOBAL_VARS            "משתנים גלובלים"
#define TR_MENULOGICALSWITCHES         "מתגים לוגיים"
#define TR_MENUCUSTOMFUNC              "פונקציות מיוחדות"
#define TR_MENUCUSTOMSCRIPTS           "סקריפטים מיוחדים"
#define TR_MENUTELEMETRY               "טלמטריה"
#define TR_MENUSPECIALFUNCS            "פונקציות גלובליות"
#define TR_MENUTRAINER                 "טריינר"
#define TR_HARDWARE                    "הגדרות חומרה"
#define TR_USER_INTERFACE               "ממשק משתמש"
#define TR_SD_CARD                     "כרטיס זיכרון"
#define TR_DEBUG                       "אבחון"
#define TR_MENU_RADIO_SWITCHES         TR("בדיקת מתגים וכפתורים", "SWITCHES")
#define TR_MENUCALIBRATION              "כיול"
#define TR_FUNCTION_SWITCHES           "מתגים בהתאמה אישית"
// End Main menu

#define TR_MINUTE_SINGULAR            "דקה"
#define TR_MINUTE_PLURAL1             "דקות"
#define TR_MINUTE_PLURAL2             "דקות"

#define TR_OFFON_1                     "כבוי"
#define TR_OFFON_2                     "דלוק"
#define TR_MMMINV_1                    "---"
#define TR_MMMINV_2                    "הפוך"
#define TR_VBEEPMODE_1                 "שקט"
#define TR_VBEEPMODE_2                 "התראות"
#define TR_VBEEPMODE_3                 "ללא מקשים"
#define TR_VBEEPMODE_4                 "הכל"
#define TR_VBLMODE_1                   "כבוי"
#define TR_VBLMODE_2                   "מקשים"
#define TR_VBLMODE_3                   TR("בקרים", "בקרים")
#define TR_VBLMODE_4                   "שניהם"
#define TR_VBLMODE_5                   "דלוק"
#define TR_TRNMODE_1                   "כבוי"
#define TR_TRNMODE_2                   TR("+=","הוספה")
#define TR_TRNMODE_3                   TR(":=","החלפה")
#define TR_TRNCHN_1                    "CH1"
#define TR_TRNCHN_2                    "CH2"
#define TR_TRNCHN_3                    "CH3"
#define TR_TRNCHN_4                    "CH4"

#define TR_AUX_SERIAL_MODES_1          "כבוי"
#define TR_AUX_SERIAL_MODES_2          "שיקוף טלמטריה"
#define TR_AUX_SERIAL_MODES_3          "בטלמטריה"
#define TR_AUX_SERIAL_MODES_4          TR("SBUS טריינר הפוך", "SBUS טריינר הפוך")
#define TR_AUX_SERIAL_MODES_5          "SBUS טריינר"
#define TR_AUX_SERIAL_MODES_6          "LUA"
#define TR_AUX_SERIAL_MODES_7          "CLI"
#define TR_AUX_SERIAL_MODES_8          "GPS"
#define TR_AUX_SERIAL_MODES_9          "ניפוי שגיאות"
#define TR_AUX_SERIAL_MODES_10         "SpaceMouse"
#define TR_AUX_SERIAL_MODES_11         "מודול חיצוני"
#define TR_SWTYPES_1                   "ללא"
#define TR_SWTYPES_2                   "החלפה"
#define TR_SWTYPES_3                   "2POS"
#define TR_SWTYPES_4                   "3POS"
#define TR_SWTYPES_5                   "גלובלי"
#define TR_POTTYPES_1                  "ללא"
#define TR_POTTYPES_2                  "גלגלת"
#define TR_POTTYPES_3                  TR("גלגלת עם נקישות", "גלגלת עם נקישות")
#define TR_POTTYPES_4                  "סליידר"
#define TR_POTTYPES_5                  TR("רב-מצבי", "מתג רב-מצבי")
#define TR_POTTYPES_6                  "Axis X"
#define TR_POTTYPES_7                  "Axis Y"
#define TR_POTTYPES_8                  "מתג"
#define TR_VPERSISTENT_1               "כבוי"
#define TR_VPERSISTENT_2               "טיסה"
#define TR_VPERSISTENT_3               "איפוס ידני"
#define TR_COUNTRY_CODES_1             TR("US", "אמריקה")
#define TR_COUNTRY_CODES_2             TR("JP", "יפן")
#define TR_COUNTRY_CODES_3             TR("EU", "אירופה")
#define TR_USBMODES_1                  "שאל"
#define TR_USBMODES_2                  TR("Joyst","חיבור משחק קבוע")
#define TR_USBMODES_3                  TR("SDCard","העברת נתונים קבוע")
#define TR_USBMODES_4                  "חיבור סיריילי קבוע"
#define TR_JACK_MODES_1                "שאל"
#define TR_JACK_MODES_2                "סאונד"
#define TR_JACK_MODES_3                "טריינר"

#define TR_SBUS_INVERSION_VALUES_1     "נורמלי"
#define TR_SBUS_INVERSION_VALUES_2     "לא מהופך"
#define TR_MULTI_CUSTOM                "עיצוב מיוחד"
#define TR_VTRIMINC_1                  TR("Expo","אקספוננציאלי")
#define TR_VTRIMINC_2                  TR("ExFine","זז ב-1")
#define TR_VTRIMINC_3                  "זז ב-2"
#define TR_VTRIMINC_4                  "זז ב-4"
#define TR_VTRIMINC_5                  "זז ב-8"
#define TR_VDISPLAYTRIMS_1             "אל תציג"
#define TR_VDISPLAYTRIMS_2             "הצג שינוי בלבד"
#define TR_VDISPLAYTRIMS_3             "הצג"
#define TR_VBEEPCOUNTDOWN_1            "שקט"
#define TR_VBEEPCOUNTDOWN_2            "ציפצופים"
#define TR_VBEEPCOUNTDOWN_3            "שמע"
#define TR_VBEEPCOUNTDOWN_4            "רטט"
#define TR_VBEEPCOUNTDOWN_5            TR("צ & ר", "ציפצופים ורטט")
#define TR_VBEEPCOUNTDOWN_6            TR("ק & ר", "קול ורטט")
#define TR_COUNTDOWNVALUES_1           "5s"
#define TR_COUNTDOWNVALUES_2           "10s"
#define TR_COUNTDOWNVALUES_3           "20s"
#define TR_COUNTDOWNVALUES_4           "30s"
#define TR_VVARIOCENTER_1              "צליל"
#define TR_VVARIOCENTER_2              "שקט"
#define TR_CURVE_TYPES_1               "סטנדרטי"
#define TR_CURVE_TYPES_2               "עיצוב מיוחד"

#define TR_ADCFILTERVALUES_1           "גלובלי"
#define TR_ADCFILTERVALUES_2           "כבוי"
#define TR_ADCFILTERVALUES_3           "דלוק"

#define TR_VCURVETYPE_1                "דיפרנציאלי"
#define TR_VCURVETYPE_2                "אקספו"
#define TR_VCURVETYPE_3                "פונקציה"
#define TR_VCURVETYPE_4                "מיוחד"
#define TR_VMLTPX_1                    "הוספה"
#define TR_VMLTPX_2                    "הכפלה"
#define TR_VMLTPX_3                    "החלפה"

#define TR_CSWTIMER                    "שעון"
#define TR_CSWSTICKY                   TR("Stky", "Stcky")
#define TR_CSWSTAY                     "קצה"

#define TR_SF_TRAINER                  "טריינר"
#define TR_SF_INST_TRIM                "קיזוז מיידי"
#define TR_SF_RESET                    "איפוס"
#define TR_SF_SET_TIMER                "הגדר"
#define TR_SF_VOLUME                   "עוצמת קול"
#define TR_SF_FAILSAFE                 "הגדר כשל קליטה"
#define TR_SF_RANGE_CHECK              "בדיקת טווח"
#define TR_SF_MOD_BIND                 "צימוד מודול"
#define TR_SF_RGBLEDS                  "נורות RGB"

#define TR_SOUND                       "הפעל סאונד"
#define TR_PLAY_TRACK                  TR("Ply Trk", "נגן קובץ קול")
#define TR_PLAY_VALUE                  TR("Play Val","השמע ערך")
#define TR_SF_HAPTIC                   "רטט"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "הפעל סקריפט Lua")
#define TR_SF_BG_MUSIC                 "מוזיקת רקע"
#define TR_SF_BG_MUSIC_PAUSE           "השהיית מוזיקת רקע"
#define TR_SF_LOGS                     "לוגים לכרטיס SD"
#define TR_ADJUST_GVAR                 "התאם"
#define TR_SF_BACKLIGHT                "אור אחורי"
#define TR_SF_VARIO                    "וריומטר"
#define TR_SF_TEST                     "בדיקה"

#define TR_SF_SAFETY                   TR("עקיפה", "עקיפה")

#define TR_SF_SCREENSHOT               "צילום מסך"
#define TR_SF_RACING_MODE              "מצב תחרות"
#define TR_SF_DISABLE_TOUCH            "ללא מסך מגע"
#define TR_SF_DISABLE_KEYS             "ללא מקשים"
#define TR_SF_DISABLE_AUDIO_AMP        "מגבר שמע כבוי"
#define TR_SF_SET_SCREEN               TR_BW_COL("Set Screen", "הגדרת מסך ראשי")
#define TR_SF_PUSH_CUST_SWITCH         "לחץ מתג מותאם"
#define TR_SF_LCD_TO_VIDEO             "מסך לוידאו"

#define TR_FSW_RESET_TELEM             TR("Telm", "טלמטריה")
#define TR_FSW_RESET_TRIMS             "קיזוזים"
#define TR_FSW_RESET_TIMERS_1          "שעון 1"
#define TR_FSW_RESET_TIMERS_2          "שעון 2"
#define TR_FSW_RESET_TIMERS_3          "שעון 3"

#define TR_VFSWRESET_1                 TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                 TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                 TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                 TR("הכל","טיסה")
#define TR_VFSWRESET_5                 TR_FSW_RESET_TELEM
#define TR_VFSWRESET_6                 TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS_1                TR("Bp1","Beep1")
#define TR_FUNCSOUNDS_2                TR("Bp2","Beep2")
#define TR_FUNCSOUNDS_3                TR("Bp3","Beep3")
#define TR_FUNCSOUNDS_4                TR("Wrn1","Warn1")
#define TR_FUNCSOUNDS_5                TR("Wrn2","Warn2")
#define TR_FUNCSOUNDS_6                TR("Chee","Cheep")
#define TR_FUNCSOUNDS_7                TR("Rata","Ratata")
#define TR_FUNCSOUNDS_8                "Tick"
#define TR_FUNCSOUNDS_9                TR("Sirn","Siren")
#define TR_FUNCSOUNDS_10               "Ring"
#define TR_FUNCSOUNDS_11               TR("SciF","SciFi")
#define TR_FUNCSOUNDS_12               TR("Robt","Robot")
#define TR_FUNCSOUNDS_13               TR("Chrp","Chirp")
#define TR_FUNCSOUNDS_14               "Tada"
#define TR_FUNCSOUNDS_15               TR("Crck","Crickt")
#define TR_FUNCSOUNDS_16               TR("Alrm","AlmClk")

#define TR_VUNITSSYSTEM_1              "מטרי"
#define TR_VUNITSSYSTEM_2              TR("Imper.","אימפריאלי")
#define TR_VTELEMUNIT_1                "-"
#define TR_VTELEMUNIT_2                "V"
#define TR_VTELEMUNIT_3                "A"
#define TR_VTELEMUNIT_4                "mA"
#define TR_VTELEMUNIT_5                "kts"
#define TR_VTELEMUNIT_6                "m/s"
#define TR_VTELEMUNIT_7                "f/s"
#define TR_VTELEMUNIT_8                "kmh"
#define TR_VTELEMUNIT_9                "mph"
#define TR_VTELEMUNIT_10               "m"
#define TR_VTELEMUNIT_11               "ft"
#define TR_VTELEMUNIT_12               "°C"
#define TR_VTELEMUNIT_13               "°F"
#define TR_VTELEMUNIT_14               "%"
#define TR_VTELEMUNIT_15               "mAh"
#define TR_VTELEMUNIT_16               "W"
#define TR_VTELEMUNIT_17               "mW"
#define TR_VTELEMUNIT_18               "dB"
#define TR_VTELEMUNIT_19               "rpm"
#define TR_VTELEMUNIT_20               "g"
#define TR_VTELEMUNIT_21               "°"
#define TR_VTELEMUNIT_22               "rad"
#define TR_VTELEMUNIT_23               "ml"
#define TR_VTELEMUNIT_24               "fOz"
#define TR_VTELEMUNIT_25               "mlm"
#define TR_VTELEMUNIT_26               "Hz"
#define TR_VTELEMUNIT_27               "ms"
#define TR_VTELEMUNIT_28               "us"
#define TR_VTELEMUNIT_29               "km"
#define TR_VTELEMUNIT_30               "dBm"

#define TR_VTELEMSCREENTYPE_1          "ללא"
#define TR_VTELEMSCREENTYPE_2          "מספרים"
#define TR_VTELEMSCREENTYPE_3          "עמודות"
#define TR_VTELEMSCREENTYPE_4          "סקריפט"
#define TR_GPSFORMAT_1                 "DMS"
#define TR_GPSFORMAT_2                 "NMEA"


#define TR_VSWASHTYPE_1                "---"
#define TR_VSWASHTYPE_2                "120"
#define TR_VSWASHTYPE_3                "120X"
#define TR_VSWASHTYPE_4                "140"
#define TR_VSWASHTYPE_5                "90"

#define TR_STICK_NAMES0                "כיוון"
#define TR_STICK_NAMES1                "ה.גוב"
#define TR_STICK_NAMES2                "מצערת"
#define TR_STICK_NAMES3                "מאזנות"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#define TR_ON_ONE_SWITCHES_1           "ON"
#define TR_ON_ONE_SWITCHES_2           "אחד"

#define TR_HATSMODE                   "מצב כובעונים"
#define TR_HATSOPT_1                  "קיזוזים בלבד"
#define TR_HATSOPT_2                  "ניווט בלבד"
#define TR_HATSOPT_3                  "משולב"
#define TR_HATSOPT_4                  "גלובלי"
#define TR_HATSMODE_TRIMS             "מצב כובעונים: קיזוזים"
#define TR_HATSMODE_KEYS              "מצב כובעונים: ניווט"
#define TR_HATSMODE_KEYS_HELP          "Left side:\n"\
                                       "   Right = MDL\n"\
                                       "   Up = SYS\n"\
                                       "   Down = TELE\n"\
                                       "\n"\
                                       "Right side:\n"\
                                       "   Left = PAGE<\n"\
                                       "   Right = PAGE>\n"\
                                       "   Up = PREV/INC\n"\
                                       "   Down = NEXT/DEC"

#define TR_ROTARY_ENC_OPT_1       "רגיל"
#define TR_ROTARY_ENC_OPT_2       "הפוך"
#define TR_ROTARY_ENC_OPT_3       "V-I H-N"
#define TR_ROTARY_ENC_OPT_4       "V-I H-A"
#define TR_ROTARY_ENC_OPT_5       "V-N E-I"

#define TR_IMU_VSRCRAW_1             "TltX"
#define TR_IMU_VSRCRAW_2             "TltY"

#define TR_CYC_VSRCRAW_1             "CYC1"
#define TR_CYC_VSRCRAW_2             "CYC2"
#define TR_CYC_VSRCRAW_3             "CYC3"

#define TR_SRC_BATT                    "סוללה"
#define TR_SRC_TIME                    "זמן"
#define TR_SRC_GPS                     "GPS"
#define	TR_SRC_LIGHT                   "תאורה אחורית"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES_1                 "כבוי"
#define TR_VTMRMODES_2                 "דלוק"
#define TR_VTMRMODES_3                 "Strt"
#define TR_VTMRMODES_4                 "THs"
#define TR_VTMRMODES_5                 "TH%"
#define TR_VTMRMODES_6                 "THt"
#define TR_VTRAINER_MASTER_OFF         "כבוי"
#define TR_VTRAINER_MASTER_JACK        "שלט ראשי"
#define TR_VTRAINER_SLAVE_JACK         "שלט חניך"
#define TR_VTRAINER_MASTER_SBUS_MODULE "ראשי/מודול SBUS"
#define TR_VTRAINER_MASTER_CPPM_MODULE "ראשי/מודול CPPM"
#define TR_VTRAINER_MASTER_BATTERY     "ראשי/טורי"
#define TR_VTRAINER_BLUETOOTH_1        "ראשי/" TR("BT", "בלוטוס")
#define TR_VTRAINER_BLUETOOTH_2        "חניך/" TR("BT", "בלוטוס")
#define TR_VTRAINER_MULTI              "ראשי/Multi"
#define TR_VTRAINER_CRSF               "ראשי/CRSF"
#define TR_VFAILSAFE_1                 "לא מוגדר"
#define TR_VFAILSAFE_2                 "מוחזק"
#define TR_VFAILSAFE_3                 "הגדרה ידנית - מומלץ!"
#define TR_VFAILSAFE_4                 "ללא פולסים"
#define TR_VFAILSAFE_5                 "מקלט"
#define TR_VSENSORTYPES_1              "מותאם"
#define TR_VSENSORTYPES_2              "מחושב"
#define TR_VFORMULAS_1                 "הוסף"
#define TR_VFORMULAS_2                 "ממוצע"
#define TR_VFORMULAS_3                 "מינימלי"
#define TR_VFORMULAS_4                 "מקסימלי"
#define TR_VFORMULAS_5                 "הכפלה"
#define TR_VFORMULAS_6                 "סכום מצטבר"
#define TR_VFORMULAS_7                 "תא"
#define TR_VFORMULAS_8                 "צריכה"
#define TR_VFORMULAS_9                 "מרחק"
#define TR_VPREC_1                     "0.--"
#define TR_VPREC_2                     "0.0 "
#define TR_VPREC_3                     "0.00"
#define TR_VCELLINDEX_1                "הכי נמוך"
#define TR_VCELLINDEX_2                "1"
#define TR_VCELLINDEX_3                "2"
#define TR_VCELLINDEX_4                "3"
#define TR_VCELLINDEX_5                "4"
#define TR_VCELLINDEX_6                "5"
#define TR_VCELLINDEX_7                "6"
#define TR_VCELLINDEX_8                "7"
#define TR_VCELLINDEX_9                "8"
#define TR_VCELLINDEX_10               "הכי גבוה"
#define TR_VCELLINDEX_11               "הפרש"
#define TR_SUBTRIMMODES_1              CHAR_DELTA " (מרכז בלבד)"
#define TR_SUBTRIMMODES_2              "= (סימטרי)"
#define TR_TIMER_DIR_1                 TR("Remain", "הצג זמן שנותר")
#define TR_TIMER_DIR_2                 TR("Elaps.", "הצג זמן שחלף")

#define TR_FONT_SIZES_1                "STD"
#define TR_FONT_SIZES_2                "BOLD"
#define TR_FONT_SIZES_3                "XXS"
#define TR_FONT_SIZES_4                "XS"
#define TR_FONT_SIZES_5                "L"
#define TR_FONT_SIZES_6                "XL"
#define TR_FONT_SIZES_7                "XXL"
#define TR_FONT_SIZES_8                "LXL"

#define TR_ENTER                       "[ENTER]"
#define TR_OK                          TR_BW_COL(TR("\010\010\010[OK]", "\010\010\010\010\010[OK]"), "Ok")
#define TR_EXIT                        TR_BW_COL("חזרה", "RTN")

#define TR_YES                         "כן"
#define TR_NO                          "לא"
#define TR_DELETEMODEL                 "! מחיקת מודל"
#define TR_COPYINGMODEL                "...מעתיק מודל"
#define TR_MOVINGMODEL                 "...מעביר מודל"
#define TR_LOADINGMODEL                "...טוען מודל"
#define TR_UNLABELEDMODEL              "ללא שם"
#define TR_NAME                        "שם"
#define TR_MODELNAME                   "שם המודל"
#define TR_PHASENAME                   "שם המצב"
#define TR_MIXNAME                     "שם המיקס"
#define TR_INPUTNAME                   TR("כניסה", "שם כניסה")
#define TR_EXPONAME                    TR("שם", "שם שורה")
#define TR_BITMAP                      "תמונת מודל"
#define TR_NO_PICTURE                  "אין תמונה"
#define TR_TIMER                       TR("שעון", "שעון ")
#define TR_NO_TIMERS                   "ללא תזמון"
#define TR_START                       "התחלה"
#define TR_NEXT                        "הבא"
#define TR_ELIMITS                     TR("E.Limits", "הרחב טווחים")
#define TR_ETRIMS                      TR("E.Trims", "הרחב קיזוזים")
#define TR_TRIMINC                     "רגישות קיזוז"
#define TR_DISPLAY_TRIMS               TR("Show Trims", "הצג קיזוזים")
#define TR_TTRACE                      TR("T-Source", "מקור")
#define TR_TTRIM                       TR("T-Trim-Idle", "קיזוז סרק בלבד")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", "מתג קיזוז")
#define TR_BEEPCTR                     TR("Ctr Beep", "ציפצוף במרכז")
#define TR_PROTOCOL                    TR("Proto", "פרוטוקול")
#define TR_PPMFRAME                    "מסגרת PPM"
#define TR_REFRESHRATE                 TR("Refresh", "קצב רענון")
#define TR_WARN_BATTVOLTAGE           TR("יציאה VBAT: ", "אזהרה: רמת היציאה היא VBAT: ")
#define TR_WARN_5VOLTS                 "אזהרה: רמת המתח ביציאה היא 5 וולט"
#define TR_MS                          "ms"
#define TR_SWITCH                      "מתג"
#define TR_FS_COLOR_LIST_1             "מותאם"
#define TR_FS_COLOR_LIST_2             "כבוי"
#define TR_FS_COLOR_LIST_3             "לבן"
#define TR_FS_COLOR_LIST_4             "אדום"
#define TR_FS_COLOR_LIST_5             "ירוק"
#define TR_FS_COLOR_LIST_6             "צהוב"
#define TR_FS_COLOR_LIST_7             "כתום"
#define TR_FS_COLOR_LIST_8             "כחול"
#define TR_FS_COLOR_LIST_9             "ורוד"
#define TR_GROUP                       "קבוצה"
#define TR_GROUP_ALWAYS_ON             "דלוק תמיד"
#define TR_LUA_OVERRIDE                "אפשר עקיפת Lua"
#define TR_LAST                        "אחרון"
#define TR_MORE_INFO                   "מידע נוסף"
#define TR_SWITCH_TYPE                 "סוג"
#define TR_SWITCH_STARTUP              "אתחול"
#define TR_SWITCH_GROUP                "קבוצה"
#define TR_SF_SWITCH                   "הדק"
#define TR_TRIMS                       "קיזוזים"
#define TR_FADEIN                      "כניסה הדרגתית"
#define TR_FADEOUT                     "יציאה הדרגתית"
#define   TR_CHECKTRIMS                TR("\006Check\012trims", "Check FM Trims")
#define TR_SWASHTYPE                   "סוג סוואשפלייט"
#define TR_COLLECTIVE                  TR("קולקטיב", "מקור פיץ' קולקטיב")
#define TR_AILERON                     TR("Lateral cyc.", "Lateral cyc. source")
#define TR_ELEVATOR                    TR("Long. cyc.", "Long. cyc. source")
#define TR_SWASHRING                   "טבעת סוואשפלייט"
#define TR_MODE                        "מצב"
#define TR_LEFT_STICK                  "שמאל"
#define TR_SUBTYPE                     "תת-סוג"
#define TR_NOFREEEXPO                  "!אין אקספו פנוי"
#define TR_NOFREEMIXER                 "!אין מיקסר פנוי"
#define TR_SOURCE                       "מקור"
#define TR_WEIGHT                      "משקל"
#define TR_SIDE                        "צד"
#define TR_OFFSET                       "היסט"
#define TR_TRIM                        "קיזוז"
#define TR_CURVE                       "עקומה"
#define TR_FLMODE                      TR("מצב", "מצבים")
#define TR_MIXWARNING                  "התראה"
#define TR_OFF                         "כבוי"
#define TR_ANTENNA                     "אנטנה"
#define TR_NO_INFORMATION              TR("No info", "אין מידע")
#define TR_MULTPX                      "הגדר מצב מיקס"
#define TR_DELAYDOWN                   TR("Delay dn", "השהיית ירידה")
#define TR_DELAYUP                     "עיכוב בעלייה"
#define TR_SLOWDOWN                    TR("Slow dn", "האטת ירידה")
#define TR_SLOWUP                      "האט עלייה"
#define TR_CV                          "CV"
#define TR_GV                          TR("G", "GV")
#define TR_RANGE                       "טווח"
#define TR_CENTER                      "מרכז"
#define TR_ALARM                       "התראה"
#define TR_BLADES                      "להבים/קטבים"
#define TR_SCREEN                      "Screen\001"
#define TR_SOUND_LABEL                 "צליל"
#define TR_LENGTH                      "אורך"
#define TR_BEEP_LENGTH                 "אורך הצפצוף"
#define TR_BEEP_PITCH                  "צפצוף"
#define TR_HAPTIC_LABEL                "רטט"
#define TR_STRENGTH                    "חוזק"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "היסט"
#define TR_IMU_MAX                     "מקס"
#define TR_CONTRAST                    "ניגודיות"
#define TR_ALARMS_LABEL                "התראות"
#define TR_BATTERY_RANGE               TR("Batt. range", "הטווח להתראת סוללה")
#define TR_BATTERYCHARGING             "השלט מחובר להטענה..."
#define TR_BATTERYFULL                 "סוללה מלאה"
#define TR_BATTERYNONE                 "!אין"
#define TR_BATTERYWARNING              "התראת סוללה חלשה"
#define TR_INACTIVITYALARM             "התראת אי שימוש ממושכת"
#define TR_MEMORYWARNING               "נפח זיכרון נמוך"
#define TR_ALARMWARNING                "צליל כבוי"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI shutdown", "בדיקת חיבור למקלט בכיבוי")
#define TR_TRAINER_SHUTDOWN_ALARM          TR("התראת טריינר", "בדוק חיבור טריינר")
#define TR_MODEL_STILL_POWERED         "מודל עדיין פעיל"
#define TR_TRAINER_STILL_CONNECTED     "מצב חניך עדיין מחובר"
#define TR_USB_STILL_CONNECTED         "חיבור עדיין מחובר"
#define TR_MODEL_SHUTDOWN              "?לכבות"
#define TR_PRESS_ENTER_TO_CONFIRM      "לחץ אנטר לאישור"
#define TR_THROTTLE_LABEL              "מצערת"
#define TR_THROTTLE_START              "מצערת מתחילה"
#define TR_THROTTLEREVERSE             TR("T-Reverse", "היפוך")
#define TR_MINUTEBEEP                  TR("דקה", "הקראת דקות")
#define TR_BEEPCOUNTDOWN               "ספירה לאחור"
#define TR_PERSISTENT                  TR("Persist.", "קבוע")
#define TR_BACKLIGHT_LABEL             "תאורת רקע"
#define TR_STATUS_LED                  "Status LED"
#define TR_STATUS_LED_ERROR            "Error"
#define TR_STATUS_LED_READY            "Ready"
#define TR_STATUS_LED_EMIT             "Transmitting"
#define TR_STATUS_LED_COLORS_1         "Red"
#define TR_STATUS_LED_COLORS_2         "Green"
#define TR_STATUS_LED_COLORS_3         "Blue"
#define TR_STATUS                      "סטטוס"
#define TR_BLONBRIGHTNESS              "בהירות פעילה"
#define TR_BLOFFBRIGHTNESS             "בהירות כבויה"
#define TR_KEYS_BACKLIGHT              "תאורת לחצנים"
#define TR_BLCOLOR                     "צבע"
#define TR_ONE_LOG_PER_DAY             "לוג אחד ליום"
#define TR_KEY_LOCK_FMT                "נעילת מקשים (החזק %s+%s)"
#define TR_KEYS_LOCKED                 "המקשים נעולים"
#define TR_KEYS_LOCKED_FMT             TR_BW_COL("%s+%s לשחרור", "המקשים נעולים (%s+%s לשחרור)")
#define TR_KEYS_UNLOCKED               "המקשים משוחררים"
#define TR_TOUCH_ENABLED               "מסך מגע פעיל"
#define TR_TOUCH_DISABLED              "מסך מגע מושבת"
#define TR_SPLASHSCREEN                "מסך פתיחה"
#define TR_PLAY_HELLO                  "צליל אתחול"
#define TR_PWR_ON_DELAY                "השהיית הדלקה"
#define TR_PWR_OFF_DELAY               "משך לחיצה לכיבוי השלט"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","כיבוי אוטומטי")
#define TR_PWR_ON_OFF_HAPTIC           TR("רטט הפ/כב","רטט בהדלקה/כיבוי")
#define TR_THROTTLE_WARNING            TR("T-Warning", "התראת מצערת פתוחה")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Cust-Pos", "עריכת מיקום ידנית")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "מיקום %")
#define TR_SWITCHWARNING               TR("S-Warning", "אזהרת מיקום מתגים")
#define TR_POTWARNINGSTATE             "סליידרים וגלגלות"
#define TR_POTWARNING                  TR("Pot warn.", "אזהרת מיקום פוטנציומטרים")
#define TR_TIMEZONE                    "אזור זמן"
#define TR_ADJUST_RTC                  "כוונון שעון RTC"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Def chan order", "מצב ערוצים מקורי")
#define TR_STICKS                      "סטיקים"
#define TR_POTS                        "גלגלות"
#define TR_SWITCHES                    "מתגים"
#define TR_SWITCHES_DELAY              TR("Play delay", "השהיית השמעת מתג")
#define TR_SLAVE                       "Slave"
#define TR_MULTIPLIER                  "מכפיל"
#define TR_CAL                         "כיול"
#define TR_CALIBRATION                 BUTTON("כיול")
#define TR_VTRIM                       "קיזוז - +"
#define TR_CALIB_DONE                  "כיול הושלם"
#define TR_MENUTOSTART                 TR_ENTER " להתחלה"
#define TR_MENUWHENDONE                TR_ENTER " בסיום"
#define TR_AXISDIR                     "כיוון ציר"
#define TR_MENUAXISDIR                 "[ENTER LONG] " TR_AXISDIR
#define TR_SETMIDPOINT                 TR_BW_COL(TR_SFC_AIR("קבע אמצע גלגלות", TR("קבע אמצע סטיקים", "מרכז סטיקים/סליידרים")), "מרכז סטיקים/סליידרים")
#define TR_MOVESTICKSPOTS              TR_BW_COL(TR_SFC_AIR("הזז סטיקים/גלגלות/צירים", "הזז סטיקים/גלגלות"), "הזז סטיקים/גלגלות")
#define TR_NODATA                      "אין נתונים"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "סטאק פנוי"
#define TR_INT_GPS_LABEL               "GPS פנימי"
#define TR_HEARTBEAT_LABEL             "דפיקות לב"
#define TR_LUA_SCRIPTS_LABEL           "סקריפטי Lua"
#define TR_FREE_MEM_LABEL              "זיכרון פנוי"
#define TR_DURATION_MS                 TR("[D]", "משך (ms): ")
#define TR_INTERVAL_MS                 TR("[I]", "מרווח (ms): ")
#define TR_MEM_USED_SCRIPT             "סקריפט (B): "
#define TR_MEM_USED_WIDGET             "וידג'ט (B): "
#define TR_MEM_USED_EXTRA              "נוסף (B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Audio: "
#define TR_GPS_FIX_YES                 "נעילה: כן"
#define TR_GPS_FIX_NO                  "נעילה: לא"
#define TR_GPS_SATS                    "לוויינים: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Menu: "
#define TR_TIMER_LABEL                 "שעון"
#define TR_THROTTLE_PERCENT_LABEL      "מצערת %"
#define TR_BATT_LABEL                  "סוללה"
#define TR_SESSION                     "טיסה"
#define TR_MENUTORESET                 TR_ENTER " to reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "ערוץ "
#define TR_MODEL                       "מודל"
#define TR_FM                          TR_SFC_AIR("DM", "FM")
#define TR_PRESS_ANY_KEY_TO_SKIP       "לחץ על המסך או על כפתור"
#define TR_THROTTLE_NOT_IDLE           "סטיק מצערת פתוח"
#define TR_ALARMSDISABLED              "התראות בוטלו"
#define TR_PRESSANYKEY                 TR("\010Press any Key", "Press any key")
#define TR_BAD_RADIO_DATA              "נתוני שלט פגומים"
#define TR_RADIO_DATA_RECOVERED        TR3("שוחזר מגיבוי", "משתמש בגיבוי הגדרות שלט", "הגדרות השלט שוחזרו מגיבוי")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("הגדרות שלט שגויות", "הגדרות שלט לא תקינות", "לא ניתן לקרוא הגדרות שלט תקינות")
#define TR_STORAGE_FORMAT              "הכנת אחסון"
#define TR_RADIO_SETUP                 "הגדרות שלט ותצוגה"
#define TR_MENUVERSION                 "גירסא"
#define TR_MENU_RADIO_ANALOGS_CALIB    "בדיקת אנלוגיות סטיקים ומגע"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "אנלוגיים גולמיים (5 Hz)"
#define TR_MENU_FSWITCH                 "מתגים בהתאמה אישית"
#define   TR_TRIMS2OFFSETS              TR_BW_COL("\006Trims => Subtrims", "Trims => Subtrims")
#define TR_CHANNELS2FAILSAFE           "הגדר מצב נוכחי כקבוע"
#define TR_CHANNEL2FAILSAFE            "הגדר מצב נוכחי כקבוע"
#define TR_MENUMODELSEL                TR("בחירת מודל", "בחירת מודל")
#define TR_MENU_MODEL_SETUP            TR("הגדרות מודל", "הגדרות מודל")
#define TR_MENULOGICALSWITCH           "מתג לוגי"
#define TR_MENUSTAT                    "סטטוס"
#define TR_MENUDEBUG                   "אבחון"
#define TR_MONITOR_CHANNELS            "ערוצים %d-%d"
#define TR_MONITOR_OUTPUT_DESC         "יציאות"
#define TR_MONITOR_MIXER_DESC          "מיקסרים"
  #define TR_RECEIVER_NUM              TR("מס' מקלט", "מספר מקלט")
  #define TR_RECEIVER                  "מקלט"
#define TR_MULTI_RFTUNE                TR("Freq tune", "מיקוד התדר (fine-tuning)")
#define TR_MULTI_RFPOWER               "עוצמת שידור"
#define TR_MULTI_WBUS                  "יציאה"
#define TR_MULTI_TELEMETRY             "טלמטריה"
#define TR_MULTI_VIDFREQ               TR("תדר וידאו", "תדר וידאו")
#define TR_RF_POWER                    "עוצמת שידור"
#define TR_MULTI_FIXEDID               TR("ID קבוע", "מזהה קבוע")
#define TR_MULTI_OPTION                TR("אפשרות", "ערך אפשרות")
#define TR_MULTI_AUTOBIND              TR("Bind Ch.", "צימוד על ערוץ")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "השבתת מיפוי ערוצים")
#define TR_DSMP_ENABLE_AETR            TR("AETR", "הפעל AETR")
#define TR_DISABLE_TELEM               TR("No Telem", "השבתת טלמטריה")
#define TR_MULTI_LOWPOWER              TR("Low power", "מצב מתח נמוך")
#define TR_MULTI_LNA_DISABLE           "השבת LNA"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("פועל", "מופעל")
#define TR_DISABLE_INTERNAL            TR("Disable int.", "ביטול שידור פנימי")
#define TR_MODULE_NO_SERIAL_MODE       TR("!מצב טורי", "לא במצב טורי")
#define TR_MODULE_NO_INPUT             TR("אין קלט", "אין קלט טורי")
#define TR_MODULE_NO_TELEMETRY         TR3("אין טלמטריה", "אין MULTI_TELEMETRY", "לא זוהתה MULTI_TELEMETRY")
#define TR_MODULE_WAITFORBIND          "בצע צימוד לטעינת הפרוטוקול"
#define TR_MODULE_BINDING              TR("Bind...","קישוריות מתבצעת")
#define TR_MODULE_UPGRADE_ALERT        TR3("Upg. needed", "עדכון מודול נדרש", "Module\nUpgrade required")
#define TR_MODULE_UPGRADE              TR("Upg. advised", "עדכון מודול מומלץ")
#define TR_REBIND                      "נדרש צימוד מחדש"
#define TR_REG_OK                      "רישום בוצע"
#define TR_BIND_OK                     "הצימוד הצליח"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem OFF"
#define TR_PROTOCOL_INVALID            TR("Prot. invalid", "פרוטוקול לא תקין")
#define TR_MODULE_STATUS               TR("Status", "סטטוס מודול")
#define TR_MODULE_SYNC                 TR("סנכרון", "סטטוס סנכרון פרוטוקול")
#define TR_MULTI_SERVOFREQ             TR("קצב סרוו", "קצב עדכון סרוו")
#define TR_MULTI_MAX_THROW             TR("מהלך מקס.", "אפשר מהלך מקסימלי")
#define TR_MULTI_RFCHAN                TR("ערוץ RF", "בחר ערוץ RF")
#define TR_AFHDS3_RX_FREQ              TR("תדר RX", "תדר RX")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("עוצמה בפועל", "עוצמה בפועל")
#define TR_AFHDS3_POWER_SOURCE         TR("מקור מתח", "מקור מתח")
#define TR_IBUS2_SENSORS_MODE_ONLY     ".ניתן להגדיר חיישנים רק במצב iBUS2"
#define TR_GPS_COORDS_FORMAT           TR("קואורדינטות", "פורמט קואורדינטות")
#define TR_VARIO                       TR("Vario", "הגדרות תדר")
#define TR_PITCH_AT_ZERO               "תדר מינימלי"
#define TR_PITCH_AT_MAX                "תדר מקסימלי"
#define TR_REPEAT_AT_ZERO              "השהיית התראה"
#define TR_BATT_CALIB                  TR("Batt. calib", "כיול מתח סוללה ידני")
#define TR_VOLTAGE                     TR("מתח", "מקור מתח")
#define TR_SELECT_MODEL                "בחירת מודל"
#define TR_SELECT_MODE                 "בחירת מצב"
#define TR_CREATE_MODEL                "יצירת מודל"
#define TR_FAVORITE_LABEL              "מועדפים"
#define TR_MODELS_MOVED                "מודלים שלא בשימוש הועברו ל"
#define TR_NEW_MODEL                   "מודל חדש"
#define TR_LABEL_MODEL                 "שינוי תווית"
#define TR_MOVE_UP                     "עלה למעלה"
#define TR_MOVE_DOWN                   "רד למטה"
#define TR_ENTER_LABEL                 "הוספת תווית"
#define TR_LABELS                      "תוויות"
#define TR_ACTIVE                      "פעיל"
#define TR_NEW                         "חדש"
#define TR_NEW_LABEL                   "תווית חדשה"
#define TR_RENAME_LABEL                "שינוי שם תווית"
#define TR_DELETE_LABEL                "מחיקת תווית"
#define TR_DUPLICATE_MODEL             "הכפלת מודל"
#define TR_COPY_MODEL                  "העתקת מודל"
#define TR_MOVE_MODEL                  "העברת מודל"
#define TR_BACKUP_MODEL                "גיבוי מודל"
#define TR_DELETE_MODEL                "מחיקת מודל"
#define TR_RESTORE_MODEL               "שיחזור מודל"
#define TR_DELETE_INPUT_LINE           "מחיקת שורת קלט"
#define TR_DELETE_MIX_LINE             "מחיקת שורת מיקס"
#define TR_SDCARD_ERROR                TR("SD תקלת", "תיקיות כרטיס SD")
#define TR_SDCARD                      "SD כרטיס"
#define TR_NO_FILES_ON_SD              "!SD אין קבצים על"
#define TR_NO_SDCARD                   "SD אין כרטיס"
#define TR_WAITING_FOR_RX              "...ממתין ל-RX"
#define TR_WAITING_FOR_TX              "...ממתין ל-TX"
#define TR_WAITING_FOR_MODULE          TR("ממתין למודול", "...ממתין למודול")
#define TR_NO_TOOLS                    "אין כלי זמין"
#define TR_NORMAL                      "רגיל"
#define TR_NOT_INVERTED                "לא הפוך"
#define TR_NOT_CONNECTED               TR("!מחובר", "לא מחובר")
#define TR_CONNECTED                   "מחובר"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH without telem.", "16CH without telemetry")
#define TR_16CH_WITH_TELEMETRY         TR("16CH with telem.", "16CH with telemetry")
#define TR_EXT_ANTENNA                 "אנטנה חיצונית"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "? לעדכן אפשרויות RX"
#define TR_UPDATE_TX_OPTIONS           "? לעדכן אפשרויות TX"
#define TR_MODULES_RX_VERSION          BUTTON("מודולים / גרסת RX")
#define TR_SHOW_MIXER_MONITORS         "הצג תצוגת מיקסים"
#define TR_MENU_MODULES_RX_VERSION     "מודולים / גרסת RX"
#define TR_MENU_FIRM_OPTIONS           "אפשרויות קושחה"
#define TR_IMU                        "IMU"
#define TR_STICKS_POTS_SLIDERS         "סטיקים/גלגלות/סליידרים"
#define TR_RF_PROTOCOL                 "RF פרוטוקול"
#define TR_MODULE_OPTIONS              "אפשרויות מודול"
#define TR_POWER                       "עוצמה"
#define TR_NO_TX_OPTIONS               "אין אפשרויות TX"
#define TR_RTC_BATT                    "RTC סוללת פנימית"
#define TR_POWER_METER_EXT             "מד הספק (חיצוני)"
#define TR_POWER_METER_INT             "מד הספק (פנימי)"
#define TR_SPECTRUM_ANALYSER_EXT       "ספקטרום (חיצוני)"
#define TR_SPECTRUM_ANALYSER_INT       "ספקטרום (פנימי)"
#define TR_GHOST_MODULE_CONFIG         "הגדרות מודול Ghost"
#define TR_GPS_MODEL_LOCATOR           "איתור מודל ב-GPS"
#define TR_REFRESH                     "רעננן"
#define TR_SDCARD_FULL                 "הכרטיס מלא"
#define TR_SDCARD_FULL_EXT             TR_BW_COL(TR_SDCARD_FULL "\036לוגים" LCDW_128_LINEBREAK "ושמירת צילומי מסך מושבתים", TR_SDCARD_FULL "\036לוגים ושמירת צילומי מסך מושבתים")
#define TR_NEEDS_FILE                  "קובץ נדרש"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "לא תואם"
#define TR_WARNING                     "אזהרה"
#define TR_STORAGE_WARNING             "אחסון"
#define TR_THROTTLE_UPPERCASE          "מצערת"
#define TR_ALARMSWARN                  "התראות"
#define TR_SWITCHWARN                  TR("מתג", "בקר")
#define TR_FAILSAFEWARN                "כשל קליטה"
#define TR_TEST_WARNING                TR("בדיקה", "גרסת בדיקה")
#define TR_TEST_NOTSAFE                "שימוש לבדיקה בלבד"
#define TR_WARN_RTC_BATTERY_LOW        "סוללה פנימית נמוכה"
#define TR_WARN_MULTI_LOWPOWER         "מצב מתח נמוך"
#define TR_BATTERY                     "סוללה"
#define TR_WRONG_PCBREV                "זוהה כרטיס שגוי"
#define TR_EMERGENCY_MODE              "מצב חרום"
#define TR_NO_FAILSAFE                 "מצב חירום לא הוגדר"
#define TR_KEYSTUCK                    "כפתור לחוץ"
#define TR_VOLUME                      "עוצמת קול"
#define TR_BRIGHTNESS                  "בהירות"
#define TR_CONTROL                     "בקר"
#define TR_SF_OVERRIDDEN               "נעקף על ידי SF/GF"
#define TR_TTL_WARNING                 "!אזהרה: אין לעבור 3.3V בפינים TX/RX"
#define TR_FUNC                        "פונקציה"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "משך"
#define TR_DELAY                       "השהיה"
#define TR_NO_SOUNDS_ON_SD             "SD אין צלילים על"
#define TR_NO_MODELS_ON_SD             "SD אין מודלים על"
#define TR_NO_BITMAPS_ON_SD            "אין תמונות בכרטיס"
#define TR_NO_SCRIPTS_ON_SD            "אין סקריפט בכרטיס"
#define TR_SCRIPT_SYNTAX_ERROR         TR("שגיאת תחביר", "שגיאת תחביר בסקריפט")
#define TR_SCRIPT_PANIC                "קריסת סקריפט"
#define TR_SCRIPT_ERROR                "תקלה לא ידועה"
#define TR_PLAY_FILE                   "נגן"
#define TR_DELETE_FILE                 "מחק"
#define TR_COPY_FILE                   "העתק"
#define TR_RENAME_FILE                 "שינוי שם"
#define TR_ASSIGN_BITMAP               "בחר תמונה"
#define TR_EXECUTE_FILE                "בצע"
#define TR_REMOVED                     " הוסר"
#define TR_SD_INFO                     "מידע"
#define TR_NA                          "לא רלוונטי"
#define TR_TIME                        "זמן"
#define TR_BAUDRATE                    "קצב שידור"
#define TR_CRSF_ARMING_MODE            "הפעלת מנוע באמצעות"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 "מצב דגימה"
#define TR_SAMPLE_MODES_1              "נורמלי"
#define TR_SAMPLE_MODES_2              "ביט אחד"
#define TR_LOADING                     "...טוען"
#define TR_DELETE_THEME                "?למחוק את הערכה"
#define TR_SAVE_THEME                  "?לשמור את הערכה"
#define TR_EDIT_COLOR                  "עריכת צבע"
#define TR_NO_THEME_IMAGE              "תמונת ערכת נושא"
#define TR_BACKLIGHT_TIMER             "זמן אי פעילות"

#define TR_MODEL_QUICK_SELECT        "בחירת מודל מהירה"
#define TR_LABELS_SELECT             "בחירת תווית"
#define TR_LABELS_MATCH              "התאמת תוויות"
#define TR_FAV_MATCH                 "התאמת מועדפים"
#define TR_LABELS_SELECT_MODE_1      "בחירה מרובה"
#define TR_LABELS_SELECT_MODE_2      "בחירת יחיד"
#define TR_LABELS_MATCH_MODE_1       "התאם את כולם"
#define TR_LABELS_MATCH_MODE_2       "התאם כלשהו"
#define TR_FAV_MATCH_MODE_1          "חייב להתאים"
#define TR_FAV_MATCH_MODE_2          "התאמה אופציונלית"

#define TR_NO_TEMPLATES                "אין מודל בספריית התבניות"
#define TR_SAVE_TEMPLATE               "שמור כתבנית"
#define TR_BLANK_MODEL                 "מודל ריק"
#define TR_BLANK_MODEL_INFO            "יצירת מודל ריק"
#define TR_FILE_EXISTS                 "קובץ כבר קיים"
#define TR_ASK_OVERWRITE               "האם אתה רוצה לכתוב על הקיים?"

#define TR_BLUETOOTH                   "בלוטוס"
#define TR_BLUETOOTH_INIT              "אתחול"
#define TR_BLUETOOTH_DIST_ADDR         "כתובת יעד"
#define TR_BLUETOOTH_LOCAL_ADDR        "כתובת מקומית"
#define TR_BLUETOOTH_PIN_CODE          "קוד PIN"
#define TR_BLUETOOTH_NODEVICES         "לא נמצאו מכשירים"
#define TR_BLUETOOTH_SCANNING          "...סורק"
#define TR_BLUETOOTH_MODES_1           "---"
#define TR_BLUETOOTH_MODES_2           "טלמטריה"
#define TR_BLUETOOTH_MODES_3           "טריינר"
#define TR_BLUETOOTH_MODES_4           "מופעל"

#define TR_SD_INFO_TITLE               "מידע כרטיס"
#define TR_SD_SECTORS                  ":סקטורים"
#define TR_SD_SIZE                     ":גודל"
#define TR_TYPE                        "סוג"
#define TR_GVARS                       "GVARS"
#define TR_GLOBAL_VAR                  "משתנה גלובלי"
#define TR_OWN                         "בעלים"
#define TR_DATE                        "תאריך"
#define TR_MONTHS_1                    "ינו"
#define TR_MONTHS_2                    "פבו"
#define TR_MONTHS_3                    "מרץ"
#define TR_MONTHS_4                    "אפר"
#define TR_MONTHS_5                    "מאי"
#define TR_MONTHS_6                    "יונ"
#define TR_MONTHS_7                    "יול"
#define TR_MONTHS_8                    "אוג"
#define TR_MONTHS_9                    "ספט"
#define TR_MONTHS_10                   "אוק"
#define TR_MONTHS_11                   "נוב"
#define TR_MONTHS_12                   "דצמ"
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("מצב מקודד", "מצב מקודד סיבובי")
#define TR_CHANNELS_MONITOR            "תצוגת ערוצים"
#define TR_MIXERS_MONITOR              "תצוגת מיקסים"
#define TR_PATH_TOO_LONG               "נתיב ארוך מדי"
#define TR_VIEW_TEXT                   "הצג טקסט"
#define TR_FLASH_BOOTLOADER            "צריבת bootloader"
#define TR_FLASH_DEVICE                TR("צריבת התקן", "צריבת התקן")
#define TR_FLASH_EXTERNAL_DEVICE       TR("צריבת S.Port", "צריבת התקן S.Port")
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "צריבת מקלט ב-OTA חיצוני"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "צריבת מקלט ב-OTA פנימי"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "צריבת בקר טיסה ב-OTA חיצוני"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "צריבת בקר טיסה ב-OTA פנימי"
#define TR_FLASH_BLUETOOTH_MODULE      TR("צריבת מודול BT", "צריבת מודול בלוטוס")
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "התקן לא מגיב")
#define TR_DEVICE_FILE_ERROR           TR("בעיית קובץ התקן", "בעיית קובץ התקן")
#define TR_DEVICE_DATA_REFUSED         TR("נתוני התקן נדחו", "נתוני התקן נדחו")
#define TR_DEVICE_WRONG_REQUEST        TR("בעיית גישה להתקן", "בעיית גישה להתקן")
#define TR_DEVICE_FILE_REJECTED        TR("קובץ התקן נדחה", "קובץ התקן נדחה")
#define TR_DEVICE_FILE_WRONG_SIG       TR("חתימת קובץ שגויה", "חתימת קובץ התקן שגויה")
#define TR_CURRENT_VERSION             TR("גרסה: ", "גרסה נוכחית: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Flash int. module", "צריבת מודול פנימי")
#define TR_FLASH_INTERNAL_MULTI        TR("צריבת מודול פנימי (Multi)", "צריבת מודול פנימי (Multi)")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash ext. module", "צריבת מודול חיצוני")
#define TR_FLASH_EXTERNAL_MULTI        TR("צריבת מודול חיצוני (Multi)", "צריבת מודול חיצוני (Multi)")
#define TR_FLASH_EXTERNAL_ELRS         TR("צריבת מודול חיצוני (ELRS)", "צריבת מודול חיצוני (ELRS)")
#define TR_FIRMWARE_UPDATE_ERROR       TR("FW update error", "שגיאת עדכון קושחה")
#define TR_FIRMWARE_UPDATE_SUCCESS     "! הפלאש עבר בהצלחה"
#define TR_WRITING                     "...כותב"
#define TR_INTERNALRF                  "מודול פנימי"
#define TR_INTERNAL_MODULE             TR("Int. module", "מודול פנימי")
#define TR_EXTERNAL_MODULE             TR("Ext. module", "מודול חיצוני")
#define TR_EDGETX_UPGRADE_REQUIRED     "נדרש עדכון EdgeTX"
#define TR_TELEMETRY_DISABLED          "טלמטריה מושבתת"
#define TR_MORE_OPTIONS_AVAILABLE      "אפשרויות נוספות זמינות"
#define TR_EXTERNALRF                  "מודול חיצוני"
#define TR_FAILSAFE                    TR("Failsafe", "הגדרת כשל קליטה")
#define TR_FAILSAFESET                 "הגדרת כשל קליטה"
#define TR_REG_ID                      "מזהה רישום"
#define TR_OWNER_ID                    "מזהה בעלים"
#define TR_HOLD                        "מוחזק"
#define TR_HOLD_UPPERCASE              "מוחזק"
#define TR_NONE                        "ללא"
#define TR_NONE_UPPERCASE              "ללא"
#define TR_MENUSENSOR                  "חיישן"
#define TR_POWERMETER_PEAK             "שיא"
#define TR_POWERMETER_POWER            "הספק"
#define TR_POWERMETER_ATTN             "החלשה"
#define TR_POWERMETER_FREQ             "תדר"
#define TR_MENUTOOLS                   "כלים וסקריפטים"
#define TR_MIC_RECORDER                "מקליט מיקרופון"
#define TR_PUSH_TO_RECORD              "לחץ להקלטה"
#define TR_RECORD                      "הקלט"
#define TR_STOP                        "עצור"
#define TR_REC                         "REC"
#define TR_STARTING_IN                 "מתחיל בעוד"
#define TR_GET_READY                   "...היה מוכן"
#define TR_SAVED                       ":נשמר"
#define TR_SAVE_AS                     "שמור בשם"
#define TR_AUTO_TRIM                   "קיזוז אוטומטי"
#define TR_TRIM_START                  "חיתוך התחלה"
#define TR_TRIM_END                    "חיתוך סוף"
#define TR_OPEN_ERROR                  "שגיאת פתיחה"
#define TR_TURN_OFF_RECEIVER           "כיבוי מקלט"
#define TR_STOPPING                    "...עוצר"
#define TR_MENU_SPECTRUM_ANALYSER      "מנתח ספקטרום"
#define TR_MENU_POWER_METER            "מד הספק"
#define TR_SENSOR                      "חיישן"
#define TR_COUNTRY_CODE                "קוד ארץ"
#define TR_USBMODE                     "USB מצב"
#define TR_USB_CHARGE                  "טעינה כאשר השלט פועל"
#define TR_JACK_MODE                   "מצב שקע"
#define TR_VOICE_LANGUAGE              "שפת שמע"
#define TR_TEXT_LANGUAGE               "שפת הטקסט"
#define TR_UNITS_SYSTEM                "יחידות"
#define TR_UNITS_PPM                   "יחידות PPM"
#define TR_EDIT                        "ערוך"
#define TR_INSERT_BEFORE               "הכנס לפני"
#define TR_INSERT_AFTER                "הכנס אחרי"
#define TR_COPY                        "העתקה"
#define TR_MOVE                        "הזז"
#define TR_PASTE                       "הדבקה"
#define TR_PASTE_AFTER                 "הדבק אחרי"
#define TR_PASTE_BEFORE                "הדבק לפני"
#define TR_DELETE                      "מחק"
#define TR_INSERT                      "הכנס"
#define TR_RESET_SESSION               "איפוס טיסה"
#define TR_RESET_TIMER1                "איפוס שעון 1 "
#define TR_RESET_TIMER2                "איפוס שעון 2"
#define TR_RESET_TIMER3                "איפוס שעון 3"
#define TR_RESET_TELEMETRY             "איפוס טלמטריה"
#define TR_STATISTICS                  "סטטיסטיקות"
#define TR_ABOUT_US                    "מידע על"
#define TR_USB_JOYSTICK                "חיבור משחק (HID)"
#define TR_USB_MASS_STORAGE            "חיבור העברת נתונים (SD)"
#define TR_USB_SERIAL                  "חיבור סריילי (VCP)"
#define TR_AND_SWITCH                  "מתג AND"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "בדיקת אנלוגיות סטיקים ומגע"
#define TR_ANADIAGS_FILTRAWDEV         "אנלוגיים גולמיים מסוננים עם סטייה"
#define TR_ANADIAGS_UNFILTRAW          "אנלוגיים גולמיים לא מסוננים"
#define TR_ANADIAGS_MINMAX             "מינימום, מקסימום וטווח"
#define TR_ANADIAGS_MOVE               "!הזז את האנלוגיים לקצוות"
#define TR_BYTES                       "בייטים"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "צימוד"))
#define TR_MODULE_UNBIND               BUTTON("בטל צימוד")
#define TR_POWERMETER_ATTN_NEEDED     "נדרש מחליש אות"
#define TR_PXX2_SELECT_RX              "בחר מקלט"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "בחר מכשיר"
#define TR_DISCOVER                    BUTTON("גלה")
#define TR_BUTTON_INIT                 BUTTON("אתחול")
#define TR_WAITING                     "...ממתין"
#define TR_RECEIVER_DELETE             "? מחיקת מקלט"
#define TR_RECEIVER_RESET              "? איפוס מקלט"
#define TR_SHARE                       "שיתוף"
#define TR_BIND                        "צימוד"
#define TR_PAIRING                     "התאמה"
#define TR_BTAUDIO                     "שמע BT"
#define TR_REGISTER                    BUTTON(TR("רשום", "רישום"))
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "בדיקת טווח קליטה"))
#define TR_RANGE_TEST                  "בדיקת טווח"
#define TR_RECEIVER_OPTIONS            TR("אפשרויות מקלט", "אפשרויות מקלט")
#define TR_RESET_BTN                   BUTTON("איפוס")
#define TR_KEYS_BTN                    BUTTON("בדיקת מתגים")
#define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "בדיקת אנלוגיות"))
#define TR_FS_BTN                      BUTTON(TR("מתגים מותאמים", TR_FUNCTION_SWITCHES))
#define TR_SET                         BUTTON("הגדר")
#define TR_TRAINER                     "טריינר"
#define TR_CHANS                       "ערוצים"
#define TR_ANTENNAPROBLEM              "בעיה באנטנת שידור!"
#define TR_MODELIDUSED                 "מספר המקלט בשימוש במודל:"
#define TR_MODELIDUNIQUE               "מספר המקלט פנוי לשימוש"
#define TR_MODULE                      "מודול"
#define TR_RX_NAME                     "שם מקלט"
#define TR_TELEMETRY_TYPE              TR("סוג", "סוג טלמטריה")
#define TR_TELEMETRY_SENSORS           "חיישנים"
#define TR_VALUE                       "ערך"
#define TR_PERIOD                      "פרק זמן"
#define TR_INTERVAL                    "מרווח"
#define TR_REPEAT                      "מספר חזרות"
#define TR_ENABLE                      "הפעל"
#define TR_DISABLE                     "השבת"
#define TR_TOPLCDTIMER                 "שעון מסך עליון"
#define TR_UNIT                        "יחידה"
#define TR_TELEMETRY_NEWSENSOR         "הוסף חדש"
#define TR_CHANNELRANGE                TR("Ch. Range", "טווח ערוצים")
#define TR_ANTENNACONFIRM1             "אנטנה חיצונית"
#define TR_ANTENNA_MODES_1           "פנימי"
#define TR_ANTENNA_MODES_2           "שאל"
#define TR_ANTENNA_MODES_3           "לפי מודל"
#define TR_ANTENNA_MODES_4           "פנימי וחיצוני"
#define TR_ANTENNA_MODES_5           "חיצוני"
#define TR_USE_INTERNAL_ANTENNA        TR("אנטנה פנימית", "השתמש באנטנה פנימית")
#define TR_USE_EXTERNAL_ANTENNA        TR("אנטנה חיצונית", "השתמש באנטנה חיצונית")
#define TR_ANTENNACONFIRM2             TR("בדוק אנטנה", "!וודא שהאנטנה מותקנת")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Requires FLEX non"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Requires FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Requires EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "certified firmware"
#define TR_LOWALARM                    "התראה נמוכה"
#define TR_CRITICALALARM               "התראה קריטית"
#define TR_DISABLE_ALARM               TR("ביטול התראות", "ביטול התראות טלמטריה")
#define TR_POPUP                       "התראות"
#define TR_MIN                         "מינימום"
#define TR_MAX                         "מקסימום"
#define TR_CURVE_PRESET                "...הגדרות מוכנות"
#define TR_PRESET                      "הגדרה מוכנה"
#define TR_MIRROR                      "היפוך"
#define TR_CLEAR                       "ניקוי"
#define TR_CLEAR_BTN                   BUTTON("ניקוי")
#define TR_RESET                       "איפוס"
#define TR_RESET_SUBMENU               "...איפוס"
#define TR_COUNT                       "ספירה"
#define TR_PT                          "נקודה"
#define TR_PTS                         "נקודות"
#define TR_SMOOTH                      "מעוגל"
#define TR_COPY_STICKS_TO_OFS          TR("העתק סטיק לסאב-טרים", "העתק סטיקים לסאב-טרים")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("העתק מינ/מקס לכולם", "העתק מינימום/מקסימום/מרכז לכל היציאות")
#define TR_COPY_TRIMS_TO_OFS           TR("העתק קיזוז לסאב-טרים", "העתק קיזוזים לסאב-טרים")
#define TR_INCDEC                      "הגדל/הקטן"
#define TR_GLOBALVAR                   "משתנה גלובלי"
#define TR_MIXSOURCE                   "מקור (%)"
#define TR_MIXSOURCERAW                "מקור (ערך)"
#define TR_CONSTANT                    "קבוע"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1 "כבוי"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2 "פועל"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3 "אוטומטי"
#define TR_PREFLIGHT                   "בדיקות טרום טיסה"
#define TR_CHECKLIST                   TR("Checklist", "הצג רשימת בדיקה")
#define TR_CHECKLIST_INTERACTIVE       TR3("אינטראקטיבי", "רשימה אינטראקטיבית", "רשימת בדיקה אינטראקטיבית")
#define TR_AUX_SERIAL_MODE             "חיבור טורי"
#define TR_AUX_SERIAL_PORT_POWER       "הפעלת מתח בחיבור"
#define TR_SCRIPT                      "סקריפט"
#define TR_INPUTS                      "כניסות"
#define TR_OUTPUTS                     "יציאות"
#define TR_TOO_MANY_LUA_SCRIPTS        "!יותר מדי סקריפטי Lua"
#define TR_SPORT_UPDATE_POWER_MODE     "מתח SP"
#define TR_SPORT_UPDATE_POWER_MODES_1  "אוטומטי"
#define TR_SPORT_UPDATE_POWER_MODES_2  "פעיל"
#define TR_NO_TELEMETRY_SCREENS        "אין מסכי טלמטריה"
#define TR_TOUCH_PANEL                 "מסך מגע:"
#define TR_FILE_SIZE                   "גודל קובץ"
#define TR_FILE_OPEN                   "? לפתוח בכל זאת"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "שם"
#define TR_PHASES_HEADERS_SW           "מתג"
#define TR_PHASES_HEADERS_RUD_TRIM     "קיזוז הגה כיוון"
#define TR_PHASES_HEADERS_ELE_TRIM     "קיזוז הגה גובה"
#define TR_PHASES_HEADERS_THT_TRIM     "קיזוז מנוע"
#define TR_PHASES_HEADERS_AIL_TRIM     "קיזוז מאזנות"
#define TR_PHASES_HEADERS_CH5_TRIM     "T5 כפתור"
#define TR_PHASES_HEADERS_CH6_TRIM     "T6 כפתור"
#define TR_PHASES_HEADERS_FAD_IN       "כניסה הדרגתית"
#define TR_PHASES_HEADERS_FAD_OUT      "יציאה הדרגתית"

#define TR_LIMITS_HEADERS_NAME         "שם"
#define TR_LIMITS_HEADERS_SUBTRIM      "סאב-טרים"
#define TR_LIMITS_HEADERS_MIN          "מינימום"
#define TR_LIMITS_HEADERS_MAX          "מקסימום"
#define TR_LIMITS_HEADERS_DIRECTION    "כיוון"
#define TR_LIMITS_HEADERS_CURVE        "עקומה"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM מרכז"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "מצב סאב-טרים"
#define TR_INVERTED                    "היפוך"

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "ערוץ ראשון"
#define TR_LAST_CHANNEL              "ערוץ אחרון"
#define TR_FILL_BACKGROUND           "מילוי רקע?"
#define TR_BG_COLOR                  "צבע רקע"
#define TR_SLIDERS                   "סליידרים"
#define TR_FLIGHT_MODE               "מצב טיסה"
#define TR_TIMER_SOURCE              "מקור השעון"
#define TR_SIZE                      "גודל"
#define TR_SHADOW                    "הצללה"
#define TR_ALIGNMENT                 "יישור"
#define TR_ALIGN_LABEL               "יישור התווית"
#define TR_ALIGN_VALUE               "יישור הערך"
#define TR_ALIGN_OPTS_1              "שמאל"
#define TR_ALIGN_OPTS_2              "מרכז"
#define TR_ALIGN_OPTS_3              "ימין"
#define TR_TEXT                      "טקסט"
#define TR_COLOR                     "צבע"
#define TR_PANEL1_BACKGROUND         "רקע פאנל 1"
#define TR_PANEL2_BACKGROUND         "רקע פאנל 2"
#define TR_PANEL_BACKGROUND          "רקע"
#define TR_PANEL_COLOR               "צבע"
#define TR_WIDGET_GAUGE              "מדיד"
#define TR_WIDGET_MODELBMP           "מידע מודל"
#define TR_WIDGET_OUTPUTS            "יציאות"
#define TR_WIDGET_TEXT               "טקסט"
#define TR_WIDGET_TIMER              "שעון"
#define TR_WIDGET_VALUE              "ערך"

// About screen
#define TR_ABOUTUS                     TR(" אודות ", "אודות")

#define TR_CHR_INPUT                   "I"   // Values between A-I will work

#define TR_BEEP_VOLUME                 "עוצמת ציפצוף"
#define TR_WAV_VOLUME                  "עוצמת גל"
#define TR_BG_VOLUME                   TR("Bg volume", "עוצמת צליל רקע")

#define TR_TOP_BAR                     "בר עליון"
#define TR_FLASH_ERASE                 "...מוחק פלאש"
#define TR_FLASH_WRITE                 "...כותב פלאש"
#define TR_OTA_UPDATE                  "...עדכון OTA"
#define TR_MODULE_RESET                "...איפוס מודול"
#define TR_UNSUPPORTED_RX              "מקלט לא נתמך"
#define TR_OTA_UPDATE_ERROR            "שגיאת עדכון OTA"
#define TR_DEVICE_RESET                "...איפוס התקן"
#define TR_ALTITUDE                    "גובה"
#define TR_SCALE                       "קנה מידה"
#define TR_VIEW_CHANNELS               "הצג ערוצים"
#define TR_VIEW_NOTES                  "הצג פתקים"
#define TR_ID                          "ID"
#define TR_PRECISION                   "דיוק עשרוני"
#define TR_RATIO                       "יחס"
#define TR_FORMULA                     "נוסחה"
#define TR_CELLINDEX                   "מיקום תא"
#define TR_LOGS                        "לוגים"
#define TR_OPTIONS                     "אפשרויות"
#define TR_FIRMWARE_OPTIONS            BUTTON("אפשרויות קושחה")

#define TR_ALTSENSOR                   "חיישן גובה"
#define TR_CELLSENSOR                  "חיישן תאים"
#define TR_GPSSENSOR                   "חיישן GPS"
#define TR_GYRO                        "ג'יירו"
#define TR_CURRENTSENSOR               "חיישן"
#define TR_AUTOOFFSET                  "היסט אוטומטי"
#define TR_ONLYPOSITIVE                "חיובי"
#define TR_FILTER                      "פילטר"
#define TR_TELEMETRYFULL               TR("!כל המשבצות מלאות", "!כל משבצות הטלמטריה מלאות")
#define TR_IGNORE_INSTANCE             TR("ללא מופע", "התעלם ממופעים")
#define TR_SHOW_INSTANCE_ID            "הצג מזהה"
#define TR_DISCOVER_SENSORS            "גלה חדשים"
#define TR_STOP_DISCOVER_SENSORS       "עצור"
#define TR_DELETE_ALL_SENSORS          "מחק הכל"
#define TR_CONFIRMDELETE               "באמת " LCDW_128_LINEBREAK "? למחוק הכל"
#define TR_SELECT_WIDGET               "בחירת וידג'ט"
#define TR_WIDGET_FULLSCREEN           "מסך מלא"
#define TR_REMOVE_WIDGET               "הסרת וידג'ט"
#define TR_WIDGET_SETTINGS             "הגדרות וידג'ט"
#define TR_REMOVE_SCREEN               "הסרת מסך"
#define TR_SETUP_WIDGETS               "הגדרות וידג'טים"
#define TR_THEME                       "ערכת נושא"
#define TR_LAYOUT                      "תצוגת מסך"
#define TR_TEXT_COLOR                  "צבע טקסט"
#define TR_MENU_INPUTS                 CHAR_INPUT "כניסות"
#define TR_MENU_LUA                    CHAR_LUA "Lua סקריפטים"
#define TR_MENU_STICKS                 CHAR_STICK "סטיקים"
#define TR_MENU_POTS                   CHAR_POT "גלגלות"
#define TR_MENU_MIN                    CHAR_FUNCTION "מינימום"
#define TR_MENU_MAX                    CHAR_FUNCTION "מקסימום"
#define TR_MENU_HELI                   CHAR_CYC "ציקלי"
#define TR_MENU_TRIMS                  CHAR_TRIM "קיזוזים"
#define TR_MENU_SWITCHES               CHAR_SWITCH "מתגים"
#define TR_MENU_LOGICAL_SWITCHES       CHAR_SWITCH "מתגים לוגיים"
#define TR_MENU_TRAINER                CHAR_TRAINER "טריינר"
#define TR_MENU_CHANNELS               CHAR_CHANNEL "ערוצים"
#define TR_MENU_GVARS                  CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY              CHAR_TELEMETRY "טלמטריה"
#define TR_MENU_DISPLAY                "תצוגה"
#define TR_MENU_OTHER                  "אחר"
#define TR_MENU_INVERT                 "הפוך"
#define TR_AUDIO_MUTE                  TR("השתקת קול","השתק כאשר אין סאונד")
#define TR_PWM_OUTPUT                  "יציאת PWM"
#define TR_JITTER_FILTER               "מסנן ADC"
#define TR_DEAD_ZONE                   "אזור מת"
#define TR_RTC_CHECK                   TR("בדוק RTC", "בדוק מתח RTC")
#define TR_AUTH_FAILURE                "כשל אימות"
#define TR_RACING_MODE                 "מצב מירוץ"

#define TR_USE_THEME_COLOR              "השתמש בצבע ערכת נושא"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "הוסף את כל הקיזוזים לסאב-טרים"
#define TR_DUPLICATE                    "שיכפול"
#define TR_ACTIVATE                     "הגדר פעיל"
#define TR_COLOR_PICKER                 "בחירת צבע"
#define TR_FIXED                        "קבוע"
#define TR_EDIT_THEME_DETAILS           "עריכת ערכת נושא"
#define TR_THEME_COLOR_PRIMARY1         "ראשי 1"
#define TR_THEME_COLOR_PRIMARY2         "ראשי 2"
#define TR_THEME_COLOR_PRIMARY3         "ראשי 3"
#define TR_THEME_COLOR_SECONDARY1       "משני 1"
#define TR_THEME_COLOR_SECONDARY2       "משני 2"
#define TR_THEME_COLOR_SECONDARY3       "משני 3"
#define TR_THEME_COLOR_FOCUS            "מיקוד"
#define TR_THEME_COLOR_EDIT             "עריכה"
#define TR_THEME_COLOR_ACTIVE           "פעיל"
#define TR_THEME_COLOR_WARNING          "אזהרה"
#define TR_THEME_COLOR_DISABLED         "לא פעיל"
#define TR_THEME_COLOR_QM_BG           "רקע תפריט מהיר"
#define TR_THEME_COLOR_QM_FG           "טקסט תפריט מהיר"
#define TR_THEME_COLOR_CUSTOM           "מתקדם"
#define TR_THEME_CHECKBOX               "תיבת סימון"
#define TR_THEME_ACTIVE                 "פעיל"
#define TR_THEME_REGULAR                "רגיל"
#define TR_THEME_WARNING                "אזהרה"
#define TR_THEME_DISABLED               "לא פעיל"
#define TR_THEME_EDIT                   "עריכה"
#define TR_THEME_FOCUS                  "מיקוד"
#define TR_AUTHOR                       "מחבר"
#define TR_DESCRIPTION                  "תיאור"
#define TR_SAVE                         "שמור"
#define TR_CANCEL                       "ביטול"
#define TR_EDIT_THEME                   "עריכת ערכת נושא"
#define TR_DETAILS                      "פרטים"

// Voice in native language
#define TR_VOICE_ENGLISH                "אנגלית"
#define TR_VOICE_CHINESE                "סינית"
#define TR_VOICE_CZECH                  "צ'כית"
#define TR_VOICE_DANISH                 "דנית"
#define TR_VOICE_DEUTSCH                "גרמנית"
#define TR_VOICE_DUTCH                  "הולנדית"
#define TR_VOICE_ESPANOL                "ספרדית"
#define TR_VOICE_FINNISH                "פינית"
#define TR_VOICE_FRANCAIS               "צרפתית"
#define TR_VOICE_HUNGARIAN              "הונגרית"
#define TR_VOICE_ITALIANO               "איטלקית"
#define TR_VOICE_POLISH                 "פולנית"
#define TR_VOICE_PORTUGUES              "פורטוגזית"
#define TR_VOICE_RUSSIAN                "רוסית"
#define TR_VOICE_SLOVAK                 "סלובקית"
#define TR_VOICE_SWEDISH                "שוודית"
#define TR_VOICE_TAIWANESE              "טייוואנית"
#define TR_VOICE_JAPANESE               "יפנית"
#define TR_VOICE_HEBREW                 "עברית"
#define TR_VOICE_UKRAINIAN              "אוקראינית"
#define TR_VOICE_KOREAN                 "קוריאנית"

#define TR_USBJOYSTICK_LABEL            "חיבור מצב משחק"
#define TR_USBJOYSTICK_EXTMODE          "מצב"
#define TR_VUSBJOYSTICK_EXTMODE_1       "רגיל"
#define TR_VUSBJOYSTICK_EXTMODE_2       "מתקדם"
#define TR_USBJOYSTICK_SETTINGS         BUTTON("הגדרות ערוץ")
#define TR_USBJOYSTICK_IF_MODE          TR("If. mode","Interface mode")
#define TR_VUSBJOYSTICK_IF_MODE_1       "Joystick"
#define TR_VUSBJOYSTICK_IF_MODE_2       "Gamepad"
#define TR_VUSBJOYSTICK_IF_MODE_3       "MultiAxis"
#define TR_USBJOYSTICK_CH_MODE          "Mode"
#define TR_VUSBJOYSTICK_CH_MODE_1       "None"
#define TR_VUSBJOYSTICK_CH_MODE_2       "Btn"
#define TR_VUSBJOYSTICK_CH_MODE_3       "Axis"
#define TR_VUSBJOYSTICK_CH_MODE_4       "Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S_1     "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2     "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3     "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4     "S"
#define TR_USBJOYSTICK_CH_BTNMODE       "Button Mode"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1    "Normal"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2    "Pulse"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3    "SWEmu"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4    "Delta"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5    "Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1  TR("Norm","Normal")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2  TR("Puls","Pulse")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3  TR("SWEm","SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4  TR("Delt","Delta")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5  TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS         "מיקומים"
#define TR_VUSBJOYSTICK_CH_SWPOS_1      "Push"
#define TR_VUSBJOYSTICK_CH_SWPOS_2      "2POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_3      "3POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_4      "4POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_5      "5POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_6      "6POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_7      "7POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_8      "8POS"
#define TR_USBJOYSTICK_CH_AXIS          "צירים"
#define TR_VUSBJOYSTICK_CH_AXIS_1       "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2       "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3       "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4       "rotX"
#define TR_VUSBJOYSTICK_CH_AXIS_5       "rotY"
#define TR_VUSBJOYSTICK_CH_AXIS_6       "rotZ"
#define TR_VUSBJOYSTICK_CH_AXIS_7       "Slider"
#define TR_VUSBJOYSTICK_CH_AXIS_8       "Dial"
#define TR_VUSBJOYSTICK_CH_AXIS_9       "Wheel"
#define TR_USBJOYSTICK_CH_SIM           "ציר סימולטור"
#define TR_VUSBJOYSTICK_CH_SIM_1        "מאזנות"
#define TR_VUSBJOYSTICK_CH_SIM_2        "ה.גובה"
#define TR_VUSBJOYSTICK_CH_SIM_3        "ה.כיוון"
#define TR_VUSBJOYSTICK_CH_SIM_4        "מנוע"
#define TR_VUSBJOYSTICK_CH_SIM_5        "Acc"
#define TR_VUSBJOYSTICK_CH_SIM_6        "Brk"
#define TR_VUSBJOYSTICK_CH_SIM_7        "Steer"
#define TR_VUSBJOYSTICK_CH_SIM_8        "Dpad"
#define TR_USBJOYSTICK_CH_INVERSION     "היפוך"
#define TR_USBJOYSTICK_CH_BTNNUM        "Button no."
#define TR_USBJOYSTICK_BTN_COLLISION    "!Button no. collision!"
#define TR_USBJOYSTICK_AXIS_COLLISION   "!Axis collision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT     TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1  "None"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2  "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3  "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4  "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES    BUTTON("החל שינויים")

#define TR_DIGITAL_SERVO          "333HZ סרוו דיגיטלי"
#define TR_ANALOG_SERVO           "50HZ סרוו אנלוגי"
#define TR_SIGNAL_OUTPUT          "יציאת סיגנל"
#define TR_SERIAL_BUS             "ערוץ טורי"
#define TR_SYNC                   "סינכרון"

#define TR_ENABLED_FEATURES       "אפשר יכולות"
#define TR_RADIO_MENU_TABS        "לשוניות תפריט שלט"
#define TR_MODEL_MENU_TABS        "לשוניות תפריט מודל"

#define TR_SELECT_MENU_ALL        "הכל"
#define TR_SELECT_MENU_CLR        "נקה"
#define TR_SELECT_MENU_INV        "הפוך"

#define TR_SORT_ORDERS_1          "A-Z סידור"
#define TR_SORT_ORDERS_2          "Z-A סידור"
#define TR_SORT_ORDERS_3          "יותר בשימוש"
#define TR_SORT_ORDERS_4          "פחות בשימוש"
#define TR_SORT_MODELS_BY         "סדר תצוגה"
#define TR_CREATE_NEW             "יצירה חדשה"

#define TR_MIX_SLOW_PREC          TR("דיוק האטה", "דיוק האטה עלייה/ירידה")
#define TR_MIX_DELAY_PREC         TR("דיוק השהיה", "דיוק השהיה עלייה/ירידה")

#define TR_THEME_EXISTS           "כבר קיימת ערכת נושא עם אותו שם"

#define TR_DATE_TIME_WIDGET       "תאריך ושעה"
#define TR_RADIO_INFO_WIDGET      "מידע השלט"
#define TR_LOW_BATT_COLOR         "מתח סוללה נמוך"
#define TR_MID_BATT_COLOR         "מתח סוללה בינוני"
#define TR_HIGH_BATT_COLOR        "מתח סוללה גבוה"

#define TR_WIDGET_SIZE            "גודל וידג'ט"

#define TR_DEL_DIR_NOT_EMPTY      "תיקייה חייבת להיות ריקה לפני מחיקה"

#define TR_KEY_SHORTCUTS          "קיצורי מקשים"
#define TR_CURRENT_SCREEN         "מסך נוכחי"
#define TR_SHORT_PRESS            "לחיצה קצרה"
#define TR_LONG_PRESS             "לחיצה ארוכה"
#define TR_OPEN_QUICK_MENU        "פתח תפריט מהיר"
#define TR_QUICK_MENU_FAVORITES   "מועדפים בתפריט מהיר"
