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

// CZ translations author: Martin Hotar <mhotar@gmail.com>
// CZ translations author: Jan Urbanek @ rcstudio.cz

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 */

// NON ZERO TERMINATED STRINGS
#define TR_OFFON                       "VYP","ZAP"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Tichý",TR("Alarm","Jen Alarm"),TR("BezKl","Bez kláves"),"Vše"
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VBLMODE                     TR("Vyp","Vypnuto"),TR("Kláv.","Klávesy"),"Páky","Vše",TR("Zap","Zapnuto")
#define TR_TRNMODE                     "X","+=",":="
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES            "VYP","Telem Mirror","Telemetry In","SBUS Trenér","LUA","CLI","GPS","Debug"
#define TR_SWTYPES                     "Žádný","Bez aretace","2-polohový","3-polohový"
#define TR_POTTYPES                    "Žádný",TR("Pot s aret.","Pot s aretací"),TR("Vícepol př.","Vícepol. přep."),TR("Pot","Potenciometr")
#define TR_SLIDERTYPES                 "Žádný","Slider"
#define TR_VLCD                        "Normal","Optrex"
#define TR_VPERSISTENT                 "Ne","V rámci letu","Reset ručně"
#define TR_COUNTRY_CODES               TR("US","Amerika"),TR("JP","Japonsko"),TR("EU","Evropa")
#define TR_USBMODES                    TR("Zeptat","Zeptat se"),TR("Joyst","Joystick"),TR("SDkarta","Úložiště"),"Serial"
#define TR_JACK_MODES                  "Zeptat","Audio","Trenér"
#define TR_TELEMETRY_PROTOCOLS         "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_TELEMETRY_MODE        "Off","On","Off+Aux","On+Aux"
#define TR_MULTI_CUSTOM                "Vlastní"
#define TR_VTRIMINC                    "Expo","ExJemný","Jemný","Střední","Hrubý"
#define TR_VDISPLAYTRIMS               "Ne","Změna","Ano"
#define TR_VBEEPCOUNTDOWN              "Ne","Zvuk","Hlas","Vibrace"
#define TR_VVARIOCENTER                "Tón","Ticho"
#define TR_CURVE_TYPES                 "Rastr-X","Volná-XY"

#define TR_ADCFILTERVALUES             "Globální","Vyp","Zap",

#if defined(PCBX10)
  #define TR_RETA123                   "S","V","P","K","1","3","2","4","5","6","7","L","R"
#elif defined(PCBHORUS)
  #define TR_RETA123                   "S","V","P","K","1","3","2","4","5","L","R"
#elif defined(PCBX9E)
  #define TR_RETA123                   "S","V","P","K","1","2","3","4","L","R","L","R"
#elif defined(PCBTARANIS)
  #define TR_RETA123                   "S","V","P","K","1","2","3","L","R"
#else
  #define TR_RETA123                   "S","V","P","K","1","2","3"
#endif

#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Sečíst","Násobit","Zaměnit"
#define TR_VMLTPX2                     "+=","*=",":="

#if defined(PCBHORUS)
  #define TR_VMIXTRIMS                 "VYP","ZAP","Směr","Výšk","Plyn","Křid","T5","T6"
#else
  #define TR_VMIXTRIMS                 "VYP","ZAP","Směr","Výšk","Plyn","Křid"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Stopky"
  #define TR_CSWSTICKY                 "Sticky"
  #define TR_CSWRANGE                  "Dosah"
  #define TR_CSWSTAY                   "Edge"
#else
  #define TR_CSWTIMER                  "Tim"
  #define TR_CSWSTICKY                 "Stky"
  #define TR_CSWRANGE                  "Rnge"
  #define TR_CSWSTAY                   "Edge"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_TEXT_SIZE                   "Standard","Nejmenší","Malé","Střední","Dvojité"

#if defined(VARIO)
  #define TR_VVARIO                    "Vario"
#else
  #define TR_VVARIO                    "[Vario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND                     TR3("\221\222Zvuk", "\221\222Zvuk", "Hrát zvuk")
#else
  #define TR_SOUND                     "Pípnout"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC                    "Vibrovat"
#else
  #define TR_HAPTIC                    "[Vibrovat]"
#endif

#if defined(VOICE)
  #define TR_PLAY_TRACK              TR3("\221\222Stopa", "\221\222Stopa", "Přehrát wav")
  #define TR_PLAY_BOTH                 "\221\222Pár stop"
  #define TR_PLAY_VALUE                TR3("\221\222Hlásit ", "\221\222Hlásit ", "Hlásit stav")
#else
  #define TR_PLAY_TRACK                "[\221\222Stopa]"
  #define TR_PLAY_BOTH                 "[\221\222Pár]"
  #define TR_PLAY_VALUE                "[\221\222Hlásit]"
#endif

#define TR_SF_BG_MUSIC                 TR3("\221\222Hudba","\221\222Hudba","Hudba"),TR3("\221\222Hudba ||","\221\222Hudba ||","Hudba pauza")

#if defined(SDCARD)
  #define TR_SDCLOGS                   "Loguj na SD"
#else
  #define TR_SDCLOGS                   "[Logování]"
#endif

#ifdef GVARS
  #define TR_ADJUST_GVAR               "Nastav"
#else
  #define TR_ADJUST_GVAR               "[Nastav GP]"
#endif

#if defined(LUA)
  #define TR_SF_PLAY_SCRIPT            "Lua Skript"
#else
  #define TR_SF_PLAY_SCRIPT            "[Lua]"
#endif

#if defined(DEBUG)
  #define TR_SF_TEST                   "Test"
#else
  #define TR_SF_TEST
#endif

#if defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "Zámek"
#else
  #define TR_SF_SAFETY                 "---"
#endif

#define TR_SF_SCREENSHOT               "Snímek LCD"
#define TR_SF_RACING_MODE              "Závodní režim"
#define TR_SF_DISABLE_TOUCH            "Deaktivace dotyku"
#define TR_SF_RESERVE                  "[rezerva]"

#define TR_VFSWFUNC                    TR_SF_SAFETY,"Trenér","Insta-Trim","Reset","Změna",TR_ADJUST_GVAR,"Hlasitost","Nastavit Failsafe","Kontrola dosahu","ModuleBind",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH,TR_SF_TEST

#define TR_FSW_RESET_TELEM             TR("Telm","Telemetrie")

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS          "Stopky1","Stopky2","Stopky3"
#else
  #define TR_FSW_RESET_TIMERS          "Čas1","Čas2","Čas3"
#endif

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,"Vše",TR_FSW_RESET_TELEM
#define TR_FUNCSOUNDS                  TR("Píp1","Pípnutí1"),TR("Píp2","Pípnutí2"),TR("Píp3","Pípnutí3"),TR("Var1","Varování1"),TR("Var2","Varování2"),TR("Chee","Cheep"),TR("Rata", "Ratata"),"Tick",TR("Sirn","Siréna"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define TR_TELEM_RESERVE               TR("[--]", "[---]")
#define TR_TELEM_TIME                  TR("Čas", "H:M")
#define TR_RAS                         "SWR"
#define TR_RX_BATT                     TR("BtRx", "BatRx")
#define TR_A3_A4                       "A3","A4"
#define TR_A3_A4_MIN                   "A3-","A4-",

#define TR_ASPD_MAX                    TR("ASp+", "ASpd+")

#if LCD_W >= 212
  #define TR_TELEM_RSSI_RX             "RSSI"
#else
  #define TR_TELEM_RSSI_RX             "Rx"
#endif

#define TR_TELEM_TIMERS                "Čas1","Čas2","Čas3"

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                TR("Metr.","Metrické"),TR("Imper.","Imperial")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Nic","Hodnota","Ukazatel","Skript"
#define TR_GPSFORMAT                   "DMS","NMEA"

#define TR_TEMPLATE_CLEAR_MIXES        "Smazat mixy "
#define TR_TEMPLATE_SIMPLE_4CH         "Zákl. 4kanál"
#define TR_TEMPLATE_STICKY_TCUT        "Sticky-T-Cut"
#define TR_TEMPLATE_VTAIL              "V-Tail"
#define TR_TEMPLATE_DELTA              "Elevon\\Delta"
#define TR_TEMPLATE_ECCPM              "eCCPM"
#define TR_TEMPLATE_HELI               "Heli nastavení"
#define TR_TEMPLATE_SERVO_TEST         "Servo test"

