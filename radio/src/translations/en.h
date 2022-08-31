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

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */


#define TR_OFFON                       "OFF","ON"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Quiet","Alarm","NoKey","All"
#define TR_VBLMODE                     "OFF","Keys",TR("Ctrl","Controls"),"Both","ON"
#define TR_TRNMODE                     "OFF","+=",":="
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"

#define TR_AUX_SERIAL_MODES            "OFF","Telem Mirror","Telemetry In","SBUS Trainer","LUA","CLI","GPS","Debug"
#define TR_SWTYPES                     "None","Toggle","2POS","3POS"
#define TR_POTTYPES                    "None",TR("Pot w. det","Pot with detent"),TR("Multipos","Multipos Switch"),"Pot"
#define TR_SLIDERTYPES                 "None","Slider"
#define TR_VPERSISTENT                 "OFF","Flight","Manual Reset"
#define TR_COUNTRY_CODES               TR("US","America"),TR("JP","Japan"),TR("EU","Europe")
#define TR_USBMODES                    "Ask",TR("Joyst","Joystick"),TR("SDCard","Storage"),"Serial"
#define TR_JACK_MODES                  "Ask","Audio","Trainer"
#define TR_TELEMETRY_PROTOCOLS         "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_TELEMETRY_MODE        "Off","On","Off+Aux","On+Aux"
#define TR_MULTI_CUSTOM                "Custom"
#define TR_VTRIMINC                    TR("Expo","Exponential"),TR("ExFine","Extra Fine"),"Fine","Medium","Coarse"
#define TR_VDISPLAYTRIMS               "No","Change","Yes"
#define TR_VBEEPCOUNTDOWN              "Silent","Beeps","Voice","Haptic"
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VVARIOCENTER                "Tone","Silent"
#define TR_CURVE_TYPES                 "Standard","Custom"

#define TR_ADCFILTERVALUES             "Global","Off","On"

#if defined(PCBX10)
  #define TR_RETA123                   "R","E","T","A","1","3","2","4","5","6","7","L","R"
#elif defined(PCBHORUS)
  #define TR_RETA123                   "R","E","T","A","1","3","2","4","5","L","R"
#elif defined(PCBX9E)
  #define TR_RETA123                   "R","E","T","A","1","2","3","4","L","R","L","R"
#elif defined(PCBTARANIS)
  #define TR_RETA123                   "R","E","T","A","1","2","3","L","R"
#else
  #define TR_RETA123                   "R","E","T","A","1","2","3"
#endif

#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Add","Multiply","Replace"
#define TR_VMLTPX2                     "+=","*=",":="
#if defined(PCBHORUS)
  #define TR_VMIXTRIMS                 "OFF","ON","Rud","Ele","Thr","Ail","T5","T6"
#else
  #define TR_VMIXTRIMS                 "OFF","ON","Rud","Ele","Thr","Ail"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Timer"
  #define TR_CSWSTICKY                 "Stcky"
  #define TR_CSWRANGE                  "Range"
  #define TR_CSWSTAY                   "Edge"
#else
  #define TR_CSWTIMER                  "Tim"
  #define TR_CSWSTICKY                 "Stky"
  #define TR_CSWRANGE                  "Rnge"
  #define TR_CSWSTAY                   "Edge"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#if defined(VARIO)
  #define TR_VVARIO                    "Vario"
#else
  #define TR_VVARIO                    "[Vario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND                     "Play Sound"
#else
  #define TR_SOUND                     "Beep"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC                    "Haptic"
#else
  #define TR_HAPTIC                    "[Haptic]"
#endif

#if defined(VOICE)
  #define TR_PLAY_TRACK                "Play Track"
  #define TR_PLAY_BOTH                 "Play Both"
  #define TR_PLAY_VALUE                TR("Play Val","Play Value")
#else
  #define TR_PLAY_TRACK                "[Play Trk]"
  #define TR_PLAY_BOTH                 "[Play Bth]"
  #define TR_PLAY_VALUE                "[Play Val]"
#endif

#define TR_SF_BG_MUSIC                 "BgMusic","BgMusic ||"

#if defined(SDCARD)
  #define TR_SDCLOGS                   "SD Logs"
#else
  #define TR_SDCLOGS                   "[SD Logs]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR               "Adjust"
#else
  #define TR_ADJUST_GVAR               "[AdjustGV]"
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
  #define TR_SF_SAFETY                 "Override"
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "Overr."
#else
  #define TR_SF_SAFETY                 "---"
#endif

#define TR_SF_SCREENSHOT               "Screenshot"
#define TR_SF_RACING_MODE              "RacingMode"
#define TR_SF_DISABLE_TOUCH            "No Touch"
#define TR_SF_RESERVE                  "[reserve]"

#define TR_VFSWFUNC                    TR_SF_SAFETY,"Trainer","Inst. Trim","Reset","Set",TR_ADJUST_GVAR,"Volume","SetFailsafe","RangeCheck","ModuleBind",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH TR_SF_TEST

#define TR_FSW_RESET_TELEM             TR("Telm", "Telemetry")

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS          "Timer 1","Timer 2","Timer 3"
#else
  #define TR_FSW_RESET_TIMERS          "Tmr1","Tmr2","Tmr3"
#endif

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("All","Flight"),TR_FSW_RESET_TELEM

