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
 *
 */

 // UA translations author: @user <user@user.com>

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */

// Main menu
#define TR_QM_MANAGE_MODELS             "Manage\nModels"
#define TR_QM_MODEL_SETUP               "Model\nSetup"
#define TR_QM_RADIO_SETUP               "Radio\nSetup"
#define TR_QM_UI_SETUP                  "UI\nSetup"
#define TR_QM_TOOLS                     "Tools"
#define TR_QM_GEN_SETTINGS              "General\nSettings"
#define TR_QM_FLIGHT_MODES              TR_SFC_AIR("Drive\nModes", "Flight\nModes")
#define TR_QM_INPUTS                    "Inputs"
#define TR_QM_MIXES                     "Mixes"
#define TR_QM_OUTPUTS                   "Outputs"
#define TR_QM_CURVES                    "Curves"
#define TR_QM_GLOBAL_VARS               "Global\nVariables"
#define TR_QM_LOGICAL_SW                "Logical\nSwitches"
#define TR_QM_SPEC_FUNC                 "Special\nFunctions"
#define TR_QM_CUSTOM_LUA                "Custom\nScripts"
#define TR_QM_TELEM                     "Telemetry"
#define TR_QM_GLOB_FUNC                 "Global\nFunctions"
#define TR_QM_TRAINER                   "Trainer"
#define TR_QM_HARDWARE                  "Hardware"
#define TR_QM_ABOUT                     "About\nEdgeTX"
#define TR_QM_THEMES                    "Themes"
#define TR_QM_TOP_BAR                   "Top Bar"
#define TR_QM_SCREEN_1                  "Screen 1"
#define TR_QM_SCREEN_2                  "Screen 2"
#define TR_QM_SCREEN_3                  "Screen 3"
#define TR_QM_SCREEN_4                  "Screen 4"
#define TR_QM_SCREEN_5                  "Screen 5"
#define TR_QM_SCREEN_6                  "Screen 6"
#define TR_QM_SCREEN_7                  "Screen 7"
#define TR_QM_SCREEN_8                  "Screen 8"
#define TR_QM_SCREEN_9                  "Screen 9"
#define TR_QM_SCREEN_10                 "Screen 10"
#define TR_QM_ADD_SCREEN                "Add\nScreen"
#define TR_QM_APPS                      "Apps"
#define TR_QM_STORAGE                   "Storage"
#define TR_QM_RESET                     TR_SFC_AIR("Drive\nReset", "Flight\nReset")
#define TR_QM_CHAN_MON                  "Channel\nMonitor"
#define TR_QM_LS_MON                    "LS\nMonitor"
#define TR_QM_STATS                     "Statistics"
#define TR_QM_DEBUG                     "Debug"
#define TR_MAIN_MENU_SETTINGS           "General Settings"
#define TR_MAIN_MENU_MANAGE_MODELS      "Керувати Моделями"
#define TR_MAIN_MENU_MODEL_NOTES        "Моделі Нотатки"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Канал Монітор"
#define TR_MONITOR_SWITCHES            "МОНІТОР ЛОГІЧНИХ ПЕРЕМИКАЧІВ"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Модель Налаштування"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Радіо Налаштування"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "UI Setup"
#define TR_MAIN_MENU_STATISTICS         "Статистика"
#define TR_MAIN_MENU_ABOUT_EDGETX       "Про... EdgeTX"
#define TR_MAIN_VIEW_X                  "Screen "
#define TR_MAIN_MENU_THEMES                "ТЕМИ"
#define TR_MAIN_MENU_APPS               "Apps"
#define TR_MENUHELISETUP               "ГЕЛІКОПТЕР"
#define TR_MENUFLIGHTMODES             TR_SFC_AIR("РЕЖИМИ ВОДІННЯ", "РЕЖИМИ ПОЛЬОТУ")
#define TR_MENUFLIGHTMODE              TR_SFC_AIR("РЕЖИМ ВОДІННЯ", "РЕЖИМ ПОЛЬОТУ")
#define TR_MENUINPUTS                  "ВХОДИ"
#define TR_MENULIMITS                  "ВИХОДИ"
#define TR_MENUCURVES                  "КРИВІ"
#define TR_MIXES                       "MIXES"
#define TR_MENU_GLOBAL_VARS            "Глобальні змінні"
#define TR_MENULOGICALSWITCHES         "ЛОГІЧНІ ПЕРЕМ."
#define TR_MENUCUSTOMFUNC              "СПЕЦІАЛ. ФУНКЦІЇ"
#define TR_MENUCUSTOMSCRIPTS           "КОРИСТ. СКРИПТИ"
#define TR_MENUTELEMETRY               "ТЕЛЕМЕТРІЯ"
#define TR_MENUSPECIALFUNCS            "ГЛОБАЛЬНІ ФУНКЦІЇ"
#define TR_MENUTRAINER                 "ТРЕНЕР"
#define TR_HARDWARE                    "АПАРАТНЕ ЗАБЕЗПЕЧ."
#define TR_USER_INTERFACE               "Top Bar"
#define TR_SD_CARD                     "SD карта"
#define TR_DEBUG                       "Відлагод."
#define TR_MENU_RADIO_SWITCHES         TR("ПЕРЕМИКАЧІ", "ТЕСТ ПЕРЕМИКАЧІВ")
#define TR_MENUCALIBRATION             "КАЛІБРУВАННЯ"
#define TR_FUNCTION_SWITCHES           "Користувацькі перемикачі"
// End Main menu

#define TR_MINUTE_SINGULAR            "хвилина"
#define TR_MINUTE_PLURAL1             "хв."
#define TR_MINUTE_PLURAL2             "хв."

#define TR_OFFON_1                     "OFF"
#define TR_OFFON_2                     "ON"
#define TR_MMMINV_1                    "---"
#define TR_MMMINV_2                    "інв."
#define TR_VBEEPMODE_1                 "Тиша"
#define TR_VBEEPMODE_2                 "Тривога"
#define TR_VBEEPMODE_3                 "Без кноп."
#define TR_VBEEPMODE_4                 "Все"
#define TR_VBLMODE_1                   "OFF"
#define TR_VBLMODE_2                   "Кноп."
#define TR_VBLMODE_3                   TR("Ctrl","Controls")
#define TR_VBLMODE_4                   "Обоє"
#define TR_VBLMODE_5                   "ON"
#define TR_TRNMODE_1                   "OFF"
#define TR_TRNMODE_2                   TR("+=","Дод.")
#define TR_TRNMODE_3                   TR(":=","Замін.")
#define TR_TRNCHN_1                    "CH1"
#define TR_TRNCHN_2                    "CH2"
#define TR_TRNCHN_3                    "CH3"
#define TR_TRNCHN_4                    "CH4"	/* use english */

#define TR_AUX_SERIAL_MODES_1          "OFF"
#define TR_AUX_SERIAL_MODES_2          "Дзерк. телем."
#define TR_AUX_SERIAL_MODES_3          "Вх. телем."
#define TR_AUX_SERIAL_MODES_4          "SBUS Трен."
#define TR_AUX_SERIAL_MODES_5          "LUA"
#define TR_AUX_SERIAL_MODES_6          "CLI"
#define TR_AUX_SERIAL_MODES_7          "GPS"
#define TR_AUX_SERIAL_MODES_8          "Відлаг."
#define TR_AUX_SERIAL_MODES_9          "SpaceMouse"
#define TR_AUX_SERIAL_MODES_10         "Зовн. модуль"	/* for untranslated - use english */
#define TR_SWTYPES_1                   "Немає"
#define TR_SWTYPES_2                   "Перемик."
#define TR_SWTYPES_3                   "2поз. пер."
#define TR_SWTYPES_4                   "3поз. пер."
#define TR_SWTYPES_5                   "Global"
#define TR_POTTYPES_1                  "Немає"
#define TR_POTTYPES_2                  "Потенц."
#define TR_POTTYPES_3                  TR("Пот. з крок.","Потенц. з кроком")
#define TR_POTTYPES_4                  "Повзун."
#define TR_POTTYPES_5                  TR("Баг.поз.пер.","Багатопозиц. перем.")
#define TR_POTTYPES_6                  "Вісь X"
#define TR_POTTYPES_7                  "Вісь Y"
#define TR_POTTYPES_8                  "Перемик."
#define TR_VPERSISTENT_1               "OFF"
#define TR_VPERSISTENT_2               "Політ"
#define TR_VPERSISTENT_3               "Ручн. скид."	/* for untranslated - use english */
#define TR_COUNTRY_CODES_1             TR("US","America")
#define TR_COUNTRY_CODES_2             TR("JP","Japan")
#define TR_COUNTRY_CODES_3             TR("EU","Europe")	/* use english */
#define TR_USBMODES_1                  "Вибір"
#define TR_USBMODES_2                  TR("Джойст.","Джойстик")
#define TR_USBMODES_3                  TR("SD карт.","SD картка")
#define TR_USBMODES_4                  "Послід."	/* for untranslated - use english */
#define TR_JACK_MODES_1                "Вибір"
#define TR_JACK_MODES_2                "Аудіо"
#define TR_JACK_MODES_3                "Трен."

#define TR_SBUS_INVERSION_VALUES_1     "норм."
#define TR_SBUS_INVERSION_VALUES_2     "не інверт."
#define TR_MULTI_CUSTOM                "Налаштований"
#define TR_VTRIMINC_1                  TR("Експо","Експоненц.")
#define TR_VTRIMINC_2                  TR("Наддрібн.","Над дрібно")
#define TR_VTRIMINC_3                  "Дрібно"
#define TR_VTRIMINC_4                  "Середньо"
#define TR_VTRIMINC_5                  "Грубо"
#define TR_VDISPLAYTRIMS_1             "Немає"
#define TR_VDISPLAYTRIMS_2             "Змін."
#define TR_VDISPLAYTRIMS_3             "Так"
#define TR_VBEEPCOUNTDOWN_1            "Тихо"
#define TR_VBEEPCOUNTDOWN_2            "Сигн."
#define TR_VBEEPCOUNTDOWN_3            "Голос"
#define TR_VBEEPCOUNTDOWN_4            "Вібро"
#define TR_VBEEPCOUNTDOWN_5            TR("С & В","Сигн. & Вібро")
#define TR_VBEEPCOUNTDOWN_6            TR("Г & В","Голос & Вібро")
#define TR_COUNTDOWNVALUES_1           "5с"
#define TR_COUNTDOWNVALUES_2           "10с"
#define TR_COUNTDOWNVALUES_3           "20с"
#define TR_COUNTDOWNVALUES_4           "30с"
#define TR_VVARIOCENTER_1              "Тон"
#define TR_VVARIOCENTER_2              "Тиша"
#define TR_CURVE_TYPES_1               "Стандарт"
#define TR_CURVE_TYPES_2               "Вибір"

