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

// NON ZERO TERMINATED STRINGS
#define TR_OFFON                       "AUS","EIN"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Stumm","Alarm","NoKey","Alle"
#define TR_VBLMODE                     "AUS","Taste","Stks","Beide","EIN"
#define TR_TRNMODE                     "AUS","+=",":="
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES            "AUS","Telem Mirror","Telemetry In","SBUS Eingang","LUA","CLI","GPS","Debug"
#define TR_SWTYPES                     "Kein","Taster","2POS","3POS"
#define TR_POTTYPES                    "Kein",TR("Poti m.Ras","Poti mit Raste"),TR("Stufens.","Stufen-Schalter"),TR("Pot","Poti ohne Raste")
#define TR_SLIDERTYPES                 "Keine","Slider"
#define TR_VLCD                        "Normal","Optrex"
#define TR_VPERSISTENT                 "AUS","Flugzeit","Manuell Ruck"
#define TR_COUNTRY_CODES               TR("US","Amerika"),TR("JP","Japan"),TR("EU","Europa")
#define TR_USBMODES                    "Fragen",TR("Joyst","Joystick"),TR("SDCard","Speicher"),TR("Serial","Seriell")
#define TR_JACK_MODES                  "Popup","Audio","Trainer"
#define TR_TELEMETRY_PROTOCOLS         "FrSky S.PORT","FrSky D","FrSky D (Kabel)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_TELEMETRY_MODE        "Off","On","Off+Aux","On+Aux"
#define TR_MULTI_CUSTOM                "Custom"
#define TR_VTRIMINC                    TR("Expo","Exponentiell"),TR("ExFein","Extrafein"),"Fein","Mittel","Grob"
#define TR_VDISPLAYTRIMS               "Nein","Kurz","Ja"  // Trimmwerte Keine, kurze Anzeigen, Ja
#define TR_VBEEPCOUNTDOWN              "Kein","Pieps","Stimme","Haptik"
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VVARIOCENTER                "Ton","Ruhe"
#define TR_CURVE_TYPES                 "Nur Y","X und Y" // "Standard","Custom"

#define TR_ADCFILTERVALUES           "Global","Aus","Ein"

#if defined(PCBX10)
  #define TR_RETA123                   "S","H","G","Q","1","3","2","4","5","6","7","L","R"
#elif defined(PCBHORUS)
  #define TR_RETA123                   "S","H","G","Q","1","3","2","4","5","L","R"
#elif defined(PCBX9E)
  #define TR_RETA123                   "S","H","G","Q","1","2","3","4","L","R","L","R"
#elif defined(PCBTARANIS)
  #define TR_RETA123                   "S","H","G","Q","1","2","3","L","R"
#else
  #define TR_RETA123                   "S","H","G","Q","1","2","3"
#endif

#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Addiere","Multipl.","Ersetze"
#define TR_VMLTPX2                     "+=","*=",":="

#if defined(PCBHORUS)
  #define TR_VMIXTRIMS                 "AUS","EIN","Sei","Höh","Gas","Que","T5","T6"
#else
  #define TR_VMIXTRIMS                 "AUS","EIN","Sei","Höh","Gas","Que"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Takt"  // TIM = Takt = Taktgenerator
  #define TR_CSWSTICKY                 "SRFF"  // Sticky = RS-Flip-Flop
  #define TR_CSWRANGE                  "Range" // Range = Bereichsabfrage von bis
  #define TR_CSWSTAY                   "Puls"  // Edge = einstellbarer Impuls
#else
  #define TR_CSWTIMER                  "Takt"  // TIM = Takt = Taktgenerator
  #define TR_CSWSTICKY                 "SRFF"  // Sticky = RS-Flip-Flop
    #define TR_CSWRANGE                "Rnge"  // Range= Bereichsabfrage von bis
    #define TR_CSWSTAY                 "Puls"  // Edge = einstellbarer Impuls
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC            			"---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#if defined(VARIO)
  #define TR_VVARIO                    "Vario"
#else
  #define TR_VVARIO                    "[Vario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND                     "Spiel Töne"
#else
  #define TR_SOUND                     "Spiel"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC                    "Haptik"
#else
  #define TR_HAPTIC                    "[Haptik]"
#endif

#if defined(VOICE)
  #define TR_PLAY_TRACK                "Sag Text"
  #define TR_PLAY_BOTH                 "Sag Beide"
  #define TR_PLAY_VALUE                "Sag Wert"
#else
  #define TR_PLAY_TRACK                "[Sag Text]"
  #define TR_PLAY_BOTH                 "[SagBeide]"
  #define TR_PLAY_VALUE                "[Sag Wert]"
#endif

#define TR_SF_BG_MUSIC                 "StartMusik","Stop Musik"

#if defined(SDCARD)
  #define TR_SDCLOGS                   "SD-Aufz."
#else
  #define TR_SDCLOGS                   "[SD Aufz.]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR               "Ändere"
#else
  #define TR_ADJUST_GVAR               "[ÄndereGV]"
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

#if defined(OVERRIDE_CHANNEL_FUNCTION) && LCD_W >= 212
  #define TR_SF_SAFETY                 "Override"
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "Overr."
#else
  #define TR_SF_SAFETY                 "---"
#endif

#define TR_SF_SCREENSHOT               "Screenshot"
#define TR_SF_RACING_MODE              "RacingMode"
#define TR_SF_DISABLE_TOUCH            "Kein Touch"
#define TR_SF_RESERVE                  "[Reserve]"

#define TR_VFSWFUNC                    TR_SF_SAFETY,"Lehrer","Inst. Trim","Rücksetz.","Setze",TR_ADJUST_GVAR,"Lautstr.","SetFailsafe","RangeCheck","ModuleBind",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,"LCD Licht",TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH,TR_SF_TEST

#define TR_FSW_RESET_TELEM             TR("Telm","Telemetrie")

#if LCD_W >= 212
 #define TR_FSW_RESET_TIMERS           "Timer 1","Timer 2","Timer 3"
#else
 #define TR_FSW_RESET_TIMERS           "Tmr1","Tmr2","Tmr3"
#endif

#define TR_VFSWRESET 	      		   TR_FSW_RESET_TIMERS,"All",TR_FSW_RESET_TELEM

#define TR_FUNCSOUNDS         		   TR("Bp1","Piep1"),TR("Bp2","Piep2"),TR("Bp3","Piep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"
#define TR_VUNITSSYSTEM                TR("Metrik","Metrisch"),TR("Imper.","Imperial")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "None","Werte","Balken","Script"
#define TR_GPSFORMAT                   "GMS","NMEA"  // Koordinatenanzeige

#define LEN2_VTEMPLATES                15  // max String Länge für Menü (original=13)
#define TR_TEMPLATE_CLEAR_MIXES        "Misch. Lösch."
#define TR_TEMPLATE_SIMPLE_4CH         "Einfach. 4-CH"
#define TR_TEMPLATE_STICKY_TCUT        "Fixe Gassperre"
#define TR_TEMPLATE_VTAIL              "V-Leitwerk"
#define TR_TEMPLATE_DELTA              "Delta Mischer"
#define TR_TEMPLATE_ECCPM              "eCCPM"
#define TR_TEMPLATE_HELI               "Hubschrauber"
#define TR_TEMPLATE_SERVO_TEST         "Servo Tester"

