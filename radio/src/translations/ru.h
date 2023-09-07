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

// RU translations author: kobakirill <blokhin.kirill@yandex.ru>

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

#define TR_MINUTE_SINGULAR            "минута"
#define TR_MINUTE_PLURAL1             "минуты"
#define TR_MINUTE_PLURAL2             "минуты"

#define TR_OFFON                       "ВЫК","ВКЛ"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Тихий","Тревога","NoKey","Все"
#define TR_VBLMODE                     "ВЫК","Keys",TR("Ctrl","Controls"),"Both","ON"
#define TR_TRNMODE                     "OFF",TR("+=","Add"),TR(":=","Replace")
#define TR_TRNCHN                      "КН1","КН2","КН3","КН4"

#define TR_AUX_SERIAL_MODES            "OFF","Telem Mirror","Telemetry In","SBUS Trainer","LUA","CLI","GPS","Debug","SpaceMouse","External module"
#define TR_SWTYPES                     "None","Toggle","2POS","3POS"
#define TR_POTTYPES                    "None",TR("Pot w. det","Pot with detent"),TR("Multipos","Multipos Switch"),"Pot", "Slider"
#define TR_VPERSISTENT                 "OFF","Flight","Manual Reset"
#define TR_COUNTRY_CODES               TR("US","Америка"),TR("JP","Япония"),TR("EU","Европа")
#define TR_USBMODES                    "Ask",TR("Joyst","Joystick"),TR("SDCard","Storage"),"Serial"
#define TR_JACK_MODES                  "Ask","Audio","Trainer"
#define TR_TELEMETRY_PROTOCOLS         "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_CUSTOM                "Custom"
#define TR_VTRIMINC                    TR("Expo","Exponential"),TR("ExFine","Extra Fine"),"Fine","Medium","Coarse"
#define TR_VDISPLAYTRIMS               "No","Change","Yes"
#define TR_VBEEPCOUNTDOWN              "Тихий","Бип","Звук","Вибро",TR2_2("Б & В","Бип & Вибро"),TR2_2("З & В","Звук & Вибро")
#define TR_COUNTDOWNVALUES             "5с","10с","20с","30с"
#define TR_VVARIOCENTER                "Тон","тихо"
#define TR_CURVE_TYPES                 "Стандарт","Выбор"

#define TR_ADCFILTERVALUES             "Global","ВЫК","ВКЛ"

#define TR_VCURVETYPE                  "Diff","Expo","Func","Cstm"
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Add","Multiply","Replace"
#define TR_VMLTPX2                     "+=","*=",":="

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
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Таймер"
#define TR_SF_INST_TRIM                "Inst. Trim"
#define TR_SF_RESET                    "Сброс"
#define TR_SF_SET_TIMER                "Установка"
#define TR_SF_VOLUME                   "Громкость"
#define TR_SF_FAILSAFE                 "SetFailsafe"
#define TR_SF_RANGE_CHECK              "RangeCheck"
#define TR_SF_MOD_BIND                 "ModuleBind"
 
#define TR_SOUND                       "Играть Звук"
#define TR_PLAY_TRACK                  "Играть Трек"
#define TR_PLAY_VALUE                  TR("Play Val","Play Value")
#define TR_SF_HAPTIC                   "Вибро"
#define TR_SF_PLAY_SCRIPT              "Lua Скрипт"
#define TR_SF_BG_MUSIC                 "Муз. фон"
#define TR_SF_BG_MUSIC_PAUSE           "Муз. ||"
#define TR_SF_LOGS                     "SD Логи"
#define TR_ADJUST_GVAR                 "Настройка"
#define TR_SF_BACKLIGHT                "Подсветка"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "Тест"
#define TR_SF_SAFETY                   TR("Безп.","Безопасность")

#if LCD_W >= 212
  #define TR_SF_SAFETY                 "Override"
#else
  #define TR_SF_SAFETY                 "Overr."
#endif

#define TR_SF_SCREENSHOT               "Скриншот"
#define TR_SF_RACING_MODE              "Режим гонка"
#define TR_SF_DISABLE_TOUCH            "Сенсор ВЫК"
#define TR_SF_SET_SCREEN               "Выбрать главный экран"
#define TR_SF_RESERVE                  "[резев]"

#define TR_FSW_RESET_TELEM             TR("Телем", "Телеметрия")
#define TR_FSW_RESET_TIMERS            "Тмр1","Трм2","Трм3"

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("All","Flight"),TR_FSW_RESET_TELEM

#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "млч"
#define LENGTH_UNIT_METR               "м"
#define SPEED_UNIT_METR                "кмч"

#define TR_VUNITSSYSTEM                "Metric",TR("Imper.","Imperial")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "None","Nums","Bars","Скрипт"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES                 "Rud", "Ele", "Thr", "Ail"
#define TR_SURFACE_NAMES               "ST", "TH"

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


#define TR_SRC_BATT                    "Batt"
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES                   "ВЫКЛ","ВКЛ","Strt","THs","TH%","THt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
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
#define TR_VCELLINDEX                  "Lowest","1","2","3","4","5","6","7","8","Highest","Delta"
#define TR_TEXT_SIZE                   "Standard","Tiny","Small","Mid","Double"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center only)","= (symmetrical)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "СТД"
#else
#define TR_FONT_SIZES                  "СТД","Жирный","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ВВОД]"
#elif defined(PCBNV14)
  #define TR_ENTER                     "[NEXT]"
