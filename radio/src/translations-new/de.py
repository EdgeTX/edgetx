#if defined(TRANSLATIONS_DE)

STR_OFFON = ['AUS','EIN']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Stumm','Alarm','NoKey','Alle']
STR_VBLMODE = ['AUS','Taste','Stks','Beide','EIN']
STR_TRNMODE = ['AUS','+=',':=']
STR_TRNCHN = ['CH1CH2CH3CH4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','Telem Mirror','Telemetry In','SBUS Eingang','LUA']
#else
STR_AUX_SERIAL_MODES = ['AUS','Telem Mirror','Telemetry In','SBUS Eingang','LUA']
#endif
STR_SWTYPES = ['','Kein','Taster','2POS','3POS']
STR_POTTYPES = ['','None','Pot w. det','Multipos','Pot','Kein','Poti mit Raste','Stufen-Schalter','Poti ohne Raste']
STR_SLIDERTYPES = ['Keine','Slider']
STR_VLCD = ['','NormalOptrex']
STR_VPERSISTENT = ['AUS','Flugzeit','Manuell Ruck']
STR_COUNTRY_CODES = ['US','JP','EU','Amerika','Japan','Europa']
STR_USBMODES = ['Fragen','Joyst','SDCard','Serial','Fragen','Joystick','Speicher','Seriell']
STR_JACK_MODES = ['Popup','Audio','Trainer']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (Kabel)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['','Expo','ExFein','Fein','Mittel','Grob','Exponentiell','Extrafein','Fein','Mittel','Grob']
STR_VDISPLAYTRIMS = ['Nein','Kurz','Ja']
STR_VBEEPCOUNTDOWN = ['Kein','Pieps','Stimme','Haptik']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VVARIOCENTER = ['Ton','Ruhe']
STR_CURVE_TYPES = ['','Nur Y','X und Y','Standard','Custom']
#if defined(PCBHORUS)
STR_RETA123 = ['S','H','G','Q','1','3','2','4','5','L','R']
#elif defined(PCBX9E)
STR_RETA123 = ['S','H','G','Q','1','2','3','4','L','R','L','R']
#elif defined(PCBTARANIS) || defined(REVX)
STR_RETA123 = ['S','H','G','Q','1','2','3','L','R']
#elif defined(PCBSKY9X)
STR_RETA123 = ['S','H','G','Q','1','2','3','a']
#else
STR_RETA123 = ['S','H','G','Q','1','2','3']
#endif
#if defined(PCBSKY9X) && defined(REVX)
STR_VOUTPUT_TYPE = ['OpenDrain','PushPull']
#endif
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['Addiere','Multipl.','Ersetze']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['AUS','EIN','Sei','Höh','Gas','Que','T5','T6']
#else
STR_VMIXTRIMS = ['AUS','EIN','Sei','Höh','Gas','Que']
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
STR_FUNCSOUNDS = ['','Bp1','Bp2','Bp3','Wrn1','Wrn2','Chee','Rata','Tick','Sirn','Ring','SciF','Robt','Chrp','Tada','Crck','Alrm','Piep1','Piep2','Piep3','Warn1','Warn2','Cheep','Ratata','Tick','Siren','Ring','SciFi','Robot','Chirp','Tada','Crickt','AlmClk']
STR_VUNITSSYSTEM = ['Metrik','Imper.','Metrisch','Imperial']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']

SSTR_A = ['']
STR_VTELEMSCREENTYPE = ['None','Werte','Balken','Script']
STR_GPSFORMAT = ['','GMS','NMEA']
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
STR_VTMRMODES = ['','AUS','EIN','GSs','GS%','GSt']
STR_VFAILSAFE = ['','Kein Failsafe','Halte Pos.','Kanäle','Kein Signal','Empfänger']
STR_VSENSORTYPES = ['','Sensor','Berechnung']
STR_VFORMULAS = ['','Addieren','Mittelwert','Min','Max','Multiplizier','Gesamt','Zelle','Verbrauch','Distanz']
STR_VPREC = ['','0.--','0.0','0.00']
STR_VCELLINDEX = ['','Niedrigst','1. Zelle','2. Zelle','3. Zelle','4. Zelle','5. Zelle','6. Zelle','Höchster','Differenz']
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