#define TR_ADCFILTERVALUES_1           "Global"
#define TR_ADCFILTERVALUES_2           "Off"
#define TR_ADCFILTERVALUES_3           "On"	/* use english */

#define TR_VCURVETYPE_1                "Різн"
#define TR_VCURVETYPE_2                "Експо"
#define TR_VCURVETYPE_3                "Функц."
#define TR_VCURVETYPE_4                "Корист."
#define TR_VMLTPX_1                    "Дод."
#define TR_VMLTPX_2                    "Множ."
#define TR_VMLTPX_3                    "Замін."

#define TR_CSWTIMER                    TR("Тмр", "Таймер")
#define TR_CSWSTICKY                   TR("Лип.", "Липучка")
#define TR_CSWSTAY                     "Край"

#define TR_SF_TRAINER                  "Тренер"
#define TR_SF_INST_TRIM                "Митєв. трим."
#define TR_SF_RESET                    "Скид."
#define TR_SF_SET_TIMER                "Встан."
#define TR_SF_VOLUME                   "Гучність"
#define TR_SF_FAILSAFE                 "АварРежим"
#define TR_SF_RANGE_CHECK              "ПеревірДіап."
#define TR_SF_MOD_BIND                 "Модуль Bind"
#define TR_SF_RGBLEDS                  "RGB leds"	/* use english */

#define TR_SOUND                       "Грати Звук"
#define TR_PLAY_TRACK                  "Грати Трек"
#define TR_PLAY_VALUE                  TR("Грати Знач.","Грати Знач.")
#define TR_SF_HAPTIC                   "Вібро"
#define TR_SF_PLAY_SCRIPT              "Lua Скрипт"
#define TR_SF_BG_MUSIC                 "Фон.Муз."
#define TR_SF_BG_MUSIC_PAUSE           "Фон.Муз. ||"
#define TR_SF_LOGS                     "SD лог"
#define TR_ADJUST_GVAR                 "Налашт."
#define TR_SF_BACKLIGHT                "Підсвіт."
#define TR_SF_VARIO                    "Варіо"
#define TR_SF_TEST                     "Тест"

#define TR_SF_SAFETY                   TR("Перевизн.", "Перевизнач.")

#define TR_SF_SCREENSHOT               "Скріншот"
#define TR_SF_RACING_MODE              "Режим перегонів"
#define TR_SF_DISABLE_TOUCH            "Сенсор вимк."
#define TR_SF_DISABLE_AUDIO_AMP        "Підсил. вимк."
#define TR_SF_SET_SCREEN               TR_BW_COL("Set Screen", "Вибір гол. екр.")
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Телем", "Телеметрія")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS_1          "Тмр1"
#define TR_FSW_RESET_TIMERS_2          "Тмр2"
#define TR_FSW_RESET_TIMERS_3          "Тмр3"

#define TR_VFSWRESET_1                 TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                 TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                 TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                 TR("Все","Політ")
#define TR_VFSWRESET_5                 TR_FSW_RESET_TELEM
#define TR_VFSWRESET_6                 TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS_1                TR("Сг1","Синг.1")
#define TR_FUNCSOUNDS_2                TR("Сг2","Синг.2")
#define TR_FUNCSOUNDS_3                TR("Сг3","Синг.3")
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

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM_1              "Metric"
#define TR_VUNITSSYSTEM_2              TR("Imper.","Imperial")
#define TR_VTELEMUNIT_1                "-"
#define TR_VTELEMUNIT_2                "V"
#define TR_VTELEMUNIT_3                "A"
#define TR_VTELEMUNIT_4                "mA"
#define TR_VTELEMUNIT_5                "вузл."
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
#define TR_VTELEMUNIT_25               "mm"
#define TR_VTELEMUNIT_26               "Hz"
#define TR_VTELEMUNIT_27               "ms"
#define TR_VTELEMUNIT_28               "us"
#define TR_VTELEMUNIT_29               "km"
#define TR_VTELEMUNIT_30               "dBm"	/* for untranslated - use english */

#define TR_VTELEMSCREENTYPE_1          "Немає"
#define TR_VTELEMSCREENTYPE_2          "Цифри"
#define TR_VTELEMSCREENTYPE_3          "Граф"
#define TR_VTELEMSCREENTYPE_4          "Скрипт"
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

#define TR_ON_ONE_SWITCHES_1           "ON"
#define TR_ON_ONE_SWITCHES_2           "Один"

#define TR_HATSMODE                    "Hats mode"	/* use english */
#define TR_HATSOPT_1                   "Тільки Трим."
#define TR_HATSOPT_2                   "Тільки Кнопки"
#define TR_HATSOPT_3                   "Перемик."
#define TR_HATSOPT_4                   "Global"	/* for untranslated - use english */
#define TR_HATSMODE_TRIMS              "Hats mode: Трим."
#define TR_HATSMODE_KEYS               "Hats mode: Кнопки"
#define TR_HATSMODE_KEYS_HELP          "Ліва сторона:\n"\
                                       "   Праворуч = MDL\n"\
                                       "   Вгору = SYS\n"\
                                       "   Вниз = TELE\n"\
                                       "\n"\
                                       "Права сторона:\n"\
                                       "   Ліворуч = PAGE<\n"\
                                       "   Праворуч = PAGE>\n"\
                                       "   Вгору = PREV/INC\n"\
                                       "   Вниз = NEXT/DEC"

#define TR_ROTARY_ENC_OPT_1       "Норм"
#define TR_ROTARY_ENC_OPT_2       "Інверт."
#define TR_ROTARY_ENC_OPT_3       "V-I H-N"
#define TR_ROTARY_ENC_OPT_4       "V-I H-A"
#define TR_ROTARY_ENC_OPT_5       "V-N E-I"

#define TR_IMU_VSRCRAW_1             "TltX"
#define TR_IMU_VSRCRAW_2             "TltY"
#define TR_CYC_VSRCRAW_1             "CYC1"
#define TR_CYC_VSRCRAW_2             "CYC2"
#define TR_CYC_VSRCRAW_3             "CYC3"

#define TR_SRC_BATT                    "Batt"	/* use english */
#define TR_SRC_TIME                    "Час"
#define TR_SRC_GPS                     "GPS"	/* use english */
#define TR_SRC_TIMER                   "Тмр"

#define TR_VTMRMODES_1                 "OFF"
#define TR_VTMRMODES_2                 "ON"
#define TR_VTMRMODES_3                 "Strt"
#define TR_VTMRMODES_4                 "THs"
#define TR_VTMRMODES_5                 "TH%"
#define TR_VTMRMODES_6                 "THt"	/* use english */
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Майстер/Розн."
#define TR_VTRAINER_SLAVE_JACK         "Слейв/Розн."
#define TR_VTRAINER_MASTER_SBUS_MODULE "Майстер/SBUS Модуль"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Майстер/CPPM Модуль"
#define TR_VTRAINER_MASTER_BATTERY     "Майстер/Послід."
#define TR_VTRAINER_BLUETOOTH_1        "Майстер/" TR("BT","Bluetooth")
#define TR_VTRAINER_BLUETOOTH_2        "Слейв/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Майстер/Мульти"
#define TR_VTRAINER_CRSF               "Майстер/CRSF"
#define TR_VFAILSAFE_1                 "Не встан."
#define TR_VFAILSAFE_2                 "Тримай"
#define TR_VFAILSAFE_3                 "Корист."
#define TR_VFAILSAFE_4                 "Немає імпульсу"
#define TR_VFAILSAFE_5                 "Приймач"
#define TR_VSENSORTYPES_1              "Корист."
#define TR_VSENSORTYPES_2              "Обчислено"
#define TR_VFORMULAS_1                 "Дод."
#define TR_VFORMULAS_2                 "Середн."
#define TR_VFORMULAS_3                 "Min"
#define TR_VFORMULAS_4                 "Max"
#define TR_VFORMULAS_5                 "Множ."
#define TR_VFORMULAS_6                 "Сума"
#define TR_VFORMULAS_7                 "Комірка"
#define TR_VFORMULAS_8                 "Витрата"
#define TR_VFORMULAS_9                 "Дистанц."
#define TR_VPREC_1                     "0.--"
#define TR_VPREC_2                     "0.0 "
#define TR_VPREC_3                     "0.00"
#define TR_VCELLINDEX_1                "Нижн."
#define TR_VCELLINDEX_2                "1"
#define TR_VCELLINDEX_3                "2"
#define TR_VCELLINDEX_4                "3"
#define TR_VCELLINDEX_5                "4"
#define TR_VCELLINDEX_6                "5"
#define TR_VCELLINDEX_7                "6"
#define TR_VCELLINDEX_8                "7"
#define TR_VCELLINDEX_9                "8"
#define TR_VCELLINDEX_10               "Вищ."
#define TR_VCELLINDEX_11               "Різниця"
#define TR_SUBTRIMMODES_1              CHAR_DELTA " (лише центр)"
#define TR_SUBTRIMMODES_2              "= (симетричн.)"
#define TR_TIMER_DIR_1                 TR("Залиш.", "Показ.Залиш")
#define TR_TIMER_DIR_2                 TR("Пройш.", "Пройшло")

#define TR_FONT_SIZES_1                "Стд"
#define TR_FONT_SIZES_2                "Жирн."
#define TR_FONT_SIZES_3                "XXS"
#define TR_FONT_SIZES_4                "XS"
#define TR_FONT_SIZES_5                "L"
#define TR_FONT_SIZES_6                "XL"
#define TR_FONT_SIZES_7                "XXL"

