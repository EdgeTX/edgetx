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

// PL translations author: Jacek Drewniak <jpdjpd33@gmail.com>

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */


#define TR_OFFON               "WYŁ","ZAŁ"
#define TR_MMMINV              "---","REV"
#define TR_VBEEPMODE           "Cichy","Alarm","BezPr","Wsz."
#define TR_VBLMODE             TR("Wył","Wyłącz"),TR("Przy","Przycisk"),TR("Drąż","Drązki"),"Oba",TR("Zał","Włącz")
#define TR_TRNMODE             "Wył","+=",":="
#define TR_TRNCHN              "KN1","KN2","KN3","KN4"
#define TR_AUX_SERIAL_MODES    "Wyłącz","S-Port Kopia","Telemetria","Trener SBUS","LUA","CLI","GPS","Debug"
#define TR_SWTYPES             "Brak","Chwil.","2POZ","3POZ"
#define TR_POTTYPES            Brak,TR("Pot w. det","Poten z zapadką"),TR("Multipos","Przeł.Wielopoz."),TR("Pot","Potencjometr")
#define TR_SLIDERTYPES         "Brak","Suwak"
#define TR_VPERSISTENT         "Wyłącz","Lot","Ręczny Reset"
#define TR_COUNTRY_CODES       TR("US","Ameryka"),TR("JP","Japonia"),TR("EU", "Europa")
#define TR_USBMODES            "Ask",TR("Joyst","Joystick"),TR("SDCard","Storage"),"Serial"
#define TR_JACK_MODES          "Ask","Audio","Trainer"
#define TR_TELEMETRY_PROTOCOLS "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"
#define TR_MULTI_CUSTOM        "Custom"
#define TR_VTRIMINC            TR("Expo","Expotencja"),TR("B.Dokł","B.Dokładny"),TR("Dokł.","Dokładny"),"Średni",TR("Zgrubn","Zgrubny")
#define TR_VDISPLAYTRIMS       "Nie","Zmień","Tak"
#define TR_VBEEPCOUNTDOWN      "Cichy","Pik","Dźwięk","Wibrac"
#define TR_COUNTDOWNVALUES     "5s","10s","20s","30s"
#define TR_VVARIOCENTER        "Ton","Cicho"
#define TR_CURVE_TYPES         "Standard","Własny"
#define TR_ADCFILTERVALUES     "Globalne","WYŁ","ZAŁ"

#if defined(PCBX10)
  #define TR_RETA123           "K","W","G","L","1","3","2","4","5","6","7","L","R"
#elif defined(PCBHORUS)
  #define TR_RETA123           "K","W","G","L","1","3","2","4","5","L","R"
#elif defined(PCBX9E)
  #define TR_RETA123           "K","W","G","L","1","2","3","4","L","R","L","R"
#elif defined(PCBTARANIS)
  #define TR_RETA123           "K","W","G","L","1","2","3","L","R"
#else
  #define TR_RETA123           "K","W","G","L","1","2","3"
#endif

#define TR_VCURVEFUNC          "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX              "Dodaj","Mnóż","Zastąp"
#define TR_VMLTPX2             "+=","*=",":="

#if defined(PCBHORUS)
  #define TR_VMIXTRIMS         "WYŁ","ZAŁ","SK","SW","GAZ","Lot","Ail","T5","T6"
#else
  #define TR_VMIXTRIMS         "WYŁ","ZAŁ","SK","SW","GAZ","Lot"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER          "Timer"
  #define TR_CSWSTICKY         "Stały"
  #define TR_CSWRANGE          "Zasię"
  #define TR_CSWSTAY           "Brzeg"
#else
  #define TR_CSWTIMER          "Tim"
  #define TR_CSWSTICKY         "Stały"
  #define TR_CSWRANGE          "Zasię"
  #define TR_CSWSTAY           "Brzeg"
#endif

#define TR_CSWEQUAL  "a=x"
#define TR_VCSWFUNC  "---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#if defined(VARIO)
  #define TR_VVARIO            "Wario"
#else
  #define TR_VVARIO            "[Wario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND             "GrajDźwięk"
#else
  #define TR_SOUND             "Pikanie"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC            "Wibracje"
#else
  #define TR_HAPTIC            "[Wibracje]"
#endif

#if defined(VOICE)
  #define TR_PLAY_TRACK        "GrajŚcieżk"
  #define TR_PLAY_BOTH         "Graj Oba"
  #define TR_PLAY_VALUE        TR("Graj Wart","GrajWartoś")
#else
  #define TR_PLAY_TRACK        "[Graj Ści]"
  #define TR_PLAY_BOTH         "[Graj Oba]"
  #define TR_PLAY_VALUE        "[Graj War]"
#endif

#define TR_SF_BG_MUSIC        "Muz. tła","Muz.tła ||"

#if defined(SDCARD)
  #define TR_SDCLOGS           "Logi->SD"
#else
  #define TR_SDCLOGS           "[Logować]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR       "Ustaw"
#else
  #define TR_ADJUST_GVAR       "[UstawZG]"
#endif

#if defined(LUA)
  #define TR_SF_PLAY_SCRIPT   "SkryptyLua"
#else
  #define TR_SF_PLAY_SCRIPT   "[Lua]"
#endif

#if defined(DEBUG)
  #define TR_SF_TEST          ,"Test"
#else
  #define TR_SF_TEST
#endif

#if defined(OVERRIDE_CHANNEL_FUNCTION) && LCD_W >= 212
  #define TR_SF_SAFETY        "Bezpiecz"
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY        "Bezp."
#else
  #define TR_SF_SAFETY        "---"
#endif

#define TR_SF_SCREENSHOT      "Zrzut Ekra"
#define TR_SF_RACING_MODE     "RacingMode"
#define TR_SF_DISABLE_TOUCH   "No Touch"
#define TR_SF_RESERVE         "[rezerwa]"

#define TR_VFSWFUNC            TR_SF_SAFETY,"Trener","Inst-Trim","Resetuj","Ustaw",TR_ADJUST_GVAR,"Głośność","SetFailsafe","RangeCheck","ModuleBind",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,"Podświetl",TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH TR_SF_TEST

