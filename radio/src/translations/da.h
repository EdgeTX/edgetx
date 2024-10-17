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

// DK translations author: Henrik Thuren <thuren.henrik@gmail.com>

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

#define TR_MINUTE_SINGULAR             "minut"
#define TR_MINUTE_PLURAL1              "minutter"
#define TR_MINUTE_PLURAL2              "minutter"

#define TR_OFFON                       "FRA","TIL"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Stille","Alarm","NoKey","Alle"
#define TR_VBLMODE                     "FRA","Taster",TR("Ctrl","Controls"),"Begge","TIL"
#define TR_TRNMODE                     "FRA",TR("+=","Læg til"),TR(":=","Erstat")
#define TR_TRNCHN                      "KA1","KA2","KA3","KA4"
#define TR_AUX_SERIAL_MODES            "FRA","Telem spejlet","Telemetri ind","SBUS træner","LUA","CLI","GPS","Debug","SpaceMouse","Eksternt modul"

#if !PORTRAIT_LCD
  #define TR_SWTYPES                   "Ingen", "2 pos skift","2 position","3 position"
#else
  #define TR_SWTYPES                   "Ingen","Skift","2POS","3POS"
#endif

#define TR_POTTYPES                    "Ingen",TR("Drejek.", "Drejekontakt"),TR("Drejek./klik","Drejekontakt med klik"),"Skyder",TR("Multipos","Multipos kontakt"),"Axis X","Axis Y","Switch"
#define TR_VPERSISTENT                 "FRA","Flyv","Manuel nulstil"
#define TR_COUNTRY_CODES               TR("US","Amerika"),TR("JP","Japan"),TR("EU","Europa")
#define TR_USBMODES                    "Spørg",TR("Joyst","Joystik"),TR("Lager","USB lager"),TR("Data", "USB data")
#define TR_JACK_MODES                  "Spørg","Audio","Træner"

#define TR_SBUS_INVERSION_VALUES       "normal","ikke invers"
#define TR_MULTI_CUSTOM                "Tilpasset"
#define TR_VTRIMINC                    TR("Expo","Exponentiel"),TR("ExFin","Ekstra fin"),"Fin","Medium","Grov"
#define TR_VDISPLAYTRIMS               "Nej","Ændre","Ja"
#define TR_VBEEPCOUNTDOWN              "Stille","Bip","Stemme","Vibration",TR("B & V","Bips & Vibration"),TR("St & Vib","Stemme & Vibration")
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VVARIOCENTER                "Tone","Stille"
#define TR_CURVE_TYPES                 "Standard","Tilpasset"

#define TR_ADCFILTERVALUES             "Global","Fra","Til"

#define TR_VCURVETYPE                  "Diff","Expo","Funk","Egen"
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Læg til","Gange","Erstat"
#define TR_VMLTPX2                     "+=","*=",":="

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Tid"
  #define TR_CSWSTICKY                 "Sej"
  #define TR_CSWSTAY                   "Edge"
#else
  #define TR_CSWTIMER                  "Tid"
  #define TR_CSWSTICKY                 "Sej"
  #define TR_CSWSTAY                   "Edge"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Træner"
#define TR_SF_INST_TRIM                "Inst. Trim"
#define TR_SF_RESET                    "Nulstil"
#define TR_SF_SET_TIMER                "Sæt"
#define TR_SF_VOLUME                   "Lydstyrke"
#define TR_SF_FAILSAFE                 TR("Sæt fejlsik", "Sæt fejlsikring")
#define TR_SF_RANGE_CHECK              TR("Afst. check", "Afstand kontrol")
#define TR_SF_MOD_BIND                 "Modul tilslut"
#define TR_SF_RGBLEDS                  "RGB led"

#define TR_SOUND                       "Spil lyd"
#define TR_PLAY_TRACK                  TR("Ply Trk", "Afspil lydfil")
#define TR_PLAY_VALUE                  TR("Spil Vær","Spil Værdi")
#define TR_SF_HAPTIC                   TR("Vib.", "Vibration")
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua Script")
#define TR_SF_BG_MUSIC                 "BgMusik"
#define TR_SF_BG_MUSIC_PAUSE           "BgMusik ||"
#define TR_SF_LOGS                     "SD Log"
#define TR_ADJUST_GVAR                 "Juster"
#define TR_SF_BACKLIGHT                "Baggrundslys"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "Test"
#define TR_SF_SAFETY                   TR("Overs.", "Overskriv")

#define TR_SF_SCREENSHOT               "Skærm klip"
#define TR_SF_RACING_MODE              "Ræs tilstand"
#define TR_SF_DISABLE_TOUCH            "Ikke berøringsaktiv"
#define TR_SF_DISABLE_AUDIO_AMP        TR("Sluk audio amp", "Sluk audio amplifier")
#define TR_SF_SET_SCREEN               "Vælg hoved skærm"
#define TR_SF_SET_TELEMSCREEN          "Vis skærm"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Telm", "Telemetri")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS            "Tid1","Tid2","Tid3"


#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("Alle","Flight"),TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS                  TR("Bi1","Bip1"),TR("Bi2","Bip2"),TR("Bi3","Bi3"),TR("Adv1","Advarsel1"),TR("Adv2","Advarsel2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Sirene"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "Metrisk",TR("Imper.","Imperiel")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Ingen","Tal","Bjælke","Script"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "Sid"
#define TR_STICK_NAMES1                "Høj"
#define TR_STICK_NAMES2                "Gas"
#define TR_STICK_NAMES3                "Kræ"
#define TR_SURFACE_NAMES0              "Sid"
#define TR_SURFACE_NAMES1              "Gas"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Standard","Høj"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "EN","En"

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "Joystik indstilling"
#define TR_HATSOPT                     "Kun trim","Kun knap","Trim / Knap", "Global"
#define TR_HATSMODE_TRIMS              "Joystik: Trim"
#define TR_HATSMODE_KEYS               "Joystik: Knap"
#define TR_HATSMODE_KEYS_HELP          "Venstre side:\n"\
                                       "   Højre = MDL\n"\
                                       "   Up    = SYS\n"\
                                       "   Down = TELE\n"\
                                       "\n"\
                                       "Højre side:\n"\
                                       "   Venstre = PAGE<\n"\
                                       "   Højre   = PAGE>\n"\
                                       "   Op  = PREV/INC\n"\
                                       "   Ned = NEXT/DEC"
#endif

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT            "Normal","Invers"
#else
  #define TR_ROTARY_ENC_OPT            "Normal","Invers","V-I H-N","V-I H-A","V-N E-I"
#endif

#if defined(IMU)
  #define TR_IMU_VSRCRAW               "TitX","TitY",
#else
  #define TR_IMU_VSRCRAW
#endif

#if defined(HELI)
  #define TR_CYC_VSRCRAW               "CYC1","CYC2","CYC3"
#else
  #define TR_CYC_VSRCRAW               "[C1]","[C2]","[C3]"
#endif


#define TR_SRC_BATT                    "Bat."
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tid"

#define TR_VTMRMODES                   "FRA","TIL","Strt","THs","TH%","THt"
#define TR_VTRAINER_MASTER_OFF         "FRA"
#define TR_VTRAINER_MASTER_JACK        "Træner/Stik"
#define TR_VTRAINER_SLAVE_JACK         "Elev/Stik"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Træner/SBUS modul"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Træner/CPPM modul"
#define TR_VTRAINER_MASTER_BATTERY     "Træner/Seriel"
#define TR_VTRAINER_BLUETOOTH          "Træner/" TR("BT","Bluetooth"), "Elev/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Træner/Multi"
#define TR_VFAILSAFE                   "Ikke sat","Hold","Tilpasset","Ingen puls","Modtager"
#define TR_VSENSORTYPES                "Tilpasset","Beregnet"
#define TR_VFORMULAS                   "Tillæg","Gennems.","Min","Max","Gange","Sum","Cell","Forbrug","Afstand"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Lavest","1","2","3","4","5","6","7","8","Højst","Delta"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center kun)","= (symmetrisk)"
#define TR_TIMER_DIR                   TR("Rest", "Vis rest"), TR("Gået tid", "Vis gået tid")
#define TR_PPMUNIT                     "0.--","0.0","us"

#if defined(COLORLCD)
  #if defined(BOLD)
    #define TR_FONT_SIZES              "STD"
  #else
    #define TR_FONT_SIZES              "STD","FED","XXS","XS","L","XL","XXL"
  #endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14) || defined(PCBPL18)
  #define TR_ENTER                     "[NÆSTE]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#if defined(COLORLCD)
  #define TR_EXIT                      "RTN"
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "FORLAD"
  #define TR_OK                        TR("\010" "\010" "\010" "[OK]", "\010" "\010" "\010" "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_FREE                        "fri"