#define TR_ENTER                       "[ENTER]"
#define TR_OK                          TR_BW_COL(TR("\010\010\010[OK]", "\010\010\010\010\010[OK]"), "Ok")
#define TR_EXIT                        TR_BW_COL("ВИХІД", "Поверн.")

#define TR_YES                         "Yes"	/* use english */
#define TR_NO                          "No"		/* use english */
#define TR_DELETEMODEL                 "ВИДАЛИТИ МОДЕЛЬ"
#define TR_COPYINGMODEL                "Копіюю модель..."
#define TR_MOVINGMODEL                 "Переміщую модель..."
#define TR_LOADINGMODEL                "Зав. модель..."
#define TR_UNLABELEDMODEL              "Без назви"
#define TR_NAME                        "Ім'я"
#define TR_MODELNAME                   "Ім'я моделі"
#define TR_PHASENAME                   "Ім'я режиму"
#define TR_MIXNAME                     "Ім'я мікш."
#define TR_INPUTNAME                   TR("Ввід", "Введіть ім'я")
#define TR_EXPONAME                    TR("Ім'я", "Ім'я рядку")
#define TR_BITMAP                      "Зображ. моделі"
#define TR_NO_PICTURE                  "Немає зображ."
#define TR_TIMER                       TR("Таймер", "Таймер ")
#define TR_NO_TIMERS                   "No timers"
#define TR_START                       "Start"	/* use english */
#define TR_NEXT                        "Next"
#define TR_ELIMITS                     TR("Розш.Ліміт", "Розширені Ліміти")
#define TR_ETRIMS                      TR("Розш.Трим.", "Розширене Обрізання")  /* ned to be clarified by context */
#define TR_TRIMINC                     "Крок трим."
#define TR_DISPLAY_TRIMS               TR("Показ. трим.", "Показати трими")
#define TR_TTRACE                      TR("T-Джерело", "Джерело")
#define TR_TTRIM                       TR("T-Трим-Порожн.", "Тримувати тількти порожнє")
#define TR_TTRIM_SW                    TR("T-Трим-Перем.", "Перемикач тримування")
#define TR_BEEPCTR                     TR("Сигн. центр", "Сигнал в центрі")
#define TR_USE_GLOBAL_FUNCS            TR("Глоб.функц.", "Викор. глобальні функції")
#define TR_PROTOCOL                    TR("Проток.", "Протокол")
#define TR_PPMFRAME                    "PPM прийм"
#define TR_REFRESHRATE                 TR("Оновл.", "Частота оновлення")
#define TR_WARN_BATTVOLTAGE           TR("Вихід VBAT: ", "Warning: вихідний рівень VBAT: ")	/* for untranslated - use english */
#define TR_WARN_5VOLTS                 "Warning: вихідний рівень 5 вольт"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Перемикач"
#define TR_FS_COLOR_LIST_1             "Custom"
#define TR_FS_COLOR_LIST_2             "Off"
#define TR_FS_COLOR_LIST_3             "White"
#define TR_FS_COLOR_LIST_4             "Red"
#define TR_FS_COLOR_LIST_5             "Green"
#define TR_FS_COLOR_LIST_6             "Yellow"
#define TR_FS_COLOR_LIST_7             "Orange"
#define TR_FS_COLOR_LIST_8             "Blue"
#define TR_FS_COLOR_LIST_9             "Pink"
#define TR_GROUP                       "Group"
#define TR_GROUP_ALWAYS_ON             "Always on"
#define TR_LUA_OVERRIDE                "Allow Lua override"
#define TR_GROUPS                      "Always on groups"
#define TR_LAST                        "Last"
#define TR_MORE_INFO                   "More info"
#define TR_SWITCH_TYPE                 "Type"
#define TR_SWITCH_STARTUP              "Startup"
#define TR_SWITCH_GROUP                "Group"
#define TR_SF_SWITCH                   "Тригери"
#define TR_TRIMS                       "Трим."
#define TR_FADEIN                      "Вх.Згасання"
#define TR_FADEOUT                     "Вих.Згасання"
#define TR_DEFAULT                     "(замовч.)"
#define   TR_CHECKTRIMS                 TR_BW_COL("\006Перев.\012трим.", "Перевірка FM трим.")
#define TR_SWASHTYPE                   "Тип перекосу"
#define TR_COLLECTIVE                  TR("Колективний", "Колективне джерело тону")
#define TR_AILERON                     TR("Крен", "Крен")
#define TR_ELEVATOR                    TR("Тангаж", "Тангаж")
#define TR_SWASHRING                   "Поворот"
#define TR_MODE                        "Режим"
#define TR_LEFT_STICK                  "Ліво"
#define TR_SUBTYPE                     "Підтип"
#define TR_NOFREEEXPO                  "Немає вільного expo!"
#define TR_NOFREEMIXER                 "Немає вільного мікшеру!"
#define TR_SOURCE                      "Джерело"
#define TR_WEIGHT                      "Вага"
#define TR_SIDE                        "Сторона"
#define TR_OFFSET                      "Зсув"
#define TR_TRIM                        "Тримування"
#define TR_CURVE                       "Крива"
#define TR_FLMODE                      TR("Режим", "Режими")
#define TR_MIXWARNING                  "Warning"		/* use english */
#define TR_OFF                         "OFF"			/* use english */
#define TR_ANTENNA                     "Антена"
#define TR_NO_INFORMATION              TR("Немає інфо", "Немає інформації")
#define TR_MULTPX                      "Multiplex"		/* use english */
#define TR_DELAYDOWN                   TR("Затрим менше", "Затримка менше")
#define TR_DELAYUP                     "Затримка більше"
#define TR_SLOWDOWN                    TR("Пов.", "Повільно")	/*need to be clarified by context*/
#define TR_SLOWUP                      "Повільно"
#define TR_CV                          "CV"
#define TR_GV                          TR("G", "GV")
#define TR_RANGE                       "Діапаз."
#define TR_CENTER                      "Центр"
#define TR_ALARM                       "Тривога"
#define TR_BLADES                      "Леза/Піни"	/*need to be clarified by context*/
#define TR_SCREEN                      "Екран\001"
#define TR_SOUND_LABEL                 "Звук"
#define TR_LENGTH                      "Довжина"
#define TR_BEEP_LENGTH                 "Довжина сигн."
#define TR_BEEP_PITCH                  "Тон сигналу"	/*need to be clarified by context*/
#define TR_HAPTIC_LABEL                "Вібро"
#define TR_STRENGTH                    "Інтенсивність"
#define TR_IMU_LABEL                   "IMU"		/*need to be clarified by context*/
#define TR_IMU_OFFSET                  "Зсув"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Контраст"
#define TR_ALARMS_LABEL                "Тривоги"
#define TR_BATTERY_RANGE               TR("Діапаз. акум.", "Діапазон акумулятора")
#define TR_BATTERYCHARGING             "Заряджання..."
#define TR_BATTERYFULL                 "Batt заряджено"
#define TR_BATTERYNONE                 "Batt відсутня!"
#define TR_BATTERYWARNING              "Batt розрядж."
#define TR_INACTIVITYALARM             "Бездіяльність"
#define TR_MEMORYWARNING               "Пам'ять закінч."
#define TR_ALARMWARNING                "Звук вимк."
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI при вимк.", "Перевіряти RSSI при вимиканнні")		/*need to be clarified by context*/
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Тренер при вимк.", "Перевіряти тренер при вимиканні")
#define TR_MODEL_STILL_POWERED         "Модель досі під живленням"
#define TR_TRAINER_STILL_CONNECTED     "Трен. досі підключено"
#define TR_USB_STILL_CONNECTED         "USB досі підключено"
#define TR_MODEL_SHUTDOWN              "Вимкнути?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Тисни enter для підтвердження"
#define TR_THROTTLE_LABEL              "Газ"
#define TR_THROTTLE_START              "Газ на старт"
#define TR_THROTTLEREVERSE             TR("Газ-реверс", "Реверс")
#define TR_MINUTEBEEP                  TR("Хв.", "Хв. сигнал")
#define TR_BEEPCOUNTDOWN               "Відлік"
#define TR_PERSISTENT                  TR("Присутн.", "Присутній")
#define TR_BACKLIGHT_LABEL             "Підсвітка"
#define TR_GHOST_MENU_LABEL            "Меню Привид"
#define TR_STATUS                      "Статус"
#define TR_BLONBRIGHTNESS              "ON яскравість"
#define TR_BLOFFBRIGHTNESS             "OFF яскравість"
#define TR_KEYS_BACKLIGHT              "Яскравість кнопки"
#define TR_BLCOLOR                     "Колір"
#define TR_SPLASHSCREEN                "Заставка"
#define TR_PLAY_HELLO                  "Звук запуску"
#define TR_PWR_ON_DELAY                "Затримка увімкн."
#define TR_PWR_OFF_DELAY               "Затримка вимкн."
#define TR_PWR_AUTO_OFF                "Автовимкнення"
#define TR_PWR_ON_OFF_HAPTIC           TR("Вібро на живлення","Вібрація на увімкн./вимкн.")
#define TR_THROTTLE_WARNING            TR("Г-попер.", "Стан Газу")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Користув. полож.", "Користувацьке положення?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Полож. %", "Положення %")
#define TR_SWITCHWARNING               TR("Поз. перем.", "Позиції пермикачів")		/*need to be clarified by context*/
#define TR_POTWARNINGSTATE             "Потенціометри & Повзунки"
#define TR_POTWARNING                  TR("Потенц.", "Позиц. потенц.")		/*need to be clarified by context*/
#define TR_TIMEZONE                    "Часовий пояс"
#define TR_ADJUST_RTC                  "Підлаштувати RTC"
#define TR_GPS                         "GPS"		/* use english */
#define TR_DEF_CHAN_ORD                TR("Порядок каналів", "Порядок каналів за замовчуванням")
#define TR_STICKS                      "Вісі"
#define TR_POTS                        "Потенціометри"
#define TR_SWITCHES                    "Перемикачі"
#define TR_SWITCHES_DELAY              TR("Затримка відтвор.", "Затримка відтворення (середнє полож. перем.)")
#define TR_SLAVE                       "Slave"		/* use english */
#define TR_MODESRC                     "Mode\006% Source"		/* use english */
#define TR_MULTIPLIER                  "Множник"
#define TR_CAL                         "Калібрув."
#define TR_CALIBRATION                 BUTTON("Калібрув.")
#define TR_VTRIM                       "Трим - +"
#define TR_CALIB_DONE                  "Калібрування завершене"
#define TR_MENUTOSTART                 TR_ENTER " ДЛЯ СТАРТУ"
#define TR_MENUWHENDONE                TR_ENTER " КОЛИ ЗАКІНЧЕНО"
#define TR_AXISDIR                     "AXIS DIR"
#define TR_MENUAXISDIR                 "[ENTER LONG] "  TR_AXISDIR
#define TR_SETMIDPOINT                 TR_BW_COL(TR_SFC_AIR("ВІДЦЕНТР. ПОТЕНЦ.", TR("ВІДЦЕНТР. ВІСІ", "ВІДЦЕНТР. ВІСІ/ПОВЗУНКИ")), "ЦЕНТРУВАТИ ВАЖЕЛІ/ПОВЗУНКИ")
#define TR_MOVESTICKSPOTS              TR_BW_COL(TR_SFC_AIR("РУХАЙТЕ ST/TH/ПОТЕНЦ./ВІСІ", "РУХАЙТЕ ВІСІ/ПОТЕНЦ."), "РУХАЙТЕ ВІСІ/ПОТЕНЦ.")
#define TR_NODATA                      "НЕМАЄ ДАНИХ"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"		/* use english */
#define TR_FREE_STACK                  "Free stack"		/* use english */
#define TR_INT_GPS_LABEL               "Внутр. GPS"
#define TR_HEARTBEAT_LABEL             "Пульс"
#define TR_LUA_SCRIPTS_LABEL           "Lua скрипт"
#define TR_FREE_MEM_LABEL              "Вільно RAM"
#define TR_DURATION_MS             TR("[D]","Тривалість(мс): ")
#define TR_INTERVAL_MS             TR("[I]","Інтервал(мс): ")
#define TR_MEM_USED_SCRIPT         "Скрипт(B): "
#define TR_MEM_USED_WIDGET         "Віджет(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Аудіо: "
#define TR_GPS_FIX_YES                 "Фіксація: Так"
#define TR_GPS_FIX_NO                  "Фіксація: Ні"
#define TR_GPS_SATS                    "Супутн: "
#define TR_GPS_HDOP                    "Hdop: "		/* use english */
#define TR_STACK_MENU                  "Меню: "
#define TR_TIMER_LABEL                 "Таймер"
#define TR_THROTTLE_PERCENT_LABEL      "Газ %"
#define TR_BATT_LABEL                  "Battery"		/* use english */
#define TR_SESSION                     "Сесія"
#define TR_MENUTORESET                 TR_ENTER " для скидання"
#define TR_PPM_TRAINER                 "TR"		/* use english */
#define TR_CH                          "CH"		/* use english */
#define TR_MODEL                       "МОДЕЛЬ"
#define TR_FM                          TR_SFC_AIR("DM", "FM")
#define TR_EEPROMLOWMEM                "мало пам'яті EEPROM"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Натисн. кнопку щоб пропустити"
#define TR_THROTTLE_NOT_IDLE           "Газ не вільний"
#define TR_ALARMSDISABLED              "Авар.Сингали вимкнені"
#define TR_PRESSANYKEY                 TR("\010Тисни будь-яку кнопку", "Тисни будь-яку кнопку")
#define TR_BADEEPROMDATA               "невірні дані EEPROM"
#define TR_BAD_RADIO_DATA              "Відсутні або погані радіодані"
#define TR_RADIO_DATA_RECOVERED        TR3("Використовую резервні радіодані", "Використовую резервні налаштування радіо", "Налаштування радіо відновлені з резервних даних")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Невірні налаштування радіо", "Невірні налаштування радіо", "Неможливо прочитати правильні налаштування радіо")
#define TR_EEPROMFORMATTING            "Форматую EEPROM"
#define TR_STORAGE_FORMAT              "Підготовка сховища"
#define TR_EEPROMOVERFLOW              "Переповнення EEPROM"
#define TR_RADIO_SETUP                 "НАЛАШТ. АПАРАТУРИ"
#define TR_MENUVERSION                 "ВЕРСІЯ"
#define TR_MENU_RADIO_ANALOGS          TR("АНАЛОГ", "ТЕСТ АНАЛОГУ")
#define TR_MENU_RADIO_ANALOGS_CALIB    "АНАЛОГИ ВІДКАЛІБРОВАНІ"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "НЕОБРОБЛЕНІ АНАЛОГИ (5 Hz)"
#define TR_MENU_FSWITCH                "КОРИСТУВАЦЬКІ ПЕРЕМИКАЧІ"
#define   TR_TRIMS2OFFSETS              TR_BW_COL("\006Трим => Субтрим", "Трим => Субтрим")
#define TR_CHANNELS2FAILSAFE           "Канали=>АварРежим"
#define TR_CHANNEL2FAILSAFE            "Канал=>АварРежим"
#define TR_MENUMODELSEL                "ВИБІР МОДЕЛІ"
#define TR_MENU_MODEL_SETUP            TR("НАЛАШТУВАННЯ", "НАЛАШТУВАННЯ МОДЕЛІ")
#define TR_MENUCURVE                   "КРИВА"
#define TR_MENULOGICALSWITCH           "ЛОГІЧНИЙ ПЕРЕМИКАЧ"
#define TR_MENUSTAT                    "СТАТ."
#define TR_MENUDEBUG                   "ВІДЛАГОДЖЕННЯ"
#define TR_MONITOR_CHANNELS            "МОНІТОР КАН %d-%d"
#define TR_MONITOR_OUTPUT_DESC         "Виходи"
#define TR_MONITOR_MIXER_DESC          "Мікшери"
#define TR_RECEIVER_NUM                TR("RxNum", "No. приймача")
#define TR_RECEIVER                    "Приймач"
#define TR_MULTI_RFTUNE                TR("Налаштув.частоти", "Точне налаштування Радіочастоти")
#define TR_MULTI_RFPOWER               "RF потужність"
#define TR_MULTI_WBUS                  "Вихід"
#define TR_MULTI_TELEMETRY             "Телеметрія"
#define TR_MULTI_VIDFREQ               TR("Від.Част.", "Відео частота")
#define TR_RF_POWER                    "RF Потужність"
#define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")		/* use english */
#define TR_MULTI_OPTION                TR("Опція", "Значення опції")
#define TR_MULTI_AUTOBIND              TR("Прив'язка до кан.", "Прив'язка до каналу")
#define TR_DISABLE_CH_MAP              TR("Викл. карту каналів", "Виключити карту каналів")
#define TR_DISABLE_TELEM               TR("Викл. телеметрії", "Виключити телеметрію")
#define TR_MULTI_LOWPOWER              TR("Низька потужн.", "Режим низької потужності")
#define TR_MULTI_LNA_DISABLE           "Вимкн. LNA"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")		/* use english */
#define TR_MODULE_TELEM_ON             TR("ON", "Увімкнено")
#define TR_DISABLE_INTERNAL            TR("Вимкн. внутр.", "Вимкнути внутрішній RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("Не Serial реж.", "Не в Serial режимі")
#define TR_MODULE_NO_INPUT             TR("Немає входу", "Немає Serial входу")
#define TR_MODULE_NO_TELEMETRY         TR3("Немає телеметрії", "Немає телеметрії", "Телеметрія не визначена")
#define TR_MODULE_WAITFORBIND          "Прив'язка до завантаж. протоколу"
#define TR_MODULE_BINDING              TR("Прив'язка...","Прив'язую")
#define TR_MODULE_UPGRADE_ALERT        TR3("Потрібне оновл.", "Потрібне оновлення модуля", "Потрібне\nОновлення модуля")
#define TR_MODULE_UPGRADE              TR("Рекоменд.Оновл.", "Рекомендоване оновлення модуля")
#define TR_REBIND                      "Потрібна переприв'язка"
#define TR_REG_OK                      "Регістрація успішна"
#define TR_BIND_OK                     "Прив'язка успішна"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Телем ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Телем OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Телем ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Телем OFF"
#define TR_PROTOCOL_INVALID            TR("Прот.невірн.", "Протокол невірний")
#define TR_MODULE_STATUS               TR("Статус", "Статус модуля")
#define TR_MODULE_SYNC                 TR("Синх", "Синхро протоколу")
#define TR_MULTI_SERVOFREQ             TR("Частота серво", "Частота оновлення серво")
#define TR_MULTI_MAX_THROW             TR("Max. Хід", "Увімкн. max. хід")
#define TR_MULTI_RFCHAN                TR("RF Канал", "Обери RF канал")
#define TR_AFHDS3_RX_FREQ              TR("RX част.", "RX частота")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Телем.", "Unicast/Телеметрія")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Факт.потужн.", "Фактична потужність")
#define TR_AFHDS3_POWER_SOURCE         TR("Джерело живл.", "Джерело живлення")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI")		/* use english */
#define TR_GPS_COORDS_FORMAT           TR("Формат коорд.", "Формат координат")
#define TR_VARIO                       TR("Варіо", "Варіометр")
#define TR_PITCH_AT_ZERO               "Тон низ"
#define TR_PITCH_AT_MAX                "Тон верх"
#define TR_REPEAT_AT_ZERO              "Повтор"
#define TR_BATT_CALIB                  TR("Калібр.акум", "Калібрування акумулятора")
#define TR_CURRENT_CALIB               "Калібровка струму"
#define TR_VOLTAGE                     TR("Напруга", "Джерело напруги")
#define TR_SELECT_MODEL                "Оберіть модель"
#define TR_MANAGE_MODELS               "КЕРУВАННЯ МОДЕЛЯМИ"
#define TR_MODELS                      "Моделі"
#define TR_SELECT_MODE                 "Оберіть режим"
#define TR_CREATE_MODEL                "Створіть модель"
#define TR_FAVORITE_LABEL              "Обрані"
#define TR_MODELS_MOVED                "Невикористані моделі переміщено до"
#define TR_NEW_MODEL                   "Нова Модель"
#define TR_INVALID_MODEL               "Невірна Модель"
#define TR_EDIT_LABELS                 "Редагувати Мітки"
#define TR_LABEL_MODEL                 "Мітка моделі"
#define TR_MOVE_UP                     "Перем. вгору"
#define TR_MOVE_DOWN                   "Перем. вниз"
#define TR_ENTER_LABEL                 "Введіть мітку"
#define TR_LABEL                       "Мітка"
#define TR_LABELS                      "Мітки"
#define TR_CURRENT_MODEL               "Поточна"
#define TR_ACTIVE                      "Активна"
#define TR_NEW                         "Нова"
#define TR_NEW_LABEL                   "Нова мітка"
#define TR_RENAME_LABEL                "Перейменувати мітку"
#define TR_DELETE_LABEL                "Видалити мітку"
#define TR_DUPLICATE_MODEL             "Дублювати мітку"
#define TR_COPY_MODEL                  "Копіювати модель"
#define TR_MOVE_MODEL                  "Перемістити модель"
#define TR_BACKUP_MODEL                "Резерв копія моделі"
#define TR_DELETE_MODEL                "Видалити модель"
#define TR_RESTORE_MODEL               "Відновити модель"
#define TR_DELETE_ERROR                "Видалити помилку"
#define TR_SDCARD_ERROR                TR("помилка SD", "помилка SD карти")
#define TR_SDCARD                      "SD Карта"
#define TR_NO_FILES_ON_SD              "Немає файлів на SD!"
#define TR_NO_SDCARD                   "SD карта відсутня"
#define TR_WAITING_FOR_RX              "Чекаємо на RX..."
#define TR_WAITING_FOR_TX              "Чекаємо на TX..."
#define TR_WAITING_FOR_MODULE          TR("Чекаємо на модуль", "Чекаємо на модуль...")
#define TR_NO_TOOLS                    "Немає інструментів"
#define TR_NORMAL                      "Нормально"
#define TR_NOT_INVERTED                "Не інв."
#define TR_NOT_CONNECTED               TR("Не з'єдн.", "Не з'єднано")
#define TR_CONNECTED                   "З'єднано"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH без телем.", "16CH без телеметрії")
#define TR_16CH_WITH_TELEMETRY         TR("16CH з телем.", "16CH з телметрією")
#define TR_8CH_WITH_TELEMETRY          TR("8CH з телем.", "8CH з телметрією")
#define TR_EXT_ANTENNA                 "Зовн. антена"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Оновити опції RX?"
#define TR_UPDATE_TX_OPTIONS           "Оновити опції TX?"
#define TR_MODULES_RX_VERSION          BUTTON("Модулі / RX версії")
#define TR_SHOW_MIXER_MONITORS         "Показати монітори мікшеру"
#define TR_MENU_MODULES_RX_VERSION     "МОДУЛІ / RX ВЕРСІЇ"
#define TR_MENU_FIRM_OPTIONS           "FIRMWARE ОПЦІЇ"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Вісі/Потенц./Повзунки"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Вісі/Потенц./Повзунки"
#define TR_RF_PROTOCOL                 "RF протокол"
#define TR_MODULE_OPTIONS              "Опції модулю"
#define TR_POWER                       "Потужність"
#define TR_NO_TX_OPTIONS               "Немає опцій TX"
#define TR_RTC_BATT                    "RTC Batt"
#define TR_POWER_METER_EXT             "Power Meter (EXT)"
#define TR_POWER_METER_INT             "Power Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Спектр (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Спектр (INT)"
#define TR_GHOST_MODULE_CONFIG         "Ghost module config"
#define TR_GPS_MODEL_LOCATOR           "GPS model locator"
#define TR_REFRESH                     "Refresh"
#define TR_SDCARD_FULL                 "SD карта повна"
#define TR_SDCARD_FULL_EXT              TR_BW_COL(TR_SDCARD_FULL "\036Логи і скріншоти" LCDW_128_LINEBREAK "вимкнені", TR_SDCARD_FULL "\nЛоги і скріншоти вимкнені")
#define TR_NEEDS_FILE                  "ПОТРІБЕН ФАЙЛ"
#define TR_EXT_MULTI_SPEC              "opentx-inv"				/*need to be clarified by context*/
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"				/*need to be clarified by context*/
#define TR_INCOMPATIBLE                "Несумісне"
#define TR_WARNING                     "ПОПЕРЕДЖ."
#define TR_STORAGE_WARNING             "СХОВИЩЕ"
#define TR_THROTTLE_UPPERCASE          "ГАЗ"
#define TR_ALARMSWARN                  "АВАРІЇ"
#define TR_SWITCHWARN                  TR("ПЕРЕМИК.", "УПРАВЛІННЯ")
#define TR_FAILSAFEWARN                "АВАРІЙНИЙ РЕЖИМ"
#define TR_TEST_WARNING                TR("ТЕСТУВАННЯ", "ТЕСТ ПОБУДОВАНО")
#define TR_TEST_NOTSAFE                "Використовувати тільки для тестування"
#define TR_WRONG_SDCARDVERSION         TR("Очікувана вер.: ", "Очікувана версія: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery розряджена"
#define TR_WARN_MULTI_LOWPOWER         "Режим низької потужності"
#define TR_BATTERY                     "BATTERY"
#define TR_WRONG_PCBREV                "Визначено невірну PCB"
#define TR_EMERGENCY_MODE              "НАДЗВИЧАЙНИЙ РЕЖИМ"
#define TR_NO_FAILSAFE                 "Аварійний режим не налаштовано"
#define TR_KEYSTUCK                    "Залипання кнопки"
#define TR_VOLUME                      "Загальна гучн."
#define TR_LCD                         "LCD"		/*use english*/
#define TR_BRIGHTNESS                  "Яскравість"
#define TR_CONTROL                     "Control"
#define TR_CPU_TEMP                    "CPU темп."
#define TR_COPROC                      "Сопроцессор"
#define TR_COPROC_TEMP                 "MB темп."
#define TR_TTL_WARNING                 "Увага: Не перевищуйте 3.3V на пінах TX/RX!"
#define TR_FUNC                        "Функція"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Тривалість"
#define TR_DELAY                       "Затримка"
#define TR_NO_SOUNDS_ON_SD             "Немає звуків на SD"
#define TR_NO_MODELS_ON_SD             "Немає моделей на SD"
#define TR_NO_BITMAPS_ON_SD            "Немає бітмапів на SD"
#define TR_NO_SCRIPTS_ON_SD            "Немає скриптів на SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Синтакс. пом.", "Синтаксична помилка")
#define TR_SCRIPT_PANIC                "Скрипт панікує"
#define TR_SCRIPT_KILLED               "Скрипт вбито"
#define TR_SCRIPT_ERROR                "Невідома помилка"
#define TR_PLAY_FILE                   "Грати"
#define TR_DELETE_FILE                 "Видалити"
#define TR_COPY_FILE                   "Копіювати"
#define TR_RENAME_FILE                 "Перейменувати"
#define TR_ASSIGN_BITMAP               "Призначити зображення"
#define TR_ASSIGN_SPLASH               "Заставка"
#define TR_EXECUTE_FILE                "Виконати"
#define TR_REMOVED                     " видалено"
#define TR_SD_INFO                     "Інформація"
#define TR_NA                          "Н/Д"
#define TR_FORMATTING                  "Форматую..."
#define TR_TEMP_CALIB                  "Калібр. темп."
#define TR_TIME                        "Час"
#define TR_MAXBAUDRATE                 "Max швидкість"
#define TR_BAUDRATE                    "Швидкість"
#define TR_CRSF_ARMING_MODE            "Arm using"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 TR("Вибірка","Режим вибірки")
#define TR_SAMPLE_MODES_1              "Нормальний"
#define TR_SAMPLE_MODES_2              "OneBit"
#define TR_LOADING                     "Завантаження..."
#define TR_DELETE_THEME                "Видалити тему?"
#define TR_SAVE_THEME                  "Зберегти тему?"
#define TR_EDIT_COLOR                  "Редагувати колір"
#define TR_NO_THEME_IMAGE              "Немає зображення теми"
#define TR_BACKLIGHT_TIMER             "Час бездіяльності"

