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

#define TR_MINUTE_SINGULAR            "хвилина"
#define TR_MINUTE_PLURAL1             "хв."
#define TR_MINUTE_PLURAL2             "хв."

#define TR_OFFON                       "OFF","ON"	/* use english */
#define TR_MMMINV                      "---","інв."
#define TR_VBEEPMODE                   "Тиша","Тривога","Без кноп.","Все"
#define TR_VBLMODE                     "OFF","Кноп.",TR("Ctrl","Controls"),"Обидві","ON"
#define TR_TRNMODE                     "OFF",TR("+=","Дод."),TR(":=","Замін.")
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"	/* use english */

#define TR_AUX_SERIAL_MODES            "OFF","Дзерк. телем.","Вх. телем.","SBUS Трен.","LUA","CLI","GPS","Відлаг.","SpaceMouse","Зовн. модуль"	/* for untranslated - use english */
#define TR_SWTYPES                     "Немає","Перемик.","2поз. пер.","3поз. пер."
#define TR_POTTYPES                    "Немає","Потенц.",TR("Пот. з крок.","Потенц. з кроком"),"Повзун.",TR("Баг.поз.пер.","Багатопозиц. перем."),"Вісь X","Вісь Y","Перемик."
#define TR_VPERSISTENT                 "OFF","Політ","Ручн. скид."	/* for untranslated - use english */
#define TR_COUNTRY_CODES               TR("US","America"),TR("JP","Japan"),TR("EU","Europe")	/* use english */
#define TR_USBMODES                    "Вибір",TR("Joyst","Joystick"),TR("SDCard","Storage"),"Послід."	/* for untranslated - use english */
#define TR_JACK_MODES                  "Вибір","Аудіо","Трен."

#define TR_SBUS_INVERSION_VALUES       "норм.","не інверт."
#define TR_MULTI_CUSTOM                "Налаштований"
#define TR_VTRIMINC                    TR("Експо","Експоненц."),TR("Наддрібн.","Над дрібно"),"Дрібно","Середньо","Грубо"
#define TR_VDISPLAYTRIMS               "Немає","Змін.","Так"
#define TR_VBEEPCOUNTDOWN              "Тихо","Сигн.","Голос","Вібро",TR("С & В","Сигн. & Вібро"),TR("Г & В","Голос & Вібро")
#define TR_COUNTDOWNVALUES             "5с","10с","20с","30с"
#define TR_VVARIOCENTER                "Тон","Тиша"
#define TR_CURVE_TYPES                 "Стандарт","Вибір"

#define TR_ADCFILTERVALUES             "Global","Off","On"	/* use english */

#define TR_VCURVETYPE                  "Різн","Експо","Функц.","Корист."
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Дод.","Множ.","Замін."
#define TR_VMLTPX2                     "+=","*=",":="

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Таймер"
  #define TR_CSWSTICKY                 "Липучка"
  #define TR_CSWSTAY                   "Край"
#else
  #define TR_CSWTIMER                  "Тмр"
  #define TR_CSWSTICKY                 "Лип."
  #define TR_CSWSTAY                   "Край"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Тренер"
#define TR_SF_INST_TRIM                "Митєв. трим."
#define TR_SF_RESET                    "Скид."
#define TR_SF_SET_TIMER                "Встан."
#define TR_SF_VOLUME                   "Гучність"
#define TR_SF_FAILSAFE                 "АварРежим"
#define TR_SF_RANGE_CHECK              "ПеревірДіап."
#define TR_SF_MOD_BIND                 "Модуль Bind"
#define TR_SF_RGBLEDS                  "RGB leds"	/* use english */

#define TR_SOUND                       "Грати Звук"
#define TR_PLAY_TRACK                  "Грати Трек"
#define TR_PLAY_VALUE                  TR("Грати Знач.","Грати Знач.")
#define TR_SF_HAPTIC                   "Вібро"
#define TR_SF_PLAY_SCRIPT              "Lua Скрипт"
#define TR_SF_BG_MUSIC                 "Фон.Муз."
#define TR_SF_BG_MUSIC_PAUSE           "Фон.Муз. ||"
#define TR_SF_LOGS                     "SD лог"
#define TR_ADJUST_GVAR                 "Налашт."
#define TR_SF_BACKLIGHT                "Підсвіт."
#define TR_SF_VARIO                    "Варіо"
#define TR_SF_TEST                     "Тест"

#if LCD_W >= 212
  #define TR_SF_SAFETY                 "Перевизнач."
#else
  #define TR_SF_SAFETY                 "Перевизн."
#endif

