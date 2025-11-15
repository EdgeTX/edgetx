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
#define TR_QM_MODEL_SETTINGS            "Model\nSettings"
#define TR_QM_RADIO_SETTINGS            "Radio\nSettings"
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
#define TR_MAIN_MODEL_SETTINGS          "Model Settings"
#define TR_MAIN_RADIO_SETTINGS          "Radio Settings"
#define TR_MAIN_MENU_MANAGE_MODELS      "Gestione Modelli"
#define TR_MAIN_MENU_MODEL_NOTES        "Note Modello"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Monitor Canali"
#define TR_MONITOR_SWITCHES             "MONITOR INTERRUTTORI LOGICI"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Settaggi Modello"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Settaggi Radio"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "UI Setup"
#define TR_MAIN_MENU_STATISTICS         "Statistiche"
#define TR_MAIN_MENU_ABOUT_EDGETX       "Riguardo EdgeTX"
#define TR_MAIN_VIEW_X                  "Screen "
#define TR_MAIN_MENU_THEMES                 "TEMI"
#define TR_MAIN_MENU_APPS               "Apps"
#define TR_MENUHELISETUP                "CONFIGURA ELI"
#define TR_MENUFLIGHTMODES              TR_SFC_AIR("MODI DI PILOTAGGIO", "FASI DI VOLO")
#define TR_MENUFLIGHTMODE               TR_SFC_AIR("MODO DI PILOTAGGIO", "FASE DI VOLO")
#define TR_MENUINPUTS                   "INGRESSI"
#define TR_MENULIMITS                   "USCITE"
#define TR_MENUCURVES                   "CURVE"
#define TR_MIXES                        "MIXES"
#define TR_MENU_GLOBAL_VARS             "Variabili Globali"
#define TR_MENULOGICALSWITCHES          TR("INTER. LOGICI", "INTERRUTTORI LOGICI")
#define TR_MENUCUSTOMFUNC               TR("FUNZ. SPECIALI", "FUNZIONI SPECIALI")
#define TR_MENUCUSTOMSCRIPTS            "SCRIPTS UTENTE"
#define TR_MENUTELEMETRY                "TELEMETRIA"
#define TR_MENUSPECIALFUNCS             "FUNZIONI GLOBALI"
#define TR_MENUTRAINER                  "MAESTRO/ALLIEVO"
#define TR_HARDWARE                     "HARDWARE"
#define TR_USER_INTERFACE               "Top Bar"
#define TR_SD_CARD                      "SD Card"
#define TR_DEBUG                        "Debug"
#define TR_MENU_RADIO_SWITCHES          "DIAG"
#define TR_MENUCALIBRATION              "CALIBRAZIONE"
#define TR_FUNCTION_SWITCHES            "Interruttori personalizzabili"
// End Main menu

#define TR_MINUTE_SINGULAR     "minuto"
#define TR_MINUTE_PLURAL1      "minuti"
#define TR_MINUTE_PLURAL2      "minuti"

#define TR_OFFON_1             TR("OFF","Disattivato")
#define TR_OFFON_2             TR("ON","Attivato")
#define TR_MMMINV_1            "---"
#define TR_MMMINV_2            "INV"
#define TR_VBEEPMODE_1         TR("Silen","Silente")
#define TR_VBEEPMODE_2         TR("Avvis","Avvisi")
#define TR_VBEEPMODE_3         TR("Notst","No Tasti")
#define TR_VBEEPMODE_4         "Tutti"
#define TR_VBLMODE_1           TR("OFF","Spenta")
#define TR_VBLMODE_2           "Tasti"
#define TR_VBLMODE_3           TR("Stks","Sticks")
#define TR_VBLMODE_4           "Tutti"
#define TR_VBLMODE_5           TR("ON","Accesa")
#define TR_TRNMODE_1           "OFF"
#define TR_TRNMODE_2           TR("+=","Add")
#define TR_TRNMODE_3           TR(":=","Sost.")
#define TR_TRNCHN_1            "CH1"
#define TR_TRNCHN_2            "CH2"
#define TR_TRNCHN_3            "CH3"
#define TR_TRNCHN_4            "CH4"

#define TR_AUX_SERIAL_MODES_1           "OFF"
#define TR_AUX_SERIAL_MODES_2           "Replica Telem"
#define TR_AUX_SERIAL_MODES_3           "Telemetria In"
#define TR_AUX_SERIAL_MODES_4           "SBUS Trainer"
#define TR_AUX_SERIAL_MODES_5           "LUA"
#define TR_AUX_SERIAL_MODES_6           "CLI"
#define TR_AUX_SERIAL_MODES_7           "GPS"
#define TR_AUX_SERIAL_MODES_8           "Debug"
#define TR_AUX_SERIAL_MODES_9           "SpaceMouse"
#define TR_AUX_SERIAL_MODES_10          "Modulo esterno"
#define TR_SWTYPES_1                    "Disab."
#define TR_SWTYPES_2                    "Toggle"
#define TR_SWTYPES_3                    "2POS"
#define TR_SWTYPES_4                    "3POS"
#define TR_SWTYPES_5                    "Globali"
#define TR_POTTYPES_1                   "Disab."
#define TR_POTTYPES_2                   TR("Pot","Potenziometro")
#define TR_POTTYPES_3                   TR("Pot c. fer","Pot. con centro")
#define TR_POTTYPES_4                   "Slider"
#define TR_POTTYPES_5                   TR("Multipos","Inter. Multipos")
#define TR_POTTYPES_6                   "Axis X"
#define TR_POTTYPES_7                   "Axis Y"
#define TR_POTTYPES_8                   "Switch"
#define TR_VPERSISTENT_1                "NO"
#define TR_VPERSISTENT_2                "Volo"
#define TR_VPERSISTENT_3                "Reset Manuale"
#define TR_COUNTRY_CODES_1              TR("US","America")
#define TR_COUNTRY_CODES_2              TR("JP","Japan")
#define TR_COUNTRY_CODES_3              TR("EU","Europa")
#define TR_USBMODES_1                   TR("Chied","Chiedi")
#define TR_USBMODES_2                   TR("Joyst","Joystick")
#define TR_USBMODES_3                   TR("SDCard","Storage")
#define TR_USBMODES_4                   TR("Serial","Seriale")
#define TR_JACK_MODES_1                 "Chiedi"
#define TR_JACK_MODES_2                 "Audio"
#define TR_JACK_MODES_3                 "Trainer"

#define TR_SBUS_INVERSION_VALUES_1      "normale"
#define TR_SBUS_INVERSION_VALUES_2      "non invertito"
#define TR_MULTI_CUSTOM                 "Person."
#define TR_VTRIMINC_1                   TR("Expo","Esponenziale")
#define TR_VTRIMINC_2                   TR("ExFine","Extra Fine")
#define TR_VTRIMINC_3                   "Fine"
#define TR_VTRIMINC_4                   "Medio"
#define TR_VTRIMINC_5                   "Ampio"
#define TR_VDISPLAYTRIMS_1              "No"
#define TR_VDISPLAYTRIMS_2              "Cambio"
#define TR_VDISPLAYTRIMS_3              "Si"
#define TR_VBEEPCOUNTDOWN_1             "Niente"
#define TR_VBEEPCOUNTDOWN_2             "Suoni"
#define TR_VBEEPCOUNTDOWN_3             "Voce"
#define TR_VBEEPCOUNTDOWN_4             "Vibra"
#define TR_VBEEPCOUNTDOWN_5             TR("S & V","Suoni & Vibra")
#define TR_VBEEPCOUNTDOWN_6             TR("V & V","Voce & Vibra")
#define TR_COUNTDOWNVALUES_1            "5s"
#define TR_COUNTDOWNVALUES_2            "10s"
#define TR_COUNTDOWNVALUES_3            "20s"
#define TR_COUNTDOWNVALUES_4            "30s"
#define TR_VVARIOCENTER_1               "Tono"
#define TR_VVARIOCENTER_2               "Silenz."
#define TR_CURVE_TYPES_1                "Standard"
#define TR_CURVE_TYPES_2                "Personali"

#define TR_ADCFILTERVALUES_1            "Globali"
#define TR_ADCFILTERVALUES_2            "Off"
#define TR_ADCFILTERVALUES_3            "On"

#define TR_VCURVETYPE_1                 "Diff"
#define TR_VCURVETYPE_2                 "Espo"
#define TR_VCURVETYPE_3                 "Funz"
#define TR_VCURVETYPE_4                 "Pers"
#define TR_VMLTPX_1                     "Add."
#define TR_VMLTPX_2                     "Molt."
#define TR_VMLTPX_3                     "Sost."

#define TR_CSWTIMER                     TR("Tim", "Timer")
#define TR_CSWSTICKY                    TR("Glue", "Stcky")
#define TR_CSWSTAY                      "Edge"

#define TR_SF_TRAINER                  "Maestro"
#define TR_SF_INST_TRIM                "Inst. Trim"
#define TR_SF_RESET                    "Azzera"
#define TR_SF_SET_TIMER                "Set"
#define TR_SF_VOLUME                   "Volume"
#define TR_SF_FAILSAFE                 "SetFailsafe"
#define TR_SF_RANGE_CHECK              "RangeCheck"
#define TR_SF_MOD_BIND                 "BindModulo"
#define TR_SF_RGBLEDS                  "Leds RGB"

#define TR_SOUND                       "Suona"
#define TR_PLAY_TRACK                  TR("Ply Trk", "Suona Traccia")
#define TR_PLAY_VALUE                  TR("LeggiVal","Leggi Valore")
#define TR_SF_HAPTIC                   "Vibrazione"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Script Lua")
#define TR_SF_BG_MUSIC                 "Musica Sf"
#define TR_SF_BG_MUSIC_PAUSE           "Musica Sf ||"
#define TR_SF_LOGS                     "Logs SDCard"
#define TR_ADJUST_GVAR                 "Regola"
#define TR_SF_BACKLIGHT                "Retroillum."
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "Test"

