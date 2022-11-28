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

#define TR_OFFON                       "FRA","TIL"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Stille","Alarm","NoKey","Alle"
#define TR_VBLMODE                     "FRA","Taster",TR("Ctrl","Controls"),"Begge","TIL"
#define TR_TRNMODE                     "FRA","+=",":="
#define TR_TRNCHN                      "KA1","KA2","KA3","KA4"
#define TR_AUX_SERIAL_MODES            "FRA","Telem spejlet","Telemetri ind","SBUS træner","Bluetooth","LUA","CLI","GPS","Debug","SpaceMouse"

#if LCD_W > LCD_H
  #define TR_SWTYPES                      "Ingen", "2 pos skift","2 position","3 position"
#else
  #define TR_SWTYPES                      "Ingen","Skift","2POS","3POS"
#endif

#define TR_POTTYPES                    "Ingen",TR("Drejekontakt med det","Drejekontakt"),TR("Multipos","Multipos kontakt"),"Drejekontakt"
#define TR_SLIDERTYPES                 "Ingen","Skyder"
#define TR_VPERSISTENT                 "FRA","Flyv","Manuel nulstil"
#define TR_COUNTRY_CODES               TR("US","Amerika"),TR("JP","Japan"),TR("EU","Europa")
#define TR_USBMODES                    "Spørg",TR("Joyst","Joystik"),TR("SDkord","SD Lager"),"Seriel"
#define TR_JACK_MODES                  "Spørg","Audio","Træner"
#define TR_TELEMETRY_PROTOCOLS         "FrSky S.PORT","FrSky D","FrSky D (kabel)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetri"

#define TR_SBUS_INVERSION_VALUES       "normal","ikke invers"
#define TR_MULTI_TELEMETRY_MODE        "Fra","Til","Fra+Aux","Til+Aux"
#define TR_MULTI_CUSTOM                "Tilpasset"
#define TR_VTRIMINC                    TR("Expo","Exponentiel"),TR("ExFin","Ekstra fin"),"Fin","Medium","Grov"
#define TR_VDISPLAYTRIMS               "Nej","Ændre","Ja"
#define TR_VBEEPCOUNTDOWN              "Stille","Bip","Stemme","Vibration"
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VVARIOCENTER                "Tone","Stille"
#define TR_CURVE_TYPES                 "Standard","Tilpasset"

#define TR_ADCFILTERVALUES             "Global","Fra","Til"

#if defined(PCBX10)
  #define TR_RETA123                   "S","H","G","K","1","3","2","4","5","6","7","V","H"
#elif defined(PCBHORUS)
  #define TR_RETA123                   "S","H","G","K","1","3","2","4","5","V","H"
#elif defined(PCBX9E)
  #define TR_RETA123                   "S","H","G","K","1","2","3","4","V","H","V","H"
#elif defined(PCBTARANIS)
  #define TR_RETA123                   "S","H","G","K","1","2","3","V","H"
#else
  #define TR_RETA123                   "S","H","G","K","1","2","3"
#endif

#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Læg til","Gange","Erstat"
#define TR_VMLTPX2                     "+=","*=",":="
#if defined(PCBHORUS)
  #define TR_VMIXTRIMS                 "FRA","TIL","Sid","Høj","Gas","Kræ","T5","T6"
#else
  #define TR_VMIXTRIMS                 "FRA","TIL","Sid","Høj","Gas","Kræ"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Tid"
  #define TR_CSWSTICKY                 "Sej"
  #define TR_CSWRANGE                  "Afstand"
  #define TR_CSWSTAY                   "Edge"
#else
  #define TR_CSWTIMER                  "Tid"
  #define TR_CSWSTICKY                 "Sej"
  #define TR_CSWRANGE                  "Afst"
  #define TR_CSWSTAY                   "Edge"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#if defined(VARIO)
  #define TR_VVARIO                    "Vario"
#else
  #define TR_VVARIO                    "[Vario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND                     "Spil lyd"
#else
  #define TR_SOUND                     "Bip"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC                    "Vibration"
#else
  #define TR_HAPTIC                    "[Vibration]"
#endif

#if defined(VOICE)
  #define TR_PLAY_TRACK                "Spil spor"
  #define TR_PLAY_BOTH                 "Spil begge"
  #define TR_PLAY_VALUE                TR("Spil Vær","Spil Værdi")
#else
  #define TR_PLAY_TRACK                "[Spil spo]"
  #define TR_PLAY_BOTH                 "[Spil beg]"
  #define TR_PLAY_VALUE                "[Spil vær]"
#endif

#define TR_SF_BG_MUSIC                 "BgMusik","BgMusik ||"

#if defined(SDCARD)
  #define TR_SDCLOGS                   "SD Log"
#else
  #define TR_SDCLOGS                   "[SD Log]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR               "Juster"
#else
  #define TR_ADJUST_GVAR               "[Juster GV]"
#endif

#if defined(LUA)
  #define TR_SF_PLAY_SCRIPT            "Lua Script"
#else
  #define TR_SF_PLAY_SCRIPT            "[Lua]"
#endif

#if defined(DEBUG)
  #define TR_SF_TEST                   ,"Test"
#else
  #define TR_SF_TEST
#endif

#if defined(OVERRIDE_CHANNEL_FUNCTION) && LCD_W >= 212
  #define TR_SF_SAFETY                 "Overskr."
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "Overs."
#else
  #define TR_SF_SAFETY                 "---"
#endif

#define TR_SF_SCREENSHOT               "Skærm klip"
#define TR_SF_RACING_MODE              "Ræs tilstand"
#define TR_SF_DISABLE_TOUCH            "Ikke berøringsaktiv"
#define TR_SF_SET_SCREEN               "Vælg hoved skærm"
#define TR_SF_RESERVE                  "[reserve]"

#define TR_VFSWFUNC                    TR_SF_SAFETY,"Træner","Inst. Trim","Nulstil","Sæt",TR_ADJUST_GVAR,"Volume","SetFejlsikr","AfstandCheck","ModuleBind",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,"Baggrundslys",TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH,TR_SF_SET_SCREEN TR_SF_TEST

