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



#define TR_OFFON               "OFF","ON"
#define TR_MMMINV              "---","INV"
#define TR_VBEEPMODE           "Silencio","Alarma","No tecla","Todo"
#define TR_VBLMODE             "OFF","Teclas","Sticks","Ambos","ON"
#define TR_TRNMODE             "OFF","+=",":="
#define TR_TRNCHN              "CH1","CH2","CH3","CH4"

#define TR_AUX_SERIAL_MODES    "UIT","Telem Mirror","Telemetría","Entrenador SBUS","LUA","CLI","GPS","Debug"
#define TR_SWTYPES             "Nada","Palanca","2POS","3POS"
#define TR_POTTYPES            "Nada",TR("Pot con fij","Pot con fijador"),TR("Multipos","Switch multipos"),"Pot"
#define TR_SLIDERTYPES         "Nada","Slider"
#define TR_VPERSISTENT         "OFF","Vuelo","Reset manual"
#define TR_COUNTRY_CODES       TR("US","América"),TR("JP","Japón"),TR("EU", "Europa")
#define TR_USBMODES            "Pregunta","Joystick","SDCard","Serie"
#define TR_JACK_MODES          "Pregunta","Audio","Entrena."
#define TR_TELEMETRY_PROTOCOLS "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_TELEMETRY_MODE        "Off","On","Off+Aux","On+Aux"
#define TR_MULTI_CUSTOM        "Custom"
#define TR_VTRIMINC            TR("Expo","Exponencial"),TR("ExFino","Extra fino")"Fino","Medio","Grueso"
#define TR_VDISPLAYTRIMS       "No","Cambiar","Si"
#define TR_VBEEPCOUNTDOWN      "Silencio","Beeps","Voz","Haptic"
#define TR_COUNTDOWNVALUES     "5s","10s","20s","30s"
#define TR_VVARIOCENTER        "Tono","Silencio"
#define TR_CURVE_TYPES         "Normal","Custom"
#define TR_ADCFILTERVALUES     "Global","Off","On"

#if defined(PCBX10)
  #define TR_RETA123           "R","E","T","A","1","3","2","4","5","6","7","L","R"
#elif defined(PCBHORUS)
  #define TR_RETA123           "R","E","T","A","1","3","2","4","5","L","R"
#elif defined(PCBX9E)
  #define TR_RETA123           "R","E","T","A","1","2","3","4","L","R","L","R"
#elif defined(PCBTARANIS)
  #define TR_RETA123           "R","E","T","A","1","2","3","L","R"
#else
  #define TR_RETA123           "R","E","T","A","1","2","3"
#endif

#define TR_VCURVEFUNC          "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX              "Añadir","Multipl.","Cambiar"
#define TR_VMLTPX2             "+=","*=",":="

#if defined(PCBHORUS)
  #define TR_VMIXTRIMS         "OFF","ON","Rud","Ele","Thr","Ail","T5","T6"
#else
  #define TR_VMIXTRIMS         "OFF","ON","Rud","Ele","Thr","Ail"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER          "Timer"
  #define TR_CSWSTICKY         "Pega"
  #define TR_CSWRANGE          "Rango"
  #define TR_CSWSTAY           "Borde"
#else
  #define TR_CSWTIMER          "Tim"
  #define TR_CSWSTICKY         "Pega"
  #define TR_CSWRANGE          "Rngo"
  #define TR_CSWSTAY           "Bord"
#endif

#define TR_CSWEQUAL      "a=x"

#define TR_VCSWFUNC      "---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#if defined(VARIO)
  #define TR_VVARIO            "Vario"
#else
  #define TR_VVARIO            "[Vario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND             "Oir sonido"
#else
  #define TR_SOUND             "Beep"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC            "Haptic"
#else
  #define TR_HAPTIC            "[Haptic]"
#endif

#if defined(VOICE)
  #define TR_PLAY_TRACK        "Oir pista"
  #define TR_PLAY_BOTH         "Oir Ambos"
  #define TR_PLAY_VALUE        "Oir valor"
#else
  #define TR_PLAY_TRACK        "[OirPista]"
  #define TR_PLAY_BOTH         "[OirAmbos]"
  #define TR_PLAY_VALUE        "[OirValor]"
#endif

#define TR_SF_BG_MUSIC         "BgMúsica","BgMúsica||"

#if defined(SDCARD)
  #define TR_SDCLOGS           "SD Logs"
#else
  #define TR_SDCLOGS           "[SD Logs]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR       "Ajuste"
#else
  #define TR_ADJUST_GVAR       "[AjusteGV]"
#endif

#if defined(LUA)
  #define TR_SF_PLAY_SCRIPT   "Lua Script"
#else
  #define TR_SF_PLAY_SCRIPT   "[Lua]"
#endif

#if defined(DEBUG)
  #define TR_SF_TEST          ,"Test"
#else
  #define TR_SF_TEST
#endif

#if defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY        "Seguro"
#else
  #define TR_SF_SAFETY        "---"
#endif

#define TR_SF_SCREENSHOT      "Captura"
#define TR_SF_RACING_MODE     "RacingMode"
#define TR_SF_DISABLE_TOUCH   "No Touch"
#define TR_SF_RESERVE         "[reserv.]"

#define TR_VFSWFUNC            TR_SF_SAFETY,"Entrenador","Inst. Trim","Reset","Ajuste",TR_ADJUST_GVAR,"Volumen","Failsafe","CheckRango","Enl.módulo",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,"Luz Fondo",TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH TR_SF_TEST

#define TR_FSW_RESET_TELEM     TR("Telm", "Telemetría")

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS  "Timer 1","Timer 2","Timer 3"
#else
  #define TR_FSW_RESET_TIMERS  "Tmr1","Tmr2","Tmr3"
#endif

#define TR_VFSWRESET           TR_FSW_RESET_TIMERS,TR("Todo","Vuelo"),TR_FSW_RESET_TELEM

#define TR_FUNCSOUNDS          TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Avs1","Aviso1"),TR("Avs2","Aviso2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP        "ft"
#define SPEED_UNIT_IMP         "mph"
#define LENGTH_UNIT_METR       "m"
#define SPEED_UNIT_METR        "kmh"

#define TR_VUNITSSYSTEM        "Métrico","Imperial"
#define TR_VTELEMUNIT          "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km"