#define TR_SF_SAFETY                   TR("Annulla", "Ignora")

#define TR_SF_SCREENSHOT               "Screenshot"
#define TR_SF_RACING_MODE              "Modo Racing"
#define TR_SF_DISABLE_TOUCH            "No Touch"
#define TR_SF_DISABLE_AUDIO_AMP        "Amp Audio Off"
#define TR_SF_SET_SCREEN               TR_BW_COL("Setta Schermo", "Setta Schermo Princ.")
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Telm", "Telemetria")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS_1          "Tmr1"
#define TR_FSW_RESET_TIMERS_2          "Tmr2"
#define TR_FSW_RESET_TIMERS_3          "Tmr3"

#define TR_VFSWRESET_1                 TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                 TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                 TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                 TR("All","Tutto")
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
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM_1              TR("Metric","Metriche")
#define TR_VUNITSSYSTEM_2              TR("Imper.","Imperiali")
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

#define TR_VTELEMSCREENTYPE_1          "Niente"
#define TR_VTELEMSCREENTYPE_2          "Valori"
#define TR_VTELEMSCREENTYPE_3          "Barre"
#define TR_VTELEMSCREENTYPE_4          "Script"
#define TR_GPSFORMAT_1                 "DMS"
#define TR_GPSFORMAT_2                 "NMEA"


#define TR_VSWASHTYPE_1                "---"
#define TR_VSWASHTYPE_2                "120"
#define TR_VSWASHTYPE_3                "120X"
#define TR_VSWASHTYPE_4                "140"
#define TR_VSWASHTYPE_5                "90"

#define TR_STICK_NAMES0                "Dir"
#define TR_STICK_NAMES1                "Ele"
#define TR_STICK_NAMES2                "Mot"
#define TR_STICK_NAMES3                "Ale"
#define TR_SURFACE_NAMES0              "Dir"
#define TR_SURFACE_NAMES1              "Mot"

#define TR_ON_ONE_SWITCHES_1            "ON"
#define TR_ON_ONE_SWITCHES_2            "One"

#define TR_HATSMODE                     "Modo joystick"
#define TR_HATSOPT_1                    "Solo trims"
#define TR_HATSOPT_2                    "Solo keys"
#define TR_HATSOPT_3                    "Commutabile"
#define TR_HATSOPT_4                    "Globale"
#define TR_HATSMODE_TRIMS               "Modo joystick: trims"
#define TR_HATSMODE_KEYS                "Modo joystick: keys"
#define TR_HATSMODE_KEYS_HELP           "Lato sinistro:\n"\
                                        " Destra = MDL\n"\
                                        " Su = SYS\n"\
                                        " Giù = TELE\n"\
                                        "\n"\
                                        "Lato destro:\n"\
                                        " Sinistra = PAGINA<\n"\
                                        " Destra = PAGINA>\n"\
                                        " Su = PREC/AUM\n"\
                                        " Giù = PROS/DIM"

#define TR_ROTARY_ENC_OPT_1          "Normale"
#define TR_ROTARY_ENC_OPT_2          "Invertito"
#define TR_ROTARY_ENC_OPT_3          "V-I H-N"
#define TR_ROTARY_ENC_OPT_4          "V-I H-A"
#define TR_ROTARY_ENC_OPT_5          "V-N E-I"

#define TR_IMU_VSRCRAW_1             "TltX"
#define TR_IMU_VSRCRAW_2             "TltY"

#define TR_CYC_VSRCRAW_1              "CYC1"
#define TR_CYC_VSRCRAW_2              "CYC2"
#define TR_CYC_VSRCRAW_3              "CYC3"

#define TR_SRC_BATT                     "Batt"
#define TR_SRC_TIME                     "Time"
#define TR_SRC_GPS                      "GPS"
#define TR_SRC_TIMER                    "Tmr"

#define TR_VTMRMODES_1                    "OFF"
#define TR_VTMRMODES_2                    "ABS"
#define TR_VTMRMODES_3                    "Strt"
#define TR_VTMRMODES_4                    "MOs"
#define TR_VTMRMODES_5                    "MO%"
#define TR_VTMRMODES_6                    "MOt"
#define TR_VTRAINER_MASTER_OFF            "OFF"
#define TR_VTRAINER_MASTER_JACK           "Maestro/Jack"
#define TR_VTRAINER_SLAVE_JACK            "Allievo/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE    "Master/Modulo SBUS"
#define TR_VTRAINER_MASTER_CPPM_MODULE    "Master/Modulo CPPM"
#define TR_VTRAINER_MASTER_BATTERY        "Master/Seriale"
#define TR_VTRAINER_BLUETOOTH_1           "Master/" TR("BT","Bluetooth")
#define TR_VTRAINER_BLUETOOTH_2           "Slave/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI                 "Master/Multi"
#define TR_VTRAINER_CRSF                  "Master/CRSF"
#define TR_VFAILSAFE_1                    "Non settato"
#define TR_VFAILSAFE_2                    "Mantieni"
#define TR_VFAILSAFE_3                    "Personali"
#define TR_VFAILSAFE_4                    "No impulsi"
#define TR_VFAILSAFE_5                    "Ricevente"
#define TR_VSENSORTYPES_1                 "Person."
#define TR_VSENSORTYPES_2                 "Calcolato"
#define TR_VFORMULAS_1                    "Somma"
#define TR_VFORMULAS_2                    "Media"
#define TR_VFORMULAS_3                    "Min"
#define TR_VFORMULAS_4                    "Max"
#define TR_VFORMULAS_5                    "Moltipl"
#define TR_VFORMULAS_6                    "Totalizza"
#define TR_VFORMULAS_7                    "Cella"
#define TR_VFORMULAS_8                    "Consumo"
#define TR_VFORMULAS_9                    "Distanza"
#define TR_VPREC_1                        "0.--"
#define TR_VPREC_2                        "0.0 "
#define TR_VPREC_3                        "0.00"
#define TR_VCELLINDEX_1                   "Minore"
#define TR_VCELLINDEX_2                   "1"
#define TR_VCELLINDEX_3                   "2"
#define TR_VCELLINDEX_4                   "3"
#define TR_VCELLINDEX_5                   "4"
#define TR_VCELLINDEX_6                   "5"
#define TR_VCELLINDEX_7                   "6"
#define TR_VCELLINDEX_8                   "7"
#define TR_VCELLINDEX_9                   "8"
#define TR_VCELLINDEX_10                  "Maggiore"
#define TR_VCELLINDEX_11                  "Delta"
#define TR_SUBTRIMMODES_1                 CHAR_DELTA" (center only)"
#define TR_SUBTRIMMODES_2                 "= (symetrical)"
#define TR_TIMER_DIR_1                    TR("Riman.", "Mostra Riman.")
#define TR_TIMER_DIR_2                    TR("Trasc.", "Mostra Trascorso")

#define TR_FONT_SIZES_1                "STD"
#define TR_FONT_SIZES_2                "BOLD"
#define TR_FONT_SIZES_3                "XXS"
#define TR_FONT_SIZES_4                "XS"
#define TR_FONT_SIZES_5                "L"
#define TR_FONT_SIZES_6                "XL"
#define TR_FONT_SIZES_7                "XXL"

#define TR_ENTER                        "[ENTER]"
#define TR_OK                           TR_BW_COL(TR("\010\010\010[OK]", "\010\010\010\010\010[OK]"), "Ok")
#define TR_EXIT                         TR_BW_COL("EXIT", "RTN")

