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

// DE translations author: Helmut Renz
// German checked 28.08.2019 r158 opentx V2.3.0 für X12S,X10,X9E,X9D+,X9D,QX7 X9Lite,XLite

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
#define TR_MAIN_MENU_MANAGE_MODELS     "Modell Manager"
#define TR_MAIN_MENU_MODEL_NOTES       "Modell Notizen"
#define TR_MAIN_MENU_CHANNEL_MONITOR   "Kanal Monitor"
#define TR_MONITOR_SWITCHES            "LOGIK SCHALTER MONITOR"
#define TR_MAIN_MENU_MODEL_SETTINGS    "Modell Konfig."
#define TR_MAIN_MENU_RADIO_SETTINGS    "Sender Konfig."
#define TR_MAIN_MENU_SCREEN_SETTINGS    "UI Setup"
#define TR_MAIN_MENU_STATISTICS        "Statistiken"
#define TR_MAIN_MENU_ABOUT_EDGETX      "Über EdgeTX"
#define TR_MAIN_VIEW_X                  "Screen "
#define TR_MAIN_MENU_THEMES                "THEMES"
#define TR_MAIN_MENU_APPS               "Apps"
#define TR_MENUHELISETUP               TR("HELI TS-Mischer", "HELI TS-Mischer CYC1-3")
#define TR_MENUFLIGHTMODES             TR_SFC_AIR("FAHRMODI", "FLUGPHASEN")
#define TR_MENUFLIGHTMODE              TR_SFC_AIR("FAHRMODUS", "FLUGPHASE")
#define TR_MENUINPUTS                  "INPUTS"  //"Inputs=Geber"
#define TR_MENULIMITS                  "SERVOS"  //"AUSGABEN" oder "Servos"
#define TR_MENUCURVES                  "KURVEN"
#define TR_MIXES                       "MISCHER"
#define TR_MENU_GLOBAL_VARS            "Globale Variablen"
#define TR_MENULOGICALSWITCHES         "LOGIKSCHALTER"
#define TR_MENUCUSTOMFUNC              TR("SPEZ.-FUNKTIONEN", "SPEZIAL-FUNKTIONEN")
#define TR_MENUCUSTOMSCRIPTS           "LUA-SCRIPTE"
#define TR_MENUTELEMETRY               "TELEMETRIE"
#define TR_MENUSPECIALFUNCS            "GLOBALE FUNKTIONEN"
#define TR_MENUTRAINER                 TR("LEHRER/SCHÜLER", "LEHRER/SCHÜLER")
#define TR_HARDWARE                    TR("Hardware einst. ", "Namen und Hardware einst.")
#define TR_SD_CARD                     "SD-Karte"
#define TR_DEBUG                       "Testen"
#define TR_MENU_RADIO_SWITCHES         TR("Schalter-Test", "Schalter-Test")
#define TR_MENUCALIBRATION             TR("KALIB. ANALOG", "KALIBRIERUNG-Analog")
#define TR_FUNCTION_SWITCHES           "Anpassbare Schalter"
// End Main menu

#define TR_MINUTE_SINGULAR             "minute"
#define TR_MINUTE_PLURAL1              "minuten"
#define TR_MINUTE_PLURAL2              "minuten"

// NON ZERO TERMINATED STRINGS
#define TR_OFFON_1                     "AUS"
#define TR_OFFON_2                     "EIN"
#define TR_MMMINV_1                    "---"
#define TR_MMMINV_2                    "INV"
#define TR_VBEEPMODE_1                 "Stumm"
#define TR_VBEEPMODE_2                 "Alarm"
#define TR_VBEEPMODE_3                 "NoKey"
#define TR_VBEEPMODE_4                 "Alle"
#define TR_VBLMODE_1                   "AUS"
#define TR_VBLMODE_2                   "Taste"
#define TR_VBLMODE_3                   "Stks"
#define TR_VBLMODE_4                   "Beide"
#define TR_VBLMODE_5                   "EIN"
#define TR_TRNMODE_1                   "AUS"
#define TR_TRNMODE_2                   TR("+=","Addiere")
#define TR_TRNMODE_3                   TR(":=","Ersetze")
#define TR_TRNCHN_1                    "CH1"
#define TR_TRNCHN_2                    "CH2"
#define TR_TRNCHN_3                    "CH3"
#define TR_TRNCHN_4                    "CH4"
#define TR_AUX_SERIAL_MODES_1          "AUS"
#define TR_AUX_SERIAL_MODES_2          "Telem weiterl."
#define TR_AUX_SERIAL_MODES_3          "Telemetrie In"
#define TR_AUX_SERIAL_MODES_4          "SBUS Eingang"
#define TR_AUX_SERIAL_MODES_5          "LUA"
#define TR_AUX_SERIAL_MODES_6          "CLI"
#define TR_AUX_SERIAL_MODES_7          "GPS"
#define TR_AUX_SERIAL_MODES_8          "Debug"
#define TR_AUX_SERIAL_MODES_9          "SpaceMouse"
#define TR_AUX_SERIAL_MODES_10         "Externes Modul"
#define TR_SWTYPES_1                   "Kein"
#define TR_SWTYPES_2                   "Taster"
#define TR_SWTYPES_3                   "2POS"
#define TR_SWTYPES_4                   "3POS"
#define TR_SWTYPES_5                   "Global"
#define TR_POTTYPES_1                  "Kein"
#define TR_POTTYPES_2                  "Poti"
#define TR_POTTYPES_3                  TR("Pot o. Ras","Poti mit Raste")
#define TR_POTTYPES_4                  "Schieber"
#define TR_POTTYPES_5                  TR("Multipos.","Multipos. Schalter")
#define TR_POTTYPES_6                  "Knüppel X"
#define TR_POTTYPES_7                  "Knüppel Y"
#define TR_POTTYPES_8                  "Schalter"
#define TR_VPERSISTENT_1               "AUS"
#define TR_VPERSISTENT_2               "Flugzeit"
#define TR_VPERSISTENT_3               "Manuell Rück"
#define TR_COUNTRY_CODES_1             TR("US","Amerika")
#define TR_COUNTRY_CODES_2             TR("JP","Japan")
#define TR_COUNTRY_CODES_3             TR("EU","Europa")
#define TR_USBMODES_1                  "Fragen"
#define TR_USBMODES_2                  TR("Joyst","Joystick")
#define TR_USBMODES_3                  TR("SDCard","Speicher")
#define TR_USBMODES_4                  TR("Serial","Seriell")
#define TR_JACK_MODES_1                "Popup"
#define TR_JACK_MODES_2                "Audio"
#define TR_JACK_MODES_3                "Trainer"

#define TR_SBUS_INVERSION_VALUES_1     "normal"
#define TR_SBUS_INVERSION_VALUES_2     "nicht inv."
#define TR_MULTI_CUSTOM                "Benutzer"
#define TR_VTRIMINC_1                  TR("Expo","Exponentiell")
#define TR_VTRIMINC_2                  TR("ExFein","Extrafein")
#define TR_VTRIMINC_3                  "Fein"
#define TR_VTRIMINC_4                  "Mittel"
#define TR_VTRIMINC_5                  "Grob"
#define TR_VDISPLAYTRIMS_1             "Nein"
#define TR_VDISPLAYTRIMS_2             "Kurz"
#define TR_VDISPLAYTRIMS_3             "Ja"  // Trimmwerte Keine, kurze Anzeigen, Ja
#define TR_VBEEPCOUNTDOWN_1            "Kein"
#define TR_VBEEPCOUNTDOWN_2            "Pieps"
#define TR_VBEEPCOUNTDOWN_3            "Stimme"
#define TR_VBEEPCOUNTDOWN_4            "Haptik"
#define TR_VBEEPCOUNTDOWN_5            TR("P & H","Pieps & Haptik")
#define TR_VBEEPCOUNTDOWN_6            TR("St & H","Stimme & Haptik")
#define TR_COUNTDOWNVALUES_1           "5s"
#define TR_COUNTDOWNVALUES_2           "10s"
#define TR_COUNTDOWNVALUES_3           "20s"
#define TR_COUNTDOWNVALUES_4           "30s"
#define TR_VVARIOCENTER_1              "Ton"
#define TR_VVARIOCENTER_2              "Ruhe"
#define TR_CURVE_TYPES_1               "Nur Y"
#define TR_CURVE_TYPES_2               "X und Y" // "Standard"
#define TR_CURVE_TYPES_3               "Custom"

#define TR_ADCFILTERVALUES_1           "Global"
#define TR_ADCFILTERVALUES_2           "Aus"
#define TR_ADCFILTERVALUES_3           "Ein"

#define TR_VCURVETYPE_1                "Diff"
#define TR_VCURVETYPE_2                "Expo"
#define TR_VCURVETYPE_3                "Funk"
#define TR_VCURVETYPE_4                "Ind."
#define TR_VMLTPX_1                    "Addiere"
#define TR_VMLTPX_2                    "Multipl."
#define TR_VMLTPX_3                    "Ersetze"

#define TR_CSWTIMER                    "Takt"  // TIM = Takt = Taktgenerator
#define TR_CSWSTICKY                   "SRFF"  // Sticky = RS-Flip-Flop
#define TR_CSWSTAY                     "Puls"  // Edge = einstellbarer Impuls

