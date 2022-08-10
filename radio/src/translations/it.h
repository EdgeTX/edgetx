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

// IT translations author: Romolo Manfredini <romolo.manfredini@gmail.com>
// IT integrations for OpenTX V2.3.x author: Marco Robustini <robustinimarco@gmail.com>

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */


#define TR_OFFON               "OFF","ON"
#define TR_MMMINV              "---","INV"
#define TR_VBEEPMODE           TR("Silen","Silente"),TR("Avvis","Avvisi"),TR("Notst","No Tasti"),"Tutti"
#define TR_VBLMODE             TR("OFF","Spenta"),"Tasti",TR("Stks","Sticks"),"Tutti",TR("ON","Accesa")
#define TR_TRNMODE             "OFF","+=",":="
#define TR_TRNCHN              "CH1","CH2","CH3","CH4"

#define TR_AUX_SERIAL_MODES    "OFF","Replica S-Port","Telemetria","SBUS Trainer","LUA","CLI","GPS","Debug"
#define TR_SWTYPES             "Disab.","Toggle","2POS","3POS"
#define TR_POTTYPES            "Disab.",TR("Pot c. fer","Pot. con centro"),TR("Multipos","Inter. Multipos"),TR("Pot","Potenziometro")
#define TR_SLIDERTYPES         "Disab.","Slider"
#define TR_VPERSISTENT         "NO","Volo","Reset Manuale"
#define TR_COUNTRY_CODES       TR("US","America"),TR("JP","Japan"),TR("EU","Europa")
#define TR_USBMODES            TR("Chied","Chiedi"),TR("Joyst","Joystick"),TR("SDCard","Storage"),TR("Serial","Seriale")
#define TR_JACK_MODES          "Chiedi","Audio","Trainer"
#define TR_TELEMETRY_PROTOCOLS         "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_TELEMETRY_MODE        "Off","On","Off+Aux","On+Aux"
#define TR_MULTI_CUSTOM        "Person."
#define TR_VTRIMINC            "Exp","ExFine","Fine","Medio","Ampio "
#define TR_VDISPLAYTRIMS       "No","Cambio","Si"
#define TR_VBEEPCOUNTDOWN      "Niente","Suoni","Voce","Vibra"
#define TR_COUNTDOWNVALUES     "5s","10s","20s","30s"
#define TR_VVARIOCENTER        "Tono","Silenz."
#define TR_CURVE_TYPES         "Fisso","Modific."
#define TR_ADCFILTERVALUES     "Globali","Off","On"

#if defined(PCBX10)
  #define TR_RETA123           "D","E","M","A","1","3","2","4","5","6","7","S","D"
#elif defined(PCBHORUS)
  #define TR_RETA123           "D","E","M","A","1","3","2","4","5","S","D"
#elif defined(PCBX9E)
  #define TR_RETA123           "D","E","M","A","1","2","3","4","S","D","S","D"
#elif defined(PCBTARANIS)
  #define TR_RETA123           "D","E","M","A","1","2","3","S","D"
#else
  #define TR_RETA123           "D","E","M","A","1","2","3"
#endif


#define TR_VCURVEFUNC          "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX              "Add.","Molt.","Sost."
#define TR_VMLTPX2             "+=","*=",":="

#if defined(PCBHORUS)
  #define TR_VMIXTRIMS         "OFF","ON","Dir","Ele","Mot","Ale","T5","T6"
#else
  #define TR_VMIXTRIMS         "OFF","ON","Dir","Ele","Mot","Ale"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER          "Timer"
  #define TR_CSWSTICKY         "Stcky"
  #define TR_CSWRANGE          "Campo"
  #define TR_CSWSTAY           "Edge"
#else
  #define TR_CSWTIMER          "Tim"
  #define TR_CSWSTICKY         "Glue"
  #define TR_CSWRANGE          "Rnge"
  #define TR_CSWSTAY           "Edge"
#endif

#define TR_CSWEQUAL  "a=x"
#define TR_VCSWFUNC  "---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#if defined(VARIO)
  #define TR_VVARIO            "Vario"
#else
  #define TR_VVARIO            "[Vario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND             "Suona"
#else
  #define TR_SOUND             "Beep"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC            "Vibrazione"
#else
  #define TR_HAPTIC            "[Vibrazione]"
#endif

#if defined(VOICE)
    #define TR_PLAY_TRACK      "SuonaTraccia"
  #define TR_PLAY_BOTH         "Play Entr."
  #define TR_PLAY_VALUE        TR("LeggiVal", "LeggiValore")
#else
  #define TR_PLAY_TRACK        "[Brano]"
  #define TR_PLAY_BOTH         "[Play Entr.]"
  #define TR_PLAY_VALUE        "[LeggiValore]"
#endif

#define TR_SF_BG_MUSIC        "Musica Sf","Musica Sf ||"

#if defined(SDCARD)
  #define TR_SDCLOGS           "Logs SDCard"
#else
  #define TR_SDCLOGS           "[Logs SDCard]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR       "Regola"
#else
  #define TR_ADJUST_GVAR       "[RegolaVG]"
#endif

#if defined(LUA)
  #define TR_SF_PLAY_SCRIPT   "Script Lua"
#else
  #define TR_SF_PLAY_SCRIPT   "[Lua]"
#endif

#if defined(DEBUG)
  #define TR_SF_TEST          ,"Test"
#else
  #define TR_SF_TEST
#endif

#if defined(OVERRIDE_CHANNEL_FUNCTION) && LCD_W >= 212
  #define TR_SF_SAFETY        "Blocco"
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY        "Blocco"
#else
  #define TR_SF_SAFETY        "---"
#endif

#define TR_SF_SCREENSHOT       "Screenshot"
#define TR_SF_RACING_MODE      "Modo Racing"
#define TR_SF_DISABLE_TOUCH    "No Touch"
#define TR_SF_RESERVE          "[riserva]"

#define TR_VFSWFUNC            TR_SF_SAFETY,"Maestro","Trim Instant.","Azzera","Set",TR_ADJUST_GVAR,"Volume","SetFailsafe","RangeCheck","ModuleBind",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,"Retroillum.",TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH TR_SF_TEST

#define TR_FSW_RESET_TELEM   TR("Telm", "Telemetr.")

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS          "Timer 1","Timer 2","Timer 3"
#else
  #define TR_FSW_RESET_TIMERS          "Tmr1","Tmr2","Tmr3"
#endif

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("All","Tutto"),TR_FSW_RESET_TELEM

#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP        "ft"
#define SPEED_UNIT_IMP         "mph"
#define LENGTH_UNIT_METR       "m"
#define SPEED_UNIT_METR        "kmh"

#define TR_VUNITSSYSTEM        TR("Metric","Metriche"),TR("Imper.","Imperiali")
#define TR_VTELEMUNIT          "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km"

#define STR_V                  (STR_VTELEMUNIT[1])
#define STR_A                  (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE    "Niente","Valori","Barre","Script"
#define TR_GPSFORMAT           "HMS","NMEA"

#define TR_TEMPLATE_CLEAR_MIXES        "Canc. Mixer"
#define TR_TEMPLATE_SIMPLE_4CH         "Semplice 4CH"
#define TR_TEMPLATE_STICKY_TCUT        "Coda-V"
#define TR_TEMPLATE_VTAIL              "V-Tail"
#define TR_TEMPLATE_DELTA              "Elevon\\Delta"
#define TR_TEMPLATE_ECCPM              "eCCPM"
#define TR_TEMPLATE_HELI               "Heli Setup"
#define TR_TEMPLATE_SERVO_TEST         "Test Servo"

#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICKS_VSRCRAW      STR_CHAR_STICK "Dir", STR_CHAR_STICK "Ele", STR_CHAR_STICK "Mot", STR_CHAR_STICK "Ale"

