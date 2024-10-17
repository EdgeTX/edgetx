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

#define TR_MIN_PLURAL2  2
#define TR_MAX_PLURAL2  2
// For this number of minute in the last decimal place singular form is used in
// plural
#define TR_USE_SINGULAR_IN_PLURAL 1
#define TR_USE_PLURAL2_SPECIAL_CASE 0
// If the number of minutes is above this value PLURAL2 is used
#define TR_USE_PLURAL2 INT_MAX

#define TR_MINUTE_SINGULAR             "minute"
#define TR_MINUTE_PLURAL1              "minuten"
#define TR_MINUTE_PLURAL2              "minuten"

// NON ZERO TERMINATED STRINGS
#define TR_OFFON                       "AUS","EIN"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Stumm","Alarm","NoKey","Alle"
#define TR_VBLMODE                     "AUS","Taste","Stks","Beide","EIN"
#define TR_TRNMODE                     "AUS",TR("+=","Addiere"),TR(":=","Ersetze")
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES            "AUS","Telem weiterl.","Telemetrie In","SBUS Eingang","LUA","CLI","GPS","Debug","SpaceMouse","Externes Modul"
#define TR_SWTYPES                     "Kein","Taster","2POS","3POS"
#define TR_POTTYPES                    "Kein","Poti", TR("Pot o. Ras","Poti mit Raste"),"Schieber",TR("Multipos.","Multipos. Schalter"),"Knüppel X","Knüppel Y","Schalter"
#define TR_VPERSISTENT                 "AUS","Flugzeit","Manuell Rück"
#define TR_COUNTRY_CODES               TR("US","Amerika"),TR("JP","Japan"),TR("EU","Europa")
#define TR_USBMODES                    "Fragen",TR("Joyst","Joystick"),TR("SDCard","Speicher"),TR("Serial","Seriell")
#define TR_JACK_MODES                  "Popup","Audio","Trainer"

#define TR_SBUS_INVERSION_VALUES       "normal","nicht inv."
#define TR_MULTI_CUSTOM                "Benutzer"
#define TR_VTRIMINC                    TR("Expo","Exponentiell"),TR("ExFein","Extrafein"),"Fein","Mittel","Grob"
#define TR_VDISPLAYTRIMS               "Nein","Kurz","Ja"  // Trimmwerte Keine, kurze Anzeigen, Ja
#define TR_VBEEPCOUNTDOWN              "Kein", "Pieps", "Stimme", "Haptik", TR("P & H","Pieps & Haptik"), TR("St & H","Stimme & Haptik")
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VVARIOCENTER                "Ton","Ruhe"
#define TR_CURVE_TYPES                 "Nur Y","X und Y" // "Standard","Custom"

#define TR_ADCFILTERVALUES             "Global","Aus","Ein"

#define TR_VCURVETYPE                  "Diff","Expo","Funk","Ind."
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Addiere","Multipl.","Ersetze"
#define TR_VMLTPX2                     "+=","*=",":="

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Takt"  // TIM = Takt = Taktgenerator
  #define TR_CSWSTICKY                 "SRFF"  // Sticky = RS-Flip-Flop
  #define TR_CSWSTAY                   "Puls"  // Edge = einstellbarer Impuls
#else
  #define TR_CSWTIMER                  "Takt"  // TIM = Takt = Taktgenerator
  #define TR_CSWSTICKY                 "SRFF"  // Sticky = RS-Flip-Flop
  #define TR_CSWSTAY                   "Puls"  // Edge = einstellbarer Impuls
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC            	       "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

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
#define TR_SF_SET_SCREEN               "Set Main Screen"
#define TR_SF_SET_TELEMSCREEN          "TelSeite anz"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Telm","Telemetrie")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS            "Tmr1","Tmr2","Tmr3"