#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICKS_VSRCRAW              STR_CHAR_STICK "Sei",STR_CHAR_STICK "Höh",STR_CHAR_STICK "Gas",STR_CHAR_STICK "Que"

#if defined(PCBHORUS)
  #define TR_TRIMS_VSRCRAW             STR_CHAR_TRIM "Sei",STR_CHAR_TRIM "Höh",STR_CHAR_TRIM "Gas",STR_CHAR_TRIM "Que",STR_CHAR_TRIM "T5",STR_CHAR_TRIM "T6"
#else
  #define TR_TRIMS_VSRCRAW             TR("TrmS",STR_CHAR_TRIM "Sei"),TR("TrmH",STR_CHAR_TRIM "Höh"),TR("TrmG",STR_CHAR_TRIM "Gas"),TR("TrmQ",STR_CHAR_TRIM "que")
#endif

#if defined(PCBHORUS)
  #define TR_TRIMS_SWITCHES            STR_CHAR_TRIM "Rl",STR_CHAR_TRIM "Rr",STR_CHAR_TRIM "Ed",STR_CHAR_TRIM "Eu",STR_CHAR_TRIM "Td",STR_CHAR_TRIM "Tu",STR_CHAR_TRIM "Al",STR_CHAR_TRIM "Ar",STR_CHAR_TRIM "5d",STR_CHAR_TRIM "5u",STR_CHAR_TRIM "6d",STR_CHAR_TRIM "6u"
#else
  #define TR_TRIMS_SWITCHES    			TR("tRl",STR_CHAR_TRIM "Rl"),TR("tRr",STR_CHAR_TRIM "Rr"),TR("tEd",STR_CHAR_TRIM "Ed"),TR("tEu",STR_CHAR_TRIM "Eu"),TR("tTd",STR_CHAR_TRIM "Td"),TR("tTu",STR_CHAR_TRIM "Tu"),TR("tAl",STR_CHAR_TRIM "Al"),TR("tAr",STR_CHAR_TRIM "Ar")
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

#define TR_ON_ONE_SWITCHES             "ON","One"

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
#define TR_CYC_VSRCRAW                 "CYC1","CYC2","CYC3"
#else
#define TR_CYC_VSRCRAW                 "[C1]","[C2]","[C3]"
#endif

#define TR_RESERVE_VSRCRAW             "[--]"
#define TR_EXTRA_VSRCRAW               "Batt","Time","GPS",TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,"Tmr1","Tmr2","Tmr3"

#define TR_VTMRMODES                   "AUS","EIN","GSs","GS%","GSt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
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
#define TR_VCELLINDEX                  "Niedrigst","1. Zelle","2. Zelle","3. Zelle","4. Zelle","5. Zelle","6. Zelle","Höchster","Differenz"
#define TR_TEXT_SIZE                   "Standard","Tiny","Small","Mid","Double"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA" (center only)","= (symetrical)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")

// ZERO TERMINATED STRINGS
#if defined(COLORLCD)
  #define INDENT                       "   "
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

#if defined(PCBFRSKY)
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
  #define TR_OK                        "\010" "\010" "\010" TR("[OK]", "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)