#define TR_MODEL_QUICK_SELECT        "Швидкий вибір моделі"
#define TR_LABELS_SELECT             "Вибір мітки"
#define TR_LABELS_MATCH              "Співпад. мітки"
#define TR_FAV_MATCH                 "Співпад. улюблен."
#define TR_LABELS_SELECT_MODE_1      "Вибір багат."
#define TR_LABELS_SELECT_MODE_2      "Вибір одного"
#define TR_LABELS_MATCH_MODE_1       "Співпад. усіх"
#define TR_LABELS_MATCH_MODE_2       "Співпад. одного"
#define TR_FAV_MATCH_MODE_1          "Обов'язк. співпад."
#define TR_FAV_MATCH_MODE_2          "Необов'язк. співпад."

#define TR_SELECT_TEMPLATE_FOLDER      "Обрати теку моделі"
#define TR_SELECT_TEMPLATE             "ОБЕРІТЬ ШАБЛОН МОДЕЛІ:"
#define TR_NO_TEMPLATES                "В цій теці не знайдено шаблонів моделей"
#define TR_SAVE_TEMPLATE               "Зберегти як шаблон"
#define TR_BLANK_MODEL                 "Порожня модель"
#define TR_BLANK_MODEL_INFO            "Створити порожню модель"
#define TR_FILE_EXISTS                 "ФАЙЛ ІСНУЄ"
#define TR_ASK_OVERWRITE               "Бажаєш перезаписати?"

