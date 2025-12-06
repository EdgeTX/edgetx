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

// SK translations author: Brano Mate <brano.mate@gmail.com>


/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 */

// Main menu
#define TR_QM_MANAGE_MODELS             "Správa\nModelov"
#define TR_QM_MODEL_SETUP               "Model\nNastavenia"
#define TR_QM_RADIO_SETUP               "Rádio\nNastavenia"
#define TR_QM_UI_SETUP                  "UI\nNastavenia"
#define TR_QM_TOOLS                     "Nástroje"
#define TR_QM_GEN_SETTINGS              "Hlavné\nNastavenia"
#define TR_QM_FLIGHT_MODES              TR_SFC_AIR("Drive\nModes", "Letové\nRežimy")
#define TR_QM_INPUTS                    "Vstupy"
#define TR_QM_MIXES                     "Mixy"
#define TR_QM_OUTPUTS                   "Výstupy"
#define TR_QM_CURVES                    "Krivky"
#define TR_QM_GLOBAL_VARS               "Globálne\nPremenné"
#define TR_QM_LOGICAL_SW                "Logické\nSpínače"
#define TR_QM_SPEC_FUNC                 "Špeciálne\nFunkcie"
#define TR_QM_CUSTOM_LUA                "Vlastné\nSkripty"
#define TR_QM_TELEM                     "Telemetria"
#define TR_QM_GLOB_FUNC                 "Globálne\nFunkcie"
#define TR_QM_TRAINER                   "Tréner"
#define TR_QM_HARDWARE                  "Hardware"
#define TR_QM_ABOUT                     "O\nEdgeTX"
#define TR_QM_THEMES                    "Témy"
#define TR_QM_TOP_BAR                   "Horná Lišta"
#define TR_QM_SCREEN_1                  "Obrazovka 1"
#define TR_QM_SCREEN_2                  "Obrazovka 2"
#define TR_QM_SCREEN_3                  "Obrazovka 3"
#define TR_QM_SCREEN_4                  "Obrazovka 4"
#define TR_QM_SCREEN_5                  "Obrazovka 5"
#define TR_QM_SCREEN_6                  "Obrazovka 6"
#define TR_QM_SCREEN_7                  "Obrazovka 7"
#define TR_QM_SCREEN_8                  "Obrazovka 8"
#define TR_QM_SCREEN_9                  "Obrazovka 9"
#define TR_QM_SCREEN_10                 "Obrazovka 10"
#define TR_QM_ADD_SCREEN                "Pridať\nObrazovku"
#define TR_QM_APPS                      "Aplikácie"
#define TR_QM_STORAGE                   "Úložisko"
#define TR_QM_RESET                     TR_SFC_AIR("Drive\nReset", "Flight\nReset")
#define TR_QM_CHAN_MON                  "Kanál\nMonitor"
#define TR_QM_LS_MON                    "LS\nMonitor"
#define TR_QM_STATS                     "Štatistiky"
#define TR_QM_DEBUG                     "Debug"
#define TR_MAIN_MENU_SETTINGS           "Hlavné nastavenia"
#define TR_MAIN_MENU_MANAGE_MODELS      "Správa modelov"
#define TR_MAIN_MENU_MODEL_NOTES        "Poznámky modelu"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Monitor kanálov"
#define TR_MONITOR_SWITCHES             "Monitor logických spínačov"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Nastavenie modelu"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Nastavenie rádia"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "UI Nastavenia"
#define TR_MAIN_MENU_STATISTICS         "Štatistiky"
#define TR_MAIN_MENU_ABOUT_EDGETX       "O EdgeTX"
#define TR_MAIN_VIEW_X                  "Obrazovka "
#define TR_MAIN_MENU_THEMES             "Motívy"
#define TR_MAIN_MENU_APPS               "Aplikácie"
#define TR_MENUHELISETUP                "HELI"
#define TR_MENUFLIGHTMODES             TR_SFC_AIR("REŽIM JAZDA", "LETOVÉ REŽIMY")
#define TR_MENUFLIGHTMODE              TR_SFC_AIR("REŽIM JAZDA", "LETOVÝ REŽIM")
#define TR_MENUINPUTS                  "Vstupy"
#define TR_MENULIMITS                  "Servá"
#define TR_MIXES                       "Mixy"
#define TR_MENU_GLOBAL_VARS            "Globálne Premenné"
#define TR_MENULOGICALSWITCHES         "Logické Spínače"
#define TR_MENUCUSTOMFUNC              "Špeciálne funkcie"
#define TR_MENUCUSTOMSCRIPTS           "Skripty LUA"
#define TR_MENUTELEMETRY               "Telemetria"
#define TR_MENUSPECIALFUNCS            "Globálne Funkcie"
#define TR_MENUTRAINER                 "Tréner"
#define TR_HARDWARE                    "Hardvér"
#define TR_USER_INTERFACE              "Top Bar"
#define TR_SD_CARD                     "SD"
#define TR_DEBUG                       "Debug"
#define TR_MENU_RADIO_SWITCHES         "DIAG"
#define TR_MENUCALIBRATION             "Kalibrácia"
#define TR_FUNCTION_SWITCHES           "Nastaviteľné prepínače"
// End Main menu

#define TR_MINUTE_SINGULAR             "minúta"
#define TR_MINUTE_PLURAL1              "minúty"
#define TR_MINUTE_PLURAL2              "minút"

// NON ZERO TERMINATED STRINGS
#define TR_OFFON_1                     "VYP"
#define TR_OFFON_2                     "ZAP"
#define TR_MMMINV_1                    "---"
#define TR_MMMINV_2                    "INV"
#define TR_VBEEPMODE_1                 "Tichý"
#define TR_VBEEPMODE_2                 TR("Alarm","Len Alarm")
#define TR_VBEEPMODE_3                 TR("BezKl","Bez kláves")
#define TR_VBEEPMODE_4                 "Všetko"
#define TR_COUNTDOWNVALUES_1           "5s"
#define TR_COUNTDOWNVALUES_2           "10s"
#define TR_COUNTDOWNVALUES_3           "20s"
#define TR_COUNTDOWNVALUES_4           "30s"
#define TR_VBLMODE_1                   TR("Vyp","Vypnuté")
#define TR_VBLMODE_2                   TR("Kláv.","Klávesy")
#define TR_VBLMODE_3                   "Páky"
#define TR_VBLMODE_4                   "Všetko"
#define TR_VBLMODE_5                   TR("Zap","Zapnuté")
#define TR_TRNMODE_1                   "X"
#define TR_TRNMODE_2                   "Sčítať"
#define TR_TRNMODE_3                   "Zameniť"
#define TR_TRNCHN_1                    "CH1"
#define TR_TRNCHN_2                    "CH2"
#define TR_TRNCHN_3                    "CH3"
#define TR_TRNCHN_4                    "CH4"
#define TR_AUX_SERIAL_MODES_1          "VYP"
#define TR_AUX_SERIAL_MODES_2          "Telemetria Zrkadlenia"
#define TR_AUX_SERIAL_MODES_3          "Telemetria Vstup"
#define TR_AUX_SERIAL_MODES_4          "SBUS Trenér"
#define TR_AUX_SERIAL_MODES_5          "LUA"
#define TR_AUX_SERIAL_MODES_6          "CLI"
#define TR_AUX_SERIAL_MODES_7          "GPS"
#define TR_AUX_SERIAL_MODES_8          "Ladenie"
#define TR_AUX_SERIAL_MODES_9          "SpaceMouse"
#define TR_AUX_SERIAL_MODES_10         "Externý modul"
#define TR_SWTYPES_1                   "Žiadny"
#define TR_SWTYPES_2                   "Bez aretácie"
#define TR_SWTYPES_3                   "2-polohový"
#define TR_SWTYPES_4                   "3-polohový"
#define TR_SWTYPES_5                   "Globálny"
#define TR_POTTYPES_1                  "Žiadny"
#define TR_POTTYPES_2                  TR("Pot","Potenciometer")
#define TR_POTTYPES_3                  TR("Pot s aret.","Pot s aretáciou
#define TR_POTTYPES_5                  TR("Viacpol pr.","Viacpolohový prepínač")
#define TR_POTTYPES_6                  "Os X"
#define TR_POTTYPES_7                  "Os Y"
#define TR_POTTYPES_8                  "Prepínač"
#define TR_VPERSISTENT_1               "Nie"
#define TR_VPERSISTENT_2               "V rámci letu"
#define TR_VPERSISTENT_3               "Reset ručne"
#define TR_COUNTRY_CODES_1             TR("US","Amerika")
#define TR_COUNTRY_CODES_2             TR("JP","Japonsko")
#define TR_COUNTRY_CODES_3             TR("EU","Európa")
#define TR_USBMODES_1                  TR("Spýtať","Spýtať sa")
#define TR_USBMODES_2                  TR("Joyst","Joystick")
#define TR_USBMODES_3                  TR("SDkarta","Úložisko")
#define TR_USBMODES_4                  "Serial"
#define TR_JACK_MODES_1                "Spýtať"
#define TR_JACK_MODES_2                "Audio"
#define TR_JACK_MODES_3                "Trenér"

#define TR_SBUS_INVERSION_VALUES_1     "Normál"
#define TR_SBUS_INVERSION_VALUES_2     "Neinvertované"
#define TR_MULTI_CUSTOM                "Vlastný"
#define TR_VTRIMINC_1                  "Expo"
#define TR_VTRIMINC_2                  "ExJemný"
#define TR_VTRIMINC_3                  "Jemný"
#define TR_VTRIMINC_4                  "Stredný"
#define TR_VTRIMINC_5                  "Hrubý"
#define TR_VDISPLAYTRIMS_1             "Nie"
#define TR_VDISPLAYTRIMS_2             "Zmena"
#define TR_VDISPLAYTRIMS_3             "Ano"
#define TR_VBEEPCOUNTDOWN_1            "Nie"
#define TR_VBEEPCOUNTDOWN_2            "Zvuk"
#define TR_VBEEPCOUNTDOWN_3            "Hlas"
#define TR_VBEEPCOUNTDOWN_4            "Vibrácie"
#define TR_VBEEPCOUNTDOWN_5            TR("Zv & Vib","Zvuk & Vibrace")
#define TR_VBEEPCOUNTDOWN_6            TR("Hl & Vib", "Hlas & Vibrace")
#define TR_VVARIOCENTER_1              "Tón"
#define TR_VVARIOCENTER_2              "Ticho"
#define TR_CURVE_TYPES_1               "Raster-X"
#define TR_CURVE_TYPES_2               "Voľná-XY"