#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_FREE                        "frei"
#define TR_YES                         "Yes"
#define TR_NO                          "No"
#define TR_DELETEMODEL                 "Modell löschen?"
#define TR_COPYINGMODEL                "Kopiere Modell"
#define TR_MOVINGMODEL                 "Schiebe Modell"
#define TR_LOADINGMODEL                "Lade Modell..."
#define TR_NAME                        "Name"
#define TR_MODELNAME                   "Modellname"
#define TR_PHASENAME                   "Phase-Name"
#define TR_MIXNAME                     "Mix-Name"
#define TR_INPUTNAME                   TR("Input", "Input name")
#define TR_EXPONAME                    TR("Name", "Line name")
#define TR_BITMAP                      "Modellfoto"
#define TR_TIMER                       "Timer"
#define TR_START                       "Start"
#define TR_ELIMITS                     TR("Erw. Limit", "Erw. Wege auf 150%")
#define TR_ETRIMS                      TR("Erw. Trims", "Erw. Trim  auf 100%")
#define TR_TRIMINC                     TR("Trimschritt", "Trimmschritte")
#define TR_DISPLAY_TRIMS               TR("Trimanzeige", "Trimwerte anzeigen")
#define TR_TTRACE                      TR("Gasquelle", INDENT "Gas-Timerquelle")
#define TR_TTRIM 	       	           TR("Gastrim", INDENT "Gas-Leerlauftrim")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", INDENT "Trim switch")
#define TR_BEEPCTR                     TR("MittePieps", "Mittelstell. -Pieps")
#define TR_USE_GLOBAL_FUNCS            TR("Glob. Funkt.", "Globale Funkt verw.")
#define TR_PROTOCOL          		   TR("Protok.", "Protokoll")
#define TR_PPMFRAME          	  	   INDENT "PPM-Frame"
#define TR_REFRESHRATE             	   TR(INDENT "Refresh", INDENT "Refresh rate")
#define STR_WARN_BATTVOLTAGE           TR(INDENT "Output is VBAT: ", INDENT "Warning: output level is VBAT: ")
#define TR_WARN_5VOLTS                 "Warning: output level is 5 volts"
#define TR_MS                 		   "ms"
#define TR_FREQUENCY                   INDENT "Frequency"
#define TR_SWITCH                      TR("Schalt.", "Schalter")
#define TR_TRIMS                       "Trims"
#define TR_FADEIN                      "Langs. Ein"
#define TR_FADEOUT                     "Langs. Aus"
#define TR_DEFAULT                     "(Normal)"
#define TR_CHECKTRIMS                  CENTER"\006Check\012Trims"
#define OFS_CHECKTRIMS                 CENTER_OFS+(9*FW)
#define TR_SWASHTYPE                   TR("Typ Taumelsch", "Typ  Taumelscheibe")
#define TR_COLLECTIVE                  TR("Kollekt. Pitch", "Kollekt. Pitch Quelle")
#define TR_AILERON                     "Roll Quelle"
#define TR_ELEVATOR                    "Nick Quelle"
#define TR_SWASHRING                   TR("Ring   Begrenz", "Ring Begrenzung")
#define TR_ELEDIRECTION                TR("Nick   Richtung", "Nick   Servo Richtung")
#define TR_AILDIRECTION                TR("Roll   Richtung", "Roll   Servo Richtung")
#define TR_COLDIRECTION                TR("Pitch Richtung", "Pitch Servo Richtung")
#define TR_MODE                        "Modus"
#define TR_SUBTYPE                     INDENT "Subtype"
#define TR_NOFREEEXPO                  "Expos voll!"
#define TR_NOFREEMIXER                 "Mischer voll!"
#define TR_SOURCE                      "Quelle"
#define TR_WEIGHT                      "Gewicht"
#define TR_EXPO                        TR("Expo", "Exponential")
#define TR_SIDE                        "Seite"
#define TR_DIFFERENTIAL                "Diff"
#define TR_OFFSET                      "Offset"
#define TR_TRIM                        "Trim"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Kurve"
#define TR_FLMODE                      TR("Phase", "Phasen")
#define TR_MIXWARNING                  "Warnung"
#define TR_OFF                         "AUS"
#define TR_ANTENNA                     "Antenna"
#define TR_NO_INFORMATION              TR("No info", "No information")
#define TR_MULTPX                      "Wirkung"
#define TR_DELAYDOWN                   "Verz. Dn"
#define TR_DELAYUP                     "Verz. Up"
#define TR_SLOWDOWN                    "Langs.Dn"
#define TR_SLOWUP                      "Langs.Up"
#define TR_MIXES                       "MISCHER"
#define TR_CV                          "KV"
#if defined(PCBNV14)
#define TR_GV                          "GV"
#else
#define TR_GV                          TR("G", "GV")
#endif
#define TR_ACHANNEL                    TR("A\004gemessen", "A\004Kanal gemessen =>") // 9XR-Pro
#define TR_RANGE                       TR(INDENT "Bereich", INDENT "Variobereich m/s") // 9XR-Pro
#define TR_CENTER                      TR(INDENT "Mitte", INDENT "Variomitte     m/s")
#define TR_BAR                         "Balken"
#define TR_ALARM                       TR("Alarme ", "Aufleuchten bei Alarm")  // 9XR-Pro
#define TR_USRDATA                     "Daten berechnen aus"
#define TR_BLADES                      TR("Prop", "Prop-Blätter") // 9XR-Pro
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
#define TR_BATTERY_RANGE               TR("Akku Bereich", "Akku Spannungsbereich") // Symbol Akku Ladezustand
#define TR_BATTERYCHARGING             "Charging..."
#define TR_BATTERYFULL                 "Battery full"
#define TR_BATTERYNONE                 "None!"
#define TR_BATTERYWARNING              TR("Akku Warnung", "Akkuspannungswarnung")
#define TR_INACTIVITYALARM             TR("Inaktivität", "Inaktivität nach")
#define TR_MEMORYWARNING               "Speicher voll"
#define TR_ALARMWARNING                TR("Alle Töne aus?", "Alle Töne ganz aus?")
#define TR_RSSI_SHUTDOWN_ALARM         TR(INDENT "RSSI-Chk  aus?", INDENT "Prüfe RSSI bei Ausschalten")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_MODEL_STILL_POWERED         "Model still powered"
#define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_MODEL_SHUTDOWN              "Shutdown ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Press enter to confirm"
#define TR_THROTTLE_LABEL              "Gas-Kontrolle"
#define TR_THROTTLEREVERSE             TR("Gas invers", INDENT "Vollgas hinten?") //Änderung wg TH9x, Taranis
#define TR_MINUTEBEEP                  TR("Min-Alarm", "Minuten-Alarm")
#define TR_BEEPCOUNTDOWN               INDENT "Countdown"
#define TR_PERSISTENT                  TR(INDENT "Permanent", INDENT "Permanent")
#define TR_BACKLIGHT_LABEL             "LCD-Beleuchtung"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLDELAY                     INDENT "Dauer"
#define TR_BLONBRIGHTNESS              INDENT "An-Helligkeit"
#define TR_BLOFFBRIGHTNESS             INDENT "Aus-Helligkeit"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR                     "Farbe"
#define TR_SPLASHSCREEN                TR("Startbild Ein", "Startbild Anzeigedauer")
#define TR_PWR_ON_DELAY                "Pwr On delay"
#define TR_PWR_OFF_DELAY               "Pwr Off delay"
#define TR_THROTTLE_WARNING            TR("Gasalarm", INDENT "Gas Alarm")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Cust-Pos", INDENT INDENT INDENT INDENT "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR("Sch. Alarm", INDENT "Schalter-Alarm")
#define TR_POTWARNINGSTATE             TR(INDENT "Pot&Slid.", INDENT "Pots & sliders")
#define TR_SLIDERWARNING               TR(INDENT "Slid. pos.", INDENT "Slider positions")
#define TR_POTWARNING                  TR("Potiwarnung", INDENT "Poti-Warnung")
#define TR_TIMEZONE                    TR("Zeitzone", "GPS-Zeitzone +/-Std")
#define TR_ADJUST_RTC                  TR("GPSzeit setzen", "Uhrzeit per GPS setzen")
#define TR_GPS                         "GPS"
#define TR_RXCHANNELORD                TR("Kanal CH1-4", "Kanalvoreinstellung")
#define TR_STICKS                      "Knüppel"
#define TR_POTS                        "Potis"
#define TR_SWITCHES                    "Schalter"
#define TR_SWITCHES_DELAY              TR("Sw. Mitte Delay", "Schaltermitte Verzögerung")   //Schalter Mitten verzögern Anpassung
#define TR_SLAVE                       TR("Schüler PPM1-16", "Schüler PPM1-16 als Ausgang")
#define TR_MODESRC                     " Modus\003%  Quelle"
#define TR_MULTIPLIER                  "Multiplik."
#define TR_CAL                         "Kal."
#define TR_VTRIM                       "Trim - +"
#define TR_BG                          "BG:"
#define TR_CALIB_DONE                  "Calibration completed"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Drücke [Enter] zum Start"
  #define TR_SETMIDPOINT               "Knüppel/Potis/Sliders auf Mitte [Enter]"
  #define TR_MOVESTICKSPOTS            "Knüppel/Potis/Sliders bewegen [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " Zum Start"
  #define TR_SETMIDPOINT               "STICKS/SLIDERS/POTIS auf Mitte"
  #define TR_MOVESTICKSPOTS            "STICKS/SLIDERS/POTIS bewegen"
  #define TR_MENUWHENDONE              TR_ENTER " wenn fertig"
#else
  #define TR_MENUTOSTART               CENTER "\010" TR_ENTER " Zum START"
  #define TR_SETMIDPOINT               TR(CENTER "\004Mitte Knüppel/Sliders", CENTER "\004Mitte Knüppel/Potis")
  #define TR_MOVESTICKSPOTS            CENTER "\004Bewege Knüppel/POTIS"
  #define TR_MENUWHENDONE              CENTER "\006" TR_ENTER " wenn fertig"
