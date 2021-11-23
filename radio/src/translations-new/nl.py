#include "../translations.h"
#if defined(TRANSLATIONS_NL)

STR_OFFON = ['UIT','AAN']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Stil','Alarm','NoKey','Alles']
STR_VBLMODE = ['UIT','Keys','Stick','Beide','AAN','UIT','Toetsen','Sticks','Beide','AAN']
STR_TRNMODE = ['UIT','+=',':=']
STR_TRNCHN = ['CH1CH2CH3CH4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','Telem Mirror','Telemetry In','SBUS Leerling','LUA']
#else
STR_AUX_SERIAL_MODES = ['UIT','Telem Mirror','Telemetry In','SBUS Leerling','LUA']
#endif
STR_SWTYPES = ['Geen','Wissel','2POS','3POS']
STR_POTTYPES = ['None','Pot w. det','Multipos','Pot','Geen','Pot met Klik','Standenschakelaar','Pot zonder Klik']
STR_SLIDERTYPES = ['Geen','Schuif']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['UIT','Vliegtijd','Handmatige Reset']
STR_COUNTRY_CODES = ['US','JP','EU','Amerika','Japan','Europa']
STR_USBMODES = ['Ask','Joyst','SDCard','Serial','Ask','Joystick','Storage','Serial']
STR_JACK_MODES = ['Popup','Audio','Trainer']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['Expo','ExFijn','Fijn','Medium','Grof','Exponentieel','Extra Fijn','Fijn','Medium','Grof']
STR_VDISPLAYTRIMS = ['Nee','Kort','Ja']
STR_VBEEPCOUNTDOWN = ['Stilte','Beeps','Spraak','Tril']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VVARIOCENTER = ['Tonen','Stilte']
STR_CURVE_TYPES = ['Standaard','Custom']
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
STR_VUNITSSYSTEM = ['Mtrsch','Engels','Metrisch','Engels']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['Geen','Nums','Balken','Script']
STR_GPSFORMAT = ['DMS','NMEA']
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
STR_VKEYS = ['Menu','Exit','Down','Up','Right','Left']
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
STR_VTMRMODES = ['UIT','AAN','THs','TH%','THt']
STR_VFAILSAFE = ['Niet Gezet','Vasthouden','Custom','Geen Pulsen','Ontvanger','Not Set','Hold','Custom','No Pulses','Receiver']
STR_VSENSORTYPES = ['Custom','Berekend']
STR_VFORMULAS = ['Optellen','Gemiddeld','Min','Max','Vermenigvuld','Totaal','Cellen','Verbruik','Afstand']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['Laagste','1e Cel','2e Cel','3e Cel','4e Cel','5e Cel','6e Cel','Hoogste','Delta']
STR_TEXT_SIZE = ['Standard','Tiny','Small','Mid','Double']
STR_SUBTRIMMODES = ['(center only)','= (symetrical)']
#if defined(COLORLCD)
#else
#endif
#if defined(PCBFRSKY)
#elif defined(PCBNV14)
#else
#endif
#if defined(PCBHORUS)
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
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['Internal','Ask','Per model','Internal + External']
#else
STR_ANTENNA_MODES = ['Internal','Ask','Per model','External']
#endif
#if defined(COLORLCD)
#else
#endif
STR_SPORT_UPDATE_POWER_MODES = ['AUTO','ON']

#endif