#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICKS_VSRCRAW              STR_CHAR_STICK "Smě", STR_CHAR_STICK "Výš", STR_CHAR_STICK "Pln", STR_CHAR_STICK "Kři"

#if defined(PCBHORUS)
  #define TR_TRIMS_VSRCRAW             STR_CHAR_TRIM "Smě", STR_CHAR_TRIM "Výš", STR_CHAR_TRIM "Pln", STR_CHAR_TRIM "Kři", STR_CHAR_TRIM "T5", STR_CHAR_TRIM "T6"
#else
  #define TR_TRIMS_VSRCRAW             STR_CHAR_TRIM "Smě", STR_CHAR_TRIM "Výš", STR_CHAR_TRIM "Pln", STR_CHAR_TRIM "Kři"
#endif

#if defined(PCBHORUS)
  #define TR_TRIMS_SWITCHES            STR_CHAR_TRIM "Sl", STR_CHAR_TRIM "Sp", STR_CHAR_TRIM "Vd", STR_CHAR_TRIM "Vn", STR_CHAR_TRIM "Pd", STR_CHAR_TRIM "Pn", STR_CHAR_TRIM "Kl", STR_CHAR_TRIM "Kp", STR_CHAR_TRIM "5d", STR_CHAR_TRIM "5n", STR_CHAR_TRIM "6d", STR_CHAR_TRIM "6n"
#else
  #define TR_TRIMS_SWITCHES            TR("tSl",STR_CHAR_TRIM "Sl"),TR("tSp",STR_CHAR_TRIM "Sp"),TR("tVd",STR_CHAR_TRIM "Vd"),TR("tVn",STR_CHAR_TRIM "Vn"),TR("tPd",STR_CHAR_TRIM "Pd"),TR("tPn",STR_CHAR_TRIM "Pn"),TR("tKl",STR_CHAR_TRIM "Kl"),TR("tKp",STR_CHAR_TRIM "Kp")
#endif

#if defined(PCBHORUS) || defined(PCBNV14)
  #define TR_VKEYS                     "PGUP","PGDN","ENTER","MDL","RTN","TELE","SYS"
#elif defined(PCBXLITE)
  #define TR_VKEYS                     "Shift","Exit","Enter","Down","Up","Right","Left"
#elif defined(RADIO_FAMILY_JUMPER_T12)
  #define TR_VKEYS                     "Exit","Enter","Down","Up","Right","Left"
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define TR_VKEYS                     "Exit","Enter","PGUP","PGDN","SYS","MDL","TELE"
#elif defined(RADIO_T8) || defined(RADIO_COMMANDO8)
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

#define TR_ON_ONE_SWITCHES             "ZAP","One"

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Normal","Inverted"
#else
  #define TR_ROTARY_ENC_OPT         "Normal","Inverted","V-I H-N","V-I H-A"
#endif

#if defined(IMU)
  #define TR_IMU_VSRCRAW               "TltX","TltY",
#else
  #define TR_IMU_VSRCRAW
#endif

#if defined(HELI)
  #define TR_CYC_VSRCRAW               "CYC1","CYC2","CYC3"
#else
  #define TR_CYC_VSRCRAW               "[C1]","[C2]","[C3]"
#endif

#define TR_RESERVE_VSRCRAW             "[--]"
#define TR_EXTRA_VSRCRAW               "Bat.","Čas","GPS",TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,"Čas1","Čas2","Čas3"

#define TR_VTMRMODES                   "VYP","ZAP","Pln>","Pln%","Pln*"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Učitel/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Žák/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Učitel/SBUS Modul"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Učitel/CPPM Modul"
#define TR_VTRAINER_MASTER_BATTERY     "Učitel/Serial"
#define TR_VTRAINER_BLUETOOTH          TR("Učitel/BT","Učitel/Bluetooth"),TR("Žák/BT","Žák/Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VFAILSAFE                   "Nenastaven","Držet","Vlastní","Bez pulzů","Přijímač"
#define TR_VSENSORTYPES                "Vlastní","Vypočtený"
#define TR_VFORMULAS                   "Součet","Průměr","Min","Max","Násobení","Celkem","Článek","Spotřeba","Vzdálenost"
#define TR_VPREC                       "X","X.X","X.XX"
#define TR_VCELLINDEX                  "Nízký","1","2","3","4","5","6","Nejvíce","Delta"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA"(pouze středy)","= (symetrický)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")

// ZERO TERMINATED STRINGS
#if defined(COLORLCD)
  #define INDENT                       "\007"
  #define LEN_INDENT                   1
  #define INDENT_WIDTH                 12
  #define BREAKSPACE                   "\036"
#else
#define INDENT                         "\001"
#define LEN_INDENT                     1
#define INDENT_WIDTH                   (FW/2)
#define BREAKSPACE                     " "
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
  #define TR_ENTER                     "[DALŠÍ]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#if defined(PCBHORUS)
  #define TR_EXIT                      "RTN"
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "EXIT"
  #define TR_OK                        "\010" "\010" "\010" TR("[OK]", "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_MENUWHENDONE                CENTER "\011" TR_ENTER " > DALŠÍ"
