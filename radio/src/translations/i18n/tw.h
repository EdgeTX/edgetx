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
#define TR_MAIN_MENU_MANAGE_MODELS    "模型管理"
#define TR_MAIN_MENU_MODEL_NOTES      "模型說明"
#define TR_MAIN_MENU_CHANNEL_MONITOR  "通道查看"
#define TR_MONITOR_SWITCHES            "邏輯開關查看"
#define TR_MAIN_MENU_MODEL_SETTINGS   "模型設置"
#define TR_MAIN_MENU_RADIO_SETTINGS   "系統設置"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "UI Setup"
#define TR_MAIN_MENU_STATISTICS       "統計信息"
#define TR_MAIN_MENU_ABOUT_EDGETX     "關於"
#define TR_MAIN_VIEW_X                  "Screen "
#define TR_MAIN_MENU_THEMES               "主題"
#define TR_MAIN_MENU_APPS               "Apps"
#define TR_MENUHELISETUP               "直升機設置"
#define TR_MENUFLIGHTMODES             TR_SFC_AIR("駕駛模式", "飛行模式設置")
#define TR_MENUFLIGHTMODE              TR_SFC_AIR("駕駛模式", "飛行模式")
#define TR_MENUINPUTS                  "輸入設置"
#define TR_MENULIMITS                  "輸出設置"
#define TR_MENUCURVES                  "曲線設置"
#define TR_MIXES                       "混控設置"
#define TR_MENU_GLOBAL_VARS            "全局變量功能"
#define TR_MENULOGICALSWITCHES         "邏輯開關設置"
#define TR_MENUCUSTOMFUNC              "特殊功能設置"
#define TR_MENUCUSTOMSCRIPTS           "自定義腳本設置"
#define TR_MENUTELEMETRY               "回傳"
#define TR_MENUSPECIALFUNCS            "全局功能"
#define TR_MENUTRAINER                 "教練"
#define TR_HARDWARE                    "硬件"
#define TR_USER_INTERFACE               "Top Bar"
#define TR_SD_CARD                     "SD卡"
#define TR_MENU_RADIO_SWITCHES         TR("SWITCHES", "開關及按鍵測試")
#define TR_MENUCALIBRATION             "校準"
#define TR_FUNCTION_SWITCHES           "可自定義開關"
// End Main menu

#define TR_MINUTE_SINGULAR              "分鐘"
#define TR_MINUTE_PLURAL1               "分鐘"
#define TR_MINUTE_PLURAL2               "分鐘"

#define TR_OFFON_1                      "禁用"
#define TR_OFFON_2                      "開啟"
#define TR_MMMINV_1                     "---"
#define TR_MMMINV_2                     "反"
#define TR_VBEEPMODE_1                  "靜音"
#define TR_VBEEPMODE_2                  "警告"
#define TR_VBEEPMODE_3                  "忽略按鍵"
#define TR_VBEEPMODE_4                  "全部"
#define TR_VBLMODE_1                    "關閉"
#define TR_VBLMODE_2                    "按鍵"
#define TR_VBLMODE_3                    "操控"
#define TR_VBLMODE_4                    "全部"
#define TR_VBLMODE_5                    "開啟"
#define TR_TRNMODE_1                    "關"
#define TR_TRNMODE_2                    "相加"
#define TR_TRNMODE_3                    "替換"
#define TR_TRNCHN_1                     "CH1"
#define TR_TRNCHN_2                     "CH2"
#define TR_TRNCHN_3                     "CH3"
#define TR_TRNCHN_4                     "CH4"

#define TR_AUX_SERIAL_MODES_1           "禁用"
#define TR_AUX_SERIAL_MODES_2           "回傳鏡像"
#define TR_AUX_SERIAL_MODES_3           "回傳輸入"
#define TR_AUX_SERIAL_MODES_4           "SBUS教練"
#define TR_AUX_SERIAL_MODES_5           "LUA腳本"
#define TR_AUX_SERIAL_MODES_6           "CLI"
#define TR_AUX_SERIAL_MODES_7           "GPS"
#define TR_AUX_SERIAL_MODES_8           "調試"
#define TR_AUX_SERIAL_MODES_9           "SpaceMouse"
#define TR_AUX_SERIAL_MODES_10          "外置發射"
#define TR_SWTYPES_1                    "無"
#define TR_SWTYPES_2                    "回彈"
#define TR_SWTYPES_3                    "2段"
#define TR_SWTYPES_4                    "3段"
#define TR_SWTYPES_5                    "全局"
#define TR_POTTYPES_1                   "無"
#define TR_POTTYPES_2                   "無中點旋鈕"
#define TR_POTTYPES_3                   TR("中點旋鈕","有中點旋鈕")
#define TR_POTTYPES_4                   "側滑塊"
#define TR_POTTYPES_5                   TR("多段","多段按鍵")
#define TR_POTTYPES_6                   "X 軸"
#define TR_POTTYPES_7                   "Y 軸"
#define TR_POTTYPES_8                   "開關"
#define TR_VPERSISTENT_1                "禁用"
#define TR_VPERSISTENT_2                "隨飛行記錄重啟"
#define TR_VPERSISTENT_3                "隨手動重啟"
#define TR_COUNTRY_CODES_1              "美國"
#define TR_COUNTRY_CODES_2              "日本"
#define TR_COUNTRY_CODES_3              "歐洲"
#define TR_USBMODES_1                   "詢問"
#define TR_USBMODES_2                   "遊戲柄"
#define TR_USBMODES_3                   "U盤"
#define TR_USBMODES_4                   "串行"
#define TR_JACK_MODES_1                 "詢問"
#define TR_JACK_MODES_2                 "音頻"
#define TR_JACK_MODES_3                 "教練"

#define TR_SBUS_INVERSION_VALUES_1     "normal"
#define TR_SBUS_INVERSION_VALUES_2     "not inverted"
#define TR_MULTI_CUSTOM                "自定義"
#define TR_VTRIMINC_1                   "指數"
#define TR_VTRIMINC_2                   "很小"
#define TR_VTRIMINC_3                   "較小"
#define TR_VTRIMINC_4                   "中等"
#define TR_VTRIMINC_5                   "較大"
#define TR_VDISPLAYTRIMS_1              "不顯示"
#define TR_VDISPLAYTRIMS_2              "改變時"
#define TR_VDISPLAYTRIMS_3              "始終顯示"
#define TR_VBEEPCOUNTDOWN_1             "靜音"
#define TR_VBEEPCOUNTDOWN_2             "蜂鳴"
#define TR_VBEEPCOUNTDOWN_3             "語音"
#define TR_VBEEPCOUNTDOWN_4             "震動"
#define TR_VBEEPCOUNTDOWN_5             "Beeps & Haptic"
#define TR_VBEEPCOUNTDOWN_6             "Voice & Haptic"
#define TR_COUNTDOWNVALUES_1            "5秒"
#define TR_COUNTDOWNVALUES_2            "10秒"
#define TR_COUNTDOWNVALUES_3            "20秒"
#define TR_COUNTDOWNVALUES_4            "30秒"
#define TR_VVARIOCENTER_1               "音調"
#define TR_VVARIOCENTER_2               "靜音"
#define TR_CURVE_TYPES_1                "標準"
#define TR_CURVE_TYPES_2                "自定義"

#define TR_ADCFILTERVALUES_1            "全局"
#define TR_ADCFILTERVALUES_2            "禁用"
#define TR_ADCFILTERVALUES_3            "开启"

#define TR_VCURVETYPE_1                "單邊"
#define TR_VCURVETYPE_2                "指數"
#define TR_VCURVETYPE_3                "函數"
#define TR_VCURVETYPE_4                "自定義"
#define TR_VMLTPX_1                    "相加"
#define TR_VMLTPX_2                    "相乘"
#define TR_VMLTPX_3                    "替換"

#define TR_CSWTIMER                    "定時"
#define TR_CSWSTICKY                   "粘滯"
#define TR_CSWSTAY                     "邊沿"

#define TR_SF_TRAINER                  "教練"
#define TR_SF_INST_TRIM                "搖桿值存儲到微調"
#define TR_SF_RESET                    "重啟"
#define TR_SF_SET_TIMER                "設置"
#define TR_SF_VOLUME                   "音量"
#define TR_SF_FAILSAFE                 "設置失控保護"
#define TR_SF_RANGE_CHECK              "測距模式"
#define TR_SF_MOD_BIND                 "模塊對頻"
#define TR_SF_RGBLEDS                  "RGB LED 燈"

#define TR_SOUND                       "播放聲音"
#define TR_PLAY_TRACK                  TR("Ply Trk", "播放音頻文件")
#define TR_PLAY_VALUE                  "播放數值"
#define TR_SF_HAPTIC                   "振動"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua腳本")
#define TR_SF_BG_MUSIC                 "播放背景音樂"
#define TR_SF_BG_MUSIC_PAUSE           "暫停背景音樂"
#define TR_SF_LOGS                     "記錄日誌LOG"
#define TR_ADJUST_GVAR                 "修改全局變量GV值"
#define TR_SF_BACKLIGHT                "背光"
#define TR_SF_VARIO                    "Vario傳感器"
#define TR_SF_TEST                     "測試"
#define TR_SF_SAFETY                   "鎖定通道值"

#define TR_SF_SCREENSHOT               "截屏"
#define TR_SF_RACING_MODE              "競速模式"
#define TR_SF_DISABLE_TOUCH            "禁用觸摸"
#define TR_SF_DISABLE_AUDIO_AMP        "關閉音頻功放"
#define TR_SF_SET_SCREEN               TR_BW_COL("設置主屏", "選擇主屏")
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "屏幕顯示圖傳"