#if defined(PCBHORUS)
  #define TR_TRIMS_VSRCRAW     STR_CHAR_TRIM "Dir", STR_CHAR_TRIM "Ele", STR_CHAR_TRIM "Mot", STR_CHAR_TRIM "Ale", STR_CHAR_TRIM "T5",  STR_CHAR_TRIM "T6"
#else
  #define TR_TRIMS_VSRCRAW     STR_CHAR_TRIM "Dir", STR_CHAR_TRIM "Ele", STR_CHAR_TRIM "Mot", STR_CHAR_TRIM "Ale"
#endif

#if defined(PCBHORUS)
  #define TR_TRIMS_SWITCHES    STR_CHAR_TRIM "Rl",  STR_CHAR_TRIM "Rr", STR_CHAR_TRIM "Ed", STR_CHAR_TRIM "Eu", STR_CHAR_TRIM "Td", STR_CHAR_TRIM "Tu", STR_CHAR_TRIM "Al", STR_CHAR_TRIM "Ar", STR_CHAR_TRIM "5d", STR_CHAR_TRIM "5u", STR_CHAR_TRIM "6d", STR_CHAR_TRIM "6u"
#else
  #define TRIM_SWITCH_CHAR     TR("t",STR_CHAR_TRIM)
#define TR_TRIMS_SWITCHES    TRIM_SWITCH_CHAR "Rl", TRIM_SWITCH_CHAR "Rr", TRIM_SWITCH_CHAR "Ed", TRIM_SWITCH_CHAR "Eu", TRIM_SWITCH_CHAR "Td", TRIM_SWITCH_CHAR "Tu", TRIM_SWITCH_CHAR "Al",  TRIM_SWITCH_CHAR "Ar"
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

#define TR_VTMRMODES                   "OFF","ABS","MOs","MO%","MOt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Maestro/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Allievo/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Master/Modulo SBUS"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Master/Modulo CPPM"
#define TR_VTRAINER_MASTER_BATTERY     "Master/Seriale"
#define TR_VTRAINER_BLUETOOTH          "Master/" TR("BT","Bluetooth"), "Slave/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VFAILSAFE                   "Non settato","Mantieni","Personali","No impulsi","Ricevente"
#define TR_VSENSORTYPES                "Custom","Calcolato"
#define TR_VFORMULAS                   "Somma","Media","Min","Max","Moltipl","Totalizza","Cella","Consumo","Distanza"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Minore","1","2","3","4","5","6","Maggiore","Delta"
#define TR_TEXT_SIZE                   "Standard","Tiny","Small","Mid","Double"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA" (center only)","= (symetrical)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")

#if defined(COLORLCD)
  #define INDENT               "\007"
  #define LEN_INDENT           1
  #define INDENT_WIDTH         12
  #define BREAKSPACE           "\036"
#else
  #define INDENT               "\001"
  #define LEN_INDENT           1
  #define INDENT_WIDTH         (FW/2)
  #define BREAKSPACE           " "
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

#define TR_MENUWHENDONE        CENTER "\007" TR_ENTER " Conferma"
#define TR_FREE                "Disp."
#define TR_YES                         "Si"
#define TR_NO                          "No"
#define TR_DELETEMODEL         "CANCELLA" BREAKSPACE "MODELLO"
#define TR_COPYINGMODEL        "Copia in corso.."
#define TR_MOVINGMODEL         "Spostamento..."
#define TR_LOADINGMODEL        "Caricamento..."
#define TR_NAME                "Nome"
#define TR_MODELNAME            TR("Nome Mode.", "Nome Modello")
#define TR_PHASENAME           "Nome fase"
#define TR_MIXNAME             "Nome mix"
#define TR_INPUTNAME           "Nome Ingr."
  #define TR_EXPONAME          "Nome expo"
