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

// RU translations author: @kobakirill <blokhin.kirill@yandex.ru>

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

#define TR_MINUTE_SINGULAR            "МИНУТА"
#define TR_MINUTE_PLURAL1             "Мин"
#define TR_MINUTE_PLURAL2             "Мин"

#define TR_OFFON                       "Выкл","Вкл"
#define TR_MMMINV                      "---","Инв"
#define TR_VBEEPMODE                   "Тихо","Сигн","Без клав","Все"
#define TR_VBLMODE                     "Выкл","Клав",TR("Стики","Стики"),"Двойн","Вкл"
#define TR_TRNMODE                     "Вык",TR("+=","Добав"),TR(":=","Замен")
#define TR_TRNCHN                      "КН1","КН2","КН3","КН4"

#define TR_AUX_SERIAL_MODES            "Выкл","Зерк телем","Телем вход","SBUS Тренаж","LUA","CLI","GPS","Отлад","SpaceMouse","Внеш модуль"
#define TR_SWTYPES                     "Нет","тумбл","2Х-поз пер","3Х-поз пер"
#define TR_POTTYPES                    "None","Pot",TR("Pot w. det","Pot with detent"),"Slider",TR("Multipos","Multipos Switch"),"Axis X","Axis Y","Switch"
#define TR_VPERSISTENT                 "Выкл","Полет","Руч сброс"
#define TR_COUNTRY_CODES               TR("US","США"),TR("JP","Япония"),TR("EU","Евро")
#define TR_USBMODES                    "Выбор",TR("Джойс","Джойстик"),TR("SD-карта","Хранил"),"Серийн"
#define TR_JACK_MODES                  "Выбор","Аудио","Трен"

#define TR_SBUS_INVERSION_VALUES       "Норм","Не перев"
#define TR_MULTI_CUSTOM                "Настроенный"
#define TR_VTRIMINC                    TR("Экспо","экспо"),TR("Оч мелк","Оч мелко"),"Мелко","Сред","Грубо"
#define TR_VDISPLAYTRIMS               "Нет","Измен","Да"
#define TR_VBEEPCOUNTDOWN              "Без звука", "Сигн", "Голос", "Вибро", TR("Звук вибро", "Сигн вибро"), TR("Голос вибро", "Голос вибро")
#define TR_COUNTDOWNVALUES             "5с","10с","20с","30с"
#define TR_VVARIOCENTER                "Тон","Тихо"
#define TR_CURVE_TYPES                 "Стд","Выбор"

#define TR_ADCFILTERVALUES             "Глоб","Вык","Вкл"

#define TR_VCURVETYPE                  "Разн", "Экспо", "Функц", "Польз"
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Добав", "Умнож", "Замен"
#define TR_VMLTPX2                     "+=","*=",":="

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Таймер"
  #define TR_CSWSTICKY                 "Липучка"
  #define TR_CSWSTAY                   "Край"
#else
  #define TR_CSWTIMER                  "Таймер"
  #define TR_CSWSTICKY                 "Липучка"
  #define TR_CSWSTAY                   "Кррай"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Таймер"
#define TR_SF_INST_TRIM                "Мгнов трим"
#define TR_SF_RESET                    "Сброс"
#define TR_SF_SET_TIMER                "Установ"
#define TR_SF_VOLUME                   "Громкость"
#define TR_SF_FAILSAFE                 "Авар управ"
#define TR_SF_RANGE_CHECK              "Пров диапаз"
#define TR_SF_MOD_BIND                 "Bind модуль"
#define TR_SF_RGBLEDS                  "RGB подсветка"
 
#define TR_SOUND                       "Воспр звук"
#define TR_PLAY_TRACK                  TR("Ply Trk","Воспр трек")
#define TR_PLAY_VALUE                  TR("Play Val","Play Value")
#define TR_SF_HAPTIC                   "Вибро"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua скрипт")
#define TR_SF_BG_MUSIC                 "Муз фон"
#define TR_SF_BG_MUSIC_PAUSE           "Муз ||"
#define TR_SF_LOGS                     "SD логи"
#define TR_ADJUST_GVAR                 "Настройка"
#define TR_SF_BACKLIGHT                "Подсветка"
#define TR_SF_VARIO                    "Варио"
#define TR_SF_TEST                     "Тест"

#if LCD_W >= 212
  #define TR_SF_SAFETY                 "Оменить"
#else
  #define TR_SF_SAFETY                 "Отмен"
#endif

#define TR_SF_SCREENSHOT               "Скриншот"
#define TR_SF_RACING_MODE              "Режим гонка"
#define TR_SF_DISABLE_TOUCH            "Сенсор вык"
#define TR_SF_DISABLE_AUDIO_AMP        "Вык аудио усил."
#define TR_SF_SET_SCREEN               "Выбр глав экран"
#define TR_SF_SET_TELEMSCREEN          "Set Screen"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Телем", "Телеметрия")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS            "Тмр1","Трм2","Трм3"

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS, TR("Все","Полетн"), TR_FSW_RESET_TELEM, TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mlch"
#define LENGTH_UNIT_METR               "м"
#define SPEED_UNIT_METR                "кмч"

#define TR_VUNITSSYSTEM                "Метрич","Импер"
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Нет","Цифры","График","Скрипты"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "Rud"
#define TR_STICK_NAMES1                "Ele"
#define TR_STICK_NAMES2                "Thr"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "По умолч","Высокий"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "Вкл","Один"

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "Hats mode"
#define TR_HATSOPT                     "Тримы","Кнопки","Тумбл","Глоб"
#define TR_HATSMODE_TRIMS              "Hats mode: Тримы"
#define TR_HATSMODE_KEYS               "Hats mode: Кнопки"
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
  #define TR_ROTARY_ENC_OPT            "Норм","Перевер"
#else
  #define TR_ROTARY_ENC_OPT            "Норм","Перевер","V-I H-N","V-I H-A","V-N E-I"
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


#define TR_SRC_BATT                    "АКБ"
#define TR_SRC_TIME                    "Время"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "ТМР"

#define TR_VTMRMODES                   "Вык","Вкл","Strt","THs","TH%","THt"
#define TR_VTRAINER_MASTER_OFF         "Вык"
#define TR_VTRAINER_MASTER_JACK        "Мастер/Разъем"
#define TR_VTRAINER_SLAVE_JACK         "Раб/Разъем"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Мастер/SBUS модуль"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Мастер/CPPM модуль"
#define TR_VTRAINER_MASTER_BATTERY     "Мастер/Серийный"
#define TR_VTRAINER_BLUETOOTH          "Мастер/" TR("BT","Bluetooth"), "Раб/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Мастер/Мульти"
#define TR_VFAILSAFE                   "Не устан","Удерж","Настр","Нет импул","Прием"
#define TR_VSENSORTYPES                "Настроен","Рассчитан"
#define TR_VFORMULAS                   "Добав","Средн","Мин","Макс","Умнож","Суммир","Ячейка","Расх","Дист"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Низ","1","2","3","4","5","6","7","8","Выс","Дельта"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (Тол цен)", "= (Симметр)"
#define TR_TIMER_DIR                   TR("Остал", "Остал"), TR("Прошло", "Прошед")
#define TR_PPMUNIT                     "0.--","0.0","us"

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "Стд"
#else
#define TR_FONT_SIZES                  "Стд","Жирн","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENT]"
#elif defined(PCBNV14) || defined(PCBPL18)
  #define TR_ENTER                     "[ДАЛЕЕ]"
#else
  #define TR_ENTER                     "[МЕНЮ]"
#endif

#if defined(COLORLCD)
  #define TR_EXIT                      "ВОЗВР"
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "ВЫХОД"
  #define TR_OK                        TR("\010" "\010" "\010" "[OK]", "\010" "\010" "\010" "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_FREE                        "Свободн"
