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


#define TR_OFFON                       "禁用","开启"
#define TR_MMMINV                      "---","反\0"
#define TR_VBEEPMODE                   "静音","警告","忽略按键","全部"
#define TR_VBLMODE                     "关闭","按键","操控","全部","开启"
#define TR_TRNMODE                     "关","相加","替换"
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES            "调试","回传镜像","回传输入","SBUS教练","LUA脚本","CLI","GPS","Debug"
#define TR_SWTYPES                     "无","回弹","2段","3段"
#define TR_POTTYPES                    "无","有中点旋钮","多段旋钮","无中点旋钮"
#define TR_SLIDERTYPES                 "无","侧滑块"
#define TR_VPERSISTENT                 "禁用","随飞行记录复位","随手动复位"
#define TR_COUNTRY_CODES               "美国","日本","欧洲"
#define TR_USBMODES                    "询问","游戏柄","U盘","串行"
#define TR_JACK_MODES                  "询问","音频","教练"
#define TR_TELEMETRY_PROTOCOLS         "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"
#define TR_VTRIMINC                    "指数","很小","较小","中等","较大"
#define TR_VDISPLAYTRIMS               "不显示","改变时","始终显示"
#define TR_VBEEPCOUNTDOWN              "静音","蜂鸣","语音","震动"
#define TR_COUNTDOWNVALUES             "5秒","10秒","20秒","30秒"
#define TR_VVARIOCENTER                "音调","静音"
#define TR_CURVE_TYPES                 "标准","自定义"
#define TR_ADCFILTERVALUES             "全局","禁用","开启"

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
#define TR_VMLTPX                      "相加","相乘","替换"
#define TR_VMLTPX2                     "+=","*=",":="

#if defined(PCBHORUS)
  #define TR_VMIXTRIMS                 "禁用","启用","Rud","Ele","Thr","Ail","T5","T6"
#else
  #define TR_VMIXTRIMS                 "禁用","启用","Rud","Ele","Thr","Ail"
#endif

#define TR_CSWTIMER                  "定时"
#define TR_CSWSTICKY                 "粘滞"
#define TR_CSWRANGE                  "范围"
#define TR_CSWSTAY                   "边沿"

#define TR_CSWEQUAL                  "a=x"
#define TR_VCSWFUNC                  "---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#if defined(VARIO)
  #define TR_VVARIO                    "Vario传感器"
#else
  #define TR_VVARIO                    "[Vario]"
#endif
#if defined(AUDIO)
  #define TR_SOUND                     "播放声音"
#endif
#if defined(HAPTIC)
  #define TR_HAPTIC                    "振动"
#else
  #define TR_HAPTIC                    "[振动]"
#endif

#if defined(VOICE)
  #define TR_PLAY_TRACK                "播放音频文件"
  #define TR_PLAY_BOTH                 "同时播放"
  #define TR_PLAY_VALUE                "播放数值"
#else
  #define TR_PLAY_TRACK                "[Play Trk]"
  #define TR_PLAY_BOTH                 "[Play Bth]"
  #define TR_PLAY_VALUE                "[Play Val]"
#endif

#define TR_SF_BG_MUSIC                 "播放背景音乐" "暂停背景音乐"

#if defined(SDCARD)
  #define TR_SDCLOGS                   "记录到SD卡"
#else
  #define TR_SDCLOGS                   "[记录到SD卡]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR               "修改全局变量GV值"
#else
  #define TR_ADJUST_GVAR               "[修改GV值]"
#endif

#if defined(LUA)
  #define TR_SF_PLAY_SCRIPT            "Lua脚本"
#else
  #define TR_SF_PLAY_SCRIPT            "[Lua]"
#endif

#if defined(DEBUG)
  #define TR_SF_TEST                   ,"测试"
#else
  #define TR_SF_TEST
#endif

#if defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "锁定通道值"
#else
  #define TR_SF_SAFETY                 "---"
#endif

#define TR_SF_SCREENSHOT               "截屏"
#define TR_SF_DISABLE_TOUCH            "禁用触摸"
#define TR_SF_RACING_MODE              "竞速模式"
#define TR_SF_RESERVE                  "[保留]"

#define TR_VFSWFUNC                    TR_SF_SAFETY,"教练","摇杆值存储到微调","复位","设置",TR_ADJUST_GVAR,"音量","设置失控保护","测距模式","模块对频",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,"背光",TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH TR_SF_TEST

#define TR_FSW_RESET_TELEM             "回传参数"
#define TR_FSW_RESET_TIMERS            "计时器1","计时器2","计时器3"
#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,"Flight",TR_FSW_RESET_TELEM
#define TR_FUNCSOUNDS                  "Beep1","Beep2","Beep3","Warn1","Warn2","Cheep","Ratata","Tick","Siren","Ring","SciFi","Robot","Chirp","Tada","Crickt","AlmClk"

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "公制","英制"
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "禁用","数值","条形图","脚本"
#define TR_GPSFORMAT                   "DMS","NMEA"

#define TR_TEMPLATE_CLEAR_MIXES 	"Clear Mixes"
#define TR_TEMPLATE_SIMPLE_4CH 		"Simple 4-CH"
#define TR_TEMPLATE_STICKY_TCUT 	"Sticky-T-Cut"
#define TR_TEMPLATE_VTAIL 		"V-Tail"
#define TR_TEMPLATE_DELTA 		"Elevon\\Delta"
#define TR_TEMPLATE_ECCPM 		"eCCPM"
#define TR_TEMPLATE_HELI 		"Heli Setup"
#define TR_TEMPLATE_SERVO_TEST 		"Servo Test"

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

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ON","One"

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
#define TR_VTRAINER_MASTER_JACK        "教练主机/教练口"
#define TR_VTRAINER_SLAVE_JACK         "学生从机/教练口"
#define TR_VTRAINER_MASTER_SBUS_MODULE "教练主机/SBUS模块"
#define TR_VTRAINER_MASTER_CPPM_MODULE "教练从机/CPPM模块"
#define TR_VTRAINER_MASTER_BATTERY     "教练主机/串口"
#define TR_VTRAINER_BLUETOOTH          "教练主机/蓝牙","教练从机/蓝牙"
#define TR_VTRAINER_MULTI              "教练主机/多协议"
#define TR_VFAILSAFE                   "未设置","失控保持","自定义","无脉冲","接收机"
#define TR_VSENSORTYPES                "自定义","运算"
#define TR_VFORMULAS                   "加","平均值","最小值","最大值","乘","总计值","单节电池","消耗量","距离"
#define TR_VPREC                       "0.--","0.0-","0.00"
#define TR_VCELLINDEX                  "最低值","1","2","3","4","5","6","最高值","差值"
#define TR_TEXT_SIZE                   "标准","小","较小","中等","双倍"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA "(中点)", "=(整体)"