#define TR_BITMAP              "Immagine"
#define TR_TIMER               TR("Timer", "Timer ")
#define TR_START                       "Start"
#define TR_ELIMITS             TR("Limiti.E", "Limiti Estesi")
#define TR_ETRIMS              TR("Trim Ext", "Trim Estesi")
#define TR_TRIMINC             "Passo Trim"
#define TR_DISPLAY_TRIMS       TR("Mos. Trims", "Mostra Trims")
#define TR_TTRACE              TR("So. motore", INDENT "Sorgente Motore")
#define TR_TTRIM               TR("Trim Mot.", INDENT "Trim Motore")
#define TR_TTRIM_SW            TR("T-Trim-Sw", INDENT "Trim switch")
#define TR_BEEPCTR             TR("Beep al c.", "Beep al centro")
#define TR_USE_GLOBAL_FUNCS    TR("Funz. Glob.", "Usa Funz. Globali")
#define TR_PROTOCOL            TR("Protoc.", "Protocollo")
#define TR_PPMFRAME            INDENT "Frame PPM"
#define TR_REFRESHRATE         TR(INDENT "Refresh", INDENT "Refresh rate")
#define STR_WARN_BATTVOLTAGE   TR(INDENT "Uscita VBAT: ", INDENT "Att.: livel. uscita VBAT: ")
#define TR_WARN_5VOLTS         "Attenzione: il livello d'uscita è 5 Volts"
#define TR_MS                  "ms"
#define TR_FREQUENCY           INDENT "Frequenza"
#define TR_SWITCH              "Inter."
#define TR_TRIMS               "Trims"
#define TR_FADEIN              "Diss.In"
#define TR_FADEOUT             "Diss.Out"
#define TR_DEFAULT             "(Predefinita)"
#define TR_CHECKTRIMS          CENTER "\006Contr.\012Trims"
#define OFS_CHECKTRIMS         CENTER_OFS+(9*FW)
#define TR_SWASHTYPE           "Tipo Ciclico"
#define TR_COLLECTIVE          TR("Collettivo", "Origine Collettivo")
#define TR_AILERON             TR("Cic. later.", "Sorg. cic. later.")
#define TR_ELEVATOR            TR("Cic. long.", "Sorg. cic. long.")
#define TR_SWASHRING           "Anello Ciclico"
#define TR_ELEDIRECTION        TR("Direzione ELE", "Direzione cic. long.")
#define TR_AILDIRECTION        TR("Direzione AIL", "Direzione cic. lat.")
#define TR_COLDIRECTION        TR("Direzione PIT", "Direzione passo coll.")
#define TR_MODE                "Modo"
#define TR_SUBTYPE             INDENT "Sottotipo"
#define TR_NOFREEEXPO          "Expo pieni!"
#define TR_NOFREEMIXER         "Mixer pieni!"
#define TR_SOURCE              "Sorg."
#define TR_WEIGHT              "Peso"
#define TR_EXPO                "Espo"
#define TR_SIDE                "Lato"
#define TR_DIFFERENTIAL        "Differ"
#define TR_OFFSET              "Offset"
#define TR_TRIM                "Trim"
#define TR_DREX                "DRex"
#define DREX_CHBOX_OFFSET      30
#define TR_CURVE               "Curva"
#define TR_FLMODE              TR("Fase", "Fasi")
#define TR_MIXWARNING          "Avviso"
#define TR_OFF                 "OFF"
#define TR_ANTENNA             "Antenna"
#define TR_NO_INFORMATION      TR("No info", "No informazione")
#define TR_MULTPX              "MultPx"
#define TR_DELAYDOWN           "Post.Giù "
#define TR_DELAYUP             "Post.Su"
#define TR_SLOWDOWN            "Rall.Giù "
#define TR_SLOWUP              "Rall.Su"
#define TR_MIXES               "MIXER"
#define TR_CV                  "CV"
#if defined(PCBNV14)
#define TR_GV                  "GV"
#else
#define TR_GV                  TR("G", "GV")
#endif
#define TR_ACHANNEL            "A\002ingresso"
#define TR_RANGE               TR(INDENT "Inter.", INDENT "Intervallo")
#define TR_CENTER              INDENT "Centro"
#define TR_BAR                 "Barra"
#define TR_ALARM               TR( "Allar.",  "Allarme")
#define TR_USRDATA             "Dati"
#define TR_BLADES              "Pale"
#define TR_SCREEN              "Schermo\001"
#define TR_SOUND_LABEL         "Suono"
#define TR_LENGTH              "Durata"
#define TR_BEEP_LENGTH         "Lung. Beep"
#define TR_BEEP_PITCH          "Tono"
#define TR_HAPTIC_LABEL        "Vibrazione"
#define TR_STRENGTH            "Forza"
#define TR_IMU_LABEL           "IMU"
#define TR_IMU_OFFSET          "Offset"
#define TR_IMU_MAX             "Max"
#define TR_CONTRAST            "Contrasto"
#define TR_ALARMS_LABEL        "Allarmi"
#define TR_BATTERY_RANGE       TR("Int. Batt.", "Intervallo batteria")
#define TR_BATTERYCHARGING     "Ricarica..."
#define TR_BATTERYFULL         "Bat. carica"
#define TR_BATTERYNONE         "Assente!"
#define TR_BATTERYWARNING      TR(INDENT "Avv. Batt.", INDENT "Avviso Batteria")
#define TR_INACTIVITYALARM     INDENT "Inattività"
#define TR_MEMORYWARNING       TR(INDENT "Avv. Mem.", INDENT "Avviso Memoria")
#define TR_ALARMWARNING        INDENT "Spegni suono"
#define TR_RSSI_SHUTDOWN_ALARM TR(INDENT "RSSI Shut.", INDENT "Control. RSSI al Shutdown")
#define TR_MODEL_STILL_POWERED "Ricevente ancora connessa"
#define TR_USB_STILL_CONNECTED         "USB ancora connesso"
#define TR_MODEL_SHUTDOWN              "Spegnimento?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Premi [ENT] per confermare"
#define TR_THROTTLE_LABEL      "Motore"
#define TR_THROTTLEREVERSE     TR("Mot inv.", "Motore Inverso")
#define TR_MINUTEBEEP          "Minuto"
#define TR_BEEPCOUNTDOWN       TR(INDENT "Conto rov", INDENT "Conto rovescia")
#define TR_PERSISTENT          TR(INDENT "Persist.", INDENT "Persistente")
#define TR_BACKLIGHT_LABEL     TR("Retroillu.", "Retroilluminazione")
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Stato"
#define TR_BLDELAY             INDENT "Durata"
#define TR_BLONBRIGHTNESS      TR(INDENT "Lumin. ON", INDENT "Luminosità ON")
#define TR_BLOFFBRIGHTNESS     TR(INDENT "Lumin. OFF", INDENT "Luminosità OFF")
#define TR_KEYS_BACKLIGHT              "Retroillum. tasti"
#define TR_BLCOLOR             "Colore"
#define TR_SPLASHSCREEN        TR("Sch. avvio", "Schermata d'avvio")
#define TR_PWR_ON_DELAY        "Rit. accens."
#define TR_PWR_OFF_DELAY       "Rit. spegni."
#define TR_THROTTLE_WARNING     TR(INDENT "All. Mot.", INDENT "Allarme Motore")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Cust-Pos", INDENT INDENT INDENT INDENT "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING       TR(INDENT "Avv. Int.", INDENT "Avviso Interr.")
#define TR_POTWARNINGSTATE     TR(INDENT "Pot&Slid.", INDENT "Pots & sliders")
#define TR_SLIDERWARNING       TR(INDENT "Pos. slid.", INDENT "Posizione slider")
#define TR_POTWARNING          TR(INDENT "Avv. Pot.", INDENT "Avviso Pot.")
#define TR_TIMEZONE            "Ora locale"
#define TR_ADJUST_RTC          TR(INDENT "Agg. RTC", "Aggiusta RTC")
#define TR_GPS                 "GPS"
#define TR_RXCHANNELORD        "Ordine CH RX"
#define TR_STICKS              "Sticks"
#define TR_POTS                "Pot."
#define TR_SWITCHES            "Interutt."
#define TR_SWITCHES_DELAY      TR("Rit. V. FV", "Ritardo Voce FV")
#define TR_SLAVE               "Allievo"
#define TR_MODESRC             "Modo\006% Origine"
#define TR_MULTIPLIER          "Moltiplica"
#define TR_CAL                 "Cal"
#define TR_VTRIM               "Trim - +"
#define TR_BG                  "BG:"
#define TR_CALIB_DONE                  "Calibratione completa"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART       "Premi [ENT] per partire"
  #define TR_SETMIDPOINT       "Centra sticks/pots/sliders e premi [ENT]"
  #define TR_MOVESTICKSPOTS    "Muovi sticks/pots/sliders e premi [ENT]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART       TR_ENTER " PER PART."
  #define TR_SETMIDPOINT       "CENTRA STICKS/SLIDERS"
  #define TR_MOVESTICKSPOTS    "MUOVI STICKS/POTS"
#else
  #define TR_MENUTOSTART       CENTER"\011" TR_ENTER " per Cal."
  #define TR_SETMIDPOINT       CENTER"\012SETTA CENTRO"
  #define TR_MOVESTICKSPOTS    CENTER"\010MUOVI STICK/POT"
#endif
#define TR_RXBATT              "Batt Rx:"
#define TR_TXnRX               "Tx:\0Rx:"
#define OFS_RX                 4
#define TR_ACCEL               "Acc:"
#define TR_NODATA              CENTER"DATI ASSENTI"
#define TR_US                         "us"
#define TR_HZ                         "Hz"
#define TR_TMIXMAXMS                  "Tmix max"
#define TR_FREE_STACK                 "Stack libero"
#define TR_INT_GPS_LABEL               "GPS interno"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL          "Lua scripts"
#define TR_FREE_MEM_LABEL             "Mem. libera"
#define TR_TIMER_LABEL                "Timer"
#define TR_THROTTLE_PERCENT_LABEL     "Throttle %"
#define TR_BATT_LABEL                 "Batteria"
#define TR_SESSION                    "Sessione"
#define TR_MENUTORESET         TR_ENTER" per azzerare"
#define TR_PPM_TRAINER         "TR"
#define TR_CH                  "CH"
#define TR_MODEL               "MODELLO"
#define TR_FM                  "FV"
#define TR_MIX                 "MIX"
#define TR_EEPROMLOWMEM        "EEPROM quasi piena!"
#define TR_PRESS_ANY_KEY_TO_SKIP   "\010Premi un tasto"
#define TR_THROTTLE_NOT_IDLE     TR("Motore non in posiz.", "Motore non in posizione")
#define TR_ALARMSDISABLED      "Avvisi Disattivati!"
#define TR_PRESSANYKEY         "\010Premi un tasto"
#define TR_BADEEPROMDATA       "Dati corrotti!"
#define TR_BAD_RADIO_DATA      "Dati radio errati"
#define TR_EEPROMFORMATTING    "Formatto EEPROM..."
#define TR_STORAGE_FORMAT      "Preparazione storage"
#define TR_EEPROMOVERFLOW      "EEPROM Piena"
#define TR_RADIO_SETUP      "CONFIGURA TX"
#define TR_MENUDATEANDTIME     "DATA ED ORA"
#define TR_MENUTRAINER         "MAESTRO/ALLIEVO"
#define TR_MENUSPECIALFUNCS    "FUNZIONI GLOBALI"
#define TR_MENUVERSION         "VERSIONE"
#define TR_MENU_RADIO_SWITCHES            "DIAG"
#define TR_MENU_RADIO_ANALOGS             "ANAS"
#define TR_MENU_RADIO_ANALOGS_CALIB       "ANALOGICI CALIBRATI"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS   "ANALOGICI RAW (5 Hz)"
#define TR_MENUCALIBRATION     "CALIBRAZIONE"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS     "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS     "\006Trims  => Offset "
#endif
#define TR_CHANNELS2FAILSAFE  "Canali => Failsafe"
#define TR_CHANNEL2FAILSAFE   "Canale => Failsafe"
#define TR_MENUMODELSEL        "MODELLI"
#define TR_MENU_MODEL_SETUP           "CONFIGURA"
#define TR_MENUFLIGHTMODE     "FASE DI VOLO"
#define TR_MENUFLIGHTMODES    "FASI DI VOLO"
#define TR_MENUHELISETUP       "CONFIGURA ELI"

  #define TR_MENUINPUTS        "INGRESSI"
  #define TR_MENULIMITS        "USCITE"