#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "Metric",TR("Imper.","Imperial")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "None","Nums","Bars","Script"
#define TR_GPSFORMAT                   "DMS","NMEA"

#define TR_TEMPLATE_CLEAR_MIXES        "Clear Mixes"
#define TR_TEMPLATE_SIMPLE_4CH         "Simple 4-CH"
#define TR_TEMPLATE_STICKY_TCUT        "Sticky-T-Cut"
#define TR_TEMPLATE_VTAIL              "V-Tail"
#define TR_TEMPLATE_DELTA              "Elevon\\Delta"
#define TR_TEMPLATE_ECCPM              "eCCPM"
#define TR_TEMPLATE_HELI               "Heli Setup"
#define TR_TEMPLATE_SERVO_TEST         "Servo Test"

#define TR_VSWASHTYPE                  "---","120","120X","140","90"

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
  #define TR_CYC_VSRCRAW               "CYC1","CYC2","CYC3"
#else
  #define TR_CYC_VSRCRAW               "[C1]","[C2]","[C3]"
#endif

#define TR_RESERVE_VSRCRAW             "[--]"
#define TR_EXTRA_VSRCRAW               "Batt","Time","GPS",TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,"Tmr1","Tmr2","Tmr3"

#define TR_VTMRMODES                   "OFF","ON","Strt","THs","TH%","THt"
#define TR_VTRAINER_MASTER_OFF         "Off"
#define TR_VTRAINER_MASTER_JACK        "Master/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Slave/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Master/SBUS Module"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Master/CPPM Module"
#define TR_VTRAINER_MASTER_BATTERY     "Master/Serial"
#define TR_VTRAINER_BLUETOOTH          "Master/" TR("BT","Bluetooth"), "Slave/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VFAILSAFE                   "Not set","Hold","Custom","No pulses","Receiver"
#define TR_VSENSORTYPES                "Custom","Calculated"
#define TR_VFORMULAS                   "Add","Average","Min","Max","Multiply","Totalize","Cell","Consumpt","Distance"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Lowest","1","2","3","4","5","6","Highest","Delta"
#define TR_TEXT_SIZE                   "Standard","Tiny","Small","Mid","Double"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center only)","= (symmetrical)"
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

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14)
  #define TR_ENTER                     "[NEXT]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#if defined(COLORLCD)
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

#define TR_FREE                        "free"
#define TR_YES                         "Yes"
#define TR_NO                          "No"
#define TR_DELETEMODEL                 "DELETE" BREAKSPACE "MODEL"
#define TR_COPYINGMODEL                "Copying model..."
#define TR_MOVINGMODEL                 "Moving model..."
#define TR_LOADINGMODEL                "Loading model..."
#define TR_NAME                        "Name"
#define TR_MODELNAME                   "Model name"
#define TR_PHASENAME                   "Mode name"
#define TR_MIXNAME                     "Mix name"
#define TR_INPUTNAME                   TR("Input", "Input name")
#define TR_EXPONAME                    TR("Name", "Line name")
#define TR_BITMAP                      "Model image"
#define TR_TIMER                       TR("Timer", "Timer ")
#define TR_START                       "Start"
#define TR_ELIMITS                     TR("E.Limits", "Extended limits")
#define TR_ETRIMS                      TR("E.Trims", "Extended trims")
#define TR_TRIMINC                     "Trim Step"
#define TR_DISPLAY_TRIMS               TR("Show Trims", "Display trims")
#define TR_TTRACE                      TR("T-Source", INDENT "Source")
#define TR_TTRIM                       TR("T-Trim-Idle", INDENT "Trim idle only")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", INDENT "Trim switch")
#define TR_BEEPCTR                     TR("Ctr Beep", "Center Beep")
#define TR_USE_GLOBAL_FUNCS            TR("Glob.Funcs", "Use global funcs")
#define TR_PROTOCOL                    TR("Proto", "Protocol")
  #define TR_PPMFRAME                  INDENT "PPM frame"
  #define TR_REFRESHRATE               TR(INDENT "Refresh", INDENT "Refresh rate")
  #define STR_WARN_BATTVOLTAGE         TR(INDENT "Output is VBAT: ", INDENT "Warning: output level is VBAT: ")