#else
  #define TR_ENTER                     "[МЕНЮ]"
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

#define TR_MENUWHENDONE                CENTER "\006" TR_ENTER " > ДАЛЕЕ"
#define TR_FREE                        "Свободн."
#define TR_YES                         "Да"
#define TR_NO                          "Нет"
#define TR_DELETEMODEL                 "Удалить Модель"
#define TR_COPYINGMODEL                "Коприровать Модель"
#define TR_MOVINGMODEL                 "Перенести Модель"
#define TR_LOADINGMODEL                "Загрузить Модель"
#define TR_UNLABELEDMODEL              "Без Названия"
#define TR_NAME                        "Название"
#define TR_MODELNAME                   "Название Модели"
#define TR_PHASENAME                   "Название Фазы"
#define TR_MIXNAME                     "Название Mix"
#define TR_INPUTNAME                   TR("Ввод", "Ввод Имени")
#define TR_EXPONAME                    TR("Название", "Линия Названия")
#define TR_BITMAP                      "Изображени"
#define TR_NO_PICTURE                  "Без изображения"
#define TR_TIMER                       TR("Таймер", "Таймер ")
#define TR_START                       "Старт"
#define TR_ELIMITS                     TR("Р.Лимит", "Расширенный Лимит")
#define TR_ETRIMS                      TR("Р.Трим", "Расширеные Тримы")
#define TR_TRIMINC                     "Шаг Трима"
#define TR_DISPLAY_TRIMS               TR("Пок. Тримы", "Показывать Тримы")
#define TR_TTRACE                      TR("T-Source", INDENT "Source")
#define TR_TTRIM                       TR("T-Trim-Idle", INDENT "Trim idle only")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", INDENT "Trim switch")
#define TR_BEEPCTR                     TR("Цент. Гудок", "Центр Гудок")
#define TR_USE_GLOBAL_FUNCS            TR("Глоб. Функции", "Используйте Глобальные Функции")
#define TR_PROTOCOL                    TR("Прото", "Протокол")
  #define TR_PPMFRAME                  INDENT "PPM frame"
  #define TR_REFRESHRATE               TR(INDENT "Refresh", INDENT "Refresh rate")
  #define STR_WARN_BATTVOLTAGE         TR(INDENT "Output is VBAT: ", INDENT "Warning: output level is VBAT: ")
#define TR_WARN_5VOLTS                 "Warning: output level is 5 volts"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Switch"
#define TR_FUNCTION_SWITCHES           "Customizable Switches"
#define TR_SF_SWITCH                   "Триггер"
#define TR_TRIMS                       "Тримы"
#define TR_FADEIN                      "Fade in"
#define TR_FADEOUT                     "Fade out"
#define TR_DEFAULT                     "(default)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Проверка Тримов"
#else
  #define TR_CHECKTRIMS                CENTER "\006Check\012trims"
#endif
#define OFS_CHECKTRIMS                 CENTER_OFS+(9*FW)
#define TR_SWASHTYPE                   "Swash Type"
#define TR_COLLECTIVE                  TR("Collective", "Coll. pitch source")
#define TR_AILERON                     TR("Lateral cyc.", "Lateral cyc. source")
#define TR_ELEVATOR                    TR("Long. cyc.", "Long. cyc. source")
#define TR_SWASHRING                   "Swash Ring"
#define TR_MODE                        "Mode"
#if LCD_W > LCD_H
  #define TR_LEFT_STICK                "Левый"
#else
  #define TR_LEFT_STICK                "Левый"
