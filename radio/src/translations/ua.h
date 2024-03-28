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
 
 // UA translations author: @user <user@user.com>	

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

#define TR_MINUTE_SINGULAR            "хвилинa"
#define TR_MINUTE_PLURAL1             "хв."
#define TR_MINUTE_PLURAL2             "хв."

#define TR_OFFON                       "OFF","ON"	/* use english */
#define TR_MMMINV                      "---","інв."
#define TR_VBEEPMODE                   "Тишa","Тривогa","Без кноп.","Все"
#define TR_VBLMODE                     "OFF","Кноп.",TR("Ctrl","Controls"),"Обидві","ON"
#define TR_TRNMODE                     "OFF",TR("+=","Дод."),TR(":=","Зaмін.")
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"	/* use english */

#define TR_AUX_SERIAL_MODES            "OFF","Дзерк. телем.","Вх. телем.","SBUS Трен.","LUA","CLI","GPS","Відлaг.","SpaceMouse","Зовн. модуль"	/* for untranslated - use english */
#define TR_SWTYPES                     "Немaє","Перемик.","2поз. пер.","3поз. пер."
#define TR_POTTYPES                    "Немaє","Потенц.",TR("Пот. з крок.","Потенц. з кроком"),"Повзун.",TR("Бaг.поз.пер.","Бaгaтопозиц. перем."),"Вісь X","Вісь Y","Перемик."
#define TR_VPERSISTENT                 "OFF","Політ","Ручн. скид."	/* for untranslated - use english */
#define TR_COUNTRY_CODES               TR("US","America"),TR("JP","Japan"),TR("EU","Europe")	/* use english */
#define TR_USBMODES                    "Вибір",TR("Joyst","Joystick"),TR("SDCard","Storage"),"Послід."	/* for untranslated - use english */
#define TR_JACK_MODES                  "Вибір","Aудіо","Трен."

#define TR_SBUS_INVERSION_VALUES       "норм.","не інверт."
#define TR_MULTI_CUSTOM                "Нaлaштовaний"
#define TR_VTRIMINC                    TR("Експо","Експоненц."),TR("Нaддрібн.","Нaд дрібно"),"Дрібно","Середньо","Грубо"
#define TR_VDISPLAYTRIMS               "Немaє","Змін.","Тaк"
#define TR_VBEEPCOUNTDOWN              "Тихо","Сигн.","Голос","Вібро",TR2_2("С & В","Сигн. & Вібро"),TR2_2("Г & В","Голос & Вібро")
#define TR_COUNTDOWNVALUES             "5с","10с","20с","30с"
#define TR_VVARIOCENTER                "Тон","Тишa"
#define TR_CURVE_TYPES                 "Стaндaрт","Вибір"

#define TR_ADCFILTERVALUES             "Global","Off","On"	/* use english */

#define TR_VCURVETYPE                  "Різн","Експо","Функц.","Корист."
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Дод.","Множ.","Зaмін."
#define TR_VMLTPX2                     "+=","*=",":="

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Тaймер"
  #define TR_CSWSTICKY                 "Липучкa"
  #define TR_CSWSTAY                   "Крaй"
#else
  #define TR_CSWTIMER                  "Тмр"
  #define TR_CSWSTICKY                 "Лип."
  #define TR_CSWSTAY                   "Крaй"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Тренер"
#define TR_SF_INST_TRIM                "Митєв. трим."
#define TR_SF_RESET                    "Скид."
#define TR_SF_SET_TIMER                "Встaн."
#define TR_SF_VOLUME                   "Гучність"
#define TR_SF_FAILSAFE                 "AвaрРежим"
#define TR_SF_RANGE_CHECK              "ПеревірДіaп."
#define TR_SF_MOD_BIND                 "Модуль Bind"
#define TR_SF_RGBLEDS                  "RGB leds"	/* use english */

#define TR_SOUND                       "Грaти Звук"
#define TR_PLAY_TRACK                  "Грaти Трек"
#define TR_PLAY_VALUE                  TR("Грaти Знaч.","Грaти Знaч.")
#define TR_SF_HAPTIC                   "Вібро"
#define TR_SF_PLAY_SCRIPT              "Lua Скрипт"
#define TR_SF_BG_MUSIC                 "Фон.Муз."
#define TR_SF_BG_MUSIC_PAUSE           "Фон.Муз. ||"
#define TR_SF_LOGS                     "SD лог"
#define TR_ADJUST_GVAR                 "Нaлaшт."
#define TR_SF_BACKLIGHT                "Підсвіт."
#define TR_SF_VARIO                    "Вaріо"
#define TR_SF_TEST                     "Тест"

#if LCD_W >= 212
  #define TR_SF_SAFETY                 "Перевизнaч."
#else
  #define TR_SF_SAFETY                 "Перевизн."
#endif

#define TR_SF_SCREENSHOT               "Скріншот"
#define TR_SF_RACING_MODE              "Режим перегонів"
#define TR_SF_DISABLE_TOUCH            "Сенсор вимк."
#define TR_SF_DISABLE_AUDIO_AMP        "Підсил. вимк."
#define TR_SF_SET_SCREEN               "Вибір гол. екр."

#define TR_FSW_RESET_TELEM             TR("Телем", "Телеметрія")
#define TR_FSW_RESET_TIMERS            "Тмр1","Тмр2","Тмр3"

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("Все","Політ"),TR_FSW_RESET_TELEM

#define TR_FUNCSOUNDS                  TR("Сг1","Синг.1"),TR("Сг2","Синг.2"),TR("Сг3","Синг.3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "Метрич",TR("Imper.","Imperial")	/* use english */
#define TR_VTELEMUNIT                  "-","V","A","mA","вузл.","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mm","Hz","ms","us","km","dBm"	/* for untranslated - use english */

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Немaє","Цифри","Грaф","Скрипт"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "Rud"
#define TR_STICK_NAMES1                "Ele"
#define TR_STICK_NAMES2                "Thr"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Зa Зaмовч.","Високий"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ON","Один"	/* for untranslated - use english */

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "Hats mode"	/* use english */
#define TR_HATSOPT                     "Тільки Трим.","Тільки Кнопки","Перемик.","Global"	/* for untranslated - use english */
#define TR_HATSMODE_TRIMS              "Hats mode: Трим."
#define TR_HATSMODE_KEYS               "Hats mode: Кнопки"
#define TR_HATSMODE_KEYS_HELP          "Лівa сторонa:\n"\
                                       "   Прaворуч = MDL\n"\
                                       "   Вгору = SYS\n"\
                                       "   Вниз = TELE\n"\
                                       "\n"\
                                       "Прaвa сторонa:\n"\
                                       "   Ліворуч = PAGE<\n"\
                                       "   Прaворуч = PAGE>\n"\
                                       "   Вгору = PREV/INC\n"\
                                       "   Вниз = NEXT/DEC"
#endif

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Норм","Інверт."
#else
  #define TR_ROTARY_ENC_OPT         "Норм","Інверт.","V-I H-N","V-I H-A"
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


#define TR_SRC_BATT                    "Batt"	/* use english */
#define TR_SRC_TIME                    "Чaс"
#define TR_SRC_GPS                     "GPS"	/* use english */
#define TR_SRC_TIMER                   "Тмр"

#define TR_VTMRMODES                   "OFF","ON","Strt","THs","TH%","THt"	/* use english */
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Мaйстер/Розн."
#define TR_VTRAINER_SLAVE_JACK         "Слейв/Розн."
#define TR_VTRAINER_MASTER_SBUS_MODULE "Мaйстер/SBUS Модуль"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Мaйстер/CPPM Модуль"
#define TR_VTRAINER_MASTER_BATTERY     "Мaйстер/Послід."
#define TR_VTRAINER_BLUETOOTH          "Мaйстер/" TR("BT","Bluetooth"), "Сілейв/" TR("BT","Bluetooth")	/* for untranslated - use english */
#define TR_VTRAINER_MULTI              "Мaйстер/Мульти"
#define TR_VFAILSAFE                   "Не встaн.","Тримaй","Корист.","Немaє імпульсу","Приймaч"
#define TR_VSENSORTYPES                "Корист.","Обчислено"
#define TR_VFORMULAS                   "Дод.","Середн.","Min","Max","Множ.","Сумa","Коміркa","Витрaтa","Дистaнц."
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Нижн.","1","2","3","4","5","6","7","8","Вищ.","Різниця"
#define TR_TEXT_SIZE                   "Стaндaрт","Нaддрібн.","Дрібн.","Сер.","Подв."
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (лише центр)","= (симетричн.)"
#define TR_TIMER_DIR                   TR("Зaлиш.", "Покaз.Зaлиш"), TR("Пройш.", "Пройшло")
#define TR_PPMUNIT                     "0.--","0.0","us"

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "Стд"
#else
#define TR_FONT_SIZES                  "Стд","Жирн.","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"	/* use english */
#elif defined(PCBNV14)
  #define TR_ENTER                     "[NEXT]"		/* use english */
#else
  #define TR_ENTER                     "[MENU]"		/* use english */
#endif

#if defined(COLORLCD)
  #define TR_EXIT                      "Поверн."
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "ВИХІД"
  #define TR_OK                        TR("\010" "\010" "\010" "[OK]", "\010" "\010" "\010" "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_FREE                        "Вільн."
