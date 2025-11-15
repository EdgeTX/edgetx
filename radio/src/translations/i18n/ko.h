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

// Main menu
#define TR_QM_MANAGE_MODELS             "Manage\nModels"
#define TR_QM_MODEL_SETUP               "Model\nSetup"
#define TR_QM_RADIO_SETUP               "Radio\nSetup"
#define TR_QM_UI_SETUP                  "UI\nSetup"
#define TR_QM_TOOLS                     "Tools"
#define TR_QM_GEN_SETTINGS              "General\nSettings"
#define TR_QM_FLIGHT_MODES              TR_SFC_AIR("Drive\nModes", "Flight\nModes")
#define TR_QM_INPUTS                    "Inputs"
#define TR_QM_MIXES                     "Mixes"
#define TR_QM_OUTPUTS                   "Outputs"
#define TR_QM_CURVES                    "Curves"
#define TR_QM_GLOBAL_VARS               "Global\nVariables"
#define TR_QM_LOGICAL_SW                "Logical\nSwitches"
#define TR_QM_SPEC_FUNC                 "Special\nFunctions"
#define TR_QM_CUSTOM_LUA                "Custom\nScripts"
#define TR_QM_TELEM                     "Telemetry"
#define TR_QM_GLOB_FUNC                 "Global\nFunctions"
#define TR_QM_TRAINER                   "Trainer"
#define TR_QM_HARDWARE                  "Hardware"
#define TR_QM_ABOUT                     "About\nEdgeTX"
#define TR_QM_THEMES                    "Themes"
#define TR_QM_TOP_BAR                   "Top Bar"
#define TR_QM_SCREEN_1                  "Screen 1"
#define TR_QM_SCREEN_2                  "Screen 2"
#define TR_QM_SCREEN_3                  "Screen 3"
#define TR_QM_SCREEN_4                  "Screen 4"
#define TR_QM_SCREEN_5                  "Screen 5"
#define TR_QM_SCREEN_6                  "Screen 6"
#define TR_QM_SCREEN_7                  "Screen 7"
#define TR_QM_SCREEN_8                  "Screen 8"
#define TR_QM_SCREEN_9                  "Screen 9"
#define TR_QM_SCREEN_10                 "Screen 10"
#define TR_QM_ADD_SCREEN                "Add\nScreen"
#define TR_QM_APPS                      "Apps"
#define TR_QM_STORAGE                   "Storage"
#define TR_QM_RESET                     TR_SFC_AIR("Drive\nReset", "Flight\nReset")
#define TR_QM_CHAN_MON                  "Channel\nMonitor"
#define TR_QM_LS_MON                    "LS\nMonitor"
#define TR_QM_STATS                     "Statistics"
#define TR_QM_DEBUG                     "Debug"
#define TR_MAIN_MENU_SETTINGS           "General Settings"
#define TR_MAIN_MENU_MANAGE_MODELS        "모델\n관리"
#define TR_MAIN_MENU_MODEL_NOTES          "모델\n노트"
#define TR_MAIN_MENU_CHANNEL_MONITOR      "채널\n모니터"
#define TR_MONITOR_SWITCHES             "로직 스위치 모니터"
#define TR_MAIN_MENU_MODEL_SETTINGS       "모델\n설정"
#define TR_MAIN_MENU_RADIO_SETTINGS       "조종기\n설정"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "UI Setup"
#define TR_MAIN_MENU_STATISTICS           "비행\n기록"
#define TR_MAIN_MENU_ABOUT_EDGETX         "EdgeTX\n정보"
#define TR_MAIN_VIEW_X                  "Screen "
#define TR_MAIN_MENU_THEMES                   "테마"
#define TR_MAIN_MENU_APPS               "Apps"
#define TR_MENUHELISETUP                "헬리 설정"
#define TR_MENUFLIGHTMODES                TR_SFC_AIR("주행 모드", "비행 모드")
#define TR_MENUFLIGHTMODE                 TR_SFC_AIR("주행 모드", "비행 모드")
#define TR_MENUINPUTS                   "입력"
#define TR_MENULIMITS                   "출력"
#define TR_MENUCURVES                   "커브"
#define TR_MIXES                          "믹스"
#define TR_MENU_GLOBAL_VARS               "전역 변수 설정"
#define TR_MENULOGICALSWITCHES          "로직 스위치 목록"
#define TR_MENUCUSTOMFUNC               "특수 기능"
#define TR_MENUCUSTOMSCRIPTS            "사용자 스크립트"
#define TR_MENUTELEMETRY                "테레메트리"
#define TR_MENUSPECIALFUNCS             "전역 기능"
#define TR_MENUTRAINER                  "트레이너"
#define TR_HARDWARE                       "하드웨어"
#define TR_USER_INTERFACE               "Top Bar"
#define TR_SD_CARD                        "SD 카드"
#define TR_DEBUG                          "디버그"
#define TR_MENU_RADIO_SWITCHES          TR("스위치", "스위치 테스트")
#define TR_MENUCALIBRATION              "입력 보정"
#define TR_FUNCTION_SWITCHES              "사용자 설정 스위치"
// End Main menu

#define TR_MINUTE_SINGULAR            "분"
#define TR_MINUTE_PLURAL1             "분"
#define TR_MINUTE_PLURAL2             "분"

#define TR_OFFON_1                    "끔"
#define TR_OFFON_2                    "켬"
#define TR_MMMINV_1                   "---"
#define TR_MMMINV_2                   "반전"
#define TR_VBEEPMODE_1                "무음"
#define TR_VBEEPMODE_2                "알람"
#define TR_VBEEPMODE_3                "키 없음"
#define TR_VBEEPMODE_4                "모두"
#define TR_VBLMODE_1                  "꺼짐"
#define TR_VBLMODE_2                  "키"
#define TR_VBLMODE_3                  TR("제어부", "제어 장치")
#define TR_VBLMODE_4                  "둘 다"
#define TR_VBLMODE_5                  "켬"
#define TR_TRNMODE_1                  "끔"
#define TR_TRNMODE_2                  TR("+=", "합치기")
#define TR_TRNMODE_3                  TR(":=", "대체")
#define TR_TRNCHN_1                   "채널1"
#define TR_TRNCHN_2                   "채널2"
#define TR_TRNCHN_3                   "채널3"
#define TR_TRNCHN_4                   "채널4"

