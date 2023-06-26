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

// PT translations author: who?

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */

#define TR_MIN_PLURAL2  2
#define TR_MAX_PLURAL2  2
// For this number of minute in the last decimal place singular form is used in
// plural
#define TR_USE_SINGULAR_IN_PLURAL 1
#define TR_USE_PLURAL2_SPECIAL_CASE 0
// If the number of minutes is above this value PLURAL2 is used
#define TR_USE_PLURAL2 INT_MAX

#define TR_MINUTE_SINGULAR     "minute"
#define TR_MINUTE_PLURAL1      "minutes"
#define TR_MINUTE_PLURAL2      "minutes"

#define TR_OFFON               "OFF","ON"
#define TR_MMMINV              "---","INV"
#define TR_VBEEPMODE           "Mudo","Alarm","NoKey","Todo"
#define TR_VBLMODE             "OFF","Chav","Stks","Tudo","ON\0"
#define TR_TRNMODE             "OFF",TR("+=","Adicionar"),TR(":=","Trocar")
#define TR_TRNCHN              "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES    "OFF","S-Port Mirror","Telemetry","SBUS Trainer","LUA","CLI","GPS","Debug","SpaceMouse","External module"
#define TR_SWTYPES             "None","Toggle","2POS","3POS"
#define TR_POTTYPES            "None",TR("Pot w. det","Pot with detent"),TR("Multipos","Multipos Switch"),"Pot"
#define TR_SLIDERTYPES         "Rien","Slider"
#define TR_VPERSISTENT         "OFF","Flight","Manual Reset"
#define TR_COUNTRY_CODES       TR("US","America"),TR("JP","Japan"),TR("EU","Europe")
#define TR_USBMODES            "Ask",TR("Joyst","Joystick"),TR("SDCard","Storage"),"Serial"
#define TR_JACK_MODES          "Popup","Audio","Trainer"
#define TR_TELEMETRY_PROTOCOLS "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_CUSTOM        "Custom"
#define TR_VTRIMINC            "Expo","ExFino","Fino","Medio","Largo"
#define TR_VDISPLAYTRIMS       "No","Change","Yes"
#define TR_VBEEPCOUNTDOWN      "Silent","Beeps","Voice","Haptic",TR2_2("B & H","Beeps & Haptic"),TR2_2("V & H","Voice & Haptic")
#define TR_COUNTDOWNVALUES     "5s","10s","20s","30s"
#define TR_VVARIOCENTER        "Tone","Silent"
#define TR_CURVE_TYPES         "Standard","Custom"
#define TR_ADCFILTERVALUES     "Global","Off","On"

#define TR_VCURVETYPE        "Diff","Expo","Func","Cstm"
#define TR_VCURVEFUNC    "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX        "Adicionar","Multipl.","Trocar"
#define TR_VMLTPX2       "+=","*=",":="

#if defined(PCBTARANIS)
  #define TR_CSWTIMER          "Timer"
  #define TR_CSWSTICKY         "Stcky"
  #define TR_CSWRANGE          "Range"
  #define TR_CSWSTAY           "Edge"
#else
  #define TR_CSWTIMER          "Tim"
  #define TR_CSWSTICKY         "Glue"
  #define TR_CSWRANGE          "Rnge"
  #define TR_CSWSTAY           "Edge"
#endif

#define TR_CSWEQUAL      "a=x"
#define TR_VCSWFUNC      "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER         "Trainer"
#define TR_SF_INST_TRIM       "Inst. Trim"
#define TR_SF_RESET           "Reset"
#define TR_SF_SET_TIMER       "Set"
#define TR_SF_VOLUME          "Volume"
#define TR_SF_FAILSAFE        "SetFailsafe"
#define TR_SF_RANGE_CHECK     "RangeCheck"
#define TR_SF_MOD_BIND        "ModuleBind"
 
#define TR_SOUND              "Tocar Audio"
#define TR_PLAY_TRACK         "Tocar"
#define TR_PLAY_VALUE         "Ouvir Valor"
#define TR_SF_HAPTIC          "Vibrar"
#define TR_SF_PLAY_SCRIPT     "Lua Script"
#define TR_SF_BG_MUSIC        "BgMusic"
#define TR_SF_BG_MUSIC_PAUSE  "BgMusic ||"
#define TR_SF_LOGS            "SDCARD Logs"
#define TR_ADJUST_GVAR        "Ajuste"
#define TR_SF_BACKLIGHT       "Backlight"
#define TR_SF_VARIO           "Vario"
#define TR_SF_TEST            "Test"
#define TR_SF_SAFETY          "Seguro"

#define TR_SF_SCREENSHOT      "Screenshot"
#define TR_SF_TEST            "Test"
#define TR_SF_RACING_MODE     "RacingMode"
#define TR_SF_DISABLE_TOUCH   "No Touch"
#define TR_SF_SET_SCREEN      "Set Main Screen"

#define TR_FSW_RESET_TELEM     TR("Telm", "Telemetry")

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS  "Timer 1","Timer 2","Timer 3"
#else
  #define TR_FSW_RESET_TIMERS  "Tmr1","Tmr2","Tmr3"
#endif

#define TR_VFSWRESET           TR_FSW_RESET_TIMERS,TR("All","Flight"),TR_FSW_RESET_TELEM
#define TR_FUNCSOUNDS          TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Avis1"),TR("Wrn2","Avis2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP        "ft"
#define SPEED_UNIT_IMP         "mph"
#define LENGTH_UNIT_METR       "m"
#define SPEED_UNIT_METR        "kmh"

#define TR_VUNITSSYSTEM        "Metric",TR("Imper.","Imperial")
#define TR_VTELEMUNIT          "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                  (STR_VTELEMUNIT[1])
#define STR_A                  (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE    "None","Nums","Bars","Script"
#define TR_GPSFORMAT           "HMS","NMEA"


#define TR_VSWASHTYPE           "---","120","120X","140","90"

#define TR_STICK_NAMES          "Lem", "Pfd", "Mot", "Ail"
#define TR_SURFACE_NAMES        "Lem", "Mot"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Default","High"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES      "ON","One"

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT  "Normal","Inverted"
#else
  #define TR_ROTARY_ENC_OPT  "Normal","Inverted","V-I H-N","V-I H-A"
#endif

#if defined(IMU)
  #define TR_IMU_VSRCRAW       "TltX","TltY",
#else
  #define TR_IMU_VSRCRAW
#endif

#if defined(HELI)
  #define TR_CYC_VSRCRAW        "CYC1","CYC2","CYC3"
#else
  #define TR_CYC_VSRCRAW        "[C1]","[C2]","[C3]"
#endif