#endif
#define TR_RXBATT                      "Rx Akku:"
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_ACCEL                       "Acc:"
#define TR_NODATA                      CENTER"Keine Daten"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS         	       "Tmix max"
#define TR_FREE_STACK     		       "Freier Stack"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua scripts"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_TIMER_LABEL                 "Timer"
#define TR_THROTTLE_PERCENT_LABEL      "Throttle %"
#define TR_BATT_LABEL                  "Battery"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " für Reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODELL"
#define TR_FM                          "FP"
#define TR_MIX                         "MIX"
#define TR_EEPROMLOWMEM                "EEPROM voll"
#define TR_PRESS_ANY_KEY_TO_SKIP	   TR("Taste drücken",CENTER"Taste drücken")
#define TR_THROTTLE_NOT_IDLE           "Gas nicht Null!"
#define TR_ALARMSDISABLED              "Alarme ausgeschaltet"
#define TR_PRESSANYKEY                 TR("Taste drücken",CENTER"Taste drücken")
#define TR_BADEEPROMDATA               "EEPROM ungültig"
#define TR_BAD_RADIO_DATA              "Bad Radio Data"
#define TR_EEPROMFORMATTING            "EEPROM Initialisieren"
#define TR_STORAGE_FORMAT              "Speicher Vorbereiten"
#define TR_EEPROMOVERFLOW              "EEPROM Überlauf"
#define TR_RADIO_SETUP                 TR("SENDER-EINSTELLEN", "SENDER-GRUNDEINSTELLUNGEN")
#define TR_MENUDATEANDTIME             "DATUM UND ZEIT"
#define TR_MENUTRAINER                 TR("LEHRER/SCHÜLER", "LEHRER/SCHÜLER")
#define TR_MENUSPECIALFUNCS            "GLOBALE FUNKTIONEN"
#define TR_MENUVERSION                 "VERSION"
#define TR_MENU_RADIO_SWITCHES         TR("Schalter-Test", "Schalter-Test")
#define TR_MENU_RADIO_ANALOGS          "Geber-Test"
#define TR_MENU_RADIO_ANALOGS_CALIB    "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION             TR("KALIB. ANALOG", "KALIBRIERUNG-Analog")
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS             "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE           "Channels=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Channel=>Failsafe"
#define TR_MENUMODELSEL        		   TR("MODELLE", "MODELL WÄHLEN")
#define TR_MENU_MODEL_SETUP            TR("MODELL-EINSTELLUNG", "MODELL-EINSTELLUNGEN")
#define TR_MENUFLIGHTMODE    		   "FLUGPHASE"
#define TR_MENUFLIGHTMODES   		   "FLUGPHASEN"
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
#define TR_MENUTEMPLATES               "VORLAGEN"
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
#define TR_MULTI_RFTUNE                TR(INDENT "RF Freq.", INDENT "RF Freq. Feintuning")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY             "Telemetry"
#define TR_MULTI_VIDFREQ               TR(INDENT "Vid. Freq.", INDENT "Video Frequenz")
#define TR_RF_POWER                    INDENT "RF Power"
#define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")
#define TR_MULTI_OPTION                TR(INDENT "Option", INDENT "Optionswert")
#define TR_MULTI_AUTOBIND              TR(INDENT "Bind Ch.",INDENT "Bind on channel")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Disable Ch. map")
#define TR_DISABLE_TELEM               TR("No Telem", "Disable Telemetry")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "Autodetect", INDENT "Autodetect format")
#define TR_MULTI_LOWPOWER              TR(INDENT "Low power", INDENT "Low power mode")
#define TR_MULTI_LNA_DISABLE           INDENT "LNA disable"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")
#define TR_MODULE_TELEM_ON             TR("EIN", "Aktiviert")
#define TR_DISABLE_INTERNAL            TR("Disable int. RF", "Disable internal RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("!serial mode", "Not in serial mode")
#define TR_MODULE_NO_INPUT             TR("No input", "No serial input")
#define TR_MODULE_NO_TELEMETRY         TR3("Keine Telem.", "Keine MULTI_TELEMETRY", "Keine Telemetrie (aktiviere MULTI_TELEMETRY)")
#define TR_MODULE_WAITFORBIND          "Bind to load protocol"
#define TR_MODULE_BINDING              "Binding"
#define TR_MODULE_UPGRADE_ALERT        TR3("Upg. needed", "Module upgrade required", "Modul\nUpdate nötig")
#define TR_MODULE_UPGRADE              TR("Upg. advised", "Modulupdate empfohlen")
#define TR_REBIND                      "Rebinding required"
#define TR_REG_OK                      "Registration ok"
#define TR_BIND_OK                     "Bind successful"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem AN"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem AUS"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem AN"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem AUS"
#define TR_PROTOCOL_INVALID            TR("Prot. invalid", "Protocol invalid")
#define TR_MODULE_STATUS               TR(INDENT "Status", INDENT "Module Status")
#define TR_MODULE_SYNC                 TR(INDENT "Sync", INDENT "Proto Sync Status")
#define TR_MULTI_SERVOFREQ             TR(INDENT "Servo rate", INDENT "Servo update rate")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Aktiviere max. throw")
#define TR_MULTI_RFCHAN                TR("RF Channel", "Select RF channel")
#define TR_SYNCMENU                    "Sync [MENU]"
#define TR_LIMIT                       INDENT "Grenzen"
#define TR_MINRSSI                     "Min. RSSI"
#define TR_LATITUDE                    "Breite:"
#define TR_LONGITUDE                   "Länge:"
#define TR_GPS_COORDS_FORMAT           TR("GPS-Koord.", "GPS-Koordinaten-Format")
#define TR_VARIO                       "Variometer"
#define TR_PITCH_AT_ZERO               "Töne sinken"
#define TR_PITCH_AT_MAX                "Töne steigen"
#define TR_REPEAT_AT_ZERO              "Wiederholrate"
#define TR_SHUTDOWN                    "Herunterfahren"
#define TR_SAVE_MODEL                  "Modelleinstellungen speichern"
#define TR_BATT_CALIB                  TR("AkkuSpgwert", "Akku Kalibrierung")
#define TR_CURRENT_CALIB               "Strom abgl."
#define TR_VOLTAGE                     TR(INDENT "Spg", INDENT "Spannungsquelle")  //9XR-Pro
#define TR_CURRENT                     TR(INDENT "Strom", INDENT "Stromquelle")
#define TR_SELECT_MODEL                "Modell auswählen"
#define TR_MODEL_CATEGORIES            "Model Categories"
#define TR_MODELS                      "Models"
#define TR_SELECT_MODE                 "Select mode"
#define TR_CREATE_CATEGORY             "Modelltyp erstellen"
#define TR_RENAME_CATEGORY             "Modelltyp umbenennen"
#define TR_DELETE_CATEGORY             "Modelltyp löschen"
#define TR_CREATE_MODEL                TR("Neues Modell" , "Neues Modell erstellen")
#define TR_DUPLICATE_MODEL             "Kopiere Modell"
#define TR_COPY_MODEL                  "Kopiere Modell"
#define TR_MOVE_MODEL                  "Verschiebe Modell"
#define TR_BACKUP_MODEL                "Modell auf SD-Karte"  //9XR-Pro
#define TR_DELETE_MODEL                "Lösche Modell" // TODO merged into DELETEMODEL?
#define TR_RESTORE_MODEL               TR("Modell wiederher.", "Modell wiederherstellen")
#define TR_DELETE_ERROR                "Fehler beim\nLöschen"
#define TR_CAT_NOT_EMPTY               "Modelltyp nicht leer"
#define TR_SDCARD_ERROR                "SD-Kartenfehler"
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "No files on SD!"
#define TR_NO_SDCARD                   "Keine SD-Karte"
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
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH ohne Telem.", "16CH ohne Telemetry")
#define TR_16CH_WITH_TELEMETRY         TR("16CH mit Telem.", "16CH mit Telemetry")
#define TR_8CH_WITH_TELEMETRY          TR("8CH mit Telem.", "8CH mit Telemetry")
#define TR_EXT_ANTENNA                 "Ext. Antenne"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Update RX options?"
#define TR_UPDATE_TX_OPTIONS           "Update TX options?"
#define TR_MODULES_RX_VERSION          "Module / RX version"
#define TR_MENU_MODULES_RX_VERSION     "MODULE / RX VERSION"
#define TR_MENU_FIRM_OPTIONS           "FIRMWARE OPTIONEN"
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
#define TR_SDCARD_FULL         		   "SD-Karte voll"
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE        		   "Nicht kompatibel"
#define TR_WARNING                     "WARNUNG"
#define TR_EEPROMWARN                  "EEPROM"
#define TR_STORAGE_WARNING             "SPEICHER"
#define TR_EEPROM_CONVERTING           "EEPROM Konvertierung"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD card conversion required"
#define TR_CONVERTING                  "Converting: "
#define TR_THROTTLE_UPPERCASE          "GAS"
#define TR_ALARMSWARN                  "ALARM"
#define TR_SWITCHWARN                  "SCHALTER"
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE                "Use for tests only"
#define TR_WRONG_SDCARDVERSION         TR("Erw. Version: ","Erwartete Version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery low"
#define TR_WARN_MULTI_LOWPOWER         "Low power mode"
#define TR_BATTERY                     "BATTERY"
#define TR_WRONG_PCBREV                "Falsche PCB erkannt"
#define TR_EMERGENCY_MODE              "NOTFALL MODUS"
#define TR_PCBREV_ERROR                "PCB Problem"
#define TR_NO_FAILSAFE                 TR("Failsafe not set", "Failsafe nicht programmiert")
#define TR_KEYSTUCK                    "Taste klemmt"  //Key stuck=Taste klemmt
#define TR_INVERT_THR                  TR("Gas umkehren?", "Vollgas hinten?") // Th9x 9XR
#define TR_VOLUME                      "Lautstärke"
#define TR_LCD                         "Bildschirm"
#define TR_BRIGHTNESS                  "Helligkeit"
#define TR_CPU_TEMP                    "CPU-Temp.\016>"
#define TR_CPU_CURRENT                 "Strom\022>"
#define TR_CPU_MAH                     "Verbrauch"
#define TR_COPROC                      "CoProz."
#define TR_COPROC_TEMP                 "MB Temp. \016>"
#define TR_CAPAWARNING                 INDENT "Kapaz. niedrig" // wg 9XR-Pro
#define TR_TEMPWARNING                 INDENT "Temp.   größer"  //wg 9XR-Pro
#define TR_TTL_WARNING                 "Warning: use 3.3V logic levels"
#define TR_FUNC                        "Funktion"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Dauer"
#define TR_DELAY                       "Verzög."
#define TR_SD_CARD                     "SD-Karte"
#define TR_SDHC_CARD                   "SDHC-Karte"
#define TR_NO_SOUNDS_ON_SD             "Keine Töne" BREAKSPACE "auf SD"
#define TR_NO_MODELS_ON_SD             "Kein Modelle" BREAKSPACE "auf SD"
#define TR_NO_BITMAPS_ON_SD            "Keine Bitmaps" BREAKSPACE "auf SD"
#define TR_NO_SCRIPTS_ON_SD            "Keine Skripte" BREAKSPACE "auf SD"
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
#define TR_SD_FORMAT                   "Formatieren"
#define TR_NA                          "N/V"	//NV=Nicht Verfügbar  Kurz-Meldung
#define TR_HARDWARE                    TR("Hardware einst. ", "Namen und Hardware einst.")
#define TR_FORMATTING                  "Formatierung..."
#define TR_TEMP_CALIB                  "Temp.  abgl."
#define TR_TIME                        "Uhrzeit:"
#define TR_BAUDRATE                    "Baudrate"
#define TR_MAXBAUDRATE                 "Max Baud"
#define TR_SAMPLE_MODE                 "Sample Mode"