#define TR_FSW_RESET_TELEM             "回傳參數"

#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS_1          TR("計時1", "計時器1")
#define TR_FSW_RESET_TIMERS_2          TR("計時2", "計時器2")
#define TR_FSW_RESET_TIMERS_3          TR("計時3", "計時器3")

#define TR_VFSWRESET_1                 TR_FSW_RESET_TIMERS_1
#define TR_VFSWRESET_2                 TR_FSW_RESET_TIMERS_2
#define TR_VFSWRESET_3                 TR_FSW_RESET_TIMERS_3
#define TR_VFSWRESET_4                 TR("全部","飛行")
#define TR_VFSWRESET_5                 TR_FSW_RESET_TELEM
#define TR_VFSWRESET_6                 TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS_1                TR("Bp1","Beep1")
#define TR_FUNCSOUNDS_2                TR("Bp2","Beep2")
#define TR_FUNCSOUNDS_3                TR("Bp3","Beep3")
#define TR_FUNCSOUNDS_4                TR("Wrn1","Warn1")
#define TR_FUNCSOUNDS_5                TR("Wrn2","Warn2")
#define TR_FUNCSOUNDS_6                TR("Chee","Cheep")
#define TR_FUNCSOUNDS_7                TR("Rata","Ratata")
#define TR_FUNCSOUNDS_8                "Tick"
#define TR_FUNCSOUNDS_9                TR("Sirn","Siren")
#define TR_FUNCSOUNDS_10               "Ring"
#define TR_FUNCSOUNDS_11               TR("SciF","SciFi")
#define TR_FUNCSOUNDS_12               TR("Robt","Robot")
#define TR_FUNCSOUNDS_13               TR("Chrp","Chirp")
#define TR_FUNCSOUNDS_14               "Tada"
#define TR_FUNCSOUNDS_15               TR("Crck","Crickt")
#define TR_FUNCSOUNDS_16               TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM_1              "公制"
#define TR_VUNITSSYSTEM_2              TR("英制","英制")
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
#define TR_VTELEMUNIT_20               "rpm"
#define TR_VTELEMUNIT_21               "°"
#define TR_VTELEMUNIT_22               "rad"
#define TR_VTELEMUNIT_23               "ml"
#define TR_VTELEMUNIT_24               "fOz"
#define TR_VTELEMUNIT_25               "mlm"
#define TR_VTELEMUNIT_26               "Hz"
#define TR_VTELEMUNIT_27               "mS"
#define TR_VTELEMUNIT_28               "uS"
#define TR_VTELEMUNIT_29               "km"
#define TR_VTELEMUNIT_30               "dBm"

#define TR_VTELEMSCREENTYPE_1          "禁用"
#define TR_VTELEMSCREENTYPE_2          "數值"
#define TR_VTELEMSCREENTYPE_3          "條形圖"
#define TR_VTELEMSCREENTYPE_4          "腳本"
#define TR_GPSFORMAT_1                 "DMS"
#define TR_GPSFORMAT_2                 "NMEA"


#define TR_VSWASHTYPE_1                "---"
#define TR_VSWASHTYPE_2                "120"
#define TR_VSWASHTYPE_3                "120X"
#define TR_VSWASHTYPE_4                "140"
#define TR_VSWASHTYPE_5                "90"

#define TR_STICK_NAMES0                "Rud"
#define TR_STICK_NAMES1                "Ele"
#define TR_STICK_NAMES2                "Thr"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#define TR_ON_ONE_SWITCHES_1           "ON"
#define TR_ON_ONE_SWITCHES_2           "One"

#define TR_HATSMODE                    "按鍵帽模式"
#define TR_HATSOPT_1                   "微調"
#define TR_HATSOPT_2                   "導航鍵"
#define TR_HATSOPT_3                   "可切換"
#define TR_HATSOPT_4                   "全局"
#define TR_HATSMODE_TRIMS              "按鍵帽: 微調"
#define TR_HATSMODE_KEYS               "按鍵帽: 導航鍵"
#define TR_HATSMODE_KEYS_HELP          "左側按鍵帽:\n"\
                                       "   右 = MDL\n"\
                                       "   上 = SYS\n"\
                                       "   下 = TELE\n"\
                                       "\n"\
                                       "右側按鍵帽:\n"\
                                       "   左 = PAGE<\n"\
                                       "   右 = PAGE>\n"\
                                       "   上 = PREV/INC\n"\
                                       "   下 = NEXT/DEC"

#define TR_ROTARY_ENC_OPT_1        "正常"
#define TR_ROTARY_ENC_OPT_2        "反向"
#define TR_ROTARY_ENC_OPT_3        "V-I H-N"
#define TR_ROTARY_ENC_OPT_4        "V-I H-A"
#define TR_ROTARY_ENC_OPT_5        "V-N E-I"

#define TR_IMU_VSRCRAW_1             "TltX"
#define TR_IMU_VSRCRAW_2             "TltY"
#define TR_CYC_VSRCRAW_1             "CYC1"
#define TR_CYC_VSRCRAW_2             "CYC2"
#define TR_CYC_VSRCRAW_3             "CYC3"

#define TR_SRC_BATT                    "Batt"
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES_1                 "OFF"
#define TR_VTMRMODES_2                 "ON"
#define TR_VTMRMODES_3                 "Strt"
#define TR_VTMRMODES_4                 "THs"
#define TR_VTMRMODES_5                 "TH%"
#define TR_VTMRMODES_6                 "THt"
#define TR_VTRAINER_MASTER_OFF         "禁用"
#define TR_VTRAINER_MASTER_JACK        "教練主機/教練口"
#define TR_VTRAINER_SLAVE_JACK         "學生從機/教練口"
#define TR_VTRAINER_MASTER_SBUS_MODULE "教練主機/SBUS模塊"
#define TR_VTRAINER_MASTER_CPPM_MODULE "教練從機/CPPM模塊"
#define TR_VTRAINER_MASTER_BATTERY     "教練主機/端口"
#define TR_VTRAINER_BLUETOOTH_1        "教練主機/藍牙"
#define TR_VTRAINER_BLUETOOTH_2        "教練從機/藍牙"
#define TR_VTRAINER_MULTI              "教練主機/多協議"
#define TR_VTRAINER_CRSF               "教練主機/CRSF"
#define TR_VFAILSAFE_1                 "未設置"
#define TR_VFAILSAFE_2                 "失控保持"
#define TR_VFAILSAFE_3                 "自定義"
#define TR_VFAILSAFE_4                 "無脈衝"
#define TR_VFAILSAFE_5                 "接收機"
#define TR_VSENSORTYPES_1              "自定義"
#define TR_VSENSORTYPES_2              "運算"
#define TR_VFORMULAS_1                 "加"
#define TR_VFORMULAS_2                 "平均值"
#define TR_VFORMULAS_3                 "最小值"
#define TR_VFORMULAS_4                 "最大值"
#define TR_VFORMULAS_5                 "乘"
#define TR_VFORMULAS_6                 "總計值"
#define TR_VFORMULAS_7                 "單節電池"
#define TR_VFORMULAS_8                 "消耗量"
#define TR_VFORMULAS_9                 "距離"
#define TR_VPREC_1                     "0.--"
#define TR_VPREC_2                     "0.0-"
#define TR_VPREC_3                     "0.00"
#define TR_VCELLINDEX_1                "最低值"
#define TR_VCELLINDEX_2                "1"
#define TR_VCELLINDEX_3                "2"
#define TR_VCELLINDEX_4                "3"
#define TR_VCELLINDEX_5                "4"
#define TR_VCELLINDEX_6                "5"
#define TR_VCELLINDEX_7                "6"
#define TR_VCELLINDEX_8                "7"
#define TR_VCELLINDEX_9                "8"
#define TR_VCELLINDEX_10               "最高值"
#define TR_VCELLINDEX_11               "差值"
#define TR_SUBTRIMMODES_1              CHAR_DELTA "(中点)"
#define TR_SUBTRIMMODES_2              "=(整體)"
#define TR_TIMER_DIR_1                 TR("Remain", "Show Remain")
#define TR_TIMER_DIR_2                 TR("Elaps.", "Show Elapsed")

#define TR_FONT_SIZES_1                "STD"
#define TR_FONT_SIZES_2                "BOLD"
#define TR_FONT_SIZES_3                "XXS"
#define TR_FONT_SIZES_4                "XS"
#define TR_FONT_SIZES_5                "L"
#define TR_FONT_SIZES_6                "XL"
#define TR_FONT_SIZES_7                "XXL"

#define TR_ENTER                       "[ENTER]"
#define TR_OK                          TR_BW_COL(TR("\010\010\010[OK]", "\010\010\010\010\010[OK]"), "Ok")
#define TR_EXIT                        TR_BW_COL("EXIT", "RTN")

