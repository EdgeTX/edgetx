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


#define TR_OFFON                        "禁用","開啟"
#define TR_MMMINV                       "---","反"
#define TR_VBEEPMODE                    "靜音","警告","忽略按鍵","全部"
#define TR_VBLMODE                      "關閉","按鍵","操控","全部","開啟"
#define TR_TRNMODE                      "關","相加","替換"
#define TR_TRNCHN                       "CH1","CH2","CH3","CH4"

#define TR_AUX_SERIAL_MODES             "禁用","回傳鏡像","回傳輸入","SBUS教練","LUA腳本","CLI","GPS","調試"
#define TR_SWTYPES                      "無","回彈","2段","3段"
#define TR_POTTYPES                     "無","有中點旋鈕","多段旋鈕","無中點旋鈕"
#define TR_SLIDERTYPES                  "無","側滑塊"
#define TR_VPERSISTENT                  "禁用","隨飛行記錄重啟","隨手動重啟"
#define TR_COUNTRY_CODES                "美國","日本","歐洲"
#define TR_USBMODES                     "詢問","遊戲柄","U盤","串行"
#define TR_JACK_MODES                   "詢問","音頻","教練"
#define TR_TELEMETRY_PROTOCOLS          "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_TELEMETRY_MODE        "Off","On","Off+Aux","On+Aux"
#define TR_MULTI_CUSTOM                "自定義"
#define TR_VTRIMINC                     "指數","很小","較小","中等","較大"
#define TR_VDISPLAYTRIMS                "不顯示","改變時","始終顯示"
#define TR_VBEEPCOUNTDOWN               "靜音","蜂鳴","語音","震動"
#define TR_COUNTDOWNVALUES              "5秒","10秒","20秒","30秒"
#define TR_VVARIOCENTER                 "音調","靜音"
#define TR_CURVE_TYPES                  "標準","自定義"

#define TR_ADCFILTERVALUES              "全局","禁用","开启"

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
#define TR_VMLTPX                      "相加","相乘","替換"
#define TR_VMLTPX2                     "+=","*=",":="
#if defined(PCBHORUS)
  #define TR_VMIXTRIMS                 "禁用","啟用","Rud","Ele","Thr","Ail","T5","T6"
#else
  #define TR_VMIXTRIMS                 "禁用","啟用","Rud","Ele","Thr","Ail","T5","T6"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER                  "定時"
  #define TR_CSWSTICKY                 "粘滯"
  #define TR_CSWRANGE                  "範圍"
  #define TR_CSWSTAY                   "邊沿"
#else
  #define TR_CSWTIMER                  "定時"
  #define TR_CSWSTICKY                 "粘滯"
  #define TR_CSWRANGE                  "範圍"
  #define TR_CSWSTAY                   "邊沿"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#if defined(VARIO)
  #define TR_VVARIO                    "Vario傳感器"
#else
  #define TR_VVARIO                    "[Vario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND                     "播放聲音"
#else
  #define TR_SOUND                     "蜂鳴"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC                    "振動"
#else
  #define TR_HAPTIC                    "[振動]"
#endif

#if defined(VOICE)
    #define TR_PLAY_TRACK              "播放音頻文件"
  #define TR_PLAY_BOTH                 "同時播放"
  #define TR_PLAY_VALUE                "播放數值"
#else
  #define TR_PLAY_TRACK                "[Play Trk]"
  #define TR_PLAY_BOTH                 "[Play Bth]"
  #define TR_PLAY_VALUE                "[Play Val]"
#endif

#define TR_SF_BG_MUSIC                 "播放背景音樂","暫停背景音樂"

#if defined(SDCARD)
  #define TR_SDCLOGS                   "記錄日誌到SD卡"
#else
  #define TR_SDCLOGS                   "[記錄日誌到SD卡]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR               "修改全局變量GV值"
#else
  #define TR_ADJUST_GVAR               "[修改GV值]"
#endif

#if defined(LUA)
  #define TR_SF_PLAY_SCRIPT            "Lua腳本"
#else
  #define TR_SF_PLAY_SCRIPT            "[Lua]"
#endif

#if defined(DEBUG)
  #define TR_SF_TEST                   ,"測試"
#else
  #define TR_SF_TEST
#endif

#if defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "鎖定通道值"
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "鎖定通道"
#else
  #define TR_SF_SAFETY                 "---"
#endif

#define TR_SF_SCREENSHOT               "截屏"
#define TR_SF_RACING_MODE              "競速模式"
#define TR_SF_DISABLE_TOUCH            "禁用觸摸"
#define TR_SF_RESERVE                  "[保留]"

#define TR_VFSWFUNC                    TR_SF_SAFETY,"教練","搖桿值存儲到微調","重啟","設置",TR_ADJUST_GVAR,"音量","設置失控保護","測距模式","模塊對頻",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH TR_SF_TEST

#define TR_FSW_RESET_TELEM             "回傳參數"

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS          "計時器1","計時器2","計時器3"
#else
  #define TR_FSW_RESET_TIMERS          "計時1","計時2","計時3"
#endif

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("全部","飛行"),TR_FSW_RESET_TELEM

#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "公制",TR("英制","英制")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "禁用","數值","條形圖","腳本"
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
#elif defined(RADIO_T8)
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
  #define TR_ROTARY_ENC_OPT          "Normal","Inverted"