#define TR_FSW_RESET_TELEM             TR("Telm", "Telemetri")

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS          "Tidstag 1","Tidstag 2","Tidstag 3"
#else
  #define TR_FSW_RESET_TIMERS          "Tid1","Tid2","Tid3"
#endif

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("Alle","Flight"),TR_FSW_RESET_TELEM

#define TR_FUNCSOUNDS                  TR("Bi1","Bip1"),TR("Bi2","Bip2"),TR("Bi3","Bi3"),TR("Adv1","Advarsel1"),TR("Adv2","Advarsel2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "Metrisk",TR("Imper.","Imperiel")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","mS","uS","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Ingen","Tal","Bjælke","Script"
#define TR_GPSFORMAT                   "DMS","NMEA"

#define TR_TEMPLATE_CLEAR_MIXES        "Slet mix"
#define TR_TEMPLATE_SIMPLE_4CH         "Simple 4-KA"
#define TR_TEMPLATE_STICKY_TCUT        "Sej-T-Cut"
#define TR_TEMPLATE_VTAIL              "V-Hale"
#define TR_TEMPLATE_DELTA              "Elevon\\Delta"
#define TR_TEMPLATE_ECCPM              "eCCPM"
#define TR_TEMPLATE_HELI               "Heli opsæt"
#define TR_TEMPLATE_SERVO_TEST         "Servo Test"

#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICKS_VSRCRAW              STR_CHAR_STICK "Sid", STR_CHAR_STICK "Høj", STR_CHAR_STICK "Gas", STR_CHAR_STICK "Kræ"

#if defined(PCBHORUS)
  #define TR_TRIMS_VSRCRAW             STR_CHAR_TRIM "Sid", STR_CHAR_TRIM "Høj", STR_CHAR_TRIM "Gas", STR_CHAR_TRIM "Kræ", STR_CHAR_TRIM "T5", STR_CHAR_TRIM "T6"
#else
  #define TR_TRIMS_VSRCRAW             STR_CHAR_TRIM "Sid", STR_CHAR_TRIM "Høj", STR_CHAR_TRIM "Gas", STR_CHAR_TRIM "Kræ"
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
  #define TR_VKEYS                     "Shift","Exit","Enter","Ned","Op","Højre","Venstre"
#elif defined(RADIO_FAMILY_JUMPER_T12)
  #define TR_VKEYS                     "Exit","Enter","Ned","Op","Højre","Venstre"
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define TR_VKEYS                     "Exit","Enter","PGUP","PGDN","SYS","MDL","TELE"
#elif defined(RADIO_T8) || defined(RADIO_COMMANDO8)
  #define TR_VKEYS                     "RTN","ENTER","PGUP","PGDN","SYS","MDL","OP","NED"
#elif defined(RADIO_ZORRO)
  #define TR_VKEYS                     "RTN","ENTER","PGUP","PGDN","SYS","MDL","TELE"
#elif defined(PCBTARANIS)
  #define TR_VKEYS                     "Menu","Exit","Enter","Page","Plus","Minus"
#else
  #define TR_VKEYS                     "Menu","Exit","Ned","Op","Højre","Venstre"
#endif

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Standard","Høj"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "EN","En"

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Normal","Invers"
#else
  #define TR_ROTARY_ENC_OPT         "Normal","Invers","V-I H-N","V-I H-A"
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

#define TR_RESERVE_VSRCRAW             "[--]"
#define TR_EXTRA_VSRCRAW               "Bat","Time","GPS",TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,"Tmr1","Tmr2","Tmr3"

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
#define TR_TEXT_SIZE                   "Standard","Tynd","Lille","Medium","Dobbelt"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center kun)","= (symmetrisk)"
#define TR_TIMER_DIR                   TR("Rest", "Vis rest"), TR("Gået tid", "Vis gået tid")

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "STD"
#else
#define TR_FONT_SIZES                  "STD","FED","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14)
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
#define TR_TRIMINC                     "Trim Step"
#define TR_DISPLAY_TRIMS               TR("Vis trim", "Vis trim")
#define TR_TTRACE                      TR("T-kilde", INDENT "kilde")
#define TR_TTRIM                       TR("T-trim-tomg", INDENT "Trim tomgang alene")
#define TR_TTRIM_SW                    TR("T-trim-ko", INDENT "Trim kontakt")
#define TR_BEEPCTR                     TR("Ctr Bip", "Center Bip")
#define TR_USE_GLOBAL_FUNCS            TR("Glob.Funk.", "Brug global funk.")
#define TR_PROTOCOL                    TR("Proto", "Protokol")
  #define TR_PPMFRAME                  INDENT "PPM frame"
  #define TR_REFRESHRATE               TR(INDENT "Genfrisk", INDENT "Genfrisk rate")
  #define STR_WARN_BATTVOLTAGE         TR(INDENT "Udgang er VBAT: ", INDENT "Advarsel: udgang niveau er VBAT: ")
#define TR_WARN_5VOLTS                 "Advarsel: udgang niveau er 5 volts"
#define TR_MS                          "ms"
#define TR_FREQUENCY                   INDENT "Frekvens"
#define TR_SWITCH                      "Kontakt"
#define TR_TRIMS                       "Trim"
#define TR_FADEIN                      "Tone ind"
#define TR_FADEOUT                     "Tone ud"
#define TR_DEFAULT                     "(standard)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Kontroller FT trim"
#else
  #define TR_CHECKTRIMS                CENTER "\006Kontroller\012trim"
