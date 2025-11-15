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

// RU translations author: @kobakirill <blokhin.kirill@yandex.ru>

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
#define TR_MAIN_MENU_MANAGE_MODELS      "Меню Моделей"
#define TR_MAIN_MENU_MODEL_NOTES        "Заметки Модели"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Монитор Каналов"
#define TR_MONITOR_SWITCHES            "МОНИТОР ЛОГИЧ ТУМБЛЕРОВ"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Меню Модели"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Меню Пульта"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "UI Setup"
#define TR_MAIN_MENU_STATISTICS         "Стат-ка Пульта"
#define TR_MAIN_MENU_ABOUT_EDGETX       "Инфо EdgeTX"
#define TR_MAIN_VIEW_X                  "Screen "
#define TR_MAIN_MENU_THEMES                 "ТЕМЫ"
#define TR_MAIN_MENU_APPS               "Apps"
#define TR_MENUHELISETUP               "НАСТРОЙКА ВЕРТ"
#define TR_MENUFLIGHTMODES               TR_SFC_AIR("РЕЖИМЫ ВОЖДЕНИЯ", "РЕЖИМЫ ПОЛЕТА")
#define TR_MENUFLIGHTMODE                TR_SFC_AIR("РЕЖИМ ВОЖДЕНИЯ", "РЕЖИМ ПОЛЕТА")
#define TR_MENUINPUTS                "ВХОДЫ"
#define TR_MENULIMITS                "ВЫХОДЫ"
#define TR_MENUCURVES                  "КРИВЫЕ"
#define TR_MIXES                       "МИКСЫ"
#define TR_MENU_GLOBAL_VARS            "Глоб переменные"
#define TR_MENULOGICALSWITCHES         "ЛОГИЧ ТУМБЛ."
#define TR_MENUCUSTOMFUNC              "СПЕЦ ФУНКЦИИ"
#define TR_MENUCUSTOMSCRIPTS           "ПОЛЬЗ СЦЕНАРИИ"
#define TR_MENUTELEMETRY               "ТЕЛЕМЕТРИЯ"
#define TR_MENUSPECIALFUNCS            "ГЛОБ ФУНКЦИИ"
#define TR_MENUTRAINER                 "ТРЕНЕР"
#define TR_HARDWARE                    "АППАРАТУРА"
#define TR_USER_INTERFACE               "Top Bar"
#define TR_SD_CARD                     "SD КАРТА"
#define TR_DEBUG                       "Отладка"
#define TR_MENU_RADIO_SWITCHES         TR("ТУМБЛЕРЫ", "ТЕСТ ТУМБЛЕРОВ")
#define TR_MENUCALIBRATION             "КАЛИБРОВКА"
#define TR_FUNCTION_SWITCHES           "Настр тумблеры"
// End Main menu

#define TR_MINUTE_SINGULAR            "МИНУТА"
#define TR_MINUTE_PLURAL1             "Мин"
#define TR_MINUTE_PLURAL2             "Мин"

#define TR_OFFON_1                     "Выкл"
#define TR_OFFON_2                     "Вкл"
#define TR_MMMINV_1                    "---"
#define TR_MMMINV_2                    "Инв"
#define TR_VBEEPMODE_1                 "Тихо"
#define TR_VBEEPMODE_2                 "Сигн"
#define TR_VBEEPMODE_3                 "Без клав"
#define TR_VBEEPMODE_4                 "Все"
#define TR_VBLMODE_1                   "Выкл"
#define TR_VBLMODE_2                   "Клав"
#define TR_VBLMODE_3                   TR("Стики","Стики")
#define TR_VBLMODE_4                   "Двойн"
#define TR_VBLMODE_5                   "Вкл"
#define TR_TRNMODE_1                   "Вык"
#define TR_TRNMODE_2                   TR("+=","Добав")
#define TR_TRNMODE_3                   TR(":=","Замен")
#define TR_TRNCHN_1                    "КН1"
#define TR_TRNCHN_2                    "КН2"
#define TR_TRNCHN_3                    "КН3"
#define TR_TRNCHN_4                    "КН4"

#define TR_AUX_SERIAL_MODES_1          "Выкл"
#define TR_AUX_SERIAL_MODES_2          "Зерк телем"
#define TR_AUX_SERIAL_MODES_3          "Телем вход"
#define TR_AUX_SERIAL_MODES_4          "SBUS Тренаж"
#define TR_AUX_SERIAL_MODES_5          "LUA"
#define TR_AUX_SERIAL_MODES_6          "CLI"
#define TR_AUX_SERIAL_MODES_7          "GPS"
#define TR_AUX_SERIAL_MODES_8          "Отлад"
#define TR_AUX_SERIAL_MODES_9          "SpaceMouse"
#define TR_AUX_SERIAL_MODES_10         "Внеш модуль"
#define TR_SWTYPES_1                   "Нет"
#define TR_SWTYPES_2                   "тумбл"
#define TR_SWTYPES_3                   "2Х-поз пер"
#define TR_SWTYPES_4                   "3Х-поз пер"
#define TR_SWTYPES_5                   "Глоб"
#define TR_POTTYPES_1                  "None"
#define TR_POTTYPES_2                  "Pot"
#define TR_POTTYPES_3                  TR("Pot w. det","Pot with detent")
#define TR_POTTYPES_4                  "Slider"
#define TR_POTTYPES_5                  TR("Multipos","Multipos Switch")
#define TR_POTTYPES_6                  "Axis X"
#define TR_POTTYPES_7                  "Axis Y"
#define TR_POTTYPES_8                  "Switch"
#define TR_VPERSISTENT_1               "Выкл"
#define TR_VPERSISTENT_2               "Полет"
#define TR_VPERSISTENT_3               "Руч сброс"
#define TR_COUNTRY_CODES_1             TR("US","США")
#define TR_COUNTRY_CODES_2             TR("JP","Япония")
#define TR_COUNTRY_CODES_3             TR("EU","Евро")
#define TR_USBMODES_1                  "Выбор"
#define TR_USBMODES_2                  TR("Джойс","Джойстик")
#define TR_USBMODES_3                  TR("SD-карта","Хранил")
#define TR_USBMODES_4                  "Серийн"
#define TR_JACK_MODES_1                "Выбор"
#define TR_JACK_MODES_2                "Аудио"
#define TR_JACK_MODES_3                "Трен"

#define TR_SBUS_INVERSION_VALUES_1     "Норм"
#define TR_SBUS_INVERSION_VALUES_2     "Не перев"
#define TR_MULTI_CUSTOM                "Настроенный"
#define TR_VTRIMINC_1                  TR("Экспо","экспо")
#define TR_VTRIMINC_2                  TR("Оч мелк","Оч мелко")
#define TR_VTRIMINC_3                  "Мелко"
#define TR_VTRIMINC_4                  "Сред"
#define TR_VTRIMINC_5                  "Грубо"
#define TR_VDISPLAYTRIMS_1             "Нет"
#define TR_VDISPLAYTRIMS_2             "Измен"
#define TR_VDISPLAYTRIMS_3             "Да"
#define TR_VBEEPCOUNTDOWN_1            "Без звука"
#define TR_VBEEPCOUNTDOWN_2            "Сигн"
#define TR_VBEEPCOUNTDOWN_3            "Голос"
#define TR_VBEEPCOUNTDOWN_4            "Вибро"
#define TR_VBEEPCOUNTDOWN_5            TR("Звук вибро", "Сигн вибро")
#define TR_VBEEPCOUNTDOWN_6            TR("Голос вибро", "Голос вибро")
#define TR_COUNTDOWNVALUES_1           "5с"
#define TR_COUNTDOWNVALUES_2           "10с"
#define TR_COUNTDOWNVALUES_3           "20с"
#define TR_COUNTDOWNVALUES_4           "30с"
#define TR_VVARIOCENTER_1              "Тон"
#define TR_VVARIOCENTER_2              "Тихо"
#define TR_CURVE_TYPES_1               "Стд"
#define TR_CURVE_TYPES_2               "Выбор"

#define TR_ADCFILTERVALUES_1           "Глоб"
#define TR_ADCFILTERVALUES_2           "Вык"
#define TR_ADCFILTERVALUES_3           "Вкл"

#define TR_VCURVETYPE_1                "Разн"
#define TR_VCURVETYPE_2                "Экспо"
#define TR_VCURVETYPE_3                "Функц"
#define TR_VCURVETYPE_4                "Польз"
#define TR_VMLTPX_1                    "Добав"
#define TR_VMLTPX_2                    "Умнож"
#define TR_VMLTPX_3                    "Замен"

#define TR_CSWTIMER                    "Таймер"
#define TR_CSWSTICKY                   "Липучка"
#define TR_CSWSTAY                     "Кррай"

#define TR_SF_TRAINER                  "Таймер"
#define TR_SF_INST_TRIM                "Мгнов трим"
#define TR_SF_RESET                    "Сброс"
#define TR_SF_SET_TIMER                "Установ"
#define TR_SF_VOLUME                   "Громкость"
#define TR_SF_FAILSAFE                 "Авар управ"
#define TR_SF_RANGE_CHECK              "Пров диапаз"
#define TR_SF_MOD_BIND                 "Bind модуль"
#define TR_SF_RGBLEDS                  "RGB подсветка"

#define TR_SOUND                       "Воспр звук"
#define TR_PLAY_TRACK                  TR("Ply Trk","Воспр трек")
#define TR_PLAY_VALUE                  TR("Play Val","Play Value")
#define TR_SF_HAPTIC                   "Вибро"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua скрипт")
#define TR_SF_BG_MUSIC                 "Муз фон"
#define TR_SF_BG_MUSIC_PAUSE           "Муз ||"
#define TR_SF_LOGS                     "SD логи"
#define TR_ADJUST_GVAR                 "Настройка"
#define TR_SF_BACKLIGHT                "Подсветка"
#define TR_SF_VARIO                    "Варио"
#define TR_SF_TEST                     "Тест"