#define TR_ADCFILTERVALUES_1           "Globálna"
#define TR_ADCFILTERVALUES_2           "Vyp"
#define TR_ADCFILTERVALUES_3           "Zap"

#define TR_VCURVETYPE_1                "Diff"
#define TR_VCURVETYPE_2                "Expo"
#define TR_VCURVETYPE_3                "Funk"
#define TR_VCURVETYPE_4                "Vlas"
#define TR_VMLTPX_1                    "Sčítať"
#define TR_VMLTPX_2                    "Násobiť"
#define TR_VMLTPX_3                    "Zameniť"

#define TR_CSWTIMER                    TR("Tim", "Stopky")
#define TR_CSWSTICKY                   TR("Stky", "Páky")
#define TR_CSWSTAY                     "Edge"

#define TR_SF_TRAINER                  "Tréner"
#define TR_SF_INST_TRIM                "Insta-Trim"
#define TR_SF_RESET                    "Reset"
#define TR_SF_SET_TIMER                "Zmena"
#define TR_SF_VOLUME                   "Hlasitosť"
#define TR_SF_FAILSAFE                 "Nastaviť Failsafe"
#define TR_SF_RANGE_CHECK              "Kontrola dosahu"
#define TR_SF_MOD_BIND                 "Párovanie Modulu"
#define TR_SF_RGBLEDS                  "RGB svetlo"

#define TR_SOUND                       TR_BW_COL("\204\205Zvuk", "Prehrať zvuk")
#define TR_PLAY_TRACK                  TR_BW_COL("\204\205Stopa", "Prehrať wav")
#define TR_PLAY_VALUE                  TR_BW_COL("\204\205Hlásiť ", "Hlásiť stav")
#define TR_SF_HAPTIC                   "Vibrovať"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua Skript")
#define TR_SF_BG_MUSIC                 TR_BW_COL("\204\205Hudba","Hudba")
#define TR_SF_BG_MUSIC_PAUSE           TR_BW_COL("\204\205Hudba ||","Hudba pauza")
#define TR_SF_LOGS                     "Loguj na SD"
#define TR_ADJUST_GVAR                 "Nastav"
#define TR_SF_BACKLIGHT                "Podsvietenie"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_SAFETY                   "Zámok"
#define TR_SF_TEST                     "Test"

#define TR_SF_SCREENSHOT               "Snímka LCD"
#define TR_SF_RACING_MODE              "Pretekársky režim"
#define TR_SF_DISABLE_TOUCH            "Deaktivácia dotyku"
#define TR_SF_DISABLE_AUDIO_AMP        "Vypnutie zosilňovača zvuku"
#define TR_SF_SET_SCREEN               TR_BW_COL("Nast obrazovku", "Vybrať hlavnú obrazovku")
#define TR_SF_PUSH_CUST_SWITCH         "Stlač CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Telm","Telemetria")

#define TR_FSW_RESET_TRIMS             "Trimre"
#define TR_FSW_RESET_TIMERS_1          TR("Čas1", "Stopky1")
#define TR_FSW_RESET_TIMERS_2          TR("Čas2", "Stopky2")
#define TR_FSW_RESET_TIMERS_3          TR("Čas3", "Stopky3")

#define TR_VFSWRESET_1                 TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                 TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                 TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                 "Všetko"
#define TR_VFSWRESET_5                 TR_FSW_RESET_TELEM
#define TR_VFSWRESET_6                 TR_FSW_RESET_TRIMS
#define TR_FUNCSOUNDS_1                TR("Píp1","Pípnutie1")
#define TR_FUNCSOUNDS_2                TR("Píp2","Pípnutie2")
#define TR_FUNCSOUNDS_3                TR("Píp3","Pípnutie3")
#define TR_FUNCSOUNDS_4                TR("Var1","Varovanie1")
#define TR_FUNCSOUNDS_5                TR("Var2","Varovanie2")
#define TR_FUNCSOUNDS_6                TR("Chee","Cheep")
#define TR_FUNCSOUNDS_7                TR("Rata", "Ratata")
#define TR_FUNCSOUNDS_8                "Tick"
#define TR_FUNCSOUNDS_9                TR("Sirn","Siréna")
#define TR_FUNCSOUNDS_10               "Zvonček"
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

#define TR_VUNITSSYSTEM_1              TR("Metr.","Metrické")
#define TR_VUNITSSYSTEM_2              TR("Imper.","Imperiálne")
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

#define TR_VTELEMSCREENTYPE_1          "Nič"
#define TR_VTELEMSCREENTYPE_2          "Hodnota"
#define TR_VTELEMSCREENTYPE_3          "Ukazovateľ"
#define TR_VTELEMSCREENTYPE_4          "Skript"
#define TR_GPSFORMAT_1                 "DMS"
#define TR_GPSFORMAT_2                 "NMEA"


#define TR_VSWASHTYPE_1                "---"
#define TR_VSWASHTYPE_2                "120"
#define TR_VSWASHTYPE_3                "120X"
#define TR_VSWASHTYPE_4                "140"
#define TR_VSWASHTYPE_5                "90"

#define TR_STICK_NAMES0                "Sme"
#define TR_STICK_NAMES1                "Výš"
#define TR_STICK_NAMES2                "Pln"
#define TR_STICK_NAMES3                "Kri"
#define TR_SURFACE_NAMES0              "Sme"
#define TR_SURFACE_NAMES1              "Pln"

#define TR_ON_ONE_SWITCHES_1           "ZAP"
#define TR_ON_ONE_SWITCHES_2           "One"

#define TR_HATSMODE                    "Režim klobúkov"
#define TR_HATSOPT_1                   "Len trimre"
#define TR_HATSOPT_2                   "Len tlačítka"
#define TR_HATSOPT_3                   "Perpínatelné"
#define TR_HATSOPT_4                   "Globálne"
#define TR_HATSMODE_TRIMS              "Režim klobúkov: Trimre"
#define TR_HATSMODE_KEYS               "Režim klobúkov: Tlačítka"
#define TR_HATSMODE_KEYS_HELP          "Ľavá strana:\n"\
                                       " Vpravo = MDL\n"\
                                       " Hore = SYS\n"\
                                       " Dole = TELE\n"\
                                       "\n"\
                                       "Pravá strana:\n"\
                                       " Vľavo = LIST<\n"\
                                       " Vpravo = LIST>\n"\
                                       " Hore = PRED/ZVÝŠ\n"\
                                       " Dole = ĎALŠÍ/ZNÍŽ"

#define TR_ROTARY_ENC_OPT_1            "Normál"
#define TR_ROTARY_ENC_OPT_2            "Invertovaný"
#define TR_ROTARY_ENC_OPT_3            "V-I H-N"
#define TR_ROTARY_ENC_OPT_4            "V-I H-A"
#define TR_ROTARY_ENC_OPT_5            "V-N E-I"

#define TR_IMU_VSRCRAW_1               "TltX"
#define TR_IMU_VSRCRAW_2               "TltY"

#define TR_CYC_VSRCRAW_1               "CYC1"
#define TR_CYC_VSRCRAW_2               "CYC2"
#define TR_CYC_VSRCRAW_3               "CYC3"

#define TR_SRC_BATT                    "Bat."
#define TR_SRC_TIME                    "Čas"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Čas"

#define TR_VTMRMODES_1                 "VYP"
#define TR_VTMRMODES_2                 "ZAP"
#define TR_VTMRMODES_3                 "Strt"
#define TR_VTMRMODES_4                 "Pln>"
#define TR_VTMRMODES_5                 "Pln%"
#define TR_VTMRMODES_6                 "Pln*"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Učiteľ/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Žiak/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Učiteľ/SBUS Modul"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Učiteľ/CPPM Modul"
#define TR_VTRAINER_MASTER_BATTERY     "Učiteľ/Serial"
#define TR_VTRAINER_BLUETOOTH_1        TR("Učiteľ/BT","Učiteľ/Bluetooth")
#define TR_VTRAINER_BLUETOOTH_2        TR("Žiak/BT","Žiak/Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VTRAINER_CRSF               "Master/CRSF"
#define TR_VFAILSAFE_1                 "Nenastavené"
#define TR_VFAILSAFE_2                 "Držať"
#define TR_VFAILSAFE_3                 "Vlastný"
#define TR_VFAILSAFE_4                 "Bez pulzov"
#define TR_VFAILSAFE_5                 "Prijímač"
#define TR_VSENSORTYPES_1              "Vlastný"
#define TR_VSENSORTYPES_2              "Vypočítaný"
#define TR_VFORMULAS_1                 "Súčet"
#define TR_VFORMULAS_2                 "Priemer"
#define TR_VFORMULAS_3                 "Min"
#define TR_VFORMULAS_4                 "Max"
#define TR_VFORMULAS_5                 "Násobenie"
#define TR_VFORMULAS_6                 "Celkom"
#define TR_VFORMULAS_7                 "Článok"
#define TR_VFORMULAS_8                 "Spotreba"
#define TR_VFORMULAS_9                 "Vzdialenosť"
#define TR_VPREC_1                     "X"
#define TR_VPREC_2                     "X.X"
#define TR_VPREC_3                     "X.XX"
#define TR_VCELLINDEX_1                "Nízky"
#define TR_VCELLINDEX_2                "1"
#define TR_VCELLINDEX_3                "2"
#define TR_VCELLINDEX_4                "3"
#define TR_VCELLINDEX_5                "4"
#define TR_VCELLINDEX_6                "5"
#define TR_VCELLINDEX_7                "6"
#define TR_VCELLINDEX_8                "7"
#define TR_VCELLINDEX_9                "8"
#define TR_VCELLINDEX_10               "Najviac"
#define TR_VCELLINDEX_11               "Delta"
#define TR_SUBTRIMMODES_1              CHAR_DELTA"(len stredy)"
#define TR_SUBTRIMMODES_2              "= (symetrický)"
#define TR_TIMER_DIR_1                 TR("Zostávajúci", "Zobraziť zostávajúci")
#define TR_TIMER_DIR_2                 TR("Uplynulý", "Zobraziť uplynulý")