#define TR_SF_TRAINER                  "Lehrer"
#define TR_SF_INST_TRIM                "Inst. Trim"
#define TR_SF_RESET                    "Rücksetz."
#define TR_SF_SET_TIMER                "Setze"
#define TR_SF_VOLUME                   "Lautstr."
#define TR_SF_FAILSAFE                 "SetFailsafe"
#define TR_SF_RANGE_CHECK              "RangeCheck"
#define TR_SF_MOD_BIND                 "ModuleBind"
#define TR_SF_RGBLEDS                  "RGB LED"

#define TR_SOUND                       "Spiel Töne"
#define TR_PLAY_TRACK                  TR("Ply Trk", "Sag Text")
#define TR_PLAY_VALUE                  "Sag Wert"
#define TR_SF_HAPTIC                   "Haptik"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua Skript")
#define TR_SF_BG_MUSIC                 "StartMusik"
#define TR_SF_BG_MUSIC_PAUSE           "Stop Musik"
#define TR_SF_LOGS                     "SD-Aufz."
#define TR_ADJUST_GVAR                 "Ändere"
#define TR_SF_BACKLIGHT                "LCD Licht"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "Test"
#define TR_SF_SAFETY                   TR("Übersch.", "Überschreibe")

#define TR_SF_SCREENSHOT               "Screenshot"
#define TR_SF_RACING_MODE              "RacingMode"
#define TR_SF_DISABLE_TOUCH            "Kein Touch"
#define TR_SF_DISABLE_AUDIO_AMP        "Audio Verst. Aus"
#define TR_SF_SET_SCREEN               TR_BW_COL("TelSeite anz", "Set Main Screen")
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Telm","Telemetrie")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS_1          "Tmr1"
#define TR_FSW_RESET_TIMERS_2          "Tmr2"
#define TR_FSW_RESET_TIMERS_3          "Tmr3"


#define TR_VFSWRESET_1                  TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                  TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                  TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                  "All"
#define TR_VFSWRESET_5                  TR_FSW_RESET_TELEM
#define TR_VFSWRESET_6                  TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS_1                 TR("Bp1","Piep1")
#define TR_FUNCSOUNDS_2                 TR("Bp2","Piep2")
#define TR_FUNCSOUNDS_3                 TR("Bp3","Piep3")
#define TR_FUNCSOUNDS_4                 TR("Wrn1","Warn1")
#define TR_FUNCSOUNDS_5                 TR("Wrn2","Warn2")
#define TR_FUNCSOUNDS_6                 TR("Chee","Cheep")
#define TR_FUNCSOUNDS_7                 TR("Rata","Ratata")
#define TR_FUNCSOUNDS_8                 "Tick"
#define TR_FUNCSOUNDS_9                 TR("Sirn","Siren")
#define TR_FUNCSOUNDS_10                "Ring"
#define TR_FUNCSOUNDS_11                TR("SciF","SciFi")
#define TR_FUNCSOUNDS_12                TR("Robt","Robot")
#define TR_FUNCSOUNDS_13                TR("Chrp","Chirp")
#define TR_FUNCSOUNDS_14                "Tada"
#define TR_FUNCSOUNDS_15                TR("Crck","Crickt")
#define TR_FUNCSOUNDS_16                TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"
#define TR_VUNITSSYSTEM_1              TR("Metrik","Metrisch")
#define TR_VUNITSSYSTEM_2              TR("Imper.","Imperial")
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

#define TR_VTELEMSCREENTYPE_1          "None"
#define TR_VTELEMSCREENTYPE_2          "Werte"
#define TR_VTELEMSCREENTYPE_3          "Balken"
#define TR_VTELEMSCREENTYPE_4          "Script"
#define TR_GPSFORMAT_1                 "GMS"
#define TR_GPSFORMAT_2                 "NMEA"

#define TR_VSWASHTYPE_1                "---"
#define TR_VSWASHTYPE_2                "120"
#define TR_VSWASHTYPE_3                "120X"
#define TR_VSWASHTYPE_4                "140"
#define TR_VSWASHTYPE_5                "90"

#define TR_STICK_NAMES0                "Sei"
#define TR_STICK_NAMES1                "Höh"
#define TR_STICK_NAMES2                "Gas"
#define TR_STICK_NAMES3                "Que"
#define TR_SURFACE_NAMES0              "Str"
#define TR_SURFACE_NAMES1              "Gas"

#define TR_ON_ONE_SWITCHES_1           "ON"
#define TR_ON_ONE_SWITCHES_2           "One"

#define TR_HATSMODE                    "Joystick Modus"
#define TR_HATSOPT_1                   "nur Trimmer"
#define TR_HATSOPT_2                   "nur Tasten"
#define TR_HATSOPT_3                   "Umschaltbar"
#define TR_HATSOPT_4                   "Global"
#define TR_HATSMODE_TRIMS              "Joystick Modus: Trimmer"
#define TR_HATSMODE_KEYS               "Joystick Modus: Tasten"
#define TR_HATSMODE_KEYS_HELP          "Linke Seite:\n"\
                                       " Rechts = MDL\n"\
                                       " Oben = SYS\n"\
                                       " Unten = TELE\n"\
                                       "\n"\
                                       "Rechte Seite:\n"\
                                       " Links = PAGE<\n"\
                                       " Rechts = PAGE>\n"\
                                       " Oben = PREV/INC\n"\
                                       " Unten = NEXT/DEC"

#define TR_ROTARY_ENC_OPT_1       "Normal"
#define TR_ROTARY_ENC_OPT_2       "Inverted"
#define TR_ROTARY_ENC_OPT_3       "V-I H-N"
#define TR_ROTARY_ENC_OPT_4       "V-I H-A"
#define TR_ROTARY_ENC_OPT_5       "V-N E-I"

#define TR_IMU_VSRCRAW_1             "TltX"
#define TR_IMU_VSRCRAW_2             "TltY"

#define TR_CYC_VSRCRAW_1               "CYC1"
#define TR_CYC_VSRCRAW_2               "CYC2"
#define TR_CYC_VSRCRAW_3               "CYC3"

#define TR_SRC_BATT                    "Batt"
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES_1                 "AUS"
#define TR_VTMRMODES_2                 "EIN"
#define TR_VTMRMODES_3                 "Strt"
#define TR_VTMRMODES_4                 "GSs"
#define TR_VTMRMODES_5                 "GS%"
#define TR_VTMRMODES_6                 "GSt"
#define TR_VTRAINER_MASTER_OFF         "AUS"
#define TR_VTRAINER_MASTER_JACK        "Lehrer/Buchse"
#define TR_VTRAINER_SLAVE_JACK         "Schüler/Buchse"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Lehrer/SBUS Modul"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Lehrer/CPPM Modul"
#define TR_VTRAINER_MASTER_BATTERY     "Lehrer/Serial"
#define TR_VTRAINER_BLUETOOTH_1        TR("Lehrer/BT","Lehrer/Bluetooth")
#define TR_VTRAINER_BLUETOOTH_2        TR("Schüler/BT","Schüler/Bluetooth")
#define TR_VTRAINER_MULTI              "Lehrer/Multi"
#define TR_VTRAINER_CRSF               "Lehrer/CRSF"
#define TR_VFAILSAFE_1                 "Kein Failsafe"
#define TR_VFAILSAFE_2                 "Halte Pos."
#define TR_VFAILSAFE_3                 "Kanäle"
#define TR_VFAILSAFE_4                 "Kein Signal"
#define TR_VFAILSAFE_5                 "Empfänger"
#define TR_VSENSORTYPES_1              "Sensor"
#define TR_VSENSORTYPES_2              "Berechnung"
#define TR_VFORMULAS_1                 "Addieren"
#define TR_VFORMULAS_2                 "Mittelwert"
#define TR_VFORMULAS_3                 "Min"
#define TR_VFORMULAS_4                 "Max"
#define TR_VFORMULAS_5                 "Multiplizier"
#define TR_VFORMULAS_6                 "Gesamt"
#define TR_VFORMULAS_7                 "Zelle"
#define TR_VFORMULAS_8                 "Verbrauch"
#define TR_VFORMULAS_9                 "Distanz"
#define TR_VPREC_1                     "0.--"
#define TR_VPREC_2                     "0.0"
#define TR_VPREC_3                     "0.00"
#define TR_VCELLINDEX_1                "Niedrigst"
#define TR_VCELLINDEX_2                "1. Zelle"
#define TR_VCELLINDEX_3                "2. Zelle"
#define TR_VCELLINDEX_4                "3. Zelle"
#define TR_VCELLINDEX_5                "4. Zelle"
#define TR_VCELLINDEX_6                "5. Zelle"
#define TR_VCELLINDEX_7                "6. Zelle"
#define TR_VCELLINDEX_8                "7. Zelle"
#define TR_VCELLINDEX_9                "8. Zelle"
#define TR_VCELLINDEX_10               "Höchster"
#define TR_VCELLINDEX_11               "Differenz"
#define TR_SUBTRIMMODES_1              CHAR_DELTA" (center only)"
#define TR_SUBTRIMMODES_2              "= (symetrical)"
#define TR_TIMER_DIR_1                 TR("Rückw.", "Rückwärts")
#define TR_TIMER_DIR_2                 TR("Vorwä.", "Vorwärts")

#define TR_FONT_SIZES_1                "STD"
#define TR_FONT_SIZES_2                "FETT"
#define TR_FONT_SIZES_3                "XXS"
#define TR_FONT_SIZES_4                "XS"
#define TR_FONT_SIZES_5                "L"
#define TR_FONT_SIZES_6                "XL"
#define TR_FONT_SIZES_7                "XXL"