#define TR_VFSWRESET 	      		       TR_FSW_RESET_TIMERS,"All",TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS         		     TR("Bp1","Piep1"),TR("Bp2","Piep2"),TR("Bp3","Piep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"
#define TR_VUNITSSYSTEM                TR("Metrik","Metrisch"),TR("Imper.","Imperial")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "None","Werte","Balken","Script"
#define TR_GPSFORMAT                   "GMS","NMEA"  // Koordinatenanzeige

#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "Sei"
#define TR_STICK_NAMES1                "Höh"
#define TR_STICK_NAMES2                "Gas"
#define TR_STICK_NAMES3                "Que"
#define TR_SURFACE_NAMES0              "Str"
#define TR_SURFACE_NAMES1              "Gas"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Default","High"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ON","One"

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "Joystick Modus"
#define TR_HATSOPT                     "nur Trimmer","nur Tasten","Umschaltbar","Global"
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
#endif

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Normal","Inverted"
#else
  #define TR_ROTARY_ENC_OPT         "Normal","Inverted","V-I H-N","V-I H-A","V-N E-I"
#endif

#if defined(IMU)
  #define TR_IMU_VSRCRAW               "TltX","TltY",
#else
  #define TR_IMU_VSRCRAW
#endif

#if defined(HELI)
#define TR_CYC_VSRCRAW                 "CYC1","CYC2","CYC3"
#else
#define TR_CYC_VSRCRAW                 "[C1]","[C2]","[C3]"
#endif


#define TR_SRC_BATT                    "Batt"
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES                   "AUS","EIN","Strt","GSs","GS%","GSt"
#define TR_VTRAINER_MASTER_OFF         "AUS"
#define TR_VTRAINER_MASTER_JACK        "Lehrer/Buchse"
#define TR_VTRAINER_SLAVE_JACK         "Schüler/Buchse"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Lehrer/SBUS Modul"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Lehrer/CPPM Modul"
#define TR_VTRAINER_MASTER_BATTERY     "Lehrer/Serial"
#define TR_VTRAINER_BLUETOOTH          TR("Lehrer/BT","Lehrer/Bluetooth"),TR("Schüler/BT","Schüler/Bluetooth")
#define TR_VTRAINER_MULTI              "Lehrer/Multi"
#define TR_VFAILSAFE                   "Kein Failsafe","Halte Pos.","Kanäle","Kein Signal","Empfänger"
#define TR_VSENSORTYPES                "Sensor","Berechnung"
#define TR_VFORMULAS                   "Addieren","Mittelwert","Min","Max","Multiplizier","Gesamt","Zelle","Verbrauch","Distanz"
#define TR_VPREC                       "0.--","0.0","0.00"
#define TR_VCELLINDEX                  "Niedrigst","1. Zelle","2. Zelle","3. Zelle","4. Zelle","5. Zelle","6. Zelle","7. Zelle","8. Zelle","Höchster","Differenz"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA" (center only)","= (symetrical)"
#define TR_TIMER_DIR                   TR("Rückw.", "Rückwärts"), TR("Vorwä.", "Vorwärts")
#define TR_PPMUNIT                     "0.--","0.0","us"

// ZERO TERMINATED STRINGS

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "STD"
#else
#define TR_FONT_SIZES                  "STD","FETT","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14) || defined(PCBPL18)
  #define TR_ENTER                     "[NEXT]"
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

#define TR_FREE                        "frei"
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
#define TR_START                       "Start"
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
#define STR_WARN_BATTVOLTAGE           TR("Ausg. ist VBAT: ", "Warnung: Ausg.pegel ist VBAT: ")
#define TR_WARN_5VOLTS                 "Warnung: Ausgangspegel ist 5 Volt"
#define TR_MS                 		     "ms"
#define TR_SWITCH                      TR("Schalt.", "Schalter")
#define TR_FUNCTION_SWITCHES           "Anpassbare Schalter"
#define TR_GROUP                       "Group"
#define TR_GROUP_ALWAYS_ON             "Always on"
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
#define TR_TRIMS                       "Trimmer"
#define TR_FADEIN                      "Langs. Ein"
#define TR_FADEOUT                     "Langs. Aus"
#define TR_DEFAULT                     "(Normal)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Prüfe Flugphasen-Trimmung"
#else
  #define TR_CHECKTRIMS                "\006Prüfe\012Trimmung"