#define TR_FREE                        TR("volné:", "volných")
#define TR_YES                         "Yes"
#define TR_NO                          "No"
#define TR_DELETEMODEL                 "SMAZAT" BREAKSPACE "MODEL"
#define TR_COPYINGMODEL                "Kopíruji model..."
#define TR_MOVINGMODEL                 "Přesouvám model..."
#define TR_LOADINGMODEL                "Aktivuji model..."
#define TR_NAME                        "Název"
#define TR_MODELNAME                   TR("Model", "Název modelu")
#define TR_PHASENAME                   "Název"
#define TR_MIXNAME                     "Název"
#define TR_INPUTNAME                   "Název"
#define TR_EXPONAME                    "Popis"
#define TR_BITMAP                      "Obrázek"
#define TR_TIMER                       "Stopky"
#define TR_START                       "Start"
#define TR_ELIMITS                     TR("Limit +25%", "Kanál +/- 125%")
#define TR_ETRIMS                      TR("širší Trim", "Široký trim")
#define TR_TRIMINC                     TR("Krok Trimu", "Krok trimu")
#define TR_DISPLAY_TRIMS               TR3("Číslo v Trm", "Zobr.hodnotu trimu", "Číslo v liště trimu")
#define TR_TTRACE                      TR("StopaPlynu", INDENT "Stopa plynu")
#define TR_TTRIM                       TR3("TrimVolnob.", INDENT "Trim jen volnoběh", "Trim jen pro volnoběh")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", INDENT "Trim spínač")
#define TR_BEEPCTR                     TR3("Středy \221\222", "Pípat středy \221\222", "Pípat středy")
#define TR_USE_GLOBAL_FUNCS            TR("Glob.Funkce", "Použít globální funkce")
#define TR_PROTOCOL                    "Protokol"
#define TR_PPMFRAME                    INDENT "PPM modulace"
#define TR_REFRESHRATE                 TR(INDENT "Obnovit", INDENT "Obn. frekv.")
#define STR_WARN_BATTVOLTAGE           TR(INDENT "Výstup VBAT: ", INDENT "Varování: výstupní hodnota VBAT: ")
#define TR_WARN_5VOLTS                 "Varování: výstupní úroveň je 5 voltů"
#define TR_MS                          "ms"
#define TR_FREQUENCY                   INDENT "Frekvence"
#define TR_SWITCH                      "Spínač"
#define TR_TRIMS                       "Trimy"
#define TR_FADEIN                      "Přechod Zap"
#define TR_FADEOUT                     "Přechod Vyp"
#define TR_DEFAULT                     "(výchozí)"
#define TR_CHECKTRIMS                  "\011Kont.\010Trimy"
#define OFS_CHECKTRIMS                 (9*FW)
#define TR_SWASHTYPE                   "Typ cykliky"
#define TR_COLLECTIVE                  "Kolektiv"
#define TR_AILERON                     "Boční cyklika"
#define TR_ELEVATOR                    TR3("Podélná cykl.", "Podélná cykl.", "Podélná cyklika")
#define TR_SWASHRING                   "Cyklika"
#define TR_ELEDIRECTION                "Směr:Výškovka"
#define TR_AILDIRECTION                "\012Křidélka"
#define TR_COLDIRECTION                "\012Kolektiv"
#define TR_MODE                        "Mód"
#define TR_SUBTYPE                     INDENT "Subtyp"
#define TR_NOFREEEXPO                  "Není volné expo!"
#define TR_NOFREEMIXER                 "Není volný mix!"
#define TR_SOURCE                      "Zdroj"
#define TR_WEIGHT                      "Váha"
#define TR_EXPO                        "Expo"
#define TR_SIDE                        "Strana"
#define TR_DIFFERENTIAL                "Dif.výchylek"
#define TR_OFFSET                      "Ofset"
#define TR_TRIM                        "Trim"
#define TR_DREX                        "DR/Expo"
#define DREX_CHBOX_OFFSET              45
#define TR_CURVE                       "Křivka"
#define TR_FLMODE                      "Režim"
#define TR_MIXWARNING                  "Varování"
#define TR_OFF                         "VYP"
#define TR_ANTENNA                     "Anténa"
#define TR_NO_INFORMATION              TR("Není info.", "Žádná informace")
#define TR_MULTPX                      TR("Mat.operace", "Operace")
#define TR_DELAYDOWN                   TR3("Zpoždění Vyp", "Zdržet(x)", "Zpoždění Vyp")
#define TR_DELAYUP                     TR3("Zpoždění Zap", "Zdržet(\43)", "Zpoždění Zap")
#define TR_SLOWDOWN                    TR3("Zpomalení(-)", "Zpomal(\177)", "Zpomalení(\177)")
#define TR_SLOWUP                      TR3("Zpomalení(+)", "Zpomal(\176)", "Zpomalení(\176)")
#define TR_MIXES                       "MIXER"
#define TR_CV                          "K"
#if defined(PCBNV14)
#define TR_GV                          "GP"
#else
#define TR_GV                          TR("G", "GP")
#endif
#define TR_ACHANNEL                    "A\004Kanál"
#define TR_RANGE                       INDENT"Rozsah"
#define TR_CENTER                      INDENT "Střed"
#define TR_BAR                         "Ukazatel"
#define TR_ALARM                       "Alarm"
#define TR_USRDATA                     TR("UživData", "Uživ. data")
#define TR_BLADES                      TR("ListyVrt", "Listy vrtule")
#define TR_SCREEN                      "Panel "
#define TR_SOUND_LABEL                 "Zvuk"
#define TR_LENGTH                      "Délka"
#define TR_BEEP_LENGTH                 TR3("Délka", "Délka", "Délka zvuku")
#define TR_BEEP_PITCH                  "Tón"
#define TR_HAPTIC_LABEL                "Vibrace"
#define TR_STRENGTH                    "Intenzita"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Offset"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Kontrast LCD"
#define TR_ALARMS_LABEL                "Alarmy"
#define TR_BATTERY_RANGE               TR("Ukazatel bat.", "Ukazatel baterie")
#define TR_BATTERYCHARGING             "Nabíjení..."
#define TR_BATTERYFULL                 "Baterie nabitá"
#define TR_BATTERYNONE                 "Žádná baterie!"
#define TR_BATTERYWARNING              "Vybitá baterie"
#define TR_INACTIVITYALARM             TR("Nečinnost", "Nečinnost rádia")
#define TR_MEMORYWARNING               "Plná pamět'"
#define TR_ALARMWARNING                TR3("Vypnutý zvuk", "Upozornit na vypnutý zvuk", "Upozornit na vyp. zvuk")
#define TR_RSSI_SHUTDOWN_ALARM         TR("Rssi při vyp.", "Hlídat RSSI při vypnutí")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Použít FlySky RSSI hodnotu bez škálování")
#define TR_MODEL_STILL_POWERED         "Model stále spuštěn"
#define TR_USB_STILL_CONNECTED         "USB stále připojeno"
#define TR_MODEL_SHUTDOWN              "Vypnout?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Zmáčkni ENTER pro potvrzení"
#define TR_THROTTLE_LABEL              "Plyn"
#define TR_THROTTLEREVERSE             TR("ReversPlyn", INDENT"Revers plynu")
#define TR_MINUTEBEEP                  TR("Minuta", "Oznamovat minuty")
#define TR_BEEPCOUNTDOWN               INDENT"Odpočet"
#define TR_PERSISTENT                  INDENT"Trvalé"
#define TR_BACKLIGHT_LABEL             "Podsvětlení"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Stav"
#define TR_BLDELAY                     INDENT"Zhasnout po"
#define TR_BLONBRIGHTNESS              TR3(INDENT"Jas Zap.", INDENT"Jas Zap.", INDENT"Jas zapnutého LCD")
#define TR_BLOFFBRIGHTNESS             TR3(INDENT"Jas Vyp.", INDENT"Jas Vyp.", INDENT"Jas vypnutého LCD")
#define TR_KEYS_BACKLIGHT              "Podsvětlení kláves"
#define TR_BLCOLOR                     "Barva"
#define TR_SPLASHSCREEN                TR("úvodní logo", "Zobrazit úvodní logo")
#define TR_PWR_ON_DELAY                "Zpoždění zapnutí"
#define TR_PWR_OFF_DELAY               "Zpoždění vypnutí"
#define TR_THROTTLE_WARNING            TR("* Plyn", INDENT "Kontrola plynu")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Vlas-Poz", INDENT INDENT INDENT INDENT "Vlastní pozice?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Poz. %", "Pozice %")
#define TR_SWITCHWARNING               TR("* Spínače", INDENT "Polohy spínačů")
#define TR_POTWARNINGSTATE             TR("* Pot&Slid.", INDENT "Kontrola Pot&Slid.")
#define TR_SLIDERWARNING               TR("* Slidery", INDENT "Pozice sliderů")
#define TR_POTWARNING                  TR("* Potenc.", INDENT "Potenciometry")
#define TR_TIMEZONE                    "Časové pásmo"
#define TR_ADJUST_RTC                  TR3("Čas z GPS", "Použít čas z GPS", "Použít čas z GPS")
#define TR_GPS                         "GPS"
#define TR_RXCHANNELORD                TR("Pořadí kanálů", "Výchozí pořadí kanálů")
#define TR_STICKS                      "Páky"
#define TR_POTS                        "Potenciometry"
#define TR_SWITCHES                    "Spínače"
#define TR_SWITCHES_DELAY              TR3("Filtr přepínače", "Filtr přepínače", "Filtr poloh přepínače")
#define TR_SLAVE                       "Podřízený"
#define TR_MODESRC                     "Mód\006% Zdroj"
#define TR_MULTIPLIER                  "Násobič"
#define TR_CAL                         "Kal."
#define TR_CALIBRATION                 "Kalibrace"
#define TR_VTRIM                       "Trim - +"
#define TR_BG                          "Poz:"
#define TR_CALIB_DONE                  "Kalibrace dokončena"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Stiskem [ENTER] začneme"
  #define TR_SETMIDPOINT               "Vycentruj páky/poty/slidery a stiskni [ENTER]"
  #define TR_MOVESTICKSPOTS            "Hýbej pákami/poty/slidery pak stiskni [ENTER]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER "= START"
  #define TR_SETMIDPOINT               "Nastav páky na střed"
  #define TR_MOVESTICKSPOTS            "Hýbej pákami/poty"
#else
  #define TR_MENUTOSTART               CENTER "\011" TR_ENTER " = START"
  #define TR_SETMIDPOINT               CENTER "\004Nastav páky na střed"
  #define TR_MOVESTICKSPOTS            TR(CENTER "\006Hýbej pákami/poty", "\014Hýbej pákami i potenciometry")
#endif
#define TR_RXBATT                      "Rx Bat.:"
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_ACCEL                       "Acc:"
#define TR_NODATA                      CENTER "ŽÁDNÁ DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMR1LATMINUS                "Tmr1Lat min\037\124us"

#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "Vnitřní GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua skripty"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_TIMER_LABEL                 "Časovač"
#define TR_THROTTLE_PERCENT_LABEL      "Plyn %"
#define TR_BATT_LABEL                  "Baterie"
#define TR_SESSION                     "Relace"