#define TR_SF_SCREENSHOT               "Скріншот"
#define TR_SF_RACING_MODE              "Режим перегонів"
#define TR_SF_DISABLE_TOUCH            "Сенсор вимк."
#define TR_SF_DISABLE_AUDIO_AMP        "Підсил. вимк."
#define TR_SF_SET_SCREEN               "Вибір гол. екр."
#define TR_SF_SET_TELEMSCREEN          "Set Screen"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Телем", "Телеметрія")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS            "Тмр1","Тмр2","Тмр3"

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("Все","Політ"),TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS                  TR("Сг1","Синг.1"),TR("Сг2","Синг.2"),TR("Сг3","Синг.3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "Метрич",TR("Imper.","Imperial")	/* use english */
#define TR_VTELEMUNIT                  "-","V","A","mA","вузл.","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mm","Hz","ms","us","km","dBm"	/* for untranslated - use english */

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Немає","Цифри","Граф","Скрипт"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "Rud"
#define TR_STICK_NAMES1                "Ele"
#define TR_STICK_NAMES2                "Thr"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "За Замовч.","Високий"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ON","Один"	/* for untranslated - use english */

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "Hats mode"	/* use english */
#define TR_HATSOPT                     "Тільки Трим.","Тільки Кнопки","Перемик.","Global"	/* for untranslated - use english */
#define TR_HATSMODE_TRIMS              "Hats mode: Трим."
#define TR_HATSMODE_KEYS               "Hats mode: Кнопки"
#define TR_HATSMODE_KEYS_HELP          "Ліва сторона:\n"\
                                       "   Праворуч = MDL\n"\
                                       "   Вгору = SYS\n"\
                                       "   Вниз = TELE\n"\
                                       "\n"\
                                       "Права сторона:\n"\
                                       "   Ліворуч = PAGE<\n"\
                                       "   Праворуч = PAGE>\n"\
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
#define TR_SRC_TIME                    "Час"
#define TR_SRC_GPS                     "GPS"	/* use english */
#define TR_SRC_TIMER                   "Тмр"

#define TR_VTMRMODES                   "OFF","ON","Strt","THs","TH%","THt"	/* use english */
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Майстер/Розн."
#define TR_VTRAINER_SLAVE_JACK         "Слейв/Розн."
#define TR_VTRAINER_MASTER_SBUS_MODULE "Майстер/SBUS Модуль"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Майстер/CPPM Модуль"
#define TR_VTRAINER_MASTER_BATTERY     "Майстер/Послід."
#define TR_VTRAINER_BLUETOOTH          "Майстер/" TR("BT","Bluetooth"), "Слейв/" TR("BT","Bluetooth")	/* for untranslated - use english */
#define TR_VTRAINER_MULTI              "Майстер/Мульти"
#define TR_VFAILSAFE                   "Не встан.","Тримай","Корист.","Немає імпульсу","Приймач"
#define TR_VSENSORTYPES                "Корист.","Обчислено"
#define TR_VFORMULAS                   "Дод.","Середн.","Min","Max","Множ.","Сума","Комірка","Витрата","Дистанц."
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Нижн.","1","2","3","4","5","6","7","8","Вищ.","Різниця"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (лише центр)","= (симетричн.)"
#define TR_TIMER_DIR                   TR("Залиш.", "Показ.Залиш"), TR("Пройш.", "Пройшло")
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
#define TR_DELETEMODEL                 "ВИДАЛИТИ МОДЕЛЬ"
#define TR_COPYINGMODEL                "Копіюю модель..."
#define TR_MOVINGMODEL                 "Переміщую модель..."
#define TR_LOADINGMODEL                "Зав. модель..."
#define TR_UNLABELEDMODEL              "Без назви"
#define TR_NAME                        "Ім'я"
#define TR_MODELNAME                   "Ім'я моделі"
#define TR_PHASENAME                   "Ім'я режиму"
#define TR_MIXNAME                     "Ім'я мікш."
#define TR_INPUTNAME                   TR("Ввід", "Введіть ім'я")
#define TR_EXPONAME                    TR("Ім'я", "Ім'я рядку")
#define TR_BITMAP                      "Зображ. моделі"
#define TR_NO_PICTURE                  "Немає зображ."
#define TR_TIMER                       TR("Таймер", "Таймер ")
#define TR_START                       "Start"	/* use english */
#define TR_ELIMITS                     TR("Розш.Ліміт", "Розширені Ліміти")
#define TR_ETRIMS                      TR("Розш.Трим.", "Розширене Обрізання")  /* ned to be clarified by context */
#define TR_TRIMINC                     "Крок трим."
#define TR_DISPLAY_TRIMS               TR("Показ. трим.", "Показати трими")
#define TR_TTRACE                      TR("T-Джерело", "Джерело")
#define TR_TTRIM                       TR("T-Трим-Порожн.", "Тримувати тількти порожнє")
#define TR_TTRIM_SW                    TR("T-Трим-Перем.", "Перемикач тримування")
#define TR_BEEPCTR                     TR("Сигн. центр", "Сигнал в центрі")
#define TR_USE_GLOBAL_FUNCS            TR("Глоб.функц.", "Викор. глобальні функції")
#define TR_PROTOCOL                    TR("Проток.", "Протокол")
#define TR_PPMFRAME                    "PPM прийм"
#define TR_REFRESHRATE                 TR("Оновл.", "Частота оновлення")
#define STR_WARN_BATTVOLTAGE           TR("Вихід VBAT: ", "Warning: вихідний рівень VBAT: ")	/* for untranslated - use english */
#define TR_WARN_5VOLTS                 "Warning: вихідний рівень 5 вольт"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Перемикач"
#define TR_FUNCTION_SWITCHES           "Користувацькі перемикачі"
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
#define TR_SF_SWITCH                   "Тригери"
#define TR_TRIMS                       "Трим."
#define TR_FADEIN                      "Вх.Згасання"
#define TR_FADEOUT                     "Вих.Згасання"
#define TR_DEFAULT                     "(замовч.)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Перевірка FM трим."
#else
  #define TR_CHECKTRIMS                "\006Перев.\012трим."
#endif
#define TR_SWASHTYPE                   "Тип перекосу"
#define TR_COLLECTIVE                  TR("Колективний", "Колективне джерело тону")
#define TR_AILERON                     TR("Крен", "Крен")
#define TR_ELEVATOR                    TR("Тангаж", "Тангаж")
#define TR_SWASHRING                   "Поворот"
#define TR_MODE                        "Режим"
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "Ліво"
#else
  #define TR_LEFT_STICK                "Ліво"
#endif
#define TR_SUBTYPE                     "Підтип"
#define TR_NOFREEEXPO                  "Немає вільного expo!"
#define TR_NOFREEMIXER                 "Немає вільного мікшеру!"
#define TR_SOURCE                      "Джерело"
#define TR_WEIGHT                      "Вага"
#define TR_SIDE                        "Сторона"
#define TR_OFFSET                      "Зсув"
#define TR_TRIM                        "Тримування"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Крива"
#define TR_FLMODE                      TR("Режим", "Режими")
#define TR_MIXWARNING                  "Warning"		/* use english */
#define TR_OFF                         "OFF"			/* use english */
#define TR_ANTENNA                     "Антена"
#define TR_NO_INFORMATION              TR("Немає інфо", "Немає інформації")
#define TR_MULTPX                      "Multiplex"		/* use english */
#define TR_DELAYDOWN                   TR("Затрим менше", "Затримка менше")
#define TR_DELAYUP                     "Затримка більше"
#define TR_SLOWDOWN                    TR("Пов.", "Повільно")	/*need to be clarified by context*/
#define TR_SLOWUP                      "Повільно"
#define TR_MIXES                       "MIXES"
#define TR_CV                          "CV"
#if defined(PCBNV14)
  #define TR_GV                        "GV"
#else
  #define TR_GV                        TR("G", "GV")
#endif
#define TR_RANGE                       "Діапаз."
#define TR_CENTER                      "Центр"
#define TR_ALARM                       "Тривога"
#define TR_BLADES                      "Леза/Піни"	/*need to be clarified by context*/
#define TR_SCREEN                      "Екран\001"
#define TR_SOUND_LABEL                 "Звук"
#define TR_LENGTH                      "Довжина"
#define TR_BEEP_LENGTH                 "Довжина сигн."
#define TR_BEEP_PITCH                  "Тон сигналу"	/*need to be clarified by context*/
#define TR_HAPTIC_LABEL                "Вібро"
#define TR_STRENGTH                    "Інтенсивність"
#define TR_IMU_LABEL                   "IMU"		/*need to be clarified by context*/
#define TR_IMU_OFFSET                  "Зсув"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Контраст"
#define TR_ALARMS_LABEL                "Тривоги"
#define TR_BATTERY_RANGE               TR("Діапаз. акум.", "Діапазон акумулятора")
#define TR_BATTERYCHARGING             "Заряджання..."
#define TR_BATTERYFULL                 "Batt заряджено"
#define TR_BATTERYNONE                 "Batt відсутня!"
#define TR_BATTERYWARNING              "Batt розрядж."
#define TR_INACTIVITYALARM             "Бездіяльність"
#define TR_MEMORYWARNING               "Пам'ять закінч."
#define TR_ALARMWARNING                "Звук вимк."
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI вимк.", "Перевір RSSI при вимкн.")		/*need to be clarified by context*/
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Тренер при вимк", "Перевіряти тренер при вимиканні")
#define TR_MODEL_STILL_POWERED         "Модель досі під живленням"
#define TR_TRAINER_STILL_CONNECTED     "Трен. досі підключено"
#define TR_USB_STILL_CONNECTED         "USB досі підключено"
#define TR_MODEL_SHUTDOWN              "Вимкнути?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Тисни enter для підтвердження"
#define TR_THROTTLE_LABEL              "Газ"
#define TR_THROTTLE_START              "Газ на старт"
#define TR_THROTTLEREVERSE             TR("Газ-реверс", "Реверс")
#define TR_MINUTEBEEP                  TR("Хв.", "Хв. сигнал")
#define TR_BEEPCOUNTDOWN               "Відлік"
#define TR_PERSISTENT                  TR("Присутн.", "Присутній")
#define TR_BACKLIGHT_LABEL             "Підсвітка"
#define TR_GHOST_MENU_LABEL            "Меню Привид"
#define TR_STATUS                      "Статус"
#define TR_BLONBRIGHTNESS              "ON яскравість"
#define TR_BLOFFBRIGHTNESS             "OFF яскравість"
#define TR_KEYS_BACKLIGHT              "Яскравість кнопки"
#define TR_BLCOLOR                     "Колір"
#define TR_SPLASHSCREEN                "Заставка"
#define TR_PLAY_HELLO                  "Звук запуску"
#define TR_PWR_ON_DELAY                "Pwr On затримка"
#define TR_PWR_OFF_DELAY               "Pwr Off затримка"
#define TR_PWR_AUTO_OFF                TR("Автовимкн.","Автовимкнення")
#define TR_PWR_ON_OFF_HAPTIC           TR("Pwr ON/OFF Haptic","Power ON/OFF Haptic")
#define TR_THROTTLE_WARNING            TR("Г-попер.", "Стан Газу")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Користув. полож.", "Користувацьке положення?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Полож. %", "Положення %")
#define TR_SWITCHWARNING               TR("Поз. перем.", "Позиції пермикачів")		/*need to be clarified by context*/
#define TR_POTWARNINGSTATE             "Потенціометри & Повзунки"
#define TR_POTWARNING                  TR("Потенц.", "Позиц. потенц.")		/*need to be clarified by context*/
#define TR_TIMEZONE                    "Часовий пояс"
#define TR_ADJUST_RTC                  "налаштувати RTC"
#define TR_GPS                         "GPS"		/* use english */
#define TR_DEF_CHAN_ORD                TR("Порядок каналів", "Порядок каналів за замовчуванням")
#define TR_STICKS                      "Вісі"
#define TR_POTS                        "Потенціометри"
#define TR_SWITCHES                    "Перемикачі"
#define TR_SWITCHES_DELAY              TR("Затримка відтвор.", "Затримка відтворення (середнє полож. перем.)")
#define TR_SLAVE                       "Slave"		/* use english */
#define TR_MODESRC                     "Mode\006% Source"		/* use english */
#define TR_MULTIPLIER                  "Множник"	
#define TR_CAL                         "Калібрув."
#define TR_CALIBRATION                 "Калібрув."
#define TR_VTRIM                       "Трим - +"
#define TR_CALIB_DONE                  "Калібрування завершене"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Тисни [Enter] для старту"
  #define TR_SETMIDPOINT               "Центруй важіль/потенц./повзунки та тисни [Enter]"
  #define TR_MOVESTICKSPOTS            "Рухайте вісі/потенц./повзунки та тисніть [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " ДЛЯ СТАРТУ"
  #define TR_SETMIDPOINT               "ЦЕНТРУВАТИ ВАЖЕЛІ/ПОВЗУНКИ"
  #define TR_MOVESTICKSPOTS            "РУХАЙТЕ ВІСІ/ПОТЕНЦ."
#else
  #define TR_MENUTOSTART               TR_ENTER " ДЛЯ СТАРТУ"
#if defined(SURFACE_RADIO)
  #define TR_SETMIDPOINT               "ВІДЦЕНТР. ПОТЕНЦ."
  #define TR_MOVESTICKSPOTS            "РУХАЙТЕ ST/TH/ПОТЕНЦ./ВІСІ"
#else
  #define TR_SETMIDPOINT               TR("ВІДЦЕНТР. ВІСІ", "ВІДЦЕНТР. ВІСІ/ПОВЗУНКИ")
  #define TR_MOVESTICKSPOTS            "РУХАЙТЕ ВІСІ/ПОТЕНЦ."
#endif
  #define TR_MENUWHENDONE              TR_ENTER " КОЛИ ЗАКІНЧЕНО"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      "НЕМАЄ ДАНИХ"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"		/* use english */
#define TR_FREE_STACK                  "Free stack"		/* use english */
#define TR_INT_GPS_LABEL               "Внутр. GPS"
#define TR_HEARTBEAT_LABEL             "Пульс"
#define TR_LUA_SCRIPTS_LABEL           "Lua скрипт"
#define TR_FREE_MEM_LABEL              "Вільно RAM"
#define TR_DURATION_MS             TR("[D]","Тривалість(мс): ")
#define TR_INTERVAL_MS             TR("[I]","Інтервал(мс): ")
#define TR_MEM_USED_SCRIPT         "Скрипт(B): "
#define TR_MEM_USED_WIDGET         "Віджет(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Аудіо: "
#define TR_GPS_FIX_YES                 "Фіксація: Так"
#define TR_GPS_FIX_NO                  "Фіксація: Ні"
#define TR_GPS_SATS                    "Супутн: "
#define TR_GPS_HDOP                    "Hdop: "		/* use english */
#define TR_STACK_MENU                  "Меню: "
#define TR_TIMER_LABEL                 "Таймер"
#define TR_THROTTLE_PERCENT_LABEL      "Газ %"
#define TR_BATT_LABEL                  "Battery"		/* use english */
#define TR_SESSION                     "Сесія"
#define TR_MENUTORESET                 TR_ENTER " для скидання"
#define TR_PPM_TRAINER                 "TR"		/* use english */
#define TR_CH                          "CH"		/* use english */
#define TR_MODEL                       "МОДЕЛЬ"
#if defined(SURFACE_RADIO)
  #define TR_FM                        "DM"		/* use english */
#else
  #define TR_FM                        "FM"		/* use english */
#endif
#define TR_EEPROMLOWMEM                "мало пам'яті EEPROM"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Натисн. кнопку щоб пропустити"
#define TR_THROTTLE_NOT_IDLE           "Газ не вільний"
#define TR_ALARMSDISABLED              "Авар.Сингали вимкнені"
#define TR_PRESSANYKEY                 TR("\010Тисни будь-яку кнопку", "Тисни будь-яку кнопку")
#define TR_BADEEPROMDATA               "невірні дані EEPROM"
#define TR_BAD_RADIO_DATA              "Відсутні або погані радіодані"
#define TR_RADIO_DATA_RECOVERED        TR3("Використовую резервні радіодані", "Використовую резервні налаштування радіо", "Налаштування радіо відновлені з резервних даних")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Невірні налаштування радіо", "Невірні налаштування радіо", "Неможливо прочитати правильні налаштування радіо")
#define TR_EEPROMFORMATTING            "Форматую EEPROM"
#define TR_STORAGE_FORMAT              "Підготовка сховища"
#define TR_EEPROMOVERFLOW              "Переповнення EEPROM"
#define TR_RADIO_SETUP                 "НАЛАШТ. АПАРАТУРИ"
#define TR_MENUTRAINER                 "ТРЕНЕР"
#define TR_MENUSPECIALFUNCS            "ГЛОБАЛЬНІ ФУНКЦІЇ"
#define TR_MENUVERSION                 "ВЕРСІЯ"
#define TR_MENU_RADIO_SWITCHES         TR("ПЕРЕМИКАЧІ", "ТЕСТ ПЕРЕМИКАЧІВ")
#define TR_MENU_RADIO_ANALOGS          TR("АНАЛОГ", "ТЕСТ АНАЛОГУ")
#define TR_MENU_RADIO_ANALOGS_CALIB    "АНАЛОГИ ВІДКАЛІБРОВАНІ"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "НЕОБРОБЛЕНІ АНАЛОГИ (5 Hz)"
#define TR_MENUCALIBRATION             "КАЛІБРУВАННЯ"
#define TR_MENU_FSWITCH                "КОРИСТУВАЦЬКІ ПЕРЕМИКАЧІ"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Трим => Субтрим"
#else
  #define TR_TRIMS2OFFSETS             "\006Трим => Субтрим"
#endif
#define TR_CHANNELS2FAILSAFE           "Канали=>АварРежим"
#define TR_CHANNEL2FAILSAFE            "Канал=>АварРежим"
#define TR_MENUMODELSEL                "ВИБІР МОДЕЛІ"
#define TR_MENU_MODEL_SETUP            TR("НАЛАШТУВАННЯ", "НАЛАШТУВАННЯ МОДЕЛІ")
#if defined(SURFACE_RADIO)
  #define TR_MENUFLIGHTMODES           "РЕЖИМИ ВОДІННЯ"
  #define TR_MENUFLIGHTMODE            "РЕЖИМ ВОДІННЯ"
#else
  #define TR_MENUFLIGHTMODES           "РЕЖИМИ ПОЛЬОТУ"
  #define TR_MENUFLIGHTMODE            "РЕЖИМ ПОЛЬОТУ"
#endif
#define TR_MENUHELISETUP               "ГЕЛІКОПТЕР"

#define TR_MENUINPUTS                  "ВХОДИ"
#define TR_MENULIMITS                  "ВИХОДИ"
#define TR_MENUCURVES                  "КРИВІ"
#define TR_MENUCURVE                   "КРИВА"
#define TR_MENULOGICALSWITCH           "ЛОГІЧНИЙ ПЕРЕМИКАЧ"
#define TR_MENULOGICALSWITCHES         "ЛОГІЧНІ ПЕРЕМ."
#define TR_MENUCUSTOMFUNC              "СПЕЦ. ФУНКЦІЇ"
#define TR_MENUCUSTOMSCRIPTS           "КОРИСТ. СКРИПТИ"
#define TR_MENUTELEMETRY               "ТЕЛЕМЕТРІЯ"
#define TR_MENUSTAT                    "СТАТ."
#define TR_MENUDEBUG                   "ВІДЛАГОДЖЕННЯ"
#define TR_MONITOR_CHANNELS1           "МОНІТОР КАН 1-8"
#define TR_MONITOR_CHANNELS2           "МОНІТОР КАН 9-16"
#define TR_MONITOR_CHANNELS3           "МОНІТОР КАН 17-24"
#define TR_MONITOR_CHANNELS4           "МОНІТОР КАН 25-32"
#define TR_MONITOR_SWITCHES            "МОНІТОР ЛОГІЧНИХ ПЕРЕМИКАЧІВ"
#define TR_MONITOR_OUTPUT_DESC         "Виходи"
#define TR_MONITOR_MIXER_DESC          "Мікшери"
#define TR_RECEIVER_NUM                TR("RxNum", "No. приймача")
#define TR_RECEIVER                    "Приймач"
#define TR_MULTI_RFTUNE                TR("Налаштув.частоти", "Точне налаштування Радіочастоти")
#define TR_MULTI_RFPOWER               "RF потужність"
#define TR_MULTI_WBUS                  "Вихід"
#define TR_MULTI_TELEMETRY             "Телеметрія"
#define TR_MULTI_VIDFREQ               TR("Від.Част.", "Відео частота")
#define TR_RF_POWER                    "RF Потужність"
#define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")		/* use english */
#define TR_MULTI_OPTION                TR("Опція", "Значення опції")
#define TR_MULTI_AUTOBIND              TR("Прив'язка до кан.", "Прив'язка до каналу")
#define TR_DISABLE_CH_MAP              TR("Викл. карту каналів", "Виключити карту каналів")
#define TR_DISABLE_TELEM               TR("Викл. телеметрії", "Виключити телеметрію")
#define TR_MULTI_LOWPOWER              TR("Низька потужн.", "Режим низької потужності")
#define TR_MULTI_LNA_DISABLE           "Вимкн. LNA"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")		/* use english */
#define TR_MODULE_TELEM_ON             TR("ON", "Увімкнено")
#define TR_DISABLE_INTERNAL            TR("Вимкн. внутр.", "Вимкнути внутрішній RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("Не Serial реж.", "Не в Serial режимі")
#define TR_MODULE_NO_INPUT             TR("Немає входу", "Немає Serial входу")
#define TR_MODULE_NO_TELEMETRY         TR3("Немає телеметрії", "Немає телеметрії", "Телеметрія не визначена")
#define TR_MODULE_WAITFORBIND          "Прив'язка до завантаж. протоколу"
#define TR_MODULE_BINDING              TR("Прив'язка...","Прив'язую")
#define TR_MODULE_UPGRADE_ALERT        TR3("Потрібне оновл.", "Потрібне оновлення модуля", "Потрібне\nОновлення модуля")
#define TR_MODULE_UPGRADE              TR("Рекоменд.Оновл.", "Рекомендоване оновлення модуля")
#define TR_REBIND                      "Потрібна переприв'язка"
#define TR_REG_OK                      "Регістрація успішна"
#define TR_BIND_OK                     "Прив'язка успішна"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Телем ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Телем OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Телем ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Телем OFF"
#define TR_PROTOCOL_INVALID            TR("Прот.невірн.", "Протокол невірний")
#define TR_MODULE_STATUS               TR("Статус", "Статус модуля")
#define TR_MODULE_SYNC                 TR("Синх", "Синхро протоколу")
#define TR_MULTI_SERVOFREQ             TR("Частота серво", "Частота оновлення серво")
#define TR_MULTI_MAX_THROW             TR("Max. Хід", "Увімкн. max. хід")
#define TR_MULTI_RFCHAN                TR("RF Канал", "Обери RF канал")
#define TR_AFHDS3_RX_FREQ              TR("RX част.", "RX частота")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Телем.", "Unicast/Телеметрія")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Факт.потужн.", "Фактична потужність")
#define TR_AFHDS3_POWER_SOURCE         TR("Джерело живл.", "Джерело живлення")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI")		/* use english */
#define TR_GPS_COORDS_FORMAT           TR("GPS Координати", "Формат координат")
#define TR_VARIO                       TR("Варіо", "Варіометр")
#define TR_PITCH_AT_ZERO               "Крок нуль"
#define TR_PITCH_AT_MAX                "Крок max"
#define TR_REPEAT_AT_ZERO              "Повтори нуль"
#define TR_BATT_CALIB                  TR("Калібр.акум", "Калібрування акумулятора")
#define TR_CURRENT_CALIB               "Калібровка струму"
#define TR_VOLTAGE                     TR("Напруга", "Джерело напруги")
#define TR_SELECT_MODEL                "Оберіть модель"
#define TR_MANAGE_MODELS               "КЕРУВАННЯ МОДЕЛЯМИ"
#define TR_MODELS                      "Моделі"
#define TR_SELECT_MODE                 "Оберіть режим"
#define TR_CREATE_MODEL                "Створіть модель"
#define TR_FAVORITE_LABEL              "Обрані"
#define TR_MODELS_MOVED                "Невикористані моделі переміщено до"
#define TR_NEW_MODEL                   "Нова Модель"
#define TR_INVALID_MODEL               "Невірна Модель"
#define TR_EDIT_LABELS                 "Редагувати Мітки"
#define TR_LABEL_MODEL                 "Мітка моделі"
#define TR_MOVE_UP                     "Перем. вгору"
#define TR_MOVE_DOWN                   "Перем. вниз"
#define TR_ENTER_LABEL                 "Введіть мітку"
#define TR_LABEL                       "Мітка"
#define TR_LABELS                      "Мітки"
#define TR_CURRENT_MODEL               "Поточна"
#define TR_ACTIVE                      "Активна"
#define TR_NEW                         "Нова"
#define TR_NEW_LABEL                   "Нова мітка"
#define TR_RENAME_LABEL                "Перейменувати мітку"
#define TR_DELETE_LABEL                "Видалити мітку"
#define TR_DUPLICATE_MODEL             "Дублювати мітку"
#define TR_COPY_MODEL                  "Копіювати модель"
#define TR_MOVE_MODEL                  "Перемістити модель"
#define TR_BACKUP_MODEL                "Резерв копія моделі"
#define TR_DELETE_MODEL                "Видалити модель"
#define TR_RESTORE_MODEL               "Відновити модель"
#define TR_DELETE_ERROR                "Видалити помилку"
#define TR_SDCARD_ERROR                TR("помилка SD", "помилка SD карти")
#define TR_SDCARD                      "SD Карта"
#define TR_NO_FILES_ON_SD              "Немає файлів на SD!"
#define TR_NO_SDCARD                   "SD карта відсутня"
#define TR_WAITING_FOR_RX              "Чекаємо на RX..."
#define TR_WAITING_FOR_TX              "Чекаємо на TX..."
#define TR_WAITING_FOR_MODULE          TR("Чекаємо на модуль", "Чекаємо на модуль...")
#define TR_NO_TOOLS                    "Немає інструментів"
#define TR_NORMAL                      "Нормально"
#define TR_NOT_INVERTED                "Не інв."
#define TR_NOT_CONNECTED               TR("Не з'єдн.", "Не з'єднано");
#define TR_CONNECTED                   "З'єднано"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH без телем.", "16CH без телеметрії")
#define TR_16CH_WITH_TELEMETRY         TR("16CH з телем.", "16CH з телметрією")
#define TR_8CH_WITH_TELEMETRY          TR("8CH з телем.", "8CH з телметрією")
#define TR_EXT_ANTENNA                 "Зовн. антена"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Оновити опції RX?"
#define TR_UPDATE_TX_OPTIONS           "Оновити опції TX?"
#define TR_MODULES_RX_VERSION          "Модулі / RX версії"
#define TR_SHOW_MIXER_MONITORS         "Показати монітори мікшеру"
#define TR_MENU_MODULES_RX_VERSION     "МОДУЛІ / RX ВЕРСІЇ"
#define TR_MENU_FIRM_OPTIONS           "FIRMWARE ОПЦІЇ"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Вісі/Потенц./Повзунки"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Вісі/Потенц./Повзунки"
#define TR_RF_PROTOCOL                 "RF протокол"
#define TR_MODULE_OPTIONS              "Опції модулю"
#define TR_POWER                       "Потужність"
#define TR_NO_TX_OPTIONS               "Немає опцій TX"
#define TR_RTC_BATT                    "RTC Batt"
#define TR_POWER_METER_EXT             "Power Meter (EXT)"
#define TR_POWER_METER_INT             "Power Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Спектр (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Спектр (INT)"
#define TR_SDCARD_FULL                 "SD карта повна"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\nЛоги і скріншоти вимкнені"
#else
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036Логи і скріншоти" LCDW_128_LINEBREAK "вимкнені"
#endif
#define TR_NEEDS_FILE                  "ПОТРІБЕН ФАЙЛ"
#define TR_EXT_MULTI_SPEC              "opentx-inv"				/*need to be clarified by context*/
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"				/*need to be clarified by context*/
#define TR_INCOMPATIBLE                "Несумісне"
#define TR_WARNING                     "ПОПЕРЕДЖ."
#define TR_STORAGE_WARNING             "СХОВИЩЕ"
#define TR_THROTTLE_UPPERCASE          "ГАЗ"
#define TR_ALARMSWARN                  "АВАРІЇ"
#define TR_SWITCHWARN                  TR("ПЕРЕМИК.", "УПРАВЛІННЯ")
#define TR_FAILSAFEWARN                "АВАРІЙНИЙ РЕЖИМ"
#define TR_TEST_WARNING                TR("ТЕСТУВАННЯ", "ТЕСТ ПОБУДОВАНО")
#define TR_TEST_NOTSAFE                "Використовувати тільки для тестування"
#define TR_WRONG_SDCARDVERSION         TR("Очікувана вер.: ", "Очікувана версія: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery розряджена"
#define TR_WARN_MULTI_LOWPOWER         "Режим низької потужності"
#define TR_BATTERY                     "BATTERY"
#define TR_WRONG_PCBREV                "Визначено невірну PCB"
#define TR_EMERGENCY_MODE              "НАДЗВИЧАЙНИЙ РЕЖИМ"
#define TR_NO_FAILSAFE                 "Аварійний режим не налаштовано"
#define TR_KEYSTUCK                    "Залипання кнопки"
#define TR_VOLUME                      "Загальна гучн."
#define TR_LCD                         "LCD"		/*use english*/
#define TR_BRIGHTNESS                  "Яскравість"
#define TR_CPU_TEMP                    "CPU темп."
#define TR_COPROC                      "Сопроцессор"
#define TR_COPROC_TEMP                 "MB темп."
#define TR_TTL_WARNING                 "Увага: Не перевищуйте 3.3V на пінах TX/RX!"
#define TR_FUNC                        "Функція"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Тривалість"
#define TR_DELAY                       "Затримка"
#define TR_SD_CARD                     "SD карта"
#define TR_SDHC_CARD                   "SD-HC карта"
#define TR_NO_SOUNDS_ON_SD             "Немає звуків на SD"
#define TR_NO_MODELS_ON_SD             "Немає моделей на SD"
#define TR_NO_BITMAPS_ON_SD            "Немає бітмапів на SD"
#define TR_NO_SCRIPTS_ON_SD            "Немає скриптів на SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Синтаксична помилка", "Ситаксична помилка скрипту")
#define TR_SCRIPT_PANIC                "Скрипт панікує"
#define TR_SCRIPT_KILLED               "Скрипт вбито"
#define TR_SCRIPT_ERROR                "Невідома помилка"
#define TR_PLAY_FILE                   "Грати"
#define TR_DELETE_FILE                 "Видалити"
#define TR_COPY_FILE                   "Копіювати"
#define TR_RENAME_FILE                 "Перейменувати"
#define TR_ASSIGN_BITMAP               "Призначити зображення"
#define TR_ASSIGN_SPLASH               "Заставка"
#define TR_EXECUTE_FILE                "Виконати"
#define TR_REMOVED                     " видалено"
#define TR_SD_INFO                     "Інформація"
#define TR_NA                          "Н/Д"
#define TR_HARDWARE                    "АПАРАТНЕ ЗАБЕЗПЕЧ."
#define TR_FORMATTING                  "Форматую..."
#define TR_TEMP_CALIB                  "Калібр. темп."
#define TR_TIME                        "Час"
#define TR_MAXBAUDRATE                 "Max швидкість"
#define TR_BAUDRATE                    "Швидкість"
#define TR_SAMPLE_MODE                 TR("Вибірка","Режим вибірки")
#define TR_SAMPLE_MODES                "Нормальний","OneBit"
#define TR_LOADING                     "Завантаження..."
#define TR_DELETE_THEME                "Видалити тему?"
#define TR_SAVE_THEME                  "Зберегти тему?"
#define TR_EDIT_COLOR                  "Редагувати колір"
#define TR_NO_THEME_IMAGE              "Немає зображення теми"
#define TR_BACKLIGHT_TIMER             "Час бездіяльності"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Швидкий вибір моделі"
  #define TR_LABELS_SELECT             "Вибір мітки"
  #define TR_LABELS_MATCH              "Співпад. мітки"
  #define TR_FAV_MATCH                 "Співпад. улюблен."
  #define TR_LABELS_SELECT_MODE        "Вибір багат.","Вибір одного"
  #define TR_LABELS_MATCH_MODE         "Співпад. усіх","Співпад. одного"
  #define TR_FAV_MATCH_MODE            "Обов'язк. співпад.","Необов'язк. співпад."
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "Обрати теку моделі"
#define TR_SELECT_TEMPLATE             "ОБЕРІТЬ ШАБЛОН МОДЕЛІ:"
#define TR_NO_TEMPLATES                "В цій теці не знайдено шаблонів моделей"
#define TR_SAVE_TEMPLATE               "Зберегти як шаблон"
#define TR_BLANK_MODEL                 "Порожня модель"
#define TR_BLANK_MODEL_INFO            "Створити порожню модель"
#define TR_FILE_EXISTS                 "ФАЙЛ ІСНУЄ"
#define TR_ASK_OVERWRITE               "Бажаєш перезаписати?"

#define TR_BLUETOOTH                   "Bluetooth"		/*use english*/
#define TR_BLUETOOTH_DISC              "Визначити"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Адр. призначення"
#define TR_BLUETOOTH_LOCAL_ADDR        "Місцева адр."
#define TR_BLUETOOTH_PIN_CODE          "PIN код"
#define TR_BLUETOOTH_NODEVICES         "Пристрій Не Знайдено"
#define TR_BLUETOOTH_SCANNING          "Сканування..."
#define TR_BLUETOOTH_BAUDRATE          "Швидкість BT"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Активовано"
#else
#define TR_BLUETOOTH_MODES             "---","Телеметрія","Тренер"
#endif

#define TR_SD_INFO_TITLE               "SD інфо"
#define TR_SD_SPEED                    "Швидкість:"
#define TR_SD_SECTORS                  "Сектори:"
#define TR_SD_SIZE                     "Розмір:"
#define TR_TYPE                        "Тип"
#define TR_GLOBAL_VARS                 "Глобальні змінні"
#define TR_GVARS                       "Г.ЗМ."
#define TR_GLOBAL_VAR                  "Глобальні змінні"
#define TR_MENU_GLOBAL_VARS              "ГЛОБАЛЬНІ ЗМІННІ"
#define TR_OWN                         "Свій"
#define TR_DATE                        "Дата"
#define TR_MONTHS                      { "Січ", "Лют", "Бер", "Кві", "Тра", "Чер", "Лип", "Сер", "Вер", "Жов", "Лис", "Гру" }
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("Режим енкодера","Режим обертального енкодера")
#define TR_CHANNELS_MONITOR            "МОНІТОР КАНАЛІВ"
#define TR_MIXERS_MONITOR              "МОНІТОР МІКШЕРІВ"
#define TR_PATH_TOO_LONG               "Шлях задовгий"
#define TR_VIEW_TEXT                   "Дивитися текст"
#define TR_FLASH_BOOTLOADER            "Прошити завантажувач"
#define TR_FLASH_DEVICE                TR("Прошити пристрій","Прошити пристрій")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Прошити посл.порт", "Прошити пристрій посл.порт.")
#define TR_FLASH_RECEIVER_OTA          "Пришити приймач OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Прошити RX зовн. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Прошити RX внутр. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Прошити FC зовн. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Прошити FC внутр. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Прошити BT модуль", "Прошити Bluetooth модуль")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Прошити пристрій управління живленням"
#define TR_DEVICE_NO_RESPONSE          TR("Пристрій не відповідає", "Пристрій не відповідає")
#define TR_DEVICE_FILE_ERROR           TR("Проблема з файлом пристрою", "Проблема з файлом пристрою")
#define TR_DEVICE_DATA_REFUSED         TR("Дані пристрою відхилено", "Дані пристрою відхилено")
#define TR_DEVICE_WRONG_REQUEST        TR("Проблема доступу до пристрою", "Проблема доступу до пристрою")
#define TR_DEVICE_FILE_REJECTED        TR("Файл пристрою відхилено", "Файл пристрою відхилено")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Невірний підпис файлу", "Невірний підпис файлу")
#define TR_CURRENT_VERSION             TR("Поточна версія: ", "Поточна версія: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Прошити внутр модуль", "Прошити внутр модуль")
#define TR_FLASH_INTERNAL_MULTI        TR("Прошити внутр. Multi", "Прошити внутрішній Multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Прошити зовн. модуль", "Прошити зовнішній модуль")
#define TR_FLASH_EXTERNAL_MULTI        TR("Прошити зовн. Multi", "Прошити зовнішній Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Прошити зовн. ELRS", "Прошити зовнішній ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Помилка оновлення FW", "Помилка оновлення Firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Прошивка успішна"
#define TR_WRITING                     "Запис..."
#define TR_CONFIRM_FORMAT              "Підтвердити формат?"
#define TR_INTERNALRF                  "Внутрійшній RF"
#define TR_INTERNAL_MODULE             TR("Внутр. модуль", "Внутрішній модуль")
#define TR_EXTERNAL_MODULE             TR("Зовн. модуль", "Зовнішній модуль")
#define TR_EDGETX_UPGRADE_REQUIRED     "Потрібне оновлення EdgeTX"
#define TR_TELEMETRY_DISABLED          "Телем. вимкнуто"
#define TR_MORE_OPTIONS_AVAILABLE      "Доступні додаткові опції"
#define TR_NO_MODULE_INFORMATION       "Інформація по модулю відсутня"
#define TR_EXTERNALRF                  "Зовнішній RF"
#define TR_FAILSAFE                    TR("Відмова", "Режим відмови")
#define TR_FAILSAFESET                 "НАЛАШТУВАННЯ АВАРІЙНОГО РЕЖИМУ"
#define TR_REG_ID                      "Рег. ID"
#define TR_OWNER_ID                    "ID Користувача"
#define TR_HOLD                        "Утримання"
#define TR_HOLD_UPPERCASE              "УТРИМАННЯ"
#define TR_NONE                        "Немає"
#define TR_NONE_UPPERCASE              "НЕМАЄ"
#define TR_MENUSENSOR                  "СЕНСОР"
#define TR_POWERMETER_PEAK             "Пікове"
#define TR_POWERMETER_POWER            "Потужність"
#define TR_POWERMETER_ATTN             "Атт"
#define TR_POWERMETER_FREQ             "Част."
#define TR_MENUTOOLS                   "ІНСТРУМЕНТИ"
#define TR_TURN_OFF_RECEIVER           "Вимкнути приймач"
#define TR_STOPPING                    "Зупинка..."
#define TR_MENU_SPECTRUM_ANALYSER      "АНАЛІЗАТОР СПЕКТРУ"
#define TR_MENU_POWER_METER            "ВИМІРЮВАЧ ПОТУЖНОСТІ"
#define TR_SENSOR                      "СЕНСОР"
#define TR_COUNTRY_CODE                "Код країни"
#define TR_USBMODE                     "Режим USB"
#define TR_JACK_MODE                   "Режим коннектора"
#define TR_VOICE_LANGUAGE              "Мова голосу"
#define TR_UNITS_SYSTEM                "Одиниці"
#define TR_UNITS_PPM                   "PPM Одиниці"
#define TR_EDIT                        "Редагувати"
#define TR_INSERT_BEFORE               "Вставка до"
#define TR_INSERT_AFTER                "Вставка після"
#define TR_COPY                        "Копіювати"
#define TR_MOVE                        "Перемістити"
#define TR_PASTE                       "Вставити"
#define TR_PASTE_AFTER                 "Вставити після"
#define TR_PASTE_BEFORE                "Вставити до"
#define TR_DELETE                      "Видалити"
#define TR_INSERT                      "Вставити"
#define TR_RESET_FLIGHT                "Скинути сесію"
#define TR_RESET_TIMER1                "Скинути таймер1"
#define TR_RESET_TIMER2                "Скинути таймер2"
#define TR_RESET_TIMER3                "Скинути таймер3"
#define TR_RESET_TELEMETRY             "Скинути телеметрію"
#define TR_STATISTICS                  "Статистика"
#define TR_ABOUT_US                    "Про..."
#define TR_USB_JOYSTICK                "USB Джойстик (HID)"
#define TR_USB_MASS_STORAGE            "USB Сховище (SD)"
#define TR_USB_SERIAL                  "USB Посл. порт (VCP)"
#define TR_SETUP_SCREENS               "Налаштування екранів"
#define TR_MONITOR_SCREENS             "Монітори"
#define TR_AND_SWITCH                  "AND switch"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Калібровані аналоги"
#define TR_ANADIAGS_FILTRAWDEV         "Фільтровані сирі аналоги з відхиленням"
#define TR_ANADIAGS_UNFILTRAW          "Неформатовані сирі аналоги"
#define TR_ANADIAGS_MINMAX             "Min., max. і діапазон"
#define TR_ANADIAGS_MOVE               "Перемістити аналоги до екстремумів!"
#define TR_SPEAKER                     "Гучномовець"
#define TR_BUZZER                      "Дзвоник"
#define TR_BYTES                       "байти"
#define TR_MODULE_BIND                 BUTTON(TR("Прив.", "Прив'язати"))
#define TR_MODULE_UNBIND               BUTTON("Відв'язати")
#define TR_POWERMETER_ATTN_NEEDED     "Потрібен атенюатор"
#define TR_PXX2_SELECT_RX              "Оберіть RX"
#define TR_PXX2_DEFAULT                "<замовч>"
#define TR_BT_SELECT_DEVICE            "Оберіть пристрій"
#define TR_DISCOVER                    "Виявити"
#define TR_BUTTON_INIT                 BUTTON("Запуск")
#define TR_WAITING                     "Очікуємо..."
#define TR_RECEIVER_DELETE             "Видалити приймач?"
#define TR_RECEIVER_RESET              "Скинути приймач?"
#define TR_SHARE                       "Поділитися"
#define TR_BIND                        "Прив'язати"
#define TR_REGISTER                    TR("Регі.", "Регістр")
#define TR_MODULE_RANGE                BUTTON(TR("Діап.", "Діапазон"))
#define TR_RANGE_TEST                  "Тест діапазону"
#define TR_RECEIVER_OPTIONS            TR("ОПЦІЇ ПРМ.", "ОПЦІЇ ПРИЙМАЧА")
#define TR_RESET_BTN                   BUTTON("Скин.")
#define TR_DEBUG                       "Відлагод."
#define TR_KEYS_BTN                    BUTTON("Кнопк")
#define TR_ANALOGS_BTN                 BUTTON(TR("Анл.", "Аналоги"))
#define TR_FS_BTN                      BUTTON(TR("Користув.перемик.", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Сенсорний екран не знайдено"
#define TR_TOUCH_EXIT                  "Натисніть на екран щоб вийти"
#define TR_SET                         BUTTON("Встановл.")
#define TR_TRAINER                     "Тренер"
#define TR_CHANS                       "Канали"
#define TR_ANTENNAPROBLEM              "Помилка TX антени!"
#define TR_MODELIDUSED                 "ID використано в:"
#define TR_MODELIDUNIQUE               "ID унікальний"
#define TR_MODULE                      "Модуль"
#define TR_RX_NAME                     "Ім'я Rx"
#define TR_TELEMETRY_TYPE              TR("Тип", "Тип телеметрії")
#define TR_TELEMETRY_SENSORS           "Сенсори"
#define TR_VALUE                       "Значення"
#define TR_PERIOD                      "Період"
#define TR_INTERVAL                    "Інтервал"
#define TR_REPEAT                      "Повтор"
#define TR_ENABLE                      "Активувати"
#define TR_DISABLE                     "Деактивувати"
#define TR_TOPLCDTIMER                 "Верхній таймер LCD"
#define TR_UNIT                        "Одиниця"
#define TR_TELEMETRY_NEWSENSOR         "Додати новий"
#define TR_CHANNELRANGE                TR("Діап. каналів", "Діапазон каналів")
#define TR_ANTENNACONFIRM1             "ЗОВН. АНТЕНА"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "Внутрішній","Запит","На модель","Внутрійшній + Зовнішній"
#else
  #define TR_ANTENNA_MODES             "Внутрішній","Запит","На модель","Зовнішній"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Вик. внурт. антену", "Використай внутрішню антену")
#define TR_USE_EXTERNAL_ANTENNA        TR("Вик. зовн. антену", "Використай зовнішню антену")
#define TR_ANTENNACONFIRM2             TR("Перевір антену", "Впевнись, що антену встановлено!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Потрібно не FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Потрібно FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Потрібно EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "сертифікований firmware"
#define TR_LOWALARM                    "Некритична аварія"
#define TR_CRITICALALARM               "Критична аварія"
#define TR_DISABLE_ALARM               TR("Вимкнути аварії", "Вимкнути аварії телеметрії")
#define TR_POPUP                       "Всплив."
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Передвстановлення..."
#define TR_PRESET                      "Передвстановлення"
#define TR_MIRROR                      "Дзеркало"
#define TR_CLEAR                       "Очистити"
#define TR_RESET                       "Скидання"
#define TR_RESET_SUBMENU               "Скидання..."
#define TR_COUNT                       "Кількість"
#define TR_PT                          "шт"
#define TR_PTS                         "штк"
#define TR_SMOOTH                      "Плавно"
#define TR_COPY_STICKS_TO_OFS          TR("Коп.вісі->обріз.", "Копіювати вісі до обрізання")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Коп min/max до всіх",  "Копіювати min/max/center до всіх виходів")
#define TR_COPY_TRIMS_TO_OFS           TR("Коп трим->субтрим", "Копіювати трим до субтрим")
#define TR_INCDEC                      "Збільш/Зменш"
#define TR_GLOBALVAR                   "Глобальна змінна"
#define TR_MIXSOURCE                   "Джерело (%)"
#define TR_MIXSOURCERAW                "Джерело (Значення)"
#define TR_CONSTANT                    "Постійна"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "OFF","ON","AUTO"
#define TR_PREFLIGHT                   "Стартові перевірки"
#define TR_CHECKLIST                   TR("Чекліст", "Чекліст Дисплею")
#define TR_CHECKLIST_INTERACTIVE       TR3("Інт. спис.", "Інтеракт. список", "Інтерактивний список")
#define TR_AUX_SERIAL_MODE             "Послідовний порт"
#define TR_AUX2_SERIAL_MODE            "Послідовний порт 2"
#define TR_AUX_SERIAL_PORT_POWER       "Порт живлення"
#define TR_SCRIPT                      "Скрипт"
#define TR_INPUTS                      "Входи"
#define TR_OUTPUTS                     "Виходи"
#define STR_EEBACKUP                   "Резерв EEPROM"
#define STR_FACTORYRESET               "Скидання на заводські"
#define TR_CONFIRMRESET                TR("Стерти все", "Стерти налаштування всіх моделей?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Забагато Lua скриптів!"
#define TR_SPORT_UPDATE_POWER_MODE     "Режим живлення SP"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "Екрани Телеметрії Відсутні"
#define TR_TOUCH_PANEL                 "Сенсорна панель:"
#define TR_FILE_SIZE                   "Розмір файлу"
#define TR_FILE_OPEN                   "Відкрити попри все?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Ім'я"
#define TR_PHASES_HEADERS_SW           "Перемикач"
#define TR_PHASES_HEADERS_RUD_TRIM     "Тримування керма"
#define TR_PHASES_HEADERS_ELE_TRIM     "Трим висоти"
#define TR_PHASES_HEADERS_THT_TRIM     "Трим газу"
#define TR_PHASES_HEADERS_AIL_TRIM     "Трим елеронів"
#define TR_PHASES_HEADERS_CH5_TRIM     "Трим 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Трим 6"
#define TR_PHASES_HEADERS_FAD_IN       "Плавно In"
#define TR_PHASES_HEADERS_FAD_OUT      "Плавно Out"

#define TR_LIMITS_HEADERS_NAME         "Ім'я"
#define TR_LIMITS_HEADERS_SUBTRIM      "Субтрим"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Напрямок"
#define TR_LIMITS_HEADERS_CURVE        "Крива"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Центр"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Режим субтрим"
#define TR_INVERTED                    "Інверс."


#define TR_LSW_DESCRIPTIONS            { "Порівняння типу функції", "Перша змінна", "Друга змінна або постійна", "Друга змінна або постійна", "Додаткова умова для активації лінії", "Мінімальна тривалість ON для логічного перемикача", "Мінімальна тривалість TRUE для перемикача, що стає ON" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "Перший канал"
  #define TR_FILL_BACKGROUND           "Заповнити фон?"
  #define TR_BG_COLOR                  "Колір фону"
  #define TR_SLIDERS_TRIMS             "Повзунки+трими"
  #define TR_SLIDERS                   "Повзунки"
  #define TR_FLIGHT_MODE               "Режим польоту"
  #define TR_INVALID_FILE              "Невірний файл"
  #define TR_TIMER_SOURCE              "Джерело таймеру"
  #define TR_SIZE                      "Розмір"
  #define TR_SHADOW                    "Тінь"
  #define TR_ALIGNMENT                 "Вирівнювання"
  #define TR_ALIGN_LABEL               "Вирівняти мітку"
  #define TR_ALIGN_VALUE               "Вирівняти значення"
  #define TR_ALIGN_OPTS                { "Ліво", "Центр", "Право" }
  #define TR_TEXT                      "Текст"
  #define TR_COLOR                     "Колір"
  #define TR_MAIN_VIEW_X               "Головний вид XX"
  #define TR_PANEL1_BACKGROUND         "Панель1 фон"
  #define TR_PANEL2_BACKGROUND         "Панель2 фон"
  #define TR_WIDGET_GAUGE              "Шкала"
  #define TR_WIDGET_MODELBMP           "Інфо про модель"
  #define TR_WIDGET_OUTPUTS            "Виходи"
  #define TR_WIDGET_TEXT               "Текст"
  #define TR_WIDGET_TIMER              "Таймер"
  #define TR_WIDGET_VALUE              "Значення"
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
  #define TR_BL_SELECT_KEY            "[ENT] to select file"
  #define TR_BL_FLASH_KEY             "Hold [ENT] long to flash"
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
#elif defined(PCBPL18)
  // Bootloader PL18/NB4+ specific - Ascii only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #if defined(RADIO_NB4P)
    #define TR_BL_SELECT_KEY          "[SW1A] to select file"
    #define TR_BL_FLASH_KEY           "Hold [SW1A] long to flash"
    #define TR_BL_ERASE_KEY           "Hold [SW1A] long to erase"
    #define TR_BL_EXIT_KEY            "[SW1B] to exit"
  #else
    #define TR_BL_SELECT_KEY          "[TR4 Dn] to select file"
    #define TR_BL_FLASH_KEY           "Hold [TR4 Dn] long to flash"
    #define TR_BL_ERASE_KEY           "Hold [TR4 Dn] long to erase"
    #define TR_BL_EXIT_KEY            "[TR4 Up] to exit"
  #endif
#endif

// About screen
#define TR_ABOUTUS                     TR(" ПРО... ", "ПРО...")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Гучн. сигналів"
#define TR_WAV_VOLUME                  "Гучність Wav"
#define TR_BG_VOLUME                   TR("Фонова гучн.", "Фонова гучність")

#define TR_TOP_BAR                     "Верхня панель"
#define TR_FLASH_ERASE                 "Стирання Flash..."
#define TR_FLASH_WRITE                 "Запис Flash..."
#define TR_OTA_UPDATE                  "Оновлення OTA..."
#define TR_MODULE_RESET                "Скидання модулю..."
#define TR_UNKNOWN_RX                  "Невідомий RX"
#define TR_UNSUPPORTED_RX              "Несумісний RX"
#define TR_OTA_UPDATE_ERROR            "Помилка оновлення OTA"
#define TR_DEVICE_RESET                "Скидання пристрою..."
#define TR_ALTITUDE                    "Висота"
#define TR_SCALE                       "Масштаб"
#define TR_VIEW_CHANNELS               "Перегляд каналів"
#define TR_VIEW_NOTES                  "Перегляд нотаток"
#define TR_MODEL_SELECT                "Вибір моделі"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Точність"
#define TR_RATIO                       "Коефіцієнт"
#define TR_FORMULA                     "Формула"
#define TR_CELLINDEX                   "Номер комірки"
#define TR_LOGS                        "Логи"
#define TR_OPTIONS                     "Опції"
#define TR_FIRMWARE_OPTIONS            "Опції Firmware"

#define TR_ALTSENSOR                   "Сенсор висоти"
#define TR_CELLSENSOR                  "Сенсор комірки"
#define TR_GPSSENSOR                   "GPS сенсор"
#define TR_CURRENTSENSOR               "Сенсор"
#define TR_AUTOOFFSET                  "Авто зсув"
#define TR_ONLYPOSITIVE                "Позитив"
#define TR_FILTER                      "Фільтр"
#define TR_TELEMETRYFULL               TR("Всі слоти повні!", "Всі слоти телеметрії повні!")
#define TR_IGNORE_INSTANCE             TR("Немає інстанса.", "Ігнорувати інстанси")
#define TR_SHOW_INSTANCE_ID            "Показати інстанс ID"
#define TR_DISCOVER_SENSORS            "Пошук нового"
#define TR_STOP_DISCOVER_SENSORS       "Stop"
#define TR_DELETE_ALL_SENSORS          "Видалити всі"
#define TR_CONFIRMDELETE               "Дійсно " LCDW_128_LINEBREAK "видалити всі ?"
#define TR_SELECT_WIDGET               "Обрати віджет"
#define TR_WIDGET_FULLSCREEN           "На весь екран"
#define TR_REMOVE_WIDGET               "Видалити віджет"
#define TR_WIDGET_SETTINGS             "Опції віджету"
#define TR_REMOVE_SCREEN               "Видалити екран"
#define TR_SETUP_WIDGETS               "Налаштування віджетів"
#define TR_USER_INTERFACE              "Інтерфейс користувача"
#define TR_THEME                       "Тема"
#define TR_SETUP                       "Встановлення"
#define TR_LAYOUT                      "Шар"
#define TR_ADD_MAIN_VIEW               "Додати головний вид"
#define TR_TEXT_COLOR                  "Колір тексту"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Входи"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua скрипти"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Вісі"
#define TR_MENU_POTS                   STR_CHAR_POT "Потенц."
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "Циклічно"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Трими"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Перемикачі"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Логічні перемикачі"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Тренер"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Канали"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "ГлобЗмін"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Телеметрія"
#define TR_MENU_DISPLAY                "ДИСПЛЕЙ"
#define TR_MENU_OTHER                  "Інше"
#define TR_MENU_INVERT                 "Інверт"
#define TR_AUDIO_MUTE                  TR("Аудіо стоп","Тихо якщо немає звуку")
#define TR_JITTER_FILTER               "Фільтр ADC"
#define TR_DEAD_ZONE                   "Мертва зона"
#define TR_RTC_CHECK                   TR("Перевір RTC", "Перевір напругу RTC")
#define TR_AUTH_FAILURE                "Помилка авторизації"
#define TR_RACING_MODE                 "Режим гонки"

#define TR_USE_THEME_COLOR              "Використ. колір теми"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Додати всі трими до субтримів"
#if !PORTRAIT_LCD
  #define TR_OPEN_CHANNEL_MONITORS        "Відкрити монітори каналів"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Відкр. мон.кан."
#endif
#define TR_DUPLICATE                    "Дублювати"
#define TR_ACTIVATE                     "Встановити активним"
#define TR_RED                          "Червоний"
#define TR_BLUE                         "Синій"
#define TR_GREEN                        "Зелений"
#define TR_COLOR_PICKER                 "Обирач кольору"
#define TR_EDIT_THEME_DETAILS           "Редагувати деталі теми"
#define TR_THEME_COLOR_DEFAULT         "ЗАМОВЧ"
#define TR_THEME_COLOR_PRIMARY1        "ОСНОВНИЙ1"
#define TR_THEME_COLOR_PRIMARY2        "ОСНОВНИЙ2"
#define TR_THEME_COLOR_PRIMARY3        "ОСНОВНИЙ3"
#define TR_THEME_COLOR_SECONDARY1      "ВТОРИННИЙ1"
#define TR_THEME_COLOR_SECONDARY2      "ВТОРИННИЙ2"
#define TR_THEME_COLOR_SECONDARY3      "ВТОРИННИЙ3"
#define TR_THEME_COLOR_FOCUS           "ФОКУС"
#define TR_THEME_COLOR_EDIT            "РЕДАГУВАТИ"
#define TR_THEME_COLOR_ACTIVE          "АКТИВНИЙ"
#define TR_THEME_COLOR_WARNING         "WARNING"
#define TR_THEME_COLOR_DISABLED        "ДЕАКТИВОВАНИЙ"
#define TR_THEME_COLOR_CUSTOM          "КОРИСТУВАЦЬКИЙ"
#define TR_THEME_CHECKBOX              "Checkbox"
#define TR_THEME_ACTIVE                "Активний"
#define TR_THEME_REGULAR               "Регулярний"
#define TR_THEME_WARNING               "Warning"
#define TR_THEME_DISABLED              "Деактивований"
#define TR_THEME_EDIT                  "Редагувати"
#define TR_THEME_FOCUS                 "Фокус"
#define TR_AUTHOR                      "Автор"
#define TR_DESCRIPTION                 "Опис"
#define TR_SAVE                        "Зберегти"
#define TR_CANCEL                      "Відміна"
#define TR_EDIT_THEME                  "РЕДАГУВАТИ ТЕМУ"
#define TR_DETAILS                     "Деталі"
#define TR_THEME_EDITOR                "ТЕМИ"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Обрати\nМодель"
#define TR_MAIN_MENU_MANAGE_MODELS      "Керувати\nМоделями"
#define TR_MAIN_MENU_MODEL_NOTES        "Моделі\nНотатки"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Канал\nМонітор"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Модель\nНалаштування"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Радіо\nНалаштування"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Екрани\nНалаштування"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Скидання\nТелеметрії"
#define TR_MAIN_MENU_STATISTICS         "Статистика"
#define TR_MAIN_MENU_ABOUT_EDGETX       "Про...\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "Англійська"
#define TR_VOICE_CHINESE                "Китайська"
#define TR_VOICE_CZECH                  "Чеська"
#define TR_VOICE_DANISH                 "Данська"
#define TR_VOICE_DEUTSCH                "Німецька"
#define TR_VOICE_DUTCH                  "Голандська"
#define TR_VOICE_ESPANOL                "Іспанська"
#define TR_VOICE_FRANCAIS               "Французська"
#define TR_VOICE_HUNGARIAN              "Угорська"
#define TR_VOICE_ITALIANO               "Італійська"
#define TR_VOICE_POLISH                 "Польська"
#define TR_VOICE_PORTUGUES              "Португальська"
#define TR_VOICE_RUSSIAN                "Pосійська"
#define TR_VOICE_SLOVAK                 "Словацька"
#define TR_VOICE_SWEDISH                "Шведська"
#define TR_VOICE_TAIWANESE              "Тайська"
#define TR_VOICE_JAPANESE               "Японська"
#define TR_VOICE_HEBREW                 "Іврит"
#define TR_VOICE_UKRAINIAN              "Українська"

#define TR_USBJOYSTICK_LABEL           "USB Джойстик"
#define TR_USBJOYSTICK_EXTMODE         "Режим"
#define TR_VUSBJOYSTICK_EXTMODE        "Класичний","Просунутий"
#define TR_USBJOYSTICK_SETTINGS        "Налаштування каналу"
#define TR_USBJOYSTICK_IF_MODE         TR("Режим ІФ","Режим інтерфейсу")
#define TR_VUSBJOYSTICK_IF_MODE        "Джойстик", "Геймпад", "MultiAxis"
#define TR_USBJOYSTICK_CH_MODE         "Режим"
#define TR_VUSBJOYSTICK_CH_MODE        "Немає","Btn","Axis","Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Режим кнопок"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "Нормальн","Імпульс","SWEmu","Дельта","Компаньон"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Норм","Нормальний"),TR("Імп","Імпульс"),TR("SWEm","SWEmul"),TR("Дельта","Дельта"),TR("КПН","Компаньйон")
#define TR_USBJOYSTICK_CH_SWPOS        "Позиції"
#define TR_VUSBJOYSTICK_CH_SWPOS       "Push","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Вісь"
#define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","rotX","rotY","rotZ","Повзунок","Dial","Колесо"
#define TR_USBJOYSTICK_CH_SIM          "Сим. вісі"
#define TR_VUSBJOYSTICK_CH_SIM         "Ail","Ele","Rud","Thr","Acc","Brk","Steer","Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Інверсія"
#define TR_USBJOYSTICK_CH_BTNNUM       "Номер Кнопки"
#define TR_USBJOYSTICK_BTN_COLLISION   "!Конфлікт номерів кнопок!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Конфлікт осей!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Круговий виріз", "Круговий виріз")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "Немає","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Застосувати зміни"

#define TR_DIGITAL_SERVO          "Серво 333HZ"
#define TR_ANALOG_SERVO           "Серво 50HZ"
#define TR_SIGNAL_OUTPUT          "Вихід сигналу"
#define TR_SERIAL_BUS             "Послід. шина"
#define TR_SYNC                   "Синхр"

#define TR_ENABLED_FEATURES       "Активовані функції"
#define TR_RADIO_MENU_TABS        "Вкладки меню радіо"
#define TR_MODEL_MENU_TABS        "Вкладки меню моделей"

#define TR_SELECT_MENU_ALL        "Все"
#define TR_SELECT_MENU_CLR        "Очистити"
#define TR_SELECT_MENU_INV        "Інвертувати"

#define TR_SORT_ORDERS            "Назва A-Z","Назва Z-A","Найменш використ.","Найбільш використ."
#define TR_SORT_MODELS_BY         "Сортувати моделі за"
#define TR_CREATE_NEW             "Створити"

#define TR_MIX_SLOW_PREC          TR("Точн. уповільн.", "Точність уповільнення")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "A theme directory with the same name already exists."

#define TR_DATE_TIME_WIDGET       "Дата & Час"
#define TR_RADIO_INFO_WIDGET      "Інформація про пульт"
#define TR_LOW_BATT_COLOR         "Розряджена батарея"
#define TR_MID_BATT_COLOR         "Середній заряд батареї"
#define TR_HIGH_BATT_COLOR        "Заряджена батарея"

#define TR_WIDGET_SIZE            "Розмір віджета"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