#define TR_SRC_BATT                    "Batt"
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES                   "OFF","ABS","Strt","MTs","MT%","MTt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Master/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Slave/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Master/SBUS Module"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Master/CPPM Module"
#define TR_VTRAINER_MASTER_BATTERY     "Master/Serial"
#define TR_VTRAINER_BLUETOOTH          "Master/" TR("BT","Bluetooth"), "Slave/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VFAILSAFE                   "Not set","Hold","Custom","No pulses","Receiver"
#define TR_VSENSORTYPES                "Custom","Calculated"
#define TR_VFORMULAS                   "Add","Average","Min","Max","Multiply","Totalize","Cell","Consumpt","Distance"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Lowest","1","2","3","4","5","6","7","8","Highest","Delta"
#define TR_TEXT_SIZE                   "Standard","Tiny","Small","Mid","Double"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center only)","= (symetrical)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "STD"
#else
#define TR_FONT_SIZES                  "STD","BOLD","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBTARANIS) || defined(PCBHORUS)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14)
  #define TR_ENTER                     "[NEXT]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#if defined(PCBHORUS)
  #define TR_EXIT                      "RTN"
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "EXIT"
  #define TR_OK                        TR("\010" "\010" "\010" "[OK]", "\010" "\010" "\010" "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)

#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_MENUWHENDONE      CENTER"\006" TR_ENTER "QDO PRONTO"
#define TR_FREE                "Livre"
#define TR_YES                         "Yes"
#define TR_NO                          "No"
#define TR_DELETEMODEL         "EXCLUI MODELO"
#define TR_COPYINGMODEL        "Copiando modelo"
#define TR_MOVINGMODEL         "Movendo modelo"
#define TR_LOADINGMODEL        "Carregar modelo"
#define TR_UNLABELEDMODEL              "Unlabeled"
#define TR_NAME                "Nome"
#define TR_MODELNAME           "Model Name"
#define TR_PHASENAME           "Phase Name"
#define TR_MIXNAME             "Mix Name"
#define TR_INPUTNAME           "Input Name"
#if defined(PCBTARANIS)
  #define TR_EXPONAME          "Line Name"
#else
  #define TR_EXPONAME          "Expo Name"
#endif
#define TR_BITMAP              "Model Image"
#define TR_NO_PICTURE          "No Picture"
#define TR_TIMER               "Cronom"
#define TR_START               "Start"
#define TR_ELIMITS             "E.Limits"
#define TR_ETRIMS              "E.Trims"
#define TR_TRIMINC             "Trim Step"
#define TR_DISPLAY_TRIMS       "Display Trims"
#define TR_TTRACE              TR("T-Trace", INDENT "T-Trace")
#define TR_TTRIM               TR("T-Trim", INDENT "T-Trim")
#define TR_TTRIM_SW            TR("T-Trim-Sw", INDENT "Trim switch")
#define TR_BEEPCTR             "Ctr Beep"
#define TR_USE_GLOBAL_FUNCS    "Use Global Funcs"
#define TR_PROTOCOL            "Proto"
#define TR_PPMFRAME            "PPM frame"
#define TR_REFRESHRATE               TR(INDENT "Refresh", INDENT "Refresh rate")
#define STR_WARN_BATTVOLTAGE           TR(INDENT "Output is VBAT: ", INDENT "Warning: output level is VBAT: ")
#define TR_WARN_5VOLTS                 "Warning: output level is 5 volts"
#define TR_MS                  "ms"
#define TR_SWITCH              "Chave"
#define TR_SF_SWITCH           "Trigger"
#define TR_TRIMS               "Trims"
#define TR_FADEIN              "Aparecer"
#define TR_FADEOUT             "Ocultar"
#define TR_DEFAULT             "(default)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS        "Check FM Trims"
#else
  #define TR_CHECKTRIMS        "\006Check\012Trims"
#endif
#define OFS_CHECKTRIMS         (9*FW)
#define TR_SWASHTYPE           "Ciclico Tipo"
#define TR_COLLECTIVE          "Coletivo"
#define TR_AILERON             TR("Lateral cyc.", "Lateral cyc. source")
#define TR_ELEVATOR            TR("Long. cyc.", "Long. cyc. source")
#define TR_SWASHRING           "Anel Coletivo"
#define TR_MODE                "Modo"
#if LCD_W > LCD_H
  #define TR_LEFT_STICK        "Esquerda"
#else
  #define TR_LEFT_STICK        "Esq"
#endif
#define TR_SUBTYPE             INDENT "Subtype"
#define TR_NOFREEEXPO          "No free expo!"
#define TR_NOFREEMIXER         "No free mixer!"
#define TR_SOURCE              "Fonte"
#define TR_WEIGHT              "Quantia"
#define TR_SIDE                "Lado"
#define TR_OFFSET              "Desvio"
#define TR_TRIM                "Trim"
#define TR_DREX                "DRex"
#define DREX_CHBOX_OFFSET      30
#define TR_CURVE               "Curvas"
#define TR_FLMODE              "Fase"
#define TR_MIXWARNING          "Aviso"
#define TR_OFF                 "OFF"
#define TR_ANTENNA                     "Antenna"
#define TR_NO_INFORMATION      TR("No info", "No information")
#define TR_MULTPX              "Aplicar"
#define TR_DELAYDOWN           "Atraso Dn"
#define TR_DELAYUP             "Atraso Up"
#define TR_SLOWDOWN            "Lento  Dn"
#define TR_SLOWUP              "Lento  Up"
#define TR_MIXES               "MIXAGENS"
#define TR_CV                  "CV"
#if defined(PCBNV14)
#define TR_GV                  "GV"
#else
#define TR_GV                  TR("G", "GV")
#endif
#define TR_RANGE               INDENT"Range"
#define TR_CENTER              INDENT "Center"
#define TR_ALARM               "Alarme"
#define TR_BLADES              "Helice"
#define TR_SCREEN              "Tela"
#define TR_SOUND_LABEL         "Som"
#define TR_LENGTH              "Comprimento"
#define TR_BEEP_LENGTH         "Beep Length"
#define TR_BEEP_PITCH          "Pitch"
#define TR_HAPTIC_LABEL        "Vibrar"
#define TR_STRENGTH            "Forca"
#define TR_IMU_LABEL           "IMU"
#define TR_IMU_OFFSET          "Offset"
#define TR_IMU_MAX             "Max"
#define TR_CONTRAST            "Contraste"
#define TR_ALARMS_LABEL        "Alarmes"
#define TR_BATTERY_RANGE       "Battery Range"
#define TR_BATTERYCHARGING     "Charging..."
#define TR_BATTERYFULL         "Battery full"
#define TR_BATTERYNONE         "None!"
#define TR_BATTERYWARNING      "Bateria Baixa"
#define TR_INACTIVITYALARM     "Inactividade"
#define TR_MEMORYWARNING       "Memoria Baixa"
#define TR_ALARMWARNING        "Som Off"
#define TR_RSSI_SHUTDOWN_ALARM   TR("Rssi Shutdown", "Check Rssi on Shutdown")
#define TR_MODEL_STILL_POWERED "Model still powered"
#define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_MODEL_SHUTDOWN              "Shutdown ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Press enter to confirm"
#define TR_THROTTLE_LABEL      "Throttle"
#define TR_THROTTLE_START      "Throttle Start"
#define TR_THROTTLEREVERSE     TR("Inverte Acel.", INDENT "Inverte Acel.")
#define TR_MINUTEBEEP          "Beep Minuto"
#define TR_BEEPCOUNTDOWN       INDENT "Beep Regressivo"
#define TR_PERSISTENT          INDENT "Persist."
#define TR_BACKLIGHT_LABEL     "Backlight"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLDELAY             INDENT "Tempo Backlight"
#define TR_BLONBRIGHTNESS      INDENT "ON Brightness"
#define TR_BLOFFBRIGHTNESS     INDENT "OFF Brightness"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR             "Color"
#define TR_SPLASHSCREEN        "Splash screen"
#define TR_PWR_ON_DELAY                "Pwr On delay"
#define TR_PWR_OFF_DELAY               "Pwr Off delay"
#define TR_THROTTLE_WARNING     TR("Avisa Acel", INDENT "Avisa Acel")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Cust-Pos", INDENT INDENT INDENT INDENT "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING       TR("Avisa Chav", INDENT "Avisa Chav")
#define TR_POTWARNINGSTATE     TR(INDENT "Pot&Slid.", INDENT "Pots & sliders")
#define TR_SLIDERWARNING       TR(INDENT "Slid. pos.", INDENT "Slider positions")
#define TR_POTWARNING          TR("Pot Warn.", INDENT "Pot Warning")
#define TR_TIMEZONE            "Time Zone"
#define TR_ADJUST_RTC          TR("Adjust RTC", "Adjust RTC")
#define TR_GPS                 "GPS"
#define TR_DEF_CHAN_ORD        "Ordem Canal RX"
#define TR_STICKS              "Sticks"
#define TR_POTS                "Pots"
#define TR_SWITCHES            "Switches"
#define TR_SWITCHES_DELAY      "Switches Delay"
#define TR_SLAVE               "Escravo"
#define TR_MODESRC             "Modo\006% Fonte"
#define TR_MULTIPLIER          "Multiplicar"
#define TR_CAL                 "Cal"
#define TR_CALIBRATION                 "Calibration"
#define TR_VTRIM               "Trim - +"
#define TR_CALIB_DONE                  "Calibration completed"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART       "Press [Enter] to start"
  #define TR_SETMIDPOINT       "Center sticks/pots/sliders and press [Enter]"
  #define TR_MOVESTICKSPOTS    "Move sticks, pots and sliders and press [Enter]"