#define TR_FONT_SIZES_1                "STD"
#define TR_FONT_SIZES_2                "BOLD"
#define TR_FONT_SIZES_3                "XXS"
#define TR_FONT_SIZES_4                "XS"
#define TR_FONT_SIZES_5                "L"
#define TR_FONT_SIZES_6                "XL"
#define TR_FONT_SIZES_7                "XXL"

#define TR_ENTER                       "[ENTER]"
#define TR_OK                          TR_BW_COL(TR("\010\010\010[OK]", "\010\010\010\010\010[OK]"), "Ok")
#define TR_EXIT                        TR_BW_COL("EXIT", "RTN")

#define TR_YES                         "Áno"
#define TR_NO                          "Nie"
#define TR_DELETEMODEL                 "Zmazať Model"
#define TR_COPYINGMODEL                "Kopírujem model..."
#define TR_MOVINGMODEL                 "Presúvam model..."
#define TR_LOADINGMODEL                "Aktivujem model..."
#define TR_UNLABELEDMODEL              "Bez štítku"
#define TR_NAME                        "Názov"
#define TR_MODELNAME                   TR("Model", "Názov modelu")
#define TR_PHASENAME                   "Názov"
#define TR_MIXNAME                     "Názov"
#define TR_INPUTNAME                   "Názov"
#define TR_EXPONAME                    "Popis"
#define TR_BITMAP                      "Obrázok"
#define TR_NO_PICTURE                  "Bez obrázku"
#define TR_TIMER                       "Stopky"
#define TR_NO_TIMERS                   "Žiadne časovače"
#define TR_START                       "Štart"
#define TR_NEXT                        "Nasledujúci"
#define TR_ELIMITS                     TR("Limit +25%", "Kanál +/- 125%")
#define TR_ETRIMS                      TR("Širší Trim", "Široký trim")
#define TR_TRIMINC                     TR("Krok Trimu", "Krok trimu")
#define TR_DISPLAY_TRIMS               TR("Číslo v Trm", "Číslo v lište trimu")
#define TR_TTRACE                      TR("StopaPlynu", "Stopa plynu")
#define TR_TTRIM                       TR("TrimVolnob.", "Trim len pre voľnobeh")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", "Trim spínač")
#define TR_BEEPCTR                     TR("Pípať stred", "Pípnutie pri stredovej polohe")
#define TR_USE_GLOBAL_FUNCS            TR("Glob.Funkce", "Použiť globálne funkcie")
#define TR_PROTOCOL                    "Protokol"
#define TR_PPMFRAME                    "PPM modulácia"
#define TR_REFRESHRATE                 TR("Obnoviť", "Obn. frekv.")
#define TR_WARN_BATTVOLTAGE            TR("Výstup VBAT: ", "Varovanie: výstupná hodnota VBAT: ")
#define TR_WARN_5VOLTS                 "Varovanie: výstupná úroveň je 5 voltov"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Spínač"
#define TR_FS_COLOR_LIST_1             "Vlastný"
#define TR_FS_COLOR_LIST_2             "Off"
#define TR_FS_COLOR_LIST_3             "Biela"
#define TR_FS_COLOR_LIST_4             "Červená"
#define TR_FS_COLOR_LIST_5             "Zelená"
#define TR_FS_COLOR_LIST_6             "Žltá"
#define TR_FS_COLOR_LIST_7             "Oranžová"
#define TR_FS_COLOR_LIST_8             "Modrá"
#define TR_FS_COLOR_LIST_9             "Ružová"
#define TR_GROUP                       "Skupina"
#define TR_GROUP_ALWAYS_ON             "Vždy Zapnuté"
#define TR_LUA_OVERRIDE                "Povoliť Lua override"
#define TR_GROUPS                      "Vždy na skupiny"
#define TR_LAST                        "Posledný"
#define TR_MORE_INFO                   "Viac Info"
#define TR_SWITCH_TYPE                 "Type"
#define TR_SWITCH_STARTUP              "Startup"
#define TR_SWITCH_GROUP                "Skupina"
#define TR_SF_SWITCH                   "Triger"
#define TR_TRIMS                       "Trimre"
#define TR_FADEIN                      "Prechod Zap"
#define TR_FADEOUT                     "Prechod Vyp"
#define TR_DEFAULT                     "(východzí)"
#define   TR_CHECKTRIMS                 TR_BW_COL("\006Kont.\012Trimre", "Skontrolovať trimre")
#define TR_SWASHTYPE                   "Typ cykliky"
#define TR_COLLECTIVE                  "Kolektív"
#define TR_AILERON                     "Bočná cyklika"
#define TR_ELEVATOR                    "Pozdĺžna cyklika"
#define TR_SWASHRING                   "Cyklika"
#define TR_MODE                        "Mód"
#define TR_LEFT_STICK                  "Vľavo"
#define TR_SUBTYPE                     "Subtyp"
#define TR_NOFREEEXPO                  "Nie je voľné expo!"
#define TR_NOFREEMIXER                 "Nie je voľný mix!"
#define TR_SOURCE                      "Zdroj"
#define TR_WEIGHT                      "Váha"
#define TR_SIDE                        "Strana"
#define TR_OFFSET                      "Ofset"
#define TR_TRIM                        "Trim"
#define TR_CURVE                       "Krivka"
#define TR_FLMODE                      "Režim"
#define TR_MIXWARNING                  "Varovanie"
#define TR_OFF                         "VYP"
#define TR_ANTENNA                     "Anténa"
#define TR_NO_INFORMATION              TR("Nie je info.", "Žiadna Informácia")
#define TR_MULTPX                      TR("Mat.operácia", "Operácia")
#define TR_DELAYDOWN                   TR("Zadržať(x)", "Oneskorenie Vyp")
#define TR_DELAYUP                     TR("Zadržať(\43)", "Oneskorenie Zap")
#define TR_SLOWDOWN                    TR("Spomaľ(\177)", "Spomalenie(-)")
#define TR_SLOWUP                      TR("Spomaľ(\176)", "Spomalenie(+)")
#define TR_CV                          "K"
#define TR_GV                          TR("G", "GP")
#define TR_RANGE                       "Rozsah"
#define TR_CENTER                      "Stred"
#define TR_ALARM                       "Alarm"
#define TR_BLADES                      TR("ListyVrt", "Listy vrtule")
#define TR_SCREEN                      "Panel "
#define TR_SOUND_LABEL                 "Zvuk"
#define TR_LENGTH                      "Dĺžka"
#define TR_BEEP_LENGTH                 TR("Dĺžka", "Dĺžka zvuku")
#define TR_BEEP_PITCH                  "Tón"
#define TR_HAPTIC_LABEL                "Vibrácia"
#define TR_STRENGTH                    "Intenzita"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Offset"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Kontrast LCD"
#define TR_ALARMS_LABEL                "Alarmy"
#define TR_BATTERY_RANGE               TR("Ukazovateľ bat.", "Ukazovateľ batérie")
#define TR_BATTERYCHARGING             "Nabíjanie..."
#define TR_BATTERYFULL                 "Batéria nabitá"
#define TR_BATTERYNONE                 "Žiadna batéria!"
#define TR_BATTERYWARNING              "Vybitá batéria"
#define TR_INACTIVITYALARM             TR("Nečinnosť", "Nečinnosť rádia")
#define TR_MEMORYWARNING               "Plná pamäť"
#define TR_ALARMWARNING                TR("Vypnutý zvuk", "Upozorniť na vypnutý zvuk")
#define TR_RSSI_SHUTDOWN_ALARM         TR("Rssi pri vyp.", "Strážiť RSSI pri vypnutí")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Použiť FlySky RSSI hodnotu bez škálovania")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Tréner vypnutý", "Kontrola trénera pri vypnutí")
#define TR_MODEL_STILL_POWERED         "Model stále spustený"
#define TR_TRAINER_STILL_CONNECTED     "Tréner stále pripojený"
#define TR_USB_STILL_CONNECTED         "USB stále pripojené"
#define TR_MODEL_SHUTDOWN              "Vypnúť?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Stlač ENTER pre potvrdenie"
#define TR_THROTTLE_LABEL              "Plyn"
#define TR_THROTTLE_START              "Plyn Štart"
#define TR_THROTTLEREVERSE             TR("ReverzPlyn", "Reverz plynu")
#define TR_MINUTEBEEP                  TR("Minúta", "Oznamovať minúty")
#define TR_BEEPCOUNTDOWN               "Odpočet"
#define TR_PERSISTENT                  "Trvalé"
#define TR_BACKLIGHT_LABEL             ""
#define TR_GHOST_MENU_LABEL            "Skyté Menu"
#define TR_STATUS                      "Stav"
#define TR_BLONBRIGHTNESS              "Jas zapnutého LCD"
#define TR_BLOFFBRIGHTNESS             "Jas vypnutého LCD"
#define TR_KEYS_BACKLIGHT              "Podsvietenie kláves"
#define TR_BLCOLOR                     "Farba"
#define TR_SPLASHSCREEN                TR("úvodné logo", "Zobrazit úvodné logo")
#define TR_PLAY_HELLO                  "Zvuk pri spustení"
#define TR_PWR_ON_DELAY                "Opozdenie zapnutia"
#define TR_PWR_OFF_DELAY               "Opozdenie vypnutia"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("Zap ON/OFF vibrácie","Zapnutie ON/OFF vibrácie")
#define TR_THROTTLE_WARNING            TR("* Plyn", "Kontrola plynu")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Vlas-Poz", "Vlastná pozícia?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Poz. %", "Pozícia %")
#define TR_SWITCHWARNING               TR("* Spínače", "Polohy spínačov")
#define TR_POTWARNINGSTATE             "Kontrola Pot&Slid."
#define TR_POTWARNING                  TR("* Potenc.", "Potenciometre")
#define TR_TIMEZONE                    "Časové pásmo"
#define TR_ADJUST_RTC                  TR("Čas z GPS", "Použiť čas z GPS")
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Def rad kanálu", "Predefinované radenie kanálu")
#define TR_STICKS                      "Osi"
#define TR_POTS                        "Potenciometre"
#define TR_SWITCHES                    "Spínače"
#define TR_SWITCHES_DELAY              TR("Filter prepínača", "Filter polôh přepínača")
#define TR_SLAVE                       "Podriadený"
#define TR_MODESRC                     "Mód\006% Zdroj"
#define TR_MULTIPLIER                  "Násobič"
#define TR_CAL                         "Kal."
#define TR_CALIBRATION                 BUTTON("Kalibrácia")
#define TR_VTRIM                       "Trim - +"
#define TR_CALIB_DONE                  "Kalibrácia dokončená"
#define TR_MENUTOSTART                 TR_ENTER " = START"
#define TR_MENUWHENDONE                TR_ENTER " > ĎALŠÍ"
#define TR_AXISDIR                     "SMER OSI"
#define TR_MENUAXISDIR                 "[ENTER LONG] "  TR_AXISDIR
#define TR_SETMIDPOINT                 TR_BW_COL(TR_SFC_AIR("Nastaviť stred potenciometra", TR("Nastaviť stred osi", "Stred osi/Ťahového ovládača")), "Vycentrovať osi/ťahové ovládače")
#define TR_MOVESTICKSPOTS              TR_BW_COL(TR_SFC_AIR("Hýbať točiť/Plyn/Potenciometre/Osi", "Hýbať osi/Potenciometre"), "Hýbať osi/Potenciometre")
#define TR_NODATA                      "Žiadne dáta"
#define TR_US                          "us"
#define TR_HZ                          "Hz"