#define INDENT
#define LEN_INDENT                   3
#define INDENT_WIDTH                 12
#define BREAKSPACE                   "\036"

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14)
  #define TR_ENTER                     "[NEXT]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#define TR_EXIT                      	"RTN"
#define TR_OK                        	TR_ENTER

#define TR_POPUPS_ENTER_EXIT         	TR_ENTER "\010" TR_EXIT

#define TR_FREE                        "空闲"
#define TR_YES                         "是"
#define TR_NO                          "否"
#define TR_DELETEMODEL                 "删除" BREAKSPACE "模型"
#define TR_COPYINGMODEL                "复制模型..."
#define TR_MOVINGMODEL                 "移动模型..."
#define TR_LOADINGMODEL                "载入模型..."
#define TR_NAME                        "名称"
#define TR_MODELNAME                   "模型名称"
#define TR_PHASENAME                   "模式名称"
#define TR_MIXNAME                     "混控名称"
#define TR_INPUTNAME                   TR("名称", "输入名称")
#define TR_EXPONAME                    TR("名称", "曲线名称")
#define TR_BITMAP                      "模型图片"
#define TR_TIMER                       TR("计时器", "计时器 ")
#define TR_START                       "开始"
#define TR_ELIMITS                     TR("E.Limits", "扩展通道行程")
#define TR_ETRIMS                      TR("E.Trims", "扩展微调量")
#define TR_TRIMINC                     "微调步幅"
#define TR_DISPLAY_TRIMS               TR("Show Trims", "微调显示")
#define TR_TTRACE                      TR("T-Source", INDENT "输入源")
#define TR_TTRIM                       TR("T-Trim-Idle", INDENT "油门微调只调整怠速")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", INDENT "微调选择")
#define TR_BEEPCTR                     TR("Ctr Beep", "中点蜂鸣")
#define TR_USE_GLOBAL_FUNCS            TR("Glob.Funcs", "全局功能可介入")
#define TR_PROTO                       TR(INDENT "Proto", INDENT "协议")
#define TR_PPMFRAME                    INDENT "PPM帧"
#define TR_REFRESHRATE                 TR(INDENT "Refresh", INDENT "刷新速率")
#define STR_WARN_BATTVOLTAGE           TR(INDENT "Output is VBAT: ", INDENT "注意输出电平为电池电压")
#define TR_WARN_5VOLTS                 "注意输出电平为5V"
#define TR_MS                          "ms"
#define TR_FREQUENCY                   INDENT "频率"
#define TR_SWITCH                      "开关"
#define TR_TRIMS                       "微调"
#define TR_FADEIN                      "渐入"
#define TR_FADEOUT                     "渐出"
#define TR_DEFAULT                     "(默认)"
#define TR_CHECKTRIMS                  "\006Check\012trims"
#define OFS_CHECKTRIMS                 CENTER_OFS+(9*FW)
#define TR_SWASHTYPE                   "斜盘类型"
#define TR_COLLECTIVE                  TR("Collective", "螺距混控源")
#define TR_AILERON                     TR("Lateral cyc.", "横滚混控源")
#define TR_ELEVATOR                    TR("Long. cyc.", "俯仰混控源")
#define TR_SWASHRING                   "斜盘行程"
#define TR_ELEDIRECTION                "俯仰方向"
#define TR_AILDIRECTION                "横滚方向"
#define TR_COLDIRECTION                "螺距方向"
#define TR_MODE                        "模式"
#define TR_SUBTYPE                     INDENT "子类型"
#define TR_NOFREEEXPO                  "指数曲线已满!"
#define TR_NOFREEMIXER                 "混控数量已满!"
#define TR_SOURCE                       "输入源"
#define TR_WEIGHT                      "比例"
#define TR_EXPO                        TR("Expo", "指数")
#define TR_SIDE                        "单边"
#define TR_DIFFERENTIAL                "差动"
#define TR_OFFSET                      "偏移量"
#define TR_TRIM                        "使用微调"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "曲线"
#define TR_FLMODE                      TR("Mode", "飞行模式")
#define TR_MIXWARNING                  "警告"
#define TR_OFF                         "禁用"
#define TR_ANTENNA                     "天线"
#define TR_NO_INFORMATION              TR("No info", "无信息")
#define TR_MULTPX                      "叠加方式"
#define TR_DELAYDOWN                   TR("Delay dn", "下行延时")
#define TR_DELAYUP                     "上行延时"
#define TR_SLOWDOWN                    TR("Slow dn", "下行慢动作")
#define TR_SLOWUP                      "上行慢动作"
#define TR_MIXES                       "混控设置"
#define TR_CV                          "曲线"
#define TR_GV                          "GV"
#define TR_ACHANNEL                    "A\004通道"
#define TR_RANGE                       INDENT "范围"
#define TR_CENTER                      INDENT "中点"
#define TR_BAR                         "条形"
#define TR_ALARM                       "报警"
#define TR_USRDATA                     TR("UsrData", "用户数据")
#define TR_BLADES                      "Blades/Poles"
#define TR_SCREEN                      "屏幕\001"
#define TR_SOUND_LABEL                 "声音"
#define TR_LENGTH                      "音长"
#define TR_BEEP_LENGTH                 "提示音长度"
#define TR_BEEP_PITCH                  "提示音音调"
#define TR_HAPTIC_LABEL                "振动"
#define TR_STRENGTH                    "强度"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "补偿"
#define TR_IMU_MAX                     "最大值"
#define TR_CONTRAST                    "对比度"
#define TR_ALARMS_LABEL                "警告"
#define TR_BATTERY_RANGE               TR("Batt. range", "电压测量范围")
#define TR_BATTERYCHARGING             "充电中..."
#define TR_BATTERYFULL                 "电量已满"
#define TR_BATTERYNONE                 "空!"
#define TR_BATTERYWARNING              "低电压报警值"
#define TR_INACTIVITYALARM             "长时间无操作"
#define TR_MEMORYWARNING               "内存不足"
#define TR_ALARMWARNING                "静音"
#define TR_RSSI_SHUTDOWN_ALARM         TR("Rssi Shutdown", "关机时检查RSSI")
#define TR_MODEL_STILL_POWERED         "模型未断电"
#define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_MODEL_SHUTDOWN              "关机 ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "按ENTER键确认"
#define TR_THROTTLE_LABEL              "油门"
#define TR_THROTTLEREVERSE             TR("T-Reverse", INDENT "油门反向")
#define TR_MINUTEBEEP                  TR("分", "分钟播报")
#define TR_BEEPCOUNTDOWN               INDENT "倒数"
#define TR_PERSISTENT                  TR(INDENT "Persist.", INDENT "关机保持")
#define TR_BACKLIGHT_LABEL             "背光"
#define TR_BLDELAY                     INDENT "Duration"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLONBRIGHTNESS              "背光开"
#define TR_BLOFFBRIGHTNESS             "背光关"
#define TR_KEYS_BACKLIGHT              "按键背光"
#define TR_BLCOLOR                     "颜色"
#define TR_SPLASHSCREEN                "开机图片"
#define TR_PWR_ON_DELAY                "开机延迟"
#define TR_PWR_OFF_DELAY               "关机延迟"
#define TR_THROTTLE_WARNING            TR(INDENT "T-Warning", INDENT "油门状态")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Cust-Pos", INDENT INDENT INDENT INDENT "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR(INDENT "S-Warning", INDENT "开关位置")
#define TR_POTWARNINGSTATE             TR(INDENT "Pot&Slid.", INDENT "旋钮和滑块")
#define TR_SLIDERWARNING               TR(INDENT "Slid. pos.", INDENT "滑块位置")
#define TR_POTWARNING                  TR(INDENT "Pot warn.", INDENT "旋钮位置")
#define TR_TIMEZONE                    "时区"
#define TR_ADJUST_RTC                  "时钟设置"
#define TR_GPS                         "GPS"
#define TR_RXCHANNELORD                TR("Rx channel ord", "默认通道顺序")
#define TR_STICKS                      "摇杆"
#define TR_POTS                        "电位器"
#define TR_SWITCHES                    "开关"
#define TR_SWITCHES_DELAY              TR("Play delay", "延时播放(开关经过)")
#define TR_SLAVE                       CENTER "从机"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "倍率"
#define TR_CAL                         "校准"
#define TR_CALIBRATION                 "校准"
#define TR_VTRIM                       "微调-+"
#define TR_BG                          "BG:"
#define TR_CALIB_DONE                  "校准完成"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "按 [Enter] 键开始"
  #define TR_SETMIDPOINT               "将摇杆/旋钮/滑块居中后按[Enter]开始"
  #define TR_MOVESTICKSPOTS            "转动摇杆/旋钮/滑块到最大边界后按[Enter]结束"
