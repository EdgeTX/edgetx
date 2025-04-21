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

#define TR_MIN_PLURAL2 2
#define TR_MAX_PLURAL2 2
// For this number of minute in the last decimal place singular form is used in
// plural
#define TR_USE_SINGULAR_IN_PLURAL 0
#define TR_USE_PLURAL2_SPECIAL_CASE 0
// If the number of minutes is above this value PLURAL2 is used
#define TR_USE_PLURAL2 INT_MAX

// #define TR_MINUTE_SINGULAR            "minute"
// #define TR_MINUTE_PLURAL1             "minutes"
// #define TR_MINUTE_PLURAL2             "minutes"
#define TR_MINUTE_SINGULAR            "분"
#define TR_MINUTE_PLURAL1             "분"
#define TR_MINUTE_PLURAL2             "분"

// #define TR_OFFON                       "OFF","ON"
// #define TR_MMMINV                      "---","INV"
// #define TR_VBEEPMODE                   "Quiet","Alarm","NoKey","All"
// #define TR_VBLMODE                     "OFF","Keys",TR("Ctrl","Controls"),"Both","ON"
// #define TR_TRNMODE                     "OFF",TR("+=","Add"),TR(":=","Replace")
// #define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_OFFON                      "끔", "켬"
#define TR_MMMINV                     "---", "반전"
#define TR_VBEEPMODE                  "무음", "알람", "키 없음", "모두"
#define TR_VBLMODE                    "꺼짐", "키", TR("제어부", "제어 장치"), "둘 다", "켬"
#define TR_TRNMODE                    "끔", TR("+=", "합치기"), TR(":=", "대체")
#define TR_TRNCHN                     "채널1", "채널2", "채널3", "채널4"

// #define TR_AUX_SERIAL_MODES            "OFF","Telem Mirror","Telemetry In","SBUS Trainer","LUA","CLI","GPS","Debug","SpaceMouse","External module"
// #define TR_SWTYPES                     "None","Toggle","2POS","3POS"
// #define TR_POTTYPES                    "None","Pot",TR("Pot w. det","Pot with detent"),"Slider",TR("Multipos","Multipos Switch"),"Axis X","Axis Y","Switch"
// #define TR_VPERSISTENT                 "OFF","Flight","Manual Reset"
// #define TR_COUNTRY_CODES               TR("US","America"),TR("JP","Japan"),TR("EU","Europe")
// #define TR_USBMODES                    "Ask",TR("Joyst","Joystick"),TR("SDCard","Storage"),"Serial"
// #define TR_JACK_MODES                  "Ask","Audio","Trainer"
#define TR_AUX_SERIAL_MODES           "끔", "텔레미러", "텔레입력", "SBUS 트레이너", "LUA", "CLI", "GPS", "디버그", "SpaceMouse", "외부 모듈"
#define TR_SWTYPES                    "없음", "토글", "2단", "3단"
#define TR_POTTYPES                   "없음", "다이얼", TR("클릭 다이얼", "중심 클릭 다이얼"), "슬라이더", TR("다단", "다단 스위치"), "X축", "Y축", "스위치"
#define TR_VPERSISTENT                "끔", "비행 중", "수동 초기화"
#define TR_COUNTRY_CODES              TR("미국", "미국"), TR("일본", "일본"), TR("유럽", "유럽")
#define TR_USBMODES                   "선택 요청", TR("조이스틱", "조이스틱"), TR("SD카드", "저장소"), "시리얼"
#define TR_JACK_MODES                 "선택 요청", "오디오", "트레이너"

// #define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
// #define TR_MULTI_CUSTOM                "Custom"
// #define TR_VTRIMINC                    TR("Expo","Exponential"),TR("ExFine","Extra Fine"),"Fine","Medium","Coarse"
// #define TR_VDISPLAYTRIMS               "No","Change","Yes"
// #define TR_VBEEPCOUNTDOWN              "Silent","Beeps","Voice","Haptic",TR("B & H","Beeps & Haptic"),TR("V & H","Voice & Haptic")
// #define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
// #define TR_VVARIOCENTER                "Tone","Silent"
// #define TR_CURVE_TYPES                 "Standard","Custom"
#define TR_SBUS_INVERSION_VALUES      "정방향", "반전 안함"
#define TR_MULTI_CUSTOM               "사용자 정의"
#define TR_VTRIMINC                   TR("지수", "지수 증가"), TR("초정밀", "아주 미세"), "미세", "중간", "큼"
#define TR_VDISPLAYTRIMS              "아니오", "변경시", "예"
#define TR_VBEEPCOUNTDOWN             "무음", "삐 소리", "음성", "진동", TR("삐+진동", "삐 소리 + 진동"), TR("음성+진동", "음성 + 진동")
#define TR_COUNTDOWNVALUES            "5초", "10초", "20초", "30초"
#define TR_VVARIOCENTER               "톤", "무음"
#define TR_CURVE_TYPES                "표준", "사용자 정의"

// #define TR_ADCFILTERVALUES             "Global","Off","On"
#define TR_ADCFILTERVALUES             "전역", "끔", "켬"

// #define TR_VCURVETYPE                  "Diff","Expo","Func","Cstm"
// #define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
// #define TR_VMLTPX                      "Add","Multiply","Replace"
// #define TR_VMLTPX2                     "+=","*=",":="
#define TR_VCURVETYPE                 "차이", "지수", "수식", "사용자"
#define TR_VCURVEFUNC                 "---", "x>0", "x<0", "|x|", "f>0", "f<0", "|f|"
#define TR_VMLTPX                     "더하기", "곱하기", "대체"
#define TR_VMLTPX2                    "+=", "*=", ":="

#if LCD_W >= 212
// #define TR_CSWTIMER                  "Timer"
// #define TR_CSWSTICKY                 "Stcky"
// #define TR_CSWSTAY                   "Edge"
  #define TR_CSWTIMER                 "타이머"
  #define TR_CSWSTICKY                "고정"
  #define TR_CSWSTAY                  "변화시"
#else
  // #define TR_CSWTIMER                  "Tim"
  // #define TR_CSWSTICKY                 "Stky"
  // #define TR_CSWSTAY                   "Edge"
  #define TR_CSWTIMER                 "타이머"
  #define TR_CSWSTICKY                "고정"
  #define TR_CSWSTAY                  "변화"
#endif

// #define TR_CSWEQUAL                "a=x"
// #define TR_VCSWFUNC                "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY
#define TR_CSWEQUAL                   "a=x"
#define TR_VCSWFUNC                   "---", TR_CSWEQUAL, "a~x", "a>x", "a<x", "|a|>x", "|a|<x", "AND", "OR", "XOR", "변화시", "a=b", "a>b", "a<b", "Δ≥x", "|Δ|≥x", "타이머", "유지"

// #define TR_SF_TRAINER                  "Trainer"
// #define TR_SF_INST_TRIM                "Inst. Trim"
// #define TR_SF_RESET                    "Reset"
// #define TR_SF_SET_TIMER                "Set"
// #define TR_SF_VOLUME                   "Volume"
// #define TR_SF_FAILSAFE                 "SetFailsafe"
// #define TR_SF_RANGE_CHECK              "RangeCheck"
// #define TR_SF_MOD_BIND                 "ModuleBind"
// #define TR_SF_RGBLEDS                  "RGB leds"
#define TR_SF_TRAINER                 "트레이너"
#define TR_SF_INST_TRIM               "즉시 트림"
#define TR_SF_RESET                   "초기화"
#define TR_SF_SET_TIMER               "타이머 설정"
#define TR_SF_VOLUME                  "볼륨"
#define TR_SF_FAILSAFE                "페일세이프 설정"
#define TR_SF_RANGE_CHECK             "거리체크"
#define TR_SF_MOD_BIND                "모듈 바인딩"
#define TR_SF_RGBLEDS                 "RGB LED"

// #define TR_SOUND                       "Play Sound"
// #define TR_PLAY_TRACK                  TR("Ply Trk", "Play Track")
// #define TR_PLAY_VALUE                  TR("Play Val","Play Value")
// #define TR_SF_HAPTIC                   "Haptic"
// #define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua Script")
// #define TR_SF_BG_MUSIC                 "BgMusic"
// #define TR_SF_BG_MUSIC_PAUSE           "BgMusic ||"
// #define TR_SF_LOGS                     "SD Logs"
// #define TR_ADJUST_GVAR                 "Adjust"
// #define TR_SF_BACKLIGHT                "Backlight"
// #define TR_SF_VARIO                    "Vario"
// #define TR_SF_TEST                     "Test"
#define TR_SOUND                      "소리 재생"
#define TR_PLAY_TRACK                 TR("재생", "트랙 재생")
#define TR_PLAY_VALUE                 TR("값 재생", "값 재생")
#define TR_SF_HAPTIC                  "진동"
#define TR_SF_PLAY_SCRIPT             TR("Lua", "Lua 스크립트 실행")
#define TR_SF_BG_MUSIC                "배경 음악"
#define TR_SF_BG_MUSIC_PAUSE          "배경 음악 일시정지"
#define TR_SF_LOGS                    "SD 로그 저장"
#define TR_ADJUST_GVAR                "전역 변수 조정"
#define TR_SF_BACKLIGHT               "백라이트"
#define TR_SF_VARIO                   "바리오"
#define TR_SF_TEST                    "테스트"

#if LCD_W >= 212
// #define TR_SF_SAFETY "Override"
#define TR_SF_SAFETY "출력 오버라이드"
#else
#define TR_SF_SAFETY "Overr."
#endif

// #define TR_SF_SCREENSHOT               "Screenshot"
// #define TR_SF_RACING_MODE              "RacingMode"
// #define TR_SF_DISABLE_TOUCH            "No Touch"
// #define TR_SF_DISABLE_AUDIO_AMP        "Audio Amp Off"
// #define TR_SF_SET_SCREEN               "Set Main Screen"
// #define TR_SF_SET_TELEMSCREEN          "Set Screen"
// #define TR_SF_PUSH_CUST_SWITCH         "Push CS"
// #define TR_SF_LCD_TO_VIDEO             "LCD to Video"
#define TR_SF_SCREENSHOT              "스크린샷 저장"
#define TR_SF_RACING_MODE             "레이싱 모드"
#define TR_SF_DISABLE_TOUCH           "터치 비활성화"
#define TR_SF_DISABLE_AUDIO_AMP       "오디오 앰프 끄기"
#define TR_SF_SET_SCREEN              "메인 화면 설정"
#define TR_SF_SET_TELEMSCREEN         "텔레 화면 설정"
#define TR_SF_PUSH_CUST_SWITCH        "커스텀 스위치 실행"
#define TR_SF_LCD_TO_VIDEO            "LCD → 비디오 출력"

// #define TR_FSW_RESET_TELEM             TR("Telm", "Telemetry")
// #define TR_FSW_RESET_TRIMS             "Trims"
// #define TR_FSW_RESET_TIMERS            "Tmr1","Tmr2","Tmr3"
#define TR_FSW_RESET_TELEM            TR("텔레", "텔레메트리")
#define TR_FSW_RESET_TRIMS            "트림"
#define TR_FSW_RESET_TIMERS           "타이머1", "타이머2", "타이머3"

// #define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("All","Flight"),TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS
#define TR_VFSWRESET                  TR_FSW_RESET_TIMERS, TR("전체", "비행"), TR_FSW_RESET_TELEM, TR_FSW_RESET_TRIMS

// #define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")
#define TR_FUNCSOUNDS                 TR("삐1", "삐 소리1"), TR("삐2", "삐 소리2"), TR("삐3", "삐 소리3"), TR("경고1", "경고음1"), TR("경고2", "경고음2"), TR("찍", "찍찍"), TR("따다", "따따다"), "틱", TR("사이렌", "사이렌"), "벨소리", TR("공상", "공상과학"), TR("로봇", "로봇음"), TR("짹", "짹짹"), "따다~", TR("귀뚜", "귀뚜라미"), TR("알람", "알람시계")

// #define LENGTH_UNIT_IMP                "ft"
// #define SPEED_UNIT_IMP                 "mph"
// #define LENGTH_UNIT_METR               "m"
// #define SPEED_UNIT_METR                "kmh"
#define LENGTH_UNIT_IMP               "피트"
#define SPEED_UNIT_IMP                "마일/시"
#define LENGTH_UNIT_METR              "미터"
#define SPEED_UNIT_METR               "킬로미터"

// #define TR_VUNITSSYSTEM                "Metric",TR("Imper.","Imperial")
#define TR_VUNITSSYSTEM               "미터법", TR("영국식", "영국 단위계")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"
#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

// #define TR_VTELEMSCREENTYPE            "None","Nums","Bars","Script"
// #define TR_GPSFORMAT                   "DMS","NMEA"
#define TR_VTELEMSCREENTYPE           "없음", "숫자", "막대", "스크립트"
#define TR_GPSFORMAT                  "도/분/초", "NMEA"

// #define TR_VSWASHTYPE                  "---","120","120X","140","90"
// #define TR_STICK_NAMES0                "Rud"
// #define TR_STICK_NAMES1                "Ele"
// #define TR_STICK_NAMES2                "Thr"
// #define TR_STICK_NAMES3                "Ail"
// #define TR_SURFACE_NAMES0              "ST"
// #define TR_SURFACE_NAMES1              "TH"
#define TR_VSWASHTYPE                 "없음", "120", "120X", "140", "90"
#define TR_STICK_NAMES0               "요"
#define TR_STICK_NAMES1               "피치"
#define TR_STICK_NAMES2               "스로틀"
#define TR_STICK_NAMES3               "에일러론"
#define TR_SURFACE_NAMES0             "스티어링"
#define TR_SURFACE_NAMES1             "스로틀"

#if defined(PCBNV14)
// #define  TR_RFPOWER_AFHDS2             "Default","High"
#define  TR_RFPOWER_AFHDS2             "기본","높음"
#endif

// #define TR_ROTARY_ENCODERS
// #define TR_ROTENC_SWITCHES
#define TR_ROTARY_ENCODERS            "로터리 인코더"
#define TR_ROTENC_SWITCHES            "인코더 스위치"

// #define TR_ON_ONE_SWITCHES             "ON","One"
#define TR_ON_ONE_SWITCHES            "개별", "하나로"


#if defined(USE_HATS_AS_KEYS)
// #define TR_HATSMODE                    "Hats mode"
// #define TR_HATSOPT                     "Trims only","Keys only","Switchable","Global"
// #define TR_HATSMODE_TRIMS              "Hats mode: Trims"
// #define TR_HATSMODE_KEYS               "Hats mode: Keys"
/* #define TR_HATSMODE_KEYS_HELP          "Left side:\n"\
                                       "   Right = MDL\n"\
                                       "   Up = SYS\n"\
                                       "   Down = TELE\n"\
                                       "\n"\
                                       "Right side:\n"\
                                       "   Left = PAGE<\n"\
                                       "   Right = PAGE>\n"\
                                       "   Up = PREV/INC\n"\
                                       "   Down = NEXT/DEC" */
#define TR_HATSMODE                    "Hat 스위치 모드"
#define TR_HATSOPT                     "트림 전용","키 전용","전환 가능","전역 설정"
#define TR_HATSMODE_TRIMS              "Hat 모드: 트림"
#define TR_HATSMODE_KEYS               "Hat 모드: 키"
#define TR_HATSMODE_KEYS_HELP          "왼쪽Hat\n"\
                                       "   오른쪽 = 모델\n"\
                                       "   위 = 시스템\n"\
                                       "   아래래 = 텔레메트리\n"\
                                       "\n"\
                                       "오른쪽Hat:\n"\
                                       "   왼쪽 = 이전 페이지\n"\
                                       "   오른쪽 = 다음 페이지\n"\
                                       "   위 = 이전/증가\n"\
                                       "   아래 = 다음/감소"
#endif

#if defined(COLORLCD)
  // #define TR_ROTARY_ENC_OPT         "Normal","Inverted"
  #define TR_ROTARY_ENC_OPT           "정방향", "역방향"
#else
  // #define TR_ROTARY_ENC_OPT         "Normal","Inverted","V-I H-N","V-I H-A","V-N E-I"
  #define TR_ROTARY_ENC_OPT           "정방향", "역방향", "수직-정, 수평-정", "수직-정, 수평-역", "수직-역역, 수평-정"
#endif

#if defined(IMU)
  // #define TR_IMU_VSRCRAW               "TltX","TltY",
  #define TR_IMU_VSRCRAW              "기울기X", "기울기Y"
#else
  #define TR_IMU_VSRCRAW
#endif

#if defined(HELI)
  // #define TR_CYC_VSRCRAW               "CYC1","CYC2","CYC3"
  #define TR_CYC_VSRCRAW              "사이클릭1", "사이클릭2", "사이클릭3"
#else
  #define TR_CYC_VSRCRAW               "[C1]","[C2]","[C3]"
#endif


// #define TR_SRC_BATT                    "Batt"
// #define TR_SRC_TIME                    "Time"
// #define TR_SRC_GPS                     "GPS"
// #define TR_SRC_TIMER                   "Tmr"
#define TR_SRC_BATT                   "배터리"
#define TR_SRC_TIME                   "시간"
#define TR_SRC_GPS                    "GPS"
#define TR_SRC_TIMER                  "타이머"


// #define TR_VTMRMODES                   "OFF","ON","Strt","THs","TH%","THt"
#define TR_VTMRMODES                  "끔", "켬", "시작", "스로틀 스틱", "스로틀 비율", "스로틀 시간"

// #define TR_VTRAINER_MASTER_OFF         "OFF"
// #define TR_VTRAINER_MASTER_JACK        "Master/Jack"
// #define TR_VTRAINER_SLAVE_JACK         "Slave/Jack"
// #define TR_VTRAINER_MASTER_SBUS_MODULE "Master/SBUS Module"
// #define TR_VTRAINER_MASTER_CPPM_MODULE "Master/CPPM Module"
// #define TR_VTRAINER_MASTER_BATTERY     "Master/Serial"
// #define TR_VTRAINER_BLUETOOTH          "Master/" TR("BT","Bluetooth"), "Slave/" TR("BT","Bluetooth")
// #define TR_VTRAINER_MULTI              "Master/Multi"
// #define TR_VTRAINER_CRSF               "Master/CRSF"
#define TR_VTRAINER_MASTER_OFF          "끔"
#define TR_VTRAINER_MASTER_JACK         "마스터/잭"
#define TR_VTRAINER_SLAVE_JACK          "슬레이브/잭"
#define TR_VTRAINER_MASTER_SBUS_MODULE  "마스터/SBUS 모듈"
#define TR_VTRAINER_MASTER_CPPM_MODULE  "마스터/CPPM 모듈"
#define TR_VTRAINER_MASTER_BATTERY      "마스터/시리얼"
#define TR_VTRAINER_BLUETOOTH           "마스터/" TR("BT","블루투스"), "슬레이브/" TR("BT","블루투스")
#define TR_VTRAINER_MULTI               "마스터/멀티"
#define TR_VTRAINER_CRSF                "마스터/CRSF"