#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "Vnútorné GPS"
#define TR_HEARTBEAT_LABEL             "Náčúvanie"
#define TR_LUA_SCRIPTS_LABEL           "Lua skripty"
#define TR_FREE_MEM_LABEL              "Voľná pamäť"
#define TR_DURATION_MS             TR("[D]","Zostatok(ms): ")
#define TR_INTERVAL_MS             TR("[I]","Interval(ms): ")
#define TR_MEM_USED_SCRIPT         "Skript(B): "
#define TR_MEM_USED_WIDGET         "Widget(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Audio: "
#define TR_GPS_FIX_YES                 "Fix: Áno"
#define TR_GPS_FIX_NO                  "Fix: Nie"
#define TR_GPS_SATS                    "Sats: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Menu: "
#define TR_TIMER_LABEL                 "Časovač"
#define TR_THROTTLE_PERCENT_LABEL      "Plyn %"
#define TR_BATT_LABEL                  "Batéria"
#define TR_SESSION                     "Relácia"

#define TR_MENUTORESET                 TR_ENTER" >> Reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODEL"
#define TR_FM                          TR_SFC_AIR("DM", "LR")
#define TR_EEPROMLOWMEM                "Kapacita EEPROM je nízka"
#define TR_PRESS_ANY_KEY_TO_SKIP       TR("\003Klávesa >>> preskočiť", "Klávesa >>> preskočiť")
#define TR_THROTTLE_NOT_IDLE           TR("\003Páka plynu je pohnutá", "Páka plynu nie je na nule")
#define TR_ALARMSDISABLED              "Alarmy sú zakázané"
#define TR_PRESSANYKEY                 TR("\006Stlač klávesu", "Stač klávesu")
#define TR_BADEEPROMDATA               TR("\006Chyba dát EEPROM", "Chyba dát EEPROM")
#define TR_BAD_RADIO_DATA              "Chybajúce alebo poškodené dáta vysielača"
#define TR_RADIO_DATA_RECOVERED        TR3(" Použitie zálohy dát TX","Použitie zálohy dát vysielača","Nastavenie vysielača bolo obnovené zo zálohy")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Neplatné nastavenie TX","Neplatné nastavenie vysielača", "Nie je možné načítať platné nastavenie vysielača")
#define TR_EEPROMFORMATTING            TR("\004Formátovanie EEPROM", "Formátovanie EEPROM")
#define TR_STORAGE_FORMAT              "Príprava úložiska"
#define TR_EEPROMOVERFLOW              "Pretiekla EEPROM"
#define TR_RADIO_SETUP                 "NASTAVENIE RÁDIA"
#define TR_MENUVERSION                 "Verzie"
#define TR_MENU_RADIO_ANALOGS          "ANALÓGY"
#define TR_MENU_RADIO_ANALOGS_CALIB    "KALIBROVAŤ ANALÓGY"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALÓGY (5 Hz)"
#define TR_MENU_FSWITCH                 "NASTAVITEĽNÉ PŘEPÍNAČE"
#define   TR_TRIMS2OFFSETS              TR_BW_COL("\006Trimy => Subtrimy", "Trimy => Subtrimy")
#define TR_CHANNELS2FAILSAFE           "Kanály=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Kanál=>Failsafe"
#define TR_MENUMODELSEL                "Model"
#define TR_MENU_MODEL_SETUP            "Nastavenie"

