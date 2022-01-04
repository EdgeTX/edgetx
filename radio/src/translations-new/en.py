#include "../translations.h"
#if defined(TRANSLATIONS_EN)

STR_OFFON = ['OFF','ON']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Quiet','Alarm','NoKey','All']
STR_VBLMODE = ['OFF','Keys','Ctrl','Both','ON','OFF','Keys','Controls','Both','ON']
STR_TRNMODE = ['OFF','+=',':=']
STR_TRNCHN = ['CH1CH2CH3CH4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','Telem Mirror','Telemetry In','SBUS Trainer','LUA']
#else
STR_AUX_SERIAL_MODES = ['OFF','Telem Mirror','Telemetry In','SBUS Trainer','LUA']
#endif
STR_SWTYPES = ['None','Toggle','2POS','3POS']
STR_POTTYPES = ['None','Pot w. det','Multipos','Pot','None','Pot with detent','Multipos Switch','Pot']
STR_SLIDERTYPES = ['None','Slider']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['OFF','Flight','Manual Reset']
STR_COUNTRY_CODES = ['US','JP','EU','America','Japan','Europe']
STR_USBMODES = ['Ask','Joyst','SDCard','Serial','Ask','Joystick','Storage','Serial']
STR_JACK_MODES = ['Ask','Audio','Trainer']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['Expo','ExFine','Fine','Medium','Coarse','Exponential','Extra Fine','Fine','Medium','Coarse']
STR_VDISPLAYTRIMS = ['No','Change','Yes']
STR_VBEEPCOUNTDOWN = ['Silent','Beeps','Voice','Haptic']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VVARIOCENTER = ['Tone','Silent']
STR_CURVE_TYPES = ['Standard','Custom']
#if defined(PCBHORUS)
STR_RETA123 = ['R','E','T','A','1','3','2','4','5','L','R']
#elif defined(PCBX9E)
STR_RETA123 = ['R','E','T','A','1','2','3','4','L','R','L','R']
#elif defined(PCBTARANIS) || defined(REVX)
STR_RETA123 = ['R','E','T','A','1','2','3','L','R']
#elif defined(PCBSKY9X)
STR_RETA123 = ['R','E','T','A','1','2','3','a']
#else
STR_RETA123 = ['R','E','T','A','1','2','3']
#endif
#if defined(PCBSKY9X) && defined(REVX)
STR_VOUTPUT_TYPE = ['OpenDrain','PushPull']
#endif
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['Add','Multiply','Replace']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['OFF','ON','Rud','Ele','Thr','Ail','T5','T6']
#else
STR_VMIXTRIMS = ['OFF','ON','Rud','Ele','Thr','Ail']
#endif
#if LCD_W >= 212
#else
#endif
#if defined(VARIO)
#else
#endif
#if defined(AUDIO)
#else
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
#if defined(OVERRIDE_CHANNEL_FUNCTION) && LCD_W >= 212
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
#else
#endif
#if LCD_W >= 212
#else
#endif
STR_FUNCSOUNDS = ['Bp1','Bp2','Bp3','Wrn1','Wrn2','Chee','Rata','Tick','Sirn','Ring','SciF','Robt','Chrp','Tada','Crck','Alrm','Beep1','Beep2','Beep3','Warn1','Warn2','Cheep','Ratata','Tick','Siren','Ring','SciFi','Robot','Chirp','Tada','Crickt','AlmClk']
STR_VUNITSSYSTEM = ['Metric','Imper.','Metric','Imperial']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['None','Nums','Bars','Script']
STR_GPSFORMAT = ['DMS','NMEA']
STR_VSWASHTYPE = ['---','120','120X','140','90']
#if defined(PCBHORUS)
#else
#endif
#if defined(PCBHORUS)
#else
#endif
#if defined(PCBSKY9X)
#else
#endif
#if defined(GYRO)
#else
#endif
#if defined(HELI)
#else
#endif
STR_VTMRMODES = ['OFF','ON','Strt','THs','TH%','THt']
STR_VFAILSAFE = ['Not set','Hold','Custom','No pulses','Receiver']
STR_VSENSORTYPES = ['Custom','Calculated']
STR_VFORMULAS = ['Add','Average','Min','Max','Multiply','Totalize','Cell','Consumpt','Distance']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['Lowest','1','2','3','4','5','6','Highest','Delta']
STR_TEXT_SIZE = ['Standard','Tiny','Small','Mid','Double']
STR_SUBTRIMMODES = ['STR_CHAR_DELTA','(center only)','= (symetrical)']
#if defined(COLORLCD)
#else
#endif
#if defined(PCBFRSKY)
#elif defined(PCBNV14)
#else
#endif
#if defined(COLORLCD)
#else
#endif
#if defined(PCBTARANIS)
#else
#endif
#if defined(PCBSKY9X) && defined(REVX)
#endif
#if defined(PCBNV14)
#else
#endif
#if defined(PCBHORUS)
#elif defined(COLORLCD)
#else
#endif
#if defined(COLORLCD)
#else
#endif
STR_SAMPLE_MODES = ['Normal','OneBit']
#if defined(PCBX9E)
STR_BLUETOOTH_MODES = ['---','Enabled']
#else
STR_BLUETOOTH_MODES = ['---','Telemetry','Trainer']
#endif
#if defined(COLORLCD)
#else
#endif
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['Internal','Ask','Per model','Internal + External']
#else
STR_ANTENNA_MODES = ['Internal','Ask','Per model','External']
#endif
STR_SPORT_UPDATE_POWER_MODES = ['AUTO','ON']

#endif