#else
  #define TR_MENUTOSTART       CENTER"\011" TR_ENTER " INICIAR"
  #define TR_SETMIDPOINT       TR(CENTER"\005CENTRAR STICK/POT",CENTER"\006CENTRAR STICK/POT")
  #define TR_MOVESTICKSPOTS    TR(CENTER"\005MOVER STICKS/POTs",CENTER"\006MOVER STICKS/POTs")
#endif
#define TR_TXnRX               "Tx:\0Rx:"
#define OFS_RX                 4
#define TR_NODATA              CENTER"SEM DADOS"
#define TR_US                         "us"
#define TR_HZ                         "Hz"
#define TR_TMIXMAXMS                  "Tmix max"
#define TR_FREE_STACK                 "Free stack"
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
#define TR_MENUTORESET         TR_ENTER" Reinicia"
#define TR_PPM_TRAINER         "TR"
#define TR_CH                  "CH"
#define TR_MODEL               "MODEL"
#define TR_FM                  "FP"
#define TR_EEPROMLOWMEM        "EEPROM BX Memoria"
#define TR_PRESS_ANY_KEY_TO_SKIP   "Pulsar Tecla p/ sair"
#define TR_THROTTLE_NOT_IDLE     "**ACELERADOR ATIVO**"
#define TR_ALARMSDISABLED      "Desabilita Alarmes"
#define TR_PRESSANYKEY         "\010Pressione Tecla"
#define TR_BADEEPROMDATA       "EEPRON INVALIDA"
#define TR_BAD_RADIO_DATA      "Bad Radio Data"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING    "Formatando EEPROM"
#define TR_STORAGE_FORMAT      "Storage Preparation"
#define TR_EEPROMOVERFLOW      "EEPROM CHEIA"
#define TR_RADIO_SETUP      "AJUSTAR RADIO"
#define TR_MENUTRAINER         "TRAINER"
#define TR_MENUSPECIALFUNCS    "GLOBAL FUNCTIONS"
#define TR_MENUVERSION         "VERSAO"
#define TR_MENU_RADIO_SWITCHES            "DIAGNOST"
#define TR_MENU_RADIO_ANALOGS             "ANALOGICOS"
#define TR_MENU_RADIO_ANALOGS_CALIB       "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS   "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION     "CALIBRAGEM"
#define TR_MENU_FSWITCH        "FUNCTION SWITCHES"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS     "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS     "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE   "Channels=>Failsafe"
#define TR_CHANNEL2FAILSAFE    "Channel=>Failsafe"
#define TR_MENUMODELSEL        "MODELOS"
#define TR_MENU_MODEL_SETUP           "SETUP"
#define TR_MENUFLIGHTMODE      "MODO DE VOO"
#define TR_MENUFLIGHTMODES     "MODOS DE VOO"
#define TR_MENUHELISETUP       "HELI SETUP"


// Alignment

#if defined(PPM_CENTER_ADJUSTABLE) || defined(PPM_LIMITS_SYMETRICAL) // The right menu titles for the gurus ...
  #define TR_MENUINPUTS          "STICKS"
  #define TR_MENULIMITS          "SAIDAS"
#else
  #define TR_MENUINPUTS          "DR/EXPO"
  #define TR_MENULIMITS          "LIMITES"
#endif