#define TR_SAMPLE_MODES                "Normal","OneBit"

#define TR_SELECT_TEMPLATE_FOLDER      "WÄHLE VORLAGENVERZEICHNIS:"
#define TR_SELECT_TEMPLATE             "WÄHLE MODELLVORLAGE:"
#define TR_NO_TEMPLATES                "Es wurden keine Modellvorlagen in diesem Verzeichnis gefunden"
#define TR_SAVE_TEMPLATE               "Als Vorlage speichern"
#define TR_BLANK_MODEL                 "Leeres Modell"
#define TR_BLANK_MODEL_INFO            "Erzeuge leeres Modell"
#define TR_FILE_EXISTS                 "DATEI EXISTIERT SCHON"
#define TR_ASK_OVERWRITE               "Möchten Sie überschreiben?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Discover"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Dist addr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Local addr"
#define TR_BLUETOOTH_PIN_CODE          "PIN Code"
#define TR_BLUETOOTH_NODEVICES         "No Devices Found"
#define TR_BLUETOOTH_SCANNING          "Scanning..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Aktiviert"
#else
#define TR_BLUETOOTH_MODES             "---","Telemetrie","Trainer"
#endif
#define TR_SD_INFO_TITLE               "SD-INFO"
#define TR_SD_TYPE                     "Typ:"
#define TR_SD_SPEED                    "Geschw:"
#define TR_SD_SECTORS                  "Sectoren:"
#define TR_SD_SIZE                     "Größe:"
#define TR_TYPE                        INDENT "Typ"
#define TR_GLOBAL_VARS                 "Globale Variablen"
#define TR_GVARS                       "GLOBALE V."
#define TR_GLOBAL_VAR                  "Globale Variable"
#define TR_MENU_GLOBAL_VARS            "GLOBALE VARIABLEN"
#define TR_OWN                         "Eigen"
#define TR_DATE                        "Datum:"
#define TR_MONTHS                      { "Jan", "Feb", "Mar", "Apr", "Mai", "Jun", "Jul", "Aug", "Sep", "Okt", "Nov", "Dez" }
#define TR_ROTARY_ENCODER              "Drehgeber"
#define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR            "Kanal-Monitor==>"
#define TR_MIXERS_MONITOR              "==>Mischer Monitor"
#define TR_PATH_TOO_LONG               "Pfad zu Lang"
#define TR_VIEW_TEXT                   "View Text"
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
#define TR_INTERNAL_MODULE             TR("Int. module","Internal module")
#define TR_EXTERNAL_MODULE             TR("Ext. module","External module")
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX upgrade nötig"
#define TR_TELEMETRY_DISABLED          "Telem. disabled"
#define TR_MORE_OPTIONS_AVAILABLE      "More options available"
#define TR_NO_MODULE_INFORMATION       "No module information"
#define TR_EXTERNALRF                  "Externes HF-Modul"
#define TR_FAILSAFE                    TR(INDENT "Failsafe", INDENT "Failsafe Mode")
#define TR_FAILSAFESET                 "Failsafe setzen"
#define TR_REG_ID                      TR("Reg. ID", "Registration ID")
#define TR_OWNER_ID                    "Owner ID"
#define TR_PINMAPSET                   "PINMAP"
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
#define TR_STOPPING                    "Stopping..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
#define TR_MENU_POWER_METER            "POWER METER"
#define TR_SENSOR                      "SENSOR"
#define TR_COUNTRY_CODE                "Landescode"
#define TR_USBMODE                     "USB Modus"
#define TR_JACK_MODE                   "Jack Mode"
#define TR_VOICE_LANGUAGE              "Sprachansagen"
#define TR_UNITS_SYSTEM                "Einheiten"
#define TR_EDIT                        "Zeile Editieren"
#define TR_INSERT_BEFORE               "Neue Zeile davor"
#define TR_INSERT_AFTER                "Neue Zeile danach"
#define TR_COPY                        "Zeile kopieren"
#define TR_MOVE                        "Zeile verschieben"
#define TR_PASTE                       "Zeile einfügen"
#define TR_PASTE_AFTER                 "Paste After"
#define TR_PASTE_BEFORE                "Paste Before"
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
#define TR_SETUP_SCREENS               "Setup Hautbildschirme"
#define TR_MONITOR_SCREENS             "Monitore Mischer Kanal Logik"
#define TR_AND_SWITCH                  "UND Schalt" // UND mit weiterem Schaltern
#define TR_SF                          "SF" // Spezial Funktionen
#define TR_GF                          "GF" // Globale Funktionen
#define TR_ANADIAGS_CALIB              "Calibrated analogs"
#define TR_ANADIAGS_FILTRAWDEV         "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW          "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX             "Min., max. and range"
#define TR_ANADIAGS_MOVE               "Move analogs to their extremes!"
#define TR_SPEAKER                     INDENT "Lautspr"
#define TR_BUZZER                      INDENT "Summer"
#define TR_BYTES                       "Bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd","Bind"))   //9XR-Pro
#define TR_POWERMETER_ATTN_NEEDED      "Attenuator needed"
#define TR_PXX2_SELECT_RX              "Select RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "Select device"
#define TR_DISCOVER                    "Discover"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Waiting..."
#define TR_RECEIVER_DELETE             "Delete receiver?"
#define TR_RECEIVER_RESET              "Reset receiver?"
#define TR_SHARE                       "Share"
#define TR_BIND                        "Bind"
#define TR_REGISTER             	   TR("Reg", "Register")
#define TR_MODULE_RANGE        		   BUTTON(TR("Rng", "Range"))  //9XR-Pro
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "RECEIVER OPTIONS")
#define TR_DEL_BUTTON                  BUTTON(TR("Del", "Delete"))
#define TR_RESET_BTN           		   BUTTON("Reset")
#define TR_DEBUG                       "Testen"
#define TR_KEYS_BTN                	   BUTTON(TR("SW","Switches"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analogs"))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_CALIBRATION                 "Kalibrieren"
#define TR_SET                   	   BUTTON("Set")
#define TR_TRAINER             		   "DSC Buchse PPM In/Out"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM     		   CENTER "TX-Antennenproblem!"
#if defined(COLORLCD)
  #define TR_MODELIDUSED               "ID used in:"