#else
  #define TR_ROTARY_ENC_OPT          "Normal","Inverted","V-I H-N","V-I H-A"
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
#define TR_VTRAINER_MASTER_OFF         "禁用"
#define TR_VTRAINER_MASTER_JACK        "教練主機/教練口"
#define TR_VTRAINER_SLAVE_JACK         "學生從機/教練口"
#define TR_VTRAINER_MASTER_SBUS_MODULE "教練主機/SBUS模塊"
#define TR_VTRAINER_MASTER_CPPM_MODULE "教練從機/CPPM模塊"
#define TR_VTRAINER_MASTER_BATTERY     "教練主機/端口"
#define TR_VTRAINER_BLUETOOTH          "教練主機/藍牙","教練從機/藍牙"
#define TR_VTRAINER_MULTI              "教練主機/多協議"
#define TR_VFAILSAFE                   "未設置","失控保持","自定義","無脈衝", "接收機"
#define TR_VSENSORTYPES                "自定義","運算"
#define TR_VFORMULAS                   "加","平均值","最小值","最大值","乘","總計值","單節電池","消耗量","距離"
#define TR_VPREC                       "0.--","0.0-","0.00"
#define TR_VCELLINDEX                  "最低值","1","2","3","4","5","6","最高值","差值"
#define TR_TEXT_SIZE                   "標準","小","較小","中等","雙倍"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA "(中点)","=(整體)"
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
#elif !defined(HARDWARE_KEYS)
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

#define TR_FREE                        "空閒"
#define TR_YES                         "是"
#define TR_NO                          "否"
#define TR_DELETEMODEL                 "刪除" BREAKSPACE "模型"
#define TR_COPYINGMODEL                "複製模型..."
#define TR_MOVINGMODEL                 "移動模型..."
#define TR_LOADINGMODEL                "載入模型..."
#define TR_NAME                        "名稱"
#define TR_MODELNAME                   "模型名稱"
#define TR_PHASENAME                   "模式名稱"
#define TR_MIXNAME                     "混控名稱"
#define TR_INPUTNAME                   TR("名稱", "輸入名稱")
#define TR_EXPONAME                    TR("名稱", "曲線名稱")
#define TR_BITMAP                      "模型圖片"
#define TR_TIMER                       TR("計時器", "計時器 ")
#define TR_START                       "開始"
#define TR_ELIMITS                     TR("E.Limits", "擴展通道行程")
#define TR_ETRIMS                      TR("E.Trims", "擴展微調量")
#define TR_TRIMINC                     "微調步幅"
#define TR_DISPLAY_TRIMS               TR("微調顯示", "微調顯示")
#define TR_TTRACE                      TR("輸入源", INDENT "輸入源")
#define TR_TTRIM                       TR("油門微調僅怠速", INDENT "油門微調只調整怠速")
#define TR_TTRIM_SW                    TR("微調選擇", INDENT "微調選擇")
#define TR_BEEPCTR                     TR("中點蜂鳴", "中點蜂鳴")
#define TR_USE_GLOBAL_FUNCS            TR("全局功能", "全局功能可介入")
#define TR_PROTO                       TR(INDENT "協議", INDENT "協議")
#define TR_PPMFRAME INDENT             "PPM幀"
#define TR_REFRESHRATE                 TR(INDENT "刷新率", INDENT "刷新速率")
#define STR_WARN_BATTVOLTAGE           TR(INDENT "輸出為電池電壓: ", INDENT "注意輸出電平為電池電壓")
#define TR_WARN_5VOLTS                 "注意輸出電平是5V"
#define TR_MS                          "ms"
#define TR_FREQUENCY                   INDENT "頻率"
#define TR_SWITCH                      "開關"
#define TR_TRIMS                       "微調"
#define TR_FADEIN                      "漸入"
#define TR_FADEOUT                     "漸出"
#define TR_DEFAULT                     "(默認)"
#define TR_CHECKTRIMS                  "\006Check\012trims"
#define OFS_CHECKTRIMS                 CENTER_OFS+(9*FW)
#define TR_SWASHTYPE                   "斜盤類型"
#define TR_COLLECTIVE                  TR("螺距源", "螺距混控源")
#define TR_AILERON                     TR("橫滾源", "橫滾混控源")
#define TR_ELEVATOR                    TR("俯仰源", "俯仰混控源")
#define TR_SWASHRING                   "斜盤行程"
#define TR_ELEDIRECTION                "俯仰方向"
#define TR_AILDIRECTION                "橫滾方向"
#define TR_COLDIRECTION                "螺距方向"
#define TR_MODE                        "模式"
#define TR_SUBTYPE INDENT              "子類型"
#define TR_NOFREEEXPO                  "指數曲線已滿!"
#define TR_NOFREEMIXER                 "混控數量已滿!"
#define TR_SOURCE                      "輸入源"
#define TR_WEIGHT                      "比例"
#define TR_EXPO                        TR("指數", "指數")
#define TR_SIDE                        "單邊"
#define TR_DIFFERENTIAL                "差動"
#define TR_OFFSET                      "偏移量"
#define TR_TRIM                        "使用微調"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
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
#define TR_MIXES                       "混控設置"
#define TR_CV                          "曲線"
#if defined(PCBNV14)
  #define TR_GV                        "GV"
#else
  #define TR_GV                        TR("G", "GV")