#define TR_YES                         "Да"
#define TR_NO                          "Нет"
#define TR_DELETEMODEL                 "Удалить модель"
#define TR_COPYINGMODEL                "Копировать модель"
#define TR_MOVINGMODEL                 "Перенести модель"
#define TR_LOADINGMODEL                "Загрузить модель"
#define TR_UNLABELEDMODEL              "Без названия"
#define TR_NAME                        "Название"
#define TR_MODELNAME                   "Название"
#define TR_PHASENAME                   "Назв фазы"
#define TR_MIXNAME                     "Название Mix"
#define TR_INPUTNAME                   TR("Ввод", "вод")
#define TR_EXPONAME                    TR("Название", "Название")
#define TR_BITMAP                      "Изображ"
#define TR_NO_PICTURE                  "Без изображ"
#define TR_TIMER                       TR("Таймер", "Таймер ")
#define TR_START                       "Старт"
#define TR_ELIMITS                     TR("расш лимит", "Расш лимит")
#define TR_ETRIMS                      TR("Расш трим", "Расш тримы")
#define TR_TRIMINC                     "Шаг трима"
#define TR_DISPLAY_TRIMS               TR("Пок. тримы", "Показ тримы")
#define TR_TTRACE                      TR("Т-источник", "Источник")
#define TR_TTRIM                       TR("Т-трим-ХХ", "Только трим хх")
#define TR_TTRIM_SW                    TR("Т-трим-перек", "Трим перек")
#define TR_BEEPCTR                     TR("Звук на цен", "Звук на центре")
#define TR_USE_GLOBAL_FUNCS            TR("Глоб. функц", "Глоб функц")
#define TR_PROTOCOL                    TR("Проток", "Протокол")
  #define TR_PPMFRAME                  "Фрейм PPM"
  #define TR_REFRESHRATE               TR("Обновл", "Част обновл")
  #define STR_WARN_BATTVOLTAGE         TR("Выход - V АКБ: ", "Предупреждение: уровень выхода - V АКБ: ")
#define TR_WARN_5VOLTS                 "Предупреждение: уровень выхода - 5 вольт"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Тумблер"
#define TR_FUNCTION_SWITCHES           "Настр тумблеры"
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
#define TR_SF_SWITCH                   "Триггер"
#define TR_TRIMS                       "тримы"
#define TR_FADEIN                      "Затух входа"
#define TR_FADEOUT                     "Затух выхода"
#define TR_DEFAULT                     "(По умолч)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Провер тримов"
#else
  #define TR_CHECKTRIMS                "\006Check\012trims"
#endif
#define TR_SWASHTYPE                   "Наклонный тип"
#define TR_COLLECTIVE                  TR("Коллек", "Коллек")
#define TR_AILERON                     TR("Крен", "Крен")
#define TR_ELEVATOR                    TR("Тангаж", "Тангаж")
#define TR_SWASHRING                   "Поворот"
#define TR_MODE                        "Режим"
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "Лев"
#else
  #define TR_LEFT_STICK                "Лев"
#endif
#define TR_SUBTYPE                     "Подтип"
#define TR_NOFREEEXPO                  "Нет своб экспо!"
#define TR_NOFREEMIXER                 "Нет своб микш!"
#define TR_SOURCE                       "Источ"
#define TR_WEIGHT                      "Вес"
#define TR_SIDE                        "Сторона"
#define TR_OFFSET                       "Смещение"
#define TR_TRIM                        "Трим"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Кривая"
#define TR_FLMODE                      TR("Режим", "Режим")
#define TR_MIXWARNING                  "Предупр"
#define TR_OFF                         "Вык"
#define TR_ANTENNA                     "Антенна"
#define TR_NO_INFORMATION              TR("Нет инфо", "Нет инфо")
#define TR_MULTPX                      "Multiplex"
#define TR_DELAYDOWN                   TR("Задер вниз", "Задер вниз")
#define TR_DELAYUP                     "Задер вверх"
#define TR_SLOWDOWN                    TR("Замедл вниз", "Замедл вниз")
#define TR_SLOWUP                      "Замед вверх"
#define TR_MIXES                       "MIXES"
#define TR_CV                          "CV"
#if defined(PCBNV14)
#define TR_GV                          "GV"
#else
#define TR_GV                          TR("G", "GV")
#endif
#define TR_RANGE                       "Диапаз"
#define TR_CENTER                      "Центр"
#define TR_ALARM                       "Сигнал тревоги"
#define TR_BLADES                      "Blades/Poles"
#define TR_SCREEN                      "Экран\001"
#define TR_SOUND_LABEL                 "Звук"
#define TR_LENGTH                      "Длин"
#define TR_BEEP_LENGTH                 "Длин звука"
#define TR_BEEP_PITCH                  "Выс звука"
#define TR_HAPTIC_LABEL                "Вибрация"
#define TR_STRENGTH                    "Сила"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Смещение"
#define TR_IMU_MAX                     "Макс"
#define TR_CONTRAST                    "Контраст"
#define TR_ALARMS_LABEL                "Сигнал тревоги"
#define TR_BATTERY_RANGE               TR("Диапаз АКБ", "Диапаз АКБ")
#define TR_BATTERYCHARGING             "Зарядка..."
#define TR_BATTERYFULL                 "АКБ Заряжена"
#define TR_BATTERYNONE                 "АКБ Отсутст"
#define TR_BATTERYWARNING              "АКБ Разряж"
#define TR_INACTIVITYALARM             "Бездействие"
#define TR_MEMORYWARNING               "Память заканч"
#define TR_ALARMWARNING                "Звук Выкл"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI выкл", "RSSI выкл")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("ВЫК Тренер", "Проверьте работу тренера при ВЫКЛ")
#define TR_MODEL_STILL_POWERED         "Модель работает"
#define TR_TRAINER_STILL_CONNECTED     "Тренер подкл"
#define TR_USB_STILL_CONNECTED         "USB подкл"
#define TR_MODEL_SHUTDOWN              "Отключение?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Нажмите [Enter] для подтв"
#define TR_THROTTLE_LABEL              "Газ"
#define TR_THROTTLE_START              "Газ cтарт"
#define TR_THROTTLEREVERSE             TR("Г-Реверс", "Реверс")
#define TR_MINUTEBEEP                  TR("Минут", "Минут")
#define TR_BEEPCOUNTDOWN               "Обр отс"
#define TR_PERSISTENT                  TR("Пост", "Пост")
#define TR_BACKLIGHT_LABEL             "Подсветка"
#define TR_GHOST_MENU_LABEL            "Меню призр"
#define TR_STATUS                      "Статус"
#define TR_BLONBRIGHTNESS              "Подсветка Вкл"
#define TR_BLOFFBRIGHTNESS             "Подсветка Вык"
#define TR_KEYS_BACKLIGHT              "Подсветка клавиш"
#define TR_BLCOLOR                     "Цвет"
#define TR_SPLASHSCREEN                "Заставка"
#define TR_PLAY_HELLO                  "Звук запуска"
#define TR_PWR_ON_DELAY                "Время включ"
#define TR_PWR_OFF_DELAY               "Время выключ"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off") 
#define TR_PWR_ON_OFF_HAPTIC           TR("Pwr ON/OFF Haptic","Power ON/OFF Haptic")
#define TR_THROTTLE_WARNING            TR("Г-Предупр", "Статис газа")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Свое пол", "Свое пол?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Пол. %", "Положение %")
#define TR_SWITCHWARNING               TR("П-Предупр", "Поз тумбл")
#define TR_POTWARNINGSTATE             "Сост потенц и тумбл"
#define TR_POTWARNING                  TR("Полож пот", "Полож потенц")
#define TR_TIMEZONE                    "Временная зона"
#define TR_ADJUST_RTC                  "Настроить RTC"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Пор. Кн. По Ум.", "Порядок кан по умолч")
#define TR_STICKS                      "Axis"
#define TR_POTS                        "Потенциом"
#define TR_SWITCHES                    "Тумблеры"
#define TR_SWITCHES_DELAY              TR("Зад воспр", "Задерж воспр (средн. полож. тумбл.)")
#define TR_SLAVE                       "Рабочий"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "Множитель"
#define TR_CAL                         "Звонок"
#define TR_CALIBRATION                 "Калибровка"
#define TR_VTRIM                       "Трим - +"
#define TR_CALIB_DONE                  "Калибровка заверш"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Наж [Enter] для прод"
  #define TR_SETMIDPOINT               "Выставить по центру стики/потенц/слайдеры и нажать [Enter]"
  #define TR_MOVESTICKSPOTS            "Перемещ axis/потенц/слайдеры и нажать [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " СТАРТ"
  #define TR_SETMIDPOINT               "ЦЕНТР AXIS/СЛАЙДЕРЫ"
  #define TR_MOVESTICKSPOTS            "Перемещ AXIS/ПОТЕНЦ"
#else
  #define TR_MENUTOSTART               TR_ENTER " СТАРТ"
#if defined(SURFACE_RADIO)
  #define TR_SETMIDPOINT               "ВЫБ СЕРЕД ПОТЕНЦ"
  #define TR_MOVESTICKSPOTS            "ДВИГ ST/TH/POTS/AXIS"