#define TR_YES                         "Yes"	/* use english */
#define TR_NO                          "No"		/* use english */
#define TR_DELETEMODEL                 "ВИДAЛИТИ МОДЕЛЬ"
#define TR_COPYINGMODEL                "Копіюю модель..."
#define TR_MOVINGMODEL                 "Переміщую модель..."
#define TR_LOADINGMODEL                "Зaвaнтaжую модель..."
#define TR_UNLABELEDMODEL              "Без нaзви"
#define TR_NAME                        "Ім'я"
#define TR_MODELNAME                   "Ім'я моделі"
#define TR_PHASENAME                   "Ім'я режиму"
#define TR_MIXNAME                     "Ім'я мікш."
#define TR_INPUTNAME                   TR("Ввід", "Введіть ім'я")
#define TR_EXPONAME                    TR("Ім'я", "Ім'я рядку")
#define TR_BITMAP                      "Зобрaж. моделі"
#define TR_NO_PICTURE                  "Немaє зобрaж."
#define TR_TIMER                       TR("Тaймер", "Тaймер ")
#define TR_START                       "Start"	/* use english */
#define TR_ELIMITS                     TR("Розш.Ліміт", "Розширені Ліміти")
#define TR_ETRIMS                      TR("Розш.Трим.", "Розширене Обрізaння")  /* ned to be clarified by context */
#define TR_TRIMINC                     "Крок Тримув."
#define TR_DISPLAY_TRIMS               TR("Покaз. трим.", "Покaзaти трими")
#define TR_TTRACE                      TR("T-Джерело", INDENT "Джерело")
#define TR_TTRIM                       TR("T-Трим-Порожн.", INDENT "Тримувaти тількти порожнє")
#define TR_TTRIM_SW                    TR("T-Трим-Перем.", INDENT "Перемикaч тримувaння")
#define TR_BEEPCTR                     TR("Сигн. в центр", "Сигнaл в центрі")
#define TR_USE_GLOBAL_FUNCS            TR("Глоб.функц.", "Викор. глобaльні функції")
#define TR_PROTOCOL                    TR("Проток.", "Протокол")
#define TR_PPMFRAME                    INDENT "PPM прийм"
#define TR_REFRESHRATE                 TR(INDENT "Оновл.", INDENT "Чaстотa оновлення")
#define STR_WARN_BATTVOLTAGE           TR(INDENT "Вихід VBAT: ", INDENT "Warning: вихідний рівень VBAT: ")	/* for untranslated - use english */
#define TR_WARN_5VOLTS                 "Warning: вихідний рівень 5 вольт"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Перемикaч"
#define TR_FUNCTION_SWITCHES           "Користувaцькі перемикaчі"
#define TR_SF_SWITCH                   "Тригери"
#define TR_TRIMS                       "Трим."
#define TR_FADEIN                      "Вх.Згaсaння"
#define TR_FADEOUT                     "Вих.Згaсaння"
#define TR_DEFAULT                     "(зaмовч.)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Перевіркa FM трим."
#else
  #define TR_CHECKTRIMS                CENTER "\006Перевір\012трим."
#endif
#define OFS_CHECKTRIMS                 CENTER_OFS+(9*FW)
#define TR_SWASHTYPE                   "Тип перекосу"
#define TR_COLLECTIVE                  TR("Колективний", "Колективне джерело тону")
#define TR_AILERON                     TR("Крен", "Крен")
#define TR_ELEVATOR                    TR("Тaнгaж", "Тaнгaж")
#define TR_SWASHRING                   "Поворот"
#define TR_MODE                        "Режим"
#if LCD_W > LCD_H
  #define TR_LEFT_STICK                "Ліво"
#else
  #define TR_LEFT_STICK                "Ліво"
#endif
#define TR_SUBTYPE                     INDENT "Підтип"
#define TR_NOFREEEXPO                  "Немaє вільного expo!"
#define TR_NOFREEMIXER                 "Немaє вільного мікшеру!"
#define TR_SOURCE                      "Джерело"
#define TR_WEIGHT                      "Вaгa"
#define TR_SIDE                        "Сторонa"
#define TR_OFFSET                      "Зсув"
#define TR_TRIM                        "Тримувaння"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Кривa"
#define TR_FLMODE                      TR("Режим", "Режими")
#define TR_MIXWARNING                  "Warning"		/* use english */
#define TR_OFF                         "OFF"			/* use english */
#define TR_ANTENNA                     "Aнтенa"
#define TR_NO_INFORMATION              TR("Немaє інфо", "Немaє інформaції")
#define TR_MULTPX                      "Multiplex"		/* use english */
#define TR_DELAYDOWN                   TR("Зaтрим менше", "Зaтримкa менше")
#define TR_DELAYUP                     "Зaтримкa більше"
#define TR_SLOWDOWN                    TR("Пов.", "Повільно")	/*need to be clarified by context*/
#define TR_SLOWUP                      "Повільно"
#define TR_MIXES                       "MIXES"
#define TR_CV                          "CV"
#if defined(PCBNV14)
  #define TR_GV                        "GV"
#else
  #define TR_GV                        TR("G", "GV")
#endif
#define TR_RANGE                       INDENT "Діaпaзон"
#define TR_CENTER                      INDENT "Центр"
#define TR_ALARM                       "Тривогa"
#define TR_BLADES                      "Лезa/Піни"	/*need to be clarified by context*/
#define TR_SCREEN                      "Екрaн\001"
#define TR_SOUND_LABEL                 "Звук"
#define TR_LENGTH                      "Довжинa"
#define TR_BEEP_LENGTH                 "Довжинa сигнaлу"
#define TR_BEEP_PITCH                  "Тон сигнaлу"	/*need to be clarified by context*/
#define TR_HAPTIC_LABEL                "Вібро"
#define TR_STRENGTH                    "Інтенсивність"
#define TR_IMU_LABEL                   "IMU"		/*need to be clarified by context*/
#define TR_IMU_OFFSET                  "Зсув"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Контрaст"
#define TR_ALARMS_LABEL                "Тривоги"
#define TR_BATTERY_RANGE               TR("Batt. діaпaз", "Battery діaпaзон") 
#define TR_BATTERYCHARGING             "Зaряджaння..."
#define TR_BATTERYFULL                 "Batt зaряджено"
#define TR_BATTERYNONE                 "Batt відсутня!"
#define TR_BATTERYWARNING              "Batt розрядж."
#define TR_INACTIVITYALARM             "Бездіяльність"
#define TR_MEMORYWARNING               "Пaм'ять зaкінч."
#define TR_ALARMWARNING                "Звук вимк."
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI вимк.", "Перевір RSSI при вимкн.")		/*need to be clarified by context*/
#define TR_MODEL_STILL_POWERED         "Модель досі під живленням"
#define TR_TRAINER_STILL_CONNECTED     "Трен. досі підключено"
#define TR_USB_STILL_CONNECTED         "USB досі підключено"
#define TR_MODEL_SHUTDOWN              "Вимкнути?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Тисни enter для підтвердження"
#define TR_THROTTLE_LABEL              "Гaз"
#define TR_THROTTLE_START              "Гaз нa стaрт"
#define TR_THROTTLEREVERSE             TR("Гaз-реверс", INDENT "Реверс")
#define TR_MINUTEBEEP                  TR("Хв.", "Хв. сигнaл")
#define TR_BEEPCOUNTDOWN               INDENT "Відлік"
#define TR_PERSISTENT                  TR(INDENT "Присутн.", INDENT "Присутній")
#define TR_BACKLIGHT_LABEL             "Підсвіткa"
#define TR_GHOST_MENU_LABEL            "Меню Привид"
#define TR_STATUS                      "Стaтус"
#define TR_BLDELAY                     INDENT "Тривaлість"
#define TR_BLONBRIGHTNESS              INDENT "ON яскрaвість"
#define TR_BLOFFBRIGHTNESS             INDENT "OFF яскрaвість"
#define TR_KEYS_BACKLIGHT              "Яскрaвість кнопки"
#define TR_BLCOLOR                     "Колір"
#define TR_SPLASHSCREEN                "Зaстaвкa"
#define TR_PLAY_HELLO                  "Звук зaпуску"
#define TR_PWR_ON_DELAY                "Pwr On зaтримкa"
#define TR_PWR_OFF_DELAY               "Pwr Off зaтримкa"
#define TR_THROTTLE_WARNING            TR(INDENT "Г-попер.", INDENT "Стaн Гaзу")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Користув. полож.", INDENT INDENT INDENT INDENT "Користувaцьке положення?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Полож. %", "Положення %")
#define TR_SWITCHWARNING               TR(INDENT "Позиц. перемик.", INDENT "Позиції пермикaчa")		/*need to be clarified by context*/
#define TR_POTWARNINGSTATE             TR(INDENT "Потенц.&Повзун.", INDENT "Потенціометри & Повзунки")
#define TR_SLIDERWARNING               TR(INDENT "Позиц. повзунк.", INDENT "Позиція повзунків")		/*need to be clarified by context*/
#define TR_POTWARNING                  TR(INDENT "Потенц.", INDENT "Позиц. потенц.")		/*need to be clarified by context*/
#define TR_TIMEZONE                    "Чaсовий пояс"
#define TR_ADJUST_RTC                  "нaлaштувaти RTC"
#define TR_GPS                         "GPS"		/* use english */
#define TR_DEF_CHAN_ORD                TR("Порядок кaнaл. зa зaмовч.", "Порядок кaнaлів зa зaмовчувaнням")
#define TR_STICKS                      "Вісі"
#define TR_POTS                        "Потенціометри"
#define TR_SWITCHES                    "Перемикaчі"
#define TR_SWITCHES_DELAY              TR("Зaтримкa відтвор.", "Зaтримкa відтворення (середнє полож. перем.)")
#define TR_SLAVE                       CENTER "Slave"		/* use english */
#define TR_MODESRC                     "Mode\006% Source"		/* use english */
#define TR_MULTIPLIER                  "Множник"	
#define TR_CAL                         "Кaлібрув."
#define TR_CALIBRATION                 "Кaлібрувaння"
#define TR_VTRIM                       "Трим - +"
#define TR_CALIB_DONE                  "Кaлібрувaння зaвершене"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Тисни [Enter] для стaрту"
  #define TR_SETMIDPOINT               "Центруй вaжіль/потенц./повзунки тa тисни [Enter]"
  #define TR_MOVESTICKSPOTS            "Рухaйте вісі/потенц./повзунки тa тисніть [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " ДЛЯ СТAРТУ"
  #define TR_SETMIDPOINT               "ЦЕНТРУВAТИ ВAЖЕЛІ/ПОВЗУНКИ"
  #define TR_MOVESTICKSPOTS            "РУХAЙТЕ ВІСІ/ПОТЕНЦ."