#define TR_YES                         "是"
#define TR_NO                          "否"
#define TR_DELETEMODEL                 "刪除 模型"
#define TR_COPYINGMODEL                "複製模型..."
#define TR_MOVINGMODEL                 "移動模型..."
#define TR_LOADINGMODEL                "載入模型..."
#define TR_UNLABELEDMODEL              "未分組"
#define TR_NAME                        "名稱"
#define TR_MODELNAME                   "模型名稱"
#define TR_PHASENAME                   "模式名稱"
#define TR_MIXNAME                     "混控名稱"
#define TR_INPUTNAME                   TR("名稱", "輸入名稱")
#define TR_EXPONAME                    TR("名稱", "曲線名稱")
#define TR_BITMAP                      "模型圖片"
#define TR_NO_PICTURE                  "無圖片"
#define TR_TIMER                       TR("計時器", "計時器 ")
#define TR_NO_TIMERS                   "No timers"
#define TR_START                       "開始"
#define TR_NEXT                        "Next"
#define TR_ELIMITS                     TR("擴展行程", "擴展通道行程")
#define TR_ETRIMS                      TR("擴展微調", "擴展微調量")
#define TR_TRIMINC                     "微調步幅"
#define TR_DISPLAY_TRIMS               TR("微調顯示", "微調顯示")
#define TR_TTRACE                      TR("輸入源", "輸入源")
#define TR_TTRIM                       TR("油門微調僅怠速", "油門微調只調整怠速")
#define TR_TTRIM_SW                    TR("微調選擇", "微調選擇")
#define TR_BEEPCTR                     TR("中點提示音", "中點蜂鳴提示音")
#define TR_USE_GLOBAL_FUNCS            TR("全局功能", "全局功能可介入")
#define TR_PPMFRAME                    "PPM幀"
#define TR_REFRESHRATE                 TR("刷新率", "刷新速率")
#define TR_WARN_BATTVOLTAGE           TR("輸出為電池電壓: ", "注意輸出電平為電池電壓")
#define TR_WARN_5VOLTS                 "注意輸出電平是5V"
#define TR_MS                          "ms"
#define TR_SWITCH                      "開關"
#define TR_FS_COLOR_LIST_1             "Custom"
#define TR_FS_COLOR_LIST_2             "Off"
#define TR_FS_COLOR_LIST_3             "White"
#define TR_FS_COLOR_LIST_4             "Red"
#define TR_FS_COLOR_LIST_5             "Green"
#define TR_FS_COLOR_LIST_6             "Yellow"
#define TR_FS_COLOR_LIST_7             "Orange"
#define TR_FS_COLOR_LIST_8             "Blue"
#define TR_FS_COLOR_LIST_9             "Pink"
#define TR_GROUP                       "Group"
#define TR_GROUP_ALWAYS_ON             "Always on"
#define TR_LUA_OVERRIDE                "Allow Lua override"
#define TR_GROUPS                      "Always on groups"
#define TR_LAST                        "Last"
#define TR_MORE_INFO                   "More info"
#define TR_SWITCH_TYPE                 "Type"
#define TR_SWITCH_STARTUP              "Startup"
#define TR_SWITCH_GROUP                "Group"
#define TR_SF_SWITCH                   "Trigger"
#define TR_TRIMS                       "微調"
#define TR_FADEIN                      "漸入"
#define TR_FADEOUT                     "漸出"
#define TR_DEFAULT                     "(默認)"
#define   TR_CHECKTRIMS                 TR_BW_COL("\006檢查\012微調", "檢查當前飛行模式微調")
#define TR_SWASHTYPE                   "斜盤類型"
#define TR_COLLECTIVE                  TR("螺距源", "螺距混控源")
#define TR_AILERON                     TR("橫滾源", "橫滾混控源")
#define TR_ELEVATOR                    TR("俯仰源", "俯仰混控源")
#define TR_SWASHRING                   "斜盤行程"
#define TR_MODE                        "模式"
#define TR_LEFT_STICK                  "左搖桿"
#define TR_SUBTYPE                     "子類型"
#define TR_NOFREEEXPO                  "指數曲線已滿!"
#define TR_NOFREEMIXER                 "混控數量已滿!"
#define TR_SOURCE                      "輸入源"
#define TR_WEIGHT                      "比例"
#define TR_SIDE                        "單邊"
#define TR_OFFSET                      "偏移量"
#define TR_TRIM                        "使用微調"
#define TR_CURVE                       "曲線"
#define TR_FLMODE                      TR("飛行模式", "飛行模式")
#define TR_MIXWARNING                  "警告"
#define TR_OFF                         "禁用"
#define TR_ANTENNA                     "天線"
#define TR_NO_INFORMATION              TR("無信息", "無信息")
#define TR_MULTPX                      "疊加方式"
#define TR_DELAYDOWN                   TR("下延時", "下行延時")
#define TR_DELAYUP                     "上行延時"
#define TR_SLOWDOWN                    TR("下慢放", "下行慢動作")
#define TR_SLOWUP                      "上行慢動作"
#define TR_CV                          "曲線"
#define TR_GV                          TR("G", "GV")
#define TR_RANGE                       "範圍"
#define TR_CENTER                      "中點"
#define TR_ALARM                       "報警"
#define TR_BLADES                      "Blades/Poles"
#define TR_SCREEN                      "屏幕\001"
#define TR_SOUND_LABEL                 "聲音"
#define TR_LENGTH                      "音長"
#define TR_BEEP_LENGTH                 "提示音長度"
#define TR_BEEP_PITCH                  "提示音音調"
#define TR_HAPTIC_LABEL                "振動"
#define TR_STRENGTH                    "強度"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "補償"
#define TR_IMU_MAX                     "最大值"
#define TR_CONTRAST                    "對比度"
#define TR_ALARMS_LABEL                "警告"
#define TR_BATTERY_RANGE               TR("電壓範圍", "電壓測量範圍")
#define TR_BATTERYCHARGING             "充電中..."
#define TR_BATTERYFULL                 "電量已滿"
#define TR_BATTERYNONE                 "空!"
#define TR_BATTERYWARNING              "低電壓報警值"
#define TR_INACTIVITYALARM             "長時間無操作"
#define TR_MEMORYWARNING               "內存不足"
#define TR_ALARMWARNING                "靜音"
#define TR_RSSI_SHUTDOWN_ALARM         TR("關機檢查RSSI", "關機時檢查RSSI")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("關機檢查教練", "關機時檢查教練信號")
#define TR_MODEL_STILL_POWERED         "模型未斷電"
#define TR_TRAINER_STILL_CONNECTED     "教練信號未斷開"
#define TR_USB_STILL_CONNECTED         "USB未斷開"
#define TR_MODEL_SHUTDOWN              "關機 ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "確認"
#define TR_THROTTLE_LABEL              "油門"
#define TR_THROTTLE_START              "油門開始"
#define TR_THROTTLEREVERSE             TR("油門反向", "油門反向")
#define TR_MINUTEBEEP                  TR("分", "分鐘播報")
#define TR_BEEPCOUNTDOWN               "倒數"
#define TR_PERSISTENT                  TR("關機保持", "關機保持")
#define TR_BACKLIGHT_LABEL             "背光"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "狀態"
#define TR_BLONBRIGHTNESS              "背光開"
#define TR_BLOFFBRIGHTNESS             "背光關"
#define TR_KEYS_BACKLIGHT              "按鍵背光"
#define TR_BLCOLOR                     "顏色"
#define TR_SPLASHSCREEN                "開機圖片"
#define TR_PLAY_HELLO                  "開機語音"
#define TR_PWR_ON_DELAY                "開機延遲"
#define TR_PWR_OFF_DELAY               "關機延遲"
#define TR_PWR_AUTO_OFF                TR("自動關機","自動關機")
#define TR_PWR_ON_OFF_HAPTIC           TR("開關機震動","開關機震動提示")
#define TR_THROTTLE_WARNING            TR("油門狀態", "油門狀態")
#define TR_CUSTOM_THROTTLE_WARNING     TR("自定位置", "自定油門位置?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("位置 %", "油門位置 %")
#define TR_SWITCHWARNING               TR("開關位置", "開關位置")
#define TR_POTWARNINGSTATE             "旋鈕和滑塊"
#define TR_POTWARNING                  TR("旋鈕位置", "旋鈕位置")
#define TR_TIMEZONE                    "時區"
#define TR_ADJUST_RTC                  "時鐘設置"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("通道順序", "默認通道順序")
#define TR_STICKS                      "搖桿(軸)"
#define TR_POTS                        "電位器"
#define TR_SWITCHES                    "開關"
#define TR_SWITCHES_DELAY              TR("開關經過延時", "延時播放(開關經過)")
#define TR_SLAVE                       "從機"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "倍率"
#define TR_CAL                         "校準"
#define TR_CALIBRATION                 BUTTON("校準")
#define TR_VTRIM                       "微調-+"
#define TR_CALIB_DONE                  "校準完成"
#define TR_MENUTOSTART                 "按" TR_ENTER " 開始"
#define TR_MENUWHENDONE                TR_ENTER " 完成"
#define TR_AXISDIR                     "AXIS DIR"
#define TR_MENUAXISDIR                 "[ENTER LONG] "  TR_AXISDIR
#define TR_SETMIDPOINT                 TR_BW_COL(TR_SFC_AIR("校準中點", TR("校準中點", "校準中點")), "校準中點，按[Enter]保存")
#define TR_MOVESTICKSPOTS              TR_BW_COL(TR_SFC_AIR("校準邊界", "校準邊界"), "校準邊界，按[Enter]保存並完成")
#define TR_NODATA                      "NO DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "內置 GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "LUA 腳本"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_DURATION_MS                 TR("[D]","持續時間(ms): ")
#define TR_INTERVAL_MS                 TR("[I]","間隔時間(ms): ")
#define TR_MEM_USED_SCRIPT             "腳本(B): "
#define TR_MEM_USED_WIDGET             "小部件(B): "
#define TR_MEM_USED_EXTRA              "附加(B): "
#define TR_STACK_MIX                   "混控: "
#define TR_STACK_AUDIO                 "音頻: "
#define TR_GPS_FIX_YES                 "修正: 是"
#define TR_GPS_FIX_NO                  "修正: 否"
#define TR_GPS_SATS                    "衛星: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "選單: "
#define TR_TIMER_LABEL                 "Timer"
#define TR_THROTTLE_PERCENT_LABEL      "油門 %"
#define TR_BATT_LABEL                  "電池"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " to 復位"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "模型"
#define TR_FM                          TR_SFC_AIR("DM", "FM")
#define TR_EEPROMLOWMEM                "EEPROM low mem"
#define TR_PRESS_ANY_KEY_TO_SKIP       "按任意鍵跳過"
#define TR_THROTTLE_NOT_IDLE           "請將油門拉至最低!"
#define TR_ALARMSDISABLED              "已禁用報警"
#define TR_PRESSANYKEY                 TR("\010按任意鍵", "按任意鍵")
#define TR_BADEEPROMDATA               "存儲數據錯誤"
#define TR_BAD_RADIO_DATA              "無法讀取系統設置參數, 請檢查SD卡"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            "格式化存儲"
#define TR_STORAGE_FORMAT              "存儲準備"
#define TR_EEPROMOVERFLOW              "存儲超限"
#define TR_RADIO_SETUP                 "系統設置"
#define TR_MENUVERSION                 "版本"
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "類比輸入測試")
#define TR_MENU_RADIO_ANALOGS_CALIB     "已校準的類比值"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "原始類比值 (5 Hz)"
#define TR_MENU_FSWITCH                "可自定義開關"
#define   TR_TRIMS2OFFSETS              TR_BW_COL("\006微調存至中點", "將當前微調量保存至中點")
#define TR_CHANNELS2FAILSAFE           "導入當前所有通道值"
#define TR_CHANNEL2FAILSAFE            "導入當前通道值"
#define TR_MENUMODELSEL                TR("模型選擇", "模型選擇")
#define TR_MENU_MODEL_SETUP            TR("模型設置", "模型設置")
#define TR_MENUCURVE                   "曲線"
#define TR_MENULOGICALSWITCH           "邏輯開關"
#define TR_MENUSTAT                    "統計"
#define TR_MENUDEBUG                   "Debug"
#define TR_MONITOR_CHANNELS            "%d-%d通道查看"
#define TR_MONITOR_OUTPUT_DESC         "輸出查看"
#define TR_MONITOR_MIXER_DESC          "混控查看"
#define TR_RECEIVER_NUM                TR("RxNum", "接收機ID.")
#define TR_RECEIVER                    "接收機ID"
#define TR_MULTI_RFTUNE                TR("頻率微調", "頻率微調")
#define TR_MULTI_RFPOWER               "發射功率"
#define TR_MULTI_WBUS                  "輸出"
#define TR_MULTI_TELEMETRY             "回傳"
#define TR_MULTI_VIDFREQ               TR("圖傳頻率", "圖傳頻率")
#define TR_RF_POWER                    "發射功率"
#define TR_MULTI_FIXEDID               TR("固定ID", "固定ID")
#define TR_MULTI_OPTION                TR("選項值", "選項值")
#define TR_MULTI_AUTOBIND              TR("對頻通道", "通道控制對頻")
#define TR_DISABLE_CH_MAP              TR("禁用通道映射", "禁用通道重映射")
#define TR_DISABLE_TELEM               TR("禁用回傳", "禁用回傳")
#define TR_MULTI_LOWPOWER              TR("低功率", "低功率模式")
#define TR_MULTI_LNA_DISABLE           "禁用接收放大器"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("開", "啟用")
#define TR_DISABLE_INTERNAL            TR("禁用內置發射", "禁用內置發射")
#define TR_MODULE_NO_SERIAL_MODE       TR("非端口模式", "未開啟端口模式")
#define TR_MODULE_NO_INPUT             TR("無輸入", "無串行數據輸入")
#define TR_MODULE_NO_TELEMETRY         TR3("無回傳", "未收到多協議回傳信息", "未收到多協議回傳信息")
#define TR_MODULE_WAITFORBIND          "使用當前協議對頻"
#define TR_MODULE_BINDING              TR(".對頻中","對頻中")
#define TR_MODULE_UPGRADE_ALERT        TR3("需要升級", "多協議模塊需要升級", "多協議模塊需要升級")
#define TR_MODULE_UPGRADE              TR("需要升級", "多協議模塊需要升級")
#define TR_REBIND                      "需要重新對頻"
#define TR_REG_OK                      "註冊成功"
#define TR_BIND_OK                     "對頻成功"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 開啟回傳"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 關閉回傳"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 開啟回傳"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 關閉回傳"
#define TR_PROTOCOL_INVALID            TR("無效協議", "無效協議")
#define TR_MODULE_STATUS               TR("狀態", "多協議狀態")
#define TR_MODULE_SYNC                 TR("同步", "協議同步狀態")
#define TR_MULTI_SERVOFREQ             TR("舵機頻率", "舵機更新頻率")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("頻道", "選擇發射頻道")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "使用 FlySky RSSI 值無需重縮放")
#define TR_GPS_COORDS_FORMAT           TR("方向角", "方向角格式")
#define TR_VARIO                       TR("變化率", "高度變化率")
#define TR_PITCH_AT_ZERO               "基準音調"
#define TR_PITCH_AT_MAX                "最高音調"
#define TR_REPEAT_AT_ZERO              "無變化報警"
#define TR_BATT_CALIB                  TR("電池校準", "電池電壓校準")
#define TR_CURRENT_CALIB               "電流校準"
#define TR_VOLTAGE                     TR("電壓源", "電壓來源")
#define TR_SELECT_MODEL                "選擇模型"
#define TR_MANAGE_MODELS               "模型管理"
#define TR_MODELS                      "個模型"
#define TR_SELECT_MODE                 "選擇模式"
#define TR_CREATE_MODEL                "創建模型"
#define TR_FAVORITE_LABEL              "收藏夾"
#define TR_MODELS_MOVED                "未使用模型移至"
#define TR_NEW_MODEL                   "新建模型"
#define TR_INVALID_MODEL               "無效模型"
#define TR_EDIT_LABELS                 "編輯分組"
#define TR_LABEL_MODEL                 "模型標簽"
#define TR_MOVE_UP                     "上移"
#define TR_MOVE_DOWN                   "下移"
#define TR_ENTER_LABEL                 "輸入分組名稱"
#define TR_LABEL                       "分組"
#define TR_LABELS                      "分組"
#define TR_CURRENT_MODEL               "當前模型"
#define TR_ACTIVE                      "當前模型"
#define TR_NEW                         "新建分組"
#define TR_NEW_LABEL                   "新建分組"
#define TR_RENAME_LABEL                "分組改名"
#define TR_DELETE_LABEL                "刪除分組"
#define TR_DUPLICATE_MODEL             "複製模型"
#define TR_COPY_MODEL                  "複製模型"
#define TR_MOVE_MODEL                  "移動模型"
#define TR_BACKUP_MODEL                "備份模型"
#define TR_DELETE_MODEL                "刪除模型"
#define TR_RESTORE_MODEL               "恢復模型"
#define TR_DELETE_ERROR                "刪除失敗"
#define TR_SDCARD_ERROR                TR("存儲卡錯誤", "存儲卡錯誤")
#define TR_SDCARD                      "存儲卡"
#define TR_NO_FILES_ON_SD              "存儲卡中沒有文件!"
#define TR_NO_SDCARD                   "存儲卡未安裝"
#define TR_WAITING_FOR_RX              "等待接收機響應..."
#define TR_WAITING_FOR_TX              "等待發射機響應..."
#define TR_WAITING_FOR_MODULE          TR("等待模塊", "等待模塊響應...")
#define TR_NO_TOOLS                    "無可用附加功能"
#define TR_NORMAL                      "正常"
#define TR_NOT_INVERTED                "正向"
#define TR_NOT_CONNECTED               "未連接"
#define TR_CONNECTED                   "已連接"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH關回傳", "16CH 禁用回傳")
#define TR_16CH_WITH_TELEMETRY         TR("16CH開回傳", "16CH 開啟回傳")
#define TR_8CH_WITH_TELEMETRY          TR("8CH開回傳", "8CH 開啟回傳")
#define TR_EXT_ANTENNA                 "外置天線"
#define TR_PIN                         "插針"
#define TR_UPDATE_RX_OPTIONS           "是否升級接收機?"
#define TR_UPDATE_TX_OPTIONS           "是否升級發射機?"
#define TR_MODULES_RX_VERSION          BUTTON("模塊/接收機版本")
#define TR_SHOW_MIXER_MONITORS         "顯示通道混控"
#define TR_MENU_MODULES_RX_VERSION     "模塊/接收機版本"
#define TR_MENU_FIRM_OPTIONS           "韌體選項"
#define TR_IMU                         "陀螺儀"
#define TR_STICKS_POTS_SLIDERS         "搖桿(軸)/旋鈕/滑塊"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM搖桿(軸)/旋鈕/滑塊"
#define TR_RF_PROTOCOL                 "發射協議"
#define TR_MODULE_OPTIONS              "選項"
#define TR_POWER                       "功率"
#define TR_NO_TX_OPTIONS               "無發射機選項"
#define TR_RTC_BATT                    "時間驅動電池電壓"
#define TR_POWER_METER_EXT             "功率計 (外置)"
#define TR_POWER_METER_INT             "功率計 (內置)"
#define TR_SPECTRUM_ANALYSER_EXT       "頻譜儀 (外置)"
#define TR_SPECTRUM_ANALYSER_INT       "頻譜儀 (內置)"
#define TR_GHOST_MODULE_CONFIG         "Ghost module config"
#define TR_GPS_MODEL_LOCATOR           "GPS model locator"
#define TR_REFRESH                     "Refresh"
#define TR_SDCARD_FULL                 "SD卡已滿"
#define TR_SDCARD_FULL_EXT              TR_BW_COL(TR_SDCARD_FULL "\036日誌和 " LCDW_128_LINEBREAK "截屏功能將被禁用", TR_SDCARD_FULL "\n日誌和截屏功能將被禁用")
#define TR_NEEDS_FILE                  "需要文件名包含"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "不兼容"
#define TR_WARNING                     "警告"
#define TR_STORAGE_WARNING             "存儲"
#define TR_THROTTLE_UPPERCASE          "油門"
#define TR_ALARMSWARN                  "警告"
#define TR_SWITCHWARN                  TR("開關", "開關位置")
#define TR_FAILSAFEWARN                "失控保護"
#define TR_TEST_WARNING                TR("測試版", "測試版本")
#define TR_TEST_NOTSAFE                "只用於測試"
#define TR_WRONG_SDCARDVERSION         TR("需要版本: ", "請將SD卡文件更換為正確版本: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC紐扣電池電壓低"
#define TR_WARN_MULTI_LOWPOWER         "低功率模式"
#define TR_BATTERY                     "電池"
#define TR_WRONG_PCBREV                "錯誤的硬件類型"
#define TR_EMERGENCY_MODE              "緊急模式"
#define TR_NO_FAILSAFE                 "失控保護未設置"
#define TR_KEYSTUCK                    "檢測到有按鍵卡住"
#define TR_VOLUME                      "音量"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "亮度"
#define TR_CPU_TEMP                    "CPU 溫度"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "主板溫度"
#define TR_TTL_WARNING                 "重要提示: TX和RX引腳電平不能超過3.3V !"
#define TR_FUNC                        "功能"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "持續時間"
#define TR_DELAY                       "延時"
#define TR_NO_SOUNDS_ON_SD             "存儲卡中無音頻"
#define TR_NO_MODELS_ON_SD             "存儲卡中無模型"
#define TR_NO_BITMAPS_ON_SD            "存儲卡中無圖片"
#define TR_NO_SCRIPTS_ON_SD            "存儲卡中無腳本"
#define TR_SCRIPT_SYNTAX_ERROR         "腳本程序語法錯誤"
#define TR_SCRIPT_PANIC                "腳本異常"
#define TR_SCRIPT_KILLED               "清除腳本進程"
#define TR_SCRIPT_ERROR                "未知錯誤"
#define TR_PLAY_FILE                   "播放"
#define TR_DELETE_FILE                 "刪除"
#define TR_COPY_FILE                   "複製"
#define TR_RENAME_FILE                 "重取名"
#define TR_ASSIGN_BITMAP               "設置為模型圖片"
#define TR_ASSIGN_SPLASH               "設置為開機圖片"
#define TR_EXECUTE_FILE                "執行"
#define TR_REMOVED                     "已刪除"
#define TR_SD_INFO                     "信息"
#define TR_NA                          "N/A"
#define TR_FORMATTING                  "正在格式化..."
#define TR_TEMP_CALIB                  "溫度校準"
#define TR_TIME                        "時間"
#define TR_MAXBAUDRATE                 "最大帶寬"
#define TR_BAUDRATE                    "波特率"
#define TR_CRSF_ARMING_MODE            "解鎖類型"
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 "採樣模式"
#define TR_SAMPLE_MODES_1              "標準"
#define TR_SAMPLE_MODES_2              "OneBit"
#define TR_LOADING                     "加載中..."
#define TR_DELETE_THEME                "刪除主題?"
#define TR_SAVE_THEME                  "保存主題?"
#define TR_EDIT_COLOR                  "编辑颜色"
#define TR_NO_THEME_IMAGE              "没有预览图"
#define TR_BACKLIGHT_TIMER             "持續時間"