#else
  #define TR_SETMIDPOINT               TR("ВЫБ AXIS СЕРЕД", "ЦЕНТР AXIS/СЛАЙДЕРЫ")
  #define TR_MOVESTICKSPOTS            "ДВИГ AXIS/Потенц"
#endif
  #define TR_MENUWHENDONE              TR_ENTER " ПО ОКОНЧАНИЮ"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      "НЕТ ДАННЫХ"
#define TR_US                          "US"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Макс Tmix"
#define TR_FREE_STACK                  "Свободн стек"
#define TR_INT_GPS_LABEL               "Внутренний GPS"
#define TR_HEARTBEAT_LABEL             "Пульсация"
#define TR_LUA_SCRIPTS_LABEL           "Lua Скрипт"
#define TR_FREE_MEM_LABEL              "Свободно памяти"
#define TR_DURATION_MS             TR("[D]","Длител(ms): ")
#define TR_INTERVAL_MS             TR("[I]","Интерв(ms): ")
#define TR_MEM_USED_SCRIPT         "Скрипт(B): "
#define TR_MEM_USED_WIDGET         "Виджет(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Аудио: "
#define TR_GPS_FIX_YES                 "Фикс: Да"
#define TR_GPS_FIX_NO                  "Фикс: Нет"
#define TR_GPS_SATS                    "Спутники: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Меню: "
#define TR_TIMER_LABEL                 "Таймер"
#define TR_THROTTLE_PERCENT_LABEL      "Газ %"
#define TR_BATT_LABEL                  "АКБ"
#define TR_SESSION                     "Сесия"
#define TR_MENUTORESET                 TR_ENTER " Для сброса"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "Модель"
#if defined(SURFACE_RADIO)
#define TR_FM                          "DM"
#else
#define TR_FM                          "FM"
#endif
#define TR_EEPROMLOWMEM                "EEPROM Нехват памяти"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Нажмите любую кнопку"
#define TR_THROTTLE_NOT_IDLE           "Газ не на ХХ"
#define TR_ALARMSDISABLED              "Аварийные сигналы отключены"
#define TR_PRESSANYKEY                 TR("\010Нажмите любую кнопку", "Нажмите любую кнопку")
#define TR_BADEEPROMDATA               "Неверные данные EEPROM"
#define TR_BAD_RADIO_DATA              "Отсутствуют радиоданные"
#define TR_RADIO_DATA_RECOVERED        TR3("Исп резер настр", "Исп резер настр", "Исп резер настр")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Недейст настр", "Недейств настр", "Не удалось проч настр")
#define TR_EEPROMFORMATTING            "Формат EEPROM"
#define TR_STORAGE_FORMAT              "Формат памяти"
#define TR_EEPROMOVERFLOW              "EEPROM заполнен"
#define TR_RADIO_SETUP                 "НАСТРОЙКА ПУЛЬТА"
#define TR_MENUTRAINER                 "ТРЕНЕР"
#define TR_MENUSPECIALFUNCS            "ГЛОБ ФУНКЦИИ"
#define TR_MENUVERSION                 "ВЕРСИЯ"
#define TR_MENU_RADIO_SWITCHES         TR("ТУМБЛЕРЫ", "ТЕСТ ТУМБЛЕРОВ")
#define TR_MENU_RADIO_ANALOGS          TR("FYFKJUB", "ТЕСТ АНАЛОГОВ")
#define TR_MENU_RADIO_ANALOGS_CALIB    "КАЛИБРОВКА АНАЛОГОВ"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW АНАЛОГИ (5 Hz)"
#define TR_MENUCALIBRATION             "КАЛИБРОВКА"
#define TR_MENU_FSWITCH                "НАСТРАИВАЕМЫЕ ТУМБЛЕРЫ"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Тримы => Субтримы"
#else
  #define TR_TRIMS2OFFSETS             "\006Тримы => Субтримы"
#endif
#define TR_CHANNELS2FAILSAFE           "Каналы=>Авар режим"
#define TR_CHANNEL2FAILSAFE            "Канал=>Авар режим"
#define TR_MENUMODELSEL                TR("ВЫБОР МОДЕЛИ", "УПРАВЛЕНИЕ МОДЕЛЬЮ")
#define TR_MENU_MODEL_SETUP            TR("НАСТРОЙКА", "НАСТРОЙКА МОДЕЛИ")
#if defined(SURFACE_RADIO)
#define TR_MENUFLIGHTMODES             "РЕЖИМЫ ВОЖДЕНИЯ"
#define TR_MENUFLIGHTMODE              "РЕЖИМ ВОЖДЕНИЯ"
#else
#define TR_MENUFLIGHTMODES             "РЕЖИМЫ ПОЛЕТА"
#define TR_MENUFLIGHTMODE              "РЕЖИМ ПОЛЕТА"
#endif
#define TR_MENUHELISETUP               "НАСТРОЙКА ВЕРТ"
  
  #define TR_MENUINPUTS                "ВХОДЫ"
  #define TR_MENULIMITS                "ВЫХОДЫ"
#define TR_MENUCURVES                  "КРИВЫЕ"
#define TR_MENUCURVE                   "КРИВАЯ"
#define TR_MENULOGICALSWITCH           "ЛОГИЧ ТУМБЛ."
#define TR_MENULOGICALSWITCHES         "ЛОГИЧ ТУМБЛ."
#define TR_MENUCUSTOMFUNC              "СПЕЦ ФУНКЦИИ"
#define TR_MENUCUSTOMSCRIPTS           "ПОЛЬЗ СЦЕНАРИИ"
#define TR_MENUTELEMETRY               "ТЕЛЕМЕТРИЯ"
#define TR_MENUSTAT                    "СТАТИСТИКА"
#define TR_MENUDEBUG                   "ОТЛАДКА"
#define TR_MONITOR_CHANNELS1           "МОНИТОР КАН 1-8"
#define TR_MONITOR_CHANNELS2           "МОНИТОР КАН 9-16"
#define TR_MONITOR_CHANNELS3           "МОНИТОР КАН 17-24"
#define TR_MONITOR_CHANNELS4           "МОНИТОР КАН 25-32"
#define TR_MONITOR_SWITCHES            "МОНИТОР ЛОГИЧ ТУМБЛЕРОВ"
#define TR_MONITOR_OUTPUT_DESC         "Выходы"
#define TR_MONITOR_MIXER_DESC          "Микшеры"
  #define TR_RECEIVER_NUM              TR("RxНом", "Номер")
  #define TR_RECEIVER                  "Приемник"