#define TR_YES                         "Ja"
#define TR_NO                          "Nej"
#define TR_DELETEMODEL                 "SLET MODEL"
#define TR_COPYINGMODEL                "Kopier model..."
#define TR_MOVINGMODEL                 "Flytter model..."
#define TR_LOADINGMODEL                "Indlæser model..."
#define TR_UNLABELEDMODEL              "Typeløs"
#define TR_NAME                        "Navn"
#define TR_MODELNAME                   "Model navn"
#define TR_PHASENAME                   "Mode navn"
#define TR_MIXNAME                     "Mix navn"
#define TR_INPUTNAME                   TR("Indgang", "Indgang navn")
#define TR_EXPONAME                    TR("Navn", "Line navn")
#define TR_BITMAP                      "Model billede"
#define TR_NO_PICTURE                  "Ingen billede"
#define TR_TIMER                       TR("Tid", "Tid ")
#define TR_START                       "Start"
#define TR_ELIMITS                     TR("Udv.Grænser", "Udvidet grænser")
#define TR_ETRIMS                      TR("Udv.Trim", "Udvidet trim")
#define TR_TRIMINC                     "Trim øge"
#define TR_DISPLAY_TRIMS               TR("Vis trim", "Vis trim")
#define TR_TTRACE                      TR("T-kilde", "kilde")
#define TR_TTRIM                       TR("T-trim-tomg", "Trim tomgang alene")
#define TR_TTRIM_SW                    TR("T-trim-ko", "Trim kontakt")
#define TR_BEEPCTR                     TR("Bip ctr pos", "Bip ved center position")
#define TR_USE_GLOBAL_FUNCS            TR("Glob.Funk.", "Brug global funk.")
#define TR_PROTOCOL                    TR("Proto", "Protokol")
#define TR_PPMFRAME                    "PPM frame"
#define TR_REFRESHRATE                 TR("Genfrisk", "Genfrisk rate")
#define STR_WARN_BATTVOLTAGE           TR("Udgang er VBAT: ", "Advarsel: udgang niveau er VBAT: ")
#define TR_WARN_5VOLTS                 "Advarsel: udgang niveau er 5 volts"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Kontakt"
#define TR_FUNCTION_SWITCHES           "Kontakter der kan tilpasses"
#define TR_GROUP                       "Gruppe"
#define TR_GROUP_ALWAYS_ON             "Altid til"
#define TR_GROUP                       "Gruppe"
#define TR_GROUP_ALWAYS_ON             "Altid til"
#define TR_GROUPS                      "Altid til gruppe"
#define TR_LAST                        "Sidste"
#define TR_MORE_INFO                   "Mere info"
#define TR_SWITCH_TYPE                 "Type"
#define TR_SWITCH_STARTUP              "Start"
#define TR_SWITCH_GROUP                "Gruppe"
#define TR_FUNCTION_SWITCH_GROUPS      "---", TR_SWITCH_GROUP" 1", TR_SWITCH_GROUP" 2", TR_SWITCH_GROUP" 3"
#define TR_SF_SWITCH                   "Udløser"
#define TR_TRIMS                       "Trim"
#define TR_FADEIN                      "Tone ind"
#define TR_FADEOUT                     "Tone ud"
#define TR_DEFAULT                     "(standard)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Kontroller FT trim"
#else
  #define TR_CHECKTRIMS                "\006Kontroller\012trim"
#endif
#define TR_SWASHTYPE                   "Swash type"
#define TR_COLLECTIVE                  TR("Collective", "Coll. pitch kilde")
#define TR_AILERON                     TR("Lateral cyc.", "Lateral cyc. kilde")
#define TR_ELEVATOR                    TR("Long. cyc.", "Long. cyc. kilde")
#define TR_SWASHRING                   "Swash ring"
#define TR_MODE                        TR("Tils.","Tilstand")
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "Venstre"
#else
  #define TR_LEFT_STICK                "Ven"
#endif
#define TR_SUBTYPE                     "Subtype"
#define TR_NOFREEEXPO                  "Ingen fri expo!"
#define TR_NOFREEMIXER                 "Ingen fri mix!"
#define TR_SOURCE                      "Kilde"
#define TR_WEIGHT                      "Vægt"
#define TR_SIDE                        "Side"
#define TR_OFFSET                       "Offset"
#define TR_TRIM                        "Trim"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Kurve"
#define TR_FLMODE                      TR("Tilstand", "Tilstande")
#define TR_MIXWARNING                  "Advarsel"
#define TR_OFF                         "FRA"
#define TR_ANTENNA                     "Antenne"
#define TR_NO_INFORMATION              TR("Ingen info", "Ingen information")
#define TR_MULTPX                      "Multiplex"
#define TR_DELAYDOWN                   TR("Forsink ned", "Forsinkelse ned")
#define TR_DELAYUP                     TR("Forsink op", "Forsinkelse op")
#define TR_SLOWDOWN                    TR("Langsom ned", "Langsom ned")
#define TR_SLOWUP                      TR("Langsom op", "Langsom op")
#define TR_MIXES                       "MIX"
#define TR_CV                          "KU"
#if defined(PCBNV14)
  #define TR_GV                        "GV"
#else
  #define TR_GV                        TR("G", "GV")
#endif

#define TR_RANGE                       TR("Max ned/op", "Max synke/stige")
#define TR_CENTER                      TR("Min ned/op", "Min synke/stige")

#define TR_ALARM                       "Alarm"
#define TR_BLADES                      "Blade/Poler"
#define TR_SCREEN                      "Skærm\001"
#define TR_SOUND_LABEL                 "Lyd"
#define TR_LENGTH                      "Længde"
#define TR_BEEP_LENGTH                 "Bip længde"
#define TR_BEEP_PITCH                  "Bip højde"
#define TR_HAPTIC_LABEL                "Vibration"
#define TR_STRENGTH                    "Styrke"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Offset"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Kontrast"
#define TR_ALARMS_LABEL                "Alarmer"
#define TR_BATTERY_RANGE               TR("Bat. interval", "Batteri spændings interval")
#define TR_BATTERYCHARGING             "Lader..."
#define TR_BATTERYFULL                 "Fuldt batteri"
#define TR_BATTERYNONE                 "Intet!"
#define TR_BATTERYWARNING              "Lavt batteri"
#define TR_INACTIVITYALARM             "Inaktiv"
#define TR_MEMORYWARNING               "Lav hukommelse"
#define TR_ALARMWARNING                "Lyd fra"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI alarm", "Kontroller RSSI forbindelse")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Træner alarm", "Kontroller træner forbindelse")
#define TR_MODEL_STILL_POWERED         "Model stadig med strøm"
#define TR_TRAINER_STILL_CONNECTED     "Træner fortsat forbundet"
#define TR_USB_STILL_CONNECTED         "USB stadig forbundet"
#define TR_MODEL_SHUTDOWN              "Luk ned?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Tryk enter for bekræft"
#define TR_THROTTLE_LABEL              "Gas"
#define TR_THROTTLE_START              "Gas Start"
#define TR_THROTTLEREVERSE             TR("T-omvendt", "Omvendt")
#define TR_MINUTEBEEP                  TR("Minut", "Minut kald")
#define TR_BEEPCOUNTDOWN               "Nedtælling"
#define TR_PERSISTENT                  TR("Varig", "Varig")
#define TR_BACKLIGHT_LABEL             "Baggrunds lys"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLONBRIGHTNESS              "TIL klarhed"
#define TR_BLOFFBRIGHTNESS             "FRA klarhed"
#define TR_KEYS_BACKLIGHT              "Taster klarhed"
#define TR_BLCOLOR                     "Farve"
#define TR_SPLASHSCREEN                "Start skærm"
#define TR_PLAY_HELLO                  "Startop lyd"
#define TR_PWR_ON_DELAY                "Forsinkelse ved tænd"
#define TR_PWR_OFF_DELAY               "Forsinkelse ved sluk"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("Vib. strøm til/fra","Vibrator strøm til/fra")
#define TR_THROTTLE_WARNING            TR("Gas adv", "Gas advarsel")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Tilp-Pos", "Tilpasset position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR("Kon. adv", "Kontakt position")
#define TR_POTWARNINGSTATE             "Drejekontakt & skyder position"
#define TR_POTWARNING                  TR("Drejk. adv", "Drejekontakt position")
#define TR_TIMEZONE                    "Tids zone"
#define TR_ADJUST_RTC                  "Juster RTC"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                "Kanalrækkefølge"
#define TR_STICKS                      "Pinde"
#define TR_POTS                        "Drejekontakt"
#define TR_SWITCHES                    "Kontakter"
#define TR_SWITCHES_DELAY              TR("Cen forsink", "Center forsinkelse")
#define TR_SLAVE                       "Slave"
#define TR_MODESRC                     "Mode\006% Kilde"
#define TR_MULTIPLIER                  "Multiplier"
#define TR_CAL                         "Kal"
#define TR_CALIBRATION                 "Kalibrering"
#define TR_VTRIM                       "Trim - +"
#define TR_CALIB_DONE                  "Kalibrering færdig"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Tryk [Enter] for start"
  #define TR_SETMIDPOINT               "Centrer pinde/drejekontakter/skydere og tryk [Enter]"
  #define TR_MOVESTICKSPOTS            "Flyt pinde/drejekontakter/skydere og tryk [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " FOR START"
  #define TR_SETMIDPOINT               "CENTRER PINDE/SKYDER"
  #define TR_MOVESTICKSPOTS            "FLYT PINDE/DREJEKONTAKT"