#endif
#define TR_SWASHTYPE                   TR("Typ Taumelsch", "Typ  Taumelscheibe")
#define TR_COLLECTIVE                  TR("Kollekt. Pitch", "Kollekt. Pitch Quelle")
#define TR_AILERON                     "Roll Quelle"
#define TR_ELEVATOR                    "Nick Quelle"
#define TR_SWASHRING                   TR("Ring   Begrenz", "Ring Begrenzung")
#define TR_MODE                        "Modus"
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "Links"
#else
  #define TR_LEFT_STICK                "Li"
#endif
#define TR_SUBTYPE                     "Subtype"
#define TR_NOFREEEXPO                  "Expos voll!"
#define TR_NOFREEMIXER                 "Mischer voll!"
#define TR_SOURCE                      "Quelle"
#define TR_WEIGHT                      "Gewicht"
#define TR_SIDE                        "Seite"
#define TR_OFFSET                      "Offset"
#define TR_TRIM                        "Trim"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
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
#define TR_MIXES                       "MISCHER"
#define TR_CV                          "KV"
#if defined(PCBNV14) || defined(PCBPL18)
#define TR_GV                          "GV"
#else
#define TR_GV                          TR("G", "GV")
#endif
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
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Drücke [Enter] zum Start"
  #define TR_SETMIDPOINT               "Knüppel/Schieber zentrieren und [Enter]"
  #define TR_MOVESTICKSPOTS            "Bewege Knüppel/Poti/Schieber und drücke [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " Zum Start"
  #define TR_SETMIDPOINT               "Knüppel/Schieber zentrieren"
  #define TR_MOVESTICKSPOTS            "Knüppel/Schieber bewegen"
  #define TR_MENUWHENDONE              TR_ENTER " wenn fertig"
#else
  #define TR_MENUTOSTART               TR_ENTER " Zum START"
#if defined(SURFACE_RADIO)
  #define TR_SETMIDPOINT               "SCHIEBER AUF MITTE"
  #define TR_MOVESTICKSPOTS            "BEWEGE LENK/GAS/POTI/KNÜPPEL"
#else
  #define TR_SETMIDPOINT               TR("KNÜPPEL AUF MITTE", "ZENTRIERE KNÜPPEL/SCHIEBER")
  #define TR_MOVESTICKSPOTS            "BEWEGE KNÜPPEL/SCHIEBER"
#endif
  #define TR_MENUWHENDONE              TR_ENTER " wenn fertig"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
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
#if defined(SURFACE_RADIO)
#define TR_FM                          "DM"
#else
#define TR_FM                          "FP"
#endif
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
#define TR_MENUTRAINER                 TR("LEHRER/SCHÜLER", "LEHRER/SCHÜLER")
#define TR_MENUSPECIALFUNCS            "GLOBALE FUNKTIONEN"
#define TR_MENUVERSION                 "VERSION"
#define TR_MENU_RADIO_SWITCHES         TR("Schalter-Test", "Schalter-Test")
#define TR_MENU_RADIO_ANALOGS          "Geber-Test"
#define TR_MENU_RADIO_ANALOGS_CALIB    "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION             TR("KALIB. ANALOG", "KALIBRIERUNG-Analog")
#define TR_MENU_FSWITCH                "ANPASSBARE SCHALTER"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS             "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE           "Channels=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Channel=>Failsafe"
#define TR_MENUMODELSEL        		   TR("MODELLE", "MODELL WÄHLEN")
#define TR_MENU_MODEL_SETUP            TR("MODELL-EINSTELLUNG", "MODELL-EINSTELLUNGEN")
#if defined(SURFACE_RADIO)
  #define TR_MENUFLIGHTMODES           "FAHRMODI"
  #define TR_MENUFLIGHTMODE            "FAHRMODUS"