#define TR_MULTI_RFTUNE                TR("Наст частоты", "Наст частоты")
#define TR_MULTI_RFPOWER               "Мощность RF"
#define TR_MULTI_WBUS                  "Выход"
#define TR_MULTI_TELEMETRY             "Телеметрия"
#define TR_MULTI_VIDFREQ               TR("Вид частота", "Вид частота")
#define TR_RF_POWER                    "Мощность RF"
#define TR_MULTI_FIXEDID               TR("Фикс ID", "Фикс ID")
#define TR_MULTI_OPTION                TR("Опция", "Знач опции")
#define TR_MULTI_AUTOBIND              TR("Прив к кан", "Прив к кан")
#define TR_DISABLE_CH_MAP              TR("Откл карту кан", "Откл карту кан")
#define TR_DISABLE_TELEM               TR("Без телеметрии", "Откл телем")
#define TR_MULTI_LOWPOWER              TR("Низк потребл", "Низк потребл")
#define TR_MULTI_LNA_DISABLE           "Откл LNA"
#define TR_MODULE_TELEMETRY            TR("S.Port", "Связь S.Port")
#define TR_MODULE_TELEM_ON             TR("Вкл", "Вкл")
#define TR_DISABLE_INTERNAL            TR("Откл внутр.", "Откл внут RF")
#define TR_MODULE_NO_SERIAL_MODE       TR("!Режим SERIAL", "Не в режиме SERIAL")
#define TR_MODULE_NO_INPUT             TR("Нет ввода", "Нет ввода")
#define TR_MODULE_NO_TELEMETRY         TR3("НЕТ ТЕЛЕМ", "НЕТ ТЕЛЕМЕТРИИ", "ТЕЛЕМЕТРИЯ НЕ ОБНАРУЖЕНА")
#define TR_MODULE_WAITFORBIND          "Привязка для загруж протокола"
#define TR_MODULE_BINDING              TR("Привязка...", "Привязка")
#define TR_MODULE_UPGRADE_ALERT        TR3("Треб обновл", "Треб обновл", "Треб\nобновл модуля")
#define TR_MODULE_UPGRADE              TR("Рекомд обновл", "Рекомд обновл")
#define TR_REBIND                      "Треб повтор привязка"
#define TR_REG_OK                      "Усп регистрация"
#define TR_BIND_OK                     "Усп привязка"
#define TR_BINDING_CH1_8_TELEM_ON      "Кн1-8 Телем Вкл"
#define TR_BINDING_CH1_8_TELEM_OFF     "Кн1-8 Телем Выкл"
#define TR_BINDING_CH9_16_TELEM_ON     "Кн9-16 Телем Вкл"
#define TR_BINDING_CH9_16_TELEM_OFF    "Кн9-16 Телем Выкл"
#define TR_PROTOCOL_INVALID            TR("Недопус протокол", "Протокол недейств")
#define TR_MODULE_STATUS               TR("Статус", "Статус")
#define TR_MODULE_SYNC                 TR("Синхро", "Синхро протокола")
#define TR_MULTI_SERVOFREQ             TR("Частота серво", "Частота серво")
#define TR_MULTI_MAX_THROW             TR("Макс. ход", "Вкл макс. ход")
#define TR_MULTI_RFCHAN                TR("RF Канал", "Выбр RF канал")
#define TR_AFHDS3_RX_FREQ              TR("Частота RX", "Частота RX")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Одноадрес/Телем", "Одноадрес/Телем")
#define TR_AFHDS3_ONE_TO_MANY          "Мультикаст"
#define TR_AFHDS3_ACTUAL_POWER         TR("Акт. мощность", "Фактич мощность")
#define TR_AFHDS3_POWER_SOURCE         TR("Источ питания", "Источ питания")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "FlySky RSSI")
#define TR_GPS_COORDS_FORMAT           TR("Формат коорд GPS", "Формат коорд")
#define TR_VARIO                       TR("Вариометр", "Вариометр")
#define TR_PITCH_AT_ZERO               "Тоны при нуле"
#define TR_PITCH_AT_MAX                "Тоны при макс."
#define TR_REPEAT_AT_ZERO              "Повт при нуле"
#define TR_BATT_CALIB                  TR("Калибр АКБ", "Калибр АКБ")
#define TR_CURRENT_CALIB               "Калибр тока"
#define TR_VOLTAGE                     TR("Напряжение", "Напряжение")
#define TR_SELECT_MODEL                "Выбор модели"
#define TR_MANAGE_MODELS               "УПРАВЛЕНИЕ МОДЕЛЯМИ"
#define TR_MODELS                      "Модели"
#define TR_SELECT_MODE                 "Выбрать модель"
#define TR_CREATE_MODEL                "Создать модель"
#define TR_FAVORITE_LABEL              "Избранное"
#define TR_MODELS_MOVED                "Неисп модели перенес в"
#define TR_NEW_MODEL                   "Новая модель"
#define TR_INVALID_MODEL               "Недопус модель"
#define TR_EDIT_LABELS                 "Редактир метки"
#define TR_LABEL_MODEL                 "Метка модели"
#define TR_MOVE_UP                     "Перемес вверх"
#define TR_MOVE_DOWN                   "Перемес вниз"
#define TR_ENTER_LABEL                 "Ввести метку"
#define TR_LABEL                       "Метка"
#define TR_LABELS                      "Метки"
#define TR_CURRENT_MODEL               "Текущая"
#define TR_ACTIVE                      "Активная"
#define TR_NEW                         "Новая"
#define TR_NEW_LABEL                   "Новая метка"
#define TR_RENAME_LABEL                "Переимен метку"
#define TR_DELETE_LABEL                "Удалить метку"
#define TR_DUPLICATE_MODEL             "Дублир модель"
#define TR_COPY_MODEL                  "Копир модель"
#define TR_MOVE_MODEL                  "Перемес модель"
#define TR_BACKUP_MODEL                "Резер копия модели"
#define TR_DELETE_MODEL                "Удалить модель"
#define TR_RESTORE_MODEL               "Восстан модель"
#define TR_DELETE_ERROR                "Удалить ошибку"
#define TR_SDCARD_ERROR                TR("Ошибка SD карты", "Ошибка SD карты")
#define TR_SDCARD                      "SD карта"
#define TR_NO_FILES_ON_SD              "Нет данных на SD карте!"
#define TR_NO_SDCARD                   "Нет CD карты"
#define TR_WAITING_FOR_RX              "Ожид приемника..."
#define TR_WAITING_FOR_TX              "Ожид передатчика..."
#define TR_WAITING_FOR_MODULE          TR("Ожид модуля", "Ожид модуля...")
#define TR_NO_TOOLS                    "Отсут инструменты"
#define TR_NORMAL                      "Обычный"
#define TR_NOT_INVERTED                "Не инвер"
#define TR_NOT_CONNECTED               TR("!Подключ", "Не подключ");
#define TR_CONNECTED                   "Подключено"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16КН без телем", "16КН без телем")
#define TR_16CH_WITH_TELEMETRY         TR("16КН с телем", "16КН с телем")
#define TR_8CH_WITH_TELEMETRY          TR("8КН с телем", "8КН с телем")
#define TR_EXT_ANTENNA                 "Внеш антенна"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Обнов опции прием?"
#define TR_UPDATE_TX_OPTIONS           "Обнов опции передат?"
#define TR_MODULES_RX_VERSION          "Модули / Версия пульта"
#define TR_SHOW_MIXER_MONITORS         "Показ мониторы миксера"
#define TR_MENU_MODULES_RX_VERSION     "МОДУЛИ / ВЕРСИЯ ПУЛЬТА"
#define TR_MENU_FIRM_OPTIONS           "ОПЦИИ ПО"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Axis/Потенц/Слайдеры"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM Axis/Потенц/Слайдеры"
#define TR_RF_PROTOCOL                 "RF Протокол"
#define TR_MODULE_OPTIONS              "Опции модуля"
#define TR_POWER                       "Питание"
#define TR_NO_TX_OPTIONS               "Отсут опции передат"
#define TR_RTC_BATT                    "RTC АКБ"
#define TR_POWER_METER_EXT             "Изм мощност (Внеш)"
#define TR_POWER_METER_INT             "Изм мощност (Внут)"
#define TR_SPECTRUM_ANALYSER_EXT       "Спек анализ (Внеш)"
#define TR_SPECTRUM_ANALYSER_INT       "Спек анализ (Внут)"
#define TR_SDCARD_FULL                 "SD карта заполнена"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\nЛоги и скриншоты откл"
#else
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036Логи и скриншоты" LCDW_128_LINEBREAK "Откл"
#endif
#define TR_NEEDS_FILE                  "НУЖЕН ФАЙЛ"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Несовмес"
#define TR_WARNING                     "ВНИМАНИЕ"
#define TR_STORAGE_WARNING             "ПАМЯТЬ"
#define TR_THROTTLE_UPPERCASE          "Газ"
#define TR_ALARMSWARN                  "СИГНАЛЫ"
#define TR_SWITCHWARN                  TR("ТУМБЛЕРЫ", "ТУМБЛЕРЫ")
#define TR_FAILSAFEWARN                "СБОЙ БЕЗОПАСНОСТИ"
#define TR_TEST_WARNING                TR("ТЕСТ", "ТЕСТ")
#define TR_TEST_NOTSAFE                "Испол для тестов"
#define TR_WRONG_SDCARDVERSION         TR("Ожид версия: ", "Ожид версия: ")
#define TR_WARN_RTC_BATTERY_LOW        "Низкий заряд АКБ RTC"
#define TR_WARN_MULTI_LOWPOWER         "Режим низ энергопотреб"
#define TR_BATTERY                     "АКБ"
#define TR_WRONG_PCBREV                "Обнаруж неправ версия платы"
#define TR_EMERGENCY_MODE              "РЕЖИМ ЧРЕЗВ СИТУАЦИИ"
#define TR_NO_FAILSAFE                 "Failsafe не настроен"
#define TR_KEYSTUCK                    "Застрявшая клавиша"
#define TR_VOLUME                      "Громкость"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Подсветка"
#define TR_CPU_TEMP                    "Темпер проц"
#define TR_COPROC                      "Сопроцессор"
#define TR_COPROC_TEMP                 "Темпер сопроц"
#define TR_TTL_WARNING                 "Внимание: Не превышать напряжение 3,3 В на выводах TX/RX !"
#define TR_FUNC                        "Функция"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Продолж"
#define TR_DELAY                       "Задержка"
#define TR_SD_CARD                     "SD КАРТА"
#define TR_SDHC_CARD                   "SD-HC КАРТА"
#define TR_NO_SOUNDS_ON_SD             "Нет звуков на SD карте"
#define TR_NO_MODELS_ON_SD             "Нет модели на SD карте"
#define TR_NO_BITMAPS_ON_SD            "Нет изобр на SD карте"
#define TR_NO_SCRIPTS_ON_SD            "Нет скриптов на SD карте"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Синтак ошибка", "Синтак ошибка")
#define TR_SCRIPT_PANIC                "Аварийное заверш сценария"
#define TR_SCRIPT_KILLED               "Сценарий остановлен"
#define TR_SCRIPT_ERROR                "Неизвестная ошибка"
#define TR_PLAY_FILE                   "ВСПР"
#define TR_DELETE_FILE                 "УДАЛИТЬ"
#define TR_COPY_FILE                   "КОПИРОВАТЬ"
#define TR_RENAME_FILE                 "ПЕРЕИМЕНОВАТЬ"
#define TR_ASSIGN_BITMAP               "Назначить изображ"
#define TR_ASSIGN_SPLASH               "Экран приветствия"
#define TR_EXECUTE_FILE                "ЗАПУСТИТЬ"
#define TR_REMOVED                     "Удалено"
#define TR_SD_INFO                     "Инфо"
#define TR_NA                          "Н/Д"
#define TR_HARDWARE                    "АППАРАТУРА"
#define TR_FORMATTING                  "Формат..."
#define TR_TEMP_CALIB                  "Калиб темп"
#define TR_TIME                        "Время"
#define TR_MAXBAUDRATE                 "Макс скорость"
#define TR_BAUDRATE                    "Скор пер"
#define TR_SAMPLE_MODE                 TR("Реж выб","Реж выб")
#define TR_SAMPLE_MODES                "Обыч","Один бит"
#define TR_LOADING                     "Загрузка..."
#define TR_DELETE_THEME                "Удалить тему?"
#define TR_SAVE_THEME                  "Сохранить тему?"
#define TR_EDIT_COLOR                  "Выбор цвета"
#define TR_NO_THEME_IMAGE              "Отсутс изображ темы"
#define TR_BACKLIGHT_TIMER             "Время бездействия"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Быстрый выбор модели"
  #define TR_LABELS_SELECT             "Выбор"
  #define TR_LABELS_MATCH              "Cоответствие"
  #define TR_FAV_MATCH                 "Избранное"
  #define TR_LABELS_SELECT_MODE        "Мультивыбор","Одиноч выбор"
  #define TR_LABELS_MATCH_MODE         "Все","Любой"
  #define TR_FAV_MATCH_MODE            "Совпадает","Не совпадает"
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "Выберите папку шаблона"
#define TR_SELECT_TEMPLATE             "ВЫБЕРИТЕ ШАБЛОН МОДЕЛИ:"
#define TR_NO_TEMPLATES                "В этой папке нет шаблона"
#define TR_SAVE_TEMPLATE               "Сохранить шаблон"
#define TR_BLANK_MODEL                 "Пустая модель"
#define TR_BLANK_MODEL_INFO            "Создайте пустую модель"
#define TR_FILE_EXISTS                 "ФАЙЛ УЖЕ СУЩЕСТВУЕТ"
#define TR_ASK_OVERWRITE               "Вы хотите перезаписать?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Найти"
#define TR_BLUETOOTH_INIT              "Инициал"
#define TR_BLUETOOTH_DIST_ADDR         "Адрес назначения"
#define TR_BLUETOOTH_LOCAL_ADDR        "Локальный адрес"
#define TR_BLUETOOTH_PIN_CODE          "PIN код"
#define TR_BLUETOOTH_NODEVICES         "Устройства не найдены"
#define TR_BLUETOOTH_SCANNING          "Поиск..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Вкл"
#else
#define TR_BLUETOOTH_MODES             "---","Телем","Тренер"
#endif