#define TR_MENUCURVES          "CURVE"
#define TR_MENUCURVE           "CURVA"
#define TR_MENULOGICALSWITCH   "INTER. PERSON."
#define TR_MENULOGICALSWITCHES TR("INTER. LOGICI", "INTERRUTTORI LOGICI")
#define TR_MENUCUSTOMFUNC      TR("FUNZ. SPECIALI", "FUNZIONI SPECIALI")
#define TR_MENUCUSTOMSCRIPTS   "SCRIPTS UTENTE"
#define TR_MENUTELEMETRY       "TELEMETRIA"
#define TR_MENUTEMPLATES       "ESEMPI GUIDA"
#define TR_MENUSTAT            "STATO"
#define TR_MENUDEBUG           "DEBUG"
#define TR_MONITOR_CHANNELS1   "MONITOR CANALI 1/8"
#define TR_MONITOR_CHANNELS2   "MONITOR CANALI 9/16"
#define TR_MONITOR_SWITCHES    "MONITOR INTERRUTTORI LOGICI"
#define TR_MONITOR_CHANNELS3   "MONITOR CANALI 17/24"
#define TR_MONITOR_CHANNELS4   "MONITOR CANALI 25/32"
#define TR_MONITOR_OUTPUT_DESC "Uscite"
#define TR_MONITOR_MIXER_DESC  "Mixers"
#define TR_RECEIVER_NUM        TR("RxNum", "Ricevente N.")
#define TR_RECEIVER            "Ricevente"
#define TR_MULTI_RFTUNE        TR("Tune RF", "Tune fine Freq. RF")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY     "Telemetria"
#define TR_MULTI_VIDFREQ       TR("Freq. video", "Frequenza video")
#define TR_RF_POWER             "Potenza RF"
#define TR_MULTI_FIXEDID               TR(INDENT"ID fisso", INDENT"ID Fisso")
#define TR_MULTI_OPTION        TR(INDENT"Opzione", INDENT"Opzione valore")
#define TR_MULTI_AUTOBIND      TR(INDENT "Ass. Ch.",INDENT "Associa al canale")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Disab. mappa Ch.")
#define TR_DISABLE_TELEM               TR("No Telem", "Telem. disabil.")
#define TR_MULTI_DSM_AUTODTECT TR(INDENT "Trova autom.", INDENT "Autoril. il formato")
#define TR_MULTI_LOWPOWER      TR(INDENT "Bassa pot.", INDENT "Modo bassa potenza")
#define TR_MULTI_LNA_DISABLE            INDENT "LNA disable"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "Link S.Port")
#define TR_MODULE_TELEM_ON             TR("ON", "Abilitato")
#define TR_DISABLE_INTERNAL         TR("Disatt. RF int.", "Disattiva RF interna")
#define TR_MODULE_NO_SERIAL_MODE       TR("!modo seriale", "Non in modo seriale")
#define TR_MODULE_NO_INPUT             TR("Nessun input", "Nessun input seriale")
#define TR_MODULE_NO_TELEMETRY         TR3("No telemetria", "MULTI_TELEMETRY disattivato", "Nessuna telem. (abilita MULTI_TELEMETRY)")
#define TR_MODULE_WAITFORBIND          "Associa per caricare il protocollo"
#define TR_MODULE_BINDING              "Associa"
#define TR_MODULE_UPGRADE_ALERT        TR3("Richiede agg.", "Richiede agg. modulo", "Modulo\nRichiede agg.")
#define TR_MODULE_UPGRADE              TR("Cons. agg.", "Consiglio agg. modulo")
#define TR_REBIND                      "Richiede associazione"
#define TR_REG_OK                      "Registrazione ok"
#define TR_BIND_OK                     "Associazione riuscita"
#define TR_BINDING_CH1_8_TELEM_ON               "Ch1-8 Telem ON"
#define TR_BINDING_CH1_8_TELEM_OFF               "Ch1-8 Telem OFF"
#define TR_BINDING_CH9_16_TELEM_ON               "Ch9-16 Telem ON"
#define TR_BINDING_CH9_16_TELEM_OFF               "Ch9-16 Telem OFF"
#define TR_PROTOCOL_INVALID            TR("Prot. invalido", "Protocollo invalido")
#define TR_MODULE_STATUS                TR(INDENT "Stato", INDENT "Stato del modulo")
#define TR_MODULE_SYNC                 TR(INDENT "Sinc.", INDENT "Sinc. del modulo")
#define TR_MULTI_SERVOFREQ     TR("Servo rate", "Aggiorna servo rate")
#define TR_MULTI_MAX_THROW             TR("Escurs. mass.", "Abilita escurs. mass.")
#define TR_MULTI_RFCHAN                TR("Canale RF", "Selez. canale RF")
#define TR_SYNCMENU            "[Sync]"
#define TR_LIMIT               INDENT "Limiti"
#define TR_MINRSSI             "Min. RSSI"
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Usa valore FlySky RSSI senza riscalare")
#define TR_LATITUDE            "Latitud."
#define TR_LONGITUDE           "Longitu."
#define TR_GPS_COORDS_FORMAT            TR("GPS Coords", "Formato Coordinate")
#define TR_VARIO               TR("Vario", "Variometro")
#define TR_PITCH_AT_ZERO       "Tono a Zero"
#define TR_PITCH_AT_MAX        "Tono al Max"
#define TR_REPEAT_AT_ZERO      "Ripeti a Zero"
#define TR_SHUTDOWN            "ARRESTO.."
#define TR_SAVE_MODEL           "Salvataggio dati modello"
#define TR_BATT_CALIB          TR("Calibra batt.", "Calibr. batteria")
#define TR_CURRENT_CALIB       "Calibra corr."
#define TR_VOLTAGE             TR(INDENT "Voltagg.",INDENT "Voltaggio")
#define TR_CURRENT             TR(INDENT "Corrente",INDENT "Corrente")
#define TR_SELECT_MODEL        TR("Scegli mod.", "Scegli Modello")
#define TR_MODEL_CATEGORIES            "Categorie Modelli"
#define TR_MODELS                      "Modelli"
#define TR_SELECT_MODE                 "Seleziona modo"
#define TR_CREATE_CATEGORY     "Crea Categoria"
#define TR_RENAME_CATEGORY     "Rinomina Categoria"
#define TR_DELETE_CATEGORY     "Cancella Categoria"
#define TR_CREATE_MODEL        "Crea Modello"
#define TR_DUPLICATE_MODEL     "Duplica Modello"
#define TR_COPY_MODEL          "Copia Modello"
#define TR_MOVE_MODEL          "Sposta Modello"
#define TR_BACKUP_MODEL        "Salva Modello"
#define TR_DELETE_MODEL        TR("Elim. Modello", "Elimina Modello")
#define TR_RESTORE_MODEL       TR("Ripr. Modello", "Ripristina Modello")
#define TR_DELETE_ERROR        "Errore cancell."
#define TR_CAT_NOT_EMPTY       "Categoria non vuota"
#define TR_SDCARD_ERROR        TR("Errore SD", "Errore SDCard")
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "SD senza file!"
#define TR_NO_SDCARD           "No SDCard"
#define TR_WAITING_FOR_RX              "Attendo la RX..."
#define TR_WAITING_FOR_TX              "Attendo la TX..."
#define TR_WAITING_FOR_MODULE          TR("Attendo modulo", "Attendo il modulo...")
#define TR_NO_TOOLS                    "Tools non disp."
#define TR_NORMAL                      "Normale"
#define TR_NOT_INVERTED                "No inv."
#define TR_NOT_CONNECTED               "!Connesso"
#define TR_CONNECTED                   "Connesso"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH senza telem.", "16CH senza telemetria")
#define TR_16CH_WITH_TELEMETRY         TR("16CH con telem.", "16CH con telemetria")
#define TR_8CH_WITH_TELEMETRY          TR("8CH con telem.", "8CH con telemetria")
#define TR_EXT_ANTENNA                 "Antenna Ext."
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Agg. opzioni RX?"
#define TR_UPDATE_TX_OPTIONS           "Agg. opzioni TX?"
#define TR_MODULES_RX_VERSION          "Versioni Moduli / RX"
#define TR_MENU_MODULES_RX_VERSION     "VERSIONI MODULI / RX"
#define TR_MENU_FIRM_OPTIONS           "OPZIONI FIRMWARE"
#define TR_IMU                        "IMU"
#define TR_STICKS_POTS_SLIDERS         "Interruttori/Pots/Sliders"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Interruttori/Pots/Sliders"
#define TR_RF_PROTOCOL                 "Protocollo RF"
#define TR_MODULE_OPTIONS              "Optioni modulo"
#define TR_POWER                       "Potenza"
#define TR_NO_TX_OPTIONS               "No Opzioni TX"
#define TR_RTC_BATT                    "RTC Batt"
#define TR_POWER_METER_EXT             "Power Meter (EST)"
#define TR_POWER_METER_INT             "Power Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spettro (EST)"
#define TR_SPECTRUM_ANALYSER_INT       "Spettro (INT)"
#define TR_SDCARD_FULL                 "SDCard Piena"
#define TR_NEEDS_FILE                  "RICHIEDE FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE        "Incompatibile"
#define TR_WARNING             "AVVISO"
#define TR_EEPROMWARN          "EEPROM"
#define TR_STORAGE_WARNING     "STORAGE"
#define TR_EEPROM_CONVERTING   "Conversione EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "Conversione SD richiesta"
#define TR_CONVERTING                  "Conversione: "
#define TR_THROTTLE_UPPERCASE        "MOTORE"
#define TR_ALARMSWARN          "ALLARMI"
#define TR_SWITCHWARN          "CONTROLLI"
#define TR_FAILSAFEWARN        "FAILSAFE"
#define TR_TEST_WARNING        TR("PER TEST", "BUILD PER TEST")
#define TR_TEST_NOTSAFE        "Usare solo per test"
#define TR_WRONG_SDCARDVERSION TR("Richiede ver: ", "Richiede versione: ")
#define TR_WARN_RTC_BATTERY_LOW        "Batteria RTC scarica"
#define TR_WARN_MULTI_LOWPOWER         "Modalità bassa pot."
#define TR_BATTERY                     "BATTERIA"
#define TR_WRONG_PCBREV        "PCB sbagliato"
#define TR_EMERGENCY_MODE      "MODALITA' EMERGENZA"
#define TR_PCBREV_ERROR        "Errore PCB"
#define TR_NO_FAILSAFE         "Failsafe" BREAKSPACE "non settato"
#define TR_KEYSTUCK            "Tasto bloccato"
#define TR_INVERT_THR          "Inverti Mot?"
#define TR_VOLUME              "Volume Audio"
#define TR_LCD                 "LCD"
#define TR_BRIGHTNESS          "Luminosità"
#define TR_CPU_TEMP            "Temp CPU \016>"
#define TR_CPU_CURRENT         "Corrente\022>"
#define TR_CPU_MAH             "Consumo"
#define TR_COPROC              "CoProc."
#define TR_COPROC_TEMP         "Temp. MB \016>"
#define TR_CAPAWARNING         INDENT "Capacità Bassa"
#define TR_TEMPWARNING         INDENT "Temp. Alta"
#define TR_TTL_WARNING         "Attenzione: usa livelli logici 3.3V"
#define TR_FUNC                "Funz"
#define TR_V1                  "V1"
#define TR_V2                  "V2"
#define TR_DURATION            "Durata"
#define TR_DELAY               "Ritardo"
#define TR_SD_CARD             "SD Card"
#define TR_SDHC_CARD           "SD-HC Card"
#define TR_NO_SOUNDS_ON_SD     "No Suoni" BREAKSPACE "su SD"
#define TR_NO_MODELS_ON_SD     "No Model." BREAKSPACE "su SD"
#define TR_NO_BITMAPS_ON_SD    "No Immag." BREAKSPACE "su SD"
#define TR_NO_SCRIPTS_ON_SD    "No Scripts" BREAKSPACE "su SD"
#define TR_SCRIPT_SYNTAX_ERROR TR("Errore sintassi", "Script errore sintassi")
#define TR_SCRIPT_PANIC        "Script panico"
#define TR_SCRIPT_KILLED       "Script fermato"
#define TR_SCRIPT_ERROR        "Errore sconosciuto"
#define TR_PLAY_FILE           "Suona"
#define TR_DELETE_FILE         "Elimina"
#define TR_COPY_FILE           "Copia"
#define TR_RENAME_FILE         "Rinomina"
#define TR_ASSIGN_BITMAP       "Assegna immagine"
#define TR_ASSIGN_SPLASH       "Splash screen"
#define TR_EXECUTE_FILE        "Esegui"
#define TR_REMOVED             " rimosso"
#define TR_SD_INFO             "Informazioni"
#define TR_SD_FORMAT           "Formatta"
#define TR_NA                  "N/A"
#define TR_HARDWARE            "HARDWARE"
#define TR_FORMATTING          "Formattazione"
#define TR_TEMP_CALIB          "Temp. Calib."
#define TR_TIME                "Ora"
#define TR_MAXBAUDRATE         "Max Bauds"
#define TR_BAUDRATE            "Baudrate"
#define TR_SAMPLE_MODE         "Modo Semplice"
#define TR_SAMPLE_MODES        "Normale","OneBit"
#define TR_SELECT_TEMPLATE_FOLDER "SELEZIONA CARTELLA TEMPLATE:"
#define TR_SELECT_TEMPLATE     "SELEZIONA TEMPLATE MODELLO:"
#define TR_NO_TEMPLATES        "Nessun modello template in questa cartella"
#define TR_SAVE_TEMPLATE       "Salva come template"
#define TR_BLANK_MODEL                 "Modello vuoto"
#define TR_BLANK_MODEL_INFO            "Crea modello vuoto"
#define TR_FILE_EXISTS                 "FILE GIA' ESISTENTE"
#define TR_ASK_OVERWRITE               "Vuoi sovrascrivere?"