#else
  #define TR_MENUTOSTART               TR_ENTER " 开始"
  #define TR_SETMIDPOINT               "将摇杆、旋钮和滑块居中"
  #define TR_MOVESTICKSPOTS            "转动摇杆、旋钮和滑块到最大边界"
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
#define TR_PRESS_ANY_KEY_TO_SKIP       "按任意键跳过"
#define TR_THROTTLE_NOT_IDLE           "请将油门拉至最低!"
#define TR_ALARMSDISABLED              "已禁用报警"
#define TR_PRESSANYKEY                 TR("\010Press any Key", "按任意键")
#define TR_BADEEPROMDATA               "存储数据错误"
#define TR_BAD_RADIO_DATA              "系统数据错误"
#define TR_EEPROMFORMATTING            "格式化存储"
#define TR_STORAGE_FORMAT              "存储准备"
#define TR_EEPROMOVERFLOW              "存储超限"
#define TR_RADIO_SETUP                 "系统设置"
#define TR_MENUDATEANDTIME             "日期和时间"
#define TR_MENUTRAINER                 "教练"
#define TR_MENUSPECIALFUNCS            "全局功能"
#define TR_MENUVERSION                 "版本"
#define TR_MENU_RADIO_SWITCHES         TR("SWITCHES", "开关及按键测试")
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "类比输入测试")
#define TR_MENU_RADIO_ANALOGS_CALIB     "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION             "校准"
#define TR_TRIMS2OFFSETS               "将当前微调量保存至中点"
#define TR_CHANNELS2FAILSAFE           "导入当前所有通道值"
#define TR_CHANNEL2FAILSAFE            "导入当前通道值"
#define TR_MENUMODELSEL                TR("MODELSEL", "模型选择")
#define TR_MENU_MODEL_SETUP            TR("SETUP", "模型设置")
#define TR_MENUFLIGHTMODES             "飞行模式设置"
#define TR_MENUFLIGHTMODE              "飞行模式"
#define TR_MENUHELISETUP               "直升机设置"

