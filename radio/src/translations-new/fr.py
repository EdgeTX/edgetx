#include "../translations.h"
#if defined(TRANSLATIONS_FR)

STR_OFFON = ['OFF','ON']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Aucun','Alarm','NoKey','Tout']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VBLMODE = ['OFF','Btns','Ctrl','Tous','ON','OFF','Touches','Controles','Tous','ON']
STR_TRNMODE = ['OFF','+=',':=']
STR_TRNCHN = ['CH1CH2CH3CH4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','Recopie Telem','Télémétrie In','Ecolage SBUS','LUA']
#else
STR_AUX_SERIAL_MODES = ['OFF','Recopie Telem','Télémétrie In','Ecolage SBUS','LUA']
#endif
STR_SWTYPES = ['Rien','Levier','2-POS','3-POS']
STR_POTTYPES = ['Rien','Pot av. ctr','Multipos','Pot','Rien','Pot avec centre','Inter multi-pos','Potentiomètre']
STR_SLIDERTYPES = ['Rien','Slider']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['OFF','Vol','Reset Manuel']
STR_COUNTRY_CODES = ['US','JP','EU','USA','Japon','Europe']
STR_USBMODES = ['Popup','Joyst','SDCard','Série','Demander','Joystick','Stockage','Série']
STR_JACK_MODES = ['Demander','Audio','Ecolage']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['Expo','ExFin','Fin','Moyen','Gros','Exponentiel','Extra Fin','Fin','Moyen','Grossier']
STR_VDISPLAYTRIMS = ['Non','Change','Oui']
STR_VBEEPCOUNTDOWN = ['Aucun','Bips','Voix   Haptic']
STR_VVARIOCENTER = ['Tone','Silent']
STR_CURVE_TYPES = ['Standard','Libre']
#if defined(PCBHORUS)
STR_RETA123 = ['D','P','G','A','1','3','2','4','5','L','R']
#elif defined(PCBX9E)
STR_RETA123 = ['D','P','G','A','1','2','3','4','L','R','L','R']
#elif defined(PCBTARANIS) || defined(REVX)
STR_RETA123 = ['D','P','G','A','1','2','3','L','R']
#elif defined(PCBSKY9X)
STR_RETA123 = ['D','P','G','A','1','2','3','a']
#else
STR_RETA123 = ['D','P','G','A','1','2','3']
#endif
#if defined(PCBSKY9X) && defined(REVX)
STR_VOUTPUT_TYPE = ['OpenDrain','PushPull']
#endif
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['Ajoute','Multipl.','Remplace','Additionner','Multiplier','Remplacer']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['OFF','ON','Dir','Prf','Gaz','Ail','T5','T6']
#else
STR_VMIXTRIMS = ['OFF','ON','Dir','Prf','Gaz','Ail']
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
STR_VUNITSSYSTEM = ['Métr.','Impér.','Métriques','Impériales']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['Rien','Valeurs','Barres','Script']
STR_GPSFORMAT = ['DMS','NMEA']
STR_VSWASHTYPE = ['---','120','120X','140','90']
#if defined(PCBHORUS) || defined(PCBNV14)
STR_VKEYS = ['PGUP','PGDN','ENTER','MDL','RTN','TELE','SYS']
#elif defined(PCBXLITE)
STR_VKEYS = ['Shift','Exit','Enter','Bas','Haut','Droit','Gauch']
#elif defined(RADIO_FAMILY_JUMPER_T12)
STR_VKEYS = ['Exit','Enter','Down','Up','Right','Left']
#elif defined(RADIO_TX12)
STR_VKEYS = ['Exit','Enter','Up','Down','SYS','MDL','TELE']
#elif defined(RADIO_T8)
STR_VKEYS = ['RTN','ENTER','PGUP','PGDN','SYS','MDL','UP','DOWN']
#elif defined(PCBTARANIS)
STR_VKEYS = ['Menu','Exit','Enter','Page','Plus','Moins']
#else
STR_VKEYS = ['Menu','Exit','Bas','Haut','Droit','Gauch']
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
STR_VTMRMODES = ['OFF','ON','GZs','GZ%','GZt']
STR_VFAILSAFE = ['Pas déf.','Maintien','Prédéf.','Pas d\'imp','Récepteur']
STR_VSENSORTYPES = ['Perso','Calculé']
STR_VFORMULAS = ['Addition','Moyenne','Min','Max','Multipl.','Totalise','Elément','Consomm.','Distance']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['Mini.','1','2','3','4','5','6','Maxi.','Diff.']
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
STR_BLUETOOTH_MODES = ['---','Activé']
#else
STR_BLUETOOTH_MODES = ['---','Télémétrie','Ecolage']
#endif
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['Interne','Demander','Par modèle','Interne + Externe']
#else
STR_ANTENNA_MODES = ['Interne','Demander','Par modèle','Externe']
#endif
#if defined(COLORLCD)
#elif defined(PCBXLITE)
#else
#endif
STR_SPORT_UPDATE_POWER_MODES = ['AUTO','ON']

#endif