#define TR_ENTER                       "[ENTER]"
#define TR_OK                          TR_BW_COL(TR("\010\010\010[OK]", "\010\010\010\010\010[OK]"), "Ok")
#define TR_EXIT                        TR_BW_COL("EXIT", "RTN")

#define TR_YES                         "Ja"
#define TR_NO                          "Nein"
#define TR_DELETEMODEL                 "Modell löschen?"
#define TR_COPYINGMODEL                "Kopiere Modell"
#define TR_MOVINGMODEL                 "Schiebe Modell"
#define TR_LOADINGMODEL                "Lade Modell..."
#define TR_UNLABELEDMODEL              "Unlabeled"
#define TR_NAME                        "Name"
#define TR_MODELNAME                   "Modellname"
#define TR_PHASENAME                   "Phase-Name"
#define TR_MIXNAME                     "Mix-Name"
#define TR_INPUTNAME                   TR("Input", "Inputname")
#define TR_EXPONAME                    TR("Name", "Zeilenname")
#define TR_BITMAP                      "Modellfoto"
#define TR_NO_PICTURE                  "kein Foto"
#define TR_TIMER                       "Timer"
#define TR_NO_TIMERS                   "No timers"
#define TR_START                       "Start"
#define TR_NEXT                        "Next"
#define TR_ELIMITS                     TR("Erw. Limit", "Erw. Wege auf 150%")
#define TR_ETRIMS                      TR("Erw. Trims", "Erw. Trim  auf 100%")
#define TR_TRIMINC                     TR("Trimschritt", "Trimmschritte")
#define TR_DISPLAY_TRIMS               TR("Trimmanzeige", "Trimmwerte anzeigen")
#define TR_TTRACE                      TR("Gasquelle", "Gas-Timerquelle")
#define TR_TTRIM 	       	             TR("Gastrim", "Gas-Leerlauftrim")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", "Trim switch")
#define TR_BEEPCTR                     TR("MittePieps", "Pieps in Mittelstellung")
#define TR_USE_GLOBAL_FUNCS            TR("Glob. Funkt.", "Globale Funkt verw.")
#define TR_PROTOCOL          		       TR("Protok.", "Protokoll")
#define TR_PPMFRAME          	  	     "PPM-Frame"
#define TR_REFRESHRATE             	   TR("Refresh", "Refresh Rate")
#define TR_WARN_BATTVOLTAGE           TR("Ausg. ist VBAT: ", "Warnung: Ausg.pegel ist VBAT: ")
#define TR_WARN_5VOLTS                 "Warnung: Ausgangspegel ist 5 Volt"
#define TR_MS                 		     "ms"
#define TR_SWITCH                      TR("Schalt.", "Schalter")
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
#define TR_SF_SWITCH                   "Trigger"
#define TR_TRIMS                       "Trimmer"
#define TR_FADEIN                      "Langs. Ein"
#define TR_FADEOUT                     "Langs. Aus"
#define TR_DEFAULT                     "(Normal)"
#define   TR_CHECKTRIMS                 TR_BW_COL("\006Prüfe\012Trimmung", "Prüfe Flugphasen-Trimmung")
#define TR_SWASHTYPE                   TR("Typ Taumelsch", "Typ  Taumelscheibe")
#define TR_COLLECTIVE                  TR("Kollekt. Pitch", "Kollekt. Pitch Quelle")
#define TR_AILERON                     "Roll Quelle"
#define TR_ELEVATOR                    "Nick Quelle"
#define TR_SWASHRING                   TR("Ring   Begrenz", "Ring Begrenzung")
#define TR_MODE                        "Modus"
#define TR_LEFT_STICK                  "Links"
#define TR_SUBTYPE                     "Subtype"
#define TR_NOFREEEXPO                  "Expos voll!"
#define TR_NOFREEMIXER                 "Mischer voll!"
#define TR_SOURCE                      "Quelle"
#define TR_WEIGHT                      "Gewicht"
#define TR_SIDE                        "Seite"
#define TR_OFFSET                      "Offset"
#define TR_TRIM                        "Trim"
#define TR_CURVE                       "Kurve"
#define TR_FLMODE                      TR("Phase", "Phasen")
#define TR_MIXWARNING                  "Warnung"
#define TR_OFF                         "AUS"
#define TR_ANTENNA                     "Antenne"
#define TR_NO_INFORMATION              TR("No info", "No information")
#define TR_MULTPX                      "Wirkung"
#define TR_DELAYDOWN                   "Verz. Dn"
#define TR_DELAYUP                     "Verz. Up"
#define TR_SLOWDOWN                    "Langs.Dn"
#define TR_SLOWUP                      "Langs.Up"
#define TR_CV                          "KV"
#define TR_GV                          TR("G", "GV")
#define TR_RANGE                       TR("Bereich", "Variobereich m/s")
#define TR_CENTER                      TR("Mitte", "Variomitte     m/s")
#define TR_ALARM                       "Alarme"
#define TR_BLADES                      TR("Prop", "Prop-Blätter")
#define TR_SCREEN                      "Seite: "
#define TR_SOUND_LABEL                 "Töne"
#define TR_LENGTH                      "Dauer"
#define TR_BEEP_LENGTH                 "Beep-Länge"
#define TR_BEEP_PITCH                  "Beep-Freq +/-"
#define TR_HAPTIC_LABEL                "Haptik"
#define TR_STRENGTH                    "Stärke"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Offset"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "LCD-Kontrast"
#define TR_ALARMS_LABEL                "Alarme"
#define TR_BATTERY_RANGE               TR("Akku Bereich", "Akku Spann. Bereich") // Symbol Akku Ladezustand
#define TR_BATTERYCHARGING             "Lädt..."
#define TR_BATTERYFULL                 "Akku voll"
#define TR_BATTERYNONE                 "None!"
#define TR_BATTERYWARNING              TR("Akku Warnung", "Akkuspannungswarnung")
#define TR_INACTIVITYALARM             TR("Inaktivität", "Inaktivität nach")
#define TR_MEMORYWARNING               "Speicher voll"
#define TR_ALARMWARNING                TR("Alle Töne aus?", "Alle Töne ganz aus?")
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI b. Aussch.", "Prüfe RSSI bei Ausschalten")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Trainer b. Aussch.", "Prüfe Trainer bei Ausschalten")
#define TR_MODEL_STILL_POWERED         "Modell noch aktiv!"
#define TR_TRAINER_STILL_CONNECTED     "Schüler noch verbunden"
#define TR_USB_STILL_CONNECTED         "USB noch verbunden"
#define TR_MODEL_SHUTDOWN              "Herunterfahren?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Drücke [ENTER] zum Bestätigen"
#define TR_THROTTLE_LABEL              "Gas-Kontrolle"
#define TR_THROTTLE_START              "Gas Start"
#define TR_THROTTLEREVERSE             TR("Gas invers", "Vollgas hinten?") //Änderung wg TH9x, Taranis
#define TR_MINUTEBEEP                  TR("Min-Alarm", "Minuten-Alarm")
#define TR_BEEPCOUNTDOWN               "Countdown"
#define TR_PERSISTENT                  TR("Permanent", "Permanent")
#define TR_BACKLIGHT_LABEL             "Bildschirm"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLONBRIGHTNESS              "An-Helligkeit"
#define TR_BLOFFBRIGHTNESS             "Aus-Helligkeit"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR                     "Farbe"
#define TR_SPLASHSCREEN                TR("Startbild Ein", "Startbild Anzeigedauer")
#define TR_PLAY_HELLO                  "Startton abspielen"
#define TR_PWR_ON_DELAY                "PWR EIN Verzög."
#define TR_PWR_OFF_DELAY               "PWR AUS Verzög."
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("Pwr AN/AUS Haptik","Power AN/AUS Haptik")
#define TR_THROTTLE_WARNING            TR("Gasalarm", "Gas Alarm")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Cust-Pos", "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR("Sch. Alarm", "Schalter-Alarm")
#define TR_POTWARNINGSTATE             "Pots & sliders"
#define TR_POTWARNING                  TR("Potiwarnung", "Poti-Warnung")
#define TR_TIMEZONE                    TR("Zeitzone", "GPS-Zeitzone +/-Std")
#define TR_ADJUST_RTC                  TR("GPSzeit setzen", "Uhrzeit per GPS setzen")
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Std.Kanal Folge", "Std. Kanal Reihenfolge")
#define TR_STICKS                      "Knüppel"
#define TR_POTS                        "Potis"
#define TR_SWITCHES                    "Schalter"
#define TR_SWITCHES_DELAY              TR("Sw. Mitte Delay", "Schaltermitte Verzögerung")   //Schalter Mitten verzögern Anpassung
#define TR_SLAVE                       TR("Schüler PPM1-16", "Schüler PPM1-16 als Ausgang")
#define TR_MODESRC                     " Modus\003%  Quelle"
#define TR_MULTIPLIER                  "Multiplik."
#define TR_CAL                         "Kal."
#define TR_VTRIM                       "Trim - +"
#define TR_CALIB_DONE                  "Kalibrierung fertig"
#define TR_MENUTOSTART                 TR_ENTER " Zum START"
#define TR_MENUWHENDONE                TR_ENTER " wenn fertig"
#define TR_AXISDIR                     "AXIS DIR"
#define TR_MENUAXISDIR                 "[ENTER LONG] "  TR_AXISDIR
#define TR_SETMIDPOINT                 TR_BW_COL(TR_SFC_AIR("SCHIEBER AUF MITTE", TR("KNÜPPEL AUF MITTE", "ZENTRIERE KNÜPPEL/SCHIEBER")), "Knüppel/Schieber zentrieren")
#define TR_MOVESTICKSPOTS              TR_BW_COL(TR_SFC_AIR("BEWEGE LENK/GAS/POTI/KNÜPPEL", "BEWEGE KNÜPPEL/SCHIEBER"), "Knüppel/Schieber bewegen")
#define TR_NODATA                      "Keine Daten"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS         	       "Tmix max"
#define TR_FREE_STACK     		       "Freier Stack"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua scripts"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_DURATION_MS             TR("[D]","Dauer(ms): ")
#define TR_INTERVAL_MS             TR("[I]","Intervall(ms): ")
#define TR_MEM_USED_SCRIPT         "Script(B): "
#define TR_MEM_USED_WIDGET         "Widget(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Audio: "
#define TR_GPS_FIX_YES                 "Fix: Yes"
#define TR_GPS_FIX_NO                  "Fix: No"
#define TR_GPS_SATS                    "Sats: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Menü: "
#define TR_TIMER_LABEL                 "Timer"
#define TR_THROTTLE_PERCENT_LABEL      "Gas %"
#define TR_BATT_LABEL                  "Battery"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " für Reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODELL"
#define TR_FM                          TR_SFC_AIR("DM", "FP")
#define TR_EEPROMLOWMEM                "EEPROM voll"
#define TR_THROTTLE_NOT_IDLE           "Gas nicht Null!"
#define TR_ALARMSDISABLED              "Alarme ausgeschaltet"
#define TR_PRESS_ANY_KEY_TO_SKIP	     "Taste drücken"
#define TR_PRESSANYKEY                 "Taste drücken"
#define TR_BADEEPROMDATA               "EEPROM ungültig"
#define TR_BAD_RADIO_DATA              "Fehlende oder fehlerhafte Daten"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            "EEPROM Initialisieren"
#define TR_STORAGE_FORMAT              "Speicher Vorbereiten"
#define TR_EEPROMOVERFLOW              "EEPROM Überlauf"
#define TR_RADIO_SETUP                 TR("SENDER-EINSTELLEN", "SENDER-GRUNDEINSTELLUNGEN")
#define TR_MENUVERSION                 "VERSION"
#define TR_MENU_RADIO_ANALOGS          "Geber-Test"
#define TR_MENU_RADIO_ANALOGS_CALIB    "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENU_FSWITCH                "ANPASSBARE SCHALTER"
#define   TR_TRIMS2OFFSETS              TR_BW_COL("\006Trims => Subtrims", "Trims => Subtrims")
#define TR_CHANNELS2FAILSAFE           "Channels=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Channel=>Failsafe"
#define TR_MENUMODELSEL        		   TR("MODELLE", "MODELL WÄHLEN")
#define TR_MENU_MODEL_SETUP            TR("MODELL-EINSTELLUNG", "MODELL-EINSTELLUNGEN")
#define TR_MENUCURVE                   "KURVE"
#define TR_MENULOGICALSWITCH           "LOGIKSCHALTER"
#define TR_MENUSTAT                    "STAT"
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS            "KANAL+MISCHER MONITOR %d-%d"
#define TR_MONITOR_OUTPUT_DESC         "Kanäle"
#define TR_MONITOR_MIXER_DESC          "Mischer"
#define TR_RECEIVER_NUM                TR("Empf Nr.", "Empfänger Nummer")
#define TR_RECEIVER                    "Empfänger"
#define TR_MULTI_RFTUNE                TR("RF Freq.", "RF Freq. Feintuning")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY             "Telemetry"
#define TR_MULTI_VIDFREQ               TR("Vid. Freq.", "Video Frequenz")
#define TR_RF_POWER                    "RF Power"
#define TR_MULTI_FIXEDID               TR("FesteID", "Feste ID")
#define TR_MULTI_OPTION                TR("Option", "Optionswert")
#define TR_MULTI_AUTOBIND              TR("Bind Ka.","Bindung an Kanal")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Deaktiviere Ch. map")
#define TR_DISABLE_TELEM               TR("No Telem", "Deaktiviere Telem.")
#define TR_MULTI_LOWPOWER              TR("Low power", "reduzierte Leistung")
#define TR_MULTI_LNA_DISABLE           "LNA disable"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("EIN", "Aktiviert")
#define TR_DISABLE_INTERNAL            TR("Deaktiv. int. RF", "Deaktiviere int. RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("!serial mode", "Not in serial mode")
#define TR_MODULE_NO_INPUT             TR("No input", "No serial input")
#define TR_MODULE_NO_TELEMETRY         TR3("Keine Telem.", "Keine MULTI_TELEMETRIE", "Keine Telemetrie (aktiviere MULTI_TELEMETRIE)")
#define TR_MODULE_WAITFORBIND          "Warten auf Bindung"
#define TR_MODULE_BINDING              "Binde"
#define TR_MODULE_UPGRADE_ALERT        TR3("Upg. nötig", "Modul Upgrade nötig", "Modul\nUpgrade nötig")
#define TR_MODULE_UPGRADE              TR("Upg. empf.", "Modul Upgrade empf.")
#define TR_REBIND                      "Neu binden nötig"
#define TR_REG_OK                      "Registration ok"
#define TR_BIND_OK                     "Binden erfolgreich"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem AN"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem AUS"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem AN"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem AUS"
#define TR_PROTOCOL_INVALID            TR("Prot. invalid", "Protokoll ungültig")
#define TR_MODULE_STATUS               TR("Status", "Modul Status")
#define TR_MODULE_SYNC                 TR("Sync", "Proto Sync Status")
#define TR_MULTI_SERVOFREQ             TR("Servo Rate", "Servo Update Rate")
#define TR_MULTI_MAX_THROW             TR("Max. Weg", "Aktiviere Max. Weg")
#define TR_MULTI_RFCHAN                TR("RF Channel", "Wähle RF Kanal")
#define TR_GPS_COORDS_FORMAT           TR("GPS-Koord.", "GPS-Koordinaten-Format")
#define TR_VARIO                       "Variometer"
#define TR_PITCH_AT_ZERO               "Töne sinken"
#define TR_PITCH_AT_MAX                "Töne steigen"
#define TR_REPEAT_AT_ZERO              "Wiederholrate"
#define TR_BATT_CALIB                  TR("AkkuSpgwert", "Akku Kalibrierung")
#define TR_CURRENT_CALIB               "Strom abgl."
#define TR_VOLTAGE                     TR("Spg", "Spannungsquelle")  //9XR-Pro
#define TR_SELECT_MODEL                "Modell auswählen"
#define TR_MANAGE_MODELS               "MODELL MANAGER"
#define TR_MODELS                      "Modelle"
#define TR_SELECT_MODE                 "Wähle Mode"
#define TR_CREATE_MODEL                TR("Neues Modell" , "Neues Modell erstellen")
#define TR_FAVORITE_LABEL              "Favoriten"
#define TR_MODELS_MOVED                "Unbenutzte Modelle werden verschoben nach"
#define TR_NEW_MODEL                   "Neues Modell"
#define TR_INVALID_MODEL               "ungültiges Modell"
#define TR_EDIT_LABELS                 "Label ändern"
#define TR_LABEL_MODEL                 "Label zuordnen"
#define TR_MOVE_UP                     "Verschiebe nach oben"
#define TR_MOVE_DOWN                   "Verschiebe nach unten"
#define TR_ENTER_LABEL                 "Enter Label"
#define TR_LABEL                       "Label"
#define TR_LABELS                      "Labels"
#define TR_CURRENT_MODEL               "aktuell"
#define TR_ACTIVE                      "Aktiv"
#define TR_NEW                         "Neu"
#define TR_NEW_LABEL                   "Neues Label"
#define TR_RENAME_LABEL                "Label umbenennen"
#define TR_DELETE_LABEL                "Label löschen"
#define TR_DUPLICATE_MODEL             "Kopiere Modell"
#define TR_COPY_MODEL                  "Kopiere Modell"
#define TR_MOVE_MODEL                  "Verschiebe Modell"
#define TR_BACKUP_MODEL                "Modell auf SD-Karte"  //9XR-Pro
#define TR_DELETE_MODEL                "Lösche Modell" // TODO merged into DELETEMODEL?
#define TR_RESTORE_MODEL               TR("Modell wiederher.", "Modell wiederherstellen")
#define TR_DELETE_ERROR                "Fehler beim\nLöschen"
#define TR_SDCARD_ERROR                "SD-Kartenfehler"
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "Keine Dateien auf SD!"
#define TR_NO_SDCARD                   "Keine SD-Karte"
#define TR_WAITING_FOR_RX              "Warten auf RX..."
#define TR_WAITING_FOR_TX              "Warten auf TX..."
#define TR_WAITING_FOR_MODULE          TR("Warten Modul", "Warten auf Modul...")
#define TR_NO_TOOLS                    "Keine Tools vorhanden"
#define TR_NORMAL                      "Normal"
#define TR_NOT_INVERTED                "Not inv"
#define TR_NOT_CONNECTED               "!Verbunden"
#define TR_CONNECTED                   "Verbunden"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH ohne Telem.", "16CH ohne Telemetrie")
#define TR_16CH_WITH_TELEMETRY         TR("16CH mit Telem.", "16CH mit Telemetrie")
#define TR_8CH_WITH_TELEMETRY          TR("8CH mit Telem.", "8CH mit Telemetrie")
#define TR_EXT_ANTENNA                 "Ext. Antenne"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Update RX Optionen?"
#define TR_UPDATE_TX_OPTIONS           "Update TX Optionen?"
#define TR_MODULES_RX_VERSION          BUTTON("Modul / RX version")
#define TR_SHOW_MIXER_MONITORS         "Zeige Mischermonitor"
#define TR_MENU_MODULES_RX_VERSION     "MODUL / RX VERSION"
#define TR_MENU_FIRM_OPTIONS           "FIRMWARE OPTIONEN"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Knüppel/Poti/Schieber"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Knüppel/Poti/Schieber"
#define TR_RF_PROTOCOL                 "RF Protokoll"
#define TR_MODULE_OPTIONS              "Modul Optionen"
#define TR_POWER                       "Power"
#define TR_NO_TX_OPTIONS               "keine TX Optionen"
#define TR_RTC_BATT                    "RTC Batt"
#define TR_POWER_METER_EXT             "Power Meter (EXT)"
#define TR_POWER_METER_INT             "Power Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spectrum (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Spectrum (INT)"
#define TR_GHOST_MODULE_CONFIG         "Ghost module config"
#define TR_GPS_MODEL_LOCATOR           "GPS model locator"
#define TR_REFRESH                     "Refresh"
#define TR_SDCARD_FULL                 "SD-Karte voll"
#define TR_SDCARD_FULL_EXT              TR_BW_COL(TR_SDCARD_FULL "\036Logs und " LCDW_128_LINEBREAK "Screenshots deaktiviert", TR_SDCARD_FULL "\nLogs und Screenshots deaktiviert")
#define TR_NEEDS_FILE                  "Datei benötigt"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE        		   "Nicht kompatibel"
#define TR_WARNING                     "WARNUNG"
#define TR_STORAGE_WARNING             "SPEICHER"
#define TR_THROTTLE_UPPERCASE          "GAS"
#define TR_ALARMSWARN                  "ALARM"
#define TR_SWITCHWARN                  "SCHALTER"
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE                "Use for tests only"
#define TR_WRONG_SDCARDVERSION         TR("Erw. Version: ","Erwartete Version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Batterie schwach"
#define TR_WARN_MULTI_LOWPOWER         "reduzierte Leistung"
#define TR_BATTERY                     "AKKU"
#define TR_WRONG_PCBREV                "Falsche PCB erkannt"
#define TR_EMERGENCY_MODE              "NOTFALL MODUS"
#define TR_NO_FAILSAFE                 TR("Failsafe not set", "Failsafe nicht programmiert")
#define TR_KEYSTUCK                    "Taste klemmt"  //Key stuck=Taste klemmt
#define TR_VOLUME                      "Lautstärke"
#define TR_LCD                         "Bildschirm"
#define TR_BRIGHTNESS                  "Helligkeit"
#define TR_CPU_TEMP                    "CPU-Temp.\016>"
#define TR_COPROC                      "CoProz."
#define TR_COPROC_TEMP                 "MB Temp. \016>"
#define TR_TTL_WARNING                 "Warnung: An den TX/RX Pins dürfen 3.3V nicht überschritten werden!"
#define TR_FUNC                        "Funktion"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Dauer"
#define TR_DELAY                       "Verzögerung"
#define TR_NO_SOUNDS_ON_SD             "Keine Töne auf SD"
#define TR_NO_MODELS_ON_SD             "Kein Modelle auf SD"
#define TR_NO_BITMAPS_ON_SD            "Keine Bitmaps auf SD"
#define TR_NO_SCRIPTS_ON_SD            "Keine Skripte auf SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntaxfehler", "Skript Syntaxfehler")
#define TR_SCRIPT_PANIC                "Skript Panik"
#define TR_SCRIPT_KILLED               "Skript beendet"
#define TR_SCRIPT_ERROR                "Unbekannter Fehler"
#define TR_PLAY_FILE                   "Abspielen"
#define TR_DELETE_FILE                 "Löschen"
#define TR_COPY_FILE                   "Kopieren"
#define TR_RENAME_FILE                 "Umbenennen"
#define TR_ASSIGN_BITMAP               "Bitmap zuordnen"
#define TR_ASSIGN_SPLASH               "Als Startbild"
#define TR_EXECUTE_FILE                "Execute"
#define TR_REMOVED                     " gelöscht"
#define TR_SD_INFO                     "Information"
#define TR_NA                          "N/V"	//NV=Nicht Verfügbar  Kurz-Meldung
#define TR_FORMATTING                  "Formatierung..."
#define TR_TEMP_CALIB                  "Temp.  abgl."
#define TR_TIME                        "Uhrzeit:"
#define TR_BAUDRATE                    "Baudrate"
#define TR_CRSF_ARMING_MODE            "Arm via"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_MAXBAUDRATE                 "Max Baud"
#define TR_SAMPLE_MODE                 "Abtastmodus"
#define TR_SAMPLE_MODES_1              "Normal"
#define TR_SAMPLE_MODES_2              "OneBit"
#define TR_LOADING                     "Wird geladen..."
#define TR_DELETE_THEME                "Theme löschen?"
#define TR_SAVE_THEME                  "Theme speichern?"
#define TR_EDIT_COLOR                  "Farbe bearbeiten"
#define TR_NO_THEME_IMAGE              "Kein Theme Bild"
#define TR_BACKLIGHT_TIMER             "Inaktivitäts Timeout"