// #define TR_VFAILSAFE                   "Not set","Hold","Custom","No pulses","Receiver"
// #define TR_VSENSORTYPES                "Custom","Calculated"
// #define TR_VFORMULAS                   "Add","Average","Min","Max","Multiply","Totalize","Cell","Consumpt","Distance"
// #define TR_VPREC                       "0.--","0.0 ","0.00"
// #define TR_VCELLINDEX                  "Lowest","1","2","3","4","5","6","7","8","Highest","Delta"
// #define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center only)","= (symmetrical)"
// #define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")
// #define TR_PPMUNIT                     "0.--","0.0","us"
#define TR_VFAILSAFE                    "설정 안됨", "고정", "사용자 지정", "신호 없음", "수신기 설정"
#define TR_VSENSORTYPES                 "사용자 정의", "계산됨"
#define TR_VFORMULAS                    "더하기", "평균", "최소값", "최대값", "곱하기", "합계", "셀", "소모량", "거리"
#define TR_VPREC                        "0.--", "0.0", "0.00"
#define TR_VCELLINDEX                   "최저", "1", "2", "3", "4", "5", "6", "7", "8", "최고", "차이"
#define TR_SUBTRIMMODES                 STR_CHAR_DELTA "(센터만)", "= (대칭)"
#define TR_TIMER_DIR                    TR("남은 시간", "잔여 시간 표시"), TR("경과 시간", "경과 시간 표시")
#define TR_PPMUNIT                      "0.--", "0.0", "μs"

#if defined(COLORLCD)
#if defined(BOLD)
// #define TR_FONT_SIZES                  "STD"
#define TR_FONT_SIZES                   "표준"
#else
// #define TR_FONT_SIZES                  "STD","BOLD","XXS","XS","L","XL","XXL"
#define TR_FONT_SIZES                   "표준", "굵게", "아주 작게", "작게", "크게", "아주 크게", "매우 크게"
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
  // #define TR_EXIT                      "RTN"
  #define TR_EXIT                       "뒤로"
  #define TR_OK                        TR_ENTER
#else
  // #define TR_EXIT                      "EXIT"
  // #define TR_OK                        TR("\010" "\010" "\010" "[OK]", "\010" "\010" "\010" "\010" "\010" "[OK]")
  #define TR_EXIT                       "나가기"
  #define TR_OK                         "[확인]"
#endif

#if defined(PCBTARANIS)
  // #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
  #define TR_POPUPS_ENTER_EXIT        "나가기 <-> 확인"
#else
  // #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
  #define TR_POPUPS_ENTER_EXIT        "확인 <-> 나가기"
#endif

// #define TR_FREE                        "free"
// #define TR_YES                         "Yes"
// #define TR_NO                          "No"
// #define TR_DELETEMODEL                 "DELETE MODEL"
// #define TR_COPYINGMODEL                "Copying model..."
// #define TR_MOVINGMODEL                 "Moving model..."
// #define TR_LOADINGMODEL                "Loading model..."
// #define TR_UNLABELEDMODEL              "Unlabeled"
// #define TR_NAME                        "Name"
// #define TR_MODELNAME                   "Model name"
// #define TR_PHASENAME                   "Mode name"
// #define TR_MIXNAME                     "Mix name"
// #define TR_INPUTNAME                   TR("Input", "Input name")
// #define TR_EXPONAME                    TR("Name", "Line name")
// #define TR_BITMAP                      "Model image"
// #define TR_NO_PICTURE                  "No Picture"
#define TR_FREE                       "사용 가능"
#define TR_YES                        "예"
#define TR_NO                         "아니오"
#define TR_DELETEMODEL                "모델 삭제"
#define TR_COPYINGMODEL               "모델 복사 중..."
#define TR_MOVINGMODEL                "모델 이동 중..."
#define TR_LOADINGMODEL               "모델 불러오는 중..."
#define TR_UNLABELEDMODEL             "이름 없음"
#define TR_NAME                       "이름"
#define TR_MODELNAME                  "모델 이름"
#define TR_PHASENAME                  "모드 이름"
#define TR_MIXNAME                    "믹스 이름"
#define TR_INPUTNAME                  TR("입력", "입력 이름")
#define TR_EXPONAME                   TR("이름", "라인 이름")
#define TR_BITMAP                     "모델 이미지"
#define TR_NO_PICTURE                 "이미지 없음"

// #define TR_TIMER                       TR("Timer", "Timer ")
// #define TR_START                       "Start"
// #define TR_ELIMITS                     TR("E.Limits", "Extended limits")
// #define TR_ETRIMS                      TR("E.Trims", "Extended trims")
// #define TR_TRIMINC                     "Trim Step"
// #define TR_DISPLAY_TRIMS               TR("Show Trims", "Display trims")
// #define TR_TTRACE                      TR("T-Source", "Source")
// #define TR_TTRIM                       TR("T-Trim-Idle", "Trim idle only")
// #define TR_TTRIM_SW                    TR("T-Trim-Sw", "Trim switch")
// #define TR_BEEPCTR                     TR("Ctr Beep", "Beep when centered")
// #define TR_USE_GLOBAL_FUNCS            TR("Glob.Funcs", "Use global funcs")
#define TR_TIMER                      TR("타이머", "타이머 ")
#define TR_START                      "시작"
#define TR_ELIMITS                    TR("확장 제한", "익스텐디드 리밋")
#define TR_ETRIMS                     TR("확장 트림", "익스텐디드 트림")
#define TR_TRIMINC                    "트림 단계"
#define TR_DISPLAY_TRIMS              TR("트림 표시", "트림 상태 표시")
#define TR_TTRACE                     TR("트림 소스", "소스")
#define TR_TTRIM                      TR("유휴 트림", "유휴 시 트림")
#define TR_TTRIM_SW                   TR("트림 스위치", "트림 스위치")
#define TR_BEEPCTR                    TR("중앙 알림", "중앙 위치에서 비프")
#define TR_USE_GLOBAL_FUNCS           TR("글로벌 기능", "전역 기능 사용")

// #define TR_PROTOCOL                    TR("Proto", "Protocol")
//   #define TR_PPMFRAME                  "PPM frame"
//   #define TR_REFRESHRATE               TR("Refresh", "Refresh rate")
//   #define STR_WARN_BATTVOLTAGE         TR("Output is VBAT: ", "Warning: output level is VBAT: ")
// #define TR_WARN_5VOLTS                 "Warning: output level is 5 volts"
// #define TR_MS                          "ms"
#define TR_PROTOCOL                   TR("프로토콜", "통신 방식")
#define TR_PPMFRAME                   "PPM 프레임"
#define TR_REFRESHRATE                TR("갱신", "갱신 속도")
#define STR_WARN_BATTVOLTAGE          TR("출력은 VBAT: ", "경고: 출력 전압은 VBAT입니다")
#define TR_WARN_5VOLTS                "경고: 출력 전압이 5볼트입니다"
#define TR_MS                         "ms"

// #define TR_SWITCH                      "Switch"
// #define TR_FUNCTION_SWITCHES           "Customizable Switches"
// #define TR_FS_COLOR_LIST               "Custom","Off","White","Red","Green","Yellow","Orange","Blue","Pink"
// #define TR_GROUP                       "Group"
// #define TR_GROUP_ALWAYS_ON             "Always on"
// #define TR_FS_ON_COLOR                 TR("ON:","ON Color")
// #define TR_FS_OFF_COLOR                TR("OFF:","OFF Color")
// #define TR_GROUPS                      "Always on groups"
// #define TR_LAST                        "Last"
// #define TR_MORE_INFO                   "More info"
// #define TR_SWITCH_TYPE                 "Type"
// #define TR_SWITCH_STARTUP              "Startup"
// #define TR_SWITCH_GROUP                "Group"
// #define TR_FUNCTION_SWITCH_GROUPS      "---", TR_SWITCH_GROUP" 1", TR_SWITCH_GROUP" 2", TR_SWITCH_GROUP" 3"
// #define TR_SF_SWITCH                   "Trigger"
// #define TR_TRIMS                       "Trims"
// #define TR_FADEIN                      "Fade in"
// #define TR_FADEOUT                     "Fade out"
// #define TR_DEFAULT                     "(default)"
#define TR_SWITCH                         "스위치"
#define TR_FUNCTION_SWITCHES              "사용자 설정 스위치"
#define TR_FS_COLOR_LIST                  "사용자 정의", "끄기", "흰색", "빨강", "초록", "노랑", "주황", "파랑", "분홍"
#define TR_GROUP                          "그룹"
#define TR_GROUP_ALWAYS_ON                "항상 켜짐"
#define TR_FS_ON_COLOR                    TR("켜짐:", "켜짐 색상")
#define TR_FS_OFF_COLOR                   TR("꺼짐:", "꺼짐 색상")
#define TR_GROUPS                         "항상 켜짐 그룹"
#define TR_LAST                           "마지막"
#define TR_MORE_INFO                      "자세히 보기"
#define TR_SWITCH_TYPE                    "유형"
#define TR_SWITCH_STARTUP                 "시작 시 상태"
#define TR_SWITCH_GROUP                   "스위치 그룹"
#define TR_FUNCTION_SWITCH_GROUPS         "---", "스위치 그룹 1", "스위치 그룹 2", "스위치 그룹 3"
#define TR_SF_SWITCH                      "트리거"
#define TR_TRIMS                          "트림"
#define TR_FADEIN                         "페이드 인"
#define TR_FADEOUT                        "페이드 아웃"
#define TR_DEFAULT                        "(기본값)"

#if defined(COLORLCD)
  // #define TR_CHECKTRIMS                "Check FM Trims"
  #define TR_CHECKTRIMS                   "FM 트림 확인"
#else
  // #define TR_CHECKTRIMS                "\006Check\012trims"
  #define TR_CHECKTRIMS                 "\006트림\012확인"
#endif
// #define TR_SWASHTYPE                   "Swash Type"
// #define TR_COLLECTIVE                  TR("Collective", "Coll. pitch source")
// #define TR_AILERON                     TR("Lateral cyc.", "Lateral cyc. source")
// #define TR_ELEVATOR                    TR("Long. cyc.", "Long. cyc. source")
// #define TR_SWASHRING                   "Swash Ring"
#define TR_SWASHTYPE                    "스와시 타입"
#define TR_COLLECTIVE                   TR("콜렉티브", "집단 피치 소스")
#define TR_AILERON                      TR("에일러론", "측면 싸이클릭 소스")
#define TR_ELEVATOR                     TR("엘리베이터", "전후 싸이클릭 소스")
#define TR_SWASHRING                    "스와시 링"

// #define TR_MODE                        "Mode"
#define TR_MODE                         "모드"
#if !PORTRAIT_LCD
  // #define TR_LEFT_STICK                "Left"
  #define TR_LEFT_STICK                 "왼쪽 스틱"
#else
  // #define TR_LEFT_STICK                "Left"
  #define TR_LEFT_STICK                 "왼쪽 스틱"
#endif

// #define TR_SUBTYPE                     "Subtype"
// #define TR_NOFREEEXPO                  "No free expo!"
// #define TR_NOFREEMIXER                 "No free mixer!"
// #define TR_SOURCE                       "Source"
// #define TR_WEIGHT                      "Weight"
// #define TR_SIDE                        "Side"
// #define TR_OFFSET                       "Offset"
// #define TR_TRIM                        "Trim"
// #define TR_DREX                        "DRex"
// #define DREX_CHBOX_OFFSET              30
// #define TR_CURVE                       "Curve"
// #define TR_FLMODE                      TR("Mode", "Modes")
// #define TR_MIXWARNING                  "Warning"
// #define TR_OFF                         "OFF"
// #define TR_ANTENNA                     "Antenna"
// #define TR_NO_INFORMATION              TR("No info", "No information")
// #define TR_MULTPX                      "Multiplex"
// #define TR_DELAYDOWN                   TR("Delay dn", "Delay down")
// #define TR_DELAYUP                     "Delay up"
// #define TR_SLOWDOWN                    TR("Slow dn", "Slow down")
// #define TR_SLOWUP                      "Slow up"
// #define TR_MIXES                       "MIXES"
// #define TR_CV                          "CV"
#define TR_SUBTYPE                        "하위 유형"
#define TR_NOFREEEXPO                     "사용 가능한 Expo 없음!"
#define TR_NOFREEMIXER                    "사용 가능한 믹서 없음!"
#define TR_SOURCE                         "소스"
#define TR_WEIGHT                         "가중치"
#define TR_SIDE                           "방향"
#define TR_OFFSET                         "오프셋"
#define TR_TRIM                           "트림"
#define TR_DREX                           "듀얼레이트/익스포"
#define DREX_CHBOX_OFFSET                 30
#define TR_CURVE                          "커브"
#define TR_FLMODE                         TR("모드", "비행 모드")
#define TR_MIXWARNING                     "경고"
#define TR_OFF                            "끔"
#define TR_ANTENNA                        "안테나"
#define TR_NO_INFORMATION                 TR("정보 없음", "데이터 없음")
#define TR_MULTPX                         "믹스 방식"
#define TR_DELAYDOWN                      TR("내림 지연", "하강 지연")
#define TR_DELAYUP                        "상승 지연"
#define TR_SLOWDOWN                       TR("느리게 내림", "하강 속도 조절")
#define TR_SLOWUP                         "느리게 올림"
#define TR_MIXES                          "믹스"
#define TR_CV                             "CV"

#if defined(PCBNV14) || defined(PCBPL18)
#define TR_GV                          "GV"
#else
#define TR_GV                          TR("G", "GV")
#endif

// #define TR_RANGE                       "Range"
// #define TR_CENTER                      "Center"
// #define TR_ALARM                       "Alarm"
// #define TR_BLADES                      "Blades/Poles"
// #define TR_SCREEN                      "Screen\001"
// #define TR_SOUND_LABEL                 "Sound"
// #define TR_LENGTH                      "Length"
// #define TR_BEEP_LENGTH                 "Beep length"
// #define TR_BEEP_PITCH                  "Beep pitch"
// #define TR_HAPTIC_LABEL                "Haptic"
// #define TR_STRENGTH                    "Strength"
// #define TR_IMU_LABEL                   "IMU"
// #define TR_IMU_OFFSET                  "Offset"
// #define TR_IMU_MAX                     "Max"
// #define TR_CONTRAST                    "Contrast"
// #define TR_ALARMS_LABEL                "Alarms"
#define TR_RANGE                          "범위"
#define TR_CENTER                         "중앙"
#define TR_ALARM                          "알람"
#define TR_BLADES                         "블레이드/극수"
#define TR_SCREEN                         "화면"
#define TR_SOUND_LABEL                    "소리"
#define TR_LENGTH                         "길이"
#define TR_BEEP_LENGTH                    "비프 길이"
#define TR_BEEP_PITCH                     "비프 음높이"
#define TR_HAPTIC_LABEL                   "진동"
#define TR_STRENGTH                       "세기"
#define TR_IMU_LABEL                      "IMU"
#define TR_IMU_OFFSET                     "오프셋"
#define TR_IMU_MAX                        "최대"
#define TR_CONTRAST                       "명암"
#define TR_ALARMS_LABEL                   "알람"

// #define TR_BATTERY_RANGE               TR("Batt. range", "Battery meter range")
// #define TR_BATTERYCHARGING             "Charging..."
// #define TR_BATTERYFULL                 "Battery full"
// #define TR_BATTERYNONE                 "None!"
// #define TR_BATTERYWARNING              "Battery low"
// #define TR_INACTIVITYALARM             "Inactivity"
// #define TR_MEMORYWARNING               "Memory low"
// #define TR_ALARMWARNING                "Sound off"
// #define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI shutdown", "Check RSSI on shutdown")
// #define TR_TRAINER_SHUTDOWN_ALARM      TR("Trainer shutdown", "Check trainer on shutdown")
// #define TR_MODEL_STILL_POWERED         "Model still powered"
// #define TR_TRAINER_STILL_CONNECTED     TR("Trainer still on","Trainer still connected")
// #define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_BATTERY_RANGE                  TR("배터리 범위", "배터리 측정 범위")
#define TR_BATTERYCHARGING                "충전 중..."
#define TR_BATTERYFULL                    "배터리 충전 완료"
#define TR_BATTERYNONE                    "배터리 없음!"
#define TR_BATTERYWARNING                 "배터리 부족"
#define TR_INACTIVITYALARM                "무작동 경고"
#define TR_MEMORYWARNING                  "메모리 부족"
#define TR_ALARMWARNING                   "소리 꺼짐"
#define TR_RSSI_SHUTDOWN_ALARM            TR("RSSI 상태 확인", "종료 시 RSSI 확인")
#define TR_TRAINER_SHUTDOWN_ALARM         TR("트레이너 상태 확인", "종료 시 트레이너 확인")
#define TR_MODEL_STILL_POWERED            "모델 전원이 켜져 있음"
#define TR_TRAINER_STILL_CONNECTED        TR("트레이너 연결됨", "트레이너 아직 연결됨")
#define TR_USB_STILL_CONNECTED            "USB 연결 중"

// #define TR_MODEL_SHUTDOWN              "Shutdown?"
// #define TR_PRESS_ENTER_TO_CONFIRM      "Press enter to confirm"
#define TR_MODEL_SHUTDOWN                 "모델을 종료할까요?"
#define TR_PRESS_ENTER_TO_CONFIRM         "ENTER를 눌러 확인"

// #define TR_THROTTLE_LABEL              "Throttle"
// #define TR_THROTTLE_START              "Throttle Start"
// #define TR_THROTTLEREVERSE             TR("T-Reverse", "Reverse")
// #define TR_MINUTEBEEP                  TR("Minute", "Minute call")
// #define TR_BEEPCOUNTDOWN               "Countdown"
// #define TR_PERSISTENT                  TR("Persist.", "Persistent")
#define TR_THROTTLE_LABEL               "스로틀"
#define TR_THROTTLE_START               "스로틀 시작"
#define TR_THROTTLEREVERSE              TR("스로틀 반전", "역방향")
#define TR_MINUTEBEEP                   TR("분 경고음", "1분마다 알림")
#define TR_BEEPCOUNTDOWN                "카운트다운 알림"
#define TR_PERSISTENT                   TR("유지", "값 유지")

// #define TR_BACKLIGHT_LABEL             "Backlight"
// #define TR_GHOST_MENU_LABEL            "GHOST MENU"
// #define TR_STATUS                      "Status"
// #define TR_BLONBRIGHTNESS              "ON brightness"
// #define TR_BLOFFBRIGHTNESS             "OFF brightness"
// #define TR_KEYS_BACKLIGHT              "Keys backlight"
// #define TR_BLCOLOR                     "Color"
// #define TR_SPLASHSCREEN                "Splash screen"
// #define TR_PLAY_HELLO                  "Startup Sound"
#define TR_BACKLIGHT_LABEL              "백라이트"
#define TR_GHOST_MENU_LABEL             "고스트 메뉴"
#define TR_STATUS                       "상태"
#define TR_BLONBRIGHTNESS               "켜짐 밝기"
#define TR_BLOFFBRIGHTNESS              "꺼짐 밝기"
#define TR_KEYS_BACKLIGHT               "키 백라이트"
#define TR_BLCOLOR                      "색상"
#define TR_SPLASHSCREEN                 "시작화면"
#define TR_PLAY_HELLO                   "시작 사운드"