#else
  #define TR_MENUFLIGHTMODE            "FLUGPHASE"
  #define TR_MENUFLIGHTMODES   		     "FLUGPHASEN"
#endif
#define TR_MENUHELISETUP               TR("HELI TS-Mischer", "HELI TS-Mischer CYC1-3")
#define TR_MENUINPUTS                  "INPUTS"  //"Inputs=Geber"
#define TR_MENULIMITS                  "SERVOS"  //"AUSGABEN" oder "Servos"
#define TR_MENUCURVES                  "KURVEN"
#define TR_MENUCURVE                   "KURVE"
#define TR_MENULOGICALSWITCH           "LOGIKSCHALTER"
#define TR_MENULOGICALSWITCHES         "LOGIKSCHALTER"
#define TR_MENUCUSTOMFUNC              TR("SPEZ.-FUNKTIONEN", "SPEZIAL-FUNKTIONEN")
#define TR_MENUCUSTOMSCRIPTS           "LUA-SCRIPTE"
#define TR_MENUTELEMETRY               "TELEMETRIE"
#define TR_MENUSTAT                    "STAT"
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS1           "KANAL+MISCHER MONITOR 1-8"
#define TR_MONITOR_CHANNELS2           "KANAL+MISCHER MONITOR 9-16"
#define TR_MONITOR_CHANNELS3           "KANAL+MISCHER MONITOR 17-24"
#define TR_MONITOR_CHANNELS4           "KANAL+MISCHER MONITOR 25-32"
#define TR_MONITOR_SWITCHES            "LOGIK SCHALTER MONITOR"
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
#define TR_MODULES_RX_VERSION          "Modul / RX version"
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
#define TR_SDCARD_FULL                 "SD-Karte voll"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\nLogs und Screenshots deaktiviert"
#else
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036Logs und " LCDW_128_LINEBREAK "Screenshots deaktiviert"
#endif
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
#define TR_SD_CARD                     "SD-Karte"
#define TR_SDHC_CARD                   "SDHC-Karte"
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
#define TR_HARDWARE                    TR("Hardware einst. ", "Namen und Hardware einst.")
#define TR_FORMATTING                  "Formatierung..."
#define TR_TEMP_CALIB                  "Temp.  abgl."
#define TR_TIME                        "Uhrzeit:"
#define TR_BAUDRATE                    "Baudrate"
#define TR_MAXBAUDRATE                 "Max Baud"
#define TR_SAMPLE_MODE                 "Abtastmodus"
#define TR_SAMPLE_MODES                "Normal","OneBit"
#define TR_LOADING                     "Wird geladen..."
#define TR_DELETE_THEME                "Theme löschen?"
#define TR_SAVE_THEME                  "Theme speichern?"
#define TR_EDIT_COLOR                  "Farbe bearbeiten"
#define TR_NO_THEME_IMAGE              "Kein Theme Bild"
#define TR_BACKLIGHT_TIMER             "Inaktivitäts Timeout"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "schnelle Modellauswahl"
  #define TR_LABELS_SELECT             "Labelauswahl"
  #define TR_LABELS_MATCH              "Labelvergleich"
  #define TR_FAV_MATCH                 "Favoriten vergleichen"
  #define TR_LABELS_SELECT_MODE        "Mehrfachauswahl", "Einfachauswahl"
  #define TR_LABELS_MATCH_MODE         "Alle", "Beliebig"
  #define TR_FAV_MATCH_MODE            "Muss übereinstimmen", "Alternative Übereinstimmung"