#define TR_YES                          "Si"
#define TR_NO                           "No"
#define TR_DELETEMODEL                  "ELIMINA MODELLO"
#define TR_COPYINGMODEL                 "Copia modello..."
#define TR_MOVINGMODEL                  "Sposta modello..."
#define TR_LOADINGMODEL                 "Carica modello..."
#define TR_UNLABELEDMODEL               "Senza etic."
#define TR_NAME                         "Nome"
#define TR_MODELNAME                    TR("Nome", "Nome Modello")
#define TR_PHASENAME                    "Nome Modo"
#define TR_MIXNAME                      "Nome Mix"
#define TR_INPUTNAME                    "Nome Ingr."
#define TR_EXPONAME                     "Nome Expo"
#define TR_BITMAP                       TR ("Foto modello", "Immagine modello")
#define TR_NO_PICTURE                   "No immagine"
#define TR_TIMER                        TR("Timer", "Timer ")
#define TR_NO_TIMERS                    "No timers"
#define TR_START                        "Avvio"
#define TR_NEXT                        "Next"
#define TR_ELIMITS                      TR("Lim. Est.", "Limiti Estesi")
#define TR_ETRIMS                       TR("Trim Est.", "Trim Estesi")
#define TR_TRIMINC                      "Fasi Trim"
#define TR_DISPLAY_TRIMS                TR("Mos.Trims", "Mostra Trims")
#define TR_TTRACE                       TR("Sor. Mot.", "Sorgente Motore")
#define TR_TTRIM                        TR("Trim Mot.", "Trim Motore")
#define TR_TTRIM_SW                     TR("T-Trim-Sw", "Trim switch")
#define TR_BEEPCTR                      TR("Beep Ctr", "Beep quando centrato")
#define TR_USE_GLOBAL_FUNCS             TR("Funz. Glob.", "Usa Funz. Globali")
#define TR_PROTOCOL                     TR("Protoc.", "Protocollo")
#define TR_PPMFRAME                     "Frame PPM"
#define TR_REFRESHRATE                  TR("Refresh", "Refresh rate")
#define TR_WARN_BATTVOLTAGE            TR("Uscita VBAT: ", "Att.: livel. uscita VBAT: ")
#define TR_WARN_5VOLTS                  "Attenzione: il livello d'uscita è 5 Volts"
#define TR_MS                           "ms"
#define TR_SWITCH                       "Inter."
#define TR_FS_COLOR_LIST_1             "Custom"
#define TR_FS_COLOR_LIST_2             "Off"
#define TR_FS_COLOR_LIST_3             "Bianco"
#define TR_FS_COLOR_LIST_4             "Rosso"
#define TR_FS_COLOR_LIST_5             "Verde"
#define TR_FS_COLOR_LIST_6             "Giallo"
#define TR_FS_COLOR_LIST_7             "Arancio"
#define TR_FS_COLOR_LIST_8             "Blue"
#define TR_FS_COLOR_LIST_9             "Rosa"
#define TR_GROUP                       "Gruppo"
#define TR_GROUP_ALWAYS_ON             "Sempre on"
#define TR_LUA_OVERRIDE                "Consenti sovrascrittura Lua"
#define TR_GROUPS                      "Sempre in gruppi"
#define TR_LAST                        "Ultimo"
#define TR_MORE_INFO                   "Più info"
#define TR_SWITCH_TYPE                 "Tipo"
#define TR_SWITCH_STARTUP              "Inizio"
#define TR_SWITCH_GROUP                "Gruppo"
#define TR_SF_SWITCH                    "Trigger"
#define TR_TRIMS                        "Trims"
#define TR_FADEIN                       "Diss.In"
#define TR_FADEOUT                      "Diss.Out"
#define TR_DEFAULT                      "(Predefinita)"
#define   TR_CHECKTRIMS                  TR_BW_COL("\006Contr.\012Trims", "Contr. Trims")
#define TR_SWASHTYPE                    "Tipo Ciclico"
#define TR_COLLECTIVE                   TR("Collettivo", "Origine Collettivo")
#define TR_AILERON                      TR("Cic. later.", "Sorg. cic. later.")
#define TR_ELEVATOR                     TR("Cic. long.", "Sorg. cic. long.")
#define TR_SWASHRING                    "Anello Ciclico"
#define TR_MODE                         "Modo"
#define TR_LEFT_STICK                   "Sinistro"
#define TR_SUBTYPE                      "Sottotipo"
#define TR_NOFREEEXPO                   "Expo pieni!"
#define TR_NOFREEMIXER                  "Mixer pieni!"
#define TR_SOURCE                       "Sorg."
#define TR_WEIGHT                       "Peso"
#define TR_SIDE                         "Lato"
#define TR_OFFSET                       "Offset"
#define TR_TRIM                         "Trim"
#define TR_CURVE                        "Curva"
#define TR_FLMODE                       TR("Fase", "Fasi")
#define TR_MIXWARNING                   "Avviso"
#define TR_OFF                          "OFF"
#define TR_ANTENNA                      "Antenna"
#define TR_NO_INFORMATION               TR("No info", "No informazione")
#define TR_MULTPX                       "MultPx"
#define TR_DELAYDOWN                    "Post.Giù "
#define TR_DELAYUP                      "Post.Su"
#define TR_SLOWDOWN                     "Rall.Giù "
#define TR_SLOWUP                       "Rall.Su"
#define TR_CV                           "CV"
#define TR_GV                           TR("G", "GV")
#define TR_RANGE                        TR("Inter.", "Intervallo")
#define TR_CENTER                       "Centro"
#define TR_ALARM                        TR( "Allar.",  "Allarme")
#define TR_BLADES                       "Pale"
#define TR_SCREEN                       "Schermo\001"
#define TR_SOUND_LABEL                  "Suono"
#define TR_LENGTH                       "Durata"
#define TR_BEEP_LENGTH                  "Lung. Beep"
#define TR_BEEP_PITCH                   "Tono"
#define TR_HAPTIC_LABEL                 "Vibrazione"
#define TR_STRENGTH                     "Forza"
#define TR_IMU_LABEL                    "IMU"
#define TR_IMU_OFFSET                   "Offset"
#define TR_IMU_MAX                      "Max"
#define TR_CONTRAST                     "Contrasto"
#define TR_ALARMS_LABEL                 "Allarmi"
#define TR_BATTERY_RANGE                TR("Int. Batt.", "Intervallo batteria")
#define TR_BATTERYCHARGING              "Ricarica..."
#define TR_BATTERYFULL                  "Bat. carica"
#define TR_BATTERYNONE                  "Assente!"
#define TR_BATTERYWARNING               TR("Batt. Bassa", "Batteria Scarica")
#define TR_INACTIVITYALARM              "Inattività"
#define TR_MEMORYWARNING                TR("Avv. Mem.", "Avviso Memoria")
#define TR_ALARMWARNING                 "Suono spento"
#define TR_RSSI_SHUTDOWN_ALARM          TR("RSSI spento", "controlla RSSI spegnendo")
#define TR_TRAINER_SHUTDOWN_ALARM       TR("Spegnimento Trainer", "Controlla trainer spegnendo")
#define TR_MODEL_STILL_POWERED          "Radio ancora\nconnessa"
#define TR_TRAINER_STILL_CONNECTED      "Trainer ancora\nconnesso"
#define TR_USB_STILL_CONNECTED          "USB ancora\nconnesso"
#define TR_MODEL_SHUTDOWN               "Spegnimento?"
#define TR_PRESS_ENTER_TO_CONFIRM       "Premi [ENT] per confermare"
#define TR_THROTTLE_LABEL               "Motore"
#define TR_THROTTLE_START               "Partenza motore"
#define TR_THROTTLEREVERSE              TR("T-Invertito", "Invertito")
#define TR_MINUTEBEEP                   TR("Minuto", "Minuto breve")
#define TR_BEEPCOUNTDOWN                TR("Conto rov", "Conto rovescia")
#define TR_PERSISTENT                   TR("Persist.", "Persistente")
#define TR_BACKLIGHT_LABEL              TR("Retroillu.", "Retroilluminazione")
#define TR_GHOST_MENU_LABEL             "GHOST MENU"
#define TR_STATUS                       "Stato"
#define TR_BLONBRIGHTNESS               "Luminosità ON"
#define TR_BLOFFBRIGHTNESS              "Luminosità OFF"
#define TR_KEYS_BACKLIGHT               "Luce tasti"
#define TR_BLCOLOR                      "Colore"
#define TR_SPLASHSCREEN                 TR("Sch. avvio", "Schermata d'avvio")
#define TR_PLAY_HELLO                   "Suono all'accensione"
#define TR_PWR_ON_DELAY                 "Rit. accens."
#define TR_PWR_OFF_DELAY                "Rit. spegni."
#define TR_PWR_AUTO_OFF                 TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC            TR("Vibraz. Pwr ON/OFF","Vibrazione Pwr ON/OFF")
#define TR_THROTTLE_WARNING             TR("All. Mot.", "Allarme Motore")
#define TR_CUSTOM_THROTTLE_WARNING      TR("Cust-Pos", "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL  TR("Pos. %", "Position %")
#define TR_SWITCHWARNING                TR("Avv. Int.", "Avviso Interr.")
#define TR_POTWARNINGSTATE              "Pots & sliders"
#define TR_POTWARNING                   TR("Avv. Pot.", "Avviso Pot.")
#define TR_TIMEZONE                     "Ora locale"
#define TR_ADJUST_RTC                   TR("Agg. RTC", "Aggiusta RTC")
#define TR_GPS                          "GPS"
#define TR_DEF_CHAN_ORD                 TR("Ord. can. pred.", "Ordine canali predefinito")
#define TR_STICKS                       "Sticks"
#define TR_POTS                         "Pot."
#define TR_SWITCHES                     "Interutt."
#define TR_SWITCHES_DELAY               TR("Rit. V. FV", "Ritardo Voce FV")
#define TR_SLAVE                        "Allievo"
#define TR_MODESRC                      "Modo\006% Origine"
#define TR_MULTIPLIER                   "Moltiplica"
#define TR_CAL                          "Cal"
#define TR_CALIBRATION                  BUTTON("Calibrazione")
#define TR_VTRIM                        "Trim - +"
#define TR_CALIB_DONE                   "Calibratione completa"
#define TR_MENUTOSTART                  TR_ENTER "PER START"
#define TR_MENUWHENDONE                 TR_ENTER " PER FINIRE"
#define TR_AXISDIR                      "AXIS DIR"
#define TR_MENUAXISDIR                  "[ENTER LONG] "  TR_AXISDIR
#define TR_SETMIDPOINT                  TR_BW_COL(TR_SFC_AIR(TR("CENTRA STICKS A META'", "CENTRA STICKS/SLIDERS"), TR("SETTA ASSI AL CENTRO", "CENTRA ASSI/SLIDERS")), "CENTRA STICKS/SLIDERS")
#define TR_MOVESTICKSPOTS               TR_BW_COL(TR_SFC_AIR("MUOVI STICK/POT", "MUOVI ASSI/POTS"), "MUOVI STICKS/POTS")
#define TR_NODATA                       "DATI ASSENTI"
#define TR_US                           "us"
#define TR_HZ                           "Hz"
#define TR_TMIXMAXMS                    "Tmix max"
#define TR_FREE_STACK                   "Stack libero"
#define TR_INT_GPS_LABEL                "GPS interno"
#define TR_HEARTBEAT_LABEL              "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL            "Lua scripts"
#define TR_FREE_MEM_LABEL               "Mem. libera"
#define TR_DURATION_MS                  TR("[D]","Duration(ms): ")
#define TR_INTERVAL_MS                  TR("[I]","Interval(ms): ")
#define TR_MEM_USED_SCRIPT              "Script(B): "
#define TR_MEM_USED_WIDGET              "Widget(B): "
#define TR_MEM_USED_EXTRA               "Extra(B): "
#define TR_STACK_MIX                    "Mix: "
#define TR_STACK_AUDIO                  "Audio: "
#define TR_GPS_FIX_YES                  "Fix: Sì"
#define TR_GPS_FIX_NO                   "Fix: No"
#define TR_GPS_SATS                     "Sats: "
#define TR_GPS_HDOP                     "Hdop: "
#define TR_STACK_MENU                   "Menu: "
#define TR_TIMER_LABEL                  "Timer"
#define TR_THROTTLE_PERCENT_LABEL       "% Motore"
#define TR_BATT_LABEL                   "Batteria"
#define TR_SESSION                      "Sessione"
#define TR_MENUTORESET                  TR_ENTER" per resettare"
#define TR_PPM_TRAINER                  "TR"
#define TR_CH                           "CH"
#define TR_MODEL                        "MODELLO"
#define TR_FM                          TR_SFC_AIR("DM", "FV")
#define TR_EEPROMLOWMEM                 "EEPROM quasi piena!"
#define TR_PRESS_ANY_KEY_TO_SKIP        "\010Premi un tasto"
#define TR_THROTTLE_NOT_IDLE            "Gas non a minimo"
#define TR_ALARMSDISABLED               "Allarmi Disattivati!"
#define TR_PRESSANYKEY                  TR("\010Premi un tasto", "Premi un tasto")
#define TR_BADEEPROMDATA                "Dati corrotti!"
#define TR_BAD_RADIO_DATA               "Dati nella cartella RADIO/nerrati o assenti"
#define TR_RADIO_DATA_RECOVERED         TR3("Uso i dati radio di backup","Uso i settaggi radio di backup","Settaggi Radio ripresi da un backup")
#define TR_RADIO_DATA_UNRECOVERABLE     TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING             "Formatto EEPROM..."
#define TR_STORAGE_FORMAT               "Preparazione storage"
#define TR_EEPROMOVERFLOW               "EEPROM Piena"
#define TR_RADIO_SETUP                  "CONFIGURA TX"
#define TR_MENUVERSION                  "VERSIONE"
#define TR_MENU_RADIO_ANALOGS           "ANAS"
#define TR_MENU_RADIO_ANALOGS_CALIB     "ANALOGICI CALIBRATI"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "ANALOGICI RAW (5 Hz)"
#define TR_MENU_FSWITCH                 "INTERRUTTORI PERSONALIZZABILI"
#define   TR_TRIMS2OFFSETS               TR_BW_COL("\006Trims  => Offset ", "Trims => Subtrims")
#define TR_CHANNELS2FAILSAFE            "Canali => Failsafe"
#define TR_CHANNEL2FAILSAFE             "Canale => Failsafe"
#define TR_MENUMODELSEL                 TR("MODELSEL", "SELEEZIONE MODELLO")
#define TR_MENU_MODEL_SETUP             TR("SETTA", "SETTA MODELLO")
#define TR_MENUCURVE                    "CURVA"
#define TR_MENULOGICALSWITCH            "INTER. PERSON."
#define TR_MENUSTAT                     "STATO"
#define TR_MENUDEBUG                    "DEBUG"
#define TR_MONITOR_CHANNELS             "MONITOR CANALI %d/%d"
#define TR_MONITOR_OUTPUT_DESC          "Uscite"
#define TR_MONITOR_MIXER_DESC           "Mixers"
#define TR_RECEIVER_NUM                 TR("RxNum", "Ricevente N.")
#define TR_RECEIVER                     "Ricevente"
#define TR_MULTI_RFTUNE                 TR("Tune RF", "Tune fine Freq. RF")
#define TR_MULTI_RFPOWER                "RF power"
#define TR_MULTI_WBUS                   "Output"
#define TR_MULTI_TELEMETRY              "Telemetria"
#define TR_MULTI_VIDFREQ                TR("Freq. video", "Frequenza video")
#define TR_RF_POWER                     "Potenza RF"
#define TR_MULTI_FIXEDID                TR("ID fisso", "ID Fisso")
#define TR_MULTI_OPTION                 TR("Opzione", "Opzione valore")
#define TR_MULTI_AUTOBIND               TR("Ass. Ch.","Associa al canale")
#define TR_DISABLE_CH_MAP               TR("No Ch. map", "Disab. mappa Ch.")
#define TR_DISABLE_TELEM                TR("No Telem", "Telem. disabil.")
#define TR_MULTI_LOWPOWER               TR("Bassa pot.", "Modo bassa potenza")
#define TR_MULTI_LNA_DISABLE            "Disabilita LNA"
#define TR_MODULE_TELEMETRY             TR("S.Port", "Link S.Port")
#define TR_MODULE_TELEM_ON              TR("ON", "Abilitato")
#define TR_DISABLE_INTERNAL             TR("Disatt. RF int.", "Disattiva RF interna")
#define TR_MODULE_NO_SERIAL_MODE        TR("!modo seriale", "Non in modo seriale")
#define TR_MODULE_NO_INPUT              TR("Nessun input", "Nessun input seriale")
#define TR_MODULE_NO_TELEMETRY          TR3("No telemetria", "No MULTI_TELEMETRY", "Nessuna MULTI_TELEMETRY rilevata")
#define TR_MODULE_WAITFORBIND           "Associa per caricare il protocollo"
#define TR_MODULE_BINDING               "Associa"
#define TR_MODULE_UPGRADE_ALERT         TR3("Richiede agg.", "Richiede agg. modulo", "Modulo\nRichiede agg.")
#define TR_MODULE_UPGRADE               TR("Cons. agg.", "Consiglio agg. modulo")
#define TR_REBIND                       "Richiede associazione"
#define TR_REG_OK                       "Registrazione ok"
#define TR_BIND_OK                      "Associazione riuscita"
#define TR_BINDING_CH1_8_TELEM_ON       "Ch1-8 Telem ON"
#define TR_BINDING_CH1_8_TELEM_OFF      "Ch1-8 Telem OFF"
#define TR_BINDING_CH9_16_TELEM_ON      "Ch9-16 Telem ON"
#define TR_BINDING_CH9_16_TELEM_OFF     "Ch9-16 Telem OFF"
#define TR_PROTOCOL_INVALID             TR("Prot. invalido", "Protocollo invalido")
#define TR_MODULE_STATUS                TR("Stato", "Stato del modulo")
#define TR_MODULE_SYNC                  TR("Sinc.", "Sinc. del modulo")
#define TR_MULTI_SERVOFREQ              TR("Servo rate", "Aggiorna servo rate")
#define TR_MULTI_MAX_THROW              TR("Escurs. mass.", "Abilita escurs. mass.")
#define TR_MULTI_RFCHAN                 TR("Canale RF", "Selez. canale RF")
#define TR_AFHDS3_RX_FREQ               TR("RX freq.", "RX frequenza")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY  TR("Unicast/Tel.", "Unicast/Telemetria")
#define TR_AFHDS3_ONE_TO_MANY           "Multicast"
#define TR_AFHDS3_ACTUAL_POWER          TR("Pot. att.", "Potenza Attuale")
#define TR_AFHDS3_POWER_SOURCE          TR("Fonte pot.", "Fonte Potenza")
#define TR_FLYSKY_TELEMETRY             TR("FlySky RSSI #", "Usa valore RSSI FlySky senza scalare")
#define TR_GPS_COORDS_FORMAT            TR("GPS Coords", "Formato Coordinate")
#define TR_VARIO                        TR("Vario", "Variometro")
#define TR_PITCH_AT_ZERO                "Pitch a Zero"
#define TR_PITCH_AT_MAX                 "Pitch al Max"
#define TR_REPEAT_AT_ZERO               "Ripeti a Zero"
#define TR_BATT_CALIB                   TR("Calibra batt.", "Calibra batteria")
#define TR_CURRENT_CALIB                "Calibrazione"
#define TR_VOLTAGE                      TR("Voltagg.","Voltaggio")
#define TR_SELECT_MODEL                 "Scegli Modello"
#define TR_MANAGE_MODELS                "GESTIONE MODELLI"
#define TR_MODELS                       "Modelli"
#define TR_SELECT_MODE                  "Seleziona modo"
#define TR_CREATE_MODEL                 "Crea Modello"
#define TR_FAVORITE_LABEL               "Favoriti"
#define TR_MODELS_MOVED                 "Modelli non in uso spostati in"
#define TR_NEW_MODEL                    "Nuovo Modello"
#define TR_INVALID_MODEL                "Modello invalido"
#define TR_EDIT_LABELS                  "Edita Etichette"
#define TR_LABEL_MODEL                  "Etichetta modello"
#define TR_MOVE_UP                      "Muovi Su"
#define TR_MOVE_DOWN                    "Muovi Giù"
#define TR_ENTER_LABEL                  "Edita Etichetta"
#define TR_LABEL                        "Etichetta"
#define TR_LABELS                       "Etichette"
#define TR_CURRENT_MODEL                "Corrente"
#define TR_ACTIVE                       "Attivo"
#define TR_NEW                          "Nuovo"
#define TR_NEW_LABEL                    "Nuova Etichetta"
#define TR_RENAME_LABEL                 "Rinomina Etichetta"
#define TR_DELETE_LABEL                 "Elimina Etichetta"
#define TR_DUPLICATE_MODEL              "Duplica Modello"
#define TR_COPY_MODEL                   "Copia Modello"
#define TR_MOVE_MODEL                   "Sposta Modello"
#define TR_BACKUP_MODEL                 "Salva Modello"
#define TR_DELETE_MODEL                 TR("Elim. Modello", "Elimina Modello")
#define TR_RESTORE_MODEL                TR("Ripr. Modello", "Ripristina Modello")
#define TR_DELETE_ERROR                 "Errore cancell."
#define TR_SDCARD_ERROR                 TR("Errore SD", "Errore SDCard")
#define TR_SDCARD                       "SD Card"
#define TR_NO_FILES_ON_SD               "SD senza file!"
#define TR_NO_SDCARD                    "No SDCard"
#define TR_WAITING_FOR_RX               "Attendo la RX..."
#define TR_WAITING_FOR_TX               "Attendo la TX..."
#define TR_WAITING_FOR_MODULE           TR("Attendo modulo", "Attendo il modulo...")
#define TR_NO_TOOLS                     "Tools non disp."
#define TR_NORMAL                       "Normale"
#define TR_NOT_INVERTED                 "Non inv."
#define TR_NOT_CONNECTED                TR("!Connesso", "Non Connesso")
#define TR_CONNECTED                    "Connesso"
#define TR_FLEX_915                     "Flex 915MHz"
#define TR_FLEX_868                     "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY       TR("16CH senza telem.", "16CH senza telemetria")
#define TR_16CH_WITH_TELEMETRY          TR("16CH con telem.", "16CH con telemetria")
#define TR_8CH_WITH_TELEMETRY           TR("8CH con telem.", "8CH con telemetria")
#define TR_EXT_ANTENNA                  "Antenna Ext."
#define TR_PIN                          "Pin"
#define TR_UPDATE_RX_OPTIONS            "Agg. opzioni RX?"
#define TR_UPDATE_TX_OPTIONS            "Agg. opzioni TX?"
#define TR_MODULES_RX_VERSION           BUTTON("Versioni Moduli / RX")
#define TR_SHOW_MIXER_MONITORS          "Mostra monitors mixer"
#define TR_MENU_MODULES_RX_VERSION      "VERSIONI MODULI / RX"
#define TR_MENU_FIRM_OPTIONS            "OPZIONI FIRMWARE"
#define TR_IMU                          "IMU"
#define TR_STICKS_POTS_SLIDERS          "Interruttori/Pots/Sliders"
#define TR_PWM_STICKS_POTS_SLIDERS      "PWM Interruttori/Pots/Sliders"
#define TR_RF_PROTOCOL                  "Protocollo RF"
#define TR_MODULE_OPTIONS               "Optioni modulo"
#define TR_POWER                        "Potenza"
#define TR_NO_TX_OPTIONS                "No Opzioni TX"
#define TR_RTC_BATT                     "RTC Batt"
#define TR_POWER_METER_EXT              "Meter Potenza (EST)"
#define TR_POWER_METER_INT              "Meter Potenza (INT)"
#define TR_SPECTRUM_ANALYSER_EXT        "Spettro (EST)"
#define TR_SPECTRUM_ANALYSER_INT        "Spettro (INT)"
#define TR_GHOST_MODULE_CONFIG         "Config. modulo Ghost"
#define TR_GPS_MODEL_LOCATOR           "Trova modello con GPS"
#define TR_REFRESH                     "Ricarica"
#define TR_SDCARD_FULL                  "SDCard piena"
#define TR_SDCARD_FULL_EXT  TR_BW_COL(TR_SDCARD_FULL "\036Logs e Screenshots" LCDW_128_LINEBREAK "disattivati", TR_SDCARD_FULL "\nLogs e Screenshots disattivati")
#define TR_NEEDS_FILE                   "RICHIEDE FILE"
#define TR_EXT_MULTI_SPEC               "opentx-inv"
#define TR_INT_MULTI_SPEC               "stm-opentx-noinv"
#define TR_INCOMPATIBLE                 "Incompatibile"
#define TR_WARNING                      "AVVISO"
#define TR_STORAGE_WARNING              "STORAGE"
#define TR_THROTTLE_UPPERCASE           "MOTORE"
#define TR_ALARMSWARN                   "ALLARMI"
#define TR_SWITCHWARN                   "CONTROLLI"
#define TR_FAILSAFEWARN                 "FAILSAFE"
#define TR_TEST_WARNING                 TR("PER TEST", "BUILD PER TEST")
#define TR_TEST_NOTSAFE                 "Usare solo per test"
#define TR_WRONG_SDCARDVERSION          TR("Richiede ver: ", "Richiede versione: ")
#define TR_WARN_RTC_BATTERY_LOW         "Batteria RTC scarica"
#define TR_WARN_MULTI_LOWPOWER          "Modalità bassa pot."
#define TR_BATTERY                      "BATTERIA"
#define TR_WRONG_PCBREV                 "PCB errato"
#define TR_EMERGENCY_MODE               "MODALITA' EMERGENZA"
#define TR_NO_FAILSAFE                  "Failsafe non settato"
#define TR_KEYSTUCK                     "Tasto bloccato"
#define TR_VOLUME                       "Volume Audio"
#define TR_LCD                          "LCD"
#define TR_BRIGHTNESS                   "Luminosità"
#define TR_CPU_TEMP                     "Temp CPU \016>"
#define TR_COPROC                       "CoProc."
#define TR_COPROC_TEMP                  "Temp. MB \016>"
#define TR_TTL_WARNING                  "Attenzione: non superare 3.3V sui TX/RX pins!"
#define TR_FUNC                         "Funzione"
#define TR_V1                           "V1"
#define TR_V2                           "V2"
#define TR_DURATION                     "Durata"
#define TR_DELAY                        "Ritardo"
#define TR_NO_SOUNDS_ON_SD              "No Suoni su SD"
#define TR_NO_MODELS_ON_SD              "No Model. su SD"
#define TR_NO_BITMAPS_ON_SD             "No Immag. su SD"
#define TR_NO_SCRIPTS_ON_SD             "No Scripts su SD"
#define TR_SCRIPT_SYNTAX_ERROR          TR("Errore sintassi", "Script errore sintassi")
#define TR_SCRIPT_PANIC                 "Script panico"
#define TR_SCRIPT_KILLED                "Script fermato"
#define TR_SCRIPT_ERROR                 "Errore sconosciuto"
#define TR_PLAY_FILE                    "Suona"
#define TR_DELETE_FILE                  "Elimina"
#define TR_COPY_FILE                    "Copia"
#define TR_RENAME_FILE                  "Rinomina"
#define TR_ASSIGN_BITMAP                "Assegna immagine"
#define TR_ASSIGN_SPLASH                "Splash screen"
#define TR_EXECUTE_FILE                 "Esegui"
#define TR_REMOVED                      " rimosso"
#define TR_SD_INFO                      "Informazioni"
#define TR_NA                           "N/A"
#define TR_FORMATTING                   "Formattazione"
#define TR_TEMP_CALIB                   "Temp. Calib."
#define TR_TIME                         "Ora"
#define TR_MAXBAUDRATE                  "Max Bauds"
#define TR_BAUDRATE                     "Baudrate"
#define TR_CRSF_ARMING_MODE             TR("Modo arm", "Modo d'armamento")
#define TR_CRSF_ARMING_MODES            TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                  "Modo Semplice"
#define TR_SAMPLE_MODES_1               "Normale"
#define TR_SAMPLE_MODES_2               "OneBit"
#define TR_LOADING                      "Caricamento..."
#define TR_DELETE_THEME                 "Cancello Tema?"
#define TR_SAVE_THEME                   "Salvo Tema?"
#define TR_EDIT_COLOR                   "Edita colore"
#define TR_NO_THEME_IMAGE               "Nessuna immagine trovata"
#define TR_BACKLIGHT_TIMER              "Tempo d'inattività"