#define TR_AUX_SERIAL_MODES_1         "끔"
#define TR_AUX_SERIAL_MODES_2         "텔레미러"
#define TR_AUX_SERIAL_MODES_3         "텔레입력"
#define TR_AUX_SERIAL_MODES_4         "SBUS 트레이너"
#define TR_AUX_SERIAL_MODES_5         "LUA"
#define TR_AUX_SERIAL_MODES_6         "CLI"
#define TR_AUX_SERIAL_MODES_7         "GPS"
#define TR_AUX_SERIAL_MODES_8         "디버그"
#define TR_AUX_SERIAL_MODES_9         "SpaceMouse"
#define TR_AUX_SERIAL_MODES_10        "외부 모듈"
#define TR_SWTYPES_1                  "없음"
#define TR_SWTYPES_2                  "토글"
#define TR_SWTYPES_3                  "2단"
#define TR_SWTYPES_4                  "3단"
#define TR_SWTYPES_5                  "Global"
#define TR_POTTYPES_1                 "없음"
#define TR_POTTYPES_2                 "다이얼"
#define TR_POTTYPES_3                 TR("클릭 다이얼", "중심 클릭 다이얼")
#define TR_POTTYPES_4                 "슬라이더"
#define TR_POTTYPES_5                 TR("다단", "다단 스위치")
#define TR_POTTYPES_6                 "X축"
#define TR_POTTYPES_7                 "Y축"
#define TR_POTTYPES_8                 "스위치"
#define TR_VPERSISTENT_1              "끔"
#define TR_VPERSISTENT_2              "비행 중"
#define TR_VPERSISTENT_3              "수동 초기화"
#define TR_COUNTRY_CODES_1            TR("미국", "미국")
#define TR_COUNTRY_CODES_2            TR("일본", "일본")
#define TR_COUNTRY_CODES_3            TR("유럽", "유럽")
#define TR_USBMODES_1                 "선택 요청"
#define TR_USBMODES_2                 TR("조이스틱", "조이스틱")
#define TR_USBMODES_3                 TR("SD카드", "저장소")
#define TR_USBMODES_4                 "시리얼"
#define TR_JACK_MODES_1               "선택 요청"
#define TR_JACK_MODES_2               "오디오"
#define TR_JACK_MODES_3               "트레이너"

#define TR_SBUS_INVERSION_VALUES_1    "정방향"
#define TR_SBUS_INVERSION_VALUES_2    "반전 안함"
#define TR_MULTI_CUSTOM               "사용자 정의"
#define TR_VTRIMINC_1                 TR("지수", "지수 증가")
#define TR_VTRIMINC_2                 TR("초정밀", "아주 미세")
#define TR_VTRIMINC_3                 "미세"
#define TR_VTRIMINC_4                 "중간"
#define TR_VTRIMINC_5                 "큼"
#define TR_VDISPLAYTRIMS_1            "아니오"
#define TR_VDISPLAYTRIMS_2            "변경시"
#define TR_VDISPLAYTRIMS_3            "예"
#define TR_VBEEPCOUNTDOWN_1           "무음"
#define TR_VBEEPCOUNTDOWN_2           "삐 소리"
#define TR_VBEEPCOUNTDOWN_3           "음성"
#define TR_VBEEPCOUNTDOWN_4           "진동"
#define TR_VBEEPCOUNTDOWN_5           TR("삐+진동", "삐 소리 + 진동")
#define TR_VBEEPCOUNTDOWN_6           TR("음성+진동", "음성 + 진동")
#define TR_COUNTDOWNVALUES_1          "5초"
#define TR_COUNTDOWNVALUES_2          "10초"
#define TR_COUNTDOWNVALUES_3          "20초"
#define TR_COUNTDOWNVALUES_4          "30초"
#define TR_VVARIOCENTER_1             "톤"
#define TR_VVARIOCENTER_2             "무음"
#define TR_CURVE_TYPES_1              "표준"
#define TR_CURVE_TYPES_2              "사용자 정의"

#define TR_ADCFILTERVALUES_1           "전역"
#define TR_ADCFILTERVALUES_2           "끔"
#define TR_ADCFILTERVALUES_3           "켬"

#define TR_VCURVETYPE_1               "차이"
#define TR_VCURVETYPE_2               "지수"
#define TR_VCURVETYPE_3               "수식"
#define TR_VCURVETYPE_4               "사용자"
#define TR_VMLTPX_1                   "더하기"
#define TR_VMLTPX_2                   "곱하기"
#define TR_VMLTPX_3                   "대체"

#define TR_CSWTIMER                    "타이머"
#define TR_CSWSTICKY                   "고정"
#define TR_CSWSTAY                     TR("변화", "변화시")

#define TR_SF_TRAINER                 "트레이너"
#define TR_SF_INST_TRIM               "즉시 트림"
#define TR_SF_RESET                   "초기화"
#define TR_SF_SET_TIMER               "타이머 설정"
#define TR_SF_VOLUME                  "볼륨"
#define TR_SF_FAILSAFE                "페일세이프 설정"
#define TR_SF_RANGE_CHECK             "거리체크"
#define TR_SF_MOD_BIND                "모듈 바인딩"
#define TR_SF_RGBLEDS                 "RGB LED"

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

#define TR_SF_SAFETY                  TR("Overr.", "출력 오버라이드")

#define TR_SF_SCREENSHOT              "스크린샷 저장"
#define TR_SF_RACING_MODE             "레이싱 모드"
#define TR_SF_DISABLE_TOUCH           "터치 비활성화"
#define TR_SF_DISABLE_AUDIO_AMP       "오디오 앰프 끄기"
#define TR_SF_SET_SCREEN              TR_BW_COL("텔레 화면 설정", "메인 화면 설정")
#define TR_SF_PUSH_CUST_SWITCH        "커스텀 스위치 실행"
#define TR_SF_LCD_TO_VIDEO            "LCD -> 비디오 출력"

#define TR_FSW_RESET_TELEM            TR("텔레", "텔레메트리")
#define TR_FSW_RESET_TRIMS            "트림"
#define TR_FSW_RESET_TIMERS_1         "타이머1"
#define TR_FSW_RESET_TIMERS_2         "타이머2"
#define TR_FSW_RESET_TIMERS_3         "타이머3"

#define TR_VFSWRESET_1                TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                TR("전체", "비행")
#define TR_VFSWRESET_5                TR_FSW_RESET_TELEM
#define TR_VFSWRESET_6                TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS_1               TR("삐1", "삐 소리1")
#define TR_FUNCSOUNDS_2               TR("삐2", "삐 소리2")
#define TR_FUNCSOUNDS_3               TR("삐3", "삐 소리3")
#define TR_FUNCSOUNDS_4               TR("경고1", "경고음1")
#define TR_FUNCSOUNDS_5               TR("경고2", "경고음2")
#define TR_FUNCSOUNDS_6               TR("찍", "찍찍")
#define TR_FUNCSOUNDS_7               TR("따다", "따따다")
#define TR_FUNCSOUNDS_8               "틱"
#define TR_FUNCSOUNDS_9               TR("사이렌", "사이렌")
#define TR_FUNCSOUNDS_10              "벨소리"
#define TR_FUNCSOUNDS_11              TR("공상", "공상과학")
#define TR_FUNCSOUNDS_12              TR("로봇", "로봇음")
#define TR_FUNCSOUNDS_13              TR("짹", "짹짹")
#define TR_FUNCSOUNDS_14              "따다~"
#define TR_FUNCSOUNDS_15              TR("귀뚜", "귀뚜라미")
#define TR_FUNCSOUNDS_16              TR("알람", "알람시계")

#define LENGTH_UNIT_IMP               "피트"
#define SPEED_UNIT_IMP                "마일/시"
#define LENGTH_UNIT_METR              "미터"
#define SPEED_UNIT_METR               "킬로미터"

