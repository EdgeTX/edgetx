#include "../translations.h"
#if defined(TRANSLATIONS_CN)

STR_OFFON = ['禁用','开启']
STR_MMMINV = ['---','反']
STR_VBEEPMODE = ['静音','警告','忽略按键','全部']
STR_VBLMODE = ['关闭','按键','操控','全部','开启']
STR_TRNMODE = ['关','相加','替换']
STR_TRNCHN = ['CH1CH2CH3CH4']
STR_AUX_SERIAL_MODES = ['调试','回传镜像','回传输入','SBUS教练','LUA脚本']
STR_SWTYPES = ['无','回弹','2段','3段']
STR_POTTYPES = ['无','有中点旋钮','多段旋钮','无中点旋钮']
STR_SLIDERTYPES = ['无','侧滑块']
STR_VLCD = ['Normal','Optrex']
STR_VPERSISTENT = ['禁用','随飞行记录复位','随手动复位']
STR_COUNTRY_CODES = ['美国','日本','欧洲']
STR_USBMODES = ['询问','游戏柄','U盘','串行']
STR_JACK_MODES = ['询问','音频','教练']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['指数','很小','较小','中等','较大']
STR_VDISPLAYTRIMS = ['不显示','改变时','始终显示']
STR_VBEEPCOUNTDOWN = ['静音','蜂鸣','语音','震动']
STR_COUNTDOWNVALUES = ['5秒','10秒','20秒','30秒']
STR_VVARIOCENTER = ['音调','静音']
STR_CURVE_TYPES = ['标准','自定义']
STR_RETA123 = ['R','E','T','A','1','3','2','4','5','L','R']
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['相加','相乘','替换']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['禁用','启用','Rud','Ele','Thr','Ail','T5','T6']
#else
STR_VMIXTRIMS = ['禁用','启用','Rud','Ele','Thr','Ail']
#endif
#if defined(VARIO)
#else
#endif
#if defined(AUDIO)
#endif
#if defined(HAPTIC)
#else
#endif
#if defined(VOICE)
#else
#endif
#if defined(SDCARD)
#else
#endif
#if defined(GVARS)
#else
#endif
#if defined(LUA)
#else
#endif
#if defined(DEBUG)
#else
#endif
#if defined(OVERRIDE_CHANNEL_FUNCTION)
#else
#endif
STR_FUNCSOUNDS = ['Beep1','Beep2','Beep3','Warn1','Warn2','Cheep','Ratata','Tick','Siren','Ring','SciFi','Robot','Chirp','Tada','Crickt','AlmClk']
STR_VUNITSSYSTEM = ['公制','英制']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS']


STR_VTELEMSCREENTYPE = ['禁用','数值','条形图','脚本']
STR_GPSFORMAT = ['DMS','NMEA']
STR_VSWASHTYPE = ['---','120','120X','140','90']
STR_VKEYS = ['菜单','退出','确认','向上','向下','向右','向左']
#if defined(PCBHORUS)
#else
#endif
#if defined(PCBHORUS)
#else
#endif
#if defined(GYRO)
#else
#endif
#if defined(HELI)
#else
#endif
STR_VTMRMODES = ['OFF','ON','Strt','THs','TH%','THt']
STR_VFAILSAFE = ['未设置','失控保持','自定义','无脉冲','接收机']
STR_VSENSORTYPES = ['自定义','运算']
STR_VFORMULAS = ['加','平均值','最小值','最大值','乘','总计值','单节电池','消耗量','距离']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['最低值','1','2','3','4','5','6','最高值','差值']
STR_TEXT_SIZE = ['标准','小','较小','中等','双倍']
STR_SUBTRIMMODES = ['STR_CHAR_DELTA','(中点)','=(整体)']
#if defined(PCBFRSKY)
#elif defined(PCBNV14)
#else
#endif
#if defined(PCBHORUS)
#else
#endif
STR_SAMPLE_MODES = ['Normal','OneBit']
STR_BLUETOOTH_MODES = ['---','回传','教练']
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['内置天线','询问','基于模型','外置天线']
#else
STR_ANTENNA_MODES = ['内置天线','询问','基于模型','外置天线']
#endif
STR_SPORT_UPDATE_POWER_MODES = ['自动','开启']

#endif