#endif
#define TR_ACHANNEL                    "A\004通道"
#define TR_RANGE INDENT                "範圍"
#define TR_CENTER INDENT               "中點"
#define TR_BAR                         "條形"
#define TR_ALARM                       "報警"
#define TR_USRDATA                     TR("用戶數據", "用戶數據")
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
#define TR_MODEL_STILL_POWERED         "模型未斷電"
#define TR_USB_STILL_CONNECTED         "USB未斷開"
#define TR_MODEL_SHUTDOWN              "關機 ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "按ENTER鍵確認"
#define TR_THROTTLE_LABEL              "油門"
#define TR_THROTTLEREVERSE             TR("油門反向", INDENT "油門反向")
#define TR_MINUTEBEEP                  TR("分", "分鐘播報")
#define TR_BEEPCOUNTDOWN               INDENT "倒數"
#define TR_PERSISTENT                  TR(INDENT "關機保持", INDENT "關機保持")
#define TR_BACKLIGHT_LABEL             "背光"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "狀態"
#define TR_BLDELAY                     INDENT "保持時間"
#define TR_BLONBRIGHTNESS              "背光開"
#define TR_BLOFFBRIGHTNESS             "背光關"
#define TR_KEYS_BACKLIGHT              "按鍵背光"
#define TR_BLCOLOR                     "顏色"
#define TR_SPLASHSCREEN                "開機圖片"
#define TR_PWR_ON_DELAY                "開機延遲"
#define TR_PWR_OFF_DELAY               "關機延遲"
#define TR_THROTTLE_WARNING            TR(INDENT "油門狀態", INDENT "油門狀態")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "自定位置", INDENT INDENT INDENT INDENT "自定油門位置?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("位置 %", "油門位置 %")
#define TR_SWITCHWARNING               TR(INDENT "開關位置", INDENT "開關位置")
#define TR_POTWARNINGSTATE             TR(INDENT "旋鈕滑塊", INDENT "旋鈕和滑塊")
#define TR_SLIDERWARNING               TR(INDENT "滑塊位置", INDENT "滑塊位置")
#define TR_POTWARNING                  TR(INDENT "旋鈕位置", INDENT "旋鈕位置")
#define TR_TIMEZONE                    "時區"
#define TR_ADJUST_RTC                  "時鐘設置"
#define TR_GPS                         "GPS"
#define TR_RXCHANNELORD                TR("通道順序", "默認通道順序")
#define TR_STICKS                      "搖桿"
#define TR_POTS                        "電位器"
#define TR_SWITCHES                    "開關"
#define TR_SWITCHES_DELAY              TR("開關經過延時", "延時播放(開關經過)")
#define TR_SLAVE CENTER                "從機"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "倍率"
#define TR_CAL                         "校準"
#define TR_CALIBRATION                 "校準"
#define TR_VTRIM                       "微調-+"
#define TR_BG                          "BG:"
#define TR_CALIB_DONE                  "校準完成"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "按 [Enter] 鍵開始"
  #define TR_SETMIDPOINT               "將搖桿、旋鈕、滑塊居中後按[Enter]開始"
  #define TR_MOVESTICKSPOTS            "轉動搖桿、旋鈕、滑塊到最大邊界後按[Enter]結束"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               "按 [Enter] 鍵開始"
  #define TR_SETMIDPOINT               "將搖桿、旋鈕、滑塊居中後按[Enter]開始"
  #define TR_MOVESTICKSPOTS            "轉動搖桿、旋鈕、滑塊到最大邊界後按[Enter]結束"
#else
  #define TR_MENUTOSTART               CENTER "\010按[Enter]键開始"
  #define TR_SETMIDPOINT               TR(CENTER "\004搖杆、旋鈕和滑塊居中", CENTER "\004摇杆、旋钮和滑块居中")
  #define TR_MOVESTICKSPOTS            CENTER "\006搖杆、旋钮和滑塊最大邊界"
  #define TR_MENUWHENDONE              CENTER "\006按[Enter]键完成"
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
#define TR_MODEL                       "模型"
#define TR_FM                          "FM"
#define TR_MIX                         "MIX"
#define TR_EEPROMLOWMEM                "EEPROM low mem"
#define TR_PRESS_ANY_KEY_TO_SKIP       "按任意鍵跳過"
#define TR_THROTTLE_NOT_IDLE           "請將油門拉至最低!"
#define TR_ALARMSDISABLED              "已禁用報警"
#define TR_PRESSANYKEY                 TR("\010按任意鍵", "按任意鍵")
#define TR_BADEEPROMDATA               "存儲數據錯誤"
#define TR_BAD_RADIO_DATA              "系統數據錯誤"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            "格式化存儲"
#define TR_STORAGE_FORMAT              "存儲準備"
#define TR_EEPROMOVERFLOW              "存儲超限"
#define TR_RADIO_SETUP                 "系統設置"
#define TR_MENUDATEANDTIME             "日期和時間"
#define TR_MENUTRAINER                 "教練"
#define TR_MENUSPECIALFUNCS            "全局功能"
#define TR_MENUVERSION                 "版本"
#define TR_MENU_RADIO_SWITCHES         TR("SWITCHES", "開關及按鍵測試")
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "類比輸入測試")
#define TR_MENU_RADIO_ANALOGS_CALIB     "已校準的類比值"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "原始類比值 (5 Hz)"
#define TR_MENUCALIBRATION             "校準"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "將當前微調量保存至中點"
#else
  #define TR_TRIMS2OFFSETS             "\006微調存至中點"
