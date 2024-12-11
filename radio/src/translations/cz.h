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

#define TR_MIN_PLURAL2 2
#define TR_MAX_PLURAL2 4
// For this number of minute in the last decimal place singular form is used in
// plural
#define TR_USE_SINGULAR_IN_PLURAL 0
#define TR_USE_PLURAL2_SPECIAL_CASE 1
// If the number of minutes is above this value PLURAL2 is used
#define TR_USE_PLURAL2 20

#define TR_MINUTE_SINGULAR             "minuta"
#define TR_MINUTE_PLURAL1              "minuty"
#define TR_MINUTE_PLURAL2              "minut"

// NON ZERO TERMINATED STRINGS
#define TR_OFFON                       "VYP","ZAP"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Tichý",TR("Alarm","Jen Alarm"),TR("BezKl","Bez kláves"),"Vše"
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VBLMODE                     TR("Vyp","Vypnuto"),TR("Kláv.","Klávesy"),"Páky","Vše",TR("Zap","Zapnuto")
#define TR_TRNMODE                     "X","Sečíst","Zaměnit"
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES            "VYP","Telemetrie zrcadlení","Telemetrie vstup","SBUS Trenér","LUA","CLI","GPS","Debug","SpaceMouse","Externí modul"
#define TR_SWTYPES                     "Žádný","Bez aretace","2-polohový","3-polohový"
#define TR_POTTYPES                    "Žádný",TR("Pot","Potenciometr"),TR("Pot s aret.","Pot s aretací"),"Slider",TR("Vícepol př.","Vícepol. přep."),"Osa X","Osa Y","Přepínač"
#define TR_VPERSISTENT                 "Ne","V rámci letu","Reset ručně"
#define TR_COUNTRY_CODES               TR("US","Amerika"),TR("JP","Japonsko"),TR("EU","Evropa")
#define TR_USBMODES                    TR("Zeptat","Zeptat se"),TR("Joyst","Joystick"),TR("SDkarta","Úložiště"),"Serial"
#define TR_JACK_MODES                  "Zeptat","Audio","Trenér"

#define TR_SBUS_INVERSION_VALUES       "normal","neinvertováno"
#define TR_MULTI_CUSTOM                "Vlastní"
#define TR_VTRIMINC                    "Expo","ExJemný","Jemný","Střední","Hrubý"
#define TR_VDISPLAYTRIMS               "Ne","Změna","Ano"
#define TR_VBEEPCOUNTDOWN              "Ne", "Zvuk", "Hlas", "Vibrace", TR("Zv & Vib","Zvuk & Vibrace"),TR("Hl & Vib", "Hlas & Vibrace")
#define TR_VVARIOCENTER                "Tón","Ticho"
#define TR_CURVE_TYPES                 "Rastr-X","Volná-XY"

#define TR_ADCFILTERVALUES             "Globální","Vyp","Zap",

#define TR_VCURVETYPE                  "Diff","Expo","Funk","Vlas"
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Sečíst","Násobit","Zaměnit"
#define TR_VMLTPX2                     "+=","*=",":="

#if defined(PCBHORUS)
#else
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Stopky"
  #define TR_CSWSTICKY                 "Sticky"
  #define TR_CSWSTAY                   "Edge"
#else
  #define TR_CSWTIMER                  "Tim"
  #define TR_CSWSTICKY                 "Stky"
  #define TR_CSWSTAY                   "Edge"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Trenér"
#define TR_SF_INST_TRIM                "Insta-Trim"
#define TR_SF_RESET                    "Reset"
#define TR_SF_SET_TIMER                "Změna"
#define TR_SF_VOLUME                   "Hlasitost"
#define TR_SF_FAILSAFE                 "Nastavit Failsafe"
#define TR_SF_RANGE_CHECK              "Kontrola dosahu"
#define TR_SF_MOD_BIND                 "ModuleBind"
#define TR_SF_RGBLEDS                  "RGB světlo"

#define TR_SOUND                       TR3("\200\201Zvuk", "\200\201Zvuk", "Hrát zvuk")
#define TR_PLAY_TRACK                  TR3("\200\201Stopa", "\200\201Stopa", "Přehrát wav")
#define TR_PLAY_VALUE                  TR3("\200\201Hlásit ", "\200\201Hlásit ", "Hlásit stav")
#define TR_SF_HAPTIC                   "Vibrovat"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua Skript")
#define TR_SF_BG_MUSIC                 TR3("\200\201Hudba","\200\201Hudba","Hudba")
#define TR_SF_BG_MUSIC_PAUSE           TR3("\200\201Hudba ||","\200\201Hudba ||","Hudba pauza")
#define TR_SF_LOGS                     "Loguj na SD"
#define TR_ADJUST_GVAR                 "Nastav"
#define TR_SF_BACKLIGHT                "Podsvětlení"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_SAFETY                   "Zámek"
#define TR_SF_TEST                     "Test"

#define TR_SF_SCREENSHOT               "Snímek LCD"
#define TR_SF_RACING_MODE              "Závodní režim"
#define TR_SF_DISABLE_TOUCH            "Deaktivace dotyku"
#define TR_SF_DISABLE_AUDIO_AMP        "Vypnutí zesilovače zvuku"
#define TR_SF_SET_SCREEN               "Vybrat hlavní obrazovku"
#define TR_SF_SET_TELEMSCREEN          "Nast obrazovku"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Telm","Telemetrie")

#if LCD_W >= 212
  #define TR_FSW_RESET_TRIMS           "Trims"
#define TR_FSW_RESET_TIMERS            "Stopky1","Stopky2","Stopky3"
#else
  #define TR_FSW_RESET_TRIMS           "Trims"