#else
  #define TR_MODELIDUSED               TR("ID used in:","Receiver ID used in:")
#endif
#define TR_MODULE             		   "Modul-Typ"
#define TR_RX_NAME                     "Rx Name"
#define TR_TELEMETRY_TYPE      		   TR("Typ", "Telemetrietyp")
#define TR_TELEMETRY_SENSORS  		   "Sensoren"
#define TR_VALUE               		   "Wert"
#define TR_REPEAT                      "Repeat"
#define TR_ENABLE                      "Enable"
#define TR_TOPLCDTIMER        		   "Top LCD Timer"
#define TR_UNIT                        "Einheit"
#define TR_TELEMETRY_NEWSENSOR         INDENT "Sensor hinzufügen ..."
#define TR_CHANNELRANGE                TR(INDENT "Kanäle", INDENT "Ausgangs Kanäle")  //wg 9XR-Pro
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
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
#define TR_LOWALARM                    INDENT "1.Warnschwelle"
#define TR_CRITICALALARM               INDENT "Kritischer Alarm"
#define TR_RSSIALARM_WARN              "RSSI"
#define TR_NO_RSSIALARM                TR(INDENT "RSSI Alarms AUS", "RSSI Alarme ausschalten")
#define TR_DISABLE_ALARM               TR(INDENT "Alarme AUS", INDENT "Telemetrie Alarme AUS")
#define TR_ENABLE_POPUP                "Freigabe Popup-Fenster"
#define TR_DISABLE_POPUP               "Sperren  Popup-Fenster"
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
#define TR_COPY_STICKS_TO_OFS          TR3("Copy Stk ->Subtrim", "Kopiere Stick zu Subtrim", "Kopiere Knüppelposition auf Subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR3("Cpy min/max to all", "Kopiere min/max zu allen" , "Kopiere Limits & Mitte auf alle Kanäle")
#define TR_COPY_TRIMS_TO_OFS           TR3("Copy Trim->Subtrim",  "Kopiere Trimm zu Subtrim" , "Kopiere Trimmposition auf Subtrim")  // "Trim to Subtrim"
#define TR_INCDEC                      "Inc/Decrement"
#define TR_GLOBALVAR                   "Global Var"
#define TR_MIXSOURCE                   "Mixer Quelle"
#define TR_CONSTANT                    "Konstant"
#define TR_PERSISTENT_MAH              TR(INDENT "Spr. mAh", INDENT "Speichern mAh") //9XR-Pro
#define TR_PREFLIGHT                   "Vorflug-Checkliste"
#define TR_CHECKLIST                   TR(INDENT "Checkliste", INDENT "Checkliste anzeigen") //9XR-Pro
#define TR_FAS_OFFSET                  TR(INDENT "FAS-Ofs", INDENT "FAS-Offset")
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
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "No Telemetry Screens"
#define TR_TOUCH_PANEL                 "Touch panel:"
#define TR_FILE_SIZE                   "File size"
#define TR_FILE_OPEN                   "Open anyway?"

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
#define TR_FIRST_CHANNEL               "First channel"
#define TR_FILL_BACKGROUND             "Fill background?"
#define TR_BG_COLOR                    "BG Color"
#define TR_SLIDERS_TRIMS               "Sliders+Trims"
#define TR_SLIDERS                     "Sliders"
#define TR_FLIGHT_MODE                 "Flight mode"
#define TR_INVALID_FILE                "Invalid File"
#define TR_TIMER_SOURCE                "Timer source"
#define TR_SIZE                        "Size"
#define TR_SHADOW                      "Shadow"
#define TR_TEXT                        "Text"
#define TR_COLOR                       "Color"
#define TR_MAIN_VIEW_X                 "Main view X"
#define TR_PANEL1_BACKGROUND           "Panel1 background"
#define TR_PANEL2_BACKGROUND           "Panel2 background"

// Taranis Info Zeile Anzeigen
#define TR_LIMITS_HEADERS_NAME         "Name"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Richtung"
#define TR_LIMITS_HEADERS_CURVE        "Kurve"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Mitte"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Subtrim Mode"
#define TR_INVERTED     "Inverted"

#define TR_LSW_HEADERS_FUNCTION        "Funktion"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "UND Schalter"
#define TR_LSW_HEADERS_DURATION        "Dauer"
#define TR_LSW_HEADERS_DELAY           "Verzögerung"