#endif
#define TR_CHANNELS2FAILSAFE           "導入當前所有通道值"
#define TR_CHANNEL2FAILSAFE            "導入當前通道值"
#define TR_MENUMODELSEL                TR("模型選擇", "模型選擇")
#define TR_MENU_MODEL_SETUP            TR("模型設置", "模型設置")
#define TR_MENUFLIGHTMODES             "飛行模式設置"
#define TR_MENUFLIGHTMODE              "飛行模式"
#define TR_MENUHELISETUP               "直升機設置"

#define TR_MENUINPUTS                  "輸入設置"
#define TR_MENULIMITS                  "輸出設置"
#define TR_MENUCURVES                  "曲線設置"
#define TR_MENUCURVE                   "曲線"
#define TR_MENULOGICALSWITCH           "邏輯開關"
#define TR_MENULOGICALSWITCHES         "邏輯開關設置"
#define TR_MENUCUSTOMFUNC              "特殊功能設置"
#define TR_MENUCUSTOMSCRIPTS           "自定義腳本設置"
#define TR_MENUTELEMETRY               "回傳"
#define TR_MENUTEMPLATES               "模板"
#define TR_MENUSTAT                    "統計"
#define TR_MENUDEBUG                   "調試"
#define TR_MONITOR_CHANNELS1           "1-8通道查看"
#define TR_MONITOR_CHANNELS2           "9-16通道查看"
#define TR_MONITOR_CHANNELS3           "17-24通道查看"
#define TR_MONITOR_CHANNELS4           "25-32通道查看"
#define TR_MONITOR_SWITCHES            "邏輯開關查看"
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
#define TR_MULTI_AUTOBIND              TR(INDENT "對頻通道", INDENT "通道控制對頻")
#define TR_DISABLE_CH_MAP              TR("禁用通道映射", "禁用通道重映射")
#define TR_DISABLE_TELEM               TR("禁用回傳", "禁用回傳")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "自動格式", INDENT "自動識別格式")
#define TR_MULTI_LOWPOWER              TR(INDENT "低功率", INDENT "低功率模式")
#define TR_MULTI_LNA_DISABLE           INDENT "禁用接收放大器"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")
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
#define TR_MODULE_STATUS               TR(INDENT "狀態", INDENT "多協議狀態")
#define TR_MODULE_SYNC                 TR(INDENT "同步", INDENT "協議同步狀態")
#define TR_MULTI_SERVOFREQ             TR("舵機頻率", "舵機更新頻率")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("頻道", "選擇發射頻道")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_SYNCMENU                    "[Sync]"
#define TR_LIMIT                       INDENT "限制"
#define TR_MINRSSI                     "Min Rssi"
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "使用 FlySky RSSI 值無需重縮放")
#define TR_LATITUDE                    "緯度"
#define TR_LONGITUDE                   "經度"
#define TR_GPS_COORDS_FORMAT           TR("方向角", "方向角格式")
#define TR_VARIO                       TR("變化率", "高度變化率")
#define TR_PITCH_AT_ZERO               "基準音調"
#define TR_PITCH_AT_MAX                "最高音調"
#define TR_REPEAT_AT_ZERO              "無變化報警"
#define TR_SHUTDOWN                    "關機"
#define TR_SAVE_MODEL                  "正在存儲模型"
#define TR_BATT_CALIB                  TR("電池校準", "電池電壓校準")
#define TR_CURRENT_CALIB               "電流校準"
#define TR_VOLTAGE                     TR(INDENT "電壓源", INDENT "電壓來源")
#define TR_CURRENT                     TR(INDENT "電流源", INDENT "電流來源")
#define TR_SELECT_MODEL                "選擇模型"
#define TR_MODEL_CATEGORIES            "模型分類"
#define TR_MODELS                      "個模型"
#define TR_SELECT_MODE                 "選擇模式"
#define TR_CREATE_CATEGORY             "創建分類"
#define TR_RENAME_CATEGORY             "分類改名"
#define TR_DELETE_CATEGORY             "刪除分類"
#define TR_CREATE_MODEL                "創建模型"
#define TR_DUPLICATE_MODEL             "複製模型"
#define TR_COPY_MODEL                  "複製模型"
#define TR_MOVE_MODEL                  "移動模型"
#define TR_BACKUP_MODEL                "備份模型"
#define TR_DELETE_MODEL                "刪除模型"
#define TR_RESTORE_MODEL               "恢復模型"
#define TR_DELETE_ERROR                "刪除失敗"
#define TR_CAT_NOT_EMPTY               "分類未清空"
#define TR_SDCARD_ERROR                TR("存儲卡錯誤", "存儲卡錯誤")
#define TR_SDCARD                      "存儲卡"
#define TR_NO_FILES_ON_SD              "存儲卡中沒有文件!"
#define TR_NO_SDCARD                   "存儲卡未安裝"
#define TR_WAITING_FOR_RX              "等待接收機響應..."
#define TR_WAITING_FOR_TX              "等待發射機響應..."
#define TR_WAITING_FOR_MODULE          TR("Waiting module", "Waiting for module...")
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
#define TR_MODULES_RX_VERSION          "模塊/接收機版本"
#define TR_MENU_MODULES_RX_VERSION     "模塊/接收機版本"
#define TR_MENU_FIRM_OPTIONS           "韌體選項"
#define TR_IMU                         "陀螺儀"
#define TR_STICKS_POTS_SLIDERS         "搖桿/旋鈕/滑塊"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM 搖桿/旋鈕/滑塊"
#define TR_RF_PROTOCOL                 "發射協議"
#define TR_MODULE_OPTIONS              "選項"
#define TR_POWER                       "功率"
#define TR_NO_TX_OPTIONS               "無發射機選項"
#define TR_RTC_BATT                    "時間驅動電池電壓"
#define TR_POWER_METER_EXT             "功率計 (外置)"
#define TR_POWER_METER_INT             "功率計 (內置)"
#define TR_SPECTRUM_ANALYSER_EXT       "頻譜儀 (外置)"
#define TR_SPECTRUM_ANALYSER_INT       "頻譜儀 (內置)"
#define TR_SDCARD_FULL                 "存儲卡已滿"
#define TR_NEEDS_FILE                  "需要文件名包含"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "不兼容"
#define TR_WARNING                     "警告"
#define TR_EEPROMWARN                  "EEPROM"
#define TR_STORAGE_WARNING             "存儲"
#define TR_EEPROM_CONVERTING           "轉換數據"
#define TR_SDCARD_CONVERSION_REQUIRE   "需要轉換SD卡"
#define TR_CONVERTING                  "轉換: "
#define TR_THROTTLE_UPPERCASE          "油門"
#define TR_ALARMSWARN                  "警告"
#define TR_SWITCHWARN                  TR("開關", "開關位置")
#define TR_FAILSAFEWARN                "失控保護"
#define TR_TEST_WARNING                TR("測試版", "測試版本")
#define TR_TEST_NOTSAFE                "只用於測試"
#define TR_WRONG_SDCARDVERSION         TR("需要版本: ", "請將SD卡文件更換為正確版本: ")
#define TR_WARN_RTC_BATTERY_LOW        "時間驅動電池電壓低"
#define TR_WARN_MULTI_LOWPOWER         "低功率模式"
#define TR_BATTERY                     "電池"
#define TR_WRONG_PCBREV                "錯誤的硬件類型"
#define TR_EMERGENCY_MODE              "緊急模式"
#define TR_PCBREV_ERROR                "錯誤的主板類型"
#define TR_NO_FAILSAFE                 "失控保護未設置"
#define TR_KEYSTUCK                    "檢測到有按鍵卡住"
#define TR_INVERT_THR                  TR("油門反向", "是否反相油門?")
#define TR_VOLUME                      "音量"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "亮度"
#define TR_CPU_TEMP                    "CPU 溫度"
#define TR_CPU_CURRENT                 "電流"
#define TR_CPU_MAH                     "消耗量"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "主板溫度"
#define TR_CAPAWARNING                 INDENT "電流過低"
#define TR_TEMPWARNING                 INDENT "過熱"
#define TR_TTL_WARNING                 "注意串口使用逻辑电平3.3V"
#define TR_FUNC                        "功能"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "持續時間"
#define TR_DELAY                       "延時"
#define TR_SD_CARD                     "SD卡"
#define TR_SDHC_CARD                   "SD-HC卡"
#define TR_NO_SOUNDS_ON_SD             "存儲卡中無音頻"
#define TR_NO_MODELS_ON_SD             "存儲卡中無模型"
#define TR_NO_BITMAPS_ON_SD            "存儲卡中無圖片"
#define TR_NO_SCRIPTS_ON_SD            "存儲卡中無腳本"
#define TR_SCRIPT_SYNTAX_ERROR         "腳本程序語法錯誤"
#define TR_SCRIPT_PANIC                "Script panic"
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
#define TR_SD_FORMAT                   "格式"
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "硬件"
#define TR_FORMATTING                  "正在格式化..."
#define TR_TEMP_CALIB                  "溫度校準"
#define TR_TIME                        "時間"
#define TR_MAXBAUDRATE                 "最大帶寬"
#define TR_BAUDRATE                    "波特率"
#define TR_SAMPLE_MODE                 "採樣模式"
#define TR_SAMPLE_MODES                "Normal","OneBit"

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
#if defined(PCBX9E)
  #define TR_BLUETOOTH_MODES           "---","啟用"