#define STR_V                  (STR_VTELEMUNIT[1])
#define STR_A                  (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE    "Nada","Números","Barras","Script"
#define TR_GPSFORMAT           "HMS","NMEA"
#define TR_TEMPLATE_CLEAR_MIXES        "Elim Mezcla"
#define TR_TEMPLATE_SIMPLE_4CH         "Simple 4-CH"
#define TR_TEMPLATE_STICKY_TCUT        "Anular Motor"
#define TR_TEMPLATE_VTAIL              "Cola en V"
#define TR_TEMPLATE_DELTA              "Elevon\\Delta"
#define TR_TEMPLATE_ECCPM              "eCCPM"
#define TR_TEMPLATE_HELI               "Heli Setup"
#define TR_TEMPLATE_SERVO_TEST         "Servo Test"

#define TR_VSWASHTYPE          "---","120","120X","140","90"

#define TR_STICKS_VSRCRAW              STR_CHAR_STICK "Rud", STR_CHAR_STICK "Ele", STR_CHAR_STICK "Thr", STR_CHAR_STICK "Ail"

#if defined(PCBHORUS)
  #define TR_TRIMS_VSRCRAW             STR_CHAR_TRIM "Rud", STR_CHAR_TRIM "Ele", STR_CHAR_TRIM "Thr", STR_CHAR_TRIM "Ail", STR_CHAR_TRIM "T5", STR_CHAR_TRIM "T6"
#else
  #define TR_TRIMS_VSRCRAW             STR_CHAR_TRIM "Rud", STR_CHAR_TRIM "Ele", STR_CHAR_TRIM "Thr", STR_CHAR_TRIM "Ail"
#endif

#if defined(PCBHORUS)
  #define TR_TRIMS_SWITCHES            STR_CHAR_TRIM "Rl", STR_CHAR_TRIM "Rr", STR_CHAR_TRIM "Ed", STR_CHAR_TRIM "Eu", STR_CHAR_TRIM "Td", STR_CHAR_TRIM "Tu", STR_CHAR_TRIM "Al", STR_CHAR_TRIM "Ar", STR_CHAR_TRIM "5d", STR_CHAR_TRIM "5u", STR_CHAR_TRIM "6d", STR_CHAR_TRIM "6u"
#else
  #define TRIM_SWITCH_CHAR             TR("t",STR_CHAR_TRIM)
  #define TR_TRIMS_SWITCHES            TRIM_SWITCH_CHAR "Rl", TRIM_SWITCH_CHAR "Rr", TRIM_SWITCH_CHAR "Ed", TRIM_SWITCH_CHAR "Eu", TRIM_SWITCH_CHAR "Td", TRIM_SWITCH_CHAR "Tu", TRIM_SWITCH_CHAR "Al", TRIM_SWITCH_CHAR "Ar"
#endif

#if defined(PCBHORUS) || defined(PCBNV14)
  #define TR_VKEYS                     "PGUP","PGDN","ENTER","MDL","RTN","TELE","SYS"
#elif defined(PCBXLITE)
  #define TR_VKEYS                     "Shift","Exit","Enter","Down","Up","Right","Left"
#elif defined(RADIO_FAMILY_JUMPER_T12)
  #define TR_VKEYS                     "Exit","Enter","Down","Up","Right","Left"
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define TR_VKEYS                     "Exit","Enter","PGUP","PGDN","SYS","MDL","TELE"
#elif defined(RADIO_T8)
  #define TR_VKEYS                     "RTN","ENTER","PGUP","PGDN","SYS","MDL","UP","DOWN"
#elif defined(RADIO_ZORRO)
  #define TR_VKEYS                     "RTN","ENTER","PGUP","PGDN","SYS","MDL","TELE"
#elif defined(PCBTARANIS)
  #define TR_VKEYS                     "Menu","Exit","Enter","Page","Plus","Minus"
#else
  #define TR_VKEYS                     "Menu","Exit","Down","Up","Right","Left"
#endif

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Default","High"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES     "ON","One"

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Normal","Inverted"
#else
  #define TR_ROTARY_ENC_OPT         "Normal","Inverted","V-I H-N","V-I H-A"
#endif

#if defined(IMU)
  #define TR_IMU_VSRCRAW       "TltX","TltY",
#else
  #define TR_IMU_VSRCRAW
#endif

#if defined(HELI)
  #define TR_CYC_VSRCRAW       "CYC1","CYC2","CYC3"
#else
  #define TR_CYC_VSRCRAW       "[C1]","[C2]","[C3]"
#endif

#define TR_RESERVE_VSRCRAW   "[--]"
#define TR_EXTRA_VSRCRAW     "Batt","Time","GPS",TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,"Tmr1","Tmr2","Tmr3"

#define TR_VTMRMODES                   "OFF","ABS","THs","TH%","THt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Master/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Esclav/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Master/Módulo SBUS"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Master/Módulo CPPM"
#define TR_VTRAINER_MASTER_BATTERY     "Master/Serie"
#define TR_VTRAINER_BLUETOOTH          TR("Master/BT","Master/Bluetooth"),TR("Esclavo/BT","Esclavo/Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VFAILSAFE                   "No","Hold","Custom","No pulsos","Receptor"
#define TR_VSENSORTYPES                "Custom","Calculado"
#define TR_VFORMULAS                   "Suma","Media","Min","Máx","Multipl.","Total","Cell","Consumo","Distancia"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Menor","1","2","3","4","5","6","Mayor","Delta"
#define TR_TEXT_SIZE                   "Normal","Muy pequeño","Pequeño","Medio","Doble"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center only)","= (symetrical)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")

#if defined(COLORLCD)
  #define INDENT
  #define LEN_INDENT                   3
  #define INDENT_WIDTH                 12
  #define BREAKSPACE                   "\036"
#else
  #define INDENT                       "\001"
  #define LEN_INDENT                   1
  #define INDENT_WIDTH                 (FW/2)
  #define BREAKSPACE                   " "
#endif

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