#define TR_MENUTORESET                 TR_ENTER" >> Reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODEL"
#define TR_FM                          "LR"
#define TR_MIX                         "MIX"
#define TR_EEPROMLOWMEM                "Dochází EEPROM"
#define TR_PRESS_ANY_KEY_TO_SKIP       TR("\003Klávesa >>> přeskočit", "Klávesa >>> přeskočit")
#define TR_THROTTLE_NOT_IDLE           TR("\003Páka plynu je pohnutá", "Páka plynu není na nule")
#define TR_ALARMSDISABLED              "Alarmy jsou zakázány"
#define TR_PRESSANYKEY                 TR("\006Stiskni klávesu", "Stiskni klávesu")
#define TR_BADEEPROMDATA               TR("\006Chyba dat EEPROM", "Chyba dat EEPROM")
#define TR_BAD_RADIO_DATA              "Chybná data rádia"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            TR("\004Formatování EEPROM", "Formatování EEPROM")
#define TR_STORAGE_FORMAT              "Storage Preparation"
#define TR_EEPROMOVERFLOW              "Přetekla EEPROM"
#define TR_RADIO_SETUP                 "NASTAVENÍ RÁDIA"
#define TR_MENUDATEANDTIME             "DATUM A ČAS"
#define TR_MENUTRAINER                 "TRENÉR"
#define TR_MENUSPECIALFUNCS            "GLOBÁLNÍ FUNKCE"
#define TR_MENUVERSION                 "VERZE"
#define TR_MENU_RADIO_SWITCHES         "DIAG"
#define TR_MENU_RADIO_ANALOGS          "ANALOGY"
#define TR_MENU_RADIO_ANALOGS_CALIB    "KALIBROVAT ANALOGY"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGY (5 Hz)"
#define TR_MENUCALIBRATION             "KALIBRACE"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trimy => Subtrimy"
#else
  #define TR_TRIMS2OFFSETS             "\006Trimy => Subtrimy"
#endif
#define TR_CHANNELS2FAILSAFE           "Kanály=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Kanál=>Failsafe"
#define TR_MENUMODELSEL                "MODEL"
#define TR_MENU_MODEL_SETUP            "NASTAVENÍ"
#define TR_MENUFLIGHTMODE              "LETOVÝ REŽIM"
#define TR_MENUFLIGHTMODES             "LETOVÉ REŽIMY"
#define TR_MENUHELISETUP               "HELI"

#define TR_MENULIMITS                  "SERVA"
#define TR_MENUINPUTS                  "VSTUPY"