#endif

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
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Aktiviert"
#else
#define TR_BLUETOOTH_MODES             "---","Telemetrie","Trainer"
#endif
#define TR_SD_INFO_TITLE               "SD-INFO"
#define TR_SD_SPEED                    "Geschw:"
#define TR_SD_SECTORS                  "Sektoren:"
#define TR_SD_SIZE                     "Größe:"
#define TR_TYPE                        "Typ"
#define TR_GLOBAL_VARS                 "Globale Variablen"
#define TR_GVARS                       "GLOBALE V."
#define TR_GLOBAL_VAR                  "Globale Variable"
#define TR_MENU_GLOBAL_VARS            "GLOBALE VARIABLEN"
#define TR_OWN                         "Eigen"
#define TR_DATE                        "Datum:"
#define TR_MONTHS                      { "Jan", "Feb", "Mar", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez" }
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
#define TR_FIRMWARE_UPDATE_SUCCESS     TR3("Flash successful","Flash successful","Update erfolgreich")
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
#define TR_SPEAKER                     "Lautspr"
#define TR_BUZZER                      "Summer"
#define TR_BYTES                       "Bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd","Binden"))   //9XR-Pro
#define TR_MODULE_UNBIND               BUTTON("Trennen")
#define TR_POWERMETER_ATTN_NEEDED      "Dämpfungsgl. nötig"
#define TR_PXX2_SELECT_RX              "Wähle RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "Wähle Gerät"
#define TR_DISCOVER                    "Suche"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Warte..."
#define TR_RECEIVER_DELETE             "Empfänger löschen?"
#define TR_RECEIVER_RESET              "Empfänger resetten?"
#define TR_SHARE                       "Share"
#define TR_BIND                        "Binden"
#define TR_REGISTER             	   TR("Reg", "Register")
#define TR_MODULE_RANGE        		   BUTTON(TR("Rng", "Reichweite"))  //9XR-Pro
#define TR_RANGE_TEST                  "Reichweitentest"
#define TR_RECEIVER_OPTIONS            TR("RX OPTIONEN", "RX OPTIONEN")
#define TR_RESET_BTN           		   BUTTON("Reset")
#define TR_DEBUG                       "Testen"
#define TR_KEYS_BTN                	   BUTTON(TR("SW","Schalter"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analoge"))
#define TR_FS_BTN                      BUTTON(TR("AnpSchalt", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_CALIBRATION                 "Kalibrieren"
#define TR_SET                   	   BUTTON("Set")
#define TR_TRAINER             		   "Lehrer/Schüler"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM     		   "TX-Antennenproblem!"
#define TR_MODELIDUSED                 "ID benutzt in:"
#define TR_MODELIDUNIQUE               "ID ist eindeutig"
#define TR_MODULE             		   "Modul-Typ"
#define TR_RX_NAME                     "Rx Name"
#define TR_TELEMETRY_TYPE      		   TR("Typ", "Telemetrietyp")
#define TR_TELEMETRY_SENSORS  		   "Sensoren"
#define TR_VALUE               		   "Wert"
#define TR_PERIOD                    "Periode"
#define TR_INTERVAL                  "Intervall"
#define TR_REPEAT                      "Wiederholung"
#define TR_ENABLE                      "Aktivieren"
#define TR_DISABLE                     "Deaktivieren"
#define TR_TOPLCDTIMER        		   "oberer LCD Timer"
#define TR_UNIT                        "Einheit"
#define TR_TELEMETRY_NEWSENSOR         "Sensor hinzufügen ..."
#define TR_CHANNELRANGE                TR("Kanäle", "Ausgangs Kanäle")  //wg 9XR-Pro
#define TR_AFHDS3_RX_FREQ              TR("RX Freq.", "RX Frequenz")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetrie")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Tat.Leis", "tatsäch. Leistung")
#define TR_AFHDS3_POWER_SOURCE         TR("Stromqu.", "Stromquelle")
#define TR_ANTENNACONFIRM1             "Ant. umschalten"
#if defined(PCBX12S)
#define TR_ANTENNA_MODES               "Intern","Frag","Modellspezifisch","Intern + Extern"
#else
#define TR_ANTENNA_MODES               "Intern","Frag","Modellspezifisch","Extern"
#endif
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
#define TR_PREFLIGHT_POTSLIDER_CHECK   "Aus","Ein","Auto"
#define TR_PREFLIGHT                   "Vorflug-Checkliste"
#define TR_CHECKLIST                   TR("Checkliste", "Checkliste anzeigen")
#define TR_CHECKLIST_INTERACTIVE       TR3("C-Interaktiv", "Interakt. Checkl.", "Interaktive Checkliste")
#define TR_AUX_SERIAL_MODE             "Serieller Port"
#define TR_AUX2_SERIAL_MODE            "Serieller Port 2"
#define TR_AUX_SERIAL_PORT_POWER       "Versorgung"
#define TR_SCRIPT                      "Lua-Skript"
#define TR_INPUTS                      "Eingaben"
#define TR_OUTPUTS                     "Ausgaben"
#define STR_EEBACKUP                   TR("EEPROM->SD", "Backup EEPROM->SD-Karte")
#define STR_FACTORYRESET               TR("Werksreset", "Auf Werkseinstellungen")
#define TR_CONFIRMRESET                TR("Alles löschen? ","ALLE Modelle+Einst. löschen?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Zu viele Skripte!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","EIN"
#define TR_NO_TELEMETRY_SCREENS        "Keine Telemetrie Seiten"
#define TR_TOUCH_PANEL                 "Touch panel:"
#define TR_FILE_SIZE                   "Dateigröße"
#define TR_FILE_OPEN                   "trotzdem öffnen?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

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

