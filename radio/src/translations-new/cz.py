#include "../translations.h"
#if defined(TRANSLATIONS_CZ)

STR_OFFON = ['VYP','ZAP']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Tichý','Alarm','BezKl','Vše','Tichý','Jen alarm','Bez kláves','Vše']
STR_VBLMODE = ['Vyp','Kláv.','Páky','Vše','Zap','Vypnuto','Klávesy','Páky','Vše','Zapnuto']
STR_TRNMODE = ['X','+=',':=']
STR_TRNCHN = ['CH1CH2CH3CH4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','Telem Mirror','Telemetry In','SBUS Trenér','LUA']
#else
STR_AUX_SERIAL_MODES = ['VYP','Telem Mirror','Telemetry In','SBUS Trenér','LUA']
#endif
STR_SWTYPES = ['Žádný','Bez aretace','2-polohový','3-polohový']
STR_POTTYPES = ['Žádný','Pot s aret.','Vícepol př.','Pot','Žádný','Pot s aretací','Vícepol. přep.','Potenciometr']
STR_SLIDERTYPES = ['Žádný','Slider']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['Ne','V rámci letu','Reset ručně']
STR_COUNTRY_CODES = ['US','JP','EU','Amerika','Japonsko','Evropa']
STR_USBMODES = ['Zeptat','Joyst','SDkarta','Serial','Zeptat se','Joystick','Úložiště','Serial']
STR_JACK_MODES = ['Zeptat','Audio','Trenér']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT   FrSky D        FrSky D (kabel)Spektrum']
STR_VTRIMINC = ['Expo','ExJemný','Jemný','Střední','Hrubý']
STR_VDISPLAYTRIMS = ['Ne','Změna','Ano']
STR_VBEEPCOUNTDOWN = ['Ne','Zvuk','Hlas','Vibrace']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VVARIOCENTER = ['Tón','Ticho']
STR_CURVE_TYPES = ['Rastr-X','Volná-XY']
#if defined(PCBHORUS)
STR_RETA123 = ['S','V','P','K','1','3','2','4','5','L','R']
#elif defined(PCBX9E)
STR_RETA123 = ['S','V','P','K','1','2','3','4','L','R','L','R']
#elif defined(PCBTARANIS) || defined(REVX)
STR_RETA123 = ['S','V','P','K','1','2','3','L','R']
#elif defined(PCBSKY9X)
STR_RETA123 = ['S','V','P','K','1','2','3','a']
#else
STR_RETA123 = ['S','V','P','K','1','2','3']
#endif
#if defined(PCBSKY9X) && defined(REVX)
STR_VOUTPUT_TYPE = ['OpenDrain','PushPull']
#endif
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['Sečíst','Násobit','Zaměnit']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['VYP','ZAP','Směr','Výšk','Plyn','Křid','T5','T6']
#else
STR_VMIXTRIMS = ['VYP','ZAP','Směr','Výšk','Plyn','Křid']
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
#if LCD_W >= 212
#else
#endif
STR_FUNCSOUNDS = ['Bp1','Bp2','Bp3','Wrn1','Wrn2','Chee','Rata','Tick','Sirn','Ring','SciF','Robt','Chrp','Tada','Crck','Alrm','Beep1','Beep2','Beep3','Warn1','Warn2','Cheep','Ratata','Tick','Siren','Ring','SciFi','Robot','Chirp','Tada','Crickt','AlmClk']
STR_VUNITSSYSTEM = ['Metr.','Imper.','Metrické','Imperial']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['Nic','Hodnota','Ukazatel','Skript']
STR_GPSFORMAT = ['DMS','NMEA']
STR_VSWASHTYPE = ['---','120','120X','140','90']
#if defined(PCBHORUS) || defined(PCBNV14)
STR_VKEYS = ['PGUP','PGDN','ENTER','MDL','RTN','TELE','SYS']
#elif defined(RADIO_FAMILY_JUMPER_T12)
STR_VKEYS = ['Exit','Enter','Dolů','Nhoru','Vprvo','Vlevo']
#elif defined(RADIO_TX12)
STR_VKEYS = ['Exit','Enter','Up','Down','SYS','MDL','TELE']
#elif defined(RADIO_T8)
STR_VKEYS = ['RTN','ENTER','PGUP','PGDN','SYS','MDL','UP','DOWN']
#elif defined(PCBTARANIS)
STR_VKEYS = ['Menu','Exit','Enter','Page','Plus','Mínus']
#else
STR_VKEYS = ['Menu','Exit','Dolů','Nhoru','Vprvo','Vlevo']
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
STR_VTMRMODES = ['VYP','ZAP','Pln>','Pln%','Pln*']
STR_VFAILSAFE = ['Nenastaven','Držet','Vlastní','Bez pulzů','Přijímač']
STR_VSENSORTYPES = ['Vlastní','Vypočtený']
STR_VFORMULAS = ['Součet','Průměr','Min','Max','Násobení','Totalize','Článek','Spotřeba','Vzdálenost']
STR_VPREC = ['X','X.X','X.XX']
STR_VCELLINDEX = ['Nízký','1','2','3','4','5','6','Nejvíce','Delta']
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
STR_BLUETOOTH_MODES = ['---','Povoleno']
#else
STR_BLUETOOTH_MODES = ['---','Telemetrie','Trenér']
#endif
#if defined(PCBX12S)
STR_ANTENNA_MODES = ['Internal','Ask','Per model','Internal + External']
#else
STR_ANTENNA_MODES = ['Internal','Ask','Per model','External']
#endif
STR_SPORT_UPDATE_POWER_MODES = ['AUTO','ON']

#endif