#define TR_FSW_RESET_TIMERS            "Čas1","Čas2","Čas3"
#endif

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,"Vše",TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS
#define TR_FUNCSOUNDS                  TR("Píp1","Pípnutí1"),TR("Píp2","Pípnutí2"),TR("Píp3","Pípnutí3"),TR("Var1","Varování1"),TR("Var2","Varování2"),TR("Chee","Cheep"),TR("Rata", "Ratata"),"Tick",TR("Sirn","Siréna"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                TR("Metr.","Metrické"),TR("Imper.","Imperiální")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Nic","Hodnota","Ukazatel","Skript"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "Smě"
#define TR_STICK_NAMES1                "Výš"
#define TR_STICK_NAMES2                "Pln"
#define TR_STICK_NAMES3                "Kři"
#define TR_SURFACE_NAMES0              "Smě"
#define TR_SURFACE_NAMES1              "Pln"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Defaultní","Vysoký"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ZAP","One"

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "Režim kloboučků"
#define TR_HATSOPT                     "Pouze trimy","Pouze tlačítka","Přepinatelné","Globální"
#define TR_HATSMODE_TRIMS              "Režim kloboučků: Trimy"
#define TR_HATSMODE_KEYS               "Režim kloboučků: Tlačítka"
#define TR_HATSMODE_KEYS_HELP          "Levá strana:\n"\
                                       " Vpravo = MDL\n"\
                                       " Nahoru = SYS\n"\
                                       " Dolů = TELE\n"\
                                       "\n"\
                                       "Pravá strana:\n"\
                                       " Vlevo = LIST<\n"\
                                       " Vpravo = LIST>\n"\
                                       " Nahoru = PŘED/ZVÝŠ\n"\
                                       " Dolů = DALŠÍ/SNÍŽ"
#endif

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Normal","Invertován"
#else
  #define TR_ROTARY_ENC_OPT         "Normal","Invertován","V-I H-N","V-I H-A","V-N E-I"
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


#define TR_SRC_BATT                    "Bat."
#define TR_SRC_TIME                    "Čas"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Čas"

#define TR_VTMRMODES                   "VYP","ZAP","Strt","Pln>","Pln%","Pln*"
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
#define TR_VCELLINDEX                  "Nízký","1","2","3","4","5","6","7","8","Nejvíce","Delta"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA"(pouze středy)","= (symetrický)"
#define TR_TIMER_DIR                   TR("Zbývající", "Zobrazit zbývající"), TR("Uplynulý", "Zobrazit uplynulý")
#define TR_PPMUNIT                     "0.--","0.0","us"

// ZERO TERMINATED STRINGS

#if defined(COLORLCD)
  #if defined(BOLD)
    #define TR_FONT_SIZES                  "STD"
  #else
    #define TR_FONT_SIZES                  "STD","BOLD","XXS","XS","L","XL","XXL"
  #endif
#endif

#if defined(PCBTARANIS) || defined(PCBHORUS)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14) || defined(PCBPL18)
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

#define TR_MENUWHENDONE                TR_ENTER " > DALŠÍ"
#define TR_FREE                        TR("volné:", "volných")
#define TR_YES                         "Ano"
#define TR_NO                          "Ne"
#define TR_DELETEMODEL                 "SMAZAT MODEL"
#define TR_COPYINGMODEL                "Kopíruji model..."
#define TR_MOVINGMODEL                 "Přesouvám model..."
#define TR_LOADINGMODEL                "Aktivuji model..."
#define TR_UNLABELEDMODEL              "Bez štítku"
#define TR_NAME                        "Název"
#define TR_MODELNAME                   TR("Model", "Název modelu")
#define TR_PHASENAME                   "Název"
#define TR_MIXNAME                     "Název"
#define TR_INPUTNAME                   "Název"
#define TR_EXPONAME                    "Popis"
#define TR_BITMAP                      "Obrázek"
#define TR_NO_PICTURE                  "Bez obrázku"
#define TR_TIMER                       "Stopky"
#define TR_START                       "Start"
#define TR_ELIMITS                     TR("Limit +25%", "Kanál +/- 125%")
#define TR_ETRIMS                      TR("širší Trim", "Široký trim")
#define TR_TRIMINC                     TR("Krok Trimu", "Krok trimu")
#define TR_DISPLAY_TRIMS               TR3("Číslo v Trm", "Zobr.hodnotu trimu", "Číslo v liště trimu")
#define TR_TTRACE                      TR("StopaPlynu", "Stopa plynu")
#define TR_TTRIM                       TR3("TrimVolnob.", "Trim jen volnoběh", "Trim jen pro volnoběh")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", "Trim spínač")
#define TR_BEEPCTR                     TR("Pípat střed", "Pípnutí při středové poloze")
#define TR_USE_GLOBAL_FUNCS            TR("Glob.Funkce", "Použít globální funkce")
#define TR_PROTOCOL                    "Protokol"
#define TR_PPMFRAME                    "PPM modulace"
#define TR_REFRESHRATE                 TR("Obnovit", "Obn. frekv.")
#define STR_WARN_BATTVOLTAGE           TR("Výstup VBAT: ", "Varování: výstupní hodnota VBAT: ")
#define TR_WARN_5VOLTS                 "Varování: výstupní úroveň je 5 voltů"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Spínač"
#define TR_FUNCTION_SWITCHES           "Nastavitelné přepínače"
#define TR_FS_COLOR_LIST               "White","Red","Green","Yellow","Orange","Blue","Pink","Off","Custom"
#define TR_GROUP                       "Group"
#define TR_GROUP_ALWAYS_ON             "Always on"
#define TR_GROUPS                      "Always on groups"
#define TR_LAST                        "Last"
#define TR_MORE_INFO                   "More info"
#define TR_SWITCH_TYPE                 "Type"
#define TR_SWITCH_STARTUP              "Startup"
#define TR_SWITCH_GROUP                "Group"
#define TR_FUNCTION_SWITCH_GROUPS      "---", TR_SWITCH_GROUP" 1", TR_SWITCH_GROUP" 2", TR_SWITCH_GROUP" 3"
#define TR_SF_SWITCH                   "Trigger"
#define TR_TRIMS                       "Trimy"
#define TR_FADEIN                      "Přechod Zap"
#define TR_FADEOUT                     "Přechod Vyp"
#define TR_DEFAULT                     "(výchozí)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Kontrolovat trimy"
#else
  #define TR_CHECKTRIMS                "\006Kont.\012Trimy"
#endif
#define TR_SWASHTYPE                   "Typ cykliky"
#define TR_COLLECTIVE                  "Kolektiv"
#define TR_AILERON                     "Boční cyklika"
#define TR_ELEVATOR                    TR3("Podélná cykl.", "Podélná cykl.", "Podélná cyklika")
#define TR_SWASHRING                   "Cyklika"
#define TR_MODE                        "Mód"
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "Vlevo"
#else
  #define TR_LEFT_STICK                "Vlevo"