#define TR_MENUCURVES                  "KŘIVKY"
#define TR_MENUCURVE                   "\002K"
#define TR_MENULOGICALSWITCH           "LOG. SPÍNAČ"
#define TR_MENULOGICALSWITCHES         "LOGICKÉ SPÍNAČE"
#define TR_MENUCUSTOMFUNC              "SPECIÁLNÍ FUNKCE"
#define TR_MENUCUSTOMSCRIPTS           "SKRIPTY LUA"
#define TR_MENUTELEMETRY               "TELEMETRIE"
#define TR_MENUTEMPLATES               "ŠABLONY"
#define TR_MENUSTAT                    "STATISTIKA"
#define TR_MENUDEBUG                   "DIAG"
#define TR_MONITOR_CHANNELS1           "MONITOR KANÁLŮ 1/8"
#define TR_MONITOR_CHANNELS2           "MONITOR KANÁLŮ 9/16"
#define TR_MONITOR_CHANNELS3           "MONITOR KANÁLŮ 17/24"
#define TR_MONITOR_CHANNELS4           "MONITOR KANÁLŮ 25/32"
#define TR_MONITOR_SWITCHES            "MONITOR LOGICKÝCH SPÍNAČŮ"
#define TR_MONITOR_OUTPUT_DESC         "Výstupy"
#define TR_MONITOR_MIXER_DESC          "Mixy"
#define TR_RECEIVER_NUM                TR("RX číslo", "Číslo přijímače")
#define TR_RECEIVER                    "Přijímač"
#define TR_MULTI_RFTUNE                TR(INDENT "Ladění frek", INDENT "Jemné ladění frek. RF")
#define TR_MULTI_RFPOWER               "RF výkon"
#define TR_MULTI_WBUS                  "Výstup"
#define TR_MULTI_TELEMETRY             "Telemetrie"
#define TR_MULTI_VIDFREQ               TR(INDENT "Freq. videa", INDENT "Frekvence videa")
#define TR_RF_POWER                     INDENT "Výkon RF"
#define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")
#define TR_MULTI_OPTION                TR(INDENT "Možnosti", INDENT "Hodnota")
#define TR_MULTI_AUTOBIND              TR(INDENT "Bind Ch.",INDENT "Bind při zapnutí")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Vypnout mapování kanálů")
#define TR_DISABLE_TELEM               TR("No Telem", "Vypnout telemetrii")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "Autodetekce", INDENT "Formát autodetekce")
#define TR_MULTI_LOWPOWER              TR(INDENT "Nízký výkon", INDENT "Režim nízkého výkonu")
#define TR_MULTI_LNA_DISABLE            INDENT "LNA vypnuto"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")
#define TR_MODULE_TELEM_ON             TR("Zap", "Zapnuto")
#define TR_DISABLE_INTERNAL            TR("Vypnout int.", "Vypnout interní RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("Nesériový", "Není v sériovém režimu")
#define TR_MODULE_NO_INPUT             TR("Žádný vstup", "Žádný sériový vstup")
#define TR_MODULE_NO_TELEMETRY         TR3("Bez telemetrie", "Bez MULTI_TELEMETIE", "Nedetekována MULTI_TELEMETRIE")
#define TR_MODULE_WAITFORBIND          "Párovat při zavedení protokolu"
#define TR_MODULE_BINDING              TR("Bind...","Párování")
#define TR_MODULE_UPGRADE_ALERT        TR3("Upg. needed", "Aktualizace modulu vyžadována", "Modul\naktualizace vyžadována")
#define TR_MODULE_UPGRADE              TR("Upg. advised", "Aktualizace modulu doporučena")
#define TR_REBIND                      "Nutno přepárovat"
#define TR_REG_OK                      "Registrace ok"
#define TR_BIND_OK                     "Úspěšné párovaní"
#define TR_BINDING_CH1_8_TELEM_ON      "k1-8 telem zap"
#define TR_BINDING_CH1_8_TELEM_OFF     "k1-8 telem vyp"
#define TR_BINDING_CH9_16_TELEM_ON     "k9-16 telem zap"
#define TR_BINDING_CH9_16_TELEM_OFF    "k9-16 telem vyp"
#define TR_PROTOCOL_INVALID            TR("Špat. protokol", "Špatný protokol")
#define TR_MODULE_STATUS               TR(INDENT "Stav", INDENT "Stav modulu")
#define TR_MODULE_SYNC                 TR(INDENT "Sync", INDENT "Proto Sync stav")
#define TR_MULTI_SERVOFREQ             TR(INDENT "Frekv. serva", INDENT "Obnovovací frequence serva")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Povolit max. throw")
#define TR_MULTI_RFCHAN                TR("RF Kanál", "Vybrat RF kanál")
#define TR_SYNCMENU                    "[Sync]"
#define TR_LIMIT                       INDENT"Limit"
#define TR_MINRSSI                     "Min RSSI"
#define TR_LATITUDE                    "Zem. šířka"
#define TR_LONGITUDE                   "Zem. délka"
#define TR_GPS_COORDS_FORMAT           "GPS souřadnice"
#define TR_VARIO                       "Vario"
#define TR_PITCH_AT_ZERO               "Tón na nule"
#define TR_PITCH_AT_MAX                "Tón na maximu"
#define TR_REPEAT_AT_ZERO              TR("Opak. na nule", "Opakování na nule")
#define TR_SHUTDOWN                    "Vypínání.."
#define TR_SAVE_MODEL                  "Ukládám nastavení modelu"
#define TR_BATT_CALIB                  "Kalib. bat."
#define TR_CURRENT_CALIB               "+=\006Proud"
#define TR_VOLTAGE                     INDENT"Napětí"
#define TR_CURRENT                     INDENT"Proud"
#define TR_SELECT_MODEL                "Vyber model"
#define TR_MODEL_CATEGORIES            "Kategorie modelů"
#define TR_MODELS                      "Modely"
#define TR_SELECT_MODE                 "Vybrat mód"
#define TR_CREATE_CATEGORY             "Nová kategorie"
#define TR_RENAME_CATEGORY             "Přejmenovat kategorii"
#define TR_DELETE_CATEGORY             "Odstranit kategorii"
#define TR_CREATE_MODEL                "Nový model"
#define TR_DUPLICATE_MODEL             "Duplikovat model"
#define TR_COPY_MODEL                  "Kopírovat"
#define TR_MOVE_MODEL                  "Přesunout model"
#define TR_BACKUP_MODEL                "Zálohovat na SD"
#define TR_DELETE_MODEL                "Smaž model"
#define TR_RESTORE_MODEL               "Obnov model z SD"
#define TR_DELETE_ERROR                "Nelze odstranit"
#define TR_CAT_NOT_EMPTY               "Kategorie není prázdná"
#define TR_SDCARD_ERROR                "Chyba SD karty"
#define TR_SDCARD                      "SD karta"
#define TR_NO_FILES_ON_SD              "Žádné soubory na SD kartě!"
#define TR_NO_SDCARD                   "Není SD karta"
#define TR_WAITING_FOR_RX              "Čekám na RX..."
#define TR_WAITING_FOR_TX              "Čekám na TX..."
#define TR_WAITING_FOR_MODULE          TR("Čekání modul", "Čekání na modul...")
#define TR_NO_TOOLS                    TR("Nejsou k dispozici", "Žádné nástroje k dispozici")
#define TR_NORMAL                      "Normální"
#define TR_NOT_INVERTED                "Neinvert"
#define TR_NOT_CONNECTED               "Nepřipojen"
#define TR_CONNECTED                   "Připojen"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16k bez telem.", "16k bez telemetrie")
#define TR_16CH_WITH_TELEMETRY         TR("16k s telem.", "16k s telemetrií")
#define TR_8CH_WITH_TELEMETRY          TR("8k s telem.", "8k s telemetrií")
#define TR_EXT_ANTENNA                 "Ext. anténa"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Uložit nastavení?"
#define TR_UPDATE_TX_OPTIONS           "Uložit nastavení?"
#define TR_MODULES_RX_VERSION          "Verze modulu a RX"
#define TR_MENU_MODULES_RX_VERSION     "VERZE MODULU A RX"
#define TR_MENU_FIRM_OPTIONS           "MOŽNOSTI FIRMWARE"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "páky/pot./slidery"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM páky/pot./slidery"
#define TR_RF_PROTOCOL                 "RF Protokol"
#define TR_MODULE_OPTIONS              "Možnosti modulu"
#define TR_POWER                       "Výkon"
#define TR_NO_TX_OPTIONS               "Žádné možnosti"
#define TR_RTC_BATT                    "RTC Bat"
#define TR_POWER_METER_EXT             "Měřič výkonu (EXT)"
#define TR_POWER_METER_INT             "Měřič výkonu (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spektální an. (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Spektální an. (INT)"
#define TR_SDCARD_FULL                 "Plná SD karta"
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Nekompatibilní"
#define TR_WARNING                     "KONTROLA"
#define TR_EEPROMWARN                  "EEPROM"
#define TR_STORAGE_WARNING             "ÚLOŽIŠTĚ"
#define TR_EEPROM_CONVERTING           "Aktualizuji EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD karta vyžaduje konverzi"
#define TR_CONVERTING                  "Konverze: "
#define TR_THROTTLE_UPPERCASE          "PLYNU"
#define TR_ALARMSWARN                  "ALARMU"
#define TR_SWITCHWARN                  "POZICE"
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("NOČNÍ", "NOČNÍ SESTAVENÍ")
#define TR_TEST_NOTSAFE                "Pouze pro testování"
#define TR_WRONG_SDCARDVERSION         "Očekávaná ver.: "
#define TR_WARN_RTC_BATTERY_LOW        "Slabá RTC baterie"
#define TR_WARN_MULTI_LOWPOWER         "Režim nízkého výkonu"
#define TR_BATTERY                     "BATERIE"
#define TR_WRONG_PCBREV                "Jiná verze PCB/firmware"
#define TR_EMERGENCY_MODE              "NOUZOVÝ REŽIM"
#define TR_PCBREV_ERROR                "Chyba PCB"
#define TR_NO_FAILSAFE                 "Failsafe není nastaveno"
#define TR_KEYSTUCK                    "Zaseklá klávesa"
#define TR_INVERT_THR                  "Invertovat plyn?"
#define TR_VOLUME                      "Hlasitost"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Jas"
#define TR_CPU_TEMP                    "Tepl. CPU\016>"
#define TR_CPU_CURRENT                 "Proud\030>"
#define TR_CPU_MAH                     "Spotřeba"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "Tepl. MB \016>"
#define TR_CAPAWARNING                 INDENT "Nízká kapacita"
#define TR_TEMPWARNING                 INDENT "Přehřátí"
#define TR_TTL_WARNING                 "Varování: používat 3.3V hodnoty"
#define TR_FUNC                        "Fce."
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Trvání"
#define TR_DELAY                       "Zdržet"
#define TR_SD_CARD                     "SD"
#define TR_SDHC_CARD                   "SD-HC"
#define TR_NO_SOUNDS_ON_SD             "žádný zvuk" BREAKSPACE "na SD"
#define TR_NO_MODELS_ON_SD             "žádný model" BREAKSPACE "na SD"
#define TR_NO_BITMAPS_ON_SD            "žádné obrázky" BREAKSPACE "na SD"
#define TR_NO_SCRIPTS_ON_SD            "žádný skript" BREAKSPACE "na SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntaktická chyba", "Syntaktická chyba skriptu")
#define TR_SCRIPT_PANIC                "Script zmaten"
#define TR_SCRIPT_KILLED               "Script ukončen"
#define TR_SCRIPT_ERROR                "Neznámá chyba"
#define TR_PLAY_FILE                   "Přehrát"
#define TR_DELETE_FILE                 "Odstranit"
#define TR_COPY_FILE                   "Kopírovat"
#define TR_RENAME_FILE                 "Přejmenovat"
#define TR_ASSIGN_BITMAP               "Zvolit obrázek"
#define TR_ASSIGN_SPLASH               "Úvodní obrazovka"
#define TR_EXECUTE_FILE                "Spustit"
#define TR_REMOVED                     "odstraněn"
#define TR_SD_INFO                     "Informace"
#define TR_SD_FORMAT                   "Formát"
#define TR_NA                          "[X]"
#define TR_HARDWARE                    "HARDWARE"
#define TR_FORMATTING                  "Formátovaní..."
#define TR_TEMP_CALIB                  "+=\006Teplota"
#define TR_TIME                        "Čas"
#define TR_MAXBAUDRATE                 "Max baudů"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE                 "Sample Mode"
#define TR_SAMPLE_MODES                "Normal","OneBit"