// #define TR_PWR_ON_DELAY                "Pwr On delay"
// #define TR_PWR_OFF_DELAY               "Pwr Off delay"
// #define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
// #define TR_PWR_ON_OFF_HAPTIC           TR("Pwr ON/OFF Haptic","Power ON/OFF Haptic")
#define TR_PWR_ON_DELAY                 "전원 켜짐 지연"
#define TR_PWR_OFF_DELAY                "전원 꺼짐 지연"
#define TR_PWR_AUTO_OFF                 TR("자동 종료", "전원 자동 끄기")
#define TR_PWR_ON_OFF_HAPTIC            TR("전원 진동", "전원 ON/OFF 진동")

// #define TR_THROTTLE_WARNING            TR("T-Warning", "Throttle state")
// #define TR_CUSTOM_THROTTLE_WARNING     TR("Cust-Pos", "Custom position?")
// #define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
// #define TR_SWITCHWARNING               TR("S-Warning", "Switch positions")
// #define TR_POTWARNINGSTATE             "Pots & sliders"
// #define TR_POTWARNING                  TR("Pot warn.", "Pot positions")
#define TR_THROTTLE_WARNING             TR("스로틀 경고", "스로틀 상태 확인")
#define TR_CUSTOM_THROTTLE_WARNING      TR("사용자 위치", "사용자 정의 스로틀 위치?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL  TR("위치 %", "스로틀 위치 (%)")
#define TR_SWITCHWARNING                TR("스위치 경고", "스위치 위치 확인")
#define TR_POTWARNINGSTATE              "노브 및 슬라이더"
#define TR_POTWARNING                   TR("노브 경고", "노브 위치")

// #define TR_TIMEZONE                    "Time zone"
// #define TR_ADJUST_RTC                  "Adjust RTC"
// #define TR_GPS                         "GPS"
// #define TR_DEF_CHAN_ORD                TR("Def chan order", "Default channel order")
// #define TR_STICKS                      "Axis"
// #define TR_POTS                        "Pots"
// #define TR_SWITCHES                    "Switches"
// #define TR_SWITCHES_DELAY              TR("Play delay", "Play delay (sw. mid pos)")
#define TR_TIMEZONE                     "시간대"
#define TR_ADJUST_RTC                   "RTC 조정"
#define TR_GPS                          "GPS"
#define TR_DEF_CHAN_ORD                 TR("기본 채널 순서", "채널 기본 순서")
#define TR_STICKS                       "스틱"
#define TR_POTS                         "노브"
#define TR_SWITCHES                     "스위치"
#define TR_SWITCHES_DELAY               TR("재생 지연", "스위치 지연 (중간 위치)")

// #define TR_SLAVE                       "Slave"
// #define TR_MODESRC                     "Mode\006% Source"
// #define TR_MULTIPLIER                  "Multiplier"
// #define TR_CAL                         "Cal"
// #define TR_CALIBRATION                 "Calibration"
// #define TR_VTRIM                       "Trim - +"
// #define TR_CALIB_DONE                  "Calibration completed"
#define TR_SLAVE                        "슬레이브"
#define TR_MODESRC                      "모드별 소스"
#define TR_MULTIPLIER                   "배율"
#define TR_CAL                          "보정"
#define TR_CALIBRATION                  "보정"
#define TR_VTRIM                        "트림 - +"
#define TR_CALIB_DONE                   "보정 완료"

#if defined(PCBHORUS)
  // #define TR_MENUTOSTART               "Press [Enter] to start"
  // #define TR_SETMIDPOINT               "Center sticks/pots/sliders and press [Enter]"
  // #define TR_MOVESTICKSPOTS            "Move axis/pots/sliders and press [Enter]"
  #define TR_MENUTOSTART                "시작하려면 [ENTER]를 누르세요"
  #define TR_SETMIDPOINT                "스틱/포트/슬라이더를 중앙에 맞춘 후 [ENTER]"
  #define TR_MOVESTICKSPOTS             "스틱/포트/슬라이더를 움직이고 [ENTER]"

#elif defined(COLORLCD)
  // #define TR_MENUTOSTART               TR_ENTER " TO START"
  // #define TR_SETMIDPOINT               "CENTER AXIS/SLIDERS"
  // #define TR_MOVESTICKSPOTS            "MOVE AXIS/POTS"
  #define TR_MENUTOSTART                  TR_ENTER " → 시작"
  #define TR_SETMIDPOINT                  "스틱/슬라이더 중앙 정렬"
  #define TR_MOVESTICKSPOTS               "스틱/포트 이동"
#else
  // #define TR_MENUTOSTART               TR_ENTER " TO START"
  #define TR_MENUTOSTART                TR_ENTER " → 시작"
#if defined(SURFACE_RADIO)
  // #define TR_SETMIDPOINT               "SET POTS MIDPOINT"
  // #define TR_MOVESTICKSPOTS            "MOVE ST/TH/POTS/AXIS"
  #define TR_SETMIDPOINT                "포트 중앙 정렬"
  #define TR_MOVESTICKSPOTS             "스티어링/스로틀/포트/슬라이더 이동"
#else
  // #define TR_SETMIDPOINT               TR("SET AXIS MIDPOINT", "CENTER AXIS/SLIDERS")
  // #define TR_MOVESTICKSPOTS            "MOVE AXIS/POTS"
  #define TR_SETMIDPOINT                TR("스틱 중앙 정렬", "스틱/슬라이더 중앙 정렬")
  #define TR_MOVESTICKSPOTS             "스틱/포트 이동"
#endif
  // #define TR_MENUWHENDONE              TR_ENTER " WHEN DONE"
  #define TR_MENUWHENDONE               TR_ENTER " → 완료"
#endif

#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
// #define TR_NODATA                      "NO DATA"
#define TR_NODATA                       "데이터 없음"
#define TR_US                          "us"
#define TR_HZ                          "Hz"

// #define TR_TMIXMAXMS                   "Tmix max"
// #define TR_FREE_STACK                  "Free stack"
// #define TR_INT_GPS_LABEL               "Internal GPS"
// #define TR_HEARTBEAT_LABEL             "Heartbeat"
// #define TR_LUA_SCRIPTS_LABEL           "Lua scripts"
// #define TR_FREE_MEM_LABEL              "Free mem"
// #define TR_DURATION_MS             TR("[D]","Duration(ms): ")
// #define TR_INTERVAL_MS             TR("[I]","Interval(ms): ")
// #define TR_MEM_USED_SCRIPT         "Script(B): "
// #define TR_MEM_USED_WIDGET         "Widget(B): "
// #define TR_MEM_USED_EXTRA          "Extra(B): "
// #define TR_STACK_MIX                   "Mix: "
// #define TR_STACK_AUDIO                 "Audio: "
// #define TR_GPS_FIX_YES                 "Fix: Yes"
// #define TR_GPS_FIX_NO                  "Fix: No"
// #define TR_GPS_SATS                    "Sats: "
// #define TR_GPS_HDOP                    "Hdop: "
// #define TR_STACK_MENU                  "Menu: "
// #define TR_TIMER_LABEL                 "Timer"
// #define TR_THROTTLE_PERCENT_LABEL      "Throttle %"
// #define TR_BATT_LABEL                  "Battery"
// #define TR_SESSION                     "Session"
// #define TR_MENUTORESET                 TR_ENTER " to reset"
// #define TR_PPM_TRAINER                 "TR"
// #define TR_CH                          "CH"
// #define TR_MODEL                       "MODEL"
#define TR_TMIXMAXMS                  "최대 믹스 시간"
#define TR_FREE_STACK                 "남은 스택"
#define TR_INT_GPS_LABEL              "내장 GPS"
#define TR_HEARTBEAT_LABEL            "하트비트"
#define TR_LUA_SCRIPTS_LABEL          "Lua 스크립트"
#define TR_FREE_MEM_LABEL             "남은 메모리"
#define TR_DURATION_MS                TR("[D]", "지속 시간(ms): ")
#define TR_INTERVAL_MS                TR("[I]", "간격(ms): ")
#define TR_MEM_USED_SCRIPT            "스크립트(B): "
#define TR_MEM_USED_WIDGET            "위젯(B): "
#define TR_MEM_USED_EXTRA             "추가(B): "
#define TR_STACK_MIX                  "믹스: "
#define TR_STACK_AUDIO                "오디오: "
#define TR_GPS_FIX_YES                "위치 고정: 예"
#define TR_GPS_FIX_NO                 "위치 고정: 아니오"
#define TR_GPS_SATS                   "위성 수: "
#define TR_GPS_HDOP                   "HDOP: "
#define TR_STACK_MENU                 "메뉴: "
#define TR_TIMER_LABEL                "타이머"
#define TR_THROTTLE_PERCENT_LABEL     "스로틀 %"
#define TR_BATT_LABEL                 "배터리"
#define TR_SESSION                    "세션"
#define TR_MENUTORESET                TR_ENTER " → 초기화"
#define TR_PPM_TRAINER                "TR"
#define TR_CH                         "채널"
#define TR_MODEL                      "모델"

#if defined(SURFACE_RADIO)
#define TR_FM                          "DM"
#else
#define TR_FM                          "FM"
#endif

// #define TR_EEPROMLOWMEM                "EEPROM low mem"
// #define TR_PRESS_ANY_KEY_TO_SKIP       "Press any key to skip"
// #define TR_THROTTLE_NOT_IDLE           "Throttle not idle"
// #define TR_ALARMSDISABLED              "Alarms disabled"
// #define TR_PRESSANYKEY                 TR("\010Press any Key", "Press any key")
// #define TR_BADEEPROMDATA               "Bad EEprom data"
// #define TR_BAD_RADIO_DATA              "Missing or bad radio data"
// #define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
// #define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
// #define TR_EEPROMFORMATTING            "Formatting EEPROM"
#define TR_EEPROMLOWMEM                 "EEPROM 메모리 부족"
#define TR_PRESS_ANY_KEY_TO_SKIP        "아무 키나 눌러 건너뛰기"
#define TR_THROTTLE_NOT_IDLE            "스로틀이 유휴 상태가 아님"
#define TR_ALARMSDISABLED               "알람 비활성화됨"
#define TR_PRESSANYKEY                  TR("\010아무 키나 누르세요", "아무 키나 누르세요")
#define TR_BADEEPROMDATA                "손상된 EEPROM 데이터"
#define TR_BAD_RADIO_DATA               "라디오 설정 없음 또는 손상됨"
#define TR_RADIO_DATA_RECOVERED         TR3("백업 설정 사용 중", "백업 라디오 설정 사용", "라디오 설정을 복구함")
#define TR_RADIO_DATA_UNRECOVERABLE     TR3("라디오 설정 무효", "라디오 설정이 올바르지 않음", "유효한 라디오 설정을 읽을 수 없음")
#define TR_EEPROMFORMATTING             "EEPROM 포맷 중"

// #define TR_STORAGE_FORMAT              "Storage preparation"
// #define TR_EEPROMOVERFLOW              "EEPROM overflow"
// #define TR_RADIO_SETUP                 "RADIO SETUP"
// #define TR_MENUTRAINER                 "TRAINER"
// #define TR_MENUSPECIALFUNCS            "GLOBAL FUNCTIONS"
// #define TR_MENUVERSION                 "VERSION"
// #define TR_MENU_RADIO_SWITCHES         TR("SWITCHES", "SWITCHES TEST")
// #define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "ANALOGS TEST")
// #define TR_MENU_RADIO_ANALOGS_CALIB    "CALIBRATED ANALOGS"
// #define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
// #define TR_MENUCALIBRATION             "CALIBRATION"
// #define TR_MENU_FSWITCH                "CUSTOMIZABLE SWITCHES"
#define TR_STORAGE_FORMAT               "스토리지 준비 중"
#define TR_EEPROMOVERFLOW               "EEPROM 메모리 초과"
#define TR_RADIO_SETUP                  "조종기 설정"
#define TR_MENUTRAINER                  "트레이너"
#define TR_MENUSPECIALFUNCS             "전역 기능"
#define TR_MENUVERSION                  "버전 정보"
#define TR_MENU_RADIO_SWITCHES          TR("스위치", "스위치 테스트")
#define TR_MENU_RADIO_ANALOGS           TR("아날로그", "아날로그 테스트")
#define TR_MENU_RADIO_ANALOGS_CALIB     "보정된 아날로그"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW 아날로그 (5 Hz)"
#define TR_MENUCALIBRATION              "입력 보정"
#define TR_MENU_FSWITCH                 "사용자 스위치 설정"

#if defined(COLORLCD)
  // #define TR_TRIMS2OFFSETS             "Trims => Subtrims"
  #define TR_TRIMS2OFFSETS              "트림 → 서브트림"
#else
  // #define TR_TRIMS2OFFSETS             "\006Trims => Subtrims"
  #define TR_TRIMS2OFFSETS              "\006트림 → 서브트림"
#endif

// #define TR_CHANNELS2FAILSAFE           "Channels=>Failsafe"
// #define TR_CHANNEL2FAILSAFE            "Channel=>Failsafe"
#define TR_CHANNELS2FAILSAFE            "채널 전체 → 페일세이프"
#define TR_CHANNEL2FAILSAFE             "채널 → 페일세이프"

// #define TR_MENUMODELSEL                TR("MODELSEL", "MODEL SELECTION")
// #define TR_MENU_MODEL_SETUP            TR("SETUP", "MODEL SETUP")
#define TR_MENUMODELSEL                 TR("모델 선택", "모델 선택 메뉴")
#define TR_MENU_MODEL_SETUP             TR("설정", "모델 설정")

#if defined(SURFACE_RADIO)
// #define TR_MENUFLIGHTMODES             "DRIVE MODES"
// #define TR_MENUFLIGHTMODE              "DRIVE MODE"
#define TR_MENUFLIGHTMODES              "주행 모드"
#define TR_MENUFLIGHTMODE               "주행 모드"
#else
// #define TR_MENUFLIGHTMODES             "FLIGHT MODES"
// #define TR_MENUFLIGHTMODE              "FLIGHT MODE"
#define TR_MENUFLIGHTMODES              "비행 모드"
#define TR_MENUFLIGHTMODE               "비행 모드"
#endif
// #define TR_MENUHELISETUP               "HELI SETUP"
//   #define TR_MENUINPUTS                "INPUTS"
//   #define TR_MENULIMITS                "OUTPUTS"
#define TR_MENUHELISETUP                "헬리 설정"
#define TR_MENUINPUTS                   "입력"
#define TR_MENULIMITS                   "출력"

// #define TR_MENUCURVES                  "CURVES"
// #define TR_MENUCURVE                   "CURVE"
// #define TR_MENULOGICALSWITCH           "LOGICAL SWITCH"
// #define TR_MENULOGICALSWITCHES         "LOGICAL SWITCHES"
// #define TR_MENUCUSTOMFUNC              "SPECIAL FUNCTIONS"
// #define TR_MENUCUSTOMSCRIPTS           "CUSTOM SCRIPTS"
#define TR_MENUCURVES                   "커브"
#define TR_MENUCURVE                    "커브 설정"
#define TR_MENULOGICALSWITCH            "로직 스위치"
#define TR_MENULOGICALSWITCHES          "로직 스위치 목록"
#define TR_MENUCUSTOMFUNC               "특수 기능"
#define TR_MENUCUSTOMSCRIPTS            "사용자 스크립트"

// #define TR_MENUTELEMETRY               "TELEMETRY"
// #define TR_MENUSTAT                    "STATS"
// #define TR_MENUDEBUG                   "DEBUG"
#define TR_MENUTELEMETRY                "테레메트리"
#define TR_MENUSTAT                     "통계"
#define TR_MENUDEBUG                    "디버그"

// #define TR_MONITOR_CHANNELS1           "CHANNELS MONITOR 1-8"
// #define TR_MONITOR_CHANNELS2           "CHANNELS MONITOR 9-16"
// #define TR_MONITOR_CHANNELS3           "CHANNELS MONITOR 17-24"
// #define TR_MONITOR_CHANNELS4           "CHANNELS MONITOR 25-32"
// #define TR_MONITOR_SWITCHES            "LOGICAL SWITCHES MONITOR"
// #define TR_MONITOR_OUTPUT_DESC         "Outputs"
// #define TR_MONITOR_MIXER_DESC          "Mixers"
#define TR_MONITOR_CHANNELS1            "채널 모니터 1-8"
#define TR_MONITOR_CHANNELS2            "채널 모니터 9-16"
#define TR_MONITOR_CHANNELS3            "채널 모니터 17-24"
#define TR_MONITOR_CHANNELS4            "채널 모니터 25-32"
#define TR_MONITOR_SWITCHES             "로직 스위치 모니터"
#define TR_MONITOR_OUTPUT_DESC          "출력 상태"
#define TR_MONITOR_MIXER_DESC           "믹서 상태"

  // #define TR_RECEIVER_NUM              TR("RxNum", "Receiver No.")
  // #define TR_RECEIVER                  "Receiver"
#define TR_RECEIVER_NUM                 TR("수신기 번호", "Receiver No.")
#define TR_RECEIVER                     "수신기"

// #define TR_MULTI_RFTUNE                TR("Freq tune", "RF Freq. fine tune")
// #define TR_MULTI_RFPOWER               "RF power"
// #define TR_MULTI_WBUS                  "Output"
// #define TR_MULTI_TELEMETRY             "Telemetry"
// #define TR_MULTI_VIDFREQ               TR("Vid. freq.", "Video frequency")
#define TR_MULTI_RFTUNE                 TR("주파수 조정", "RF 주파수 미세 조정")
#define TR_MULTI_RFPOWER                "RF 출력"
#define TR_MULTI_WBUS                   "출력"
#define TR_MULTI_TELEMETRY              "텔레메트리"
#define TR_MULTI_VIDFREQ                TR("영상 주파수", "비디오 송출 주파수")

// #define TR_RF_POWER                    "RF Power"
// #define TR_MULTI_FIXEDID               TR("FixedID", "Fixed ID")
// #define TR_MULTI_OPTION                TR("Option", "Option value")
// #define TR_MULTI_AUTOBIND              TR("Bind Ch.", "Bind on channel")
// #define TR_DISABLE_CH_MAP              TR("No Ch. map", "Disable Ch. map")
// #define TR_DISABLE_TELEM               TR("No Telem", "Disable Telemetry")
// #define TR_MULTI_LOWPOWER              TR("Low power", "Low power mode")
// #define TR_MULTI_LNA_DISABLE           "LNA disable"
#define TR_RF_POWER                     "RF 출력"
#define TR_MULTI_FIXEDID                TR("고정ID", "고정 ID")
#define TR_MULTI_OPTION                 TR("옵션", "옵션 값")
#define TR_MULTI_AUTOBIND               TR("바인드 채널", "채널에서 자동 바인딩")
#define TR_DISABLE_CH_MAP               TR("채널 매핑 없음", "채널 매핑 비활성화")
#define TR_DISABLE_TELEM                TR("텔레메트리 없음", "텔레메트리 비활성화")
#define TR_MULTI_LOWPOWER               TR("저전력", "저전력 모드")
#define TR_MULTI_LNA_DISABLE            "LNA 비활성화"