#endif
#define TR_SUBTYPE                     INDENT "Subtype"
#define TR_NOFREEEXPO                  "No free expo!"
#define TR_NOFREEMIXER                 "No free mixer!"
#define TR_SOURCE                       "Source"
#define TR_WEIGHT                      "Weight"
#define TR_SIDE                        "Side"
#define TR_OFFSET                       "Offset"
#define TR_TRIM                        "Трим"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Curve"
#define TR_FLMODE                      TR("Mode", "Modes")
#define TR_MIXWARNING                  "Warning"
#define TR_OFF                         "Вык"
#define TR_ANTENNA                     "Антена"
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
#define TR_RANGE                       INDENT "Диапазон"
#define TR_CENTER                      INDENT "Центр"
#define TR_ALARM                       "Тревога"
#define TR_BLADES                      "Blades/Poles"
#define TR_SCREEN                      "Экран\001"
#define TR_SOUND_LABEL                 "Звук"
#define TR_LENGTH                      "Длина"
#define TR_BEEP_LENGTH                 "Длина Гудка"
#define TR_BEEP_PITCH                  "Высота Гудка"
#define TR_HAPTIC_LABEL                "Haptic"
#define TR_STRENGTH                    "Strength"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Offset"
#define TR_IMU_MAX                     "Макс"
#define TR_CONTRAST                    "Контраст"
#define TR_ALARMS_LABEL                "Тревоги"
#define TR_BATTERY_RANGE               TR("Диапазон АКБ", "Диапазон Измерения АКБ")
#define TR_BATTERYCHARGING             "Зарядка..."
#define TR_BATTERYFULL                 "АКБ Заряжена"
#define TR_BATTERYNONE                 "АКБ Отсутствует"
#define TR_BATTERYWARNING              "АКБ Разряжена"
#define TR_INACTIVITYALARM             "Отключено"
#define TR_MEMORYWARNING               "Память Заканчивается"
#define TR_ALARMWARNING                "Звук ВЫКЛ"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI shutdown", "Проверьте RSSI При Выключении")
#define TR_MODEL_STILL_POWERED         "Модель Еще Работает"
#define TR_USB_STILL_CONNECTED         "USB Еще Подключен"
#define TR_MODEL_SHUTDOWN              "Отключение?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Нажмите enter Для Подтверждения"
#define TR_THROTTLE_LABEL              "Дроссель"
#define TR_THROTTLE_START              "Дроссель Старт"
#define TR_THROTTLEREVERSE             TR("Д-Реверс", INDENT "Реверс")
#define TR_MINUTEBEEP                  TR("Минут", "Звонок минуты")
#define TR_BEEPCOUNTDOWN               INDENT "Обратный отсчет"
#define TR_PERSISTENT                  TR(INDENT "Persist.", INDENT "Persistent")
#define TR_BACKLIGHT_LABEL             "Подсветка"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Статус"
#define TR_BLDELAY                     INDENT "Продолжительность"
#define TR_BLONBRIGHTNESS              INDENT "Подсветка ВКЛ"
#define TR_BLOFFBRIGHTNESS             INDENT "Подсветка ВЫКЛ"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR                     "Цвет"
#define TR_SPLASHSCREEN                "Заставка"
#define TR_PLAY_HELLO                  "Звук Запуска"
#define TR_PWR_ON_DELAY                "Время Включения"
#define TR_PWR_OFF_DELAY               "Время Выключения"
#define TR_THROTTLE_WARNING            TR(INDENT "Д-Предупреждение", INDENT "Положение Дроселя")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Свое-Пол", INDENT INDENT INDENT INDENT "Свое Положение?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Пол. %", "Положение %")
#define TR_SWITCHWARNING               TR(INDENT "С-Предупреждение", INDENT "Положения Свитча")
#define TR_POTWARNINGSTATE             TR(INDENT "Pot&Slid.", INDENT "Pots & sliders")
#define TR_SLIDERWARNING               TR(INDENT "Полож. Свитч.", INDENT "Положение Свитча")
#define TR_POTWARNING                  TR(INDENT "Pot warn.", INDENT "Pot positions")
#define TR_TIMEZONE                    "Временная зона"
#define TR_ADJUST_RTC                  "Настроить RTC"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Пор. Кн. По Ум.", "Порядок Каналов По Умолчанию")
#define TR_STICKS                      "Стики"
#define TR_POTS                        "Pots"
#define TR_SWITCHES                    "Свитчи"
#define TR_SWITCHES_DELAY              TR("Play delay", "Play delay (sw. mid pos)")
#define TR_SLAVE                       CENTER "Slave"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "Multiplier"
#define TR_CAL                         "Звонок"
#define TR_CALIBRATION                 "Калибровка"
#define TR_VTRIM                       "Трим - +"
#define TR_CALIB_DONE                  "Калибровка Завершена"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Надмите [Enter] Для Продолжения"
  #define TR_SETMIDPOINT               "По Центру Стики/pots/Слайдеры И Нажмите [Enter]"
  #define TR_MOVESTICKSPOTS            "Двигайте Стики/pots/Слайдеры И Нажмите [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " Начать"
  #define TR_SETMIDPOINT               "CENTER STICKS/SLIDERS"
  #define TR_MOVESTICKSPOTS            "MOVE STICKS/POTS"
#else
  #define TR_MENUTOSTART               CENTER "\010" TR_ENTER " Начать"
  #define TR_SETMIDPOINT               TR(CENTER "\004SET STICKS MIDPOINT", CENTER "\004CENTER STICKS/SLIDERS")
  #define TR_MOVESTICKSPOTS            CENTER "\006MOVE STICKS/POTS"
  #define TR_MENUWHENDONE              CENTER "\006" TR_ENTER " WHEN DONE"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      CENTER "Нет Данных"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua Скрипт"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_DURATION_MS             TR("[D]","Duration(ms): ")