#define TR_WARN_5VOLTS                 "Warning: output level is 5 volts"
#define TR_MS                          "ms"
#define TR_FREQUENCY                   INDENT "Frequency"
#define TR_SWITCH                      "Switch"
#define TR_TRIMS                       "Trims"
#define TR_FADEIN                      "Fade in"
#define TR_FADEOUT                     "Fade out"
#define TR_DEFAULT                     "(default)"
#define TR_CHECKTRIMS                  CENTER "\006Check\012trims"
#define OFS_CHECKTRIMS                 CENTER_OFS+(9*FW)
#define TR_SWASHTYPE                   "Swash Type"
#define TR_COLLECTIVE                  TR("Collective", "Coll. pitch source")
#define TR_AILERON                     TR("Lateral cyc.", "Lateral cyc. source")
#define TR_ELEVATOR                    TR("Long. cyc.", "Long. cyc. source")
#define TR_SWASHRING                   "Swash Ring"
#define TR_ELEDIRECTION                "ELE Direction"
#define TR_AILDIRECTION                "AIL Direction"
#define TR_COLDIRECTION                "PIT Direction"
#define TR_MODE                        "Mode"
#define TR_SUBTYPE                     INDENT "Subtype"
#define TR_NOFREEEXPO                  "No free expo!"
#define TR_NOFREEMIXER                 "No free mixer!"
#define TR_SOURCE                       "Source"
#define TR_WEIGHT                      "Weight"
#define TR_EXPO                        TR("Expo", "Exponential")
#define TR_SIDE                        "Side"
#define TR_DIFFERENTIAL                "Differ"
#define TR_OFFSET                       "Offset"
#define TR_TRIM                        "Trim"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Curve"
#define TR_FLMODE                      TR("Mode", "Modes")
#define TR_MIXWARNING                  "Warning"
#define TR_OFF                         "OFF"
#define TR_ANTENNA                     "Antenna"
#define TR_NO_INFORMATION              TR("No info", "No information")
#define TR_MULTPX                      "Multiplex"
#define TR_DELAYDOWN                   TR("Delay dn", "Delay down")
#define TR_DELAYUP                     "Delay up"
#define TR_SLOWDOWN                    TR("Slow dn", "Slow down")
#define TR_SLOWUP                      "Slow up"
#define TR_MIXES                       "MIXES"
#define TR_CV                          "CV"
#if defined(PCBNV14)
#define TR_GV                          "GV"
#else
#define TR_GV                          TR("G", "GV")
#endif
#define TR_ACHANNEL                    "A\004channel"
#define TR_RANGE                       INDENT "Range"
#define TR_CENTER                      INDENT "Center"
#define TR_BAR                         "Bar"
#define TR_ALARM                       "Alarm"
#define TR_USRDATA                     TR("UsrData", "User data")
#define TR_BLADES                      "Blades/Poles"
#define TR_SCREEN                      "Screen\001"
#define TR_SOUND_LABEL                 "Sound"
#define TR_LENGTH                      "Length"
#define TR_BEEP_LENGTH                 "Beep length"
#define TR_BEEP_PITCH                  "Beep pitch"
#define TR_HAPTIC_LABEL                "Haptic"
#define TR_STRENGTH                    "Strength"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Offset"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Contrast"
#define TR_ALARMS_LABEL                "Alarms"
#define TR_BATTERY_RANGE               TR("Batt. range", "Battery meter range")
#define TR_BATTERYCHARGING             "Charging..."
#define TR_BATTERYFULL                 "Battery full"
#define TR_BATTERYNONE                 "None!"
#define TR_BATTERYWARNING              "Battery low"
#define TR_INACTIVITYALARM             "Inactivity"
#define TR_MEMORYWARNING               "Memory low"
#define TR_ALARMWARNING                "Sound off"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI shutdown", "Check RSSI on shutdown")
#define TR_MODEL_STILL_POWERED         "Model still powered"
#define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_MODEL_SHUTDOWN              "Shutdown?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Press enter to confirm"
#define TR_THROTTLE_LABEL              "Throttle"
#define TR_THROTTLEREVERSE             TR("T-Reverse", INDENT "Reverse")
#define TR_MINUTEBEEP                  TR("Minute", "Minute call")
#define TR_BEEPCOUNTDOWN               INDENT "Countdown"
#define TR_PERSISTENT                  TR(INDENT "Persist.", INDENT "Persistent")
#define TR_BACKLIGHT_LABEL             "Backlight"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLDELAY                     INDENT "Duration"
#define TR_BLONBRIGHTNESS              INDENT "ON brightness"
#define TR_BLOFFBRIGHTNESS             INDENT "OFF brightness"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR                     "Color"
#define TR_SPLASHSCREEN                "Splash screen"
#define TR_PWR_ON_DELAY                "Pwr On delay"
#define TR_PWR_OFF_DELAY               "Pwr Off delay"
#define TR_THROTTLE_WARNING            TR(INDENT "T-Warning", INDENT "Throttle state")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Cust-Pos", INDENT INDENT INDENT INDENT "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR(INDENT "S-Warning", INDENT "Switch positions")
#define TR_POTWARNINGSTATE             TR(INDENT "Pot&Slid.", INDENT "Pots & sliders")
#define TR_SLIDERWARNING               TR(INDENT "Slid. pos.", INDENT "Slider positions")
#define TR_POTWARNING                  TR(INDENT "Pot warn.", INDENT "Pot positions")
#define TR_TIMEZONE                    "Time zone"
#define TR_ADJUST_RTC                  "Adjust RTC"
#define TR_GPS                         "GPS"
#define TR_RXCHANNELORD                TR("Rx channel ord", "Default channel order")
#define TR_STICKS                      "Sticks"
#define TR_POTS                        "Pots"
#define TR_SWITCHES                    "Switches"
#define TR_SWITCHES_DELAY              TR("Play delay", "Play delay (sw. mid pos)")
#define TR_SLAVE                       CENTER "Slave"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "Multiplier"
#define TR_CAL                         "Cal"
#define TR_CALIBRATION                 "Calibration"
#define TR_VTRIM                       "Trim - +"
#define TR_BG                          "BG:"
#define TR_CALIB_DONE                  "Calibration completed"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Press [Enter] to start"
  #define TR_SETMIDPOINT               "Center sticks/pots/sliders and press [Enter]"
  #define TR_MOVESTICKSPOTS            "Move sticks/pots/sliders and press [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " TO START"
  #define TR_SETMIDPOINT               "CENTER STICKS/SLIDERS"
  #define TR_MOVESTICKSPOTS            "MOVE STICKS/POTS"