// #define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
// #define TR_MODULE_TELEM_ON             TR("ON", "Enabled")
// #define TR_DISABLE_INTERNAL            TR("Disable int.", "Disable internal RF")
// #define TR_MODULE_NO_SERIAL_MODE       TR("!serial mode", "Not in serial mode")
// #define TR_MODULE_NO_INPUT             TR("No input", "No serial input")
// #define TR_MODULE_NO_TELEMETRY         TR3("No telemetry", "No MULTI_TELEMETRY", "No MULTI_TELEMETRY detected")
// #define TR_MODULE_WAITFORBIND          "Bind to load protocol"
// #define TR_MODULE_BINDING              TR("Bind...","Binding")
// #define TR_MODULE_UPGRADE_ALERT        TR3("Upg. needed", "Module upgrade required", "Module\nUpgrade required")
// #define TR_MODULE_UPGRADE              TR("Upg. advised", "Module update recommended")
#define TR_MODULE_TELEMETRY             TR("S.Port", "S.Port 연결")
#define TR_MODULE_TELEM_ON              TR("켬", "활성화됨")
#define TR_DISABLE_INTERNAL             TR("내장 RF 끄기", "내부 RF 모듈 비활성화")
#define TR_MODULE_NO_SERIAL_MODE        TR("직렬 모드 아님", "시리얼 모드가 아닙니다")
#define TR_MODULE_NO_INPUT              TR("입력 없음", "시리얼 입력 없음")
#define TR_MODULE_NO_TELEMETRY          TR3("텔레메트리 없음", "MULTI_TELEMETRY 없음", "텔레메트리 모듈을 찾을 수 없습니다")
#define TR_MODULE_WAITFORBIND           "바인딩 중 프로토콜 로딩"
#define TR_MODULE_BINDING               TR("바인딩 중...", "바인딩 중")
#define TR_MODULE_UPGRADE_ALERT         TR3("업데이트 필요", "모듈 업그레이드 필요", "모듈\n업데이트가 필요합니다")
#define TR_MODULE_UPGRADE               TR("업데이트 권장", "모듈 업데이트를 권장합니다")

// #define TR_REBIND                      "Rebinding required"
// #define TR_REG_OK                      "Registration ok"
// #define TR_BIND_OK                     "Bind successful"
// #define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem ON"
// #define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem OFF"
// #define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem ON"
// #define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem OFF"
#define TR_REBIND                       "재바인딩 필요"
#define TR_REG_OK                       "등록 완료"
#define TR_BIND_OK                      "바인딩 성공"
#define TR_BINDING_CH1_8_TELEM_ON       "CH1-8 텔레메트리 켬"
#define TR_BINDING_CH1_8_TELEM_OFF      "CH1-8 텔레메트리 끔"
#define TR_BINDING_CH9_16_TELEM_ON      "CH9-16 텔레메트리 켬"
#define TR_BINDING_CH9_16_TELEM_OFF     "CH9-16 텔레메트리 끔"

// #define TR_PROTOCOL_INVALID            TR("Prot. invalid", "Protocol invalid")
// #define TR_MODULE_STATUS               TR("Status", "Module Status")
// #define TR_MODULE_SYNC                 TR("Sync", "Proto Sync Status")
// #define TR_MULTI_SERVOFREQ             TR("Servo rate", "Servo update rate")
// #define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
// #define TR_MULTI_RFCHAN                TR("RF Channel", "Select RF channel")
// #define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
// #define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
// #define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_PROTOCOL_INVALID             TR("프로토콜 오류", "프로토콜이 유효하지 않음")
#define TR_MODULE_STATUS                TR("상태", "모듈 상태")
#define TR_MODULE_SYNC                  TR("동기화", "프로토콜 동기화 상태")
#define TR_MULTI_SERVOFREQ              TR("서보 속도", "서보 업데이트 속도")
#define TR_MULTI_MAX_THROW              TR("최대 스로우", "최대 이동량 허용")
#define TR_MULTI_RFCHAN                 TR("RF 채널", "RF 채널 선택")
#define TR_AFHDS3_RX_FREQ               TR("수신기 주파수", "수신기 RF 주파수")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY  TR("단일/텔레메트리", "Unicast/텔레메트리 모드")
#define TR_AFHDS3_ONE_TO_MANY           "멀티캐스트"

// #define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
// #define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
// #define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
// #define TR_GPS_COORDS_FORMAT           TR("GPS Coords", "Coordinate format")
// #define TR_VARIO                       TR("Vario", "Variometer")
// #define TR_PITCH_AT_ZERO               "Pitch zero"
// #define TR_PITCH_AT_MAX                "Pitch max"
// #define TR_REPEAT_AT_ZERO              "Repeat zero"
// #define TR_BATT_CALIB                  TR("Batt. calib", "Battery calibration")
// #define TR_CURRENT_CALIB               "Current calib"
// #define TR_VOLTAGE                     TR("Voltage", "Voltage source")
// #define TR_SELECT_MODEL                "Select model"
// #define TR_MANAGE_MODELS               "MANAGE MODELS"
// #define TR_MODELS                      "Models"
// #define TR_SELECT_MODE                 "Select mode"
// #define TR_CREATE_MODEL                "Create model"
// #define TR_FAVORITE_LABEL              "Favorites"
#define TR_AFHDS3_ACTUAL_POWER          TR("실제 출력", "실제 RF 출력")
#define TR_AFHDS3_POWER_SOURCE          TR("전원 소스", "전원 공급 방식")
#define TR_FLYSKY_TELEMETRY             TR("FlySky RSSI 값", "RSSI 값을 리스케일 없이 사용")
#define TR_GPS_COORDS_FORMAT            TR("GPS 좌표", "좌표 표시 형식")
#define TR_VARIO                        TR("바리오", "수직 속도계")
#define TR_PITCH_AT_ZERO                "제로 피치"
#define TR_PITCH_AT_MAX                 "최대 피치"
#define TR_REPEAT_AT_ZERO               "제로에서 반복"
#define TR_BATT_CALIB                   TR("배터리 보정", "배터리 전압 보정")
#define TR_CURRENT_CALIB                "전류 보정"
#define TR_VOLTAGE                      TR("전압", "전압 소스")
#define TR_SELECT_MODEL                 "모델 선택"
#define TR_MANAGE_MODELS                "모델 관리"
#define TR_MODELS                       "모델"
#define TR_SELECT_MODE                  "모드 선택"
#define TR_CREATE_MODEL                 "모델 생성"
#define TR_FAVORITE_LABEL               "즐겨찾기"

// #define TR_MODELS_MOVED                "Unused models moved to"
// #define TR_NEW_MODEL                   "New Model"
// #define TR_INVALID_MODEL               "Invalid Model"
// #define TR_EDIT_LABELS                 "Edit Labels"
// #define TR_LABEL_MODEL                 "Label model"
// #define TR_MOVE_UP                     "Move Up"
// #define TR_MOVE_DOWN                   "Move Down"
// #define TR_ENTER_LABEL                 "Enter Label"
// #define TR_LABEL                       "Label"
// #define TR_LABELS                      "Labels"
// #define TR_CURRENT_MODEL               "Current"
// #define TR_ACTIVE                      "Active"
// #define TR_NEW                         "New"
// #define TR_NEW_LABEL                   "New Label"
// #define TR_RENAME_LABEL                "Rename Label"
// #define TR_DELETE_LABEL                "Delete Label"
// #define TR_DUPLICATE_MODEL             "Duplicate model"
// #define TR_COPY_MODEL                  "Copy model"
// #define TR_MOVE_MODEL                  "Move model"
// #define TR_BACKUP_MODEL                "Backup model"
// #define TR_DELETE_MODEL                "Delete model"
// #define TR_RESTORE_MODEL               "Restore model"
// #define TR_DELETE_ERROR                "Delete error"
#define TR_MODELS_MOVED                   "사용하지 않는 모델이 이동됨 →"
#define TR_NEW_MODEL                      "새 모델"
#define TR_INVALID_MODEL                  "잘못된 모델"
#define TR_EDIT_LABELS                    "라벨 편집"
#define TR_LABEL_MODEL                    "모델 라벨"
#define TR_MOVE_UP                        "위로 이동"
#define TR_MOVE_DOWN                      "아래로 이동"
#define TR_ENTER_LABEL                    "라벨 입력"
#define TR_LABEL                          "라벨"
#define TR_LABELS                         "라벨 목록"
#define TR_CURRENT_MODEL                  "현재 모델"
#define TR_ACTIVE                         "활성화됨"
#define TR_NEW                            "새로 만들기"
#define TR_NEW_LABEL                      "새 라벨"
#define TR_RENAME_LABEL                   "라벨 이름 변경"
#define TR_DELETE_LABEL                   "라벨 삭제"
#define TR_DUPLICATE_MODEL                "모델 복제"
#define TR_COPY_MODEL                     "모델 복사"
#define TR_MOVE_MODEL                     "모델 이동"
#define TR_BACKUP_MODEL                   "모델 백업"
#define TR_DELETE_MODEL                   "모델 삭제"
#define TR_RESTORE_MODEL                  "모델 복원"
#define TR_DELETE_ERROR                   "삭제 오류"

// #define TR_SDCARD_ERROR                TR("SD error", "SD card error")
// #define TR_SDCARD                      "SD Card"
// #define TR_NO_FILES_ON_SD              "No files on SD!"
// #define TR_NO_SDCARD                   "No SD card"
// #define TR_WAITING_FOR_RX              "Waiting for RX..."
// #define TR_WAITING_FOR_TX              "Waiting for TX..."
// #define TR_WAITING_FOR_MODULE          TR("Waiting module", "Waiting for module...")
// #define TR_NO_TOOLS                    "No tools available"
// #define TR_NORMAL                      "Normal"
#define TR_SDCARD_ERROR                   TR("SD 오류", "SD카드 오류")
#define TR_SDCARD                         "SD 카드"
#define TR_NO_FILES_ON_SD                 "SD 카드에 파일 없음!"
#define TR_NO_SDCARD                      "SD 카드 없음"
#define TR_WAITING_FOR_RX                 "수신기 대기 중..."
#define TR_WAITING_FOR_TX                 "송신기 대기 중..."
#define TR_WAITING_FOR_MODULE             TR("모듈 대기 중", "모듈 연결 대기 중...")
#define TR_NO_TOOLS                       "사용 가능한 도구 없음"
#define TR_NORMAL                         "정상"

// #define TR_NOT_INVERTED                "Not inv"
// #define TR_NOT_CONNECTED               TR("!Connected", "Not Connected");
// #define TR_CONNECTED                   "Connected"
// #define TR_FLEX_915                    "Flex 915MHz"
// #define TR_FLEX_868                    "Flex 868MHz"
// #define TR_16CH_WITHOUT_TELEMETRY      TR("16CH without telem.", "16CH without telemetry")
// #define TR_16CH_WITH_TELEMETRY         TR("16CH with telem.", "16CH with telemetry")
// #define TR_8CH_WITH_TELEMETRY          TR("8CH with telem.", "8CH with telemetry")
// #define TR_EXT_ANTENNA                 "Ext. antenna"
// #define TR_PIN                         "Pin"
#define TR_NOT_INVERTED                   "비반전"
#define TR_NOT_CONNECTED                  TR("미연결", "연결되지 않음")
#define TR_CONNECTED                      "연결됨"
#define TR_FLEX_915                       "Flex 915MHz"
#define TR_FLEX_868                       "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY         TR("16채널(텔레메트리 없음)", "16채널(텔레메트리 비활성)")
#define TR_16CH_WITH_TELEMETRY            TR("16채널(텔레메트리 포함)", "16채널(텔레메트리 사용)")
#define TR_8CH_WITH_TELEMETRY             TR("8채널(텔레메트리 포함)", "8채널(텔레메트리 사용)")
#define TR_EXT_ANTENNA                    "외부 안테나"
#define TR_PIN                            "핀"

// #define TR_UPDATE_RX_OPTIONS           "Update RX options?"
// #define TR_UPDATE_TX_OPTIONS           "Update TX options?"
// #define TR_MODULES_RX_VERSION          "Modules / RX version"
// #define TR_SHOW_MIXER_MONITORS         "Show mixer monitors"
// #define TR_MENU_MODULES_RX_VERSION     "MODULES / RX VERSION"
// #define TR_MENU_FIRM_OPTIONS           "FIRMWARE OPTIONS"
// #define TR_IMU                        "IMU"
// #define TR_STICKS_POTS_SLIDERS         "Axis/Pots/Sliders"
// #define TR_PWM_STICKS_POTS_SLIDERS     "PWM Axis/Pots/Sliders"
// #define TR_RF_PROTOCOL                 "RF Protocol"
// #define TR_MODULE_OPTIONS              "Module options"
// #define TR_POWER                       "Power"
// #define TR_NO_TX_OPTIONS               "No TX options"
// #define TR_RTC_BATT                    "RTC Batt"
// #define TR_POWER_METER_EXT             "Power Meter (EXT)"
// #define TR_POWER_METER_INT             "Power Meter (INT)"
// #define TR_SPECTRUM_ANALYSER_EXT       "Spectrum (EXT)"
// #define TR_SPECTRUM_ANALYSER_INT       "Spectrum (INT)"
// #define TR_SDCARD_FULL                 "SD card full"
#define TR_UPDATE_RX_OPTIONS              "수신기 옵션을 업데이트할까요?"
#define TR_UPDATE_TX_OPTIONS              "송신기 옵션을 업데이트할까요?"
#define TR_MODULES_RX_VERSION             "모듈 / 수신기 버전"
#define TR_SHOW_MIXER_MONITORS            "믹서 모니터 표시"
#define TR_MENU_MODULES_RX_VERSION        "모듈 / 수신기 버전"
#define TR_MENU_FIRM_OPTIONS              "펌웨어 옵션"
#define TR_IMU                            "IMU"
#define TR_STICKS_POTS_SLIDERS            "스틱 / 포트 / 슬라이더"
#define TR_PWM_STICKS_POTS_SLIDERS        "PWM 스틱 / 포트 / 슬라이더"
#define TR_RF_PROTOCOL                    "RF 프로토콜"
#define TR_MODULE_OPTIONS                 "모듈 옵션"
#define TR_POWER                          "전원"
#define TR_NO_TX_OPTIONS                  "송신기 옵션 없음"
#define TR_RTC_BATT                       "RTC 배터리"
#define TR_POWER_METER_EXT                "전력 측정기 (외부)"
#define TR_POWER_METER_INT                "전력 측정기 (내부)"
#define TR_SPECTRUM_ANALYSER_EXT          "스펙트럼 분석 (외부)"
#define TR_SPECTRUM_ANALYSER_INT          "스펙트럼 분석 (내부)"
#define TR_SDCARD_FULL                    "SD 카드가 가득 찼습니다"

#if defined(COLORLCD)
// #define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\nLogs and Screenshots disabled"
#define TR_SDCARD_FULL_EXT              TR_SDCARD_FULL "\n로그 및 스크린샷 저장 불가"
#else
// #define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036Logs & Screenshots" LCDW_128_LINEBREAK "disabled"
#define TR_SDCARD_FULL_EXT              TR_SDCARD_FULL "\036로그 및 스크린샷" LCDW_128_LINEBREAK "저장 불가"
#endif

// #define TR_NEEDS_FILE                  "NEEDS FILE"
// #define TR_EXT_MULTI_SPEC              "opentx-inv"
// #define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
// #define TR_INCOMPATIBLE                "Incompatible"
// #define TR_WARNING                     "WARNING"
// #define TR_STORAGE_WARNING             "STORAGE"
// #define TR_THROTTLE_UPPERCASE          "THROTTLE"
// #define TR_ALARMSWARN                  "ALARMS"
// #define TR_SWITCHWARN                  TR("SWITCH", "CONTROL")
// #define TR_FAILSAFEWARN                "FAILSAFE"
// #define TR_TEST_WARNING                TR("TESTING", "TEST BUILD")
// #define TR_TEST_NOTSAFE                "Use for tests only"
// #define TR_WRONG_SDCARDVERSION         TR("Expected ver: ", "Expected version: ")
// #define TR_WARN_RTC_BATTERY_LOW        "RTC Battery low"
// #define TR_WARN_MULTI_LOWPOWER         "Low power mode"
// #define TR_BATTERY                     "BATTERY"
#define TR_NEEDS_FILE                     "파일 필요"
#define TR_EXT_MULTI_SPEC                 "opentx-inv"
#define TR_INT_MULTI_SPEC                 "stm-opentx-noinv"
#define TR_INCOMPATIBLE                   "호환되지 않음"
#define TR_WARNING                        "경고"
#define TR_STORAGE_WARNING                "저장소"
#define TR_THROTTLE_UPPERCASE             "스로틀"
#define TR_ALARMSWARN                     "알람"
#define TR_SWITCHWARN                     TR("스위치", "컨트롤러")
#define TR_FAILSAFEWARN                   "페일세이프"
#define TR_TEST_WARNING                   TR("테스트 중", "테스트 빌드")
#define TR_TEST_NOTSAFE                   "테스트 전용, 안전하지 않음"
#define TR_WRONG_SDCARDVERSION            TR("필요 버전: ", "필요한 SD 카드 버전: ")
#define TR_WARN_RTC_BATTERY_LOW           "RTC 배터리 부족"
#define TR_WARN_MULTI_LOWPOWER            "저전력 모드 경고"
#define TR_BATTERY                        "배터리"

// #define TR_WRONG_PCBREV                "Wrong PCB detected"
// #define TR_EMERGENCY_MODE              "EMERGENCY MODE"
// #define TR_NO_FAILSAFE                 "Failsafe not set"
// #define TR_KEYSTUCK                    "Key stuck"
// #define TR_VOLUME                      "Volume"
// #define TR_LCD                         "LCD"
// #define TR_BRIGHTNESS                  "Brightness"
// #define TR_CPU_TEMP                    "CPU temp."
// #define TR_COPROC                      "CoProc."
// #define TR_COPROC_TEMP                 "MB Temp."
// #define TR_TTL_WARNING                 "Warning: Do not exceed 3.3V on TX/RX pins!"
// #define TR_FUNC                        "Function"
// #define TR_V1                          "V1"
// #define TR_V2                          "V2"
// #define TR_DURATION                    "Duration"
// #define TR_DELAY                       "Delay"
// #define TR_SD_CARD                     "SD CARD"
// #define TR_SDHC_CARD                   "SD-HC CARD"
#define TR_WRONG_PCBREV                   "잘못된 PCB가 감지됨"
#define TR_EMERGENCY_MODE                 "비상 모드"
#define TR_NO_FAILSAFE                    "페일세이프가 설정되지 않음"
#define TR_KEYSTUCK                       "버튼이 눌려 있음"
#define TR_VOLUME                         "볼륨"
#define TR_LCD                            "LCD"
#define TR_BRIGHTNESS                     "밝기"
#define TR_CPU_TEMP                       "CPU 온도"
#define TR_COPROC                         "보조 프로세서"
#define TR_COPROC_TEMP                    "메인보드 온도"
#define TR_TTL_WARNING                    "[경고] TX/RX 핀에 3.3V를 초과하지 마세요!"
#define TR_FUNC                           "기능"
#define TR_V1                             "V1"
#define TR_V2                             "V2"
#define TR_DURATION                       "지속 시간"
#define TR_DELAY                          "지연"
#define TR_SD_CARD                        "SD 카드"
#define TR_SDHC_CARD                      "SD-HC 카드"