#define TR_SD_INFO_TITLE               "SD Инфо"
#define TR_SD_SPEED                    "Скорость:"
#define TR_SD_SECTORS                  "Сектора:"
#define TR_SD_SIZE                     "Размер:"
#define TR_TYPE                        "Тип"
#define TR_GLOBAL_VARS                 "Глоб переменные"
#define TR_GVARS                       "ГЛОБ ПЕРЕМ"
#define TR_GLOBAL_VAR                  "Глоб переменная"
#define TR_MENU_GLOBAL_VARS            "ГЛОБ ПЕРЕМЕННЫЕ"
#define TR_OWN                         "Свой"
#define TR_DATE                        "Дата"
#define TR_MONTHS                      { "Янв", "Фев", "Мар", "Апр", "Май", "Июн", "Июл", "Авг", "Сен", "Окт", "Ноя", "Дек" }
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("Режим энкод","Режим энкод")
#define TR_CHANNELS_MONITOR            "МОНИТОР КАНАЛОВ"
#define TR_MIXERS_MONITOR              "МОНИТОР МИКСЕРОВ"
#define TR_PATH_TOO_LONG               "Путь слишком длинный"
#define TR_VIEW_TEXT                   "ПРОСМОТР ТЕКСТА"
#define TR_FLASH_BOOTLOADER            "ПРОШИТЬ ПО"
#define TR_FLASH_DEVICE                TR("Прошить устр","Прошить устр")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Записать S.Port", "Записать устройство S.Port")
#define TR_FLASH_RECEIVER_OTA          "Записать устр через OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Записать устр через внеш OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Записать устр через внут OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Записать через внеш OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Записать через внут OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Записать модуль Bluetooth", "Записать модуль Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Записать блок управл питанием"
#define TR_DEVICE_NO_RESPONSE          TR("Устр не отвеч", "Устр не отвеч")
#define TR_DEVICE_FILE_ERROR           TR("Проблема с файлом", "Проблема с файлом")
#define TR_DEVICE_DATA_REFUSED         TR("Отказ в перед данных", "Отказ в передаче данных")
#define TR_DEVICE_WRONG_REQUEST        TR("Проблема доступа", "Проблема доступа")
#define TR_DEVICE_FILE_REJECTED        TR("Файл отклонен", "Файл отклонен")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Файл имеет неправ подпись", "Файл имеет неправ подпись")
#define TR_CURRENT_VERSION             TR("Текущая версия: ", "Текущая версия: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Зап внут модуль", "Зап внут модуль")
#define TR_FLASH_INTERNAL_MULTI        TR("Зап внут MULTI", "Зап внут MULTI")
#define TR_FLASH_EXTERNAL_MODULE       TR("Зап внеш модуль", "Зап внеш модуль")
#define TR_FLASH_EXTERNAL_MULTI        TR("Зап внеш MULTI", "Зап внеш MULTI")
#define TR_FLASH_EXTERNAL_ELRS         TR("Зап внеш ELRS", "Зап внеш ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Ошибка обнов ПО", "Ошибка обнов ПО")
#define TR_FIRMWARE_UPDATE_SUCCESS     "ПРОШИТО"
#define TR_WRITING                     "Запись..."
#define TR_CONFIRM_FORMAT              "Подтв формат?"
#define TR_INTERNALRF                  "Внут RF"
#define TR_INTERNAL_MODULE             TR("Внут. Модуль", "Внут Модуль")
#define TR_EXTERNAL_MODULE             TR("Внеш. Модуль", "Внеш Модуль")
#define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX требуется обновл"
#define TR_TELEMETRY_DISABLED          "Телем. откл"
#define TR_MORE_OPTIONS_AVAILABLE      "Доступно больше опций"
#define TR_NO_MODULE_INFORMATION       "Нет инфо о модуле"
#define TR_EXTERNALRF                  "Внеш RF"
#define TR_FAILSAFE                    TR("Сбой", "Режим сбоя")
#define TR_FAILSAFESET                 "Настройки сбоя"
#define TR_REG_ID                      "ID регистр."
#define TR_OWNER_ID                    "ID владел."
#define TR_HOLD                        "Удержание"
#define TR_HOLD_UPPERCASE              "УДЕРЖАНИЕ"
#define TR_NONE                        "Отсутствует"
#define TR_NONE_UPPERCASE              "ОТСУТСТВУЕТ"
#define TR_MENUSENSOR                  "Сенсор"
#define TR_POWERMETER_PEAK             "Пиковое"
#define TR_POWERMETER_POWER            "Мощность"
#define TR_POWERMETER_ATTN             "Аттен"
#define TR_POWERMETER_FREQ             "Частота"
#define TR_MENUTOOLS                   "ИНСТРУМЕНТЫ"
#define TR_TURN_OFF_RECEIVER           "Выкл приемник"
#define TR_STOPPING                    "Остановка..."
#define TR_MENU_SPECTRUM_ANALYSER      "АНАЛИЗАТОР СПЕКТРА"
#define TR_MENU_POWER_METER            "ИЗМЕРИТЕЛЬ МОЩНОСТИ"
#define TR_SENSOR                      "СЕНСОР"
#define TR_COUNTRY_CODE                "Код страны"
#define TR_USBMODE                     "Режим USB"
#define TR_JACK_MODE                   "Режим разъема"
#define TR_VOICE_LANGUAGE              "Язык озвучки"
#define TR_UNITS_SYSTEM                "Единицы"
#define TR_UNITS_PPM                   "PPM Един"
#define TR_EDIT                        "Редактировать"
#define TR_INSERT_BEFORE               "Вставить перед"
#define TR_INSERT_AFTER                "Вставить после"
#define TR_COPY                        "Копировать"
#define TR_MOVE                        "Переход"
#define TR_PASTE                       "Вставить"
#define TR_PASTE_AFTER                 "Вставить после"
#define TR_PASTE_BEFORE                "Вставить перед"
#define TR_DELETE                      "Удалить"
#define TR_INSERT                      "Вставить"
#define TR_RESET_FLIGHT                "Сброс сесии"
#define TR_RESET_TIMER1                "Сбросить таймер1"
#define TR_RESET_TIMER2                "Сбросить таймер2"
#define TR_RESET_TIMER3                "Сбросить таймер3"
#define TR_RESET_TELEMETRY             "Сбросить тел-рию"
#define TR_STATISTICS                  "Статистика"
#define TR_ABOUT_US                    "Информация"
#define TR_USB_JOYSTICK                "USB Джойс (HID)"
#define TR_USB_MASS_STORAGE            "USB Карта SD (SD)"
#define TR_USB_SERIAL                  "USB Serial (VCP)"
#define TR_SETUP_SCREENS               "Выбор экранов"
#define TR_MONITOR_SCREENS             "Мониторы"
#define TR_AND_SWITCH                  "AND Тумблер"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Калибровка аналогов"
#define TR_ANADIAGS_FILTRAWDEV         "Отфильтр необработ аналог сигн с отклон"
#define TR_ANADIAGS_UNFILTRAW          "Неотфильтр необработ аналог сигн"
#define TR_ANADIAGS_MINMAX             "Мин., Макс. и Диапазон"
#define TR_ANADIAGS_MOVE               "Двигайте аналоговые датчики!"
#define TR_SPEAKER                     "Динамик"
#define TR_BUZZER                      "Звук сигнал"
#define TR_BYTES                       "Байты"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_MODULE_UNBIND               BUTTON("Unbind")
#define TR_POWERMETER_ATTN_NEEDED     "Требуется аттенюатор"
#define TR_PXX2_SELECT_RX              "Выбор RX"
#define TR_PXX2_DEFAULT                "<По умолчанию>"
#define TR_BT_SELECT_DEVICE            "Выбор устройства"
#define TR_DISCOVER                    "Обнаружить"
#define TR_BUTTON_INIT                 BUTTON("Иниц")
#define TR_WAITING                     "Предупреждение..."
#define TR_RECEIVER_DELETE             "Удалить приемник?"
#define TR_RECEIVER_RESET              "Сбросить приемник?"
#define TR_SHARE                       "Поделиться"
#define TR_BIND                        "Bind"
#define TR_REGISTER                    TR("Рег", "Регистр")
#define TR_MODULE_RANGE                BUTTON(TR("Диап", "Диапазон"))
#define TR_RANGE_TEST                  "Тест диапазона"
#define TR_RECEIVER_OPTIONS            TR("ОПЦИИ ПРИЕМНИКА", "ОПЦИИ ПРИЕМНИКА")
#define TR_RESET_BTN                   BUTTON("Сброс")
#define TR_DEBUG                       "Отладка"
#define TR_KEYS_BTN                    BUTTON("Кноп")
#define TR_ANALOGS_BTN                 BUTTON(TR("Анлг", "Аналоги"))
#define TR_FS_BTN                      BUTTON(TR("Пользов тумблер", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Сенсор не найден"
#define TR_TOUCH_EXIT                  "Нажмите на экран, чтобы выйти"
#define TR_SET                         BUTTON("Установить")
#define TR_TRAINER                     "Тренажер"
#define TR_CHANS                       "Каналы"
#define TR_ANTENNAPROBLEM              "Проблема с антенной TX!"
#define TR_MODELIDUSED                 "ID использован в:"
#define TR_MODELIDUNIQUE               "ID уникален"
#define TR_MODULE                      "Модуль"
#define TR_RX_NAME                     "Rx Имя"
#define TR_TELEMETRY_TYPE              TR("Тип", "Тип")
#define TR_TELEMETRY_SENSORS           "Сенсоры"
#define TR_VALUE                       "Значен"
#define TR_PERIOD                      "Период"
#define TR_INTERVAL                    "Интервал"
#define TR_REPEAT                      "Повтор"
#define TR_ENABLE                      "Включено"
#define TR_DISABLE                     "Выключено"
#define TR_TOPLCDTIMER                 "Верхний таймер LCD"
#define TR_UNIT                        "Единица"
#define TR_TELEMETRY_NEWSENSOR         "Добавить новый"
#define TR_CHANNELRANGE                TR("Диапазон", "Диапазон кан")
#define TR_ANTENNACONFIRM1             "ВНЕШ АНТЕННА"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "Внут","Запрос","На модель","Внутр + Внеш"
#else
  #define TR_ANTENNA_MODES             "Внутр","Запрос","На модель","Внеш"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Исп внут антенну", "Испол внутр антенну")