#define TR_INTERVAL_MS             TR("[I]","Interval(ms): ")
#define TR_MEM_USED_SCRIPT         "Скрипт(B): "
#define TR_MEM_USED_WIDGET         "Виджет(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Аудио: "
#define TR_GPS_FIX_YES                 "Fix: Yes"
#define TR_GPS_FIX_NO                  "Fix: No"
#define TR_GPS_SATS                    "Sats: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Меню: "
#define TR_TIMER_LABEL                 "Таймер"
#define TR_THROTTLE_PERCENT_LABEL      "Дросель %"
#define TR_BATT_LABEL                  "АКБ"
#define TR_SESSION                     "Сесия"
#define TR_MENUTORESET                 TR_ENTER " Для Сброса"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "Модель"
#define TR_FM                          "FM"
#define TR_EEPROMLOWMEM                "EEPROM Нехватает Памяти"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Нажать Любую Кнопку Для Сброса"
#define TR_THROTTLE_NOT_IDLE           "Дросель Не Работает На ХХ"
#define TR_ALARMSDISABLED              "Аварийные Сигналы Отключены"
#define TR_PRESSANYKEY                 TR("\010Нажмите Любую Кнопку", "Нажмите Любую Кнопку")
#define TR_BADEEPROMDATA               "Неверные Данные EEprom"
#define TR_BAD_RADIO_DATA              "Отсутствуют Или Некорректные Радиоданные"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            "Форматирование EEPROM"
#define TR_STORAGE_FORMAT              "Форматирование Памяти"
#define TR_EEPROMOVERFLOW              "EEPROM overflow"
#define TR_RADIO_SETUP                 "НАСТРОЙКА РАДИО"
#define TR_MENUTRAINER                 "ТРЕНЕР"
#define TR_MENUSPECIALFUNCS            "ГЛОБАЛЬНЫЕ ФУНКЦИИ"
#define TR_MENUVERSION                 "ВЕРСИЯ"
#define TR_MENU_RADIO_SWITCHES         TR("СВИТЧИ", "ТЕСТ СВИТЧЕЙ")
#define TR_MENU_RADIO_ANALOGS          TR("FYFKJUB", "ТЕСТ АНАЛОГОВ")
#define TR_MENU_RADIO_ANALOGS_CALIB    "КАЛИБРОВКА АНАЛОГОВ"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION             "КАЛИБРОВКА"
#define TR_MENU_FSWITCH                "НАСТРАИВАЕМЫЕ СВИТЧИ"
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
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_GPS_COORDS_FORMAT           TR("GPS Coords", "Coordinate format")
#define TR_VARIO                       TR("Vario", "Variometer")
#define TR_PITCH_AT_ZERO               "Pitch zero"
#define TR_PITCH_AT_MAX                "Pitch max"
#define TR_REPEAT_AT_ZERO              "Repeat zero"
#define TR_BATT_CALIB                  TR("Batt. calib", "Battery calibration")
#define TR_CURRENT_CALIB               "Current calib"
#define TR_VOLTAGE                     TR(INDENT "Voltage", INDENT "Voltage source")
#define TR_SELECT_MODEL                "Select model"
#define TR_MANAGE_MODELS               "MANAGE MODELS"
#define TR_MODELS                      "Models"
#define TR_SELECT_MODE                 "Select mode"
#define TR_CREATE_MODEL                "Create model"
#define TR_FAVORITE_LABEL              "Favorites"
#define TR_MODELS_MOVED                "Unused models moved to"
#define TR_NEW_MODEL                   "New Model"
#define TR_INVALID_MODEL               "Invalid Model"
#define TR_EDIT_LABELS                 "Edit Labels"
#define TR_LABEL_MODEL                 "Label model"
#define TR_MOVE_UP                     "Move Up"
#define TR_MOVE_DOWN                   "Move Down"
#define TR_ENTER_LABEL                 "Enter Label"
#define TR_LABEL                       "Label"
#define TR_LABELS                      "Labels"
#define TR_CURRENT_MODEL               "Current"
#define TR_ACTIVE                      "Active"
#define TR_NEW                         "New"
#define TR_NEW_LABEL                   "New Label"
#define TR_RENAME_LABEL                "Rename Label"
#define TR_DELETE_LABEL                "Delete Label"
#define TR_DUPLICATE_MODEL             "Duplicate model"
#define TR_COPY_MODEL                  "Copy model"
#define TR_MOVE_MODEL                  "Move model"
#define TR_BACKUP_MODEL                "Backup model"
#define TR_DELETE_MODEL                "Delete model"
#define TR_RESTORE_MODEL               "Restore model"
#define TR_DELETE_ERROR                "Delete error"
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
#define TR_SHOW_MIXER_MONITORS         "Show mixer monitors"
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
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\nLogs and Screenshots disabled"
#else
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036Logs & Screenshots" LCDW_128_480_LINEBREAK "disabled"
#endif
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Incompatible"
#define TR_WARNING                     "ПРЕДУПРЕЖДЕНИЕ"
#define TR_EEPROMWARN                  "EEPROM"
#define TR_STORAGE_WARNING             "ПАМЯТЬ"
#define TR_EEPROM_CONVERTING           "Converting EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD card conversion required"
#define TR_CONVERTING                  "Converting: "
#define TR_THROTTLE_UPPERCASE          "ДРОСЕЛЬ"
#define TR_ALARMSWARN                  "СИГНАЛЫ"
#define TR_SWITCHWARN                  TR("SWITCH", "CONTROL")
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE                "Use for tests only"
#define TR_WRONG_SDCARDVERSION         TR("Expected ver: ", "Expected version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery low"
#define TR_WARN_MULTI_LOWPOWER         "Low power mode"
#define TR_BATTERY                     "АКБ"
#define TR_WRONG_PCBREV                "Wrong PCB detected"
#define TR_EMERGENCY_MODE              "EMERGENCY MODE"
#define TR_NO_FAILSAFE                 "Failsafe not set"
#define TR_KEYSTUCK                    "Key stuck"
#define TR_VOLUME                      "Громкость"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "ПОДСВЕТКА"
#define TR_CPU_TEMP                    "CPU Темп."
#define TR_CPU_CURRENT                 "Current"
#define TR_CPU_MAH                     "Consumpt."
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "MB Темп."
#define TR_CAPAWARNING                 INDENT "Capacity low"
#define TR_TEMPWARNING                 INDENT "Overheat"
#define TR_TTL_WARNING                 "Warning: use 3.3V logic levels"
#define TR_FUNC                        "Function"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Duration"
#define TR_DELAY                       "Delay"
#define TR_SD_CARD                     "SD КАРТА"
#define TR_SDHC_CARD                   "SD-HC КАРТА"
#define TR_NO_SOUNDS_ON_SD             "Нт звуков на SD"
#define TR_NO_MODELS_ON_SD             "Нет Модели На SD"
#define TR_NO_BITMAPS_ON_SD            "No bitmaps on SD"
#define TR_NO_SCRIPTS_ON_SD            "Нет скриптов на SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntax error", "Script syntax error")
#define TR_SCRIPT_PANIC                "Script panic"
#define TR_SCRIPT_KILLED               "Script killed"
#define TR_SCRIPT_ERROR                "Неизвестная ошибка"
#define TR_PLAY_FILE                   "Play"
#define TR_DELETE_FILE                 "УДАЛИТЬ"
#define TR_COPY_FILE                   "КОПИРОВАТЬ"
#define TR_RENAME_FILE                 "ПЕРЕИМЕНОВАТЬ"
#define TR_ASSIGN_BITMAP               "Assign bitmap"
#define TR_ASSIGN_SPLASH               "Splash screen"
#define TR_EXECUTE_FILE                "Execute"
#define TR_REMOVED                     " removed"
#define TR_SD_INFO                     "Information"
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "HARDWARE"
#define TR_FORMATTING                  "ФОРМАТИРОВАНИЕ..."
#define TR_TEMP_CALIB                  "Temp. Calib"
#define TR_TIME                        "Время"
#define TR_MAXBAUDRATE                 "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE                 TR("Sampling","Sample Mode")
#define TR_SAMPLE_MODES                "Normal","OneBit"
#define TR_LOADING                     "Loading..."
#define TR_DELETE_THEME                "Удалить тему?"
#define TR_SAVE_THEME                  "Сохранить тему?"
#define TR_EDIT_COLOR                  "Выбор Цвета"
#define TR_NO_THEME_IMAGE              "No theme image"
#define TR_BACKLIGHT_TIMER             "Время Бездействия"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Быстрый выбор модели"
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "Выберите Папку Шаблона"
#define TR_SELECT_TEMPLATE             "ВЫБЕРИТЕ ШАБЛОН МОДЕЛИ:"
#define TR_NO_TEMPLATES                "В этой папке нет ни одного шаблона модели"
#define TR_SAVE_TEMPLATE               "Сохранить Шаблон"
#define TR_BLANK_MODEL                 "Пустая Модель"
#define TR_BLANK_MODEL_INFO            "Создайте Пустую Модель"
#define TR_FILE_EXISTS                 "ФАЙЛ УЖЕ СУЩЕСТВУЕТ"
#define TR_ASK_OVERWRITE               "Вы Хотите Перезаписать?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "НАЙТИ"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Dist addr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Local addr"
#define TR_BLUETOOTH_PIN_CODE          "PIN код"
#define TR_BLUETOOTH_NODEVICES         "Устройства Не Найдены"
#define TR_BLUETOOTH_SCANNING          "ПОИСК..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Включено"
#else
#define TR_BLUETOOTH_MODES             "---","Телеметрия","Тренер"
#endif