#else
  #define TR_BLUETOOTH_MODES           "---","回傳","教練"
#endif          

#define TR_SD_INFO_TITLE               "存儲卡詳情"
#define TR_SD_TYPE                     "類型:"
#define TR_SD_SPEED                    "速度:"
#define TR_SD_SECTORS                  "扇區:"
#define TR_SD_SIZE                     "容量:"
#define TR_TYPE                        INDENT "類型"
#define TR_GLOBAL_VARS                 "全局變量"
#define TR_GVARS                       "全局變量"
#define TR_GLOBAL_VAR                  "全局變量"
#define TR_MENU_GLOBAL_VARS            "全局變量功能"
#define TR_OWN                         "Own"
#define TR_DATE                        "日期"
#define TR_MONTHS                      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec " }
#define TR_ROTARY_ENCODER              "滾輪"
#define TR_ROTARY_ENC_MODE              TR("RotEnc Mode","Rotary Encoder Mode")
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
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash BT module", "刷新藍牙模塊")
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
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX需要升級"
#define TR_TELEMETRY_DISABLED          "禁用回傳"
#define TR_MORE_OPTIONS_AVAILABLE      "更多可用選項"
#define TR_NO_MODULE_INFORMATION       "無模塊信息"
#define TR_EXTERNALRF                  "外置發射"
#define TR_FAILSAFE                    TR(INDENT "失控保護", INDENT "失控保護模式")
#define TR_FAILSAFESET                 "失控保護設置"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "所有者ID"
#define TR_PINMAPSET                   "PINMAP"
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
#define TR_EDIT                        "編輯"
#define TR_INSERT_BEFORE               "在本條前插入"
#define TR_INSERT_AFTER                "在本條後插入"
#define TR_COPY                        "複製"
#define TR_MOVE                        "移動"
#define TR_PASTE                       "貼上"
#define TR_PASTE_AFTER                 "貼上到之前"
#define TR_PASTE_BEFORE                "貼上到之後"
#define TR_DELETE                      "刪除"
#define TR_INSERT                      "插入"
#define TR_RESET_FLIGHT                "重啟飛行記錄"
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
#define TR_ANADIAGS_CALIB              "已校準的類比量"
#define TR_ANADIAGS_FILTRAWDEV         "經差值過濾的原始類比量"
#define TR_ANADIAGS_UNFILTRAW          "未經過濾的原始類比量"
#define TR_ANADIAGS_MINMAX             "最低值、最高值、行程量"
#define TR_ANADIAGS_MOVE               "移動類比量全行程以查看結果"
#define TR_SPEAKER INDENT              "揚聲器"
#define TR_BUZZER INDENT               "蜂鳴器"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("對頻", "對頻"))
#define TR_POWERMETER_ATTN_NEEDED      "需要串聯衰減器"
#define TR_PXX2_SELECT_RX              "Select RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "選擇設備"
#define TR_DISCOVER                    "發現"
#define TR_BUTTON_INIT                 BUTTON("初始化")
#define TR_WAITING                     "等待..."
#define TR_RECEIVER_DELETE             "是否刪除接收機?"
#define TR_RECEIVER_RESET              "是否重啟接收機?"
#define TR_SHARE                       "分享"
#define TR_BIND                        "對頻"
#define TR_REGISTER                    TR("註冊", "註冊")
#define TR_MODULE_RANGE                BUTTON(TR("測距", "測距"))
#define TR_RECEIVER_OPTIONS            TR("選項", "接收機選項")
#define TR_DEL_BUTTON                  BUTTON(TR("刪除", "刪除"))
#define TR_RESET_BTN                   BUTTON("重啟")
#define TR_DEBUG                       "調試"
#define TR_KEYS_BTN                    BUTTON("按鍵")
#define TR_ANALOGS_BTN                 BUTTON(TR("類比", "類比"))
#define TR_TOUCH_NOTFOUND              "未找到觸摸硬件"
#define TR_TOUCH_EXIT                  "點擊屏幕退出"
#define TR_SET                         BUTTON("設置")
#define TR_TRAINER                     "教練"
#define TR_CHANS                       "通道"
#define TR_ANTENNAPROBLEM               CENTER "發射機天線故障!"
#if defined(COLORLCD)
  #define TR_MODELIDUSED               "ID已使用:"