#define TR_MENUINPUTS                  "输入设置"
#define TR_MENULIMITS                  "输出设置"
#define TR_MENUCURVES                  "曲线设置"
#define TR_MENUCURVE                   "曲线"
#define TR_MENULOGICALSWITCH           "逻辑开关"
#define TR_MENULOGICALSWITCHES         "逻辑开关设置"
#define TR_MENUCUSTOMFUNC              "特殊功能设置"
#define TR_MENUCUSTOMSCRIPTS           "自定义脚本设置"
#define TR_MENUTELEMETRY               "回传"
#define TR_MENUTEMPLATES               "模板"
#define TR_MENUSTAT                    "统计"
#define TR_MENUDEBUG                   "调试"
#define TR_MONITOR_CHANNELS1           "1-8通道查看"
#define TR_MONITOR_CHANNELS2           "9-16通道查看"
#define TR_MONITOR_CHANNELS3           "17-24通道查看"
#define TR_MONITOR_CHANNELS4           "25-32通道查看"
#define TR_MONITOR_SWITCHES            "逻辑开关查看"
#define TR_MONITOR_OUTPUT_DESC         "输出查看"
#define TR_MONITOR_MIXER_DESC          "混控查看"
#define TR_RECEIVER_NUM                TR("RxNum", "接收机编号")
#define TR_RECEIVER                    "接收机ID"
#define TR_MULTI_CUSTOM                "自定义"
#define TR_MULTI_RFTUNE                TR("Freq tune", "频率微调")
#define TR_MULTI_RFPOWER               "发射功率"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY             "回传"
#define TR_MULTI_VIDFREQ               TR("Vid. freq.", "Video frequency")
#define TR_RF_POWER                    "发射功率"
#define TR_MULTI_FIXEDID               TR("FixedID", "固定ID")
#define TR_MULTI_OPTION                TR("Option", "选项值")
#define TR_MULTI_AUTOBIND              TR(INDENT "Bind Ch.", INDENT "通道控制对频")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "禁用通道重映射")
#define TR_DISABLE_TELEM               TR("No Telem", "禁用回传")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "Autodetect", INDENT "自动识别格式")
#define TR_MULTI_LOWPOWER              TR(INDENT "Low power", INDENT "低功率模式")
#define TR_MULTI_LNA_DISABLE           INDENT "禁用接收放大器"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "S.Port link")
#define TR_MODULE_TELEM_ON             TR("ON", "启用")
#define TR_DISABLE_INTERNAL            TR("Disable int.", "禁用内置射频")
#define TR_MODULE_NO_SERIAL_MODE       TR("!serial mode", "未开启串口模式")
#define TR_MODULE_NO_INPUT             TR("No input", "无串行数据输入")
#define TR_MODULE_NO_TELEMETRY         TR3("No telemetry", "No MULTI_TELEMETRY", "未收到多协议回传信息")
#define TR_MODULE_WAITFORBIND          "使用当前协议对频"
#define TR_MODULE_BINDING              TR("Bind...","对频中")
#define TR_MODULE_UPGRADE_ALERT        TR3("Upg. needed", "Module upgrade required", "多协议模块需要升级")
#define TR_MODULE_UPGRADE              TR("Upg. advised", "多协议模块需要升级")
#define TR_REBIND                      "需要重新对频"
#define TR_REG_OK                      "注册成功"
#define TR_BIND_OK                     "对频成功"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 开启回传"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 关闭回传"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 开启回传"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 关闭回传"
#define TR_PROTOCOL                    "Protocol"
#define TR_PROTOCOL_INVALID            TR("Prot. invalid", "无效协议")
#define TR_MODULE_STATUS               TR(INDENT "Status", INDENT "多协议状态")
#define TR_MODULE_SYNC                 TR(INDENT "Sync", INDENT "协议同步状态")
#define TR_MULTI_SERVOFREQ             TR("Servo rate", "伺服更新频率")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("RF Channel", "选择射频频道")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_SYNCMENU                    "[Sync]"
#define TR_LIMIT                       INDENT "限制"
#define TR_MINRSSI                     "Min Rssi"
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "使用 FlySky RSSI 值无需重缩放")
#define TR_LATITUDE                    "纬度"
#define TR_LONGITUDE                   "经度"
#define TR_GPS_COORDS_FORMAT           TR("GPS Coords", "方向角格式")
#define TR_VARIO                       TR("Vario", "高度变化率")
#define TR_PITCH_AT_ZERO               "基准音调"
#define TR_PITCH_AT_MAX                "最高音调"
#define TR_REPEAT_AT_ZERO              "无变化报警"
#define TR_SHUTDOWN                    "关机"
#define TR_SAVE_MODEL                  "正在存储模型"
#define TR_BATT_CALIB                  TR("Batt. calib", "电池电压校准")
#define TR_CURRENT_CALIB               "Current calib"
#define TR_VOLTAGE                     TR(INDENT "Voltage", INDENT "电压来源")
#define TR_CURRENT                     TR(INDENT "Current", INDENT "电流来源")
#define TR_SELECT_MODEL                "选择模型"
#define TR_MODEL_CATEGORIES            "模型分类"
#define TR_MODELS                      "个模型"
#define TR_SELECT_MODE                 "选择模式"
#define TR_CREATE_CATEGORY             "创建分类"
#define TR_RENAME_CATEGORY             "分类改名"
#define TR_DELETE_CATEGORY             "删除分类"
#define TR_CREATE_MODEL                "创建模型"
#define TR_DUPLICATE_MODEL             "复制模型"
#define TR_COPY_MODEL                  "复制模型"
#define TR_MOVE_MODEL                  "移动模型"
#define TR_BACKUP_MODEL                "备份模型"
#define TR_DELETE_MODEL                "删除模型"
#define TR_RESTORE_MODEL               "恢复模型"
#define TR_DELETE_ERROR                "删除失败"
#define TR_CAT_NOT_EMPTY               "分类未清空"
#define TR_SDCARD_ERROR                TR("SD error", "存储卡错误")
#define TR_SDCARD                      "存储卡"
#define TR_NO_FILES_ON_SD              "存储卡中没有文件!"
#define TR_NO_SDCARD                   "存储卡未安装"
#define TR_WAITING_FOR_RX              "等待接收机响应..."
#define TR_WAITING_FOR_TX              "等待发射机响应..."
#define TR_WAITING_FOR_MODULE          "Waiting for module..."
#define TR_NO_TOOLS                    "无可用附加功能"
#define TR_NORMAL                      "正常"
#define TR_NOT_INVERTED                "正向"
#define TR_NOT_CONNECTED               "未连接"
#define TR_CONNECTED                   "已连接"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH without telem.", "16CH 禁用回传")
#define TR_16CH_WITH_TELEMETRY         TR("16CH with telem.", "16CH 开启回传")
#define TR_8CH_WITH_TELEMETRY          TR("8CH with telem.", "8CH 开启回传")
#define TR_EXT_ANTENNA                 "外置天线"
#define TR_PIN                         "插针"
#define TR_UPDATE_RX_OPTIONS           "是否升级接收机?"
#define TR_UPDATE_TX_OPTIONS           "是否升级发射机?"
#define TR_MODULES_RX_VERSION          "模块/接收机版本"
#define TR_MENU_MODULES_RX_VERSION     "模块/接收机版本"
#define TR_MENU_FIRM_OPTIONS           "固件选项"
#define TR_IMU                        "陀螺仪"
#define TR_STICKS_POTS_SLIDERS         "摇杆/旋钮/滑块"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM 摇杆/旋钮/滑块"
#define TR_RF_PROTOCOL                 "射频协议"
#define TR_MODULE_OPTIONS              "选项"
#define TR_POWER                       "功率"
#define TR_NO_TX_OPTIONS               "无发射机选项"
#define TR_RTC_BATT                    "时间驱动电池电压"
#define TR_POWER_METER_EXT             "功率计 (外置)"
#define TR_POWER_METER_INT             "功率计 (内置)"
#define TR_SPECTRUM_ANALYSER_EXT       "频谱仪 (外置)"
#define TR_SPECTRUM_ANALYSER_INT       "频谱仪 (内置)"
#define TR_SDCARD_FULL                 "存储卡已满"
#define TR_NEEDS_FILE                  "需要文件名包含"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "不兼容"
#define TR_WARNING                     "警告"
#define TR_EEPROMWARN                  "EEPROM"
#define TR_STORAGE_WARNING             "存储"
#define TR_EEPROM_CONVERTING           "转换数据"
#define TR_SDCARD_CONVERSION_REQUIRE   "需要转换SD卡"
#define TR_CONVERTING                  "转换:"
#define TR_THROTTLE_UPPERCASE          "油门"
#define TR_ALARMSWARN                  "警告"
#define TR_SWITCHWARN                  TR("开关", "开关位置")
#define TR_FAILSAFEWARN                "失控保护"
#define TR_TEST_WARNING                TR("TESTING", "测试版本")
#define TR_TEST_NOTSAFE                "只用于测试"
#define TR_WRONG_SDCARDVERSION         TR("Expected ver: ", "请将SD卡文件更换为正确版本: ")
#define TR_WARN_RTC_BATTERY_LOW        "时间驱动电池电压低"
#define TR_WARN_MULTI_LOWPOWER         "低功率模式"
#define TR_BATTERY                     "电池"
#define TR_WRONG_PCBREV                "错误的硬件类型"
#define TR_EMERGENCY_MODE              "紧急模式"
#define TR_PCBREV_ERROR                "错误的主板类型"
#define TR_NO_FAILSAFE                 "失控保护未设置"
#define TR_KEYSTUCK                    "检测到有按键卡住"
#define TR_INVERT_THR                  TR("Invert thr?", "是否反相油门?")
#define TR_VOLUME                      "音量"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "亮度"
#define TR_CPU_TEMP                    "CPU 温度"
#define TR_CPU_CURRENT                 "电流"
#define TR_CPU_MAH                     "消耗量"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "主板温度"
#define TR_CAPAWARNING                 INDENT "电流过低"
#define TR_TEMPWARNING                 INDENT "过热"
#define TR_TTL_WARNING                 "注意串口使用逻辑电平3.3V"
#define TR_FUNC                        "功能"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "持续时间"
#define TR_DELAY                       "延时"
#define TR_SD_CARD                     "SD卡"
#define TR_SDHC_CARD                   "SD-HC卡"
#define TR_NO_SOUNDS_ON_SD             "存储卡中无音频"
#define TR_NO_MODELS_ON_SD             "存储卡中无模型"
#define TR_NO_BITMAPS_ON_SD            "存储卡中无图片"
#define TR_NO_SCRIPTS_ON_SD            "存储卡中无脚本"
#define TR_SCRIPT_SYNTAX_ERROR         "脚本程序语法错误"
#define TR_SCRIPT_PANIC                "Script panic"
#define TR_SCRIPT_KILLED               "清除脚本进程"
#define TR_SCRIPT_ERROR                "未知错误"
#define TR_PLAY_FILE                   "播放"
#define TR_DELETE_FILE                 "删除"
#define TR_COPY_FILE                   "复制"
#define TR_RENAME_FILE                 "重命名"
#define TR_ASSIGN_BITMAP               "设置为模型图片"
#define TR_ASSIGN_SPLASH               "设置为开机图片"
#define TR_EXECUTE_FILE                "执行"
#define TR_REMOVED                     "已删除"
#define TR_SD_INFO                     "信息"
#define TR_SD_FORMAT                   "格式"
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "硬件"
#define TR_FORMATTING                  "正在格式化..."
#define TR_TEMP_CALIB                  "温度校准"
#define TR_TIME                        "时间"
#define TR_MAXBAUDRATE                 "最大带宽"
#define TR_BAUDRATE                    "波特率"
#define TR_SAMPLE_MODE                 "采样模式"
#define TR_SAMPLE_MODES                "Normal","OneBit"
#define TR_BLUETOOTH                   "蓝牙"
#define TR_BLUETOOTH_DISC              "发现"
#define TR_BLUETOOTH_INIT              "初始化"
#define TR_BLUETOOTH_DIST_ADDR         "目标地址"
#define TR_BLUETOOTH_LOCAL_ADDR        "本机地址"
#define TR_BLUETOOTH_PIN_CODE          "PIN码"
#define TR_BLUETOOTH_BAUDRATE          "蓝牙带宽"
#define TR_BLUETOOTH_MODES             "---","回传","教练"
#define TR_SELECT_TEMPLATE_FOLDER      "选择一个模板文件夹:"
#define TR_SELECT_TEMPLATE             "选择一个模板:"
#define TR_NO_TEMPLATES                "在此文件夹中未找到模型模板"
#define TR_SAVE_TEMPLATE               "存储为模板"
#define TR_BLANK_MODEL                 "空白模型"
#define TR_BLANK_MODEL_INFO            "创建一个空白模型"
#define TR_FILE_EXISTS                 "文件已存在"
#define TR_ASK_OVERWRITE               "是否覆盖?"