#define TR_VUNITSSYSTEM_1             "미터법"
#define TR_VUNITSSYSTEM_2             TR("영국식", "영국 단위계")
#define TR_VTELEMUNIT_1                "-"
#define TR_VTELEMUNIT_2                "V"
#define TR_VTELEMUNIT_3                "A"
#define TR_VTELEMUNIT_4                "mA"
#define TR_VTELEMUNIT_5                "kts"
#define TR_VTELEMUNIT_6                "m/s"
#define TR_VTELEMUNIT_7                "f/s"
#define TR_VTELEMUNIT_8                "kmh"
#define TR_VTELEMUNIT_9                "mph"
#define TR_VTELEMUNIT_10               "m"
#define TR_VTELEMUNIT_11               "ft"
#define TR_VTELEMUNIT_12               "°C"
#define TR_VTELEMUNIT_13               "°F"
#define TR_VTELEMUNIT_14               "%"
#define TR_VTELEMUNIT_15               "mAh"
#define TR_VTELEMUNIT_16               "W"
#define TR_VTELEMUNIT_17               "mW"
#define TR_VTELEMUNIT_18               "dB"
#define TR_VTELEMUNIT_19               "rpm"
#define TR_VTELEMUNIT_20               "g"
#define TR_VTELEMUNIT_21               "°"
#define TR_VTELEMUNIT_22               "rad"
#define TR_VTELEMUNIT_23               "ml"
#define TR_VTELEMUNIT_24               "fOz"
#define TR_VTELEMUNIT_25               "mlm"
#define TR_VTELEMUNIT_26               "Hz"
#define TR_VTELEMUNIT_27               "ms"
#define TR_VTELEMUNIT_28               "us"
#define TR_VTELEMUNIT_29               "km"
#define TR_VTELEMUNIT_30               "dBm"

#define TR_VTELEMSCREENTYPE_1         "없음"
#define TR_VTELEMSCREENTYPE_2         "숫자"
#define TR_VTELEMSCREENTYPE_3         "막대"
#define TR_VTELEMSCREENTYPE_4         "스크립트"
#define TR_GPSFORMAT_1                "도/분/초"
#define TR_GPSFORMAT_2                "NMEA"

#define TR_VSWASHTYPE_1               "없음"
#define TR_VSWASHTYPE_2               "120"
#define TR_VSWASHTYPE_3               "120X"
#define TR_VSWASHTYPE_4               "140"
#define TR_VSWASHTYPE_5               "90"
#define TR_STICK_NAMES0               "Rud"
#define TR_STICK_NAMES1               "Ele"
#define TR_STICK_NAMES2               "Thr"
#define TR_STICK_NAMES3               "Ail"
#define TR_SURFACE_NAMES0             "스티어링"
#define TR_SURFACE_NAMES1             "스로틀"

#define TR_ON_ONE_SWITCHES_1          "개별"
#define TR_ON_ONE_SWITCHES_2          "하나로"

#define TR_HATSMODE                    "Hat 스위치 모드"
#define TR_HATSOPT_1                   "트림 전용"
#define TR_HATSOPT_2                   "키 전용"
#define TR_HATSOPT_3                   "전환 가능"
#define TR_HATSOPT_4                   "전역 설정"
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

#define TR_ROTARY_ENC_OPT_1         "정방향"
#define TR_ROTARY_ENC_OPT_2         "역방향"
#define TR_ROTARY_ENC_OPT_3         "수직-정, 수평-정"
#define TR_ROTARY_ENC_OPT_4         "수직-정, 수평-역"
#define TR_ROTARY_ENC_OPT_5         "수직-역역, 수평-정"

#define TR_IMU_VSRCRAW_1            "기울기X"
#define TR_IMU_VSRCRAW_2            "기울기Y"

#define TR_CYC_VSRCRAW_1            "사이클릭1"
#define TR_CYC_VSRCRAW_2            "사이클릭2"
#define TR_CYC_VSRCRAW_3            "사이클릭3"

#define TR_SRC_BATT                   "배터리"
#define TR_SRC_TIME                   "시간"
#define TR_SRC_GPS                    "GPS"
#define TR_SRC_TIMER                  "타이머"


#define TR_VTMRMODES_1                "끔"
#define TR_VTMRMODES_2                "켬"
#define TR_VTMRMODES_3                "시작"
#define TR_VTMRMODES_4                "스로틀 스틱"
#define TR_VTMRMODES_5                "스로틀 비율"
#define TR_VTMRMODES_6                "스로틀 시간"

#define TR_VTRAINER_MASTER_OFF          "끔"
#define TR_VTRAINER_MASTER_JACK         "마스터/잭"
#define TR_VTRAINER_SLAVE_JACK          "슬레이브/잭"
#define TR_VTRAINER_MASTER_SBUS_MODULE  "마스터/SBUS 모듈"
#define TR_VTRAINER_MASTER_CPPM_MODULE  "마스터/CPPM 모듈"
#define TR_VTRAINER_MASTER_BATTERY      "마스터/시리얼"
#define TR_VTRAINER_BLUETOOTH_1         "마스터/" TR("BT","블루투스")
#define TR_VTRAINER_BLUETOOTH_2         "슬레이브/" TR("BT","블루투스")
#define TR_VTRAINER_MULTI               "마스터/멀티"
#define TR_VTRAINER_CRSF                "마스터/CRSF"

#define TR_VFAILSAFE_1                  "설정 안됨"
#define TR_VFAILSAFE_2                  "고정"
#define TR_VFAILSAFE_3                  "사용자 지정"
#define TR_VFAILSAFE_4                  "신호 없음"
#define TR_VFAILSAFE_5                  "수신기 설정"
#define TR_VSENSORTYPES_1               "사용자 정의"
#define TR_VSENSORTYPES_2               "계산됨"
#define TR_VFORMULAS_1                  "더하기"
#define TR_VFORMULAS_2                  "평균"
#define TR_VFORMULAS_3                  "최소값"
#define TR_VFORMULAS_4                  "최대값"
#define TR_VFORMULAS_5                  "곱하기"
#define TR_VFORMULAS_6                  "합계"
#define TR_VFORMULAS_7                  "셀"
#define TR_VFORMULAS_8                  "소모량"
#define TR_VFORMULAS_9                  "거리"
#define TR_VPREC_1                      "0.--"
#define TR_VPREC_2                      "0.0"
#define TR_VPREC_3                      "0.00"
#define TR_VCELLINDEX_1                 "최저"
#define TR_VCELLINDEX_2                 "1"
#define TR_VCELLINDEX_3                 "2"
#define TR_VCELLINDEX_4                 "3"
#define TR_VCELLINDEX_5                 "4"
#define TR_VCELLINDEX_6                 "5"
#define TR_VCELLINDEX_7                 "6"
#define TR_VCELLINDEX_8                 "7"
#define TR_VCELLINDEX_9                 "8"
#define TR_VCELLINDEX_10                "최고"
#define TR_VCELLINDEX_11                "차이"
#define TR_SUBTRIMMODES_1               CHAR_DELTA "(센터만)"
#define TR_SUBTRIMMODES_2               "= (대칭)"
#define TR_TIMER_DIR_1                  TR("남은 시간", "잔여 시간 표시")
#define TR_TIMER_DIR_2                  TR("경과 시간", "경과 시간 표시")