// #define TR_NO_SOUNDS_ON_SD             "No sounds on SD"
// #define TR_NO_MODELS_ON_SD             "No models on SD"
// #define TR_NO_BITMAPS_ON_SD            "No bitmaps on SD"
// #define TR_NO_SCRIPTS_ON_SD            "No scripts on SD"
// #define TR_SCRIPT_SYNTAX_ERROR         TR("Syntax error", "Script syntax error")
// #define TR_SCRIPT_PANIC                "Script panic"
// #define TR_SCRIPT_KILLED               "Script killed"
// #define TR_SCRIPT_ERROR                "Unknown error"
// #define TR_PLAY_FILE                   "Play"
// #define TR_DELETE_FILE                 "Delete"
// #define TR_COPY_FILE                   "Copy"
// #define TR_RENAME_FILE                 "Rename"
// #define TR_ASSIGN_BITMAP               "Assign bitmap"
// #define TR_ASSIGN_SPLASH               "Splash screen"
// #define TR_EXECUTE_FILE                "Execute"
// #define TR_REMOVED                     " removed"
// #define TR_SD_INFO                     "Information"
// #define TR_NA                          "N/A"
#define TR_NO_SOUNDS_ON_SD                "SD 카드에 사운드 없음"
#define TR_NO_MODELS_ON_SD                "SD 카드에 모델 없음"
#define TR_NO_BITMAPS_ON_SD               "SD 카드에 비트맵 없음"
#define TR_NO_SCRIPTS_ON_SD               "SD 카드에 스크립트 없음"
#define TR_SCRIPT_SYNTAX_ERROR            TR("문법 오류", "스크립트 문법 오류")
#define TR_SCRIPT_PANIC                   "스크립트 중단됨"
#define TR_SCRIPT_KILLED                  "스크립트 강제 종료됨"
#define TR_SCRIPT_ERROR                   "알 수 없는 오류"
#define TR_PLAY_FILE                      "재생"
#define TR_DELETE_FILE                    "삭제"
#define TR_COPY_FILE                      "복사"
#define TR_RENAME_FILE                    "이름 바꾸기"
#define TR_ASSIGN_BITMAP                  "비트맵 지정"
#define TR_ASSIGN_SPLASH                  "시작화면 지정"
#define TR_EXECUTE_FILE                   "실행"
#define TR_REMOVED                        "제거됨"
#define TR_SD_INFO                        "SD 카드 정보"
#define TR_NA                             "N/A"

// #define TR_HARDWARE                    "HARDWARE"
// #define TR_FORMATTING                  "Formatting..."
// #define TR_TEMP_CALIB                  "Temp. Calib"
// #define TR_TIME                        "Time"
// #define TR_MAXBAUDRATE                 "Max bauds"
// #define TR_BAUDRATE                    "Baudrate"
// #define TR_CRSF_ARMING_MODE            "Arm using"
// #define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
// #define TR_SAMPLE_MODE                 TR("Sampling","Sample Mode")
// #define TR_SAMPLE_MODES                "Normal","OneBit"
// #define TR_LOADING                     "Loading..."
// #define TR_DELETE_THEME                "Delete Theme?"
// #define TR_SAVE_THEME                  "Save Theme?"
// #define TR_EDIT_COLOR                  "Edit Color"
// #define TR_NO_THEME_IMAGE              "No theme image"
// #define TR_BACKLIGHT_TIMER             "Inactivity timeout"
#define TR_HARDWARE                       "하드웨어"
#define TR_FORMATTING                     "포맷 중..."
#define TR_TEMP_CALIB                     "온도 보정"
#define TR_TIME                           "시간"
#define TR_MAXBAUDRATE                    "최대 보드레이트"
#define TR_BAUDRATE                       "보드레이트"
#define TR_CRSF_ARMING_MODE               "아밍 방식"


#define TR_CRSF_ARMING_MODES              TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                    TR("샘플링", "샘플 모드")
#define TR_SAMPLE_MODES                   "일반", "1비트"
#define TR_LOADING                        "불러오는 중..."
#define TR_DELETE_THEME                   "테마를 삭제할까요?"
#define TR_SAVE_THEME                     "테마를 저장할까요?"
#define TR_EDIT_COLOR                     "색상 편집"
#define TR_NO_THEME_IMAGE                 "테마 이미지 없음"
#define TR_BACKLIGHT_TIMER                "무작동 타이머"

#if defined(COLORLCD)
  // #define TR_MODEL_QUICK_SELECT        "Model quick select"
  // #define TR_LABELS_SELECT             "Label select"
  // #define TR_LABELS_MATCH              "Label matching"
  // #define TR_FAV_MATCH                 "Favorites matching"
  // #define TR_LABELS_SELECT_MODE        "Multi select","Single select"
  // #define TR_LABELS_MATCH_MODE         "Match all","Match any"
  // #define TR_FAV_MATCH_MODE            "Must match","Optional match"
#define TR_MODEL_QUICK_SELECT             "빠른 모델 선택"
#define TR_LABELS_SELECT                  "라벨 선택"
#define TR_LABELS_MATCH                   "라벨 매칭"
#define TR_FAV_MATCH                      "즐겨찾기 조건"
#define TR_LABELS_SELECT_MODE             "다중 선택", "단일 선택"
#define TR_LABELS_MATCH_MODE              "모두 일치", "하나 이상 일치"
#define TR_FAV_MATCH_MODE                 "일치해야 함", "일치해도 됨"
#endif

// #define TR_SELECT_TEMPLATE_FOLDER      "Select a template folder"
// #define TR_SELECT_TEMPLATE             "SELECT A MODEL TEMPLATE:"
// #define TR_NO_TEMPLATES                "No model templates were found in this folder"
// #define TR_SAVE_TEMPLATE               "Save as template"
// #define TR_BLANK_MODEL                 "Blank Model"
// #define TR_BLANK_MODEL_INFO            "Create a blank model"
// #define TR_FILE_EXISTS                 "FILE ALREADY EXISTS"
// #define TR_ASK_OVERWRITE               "Do you want to overwrite?"
#define TR_SELECT_TEMPLATE_FOLDER         "템플릿 폴더 선택"
#define TR_SELECT_TEMPLATE                "모델 템플릿 선택:"
#define TR_NO_TEMPLATES                   "이 폴더에 템플릿이 없습니다"
#define TR_SAVE_TEMPLATE                  "템플릿으로 저장"
#define TR_BLANK_MODEL                    "빈 모델"
#define TR_BLANK_MODEL_INFO               "빈 모델을 새로 생성합니다"
#define TR_FILE_EXISTS                    "파일이 이미 존재합니다"
#define TR_ASK_OVERWRITE                  "덮어쓰시겠습니까?"

// #define TR_BLUETOOTH                   "Bluetooth"
// #define TR_BLUETOOTH_DISC              "Discover"
// #define TR_BLUETOOTH_INIT              "Init"
// #define TR_BLUETOOTH_DIST_ADDR         "Dist addr"
// #define TR_BLUETOOTH_LOCAL_ADDR        "Local addr"
// #define TR_BLUETOOTH_PIN_CODE          "PIN code"
// #define TR_BLUETOOTH_NODEVICES         "No Devices Found"
// #define TR_BLUETOOTH_SCANNING          "Scanning..."
// #define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#define TR_BLUETOOTH                      "블루투스"
#define TR_BLUETOOTH_DISC                 "기기 검색"
#define TR_BLUETOOTH_INIT                 "초기화"
#define TR_BLUETOOTH_DIST_ADDR            "원격 주소"
#define TR_BLUETOOTH_LOCAL_ADDR           "로컬 주소"
#define TR_BLUETOOTH_PIN_CODE             "PIN 코드"
#define TR_BLUETOOTH_NODEVICES            "기기를 찾을 수 없습니다"
#define TR_BLUETOOTH_SCANNING             "검색 중..."
#define TR_BLUETOOTH_BAUDRATE             "블루투스 보드레이트"

#if defined(PCBX9E)
// #define TR_BLUETOOTH_MODES             "---","Enabled"
#define TR_BLUETOOTH_MODES                "---", "사용함"
#else
// #define TR_BLUETOOTH_MODES             "---","Telemetry","Trainer"
#define TR_BLUETOOTH_MODES                "---", "텔레메트리", "트레이너"
#endif

// #define TR_SD_INFO_TITLE               "SD INFO"
// #define TR_SD_SPEED                    "Speed:"
// #define TR_SD_SECTORS                  "Sectors:"
// #define TR_SD_SIZE                     "Size:"
// #define TR_TYPE                        "Type"
// #define TR_GLOBAL_VARS                 "Global variables"
// #define TR_GVARS                       "GVARS"
// #define TR_GLOBAL_VAR                  "Global variable"
// #define TR_MENU_GLOBAL_VARS            "GLOBAL VARIABLES"
// #define TR_OWN                         "Own"
// #define TR_DATE                        "Date"
// #define TR_MONTHS                      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
// #define TR_ROTARY_ENCODER              "R.E."
#define TR_SD_INFO_TITLE                  "SD 카드 정보"
#define TR_SD_SPEED                       "속도:"
#define TR_SD_SECTORS                     "섹터 수:"
#define TR_SD_SIZE                        "용량:"
#define TR_TYPE                           "유형"
#define TR_GLOBAL_VARS                    "전역 변수"
#define TR_GVARS                          "전역값"
#define TR_GLOBAL_VAR                     "전역 변수"
#define TR_MENU_GLOBAL_VARS               "전역 변수 설정"
#define TR_OWN                            "사용자 설정"
#define TR_DATE                           "날짜"
#define TR_MONTHS                         { "1월", "2월", "3월", "4월", "5월", "6월", "7월", "8월", "9월", "10월", "11월", "12월" }
#define TR_ROTARY_ENCODER                 "로터리 인코더"

// #define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Rotary Encoder Mode")
// #define TR_CHANNELS_MONITOR            "CHANNELS MONITOR"
// #define TR_MIXERS_MONITOR              "MIXERS MONITOR"
// #define TR_PATH_TOO_LONG               "Path too long"
// #define TR_VIEW_TEXT                   "View text"
// #define TR_FLASH_BOOTLOADER            "Flash bootloader"
// #define TR_FLASH_DEVICE                TR("Flash device","Flash device")
// #define TR_FLASH_EXTERNAL_DEVICE       TR("Flash S.Port", "Flash S.Port device")
// #define TR_FLASH_RECEIVER_OTA          "Flash receiver OTA"
// #define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
// #define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
// #define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
// #define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
// #define TR_FLASH_BLUETOOTH_MODULE      TR("Flash BT module", "Flash Bluetooth module")
// #define TR_FLASH_POWER_MANAGEMENT_UNIT "Flash pwr mngt unit"
#define TR_ROTARY_ENC_MODE                TR("인코더 모드", "로터리 인코더 모드")
#define TR_CHANNELS_MONITOR               "채널 모니터"
#define TR_MIXERS_MONITOR                 "믹서 모니터"
#define TR_PATH_TOO_LONG                  "경로가 너무 깁니다."
#define TR_VIEW_TEXT                      "텍스트 보기"
#define TR_FLASH_BOOTLOADER               "부트로더 플래시"
#define TR_FLASH_DEVICE                   TR("장치 플래시", "장치에 펌웨어 쓰기")
#define TR_FLASH_EXTERNAL_DEVICE          TR("S.Port 플래시", "S.Port 장치 플래시")
#define TR_FLASH_RECEIVER_OTA             "수신기 OTA 플래시"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "외부 모듈로 수신기 OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "내부 모듈로 수신기 OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "외부 모듈로 FC OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "내부 모듈로 FC OTA"
#define TR_FLASH_BLUETOOTH_MODULE         TR("BT 모듈 플래시", "블루투스 모듈 플래시")
#define TR_FLASH_POWER_MANAGEMENT_UNIT  "전원 관리 장치 플래시"

// #define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "Device not responding")
// #define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Device file prob.")
// #define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "Device data refused")
// #define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "Device access problem")
// #define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "Device file refused")
// #define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", "Device file sig.")
// #define TR_CURRENT_VERSION             TR("Curr Vers: ", "Current version: ")
// #define TR_FLASH_INTERNAL_MODULE       TR("Flash int. module", "Flash internal module")
// #define TR_FLASH_INTERNAL_MULTI        TR("Flash Int. Multi", "Flash Internal Multi")
// #define TR_FLASH_EXTERNAL_MODULE       TR("Flash ext. module", "Flash external module")
// #define TR_FLASH_EXTERNAL_MULTI        TR("Flash Ext. Multi", "Flash External Multi")
// #define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flash External ELRS")
// #define TR_FIRMWARE_UPDATE_ERROR       TR("FW update error", "Firmware update error")
#define TR_DEVICE_NO_RESPONSE             TR("장치 응답 없음", "장치가 응답하지 않음")
#define TR_DEVICE_FILE_ERROR              TR("장치 파일 오류", "장치 파일 문제 발생")
#define TR_DEVICE_DATA_REFUSED            TR("데이터 거부됨", "장치가 데이터를 거부함")
#define TR_DEVICE_WRONG_REQUEST           TR("장치 접근 오류", "장치 접근에 문제가 있습니다")
#define TR_DEVICE_FILE_REJECTED           TR("파일 거부됨", "장치에서 파일을 거부함")
#define TR_DEVICE_FILE_WRONG_SIG          TR("서명 불일치", "장치 파일 서명이 일치하지 않음")
#define TR_CURRENT_VERSION                TR("현재 버전: ", "현재 펌웨어 버전: ")
#define TR_FLASH_INTERNAL_MODULE          TR("내부 모듈 플래시", "내장 모듈 펌웨어 쓰기")
#define TR_FLASH_INTERNAL_MULTI           TR("내부 Multi 플래시", "내부 멀티모듈 플래시")
#define TR_FLASH_EXTERNAL_MODULE          TR("외부 모듈 플래시", "외장 모듈 펌웨어 쓰기")
#define TR_FLASH_EXTERNAL_MULTI           TR("외부 Multi 플래시", "외부 멀티모듈 플래시")
#define TR_FLASH_EXTERNAL_ELRS            TR("외부 ELRS 플래시", "외장 ELRS 모듈 플래시")
#define TR_FIRMWARE_UPDATE_ERROR          TR("펌웨어 업데이트 오류", "펌웨어 업데이트 중 오류 발생")

// #define TR_FIRMWARE_UPDATE_SUCCESS     "Flash successful"
// #define TR_WRITING                     "Writing..."
// #define TR_CONFIRM_FORMAT              "Confirm Format?"
// #define TR_INTERNALRF                  "Internal RF"
// #define TR_INTERNAL_MODULE             TR("Int. module", "Internal module")
// #define TR_EXTERNAL_MODULE             TR("Ext. module", "External module")
// #define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX upgrade required"
// #define TR_TELEMETRY_DISABLED          "Telem. disabled"
// #define TR_MORE_OPTIONS_AVAILABLE      "More options available"
// #define TR_NO_MODULE_INFORMATION       "No module information"
// #define TR_EXTERNALRF                  "External RF"
// #define TR_FAILSAFE                    TR("Failsafe", "Failsafe mode")
// #define TR_FAILSAFESET                 "FAILSAFE SETTINGS"
// #define TR_REG_ID                      "Reg. ID"
// #define TR_OWNER_ID                    "Owner ID"
// #define TR_HOLD                        "Hold"
// #define TR_HOLD_UPPERCASE              "HOLD"
// #define TR_NONE                        "None"
// #define TR_NONE_UPPERCASE              "NONE"
// #define TR_MENUSENSOR                  "SENSOR"
// #define TR_POWERMETER_PEAK             "Peak"
// #define TR_POWERMETER_POWER            "Power"
// #define TR_POWERMETER_ATTN             "Attn"
// #define TR_POWERMETER_FREQ             "Freq."
// #define TR_MENUTOOLS                   "TOOLS"
#define TR_FIRMWARE_UPDATE_SUCCESS        "플래시 완료"
#define TR_WRITING                        "쓰기 중..."
#define TR_CONFIRM_FORMAT                 "포맷하시겠습니까?"
#define TR_INTERNALRF                     "내장 RF"
#define TR_INTERNAL_MODULE                TR("내장 모듈", "내부 RF 모듈")
#define TR_EXTERNAL_MODULE                TR("외장 모듈", "외부 RF 모듈")
#define TR_EDGETX_UPGRADE_REQUIRED        "EdgeTX 업그레이드 필요"
#define TR_TELEMETRY_DISABLED             "텔레메트리 비활성화됨"
#define TR_MORE_OPTIONS_AVAILABLE         "추가 옵션 사용 가능"
#define TR_NO_MODULE_INFORMATION          "모듈 정보 없음"
#define TR_EXTERNALRF                     "외장 RF"
#define TR_FAILSAFE                       TR("페일세이프", "페일세이프 모드")
#define TR_FAILSAFESET                    "페일세이프 설정"
#define TR_REG_ID                         "등록 ID"
#define TR_OWNER_ID                       "소유자 ID"
#define TR_HOLD                           "홀드"
#define TR_HOLD_UPPERCASE                 "HOLD"
#define TR_NONE                           "없음"
#define TR_NONE_UPPERCASE                 "NONE"
#define TR_MENUSENSOR                     "센서"
#define TR_POWERMETER_PEAK                "최대값"
#define TR_POWERMETER_POWER               "출력"
#define TR_POWERMETER_ATTN                "감쇠"
#define TR_POWERMETER_FREQ                "주파수"
#define TR_MENUTOOLS                      "도구"

// #define TR_TURN_OFF_RECEIVER           "Turn off receiver"
// #define TR_STOPPING                    "Stopping..."
// #define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
// #define TR_MENU_POWER_METER            "POWER METER"
// #define TR_SENSOR                      "SENSOR"
// #define TR_COUNTRY_CODE                "Country code"
// #define TR_USBMODE                     "USB mode"
// #define TR_JACK_MODE                   "Jack mode"
// #define TR_VOICE_LANGUAGE              "Voice language"
// #define TR_UNITS_SYSTEM                "Units"
// #define TR_UNITS_PPM                   "PPM Units"
// #define TR_EDIT                        "Edit"
// #define TR_INSERT_BEFORE               "Insert before"
// #define TR_INSERT_AFTER                "Insert after"
// #define TR_COPY                        "Copy"
// #define TR_MOVE                        "Move"
// #define TR_PASTE                       "Paste"
// #define TR_PASTE_AFTER                 "Paste after"
// #define TR_PASTE_BEFORE                "Paste before"
// #define TR_DELETE                      "Delete"
// #define TR_INSERT                      "Insert"
// #define TR_RESET_FLIGHT                "Reset session"
// #define TR_RESET_TIMER1                "Reset timer1"
// #define TR_RESET_TIMER2                "Reset timer2"
// #define TR_RESET_TIMER3                "Reset timer3"
// #define TR_RESET_TELEMETRY             "Reset telemetry"
// #define TR_STATISTICS                  "Statistics"
// #define TR_ABOUT_US                    "About"
#define TR_TURN_OFF_RECEIVER              "수신기 전원 끄기"
#define TR_STOPPING                       "중지 중..."
#define TR_MENU_SPECTRUM_ANALYSER         "스펙트럼 분석기"
#define TR_MENU_POWER_METER               "전력 측정기"
#define TR_SENSOR                         "센서"
#define TR_COUNTRY_CODE                   "국가 코드"
#define TR_USBMODE                        "USB 모드"
#define TR_JACK_MODE                      "잭 모드"
#define TR_VOICE_LANGUAGE                 "음성 언어"
#define TR_UNITS_SYSTEM                   "단위 체계"
#define TR_UNITS_PPM                      "PPM 단위"
#define TR_EDIT                           "편집"
#define TR_INSERT_BEFORE                  "앞에 삽입"
#define TR_INSERT_AFTER                   "뒤에 삽입"
#define TR_COPY                           "복사"
#define TR_MOVE                           "이동"
#define TR_PASTE                          "붙여넣기"
#define TR_PASTE_AFTER                    "뒤에 붙여넣기"
#define TR_PASTE_BEFORE                   "앞에 붙여넣기"
#define TR_DELETE                         "삭제"
#define TR_INSERT                         "삽입"
#define TR_RESET_FLIGHT                   "세션 초기화"
#define TR_RESET_TIMER1                   "타이머1 초기화"
#define TR_RESET_TIMER2                   "타이머2 초기화"
#define TR_RESET_TIMER3                   "타이머3 초기화"
#define TR_RESET_TELEMETRY                "텔레메트리 초기화"
#define TR_STATISTICS                     "통계"
#define TR_ABOUT_US                       "정보"