#define TR_MODEL_QUICK_SELECT        "schnelle Modellauswahl"
#define TR_LABELS_SELECT             "Labelauswahl"
#define TR_LABELS_MATCH              "Labelvergleich"
#define TR_FAV_MATCH                 "Favoriten vergleichen"
#define TR_LABELS_SELECT_MODE_1      "Mehrfachauswahl"
#define TR_LABELS_SELECT_MODE_2      "Einfachauswahl"
#define TR_LABELS_MATCH_MODE_1       "Alle"
#define TR_LABELS_MATCH_MODE_2       "Beliebig"
#define TR_FAV_MATCH_MODE_1          "Muss übereinstimmen"
#define TR_FAV_MATCH_MODE_2          "Alternative Übereinstimmung"

#define TR_SELECT_TEMPLATE_FOLDER      "WÄHLE VORLAGENVERZEICHNIS:"
#define TR_SELECT_TEMPLATE             "WÄHLE MODELLVORLAGE:"
#define TR_NO_TEMPLATES                "Es wurden keine Modellvorlagen in diesem Verzeichnis gefunden"
#define TR_SAVE_TEMPLATE               "Als Vorlage speichern"
#define TR_BLANK_MODEL                 "Leeres Modell"
#define TR_BLANK_MODEL_INFO            "Erzeuge leeres Modell"
#define TR_FILE_EXISTS                 "DATEI EXISTIERT SCHON"
#define TR_ASK_OVERWRITE               "Möchten Sie überschreiben?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Suchen"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Dist addr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Local addr"
#define TR_BLUETOOTH_PIN_CODE          "PIN Code"
#define TR_BLUETOOTH_NODEVICES         "kein Gerät gefunden"
#define TR_BLUETOOTH_SCANNING          "Suche..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#define TR_BLUETOOTH_MODES_1           "---"
#define TR_BLUETOOTH_MODES_2           "Telemetrie"
#define TR_BLUETOOTH_MODES_3           "Trainer"
#define TR_BLUETOOTH_MODES_4           "Aktiviert"
#define TR_SD_INFO_TITLE               "SD-INFO"
#define TR_SD_SPEED                    "Geschw:"
#define TR_SD_SECTORS                  "Sektoren:"
#define TR_SD_SIZE                     "Größe:"
#define TR_TYPE                        "Typ"
#define TR_GVARS                       "GLOBALE V."
#define TR_GLOBAL_VAR                  "Globale Variable"
#define TR_OWN                         "Eigen"
#define TR_DATE                        "Datum:"
#define TR_MONTHS_1                    "Jan"
#define TR_MONTHS_2                    "Feb"
#define TR_MONTHS_3                    "Mar"
#define TR_MONTHS_4                    "Apr"
#define TR_MONTHS_5                    "Mai"
#define TR_MONTHS_6                    "Jun"
#define TR_MONTHS_7                    "Jul"
#define TR_MONTHS_8                    "Aug"
#define TR_MONTHS_9                    "Sep"
#define TR_MONTHS_10                   "Okt"
#define TR_MONTHS_11                   "Nov"
#define TR_MONTHS_12                   "Dez"
#define TR_ROTARY_ENCODER              "Drehg."
#define TR_ROTARY_ENC_MODE             TR("Drehg. Modus","Drehgeber Modus")
#define TR_CHANNELS_MONITOR            "Kanal-Monitor==>"
#define TR_MIXERS_MONITOR              "==>Mischer Monitor"
#define TR_PATH_TOO_LONG               "Pfad zu Lang"
#define TR_VIEW_TEXT                   "Zeige Text"
#define TR_FLASH_DEVICE                TR("Flash Gerät","Flash Gerät")
#define TR_FLASH_BOOTLOADER            TR("Flash bootloader","Flash bootloader")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Flash ext. Gerät","Flash externes Gerät")
#define TR_FLASH_RECEIVER_OTA          "Flash receiver OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash BT module", "Flash Bluetoothmodul")
#define TR_FLASH_POWER_MANAGEMENT_UNIT TR("Flash pwr mngt unit", "Flash power management unit")
#define TR_DEVICE_NO_RESPONSE          TR("Gerät antwortet nicht", "Gerät antwortet nicht")
#define TR_DEVICE_FILE_ERROR           TR("G.-Dateiproblem", "G.-Dateiproblem")
#define TR_DEVICE_DATA_REFUSED         TR("G.-Daten abg.", "G.-Daten abg.")
#define TR_DEVICE_WRONG_REQUEST        TR("G.-Zugriffsfehler", "G.-Zugriffsfehler")
#define TR_DEVICE_FILE_REJECTED        TR("G.-Datei abg.", "G.-Datei abg.")
#define TR_DEVICE_FILE_WRONG_SIG       TR("G.-Datei Sig.", "G.-Datei Sig.")
#define TR_CURRENT_VERSION             TR("Current vers. ", "Current version: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Flash int. XJT","Flash internes XJT-Modul")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Int. Multi", "Flash int. Multimodul")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash ext. mod","Flash ext. Modul")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Ext. Multi", "Flash ext. Multimodul")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flash External ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("FW update Error","Firmware Updatefehler")
#define TR_FIRMWARE_UPDATE_SUCCESS     TR("Flash successful","Update erfolgreich")
#define TR_WRITING                     "Schreibe..."
#define TR_CONFIRM_FORMAT              "Formatieren bestätigen?"
#define TR_INTERNALRF                  "Internes HF-Modul"
#define TR_INTERNAL_MODULE             TR("Int. Modul", "Internes Modul")
#define TR_EXTERNAL_MODULE             TR("Ext. Modul", "Externes Modul")
#define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX upgrade nötig"
#define TR_TELEMETRY_DISABLED          "Deaktiv. Telem."  //more chars doesn't fit on QX7
#define TR_MORE_OPTIONS_AVAILABLE      "mehr Optionen verfügbar"
#define TR_NO_MODULE_INFORMATION       "keine Modul Info"
#define TR_EXTERNALRF                  "Externes HF-Modul"
#define TR_FAILSAFE                    TR("Failsafe", "Failsafe Mode")
#define TR_FAILSAFESET                 "Failsafe setzen"
#define TR_REG_ID                      TR("Reg. ID", "Registration ID")
#define TR_OWNER_ID                    "Owner ID"
#define TR_HOLD                        "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                        "None"
#define TR_NONE_UPPERCASE              "NONE"
#define TR_MENUSENSOR                  "SENSOR"
#define TR_POWERMETER_PEAK             "Peak"
#define TR_POWERMETER_POWER            "Power"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Freq."
#define TR_MENUTOOLS                   "TOOLS"
#define TR_TURN_OFF_RECEIVER           "Empf. ausschalten"
#define TR_STOPPING                    "Stoppe..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
#define TR_MENU_POWER_METER            "POWER METER"
#define TR_SENSOR                      "SENSOR"
#define TR_COUNTRY_CODE                "Landescode"
#define TR_USBMODE                     "USB Modus"
#define TR_JACK_MODE                   "Jack Mode"
#define TR_VOICE_LANGUAGE              "Sprachansagen"
#define TR_UNITS_SYSTEM                "Einheiten"
#define TR_UNITS_PPM                   "PPM Einheiten"
#define TR_EDIT                        "Zeile Editieren"
#define TR_INSERT_BEFORE               "Neue Zeile davor"
#define TR_INSERT_AFTER                "Neue Zeile danach"
#define TR_COPY                        "Zeile kopieren"
#define TR_MOVE                        "Zeile verschieben"
#define TR_PASTE                       "Zeile einfügen"
#define TR_PASTE_AFTER                 "Einfügen danach"
#define TR_PASTE_BEFORE                "Einfügen davor"
#define TR_DELETE                      "Zeile löschen"
#define TR_INSERT                      "Neue Zeile"
#define TR_RESET_FLIGHT                "Reset Flugdaten"
#define TR_RESET_TIMER1                "Reset Timer1"
#define TR_RESET_TIMER2                "Reset Timer2"
#define TR_RESET_TIMER3	               "Reset Timer3"
#define TR_RESET_TELEMETRY             "Reset Telemetrie"
#define TR_STATISTICS                  "Statistik Timer Gas"
#define TR_ABOUT_US                    "Die Programmierer"
#define TR_USB_JOYSTICK                "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB Speicher (SD)"
#define TR_USB_SERIAL                  "USB Seriell (VCP)"
#define TR_SETUP_SCREENS               "Setup Hauptbildschirme"
#define TR_MONITOR_SCREENS             "Monitore Mischer Kanal Logik"
#define TR_AND_SWITCH                  "UND Schalt" // UND mit weiterem Schaltern
#define TR_SF                          "SF" // Spezial Funktionen
#define TR_GF                          "GF" // Globale Funktionen
#define TR_ANADIAGS_CALIB              "analoge Geber Kalibriert"
#define TR_ANADIAGS_FILTRAWDEV         "analoge Geber gefiltert und unbearbeitet mit Abweichungen"
#define TR_ANADIAGS_UNFILTRAW          "analoge Geber ungefiltert und unbearbeitet"
#define TR_ANADIAGS_MINMAX             "Min., Max. und Bereich"
#define TR_ANADIAGS_MOVE               "Move analogs to their extremes!"
#define TR_BYTES                       "Bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd","Binden"))   //9XR-Pro
#define TR_MODULE_UNBIND               BUTTON("Trennen")
#define TR_POWERMETER_ATTN_NEEDED      "Dämpfungsgl. nötig"
#define TR_PXX2_SELECT_RX              "Wähle RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "Wähle Gerät"
#define TR_DISCOVER                    BUTTON("Suche")
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Warte..."
#define TR_RECEIVER_DELETE             "Empfänger löschen?"
#define TR_RECEIVER_RESET              "Empfänger resetten?"
#define TR_SHARE                       "Share"
#define TR_BIND                        "Binden"
#define TR_REGISTER                    BUTTON(TR("Reg", "Register"))
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "Reichweite"))
#define TR_RANGE_TEST                  "Reichweitentest"
#define TR_RECEIVER_OPTIONS            TR("RX OPTIONEN", "RX OPTIONEN")
#define TR_RESET_BTN           		     BUTTON("Reset")
#define TR_KEYS_BTN                	   BUTTON(TR("SW","Schalter"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analoge"))
#define TR_FS_BTN                      BUTTON(TR("AnpSchalt", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_CALIBRATION                 BUTTON("Kalibrieren")
#define TR_SET                   	     BUTTON("Set")
#define TR_TRAINER             		     "Lehrer/Schüler"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM     		     "TX-Antennenproblem!"
#define TR_MODELIDUSED                 "ID benutzt in:"
#define TR_MODELIDUNIQUE               "ID ist eindeutig"
#define TR_MODULE             		     "Modul-Typ"
#define TR_RX_NAME                     "Rx Name"
#define TR_TELEMETRY_TYPE      		     TR("Typ", "Telemetrietyp")
#define TR_TELEMETRY_SENSORS  		     "Sensoren"
#define TR_VALUE               		     "Wert"
#define TR_PERIOD                      "Periode"
#define TR_INTERVAL                    "Intervall"
#define TR_REPEAT                      "Wiederholung"
#define TR_ENABLE                      "Aktivieren"
#define TR_DISABLE                     "Deaktivieren"
#define TR_TOPLCDTIMER        		     "oberer LCD Timer"
#define TR_UNIT                        "Einheit"
#define TR_TELEMETRY_NEWSENSOR         "Sensor hinzufügen ..."
#define TR_CHANNELRANGE                TR("Kanäle", "Ausgangs Kanäle")  //wg 9XR-Pro
#define TR_AFHDS3_RX_FREQ              TR("RX Freq.", "RX Frequenz")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetrie")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Tat.Leis", "tatsäch. Leistung")
#define TR_AFHDS3_POWER_SOURCE         TR("Stromqu.", "Stromquelle")
#define TR_ANTENNACONFIRM1             "Ant. umschalten"
#define TR_ANTENNA_MODES_1             "Intern"
#define TR_ANTENNA_MODES_2             "Frag"
#define TR_ANTENNA_MODES_3             "Modellspezifisch"
#define TR_ANTENNA_MODES_4             "Intern + Extern"
#define TR_ANTENNA_MODES_5             "Extern"
#define TR_USE_INTERNAL_ANTENNA        TR("Use int. antenna", "Use internal antenna")
#define TR_USE_EXTERNAL_ANTENNA        TR("Use ext. antenna", "Use external antenna")
#define TR_ANTENNACONFIRM2     		   TR("Check antenna", "Ist eine externe Antenne installiert?")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Requires non"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Benötigt FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Benötigt EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "certified firmware"
#define TR_LOWALARM                    "1.Warnschwelle"
#define TR_CRITICALALARM               "Kritischer Alarm"
#define TR_DISABLE_ALARM               TR("Alarme AUS", "Telemetrie Alarme AUS")
#define TR_POPUP                       "Popup"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Gerade 0 11 22 33 45"
#define TR_PRESET                      "Preset"
#define TR_MIRROR                      "Spiegeln"
#define TR_CLEAR                       "Löschen"
#define TR_CLEAR_BTN                   BUTTON("Löschen")
#define TR_RESET                       TR("Servowert reset","Servowerte zurücksetzen")
#define TR_RESET_SUBMENU               TR("Reset Werte   ==>", "Reset=>Timer Flug Telem")
#define TR_COUNT                       "Punkte"
#define TR_PT                          "Pt"
#define TR_PTS                         "Pts"
#define TR_SMOOTH                      "Runden"
#define TR_COPY_STICKS_TO_OFS          TR("Kop. Knüppel->Subtrim", "Kopiere Knüppel zu Subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR3("Cpy min/max to all", "Kopiere min/max zu allen" , "Kopiere Limits & Mitte auf alle Kanäle")
#define TR_COPY_TRIMS_TO_OFS           TR3("Copy Trim->Subtrim",  "Kopiere Trimm zu Subtrim" , "Kopiere Trimmposition auf Subtrim")  // "Trim to Subtrim"
#define TR_INCDEC                      "Inc/Decrement"
#define TR_GLOBALVAR                   "Global Var"
#define TR_MIXSOURCE                   "Quelle (%)"
#define TR_MIXSOURCERAW                "Quelle (Wert)"
#define TR_CONSTANT                    "Konstant"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1 "Aus"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2 "Ein"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3 "Auto"
#define TR_PREFLIGHT                   "Vorflug-Checkliste"
#define TR_CHECKLIST                   TR("Checkliste", "Checkliste anzeigen")
#define TR_CHECKLIST_INTERACTIVE       TR3("C-Interaktiv", "Interakt. Checkl.", "Interaktive Checkliste")
#define TR_AUX_SERIAL_MODE             "Serieller Port"
#define TR_AUX2_SERIAL_MODE            "Serieller Port 2"
#define TR_AUX_SERIAL_PORT_POWER       "Versorgung"
#define TR_SCRIPT                      "Lua-Skript"
#define TR_INPUTS                      "Eingaben"
#define TR_OUTPUTS                     "Ausgaben"
#define TR_CONFIRMRESET                TR("Alles löschen? ","ALLE Modelle+Einst. löschen?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Zu viele Skripte!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES_1  "AUTO"
#define TR_SPORT_UPDATE_POWER_MODES_2  "EIN"
#define TR_NO_TELEMETRY_SCREENS        "Keine Telemetrie Seiten"
#define TR_TOUCH_PANEL                 "Touch panel:"
#define TR_FILE_SIZE                   "Dateigröße"
#define TR_FILE_OPEN                   "trotzdem öffnen?"

