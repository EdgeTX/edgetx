/*  translation by adir kahsharo
 *  make configure system by stav raviv
 *  Ver 1.023
 *
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
// For this number of minute in the last decimal place singular form is used in plural
#define TR_USE_SINGULAR_IN_PLURAL   0
#define TR_USE_PLURAL2_SPECIAL_CASE  0
// If the number of minutes is above this value PLURAL2 is used
#define TR_USE_PLURAL2  INT_MAX

#define TR_MINUTE_SINGULAR            "דקה"
#define TR_MINUTE_PLURAL1             "דקות"
#define TR_MINUTE_PLURAL2             "דקות"

#define TR_OFFON                       "OFF","ON"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Quiet","Alarm","NoKey","All"
#define TR_VBLMODE                     "OFF","Keys",TR("Ctrl","Controls"),"Both","ON"
#define TR_TRNMODE                     "OFF",TR("+=","הוספה"),TR(":=","החלפה")
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"

#define TR_AUX_SERIAL_MODES            "OFF","Telem Mirror","Telemetry In","SBUS Trainer","LUA","CLI","GPS","Debug","SpaceMouse","External module"
#define TR_SWTYPES                     "None","Toggle","2POS","3POS"
#define TR_POTTYPES                    "None","Pot",TR("Pot w. det","Pot with detent"),"Slider",TR("Multipos","Multipos Switch"),"Axis X","Axis Y","Switch"
#define TR_VPERSISTENT                 "OFF","Flight","Manual Reset"
#define TR_COUNTRY_CODES               TR("US","America"),TR("JP","Japan"),TR("EU","Europe")
#define TR_USBMODES                    "הצג לי אופציות",TR("Joyst","חיבור משחק קבוע"),TR("SDCard","העברת נתונים קבוע"),"חיבור סיריילי קבוע"
#define TR_JACK_MODES                  "Ask","Audio","Trainer"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_CUSTOM                "עיצוב מיוחד"
#define TR_VTRIMINC                    TR("Expo","הולך ומתעצם"),TR("ExFine","זז ב-1"),"זז ב-2","זז ב-3","זז ב-6"
#define TR_VDISPLAYTRIMS               "אל תציג","הצג שינוי בלבד","הצג"
#define TR_VBEEPCOUNTDOWN              "שקט","ציפצופים","שמע","לחן",TR("B & H","Beeps & Haptic"),TR("V & H","Voice & Haptic")
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VVARIOCENTER                "Tone","Silent"
#define TR_CURVE_TYPES                 "סטנדרטי","עיצוב מיוחד"

#define TR_ADCFILTERVALUES             "Global","Off","On"

#define TR_VCURVETYPE                  "דיפרנציאלי","אקספו","פונקציה","עריכת מיוחד"
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "אוסף תנועה","צרף תנועות יחד","החלף תנועה"
#define TR_VMLTPX2                     "+=","*=",":="

#if LCD_W >= 212
  #define TR_CSWTIMER                  "שעון"
  #define TR_CSWSTICKY                 "Stcky"
  #define TR_CSWSTAY                   "Edge"
#else
  #define TR_CSWTIMER                  "שעון"
  #define TR_CSWSTICKY                 "Stky"
  #define TR_CSWSTAY                   "Edge"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Trainer"
#define TR_SF_INST_TRIM                "Inst. Trim"
#define TR_SF_RESET                    "Reset"
#define TR_SF_SET_TIMER                "Set"
#define TR_SF_VOLUME                   "Volume"
#define TR_SF_FAILSAFE                 "SetFailsafe"
#define TR_SF_RANGE_CHECK              "RangeCheck"
#define TR_SF_MOD_BIND                 "ModuleBind"
#define TR_SF_RGBLEDS                  "RGB leds"

#define TR_SOUND                       "Play Sound"
#define TR_PLAY_TRACK                  TR("Ply Trk", "Play Track")
#define TR_PLAY_VALUE                  TR("Play Val","Play Value")
#define TR_SF_HAPTIC                   "Haptic"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua Script")
#define TR_SF_BG_MUSIC                 "BgMusic"
#define TR_SF_BG_MUSIC_PAUSE           "BgMusic ||"
#define TR_SF_LOGS                     "SD Logs"
#define TR_ADJUST_GVAR                 "Adjust"
#define TR_SF_BACKLIGHT                "Backlight"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "Test"

#if LCD_W >= 212
  #define TR_SF_SAFETY                 "Override"
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "Overr."
#endif

#define TR_SF_SCREENSHOT               "צילום מסך"
#define TR_SF_RACING_MODE              "מצב תחרות"
#define TR_SF_DISABLE_TOUCH            "ללא מסך מגע"
#define TR_SF_DISABLE_AUDIO_AMP        "Audio Amp Off"
#define TR_SF_SET_SCREEN               "הגדרת מסך ראשי"
#define TR_SF_SET_TELEMSCREEN          "Set Screen"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Telm", "טלמטריה")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS            "שעון 1","שעון 2","שעון 3"


#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("All","Flight"),TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "תצוגת מילימטר",TR("Imper.","תצוגת אינצ'ים")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "None","Nums","Bars","Script"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "כיוון"
#define TR_STICK_NAMES1                "ה.גוב"
#define TR_STICK_NAMES2                "מצערת"
#define TR_STICK_NAMES3                "מאזנות"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Default","High"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ON","One"

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                   "מצב כובעונים"
#define TR_HATSOPT                    "קיזוזים בלבד","ניווט בלבד","משולב","גלובאלי"
#define TR_HATSMODE_TRIMS             "מצב כובעונים: קיזוזים"
#define TR_HATSMODE_KEYS              "מצב כובעונים: ניווט"
#define TR_HATSMODE_KEYS_HELP          "Left side:\n"\
                                       "   Right = MDL\n"\
                                       "   Up = SYS\n"\
                                       "   Down = TELE\n"\
                                       "\n"\
                                       "Right side:\n"\
                                       "   Left = PAGE<\n"\
                                       "   Right = PAGE>\n"\
                                       "   Up = PREV/INC\n"\
                                       "   Down = NEXT/DEC"
#endif

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "רגיל","הפוך"
#else
  #define TR_ROTARY_ENC_OPT         "רגיל","הפוך","V-I H-N","V-I H-A","V-N E-I"
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


#define TR_SRC_BATT                    "Batt"
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES                   "OFF","ON","Strt","THs","TH%","THt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "שלט ראשי"
#define TR_VTRAINER_SLAVE_JACK         "שלט חניך"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Master/SBUS Module"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Master/CPPM Module"
#define TR_VTRAINER_MASTER_BATTERY     "Master/Serial"
#define TR_VTRAINER_BLUETOOTH          "Master/" TR("BT","Bluetooth"), "Slave/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VFAILSAFE                   "לא מוגדר","מוחזק","הגדרה ידנית - מומלץ!","ללא שינוי","מקלט"
#define TR_VSENSORTYPES                "Custom","Calculated"
#define TR_VFORMULAS                   "Add","Average","Min","Max","Multiply","Totalize","Cell","Consumpt","Distance"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Lowest","1","2","3","4","5","6","7","8","Highest","Delta"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center only)","= (symmetrical)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")
#define TR_PPMUNIT                     "0.--","0.0","us"

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "STD"
#else
#define TR_FONT_SIZES                  "STD","BOLD","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14) || defined(PCBPL18)
  #define TR_ENTER                     "[NEXT]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#if defined(COLORLCD)
  #define TR_EXIT                      "RTN"
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "חזרה"
  #define TR_OK                        TR("\010" "\010" "\010" "[OK]", "\010" "\010" "\010" "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_FREE                        "חופשי"
#define TR_YES                         "כן"
#define TR_NO                          "לא"
#define TR_DELETEMODEL                 "! מחיקת מודל"
#define TR_COPYINGMODEL                "...מעתיק מודל"
#define TR_MOVINGMODEL                 "...מעביר מודל"
#define TR_LOADINGMODEL                "...טוען מודל"
#define TR_UNLABELEDMODEL              "Unlabeled"
#define TR_NAME                        "שם"
#define TR_MODELNAME                   "שם המודל"
#define TR_PHASENAME                   "שם המצב"
#define TR_MIXNAME                     "שם המיקס"
#define TR_INPUTNAME                   TR("Input", "Input name")
#define TR_EXPONAME                    TR("Name", "Line name")
#define TR_BITMAP                      "תמונה מודל"
#define TR_NO_PICTURE                  "אין תמונה"
#define TR_TIMER                       TR("שעון", "שעון ")
#define TR_START                       "התחלה"
#define TR_ELIMITS                     TR("E.Limits", "הרחב תצוגות")
#define TR_ETRIMS                      TR("E.Trims", "הרחב תצוגות")
#define TR_TRIMINC                     "רגישות קיזוז"
#define TR_DISPLAY_TRIMS               TR("Show Trims", "הצג קיזוזים")
#define TR_TTRACE                      TR("T-Source", "מקור")
#define TR_TTRIM                       TR("T-Trim-Idle", "Trim idle only")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", "מתג קיזוז")
#define TR_BEEPCTR                     TR("Ctr Beep", "ציפצוף במרכז")
#define TR_USE_GLOBAL_FUNCS            TR("Glob.Funcs", "שימוש בפונקציות גלובליות")
#define TR_PROTOCOL                    TR("Proto", "Protocol")
#define TR_PPMFRAME                    "PPM frame"
#define TR_REFRESHRATE                 TR("Refresh", "Refresh rate")
#define STR_WARN_BATTVOLTAGE           TR("Output is VBAT: ", "Warning: output level is VBAT: ")
#define TR_WARN_5VOLTS                 "Warning: output level is 5 volts"
#define TR_MS                          "ms"
#define TR_SWITCH                      "מתג"
#define TR_FUNCTION_SWITCHES           "מפסקים בהתאמה אישית"
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
#define TR_SF_SWITCH                   "הדק"
#define TR_TRIMS                       "קיזוזים"
#define TR_FADEIN                      "Fade in"
#define TR_FADEOUT                     "Fade out"
#define TR_DEFAULT                     "(ברירת מחדל)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Check FM Trims"
#else
  #define TR_CHECKTRIMS                "\006Check\012trims"
#endif
#define TR_SWASHTYPE                   "Swash Type"
#define TR_COLLECTIVE                  TR("Collective", "Coll. pitch source")
#define TR_AILERON                     TR("Lateral cyc.", "Lateral cyc. source")
#define TR_ELEVATOR                    TR("Long. cyc.", "Long. cyc. source")
#define TR_SWASHRING                   "Swash Ring"
#define TR_MODE                        "Mode"
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "Left"
#else
  #define TR_LEFT_STICK                "Left"
#endif
#define TR_SUBTYPE                     "Subtype"
#define TR_NOFREEEXPO                  "No free expo!"
#define TR_NOFREEMIXER                 "No free mixer!"
#define TR_SOURCE                       "מקור"
#define TR_WEIGHT                      "כמות"
#define TR_SIDE                        "Side"
#define TR_OFFSET                       "Offset"
#define TR_TRIM                        "קיזוז"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "עקומה"
#define TR_FLMODE                      TR("מצב", "מצבים")
#define TR_MIXWARNING                  "התראה"
#define TR_OFF                         "OFF"
#define TR_ANTENNA                     "אנטנה"
#define TR_NO_INFORMATION              TR("No info", "אין מידע")
#define TR_MULTPX                      "הגדר מצב מיקס"
#define TR_DELAYDOWN                   TR("Delay dn", "עיקוב בירידה")
#define TR_DELAYUP                     "עיקוב בעלייה"
#define TR_SLOWDOWN                    TR("Slow dn", "האט ירידה")
#define TR_SLOWUP                      "האט עלייה"
#define TR_MIXES                       "מיקסים"
#define TR_CV                          "CV"
#if defined(PCBNV14)
#define TR_GV                          "GV"
#else
#define TR_GV                          TR("G", "GV")
#endif
#define TR_RANGE                       "טווח"
#define TR_CENTER                      "מרכז"
#define TR_ALARM                       "התראה"
#define TR_BLADES                      "Blades/Poles"
#define TR_SCREEN                      "Screen\001"
#define TR_SOUND_LABEL                 "צליל"
#define TR_LENGTH                      "Length"
#define TR_BEEP_LENGTH                 "Beep length"
#define TR_BEEP_PITCH                  "Beep pitch"
#define TR_HAPTIC_LABEL                "Haptic"
#define TR_STRENGTH                    "Strength"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Offset"
#define TR_IMU_MAX                     "מקס"
#define TR_CONTRAST                    "Contrast"
#define TR_ALARMS_LABEL                "התראות"
#define TR_BATTERY_RANGE               TR("Batt. range", "הטווח להתראת סוללה")
#define TR_BATTERYCHARGING             "השלט מחובר להטענה..."
#define TR_BATTERYFULL                 "סוללה מלאה"
#define TR_BATTERYNONE                 "None!"
#define TR_BATTERYWARNING              "התראת סוללה חלשה"
#define TR_INACTIVITYALARM             "התראת אי שימוש ממושכת"
#define TR_MEMORYWARNING               "נפח זיכרון נמוך"
#define TR_ALARMWARNING                "sound off"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI shutdown", "בדיקת חיבור למקלט בכיבוי")
#define TR_TRAINER_SHUTDOWN_ALARM          TR("Trainer alarm", "check trainer connection")
#define TR_MODEL_STILL_POWERED         "מודל עדיין פעיל"
#define TR_TRAINER_STILL_CONNECTED     "מצב חניך עדיין מחובר"
#define TR_USB_STILL_CONNECTED         "חיבור עדיין מחובר"
#define TR_MODEL_SHUTDOWN              "?לכבות"
#define TR_PRESS_ENTER_TO_CONFIRM      "לחץ אנטר לאישור"
#define TR_THROTTLE_LABEL              "מצערת"
#define TR_THROTTLE_START              "מצערת מתחילה"
#define TR_THROTTLEREVERSE             TR("T-Reverse", "היפוך")
#define TR_MINUTEBEEP                  TR("דקה", "הקראת דקות")
#define TR_BEEPCOUNTDOWN               "ספירה לאחור"
#define TR_PERSISTENT                  TR("Persist.", "Persistent")
#define TR_BACKLIGHT_LABEL             "תאורת רקע"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "סטטוס"
#define TR_BLONBRIGHTNESS              "בהירות פעילה"
#define TR_BLOFFBRIGHTNESS             "בהירות כבויה"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR                     "צבע"
#define TR_SPLASHSCREEN                "Splash screen"
#define TR_PLAY_HELLO                  "צליל אתחול"
#define TR_PWR_ON_DELAY                "השהיית הפעלה פעיל"
#define TR_PWR_OFF_DELAY               "משך לחיצה לכיבוי השלט"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("Pwr ON/OFF Haptic","Power ON/OFF Haptic")
#define TR_THROTTLE_WARNING            TR("T-Warning", "התראת מצערת פתוחה")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Cust-Pos", "עריכת מיקום ידנית")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR("S-Warning", "מיקום מתגים")
#define TR_POTWARNINGSTATE             "סליידרים וגלגלות"
#define TR_POTWARNING                  TR("Pot warn.", "Pot positions")
#define TR_TIMEZONE                    "אזור זמן"
#define TR_ADJUST_RTC                  "Adjust RTC"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Def chan order", "Default channel order")
#define TR_STICKS                      "סטיקים"
#define TR_POTS                        "גלגלות"
#define TR_SWITCHES                    "מתגים"
#define TR_SWITCHES_DELAY              TR("Play delay", "השהיית השמעת מתג")
#define TR_SLAVE                       "Slave"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "Multiplier"
#define TR_CAL                         "Cal"
#define TR_CALIBRATION                 "כיול"
#define TR_VTRIM                       "קיזוז - +"
#define TR_CALIB_DONE                  "כיול הושלם"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Press [Enter] to start"
  #define TR_SETMIDPOINT               "Center sticks/pots/sliders and press [Enter]"
  #define TR_MOVESTICKSPOTS            "Move sticks/pots/sliders and press [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " TO START"
  #define TR_SETMIDPOINT               "CENTER STICKS/SLIDERS"
  #define TR_MOVESTICKSPOTS            "MOVE STICKS/POTS"
#else
  #define TR_MENUTOSTART               TR_ENTER " TO START"
  #define TR_SETMIDPOINT               TR("SET STICKS MIDPOINT", "CENTER STICKS/SLIDERS")
  #define TR_MOVESTICKSPOTS            "MOVE STICKS/POTS"
  #define TR_MENUWHENDONE              TR_ENTER " WHEN DONE"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      "NO DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua scripts"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_DURATION_MS                 TR("[D]","Duration(ms): ")
#define TR_INTERVAL_MS                 TR("[I]","Interval(ms): ")
#define TR_MEM_USED_SCRIPT             "Script(B): "
#define TR_MEM_USED_WIDGET             "Widget(B): "
#define TR_MEM_USED_EXTRA              "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Audio: "
#define TR_GPS_FIX_YES                 "Fix: Yes"
#define TR_GPS_FIX_NO                  "Fix: No"
#define TR_GPS_SATS                    "Sats: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Menu: "
#define TR_TIMER_LABEL                 "שעון"
#define TR_THROTTLE_PERCENT_LABEL      "מצערת %"
#define TR_BATT_LABEL                  "סוללה"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " to reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "ערוץ "
#define TR_MODEL                       "מודל"
#if defined(SURFACE_RADIO)
#define TR_FM                          "DM"
#else
#define TR_FM                          "FM"
#endif
#define TR_EEPROMLOWMEM                "EEPROM low mem"
#define TR_PRESS_ANY_KEY_TO_SKIP       "לחץ על המסך או על כפתור"
#define TR_THROTTLE_NOT_IDLE           "סטיק מצערת פתוח"
#define TR_ALARMSDISABLED              "התראות בוטלו"
#define TR_PRESSANYKEY                 TR("\010Press any Key", "Press any key")
#define TR_BADEEPROMDATA               "Bad EEprom data"
#define TR_BAD_RADIO_DATA              "Bad radio data"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            "Formatting EEPROM"
#define TR_STORAGE_FORMAT              "Storage preparation"
#define TR_EEPROMOVERFLOW              "EEPROM overflow"
#define TR_RADIO_SETUP                 "הגדרות שלט ותצוגה"
#define TR_MENUTRAINER                 "טריינר"
#define TR_MENUSPECIALFUNCS            "פונקציות לכל המודלים והשלט"
#define TR_MENUVERSION                 "גירסא"
#define TR_MENU_RADIO_SWITCHES         TR("בדיקת מתגים וכפתורים", "SWITCHES")
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "בדיקת אנלוגיות")
#define TR_MENU_RADIO_ANALOGS_CALIB    "בדיקת אנלוגיות סטיקים ומגע"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION              "כיול"
#define TR_MENU_FSWITCH                 "מפסקים בהתאמה אישית"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS             "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE           "הגדר מצב נוכחי כקבוע"
#define TR_CHANNEL2FAILSAFE            "הגדר מצב נוכחי כקבוע"
#define TR_MENUMODELSEL                TR("MODELSEL", "MODEL SELECTION")
#define TR_MENU_MODEL_SETUP            TR("הגדרות מודל", "הגדרות מודל")
#if defined(SURFACE_RADIO)
#define TR_MENUFLIGHTMODES             "DRIVE MODES"
#define TR_MENUFLIGHTMODE              "DRIVE MODE"
#else
#define TR_MENUFLIGHTMODES             "מצבי טיסה"
#define TR_MENUFLIGHTMODE              "מצב טיסה"
#endif
#define TR_MENUHELISETUP               "הגדרות מסוק"

  #define TR_MENUINPUTS                "כניסות"
  #define TR_MENULIMITS                "יציאות"
#define TR_MENUCURVES                  "עקומות"
#define TR_MENUCURVE                   "עקומה"
#define TR_MENULOGICALSWITCH           "מתג לוגי"
#define TR_MENULOGICALSWITCHES         "מתגים לוגים"
#define TR_MENUCUSTOMFUNC              "אופציות מיוחדות למודל"
#define TR_MENUCUSTOMSCRIPTS           "סקריפטים מיוחדים"
#define TR_MENUTELEMETRY               "טלמטריה"
#define TR_MENUSTAT                    "סטטוס"
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS1           "מצגי ערוצים 8-1"
#define TR_MONITOR_CHANNELS2           "מצגי ערוצים 16-9"
#define TR_MONITOR_CHANNELS3           "מצגי ערוצים 24-17"
#define TR_MONITOR_CHANNELS4           "מצגי ערוצים 32-25"
#define TR_MONITOR_SWITCHES            "מסך מתגים לוגים"
#define TR_MONITOR_OUTPUT_DESC         "יציאות"
#define TR_MONITOR_MIXER_DESC          "מיקסרים"
  #define TR_RECEIVER_NUM              TR("RxNum", "Receiver No.")
  #define TR_RECEIVER                  "מספר מקלט"
#define TR_MULTI_RFTUNE                TR("Freq tune", "מיקוד התדר (fine-tuning)")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "יציאה"
#define TR_MULTI_TELEMETRY             "טלמטריה"
#define TR_MULTI_VIDFREQ               TR("Vid. freq.", "Video frequency")
#define TR_RF_POWER                    "RF Power"
#define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")
#define TR_MULTI_OPTION                TR("Option", "Option value")
#define TR_MULTI_AUTOBIND              TR("Bind Ch.", "צימוד על ערוץ")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "השבתת מיפוי ערוצים")
#define TR_DISABLE_TELEM               TR("No Telem", "השבתת טלמטריה")
#define TR_MULTI_LOWPOWER              TR("Low power", "מצב מתח נמוך")
#define TR_MULTI_LNA_DISABLE           "LNA disable"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("ON", "Enabled")
#define TR_DISABLE_INTERNAL            TR("Disable int.", "Disable internal RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("!serial mode", "Not in serial mode")
#define TR_MODULE_NO_INPUT             TR("No input", "No serial input")
#define TR_MODULE_NO_TELEMETRY         TR3("No telemetry", "No MULTI_TELEMETRY", "No MULTI_TELEMETRY detected")
#define TR_MODULE_WAITFORBIND          "Bind to load protocol"
#define TR_MODULE_BINDING              TR("Bind...","Binding")
#define TR_MODULE_UPGRADE_ALERT        TR3("Upg. needed", "Module upgrade required", "Module\nUpgrade required")
#define TR_MODULE_UPGRADE              TR("Upg. advised", "Module update recommended")
#define TR_REBIND                      "Rebinding required"
#define TR_REG_OK                      "Registration ok"
#define TR_BIND_OK                     "הצימוד הצליח"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem OFF"
#define TR_PROTOCOL_INVALID            TR("Prot. invalid", "Protocol invalid")
#define TR_MODULE_STATUS               TR("Status", "גרסאת מולטי פרוטוקול")
#define TR_MODULE_SYNC                 TR("Sync", "Proto Sync Status")
#define TR_MULTI_SERVOFREQ             TR("Servo rate", "Servo update rate")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("RF Channel", "Select RF channel")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_GPS_COORDS_FORMAT           TR("GPS Coords", "Coordinate format")
#define TR_VARIO                       TR("Vario", "הגדרות תדר")
#define TR_PITCH_AT_ZERO               "תדר מינימלי"
#define TR_PITCH_AT_MAX                "תדר מקסימלי"
#define TR_REPEAT_AT_ZERO              "השהיית התראה"
#define TR_BATT_CALIB                  TR("Batt. calib", "כיול מתח סוללה ידני")
#define TR_CURRENT_CALIB               "Current calib"
#define TR_VOLTAGE                     TR("Voltage", "Voltage source")
#define TR_SELECT_MODEL                "בחירת מודל"
#define TR_MANAGE_MODELS               "ניהול מודלים"
#define TR_MODELS                      "מודלים"
#define TR_SELECT_MODE                 "בחירת מצב"
#define TR_CREATE_MODEL                "יצירת מודל"
#define TR_FAVORITE_LABEL              "מעודפים"
#define TR_MODELS_MOVED                "Unused models moved to"
#define TR_NEW_MODEL                   "מודל חדש"
#define TR_INVALID_MODEL               "Invalid Model"
#define TR_EDIT_LABELS                 "עריכת לשונית"
#define TR_LABEL_MODEL                 "שינוי לשונית"
#define TR_MOVE_UP                     "עלה למעלה"
#define TR_MOVE_DOWN                   "רד למטה"
#define TR_ENTER_LABEL                 "הוספת לשונית"
#define TR_LABEL                       "לשונית"
#define TR_LABELS                      "לשוניות"
#define TR_CURRENT_MODEL               "Current"
#define TR_ACTIVE                      "פעיל"
#define TR_NEW                         "חדש"
#define TR_NEW_LABEL                   "לשונית חדשה"
#define TR_RENAME_LABEL                "שינוי שם לשונית"
#define TR_DELETE_LABEL                "מחיקת לשונית"
#define TR_DUPLICATE_MODEL             "הכפלת מודל"
#define TR_COPY_MODEL                  "העתקת מודל"
#define TR_MOVE_MODEL                  "העברת מודל"
#define TR_BACKUP_MODEL                "גיבוי מודל"
#define TR_DELETE_MODEL                "מחיקת מודל"
#define TR_RESTORE_MODEL               "שיחזור מודל"
#define TR_DELETE_ERROR                "שיגאת מחיקה"
#define TR_SDCARD_ERROR                TR("SD תקלת", "תיקיות כרטיס SD")
#define TR_SDCARD                      "SD כרטיס"
#define TR_NO_FILES_ON_SD              "!SD אין קצבים על"
#define TR_NO_SDCARD                   "SD אין כרטיס"
#define TR_WAITING_FOR_RX              "Waiting for RX..."
#define TR_WAITING_FOR_TX              "Waiting for TX..."
#define TR_WAITING_FOR_MODULE          TR("Waiting module", "Waiting for module...")
#define TR_NO_TOOLS                    "No tools available"
#define TR_NORMAL                      "רגיל"
#define TR_NOT_INVERTED                "Not inv"
#define TR_NOT_CONNECTED               TR("!Connected", "Not Connected");
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
#define TR_MODULES_RX_VERSION          "ריכוז מידע חשוב על המערכת"
#define TR_SHOW_MIXER_MONITORS         "הצג מצגי מיקסים"
#define TR_MENU_MODULES_RX_VERSION     "MODULES / RX VERSION"
#define TR_MENU_FIRM_OPTIONS           "FIRMWARE OPTIONS"
#define TR_IMU                        "IMU"
#define TR_STICKS_POTS_SLIDERS         "Sticks/Pots/Sliders"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Sticks/Pots/Sliders"
#define TR_RF_PROTOCOL                 "RF פרוטוקול"
#define TR_MODULE_OPTIONS              "אפשריות מודל"
#define TR_POWER                       "Power"
#define TR_NO_TX_OPTIONS               "No TX options"
#define TR_RTC_BATT                    "RTC Batt"
#define TR_POWER_METER_EXT             "Power Meter (EXT)"
#define TR_POWER_METER_INT             "Power Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Spectrum (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Spectrum (INT)"
#define TR_SDCARD_FULL                  "הדיסק מלא״"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT TR_SDCARD_FULL "\לוגים ושמירת צילומי מסך מושבתים"
#else
#define TR_SDCARD_FULL_EXT TR_SDCARD_FULL "\036לוגים" LCDW_128_LINEBREAK "ושמירת צילומי מסך מושבתים"
#endif
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Incompatible"
#define TR_WARNING                     "אזהרה"
#define TR_STORAGE_WARNING             "איחסון"
#define TR_THROTTLE_UPPERCASE          "מצערת"
#define TR_ALARMSWARN                  "התראות"
#define TR_SWITCHWARN                  TR("SWITCH", "CONTROL")
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE                "Use for tests only"
#define TR_WRONG_SDCARDVERSION         TR("Expected ver: ", "Expected version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery low"
#define TR_WARN_MULTI_LOWPOWER         "מצב מתח נמוך"
#define TR_BATTERY                     "סוללה"
#define TR_WRONG_PCBREV                "Wrong PCB detected"
#define TR_EMERGENCY_MODE              "EMERGENCY MODE"
#define TR_NO_FAILSAFE                 "מצב חירום לא הוגדר"
#define TR_KEYSTUCK                    "Key stuck"
#define TR_VOLUME                      "עוצמה"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "בהירות"
#define TR_CPU_TEMP                    "CPU temp."
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "MB Temp."
#define TR_TTL_WARNING                 "Warning: Do not exceed 3.3V on TX/RX pins!"
#define TR_FUNC                        "Function"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Duration"
#define TR_DELAY                       "דיליי"
#define TR_SD_CARD                     "SD כרטיס"
#define TR_SDHC_CARD                   "SD תיקיות כרטיס"
#define TR_NO_SOUNDS_ON_SD             "SD אין צלילים על"
#define TR_NO_MODELS_ON_SD             "SD אין מודלים על"
#define TR_NO_BITMAPS_ON_SD            "No bitmaps on SD"
#define TR_NO_SCRIPTS_ON_SD            "No scripts on SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntax error", "Script syntax error")
#define TR_SCRIPT_PANIC                "Script panic"
#define TR_SCRIPT_KILLED               "Script killed"
#define TR_SCRIPT_ERROR                "Unknown error"
#define TR_PLAY_FILE                   "נגן"
#define TR_DELETE_FILE                 "מחק"
#define TR_COPY_FILE                   "העתק"
#define TR_RENAME_FILE                 "שינוי שם"
#define TR_ASSIGN_BITMAP               "Assign bitmap"
#define TR_ASSIGN_SPLASH               "Splash screen"
#define TR_EXECUTE_FILE                "Execute"
#define TR_REMOVED                     " הוסר"
#define TR_SD_INFO                     "מידע"
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "הגדרות חומרה"
#define TR_FORMATTING                  "...מפרמט"
#define TR_TEMP_CALIB                  "Temp. Calib"
#define TR_TIME                        "זמן"
#define TR_MAXBAUDRATE                 "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_CRSF_ARMING_MODE            "Arm using"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 "Sample Mode"
#define TR_SAMPLE_MODES                "Normal","OneBit"
#define TR_LOADING                     "...טוען"
#define TR_DELETE_THEME                "?למחוק את הערכה"
#define TR_SAVE_THEME                  "?לשמור את הערכה"
#define TR_EDIT_COLOR                  "עריכת צבע"
#define TR_NO_THEME_IMAGE              "תמונת ערכת נושא"
#define TR_BACKLIGHT_TIMER             "זמן אי פעילות"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "בחירת מודל מהירה"
  #define TR_LABELS_SELECT             "Label select"
  #define TR_LABELS_MATCH              "Label matching"
  #define TR_FAV_MATCH                 "Favorites matching"
  #define TR_LABELS_SELECT_MODE        "Multi select","Single select"
  #define TR_LABELS_MATCH_MODE         "Match all","Match any"
  #define TR_FAV_MATCH_MODE            "Must match","Optional match"
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "בחר ספרית תבנית"
#define TR_SELECT_TEMPLATE             "בחר מודל מתבנית:"
#define TR_NO_TEMPLATES                "אין מודל בספריית התבניות"
#define TR_SAVE_TEMPLATE               "שמור כשמירת בסיס"
#define TR_BLANK_MODEL                 "מודל ריק"
#define TR_BLANK_MODEL_INFO            "יצירת מודל ריק"
#define TR_FILE_EXISTS                 "FILE ALREADY EXISTS"
#define TR_ASK_OVERWRITE               "Do you want to overwrite?"

#define TR_BLUETOOTH                   "בלוטוס"
#define TR_BLUETOOTH_DISC              "גילוי"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Dist addr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Local addr"
#define TR_BLUETOOTH_PIN_CODE          "PIN code"
#define TR_BLUETOOTH_NODEVICES         "לא נמצאו מכשירים"
#define TR_BLUETOOTH_SCANNING          "...סורק"
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","אפשר"
#else
#define TR_BLUETOOTH_MODES             "---","טלמטריה","טריינר"
#endif

#define TR_SD_INFO_TITLE               "מידע כרטיס"
#define TR_SD_SPEED                    ":מהירות"
#define TR_SD_SECTORS                  ":סקטורים"
#define TR_SD_SIZE                     ":גודל"
#define TR_TYPE                        "סוג"
#define TR_GLOBAL_VARS                 "משתנים גלובלים"
#define TR_GVARS                       "GVARS"
#define TR_GLOBAL_VAR                  "משתנה גלובלי"
#define TR_MENU_GLOBAL_VARS            "משתנים גלובלים"
#define TR_OWN                         "בעלים"
#define TR_DATE                        "תאריך"
#define TR_MONTHS                      { "ינו", "פבו", "מרץ", "אפר", "מאי", "יונ", "יול", "אוג", "ספט", "אוק", "נוב", "דצמ" }
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR            "מצגי ערוצים"
#define TR_MIXERS_MONITOR              "מצגי מיקסים"
#define TR_PATH_TOO_LONG               "נתיב ארוך מדי"
#define TR_VIEW_TEXT                   "הצג טקסט"
#define TR_FLASH_BOOTLOADER            "Flash bootloader"
#define TR_FLASH_DEVICE                TR("Flash device","Flash device")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Flash S.Port", "Flash S.Port device")
#define TR_FLASH_RECEIVER_OTA          "Flash receiver OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash BT module", "Flash Bluetooth module")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Flash pwr mngt unit"
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "Device not responding")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Device file prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "Device data refused")
#define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "Device access problem")
#define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "Device file refused")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", "Device file sig.")
#define TR_CURRENT_VERSION             TR("Curr Vers: ", "Current version: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Flash int. module", "Flash internal module")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Int. Multi", "Flash Internal Multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash ext. module", "Flash external module")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Ext. Multi", "Flash External Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flash External ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("FW update error", "Firmware update error")
#define TR_FIRMWARE_UPDATE_SUCCESS     "! הפלאש עבר בהצלחה"
#define TR_WRITING                     "...ממתין"
#define TR_CONFIRM_FORMAT              "? מאשר פירמוט"
#define TR_INTERNALRF                  "מודול פנימי"
#define TR_INTERNAL_MODULE             TR("Int. module", "מודול פנימי")
#define TR_EXTERNAL_MODULE             TR("Ext. module", "מודול חיצוני")
#define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX upgrade required"
#define TR_TELEMETRY_DISABLED          "Telem. disabled"
#define TR_MORE_OPTIONS_AVAILABLE      "More options available"
#define TR_NO_MODULE_INFORMATION       "No module information"
#define TR_EXTERNALRF                  "מודול חיצוני"
#define TR_FAILSAFE                    TR("Failsafe", "הגדרת כשל קליטה")
#define TR_FAILSAFESET                 "הגדרת כשל קליטה"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Owner ID"
#define TR_HOLD                        "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                        "None"
#define TR_NONE_UPPERCASE              "NONE"
#define TR_MENUSENSOR                  "חיישן"
#define TR_POWERMETER_PEAK             "Peak"
#define TR_POWERMETER_POWER            "Power"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Freq."
#define TR_MENUTOOLS                   "כלים וסקריפטים"
#define TR_TURN_OFF_RECEIVER           "כיבוי מקלט"
#define TR_STOPPING                    "...עוצר"
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
#define TR_MENU_POWER_METER            "POWER METER"
#define TR_SENSOR                      "חיישן"
#define TR_COUNTRY_CODE                "Country code"
#define TR_USBMODE                     "USB מצב"
#define TR_JACK_MODE                   "Jack mode"
#define TR_VOICE_LANGUAGE              "שפת שמע"
#define TR_UNITS_SYSTEM                "יחידות"
#define TR_UNITS_PPM                   "PPM Units"
#define TR_EDIT                        "ערוך"
#define TR_INSERT_BEFORE               "כלול לפני"
#define TR_INSERT_AFTER                "כלול אחרי"
#define TR_COPY                        "העתקה"
#define TR_MOVE                        "הזז"
#define TR_PASTE                       "הדבקה"
#define TR_PASTE_AFTER                 "הדבק אחרי"
#define TR_PASTE_BEFORE                "הדבק לפני"
#define TR_DELETE                      "מחק"
#define TR_INSERT                      "Insert"
#define TR_RESET_FLIGHT                "איפוס טיסה"
#define TR_RESET_TIMER1                "איפוס שעון 1 "
#define TR_RESET_TIMER2                "איפוס שעון 2"
#define TR_RESET_TIMER3                "איפוס שעון 3"
#define TR_RESET_TELEMETRY             "איפוס טלמטריה"
#define TR_STATISTICS                  "סטטיסטיקות"
#define TR_ABOUT_US                    "מידע על"
#define TR_USB_JOYSTICK                "חיבור משחק (HID)"
#define TR_USB_MASS_STORAGE            "חיבור העברת נתונים (SD)"
#define TR_USB_SERIAL                  "חיבור סריילי (VCP)"
#define TR_SETUP_SCREENS               "הגדרות מסכים"
#define TR_MONITOR_SCREENS             "מצגים"
#define TR_AND_SWITCH                  "AND switch"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "בדיקת אנלוגיות סטיקים ומגע"
#define TR_ANADIAGS_FILTRAWDEV         "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW          "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX             "Min., max. and range"
#define TR_ANADIAGS_MOVE               "Move analogs to their extremes!"
#define TR_SPEAKER                     "רמקול"
#define TR_BUZZER                      "זמזם"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "צימוד"))
#define TR_MODULE_UNBIND               BUTTON("Unbind")
#define TR_POWERMETER_ATTN_NEEDED     "Attenuator needed"
#define TR_PXX2_SELECT_RX              "Select RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "בחר מכשיר"
#define TR_DISCOVER                    "Discover"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "...ממתין"
#define TR_RECEIVER_DELETE             "? מחיקת מקלט"
#define TR_RECEIVER_RESET              "? איפוס מקלט"
#define TR_SHARE                       "שיתוף"
#define TR_BIND                        "צימוד"
#define TR_REGISTER                    TR("Reg", "Register")
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "בדיקת טווח קליטה"))
#define TR_RANGE_TEST                  "בדיקת טווח"
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "RECEIVER OPTIONS")
#define TR_RESET_BTN                   BUTTON("איפוס")
#define TR_DEBUG                       "איבחון"
#define TR_KEYS_BTN                    BUTTON("בדיקת מתגים")
#define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "בדיקת אנלוגיות"))
#define TR_FS_BTN                      BUTTON(TR("Custom sw", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "!מסך זה אינו מסך מגע"
#define TR_TOUCH_EXIT                  "גע במסך כדי לצאת"
#define TR_SET                         BUTTON("הגדר")
#define TR_TRAINER                     "טריינר"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM              "TX antenna problem!"
#define TR_MODELIDUSED                 "מספר המקלט בשימוש במודל:"
#define TR_MODELIDUNIQUE               "מספר המקלט פנוי לשימוש"
#define TR_MODULE                      "מודל"
#define TR_RX_NAME                     "Rx Name"
#define TR_TELEMETRY_TYPE              TR("Type", "Telemetry type")
#define TR_TELEMETRY_SENSORS           "סנסורים"
#define TR_VALUE                       "ערך"
#define TR_PERIOD                      "Period"
#define TR_INTERVAL                    "Interval"
#define TR_REPEAT                      "מספר חזרות"
#define TR_ENABLE                      "זמין"
#define TR_DISABLE                     "אל תאפשר"
#define TR_TOPLCDTIMER                 "Top LCD Timer"
#define TR_UNIT                        "יחידה"
#define TR_TELEMETRY_NEWSENSOR         "הוסף חדש"
#define TR_CHANNELRANGE                TR("Ch. Range", "טווח ערוצים")
#define TR_ANTENNACONFIRM1             "EXT. ANTENNA"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "Internal","Ask","Per model","Internal + External"
#else
  #define TR_ANTENNA_MODES             "Internal","Ask","Per model","External"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Use int. antenna", "Use internal antenna")
#define TR_USE_EXTERNAL_ANTENNA        TR("Use ext. antenna", "Use external antenna")
#define TR_ANTENNACONFIRM2             TR("Check antenna", "Make sure antenna is installed!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Requires FLEX non"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Requires FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Requires EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "certified firmware"
#define TR_LOWALARM                    "התראה נמוכה"
#define TR_CRITICALALARM               "התראה קריטית"
#define TR_DISABLE_ALARM               TR("ביטול התראות", "ביטול התראות טלמטריה")
#define TR_POPUP                       "התראות"
#define TR_MIN                         "מינימום"
#define TR_MAX                         "מקסימום"
#define TR_CURVE_PRESET                "Preset..."
#define TR_PRESET                      "Preset"
#define TR_MIRROR                      "היפוך"
#define TR_CLEAR                       "ניקוי"
#define TR_RESET                       "איפוס"
#define TR_RESET_SUBMENU               "...איפוס"
#define TR_COUNT                       "ספירה"
#define TR_PT                          "נקודה"
#define TR_PTS                         "נקודות"
#define TR_SMOOTH                      "מעוגל"
#define TR_COPY_STICKS_TO_OFS          TR("Cpy stick->subtrim", "Copy sticks to subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
#define TR_COPY_TRIMS_TO_OFS           TR("Cpy trim->subtrim", "Copy trims to subtrim")
#define TR_INCDEC                      "Inc/Decrement"
#define TR_GLOBALVAR                   "Global var"
#define TR_MIXSOURCE                   "מקור (%)"
#define TR_MIXSOURCERAW                "מקור (ערך)"
#define TR_CONSTANT                    "Constant"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "כבוי","פועל","אוטומטי"
#define TR_PREFLIGHT                   "מצב הביקורת לשלט"
#define TR_CHECKLIST                   TR("Checklist", "הצג מצב מערכת")
#define TR_CHECKLIST_INTERACTIVE       TR3("C-Interact", "Interact. checklist", "Interactive checklist")
#define TR_AUX_SERIAL_MODE             "חיבור פנימי נוסף"
#define TR_AUX2_SERIAL_MODE            "חיבור פנימי נוסף 2"
#define TR_AUX_SERIAL_PORT_POWER       "הפעלת מתח בחיבור"
#define TR_SCRIPT                      "Script"
#define TR_INPUTS                      "כניסות"
#define TR_OUTPUTS                     "יציאות"
#define STR_EEBACKUP                   "EEPROM backup"
#define STR_FACTORYRESET               "איפוס לנתוני יצרן"
#define TR_CONFIRMRESET                TR("Erase ALL", "Erase ALL models and settings?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Too many Lua scripts!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "אוטומטי","פעיל"
#define TR_NO_TELEMETRY_SCREENS        "אין מסכי טלמטריה"
#define TR_TOUCH_PANEL                 "מסך מגע:"
#define TR_FILE_SIZE                   "גודל קובץ"
#define TR_FILE_OPEN                   "? לפתוח בכל זאת"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "שם"
#define TR_PHASES_HEADERS_SW           "מתג"
#define TR_PHASES_HEADERS_RUD_TRIM     "קיזוז הגה כיוון"
#define TR_PHASES_HEADERS_ELE_TRIM     "קיזוז הגה גובה"
#define TR_PHASES_HEADERS_THT_TRIM     "קיזוז מנוע"
#define TR_PHASES_HEADERS_AIL_TRIM     "קיזוז מאזנות"
#define TR_PHASES_HEADERS_CH5_TRIM     "T5 כפתור"
#define TR_PHASES_HEADERS_CH6_TRIM     "T6 כפתור"
#define TR_PHASES_HEADERS_FAD_IN       "Fade In"
#define TR_PHASES_HEADERS_FAD_OUT      "Fade Out"

#define TR_LIMITS_HEADERS_NAME         "שם"
#define TR_LIMITS_HEADERS_SUBTRIM      "מרכוז ראשי"
#define TR_LIMITS_HEADERS_MIN          "מינימום"
#define TR_LIMITS_HEADERS_MAX          "מקסימום"
#define TR_LIMITS_HEADERS_DIRECTION    "כיוון"
#define TR_LIMITS_HEADERS_CURVE        "עקומה"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM מרכז"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "מצב מירכוז"
#define TR_INVERTED                    "היפוך"


#define TR_LSW_DESCRIPTIONS            { "Comparison type or function", "First variable", "Second variable or constant", "Second variable or constant", "Additional condition for line to be enabled", "Minimum ON duration of the logical switch", "Minimum TRUE duration for the switch to become ON" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "ערוץ ראשון"
  #define TR_FILL_BACKGROUND           "מילוי רקע?"
  #define TR_BG_COLOR                  "BG צבעי"
  #define TR_SLIDERS_TRIMS             "Sliders+Trims"
  #define TR_SLIDERS                   "סליידרים"
  #define TR_FLIGHT_MODE               "מצב טיסה"
  #define TR_INVALID_FILE              "Invalid File"
  #define TR_TIMER_SOURCE              "מקור השעון"
  #define TR_SIZE                      "גודל"
  #define TR_SHADOW                    "הצללה"
  #define TR_ALIGNMENT                 "יישור"
  #define TR_ALIGN_LABEL               "Align label"
  #define TR_ALIGN_VALUE               "Align value"
  #define TR_ALIGN_OPTS                { "שמאל", "מרכז", "ימין" }
  #define TR_TEXT                      "טקסט"
  #define TR_COLOR                     "צבע"
  #define TR_MAIN_VIEW_X               "מסך תצוגה XX"
  #define TR_PANEL1_BACKGROUND         "Panel1 background"
  #define TR_PANEL2_BACKGROUND         "Panel2 background"
  #define TR_WIDGET_GAUGE              "מקור"
  #define TR_WIDGET_MODELBMP           "תמונה מודל"
  #define TR_WIDGET_OUTPUTS            "יציאות"
  #define TR_WIDGET_TEXT               "טקסט"
  #define TR_WIDGET_TIMER              "שעון"
  #define TR_WIDGET_VALUE              "ערך"
#endif

// Bootloader common - ASCII characters only
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
#define TR_BL_WRITING_FW              "Writing..."
#define TR_BL_WRITING_COMPL           "Writing complete"

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
   // Bootloader Taranis specific - ASCII characters only
  #define TR_BL_RESTORE_EEPROM        "Restore EEPROM"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Press the power button."
    #define TR_BL_FLASH_EXIT          "Exit the flashing mode."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - ASCII characters only
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_SELECT_KEY            "[ENT] to select file"
  #define TR_BL_FLASH_KEY             "Hold [ENT] long to flash"
  #define TR_BL_ERASE_KEY             "Hold [ENT] long to erase"
  #define TR_BL_EXIT_KEY              "[RTN] to exit"
#elif defined(PCBNV14)
   // Bootloader NV14 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] to select file"
  #define TR_BL_FLASH_KEY             "Hold [R TRIM] long to flash"
  #define TR_BL_EXIT_KEY              " [L TRIM] to exit"
  #define TR_BL_ENABLE                "Enable"
  #define TR_BL_DISABLE               "Disable"
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
#define TR_ABOUTUS                     TR(" ABOUT ", "ABOUT")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "עוצמת ציפצוף"
#define TR_WAV_VOLUME                  "עוצמת גל"
#define TR_BG_VOLUME                   TR("Bg volume", "עוצמת צליל רקע")

#define TR_TOP_BAR                     "בר עליון"
#define TR_FLASH_ERASE                 "Flash erase..."
#define TR_FLASH_WRITE                 "Flash write..."
#define TR_OTA_UPDATE                  "OTA update..."
#define TR_MODULE_RESET                "...איפוס מודל"
#define TR_UNKNOWN_RX                  "Unknown RX"
#define TR_UNSUPPORTED_RX              "Unsupported RX"
#define TR_OTA_UPDATE_ERROR            "OTA update error"
#define TR_DEVICE_RESET                "Device reset..."
#define TR_ALTITUDE                    "Altitude"
#define TR_SCALE                       "מד"
#define TR_VIEW_CHANNELS               "הצג ערוצים"
#define TR_VIEW_NOTES                  "הצג פתקים"
#define TR_MODEL_SELECT                "בחר מודל"
#define TR_ID                          "ID"
#define TR_PRECISION                   "קירוב עישריות"
#define TR_RATIO                       "יחס"
#define TR_FORMULA                     "נוסחה"
#define TR_CELLINDEX                   "מיקום תא"
#define TR_LOGS                        "לוגים"
#define TR_OPTIONS                     "אופציות"
#define TR_FIRMWARE_OPTIONS            "אופציןת קושחה"

#define TR_ALTSENSOR                   "Alt sensor"
#define TR_CELLSENSOR                  "Cell sensor"
#define TR_GPSSENSOR                   "GPS sensor"
#define TR_CURRENTSENSOR               "חיישן"
#define TR_AUTOOFFSET                  "Auto Offset"
#define TR_ONLYPOSITIVE                "Positive"
#define TR_FILTER                      "Filter"
#define TR_TELEMETRYFULL               TR("All slots full!", "All telemetry slots full!")
#define TR_IGNORE_INSTANCE             TR("No inst.", "Ignore instances")
#define TR_SHOW_INSTANCE_ID            "הצג מזהה"
#define TR_DISCOVER_SENSORS            "גלה הכל"
#define TR_STOP_DISCOVER_SENSORS       "עצור"
#define TR_DELETE_ALL_SENSORS          "מחק הכל"
#define TR_CONFIRMDELETE               "Really " LCDW_128_LINEBREAK "? למחוק הכל"
#define TR_SELECT_WIDGET               "בחירת וידג'ט"
#define TR_WIDGET_FULLSCREEN           "מסך מלא"
#define TR_REMOVE_WIDGET               "הסרת וידג'ט"
#define TR_WIDGET_SETTINGS             "הגדרות וידג'ט"
#define TR_REMOVE_SCREEN               "הסרת מסך"
#define TR_SETUP_WIDGETS               "הגדרות וידג'טים"
#define TR_USER_INTERFACE              "ממשק משתמש"
#define TR_THEME                       "ערכת נושא"
#define TR_SETUP                       "להגדיר"
#define TR_LAYOUT                      "תצוגת מסך"
#define TR_ADD_MAIN_VIEW               "הוסף מסך תצוגה נוסף"
#define TR_TEXT_COLOR                  "צבע טקסט"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "כניסות"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua סקריפטים"
#define TR_MENU_STICKS                 STR_CHAR_STICK "סטיקים"
#define TR_MENU_POTS                   STR_CHAR_POT "גלגלות"
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "מינימום"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "מקסימום"
#define TR_MENU_HELI                   STR_CHAR_CYC "Cyclic"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "קיזוזים"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "מתגים"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "מתגים לוגים"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "טריינר"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "עורצים"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "טלמטריה"
#define TR_MENU_DISPLAY                "DISPLAY"
#define TR_MENU_OTHER                  "Other"
#define TR_MENU_INVERT                 "Invert"
#define TR_AUDIO_MUTE                  TR("השתקת קול","השתק כאשר אין סאונד")
#define TR_JITTER_FILTER               "ADC filter"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Check RTC", "Check RTC voltage")
#define TR_AUTH_FAILURE                "Auth-failure"
#define TR_RACING_MODE                 "Racing mode"

#define TR_USE_THEME_COLOR              "Use theme color"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "מיכרוז כל הקיזוזים"
#if !PORTRAIT_LCD
  #define TR_OPEN_CHANNEL_MONITORS      "פתח מצג ערוצים"
#else
  #define TR_OPEN_CHANNEL_MONITORS      "פתח מצג ערוצים."
#endif
#define TR_DUPLICATE                    "שיכפול"
#define TR_ACTIVATE                     "הגדר פעיל"
#define TR_RED                          "אדום"
#define TR_BLUE                         "כחול"
#define TR_GREEN                        "ירוק"
#define TR_COLOR_PICKER                 "בחירת צבע"
#define TR_EDIT_THEME_DETAILS           "עריכת ערכת נושא"
#define TR_THEME_COLOR_DEFAULT          "DEFAULT"
#define TR_THEME_COLOR_PRIMARY1         "ראשי 1"
#define TR_THEME_COLOR_PRIMARY2         "ראשי 2"
#define TR_THEME_COLOR_PRIMARY3         "ראשי 3"
#define TR_THEME_COLOR_SECONDARY1       "משני 1"
#define TR_THEME_COLOR_SECONDARY2       "משני 2"
#define TR_THEME_COLOR_SECONDARY3       "משני 3"
#define TR_THEME_COLOR_FOCUS            "מיקוד"
#define TR_THEME_COLOR_EDIT             "עריכה"
#define TR_THEME_COLOR_ACTIVE           "פעיל"
#define TR_THEME_COLOR_WARNING          "אזהרה"
#define TR_THEME_COLOR_DISABLED         "לא פעיל"
#define TR_THEME_COLOR_CUSTOM           "מתקדם"
#define TR_THEME_CHECKBOX               "Checkbox"
#define TR_THEME_ACTIVE                 "פעיל"
#define TR_THEME_REGULAR                "רגיל"
#define TR_THEME_WARNING                "אזהרה"
#define TR_THEME_DISABLED               "לא פעיל"
#define TR_THEME_EDIT                   "עריכה"
#define TR_THEME_FOCUS                  "מיקוד"
#define TR_AUTHOR                       "Author"
#define TR_DESCRIPTION                  "תיאור"
#define TR_SAVE                         "שמור"
#define TR_CANCEL                       "ביטול"
#define TR_EDIT_THEME                   "עריכת ערכת נושא"
#define TR_DETAILS                      "פרטים"
#define TR_THEME_EDITOR                 "ערכות נושא"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "מודל\nבחירת"
#define TR_MAIN_MENU_MANAGE_MODELS      "ניהול\nמודלים"
#define TR_MAIN_MENU_MODEL_NOTES        "Model\nNotes"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "מצגי\nערוצים"
#define TR_MAIN_MENU_MODEL_SETTINGS     "הגדרות\nמודל"
#define TR_MAIN_MENU_RADIO_SETTINGS     "הגדרות\nשלט"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "הגדרות\nמסכים"
#define TR_MAIN_MENU_RESET_TELEMETRY    "איפוס\nשעונים"
#define TR_MAIN_MENU_STATISTICS         "סטטיסטיקות"
#define TR_MAIN_MENU_ABOUT_EDGETX       "מידע על\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "English"
#define TR_VOICE_CHINESE                "Chinese"
#define TR_VOICE_CZECH                  "Czech"
#define TR_VOICE_DANISH                 "Danish"
#define TR_VOICE_DEUTSCH                "German"
#define TR_VOICE_DUTCH                  "Dutch"
#define TR_VOICE_ESPANOL                "Spanish"
#define TR_VOICE_FRANCAIS               "French"
#define TR_VOICE_HEBREW                 "Hebrew"
#define TR_VOICE_HUNGARIAN              "Hungarian"
#define TR_VOICE_ITALIANO               "Italian"
#define TR_VOICE_POLISH                 "Polish"
#define TR_VOICE_PORTUGUES              "Portuguese"
#define TR_VOICE_RUSSIAN                "Russian"
#define TR_VOICE_SLOVAK                 "Slovak"
#define TR_VOICE_SWEDISH                "Swedish"
#define TR_VOICE_TAIWANESE              "Taiwanese"
#define TR_VOICE_JAPANESE               "Japanese"
#define TR_VOICE_UKRAINIAN               "Ukrainian"

#define TR_USBJOYSTICK_LABEL            "חיבור מצב משחק"
#define TR_USBJOYSTICK_EXTMODE          "מצב"
#define TR_VUSBJOYSTICK_EXTMODE         "רגיל","מתקדם"
#define TR_USBJOYSTICK_SETTINGS         "הגדרות ערוץ"
#define TR_USBJOYSTICK_IF_MODE          TR("If. mode","Interface mode")
#define TR_VUSBJOYSTICK_IF_MODE         "Joystick","Gamepad","MultiAxis"
#define TR_USBJOYSTICK_CH_MODE          "Mode"
#define TR_VUSBJOYSTICK_CH_MODE         "None","Btn","Axis","Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S       "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE       "Button Mode"
#define TR_VUSBJOYSTICK_CH_BTNMODE      "Normal","Pulse","SWEmu","Delta","Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S    TR("Norm","Normal"),TR("Puls","Pulse"),TR("SWEm","SWEmul"),TR("Delt","Delta"),TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS         "מיקומים"
#define TR_VUSBJOYSTICK_CH_SWPOS        "Push","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS          "צירים"
#define TR_VUSBJOYSTICK_CH_AXIS         "X","Y","Z","rotX","rotY","rotZ","Slider","Dial","Wheel"
#define TR_USBJOYSTICK_CH_SIM           "ציר סימולטור"
#define TR_VUSBJOYSTICK_CH_SIM          "מאזנות","ה.גובה","ה.כיוון","מנוע","Acc","Brk","Steer","Dpad"
#define TR_USBJOYSTICK_CH_INVERSION     "היפוך"
#define TR_USBJOYSTICK_CH_BTNNUM        "Button no."
#define TR_USBJOYSTICK_BTN_COLLISION    "!Button no. collision!"
#define TR_USBJOYSTICK_AXIS_COLLISION   "!Axis collision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT     TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT    "None","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES    "החל שינויים"

#define TR_DIGITAL_SERVO          "333HZ סרוו דיגיטאלי"
#define TR_ANALOG_SERVO           "50HZ סרוו אנלוגי"
#define TR_SIGNAL_OUTPUT          "יציאת סיגנל"
#define TR_SERIAL_BUS             "ערוץ טורי"
#define TR_SYNC                   "סינכרון"

#define TR_ENABLED_FEATURES       "אפשר יכולות"
#define TR_RADIO_MENU_TABS        "לשוניות תפריט רדיו"
#define TR_MODEL_MENU_TABS        "לשוניות תפריט מודל"

#define TR_SELECT_MENU_ALL        "All"
#define TR_SELECT_MENU_CLR        "Clear"
#define TR_SELECT_MENU_INV        "Invert"

#define TR_SORT_ORDERS            "A-Z סידור","Z-A סידור","יותר בשימוש","פחות בשימוש"
#define TR_SORT_MODELS_BY         "סדר תצוגה"
#define TR_CREATE_NEW             "יצירה חדשה"

#define TR_MIX_SLOW_PREC          TR("Slow prec", "Slow up/dn prec")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "כבר קיימת ערכת נושא עם אותו שם"

#define TR_DATE_TIME_WIDGET       "תאריך ושעה"
#define TR_RADIO_INFO_WIDGET      "מידע השלט"
#define TR_LOW_BATT_COLOR         "מתח סוללה נמוך"
#define TR_MID_BATT_COLOR         "מתח סוללה בינוני"
#define TR_HIGH_BATT_COLOR        "מתח סוללה גבוה"

#define TR_WIDGET_SIZE            "Widget size"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