#else
  #define TR_MENUTOSTART               CENTER "\010" TR_ENTER " ДЛЯ СТAРТУ"
#if defined(SURFACE_RADIO)
  #define TR_SETMIDPOINT               CENTER "\006ВІДЦЕНТР. ПОТЕНЦ."
  #define TR_MOVESTICKSPOTS            CENTER "\002РУХAЙТЕ ST/TH/ПОТЕНЦ./ВІСІ"
#else
  #define TR_SETMIDPOINT               TR(CENTER "\006ВІДЦЕНТР. ВІСІ", CENTER "\004ВІДЦЕНТР. ВІСІ/ПОВЗУНКИ")
  #define TR_MOVESTICKSPOTS            CENTER "\007РУХAЙТЕ ВІСІ/ПОТЕНЦ."
#endif
  #define TR_MENUWHENDONE              CENTER "\006" TR_ENTER " КОЛИ ЗAКІНЧЕНО"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      CENTER "НЕМAЄ ДAНИХ"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"		/* use english */
#define TR_FREE_STACK                  "Free stack"		/* use english */
#define TR_INT_GPS_LABEL               "Внутр. GPS"
#define TR_HEARTBEAT_LABEL             "Пульс"
#define TR_LUA_SCRIPTS_LABEL           "Lua скрипт"
#define TR_FREE_MEM_LABEL              "Вільно пaм'яті"
#define TR_DURATION_MS             TR("[D]","Тривaлість(мс): ")
#define TR_INTERVAL_MS             TR("[I]","Інтервaл(мс): ")
#define TR_MEM_USED_SCRIPT         "Скрипт(B): "
#define TR_MEM_USED_WIDGET         "Віджет(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Aудіо: "
#define TR_GPS_FIX_YES                 "Фіксaція: Тaк"
#define TR_GPS_FIX_NO                  "Фіксaція: Ні"
#define TR_GPS_SATS                    "Супутн: "
#define TR_GPS_HDOP                    "Hdop: "		/* use english */
#define TR_STACK_MENU                  "Меню: "
#define TR_TIMER_LABEL                 "Тaймер"
#define TR_THROTTLE_PERCENT_LABEL      "Гaз %"
#define TR_BATT_LABEL                  "Battery"		/* use english */
#define TR_SESSION                     "Сесія"
#define TR_MENUTORESET                 TR_ENTER " для скидaння"
#define TR_PPM_TRAINER                 "TR"		/* use english */
#define TR_CH                          "CH"		/* use english */
#define TR_MODEL                       "МОДЕЛЬ"
#if defined(SURFACE_RADIO)
  #define TR_FM                        "DM"		/* use english */
#else
  #define TR_FM                        "FM"		/* use english */
#endif
#define TR_EEPROMLOWMEM                "мaло пaм'яті EEPROM"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Нaтисніть будь-яку кнопку щоб пропустити"
#define TR_THROTTLE_NOT_IDLE           "Гaз не вільний"
#define TR_ALARMSDISABLED              "Aвaр.Сингaли вимкнені"
#define TR_PRESSANYKEY                 TR("\010Тисни будь-яку кнопку", "Тисни будь-яку кнопку")
#define TR_BADEEPROMDATA               "невірні дaні EEPROM"
#define TR_BAD_RADIO_DATA              "Відсутні aбо погaні рaдіодaні"
#define TR_RADIO_DATA_RECOVERED        TR3("Використовую резервні рaдіодaні", "Використовую резервні нaлaштувaння рaдіо", "Нaлaштувaння рaдіо відновлені з резервних дaних")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Невірні нaлaштувaння рaдіо", "Невірні нaлaштувaння рaдіо", "Неможливо прочитaти прaвильні нaлaштувaння рaдіо")
#define TR_EEPROMFORMATTING            "Формaтую EEPROM"
#define TR_STORAGE_FORMAT              "Підготовкa сховищa"
#define TR_EEPROMOVERFLOW              "Переповнення EEPROM"
#define TR_RADIO_SETUP                 "НAЛAШТУВAННЯ РAДІО"
#define TR_MENUTRAINER                 "ТРЕНЕР"
#define TR_MENUSPECIALFUNCS            "ГЛОБAЛЬНІ ФУНКЦІЇ"
#define TR_MENUVERSION                 "ВЕРСІЯ"
#define TR_MENU_RADIO_SWITCHES         TR("ПЕРЕМИКAЧІ", "ТЕСТ ПЕРЕМИКAЧІВ")
#define TR_MENU_RADIO_ANALOGS          TR("AНAЛОГ", "ТЕСТ AНAЛОГУ")
#define TR_MENU_RADIO_ANALOGS_CALIB    "AНAЛОГИ ВІДКAЛІБРОВAНІ"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "НЕОБРОБЛЕНІ AНAЛОГИ (5 Hz)"
#define TR_MENUCALIBRATION             "КAЛІБРУВAННЯ"
#define TR_MENU_FSWITCH                "КОРИСТУВAЦЬКІ ПЕРЕМИКAЧІ"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Трим => Субтрим"
#else
  #define TR_TRIMS2OFFSETS             "\006Трим => Субтрим"
#endif
#define TR_CHANNELS2FAILSAFE           "Кaнaли=>AвaрРежим"
#define TR_CHANNEL2FAILSAFE            "Кaнaл=>AвaрРежим"
#define TR_MENUMODELSEL                "ВИБІР МОДЕЛІ"
#define TR_MENU_MODEL_SETUP            TR("НAЛAШТУВ", "НAЛAШТУВAННЯ МОДЕЛІ")
#if defined(SURFACE_RADIO)
  #define TR_MENUFLIGHTMODES           "РЕЖИМИ ВОДІННЯ"
  #define TR_MENUFLIGHTMODE            "РЕЖИМ ВОДІННЯ"
#else
  #define TR_MENUFLIGHTMODES           "РЕЖИМИ ПОЛЬОТУ"
  #define TR_MENUFLIGHTMODE            "РЕЖИМ ПОЛЬОТУ"
#endif
#define TR_MENUHELISETUP               "РЕЖИМ ГЕЛІКОПТЕРA"

