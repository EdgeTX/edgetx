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

#define TR_MIN_PLURAL2  2
#define TR_MAX_PLURAL2  2
// For this number of minute in the last decimal place singular form is used in
// plural
#define TR_USE_SINGULAR_IN_PLURAL 1
#define TR_USE_PLURAL2_SPECIAL_CASE 0
// If the number of minutes is above this value PLURAL2 is used
#define TR_USE_PLURAL2 INT_MAX

#define TR_MINUTE_SINGULAR             "分钟"
#define TR_MINUTE_PLURAL1              "分钟"
#define TR_MINUTE_PLURAL2              "分钟"

#define TR_OFFON                       "禁用","开启"
#define TR_MMMINV                      "---","反\0"
#define TR_VBEEPMODE                   "静音","警告","忽略按键","全部"
#define TR_VBLMODE                     "关闭","按键","操控","全部","开启"
#define TR_TRNMODE                     "关","相加","替换"
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"

#define TR_AUX_SERIAL_MODES            "调试","回传镜像","回传输入","SBUS教练","LUA脚本","CLI","GPS","Debug","SpaceMouse","外置发射"
#define TR_SWTYPES                     "无","回弹","2段","3段"
#define TR_POTTYPES                    "无","无中点旋钮",TR("中点旋钮","有中点旋钮"),"侧滑块",TR("多段","多段按键"),"X 轴","Y 轴","开关"
#define TR_VPERSISTENT                 "禁用","随飞行记录复位","随手动复位"
#define TR_COUNTRY_CODES               "美国","日本","欧洲"
#define TR_USBMODES                    "询问","游戏柄","U盘","串行"
#define TR_JACK_MODES                  "询问","音频","教练"

#define TR_SBUS_INVERSION_VALUES       "正常","未反相"
#define TR_MULTI_CUSTOM                "自定义"
#define TR_VTRIMINC                    "指数","很小","较小","中等","较大"
#define TR_VDISPLAYTRIMS               "不显示","改变时","始终显示"
#define TR_VBEEPCOUNTDOWN              "静音","蜂鸣","语音","震动",TR("B & H","Beeps & Haptic"),TR("V & H","Voice & Haptic")
#define TR_COUNTDOWNVALUES             "5秒","10秒","20秒","30秒"
#define TR_VVARIOCENTER                "音调","静音"
#define TR_CURVE_TYPES                 "标准","自定义"

#define TR_ADCFILTERVALUES             "全局","禁用","开启"

#define TR_VCURVETYPE                  "单边","指数","函数","自定义"
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "相加","相乘","替换"
#define TR_VMLTPX2                     "+=","*=",":="

#define TR_CSWTIMER                  "定时"
#define TR_CSWSTICKY                 "粘滞"
#define TR_CSWSTAY                   "边沿"

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY


#define TR_SF_TRAINER                  "教练"
#define TR_SF_INST_TRIM                "摇杆值存储到微调"
#define TR_SF_RESET                    "复位"
#define TR_SF_SET_TIMER                "设置"
#define TR_SF_VOLUME                   "音量"
#define TR_SF_FAILSAFE                 "设置失控保护"
#define TR_SF_RANGE_CHECK              "测距模式"
#define TR_SF_MOD_BIND                 "模块对频"
#define TR_SF_RGBLEDS                  "RGB LED 灯"

#define TR_SOUND                       "播放声音"
#define TR_PLAY_TRACK                  TR("Ply Trk", "播放音频文件")
#define TR_PLAY_VALUE                  "播放数值"
#define TR_SF_HAPTIC                   "振动"
#define TR_SF_BG_MUSIC                 "播放背景音乐"
#define TR_SF_BG_MUSIC_PAUSE           "暂停背景音乐"
#define TR_SF_LOGS                     "记录日志到SD卡"
#define TR_ADJUST_GVAR                 "修改全局变量GV值"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Lua脚本")
#define TR_SF_BACKLIGHT                "背光"
#define TR_SF_VARIO                    "Vario传感器"
#define TR_SF_TEST                     "测试"
#define TR_SF_SAFETY                   "锁定通道值"

#define TR_SF_SCREENSHOT               "截屏"
#define TR_SF_RACING_MODE              "竞速模式"
#define TR_SF_DISABLE_TOUCH            "禁用触摸"
#define TR_SF_DISABLE_AUDIO_AMP        "关闭音频功放"
#define TR_SF_SET_SCREEN               "选择主屏"
#define TR_SF_SET_TELEMSCREEN          "设置主屏"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "屏幕显示图传"

#define TR_FSW_RESET_TELEM             "回传参数"

#if LCD_W >= 212
  #define TR_FSW_RESET_TRIMS           "Trims"
  #define TR_FSW_RESET_TIMERS          "计时器1","计时器2","计时器3"
#else
  #define TR_FSW_RESET_TRIMS           "Trims"
  #define TR_FSW_RESET_TIMERS          "计时1","计时2","计时3"
#endif

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("全部","飞行"),TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "公制",TR("英制","英制")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "禁用","数值","条形图","脚本"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "Rud"
#define TR_STICK_NAMES1                "Ele"
#define TR_STICK_NAMES2                "Thr"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "ST"
#define TR_SURFACE_NAMES1              "TH"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Default","High"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ON","One"

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "按键帽模式"
#define TR_HATSOPT                     "微调","导航键","可切换","全局"
#define TR_HATSMODE_TRIMS              "按键帽: 微调"
#define TR_HATSMODE_KEYS               "按键帽: 导航键"
#define TR_HATSMODE_KEYS_HELP          "左侧按键帽:\n"\
                                       "   右 = MDL\n"\
                                       "   上 = SYS\n"\
                                       "   下 = TELE\n"\
                                       "\n"\
                                       "右侧按键帽:\n"\
                                       "   左 = PAGE<\n"\
                                       "   右 = PAGE>\n"\
                                       "   上 = PREV/INC\n"\
                                       "   下 = NEXT/DEC"
#endif

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "正常","反向"
#else
  #define TR_ROTARY_ENC_OPT         "正常","反向","V-I H-N","V-I H-A","V-N E-I"
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
#define TR_VCELLINDEX                  "最低值","1","2","3","4","5","6","7","8","最高值","差值"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA "(中点)", "=(整体)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")
#define TR_PPMUNIT                     "0.--","0.0","us"

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "STD"
#else
#define TR_FONT_SIZES                  "STD","BOLD","XXS","XS","L","XL","XXL"
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