#define TR_SF_SAFETY                   TR("Отмен", "Оменить")

#define TR_SF_SCREENSHOT               "Скриншот"
#define TR_SF_RACING_MODE              "Режим гонка"
#define TR_SF_DISABLE_TOUCH            "Сенсор вык"
#define TR_SF_DISABLE_AUDIO_AMP        "Вык аудио усил."
#define TR_SF_SET_SCREEN               TR_BW_COL("Устан экран", "Выбр глав экран")
#define TR_SF_PUSH_CUST_SWITCH         "Нажать CS"
#define TR_SF_LCD_TO_VIDEO             "LCD в видео"

#define TR_FSW_RESET_TELEM             TR("Телем", "Телеметрия")
#define TR_FSW_RESET_TRIMS             "Тримы"
#define TR_FSW_RESET_TIMERS_1          "Тмр1"
#define TR_FSW_RESET_TIMERS_2          "Трм2"
#define TR_FSW_RESET_TIMERS_3          "Трм3"

#define TR_VFSWRESET_1                 TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                 TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                 TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                 TR("Все","Полетн")
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

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mlch"
#define LENGTH_UNIT_METR               "м"
#define SPEED_UNIT_METR                "кмч"

#define TR_VUNITSSYSTEM_1              "Метрич"
#define TR_VUNITSSYSTEM_2              "Импер"
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

#define TR_VTELEMSCREENTYPE_1          "Нет"
#define TR_VTELEMSCREENTYPE_2          "Цифры"
#define TR_VTELEMSCREENTYPE_3          "График"
#define TR_VTELEMSCREENTYPE_4          "Скрипты"
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

#define TR_ON_ONE_SWITCHES_1           "Вкл"
#define TR_ON_ONE_SWITCHES_2           "Один"

#define TR_HATSMODE                    "Hats mode"
#define TR_HATSOPT_1                   "Тримы"
#define TR_HATSOPT_2                   "Кнопки"
#define TR_HATSOPT_3                   "Тумбл"
#define TR_HATSOPT_4                   "Глоб"
#define TR_HATSMODE_TRIMS              "Hats mode: Тримы"
#define TR_HATSMODE_KEYS               "Hats mode: Кнопки"
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

#define TR_ROTARY_ENC_OPT_1          "Норм"
#define TR_ROTARY_ENC_OPT_2          "Перевер"
#define TR_ROTARY_ENC_OPT_3          "V-I H-N"
#define TR_ROTARY_ENC_OPT_4          "V-I H-A"
#define TR_ROTARY_ENC_OPT_5          "V-N E-I"

#define TR_IMU_VSRCRAW_1             "TltX"
#define TR_IMU_VSRCRAW_2             "TltY"
#define TR_CYC_VSRCRAW_1             "CYC1"
#define TR_CYC_VSRCRAW_2             "CYC2"
#define TR_CYC_VSRCRAW_3             "CYC3"

#define TR_SRC_BATT                    "АКБ"
#define TR_SRC_TIME                    "Время"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "ТМР"

#define TR_VTMRMODES_1                 "Вык"
#define TR_VTMRMODES_2                 "Вкл"
#define TR_VTMRMODES_3                 "Strt"
#define TR_VTMRMODES_4                 "THs"
#define TR_VTMRMODES_5                 "TH%"
#define TR_VTMRMODES_6                 "THt"
#define TR_VTRAINER_MASTER_OFF         "Вык"
#define TR_VTRAINER_MASTER_JACK        "Мастер/Разъем"
#define TR_VTRAINER_SLAVE_JACK         "Раб/Разъем"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Мастер/SBUS модуль"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Мастер/CPPM модуль"
#define TR_VTRAINER_MASTER_BATTERY     "Мастер/Серийный"
#define TR_VTRAINER_BLUETOOTH_1        "Мастер/" TR("BT","Bluetooth")
#define TR_VTRAINER_BLUETOOTH_2        "Раб/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Мастер/Мульти"
#define TR_VTRAINER_CRSF               "Мастер/CRSF"
#define TR_VFAILSAFE_1                 "Не устан"
#define TR_VFAILSAFE_2                 "Удерж"
#define TR_VFAILSAFE_3                 "Настр"
#define TR_VFAILSAFE_4                 "Нет импул"
#define TR_VFAILSAFE_5                 "Прием"
#define TR_VSENSORTYPES_1              "Настроен"
#define TR_VSENSORTYPES_2              "Рассчитан"
#define TR_VFORMULAS_1                 "Добав"
#define TR_VFORMULAS_2                 "Средн"
#define TR_VFORMULAS_3                 "Мин"
#define TR_VFORMULAS_4                 "Макс"
#define TR_VFORMULAS_5                 "Умнож"
#define TR_VFORMULAS_6                 "Суммир"
#define TR_VFORMULAS_7                 "Ячейка"
#define TR_VFORMULAS_8                 "Расх"
#define TR_VFORMULAS_9                 "Дист"
#define TR_VPREC_1                     "0.--"
#define TR_VPREC_2                     "0.0 "
#define TR_VPREC_3                     "0.00"
#define TR_VCELLINDEX_1                "Низ"
#define TR_VCELLINDEX_2                "1"
#define TR_VCELLINDEX_3                "2"
#define TR_VCELLINDEX_4                "3"
#define TR_VCELLINDEX_5                "4"
#define TR_VCELLINDEX_6                "5"
#define TR_VCELLINDEX_7                "6"
#define TR_VCELLINDEX_8                "7"
#define TR_VCELLINDEX_9                "8"
#define TR_VCELLINDEX_10               "Выс"
#define TR_VCELLINDEX_11               "Дельта"
#define TR_SUBTRIMMODES_1              CHAR_DELTA " (Тол цен)"
#define TR_SUBTRIMMODES_2              "= (Симметр)"
#define TR_TIMER_DIR_1                 TR("Остал", "Остал")
#define TR_TIMER_DIR_2                 TR("Прошло", "Прошед")

#define TR_FONT_SIZES_1                "Стд"
#define TR_FONT_SIZES_2                "Жирн"
#define TR_FONT_SIZES_3                "XXS"
#define TR_FONT_SIZES_4                "XS"
#define TR_FONT_SIZES_5                "L"
#define TR_FONT_SIZES_6                "XL"
#define TR_FONT_SIZES_7                "XXL"

#define TR_ENTER                       "[МЕНЮ]"
#define TR_OK                          TR_BW_COL(TR("\010\010\010[OK]", "\010\010\010\010\010[OK]"), "Ok")
#define TR_EXIT                        TR_BW_COL("ВЫХОД", "ВОЗВР")

#define TR_YES                         "Да"
#define TR_NO                          "Нет"
#define TR_DELETEMODEL                 "Удалить модель"
#define TR_COPYINGMODEL                "Копировать модель"
#define TR_MOVINGMODEL                 "Перенести модель"
#define TR_LOADINGMODEL                "Загрузить модель"
#define TR_UNLABELEDMODEL              "Без названия"
#define TR_NAME                        "Название"
#define TR_MODELNAME                   "Название"
#define TR_PHASENAME                   "Назв фазы"
#define TR_MIXNAME                     "Название Mix"
#define TR_INPUTNAME                   TR("Ввод", "вод")
#define TR_EXPONAME                    TR("Название", "Название")
#define TR_BITMAP                      "Изображ"
#define TR_NO_PICTURE                  "Без изображ"
#define TR_TIMER                       TR("Таймер", "Таймер ")
#define TR_NO_TIMERS                   "No timers"
#define TR_START                       "Старт"
#define TR_NEXT                        "Next"
#define TR_ELIMITS                     TR("расш лимит", "Расш лимит")
#define TR_ETRIMS                      TR("Расш трим", "Расш тримы")
#define TR_TRIMINC                     "Шаг трима"
#define TR_DISPLAY_TRIMS               TR("Пок. тримы", "Показ тримы")
#define TR_TTRACE                      TR("Т-источник", "Источник")
#define TR_TTRIM                       TR("Т-трим-ХХ", "Только трим хх")
#define TR_TTRIM_SW                    TR("Т-трим-перек", "Трим перек")
#define TR_BEEPCTR                     TR("Звук на цен", "Звук на центре")
#define TR_USE_GLOBAL_FUNCS            TR("Глоб. функц", "Глоб функц")
#define TR_PROTOCOL                    TR("Проток", "Протокол")
  #define TR_PPMFRAME                  "Фрейм PPM"
  #define TR_REFRESHRATE               TR("Обновл", "Част обновл")
  #define TR_WARN_BATTVOLTAGE         TR("Выход - V АКБ: ", "Предупреждение: уровень выхода - V АКБ: ")