#define TR_MENUWHENDONE        CENTER "\007" TR_ENTER " AL ACABAR "
#define TR_FREE                "libre"
#define TR_YES                         "Yes"
#define TR_NO                          "No"
#define TR_DELETEMODEL         "BORRAR MODELO"
#define TR_COPYINGMODEL        "Copiando modelo.."
#define TR_MOVINGMODEL         "Moviendo modelo..."
#define TR_LOADINGMODEL        "Cargando modelo..."
#define TR_NAME                "Nombre"
#define TR_MODELNAME           TR("Nom. modelo", "Nombre modelo")
#define TR_PHASENAME           "Nombre fase "
#define TR_MIXNAME             TR("Nom. mezcla", "Nombre mezcla")
#define TR_INPUTNAME           TR("Entrada", "Nom. entrada")
#define TR_EXPONAME            TR("Nom.", "Nombre expo")
#define TR_BITMAP              "Imagen modelo"
#define TR_TIMER               TR("Timer", "Timer ")
#define TR_START                       "Start"
#define TR_ELIMITS             TR("E.Límite", "Ampliar límites")
#define TR_ETRIMS              TR("E.Trims", "Ampliar trims")
#define TR_TRIMINC             "Paso trim"
#define TR_DISPLAY_TRIMS       "Ver trims"
#define TR_TTRACE              TR("Fuente-A", INDENT "Fuente acelerador")
#define TR_TTRIM               TR("Trim-A", INDENT "Trim acelerador")
#define TR_TTRIM_SW            TR("T-Trim-Sw", INDENT "Trim switch")
#define TR_BEEPCTR             TR("Beep ctr", "Beep centro")
#define TR_USE_GLOBAL_FUNCS    TR("Funcs. glob.", "Usar func. globales")
#define TR_PROTOCOL            TR("Proto", "Protocolo")
#define TR_PPMFRAME            INDENT "Trama PPM"
#define TR_REFRESHRATE         TR(INDENT "Refresco", INDENT "Velocidad refresco")
#define STR_WARN_BATTVOLTAGE   TR(INDENT "Salida es VBAT: ", INDENT "Aviso: señal salida es VBAT: ")
#define TR_WARN_5VOLTS                 "Aviso: nivel de salida 5 voltios"
#define TR_MS                  "ms"
#define TR_FREQUENCY           INDENT "Frequencia"
#define TR_SWITCH              TR("Interr.", "Interruptor")
#define TR_TRIMS               "Trims"
#define TR_FADEIN              "Inicio"
#define TR_FADEOUT             "Final"
#define TR_DEFAULT             "(defecto)"
#define TR_CHECKTRIMS          CENTER "\006Check\012Trims"
#define OFS_CHECKTRIMS         CENTER_OFS+(9*FW)
#define TR_SWASHTYPE           "Tipo cíclico"
#define TR_COLLECTIVE          TR("Colectivo", "Fuente colectivo")
#define TR_AILERON             TR("Col. lateral", "Fuente col. lateral")
#define TR_ELEVATOR            TR("Col. long. ", "Fuente col. longitudinal")
#define TR_SWASHRING           "Ciclico"
#define TR_ELEDIRECTION        TR("ELE Dirección", "Largo cyc. dirección")
#define TR_AILDIRECTION        TR("AIL Dirección", "Lateral cyc. dirección")
#define TR_COLDIRECTION        TR("PIT Dirección", "Coll. pitch dirección")
#define TR_MODE                "Modo"
#define TR_SUBTYPE             INDENT "Subtipo"
#define TR_NOFREEEXPO          "No expo libre!"
#define TR_NOFREEMIXER         "No mezcla lib!"
#define TR_SOURCE              "Fuente"
#define TR_WEIGHT              "Cantidad"
#define TR_EXPO                TR("Expo", "Exponencial")
#define TR_SIDE                "Zona"
#define TR_DIFFERENTIAL        "Diferenc"
#define TR_OFFSET              "Offset"
#define TR_TRIM                "Trim"
#define TR_DREX                "DRex"
#define DREX_CHBOX_OFFSET      30
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
#define TR_MIXES               "MIXES"
#define TR_CV                  "CV"
#if defined(PCBNV14)
#define TR_GV                  "GV"
#else
#define TR_GV                  TR("G", "GV")
#endif
#define TR_ACHANNEL            "A\004canal"
#define TR_RANGE               INDENT"Alcance"
#define TR_CENTER              INDENT "Centro"
#define TR_BAR                 "Bar"
#define TR_ALARM               "Alarma"
#define TR_USRDATA             "UsrData"
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
#define TR_MODEL_STILL_POWERED "Modelo aun encendido"
#define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_MODEL_SHUTDOWN              "Apagar ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Enter para confirmar"
#define TR_THROTTLE_LABEL      "Potencia"
#define TR_THROTTLEREVERSE     TR("Invert acel", INDENT "Invertir acel.")
#define TR_MINUTEBEEP          TR("Minuto", "Cada minuto")
#define TR_BEEPCOUNTDOWN       TR(INDENT"Cta. atrás", INDENT"Cuenta atrás")
#define TR_PERSISTENT          TR(INDENT"Persisten.", INDENT"Persistente")
#define TR_BACKLIGHT_LABEL     "Luz fondo"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLDELAY             INDENT"Duración"
#define TR_BLONBRIGHTNESS      INDENT"MAS brillo"
#define TR_BLOFFBRIGHTNESS     INDENT"MENOS brillo"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR             INDENT "Color"
#define TR_SPLASHSCREEN        "Pantalla inicio"
#define TR_PWR_ON_DELAY        TR("Atraso On", "Atraso encendido")
#define TR_PWR_OFF_DELAY       TR("Atraso Off", "Atraso apagado")
#define TR_THROTTLE_WARNING    TR("Aviso-A", INDENT "Aviso acelerador")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Cust-Pos", INDENT INDENT INDENT INDENT "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING       TR("Aviso-I", INDENT "Aviso interruptor")
#define TR_POTWARNINGSTATE     TR(INDENT "Pot&Slid.", INDENT "Pots & sliders")
#define TR_SLIDERWARNING       TR(INDENT "Pos. slid.", INDENT "Posiciones slider")
#define TR_POTWARNING          TR("Aviso pot", INDENT "Aviso pot")
#define TR_TIMEZONE            TR("Zona horaria", INDENT "Zona horaria")
#define TR_ADJUST_RTC          TR("Ajustar RTC", "Ajustar RTC")
#define TR_GPS                 "GPS"
#define TR_RXCHANNELORD        TR("Rx Orden canal", "Orden habitual canales")
#define TR_STICKS              "Sticks"
#define TR_POTS                "Pots"
#define TR_SWITCHES            "Switches"
#define TR_SWITCHES_DELAY      TR("Atraso switch.", "Atraso switches")
#define TR_SLAVE               "Esclavo"
#define TR_MODESRC             "Modo\006% Fuente"
#define TR_MULTIPLIER          "Multiplicar"
#define TR_CAL                 "Cal"
#define TR_VTRIM               "Trim - +"
#define TR_BG                  "BG:"
#define TR_CALIB_DONE          "Calibration completed"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART       "Presiona [Enter] para empezar"
  #define TR_SETMIDPOINT       "Centra sticks/pots/sliders y presiona [Enter]"
  #define TR_MOVESTICKSPOTS    "Mueve sticks, pots and sliders y presiona [Enter]"