// #define TR_USB_JOYSTICK                "USB Joystick (HID)"
// #define TR_USB_MASS_STORAGE            "USB Storage (SD)"
// #define TR_USB_SERIAL                  "USB Serial (VCP)"
// #define TR_SETUP_SCREENS               "Setup screens"
// #define TR_MONITOR_SCREENS             "Monitors"
// #define TR_AND_SWITCH                  "AND switch"
// #define TR_SF                          "SF"
// #define TR_GF                          "GF"
// #define TR_ANADIAGS_CALIB              "Calibrated analogs"
// #define TR_ANADIAGS_FILTRAWDEV         "Filtered raw analogs with deviation"
// #define TR_ANADIAGS_UNFILTRAW          "Unfiltered raw analogs"
// #define TR_ANADIAGS_MINMAX             "Min., max. and range"
// #define TR_ANADIAGS_MOVE               "Move analogs to their extremes!"
// #define TR_SPEAKER                     "Speaker"
// #define TR_BUZZER                      "Buzzer"
// #define TR_BYTES                       "bytes"
// #define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
// #define TR_MODULE_UNBIND               BUTTON("Unbind")
// #define TR_POWERMETER_ATTN_NEEDED     "Attenuator needed"
// #define TR_PXX2_SELECT_RX              "Select RX"
// #define TR_PXX2_DEFAULT                "<default>"
// #define TR_BT_SELECT_DEVICE            "Select device"
// #define TR_DISCOVER                    "Discover"
#define TR_USB_JOYSTICK                   "USB 조이스틱 (HID)"
#define TR_USB_MASS_STORAGE               "USB 저장소 (SD)"
#define TR_USB_SERIAL                     "USB 시리얼 (VCP)"
#define TR_SETUP_SCREENS                  "설정 화면"
#define TR_MONITOR_SCREENS                "모니터 화면"
#define TR_AND_SWITCH                     "AND 스위치"
#define TR_SF                             "특수 기능 (SF)"
#define TR_GF                             "전역 기능 (GF)"
#define TR_ANADIAGS_CALIB                 "보정된 아날로그"
#define TR_ANADIAGS_FILTRAWDEV            "필터링된 원시 아날로그 (편차 포함)"
#define TR_ANADIAGS_UNFILTRAW             "필터링 안 된 원시 아날로그"
#define TR_ANADIAGS_MINMAX                "최소, 최대, 범위"
#define TR_ANADIAGS_MOVE                  "아날로그를 끝까지 움직이세요!"
#define TR_SPEAKER                        "스피커"
#define TR_BUZZER                         "부저"
#define TR_BYTES                          "바이트"
#define TR_MODULE_BIND                    BUTTON(TR("바인드", "바인딩"))
#define TR_MODULE_UNBIND                  BUTTON("바인딩 해제")
#define TR_POWERMETER_ATTN_NEEDED         "감쇠기 필요"
#define TR_PXX2_SELECT_RX                 "수신기 선택"
#define TR_PXX2_DEFAULT                   "<기본값>"
#define TR_BT_SELECT_DEVICE               "기기 선택"
#define TR_DISCOVER                       "검색"

// #define TR_BUTTON_INIT                 BUTTON("Init")
// #define TR_WAITING                     "Waiting..."
// #define TR_RECEIVER_DELETE             "Delete receiver?"
// #define TR_RECEIVER_RESET              "Reset receiver?"
// #define TR_SHARE                       "Share"
// #define TR_BIND                        "Bind"
// #define TR_REGISTER                    TR("Reg", "Register")
// #define TR_MODULE_RANGE                BUTTON(TR("Rng", "Range"))
// #define TR_RANGE_TEST                  "Range Test"
// #define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "RECEIVER OPTIONS")
// #define TR_RESET_BTN                   BUTTON("Reset")
// #define TR_DEBUG                       "Debug"
// #define TR_KEYS_BTN                    BUTTON("Keys")
// #define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "Analogs"))
// #define TR_FS_BTN                      BUTTON(TR("Custom sw", TR_FUNCTION_SWITCHES))
// #define TR_TOUCH_NOTFOUND              "Touch hardware not found"
// #define TR_TOUCH_EXIT                  "Touch screen to exit"
// #define TR_SET                         BUTTON("Set")
// #define TR_TRAINER                     "Trainer"
// #define TR_CHANS                       "Chans"
#define TR_BUTTON_INIT                    BUTTON("초기화")
#define TR_WAITING                        "대기 중..."
#define TR_RECEIVER_DELETE                "수신기를 삭제할까요?"
#define TR_RECEIVER_RESET                 "수신기를 초기화할까요?"
#define TR_SHARE                          "공유"
#define TR_BIND                           "바인딩"
#define TR_REGISTER                       TR("등록", "등록")
#define TR_MODULE_RANGE                   BUTTON(TR("범위", "범위 테스트"))
#define TR_RANGE_TEST                     "범위 테스트"
#define TR_RECEIVER_OPTIONS               TR("수신기 옵션", "RECEIVER OPTIONS")
#define TR_RESET_BTN                      BUTTON("초기화")
#define TR_DEBUG                          "디버그"
#define TR_KEYS_BTN                       BUTTON("버튼")
#define TR_ANALOGS_BTN                    BUTTON(TR("아날로그", "아날로그 입력"))
#define TR_FS_BTN                         BUTTON(TR("사용자 스위치", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND                 "터치 하드웨어를 찾을 수 없습니다."
#define TR_TOUCH_EXIT                     "화면을 터치하여 종료하세요."
#define TR_SET                            BUTTON("설정")
#define TR_TRAINER                        "트레이너"
#define TR_CHANS                          "채널"

// #define TR_ANTENNAPROBLEM              "TX antenna problem!"
// #define TR_MODELIDUSED                 "ID used in:"
// #define TR_MODELIDUNIQUE               "ID is unique"
// #define TR_MODULE                      "Module"
// #define TR_RX_NAME                     "Rx Name"
// #define TR_TELEMETRY_TYPE              TR("Type", "Telemetry type")
// #define TR_TELEMETRY_SENSORS           "Sensors"
// #define TR_VALUE                       "Value"
// #define TR_PERIOD                      "Period"
// #define TR_INTERVAL                    "Interval"
// #define TR_REPEAT                      "Repeat"
// #define TR_ENABLE                      "Enable"
// #define TR_DISABLE                     "Disable"
// #define TR_TOPLCDTIMER                 "Top LCD Timer"
// #define TR_UNIT                        "Unit"
// #define TR_TELEMETRY_NEWSENSOR         "Add new"
// #define TR_CHANNELRANGE                TR("Ch. Range", "Channel Range")
// #define TR_ANTENNACONFIRM1             "EXT. ANTENNA"
#define TR_ANTENNAPROBLEM                 "송신기 안테나 이상!"
#define TR_MODELIDUSED                    "ID 중복됨: 사용 중인 모델:"
#define TR_MODELIDUNIQUE                  "ID가 고유합니다"
#define TR_MODULE                         "모듈"
#define TR_RX_NAME                        "수신기 이름"
#define TR_TELEMETRY_TYPE                 TR("유형", "텔레메트리 유형")
#define TR_TELEMETRY_SENSORS              "센서 목록"
#define TR_VALUE                          "값"
#define TR_PERIOD                         "주기"
#define TR_INTERVAL                       "간격"
#define TR_REPEAT                         "반복"
#define TR_ENABLE                         "사용"
#define TR_DISABLE                        "사용 안 함"
#define TR_TOPLCDTIMER                    "상단 LCD 타이머"
#define TR_UNIT                           "단위"
#define TR_TELEMETRY_NEWSENSOR            "신규 추가"
#define TR_CHANNELRANGE                   TR("채널 범위", "사용할 채널 범위")
#define TR_ANTENNACONFIRM1                "외장 안테나"

#if defined(PCBX12S)
  // #define TR_ANTENNA_MODES             "Internal","Ask","Per model","Internal + External"
  #define TR_ANTENNA_MODES                "내장", "확인 요청", "모델별 설정", "내장 + 외장"
#else
  // #define TR_ANTENNA_MODES             "Internal","Ask","Per model","External"
  #define TR_ANTENNA_MODES                "내장", "확인 요청", "모델별 설정", "외장"
#endif

// #define TR_USE_INTERNAL_ANTENNA        TR("Use int. antenna", "Use internal antenna")
// #define TR_USE_EXTERNAL_ANTENNA        TR("Use ext. antenna", "Use external antenna")
// #define TR_ANTENNACONFIRM2             TR("Check antenna", "Make sure antenna is installed!")
// #define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Requires FLEX non"
// #define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Requires FCC"
// #define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Requires EU"
// #define TR_MODULE_PROTOCOL_WARN_LINE2        "certified firmware"
// #define TR_LOWALARM                    "Low alarm"
// #define TR_CRITICALALARM               "Critical alarm"
// #define TR_DISABLE_ALARM               TR("Disable alarms", "Disable telemetry alarms")
// #define TR_POPUP                       "Popup"
// #define TR_MIN                         "Min"
// #define TR_MAX                         "Max"
// #define TR_CURVE_PRESET                "Preset..."
// #define TR_PRESET                      "Preset"
// #define TR_MIRROR                      "Mirror"
// #define TR_CLEAR                       "Clear"
// #define TR_RESET                       "Reset"
// #define TR_RESET_SUBMENU               "Reset..."
// #define TR_COUNT                       "Count"
#define TR_USE_INTERNAL_ANTENNA           TR("내장 안테나 사용", "내부 안테나 사용")
#define TR_USE_EXTERNAL_ANTENNA           TR("외장 안테나 사용", "외부 안테나 사용")
#define TR_ANTENNACONFIRM2                TR("안테나 확인", "안테나가 연결되었는지 확인하세요!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1  "FLEX 인증 필요"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1   "FCC 인증 필요"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1    "EU 인증 필요"
#define TR_MODULE_PROTOCOL_WARN_LINE2       "인증된 펌웨어 필요"
#define TR_LOWALARM                       "저전압 경고"
#define TR_CRITICALALARM                  "위험 경고"
#define TR_DISABLE_ALARM                  TR("경고 끄기", "텔레메트리 경고 끄기")
#define TR_POPUP                          "팝업"
#define TR_MIN                            "최소"
#define TR_MAX                            "최대"
#define TR_CURVE_PRESET                   "프리셋..."
#define TR_PRESET                         "프리셋"
#define TR_MIRROR                         "미러"
#define TR_CLEAR                          "지우기"
#define TR_RESET                          "초기화"
#define TR_RESET_SUBMENU                  "초기화..."
#define TR_COUNT                          "카운트"

// #define TR_PT                          "pt"
// #define TR_PTS                         "pts"
// #define TR_SMOOTH                      "Smooth"
// #define TR_COPY_STICKS_TO_OFS          TR("Cpy stick->subtrim", "Copy axis to subtrim")
// #define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
// #define TR_COPY_TRIMS_TO_OFS           TR("Cpy trim->subtrim", "Copy trims to subtrim")
// #define TR_INCDEC                      "Inc/Decrement"
// #define TR_GLOBALVAR                   "Global var"
// #define TR_MIXSOURCE                   "Source (%)"
// #define TR_MIXSOURCERAW                "Source (value)"
// #define TR_CONSTANT                    "Constant"
// #define TR_PREFLIGHT_POTSLIDER_CHECK   "OFF","ON","AUTO"
// #define TR_PREFLIGHT                   "Pre-start Checks"
// #define TR_CHECKLIST                   TR("Checklist", "Display checklist")
// #define TR_CHECKLIST_INTERACTIVE       TR3("C-Interact", "Interact. checklist", "Interactive checklist")
// #define TR_AUX_SERIAL_MODE             "Serial port"
// #define TR_AUX2_SERIAL_MODE            "Serial port 2"
// #define TR_AUX_SERIAL_PORT_POWER       "Port power"
// #define TR_SCRIPT                      "Script"
#define TR_PT                             "pt"
#define TR_PTS                            "pts"
#define TR_SMOOTH                         "부드럽게"
#define TR_COPY_STICKS_TO_OFS             TR("스틱 → 서브트림", "스틱 위치를 서브트림으로 복사")
#define TR_COPY_MIN_MAX_TO_OUTPUTS        TR("최소/최대 → 출력", "최소/최대/중앙값을 전체 출력에 복사")
#define TR_COPY_TRIMS_TO_OFS              TR("트림 → 서브트림", "트림 값을 서브트림으로 복사")
#define TR_INCDEC                         "증가/감소"
#define TR_GLOBALVAR                      "전역 변수"
#define TR_MIXSOURCE                      "소스 (%)"
#define TR_MIXSOURCERAW                   "소스 (값)"
#define TR_CONSTANT                       "상수"
#define TR_PREFLIGHT_POTSLIDER_CHECK      "끔", "켬", "자동"
#define TR_PREFLIGHT                      "비행 전 점검"
#define TR_CHECKLIST                      TR("체크리스트", "체크리스트 보기")
#define TR_CHECKLIST_INTERACTIVE          TR3("체크리스트 상호작용", "상호작용 체크리스트", "인터랙티브 체크리스트")
#define TR_AUX_SERIAL_MODE                "보조 시리얼 포트"
#define TR_AUX2_SERIAL_MODE               "보조 시리얼 포트 2"
#define TR_AUX_SERIAL_PORT_POWER          "포트 전원"
#define TR_SCRIPT                         "스크립트"

// #define TR_INPUTS                      "Inputs"
// #define TR_OUTPUTS                     "Outputs"
// #define STR_EEBACKUP                   "EEPROM backup"
// #define STR_FACTORYRESET               "Factory reset"
// #define TR_CONFIRMRESET                TR("Erase ALL", "Erase ALL models and settings?")
// #define TR_TOO_MANY_LUA_SCRIPTS        "Too many Lua scripts!"
// #define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
// #define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
// #define TR_NO_TELEMETRY_SCREENS        "No Telemetry Screens"
// #define TR_TOUCH_PANEL                 "Touch panel:"
// #define TR_FILE_SIZE                   "File size"
// #define TR_FILE_OPEN                   "Open anyway?"
// #define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}
#define TR_INPUTS                         "입력"
#define TR_OUTPUTS                        "출력"
#define STR_EEBACKUP                      "EEPROM 백업"
#define STR_FACTORYRESET                  "공장 초기화"
#define TR_CONFIRMRESET                   TR("전체 삭제", "모델과 설정을 모두 삭제할까요?")
#define TR_TOO_MANY_LUA_SCRIPTS           "Lua 스크립트가 너무 많습니다!"
#define TR_SPORT_UPDATE_POWER_MODE        "S.Port 전원"
#define TR_SPORT_UPDATE_POWER_MODES       "자동", "항상 켜짐"
#define TR_NO_TELEMETRY_SCREENS           "텔레메트리 화면 없음"
#define TR_TOUCH_PANEL                    "터치 패널:"
#define TR_FILE_SIZE                      "파일 크기"
#define TR_FILE_OPEN                      "강제로 열까요?"
#define TR_TIMER_MODES                    {TR_OFFON, TR_START, TR_THROTTLE_LABEL, TR_THROTTLE_PERCENT_LABEL, TR_THROTTLE_START}

// Horus and Taranis column headers
// #define TR_PHASES_HEADERS_NAME         "Name"
// #define TR_PHASES_HEADERS_SW           "Switch"
// #define TR_PHASES_HEADERS_RUD_TRIM     "Rudder Trim"
// #define TR_PHASES_HEADERS_ELE_TRIM     "Elevator Trim"
// #define TR_PHASES_HEADERS_THT_TRIM     "Throttle Trim"
// #define TR_PHASES_HEADERS_AIL_TRIM     "Aileron Trim"
// #define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
// #define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
// #define TR_PHASES_HEADERS_FAD_IN       "Fade In"
// #define TR_PHASES_HEADERS_FAD_OUT      "Fade Out"
#define TR_PHASES_HEADERS_NAME            "이름"
#define TR_PHASES_HEADERS_SW              "스위치"
#define TR_PHASES_HEADERS_RUD_TRIM        "러더 트림"
#define TR_PHASES_HEADERS_ELE_TRIM        "엘리베이터 트림"
#define TR_PHASES_HEADERS_THT_TRIM        "스로틀 트림"
#define TR_PHASES_HEADERS_AIL_TRIM        "에일러론 트림"
#define TR_PHASES_HEADERS_CH5_TRIM        "트림 5"
#define TR_PHASES_HEADERS_CH6_TRIM        "트림 6"
#define TR_PHASES_HEADERS_FAD_IN          "페이드 인"
#define TR_PHASES_HEADERS_FAD_OUT         "페이드 아웃"

// #define TR_LIMITS_HEADERS_NAME         "Name"
// #define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
// #define TR_LIMITS_HEADERS_MIN          "Min"
// #define TR_LIMITS_HEADERS_MAX          "Max"
// #define TR_LIMITS_HEADERS_DIRECTION    "Direction"
// #define TR_LIMITS_HEADERS_CURVE        "Curve"
// #define TR_LIMITS_HEADERS_PPMCENTER    "PPM Center"
// #define TR_LIMITS_HEADERS_SUBTRIMMODE  "Subtrim mode"
// #define TR_INVERTED                    "Inverted"
#define TR_LIMITS_HEADERS_NAME            "이름"
#define TR_LIMITS_HEADERS_SUBTRIM         "서브트림"
#define TR_LIMITS_HEADERS_MIN             "최소값"
#define TR_LIMITS_HEADERS_MAX             "최대값"
#define TR_LIMITS_HEADERS_DIRECTION       "방향"
#define TR_LIMITS_HEADERS_CURVE           "곡선"
#define TR_LIMITS_HEADERS_PPMCENTER       "PPM 중심값"
#define TR_LIMITS_HEADERS_SUBTRIMMODE     "서브트림 모드"
#define TR_INVERTED                       "반전됨"