#define TR_BLUETOOTH            "Bluetooth"
#define TR_BLUETOOTH_DISC       "Cerca"
#define TR_BLUETOOTH_INIT       "Iniz."
#define TR_BLUETOOTH_DIST_ADDR  "Ind. Dist."
#define TR_BLUETOOTH_LOCAL_ADDR "Ind. Loc."
#define TR_BLUETOOTH_PIN_CODE   "Codice PIN"
#define TR_BLUETOOTH_NODEVICES  "No Devices Found"
#define TR_BLUETOOTH_SCANNING   "Scanning..."
#define TR_BLUETOOTH_BAUDRATE   "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES      "---","Attivo"
#else
#define TR_BLUETOOTH_MODES      "---","Telemetr.","Trainer"
#endif
#define TR_SD_INFO_TITLE       "INFO SD"
#define TR_SD_TYPE             "Tipo:"
#define TR_SD_SPEED            "Veloc.:"
#define TR_SD_SECTORS          "Settori:"
#define TR_SD_SIZE             "Dimens:"
#define TR_TYPE                INDENT "Tipo"
#define TR_GLOBAL_VARS         "Variabili Globali"
#define TR_GVARS               "V.GLOBALI"
#define TR_GLOBAL_VAR          "Variabile globale"
#define TR_MENU_GLOBAL_VARS      "VARIABILI GLOBALI"
#define TR_OWN                 "Fase"
#define TR_DATE                "Data"
#define TR_MONTHS              { "Gen", "Feb", "Mar", "Apr", "Mag", "Giu", "Lug", "Ago", "Set", "Ott", "Nov", "Dic" }
#define TR_ROTARY_ENCODER      "R.E."
#define TR_ROTARY_ENC_MODE     TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR    "MONITOR CANALI"
#define TR_MIXERS_MONITOR      "MONITOR MIXER"
#define TR_PATH_TOO_LONG       "Path troppo lungo"
#define TR_VIEW_TEXT           "Vedi testo"
#define TR_FLASH_BOOTLOADER            TR("Prog. bootloader", "Programma bootloader")
#define TR_FLASH_DEVICE                TR("Flash device","Flash device")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Prog. disp. est.", "Programma dispositivo esterno")
#define TR_FLASH_RECEIVER_OTA          TR("Prog. RX OTA", INDENT "Programma RX OTA")
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA          TR("Prog. RX da OTA est.", INDENT "Prog. RX da OTA esterno")
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA          TR("Prog. RX da OTA int.", INDENT "Prog. RX da OTA interno")
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA TR("Prog. FC da OTA est.", INDENT "Prog. FC da OTA esterno")
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA TR("Prog. FC da OTA int.", INDENT "Prog. FC da OTA interno")
#define TR_FLASH_BLUETOOTH_MODULE      TR("Prog. mod. BT", INDENT "Prog. modulo Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT TR("Prog. PMU", "Programma PMU")
#define TR_DEVICE_NO_RESPONSE          TR("Device non risponde", "Device non risponde")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Device file prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Device dati rifiutati", "Device dati rifiutati")
#define TR_DEVICE_WRONG_REQUEST        TR("Device problema accesso", "Device problema accesso")
#define TR_DEVICE_FILE_REJECTED        TR("Device file rifiutato", "Device file rrifiutato")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device firma file", "Device firma file")
#define TR_CURRENT_VERSION             TR("Vers. currente ", "Versione corrente: ")
#define TR_FLASH_EXTERNAL_ELRS         TR("Scrivi ELRS Est.", "Scrivi ELRS Esterno")
#define TR_FLASH_INTERNAL_MODULE       TR("Prog. modulo int.", "Programma modulo interno")
#define TR_FLASH_EXTERNAL_MODULE       TR("Prog. modulo est.", "Programma modulo esterno")
#define TR_FLASH_INTERNAL_MULTI        TR("Prog. MULTI int.", "Programma MULTI interno")
#define TR_FLASH_EXTERNAL_MULTI        TR("Prog. MULTI est.", "Programma MULTI esterno")
#define TR_FIRMWARE_UPDATE_ERROR       TR(INDENT "Errore agg. FW",INDENT "Errore aggiornamento firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Aggiornamento" BREAKSPACE "riuscito"
#define TR_WRITING                     "Scrittura..."
#define TR_CONFIRM_FORMAT              "Confermi formattazione?"
#define TR_INTERNALRF                  "Modulo interno"
#define TR_INTERNAL_MODULE             TR("Modulo int.", "Modulo interno")
#define TR_EXTERNAL_MODULE             TR("Modulo est.", "Modulo esterno")
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX richiede aggiornamento"
#define TR_TELEMETRY_DISABLED          TR("Telem. off", "Telem. disabilitata")
#define TR_MORE_OPTIONS_AVAILABLE      "Altre opzioni assenti"
#define TR_NO_MODULE_INFORMATION       "Nessuna info del modulo"
#define TR_EXTERNALRF                  "Modulo esterno"
#define TR_FAILSAFE                     TR(INDENT "Failsafe", INDENT "Modo failsafe")
#define TR_FAILSAFESET                  TR(INDENT "FAILSAFE", INDENT "IMPOSTAZIONI FAILSAFE")
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Owner ID"
#define TR_PINMAPSET                   "PINMAP"
#define TR_HOLD                        "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                        "None"
#define TR_NONE_UPPERCASE              "NONE"
#define TR_MENUSENSOR                  "SENSORE"
#define TR_POWERMETER_PEAK             "Picco"
#define TR_POWERMETER_POWER            "Potenza"
#define TR_POWERMETER_ATTN             "Att."
#define TR_POWERMETER_FREQ             "Freq."
#define TR_MENUTOOLS                   "TOOLS"
#define TR_TURN_OFF_RECEIVER           "Spegni la RX"
#define TR_STOPPING                    "Fermando..."
#define TR_MENU_SPECTRUM_ANALYSER      "ANALIZZATORE SPETTRO"
#define TR_MENU_POWER_METER            "MISURATORE POTENZA"
#define TR_SENSOR              "SENSORE"
#define TR_COUNTRY_CODE         TR("Cod. Paese", "Codice del Paese")
#define TR_USBMODE             "Modo USB"
#define TR_JACK_MODE            "Modo JACK"
#define TR_VOICE_LANGUAGE           "Lingua vocale"
#define TR_UNITS_SYSTEM         "Unità"
#define TR_EDIT                "Modifica"
#define TR_INSERT_BEFORE       "Inserisci prima"
#define TR_INSERT_AFTER        "Inserisci dopo"
#define TR_COPY                "Copia"
#define TR_MOVE                "Sposta"
#define TR_PASTE               "Incolla"
#define TR_PASTE_AFTER                 "Incolla Dopo"
#define TR_PASTE_BEFORE                "Incolla Prima"
#define TR_DELETE              "Elimina"
#define TR_INSERT              TR("Inser.","Inserisci")
#define TR_RESET_FLIGHT        "Azzera volo"
#define TR_RESET_TIMER1        "Azzera Timer1"
#define TR_RESET_TIMER2        "Azzera Timer2"
#define TR_RESET_TIMER3        "Azzera Timer3"
#define TR_RESET_TELEMETRY     "Azzera Telemetria"
#define TR_STATISTICS          "Statistiche"
#define TR_ABOUT_US            "Info su"
#define TR_USB_JOYSTICK        "Joystick USB (HID)"
#define TR_USB_MASS_STORAGE    "Storage USB (SD)"
#define TR_USB_SERIAL          "Seriale USB (VCP)"
#define TR_SETUP_SCREENS       "Schermate conf."
#define TR_MONITOR_SCREENS     "Monitors"
#define TR_AND_SWITCH          "Inter. AND"
#define TR_SF                  "CF"
#define TR_GF                  "GF"
#define TR_ANADIAGS_CALIB      "Analogici calibrati"
#define TR_ANADIAGS_FILTRAWDEV "Analogici grezzi filtrati con deviazione"
#define TR_ANADIAGS_UNFILTRAW  "Analogici RAW non filtrati"
#define TR_ANADIAGS_MINMAX     "Min., max. e range"
#define TR_ANADIAGS_MOVE       "Muovi analogici ai loro estremi!"
#define TR_SPEAKER             INDENT "Speaker"
#define TR_BUZZER              INDENT "Buzzer"
#define TR_BYTES               "Bytes"
#define TR_MODULE_BIND         TR("[Bnd]","[Bind]")
#define TR_POWERMETER_ATTN_NEEDED      "Attenuatore necessario"
#define TR_PXX2_SELECT_RX              "Seleziona RX..."
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "Seleziona dispositivo"
#define TR_DISCOVER             "Cerca"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "In attesa..."
#define TR_RECEIVER_DELETE             "Cancello RX?"
#define TR_RECEIVER_RESET              "Resetto RX?"
#define TR_SHARE                       "Condividere"
#define TR_BIND                        "Bind"
#define TR_REGISTER                    TR("Reg", "Registrare")
#define TR_MODULE_RANGE                TR("[Rng]","[Range]")
#define TR_RECEIVER_OPTIONS            TR("OPZIONI RX", "OPZIONI RICEVENTE")
#define TR_DEL_BUTTON                  BUTTON(TR("Canc.", "Cancella"))
#define TR_RESET_BTN           "[Reset]"
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                BUTTON(TR("SW", "Tasti"))
#define TR_ANALOGS_BTN             BUTTON("Analogici")
#define TR_TOUCH_NOTFOUND          "Schermo touch non trovato"
#define TR_TOUCH_EXIT              "Tocca lo schermo per uscire"
#define TR_CALIBRATION             TR("Calibraz.", "Calibrazione")
#define TR_SET                 "[Set]"
#define TR_TRAINER             TR("Trainer", "Maestro/Allievo")
#define TR_CHANS                       "Canali"
#define TR_ANTENNAPROBLEM      CENTER "Problemi antenna TX!"
#define TR_MODELIDUSED         TR("ID già usato", "ID Modello già usato")
#define TR_MODULE              "Modulo"
#define TR_RX_NAME                     "Nome RX"
#define TR_TELEMETRY_TYPE      "Tipo Telemetria"
#define TR_TELEMETRY_SENSORS   "Sensori"
#define TR_VALUE               "Valore"
#define TR_REPEAT                      "Ripeti"
#define TR_ENABLE                      "Attivo"
#define TR_TOPLCDTIMER         "Timer LCD Sup."
#define TR_UNIT                "Unita"
#define TR_TELEMETRY_NEWSENSOR INDENT "Aggiungi nuovo sensore"
#define TR_CHANNELRANGE        TR(INDENT "Num Canali", INDENT "Numero Canali")
#define TR_RXFREQUENCY         TR("Rx Freqency", "Rx Servo Frequency")
#define TR_AFHDS3_RX_FREQ              TR("Freq. RX", "Frequenza RX")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetria")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Pot. att.", "Potenza attuale")
#define TR_AFHDS3_POWER_SOURCE         TR("Sorg. al.", "Sorgente alimentazione")
#define TR_ANTENNACONFIRM1     "ANTENNA EST."
#if defined(PCBX12S)
#define TR_ANTENNA_MODES       "Interna","Chiedi","Per modello","Interna + esterna"
#else
#define TR_ANTENNA_MODES       "Interna","Chiedi","Per model","Esterna"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Usa ant. int.", "Usa antenna interna")
#define TR_USE_EXTERNAL_ANTENNA        TR("Usa ant. est.", "Usa antenna esterna")
#define TR_ANTENNACONFIRM2     TR("Controlla instal.", "Controlla prima se installata!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1                "Non richiede"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1         "Richiede FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1          "Richiede EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2                "firmware certificato"
#define TR_LOWALARM            INDENT "Allarme Basso"
#define TR_CRITICALALARM       INDENT "Allarme Critico"
#define TR_RSSIALARM_WARN      TR(INDENT "Tel. RSSI", "TELEMETRIA RSSI")
#define TR_NO_RSSIALARM        TR("Allarmi disab.", "Allarmi telemetrici disabilitati")
#define TR_DISABLE_ALARM       TR(INDENT "Allarmi disab.", INDENT "Allarmi telemetrici disabilitati")
#define TR_ENABLE_POPUP        "Abilita Popup"
#define TR_DISABLE_POPUP       "Disabilita Popup"
#define TR_POPUP               "Popup"
#define TR_MIN                 "Min"
#define TR_MAX                 "Max"
#define TR_CURVE_PRESET        "Preimpostate..."
#define TR_PRESET              "Preimpostate"
#define TR_MIRROR              "Mirror"
#define TR_CLEAR               "Cancella"
#define TR_RESET               "Azzera"
#define TR_RESET_SUBMENU       "Azzera..."
#define TR_COUNT               "Punti"
#define TR_PT                  "pt"
#define TR_PTS                 "pti"
#define TR_SMOOTH              "Smussa"
#define TR_COPY_STICKS_TO_OFS  TR(INDENT "Cp. stick->subtrim", INDENT "Copia Sticks su Subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
#define TR_COPY_TRIMS_TO_OFS   TR(INDENT "Cp. trim->subtrim", INDENT "Copia Trims su Subtrim")
#define TR_INCDEC              "Inc/Decrementa"
#define TR_GLOBALVAR           "Var Globale"
#define TR_MIXSOURCE           "Sorgente Mixer"
#define TR_CONSTANT            "Constante"
#define TR_PERSISTENT_MAH      INDENT "Memo mAh"
#define TR_PREFLIGHT           "Controlli Prevolo"
#define TR_CHECKLIST           INDENT "Mostra Checklist"
#define TR_FAS_OFFSET          TR(INDENT "FAS Ofs", INDENT "FAS Offset")
#define TR_AUX_SERIAL_MODE     "Porta Seriale"
#define TR_AUX2_SERIAL_MODE    "Porta Seriale 2"
#define TR_AUX_SERIAL_PORT_POWER       "Potenza porta AUX"
#define TR_SCRIPT              "Script"
#define TR_INPUTS              "Ingresso"
#define TR_OUTPUTS             "Uscita"
#define STR_EEBACKUP           "Copiare l'EEPROM"
#define STR_FACTORYRESET       "Inizializza dati"
#define TR_CONFIRMRESET        "Resettare TUTTI i dati?"
#define TR_TOO_MANY_LUA_SCRIPTS "Troppi Scripts Lua!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "Nessuno Schermo Telemetria"
#define TR_TOUCH_PANEL                 "Schermo touch:"
#define TR_FILE_SIZE                   "Dimensione file"
#define TR_FILE_OPEN                   "Apro comunque?"

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
#define TR_INVERTED                    "Inverted"