#else
  #define TR_MENUTOSTART       CENTER "\010" TR_ENTER " EMPEZAR"
  #define TR_SETMIDPOINT       TR(CENTER "\007STICKS AL CENTRO",CENTER "\010STICKS AL CENTRO")
  #define TR_MOVESTICKSPOTS    CENTER "\006MOVER STICKS/POTS"
#endif
#define TR_RXBATT              "Rx Bat.:"
#define TR_TXnRX               "Tx:\0Rx:"
#define OFS_RX                 4
#define TR_ACCEL               "Acc:"
#define TR_NODATA              CENTER "SIN DATOS"
#define TR_US                         "us"
#define TR_HZ                         "Hz"
#define TR_TMIXMAXMS                  "Tmix máx"
#define TR_FREE_STACK                 "Stack libre"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL          "Lua scripts"
#define TR_FREE_MEM_LABEL             "Free mem"
#define TR_TIMER_LABEL                "Timer"
#define TR_THROTTLE_PERCENT_LABEL     "Throttle %"
#define TR_BATT_LABEL                 "Battery"
#define TR_SESSION                    "Session"
#define TR_MENUTORESET         TR_ENTER "Resetear"
#define TR_PPM_TRAINER         "TR"
#define TR_CH                  "CH"
#define TR_MODEL               "MODELO"
#define TR_FM                  "FM"
#define TR_MIX                 "MIX"
#define TR_EEPROMLOWMEM        "EEPROM mem.baja"
#define TR_PRESS_ANY_KEY_TO_SKIP   "Pulsar tecla para omitir"
#define TR_THROTTLE_NOT_IDLE     "Aceler. Activado"
#define TR_ALARMSDISABLED      "Alarmas Desact."
#define TR_PRESSANYKEY         TR("\010Pulsa una tecla", "Pulsa una tecla")
#define TR_BADEEPROMDATA       "Error datos EEPROM"
#define TR_BAD_RADIO_DATA      "Error datos radio"
#define TR_EEPROMFORMATTING    "Formateo EEPROM"
#define TR_STORAGE_FORMAT      "Preparación alamacenamiento"
#define TR_EEPROMOVERFLOW      "Desborde EEPROM"
#define TR_RADIO_SETUP      "CONFIGURACIÓN"
#define TR_MENUDATEANDTIME     "FECHA Y HORA"
#define TR_MENUTRAINER         "ENTRENADOR"
#define TR_MENUSPECIALFUNCS    "FUNCIONES GLOBALES"
#define TR_MENUVERSION         "VERSIÓN"
#define TR_MENU_RADIO_SWITCHES TR("INTERUPTS", "TEST INTERRUPTORES")
#define TR_MENU_RADIO_ANALOGS  TR("ANALÓGICOS", "TEST ANALÓGICOS")
#define TR_MENU_RADIO_ANALOGS_CALIB    "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION     "CALIBRACIÓN"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS       "Trims => Offsets"
#else
  #define TR_TRIMS2OFFSETS       "\006Trims => Offsets"
#endif
#define TR_CHANNELS2FAILSAFE   "Canales=>Failsafe"
#define TR_CHANNEL2FAILSAFE    "Canal=>Failsafe"
#define TR_MENUMODELSEL        TR("MODELOS", "SELECCIÓN MODELO")
#define TR_MENU_MODEL_SETUP            TR("SETUP", "MODEL SETUP")
#define TR_MENUSETUP           TR("CONFIG. MODELO", "CONFIGURACIÓN MODELO")
#define TR_MENUFLIGHTMODE      "MODO DE VUELO"
#define TR_MENUFLIGHTMODES     "MODOS DE VUELO"
#define TR_MENUHELISETUP       "CONFIGURACIÓN HELI"


// Alignment

#if defined(PPM_CENTER_ADJUSTABLE) || defined(PPM_LIMITS_SYMETRICAL) // The right menu titles for the gurus ...
  #define TR_MENUINPUTS        "STICKS"
  #define TR_MENULIMITS        "SERVOS"
#else
  #define TR_MENUINPUTS        "DR/EXPO"
  #define TR_MENULIMITS        "LÍMITES"
#endif

