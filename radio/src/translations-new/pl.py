#if defined(TRANSLATIONS_PL)

STR_OFFON = ['WYŁ','ZAŁ']
STR_MMMINV = ['---','REV']
STR_VBEEPMODE = ['Cichy','Alarm','BezPr','Wsz.']
STR_VBLMODE = ['Wył','Przy','Drąż','Oba','Zał','Wyłącz','Przycisk','Drązki','Oba','Włącz']
STR_TRNMODE = ['Wył','+=',':=']
STR_TRNCHN = ['KN1KN2KN3KN4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','S-Port Kopia','Telemetria','Trener SBUS','LUA']
#else
STR_AUX_SERIAL_MODES = ['Wyłącz','S-Port Kopia','Telemetria','Trener SBUS','LUA']
#endif
STR_SWTYPES = ['Brak','Chwil.','2POZ','3POZ']
STR_POTTYPES = ['None','Pot w. det','Multipos','Pot','Brak','Poten z zapadką','Przeł.Wielopoz.','Potencjometr']
STR_SLIDERTYPES = ['Brak','Suwak']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['Wyłącz','Lot','Ręczny Reset']
STR_COUNTRY_CODES = ['US','JP','EU','Ameryka','Japonia','Europa']
STR_USBMODES = ['Ask','Joyst','SDCard','Serial','Ask','Joystick','Storage','Serial']
STR_JACK_MODES = ['Ask','Audio','Trainer']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['Expo','B.Dokł','Dokł.','Średni','Zgrubn','Expotencja','B.Dokładny','Dokładny','Średni','Zgrubny']
STR_VDISPLAYTRIMS = ['Nie','Zmień','Tak']
STR_VBEEPCOUNTDOWN = ['Cichy','Pik','Dźwięk','Wibrac']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VVARIOCENTER = ['Ton','Cicho']
STR_CURVE_TYPES = ['Standard','Własny']
#if defined(PCBHORUS)
STR_RETA123 = ['K','W','G','L','1','3','2','4','5','L','R']
#elif defined(PCBX9E)
STR_RETA123 = ['K','W','G','L','1','2','3','4','L','R','L','R']
#elif defined(PCBTARANIS) || defined(REVX)
STR_RETA123 = ['K','W','G','L','1','2','3','L','R']
#elif defined(PCBSKY9X)
STR_RETA123 = ['K','W','G','L','1','2','3','a']
#else
STR_RETA123 = ['K','W','G','L','1','2','3']
#endif
#if defined(PCBSKY9X) && defined(REVX)
STR_VOUTPUT_TYPE = ['OpenDrain','PushPull']
#endif
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['Dodaj','Mnóż','Zastąp']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['WYŁ','ZAŁ','SK','SW','GAZ','Lot','Ail','T5','T6']
#else
STR_VMIXTRIMS = ['WYŁ','ZAŁ','SK','SW','GAZ','Lot']
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
STR_FUNCSOUNDS = ['Bp1','Bp2','Bp3','Ost1','Ost2','Chee','Rata','Tik','Syre','Dzwo','SciF','Robt','Chrp','Tada','Crck','Alrm','Beep1','Beep2','Beep3','Ostrz1','Ostrz1','Cheep','Ratata','Tick','Syrena','Dzwone','SciFi','Robot','Chirp','Tada','Krytcz','AlmZeg']
#if LCD_W >= 212
#else
#endif
STR_VUNITSSYSTEM = ['Metr.','Imper.','Metryczn','Imperial']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['Brak','Liczb','Paski','Skrypt']
STR_GPSFORMAT = ['DMS','NMEA']
STR_VSWASHTYPE = ['---','120','120X','140','90']
#if defined(PCBHORUS) || defined(PCBNV14)
STR_VKEYS = ['PGUP','PGDN','ENTER','MDL','RTN','TELE','SYS']
#elif defined(PCBXLITE)
STR_VKEYS = ['Shift','Exit','Enter','Down','Up','Right','Left']
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
STR_VTMRMODES = ['WYŁ','ABS','THs','TH%','THt']
STR_VFAILSAFE = ['Brak','Utrzymuj','Własne','0 sygnału','Odbiornik']
STR_VSENSORTYPES = ['Użytkownik','Obliczone']
STR_VFORMULAS = ['Dodaj','Średnie','Min','Max','Mnóż','Zliczani','Komórka','Zużycie','Zasięg']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['Niskie','1','2','3','4','5','6','Wysokie','Delta']
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
STR_SPORT_UPDATE_POWER_MODES = ['AUTO','ON']

#endif