#define TR_FREE                        "空闲"
#define TR_YES                         "是"
#define TR_NO                          "否"
#define TR_DELETEMODEL                 "删除 模型"
#define TR_COPYINGMODEL                "复制模型..."
#define TR_MOVINGMODEL                 "移动模型..."
#define TR_LOADINGMODEL                "载入模型..."
#define TR_UNLABELEDMODEL              "未分组"
#define TR_NAME                        "名称"
#define TR_MODELNAME                   "模型名称"
#define TR_PHASENAME                   "模式名称"
#define TR_MIXNAME                     "混控名称"
#define TR_INPUTNAME                   TR("名称", "输入名称")
#define TR_EXPONAME                    TR("名称", "曲线名称")
#define TR_BITMAP                      "模型图片"
#define TR_NO_PICTURE                  "无图片"
#define TR_TIMER                       TR("计时器", "计时器 ")
#define TR_START                       "开始"
#define TR_ELIMITS                     TR("扩展行程", "扩展通道行程")
#define TR_ETRIMS                      TR("扩展微调", "扩展微调量")
#define TR_TRIMINC                     "微调步幅"
#define TR_DISPLAY_TRIMS               TR("微调显示", "微调显示")
#define TR_TTRACE                      TR("输入源", "输入源")
#define TR_TTRIM                       TR("油门微调仅怠速", "油门微调只调整怠速")
#define TR_TTRIM_SW                    TR("微调选择", "微调选择")
#define TR_BEEPCTR                     TR("中点提示音", "中点蜂鸣提示音")
#define TR_USE_GLOBAL_FUNCS            TR("全局功能", "全局功能可介入")
#define TR_PPMFRAME                    "PPM帧"
#define TR_REFRESHRATE                 TR("刷新率", "刷新速率")
#define STR_WARN_BATTVOLTAGE           TR("输出为电池电压: ", "注意输出电平为电池电压")
#define TR_WARN_5VOLTS                 "注意输出电平为5V"
#define TR_MS                          "ms"
#define TR_SWITCH                      "开关"
#define TR_FUNCTION_SWITCHES           "可自定义开关"
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
#define TR_SF_SWITCH                   "Trigger"
#define TR_TRIMS                       "微调"
#define TR_FADEIN                      "渐入"
#define TR_FADEOUT                     "渐出"
#define TR_DEFAULT                     "(默认)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "检查当前飞行模式微调"
#else
  #define TR_CHECKTRIMS                "\006检查\012微调"
#endif
#define TR_SWASHTYPE                   "斜盘类型"
#define TR_COLLECTIVE                  TR("螺距源", "螺距混控源")
#define TR_AILERON                     TR("横滚源", "横滚混控源")
#define TR_ELEVATOR                    TR("俯仰源", "俯仰混控源")
#define TR_SWASHRING                   "斜盘行程"
#define TR_MODE                        "模式"
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "左摇杆"
#else
  #define TR_LEFT_STICK                "左摇杆"
#endif
#define TR_SUBTYPE                     "子类型"
#define TR_NOFREEEXPO                  "指数曲线已满!"
#define TR_NOFREEMIXER                 "混控数量已满!"
#define TR_SOURCE                      "输入源"
#define TR_WEIGHT                      "比例"
#define TR_SIDE                        "单边"
#define TR_OFFSET                      "偏移量"
#define TR_TRIM                        "使用微调"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "曲线"
#define TR_FLMODE                      TR("飞行模式", "飞行模式")
#define TR_MIXWARNING                  "警告"
#define TR_OFF                         "禁用"
#define TR_ANTENNA                     "天线"
#define TR_NO_INFORMATION              TR("无信息", "无信息")
#define TR_MULTPX                      "叠加方式"
#define TR_DELAYDOWN                   TR("下延时", "下行延时")
#define TR_DELAYUP                     "上行延时"
#define TR_SLOWDOWN                    TR("下慢放", "下行慢动作")
#define TR_SLOWUP                      "上行慢动作"
#define TR_MIXES                       "混控设置"
#define TR_CV                          "曲线"
#if defined(PCBNV14)
  #define TR_GV                        "GV"
#else
  #define TR_GV                        TR("G", "GV")
#endif
#define TR_RANGE                       "范围"
#define TR_CENTER                      "中点"
#define TR_ALARM                       "报警"
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
#define TR_BATTERY_RANGE               TR("电压范围", "电压测量范围")
#define TR_BATTERYCHARGING             "充电中..."
#define TR_BATTERYFULL                 "电量已满"
#define TR_BATTERYNONE                 "空!"
#define TR_BATTERYWARNING              "低电压报警值"
#define TR_INACTIVITYALARM             "长时间无操作"
#define TR_MEMORYWARNING               "内存不足"
#define TR_ALARMWARNING                "静音"
#define TR_RSSI_SHUTDOWN_ALARM         TR("关机检查RSSI", "关机时检查RSSI")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("关机检查教练", "关机时检查教练信号")
#define TR_MODEL_STILL_POWERED         "模型未断电"
#define TR_TRAINER_STILL_CONNECTED     "教练信号未断开"
#define TR_USB_STILL_CONNECTED         "USB未断开"
#define TR_MODEL_SHUTDOWN              "关机 ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "按ENTER键确认"
#define TR_THROTTLE_LABEL              "油门"
#define TR_THROTTLE_START              "油门开始"
#define TR_THROTTLEREVERSE             TR("油门反向", "油门反向")
#define TR_MINUTEBEEP                  TR("分", "分钟播报")
#define TR_BEEPCOUNTDOWN               "倒数"
#define TR_PERSISTENT                  TR("关机保持", "关机保持")
#define TR_BACKLIGHT_LABEL             "背光"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "状态"
#define TR_BLONBRIGHTNESS              "背光开"
#define TR_BLOFFBRIGHTNESS             "背光关"
#define TR_KEYS_BACKLIGHT              "按键背光"
#define TR_BLCOLOR                     "颜色"
#define TR_SPLASHSCREEN                "开机图片"
#define TR_PLAY_HELLO                  "开机语音"
#define TR_PWR_ON_DELAY                "开机延迟"
#define TR_PWR_OFF_DELAY               "关机延迟"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("开关机震动","开关机震动提示")
#define TR_THROTTLE_WARNING            TR("油门状态", "油门状态")
#define TR_CUSTOM_THROTTLE_WARNING     TR("自定位置", "自定油门位置?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("位置 %", "油门位置 %")
#define TR_SWITCHWARNING               TR("开关位置", "开关位置")
#define TR_POTWARNINGSTATE             "旋钮和滑块"
#define TR_POTWARNING                  TR("旋钮位置", "旋钮位置")
#define TR_TIMEZONE                    "时区"
#define TR_ADJUST_RTC                  "时钟设置"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("通道顺序", "默认通道顺序")
#define TR_STICKS                      "摇杆(轴)"
#define TR_POTS                        "电位器"
#define TR_SWITCHES                    "开关"
#define TR_SWITCHES_DELAY              TR("开关经过延时", "延时播放(开关经过)")
#define TR_SLAVE                       "从机"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "倍率"
#define TR_CAL                         "校准"
#define TR_CALIBRATION                 "校准"
#define TR_VTRIM                       "微调-+"
#define TR_CALIB_DONE                  "校准完成"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "按[Enter]键开始校准"
  #define TR_SETMIDPOINT               "校准中点：摇杆(轴)、旋钮、滑块，按[Enter]保存"
  #define TR_MOVESTICKSPOTS            "校准边界：摇杆(轴)、旋钮、滑块，按[Enter]保存并完成"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               "按[Enter]键开始"
  #define TR_SETMIDPOINT               "校准中点，按[Enter]保存"
  #define TR_MOVESTICKSPOTS            "校准边界，按[Enter]保存并完成"