#define TR_BLUETOOTH                   "Bluetooth"		/*use english*/
#define TR_BLUETOOTH_DISC              "Визначити"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Адр. призначення"
#define TR_BLUETOOTH_LOCAL_ADDR        "Місцева адр."
#define TR_BLUETOOTH_PIN_CODE          "PIN код"
#define TR_BLUETOOTH_NODEVICES         "Пристрій Не Знайдено"
#define TR_BLUETOOTH_SCANNING          "Сканування..."
#define TR_BLUETOOTH_BAUDRATE          "Швидкість BT"
#define TR_BLUETOOTH_MODES_1           "---"
#define TR_BLUETOOTH_MODES_2           "Телеметрія"
#define TR_BLUETOOTH_MODES_3           "Тренер"
#define TR_BLUETOOTH_MODES_4           "Активовано"

#define TR_SD_INFO_TITLE               "SD інфо"
#define TR_SD_SPEED                    "Швидкість:"
#define TR_SD_SECTORS                  "Сектори:"
#define TR_SD_SIZE                     "Розмір:"
#define TR_TYPE                        "Тип"
#define TR_GVARS                       "Г.ЗМ."
#define TR_GLOBAL_VAR                  "Глобальні змінні"
#define TR_OWN                         "Свій"
#define TR_DATE                        "Дата"
#define TR_MONTHS_1                    "Січ"
#define TR_MONTHS_2                    "Лют"
#define TR_MONTHS_3                    "Бер"
#define TR_MONTHS_4                    "Кві"
#define TR_MONTHS_5                    "Тра"
#define TR_MONTHS_6                    "Чер"
#define TR_MONTHS_7                    "Лип"
#define TR_MONTHS_8                    "Сер"
#define TR_MONTHS_9                    "Вер"
#define TR_MONTHS_10                   "Жов"
#define TR_MONTHS_11                   "Лис"
#define TR_MONTHS_12                   "Гру"
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("Режим енкодера","Режим обертального енкодера")
#define TR_CHANNELS_MONITOR            "МОНІТОР КАНАЛІВ"
#define TR_MIXERS_MONITOR              "МОНІТОР МІКШЕРІВ"
#define TR_PATH_TOO_LONG               "Шлях задовгий"
#define TR_VIEW_TEXT                   "Дивитися текст"
#define TR_FLASH_BOOTLOADER            "Прошити завантаж."
#define TR_FLASH_DEVICE                TR("Прошити пристрій","Прошити пристрій")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Прошити посл.порт", "Прошити пристрій посл.порт.")
#define TR_FLASH_RECEIVER_OTA          "Пришити приймач OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Прошити RX зовн. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Прошити RX внутр. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Прошити FC зовн. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Прошити FC внутр. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Прошити BT модуль", "Прошити Bluetooth модуль")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Прошити пристрій управління живленням"
#define TR_DEVICE_NO_RESPONSE          TR("Пристрій не відповідає", "Пристрій не відповідає")
#define TR_DEVICE_FILE_ERROR           TR("Проблема з файлом пристрою", "Проблема з файлом пристрою")
#define TR_DEVICE_DATA_REFUSED         TR("Дані пристрою відхилено", "Дані пристрою відхилено")
#define TR_DEVICE_WRONG_REQUEST        TR("Проблема доступу до пристрою", "Проблема доступу до пристрою")
#define TR_DEVICE_FILE_REJECTED        TR("Файл пристрою відхилено", "Файл пристрою відхилено")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Невірний підпис файлу", "Невірний підпис файлу")
#define TR_CURRENT_VERSION             TR("Поточна версія: ", "Поточна версія: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Прошити внутр модуль", "Прошити внутр модуль")
#define TR_FLASH_INTERNAL_MULTI        TR("Прошити внутр. Multi", "Прошити внутрішній Multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Прошити зовн. модуль", "Прошити зовнішній модуль")
#define TR_FLASH_EXTERNAL_MULTI        TR("Прошити зовн. Multi", "Прошити зовнішній Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Прошити зовн. ELRS", "Прошити зовнішній ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Помилка оновлення FW", "Помилка оновлення Firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Прошивка успішна"
#define TR_WRITING                     "Запис..."
#define TR_CONFIRM_FORMAT              "Підтвердити формат?"
#define TR_INTERNALRF                  "Внутрійшній RF"
#define TR_INTERNAL_MODULE             TR("Внутр. модуль", "Внутрішній модуль")
#define TR_EXTERNAL_MODULE             TR("Зовн. модуль", "Зовнішній модуль")
#define TR_EDGETX_UPGRADE_REQUIRED     "Потрібне оновлення EdgeTX"
#define TR_TELEMETRY_DISABLED          "Телем. вимкнуто"
#define TR_MORE_OPTIONS_AVAILABLE      "Доступні додаткові опції"
#define TR_NO_MODULE_INFORMATION       "Інформація по модулю відсутня"
#define TR_EXTERNALRF                  "Зовнішній RF"
#define TR_FAILSAFE                    TR("Відмова", "Режим відмови")
#define TR_FAILSAFESET                 "НАЛАШТУВАННЯ АВАРІЙНОГО РЕЖИМУ"
#define TR_REG_ID                      "Рег. ID"
#define TR_OWNER_ID                    "ID користув."
#define TR_HOLD                        "Утримання"
#define TR_HOLD_UPPERCASE              "УТРИМАННЯ"
#define TR_NONE                        "Немає"
#define TR_NONE_UPPERCASE              "НЕМАЄ"
#define TR_MENUSENSOR                  "СЕНСОР"
#define TR_POWERMETER_PEAK             "Пікове"
#define TR_POWERMETER_POWER            "Потужність"
#define TR_POWERMETER_ATTN             "Атт"
#define TR_POWERMETER_FREQ             "Част."
#define TR_MENUTOOLS                   "ІНСТРУМЕНТИ"
#define TR_TURN_OFF_RECEIVER           "Вимкнути приймач"
#define TR_STOPPING                    "Зупинка..."
#define TR_MENU_SPECTRUM_ANALYSER      "АНАЛІЗАТОР СПЕКТРУ"
#define TR_MENU_POWER_METER            "ВИМІРЮВАЧ ПОТУЖНОСТІ"
#define TR_SENSOR                      "СЕНСОР"
#define TR_COUNTRY_CODE                "Код країни"
#define TR_USBMODE                     "Режим USB"
#define TR_JACK_MODE                   "Режим коннектора"
#define TR_VOICE_LANGUAGE              "Мова голосу"
#define TR_UNITS_SYSTEM                "Одиниці"
#define TR_UNITS_PPM                   "PPM одиниці"
#define TR_EDIT                        "Редагувати"
#define TR_INSERT_BEFORE               "Вставка до"
#define TR_INSERT_AFTER                "Вставка після"
#define TR_COPY                        "Копіювати"
#define TR_MOVE                        "Перемістити"
#define TR_PASTE                       "Вставити"
#define TR_PASTE_AFTER                 "Вставити після"
#define TR_PASTE_BEFORE                "Вставити до"
#define TR_DELETE                      "Видалити"
#define TR_INSERT                      "Вставити"
#define TR_RESET_FLIGHT                "Скинути сесію"
#define TR_RESET_TIMER1                "Скинути таймер1"
#define TR_RESET_TIMER2                "Скинути таймер2"
#define TR_RESET_TIMER3                "Скинути таймер3"
#define TR_RESET_TELEMETRY             "Скинути телеметрію"
#define TR_STATISTICS                  "Статистика"
#define TR_ABOUT_US                    "Про..."
#define TR_USB_JOYSTICK                "USB Джойстик (HID)"
#define TR_USB_MASS_STORAGE            "USB Сховище (SD)"
#define TR_USB_SERIAL                  "USB Посл. порт (VCP)"
#define TR_SETUP_SCREENS               "Налаштування екранів"
#define TR_MONITOR_SCREENS             "Монітори"
#define TR_AND_SWITCH                  "AND switch"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Калібровані аналоги"
#define TR_ANADIAGS_FILTRAWDEV         "Фільтровані сирі аналоги з відхиленням"
#define TR_ANADIAGS_UNFILTRAW          "Неформатовані сирі аналоги"
#define TR_ANADIAGS_MINMAX             "Min., max. і діапазон"
#define TR_ANADIAGS_MOVE               "Перемістити аналоги до екстремумів!"
#define TR_BYTES                       "байти"
#define TR_MODULE_BIND                 BUTTON(TR("Прив.", "Прив'язати"))
#define TR_MODULE_UNBIND               BUTTON("Відв'язати")
#define TR_POWERMETER_ATTN_NEEDED     "Потрібен атенюатор"
#define TR_PXX2_SELECT_RX              "Оберіть RX"
#define TR_PXX2_DEFAULT                "<замовч>"
#define TR_BT_SELECT_DEVICE            "Оберіть пристрій"
#define TR_DISCOVER                    BUTTON("Виявити")
#define TR_BUTTON_INIT                 BUTTON("Запуск")
#define TR_WAITING                     "Очікуємо..."
#define TR_RECEIVER_DELETE             "Видалити приймач?"
#define TR_RECEIVER_RESET              "Скинути приймач?"
#define TR_SHARE                       "Поділитися"
#define TR_BIND                        "Прив'язати"
#define TR_REGISTER                    BUTTON(TR("Регі.", "Регістр"))
#define TR_MODULE_RANGE                BUTTON(TR("Діап.", "Діапазон"))
#define TR_RANGE_TEST                  "Тест діапазону"
#define TR_RECEIVER_OPTIONS            TR("ОПЦІЇ ПРМ.", "ОПЦІЇ ПРИЙМАЧА")
#define TR_RESET_BTN                   BUTTON("Скин.")
#define TR_KEYS_BTN                    BUTTON("Кнопк")
#define TR_ANALOGS_BTN                 BUTTON(TR("Анл.", "Аналоги"))
#define TR_FS_BTN                      BUTTON(TR("Користув.перемик.", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Сенсорний екран не знайдено"
#define TR_TOUCH_EXIT                  "Натисніть на екран щоб вийти"
#define TR_SET                         BUTTON("Встановл.")
#define TR_TRAINER                     "Тренер"
#define TR_CHANS                       "Канали"
#define TR_ANTENNAPROBLEM              "Помилка TX антени!"
#define TR_MODELIDUSED                 "ID використано в:"
#define TR_MODELIDUNIQUE               "ID унікальний"
#define TR_MODULE                      "Модуль"
#define TR_RX_NAME                     "Ім'я Rx"
#define TR_TELEMETRY_TYPE              TR("Тип", "Тип телеметрії")
#define TR_TELEMETRY_SENSORS           "Сенсори"
#define TR_VALUE                       "Значення"
#define TR_PERIOD                      "Період"
#define TR_INTERVAL                    "Інтервал"
#define TR_REPEAT                      "Повтор"
#define TR_ENABLE                      "Активувати"
#define TR_DISABLE                     "Деактивувати"
#define TR_TOPLCDTIMER                 "Верхній таймер LCD"
#define TR_UNIT                        "Одиниця"
#define TR_TELEMETRY_NEWSENSOR         "Додати новий"
#define TR_CHANNELRANGE                TR("Діап. каналів", "Діапазон каналів")
#define TR_ANTENNACONFIRM1             "ЗОВН. АНТЕНА"
#define TR_ANTENNA_MODES_1           "Внутрішній"
#define TR_ANTENNA_MODES_2           "Запит"
#define TR_ANTENNA_MODES_3           "На модель"
#define TR_ANTENNA_MODES_4           "Внутрійшній + Зовнішній"
#define TR_ANTENNA_MODES_5           "Зовнішній"
#define TR_USE_INTERNAL_ANTENNA        TR("Вик. внурт. антену", "Використай внутрішню антену")
#define TR_USE_EXTERNAL_ANTENNA        TR("Вик. зовн. антену", "Використай зовнішню антену")
#define TR_ANTENNACONFIRM2             TR("Перевір антену", "Впевнись, що антену встановлено!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Потрібно не FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Потрібно FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Потрібно EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "сертифікований firmware"
#define TR_LOWALARM                    "Некритична аварія"
#define TR_CRITICALALARM               "Критична аварія"
#define TR_DISABLE_ALARM               TR("Вимкнути аварії", "Вимкнути аварії телеметрії")
#define TR_POPUP                       "Всплив."
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Передвстановлення..."
#define TR_PRESET                      "Передвстановлення"
#define TR_MIRROR                      "Дзеркало"
#define TR_CLEAR                       "Очистити"
#define TR_CLEAR_BTN                   BUTTON("Очистити")
#define TR_RESET                       "Скидання"
#define TR_RESET_SUBMENU               "Скидання..."
#define TR_COUNT                       "Кількість"
#define TR_PT                          "шт"
#define TR_PTS                         "штк"
#define TR_SMOOTH                      "Плавно"
#define TR_COPY_STICKS_TO_OFS          TR("Коп.вісі->обріз.", "Копіювати вісі до обрізання")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Коп min/max до всіх",  "Копіювати min/max/center до всіх виходів")
#define TR_COPY_TRIMS_TO_OFS           TR("Коп трим->субтрим", "Копіювати трим до субтрим")
#define TR_INCDEC                      "Збільш/Зменш"
#define TR_GLOBALVAR                   "Глобальна змінна"
#define TR_MIXSOURCE                   "Джерело (%)"
#define TR_MIXSOURCERAW                "Джерело (Значення)"
#define TR_CONSTANT                    "Постійна"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1 "OFF"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2 "ON"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3 "AUTO"
#define TR_PREFLIGHT                   "Стартові перевірки"
#define TR_CHECKLIST                   TR("Чекліст", "Чекліст Дисплею")
#define TR_CHECKLIST_INTERACTIVE       TR3("Інт. спис.", "Інтеракт. список", "Інтерактивний список")
#define TR_AUX_SERIAL_MODE             "Послідовний порт"
#define TR_AUX2_SERIAL_MODE            "Послідовний порт 2"
#define TR_AUX_SERIAL_PORT_POWER       "Порт живлення"
#define TR_SCRIPT                      "Скрипт"
#define TR_INPUTS                      "Входи"
#define TR_OUTPUTS                     "Виходи"
#define TR_CONFIRMRESET                TR("Стерти все", "Стерти налаштування всіх моделей?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Забагато Lua скриптів!"
#define TR_SPORT_UPDATE_POWER_MODE     "Режим живлення SP"
#define TR_SPORT_UPDATE_POWER_MODES_1  "AUTO"
#define TR_SPORT_UPDATE_POWER_MODES_2  "ON"
#define TR_NO_TELEMETRY_SCREENS        "Екрани Телеметрії Відсутні"
#define TR_TOUCH_PANEL                 "Сенсорна панель:"
#define TR_FILE_SIZE                   "Розмір файлу"
#define TR_FILE_OPEN                   "Відкрити попри все?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Ім'я"
#define TR_PHASES_HEADERS_SW           "Перемикач"
#define TR_PHASES_HEADERS_RUD_TRIM     "Тримування керма"
#define TR_PHASES_HEADERS_ELE_TRIM     "Трим висоти"
#define TR_PHASES_HEADERS_THT_TRIM     "Трим газу"
#define TR_PHASES_HEADERS_AIL_TRIM     "Трим елеронів"
#define TR_PHASES_HEADERS_CH5_TRIM     "Трим 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Трим 6"
#define TR_PHASES_HEADERS_FAD_IN       "Плавно In"
#define TR_PHASES_HEADERS_FAD_OUT      "Плавно Out"