#endif
#define TR_SUBTYPE                     "Subtyp"
#define TR_NOFREEEXPO                  "Není volné expo!"
#define TR_NOFREEMIXER                 "Není volný mix!"
#define TR_SOURCE                      "Zdroj"
#define TR_WEIGHT                      "Váha"
#define TR_SIDE                        "Strana"
#define TR_OFFSET                      "Ofset"
#define TR_TRIM                        "Trim"
#define TR_DREX                        "DR/Expo"
#define DREX_CHBOX_OFFSET              30
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
#if defined(PCBNV14) || defined(PCBPL18)
#define TR_GV                          "GP"
#else
#define TR_GV                          TR("G", "GP")
#endif
#define TR_RANGE                       "Rozsah"
#define TR_CENTER                      "Střed"
#define TR_ALARM                       "Alarm"
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
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Trenér vypnutí", "Kontrola trenéra při vypnutí")
#define TR_MODEL_STILL_POWERED         "Model stále spuštěn"
#define TR_TRAINER_STILL_CONNECTED     "Trenér stále připojen"
#define TR_USB_STILL_CONNECTED         "USB stále připojeno"
#define TR_MODEL_SHUTDOWN              "Vypnout?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Zmáčkni ENTER pro potvrzení"
#define TR_THROTTLE_LABEL              "Plyn"
#define TR_THROTTLE_START              "Plyn Start"
#define TR_THROTTLEREVERSE             TR("ReverzPlyn", "Reverz plynu")
#define TR_MINUTEBEEP                  TR("Minuta", "Oznamovat minuty")
#define TR_BEEPCOUNTDOWN               "Odpočet"
#define TR_PERSISTENT                  "Trvalé"
#define TR_BACKLIGHT_LABEL             "Podsvětlení"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Stav"
#define TR_BLONBRIGHTNESS              "Jas zapnutého LCD"
#define TR_BLOFFBRIGHTNESS             "Jas vypnutého LCD"
#define TR_KEYS_BACKLIGHT              "Podsvětlení kláves"
#define TR_BLCOLOR                     "Barva"
#define TR_SPLASHSCREEN                TR("úvodní logo", "Zobrazit úvodní logo")
#define TR_PLAY_HELLO                  "Zvuk při spuštění"
#define TR_PWR_ON_DELAY                "Zpoždění zapnutí"
#define TR_PWR_OFF_DELAY               "Zpoždění vypnutí"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("Zap ON/OFF vibrace","Zapnutí ON/OFF vibrace")
#define TR_THROTTLE_WARNING            TR("* Plyn", "Kontrola plynu")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Vlas-Poz", "Vlastní pozice?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Poz. %", "Pozice %")
#define TR_SWITCHWARNING               TR("* Spínače", "Polohy spínačů")
#define TR_POTWARNINGSTATE             "Kontrola Pot&Slid."
#define TR_POTWARNING                  TR("* Potenc.", "Potenciometry")
#define TR_TIMEZONE                    "Časové pásmo"
#define TR_ADJUST_RTC                  TR3("Čas z GPS", "Použít čas z GPS", "Použít čas z GPS")
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Def řaz kanálu", "Defaultní řazení kanálu")
#define TR_STICKS                      "Osy"
#define TR_POTS                        "Potenciometry"
#define TR_SWITCHES                    "Spínače"
#define TR_SWITCHES_DELAY              TR3("Filtr přepínače", "Filtr přepínače", "Filtr poloh přepínače")
#define TR_SLAVE                       "Podřízený"
#define TR_MODESRC                     "Mód\006% Zdroj"
#define TR_MULTIPLIER                  "Násobič"
#define TR_CAL                         "Kal."
#define TR_CALIBRATION                 "Kalibrace"
#define TR_VTRIM                       "Trim - +"
#define TR_CALIB_DONE                  "Kalibrace dokončena"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Stiskem [ENTER] začneme"
  #define TR_SETMIDPOINT               "Vycentruj páky/poty/slidery a stiskni [ENTER]"
  #define TR_MOVESTICKSPOTS            "Hýbat osy/poty/slidery a stisknout [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER "= START"
  #define TR_SETMIDPOINT               "VYCENTROVAT OSY/SLIDERY"
  #define TR_MOVESTICKSPOTS            "HÝBAT OSY/POTY"
#else
  #define TR_MENUTOSTART               TR_ENTER " = START"
#if defined(SURFACE_RADIO)
  #define TR_SETMIDPOINT               "NASTAVIT STŘED POTU"
  #define TR_MOVESTICKSPOTS            "HÝBAT TOČIT/PLYN/POTY/OSY"
#else
  #define TR_SETMIDPOINT               TR("NASTAVIT STŘED OSY", "STŘED OSY/SLIDERU")
  #define TR_MOVESTICKSPOTS            "HÝBAT OSY/POTY"
#endif
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      "ŽÁDNÁ DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"

#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "Vnitřní GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua skripty"
#define TR_FREE_MEM_LABEL              "Free mem"
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
#define TR_TIMER_LABEL                 "Časovač"
#define TR_THROTTLE_PERCENT_LABEL      "Plyn %"
#define TR_BATT_LABEL                  "Baterie"
#define TR_SESSION                     "Relace"

#define TR_MENUTORESET                 TR_ENTER" >> Reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODEL"
#if defined(SURFACE_RADIO)
#define TR_FM                          "DM"
#else
#define TR_FM                          "LR"
#endif
#define TR_EEPROMLOWMEM                "Dochází EEPROM"
#define TR_PRESS_ANY_KEY_TO_SKIP       TR("\003Klávesa >>> přeskočit", "Klávesa >>> přeskočit")
#define TR_THROTTLE_NOT_IDLE           TR("\003Páka plynu je pohnutá", "Páka plynu není na nule")
#define TR_ALARMSDISABLED              "Alarmy jsou zakázány"
#define TR_PRESSANYKEY                 TR("\006Stiskni klávesu", "Stiskni klávesu")
#define TR_BADEEPROMDATA               TR("\006Chyba dat EEPROM", "Chyba dat EEPROM")
#define TR_BAD_RADIO_DATA              "Chybějící nebo poškozená data vysílače"
#define TR_RADIO_DATA_RECOVERED        TR3("Použití zálohy dat TX","Použití zálohy dat vysílače","Nastavení vysílače bylo obnoveno ze zálohy")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Neplatné nastavení TX","Neplatné nastavení vysílače", "Nelze načíst platné nastavení vysílače")
#define TR_EEPROMFORMATTING            TR("\004Formatování EEPROM", "Formatování EEPROM")
#define TR_STORAGE_FORMAT              "Příprava úložiště"
#define TR_EEPROMOVERFLOW              "Přetekla EEPROM"
#define TR_RADIO_SETUP                 "NASTAVENÍ RÁDIA"
#define TR_MENUTRAINER                 "TRENÉR"
#define TR_MENUSPECIALFUNCS            "GLOBÁLNÍ FUNKCE"
#define TR_MENUVERSION                 "VERZE"
#define TR_MENU_RADIO_SWITCHES         "DIAG"
#define TR_MENU_RADIO_ANALOGS          "ANALOGY"
#define TR_MENU_RADIO_ANALOGS_CALIB    "KALIBROVAT ANALOGY"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGY (5 Hz)"
#define TR_MENUCALIBRATION              "KALIBRACE"
#define TR_MENU_FSWITCH                 "NASTAVITELNÉ PŘEPÍNAČE"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trimy => Subtrimy"
#else
  #define TR_TRIMS2OFFSETS             "\006Trimy => Subtrimy"