#define TR_FSW_RESET_TELEM   TR("Telm", "Telemetra")

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS  "Timer 1","Timer 2","Timer 3"
#else
  #define TR_FSW_RESET_TIMERS  "Tmr1","Tmr2","Tmr3"
#endif

#define TR_VFSWRESET           TR_FSW_RESET_TIMERS,TR("All","Flight"),TR_FSW_RESET_TELEM
#define TR_FUNCSOUNDS          TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Ost1","Ostrz1"),TR("Ost2","Ostrz2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Syre","Syrena"),TR("Dzwo","Dzwone"),TR("Crck","Krytcz"),TR("Alrm","AlmZeg")

#define LENGTH_UNIT_IMP        "ft"
#define SPEED_UNIT_IMP         "mph"
#define LENGTH_UNIT_METR       "m"
#define SPEED_UNIT_METR        "kmh"

#define TR_VUNITSSYSTEM        TR("Metr.","Metryczn"),TR("Imper.","Imperial")
#define TR_VTELEMUNIT          "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km"

#define STR_V                  (STR_VTELEMUNIT[1])
#define STR_A                  (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE    "Brak","Liczb","Paski","Skrypt"
#define TR_GPSFORMAT           "DMS","NMEA"

#define TR_TEMPLATE_CLEAR_MIXES        "Usuń Miksy"
#define TR_TEMPLATE_SIMPLE_4CH         "Prosty. 4kn"
#define TR_TEMPLATE_STICKY_TCUT        "Przełą-T-Cut"
#define TR_TEMPLATE_VTAIL              "V-Tail"
#define TR_TEMPLATE_DELTA              "Elevon\\Delta "
#define TR_TEMPLATE_ECCPM              "eCCPM"
#define TR_TEMPLATE_HELI               "Ustaw Heli"
#define TR_TEMPLATE_SERVO_TEST         "Test serwa"

#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICKS_VSRCRAW      STR_CHAR_STICK "SK", STR_CHAR_STICK "SW", STR_CHAR_STICK "Gaz", STR_CHAR_STICK "Lot"

#if defined(PCBHORUS)
  #define TR_TRIMS_VSRCRAW     STR_CHAR_TRIM "SK", STR_CHAR_TRIM "SW", STR_CHAR_TRIM "Gaz", STR_CHAR_TRIM "Lot", STR_CHAR_TRIM "T5",  STR_CHAR_TRIM "T6"
#else
  #define TR_TRIMS_VSRCRAW     STR_CHAR_TRIM "SK", STR_CHAR_TRIM "SW", STR_CHAR_TRIM "Gaz", STR_CHAR_TRIM "Lot"
#endif

#if defined(PCBHORUS)
#define TR_TRIMS_SWITCHES    STR_CHAR_TRIM "Rl", STR_CHAR_TRIM "Rr", STR_CHAR_TRIM "Ed", STR_CHAR_TRIM "Eu", STR_CHAR_TRIM "Td", STR_CHAR_TRIM "Tu", STR_CHAR_TRIM "Al", STR_CHAR_TRIM "Ar", STR_CHAR_TRIM "5d", STR_CHAR_TRIM "5u", STR_CHAR_TRIM "6d", STR_CHAR_TRIM "6u"
#else
  #define TRIM_SWITCH_CHAR   TR("t",STR_CHAR_TRIM)
  #define TR_TRIMS_SWITCHES  TRIM_SWITCH_CHAR "Rl", TRIM_SWITCH_CHAR "Rr", TRIM_SWITCH_CHAR "Ed", TRIM_SWITCH_CHAR "Eu", TRIM_SWITCH_CHAR "Td", TRIM_SWITCH_CHAR "Tu", TRIM_SWITCH_CHAR "Al", TRIM_SWITCH_CHAR "Ar"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES    "ON","One"

#if defined(IMU)
  #define TR_IMU_VSRCRAW       "TltX","TltY",
#else
  #define TR_IMU_VSRCRAW
#endif

#if defined(HELI)
  #define TR_CYC_VSRCRAW      "CYC1","CYC2","CYC3"
#else
  #define TR_CYC_VSRCRAW      "[C1]","[C2]","[C3]"
#endif

#define TR_RESERVE_VSRCRAW   "[--]"
#define TR_EXTRA_VSRCRAW     "Batt","Time","GPS",TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,"Tmr1","Tmr2","Tmr3"

#define TR_VTMRMODES                   "WYŁ","ABS","THs","TH%","THt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Trener/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Uczeń/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Trener/SBUS Moduł"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Trener/CPPM Moduł"
#define TR_VTRAINER_MASTER_BATTERY     "Trener/Serial"
#define TR_VTRAINER_BLUETOOTH          "Master/" TR("BT","Bluetooth"), "Slave/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VFAILSAFE                   "Brak","Utrzymuj","Własne","0 sygnału","Odbiornik"
#define TR_VSENSORTYPES                "Użytkownik","Obliczone"
#define TR_VFORMULAS                   "Dodaj","Średnie","Min","Max","Mnóż","Zliczani","Komórka","Zużycie","Zasięg"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Niskie","1","2","3","4","5","6","Wysokie","Delta"
#define TR_TEXT_SIZE                   "Standard","Tiny","Small","Mid","Double"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA" (center only)","= (symetrical)"

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

#define TR_MENUWHENDONE        CENTER "\006" TR_ENTER " > DALEJ"
#define TR_FREE                "woln"
#define TR_YES                         "Yes"
#define TR_NO                          "No"
#define TR_DELETEMODEL         "SKASUJ MODEL"
#define TR_COPYINGMODEL        "Kopiuj model.."
#define TR_MOVINGMODEL         "Przenieś model."
#define TR_LOADINGMODEL        "Załaduj model.."
#define TR_NAME                "Nazwa"
#define TR_MODELNAME           "Nazwa modelu"
#define TR_PHASENAME           "Nazwa Fazy"
#define TR_MIXNAME             "Nazwa Mix"
#define TR_INPUTNAME           "Wpisz Nazw"
  #define TR_EXPONAME          "LiniaNazw"