#define TR_LIMITS_HEADERS_NAME         "Ім'я"
#define TR_LIMITS_HEADERS_SUBTRIM      "Субтрим"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Напрямок"
#define TR_LIMITS_HEADERS_CURVE        "Крива"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Центр"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Режим субтрим"
#define TR_INVERTED                    "Інверс."

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "Перший канал"
#define TR_FILL_BACKGROUND           "Заповнити фон?"
#define TR_BG_COLOR                  "Колір фону"
#define TR_SLIDERS_TRIMS             "Повзунки+трими"
#define TR_SLIDERS                   "Повзунки"
#define TR_FLIGHT_MODE               "Режим польоту"
#define TR_INVALID_FILE              "Невірний файл"
#define TR_TIMER_SOURCE              "Джерело таймеру"
#define TR_SIZE                      "Розмір"
#define TR_SHADOW                    "Тінь"
#define TR_ALIGNMENT                 "Вирівнювання"
#define TR_ALIGN_LABEL               "Вирівняти мітку"
#define TR_ALIGN_VALUE               "Вирівняти значення"
#define TR_ALIGN_OPTS_1              "Ліво"
#define TR_ALIGN_OPTS_2              "Центр"
#define TR_ALIGN_OPTS_3              "Право"
#define TR_TEXT                      "Текст"
#define TR_COLOR                     "Колір"
#define TR_PANEL1_BACKGROUND         "Панель1 фон"
#define TR_PANEL2_BACKGROUND         "Панель2 фон"
#define TR_PANEL_BACKGROUND          "Background"
#define TR_PANEL_COLOR               "  Color"
#define TR_WIDGET_GAUGE              "Шкала"
#define TR_WIDGET_MODELBMP           "Інфо про модель"
#define TR_WIDGET_OUTPUTS            "Виходи"
#define TR_WIDGET_TEXT               "Текст"
#define TR_WIDGET_TIMER              "Таймер"
#define TR_WIDGET_VALUE              "Значення"