#define TR_USE_EXTERNAL_ANTENNA        TR("Исп внеш антенну", "Испол внеш антенну")
#define TR_ANTENNACONFIRM2             TR("Проверьте антенну", "Проверьте антенну!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Требует не FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Требует FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Требует EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "Сертифиц ПО"
#define TR_LOWALARM                    "Низкий сигнал"
#define TR_CRITICALALARM               "Критич сигнал"
#define TR_DISABLE_ALARM               TR("Откл сигналы", "Откл сигналы")
#define TR_POPUP                       "Всплыв"
#define TR_MIN                         "Мин"
#define TR_MAX                         "Макс"
#define TR_CURVE_PRESET                "Предустановка..."
#define TR_PRESET                      "Предустановка"
#define TR_MIRROR                      "Зеркало"
#define TR_CLEAR                       "Очистить"
#define TR_RESET                       "Сброс"
#define TR_RESET_SUBMENU               "Сброс..."
#define TR_COUNT                       "Колличество"
#define TR_PT                          "Шт"
#define TR_PTS                         "Штк"
#define TR_SMOOTH                      "Плавно"
#define TR_COPY_STICKS_TO_OFS          TR("Копир стик->субтрим", "Копир axis в субтрим")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Коп мин/макс во все вых", "Коп мин/макс/центр во все вых")
#define TR_COPY_TRIMS_TO_OFS           TR("Коп трим->субтрим", "Коп тримы в субтрим")
#define TR_INCDEC                      "Увел/Умен"
#define TR_GLOBALVAR                   "Глоб переменная"
#define TR_MIXSOURCE                   "Источ (%)"
#define TR_MIXSOURCERAW                "Источ (Значен)"
#define TR_CONSTANT                    "Постоянное значение"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "Выкл","Вкл","Авто"
#define TR_PREFLIGHT                   "Предстарт проверки"
#define TR_CHECKLIST                   TR("Контр список", "Контр список")
#define TR_CHECKLIST_INTERACTIVE       TR3("C-интерактив", "Интерактив checklist", "Интерактивный checklist")
#define TR_AUX_SERIAL_MODE             "Послед порт"
#define TR_AUX2_SERIAL_MODE            "Послед порт 2"
#define TR_AUX_SERIAL_PORT_POWER       "Мощность порта"
#define TR_SCRIPT                      "Скрипт"
#define TR_INPUTS                      "Входные"
#define TR_OUTPUTS                     "Выходные"
#define STR_EEBACKUP                   "EEPROM Резерв копия"
#define STR_FACTORYRESET               "Сброс к завод настройкам"
#define TR_CONFIRMRESET                TR("Удалить все?", "Удалить все?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Много Lua-скриптов!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Режим питания"
#define TR_SPORT_UPDATE_POWER_MODES    "Авто","Вкл"
#define TR_NO_TELEMETRY_SCREENS        "Нет экранов телем"
#define TR_TOUCH_PANEL                 "Сенсорная панель:"
#define TR_FILE_SIZE                   "Размер файла"
#define TR_FILE_OPEN                   "Все равно откр?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Имя"
#define TR_PHASES_HEADERS_SW           "Тумблер"
#define TR_PHASES_HEADERS_RUD_TRIM     "Трим руля"
#define TR_PHASES_HEADERS_ELE_TRIM     "Трим высоты"
#define TR_PHASES_HEADERS_THT_TRIM     "Трим газа"
#define TR_PHASES_HEADERS_AIL_TRIM     "Трим элеронов"
#define TR_PHASES_HEADERS_CH5_TRIM     "Трим 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Трим 6"
#define TR_PHASES_HEADERS_FAD_IN       "Плавн появлен"
#define TR_PHASES_HEADERS_FAD_OUT      "Плавн исчезнов"