#else
  #define TR_MENUTOSTART               CENTER "\010" TR_ENTER " TO START"
  #define TR_SETMIDPOINT               TR(CENTER "\004SET STICKS MIDPOINT", CENTER "\004CENTER STICKS/SLIDERS")
  #define TR_MOVESTICKSPOTS            CENTER "\006MOVE STICKS/POTS"
  #define TR_MENUWHENDONE              CENTER "\006" TR_ENTER " WHEN DONE"
#endif
#define TR_RXBATT                      "Rx Batt:"
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_ACCEL                       "Acc:"
#define TR_NODATA                      CENTER "NO DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua scripts"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_TIMER_LABEL                 "Timer"
#define TR_THROTTLE_PERCENT_LABEL      "Throttle %"
#define TR_BATT_LABEL                  "Battery"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " to reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODEL"
#define TR_FM                          "FM"
#define TR_MIX                         "MIX"
#define TR_EEPROMLOWMEM                "EEPROM low mem"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Press any key to skip"
#define TR_THROTTLE_NOT_IDLE           "Throttle not idle"
#define TR_ALARMSDISABLED              "Alarms disabled"
#define TR_PRESSANYKEY                 TR("\010Press any Key", "Press any key")
#define TR_BADEEPROMDATA               "Bad EEprom data"
#define TR_BAD_RADIO_DATA              "Bad radio data"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            "Formatting EEPROM"
#define TR_STORAGE_FORMAT              "Storage preparation"
#define TR_EEPROMOVERFLOW              "EEPROM overflow"
#define TR_RADIO_SETUP                 "RADIO SETUP"
#define TR_MENUDATEANDTIME             "DATE AND TIME"
#define TR_MENUTRAINER                 "TRAINER"
#define TR_MENUSPECIALFUNCS            "GLOBAL FUNCTIONS"
#define TR_MENUVERSION                 "VERSION"
#define TR_MENU_RADIO_SWITCHES         TR("SWITCHES", "SWITCHES TEST")
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "ANALOGS TEST")
#define TR_MENU_RADIO_ANALOGS_CALIB    "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION             "CALIBRATION"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS             "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE           "Channels=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Channel=>Failsafe"
#define TR_MENUMODELSEL                TR("MODELSEL", "MODEL SELECTION")
#define TR_MENU_MODEL_SETUP            TR("SETUP", "MODEL SETUP")
#define TR_MENUFLIGHTMODES             "FLIGHT MODES"
#define TR_MENUFLIGHTMODE              "FLIGHT MODE"
#define TR_MENUHELISETUP               "HELI SETUP"

  #define TR_MENUINPUTS                "INPUTS"
  #define TR_MENULIMITS                "OUTPUTS"
#define TR_MENUCURVES                  "CURVES"
#define TR_MENUCURVE                   "CURVE"
#define TR_MENULOGICALSWITCH           "LOGICAL SWITCH"
#define TR_MENULOGICALSWITCHES         "LOGICAL SWITCHES"
#define TR_MENUCUSTOMFUNC              "SPECIAL FUNCTIONS"
#define TR_MENUCUSTOMSCRIPTS           "CUSTOM SCRIPTS"
#define TR_MENUTELEMETRY               "TELEMETRY"
#define TR_MENUTEMPLATES               "TEMPLATES"
#define TR_MENUSTAT                    "STATS"
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS1           "CHANNELS MONITOR 1-8"
#define TR_MONITOR_CHANNELS2           "CHANNELS MONITOR 9-16"
#define TR_MONITOR_CHANNELS3           "CHANNELS MONITOR 17-24"
#define TR_MONITOR_CHANNELS4           "CHANNELS MONITOR 25-32"
#define TR_MONITOR_SWITCHES            "LOGICAL SWITCHES MONITOR"
#define TR_MONITOR_OUTPUT_DESC         "Outputs"
#define TR_MONITOR_MIXER_DESC          "Mixers"
  #define TR_RECEIVER_NUM              TR("RxNum", "Receiver No.")
  #define TR_RECEIVER                  "Receiver"