#else
  #define TR_MODELIDUSED               TR("ID已使用:","ID已使用:")
#endif
#define TR_MODULE                      "模塊"
#define TR_RX_NAME                     "接收機名稱"
#define TR_TELEMETRY_TYPE              TR("類型", "回傳類型")
#define TR_TELEMETRY_SENSORS           "傳感器"
#define TR_VALUE                       "數值"
#define TR_REPEAT                      "循環"
#define TR_ENABLE                      "啟用"
#define TR_TOPLCDTIMER                 "Top LCD Timer"
#define TR_UNIT                        "單位"
#define TR_TELEMETRY_NEWSENSOR         INDENT "添加新傳感器..."
#define TR_CHANNELRANGE                TR(INDENT "通道範圍", INDENT "通道範圍")
#define TR_RXFREQUENCY                 TR("接收機頻率", "接收機端口輸出頻率")
#define TR_ANTENNACONFIRM1             "外置天線"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "內置天線","詢問","基於模型","外置天線"
#else
  #define TR_ANTENNA_MODES             "內置天線","詢問","基於模型","外置天線"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("請使用內置天線", "請使用內置天線")
#define TR_USE_EXTERNAL_ANTENNA        TR("請使用外置天線", "請使用外置天線")
#define TR_ANTENNACONFIRM2             TR("檢查天線", "請確認天線安裝完好!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "請使用FLEX版本"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "請使用FCC版本"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "請使用EU版本"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "經認證的韌體"
#define TR_LOWALARM                    INDENT "信號弱警告"
#define TR_CRITICALALARM               INDENT "信號極弱警告"
#define TR_RSSIALARM_WARN              "RSSI"
#define TR_NO_RSSIALARM                TR(INDENT "回傳警告已禁止", "回傳警告已禁止")
#define TR_DISABLE_ALARM               TR(INDENT "禁止回傳警告", INDENT "禁止回傳警告")
#define TR_ENABLE_POPUP                "允許彈窗"
#define TR_DISABLE_POPUP               "禁止彈窗"
#define TR_POPUP                       "彈窗提示"
#define TR_MIN                         "最小"
#define TR_MAX                         "最大"
#define TR_CURVE_PRESET                "預設..."
#define TR_PRESET                      "預設"
#define TR_MIRROR                      "鏡像"
#define TR_CLEAR                       "清除"
#define TR_RESET                       "重啟"
#define TR_RESET_SUBMENU               "重啟..."
#define TR_COUNT                       "點數"
#define TR_PT                          "點"
#define TR_PTS                         "點"
#define TR_SMOOTH                      "平滑"
#define TR_COPY_STICKS_TO_OFS          TR("搖杆位置存為中點", "當前搖杆位置保存為中點")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("通道行程存為輸出", "當前所有通道中點/低位/高位保存為所有輸出值")
#define TR_COPY_TRIMS_TO_OFS           TR("微調存為中點", "當前微調按鈕值保存到中點")
#define TR_INCDEC                      "增減"
#define TR_GLOBALVAR                   "全局變量"
#define TR_MIXSOURCE                   "混控源"
#define TR_CONSTANT                    "不變化"
#define TR_PERSISTENT_MAH              TR(INDENT "關機保持mAh", INDENT "關機保持 mAh")
#define TR_PREFLIGHT                   "初始位置檢查"
#define TR_CHECKLIST                   TR(INDENT "顯示列表", INDENT "顯示列表")
#define TR_FAS_OFFSET                  TR(INDENT "FAS偏移", INDENT "FAS 偏移量")
#define TR_AUX_SERIAL_MODE             "端口"
#define TR_AUX2_SERIAL_MODE            "端口 2"
#define TR_AUX_SERIAL_PORT_POWER       "端口供電"
#define TR_SCRIPT                      "腳本"
#define TR_INPUTS                      "輸入"
#define TR_OUTPUTS                     "輸出"
#define STR_EEBACKUP                   "EEPROM 數據備份"
#define STR_FACTORYRESET               "恢復出廠設置"
#define TR_CONFIRMRESET                TR("是否清除", "是否清除所有模型和設置數據?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Lua腳本數量超出限制!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP 電源"
#define TR_SPORT_UPDATE_POWER_MODES    "自動","開啟"
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