#define TR_FONT_SIZES_1                 "표준"
#define TR_FONT_SIZES_2                 "굵게"
#define TR_FONT_SIZES_3                 "아주 작게"
#define TR_FONT_SIZES_4                 "작게"
#define TR_FONT_SIZES_5                 "크게"
#define TR_FONT_SIZES_6                 "아주 크게"
#define TR_FONT_SIZES_7                 "매우 크게"

#define TR_ENTER                       "[ENTER]"
#define TR_OK                          TR_BW_COL(TR("\010\010\010[확인]", "\010\010\010\010\010[확인]"), "Ok")
#define TR_EXIT                        TR_BW_COL("나가기", "RTN")

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

#define TR_TIMER                      TR("타이머", "타이머 ")
#define TR_NO_TIMERS                   "No timers"
#define TR_START                      "시작"
#define TR_NEXT                        "Next"
#define TR_ELIMITS                    TR("확장 제한", "익스텐디드 리밋")
#define TR_ETRIMS                     TR("확장 트림", "익스텐디드 트림")
#define TR_TRIMINC                    "트림 단계"
#define TR_DISPLAY_TRIMS              TR("트림 표시", "트림 상태 표시")
#define TR_TTRACE                     TR("트림 소스", "소스")
#define TR_TTRIM                      TR("유휴 트림", "유휴 시 트림")
#define TR_TTRIM_SW                   TR("트림 스위치", "트림 스위치")
#define TR_BEEPCTR                    TR("중앙 알림", "중앙 위치에서 비프")
#define TR_USE_GLOBAL_FUNCS           TR("글로벌 기능", "전역 기능 사용")

#define TR_PROTOCOL                   TR("프로토콜", "통신 방식")
#define TR_PPMFRAME                   "PPM 프레임"
#define TR_REFRESHRATE                TR("갱신", "갱신 속도")
#define TR_WARN_BATTVOLTAGE          TR("출력은 VBAT: ", "경고: 출력 전압은 VBAT입니다")
#define TR_WARN_5VOLTS                "경고: 출력 전압이 5볼트입니다"
#define TR_MS                         "ms"

#define TR_SWITCH                         "스위치"
#define TR_FS_COLOR_LIST_1                "사용자 정의"
#define TR_FS_COLOR_LIST_2                "끄기"
#define TR_FS_COLOR_LIST_3                "흰색"
#define TR_FS_COLOR_LIST_4                "빨강"
#define TR_FS_COLOR_LIST_5                "초록"
#define TR_FS_COLOR_LIST_6                "노랑"
#define TR_FS_COLOR_LIST_7                "주황"
#define TR_FS_COLOR_LIST_8                "파랑"
#define TR_FS_COLOR_LIST_9                "분홍"
#define TR_GROUP                          "그룹"
#define TR_GROUP_ALWAYS_ON                "항상 켜짐"
#define TR_LUA_OVERRIDE                "Allow Lua override"
#define TR_GROUPS                         "항상 켜짐 그룹"
#define TR_LAST                           "마지막"
#define TR_MORE_INFO                      "자세히 보기"
#define TR_SWITCH_TYPE                    "유형"
#define TR_SWITCH_STARTUP                 "시작 시 상태"
#define TR_SWITCH_GROUP                   "스위치 그룹"
#define TR_SF_SWITCH                      "트리거"
#define TR_TRIMS                          "트림"
#define TR_FADEIN                         "페이드 인"
#define TR_FADEOUT                        "페이드 아웃"
#define TR_DEFAULT                        "(기본값)"

#define  TR_CHECKTRIMS                    TR_BW_COL("\006트림\012확인", "FM 트림 확인")
#define TR_SWASHTYPE                    "스와시 타입"
#define TR_COLLECTIVE                   TR("콜렉티브", "집단 피치 소스")
#define TR_AILERON                      TR("에일러론", "측면 싸이클릭 소스")
#define TR_ELEVATOR                     TR("엘리베이터", "전후 싸이클릭 소스")
#define TR_SWASHRING                    "스와시 링"
#define TR_MODE                         "모드"
#define TR_LEFT_STICK                   "왼쪽 스틱"
#define TR_SUBTYPE                        "하위 유형"
#define TR_NOFREEEXPO                     "사용 가능한 Expo 없음!"
#define TR_NOFREEMIXER                    "사용 가능한 믹서 없음!"
#define TR_SOURCE                         "소스"
#define TR_WEIGHT                         "가중치"
#define TR_SIDE                           "방향"
#define TR_OFFSET                         "오프셋"
#define TR_TRIM                           "트림"
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
#define TR_CV                             "CV"
#define TR_GV                             TR("G", "GV")
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

#define TR_MODEL_SHUTDOWN                 "모델을 종료할까요?"
#define TR_PRESS_ENTER_TO_CONFIRM         "ENTER를 눌러 확인"

#define TR_THROTTLE_LABEL               "스로틀"
#define TR_THROTTLE_START               "스로틀 시작"
#define TR_THROTTLEREVERSE              TR("스로틀 반전", "역방향")
#define TR_MINUTEBEEP                   TR("분 경고음", "1분마다 알림")
#define TR_BEEPCOUNTDOWN                "카운트다운 알림"
#define TR_PERSISTENT                   TR("유지", "값 유지")

#define TR_BACKLIGHT_LABEL              "백라이트"
#define TR_GHOST_MENU_LABEL             "고스트 메뉴"
#define TR_STATUS                       "상태"
#define TR_BLONBRIGHTNESS               "켜짐 밝기"
#define TR_BLOFFBRIGHTNESS              "꺼짐 밝기"
#define TR_KEYS_BACKLIGHT               "키 백라이트"
#define TR_BLCOLOR                      "색상"
#define TR_SPLASHSCREEN                 "시작화면"
#define TR_PLAY_HELLO                   "시작 사운드"

#define TR_PWR_ON_DELAY                 "전원 켜짐 지연"
#define TR_PWR_OFF_DELAY                "전원 꺼짐 지연"
#define TR_PWR_AUTO_OFF                 TR("자동 종료", "전원 자동 끄기")
#define TR_PWR_ON_OFF_HAPTIC            TR("전원 진동", "전원 ON/OFF 진동")

#define TR_THROTTLE_WARNING             TR("스로틀 경고", "스로틀 상태 확인")
#define TR_CUSTOM_THROTTLE_WARNING      TR("사용자 위치", "사용자 정의 스로틀 위치?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL  TR("위치 %", "스로틀 위치 (%)")
#define TR_SWITCHWARNING                TR("스위치 경고", "스위치 위치 확인")
#define TR_POTWARNINGSTATE              "노브 및 슬라이더"
#define TR_POTWARNING                   TR("노브 경고", "노브 위치")

#define TR_TIMEZONE                     "시간대"
#define TR_ADJUST_RTC                   "RTC 조정"
#define TR_GPS                          "GPS"
#define TR_DEF_CHAN_ORD                 TR("기본 채널 순서", "채널 기본 순서")
#define TR_STICKS                       "스틱"
#define TR_POTS                         "노브"
#define TR_SWITCHES                     "스위치"
#define TR_SWITCHES_DELAY               TR("재생 지연", "스위치 지연 (중간 위치)")