#else
  #define TR_MENUTOSTART               TR_ENTER " FOR START"
  #define TR_SETMIDPOINT               TR("SÆT PINDE I MIDT", "CENTRER PINDE/SKYDERE")
  #define TR_MOVESTICKSPOTS            "FLYT PINDE/DREJEKONTAKTER"
  #define TR_MENUWHENDONE              TR_ENTER " NÅR FÆRDIG"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      "INGEN DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Fri stak"
#define TR_INT_GPS_LABEL               "Intern GPS"
#define TR_HEARTBEAT_LABEL             "Hjerte puls"
#define TR_LUA_SCRIPTS_LABEL           "Lua script"
#define TR_FREE_MEM_LABEL              "Fri mem"
#define TR_DURATION_MS                 TR("[D]","Varighed(ms): ")
#define TR_INTERVAL_MS                 TR("[I]","Interval(ms): ")
#define TR_MEM_USED_SCRIPT             "Script(B): "
#define TR_MEM_USED_WIDGET             "Widget(B): "
#define TR_MEM_USED_EXTRA              "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Audio: "
#define TR_GPS_FIX_YES                 "Fix: Ja"
#define TR_GPS_FIX_NO                  "Fix: Nej"
#define TR_GPS_SATS                    TR("Sat.: ", "Satelitter:")
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Menu: "
#define TR_TIMER_LABEL                 "Tid"
#define TR_THROTTLE_PERCENT_LABEL      "Gas %"
#define TR_BATT_LABEL                  "Batteri"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " for nulstil"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "KA"
#define TR_MODEL                       "MODEL"
#if defined(SURFACE_RADIO)
  #define TR_FM                        "DM"
#else
  #define TR_FM                        "FT"
#endif
#define TR_EEPROMLOWMEM                "EEPROM lav mem"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Tryk en tast ..."
#define TR_THROTTLE_NOT_IDLE           TR("Gas ikke sat lav", "Gas ikke sat i tomgang")
#define TR_ALARMSDISABLED              "Alarmer afkoblet"
#define TR_PRESSANYKEY                 TR("\010Tryk en tast", "Tryk en tast")
#define TR_BADEEPROMDATA               "Dårlig EEprom data"
#define TR_BAD_RADIO_DATA              "Manglende eller dårlig radio data"
#define TR_RADIO_DATA_RECOVERED        TR3("Anvender radio data fra backup","Anvender radio indstillinger fra backup","Radio indstillinger genskabt fra backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio indstillinger fejlagtige","Radio indstillinger ikke gyyldige", "Ikke muligt at indlæse gyyldige radio indstillinger")
#define TR_EEPROMFORMATTING            "Formaterer EEPROM"
#define TR_STORAGE_FORMAT              "Lager klargøres"
#define TR_EEPROMOVERFLOW              "EEPROM overflow"
#define TR_RADIO_SETUP                 "RADIO SETUP"
#define TR_MENUTRAINER                 "TRÆNER"
#define TR_MENUSPECIALFUNCS            "GLOBALE FUNKTIONER"
#define TR_MENUVERSION                 "VERSION"
#define TR_MENU_RADIO_SWITCHES         TR("KONTAKTER", "KONTAKTER TEST")
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "ANALOGS TEST")
#define TR_MENU_RADIO_ANALOGS_CALIB    "KALIBRERET ANALOGE"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RÅ ANALOGE (5 Hz)"
#define TR_MENUCALIBRATION              "KALIBRIERING"
#define TR_MENU_FSWITCH                 "KONTAKTER DER KAN TILPASSES"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trim => Subtrim"
#else
  #define TR_TRIMS2OFFSETS             "\006Trim => Subtrim"
#endif
#define TR_CHANNELS2FAILSAFE           "Kanaler=>Fejlsikring"
#define TR_CHANNEL2FAILSAFE            "Kanal=>Fejlsikring"
#define TR_MENUMODELSEL                TR("MODELLER", "MODEL VALG")
#define TR_MENU_MODEL_SETUP            TR("SETUP", "MODEL SETUP")
#if defined(SURFACE_RADIO)
  #define TR_MENUFLIGHTMODES           "FLYVE TILSTANDE"
  #define TR_MENUFLIGHTMODE            "FLYVE TILSTAND"
#else
  #define TR_MENUFLIGHTMODES           "FLYVE TILSTANDE"
  #define TR_MENUFLIGHTMODE            "FLYVE TILSTAND"
#endif
#define TR_MENUHELISETUP               "HELI SETUP"