#define TR_LSW_HEADERS_FUNCTION        "Funzione"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "Inter. AND"
#define TR_LSW_HEADERS_DURATION        "Durata"
#define TR_LSW_HEADERS_DELAY           "Ritardo"

#define TR_GVAR_HEADERS_NAME          "Nome"
#define TR_GVAR_HEADERS_FM0           "Valore su FM0"
#define TR_GVAR_HEADERS_FM1           "Valore su FM1"
#define TR_GVAR_HEADERS_FM2           "Valore su FM2"
#define TR_GVAR_HEADERS_FM3           "Valore su FM3"
#define TR_GVAR_HEADERS_FM4           "Valore su FM4"
#define TR_GVAR_HEADERS_FM5           "Valore su FM5"
#define TR_GVAR_HEADERS_FM6           "Valore su FM6"
#define TR_GVAR_HEADERS_FM7           "Valore su FM7"
#define TR_GVAR_HEADERS_FM8           "Valore su FM8"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS    { "Tipo o funzione di confronto", "Prima variabile", "Seconda variabile o costante", "Seconda variabile o costante", "Condizione aggiuntiva da abilitare linea", "Durata minima ON dell'interruttore logico", "Durata minima TRUE affinché l'interruttore si accenda" }

// Horus layouts and widgets
#define TR_FIRST_CHANNEL               "Primo canale"
#define TR_FILL_BACKGROUND             "Riempi lo sfondo?"
#define TR_BG_COLOR                    "Colore sfondo"
#define TR_SLIDERS_TRIMS               "Sliders+Trims"
#define TR_SLIDERS                     "Sliders"
#define TR_FLIGHT_MODE                 "Modo di volo"
#define TR_INVALID_FILE                "File invalido"
#define TR_TIMER_SOURCE                "Sorgente timer"
#define TR_SIZE                        "Dimensione"
#define TR_SHADOW                      "Ombra"
#define TR_TEXT                        "Testo"
#define TR_COLOR                       "Colore"
#define TR_MAIN_VIEW_X                 "Vista principale X"
#define TR_PANEL1_BACKGROUND           "Sfondo Panello 1"
#define TR_PANEL2_BACKGROUND           "Sfondo Panello 2"

