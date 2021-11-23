#if defined(TRANSLATIONS_ES)

STR_OFFON = ['OFF','ON']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Silencio','Alarma','No tecla','Todo']
STR_VBLMODE = ['OFF','Teclas','Sticks','Ambos','ON']
STR_TRNMODE = ['OFF','+=',':=']
STR_TRNCHN = ['CH1CH2CH3CH4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','Telem Mirror','Telemetría','Entrenador SBUS','LUA']
#else
STR_AUX_SERIAL_MODES = ['OFF','Telem Mirror','Telemetría','Entrenador SBUS','LUA']
#endif
STR_SWTYPES = ['Nada','Palanca','2POS','3POS']
STR_POTTYPES = ['Nada','Pot con fij','Multipos','Pot','Nada','Pot con fijador','Switch multipos','Pot']
STR_SLIDERTYPES = ['Nada','Slider']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['OFF','Vuelo','Reset manual']
STR_COUNTRY_CODES = ['US','JP','EU','América','Japón','Europa']
STR_USBMODES = ['Pregunta','Joystick','SDCard','Serie']
STR_JACK_MODES = ['Pregunta','Audio','Entrena.']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetr.']
STR_VTRIMINC = ['Expo','ExFino','Fino','Medio','Grueso','Exponencial','Extra fino','Fino','Medio','Grueso']
STR_VDISPLAYTRIMS = ['No','Cambiar','Si']
STR_VBEEPCOUNTDOWN = ['Silencio','Beeps','Voz','Haptic']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VVARIOCENTER = ['Tono','Silencio']
STR_CURVE_TYPES = ['Normal','Custom']
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
STR_VMLTPX = ['A\201adir','Multipl.','Cambiar']
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
#if defined(OVERRIDE_CHANNEL_FUNCTION)
#else
#endif
#if LCD_W >= 212
#else
#endif
STR_FUNCSOUNDS = ['Bp1','Bp2','Bp3','Avs1','Avs2','Chee','Rata','Tick','Sirn','Ring','SciF','Robt','Chrp','Tada','Crck','Alrm','Beep1','Beep2','Beep3','Aviso1','Aviso2','Cheep','Ratata','Tick','Sirena','Ring','SciFi','Robot','Chirp','Tada','Crickt','AlmClk']
STR_VUNITSSYSTEM = ['Métrico','Imperial']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['Nada','Números','Barras','Script']
STR_GPSFORMAT = ['HMS NMEA']
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
STR_VTMRMODES = ['OFF','ABS','THs','TH%','THt']
STR_VFAILSAFE = ['No','Hold','Custom','No pulsos','Receptor']
STR_VSENSORTYPES = ['Custom','Calculado']
STR_VFORMULAS = ['Suma','Media','Min','Máx','Multipl.','Total','Cell','Consumo','Distancia']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['Menor','1','2','3','4','5','6','Mayor','Delta']
STR_TEXT_SIZE = ['Normal','Muy pequeño','Pequeño','Medio','Doble']
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
STR_BLUETOOTH_MODES = ['---','Activo']
#else
STR_BLUETOOTH_MODES = ['---','Telemetría','Entrenador']
#endif
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['Interna','Preguntar','Por modelo','Interno + Externo']
#else
STR_ANTENNA_MODES = ['Interna','Preguntar','Por modelo','Externa']
#endif
STR_SPORT_UPDATE_POWER_MODES = ['AUTO','ON']

#endif