#define TR_MENUINPUTS                  "INDGANGE"
#define TR_MENULIMITS                  "UDGANGE"
#define TR_MENUCURVES                  "KURVER"
#define TR_MENUCURVE                   "KURVE"
#define TR_MENULOGICALSWITCH           "LOGISK FUNKTION"
#define TR_MENULOGICALSWITCHES         "LOGISKE FUNKTIONER"
#define TR_MENUCUSTOMFUNC              "SPECIALFUNKTIONER"
#define TR_MENUCUSTOMSCRIPTS           "TILPASSET SCRIPT"
#define TR_MENUTELEMETRY               "TELEMETRI"
#define TR_MENUSTAT                    "STAT"
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS1           "KANAL MONITOR 1-8"
#define TR_MONITOR_CHANNELS2           "KANAL MONITOR 9-16"
#define TR_MONITOR_CHANNELS3           "KANAL MONITOR 17-24"
#define TR_MONITOR_CHANNELS4           "KANAL MONITOR 25-32"
#define TR_MONITOR_SWITCHES            "LOGISK FUNKTION MONITOR"
#define TR_MONITOR_OUTPUT_DESC         "Udgange"
#define TR_MONITOR_MIXER_DESC          "Mix"
#define TR_RECEIVER_NUM                TR("RxNr", "Rx modtager Nr.")
#define TR_RECEIVER                    "Modtager"
#define TR_MULTI_RFTUNE                TR("Freq tune", "RF Frekv. fin tune")
#define TR_MULTI_RFPOWER               "RF strøm"
#define TR_MULTI_WBUS                  "Udgang"
#define TR_MULTI_TELEMETRY             "Telemetri"
#define TR_MULTI_VIDFREQ               TR("Vid. frekv.", "Video frekvens")
#define TR_RF_POWER                    "RF Strøm"
#define TR_MULTI_FIXEDID               TR("LåstID", "Låst ID")
#define TR_MULTI_OPTION                TR("Tilvalg", "Tilvalg værdi")
#define TR_MULTI_AUTOBIND              TR("Tilslut ka.", "Tilslut kanal")
#define TR_DISABLE_CH_MAP              TR("% ka. kort", "Deaktiver kanal kort")
#define TR_DISABLE_TELEM               TR("% Telem.", "Deaktiver telemetri")
#define TR_MULTI_LOWPOWER              TR("Lav strøm", "Lav strøm tilstand")
#define TR_MULTI_LNA_DISABLE           "LNA deaktiver"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("TIL", "Aktiveret")
#define TR_DISABLE_INTERNAL            TR("Deaktiver int.", "Deaktiver intern RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("!seriel tilstand", "Ikke i seriel tilstand")
#define TR_MODULE_NO_INPUT             TR("Ingen indgang", "Ingen seriel indgang")
#define TR_MODULE_NO_TELEMETRY         TR3("Uden telemetri", "Uden MULTI_TELEMETRI", "Ingen MULTI_TELEMETRI opdaget")
#define TR_MODULE_WAITFORBIND          "Tilslut for at indlæse protokol"
#define TR_MODULE_BINDING              TR("Tilslut..","Tilslutter")
#define TR_MODULE_UPGRADE_ALERT        TR3("Opg. krævet", "Modul opdatering krævet", "Module\nopdatering krævet")
#define TR_MODULE_UPGRADE              TR("Opg. anbefalet", "Modul opdatering anbefalet")
#define TR_REBIND                      "Tilslutning påkrævet"
#define TR_REG_OK                      "Registration ok"
#define TR_BIND_OK                     "Tilslut lykkes"
#define TR_BINDING_CH1_8_TELEM_ON      "KA1-8 Telem TIL"
#define TR_BINDING_CH1_8_TELEM_OFF     "KA1-8 Telem FRA"
#define TR_BINDING_CH9_16_TELEM_ON     "KA9-16 Telem TIL"
#define TR_BINDING_CH9_16_TELEM_OFF    "KA9-16 Telem FRA"
#define TR_PROTOCOL_INVALID            TR("Prot. ugyldig", "Protocol ugyldig")
#define TR_MODULE_STATUS               TR("Status", "Modul status")
#define TR_MODULE_SYNC                 TR("Synk", "Proto Synk status")
#define TR_MULTI_SERVOFREQ             TR("Servo hast", "Servo opdaterings hastighed")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("RF kanal", "Vælg RF kanal")
#define TR_AFHDS3_RX_FREQ              TR("RX frekv.", "RX frekvens")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetri")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Akt. strøm", "Aktuel strøm")
#define TR_AFHDS3_POWER_SOURCE         TR("Strøm", "Strøm kilde")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Brug FlySky RSSI værdi uden reskalering")
#define TR_GPS_COORDS_FORMAT           TR("GPS kordi.", "Kordinat format")
#define TR_VARIO                       TR("Vario", "Variometer")
#define TR_PITCH_AT_ZERO               "Højde nul"
#define TR_PITCH_AT_MAX                "Højde max"
#define TR_REPEAT_AT_ZERO              "Gentag nul"
#define TR_BATT_CALIB                  TR("Bat. kalib", "Batteri kalibering")
#define TR_CURRENT_CALIB               "Aktuel kalib"
#define TR_VOLTAGE                     TR("Spænding", "Spænding kilde")
#define TR_SELECT_MODEL                "Vælg model"
#define TR_MANAGE_MODELS               "Vælg Model"
#define TR_MODELS                      "Modeller"
#define TR_SELECT_MODE                 "Vælg tilstand"
#define TR_CREATE_MODEL                "Opret model"
#define TR_FAVORITE_LABEL              "Favoritter"
#define TR_MODELS_MOVED                "Ubrugte modeller flyttet til"
#define TR_NEW_MODEL                   "Ny model"
#define TR_INVALID_MODEL               "Ingen billede"
#define TR_EDIT_LABELS                 "Ret type"
#define TR_LABEL_MODEL                 "Navngiv model"
#define TR_MOVE_UP                     "Ingen billede"
#define TR_MOVE_DOWN                   "Flyt ned"
#define TR_ENTER_LABEL                 "Vælg type"
#define TR_LABEL                       "Type"
#define TR_LABELS                      "Typer"
#define TR_CURRENT_MODEL               "Aktuel"
#define TR_ACTIVE                      "Aktiv"
#define TR_NEW                         "Ny"
#define TR_NEW_LABEL                   "Ny type"
#define TR_RENAME_LABEL                "Omdøb type"
#define TR_DELETE_LABEL                "Slet type"
#define TR_DUPLICATE_MODEL             "Dupliker model"
#define TR_COPY_MODEL                  "Kopier model"
#define TR_MOVE_MODEL                  "Flyt model"
#define TR_BACKUP_MODEL                "Backup model"
#define TR_DELETE_MODEL                "Slet model"
#define TR_RESTORE_MODEL               "Genskab model"
#define TR_DELETE_ERROR                "Slet fejl"
#define TR_SDCARD_ERROR                TR("SD fejl", "SD kort fejl")
#define TR_SDCARD                      "SD kort"
#define TR_NO_FILES_ON_SD              "Ingen filer på SD!"
#define TR_NO_SDCARD                   "Ingen SD kort"
#define TR_WAITING_FOR_RX              "Venter på RX..."
#define TR_WAITING_FOR_TX              "Venter på TX..."
#define TR_WAITING_FOR_MODULE          TR("Venter modul", "Venter på modul...")
#define TR_NO_TOOLS                    "Ingen mulige værktøjer"
#define TR_NORMAL                      "Normal"
#define TR_NOT_INVERTED                "Ikke inv"
#define TR_NOT_CONNECTED               TR("!Forbundet", "Ikke forbundet");
#define TR_CONNECTED                   "Forbundet"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16KA uden telem.", "16KA uden telemetri")
#define TR_16CH_WITH_TELEMETRY         TR("16KA med telem.", "16KA med telemetri")
#define TR_8CH_WITH_TELEMETRY          TR(" 8KA med telem.", " 8KA med telemetri")
#define TR_EXT_ANTENNA                 "Ext. antenna"
#define TR_PIN                         "Pind"
#define TR_UPDATE_RX_OPTIONS           "Opdater RX tilvalg?"
#define TR_UPDATE_TX_OPTIONS           "Opdater TX tilvalg?"
#define TR_MODULES_RX_VERSION          "Moduler / RX version"
#define TR_SHOW_MIXER_MONITORS         "Vis mixer monitor"
#define TR_MENU_MODULES_RX_VERSION     "MODULER / RX VERSION"
#define TR_MENU_FIRM_OPTIONS           "FIRMWARE TILVALG"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "pinde/drejekontakter/skydere"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM pinde/drejekontakter/skydere"
#define TR_RF_PROTOCOL                 "RF protokol"
#define TR_MODULE_OPTIONS              "Modul tilvalg"
#define TR_POWER                       "Strøm"
#define TR_NO_TX_OPTIONS               "Ingen TX tilvalg"
#define TR_RTC_BATT                    "RTC Bat."
#define TR_POWER_METER_EXT             "Strøm meter (EXT)"
#define TR_POWER_METER_INT             "Strøm meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spectrum (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Spectrum (INT)"
#define TR_SDCARD_FULL                 "SD kort fuldt"
#if defined(COLORLCD)
  #define TR_SDCARD_FULL_EXT           TR_SDCARD_FULL "\nLog & skærmklip deaktiveret"
#else
  #define TR_SDCARD_FULL_EXT           TR_SDCARD_FULL "\036Log & skærmklip" LCDW_128_LINEBREAK "deaktiveret"