#define TR_SLAVE                        "슬레이브"
#define TR_MODESRC                      "모드별 소스"
#define TR_MULTIPLIER                   "배율"
#define TR_CAL                          "보정"
#define TR_CALIBRATION                  BUTTON("보정")
#define TR_VTRIM                        "트림 - +"
#define TR_CALIB_DONE                   "보정 완료"

#define TR_MENUTOSTART                  TR_ENTER " -> 시작"
#define TR_MENUWHENDONE                 TR_ENTER " -> 완료"
#define TR_AXISDIR                      "AXIS DIR"
#define TR_MENUAXISDIR                  "[ENTER LONG] "  TR_AXISDIR
#define TR_SETMIDPOINT                  TR_BW_COL(TR_SFC_AIR("포트 중앙 정렬", TR("스틱 중앙 정렬", "스틱/슬라이더 중앙 정렬")), "스틱/슬라이더 중앙 정렬")
#define TR_MOVESTICKSPOTS               TR_BW_COL(TR_SFC_AIR("스티어링/스로틀/포트/슬라이더 이동", "스틱/포트 이동"), "스틱/포트 이동")
#define TR_NODATA                       "데이터 없음"
#define TR_US                          "us"
#define TR_HZ                          "Hz"

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
#define TR_MENUTORESET                TR_ENTER " -> 초기화"
#define TR_PPM_TRAINER                "TR"
#define TR_CH                         "채널"
#define TR_MODEL                      "모델"
#define TR_FM                          TR_SFC_AIR("DM", "FM")
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

#define TR_STORAGE_FORMAT               "스토리지 준비 중"
#define TR_EEPROMOVERFLOW               "EEPROM 메모리 초과"
#define TR_RADIO_SETUP                  "조종기 설정"
#define TR_MENUVERSION                  "버전 정보"
#define TR_MENU_RADIO_ANALOGS           TR("아날로그", "아날로그 테스트")
#define TR_MENU_RADIO_ANALOGS_CALIB     "보정된 아날로그"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW 아날로그 (5 Hz)"
#define TR_MENU_FSWITCH                 "사용자 스위치 설정"

#define  TR_TRIMS2OFFSETS               TR_BW_COL("\006트림 -> 서브트림", "트림 -> 서브트림")

#define TR_CHANNELS2FAILSAFE            "채널 전체 -> 페일세이프"
#define TR_CHANNEL2FAILSAFE             "채널 -> 페일세이프"

#define TR_MENUMODELSEL                 TR("모델 선택", "모델 선택 메뉴")
#define TR_MENU_MODEL_SETUP             TR("설정", "모델 설정")

#define TR_MENUCURVE                    "커브 설정"
#define TR_MENULOGICALSWITCH            "로직 스위치"
#define TR_MENUSTAT                     "통계"
#define TR_MENUDEBUG                    "디버그"

#define TR_MONITOR_CHANNELS             "채널 모니터 %d-%d"
#define TR_MONITOR_OUTPUT_DESC          "출력 상태"
#define TR_MONITOR_MIXER_DESC           "믹서 상태"

#define TR_RECEIVER_NUM                 TR("수신기 번호", "Receiver No.")
#define TR_RECEIVER                     "수신기"

#define TR_MULTI_RFTUNE                 TR("주파수 조정", "RF 주파수 미세 조정")
#define TR_MULTI_RFPOWER                "RF 출력"
#define TR_MULTI_WBUS                   "출력"
#define TR_MULTI_TELEMETRY              "텔레메트리"
#define TR_MULTI_VIDFREQ                TR("영상 주파수", "비디오 송출 주파수")

#define TR_RF_POWER                     "RF 출력"
#define TR_MULTI_FIXEDID                TR("고정ID", "고정 ID")
#define TR_MULTI_OPTION                 TR("옵션", "옵션 값")
#define TR_MULTI_AUTOBIND               TR("바인드 채널", "채널에서 자동 바인딩")
#define TR_DISABLE_CH_MAP               TR("채널 매핑 없음", "채널 매핑 비활성화")
#define TR_DISABLE_TELEM                TR("텔레메트리 없음", "텔레메트리 비활성화")
#define TR_MULTI_LOWPOWER               TR("저전력", "저전력 모드")
#define TR_MULTI_LNA_DISABLE            "LNA 비활성화"

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

#define TR_REBIND                       "재바인딩 필요"
#define TR_REG_OK                       "등록 완료"
#define TR_BIND_OK                      "바인딩 성공"
#define TR_BINDING_CH1_8_TELEM_ON       "CH1-8 텔레메트리 켬"
#define TR_BINDING_CH1_8_TELEM_OFF      "CH1-8 텔레메트리 끔"
#define TR_BINDING_CH9_16_TELEM_ON      "CH9-16 텔레메트리 켬"
#define TR_BINDING_CH9_16_TELEM_OFF     "CH9-16 텔레메트리 끔"

#define TR_PROTOCOL_INVALID             TR("프로토콜 오류", "프로토콜이 유효하지 않음")
#define TR_MODULE_STATUS                TR("상태", "모듈 상태")
#define TR_MODULE_SYNC                  TR("동기화", "프로토콜 동기화 상태")
#define TR_MULTI_SERVOFREQ              TR("서보 속도", "서보 업데이트 속도")
#define TR_MULTI_MAX_THROW              TR("최대 스로우", "최대 이동량 허용")
#define TR_MULTI_RFCHAN                 TR("RF 채널", "RF 채널 선택")
#define TR_AFHDS3_RX_FREQ               TR("수신기 주파수", "수신기 RF 주파수")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY  TR("단일/텔레메트리", "Unicast/텔레메트리 모드")
#define TR_AFHDS3_ONE_TO_MANY           "멀티캐스트"

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

#define TR_MODELS_MOVED                   "사용하지 않는 모델이 이동됨 ->"
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

#define TR_SDCARD_ERROR                   TR("SD 오류", "SD카드 오류")
#define TR_SDCARD                         "SD 카드"
#define TR_NO_FILES_ON_SD                 "SD 카드에 파일 없음!"
#define TR_NO_SDCARD                      "SD 카드 없음"
#define TR_WAITING_FOR_RX                 "수신기 대기 중..."
#define TR_WAITING_FOR_TX                 "송신기 대기 중..."
#define TR_WAITING_FOR_MODULE             TR("모듈 대기 중", "모듈 연결 대기 중...")
#define TR_NO_TOOLS                       "사용 가능한 도구 없음"
#define TR_NORMAL                         "정상"

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

#define TR_UPDATE_RX_OPTIONS              "수신기 옵션을 업데이트할까요?"
#define TR_UPDATE_TX_OPTIONS              "송신기 옵션을 업데이트할까요?"
#define TR_MODULES_RX_VERSION             BUTTON("모듈 / 수신기 버전")
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
#define TR_GHOST_MODULE_CONFIG         "Ghost module config"
#define TR_GPS_MODEL_LOCATOR           "GPS model locator"
#define TR_REFRESH                     "Refresh"
#define TR_SDCARD_FULL                    "SD 카드가 가득 찼습니다"