#define TR_WARN_5VOLTS                 "Предупреждение: уровень выхода - 5 вольт"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Тумблер"
#define TR_FS_COLOR_LIST_1             "Свой"
#define TR_FS_COLOR_LIST_2             "Выкл"
#define TR_FS_COLOR_LIST_3             "Белый"
#define TR_FS_COLOR_LIST_4             "Красн"
#define TR_FS_COLOR_LIST_5             "Зелен"
#define TR_FS_COLOR_LIST_6             "Желт"
#define TR_FS_COLOR_LIST_7             "Оранж"
#define TR_FS_COLOR_LIST_8             "Синий"
#define TR_FS_COLOR_LIST_9             "Розов"
#define TR_GROUP                       "Группа"
#define TR_GROUP_ALWAYS_ON             "Включить все"
#define TR_LUA_OVERRIDE                "Allow Lua override"
#define TR_GROUPS                      "Вкл все группы"
#define TR_LAST                        "послед"
#define TR_MORE_INFO                   "Доп инфо"
#define TR_SWITCH_TYPE                 "Тип"
#define TR_SWITCH_STARTUP              "Запуск"
#define TR_SWITCH_GROUP                "группа"
#define TR_SF_SWITCH                   "Триггер"
#define TR_TRIMS                       "тримы"
#define TR_FADEIN                      "Затух входа"
#define TR_FADEOUT                     "Затух выхода"
#define TR_DEFAULT                     "(По умолч)"
#define   TR_CHECKTRIMS                 TR_BW_COL("\006Check\012trims", "Провер тримов")
#define TR_SWASHTYPE                   "Наклонный тип"
#define TR_COLLECTIVE                  TR("Коллек", "Коллек")
#define TR_AILERON                     TR("Крен", "Крен")
#define TR_ELEVATOR                    TR("Тангаж", "Тангаж")
#define TR_SWASHRING                   "Поворот"
#define TR_MODE                        "Режим"
#define TR_LEFT_STICK                  "Лев"
#define TR_SUBTYPE                     "Подтип"
#define TR_NOFREEEXPO                  "Нет своб экспо!"
#define TR_NOFREEMIXER                 "Нет своб микш!"
#define TR_SOURCE                       "Источ"
#define TR_WEIGHT                      "Вес"
#define TR_SIDE                        "Сторона"
#define TR_OFFSET                       "Смещение"
#define TR_TRIM                        "Трим"
#define TR_CURVE                       "Кривая"
#define TR_FLMODE                      TR("Режим", "Режим")
#define TR_MIXWARNING                  "Предупр"
#define TR_OFF                         "Вык"
#define TR_ANTENNA                     "Антенна"
#define TR_NO_INFORMATION              TR("Нет инфо", "Нет инфо")
#define TR_MULTPX                      "Multiplex"
#define TR_DELAYDOWN                   TR("Задер вниз", "Задер вниз")
#define TR_DELAYUP                     "Задер вверх"
#define TR_SLOWDOWN                    TR("Замедл вниз", "Замедл вниз")
#define TR_SLOWUP                      "Замед вверх"
#define TR_CV                          "CV"
#define TR_GV                          TR("G", "GV")
#define TR_RANGE                       "Диапаз"
#define TR_CENTER                      "Центр"
#define TR_ALARM                       "Сигнал тревоги"
#define TR_BLADES                      "Blades/Poles"
#define TR_SCREEN                      "Экран\001"
#define TR_SOUND_LABEL                 "Звук"
#define TR_LENGTH                      "Длин"
#define TR_BEEP_LENGTH                 "Длин звука"
#define TR_BEEP_PITCH                  "Выс звука"
#define TR_HAPTIC_LABEL                "Вибрация"
#define TR_STRENGTH                    "Сила"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Смещение"
#define TR_IMU_MAX                     "Макс"
#define TR_CONTRAST                    "Контраст"
#define TR_ALARMS_LABEL                "Сигнал тревоги"
#define TR_BATTERY_RANGE               TR("Диапаз АКБ", "Диапаз АКБ")
#define TR_BATTERYCHARGING             "Зарядка..."
#define TR_BATTERYFULL                 "АКБ Заряжена"
#define TR_BATTERYNONE                 "АКБ Отсутст"
#define TR_BATTERYWARNING              "АКБ Разряж"
#define TR_INACTIVITYALARM             "Бездействие"
#define TR_MEMORYWARNING               "Память заканч"
#define TR_ALARMWARNING                "Звук Выкл"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI выкл", "RSSI выкл")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("ВЫК Тренер", "Проверьте работу тренера при ВЫКЛ")
#define TR_MODEL_STILL_POWERED         "Модель работает"
#define TR_TRAINER_STILL_CONNECTED     "Тренер подкл"
#define TR_USB_STILL_CONNECTED         "USB подкл"
#define TR_MODEL_SHUTDOWN              "Отключение?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Нажмите [Enter] для подтв"
#define TR_THROTTLE_LABEL              "Газ"
#define TR_THROTTLE_START              "Газ cтарт"
#define TR_THROTTLEREVERSE             TR("Г-Реверс", "Реверс")
#define TR_MINUTEBEEP                  TR("Минут", "Минут")
#define TR_BEEPCOUNTDOWN               "Обр отс"
#define TR_PERSISTENT                  TR("Пост", "Пост")
#define TR_BACKLIGHT_LABEL             "Подсветка"
#define TR_GHOST_MENU_LABEL            "Меню призр"
#define TR_STATUS                      "Статус"
#define TR_BLONBRIGHTNESS              "Подсветка Вкл"
#define TR_BLOFFBRIGHTNESS             "Подсветка Вык"
#define TR_KEYS_BACKLIGHT              "Подсветка клавиш"
#define TR_BLCOLOR                     "Цвет"
#define TR_SPLASHSCREEN                "Заставка"
#define TR_PLAY_HELLO                  "Звук запуска"
#define TR_PWR_ON_DELAY                "Время включ"
#define TR_PWR_OFF_DELAY               "Время выключ"
#define TR_PWR_AUTO_OFF                TR("Авто выкл питания","Авто выкл питания")
#define TR_PWR_ON_OFF_HAPTIC           TR("Тактил Вкл/Выкл питания","Тактил Вкл/Выкл питания")
#define TR_THROTTLE_WARNING            TR("Г-Предупр", "Статис газа")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Свое пол", "Свое пол?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Пол. %", "Положение %")
#define TR_SWITCHWARNING               TR("П-Предупр", "Поз тумбл")
#define TR_POTWARNINGSTATE             "Сост потенц и тумбл"
#define TR_POTWARNING                  TR("Полож пот", "Полож потенц")
#define TR_TIMEZONE                    "Временная зона"
#define TR_ADJUST_RTC                  "Настроить RTC"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Пор. Кн. По Ум.", "Порядок кан по умолч")
#define TR_STICKS                      "Axis"
#define TR_POTS                        "Потенциом"
#define TR_SWITCHES                    "Тумблеры"
#define TR_SWITCHES_DELAY              TR("Зад воспр", "Задерж воспр (средн. полож. тумбл.)")
#define TR_SLAVE                       "Рабочий"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "Множитель"
#define TR_CAL                         "Звонок"
#define TR_CALIBRATION                 BUTTON("Калибровка")
#define TR_VTRIM                       "Трим - +"
#define TR_CALIB_DONE                  "Калибровка заверш"
#define TR_MENUTOSTART                 TR_ENTER " СТАРТ"
#define TR_MENUWHENDONE                TR_ENTER " ПО ОКОНЧАНИЮ"
#define TR_AXISDIR                     "AXIS DIR"
#define TR_MENUAXISDIR                 "[ENTER LONG] "  TR_AXISDIR
#define TR_SETMIDPOINT                 TR_BW_COL(TR_SFC_AIR("ВЫБ СЕРЕД ПОТЕНЦ", TR("ВЫБ СТИКИ СЕРЕД", "ЦЕНТР СТИК/СЛАЙДЕР")), "ЦЕНТР СТИКИ/СЛАЙДЕРЫ")
#define TR_MOVESTICKSPOTS              TR_BW_COL(TR_SFC_AIR("ДВИГ ПОТЕНЦ/СТИКИ/СЛАЙДЕРЫ", "ДВИГ СТИКИ/ПОТЕНЦ"), "Перемещ СТИКИ/ПОТЕНЦ")
#define TR_NODATA                      "НЕТ ДАННЫХ"
#define TR_US                          "US"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Макс Tmix"
#define TR_FREE_STACK                  "Свободн стек"
#define TR_INT_GPS_LABEL               "Внутренний GPS"
#define TR_HEARTBEAT_LABEL             "Пульсация"
#define TR_LUA_SCRIPTS_LABEL           "Lua Скрипт"
#define TR_FREE_MEM_LABEL              "Свободно памяти"
#define TR_DURATION_MS             TR("[D]","Длител(ms): ")
#define TR_INTERVAL_MS             TR("[I]","Интерв(ms): ")
#define TR_MEM_USED_SCRIPT         "Скрипт(B): "
#define TR_MEM_USED_WIDGET         "Виджет(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Аудио: "
#define TR_GPS_FIX_YES                 "Фикс: Да"
#define TR_GPS_FIX_NO                  "Фикс: Нет"
#define TR_GPS_SATS                    "Спутники: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Меню: "
#define TR_TIMER_LABEL                 "Таймер"
#define TR_THROTTLE_PERCENT_LABEL      "Газ %"
#define TR_BATT_LABEL                  "АКБ"
#define TR_SESSION                     "Сесия"
#define TR_MENUTORESET                 TR_ENTER " Для сброса"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "Модель"
#define TR_FM                          TR_SFC_AIR("DM", "FM")
#define TR_EEPROMLOWMEM                "EEPROM Нехват памяти"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Нажмите любую кнопку"
#define TR_THROTTLE_NOT_IDLE           "Газ не на ХХ"
#define TR_ALARMSDISABLED              "Аварийные сигналы отключены"
#define TR_PRESSANYKEY                 TR("\010Нажмите любую кнопку", "Нажмите любую кнопку")
#define TR_BADEEPROMDATA               "Неверные данные EEPROM"
#define TR_BAD_RADIO_DATA              "Отсутствуют радиоданные"
#define TR_RADIO_DATA_RECOVERED        TR3("Исп резер настр", "Исп резер настр", "Исп резер настр")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Недейст настр", "Недейств настр", "Не удалось проч настр")
#define TR_EEPROMFORMATTING            "Формат EEPROM"
#define TR_STORAGE_FORMAT              "Формат памяти"
#define TR_EEPROMOVERFLOW              "EEPROM заполнен"
#define TR_RADIO_SETUP                 "НАСТРОЙКА ПУЛЬТА"
#define TR_MENUVERSION                 "ВЕРСИЯ"
#define TR_MENU_RADIO_ANALOGS          TR("АНАЛОГИ", "ТЕСТ АНАЛОГОВ")
#define TR_MENU_RADIO_ANALOGS_CALIB    "КАЛИБРОВКА АНАЛОГОВ"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW АНАЛОГИ (5 Hz)"
#define TR_MENU_FSWITCH                "НАСТРАИВАЕМЫЕ ТУМБЛЕРЫ"
#define   TR_TRIMS2OFFSETS              TR_BW_COL("\006Тримы => Субтримы", "Тримы => Субтримы")
#define TR_CHANNELS2FAILSAFE           "Каналы=>Авар режим"
#define TR_CHANNEL2FAILSAFE            "Канал=>Авар режим"
#define TR_MENUMODELSEL                TR("ВЫБОР МОДЕЛИ", "УПРАВЛЕНИЕ МОДЕЛЬЮ")
#define TR_MENU_MODEL_SETUP            TR("НАСТРОЙКА", "НАСТРОЙКА МОДЕЛИ")
#define TR_MENUCURVE                   "КРИВАЯ"
#define TR_MENULOGICALSWITCH           "ЛОГИЧ ТУМБЛ."
#define TR_MENUSTAT                    "СТАТИСТИКА"
#define TR_MENUDEBUG                   "ОТЛАДКА"
#define TR_MONITOR_CHANNELS            "МОНИТОР КАН %d-%d"
#define TR_MONITOR_OUTPUT_DESC         "Выходы"
#define TR_MONITOR_MIXER_DESC          "Микшеры"
  #define TR_RECEIVER_NUM              TR("Rx Ном", "Номер")
  #define TR_RECEIVER                  "Приемник"