#endif
#define TR_NEEDS_FILE                  "MANGLER FIL"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Inkompatibel"
#define TR_WARNING                     "ADVARSEL"
#define TR_STORAGE_WARNING             "LAGER"
#define TR_THROTTLE_UPPERCASE          "GAS"
#define TR_ALARMSWARN                  "ALARMER"
#define TR_SWITCHWARN                  TR("KONTAKT", "KONTROL")
#define TR_FAILSAFEWARN                "FEJLSIKR."
#define TR_TEST_WARNING                TR("TESTER", "TEST BYG")
#define TR_TEST_NOTSAFE                "Brug kun til test"
#define TR_WRONG_SDCARDVERSION         TR("Forventet ver: ", "Forventet version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC batteri lav"
#define TR_WARN_MULTI_LOWPOWER         "Lav strøm tilstand"
#define TR_BATTERY                     "BATTERI"
#define TR_WRONG_PCBREV                "Forkert PCB opdaget"
#define TR_EMERGENCY_MODE              "NØD TILSTAND"
#define TR_NO_FAILSAFE                 "Fejlsikring ikke defineret"
#define TR_KEYSTUCK                    "Tast hænger"
#define TR_VOLUME                      "Lydstyrke"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Skarphed"
#define TR_CPU_TEMP                    "CPU temp."
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "MB Temp."
#define TR_TTL_WARNING                 "Advarsel: Spænding på TX/RX poler må ikke overstige 3,3V!"
#define TR_FUNC                        "Funktion"
#define TR_V1                          TR("V1", "Værdi 1")
#define TR_V2                          TR("V2", "Værdi 2")
#define TR_DURATION                    "Varighed"
#define TR_DELAY                       "Forsinkelse"
#define TR_SD_CARD                     "SD CARD"
#define TR_SDHC_CARD                   "SD-HC CARD"
#define TR_NO_SOUNDS_ON_SD             "Ingen lyde på SD"
#define TR_NO_MODELS_ON_SD             "Ingen modeller på SD"
#define TR_NO_BITMAPS_ON_SD            "Ingen bitmapper på SD"
#define TR_NO_SCRIPTS_ON_SD            "Ingen scripts på SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntax fejl", "Script syntax fejl")
#define TR_SCRIPT_PANIC                "Script panik"
#define TR_SCRIPT_KILLED               "Script dræbt"
#define TR_SCRIPT_ERROR                "Ukendt fejl"
#define TR_PLAY_FILE                   "Spil"
#define TR_DELETE_FILE                 "Slet"
#define TR_COPY_FILE                   "Kopi"
#define TR_RENAME_FILE                 "Omdøb"
#define TR_ASSIGN_BITMAP               "Tildel billede"
#define TR_ASSIGN_SPLASH               "Start billde"
#define TR_EXECUTE_FILE                "Udfør"
#define TR_REMOVED                     " slettet"
#define TR_SD_INFO                     "Information"
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "HARDWARE"
#define TR_FORMATTING                  "Formattering..."
#define TR_TEMP_CALIB                  "Temp. kalib"
#define TR_TIME                        "Tid"
#define TR_MAXBAUDRATE                 "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE                 "Måle tilstand"
#define TR_SAMPLE_MODES                "Normal","EnBit"
#define TR_LOADING                     "Indlæser..."
#define TR_DELETE_THEME                "Slet tema?"
#define TR_SAVE_THEME                  "Gem tema?"
#define TR_EDIT_COLOR                  "Rediger farve"
#define TR_NO_THEME_IMAGE              "Ingen billede for tema"
#define TR_BACKLIGHT_TIMER             TR("Inaktivitet", "Ved inaktivitet dæmp efter")

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Hurtigvalg af model"
  #define TR_LABELS_SELECT             "Label valg"
  #define TR_LABELS_MATCH              "Label match"
  #define TR_FAV_MATCH                 "Favorites matching"
  #define TR_LABELS_SELECT_MODE        "Multi valg","Enkelt valg"
  #define TR_LABELS_MATCH_MODE         "Match alle","Match enhver"
  #define TR_FAV_MATCH_MODE            "Skal have match","Kan have match"
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "VÆLG EN SKABELON MAPPE:"
#define TR_SELECT_TEMPLATE             "VÆLG EN SKABELON til MODEL:"
#define TR_NO_TEMPLATES                "Ingen model skabeloner fundet i mappe"
#define TR_SAVE_TEMPLATE               "Gem som skabelon"
#define TR_BLANK_MODEL                 "Tom model"
#define TR_BLANK_MODEL_INFO            "Opret en tom model"
#define TR_FILE_EXISTS                 "FIL FINDES ALLEREDE"
#define TR_ASK_OVERWRITE               "Vil du overskrive?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Opdag/søg"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Dist adr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Lokal adr"
#define TR_BLUETOOTH_PIN_CODE          "PIN kode"
#define TR_BLUETOOTH_NODEVICES         "Ingen enheder fundet"
#define TR_BLUETOOTH_SCANNING          "Søger..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
  #define TR_BLUETOOTH_MODES           "---","Aktiveret"
#else
  #define TR_BLUETOOTH_MODES           "---","Telemetri","Træner"
#endif

#define TR_SD_INFO_TITLE               "SD INFO"
#define TR_SD_SPEED                    "Hastighed:"
#define TR_SD_SECTORS                  "Sektorer:"
#define TR_SD_SIZE                     "Størrelse:"
#define TR_TYPE                        "Type"
#define TR_GLOBAL_VARS                 "Globale variable"
#define TR_GVARS                       "GV"
#define TR_GLOBAL_VAR                  "Globale variable"
#define TR_MENU_GLOBAL_VARS            "GLOBALE VARIABLE"
#define TR_OWN                         "Egen"
#define TR_DATE                        "Dato"
#define TR_MONTHS                      { "Jan", "Feb", "Mar", "Apr", "Maj", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dec" }
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("DrejeK. tilst.","Drejeknap tilstand")
#define TR_CHANNELS_MONITOR            "KANAL MONITOR"
#define TR_MIXERS_MONITOR              "MIX MONITOR"
#define TR_PATH_TOO_LONG               "Sti for lang"
#define TR_VIEW_TEXT                   "Vis tekst"
#define TR_FLASH_BOOTLOADER            "Installer bootloader"
#define TR_FLASH_DEVICE                TR("Inst. enhed","Installer enhed")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Inst. S.Port", "Installer S.Port enhed")
#define TR_FLASH_RECEIVER_OTA          "Installer modtager OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Installer RX via ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Installer RX via int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Installer FC via ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Installer FC via int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Inst. BT module", "Installer Bluetooth modul")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Installer strøm styring"
#define TR_DEVICE_NO_RESPONSE          TR("Enhed svarer ikke", "Enhed svarer ikke")
#define TR_DEVICE_FILE_ERROR           TR("Enhed fil prob.", "Enhed fil prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Enhed data afvist", "Enhed data afvist")
#define TR_DEVICE_WRONG_REQUEST        TR("Enhed adgang problem", "Enhed adgang problem")
#define TR_DEVICE_FILE_REJECTED        TR("Enhed fil afvist", "Enhed fil afvist")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Enhed file sig.", "Enhed file sig.")
#define TR_CURRENT_VERSION             TR("Aktuel ver: ", "Aktuel version: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Inst. int. module", "Installer intern modul")
#define TR_FLASH_INTERNAL_MULTI        TR("Inst. int. multi", "Installer intern multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Inst. ext. module", "Installer ekstern modul")
#define TR_FLASH_EXTERNAL_MULTI        TR("Inst. ext. multi", "Installer ekstern multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Inst. ext. ELRS", "Installer ekstern ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("FW opdatering fejl", "Firmware opdaterings fejl")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Installation successful"
#define TR_WRITING                     "Skriver..."
#define TR_CONFIRM_FORMAT              "Bekræft formattering?"
#define TR_INTERNALRF                  "Intern RF"
#define TR_INTERNAL_MODULE             TR("Int. module", "Intern modul")
#define TR_EXTERNAL_MODULE             TR("Ext. module", "Eksternt modul")
#define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX kræver opgradering"
#define TR_TELEMETRY_DISABLED          "Telem. deaktiveret"
#define TR_MORE_OPTIONS_AVAILABLE      "Flere tilvalg mulige"
#define TR_NO_MODULE_INFORMATION       "Ingen modul information"
#define TR_EXTERNALRF                  "Ekstern RF"
#define TR_FAILSAFE                    TR("Fejlsikring", "Fejlsikring tilst")
#define TR_FAILSAFESET                 "FEJLSIKRING INDSTILLING"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Ejer ID"
#define TR_HOLD                        "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                        "Ingen"
#define TR_NONE_UPPERCASE              "INGEN"
#define TR_MENUSENSOR                  "SENSOR"
#define TR_POWERMETER_PEAK             "Spids"
#define TR_POWERMETER_POWER            "Strøm"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Freq."
#define TR_MENUTOOLS                   "VÆRKTØJ"
#define TR_TURN_OFF_RECEIVER           "Sluk modtager"
#define TR_STOPPING                    "Standser..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSE"
#define TR_MENU_POWER_METER            "STRØM MÅLER"
#define TR_SENSOR                      "SENSOR"
#define TR_COUNTRY_CODE                "Lande kode"
#define TR_USBMODE                     "USB tilstand"
#define TR_JACK_MODE                   "Jack tilstand"
#define TR_VOICE_LANGUAGE              "Stemme sprog"
#define TR_UNITS_SYSTEM                "Enheder"
#define TR_UNITS_PPM                   "PPM enheder"
#define TR_EDIT                        "Rediger"
#define TR_INSERT_BEFORE               "Insæt før"
#define TR_INSERT_AFTER                "Insæt efter"
#define TR_COPY                        "Kopi"
#define TR_MOVE                        "Flyt"
#define TR_PASTE                       "Sæt ind"
#define TR_PASTE_AFTER                 "Sæt ind efter"
#define TR_PASTE_BEFORE                "Sæt ind før"
#define TR_DELETE                      "Slet"
#define TR_INSERT                      "Indsæt"
#define TR_RESET_FLIGHT                "Nulstil flyvning"
#define TR_RESET_TIMER1                "Nulstil tidtag 1"
#define TR_RESET_TIMER2                "Nulstil tidtag 2"
#define TR_RESET_TIMER3                "Nulstil tidtag 3"
#define TR_RESET_TELEMETRY             "Nulstil telemetri"
#define TR_STATISTICS                  "Statistik"
#define TR_ABOUT_US                    "Om"
#define TR_USB_JOYSTICK                "USB joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB lager (SD)"
#define TR_USB_SERIAL                  "USB data (VCP)"
#define TR_SETUP_SCREENS               "Setup skærm"
#define TR_MONITOR_SCREENS             "Monitor"
#define TR_AND_SWITCH                  "OG kontakt"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Kalibreret analog"
#define TR_ANADIAGS_FILTRAWDEV         "Filteret rå analog med afvigelser"
#define TR_ANADIAGS_UNFILTRAW          "Ufilteret rå analog"
#define TR_ANADIAGS_MINMAX             "Min., max. og interval"
#define TR_ANADIAGS_MOVE               "Flyt analoge til deres yderpunkter!"
#define TR_SPEAKER                     "Højttaler"
#define TR_BUZZER                      "Biper"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Tils", "Tilslut"))
#define TR_MODULE_UNBIND               BUTTON("Afkobl")
#define TR_POWERMETER_ATTN_NEEDED     "Attestering nødvendig"
#define TR_PXX2_SELECT_RX              "Vælg RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "Vælg enhed"
#define TR_DISCOVER                    "Opdag"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Venter..."
#define TR_RECEIVER_DELETE             "Slet modtager?"
#define TR_RECEIVER_RESET              "Nulstil modtager?"
#define TR_SHARE                       "Del"
#define TR_BIND                        "Tilslut"
#define TR_REGISTER                    TR("Reg", "Registrer")
#define TR_MODULE_RANGE                BUTTON(TR("Ræk", "Rækkevidde"))
#define TR_RANGE_TEST                  "Test af rækkevidde"
#define TR_RECEIVER_OPTIONS            TR("MODT. VALG", "MODTAGER VALG")
#define TR_RESET_BTN                   BUTTON("Nulstil")
#define TR_DEBUG                       "Fejlfind"
#define TR_KEYS_BTN                    BUTTON("Taster")
#define TR_ANALOGS_BTN                 BUTTON(TR("Analo", "Analoge"))
#define TR_FS_BTN                      BUTTON(TR("Tilpasset sw", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Tryk skærm ikke fundet"
#define TR_TOUCH_EXIT                  "Tryk på skærm for at forlade"
#define TR_SET                         BUTTON("Sæt")
#define TR_TRAINER                     "Træner"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM              "TX antenne problem!"
#define TR_MODELIDUSED                 "ID brugt i:"
#define TR_MODELIDUNIQUE               "ID er entydig"
#define TR_MODULE                      "Modul"
#define TR_RX_NAME                     "Rx Navn"
#define TR_TELEMETRY_TYPE              TR("Type", "Telemetri type")
#define TR_TELEMETRY_SENSORS           "Sensorer"
#define TR_VALUE                       "Værdi"
#define TR_PERIOD                      "Periode"
#define TR_INTERVAL                    "Interval"
#define TR_REPEAT                      "Gentag"
#define TR_ENABLE                      "Aktiver"
#define TR_DISABLE                     "Deaktiver"
#define TR_TOPLCDTIMER                 "Top LCD ur"
#define TR_UNIT                        "Enhed"
#define TR_TELEMETRY_NEWSENSOR         "Tilføj ny"
#define TR_CHANNELRANGE                TR("Kan. rækkev", "Kanal rækkevidde")
#define TR_ANTENNACONFIRM1             "EXT. ANTENNE"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "Intern","Spørg","Per model","Intern + Ekstern"
#else
  #define TR_ANTENNA_MODES             "Intern","Spørg","Per model","Ekstern"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Brug int. antenne", "Brug intern antenne")