#endif
#define OFS_CHECKTRIMS                 CENTER_OFS+(9*FW)
#define TR_SWASHTYPE                   "Swash Type"
#define TR_COLLECTIVE                  TR("Collective", "Coll. pitch kilde")
#define TR_AILERON                     TR("Lateral cyc.", "Lateral cyc. kilde")
#define TR_ELEVATOR                    TR("Long. cyc.", "Long. cyc. kilde")
#define TR_SWASHRING                   "Swash Ring"
#define TR_ELEDIRECTION                "HØJ retning"
#define TR_AILDIRECTION                "KRÆ retning"
#define TR_COLDIRECTION                "PIT retning"
#define TR_MODE                        TR("Tils.","Tilstand")
#if LCD_W > LCD_H
  #define TR_LEFT_STICK                "Venstre"
#else
  #define TR_LEFT_STICK                "Ven"
#endif
#define TR_SUBTYPE                     INDENT "Subtype"
#define TR_NOFREEEXPO                  "Ingen fri expo!"
#define TR_NOFREEMIXER                 "Ingen fri mix!"
#define TR_SOURCE                       "Kilde"
#define TR_WEIGHT                      "Vægt"
#define TR_EXPO                        TR("Expo", "Exponentiel")
#define TR_SIDE                        "Side"
#define TR_DIFFERENTIAL                "Forskel"
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
#define TR_GV                          "GV"
#else
#define TR_GV                          TR("G", "GV")
#endif
#define TR_ACHANNEL                    "En\004kanal"
#define TR_RANGE                       INDENT "Rækkevid"
#define TR_CENTER                      INDENT "Centrer"
#define TR_BAR                         "Bjælke"
#define TR_ALARM                       "Alarm"
#define TR_USRDATA                     TR("UsrData", "User data")
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
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI nedluk", "Check RSSI ved nedluk")
#define TR_MODEL_STILL_POWERED         "Model stadig med strøm"
#define TR_USB_STILL_CONNECTED         "USB stadig forbundet"
#define TR_MODEL_SHUTDOWN              "Luk ned?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Tryk enter for bekræft"
#define TR_THROTTLE_LABEL              "Gas"
#define TR_THROTTLE_START              "Gas Start"
#define TR_THROTTLEREVERSE             TR("T-omvendt", INDENT "Omvendt")
#define TR_MINUTEBEEP                  TR("Minut", "Minut kald")
#define TR_BEEPCOUNTDOWN               INDENT "Nedtælling"
#define TR_PERSISTENT                  TR(INDENT "Varig", INDENT "Varig")
#define TR_BACKLIGHT_LABEL             "Baggrunds lys"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLDELAY                     INDENT "Varighed"
#define TR_BLONBRIGHTNESS              INDENT "TIL klarhed"
#define TR_BLOFFBRIGHTNESS             INDENT "FRA klarhed"
#define TR_KEYS_BACKLIGHT              "Taster klarhed"
#define TR_BLCOLOR                     "Farve"
#define TR_SPLASHSCREEN                "Start skærm"
#define TR_PWR_ON_DELAY                "Forsinkelse ved tænd"
#define TR_PWR_OFF_DELAY               "Forsinkelse ved sluk"
#define TR_THROTTLE_WARNING            TR(INDENT "Gas adv", INDENT "Gas advarsel")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Tilp-Pos", INDENT INDENT INDENT INDENT "Tilpasset position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR(INDENT "Kon. adv", INDENT "Kontakt position")
#define TR_POTWARNINGSTATE             TR(INDENT "Drejk.&Skyd.", INDENT "Drejekontakt & skyder position")
#define TR_SLIDERWARNING               TR(INDENT "Skyd. pos", INDENT "Skyder position")
#define TR_POTWARNING                  TR(INDENT "Drejk. adv", INDENT "Drejekontakt position")
#define TR_TIMEZONE                    "Tids zone"
#define TR_ADJUST_RTC                  "Juster RTC"
#define TR_GPS                         "GPS"
#define TR_RXCHANNELORD                TR("Rx kanal ræk.", "Standard kanal rækkefølge")
#define TR_STICKS                      "Pinde"
#define TR_POTS                        "Drejekontakt"
#define TR_SWITCHES                    "Kontakter"
#define TR_SWITCHES_DELAY              TR("Cen forsink", "Center forsinkelse")
#define TR_SLAVE                       CENTER "Slave"
#define TR_MODESRC                     "Mode\006% Kilde"
#define TR_MULTIPLIER                  "Multiplier"
#define TR_CAL                         "Kal"
#define TR_CALIBRATION                 "Kalibrering"
#define TR_VTRIM                       "Trim - +"
#define TR_BG                          "BG:"
#define TR_CALIB_DONE                  "Kalibrering færdig"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Tryk [Enter] for start"
  #define TR_SETMIDPOINT               "Centrer pinde/drejekontakter/skydere og tryk [Enter]"
  #define TR_MOVESTICKSPOTS            "Flyt pinde/drejekontakter/skydere og tryk [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " FOR START"
  #define TR_SETMIDPOINT               "CENTER PINDE/SKYDER"
  #define TR_MOVESTICKSPOTS            "FLYT PINDE/DREJEKONTAKT"
#else
  #define TR_MENUTOSTART               CENTER "\010" TR_ENTER " FOR START"
  #define TR_SETMIDPOINT               TR(CENTER "\004SET PINDE I MIDTEN", CENTER "\004CENTRER PINDE/SKYDERE")
  #define TR_MOVESTICKSPOTS            CENTER "\006FLYT PINDE/DREJEKONTAKTER"
  #define TR_MENUWHENDONE              CENTER "\006" TR_ENTER " NÅR FÆRDIG"