#define TR_SD_INFO_TITLE               "SD Инфо"
#define TR_SD_TYPE                     "Тип:"
#define TR_SD_SPEED                    "Скорость:"
#define TR_SD_SECTORS                  "Сектора:"
#define TR_SD_SIZE                     "Размер:"
#define TR_TYPE                        INDENT "Type"
#define TR_GLOBAL_VARS                 "Global variables"
#define TR_GVARS                       "GVARS"
#define TR_GLOBAL_VAR                  "Global variable"
#define TR_MENU_GLOBAL_VARS              "GLOBAL VARIABLES"
#define TR_OWN                         "Own"
#define TR_DATE                        "Date"
#define TR_MONTHS                      { "Янв", "Фев", "Мар", "Апр", "Май", "Июн", "Июл", "Авг", "Сен", "Окт", "Ноя", "Дек" }
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR            "МОНИТОР КАНАЛОВ"
#define TR_MIXERS_MONITOR              "МОНИТОРMIXERS"
#define TR_PATH_TOO_LONG               "Путь Слишком Длинный"
#define TR_VIEW_TEXT                   "View text"
#define TR_FLASH_BOOTLOADER            "Прошить bootloader"
#define TR_FLASH_DEVICE                TR("Прошить Устройство","Прошить Устройство")
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
#define TR_FIRMWARE_UPDATE_ERROR       TR("Ошибка Обновления FW", "Ошибка обновления Firmware")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Успешная Прошивка"
#define TR_WRITING                     "Запись..."
#define TR_CONFIRM_FORMAT              "Подтвердите Форматирование?"
#define TR_INTERNALRF                  "Внутренний RF"
#define TR_INTERNAL_MODULE             TR("Внут. Модуль", "Внутренний Модуль")
#define TR_EXTERNAL_MODULE             TR("Внеш. Модуль", "Внешний Модуль")
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX требуется обновление"
#define TR_TELEMETRY_DISABLED          "Телем. Отключена"
#define TR_MORE_OPTIONS_AVAILABLE      "More options available"
#define TR_NO_MODULE_INFORMATION       "Нет информации о модуле"
#define TR_EXTERNALRF                  "Внешний RF"
#define TR_FAILSAFE                    TR(INDENT "Failsafe", INDENT "Failsafe mode")
#define TR_FAILSAFESET                 "FAILSAFE SETTINGS"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Owner ID"
#define TR_HOLD                        "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                        "None"
#define TR_NONE_UPPERCASE              "NONE"
#define TR_MENUSENSOR                  "Сенсор"
#define TR_POWERMETER_PEAK             "Peak"
#define TR_POWERMETER_POWER            "Power"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Freq."
#define TR_MENUTOOLS                   "TOOLS"
#define TR_TURN_OFF_RECEIVER           "Turn off receiver"
#define TR_STOPPING                    "Остановка..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
#define TR_MENU_POWER_METER            "POWER METER"
#define TR_SENSOR                      "СЕНСОР"
#define TR_COUNTRY_CODE                "Код страны"
#define TR_USBMODE                     "USB mode"
#define TR_JACK_MODE                   "Jack mode"
#define TR_VOICE_LANGUAGE              "Язык Озвучки"
#define TR_UNITS_SYSTEM                "Единицы"
#define TR_EDIT                        "Редактировать"
#define TR_INSERT_BEFORE               "Вставить перед"
#define TR_INSERT_AFTER                "Вставить после"
#define TR_COPY                        "Копировать"
#define TR_MOVE                        "Move"
#define TR_PASTE                       "Вставить"
#define TR_PASTE_AFTER                 "Paste after"
#define TR_PASTE_BEFORE                "Paste before"
#define TR_DELETE                      "Удалить"
#define TR_INSERT                      "Вставить"
#define TR_RESET_FLIGHT                "Удалить полет"
#define TR_RESET_TIMER1                "Удалить таймер1"
#define TR_RESET_TIMER2                "Удалить таймер2"
#define TR_RESET_TIMER3                "Удалить таймер3"
#define TR_RESET_TELEMETRY             "Удалить телеметрию"
#define TR_STATISTICS                  "Статистика"
#define TR_ABOUT_US                    "About"
#define TR_USB_JOYSTICK                "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB Память (SD)"
#define TR_USB_SERIAL                  "USB Serial (VCP)"
#define TR_SETUP_SCREENS               "Выбор экранов"
#define TR_MONITOR_SCREENS             "Мониторы"
#define TR_AND_SWITCH                  "AND Свич"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Калибровка Аналогов"
#define TR_ANADIAGS_FILTRAWDEV         "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW          "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX             "Мин., Макс. и Диапазон"
#define TR_ANADIAGS_MOVE               "Дигайте аналоги до конца!"
#define TR_SPEAKER                     INDENT "Speaker"
#define TR_BUZZER                      INDENT "Buzzer"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_POWERMETER_ATTN_NEEDED      "Attenuator needed"
#define TR_PXX2_SELECT_RX              "Выбор RX"
#define TR_PXX2_DEFAULT                "<по умолчанию>"
#define TR_BT_SELECT_DEVICE            "Выбор устройства"
#define TR_DISCOVER                    "Discover"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Предупреждение..."
#define TR_RECEIVER_DELETE             "Delete receiver?"
#define TR_RECEIVER_RESET              "Reset receiver?"
#define TR_SHARE                       "Share"
#define TR_BIND                        "Bind"
#define TR_REGISTER                    TR("Reg", "Register")
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "Range"))
#define TR_RANGE_TEST                  "Range Test"
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "RECEIVER OPTIONS")
#define TR_RESET_BTN                   BUTTON("Reset")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON("Keys")
#define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "Аналоги"))
#define TR_FS_BTN                      BUTTON(TR("Custom sw", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_SET                         BUTTON("Set")
#define TR_TRAINER                     "Trainer"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM              CENTER "TX antenna problem!"
#define TR_MODELIDUSED                 "ID used in:"
#define TR_MODELIDUNIQUE               "ID is unique"
#define TR_MODULE                      "Модуль"
#define TR_RX_NAME                     "Rx Имя"
#define TR_TELEMETRY_TYPE              TR("Тип", "Тип Телеметрии")
#define TR_TELEMETRY_SENSORS           "Сенсоры"
#define TR_VALUE                       "Value"
#define TR_PERIOD                      "Период"
#define TR_INTERVAL                    "Интервал"
#define TR_REPEAT                      "Повтор"
#define TR_ENABLE                      "Включено"
#define TR_TOPLCDTIMER                 "Top LCD Timer"
#define TR_UNIT                        "Unit"
#define TR_TELEMETRY_NEWSENSOR         INDENT "Add new"
#define TR_CHANNELRANGE                TR(INDENT "Ch. Range", INDENT "Channel Range")
#define TR_ANTENNACONFIRM1             "НАР. Антенна"
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
#define TR_POPUP                       "Popup"
#define TR_MIN                         "Мин"
#define TR_MAX                         "Макс"
#define TR_CURVE_PRESET                "Предустановка..."
#define TR_PRESET                      "Предустановка"
#define TR_MIRROR                      "Mirror"
#define TR_CLEAR                       "Clear"
#define TR_RESET                       "Сброс"
#define TR_RESET_SUBMENU               "Сброс..."
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
#define TR_PREFLIGHT_POTSLIDER_CHECK   "ВЫКЛ","ВКЛ","АВТО"
#define TR_PREFLIGHT                   "Предполетные проверки"
#define TR_CHECKLIST                   TR(INDENT "Контрольный список", INDENT "Отобразить контрольный список")
#define TR_CHECKLIST_INTERACTIVE       TR3(INDENT "КС-Интерактивный", INDENT "Интер контр список", INDENT "Интерактивный контрольный список")
#define TR_AUX_SERIAL_MODE             "Последовательный порт"
#define TR_AUX2_SERIAL_MODE            "Последовательный порт 2"
#define TR_AUX_SERIAL_PORT_POWER       "Мощность порта"
#define TR_SCRIPT                      "Скрипт"
#define TR_INPUTS                      "Входные"
#define TR_OUTPUTS                     "ВЫходные"
#define STR_EEBACKUP                   "EEPROM Резервная копия"
#define STR_FACTORYRESET               "Сброс к заводским настройкам"
#define TR_CONFIRMRESET                TR("Удалить все", "Удалить все модели и настройки?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Слишком много Lua-скриптов!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "Авто","Вкл"
#define TR_NO_TELEMETRY_SCREENS        "No Telemetry Screens"
#define TR_TOUCH_PANEL                 "Сенсорная панель:"
#define TR_FILE_SIZE                   "Размер файла"
#define TR_FILE_OPEN                   "Все равно открыть?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Имя"
#define TR_PHASES_HEADERS_SW           "Свич"
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
#define TR_LIMITS_HEADERS_MIN          "Мин"
#define TR_LIMITS_HEADERS_MAX          "Макс"
#define TR_LIMITS_HEADERS_DIRECTION    "Направление"
#define TR_LIMITS_HEADERS_CURVE        "Кривая"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Центр"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Subtrim mode"
#define TR_INVERTED                    "Перевернутый"