#define TR_MULTI_RFTUNE                TR("Наст частоты", "Наст частоты")
#define TR_MULTI_RFPOWER               "Мощность RF"
#define TR_MULTI_WBUS                  "Выход"
#define TR_MULTI_TELEMETRY             "Телеметрия"
#define TR_MULTI_VIDFREQ               TR("Вид частота", "Вид частота")
#define TR_RF_POWER                    "Мощность RF"
#define TR_MULTI_FIXEDID               TR("Фикс ID", "Фикс ID")
#define TR_MULTI_OPTION                TR("Опция", "Знач опции")
#define TR_MULTI_AUTOBIND              TR("Прив к кан", "Прив к кан")
#define TR_DISABLE_CH_MAP              TR("Откл карту кан", "Откл карту кан")
#define TR_DISABLE_TELEM               TR("Без телеметрии", "Откл телем")
#define TR_MULTI_LOWPOWER              TR("Низк потребл", "Низк потребл")
#define TR_MULTI_LNA_DISABLE           "Откл LNA"
#define TR_MODULE_TELEMETRY            TR("S.Port", "Связь S.Port")
#define TR_MODULE_TELEM_ON             TR("Вкл", "Вкл")
#define TR_DISABLE_INTERNAL            TR("Откл внутр.", "Откл внут TX")
#define TR_MODULE_NO_SERIAL_MODE       TR("!Режим SERIAL", "Не в режиме SERIAL")
#define TR_MODULE_NO_INPUT             TR("Нет ввода", "Нет ввода")
#define TR_MODULE_NO_TELEMETRY         TR3("НЕТ ТЕЛЕМ", "НЕТ ТЕЛЕМЕТРИИ", "ТЕЛЕМЕТРИЯ НЕ ОБНАРУЖЕНА")
#define TR_MODULE_WAITFORBIND          "Привязка для загруж протокола"
#define TR_MODULE_BINDING              TR("Привязка...", "Привязка")
#define TR_MODULE_UPGRADE_ALERT        TR3("Треб обновл", "Треб обновл", "Треб\nобновл модуля")
#define TR_MODULE_UPGRADE              TR("Рекомд обновл", "Рекомд обновл")
#define TR_REBIND                      "Треб повтор привязка"
#define TR_REG_OK                      "Усп регистрация"
#define TR_BIND_OK                     "Усп привязка"
#define TR_BINDING_CH1_8_TELEM_ON      "Кн1-8 Телем Вкл"
#define TR_BINDING_CH1_8_TELEM_OFF     "Кн1-8 Телем Выкл"
#define TR_BINDING_CH9_16_TELEM_ON     "Кн9-16 Телем Вкл"
#define TR_BINDING_CH9_16_TELEM_OFF    "Кн9-16 Телем Выкл"
#define TR_PROTOCOL_INVALID            TR("Недопус протокол", "Протокол недейств")
#define TR_MODULE_STATUS               TR("Статус", "Статус")
#define TR_MODULE_SYNC                 TR("Синхро", "Синхро протокола")
#define TR_MULTI_SERVOFREQ             TR("Частота серво", "Частота серво")
#define TR_MULTI_MAX_THROW             TR("Макс. ход", "Вкл макс. ход")
#define TR_MULTI_RFCHAN                TR("RF Канал", "Выбр RF канал")
#define TR_AFHDS3_RX_FREQ              TR("Частота RX", "Частота RX")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Одноадрес/Телем", "Одноадрес/Телем")
#define TR_AFHDS3_ONE_TO_MANY          "Мультикаст"
#define TR_AFHDS3_ACTUAL_POWER         TR("Акт. мощность", "Фактич мощность")
#define TR_AFHDS3_POWER_SOURCE         TR("Источ питания", "Источ питания")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "FlySky RSSI")
#define TR_GPS_COORDS_FORMAT           TR("Формат коорд GPS", "Формат коорд")
#define TR_VARIO                       TR("Вариометр", "Вариометр")
#define TR_PITCH_AT_ZERO               "Тоны при нуле"
#define TR_PITCH_AT_MAX                "Тоны при макс."
#define TR_REPEAT_AT_ZERO              "Повт при нуле"
#define TR_BATT_CALIB                  TR("Калибр АКБ", "Калибр АКБ")
#define TR_CURRENT_CALIB               "Калибр тока"
#define TR_VOLTAGE                     TR("Напряжение", "Напряжение")
#define TR_SELECT_MODEL                "Выбор модели"
#define TR_MANAGE_MODELS               "УПРАВЛЕНИЕ МОДЕЛЯМИ"
#define TR_MODELS                      "Модели"
#define TR_SELECT_MODE                 "Выбрать модель"
#define TR_CREATE_MODEL                "Создать модель"
#define TR_FAVORITE_LABEL              "Избранное"
#define TR_MODELS_MOVED                "Неисп модели перенес в"
#define TR_NEW_MODEL                   "Новая модель"
#define TR_INVALID_MODEL               "Недопус модель"
#define TR_EDIT_LABELS                 "Редактир метки"
#define TR_LABEL_MODEL                 "Метка модели"
#define TR_MOVE_UP                     "Перемес вверх"
#define TR_MOVE_DOWN                   "Перемес вниз"
#define TR_ENTER_LABEL                 "Ввести метку"
#define TR_LABEL                       "Метка"
#define TR_LABELS                      "Метки"
#define TR_CURRENT_MODEL               "Текущая"
#define TR_ACTIVE                      "Активная"
#define TR_NEW                         "Новая"
#define TR_NEW_LABEL                   "Новая метка"
#define TR_RENAME_LABEL                "Переимен метку"
#define TR_DELETE_LABEL                "Удалить метку"
#define TR_DUPLICATE_MODEL             "Дублир модель"
#define TR_COPY_MODEL                  "Копир модель"
#define TR_MOVE_MODEL                  "Перемес модель"
#define TR_BACKUP_MODEL                "Резер копия модели"
#define TR_DELETE_MODEL                "Удалить модель"
#define TR_RESTORE_MODEL               "Восстан модель"
#define TR_DELETE_ERROR                "Удалить ошибку"
#define TR_SDCARD_ERROR                TR("Ошибка SD карты", "Ошибка SD карты")
#define TR_SDCARD                      "SD карта"
#define TR_NO_FILES_ON_SD              "Нет данных на SD карте!"
#define TR_NO_SDCARD                   "Нет CD карты"
#define TR_WAITING_FOR_RX              "Ожид приемника..."
#define TR_WAITING_FOR_TX              "Ожид передатчика..."
#define TR_WAITING_FOR_MODULE          TR("Ожид модуля", "Ожид модуля...")
#define TR_NO_TOOLS                    "Отсут инструменты"
#define TR_NORMAL                      "Обычный"
#define TR_NOT_INVERTED                "Не инвер"
#define TR_NOT_CONNECTED               TR("!Подключ", "Не подключ")
#define TR_CONNECTED                   "Подключено"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16КН без телем", "16КН без телем")
#define TR_16CH_WITH_TELEMETRY         TR("16КН с телем", "16КН с телем")
#define TR_8CH_WITH_TELEMETRY          TR("8КН с телем", "8КН с телем")
#define TR_EXT_ANTENNA                 "Внеш антенна"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Обнов опции прием?"
#define TR_UPDATE_TX_OPTIONS           "Обнов опции передат?"
#define TR_MODULES_RX_VERSION          BUTTON("Модули / Версия пульта")
#define TR_SHOW_MIXER_MONITORS         "Показ мониторы миксера"
#define TR_MENU_MODULES_RX_VERSION     "МОДУЛИ / ВЕРСИЯ ПУЛЬТА"
#define TR_MENU_FIRM_OPTIONS           "ОПЦИИ ПО"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Стики/Потенц/Слайдеры"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Стики/Потенц/Слайдеры"
#define TR_RF_PROTOCOL                 "TX Протокол"
#define TR_MODULE_OPTIONS              "Опции модуля"
#define TR_POWER                       "Питание"
#define TR_NO_TX_OPTIONS               "Отсут опции передат"
#define TR_RTC_BATT                    "RTC АКБ"
#define TR_POWER_METER_EXT             "Изм мощност (Внеш)"
#define TR_POWER_METER_INT             "Изм мощност (Внут)"
#define TR_SPECTRUM_ANALYSER_EXT       "Спек анализ (Внеш)"
#define TR_SPECTRUM_ANALYSER_INT       "Спек анализ (Внут)"
#define TR_GHOST_MODULE_CONFIG         "Ghost module config"
#define TR_GPS_MODEL_LOCATOR           "GPS model locator"
#define TR_REFRESH                     "Refresh"
#define TR_SDCARD_FULL                 "SD карта заполнена"
#define TR_SDCARD_FULL_EXT              TR_BW_COL(TR_SDCARD_FULL "\036Логи и скриншоты" LCDW_128_LINEBREAK "Откл", TR_SDCARD_FULL "\nЛоги и скриншоты откл")
#define TR_NEEDS_FILE                  "НУЖЕН ФАЙЛ"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Несовмес"
#define TR_WARNING                     "ВНИМАНИЕ"
#define TR_STORAGE_WARNING             "ПАМЯТЬ"
#define TR_THROTTLE_UPPERCASE          "Газ"
#define TR_ALARMSWARN                  "СИГНАЛЫ"
#define TR_SWITCHWARN                  TR("ТУМБЛЕРЫ", "ТУМБЛЕРЫ")
#define TR_FAILSAFEWARN                "СБОЙ БЕЗОПАСНОСТИ"
#define TR_TEST_WARNING                TR("ТЕСТ", "ТЕСТ")
#define TR_TEST_NOTSAFE                "Испол для тестов"
#define TR_WRONG_SDCARDVERSION         TR("Ожид версия: ", "Ожид версия: ")
#define TR_WARN_RTC_BATTERY_LOW        "Низкий заряд АКБ RTC"
#define TR_WARN_MULTI_LOWPOWER         "Режим низ энергопотреб"
#define TR_BATTERY                     "АКБ"
#define TR_WRONG_PCBREV                "Обнаруж неправ версия платы"
#define TR_EMERGENCY_MODE              "РЕЖИМ ЧРЕЗВ СИТУАЦИИ"
#define TR_NO_FAILSAFE                 "Failsafe не настроен"
#define TR_KEYSTUCK                    "Застрявшая клавиша"
#define TR_VOLUME                      "Громкость"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Подсветка"
#define TR_CONTROL                     "Control"
#define TR_CPU_TEMP                    "Темпер проц"
#define TR_COPROC                      "Сопроцессор"
#define TR_COPROC_TEMP                 "Темпер сопроц"
#define TR_TTL_WARNING                 "Внимание: Не превышать напряжение 3,3 В на выводах TX/RX !"
#define TR_FUNC                        "Функция"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Продолж"
#define TR_DELAY                       "Задержка"
#define TR_NO_SOUNDS_ON_SD             "Нет звуков на SD карте"
#define TR_NO_MODELS_ON_SD             "Нет модели на SD карте"
#define TR_NO_BITMAPS_ON_SD            "Нет изобр на SD карте"
#define TR_NO_SCRIPTS_ON_SD            "Нет скриптов на SD карте"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Синтак ошибка", "Синтак ошибка")
#define TR_SCRIPT_PANIC                "Аварийное заверш сценария"
#define TR_SCRIPT_KILLED               "Сценарий остановлен"
#define TR_SCRIPT_ERROR                "Неизвестная ошибка"
#define TR_PLAY_FILE                   "ВСПР"
#define TR_DELETE_FILE                 "УДАЛИТЬ"
#define TR_COPY_FILE                   "КОПИРОВАТЬ"
#define TR_RENAME_FILE                 "ПЕРЕИМЕНОВАТЬ"
#define TR_ASSIGN_BITMAP               "Назначить изображ"
#define TR_ASSIGN_SPLASH               "Экран приветствия"
#define TR_EXECUTE_FILE                "ЗАПУСТИТЬ"
#define TR_REMOVED                     "Удалено"
#define TR_SD_INFO                     "Инфо"
#define TR_NA                          "Н/Д"
#define TR_FORMATTING                  "Формат..."
#define TR_TEMP_CALIB                  "Калиб темп"
#define TR_TIME                        "Время"
#define TR_MAXBAUDRATE                 "Макс скорость"
#define TR_BAUDRATE                    "Скор пер"
#define TR_CRSF_ARMING_MODE            "Arm режим"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 TR("Реж выб","Реж выб")
#define TR_SAMPLE_MODES_1              "Обыч"
#define TR_SAMPLE_MODES_2              "Один бит"
#define TR_LOADING                     "Загрузка..."
#define TR_DELETE_THEME                "Удалить тему?"
#define TR_SAVE_THEME                  "Сохранить тему?"
#define TR_EDIT_COLOR                  "Выбор цвета"
#define TR_NO_THEME_IMAGE              "Отсутс изображ темы"
#define TR_BACKLIGHT_TIMER             "Время бездействия"