#endif
#define TR_RXBATT                      "Rx Bat:"
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_ACCEL                       "Acc:"
#define TR_NODATA                      CENTER "INGEN DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Fri stak"
#define TR_INT_GPS_LABEL               "Intern GPS"
#define TR_HEARTBEAT_LABEL             "Hjerte puls"
#define TR_LUA_SCRIPTS_LABEL           "Lua script"
#define TR_FREE_MEM_LABEL              "Fri mem"
#define TR_TIMER_LABEL                 "Tid"
#define TR_THROTTLE_PERCENT_LABEL      "Gas %"
#define TR_BATT_LABEL                  "Batteri"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " for nulstil"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "KA"
#define TR_MODEL                       "MODEL"
#define TR_FM                          "FT"
#define TR_MIX                         "MIX"
#define TR_EEPROMLOWMEM                "EEPROM lav mem"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Tryk tast for at droppe"
#define TR_THROTTLE_NOT_IDLE           "Gas ikke i tomgang"
#define TR_ALARMSDISABLED              "Alarmer afkoblet"
#define TR_PRESSANYKEY                 TR("\010Tryk en tast", "Tryk en tast")
#define TR_BADEEPROMDATA               "Dårlig EEprom data"
#define TR_BAD_RADIO_DATA              "Dårlig radio data"
#define TR_RADIO_DATA_RECOVERED        TR3("Anvender radio data fra backup","Anvender radio indstillinger fra backup","Radio indstillinger genskabt fra backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio indstillinger fejlagtige","Radio indstillinger ikke gyyldige", "Ikke muligt at indlæse gyyldige radio indstillinger")
#define TR_EEPROMFORMATTING            "Formaterer EEPROM"
#define TR_STORAGE_FORMAT              "Lager klargøres"
#define TR_EEPROMOVERFLOW              "EEPROM overflow"
#define TR_RADIO_SETUP                 "RADIO SETUP"
#define TR_MENUDATEANDTIME             "DATO OG TID"
#define TR_MENUTRAINER                 "TRÆNER"
#define TR_MENUSPECIALFUNCS            "GLOBALE FUNKTIONER"
#define TR_MENUVERSION                 "VERSION"
#define TR_MENU_RADIO_SWITCHES         TR("KONTAKTER", "KONTAKTER TEST")
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "ANALOGS TEST")
#define TR_MENU_RADIO_ANALOGS_CALIB    "KALIBRERET ANALOGE"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RÅ ANALOGE (5 Hz)"
#define TR_MENUCALIBRATION             "KALIBRIERING"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trim => Subtrim"
#else
  #define TR_TRIMS2OFFSETS             "\006Trim => Subtrim"
#endif
#define TR_CHANNELS2FAILSAFE           "Kanaler=>Fejlsikring"
#define TR_CHANNEL2FAILSAFE            "Kanal=>Fejlsikring"
#define TR_MENUMODELSEL                TR("MODELLER", "MODEL VALG")
#define TR_MENU_MODEL_SETUP            TR("SETUP", "MODEL SETUP")
#define TR_MENUFLIGHTMODES             "FLYVE TILSTANDE"
#define TR_MENUFLIGHTMODE              "FLYVE TILSTAND"
#define TR_MENUHELISETUP               "HELI SETUP"

  #define TR_MENUINPUTS                "INDGANGE"
  #define TR_MENULIMITS                "UDGANGE"