#define TR_MENUCURVES          "CURVAS"
#define TR_MENUCURVE           "CURVA"
#define TR_MENULOGICALSWITCH    "CHAVE ESPECIAL"
#define TR_MENULOGICALSWITCHES  "CHAVES ESPECIAIS"
#define TR_MENUCUSTOMFUNC      "FUNCAO ESPECIAL"
#define TR_MENUCUSTOMSCRIPTS   "CUSTOM SCRIPTS"
#define TR_MENUTELEMETRY       "TELEMETRIA"
#define TR_MENUSTAT            "REGISTROS"
#define TR_MENUDEBUG           "DEPURAR"
#define TR_MONITOR_CHANNELS1   "CHANNELS MONITOR 1/8"
#define TR_MONITOR_CHANNELS2   "CHANNELS MONITOR 9/16"
#define TR_MONITOR_SWITCHES    "LOGICAL SWITCHES MONITOR"
#define TR_MONITOR_CHANNELS3   "CHANNELS MONITOR 17/24"
#define TR_MONITOR_CHANNELS4   "CHANNELS MONITOR 25/32"
#define TR_MONITOR_OUTPUT_DESC "Outputs"
#define TR_MONITOR_MIXER_DESC  "Mixers"
#define TR_RECEIVER_NUM        "RxNum"
#define TR_RECEIVER            "Receiver"
#define TR_MULTI_RFTUNE        TR("Freq tune", "RF Freq. fine tune")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY     "Telemetry"
#define TR_MULTI_VIDFREQ       TR("Vid. freq.", "Video frequency")
#define TR_RF_POWER       "RF Power"
#define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")
#define TR_MULTI_OPTION        TR("Option", "Option value")
#define TR_MULTI_AUTOBIND      TR(INDENT "Bind Ch.",INDENT "Bind on channel")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Disable Ch. map")
#define TR_DISABLE_TELEM               TR("No Telem", "Disable Telemetry")
#define TR_MULTI_DSM_AUTODTECT TR(INDENT "Autodetect", INDENT "Autodetect format")
#define TR_MULTI_LOWPOWER      TR(INDENT "Low power", INDENT "Low power mode")
#define TR_MULTI_LNA_DISABLE            INDENT "LNA disable"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")
#define TR_MODULE_TELEM_ON             TR("ON", "Enabled")
#define TR_DISABLE_INTERNAL         TR("Disable int. RF", "Disable internal RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("!serial mode", "Not in serial mode")
#define TR_MODULE_NO_INPUT             TR("No input", "No serial input")
#define TR_MODULE_NO_TELEMETRY         TR3( "No telmetry", "No MULTI_TELEMETRY", "No telemetry (enable MULTI_TELEMETRY)")
#define TR_MODULE_WAITFORBIND          "Bind to load protocol"
#define TR_MODULE_BINDING              "Binding"
#define TR_MODULE_UPGRADE_ALERT        TR3("Upg. needed", "Module upgrade required", "Module\nUpgrade required")
#define TR_MODULE_UPGRADE              TR("Upg. advised", "Module update recommended")
#define TR_REBIND                      "Rebinding required"
#define TR_REG_OK                      "Registration ok"
#define TR_BIND_OK                     "Bind successful"
#define TR_BINDING_CH1_8_TELEM_ON               "Ch1-8 Telem ON"
#define TR_BINDING_CH1_8_TELEM_OFF               "Ch1-8 Telem OFF"
#define TR_BINDING_CH9_16_TELEM_ON               "Ch9-16 Telem ON"
#define TR_BINDING_CH9_16_TELEM_OFF               "Ch9-16 Telem OFF"
#define TR_PROTOCOL_INVALID            TR("Prot. invalid", "Protocol invalid")
#define TR_MODULE_STATUS                TR(INDENT "Status", INDENT "Module Status")
#define TR_MODULE_SYNC                 TR(INDENT "Sync", INDENT "Proto Sync Status")
#define TR_MULTI_SERVOFREQ     TR("Servo rate", "Servo update rate")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("RF Channel", "Select RF channel")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_GPS_COORDS_FORMAT            "Gps Coords"
#define TR_VARIO               "Vario"
#define TR_PITCH_AT_ZERO       "Pitch at Zero"
#define TR_PITCH_AT_MAX        "Pitch at Max"
#define TR_REPEAT_AT_ZERO      "Repeat at Zero"
#define TR_BATT_CALIB          "CALIBRAR BATT"
#define TR_CURRENT_CALIB       "Corrente Calib"
#define TR_VOLTAGE             INDENT"Volts"
#define TR_SELECT_MODEL        "Selec. Modelo"
#define TR_MANAGE_MODELS               "MODEL MANAGER"
#define TR_MODELS                      "Models"
#define TR_SELECT_MODE                 "Select mode"
#define TR_CREATE_MODEL        "Criar Modelo"
#define TR_FAVORITE_LABEL      "Favorites"
#define TR_MODELS_MOVED        "Unused models moved to"
#define TR_NEW_MODEL           "New Model"
#define TR_INVALID_MODEL       "Invalid Model"
#define TR_EDIT_LABELS         "Edit Labels"
#define TR_LABEL_MODEL                 "Label model"
#define TR_MOVE_UP             "Move Up"
#define TR_MOVE_DOWN           "Move Down"
#define TR_ENTER_LABEL         "Enter Label"
#define TR_LABEL               "Label"
#define TR_LABELS              "Labels"
#define TR_CURRENT_MODEL       "Current"
#define TR_ACTIVE                      "Active"
#define TR_NEW                 "New"
#define TR_NEW_LABEL           "New Label"
#define TR_RENAME_LABEL        "Rename Label"
#define TR_DELETE_LABEL        "Delete Label"
#define TR_DUPLICATE_MODEL     "Duplicate Model"
#define TR_COPY_MODEL          "Copy Model"
#define TR_MOVE_MODEL          "Move Model"
#define TR_BACKUP_MODEL        "Salvar Modelo"
#define TR_DELETE_MODEL        "Apagar Modelo"
#define TR_RESTORE_MODEL       "Restaura Modelo"
#define TR_DELETE_ERROR        "Delete error"
#define TR_SDCARD_ERROR        "SDCARD Erro"
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "No files on SD!"
#define TR_NO_SDCARD           "Sem SDCARD"
#define TR_WAITING_FOR_RX              "Waiting for RX..."
#define TR_WAITING_FOR_TX              "Waiting for TX..."
#define TR_WAITING_FOR_MODULE          TR("Waiting module", "Waiting for module...")
#define TR_NO_TOOLS                    "No tools available"
#define TR_NORMAL                      "Normal"
#define TR_NOT_INVERTED                "Not inv"
#define TR_NOT_CONNECTED               "!Connected"
#define TR_CONNECTED                   "Connected"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH without telem.", "16CH without telemetry")
#define TR_16CH_WITH_TELEMETRY         TR("16CH with telem.", "16CH with telemetry")
#define TR_8CH_WITH_TELEMETRY          TR("8CH with telem.", "8CH with telemetry")
#define TR_EXT_ANTENNA                 "Ext. antenna"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Update RX options?"
#define TR_UPDATE_TX_OPTIONS           "Update TX options?"
#define TR_MODULES_RX_VERSION          "Modules / RX version"
#define TR_SHOW_MIXER_MONITORS         "Show mixer monitors"
#define TR_MENU_MODULES_RX_VERSION     "MODULES / RX VERSION"
#define TR_MENU_FIRM_OPTIONS           "FIRMWARE OPTIONS"
#define TR_IMU                        "IMU"
#define TR_STICKS_POTS_SLIDERS         "Sticks/Pots/Sliders"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Sticks/Pots/Sliders"
#define TR_RF_PROTOCOL                 "RF Protocol"
#define TR_MODULE_OPTIONS              "Module options"
#define TR_POWER                       "Power"
#define TR_NO_TX_OPTIONS               "No TX options"
#define TR_RTC_BATT                    "RTC Batt"
#define TR_POWER_METER_EXT             "Power Meter (EXT)"
#define TR_POWER_METER_INT             "Power Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spectrum (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Spectrum (INT)"
#define TR_SDCARD_FULL                 "SD Card Full"
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE        "Incompativel"
#define TR_WARNING             "AVISO"
#define TR_EEPROMWARN          "EEPROM"
#define TR_STORAGE_WARNING     "STORAGE"
#define TR_EEPROM_CONVERTING   "EEPROM Converting"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD card conversion required"
#define TR_CONVERTING                  "Converting: "
#define TR_THROTTLE_UPPERCASE        "ACELERAD"
#define TR_ALARMSWARN          "ALARMES"
#define TR_SWITCHWARN          "CHAVES"
#define TR_FAILSAFEWARN        "FAILSAFE"
#define TR_TEST_WARNING        TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE        "Use for tests only"
#define TR_WRONG_SDCARDVERSION TR("Expected ver: ","Expected version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery low"
#define TR_WARN_MULTI_LOWPOWER         "Low power mode"
#define TR_BATTERY                     "BATTERY"
#define TR_WRONG_PCBREV        "Wrong PCB detected"
#define TR_EMERGENCY_MODE      "EMERGENCY MODE"
#define TR_NO_FAILSAFE         "Failsafe not set"
#define TR_KEYSTUCK            "Key stuck"
#define TR_VOLUME              "Volume"
#define TR_LCD                 "LCD"
#define TR_BRIGHTNESS          "Brilho"
#define TR_CPU_TEMP            "CPU Temp.\016>"
#define TR_CPU_CURRENT         "Current\022>"
#define TR_CPU_MAH             "Consumo."
#define TR_COPROC              "CoProc."
#define TR_COPROC_TEMP         "MB Temp. \016>"
#define TR_CAPAWARNING         INDENT "Aviso Capacidade"
#define TR_TEMPWARNING         INDENT "Temperat. ALTA"
#define TR_TTL_WARNING         "Warning: use 3.3V logic levels"
#define TR_FUNC                "Funcao"
#define TR_V1                  "V1"
#define TR_V2                  "V2"
#define TR_DURATION            "Tempo"
#define TR_DELAY               "Atraso"
#define TR_SD_CARD             "SD CARD"
#define TR_SDHC_CARD           "SD-HC CARD"
#define TR_NO_SOUNDS_ON_SD     "Sem Som no SD"
#define TR_NO_MODELS_ON_SD     "Sem Modelo no SD"
#define TR_NO_BITMAPS_ON_SD    "No Bitmaps on SD"
#define TR_NO_SCRIPTS_ON_SD    "No Scripts on SD"
#define TR_SCRIPT_SYNTAX_ERROR TR("Syntax error", "Script syntax error")
#define TR_SCRIPT_PANIC        "Script panic"
#define TR_SCRIPT_KILLED       "Script killed"
#define TR_SCRIPT_ERROR        "Unknown error"
#define TR_PLAY_FILE           "Play"
#define TR_DELETE_FILE         "Apagar"
#define TR_COPY_FILE           "Copiar"
#define TR_RENAME_FILE         "Renomear"
#define TR_ASSIGN_BITMAP       "Assign Bitmap"
#define TR_ASSIGN_SPLASH       "Splash screen"
#define TR_EXECUTE_FILE        "Execute"
#define TR_REMOVED             "Remover"
#define TR_SD_INFO             "Dados SD"
#define TR_NA                  "N/A"
#define TR_HARDWARE            "HARDWARE"
#define TR_FORMATTING          "Formatando..."
#define TR_TEMP_CALIB          "Temp. Calib"
#define TR_TIME                "Time"
#define TR_MAXBAUDRATE         "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE         TR("Sampling","Sample Mode")
#define TR_SAMPLE_MODES        "Normal","OneBit"
#define TR_LOADING                     "Loading..."
#define TR_DELETE_THEME                "Delete Theme?"
#define TR_SAVE_THEME                  "Save Theme?"
#define TR_EDIT_COLOR                  "Edit Color"
#define TR_NO_THEME_IMAGE              "No theme image"
#define TR_BACKLIGHT_TIMER             "Inactivity timeout"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Model quick select"
#endif

