/*
 * Arabic (AR) translation.
 *
 * Common EdgeTX/FPV terms (Inputs, Mixes, Trims, Telemetry, etc.) are
 * intentionally kept in English, as that is what Arabic pilots use.
 * Avoid harakat (diacritic marks): LVGL does not position combining
 * marks, so they render as separate glyphs.
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
#define TR_QM_MANAGE_MODELS             "إدارة\nالموديلات"
#define TR_QM_MODEL_SETUP               "إعداد\nالموديل"
#define TR_QM_RADIO_SETUP               "إعداد\nالريموت"
#define TR_QM_UI_SETUP                  "إعداد\nالواجهة"
#define TR_QM_TOOLS                     "الأدوات"
#define TR_QM_MODEL_SETTINGS            "إعدادات\nالموديل"
#define TR_QM_RADIO_SETTINGS            "إعدادات\nالريموت"
#define TR_QM_FLIGHT_MODES              TR_SFC_AIR("أوضاع\nالقيادة", "Flight\nModes")
#define TR_QM_INPUTS                    "Inputs"
#define TR_QM_MIXES                     "Mixes"
#define TR_QM_OUTPUTS                   "Outputs"
#define TR_QM_CURVES                    "Curves"
#define TR_QM_GLOBAL_VARS               "Global\nVariables"
#define TR_QM_LOGICAL_SW                "Logical\nSwitches"
#define TR_QM_SPEC_FUNC                 "Special\nFunctions"
#define TR_QM_CUSTOM_LUA                "Mixer\nScripts"
#define TR_QM_TELEM                     "Telemetry"
#define TR_QM_GLOB_FUNC                 "Global\nFunctions"
#define TR_QM_TRAINER                   "Trainer"
#define TR_QM_HARDWARE                  "الأجهزة"
#define TR_QM_ABOUT                     "عن\nEdgeTX"
#define TR_QM_THEMES                    "المظهر"
#define TR_QM_TOP_BAR                   "الشريط العلوي"
#define TR_QM_SCREEN_1                  "الشاشة 1"
#define TR_QM_SCREEN_2                  "الشاشة 2"
#define TR_QM_SCREEN_3                  "الشاشة 3"
#define TR_QM_SCREEN_4                  "الشاشة 4"
#define TR_QM_SCREEN_5                  "الشاشة 5"
#define TR_QM_SCREEN_6                  "الشاشة 6"
#define TR_QM_SCREEN_7                  "الشاشة 7"
#define TR_QM_SCREEN_8                  "الشاشة 8"
#define TR_QM_SCREEN_9                  "الشاشة 9"
#define TR_QM_SCREEN_10                 "الشاشة 10"
#define TR_QM_ADD_SCREEN                "إضافة\nشاشة"
#define TR_QM_APPS                      "التطبيقات"
#define TR_QM_STORAGE                   "التخزين"
#define TR_QM_RESET                     TR_SFC_AIR("إعادة ضبط\nالقيادة", "إعادة ضبط\nالطيران")
#define TR_QM_CHAN_MON                  "مراقبة\nالقنوات"
#define TR_QM_LS_MON                    "مراقبة\nLS"
#define TR_QM_STATS                     "الإحصائيات"
#define TR_QM_DEBUG                     "التصحيح"
#define TR_MAIN_MODEL_SETTINGS          "إعدادات الموديل"
#define TR_MAIN_RADIO_SETTINGS          "إعدادات الريموت"
#define TR_MAIN_MENU_MANAGE_MODELS      "إدارة الموديلات"
#define TR_MAIN_MENU_MODEL_NOTES        "ملاحظات الموديل"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "مراقبة القنوات"
#define TR_MONITOR_SWITCHES             "مراقبة Logical Switches"
#define TR_MAIN_MENU_MODEL_SETTINGS     "إعداد الموديل"
#define TR_MAIN_MENU_RADIO_SETTINGS     "إعداد الريموت"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "إعداد الواجهة"
#define TR_MAIN_MENU_STATISTICS         "الإحصائيات"
#define TR_MAIN_MENU_ABOUT_EDGETX       "عن EdgeTX"
#define TR_MAIN_VIEW_X                  "الشاشة "
#define TR_MAIN_MENU_THEMES             "المظهر"
#define TR_MAIN_MENU_APPS               "التطبيقات"
#define TR_MENUHELISETUP                TR_BW_COL("HELI SETUP", "إعدادات الهليكوبتر")
#define TR_MENUFLIGHTMODES              TR_SFC_AIR(TR_BW_COL("DRIVE MODES", "أوضاع القيادة"), TR_BW_COL("FLIGHT MODES", "Flight Modes"))
#define TR_MENUFLIGHTMODE               TR_SFC_AIR("وضع القيادة", "FLIGHT MODE")
#define TR_MENUINPUTS                   TR_BW_COL("INPUTS", "Inputs")
#define TR_MENULIMITS                   TR_BW_COL("OUTPUTS", "Outputs")
#define TR_MENUCURVES                   TR_BW_COL("CURVES", "Curves")
#define TR_MIXES                        TR_BW_COL("MIXES", "Mixes")
#define TR_MENU_GLOBAL_VARS             "Global Variables"
#define TR_MENULOGICALSWITCHES          TR_BW_COL("LOGICAL SWITCHES", "Logical Switches")
#define TR_MENUCUSTOMFUNC               TR_BW_COL("SPECIAL FUNCTIONS", "Special Functions")
#define TR_MENUCUSTOMSCRIPTS            TR_BW_COL("MIXER SCRIPTS", "Mixer Scripts")
#define TR_MENUTELEMETRY                TR_BW_COL("TELEMETRY", "Telemetry")
#define TR_MENUSPECIALFUNCS             TR_BW_COL("GLOBAL FUNCTIONS", "Global Functions")
#define TR_MENUTRAINER                  TR_BW_COL("TRAINER", "Trainer")
#define TR_HARDWARE                     TR_BW_COL("HARDWARE", "الأجهزة")
#define TR_USER_INTERFACE               "الشريط العلوي"
#define TR_SD_CARD                      TR_BW_COL("SD CARD", "التخزين")
#define TR_DEBUG                        "التصحيح"
#define TR_MENU_RADIO_SWITCHES          TR_BW_COL("SWITCHES", "اختبار المفاتيح")
#define TR_MENUCALIBRATION              TR_BW_COL("CALIBRATION", "المعايرة")
#define TR_FUNCTION_SWITCHES            "المفاتيح المخصصة"
// End Main menu

#define TR_MINUTE_SINGULAR            "دقيقة"
#define TR_MINUTE_PLURAL1             "دقائق"
#define TR_MINUTE_PLURAL2             "دقيقة"

#define TR_OFFON_1                     "إيقاف"
#define TR_OFFON_2                     "تشغيل"
#define TR_MMMINV_1                    "---"
#define TR_MMMINV_2                    "عكس"
#define TR_VBEEPMODE_1                 "صامت"
#define TR_VBEEPMODE_2                 "التنبيهات"
#define TR_VBEEPMODE_3                 "بلا أزرار"
#define TR_VBEEPMODE_4                 "الكل"
#define TR_VBLMODE_1                   "إيقاف"
#define TR_VBLMODE_2                   "الأزرار"
#define TR_VBLMODE_3                   TR("العصيان","العصيان")
#define TR_VBLMODE_4                   "الاثنين"
#define TR_VBLMODE_5                   "تشغيل"
#define TR_TRNMODE_1                   "إيقاف"
#define TR_TRNMODE_2                   TR("+=","إضافة")
#define TR_TRNMODE_3                   TR(":=","استبدال")
#define TR_TRNCHN_1                    "CH1"
#define TR_TRNCHN_2                    "CH2"
#define TR_TRNCHN_3                    "CH3"
#define TR_TRNCHN_4                    "CH4"

#define TR_AUX_SERIAL_MODES_1          "إيقاف"
#define TR_AUX_SERIAL_MODES_2          "إرسال Telemetry"
#define TR_AUX_SERIAL_MODES_3          "استقبال Telemetry"
#define TR_AUX_SERIAL_MODES_4          TR("SBUS Trainer معكوس",TR("SBUS Trainer معكوس","SBUS Trainer معكوس"))
#define TR_AUX_SERIAL_MODES_5          "SBUS Trainer"
#define TR_AUX_SERIAL_MODES_6          "LUA"
#define TR_AUX_SERIAL_MODES_7          "CLI"
#define TR_AUX_SERIAL_MODES_8          "GPS"
#define TR_AUX_SERIAL_MODES_9          "التصحيح"
#define TR_AUX_SERIAL_MODES_10         "SpaceMouse"
#define TR_AUX_SERIAL_MODES_11         "موديول خارجي"
#define TR_GLOBAL                      "عام"
#define TR_SWTYPES_1                   "لا شيء"
#define TR_SWTYPES_2                   "ضغطة"
#define TR_SWTYPES_3                   "وضعين"
#define TR_SWTYPES_4                   "3 أوضاع"
#define TR_SWTYPES_5                   TR_GLOBAL
#define TR_POTTYPES_1                  "لا شيء"
#define TR_POTTYPES_2                  "Pot"
#define TR_POTTYPES_3                  TR("Pot مع نقرة","Pot مع نقرة")
#define TR_POTTYPES_4                  "Slider"
#define TR_POTTYPES_5                  TR("مفتاح متعدد","مفتاح متعدد")
#define TR_POTTYPES_6                  "المحور X"
#define TR_POTTYPES_7                  "المحور Y"
#define TR_POTTYPES_8                  "مفتاح"
#define TR_VPERSISTENT_1               "إيقاف"
#define TR_VPERSISTENT_2               "الطيران"
#define TR_VPERSISTENT_3               "ضبط يدوي"
#define TR_COUNTRY_CODES_1             TR("US","أمريكا")
#define TR_COUNTRY_CODES_2             TR("JP","اليابان")
#define TR_COUNTRY_CODES_3             TR("EU","أوروبا")
#define TR_USBMODES_1                  "اسأل"
#define TR_USBMODES_2                  TR("Joystick","Joystick")
#define TR_USBMODES_3                  TR("التخزين","التخزين")
#define TR_USBMODES_4                  "Serial"
#define TR_MODEL_USBMODES_1            TR_GLOBAL
#define TR_MODEL_USBMODES_2            TR_USBMODES_1
#define TR_MODEL_USBMODES_3            TR_USBMODES_2
#define TR_MODEL_USBMODES_4            TR_USBMODES_3
#define TR_MODEL_USBMODES_5            TR_USBMODES_4
#define TR_JACK_MODES_1                "اسأل"
#define TR_JACK_MODES_2                "الصوت"
#define TR_JACK_MODES_3                "Trainer"

#define TR_SBUS_INVERSION_VALUES_1     "عادي"
#define TR_SBUS_INVERSION_VALUES_2     "غير معكوس"
#define TR_MULTI_CUSTOM                "مخصص"
#define TR_VTRIMINC_1                  TR("Expo","Expo")
#define TR_VTRIMINC_2                  TR("دقيق جدا","دقيق جدا")
#define TR_VTRIMINC_3                  "دقيق"
#define TR_VTRIMINC_4                  "متوسط"
#define TR_VTRIMINC_5                  "خشن"
#define TR_VDISPLAYTRIMS_1             "لا"
#define TR_VDISPLAYTRIMS_2             "التغيير"
#define TR_VDISPLAYTRIMS_3             "نعم"
#define TR_VBEEPCOUNTDOWN_1            "صامت"
#define TR_VBEEPCOUNTDOWN_2            "صفارات"
#define TR_VBEEPCOUNTDOWN_3            "صوت"
#define TR_VBEEPCOUNTDOWN_4            "اهتزاز"
#define TR_VBEEPCOUNTDOWN_5            TR("صفارات واهتزاز","صفارات واهتزاز")
#define TR_VBEEPCOUNTDOWN_6            TR("صوت واهتزاز","صوت واهتزاز")
#define TR_COUNTDOWNVALUES_1           "5 ث"
#define TR_COUNTDOWNVALUES_2           "10 ث"
#define TR_COUNTDOWNVALUES_3           "20 ث"
#define TR_COUNTDOWNVALUES_4           "30 ث"
#define TR_VVARIOCENTER_1              "نغمة"
#define TR_VVARIOCENTER_2              "صامت"
#define TR_CURVE_TYPES_1               "قياسي"
#define TR_CURVE_TYPES_2               "مخصص"

#define TR_ADCFILTERVALUES_1           TR_GLOBAL
#define TR_ADCFILTERVALUES_2           "إيقاف"
#define TR_ADCFILTERVALUES_3           "تشغيل"

#define TR_VCURVETYPE_1                "Diff"
#define TR_VCURVETYPE_2                "Expo"
#define TR_VCURVETYPE_3                "دالة"
#define TR_VCURVETYPE_4                "مخصص"
#define TR_VMLTPX_1                    "إضافة"
#define TR_VMLTPX_2                    "ضرب"
#define TR_VMLTPX_3                    "استبدال"

#define TR_CSWTIMER                    TR("مؤقت", "مؤقت")
#define TR_CSWSTICKY                   TR("ثابت", "ثابت")
#define TR_CSWSTAY                     "Edge"

#define TR_SF_TRAINER                  "Trainer"
#define TR_SF_INST_TRIM                "Trim فوري"
#define TR_SF_RESET                    "إعادة ضبط"
#define TR_SF_SET_TIMER                "ضبط"
#define TR_SF_VOLUME                   "الصوت"
#define TR_SF_FAILSAFE                 "ضبط Failsafe"
#define TR_SF_RANGE_CHECK              "فحص المدى"
#define TR_SF_MOD_BIND                 "ربط الموديول"
#define TR_SF_RGBLEDS                  "إضاءة RGB"

#define TR_SOUND                       "تشغيل صوت"
#define TR_PLAY_TRACK                  TR("تشغيل ملف", "تشغيل ملف")
#define TR_PLAY_VALUE                  TR("نطق قيمة","نطق قيمة")
#define TR_SF_HAPTIC                   "اهتزاز"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "سكربت Lua")
#define TR_SF_BG_MUSIC                 "موسيقى"
#define TR_SF_BG_MUSIC_PAUSE           "موسيقى ||"
#define TR_SF_LOGS                     "سجلات SD"
#define TR_ADJUST_GVAR                 "تعديل"
#define TR_SF_BACKLIGHT                "الإضاءة"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "اختبار"

#define TR_SF_SAFETY                   TR("تجاوز", "تجاوز")

#define TR_SF_SCREENSHOT               "لقطة شاشة"
#define TR_SF_RACING_MODE              "وضع السباق"
#define TR_SF_DISABLE_TOUCH            "بلا لمس"
#define TR_SF_DISABLE_KEYS             "بلا أزرار"
#define TR_SF_DISABLE_AUDIO_AMP        "إيقاف المكبر"
#define TR_SF_SET_SCREEN               TR_BW_COL("Set Screen", "تعيين الشاشة")
#define TR_SF_PUSH_CUST_SWITCH         "ضغط مفتاح"
#define TR_SF_LCD_TO_VIDEO             "الشاشة للفيديو"

#define TR_FSW_RESET_TELEM             TR("Telm", "Telemetry")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS_1          "مؤقت1"
#define TR_FSW_RESET_TIMERS_2          "مؤقت2"
#define TR_FSW_RESET_TIMERS_3          "مؤقت3"

#define TR_VFSWRESET_1                 TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                 TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                 TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                 TR("الكل","الطيران")
#define TR_VFSWRESET_5                 TR_FSW_RESET_TELEM
#define TR_VFSWRESET_6                 TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS_1                TR("صفارة1","صفارة1")
#define TR_FUNCSOUNDS_2                TR("صفارة2","صفارة2")
#define TR_FUNCSOUNDS_3                TR("صفارة3","صفارة3")
#define TR_FUNCSOUNDS_4                TR("تحذير1","تحذير1")
#define TR_FUNCSOUNDS_5                TR("تحذير2","تحذير2")
#define TR_FUNCSOUNDS_6                TR("زقزقة","زقزقة")
#define TR_FUNCSOUNDS_7                TR("Rata","Ratata")
#define TR_FUNCSOUNDS_8                "تك"
#define TR_FUNCSOUNDS_9                TR("إنذار","إنذار")
#define TR_FUNCSOUNDS_10               "رنين"
#define TR_FUNCSOUNDS_11               TR("SciF","SciFi")
#define TR_FUNCSOUNDS_12               TR("روبوت","روبوت")
#define TR_FUNCSOUNDS_13               TR("تغريد","تغريد")
#define TR_FUNCSOUNDS_14               "تادا"
#define TR_FUNCSOUNDS_15               TR("صرصور","صرصور")
#define TR_FUNCSOUNDS_16               TR("منبه","منبه")

#define TR_VUNITSSYSTEM_1              "متري"
#define TR_VUNITSSYSTEM_2              TR("إمبراطوري","إمبراطوري")
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
#define TR_VTELEMUNIT_31               ""
#define TR_VTELEMUNIT_32               ""
#define TR_VTELEMUNIT_33               ""
#define TR_VTELEMUNIT_34               ""
#define TR_VTELEMUNIT_35               ""
#define TR_VTELEMUNIT_36               "h"
#define TR_VTELEMUNIT_37               "min"
#define TR_VTELEMUNIT_38               "s"

#define TR_VTELEMSCREENTYPE_1          "لا شيء"
#define TR_VTELEMSCREENTYPE_2          "أرقام"
#define TR_VTELEMSCREENTYPE_3          "أشرطة"
#define TR_VTELEMSCREENTYPE_4          "سكربت"
#define TR_GPSFORMAT_1                 "DMS"
#define TR_GPSFORMAT_2                 "NMEA"


#define TR_VSWASHTYPE_1                "---"
#define TR_VSWASHTYPE_2                "120"
#define TR_VSWASHTYPE_3                "120X"
#define TR_VSWASHTYPE_4                "140"
#define TR_VSWASHTYPE_5                "90"

#define TR_STICK_NAMES0                "Rud"
#define TR_STICK_NAMES1                "Ele"
#define TR_STICK_NAMES2                "Thr"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#define TR_ON_ONE_SWITCHES_1           "تشغيل"
#define TR_ON_ONE_SWITCHES_2           "مرة"

#define TR_HATSMODE                    "وضع Hats"
#define TR_HATSOPT_1                   "Trims فقط"
#define TR_HATSOPT_2                   "أزرار فقط"
#define TR_HATSOPT_3                   "بالتبديل"
#define TR_HATSOPT_4                   TR_GLOBAL
#define TR_HATSMODE_TRIMS              "وضع Hats: Trims"
#define TR_HATSMODE_KEYS               "وضع Hats: أزرار"
#define TR_HATSMODE_KEYS_HELP          "الجهة اليسرى:\n"\
                                       "   يمين = MDL\n"\
                                       "   فوق = SYS\n"\
                                       "   تحت = TELE\n"\
                                       "\n"\
                                       "الجهة اليمنى:\n"\
                                       "   يسار = PAGE<\n"\
                                       "   يمين = PAGE>\n"\
                                       "   فوق = PREV/INC\n"\
                                       "   تحت = NEXT/DEC"

#define TR_ROTARY_ENC_OPT_1       "عادي"
#define TR_ROTARY_ENC_OPT_2       "معكوس"
#define TR_ROTARY_ENC_OPT_3       "V-I H-N"
#define TR_ROTARY_ENC_OPT_4       "V-I H-A"
#define TR_ROTARY_ENC_OPT_5       "V-N E-I"

#define TR_IMU_VSRCRAW_1             "TltX"
#define TR_IMU_VSRCRAW_2             "TltY"
#define TR_IMU_VSRCRAW_3             "Accl"

#define TR_CYC_VSRCRAW_1             "CYC1"
#define TR_CYC_VSRCRAW_2             "CYC2"
#define TR_CYC_VSRCRAW_3             "CYC3"

#define TR_SRC_BATT                    "Batt"
#define TR_SRC_BAT_CURRENT             "BatI"
#define TR_SRC_BAT_POWER               "BatP"
#define TR_SRC_TIME                    "الوقت"
#define TR_SRC_GPS                     "GPS"
#define	TR_SRC_LIGHT                    "ضوء محيط"
#define TR_SRC_TIMER                   "مؤقت"

#define TR_VTMRMODES_1                 "إيقاف"
#define TR_VTMRMODES_2                 "تشغيل"
#define TR_VTMRMODES_3                 "بدء"
#define TR_VTMRMODES_4                 "THs"
#define TR_VTMRMODES_5                 "TH%"
#define TR_VTMRMODES_6                 "THt"
#define TR_VTRAINER_MASTER_OFF         "إيقاف"
#define TR_VTRAINER_MASTER_JACK        "Master/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Slave/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Master/SBUS Module"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Master/CPPM Module"
#define TR_VTRAINER_MASTER_BATTERY     "Master/Serial"
#define TR_VTRAINER_BLUETOOTH_1        "Master/" TR("BT","Bluetooth")
#define TR_VTRAINER_BLUETOOTH_2        "Slave/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VTRAINER_CRSF               "Master/CRSF"
#define TR_VFAILSAFE_1                 "غير مضبوط"
#define TR_VFAILSAFE_2                 "تثبيت"
#define TR_VFAILSAFE_3                 "مخصص"
#define TR_VFAILSAFE_4                 "بلا إشارة"
#define TR_VFAILSAFE_5                 "الرسيفر"
#define TR_VSENSORTYPES_1              "مخصص"
#define TR_VSENSORTYPES_2              "محسوب"
#define TR_VFORMULAS_1                 "جمع"
#define TR_VFORMULAS_2                 "متوسط"
#define TR_VFORMULAS_3                 "أدنى"
#define TR_VFORMULAS_4                 "أعلى"
#define TR_VFORMULAS_5                 "ضرب"
#define TR_VFORMULAS_6                 "تراكمي"
#define TR_VFORMULAS_7                 "خلية"
#define TR_VFORMULAS_8                 "استهلاك"
#define TR_VFORMULAS_9                 "مسافة"
#define TR_VPREC_1                     "0.--"
#define TR_VPREC_2                     "0.0 "
#define TR_VPREC_3                     "0.00"
#define TR_VCELLINDEX_1                "الأدنى"
#define TR_VCELLINDEX_2                "1"
#define TR_VCELLINDEX_3                "2"
#define TR_VCELLINDEX_4                "3"
#define TR_VCELLINDEX_5                "4"
#define TR_VCELLINDEX_6                "5"
#define TR_VCELLINDEX_7                "6"
#define TR_VCELLINDEX_8                "7"
#define TR_VCELLINDEX_9                "8"
#define TR_VCELLINDEX_10               "الأعلى"
#define TR_VCELLINDEX_11               "الفرق"
#define TR_SUBTRIMMODES_1              CHAR_DELTA " (الوسط فقط)"
#define TR_SUBTRIMMODES_2              "= (متماثل)"
#define TR_TIMER_DIR_1                 TR("المتبقي", "المتبقي")
#define TR_TIMER_DIR_2                 TR("المنقضي", "المنقضي")

#define TR_FONT_SIZES_1                "STD"
#define TR_FONT_SIZES_2                "BOLD"
#define TR_FONT_SIZES_3                "XXS"
#define TR_FONT_SIZES_4                "XS"
#define TR_FONT_SIZES_5                "L"
#define TR_FONT_SIZES_6                "XL"
#define TR_FONT_SIZES_7                "XXL"
#define TR_FONT_SIZES_8                "LXL"

#define TR_ENTER                       "[ENTER]"
#define TR_OK                          TR_BW_COL(TR("\010\010\010[OK]", "\010\010\010\010\010[OK]"), "موافق")
#define TR_EXIT                        TR_BW_COL("EXIT", "RTN")

#define TR_YES                         "نعم"
#define TR_NO                          "لا"
#define TR_DELETEMODEL                 "حذف الموديل"
#define TR_COPYINGMODEL                "نسخ الموديل..."
#define TR_MOVINGMODEL                 "نقل الموديل..."
#define TR_LOADINGMODEL                "تحميل الموديل..."
#define TR_UNLABELEDMODEL              "بدون تصنيف"
#define TR_NAME                        "الاسم"
#define TR_MODELNAME                   "اسم الموديل"
#define TR_PHASENAME                   "اسم الوضع"
#define TR_MIXNAME                     "اسم Mix"
#define TR_INPUTNAME                   TR("Input", "اسم Input")
#define TR_EXPONAME                    TR("الاسم", "اسم السطر")
#define TR_BITMAP                      "صورة الموديل"
#define TR_NO_PICTURE                  "بلا صورة"
#define TR_TIMER                       TR("مؤقت", "مؤقت ")
#define TR_NO_TIMERS                   "بلا مؤقتات"
#define TR_START                       "بدء"
#define TR_NEXT                        "التالي"
#define TR_ELIMITS                     TR("حدود موسعة", "حدود موسعة")
#define TR_ETRIMS                      TR("Trims موسعة", "Trims موسعة")
#define TR_TRIMINC                     "خطوة Trim"
#define TR_DISPLAY_TRIMS               TR("عرض Trims", "عرض Trims")
#define TR_TTRACE                      TR("المصدر", "المصدر")
#define TR_TTRIM                       TR("Trim عند Idle", "Trim عند Idle")
#define TR_TTRIM_SW                    TR("مفتاح Trim", "مفتاح Trim")
#define TR_BEEPCTR                     TR("صفارة بالوسط", "صفارة بالوسط")
#define TR_PROTOCOL                    TR("البروتوكول", "البروتوكول")
  #define TR_PPMFRAME                  "إطار PPM"
  #define TR_REFRESHRATE               TR("التحديث", "معدل التحديث")
  #define TR_WARN_BATTVOLTAGE         TR("الخرج VBAT: ", "تحذير: الخرج VBAT: ")
#define TR_WARN_5VOLTS                 "تحذير: الخرج 5 فولت"
#define TR_MS                          "ms"
#define TR_SWITCH                      "المفتاح"
#define TR_FS_COLOR_LIST_1             "مخصص"
#define TR_FS_COLOR_LIST_2             "إيقاف"
#define TR_FS_COLOR_LIST_3             "أبيض"
#define TR_FS_COLOR_LIST_4             "أحمر"
#define TR_FS_COLOR_LIST_5             "أخضر"
#define TR_FS_COLOR_LIST_6             "أصفر"
#define TR_FS_COLOR_LIST_7             "برتقالي"
#define TR_FS_COLOR_LIST_8             "أزرق"
#define TR_FS_COLOR_LIST_9             "وردي"
#define TR_GROUP                       "المجموعة"
#define TR_GROUP_ALWAYS_ON             "تشغيل دائم"
#define TR_LUA_OVERRIDE                "تحكم Lua"
#define TR_LAST                        "الأخير"
#define TR_MORE_INFO                   "معلومات أكثر"
#define TR_SWITCH_TYPE                 "النوع"
#define TR_SWITCH_STARTUP              "عند التشغيل"
#define TR_SWITCH_GROUP                "المجموعة"
#define TR_SF_SWITCH                   "المحفز"
#define TR_TRIMS                       "Trims"
#define TR_FADEIN                      "دخول تدريجي"
#define TR_FADEOUT                     "خروج تدريجي"
#define TR_CHECKTRIMS                  TR("\006Check\012trims", "فحص Trims الأوضاع")
#define TR_SWASHTYPE                   "نوع Swash"
#define TR_COLLECTIVE                  TR("Collective", "Coll. pitch source")
#define TR_AILERON                     TR("Lateral cyc.", "Lateral cyc. source")
#define TR_ELEVATOR                    TR("Long. cyc.", "Long. cyc. source")
#define TR_SWASHRING                   "Swash Ring"
#define TR_MODE                        "الوضع"
#define TR_LEFT_STICK                  "يسار"
#define TR_SUBTYPE                     "النوع الفرعي"
#define TR_NOFREEEXPO                  "لا مساحة لـ Expo!"
#define TR_NOFREEMIXER                 "لا مساحة لـ Mix!"
#define TR_SOURCE                       "المصدر"
#define TR_WEIGHT                      "الوزن"
#define TR_SIDE                        "الجهة"
#define TR_OFFSET                       "الإزاحة"
#define TR_TRIM                        "Trim"
#define TR_CURVE                       "Curve"
#define TR_FLMODE                      TR("الوضع", "الأوضاع")
#define TR_MIXWARNING                  "تحذير"
#define TR_OFF                         "إيقاف"
#define TR_ANTENNA                     "الهوائي"
#define TR_NO_INFORMATION              TR("لا معلومات", "لا معلومات")
#define TR_MULTPX                      "نوع الدمج"
#define TR_DELAYDOWN                   TR("تأخير نزول", "تأخير النزول")
#define TR_DELAYUP                     "تأخير صعود"
#define TR_SLOWDOWN                    TR("إبطاء نزول", "إبطاء النزول")
#define TR_SLOWUP                      "إبطاء صعود"
#define TR_CV                          "CV"
#define TR_GV                          TR("G", "GV")
#define TR_RANGE                       "المدى"
#define TR_CENTER                      "الوسط"
#define TR_ALARM                       "تنبيه"
#define TR_BLADES                      "الريش/الأقطاب"
#define TR_SCREEN                      "الشاشة\001"
#define TR_SOUND_LABEL                 "الصوت"
#define TR_LENGTH                      "المدة"
#define TR_BEEP_LENGTH                 "مدة الصفارة"
#define TR_BEEP_PITCH                  "حدة الصفارة"
#define TR_HAPTIC_LABEL                "الاهتزاز"
#define TR_STRENGTH                    "القوة"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "الإزاحة"
#define TR_IMU_MAX                     "الأقصى"
#define TR_CONTRAST                    "التباين"
#define TR_ALARMS_LABEL                "التنبيهات"
#define TR_BATTERY_RANGE               TR("مدى البطارية", "مدى مؤشر البطارية")
#define TR_BATTERYCHARGING             "جاري الشحن..."
#define TR_BATTERYFULL                 "البطارية ممتلئة"
#define TR_CHARGE_DETECTING            "جاري الفحص..."
#define TR_CHARGE_TOPPING_OFF          "تكملة الشحن..."
#define TR_CHARGE_NOT_CHARGING         "لا يشحن"
#define TR_PRESS_RTN_TO_CLOSE          "اضغط RTN للإغلاق"
#define TR_PRESS_RTN_TO_POWER_OFF      "اضغط RTN للإطفاء"
#define TR_BATTERYNONE                 "لا شيء!"
#define TR_BATTERYWARNING              "البطارية ضعيفة"
#define TR_INACTIVITYALARM             "عدم النشاط"
#define TR_MEMORYWARNING               "الذاكرة قليلة"
#define TR_ALARMWARNING                "الصوت مغلق"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI عند الإطفاء", "فحص RSSI عند الإطفاء")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Trainer عند الإطفاء", "فحص Trainer عند الإطفاء")
#define TR_MODEL_STILL_POWERED         "الموديل ما زال يعمل"
#define TR_TRAINER_STILL_CONNECTED     TR("Trainer ما زال يعمل","Trainer ما زال متصلا")
#define TR_TRAINER_JACK_MODE           "سلك Trainer"
#define TR_TRAINER_JACK_MODES_1        "متقاطع"
#define TR_TRAINER_JACK_MODES_2        "قياسي"
#define TR_USB_STILL_CONNECTED         "USB ما زال متصلا"
#define TR_MODEL_SHUTDOWN              "إطفاء؟"
#define TR_PRESS_ENTER_TO_CONFIRM      "اضغط Enter للتأكيد"
#define TR_THROTTLE_LABEL              "Throttle"
#define TR_THROTTLE_START              "بداية Throttle"
#define TR_THROTTLEREVERSE             TR("عكس Throttle", "عكس")
#define TR_MINUTEBEEP                  TR("دقيقة", "نداء كل دقيقة")
#define TR_BEEPCOUNTDOWN               "العد التنازلي"
#define TR_PERSISTENT                  TR("دائم", "دائم")
#define TR_BACKLIGHT_LABEL             "الإضاءة"
#define TR_STATUS_LED                  "ضوء الحالة"
#define TR_STATUS_LED_ERROR            "خطأ"
#define TR_STATUS_LED_READY            "جاهز"
#define TR_STATUS_LED_EMIT             "يرسل"
#define TR_STATUS_LED_COLORS_1         "أحمر"
#define TR_STATUS_LED_COLORS_2         "أخضر"
#define TR_STATUS_LED_COLORS_3         "أزرق"
#define TR_STATUS                      "الحالة"
#define TR_BLONBRIGHTNESS              "السطوع عند التشغيل"
#define TR_BLOFFBRIGHTNESS             "السطوع عند الإطفاء"
#define TR_KEYS_BACKLIGHT              "إضاءة الأزرار"
#define TR_BLCOLOR                     "اللون"
#define TR_ONE_LOG_PER_DAY             "سجل واحد باليوم"
#define TR_KEY_LOCK_FMT                "قفل الأزرار (%s+%s مطول)"
#define TR_KEYS_LOCKED                 "الأزرار مقفلة"
#define TR_KEYS_LOCKED_FMT             TR_BW_COL("%s+%s to unlock", "الأزرار مقفلة (%s+%s للفتح)")
#define TR_KEYS_UNLOCKED               "الأزرار مفتوحة"
#define TR_TOUCH_ENABLED               "اللمس مفعل"
#define TR_TOUCH_DISABLED              "اللمس معطل"
#define TR_SPLASHSCREEN                "شاشة البداية"
#define TR_PLAY_HELLO                  "صوت التشغيل"
#define TR_PWR_ON_DELAY                "تأخير التشغيل"
#define TR_PWR_OFF_DELAY               "تأخير الإطفاء"
#define TR_PWR_AUTO_OFF                TR("إطفاء تلقائي","إطفاء تلقائي")
#define TR_PWR_ON_OFF_HAPTIC           TR("اهتزاز التشغيل","اهتزاز التشغيل/الإطفاء")
#define TR_THROTTLE_WARNING            TR("تحذير Throttle", "حالة Throttle")
#define TR_CUSTOM_THROTTLE_WARNING     TR("موضع مخصص", "موضع مخصص؟")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("الموضع %", "الموضع %")
#define TR_SWITCHWARNING               TR("تحذير المفاتيح", "وضعية المفاتيح")
#define TR_POTWARNINGSTATE             "Pots و Sliders"
#define TR_POTWARNING                  TR("تحذير Pots", "وضعية Pots")
#define TR_TIMEZONE                    "المنطقة الزمنية"
#define TR_ADJUST_RTC                  "ضبط الساعة"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("ترتيب القنوات", "ترتيب القنوات الافتراضي")
#define TR_STICKS                      "المحاور"
#define TR_POTS                        "Pots"
#define TR_SWITCHES                    "المفاتيح"
#define TR_SWITCHES_DELAY              TR("تأخير النطق", "تأخير النطق (الوضع الأوسط)")
#define TR_SLAVE                       "Slave"
#define TR_MULTIPLIER                  "المضاعف"
#define TR_CAL                         "معايرة"
#define TR_CALIBRATION                 BUTTON("المعايرة")
#define TR_VTRIM                       "Trim - +"
#define TR_CALIB_DONE                  "اكتملت المعايرة"
#define TR_MENUTOSTART                 TR_ENTER " للبدء"
#define TR_MENUWHENDONE                TR_ENTER " عند الانتهاء"
#define TR_AXISDIR                     "اتجاه المحور"
#define TR_MENUAXISDIR                 "[ENTER LONG] " TR_AXISDIR
#define TR_SETMIDPOINT                 TR_BW_COL(TR_SFC_AIR("SET POTS MIDPOINT", TR("SET AXIS MIDPOINT", "CENTER AXIS/SLIDERS")), "ضع العصيان و Sliders بالوسط")
#define TR_MOVESTICKSPOTS              TR_BW_COL(TR_SFC_AIR("MOVE ST/TH/POTS/AXIS", "MOVE AXIS/POTS"), "حرك العصيان و Pots")
#define TR_NODATA                      "لا بيانات"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Stack المتاح"
#define TR_INT_GPS_LABEL               "GPS داخلي"
#define TR_HEARTBEAT_LABEL             "نبض"
#define TR_LUA_SCRIPTS_LABEL           "سكربتات Lua"
#define TR_FREE_MEM_LABEL              "الذاكرة المتاحة"
#define TR_DURATION_MS             TR("[D]","المدة(ms): ")
#define TR_INTERVAL_MS             TR("[I]","الفاصل(ms): ")
#define TR_MEM_USED_SCRIPT         "Script(B): "
#define TR_MEM_USED_WIDGET         "Widget(B): "
#define TR_MEM_USED_EXTRA          "إضافي(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "الصوت: "
#define TR_GPS_FIX_YES                 "Fix: نعم"
#define TR_GPS_FIX_NO                  "Fix: لا"
#define TR_GPS_SATS                    "أقمار: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "القائمة: "
#define TR_TIMER_LABEL                 "مؤقت"
#define TR_THROTTLE_PERCENT_LABEL      "Throttle %"
#define TR_BATT_LABEL                  "البطارية"
#define TR_SESSION                     "الجلسة"
#define TR_MENUTORESET                 TR_ENTER " لإعادة الضبط"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "الموديل"
#define TR_FM                          TR_SFC_AIR("DM", "FM")
#define TR_PRESS_ANY_KEY_TO_SKIP       "اضغط أي زر للتخطي"
#define TR_THROTTLE_NOT_IDLE           "Throttle مرفوع"
#define TR_ALARMSDISABLED              "التنبيهات معطلة"
#define TR_PRESSANYKEY                 TR("\010اضغط أي زر", "اضغط أي زر")
#define TR_BAD_RADIO_DATA              "بيانات الريموت ناقصة أو تالفة"
#define TR_RADIO_DATA_RECOVERED        TR3("استخدام النسخة الاحتياطية","استخدام النسخة الاحتياطية","تم استرجاع الإعدادات من النسخة الاحتياطية")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("إعدادات الريموت غير صالحة","إعدادات الريموت غير صالحة", "تعذرت قراءة إعدادات الريموت")
#define TR_STORAGE_FORMAT              "تجهيز التخزين"
#define TR_RADIO_SETUP                 "إعداد الريموت"
#define TR_MENUVERSION                 "الإصدار"
#define TR_MENU_RADIO_ANALOGS_CALIB    "القيم بعد المعايرة"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "القيم الخام (5 Hz)"
#define TR_MENU_FSWITCH                "المفاتيح المخصصة"
#define   TR_TRIMS2OFFSETS              TR_BW_COL("\006Trims => Subtrims", "Trims => Subtrims")
#define TR_CHANNELS2FAILSAFE           "القنوات=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "القناة=>Failsafe"
#define TR_MENUMODELSEL                TR("الموديلات", "اختيار الموديل")
#define TR_MENU_MODEL_SETUP            TR("الإعداد", "إعداد الموديل")

#define TR_MENULOGICALSWITCH           "LOGICAL SWITCH"
#define TR_MENUSTAT                    "الإحصائيات"
#define TR_MENUDEBUG                   "التصحيح"
#define TR_MONITOR_CHANNELS            "القنوات %d-%d"
#define TR_MONITOR_OUTPUT_DESC         "Outputs"
#define TR_MONITOR_MIXER_DESC          "Mixers"
  #define TR_RECEIVER_NUM              TR("رقم Rx", "رقم الرسيفر")
  #define TR_RECEIVER                  "الرسيفر"
#define TR_MULTI_RFTUNE                TR("ضبط التردد", "ضبط التردد الدقيق")
#define TR_MULTI_RFPOWER               "قوة RF"
#define TR_MULTI_WBUS                  "الخرج"
#define TR_MULTI_TELEMETRY             "Telemetry"
#define TR_MULTI_VIDFREQ               TR("تردد الفيديو", "تردد الفيديو")
#define TR_RF_POWER                    "قوة RF"
#define TR_MULTI_FIXEDID               TR("ID ثابت", "ID ثابت")
#define TR_MULTI_OPTION                TR("الخيار", "قيمة الخيار")
#define TR_MULTI_AUTOBIND              TR("ربط بقناة", "الربط عن طريق قناة")
#define TR_DISABLE_CH_MAP              TR("بلا ترتيب قنوات", "تعطيل ترتيب القنوات")
#define TR_DSMP_ENABLE_AETR            TR("تفعيل AETR", "تفعيل AETR")
#define TR_DISABLE_TELEM               TR("بلا Telemetry", "تعطيل Telemetry")
#define TR_MULTI_LOWPOWER              TR("طاقة منخفضة", "وضع الطاقة المنخفضة")
#define TR_MULTI_LNA_DISABLE           "تعطيل LNA"
#define TR_MODULE_TELEMETRY            TR("S.Port", "رابط S.Port")
#define TR_MODULE_TELEM_ON             TR("تشغيل", "مفعل")
#define TR_DISABLE_INTERNAL            TR("تعطيل الداخلي", "تعطيل RF الداخلي")
#define TR_MODULE_NO_SERIAL_MODE       TR("ليس Serial", "ليس بوضع Serial")
#define TR_MODULE_NO_INPUT             TR("لا إدخال", "لا يوجد إدخال Serial")
#define TR_MODULE_NO_TELEMETRY         TR3("لا Telemetry", "لا MULTI_TELEMETRY", "لم يتم رصد MULTI_TELEMETRY")
#define TR_MODULE_WAITFORBIND          "اربط لتحميل البروتوكول"
#define TR_MODULE_BINDING              TR("ربط...","جاري الربط")
#define TR_MODULE_UPGRADE_ALERT        TR3("يحتاج تحديث", "الموديول يحتاج تحديث", "الموديول\nيحتاج تحديث")
#define TR_MODULE_UPGRADE              TR("ينصح بالتحديث", "ينصح بتحديث الموديول")
#define TR_REBIND                      "يحتاج ربط من جديد"
#define TR_REG_OK                      "تم التسجيل"
#define TR_BIND_OK                     "تم الربط بنجاح"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem تشغيل"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem إيقاف"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem تشغيل"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem إيقاف"
#define TR_PROTOCOL_INVALID            TR("بروتوكول خطأ", "البروتوكول غير صالح")
#define TR_MODULE_STATUS               TR("الحالة", "حالة الموديول")
#define TR_MODULE_SYNC                 TR("المزامنة", "حالة مزامنة البروتوكول")
#define TR_MULTI_SERVOFREQ             TR("معدل السيرفو", "معدل تحديث السيرفو")
#define TR_MULTI_MAX_THROW             TR("أقصى حركة", "تفعيل أقصى حركة")
#define TR_MULTI_RFCHAN                TR("قناة RF", "اختيار قناة RF")
#define TR_AFHDS3_RX_FREQ              TR("تردد RX", "تردد RX")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("القوة الفعلية", "القوة الفعلية")
#define TR_AFHDS3_POWER_SOURCE         TR("مصدر الطاقة", "مصدر الطاقة")
#define TR_IBUS2_SENSORS_MODE_ONLY     "ضبط الحساسات متاح بوضع iBUS2 فقط."
#define TR_GPS_COORDS_FORMAT           TR("إحداثيات GPS", "صيغة الإحداثيات")
#define TR_VARIO                       TR("Vario", "Variometer")
#define TR_PITCH_AT_ZERO               "النغمة عند الصفر"
#define TR_PITCH_AT_MAX                "النغمة عند الأقصى"
#define TR_REPEAT_AT_ZERO              "التكرار عند الصفر"
#define TR_BATT_CALIB                  TR("معايرة البطارية", "معايرة البطارية")
#define TR_VOLTAGE                     TR("الجهد", "مصدر الجهد")
#define TR_MODULE_SENSOR_VOLTAGE       "الجهد"
#define TR_MODULE_SENSOR_CURRENT       "التيار"
#define TR_SELECT_MODEL                "اختيار الموديل"
#define TR_SELECT_MODE                 "اختيار الوضع"
#define TR_CREATE_MODEL                "إنشاء موديل"
#define TR_FAVORITE_LABEL              "المفضلة"
#define TR_MODELS_MOVED                "الموديلات غير المستخدمة نقلت إلى"
#define TR_NEW_MODEL                   "موديل جديد"
#define TR_LABEL_MODEL                 "تصنيف الموديل"
#define TR_MOVE_UP                     "تحريك لأعلى"
#define TR_MOVE_DOWN                   "تحريك لأسفل"
#define TR_ENTER_LABEL                 "أدخل التصنيف"
#define TR_LABELS                      "التصنيفات"
#define TR_ACTIVE                      "نشط"
#define TR_NEW                         "جديد"
#define TR_NEW_LABEL                   "تصنيف جديد"
#define TR_RENAME_LABEL                "إعادة تسمية التصنيف"
#define TR_DELETE_LABEL                "حذف التصنيف"
#define TR_DUPLICATE_MODEL             "تكرار الموديل"
#define TR_COPY_MODEL                  "نسخ الموديل"
#define TR_MOVE_MODEL                  "نقل الموديل"
#define TR_BACKUP_MODEL                "نسخة احتياطية"
#define TR_DELETE_MODEL                "حذف الموديل"
#define TR_RESTORE_MODEL               "استرجاع الموديل"
#define TR_DELETE_INPUT_LINE           "حذف سطر Input"
#define TR_DELETE_MIX_LINE             "حذف سطر Mix"
#define TR_SDCARD_ERROR                TR("خطأ SD", "خطأ بكرت SD")
#define TR_SDCARD                      "كرت SD"
#define TR_NO_FILES_ON_SD              "لا ملفات على SD!"
#define TR_NO_SDCARD                   "لا يوجد كرت SD"
#define TR_WAITING_FOR_RX              "بانتظار RX..."
#define TR_WAITING_FOR_TX              "بانتظار TX..."
#define TR_WAITING_FOR_MODULE          TR("بانتظار الموديول", "بانتظار الموديول...")
#define TR_NO_TOOLS                    "لا توجد أدوات"
#define TR_NORMAL                      "عادي"
#define TR_NOT_INVERTED                "غير معكوس"
#define TR_NOT_CONNECTED               TR("غير متصل", "غير متصل")
#define TR_CONNECTED                   "متصل"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH بلا Telem", "16CH بدون Telemetry")
#define TR_16CH_WITH_TELEMETRY         TR("16CH مع Telem", "16CH مع Telemetry")
#define TR_EXT_ANTENNA                 "هوائي خارجي"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "تحديث خيارات RX؟"
#define TR_UPDATE_TX_OPTIONS           "تحديث خيارات TX؟"
#define TR_MODULES_RX_VERSION          BUTTON("إصدار الموديولات / RX")
#define TR_SHOW_MIXER_MONITORS         "عرض مراقبة Mixer"
#define TR_MENU_MODULES_RX_VERSION     "إصدار الموديولات / RX"
#define TR_MENU_FIRM_OPTIONS           "خيارات Firmware"
#define TR_IMU                        "IMU"
#define TR_STICKS_POTS_SLIDERS         "المحاور/Pots/Sliders"
#define TR_RF_PROTOCOL                 "بروتوكول RF"
#define TR_MODULE_OPTIONS              "خيارات الموديول"
#define TR_POWER                       "القوة"
#define TR_NO_TX_OPTIONS               "لا خيارات TX"
#define TR_RTC_BATT                    "بطارية RTC"
#define TR_POWER_METER_EXT             "مقياس القوة (خارجي)"
#define TR_POWER_METER_INT             "مقياس القوة (داخلي)"
#define TR_SPECTRUM_ANALYSER_EXT       "الطيف (خارجي)"
#define TR_SPECTRUM_ANALYSER_INT       "الطيف (داخلي)"
#define TR_GHOST_MODULE_CONFIG         "إعداد موديول Ghost"
#define TR_GPS_MODEL_LOCATOR           "تحديد موقع الموديل GPS"
#define TR_GPS_URI_STANDARD            "قياسي"
#define TR_GPS_URI_APPLE_MAPS          "Apple Maps"
#define TR_REFRESH                     "تحديث"
#define TR_SDCARD_FULL                 "كرت SD ممتلئ"
#define TR_SDCARD_FULL_EXT              TR_BW_COL(TR_SDCARD_FULL "\036Logs & Screenshots" LCDW_128_LINEBREAK "disabled", TR_SDCARD_FULL "\nالسجلات ولقطات الشاشة معطلة")
#define TR_NEEDS_FILE                  "يحتاج ملف"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "غير متوافق"
#define TR_WARNING                     "تحذير"
#define TR_STORAGE_WARNING             "التخزين"
#define TR_THROTTLE_UPPERCASE          "THROTTLE"
#define TR_ALARMSWARN                  "التنبيهات"
#define TR_SWITCHWARN                  TR("المفاتيح", "التحكم")
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("تجربة", "نسخة تجريبية")
#define TR_TEST_NOTSAFE                "للتجربة فقط"
#define TR_WARN_RTC_BATTERY_LOW        "بطارية RTC ضعيفة"
#define TR_WARN_MULTI_LOWPOWER         "وضع الطاقة المنخفضة"
#define TR_BATTERY                     "البطارية"
#define TR_WRONG_PCBREV                "تم رصد PCB خطأ"
#define TR_EMERGENCY_MODE              "وضع الطوارئ"
#define TR_NO_FAILSAFE                 "Failsafe غير مضبوط"
#define TR_KEYSTUCK                    "زر عالق"
#define TR_VOLUME                      "الصوت"
#define TR_BRIGHTNESS                  "السطوع"
#define TR_CONTROL                     "التحكم"
#define TR_SF_OVERRIDDEN               "تم تجاوزه بواسطة SF/GF"
#define TR_TTL_WARNING                 "تحذير: لا تتجاوز 3.3V على أطراف TX/RX!"
#define TR_FUNC                        "الوظيفة"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "المدة"
#define TR_DELAY                       "التأخير"
#define TR_NO_SOUNDS_ON_SD             "لا أصوات على SD"
#define TR_NO_MODELS_ON_SD             "لا موديلات على SD"
#define TR_NO_BITMAPS_ON_SD            "لا صور على SD"
#define TR_NO_SCRIPTS_ON_SD            "لا سكربتات على SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("خطأ صياغة", "خطأ صياغة بالسكربت")
#define TR_SCRIPT_PANIC                "انهيار السكربت"
#define TR_SCRIPT_ERROR                "خطأ غير معروف"
#define TR_PLAY_FILE                   "تشغيل"
#define TR_DELETE_FILE                 "حذف"
#define TR_COPY_FILE                   "نسخ"
#define TR_RENAME_FILE                 "إعادة تسمية"
#define TR_ASSIGN_BITMAP               "تعيين كصورة"
#define TR_EXECUTE_FILE                "تنفيذ"
#define TR_REMOVED                     " تم حذفه"
#define TR_SD_INFO                     "معلومات"
#define TR_NA                          "غير متاح"
#define TR_TIME                        "الوقت"
#define TR_BAUDRATE                    "Baudrate"
#define TR_CRSF_ARMING_MODE            "ARM باستخدام"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 TR("أخذ العينات","وضع العينات")
#define TR_SAMPLE_MODES_1              "عادي"
#define TR_SAMPLE_MODES_2              "OneBit"
#define TR_LOADING                     "جاري التحميل..."
#define TR_DELETE_THEME                "حذف المظهر؟"
#define TR_SAVE_THEME                  "حفظ المظهر؟"
#define TR_EDIT_COLOR                  "تعديل اللون"
#define TR_NO_THEME_IMAGE              "لا صورة للمظهر"
#define TR_BACKLIGHT_TIMER             "مهلة عدم النشاط"

#define TR_MODEL_QUICK_SELECT        "اختيار سريع للموديل"
#define TR_LABELS_SELECT             "اختيار التصنيف"
#define TR_LABELS_MATCH              "مطابقة التصنيفات"
#define TR_FAV_MATCH                 "مطابقة المفضلة"
#define TR_LABELS_SELECT_MODE_1      "اختيار متعدد"
#define TR_LABELS_SELECT_MODE_2      "اختيار واحد"
#define TR_LABELS_MATCH_MODE_1       "مطابقة الكل"
#define TR_LABELS_MATCH_MODE_2       "مطابقة أي"
#define TR_FAV_MATCH_MODE_1          "مطابقة إلزامية"
#define TR_FAV_MATCH_MODE_2          "مطابقة اختيارية"

#define TR_NO_TEMPLATES                "لا توجد قوالب موديلات بهذا المجلد"
#define TR_SAVE_TEMPLATE               "حفظ كقالب"
#define TR_BLANK_MODEL                 "موديل فارغ"
#define TR_BLANK_MODEL_INFO            "إنشاء موديل فارغ"
#define TR_FILE_EXISTS                 "الملف موجود"
#define TR_ASK_OVERWRITE               "هل تريد استبداله؟"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_INIT              "تهيئة"
#define TR_BLUETOOTH_DIST_ADDR         "العنوان البعيد"
#define TR_BLUETOOTH_LOCAL_ADDR        "العنوان المحلي"
#define TR_BLUETOOTH_PIN_CODE          "رمز PIN"
#define TR_BLUETOOTH_NODEVICES         "لم يتم العثور على أجهزة"
#define TR_BLUETOOTH_SCANNING          "جاري البحث..."
#define TR_BLUETOOTH_MODES_1           "---"
#define TR_BLUETOOTH_MODES_2           "Telemetry"
#define TR_BLUETOOTH_MODES_3           "Trainer"
#define TR_BLUETOOTH_MODES_4           "مفعل"

#define TR_SD_INFO_TITLE               "معلومات SD"
#define TR_SD_SECTORS                  "القطاعات:"
#define TR_SD_SIZE                     "الحجم:"
#define TR_TYPE                        "النوع"
#define TR_GVARS                       "GVARS"
#define TR_GLOBAL_VAR                  "Global variable"
#define TR_OWN                         "خاص"
#define TR_DATE                        "التاريخ"
#define TR_MONTHS_1                    "يناير"
#define TR_MONTHS_2                    "فبراير"
#define TR_MONTHS_3                    "مارس"
#define TR_MONTHS_4                    "أبريل"
#define TR_MONTHS_5                    "مايو"
#define TR_MONTHS_6                    "يونيو"
#define TR_MONTHS_7                    "يوليو"
#define TR_MONTHS_8                    "أغسطس"
#define TR_MONTHS_9                    "سبتمبر"
#define TR_MONTHS_10                   "أكتوبر"
#define TR_MONTHS_11                   "نوفمبر"
#define TR_MONTHS_12                   "ديسمبر"
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("وضع العجلة","وضع العجلة الدوارة")
#define TR_CHANNELS_MONITOR            "مراقبة القنوات"
#define TR_MIXERS_MONITOR              "مراقبة Mixers"
#define TR_PATH_TOO_LONG               "المسار طويل جدا"
#define TR_VIEW_TEXT                   "عرض النص"
#define TR_FLASH_BOOTLOADER            "تحديث Bootloader"
#define TR_FLASH_DEVICE                TR("تحديث جهاز","تحديث جهاز")
#define TR_FLASH_EXTERNAL_DEVICE       TR("تحديث S.Port", "تحديث جهاز S.Port")
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "تحديث RX بـ OTA خارجي"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "تحديث RX بـ OTA داخلي"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "تحديث FC بـ OTA خارجي"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "تحديث FC بـ OTA داخلي"
#define TR_FLASH_BLUETOOTH_MODULE      TR("تحديث موديول BT", "تحديث موديول Bluetooth")
#define TR_DEVICE_NO_RESPONSE          TR("الجهاز لا يستجيب", "الجهاز لا يستجيب")
#define TR_DEVICE_FILE_ERROR           TR("مشكلة بملف الجهاز", "مشكلة بملف الجهاز")
#define TR_DEVICE_DATA_REFUSED         TR("الجهاز رفض البيانات", "الجهاز رفض البيانات")
#define TR_DEVICE_WRONG_REQUEST        TR("مشكلة بالوصول للجهاز", "مشكلة بالوصول للجهاز")
#define TR_DEVICE_FILE_REJECTED        TR("الجهاز رفض الملف", "الجهاز رفض الملف")
#define TR_DEVICE_FILE_WRONG_SIG       TR("توقيع الملف خطأ", "توقيع الملف خطأ")
#define TR_CURRENT_VERSION             TR("الإصدار: ", "الإصدار الحالي: ")
#define TR_FLASH_INTERNAL_MODULE       TR("تحديث الموديول الداخلي", "تحديث الموديول الداخلي")
#define TR_FLASH_INTERNAL_MULTI        TR("تحديث Multi الداخلي", "تحديث Multi الداخلي")
#define TR_FLASH_EXTERNAL_MODULE       TR("تحديث الموديول الخارجي", "تحديث الموديول الخارجي")
#define TR_FLASH_EXTERNAL_MULTI        TR("تحديث Multi الخارجي", "تحديث Multi الخارجي")
#define TR_FLASH_EXTERNAL_ELRS         TR("تحديث ELRS الخارجي", "تحديث ELRS الخارجي")
#define TR_FIRMWARE_UPDATE_ERROR       TR("خطأ بالتحديث", "خطأ بتحديث Firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "تم التحديث بنجاح"
#define TR_WRITING                     "جاري الكتابة..."
#define TR_INTERNALRF                  "RF داخلي"
#define TR_INTERNAL_MODULE             TR("الموديول الداخلي", "الموديول الداخلي")
#define TR_EXTERNAL_MODULE             TR("الموديول الخارجي", "الموديول الخارجي")
#define TR_EDGETX_UPGRADE_REQUIRED     "يحتاج تحديث EdgeTX"
#define TR_TELEMETRY_DISABLED          "Telemetry معطل"
#define TR_MORE_OPTIONS_AVAILABLE      "توجد خيارات أكثر"
#define TR_EXTERNALRF                  "RF خارجي"
#define TR_FAILSAFE                    TR("Failsafe", "وضع Failsafe")
#define TR_FAILSAFESET                 "إعدادات Failsafe"
#define TR_REG_ID                      "رقم التسجيل"
#define TR_OWNER_ID                    "رقم المالك"
#define TR_HOLD                        "تثبيت"
#define TR_HOLD_UPPERCASE              "تثبيت"
#define TR_NONE                        "لا شيء"
#define TR_NONE_UPPERCASE              "لا شيء"
#define TR_MENUSENSOR                  "الحساس"
#define TR_POWERMETER_PEAK             "الذروة"
#define TR_POWERMETER_POWER            "القوة"
#define TR_POWERMETER_ATTN             "التخفيف"
#define TR_POWERMETER_FREQ             "التردد"
#define TR_MENUTOOLS                   "الأدوات"
#define TR_MIC_RECORDER                "مسجل الصوت"
#define TR_PUSH_TO_RECORD              "اضغط للتسجيل"
#define TR_RECORD                      "تسجيل"
#define TR_STOP                        "إيقاف"
#define TR_REC                         "REC"
#define TR_STARTING_IN                 "يبدأ خلال"
#define TR_GET_READY                   "استعد..."
#define TR_SAVED                       "تم الحفظ:"
#define TR_SAVE_AS                     "حفظ باسم"
#define TR_AUTO_TRIM                   "Trim تلقائي"
#define TR_TRIM_START                  "بداية القص"
#define TR_TRIM_END                    "نهاية القص"
#define TR_OPEN_ERROR                  "خطأ بالفتح"
#define TR_TURN_OFF_RECEIVER           "أطفئ الرسيفر"
#define TR_STOPPING                    "جاري الإيقاف..."
#define TR_MENU_SPECTRUM_ANALYSER      "محلل الطيف"
#define TR_MENU_POWER_METER            "مقياس القوة"
#define TR_SENSOR                      "الحساس"
#define TR_COUNTRY_CODE                "رمز الدولة"
#define TR_USBMODE                     "وضع USB"
#define TR_USB_CHARGE                  "الشحن أثناء التشغيل"
#define TR_JACK_MODE                   "وضع المنفذ"
#define TR_VOICE_LANGUAGE              "لغة الصوت"
#define TR_TEXT_LANGUAGE               "لغة النص"
#define TR_UNITS_SYSTEM                "الوحدات"
#define TR_UNITS_PPM                   "وحدات PPM"
#define TR_EDIT                        "تعديل"
#define TR_INSERT_BEFORE               "إدراج قبل"
#define TR_INSERT_AFTER                "إدراج بعد"
#define TR_COPY                        "نسخ"
#define TR_MOVE                        "نقل"
#define TR_PASTE                       "لصق"
#define TR_PASTE_AFTER                 "لصق بعد"
#define TR_PASTE_BEFORE                "لصق قبل"
#define TR_DELETE                      "حذف"
#define TR_INSERT                      "إدراج"
#define TR_RESET_SESSION               "إعادة ضبط الجلسة"
#define TR_RESET_TIMER1                "إعادة ضبط مؤقت1"
#define TR_RESET_TIMER2                "إعادة ضبط مؤقت2"
#define TR_RESET_TIMER3                "إعادة ضبط مؤقت3"
#define TR_RESET_TELEMETRY             "إعادة ضبط Telemetry"
#define TR_STATISTICS                  "الإحصائيات"
#define TR_ABOUT_US                    "حول"
#define TR_USB_JOYSTICK                "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB تخزين (SD)"
#define TR_USB_SERIAL                  "USB Serial (VCP)"
#define TR_AND_SWITCH                  "مفتاح AND"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "القيم بعد المعايرة"
#define TR_ANADIAGS_FILTRAWDEV         "القيم الخام المفلترة مع الانحراف"
#define TR_ANADIAGS_UNFILTRAW          "القيم الخام بدون فلترة"
#define TR_ANADIAGS_MINMAX             "الأدنى والأعلى والمدى"
#define TR_ANADIAGS_MOVE               "حرك العصا لأقصى حد!"
#define TR_BYTES                       "بايت"
#define TR_MODULE_BIND                 BUTTON(TR("ربط", "ربط"))
#define TR_MODULE_UNBIND               BUTTON("فك الربط")
#define TR_POWERMETER_ATTN_NEEDED     "يحتاج مخفف إشارة"
#define TR_PXX2_SELECT_RX              "اختيار RX"
#define TR_PXX2_DEFAULT                "<افتراضي>"
#define TR_BT_SELECT_DEVICE            "اختيار جهاز"
#define TR_DISCOVER                    BUTTON("بحث")
#define TR_BUTTON_INIT                 BUTTON("تهيئة")
#define TR_WAITING                     "بالانتظار..."
#define TR_RECEIVER_DELETE             "حذف الرسيفر؟"
#define TR_RECEIVER_RESET              "إعادة ضبط الرسيفر؟"
#define TR_SHARE                       "مشاركة"
#define TR_BIND                        "ربط"
#define TR_PAIRING                     "الاقتران"
#define TR_BTAUDIO                     "صوت BT"
#define TR_REGISTER                    BUTTON(TR("تسجيل", "تسجيل"))
#define TR_MODULE_RANGE                BUTTON(TR("المدى", "المدى"))
#define TR_RANGE_TEST                  "اختبار المدى"
#define TR_RECEIVER_OPTIONS            TR("خيارات الرسيفر", "خيارات الرسيفر")
#define TR_RESET_BTN                   BUTTON("إعادة ضبط")
#define TR_KEYS_BTN                    BUTTON("الأزرار")
#define TR_ANALOGS_BTN                 BUTTON(TR("التناظري", "التناظري"))
#define TR_FS_BTN                      BUTTON(TR("مفاتيح مخصصة", TR_FUNCTION_SWITCHES))
#define TR_SET                         BUTTON("ضبط")
#define TR_TRAINER                     "Trainer"
#define TR_CHANS                       "القنوات"
#define TR_ANTENNAPROBLEM              "مشكلة بهوائي الإرسال!"
#define TR_MODELIDUSED                 "ID مستخدم في:"
#define TR_MODELIDUNIQUE               "ID فريد"
#define TR_MODULE                      "الموديول"
#define TR_RX_NAME                     "اسم Rx"
#define TR_TELEMETRY_TYPE              TR("النوع", "نوع Telemetry")
#define TR_TELEMETRY_SENSORS           "الحساسات"
#define TR_VALUE                       "القيمة"
#define TR_PERIOD                      "الفترة"
#define TR_INTERVAL                    "الفاصل"
#define TR_REPEAT                      "التكرار"
#define TR_ENABLE                      "تفعيل"
#define TR_DISABLE                     "تعطيل"
#define TR_TOPLCDTIMER                 "مؤقت الشاشة العلوية"
#define TR_UNIT                        "الوحدة"
#define TR_TELEMETRY_NEWSENSOR         "إضافة جديد"
#define TR_CHANNELRANGE                TR("مدى القنوات", "مدى القنوات")
#define TR_ANTENNACONFIRM1             "هوائي خارجي"
#define TR_ANTENNA_MODES_1           "داخلي"
#define TR_ANTENNA_MODES_2           "اسأل"
#define TR_ANTENNA_MODES_3           "لكل موديل"
#define TR_ANTENNA_MODES_4           "داخلي + خارجي"
#define TR_ANTENNA_MODES_5           "خارجي"
#define TR_USE_INTERNAL_ANTENNA        TR("هوائي داخلي", "استخدم الهوائي الداخلي")
#define TR_USE_EXTERNAL_ANTENNA        TR("هوائي خارجي", "استخدم الهوائي الخارجي")
#define TR_ANTENNACONFIRM2             TR("افحص الهوائي", "تأكد أن الهوائي مركب!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "يتطلب Firmware غير FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "يتطلب Firmware FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "يتطلب Firmware EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "معتمد"
#define TR_LOWALARM                    "تنبيه منخفض"
#define TR_CRITICALALARM               "تنبيه حرج"
#define TR_DISABLE_ALARM               TR("تعطيل التنبيهات", "تعطيل تنبيهات Telemetry")
#define TR_POPUP                       "نافذة منبثقة"
#define TR_MIN                         "أدنى"
#define TR_MAX                         "أعلى"
#define TR_CURVE_PRESET                "جاهز..."
#define TR_PRESET                      "جاهز"
#define TR_MIRROR                      "عكس"
#define TR_CLEAR                       "مسح"
#define TR_CLEAR_BTN                   BUTTON("مسح")
#define TR_RESET                       "إعادة ضبط"
#define TR_RESET_SUBMENU               "إعادة ضبط..."
#define TR_COUNT                       "العدد"
#define TR_PT                          "نقطة"
#define TR_PTS                         "نقاط"
#define TR_SMOOTH                      "ناعم"
#define TR_COPY_STICKS_TO_OFS          TR("نسخ المحور->Subtrim", "نسخ المحور إلى Subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("نسخ الحدود للكل",  "نسخ الأدنى/الأعلى/الوسط لكل Outputs")
#define TR_COPY_TRIMS_TO_OFS           TR("نسخ Trim->Subtrim", "نسخ Trims إلى Subtrim")
#define TR_INCDEC                      "زيادة/نقصان"
#define TR_GLOBALVAR                   "Global var"
#define TR_MIXSOURCE                   "المصدر (%)"
#define TR_MIXSOURCERAW                "المصدر (قيمة)"
#define TR_CONSTANT                    "ثابت"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1 "إيقاف"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2 "تشغيل"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3 "تلقائي"
#define TR_PREFLIGHT                   "فحوصات قبل التشغيل"
#define TR_CHECKLIST                   TR("قائمة الفحص", "عرض قائمة الفحص")
#define TR_CHECKLIST_INTERACTIVE       TR3("فحص تفاعلي", "قائمة فحص تفاعلية", "قائمة فحص تفاعلية")
#define TR_AUX_SERIAL_MODE             "منفذ Serial"
#define TR_AUX_SERIAL_PORT_POWER       "طاقة المنفذ"
#define TR_SCRIPT                      "سكربت"
#define TR_INPUTS                      "Inputs"
#define TR_OUTPUTS                     "Outputs"
#define TR_TOO_MANY_LUA_SCRIPTS        "سكربتات Lua كثيرة!"
#define TR_SPORT_UPDATE_POWER_MODE     "طاقة SP"
#define TR_SPORT_UPDATE_POWER_MODES_1  "تلقائي"
#define TR_SPORT_UPDATE_POWER_MODES_2  "تشغيل"
#define TR_NO_TELEMETRY_SCREENS        "لا شاشات Telemetry"
#define TR_TOUCH_PANEL                 "شاشة اللمس:"
#define TR_FILE_SIZE                   "حجم الملف"
#define TR_FILE_OPEN                   "فتح على أي حال؟"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "الاسم"
#define TR_PHASES_HEADERS_SW           "المفتاح"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trim الـ Rudder"
#define TR_PHASES_HEADERS_ELE_TRIM     "Trim الـ Elevator"
#define TR_PHASES_HEADERS_THT_TRIM     "Trim الـ Throttle"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trim الـ Aileron"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "دخول تدريجي"
#define TR_PHASES_HEADERS_FAD_OUT      "خروج تدريجي"

#define TR_LIMITS_HEADERS_NAME         "الاسم"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "أدنى"
#define TR_LIMITS_HEADERS_MAX          "أعلى"
#define TR_LIMITS_HEADERS_DIRECTION    "الاتجاه"
#define TR_LIMITS_HEADERS_CURVE        "Curve"
#define TR_LIMITS_HEADERS_PPMCENTER    "وسط PPM"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "وضع Subtrim"
#define TR_INVERTED                    "معكوس"

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "أول قناة"
#define TR_LAST_CHANNEL              "آخر قناة"
#define TR_FILL_BACKGROUND           "تعبئة الخلفية؟"
#define TR_BG_COLOR                  "لون الخلفية"
#define TR_SLIDERS                   "Sliders"
#define TR_FLIGHT_MODE               "Flight mode"
#define TR_TIMER_SOURCE              "مصدر المؤقت"
#define TR_SIZE                      "الحجم"
#define TR_SHADOW                    "الظل"
#define TR_ALIGNMENT                 "المحاذاة"
#define TR_ALIGN_LABEL               "محاذاة العنوان"
#define TR_ALIGN_VALUE               "محاذاة القيمة"
#define TR_ALIGN_OPTS_1              "يسار"
#define TR_ALIGN_OPTS_2              "وسط"
#define TR_ALIGN_OPTS_3              "يمين"
#define TR_TEXT                      "النص"
#define TR_COLOR                     "اللون"
#define TR_PANEL1_BACKGROUND         "خلفية اللوحة1"
#define TR_PANEL2_BACKGROUND         "خلفية اللوحة2"
#define TR_PANEL_BACKGROUND          "الخلفية"
#define TR_PANEL_COLOR               "  اللون"
#define TR_WIDGET_GAUGE              "مؤشر"
#define TR_WIDGET_MODELBMP           "معلومات الموديل"
#define TR_WIDGET_OUTPUTS            "Outputs"
#define TR_WIDGET_TEXT               "نص"
#define TR_WIDGET_TIMER              "مؤقت"
#define TR_WIDGET_VALUE              "قيمة"

// About screen
#define TR_ABOUTUS                     TR(" حول ", "حول")

#define TR_CHR_INPUT                   "I"   // Values between A-I will work

#define TR_BEEP_VOLUME                 "صوت الصفارة"
#define TR_WAV_VOLUME                  "صوت الملفات"
#define TR_BG_VOLUME                   TR("صوت الخلفية", "صوت الخلفية")

#define TR_TOP_BAR                     "الشريط العلوي"
#define TR_FLASH_ERASE                 "جاري المسح..."
#define TR_FLASH_WRITE                 "جاري الكتابة..."
#define TR_OTA_UPDATE                  "تحديث OTA..."
#define TR_MODULE_RESET                "إعادة تشغيل الموديول..."
#define TR_UNSUPPORTED_RX              "RX غير مدعوم"
#define TR_OTA_UPDATE_ERROR            "خطأ بتحديث OTA"
#define TR_DEVICE_RESET                "إعادة تشغيل الجهاز..."
#define TR_ALTITUDE                    "الارتفاع"
#define TR_SCALE                       "المقياس"
#define TR_VIEW_CHANNELS               "عرض القنوات"
#define TR_VIEW_NOTES                  "عرض الملاحظات"
#define TR_ID                          "ID"
#define TR_PRECISION                   "الدقة"
#define TR_RATIO                       "النسبة"
#define TR_FORMULA                     "المعادلة"
#define TR_CELLINDEX                   "رقم الخلية"
#define TR_LOGS                        "السجلات"
#define TR_OPTIONS                     "الخيارات"
#define TR_FIRMWARE_OPTIONS            BUTTON("خيارات Firmware")

#define TR_ALTSENSOR                   "حساس الارتفاع"
#define TR_CELLSENSOR                  "حساس الخلايا"
#define TR_GPSSENSOR                   "حساس GPS"
#define TR_GYRO                        "Gyro"
#define TR_CURRENTSENSOR               "الحساس"
#define TR_AUTOOFFSET                  "إزاحة تلقائية"
#define TR_ONLYPOSITIVE                "موجب"
#define TR_FILTER                      "فلتر"
#define TR_TELEMETRYFULL               TR("كل الخانات ممتلئة!", "كل خانات Telemetry ممتلئة!")
#define TR_IGNORE_INSTANCE             TR("تجاهل النسخ", "تجاهل النسخ")
#define TR_SHOW_INSTANCE_ID            "عرض رقم النسخة"
#define TR_DISCOVER_SENSORS            "بحث عن جديد"
#define TR_STOP_DISCOVER_SENSORS       "إيقاف"
#define TR_DELETE_ALL_SENSORS          "حذف الكل"
#define TR_CONFIRMDELETE               "متأكد من " LCDW_128_LINEBREAK "حذف الكل؟"
#define TR_SELECT_WIDGET               "اختيار Widget"
#define TR_WIDGET_FULLSCREEN           "ملء الشاشة"
#define TR_REMOVE_WIDGET               "إزالة Widget"
#define TR_WIDGET_SETTINGS             "إعدادات Widget"
#define TR_REMOVE_SCREEN               "إزالة الشاشة"
#define TR_SETUP_WIDGETS               "إعداد Widgets"
#define TR_THEME                       "المظهر"
#define TR_LAYOUT                      "التخطيط"
#define TR_TEXT_COLOR                  "لون النص"
#define TR_MENU_INPUTS                 CHAR_INPUT "Inputs"
#define TR_MENU_LUA                    CHAR_LUA "سكربتات Lua"
#define TR_MENU_STICKS                 CHAR_STICK "المحاور"
#define TR_MENU_POTS                   CHAR_POT "Pots"
#define TR_MENU_MIN                    CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   CHAR_CYC "Cyclic"
#define TR_MENU_TRIMS                  CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES               CHAR_SWITCH "المفاتيح"
#define TR_MENU_LOGICAL_SWITCHES       CHAR_SWITCH "Logical switches"
#define TR_MENU_TRAINER                CHAR_TRAINER "Trainer"
#define TR_MENU_CHANNELS               CHAR_CHANNEL "القنوات"
#define TR_MENU_GVARS                  CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY              CHAR_TELEMETRY "Telemetry"
#define TR_MENU_DISPLAY                "العرض"
#define TR_MENU_OTHER                  "أخرى"
#define TR_MENU_INVERT                 "عكس"
#define TR_AUDIO_MUTE                  TR("كتم الصوت","كتم عند عدم وجود صوت")
#define TR_PWM_OUTPUT                  "خرج PWM"
#define TR_JITTER_FILTER               "فلتر ADC"
#define TR_DEAD_ZONE                   "المنطقة الميتة"
#define TR_RTC_CHECK                   TR("فحص RTC", "فحص جهد RTC")
#define TR_AUTH_FAILURE                "فشل التحقق"
#define TR_RACING_MODE                 "وضع السباق"

#define TR_USE_THEME_COLOR              "استخدم لون المظهر"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "إضافة كل Trims إلى Subtrims"
#define TR_DUPLICATE                    "تكرار"
#define TR_ACTIVATE                     "تعيين كنشط"
#define TR_COLOR_PICKER                 "اختيار اللون"
#define TR_FIXED                        "ثابت"
#define TR_EDIT_THEME_DETAILS           "تعديل تفاصيل المظهر"
#define TR_THEME_COLOR_PRIMARY1        "PRIMARY1"
#define TR_THEME_COLOR_PRIMARY2        "PRIMARY2"
#define TR_THEME_COLOR_PRIMARY3        "PRIMARY3"
#define TR_THEME_COLOR_SECONDARY1      "SECONDARY1"
#define TR_THEME_COLOR_SECONDARY2      "SECONDARY2"
#define TR_THEME_COLOR_SECONDARY3      "SECONDARY3"
#define TR_THEME_COLOR_FOCUS           "التركيز"
#define TR_THEME_COLOR_EDIT            "التعديل"
#define TR_THEME_COLOR_ACTIVE          "النشط"
#define TR_THEME_COLOR_WARNING         "التحذير"
#define TR_THEME_COLOR_DISABLED        "المعطل"
#define TR_THEME_COLOR_QM_BG           "خلفية القائمة السريعة"
#define TR_THEME_COLOR_QM_FG           "نص القائمة السريعة"
#define TR_THEME_COLOR_CUSTOM          "مخصص"
#define TR_THEME_CHECKBOX              "مربع اختيار"
#define TR_THEME_ACTIVE                "نشط"
#define TR_THEME_REGULAR               "عادي"
#define TR_THEME_WARNING               "تحذير"
#define TR_THEME_DISABLED              "معطل"
#define TR_THEME_EDIT                  "تعديل"
#define TR_THEME_FOCUS                 "تركيز"
#define TR_AUTHOR                       "المؤلف"
#define TR_DESCRIPTION                  "الوصف"
#define TR_SAVE                         "حفظ"
#define TR_CANCEL                       "إلغاء"
#define TR_EDIT_THEME                   "تعديل المظهر"
#define TR_DETAILS                      "التفاصيل"

// Voice in native language
#define TR_VOICE_ENGLISH                "الإنجليزية"
#define TR_VOICE_CHINESE                "الصينية"
#define TR_VOICE_CZECH                  "التشيكية"
#define TR_VOICE_DANISH                 "الدنماركية"
#define TR_VOICE_DEUTSCH                "الألمانية"
#define TR_VOICE_DUTCH                  "الهولندية"
#define TR_VOICE_ESPANOL                "الإسبانية"
#define TR_VOICE_FINNISH                "الفنلندية"
#define TR_VOICE_FRANCAIS               "الفرنسية"
#define TR_VOICE_HUNGARIAN              "المجرية"
#define TR_VOICE_ITALIANO               "الإيطالية"
#define TR_VOICE_POLISH                 "البولندية"
#define TR_VOICE_PORTUGUES              "البرتغالية"
#define TR_VOICE_RUSSIAN                "الروسية"
#define TR_VOICE_SLOVAK                 "السلوفاكية"
#define TR_VOICE_SWEDISH                "السويدية"
#define TR_VOICE_TAIWANESE              "التايوانية"
#define TR_VOICE_JAPANESE               "اليابانية"
#define TR_VOICE_ARABIC                 "العربية"
#define TR_VOICE_HEBREW                 "العبرية"
#define TR_VOICE_UKRAINIAN              "الأوكرانية"
#define TR_VOICE_KOREAN                 "الكورية"

#define TR_USBJOYSTICK_LABEL           "USB Joystick"
#define TR_USBJOYSTICK_EXTMODE         "الوضع"
#define TR_VUSBJOYSTICK_EXTMODE_1      "كلاسيكي"
#define TR_VUSBJOYSTICK_EXTMODE_2      "متقدم"
#define TR_USBJOYSTICK_SETTINGS        BUTTON("إعدادات القنوات")
#define TR_USBJOYSTICK_IF_MODE         TR("الواجهة","وضع الواجهة")
#define TR_VUSBJOYSTICK_IF_MODE_1      "Joystick"
#define TR_VUSBJOYSTICK_IF_MODE_2      "Gamepad"
#define TR_VUSBJOYSTICK_IF_MODE_3      "MultiAxis"
#define TR_USBJOYSTICK_CH_MODE         "الوضع"
#define TR_VUSBJOYSTICK_CH_MODE_1      "لا شيء"
#define TR_VUSBJOYSTICK_CH_MODE_2      "زر"
#define TR_VUSBJOYSTICK_CH_MODE_3      "محور"
#define TR_VUSBJOYSTICK_CH_MODE_4      "محاكي"
#define TR_VUSBJOYSTICK_CH_MODE_S_1    "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2    "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3    "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4    "S"
#define TR_USBJOYSTICK_CH_BTNMODE      "وضع الزر"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1   "عادي"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2   "نبضة"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3   "SWEmu"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4   "Delta"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5   "Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1 TR("عادي","عادي")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2 TR("نبضة","نبضة")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3 TR("SWEm","SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4 TR("Delt","Delta")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5 TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "الأوضاع"
#define TR_VUSBJOYSTICK_CH_SWPOS_1     "ضغطة"
#define TR_VUSBJOYSTICK_CH_SWPOS_2     "2POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_3     "3POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_4     "4POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_5     "5POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_6     "6POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_7     "7POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_8     "8POS"
#define TR_USBJOYSTICK_CH_AXIS         "المحور"
#define TR_VUSBJOYSTICK_CH_AXIS_1      "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2      "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3      "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4      "rotX"
#define TR_VUSBJOYSTICK_CH_AXIS_5      "rotY"
#define TR_VUSBJOYSTICK_CH_AXIS_6      "rotZ"
#define TR_VUSBJOYSTICK_CH_AXIS_7      "Slider"
#define TR_VUSBJOYSTICK_CH_AXIS_8      "قرص"
#define TR_VUSBJOYSTICK_CH_AXIS_9      "عجلة"
#define TR_USBJOYSTICK_CH_SIM          "محور المحاكي"
#define TR_VUSBJOYSTICK_CH_SIM_1       "Ail"
#define TR_VUSBJOYSTICK_CH_SIM_2       "Ele"
#define TR_VUSBJOYSTICK_CH_SIM_3       "Rud"
#define TR_VUSBJOYSTICK_CH_SIM_4       "Thr"
#define TR_VUSBJOYSTICK_CH_SIM_5       "Acc"
#define TR_VUSBJOYSTICK_CH_SIM_6       "Brk"
#define TR_VUSBJOYSTICK_CH_SIM_7       "Steer"
#define TR_VUSBJOYSTICK_CH_SIM_8       "Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "العكس"
#define TR_USBJOYSTICK_CH_BTNNUM       "رقم الزر"
#define TR_USBJOYSTICK_BTN_COLLISION   "!تعارض برقم الزر!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!تعارض بالمحاور!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("قص دائري", "قص دائري")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1 "لا شيء"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2 "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3 "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4 "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   BUTTON("تطبيق التغييرات")

#define TR_DIGITAL_SERVO          "Servo333HZ"
#define TR_ANALOG_SERVO           "Servo 50HZ"
#define TR_SIGNAL_OUTPUT          "خرج الإشارة"
#define TR_SERIAL_BUS             "Serial bus"
#define TR_SYNC                   "مزامنة"

#define TR_ENABLED_FEATURES       "الميزات المفعلة"
#define TR_RADIO_MENU_TABS        "قائمة الريموت"
#define TR_MODEL_MENU_TABS        "قائمة الموديل"

#define TR_SELECT_MENU_ALL        "الكل"
#define TR_SELECT_MENU_CLR        "مسح"
#define TR_SELECT_MENU_INV        "عكس"

#define TR_SORT_ORDERS_1          "الاسم A-Z"
#define TR_SORT_ORDERS_2          "الاسم Z-A"
#define TR_SORT_ORDERS_3          "الأقل استخداما"
#define TR_SORT_ORDERS_4          "الأكثر استخداما"
#define TR_SORT_MODELS_BY         "ترتيب الموديلات حسب"
#define TR_CREATE_NEW             "إنشاء"

#define TR_MIX_SLOW_PREC          TR("دقة الإبطاء", "دقة الإبطاء")
#define TR_MIX_DELAY_PREC         TR("دقة التأخير", "دقة التأخير")

#define TR_THEME_EXISTS           "يوجد مجلد مظهر بنفس الاسم."

#define TR_DATE_TIME_WIDGET       "التاريخ والوقت"
#define TR_RADIO_INFO_WIDGET      "معلومات الريموت"
#define TR_LOW_BATT_COLOR         "بطارية منخفضة"
#define TR_MID_BATT_COLOR         "بطارية متوسطة"
#define TR_HIGH_BATT_COLOR        "بطارية عالية"

#define TR_WIDGET_SIZE            "حجم Widget"

#define TR_DEL_DIR_NOT_EMPTY      "يجب أن يكون المجلد فارغا قبل الحذف"

#define TR_KEY_SHORTCUTS          "اختصارات الأزرار"
#define TR_CURRENT_SCREEN         "الشاشة الحالية"
#define TR_SHORT_PRESS            "ضغطة قصيرة"
#define TR_LONG_PRESS             "ضغطة مطولة"
#define TR_OPEN_QUICK_MENU        "فتح القائمة السريعة"
#define TR_QUICK_MENU_FAVORITES   "مفضلة القائمة السريعة"