#define TR_SELECT_TEMPLATE_FOLDER      "VYBRAT SLOŽKU SE ŠABLONOU:"
#define TR_SELECT_TEMPLATE             "VYBRAT ŠABLONU MODELU:"
#define TR_NO_TEMPLATES                "Žádná šablona modelu v této složce nebyla nalezena"
#define TR_SAVE_TEMPLATE               "Uložit jako šablonu"
#define TR_BLANK_MODEL                 "Prázdný model"
#define TR_BLANK_MODEL_INFO            "Vytvořit prázdný model"
#define TR_FILE_EXISTS                 "SOUBOR JIŽ EXISTUJE"
#define TR_ASK_OVERWRITE               "Chcete přepsat?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Hledat"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Vzdál. addr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Lokál. addr"
#define TR_BLUETOOTH_PIN_CODE          TR("PIN kód", "PIN kód")
#define TR_BLUETOOTH_NODEVICES         "No Devices Found"
#define TR_BLUETOOTH_SCANNING          "Scanning..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Povoleno"
#else
#define TR_BLUETOOTH_MODES             "---","Telemetrie","Trenér  "
#endif
#define TR_SD_INFO_TITLE               "SD INFO"
#define TR_SD_TYPE                     "Typ:"
#define TR_SD_SPEED                    "Rychlost:"
#define TR_SD_SECTORS                  "Sektorů :"
#define TR_SD_SIZE                     "Velikost:"
#define TR_TYPE                        INDENT TR_SD_TYPE
#define TR_GLOBAL_VARS                 "Globální proměnné"
#define TR_GVARS                       "GLOB.PROM."
#define TR_GLOBAL_VAR                  "Globální proměnná"
#define TR_MENU_GLOBAL_VARS            "GLOBÁLNÍ PROMĚNNÉ"
#define TR_OWN                         "\043"
#define TR_DATE                        "Datum"
#define TR_MONTHS                      { "Led", "Úno", "Bře", "Dub", "Kvě", "Čvn", "Čvc", "Srp", "Zář", "Říj", "Lis", "Pro" }
#define TR_ROTARY_ENCODER              "R.Enko"
#define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR            "MONITOR KANÁLU"
#define TR_MIXERS_MONITOR              "MONITOR MIXU"
#define TR_PATH_TOO_LONG               "Cesta je moc dlouhá"
#define TR_VIEW_TEXT                   "Zobrazit text"
#define TR_FLASH_BOOTLOADER            "Flash bootloaderu"
#define TR_FLASH_DEVICE                TR("Flash zař.","Flash zařízení")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Flash S.Portem", "Flash S.Portem")
#define TR_FLASH_RECEIVER_OTA          "Flash příjímače OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash modulu BT", "Flash modulu Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT TR("Flash řízení spotř.", "Flash jednotky řízení spotřeby")
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "Zařízení neodpovídá")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Device file prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "Device data refused")
#define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "Device access problem")
#define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "Device file refused")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", "Device file sig.")
#define TR_CURRENT_VERSION             TR("Současná ver. ", "Současná verze: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Flash vnitř. modulu", "Flash vnitřního modulu")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Int. Multi", "Flash Internal Multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash ext. modulu", "Flash externího zařízení")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Ext. Multi", "Flash External Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flash External ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Chyba zápisu FW", "Chyba zápisu firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Úspěšný zápis FW"
#define TR_WRITING                     "Zapisuji..."
#define TR_CONFIRM_FORMAT              "Formátovat?"
#define TR_INTERNALRF                  "Interní RF modul"
#define TR_INTERNAL_MODULE             "Interní modul"
#define TR_EXTERNAL_MODULE             "Externí modul"
#define TR_OPENTX_UPGRADE_REQUIRED     TR("Aktualizujte EdgeTX", "Vyžadována aktualizace EdgeTX")
#define TR_TELEMETRY_DISABLED          "Telem. zakázána"
#define TR_MORE_OPTIONS_AVAILABLE      TR("Více možností", "Více dostupných možností")
#define TR_NO_MODULE_INFORMATION       "Žádné info. o modulu"
#define TR_EXTERNALRF                  "Externí RF modul"
#define TR_FAILSAFE                    TR("Failsafe", "Mód Failsafe")
#define TR_FAILSAFESET                 "NASTAVENÍ FAILSAFE"
#define TR_REG_ID                      TR("Reg. ID", "Registrační ID")
#define TR_OWNER_ID                    "ID vlastníka"
#define TR_PINMAPSET                   "PINMAP"
#define TR_HOLD                        "Držet"
#define TR_HOLD_UPPERCASE              "DRŽET"
#define TR_NONE                        "Nic"
#define TR_NONE_UPPERCASE              "NIC"
#define TR_MENUSENSOR                  "SENZOR"
#define TR_POWERMETER_PEAK             "Špička"
#define TR_POWERMETER_POWER            "Výkon"
#define TR_POWERMETER_ATTN             "Útlum"
#define TR_POWERMETER_FREQ             "Frek."
#define TR_MENUTOOLS                   "NÁSTROJE"
#define TR_TURN_OFF_RECEIVER           "Vypněte přijímač"
#define TR_STOPPING                    "Zastavuji..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPEKTRÁLNÍ ANALYZER"
#define TR_MENU_POWER_METER            "MĚŘIČ VÝKONU"
#define TR_SENSOR                      "SENZOR"
#define TR_COUNTRY_CODE                "Kód regionu"
#define TR_USBMODE                     "Režim USB"
#define TR_JACK_MODE                   "Režim Jack"
#define TR_VOICE_LANGUAGE              "Jazyk hlasu"
#define TR_UNITS_SYSTEM                "Jednotky"
#define TR_EDIT                        "Upravit"
#define TR_INSERT_BEFORE               "Vložit před"
#define TR_INSERT_AFTER                "Vložit za"
#define TR_COPY                        "Kopírovat"
#define TR_MOVE                        "Přesunout"
#define TR_PASTE                       "Vložit"
#define TR_PASTE_AFTER                 "Vložit za"
#define TR_PASTE_BEFORE                "Vložit před"
#define TR_DELETE                      "Odstranit"
#define TR_INSERT                      "Přidat"
#define TR_RESET_FLIGHT                "Let"
#define TR_RESET_TIMER1                "Čas1"
#define TR_RESET_TIMER2                "Čas2"
#define TR_RESET_TIMER3                "Čas3"
#define TR_RESET_TELEMETRY             "Telemetrii"
#define TR_STATISTICS                  "Statistika"
#define TR_ABOUT_US                    "O nás"
#define TR_USB_JOYSTICK                "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB Disk (SD)"
#define TR_USB_SERIAL                  "USB Serial (VCP)"
#define TR_SETUP_SCREENS               "Obrazovky nastavení"
#define TR_MONITOR_SCREENS             "Monitory"
#define TR_AND_SWITCH                  "AND Spínač"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Kalibrovat analogy"
#define TR_ANADIAGS_FILTRAWDEV         "Filtrované surové hodnoty analogů s odchylkou"
#define TR_ANADIAGS_UNFILTRAW          "Nefiltrované surové hodnoty analogů"
#define TR_ANADIAGS_MINMAX             "Min., max. a rozsah"
#define TR_ANADIAGS_MOVE               "Posuňte analogy do jejich maximálních poloh!"
#define TR_SPEAKER                     INDENT"Repro"
#define TR_BUZZER                      INDENT"Pípák"
#define TR_BYTES                       "[B]"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_POWERMETER_ATTN_NEEDED      "Útlumový člen nutný!"
#define TR_PXX2_SELECT_RX              "Vyber RX..."
#define TR_PXX2_DEFAULT                "<výchozí>"
#define TR_BT_SELECT_DEVICE            "Vyberte zařízení"
#define TR_DISCOVER                    "Hledat"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Čekám..."
#define TR_RECEIVER_DELETE             "Smazat přijímač?"
#define TR_RECEIVER_RESET              "Resetovat přijímač?"
#define TR_SHARE                       "Sdílet"
#define TR_BIND                        "Párovat"
#define TR_REGISTER                    TR("Reg", "Registrovat")
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "Dosah"))
#define TR_RECEIVER_OPTIONS            TR("MOŽNOSTI RX", "MOŽNOSTI PŘIJÍMAČE")
#define TR_DEL_BUTTON                  BUTTON(TR("Smaž", "Smazat"))
#define TR_RESET_BTN                   BUTTON("Reset")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON(TR("SW","Přepínače"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analogy"))
#define TR_TOUCH_NOTFOUND              "Dotyková obrazovka nenalezena"
#define TR_TOUCH_EXIT                  "Dotkněte se obrazovky pro ukončení"
#define TR_SET                         BUTTON("Nast")
#define TR_TRAINER                     "Trenér"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM              CENTER "Problém s TX anténou!"
#define TR_MODELIDUSED                 TR("ID již použito","ID modelu je již použito")
#define TR_MODULE                      "Modul"
#define TR_RX_NAME                     "Jméno RX"
#define TR_TELEMETRY_TYPE              TR("Typ", "Typ telemetrie")
#define TR_TELEMETRY_SENSORS           "Senzory"
#define TR_VALUE                       "Hodnota"
#define TR_REPEAT                      "Opakovat"
#define TR_ENABLE                      "Povoleno"
#define TR_TOPLCDTIMER                 "Stopky horního LCD"
#define TR_UNIT                        "Jednotky"
#define TR_TELEMETRY_NEWSENSOR         INDENT "Přidat senzor ručně"
#define TR_CHANNELRANGE                TR(INDENT "Kanály", INDENT "Rozsah kanálů")
#define TR_AFHDS3_RX_FREQ              TR("RX frek.", "RX frekvence")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetrie")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Aktuální výkon")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", TR("Power src.", "Power source"))
#define TR_ANTENNACONFIRM1             "Opravdu přepnout?"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "Interní","Zeptat se","Dle modelu","Interní + Externí"
#else
  #define TR_ANTENNA_MODES             "Interní","Zeptat se","Dle modelu","Externí"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Použít int. ant.", "Použít interní anténu")