#define TR_MULTI_RFTUNE                TR("Freq tune", "RF Freq. fine tune")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY             "Telemetry"
#define TR_MULTI_VIDFREQ               TR("Vid. freq.", "Video frequency")
#define TR_RF_POWER                    "RF Power"
#define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")
#define TR_MULTI_OPTION                TR("Option", "Option value")
#define TR_MULTI_AUTOBIND              TR(INDENT "Bind Ch.", INDENT "Bind on channel")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Disable Ch. map")
#define TR_DISABLE_TELEM               TR("No Telem", "Disable Telemetry")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "Autodetect", INDENT "Autodetect format")
#define TR_MULTI_LOWPOWER              TR(INDENT "Low power", INDENT "Low power mode")
#define TR_MULTI_LNA_DISABLE           INDENT "LNA disable"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")
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
#define TR_BIND_OK                     "Bind successful"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem OFF"
#define TR_PROTOCOL_INVALID            TR("Prot. invalid", "Protocol invalid")
#define TR_MODULE_STATUS               TR(INDENT "Status", INDENT "Module Status")
#define TR_MODULE_SYNC                 TR(INDENT "Sync", INDENT "Proto Sync Status")
#define TR_MULTI_SERVOFREQ             TR("Servo rate", "Servo update rate")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("RF Channel", "Select RF channel")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_SYNCMENU                    "[Sync]"
#define TR_LIMIT                       INDENT "Limit"
#define TR_MINRSSI                     "Min Rssi"
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_LATITUDE                    "Latitude"
#define TR_LONGITUDE                   "Longitude"
#define TR_GPS_COORDS_FORMAT           TR("GPS Coords", "Coordinate format")
#define TR_VARIO                       TR("Vario", "Variometer")
#define TR_PITCH_AT_ZERO               "Pitch zero"
#define TR_PITCH_AT_MAX                "Pitch max"
#define TR_REPEAT_AT_ZERO              "Repeat zero"
#define TR_SHUTDOWN                    "SHUTTING DOWN"
#define TR_SAVE_MODEL                  "Saving model settings"
#define TR_BATT_CALIB                  TR("Batt. calib", "Battery calibration")
#define TR_CURRENT_CALIB               "Current calib"
#define TR_VOLTAGE                     TR(INDENT "Voltage", INDENT "Voltage source")
#define TR_CURRENT                     TR(INDENT "Current", INDENT "Current source")
#define TR_SELECT_MODEL                "Select model"
#define TR_MODEL_CATEGORIES            "Model Categories"
#define TR_MODELS                      "Models"
#define TR_SELECT_MODE                 "Select mode"
#define TR_CREATE_CATEGORY             "Create category"
#define TR_RENAME_CATEGORY             "Rename category"
#define TR_DELETE_CATEGORY             "Delete category"
#define TR_CREATE_MODEL                "Create model"
#define TR_DUPLICATE_MODEL             "Duplicate model"
#define TR_COPY_MODEL                  "Copy model"
#define TR_MOVE_MODEL                  "Move model"
#define TR_BACKUP_MODEL                "Backup model"
#define TR_DELETE_MODEL                "Delete model"
#define TR_RESTORE_MODEL               "Restore model"
#define TR_DELETE_ERROR                "Delete error"
#define TR_CAT_NOT_EMPTY               "Category is not empty"
#define TR_SDCARD_ERROR                TR("SD error", "SD card error")
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "No files on SD!"
#define TR_NO_SDCARD                   "No SD card"
#define TR_WAITING_FOR_RX              "Waiting for RX..."
#define TR_WAITING_FOR_TX              "Waiting for TX..."
#define TR_WAITING_FOR_MODULE          TR("Waiting module", "Waiting for module...")
#define TR_NO_TOOLS                    "No tools available"
#define TR_NORMAL                      "Normal"
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
#define TR_SDCARD_FULL                 "SD card full"
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Incompatible"
#define TR_WARNING                     "WARNING"
#define TR_EEPROMWARN                  "EEPROM"
#define TR_STORAGE_WARNING             "STORAGE"
#define TR_EEPROM_CONVERTING           "Converting EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD card conversion required"
#define TR_CONVERTING                  "Converting: "
#define TR_THROTTLE_UPPERCASE          "THROTTLE"
#define TR_ALARMSWARN                  "ALARMS"
#define TR_SWITCHWARN                  TR("SWITCH", "CONTROL")
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE                "Use for tests only"
#define TR_WRONG_SDCARDVERSION         TR("Expected ver: ", "Expected version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery low"
#define TR_WARN_MULTI_LOWPOWER         "Low power mode"
#define TR_BATTERY                     "BATTERY"
#define TR_WRONG_PCBREV                "Wrong PCB detected"
#define TR_EMERGENCY_MODE              "EMERGENCY MODE"
#define TR_PCBREV_ERROR                "PCB error"
#define TR_NO_FAILSAFE                 "Failsafe not set"
#define TR_KEYSTUCK                    "Key stuck"
#define TR_INVERT_THR                  TR("Invert thr?", "Invert throttle?")
#define TR_VOLUME                      "Volume"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Brightness"
#define TR_CPU_TEMP                    "CPU temp."
#define TR_CPU_CURRENT                 "Current"
#define TR_CPU_MAH                     "Consumpt."
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "MB Temp."
#define TR_CAPAWARNING                 INDENT "Capacity low"
#define TR_TEMPWARNING                 INDENT "Overheat"
#define TR_TTL_WARNING                 "Warning: use 3.3V logic levels"
#define TR_FUNC                        "Func"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Duration"
#define TR_DELAY                       "Delay"
#define TR_SD_CARD                     "SD CARD"
#define TR_SDHC_CARD                   "SD-HC CARD"
#define TR_NO_SOUNDS_ON_SD             "No sounds" BREAKSPACE "on SD"
#define TR_NO_MODELS_ON_SD             "No models" BREAKSPACE "on SD"
#define TR_NO_BITMAPS_ON_SD            "No bitmaps" BREAKSPACE "on SD"
#define TR_NO_SCRIPTS_ON_SD            "No scripts" BREAKSPACE "on SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntax error", "Script syntax error")
#define TR_SCRIPT_PANIC                "Script panic"
#define TR_SCRIPT_KILLED               "Script killed"
#define TR_SCRIPT_ERROR                "Unknown error"
#define TR_PLAY_FILE                   "Play"
#define TR_DELETE_FILE                 "Delete"
#define TR_COPY_FILE                   "Copy"
#define TR_RENAME_FILE                 "Rename"
#define TR_ASSIGN_BITMAP               "Assign bitmap"
#define TR_ASSIGN_SPLASH               "Splash screen"
#define TR_EXECUTE_FILE                "Execute"
#define TR_REMOVED                     " removed"
#define TR_SD_INFO                     "Information"
#define TR_SD_FORMAT                   "Format"
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "HARDWARE"
#define TR_FORMATTING                  "Formatting..."
#define TR_TEMP_CALIB                  "Temp. Calib"
#define TR_TIME                        "Time"
#define TR_MAXBAUDRATE                 "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE                 "Sample Mode"
#define TR_SAMPLE_MODES                "Normal","OneBit"