#define TR_GVAR_HEADERS_NAME           "Name"
#define TR_GVAR_HEADERS_FM0            "Wert im FM0"
#define TR_GVAR_HEADERS_FM1            "Wert im FM1"
#define TR_GVAR_HEADERS_FM2            "Wert im FM2"
#define TR_GVAR_HEADERS_FM3            "Wert im FM3"
#define TR_GVAR_HEADERS_FM4            "Wert im FM4"
#define TR_GVAR_HEADERS_FM5            "Wert im FM5"
#define TR_GVAR_HEADERS_FM6            "Wert im FM6"
#define TR_GVAR_HEADERS_FM7            "Wert im FM7"
#define TR_GVAR_HEADERS_FM8            "Wert im FM8"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS            { "Vergleich oder Funktion", "Erste Variable", "Zweite Variable/Konstante", "Zweite Variable/Konstante", "Weitere UND Bedingung für Freigabe des Log Schalters", "ON-Zeit des Log Schalters wenn Bedingung ok", "Mindestdauer der Bedingung damit Log Schalter ON geht" }

//Taranis About screen
#define TR_ABOUTUS                     TR(" Info ", "Info")

#define TR_CHR_SHORT  				   's' // Taste short
#define TR_CHR_LONG   			       'l' // Taste long
#define TR_CHR_TOGGLE 				   't' // Taste als togglefunktion = Ein Aus Ein
#define TR_CHR_HOUR   				   'h' // Stunden
#define TR_CHR_INPUT  				   'I' // Values between A-I will work

#define TR_BEEP_VOLUME                 "Beep-Lautst."
#define TR_WAV_VOLUME                  "Wav-Lautst."
#define TR_BG_VOLUME                   TR("Bgr-Lautst.", "Hintergrund-Lautstärke")

#define TR_TOP_BAR                     "Infozeile"
#define TR_FLASH_ERASE                 "Flash erase..."
#define TR_FLASH_WRITE                 "Flash write..."
#define TR_OTA_UPDATE                  "OTA update..."
#define TR_MODULE_RESET                "Module reset..."
#define TR_UNKNOWN_RX                  "Unknown RX"
#define TR_UNSUPPORTED_RX              "Unsupported RX"
#define TR_OTA_UPDATE_ERROR            "OTA update error"
#define TR_DEVICE_RESET                "Device reset..."
#define TR_ALTITUDE                    INDENT "Höhenanzeige"
#define TR_SCALE                       "Skalieren"
#define TR_VIEW_CHANNELS               "Zeige Kanäle"
#define TR_VIEW_NOTES                  "Zeige Notizen"
#define TR_MODEL_SELECT                "Modell auswählen"
#define TR_MODS_FORBIDDEN              "Anpassung verboten!"
#define TR_UNLOCKED                    "Entsperrt"
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
#define TR_SERVOS_OK                   "Servos OK"
#define TR_SERVOS_KO                   "Servos KO"
#define TR_INVERTED_SERIAL             INDENT "Invert."
#define TR_IGNORE_INSTANCE             TR(INDENT "No Inst.", INDENT "Ignor. Instanzen")
#define TR_DISCOVER_SENSORS            "Start Sensorsuche"
#define TR_STOP_DISCOVER_SENSORS       "Stop Sensorsuche"
#define TR_DELETE_ALL_SENSORS          "Lösche alle Sensoren"
#define TR_CONFIRMDELETE               "Wirklich alle " LCDW_128_480_LINEBREAK "löschen ?"
#define TR_SELECT_WIDGET               "Widget auswählen"  // grafisches Element
#define TR_REMOVE_WIDGET               "Widget löschen"
#define TR_WIDGET_SETTINGS             "Widget einstellen"
#define TR_REMOVE_SCREEN               "Screen löschen"
#define TR_SETUP_WIDGETS               "Setup widgets"
#define TR_USER_INTERFACE              "User interface"
#define TR_THEME                       "Theme"
#define TR_SETUP                       "Setup"
#define TR_LAYOUT                      "Layout"
#define TR_ADD_MAIN_VIEW               "Ansicht hinzufügen"
#define TR_BACKGROUND_COLOR            "Hintergrundfarbe"
#define TR_MAIN_COLOR                  "Hauptfarbe"
#define TR_BAR2_COLOR                  "Secondary bar color"
#define TR_BAR1_COLOR                  "Main bar color"
#define TR_TEXT_COLOR                  "Text color"
#define TR_TEXT_VIEWER                 "Text Viewer"
// ----------------------------- Symbole für Auswahlliste----------
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Inputs"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua Skripte"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Knüppel"
#define TR_MENU_POTS                   STR_CHAR_POT "Potis"
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
#define TR_JITTER_FILTER               "ADC Filter"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Check RTC", "Check RTC voltage")
#define TR_AUTH_FAILURE                "Auth-failure"
#define TR_RACING_MODE                 "Racing mode"