#if defined(COLORLCD)
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
  #define TR_ALIGN_OPTS                { "Links", "Mitte", "Rechts" }
  #define TR_TEXT                      "Text"
  #define TR_COLOR                     "Farbe"
  #define TR_MAIN_VIEW_X               "Hauptansicht XX"
  #define TR_PANEL1_BACKGROUND         "Panel1 Hintergrund"
  #define TR_PANEL2_BACKGROUND         "Panel2 Hintergrund"
  #define TR_WIDGET_GAUGE              "Pegel"
  #define TR_WIDGET_MODELBMP           "Modellinfo"
  #define TR_WIDGET_OUTPUTS            "Ausgänge"
  #define TR_WIDGET_TEXT               "Text"
  #define TR_WIDGET_TIMER              "Timer"
  #define TR_WIDGET_VALUE              "Wert"
#endif

// Bootloader common - ASCII characters only
#define TR_BL_USB_CONNECTED           "USB verbunden"
#define TR_BL_USB_PLUGIN              "oder USB-Kabel anschl."
#define TR_BL_USB_MASS_STORE          "für den SD-Speicher an"
#define TR_BL_USB_PLUGIN_MASS_STORE   "Oder schließen Sie ein USB-Kabel für den SD-Speicher an"
#define TR_BL_WRITE_FW                "Schreibe Firmware"
#define TR_BL_FORK                    "Fork:"
#define TR_BL_VERSION                 "Version:"
#define TR_BL_RADIO                   "Radio:"
#define TR_BL_EXIT                    "Beenden"
#define TR_BL_DIR_MISSING             " Verzeichnis fehlt"
#define TR_BL_DIR_EMPTY               " Verzeichnis leer"
#define TR_BL_WRITING_FW              "Schreibe..."
#define TR_BL_WRITING_COMPL           TR("Schreiben fertig","Schreiben abgeschlossen")

#if LCD_W >= 480
  #define TR_BL_INVALID_FIRMWARE       "Keine gültige Firmwaredatei"
#elif LCD_W >= 212
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN_MASS_STORE
  #define TR_BL_HOLD_ENTER_TO_START    "\012Halte [ENT] gedrückt, um mit dem Schreiben zu beginnen"
  #define TR_BL_INVALID_FIRMWARE       "\011Keine gültige Firmwaredatei!        "
  #define TR_BL_INVALID_EEPROM         "\011Keine gültige EEPROM Datei!         "