#define TR_MODEL_QUICK_SELECT        "快速選擇模型"
#define TR_LABELS_SELECT             "標籤類型選擇"
#define TR_LABELS_MATCH              "標籤匹配類型"
#define TR_FAV_MATCH                 "收藏夾匹配類型"
#define TR_LABELS_SELECT_MODE_1      "復選項"
#define TR_LABELS_SELECT_MODE_2      "單選項"
#define TR_LABELS_MATCH_MODE_1       "全部匹配"
#define TR_LABELS_MATCH_MODE_2       "任意匹配"
#define TR_FAV_MATCH_MODE_1          "必須匹配"
#define TR_FAV_MATCH_MODE_2          "可選匹配"

#define TR_SELECT_TEMPLATE_FOLDER      "選擇一個樣板檔案夾:"
#define TR_SELECT_TEMPLATE             "選擇一個樣板:"
#define TR_NO_TEMPLATES                "在此檔案夾中未找到模型樣板"
#define TR_SAVE_TEMPLATE               "存儲為樣板"
#define TR_BLANK_MODEL                 "空白模型"
#define TR_BLANK_MODEL_INFO            "創建一個空白模型"
#define TR_FILE_EXISTS                 "文件已存在"
#define TR_ASK_OVERWRITE               "是否覆蓋?"