// ----------------------------------------------------------------
#define STR_VFR                        "VFR"
#define STR_RSSI                       "RSSI"
#define STR_R9PW                       "R9PW"
#define STR_RAS                        "SWR"
#define STR_A1                         "A1"
#define STR_A2                         "A2"
#define STR_A3                         "A3"
#define STR_A4                         "A4"
#define STR_BATT                       "RxBt"
#define STR_ALT                        "Alt"
#define STR_TEMP1                      "Tmp1"
#define STR_TEMP2                      "Tmp2"
#define STR_TEMP3                      "Tmp3"
#define STR_TEMP4                      "Tmp4"
#define STR_RPM2                       "RPM2"
#define STR_PRES                       "Pres"
#define STR_ODO1                       "Odo1"
#define STR_ODO2                       "Odo2"
#define STR_TXV                        "TX_V"
#define STR_CURR_SERVO1                "CSv1"
#define STR_CURR_SERVO2                "CSv2"
#define STR_CURR_SERVO3                "CSv3"
#define STR_CURR_SERVO4                "CSv4"
#define STR_DIST                       "Dist"
#define STR_ARM                        "Arm"
#define STR_C50                        "C50"
#define STR_C200                       "C200"
#define STR_RPM                        "RPM"
#define STR_FUEL                       "Fuel"
#define STR_VSPD                       "VSpd"
#define STR_ACCX                       "AccX"
#define STR_ACCY                       "AccY"
#define STR_ACCZ                       "AccZ"
#define STR_GYROX                      "GYRX"
#define STR_GYROY                      "GYRY"
#define STR_GYROZ                      "GYRZ"
#define STR_CURR                       "Curr"
#define STR_CAPACITY                   "Capa"
#define STR_VFAS                       "VFAS"
#define STR_BATT_PERCENT               "Bat%"
#define STR_ASPD                       "ASpd"
#define STR_GSPD                       "GSpd"
#define STR_HDG                        "Hdg"
#define STR_SATELLITES                 "Sats"
#define STR_CELLS                      "Cels"
#define STR_GPSALT                     "GAlt"
#define STR_GPSDATETIME                "Date"
#define STR_BATT1_VOLTAGE              "RB1V"
#define STR_BATT2_VOLTAGE              "RB2V"
#define STR_BATT1_CURRENT              "RB1A"
#define STR_BATT2_CURRENT              "RB2A"
#define STR_BATT1_CONSUMPTION          "RB1C"
#define STR_BATT2_CONSUMPTION          "RB2C"
#define STR_BATT1_TEMP                 "RB1T"
#define STR_BATT2_TEMP                 "RB2T"
#define STR_RB_STATE                   "RBS"
#define STR_CHANS_STATE                "RBCS"
#define STR_RX_RSSI1                   "1RSS"
#define STR_RX_RSSI2                   "2RSS"
#define STR_RX_QUALITY                 "RQly"
#define STR_RX_SNR                     "RSNR"
#define STR_RX_NOISE                   "RNse"
#define STR_RF_MODE                    "RFMD"
#define STR_TX_POWER                   "TPWR"
#define STR_TX_RSSI                    "TRSS"
#define STR_TX_QUALITY                 "TQly"
#define STR_TX_SNR                     "TSNR"
#define STR_TX_NOISE                   "TNse"
#define STR_PITCH                      "Ptch"
#define STR_ROLL                       "Roll"
#define STR_YAW                        "Yaw"
#define STR_THROTTLE                   "Thr"
#define STR_QOS_A                      "FdeA"
#define STR_QOS_B                      "FdeB"
#define STR_QOS_L                      "FdeL"
#define STR_QOS_R                      "FdeR"
#define STR_QOS_F                      "FLss"
#define STR_QOS_H                      "Hold"
#define STR_LAP_NUMBER                 "Lap "
#define STR_GATE_NUMBER                "Gate"
#define STR_LAP_TIME                   "LapT"
#define STR_GATE_TIME                  "GteT"
#define STR_ESC_VOLTAGE                "EscV"
#define STR_ESC_CURRENT                "EscA"
#define STR_ESC_RPM                    "Erpm"
#define STR_ESC_CONSUMPTION            "EscC"
#define STR_ESC_TEMP                   "EscT"
#define STR_SD1_CHANNEL                "Chan"
#define STR_GASSUIT_TEMP1              "GTp1"
#define STR_GASSUIT_TEMP2              "GTp2"
#define STR_GASSUIT_RPM                "GRPM"
#define STR_GASSUIT_FLOW               "GFlo"
#define STR_GASSUIT_CONS               "GFue"
#define STR_GASSUIT_RES_VOL            "GRVl"
#define STR_GASSUIT_RES_PERC           "GRPc"
#define STR_GASSUIT_MAX_FLOW           "GMFl"
#define STR_GASSUIT_AVG_FLOW           "GAFl"
#define STR_SBEC_VOLTAGE               "BecV"
#define STR_SBEC_CURRENT               "BecA"
#define STR_RB3040_EXTRA_STATE         "RBES"
#define STR_RB3040_CHANNEL1            "CH1A"
#define STR_RB3040_CHANNEL2            "CH2A"
#define STR_RB3040_CHANNEL3            "CH3A"
#define STR_RB3040_CHANNEL4            "CH4A"
#define STR_RB3040_CHANNEL5            "CH5A"
#define STR_RB3040_CHANNEL6            "CH6A"
#define STR_RB3040_CHANNEL7            "CH7A"
#define STR_RB3040_CHANNEL8            "CH8A"
#define STR_ESC_VIN                    "EVIN"
#define STR_ESC_TFET                   "TFET"
#define STR_ESC_CUR                    "ECUR"
#define STR_ESC_TBEC                   "TBEC"
#define STR_ESC_BCUR                   "CBEC"
#define STR_ESC_VBEC                   "VBEC"
#define STR_ESC_THR                    "ETHR"
#define STR_ESC_POUT                   "EOUT"
#define STR_SMART_BAT_BTMP             "BTmp"
#define STR_SMART_BAT_BCUR             "BCur"
#define STR_SMART_BAT_BCAP             "BUse"
#define STR_SMART_BAT_MIN_CEL          "CLMi"
#define STR_SMART_BAT_MAX_CEL          "CLMa"
#define STR_SMART_BAT_CYCLES           "Cycl"
#define STR_SMART_BAT_CAPACITY         "BCpT"
#define STR_CL01                       "Cel1"
#define STR_CL02                       "Cel2"
#define STR_CL03                       "Cel3"
#define STR_CL04                       "Cel4"
#define STR_CL05                       "Cel5"
#define STR_CL06                       "Cel6"
#define STR_CL07                       "Cel7"
#define STR_CL08                       "Cel8"
#define STR_CL09                       "Cel9"
#define STR_CL10                       "Cl10"
#define STR_CL11                       "Cl11"
#define STR_CL12                       "Cl12"
#define STR_CL13                       "Cl13"
#define STR_CL14                       "Cl14"
#define STR_CL15                       "Cl15"
#define STR_CL16                       "Cl16"
#define STR_CL17                       "Cl17"
#define STR_CL18                       "Cl18"
#define STR_FRAME_RATE                 "FRat"
#define STR_TOTAL_LATENCY              "TLat"
#define STR_VTX_FREQ                   "VFrq"
#define STR_VTX_PWR                    "VPwr"
#define STR_VTX_CHAN                   "VChn"
#define STR_VTX_BAND                   "VBan"
#define STR_SERVO_CURRENT              "SrvA"
#define STR_SERVO_VOLTAGE              "SrvV"
#define STR_SERVO_TEMPERATURE          "SrvT"
#define STR_SERVO_STATUS               "SrvS"
#define STR_LOSS                       "Loss"
#define STR_SPEED                      "Spd "
#define STR_FLOW                       "Flow"

// The following content is Untranslated)

#define TR_USE_THEME_COLOR             "Use theme color"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS   "Add all Trims to Subtrims"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS        "Open Channel Monitor"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Open Channel Mon."
#endif
#define TR_DUPLICATE                   "Duplicate"
#define TR_ACTIVATE                    "Set Active"
#define TR_RED                         "Red"
#define TR_BLUE                        "Blue"
#define TR_GREEN                       "Green"
#define TR_COLOR_PICKER                "Color Picker"
#define TR_EDIT_THEME_DETAILS          "Edit Theme Details"
#define TR_AUTHOR                      "Author"
#define TR_DESCRIPTION                 "Description"
#define TR_SAVE                        "Save"
#define TR_CANCEL                      "Cancel"
#define TR_EDIT_THEME                  "EDIT THEME"
#define TR_DETAILS                     "Details"
#define TR_THEME_EDITOR                "THEMES"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL      "Select\nModel"
#define TR_MAIN_MENU_MODEL_NOTES       "Model\nNotes"
#define TR_MAIN_MENU_CHANNEL_MONITOR   "Channel\nMonitor"
#define TR_MAIN_MENU_MODEL_SETTINGS    "Model\nSettings"
#define TR_MAIN_MENU_RADIO_SETTINGS    "Radio\nSettings"
#define TR_MAIN_MENU_SCREEN_SETTINGS   "Screens\nSettings"
#define TR_MAIN_MENU_RESET_TELEMETRY   "Reset\nTelemetry"
#define TR_MAIN_MENU_STATISTICS        "Statistics"
#define TR_MAIN_MENU_ABOUT_EDGETX      "About\nEdgeTX"
// End Main menu