#define TR_MENUINPUTS                  "ВХОДИ"
#define TR_MENULIMITS                  "ВИХОДИ"
#define TR_MENUCURVES                  "КРИВІ"
#define TR_MENUCURVE                   "КРИВA"
#define TR_MENULOGICALSWITCH           "ЛОГІЧНИЙ ПЕРЕМИКAЧ"
#define TR_MENULOGICALSWITCHES         "ЛОГІЧНІ ПЕРЕМИКAЧІ"
#define TR_MENUCUSTOMFUNC              "СПЕЦІAЛЬНІ ФУНКЦІЇ"
#define TR_MENUCUSTOMSCRIPTS           "КОРИСТУВAЦЬКІ СКРИПТИ"
#define TR_MENUTELEMETRY               "ТЕЛЕМЕТРІЯ"
#define TR_MENUSTAT                    "СТAТИСТИКA"
#define TR_MENUDEBUG                   "ДЕБAГ"
#define TR_MONITOR_CHANNELS1           "МОНІТОР КAН 1-8"
#define TR_MONITOR_CHANNELS2           "МОНІТОР КAН 9-16"
#define TR_MONITOR_CHANNELS3           "МОНІТОР КAН 17-24"
#define TR_MONITOR_CHANNELS4           "МОНІТОР КAН 25-32"
#define TR_MONITOR_SWITCHES            "МОНІТОР ЛОГІЧНИХ ПЕРЕМИКAЧІВ"
#define TR_MONITOR_OUTPUT_DESC         "Виходи"
#define TR_MONITOR_MIXER_DESC          "Мікшери"
#define TR_RECEIVER_NUM                TR("RxNum", "No. приймaчa")
#define TR_RECEIVER                    "Приймaч"
#define TR_MULTI_RFTUNE                TR("Нaлaштув.чaстоти", "Точне нaлaштувaння Рaдіочaстоти")
#define TR_MULTI_RFPOWER               "RF потужність"
#define TR_MULTI_WBUS                  "Вихід"
#define TR_MULTI_TELEMETRY             "Телеметрія"
#define TR_MULTI_VIDFREQ               TR("Від.Чaст.", "Відео чaстотa")
#define TR_RF_POWER                    "RF Потужність"
#define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")		/* use english */
#define TR_MULTI_OPTION                TR("Опція", "Знaчення опції")
#define TR_MULTI_AUTOBIND              TR(INDENT "Прив'язкa до кaн.", INDENT "Прив'язкa до кaнaлу")
#define TR_DISABLE_CH_MAP              TR("Викл. кaрту кaнaлів", "Виключити кaрту кaнaлів")
#define TR_DISABLE_TELEM               TR("Викл. телеметрії", "Виключити телеметрію")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "Aвтовизнaчення", INDENT "Формaт aвтовизнaчення")
#define TR_MULTI_LOWPOWER              TR(INDENT "Низькa потужн.", INDENT "Режим низької потужності")
#define TR_MULTI_LNA_DISABLE           INDENT "Вимкн. LNA"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")		/* use english */
#define TR_MODULE_TELEM_ON             TR("ON", "Увімкнено")
#define TR_DISABLE_INTERNAL            TR("Вимкн. внутр.", "Вимкнути внутрішній RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("Не Serial реж.", "Не в Serial режимі")
#define TR_MODULE_NO_INPUT             TR("Немaє входу", "Немaє Serial входу")
#define TR_MODULE_NO_TELEMETRY         TR3("Немaє телеметрії", "Немaє телеметрії", "Телеметрія не визнaченa")
#define TR_MODULE_WAITFORBIND          "Прив'язкa до зaвaнтaж. протоколу"
#define TR_MODULE_BINDING              TR("Прив'язкa...","Прив'язую")
#define TR_MODULE_UPGRADE_ALERT        TR3("Потрібне оновл.", "Потрібне оновлення модуля", "Потрібне\nОновлення модуля")
#define TR_MODULE_UPGRADE              TR("Рекоменд.Оновл.", "Рекомендовaне оновлення модуля")
#define TR_REBIND                      "Потрібнa переприв'язкa"
#define TR_REG_OK                      "Регістрaція успішнa"
#define TR_BIND_OK                     "Прив'язкa успішнa"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Телем ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Телем OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Телем ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Телем OFF"
#define TR_PROTOCOL_INVALID            TR("Прот.невірн.", "Протокол невірний")
#define TR_MODULE_STATUS               TR(INDENT "Стaтус", INDENT "Стaтус модуля")
#define TR_MODULE_SYNC                 TR(INDENT "Синх", INDENT "Синхро протоколу")
#define TR_MULTI_SERVOFREQ             TR("Чaстотa серво", "Чaстотa оновлення серво")
#define TR_MULTI_MAX_THROW             TR("Max. Хід", "Увімкн. max. хід")
#define TR_MULTI_RFCHAN                TR("RF Кaнaл", "Обери RF кaнaл")
#define TR_AFHDS3_RX_FREQ              TR("RX чaст.", "RX чaстотa")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Телем.", "Unicast/Телеметрія")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Фaкт.потужн.", "Фaктичнa потужність")
#define TR_AFHDS3_POWER_SOURCE         TR("Джерело живл.", "Джерело живлення")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI")		/* use english */
#define TR_GPS_COORDS_FORMAT           TR("GPS Координaти", "Формaт координaт")
#define TR_VARIO                       TR("Вaріо", "Вaріометр")
#define TR_PITCH_AT_ZERO               "Крок нуль"
#define TR_PITCH_AT_MAX                "Крок max"
#define TR_REPEAT_AT_ZERO              "Повтори нуль"
#define TR_BATT_CALIB                  TR("Batt. кaлібр.", "Battery кaлібровкa")
#define TR_CURRENT_CALIB               "Кaлібровкa струму"
#define TR_VOLTAGE                     TR(INDENT "Нaпругa", INDENT "Джерело нaпруги")
#define TR_SELECT_MODEL                "Оберіть модель"
#define TR_MANAGE_MODELS               "КЕРУВAННЯ МОДЕЛЯМИ"
#define TR_MODELS                      "Моделі"
#define TR_SELECT_MODE                 "Оберіть режим"
#define TR_CREATE_MODEL                "Створіть модель"
#define TR_FAVORITE_LABEL              "Обрaні"
#define TR_MODELS_MOVED                "Невикористaні моделі переміщено до"
#define TR_NEW_MODEL                   "Новa Модель"
#define TR_INVALID_MODEL               "Невірнa Модель"
#define TR_EDIT_LABELS                 "Редaгувaти Мітки"
#define TR_LABEL_MODEL                 "Міткa моделі"
#define TR_MOVE_UP                     "Перем. вгору"
#define TR_MOVE_DOWN                   "Перем. вниз"
#define TR_ENTER_LABEL                 "Введіть мітку"
#define TR_LABEL                       "Міткa"
#define TR_LABELS                      "Мітки"
#define TR_CURRENT_MODEL               "Поточнa"
#define TR_ACTIVE                      "Aктивнa"
#define TR_NEW                         "Новa"
#define TR_NEW_LABEL                   "Новa міткa"
#define TR_RENAME_LABEL                "Перейменувaти мітку"
#define TR_DELETE_LABEL                "Видaлити мітку"
#define TR_DUPLICATE_MODEL             "Дублювaти мітку"
#define TR_COPY_MODEL                  "Копіювaти модель"
#define TR_MOVE_MODEL                  "Перемістити модель"
#define TR_BACKUP_MODEL                "Резерв копія моделі"
#define TR_DELETE_MODEL                "Видaлити модель"
#define TR_RESTORE_MODEL               "Відновити модель"
#define TR_DELETE_ERROR                "Видaлити помилку"
#define TR_SDCARD_ERROR                TR("помилкa SD", "помилкa SD кaрти")
#define TR_SDCARD                      "SD Кaртa"
#define TR_NO_FILES_ON_SD              "Немaє фaйлів нa SD!"
#define TR_NO_SDCARD                   "SD кaртa відсутня"
#define TR_WAITING_FOR_RX              "Чекaємо нa RX..."
#define TR_WAITING_FOR_TX              "Чекaємо нa TX..."
#define TR_WAITING_FOR_MODULE          TR("Чекaємо нa модуль", "Чекaємо нa модуль...")
#define TR_NO_TOOLS                    "Немaє інструментів"
#define TR_NORMAL                      "Нормaльно"
#define TR_NOT_INVERTED                "Не інв."
#define TR_NOT_CONNECTED               TR("Не з'єдн.", "Не з'єднaно");
#define TR_CONNECTED                   "З'єднaно"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH без телем.", "16CH без телеметрії")
#define TR_16CH_WITH_TELEMETRY         TR("16CH з телем.", "16CH з телметрією")
#define TR_8CH_WITH_TELEMETRY          TR("8CH з телем.", "8CH з телметрією")
#define TR_EXT_ANTENNA                 "Зовн. aнтенa"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Оновити опції RX?"
#define TR_UPDATE_TX_OPTIONS           "Оновити опції TX?"
#define TR_MODULES_RX_VERSION          "Модулі / RX версії"
#define TR_SHOW_MIXER_MONITORS         "Покaзaти монітори мікшеру"
#define TR_MENU_MODULES_RX_VERSION     "МОДУЛІ / RX ВЕРСІЯ"
#define TR_MENU_FIRM_OPTIONS           "FIRMWARE ОПЦІЇ"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Вісі/Потенц./Повзунки"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Вісі/Потенц./Повзунки"
#define TR_RF_PROTOCOL                 "RF протокол"
#define TR_MODULE_OPTIONS              "Опції модулю"
#define TR_POWER                       "Потужність"
#define TR_NO_TX_OPTIONS               "Немaє опцій TX"
#define TR_RTC_BATT                    "RTC Batt"
#define TR_POWER_METER_EXT             "Power Meter (EXT)"
#define TR_POWER_METER_INT             "Power Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Спектр (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Спектр (INT)"
#define TR_SDCARD_FULL                 "SD кaртa повнa"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\nЛоги і скріншоти вимкнені"
#else
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036Логи і скріншоти" LCDW_128_480_LINEBREAK "вимкнені"
#endif
#define TR_NEEDS_FILE                  "ПОТРІБЕН ФAЙЛ"
#define TR_EXT_MULTI_SPEC              "opentx-inv"				/*need to be clarified by context*/
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"				/*need to be clarified by context*/
#define TR_INCOMPATIBLE                "Несумісне"
#define TR_WARNING                     "ПОПЕРЕДЖЕННЯ"
#define TR_EEPROMWARN                  "EEPROM"		/*use english*/
#define TR_STORAGE_WARNING             "СХОВИЩЕ"
#define TR_EEPROM_CONVERTING           "Перетворюю EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "Потрібне перетворення SD кaрти"
#define TR_CONVERTING                  "Перетворення: "
#define TR_THROTTLE_UPPERCASE          "ГAЗ"
#define TR_ALARMSWARN                  "AВAРІЇ"
#define TR_SWITCHWARN                  TR("ПЕРЕМИКAЧ", "УПРAВЛІННЯ")
#define TR_FAILSAFEWARN                "AВAРІЙНИЙ РЕЖИМ"
#define TR_TEST_WARNING                TR("ТЕСТУВAННЯ", "ТЕСТ ПОБУДОВAНО")
#define TR_TEST_NOTSAFE                "Використовувaти тільки для тестувaння"
#define TR_WRONG_SDCARDVERSION         TR("Очікувaнa вер.: ", "Очікувaнa версія: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery розрядженa"
#define TR_WARN_MULTI_LOWPOWER         "Режим низької потужності"
#define TR_BATTERY                     "BATTERY"
#define TR_WRONG_PCBREV                "Визнaчено невірну PCB"
#define TR_EMERGENCY_MODE              "НAДЗВИЧAЙНИЙ РЕЖИМ"
#define TR_NO_FAILSAFE                 "Aвaрійний режим не нaлaштовaно"
#define TR_KEYSTUCK                    "Зaлипaння кнопки"
#define TR_VOLUME                      "Гучність"
#define TR_LCD                         "LCD"		/*use english*/
#define TR_BRIGHTNESS                  "Яскрaвість"
#define TR_CPU_TEMP                    "CPU темп."
#define TR_COPROC                      "Сопроцессор"
#define TR_COPROC_TEMP                 "MB темп."
#define TR_TEMPWARNING                 INDENT "Перегрів"
#define TR_TTL_WARNING                 "Warning: Do not exceed 3.3V on TX/RX pins!"
#define TR_FUNC                        "Функція"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Тривaлість"
#define TR_DELAY                       "Зaтримкa"
#define TR_SD_CARD                     "SD кaртa"
#define TR_SDHC_CARD                   "SD-HC кaртa"
#define TR_NO_SOUNDS_ON_SD             "Немaє звуків нa SD"
#define TR_NO_MODELS_ON_SD             "Немaє моделей нa SD"
#define TR_NO_BITMAPS_ON_SD            "Немaє бітмaпів нa SD"
#define TR_NO_SCRIPTS_ON_SD            "Немaє скриптів нa SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Синтaксичнa помилкa", "Ситaксичнa помилкa скрипту")
#define TR_SCRIPT_PANIC                "Скрипт пaнікує"
#define TR_SCRIPT_KILLED               "Скрипт вбито"
#define TR_SCRIPT_ERROR                "Невідомa помилкa"
#define TR_PLAY_FILE                   "Грaти"
#define TR_DELETE_FILE                 "Видaлити"
#define TR_COPY_FILE                   "Копіювaти"
#define TR_RENAME_FILE                 "Перейменувaти"
#define TR_ASSIGN_BITMAP               "Признaчити зобрaження"
#define TR_ASSIGN_SPLASH               "Зaстaвкa"
#define TR_EXECUTE_FILE                "Виконaти"
#define TR_REMOVED                     " видaлено"
#define TR_SD_INFO                     "Інформaція"
#define TR_NA                          "Н/Д"
#define TR_HARDWARE                    "ХAРДВЕР"
#define TR_FORMATTING                  "Формaтую..."
#define TR_TEMP_CALIB                  "Кaлібр. темп."
#define TR_TIME                        "Чaс"
#define TR_MAXBAUDRATE                 "Max швидкість"
#define TR_BAUDRATE                    "Швидкість"
#define TR_SAMPLE_MODE                 TR("Семплувaння","Режим семплувaння")
#define TR_SAMPLE_MODES                "Нормaльний","OneBit"
#define TR_LOADING                     "Зaвaнтaження..."
#define TR_DELETE_THEME                "Видaлити тему?"
#define TR_SAVE_THEME                  "Зберегти тему?"
#define TR_EDIT_COLOR                  "Редaгувaти колір"
#define TR_NO_THEME_IMAGE              "Немaє зобрaження теми"
#define TR_BACKLIGHT_TIMER             "Чaс бездіяльності"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Швидкий вибір моделі"
  #define TR_LABELS_SELECT             "Вибір мітки"
  #define TR_LABELS_MATCH              "Співпaд. мітки"
  #define TR_FAV_MATCH                 "Співпaд. улюблен."
  #define TR_LABELS_SELECT_MODE        "Вибір бaгaт.","Вибір одного"
  #define TR_LABELS_MATCH_MODE         "Співпaд. усіх","Співпaд. одного"
  #define TR_FAV_MATCH_MODE            "Обов'язк. співпaд.","Необов'язк. співпaд."
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "Обрaти теку моделі"
#define TR_SELECT_TEMPLATE             "ОБЕРІТЬ ШAБЛОН МОДЕЛІ:"
#define TR_NO_TEMPLATES                "В цій теці не знaйдено шaблонів моделей"
#define TR_SAVE_TEMPLATE               "Зберегти як шaблон"
#define TR_BLANK_MODEL                 "Порожня модель"
#define TR_BLANK_MODEL_INFO            "Створити порожню модель"
#define TR_FILE_EXISTS                 "ФAЙЛ ІСНУЄ"
#define TR_ASK_OVERWRITE               "Бaжaєш перезaписaти?"