#define TR_BLUETOOTH                   "藍牙"
#define TR_BLUETOOTH_DISC              "發現"
#define TR_BLUETOOTH_INIT              "初始化"
#define TR_BLUETOOTH_DIST_ADDR         "目標地址"
#define TR_BLUETOOTH_LOCAL_ADDR        "本機地址"
#define TR_BLUETOOTH_PIN_CODE          "PIN碼"
#define TR_BLUETOOTH_NODEVICES         "未找到設備"
#define TR_BLUETOOTH_SCANNING          "正在掃描..."
#define TR_BLUETOOTH_BAUDRATE          "藍牙帶寬"
#define TR_BLUETOOTH_MODES_1         "---"
#define TR_BLUETOOTH_MODES_2         "回傳"
#define TR_BLUETOOTH_MODES_3         "教練"
#define TR_BLUETOOTH_MODES_4         "啟用"

#define TR_SD_INFO_TITLE               "存儲卡詳情"
#define TR_SD_SPEED                    "速度:"
#define TR_SD_SECTORS                  "扇區:"
#define TR_SD_SIZE                     "容量:"
#define TR_TYPE                        "類型"
#define TR_GVARS                       "全局變量"
#define TR_GLOBAL_VAR                  "全局變量"
#define TR_OWN                         "Own"
#define TR_DATE                        "日期"
#define TR_MONTHS_1                    "Jan"
#define TR_MONTHS_2                    "Feb"
#define TR_MONTHS_3                    "Mar"
#define TR_MONTHS_4                    "Apr"
#define TR_MONTHS_5                    "May"
#define TR_MONTHS_6                    "Jun"
#define TR_MONTHS_7                    "Jul"
#define TR_MONTHS_8                    "Aug"
#define TR_MONTHS_9                    "Sep"
#define TR_MONTHS_10                   "Oct"
#define TR_MONTHS_11                   "Nov"
#define TR_MONTHS_12                   "Dec "
#define TR_ROTARY_ENCODER              "滾輪"
#define TR_ROTARY_ENC_MODE              TR("滾輪模式","滾輪模式")
#define TR_CHANNELS_MONITOR            "通道查看器"
#define TR_MIXERS_MONITOR              "混控查看器"
#define TR_PATH_TOO_LONG               "路徑太長"
#define TR_VIEW_TEXT                   "查看文本"
#define TR_FLASH_BOOTLOADER            "更新引導程序"
#define TR_FLASH_DEVICE                TR("更新設備","更新設備")
#define TR_FLASH_EXTERNAL_DEVICE       TR("從S.Port更新", "從 S.Port端口更新外設備韌體")
#define TR_FLASH_RECEIVER_OTA          "Flash receiver OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("刷新藍牙模塊", "刷新藍牙模塊")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Flash pwr mngt unit"
#define TR_DEVICE_NO_RESPONSE          TR("無響應", "設備無響應")
#define TR_DEVICE_FILE_ERROR           TR("文件錯誤", "設備文件錯誤")
#define TR_DEVICE_DATA_REFUSED         TR("數據被拒絕", "設備數據被拒絕")
#define TR_DEVICE_WRONG_REQUEST        TR("無法訪問設備", "無法訪問設備")
#define TR_DEVICE_FILE_REJECTED        TR("文件被拒絕", "設備文件被拒絕")
#define TR_DEVICE_FILE_WRONG_SIG       TR("簽名無效", "錯誤的文件簽名")
#define TR_CURRENT_VERSION             TR("當前版本: ", "當前版本: ")
#define TR_FLASH_INTERNAL_MODULE       TR("更新內置韌體", "更新內置模塊韌體")
#define TR_FLASH_INTERNAL_MULTI        TR("更新內置多協議", "更新內置多協議模塊韌體")
#define TR_FLASH_EXTERNAL_MODULE       TR("更新外置模塊", "更新外置模塊韌體")
#define TR_FLASH_EXTERNAL_MULTI        TR("更新外置多協議", "更新外置多協議模塊韌體")
#define TR_FLASH_EXTERNAL_ELRS         TR("更新外置ELRS", "更新外置ELRS模塊韌體")
#define TR_FIRMWARE_UPDATE_ERROR       TR("更新失败", "韌體更新失敗")
#define TR_FIRMWARE_UPDATE_SUCCESS     "韌體更新成功"
#define TR_WRITING                     "正在寫入..."
#define TR_CONFIRM_FORMAT              "是否格式化?"
#define TR_INTERNALRF                  "內置發射"
#define TR_INTERNAL_MODULE             TR("內置發射", "內置發射")
#define TR_EXTERNAL_MODULE             TR("外置發射", "外置發射")
#define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX需要升級"
#define TR_TELEMETRY_DISABLED          "禁用回傳"
#define TR_MORE_OPTIONS_AVAILABLE      "更多可用選項"
#define TR_NO_MODULE_INFORMATION       "無模塊信息"
#define TR_EXTERNALRF                  "外置發射"
#define TR_FAILSAFE                    TR("失控保護", "失控保護模式")
#define TR_FAILSAFESET                 "失控保護設置"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "所有者ID"
#define TR_HOLD                        "保持"
#define TR_HOLD_UPPERCASE              "保持"
#define TR_NONE                        "無效"
#define TR_NONE_UPPERCASE              "無效"
#define TR_MENUSENSOR                  "傳感器"
#define TR_POWERMETER_PEAK             "提取"
#define TR_POWERMETER_POWER            "功率"
#define TR_POWERMETER_ATTN             "衰減器"
#define TR_POWERMETER_FREQ             "頻率"
#define TR_MENUTOOLS                   "擴展工具"
#define TR_TURN_OFF_RECEIVER           "請關閉接收機"
#define TR_STOPPING                    "正在停止..."
#define TR_MENU_SPECTRUM_ANALYSER      "頻譜儀"
#define TR_MENU_POWER_METER            "功率計"
#define TR_SENSOR                      "傳感器"
#define TR_COUNTRY_CODE                "國別碼"
#define TR_USBMODE                     "USB模式"
#define TR_JACK_MODE                   "教練插口模式"
#define TR_VOICE_LANGUAGE              "播報語言"
#define TR_UNITS_SYSTEM                "單位"
#define TR_UNITS_PPM                   "PPM 單位"
#define TR_EDIT                        "編輯"
#define TR_INSERT_BEFORE               "在本條前插入"
#define TR_INSERT_AFTER                "在本條後插入"
#define TR_COPY                        "複製"
#define TR_MOVE                        "移動"
#define TR_PASTE                       "貼上"
#define TR_PASTE_AFTER                 "貼上到本條之後"
#define TR_PASTE_BEFORE                "貼上到本條之前"
#define TR_DELETE                      "刪除"
#define TR_INSERT                      "插入"
#define TR_RESET_FLIGHT                "復位飛行數據"
#define TR_RESET_TIMER1                "重啟計時器1"
#define TR_RESET_TIMER2                "重啟計時器2"
#define TR_RESET_TIMER3                "重啟計時器3"
#define TR_RESET_TELEMETRY             "重啟回傳參數"
#define TR_STATISTICS                  "統計"
#define TR_ABOUT_US                    "關於"
#define TR_USB_JOYSTICK                "USB 遊戲柄 (HID)"
#define TR_USB_MASS_STORAGE            "USB 存儲器 (SD)"
#define TR_USB_SERIAL                  "USB 端口 (VCP)"
#define TR_SETUP_SCREENS               "設置顯示頁面"
#define TR_MONITOR_SCREENS             "查看器"
#define TR_AND_SWITCH                  "與開關"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "已校準的ADC值"
#define TR_ANADIAGS_FILTRAWDEV         "經差值過濾的原始ADC值"
#define TR_ANADIAGS_UNFILTRAW          "未經過濾的原始ADC值"
#define TR_ANADIAGS_MINMAX             "最低值、最高值、行程量"
#define TR_ANADIAGS_MOVE               "移動ADC輸入以查看行程量"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("對頻", "對頻"))
#define TR_MODULE_UNBIND               BUTTON("解綁")
#define TR_POWERMETER_ATTN_NEEDED     "需要串聯衰減器"
#define TR_PXX2_SELECT_RX              "Select RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "選擇設備"
#define TR_DISCOVER                    BUTTON("發現")
#define TR_BUTTON_INIT                 BUTTON("初始化")
#define TR_WAITING                     "等待..."
#define TR_RECEIVER_DELETE             "是否刪除接收機?"
#define TR_RECEIVER_RESET              "是否重啟接收機?"
#define TR_SHARE                       "分享"
#define TR_BIND                        "對頻"
#define TR_REGISTER                    BUTTON(TR("註冊", "註冊"))
#define TR_MODULE_RANGE                BUTTON(TR("測距", "測距"))
#define TR_RANGE_TEST                  "距離測試(低功率)"
#define TR_RECEIVER_OPTIONS            TR("選項", "接收機選項")
#define TR_RESET_BTN                   BUTTON("重啟")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON("按鍵")
#define TR_ANALOGS_BTN                 BUTTON(TR("類比", "類比"))
#define TR_FS_BTN                      BUTTON(TR("自定義按鍵", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "未找到觸摸硬件"
#define TR_TOUCH_EXIT                  "點擊屏幕退出"
#define TR_SET                         BUTTON("設置")
#define TR_TRAINER                     "教練"
#define TR_CHANS                       "通道"
#define TR_ANTENNAPROBLEM               "發射機天線故障!"
#define TR_MODELIDUSED                 "ID已使用:"
#define TR_MODELIDUNIQUE               "ID is unique"
#define TR_MODULE                      "模塊"
#define TR_RX_NAME                     "接收機名稱"
#define TR_TELEMETRY_TYPE              TR("類型", "回傳類型")
#define TR_TELEMETRY_SENSORS           "傳感器"
#define TR_VALUE                       "數值"
#define TR_PERIOD                      "週期"
#define TR_INTERVAL                    "間隔"
#define TR_REPEAT                      "循環"
#define TR_ENABLE                      "啟用"
#define TR_DISABLE                     "禁用"
#define TR_TOPLCDTIMER                 "Top LCD Timer"
#define TR_UNIT                        "單位"
#define TR_TELEMETRY_NEWSENSOR         "添加新傳感器..."
#define TR_CHANNELRANGE                TR("通道範圍", "通道範圍")
#define TR_ANTENNACONFIRM1             "外置天線"
#define TR_ANTENNA_MODES_1           "內置天線"
#define TR_ANTENNA_MODES_2           "詢問"
#define TR_ANTENNA_MODES_3           "基於模型"
#define TR_ANTENNA_MODES_4           "外置天線"
#define TR_ANTENNA_MODES_5           "外置天線"
#define TR_USE_INTERNAL_ANTENNA        TR("請使用內置天線", "請使用內置天線")
#define TR_USE_EXTERNAL_ANTENNA        TR("請使用外置天線", "請使用外置天線")
#define TR_ANTENNACONFIRM2             TR("檢查天線", "請確認天線安裝完好!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "請使用FLEX版本"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "請使用FCC版本"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "請使用EU版本"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "經認證的韌體"
#define TR_LOWALARM                    "信號弱警告"
#define TR_CRITICALALARM               "信號極弱警告"
#define TR_DISABLE_ALARM               TR("禁止回傳警告", "禁止回傳警告")
#define TR_POPUP                       "彈窗提示"
#define TR_MIN                         "最小"
#define TR_MAX                         "最大"
#define TR_CURVE_PRESET                "預設..."
#define TR_PRESET                      "預設"
#define TR_MIRROR                      "鏡像"
#define TR_CLEAR                       "清除"
#define TR_CLEAR_BTN                   BUTTON("清除")
#define TR_RESET                       "重啟"
#define TR_RESET_SUBMENU               "重啟..."
#define TR_COUNT                       "點數"
#define TR_PT                          "點"
#define TR_PTS                         "點"
#define TR_SMOOTH                      "平滑"
#define TR_COPY_STICKS_TO_OFS          TR("搖桿位置存為中點", "當前搖桿位置保存為中點")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("通道行程存為輸出", "當前所有通道中點/低位/高位保存為所有輸出值")
#define TR_COPY_TRIMS_TO_OFS           TR("微調存為中點", "當前微調按鈕值保存到中點")
#define TR_INCDEC                      "增減"
#define TR_GLOBALVAR                   "全局變量"
#define TR_MIXSOURCE                   "輸入源 (%)"
#define TR_MIXSOURCERAW                "輸入源 (數值)"
#define TR_CONSTANT                    "固定值"
#define TR_PREFLIGHT_POTSLIDER_CHECK_1 "關閉"
#define TR_PREFLIGHT_POTSLIDER_CHECK_2 "開啟"
#define TR_PREFLIGHT_POTSLIDER_CHECK_3 "自動"
#define TR_PREFLIGHT                   "初始檢查"
#define TR_CHECKLIST                   TR("顯示列表", "顯示列表")
#define TR_CHECKLIST_INTERACTIVE       TR3("檢查列表", "自定義檢查列表", "自定義檢查列表")
#define TR_AUX_SERIAL_MODE             "端口"
#define TR_AUX2_SERIAL_MODE            "端口 2"
#define TR_AUX_SERIAL_PORT_POWER       "端口供電"
#define TR_SCRIPT                      "腳本"
#define TR_INPUTS                      "輸入"
#define TR_OUTPUTS                     "輸出"
#define TR_CONFIRMRESET                TR("是否清除", "是否清除所有模型和設置數據?")
#define TR_TOO_MANY_LUA_SCRIPTS        "LUA腳本數量超出限制!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP 電源"
#define TR_SPORT_UPDATE_POWER_MODES_1  "自動"
#define TR_SPORT_UPDATE_POWER_MODES_2  "開啟"
#define TR_NO_TELEMETRY_SCREENS        "無回傳頁面"
#define TR_TOUCH_PANEL                 "觸摸屏:"
#define TR_FILE_SIZE                   "文件大小"
#define TR_FILE_OPEN                   "強制打開?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "名稱"
#define TR_PHASES_HEADERS_SW           "開關"
#define TR_PHASES_HEADERS_RUD_TRIM     "方向微調"
#define TR_PHASES_HEADERS_ELE_TRIM     "俯仰微調"
#define TR_PHASES_HEADERS_THT_TRIM     "油門微調"
#define TR_PHASES_HEADERS_AIL_TRIM     "橫滾微調"
#define TR_PHASES_HEADERS_CH5_TRIM     "微調 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "微調 6"
#define TR_PHASES_HEADERS_FAD_IN       "漸入"
#define TR_PHASES_HEADERS_FAD_OUT      "漸出"