#endif
#define TR_CHANNELS2FAILSAFE           "Kanály=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Kanál=>Failsafe"
#define TR_MENUMODELSEL                "MODEL"
#define TR_MENU_MODEL_SETUP            "NASTAVENÍ"
#if defined(SURFACE_RADIO)
#define TR_MENUFLIGHTMODES             "REŽIM JÍZDA"
#define TR_MENUFLIGHTMODE              "REŽIM JÍZDA"
#else
#define TR_MENUFLIGHTMODE              "LETOVÝ REŽIM"
#define TR_MENUFLIGHTMODES             "LETOVÉ REŽIMY"
#endif
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
#define TR_MULTI_RFTUNE                TR("Ladění frek", "Jemné ladění frek. RF")
#define TR_MULTI_RFPOWER               "RF výkon"
#define TR_MULTI_WBUS                  "Výstup"
#define TR_MULTI_TELEMETRY             "Telemetrie"
#define TR_MULTI_VIDFREQ               TR("Freq. videa", "Frekvence videa")
#define TR_RF_POWER                    "Výkon RF"
#define TR_MULTI_FIXEDID               TR("PevnéID", "Pevné ID")
#define TR_MULTI_OPTION                TR("Možnosti", "Hodnota")
#define TR_MULTI_AUTOBIND              TR("Bind Ch.","Bind při zapnutí")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Vypnout mapování kanálů")
#define TR_DISABLE_TELEM               TR("No Telem", "Vypnout telemetrii")
#define TR_MULTI_LOWPOWER              TR("Nízký výkon", "Režim nízkého výkonu")
#define TR_MULTI_LNA_DISABLE           "LNA vypnuto"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
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
#define TR_REG_OK                      "Registrace OK"
#define TR_BIND_OK                     "Úspěšné párovaní"
#define TR_BINDING_CH1_8_TELEM_ON      "k1-8 telem ZAP"
#define TR_BINDING_CH1_8_TELEM_OFF     "k1-8 telem VYP"
#define TR_BINDING_CH9_16_TELEM_ON     "k9-16 telem ZAP"
#define TR_BINDING_CH9_16_TELEM_OFF    "k9-16 telem VYP"
#define TR_PROTOCOL_INVALID            TR("Špat. protokol", "Špatný protokol")
#define TR_MODULE_STATUS               TR("Stav", "Stav modulu")
#define TR_MODULE_SYNC                 TR("Sync", "Proto Sync stav")
#define TR_MULTI_SERVOFREQ             TR("Frekv. serva", "Obnovovací frekvence serva")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Povolit max. throw")
#define TR_MULTI_RFCHAN                TR("RF Kanál", "Vybrat RF kanál")
#define TR_GPS_COORDS_FORMAT           "GPS souřadnice"
#define TR_VARIO                       "Vario"
#define TR_PITCH_AT_ZERO               "Tón na nule"
#define TR_PITCH_AT_MAX                "Tón na maximu"
#define TR_REPEAT_AT_ZERO              TR("Opak. na nule", "Opakování na nule")
#define TR_BATT_CALIB                  "Kalib. bat."
#define TR_CURRENT_CALIB               "+=\006Proud"
#define TR_VOLTAGE                     "Napětí"
#define TR_SELECT_MODEL                "Vyber model"
#define TR_MANAGE_MODELS               "NASTAVENÍ MODELU"
#define TR_MODELS                      "Modely"
#define TR_SELECT_MODE                 "Vybrat mód"
#define TR_CREATE_MODEL                "Nový model"
#define TR_FAVORITE_LABEL              "Oblíbené"
#define TR_MODELS_MOVED                "Nepoužívané modely přesunuty do"
#define TR_NEW_MODEL                   "Nový model"
#define TR_INVALID_MODEL               "Neplatný model"
#define TR_EDIT_LABELS                 "Upravit štítek"
#define TR_LABEL_MODEL                 "Štítek modelu"
#define TR_MOVE_UP                     "Posunout nahoru"
#define TR_MOVE_DOWN                   "Posunout dolů"
#define TR_ENTER_LABEL                 "Vložit štítek"
#define TR_LABEL                       "Štítek"
#define TR_LABELS                      "Štítky"
#define TR_CURRENT_MODEL               "Aktuální"
#define TR_ACTIVE                      "Aktivní"
#define TR_NEW                         "Nový"
#define TR_NEW_LABEL                   "Nový štítek"
#define TR_RENAME_LABEL                "Přejmenovat štítek"
#define TR_DELETE_LABEL                "Odstranit štítek"
#define TR_DUPLICATE_MODEL             "Duplikovat model"
#define TR_COPY_MODEL                  "Kopírovat"
#define TR_MOVE_MODEL                  "Přesunout model"
#define TR_BACKUP_MODEL                "Zálohovat na SD kartu"
#define TR_DELETE_MODEL                "Smazat model"
#define TR_RESTORE_MODEL               "Obnov model z SD karty"
#define TR_DELETE_ERROR                "Nelze odstranit"
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
#define TR_SHOW_MIXER_MONITORS         "Zobrazit monitor mixů"
#define TR_MENU_MODULES_RX_VERSION     "VERZE MODULU A RX"
#define TR_MENU_FIRM_OPTIONS           "MOŽNOSTI FIRMWARE"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Osa/Pot/Slider"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Osa/Pot/Slider"
#define TR_RF_PROTOCOL                 "RF Protokol"
#define TR_MODULE_OPTIONS              "Možnosti modulu"
#define TR_POWER                       "Výkon"
#define TR_NO_TX_OPTIONS               "Žádné možnosti"
#define TR_RTC_BATT                    "RTC Bat"
#define TR_POWER_METER_EXT             "Měřič výkonu (EXT)"
#define TR_POWER_METER_INT             "Měřič výkonu (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spektální an. (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Spektální an. (INT)"
#define TR_SDCARD_FULL                 "Plná karta SD"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT TR_SDCARD_FULL "\nLogování dat a snímky obrazovky vypnuty"
#else
#define TR_SDCARD_FULL_EXT TR_SDCARD_FULL "\036Logy a " LCDW_128_LINEBREAK " Snímky obrazovky vypnuty"
#endif
#define TR_NEEDS_FILE                  "Vyžadován soubor"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Nekompatibilní"
#define TR_WARNING                     "KONTROLA"
#define TR_STORAGE_WARNING             "ÚLOŽIŠTĚ"
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
#define TR_NO_FAILSAFE                 "Failsafe není nastaveno"
#define TR_KEYSTUCK                    "Zaseklá klávesa"
#define TR_VOLUME                      "Hlasitost"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Jas"
#define TR_CPU_TEMP                    "Tepl. CPU\016>"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "Tepl. MB \016>"
#define TR_TTL_WARNING                 "Varování: Nepřekračovat napětí 3.3V u pinů pro TX/RX!"
#define TR_FUNC                        "Funkce"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Trvání"
#define TR_DELAY                       "Zdržet"
#define TR_SD_CARD                     "SD"
#define TR_SDHC_CARD                   "SD-HC"
#define TR_NO_SOUNDS_ON_SD             "žádný zvuk na SD"
#define TR_NO_MODELS_ON_SD             "žádný model na SD"
#define TR_NO_BITMAPS_ON_SD            "žádné obrázky na SD"
#define TR_NO_SCRIPTS_ON_SD            "žádný skript na SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntaktická chyba", "Syntaktická chyba skriptu")
#define TR_SCRIPT_PANIC                "Skript zmaten"
#define TR_SCRIPT_KILLED               "Skript ukončen"
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
#define TR_NA                          "[X]"
#define TR_HARDWARE                    "HARDWARE"
#define TR_FORMATTING                  "Formátovaní..."
#define TR_TEMP_CALIB                  "+=\006Teplota"
#define TR_TIME                        "Čas"
#define TR_MAXBAUDRATE                 "Max baudů"
#define TR_BAUDRATE                    "Baudrate"
#define TR_CRSF_ARMING_MODE            "Arm mód"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 "Vzorkovací režim"
#define TR_SAMPLE_MODES                "Normální","OneBit"
#define TR_LOADING                     "Načítání..."
#define TR_DELETE_THEME                "Smazat motiv?"
#define TR_SAVE_THEME                  "Uložit motiv?"
#define TR_EDIT_COLOR                  "Upravit barvu"
#define TR_NO_THEME_IMAGE              "Náhled motivu nedostupný"
#define TR_BACKLIGHT_TIMER             "Čas zhasnutí displeje"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Rychlý výběr modelu"
  #define TR_LABELS_SELECT             "Label select"
  #define TR_LABELS_MATCH              "Label matching"
  #define TR_FAV_MATCH                 "Favorites matching"
  #define TR_LABELS_SELECT_MODE        "Multi select","Single select"
  #define TR_LABELS_MATCH_MODE         "Match all","Match any"
  #define TR_FAV_MATCH_MODE            "Must match","Optional match"