#define TR_SELECT_TEMPLATE_FOLDER      "SELECT A TEMPLATE FOLDER:"
#define TR_SELECT_TEMPLATE             "SELECT A MODEL TEMPLATE:"
#define TR_NO_TEMPLATES                "No model templates were found in this folder"
#define TR_SAVE_TEMPLATE               "Save as template"
#define TR_BLANK_MODEL                 "Blank Model"
#define TR_BLANK_MODEL_INFO            "Create a blank model"
#define TR_FILE_EXISTS                 "FILE ALREADY EXISTS"
#define TR_ASK_OVERWRITE               "Do you want to overwrite?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Discover"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Dist addr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Local addr"
#define TR_BLUETOOTH_PIN_CODE          "PIN code"
#define TR_BLUETOOTH_NODEVICES         "No Devices Found"
#define TR_BLUETOOTH_SCANNING          "Scanning..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Enabled"
#else
#define TR_BLUETOOTH_MODES             "---","Telemetry","Trainer"
#endif

#define TR_SD_INFO_TITLE               "SD INFO"
#define TR_SD_TYPE                     "Type:"
#define TR_SD_SPEED                    "Speed:"
#define TR_SD_SECTORS                  "Sectors:"
#define TR_SD_SIZE                     "Size:"
#define TR_TYPE                        INDENT "Type"
#define TR_GLOBAL_VARS                 "Global variables"
#define TR_GVARS                       "GVARS"
#define TR_GLOBAL_VAR                  "Global variable"
#define TR_MENU_GLOBAL_VARS              "GLOBAL VARIABLES"
#define TR_OWN                         "Own"
#define TR_DATE                        "Date"
#define TR_MONTHS                      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR            "CHANNELS MONITOR"
#define TR_MIXERS_MONITOR              "MIXERS MONITOR"
#define TR_PATH_TOO_LONG               "Path too long"
#define TR_VIEW_TEXT                   "View text"
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
#define TR_FIRMWARE_UPDATE_SUCCESS     "Flash successful"
#define TR_WRITING                     "Writing..."
#define TR_CONFIRM_FORMAT              "Confirm Format?"
#define TR_INTERNALRF                  "Internal RF"
#define TR_INTERNAL_MODULE             TR("Int. module", "Internal module")
#define TR_EXTERNAL_MODULE             TR("Ext. module", "External module")
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX upgrade required"
#define TR_TELEMETRY_DISABLED          "Telem. disabled"
#define TR_MORE_OPTIONS_AVAILABLE      "More options available"
#define TR_NO_MODULE_INFORMATION       "No module information"
#define TR_EXTERNALRF                  "External RF"
#define TR_FAILSAFE                    TR(INDENT "Failsafe", INDENT "Failsafe mode")
#define TR_FAILSAFESET                 "FAILSAFE SETTINGS"
#define TR_REG_ID                      "Reg. ID"
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
#define TR_TURN_OFF_RECEIVER           "Turn off receiver"
#define TR_STOPPING                    "Stopping..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
#define TR_MENU_POWER_METER            "POWER METER"
#define TR_SENSOR                      "SENSOR"
#define TR_COUNTRY_CODE                "Country code"
#define TR_USBMODE                     "USB mode"
#define TR_JACK_MODE                   "Jack mode"
#define TR_VOICE_LANGUAGE              "Voice language"
#define TR_UNITS_SYSTEM                "Units"
#define TR_EDIT                        "Edit"
#define TR_INSERT_BEFORE               "Insert before"
#define TR_INSERT_AFTER                "Insert after"
#define TR_COPY                        "Copy"
#define TR_MOVE                        "Move"
#define TR_PASTE                       "Paste"
#define TR_PASTE_AFTER                 "Paste after"
#define TR_PASTE_BEFORE                "Paste before"
#define TR_DELETE                      "Delete"
#define TR_INSERT                      "Insert"
#define TR_RESET_FLIGHT                "Reset flight"
#define TR_RESET_TIMER1                "Reset timer1"
#define TR_RESET_TIMER2                "Reset timer2"
#define TR_RESET_TIMER3                "Reset timer3"
#define TR_RESET_TELEMETRY             "Reset telemetry"
#define TR_STATISTICS                  "Statistics"
#define TR_ABOUT_US                    "About"
#define TR_USB_JOYSTICK                "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB Storage (SD)"
#define TR_USB_SERIAL                  "USB Serial (VCP)"
#define TR_SETUP_SCREENS               "Setup screens"
#define TR_MONITOR_SCREENS             "Monitors"
#define TR_AND_SWITCH                  "AND switch"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Calibrated analogs"
#define TR_ANADIAGS_FILTRAWDEV         "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW          "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX             "Min., max. and range"
#define TR_ANADIAGS_MOVE               "Move analogs to their extremes!"
#define TR_SPEAKER                     INDENT "Speaker"
#define TR_BUZZER                      INDENT "Buzzer"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
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
#define TR_REGISTER                    TR("Reg", "Register")
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "Range"))
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "RECEIVER OPTIONS")
#define TR_DEL_BUTTON                  BUTTON(TR("Del", "Delete"))
#define TR_RESET_BTN                   BUTTON("Reset")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON("Keys")
#define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "Analogs"))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_SET                         BUTTON("Set")
#define TR_TRAINER                     "Trainer"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM              CENTER "TX antenna problem!"
#if defined(COLORLCD)
  #define TR_MODELIDUSED               "ID used in:"