#define TR_MODEL_QUICK_SELECT       "Selezione rapida modello"
#define TR_LABELS_SELECT            "Seleziona etichette"
#define TR_LABELS_MATCH             "Abbinamento etichette"
#define TR_FAV_MATCH                "Abbinamento favoriti"
#define TR_LABELS_SELECT_MODE_1     "Selezione multipla"
#define TR_LABELS_SELECT_MODE_2     "Selezione singola"
#define TR_LABELS_MATCH_MODE_1      "Abbina tutti"
#define TR_LABELS_MATCH_MODE_2      "Abbina qualsiasi"
#define TR_FAV_MATCH_MODE_1         "Deve coincidere"
#define TR_FAV_MATCH_MODE_2         "Abbinamento facoltativo"

#define TR_SELECT_TEMPLATE_FOLDER     "Seleziona Cartella Template:"
#define TR_SELECT_TEMPLATE            "SELEZIONA TEMPLATE MODELLO:"
#define TR_NO_TEMPLATES               "Nessun template di modello in questa cartella"
#define TR_SAVE_TEMPLATE              "Salva come template"
#define TR_BLANK_MODEL                "Modello vuoto"
#define TR_BLANK_MODEL_INFO           "Crea modello vuoto"
#define TR_FILE_EXISTS                "FILE GIA' ESISTENTE"
#define TR_ASK_OVERWRITE              "Vuoi sovrascrivere?"