#endif

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
#define TR_BLUETOOTH_NODEVICES         "Žádné zařízení nalezeno"
#define TR_BLUETOOTH_SCANNING          "Skenování..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Povoleno"
#else
#define TR_BLUETOOTH_MODES             "---","Telemetrie","Trenér  "
#endif
#define TR_SD_INFO_TITLE               "SD INFO"
#define TR_SD_SPEED                    "Rychlost:"
#define TR_SD_SECTORS                  "Sektorů :"
#define TR_SD_SIZE                     "Velikost:"
#define TR_TYPE                        "Typ"
#define TR_GLOBAL_VARS                 "Globální proměnné"
#define TR_GVARS                       "GLOB.PROM."
#define TR_GLOBAL_VAR                  "Globální proměnná"
#define TR_MENU_GLOBAL_VARS            "GLOBÁLNÍ PROMĚNNÉ"
#define TR_OWN                         "\043"
#define TR_DATE                        "Datum"
#define TR_MONTHS                      { "Led", "Úno", "Bře", "Dub", "Kvě", "Čvn", "Čvc", "Srp", "Zář", "Říj", "Lis", "Pro" }
#define TR_ROTARY_ENCODER              "R.Enko"
#define TR_ROTARY_ENC_MODE             TR("R.Enko mód","Režim otočného tlačítka")
#define TR_CHANNELS_MONITOR            "MONITOR KANÁLU"
#define TR_MIXERS_MONITOR              "MONITOR MIXU"
#define TR_PATH_TOO_LONG               "Cesta je moc dlouhá"
#define TR_VIEW_TEXT                   "Zobrazit text"
#define TR_FLASH_BOOTLOADER            "Flash bootloaderu"
#define TR_FLASH_DEVICE                TR("Flash zař.","Flash zařízení")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Flash S.Portem", "Flash S.Portem")
#define TR_FLASH_RECEIVER_OTA          "Flash příjímače OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX skrze ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX skrze int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC skrze ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC skrze int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash modulu BT", "Flash modulu Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT TR("Flash řízení spotř.", "Flash jednotky řízení spotřeby")
#define TR_DEVICE_NO_RESPONSE          TR("Zařízení neodpovídá", "Zařízení neodpovídá")
#define TR_DEVICE_FILE_ERROR           TR("Zař. soubor problém", "Zařízení-problém se souborem")
#define TR_DEVICE_DATA_REFUSED         TR("Zař. data odmítnuta", "Zařízení-data odmítnuta")
#define TR_DEVICE_WRONG_REQUEST        TR("Zař. přístup problém", "Zařízení-problém s přístupem")
#define TR_DEVICE_FILE_REJECTED        TR("Zař. soubor odmítnut", "Zařízení-soubor odmítnut")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Zař. podpis souboru", "Zařízení-neodpovídá podpis")
#define TR_CURRENT_VERSION             TR("Současná ver. ", "Současná verze: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Flash vnitř. modulu", "Flash interního modulu")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash int. Multi", "Flash interního modulu Multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash ext. modulu", "Flash externího modulu")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash ext. Multi", "Flash externího modulu Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash ext. ELRS", "Flash externího modulu ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Chyba zápisu FW", "Chyba zápisu firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Úspěšný zápis FW"
#define TR_WRITING                     "Zapisuji..."
#define TR_CONFIRM_FORMAT              "Formátovat?"
#define TR_INTERNALRF                  "Interní RF modul"
#define TR_INTERNAL_MODULE             "Interní modul"
#define TR_EXTERNAL_MODULE             "Externí modul"
#define TR_EDGETX_UPGRADE_REQUIRED     TR("Aktualizujte EdgeTX", "Vyžadována aktualizace EdgeTX")
#define TR_TELEMETRY_DISABLED          "Telem. zakázána"
#define TR_MORE_OPTIONS_AVAILABLE      TR("Více možností", "Více dostupných možností")
#define TR_NO_MODULE_INFORMATION       "Žádné info o modulu"
#define TR_EXTERNALRF                  "Externí RF modul"
#define TR_FAILSAFE                    TR("Failsafe", "Mód Failsafe")
#define TR_FAILSAFESET                 "NASTAVENÍ FAILSAFE"
#define TR_REG_ID                      TR("Reg. ID", "Registrační ID")
#define TR_OWNER_ID                    "ID vlastníka"
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
#define TR_MENU_SPECTRUM_ANALYSER      "SPEKTRÁLNÍ ANALYZÉR"
#define TR_MENU_POWER_METER            "MĚŘIČ VÝKONU"
#define TR_SENSOR                      "SENZOR"
#define TR_COUNTRY_CODE                "Kód regionu"
#define TR_USBMODE                     "Režim USB"
#define TR_JACK_MODE                   "Režim Jack"
#define TR_VOICE_LANGUAGE              "Jazyk hlasu"
#define TR_UNITS_SYSTEM                "Jednotky"
#define TR_UNITS_PPM                   "PPM jednotky"
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
#define TR_RESET_FLIGHT                "Reset relace"
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
#define TR_SPEAKER                     "Repro"
#define TR_BUZZER                      "Pípák"
#define TR_BYTES                       "[B]"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_MODULE_UNBIND               BUTTON("Odpárovat")
#define TR_POWERMETER_ATTN_NEEDED     "Útlumový člen nutný!"
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
#define TR_RANGE_TEST                  "Test dosahu"
#define TR_RECEIVER_OPTIONS            TR("MOŽNOSTI RX", "MOŽNOSTI PŘIJÍMAČE")
#define TR_RESET_BTN                   BUTTON("Reset")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON(TR("SW","Přepínače"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analogy"))
#define TR_FS_BTN                      BUTTON(TR("Vlastní Sw", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Dotyková obrazovka nenalezena"
#define TR_TOUCH_EXIT                  "Dotkněte se obrazovky pro ukončení"
#define TR_SET                         BUTTON("Nast")
#define TR_TRAINER                     "Trenér"
#define TR_CHANS                       "Kanály"
#define TR_ANTENNAPROBLEM              "Problém s TX anténou!"
#define TR_MODELIDUSED                 TR("ID již použito","ID modelu je již použito")
#define TR_MODELIDUNIQUE               "ID je unikátní"
#define TR_MODULE                      "Modul"
#define TR_RX_NAME                     "Jméno RX"
#define TR_TELEMETRY_TYPE              TR("Typ", "Typ telemetrie")
#define TR_TELEMETRY_SENSORS           "Senzory"
#define TR_VALUE                       "Hodnota"
#define TR_PERIOD                      "Perioda"
#define TR_INTERVAL                    "Interval"
#define TR_REPEAT                      "Opakovat"
#define TR_ENABLE                      "Povoleno"
#define TR_DISABLE                     "Zakazano"
#define TR_TOPLCDTIMER                 "Stopky horního LCD"
#define TR_UNIT                        "Jednotky"
#define TR_TELEMETRY_NEWSENSOR         "Přidat senzor ručně"
#define TR_CHANNELRANGE                TR("Kanály", "Rozsah kanálů")
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
#define TR_LOWALARM                    "Nízký Alarm"
#define TR_CRITICALALARM               "Kritický Alarm"
#define TR_DISABLE_ALARM               TR("Vypnout alarmy", "Vypnout alarmy telemetrie")
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
#define TR_COPY_STICKS_TO_OFS          TR("Kop. osy->subtrim", "Kopírovat osy do subtrimu")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Kopie min/max do všech",  "Kopírovat min/max/střed do všech výstupů")
#define TR_COPY_TRIMS_TO_OFS           TR("Trimy do subtrimu", "Kopírovat trimy do subtrimů")
#define TR_INCDEC                      "Zvětšit/Zmenšit"
#define TR_GLOBALVAR                   "Glob. proměnná"
#define TR_MIXSOURCE                   "Zdroj (%)"
#define TR_MIXSOURCERAW                "Zdroj (hodnota)"
#define TR_CONSTANT                    "Konstanta"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "Vyp","Zap","Auto"
#define TR_PREFLIGHT                   "Předletová kontrola"
#define TR_CHECKLIST                   TR("Poznámky", "Zobrazit poznámky")
#define TR_CHECKLIST_INTERACTIVE       TR3("S-interkativni", "Interaktivní seznam", "Interaktivní seznam")
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
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

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