#define TR_LSW_HEADERS_FUNCTION        "功能"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "與開關"
#define TR_LSW_HEADERS_DURATION        "持續時間"
#define TR_LSW_HEADERS_DELAY           "延時"

#define TR_GVAR_HEADERS_NAME           "名稱"
#define TR_GVAR_HEADERS_FM0            "FM0值"
#define TR_GVAR_HEADERS_FM1            "FM1值"
#define TR_GVAR_HEADERS_FM2            "FM2值"
#define TR_GVAR_HEADERS_FM3            "FM3值"
#define TR_GVAR_HEADERS_FM4            "FM4值"
#define TR_GVAR_HEADERS_FM5            "FM5值"
#define TR_GVAR_HEADERS_FM6            "FM6值"
#define TR_GVAR_HEADERS_FM7            "FM7值"
#define TR_GVAR_HEADERS_FM8            "FM8值"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS            { "比較類型或功能", "第一個變量", "第二個變量或常量", "第二個變量或常量", "啟用本行的附加條件", "邏輯開關持續時間", "邏輯開關延時開啟" }

// Horus layouts and widgets
#define TR_FIRST_CHANNEL               "起始通道"
#define TR_FILL_BACKGROUND             "是否填充背景?"
#define TR_BG_COLOR                    "背景顏色"
#define TR_SLIDERS_TRIMS               "滑塊和微調"
#define TR_SLIDERS                     "滑塊"
#define TR_FLIGHT_MODE                 "飛行模式"
#define TR_INVALID_FILE                "無效的文件"
#define TR_TIMER_SOURCE                "計時器選擇"
#define TR_SIZE                        "尺寸"
#define TR_SHADOW                      "陰影"
#define TR_TEXT                        "文本"
#define TR_COLOR                       "顏色"
#define TR_MAIN_VIEW_X                 "Main view X"
#define TR_PANEL1_BACKGROUND           "面板1背景"
#define TR_PANEL2_BACKGROUND           "面板2背景"

// About screen
#define TR_ABOUTUS                     TR(" 關於 ", "關於")

#define TR_CHR_SHORT                   's'
#define TR_CHR_LONG                    'l'
#define TR_CHR_TOGGLE                  't'
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
#define TR_ALTITUDE INDENT             "高度"
#define TR_SCALE                       "比例"
#define TR_VIEW_CHANNELS               "查看通道"
#define TR_VIEW_NOTES                  "View Notes"
#define TR_MODEL_SELECT                "模型選擇"
#define TR_MODS_FORBIDDEN              "禁止修改!"
#define TR_UNLOCKED                    "已解鎖"
#define TR_ID                          "ID"
#define TR_PRECISION                   "精度"
#define TR_RATIO                       "比率"
#define TR_FORMULA                     "公式"
#define TR_CELLINDEX                   "單節電池編號"
#define TR_LOGS                        "日誌"
#define TR_OPTIONS                     "選項"
#define TR_FIRMWARE_OPTIONS            "韌體選項"

