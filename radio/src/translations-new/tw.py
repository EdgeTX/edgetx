#include "../translations.h"
#if defined(TRANSLATIONS_TW)

STR_OFFON = ['禁用','開啟']
STR_MMMINV = ['---','反']
STR_VBEEPMODE = ['靜音','警告','忽略按鍵','全部']
STR_VBLMODE = ['關閉','按鍵','操控','全部','開啟']
STR_TRNMODE = ['關','相加','替換']
STR_TRNCHN = ['CH1CH2CH3CH4']
STR_AUX_SERIAL_MODES = ['調試','回傳鏡像','回傳輸入','SBUS教練','LUA腳本']
STR_SWTYPES = ['無','回彈','2段','3段']
STR_POTTYPES = ['無','有中點旋鈕','多段旋鈕','無中點旋鈕']
STR_SLIDERTYPES = ['無','側滑塊']
STR_VLCD = ['Normal','Optrex']
STR_VPERSISTENT = ['禁用','隨飛行記錄重啟','隨手動重啟']
STR_COUNTRY_CODES = ['美國','日本','歐洲']
STR_USBMODES = ['詢問','遊戲柄','U盤','串行']
STR_JACK_MODES = ['詢問','音頻','教練']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['指數','很小','較小','中等','較大']
STR_VDISPLAYTRIMS = ['不顯示','改變時','始終顯示']
STR_VBEEPCOUNTDOWN = ['靜音','蜂鳴','語音','震動']
STR_COUNTDOWNVALUES = ['5秒','10秒','20秒','30秒']
STR_VVARIOCENTER = ['音調','靜音']
STR_CURVE_TYPES = ['標準','自定義']
STR_RETA123 = ['R','E','T','A','1','3','2','4','5','L','R']
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['相加','相乘','替換']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['禁用','啟用','Rud','Ele','Thr','Ail','T5','T6']
#else
STR_VMIXTRIMS = ['禁用','啟用','Rud','Ele','Thr','Ail']
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


STR_VTELEMSCREENTYPE = ['禁用','數值','條形圖','腳本']
STR_GPSFORMAT = ['DMS','NMEA']
STR_VSWASHTYPE = ['---','120','120X','140','90']
STR_VKEYS = ['菜單','退出','確認','向上','向下','向右','向左']
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
STR_VFAILSAFE = ['未設置','失控保持','自定義','無脈衝','接收機']
STR_VSENSORTYPES = ['自定義','運算']
STR_VFORMULAS = ['加','平均值','最小值','最大值','乘','總計值','單節電池','消耗量','距離']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['最低值','1','2','3','4','5','6','最高值','差值']
STR_TEXT_SIZE = ['標準','小','較小','中等','雙倍']
STR_SUBTRIMMODES = ['STR_CHAR_DELTA','(中点)','=(整體)']
#if defined(PCBFRSKY)
#elif defined(PCBNV14)
#else
#endif
#if defined(PCBHORUS)
#else
#endif
STR_SAMPLE_MODES = ['Normal','OneBit']
STR_BLUETOOTH_MODES = ['---','回傳','教練']
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['內置天線','詢問','基於模型','外置天線']
#else
STR_ANTENNA_MODES = ['內置天線','詢問','基於模型','外置天線']
#endif
STR_SPORT_UPDATE_POWER_MODES = ['自動','開啟']

#endif