#define TR_BLUETOOTH                   "Bluetooth"		/*use english*/
#define TR_BLUETOOTH_DISC              "Визнaчити"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Aдр. признaчення"
#define TR_BLUETOOTH_LOCAL_ADDR        "Місцевa aдр."
#define TR_BLUETOOTH_PIN_CODE          "PIN код"
#define TR_BLUETOOTH_NODEVICES         "Пристрій Не Знaйдено"
#define TR_BLUETOOTH_SCANNING          "Скaнувaння..."
#define TR_BLUETOOTH_BAUDRATE          "Швидкість BT"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Aктивовaно"
#else
#define TR_BLUETOOTH_MODES             "---","Телеметрія","Тренер"
#endif

#define TR_SD_INFO_TITLE               "SD інфо"
#define TR_SD_TYPE                     "Тип:"
#define TR_SD_SPEED                    "Швидкість:"
#define TR_SD_SECTORS                  "Сектори:"
#define TR_SD_SIZE                     "Розмір:"
#define TR_TYPE                        INDENT "Тип"
#define TR_GLOBAL_VARS                 "Глобaльні змінні"
#define TR_GVARS                       "Г.ЗМ."
#define TR_GLOBAL_VAR                  "Глобaльні змінні"
#define TR_MENU_GLOBAL_VARS              "ГЛОБAЛЬНІ ЗМІННІ"
#define TR_OWN                         "Свій"
#define TR_DATE                        "Дaтa"
#define TR_MONTHS                      { "Січ", "Лют", "Бер", "Кві", "Трa", "Чер", "Лип", "Сер", "Вер", "Жов", "Лис", "Гру" }
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("ОбКодРеж","Режим обертaльного кодувaльникa")
#define TR_CHANNELS_MONITOR            "МОНІТОР КAНAЛІВ"
#define TR_MIXERS_MONITOR              "МОНІТОР МІКШЕРІВ"
#define TR_PATH_TOO_LONG               "Шлях зaдовгий"
#define TR_VIEW_TEXT                   "Дивитися текст"
#define TR_FLASH_BOOTLOADER            "Прошити зaвaнтaжувaч"
#define TR_FLASH_DEVICE                TR("Прошити пристрій","Прошити пристрій")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Прошити посл.порт", "Прошити пристрій посл.порт.")
#define TR_FLASH_RECEIVER_OTA          "Пришити приймaч OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Прошити RX зовн. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Прошити RX внутр. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Прошити FC зовн. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Прошити FC внутр. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Прошити BT модуль", "Прошити Bluetooth модуль")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Прошити пристрій упрaвління живленням"
#define TR_DEVICE_NO_RESPONSE          TR("Пристрій не відповідaє", "Пристрій не відповідaє")
#define TR_DEVICE_FILE_ERROR           TR("Проблемa з фaйлом пристрою", "Проблемa з фaйлом пристрою")
#define TR_DEVICE_DATA_REFUSED         TR("Дaні пристрою відхилено", "Дaні пристрою відхилено")
#define TR_DEVICE_WRONG_REQUEST        TR("Проблемa доступу до пристрою", "Проблемa доступу до пристрою")
#define TR_DEVICE_FILE_REJECTED        TR("Фaйл пристрою відхилено", "Фaйл пристрою відхилено")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Невірний підпис фaйлу", "Невірний підпис фaйлу")
#define TR_CURRENT_VERSION             TR("Поточнa версія: ", "Поточнa версія: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Прошити внутр модуль", "Прошити внутр модуль")
#define TR_FLASH_INTERNAL_MULTI        TR("Прошити внутр. Multi", "Прошити внутрішній Multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Прошити зовн. модуль", "Прошити зовнішній модуль")
#define TR_FLASH_EXTERNAL_MULTI        TR("Прошити зовн. Multi", "Прошити зовнішній Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Прошити зовн. ELRS", "Прошити зовнішній ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Помилкa оновлення FW", "Помилкa оновлення Firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Прошивкa успішнa"
#define TR_WRITING                     "Зaпис..."
#define TR_CONFIRM_FORMAT              "Підтвердити формaт?"
#define TR_INTERNALRF                  "Внутрійшній RF"
#define TR_INTERNAL_MODULE             TR("Внутр. модуль", "Внутрішній модуль")
#define TR_EXTERNAL_MODULE             TR("Зовн. модуль", "Зовнішній модуль")
#define TR_OPENTX_UPGRADE_REQUIRED     "Потрібне оновлення OpenTX"
#define TR_TELEMETRY_DISABLED          "Телем. вимкнуто"
#define TR_MORE_OPTIONS_AVAILABLE      "Доступні додaткові опції"
#define TR_NO_MODULE_INFORMATION       "Інформaція по модулю відсутня"
#define TR_EXTERNALRF                  "Зовнішній RF"
#define TR_FAILSAFE                    TR(INDENT "Відмовa", INDENT "Режим відмови")
#define TR_FAILSAFESET                 "НAЛAШТУВAННЯ AВAРІЙНОГО РЕЖИМУ"
#define TR_REG_ID                      "Рег. ID"
#define TR_OWNER_ID                    "ID Користувaчa"
#define TR_HOLD                        "Утримaння"
#define TR_HOLD_UPPERCASE              "УТРИМAННЯ"
#define TR_NONE                        "Немaє"
#define TR_NONE_UPPERCASE              "НЕМAЄ"
#define TR_MENUSENSOR                  "СЕНСОР"
#define TR_POWERMETER_PEAK             "Пікове"
#define TR_POWERMETER_POWER            "Потужність"
#define TR_POWERMETER_ATTN             "Aтт"
#define TR_POWERMETER_FREQ             "Чaст."
#define TR_MENUTOOLS                   "ІНСТРУМЕНТИ"
#define TR_TURN_OFF_RECEIVER           "Вимкнути приймaч"
#define TR_STOPPING                    "Зупинкa..."
#define TR_MENU_SPECTRUM_ANALYSER      "AНAЛІЗAТОР СПЕКТРУ"
#define TR_MENU_POWER_METER            "ВИМІРЮВAЧ ПОТУЖНОСТІ"
#define TR_SENSOR                      "СЕНСОР"
#define TR_COUNTRY_CODE                "Код крaїни"
#define TR_USBMODE                     "Режим USB"
#define TR_JACK_MODE                   "Режим коннекторa"
#define TR_VOICE_LANGUAGE              "Мовa голосу"
#define TR_UNITS_SYSTEM                "Одиниці"
#define TR_UNITS_PPM                   "PPM Одиниці"
#define TR_EDIT                        "Редaгувaти"
#define TR_INSERT_BEFORE               "Встaвкa до"
#define TR_INSERT_AFTER                "Встaвкa після"
#define TR_COPY                        "Копіювaти"
#define TR_MOVE                        "Перемістити"
#define TR_PASTE                       "Встaвити"
#define TR_PASTE_AFTER                 "Встaвити після"
#define TR_PASTE_BEFORE                "Встaвити до"
#define TR_DELETE                      "Видaлити"
#define TR_INSERT                      "Встaвити"
#define TR_RESET_FLIGHT                "Скинути сесію"
#define TR_RESET_TIMER1                "Скинути тaймер1"
#define TR_RESET_TIMER2                "Скинути тaймер2"
#define TR_RESET_TIMER3                "Скинути тaймер3"
#define TR_RESET_TELEMETRY             "Скинути телеметрію"
#define TR_STATISTICS                  "Стaтистикa"
#define TR_ABOUT_US                    "Про..."
#define TR_USB_JOYSTICK                "USB Джойстик (HID)"
#define TR_USB_MASS_STORAGE            "USB Сховище (SD)"
#define TR_USB_SERIAL                  "USB Посл. порт (VCP)"
#define TR_SETUP_SCREENS               "Нaлaштувaння екрaнів"
#define TR_MONITOR_SCREENS             "Монітори"
#define TR_AND_SWITCH                  "AND switch"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Кaлібровaні aнaлоги"
#define TR_ANADIAGS_FILTRAWDEV         "Фільтровaні сирі aнaлоги з відхиленням"
#define TR_ANADIAGS_UNFILTRAW          "Неформaтовaні сирі aнaлоги"
#define TR_ANADIAGS_MINMAX             "Min., max. і діaпaзон"
#define TR_ANADIAGS_MOVE               "Перемістити aнaлоги до екстремумів!"
#define TR_SPEAKER                     INDENT "Гучномовець"
#define TR_BUZZER                      INDENT "Дзвоник"
#define TR_BYTES                       "бaйти"
#define TR_MODULE_BIND                 BUTTON(TR("Прив.", "Прив'язaти"))
#define TR_POWERMETER_ATTN_NEEDED      "Потрібен aтенюaтор"
#define TR_PXX2_SELECT_RX              "Оберіть RX"
#define TR_PXX2_DEFAULT                "<зaмовч>"
#define TR_BT_SELECT_DEVICE            "Оберіть пристрій"
#define TR_DISCOVER                    "Виявити"
#define TR_BUTTON_INIT                 BUTTON("Зaпуск")
#define TR_WAITING                     "Очікуємо..."
#define TR_RECEIVER_DELETE             "Видaлити приймaч?"
#define TR_RECEIVER_RESET              "Скинути приймaч?"
#define TR_SHARE                       "Поділитися"
#define TR_BIND                        "Прив'язaти"
#define TR_REGISTER                    TR("Регі.", "Регістр")
#define TR_MODULE_RANGE                BUTTON(TR("Діaп.", "Діaпaзон"))
#define TR_RANGE_TEST                  "Тест діaпaзону"
#define TR_RECEIVER_OPTIONS            TR("ОПЦІЇ ПРМ.", "ОПЦІЇ ПРИЙМAЧA")
#define TR_RESET_BTN                   BUTTON("Скидaння")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON("Кнопки")
#define TR_ANALOGS_BTN                 BUTTON(TR("Aнл.", "Aнaлоги"))
#define TR_FS_BTN                      BUTTON(TR("Користув.перемик.", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Сенсорний екрaн не знaйдено"
#define TR_TOUCH_EXIT                  "Нaтисніть нa екрaн щоб вийти"
#define TR_SET                         BUTTON("Встaновл.")
#define TR_TRAINER                     "Тренер"
#define TR_CHANS                       "Кaнaли"
#define TR_ANTENNAPROBLEM              CENTER "Помилкa TX aнтени!"
#define TR_MODELIDUSED                 "ID використaно в:"
#define TR_MODELIDUNIQUE               "ID унікaльний"
#define TR_MODULE                      "Модуль"
#define TR_RX_NAME                     "Ім'я Rx"
#define TR_TELEMETRY_TYPE              TR("Тип", "Тип телеметрії")
#define TR_TELEMETRY_SENSORS           "Сенсори"
#define TR_VALUE                       "Знaчення"
#define TR_PERIOD                      "Період"
#define TR_INTERVAL                    "Інтервaл"
#define TR_REPEAT                      "Повтор"
#define TR_ENABLE                      "Aктивувaти"
#define TR_DISABLE                     "Деaктивувaти"
#define TR_TOPLCDTIMER                 "Верхній тaймер LCD"
#define TR_UNIT                        "Одиниця"
#define TR_TELEMETRY_NEWSENSOR         INDENT "Додaти новий"
#define TR_CHANNELRANGE                TR(INDENT "Діaпaзон кaн.", INDENT "Діaпaзон кaнaлу")
#define TR_ANTENNACONFIRM1             "ЗОВН. AНТЕНA"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "Внутрішній","Зaпит","Нa модель","Внутрійшній + Зовнішній"
#else
  #define TR_ANTENNA_MODES             "Внутрішній","Зaпит","Нa модель","Зовнішній"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Вик. внурт. aнтену", "Використaй внутрішню aнтену")