#define TR_BITMAP              "Obrazek    "
#define TR_TIMER               TR("Timer", "Timer ")
#define TR_START                       "Start"
#define TR_ELIMITS             TR("Limi+25%", "Kanał +/- 125% ")
#define TR_ETRIMS              TR("E.Trym ", "Poszer Trymery")
#define TR_TRIMINC             "Krok Trym"
#define TR_DISPLAY_TRIMS       TR("Pok.Trym.","Pokaż Trymery")
#define TR_TTRACE              TR("Źród gaz", INDENT "Źródło gazu ")
#define TR_TTRIM               TR("TryGaz", INDENT "TrymWolnObrotó")
#define TR_TTRIM_SW            TR("T-Trim-Sw", INDENT "Trim switch")
#define TR_BEEPCTR             TR("ŚrodBeep", "Pikn.Środka")
#define TR_USE_GLOBAL_FUNCS    TR("Funk.Glb.","Użyj Funkcji Glb")
#define TR_PROTOCOL            TR("Proto", "Protokół")
#define TR_PPMFRAME            INDENT "Ramka PPM"
#define TR_REFRESHRATE               TR(INDENT "Refresh", INDENT "Refresh rate")
#define STR_WARN_BATTVOLTAGE           TR(INDENT "Output is VBAT: ", INDENT "Warning: output level is VBAT: ")
#define TR_WARN_5VOLTS                 "Warning: output level is 5 volts"
#define TR_MS                  "ms"
#define TR_FREQUENCY                   INDENT "Frequency"
#define TR_SWITCH              "Przełą"
#define TR_TRIMS               "Trymy"
#define TR_FADEIN              "Pojawia"
#define TR_FADEOUT             "Zanik   "
#define TR_DEFAULT             "(Bazowa) "
#define TR_CHECKTRIMS          CENTER "\006Spr  \012Trymy"
#define OFS_CHECKTRIMS         CENTER_OFS+(9*FW)
#define TR_SWASHTYPE           "Typ tarczy"
#define TR_COLLECTIVE          TR("Kolektyw", "Źródło Kolektywu")
#define TR_AILERON             TR("Lateral cyc.", "Lateral cyc. source")
#define TR_ELEVATOR            TR("Long. cyc.", "Long. cyc. source")
#define TR_SWASHRING           "Tarcza    "
#define TR_ELEDIRECTION        TR("Ster Wysokośc","Dług. Kierunku cyklu")
#define TR_AILDIRECTION        TR("Kierunek Lotk","Lateral cyc. direction")
#define TR_COLDIRECTION        TR("PIT Direction","Coll. pitch direction")
#define TR_MODE                "Tryb"
#define TR_SUBTYPE             INDENT "Subtype"
#define TR_NOFREEEXPO          "BrakWoln.Expo!"
#define TR_NOFREEMIXER         "BrakWoln.Mixów!"
#define TR_SOURCE               "Źródło"
#define TR_WEIGHT              "Waga "
#define TR_EXPO                TR("Expo", "Exponential")
#define TR_SIDE                "Strona"
#define TR_DIFFERENTIAL        "Różnic"
#define TR_OFFSET              "Ofset "
#define TR_TRIM                "Trym"
#define TR_DREX                "DRex"
#define DREX_CHBOX_OFFSET      30
#define TR_CURVE               "Krzywa"
#define TR_FLMODE              "Tryb"
#define TR_MIXWARNING          "UWAGA"
#define TR_OFF                 "Wył."
#define TR_ANTENNA                     "Antenna"
#define TR_NO_INFORMATION              TR("No info", "No information")
#define TR_MULTPX              "Złącz."
#define TR_DELAYDOWN           "Opózn.(-)"
#define TR_DELAYUP             "Opózn.(+)"
#define TR_SLOWDOWN            "Spowoln.(-)"
#define TR_SLOWUP              "Spowoln.(+)"
#define TR_MIXES               "MIKSERY"
#define TR_CV                  "Kr"
#if defined(PCBNV14)
#define TR_GV                  "ZG"
#else
#define TR_GV                  TR("G", "ZG")
#endif
#define TR_ACHANNEL            "A\004Kanał"
#define TR_RANGE               INDENT "Zakres"
#define TR_CENTER              INDENT "Środek"
#define TR_BAR                 "Pas"
#define TR_ALARM               "Alarm"
#define TR_USRDATA             TR("Dane Użytk.", "Dane użytkownika")
#define TR_BLADES              "Łopaty śmigla"
#define TR_SCREEN              "Ekran "
#define TR_SOUND_LABEL         "Dźwięk"
#define TR_LENGTH              "Długość"
#define TR_BEEP_LENGTH         "Dług. Beepa"
#define TR_BEEP_PITCH          "Dźwięk"
#define TR_HAPTIC_LABEL        "Wibracja"
#define TR_STRENGTH            "Siła"
#define TR_IMU_LABEL           "IMU"
#define TR_IMU_OFFSET          "Offset"
#define TR_IMU_MAX             "Max"
#define TR_CONTRAST            "Kontrast"
#define TR_ALARMS_LABEL        "Alarmy"
#define TR_BATTERY_RANGE       TR("PokażPasBat.", "Pokaż pasek bateri")
#define TR_BATTERYCHARGING     "Ładowanie..."
#define TR_BATTERYFULL         "Bateria naładowana"
#define TR_BATTERYNONE         "Brak!"
#define TR_BATTERYWARNING      "Bateria rozładowana"
#define TR_INACTIVITYALARM     "Alarm bezczynności"
#define TR_MEMORYWARNING       "Pełna pamięć"
#define TR_ALARMWARNING        "Wyłącz dźwięk"
#define TR_RSSI_SHUTDOWN_ALARM   TR("Rssi Shutdown", "Check Rssi on Shutdown")
#define TR_MODEL_STILL_POWERED "Model still powered"
#define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_MODEL_SHUTDOWN              "Shutdown ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Press enter to confirm"
#define TR_THROTTLE_LABEL      "Gaz    "
#define TR_THROTTLEREVERSE     TR("OdwrGaz", INDENT "OdwrGaz")
#define TR_MINUTEBEEP          TR("Minuta", "PikCoMinutę")
#define TR_BEEPCOUNTDOWN       INDENT "Odliczanie"
#define TR_PERSISTENT          TR(INDENT "Dokł.", INDENT "Dokładny")
#define TR_BACKLIGHT_LABEL     "Podświetl"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLDELAY             INDENT"Czas trwania"
#define TR_BLONBRIGHTNESS      INDENT"Jasnośc wł."
#define TR_BLOFFBRIGHTNESS     INDENT"Jasność wył."
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR             "Color"
#define TR_SPLASHSCREEN        "Logo ekranu"
#define TR_PWR_ON_DELAY                "Pwr On delay"
#define TR_PWR_OFF_DELAY               "Pwr Off delay"
#define TR_THROTTLE_WARNING     TR(INDENT"OstrzGaz", INDENT "OstrzeżenieGaz")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "InnePoł", INDENT INDENT INDENT INDENT "Inne położenie?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Poł. %", "Położenie %")
#define TR_SWITCHWARNING       TR(INDENT "OstrzPrzeł", INDENT "PozycjaPrzeł")
#define TR_POTWARNINGSTATE     TR(INDENT "Pot&Slid.", INDENT "Pots & sliders")
#define TR_SLIDERWARNING       TR(INDENT "Slid. pos.", INDENT "Slider positions")
#define TR_POTWARNING          TR(INDENT "Ostrz.Poten.", INDENT "PozycjaPotencj.")
#define TR_TIMEZONE            TR("Strefa czasowa" ,"Strefa czasowa GPS")
#define TR_ADJUST_RTC          TR("RegulujRTC", "RegulujRTC")
#define TR_GPS                 "GPS"
#define TR_RXCHANNELORD        TR("KolejnKan.RX", "Bazowa kolejność kanałów")
#define TR_STICKS              "Drążki"
#define TR_POTS                "Pots"
#define TR_SWITCHES            "Przełącz"
#define TR_SWITCHES_DELAY      "Opóźn.Przeł."
#define TR_SLAVE               "Uczeń"
#define TR_MODESRC             " Tryb\006% Źródło"
#define TR_MULTIPLIER          "Mnożnik"
#define TR_CAL                 "Kal."
#define TR_CALIBRATION                 "Calibration"
#define TR_VTRIM               "Trym- +"
#define TR_BG                  "BG:"
#define TR_CALIB_DONE          "Calibration completed"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART       "Press [Enter] to start"
  #define TR_SETMIDPOINT       "Center sticks/pots/sliders and press [Enter]"
  #define TR_MOVESTICKSPOTS    "Move sticks, pots and sliders and press [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART       TR_ENTER " KALIBRUJ"
  #define TR_SETMIDPOINT       "Centruj drażki/potencj"
  #define TR_MOVESTICKSPOTS    "Poruszaj drążkami/pot"