#define TR_MENUCURVE                   "\002K"
#define TR_MENULOGICALSWITCH           "Log.Spínač"
#define TR_MENUSTAT                    "Štatistika"
#define TR_MENUDEBUG                   "Diag"
#define TR_MONITOR_CHANNELS            "Monitor Kanálov %d/%d"
#define TR_MONITOR_OUTPUT_DESC         "Výstupy"
#define TR_MONITOR_MIXER_DESC          "Mixy"
#define TR_RECEIVER_NUM                TR("RX číslo", "Číslo prijímača")
#define TR_RECEIVER                    "Prijímač"
#define TR_MULTI_RFTUNE                TR("Ladenie frek", "Jemné ladenie frek. RF")
#define TR_MULTI_RFPOWER               "RF výkon"
#define TR_MULTI_WBUS                  "Výstup"
#define TR_MULTI_TELEMETRY             "Telemetria"
#define TR_MULTI_VIDFREQ               TR("Freq. videa", "Frekvencie videa")
#define TR_RF_POWER                    "Výkon RF"
#define TR_MULTI_FIXEDID               TR("PevnéID", "Pevné ID")
#define TR_MULTI_OPTION                TR("Možnosti", "Hodnota")
#define TR_MULTI_AUTOBIND              TR("Párovať Ch.","Párovať pri zapnutí")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Vypnúť mapovanie kanálov")
#define TR_DISABLE_TELEM               TR("No Telem", "Vypnúť telemetriu")
#define TR_MULTI_LOWPOWER              TR("Nízky výkon", "Režim nízkeho výkonu")
#define TR_MULTI_LNA_DISABLE           "LNA vypnuté"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("Zap", "Zapnuté")
#define TR_DISABLE_INTERNAL            TR("Vypnúť int.", "Vypnúť interný RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("Nesériový", "Nie je v sériovom režime")
#define TR_MODULE_NO_INPUT             TR("Žiadny vstup", "Žiadny sériový vstup")
#define TR_MODULE_NO_TELEMETRY         TR3("Bez telemetrie", "Bez Multi_Telemetria", "Nedetekovaná Multi_Telemetria")
#define TR_MODULE_WAITFORBIND          "Párovať pri zavedení protokolu"
#define TR_MODULE_BINDING              TR("Párovať...","Párovanie")
#define TR_MODULE_UPGRADE_ALERT        TR3("Nut.upg.", "Nutná aktualizácia modulu", "Modul\nnutná aktualizácia")
#define TR_MODULE_UPGRADE              TR("Upg.dop.", "Doporučuje sa aktualizácia modulu")
#define TR_REBIND                      "Je nutné prepárovať"
#define TR_REG_OK                      "Registrácia OK"
#define TR_BIND_OK                     "Úspešné párovanie"
#define TR_BINDING_CH1_8_TELEM_ON      "k1-8 telem ZAP"
#define TR_BINDING_CH1_8_TELEM_OFF     "k1-8 telem VYP"
#define TR_BINDING_CH9_16_TELEM_ON     "k9-16 telem ZAP"
#define TR_BINDING_CH9_16_TELEM_OFF    "k9-16 telem VYP"
#define TR_PROTOCOL_INVALID            TR("Nespr. protokol", "Nesprávny protokol")
#define TR_MODULE_STATUS               TR("Stav", "Stav modulu")
#define TR_MODULE_SYNC                 TR("Sync", "Proto Sync stav")
#define TR_MULTI_SERVOFREQ             TR("Frekv. serva", "Obnovovacia frekvencia serva")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Povoliť max. throw")
#define TR_MULTI_RFCHAN                TR("RF Kanál", "Vybrať RF kanál")
#define TR_GPS_COORDS_FORMAT           "GPS súradnice"
#define TR_VARIO                       "Vario"
#define TR_PITCH_AT_ZERO               "Tón na nule"
#define TR_PITCH_AT_MAX                "Tón na maxime"
#define TR_REPEAT_AT_ZERO              TR("Opak. na nule", "Opakovanie na nule")
#define TR_BATT_CALIB                  "Kalib. bat."
#define TR_CURRENT_CALIB               "+=\006Prúd"
#define TR_VOLTAGE                     "Napätie"
#define TR_SELECT_MODEL                "Vyber model"
#define TR_MANAGE_MODELS               "Nastavenie Modelu"
#define TR_MODELS                      "Modely"
#define TR_SELECT_MODE                 "Vybrať mód"
#define TR_CREATE_MODEL                "Nový model"
#define TR_FAVORITE_LABEL              "Obľúbené"
#define TR_MODELS_MOVED                "Nepoužívané modely presunuté do"
#define TR_NEW_MODEL                   "Nový model"
#define TR_INVALID_MODEL               "Neplatný model"
#define TR_EDIT_LABELS                 "Upraviť štítok"
#define TR_LABEL_MODEL                 "Štítok modelu"
#define TR_MOVE_UP                     "Posunúť hore"
#define TR_MOVE_DOWN                   "Posunúť dole"
#define TR_ENTER_LABEL                 "Vložiť štítok"
#define TR_LABEL                       "Štítok"
#define TR_LABELS                      "Štítky"
#define TR_CURRENT_MODEL               "Aktuálny"
#define TR_ACTIVE                      "Aktívny"
#define TR_NEW                         "Nový"
#define TR_NEW_LABEL                   "Nový štítok"
#define TR_RENAME_LABEL                "Premenovať štítok"
#define TR_DELETE_LABEL                "Odstrániť štítok"
#define TR_DUPLICATE_MODEL             "Duplikovať model"
#define TR_COPY_MODEL                  "Kopírovať"
#define TR_MOVE_MODEL                  "Presunúť model"
#define TR_BACKUP_MODEL                "Zálohovať na SD kartu"
#define TR_DELETE_MODEL                "Zmazať model"
#define TR_RESTORE_MODEL               "Obnov model z SD karty"
#define TR_DELETE_ERROR                "Nie je možné odstrániť"
#define TR_SDCARD_ERROR                "Chyba SD karty"
#define TR_SDCARD                      "SD karta"
#define TR_NO_FILES_ON_SD              "Žiadne súbory na SD karte!"
#define TR_NO_SDCARD                   "Chýba SD karta"
#define TR_WAITING_FOR_RX              "Čakám na RX..."
#define TR_WAITING_FOR_TX              "Čakám na TX..."
#define TR_WAITING_FOR_MODULE          TR("Čakanie modul", "Čakanie na modul...")
#define TR_NO_TOOLS                    TR("Nie sú k dispozícii", "Žiadne nástroje k dispozícii")
#define TR_NORMAL                      "Normálne"
#define TR_NOT_INVERTED                "Neinvertované"
#define TR_NOT_CONNECTED               "Nepripojený"
#define TR_CONNECTED                   "Pripojený"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16k bez telem.", "16k bez telemetrie")
#define TR_16CH_WITH_TELEMETRY         TR("16k s telem.", "16k s telemetriou")
#define TR_8CH_WITH_TELEMETRY          TR("8k s telem.", "8k s telemetriou")
#define TR_EXT_ANTENNA                 "Ext. anténa"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Uložiť nastavenie?"
#define TR_UPDATE_TX_OPTIONS           "Uložiť nastavenie?"
#define TR_MODULES_RX_VERSION          BUTTON("Verzia modulu a RX")
#define TR_SHOW_MIXER_MONITORS         "Zobraziť monitor mixov"
#define TR_MENU_MODULES_RX_VERSION     "Verzia modulu a RX"
#define TR_MENU_FIRM_OPTIONS           "Možnosti firmwéru"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Os/Pot/Ťahový"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Os/Pot/Ťahový"
#define TR_RF_PROTOCOL                 "RF Protokol"
#define TR_MODULE_OPTIONS              "Možnosti modulu"
#define TR_POWER                       "Výkon"
#define TR_NO_TX_OPTIONS               "Žiadne možnosti"
#define TR_RTC_BATT                    "RTC Bat"
#define TR_POWER_METER_EXT             "Merač výkonu (EXT)"
#define TR_POWER_METER_INT             "Merač výkonu (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spektrálny an. (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Spektrálny an. (INT)"
#define TR_GHOST_MODULE_CONFIG         "Ghost module config"
#define TR_GPS_MODEL_LOCATOR           "GPS model lokátor"
#define TR_REFRESH                     "Obnovenie"
#define TR_SDCARD_FULL                 "Plná karta SD"
#define TR_SDCARD_FULL_EXT  TR_BW_COL(TR_SDCARD_FULL "\036Logy a " LCDW_128_LINEBREAK " Snímky obrazovky vypnuté", TR_SDCARD_FULL "\nLogovanie dát a snímky obrazovky vypnuté")
#define TR_NEEDS_FILE                  "Požadovaný súbor"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Nekompatibilný"
#define TR_WARNING                     "Kontrola"
#define TR_STORAGE_WARNING             "Úložisko"
#define TR_THROTTLE_UPPERCASE          "Plynu"
#define TR_ALARMSWARN                  "Alarmu"
#define TR_SWITCHWARN                  "Pozícia"
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("Testovacia", "Testovacia verzia")
#define TR_TEST_NOTSAFE                "Len pre test"
#define TR_WRONG_SDCARDVERSION         "Očakávaná ver.: "
#define TR_WARN_RTC_BATTERY_LOW        "Slabá RTC batéria"
#define TR_WARN_MULTI_LOWPOWER         "Režim nízkeho výkonu"
#define TR_BATTERY                     "Batéria"
#define TR_WRONG_PCBREV                "Iná verzia PCB/firmwéru"
#define TR_EMERGENCY_MODE              "Núdzový režimn"
#define TR_NO_FAILSAFE                 "Failsafe nie je nastavené"
#define TR_KEYSTUCK                    "Zaseknutá klávesnica"
#define TR_VOLUME                      "Hlasitosť"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Jas"
#define TR_CPU_TEMP                    "Tepl. CPU\016>"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "Tepl. MB \016>"
#define TR_TTL_WARNING                 "Varovanie: Neprekračovať napätie 3.3V pri pinoch pre TX/RX!"
#define TR_FUNC                        "Funkcie"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Trvanie"
#define TR_DELAY                       "Zdržať"
#define TR_NO_SOUNDS_ON_SD             "Žiadny zvuk na SD"
#define TR_NO_MODELS_ON_SD             "Žiadny model na SD"
#define TR_NO_BITMAPS_ON_SD            "Žiadne obrázky na SD"
#define TR_NO_SCRIPTS_ON_SD            "Žiadny skript na SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntaktická chyba", "Syntaktická chyba skriptu")
#define TR_SCRIPT_PANIC                "Skript panika"
#define TR_SCRIPT_KILLED               "Skript ukončený"
#define TR_SCRIPT_ERROR                "Neznáma chyba"
#define TR_PLAY_FILE                   "Prehrať"
#define TR_DELETE_FILE                 "Odstrániť"
#define TR_COPY_FILE                   "Kopírovať"
#define TR_RENAME_FILE                 "Premenovať"
#define TR_ASSIGN_BITMAP               "Vybrať obrázok"
#define TR_ASSIGN_SPLASH               "Úvodná obrazovka"
#define TR_EXECUTE_FILE                "Spustiť"
#define TR_REMOVED                     "Odstránený"
#define TR_SD_INFO                     "Informácie"
#define TR_NA                          "[X]"
#define TR_FORMATTING                  "Formátovanie..."
#define TR_TEMP_CALIB                  "+=\006Teplota"
#define TR_TIME                        "Čas"
#define TR_MAXBAUDRATE                 "Max baudov"
#define TR_BAUDRATE                    "Baudrate"
#define TR_CRSF_ARMING_MODE            "Arm mód"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 "Vzorkovací režim"
#define TR_SAMPLE_MODES_1              "Normálny"
#define TR_SAMPLE_MODES_2              "OneBit"
#define TR_LOADING                     "Načítanie.."
#define TR_DELETE_THEME                "Zmazať motív?"
#define TR_SAVE_THEME                  "Uložiť motív?"
#define TR_EDIT_COLOR                  "Upraviť farbu"
#define TR_NO_THEME_IMAGE              "Náhľad motívu nedostupný"
#define TR_BACKLIGHT_TIMER             "Čas zhasnutia displeja"

#define TR_MODEL_QUICK_SELECT          "Rýchly výber modelu"
#define TR_LABELS_SELECT               "Výber štítku"
#define TR_LABELS_MATCH                "Zhoda štítkov"
#define TR_FAV_MATCH                   "Zhoda obľúbených"
#define TR_LABELS_SELECT_MODE_1        "Viacnásobný výber"
#define TR_LABELS_SELECT_MODE_2        "Jeden výber"
#define TR_LABELS_MATCH_MODE_1         "Zhoda so všetkými"
#define TR_LABELS_MATCH_MODE_2         "Zhoda s ľubovoľným"
#define TR_FAV_MATCH_MODE_1            "Musí sa zhodovať"
#define TR_FAV_MATCH_MODE_2            "Voliteľná zhoda"