#define TR_LIMITS_HEADERS_NAME         "Имя"
#define TR_LIMITS_HEADERS_SUBTRIM      "Субтрим"
#define TR_LIMITS_HEADERS_MIN          "Мин"
#define TR_LIMITS_HEADERS_MAX          "Макс"
#define TR_LIMITS_HEADERS_DIRECTION    "Направление"
#define TR_LIMITS_HEADERS_CURVE        "Кривая"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM Центр"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Режим субтрим"
#define TR_INVERTED                    "Перевернутый"


#define TR_LSW_DESCRIPTIONS            { "Comparison type or function", "First variable", "Second variable or constant", "Second variable or constant", "Additional condition for line to be enabled", "Minimum ON duration of the logical switch", "Minimum TRUE duration for the switch to become ON" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "Первый канал"
  #define TR_FILL_BACKGROUND           "Заполнить фон?"
  #define TR_BG_COLOR                  "Цвет фона"
  #define TR_SLIDERS_TRIMS             "Тумблеры+Трим"
  #define TR_SLIDERS                   "Тумблеры"
  #define TR_FLIGHT_MODE               "Режим полета"
  #define TR_INVALID_FILE              "Недопустимый файл"
  #define TR_TIMER_SOURCE              "Источник таймера"
  #define TR_SIZE                      "Размер"
  #define TR_SHADOW                    "Тень"
  #define TR_ALIGNMENT                 "Вырав"
  #define TR_ALIGN_LABEL               "Вырав метки"
  #define TR_ALIGN_VALUE               "Вырав значения"
  #define TR_ALIGN_OPTS                { "Слева", "По центру", "Справа" }
  #define TR_TEXT                      "Текст"
  #define TR_COLOR                     "Цвет"
  #define TR_MAIN_VIEW_X               "Основное экран X"
  #define TR_PANEL1_BACKGROUND         "Фон панели 1"
  #define TR_PANEL2_BACKGROUND         "Фон панели 2"
  #define TR_WIDGET_GAUGE              "Шкала"
  #define TR_WIDGET_MODELBMP           "Информация о модели"
  #define TR_WIDGET_OUTPUTS            "Выходы"
  #define TR_WIDGET_TEXT               "Текст"
  #define TR_WIDGET_TIMER              "Таймер"
  #define TR_WIDGET_VALUE              "Значение"
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
#elif defined(PCBPL18)
   // Bootloader PL18/NB4+ specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_ENABLE                "Enable"
  #define TR_BL_DISABLE               "Disable"
  #if defined(RADIO_NV14_FAMILY)
    #define TR_BL_SELECT_KEY          "[R TRIM] to select file"
    #define TR_BL_FLASH_KEY           "Hold [R TRIM] long to flash"
    #define TR_BL_ERASE_KEY           "Hold [R TRIM] long to erase"
    #define TR_BL_EXIT_KEY            "[L TRIM] to exit"
  #elif defined(RADIO_NB4P)
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
#elif defined(PCBNV14)
   // Bootloader NV14 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] to select file"
  #define TR_BL_FLASH_KEY             "Hold [R TRIM] long to flash"
  #define TR_BL_EXIT_KEY              " [L TRIM] to exit"
  #define TR_BL_ENABLE                "Enable"
  #define TR_BL_DISABLE               "Disable"
#endif

// About screen
#define TR_ABOUTUS                     TR("ИНФОРМАЦИЯ", "ИНФОРМАЦИЯ")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Громк сигнала"
#define TR_WAV_VOLUME                  "Громкость WAV"
#define TR_BG_VOLUME                   TR("Гром фон зв", "Гром фон зв")

#define TR_TOP_BAR                     "Верх панель"
#define TR_FLASH_ERASE                 "Стирание флеш..."
#define TR_FLASH_WRITE                 "Запись в флеш..."
#define TR_OTA_UPDATE                  "OTA-обновление..."
#define TR_MODULE_RESET                "Сброс модуля..."
#define TR_UNKNOWN_RX                  "Неизвес приёмник"
#define TR_UNSUPPORTED_RX              "Непод приёмник"
#define TR_OTA_UPDATE_ERROR            "Ошибка OTA-обновл"
#define TR_DEVICE_RESET                "Сброс устройства..."
#define TR_ALTITUDE                    "Высота"
#define TR_SCALE                       "Масштаб"
#define TR_VIEW_CHANNELS               "Просмотр каналов"
#define TR_VIEW_NOTES                  "Просмотр заметок"
#define TR_MODEL_SELECT                "Выбор модели"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Точность"
#define TR_RATIO                       "Коэффициент"
#define TR_FORMULA                     "Формула"
#define TR_CELLINDEX                   "Инд. ячейки"
#define TR_LOGS                        "Логи"
#define TR_OPTIONS                     "Опции"
#define TR_FIRMWARE_OPTIONS            "Опции ПО"

#define TR_ALTSENSOR                   "Датчик высот"
#define TR_CELLSENSOR                  "Датчик АКБ"
#define TR_GPSSENSOR                   "Датчик GPS"
#define TR_CURRENTSENSOR               "Датчик тока"
#define TR_AUTOOFFSET                  "Авто смещ"
#define TR_ONLYPOSITIVE                "Положител"
#define TR_FILTER                      "Фильтр"
#define TR_TELEMETRYFULL               TR("Слоты заняты!", "Слоты заняты!")
#define TR_IGNORE_INSTANCE             TR("Нет инстанса", "Игнор инстансы")
#define TR_SHOW_INSTANCE_ID            "Показать ID инстанса"
#define TR_DISCOVER_SENSORS            "Поиск нов датч"
#define TR_STOP_DISCOVER_SENSORS       "Остановить"
#define TR_DELETE_ALL_SENSORS          "Удалить все"
#define TR_CONFIRMDELETE               "Удалить все?"
#define TR_SELECT_WIDGET               "Выбрать виджет"
#define TR_WIDGET_FULLSCREEN           "Полноэкран режим"
#define TR_REMOVE_WIDGET               "Удалить виджет"
#define TR_WIDGET_SETTINGS             "Настройки виджета"
#define TR_REMOVE_SCREEN               "Удалить экран"
#define TR_SETUP_WIDGETS               "Настройка виджетов"
#define TR_USER_INTERFACE              "Пользов интерфейс"
#define TR_THEME                       "Тема"
#define TR_SETUP                       "Настройка"
#define TR_LAYOUT                      "Макет"
#define TR_ADD_MAIN_VIEW               "Добавить экран"
#define TR_TEXT_COLOR                  "Цвет текста"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Входы"
#define TR_MENU_LUA                    STR_CHAR_LUA "Скрипты Lua"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Axis"
#define TR_MENU_POTS                   STR_CHAR_POT "Потенц"
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "Мин"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "Макс"
#define TR_MENU_HELI                   STR_CHAR_CYC "Циклич управл"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Триммеры"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Тумблеры"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Логич тумблеры"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Тренажер"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Каналы"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "Глоб перем"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Телеметрия"
#define TR_MENU_DISPLAY                "ДИСПЛЕЙ"
#define TR_MENU_OTHER                  "Другое"
#define TR_MENU_INVERT                 "Инвертир"
#define TR_AUDIO_MUTE                  TR("Выкл звук", "Выкл звук")
#define TR_JITTER_FILTER               "Фильтр АЦП"
#define TR_DEAD_ZONE                   "Зона нечувств"
#define TR_RTC_CHECK                   TR("Проверка RTC", "Проверка RTC")
#define TR_AUTH_FAILURE                "Ошибка аутентиф"
#define TR_RACING_MODE                 "Режим гонок"