#else
  #define TR_MENUTOSTART               "按" TR_ENTER " 开始"
#if defined(SURFACE_RADIO)
  #define TR_SETMIDPOINT               "校准中点"
  #define TR_MOVESTICKSPOTS            "校准边界"
#else
  #define TR_SETMIDPOINT               TR("校准中点", "校准中点")
  #define TR_MOVESTICKSPOTS            "校准边界"
#endif
  #define TR_MENUWHENDONE              TR_ENTER " 完成"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      "NO DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "内置 GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua 脚本"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_DURATION_MS                 TR("[D]","持续时间(ms): ")
#define TR_INTERVAL_MS                 TR("[I]","间隔时间(ms): ")
#define TR_MEM_USED_SCRIPT             "脚本(B): "
#define TR_MEM_USED_WIDGET             "小部件(B): "
#define TR_MEM_USED_EXTRA              "附加(B): "
#define TR_STACK_MIX                   "混控: "
#define TR_STACK_AUDIO                 "音频: "
#define TR_GPS_FIX_YES                 "修正: 是"
#define TR_GPS_FIX_NO                  "修正: 否"
#define TR_GPS_SATS                    "卫星: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "选单: "
#define TR_TIMER_LABEL                 "Timer"
#define TR_THROTTLE_PERCENT_LABEL      "Throttle %"
#define TR_BATT_LABEL                  "Battery"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " to reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "模型"
#if defined(SURFACE_RADIO)
#define TR_FM                          "DM"
#else
#define TR_FM                          "FM"
#endif
#define TR_EEPROMLOWMEM                "EEPROM low mem"
#define TR_PRESS_ANY_KEY_TO_SKIP       "按任意键跳过"
#define TR_THROTTLE_NOT_IDLE           "请将油门拉至最低!"
#define TR_ALARMSDISABLED              "已禁用报警"
#define TR_PRESSANYKEY                 TR("\010按任意键", "按任意键")
#define TR_BADEEPROMDATA               "存储数据错误"
#define TR_BAD_RADIO_DATA              "无法读取系统设置参数, 请检查SD卡"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            "格式化存储"
#define TR_STORAGE_FORMAT              "存储准备"
#define TR_EEPROMOVERFLOW              "存储超限"
#define TR_RADIO_SETUP                 "系统设置"
#define TR_MENUTRAINER                 "教练"
#define TR_MENUSPECIALFUNCS            "全局功能"
#define TR_MENUVERSION                 "版本"
#define TR_MENU_RADIO_SWITCHES         TR("开关", "开关及按键测试")
#define TR_MENU_RADIO_ANALOGS          TR("模拟", "模拟输入测试")
#define TR_MENU_RADIO_ANALOGS_CALIB     "已校准的模拟值"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "原始模拟值 (5 Hz)"
#define TR_MENUCALIBRATION             "校准"
#define TR_MENU_FSWITCH                "可自定义开关"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "将当前微调量保存至中点"
#else
  #define TR_TRIMS2OFFSETS             "\006微调存至中点"
#endif
#define TR_CHANNELS2FAILSAFE           "导入当前所有通道值"
#define TR_CHANNEL2FAILSAFE            "导入当前通道值"
#define TR_MENUMODELSEL                TR("模型选择", "模型选择")
#define TR_MENU_MODEL_SETUP            TR("模型设置", "模型设置")
#if defined(SURFACE_RADIO)
  #define TR_MENUFLIGHTMODES           "驾驶模式"
  #define TR_MENUFLIGHTMODE            "驾驶模式"
#else
  #define TR_MENUFLIGHTMODES           "飞行模式设置"
  #define TR_MENUFLIGHTMODE            "飞行模式"