#define TR_MENUCURVES          "CURVAS"
#define TR_MENUCURVE           "CURVA"
#define TR_MENULOGICALSWITCH   "INTERRUP.LÓGICO"
#define TR_MENULOGICALSWITCHES TR3("INTERRUP. LÓGICOS", "INTERRUP. LÓGICOS", "INTERRUPTORES LÓGICOS")
#define TR_MENUCUSTOMFUNC      TR("FUNCIONES", "FUNCIONES ESPECIALES")
#define TR_MENUCUSTOMSCRIPTS   "CUSTOM SCRIPTS"
#define TR_MENUTELEMETRY       "TELEMETRÍA"
#define TR_MENUTEMPLATES       "PLANTILLAS"
#define TR_MENUSTAT            TR("ESTAD.", "ESTADÍSTICAS")
#define TR_MENUDEBUG           "DEBUG"
#define TR_MONITOR_CHANNELS1   "MONITOR CANALES 1/8"
#define TR_MONITOR_CHANNELS2   "MONITOR CANALES 9/16"
#define TR_MONITOR_SWITCHES    "MONITOR INTERRP LÓGICOS"
#define TR_MONITOR_CHANNELS3   "MONITOR CANALES 17/24"
#define TR_MONITOR_CHANNELS4   "MONITOR CANALES 25/32"
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
#define TR_MULTI_AUTOBIND              TR(INDENT "Emp Cnl",INDENT "Emparejar en canal")
#define TR_DISABLE_CH_MAP              TR("No ch map", "Desactivar mapa cnl")
#define TR_DISABLE_TELEM               TR("No telem", "Desactivar telem.")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "Autodetect", INDENT "Autodetectar formato")
#define TR_MULTI_LOWPOWER              TR(INDENT "Baja poten.", INDENT "Modo de baja poten.")
#define TR_MULTI_LNA_DISABLE           INDENT "LNA desact."
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")
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
#define TR_MODULE_STATUS               TR(INDENT "Estado", INDENT "Estado módulo")
#define TR_MODULE_SYNC                 TR(INDENT "Sync", INDENT "Estado proto sync")
#define TR_MULTI_SERVOFREQ             TR("V ref serv", "Vel. refr. servo")
#define TR_MULTI_MAX_THROW             TR("Máx. Throw", "Activar máx. throw")
#define TR_MULTI_RFCHAN                TR("Canal RF", "Selecciona canal RF")
#define TR_SYNCMENU                    "Sync " TR_ENTER
#define TR_LIMIT                       INDENT"Límite"
#define TR_MINRSSI                     "Min Rssi"
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_LATITUDE                    "Latitud"
#define TR_LONGITUDE                   "Longitud"
#define TR_GPS_COORDS_FORMAT           TR("Coords GPS", INDENT "Formato coordenadas")
#define TR_VARIO                       TR("Vario", "Variómetro")
#define TR_PITCH_AT_ZERO               INDENT "Pitch en cero"
#define TR_PITCH_AT_MAX                INDENT "Pitch en máx"
#define TR_REPEAT_AT_ZERO              TR(INDENT "Repet. en cero", INDENT "Repetir en cero")
#define TR_SHUTDOWN                    "APAGANDO"
#define TR_SAVE_MODEL                   "Grabando modelo"
#define TR_BATT_CALIB                  TR("Calib.bat.", "Calibración batería")
#define TR_CURRENT_CALIB               "Calib. actual"
#define TR_VOLTAGE                     INDENT"Voltaje"
#define TR_CURRENT                     INDENT"Actual"
#define TR_SELECT_MODEL                TR("Selec. modelo", "Seleccionar modelo")
#define TR_MODEL_CATEGORIES            "Model Categories"
#define TR_MODELS                      "Models"
#define TR_SELECT_MODE                 "Select mode"
#define TR_CREATE_CATEGORY             "Crear categoría"
#define TR_RENAME_CATEGORY             "Renombrar categoría"
#define TR_DELETE_CATEGORY             "Borrar categoría"
#define TR_CREATE_MODEL                "Crear modelo"
#define TR_DUPLICATE_MODEL             "Duplicar modelo"
#define TR_COPY_MODEL                  "Copiar modelo"
#define TR_MOVE_MODEL                  "Mover modelo"
#define TR_BACKUP_MODEL                TR("Copia sgdad. mod.", "Copia seguridad modelo")
#define TR_DELETE_MODEL                "Borrar modelo"
#define TR_RESTORE_MODEL               "Restaurar modelo"
#define TR_DELETE_ERROR                "Error de borrado"
#define TR_CAT_NOT_EMPTY               "Categoría no esta vacía"
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
#define TR_MODULES_RX_VERSION          "Módulos / versión RX"
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
#define TR_SDCARD_FULL                 "SD Card llena"
#define TR_NEEDS_FILE                  "NECESITA ARCHIVO"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE        "Incompatible"
#define TR_WARNING             "AVISO"
#define TR_EEPROMWARN          "EEPROM"
#define TR_STORAGE_WARNING     "ALMAC."
#define TR_EEPROM_CONVERTING   "Convirtiendo EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD card conversion required"
#define TR_CONVERTING                  "Converting: "
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
#define TR_PCBREV_ERROR        "Error PCB"
#define TR_NO_FAILSAFE         "Failsafe no fijado"
#define TR_KEYSTUCK            "Tecla atascada"
#define TR_INVERT_THR          TR("Invertir acel?", "Invertir acel.?")
#define TR_VOLUME              "Volumen"
#define TR_LCD                 "LCD"
#define TR_BRIGHTNESS          "Brillo"
#define TR_CPU_TEMP            "CPU Temp.\016>"
#define TR_CPU_CURRENT         "Actual\022>"
#define TR_CPU_MAH             "Consumo"
#define TR_COPROC              "CoProc."
#define TR_COPROC_TEMP         "MB Temp. \016>"
#define TR_CAPAWARNING         INDENT "Capacidad baja"
#define TR_TEMPWARNING         INDENT "Sobrecalent"
#define TR_TTL_WARNING         "Warning: use 3.3V logic levels"
#define TR_FUNC                "Función"
#define TR_V1                  "V1"
#define TR_V2                  "V2"
#define TR_DURATION            "Duración"
#define TR_DELAY               "Atraso"
#define TR_SD_CARD             "SD CARD"
#define TR_SDHC_CARD           "SD-HC CARD"
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
#define TR_SD_FORMAT           "Formatear"
#define TR_NA                  "N/A"
#define TR_HARDWARE            "COMPONENTES"
#define TR_FORMATTING          "Formateando.."
#define TR_TEMP_CALIB          "Temp. Calib"
#define TR_TIME                "Hora"
#define TR_MAXBAUDRATE         "Máx bauds"
#define TR_BAUDRATE            "Baudrate"