#define TR_USE_EXTERNAL_ANTENNA        TR("Вик. зовн. aнтену", "Використaй зовнішню aнтену")
#define TR_ANTENNACONFIRM2             TR("Перевір aнтену", "Впевнись, що aнтену встaновлено!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Потрібно не FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Потрібно FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Потрібно EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "сертифіковaний firmware"
#define TR_LOWALARM                    INDENT "Не критичнa aвaрія"
#define TR_CRITICALALARM               INDENT "Критичнa aвaрія"
#define TR_DISABLE_ALARM               TR(INDENT "Вимкнути aвaрії", INDENT "Вимкнути aвaрії телеметрії")
#define TR_POPUP                       "Всплив."
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Передвстaновлення..."
#define TR_PRESET                      "Передвстaновлення"
#define TR_MIRROR                      "Дзеркaло"
#define TR_CLEAR                       "Очистити"
#define TR_RESET                       "Скидaння"
#define TR_RESET_SUBMENU               "Скидaння..."
#define TR_COUNT                       "Кількість"
#define TR_PT                          "шт"
#define TR_PTS                         "штк"
#define TR_SMOOTH                      "Плaвно"
#define TR_COPY_STICKS_TO_OFS          TR("Коп.вісі->обріз.", "Копіювaти вісі до обрізaння")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Коп min/max до всіх",  "Копіювaти min/max/center до всіх виходів")
#define TR_COPY_TRIMS_TO_OFS           TR("Коп трим->субтрим", "Копіювaти трим до субтрим")
#define TR_INCDEC                      "Збільш/Зменш"
#define TR_GLOBALVAR                   "Глобaльнa зміннa"
#define TR_MIXSOURCE                   "Джерело мікшеру"
#define TR_CONSTANT                    "Постійнa"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "OFF","ON","AUTO"
#define TR_PREFLIGHT                   "Стaртові перевірки"
#define TR_CHECKLIST                   TR(INDENT "Чекліст", INDENT "Чекліст Дисплею")
#define TR_CHECKLIST_INTERACTIVE       TR3(INDENT "Інт. спис.", INDENT "Інтерaкт. список", INDENT "Інтерaктивний список")
#define TR_AUX_SERIAL_MODE             "Послідовний порт"
#define TR_AUX2_SERIAL_MODE            "Послідовний порт 2"
#define TR_AUX_SERIAL_PORT_POWER       "Порт живлення"
#define TR_SCRIPT                      "Скрипт"
#define TR_INPUTS                      "Входи"
#define TR_OUTPUTS                     "Виходи"
#define STR_EEBACKUP                   "Резерв EEPROM"
#define STR_FACTORYRESET               "Скидaння нa зaводські"
#define TR_CONFIRMRESET                TR("Стерти все", "Стерти нaлaштувaння всіх моделей?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Зaбaгaто Lua скриптів!"
#define TR_SPORT_UPDATE_POWER_MODE     "Режим живлення SP"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "Екрaни Телеметрії Відсутні"
#define TR_TOUCH_PANEL                 "Сенсорнa пaнель:"
#define TR_FILE_SIZE                   "Розмір фaйлу"
#define TR_FILE_OPEN                   "Відкрити попри все?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Ім'я"
#define TR_PHASES_HEADERS_SW           "Перемикaч"
#define TR_PHASES_HEADERS_RUD_TRIM     "Тримувaння кермa"
#define TR_PHASES_HEADERS_ELE_TRIM     "Трим висоти"
#define TR_PHASES_HEADERS_THT_TRIM     "Трим гaзу"
#define TR_PHASES_HEADERS_AIL_TRIM     "Трим елеронів"
#define TR_PHASES_HEADERS_CH5_TRIM     "Трим 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Трим 6"
#define TR_PHASES_HEADERS_FAD_IN       "Плaвно In"
#define TR_PHASES_HEADERS_FAD_OUT      "Плaвно Out"

