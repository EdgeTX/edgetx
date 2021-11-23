#include "../translations.h"
#if defined(TRANSLATIONS_PT)

STR_OFFON = ['OFF','ON']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Mudo','Alarm','NoKey','Todo']
STR_VBLMODE = ['OFF','Chav','Stks','Tudo','ON']
STR_TRNMODE = ['OFF','+=',':=']
STR_TRNCHN = ['CH1CH2CH3CH4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','S-Port Mirror','Telemetry','SBUS Trainer','LUA']
#else
STR_AUX_SERIAL_MODES = ['OFF','S-Port Mirror','Telemetry','SBUS Trainer','LUA']
#endif
STR_SWTYPES = ['None','Toggle','2POS','3POS']
STR_POTTYPES = ['None','Pot w. det','Multipos','Pot','None','Pot with detent','Multipos Switch','Pot']
STR_SLIDERTYPES = ['Rien','Slider']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['OFF','Flight','Manual Reset']
STR_COUNTRY_CODES = ['US','JP','EU','America','Japan','Europe']
STR_USBMODES = ['Ask','Joyst','SDCard','Serial','Ask','Joystick','Storage','Serial']
STR_JACK_MODES = ['Popup','Audio','Trainer']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['Expo','ExFino','Fino','Medio','Largo']
STR_VDISPLAYTRIMS = ['No','Change','Yes']
STR_VBEEPCOUNTDOWN = ['Silent','Beeps','Voice','Haptic']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VVARIOCENTER = ['Tone','Silent']
STR_CURVE_TYPES = ['Standard','Custom']
#if defined(PCBTARANIS) || defined(REVX)
STR_RETA123 = ['L','P','M','A','1','2','3','L','R']
#elif defined(PCBSKY9X)
STR_RETA123 = ['L','P','M','A','1','2','3','a']
#else
STR_RETA123 = ['L','P','M','A','1','2','3']
#endif
#if defined(PCBSKY9X) && defined(REVX)
STR_VOUTPUT_TYPE = ['OpenDrain','PushPull']
#endif
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['Adicionar','Multipl.','Trocar  0']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['OFF','ON','Lem','Pfd','Mot','Ail','T5','T6']
#else
STR_VMIXTRIMS = ['OFF','ON','Lem','Pfd','Mot','Ail']
#endif
#if defined(PCBTARANIS)
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
#ifdef GVARS
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
#if defined(PCBTARANIS)
#else
#endif
STR_FUNCSOUNDS = ['Bp1','Bp2','Bp3','Wrn1','Wrn2','Chee','Rata','Tick','Sirn','Ring','SciF','Robt','Chrp','Tada','Crck','Alrm','Beep1','Beep2','Beep3','Avis1','Avis2','Cheep','Ratata','Tick','Siren','Ring','SciFi','Robot','Chirp','Tada','Crickt','AlmClk']
#if defined(PCBTARANIS)
#else
#endif
STR_VUNITSSYSTEM = ['Metric','Imper.','Metric','Imperial']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['Nums','Bars']
STR_GPSFORMAT = ['HMS NMEA']
STR_VSWASHTYPE = ['---','120','120X','140','90']
#if defined(PCBHORUS) || defined(PCBNV14)
STR_VKEYS = ['PGUP','PGDN','ENTER','MDL','RTN','TELE','SYS']
#elif defined(RADIO_FAMILY_JUMPER_T12)
STR_VKEYS = ['Exit','Enter','Down','Up','Right','Left']
#elif defined(RADIO_TX12)
STR_VKEYS = ['Exit','Enter','Up','Down','SYS','MDL','TELE']
#elif defined(RADIO_T8)
STR_VKEYS = ['RTN','ENTER','PGUP','PGDN','SYS','MDL','UP','DOWN']
#elif defined(PCBTARANIS)
STR_VKEYS = ['Menu','Exit','Enter','Page','Plus','Minus']
#else
STR_VKEYS = ['Menu','Exit','Ned','Upp','Höger','Vänst']
#endif
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
STR_VTMRMODES = ['OFF','ABS','MTs','MT%','MTt']
STR_VFAILSAFE = ['Not set','Hold','Custom','No pulses','Receiver']
STR_VSENSORTYPES = ['Custom','Calculated']
STR_VFORMULAS = ['Add','Average','Min','Max','Multiply','Totalize','Cell','Consumpt','Distance']
STR_VPREC = ['PREC0','PREC1','PREC2']
STR_VCELLINDEX = ['Lowest','1','2','3','4','5','6','Highest','Delta']
STR_TEXT_SIZE = ['Standard','Tiny','Small','Mid','Double']
STR_SUBTRIMMODES = ['(center only)','= (symetrical)']
#if defined(COLORLCD)
#else
#endif
#if defined(PCBTARANIS) || defined(PCBHORUS)
#elif defined(PCBNV14)
#else
#endif
#if defined(PCBHORUS)
#else
#endif
#if defined(PCBTARANIS)
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
#else
#endif
#if defined(COLORLCD)
#else
#endif
#if defined(PPM_CENTER_ADJUSTABLE) || defined(PPM_LIMITS_SYMETRICAL) // The right menu titles for the gurus ...
#else
#endif
STR_SAMPLE_MODES = ['Normal','OneBit']
#if defined(PCBX9E)
STR_BLUETOOTH_MODES = ['---','Enabled']
#else
STR_BLUETOOTH_MODES = ['---','Telemetry','Trainer']
#endif
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['Internal','Ask','Per model','Internal + External']
#else
STR_ANTENNA_MODES = ['Internal','Ask','Per model','External']
#endif
STR_SPORT_UPDATE_POWER_MODES = ['AUTO','ON']

#endif