#define TR_SELECT_TEMPLATE_FOLDER      "Vybrať zložku so šablónou:"
#define TR_SELECT_TEMPLATE             "Vybrať šablónu modelu:"
#define TR_NO_TEMPLATES                "Žiadna šablóna modelu v tejto zložke nebola nájdená"
#define TR_SAVE_TEMPLATE               "Uložiť ako šablónu"
#define TR_BLANK_MODEL                 "Prázdny model"
#define TR_BLANK_MODEL_INFO            "Vytvoriť prázdny model"
#define TR_FILE_EXISTS                 "Súbor"
#define TR_ASK_OVERWRITE               "Chcete prepísať?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Hľadať"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Vzdial. adr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Lokál. adr"
#define TR_BLUETOOTH_PIN_CODE          TR("PIN kód", "PIN kód")
#define TR_BLUETOOTH_NODEVICES         "Nebolo nájdené žiadne zariadenie"
#define TR_BLUETOOTH_SCANNING          "Skenovanie..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#define TR_BLUETOOTH_MODES_1           "---"
#define TR_BLUETOOTH_MODES_2           "Telemetria"
#define TR_BLUETOOTH_MODES_3           "Trenér  "
#define TR_BLUETOOTH_MODES_4           "Povolené"
#define TR_SD_INFO_TITLE               "SD INFO"
#define TR_SD_SPEED                    "Rýchlosť:"
#define TR_SD_SECTORS                  "Sektorov:"
#define TR_SD_SIZE                     "Veľkosť:"
#define TR_TYPE                        "Typ"
#define TR_GVARS                       "Glob.Prem."
#define TR_GLOBAL_VAR                  "Globálna premenná"
#define TR_OWN                         "\043"
#define TR_DATE                        "Dátum"
#define TR_MONTHS_1                    "Jan"
#define TR_MONTHS_2                    "Feb"
#define TR_MONTHS_3                    "Mar"
#define TR_MONTHS_4                    "Apr"
#define TR_MONTHS_5                    "Máj"
#define TR_MONTHS_6                    "Jún"
#define TR_MONTHS_7                    "Júl"
#define TR_MONTHS_8                    "Aug"
#define TR_MONTHS_9                    "Sep"
#define TR_MONTHS_10                   "Okt"
#define TR_MONTHS_11                   "Nov"
#define TR_MONTHS_12                   "Dec"
#define TR_ROTARY_ENCODER              "R.Enko"
#define TR_ROTARY_ENC_MODE             TR("R.Enko mód","Režim otočného tlačítka")
#define TR_CHANNELS_MONITOR            "Monitor Kanálu"
#define TR_MIXERS_MONITOR              "Monitor Mixu"
#define TR_PATH_TOO_LONG               "Cesta je veľmi dlhá"
#define TR_VIEW_TEXT                   "Zobraziť text"
#define TR_FLASH_BOOTLOADER            "Zápis bootloadera"
#define TR_FLASH_DEVICE                TR("Zápis zar.","Zápis zariadenia")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Zápis S.Portom", "Zápis cez S.Port")
#define TR_FLASH_RECEIVER_OTA          "Zápis príjímača cez OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Zápis RX cez ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Zápis RX cez int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Zápis FC cez ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Zápis FC cez int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Zápis modulu BT", "Zápis modulu Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT TR("Zápis riadenia spotr.", "Flash jednotky riadenia spotreby")
#define TR_DEVICE_NO_RESPONSE          TR("Zariadenie neodpovedá", "Zariadenie neodpovedá")
#define TR_DEVICE_FILE_ERROR           TR("Zar. súbor problém", "Zariadenie-problém sa súborom")
#define TR_DEVICE_DATA_REFUSED         TR("Zar. dáta odmietnuté", "Zariadenie-dáta odmietnuté")
#define TR_DEVICE_WRONG_REQUEST        TR("Zar. prístup problém", "Zariadenie-problém s prístupom")
#define TR_DEVICE_FILE_REJECTED        TR("Zar. súbor odmietnutý", "Zariadenie-súbor odmietnutý")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Zar. podpis súboru", "Zariadenie-neodpovedá podpis")
#define TR_CURRENT_VERSION             TR("Súčasná ver. ", "Súčasná verzia: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Flash vnitř. modulu", "Flash interného modulu")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash int. Multi", "Flash interného modulu Multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash ext. modulu", "Flash externého modulu")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash ext. Multi", "Flash externého modulu Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash ext. ELRS", "Flash externého modulu ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Chyba zápisu FW", "Chyba zápisu firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Úspešný zápis FW"
#define TR_WRITING                     "Zapisujem..."
#define TR_CONFIRM_FORMAT              "Formátovat?"
#define TR_INTERNALRF                  "Interný RF modul"
#define TR_INTERNAL_MODULE             "Interný modul"
#define TR_EXTERNAL_MODULE             "Externý modul"
#define TR_EDGETX_UPGRADE_REQUIRED     TR("Aktualizujte EdgeTX", "Vyžadovaná aktualizácia EdgeTX")
#define TR_TELEMETRY_DISABLED          "Telem. zakázaná"
#define TR_MORE_OPTIONS_AVAILABLE      TR("Viac možností", "Viac dostupných možností")
#define TR_NO_MODULE_INFORMATION       "Žiadne info o module"
#define TR_EXTERNALRF                  "Externý RF modul"
#define TR_FAILSAFE                    TR("Failsafe", "Mód Failsafe")
#define TR_FAILSAFESET                 "NASTAVENIE FAILSAFE"
#define TR_REG_ID                      TR("Reg. ID", "Registračné ID")
#define TR_OWNER_ID                    "ID vlastníka"
#define TR_HOLD                        "Držať"
#define TR_HOLD_UPPERCASE              "DRŽAŤ"
#define TR_NONE                        "Nič"
#define TR_NONE_UPPERCASE              "NIČ"
#define TR_MENUSENSOR                  "SENZOR"
#define TR_POWERMETER_PEAK             "Špička"
#define TR_POWERMETER_POWER            "Výkon"
#define TR_POWERMETER_ATTN             "Útlm"
#define TR_POWERMETER_FREQ             "Frek."
#define TR_MENUTOOLS                   "NÁSTROJE"
#define TR_TURN_OFF_RECEIVER           "Vypnite prijímač"
#define TR_STOPPING                    "Zastavujem..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPEKTRÁLNY ANALYZÁTOR"
#define TR_MENU_POWER_METER            "MERAČ VÝKONU"
#define TR_SENSOR                      "SENZOR"
#define TR_COUNTRY_CODE                "Kód regiónu"
#define TR_USBMODE                     "Režim USB"
#define TR_JACK_MODE                   "Režim Jack"
#define TR_VOICE_LANGUAGE              "Jazyk hlasu"
#define TR_UNITS_SYSTEM                "Jednotky"
#define TR_UNITS_PPM                   "PPM jednotky"
#define TR_EDIT                        "Upraviť"
#define TR_INSERT_BEFORE               "Vložiť pred"
#define TR_INSERT_AFTER                "Vložiť za"
#define TR_COPY                        "Kopírovať"
#define TR_MOVE                        "Presunúť"
#define TR_PASTE                       "Vložiť"
#define TR_PASTE_AFTER                 "Vložiť za"
#define TR_PASTE_BEFORE                "Vložiť pred"
#define TR_DELETE                      "Odstrániť"
#define TR_INSERT                      "Pridať"
#define TR_RESET_FLIGHT                "Reset relácie"
#define TR_RESET_TIMER1                "Čas1"
#define TR_RESET_TIMER2                "Čas2"
#define TR_RESET_TIMER3                "Čas3"
#define TR_RESET_TELEMETRY             "Telemetria"
#define TR_STATISTICS                  "Štatistika"
#define TR_ABOUT_US                    "O nás"
#define TR_USB_JOYSTICK                "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB Disk (SD)"
#define TR_USB_SERIAL                  "USB Serial (VCP)"
#define TR_SETUP_SCREENS               "Obrazovky nastavenie"
#define TR_MONITOR_SCREENS             "Monitory"
#define TR_AND_SWITCH                  "AND Spínač"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Kalibrovať analógy"
#define TR_ANADIAGS_FILTRAWDEV         "Filtrované surové hodnoty analógov s odchýlkou"
#define TR_ANADIAGS_UNFILTRAW          "Nefiltrované surové hodnoty analógov"
#define TR_ANADIAGS_MINMAX             "Min., max. a rozsah"
#define TR_ANADIAGS_MOVE               "Posuňte analógy do ich maximálnych polôh!"
#define TR_BYTES                       "[B]"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Párovať"))
#define TR_MODULE_UNBIND               BUTTON("Zrušiť párovanie")
#define TR_POWERMETER_ATTN_NEEDED     "Útlmový člen nutný!"
#define TR_PXX2_SELECT_RX              "Vyber RX..."
#define TR_PXX2_DEFAULT                "<východzí>"
#define TR_BT_SELECT_DEVICE            "Vyberte zariadenie"
#define TR_DISCOVER                    BUTTON("Hľadať")
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Čakám..."
#define TR_RECEIVER_DELETE             "Zmazať prijímač?"
#define TR_RECEIVER_RESET              "Resetovať prijímač?"
#define TR_SHARE                       "Zdieľať"
#define TR_BIND                        "Párovať"
#define TR_REGISTER                    BUTTON(TR("Reg", "Registrovať"))
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "Dosah"))
#define TR_RANGE_TEST                  "Test dosahu"
#define TR_RECEIVER_OPTIONS            TR("MOŽNOSTI RX", "MOŽNOSTI PRIJÍMAČE")
#define TR_RESET_BTN                   BUTTON("Reset")
#define TR_KEYS_BTN                    BUTTON(TR("SW","Prepínače"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analógy"))
#define TR_FS_BTN                      BUTTON(TR("Vlastný Sw", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Dotyková obrazovka nenájdená"
#define TR_TOUCH_EXIT                  "Dotknite sa obrazovky pre ukončenie"
#define TR_SET                         BUTTON("Nast")
#define TR_TRAINER                     "Trenér"
#define TR_CHANS                       "Kanály"
#define TR_ANTENNAPROBLEM              "Problém s TX anténou!"
#define TR_MODELIDUSED                 TR("ID už použité","ID modelu je už použité")
#define TR_MODELIDUNIQUE               "ID je unikátne"
#define TR_MODULE                      "Modul"
#define TR_RX_NAME                     "Meno RX"
#define TR_TELEMETRY_TYPE              TR("Typ", "Typ telemetrie")
#define TR_TELEMETRY_SENSORS           "Senzory"
#define TR_VALUE                       "Hodnota"
#define TR_PERIOD                      "Perióda"
#define TR_INTERVAL                    "Interval"
#define TR_REPEAT                      "Opakovať"
#define TR_ENABLE                      "Povolené"
#define TR_DISABLE                     "Zakázané"
#define TR_TOPLCDTIMER                 "Stopky horného LCD"
#define TR_UNIT                        "Jednotky"
#define TR_TELEMETRY_NEWSENSOR         "Pridať senzor ručne"
#define TR_CHANNELRANGE                TR("Kanály", "Rozsah kanálov")
#define TR_AFHDS3_RX_FREQ              TR("RX frek.", "RX frekvencia")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetria")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Aktuálny výkon")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", TR("Power src.", "Power source"))
#define TR_ANTENNACONFIRM1             "Skutočne prepnúť?"
#define TR_ANTENNA_MODES_1           "Interný"
#define TR_ANTENNA_MODES_2           "Spýtať sa"
#define TR_ANTENNA_MODES_3           "Podľa modelu"
#define TR_ANTENNA_MODES_4           "Interný + Externý"
#define TR_ANTENNA_MODES_5           "Externý"
#define TR_USE_INTERNAL_ANTENNA        TR("Použiť int. ant.", "Použiť internú anténu")
#define TR_USE_EXTERNAL_ANTENNA        TR("Použiť ext. ant.", "Použiť externú anténu")
#define TR_ANTENNACONFIRM2             TR("Zkont. anténu", "Uistite sa, že je anténa pripojená!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Vyžaduje FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Vyžaduje FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Vyžaduje EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "firmware."
#define TR_LOWALARM                    "Nízky Alarm"
#define TR_CRITICALALARM               "Kritický Alarm"
#define TR_DISABLE_ALARM               TR("Vypnúť alarmy", "Vypnúť alarmy telemetrie")
#define TR_POPUP                       "Vyskočiť"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Šablona"
#define TR_PRESET                      "Šablona"
#define TR_MIRROR                      "Zrkadliť"
#define TR_CLEAR                       "Zmazať"
#define TR_CLEAR_BTN                   BUTTON("Zmazať")
#define TR_RESET                       "Reset"
#define TR_RESET_SUBMENU               "Inicializovať ..."
#define TR_COUNT                       "Veľkosť"
#define TR_PT                          "č."
#define TR_PTS                         "b."
#define TR_SMOOTH                      "Hladká"
#define TR_COPY_STICKS_TO_OFS          TR("Kop. osi->subtrim", "Kopírovať osi do subtrimu")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Kópia min/max do všetkých",  "Kopírovať min/max/střed do všetkých výstupů")
#define TR_COPY_TRIMS_TO_OFS           TR("Trimy do subtrimu", "Kopírovať trimy do subtrimů")
#define TR_INCDEC                      "Zvätšiť/Zmenšiť"
#define TR_GLOBALVAR                   "Glob. premenná"
#define TR_MIXSOURCE                   "Zdroj (%)"
#define TR_MIXSOURCERAW                "Zdroj (hodnota)"
#define TR_CONSTANT                    "Konštanta"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1 "Vyp"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2 "Zap"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3 "Auto"
#define TR_PREFLIGHT                   "Predletová kontrola"
#define TR_CHECKLIST                   TR("Poznámky", "Zobrazit poznámky")
#define TR_CHECKLIST_INTERACTIVE       TR3("S-interaktívny", "Interaktívny zoznam", "Interaktívny zoznam")
#define TR_AUX_SERIAL_MODE             "Seriový port"
#define TR_AUX2_SERIAL_MODE            "Seriový port 2"
#define TR_AUX_SERIAL_PORT_POWER       "Výkon portu"
#define TR_SCRIPT                      "Skript"
#define TR_INPUTS                      "Vstupy"
#define TR_OUTPUTS                     "Výstupy"
#define TR_CONFIRMRESET                TR("Zmazať všetko?", "Zmazať modely a nastavenie?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Príliš mnoho skriptov!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP výkon"
#define TR_SPORT_UPDATE_POWER_MODES_1  "AUTO"
#define TR_SPORT_UPDATE_POWER_MODES_2  "ON"
#define TR_NO_TELEMETRY_SCREENS        "Žiadne obrazovky telemetrie"
#define TR_TOUCH_PANEL                 "Dotyková obrazovka:"
#define TR_FILE_SIZE                   "Veľkosť súboru"
#define TR_FILE_OPEN                   "Otvoriť?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Názov"
#define TR_PHASES_HEADERS_SW           "Spínač"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trim smerovky"
#define TR_PHASES_HEADERS_ELE_TRIM     "Trim výškovky"
#define TR_PHASES_HEADERS_THT_TRIM     "Trim plynu"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trim krídelok"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Prechod nábehu"
#define TR_PHASES_HEADERS_FAD_OUT      "Prechod konca"