#define TR_MODEL_QUICK_SELECT        "Быстрый выбор модели"
#define TR_LABELS_SELECT             "Выбор"
#define TR_LABELS_MATCH              "Cоответствие"
#define TR_FAV_MATCH                 "Избранное"
#define TR_LABELS_SELECT_MODE_1      "Мультивыбор"
#define TR_LABELS_SELECT_MODE_2      "Одиноч выбор"
#define TR_LABELS_MATCH_MODE_1       "Все"
#define TR_LABELS_MATCH_MODE_2       "Любой"
#define TR_FAV_MATCH_MODE_1          "Совпадает"
#define TR_FAV_MATCH_MODE_2          "Не совпадает"

#define TR_SELECT_TEMPLATE_FOLDER      "Выберите папку шаблона"
#define TR_SELECT_TEMPLATE             "ВЫБЕРИТЕ ШАБЛОН МОДЕЛИ:"
#define TR_NO_TEMPLATES                "В этой папке нет шаблона"
#define TR_SAVE_TEMPLATE               "Сохранить шаблон"
#define TR_BLANK_MODEL                 "Пустая модель"
#define TR_BLANK_MODEL_INFO            "Создайте пустую модель"
#define TR_FILE_EXISTS                 "ФАЙЛ УЖЕ СУЩЕСТВУЕТ"
#define TR_ASK_OVERWRITE               "Вы хотите перезаписать?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Найти"
#define TR_BLUETOOTH_INIT              "Инициал"
#define TR_BLUETOOTH_DIST_ADDR         "Адрес назначения"
#define TR_BLUETOOTH_LOCAL_ADDR        "Локальный адрес"
#define TR_BLUETOOTH_PIN_CODE          "PIN код"
#define TR_BLUETOOTH_NODEVICES         "Устройства не найдены"
#define TR_BLUETOOTH_SCANNING          "Поиск..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#define TR_BLUETOOTH_MODES_1           "---"
#define TR_BLUETOOTH_MODES_2           "Телем"
#define TR_BLUETOOTH_MODES_3           "Тренер"
#define TR_BLUETOOTH_MODES_4           "Вкл"