#define TR_LSW_DESCRIPTIONS            { "Тип сравнения или функция", "Первая переменная", "Вторая переменная или константа", "Вторая переменная или константа", "Дополнительное условие для включения линии", "Минимальная продолжительность логического переключения", "Минимальная Истинная продолжительность включения переключателя" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "Первый канал"
  #define TR_FILL_BACKGROUND           "Заполнить фон?"
  #define TR_BG_COLOR                  "BG Color"
  #define TR_SLIDERS_TRIMS             "Sliders+Trims"
  #define TR_SLIDERS                   "Sliders"
  #define TR_FLIGHT_MODE               "Ht;bv gjktnf"
  #define TR_INVALID_FILE              "Invalid File"
  #define TR_TIMER_SOURCE              "Timer source"
  #define TR_SIZE                      "Размер"
  #define TR_SHADOW                    "Тень"
  #define TR_ALIGN_LABEL               "Align label"
  #define TR_ALIGN_VALUE               "Align value"
  #define TR_ALIGN_OPTS                { "Left", "Center", "Right" }
  #define TR_TEXT                      "Text"
  #define TR_COLOR                     "Color"
  #define TR_MAIN_VIEW_X               "Main view XX"
  #define TR_PANEL1_BACKGROUND         "Panel1 background"
  #define TR_PANEL2_BACKGROUND         "Panel2 background"
  #define TR_WIDGET_GAUGE              "Gauge"
  #define TR_WIDGET_MODELBMP           "Model info"
  #define TR_WIDGET_OUTPUTS            "Outputs"
  #define TR_WIDGET_TEXT               "Text"
  #define TR_WIDGET_TIMER              "Timer"
  #define TR_WIDGET_VALUE              "Value"