#define TR_SD_INFO_TITLE               "存储卡详情"
#define TR_SD_TYPE                     "类型:"
#define TR_SD_SPEED                    "速度:"
#define TR_SD_SECTORS                  "扇区:"
#define TR_SD_SIZE                     "容量:"
#define TR_TYPE                        INDENT "类型"
#define TR_GLOBAL_VARS                 "全局变量"
#define TR_GVARS                       "全局变量"
#define TR_GLOBAL_VAR                  "全局变量"
#define TR_MENU_GLOBAL_VARS            "全局变量功能"
#define TR_OWN                         "Own"
#define TR_DATE                        "日期"
#define TR_MONTHS                      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
#define TR_YEAR                        "年"
#define TR_MONTH                       "月"
#define TR_DAY                         "日"
#define TR_ROTARY_ENCODER              "滚轮"
#define TR_INVERT_ROTARY               "滚轮反向"
#define TR_CHANNELS_MONITOR            "通道查看器"
#define TR_MIXERS_MONITOR              "混控查看器"
#define TR_PATH_TOO_LONG               "路径太长"
#define TR_VIEW_TEXT                   "查看文本"
#define TR_FLASH_BOOTLOADER            "更新引导程序"
#define TR_FLASH_DEVICE                TR("Flash device","刷新设备")      
#define TR_FLASH_EXTERNAL_DEVICE       TR("Flash S.Port", "从 S.Port 端口更新外设固件")
#define TR_FLASH_RECEIVER_OTA          "Flash receiver OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash BT module", "刷新蓝牙模块")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Flash pwr mngt unit"
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "设备无响应")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "设备文件错误")
#define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "设备数据被拒绝")
#define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "无法访问设备")
#define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "设备文件被拒绝")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", "错误的文件签名")
#define TR_CURRENT_VERSION             "当前版本: "
#define TR_FLASH_INTERNAL_MODULE       "更新内置模块固件"
#define TR_FLASH_INTERNAL_MULTI        "更新内置多协议模块固件"
#define TR_FLASH_EXTERNAL_MODULE       "更新外置模块固件"
#define TR_FLASH_EXTERNAL_MULTI        "更新外置多协议模块固件"
#define TR_FLASH_EXTERNAL_ELRS         "更新外置ELRS模块固件"
#define TR_FIRMWARE_UPDATE_ERROR       "固件更新失败"
#define TR_FIRMWARE_UPDATE_SUCCESS     "固件更新成功"
#define TR_WRITING                     "正在写入..."
#define TR_CONFIRM_FORMAT              "是否格式化?"
#define TR_INTERNALRF                  "内置射频"
#define TR_INTERNAL_MODULE             TR("Int. module", "内置射频")
#define TR_EXTERNAL_MODULE             TR("Ext. module", "外置射频")
#define TR_OPENTX_UPGRADE_REQUIRED     "OpenTX需要升级"
#define TR_TELEMETRY_DISABLED          "禁用回传"
#define TR_MORE_OPTIONS_AVAILABLE      "更多可用选项"
#define TR_NO_MODULE_INFORMATION       "无模块信息"
#define TR_EXTERNALRF                  "外置射频"
#define TR_FAILSAFE                    TR(INDENT "Failsafe", INDENT "失控保护模式")
#define TR_FAILSAFESET                 "失控保护设置"
#define TR_REG_ID                      "Reg. ID"
#define TR_OWNER_ID                    "Owner ID"
#define TR_PINMAPSET                   "PINMAP"
#define TR_HOLD                        "保持"
#define TR_HOLD_UPPERCASE              "保持"
#define TR_NONE                        "无效"
#define TR_NONE_UPPERCASE              "无效"
#define TR_MENUSENSOR                  "传感器"
#define TR_POWERMETER_PEAK             "提取"
#define TR_POWERMETER_POWER            "功率"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "频率"
#define TR_MENUTOOLS                   "扩展工具"
#define TR_TURN_OFF_RECEIVER           "请关闭接收机"
#define TR_STOPPING                    "正在停止..."
#define TR_MENU_SPECTRUM_ANALYSER      "频谱仪"
#define TR_MENU_POWER_METER            "功率计"
#define TR_SENSOR                      "传感器"
#define TR_COUNTRY_CODE                "国别码"
#define TR_USBMODE                     "USB模式"
#define TR_JACK_MODE                   "音频插孔模式"
#define TR_VOICE_LANGUAGE              "播报语言"
#define TR_UNITS_SYSTEM                "单位"
#define TR_EDIT                        "编辑"
#define TR_INSERT_BEFORE               "在本条前插入"
#define TR_INSERT_AFTER                "在本条后插入"
#define TR_COPY                        "复制"
#define TR_MOVE                        "移动"
#define TR_PASTE                       "粘贴"
#define TR_PASTE_AFTER                 "粘贴到之前"
#define TR_PASTE_BEFORE                "粘贴到之后"
#define TR_DELETE                      "删除"
#define TR_INSERT                      "插入"
#define TR_RESET_FLIGHT                "复位飞行记录"
#define TR_RESET_TIMER1                "复位计时器1"
#define TR_RESET_TIMER2                "复位计时器2"
#define TR_RESET_TIMER3                "复位计时器3"
#define TR_RESET_TELEMETRY             "复位回传参数"
#define TR_STATISTICS                  "统计"
#define TR_ABOUT_US                    "关于"
#define TR_USB_JOYSTICK                "USB 游戏柄 (HID)"
#define TR_USB_MASS_STORAGE            "USB 存储器 (SD)"
#define TR_USB_SERIAL                  "USB 串口 (VCP)"
#define TR_SETUP_SCREENS               "设置显示页面"
#define TR_MONITOR_SCREENS             "查看器"
#define TR_AND_SWITCH                  "与开关"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Calibrated analogs"
#define TR_ANADIAGS_FILTRAWDEV         "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW          "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX             "Min., max. and range"
#define TR_ANADIAGS_MOVE               "Move analogs to their extremes!"
#define TR_SPEAKER                     INDENT "扬声器"
#define TR_BUZZER                      INDENT "蜂鸣器"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "对频"))
#define TR_POWERMETER_ATTN_NEEDED      "需要串联衰减器"
#define TR_PXX2_SELECT_RX              "Select RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "选择设备"
#define TR_DISCOVER                    "发现"
#define TR_BUTTON_INIT                 BUTTON("初始化")
#define TR_WAITING                     "等待..."
#define TR_RECEIVER_DELETE             "是否删除接收机?"
#define TR_RECEIVER_RESET              "是否复位接收机?"
#define TR_SHARE                       "分享"
#define TR_BIND                        "对频"
#define TR_REGISTER                    TR("Reg", "注册")
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "测距"))
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "接收机选项")
#define TR_DEL_BUTTON                  BUTTON(TR("Del", "删除"))
#define TR_RESET_BTN                   BUTTON("复位")
#define TR_DEBUG                       "调试"
#define TR_KEYS_BTN                    BUTTON("按键")
#define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "类比"))
#define TR_TOUCH_NOTFOUND              "未找到触摸硬件"
#define TR_TOUCH_EXIT                  "点击屏幕退出"
#define TR_SET                         BUTTON("设置")
#define TR_TRAINER                     "教练"
#define TR_CHANS                       "通道"
#define TR_ANTENNAPROBLEM              CENTER "发射机天线故障!"
#define TR_MODELIDUSED                 "ID已使用:"
#define TR_MODULE                      "模块"
#define TR_RX_NAME                     "接收机名称"
#define TR_TELEMETRY_TYPE              TR("Type", "回传类型")
#define TR_TELEMETRY_SENSORS           "传感器"
#define TR_VALUE                       "数值"
#define TR_REPEAT                      "循环"
#define TR_ENABLE                      "启用"
#define TR_TOPLCDTIMER                 "Top LCD Timer"
#define TR_UNIT                        "单位"
#define TR_TELEMETRY_NEWSENSOR         INDENT "添加新传感器..."
#define TR_CHANNELRANGE                TR(INDENT "Ch. Range", INDENT "通道范围")
#define TR_RXFREQUENCY                 TR("Rx Freqency", "接收机端口输出频率")
#define TR_ANTENNACONFIRM1             "外置天线"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "内置天线","询问","基于模型","外置天线"
#else
  #define TR_ANTENNA_MODES             "内置天线","询问","基于模型","外置天线"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Use int. antenna", "请使用内置天线")