#define TR_USE_EXTERNAL_ANTENNA        TR("Brug ext. antenne", "Brug ekstern antenne")
#define TR_ANTENNACONFIRM2             TR("Check antenne", "Kontroller at antenne er installeret!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Kræver FLEX non"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Kræver FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Kræver EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "certificeret firmware"
#define TR_LOWALARM                    "Lav alarm"
#define TR_CRITICALALARM               "Kritisk alarm"
#define TR_DISABLE_ALARM               TR("Afkobl alarmer", "Afkobl telemetri alarmer")
#define TR_POPUP                       "Popup"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                TR ("Forudindstil", "Forudind.")
#define TR_PRESET                      "Forudindstil"
#define TR_MIRROR                      "Spejle"
#define TR_CLEAR                       "Slet"
#define TR_RESET                       "Nulstil"
#define TR_RESET_SUBMENU               "Nulstil..."
#define TR_COUNT                       "Tæller"
#define TR_PT                          "pt"
#define TR_PTS                         " punkter"
#define TR_SMOOTH                      "Jævn"
#define TR_COPY_STICKS_TO_OFS          TR("Kopi pind->subtrim", "Kopier pinde til subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Kopi min/max ->alle",  "Kopier min/max/center til alle udgange")
#define TR_COPY_TRIMS_TO_OFS           TR("Kopi trim->subtrim", "Kopier trim til subtrim")
#define TR_INCDEC                      "Øg/sænk"
#define TR_GLOBALVAR                   TR("Global var", "Global variabel")
#define TR_MIXSOURCE                   "Kilde (%)"
#define TR_MIXSOURCERAW                "Kilde (værdi)"
#define TR_CONSTANT                    "Konstant"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "Fra","Til","Auto"
#define TR_PREFLIGHT                   "Fly checkliste"
#define TR_CHECKLIST                   TR("Checkliste", "Vis checkliste")
#define TR_CHECKLIST_INTERACTIVE       TR3("C-Interact", "Interact. checklist", "Interactive checklist")
#define TR_AUX_SERIAL_MODE             "Seriel port"
#define TR_AUX2_SERIAL_MODE            "Seriel port 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port strøm"
#define TR_SCRIPT                      "Script"
#define TR_INPUTS                      "Indgange"
#define TR_OUTPUTS                     "Udgange"
#define STR_EEBACKUP                   "EEPROM backup"
#define STR_FACTORYRESET               "Nulstil til fabrik indstillinger"
#define TR_CONFIRMRESET                TR("Slet ALLE", "Slet ALLE modeller og indstillinger?")
#define TR_TOO_MANY_LUA_SCRIPTS        "For mange Lua scripts!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Strøm"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","TIL"
#define TR_NO_TELEMETRY_SCREENS        "Ingen telemetri skærme"
#define TR_TOUCH_PANEL                 "Tryk panel:"
#define TR_FILE_SIZE                   "Fil størrelse"
#define TR_FILE_OPEN                   "Åbn alligevel?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Navn"
#define TR_PHASES_HEADERS_SW           "Kontakt"
#define TR_PHASES_HEADERS_RUD_TRIM     "Sideror Trim"
#define TR_PHASES_HEADERS_ELE_TRIM     "Højderor Trim"
#define TR_PHASES_HEADERS_THT_TRIM     "Gas Trim"
#define TR_PHASES_HEADERS_AIL_TRIM     "Krængror Trim"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Tone ind"
#define TR_PHASES_HEADERS_FAD_OUT      "Tone ud"

#define TR_LIMITS_HEADERS_NAME         "Navn"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Retning"
#define TR_LIMITS_HEADERS_CURVE        "Kurve"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Center"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Subtrim tilstand"
#define TR_INVERTED                    "Invers"

#define TR_LSW_DESCRIPTIONS            { "Sammenlign type eller funktion", "Første variabel", "Anden variabel eller konstant", "Anden variabel eller konstant", "Endnu en betingelse for linje", "Minimum TIL varighed for logisk funktion", "Minimum SAND varighed for betingelse bliver sand" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "Første kanal"
  #define TR_FILL_BACKGROUND           "Udfyld baggrund?"
  #define TR_BG_COLOR                  "BG farve"
  #define TR_SLIDERS_TRIMS             "Skyder+Trim"
  #define TR_SLIDERS                   "Skydere"
  #define TR_FLIGHT_MODE               "Flyve tilstand"
  #define TR_INVALID_FILE              "Ugyldig fil"
  #define TR_TIMER_SOURCE              "Tidtagning kilde"
  #define TR_SIZE                      "Størrelse"
  #define TR_SHADOW                    "Skygge"
  #define TR_ALIGNMENT                 "Justering"
  #define TR_ALIGN_LABEL               "Justere navn"
  #define TR_ALIGN_VALUE               "Justere værdi"
  #define TR_ALIGN_OPTS                { "Venstre", "Center", "Højre" }
  #define TR_TEXT                      "Tekst"
  #define TR_COLOR                     "Farve"
  #define TR_MAIN_VIEW_X               "Hoved visning XX"
  #define TR_PANEL1_BACKGROUND         "Panel1 baggrund"
  #define TR_PANEL2_BACKGROUND         "Panel2 baggrund"
  #define TR_WIDGET_GAUGE              "Måling"
  #define TR_WIDGET_MODELBMP           "Modeller"
  #define TR_WIDGET_OUTPUTS            "Udgange"
  #define TR_WIDGET_TEXT               "Tekst"
  #define TR_WIDGET_TIMER              "Tider"
  #define TR_WIDGET_VALUE              "Værdi"