#define TR_LSW_DESCRIPTIONS            { "Typ porovnávací funkce", "První proměnná", "Druhá proměnná nebo konstanta", "Druhá proměnná nebo konstanta", "Další stav povolující tenko spínač", "Minimální doba trvání sepnutého stavu", "Minimální doba platné podmínky pro aktivaci" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "První kanál"
  #define TR_FILL_BACKGROUND           "Vyplnit pozadí?"
  #define TR_BG_COLOR                  "Barva pozadí"
  #define TR_SLIDERS_TRIMS             "Slidery+Trimy"
  #define TR_SLIDERS                   "Slidery"
  #define TR_FLIGHT_MODE               "Letový režim"
  #define TR_INVALID_FILE              "Neplatný soubor"
  #define TR_TIMER_SOURCE              "Časovač zdroj"
  #define TR_SIZE                      "Velikost"
  #define TR_SHADOW                    "Stíny"
  #define TR_ALIGNMENT                 "Zarovnání"
  #define TR_ALIGN_LABEL               "Zarovnat název"
  #define TR_ALIGN_VALUE               "Zarovnat hodnotu"
  #define TR_ALIGN_OPTS                { "Vlevo", "Uprostřed", "Vpravo" }
  #define TR_TEXT                      "Text"
  #define TR_COLOR                     "Barva"
  #define TR_MAIN_VIEW_X               "Hlavní pohled XX"
  #define TR_PANEL1_BACKGROUND         "Panel1 pozadí"
  #define TR_PANEL2_BACKGROUND         "Panel2 pozadí"
  #define TR_WIDGET_GAUGE              "Měřidlo"
  #define TR_WIDGET_MODELBMP           "Model info"
  #define TR_WIDGET_OUTPUTS            "Výstupy"
  #define TR_WIDGET_TEXT               "Text"
  #define TR_WIDGET_TIMER              "Časovač"
  #define TR_WIDGET_VALUE              "Hodnota"