// #define TR_LSW_DESCRIPTIONS            { "Comparison type or function", "First variable", "Second variable or constant", "Second variable or constant", "Additional condition for line to be enabled", "Minimum ON duration of the logical switch", "Minimum TRUE duration for the switch to become ON" }
#define TR_LSW_DESCRIPTIONS               { "비교 방식 또는 기능", "첫 번째 변수", "두 번째 변수 또는 상수", "두 번째 변수 또는 상수", "라인이 활성화되기 위한 추가 조건", "스위치 ON 유지 최소 시간", "ON 되기 위한 조건 지속 시간" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  // #define TR_FIRST_CHANNEL             "First channel"
  // #define TR_FILL_BACKGROUND           "Fill background?"
  // #define TR_BG_COLOR                  "BG Color"
  // #define TR_SLIDERS_TRIMS             "Sliders+Trims"
  // #define TR_SLIDERS                   "Sliders"
  // #define TR_FLIGHT_MODE               "Flight mode"
  // #define TR_INVALID_FILE              "Invalid File"
  // #define TR_TIMER_SOURCE              "Timer source"
  // #define TR_SIZE                      "Size"
  // #define TR_SHADOW                    "Shadow"
  // #define TR_ALIGNMENT                 "Alignment"
  // #define TR_ALIGN_LABEL               "Align label"
  // #define TR_ALIGN_VALUE               "Align value"
  // #define TR_ALIGN_OPTS                { "Left", "Center", "Right" }
  // #define TR_TEXT                      "Text"
  // #define TR_COLOR                     "Color"
  // #define TR_MAIN_VIEW_X               "Main view XX"
  // #define TR_PANEL1_BACKGROUND         "Panel1 background"
  // #define TR_PANEL2_BACKGROUND         "Panel2 background"
  // #define TR_WIDGET_GAUGE              "Gauge"
  // #define TR_WIDGET_MODELBMP           "Model info"
  // #define TR_WIDGET_OUTPUTS            "Outputs"
  // #define TR_WIDGET_TEXT               "Text"
  // #define TR_WIDGET_TIMER              "Timer"
  // #define TR_WIDGET_VALUE              "Value"
  #define TR_FIRST_CHANNEL                "첫 번째 채널"
  #define TR_FILL_BACKGROUND              "배경 채우기?"
  #define TR_BG_COLOR                     "배경 색상"
  #define TR_SLIDERS_TRIMS                "슬라이더 + 트림"
  #define TR_SLIDERS                      "슬라이더"
  #define TR_FLIGHT_MODE                  "비행 모드"
  #define TR_INVALID_FILE                 "잘못된 파일"
  #define TR_TIMER_SOURCE                 "타이머 소스"
  #define TR_SIZE                         "크기"
  #define TR_SHADOW                       "그림자"
  #define TR_ALIGNMENT                    "정렬"
  #define TR_ALIGN_LABEL                  "레이블 정렬"
  #define TR_ALIGN_VALUE                  "값 정렬"
  #define TR_ALIGN_OPTS                   { "왼쪽", "가운데", "오른쪽" }
  #define TR_TEXT                         "텍스트"
  #define TR_COLOR                        "색상"
  #define TR_MAIN_VIEW_X                  "메인 뷰 X축"
  #define TR_PANEL1_BACKGROUND            "패널1 배경"
  #define TR_PANEL2_BACKGROUND            "패널2 배경"
  #define TR_WIDGET_GAUGE                 "게이지"
  #define TR_WIDGET_MODELBMP              "모델 정보"
  #define TR_WIDGET_OUTPUTS               "출력"
  #define TR_WIDGET_TEXT                  "텍스트"
  #define TR_WIDGET_TIMER                 "타이머"
  #define TR_WIDGET_VALUE                 "값"
#endif

// Bootloader common - ASCII characters only
// #define TR_BL_USB_CONNECTED           "USB Connected"
// #define TR_BL_USB_PLUGIN              "Or plug in a USB cable"
// #define TR_BL_USB_MASS_STORE          "for mass storage"
// #define TR_BL_USB_PLUGIN_MASS_STORE   "Or plug in a USB cable for mass storage"
// #define TR_BL_WRITE_FW                "Write Firmware"
// #define TR_BL_FORK                    "Fork:"
// #define TR_BL_VERSION                 "Version:"
// #define TR_BL_RADIO                   "Radio:"
// #define TR_BL_EXIT                    "Exit"
// #define TR_BL_DIR_MISSING             " Directory is missing"
// #define TR_BL_DIR_EMPTY               " Directory is empty"
// #define TR_BL_WRITING_FW              "Writing..."
// #define TR_BL_WRITING_COMPL           "Writing complete"
#define TR_BL_USB_CONNECTED               "USB 연결됨"
#define TR_BL_USB_PLUGIN                  "또는 USB 케이블을 연결하세요"
#define TR_BL_USB_MASS_STORE              "대용량 저장 장치 모드"
#define TR_BL_USB_PLUGIN_MASS_STORE       "또는 USB 케이블을 연결하여 저장 모드 진입"
#define TR_BL_WRITE_FW                    "펌웨어 쓰기"
#define TR_BL_FORK                        "포크:"
#define TR_BL_VERSION                     "버전:"
#define TR_BL_RADIO                       "기종:"
#define TR_BL_EXIT                        "종료"
#define TR_BL_DIR_MISSING                 " 디렉토리가 없습니다"
#define TR_BL_DIR_EMPTY                   " 디렉토리가 비어 있습니다"
#define TR_BL_WRITING_FW                  "펌웨어 쓰는 중..."
#define TR_BL_WRITING_COMPL               "펌웨어 쓰기 완료"

#if LCD_W >= 480
  // #define TR_BL_INVALID_FIRMWARE       "Not a valid firmware file"
  #define TR_BL_INVALID_FIRMWARE          "유효하지 않은 펌웨어 파일입니다"
#elif LCD_W >= 212
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN_MASS_STORE
  // #define TR_BL_HOLD_ENTER_TO_START    "\012Hold [ENT] to start writing"
  // #define TR_BL_INVALID_FIRMWARE       "\011Not a valid firmware file!        "
  // #define TR_BL_INVALID_EEPROM         "\011Not a valid EEPROM file!          "
  #define TR_BL_HOLD_ENTER_TO_START     "\012[ENT]를 길게 눌러 쓰기 시작"
  #define TR_BL_INVALID_FIRMWARE        "\011유효하지 않은 펌웨어 파일입니다"
  #define TR_BL_INVALID_EEPROM          "\011유효하지 않은 EEPROM 파일입니다"
#else
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
  // #define TR_BL_HOLD_ENTER_TO_START    "\006Hold [ENT] to start"
  // #define TR_BL_INVALID_FIRMWARE       "\004Not a valid firmware!        "
  // #define TR_BL_INVALID_EEPROM         "\004Not a valid EEPROM!          "
  #define TR_BL_HOLD_ENTER_TO_START     "\006[ENT] 길게 눌러 시작"
  #define TR_BL_INVALID_FIRMWARE        "\004펌웨어 파일 오류!"
  #define TR_BL_INVALID_EEPROM          "\004EEPROM 파일 오류!"
#endif

// Bootloader Taranis specific - ASCII characters only
// #define TR_BL_RESTORE_EEPROM           "Restore EEPROM"
// #define TR_BL_POWER_KEY                "Press the power button."
// #define TR_BL_FLASH_EXIT               "Exit the flashing mode."
#define TR_BL_RESTORE_EEPROM            "EEPROM 복원"
#define TR_BL_POWER_KEY                 "전원 버튼을 누르세요."
#define TR_BL_FLASH_EXIT                "플래시 모드를 종료합니다."

// Bootloader Horus specific - ASCII characters only
// #define TR_BL_ERASE_INT_FLASH          "Erase Internal Flash Storage"
// #define TR_BL_ERASE_FLASH              "Erase Flash Storage"
// #define TR_BL_ERASE_FLASH_MSG          "This may take up to 200s"
// #define TR_BL_RF_USB_ACCESS            "RF USB access"
// #define TR_BL_CURRENT_FW               "Current Firmware:"
#define TR_BL_ERASE_INT_FLASH         "내장 플래시 저장소 삭제"
#define TR_BL_ERASE_FLASH             "플래시 저장소 삭제"
#define TR_BL_ERASE_FLASH_MSG         "최대 200초까지 소요될 수 있습니다"
#define TR_BL_RF_USB_ACCESS           "RF USB 접근"
#define TR_BL_CURRENT_FW              "현재 펌웨어:"

#if defined(PCBPL18)
  // Bootloader PL18/NB4+ specific - ASCII characters only
  // #define TR_BL_ENABLE                 "Enable"
  // #define TR_BL_DISABLE                "Disable"
  #define TR_BL_ENABLE                "사용"
  #define TR_BL_DISABLE               "사용 안 함"
  #if defined(RADIO_NV14_FAMILY)
    // #define TR_BL_SELECT_KEY           "[R TRIM] to select file"
    // #define TR_BL_FLASH_KEY            "Hold [R TRIM] long to flash"
    // #define TR_BL_ERASE_KEY            "Hold [R TRIM] long to erase"
    // #define TR_BL_EXIT_KEY             "[L TRIM] to exit"
    #define TR_BL_SELECT_KEY          "[R TRIM] - 파일 선택"
    #define TR_BL_FLASH_KEY           "[R TRIM 길게] - 플래시 실행"
    #define TR_BL_ERASE_KEY           "[R TRIM 길게] - 지우기 실행"
    #define TR_BL_EXIT_KEY            "[L TRIM] - 종료"
  #elif defined(RADIO_NB4P)
    // #define TR_BL_SELECT_KEY           "[SW1A] to select file"
    // #define TR_BL_FLASH_KEY            "Hold [SW1A] long to flash"
    // #define TR_BL_ERASE_KEY            "Hold [SW1A] long to erase"
    // #define TR_BL_EXIT_KEY             "[SW1B] to exit"
    #define TR_BL_SELECT_KEY          "[SW1A] - 파일 선택"
    #define TR_BL_FLASH_KEY           "[SW1A 길게] - 플래시 실행"
    #define TR_BL_ERASE_KEY           "[SW1A 길게] - 지우기 실행"
    #define TR_BL_EXIT_KEY            "[SW1B] - 종료"
  #else
    // #define TR_BL_SELECT_KEY           "[TR4 Dn] to select file"
    // #define TR_BL_FLASH_KEY            "Hold [TR4 Dn] long to flash"
    // #define TR_BL_ERASE_KEY            "Hold [TR4 Dn] long to erase"
    // #define TR_BL_EXIT_KEY             "[TR4 Up] to exit"
    #define TR_BL_SELECT_KEY          "[TR4 아래] - 파일 선택"
    #define TR_BL_FLASH_KEY           "[TR4 아래 길게] - 플래시 실행"
    #define TR_BL_ERASE_KEY           "[TR4 아래 길게] - 지우기 실행"
    #define TR_BL_EXIT_KEY            "[TR4 위] - 종료"
  #endif
#elif defined(PCBNV14)
  // #define TR_BL_SELECT_KEY             "[R TRIM] to select file"
  // #define TR_BL_FLASH_KEY              "Hold [R TRIM] long to flash"
  // #define TR_BL_EXIT_KEY               " [L TRIM] to exit"
  // #define TR_BL_ENABLE                 "Enable"
  // #define TR_BL_DISABLE                "Disable"
  #define TR_BL_SELECT_KEY            "[R TRIM] - 파일 선택"
  #define TR_BL_FLASH_KEY             "[R TRIM 길게] - 플래시 실행"
  #define TR_BL_EXIT_KEY              "[L TRIM] - 종료"
  #define TR_BL_ENABLE                "사용"
  #define TR_BL_DISABLE               "사용 안 함"
#else
  // #define TR_BL_SELECT_KEY             "[ENT] to select file"
  // #define TR_BL_FLASH_KEY              "Hold [ENT] long to flash"
  // #define TR_BL_ERASE_KEY              "Hold [ENT] long to erase"
  // #define TR_BL_EXIT_KEY               "[RTN] to exit"
  #define TR_BL_SELECT_KEY            "[ENT] - 파일 선택"
  #define TR_BL_FLASH_KEY             "[ENT 길게] - 플래시 실행"
  #define TR_BL_ERASE_KEY             "[ENT 길게] - 지우기 실행"
  #define TR_BL_EXIT_KEY              "[RTN] - 종료"
#endif

// About screen
// #define TR_ABOUTUS                     TR(" ABOUT ", "ABOUT")
#define TR_ABOUTUS                    TR(" 정보 ", "정보")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

// #define TR_BEEP_VOLUME                 "Beep volume"
// #define TR_WAV_VOLUME                  "Wav volume"
// #define TR_BG_VOLUME                   TR("Bg volume", "Background volume")
#define TR_BEEP_VOLUME                "비프음 볼륨"
#define TR_WAV_VOLUME                 "음성 파일 볼륨"
#define TR_BG_VOLUME                  TR("배경음 볼륨", "배경 사운드 볼륨")

// #define TR_TOP_BAR                     "Top bar"
// #define TR_FLASH_ERASE                 "Flash erase..."
// #define TR_FLASH_WRITE                 "Flash write..."
// #define TR_OTA_UPDATE                  "OTA update..."
// #define TR_MODULE_RESET                "Module reset..."
// #define TR_UNKNOWN_RX                  "Unknown RX"
// #define TR_UNSUPPORTED_RX              "Unsupported RX"
// #define TR_OTA_UPDATE_ERROR            "OTA update error"
// #define TR_DEVICE_RESET                "Device reset..."
// #define TR_ALTITUDE                    "Altitude"
// #define TR_SCALE                       "Scale"
// #define TR_VIEW_CHANNELS               "View Channels"
// #define TR_VIEW_NOTES                  "View Notes"
// #define TR_MODEL_SELECT                "Model Select"
// #define TR_ID                          "ID"
// #define TR_PRECISION                   "Precision"
// #define TR_RATIO                       "Ratio"
// #define TR_FORMULA                     "Formula"
// #define TR_CELLINDEX                   "Cell index"
// #define TR_LOGS                        "Logs"
// #define TR_OPTIONS                     "Options"
// #define TR_FIRMWARE_OPTIONS            "Firmware options"
#define TR_TOP_BAR                        "상단 바"
#define TR_FLASH_ERASE                    "플래시 지우는 중..."
#define TR_FLASH_WRITE                    "플래시 쓰는 중..."
#define TR_OTA_UPDATE                     "OTA 업데이트 중..."
#define TR_MODULE_RESET                   "모듈 초기화 중..."
#define TR_UNKNOWN_RX                     "알 수 없는 수신기"
#define TR_UNSUPPORTED_RX                 "지원되지 않는 수신기"
#define TR_OTA_UPDATE_ERROR               "OTA 업데이트 오류"
#define TR_DEVICE_RESET                   "장치 초기화 중..."
#define TR_ALTITUDE                       "고도"
#define TR_SCALE                          "스케일"
#define TR_VIEW_CHANNELS                  "채널 보기"
#define TR_VIEW_NOTES                     "노트 보기"
#define TR_MODEL_SELECT                   "모델 선택"
#define TR_ID                             "ID"
#define TR_PRECISION                      "정밀도"
#define TR_RATIO                          "비율"
#define TR_FORMULA                        "공식"
#define TR_CELLINDEX                      "셀 인덱스"
#define TR_LOGS                           "로그"
#define TR_OPTIONS                        "옵션"
#define TR_FIRMWARE_OPTIONS               "펌웨어 옵션"

// #define TR_ALTSENSOR                   "Alt sensor"
// #define TR_CELLSENSOR                  "Cell sensor"
// #define TR_GPSSENSOR                   "GPS sensor"
// #define TR_CURRENTSENSOR               "Sensor"
// #define TR_AUTOOFFSET                  "Auto Offset"
// #define TR_ONLYPOSITIVE                "Positive"
// #define TR_FILTER                      "Filter"
// #define TR_TELEMETRYFULL               TR("All slots full!", "All telemetry slots full!")
// #define TR_IGNORE_INSTANCE             TR("No inst.", "Ignore instances")
// #define TR_SHOW_INSTANCE_ID            "Show instance ID"
// #define TR_DISCOVER_SENSORS            "Discover new"
// #define TR_STOP_DISCOVER_SENSORS       "Stop"
// #define TR_DELETE_ALL_SENSORS          "Delete all"
// #define TR_CONFIRMDELETE               "Really " LCDW_128_LINEBREAK "delete all ?"
// #define TR_SELECT_WIDGET               "Select widget"
// #define TR_WIDGET_FULLSCREEN           "Full screen"
// #define TR_REMOVE_WIDGET               "Remove widget"
// #define TR_WIDGET_SETTINGS             "Widget settings"
// #define TR_REMOVE_SCREEN               "Remove screen"
// #define TR_SETUP_WIDGETS               "Setup widgets"
// #define TR_USER_INTERFACE              "User interface"
#define TR_ALTSENSOR                      "고도 센서"
#define TR_CELLSENSOR                     "셀 전압 센서"
#define TR_GPSSENSOR                      "GPS 센서"
#define TR_CURRENTSENSOR                  "전류 센서"
#define TR_AUTOOFFSET                     "자동 오프셋"
#define TR_ONLYPOSITIVE                   "양수만"
#define TR_FILTER                         "필터"
#define TR_TELEMETRYFULL                  TR("슬롯 가득 참!", "텔레메트리 슬롯이 모두 찼습니다!")
#define TR_IGNORE_INSTANCE                TR("인스턴스 무시", "인스턴스 무시")
#define TR_SHOW_INSTANCE_ID               "인스턴스 ID 표시"
#define TR_DISCOVER_SENSORS               "새 센서 탐색"
#define TR_STOP_DISCOVER_SENSORS          "탐색 중지"
#define TR_DELETE_ALL_SENSORS             "모든 센서 삭제"
#define TR_CONFIRMDELETE                  "정말로 " LCDW_128_LINEBREAK "모두 삭제할까요?"
#define TR_SELECT_WIDGET                  "위젯 선택"
#define TR_WIDGET_FULLSCREEN              "전체 화면"
#define TR_REMOVE_WIDGET                  "위젯 제거"
#define TR_WIDGET_SETTINGS                "위젯 설정"
#define TR_REMOVE_SCREEN                  "화면 제거"
#define TR_SETUP_WIDGETS                  "위젯 구성"
#define TR_USER_INTERFACE                 "사용자 인터페이스"