// About screen
#define TR_ABOUTUS                     TR(" ПРО... ", "ПРО...")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Гучн. сигналу"
#define TR_WAV_VOLUME                  "Гучність Wav"
#define TR_BG_VOLUME                   TR("Фонова гучн.", "Фонова гучність")

#define TR_TOP_BAR                     "Верхня панель"
#define TR_FLASH_ERASE                 "Стирання Flash..."
#define TR_FLASH_WRITE                 "Запис Flash..."
#define TR_OTA_UPDATE                  "Оновлення OTA..."
#define TR_MODULE_RESET                "Скидання модулю..."
#define TR_UNKNOWN_RX                  "Невідомий RX"
#define TR_UNSUPPORTED_RX              "Несумісний RX"
#define TR_OTA_UPDATE_ERROR            "Помилка оновлення OTA"
#define TR_DEVICE_RESET                "Скидання пристрою..."
#define TR_ALTITUDE                    "Висота"
#define TR_SCALE                       "Масштаб"
#define TR_VIEW_CHANNELS               "Перегляд каналів"
#define TR_VIEW_NOTES                  "Перегляд нотаток"
#define TR_MODEL_SELECT                "Вибір моделі"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Точність"
#define TR_RATIO                       "Коефіцієнт"
#define TR_FORMULA                     "Формула"
#define TR_CELLINDEX                   "Номер комірки"
#define TR_LOGS                        "Логи"
#define TR_OPTIONS                     "Опції"
#define TR_FIRMWARE_OPTIONS            BUTTON("Опції Firmware")

#define TR_ALTSENSOR                   "Сенсор висоти"
#define TR_CELLSENSOR                  "Сенсор комірки"
#define TR_GPSSENSOR                   "GPS сенсор"
#define TR_CURRENTSENSOR               "Сенсор"
#define TR_AUTOOFFSET                  "Авто зсув"
#define TR_ONLYPOSITIVE                "Позитив"
#define TR_FILTER                      "Фільтр"
#define TR_TELEMETRYFULL               TR("Всі слоти повні!", "Всі слоти телеметрії повні!")
#define TR_IGNORE_INSTANCE             TR("Немає інстанса.", "Ігнорувати інстанси")
#define TR_SHOW_INSTANCE_ID            "Показати інстанс ID"
#define TR_DISCOVER_SENSORS            "Пошук нового"
#define TR_STOP_DISCOVER_SENSORS       "Stop"
#define TR_DELETE_ALL_SENSORS          "Видалити всі"
#define TR_CONFIRMDELETE               "Дійсно " LCDW_128_LINEBREAK "видалити всі ?"
#define TR_SELECT_WIDGET               "Обрати віджет"
#define TR_WIDGET_FULLSCREEN           "На весь екран"
#define TR_REMOVE_WIDGET               "Видалити віджет"
#define TR_WIDGET_SETTINGS             "Опції віджету"
#define TR_REMOVE_SCREEN               "Видалити екран"
#define TR_SETUP_WIDGETS               "Налаштування віджетів"
#define TR_THEME                       "Тема"
#define TR_SETUP                       "Встановлення"
#define TR_LAYOUT                      "Шар"
#define TR_TEXT_COLOR                  "Колір тексту"
#define TR_MENU_INPUTS                 CHAR_INPUT "Входи"
#define TR_MENU_LUA                    CHAR_LUA "Lua скрипти"
#define TR_MENU_STICKS                 CHAR_STICK "Вісі"
#define TR_MENU_POTS                   CHAR_POT "Потенц."
#define TR_MENU_MIN                    CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   CHAR_CYC "Циклічно"
#define TR_MENU_TRIMS                  CHAR_TRIM "Трими"
#define TR_MENU_SWITCHES               CHAR_SWITCH "Перемикачі"
#define TR_MENU_LOGICAL_SWITCHES       CHAR_SWITCH "Логічні перемикачі"
#define TR_MENU_TRAINER                CHAR_TRAINER "Тренер"
#define TR_MENU_CHANNELS               CHAR_CHANNEL "Канали"
#define TR_MENU_GVARS                  CHAR_SLIDER "ГлобЗмін"
#define TR_MENU_TELEMETRY              CHAR_TELEMETRY "Телеметрія"
#define TR_MENU_DISPLAY                "ДИСПЛЕЙ"
#define TR_MENU_OTHER                  "Інше"
#define TR_MENU_INVERT                 "Інверт"
#define TR_AUDIO_MUTE                  TR("Аудіо стоп","Тихо якщо немає звуку")
#define TR_JITTER_FILTER               "Фільтр ADC"
#define TR_DEAD_ZONE                   "Мертва зона"
#define TR_RTC_CHECK                   TR("Перевір RTC", "Перевір напругу RTC")
#define TR_AUTH_FAILURE                "Помилка авторизації"
#define TR_RACING_MODE                 "Режим гонки"