#define TR_LIMITS_HEADERS_NAME         "名稱"
#define TR_LIMITS_HEADERS_SUBTRIM      "中點"
#define TR_LIMITS_HEADERS_MIN          "低位"
#define TR_LIMITS_HEADERS_MAX          "高位"
#define TR_LIMITS_HEADERS_DIRECTION    "方向"
#define TR_LIMITS_HEADERS_CURVE        "曲線"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM中點"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "微調模式"
#define TR_INVERTED                    "反向"

// Horus layouts and widgets
#define TR_FIRST_CHANNEL             "起始通道"
#define TR_FILL_BACKGROUND           "是否填充背景?"
#define TR_BG_COLOR                  "背景顏色"
#define TR_SLIDERS_TRIMS             "滑塊和微調"
#define TR_SLIDERS                   "滑塊"
#define TR_FLIGHT_MODE               "飛行模式"
#define TR_INVALID_FILE              "無效的文件"
#define TR_TIMER_SOURCE              "計時器選擇"
#define TR_SIZE                      "尺寸"
#define TR_SHADOW                    "陰影"
#define TR_ALIGNMENT                 "對齊"
#define TR_ALIGN_LABEL               "对齐名称"
#define TR_ALIGN_VALUE               "对齐值"
#define TR_ALIGN_OPTS_1              "左"
#define TR_ALIGN_OPTS_2              "中"
#define TR_ALIGN_OPTS_3              "右"
#define TR_TEXT                      "文本"
#define TR_COLOR                     "顏色"
#define TR_PANEL1_BACKGROUND         "面板1背景"
#define TR_PANEL2_BACKGROUND         "面板2背景"
#define TR_PANEL_BACKGROUND          "Background"
#define TR_PANEL_COLOR               "  Color"
#define TR_WIDGET_GAUGE              "行程"
#define TR_WIDGET_MODELBMP           "模型信息"
#define TR_WIDGET_OUTPUTS            "輸出"
#define TR_WIDGET_TEXT               "文本"
#define TR_WIDGET_TIMER              "計時器"
#define TR_WIDGET_VALUE              "數值"