#define TR_USE_EXTERNAL_ANTENNA        TR("Použít ext. ant.", "Použít externí anténu")
#define TR_ANTENNACONFIRM2             TR("Zkont. anténu", "Ujistěte se, že je anténa připojena!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Vyžaduje FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Vyžaduje FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Vyžaduje EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "firmware."
#define TR_LOWALARM                    INDENT "Nízký Alarm"
#define TR_CRITICALALARM               INDENT "Kritický Alarm"
#define TR_RSSIALARM_WARN              TR("RSSI","RSSI TELEMETRIE")
#define TR_NO_RSSIALARM                TR(INDENT "Vypnuté alarmy", INDENT "Alarmy telemetrie vypnuty")
#define TR_DISABLE_ALARM               TR(INDENT "Vypnout alarmy", INDENT "Vypnout alarmy telemetrie")
#define TR_ENABLE_POPUP                "Povolit vyskakovací okno"
#define TR_DISABLE_POPUP               "Zakázat vyskakovací okno"
#define TR_POPUP                       "Vyskočit"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Šablona"
#define TR_PRESET                      "Šablona"
#define TR_MIRROR                      "Zrcadlit"
#define TR_CLEAR                       "Smazat"
#define TR_RESET                       "Reset"
#define TR_RESET_SUBMENU               "Inicializovat ..."
#define TR_COUNT                       "Velikost"
#define TR_PT                          "č."
#define TR_PTS                         "b."
#define TR_SMOOTH                      "Hladká"
#define TR_COPY_STICKS_TO_OFS          TR("Páky do subtrimu", "Kopie pák do subtrimu")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
#define TR_COPY_TRIMS_TO_OFS           TR("Trimy do subtrimu", "Kopírovat trimy do subtrimů")
#define TR_INCDEC                      "Zvěšit/Zmenšit"
#define TR_GLOBALVAR                   "Glob. proměnná"
#define TR_MIXSOURCE                   "Zdroje mixeru"
#define TR_CONSTANT                    "Konstanta"
#define TR_PERSISTENT_MAH              TR(INDENT "Ulož mAh", INDENT "Ukládat mAh")
#define TR_PREFLIGHT                   "Předletová kontrola"
#define TR_CHECKLIST                   TR(INDENT "Poznámky", INDENT "Zobrazit poznámky")
#define TR_FAS_OFFSET                  TR(INDENT "FAS Ofs", INDENT "FAS Ofset")
#define TR_AUX_SERIAL_MODE             "Seriový port"
#define TR_AUX2_SERIAL_MODE            "Seriový port 2"
#define TR_AUX_SERIAL_PORT_POWER       "Výkon portu"
#define TR_SCRIPT                      "Skript"
#define TR_INPUTS                      "Vstupy"
#define TR_OUTPUTS                     "Výstupy"
#define STR_EEBACKUP                   "Záloha EEPROM"
#define STR_FACTORYRESET               "Tovární reset"
#define TR_CONFIRMRESET                TR("Smazat vše?", "Smazat modely a nastavení?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Příliš mnoho skriptů!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP výkon"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "Žádné obrazovky telemetrie"
#define TR_TOUCH_PANEL                 "Dotyková obrazovka:"
#define TR_FILE_SIZE                   "Velikost souboru"
#define TR_FILE_OPEN                   "Otevřít?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Název"
#define TR_PHASES_HEADERS_SW           "Spínač"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trim směrovky"
#define TR_PHASES_HEADERS_ELE_TRIM     "Trim výškovky"
#define TR_PHASES_HEADERS_THT_TRIM     "Trim plynu"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trim křidélek"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Přechod náběhu"
#define TR_PHASES_HEADERS_FAD_OUT      "Přechod konce"

#define TR_LIMITS_HEADERS_NAME         "Název"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Invertovat"
#define TR_LIMITS_HEADERS_CURVE        "Křivka"
#define TR_LIMITS_HEADERS_PPMCENTER    "Střed PPM"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Symetrické"
#define TR_INVERTED                    "Invertováno"

#define TR_LSW_HEADERS_FUNCTION        "Funkce"
#define TR_LSW_HEADERS_V1              "Hodnota 1"
#define TR_LSW_HEADERS_V2              "Hodnota 2"
#define TR_LSW_HEADERS_ANDSW           "AND Spínač"
#define TR_LSW_HEADERS_DURATION        "Trvání"
#define TR_LSW_HEADERS_DELAY           "Zpoždění"

#define TR_GVAR_HEADERS_NAME           "Název"
#define TR_GVAR_HEADERS_FM0            "Hodnota v režimu LR0"
#define TR_GVAR_HEADERS_FM1            "Hodnota v režimu LR1"
#define TR_GVAR_HEADERS_FM2            "Hodnota v režimu LR2"
#define TR_GVAR_HEADERS_FM3            "Hodnota v režimu LR3"
#define TR_GVAR_HEADERS_FM4            "Hodnota v režimu LR4"
#define TR_GVAR_HEADERS_FM5            "Hodnota v režimu LR5"
#define TR_GVAR_HEADERS_FM6            "Hodnota v režimu LR6"
#define TR_GVAR_HEADERS_FM7            "Hodnota v režimu LR7"
#define TR_GVAR_HEADERS_FM8            "Hodnota v režimu LR8"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS            { "Typ porovnávací funkce", "První proměnná", "Druhá proměnná nebo konstanta", "Druhá proměnná nebo konstanta", "Další stav povolující tenko spínač", "Minimální doba trvání sepnutého stavu", "Minimální doba platné podmínky pro aktivaci" }

// Horus layouts and widgets
#define TR_FIRST_CHANNEL               "První kanál"
#define TR_FILL_BACKGROUND             "Vyplnit pozadí?"
#define TR_BG_COLOR                    "Barva pozadí"
#define TR_SLIDERS_TRIMS               "Slidery+Trimy"
#define TR_SLIDERS                     "Slidery"
#define TR_FLIGHT_MODE                 "Letový režim"
#define TR_INVALID_FILE                "Neplatní soubor"
#define TR_TIMER_SOURCE                "Časovač zdroj"
#define TR_SIZE                        "Velikost"
#define TR_SHADOW                      "Stíny"
#define TR_TEXT                        "Text"
#define TR_COLOR                       "Barva"
#define TR_MAIN_VIEW_X                 "Hlavní pohled X"
#define TR_PANEL1_BACKGROUND           "Panel1 pozadí"
#define TR_PANEL2_BACKGROUND           "Panel2 pozadí"

// Taranis About screen
#define TR_ABOUTUS                     "O nás"

#define TR_CHR_SHORT                   's'
#define TR_CHR_LONG                    'l'
#define TR_CHR_TOGGLE                  't'
#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'              // Values between A-I will work

#define TR_BEEP_VOLUME                 "Upozornění"
#define TR_WAV_VOLUME                  "Zvuky WAV"
#define TR_BG_VOLUME                   "WAV na pozadí"

#define TR_TOP_BAR                     "Horní lišta"
#define TR_FLASH_ERASE                 "Mazaní flash..."
#define TR_FLASH_WRITE                 "Zápis flash..."
#define TR_OTA_UPDATE                  "Aktualizace OTA..."
#define TR_MODULE_RESET                "Reset modulu..."
#define TR_UNKNOWN_RX                  "Neznámý RX"
#define TR_UNSUPPORTED_RX              "Nepodporovaný RX"
#define TR_OTA_UPDATE_ERROR            "Chyba aktualizace"
#define TR_DEVICE_RESET                "Reset zařízení..."
#define TR_ALTITUDE                    INDENT "Výška"
#define TR_SCALE                       "Měřítko"
#define TR_VIEW_CHANNELS               "Zobrazit kanály"
#define TR_VIEW_NOTES                  "Zobrazit poznámky"
#define TR_MODEL_SELECT                "Zvolit model"
#define TR_MODS_FORBIDDEN              "Změny nejsou povoleny!"
#define TR_UNLOCKED                    "Odemčeno"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Přesnost"
#define TR_RATIO                       "Koeficient"
#define TR_FORMULA                     "Operace"
#define TR_CELLINDEX                   "Článek"
#define TR_LOGS                        "Logovat"
#define TR_OPTIONS                     "Možnosti"
#define TR_FIRMWARE_OPTIONS            "Možnosti firmwaru"