#define TR_MENUCURVES                  "KURVER"
#define TR_MENUCURVE                   "KURVE"
#define TR_MENULOGICALSWITCH           "LOGISK KONTAKT"
#define TR_MENULOGICALSWITCHES         "LOGISKE KONTAKTER"
#define TR_MENUCUSTOMFUNC              "SPECIALFUNKTIONER"
#define TR_MENUCUSTOMSCRIPTS           "TILPASSET SCRIPT"
#define TR_MENUTELEMETRY               "TELEMETRI"
#define TR_MENUTEMPLATES               "SKABELONER"
#define TR_MENUSTAT                    "STAT"
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS1           "KANAL MONITOR 1-8"
#define TR_MONITOR_CHANNELS2           "KANAL MONITOR 9-16"
#define TR_MONITOR_CHANNELS3           "KANAL MONITOR 17-24"
#define TR_MONITOR_CHANNELS4           "KANAL MONITOR 25-32"
#define TR_MONITOR_SWITCHES            "LOGISK KONTAKT MONITOR"
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
#define TR_MULTI_AUTOBIND              TR(INDENT "Bind Ka.", INDENT "Bind til kanal")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Deaktiver Ch. map")
#define TR_DISABLE_TELEM               TR("No Telem", "Deaktiver telemetri")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "Automatisk", INDENT "Automatisk format")
#define TR_MULTI_LOWPOWER              TR(INDENT "Lav strøm", INDENT "Lav strøm tilstand")
#define TR_MULTI_LNA_DISABLE           INDENT "LNA deaktiver"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")
#define TR_MODULE_TELEM_ON             TR("TIL", "Aktiveret")
#define TR_DISABLE_INTERNAL            TR("Deaktiver int.", "Deaktiver intern RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("!seriel tilstand", "Ikke i seriel tilstand")
#define TR_MODULE_NO_INPUT             TR("Ingen indgang", "Ingen seriel indgang")
#define TR_MODULE_NO_TELEMETRY         TR3("Uden telemetri", "Uden MULTI_TELEMETRI", "Ingen MULTI_TELEMETRI opdaget")
#define TR_MODULE_WAITFORBIND          "Bind for at indlæse protokol"
#define TR_MODULE_BINDING              TR("Parko...","Parkobler")
#define TR_MODULE_UPGRADE_ALERT        TR3("Opg. krævet", "Modul opdatering krævet", "Module\nopdatering krævet")
#define TR_MODULE_UPGRADE              TR("Opg. anbefalet", "Modul opdatering anbefalet")
#define TR_REBIND                      "Rebinding påkrævet"
#define TR_REG_OK                      "Registration ok"
#define TR_BIND_OK                     "Bind lykkes"
#define TR_BINDING_CH1_8_TELEM_ON      "KA1-8 Telem TIL"
#define TR_BINDING_CH1_8_TELEM_OFF     "KA1-8 Telem FRA"
#define TR_BINDING_CH9_16_TELEM_ON     "KA9-16 Telem TIL"
#define TR_BINDING_CH9_16_TELEM_OFF    "KA9-16 Telem FRA"
#define TR_PROTOCOL_INVALID            TR("Prot. ugyldig", "Protocol ugyldig")
#define TR_MODULE_STATUS               TR(INDENT "Status", INDENT "Modul status")
#define TR_MODULE_SYNC                 TR(INDENT "Synk", INDENT "Proto Synk status")
#define TR_MULTI_SERVOFREQ             TR("Servo hast", "Servo opdaterings hastighed")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("RF kanal", "Vælg RF kanal")
#define TR_AFHDS3_RX_FREQ              TR("RX frekv.", "RX frekvens")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetri")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Akt. strøm", "Aktuel strøm")
#define TR_AFHDS3_POWER_SOURCE         TR("Strøm", "Strøm kilde")
#define TR_SYNCMENU                    "[Synk]"
#define TR_LIMIT                       INDENT "Grænse"
#define TR_MINRSSI                     "Min Rssi"
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Brug FlySky RSSI værdi uden reskalering")
#define TR_LATITUDE                    "Breddegrad"
#define TR_LONGITUDE                   "Længdegrad"
#define TR_GPS_COORDS_FORMAT           TR("GPS kordi.", "Kordinat format")
#define TR_VARIO                       TR("Vario", "Variometer")
#define TR_PITCH_AT_ZERO               "Højde nul"
#define TR_PITCH_AT_MAX                "Højde max"
#define TR_REPEAT_AT_ZERO              "Gentag nul"
#define TR_SHUTDOWN                    "LUKKER NED"
#define TR_SAVE_MODEL                  "Gemmer model indstillinger"
#define TR_BATT_CALIB                  TR("Bat. kalib", "Batteri kalibering")
#define TR_CURRENT_CALIB               "Aktuel kalib"
#define TR_VOLTAGE                     TR(INDENT "Spænding", INDENT "Spænding kilde")
#define TR_CURRENT                     TR(INDENT "Strøm", INDENT "Strøm kilde")
#define TR_SELECT_MODEL                "Vælg model"
#define TR_MODELS                      "Modeller"
#define TR_SELECT_MODE                 "Vælg tilstand"
#define TR_CREATE_MODEL                "Opret model"
#define TR_FAVORITE_LABEL              "Favoritter"
#define TR_MODELS_MOVED                "Unused models moved to"
#define TR_NEW_MODEL                   "Ny model"
#define TR_INVALID_MODEL               "Ingen billede"
#define TR_EDIT_LABELS                 "Ret type"
#define TR_MOVE_UP                     "Ingen billede"
#define TR_MOVE_DOWN                   "Flyt ned"
#define TR_ENTER_LABEL                 "Vælg type"
#define TR_LABEL                       "Type"
#define TR_LABELS                      "Typer"
#define TR_CURRENT_MODEL               "Aktuel"
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
#define TR_SHOW_MIXER_MONITORS         "Show mixer monitors"
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
#define TR_POWER_METER_EXT             "Strøm Meter (EXT)"
#define TR_POWER_METER_INT             "Strøm Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spectrum (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Spectrum (INT)"
#define TR_SDCARD_FULL                 "SD kort fuldt"
#define TR_NEEDS_FILE                  "MANGLER FIL"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Inkompatibel"
#define TR_WARNING                     "ADVARSEL"
#define TR_EEPROMWARN                  "EEPROM"
#define TR_STORAGE_WARNING             "LAGER"
#define TR_EEPROM_CONVERTING           "Konverterer EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD kort konvertering krævet"
#define TR_CONVERTING                  "Konvertering: "
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
#define TR_PCBREV_ERROR                "PCB fejl"
#define TR_NO_FAILSAFE                 "Fejlsikring ikke defineret"
#define TR_KEYSTUCK                    "Tast hænger"
#define TR_INVERT_THR                  TR("Omvendt gas?", "Omvendt (INV) gas?")
#define TR_VOLUME                      "Volumne"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Skarphed"
#define TR_CPU_TEMP                    "CPU temp."
#define TR_CPU_CURRENT                 "CPU strøm"
#define TR_CPU_MAH                     "Forbrug"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "MB Temp."
#define TR_CAPAWARNING                 INDENT "Kapacitet lav"
#define TR_TEMPWARNING                 INDENT "For varm"
#define TR_TTL_WARNING                 "Advarsel: brug 3.3V logisk niveau"
#define TR_FUNC                        "Funk"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
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
#define TR_SD_FORMAT                   "Format"
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "HARDWARE"
#define TR_FORMATTING                  "Formattering..."
#define TR_TEMP_CALIB                  "Temp. kalib"
#define TR_TIME                        "Tid"
#define TR_MAXBAUDRATE                 "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE                 "Måle tilstand"
#define TR_SAMPLE_MODES                "Normal","EnBit"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Hurtigvalg af model"
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
#define TR_BLUETOOTH_MODES             "---","Aktiveret"
#else
#define TR_BLUETOOTH_MODES             "---","Telemetri","Træner"
#endif