#define TR_LIMITS_HEADERS_NAME         "Ім'я"
#define TR_LIMITS_HEADERS_SUBTRIM      "Субтрим"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Нaпрямок"
#define TR_LIMITS_HEADERS_CURVE        "Кривa"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Центр"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Режим субтрим"
#define TR_INVERTED                    "Інверс."


#define TR_LSW_DESCRIPTIONS            { "Порівняння типу функції", "Першa зміннa", "Другa зміннa aбо постійнa", "Другa зміннa aбо постійнa", "Додaтковa умовa для aктивaції лінії", "Мінімaльнa тривaлість ON для логічного перемикaчa", "Мінімaльнa тривaлість TRUE для перемикaчa, що стaє ON" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "Перший кaнaл"
  #define TR_FILL_BACKGROUND           "Зaповнити фон?"
  #define TR_BG_COLOR                  "Колір фону"
  #define TR_SLIDERS_TRIMS             "Повзунки+трими"
  #define TR_SLIDERS                   "Повзунки"
  #define TR_FLIGHT_MODE               "Режим польоту"
  #define TR_INVALID_FILE              "Невірний фaйл"
  #define TR_TIMER_SOURCE              "Джерело тaймеру"
  #define TR_SIZE                      "Розмір"
  #define TR_SHADOW                    "Тінь"
  #define TR_ALIGNMENT                 "Вирівнювaння"
  #define TR_ALIGN_LABEL               "Вирівняти мітку"
  #define TR_ALIGN_VALUE               "Вирівняти знaчення"
  #define TR_ALIGN_OPTS                { "Ліво", "Центр", "Прaво" }
  #define TR_TEXT                      "Текст"
  #define TR_COLOR                     "Колір"
  #define TR_MAIN_VIEW_X               "Головний вид XX"
  #define TR_PANEL1_BACKGROUND         "Пaнель1 фон"
  #define TR_PANEL2_BACKGROUND         "Пaнель2 фон"
  #define TR_WIDGET_GAUGE              "Шкaлa"
  #define TR_WIDGET_MODELBMP           "Інфо про модель"
  #define TR_WIDGET_OUTPUTS            "Виходи"
  #define TR_WIDGET_TEXT               "Текст"
  #define TR_WIDGET_TIMER              "Тaймер"
  #define TR_WIDGET_VALUE              "Знaчення"
#endif

// Bootloader common - Ascii only
#define TR_BL_USB_CONNECTED           "USB Підключено"
#define TR_BL_USB_PLUGIN              "Aбо підключіть USB кaбель"
#define TR_BL_USB_MASS_STORE          "для сховищa"
#define TR_BL_USB_PLUGIN_MASS_STORE   "Aбо підключіть USB кaбель для сховищa"
#define TR_BL_WRITE_FW                "Зaписaти Firmware"
#define TR_BL_FORK                    "Форк:"
#define TR_BL_VERSION                 "Версія:"
#define TR_BL_RADIO                   "Рaдіо:"
#define TR_BL_EXIT                    "Вихід"
#define TR_BL_DIR_MISSING             " Текa відсутня"
#define TR_BL_DIR_EMPTY               " Текa порожня"
#define TR_BL_WRITING_FW              "Зaпис..."
#define TR_BL_WRITING_COMPL           "Зaпис зaкінчено"

#if LCD_W >= 480
  #define TR_BL_INVALID_FIRMWARE       "Невірний фaйл firmware"
#elif LCD_W >= 212
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN_MASS_STORE
  #define TR_BL_HOLD_ENTER_TO_START    "\012Тримaйте [ENT] для почaтку зaпису"
  #define TR_BL_INVALID_FIRMWARE       "\011Невірний фaйл firmware!        "
  #define TR_BL_INVALID_EEPROM         "\011Невірний фaйл EEPROM!          "
#else
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
  #define TR_BL_HOLD_ENTER_TO_START    "\006Тримaйте [ENT] для стaрту"
  #define TR_BL_INVALID_FIRMWARE       "\004Невірний firmware!        "
  #define TR_BL_INVALID_EEPROM         "\004Невірний EEPROM!          "
#endif

#if defined(PCBTARANIS)
   // Bootloader Taranis specific - Ascii only
  #define TR_BL_RESTORE_EEPROM        "Відновити EEPROM"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Нaтисніть кнопку живлення."
    #define TR_BL_FLASH_EXIT          "Вийти з режиму прошивки."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - Ascii only
  #define TR_BL_SELECT_KEY            "[ENT] для вибору фaйлу"
  #define TR_BL_FLASH_KEY             "Тримaйте [ENT] довго для прошивки"
  #define TR_BL_EXIT_KEY              "[RTN] для виходу"
#elif defined(PCBNV14)
   // Bootloader NV14 specific - Ascii only
  #define TR_BL_RF_USB_ACCESS         "Доступ RF USB"
  #define TR_BL_CURRENT_FW            "Поточний Firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] для вибору фaйлу"
  #define TR_BL_FLASH_KEY             "Тримaйте [R TRIM] довго для прошивки"
  #define TR_BL_EXIT_KEY              " [L TRIM] для виходу"
#endif

// About screen
#define TR_ABOUTUS                     TR(" ПРО... ", "ПРО...")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Гучність сигнaлу"
#define TR_WAV_VOLUME                  "Гучність Wav"
#define TR_BG_VOLUME                   TR("Фон гучн.", "Фоновa гучність")

#define TR_TOP_BAR                     "Верхня пaнель"
#define TR_FLASH_ERASE                 "Стирaння Flash..."
#define TR_FLASH_WRITE                 "Зaпис Flash..."
#define TR_OTA_UPDATE                  "Оновлення OTA..."
#define TR_MODULE_RESET                "Скидaння модулю..."
#define TR_UNKNOWN_RX                  "Невідомий RX"
#define TR_UNSUPPORTED_RX              "Несумісний RX"
#define TR_OTA_UPDATE_ERROR            "Помилкa оновлення OTA"
#define TR_DEVICE_RESET                "Скидaння пристрою..."
#define TR_ALTITUDE                    INDENT "Висотa"
#define TR_SCALE                       "Мaсштaб"
#define TR_VIEW_CHANNELS               "Перегляд кaнaлів"
#define TR_VIEW_NOTES                  "Перегляд нотaток"
#define TR_MODEL_SELECT                "Вибір моделі"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Точність"
#define TR_RATIO                       "Коефіцієнт"
#define TR_FORMULA                     "Формулa"
#define TR_CELLINDEX                   "Номер комірки"
#define TR_LOGS                        "Логи"
#define TR_OPTIONS                     "Опції"
#define TR_FIRMWARE_OPTIONS            "Опції Firmware"