#define TR_LIMITS_HEADERS_NAME         "Názov"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Invertovať"
#define TR_LIMITS_HEADERS_CURVE        "Krivka"
#define TR_LIMITS_HEADERS_PPMCENTER    "Stred PPM"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Symetrické"
#define TR_INVERTED                    "Invertované"

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "Prvý kanál"
#define TR_FILL_BACKGROUND           "Vyplniť pozadie?"
#define TR_BG_COLOR                  "Farba pozadie"
#define TR_SLIDERS_TRIMS             "Ťah. ovl.+Trimy"
#define TR_SLIDERS                   "Ťah. ovl."
#define TR_FLIGHT_MODE               "Letový režim"
#define TR_INVALID_FILE              "Neplatný súbor"
#define TR_TIMER_SOURCE              "Časovač zdroj"
#define TR_SIZE                      "Veľkosť"
#define TR_SHADOW                    "Tiene"
#define TR_ALIGNMENT                 "Zarovnanie"
#define TR_ALIGN_LABEL               "Zarovnať názov"
#define TR_ALIGN_VALUE               "Zarovnať hodnotu"
#define TR_ALIGN_OPTS_1              "Vľavo"
#define TR_ALIGN_OPTS_2              "Uprostred"
#define TR_ALIGN_OPTS_3              "Vpravo"
#define TR_TEXT                      "Text"
#define TR_COLOR                     "Farba"
#define TR_PANEL1_BACKGROUND         "Panel1 pozadie"
#define TR_PANEL2_BACKGROUND         "Panel2 pozadie"
#define TR_PANEL_BACKGROUND          "Pozadie"
#define TR_PANEL_COLOR               "Farba"
#define TR_WIDGET_GAUGE              "Meradlo"
#define TR_WIDGET_MODELBMP           "Model info"
#define TR_WIDGET_OUTPUTS            "Výstupy"
#define TR_WIDGET_TEXT               "Text"
#define TR_WIDGET_TIMER              "Časovač"
#define TR_WIDGET_VALUE              "Hodnota"

// About screen
#define TR_ABOUTUS                     "O nás"

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'              // Values between A-I will work

#define TR_BEEP_VOLUME                 "Upozornění"
#define TR_WAV_VOLUME                  "Zvuky WAV"
#define TR_BG_VOLUME                   "WAV na pozadie"

#define TR_TOP_BAR                     "Horní lišta"
#define TR_FLASH_ERASE                 "Mazanie flash..."
#define TR_FLASH_WRITE                 "Zápis flash..."
#define TR_OTA_UPDATE                  "Aktualizácia OTA..."
#define TR_MODULE_RESET                "Reset modulu..."
#define TR_UNKNOWN_RX                  "Neznámy RX"
#define TR_UNSUPPORTED_RX              "Nepodporovaný RX"
#define TR_OTA_UPDATE_ERROR            "Chyba aktualizácie"
#define TR_DEVICE_RESET                "Reset zariadenia..."
#define TR_ALTITUDE                    "Výška"
#define TR_SCALE                       "Mierka"
#define TR_VIEW_CHANNELS               "Zobraziť kanály"
#define TR_VIEW_NOTES                  "Zobraziť poznámky"
#define TR_MODEL_SELECT                "Zvoliť model"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Presnosť"
#define TR_RATIO                       "Koeficient"
#define TR_FORMULA                     "Operácia"
#define TR_CELLINDEX                   "Článek"
#define TR_LOGS                        "Logovať"
#define TR_OPTIONS                     "Možnosti"
#define TR_FIRMWARE_OPTIONS            BUTTON("Možnosti firmwaru")

#define TR_ALTSENSOR                   "Senzor výšky"
#define TR_CELLSENSOR                  "Senzor článkov"
#define TR_GPSSENSOR                   "GPS senzor"
#define TR_CURRENTSENSOR               "Senzor"
#define TR_AUTOOFFSET                  "Auto offset"
#define TR_ONLYPOSITIVE                "Jen kladné"
#define TR_FILTER                      "Filtr"
#define TR_TELEMETRYFULL               "Všetky sloty sú plné!"
#define TR_IGNORE_INSTANCE             TR("Chybné ID", "Ignoruj chyby ID")
#define TR_SHOW_INSTANCE_ID            "Zobraziť ID inštancie"
#define TR_DISCOVER_SENSORS            "Detekovat nové senzory"
#define TR_STOP_DISCOVER_SENSORS       "Zastaviť autodetekciu"
#define TR_DELETE_ALL_SENSORS          "Odobrať všetky senzory"
#define TR_CONFIRMDELETE               "Skutočne " LCDW_128_LINEBREAK "odstrániť všetko?"
#define TR_SELECT_WIDGET               "Zvoliť widget"
#define TR_WIDGET_FULLSCREEN           "Celá obrazovka"
#define TR_REMOVE_WIDGET               "Odstrániť widget"
#define TR_WIDGET_SETTINGS             "Nastavenie widgetu"
#define TR_REMOVE_SCREEN               "Odstrániť panel"
#define TR_SETUP_WIDGETS               "Nastaviť widgety"
#define TR_THEME                       "Motív"
#define TR_SETUP                       "Nastavenie"
#define TR_LAYOUT                      "Rozloženie"
#define TR_TEXT_COLOR                  "Farba textu"