#define TR_SD_INFO_TITLE               "SD Инфо"
#define TR_SD_SPEED                    "Скорость:"
#define TR_SD_SECTORS                  "Сектора:"
#define TR_SD_SIZE                     "Размер:"
#define TR_TYPE                        "Тип"
#define TR_GVARS                       "ГЛОБ ПЕРЕМ"
#define TR_GLOBAL_VAR                  "Глоб переменная"
#define TR_OWN                         "Свой"
#define TR_DATE                        "Дата"
#define TR_MONTHS_1                    "Янв"
#define TR_MONTHS_2                    "Фев"
#define TR_MONTHS_3                    "Мар"
#define TR_MONTHS_4                    "Апр"
#define TR_MONTHS_5                    "Май"
#define TR_MONTHS_6                    "Июн"
#define TR_MONTHS_7                    "Июл"
#define TR_MONTHS_8                    "Авг"
#define TR_MONTHS_9                    "Сен"
#define TR_MONTHS_10                   "Окт"
#define TR_MONTHS_11                   "Ноя"
#define TR_MONTHS_12                   "Дек"
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("Режим энкод","Режим энкод")
#define TR_CHANNELS_MONITOR            "МОНИТОР КАНАЛОВ"
#define TR_MIXERS_MONITOR              "МОНИТОР МИКСЕРОВ"
#define TR_PATH_TOO_LONG               "Путь слишком длинный"
#define TR_VIEW_TEXT                   "ПРОСМОТР ТЕКСТА"
#define TR_FLASH_BOOTLOADER            "ПРОШИТЬ ПО"
#define TR_FLASH_DEVICE                TR("Прошить устр","Прошить устр")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Записать S.Port", "Записать устройство S.Port")
#define TR_FLASH_RECEIVER_OTA          "Записать устр через OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Записать устр через внеш OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Записать устр через внут OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Записать через внеш OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Записать через внут OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Записать модуль Bluetooth", "Записать модуль Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Записать блок управл питанием"
#define TR_DEVICE_NO_RESPONSE          TR("Устр не отвеч", "Устр не отвеч")
#define TR_DEVICE_FILE_ERROR           TR("Проблема с файлом", "Проблема с файлом")
#define TR_DEVICE_DATA_REFUSED         TR("Отказ в перед данных", "Отказ в передаче данных")
#define TR_DEVICE_WRONG_REQUEST        TR("Проблема доступа", "Проблема доступа")
#define TR_DEVICE_FILE_REJECTED        TR("Файл отклонен", "Файл отклонен")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Файл имеет неправ подпись", "Файл имеет неправ подпись")
#define TR_CURRENT_VERSION             TR("Текущая версия: ", "Текущая версия: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Зап внут модуль", "Зап внут модуль")
#define TR_FLASH_INTERNAL_MULTI        TR("Зап внут MULTI", "Зап внут MULTI")
#define TR_FLASH_EXTERNAL_MODULE       TR("Зап внеш модуль", "Зап внеш модуль")
#define TR_FLASH_EXTERNAL_MULTI        TR("Зап внеш MULTI", "Зап внеш MULTI")
#define TR_FLASH_EXTERNAL_ELRS         TR("Зап внеш ELRS", "Зап внеш ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Ошибка обнов ПО", "Ошибка обнов ПО")
#define TR_FIRMWARE_UPDATE_SUCCESS     "ОБНОВЛЕНО"
#define TR_WRITING                     "Запись..."
#define TR_CONFIRM_FORMAT              "Подтв формат?"
#define TR_INTERNALRF                  "Внут TX"
#define TR_INTERNAL_MODULE             TR("Внут. Модуль", "Внут Модуль")
#define TR_EXTERNAL_MODULE             TR("Внеш. Модуль", "Внеш Модуль")
#define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX требуется обновл"
#define TR_TELEMETRY_DISABLED          "Телем. откл"
#define TR_MORE_OPTIONS_AVAILABLE      "Доступно больше опций"
#define TR_NO_MODULE_INFORMATION       "Нет инфо о модуле"
#define TR_EXTERNALRF                  "Внеш TX"
#define TR_FAILSAFE                    TR("Сбой", "Режим сбоя")
#define TR_FAILSAFESET                 "Настройки сбоя"
#define TR_REG_ID                      "ID регистр."
#define TR_OWNER_ID                    "ID владел."
#define TR_HOLD                        "Удержание"
#define TR_HOLD_UPPERCASE              "УДЕРЖАНИЕ"
#define TR_NONE                        "Отсутствует"
#define TR_NONE_UPPERCASE              "ОТСУТСТВУЕТ"
#define TR_MENUSENSOR                  "Сенсор"
#define TR_POWERMETER_PEAK             "Пиковое"
#define TR_POWERMETER_POWER            "Мощность"
#define TR_POWERMETER_ATTN             "Аттен"
#define TR_POWERMETER_FREQ             "Частота"
#define TR_MENUTOOLS                   "ИНСТРУМЕНТЫ"
#define TR_TURN_OFF_RECEIVER           "Выкл приемник"
#define TR_STOPPING                    "Остановка..."
#define TR_MENU_SPECTRUM_ANALYSER      "АНАЛИЗАТОР СПЕКТРА"
#define TR_MENU_POWER_METER            "ИЗМЕРИТЕЛЬ МОЩНОСТИ"
#define TR_SENSOR                      "СЕНСОР"
#define TR_COUNTRY_CODE                "Код страны"
#define TR_USBMODE                     "Режим USB"
#define TR_JACK_MODE                   "Режим разъема"
#define TR_VOICE_LANGUAGE              "Язык озвучки"
#define TR_UNITS_SYSTEM                "Единицы"
#define TR_UNITS_PPM                   "PPM Един"
#define TR_EDIT                        "Редактировать"
#define TR_INSERT_BEFORE               "Вставить перед"
#define TR_INSERT_AFTER                "Вставить после"
#define TR_COPY                        "Копировать"
#define TR_MOVE                        "Переход"
#define TR_PASTE                       "Вставить"
#define TR_PASTE_AFTER                 "Вставить после"
#define TR_PASTE_BEFORE                "Вставить перед"
#define TR_DELETE                      "Удалить"
#define TR_INSERT                      "Вставить"
#define TR_RESET_FLIGHT                "Сброс сесии"
#define TR_RESET_TIMER1                "Сбросить таймер1"
#define TR_RESET_TIMER2                "Сбросить таймер2"
#define TR_RESET_TIMER3                "Сбросить таймер3"
#define TR_RESET_TELEMETRY             "Сбросить тел-рию"
#define TR_STATISTICS                  "Статистика"
#define TR_ABOUT_US                    "Информация"
#define TR_USB_JOYSTICK                "USB Джойс (HID)"
#define TR_USB_MASS_STORAGE            "USB Карта SD (SD)"
#define TR_USB_SERIAL                  "USB Serial (VCP)"
#define TR_SETUP_SCREENS               "Выбор экранов"
#define TR_MONITOR_SCREENS             "Мониторы"
#define TR_AND_SWITCH                  "AND Тумблер"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Калибровка аналогов"
#define TR_ANADIAGS_FILTRAWDEV         "Отфильтр необработ аналог сигн с отклон"
#define TR_ANADIAGS_UNFILTRAW          "Неотфильтр необработ аналог сигн"
#define TR_ANADIAGS_MINMAX             "Мин., Макс. и Диапазон"
#define TR_ANADIAGS_MOVE               "Двигайте аналоговые датчики!"
#define TR_BYTES                       "Байты"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_MODULE_UNBIND               BUTTON("Unbind")
#define TR_POWERMETER_ATTN_NEEDED     "Требуется аттенюатор"
#define TR_PXX2_SELECT_RX              "Выбор RX"
#define TR_PXX2_DEFAULT                "<По умолчанию>"
#define TR_BT_SELECT_DEVICE            "Выбор устройства"
#define TR_DISCOVER                    BUTTON("Обнаружить")
#define TR_BUTTON_INIT                 BUTTON("Иниц")
#define TR_WAITING                     "Предупреждение..."
#define TR_RECEIVER_DELETE             "Удалить приемник?"
#define TR_RECEIVER_RESET              "Сбросить приемник?"
#define TR_SHARE                       "Поделиться"
#define TR_BIND                        "Bind"
#define TR_REGISTER                    BUTTON(TR("Рег", "Регистр"))
#define TR_MODULE_RANGE                BUTTON(TR("Диап", "Диапазон"))
#define TR_RANGE_TEST                  "Тест диапазона"
#define TR_RECEIVER_OPTIONS            TR("ОПЦИИ ПРИЕМНИКА", "ОПЦИИ ПРИЕМНИКА")
#define TR_RESET_BTN                   BUTTON("Сброс")
#define TR_KEYS_BTN                    BUTTON("Кноп")
#define TR_ANALOGS_BTN                 BUTTON(TR("Анлг", "Аналоги"))
#define TR_FS_BTN                      BUTTON(TR("Пользов тумблер", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Сенсор не найден"
#define TR_TOUCH_EXIT                  "Нажмите на экран, чтобы выйти"
#define TR_SET                         BUTTON("Установить")
#define TR_TRAINER                     "Тренажер"
#define TR_CHANS                       "Каналы"
#define TR_ANTENNAPROBLEM              "Проблема с антенной TX!"
#define TR_MODELIDUSED                 "ID использован в:"
#define TR_MODELIDUNIQUE               "ID уникален"
#define TR_MODULE                      "Модуль"
#define TR_RX_NAME                     "Rx Имя"
#define TR_TELEMETRY_TYPE              TR("Тип", "Тип")
#define TR_TELEMETRY_SENSORS           "Сенсоры"
#define TR_VALUE                       "Значен"
#define TR_PERIOD                      "Период"
#define TR_INTERVAL                    "Интервал"
#define TR_REPEAT                      "Повтор"
#define TR_ENABLE                      "Включено"
#define TR_DISABLE                     "Выключено"
#define TR_TOPLCDTIMER                 "Верхний таймер LCD"
#define TR_UNIT                        "Единица"
#define TR_TELEMETRY_NEWSENSOR         "Добавить новый"
#define TR_CHANNELRANGE                TR("Диапазон", "Диапазон кан")
#define TR_ANTENNACONFIRM1             "ВНЕШ АНТЕННА"
#define TR_ANTENNA_MODES_1           "Внут"
#define TR_ANTENNA_MODES_2           "Запрос"
#define TR_ANTENNA_MODES_3           "На модель"
#define TR_ANTENNA_MODES_4           "Внутр + Внеш"
#define TR_ANTENNA_MODES_5           "Внеш"
#define TR_USE_INTERNAL_ANTENNA        TR("Исп внут антенну", "Испол внутр антенну")
#define TR_USE_EXTERNAL_ANTENNA        TR("Исп внеш антенну", "Испол внеш антенну")
#define TR_ANTENNACONFIRM2             TR("Проверьте антенну", "Проверьте антенну!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Требует не FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Требует FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Требует EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "Сертифиц ПО"
#define TR_LOWALARM                    "Низкий сигнал"
#define TR_CRITICALALARM               "Критич сигнал"
#define TR_DISABLE_ALARM               TR("Откл сигналы", "Откл сигналы")
#define TR_POPUP                       "Всплыв"
#define TR_MIN                         "Мин"
#define TR_MAX                         "Макс"
#define TR_CURVE_PRESET                "Предустановка..."
#define TR_PRESET                      "Предустановка"
#define TR_MIRROR                      "Зеркало"
#define TR_CLEAR                       "Очистить"
#define TR_CLEAR_BTN                   BUTTON("Очистить")
#define TR_RESET                       "Сброс"
#define TR_RESET_SUBMENU               "Сброс..."
#define TR_COUNT                       "Колличество"
#define TR_PT                          "Шт"
#define TR_PTS                         "Штк"
#define TR_SMOOTH                      "Плавно"
#define TR_COPY_STICKS_TO_OFS          TR("Копир стик->субтрим", "Копир стики в субтрим")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Коп мин/макс во все вых", "Коп мин/макс/центр во все вых")
#define TR_COPY_TRIMS_TO_OFS           TR("Коп трим->субтрим", "Коп тримы в субтрим")
#define TR_INCDEC                      "Увел/Умен"
#define TR_GLOBALVAR                   "Глоб переменная"
#define TR_MIXSOURCE                   "Источ (%)"
#define TR_MIXSOURCERAW                "Источ (Значен)"
#define TR_CONSTANT                    "Постоянное значение"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1 "Выкл"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2 "Вкл"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3 "Авто"
#define TR_PREFLIGHT                   "Предстарт проверки"
#define TR_CHECKLIST                   TR("Контр список", "Контр список")
#define TR_CHECKLIST_INTERACTIVE       TR3("C-интерактив", "Интерактив checklist", "Интерактивный checklist")
#define TR_AUX_SERIAL_MODE             "Послед порт"
#define TR_AUX2_SERIAL_MODE            "Послед порт 2"
#define TR_AUX_SERIAL_PORT_POWER       "Мощность порта"
#define TR_SCRIPT                      "Скрипт"
#define TR_INPUTS                      "Входные"
#define TR_OUTPUTS                     "Выходные"
#define TR_CONFIRMRESET                TR("Удалить все?", "Удалить все?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Много Lua-скриптов!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Режим питания"
#define TR_SPORT_UPDATE_POWER_MODES_1  "Авто"
#define TR_SPORT_UPDATE_POWER_MODES_2  "Вкл"
#define TR_NO_TELEMETRY_SCREENS        "Нет экранов телем"
#define TR_TOUCH_PANEL                 "Сенсорная панель:"
#define TR_FILE_SIZE                   "Размер файла"
#define TR_FILE_OPEN                   "Все равно откр?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Имя"
#define TR_PHASES_HEADERS_SW           "Тумблер"
#define TR_PHASES_HEADERS_RUD_TRIM     "Трим руля"
#define TR_PHASES_HEADERS_ELE_TRIM     "Трим высоты"
#define TR_PHASES_HEADERS_THT_TRIM     "Трим газа"
#define TR_PHASES_HEADERS_AIL_TRIM     "Трим элеронов"
#define TR_PHASES_HEADERS_CH5_TRIM     "Трим 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Трим 6"
#define TR_PHASES_HEADERS_FAD_IN       "Плавн появлен"
#define TR_PHASES_HEADERS_FAD_OUT      "Плавн исчезнов"