#define TR_SDCARD_FULL_EXT               TR_BW_COL(TR_SDCARD_FULL "\036로그 및 스크린샷" LCDW_128_LINEBREAK "저장 불가", TR_SDCARD_FULL "\n로그 및 스크린샷 저장 불가")

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

#define TR_WRONG_PCBREV                   "잘못된 PCB가 감지됨"
#define TR_EMERGENCY_MODE                 "비상 모드"
#define TR_NO_FAILSAFE                    "페일세이프가 설정되지 않음"
#define TR_KEYSTUCK                       "버튼이 눌려 있음"
#define TR_VOLUME                         "볼륨"
#define TR_LCD                            "LCD"
#define TR_BRIGHTNESS                     "밝기"
#define TR_CONTROL                     "Control"
#define TR_CPU_TEMP                       "CPU 온도"
#define TR_COPROC                         "보조 프로세서"
#define TR_COPROC_TEMP                    "메인보드 온도"
#define TR_TTL_WARNING                    "[경고] TX/RX 핀에 3.3V를 초과하지 마세요!"
#define TR_FUNC                           "기능"
#define TR_V1                             "V1"
#define TR_V2                             "V2"
#define TR_DURATION                       "지속 시간"
#define TR_DELAY                          "지연"

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

#define TR_FORMATTING                     "포맷 중..."
#define TR_TEMP_CALIB                     "온도 보정"
#define TR_TIME                           "시간"
#define TR_MAXBAUDRATE                    "최대 보드레이트"
#define TR_BAUDRATE                       "보드레이트"
#define TR_CRSF_ARMING_MODE               "아밍 방식"


#define TR_CRSF_ARMING_MODES              TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                    TR("샘플링", "샘플 모드")
#define TR_SAMPLE_MODES_1                 "일반"
#define TR_SAMPLE_MODES_2                 "1비트"
#define TR_LOADING                        "불러오는 중..."
#define TR_DELETE_THEME                   "테마를 삭제할까요?"
#define TR_SAVE_THEME                     "테마를 저장할까요?"
#define TR_EDIT_COLOR                     "색상 편집"
#define TR_NO_THEME_IMAGE                 "테마 이미지 없음"
#define TR_BACKLIGHT_TIMER                "무작동 타이머"

#define TR_MODEL_QUICK_SELECT             "빠른 모델 선택"
#define TR_LABELS_SELECT                  "라벨 선택"
#define TR_LABELS_MATCH                   "라벨 매칭"
#define TR_FAV_MATCH                      "즐겨찾기 조건"
#define TR_LABELS_SELECT_MODE_1           "다중 선택"
#define TR_LABELS_SELECT_MODE_2           "단일 선택"
#define TR_LABELS_MATCH_MODE_1            "모두 일치"
#define TR_LABELS_MATCH_MODE_2            "하나 이상 일치"
#define TR_FAV_MATCH_MODE_1               "일치해야 함"
#define TR_FAV_MATCH_MODE_2               "일치해도 됨"

#define TR_SELECT_TEMPLATE_FOLDER         "템플릿 폴더 선택"
#define TR_SELECT_TEMPLATE                "모델 템플릿 선택:"
#define TR_NO_TEMPLATES                   "이 폴더에 템플릿이 없습니다"
#define TR_SAVE_TEMPLATE                  "템플릿으로 저장"
#define TR_BLANK_MODEL                    "빈 모델"
#define TR_BLANK_MODEL_INFO               "빈 모델을 새로 생성합니다"
#define TR_FILE_EXISTS                    "파일이 이미 존재합니다"
#define TR_ASK_OVERWRITE                  "덮어쓰시겠습니까?"

#define TR_BLUETOOTH                      "블루투스"
#define TR_BLUETOOTH_DISC                 "기기 검색"
#define TR_BLUETOOTH_INIT                 "초기화"
#define TR_BLUETOOTH_DIST_ADDR            "원격 주소"
#define TR_BLUETOOTH_LOCAL_ADDR           "로컬 주소"
#define TR_BLUETOOTH_PIN_CODE             "PIN 코드"
#define TR_BLUETOOTH_NODEVICES            "기기를 찾을 수 없습니다"
#define TR_BLUETOOTH_SCANNING             "검색 중..."
#define TR_BLUETOOTH_BAUDRATE             "블루투스 보드레이트"

#define TR_BLUETOOTH_MODES_1              "---"
#define TR_BLUETOOTH_MODES_2              "텔레메트리"
#define TR_BLUETOOTH_MODES_3              "트레이너"
#define TR_BLUETOOTH_MODES_4               "사용함"

#define TR_SD_INFO_TITLE                  "SD 카드 정보"
#define TR_SD_SPEED                       "속도:"
#define TR_SD_SECTORS                     "섹터 수:"
#define TR_SD_SIZE                        "용량:"
#define TR_TYPE                           "유형"
#define TR_GVARS                          "전역값"
#define TR_GLOBAL_VAR                     "전역 변수"
#define TR_OWN                            "사용자 설정"
#define TR_DATE                           "날짜"
#define TR_MONTHS_1                       "1월"
#define TR_MONTHS_2                       "2월"
#define TR_MONTHS_3                       "3월"
#define TR_MONTHS_4                       "4월"
#define TR_MONTHS_5                       "5월"
#define TR_MONTHS_6                       "6월"
#define TR_MONTHS_7                       "7월"
#define TR_MONTHS_8                       "8월"
#define TR_MONTHS_9                       "9월"
#define TR_MONTHS_10                      "10월"
#define TR_MONTHS_11                      "11월"
#define TR_MONTHS_12                      "12월"
#define TR_ROTARY_ENCODER                 "로터리 인코더"

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
#define TR_BYTES                          "바이트"
#define TR_MODULE_BIND                    BUTTON(TR("바인드", "바인딩"))
#define TR_MODULE_UNBIND                  BUTTON("바인딩 해제")
#define TR_POWERMETER_ATTN_NEEDED         "감쇠기 필요"
#define TR_PXX2_SELECT_RX                 "수신기 선택"
#define TR_PXX2_DEFAULT                   "<기본값>"
#define TR_BT_SELECT_DEVICE               "기기 선택"
#define TR_DISCOVER                       BUTTON("검색")

#define TR_BUTTON_INIT                    BUTTON("초기화")
#define TR_WAITING                        "대기 중..."
#define TR_RECEIVER_DELETE                "수신기를 삭제할까요?"
#define TR_RECEIVER_RESET                 "수신기를 초기화할까요?"
#define TR_SHARE                          "공유"
#define TR_BIND                           "바인딩"
#define TR_REGISTER                       BUTTON(TR("등록", "등록"))
#define TR_MODULE_RANGE                   BUTTON(TR("범위", "범위 테스트"))
#define TR_RANGE_TEST                     "범위 테스트"
#define TR_RECEIVER_OPTIONS               TR("수신기 옵션", "RECEIVER OPTIONS")
#define TR_RESET_BTN                      BUTTON("초기화")
#define TR_KEYS_BTN                       BUTTON("버튼")
#define TR_ANALOGS_BTN                    BUTTON(TR("아날로그", "아날로그 입력"))
#define TR_FS_BTN                         BUTTON(TR("사용자 스위치", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND                 "터치 하드웨어를 찾을 수 없습니다."
#define TR_TOUCH_EXIT                     "화면을 터치하여 종료하세요."
#define TR_SET                            BUTTON("설정")
#define TR_TRAINER                        "트레이너"
#define TR_CHANS                          "채널"

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