#else
  #define TR_MENUTOSTART       CENTER "\010" TR_ENTER " KALIBRUJ"
  #define TR_SETMIDPOINT       TR(CENTER "\002Cenruj drążki/potencj",CENTER "\010Ustaw drązki/potencj na środku")
  #define TR_MOVESTICKSPOTS    "Poruszaj drążkami/pot"
#endif
#define TR_RXBATT              "Rx Batt:"
#define TR_TXnRX               "Tx:\0Rx:"
#define OFS_RX                 4
#define TR_ACCEL               "Acc:"
#define TR_NODATA              CENTER "BrakDAN"
#define TR_US                         "us"
#define TR_HZ                         "Hz"
#define TR_TMIXMAXMS                  "TmixMaks"
#define TR_FREE_STACK                 "Wolny stos"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL          "Lua scripts"
#define TR_FREE_MEM_LABEL             "Free mem"
#define TR_TIMER_LABEL                "Timer"
#define TR_THROTTLE_PERCENT_LABEL     "Throttle %"
#define TR_BATT_LABEL                 "Battery"
#define TR_SESSION                    "Session"
#define TR_MENUTORESET         TR_ENTER " >> Reset"
#define TR_PPM_TRAINER         "TR"
#define TR_CH                  "KN"
#define TR_MODEL               "MODEL"
#define TR_FM                  "FL"
#define TR_MIX                 "MIX"
#define TR_EEPROMLOWMEM        "Niska pamięć EEPROM"
#define TR_PRESS_ANY_KEY_TO_SKIP   "Wciśnij guzik->ignoruj"
#define TR_THROTTLE_NOT_IDLE     "Gaz nie na zerze"
#define TR_ALARMSDISABLED      "Alarmy wyłączone"
#define TR_PRESSANYKEY         TR("\010Wciśnij jakiś klawisz","Wciśnij jakiś klawisz")
#define TR_BADEEPROMDATA       "\006Błąd danych EEPROM"
#define TR_BAD_RADIO_DATA      "Bad Radio Data"
#define TR_EEPROMFORMATTING    "\004Formatowanie EEPROM"
#define TR_STORAGE_FORMAT      "Storage Preparation"
#define TR_EEPROMOVERFLOW      "Przeładowany EEPROM"
#define TR_RADIO_SETUP      "USTAWIENIA RADIA"
#define TR_MENUDATEANDTIME     "DATA I CZAS"
#define TR_MENUTRAINER         "TRENER"
#define TR_MENUSPECIALFUNCS    "FUNKCJE GLOBALNE"
#define TR_MENUVERSION         "WERSJA"
#define TR_MENU_RADIO_SWITCHES            TR("PRZEŁ","TEST PRZEŁ")
#define TR_MENU_RADIO_ANALOGS             TR("WE-ANA","WEJŚCIA ANALOG.")
#define TR_MENU_RADIO_ANALOGS_CALIB       "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS   "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION     "KALIBRACJA"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS     "Trymery => Subtrimy"
#else
  #define TR_TRIMS2OFFSETS     "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE  "Channels=>Failsafe"
#define TR_CHANNEL2FAILSAFE   "Channel=>Failsafe"
#define TR_MENUMODELSEL        TR("WYB.MODEL","WYBÓR MODELU")
#define TR_MENU_MODEL_SETUP           TR("USTAW","USTAW MODELU")
#define TR_MENUFLIGHTMODE     "FAZA LOTU"
#define TR_MENUFLIGHTMODES    "FAZY LOTU"
#define TR_MENUHELISETUP       "USTAW HELI"

  #define TR_MENUINPUTS        "WEJŚCIA"
  #define TR_MENULIMITS        "WYJŚCIA"