// Horus and Taranis specific column headers
#define TR_PHASES_HEADERS_NAME         "Name "
#define TR_PHASES_HEADERS_SW           "Schalter"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trim Seite"
#define TR_PHASES_HEADERS_ELE_TRIM     "Trim Höhe"
#define TR_PHASES_HEADERS_THT_TRIM     "Trim Gas"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trim Quer"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Langs Ein"
#define TR_PHASES_HEADERS_FAD_OUT      "Langs Aus"

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "Erster Kanal"
#define TR_FILL_BACKGROUND           "Hintergrund füllen?"
#define TR_BG_COLOR                  "Hintergrundfarbe"
#define TR_SLIDERS_TRIMS             "Schieber+Trim"
#define TR_SLIDERS                   "Schieber"
#define TR_FLIGHT_MODE               "Flugphase"
#define TR_INVALID_FILE              "ungültige Datei"
#define TR_TIMER_SOURCE              "Timer Quelle"
#define TR_SIZE                      "Größe"
#define TR_SHADOW                    "Schatten"
#define TR_ALIGNMENT                 "Ausrichtung"
#define TR_ALIGN_LABEL               "Name ausrichten"
#define TR_ALIGN_VALUE               "Wert ausrichten"
#define TR_ALIGN_OPTS_1              "Links"
#define TR_ALIGN_OPTS_2              "Mitte"
#define TR_ALIGN_OPTS_3              "Rechts"
#define TR_TEXT                      "Text"
#define TR_COLOR                     "Farbe"
#define TR_PANEL1_BACKGROUND         "Panel1 Hintergrund"
#define TR_PANEL2_BACKGROUND         "Panel2 Hintergrund"
#define TR_PANEL_BACKGROUND          "Background"
#define TR_PANEL_COLOR               "  Color"
#define TR_WIDGET_GAUGE              "Pegel"
#define TR_WIDGET_MODELBMP           "Modellinfo"
#define TR_WIDGET_OUTPUTS            "Ausgänge"
#define TR_WIDGET_TEXT               "Text"
#define TR_WIDGET_TIMER              "Timer"
#define TR_WIDGET_VALUE              "Wert"