#define TR_BLUETOOTH                  "Bluetooth"
#define TR_BLUETOOTH_DISC             "Cerca"
#define TR_BLUETOOTH_INIT             "Iniz."
#define TR_BLUETOOTH_DIST_ADDR        "Ind. Dist."
#define TR_BLUETOOTH_LOCAL_ADDR       "Ind. Loc."
#define TR_BLUETOOTH_PIN_CODE         "Codice PIN"
#define TR_BLUETOOTH_NODEVICES        "Nessun Dispositivo Trovato"
#define TR_BLUETOOTH_SCANNING         "Scansione..."
#define TR_BLUETOOTH_BAUDRATE         "BT Baudrate"
#define TR_BLUETOOTH_MODES_1          "---"
#define TR_BLUETOOTH_MODES_2          "Telemetr."
#define TR_BLUETOOTH_MODES_3          "Trainer"
#define TR_BLUETOOTH_MODES_4          "Attivo"

#define TR_SD_INFO_TITLE                "INFO SD"
#define TR_SD_SPEED                     "Veloc.:"
#define TR_SD_SECTORS                   "Settori:"
#define TR_SD_SIZE                      "Dimens:"
#define TR_TYPE                         "Tipo"
#define TR_GVARS                        "V.GLOBALI"
#define TR_GLOBAL_VAR                   "Variabile globale"
#define TR_OWN                          "Fase"
#define TR_DATE                         "Data"
#define TR_MONTHS_1                     "Gen"
#define TR_MONTHS_2                     "Feb"
#define TR_MONTHS_3                     "Mar"
#define TR_MONTHS_4                     "Apr"
#define TR_MONTHS_5                     "Mag"
#define TR_MONTHS_6                     "Giu"
#define TR_MONTHS_7                     "Lug"
#define TR_MONTHS_8                     "Ago"
#define TR_MONTHS_9                     "Set"
#define TR_MONTHS_10                    "Ott"
#define TR_MONTHS_11                    "Nov"
#define TR_MONTHS_12                    "Dic"
#define TR_ROTARY_ENCODER               "R.E."
#define TR_ROTARY_ENC_MODE              TR("Modo RotEnc","Modo Rotary Encoder")
#define TR_CHANNELS_MONITOR             "MONITOR CANALI"
#define TR_MIXERS_MONITOR               "MONITOR MIXER"
#define TR_PATH_TOO_LONG                "Path troppo lungo"
#define TR_VIEW_TEXT                    "Vedi testo"
#define TR_FLASH_BOOTLOADER             TR("Prog. bootloader", "Programma bootloader")
#define TR_FLASH_DEVICE                 TR("Flash device","Flash device")
#define TR_FLASH_EXTERNAL_DEVICE        TR("Prog. disp. est.", "Programma dispositivo esterno")
#define TR_FLASH_RECEIVER_OTA           TR("Prog. RX OTA", "Programma RX OTA")
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA          TR("Flash RX da OTA est.", "Flash RX da OTA esterno")
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA          TR("Flash RX da OTA int.", "Flash RX da OTA interno")
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA TR("Flash FC da OTA est.", "Flash FC da OTA esterno")
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA TR("Flash FC da OTA int.", "Flash FC da OTA interno")
#define TR_FLASH_BLUETOOTH_MODULE       TR("Flash mod. BT", "Flash modulo Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT  TR("Flash PMU", "Programma PMU")
#define TR_DEVICE_NO_RESPONSE           TR("Device non risponde", "Device non risponde")
#define TR_DEVICE_FILE_ERROR            TR("Device file prob.", "Device file prob.")
#define TR_DEVICE_DATA_REFUSED          TR("Device dati rifiutati", "Device dati rifiutati")
#define TR_DEVICE_WRONG_REQUEST         TR("Device problema accesso", "Device problema accesso")
#define TR_DEVICE_FILE_REJECTED         TR("Device file rifiutato", "Device file rifiutato")
#define TR_DEVICE_FILE_WRONG_SIG        TR("Device firma file", "Device firma file")
#define TR_CURRENT_VERSION              TR("Vers. currente ", "Versione corrente: ")
#define TR_FLASH_EXTERNAL_ELRS          TR("Scrivi ELRS Est.", "Scrivi ELRS Esterno")
#define TR_FLASH_INTERNAL_MODULE        TR("Prog. modulo int.", "Programma modulo interno")
#define TR_FLASH_EXTERNAL_MODULE        TR("Prog. modulo est.", "Programma modulo esterno")
#define TR_FLASH_INTERNAL_MULTI         TR("Prog. MULTI int.", "Programma MULTI interno")
#define TR_FLASH_EXTERNAL_MULTI         TR("Prog. MULTI est.", "Programma MULTI esterno")
#define TR_FIRMWARE_UPDATE_ERROR        TR( "Errore agg. FW","Errore aggiornamento firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS      "Flash riuscito"
#define TR_WRITING                      "Scrittura..."
#define TR_CONFIRM_FORMAT               "Confermi formattazione?"
#define TR_INTERNALRF                   "Modulo interno"
#define TR_INTERNAL_MODULE              TR("Modulo int.", "Modulo interno")
#define TR_EXTERNAL_MODULE              TR("Modulo est.", "Modulo esterno")
#define TR_EDGETX_UPGRADE_REQUIRED      "EdgeTX richiede aggiornamento"
#define TR_TELEMETRY_DISABLED           TR("Telem. off", "Telem. disabilitata")
#define TR_MORE_OPTIONS_AVAILABLE       "Altre opzioni assenti"
#define TR_NO_MODULE_INFORMATION        "Nessuna info del modulo"
#define TR_EXTERNALRF                   "Modulo esterno"
#define TR_FAILSAFE                     TR("Failsafe", "Modo failsafe")
#define TR_FAILSAFESET                  TR("FAILSAFE", "IMPOSTAZIONI FAILSAFE")
#define TR_REG_ID                       "Reg. ID"
#define TR_OWNER_ID                     "Owner ID"
#define TR_HOLD                         "tieni"
#define TR_HOLD_UPPERCASE               "TIENI"
#define TR_NONE                         "None"
#define TR_NONE_UPPERCASE               "NONE"
#define TR_MENUSENSOR                   "SENSORE"
#define TR_POWERMETER_PEAK              "Picco"
#define TR_POWERMETER_POWER             "Potenza"
#define TR_POWERMETER_ATTN              "Att."
#define TR_POWERMETER_FREQ              "Freq."
#define TR_MENUTOOLS                    "TOOLS"
#define TR_TURN_OFF_RECEIVER            "Spegni la RX"
#define TR_STOPPING                     "Fermando..."
#define TR_MENU_SPECTRUM_ANALYSER       "ANALIZZATORE SPETTRO"
#define TR_MENU_POWER_METER             "MISURATORE POTENZA"
#define TR_SENSOR                       "SENSORE"
#define TR_COUNTRY_CODE                 TR("Cod. Paese", "Codice del Paese")
#define TR_USBMODE                      "Modo USB"
#define TR_JACK_MODE                    "Modo JACK"
#define TR_VOICE_LANGUAGE               "Lingua vocale"
#define TR_UNITS_SYSTEM                 "Unità"
#define TR_UNITS_PPM                    "Unità PPM"
#define TR_EDIT                         "Modifica"
#define TR_INSERT_BEFORE                "Inserisci prima"
#define TR_INSERT_AFTER                 "Inserisci dopo"
#define TR_COPY                         "Copia"
#define TR_MOVE                         "Sposta"
#define TR_PASTE                        "Incolla"
#define TR_PASTE_AFTER                  "Incolla Dopo"
#define TR_PASTE_BEFORE                 "Incolla Prima"
#define TR_DELETE                       "Elimina"
#define TR_INSERT                       TR("Inser.","Inserisci")
#define TR_RESET_FLIGHT                 "Azzera volo"
#define TR_RESET_TIMER1                 "Azzera Timer1"
#define TR_RESET_TIMER2                 "Azzera Timer2"
#define TR_RESET_TIMER3                 "Azzera Timer3"
#define TR_RESET_TELEMETRY              "Azzera Telemetria"
#define TR_STATISTICS                   "Statistiche"
#define TR_ABOUT_US                     "Info su"
#define TR_USB_JOYSTICK                 "Joystick USB (HID)"
#define TR_USB_MASS_STORAGE             "Storage USB (SD)"
#define TR_USB_SERIAL                   "Seriale USB (VCP)"
#define TR_SETUP_SCREENS                "Schermate conf."
#define TR_MONITOR_SCREENS              "Monitors"
#define TR_AND_SWITCH                   "Inter. AND"
#define TR_SF                           "CF"
#define TR_GF                           "GF"
#define TR_ANADIAGS_CALIB               "Analogici calibrati"
#define TR_ANADIAGS_FILTRAWDEV          "Analogici grezzi filtrati con deviazione"
#define TR_ANADIAGS_UNFILTRAW           "Analogici RAW non filtrati"
#define TR_ANADIAGS_MINMAX              "Min., max. e range"
#define TR_ANADIAGS_MOVE                "Muovi analogici ai loro estremi!"
#define TR_BYTES                        "Bytes"
#define TR_MODULE_BIND                  TR("[Bnd]","[Bind]")
#define TR_MODULE_UNBIND                BUTTON("Unbind")
#define TR_POWERMETER_ATTN_NEEDED       "Attenuatore necessario"
#define TR_PXX2_SELECT_RX               "Seleziona RX..."
#define TR_PXX2_DEFAULT                 "<default>"
#define TR_BT_SELECT_DEVICE             "Seleziona dispositivo"
#define TR_DISCOVER                     BUTTON("Cerca")
#define TR_BUTTON_INIT                  BUTTON("Init")
#define TR_WAITING                      "In attesa..."
#define TR_RECEIVER_DELETE              "Cancello RX?"
#define TR_RECEIVER_RESET               "Resetto RX?"
#define TR_SHARE                        "Condividere"
#define TR_BIND                         "Bind"
#define TR_REGISTER                     BUTTON(TR("Reg", "Registrare"))
#define TR_MODULE_RANGE                 TR("[Rng]","[Range]")
#define TR_RANGE_TEST                  "Test del link radio a bassa potenza"
#define TR_RECEIVER_OPTIONS             TR("OPZIONI RX", "OPZIONI RICEVENTE")
#define TR_RESET_BTN                    "[Reset]"
#define TR_KEYS_BTN                     BUTTON(TR("SW", "Tasti"))
#define TR_ANALOGS_BTN                  BUTTON("Analogici")
#define TR_FS_BTN                       BUTTON(TR("Int. pers.", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND               "Schermo touch non trovato"
#define TR_TOUCH_EXIT                   "Tocca lo schermo per uscire"
#define TR_SET                          BUTTON("Set")
#define TR_TRAINER                      TR("Trainer", "Maestro/Allievo")
#define TR_CHANS                        "Canali"
#define TR_ANTENNAPROBLEM               "Problemi antenna TX!"
#define TR_MODELIDUSED                  "ID usato su:"
#define TR_MODELIDUNIQUE                "ID è unico"
#define TR_MODULE                       "Modulo"
#define TR_RX_NAME                      "Nome RX"
#define TR_TELEMETRY_TYPE               TR("Tipo", "Telemetria tipo")
#define TR_TELEMETRY_SENSORS            "Sensori"
#define TR_VALUE                        "Valore"
#define TR_PERIOD                       "Periodo"
#define TR_INTERVAL                     "Intervallo"
#define TR_REPEAT                       "Ripeti"
#define TR_ENABLE                       "Abilita"
#define TR_DISABLE                      "Disabilita"
#define TR_TOPLCDTIMER                  "Timer LCD Su"
#define TR_UNIT                         "Unità"
#define TR_TELEMETRY_NEWSENSOR          "Aggiungi sensore"
#define TR_CHANNELRANGE                 TR("Num Canali", "Numero Canali")
#define TR_ANTENNACONFIRM1              "ANTENNA EST."
#define TR_ANTENNA_MODES_1           "Interna"
#define TR_ANTENNA_MODES_2           "Chiedi"
#define TR_ANTENNA_MODES_3           "Per modello"
#define TR_ANTENNA_MODES_4           "Intern + Esterno"
#define TR_ANTENNA_MODES_5           "Esterno"
#define TR_USE_INTERNAL_ANTENNA               TR("Usa ant. int.", "Usa antenna interna")
#define TR_USE_EXTERNAL_ANTENNA               TR("Usa ant. est.", "Usa antenna esterna")
#define TR_ANTENNACONFIRM2                    TR("Controlla instal.", "Controlla prima se installata!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1    "Non richiede"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1     "Richiede FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1      "Richiede EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2         "firmware certificato"
#define TR_LOWALARM                        "Allarme Basso"
#define TR_CRITICALALARM                   "Allarme Critico"
#define TR_DISABLE_ALARM                   TR("Allarmi disab.", "Allarmi telemetrici disabilitati")
#define TR_POPUP                           "Popup"
#define TR_MIN                             "Min"
#define TR_MAX                             "Max"
#define TR_CURVE_PRESET                    "Preimpostate..."
#define TR_PRESET                          "Preimpostate"
#define TR_MIRROR                          "Mirror"
#define TR_CLEAR                           "Cancella"
#define TR_CLEAR_BTN                       BUTTON("Cancella")
#define TR_RESET                           "Azzera"
#define TR_RESET_SUBMENU                   "Azzera..."
#define TR_COUNT                           "Punti"
#define TR_PT                              "pt"
#define TR_PTS                             "pti"
#define TR_SMOOTH                          "Smussa"
#define TR_COPY_STICKS_TO_OFS              TR("Cp. stick->subtrim", "Copia Sticks su Subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS         TR("Cp. min/max a tutti",  "Copia min/max/centro a tutte le uscite")
#define TR_COPY_TRIMS_TO_OFS               TR("Cp. trim->subtrim", "Copia Trims su Subtrim")
#define TR_INCDEC                          "Inc/Decrementa"
#define TR_GLOBALVAR                       "Var Globale"
#define TR_MIXSOURCE                       "Sorgente (%)"
#define TR_MIXSOURCERAW                    "Sorgente (valore)"
#define TR_CONSTANT                        "Constante"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1     "Off"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2     "On"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3     "Auto"
#define TR_PREFLIGHT                       "Controlli Prevolo"
#define TR_CHECKLIST                       "Mostra Checklist"
#define TR_CHECKLIST_INTERACTIVE           TR3("Interatt-C", "Interact. checklist", "Checklist interattiva")
#define TR_AUX_SERIAL_MODE                 "Porta Seriale"
#define TR_AUX2_SERIAL_MODE                "Porta Seriale 2"
#define TR_AUX_SERIAL_PORT_POWER           "Potenza porta AUX"
#define TR_SCRIPT                          "Script"
#define TR_INPUTS                          "Ingresso"
#define TR_OUTPUTS                         "Uscita"
#define TR_CONFIRMRESET                    "Resettare TUTTI i dati?"
#define TR_TOO_MANY_LUA_SCRIPTS            "Troppi Scripts Lua!"
#define TR_SPORT_UPDATE_POWER_MODE         "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES_1      "AUTO"
#define TR_SPORT_UPDATE_POWER_MODES_2      "ON"
#define TR_NO_TELEMETRY_SCREENS            "Nessuno Schermo Telemetria"
#define TR_TOUCH_PANEL                     "Schermo touch:"
#define TR_FILE_SIZE                       "Dimensione file"
#define TR_FILE_OPEN                       "Apro comunque?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Nome"
#define TR_PHASES_HEADERS_SW           "Inter"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trim Deriva"
#define TR_PHASES_HEADERS_ELE_TRIM     "Trim Elevatore"
#define TR_PHASES_HEADERS_THT_TRIM     "Trim Motore"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trim Alettoni"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Dissolv. In"
#define TR_PHASES_HEADERS_FAD_OUT      "Dissolv. Out"