#endif

// Bootloader common (Poznamka: nutne pouziti textu bez diakritiky - omezeni velikosti pameti bootloader!)
#define TR_BL_USB_CONNECTED           "USB pripojeno"
#define TR_BL_USB_PLUGIN              "nebo pripojte USB kabel"
#define TR_BL_USB_MASS_STORE          "pro pouziti uloziste"
#define TR_BL_USB_PLUGIN_MASS_STORE   "nebo pripojte USB kabel pro pouziti uloziste"
#define TR_BL_WRITE_FW                "Nahrat firmware"
#define TR_BL_FORK                    "Fork:"
#define TR_BL_VERSION                 "Verze:"
#define TR_BL_RADIO                   "Radio:"
#define TR_BL_EXIT                    "Ukoncit"
#define TR_BL_DIR_MISSING             " Adresar chybi"
#define TR_BL_DIR_EMPTY               " Adresar je prazdny"
#define TR_BL_WRITING_FW              "Nahravani firmware ..."
#define TR_BL_WRITING_COMPL           "Nahravani dokonceno"

#if LCD_W >= 480
  #define TR_BL_INVALID_FIRMWARE       "Neplatny soubor s firmwarem"
#elif LCD_W >= 212
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN_MASS_STORE
  #define TR_BL_HOLD_ENTER_TO_START    "\012Drzet [ENT] pro zahajeni nahravani"
  #define TR_BL_INVALID_FIRMWARE       "\011Neplatny firmware soubor!        "
  #define TR_BL_INVALID_EEPROM         "\011Neplatny EEPROM soubor!          "
#else
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
  #define TR_BL_HOLD_ENTER_TO_START    "\006Drzet [ENT] pro zahajeni"
  #define TR_BL_INVALID_FIRMWARE       "\004Neplatny firmware!        "
  #define TR_BL_INVALID_EEPROM         "\004Neplatny EEPROM!          "
#endif

#if defined(PCBTARANIS)
   // Bootloader Taranis specific - ASCII characters only
  #define TR_BL_RESTORE_EEPROM        "Obnovit EEPROM"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Stisknete tlacitko napajeni."
    #define TR_BL_FLASH_EXIT          "Ukoncit rezim nahravani."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - ASCII characters only
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_SELECT_KEY            "[ENT] pro vybrani souboru"
  #define TR_BL_FLASH_KEY             "Drzet dlouze [ENT] pro nahrani"
  #define TR_BL_ERASE_KEY             "Hold [ENT] long to erase"
  #define TR_BL_EXIT_KEY              "[RTN] pro ukonceni"
#elif defined(PCBNV14)
   // Bootloader NV14 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB pristup"
  #define TR_BL_CURRENT_FW            "Aktualni firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] pro vybrani souboru"
  #define TR_BL_FLASH_KEY             "Drzet dlouze [R TRIM] pro nahrani"
  #define TR_BL_EXIT_KEY              " [L TRIM] pro ukonceni"
  #define TR_BL_ENABLE                "Povoleno"
  #define TR_BL_DISABLE               "Zakazano"
#elif defined(PCBPL18)
   // Bootloader PL18 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_SELECT_KEY            " [TR4 Dn] to select file"
  #define TR_BL_FLASH_KEY             " Hold [TR4 Dn] long to flash"
  #define TR_BL_ERASE_KEY             " Hold [TR4 Dn] long to erase"
  #define TR_BL_EXIT_KEY              " [TR4 Up] to exit"
#endif

// About screen
#define TR_ABOUTUS                     "O nás"

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
#define TR_ALTITUDE                    "Výška"
#define TR_SCALE                       "Měřítko"
#define TR_VIEW_CHANNELS               "Zobrazit kanály"
#define TR_VIEW_NOTES                  "Zobrazit poznámky"
#define TR_MODEL_SELECT                "Zvolit model"
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
#define TR_IGNORE_INSTANCE             TR("Chybné ID", "Ignoruj chyby ID")
#define TR_SHOW_INSTANCE_ID            "Zobrazit ID instance"
#define TR_DISCOVER_SENSORS            "Detekovat nové senzory"
#define TR_STOP_DISCOVER_SENSORS       "Zastavit autodetekci"
#define TR_DELETE_ALL_SENSORS          "Odebrat všechny senzory"
#define TR_CONFIRMDELETE               "Opravdu " LCDW_128_LINEBREAK "odstranit vše?"
#define TR_SELECT_WIDGET               "Zvolit widget"
#define TR_WIDGET_FULLSCREEN           "Celá obrazovka"
#define TR_REMOVE_WIDGET               "Odstranit widget"
#define TR_WIDGET_SETTINGS             "Nastavení widgetu"
#define TR_REMOVE_SCREEN               "Odstranit panel"
#define TR_SETUP_WIDGETS               "Nastavit widgety"
#define TR_USER_INTERFACE              "Uživatelské rozhraní"
#define TR_THEME                       "Motiv"
#define TR_SETUP                       "Nastavení"
#define TR_LAYOUT                      "Rozložení"
#define TR_ADD_MAIN_VIEW               "Přidat hlavní panel"
#define TR_TEXT_COLOR                  "Barva textu"