#else
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
  #define TR_BL_HOLD_ENTER_TO_START    "Zum Starten [ENT] halten"
  #define TR_BL_INVALID_FIRMWARE       "Keine gültige Firmware!"
  #define TR_BL_INVALID_EEPROM         "Kein gültiges EEPROM!"
#endif

#if defined(PCBTARANIS)
   // Bootloader Taranis specific - ASCII characters only
  #define TR_BL_RESTORE_EEPROM        "EEPROM wiederherstellen"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Drücke den Power Knopf."
    #define TR_BL_FLASH_EXIT          "Verlasse den Flashmodus."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - ASCII characters only
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_SELECT_KEY            "[ENT] um Datei auszuwählen"
  #define TR_BL_FLASH_KEY             "Halte [ENT] gedrückt, zum schreiben"
  #define TR_BL_ERASE_KEY             "Hold [ENT] long to erase"
  #define TR_BL_EXIT_KEY              "[RTN] zum beenden"
#elif defined(PCBPL18)
   // Bootloader PL18/NB4+ specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB Zugriff"
  #define TR_BL_CURRENT_FW            "Aktuelle Firmware:"
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_ENABLE                "Aktivieren"
  #define TR_BL_DISABLE               "Deaktivieren"
  #if defined(RADIO_NV14_FAMILY)
    #define TR_BL_SELECT_KEY          "[R TRIM] um Datei auszuwählen"
    #define TR_BL_FLASH_KEY           "Halte [R TRIM] gedrückt, zum schreiben"
    #define TR_BL_ERASE_KEY           "Halte [R TRIM] gedrückt, to erase"
    #define TR_BL_EXIT_KEY            "[L TRIM] zum beenden"
  #elif defined(RADIO_NB4P)
    #define TR_BL_SELECT_KEY          "[SW1A] um Datei auszuwählen"
    #define TR_BL_FLASH_KEY           "Halte [SW1A] gedrückt, zum schreiben"
    #define TR_BL_ERASE_KEY           "Halte [SW1A] gedrückt, to erase"
    #define TR_BL_EXIT_KEY            "[SW1B] zum beenden"
  #else
    #define TR_BL_SELECT_KEY          "[TR4 Dn] um Datei auszuwählen"
    #define TR_BL_FLASH_KEY           "Halte [TR4 Dn] gedrückt, zum schreiben"
    #define TR_BL_ERASE_KEY           "Halte [TR4 Dn] gedrückt, to erase"
    #define TR_BL_EXIT_KEY            "[TR4 Up] zum beenden"
  #endif
#elif defined(PCBNV14)
   // Bootloader NV14 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB Zugriff"
  #define TR_BL_CURRENT_FW            "Aktuelle Firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] um Datei auszuwählen"
  #define TR_BL_FLASH_KEY             "Halte [R TRIM] gedrückt, zum schreiben"
  #define TR_BL_EXIT_KEY              " [L TRIM] zum beenden"
  #define TR_BL_ENABLE                "Aktivieren"
  #define TR_BL_DISABLE               "Deaktivieren"
#endif

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


// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS            { "Vergleich oder Funktion", "Erste Variable", "Zweite Variable/Konstante", "Zweite Variable/Konstante", "Weitere UND Bedingung für Freigabe des Log Schalters", "ON-Zeit des Log Schalters wenn Bedingung ok", "Mindestdauer der Bedingung damit Log Schalter ON geht" }

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
#define TR_FIRMWARE_OPTIONS            "Firmwareoptionen"

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
#define TR_USER_INTERFACE              "Benutzerschnittstelle"
#define TR_THEME                       "Theme"
#define TR_SETUP                       "Einrichten"
#define TR_LAYOUT                      "Layout"
#define TR_ADD_MAIN_VIEW               "Ansicht hinzufügen"
#define TR_TEXT_COLOR                  "Textfarbe"
// ----------------------------- Symbole für Auswahlliste----------
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Inputs"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua Skripte"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Knüppel"
#define TR_MENU_POTS                   STR_CHAR_POT "Potis"
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "Heli-TS CYC1-3"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Trimmung"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Schalter"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Log. Schalter"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Trainer"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Kanäle"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "Glob. Vars"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Telemetrie"
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
#if !PORTRAIT_LCD
  #define TR_OPEN_CHANNEL_MONITORS        "Öffne Kanalmonitor"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Öffne Kanalmon."