// #define TR_THEME                       "Theme"
// #define TR_SETUP                       "Setup"
// #define TR_LAYOUT                      "Layout"
// #define TR_ADD_MAIN_VIEW               "Add main view"
// #define TR_TEXT_COLOR                  "Text color"
// #define TR_MENU_INPUTS                 STR_CHAR_INPUT "Inputs"
// #define TR_MENU_LUA                    STR_CHAR_LUA "Lua scripts"
// #define TR_MENU_STICKS                 STR_CHAR_STICK "Axis"
// #define TR_MENU_POTS                   STR_CHAR_POT "Pots"
// #define TR_MENU_MIN                    STR_CHAR_FUNCTION "MIN"
// #define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
// #define TR_MENU_HELI                   STR_CHAR_CYC "Cyclic"
// #define TR_MENU_TRIMS                  STR_CHAR_TRIM "Trims"
// #define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Switches"
// #define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Logical switches"
// #define TR_MENU_TRAINER                STR_CHAR_TRAINER "Trainer"
// #define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Channels"
// #define TR_MENU_GVARS                  STR_CHAR_SLIDER "GVars"
// #define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Telemetry"
// #define TR_MENU_DISPLAY                "DISPLAY"
// #define TR_MENU_OTHER                  "Other"
// #define TR_MENU_INVERT                 "Invert"
#define TR_THEME                          "테마"
#define TR_SETUP                          "설정"
#define TR_LAYOUT                         "레이아웃"
#define TR_ADD_MAIN_VIEW                  "메인 화면 추가"
#define TR_TEXT_COLOR                     "텍스트 색상"
#define TR_MENU_INPUTS                    STR_CHAR_INPUT "입력"
#define TR_MENU_LUA                       STR_CHAR_LUA "Lua 스크립트"
#define TR_MENU_STICKS                    STR_CHAR_STICK "스틱"
#define TR_MENU_POTS                      STR_CHAR_POT "포트"
#define TR_MENU_MIN                       STR_CHAR_FUNCTION "최소값"
#define TR_MENU_MAX                       STR_CHAR_FUNCTION "최대값"
#define TR_MENU_HELI                      STR_CHAR_CYC "사이클릭"
#define TR_MENU_TRIMS                     STR_CHAR_TRIM "트림"
#define TR_MENU_SWITCHES                  STR_CHAR_SWITCH "스위치"
#define TR_MENU_LOGICAL_SWITCHES          STR_CHAR_SWITCH "로직 스위치"
#define TR_MENU_TRAINER                   STR_CHAR_TRAINER "트레이너"
#define TR_MENU_CHANNELS                  STR_CHAR_CHANNEL "채널"
#define TR_MENU_GVARS                     STR_CHAR_SLIDER "GVAR"
#define TR_MENU_TELEMETRY                 STR_CHAR_TELEMETRY "텔레메트리"
#define TR_MENU_DISPLAY                   "디스플레이"
#define TR_MENU_OTHER                     "기타"
#define TR_MENU_INVERT                    "반전"

// #define TR_AUDIO_MUTE                  TR("Audio mute","Mute if no sound")
// #define TR_JITTER_FILTER               "ADC filter"
// #define TR_DEAD_ZONE                   "Dead zone"
// #define TR_RTC_CHECK                   TR("Check RTC", "Check RTC voltage")
// #define TR_AUTH_FAILURE                "Auth-failure"
// #define TR_RACING_MODE                 "Racing mode"
#define TR_AUDIO_MUTE                     TR("오디오 음소거", "소리가 없을 때 음소거")
#define TR_JITTER_FILTER                  "ADC 필터"
#define TR_DEAD_ZONE                      "데드존"
#define TR_RTC_CHECK                      TR("RTC 확인", "RTC 전압 확인")
#define TR_AUTH_FAILURE                   "인증 실패"
#define TR_RACING_MODE                    "레이싱 모드"

// #define TR_USE_THEME_COLOR              "Use theme color"
#define TR_USE_THEME_COLOR                "테마 색상 사용"

// #define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Add all Trims to Subtrims"
#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS      "모든 트림 → 서브트림 복사"
#if !PORTRAIT_LCD
  // #define TR_OPEN_CHANNEL_MONITORS        "Open Channel Monitor"
  #define TR_OPEN_CHANNEL_MONITORS        "채널 모니터 열기"
#else
  // #define TR_OPEN_CHANNEL_MONITORS        "Open Channel Mon."
  #define TR_OPEN_CHANNEL_MONITORS        "채널 모니터 열기"
#endif

// #define TR_DUPLICATE                    "Duplicate"
// #define TR_ACTIVATE                     "Set Active"
// #define TR_RED                          "Red"
// #define TR_BLUE                         "Blue"
// #define TR_GREEN                        "Green"
// #define TR_COLOR_PICKER                 "Color Picker"
// #define TR_EDIT_THEME_DETAILS           "Edit Theme Details"
// #define TR_THEME_COLOR_DEFAULT         "DEFAULT"
// #define TR_THEME_COLOR_PRIMARY1        "PRIMARY1"
// #define TR_THEME_COLOR_PRIMARY2        "PRIMARY2"
// #define TR_THEME_COLOR_PRIMARY3        "PRIMARY3"
// #define TR_THEME_COLOR_SECONDARY1      "SECONDARY1"
// #define TR_THEME_COLOR_SECONDARY2      "SECONDARY2"
// #define TR_THEME_COLOR_SECONDARY3      "SECONDARY3"
// #define TR_THEME_COLOR_FOCUS           "FOCUS"
// #define TR_THEME_COLOR_EDIT            "EDIT"
// #define TR_THEME_COLOR_ACTIVE          "ACTIVE"
// #define TR_THEME_COLOR_WARNING         "WARNING"
// #define TR_THEME_COLOR_DISABLED        "DISABLED"
// #define TR_THEME_COLOR_CUSTOM          "CUSTOM"
// #define TR_THEME_CHECKBOX              "Checkbox"
// #define TR_THEME_ACTIVE                "Active"
// #define TR_THEME_REGULAR               "Regular"
// #define TR_THEME_WARNING               "Warning"
// #define TR_THEME_DISABLED              "Disabled"
// #define TR_THEME_EDIT                  "Edit"
// #define TR_THEME_FOCUS                 "Focus"
// #define TR_AUTHOR                       "Author"
// #define TR_DESCRIPTION                  "Description"
// #define TR_SAVE                         "Save"
// #define TR_CANCEL                       "Cancel"
// #define TR_EDIT_THEME                   "EDIT THEME"
// #define TR_DETAILS                      "Details"
// #define TR_THEME_EDITOR                 "THEMES"
#define TR_DUPLICATE                      "복제"
#define TR_ACTIVATE                       "활성화"
#define TR_RED                            "빨강"
#define TR_BLUE                           "파랑"
#define TR_GREEN                          "초록"
#define TR_COLOR_PICKER                   "색상 선택"
#define TR_EDIT_THEME_DETAILS             "테마 세부 설정"
#define TR_THEME_COLOR_DEFAULT            "기본"
#define TR_THEME_COLOR_PRIMARY1           "기본색 1"
#define TR_THEME_COLOR_PRIMARY2           "기본색 2"
#define TR_THEME_COLOR_PRIMARY3           "기본색 3"
#define TR_THEME_COLOR_SECONDARY1         "보조색 1"
#define TR_THEME_COLOR_SECONDARY2         "보조색 2"
#define TR_THEME_COLOR_SECONDARY3         "보조색 3"
#define TR_THEME_COLOR_FOCUS              "포커스"
#define TR_THEME_COLOR_EDIT               "편집 중"
#define TR_THEME_COLOR_ACTIVE             "활성 항목"
#define TR_THEME_COLOR_WARNING            "경고"
#define TR_THEME_COLOR_DISABLED           "비활성"
#define TR_THEME_COLOR_CUSTOM             "사용자 정의"
#define TR_THEME_CHECKBOX                 "체크박스"
#define TR_THEME_ACTIVE                   "활성"
#define TR_THEME_REGULAR                  "기본"
#define TR_THEME_WARNING                  "경고"
#define TR_THEME_DISABLED                 "비활성"
#define TR_THEME_EDIT                     "편집"
#define TR_THEME_FOCUS                    "포커스"
#define TR_AUTHOR                         "제작자"
#define TR_DESCRIPTION                    "설명"
#define TR_SAVE                           "저장"
#define TR_CANCEL                         "취소"
#define TR_EDIT_THEME                     "테마 편집"
#define TR_DETAILS                        "세부 설정"
#define TR_THEME_EDITOR                   "테마"

// Main menu
// #define TR_MAIN_MENU_SELECT_MODEL       "Select\nModel"
// #define TR_MAIN_MENU_MANAGE_MODELS      "Manage\nModels"
// #define TR_MAIN_MENU_MODEL_NOTES        "Model\nNotes"
// #define TR_MAIN_MENU_CHANNEL_MONITOR    "Channel\nMonitor"
// #define TR_MAIN_MENU_MODEL_SETTINGS     "Model\nSettings"
// #define TR_MAIN_MENU_RADIO_SETTINGS     "Radio\nSettings"
// #define TR_MAIN_MENU_SCREEN_SETTINGS    "Screens\nSettings"
// #define TR_MAIN_MENU_RESET_TELEMETRY    "Reset\nTelemetry"
// #define TR_MAIN_MENU_STATISTICS         "Statistics"
// #define TR_MAIN_MENU_ABOUT_EDGETX       "About\nEdgeTX"
#define TR_MAIN_MENU_SELECT_MODEL         "모델\n선택"
#define TR_MAIN_MENU_MANAGE_MODELS        "모델\n관리"
#define TR_MAIN_MENU_MODEL_NOTES          "모델\n노트"
#define TR_MAIN_MENU_CHANNEL_MONITOR      "채널\n모니터"
#define TR_MAIN_MENU_MODEL_SETTINGS       "모델\n설정"
#define TR_MAIN_MENU_RADIO_SETTINGS       "조종기\n설정"
#define TR_MAIN_MENU_SCREEN_SETTINGS      "화면\n설정"
#define TR_MAIN_MENU_RESET_TELEMETRY      "텔레메트리\n초기화"
#define TR_MAIN_MENU_STATISTICS           "비행\n기록"
#define TR_MAIN_MENU_ABOUT_EDGETX         "EdgeTX\n정보"

// Voice in native language
// #define TR_VOICE_ENGLISH                "English"
// #define TR_VOICE_CHINESE                "Chinese"
// #define TR_VOICE_CZECH                  "Czech"
// #define TR_VOICE_DANISH                 "Danish"
// #define TR_VOICE_DEUTSCH                "German"
// #define TR_VOICE_DUTCH                  "Dutch"
// #define TR_VOICE_ESPANOL                "Spanish"
// #define TR_VOICE_FRANCAIS               "French"
// #define TR_VOICE_HUNGARIAN              "Hungarian"
// #define TR_VOICE_ITALIANO               "Italian"
// #define TR_VOICE_POLISH                 "Polish"
// #define TR_VOICE_PORTUGUES              "Portuguese"
// #define TR_VOICE_RUSSIAN                "Russian"
// #define TR_VOICE_SLOVAK                 "Slovak"
// #define TR_VOICE_SWEDISH                "Swedish"
// #define TR_VOICE_TAIWANESE              "Taiwanese"
// #define TR_VOICE_JAPANESE               "Japanese"
// #define TR_VOICE_HEBREW                 "Hebrew"
// #define TR_VOICE_UKRAINIAN               "Ukrainian"
#define TR_VOICE_ENGLISH                "영어"
#define TR_VOICE_CHINESE                "중국어"
#define TR_VOICE_CZECH                  "체코어"
#define TR_VOICE_DANISH                 "덴마크어"
#define TR_VOICE_DEUTSCH                "독일어"
#define TR_VOICE_DUTCH                  "네덜란드어"
#define TR_VOICE_ESPANOL                "스페인어"
#define TR_VOICE_FRANCAIS               "프랑스어"
#define TR_VOICE_HUNGARIAN              "헝가리어"
#define TR_VOICE_ITALIANO               "이탈리아어"
#define TR_VOICE_POLISH                 "폴란드어"
#define TR_VOICE_PORTUGUES              "포르투갈어"
#define TR_VOICE_RUSSIAN                "러시아어"
#define TR_VOICE_SLOVAK                 "슬로바키아어"
#define TR_VOICE_SWEDISH                "스웨덴어"
#define TR_VOICE_TAIWANESE              "대만어"
#define TR_VOICE_JAPANESE               "일본어"
#define TR_VOICE_HEBREW                 "히브리어"
#define TR_VOICE_UKRAINIAN              "우크라이나어"
#define TR_VOICE_KOREAN                 "한국어"


// #define TR_USBJOYSTICK_LABEL           "USB Joystick"
// #define TR_USBJOYSTICK_EXTMODE         "Mode"
// #define TR_VUSBJOYSTICK_EXTMODE        "Classic","Advanced"
// #define TR_USBJOYSTICK_SETTINGS        "Channel Settings"
// #define TR_USBJOYSTICK_IF_MODE         TR("If. mode","Interface mode")
// #define TR_VUSBJOYSTICK_IF_MODE        "Joystick","Gamepad","MultiAxis"
// #define TR_USBJOYSTICK_CH_MODE         "Mode"
// #define TR_VUSBJOYSTICK_CH_MODE        "None","Btn","Axis","Sim"
// #define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
// #define TR_USBJOYSTICK_CH_BTNMODE      "Button Mode"
// #define TR_VUSBJOYSTICK_CH_BTNMODE     "Normal","Pulse","SWEmu","Delta","Companion"
// #define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Norm","Normal"),TR("Puls","Pulse"),TR("SWEm","SWEmul"),TR("Delt","Delta"),TR("CPN","Companion")
// #define TR_USBJOYSTICK_CH_SWPOS        "Positions"
// #define TR_VUSBJOYSTICK_CH_SWPOS       "Push","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
// #define TR_USBJOYSTICK_CH_AXIS         "Axis"
// #define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","rotX","rotY","rotZ","Slider","Dial","Wheel"
// #define TR_USBJOYSTICK_CH_SIM          "Sim axis"
// #define TR_VUSBJOYSTICK_CH_SIM         "Ail","Ele","Rud","Thr","Acc","Brk","Steer","Dpad"
// #define TR_USBJOYSTICK_CH_INVERSION    "Inversion"
// #define TR_USBJOYSTICK_CH_BTNNUM       "Button no."
// #define TR_USBJOYSTICK_BTN_COLLISION   "!Button no. collision!"
// #define TR_USBJOYSTICK_AXIS_COLLISION  "!Axis collision!"
// #define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
// #define TR_VUSBJOYSTICK_CIRC_COUTOUT   "None","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
// #define TR_USBJOYSTICK_APPLY_CHANGES   "Apply changes"
#define TR_USBJOYSTICK_LABEL                "USB 조이스틱"
#define TR_USBJOYSTICK_EXTMODE              "모드"
#define TR_VUSBJOYSTICK_EXTMODE             "클래식", "고급"
#define TR_USBJOYSTICK_SETTINGS             "채널 설정"
#define TR_USBJOYSTICK_IF_MODE              TR("인터페이스 모드", "입력 방식")
#define TR_VUSBJOYSTICK_IF_MODE             "조이스틱", "게임패드", "멀티축"
#define TR_USBJOYSTICK_CH_MODE              "채널 모드"
#define TR_VUSBJOYSTICK_CH_MODE             "없음", "버튼", "축", "시뮬"
#define TR_VUSBJOYSTICK_CH_MODE_S           "-", "B", "A", "S"
#define TR_USBJOYSTICK_CH_BTNMODE           "버튼 동작"
#define TR_VUSBJOYSTICK_CH_BTNMODE          "일반", "펄스", "스위치에뮬", "델타", "컴패니언"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S        TR("일반", "Normal"), TR("펄스", "Pulse"), TR("SW에뮬", "SWEmul"), TR("델타", "Delta"), TR("CPN", "Companion")
#define TR_USBJOYSTICK_CH_SWPOS             "스위치 위치"
#define TR_VUSBJOYSTICK_CH_SWPOS            "푸시", "2단", "3단", "4단", "5단", "6단", "7단", "8단"
#define TR_USBJOYSTICK_CH_AXIS              "축"
#define TR_VUSBJOYSTICK_CH_AXIS             "X","Y","Z","회전X","회전Y","회전Z","슬라이더","다이얼","휠"
#define TR_USBJOYSTICK_CH_SIM               "시뮬레이션 축"
#define TR_VUSBJOYSTICK_CH_SIM              "에일러론", "엘리베이터", "러더", "스로틀", "가속", "브레이크", "조향", "D패드"
#define TR_USBJOYSTICK_CH_INVERSION         "반전"
#define TR_USBJOYSTICK_CH_BTNNUM            "버튼 번호"
#define TR_USBJOYSTICK_BTN_COLLISION        "!버튼 번호 중복!"
#define TR_USBJOYSTICK_AXIS_COLLISION       "!축 중복!"
#define TR_USBJOYSTICK_CIRC_COUTOUT         TR("원형 컷", "원형 제한")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT        "없음", "X-Y, Z-rX", "X-Y, rX-rY", "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES        "변경사항 적용"

// #define TR_DIGITAL_SERVO          "Servo333HZ"
// #define TR_ANALOG_SERVO           "Servo 50HZ"
// #define TR_SIGNAL_OUTPUT          "Signal output"
// #define TR_SERIAL_BUS             "Serial bus"
// #define TR_SYNC                   "Sync"
#define TR_DIGITAL_SERVO              "디지털 서보 (333Hz)"
#define TR_ANALOG_SERVO               "아날로그 서보 (50Hz)"
#define TR_SIGNAL_OUTPUT              "신호 출력"
#define TR_SERIAL_BUS                 "시리얼 버스"
#define TR_SYNC                       "동기화"

// #define TR_ENABLED_FEATURES       "Enabled Features"
// #define TR_RADIO_MENU_TABS        "Radio Menu Tabs"
// #define TR_MODEL_MENU_TABS        "Model Menu Tabs"
#define TR_ENABLED_FEATURES         "활성화된 기능"
#define TR_RADIO_MENU_TABS          "조종기 메뉴 탭"
#define TR_MODEL_MENU_TABS          "모델 메뉴 탭"

// #define TR_SELECT_MENU_ALL        "All"
// #define TR_SELECT_MENU_CLR        "Clear"
// #define TR_SELECT_MENU_INV        "Invert"
#define TR_SELECT_MENU_ALL          "전체 선택"
#define TR_SELECT_MENU_CLR          "선택 해제"
#define TR_SELECT_MENU_INV          "선택 반전"

// #define TR_SORT_ORDERS            "Name A-Z","Name Z-A","Least used","Most used"
// #define TR_SORT_MODELS_BY         "Sort models by"
// #define TR_CREATE_NEW             "Create"
#define TR_SORT_ORDERS              "이름순 A-Z", "이름순 Z-A", "최소 사용순", "최다 사용순"
#define TR_SORT_MODELS_BY           "모델 정렬 기준"
#define TR_CREATE_NEW               "새로 만들기"

// #define TR_MIX_SLOW_PREC          TR("Slow prec", "Slow up/dn prec")
// #define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")
#define TR_MIX_SLOW_PREC            TR("슬로우 정밀도", "상/하 슬로우 전환 정밀도")
#define TR_MIX_DELAY_PREC           TR("지연 정밀도", "상/하 지연 정밀도")

// #define TR_THEME_EXISTS           "A theme directory with the same name already exists."
#define TR_THEME_EXISTS             "같은 이름의 테마 폴더가 이미 존재합니다."

// #define TR_DATE_TIME_WIDGET       "Date & Time"
// #define TR_RADIO_INFO_WIDGET      "Radio Info"
// #define TR_LOW_BATT_COLOR         "Low battery"
// #define TR_MID_BATT_COLOR         "Mid battery"
// #define TR_HIGH_BATT_COLOR        "High battery"
#define TR_DATE_TIME_WIDGET         "날짜 및 시간"
#define TR_RADIO_INFO_WIDGET        "조종기 정보"
#define TR_LOW_BATT_COLOR           "배터리 부족 색상"
#define TR_MID_BATT_COLOR           "배터리 중간 색상"
#define TR_HIGH_BATT_COLOR          "배터리 충전 색상"

// #define TR_WIDGET_SIZE              "Widget size"
#define TR_WIDGET_SIZE              "위젯 크기"

// #define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
#define TR_DEL_DIR_NOT_EMPTY        "폴더를 삭제하려면 먼저 비워야 합니다"
