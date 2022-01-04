#include "../translations.h"
#if defined(TRANSLATIONS_IT)

STR_OFFON = ['OFF','ON']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Silen','Avvis','Notst','Tutti','Silente','Avvisi','No Tasti','Tutti']
STR_VBLMODE = ['OFF','Tasti','Stks','Tutti','ON','Spenta','Tasti','Sticks','Tutti','Accesa']
STR_TRNMODE = ['OFF','+=',':=']
STR_TRNCHN = ['ch1ch2ch3ch4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','Replica S-Port','Telemetria','SBUS Trainer','LUA']
#else
STR_AUX_SERIAL_MODES = ['OFF','Replica S-Port','Telemetria','SBUS Trainer','LUA']
#endif
STR_SWTYPES = ['Dis.','Toggle','2POS','3POS']
STR_POTTYPES = ['Dis.','Pot c. fer','Multipos','Pot','Dis.','Pot. con centro','Inter. Multipos','Potenziometro']
STR_SLIDERTYPES = ['Disat.','Slider']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['NO','Volo','Reset Manuale']
STR_COUNTRY_CODES = ['US','JP','EU','America','Japan','Europa']
STR_USBMODES = ['Chied','Joyst','SDCard','Serial','Chiedi','Joystick','Storage','Seriale']
STR_JACK_MODES = ['Chiedi','Audio','Trainer']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['Exp','ExFine','Fine','Medio','Ampio']
STR_VDISPLAYTRIMS = ['No','Cambio','Si']
STR_VBEEPCOUNTDOWN = ['Niente','Suoni','Voce','Vibra']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VVARIOCENTER = ['Tono','Silenz.']
STR_CURVE_TYPES = ['Fisso','Modific.']
#if defined(PCBHORUS)
STR_RETA123 = ['D','E','M','A','1','3','2','4','5','L','R']
#elif defined(PCBX9E)
STR_RETA123 = ['D','E','M','A','1','2','3','S','D','S','D']
#elif defined(PCBTARANIS) || defined(REVX)
STR_RETA123 = ['D','E','M','A','1','2','3','S','D']
#elif defined(PCBSKY9X)
STR_RETA123 = ['D','E','M','A','1','2','3','a']
#else
STR_RETA123 = ['D','E','M','A','1','2','3']
#endif
#if defined(PCBSKY9X) && defined(REVX)
STR_VOUTPUT_TYPE = ['OpenDrain','PushPull']
#endif
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['Add.','Molt.','Sost.']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['OFF','ON','Dir','Ele','Mot','Ale','T5','T6']
#else
STR_VMIXTRIMS = ['OFF','ON','Dir','Ele','Mot','Ale']
#endif
#if LCD_W >= 212
#else
#endif
STR_TEXT_SIZE = ['Standard','Tiny','Small','Mid','Double']
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
#if LCD_W >= 212
#else
#endif
STR_VUNITSSYSTEM = ['Metric','Imper.','Metriche','Imperiali']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['Niente','Valori','Barre','Script']
STR_GPSFORMAT = ['HMS NMEA']
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
STR_VTMRMODES = ['OFF','ABS','MOs','MO%','MOt']
STR_VFAILSAFE = ['Non settato','Mantieni','Personali','No impulsi','Ricevente']
STR_VSENSORTYPES = ['Custom','Calcolato']
STR_VFORMULAS = ['Somma','Media','Min','Max','Moltipl','Totalizza','Cella','Consumo','Distanza']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['Minore','1','2','3','4','5','6','Maggiore','Delta']
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
STR_BLUETOOTH_MODES = ['---','Attivo']
#else
STR_BLUETOOTH_MODES = ['---','Telemetr.','Trainer']
#endif
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['Interna','Chiedi','Per modello','Interna + esterna']
#else
STR_ANTENNA_MODES = ['Interna','Chiedi','Per model','Esterna']
#endif
STR_SPORT_UPDATE_POWER_MODES = ['AUTO','ON']

#endif