#endif

// Bootloader common - Ascii only
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
   // Bootloader Taranis specific - Ascii only
  #define TR_BL_RESTORE_EEPROM        "Restore EEPROM"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Press the power button."
    #define TR_BL_FLASH_EXIT          "Exit the flashing mode."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - Ascii only
  #define TR_BL_SELECT_KEY            "[ENT] to select file"
  #define TR_BL_FLASH_KEY             "Hold [ENT] long to flash"
  #define TR_BL_EXIT_KEY              "[RTN] to exit"
#elif defined(PCBNV14)
   // Bootloader NV14 specific - Ascii only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] to select file"
  #define TR_BL_FLASH_KEY             "Hold [R TRIM] long to flash"
  #define TR_BL_EXIT_KEY              " [L TRIM] to exit"
  #define TR_BL_ENABLE                "Enable"
  #define TR_BL_DISABLE               "Disable"
#endif

// About screen
#define TR_ABOUTUS                     TR(" ABOUT ", "ABOUT")

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
#define TR_INVERTED_SERIAL             INDENT "Invert"
#define TR_IGNORE_INSTANCE             TR(INDENT "No inst.", INDENT "Ignore instances")
#define TR_SHOW_INSTANCE_ID            "Show instance ID"
#define TR_DISCOVER_SENSORS            "Discover new"
#define TR_STOP_DISCOVER_SENSORS       "Stop"
#define TR_DELETE_ALL_SENSORS          "Delete all"
#define TR_CONFIRMDELETE               "Really " LCDW_128_480_LINEBREAK "delete all ?"
#define TR_SELECT_WIDGET               "Select widget"
#define TR_WIDGET_FULLSCREEN           "Full screen"
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
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "MIN"
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
#define TR_AUDIO_MUTE                  TR("Audio mute","Mute if no sound")
#define TR_JITTER_FILTER               "ADC filter"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Check RTC", "Check RTC voltage")
#define TR_AUTH_FAILURE                "Auth-failure"
#define TR_RACING_MODE                 "Racing mode"