#define TR_USE_THEME_COLOR              "Використ. колір теми"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Додати всі трими до субтримів"
#define TR_DUPLICATE                    "Дублювати"
#define TR_ACTIVATE                     "Встановити активним"
#define TR_RED                          "Червоний"
#define TR_BLUE                         "Синій"
#define TR_GREEN                        "Зелений"
#define TR_COLOR_PICKER                 "Обирач кольору"
#define TR_FIXED                        "Fixed"
#define TR_EDIT_THEME_DETAILS           "Редагувати деталі теми"
#define TR_THEME_COLOR_DEFAULT         "ЗАМОВЧ"
#define TR_THEME_COLOR_PRIMARY1        "ОСНОВНИЙ1"
#define TR_THEME_COLOR_PRIMARY2        "ОСНОВНИЙ2"
#define TR_THEME_COLOR_PRIMARY3        "ОСНОВНИЙ3"
#define TR_THEME_COLOR_SECONDARY1      "ВТОРИННИЙ1"
#define TR_THEME_COLOR_SECONDARY2      "ВТОРИННИЙ2"
#define TR_THEME_COLOR_SECONDARY3      "ВТОРИННИЙ3"
#define TR_THEME_COLOR_FOCUS           "ФОКУС"
#define TR_THEME_COLOR_EDIT            "РЕДАГУВАТИ"
#define TR_THEME_COLOR_ACTIVE          "АКТИВНИЙ"
#define TR_THEME_COLOR_WARNING         "WARNING"
#define TR_THEME_COLOR_DISABLED        "ДЕАКТИВОВАНИЙ"
#define TR_THEME_COLOR_CUSTOM          "КОРИСТУВАЦЬКИЙ"
#define TR_THEME_CHECKBOX              "Checkbox"
#define TR_THEME_ACTIVE                "Активний"
#define TR_THEME_REGULAR               "Регулярний"
#define TR_THEME_WARNING               "Warning"
#define TR_THEME_DISABLED              "Деактивований"
#define TR_THEME_EDIT                  "Редагувати"
#define TR_THEME_FOCUS                 "Фокус"
#define TR_AUTHOR                      "Автор"
#define TR_DESCRIPTION                 "Опис"
#define TR_SAVE                        "Зберегти"
#define TR_CANCEL                      "Відміна"
#define TR_EDIT_THEME                  "РЕДАГУВАТИ ТЕМУ"
#define TR_DETAILS                     "Деталі"

// Voice in native language
#define TR_VOICE_ENGLISH                "Англійська"
#define TR_VOICE_CHINESE                "Китайська"
#define TR_VOICE_CZECH                  "Чеська"
#define TR_VOICE_DANISH                 "Данська"
#define TR_VOICE_DEUTSCH                "Німецька"
#define TR_VOICE_DUTCH                  "Голандська"
#define TR_VOICE_ESPANOL                "Іспанська"
#define TR_VOICE_FINNISH                "Finnish"
#define TR_VOICE_FRANCAIS               "Француз."
#define TR_VOICE_HUNGARIAN              "Угорська"
#define TR_VOICE_ITALIANO               "Італійська"
#define TR_VOICE_POLISH                 "Польська"
#define TR_VOICE_PORTUGUES              "Португал."
#define TR_VOICE_RUSSIAN                "Pосійська"
#define TR_VOICE_SLOVAK                 "Словацька"
#define TR_VOICE_SWEDISH                "Шведська"
#define TR_VOICE_TAIWANESE              "Тайська"
#define TR_VOICE_JAPANESE               "Японська"
#define TR_VOICE_HEBREW                 "Іврит"
#define TR_VOICE_UKRAINIAN              "Українська"
#define TR_VOICE_KOREAN                 "корейська"

#define TR_USBJOYSTICK_LABEL           "USB Джойстик"
#define TR_USBJOYSTICK_EXTMODE         "Режим"
#define TR_VUSBJOYSTICK_EXTMODE_1      "Класичний"
#define TR_VUSBJOYSTICK_EXTMODE_2      "Просунутий"
#define TR_USBJOYSTICK_SETTINGS        BUTTON("Налаштування каналу")
#define TR_USBJOYSTICK_IF_MODE         TR("Режим ІФ","Режим інтерфейсу")
#define TR_VUSBJOYSTICK_IF_MODE_1      "Джойстик"
#define TR_VUSBJOYSTICK_IF_MODE_2      "Геймпад"
#define TR_VUSBJOYSTICK_IF_MODE_3      "MultiAxis"
#define TR_USBJOYSTICK_CH_MODE         "Режим"
#define TR_VUSBJOYSTICK_CH_MODE_1      "Немає"
#define TR_VUSBJOYSTICK_CH_MODE_2      "Btn"
#define TR_VUSBJOYSTICK_CH_MODE_3      "Axis"
#define TR_VUSBJOYSTICK_CH_MODE_4      "Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S_1    "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2    "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3    "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4    "S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Режим кнопок"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1   "Нормальн"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2   "Імпульс"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3   "SWEmu"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4   "Дельта"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5   "Компаньон"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1 TR("Норм","Нормальний")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2 TR("Імп","Імпульс")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3 TR("SWEm","SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4 TR("Дельта","Дельта")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5 TR("КПН","Компаньйон")
#define TR_USBJOYSTICK_CH_SWPOS        "Позиції"
#define TR_VUSBJOYSTICK_CH_SWPOS_1     "Push"
#define TR_VUSBJOYSTICK_CH_SWPOS_2     "2POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_3     "3POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_4     "4POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_5     "5POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_6     "6POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_7     "7POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_8     "8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Вісь"
#define TR_VUSBJOYSTICK_CH_AXIS_1      "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2      "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3      "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4      "rotX"
#define TR_VUSBJOYSTICK_CH_AXIS_5      "rotY"
#define TR_VUSBJOYSTICK_CH_AXIS_6      "rotZ"
#define TR_VUSBJOYSTICK_CH_AXIS_7      "Повзунок"
#define TR_VUSBJOYSTICK_CH_AXIS_8      "Dial"
#define TR_VUSBJOYSTICK_CH_AXIS_9      "Колесо"
#define TR_USBJOYSTICK_CH_SIM          "Сим. вісі"
#define TR_VUSBJOYSTICK_CH_SIM_1       "Ail"
#define TR_VUSBJOYSTICK_CH_SIM_2       "Ele"
#define TR_VUSBJOYSTICK_CH_SIM_3       "Rud"
#define TR_VUSBJOYSTICK_CH_SIM_4       "Thr"
#define TR_VUSBJOYSTICK_CH_SIM_5       "Acc"
#define TR_VUSBJOYSTICK_CH_SIM_6       "Brk"
#define TR_VUSBJOYSTICK_CH_SIM_7       "Steer"
#define TR_VUSBJOYSTICK_CH_SIM_8       "Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Інверсія"
#define TR_USBJOYSTICK_CH_BTNNUM       "Номер Кнопки"
#define TR_USBJOYSTICK_BTN_COLLISION   "!Конфлікт номерів кнопок!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Конфлікт осей!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Круговий виріз", "Круговий виріз")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1 "Немає"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2 "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3 "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4 "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   BUTTON("Застосувати зміни")

#define TR_DIGITAL_SERVO          "Серво 333HZ"
#define TR_ANALOG_SERVO           "Серво 50HZ"
#define TR_SIGNAL_OUTPUT          "Вихід сигналу"
#define TR_SERIAL_BUS             "Послід. шина"
#define TR_SYNC                   "Синхр"

#define TR_ENABLED_FEATURES       "Увімкнені функції"
#define TR_RADIO_MENU_TABS        "Вкладки меню апаратури"
#define TR_MODEL_MENU_TABS        "Вкладки меню моделей"

#define TR_SELECT_MENU_ALL        "Все"
#define TR_SELECT_MENU_CLR        "Очистити"
#define TR_SELECT_MENU_INV        "Інвертувати"

#define TR_SORT_ORDERS_1          "Назва A-Z"
#define TR_SORT_ORDERS_2          "Назва Z-A"
#define TR_SORT_ORDERS_3          "Найменш використ."
#define TR_SORT_ORDERS_4          "Найбільш використ."
#define TR_SORT_MODELS_BY         "Сортувати моделі за"
#define TR_CREATE_NEW             "Створити"

#define TR_MIX_SLOW_PREC          TR("Точн. уповільн.", "Точність уповільнення")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "A theme directory with the same name already exists."

#define TR_DATE_TIME_WIDGET       "Дата & Час"
#define TR_RADIO_INFO_WIDGET      "Інформація про пульт"
#define TR_LOW_BATT_COLOR         "Розряджена батарея"
#define TR_MID_BATT_COLOR         "Середній заряд батареї"
#define TR_HIGH_BATT_COLOR        "Заряджена батарея"

#define TR_WIDGET_SIZE            "Розмір віджета"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