// About screen
#define TR_ABOUTUS             TR(" INFO ", "INFO SU")

#define TR_CHR_SHORT  's'
#define TR_CHR_LONG   'l'
#define TR_CHR_TOGGLE 't'
#define TR_CHR_HOUR   'h'
#define TR_CHR_INPUT  'I'   // Values between A-I will work

#define TR_BEEP_VOLUME         "Volume Beep"
#define TR_WAV_VOLUME          "Volume Wav"
#define TR_BG_VOLUME           "Volume Sf"

#define TR_TOP_BAR             "Barra sup."
#define TR_FLASH_ERASE                 "Cancello flash..."
#define TR_FLASH_WRITE                 "Scrivo flash..."
#define TR_OTA_UPDATE                  "Aggiorn. OTA..."
#define TR_MODULE_RESET                "Reset modulo..."
#define TR_UNKNOWN_RX                  "RX sconociuta"
#define TR_UNSUPPORTED_RX              "RX non compatibile"
#define TR_OTA_UPDATE_ERROR            "Errore agg. OTA"
#define TR_DEVICE_RESET                "Resetto dispositivo..."
#define TR_ALTITUDE            INDENT "Altitudine"
#define TR_SCALE               "Scala"
#define TR_VIEW_CHANNELS       "Vedi Canali"
#define TR_VIEW_NOTES          "Vedi Note"
#define TR_MODEL_SELECT        "Seleziona Modello"
#define TR_MODS_FORBIDDEN      "Modifica proibita!"
#define TR_UNLOCKED            "Sbloccato"
#define TR_ID                  "ID"
#define TR_PRECISION           "Precisione"
#define TR_RATIO               "Ratio"
#define TR_FORMULA             "Formula"
#define TR_CELLINDEX           "Indice cella"
#define TR_LOGS                "Logs"
#define TR_OPTIONS             "Opzioni"
#define TR_FIRMWARE_OPTIONS    "Opzioni firmware"