#define TR_LIMITS_HEADERS_NAME         "Nome"
#define TR_LIMITS_HEADERS_SUBTRIM      "Offset"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Direzione"
#define TR_LIMITS_HEADERS_CURVE        "Curve"
#define TR_LIMITS_HEADERS_PPMCENTER    "Centro PPM"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Simmetria"
#define TR_INVERTED                    "Invertito"

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "Primo canale"
#define TR_FILL_BACKGROUND           "Riempi lo sfondo?"
#define TR_BG_COLOR                  "Colore sfondo"
#define TR_SLIDERS_TRIMS             "Sliders+Trims"
#define TR_SLIDERS                   "Sliders"
#define TR_FLIGHT_MODE               "Modo di volo"
#define TR_INVALID_FILE              "File invalido"
#define TR_TIMER_SOURCE              "Sorgente timer"
#define TR_SIZE                      "Dimensione"
#define TR_SHADOW                    "Ombra"
#define TR_ALIGNMENT                 "Allineamento"
#define TR_ALIGN_LABEL               "Allinea etichetta"
#define TR_ALIGN_VALUE               "Allinea il valore"
#define TR_ALIGN_OPTS_1              "Sinistra"
#define TR_ALIGN_OPTS_2              "Centro"
#define TR_ALIGN_OPTS_3              "Destra"
#define TR_TEXT                      "Testo"
#define TR_COLOR                     "Colore"
#define TR_PANEL1_BACKGROUND         "Sfondo Panello 1"
#define TR_PANEL2_BACKGROUND         "Sfondo Panello 2"
#define TR_PANEL_BACKGROUND          "Background"
#define TR_PANEL_COLOR               "  Color"
#define TR_WIDGET_GAUGE              "Indicatore"
#define TR_WIDGET_MODELBMP           "Immagine e nome modello"
#define TR_WIDGET_OUTPUTS            "Uscite"
#define TR_WIDGET_TEXT               "Testo"
#define TR_WIDGET_TIMER              "Timer"
#define TR_WIDGET_VALUE              "Valore"