#define TR_ANTENNA_MODES_1              "내장"
#define TR_ANTENNA_MODES_2              "확인 요청"
#define TR_ANTENNA_MODES_3              "모델별 설정"
#define TR_ANTENNA_MODES_4              "내장 + 외장"
#define TR_ANTENNA_MODES_5              "외장"

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
#define TR_CLEAR_BTN                      BUTTON("지우기")
#define TR_RESET                          "초기화"
#define TR_RESET_SUBMENU                  "초기화..."
#define TR_COUNT                          "카운트"

#define TR_PT                             "pt"
#define TR_PTS                            "pts"
#define TR_SMOOTH                         "부드럽게"
#define TR_COPY_STICKS_TO_OFS             TR("스틱 -> 서브트림", "스틱 위치를 서브트림으로 복사")
#define TR_COPY_MIN_MAX_TO_OUTPUTS        TR("최소/최대 -> 출력", "최소/최대/중앙값을 전체 출력에 복사")
#define TR_COPY_TRIMS_TO_OFS              TR("트림 -> 서브트림", "트림 값을 서브트림으로 복사")
#define TR_INCDEC                         "증가/감소"
#define TR_GLOBALVAR                      "전역 변수"
#define TR_MIXSOURCE                      "소스 (%)"
#define TR_MIXSOURCERAW                   "소스 (값)"
#define TR_CONSTANT                       "상수"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1    "끔"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2    "켬"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3    "자동"
#define TR_PREFLIGHT                      "비행 전 점검"
#define TR_CHECKLIST                      TR("체크리스트", "체크리스트 보기")
#define TR_CHECKLIST_INTERACTIVE          TR3("체크리스트 상호작용", "상호작용 체크리스트", "인터랙티브 체크리스트")
#define TR_AUX_SERIAL_MODE                "보조 시리얼 포트"
#define TR_AUX2_SERIAL_MODE               "보조 시리얼 포트 2"
#define TR_AUX_SERIAL_PORT_POWER          "포트 전원"
#define TR_SCRIPT                         "스크립트"

#define TR_INPUTS                         "입력"
#define TR_OUTPUTS                        "출력"
#define TR_CONFIRMRESET                   TR("전체 삭제", "모델과 설정을 모두 삭제할까요?")
#define TR_TOO_MANY_LUA_SCRIPTS           "Lua 스크립트가 너무 많습니다!"
#define TR_SPORT_UPDATE_POWER_MODE        "S.Port 전원"
#define TR_SPORT_UPDATE_POWER_MODES_1     "자동"
#define TR_SPORT_UPDATE_POWER_MODES_2     "항상 켜짐"
#define TR_NO_TELEMETRY_SCREENS           "텔레메트리 화면 없음"
#define TR_TOUCH_PANEL                    "터치 패널:"
#define TR_FILE_SIZE                      "파일 크기"
#define TR_FILE_OPEN                      "강제로 열까요?"

 // Horus and Taranis column headers
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

#define TR_LIMITS_HEADERS_NAME            "이름"
#define TR_LIMITS_HEADERS_SUBTRIM         "서브트림"
#define TR_LIMITS_HEADERS_MIN             "최소값"
#define TR_LIMITS_HEADERS_MAX             "최대값"
#define TR_LIMITS_HEADERS_DIRECTION       "방향"
#define TR_LIMITS_HEADERS_CURVE           "곡선"
#define TR_LIMITS_HEADERS_PPMCENTER       "PPM 중심값"
#define TR_LIMITS_HEADERS_SUBTRIMMODE     "서브트림 모드"
#define TR_INVERTED                       "반전됨"

  // Horus layouts and widgets
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
#define TR_ALIGN_OPTS_1                 "왼쪽"
#define TR_ALIGN_OPTS_2                 "가운데"
#define TR_ALIGN_OPTS_3                 "오른쪽"
#define TR_TEXT                         "텍스트"
#define TR_COLOR                        "색상"
#define TR_PANEL1_BACKGROUND            "패널1 배경"
#define TR_PANEL2_BACKGROUND            "패널2 배경"
#define TR_PANEL_BACKGROUND             "Background"
#define TR_PANEL_COLOR                  "  Color"
#define TR_WIDGET_GAUGE                 "게이지"
#define TR_WIDGET_MODELBMP              "모델 정보"
#define TR_WIDGET_OUTPUTS               "출력"
#define TR_WIDGET_TEXT                  "텍스트"
#define TR_WIDGET_TIMER                 "타이머"
#define TR_WIDGET_VALUE                 "값"

// About screen
#define TR_ABOUTUS                    TR(" 정보 ", "정보")
#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work
#define TR_BEEP_VOLUME                "비프음 볼륨"
#define TR_WAV_VOLUME                 "음성 파일 볼륨"
#define TR_BG_VOLUME                  TR("배경음 볼륨", "배경 사운드 볼륨")
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
#define TR_FIRMWARE_OPTIONS               BUTTON("펌웨어 옵션")
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
#define TR_THEME                          "테마"
#define TR_SETUP                          "설정"
#define TR_LAYOUT                         "레이아웃"
#define TR_TEXT_COLOR                     "텍스트 색상"
#define TR_MENU_INPUTS                    CHAR_INPUT "입력"
#define TR_MENU_LUA                       CHAR_LUA "Lua 스크립트"
#define TR_MENU_STICKS                    CHAR_STICK "스틱"
#define TR_MENU_POTS                      CHAR_POT "포트"
#define TR_MENU_MIN                       CHAR_FUNCTION "최소값"
#define TR_MENU_MAX                       CHAR_FUNCTION "최대값"
#define TR_MENU_HELI                      CHAR_CYC "사이클릭"
#define TR_MENU_TRIMS                     CHAR_TRIM "트림"
#define TR_MENU_SWITCHES                  CHAR_SWITCH "스위치"
#define TR_MENU_LOGICAL_SWITCHES          CHAR_SWITCH "로직 스위치"
#define TR_MENU_TRAINER                   CHAR_TRAINER "트레이너"
#define TR_MENU_CHANNELS                  CHAR_CHANNEL "채널"
#define TR_MENU_GVARS                     CHAR_SLIDER "GVAR"
#define TR_MENU_TELEMETRY                 CHAR_TELEMETRY "텔레메트리"
#define TR_MENU_DISPLAY                   "디스플레이"
#define TR_MENU_OTHER                     "기타"
#define TR_MENU_INVERT                    "반전"
#define TR_AUDIO_MUTE                     TR("오디오 음소거", "소리가 없을 때 음소거")
#define TR_JITTER_FILTER                  "ADC 필터"
#define TR_DEAD_ZONE                      "데드존"
#define TR_RTC_CHECK                      TR("RTC 확인", "RTC 전압 확인")
#define TR_AUTH_FAILURE                   "인증 실패"
#define TR_RACING_MODE                    "레이싱 모드"
#define TR_USE_THEME_COLOR                "테마 색상 사용"
#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS      "모든 트림 -> 서브트림 복사"
#define TR_DUPLICATE                      "복제"
#define TR_ACTIVATE                       "활성화"
#define TR_RED                            "빨강"
#define TR_BLUE                           "파랑"
#define TR_GREEN                          "초록"
#define TR_COLOR_PICKER                   "색상 선택"
#define TR_FIXED                          "Fixed"
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