#else
  #define TR_MODELIDUSED               TR("ID used in:","Receiver ID used in:")
#endif
#define TR_MODULE                      "Module"
#define TR_RX_NAME                     "Rx Name"
#define TR_TELEMETRY_TYPE              TR("Type", "Telemetry type")
#define TR_TELEMETRY_SENSORS           "Sensors"
#define TR_VALUE                       "Value"
#define TR_REPEAT                      "Repeat"
#define TR_ENABLE                      "Enable"
#define TR_TOPLCDTIMER                 "Top LCD Timer"
#define TR_UNIT                        "Unit"
#define TR_TELEMETRY_NEWSENSOR         INDENT "Add new"
#define TR_CHANNELRANGE                TR(INDENT "Ch. Range", INDENT "Channel Range")
#define TR_RXFREQUENCY                 TR("Rx Freqency", "Rx Servo Frequency")
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
#define TR_LOWALARM                    INDENT "Low alarm"
#define TR_CRITICALALARM               INDENT "Critical alarm"
#define TR_RSSIALARM_WARN              "RSSI"
#define TR_NO_RSSIALARM                TR(INDENT "Alarms disabled", "Telemetry alarms disabled")
#define TR_DISABLE_ALARM               TR(INDENT "Disable alarms", INDENT "Disable telemetry alarms")
#define TR_ENABLE_POPUP                "Enable popup"
#define TR_DISABLE_POPUP               "Disable popup"
#define TR_POPUP                       "Popup"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Preset..."
#define TR_PRESET                      "Preset"
#define TR_MIRROR                      "Mirror"
#define TR_CLEAR                       "Clear"
#define TR_RESET                       "Reset"
#define TR_RESET_SUBMENU               "Reset..."
#define TR_COUNT                       "Count"
#define TR_PT                          "pt"
#define TR_PTS                         "pts"
#define TR_SMOOTH                      "Smooth"
#define TR_COPY_STICKS_TO_OFS          TR("Cpy stick->subtrim", "Copy sticks to subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
#define TR_COPY_TRIMS_TO_OFS           TR("Cpy trim->subtrim", "Copy trims to subtrim")
#define TR_INCDEC                      "Inc/Decrement"
#define TR_GLOBALVAR                   "Global var"
#define TR_MIXSOURCE                   "Mixer source"
#define TR_CONSTANT                    "Constant"
#define TR_PERSISTENT_MAH              TR(INDENT "Str mAh", INDENT "Persistent mAh")
#define TR_PREFLIGHT                   "Preflight Checks"
#define TR_CHECKLIST                   TR(INDENT "Checklist", INDENT "Display checklist")
#define TR_FAS_OFFSET                  TR(INDENT "FAS Ofs", INDENT "FAS offset")
#define TR_AUX_SERIAL_MODE             "Serial port"
#define TR_AUX2_SERIAL_MODE            "Serial port 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port power"
#define TR_SCRIPT                      "Script"
#define TR_INPUTS                      "Inputs"
#define TR_OUTPUTS                     "Outputs"
#define STR_EEBACKUP                   "EEPROM backup"
#define STR_FACTORYRESET               "Factory reset"
#define TR_CONFIRMRESET                TR("Erase ALL", "Erase ALL models and settings?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Too many Lua scripts!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "No Telemetry Screens"
#define TR_TOUCH_PANEL                 "Touch panel:"
#define TR_FILE_SIZE                   "File size"
#define TR_FILE_OPEN                   "Open anyway?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Name"
#define TR_PHASES_HEADERS_SW           "Switch"
#define TR_PHASES_HEADERS_RUD_TRIM     "Rudder Trim"
#define TR_PHASES_HEADERS_ELE_TRIM     "Elevator Trim"
#define TR_PHASES_HEADERS_THT_TRIM     "Throttle Trim"
#define TR_PHASES_HEADERS_AIL_TRIM     "Aileron Trim"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Fade In"
#define TR_PHASES_HEADERS_FAD_OUT      "Fade Out"

#define TR_LIMITS_HEADERS_NAME         "Name"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Direction"
#define TR_LIMITS_HEADERS_CURVE        "Curve"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Center"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Subtrim mode"
#define TR_INVERTED                    "Inverted"

#define TR_LSW_HEADERS_FUNCTION        "Function"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "AND Switch"
#define TR_LSW_HEADERS_DURATION        "Duration"
#define TR_LSW_HEADERS_DELAY           "Delay"