#define TR_SAMPLE_MODE         "Modo de muestra"
#define TR_SAMPLE_MODES        "Normal","OneBit"
#define TR_SELECT_TEMPLATE_FOLDER "SELECT A TEMPLATE FOLDER:"
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
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES      "---","Activo"
#else
#define TR_BLUETOOTH_MODES      "---","Telemetría","Entrenador"
#endif
#define TR_SD_INFO_TITLE       "SD INFO"
#define TR_SD_TYPE             "Tipo:"
#define TR_SD_SPEED            "Velocidad:"
#define TR_SD_SECTORS          "Sectores:"
#define TR_SD_SIZE             "Tamaño:"
#define TR_TYPE                INDENT "Tipo"
#define TR_GLOBAL_VARS         "Variables globales"
#define TR_GVARS               "V. GLOBAL"
#define TR_GLOBAL_VAR                  "Variable global"
#define TR_MENU_GLOBAL_VARS              "VARIABLES GLOBALES"
#define TR_OWN                         "Propio"
#define TR_DATE                        "Fecha"
#define TR_MONTHS                      { "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic" }
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
#define TR_OPENTX_UPGRADE_REQUIRED     "Actualiz. OpenTX requerida"
#define TR_TELEMETRY_DISABLED          "Telem. inactiva"
#define TR_MORE_OPTIONS_AVAILABLE      "Más opciones disp."
#define TR_NO_MODULE_INFORMATION       "Sin información módulo"
#define TR_EXTERNALRF                  "RF externa"
#define TR_FAILSAFE                    INDENT"Failsafe"
#define TR_FAILSAFESET                 "AJUSTES FAILSAFE"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Pers. ID"
#define TR_PINMAPSET                   "PINMAP"
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
#define TR_SPEAKER             INDENT"Altavoz"
#define TR_BUZZER              INDENT"Zumbador"
#define TR_BYTES               "bytes"
#define TR_MODULE_BIND         "[Enl.]"
#define TR_POWERMETER_ATTN_NEEDED      "Necesita atenuador"
#define TR_PXX2_SELECT_RX              "Selecciona RX..."
#define TR_PXX2_DEFAULT                "<defecto>"
#define TR_BT_SELECT_DEVICE            "Selecciona dispositivo"
#define TR_DISCOVER             "Buscar"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Espera..."
#define TR_RECEIVER_DELETE             "Borrar receptor?"
#define TR_RECEIVER_RESET              "Reset receptor?"
#define TR_SHARE                       "Compartido"
#define TR_BIND                        "Emparejar"
#define TR_REGISTER             TR("Reg", "Registrar")
#define TR_MODULE_RANGE        "[Lim.]"
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "OPCIONES RECEPTOR")
#define TR_DEL_BUTTON                  BUTTON(TR("Del", "Borrar"))
#define TR_RESET_BTN           "[Reset]"
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                BUTTON(TR("SW","Switches"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analogs"))
#define TR_TOUCH_NOTFOUND              "Hardware táctil no encontrado"
#define TR_TOUCH_EXIT                  "Tocar pantalla para salir"
#define TR_CALIBRATION                   "Calibración"
#define TR_SET                 "[Ajuste]"
#define TR_TRAINER             "Entrenador"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM      CENTER "Problema antena TX!"
#define TR_MODELIDUSED         TR("ID en uso", "ID modelo en uso")
#define TR_MODULE              "Módulo"
#define TR_RX_NAME                     "Nombre Rx"
#define TR_TELEMETRY_TYPE      TR("Tipo", "Tipo telemetría")
#define TR_TELEMETRY_SENSORS   "Sensores"
#define TR_VALUE               "Valor"
#define TR_REPEAT                      "Repeat"
#define TR_ENABLE                      "Enable"
#define TR_TOPLCDTIMER         "Timer LCD superior"
#define TR_UNIT                "Unidad"
#define TR_TELEMETRY_NEWSENSOR INDENT "Añadir sensor..."
#define TR_CHANNELRANGE        INDENT "Canales"
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequencia")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetría")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_RXFREQUENCY         TR("Rx Freqency", "Rx Servo Frequency")
#define TR_ANTENNACONFIRM1     "ANTENA EXT."
#if defined(PCBX12S)
#define TR_ANTENNA_MODES       "Interna","Preguntar","Por modelo","Interno + Externo"
#else
#define TR_ANTENNA_MODES       "Interna","Preguntar","Por modelo","Externa"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Usa antena int.", "Usa antena interna")
#define TR_USE_EXTERNAL_ANTENNA        TR("Use antena ext.", "Usa antena externa")
#define TR_ANTENNACONFIRM2     TR("Revisa antena", "Revisa que la antena esté instalada!")

#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1        "No requerido"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1         "Requiere FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1          "Requiere EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2             "firmware certificado"
#define TR_LOWALARM            INDENT "Alarma baja"
#define TR_CRITICALALARM       INDENT "Alarma crítica"
#define TR_RSSIALARM_WARN      TR("RSSI", "TELEMETRY RSSI")
#define TR_NO_RSSIALARM        TR(INDENT "Alarmas desact.", INDENT "Alarmas telemetría desact.")
#define TR_DISABLE_ALARM       TR(INDENT "Desact. alarmas", INDENT "Desact. alarmas telem.")
#define TR_ENABLE_POPUP        "Activa Popup"
#define TR_DISABLE_POPUP       "Desactiva Popup"
#define TR_POPUP               "Popup"
#define TR_MIN                 "Min"
#define TR_MAX                 "Máx"
#define TR_CURVE_PRESET        "Preset..."
#define TR_PRESET              "Preset"
#define TR_MIRROR              "Mirror"
#define TR_CLEAR               "Borrar"
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
#define TR_MIXSOURCE           "Entrada mixer"
#define TR_CONSTANT            "Constante"
#define TR_PERSISTENT_MAH      INDENT "Valor mAh"
#define TR_PREFLIGHT           "Chequeos prevuelo"
#define TR_CHECKLIST           TR(INDENT "Lista verif", INDENT "Lista verificación")
#define TR_FAS_OFFSET          TR(INDENT "FAS Ofs", INDENT "FAS Offset")
#define TR_AUX_SERIAL_MODE     "Puerto serie"
#define TR_AUX2_SERIAL_MODE    "Puerto serie 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port power"
#define TR_SCRIPT              "Script"
#define TR_INPUTS              "Entradas"
#define TR_OUTPUTS             "Salidas"
#define STR_EEBACKUP           "Copia seg. EEPROM"
#define STR_FACTORYRESET       "Rest. fabrica"
#define TR_CONFIRMRESET        "Borrar TODOS los " LCDW_128_480_LINEBREAK "modelos y conf?"
#define TR_TOO_MANY_LUA_SCRIPTS "Demasiados Lua scripts!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
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

#define TR_LSW_HEADERS_FUNCTION        "Función"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "Interrup. AND"
#define TR_LSW_HEADERS_DURATION        "Duración"
#define TR_LSW_HEADERS_DELAY           "Retardo"

#define TR_GVAR_HEADERS_NAME          "Nombre"
#define TR_GVAR_HEADERS_FM0           "Valor en FM0"
#define TR_GVAR_HEADERS_FM1           "Valor en FM1"
#define TR_GVAR_HEADERS_FM2           "Valor en FM2"
#define TR_GVAR_HEADERS_FM3           "Valor en FM3"
#define TR_GVAR_HEADERS_FM4           "Valor en FM4"
#define TR_GVAR_HEADERS_FM5           "Valor en FM5"
#define TR_GVAR_HEADERS_FM6           "Valor en FM6"
#define TR_GVAR_HEADERS_FM7           "Valor en FM7"
#define TR_GVAR_HEADERS_FM8           "Valor en FM8"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS    { "Tipo de comparación o función", "Primera variable", "Segunda variable o constante", "Segunda variable o constante", "Condición adicional para activar línea", "Duración mínima de switch lógico", "Duración mínima TRUE para activar switch lógico" }

// Horus layouts and widgets
#define TR_FIRST_CHANNEL              "Primer canal"
#define TR_FILL_BACKGROUND            "Rellenar fondo"
#define TR_BG_COLOR                   "Color BG"
#define TR_SLIDERS_TRIMS              "Sliders+Trims"
#define TR_SLIDERS                    "Sliders"
#define TR_FLIGHT_MODE                "Modo de vuelo"
#define TR_INVALID_FILE               "Fichero no valido"
#define TR_TIMER_SOURCE               "Entrada timer"
#define TR_SIZE                       "Tamaño"
#define TR_SHADOW                     "Sombra"
#define TR_TEXT                       "Texto"
#define TR_COLOR                      "Color"
#define TR_MAIN_VIEW_X                "Vista principal X"
#define TR_PANEL1_BACKGROUND          "Fondo panel1"
#define TR_PANEL2_BACKGROUND          "Fondo panel2"

// Taranis About screen
#define TR_ABOUTUS             "Nosotros"

#define TR_CHR_SHORT  's'
#define TR_CHR_LONG   'l'
#define TR_CHR_TOGGLE 't'
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
#define TR_ALTITUDE            INDENT "Altitud"
#define TR_SCALE               "Escala"
#define TR_VIEW_CHANNELS       "Ver Canales"
#define TR_VIEW_NOTES          "Ver Notas"
#define TR_MODEL_SELECT        "Seleccionar modelo"
#define TR_MODS_FORBIDDEN      "Modificaciones prohibidas"
#define TR_UNLOCKED            "Desbloqueado"
#define TR_ID                  "ID"
#define TR_PRECISION           "Precisión"
#define TR_RATIO               "Ratio"
#define TR_FORMULA             "Fórmula"
#define TR_CELLINDEX           "Cell index"
#define TR_LOGS                "Logs"
#define TR_OPTIONS             "Opciones"
#define TR_FIRMWARE_OPTIONS    "Opciones firmware"

#define TR_ALTSENSOR           "Alt sensor"
#define TR_CELLSENSOR          "Cell sensor"
#define TR_GPSSENSOR           "GPS sensor"
#define TR_CURRENTSENSOR       "Sensor"
#define TR_AUTOOFFSET          "Auto offset"
#define TR_ONLYPOSITIVE        "Positivo"
#define TR_FILTER              "Filtro"
#define TR_TELEMETRYFULL       TR("Telem. llena!", "Todas las entradas de telemetría llenas!")
#define TR_SERVOS_OK           "Servos OK"
#define TR_SERVOS_KO           "Servos KO"
#define TR_INVERTED_SERIAL     INDENT "Invertir"
#define TR_IGNORE_INSTANCE     TR(INDENT "No inst.", INDENT "Ignora instancias")
#define TR_DISCOVER_SENSORS    "Buscar sensores"
#define TR_STOP_DISCOVER_SENSORS "Parar busqueda"
#define TR_DELETE_ALL_SENSORS  "Borrar sensores"
#define TR_CONFIRMDELETE       "Seguro " LCDW_128_480_LINEBREAK "borrar todo ?"
#define TR_SELECT_WIDGET       "Seleccionar widget"
#define TR_REMOVE_WIDGET       "Borrar widget"
#define TR_WIDGET_SETTINGS     "Config. widget"
#define TR_REMOVE_SCREEN       "Borrar pantalla"
#define TR_SETUP_WIDGETS       "Config. widgets"
#define TR_USER_INTERFACE      "Interfaz"
#define TR_THEME               "Tema"
#define TR_SETUP               "Configuración"
#define TR_LAYOUT              "Diseño"
#define TR_ADD_MAIN_VIEW         "Añadir vista pral."
#define TR_BACKGROUND_COLOR    "Color de fondo"
#define TR_MAIN_COLOR          "Color principal"
#define TR_BAR2_COLOR                  "Color barra secundaria"
#define TR_BAR1_COLOR                  "Color barra principal"
#define TR_TEXT_COLOR                  "Color texto"
#define TR_TEXT_VIEWER         "Visor de texto"

#define TR_MENU_INPUTS          STR_CHAR_INPUT "Entradas"
#define TR_MENU_LUA             STR_CHAR_LUA "Lua scripts"
#define TR_MENU_STICKS           STR_CHAR_STICK "Sticks"
#define TR_MENU_POTS            STR_CHAR_POT "Pots"
#define TR_MENU_MAX             STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI            STR_CHAR_CYC "Cíclico"
#define TR_MENU_TRIMS            STR_CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES        STR_CHAR_SWITCH "Interruptores"
#define TR_MENU_LOGICAL_SWITCHES TR( STR_CHAR_SWITCH "Interr. lógicos",  STR_CHAR_SWITCH "Interruptores lógicos")
#define TR_MENU_TRAINER         STR_CHAR_TRAINER "Entrenador"
#define TR_MENU_CHANNELS        STR_CHAR_CHANNEL "Canales"
#define TR_MENU_GVARS           STR_CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY       STR_CHAR_TELEMETRY "Telemetría"
#define TR_MENU_DISPLAY        "MONITOR"
#define TR_MENU_OTHER          "Otros"
#define TR_MENU_INVERT         "Invertir"
#define TR_JITTER_FILTER       "Filtro ADC"
#define TR_DEAD_ZONE           "Dead zone"
#define TR_RTC_CHECK           TR("Check RTC", "Check RTC voltaje")
#define TR_AUTH_FAILURE        "Fallo " LCDW_128_480_LINEBREAK  "autentificación"
#define TR_RACING_MODE         "Racing mode"

#define STR_VFR               "VFR"
#define STR_RSSI              "RSSI"
#define STR_R9PW                      "R9PW"
#define STR_RAS               "SWR"
#define STR_A1                "A1"
#define STR_A2                "A2"
#define STR_A3                "A3"
#define STR_A4                "A4"
#define STR_BATT              "RxBt"
#define STR_ALT               "Alt"
#define STR_TEMP1             "Tmp1"
#define STR_TEMP2             "Tmp2"
#define STR_TEMP3                     "Tmp3"
#define STR_TEMP4                     "Tmp4"
#define STR_RPM2                      "RPM2"
#define STR_PRES                      "Pres"
#define STR_ODO1                      "Odo1"
#define STR_ODO2                      "Odo2"
#define STR_TXV                       "TX_V"
#define STR_CURR_SERVO1               "CSv1"
#define STR_CURR_SERVO2               "CSv2"
#define STR_CURR_SERVO3               "CSv3"
#define STR_CURR_SERVO4               "CSv4"
#define STR_DIST                      "Dist"
#define STR_ARM                       "Arm"
#define STR_C50                       "C50"
#define STR_C200                      "C200"
#define STR_RPM               "RPM"
#define STR_FUEL              "Fuel"
#define STR_VSPD              "VSpd"
#define STR_ACCX              "AccX"
#define STR_ACCY              "AccY"
#define STR_ACCZ              "AccZ"
#define STR_GYROX             "GYRX"
#define STR_GYROY             "GYRY"
#define STR_GYROZ             "GYRZ"
#define STR_CURR              "Curr"
#define STR_CAPACITY          "Capa"
#define STR_VFAS              "VFAS"
#define STR_BATT_PERCENT      "Bat%"
#define STR_ASPD              "ASpd"
#define STR_GSPD              "GSpd"
#define STR_HDG               "Hdg"
#define STR_SATELLITES        "Sats"
#define STR_CELLS             "Cels"
#define STR_GPSALT            "GAlt"
#define STR_GPSDATETIME       "Date"
#define STR_BATT1_VOLTAGE     "RB1V"
#define STR_BATT2_VOLTAGE     "RB2V"
#define STR_BATT1_CURRENT     "RB1A"
#define STR_BATT2_CURRENT     "RB2A"
#define STR_BATT1_CONSUMPTION "RB1C"
#define STR_BATT2_CONSUMPTION "RB2C"
#define STR_BATT1_TEMP        "RB1T"
#define STR_BATT2_TEMP        "RB2T"
#define STR_RB_STATE          "RBS"
#define STR_CHANS_STATE       "RBCS"
#define STR_RX_RSSI1          "1RSS"
#define STR_RX_RSSI2          "2RSS"
#define STR_RX_QUALITY        "RQly"
#define STR_RX_SNR            "RSNR"
#define STR_RX_NOISE          "RNse"
#define STR_RF_MODE           "RFMD"
#define STR_TX_POWER          "TPWR"
#define STR_TX_RSSI           "TRSS"
#define STR_TX_QUALITY        "TQly"
#define STR_TX_SNR            "TSNR"
#define STR_TX_NOISE          "TNse"
#define STR_PITCH             "Ptch"
#define STR_ROLL              "Roll"
#define STR_YAW               "Yaw"
#define STR_THROTTLE          "Thr"
#define STR_QOS_A             "FdeA"
#define STR_QOS_B             "FdeB"
#define STR_QOS_L             "FdeL"
#define STR_QOS_R             "FdeR"
#define STR_QOS_F             "FLss"
#define STR_QOS_H             "Hold"
#define STR_LAP_NUMBER        "Lap "
#define STR_GATE_NUMBER       "Gate"
#define STR_LAP_TIME          "LapT"
#define STR_GATE_TIME         "GteT"
#define STR_ESC_VOLTAGE       "EscV"
#define STR_ESC_CURRENT       "EscA"
#define STR_ESC_RPM           "Erpm"
#define STR_ESC_CONSUMPTION   "EscC"
#define STR_ESC_TEMP          "EscT"
#define STR_SD1_CHANNEL       "Chan"
#define STR_GASSUIT_TEMP1     "GTp1"
#define STR_GASSUIT_TEMP2     "GTp2"
#define STR_GASSUIT_RPM       "GRPM"
#define STR_GASSUIT_FLOW      "GFlo"
#define STR_GASSUIT_CONS      "GFue"
#define STR_GASSUIT_RES_VOL   "GRVl"
#define STR_GASSUIT_RES_PERC  "GRPc"
#define STR_GASSUIT_MAX_FLOW  "GMFl"
#define STR_GASSUIT_AVG_FLOW  "GAFl"
#define STR_SBEC_VOLTAGE      "BecV"
#define STR_SBEC_CURRENT      "BecA"
#define STR_RB3040_EXTRA_STATE        "RBES"
#define STR_RB3040_CHANNEL1           "CH1A"
#define STR_RB3040_CHANNEL2           "CH2A"
#define STR_RB3040_CHANNEL3           "CH3A"
#define STR_RB3040_CHANNEL4           "CH4A"
#define STR_RB3040_CHANNEL5           "CH5A"
#define STR_RB3040_CHANNEL6           "CH6A"
#define STR_RB3040_CHANNEL7           "CH7A"
#define STR_RB3040_CHANNEL8           "CH8A"
#define STR_ESC_VIN                   "EVIN"
#define STR_ESC_TFET                  "TFET"
#define STR_ESC_CUR                   "ECUR"
#define STR_ESC_TBEC                  "TBEC"
#define STR_ESC_BCUR                  "CBEC"
#define STR_ESC_VBEC                  "VBEC"
#define STR_ESC_THR                   "ETHR"
#define STR_ESC_POUT                  "EOUT"
#define STR_SMART_BAT_BTMP            "BTmp"
#define STR_SMART_BAT_BCUR            "BCur"
#define STR_SMART_BAT_BCAP            "BUse"
#define STR_SMART_BAT_MIN_CEL         "CLMi"
#define STR_SMART_BAT_MAX_CEL         "CLMa"
#define STR_SMART_BAT_CYCLES          "Cycl"
#define STR_SMART_BAT_CAPACITY        "BCpT"
#define STR_CL01                      "Cel1"
#define STR_CL02                      "Cel2"
#define STR_CL03                      "Cel3"
#define STR_CL04                      "Cel4"
#define STR_CL05                      "Cel5"
#define STR_CL06                      "Cel6"
#define STR_CL07                      "Cel7"
#define STR_CL08                      "Cel8"
#define STR_CL09                      "Cel9"
#define STR_CL10                      "Cl10"
#define STR_CL11                      "Cl11"
#define STR_CL12                      "Cl12"
#define STR_CL13                      "Cl13"
#define STR_CL14                      "Cl14"
#define STR_CL15                      "Cl15"
#define STR_CL16                      "Cl16"
#define STR_CL17                      "Cl17"
#define STR_CL18                      "Cl18"
#define STR_FRAME_RATE                "FRat"
#define STR_TOTAL_LATENCY             "TLat"
#define STR_VTX_FREQ                  "VFrq"
#define STR_VTX_PWR                   "VPwr"
#define STR_VTX_CHAN                  "VChn"
#define STR_VTX_BAND                  "VBan"
#define STR_SERVO_CURRENT             "SrvA"
#define STR_SERVO_VOLTAGE             "SrvV"
#define STR_SERVO_TEMPERATURE         "SrvT"
#define STR_SERVO_STATUS              "SrvS"
#define STR_LOSS                      "Loss"
#define STR_SPEED                     "Spd "
#define STR_FLOW                      "Flow"

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
#define TR_AUTHOR                       "Author"
#define TR_DESCRIPTION                  "Description"
#define TR_SAVE                         "Save"
#define TR_CANCEL                       "Cancel"
#define TR_EDIT_THEME                   "EDIT THEME"
#define TR_DETAILS                      "Details"
#define TR_THEME_EDITOR                 "THEMES"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Select\nModel"
#define TR_MAIN_MENU_MODEL_NOTES        "Model\nNotes"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Channel\nMonitor"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Model\nSettings"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Radio\nSettings"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Screens\nSettings"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Reset\nTelemetry"
#define TR_MAIN_MENU_STATISTICS         "Statistics"
#define TR_MAIN_MENU_ABOUT_EDGETX       "About\nEdgeTX"
// End Main menu