#define TR_SD_INFO_TITLE               "SD INFO"
#define TR_SD_TYPE                     "Type:"
#define TR_SD_SPEED                    "Hastighed:"
#define TR_SD_SECTORS                  "Sektorer:"
#define TR_SD_SIZE                     "Størrelse:"
#define TR_TYPE                        INDENT "Type"
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
#define TR_FLASH_BOOTLOADER            "Brænde bootloader"
#define TR_FLASH_DEVICE                TR("Brænde enhed","Brænde enhed")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Brænde S.Port", "Brænde S.Port enhed")
#define TR_FLASH_RECEIVER_OTA          "Brænde modtager OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Brænde RX med ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Brænde RX med int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Brænde FC med ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Brænde FC med int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Brænde BT module", "Brænde Bluetooth modul")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Brænde pwr mngt unit"
#define TR_DEVICE_NO_RESPONSE          TR("Enhed svarer ikke", "Enhed svarer ikke")
#define TR_DEVICE_FILE_ERROR           TR("Enhed fil prob.", "Enhed fil prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Enhed data afvist", "Enhed data afvist")
#define TR_DEVICE_WRONG_REQUEST        TR("Enhed adgang problem", "Enhed adgang problem")
#define TR_DEVICE_FILE_REJECTED        TR("Enhed fil afvist", "Enhed fil afvist")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Enhed file sig.", "Enhed file sig.")
#define TR_CURRENT_VERSION             TR("Aktuel ver: ", "Aktuel version: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Brænde int. module", "Brænde intern modul")
#define TR_FLASH_INTERNAL_MULTI        TR("Brænde int. multi", "Brænde intern multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Brænde ext. module", "Brænde ekstern modul")
#define TR_FLASH_EXTERNAL_MULTI        TR("Brænde ext. multi", "Brænde ekstern multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Brænde ext. ELRS", "Brænde ekstern ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("FW opdatering fejl", "Firmware opdaterings fejl")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Brændning successful"
#define TR_WRITING                     "Skriver..."
#define TR_CONFIRM_FORMAT              "Bekræft formattering?"
#define TR_INTERNALRF                  "Intern RF"
#define TR_INTERNAL_MODULE             TR("Int. module", "Intern modul")
#define TR_EXTERNAL_MODULE             TR("Ext. module", "Eksternt modul")
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX kræver opgradering"
#define TR_TELEMETRY_DISABLED          "Telem. deaktiveret"
#define TR_MORE_OPTIONS_AVAILABLE      "Flere tilvalg mulige"
#define TR_NO_MODULE_INFORMATION       "Ingen modul information"
#define TR_EXTERNALRF                  "Ekstern RF"
#define TR_FAILSAFE                    TR(INDENT "Fejlsikring", INDENT "Fejlsikring tilst")
#define TR_FAILSAFESET                 "FEJLSIKRING INDSTILLING"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Ejer ID"
#define TR_PINMAPSET                   "PINDE"
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
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
#define TR_MENU_POWER_METER            "POWER METER"
#define TR_SENSOR                      "SENSOR"
#define TR_COUNTRY_CODE                "Lande kode"
#define TR_USBMODE                     "USB tilstand"
#define TR_JACK_MODE                   "Jack tilstand"
#define TR_VOICE_LANGUAGE              "Stemme sprog"
#define TR_UNITS_SYSTEM                "Enheder"
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
#define TR_RESET_TIMER1                "Nulstil tidstag 1"
#define TR_RESET_TIMER2                "Nulstil tidstag 2"
#define TR_RESET_TIMER3                "Nulstil tidstag 3"
#define TR_RESET_TELEMETRY             "Nulstil telemetri"
#define TR_STATISTICS                  "Statistik"
#define TR_ABOUT_US                    "Om"
#define TR_USB_JOYSTICK                "USB joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB lager (SD)"
#define TR_USB_SERIAL                  "USB seriel (VCP)"
#define TR_SETUP_SCREENS               "Setup skærm"
#define TR_MONITOR_SCREENS             "Monitor"
#define TR_AND_SWITCH                  "OG kontakt"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Kalibreret analog"
#define TR_ANADIAGS_FILTRAWDEV         "Filteret rå analog med deviation"
#define TR_ANADIAGS_UNFILTRAW          "Ufilteret rå analog"
#define TR_ANADIAGS_MINMAX             "Min., max. and range"
#define TR_ANADIAGS_MOVE               "Flyt analoge til deres yderpunkter!"
#define TR_SPEAKER                     INDENT "Højttaler"
#define TR_BUZZER                      INDENT "Biper"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_POWERMETER_ATTN_NEEDED      "Attestering nødvendig"
#define TR_PXX2_SELECT_RX              "Vælg RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "Vælg enhed"
#define TR_DISCOVER                    "Opdag"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Venter..."
#define TR_RECEIVER_DELETE             "Slet modtager?"
#define TR_RECEIVER_RESET              "Nulstil modtager?"
#define TR_SHARE                       "Del"
#define TR_BIND                        "Bind"
#define TR_REGISTER                    TR("Reg", "Registrer")
#define TR_MODULE_RANGE                BUTTON(TR("Ræk", "Rækkevid"))
#define TR_RECEIVER_OPTIONS            TR("MODT. TILVALG", "MODTAGER TILVALG")
#define TR_DEL_BUTTON                  BUTTON(TR("Sle", "Sletter"))
#define TR_RESET_BTN                   BUTTON("Nulstil")
#define TR_DEBUG                       "Fejlfind"
#define TR_KEYS_BTN                    BUTTON("Taster")
#define TR_ANALOGS_BTN                 BUTTON(TR("Analo", "Analoge"))
#define TR_TOUCH_NOTFOUND              "Tryk skærm ikke fundet"
#define TR_TOUCH_EXIT                  "Tryk på skærm for at forlade"
#define TR_SET                         BUTTON("Set")
#define TR_TRAINER                     "Træner"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM              CENTER "TX antenne problem!"
#define TR_MODELIDUSED                 "ID brugt i:"
#define TR_MODELIDUNIQUE               "ID er entydig"
#define TR_MODULE                      "Modul"
#define TR_RX_NAME                     "Rx Navn"
#define TR_TELEMETRY_TYPE              TR("Type", "Telemetri type")
#define TR_TELEMETRY_SENSORS           "Sensorer"
#define TR_VALUE                       "Værdi"
#define TR_REPEAT                      "Gentag"
#define TR_ENABLE                      "Aktiver"
#define TR_TOPLCDTIMER                 "Top LCD ur"
#define TR_UNIT                        "Enhed"
#define TR_TELEMETRY_NEWSENSOR         INDENT "Tilføj ny"
#define TR_CHANNELRANGE                TR(INDENT "Kan. rækkev", INDENT "Kanal rækkevid")
#define TR_RXFREQUENCY                 TR("Rx Frekvens", "Rx Servo Frekvens")
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
#define TR_LOWALARM                    INDENT "Lav alarm"
#define TR_CRITICALALARM               INDENT "Kritisk alarm"
#define TR_RSSIALARM_WARN              "RSSI"
#define TR_NO_RSSIALARM                TR(INDENT "Alarmer afkoblet", "Telemetri alarmer afkoblet")
#define TR_DISABLE_ALARM               TR(INDENT "Afkobl alarmer", INDENT "Afkobl telemetri alarmer")
#define TR_ENABLE_POPUP                "Aktiver popup"
#define TR_DISABLE_POPUP               "Deaktiver popup"
#define TR_POPUP                       "Popup"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Forudindstil..."
#define TR_PRESET                      "Forudindstil"
#define TR_MIRROR                      "Spejle"
#define TR_CLEAR                       "Slet"
#define TR_RESET                       "Nulstil"
#define TR_RESET_SUBMENU               "Nulstil..."
#define TR_COUNT                       "Tæller"
#define TR_PT                          "pt"
#define TR_PTS                         "pts"
#define TR_SMOOTH                      "Jævn"
#define TR_COPY_STICKS_TO_OFS          TR("Kopi pind->subtrim", "Kopier pinde til subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Kopi min/max ->alle",  "Kopier min/max/center til alle udgange")
#define TR_COPY_TRIMS_TO_OFS           TR("Kopi trim->subtrim", "Kopier trim til subtrim")
#define TR_INCDEC                      "Øg/sænk"
#define TR_GLOBALVAR                   "Global var"
#define TR_MIXSOURCE                   "Mix kilde"
#define TR_CONSTANT                    "Konstant"
#define TR_PERSISTENT_MAH              TR(INDENT "Varig mAh", INDENT "Varig mAh")
#define TR_PREFLIGHT                   "Fly checkliste"
#define TR_CHECKLIST                   TR(INDENT "Checkliste", INDENT "Vis checkliste")
#define TR_FAS_OFFSET                  TR(INDENT "FAS Ofs", INDENT "FAS offset")
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