#define TR_MENUCURVES          "KRZYWE"
#define TR_MENUCURVE           "KRZYWA"
#define TR_MENULOGICALSWITCH    "PRZEŁ. LOGICZNY"
#define TR_MENULOGICALSWITCHES   TR("PRZEŁ. LOG.", "PRZEŁ. LOGICZNE")
#define TR_MENUCUSTOMFUNC      "FUNKCJE SPECJALNE"
#define TR_MENUCUSTOMSCRIPTS   "WŁASNE SKRYPTY"
#define TR_MENUTELEMETRY       "TELEMETRIA"
#define TR_MENUTEMPLATES       "SZABLONY"
#define TR_MENUSTAT            "STAT"
#define TR_MENUDEBUG           "DEBUG"
#define TR_MONITOR_CHANNELS1   "CHANNELS MONITOR 1/8"
#define TR_MONITOR_CHANNELS2   "CHANNELS MONITOR 9/16"
#define TR_MONITOR_SWITCHES    "LOGICAL SWITCHES MONITOR"
#define TR_MONITOR_CHANNELS3   "CHANNELS MONITOR 17/24"
#define TR_MONITOR_CHANNELS4   "CHANNELS MONITOR 25/32"
#define TR_MONITOR_OUTPUT_DESC "Outputs"
#define TR_MONITOR_MIXER_DESC  "Mixers"
  #define TR_RECEIVER_NUM      TR("Nr RX", "Nr odbiornika")
  #define TR_RECEIVER          INDENT "Receiver"