#endif
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
#define TR_MULTI_RFTUNE                TR("频率微调", "频率微调")
#define TR_MULTI_RFPOWER               "发射功率"
#define TR_MULTI_WBUS                  "输出"
#define TR_MULTI_TELEMETRY             "回传"
#define TR_MULTI_VIDFREQ               TR("图传频率", "图传频率")
#define TR_RF_POWER                    "发射功率"
#define TR_MULTI_FIXEDID               TR("固定ID", "固定ID")
#define TR_MULTI_OPTION                TR("选项值", "选项值")
#define TR_MULTI_AUTOBIND              TR("对频通道", "通道控制对频")
#define TR_DISABLE_CH_MAP              TR("禁用通道映射", "禁用通道重映射")
#define TR_DISABLE_TELEM               TR("禁用回传", "禁用回传")
#define TR_MULTI_LOWPOWER              TR("低功率", "低功率模式")
#define TR_MULTI_LNA_DISABLE           "禁用接收放大器"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("开", "启用")
#define TR_DISABLE_INTERNAL            TR("禁用内置发射", "禁用内置发射")
#define TR_MODULE_NO_SERIAL_MODE       TR("非串口模式", "未开启串口模式")
#define TR_MODULE_NO_INPUT             TR("无输入", "无串行数据输入")
#define TR_MODULE_NO_TELEMETRY         TR3("无回传", "未收到多协议回传信息", "未收到多协议回传信息")
#define TR_MODULE_WAITFORBIND          "使用当前协议对频"
#define TR_MODULE_BINDING              TR("对频中","对频中")
#define TR_MODULE_UPGRADE_ALERT        TR3("需要升级", "多协议模块需要升级", "多协议模块需要升级")
#define TR_MODULE_UPGRADE              TR("需要升级", "多协议模块需要升级")
#define TR_REBIND                      "需要重新对频"
#define TR_REG_OK                      "注册成功"
#define TR_BIND_OK                     "对频成功"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 开启回传"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 关闭回传"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 开启回传"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 关闭回传"
#define TR_PROTOCOL_INVALID            TR("无效协议", "无效协议")
#define TR_MODULE_STATUS               TR("状态", "多协议状态")
#define TR_MODULE_SYNC                 TR("同步", "协议同步状态")
#define TR_MULTI_SERVOFREQ             TR("舵机频率", "舵机更新频率")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("频道", "选择射频频道")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "使用 FlySky RSSI 值无需重缩放")
#define TR_GPS_COORDS_FORMAT           TR("方向角", "方向角格式")
#define TR_VARIO                       TR("变化率", "高度变化率")
#define TR_PITCH_AT_ZERO               "基准音调"
#define TR_PITCH_AT_MAX                "最高音调"
#define TR_REPEAT_AT_ZERO              "无变化报警"
#define TR_BATT_CALIB                  TR("电池校准", "电池电压校准")
#define TR_CURRENT_CALIB               "电流校准"
#define TR_VOLTAGE                     TR("电压源", "电压来源")
#define TR_SELECT_MODEL                "选择模型"
#define TR_MANAGE_MODELS               "模型管理"
#define TR_MODELS                      "个模型"
#define TR_SELECT_MODE                 "选择模式"
#define TR_CREATE_MODEL                "创建模型"
#define TR_FAVORITE_LABEL              "收藏夹"
#define TR_MODELS_MOVED                "未使用模型移至"
#define TR_NEW_MODEL                   "新建模型"
#define TR_INVALID_MODEL               "无效模型"
#define TR_EDIT_LABELS                 "编辑分组"
#define TR_LABEL_MODEL                 "模型标签"
#define TR_MOVE_UP                     "上移"
#define TR_MOVE_DOWN                   "下移"
#define TR_ENTER_LABEL                 "输入分组名称"
#define TR_LABEL                       "分组"
#define TR_LABELS                      "分组"
#define TR_CURRENT_MODEL               "当前模型"
#define TR_ACTIVE                      "当前模型"
#define TR_NEW                         "新建"
#define TR_NEW_LABEL                   "新建分组"
#define TR_RENAME_LABEL                "分组改名"
#define TR_DELETE_LABEL                "删除分组"
#define TR_DUPLICATE_MODEL             "复制模型"
#define TR_COPY_MODEL                  "复制模型"
#define TR_MOVE_MODEL                  "移动模型"
#define TR_BACKUP_MODEL                "备份模型"
#define TR_DELETE_MODEL                "删除模型"
#define TR_RESTORE_MODEL               "恢复模型"
#define TR_DELETE_ERROR                "删除失败"
#define TR_SDCARD_ERROR                TR("存储卡错误", "存储卡错误")
#define TR_SDCARD                      "存储卡"
#define TR_NO_FILES_ON_SD              "存储卡中没有文件!"
#define TR_NO_SDCARD                   "存储卡未安装"
#define TR_WAITING_FOR_RX              "等待接收机响应..."
#define TR_WAITING_FOR_TX              "等待发射机响应..."
#define TR_WAITING_FOR_MODULE          TR("等待模块", "等待模块响应...")
#define TR_NO_TOOLS                    "无可用附加功能"
#define TR_NORMAL                      "正常"
#define TR_NOT_INVERTED                "正向"
#define TR_NOT_CONNECTED               "未连接"
#define TR_CONNECTED                   "已连接"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH关回传", "16CH 禁用回传")
#define TR_16CH_WITH_TELEMETRY         TR("16CH开回传", "16CH 开启回传")
#define TR_8CH_WITH_TELEMETRY          TR("8CH开回传", "8CH 开启回传")
#define TR_EXT_ANTENNA                 "外置天线"
#define TR_PIN                         "插针"
#define TR_UPDATE_RX_OPTIONS           "是否升级接收机?"
#define TR_UPDATE_TX_OPTIONS           "是否升级发射机?"
#define TR_MODULES_RX_VERSION          "模块/接收机版本"
#define TR_SHOW_MIXER_MONITORS         "显示通道混控"
#define TR_MENU_MODULES_RX_VERSION     "模块/接收机版本"
#define TR_MENU_FIRM_OPTIONS           "固件选项"
#define TR_IMU                         "陀螺仪"
#define TR_STICKS_POTS_SLIDERS         "摇杆(轴)/旋钮/滑块"
#define TR_PWM_STICKS_POTS_SLIDERS     "PWM摇杆(轴)/旋钮/滑块"
#define TR_RF_PROTOCOL                 "射频协议"
#define TR_MODULE_OPTIONS              "选项"
#define TR_POWER                       "功率"
#define TR_NO_TX_OPTIONS               "无发射机选项"
#define TR_RTC_BATT                    "时间驱动电池电压"
#define TR_POWER_METER_EXT             "功率计 (外置)"
#define TR_POWER_METER_INT             "功率计 (内置)"
#define TR_SPECTRUM_ANALYSER_EXT       "频谱仪 (外置)"
#define TR_SPECTRUM_ANALYSER_INT       "频谱仪 (内置)"
#define TR_SDCARD_FULL                 "SD卡已满"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\n日志和截屏功能将被禁用"
#else
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036日志和 " LCDW_128_LINEBREAK "截屏功能将被禁用"
#endif
#define TR_NEEDS_FILE                  "需要文件名包含"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "不兼容"
#define TR_WARNING                     "警告"
#define TR_STORAGE_WARNING             "存储"
#define TR_THROTTLE_UPPERCASE          "油门"
#define TR_ALARMSWARN                  "警告"
#define TR_SWITCHWARN                  TR("开关", "开关位置")
#define TR_FAILSAFEWARN                "失控保护"
#define TR_TEST_WARNING                TR("测试版", "测试版本")
#define TR_TEST_NOTSAFE                "只用于测试"
#define TR_WRONG_SDCARDVERSION         TR("需要版本: ", "请将SD卡文件更换为正确版本: ")
#define TR_WARN_RTC_BATTERY_LOW        "时间驱动电池电压低"
#define TR_WARN_MULTI_LOWPOWER         "低功率模式"
#define TR_BATTERY                     "电池"
#define TR_WRONG_PCBREV                "错误的硬件类型"
#define TR_EMERGENCY_MODE              "紧急模式"
#define TR_NO_FAILSAFE                 "失控保护未设置"
#define TR_KEYSTUCK                    "检测到有按键卡住"
#define TR_VOLUME                      "音量"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "亮度"
#define TR_CPU_TEMP                    "CPU 温度"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "主板温度"
#define TR_TTL_WARNING                 "警告: TX和RX引脚电平不能超过3.3V !"
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
#define TR_SCRIPT_PANIC                "脚本异常"
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
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "硬件"
#define TR_FORMATTING                  "正在格式化..."
#define TR_TEMP_CALIB                  "温度校准"
#define TR_TIME                        "时间"
#define TR_MAXBAUDRATE                 "最大带宽"
#define TR_BAUDRATE                    "波特率"
#define TR_SAMPLE_MODE                 "采样模式"
#define TR_SAMPLE_MODES                "标准","OneBit"
#define TR_LOADING                     "加载中..."
#define TR_DELETE_THEME                "删除主题?"
#define TR_SAVE_THEME                  "保存主题?"
#define TR_EDIT_COLOR                  "编辑颜色"
#define TR_NO_THEME_IMAGE              "没有预览图"
#define TR_BACKLIGHT_TIMER             "持续时间"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "快速选择模型"
  #define TR_LABELS_SELECT             "标签类型选择"
  #define TR_LABELS_MATCH              "标签匹配类型"
  #define TR_FAV_MATCH                 "收藏夹匹配类型"
  #define TR_LABELS_SELECT_MODE        "复选项","单选项"
  #define TR_LABELS_MATCH_MODE         "全部匹配","任意匹配"
  #define TR_FAV_MATCH_MODE            "必须匹配","可选匹配"
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "选择一个模板文件夹:"
#define TR_SELECT_TEMPLATE             "选择一个模板:"
#define TR_NO_TEMPLATES                "在此文件夹中未找到模型模板"
#define TR_SAVE_TEMPLATE               "存储为模板"
#define TR_BLANK_MODEL                 "空白模型"
#define TR_BLANK_MODEL_INFO            "创建一个空白模型"
#define TR_FILE_EXISTS                 "文件已存在"
#define TR_ASK_OVERWRITE               "是否覆盖?"