// Taranis Info Zeile Anzeigen
#define TR_LIMITS_HEADERS_NAME         "Name"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Richtung"
#define TR_LIMITS_HEADERS_CURVE        "Kurve"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Mitte"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Subtrim Modus"
#define TR_INVERTED                    "Invertiert"

//Taranis About screen
#define TR_ABOUTUS                     TR(" Info ", "Info")

#define TR_CHR_HOUR   				   'h' // Stunden
#define TR_CHR_INPUT  				   'I' // Values between A-I will work

#define TR_BEEP_VOLUME                 "Beep-Lautst."
#define TR_WAV_VOLUME                  "Wav-Lautst."
#define TR_BG_VOLUME                   TR("Bgr-Lautst.", "Hintergrund-Lautstärke")

#define TR_TOP_BAR                     "Infozeile"
#define TR_FLASH_ERASE                 "Flash erase..."
#define TR_FLASH_WRITE                 "Flash write..."
#define TR_OTA_UPDATE                  "OTA update..."
#define TR_MODULE_RESET                "Modul reset..."
#define TR_UNKNOWN_RX                  "unbekannter RX"
#define TR_UNSUPPORTED_RX              "nicht unterstützter RX"
#define TR_OTA_UPDATE_ERROR            "OTA Update Fehler"
#define TR_DEVICE_RESET                "Gerät Reset..."
#define TR_ALTITUDE                    "Höhenanzeige"
#define TR_SCALE                       "Skalieren"
#define TR_VIEW_CHANNELS               "Zeige Kanäle"
#define TR_VIEW_NOTES                  "Zeige Notizen"
#define TR_MODEL_SELECT                "Modell auswählen"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Präzision"
#define TR_RATIO                       "Umrechnung"  //Faktor, Mulitplikator, Teiler  0,1 bis 10,0
#define TR_FORMULA                     "Formel"
#define TR_CELLINDEX                   "Zellenindex"
#define TR_LOGS                        "Log Daten"
#define TR_OPTIONS                     "Optionen"
#define TR_FIRMWARE_OPTIONS            BUTTON("Firmwareoptionen")