#define TR_ALTSENSOR                   "Alt 傳感器"
#define TR_CELLSENSOR                  "Cell 傳感器"
#define TR_GPSSENSOR                   "GPS 傳感器"
#define TR_CURRENTSENSOR               "傳感器"
#define TR_AUTOOFFSET                  "自動偏移值"
#define TR_ONLYPOSITIVE                "正向"
#define TR_FILTER                      "濾波"
#define TR_TELEMETRYFULL               TR("項目已滿!", "回傳項目已滿!")
#define TR_SERVOS_OK                   "Servos OK"
#define TR_SERVOS_KO                   "Servos KO"
#define TR_INVERTED_SERIAL             INDENT "反向"
#define TR_IGNORE_INSTANCE             TR(INDENT "忽略ID", INDENT "忽略ID鑑別")
#define TR_DISCOVER_SENSORS            "掃描新的回傳項目"
#define TR_STOP_DISCOVER_SENSORS       "停止掃描"
#define TR_DELETE_ALL_SENSORS          "刪除所有回傳項目"
#define TR_CONFIRMDELETE               "真的 " LCDW_128_480_LINEBREAK "要刪除全部嗎 ?"
#define TR_SELECT_WIDGET               "選擇小部件"
#define TR_REMOVE_WIDGET               "移除小部件"
#define TR_WIDGET_SETTINGS             "小部件設置"
#define TR_REMOVE_SCREEN               "移除頁面"
#define TR_SETUP_WIDGETS               "設置小部件"
#define TR_USER_INTERFACE              "用戶界面"
#define TR_THEME                       "主題"
#define TR_SETUP                       "設置"
#define TR_LAYOUT                      "佈局"
#define TR_ADD_MAIN_VIEW               "添加主視圖"
#define TR_BACKGROUND_COLOR            "背景顏色"
#define TR_MAIN_COLOR                  "主顏色"
#define TR_BAR2_COLOR                  "二級條形圖顏色"
#define TR_BAR1_COLOR                  "主條形圖顏色"
#define TR_TEXT_COLOR                  "文本顏色"
#define TR_TEXT_VIEWER                 "文本視圖"
#define TR_MENU_INPUTS                  STR_CHAR_INPUT "輸入"
#define TR_MENU_LUA                     STR_CHAR_LUA "LUA腳本"
#define TR_MENU_STICKS                  STR_CHAR_STICK "搖桿"
#define TR_MENU_POTS                    STR_CHAR_POT "旋鈕"
#define TR_MENU_MAX                     STR_CHAR_FUNCTION "固定值MAX"
#define TR_MENU_HELI                    STR_CHAR_CYC "斜盤混控CYC"
#define TR_MENU_TRIMS                   STR_CHAR_TRIM "微調"
#define TR_MENU_SWITCHES                STR_CHAR_SWITCH "開關"
#define TR_MENU_LOGICAL_SWITCHES        STR_CHAR_SWITCH "邏輯開關"
#define TR_MENU_TRAINER                 STR_CHAR_TRAINER "教練"
#define TR_MENU_CHANNELS                STR_CHAR_CHANNEL "通道"
#define TR_MENU_GVARS                   STR_CHAR_SLIDER "全局變量"
#define TR_MENU_TELEMETRY               STR_CHAR_TELEMETRY "回傳"
#define TR_MENU_DISPLAY                "顯示"
#define TR_MENU_OTHER                  "其它"
#define TR_MENU_INVERT                 "反向"
#define TR_JITTER_FILTER               "類比輸入濾波"
#define TR_DEAD_ZONE                   "死區"
#define TR_RTC_CHECK                   TR("檢查時間電池", "檢查時間驅動電池電壓")
#define TR_AUTH_FAILURE                "驗證失敗"
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

#define TR_USE_THEME_COLOR            "使用主題顏色"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS  "將所有微調導入中點偏移值"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS    "打開通道監視器" 
#else
  #define TR_OPEN_CHANNEL_MONITORS    "通道監視" 
#endif
#define TR_DUPLICATE                  "複製"
#define TR_ACTIVATE                   "啟用"
#define TR_RED                        "紅"
#define TR_BLUE                       "藍"
#define TR_GREEN                      "綠"
#define TR_COLOR_PICKER               "拾取顏色"
#define TR_EDIT_THEME_DETAILS         "主題信息"
#define TR_AUTHOR                     "作者"
#define TR_DESCRIPTION                "描述"
#define TR_SAVE                       "保存"
#define TR_CANCEL                     "取消"
#define TR_EDIT_THEME                 "編輯主題"
#define TR_DETAILS                    "詳細信息"
#define TR_THEME_EDITOR               "主題"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL     "模型選擇"
#define TR_MAIN_MENU_MODEL_NOTES      "模型說明"
#define TR_MAIN_MENU_CHANNEL_MONITOR  "通道查看"
#define TR_MAIN_MENU_MODEL_SETTINGS   "模型設置"
#define TR_MAIN_MENU_RADIO_SETTINGS   "系統設置"
#define TR_MAIN_MENU_SCREEN_SETTINGS  "顯示設置"
#define TR_MAIN_MENU_RESET_TELEMETRY  "復位功能"
#define TR_MAIN_MENU_STATISTICS       "統計信息"
#define TR_MAIN_MENU_ABOUT_EDGETX     "關於"
// End Main menu

#define TR_PROTOCOL                    "協議"
#define TR_YEAR                        "年"
#define TR_MONTH                       "月"
#define TR_DAY                         "日"