// About screen
#define TR_ABOUTUS                      TR(" INFO ", "INFO SU")

#define TR_CHR_HOUR   'h'
#define TR_CHR_INPUT  'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                  "Volume Beep"
#define TR_WAV_VOLUME                   "Volume Wav"
#define TR_BG_VOLUME                    "Volume Sf"

#define TR_TOP_BAR                      "Barra sup."
#define TR_FLASH_ERASE                  "Cancello flash..."
#define TR_FLASH_WRITE                  "Scrivo flash..."
#define TR_OTA_UPDATE                   "Aggiorn. OTA..."
#define TR_MODULE_RESET                 "Reset modulo..."
#define TR_UNKNOWN_RX                   "RX sconociuta"
#define TR_UNSUPPORTED_RX               "RX non compatibile"
#define TR_OTA_UPDATE_ERROR             "Errore agg. OTA"
#define TR_DEVICE_RESET                 "Resetto dispositivo..."
#define TR_ALTITUDE                     "Altitudine"
#define TR_SCALE                        "Scala"
#define TR_VIEW_CHANNELS                "Vedi Canali"
#define TR_VIEW_NOTES                   "Vedi Note"
#define TR_MODEL_SELECT                 "Seleziona Modello"
#define TR_ID                           "ID"
#define TR_PRECISION                    "Precisione"
#define TR_RATIO                        "Ratio"
#define TR_FORMULA                      "Formula"
#define TR_CELLINDEX                    "Indice cella"
#define TR_LOGS                         "Logs"
#define TR_OPTIONS                      "Opzioni"
#define TR_FIRMWARE_OPTIONS             BUTTON("Opzioni firmware")

#define TR_ALTSENSOR                    "Sensore Alt"
#define TR_CELLSENSOR                   "Sensore Cell"
#define TR_GPSSENSOR                    "Sensore GPS"
#define TR_CURRENTSENSOR                "Sensore"
#define TR_AUTOOFFSET                   "Offset Auto"
#define TR_ONLYPOSITIVE                 "Positivo"
#define TR_FILTER                       "Filtro"
#define TR_TELEMETRYFULL                "Tutti gli slot sono pieni!"
#define TR_IGNORE_INSTANCE              TR("No inst.", "Ignora instanza")
#define TR_SHOW_INSTANCE_ID             "Mostra ID dell'istanza"
#define TR_DISCOVER_SENSORS             "Cerca nuovi sensori"
#define TR_STOP_DISCOVER_SENSORS        "Ferma ricerca"
#define TR_DELETE_ALL_SENSORS           "Elimina tutti i sensori"
#define TR_CONFIRMDELETE                "Confermi " LCDW_128_LINEBREAK "eliminazione?"
#define TR_SELECT_WIDGET                "Seleziona widget"
#define TR_WIDGET_FULLSCREEN            "Schermo intero"
#define TR_REMOVE_WIDGET                "Rimuovi widget"
#define TR_WIDGET_SETTINGS              "Settaggio widget"
#define TR_REMOVE_SCREEN                "Rimuovi schermo"
#define TR_SETUP_WIDGETS                "Setta widgets"
#define TR_THEME                        "Tema"
#define TR_SETUP                        "Imposta"
#define TR_LAYOUT                       "Layout"
#define TR_TEXT_COLOR                   "Colore testo"
#define TR_MENU_INPUTS                  CHAR_INPUT "Ingressi"
#define TR_MENU_LUA                     CHAR_LUA "Lua scripts"
#define TR_MENU_STICKS                  CHAR_STICK "Sticks"
#define TR_MENU_POTS                    CHAR_POT "Potenziometri"
#define TR_MENU_MIN                     CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                     CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                    CHAR_CYC "Ciclico"
#define TR_MENU_TRIMS                   CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES                CHAR_SWITCH "Interrut."
#define TR_MENU_LOGICAL_SWITCHES        CHAR_SWITCH "Interrut. Logici"
#define TR_MENU_TRAINER                 CHAR_TRAINER "Trainer"
#define TR_MENU_CHANNELS                CHAR_CHANNEL "Canali"
#define TR_MENU_GVARS                   CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY               CHAR_TELEMETRY "Telemetria"
#define TR_MENU_DISPLAY                 "DISPLAY"
#define TR_MENU_OTHER                   "Altro"
#define TR_MENU_INVERT                  "Inverti"
#define TR_AUDIO_MUTE                   TR("Audio muto","Muto senza suono")
#define TR_JITTER_FILTER                "Filtro ADC"
#define TR_DEAD_ZONE                    "Zona morta"
#define TR_RTC_CHECK                    TR("Controllo RTC", "Controllo volt. RTC")
#define TR_AUTH_FAILURE                 "Fallimento Auth"
#define TR_RACING_MODE                  "Modo Racing"