#define TR_MENU_INPUTS                 CHAR_INPUT "Vstupy"
#define TR_MENU_LUA                    CHAR_LUA "Lua skripty"
#define TR_MENU_STICKS                 CHAR_STICK "Os"
#define TR_MENU_POTS                   CHAR_POT "Potenciometre"
#define TR_MENU_MIN                    CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   CHAR_CYC "Cyklika"
#define TR_MENU_TRIMS                  CHAR_TRIM "Trimy"
#define TR_MENU_SWITCHES               CHAR_SWITCH "Spínače"
#define TR_MENU_LOGICAL_SWITCHES       CHAR_SWITCH "Logické spínače"
#define TR_MENU_TRAINER                CHAR_TRAINER "Tréner"
#define TR_MENU_CHANNELS               CHAR_CHANNEL "Kanály"
#define TR_MENU_GVARS                  CHAR_SLIDER "Glob.premenné"
#define TR_MENU_TELEMETRY              CHAR_TELEMETRY "Telemetria"
#define TR_MENU_DISPLAY                "DISPLAY"
#define TR_MENU_OTHER                  "Ostatné"
#define TR_MENU_INVERT                 "Invertovať"
#define TR_AUDIO_MUTE                  TR("Stlmenie zvuku","Stlmenie, pokiaľ nie je počuť zvuk")
#define TR_JITTER_FILTER               "ADC Filtr"
#define TR_DEAD_ZONE                   "Mŕtva zóna"
#define TR_RTC_CHECK                   TR("Kontr RTC", "Sledovať RTC napätie")
#define TR_AUTH_FAILURE                "Auth-zlyhala"
#define TR_RACING_MODE                 "Pretekársky režim"

#define TR_USE_THEME_COLOR              "Použiť farebný motív"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Trimy do subtrimov"
#define TR_DUPLICATE                    "Duplikovať"
#define TR_ACTIVATE                     "Aktivovať"
#define TR_RED                          "Červená"
#define TR_BLUE                         "Modrá"
#define TR_GREEN                        "Zelená"
#define TR_COLOR_PICKER                 "Výber farby"
#define TR_FIXED                        "Fixed"
#define TR_EDIT_THEME_DETAILS           "Editovať motív"
#define TR_THEME_COLOR_DEFAULT          "PREDNASTAVENÉ"
#define TR_THEME_COLOR_PRIMARY1         "PRIMÁRNE1"
#define TR_THEME_COLOR_PRIMARY2         "PRIMÁRNE2"
#define TR_THEME_COLOR_PRIMARY3         "PRIMÁRNE3"
#define TR_THEME_COLOR_SECONDARY1       "SEKUNDÁRNE1"
#define TR_THEME_COLOR_SECONDARY2       "SEKUNDÁRNE2"
#define TR_THEME_COLOR_SECONDARY3       "SEKUNDÁRNE3"
#define TR_THEME_COLOR_FOCUS            "VYBRANÉ"
#define TR_THEME_COLOR_EDIT             "UPRAVIŤ"
#define TR_THEME_COLOR_ACTIVE           "AKTÍVNE"
#define TR_THEME_COLOR_WARNING          "VAROVANIE"
#define TR_THEME_COLOR_DISABLED         "VYPNUTÉ"
#define TR_THEME_COLOR_CUSTOM           "VLASTNÉ"
#define TR_THEME_CHECKBOX               "Pole výberu"
#define TR_THEME_ACTIVE                 "Aktívne"
#define TR_THEME_REGULAR                "Normálne"
#define TR_THEME_WARNING                "Varovanie"
#define TR_THEME_DISABLED               "Vypnuté"
#define TR_THEME_EDIT                   "Upraviť"
#define TR_THEME_FOCUS                  "Vybrané"
#define TR_AUTHOR                       "Autor"
#define TR_DESCRIPTION                  "Popis"
#define TR_SAVE                         "Uložiť"
#define TR_CANCEL                       "Zrušiť"
#define TR_EDIT_THEME                   "EDITOVAŤ MOTÍV"
#define TR_DETAILS                      "Detaily"

// Voice in native language
#define TR_VOICE_ENGLISH                "Angličtina"
#define TR_VOICE_CHINESE                "Čínština"
#define TR_VOICE_CZECH                  "Čeština"
#define TR_VOICE_DANISH                 "Dánčina"
#define TR_VOICE_DEUTSCH                "Nemčina"
#define TR_VOICE_DUTCH                  "Holandčina"
#define TR_VOICE_ESPANOL                "Španielčina"
#define TR_VOICE_FINNISH                "Fínčina"
#define TR_VOICE_FRANCAIS               "Francúština"
#define TR_VOICE_HUNGARIAN              "Maďarčina"
#define TR_VOICE_ITALIANO               "Taliančina"
#define TR_VOICE_POLISH                 "Poľština"
#define TR_VOICE_PORTUGUES              "Portugačina"
#define TR_VOICE_RUSSIAN                "Ruština"
#define TR_VOICE_SLOVAK                 "Slovenčina"
#define TR_VOICE_SWEDISH                "Švédčina"
#define TR_VOICE_TAIWANESE              "Tchajwančina"
#define TR_VOICE_JAPANESE               "Japončina"
#define TR_VOICE_HEBREW                 "Hebrečina"
#define TR_VOICE_UKRAINIAN              "Ukrainčina"
#define TR_VOICE_KOREAN                 "Kórejčina"

#define TR_USBJOYSTICK_LABEL           "USB Joystick"
#define TR_USBJOYSTICK_EXTMODE         "Mód"
#define TR_VUSBJOYSTICK_EXTMODE_1      "Základný"
#define TR_VUSBJOYSTICK_EXTMODE_2      "Rozšírený"
#define TR_USBJOYSTICK_SETTINGS        BUTTON("Nastavenie kanálov")
#define TR_USBJOYSTICK_IF_MODE         TR("If. mode","Interface mód")
#define TR_VUSBJOYSTICK_IF_MODE_1      "Joystick"
#define TR_VUSBJOYSTICK_IF_MODE_2      "Gamepad"
#define TR_VUSBJOYSTICK_IF_MODE_3      "MultiAxis"
#define TR_USBJOYSTICK_CH_MODE         "Mód"
#define TR_VUSBJOYSTICK_CH_MODE_1      "Žiadny"
#define TR_VUSBJOYSTICK_CH_MODE_2      "Tlač."
#define TR_VUSBJOYSTICK_CH_MODE_3      "Os"
#define TR_VUSBJOYSTICK_CH_MODE_4      "Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S_1    "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2    "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3    "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4    "S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Tlač.mód"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1   "Normál"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2   "Pulz"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3   "SWEmu"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4   "Delta"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5   "Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1 TR("Norm","Normál")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2 TR("Pulz","Pulz")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3 TR("SWEm","SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4 TR("Delt","Delta")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5 TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Pozície"
#define TR_VUSBJOYSTICK_CH_SWPOS_1     "Push"
#define TR_VUSBJOYSTICK_CH_SWPOS_2     "2POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_3     "3POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_4     "4POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_5     "5POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_6     "6POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_7     "7POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_8     "8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Axis"
#define TR_VUSBJOYSTICK_CH_AXIS_1      "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2      "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3      "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4      "rotX"
#define TR_VUSBJOYSTICK_CH_AXIS_5      "rotY"
#define TR_VUSBJOYSTICK_CH_AXIS_6      "rotZ"
#define TR_VUSBJOYSTICK_CH_AXIS_7      "Ťah.ovl."
#define TR_VUSBJOYSTICK_CH_AXIS_8      "Dial"
#define TR_VUSBJOYSTICK_CH_AXIS_9      "Koliesko"
#define TR_USBJOYSTICK_CH_SIM          "Sim osi"
#define TR_VUSBJOYSTICK_CH_SIM_1       "Ail"
#define TR_VUSBJOYSTICK_CH_SIM_2       "Ele"
#define TR_VUSBJOYSTICK_CH_SIM_3       "Rud"
#define TR_VUSBJOYSTICK_CH_SIM_4       "Thr"
#define TR_VUSBJOYSTICK_CH_SIM_5       "Acc"
#define TR_VUSBJOYSTICK_CH_SIM_6       "Brk"
#define TR_VUSBJOYSTICK_CH_SIM_7       "Steer"
#define TR_VUSBJOYSTICK_CH_SIM_8       "Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Inverzia"
#define TR_USBJOYSTICK_CH_BTNNUM       "Tlačidlo no."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Tlačidlo no. kolízia!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Os kolízia!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1 "Žiadny"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2 "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3 "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4 "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   BUTTON("Použiť zmeny")

#define TR_DIGITAL_SERVO               "Servo 333Hz"
#define TR_ANALOG_SERVO                "Servo 50Hz"
#define TR_SIGNAL_OUTPUT               "Signál výstup"
#define TR_SERIAL_BUS                  "Serial BUS"
#define TR_SYNC                        "Synchronizovať"

#define TR_ENABLED_FEATURES            "Aktívne funkcie"
#define TR_RADIO_MENU_TABS             "Rádiové menu záložky"
#define TR_MODEL_MENU_TABS             "Model menu záložky"

#define TR_SELECT_MENU_ALL        "Všetko"
#define TR_SELECT_MENU_CLR        "Vymazať"
#define TR_SELECT_MENU_INV        "Invertovať"

#define TR_SORT_ORDERS_1          "Názov A-Z"
#define TR_SORT_ORDERS_2          "Názov Z-A"
#define TR_SORT_ORDERS_3          "Najmenej používané"
#define TR_SORT_ORDERS_4          "Najviac používané"
#define TR_SORT_MODELS_BY         "Zoradiť modely podľa"
#define TR_CREATE_NEW             "Vytvoriť"

#define TR_MIX_SLOW_PREC          TR("Presnosť spomal", "Presnosť spomalenia")
#define TR_MIX_DELAY_PREC         TR("Oneskorenie prec", "Oneskorenie up/dn prec")

#define TR_THEME_EXISTS           "Adresár s týmto názvom už existuje."

#define TR_DATE_TIME_WIDGET       "Dátum a čas"
#define TR_RADIO_INFO_WIDGET      "Informácie o rádiu"
#define TR_LOW_BATT_COLOR         "Vybitá batéria"
#define TR_MID_BATT_COLOR         "Stredne nabitá batéria"
#define TR_HIGH_BATT_COLOR        "Plne nabitá batéria"

#define TR_WIDGET_SIZE            "Veľkosť widgetu"

#define TR_DEL_DIR_NOT_EMPTY      "Adresár pre zmazaním musí byť prázdny"