#define TR_ALTSENSOR                   "Senzor výšky"
#define TR_CELLSENSOR                  "Senzor článků"
#define TR_GPSSENSOR                   "GPS senzor"
#define TR_CURRENTSENSOR               "Senzor"
#define TR_AUTOOFFSET                  "Auto offset"
#define TR_ONLYPOSITIVE                "Jen kladné"
#define TR_FILTER                      "Filtr"
#define TR_TELEMETRYFULL               "Všechny sloty jsou plné!"
#define TR_SERVOS_OK                   "Serva OK"
#define TR_SERVOS_KO                   "Serva KO"
#define TR_INVERTED_SERIAL             INDENT "Invert"
#define TR_IGNORE_INSTANCE             TR(INDENT "Chybné ID", INDENT "Ignoruj chyby ID")
#define TR_DISCOVER_SENSORS            "Detekovat nové senzory"
#define TR_STOP_DISCOVER_SENSORS       "Zastavit autodetekci"
#define TR_DELETE_ALL_SENSORS          "Odebrat všechny senzory"
#define TR_CONFIRMDELETE               "Opravdu " LCDW_128_480_LINEBREAK "odstranit vše?"
#define TR_SELECT_WIDGET               "Zvolit widget"
#define TR_REMOVE_WIDGET               "Odstranit widget"
#define TR_WIDGET_SETTINGS             "Nastavení widgetu"
#define TR_REMOVE_SCREEN               "Odstranit panel"
#define TR_SETUP_WIDGETS               "Nastavit widgety"
#define TR_USER_INTERFACE              "Uživatelské rozhraní"
#define TR_THEME                       "Téma"
#define TR_SETUP                       "Nastavení"
#define TR_LAYOUT                      "Rozložení"
#define TR_ADD_MAIN_VIEW               "Přidat hlavní panel"
#define TR_BACKGROUND_COLOR            "Barva pozadí"
#define TR_MAIN_COLOR                  "Hlavní barva"
#define TR_BAR2_COLOR                  "Další barva lišty"
#define TR_BAR1_COLOR                  "Hlavní barva lišty"
#define TR_TEXT_COLOR                  "Barva textu"
#define TR_TEXT_VIEWER                 "Prohlížeč textu"

#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Vstupy"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua skripty"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Páky"
#define TR_MENU_POTS                   STR_CHAR_POT "Potenciometry"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "Cyklika"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Trimy"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Spínače"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Logické spínače"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Trenér"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Kanály"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "Glob.proměnné"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Telemetrie"
#define TR_MENU_DISPLAY                "DISPLAY"
#define TR_MENU_OTHER                  "Ostatní"
#define TR_MENU_INVERT                 "Invertovat"
#define TR_JITTER_FILTER               "ADC Filtr"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Kontr RTC", "Hlídat RTC napětí")
#define TR_AUTH_FAILURE                "Auth-selhala"
#define TR_RACING_MODE                 "Závodní režim"

#define STR_VFR                       "VFR"
#define STR_RSSI                      "RSSI"
#define STR_R9PW                      "R9PW"
#define STR_RAS                       "SWR"
#define STR_A1                        "A1"
#define STR_A2                        "A2"
#define STR_A3                        "A3"
#define STR_A4                        "A4"
#define STR_BATT                      "RxBt"
#define STR_ALT                       "Alt"
#define STR_TEMP1                     "Tmp1"
#define STR_TEMP2                     "Tmp2"
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
#define STR_RPM                       "RPM"
#define STR_FUEL                      "Fuel"
#define STR_VSPD                      "VSpd"
#define STR_ACCX                      "AccX"
#define STR_ACCY                      "AccY"
#define STR_ACCZ                      "AccZ"
#define STR_GYROX                     "GYRX"
#define STR_GYROY                     "GYRY"
#define STR_GYROZ                     "GYRZ"
#define STR_CURR                      "Curr"
#define STR_CAPACITY                  "Capa"
#define STR_VFAS                      "VFAS"
#define STR_BATT_PERCENT              "Bat%"
#define STR_ASPD                      "ASpd"
#define STR_GSPD                      "GSpd"
#define STR_HDG                       "Hdg"
#define STR_SATELLITES                "Sats"
#define STR_CELLS                     "Cels"
#define STR_GPSALT                    "GAlt"
#define STR_GPSDATETIME               "Date"
#define STR_BATT1_VOLTAGE             "RB1V"
#define STR_BATT2_VOLTAGE             "RB2V"
#define STR_BATT1_CURRENT             "RB1A"
#define STR_BATT2_CURRENT             "RB2A"
#define STR_BATT1_CONSUMPTION         "RB1C"
#define STR_BATT2_CONSUMPTION         "RB2C"
#define STR_BATT1_TEMP                "RB1T"
#define STR_BATT2_TEMP                "RB2T"
#define STR_RB_STATE                  "RBS"
#define STR_CHANS_STATE               "RBCS"
#define STR_RX_RSSI1                  "1RSS"
#define STR_RX_RSSI2                  "2RSS"
#define STR_RX_QUALITY                "RQly"
#define STR_RX_SNR                    "RSNR"
#define STR_RX_NOISE                  "RNse"
#define STR_RF_MODE                   "RFMD"
#define STR_TX_POWER                  "TPWR"
#define STR_TX_RSSI                   "TRSS"
#define STR_TX_QUALITY                "TQly"
#define STR_TX_SNR                    "TSNR"
#define STR_TX_NOISE                  "TNse"
#define STR_PITCH                     "Ptch"
#define STR_ROLL                      "Roll"
#define STR_YAW                       "Yaw"
#define STR_THROTTLE                  "Thr"
#define STR_QOS_A                     "FdeA"
#define STR_QOS_B                     "FdeB"
#define STR_QOS_L                     "FdeL"
#define STR_QOS_R                     "FdeR"
#define STR_QOS_F                     "FLss"
#define STR_QOS_H                     "Hold"
#define STR_LAP_NUMBER                "Lap "
#define STR_GATE_NUMBER               "Gate"
#define STR_LAP_TIME                  "LapT"
#define STR_GATE_TIME                 "GteT"
#define STR_ESC_VOLTAGE               "EscV"
#define STR_ESC_CURRENT               "EscA"
#define STR_ESC_RPM                   "Erpm"
#define STR_ESC_CONSUMPTION           "EscC"
#define STR_ESC_TEMP                  "EscT"
#define STR_SD1_CHANNEL               "Chan"
#define STR_GASSUIT_TEMP1             "GTp1"
#define STR_GASSUIT_TEMP2             "GTp2"
#define STR_GASSUIT_RPM               "GRPM"
#define STR_GASSUIT_FLOW              "GFlo"
#define STR_GASSUIT_CONS              "GFue"
#define STR_GASSUIT_RES_VOL           "GRVl"
#define STR_GASSUIT_RES_PERC          "GRPc"
#define STR_GASSUIT_MAX_FLOW          "GMFl"
#define STR_GASSUIT_AVG_FLOW          "GAFl"
#define STR_SBEC_VOLTAGE              "BecV"
#define STR_SBEC_CURRENT              "BecA"
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

#define TR_USE_THEME_COLOR              "Použít barevný motiv"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Trimy do subtrimů"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS        "Otevřít monitor kanálů"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Otevřít mon. kanálů"
#endif
#define TR_DUPLICATE                    "Duplikovat"
#define TR_ACTIVATE                     "Aktivovat"
#define TR_RED                          "Červená"
#define TR_BLUE                         "Modrá"
#define TR_GREEN                        "Zelená"
#define TR_COLOR_PICKER                 "Výběr barvy"
#define TR_EDIT_THEME_DETAILS           "Editovat motiv"
#define TR_AUTHOR                       "Autor"
#define TR_DESCRIPTION                  "Popis"
#define TR_SAVE                         "Uložit"
#define TR_CANCEL                       "Zrušit"
#define TR_EDIT_THEME                   "EDITOVAT MOTIV"
#define TR_DETAILS                      "Detaily"
#define TR_THEME_EDITOR                 "Motivy"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Vybrat\nmodel"
#define TR_MAIN_MENU_MODEL_NOTES        "Poznámky\nmodelu"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Monitor\nkanálů"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Nastavení\nmodelu"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Nastavení\rádia"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Nastavení\nobrazovky"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Reset\ntelemetrie"
#define TR_MAIN_MENU_STATISTICS         "Statistiky"
#define TR_MAIN_MENU_ABOUT_EDGETX       "O\nEdgeTX"
// End Main menu