#define TR_LIMITS_HEADERS_NAME         "Имя"
#define TR_LIMITS_HEADERS_SUBTRIM      "Субтрим"
#define TR_LIMITS_HEADERS_MIN          "Мин"
#define TR_LIMITS_HEADERS_MAX          "Макс"
#define TR_LIMITS_HEADERS_DIRECTION    "Направление"
#define TR_LIMITS_HEADERS_CURVE        "Кривая"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Центр"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Режим субтрим"
#define TR_INVERTED                    "Перевернутый"

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "Первый канал"
#define TR_FILL_BACKGROUND           "Заполнить фон?"
#define TR_BG_COLOR                  "Цвет фона"
#define TR_SLIDERS_TRIMS             "Тумблеры+Трим"
#define TR_SLIDERS                   "Тумблеры"
#define TR_FLIGHT_MODE               "Режим полета"
#define TR_INVALID_FILE              "Недопустимый файл"
#define TR_TIMER_SOURCE              "Источник таймера"
#define TR_SIZE                      "Размер"
#define TR_SHADOW                    "Тень"
#define TR_ALIGNMENT                 "Вырав"
#define TR_ALIGN_LABEL               "Вырав метки"
#define TR_ALIGN_VALUE               "Вырав значения"
#define TR_ALIGN_OPTS_1              "Слева"
#define TR_ALIGN_OPTS_2              "По центру"
#define TR_ALIGN_OPTS_3              "Справа"
#define TR_TEXT                      "Текст"
#define TR_COLOR                     "Цвет"
#define TR_PANEL1_BACKGROUND         "Фон панели 1"
#define TR_PANEL2_BACKGROUND         "Фон панели 2"
#define TR_PANEL_BACKGROUND          "Background"
#define TR_PANEL_COLOR               "  Color"
#define TR_WIDGET_GAUGE              "Шкала"
#define TR_WIDGET_MODELBMP           "Информация о модели"
#define TR_WIDGET_OUTPUTS            "Выходы"
#define TR_WIDGET_TEXT               "Текст"
#define TR_WIDGET_TIMER              "Таймер"
#define TR_WIDGET_VALUE              "Значение"

// About screen
#define TR_ABOUTUS                     TR("ИНФОРМАЦИЯ", "ИНФОРМАЦИЯ")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Громк сигнала"
#define TR_WAV_VOLUME                  "Громкость WAV"
#define TR_BG_VOLUME                   TR("Гром фон зв", "Гром фон зв")

#define TR_TOP_BAR                     "Верх панель"
#define TR_FLASH_ERASE                 "Стирание флеш..."
#define TR_FLASH_WRITE                 "Запись в флеш..."
#define TR_OTA_UPDATE                  "OTA-обновление..."
#define TR_MODULE_RESET                "Сброс модуля..."
#define TR_UNKNOWN_RX                  "Неизвес приёмник"
#define TR_UNSUPPORTED_RX              "Непод приёмник"
#define TR_OTA_UPDATE_ERROR            "Ошибка OTA-обновл"
#define TR_DEVICE_RESET                "Сброс устройства..."
#define TR_ALTITUDE                    "Высота"
#define TR_SCALE                       "Масштаб"
#define TR_VIEW_CHANNELS               "Просмотр каналов"
#define TR_VIEW_NOTES                  "Просмотр заметок"
#define TR_MODEL_SELECT                "Выбор модели"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Точность"
#define TR_RATIO                       "Коэффициент"
#define TR_FORMULA                     "Формула"
#define TR_CELLINDEX                   "Инд. ячейки"
#define TR_LOGS                        "Логи"
#define TR_OPTIONS                     "Опции"
#define TR_FIRMWARE_OPTIONS            BUTTON("Опции ПО")

#define TR_ALTSENSOR                   "Датчик высот"
#define TR_CELLSENSOR                  "Датчик АКБ"
#define TR_GPSSENSOR                   "Датчик GPS"
#define TR_CURRENTSENSOR               "Датчик тока"
#define TR_AUTOOFFSET                  "Авто смещ"
#define TR_ONLYPOSITIVE                "Положител"
#define TR_FILTER                      "Фильтр"
#define TR_TELEMETRYFULL               TR("Слоты заняты!", "Слоты заняты!")
#define TR_IGNORE_INSTANCE             TR("Нет инстанса", "Игнор инстансы")
#define TR_SHOW_INSTANCE_ID            "Показать ID инстанса"
#define TR_DISCOVER_SENSORS            "Поиск нов датч"
#define TR_STOP_DISCOVER_SENSORS       "Остановить"
#define TR_DELETE_ALL_SENSORS          "Удалить все"
#define TR_CONFIRMDELETE               "Удалить все?"
#define TR_SELECT_WIDGET               "Выбрать виджет"
#define TR_WIDGET_FULLSCREEN           "Полноэкран режим"
#define TR_REMOVE_WIDGET               "Удалить виджет"
#define TR_WIDGET_SETTINGS             "Настройки виджета"
#define TR_REMOVE_SCREEN               "Удалить экран"
#define TR_SETUP_WIDGETS               "Настройка виджетов"
#define TR_THEME                       "Тема"
#define TR_SETUP                       "Настройка"
#define TR_LAYOUT                      "Макет"
#define TR_TEXT_COLOR                  "Цвет текста"
#define TR_MENU_INPUTS                 CHAR_INPUT "Входы"
#define TR_MENU_LUA                    CHAR_LUA "Скрипты Lua"
#define TR_MENU_STICKS                 CHAR_STICK "Стики"
#define TR_MENU_POTS                   CHAR_POT "Потенц"
#define TR_MENU_MIN                    CHAR_FUNCTION "Мин"
#define TR_MENU_MAX                    CHAR_FUNCTION "Макс"
#define TR_MENU_HELI                   CHAR_CYC "Циклич управл"
#define TR_MENU_TRIMS                  CHAR_TRIM "Триммеры"
#define TR_MENU_SWITCHES               CHAR_SWITCH "Тумблеры"
#define TR_MENU_LOGICAL_SWITCHES       CHAR_SWITCH "Логич тумблеры"
#define TR_MENU_TRAINER                CHAR_TRAINER "Тренажер"
#define TR_MENU_CHANNELS               CHAR_CHANNEL "Каналы"
#define TR_MENU_GVARS                  CHAR_SLIDER "Глоб перем"
#define TR_MENU_TELEMETRY              CHAR_TELEMETRY "Телеметрия"
#define TR_MENU_DISPLAY                "ДИСПЛЕЙ"
#define TR_MENU_OTHER                  "Другое"
#define TR_MENU_INVERT                 "Инвертир"
#define TR_AUDIO_MUTE                  TR("Выкл звук", "Выкл звук")
#define TR_JITTER_FILTER               "Фильтр АЦП"
#define TR_DEAD_ZONE                   "Зона нечувств"
#define TR_RTC_CHECK                   TR("Проверка RTC", "Проверка RTC")
#define TR_AUTH_FAILURE                "Ошибка аутентиф"
#define TR_RACING_MODE                 "Режим гонок"