#define TR_USE_EXTERNAL_ANTENNA        TR("Use ext. antenna", "请使用外置天线")
#define TR_ANTENNACONFIRM2             TR("Check antenna", "请确认天线安装完好!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "请使用FLEX版本"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "请使用FCC版本"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "请使用EU版本"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "经认证的固件"
#define TR_LOWALARM                    INDENT "信号弱报警"
#define TR_CRITICALALARM               INDENT "信号极弱报警"
#define TR_RSSIALARM_WARN              "RSSI"
#define TR_NO_RSSIALARM                TR(INDENT "Alarms disabled", "回传报警已禁止")
#define TR_DISABLE_ALARM               TR(INDENT "Disable alarms", INDENT "禁止回传报警")
#define TR_ENABLE_POPUP                "允许弹窗"
#define TR_DISABLE_POPUP               "禁止弹窗"
#define TR_POPUP                       "弹窗提示"
#define TR_MIN                         "最小"
#define TR_MAX                         "最大"
#define TR_CURVE_PRESET                "预设..."
#define TR_PRESET                      "预设"
#define TR_MIRROR                      "镜像"
#define TR_CLEAR                       "清除"
#define TR_RESET                       "复位"
#define TR_RESET_SUBMENU               "复位..."
#define TR_COUNT                       "点数"
#define TR_PT                          "点"
#define TR_PTS                         "点"
#define TR_SMOOTH                      "平滑"
#define TR_COPY_STICKS_TO_OFS          TR("Cpy stick->subtrim", "当前摇杆位置保存为中点")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "当前所有通道中点/低位/高位保存为所有输出值")
#define TR_COPY_TRIMS_TO_OFS           TR("Cpy trim->subtrim", "当前微调按钮值保存到中点")
#define TR_INCDEC                      "增减"
#define TR_GLOBALVAR                   "全局变量"
#define TR_MIXSOURCE                   "混控源"
#define TR_CONSTANT                    "不变化"
#define TR_PERSISTENT_MAH              TR(INDENT "Str mAh", INDENT "关机保持 mAh")
#define TR_PREFLIGHT                   "初始位置检查"
#define TR_CHECKLIST                   TR(INDENT "Checklist", INDENT "显示列表")
#define TR_FAS_OFFSET                  TR(INDENT "FAS Ofs", INDENT "FAS 偏移量")
#define TR_AUX_SERIAL_MODE             "串口"
#define TR_AUX2_SERIAL_MODE            "串口 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port power"
#define TR_SCRIPT                      "脚本"
#define TR_INPUTS                      "输入"
#define TR_OUTPUTS                     "输出"
#define STR_EEBACKUP                   "EEPROM 数据备份"
#define STR_FACTORYRESET               "恢复出厂设置"
#define TR_CONFIRMRESET                TR("Erase ALL", "是否清除所有模型和设置数据?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Lua脚本数量超出限制!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP 电源"
#define TR_SPORT_UPDATE_POWER_MODES    "自动","开启"
#define TR_NO_TELEMETRY_SCREENS        "无回传页面"
#define TR_TOUCH_PANEL                 "触摸屏:"
#define TR_FILE_SIZE                   "文件大小"
#define TR_FILE_OPEN                   "还要打开文件吗?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "名称"
#define TR_PHASES_HEADERS_SW           "开关"
#define TR_PHASES_HEADERS_RUD_TRIM     "方向微调"
#define TR_PHASES_HEADERS_ELE_TRIM     "俯仰微调"
#define TR_PHASES_HEADERS_THT_TRIM     "油门微调"
#define TR_PHASES_HEADERS_AIL_TRIM     "横滚微调"
#define TR_PHASES_HEADERS_CH5_TRIM     "微调 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "微调 6"
#define TR_PHASES_HEADERS_FAD_IN       "渐入"
#define TR_PHASES_HEADERS_FAD_OUT      "渐出"