#define TR_ALTSENSOR           "Sensore Alt"
#define TR_CELLSENSOR          "Sensore Cell"
#define TR_GPSSENSOR           "Sensore GPS"
#define TR_CURRENTSENSOR       "Sensore"
#define TR_AUTOOFFSET          "Offset Auto"
#define TR_ONLYPOSITIVE        "Positivo"
#define TR_FILTER              "Filtro"
#define TR_TELEMETRYFULL       "Tutti gli slot sono pieni!"
#define TR_SERVOS_OK           "Servi OK"
#define TR_SERVOS_KO           "Servi KO"
//TODO: translation
#define TR_INVERTED_SERIAL     INDENT "Invert."
#define TR_IGNORE_INSTANCE     TR(INDENT "No inst.", INDENT "Ignora instanza")
#define TR_DISCOVER_SENSORS    "Cerca nuovi sensori"
#define TR_STOP_DISCOVER_SENSORS "Ferma ricerca"
#define TR_DELETE_ALL_SENSORS  "Elimina tutti i sensori"
#define TR_CONFIRMDELETE       "Confermi " LCDW_128_480_LINEBREAK "eliminazione?"
#define TR_SELECT_WIDGET       "Seleziona widget"
#define TR_REMOVE_WIDGET       "Rimuovi widget"
#define TR_WIDGET_SETTINGS     "Settaggio widget"
#define TR_REMOVE_SCREEN       "Rimuovi schermo"
#define TR_SETUP_WIDGETS       "Setta widgets"
#define TR_USER_INTERFACE      "Interfaccia utente"
#define TR_THEME               "Tema"
#define TR_SETUP               "Imposta"
#define TR_LAYOUT              "Layout"
#define TR_ADD_MAIN_VIEW         "Aggiungi vista princ."
#define TR_BACKGROUND_COLOR    "Colore background"
#define TR_MAIN_COLOR          "Colore principale"
#define TR_BAR2_COLOR                  "Barra colore secondaria"
#define TR_BAR1_COLOR                  "Barra colore primaria"
#define TR_TEXT_COLOR                  "Colore testo"
#define TR_TEXT_VIEWER         "Visualizzatore testi"

#define TR_MENU_INPUTS          STR_CHAR_INPUT "Ingressi"
#define TR_MENU_LUA             STR_CHAR_LUA "Lua scripts"
#define TR_MENU_STICKS           STR_CHAR_STICK "Sticks"
#define TR_MENU_POTS            STR_CHAR_POT "Pots"
#define TR_MENU_MAX             STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI            STR_CHAR_CYC "Ciclico"
#define TR_MENU_TRIMS            STR_CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES        STR_CHAR_SWITCH "Interrut."
#define TR_MENU_LOGICAL_SWITCHES  STR_CHAR_SWITCH "Interrut. Logici"
#define TR_MENU_TRAINER         STR_CHAR_TRAINER "Trainer"
#define TR_MENU_CHANNELS        STR_CHAR_CHANNEL "Canali"
#define TR_MENU_GVARS           STR_CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY       STR_CHAR_TELEMETRY "Telemetry"
#define TR_MENU_DISPLAY        "DISPLAY"
#define TR_MENU_OTHER          "Altro"
#define TR_MENU_INVERT         "Inverti"
#define TR_JITTER_FILTER       "Filtro ADC"
#define TR_DEAD_ZONE           "Zona morta"
#define TR_RTC_CHECK           TR("Controllo RTC", "Controllo volt. RTC")
#define TR_AUTH_FAILURE        "Fallimento Auth"
#define TR_RACING_MODE         "Modo Racing"

#define STR_VFR               "VFR"
#define STR_RSSI              "RSSI"
#define STR_R9PW              "R9PW"
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
#define STR_GPSDATETIME       "Data"
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

#define TR_USE_THEME_COLOR              "Usa colore tema"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Agg. Trim a Subtrim"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS        "Apri Monitor Canali"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Apri Mon. Canali"
#endif
#define TR_DUPLICATE                    "Duplica"
#define TR_ACTIVATE                     "Setta Attivo"
#define TR_RED                          "Rosso"
#define TR_BLUE                         "Blu"
#define TR_GREEN                        "Verde"
#define TR_COLOR_PICKER                 "Picker Colori"
#define TR_EDIT_THEME_DETAILS           "Edita dett. tema"
#define TR_AUTHOR                       "Autore"
#define TR_DESCRIPTION                  "Descrizione"
#define TR_SAVE                         "Salva"
#define TR_CANCEL                       "Cancella"
#define TR_EDIT_THEME                   "EDITA TEMA"
#define TR_DETAILS                      "Dettagli"
#define TR_THEME_EDITOR                 "TEMI"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Seleziona\nModello"
#define TR_MAIN_MENU_MODEL_NOTES        "Note\nModello"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Monitor\nCanali"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Settaggi\nModello"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Settaggi\nRadio"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Settaggi\nSchermo"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Resetta\nTelemetria"
#define TR_MAIN_MENU_STATISTICS         "Statistiche"
#define TR_MAIN_MENU_ABOUT_EDGETX       "Riguardo\nEdgeTX"
// End Main menu