#define TR_LSW_HEADERS_FUNCTION        "Funktion"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "OG kontakt"
#define TR_LSW_HEADERS_DURATION        "Varighed"
#define TR_LSW_HEADERS_DELAY           "Forsinkelse"

#define TR_GVAR_HEADERS_NAME           "Navn"
#define TR_GVAR_HEADERS_FM0            "Værdi ved FT0"
#define TR_GVAR_HEADERS_FM1            "Værdi ved FT1"
#define TR_GVAR_HEADERS_FM2            "Værdi ved FT2"
#define TR_GVAR_HEADERS_FM3            "Værdi ved FT3"
#define TR_GVAR_HEADERS_FM4            "Værdi ved FT4"
#define TR_GVAR_HEADERS_FM5            "Værdi ved FT5"
#define TR_GVAR_HEADERS_FM6            "Værdi ved FT6"
#define TR_GVAR_HEADERS_FM7            "Værdi ved FT7"
#define TR_GVAR_HEADERS_FM8            "Værdi ved FT8"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS            { "Sammenlign type eller funktion", "Første variabel", "Anden variabel eller konstant", "Anden variabel eller konstant", "Endnu en betingelse for linje", "Minimum TIL varighed for logisk kontakt", "Minimum SAND varighed for betingelse bliver sand" }

// Horus layouts and widgets
#define TR_FIRST_CHANNEL               "Første kanal"
#define TR_FILL_BACKGROUND             "Udfyld baggrund?"
#define TR_BG_COLOR                    "BG farve"
#define TR_SLIDERS_TRIMS               "Skyder+Trim"
#define TR_SLIDERS                     "Skydere"
#define TR_FLIGHT_MODE                 "Flyve tilstand"
#define TR_INVALID_FILE                "Ugyldig fil"
#define TR_TIMER_SOURCE                "Tidstagning kilde"
#define TR_SIZE                        "Størrelse"
#define TR_SHADOW                      "Skygge"
#define TR_TEXT                        "Tekst"
#define TR_COLOR                       "Farve"
#define TR_MAIN_VIEW_X                 "Hoved visning X"
#define TR_PANEL1_BACKGROUND           "Panel1 baggrund"
#define TR_PANEL2_BACKGROUND           "Panel2 baggrund"
#define TR_WIDGET_GAUGE                "Måling"
#define TR_WIDGET_MODELBMP             "Modeller"
#define TR_WIDGET_OUTPUTS              "Udgange"
#define TR_WIDGET_TEXT                 "Tekst"
#define TR_WIDGET_TIMER                "Tider"
#define TR_WIDGET_VALUE                "Værdi"

// About screen
#define TR_ABOUTUS                     TR(" OM ", "OM")

#define TR_CHR_SHORT                   'k'
#define TR_CHR_LONG                    'l'
#define TR_CHR_TOGGLE                  't'
#define TR_CHR_HOUR                    't'
#define TR_CHR_INPUT                   'K'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Bip volume"
#define TR_WAV_VOLUME                  "Wav volume"
#define TR_BG_VOLUME                   TR("Bg volume", "Baggrund volume")

#define TR_TOP_BAR                     "Top bjælke"
#define TR_FLASH_ERASE                 "Flash slettes..."
#define TR_FLASH_WRITE                 "Flash skrives..."
#define TR_OTA_UPDATE                  "OTA opdatering..."
#define TR_MODULE_RESET                "Nulstil modul..."
#define TR_UNKNOWN_RX                  "Ukendt RX"
#define TR_UNSUPPORTED_RX              "Ikke understøttet RX"
#define TR_OTA_UPDATE_ERROR            "OTA opdaterings fejl"
#define TR_DEVICE_RESET                "Nulstil enhed..."
#define TR_ALTITUDE                    INDENT "Højde"
#define TR_SCALE                       "Skala"
#define TR_VIEW_CHANNELS               "Vis kanaler"
#define TR_VIEW_NOTES                  "Vis noter"
#define TR_MODEL_SELECT                "Vælg model"
#define TR_MODS_FORBIDDEN              "Ændringer forbudt!"
#define TR_UNLOCKED                    "Ulåst"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Præcision"
#define TR_RATIO                       "Forhold"
#define TR_FORMULA                     "Formel"
#define TR_CELLINDEX                   "Cell index"
#define TR_LOGS                        "Log"
#define TR_OPTIONS                     "Tilvalg"
#define TR_FIRMWARE_OPTIONS            "Firmware tilvalg"