#define TR_LIMITS_HEADERS_NAME         "名称"
#define TR_LIMITS_HEADERS_SUBTRIM      "中点"
#define TR_LIMITS_HEADERS_MIN          "低位"
#define TR_LIMITS_HEADERS_MAX          "高位"
#define TR_LIMITS_HEADERS_DIRECTION    "方向"
#define TR_LIMITS_HEADERS_CURVE        "曲线"
#define TR_LIMITS_HEADERS_PPMCENTER    "PPM中点"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "微调模式"
#define TR_INVERTED     "反向"

#define TR_LSW_HEADERS_FUNCTION        "功能"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "与开关"
#define TR_LSW_HEADERS_DURATION        "持续时间"
#define TR_LSW_HEADERS_DELAY           "延时"

#define TR_GVAR_HEADERS_NAME           "名称"
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
#define TR_LSW_DESCRIPTIONS            { "比较类型或功能", "第一个变量", "第二个变量或常量", "第二个变量或常量", "启用本行的附加条件", "逻辑开关持续时间", "逻辑开关延时开启" }

// Horus layouts and widgets
#define TR_FIRST_CHANNEL               "起始通道"
#define TR_FILL_BACKGROUND             "是否填充背景?"
#define TR_BG_COLOR                    "背景颜色"
#define TR_SLIDERS_TRIMS               "滑块和微调"
#define TR_SLIDERS                     "滑块"
#define TR_FLIGHT_MODE                 "飞行模式"
#define TR_INVALID_FILE                "无效的文件"
#define TR_TIMER_SOURCE                "计时器选择"
#define TR_SIZE                        "尺寸"
#define TR_SHADOW                      "阴影"
#define TR_TEXT                        "文本"
#define TR_COLOR                       "颜色"
#define TR_MAIN_VIEW_X                 "Main view X"
#define TR_PANEL1_BACKGROUND           "面板1背景"
#define TR_PANEL2_BACKGROUND           "面板2背景"