#define TR_BLUETOOTH                   "蓝牙"
#define TR_BLUETOOTH_DISC              "发现"
#define TR_BLUETOOTH_INIT              "初始化"
#define TR_BLUETOOTH_DIST_ADDR         "目标地址"
#define TR_BLUETOOTH_LOCAL_ADDR        "本机地址"
#define TR_BLUETOOTH_PIN_CODE          "PIN码"
#define TR_BLUETOOTH_NODEVICES         "未找到设备"
#define TR_BLUETOOTH_SCANNING          "正在扫描..."
#define TR_BLUETOOTH_BAUDRATE          "蓝牙带宽"
#if defined(PCBX9E)
  #define TR_BLUETOOTH_MODES           "---","启用"
#else
  #define TR_BLUETOOTH_MODES           "---","回传","教练"
#endif

#define TR_SD_INFO_TITLE               "存储卡详情"
#define TR_SD_SPEED                    "速度:"
#define TR_SD_SECTORS                  "扇区:"
#define TR_SD_SIZE                     "容量:"
#define TR_TYPE                        "类型"
#define TR_GLOBAL_VARS                 "全局变量"
#define TR_GVARS                       "全局变量"
#define TR_GLOBAL_VAR                  "全局变量"
#define TR_MENU_GLOBAL_VARS            "全局变量功能"
#define TR_OWN                         "Own"
#define TR_DATE                        "日期"
#define TR_MONTHS                      { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
#define TR_ROTARY_ENCODER              "滚轮"
#define TR_ROTARY_ENC_MODE             TR("滚轮模式","滚轮模式")
#define TR_CHANNELS_MONITOR            "通道查看器"
#define TR_MIXERS_MONITOR              "混控查看器"
#define TR_PATH_TOO_LONG               "路径太长"
#define TR_VIEW_TEXT                   "查看文本"
#define TR_FLASH_BOOTLOADER            "更新引导程序"
#define TR_FLASH_DEVICE                TR("更新设备","更新设备")
#define TR_FLASH_EXTERNAL_DEVICE       TR("从S.Port更新", "从 S.Port 端口更新外设固件")
#define TR_FLASH_RECEIVER_OTA          "Flash receiver OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("刷新蓝牙模块", "刷新蓝牙模块")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Flash pwr mngt unit"
#define TR_DEVICE_NO_RESPONSE          TR("无响应", "设备无响应")
#define TR_DEVICE_FILE_ERROR           TR("文件错误", "设备文件错误")
#define TR_DEVICE_DATA_REFUSED         TR("数据被拒绝", "设备数据被拒绝")
#define TR_DEVICE_WRONG_REQUEST        TR("无法访问设备", "无法访问设备")
#define TR_DEVICE_FILE_REJECTED        TR("文件被拒绝", "设备文件被拒绝")
#define TR_DEVICE_FILE_WRONG_SIG       TR("签名无效", "错误的文件签名")
#define TR_CURRENT_VERSION             TR("当前版本: ", "当前版本: ")
#define TR_FLASH_INTERNAL_MODULE       TR("更新内置模块", "更新内置模块固件")
#define TR_FLASH_INTERNAL_MULTI        TR("更新内置多协议", "更新内置多协议模块固件")
#define TR_FLASH_EXTERNAL_MODULE       TR("更新外置模块", "更新外置模块固件")
#define TR_FLASH_EXTERNAL_MULTI        TR("更新外置多协议", "更新外置多协议模块固件")
#define TR_FLASH_EXTERNAL_ELRS         TR("更新外置ELRS", "更新外置ELRS模块固件")
#define TR_FIRMWARE_UPDATE_ERROR       TR("更新失败", "固件更新失败")
#define TR_FIRMWARE_UPDATE_SUCCESS     "固件更新成功"
#define TR_WRITING                     "正在写入..."
#define TR_CONFIRM_FORMAT              "是否格式化?"
#define TR_INTERNALRF                  "内置发射"
#define TR_INTERNAL_MODULE             TR("内置发射", "内置发射")
#define TR_EXTERNAL_MODULE             TR("外置发射", "外置发射")
#define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX需要升级"
#define TR_TELEMETRY_DISABLED          "禁用回传"
#define TR_MORE_OPTIONS_AVAILABLE      "更多可用选项"
#define TR_NO_MODULE_INFORMATION       "无模块信息"
#define TR_EXTERNALRF                  "外置发射"
#define TR_FAILSAFE                    TR("失控保护", "失控保护模式")
#define TR_FAILSAFESET                 "失控保护设置"
#define TR_REG_ID                      "注册ID"
#define TR_OWNER_ID                    "所有者ID"
#define TR_HOLD                        "保持"
#define TR_HOLD_UPPERCASE              "保持"
#define TR_NONE                        "无效"
#define TR_NONE_UPPERCASE              "无效"
#define TR_MENUSENSOR                  "传感器"
#define TR_POWERMETER_PEAK             "提取"
#define TR_POWERMETER_POWER            "功率"
#define TR_POWERMETER_ATTN             "衰减器"
#define TR_POWERMETER_FREQ             "频率"
#define TR_MENUTOOLS                   "扩展工具"
#define TR_TURN_OFF_RECEIVER           "请关闭接收机"
#define TR_STOPPING                    "正在停止..."
#define TR_MENU_SPECTRUM_ANALYSER      "频谱仪"
#define TR_MENU_POWER_METER            "功率计"
#define TR_SENSOR                      "传感器"
#define TR_COUNTRY_CODE                "国别码"
#define TR_USBMODE                     "USB模式"
#define TR_JACK_MODE                   "教练插口模式"
#define TR_VOICE_LANGUAGE              "播报语言"
#define TR_UNITS_SYSTEM                "单位"
#define TR_UNITS_PPM                   "PPM 单位"
#define TR_EDIT                        "编辑"
#define TR_INSERT_BEFORE               "在本条前插入"
#define TR_INSERT_AFTER                "在本条后插入"
#define TR_COPY                        "复制"
#define TR_MOVE                        "移动"
#define TR_PASTE                       "粘贴"
#define TR_PASTE_AFTER                 "粘贴到本条之后"
#define TR_PASTE_BEFORE                "粘贴到本条之前"
#define TR_DELETE                      "删除"
#define TR_INSERT                      "插入"
#define TR_RESET_FLIGHT                "复位飞行数据"
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
#define TR_ANADIAGS_CALIB              "已校准的模拟量"
#define TR_ANADIAGS_FILTRAWDEV         "经差值过滤的原始模拟量"
#define TR_ANADIAGS_UNFILTRAW          "未经过滤的原始模拟量"
#define TR_ANADIAGS_MINMAX             "最低值、最高值、行程量"
#define TR_ANADIAGS_MOVE               "移动模拟量全行程以查看结果!"
#define TR_SPEAKER                     "扬声器"
#define TR_BUZZER                      "蜂鸣器"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("对频", "对频"))
#define TR_MODULE_UNBIND               BUTTON("解绑")
#define TR_POWERMETER_ATTN_NEEDED     "需要串联衰减器"
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
#define TR_REGISTER                    TR("注册", "注册")
#define TR_MODULE_RANGE                BUTTON(TR("测距", "测距"))
#define TR_RANGE_TEST                  "距离测试(低功率)"
#define TR_RECEIVER_OPTIONS            TR("选项", "接收机选项")
#define TR_RESET_BTN                   BUTTON("复位")
#define TR_DEBUG                       "调试"
#define TR_KEYS_BTN                    BUTTON("按键")
#define TR_ANALOGS_BTN                 BUTTON(TR("模拟", "模拟量"))
#define TR_FS_BTN                      BUTTON(TR("自定义开关", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "未找到触摸硬件"
#define TR_TOUCH_EXIT                  "点击屏幕退出"
#define TR_SET                         BUTTON("设置")
#define TR_TRAINER                     "教练"
#define TR_CHANS                       "通道"
#define TR_ANTENNAPROBLEM              "发射机天线故障!"
#define TR_MODELIDUSED                 "ID已使用:"
#define TR_MODELIDUNIQUE               "ID is unique"
#define TR_MODULE                      "模块"
#define TR_RX_NAME                     "接收机名称"
#define TR_TELEMETRY_TYPE              TR("类型", "回传类型")
#define TR_TELEMETRY_SENSORS           "传感器"
#define TR_VALUE                       "数值"
#define TR_PERIOD                      "周期"
#define TR_INTERVAL                    "间隔"
#define TR_REPEAT                      "循环"
#define TR_ENABLE                      "启用"
#define TR_DISABLE                     "禁用"
#define TR_TOPLCDTIMER                 "Top LCD Timer"
#define TR_UNIT                        "单位"
#define TR_TELEMETRY_NEWSENSOR         "添加新传感器..."
#define TR_CHANNELRANGE                TR("通道范围", "通道范围")
#define TR_ANTENNACONFIRM1             "外置天线"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "内置天线","询问","基于模型","外置天线"
#else
  #define TR_ANTENNA_MODES             "内置天线","询问","基于模型","外置天线"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("请使用内置天线", "请使用内置天线")