#endif

// Bootloader common - ASCII characters only
#define TR_BL_USB_CONNECTED           "USB forbundet"
#define TR_BL_USB_PLUGIN              "eller brug USB kabel"
#define TR_BL_USB_MASS_STORE          "for USB disk"
#define TR_BL_USB_PLUGIN_MASS_STORE   "eller brug USB kabel for USB disk"
#define TR_BL_WRITE_FW                "Installer firmware"
#define TR_BL_FORK                    "Fork:"
#define TR_BL_VERSION                 "Version:"
#define TR_BL_RADIO                   "Radio:"
#define TR_BL_EXIT                    "Forlad"
#define TR_BL_DIR_MISSING             " Katalog mangler"
#define TR_BL_DIR_EMPTY               " Katalog er tomt"
#define TR_BL_WRITING_FW              "Installerer..."
#define TR_BL_WRITING_COMPL           "Installation slut"

#if LCD_W >= 480
  #define TR_BL_INVALID_FIRMWARE       "Ikke en installationsfil!"
#elif LCD_W >= 212
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN_MASS_STORE
  #define TR_BL_HOLD_ENTER_TO_START    "\012[ENT] for at starte installation"
  #define TR_BL_INVALID_FIRMWARE       "\011Ikke en installationsfil!         "
  #define TR_BL_INVALID_EEPROM         "\011Ikke en EEPROM fil!               "
#else
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
  #define TR_BL_HOLD_ENTER_TO_START    "\006[ENT] for at starte"
  #define TR_BL_INVALID_FIRMWARE       "\004Ikke en installationsfil!    "
  #define TR_BL_INVALID_EEPROM         "\004Ikke en EEPROM fil!          "
#endif

#if defined(PCBTARANIS)
   // Bootloader Taranis specific - ASCII characters only
  #define TR_BL_RESTORE_EEPROM        "Genskab EEPROM"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Tryk power knap."
    #define TR_BL_FLASH_EXIT          "Forlad installation tilstand."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - ASCII characters only
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_SELECT_KEY            "[ENT] for at bruge fil"
  #define TR_BL_FLASH_KEY             "[ENT] længe, for at starte"
  #define TR_BL_ERASE_KEY             "Hold [ENT] long to erase"
  #define TR_BL_EXIT_KEY              "[RTN] for at forlade"
#elif defined(PCBNV14)
   // Bootloader NV14 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB adgang"
  #define TR_BL_CURRENT_FW            "Firmware version:"
  #define TR_BL_SELECT_KEY            "[R TRIM] for at bruge fil"
  #define TR_BL_FLASH_KEY             "[R TRIM] længe, for at starte"
  #define TR_BL_EXIT_KEY              "[L TRIM] for at forlade"
  #define TR_BL_ENABLE                "Aktiver"
  #define TR_BL_DISABLE               "Deaktiver"
#elif defined(PCBPL18)
   // Bootloader PL18/NB4+ specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB adgang"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_ERASE_INT_FLASH       "Slet intern flash lager"
  #define TR_BL_ERASE_FLASH           "Slet flash lager"
  #define TR_BL_ERASE_FLASH_MSG       "Dette kan vare op til 200s"
  #if defined(RADIO_NB4P)
    #define TR_BL_SELECT_KEY            " [SW1A] for at bruge fil"
    #define TR_BL_FLASH_KEY             " Hold [SW1A] længe, for brænding"
    #define TR_BL_ERASE_KEY             " Hold [SW1A] længe, for sletning"
    #define TR_BL_EXIT_KEY              " [SW1B] for at forlade"
  #else
    #define TR_BL_SELECT_KEY            " [TR4 Dn] for at bruge fil"
    #define TR_BL_FLASH_KEY             " Hold [TR4 Dn] længe, for brænding"
    #define TR_BL_ERASE_KEY             " Hold [TR4 Dn] længe, for sletning"
    #define TR_BL_EXIT_KEY              " [TR4 Up] for at forlade"
#endif


  // Bootloader PL18/NB4+ specific - Ascii only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
#endif

// About screen
#define TR_ABOUTUS                     TR(" OM ", "OM")

#define TR_CHR_HOUR                    't'
#define TR_CHR_INPUT                   'K'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Bip lydstyrke"
#define TR_WAV_VOLUME                  "Wav lydstyrke"
#define TR_BG_VOLUME                   TR("Bg lydsty.", "Baggrund lydstyrke")

#define TR_TOP_BAR                     "Top bjælke"
#define TR_FLASH_ERASE                 "Flash slettes..."
#define TR_FLASH_WRITE                 "Flash skrives..."
#define TR_OTA_UPDATE                  "OTA opdatering..."
#define TR_MODULE_RESET                "Nulstil modul..."
#define TR_UNKNOWN_RX                  "Ukendt RX"
#define TR_UNSUPPORTED_RX              "Ikke understøttet RX"
#define TR_OTA_UPDATE_ERROR            "OTA opdaterings fejl"
#define TR_DEVICE_RESET                "Nulstil enhed..."
#define TR_ALTITUDE                    "Højde"
#define TR_SCALE                       "Skala"
#define TR_VIEW_CHANNELS               "Vis kanaler"
#define TR_VIEW_NOTES                  "Vis noter"
#define TR_MODEL_SELECT                "Vælg model"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Præcision"
#define TR_RATIO                       "Forhold"
#define TR_FORMULA                     "Formel"
#define TR_CELLINDEX                   "Celle indeks"
#define TR_LOGS                        "Log"
#define TR_OPTIONS                     "Tilvalg"
#define TR_FIRMWARE_OPTIONS            "Firmware tilvalg"

#define TR_ALTSENSOR                   "Højde sensor"
#define TR_CELLSENSOR                  "Cell sensor"
#define TR_GPSSENSOR                   "GPS sensor"
#define TR_CURRENTSENSOR               "Sensor"
#define TR_AUTOOFFSET                  "Auto offset"
#define TR_ONLYPOSITIVE                "Positive"
#define TR_FILTER                      "Filter"
#define TR_TELEMETRYFULL               TR("Alle slots fulde!", "Alle telemetri slots fulde!")
#define TR_IGNORE_INSTANCE             TR("Ingen inst.", "Ignorer instans")
#define TR_SHOW_INSTANCE_ID            "Show instance ID"
#define TR_DISCOVER_SENSORS            "Søg efter nye"
#define TR_STOP_DISCOVER_SENSORS       "Stop"
#define TR_DELETE_ALL_SENSORS          "Slet alle"
#define TR_CONFIRMDELETE               "Virkeligt " LCDW_128_LINEBREAK "slet alle ?"
#define TR_SELECT_WIDGET               "Vælg widget"
#define TR_WIDGET_FULLSCREEN           "Fuld skærm"
#define TR_REMOVE_WIDGET               "Slet widget"
#define TR_WIDGET_SETTINGS             "Widget indstilinger"
#define TR_REMOVE_SCREEN               "Slet skærm"

#if !PORTRAIT_LCD
  #define TR_SETUP_WIDGETS             "Opsæt widget"
#else
  #define TR_SETUP_WIDGETS             "Widget"
#endif