#define TR_ALTSENSOR                   "Höhen Sensor"
#define TR_CELLSENSOR                  "Zellen Sensor"
#define TR_GPSSENSOR                   "GPS Sensor"
#define TR_CURRENTSENSOR               "Sensor"
#define TR_AUTOOFFSET                  "Auto Offset"
#define TR_ONLYPOSITIVE                "Nur Positiv"
#define TR_FILTER                      "Filter aktiv"
#define TR_TELEMETRYFULL               TR("Telem voll!", "Telemetriezeilen voll!")
#define TR_IGNORE_INSTANCE             TR("No Inst.", "Ignor. Instanzen")
#define TR_SHOW_INSTANCE_ID            "zeige Instanz ID"
#define TR_DISCOVER_SENSORS            "Start Sensorsuche"
#define TR_STOP_DISCOVER_SENSORS       "Stop Sensorsuche"
#define TR_DELETE_ALL_SENSORS          "Lösche alle Sensoren"
#define TR_CONFIRMDELETE               "Wirklich alle " LCDW_128_LINEBREAK "löschen ?"
#define TR_SELECT_WIDGET               "Widget auswählen"  // grafisches Element
#define TR_WIDGET_FULLSCREEN           "Vollbild"
#define TR_REMOVE_WIDGET               "Widget löschen"
#define TR_WIDGET_SETTINGS             "Widget einstellen"
#define TR_REMOVE_SCREEN               "Seite löschen"
#define TR_SETUP_WIDGETS               "Widget einrichten"
#define TR_USER_INTERFACE               "Top Bar"
#define TR_THEME                       "Theme"
#define TR_SETUP                       "Einrichten"
#define TR_LAYOUT                      "Layout"
#define TR_TEXT_COLOR                  "Textfarbe"
// ----------------------------- Symbole für Auswahlliste----------
#define TR_MENU_INPUTS                 CHAR_INPUT "Inputs"
#define TR_MENU_LUA                    CHAR_LUA "Lua Skripte"
#define TR_MENU_STICKS                 CHAR_STICK "Knüppel"
#define TR_MENU_POTS                   CHAR_POT "Potis"
#define TR_MENU_MIN                    CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   CHAR_CYC "Heli-TS CYC1-3"
#define TR_MENU_TRIMS                  CHAR_TRIM "Trimmung"
#define TR_MENU_SWITCHES               CHAR_SWITCH "Schalter"
#define TR_MENU_LOGICAL_SWITCHES       CHAR_SWITCH "Log. Schalter"
#define TR_MENU_TRAINER                CHAR_TRAINER "Trainer"
#define TR_MENU_CHANNELS               CHAR_CHANNEL "Kanäle"
#define TR_MENU_GVARS                  CHAR_SLIDER "Glob. Vars"
#define TR_MENU_TELEMETRY              CHAR_TELEMETRY "Telemetrie"
#define TR_MENU_DISPLAY                "TELM-SEITEN"
#define TR_MENU_OTHER                  "Weitere"
#define TR_MENU_INVERT                 "Invertieren<!>"
#define TR_AUDIO_MUTE                  TR("Ton Stumm","Geräuschunterdrückung")
#define TR_JITTER_FILTER               "ADC Filter"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("RTC Prüfen", "RTC Spann. prüfen")
#define TR_AUTH_FAILURE                "Auth-Fehler"
#define TR_RACING_MODE                 "Racing mode"