#define TR_ALTSENSOR                   "Alt sensor"
#define TR_CELLSENSOR                  "Cell sensor"
#define TR_GPSSENSOR                   "GPS sensor"
#define TR_CURRENTSENSOR               "Sensor"
#define TR_AUTOOFFSET                  "Auto Offset"
#define TR_ONLYPOSITIVE                "Positive"
#define TR_FILTER                      "Filter"
#define TR_TELEMETRYFULL               TR("Alle slots fulde!", "Alle telemetri slots fulde!")
#define TR_SERVOS_OK                   "Servo OK"
#define TR_SERVOS_KO                   "Servo KO"
#define TR_INVERTED_SERIAL             INDENT "Invers"
#define TR_IGNORE_INSTANCE             TR(INDENT "Ingen inst.", INDENT "Ignorer instans")
#define TR_DISCOVER_SENSORS            "Søg efter nye"
#define TR_STOP_DISCOVER_SENSORS       "Stop"
#define TR_DELETE_ALL_SENSORS          "Slet alle"
#define TR_CONFIRMDELETE               "Virkeligt " LCDW_128_480_LINEBREAK "slet alle ?"
#define TR_SELECT_WIDGET               "Vælg widget"
#define TR_WIDGET_FULLSCREEN           "Fuld skærm"
#define TR_REMOVE_WIDGET               "Slet widget"
#define TR_WIDGET_SETTINGS             "Widget indstilinger"
#define TR_REMOVE_SCREEN               "Slet skærm"

#if LCD_W > LCD_H
  #define TR_SETUP_WIDGETS              "Opsæt widget"
#else
  #define TR_SETUP_WIDGETS              "Widget"
#endif

#define TR_USER_INTERFACE              "Bruger interface"
#define TR_THEME                       "Tema"
#define TR_SETUP                       "Opsætning"
#define TR_LAYOUT                      "Layout"
#define TR_ADD_MAIN_VIEW               "Tilføj hoved billede"
#define TR_BACKGROUND_COLOR            "Baggrund farve"
#define TR_MAIN_COLOR                  "Hoved visning farve"
#define TR_BAR2_COLOR                  "Anden bjælke bar farve"
#define TR_BAR1_COLOR                  "Hoved bjælke farve"
#define TR_TEXT_COLOR                  "Tekst farve"
#define TR_TEXT_VIEWER                 "Tekst visning"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Indgange"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua script"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Pinde"
#define TR_MENU_POTS                   STR_CHAR_POT "Drejekontakt"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "Cyklisk"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Trim"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Kontakter"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Logiske kontakter"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Træner"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Kanaler"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "Globale Var"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Telemetri"
#define TR_MENU_DISPLAY                "DISPLAY"
#define TR_MENU_OTHER                  "Andet"
#define TR_MENU_INVERT                 "Invers"
#define TR_JITTER_FILTER               "ADC filter"
#define TR_DEAD_ZONE                   "Dødt område"
#define TR_RTC_CHECK                   TR("Check RTC", "Check RTC spænding")
#define TR_AUTH_FAILURE                "Godkendelse fejlet"
#define TR_RACING_MODE                 "Racing tilstand"

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
#define STR_PITCH                     "Højd"
#define STR_ROLL                      "Kræn"
#define STR_YAW                       "Side"
#define STR_THROTTLE                  "Gas"
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
#define STR_SPEED                     "Fart"
#define STR_FLOW                      "Flow"

#define TR_USE_THEME_COLOR              "Brug tema farve"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Tillæg alle trim til subtrim"

#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS        "Åbn kanal monitor"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Åbn kanal mon."
#endif

#define TR_DUPLICATE                    "Dupliker"
#define TR_ACTIVATE                     "Set aktiv"
#define TR_RED                          "Rød"
#define TR_BLUE                         "Blå"
#define TR_GREEN                        "Grøn"
#define TR_COLOR_PICKER                 "Farve vælger"
#define TR_EDIT_THEME_DETAILS           "Ret tema detajler"
#define TR_AUTHOR                       "Forfatter"
#define TR_DESCRIPTION                  "Beskrivelse"
#define TR_SAVE                         "Gem"
#define TR_CANCEL                       "Fortryd"
#define TR_EDIT_THEME                   "RET TEMA"
#define TR_DETAILS                      "Detajler"
#define TR_THEME_EDITOR                 "TEMAER"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Vælg\nModel"
#define TR_MAIN_MENU_MODEL_NOTES        "Model\nNoter"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Kanal\nMonitor"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Model\nIndstilling"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Radio\nIndstilling"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Skærm\nIndstilling"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Nulstil\nTelemetri"
#define TR_MAIN_MENU_STATISTICS         "Statistik"
#define TR_MAIN_MENU_ABOUT_EDGETX       "Om\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "Engelsk"
#define TR_VOICE_CHINESE                "Kinesisk"
#define TR_VOICE_CZECH                  "Tjekkisk"
#define TR_VOICE_DANISH                 "Dansk"
#define TR_VOICE_DEUTSCH                "Tysk"
#define TR_VOICE_DUTCH                  "Hollandsk"
#define TR_VOICE_ESPANOL                "Spansk"
#define TR_VOICE_FRANCAIS               "Fransk"
#define TR_VOICE_HUNGARIAN              "Ungarsk"
#define TR_VOICE_ITALIANO               "Italiensk"
#define TR_VOICE_POLISH                 "Polsk"
#define TR_VOICE_PORTUGUES              "Portugisisk"
#define TR_VOICE_RUSSIAN                "Russisk"
#define TR_VOICE_SLOVAK                 "Slovakisk"
#define TR_VOICE_SWEDISH                "Svensk"
#define TR_VOICE_TAIWANESE              "Taiwanesisk"