#define TR_SELECT_TEMPLATE_FOLDER "Select a template folder"
#define TR_SELECT_TEMPLATE     "SELECT A MODEL TEMPLATE:"
#define TR_NO_TEMPLATES        "No model templates were found in this folder"
#define TR_SAVE_TEMPLATE       "Save as template"
#define TR_BLANK_MODEL                 "Blank Model"
#define TR_BLANK_MODEL_INFO            "Create a blank model"
#define TR_FILE_EXISTS         "FILE ALREADY EXISTS"
#define TR_ASK_OVERWRITE       "Do you want to overwrite?"

#define TR_BLUETOOTH            "Bluetooth"
#define TR_BLUETOOTH_DISC       "Discover"
#define TR_BLUETOOTH_INIT       "Init"
#define TR_BLUETOOTH_DIST_ADDR  "Dist addr"
#define TR_BLUETOOTH_LOCAL_ADDR "Local addr"
#define TR_BLUETOOTH_PIN_CODE   "PIN code"
#define TR_BLUETOOTH_NODEVICES  "No Devices Found"
#define TR_BLUETOOTH_SCANNING   "Scanning..."
#define TR_BLUETOOTH_BAUDRATE   "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES      "---","Enabled"
#else
#define TR_BLUETOOTH_MODES      "---","Telemetry","Trainer"
#endif
#define TR_SD_INFO_TITLE       "SD INFO"
#define TR_SD_TYPE             "Tipe"
#define TR_SD_SPEED            "Velocidade"
#define TR_SD_SECTORS          "Sectores"
#define TR_SD_SIZE             "Tamanho"
#define TR_TYPE                INDENT "Tipe"
#define TR_GLOBAL_VARS         "Variaveis"
#define TR_GVARS               "GLOBAL V."
#define TR_GLOBAL_VAR          "Global Variable"
#define TR_MENU_GLOBAL_VARS      "VARIAVEIS"
#define TR_OWN                 "Propr."
#define TR_DATE                "Data"
#define TR_MONTHS              { "Jan", "Fev", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
#define TR_ROTARY_ENCODER      "R.E."
#define TR_ROTARY_ENC_MODE     TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR    "CHANNELS MONITOR"
#define TR_MIXERS_MONITOR      "MIXERS MONITOR"
#define TR_PATH_TOO_LONG       "Path too long"
#define TR_VIEW_TEXT           "View text"
#define TR_FLASH_BOOTLOADER    "Flash bootloader"
#define TR_FLASH_DEVICE                TR("Flash device","Flash device")
#define TR_FLASH_EXTERNAL_DEVICE "Flash External Device"
#define TR_FLASH_RECEIVER_OTA          "Flash receiver OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash BT module", "Flash Bluetooth module")
#define TR_FLASH_POWER_MANAGEMENT_UNIT          "Flash pwr mngt unit"
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "Device not responding")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Device file prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "Device data refused")
#define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "Device access problem")
#define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "Device file refused")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", "Device file sig.")
#define TR_CURRENT_VERSION             TR("Current vers. ", "Current version: ")
#define TR_FLASH_INTERNAL_MODULE "Flash Internal Module"
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Int. Multi", "Flash Internal Multi")
#define TR_FLASH_EXTERNAL_MODULE       "Flash external module"
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Ext. Multi", "Flash External Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flash External ELRS")
#define TR_FIRMWARE_UPDATE_ERROR TR("FW update Error","Firmware update error")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Flash successful"
#define TR_WRITING             "Writing..."
#define TR_CONFIRM_FORMAT      "Confirm Format?"
#define TR_INTERNALRF          "Internal RF"
#define TR_INTERNAL_MODULE             TR("Int. module","Internal module")
#define TR_EXTERNAL_MODULE             TR("Ext. module","External module")
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX upgrade required"
#define TR_TELEMETRY_DISABLED          "Telem. disabled"
#define TR_MORE_OPTIONS_AVAILABLE      "More options available"
#define TR_NO_MODULE_INFORMATION       "No module information"
#define TR_EXTERNALRF          "External RF"
#define TR_FAILSAFE            "Failsafe mode"
#define TR_FAILSAFESET         "FAILSAFE SETTINGS"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Owner ID"
#define TR_HOLD                "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                "None"
#define TR_NONE_UPPERCASE              "NONE"
#define TR_MENUSENSOR          "SENSOR"
#define TR_POWERMETER_PEAK             "Peak"
#define TR_POWERMETER_POWER            "Power"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Freq."
#define TR_MENUTOOLS                   "TOOLS"
#define TR_TURN_OFF_RECEIVER           "Turn off receiver"
#define TR_STOPPING                    "Stopping..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
#define TR_MENU_POWER_METER            "POWER METER"
#define TR_SENSOR              "SENSOR"
#define TR_COUNTRY_CODE         "Country Code"
#define TR_USBMODE             "USB Mode"
#define TR_JACK_MODE                    "Jack Mode"
#define TR_VOICE_LANGUAGE           "Voice Language"
#define TR_UNITS_SYSTEM         "Units"
#define TR_EDIT                "Edit"
#define TR_INSERT_BEFORE       "Insert Before"
#define TR_INSERT_AFTER        "Insert After"
#define TR_COPY                "Copy"
#define TR_MOVE                "Move"
#define TR_PASTE               "Paste"
#define TR_PASTE_AFTER                 "Paste After"
#define TR_PASTE_BEFORE                "Paste Before"
#define TR_DELETE              "Delete"
#define TR_INSERT              "Insert"
#define TR_RESET_FLIGHT        "Reset Flight"
#define TR_RESET_TIMER1        "Reset Timer1"
#define TR_RESET_TIMER2        "Reset Timer2"
#define TR_RESET_TIMER3        "Reset Timer3"
#define TR_RESET_TELEMETRY     "Reset Telemetry"
#define TR_STATISTICS          "Statistics"
#define TR_ABOUT_US            "About Us"
#define TR_USB_JOYSTICK        "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE    "USB Storage (SD)"
#define TR_USB_SERIAL          "USB Serial (VCP)"
#define TR_SETUP_SCREENS       "Setup screens"
#define TR_MONITOR_SCREENS     "Monitors"
#define TR_AND_SWITCH          "AND Switch"
#define TR_SF                  "CF"
#define TR_GF                  "GF"
#define TR_ANADIAGS_CALIB      "Calibrated analogs"
#define TR_ANADIAGS_FILTRAWDEV "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW  "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX     "Min., max. and range"
#define TR_ANADIAGS_MOVE       "Move analogs to their extremes!"
#define TR_SPEAKER             INDENT"Speaker"
#define TR_BUZZER              INDENT"Buzzer"
#define TR_BYTES               "bytes"
#define TR_MODULE_BIND         "[Bind]"
#define TR_POWERMETER_ATTN_NEEDED      "Attenuator needed"
#define TR_PXX2_SELECT_RX              "Select RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "Select device"
#define TR_DISCOVER             "Discover"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Waiting..."
#define TR_RECEIVER_DELETE             "Delete receiver?"
#define TR_RECEIVER_RESET              "Reset receiver?"
#define TR_SHARE                       "Share"
#define TR_BIND                        "Bind"
#define TR_REGISTER             TR("Reg", "Register")
#define TR_MODULE_RANGE        "[Range]"
#define TR_RANGE_TEST                  "Range test"
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "RECEIVER OPTIONS")
#define TR_RESET_BTN           "[Reset]"
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                BUTTON(TR("SW","Switches"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analogs"))
#define TR_FS_BTN                      BUTTON(TR("F. Switches", "Function switches"))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_SET                 "[Set]"
#define TR_TRAINER             "Trainer"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM      CENTER "TX Antenna problem!"
#define TR_MODELIDUSED         TR("ID used in:", "Receiver ID used in:")
#define TR_MODELIDUNIQUE             "ID is unique"
#define TR_MODULE              "Module"
#define TR_RX_NAME                     "Rx Name"
#define TR_TELEMETRY_TYPE      TR("Type", "Telemetry Type")
#define TR_TELEMETRY_SENSORS   "Sensors"
#define TR_VALUE               "Value"
#define TR_PERIOD              "Period"
#define TR_INTERVAL            "Interval"
#define TR_REPEAT                      "Repeat"
#define TR_ENABLE                      "Enable"
#define TR_DISABLE                     "Disable"
#define TR_TOPLCDTIMER         "Top LCD Timer"
#define TR_UNIT                "Unit"
#define TR_TELEMETRY_NEWSENSOR INDENT "Add new"
#define TR_CHANNELRANGE        INDENT "Channel Range"
#define TR_RXFREQUENCY         TR("Rx Freqency", "Rx Servo Frequency")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_ANTENNACONFIRM1     "EXT. ANTENNA"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES     "Internal","Ask","Per model","Internal + External"
#else
  #define TR_ANTENNA_MODES     "Internal","Ask","Per model","External"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Use int. antenna", "Use internal antenna")
#define TR_USE_EXTERNAL_ANTENNA        TR("Use ext. antenna", "Use external antenna")
#define TR_ANTENNACONFIRM2     TR("Check antenna", "Make sure antenna is installed!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1                "Requires non"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1         "Requires FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1          "Requires EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2                "certified firmware"
#define TR_LOWALARM            INDENT "Low Alarm"
#define TR_CRITICALALARM       INDENT "Critical Alarm"
#define TR_RSSIALARM_WARN             TR("RSSI","TELEMETRY RSSI")
#define TR_NO_RSSIALARM                TR(INDENT "Alarms disabled", INDENT "Telemetry alarms disabled")
#define TR_DISABLE_ALARM               TR(INDENT "Disable alarms", INDENT "Disable telemetry alarms")
#define TR_POPUP               "Popup"
#define TR_MIN                 "Min"
#define TR_MAX                 "Max"
#define TR_CURVE_PRESET        "Preset..."
#define TR_PRESET              "Preset"
#define TR_MIRROR              "Mirror"
#define TR_CLEAR               "Clear"
#define TR_RESET               "Reset"
#define TR_RESET_SUBMENU       "Reset..."
#define TR_COUNT               "Count"
#define TR_PT                  "pt"
#define TR_PTS                 "pts"
#define TR_SMOOTH              "Smooth"
#define TR_COPY_STICKS_TO_OFS  TR("Cpy stick->subtrim", "Copy Sticks To Offset")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
#define TR_COPY_TRIMS_TO_OFS   TR("Cpy trim->subtrim", "Copy trims to subtrim")
#define TR_INCDEC              "Inc/Decrement"
#define TR_GLOBALVAR           "Global Var"
#define TR_MIXSOURCE           "Mixer Source"
#define TR_CONSTANT            "Constant"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "Off","On","Auto"
#define TR_PREFLIGHT           "Preflight Checks"
#define TR_CHECKLIST           INDENT "Display Checklist"
#define TR_AUX_SERIAL_MODE     "Serial port"
#define TR_AUX2_SERIAL_MODE    "Serial port 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port power"
#define TR_SCRIPT              "Script"
#define TR_INPUTS              "Inputs"
#define TR_OUTPUTS             "Outputs"
#define STR_EEBACKUP           "Backup the EEPROM"
#define STR_FACTORYRESET       "Factory reset"
#define TR_CONFIRMRESET        "Erase ALL models and settings?"
#define TR_TOO_MANY_LUA_SCRIPTS "Too many Lua scripts!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "No Telemetry Screens"
#define TR_TOUCH_PANEL                 "Touch panel:"
#define TR_FILE_SIZE                   "File size"
#define TR_FILE_OPEN                   "Open anyway?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Name"
#define TR_PHASES_HEADERS_SW           "Switch"
#define TR_PHASES_HEADERS_RUD_TRIM     "Rudder Trim"
#define TR_PHASES_HEADERS_ELE_TRIM     "Elevator Trim"
#define TR_PHASES_HEADERS_THT_TRIM     "Throttle Trim"
#define TR_PHASES_HEADERS_AIL_TRIM     "Aileron Trim"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Fade In"
#define TR_PHASES_HEADERS_FAD_OUT      "Fade Out"

#define TR_LIMITS_HEADERS_NAME         "Name"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Direction"
#define TR_LIMITS_HEADERS_CURVE        "Curve"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Center"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Subtrim mode"
#define TR_INVERTED                    "Inverted"


#define TR_LSW_DESCRIPTIONS            { "Comparison type or function", "First variable", "Second variable or constant", "Second variable or constant", "Additional condition for line to be enabled", "Minimum ON duration of the logical switch", "Minimum TRUE duration for the switch to become ON" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "First channel"
  #define TR_FILL_BACKGROUND           "Fill background?"
  #define TR_BG_COLOR                  "BG Color"
  #define TR_SLIDERS_TRIMS             "Sliders+Trims"
  #define TR_SLIDERS                   "Sliders"
  #define TR_FLIGHT_MODE               "Flight mode"
  #define TR_INVALID_FILE              "Invalid File"
  #define TR_TIMER_SOURCE              "Timer source"
  #define TR_SIZE                      "Size"
  #define TR_SHADOW                    "Shadow"
  #define TR_ALIGN_LABEL               "Alinhar legenda"
  #define TR_ALIGN_VALUE               "Alinhar valor"
  #define TR_ALIGN_OPTS                { "Esquerda", "Centro", "Direita" }
  #define TR_TEXT                      "Text"
  #define TR_COLOR                     "Color"
  #define TR_MAIN_VIEW_X               "Main view XX"
  #define TR_PANEL1_BACKGROUND         "Panel1 background"
  #define TR_PANEL2_BACKGROUND         "Panel2 background"
  #define TR_WIDGET_GAUGE              "Gauge"
  #define TR_WIDGET_MODELBMP           "Models"
  #define TR_WIDGET_OUTPUTS            "Outputs"
  #define TR_WIDGET_TEXT               "Text"
  #define TR_WIDGET_TIMER              "Timer"
  #define TR_WIDGET_VALUE              "Value"
#endif

// Bootloader common - Ascii only
#define TR_BL_USB_CONNECT_BOOT        CENTER "\011USB Connected"
#define TR_BL_USB_CONNECTED           "USB Connected"
#define TR_BL_USB_PLUGIN              "Or plug in a USB cable"
#define TR_BL_USB_MASS_STORE          "for mass storage"
#define TR_BL_USB_PLUGIN_MASS_STORE   "Or plug in a USB cable for mass storage"
#define TR_BL_WRITE_FW                "Write Firmware"
#define TR_BL_FORK                    "Fork:"
#define TR_BL_VERSION                 "Version:"
#define TR_BL_RADIO                   "Radio:"
#define TR_BL_EXIT                    "Exit"
#define TR_BL_DIR_MISSING             " Directory is missing"
#define TR_BL_DIR_EMPTY               " Directory is empty"

#if LCD_W >= 480
  #define TR_BL_INVALID_FIRMWARE       "Not a valid firmware file"
#elif LCD_W >= 212
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN_MASS_STORE
  #define TR_BL_HOLD_ENTER_TO_START    "\012Hold [ENT] to start writing"
  #define TR_BL_INVALID_FIRMWARE       "\011Not a valid firmware file!        "
  #define TR_BL_INVALID_EEPROM         "\011Not a valid EEPROM file!          "
#else
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
  #define TR_BL_HOLD_ENTER_TO_START    "\006Hold [ENT] to start"
  #define TR_BL_INVALID_FIRMWARE       "\004Not a valid firmware!        "
  #define TR_BL_INVALID_EEPROM         "\004Not a valid EEPROM!          "
#endif

#if defined(PCBTARANIS)
   // Bootloader Taranis specific - Ascii only
  #define TR_BL_RESTORE_EEPROM        "Restore EEPROM"
  #define TR_BL_WRITING_FW            CENTER "\015Writing..."
  #define TR_BL_WRITING_COMPL         CENTER "\007Writing complete"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Press the power button."
    #define TR_BL_FLASH_EXIT          "Exit the flashing mode."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - Ascii only
  #define TR_BL_WRITING_FW            "Writing Firmware ..."
  #define TR_BL_WRITING_COMPL         "Writing Completed"
  #define TR_BL_SELECT_KEY            "[ENT] to select file"
  #define TR_BL_FLASH_KEY             "Hold [ENT] long to flash"
  #define TR_BL_EXIT_KEY              "[RTN] to exit"
#elif defined(PCBNV14)
   // Bootloader NV14 specific - Ascii only
  #define TR_BL_WRITING_FW            "Writing Firmware ..."
  #define TR_BL_WRITING_COMPL         "Writing Completed"
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] to select file"
  #define TR_BL_FLASH_KEY             "Hold [R TRIM] long to flash"
  #define TR_BL_EXIT_KEY              " [L TRIM] to exit"
  #define TR_BL_ENABLE                "Enable"
  #define TR_BL_DISABLE               "Disable"
#endif

// About screen
#define TR_ABOUTUS                     TR("ABOUT", " ABOUT ")

#define TR_CHR_HOUR   'h'
#define TR_CHR_INPUT  'I'   // Values between A-I will work

#define TR_BEEP_VOLUME         "Beep Volume"
#define TR_WAV_VOLUME          "Wav Volume"
#define TR_BG_VOLUME           "Bg Volume"

#define TR_TOP_BAR             "Top Bar"
#define TR_FLASH_ERASE                 "Flash erase..."
#define TR_FLASH_WRITE                 "Flash write..."
#define TR_OTA_UPDATE                  "OTA update..."
#define TR_MODULE_RESET                "Module reset..."
#define TR_UNKNOWN_RX                  "Unknown RX"
#define TR_UNSUPPORTED_RX              "Unsupported RX"
#define TR_OTA_UPDATE_ERROR            "OTA update error"
#define TR_DEVICE_RESET                "Device reset..."
#define TR_ALTITUDE            INDENT "Altitude"
#define TR_SCALE               "Scale"
#define TR_VIEW_CHANNELS       "View Channels"
#define TR_VIEW_NOTES          "View Notes"
#define TR_MODEL_SELECT        "Model Select"
#define TR_ID                  "ID"
#define TR_PRECISION           "Precision"
#define TR_RATIO               "Ratio"
#define TR_FORMULA             "Formula"
#define TR_CELLINDEX           "Cell index"
#define TR_LOGS                "Logs"
#define TR_OPTIONS             "Options"
#define TR_FIRMWARE_OPTIONS    "Firmware options"

#define TR_ALTSENSOR           "Alt sensor"
#define TR_CELLSENSOR          "Cell sensor"
#define TR_GPSSENSOR           "GPS sensor"
#define TR_CURRENTSENSOR       "Sensor"
#define TR_AUTOOFFSET          "Auto Offset"
#define TR_ONLYPOSITIVE        "Positive"
#define TR_FILTER              "Filter"
#define TR_TELEMETRYFULL       TR("All slots full!", "All telemetry slots full!")
#define TR_INVERTED_SERIAL     INDENT "Invert"
#define TR_IGNORE_INSTANCE     INDENT "Ignore instance"
#define TR_SHOW_INSTANCE_ID            "Show instance ID"
#define TR_DISCOVER_SENSORS    "Discover new"
#define TR_STOP_DISCOVER_SENSORS "Stop discovery"
#define TR_DELETE_ALL_SENSORS  "Delete all"
#define TR_CONFIRMDELETE       "Really " LCDW_128_480_LINEBREAK "delete all ?"
#define TR_SELECT_WIDGET       "Select widget"
#define TR_WIDGET_FULLSCREEN           "Full screen"
#define TR_REMOVE_WIDGET       "Remove widget"
#define TR_WIDGET_SETTINGS     "Widget settings"
#define TR_REMOVE_SCREEN       "Remove screen"
#define TR_SETUP_WIDGETS       "Setup widgets"
#define TR_USER_INTERFACE      "User interface"
#define TR_THEME               "Theme"
#define TR_SETUP               "Setup"
#define TR_LAYOUT              "Layout"
#define TR_ADD_MAIN_VIEW         "Add main view"
#define TR_BACKGROUND_COLOR    "Background color"
#define TR_MAIN_COLOR          "Main color"
#define TR_BAR2_COLOR                  "Secondary bar color"
#define TR_BAR1_COLOR                  "Main bar color"
#define TR_TEXT_COLOR                  "Text color"
#define TR_TEXT_VIEWER                 "Text Viewer"

#define TR_MENU_INPUTS          STR_CHAR_INPUT "Inputs"
#define TR_MENU_LUA             STR_CHAR_LUA "Lua scripts"
#define TR_MENU_STICKS           STR_CHAR_STICK "Sticks"
#define TR_MENU_POTS            STR_CHAR_POT "Pots"
#define TR_MENU_MAX             STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI            STR_CHAR_CYC "Cyclic"
#define TR_MENU_TRIMS            STR_CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES        STR_CHAR_SWITCH "Switches"
#define TR_MENU_LOGICAL_SWITCHES  STR_CHAR_SWITCH "Logical Switches"
#define TR_MENU_TRAINER         STR_CHAR_TRAINER "Trainer"
#define TR_MENU_CHANNELS        STR_CHAR_CHANNEL "Channels"
#define TR_MENU_GVARS           STR_CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY       STR_CHAR_TELEMETRY "Telemetry"
#define TR_MENU_DISPLAY        "DISPLAY"
#define TR_MENU_OTHER          "Other"
#define TR_MENU_INVERT         "Invert"
#define TR_AUDIO_MUTE                  TR("Audio mute","Mute if no sound")
#define TR_JITTER_FILTER       "ADC Filter"
#define TR_DEAD_ZONE           "Dead zone"
#define TR_RTC_CHECK           TR("Check RTC", "Check RTC voltage")
#define TR_AUTH_FAILURE        "Auth-failure"
#define TR_RACING_MODE         "Racing mode"

// The following content is Untranslated)

#define TR_USE_THEME_COLOR              "Use theme color"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Add all Trims to Subtrims"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS        "Open Channel Monitor"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Open Channel Mon."
#endif
#define TR_DUPLICATE                    "Duplicate"
#define TR_ACTIVATE                     "Set Active"
#define TR_RED                          "Red"
#define TR_BLUE                         "Blue"
#define TR_GREEN                        "Green"
#define TR_COLOR_PICKER                 "Color Picker"
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
#define TR_THEME_EDITOR                 "THEMES"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Select\nModel"
#define TR_MAIN_MENU_MANAGE_MODELS      "Model\nManager"
#define TR_MAIN_MENU_MODEL_NOTES        "Model\nNotes"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Channel\nMonitor"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Model\nSettings"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Radio\nSettings"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Screens\nSettings"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Reset\nTelemetry"
#define TR_MAIN_MENU_STATISTICS         "Statistics"
#define TR_MAIN_MENU_ABOUT_EDGETX       "About\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "English"
#define TR_VOICE_CHINESE                "Chinese"
#define TR_VOICE_CZECH                  "Czech"
#define TR_VOICE_DANISH                 "Danish"
#define TR_VOICE_DEUTSCH                "Deutsch"
#define TR_VOICE_DUTCH                  "Dutch"
#define TR_VOICE_ESPANOL                "Espanol"
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
#define TR_VOICE_HEBREW                 "Hebraico"

#define TR_USBJOYSTICK_LABEL           "USB Joystick"
#define TR_USBJOYSTICK_EXTMODE         "Mode"
#define TR_VUSBJOYSTICK_EXTMODE        "Classic","Advanced"
#define TR_USBJOYSTICK_SETTINGS        "Channel Settings"
#define TR_USBJOYSTICK_IF_MODE         TR("If. mode","Interface mode")
#define TR_VUSBJOYSTICK_IF_MODE        "Joystick","Gamepad","MultiAxis"
#define TR_USBJOYSTICK_CH_MODE         "Mode"
#define TR_VUSBJOYSTICK_CH_MODE        "None","Btn","Axis","Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Button Mode"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "Normal","Pulse","SWEmu","Delta","Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Norm","Normal"),TR("Puls","Pulse"),TR("SWEm","SWEmul"),TR("Delt","Delta"),TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Positions"
#define TR_VUSBJOYSTICK_CH_SWPOS       "Push","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Axis"
#define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","rotX","rotY","rotZ","Slider","Dial","Wheel"
#define TR_USBJOYSTICK_CH_SIM          "Sim axis"
#define TR_VUSBJOYSTICK_CH_SIM         "Ail","Ele","Rud","Thr"
#define TR_USBJOYSTICK_CH_INVERSION    "Inversion"
#define TR_USBJOYSTICK_CH_BTNNUM       "Button no."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Button no. collision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Axis collision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "None","X-Y, Z-rX","X-Y, rX-rY"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Apply changes"

#define TR_DIGITAL_SERVO          "Servo333HZ"
#define TR_ANALOG_SERVO           "Servo 50HZ"
#define TR_SIGNAL_OUTPUT          "Signal output"
#define TR_SERIAL_BUS             "Serial bus"
#define TR_SYNC                   "Sync"

#define TR_ENABLED_FEATURES       "Enabled Features"
#define TR_RADIO_MENU_TABS        "Radio Menu Tabs"
#define TR_MODEL_MENU_TABS        "Model Menu Tabs"