#define TR_USE_THEME_COLOR              "Используйте Цвет Темы"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Add all Trims to Subtrims"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS      "Монитор Открытого Канала"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Мон. Открытого Канала"
#endif
#define TR_DUPLICATE                    "Дюбликат"
#define TR_ACTIVATE                     "Активный"
#define TR_RED                          "Красная"
#define TR_BLUE                         "Синяяя"
#define TR_GREEN                        "Зеленая"
#define TR_COLOR_PICKER                 "Средство выбора цвета"
#define TR_EDIT_THEME_DETAILS           "Редактировать детали темы"
#define TR_THEME_COLOR_DEFAULT         "По умолчанию"
#define TR_THEME_COLOR_PRIMARY1        "PRIMARY1"
#define TR_THEME_COLOR_PRIMARY2        "PRIMARY2"
#define TR_THEME_COLOR_PRIMARY3        "PRIMARY3"
#define TR_THEME_COLOR_SECONDARY1      "SECONDARY1"
#define TR_THEME_COLOR_SECONDARY2      "PRIMARY2"
#define TR_THEME_COLOR_SECONDARY3      "SECONDARY3"
#define TR_THEME_COLOR_FOCUS           "FOCUS"
#define TR_THEME_COLOR_EDIT            "РЕДАКТИРОВАТЬ"
#define TR_THEME_COLOR_ACTIVE          "АКТИВНАЯ"
#define TR_THEME_COLOR_WARNING         "ПРЕДУПРЕЖДЕНИЕ"
#define TR_THEME_COLOR_DISABLED        "ОТКЛЮЧЕННАЯ"
#define TR_THEME_COLOR_CUSTOM          "ПОЛЬЗОВАТЕЛЬСКАЯ"
#define TR_THEME_CHECKBOX              "Checkbox"
#define TR_THEME_ACTIVE                "АКТИВНАЯ"
#define TR_THEME_REGULAR               "РЕГУЛЯРНАЯ"
#define TR_THEME_WARNING               "ПРЕДУПРЕЖДЕНИЕ"
#define TR_THEME_DISABLED              "ОТКЛЮЧЕННАЯ"
#define TR_THEME_EDIT                  "РЕДАКТИРОВАТЬ"
#define TR_THEME_FOCUS                 "Focus"
#define TR_AUTHOR                       "Автор"
#define TR_DESCRIPTION                  "Описание"
#define TR_SAVE                         "Сохранить"
#define TR_CANCEL                       "Отменить"
#define TR_EDIT_THEME                   "РЕДАКТИРОВАТЬ ТЕМУ"
#define TR_DETAILS                      "Детали"
#define TR_THEME_EDITOR                 "ТЕМЫ"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Select\nModel"
#define TR_MAIN_MENU_MANAGE_MODELS      "Manage\nModels"
#define TR_MAIN_MENU_MODEL_NOTES        "Model\nNotes"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Channel\nMonitor"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Model\nSettings"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Radio\nSettings"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Screens\nSettings"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Reset\nTelemetry"
#define TR_MAIN_MENU_STATISTICS         "Statistics"
#define TR_MAIN_MENU_ABOUT_EDGETX       "About\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "Английский"
#define TR_VOICE_CHINESE                "Китайский"
#define TR_VOICE_CZECH                  "Чешский"
#define TR_VOICE_DANISH                 "Датский"
#define TR_VOICE_DEUTSCH                "Немецкий"
#define TR_VOICE_DUTCH                  "Голландский"
#define TR_VOICE_ESPANOL                "Испанский"
#define TR_VOICE_FRANCAIS               "Французский"
#define TR_VOICE_HUNGARIAN              "Венгерский"
#define TR_VOICE_ITALIANO               "Итальянский"
#define TR_VOICE_POLISH                 "Польский"
#define TR_VOICE_PORTUGUES              "Португальский"
#define TR_VOICE_RUSSIAN                "Русский"
#define TR_VOICE_SLOVAK                 "Словацкий"
#define TR_VOICE_SWEDISH                "Шведский"
#define TR_VOICE_TAIWANESE              "Тайваньский"
#define TR_VOICE_JAPANESE               "Японский"
#define TR_VOICE_HEBREW                 "Иврит"

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
#define TR_VUSBJOYSTICK_CH_SIM         "Ail","Ele","Rud","Thr"
#define TR_USBJOYSTICK_CH_INVERSION    "Inversion"
#define TR_USBJOYSTICK_CH_BTNNUM       "Button no."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Button no. collision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Axis collision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "None","X-Y, Z-rX","X-Y, rX-rY"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Apply changes"

#define TR_DIGITAL_SERVO          "Servo333HZ"
#define TR_ANALOG_SERVO           "Servo 50HZ"
#define TR_SIGNAL_OUTPUT          "Signal output"
#define TR_SERIAL_BUS             "Serial bus"
#define TR_SYNC                   "Sync"

#define TR_ENABLED_FEATURES       "Enabled Features"
#define TR_RADIO_MENU_TABS        "Radio Menu Tabs"
#define TR_MODEL_MENU_TABS        "Вкладки Меню Модели"