// About screen
#define TR_ABOUTUS                     TR(" 关于 ", "关于")

#define TR_CHR_SHORT                   's'
#define TR_CHR_LONG                    'l'
#define TR_CHR_TOGGLE                  't'
#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "提示音音量"
#define TR_WAV_VOLUME                  "语音音量"
#define TR_BG_VOLUME                   TR("Bg volume", "背景声音量")

#define TR_TOP_BAR                     "顶栏"
#define TR_FLASH_ERASE                 "正在擦除..."
#define TR_FLASH_WRITE                 "正在写入..."
#define TR_OTA_UPDATE                  "OTA 更新..."
#define TR_MODULE_RESET                "正在复位模块..."
#define TR_UNKNOWN_RX                  "未知接收机"
#define TR_UNSUPPORTED_RX              "不支持此接收机"
#define TR_OTA_UPDATE_ERROR            "OTA 更新失败"
#define TR_DEVICE_RESET                "正在复位设备..."
#define TR_ALTITUDE                    INDENT "高度"
#define TR_SCALE                       "比例"
#define TR_VIEW_CHANNELS               "查看通道"
#define TR_VIEW_NOTES                  "View Notes"
#define TR_MODEL_SELECT                "模型选择"
#define TR_MODS_FORBIDDEN              "禁止修改!"
#define TR_UNLOCKED                    "已解锁"
#define TR_ID                          "ID"
#define TR_PRECISION                   "精度"
#define TR_RATIO                       "比率"
#define TR_FORMULA                     "公式"
#define TR_CELLINDEX                   "单节电池编号"
#define TR_LOGS                        "日志"
#define TR_OPTIONS                     "选项"
#define TR_FIRMWARE_OPTIONS            "固件选项"

#define TR_ALTSENSOR                   "Alt 传感器"
#define TR_CELLSENSOR                  "Cell 传感器"
#define TR_GPSSENSOR                   "GPS 传感器"
#define TR_CURRENTSENSOR               "传感器"
#define TR_AUTOOFFSET                  "自动偏移值"
#define TR_ONLYPOSITIVE                "正向"
#define TR_FILTER                      "滤波"
#define TR_TELEMETRYFULL               TR("All slots full!", "回传项目已满!")
#define TR_SERVOS_OK                   "Servos OK"
#define TR_SERVOS_KO                   "Servos KO"
#define TR_INVERTED_SERIAL             INDENT "反向"
#define TR_IGNORE_INSTANCE             TR(INDENT "No inst.", INDENT "忽略ID鉴别")
#define TR_DISCOVER_SENSORS            "扫描新的回传项目"
#define TR_STOP_DISCOVER_SENSORS       "停止扫描"
#define TR_DELETE_ALL_SENSORS          "删除所有回传项目"
#define TR_CONFIRMDELETE               "真的 " LCDW_128_480_LINEBREAK "要删除全部吗 ?"
#define TR_SELECT_WIDGET               "选择小部件"
#define TR_REMOVE_WIDGET               "移除小部件"
#define TR_WIDGET_SETTINGS             "小部件设置"
#define TR_REMOVE_SCREEN               "移除页面"
#define TR_SETUP_WIDGETS               "设置小部件"
#define TR_USER_INTERFACE              "显示设置"
#define TR_THEME                       "主题"
#define TR_SETUP                       "设置"
#define TR_LAYOUT                      "布局"
#define TR_ADD_MAIN_VIEW               "添加主视图"
#define TR_BACKGROUND_COLOR            "背景颜色"
#define TR_MAIN_COLOR                  "主颜色"
#define TR_BAR2_COLOR                  "二级条形图颜色"
#define TR_BAR1_COLOR                  "主条形图颜色"
#define TR_TEXT_COLOR                  "文本颜色"
#define TR_TEXT_VIEWER                 "文本视图"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "输入"
#define TR_MENU_LUA                    STR_CHAR_LUA "LUA脚本"
#define TR_MENU_STICKS                 STR_CHAR_STICK "摇杆"
#define TR_MENU_POTS                   STR_CHAR_POT "旋钮"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "固定值MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "斜盘混控CYC"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "微调"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "开关"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "逻辑开关"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "教练"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "通道"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "全局变量"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "回传"
#define TR_MENU_DISPLAY                "显示"
#define TR_MENU_OTHER                  "其它"
#define TR_MENU_INVERT                 "反向"
#define TR_JITTER_FILTER               "类比输入滤波"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Check RTC", "检查时间驱动电池电压")
#define TR_AUTH_FAILURE                "验证失败"
#define TR_RACING_MODE                 "竞速模式"

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

#define TR_USE_THEME_COLOR            "使用主题颜色"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS  "将所有微调导入中点偏移值"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS        "打开通道监视器" 
#else
  #define TR_OPEN_CHANNEL_MONITORS        "通道监视" 
#endif
#define TR_DUPLICATE                  "复制"
#define TR_ACTIVATE                   "启用"
#define TR_RED                        "红"
#define TR_BLUE                       "蓝"
#define TR_GREEN                      "绿"
#define TR_COLOR_PICKER               "拾取颜色"
#define TR_EDIT_THEME_DETAILS         "主题信息"
#define TR_AUTHOR                     "作者"
#define TR_DESCRIPTION                "描述"
#define TR_SAVE                       "保存"
#define TR_CANCEL                     "取消"
#define TR_EDIT_THEME                 "编辑主题"
#define TR_DETAILS                    "详细信息"
#define TR_THEME_EDITOR               "主题"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL     "模型选择"
#define TR_MAIN_MENU_MODEL_NOTES      "模型说明"
#define TR_MAIN_MENU_CHANNEL_MONITOR  "通道查看"
#define TR_MAIN_MENU_MODEL_SETTINGS   "模型设置"
#define TR_MAIN_MENU_RADIO_SETTINGS   "系统设置"
#define TR_MAIN_MENU_SCREEN_SETTINGS  "显示设置"
#define TR_MAIN_MENU_RESET_TELEMETRY  "复位功能"
#define TR_MAIN_MENU_STATISTICS       "统计信息"
#define TR_MAIN_MENU_ABOUT_EDGETX     "关于"
// End Main menu