#define TR_USE_THEME_COLOR             "Farbe des Themes verwenden"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS   "Alle Trimmungen übernehmen"
#define TR_DUPLICATE                   "Duplizieren"
#define TR_ACTIVATE                    "Aktivieren"
#define TR_RED                         "Rot"
#define TR_BLUE                        "Blau"
#define TR_GREEN                       "Grün"
#define TR_COLOR_PICKER                "Farbauswahl"
#define TR_FIXED                       "Fixed"
#define TR_EDIT_THEME_DETAILS          "Theme Details Bearb."
#define TR_THEME_COLOR_DEFAULT         "Standard"
#define TR_THEME_COLOR_PRIMARY1        "Primär1"
#define TR_THEME_COLOR_PRIMARY2        "Primär2"
#define TR_THEME_COLOR_PRIMARY3        "Primär3"
#define TR_THEME_COLOR_SECONDARY1      "Sekundär1"
#define TR_THEME_COLOR_SECONDARY2      "Sekundär2"
#define TR_THEME_COLOR_SECONDARY3      "Sekundär3"
#define TR_THEME_COLOR_FOCUS           "Fokus"
#define TR_THEME_COLOR_EDIT            "Edit"
#define TR_THEME_COLOR_ACTIVE          "Aktiv"
#define TR_THEME_COLOR_WARNING         "Warnung"
#define TR_THEME_COLOR_DISABLED        "Deaktiviert"
#define TR_THEME_COLOR_CUSTOM          "Eigene"
#define TR_THEME_CHECKBOX              "Schalter"
#define TR_THEME_ACTIVE                "Aktiv"
#define TR_THEME_REGULAR               "Regulär"
#define TR_THEME_WARNING               "Warnung"
#define TR_THEME_DISABLED              "Inaktiv"
#define TR_THEME_EDIT                  "Editieren"
#define TR_THEME_FOCUS                 "Fokus"
#define TR_AUTHOR                      "Author"
#define TR_DESCRIPTION                 "Beschreibung"
#define TR_SAVE                        "Speichern"
#define TR_CANCEL                      "Abbruch"
#define TR_EDIT_THEME                  "THEME Editieren"
#define TR_DETAILS                     "Details"

// Voice in native language
#define TR_VOICE_ENGLISH               "Englisch"
#define TR_VOICE_CHINESE               "Chinesisch"
#define TR_VOICE_CZECH                 "Tschechisch"
#define TR_VOICE_DANISH                "Dänisch"
#define TR_VOICE_DEUTSCH               "Deutsch"
#define TR_VOICE_DUTCH                 "Holländisch"
#define TR_VOICE_ESPANOL               "Spanisch"
#define TR_VOICE_FINNISH               "Finnish"
#define TR_VOICE_FRANCAIS              "Französisch"
#define TR_VOICE_HUNGARIAN             "Ungarisch"
#define TR_VOICE_ITALIANO              "Italienisch"
#define TR_VOICE_POLISH                "Polnisch"
#define TR_VOICE_PORTUGUES             "Portugiesisch"
#define TR_VOICE_RUSSIAN               "Russisch"
#define TR_VOICE_SLOVAK                "Slowenisch"
#define TR_VOICE_SWEDISH               "Schwedisch"
#define TR_VOICE_TAIWANESE             "Taiwanese"
#define TR_VOICE_JAPANESE              "Japanisch"
#define TR_VOICE_HEBREW                "Hebräisch"
#define TR_VOICE_UKRAINIAN             "Ukrainisch"
#define TR_VOICE_KOREAN                "Koreanisch"

#define TR_USBJOYSTICK_LABEL           "USB Joystick"
#define TR_USBJOYSTICK_EXTMODE         "Modus"
#define TR_VUSBJOYSTICK_EXTMODE_1      "Klassisch"
#define TR_VUSBJOYSTICK_EXTMODE_2      "Erweitert"
#define TR_USBJOYSTICK_SETTINGS        BUTTON("Kanal Einstellungen")
#define TR_USBJOYSTICK_IF_MODE         TR("If.Modus","Interface Modus")
#define TR_VUSBJOYSTICK_IF_MODE_1      "Joystick"
#define TR_VUSBJOYSTICK_IF_MODE_2      "Gamepad"
#define TR_VUSBJOYSTICK_IF_MODE_3      "MultiAchsen"
#define TR_USBJOYSTICK_CH_MODE         "Modus"
#define TR_VUSBJOYSTICK_CH_MODE_1      "Kein"
#define TR_VUSBJOYSTICK_CH_MODE_2      "Tasten"
#define TR_VUSBJOYSTICK_CH_MODE_3      "Achse"
#define TR_VUSBJOYSTICK_CH_MODE_4      "Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S_1    "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2    "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3    "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4    "S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Tasten Modus"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1   "Normal"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2   "Puls"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3   "SWEmu"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4   "Delta"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5   "Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1 TR("Norm","Normal")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2 TR("Puls","Puls")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3 TR("SWEm","SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4 TR("Delt","Delta")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5 TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Positionen"
#define TR_VUSBJOYSTICK_CH_SWPOS_1     "Drücken"
#define TR_VUSBJOYSTICK_CH_SWPOS_2     "2POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_3     "3POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_4     "4POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_5     "5POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_6     "6POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_7     "7POS"
#define TR_VUSBJOYSTICK_CH_SWPOS_8     "8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Achse"
#define TR_VUSBJOYSTICK_CH_AXIS_1      "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2      "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3      "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4      "rotX"
#define TR_VUSBJOYSTICK_CH_AXIS_5      "rotY"
#define TR_VUSBJOYSTICK_CH_AXIS_6      "rotZ"
#define TR_VUSBJOYSTICK_CH_AXIS_7      "Schieber"
#define TR_VUSBJOYSTICK_CH_AXIS_8      "Dial"
#define TR_VUSBJOYSTICK_CH_AXIS_9      "Rad"
#define TR_USBJOYSTICK_CH_SIM          "Sim Achse"
#define TR_VUSBJOYSTICK_CH_SIM_1       "Quer"
#define TR_VUSBJOYSTICK_CH_SIM_2       "Höhe"
#define TR_VUSBJOYSTICK_CH_SIM_3       "Seite"
#define TR_VUSBJOYSTICK_CH_SIM_4       "Gas"
#define TR_VUSBJOYSTICK_CH_SIM_5       "Beschl."
#define TR_VUSBJOYSTICK_CH_SIM_6       "Bremsen"
#define TR_VUSBJOYSTICK_CH_SIM_7       "Lenkung"
#define TR_VUSBJOYSTICK_CH_SIM_8       "Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Invers"
#define TR_USBJOYSTICK_CH_BTNNUM       "Tastennr."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Tastennr. Kollision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Achsen Kollision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1 "kein"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2 "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3 "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4 "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   BUTTON("Änd. übernehmen")

#define TR_DIGITAL_SERVO          "Servo333HZ"
#define TR_ANALOG_SERVO           "Servo 50HZ"
#define TR_SIGNAL_OUTPUT          "Signal Ausgang"
#define TR_SERIAL_BUS             "Serialbus"
#define TR_SYNC                   "Sync"

#define TR_ENABLED_FEATURES       "Menüpunkte"
#define TR_RADIO_MENU_TABS        "Sender Menüpunkte"
#define TR_MODEL_MENU_TABS        "Modell Menüpunkte"

#define TR_SELECT_MENU_ALL        "Alle"
#define TR_SELECT_MENU_CLR        "Löschen"
#define TR_SELECT_MENU_INV        "Invertiert"

#define TR_SORT_ORDERS_1          "Name A-Z"
#define TR_SORT_ORDERS_2          "Name Z-A"
#define TR_SORT_ORDERS_3          "Wenig benutzt"
#define TR_SORT_ORDERS_4          "Meist benutzt"
#define TR_SORT_MODELS_BY         "Modelle sortieren nach"
#define TR_CREATE_NEW             "Erstelle"

#define TR_MIX_SLOW_PREC          TR("Langs. Vorlauf", "Langs. Vor-/Rücklauf")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "Ein Theme-Verzeichnis mit demselben Namen existiert bereits"

#define TR_DATE_TIME_WIDGET       "Datum & Uhrzeit"
#define TR_RADIO_INFO_WIDGET      "Fernst. Info"
#define TR_LOW_BATT_COLOR         "Farbe Akku fast leer"
#define TR_MID_BATT_COLOR         "Farbe Akku mittel"
#define TR_HIGH_BATT_COLOR        "Farbe Akku voll"

#define TR_WIDGET_SIZE            "Widget Größe"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"

#define TR_KEY_SHORTCUTS          "Key Shortcuts"
#define TR_CURRENT_SCREEN         "Current Screen"
#define TR_SHORT_PRESS            "Short Press"
#define TR_LONG_PRESS             "Long Press"
#define TR_OPEN_QUICK_MENU        "Open Quick Menu"
#define TR_QUICK_MENU_FAVORITES   "Quick Menu Favorites"