#define TR_USER_INTERFACE              "Bruger interface"
#define TR_THEME                       "Tema"
#define TR_SETUP                       "Opsætning"
#define TR_LAYOUT                      "Layout"
#define TR_ADD_MAIN_VIEW               "Tilføj hoved billede"
#define TR_TEXT_COLOR                  "Tekst farve"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Indgange"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua script"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Pinde"
#define TR_MENU_POTS                   STR_CHAR_POT "Drejekontakt"
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "Cyklisk"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Trim"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Kontakter"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Logiske funktioner"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Træner"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Kanaler"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "Globale Var"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Telemetri"
#define TR_MENU_DISPLAY                "DISPLAY"
#define TR_MENU_OTHER                  "Andet"
#define TR_MENU_INVERT                 "Invers"
#define TR_AUDIO_MUTE                  TR("Audio fra","Audio fra, hvis der ikke gives lyd")
#define TR_JITTER_FILTER               "ADC filter"
#define TR_DEAD_ZONE                   "Dødt område"
#define TR_RTC_CHECK                   TR("Check RTC", "Check RTC spænding")
#define TR_AUTH_FAILURE                "Godkendelse fejlet"
#define TR_RACING_MODE                 "Racing tilstand"

#undef  STR_SENSOR_THROTTLE
#define STR_SENSOR_THROTTLE            "Gas"
#undef  STR_SENSOR_SPEED
#define STR_SENSOR_SPEED               "Fart"

#define TR_USE_THEME_COLOR             "Brug tema farve"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS   "Tilføj alle trim til subtrim"

#if !PORTRAIT_LCD
  #define TR_OPEN_CHANNEL_MONITORS     "Åbn kanal monitor"
#else
  #define TR_OPEN_CHANNEL_MONITORS     "Åbn kanal mon."
#endif

#define TR_DUPLICATE                   "Dupliker"
#define TR_ACTIVATE                    "Sæt aktiv"
#define TR_RED                         "Rød"
#define TR_BLUE                        "Blå"
#define TR_GREEN                       "Grøn"
#define TR_COLOR_PICKER                "Farve vælger"
#define TR_EDIT_THEME_DETAILS          "Ret tema detajler"
#define TR_THEME_COLOR_DEFAULT         "STANDARD"
#define TR_THEME_COLOR_PRIMARY1        "PRIMÆR 1"
#define TR_THEME_COLOR_PRIMARY2        "PRIMÆR 2"
#define TR_THEME_COLOR_PRIMARY3        "PRIMÆR 3"
#define TR_THEME_COLOR_SECONDARY1      "SEKUNDÆR 1"
#define TR_THEME_COLOR_SECONDARY2      "SEKUNDÆR 2"
#define TR_THEME_COLOR_SECONDARY3      "SEKUNDÆR 3"
#define TR_THEME_COLOR_FOCUS           "FOKUS"
#define TR_THEME_COLOR_EDIT            "REDIGER"
#define TR_THEME_COLOR_ACTIVE          "AKTIV"
#define TR_THEME_COLOR_WARNING         "ADVARSEL"
#define TR_THEME_COLOR_DISABLED        "DEAKTIV"
#define TR_THEME_COLOR_CUSTOM          "BRUGER STYRET"
#define TR_THEME_CHECKBOX              "Checkbox"
#define TR_THEME_ACTIVE                "Aktiv"
#define TR_THEME_REGULAR               "Almindelig"
#define TR_THEME_WARNING               "Advarsel"
#define TR_THEME_DISABLED              "Deaktiv"
#define TR_THEME_EDIT                  "Rediger"
#define TR_THEME_FOCUS                 "Fokus"
#define TR_AUTHOR                      "Forfatter"
#define TR_DESCRIPTION                 "Beskrivelse"
#define TR_SAVE                        "Gem"
#define TR_CANCEL                      "Fortryd"
#define TR_EDIT_THEME                  "RET TEMA"
#define TR_DETAILS                     "Detajler"
#define TR_THEME_EDITOR                "TEMAER"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL      "Vælg\nModel"
#define TR_MAIN_MENU_MANAGE_MODELS     "Vælg\nModel"
#define TR_MAIN_MENU_MODEL_NOTES       "Model\nNoter"
#define TR_MAIN_MENU_CHANNEL_MONITOR   "Kanal\nMonitor"
#define TR_MAIN_MENU_MODEL_SETTINGS    "Model\nIndstilling"
#define TR_MAIN_MENU_RADIO_SETTINGS    "Radio\nIndstilling"
#define TR_MAIN_MENU_SCREEN_SETTINGS   "Skærm\nIndstilling"
#define TR_MAIN_MENU_RESET_TELEMETRY   "Nulstil\nTelemetri"
#define TR_MAIN_MENU_STATISTICS        "Statistik"
#define TR_MAIN_MENU_ABOUT_EDGETX      "Om\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH               "Engelsk"
#define TR_VOICE_CHINESE               "Kinesisk"
#define TR_VOICE_CZECH                 "Tjekkisk"
#define TR_VOICE_DANISH                "Dansk"
#define TR_VOICE_DEUTSCH               "Tysk"
#define TR_VOICE_DUTCH                 "Hollandsk"
#define TR_VOICE_ESPANOL               "Spansk"
#define TR_VOICE_FRANCAIS              "Fransk"
#define TR_VOICE_HUNGARIAN             "Ungarsk"
#define TR_VOICE_ITALIANO              "Italiensk"
#define TR_VOICE_POLISH                "Polsk"
#define TR_VOICE_PORTUGUES             "Portugisisk"
#define TR_VOICE_RUSSIAN               "Russisk"
#define TR_VOICE_SLOVAK                "Slovakisk"
#define TR_VOICE_SWEDISH               "Svensk"
#define TR_VOICE_TAIWANESE             "Taiwanesisk"
#define TR_VOICE_JAPANESE              "Japansk"
#define TR_VOICE_HEBREW                "Hebraisk"
#define TR_VOICE_UKRAINIAN             "Ukrainisk"

#define TR_USBJOYSTICK_LABEL           "USB joystik"
#define TR_USBJOYSTICK_EXTMODE         "Tilstand"
#define TR_VUSBJOYSTICK_EXTMODE        "Klassisk","Advanceret"
#define TR_USBJOYSTICK_SETTINGS        "Kanal indstillinger"
#define TR_USBJOYSTICK_IF_MODE         TR("Intf. tils.","Interface tilstand")
#define TR_VUSBJOYSTICK_IF_MODE        "Joystik","Gamepad","MultiAkse"
#define TR_USBJOYSTICK_CH_MODE         "Tilstand"
#define TR_VUSBJOYSTICK_CH_MODE        "Ingen","Knap","Akse","Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Knap tilstand"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "Normal","Puls","SWEmu","Delta","Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Norm","Normal"),TR("Puls","Puls"),TR("SWEm","SWEmul"),TR("Delt","Delta"),TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Positioner"
#define TR_VUSBJOYSTICK_CH_SWPOS       "Skub","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Akse"
#define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","rotX","rotY","rotZ","Skyder","Drej","Hjul"
#define TR_USBJOYSTICK_CH_SIM          "Sim akse"
#define TR_VUSBJOYSTICK_CH_SIM         TR_STICK_NAMES3,TR_STICK_NAMES1,TR_STICK_NAMES0,TR_STICK_NAMES2,"Acc","Brems","Styr","Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Omvendt"
#define TR_USBJOYSTICK_CH_BTNNUM       "Knap #"
#define TR_USBJOYSTICK_BTN_COLLISION   "!Knap # kollision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Akse kollision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Cirk. klip", "Cikulær klip")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "Ingen","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Gem ændringer"

#define TR_DIGITAL_SERVO               "Servo333HZ"
#define TR_ANALOG_SERVO                "Servo 50HZ"
#define TR_SIGNAL_OUTPUT               "Signal uddata"
#define TR_SERIAL_BUS                  "Seriel bus"
#define TR_SYNC                        "Synk"

#define TR_ENABLED_FEATURES            "Aktiverede funktioner"
#define TR_RADIO_MENU_TABS             "Radio menu tab"
#define TR_MODEL_MENU_TABS             "Model menu tab"

#define TR_SELECT_MENU_ALL        "Alle"
#define TR_SELECT_MENU_CLR        "Slet"
#define TR_SELECT_MENU_INV        "Invers"

#define TR_SORT_ORDERS            "Navn A-Z","Navn Z-A","Sidst anvendt","Mest anvendt"
#define TR_SORT_MODELS_BY         "Sorter modeller"
#define TR_CREATE_NEW             "Opret"

#define TR_MIX_SLOW_PREC          TR("Træg præc", "Træg op/ned præcision")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "Der findes allerede et katalog med samme navn."

#define TR_DATE_TIME_WIDGET       "Dato & Klokke"
#define TR_RADIO_INFO_WIDGET      "Radio info"
#define TR_LOW_BATT_COLOR         "Batteri lavt"
#define TR_MID_BATT_COLOR         "Batteri medio"
#define TR_HIGH_BATT_COLOR        "Batteri højt"

#define TR_WIDGET_SIZE            "Widget størrelse"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