#endif
#define TR_DUPLICATE                   "Duplizieren"
#define TR_ACTIVATE                    "Aktivieren"
#define TR_RED                         "Rot"
#define TR_BLUE                        "Blau"
#define TR_GREEN                       "Grün"
#define TR_COLOR_PICKER                "Farbauswahl"
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
#define TR_THEME_EDITOR                "THEMES"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL      "Wähle\nModell"
#define TR_MAIN_MENU_MANAGE_MODELS     "Modell\nManager"
#define TR_MAIN_MENU_MODEL_NOTES       "Modell\nNotizen"
#define TR_MAIN_MENU_CHANNEL_MONITOR   "Kanal\nMonitor"
#define TR_MAIN_MENU_MODEL_SETTINGS    "Modell\nKonfig."
#define TR_MAIN_MENU_RADIO_SETTINGS    "Sender\nKonfig."
#define TR_MAIN_MENU_SCREEN_SETTINGS   "Seiten\nKonfig."
#define TR_MAIN_MENU_RESET_TELEMETRY   "Reset\nTelemetrie"
#define TR_MAIN_MENU_STATISTICS        "Statistiken"
#define TR_MAIN_MENU_ABOUT_EDGETX      "Über\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH               "Englisch"
#define TR_VOICE_CHINESE               "Chinesisch"
#define TR_VOICE_CZECH                 "Tschechisch"
#define TR_VOICE_DANISH                "Dänisch"
#define TR_VOICE_DEUTSCH               "Deutsch"
#define TR_VOICE_DUTCH                 "Holländisch"
#define TR_VOICE_ESPANOL               "Spanisch"
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

#define TR_USBJOYSTICK_LABEL           "USB Joystick"
#define TR_USBJOYSTICK_EXTMODE         "Modus"
#define TR_VUSBJOYSTICK_EXTMODE        "Klassisch","Erweitert"
#define TR_USBJOYSTICK_SETTINGS        "Kanal Einstellungen"
#define TR_USBJOYSTICK_IF_MODE         TR("If.Modus","Interface Modus")
#define TR_VUSBJOYSTICK_IF_MODE        "Joystick","Gamepad","MultiAchsen"
#define TR_USBJOYSTICK_CH_MODE         "Modus"
#define TR_VUSBJOYSTICK_CH_MODE        "Kein","Tasten","Achse","Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Tasten Modus"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "Normal","Puls","SWEmu","Delta","Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Norm","Normal"),TR("Puls","Puls"),TR("SWEm","SWEmul"),TR("Delt","Delta"),TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Positionen"
#define TR_VUSBJOYSTICK_CH_SWPOS       "Drücken","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Achse"
#define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","rotX","rotY","rotZ","Schieber","Dial","Rad"
#define TR_USBJOYSTICK_CH_SIM          "Sim Achse"
#define TR_VUSBJOYSTICK_CH_SIM         "Quer","Höhe","Seite","Gas","Beschl.","Bremsen","Lenkung","Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Invers"
#define TR_USBJOYSTICK_CH_BTNNUM       "Tastennr."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Tastennr. Kollision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Achsen Kollision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "kein","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Änd. übernehmen"

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

#define TR_SORT_ORDERS            "Name A-Z","Name Z-A","Wenig benutzt","Meist benutzt"
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