#define TR_USE_EXTERNAL_ANTENNA        TR("请使用外置天线", "请使用外置天线")
#define TR_ANTENNACONFIRM2             TR("检查天线", "请确认天线安装完好!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "请使用FLEX版本"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "请使用FCC版本"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "请使用EU版本"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "经认证的固件"
#define TR_LOWALARM                    "信号弱报警"
#define TR_CRITICALALARM               "信号极弱报警"
#define TR_DISABLE_ALARM               TR("禁止回传报警", "禁止回传报警")
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
#define TR_COPY_STICKS_TO_OFS          TR("摇杆位置存为中点", "当前摇杆位置保存为中点")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("通道行程存为输出",  "当前所有通道中点/低位/高位保存为所有输出值")
#define TR_COPY_TRIMS_TO_OFS           TR("微调存为中点", "当前微调按钮值保存到中点")
#define TR_INCDEC                      "增减"
#define TR_GLOBALVAR                   "全局变量"
#define TR_MIXSOURCE                   "输入源 (%)"
#define TR_MIXSOURCERAW                "输入源 (数值)"
#define TR_CONSTANT                    "固定值"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "关闭","开启","自动"
#define TR_PREFLIGHT                   "初始检查"
#define TR_CHECKLIST                   TR("显示列表", "显示列表")
#define TR_CHECKLIST_INTERACTIVE       TR3("检查列表", "自定义检查列表", "自定义检查列表")
#define TR_AUX_SERIAL_MODE             "串口"
#define TR_AUX2_SERIAL_MODE            "串口 2"
#define TR_AUX_SERIAL_PORT_POWER       "串口供电"
#define TR_SCRIPT                      "脚本"
#define TR_INPUTS                      "输入"
#define TR_OUTPUTS                     "输出"
#define STR_EEBACKUP                   "EEPROM 数据备份"
#define STR_FACTORYRESET               "恢复出厂设置"
#define TR_CONFIRMRESET                TR("是否清除?", "是否清除所有模型和设置数据?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Lua脚本数量超出限制!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP 电源"
#define TR_SPORT_UPDATE_POWER_MODES    "自动","开启"
#define TR_NO_TELEMETRY_SCREENS        "无回传页面"
#define TR_TOUCH_PANEL                 "触摸屏:"
#define TR_FILE_SIZE                   "文件大小"
#define TR_FILE_OPEN                   "强制打开?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

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
#define TR_INVERTED                    "反向"