// About screen
#define TR_ABOUTUS                     TR(" 關於 ", "關於")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I' // Values between A-I will work

#define TR_BEEP_VOLUME                 "提示音音量"
#define TR_WAV_VOLUME                  "語音音量"
#define TR_BG_VOLUME                   TR("背景音量", "背景聲音量")

#define TR_TOP_BAR                     "頂欄"
#define TR_FLASH_ERASE                 "正在擦除..."
#define TR_FLASH_WRITE                 "正在寫入..."
#define TR_OTA_UPDATE                  "OTA 更新..."
#define TR_MODULE_RESET                "正在重啟模塊..."
#define TR_UNKNOWN_RX                  "未知接收機"
#define TR_UNSUPPORTED_RX              "不支持此接收機"
#define TR_OTA_UPDATE_ERROR            "OTA 更新失敗"
#define TR_DEVICE_RESET                "正在重啟設備..."
#define TR_ALTITUDE                    "高度"
#define TR_SCALE                       "比例"
#define TR_VIEW_CHANNELS               "查看通道"
#define TR_VIEW_NOTES                  "View Notes"
#define TR_MODEL_SELECT                "模型選擇"
#define TR_ID                          "ID"
#define TR_PRECISION                   "精度"
#define TR_RATIO                       "比率"
#define TR_FORMULA                     "公式"
#define TR_CELLINDEX                   "單節電池編號"
#define TR_LOGS                        "日誌"
#define TR_OPTIONS                     "選項"
#define TR_FIRMWARE_OPTIONS            BUTTON("韌體選項")

#define TR_ALTSENSOR                   "Alt 傳感器"
#define TR_CELLSENSOR                  "Cell 傳感器"
#define TR_GPSSENSOR                   "GPS 傳感器"
#define TR_CURRENTSENSOR               "傳感器"
#define TR_AUTOOFFSET                  "自動偏移值"
#define TR_ONLYPOSITIVE                "正向"
#define TR_FILTER                      "濾波器"
#define TR_TELEMETRYFULL               TR("項目已滿!", "回傳項目已滿!")
#define TR_IGNORE_INSTANCE             TR("忽略ID", "忽略ID鑑別")
#define TR_SHOW_INSTANCE_ID            "顯示實例ID"
#define TR_DISCOVER_SENSORS            "掃描新的回傳項目"
#define TR_STOP_DISCOVER_SENSORS       "停止掃描"
#define TR_DELETE_ALL_SENSORS          "刪除所有回傳項目"
#define TR_CONFIRMDELETE               "真的 " LCDW_128_LINEBREAK "要刪除全部嗎 ?"
#define TR_SELECT_WIDGET               "選擇小部件"
#define TR_WIDGET_FULLSCREEN           "全屏"
#define TR_REMOVE_WIDGET               "移除小部件"
#define TR_WIDGET_SETTINGS             "小部件設置"
#define TR_REMOVE_SCREEN               "移除頁面"
#define TR_SETUP_WIDGETS               "設置小部件"
#define TR_THEME                       "主題"
#define TR_SETUP                       "設置"
#define TR_LAYOUT                      "佈局"
#define TR_TEXT_COLOR                  "文本顏色"
#define TR_MENU_INPUTS                  CHAR_INPUT "輸入"
#define TR_MENU_LUA                     CHAR_LUA "LUA腳本"
#define TR_MENU_STICKS                  CHAR_STICK "搖桿(軸)"
#define TR_MENU_POTS                    CHAR_POT "旋鈕"
#define TR_MENU_MIN                     CHAR_FUNCTION "最小"
#define TR_MENU_MAX                     CHAR_FUNCTION "最大"
#define TR_MENU_HELI                    CHAR_CYC "斜盤混控CYC"
#define TR_MENU_TRIMS                   CHAR_TRIM "微調"
#define TR_MENU_SWITCHES                CHAR_SWITCH "開關"
#define TR_MENU_LOGICAL_SWITCHES        CHAR_SWITCH "邏輯開關"
#define TR_MENU_TRAINER                 CHAR_TRAINER "教練"
#define TR_MENU_CHANNELS                CHAR_CHANNEL "通道"
#define TR_MENU_GVARS                   CHAR_SLIDER "全局變量"
#define TR_MENU_TELEMETRY               CHAR_TELEMETRY "回傳"
#define TR_MENU_DISPLAY                "顯示"
#define TR_MENU_OTHER                  "其它"
#define TR_MENU_INVERT                 "反向"
#define TR_AUDIO_MUTE                  TR("自動靜音","音頻停播時自動靜音")
#define TR_JITTER_FILTER               "ADC濾波器"
#define TR_DEAD_ZONE                   "死區"
#define TR_RTC_CHECK                   TR("檢查時間電池", "檢查時間驅動電池電壓")
#define TR_AUTH_FAILURE                "驗證失敗"
#define TR_RACING_MODE                 "競速模式"

#define TR_USE_THEME_COLOR            "使用主題顏色"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS  "將所有微調導入中點偏移值"
#define TR_DUPLICATE                  "複製"
#define TR_ACTIVATE                   "啟用"
#define TR_RED                        "紅"
#define TR_BLUE                       "藍"
#define TR_GREEN                      "綠"
#define TR_COLOR_PICKER               "拾取顏色"
#define TR_FIXED                      "Fixed"
#define TR_EDIT_THEME_DETAILS         "主題信息"
#define TR_THEME_COLOR_DEFAULT        "默認"
#define TR_THEME_COLOR_PRIMARY1       "主色1"
#define TR_THEME_COLOR_PRIMARY2       "主色2"
#define TR_THEME_COLOR_PRIMARY3       "主色3"
#define TR_THEME_COLOR_SECONDARY1     "副色1"
#define TR_THEME_COLOR_SECONDARY2     "副色2"
#define TR_THEME_COLOR_SECONDARY3     "副色3"
#define TR_THEME_COLOR_FOCUS          "焦點"
#define TR_THEME_COLOR_EDIT           "編輯"
#define TR_THEME_COLOR_ACTIVE         "激活"
#define TR_THEME_COLOR_WARNING        "警告"
#define TR_THEME_COLOR_DISABLED       "禁用"
#define TR_THEME_COLOR_CUSTOM         "自定義"
#define TR_THEME_CHECKBOX             "復選框"
#define TR_THEME_ACTIVE               "激活"
#define TR_THEME_REGULAR              "常規"
#define TR_THEME_WARNING              "警告"
#define TR_THEME_DISABLED             "禁用"
#define TR_THEME_EDIT                 "編輯"
#define TR_THEME_FOCUS                "焦點"
#define TR_AUTHOR                     "作者"
#define TR_DESCRIPTION                "描述"
#define TR_SAVE                       "保存"
#define TR_CANCEL                     "取消"
#define TR_EDIT_THEME                 "編輯主題"
#define TR_DETAILS                    "詳細信息"