#define TR_USE_THEME_COLOR              "Usa colore tema"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Agg. Trim a Subtrim"
#define TR_DUPLICATE                    "Duplica"
#define TR_ACTIVATE                     "Setta Attivo"
#define TR_RED                          "Rosso"
#define TR_BLUE                         "Blu"
#define TR_GREEN                        "Verde"
#define TR_COLOR_PICKER                 "Picker Colori"
#define TR_FIXED                        "Fixed"
#define TR_EDIT_THEME_DETAILS           "Edita dett. tema"
#define TR_THEME_COLOR_DEFAULT          "PREDEFINITO"
#define TR_THEME_COLOR_PRIMARY1         "PRIMARIO1"
#define TR_THEME_COLOR_PRIMARY2         "PRIMARIO2"
#define TR_THEME_COLOR_PRIMARY3         "PRIMARIO3"
#define TR_THEME_COLOR_SECONDARY1       "SECONDARIO1"
#define TR_THEME_COLOR_SECONDARY2       "SECONDARIO2"
#define TR_THEME_COLOR_SECONDARY3       "SECONDARIO3"
#define TR_THEME_COLOR_FOCUS            "FUOCO"
#define TR_THEME_COLOR_EDIT             "MODIFICA"
#define TR_THEME_COLOR_ACTIVE           "ATTIVO"
#define TR_THEME_COLOR_WARNING          "ATTENZIONE"
#define TR_THEME_COLOR_DISABLED         "DISABILITATO"
#define TR_THEME_COLOR_CUSTOM           "PERSONALE"
#define TR_THEME_CHECKBOX               "Casella di spunta"
#define TR_THEME_ACTIVE                 "Attivo"
#define TR_THEME_REGULAR                "Regolare"
#define TR_THEME_WARNING                "Attenzione"
#define TR_THEME_DISABLED               "Disabilitato"
#define TR_THEME_EDIT                   "Modifica"
#define TR_THEME_FOCUS                  "Fuoco"
#define TR_AUTHOR                       "Autore"
#define TR_DESCRIPTION                  "Descrizione"
#define TR_SAVE                         "Salva"
#define TR_CANCEL                       "Elimina"
#define TR_EDIT_THEME                   "EDITA TEMA"
#define TR_DETAILS                      "Dettagli"

// Voice in native language
#define TR_VOICE_ENGLISH                "Inglese"
#define TR_VOICE_CHINESE                "Cinese"
#define TR_VOICE_CZECH                  "Cecoslovacco"
#define TR_VOICE_DANISH                 "Danese"
#define TR_VOICE_DEUTSCH                "Tedesco"
#define TR_VOICE_DUTCH                  "Olandese"
#define TR_VOICE_ESPANOL                "Spagnolo"
#define TR_VOICE_FINNISH                "Finnish"
#define TR_VOICE_FRANCAIS               "Francese"
#define TR_VOICE_HUNGARIAN              "Ungherese"
#define TR_VOICE_ITALIANO               "Italiano"
#define TR_VOICE_POLISH                 "Polacco"
#define TR_VOICE_PORTUGUES              "Portoghese"
#define TR_VOICE_RUSSIAN                "Russo"
#define TR_VOICE_SLOVAK                 "Slovacco"
#define TR_VOICE_SWEDISH                "Svedese"
#define TR_VOICE_TAIWANESE              "Taiwanese"
#define TR_VOICE_JAPANESE               "Giapponese"
#define TR_VOICE_HEBREW                 "Ebraico"
#define TR_VOICE_UKRAINIAN              "Ucraino"
#define TR_VOICE_KOREAN                 "coreano"

#define TR_USBJOYSTICK_LABEL           "Joystick USB"
#define TR_USBJOYSTICK_EXTMODE         "Modo"
#define TR_VUSBJOYSTICK_EXTMODE_1      "Classico"
#define TR_VUSBJOYSTICK_EXTMODE_2      "Avanzato"
#define TR_USBJOYSTICK_SETTINGS        BUTTON("Settaggio canali")
#define TR_USBJOYSTICK_IF_MODE         TR("Modo in.","Modo interfaccia")
#define TR_VUSBJOYSTICK_IF_MODE_1      "Joystick"
#define TR_VUSBJOYSTICK_IF_MODE_2      "Gamepad"
#define TR_VUSBJOYSTICK_IF_MODE_3      "Multi Assi"
#define TR_USBJOYSTICK_CH_MODE         "Modo"
#define TR_VUSBJOYSTICK_CH_MODE_1      "Nessuno"
#define TR_VUSBJOYSTICK_CH_MODE_2      "Btn"
#define TR_VUSBJOYSTICK_CH_MODE_3      "Assi"
#define TR_VUSBJOYSTICK_CH_MODE_4      "Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S_1    "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2    "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3    "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4    "S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Modo pulsante"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1   "Normale"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2   "Impulsi"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3   "SWEmu"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4   "Delta"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5   "Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1 TR("Norm","Normale")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2 TR("Impu","Impulsi")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3 TR("SWEm","SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4 TR("Delt","Delta")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5 TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Posizioni"
#define TR_VUSBJOYSTICK_CH_SWPOS_1     "Pulsante"
#define TR_VUSBJOYSTICK_CH_SWPOS_2     "2POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_3     "3POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_4     "4POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_5     "5POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_6     "6POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_7     "7POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_8     "8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Assi"
#define TR_VUSBJOYSTICK_CH_AXIS_1      "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2      "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3      "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4      "rotX"
#define TR_VUSBJOYSTICK_CH_AXIS_5      "rotY"
#define TR_VUSBJOYSTICK_CH_AXIS_6      "rotZ"
#define TR_VUSBJOYSTICK_CH_AXIS_7      "Slider"
#define TR_VUSBJOYSTICK_CH_AXIS_8      "Dial"
#define TR_VUSBJOYSTICK_CH_AXIS_9      "Wheel"
#define TR_USBJOYSTICK_CH_SIM          "Sim assi"
#define TR_VUSBJOYSTICK_CH_SIM_1       "Ale"
#define TR_VUSBJOYSTICK_CH_SIM_2       "Ele"
#define TR_VUSBJOYSTICK_CH_SIM_3       "Dir"
#define TR_VUSBJOYSTICK_CH_SIM_4       "Mot"
#define TR_VUSBJOYSTICK_CH_SIM_5       "Acc"
#define TR_VUSBJOYSTICK_CH_SIM_6       "Fre"
#define TR_VUSBJOYSTICK_CH_SIM_7       "Sterz"
#define TR_VUSBJOYSTICK_CH_SIM_8       "Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Inversione"
#define TR_USBJOYSTICK_CH_BTNNUM       "Pulsante no."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Collisione pulsante no.!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Collisione assi!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Rit. Cir.", "Ritaglio circolare")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1 "Nessuno"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2 "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3 "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4 "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   BUTTON("Applica modifiche")

#define TR_DIGITAL_SERVO               "Servo 333HZ"
#define TR_ANALOG_SERVO                "Servo 50HZ"
#define TR_SIGNAL_OUTPUT               "Segnale d'uscita"
#define TR_SERIAL_BUS                  "Bus seriale"
#define TR_SYNC                        "Sincronismo"

#define TR_ENABLED_FEATURES            "Funzionalità abilitate"
#define TR_RADIO_MENU_TABS             "Schede del Menu Radio"
#define TR_MODEL_MENU_TABS             "Schede del Menu Modello"

#define TR_SELECT_MENU_ALL        "Tutto"
#define TR_SELECT_MENU_CLR        "Pulisci"
#define TR_SELECT_MENU_INV        "Inverti"

#define TR_SORT_ORDERS_1          "Nome A-Z"
#define TR_SORT_ORDERS_2          "Nome Z-A"
#define TR_SORT_ORDERS_3          "Meno usati"
#define TR_SORT_ORDERS_4          "Più usati"
#define TR_SORT_MODELS_BY         "Ordina modelli per"
#define TR_CREATE_NEW             "Crea"

#define TR_MIX_SLOW_PREC          TR("Prec. lenta", "Prec. su/giù lenta")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "Esiste già una cartella di temi con lo stesso nome."

#define TR_DATE_TIME_WIDGET       "Data & Ora"
#define TR_RADIO_INFO_WIDGET      "Radio Info"
#define TR_LOW_BATT_COLOR         "Batteria scarica"
#define TR_MID_BATT_COLOR         "Batteria media"
#define TR_HIGH_BATT_COLOR        "Batteria carica"

#define TR_WIDGET_SIZE            "Dimensione widget"

#define TR_DEL_DIR_NOT_EMPTY      "La directory non è vuota!"

/*
 * IT translations by: Romolo Manfredini <romolo.manfredini@gmail.com>
 * Update V2.11.x by: Marco Robustini <robustinimarco@gmail.com>
 * Fix V2.11.3 by: MatCron <pancronos@gmail.com> on 4/7/2025
 */

#define TR_KEY_SHORTCUTS          "Key Shortcuts"
#define TR_CURRENT_SCREEN         "Current Screen"
#define TR_SHORT_PRESS            "Short Press"
#define TR_LONG_PRESS             "Long Press"
#define TR_OPEN_QUICK_MENU        "Open Quick Menu"
#define TR_QUICK_MENU_FAVORITES   "Quick Menu Favorites"