#define TR_LSW_DESCRIPTIONS            { "比较类型或功能", "第一个变量", "第二个变量或常量", "第二个变量或常量", "启用本行的附加条件", "逻辑开关持续时间", "逻辑开关延时开启" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "起始通道"
  #define TR_FILL_BACKGROUND           "是否填充背景?"
  #define TR_BG_COLOR                  "背景颜色"
  #define TR_SLIDERS_TRIMS             "滑块和微调"
  #define TR_SLIDERS                   "滑块"
  #define TR_FLIGHT_MODE               "飞行模式"
  #define TR_INVALID_FILE              "无效的文件"
  #define TR_TIMER_SOURCE              "计时器选择"
  #define TR_SIZE                      "尺寸"
  #define TR_SHADOW                    "阴影"
  #define TR_ALIGNMENT                 "对齐"
  #define TR_ALIGN_LABEL               "对齐名称"
  #define TR_ALIGN_VALUE               "对齐值"
  #define TR_ALIGN_OPTS                { "左", "中", "右" }
  #define TR_TEXT                      "文本"
  #define TR_COLOR                     "颜色"
  #define TR_MAIN_VIEW_X               "Main view XX"
  #define TR_PANEL1_BACKGROUND         "面板1背景"
  #define TR_PANEL2_BACKGROUND         "面板2背景"
  #define TR_WIDGET_GAUGE              "行程"
  #define TR_WIDGET_MODELBMP           "模型信息"
  #define TR_WIDGET_OUTPUTS            "输出"
  #define TR_WIDGET_TEXT               "文本"
  #define TR_WIDGET_TIMER              "计时器"
  #define TR_WIDGET_VALUE              "数值"
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
  #define TR_BL_ENABLE                 "Enable"
  #define TR_BL_DISABLE                "Disable"
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
#elif defined(PCBNV14)
   // Bootloader NV14 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] to select file"
  #define TR_BL_FLASH_KEY             "Hold [R TRIM] long to flash"
  #define TR_BL_EXIT_KEY              " [L TRIM] to exit"
#elif defined(PCBPL18)
   // Bootloader PL18/NB4+ specific - ASCII characters only
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
#define TR_ABOUTUS                     TR(" 关于 ", "关于")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "提示音音量"
#define TR_WAV_VOLUME                  "语音音量"
#define TR_BG_VOLUME                   TR("背景音量", "背景声音量")

#define TR_TOP_BAR                     "顶栏"
#define TR_FLASH_ERASE                 "正在擦除..."
#define TR_FLASH_WRITE                 "正在写入..."
#define TR_OTA_UPDATE                  "OTA 更新..."
#define TR_MODULE_RESET                "正在复位模块..."
#define TR_UNKNOWN_RX                  "未知接收机"
#define TR_UNSUPPORTED_RX              "不支持此接收机"
#define TR_OTA_UPDATE_ERROR            "OTA 更新失败"
#define TR_DEVICE_RESET                "正在复位设备..."
#define TR_ALTITUDE                    "高度"
#define TR_SCALE                       "比例"
#define TR_VIEW_CHANNELS               "查看通道"
#define TR_VIEW_NOTES                  "View Notes"
#define TR_MODEL_SELECT                "模型选择"
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
#define TR_FILTER                      "滤波器"
#define TR_TELEMETRYFULL               TR("项目已满!", "回传项目已满!")
#define TR_IGNORE_INSTANCE             TR("忽略ID", "忽略ID鉴别")
#define TR_SHOW_INSTANCE_ID            "显示实例ID"
#define TR_DISCOVER_SENSORS            "扫描新的回传项目"
#define TR_STOP_DISCOVER_SENSORS       "停止扫描"
#define TR_DELETE_ALL_SENSORS          "删除所有回传项目"
#define TR_CONFIRMDELETE               "真的 " LCDW_128_LINEBREAK "要删除全部吗 ?"
#define TR_SELECT_WIDGET               "选择小部件"
#define TR_WIDGET_FULLSCREEN           "全屏"
#define TR_REMOVE_WIDGET               "移除小部件"
#define TR_WIDGET_SETTINGS             "小部件设置"
#define TR_REMOVE_SCREEN               "移除页面"
#define TR_SETUP_WIDGETS               "设置小部件"
#define TR_USER_INTERFACE              "显示设置"
#define TR_THEME                       "主题"
#define TR_SETUP                       "设置"
#define TR_LAYOUT                      "布局"
#define TR_ADD_MAIN_VIEW               "添加主视图"
#define TR_TEXT_COLOR                  "文本颜色"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "输入"
#define TR_MENU_LUA                    STR_CHAR_LUA "LUA脚本"
#define TR_MENU_STICKS                 STR_CHAR_STICK "摇杆(轴)"
#define TR_MENU_POTS                   STR_CHAR_POT "旋钮"
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "最小"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "最大"
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
#define TR_AUDIO_MUTE                  TR("自动静音","音频停播时自动静音")
#define TR_JITTER_FILTER               "ADC滤波器"
#define TR_DEAD_ZONE                   "死区"
#define TR_RTC_CHECK                   TR("检查时间电池", "检查时间驱动电池电压")
#define TR_AUTH_FAILURE                "验证失败"
#define TR_RACING_MODE                 "竞速模式"

#define TR_USE_THEME_COLOR            "使用主题颜色"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS  "将所有微调导入中点偏移值"
#if !PORTRAIT_LCD
  #define TR_OPEN_CHANNEL_MONITORS    "打开通道监视器"
#else
  #define TR_OPEN_CHANNEL_MONITORS    "通道监视"