#define TR_USE_THEME_COLOR              "Используйте цвет темы"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Добав все трим в субтрим"
#if !PORTRAIT_LCD
  #define TR_OPEN_CHANNEL_MONITORS      "Монитор откр кан"
#else
  #define TR_OPEN_CHANNEL_MONITORS      "Мон открыт кан"
#endif
#define TR_DUPLICATE                    "Дублировать"
#define TR_ACTIVATE                     "Активация"
#define TR_RED                          "Красная"
#define TR_BLUE                         "Синяяя"
#define TR_GREEN                        "Зеленая"
#define TR_COLOR_PICKER                 "Средство выбора цвета"
#define TR_EDIT_THEME_DETAILS           "Редакт детали темы"
#define TR_THEME_COLOR_DEFAULT          "По умолчанию"
#define TR_THEME_COLOR_PRIMARY1         "ОСНОВНОЙ1"
#define TR_THEME_COLOR_PRIMARY2         "ОСНОВНОЙ2"
#define TR_THEME_COLOR_PRIMARY3         "ОСНОВНОЙ3"
#define TR_THEME_COLOR_SECONDARY1       "ВТОРОСТЕП1"
#define TR_THEME_COLOR_SECONDARY2       "ВТОРОСТЕП2"
#define TR_THEME_COLOR_SECONDARY3       "ВТОРОСТЕП3"
#define TR_THEME_COLOR_FOCUS            "ФОКУС"
#define TR_THEME_COLOR_EDIT             "РЕДАК-ТЬ"
#define TR_THEME_COLOR_ACTIVE           "АКТИВНАЯ"
#define TR_THEME_COLOR_WARNING          "ПРЕД-НИЕ"
#define TR_THEME_COLOR_DISABLED         "ОТКЛ-НАЯ"
#define TR_THEME_COLOR_CUSTOM           "ПОЛ-КАЯ"
#define TR_THEME_CHECKBOX               "Checkbox"
#define TR_THEME_ACTIVE                 "АКТИВНАЯ"
#define TR_THEME_REGULAR                "РЕГ-НАЯ"
#define TR_THEME_WARNING                "ПРЕД-НИЕ"
#define TR_THEME_DISABLED               "ОТКЛ-НАЯ"
#define TR_THEME_EDIT                   "РЕДАК-ТЬ"
#define TR_THEME_FOCUS                  "Фокус"
#define TR_AUTHOR                       "Автор"
#define TR_DESCRIPTION                  "Описание"
#define TR_SAVE                         "Сохранить"
#define TR_CANCEL                       "Отменить"
#define TR_EDIT_THEME                   "РЕДАК ТЕМУ"
#define TR_DETAILS                      "Детали"
#define TR_THEME_EDITOR                 "ТЕМЫ"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Выбор\nМодели"
#define TR_MAIN_MENU_MANAGE_MODELS      "Меню\nМоделей"
#define TR_MAIN_MENU_MODEL_NOTES        "Заметки\nМодели"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Монитор\nКаналов"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Меню\nМодели"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Меню\nПульта"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Меню\nЭкрана"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Сброс\nТел-рии"
#define TR_MAIN_MENU_STATISTICS         "Стат-ка\nПульта"
#define TR_MAIN_MENU_ABOUT_EDGETX       "Инфо\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                TR("Англ","Английский")
#define TR_VOICE_CHINESE                TR("Китайс","Китайский")
#define TR_VOICE_CZECH                  "Чешский"
#define TR_VOICE_DANISH                 "Датский"
#define TR_VOICE_DEUTSCH                TR("Немец","Немецкий")
#define TR_VOICE_DUTCH                  TR("Голланд","Голландский")
#define TR_VOICE_ESPANOL                TR("Испанс","Испанский")
#define TR_VOICE_FRANCAIS               TR("Францу","Французский")
#define TR_VOICE_HUNGARIAN              TR("Венгер","Венгерский")
#define TR_VOICE_ITALIANO               TR("Итальян","Итальянский")
#define TR_VOICE_POLISH                 TR("Польск","Польский")
#define TR_VOICE_PORTUGUES              TR("Порту","Португальск")
#define TR_VOICE_RUSSIAN                "Русский"
#define TR_VOICE_SLOVAK                 TR("Словацк","Словацкий")
#define TR_VOICE_SWEDISH                "Шведский"
#define TR_VOICE_TAIWANESE              TR("Тайван","Тайваньский")
#define TR_VOICE_JAPANESE               "Японский"
#define TR_VOICE_HEBREW                 "Иврит"
#define TR_VOICE_UKRAINIAN              TR("Украинс","Украинский")

#define TR_USBJOYSTICK_LABEL           "USB Джойс"
#define TR_USBJOYSTICK_EXTMODE         "Режим"
#define TR_VUSBJOYSTICK_EXTMODE        "Обычный","Расшир"
#define TR_USBJOYSTICK_SETTINGS        "Настр канала"
#define TR_USBJOYSTICK_IF_MODE         TR("Интерф","Интерф")
#define TR_VUSBJOYSTICK_IF_MODE        "Джойс","Геймпад","Многоз"
#define TR_USBJOYSTICK_CH_MODE         "Режим"
#define TR_VUSBJOYSTICK_CH_MODE        "Нет","Кнопка","Ось","Симул"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Режим Кнопки"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "Обыч","Импульс","Эмул перек","Дельта","Комп"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Норм","Нормально"),TR("Пулс","Пульс"),TR("SWEm","SWEmul"),TR("Делт","Дельта"),TR("КМП","Компаньон")
#define TR_USBJOYSTICK_CH_SWPOS        "Положения"
#define TR_VUSBJOYSTICK_CH_SWPOS       "Push","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Ось"
#define TR_VUSBJOYSTICK_CH_AXIS         "X","Y","Z","rotX","rotY","rotZ","Slider","Dial","Wheel"
#define TR_USBJOYSTICK_CH_SIM          "Симуляция оси"
#define TR_VUSBJOYSTICK_CH_SIM         "Ail","Ele","Rud","Thr","Acc","Brk","Steer","Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Инверсия"
#define TR_USBJOYSTICK_CH_BTNNUM       "Номер кнопки"
#define TR_USBJOYSTICK_BTN_COLLISION   "!Конфликт ном кнопок!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Конфликт осей!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Круг. вырез", "Круглый вырез")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "нет","X-Y, Z-rotX","X-Y, rotX-rotY","X-Y, Z-rotZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Применить изменения"

#define TR_DIGITAL_SERVO          "Серво 333HZ"
#define TR_ANALOG_SERVO           "Серво 50HZ"
#define TR_SIGNAL_OUTPUT          "Выход сигнал"
#define TR_SERIAL_BUS             "Последов шина"
#define TR_SYNC                   "Синхро"

#define TR_ENABLED_FEATURES       "Включ функции"
#define TR_RADIO_MENU_TABS        "Вкладки меню пульта"
#define TR_MODEL_MENU_TABS        "Вкладки меню модели"

#define TR_SELECT_MENU_ALL        "Все"
#define TR_SELECT_MENU_CLR        "Очист"
#define TR_SELECT_MENU_INV        "Перев"

#define TR_SORT_ORDERS            "Имя A-Z","Имя Z-A","Редко испол","Часто испол"
#define TR_SORT_MODELS_BY         "Сортировка моделей"
#define TR_CREATE_NEW             "Создать"

#define TR_MIX_SLOW_PREC          TR("Точн замедл", "Точность замедления")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "Уже есть каталог с таким названием"

#define TR_DATE_TIME_WIDGET       "Дата и время"
#define TR_RADIO_INFO_WIDGET      "Инфо о пульте"
#define TR_LOW_BATT_COLOR         "Разряженная АКБ"
#define TR_MID_BATT_COLOR         "Середина АКБ "
#define TR_HIGH_BATT_COLOR        "Заряженная АКБ"

#define TR_WIDGET_SIZE            "Размер виджета"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