#define TR_GVAR_HEADERS_NAME           "Name"
#define TR_GVAR_HEADERS_FM0            "Value on FM0"
#define TR_GVAR_HEADERS_FM1            "Value on FM1"
#define TR_GVAR_HEADERS_FM2            "Value on FM2"
#define TR_GVAR_HEADERS_FM3            "Value on FM3"
#define TR_GVAR_HEADERS_FM4            "Value on FM4"
#define TR_GVAR_HEADERS_FM5            "Value on FM5"
#define TR_GVAR_HEADERS_FM6            "Value on FM6"
#define TR_GVAR_HEADERS_FM7            "Value on FM7"
#define TR_GVAR_HEADERS_FM8            "Value on FM8"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS            { "Comparison type or function", "First variable", "Second variable or constant", "Second variable or constant", "Additional condition for line to be enabled", "Minimum ON duration of the logical switch", "Minimum TRUE duration for the switch to become ON" }

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
#define TR_ABOUTUS                     TR(" ABOUT ", "ABOUT")

#define TR_CHR_SHORT                   's'
#define TR_CHR_LONG                    'l'
#define TR_CHR_TOGGLE                  't'
#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Beep volume"
#define TR_WAV_VOLUME                  "Wav volume"
#define TR_BG_VOLUME                   TR("Bg volume", "Background volume")

#define TR_TOP_BAR                     "Top bar"
#define TR_FLASH_ERASE                 "Flash erase..."
#define TR_FLASH_WRITE                 "Flash write..."
#define TR_OTA_UPDATE                  "OTA update..."
#define TR_MODULE_RESET                "Module reset..."
#define TR_UNKNOWN_RX                  "Unknown RX"
#define TR_UNSUPPORTED_RX              "Unsupported RX"
#define TR_OTA_UPDATE_ERROR            "OTA update error"
#define TR_DEVICE_RESET                "Device reset..."
#define TR_ALTITUDE                    INDENT "Altitude"
#define TR_SCALE                       "Scale"
#define TR_VIEW_CHANNELS               "View Channels"
#define TR_VIEW_NOTES                  "View Notes"
#define TR_MODEL_SELECT                "Model Select"
#define TR_MODS_FORBIDDEN              "Modifications forbidden!"
#define TR_UNLOCKED                    "Unlocked"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Precision"
#define TR_RATIO                       "Ratio"
#define TR_FORMULA                     "Formula"
#define TR_CELLINDEX                   "Cell index"
#define TR_LOGS                        "Logs"
#define TR_OPTIONS                     "Options"
#define TR_FIRMWARE_OPTIONS            "Firmware options"

#define TR_ALTSENSOR                   "Alt sensor"
#define TR_CELLSENSOR                  "Cell sensor"
#define TR_GPSSENSOR                   "GPS sensor"
#define TR_CURRENTSENSOR               "Sensor"
#define TR_AUTOOFFSET                  "Auto Offset"
#define TR_ONLYPOSITIVE                "Positive"
#define TR_FILTER                      "Filter"
#define TR_TELEMETRYFULL               TR("All slots full!", "All telemetry slots full!")
#define TR_SERVOS_OK                   "Servos OK"
#define TR_SERVOS_KO                   "Servos KO"
#define TR_INVERTED_SERIAL             INDENT "Invert"
#define TR_IGNORE_INSTANCE             TR(INDENT "No inst.", INDENT "Ignore instances")
#define TR_DISCOVER_SENSORS            "Discover new"
#define TR_STOP_DISCOVER_SENSORS       "Stop"
#define TR_DELETE_ALL_SENSORS          "Delete all"
#define TR_CONFIRMDELETE               "Really " LCDW_128_480_LINEBREAK "delete all ?"
#define TR_SELECT_WIDGET               "Select widget"
#define TR_REMOVE_WIDGET               "Remove widget"
#define TR_WIDGET_SETTINGS             "Widget settings"
#define TR_REMOVE_SCREEN               "Remove screen"
#define TR_SETUP_WIDGETS               "Setup widgets"
#define TR_USER_INTERFACE              "User interface"
#define TR_THEME                       "Theme"
#define TR_SETUP                       "Setup"
#define TR_LAYOUT                      "Layout"
#define TR_ADD_MAIN_VIEW               "Add main view"
#define TR_BACKGROUND_COLOR            "Background color"
#define TR_MAIN_COLOR                  "Main color"
#define TR_BAR2_COLOR                  "Secondary bar color"
#define TR_BAR1_COLOR                  "Main bar color"
#define TR_TEXT_COLOR                  "Text color"
#define TR_TEXT_VIEWER                 "Text Viewer"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Inputs"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua scripts"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Sticks"
#define TR_MENU_POTS                   STR_CHAR_POT "Pots"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "Cyclic"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Switches"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Logical switches"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Trainer"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Channels"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Telemetry"
#define TR_MENU_DISPLAY                "DISPLAY"
#define TR_MENU_OTHER                  "Other"
#define TR_MENU_INVERT                 "Invert"
#define TR_JITTER_FILTER               "ADC filter"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Check RTC", "Check RTC voltage")
#define TR_AUTH_FAILURE                "Auth-failure"
#define TR_RACING_MODE                 "Racing mode"

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