#endif
#define TR_DUPLICATE                  "复制"
#define TR_ACTIVATE                   "启用"
#define TR_RED                        "红"
#define TR_BLUE                       "蓝"
#define TR_GREEN                      "绿"
#define TR_COLOR_PICKER               "拾取颜色"
#define TR_EDIT_THEME_DETAILS         "主题信息"
#define TR_THEME_COLOR_DEFAULT        "默认"
#define TR_THEME_COLOR_PRIMARY1       "主色1"
#define TR_THEME_COLOR_PRIMARY2       "主色2"
#define TR_THEME_COLOR_PRIMARY3       "主色3"
#define TR_THEME_COLOR_SECONDARY1     "副色1"
#define TR_THEME_COLOR_SECONDARY2     "副色2"
#define TR_THEME_COLOR_SECONDARY3     "副色3"
#define TR_THEME_COLOR_FOCUS          "焦点"
#define TR_THEME_COLOR_EDIT           "编辑"
#define TR_THEME_COLOR_ACTIVE         "激活"
#define TR_THEME_COLOR_WARNING        "警告"
#define TR_THEME_COLOR_DISABLED       "禁用"
#define TR_THEME_COLOR_CUSTOM         "自定义"
#define TR_THEME_CHECKBOX             "复选框"
#define TR_THEME_ACTIVE               "激活"
#define TR_THEME_REGULAR              "常规"
#define TR_THEME_WARNING              "警告"
#define TR_THEME_DISABLED             "禁用"
#define TR_THEME_EDIT                 "编辑"
#define TR_THEME_FOCUS                "焦点"
#define TR_AUTHOR                     "作者"
#define TR_DESCRIPTION                "描述"
#define TR_SAVE                       "保存"
#define TR_CANCEL                     "取消"
#define TR_EDIT_THEME                 "编辑主题"
#define TR_DETAILS                    "详细信息"
#define TR_THEME_EDITOR               "主题"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL     "模型选择"
#define TR_MAIN_MENU_MANAGE_MODELS    "模型管理"
#define TR_MAIN_MENU_MODEL_NOTES      "模型说明"
#define TR_MAIN_MENU_CHANNEL_MONITOR  "通道查看"
#define TR_MAIN_MENU_MODEL_SETTINGS   "模型设置"
#define TR_MAIN_MENU_RADIO_SETTINGS   "系统设置"
#define TR_MAIN_MENU_SCREEN_SETTINGS  "显示设置"
#define TR_MAIN_MENU_RESET_TELEMETRY  "复位功能"
#define TR_MAIN_MENU_STATISTICS       "统计信息"
#define TR_MAIN_MENU_ABOUT_EDGETX     "关于"
// End Main menu

#define TR_PROTOCOL                    "协议"

// Voice in native language
#define TR_VOICE_ENGLISH              "英语"
#define TR_VOICE_CHINESE              "中文-普通话"
#define TR_VOICE_CZECH                "捷克语"
#define TR_VOICE_DANISH               "丹麦语"
#define TR_VOICE_DEUTSCH              "德语"
#define TR_VOICE_DUTCH                "荷兰语"
#define TR_VOICE_ESPANOL              "西班牙语"
#define TR_VOICE_FRANCAIS             "法语"
#define TR_VOICE_HUNGARIAN            "匈牙利语"
#define TR_VOICE_ITALIANO             "意大利语"
#define TR_VOICE_POLISH               "波兰语"
#define TR_VOICE_PORTUGUES            "葡萄牙语"
#define TR_VOICE_RUSSIAN              "俄语"
#define TR_VOICE_SLOVAK               "斯洛伐克语"
#define TR_VOICE_SWEDISH              "瑞典语"
#define TR_VOICE_TAIWANESE            "中文-台湾国语"
#define TR_VOICE_JAPANESE             "日文"
#define TR_VOICE_HEBREW               "希伯来语"
#define TR_VOICE_UKRAINIAN            "乌克兰语"

#define TR_USBJOYSTICK_LABEL           "USB 游戏柄"
#define TR_USBJOYSTICK_EXTMODE         "模式"
#define TR_VUSBJOYSTICK_EXTMODE        "常规","高级"
#define TR_USBJOYSTICK_SETTINGS        "通道设置"
#define TR_USBJOYSTICK_IF_MODE         TR("接口模式","接口模式")
#define TR_VUSBJOYSTICK_IF_MODE        "操纵杆","游戏手柄","多轴摇杆"
#define TR_USBJOYSTICK_CH_MODE         "通道模式"
#define TR_VUSBJOYSTICK_CH_MODE        "禁用","按钮","轴","模拟"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "按钮模式"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "普通","脉冲","开关","双联","多联"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("普通","普通"),TR("脉冲","脉冲"),TR("开关","开关"),TR("双联","双联"),TR("多联","多联")
#define TR_USBJOYSTICK_CH_SWPOS        "位置"
#define TR_VUSBJOYSTICK_CH_SWPOS       "单按","2联","3联","4联","5联","6联","7联","8联"
#define TR_USBJOYSTICK_CH_AXIS         "轴"
#define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","旋转X","旋转Y","旋转Z","推杆","拨轮","方向盘"
#define TR_USBJOYSTICK_CH_SIM          "模拟轴"
#define TR_VUSBJOYSTICK_CH_SIM         "横滚","俯仰","航向","油门","加速","刹车","转盘","方向"
#define TR_USBJOYSTICK_CH_INVERSION    "反转"
#define TR_USBJOYSTICK_CH_BTNNUM       "按钮编号."
#define TR_USBJOYSTICK_BTN_COLLISION   "!按钮编号冲突!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!轴冲突!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("圆口行程", "圆口行程")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "无","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   "应用更改"

#define TR_DIGITAL_SERVO          "数字舵机333HZ"
#define TR_ANALOG_SERVO           "模拟舵机50HZ"
#define TR_SIGNAL_OUTPUT          "信号强度输出"
#define TR_SERIAL_BUS             "串行总线"
#define TR_SYNC                   "同步"

#define TR_ENABLED_FEATURES       "启用的功能"
#define TR_RADIO_MENU_TABS        "系统功能选项卡"
#define TR_MODEL_MENU_TABS        "模型功能选项卡"

#define TR_SELECT_MENU_ALL        "全部"
#define TR_SELECT_MENU_CLR        "清除"
#define TR_SELECT_MENU_INV        "反向"

#define TR_SORT_ORDERS            "名称 A-Z","名称 Z-A","不常用","常用"
#define TR_SORT_MODELS_BY         "排序"
#define TR_CREATE_NEW             "新建"

#define TR_MIX_SLOW_PREC          TR("慢放精度", "慢放精度 上行/下行")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "已存在同名主题目录 !"

#define ROTORFLIGHT_LUA_CH        "按败板半保备闭标表并波补不菜参叉差偿超车持池出存打大带待怠单到等低地点电调定动度舵二翻方放飞副复负高跟关管航号何合后护环缓回混火或机积集几计加间减件降交角教接阶截进救距开控宽馈拉来累类连练量零灵率滤螺落门面敏模目逆爬盘偏频平其启器前曲取确然入刹上设身升失时使式释试数衰水瞬顺速缩他态体停通退陀微尾位文稳误息熄下陷限线相响向消小校斜新心信型行悬旋循压页一移仪益翼因应用油右预援源载增针正直值止置制中重轴主转准子自踪总最左耦"

#define TR_DATE_TIME_WIDGET       "日期和时间"
#define TR_RADIO_INFO_WIDGET      "遥控器信息"
#define TR_LOW_BATT_COLOR         "低电量"
#define TR_MID_BATT_COLOR         "中电量"
#define TR_HIGH_BATT_COLOR        "高电量"

#define TR_WIDGET_SIZE            "小部件尺寸"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