#define TR_PROTOCOL                    "協議"

// Voice in native language
#define TR_VOICE_ENGLISH              "英語"
#define TR_VOICE_CHINESE              "中文-普通話"
#define TR_VOICE_CZECH                "捷克語"
#define TR_VOICE_DANISH               "丹麥語"
#define TR_VOICE_DEUTSCH              "德語"
#define TR_VOICE_DUTCH                "荷蘭語"
#define TR_VOICE_ESPANOL              "西班牙語"
#define TR_VOICE_FINNISH              "Finnish"
#define TR_VOICE_FRANCAIS             "法語"
#define TR_VOICE_HUNGARIAN            "匈牙利語"
#define TR_VOICE_ITALIANO             "意大利語"
#define TR_VOICE_POLISH               "波蘭語"
#define TR_VOICE_PORTUGUES            "葡萄牙語"
#define TR_VOICE_RUSSIAN              "俄語"
#define TR_VOICE_SLOVAK               "斯洛伐克語"
#define TR_VOICE_SWEDISH              "瑞典語"
#define TR_VOICE_TAIWANESE            "中文-台灣國語"
#define TR_VOICE_JAPANESE             "日文"
#define TR_VOICE_HEBREW               "希伯來語"
#define TR_VOICE_UKRAINIAN            "烏克蘭語"
#define TR_VOICE_KOREAN               "韓語"

#define TR_USBJOYSTICK_LABEL           "USB 遊戲柄"
#define TR_USBJOYSTICK_EXTMODE         "模式"
#define TR_VUSBJOYSTICK_EXTMODE_1      "常規"
#define TR_VUSBJOYSTICK_EXTMODE_2      "高級"
#define TR_USBJOYSTICK_SETTINGS        BUTTON("通道設置")
#define TR_USBJOYSTICK_IF_MODE         TR("接口模式","接口模式")
#define TR_VUSBJOYSTICK_IF_MODE_1      "操縱桿"
#define TR_VUSBJOYSTICK_IF_MODE_2      "遊戲手柄"
#define TR_VUSBJOYSTICK_IF_MODE_3      "多軸搖桿"
#define TR_USBJOYSTICK_CH_MODE         "通道模式"
#define TR_VUSBJOYSTICK_CH_MODE_1      "禁用"
#define TR_VUSBJOYSTICK_CH_MODE_2      "按鈕"
#define TR_VUSBJOYSTICK_CH_MODE_3      "軸"
#define TR_VUSBJOYSTICK_CH_MODE_4      "模擬"
#define TR_VUSBJOYSTICK_CH_MODE_S_1    "-"
#define TR_VUSBJOYSTICK_CH_MODE_S_2    "B"
#define TR_VUSBJOYSTICK_CH_MODE_S_3    "A"
#define TR_VUSBJOYSTICK_CH_MODE_S_4    "S"
#define TR_USBJOYSTICK_CH_BTNMODE      "按鈕模式"
#define TR_VUSBJOYSTICK_CH_BTNMODE_1   "普通"
#define TR_VUSBJOYSTICK_CH_BTNMODE_2   "脈衝"
#define TR_VUSBJOYSTICK_CH_BTNMODE_3   "開關"
#define TR_VUSBJOYSTICK_CH_BTNMODE_4   "雙聯"
#define TR_VUSBJOYSTICK_CH_BTNMODE_5   "多聯"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_1 TR("普通","普通")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_2 TR("脈衝","脈衝")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_3 TR("開關","開關")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_4 TR("雙聯","雙聯")
#define TR_VUSBJOYSTICK_CH_BTNMODE_S_5 TR("多聯","多聯")
#define TR_USBJOYSTICK_CH_SWPOS        "位置"
#define TR_VUSBJOYSTICK_CH_SWPOS_1     "單按"
#define TR_VUSBJOYSTICK_CH_SWPOS_2     "2聯"
#define TR_VUSBJOYSTICK_CH_SWPOS_3     "3聯"
#define TR_VUSBJOYSTICK_CH_SWPOS_4     "4聯"
#define TR_VUSBJOYSTICK_CH_SWPOS_5     "5聯"
#define TR_VUSBJOYSTICK_CH_SWPOS_6     "6聯"
#define TR_VUSBJOYSTICK_CH_SWPOS_7     "7聯"
#define TR_VUSBJOYSTICK_CH_SWPOS_8     "8聯"
#define TR_USBJOYSTICK_CH_AXIS         "轴"
#define TR_VUSBJOYSTICK_CH_AXIS_1      "X"
#define TR_VUSBJOYSTICK_CH_AXIS_2      "Y"
#define TR_VUSBJOYSTICK_CH_AXIS_3      "Z"
#define TR_VUSBJOYSTICK_CH_AXIS_4      "旋轉X"
#define TR_VUSBJOYSTICK_CH_AXIS_5      "旋轉Y"
#define TR_VUSBJOYSTICK_CH_AXIS_6      "旋轉Z"
#define TR_VUSBJOYSTICK_CH_AXIS_7      "推桿"
#define TR_VUSBJOYSTICK_CH_AXIS_8      "撥輪"
#define TR_VUSBJOYSTICK_CH_AXIS_9      "方向盤"
#define TR_USBJOYSTICK_CH_SIM          "模擬軸"
#define TR_VUSBJOYSTICK_CH_SIM_1       "橫滾"
#define TR_VUSBJOYSTICK_CH_SIM_2       "俯仰"
#define TR_VUSBJOYSTICK_CH_SIM_3       "航向"
#define TR_VUSBJOYSTICK_CH_SIM_4       "油門"
#define TR_VUSBJOYSTICK_CH_SIM_5       "加速"
#define TR_VUSBJOYSTICK_CH_SIM_6       "剎車"
#define TR_VUSBJOYSTICK_CH_SIM_7       "轉盤"
#define TR_VUSBJOYSTICK_CH_SIM_8       "方向"
#define TR_USBJOYSTICK_CH_INVERSION    "反轉"
#define TR_USBJOYSTICK_CH_BTNNUM       "按鈕編號."
#define TR_USBJOYSTICK_BTN_COLLISION   "!按鈕編號衝突!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!軸衝突!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("圓口行程", "圓口行程")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_1 "無"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_2 "X-Y, Z-rX"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_3 "X-Y, rX-rY"
#define TR_VUSBJOYSTICK_CIRC_COUTOUT_4 "X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   BUTTON("應用更改")

#define TR_DIGITAL_SERVO          "數字舵機333HZ"
#define TR_ANALOG_SERVO           "模擬舵機50HZ"
#define TR_SIGNAL_OUTPUT          "信號強度輸出"
#define TR_SERIAL_BUS             "串行總線"
#define TR_SYNC                   "同步"

#define TR_ENABLED_FEATURES       "啟用的功能"
#define TR_RADIO_MENU_TABS        "系統功能選項卡"
#define TR_MODEL_MENU_TABS        "模型功能選項卡"

#define TR_SELECT_MENU_ALL        "全部"
#define TR_SELECT_MENU_CLR        "清除"
#define TR_SELECT_MENU_INV        "反向"

#define TR_SORT_ORDERS_1          "名稱 A-Z"
#define TR_SORT_ORDERS_2          "名稱 Z-A"
#define TR_SORT_ORDERS_3          "不常用"
#define TR_SORT_ORDERS_4          "常用"
#define TR_SORT_MODELS_BY         "排序"
#define TR_CREATE_NEW             "新建"

#define TR_MIX_SLOW_PREC          TR("慢放精度", "慢放精度 上行/下行")
#define TR_MIX_DELAY_PREC         TR("延時精度", "延時精度 上行/下行")

#define TR_THEME_EXISTS           "已存在同名主題目錄 !"

#define ROTORFLIGHT_LUA_CH        "按敗板半保備閉標表並波補不菜參叉差償超車持池出存打大帶待怠單到等低地點電調定動度舵二翻方放飛副復負高跟關管航號何合后護環緩回混火或機積集幾計加間減件降交角教接階截進救距開控寬饋拉來累類連練量零靈率濾螺落門面敏模目逆爬盤偏頻平其啟器前曲取確然入剎上設身升失時使式釋試數衰水瞬順速縮他態體停通退陀微尾位文穩誤息熄下陷限線相響向消小校斜新心信型行懸旋循壓頁一移儀益翼因應用油右預援源載增針正直值止置制中重軸主轉准子自蹤總最左耦"

#define TR_DATE_TIME_WIDGET       "日期和時間"
#define TR_RADIO_INFO_WIDGET      "遙控器信息"
#define TR_LOW_BATT_COLOR         "低電量"
#define TR_MID_BATT_COLOR         "中電量"
#define TR_HIGH_BATT_COLOR        "高電量"

#define TR_WIDGET_SIZE            "小部件尺寸"

#define TR_DEL_DIR_NOT_EMPTY      "刪除檔案夾必須爲空 !"

#define TR_KEY_SHORTCUTS          "Key Shortcuts"
#define TR_CURRENT_SCREEN         "Current Screen"
#define TR_SHORT_PRESS            "Short Press"
#define TR_LONG_PRESS             "Long Press"
#define TR_OPEN_QUICK_MENU        "Open Quick Menu"