#define TR_USE_THEME_COLOR              "Используйте цвет темы"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Добав все трим в субтрим"
#define TR_DUPLICATE                    "Дублировать"
#define TR_ACTIVATE                     "Активация"
#define TR_RED                          "Красная"
#define TR_BLUE                         "Синяяя"
#define TR_GREEN                        "Зеленая"
#define TR_COLOR_PICKER                 "Средство выбора цвета"
#define TR_FIXED                        "Fixed"
#define TR_EDIT_THEME_DETAILS           "Редакт детали темы"
#define TR_THEME_COLOR_DEFAULT          "По умолчанию"
#define TR_THEME_COLOR_PRIMARY1         "ОСНОВНОЙ1"
#define TR_THEME_COLOR_PRIMARY2         "ОСНОВНОЙ2"
#define TR_THEME_COLOR_PRIMARY3         "ОСНОВНОЙ3"
#define TR_THEME_COLOR_SECONDARY1       "ВТОРОСТЕП1"
#define TR_THEME_COLOR_SECONDARY2       "ВТОРОСТЕП2"
#define TR_THEME_COLOR_SECONDARY3       "ВТОРОСТЕП3"
#define TR_THEME_COLOR_FOCUS            "ФОКУС"
#define TR_THEME_COLOR_EDIT             "РЕДАК-ТЬ"
#define TR_THEME_COLOR_ACTIVE           "АКТИВНАЯ"
#define TR_THEME_COLOR_WARNING          "ПРЕД-НИЕ"
#define TR_THEME_COLOR_DISABLED         "ОТКЛ-НАЯ"
#define TR_THEME_COLOR_CUSTOM           "ПОЛ-КАЯ"
#define TR_THEME_CHECKBOX               "Checkbox"
#define TR_THEME_ACTIVE                 "АКТИВНАЯ"
#define TR_THEME_REGULAR                "РЕГ-НАЯ"
#define TR_THEME_WARNING                "ПРЕД-НИЕ"
#define TR_THEME_DISABLED               "ОТКЛ-НАЯ"
#define TR_THEME_EDIT                   "РЕДАК-ТЬ"
#define TR_THEME_FOCUS                  "Фокус"
#define TR_AUTHOR                       "Автор"
#define TR_DESCRIPTION                  "Описание"
#define TR_SAVE                         "Сохранить"
#define TR_CANCEL                       "Отменить"
#define TR_EDIT_THEME                   "РЕДАК ТЕМУ"
#define TR_DETAILS                      "Детали"

// Voice in native language
#define TR_VOICE_ENGLISH                TR("Англ","Английский")
#define TR_VOICE_CHINESE                TR("Китайс","Китайский")
#define TR_VOICE_CZECH                  "Чешский"
#define TR_VOICE_DANISH                 "Датский"
#define TR_VOICE_DEUTSCH                TR("Немец","Немецкий")
#define TR_VOICE_DUTCH                  TR("Голланд","Голландск")
#define TR_VOICE_ESPANOL                TR("Испанс","Испанский")
#define TR_VOICE_FINNISH                "Finnish"
#define TR_VOICE_FRANCAIS               TR("Францу","Французск")
#define TR_VOICE_HUNGARIAN              TR("Венгер","Венгерский")
#define TR_VOICE_ITALIANO               TR("Итальян","Итальянск")
#define TR_VOICE_POLISH                 TR("Польск","Польский")
#define TR_VOICE_PORTUGUES              TR("Порту","Португальск")
#define TR_VOICE_RUSSIAN                "Русский"
#define TR_VOICE_SLOVAK                 TR("Словацк","Словацкий")
#define TR_VOICE_SWEDISH                "Шведский"
#define TR_VOICE_TAIWANESE              TR("Тайван","Тайваньский")
#define TR_VOICE_JAPANESE               "Японский"
#define TR_VOICE_HEBREW                 "Иврит"
#define TR_VOICE_UKRAINIAN              TR("Украинс","Украинский")
#define TR_VOICE_KOREAN                 "корейский"

#define TR_USBJOYSTICK_LABEL           "USB Джойс"
#define TR_USBJOYSTICK_EXTMODE         "Режим"
#define TR_VUSBJOYSTICK_EXTMODE_1      "Обычный"
#define TR_VUSBJOYSTICK_EXTMODE_2      "Расшир"
#define TR_USBJOYSTICK_SETTINGS        BUTTON("Настр канала")
#define TR_USBJOYSTICK_IF_MODE         TR("Интерф","Интерф")
#define TR_VUSBJOYSTICK_IF_MODE_1      "Джойс"
#define TR_VUSBJOYSTICK_IF_MODE_2      "Геймпад"
#define TR_VUSBJOYSTICK_IF_MODE_3      "Многоз"
#define TR_USBJOYSTICK_CH_MODE         "Режим"
#define TR_VUSBJOYSTICK_CH_MODE_1      "Нет"
#define TR_VUSBJOYSTICK_CH_MODE_2      "Кнопка"
#define TR_VUSBJOYSTICK_CH_MODE_3      "Ось"
#define TR_VUSBJOYSTICK_CH_MODE_4      "Симул"
#define TR_VUSBJOYSTICK_CH_MODE_S_1    "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2    "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3    "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4    "S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Режим Кнопки"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1   "Обыч"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2   "Импульс"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3   "Эмул перек"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4   "Дельта"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5   "Комп"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1 TR("Норм","Нормально")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2 TR("Пулс","Пульс")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3 TR("SWEm","SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4 TR("Делт","Дельта")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5 TR("КМП","Компаньон")
#define TR_USBJOYSTICK_CH_SWPOS        "Положения"
#define TR_VUSBJOYSTICK_CH_SWPOS_1     "Кноп"
#define TR_VUSBJOYSTICK_CH_SWPOS_2     "2ПОЗ"
#define TR_VUSBJOYSTICK_CH_SWPOS_3     "3ПОЗ"
#define TR_VUSBJOYSTICK_CH_SWPOS_4     "4ПОЗ"
#define TR_VUSBJOYSTICK_CH_SWPOS_5     "5ПОЗ"
#define TR_VUSBJOYSTICK_CH_SWPOS_6     "6ПОЗ"
#define TR_VUSBJOYSTICK_CH_SWPOS_7     "7ПОЗ"
#define TR_VUSBJOYSTICK_CH_SWPOS_8     "8ПОЗ"
#define TR_USBJOYSTICK_CH_AXIS         "Ось"
#define TR_VUSBJOYSTICK_CH_AXIS_1       "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2       "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3       "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4       "rotX"
#define TR_VUSBJOYSTICK_CH_AXIS_5       "rotY"
#define TR_VUSBJOYSTICK_CH_AXIS_6       "rotZ"
#define TR_VUSBJOYSTICK_CH_AXIS_7       "Slider"
#define TR_VUSBJOYSTICK_CH_AXIS_8       "Dial"
#define TR_VUSBJOYSTICK_CH_AXIS_9       "Wheel"
#define TR_USBJOYSTICK_CH_SIM          "Симуляция оси"
#define TR_VUSBJOYSTICK_CH_SIM_1       "Ail"
#define TR_VUSBJOYSTICK_CH_SIM_2       "Ele"
#define TR_VUSBJOYSTICK_CH_SIM_3       "Rud"
#define TR_VUSBJOYSTICK_CH_SIM_4       "Thr"
#define TR_VUSBJOYSTICK_CH_SIM_5       "Acc"
#define TR_VUSBJOYSTICK_CH_SIM_6       "Brk"
#define TR_VUSBJOYSTICK_CH_SIM_7       "Steer"
#define TR_VUSBJOYSTICK_CH_SIM_8       "Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Инверсия"
#define TR_USBJOYSTICK_CH_BTNNUM       "Номер кнопки"
#define TR_USBJOYSTICK_BTN_COLLISION   "!Конфликт ном кнопок!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Конфликт осей!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Круг. вырез", "Круглый вырез")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1 "нет"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2 "X-Y, Z-rotX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3 "X-Y, rotX-rotY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4 "X-Y, Z-rotZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   BUTTON("Применить изменения")

#define TR_DIGITAL_SERVO          "Серво 333HZ"
#define TR_ANALOG_SERVO           "Серво 50HZ"
#define TR_SIGNAL_OUTPUT          "Выход сигнал"
#define TR_SERIAL_BUS             "Последов шина"
#define TR_SYNC                   "Синхро"

#define TR_ENABLED_FEATURES       "Включ функции"
#define TR_RADIO_MENU_TABS        "Вкладки меню пульта"
#define TR_MODEL_MENU_TABS        "Вкладки меню модели"

#define TR_SELECT_MENU_ALL        "Все"
#define TR_SELECT_MENU_CLR        "Очист"
#define TR_SELECT_MENU_INV        "Перев"

#define TR_SORT_ORDERS_1          "Имя A-Z"
#define TR_SORT_ORDERS_2          "Имя Z-A"
#define TR_SORT_ORDERS_3          "Редко испол"
#define TR_SORT_ORDERS_4          "Часто испол"
#define TR_SORT_MODELS_BY         "Сортировка моделей"
#define TR_CREATE_NEW             "Создать"

#define TR_MIX_SLOW_PREC          TR("Точн замедл", "Точность замедления")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "Уже есть каталог с таким названием"

#define TR_DATE_TIME_WIDGET       "Дата и время"
#define TR_RADIO_INFO_WIDGET      "Инфо о пульте"
#define TR_LOW_BATT_COLOR         "Разряженная АКБ"
#define TR_MID_BATT_COLOR         "Середина АКБ "
#define TR_HIGH_BATT_COLOR        "Заряженная АКБ"

#define TR_WIDGET_SIZE            "Размер виджета"

#define TR_DEL_DIR_NOT_EMPTY      "Перед удал каталог должен быть пустым"
