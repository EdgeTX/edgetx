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

// ES translations authors: Jose Moreno <josemoreno259@gmail.com>, Daniel GeA <daniel.gea.1000@gmail.com>

/* Formatting octal codes available in TR_ strings:
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
#define TR_MAIN_MENU_MANAGE_MODELS      "Model Manager"
#define TR_MAIN_MENU_MODEL_NOTES        "Model Notes"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Channel Monitor"
#define TR_MONITOR_SWITCHES    "MONITOR INTERRP LÓGICOS"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Model Setup"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Radio Setup"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "UI Setup"
#define TR_MAIN_MENU_STATISTICS         "Statistics"
#define TR_MAIN_MENU_ABOUT_EDGETX       "About EdgeTX"
#define TR_MAIN_VIEW_X                  "Screen "
#define TR_MAIN_MENU_THEMES                 "THEMES"
#define TR_MAIN_MENU_APPS               "Apps"
#define TR_MENUHELISETUP       "CONFIGURACIÓN HELI"
#define TR_MENUFLIGHTMODES     TR_SFC_AIR("DRIVE MODES", "MODOS DE VUELO")
#define TR_MENUFLIGHTMODE      TR_SFC_AIR("DRIVE MODE", "MODO DE VUELO")
#define TR_MENUINPUTS          "INPUTS"
#define TR_MENULIMITS          "OUTPUTS"
#define TR_MENUCURVES          "CURVAS"
#define TR_MIXES               "MIXES"
#define TR_MENU_GLOBAL_VARS            "Variables Globales"
#define TR_MENULOGICALSWITCHES TR("INTERRUP. LÓGICOS", "INTERRUPTORES LÓGICOS")
#define TR_MENUCUSTOMFUNC      TR("FUNCIONES", "FUNCIONES ESPECIALES")
#define TR_MENUCUSTOMSCRIPTS   "CUSTOM SCRIPTS"
#define TR_MENUTELEMETRY       "TELEMETRÍA"
#define TR_MENUSPECIALFUNCS    "FUNCIONES GLOBALES"
#define TR_MENUTRAINER         "ENTRENADOR"
#define TR_HARDWARE            "COMPONENTES"
#define TR_USER_INTERFACE               "Top Bar"
#define TR_SD_CARD             "SD CARD"
#define TR_DEBUG                       "Debug"
#define TR_MENU_RADIO_SWITCHES TR("INTERUPTS", "TEST INTERRUPTORES")
#define TR_MENUCALIBRATION     "CALIBRACIÓN"
#define TR_FUNCTION_SWITCHES           "Customizable switches"
// End Main menu

#define TR_MINUTE_SINGULAR     "minute"
#define TR_MINUTE_PLURAL1      "minutes"
#define TR_MINUTE_PLURAL2      "minutes"

#define TR_OFFON_1             "OFF"
#define TR_OFFON_2             "ON"
#define TR_MMMINV_1            "---"
#define TR_MMMINV_2            "INV"
#define TR_VBEEPMODE_1         "Silencio"
#define TR_VBEEPMODE_2         "Alarma"
#define TR_VBEEPMODE_3         "No tecla"
#define TR_VBEEPMODE_4         "Todo"
#define TR_VBLMODE_1           "OFF"
#define TR_VBLMODE_2           "Teclas"
#define TR_VBLMODE_3           "Sticks"
#define TR_VBLMODE_4           "Ambos"
#define TR_VBLMODE_5           "ON"
#define TR_TRNMODE_1           "OFF"
#define TR_TRNMODE_2           TR("+=","Añadir")
#define TR_TRNMODE_3           TR(":=","Cambiar")
#define TR_TRNCHN_1            "CH1"
#define TR_TRNCHN_2            "CH2"
#define TR_TRNCHN_3            "CH3"
#define TR_TRNCHN_4            "CH4"

#define TR_AUX_SERIAL_MODES_1  "UIT"
#define TR_AUX_SERIAL_MODES_2  "Telem Mirror"
#define TR_AUX_SERIAL_MODES_3  "Telemetría"
#define TR_AUX_SERIAL_MODES_4  "Entrenador SBUS"
#define TR_AUX_SERIAL_MODES_5  "LUA"
#define TR_AUX_SERIAL_MODES_6  "CLI"
#define TR_AUX_SERIAL_MODES_7  "GPS"
#define TR_AUX_SERIAL_MODES_8  "Debug"
#define TR_AUX_SERIAL_MODES_9  "SpaceMouse"
#define TR_AUX_SERIAL_MODES_10 "Módulo externo"
#define TR_SWTYPES_1           "Nada"
#define TR_SWTYPES_2           "Palanca"
#define TR_SWTYPES_3           "2POS"
#define TR_SWTYPES_4           "3POS"
#define TR_SWTYPES_5           "Global"
#define TR_POTTYPES_1          "Nada"
#define TR_POTTYPES_2          "Pot"
#define TR_POTTYPES_3          TR("Pot con fij","Pot con fijador")
#define TR_POTTYPES_4          "Slider"
#define TR_POTTYPES_5          TR("Multipos","Switch multipos")
#define TR_POTTYPES_6          "Axis X"
#define TR_POTTYPES_7          "Axis Y"
#define TR_POTTYPES_8          "Switch"
#define TR_VPERSISTENT_1       "OFF"
#define TR_VPERSISTENT_2       "Vuelo"
#define TR_VPERSISTENT_3       "Reset manual"
#define TR_COUNTRY_CODES_1     TR("US","América")
#define TR_COUNTRY_CODES_2     TR("JP","Japón")
#define TR_COUNTRY_CODES_3     TR("EU", "Europa")
#define TR_USBMODES_1          "Pregunta"
#define TR_USBMODES_2          "Joystick"
#define TR_USBMODES_3          "SDCard"
#define TR_USBMODES_4          "Serie"
#define TR_JACK_MODES_1        "Pregunta"
#define TR_JACK_MODES_2        "Audio"
#define TR_JACK_MODES_3        "Entrena."

#define TR_SBUS_INVERSION_VALUES_1     "normal"
#define TR_SBUS_INVERSION_VALUES_2     "not inverted"
#define TR_MULTI_CUSTOM        "Custom"
#define TR_VTRIMINC_1          TR("Expo","Exponencial")
#define TR_VTRIMINC_2          TR("ExFino","Extra fino")
#define TR_VTRIMINC_3          "Fino"
#define TR_VTRIMINC_4          "Medio"
#define TR_VTRIMINC_5          "Grueso"
#define TR_VDISPLAYTRIMS_1     "No"
#define TR_VDISPLAYTRIMS_2     "Cambiar"
#define TR_VDISPLAYTRIMS_3     "Si"
#define TR_VBEEPCOUNTDOWN_1    "Silencio"
#define TR_VBEEPCOUNTDOWN_2    "Beeps"
#define TR_VBEEPCOUNTDOWN_3    "Voz"
#define TR_VBEEPCOUNTDOWN_4    "Haptic"
#define TR_VBEEPCOUNTDOWN_5    TR("B & H","Beeps & Haptic")
#define TR_VBEEPCOUNTDOWN_6    TR("V & H", "Voz & Haptic")
#define TR_COUNTDOWNVALUES_1   "5s"
#define TR_COUNTDOWNVALUES_2   "10s"
#define TR_COUNTDOWNVALUES_3   "20s"
#define TR_COUNTDOWNVALUES_4   "30s"
#define TR_VVARIOCENTER_1      "Tono"
#define TR_VVARIOCENTER_2      "Silencio"
#define TR_CURVE_TYPES_1       "Normal"
#define TR_CURVE_TYPES_2       "Custom"
#define TR_ADCFILTERVALUES_1   "Global"
#define TR_ADCFILTERVALUES_2   "Off"
#define TR_ADCFILTERVALUES_3   "On"

#define TR_VCURVETYPE_1        "Diff"
#define TR_VCURVETYPE_2        "Expo"
#define TR_VCURVETYPE_3        "Func"
#define TR_VCURVETYPE_4        "Cstm"
#define TR_VMLTPX_1            "Añadir"
#define TR_VMLTPX_2            "Multipl."
#define TR_VMLTPX_3            "Cambiar"

#define TR_CSWTIMER            TR("Tim", "Timer")
#define TR_CSWSTICKY           "Pega"
#define TR_CSWSTAY             TR("Bord", "Borde")

#define TR_SF_TRAINER                  "Entrenador"
#define TR_SF_INST_TRIM                "Inst. Trim"
#define TR_SF_RESET                    "Reset"
#define TR_SF_SET_TIMER                "Ajuste"
#define TR_SF_VOLUME                   "Volumen"
#define TR_SF_FAILSAFE                 "Failsafe"
#define TR_SF_RANGE_CHECK              "CheckRango"
#define TR_SF_MOD_BIND                 "Enl.módulo"
#define TR_SF_RGBLEDS                  "RGB leds"

#define TR_SOUND                       "Oir sonido"
#define TR_PLAY_TRACK                  TR("Ply Trk", "Oir pista")
#define TR_PLAY_VALUE                  "Oir valor"
#define TR_SF_HAPTIC                   "Haptic"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua Script")
#define TR_SF_BG_MUSIC                 "BgMúsica"
#define TR_SF_BG_MUSIC_PAUSE           "BgMúsica||"
#define TR_SF_LOGS                     "SD Logs"
#define TR_ADJUST_GVAR                 "Ajuste"
#define TR_SF_BACKLIGHT                "Luz Fondo"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "Test"
#define TR_SF_SAFETY                   "Seguro"

#define TR_SF_SCREENSHOT      "Captura"
#define TR_SF_RACING_MODE     "RacingMode"
#define TR_SF_DISABLE_TOUCH   "No Touch"
#define TR_SF_DISABLE_AUDIO_AMP        "Audio Amp Off"
#define TR_SF_SET_SCREEN      TR_BW_COL("Ajus. pantalla", "Set Main Screen")
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM     TR("Telm", "Telemetría")
#define TR_FSW_RESET_TRIMS     "Trims"
#define TR_FSW_RESET_TIMERS_1  "Tmr1"
#define TR_FSW_RESET_TIMERS_2  "Tmr2"
#define TR_FSW_RESET_TIMERS_3  "Tmr3"

#define TR_VFSWRESET_1         TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2         TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3         TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4         TR("Todo","Vuelo")
#define TR_VFSWRESET_5         TR_FSW_RESET_TELEM
#define TR_VFSWRESET_6         TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS_1        TR("Bp1","Beep1")
#define TR_FUNCSOUNDS_2        TR("Bp2","Beep2")
#define TR_FUNCSOUNDS_3        TR("Bp3","Beep3")
#define TR_FUNCSOUNDS_4        TR("Avs1","Aviso1")
#define TR_FUNCSOUNDS_5        TR("Avs2","Aviso2")
#define TR_FUNCSOUNDS_6        TR("Chee","Cheep")
#define TR_FUNCSOUNDS_7        TR("Rata","Ratata")
#define TR_FUNCSOUNDS_8        "Tick"
#define TR_FUNCSOUNDS_9        TR("Sirn","Siren")
#define TR_FUNCSOUNDS_10       "Ring"
#define TR_FUNCSOUNDS_11       TR("SciF","SciFi")
#define TR_FUNCSOUNDS_12       TR("Robt","Robot")
#define TR_FUNCSOUNDS_13       TR("Chrp","Chirp")
#define TR_FUNCSOUNDS_14       "Tada"
#define TR_FUNCSOUNDS_15       TR("Crck","Crickt")
#define TR_FUNCSOUNDS_16       TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP        "ft"
#define SPEED_UNIT_IMP         "mph"
#define LENGTH_UNIT_METR       "m"
#define SPEED_UNIT_METR        "kmh"

#define TR_VUNITSSYSTEM_1      "Métrico"
#define TR_VUNITSSYSTEM_2      "Imperial"
#define TR_VTELEMUNIT_1        "-"
#define TR_VTELEMUNIT_2        "V"
#define TR_VTELEMUNIT_3        "A"
#define TR_VTELEMUNIT_4        "mA"
#define TR_VTELEMUNIT_5        "kts"
#define TR_VTELEMUNIT_6        "m/s"
#define TR_VTELEMUNIT_7        "f/s"
#define TR_VTELEMUNIT_8        "kmh"
#define TR_VTELEMUNIT_9        "mph"
#define TR_VTELEMUNIT_10       "m"
#define TR_VTELEMUNIT_11       "ft"
#define TR_VTELEMUNIT_12       "°C"
#define TR_VTELEMUNIT_13       "°F"
#define TR_VTELEMUNIT_14       "%"
#define TR_VTELEMUNIT_15       "mAh"
#define TR_VTELEMUNIT_16       "W"
#define TR_VTELEMUNIT_17       "mW"
#define TR_VTELEMUNIT_18       "dB"
#define TR_VTELEMUNIT_19       "rpm"
#define TR_VTELEMUNIT_20       "g"
#define TR_VTELEMUNIT_21       "°"
#define TR_VTELEMUNIT_22       "rad"
#define TR_VTELEMUNIT_23       "ml"
#define TR_VTELEMUNIT_24       "fOz"
#define TR_VTELEMUNIT_25       "mlm"
#define TR_VTELEMUNIT_26       "Hz"
#define TR_VTELEMUNIT_27       "ms"
#define TR_VTELEMUNIT_28       "us"
#define TR_VTELEMUNIT_29       "km"
#define TR_VTELEMUNIT_30       "dBm"

#define TR_VTELEMSCREENTYPE_1  "Nada"
#define TR_VTELEMSCREENTYPE_2  "Números"
#define TR_VTELEMSCREENTYPE_3  "Barras"
#define TR_VTELEMSCREENTYPE_4  "Script"
#define TR_GPSFORMAT_1         "HMS"
#define TR_GPSFORMAT_2         "NMEA"

#define TR_VSWASHTYPE_1        "---"
#define TR_VSWASHTYPE_2        "120"
#define TR_VSWASHTYPE_3        "120X"
#define TR_VSWASHTYPE_4        "140"
#define TR_VSWASHTYPE_5        "90"

#define TR_STICK_NAMES0                "Rud"
#define TR_STICK_NAMES1                "Ele"
#define TR_STICK_NAMES2                "Thr"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#define TR_ON_ONE_SWITCHES_1   "ON"
#define TR_ON_ONE_SWITCHES_2   "One"

#define TR_HATSMODE                    "Hats mode"
#define TR_HATSOPT_1                   "Trims only"
#define TR_HATSOPT_2                   "Keys only"
#define TR_HATSOPT_3                   "Switchable"
#define TR_HATSOPT_4                   "Global"
#define TR_HATSMODE_TRIMS              "Hats mode: Trims"
#define TR_HATSMODE_KEYS               "Hats mode: Keys"
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

#define TR_ROTARY_ENC_OPT_1       "Normal"
#define TR_ROTARY_ENC_OPT_2       "Inverted"
#define TR_ROTARY_ENC_OPT_3       "V-I H-N"
#define TR_ROTARY_ENC_OPT_4       "V-I H-A"
#define TR_ROTARY_ENC_OPT_5       "V-N E-I"

#define TR_IMU_VSRCRAW_1             "TltX"
#define TR_IMU_VSRCRAW_2             "TltY"

#define TR_CYC_VSRCRAW_1     "CYC1"
#define TR_CYC_VSRCRAW_2     "CYC2"
#define TR_CYC_VSRCRAW_3     "CYC3"

#define TR_SRC_BATT                    "Batt"
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES_1                 "OFF"
#define TR_VTMRMODES_2                 "ABS"
#define TR_VTMRMODES_3                 "Strt"
#define TR_VTMRMODES_4                 "THs"
#define TR_VTMRMODES_5                 "TH%"
#define TR_VTMRMODES_6                 "THt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Master/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Esclav/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Master/Módulo SBUS"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Master/Módulo CPPM"
#define TR_VTRAINER_MASTER_BATTERY     "Master/Serie"
#define TR_VTRAINER_BLUETOOTH_1        TR("Master/BT","Master/Bluetooth")
#define TR_VTRAINER_BLUETOOTH_2        TR("Esclavo/BT","Esclavo/Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VTRAINER_CRSF               "Master/CRSF"
#define TR_VFAILSAFE_1                 "No"
#define TR_VFAILSAFE_2                 "Hold"
#define TR_VFAILSAFE_3                 "Custom"
#define TR_VFAILSAFE_4                 "No pulsos"
#define TR_VFAILSAFE_5                 "Receptor"
#define TR_VSENSORTYPES_1              "Custom"
#define TR_VSENSORTYPES_2              "Calculado"
#define TR_VFORMULAS_1                 "Suma"
#define TR_VFORMULAS_2                 "Media"
#define TR_VFORMULAS_3                 "Min"
#define TR_VFORMULAS_4                 "Máx"
#define TR_VFORMULAS_5                 "Multipl."
#define TR_VFORMULAS_6                 "Total"
#define TR_VFORMULAS_7                 "Cell"
#define TR_VFORMULAS_8                 "Consumo"
#define TR_VFORMULAS_9                 "Distancia"
#define TR_VPREC_1                     "0.--"
#define TR_VPREC_2                     "0.0 "
#define TR_VPREC_3                     "0.00"
#define TR_VCELLINDEX_1                "Menor"
#define TR_VCELLINDEX_2                "1"
#define TR_VCELLINDEX_3                "2"
#define TR_VCELLINDEX_4                "3"
#define TR_VCELLINDEX_5                "4"
#define TR_VCELLINDEX_6                "5"
#define TR_VCELLINDEX_7                "6"
#define TR_VCELLINDEX_8                "7"
#define TR_VCELLINDEX_9                "8"
#define TR_VCELLINDEX_10               "Mayor"
#define TR_VCELLINDEX_11               "Delta"
#define TR_SUBTRIMMODES_1              CHAR_DELTA " (center only)"
#define TR_SUBTRIMMODES_2              "= (symetrical)"
#define TR_TIMER_DIR_1                 TR("Remain", "Show Remain")
#define TR_TIMER_DIR_2                 TR("Elaps.", "Show Elapsed")

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

#define TR_YES                         "Yes"
#define TR_NO                          "No"
#define TR_DELETEMODEL         "BORRAR MODELO"
#define TR_COPYINGMODEL        "Copiando modelo.."
#define TR_MOVINGMODEL         "Moviendo modelo..."
#define TR_LOADINGMODEL        "Cargando modelo..."
#define TR_UNLABELEDMODEL              "Unlabeled"
#define TR_NAME                "Nombre"
#define TR_MODELNAME           TR("Nom. modelo", "Nombre modelo")
#define TR_PHASENAME           "Nombre fase "
#define TR_MIXNAME             TR("Nom. mezcla", "Nombre mezcla")
#define TR_INPUTNAME           TR("Entrada", "Nom. entrada")
#define TR_EXPONAME            TR("Nom.", "Nombre expo")
#define TR_BITMAP              "Imagen modelo"
#define TR_NO_PICTURE                  "No Picture"
#define TR_TIMER               TR("Timer", "Timer ")
#define TR_NO_TIMERS                   "No timers"
#define TR_START                       "Start"
#define TR_NEXT                        "Next"
#define TR_ELIMITS             TR("E.Límite", "Ampliar límites")
#define TR_ETRIMS              TR("E.Trims", "Ampliar trims")
#define TR_TRIMINC             "Paso trim"
#define TR_DISPLAY_TRIMS       "Ver trims"
#define TR_TTRACE              TR("Fuente-A", "Fuente acelerador")
#define TR_TTRIM               TR("Trim-A", "Trim acelerador")
#define TR_TTRIM_SW            TR("T-Trim-Sw", "Trim switch")
#define TR_BEEPCTR             TR("Beep ctr", "Beep centro")
#define TR_USE_GLOBAL_FUNCS    TR("Funcs. glob.", "Usar func. globales")
#define TR_PROTOCOL            TR("Proto", "Protocolo")
#define TR_PPMFRAME            "Trama PPM"
#define TR_REFRESHRATE         TR("Refresco", "Velocidad refresco")
#define TR_WARN_BATTVOLTAGE   TR("Salida es VBAT: ", "Aviso: señal salida es VBAT: ")
#define TR_WARN_5VOLTS                 "Aviso: nivel de salida 5 voltios"
#define TR_MS                  "ms"
#define TR_SWITCH              TR("Interr.", "Interruptor")
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
#define TR_SF_SWITCH           "Trigger"
#define TR_TRIMS               "Trims"
#define TR_FADEIN              "Inicio"
#define TR_FADEOUT             "Final"
#define TR_DEFAULT             "(defecto)"
#define   TR_CHECKTRIMS         TR_BW_COL("\006Check\012trims", "Check FM Trims")
#define TR_SWASHTYPE           "Tipo cíclico"
#define TR_COLLECTIVE          TR("Colectivo", "Fuente colectivo")
#define TR_AILERON             TR("Col. lateral", "Fuente col. lateral")
#define TR_ELEVATOR            TR("Col. long. ", "Fuente col. longitudinal")
#define TR_SWASHRING           "Ciclico"
#define TR_MODE                "Modo"
#define TR_LEFT_STICK          "Izquierdo"
#define TR_SUBTYPE             "Subtipo"
#define TR_NOFREEEXPO          "No expo libre!"
#define TR_NOFREEMIXER         "No mezcla lib!"
#define TR_SOURCE              "Fuente"
#define TR_WEIGHT              "Cantidad"
#define TR_SIDE                "Zona"
#define TR_OFFSET              "Offset"
#define TR_TRIM                "Trim"
#define TR_CURVE               "Curva"
#define TR_FLMODE              TR("Modo", "Modos")
#define TR_MIXWARNING          "Aviso"
#define TR_OFF                 "OFF"
#define TR_ANTENNA             "Antena"
#define TR_NO_INFORMATION      TR("Sin info", "Sin información")
#define TR_MULTPX              "Multipx"
#define TR_DELAYDOWN           "Atraso bajar"
#define TR_DELAYUP             "Atraso subir"
#define TR_SLOWDOWN            "Bajar lento "
#define TR_SLOWUP              "Subir lento"
#define TR_CV                  "CV"
#define TR_GV                  TR("G", "GV")
#define TR_RANGE               "Alcance"
#define TR_CENTER              "Centro"
#define TR_ALARM               "Alarma"
#define TR_BLADES              "Palas"
#define TR_SCREEN              "Pant. "
#define TR_SOUND_LABEL         "Sonido"
#define TR_LENGTH              "Longitud"
#define TR_BEEP_LENGTH         "Duración Beep"
#define TR_BEEP_PITCH           "Tono"
#define TR_HAPTIC_LABEL        "Haptic"
#define TR_STRENGTH            "Intensidad"
#define TR_IMU_LABEL           "IMU"
#define TR_IMU_OFFSET          "Offset"
#define TR_IMU_MAX             "Máx"
#define TR_CONTRAST            "Contraste"
#define TR_ALARMS_LABEL        "Alarmas"
#define TR_BATTERY_RANGE       TR("Rango batería", "Rango medidor batería")
#define TR_BATTERYCHARGING     "Charging..."
#define TR_BATTERYFULL         "Battery full"
#define TR_BATTERYNONE         "None!"
#define TR_BATTERYWARNING      "Batería baja"
#define TR_INACTIVITYALARM     "Inactividad"
#define TR_MEMORYWARNING       "Memoria baja"
#define TR_ALARMWARNING        "Sin sonido"
#define TR_RSSI_SHUTDOWN_ALARM   TR("Apagado Rssi", "Check Rssi al apagar")
#define TR_TRAINER_SHUTDOWN_ALARM          TR("Trainer alarm", "check trainer connection")
#define TR_MODEL_STILL_POWERED "Modelo aun encendido"
#define TR_TRAINER_STILL_CONNECTED     "Trainer still connected"
#define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_MODEL_SHUTDOWN              "Apagar ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Enter para confirmar"
#define TR_THROTTLE_LABEL      "Potencia"
#define TR_THROTTLE_START      "Potencia Start"
#define TR_THROTTLEREVERSE     TR("Invert acel", "Invertir acel.")
#define TR_MINUTEBEEP          TR("Minuto", "Cada minuto")
#define TR_BEEPCOUNTDOWN       TR("Cta. atrás", "Cuenta atrás")
#define TR_PERSISTENT          TR("Persisten.", "Persistente")
#define TR_BACKLIGHT_LABEL     "Luz fondo"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLONBRIGHTNESS      "MAS brillo"
#define TR_BLOFFBRIGHTNESS     "MENOS brillo"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR             "Color"
#define TR_SPLASHSCREEN        "Pantalla inicio"
#define TR_PLAY_HELLO                  "Startup Sound"
#define TR_PWR_ON_DELAY        TR("Atraso On", "Atraso encendido")
#define TR_PWR_OFF_DELAY       TR("Atraso Off", "Atraso apagado")
#define TR_PWR_AUTO_OFF        TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("Pwr ON/OFF Haptic","Power ON/OFF Haptic")
#define TR_THROTTLE_WARNING    TR("Aviso-A", "Aviso acelerador")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Cust-Pos", "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING       TR("Aviso-I", "Aviso interruptor")
#define TR_POTWARNINGSTATE     "Pots & sliders"
#define TR_POTWARNING          TR("Aviso pot", "Aviso pot")
#define TR_TIMEZONE            "Zona horaria"
#define TR_ADJUST_RTC          TR("Ajustar RTC", "Ajustar RTC")
#define TR_GPS                 "GPS"
#define TR_DEF_CHAN_ORD        TR("Rx Orden canal", "Orden habitual canales")
#define TR_STICKS              "Sticks"
#define TR_POTS                "Pots"
#define TR_SWITCHES            "Switches"
#define TR_SWITCHES_DELAY      TR("Atraso switch.", "Atraso switches")
#define TR_SLAVE               "Esclavo"
#define TR_MODESRC             "Modo\006% Fuente"
#define TR_MULTIPLIER          "Multiplicar"
#define TR_CAL                 "Cal"
#define TR_VTRIM               "Trim - +"
#define TR_CALIB_DONE          "Calibration completed"
#define TR_MENUTOSTART         TR_ENTER " EMPEZAR"
#define TR_MENUWHENDONE        TR_ENTER " AL ACABAR "
#define TR_AXISDIR             "AXIS DIR"
#define TR_MENUAXISDIR         "[ENTER LONG] "  TR_AXISDIR
#define TR_SETMIDPOINT         TR_BW_COL(TR_SFC_AIR("SET POTS MIDPOINT", TR("STICKS AL CENTRO", "STICKS AL CENTRO")), "Centra sticks/pots/sliders")
#define TR_MOVESTICKSPOTS      TR_BW_COL(TR_SFC_AIR("MOVE ST/TH/POTS/AXIS", "MOVER STICKS/POTS"), "Mueve sticks, pots and sliders")
#define TR_NODATA              "SIN DATOS"
#define TR_US                         "us"
#define TR_HZ                         "Hz"
#define TR_TMIXMAXMS                  "Tmix máx"
#define TR_FREE_STACK                 "Stack libre"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL          "Lua scripts"
#define TR_FREE_MEM_LABEL             "Free mem"
#define TR_DURATION_MS             TR("[D]","Duration(ms): ")
#define TR_INTERVAL_MS             TR("[I]","Interval(ms): ")
#define TR_MEM_USED_SCRIPT         "Script(B): "
#define TR_MEM_USED_WIDGET         "Widget(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Audio: "
#define TR_GPS_FIX_YES                 "Fix: Yes"
#define TR_GPS_FIX_NO                  "Fix: No"
#define TR_GPS_SATS                    "Sats: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Menu: "
#define TR_TIMER_LABEL                "Timer"
#define TR_THROTTLE_PERCENT_LABEL     "Throttle %"
#define TR_BATT_LABEL                 "Battery"
#define TR_SESSION                    "Session"
#define TR_MENUTORESET         TR_ENTER "Resetear"
#define TR_PPM_TRAINER         "TR"
#define TR_CH                  "CH"
#define TR_MODEL               "MODELO"
#define TR_FM                  TR_SFC_AIR("DM", "FM")
#define TR_EEPROMLOWMEM        "EEPROM mem.baja"
#define TR_PRESS_ANY_KEY_TO_SKIP   "Pulsar tecla para omitir"
#define TR_THROTTLE_NOT_IDLE     "Aceler. Activado"
#define TR_ALARMSDISABLED      "Alarmas Desact."
#define TR_PRESSANYKEY         TR("\010Pulsa una tecla", "Pulsa una tecla")
#define TR_BADEEPROMDATA       "Error datos EEPROM"
#define TR_BAD_RADIO_DATA      "Error datos radio"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING    "Formateo EEPROM"
#define TR_STORAGE_FORMAT      "Preparación alamacenamiento"
#define TR_EEPROMOVERFLOW      "Desborde EEPROM"
#define TR_RADIO_SETUP      "CONFIGURACIÓN"
#define TR_MENUVERSION         "VERSIÓN"
#define TR_MENU_RADIO_ANALOGS  TR("ANALÓGICOS", "TEST ANALÓGICOS")
#define TR_MENU_RADIO_ANALOGS_CALIB    "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENU_FSWITCH        "CUSTOMIZABLE SWITCHES"
#define   TR_TRIMS2OFFSETS        TR_BW_COL("\006Trims => Offsets", "Trims => Offsets")
#define TR_CHANNELS2FAILSAFE   "Canales=>Failsafe"
#define TR_CHANNEL2FAILSAFE    "Canal=>Failsafe"
#define TR_MENUMODELSEL        TR("MODELOS", "SELECCIÓN MODELO")
#define TR_MENU_MODEL_SETUP            TR("SETUP", "MODEL SETUP")

// Alignment

#define TR_MENUCURVE           "CURVA"
#define TR_MENULOGICALSWITCH   "INTERRUP.LÓGICO"
#define TR_MENUSTAT            TR("ESTAD.", "ESTADÍSTICAS")
#define TR_MENUDEBUG           "DEBUG"
#define TR_MONITOR_CHANNELS    "MONITOR CANALES %d/%d"
#define TR_MONITOR_OUTPUT_DESC "SALIDAS"
#define TR_MONITOR_MIXER_DESC          "Mixers"
#define TR_RECEIVER_NUM                TR("Num Rx", "Receptor No.")
#define TR_RECEIVER                    "Receptor"
#define TR_MULTI_RFTUNE                TR("Sint.freq.", "RF Freq. sint.fina")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY             "Telemetría"
#define TR_MULTI_VIDFREQ               TR("Freq.vídeo", "Frecuencia vídeo")
#define TR_RF_POWER                     "RF Power"
#define TR_MULTI_FIXEDID               TR("ID Fijo", "ID Fijo")
#define TR_MULTI_OPTION                TR("Opción", "Valor opción")
#define TR_MULTI_AUTOBIND              TR("Emp Cnl","Emparejar en canal")
#define TR_DISABLE_CH_MAP              TR("No ch map", "Desactivar mapa cnl")
#define TR_DISABLE_TELEM               TR("No telem", "Desactivar telem.")
#define TR_MULTI_LOWPOWER              TR("Baja poten.", "Modo de baja poten.")
#define TR_MULTI_LNA_DISABLE           "LNA desact."
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("ON", "Activado")
#define TR_DISABLE_INTERNAL            TR("Desac.RF int", "Desact. mód. int. RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("No modo serie", "No en modo serie")
#define TR_MODULE_NO_INPUT             "Sin señal"
#define TR_MODULE_NO_TELEMETRY         TR3( "No telemetría", "No MULTI_TELEMETRY", "No telemetría (activa MULTI_TELEMETRY)")
#define TR_MODULE_WAITFORBIND          "Emparejar con protocolo"
#define TR_MODULE_BINDING              "Emparejando"
#define TR_MODULE_UPGRADE_ALERT        TR3("Actualizar", "Actualizar módulo", "Actualizar\nmódulo")
#define TR_MODULE_UPGRADE              TR("Act recom", "Actualizar módulo recomendado")
#define TR_REBIND                      "Reemparejar "
#define TR_REG_OK                      "Registro ok"
#define TR_BIND_OK                     "Emparejado ok"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem OFF"
#define TR_PROTOCOL_INVALID            TR("Prot. inválido", "Protocolo inválido")
#define TR_MODULE_STATUS               TR("Estado", "Estado módulo")
#define TR_MODULE_SYNC                 TR("Sync", "Estado proto sync")
#define TR_MULTI_SERVOFREQ             TR("V ref serv", "Vel. refr. servo")
#define TR_MULTI_MAX_THROW             TR("Máx. Throw", "Activar máx. throw")
#define TR_MULTI_RFCHAN                TR("Canal RF", "Selecciona canal RF")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_GPS_COORDS_FORMAT           TR("Coords GPS", "Formato coordenadas")
#define TR_VARIO                       TR("Vario", "Variómetro")
#define TR_PITCH_AT_ZERO               "Pitch en cero"
#define TR_PITCH_AT_MAX                "Pitch en máx"
#define TR_REPEAT_AT_ZERO              TR("Repet. en cero", "Repetir en cero")
#define TR_BATT_CALIB                  TR("Calib.bat.", "Calibración batería")
#define TR_CURRENT_CALIB               "Calib. actual"
#define TR_VOLTAGE                     "Voltaje"
#define TR_SELECT_MODEL                TR("Selec. modelo", "Seleccionar modelo")
#define TR_MANAGE_MODELS               "MODEL MANAGER"
#define TR_MODELS                      "Models"
#define TR_SELECT_MODE                 "Select mode"
#define TR_CREATE_MODEL                "Crear modelo"
#define TR_FAVORITE_LABEL              "Favorites"
#define TR_MODELS_MOVED                "Unused models moved to"
#define TR_NEW_MODEL                   "New Model"
#define TR_INVALID_MODEL               "Invalid Model"
#define TR_EDIT_LABELS                 "Edit Labels"
#define TR_LABEL_MODEL                 "Label model"
#define TR_MOVE_UP                     "Move Up"
#define TR_MOVE_DOWN                   "Move Down"
#define TR_ENTER_LABEL                 "Enter Label"
#define TR_LABEL                       "Label"
#define TR_LABELS                      "Labels"
#define TR_CURRENT_MODEL               "Current"
#define TR_ACTIVE                      "Active"
#define TR_NEW                         "New"
#define TR_NEW_LABEL                   "New Label"
#define TR_RENAME_LABEL                "Rename Label"
#define TR_DELETE_LABEL                "Delete Label"
#define TR_DUPLICATE_MODEL             "Duplicar modelo"
#define TR_COPY_MODEL                  "Copiar modelo"
#define TR_MOVE_MODEL                  "Mover modelo"
#define TR_BACKUP_MODEL                TR("Copia sgdad. mod.", "Copia seguridad modelo")
#define TR_DELETE_MODEL                "Borrar modelo"
#define TR_RESTORE_MODEL               "Restaurar modelo"
#define TR_DELETE_ERROR                "Error de borrado"
#define TR_SDCARD_ERROR                "Error SDCARD"
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "No files on SD!"
#define TR_NO_SDCARD                   "No SDCARD"
#define TR_WAITING_FOR_RX              "Esperando a RX..."
#define TR_WAITING_FOR_TX              "Esperando a TX..."
#define TR_WAITING_FOR_MODULE          TR("Waiting module", "Waiting for module...")
#define TR_NO_TOOLS                    "No hay utils"
#define TR_NORMAL                      "Normal"
#define TR_NOT_INVERTED                "No inv"
#define TR_NOT_CONNECTED               "!Conectado"
#define TR_CONNECTED                   "Conectado"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH sin telem.", "16CH sin telemetría")
#define TR_16CH_WITH_TELEMETRY         TR("16CH con telem.", "16CH con telemetría")
#define TR_8CH_WITH_TELEMETRY          TR("8CH con telem.", "8CH con telemetría")
#define TR_EXT_ANTENNA                 "Antena ext."
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Actualizar opciones RX?"
#define TR_UPDATE_TX_OPTIONS           "Actualizar opciones TX?"
#define TR_MODULES_RX_VERSION          BUTTON("Módulos / versión RX")
#define TR_SHOW_MIXER_MONITORS         "Show mixer monitors"
#define TR_MENU_MODULES_RX_VERSION     "MÓDULOS / VERSIÓN RX"
#define TR_MENU_FIRM_OPTIONS           "OPCIONES FIRMWARE"
#define TR_IMU                        "IMU"
#define TR_STICKS_POTS_SLIDERS         "Sticks/Pots/Sliders"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Sticks/Pots/Sliders"
#define TR_RF_PROTOCOL                 "Protocolo RF"
#define TR_MODULE_OPTIONS              "Opciones módulo"
#define TR_POWER                       "Potencia"
#define TR_NO_TX_OPTIONS               "Sin opciones TX"
#define TR_RTC_BATT                    TR("Bat. RTC", "Batería RTC")
#define TR_POWER_METER_EXT             "Medidor potencia(EXT)"
#define TR_POWER_METER_INT             "Medidor potencia(INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Espectro (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Espectro (INT)"
#define TR_GHOST_MODULE_CONFIG         "Ghost module config"
#define TR_GPS_MODEL_LOCATOR           "GPS model locator"
#define TR_REFRESH                     "Refresh"
#define TR_SDCARD_FULL                 "SD Card llena"
#define TR_SDCARD_FULL_EXT              TR_BW_COL(TR_SDCARD_FULL "\036Logs and " LCDW_128_LINEBREAK "Screenshots disabled", TR_SDCARD_FULL "\nLogs and Screenshots disabled")
#define TR_NEEDS_FILE                  "NECESITA ARCHIVO"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE        "Incompatible"
#define TR_WARNING             "AVISO"
#define TR_STORAGE_WARNING     "ALMAC."
#define TR_THROTTLE_UPPERCASE        TR("ACELERAD.", "ACELERADOR")
#define TR_ALARMSWARN          "ALARMAS"
#define TR_SWITCHWARN          TR("INTERRUP.", "INTERRUPTOR")
#define TR_FAILSAFEWARN        "FAILSAFE"
#define TR_TEST_WARNING         TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE         "Usar solo para test"
#define TR_WRONG_SDCARDVERSION  TR("Ver esperada: ", "Versión esperada: ")
#define TR_WARN_RTC_BATTERY_LOW "Batería RTC baja"
#define TR_WARN_MULTI_LOWPOWER  "Modo baja potencia"
#define TR_BATTERY              "BATERÍA"
#define TR_WRONG_PCBREV        "Placa PCB errónea"
#define TR_EMERGENCY_MODE      "MODO EMERGENCIA"
#define TR_NO_FAILSAFE         "Failsafe no fijado"
#define TR_KEYSTUCK            "Tecla atascada"
#define TR_VOLUME              "Volumen"
#define TR_LCD                 "LCD"
#define TR_BRIGHTNESS          "Brillo"
#define TR_CPU_TEMP            "CPU Temp.\016>"
#define TR_COPROC              "CoProc."
#define TR_COPROC_TEMP         "MB Temp. \016>"
#define TR_TTL_WARNING                 "Warning: Do not exceed 3.3V on TX/RX pins!"
#define TR_FUNC                "Función"
#define TR_V1                  "V1"
#define TR_V2                  "V2"
#define TR_DURATION            "Duración"
#define TR_DELAY               "Atraso"
#define TR_NO_SOUNDS_ON_SD     "Sin sonidos en SD"
#define TR_NO_MODELS_ON_SD     "Sin modelos en SD"
#define TR_NO_BITMAPS_ON_SD    "Sin imágenes en SD"
#define TR_NO_SCRIPTS_ON_SD    "No scripts en SD"
#define TR_SCRIPT_SYNTAX_ERROR TR("Syntax error", "Script syntax error")
#define TR_SCRIPT_PANIC        "Script panic"
#define TR_SCRIPT_KILLED       "Script killed"
#define TR_SCRIPT_ERROR        "Error desconocido"
#define TR_PLAY_FILE           "Play"
#define TR_DELETE_FILE         "Borrar"
#define TR_COPY_FILE           "Copiar"
#define TR_RENAME_FILE         "Renombrar"
#define TR_ASSIGN_BITMAP       "Asignar imagen"
#define TR_ASSIGN_SPLASH       "Pant. bienvenida"
#define TR_EXECUTE_FILE        "Ejecutar"
#define TR_REMOVED             "Borrado"
#define TR_SD_INFO             "Información"
#define TR_NA                  "N/A"
#define TR_FORMATTING          "Formateando.."
#define TR_TEMP_CALIB          "Temp. Calib"
#define TR_TIME                "Hora"
#define TR_MAXBAUDRATE         "Máx bauds"
#define TR_BAUDRATE            "Baudrate"
#define TR_CRSF_ARMING_MODE            TR("Modo Arm.", "Modo Armado")
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE         "Modo de muestra"
#define TR_SAMPLE_MODES_1      "Normal"
#define TR_SAMPLE_MODES_2      "OneBit"
#define TR_LOADING                     "Loading..."
#define TR_DELETE_THEME                "Delete Theme?"
#define TR_SAVE_THEME                  "Save Theme?"
#define TR_EDIT_COLOR                  "Edit Color"
#define TR_NO_THEME_IMAGE              "No theme image"
#define TR_BACKLIGHT_TIMER             "Inactivity timeout"

#define TR_MODEL_QUICK_SELECT        "Model quick select"
#define TR_LABELS_SELECT             "Label select"
#define TR_LABELS_MATCH              "Label matching"
#define TR_FAV_MATCH                 "Favorites matching"
#define TR_LABELS_SELECT_MODE_1      "Multi select"
#define TR_LABELS_SELECT_MODE_2      "Single select"
#define TR_LABELS_MATCH_MODE_1       "Match all"
#define TR_LABELS_MATCH_MODE_2       "Match any"
#define TR_FAV_MATCH_MODE_1          "Must match"
#define TR_FAV_MATCH_MODE_2          "Optional match"

#define TR_SELECT_TEMPLATE_FOLDER "Select a template folder"
#define TR_SELECT_TEMPLATE     "SELECT A MODEL TEMPLATE:"
#define TR_NO_TEMPLATES        "No model templates were found in this folder"
#define TR_SAVE_TEMPLATE       "Save as template"
#define TR_BLANK_MODEL                 "Blank Model"
#define TR_BLANK_MODEL_INFO            "Create a blank model"
#define TR_FILE_EXISTS                 "FILE ALREADY EXISTS"
#define TR_ASK_OVERWRITE               "Do you want to overwrite?"

#define TR_BLUETOOTH            "Bluetooth"
#define TR_BLUETOOTH_DISC       "Buscar"
#define TR_BLUETOOTH_INIT       "Init"
#define TR_BLUETOOTH_DIST_ADDR  "Dir. remota"
#define TR_BLUETOOTH_LOCAL_ADDR "Dir. local"
#define TR_BLUETOOTH_PIN_CODE   "Código PIN"
#define TR_BLUETOOTH_NODEVICES  "No Devices Found"
#define TR_BLUETOOTH_SCANNING   "Scanning..."
#define TR_BLUETOOTH_BAUDRATE   "BT Velocidad"
#define TR_BLUETOOTH_MODES_1    "---"
#define TR_BLUETOOTH_MODES_2    "Telemetría"
#define TR_BLUETOOTH_MODES_3    "Entrenador"
#define TR_BLUETOOTH_MODES_4    "Activo"
#define TR_SD_INFO_TITLE       "SD INFO"
#define TR_SD_SPEED            "Velocidad:"
#define TR_SD_SECTORS          "Sectores:"
#define TR_SD_SIZE             "Tamaño:"
#define TR_TYPE                "Tipo"
#define TR_GVARS               "V. GLOBAL"
#define TR_GLOBAL_VAR                  "Variable global"
#define TR_OWN                         "Propio"
#define TR_DATE                        "Fecha"
#define TR_MONTHS_1                    "Ene"
#define TR_MONTHS_2                    "Feb"
#define TR_MONTHS_3                    "Mar"
#define TR_MONTHS_4                    "Abr"
#define TR_MONTHS_5                    "May"
#define TR_MONTHS_6                    "Jun"
#define TR_MONTHS_7                    "Jul"
#define TR_MONTHS_8                    "Ago"
#define TR_MONTHS_9                    "Sep"
#define TR_MONTHS_10                   "Oct"
#define TR_MONTHS_11                   "Nov"
#define TR_MONTHS_12                   "Dic"
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR            "MONITOR CANALES"
#define TR_MIXERS_MONITOR              "MONITOR MEZCLAS"
#define TR_PATH_TOO_LONG               "Path muy largo"
#define TR_VIEW_TEXT                   "Ver texto"
#define TR_FLASH_BOOTLOADER            "Flash bootloader"
#define TR_FLASH_DEVICE                TR("Flash device","Flash device")
#define TR_FLASH_EXTERNAL_DEVICE       "Flash disp. externo"
#define TR_FLASH_RECEIVER_OTA          "Flash RX OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX por ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX por int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash módulo BT", "Flash módulo bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Flash unid. pwr mngt"
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "Device not responding")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Device file prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "Device data refused")
#define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "Device access problem")
#define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "Device file refused")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", "Device file sig.")
#define TR_CURRENT_VERSION             TR("Vers. actual", "Versión actual: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Flash módulo int", "Flash módulo interno")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Multi int", "Flash Multi interno")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash módulo ext", "Flash módulo externo")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Multi ext", "Flash Multi externo")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flash External ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Error act FW", "Error actualiz. firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Flash ok"
#define TR_WRITING                     "Escribiendo..."
#define TR_CONFIRM_FORMAT              "Confirmar formato?"
#define TR_INTERNALRF                  "RF interna"
#define TR_INTERNAL_MODULE             TR("Módulo int", "Módulo interno")
#define TR_EXTERNAL_MODULE             TR("Módulo ext", "Módulo externo")
#define TR_EDGETX_UPGRADE_REQUIRED     "Actualiz. EdgeTX requerida"
#define TR_TELEMETRY_DISABLED          "Telem. inactiva"
#define TR_MORE_OPTIONS_AVAILABLE      "Más opciones disp."
#define TR_NO_MODULE_INFORMATION       "Sin información módulo"
#define TR_EXTERNALRF                  "RF externa"
#define TR_FAILSAFE                    "Failsafe"
#define TR_FAILSAFESET                 "AJUSTES FAILSAFE"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Pers. ID"
#define TR_HOLD                        "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                        "Nada"
#define TR_NONE_UPPERCASE              "NADA"
#define TR_MENUSENSOR                  "SENSOR"
#define TR_POWERMETER_PEAK             "Pico"
#define TR_POWERMETER_POWER            "Potencia"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Frec."
#define TR_MENUTOOLS                   "UTILS"
#define TR_TURN_OFF_RECEIVER           "Apaga el receptor"
#define TR_STOPPING                    "Parando..."
#define TR_MENU_SPECTRUM_ANALYSER      "ANALIZADOR DE ESPECTRO"
#define TR_MENU_POWER_METER            "MEDIDOR POTENCIA"
#define TR_SENSOR                      "SENSOR"
#define TR_COUNTRY_CODE                 "Código país"
#define TR_USBMODE                     "Modo USB"
#define TR_JACK_MODE                    "Modo Jack"
#define TR_VOICE_LANGUAGE                   "Idioma voces"
#define TR_UNITS_SYSTEM                 "Unidades"
#define TR_UNITS_PPM                   "PPM Units"
#define TR_EDIT                        "Editar"
#define TR_INSERT_BEFORE               "Insertar antes"
#define TR_INSERT_AFTER                "Insertar después"
#define TR_COPY                        "Copiar"
#define TR_MOVE                "Mover"
#define TR_PASTE               "Pegar"
#define TR_PASTE_AFTER                 "Paste After"
#define TR_PASTE_BEFORE                "Paste Before"
#define TR_DELETE              "Borrar"
#define TR_INSERT              "Insertar"
#define TR_RESET_FLIGHT        "Reset Vuelo"
#define TR_RESET_TIMER1        "Reset Reloj 1"
#define TR_RESET_TIMER2        "Reset Reloj 2"
#define TR_RESET_TIMER3        "Reset Reloj 3"
#define TR_RESET_TELEMETRY     "Reset Telemetría"
#define TR_STATISTICS          "Estadísticas"
#define TR_ABOUT_US            "Nosotros"
#define TR_USB_JOYSTICK        "Joystick USB (HID)"
#define TR_USB_MASS_STORAGE    "Almaz. USB (SD)"
#define TR_USB_SERIAL          "Serie USB (VCP)"
#define TR_SETUP_SCREENS       "Pantallas config"
#define TR_MONITOR_SCREENS     "Monitores"
#define TR_AND_SWITCH          TR("Inter. AND", "Interruptor AND")
#define TR_SF                  "CF"
#define TR_GF                  "GF"
#define TR_ANADIAGS_CALIB      "Calibrated analogs"
#define TR_ANADIAGS_FILTRAWDEV "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW  "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX     "Min., max. and range"
#define TR_ANADIAGS_MOVE       "Move analogs to their extremes!"
#define TR_BYTES               "bytes"
#define TR_MODULE_BIND         "[Enl.]"
#define TR_MODULE_UNBIND               BUTTON("Unbind")
#define TR_POWERMETER_ATTN_NEEDED     "Necesita atenuador"
#define TR_PXX2_SELECT_RX              "Selecciona RX..."
#define TR_PXX2_DEFAULT                "<defecto>"
#define TR_BT_SELECT_DEVICE            "Selecciona dispositivo"
#define TR_DISCOVER             BUTTON("Buscar")
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Espera..."
#define TR_RECEIVER_DELETE             "Borrar receptor?"
#define TR_RECEIVER_RESET              "Reset receptor?"
#define TR_SHARE                       "Compartido"
#define TR_BIND                        "Emparejar"
#define TR_REGISTER             BUTTON(TR("Reg", "Registrar"))
#define TR_MODULE_RANGE        "[Lim.]"
#define TR_RANGE_TEST                  "Range test"
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "OPCIONES RECEPTOR")
#define TR_RESET_BTN           "[Reset]"
#define TR_KEYS_BTN                BUTTON(TR("SW","Switches"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analogs"))
#define TR_FS_BTN                      BUTTON(TR("Custom sw", "Customizable switches"))
#define TR_TOUCH_NOTFOUND              "Hardware táctil no encontrado"
#define TR_TOUCH_EXIT                  "Tocar pantalla para salir"
#define TR_CALIBRATION                   BUTTON("Calibración")
#define TR_SET                 "[Ajuste]"
#define TR_TRAINER             "Entrenador"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM      "Problema antena TX!"
#define TR_MODELIDUSED         TR("ID en uso", "ID modelo en uso")
#define TR_MODELIDUNIQUE       "ID is unique"
#define TR_MODULE              "Módulo"
#define TR_RX_NAME                     "Nombre Rx"
#define TR_TELEMETRY_TYPE      TR("Tipo", "Tipo telemetría")
#define TR_TELEMETRY_SENSORS   "Sensores"
#define TR_VALUE               "Valor"
#define TR_PERIOD              "Period"
#define TR_INTERVAL            "Interval"
#define TR_REPEAT              "Repeat"
#define TR_ENABLE              "Enable"
#define TR_DISABLE             "Disable"
#define TR_TOPLCDTIMER         "Timer LCD superior"
#define TR_UNIT                "Unidad"
#define TR_TELEMETRY_NEWSENSOR "Añadir sensor..."
#define TR_CHANNELRANGE        "Canales"
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequencia")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetría")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_ANTENNACONFIRM1     "ANTENA EXT."
#define TR_ANTENNA_MODES_1     "Interna"
#define TR_ANTENNA_MODES_2     "Preguntar"
#define TR_ANTENNA_MODES_3     "Por modelo"
#define TR_ANTENNA_MODES_4     "Interno + Externo"
#define TR_ANTENNA_MODES_5     "Externa"
#define TR_USE_INTERNAL_ANTENNA        TR("Usa antena int.", "Usa antena interna")
#define TR_USE_EXTERNAL_ANTENNA        TR("Use antena ext.", "Usa antena externa")
#define TR_ANTENNACONFIRM2     TR("Revisa antena", "Revisa que la antena esté instalada!")

#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1        "No requerido"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1         "Requiere FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1          "Requiere EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2             "firmware certificado"
#define TR_LOWALARM            "Alarma baja"
#define TR_CRITICALALARM       "Alarma crítica"
#define TR_DISABLE_ALARM       TR("Desact. alarmas", "Desact. alarmas telem.")
#define TR_POPUP               "Popup"
#define TR_MIN                 "Min"
#define TR_MAX                 "Máx"
#define TR_CURVE_PRESET        "Preset..."
#define TR_PRESET              "Preset"
#define TR_MIRROR              "Mirror"
#define TR_CLEAR               "Borrar"
#define TR_CLEAR_BTN           BUTTON("Borrar")
#define TR_RESET               "Reset"
#define TR_RESET_SUBMENU       "Reset..."
#define TR_COUNT               "Puntos"
#define TR_PT                  "pt"
#define TR_PTS                 "pts"
#define TR_SMOOTH              TR3("Suaviz.", "Suaviz.", "Suavizado")
#define TR_COPY_STICKS_TO_OFS  TR("Copia stick->subtr", "Copia Sticks a Offset")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Copia min/máx a sal",  "Copia min/máx/ctr a salidas")
#define TR_COPY_TRIMS_TO_OFS   TR("Copia trim->subtr", "Copia trims a subtrim")
#define TR_INCDEC              "Inc/Decrement"
#define TR_GLOBALVAR           "Var global"
#define TR_MIXSOURCE           "Fuente (%)"
#define TR_MIXSOURCERAW        "Fuente (valor)"
#define TR_CONSTANT            "Constante"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1 "Off"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2 "On"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3 "Auto"
#define TR_PREFLIGHT           "Chequeos prevuelo"
#define TR_CHECKLIST           TR("Lista verif", "Lista verificación")
#define TR_CHECKLIST_INTERACTIVE       TR3("C-Interact", "Interact. checklist", "Interactive checklist")
#define TR_AUX_SERIAL_MODE     "Puerto serie"
#define TR_AUX2_SERIAL_MODE    "Puerto serie 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port power"
#define TR_SCRIPT              "Script"
#define TR_INPUTS              "Entradas"
#define TR_OUTPUTS             "Salidas"
#define TR_CONFIRMRESET        "Borrar TODOS los " LCDW_128_LINEBREAK "modelos y conf?"
#define TR_TOO_MANY_LUA_SCRIPTS "Demasiados Lua scripts!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES_1  "AUTO"
#define TR_SPORT_UPDATE_POWER_MODES_2  "ON"
#define TR_NO_TELEMETRY_SCREENS        TR("No hay pant. telemetría", "No hay pantallas de telemetría")
#define TR_TOUCH_PANEL                 "Pantalla táctil:"
#define TR_FILE_SIZE                   "File size"
#define TR_FILE_OPEN                   "Open anyway?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Nombre"
#define TR_PHASES_HEADERS_SW           "Interrup"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trim timon"
#define TR_PHASES_HEADERS_ELE_TRIM     "Trim elevador"
#define TR_PHASES_HEADERS_THT_TRIM     "Trim potencia"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trim alerón"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Aparecer"
#define TR_PHASES_HEADERS_FAD_OUT      "Desparecer"

#define TR_LIMITS_HEADERS_NAME         "Nombre"
#define TR_LIMITS_HEADERS_SUBTRIM      "Compensación"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Máx"
#define TR_LIMITS_HEADERS_DIRECTION    "Dirección"
#define TR_LIMITS_HEADERS_CURVE        "Curva"
#define TR_LIMITS_HEADERS_PPMCENTER    "Centrado PPM"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Simétrica"
#define TR_INVERTED                    "Inverted"

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "Primer canal"
#define TR_FILL_BACKGROUND           "Rellenar fondo"
#define TR_BG_COLOR                  "Color BG"
#define TR_SLIDERS_TRIMS             "Sliders+Trims"
#define TR_SLIDERS                   "Sliders"
#define TR_FLIGHT_MODE               "Modo de vuelo"
#define TR_INVALID_FILE              "Fichero no valido"
#define TR_TIMER_SOURCE              "Entrada timer"
#define TR_SIZE                      "Tamaño"
#define TR_SHADOW                    "Sombra"
#define TR_ALIGNMENT                 "Alinear"
#define TR_ALIGN_LABEL               "Alinear marbete"
#define TR_ALIGN_VALUE               "Alinear valor"
#define TR_ALIGN_OPTS_1              "Lzquierdo"
#define TR_ALIGN_OPTS_2              "Centro"
#define TR_ALIGN_OPTS_3              "Derecho"
#define TR_TEXT                      "Texto"
#define TR_COLOR                     "Color"
#define TR_PANEL1_BACKGROUND         "Fondo panel1"
#define TR_PANEL2_BACKGROUND         "Fondo panel2"
#define TR_PANEL_BACKGROUND          "Background"
#define TR_PANEL_COLOR               "  Color"
#define TR_WIDGET_GAUGE              "Gauge"
#define TR_WIDGET_MODELBMP           "Models"
#define TR_WIDGET_OUTPUTS            "Outputs"
#define TR_WIDGET_TEXT               "Text"
#define TR_WIDGET_TIMER              "Timer"
#define TR_WIDGET_VALUE              "Value"

// About screen
#define TR_ABOUTUS             "Nosotros"

#define TR_CHR_HOUR   'h'
#define TR_CHR_INPUT  'I'   // Values between A-I will work

#define TR_BEEP_VOLUME         "Volumen Beep"
#define TR_WAV_VOLUME          "Volumen Wav"
#define TR_BG_VOLUME           "Volumen Bg"

#define TR_TOP_BAR             "Panel superior"
#define TR_FLASH_ERASE                 "Borrar flash..."
#define TR_FLASH_WRITE                 "Escribir flash..."
#define TR_OTA_UPDATE                  "Actualización OTA..."
#define TR_MODULE_RESET                "Reset módulo..."
#define TR_UNKNOWN_RX                  "RX desconocido"
#define TR_UNSUPPORTED_RX              "RX no soportado"
#define TR_OTA_UPDATE_ERROR            "Error actualización OTA"
#define TR_DEVICE_RESET                "Reset dispositivo..."
#define TR_ALTITUDE            "Altitud"
#define TR_SCALE               "Escala"
#define TR_VIEW_CHANNELS       "Ver Canales"
#define TR_VIEW_NOTES          "Ver Notas"
#define TR_MODEL_SELECT        "Seleccionar modelo"
#define TR_ID                  "ID"
#define TR_PRECISION           "Precisión"
#define TR_RATIO               "Ratio"
#define TR_FORMULA             "Fórmula"
#define TR_CELLINDEX           "Cell index"
#define TR_LOGS                "Logs"
#define TR_OPTIONS             "Opciones"
#define TR_FIRMWARE_OPTIONS    BUTTON("Opciones firmware")

#define TR_ALTSENSOR           "Alt sensor"
#define TR_CELLSENSOR          "Cell sensor"
#define TR_GPSSENSOR           "GPS sensor"
#define TR_CURRENTSENSOR       "Sensor"
#define TR_AUTOOFFSET          "Auto offset"
#define TR_ONLYPOSITIVE        "Positivo"
#define TR_FILTER              "Filtro"
#define TR_TELEMETRYFULL       TR("Telem. llena!", "Todas las entradas de telemetría llenas!")
#define TR_IGNORE_INSTANCE     TR("No inst.", "Ignora instancias")
#define TR_SHOW_INSTANCE_ID            "Show instance ID"
#define TR_DISCOVER_SENSORS    "Buscar sensores"
#define TR_STOP_DISCOVER_SENSORS "Parar busqueda"
#define TR_DELETE_ALL_SENSORS  "Borrar sensores"
#define TR_CONFIRMDELETE       "Seguro " LCDW_128_LINEBREAK "borrar todo ?"
#define TR_SELECT_WIDGET       "Seleccionar widget"
#define TR_WIDGET_FULLSCREEN           "Full screen"
#define TR_REMOVE_WIDGET       "Borrar widget"
#define TR_WIDGET_SETTINGS     "Config. widget"
#define TR_REMOVE_SCREEN       "Borrar pantalla"
#define TR_SETUP_WIDGETS       "Config. widgets"
#define TR_THEME               "Tema"
#define TR_SETUP               "Configuración"
#define TR_LAYOUT              "Diseño"
#define TR_TEXT_COLOR                  "Color texto"

#define TR_MENU_INPUTS          CHAR_INPUT "Entradas"
#define TR_MENU_LUA             CHAR_LUA "Lua scripts"
#define TR_MENU_STICKS           CHAR_STICK "Sticks"
#define TR_MENU_POTS            CHAR_POT "Pots"
#define TR_MENU_MIN             CHAR_FUNCTION "MIN"
#define TR_MENU_MAX             CHAR_FUNCTION "MAX"
#define TR_MENU_HELI            CHAR_CYC "Cíclico"
#define TR_MENU_TRIMS            CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES        CHAR_SWITCH "Interruptores"
#define TR_MENU_LOGICAL_SWITCHES TR( CHAR_SWITCH "Interr. lógicos",  CHAR_SWITCH "Interruptores lógicos")
#define TR_MENU_TRAINER         CHAR_TRAINER "Entrenador"
#define TR_MENU_CHANNELS        CHAR_CHANNEL "Canales"
#define TR_MENU_GVARS           CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY       CHAR_TELEMETRY "Telemetría"
#define TR_MENU_DISPLAY        "MONITOR"
#define TR_MENU_OTHER          "Otros"
#define TR_MENU_INVERT         "Invertir"
#define TR_AUDIO_MUTE                  TR("Audio mute","Mute if no sound")
#define TR_JITTER_FILTER       "Filtro ADC"
#define TR_DEAD_ZONE           "Dead zone"
#define TR_RTC_CHECK           TR("Check RTC", "Check RTC voltaje")
#define TR_AUTH_FAILURE        "Fallo " LCDW_128_LINEBREAK  "autentificación"
#define TR_RACING_MODE         "Racing mode"

// The following content is Untranslated)

#define TR_USE_THEME_COLOR              "Use theme color"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Add all Trims to Subtrims"
#define TR_DUPLICATE                    "Duplicate"
#define TR_ACTIVATE                     "Set Active"
#define TR_RED                          "Red"
#define TR_BLUE                         "Blue"
#define TR_GREEN                        "Green"
#define TR_COLOR_PICKER                 "Color Picker"
#define TR_FIXED                        "Fixed"
#define TR_EDIT_THEME_DETAILS           "Edit Theme Details"
#define TR_THEME_COLOR_DEFAULT         "DEFAULT"
#define TR_THEME_COLOR_PRIMARY1        "PRIMARY1"
#define TR_THEME_COLOR_PRIMARY2        "PRIMARY2"
#define TR_THEME_COLOR_PRIMARY3        "PRIMARY3"
#define TR_THEME_COLOR_SECONDARY1      "SECONDARY1"
#define TR_THEME_COLOR_SECONDARY2      "SECONDARY2"
#define TR_THEME_COLOR_SECONDARY3      "SECONDARY3"
#define TR_THEME_COLOR_FOCUS           "FOCUS"
#define TR_THEME_COLOR_EDIT            "EDIT"
#define TR_THEME_COLOR_ACTIVE          "ACTIVE"
#define TR_THEME_COLOR_WARNING         "WARNING"
#define TR_THEME_COLOR_DISABLED        "DISABLED"
#define TR_THEME_COLOR_CUSTOM          "CUSTOM"
#define TR_THEME_CHECKBOX              "Checkbox"
#define TR_THEME_ACTIVE                "Active"
#define TR_THEME_REGULAR               "Regular"
#define TR_THEME_WARNING               "Warning"
#define TR_THEME_DISABLED              "Disabled"
#define TR_THEME_EDIT                  "Edit"
#define TR_THEME_FOCUS                 "Focus"
#define TR_AUTHOR                       "Author"
#define TR_DESCRIPTION                  "Description"
#define TR_SAVE                         "Save"
#define TR_CANCEL                       "Cancel"
#define TR_EDIT_THEME                   "EDIT THEME"
#define TR_DETAILS                      "Details"

// Voice in native language
#define TR_VOICE_ENGLISH                "English"
#define TR_VOICE_CHINESE                "Chinese"
#define TR_VOICE_CZECH                  "Czech"
#define TR_VOICE_DANISH                 "Danish"
#define TR_VOICE_DEUTSCH                "Deutsch"
#define TR_VOICE_DUTCH                  "Dutch"
#define TR_VOICE_ESPANOL                "Espanol"
#define TR_VOICE_FINNISH                "Finnish"
#define TR_VOICE_FRANCAIS               "Francais"
#define TR_VOICE_HUNGARIAN              "Hungarian"
#define TR_VOICE_ITALIANO               "Italiano"
#define TR_VOICE_POLISH                 "Polish"
#define TR_VOICE_PORTUGUES              "Portugues"
#define TR_VOICE_RUSSIAN                "Russian"
#define TR_VOICE_SLOVAK                 "Slovak"
#define TR_VOICE_SWEDISH                "Swedish"
#define TR_VOICE_TAIWANESE              "Taiwanese"
#define TR_VOICE_JAPANESE               "Japanese"
#define TR_VOICE_HEBREW                 "Hebreo"
#define TR_VOICE_UKRAINIAN              "Ukrainian"
#define TR_VOICE_KOREAN                 "coreano"

#define TR_USBJOYSTICK_LABEL           "USB Joystick"
#define TR_USBJOYSTICK_EXTMODE         "Mode"
#define TR_VUSBJOYSTICK_EXTMODE_1      "Classic"
#define TR_VUSBJOYSTICK_EXTMODE_2      "Advanced"
#define TR_USBJOYSTICK_SETTINGS        BUTTON("Channel Settings")
#define TR_USBJOYSTICK_IF_MODE         TR("If. mode","Interface mode")
#define TR_VUSBJOYSTICK_IF_MODE_1      "Joystick"
#define TR_VUSBJOYSTICK_IF_MODE_2      "Gamepad"
#define TR_VUSBJOYSTICK_IF_MODE_3      "MultiAxis"
#define TR_USBJOYSTICK_CH_MODE         "Mode"
#define TR_VUSBJOYSTICK_CH_MODE_1      "None"
#define TR_VUSBJOYSTICK_CH_MODE_2      "Btn"
#define TR_VUSBJOYSTICK_CH_MODE_3      "Axis"
#define TR_VUSBJOYSTICK_CH_MODE_4      "Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S_1    "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2    "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3    "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4    "S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Button Mode"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1   "Normal"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2   "Pulse"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3   "SWEmu"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4   "Delta"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5   "Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1 TR("Norm","Normal")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2 TR("Puls","Pulse")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3 TR("SWEm","SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4 TR("Delt","Delta")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5 TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Positions"
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
#define TR_VUSBJOYSTICK_CH_AXIS_7      "Slider"
#define TR_VUSBJOYSTICK_CH_AXIS_8      "Dial"
#define TR_VUSBJOYSTICK_CH_AXIS_9      "Wheel"
#define TR_USBJOYSTICK_CH_SIM          "Sim axis"
#define TR_VUSBJOYSTICK_CH_SIM_1       "Ail"
#define TR_VUSBJOYSTICK_CH_SIM_2       "Ele"
#define TR_VUSBJOYSTICK_CH_SIM_3       "Rud"
#define TR_VUSBJOYSTICK_CH_SIM_4       "Thr"
#define TR_VUSBJOYSTICK_CH_SIM_5       "Acc"
#define TR_VUSBJOYSTICK_CH_SIM_6       "Brk"
#define TR_VUSBJOYSTICK_CH_SIM_7       "Steer"
#define TR_VUSBJOYSTICK_CH_SIM_8       "Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Inversion"
#define TR_USBJOYSTICK_CH_BTNNUM       "Button no."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Button no. collision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Axis collision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1 "None"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2 "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3 "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4 "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   BUTTON("Apply changes")

#define TR_DIGITAL_SERVO          "Servo333HZ"
#define TR_ANALOG_SERVO           "Servo 50HZ"
#define TR_SIGNAL_OUTPUT          "Signal output"
#define TR_SERIAL_BUS             "Serial bus"
#define TR_SYNC                   "Sync"

#define TR_ENABLED_FEATURES       "Enabled Features"
#define TR_RADIO_MENU_TABS        "Radio Menu"
#define TR_MODEL_MENU_TABS        "Model Menu"

#define TR_SELECT_MENU_ALL        "All"
#define TR_SELECT_MENU_CLR        "Clear"
#define TR_SELECT_MENU_INV        "Invert"

#define TR_SORT_ORDERS_1          "Name A-Z"
#define TR_SORT_ORDERS_2          "Name Z-A"
#define TR_SORT_ORDERS_3          "Least used"
#define TR_SORT_ORDERS_4          "Most used"
#define TR_SORT_MODELS_BY         "Sort models by"
#define TR_CREATE_NEW             "Create"

#define TR_MIX_SLOW_PREC          TR("Slow prec", "Slow up/dn prec")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "A theme directory with the same name already exists."

#define TR_DATE_TIME_WIDGET       "Date & Time"
#define TR_RADIO_INFO_WIDGET      "Radio Info"
#define TR_LOW_BATT_COLOR         "Low battery"
#define TR_MID_BATT_COLOR         "Mid battery"
#define TR_HIGH_BATT_COLOR        "High battery"

#define TR_WIDGET_SIZE            "Widget size"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"

#define TR_KEY_SHORTCUTS          "Key Shortcuts"
#define TR_CURRENT_SCREEN         "Current Screen"
#define TR_SHORT_PRESS            "Short Press"
#define TR_LONG_PRESS             "Long Press"
#define TR_OPEN_QUICK_MENU        "Open Quick Menu"
#define TR_QUICK_MENU_FAVORITES   "Quick Menu Favorites"