#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Vstupy"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua skripty"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Osa"
#define TR_MENU_POTS                   STR_CHAR_POT "Potenciometry"
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "MIN"
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
#define TR_AUDIO_MUTE                  TR("Ztlumení zvuku","Ztlumení, pokud není slyšet zvuk")
#define TR_JITTER_FILTER               "ADC Filtr"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Kontr RTC", "Hlídat RTC napětí")
#define TR_AUTH_FAILURE                "Auth-selhala"
#define TR_RACING_MODE                 "Závodní režim"

#define TR_USE_THEME_COLOR              "Použít barevný motiv"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Trimy do subtrimů"
#if !PORTRAIT_LCD
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
#define TR_THEME_COLOR_DEFAULT          "DEFAULTNÍ"
#define TR_THEME_COLOR_PRIMARY1         "PRIMÁRNÍ1"
#define TR_THEME_COLOR_PRIMARY2         "PRIMÁRNÍ2"
#define TR_THEME_COLOR_PRIMARY3         "PRIMÁRNÍ3"
#define TR_THEME_COLOR_SECONDARY1       "SEKUNDÁRNÍ1"
#define TR_THEME_COLOR_SECONDARY2       "SEKUNDÁRNÍ2"
#define TR_THEME_COLOR_SECONDARY3       "SEKUNDÁRNÍ3"
#define TR_THEME_COLOR_FOCUS            "VYBRANÉ"
#define TR_THEME_COLOR_EDIT             "UPRAVIT"
#define TR_THEME_COLOR_ACTIVE           "AKTIVNÍ"
#define TR_THEME_COLOR_WARNING          "VAROVÁNÍ"
#define TR_THEME_COLOR_DISABLED         "VYPNUTÉ"
#define TR_THEME_COLOR_CUSTOM           "VLASTNÍ"
#define TR_THEME_CHECKBOX               "Pole výběru"
#define TR_THEME_ACTIVE                 "Aktivní"
#define TR_THEME_REGULAR                "Normální"
#define TR_THEME_WARNING                "Varování"
#define TR_THEME_DISABLED               "Vypnuté"
#define TR_THEME_EDIT                   "Upravit"
#define TR_THEME_FOCUS                  "Vybrané"
#define TR_AUTHOR                       "Autor"
#define TR_DESCRIPTION                  "Popis"
#define TR_SAVE                         "Uložit"
#define TR_CANCEL                       "Zrušit"
#define TR_EDIT_THEME                   "EDITOVAT MOTIV"
#define TR_DETAILS                      "Detaily"
#define TR_THEME_EDITOR                 "Motivy"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Vybrat\nmodel"
#define TR_MAIN_MENU_MANAGE_MODELS      "Správa\nmodelů"
#define TR_MAIN_MENU_MODEL_NOTES        "Poznámky\nmodelu"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Monitor\nkanálů"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Nastavení\nmodelu"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Nastavení\nrádia"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Nastavení\nobrazovky"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Reset\ntelemetrie"
#define TR_MAIN_MENU_STATISTICS         "Statistiky"
#define TR_MAIN_MENU_ABOUT_EDGETX       "O\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "Angličtina"
#define TR_VOICE_CHINESE                "Čínština"
#define TR_VOICE_CZECH                  "Čeština"
#define TR_VOICE_DANISH                 "Dánština"
#define TR_VOICE_DEUTSCH                "Němčina"
#define TR_VOICE_DUTCH                  "Nizozemština"
#define TR_VOICE_ESPANOL                "Španělština"
#define TR_VOICE_FRANCAIS               "Francouzština"
#define TR_VOICE_HUNGARIAN              "Maďarština"
#define TR_VOICE_ITALIANO               "Italština"
#define TR_VOICE_POLISH                 "Polština"
#define TR_VOICE_PORTUGUES              "Portugalština"
#define TR_VOICE_RUSSIAN                "Ruština"
#define TR_VOICE_SLOVAK                 "Slovenština"
#define TR_VOICE_SWEDISH                "Švédština"
#define TR_VOICE_TAIWANESE              "Tchajwanština"
#define TR_VOICE_JAPANESE               "Japonština"
#define TR_VOICE_HEBREW                 "Hebrejština"
#define TR_VOICE_UKRAINIAN              "Ukrainština"

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
#define TR_VUSBJOYSTICK_CH_SIM         "Ail","Ele","Rud","Thr","Acc","Brk","Steer","Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Inversion"
#define TR_USBJOYSTICK_CH_BTNNUM       "Button no."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Button no. collision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Axis collision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "Žádný","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Apply changes"

#define TR_DIGITAL_SERVO               "Servo 333Hz"
#define TR_ANALOG_SERVO                "Servo 50Hz"
#define TR_SIGNAL_OUTPUT               "Signal výstup"
#define TR_SERIAL_BUS                  "Serial BUS"
#define TR_SYNC                        "Synchronizovat"

#define TR_ENABLED_FEATURES            "Aktivní funkce"
#define TR_RADIO_MENU_TABS             "Rádiové menu záložky"
#define TR_MODEL_MENU_TABS             "Model menu záložky"

#define TR_SELECT_MENU_ALL        "Vše"
#define TR_SELECT_MENU_CLR        "Vymazat"
#define TR_SELECT_MENU_INV        "Invertovat"

#define TR_SORT_ORDERS            "Název A-Z","Název Z-A","Nejméne používané","Nejvíce používané"
#define TR_SORT_MODELS_BY         "Seřadit modely podle"
#define TR_CREATE_NEW             "Vytvořit"

#define TR_MIX_SLOW_PREC          TR("Přesnost zpomal", "Přesnost zpomalení")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "Adresář vzhledu s tímto názvem již existuje."

#define TR_DATE_TIME_WIDGET       "Datum a čas"
#define TR_RADIO_INFO_WIDGET      "Informace o vysílačce"
#define TR_LOW_BATT_COLOR         "Vybitá baterie"
#define TR_MID_BATT_COLOR         "Středně nabitá baterie"
#define TR_HIGH_BATT_COLOR        "Plně nabitá baterie"

#define TR_WIDGET_SIZE            "Velikost widgetu"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