#define TR_MULTI_RFTUNE        TR("Freq tune", "RF Freq. fine tune")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY             "Telemetry"
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
#define TR_SYNCMENU            "[Synch]"
#define TR_LIMIT               INDENT "Limit"
#define TR_MINRSSI             "Min Rssi"
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_LATITUDE            "Szer. geo."
#define TR_LONGITUDE           "Dł.   geo."
#define TR_GPS_COORDS_FORMAT            TR("Koord.GPS","Typ koordynat GPS")
#define TR_VARIO               TR("Wario","Wariometr")
#define TR_PITCH_AT_ZERO       "Ton przy Zerze"
#define TR_PITCH_AT_MAX        "Ton przy Max"
#define TR_REPEAT_AT_ZERO      "Powtórz przy Zerze"
#define TR_SHUTDOWN            "Wyłączanie.."
#define TR_SAVE_MODEL           "Zapisz ustwienia modelu"
#define TR_BATT_CALIB          "Kalib:Baterii"
#define TR_CURRENT_CALIB       " +=\006Kalibracja prądu"
#define TR_VOLTAGE             TR (INDENT"Napięcie",INDENT"Źródło Napięcia")
#define TR_CURRENT             TR (INDENT"Prąd",INDENT"Źródło Prądu")
#define TR_SELECT_MODEL        "Wybór modelu"
#define TR_MODEL_CATEGORIES            "Model Categories"
#define TR_MODELS                      "Models"
#define TR_SELECT_MODE                 "Select mode"
#define TR_CREATE_CATEGORY     "Create Category"
#define TR_RENAME_CATEGORY     "Rename Category"
#define TR_DELETE_CATEGORY     "Delete Category"
#define TR_CREATE_MODEL        "Nowy model"
#define TR_DUPLICATE_MODEL     "Duplicate Model"
#define TR_COPY_MODEL          "Skopiuj model"
#define TR_MOVE_MODEL          "Przenieś model"
#define TR_BACKUP_MODEL        "Zbackupuj model"
#define TR_DELETE_MODEL        "Skasuj model"
#define TR_RESTORE_MODEL       "Odtwórz model"
#define TR_DELETE_ERROR        "Delete error"
#define TR_CAT_NOT_EMPTY       "Category is not empty"
#define TR_SDCARD_ERROR        "Błąd karty SD"
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "No files on SD!"
#define TR_NO_SDCARD           "Brak karty SD"
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
#define TR_SDCARD_FULL                 "Karta Pełna "
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE        "Niekompatybilne"
#define TR_WARNING             "UWAGA"
#define TR_EEPROMWARN          "EEPROM"
#define TR_STORAGE_WARNING     "STORAGE"
#define TR_EEPROM_CONVERTING   "Konwersja EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD card conversion required"
#define TR_CONVERTING                  "Converting: "
#define TR_THROTTLE_UPPERCASE        "GAZ"
#define TR_ALARMSWARN          "ALARM"
#define TR_SWITCHWARN          TR("Przełą","Kontrola")
#define TR_FAILSAFEWARN        "FAILSAFE"
#define TR_TEST_WARNING        TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE        "Use for tests only"
#define TR_WRONG_SDCARDVERSION TR("Expected ver: ","Expected version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery low"
#define TR_WARN_MULTI_LOWPOWER         "Low power mode"
#define TR_BATTERY                     "BATTERY"
#define TR_WRONG_PCBREV        "Wrong PCB detected"
#define TR_EMERGENCY_MODE      "EMERGENCY MODE"
#define TR_PCBREV_ERROR        "PCB error"
#define TR_NO_FAILSAFE         "Brak Failsafe"
#define TR_KEYSTUCK            "Blokada klucza"
#define TR_INVERT_THR          TR("Odw.Gaz?","Odwróć gaz?")
#define TR_VOLUME              "Głośność"
#define TR_LCD                 "LCD"
#define TR_BRIGHTNESS          "Jasność"
#define TR_CPU_TEMP            "Temp. CPU\016>"
#define TR_CPU_CURRENT         "Prąd"
#define TR_CPU_MAH             "Zużycie"
#define TR_COPROC              "CoProc."
#define TR_COPROC_TEMP         "Temp. MB"
#define TR_CAPAWARNING         INDENT "Mała pojemność"
#define TR_TEMPWARNING         INDENT "Przegrzanie"
#define TR_TTL_WARNING         "Warning: use 3.3V logic levels"
#define TR_FUNC                "Funkc"
#define TR_V1                  "V1"
#define TR_V2                  "V2"
#define TR_DURATION            "Czas trwania"
#define TR_DELAY               "Opóźnienie"
#define TR_SD_CARD             "SD"
#define TR_SDHC_CARD           "SD-HC"
#define TR_NO_SOUNDS_ON_SD     "Brak dźwięków na SD"
#define TR_NO_MODELS_ON_SD     "Brak modelu na SD"
#define TR_NO_BITMAPS_ON_SD    "Brak obrazków na SD"
#define TR_NO_SCRIPTS_ON_SD    "Brak skryptów na SD"
#define TR_SCRIPT_SYNTAX_ERROR TR("Syntax error", "Skrypt:syntax error")
#define TR_SCRIPT_PANIC        "Skrypt:panic"
#define TR_SCRIPT_KILLED       "Skrypt:killed"
#define TR_SCRIPT_ERROR        "Nieznany błąd"
#define TR_PLAY_FILE           "Odtwórz"
#define TR_DELETE_FILE         "Skasuj"
#define TR_COPY_FILE           "Kopiuj"
#define TR_RENAME_FILE         "Zmień nazwę"
#define TR_ASSIGN_BITMAP       "Wybierz obrazek"
#define TR_ASSIGN_SPLASH       "Splash screen"
#define TR_EXECUTE_FILE        "Wykonaj"
#define TR_REMOVED             " skasowane"
#define TR_SD_INFO             "Informacje"
#define TR_SD_FORMAT           "Formatuj"
#define TR_NA                  "[X]"
#define TR_HARDWARE            "HARDWARE"
#define TR_FORMATTING          "Formatowanie.."
#define TR_TEMP_CALIB          "Temp. kalibracji"
#define TR_TIME                "Czas"
#define TR_MAXBAUDRATE         "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE         "Sample Mode"
#define TR_SAMPLE_MODES        "Normal","OneBit"
#define TR_SELECT_TEMPLATE_FOLDER "SELECT A TEMPLATE FOLDER:"
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
#define TR_BLUETOOTH_BAUDRATE   "Prędkość BT"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES      "---","Enabled"
#else
#define TR_BLUETOOTH_MODES      "---","Telemetry","Trainer"
#endif
#define TR_SD_INFO_TITLE       "SD INFO"
#define TR_SD_TYPE             "Typ:"
#define TR_SD_SPEED            "Prędkość:"
#define TR_SD_SECTORS          "Sektory :"
#define TR_SD_SIZE             "Wielkość:"
#define TR_TYPE                INDENT TR_SD_TYPE
#define TR_GLOBAL_VARS         "Zmienne Globalne"
#define TR_GVARS               "Zmien.Glob."
#define TR_GLOBAL_VAR          "Zmienna Globalna"
#define TR_MENU_GLOBAL_VARS      "ZMIENNE GLOBALNE"
#define TR_OWN                 "Własność"
#define TR_DATE                "Data"
#define TR_MONTHS              { "Jan", "Fev", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
#define TR_ROTARY_ENCODER      "R.E."
#define TR_INVERT_ROTARY       "Invert Rotary"
#define TR_CHANNELS_MONITOR    "MONITOR KANAŁÓW"
#define TR_MIXERS_MONITOR      "MONITOR MIKSER"
#define TR_PATH_TOO_LONG       "Ścieżka za długa"
#define TR_VIEW_TEXT           "Pokaż tekst"
#define TR_FLASH_BOOTLOADER    "Flash bootloader"
#define TR_FLASH_DEVICE                TR("Flash device","Flash device")
#define TR_FLASH_EXTERNAL_DEVICE "Sflashuj Moduł Zewnętrzny"
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
#define TR_FLASH_INTERNAL_MODULE "Sflashuj Moduł Wewnętrzny"
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Int. Multi", "Flash Internal Multi")
#define TR_FLASH_EXTERNAL_MODULE       "Flash external module"
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Ext. Multi", "Flash External Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flash External ELRS")
#define TR_FIRMWARE_UPDATE_ERROR TR("FW update Error","Firmware update error")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Flash successful"
#define TR_WRITING                     "Zapis...  "
#define TR_CONFIRM_FORMAT              "Zatwierdź Format?"
#define TR_INTERNALRF                  "Wewn.Moduł RF"
#define TR_INTERNAL_MODULE             TR("Int. module","Internal module")
#define TR_EXTERNAL_MODULE             TR("Ext. module","External module")
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX upgrade required"
#define TR_TELEMETRY_DISABLED          "Telem. disabled"
#define TR_MORE_OPTIONS_AVAILABLE      "More options available"
#define TR_NO_MODULE_INFORMATION       "No module information"
#define TR_EXTERNALRF          "Zewn.Moduł RF"
#define TR_FAILSAFE            TR(INDENT"Failsafe",INDENT"Tryb Failsafe")
#define TR_FAILSAFESET         "USTAWIENIE FAILSAFE"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Owner ID"
#define TR_PINMAPSET                   "PINMAP"
#define TR_HOLD                "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                "None"
#define TR_NONE_UPPERCASE              "NONE"
#define TR_MENUSENSOR          "CZUJNIK"
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
#define TR_COUNTRY_CODE         "Kod regionu"
#define TR_USBMODE             "USB Mode"
#define TR_JACK_MODE                    "Jack Mode"
#define TR_VOICE_LANGUAGE           "Język głosu"
#define TR_UNITS_SYSTEM         "Jednostki"
#define TR_EDIT                "Edytuj"
#define TR_INSERT_BEFORE       "Wstaw przed"
#define TR_INSERT_AFTER        "Wstaw za"
#define TR_COPY                "Kopiuj"
#define TR_MOVE                "Przenieś"
#define TR_PASTE               "Wklej"
#define TR_PASTE_AFTER                 "Paste After"
#define TR_PASTE_BEFORE                "Paste Before"
#define TR_DELETE              "Kasuj"
#define TR_INSERT              "Wstaw"
#define TR_RESET_FLIGHT        "Zeruj lot"
#define TR_RESET_TIMER1        "Zeruj Timer1"
#define TR_RESET_TIMER2        "Zeruj Timer2"
#define TR_RESET_TIMER3        "Zeruj Timer3"
#define TR_RESET_TELEMETRY     "Wyczyść telemetrię"
#define TR_STATISTICS          "Statystyki"
#define TR_ABOUT_US            "O nas"
#define TR_USB_JOYSTICK        "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE    "USB Storage (SD)"
#define TR_USB_SERIAL          "USB Serial (VCP)"
#define TR_SETUP_SCREENS       "Setup screens"
#define TR_MONITOR_SCREENS     "Monitors"
#define TR_AND_SWITCH          "Przełącznik AND"
#define TR_SF                  "FS"
#define TR_GF                  "FG"
#define TR_ANADIAGS_CALIB      "Calibrated analogs"
#define TR_ANADIAGS_FILTRAWDEV "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW  "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX     "Min., max. and range"
#define TR_ANADIAGS_MOVE       "Move analogs to their extremes!"
#define TR_SPEAKER             INDENT"Głośnik"
#define TR_BUZZER              INDENT"Brzęczyk"
#define TR_BYTES               "bajty"
#define TR_MODULE_BIND         TR("[BND]","[Bind]")
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
#define TR_REGISTER                    TR("Reg", "Register")
#define TR_MODULE_RANGE        TR("[Zas]","Zasięg")
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "RECEIVER OPTIONS")
#define TR_DEL_BUTTON                  BUTTON(TR("Del", "Delete"))
#define TR_RESET_BTN           "[Reset]"
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                BUTTON(TR("SW","Switches"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Analog","Analogs"))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_SET                 "[Set]"
#define TR_TRAINER             "Port Trenera"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM      CENTER "Problem z anteną TX"
#define TR_MODELIDUSED         TR("ID zajęte","ID modelu zajęte")
#define TR_MODULE              "Moduł "
#define TR_RX_NAME                     "Rx Name"
#define TR_TELEMETRY_TYPE      TR("Typ ", "Typ Telemetrii")
#define TR_TELEMETRY_SENSORS   "Czujniki"
#define TR_VALUE               "Wartość"
#define TR_REPEAT                      "Repeat"
#define TR_ENABLE                      "Enable"
#define TR_TOPLCDTIMER         "Top LCD Timer"
#define TR_UNIT                "Jedn"
#define TR_TELEMETRY_NEWSENSOR INDENT "Dodaj nowy czujnik..."
#define TR_CHANNELRANGE        TR(INDENT "ZakrKn",INDENT "Zakres kanału")
#define TR_RXFREQUENCY         TR("Rx Freqency", "Rx Servo Frequency")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_ANTENNACONFIRM1     "EXT. ANTENNA"
#if defined(PCBX12S)
#define TR_ANTENNA_MODES       "Internal","Ask","Per model","Internal + External"
#else
#define TR_ANTENNA_MODES       "Internal","Ask","Per model","External"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Use int. antenna", "Use internal antenna")
#define TR_USE_EXTERNAL_ANTENNA        TR("Use ext. antenna", "Use external antenna")
#define TR_ANTENNACONFIRM2     TR("Check antenna", "Make sure antenna is installed!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1                "Requires non"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1         "Requires FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1          "Requires EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2                "certified firmware"
#define TR_LOWALARM            INDENT "Alarm niski"
#define TR_CRITICALALARM       INDENT "Alarm krytyczny"
#define TR_RSSIALARM_WARN             TR("RSSI","TELEMETRY RSSI")
#define TR_NO_RSSIALARM                TR(INDENT "Alarms disabled", INDENT "Telemetry alarms disabled")
#define TR_DISABLE_ALARM               TR(INDENT "Disable alarms", INDENT "Disable telemetry alarms")
#define TR_ENABLE_POPUP        "Aktywuj Popup"
#define TR_DISABLE_POPUP       "Wyłącz Popup"
#define TR_POPUP               "Popup"
#define TR_MIN                 "Min"
#define TR_MAX                 "Max"
#define TR_CURVE_PRESET        "Predef..."
#define TR_PRESET              "Predef."
#define TR_MIRROR              "Lustro"
#define TR_CLEAR               "Wyczyść"
#define TR_RESET               "Resetuj"
#define TR_RESET_SUBMENU       "Reset..."
#define TR_COUNT               "Punkty"
#define TR_PT                  "pkt"
#define TR_PTS                 "pkty"
#define TR_SMOOTH              "Gładka"
#define TR_COPY_STICKS_TO_OFS  TR("Cpy stick->subtrim", "Skopiuj Drążki Do Offsetu")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
#define TR_COPY_TRIMS_TO_OFS   TR("Cpy trim->subtrim", "Skopiuj Trymery Do subtrymerów")
#define TR_INCDEC              "Zwiększ/Zmnie"
#define TR_GLOBALVAR           "Zm.Global."
#define TR_MIXSOURCE           "Źródło Miks."
#define TR_CONSTANT            "Stala   "
#define TR_PERSISTENT_MAH      TR(INDENT "Zap. mAh", INDENT "Zapisz mAh")
#define TR_PREFLIGHT           "Lista Ostrzeżeń"
#define TR_CHECKLIST           TR(INDENT "Czeklista",INDENT "Pokaż Listę Ostrzeżeń")
#define TR_FAS_OFFSET          TR(INDENT "FAS Ofs", INDENT "FAS Offset")
#define TR_AUX_SERIAL_MODE     "Port szreg."
#define TR_AUX2_SERIAL_MODE    "Port szreg. 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port power"
#define TR_SCRIPT              "Skrypt"
#define TR_INPUTS              "Wejścia"
#define TR_OUTPUTS             "Wyjścia"
#define STR_EEBACKUP            "Backup EEPROMu"
#define STR_FACTORYRESET        "Ustaw.Fabrycz"
#define TR_CONFIRMRESET        "WYkasować wszytkie modele?    "
#define TR_TOO_MANY_LUA_SCRIPTS "Za dużo skryptów Lua!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "No Telemetry Screens"
#define TR_TOUCH_PANEL                 "Touch panel:"
#define TR_FILE_SIZE                   "File size"
#define TR_FILE_OPEN                   "Open anyway?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Nazwa"
#define TR_PHASES_HEADERS_SW           "Przełą"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trymer SK"
#define TR_PHASES_HEADERS_ELE_TRIM     "Trymer SW"
#define TR_PHASES_HEADERS_THT_TRIM     "Trymer Gazu"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trymer Lotek"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trymer 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trymer 6"
#define TR_PHASES_HEADERS_FAD_IN       "Pojawian"
#define TR_PHASES_HEADERS_FAD_OUT      "Zanik"

#define TR_LIMITS_HEADERS_NAME         "Nazwa"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Kierunek"
#define TR_LIMITS_HEADERS_CURVE        "Krzywa"
#define TR_LIMITS_HEADERS_PPMCENTER    "Środek PPM"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Tryb subtrim"
#define TR_INVERTED     "Inverted"

#define TR_LSW_HEADERS_FUNCTION        "Funkcja"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "AND Przełą"
#define TR_LSW_HEADERS_DURATION        "CzasTrwa"
#define TR_LSW_HEADERS_DELAY           "Opóźn"

#define TR_GVAR_HEADERS_NAME          "Name"
#define TR_GVAR_HEADERS_FM0           "Value on FM0"
#define TR_GVAR_HEADERS_FM1           "Value on FM1"
#define TR_GVAR_HEADERS_FM2           "Value on FM2"
#define TR_GVAR_HEADERS_FM3           "Value on FM3"
#define TR_GVAR_HEADERS_FM4           "Value on FM4"
#define TR_GVAR_HEADERS_FM5           "Value on FM5"
#define TR_GVAR_HEADERS_FM6           "Value on FM6"
#define TR_GVAR_HEADERS_FM7           "Value on FM7"
#define TR_GVAR_HEADERS_FM8           "Value on FM8"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS    { "Comparison type or function", "First variable", "Second variable or constant", "Second variable or constant", "Additional condition for line to be enabled", "Minimum ON duration of the logical switch", "Minimum TRUE duration for the switch to become ON" }

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

// About screen
#define TR_ABOUTUS             TR("   O   ", "  O  ")

#define TR_CHR_SHORT           'k'
#define TR_CHR_LONG            'd'
#define TR_CHR_TOGGLE          't'
#define TR_CHR_HOUR            'h'
#define TR_CHR_INPUT           'I'   // Values between A-I will work

#define TR_BEEP_VOLUME         "Głośn pikan"
#define TR_WAV_VOLUME          "Głośn wav  "
#define TR_BG_VOLUME           "Głośn tła"

#define TR_TOP_BAR             "GórnPas"
#define TR_FLASH_ERASE                 "Flash erase..."
#define TR_FLASH_WRITE                 "Flash write..."
#define TR_OTA_UPDATE                  "OTA update..."
#define TR_MODULE_RESET                "Module reset..."
#define TR_UNKNOWN_RX                  "Unknown RX"
#define TR_UNSUPPORTED_RX              "Unsupported RX"
#define TR_OTA_UPDATE_ERROR            "OTA update error"
#define TR_DEVICE_RESET                "Device reset..."
#define TR_ALTITUDE            INDENT "Wysokość"
#define TR_SCALE               "Skala"
#define TR_VIEW_CHANNELS       "Pokaż Kanały"
#define TR_VIEW_NOTES          "Pokaż Notki"
#define TR_MODEL_SELECT        "Model Select"
#define TR_MODS_FORBIDDEN      "Modyfikacje zabronione!"
#define TR_UNLOCKED            "Odblokowane"
#define TR_ID                  "ID"
#define TR_PRECISION           "Precyzja "
#define TR_RATIO               "Współcz."
#define TR_FORMULA             "Formuła"
#define TR_CELLINDEX           "Cell index"
#define TR_LOGS                "Logi"
#define TR_OPTIONS             "Opcje  "
#define TR_FIRMWARE_OPTIONS    "Firmware options"

#define TR_ALTSENSOR           "Alt sensor"
#define TR_CELLSENSOR          "Cell sensor"
#define TR_GPSSENSOR           "GPS sensor"
#define TR_CURRENTSENSOR       "Sensor"
#define TR_AUTOOFFSET          "Auto Ofset"
#define TR_ONLYPOSITIVE        "Dodatni"
#define TR_FILTER              "Filtr"
#define TR_TELEMETRYFULL       "Wszyskie miejsca zajęte!"
#define TR_SERVOS_OK           "Servos OK"
#define TR_SERVOS_KO           "Servos KO"
//TODO: translation
#define TR_INVERTED_SERIAL     INDENT "Invert"
#define TR_IGNORE_INSTANCE     INDENT "Ignoruj przypadek"
#define TR_DISCOVER_SENSORS    "Znajdź nowe czujniki"
#define TR_STOP_DISCOVER_SENSORS "Szukanie STOP "
#define TR_DELETE_ALL_SENSORS  "Usuń czujniki     "
#define TR_CONFIRMDELETE       "Usunąć wszystkie?"
#define TR_SELECT_WIDGET       "Select widget"
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

#define TR_MENU_INPUTS          STR_CHAR_INPUT "Wejści"
#define TR_MENU_LUA             STR_CHAR_LUA "SkryptyLUA"
#define TR_MENU_STICKS           STR_CHAR_STICK "Drążki"
#define TR_MENU_POTS            STR_CHAR_POT "Pots"
#define TR_MENU_MAX             STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI            STR_CHAR_CYC "Cyclic"
#define TR_MENU_TRIMS            STR_CHAR_TRIM "Trymy"
#define TR_MENU_SWITCHES        STR_CHAR_SWITCH "Przełczn"
#define TR_MENU_LOGICAL_SWITCHES  STR_CHAR_SWITCH "Przełączn.Logicz"
#define TR_MENU_TRAINER         STR_CHAR_TRAINER "Trener "
#define TR_MENU_CHANNELS        STR_CHAR_CHANNEL "Kanały  "
#define TR_MENU_GVARS           STR_CHAR_SLIDER "ZmGlo"
#define TR_MENU_TELEMETRY       STR_CHAR_TELEMETRY "Telemetry"
#define TR_MENU_DISPLAY        "DISPLAY"
#define TR_MENU_OTHER          "Inny "
#define TR_MENU_INVERT         "Odwróć"
#define TR_JITTER_FILTER       "ADC Filter"
#define TR_DEAD_ZONE           "Dead zone"
#define TR_RTC_CHECK           TR("Check RTC", "Check RTC voltage")
#define TR_AUTH_FAILURE                "Auth-failure"
#define TR_RACING_MODE                 "Racing mode"

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