// Voice in native language
#define TR_VOICE_ENGLISH                "영어"
#define TR_VOICE_CHINESE                "중국어"
#define TR_VOICE_CZECH                  "체코어"
#define TR_VOICE_DANISH                 "덴마크어"
#define TR_VOICE_DEUTSCH                "독일어"
#define TR_VOICE_DUTCH                  "네덜란드어"
#define TR_VOICE_ESPANOL                "스페인어"
#define TR_VOICE_FINNISH                "Finnish"
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
#define TR_USBJOYSTICK_LABEL                "USB 조이스틱"
#define TR_USBJOYSTICK_EXTMODE              "모드"
#define TR_VUSBJOYSTICK_EXTMODE_1           "클래식"
#define TR_VUSBJOYSTICK_EXTMODE_2           "고급"
#define TR_USBJOYSTICK_SETTINGS             BUTTON("채널 설정")
#define TR_USBJOYSTICK_IF_MODE              TR("인터페이스 모드", "입력 방식")
#define TR_VUSBJOYSTICK_IF_MODE_1           "조이스틱"
#define TR_VUSBJOYSTICK_IF_MODE_2           "게임패드"
#define TR_VUSBJOYSTICK_IF_MODE_3           "멀티축"
#define TR_USBJOYSTICK_CH_MODE              "채널 모드"
#define TR_VUSBJOYSTICK_CH_MODE_1           "없음"
#define TR_VUSBJOYSTICK_CH_MODE_2           "버튼"
#define TR_VUSBJOYSTICK_CH_MODE_3           "축"
#define TR_VUSBJOYSTICK_CH_MODE_4           "시뮬"
#define TR_VUSBJOYSTICK_CH_MODE_S_1         "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2         "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3         "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4         "S"
#define TR_USBJOYSTICK_CH_BTNMODE           "버튼 동작"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1        "일반"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2        "펄스"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3        "스위치에뮬"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4        "델타"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5        "컴패니언"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1      TR("일반", "Normal")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2      TR("펄스", "Pulse")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3      TR("SW에뮬", "SWEmul")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4      TR("델타", "Delta")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5      TR("CPN", "Companion")
#define TR_USBJOYSTICK_CH_SWPOS             "스위치 위치"
#define TR_VUSBJOYSTICK_CH_SWPOS_1          "푸시"
#define TR_VUSBJOYSTICK_CH_SWPOS_2          "2단"
#define TR_VUSBJOYSTICK_CH_SWPOS_3          "3단"
#define TR_VUSBJOYSTICK_CH_SWPOS_4          "4단"
#define TR_VUSBJOYSTICK_CH_SWPOS_5          "5단"
#define TR_VUSBJOYSTICK_CH_SWPOS_6          "6단"
#define TR_VUSBJOYSTICK_CH_SWPOS_7          "7단"
#define TR_VUSBJOYSTICK_CH_SWPOS_8          "8단"
#define TR_USBJOYSTICK_CH_AXIS              "축"
#define TR_VUSBJOYSTICK_CH_AXIS_1           "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2           "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3           "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4           "회전X"
#define TR_VUSBJOYSTICK_CH_AXIS_5           "회전Y"
#define TR_VUSBJOYSTICK_CH_AXIS_6           "회전Z"
#define TR_VUSBJOYSTICK_CH_AXIS_7           "슬라이더"
#define TR_VUSBJOYSTICK_CH_AXIS_8           "다이얼"
#define TR_VUSBJOYSTICK_CH_AXIS_9           "휠"
#define TR_USBJOYSTICK_CH_SIM               "시뮬레이션 축"
#define TR_VUSBJOYSTICK_CH_SIM_1            "에일러론"
#define TR_VUSBJOYSTICK_CH_SIM_2            "엘리베이터"
#define TR_VUSBJOYSTICK_CH_SIM_3            "러더"
#define TR_VUSBJOYSTICK_CH_SIM_4            "스로틀"
#define TR_VUSBJOYSTICK_CH_SIM_5            "가속"
#define TR_VUSBJOYSTICK_CH_SIM_6            "브레이크"
#define TR_VUSBJOYSTICK_CH_SIM_7            "조향"
#define TR_VUSBJOYSTICK_CH_SIM_8            "D패드"
#define TR_USBJOYSTICK_CH_INVERSION         "반전"
#define TR_USBJOYSTICK_CH_BTNNUM            "버튼 번호"
#define TR_USBJOYSTICK_BTN_COLLISION        "!버튼 번호 중복!"
#define TR_USBJOYSTICK_AXIS_COLLISION       "!축 중복!"
#define TR_USBJOYSTICK_CIRC_COUTOUT         TR("원형 컷", "원형 제한")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1      "없음"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2      "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3      "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4      "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES        BUTTON("변경사항 적용")
#define TR_DIGITAL_SERVO              "디지털 서보 (333Hz)"
#define TR_ANALOG_SERVO               "아날로그 서보 (50Hz)"
#define TR_SIGNAL_OUTPUT              "신호 출력"
#define TR_SERIAL_BUS                 "시리얼 버스"
#define TR_SYNC                       "동기화"
#define TR_ENABLED_FEATURES         "활성화된 기능"
#define TR_RADIO_MENU_TABS          "조종기 메뉴 탭"
#define TR_MODEL_MENU_TABS          "모델 메뉴 탭"
#define TR_SELECT_MENU_ALL          "전체 선택"
#define TR_SELECT_MENU_CLR          "선택 해제"
#define TR_SELECT_MENU_INV          "선택 반전"
#define TR_SORT_ORDERS_1            "이름순 A-Z"
#define TR_SORT_ORDERS_2            "이름순 Z-A"
#define TR_SORT_ORDERS_3            "최소 사용순"
#define TR_SORT_ORDERS_4            "최다 사용순"
#define TR_SORT_MODELS_BY           "모델 정렬 기준"
#define TR_CREATE_NEW               "새로 만들기"
#define TR_MIX_SLOW_PREC            TR("슬로우 정밀도", "상/하 슬로우 전환 정밀도")
#define TR_MIX_DELAY_PREC           TR("지연 정밀도", "상/하 지연 정밀도")
#define TR_THEME_EXISTS             "같은 이름의 테마 폴더가 이미 존재합니다."
#define TR_DATE_TIME_WIDGET         "날짜 및 시간"
#define TR_RADIO_INFO_WIDGET        "조종기 정보"
#define TR_LOW_BATT_COLOR           "배터리 부족 색상"
#define TR_MID_BATT_COLOR           "배터리 중간 색상"
#define TR_HIGH_BATT_COLOR          "배터리 충전 색상"
#define TR_WIDGET_SIZE              "위젯 크기"
#define TR_DEL_DIR_NOT_EMPTY        "폴더를 삭제하려면 먼저 비워야 합니다"