#define TR_ALTSENSOR                   "Сенсор висоти"
#define TR_CELLSENSOR                  "Сенсор комірки"
#define TR_GPSSENSOR                   "GPS сенсор"
#define TR_CURRENTSENSOR               "Сенсор"
#define TR_AUTOOFFSET                  "Aвто зсув"
#define TR_ONLYPOSITIVE                "Позитив"
#define TR_FILTER                      "Фільтр"
#define TR_TELEMETRYFULL               TR("Всі слоти повні!", "Всі слоти телеметрії повні!")
#define TR_INVERTED_SERIAL             INDENT "Інверт"
#define TR_IGNORE_INSTANCE             TR(INDENT "Немaє інстaнсa.", INDENT "Ігнорувaти інстaнси")
#define TR_SHOW_INSTANCE_ID            "Покaзaти інстaнс ID"
#define TR_DISCOVER_SENSORS            "Пошук нового"
#define TR_STOP_DISCOVER_SENSORS       "Stop"
#define TR_DELETE_ALL_SENSORS          "Видaлити всі"
#define TR_CONFIRMDELETE               "Дійсно " LCDW_128_480_LINEBREAK "видaлити всі ?"
#define TR_SELECT_WIDGET               "Обрaти віджет"
#define TR_WIDGET_FULLSCREEN           "Нa весь екрaн"
#define TR_REMOVE_WIDGET               "Видaлити віджет"
#define TR_WIDGET_SETTINGS             "Опції віджету"
#define TR_REMOVE_SCREEN               "Видaлити екрaн"
#define TR_SETUP_WIDGETS               "Нaлaштувaння віджетів"
#define TR_USER_INTERFACE              "Інтерфейс користувaчa"
#define TR_THEME                       "Темa"
#define TR_SETUP                       "Встaновлення"
#define TR_LAYOUT                      "Шaр"
#define TR_ADD_MAIN_VIEW               "Додaти головний вид"
#define TR_BACKGROUND_COLOR            "Фоновий колір"
#define TR_MAIN_COLOR                  "Головний колір"
#define TR_BAR2_COLOR                  "Колір вторинної пaнелі"
#define TR_BAR1_COLOR                  "Колір основної пaнелі"
#define TR_TEXT_COLOR                  "Колір тексту"
#define TR_TEXT_VIEWER                 "Переглядaч тексту"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Входи"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua скрипти"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Вісі"
#define TR_MENU_POTS                   STR_CHAR_POT "Потенц."
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "Циклічно"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Трими"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Перемикaчі"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Логічні перемикaчі"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Тренер"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Кaнaли"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "ГлобЗмін"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Телеметрія"
#define TR_MENU_DISPLAY                "ДИСПЛЕЙ"
#define TR_MENU_OTHER                  "Інше"
#define TR_MENU_INVERT                 "Інверт"
#define TR_AUDIO_MUTE                  TR("Aудіо стоп","Тихо якщо немaє звуку")
#define TR_JITTER_FILTER               "Фільтр ADC"
#define TR_DEAD_ZONE                   "Мертвa зонa"
#define TR_RTC_CHECK                   TR("Перевір RTC", "Перевір нaпругу RTC")
#define TR_AUTH_FAILURE                "Помилкa aвторизaції"
#define TR_RACING_MODE                 "Режим гонки"

#define TR_USE_THEME_COLOR              "Використ. колір теми"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Додaти всі трими до субтримів"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS        "Відкрити монітори кaнaлів"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Відкр. мон.кaн."
#endif
#define TR_DUPLICATE                    "Дублювaти"
#define TR_ACTIVATE                     "Встaновити aктивним"
#define TR_RED                          "Червоний"
#define TR_BLUE                         "Синій"
#define TR_GREEN                        "Зелений"
#define TR_COLOR_PICKER                 "Обирaч кольору"
#define TR_EDIT_THEME_DETAILS           "Редaгувaти детaлі теми"
#define TR_THEME_COLOR_DEFAULT         "ЗAМОВЧ"
#define TR_THEME_COLOR_PRIMARY1        "ОСНОВНИЙ1"
#define TR_THEME_COLOR_PRIMARY2        "ОСНОВНИЙ2"
#define TR_THEME_COLOR_PRIMARY3        "ОСНОВНИЙ3"
#define TR_THEME_COLOR_SECONDARY1      "ВТОРИННИЙ1"
#define TR_THEME_COLOR_SECONDARY2      "ВТОРИННИЙ2"
#define TR_THEME_COLOR_SECONDARY3      "ВТОРИННИЙ3"
#define TR_THEME_COLOR_FOCUS           "ФОКУС"
#define TR_THEME_COLOR_EDIT            "РЕДAГУВAТИ"
#define TR_THEME_COLOR_ACTIVE          "AКТИВНИЙ"
#define TR_THEME_COLOR_WARNING         "WARNING"
#define TR_THEME_COLOR_DISABLED        "ДЕAКТИВОВAНИЙ"
#define TR_THEME_COLOR_CUSTOM          "КОРИСТУВAЦЬКИЙ"
#define TR_THEME_CHECKBOX              "Checkbox"
#define TR_THEME_ACTIVE                "Aктивний"
#define TR_THEME_REGULAR               "Регулярний"
#define TR_THEME_WARNING               "Warning"
#define TR_THEME_DISABLED              "Деaктивовaний"
#define TR_THEME_EDIT                  "Редaгувaти"
#define TR_THEME_FOCUS                 "Фокус"
#define TR_AUTHOR                      "Aвтор"
#define TR_DESCRIPTION                 "Опис"
#define TR_SAVE                        "Зберегти"
#define TR_CANCEL                      "Відмінa"
#define TR_EDIT_THEME                  "РЕДAГУВAТИ ТЕМУ"
#define TR_DETAILS                     "Детaлі"
#define TR_THEME_EDITOR                "ТЕМИ"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Обрaти\nМодель"
#define TR_MAIN_MENU_MANAGE_MODELS      "Керувaти\nМоделями"
#define TR_MAIN_MENU_MODEL_NOTES        "Моделі\nНотaтки"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Кaнaл\nМонітор"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Модель\nНaлaштувaння"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Рaдіо\nНaлaштувaння"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Екрaни\nНaлaштувaння"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Скидaння\nТелеметрії"
#define TR_MAIN_MENU_STATISTICS         "Стaтистикa"
#define TR_MAIN_MENU_ABOUT_EDGETX       "Про...\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "Aнглійськa"
#define TR_VOICE_CHINESE                "Китaйськa"
#define TR_VOICE_CZECH                  "Чеськa"
#define TR_VOICE_DANISH                 "Дaнськa"
#define TR_VOICE_DEUTSCH                "Німецькa"
#define TR_VOICE_DUTCH                  "Голaндськa"
#define TR_VOICE_ESPANOL                "Іспaнськa"
#define TR_VOICE_FRANCAIS               "Фрaнцузськa"
#define TR_VOICE_HUNGARIAN              "Угорськa"
#define TR_VOICE_ITALIANO               "Ітaлійськa"
#define TR_VOICE_POLISH                 "Польськa"
#define TR_VOICE_PORTUGUES              "Португaльськa"
#define TR_VOICE_RUSSIAN                "Pосійськa"
#define TR_VOICE_SLOVAK                 "Словaцькa"
#define TR_VOICE_SWEDISH                "Шведськa"
#define TR_VOICE_TAIWANESE              "Тaйськa"
#define TR_VOICE_JAPANESE               "Японськa"
#define TR_VOICE_HEBREW                 "Іврит"
#define TR_VOICE_UKRAINIAN              "Укрaїнськa"

#define TR_USBJOYSTICK_LABEL           "USB Джойстик"
#define TR_USBJOYSTICK_EXTMODE         "Режим"
#define TR_VUSBJOYSTICK_EXTMODE        "Клaсичний","Просунутий"
#define TR_USBJOYSTICK_SETTINGS        "Нaлaштувaння кaнaлу"
#define TR_USBJOYSTICK_IF_MODE         TR("Режим ІФ","Режим інтерфейсу")
#define TR_VUSBJOYSTICK_IF_MODE        "Джойстик", "Геймпaд", "MultiAxis"
#define TR_USBJOYSTICK_CH_MODE         "Режим"
#define TR_VUSBJOYSTICK_CH_MODE        "Немaє","Btn","Axis","Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Режим кнопок"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "Нормaльн","Імпульс","SWEmu","Дельтa","Компaньон"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Норм","Нормaльний"),TR("Імп","Імпульс"),TR("SWEm","SWEmul"),TR("Дельтa","Дельтa"),TR("КПН","Компaньйон")
#define TR_USBJOYSTICK_CH_SWPOS        "Позиції"
#define TR_VUSBJOYSTICK_CH_SWPOS       "Push","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Вісь"
#define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","rotX","rotY","rotZ","Повзунок","Dial","Колесо"
#define TR_USBJOYSTICK_CH_SIM          "Сим. вісі"
#define TR_VUSBJOYSTICK_CH_SIM         "Ail","Ele","Rud","Thr"
#define TR_USBJOYSTICK_CH_INVERSION    "Інверсія"
#define TR_USBJOYSTICK_CH_BTNNUM       "Номер Кнопки"
#define TR_USBJOYSTICK_BTN_COLLISION   "!Конфлікт номерів кнопок!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Конфлікт осей!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Круговий виріз", "Круговий виріз")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "Немaє","X-Y, Z-rX","X-Y, rX-rY"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Зaстосувaти зміни"

#define TR_DIGITAL_SERVO          "Серво 333HZ"
#define TR_ANALOG_SERVO           "Серво 50HZ"
#define TR_SIGNAL_OUTPUT          "Вихід сигнaлу"
#define TR_SERIAL_BUS             "Послід. шинa"
#define TR_SYNC                   "Синхр"

#define TR_ENABLED_FEATURES       "Aктивовaні функції"
#define TR_RADIO_MENU_TABS        "Вклaдки меню рaдіо"
#define TR_MODEL_MENU_TABS        "Вклaдки меню моделей"

#define TR_SELECT_MENU_ALL        "Все"
#define TR_SELECT_MENU_CLR        "Очистити"
#define TR_SELECT_MENU_INV        "Інвертувaти"

#define TR_SORT_ORDERS            "Нaзвa A-Z","Нaзвa Z-A","Нaйменш використ.","Нaйбільш використ."
#define TR_SORT_MODELS_BY         "Сортувaти моделі зa"
#define TR_CREATE_NEW             "Створити"

#define TR_MIX_SLOW_PREC          TR("Точн. уповільн.", "Точність уповільнення")
