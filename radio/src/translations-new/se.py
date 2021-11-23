#include "../translations.h"
#if defined(TRANSLATIONS_SE)

STR_OFFON = ['Av','På']
STR_MMMINV = ['---','INV']
STR_VBEEPMODE = ['Tyst','Alarm','EjKnp','Alla']
STR_COUNTDOWNVALUES = ['5s','10s','20s','30s']
STR_VBLMODE = ['Av','Knapp','Spak','Allt','På','Av','Knappar','Spakar','Allt','På']
STR_TRNMODE = ['Av','+=',':=']
STR_TRNCHN = ['KN1KN2KN3KN4']
#if defined(CLI) || defined(DEBUG)
STR_AUX_SERIAL_MODES = ['Debug','Spegling av S-Port','Telemetri','SBUS Trainer','LUA']
#else
STR_AUX_SERIAL_MODES = ['Av','Spegling av S-Port','Telemetri','SBUS Trainer','LUA']
#endif
STR_SWTYPES = ['Ingen','Flipp','2Pos','3Pos']
STR_POTTYPES = ['None','Pot w. det','Multipos','Pot','Ingen','Mittläges-pot','Flerlägesväljare','Potentiometer']
STR_SLIDERTYPES = ['Ingen','Skjutpot']
STR_VLCD = ['NormalOptrex']
STR_VPERSISTENT = ['Av','Flygning','Nolla Själv']
STR_COUNTRY_CODES = ['US','JP','EU','Amerika','Japan','Europa']
STR_USBMODES = ['Ask','Joyst','SDCard','Serial','Ask','Joystick','Storage','Serial']
STR_JACK_MODES = ['Ask','Audio','Trainer']
STR_TELEMETRY_PROTOCOLS = ['FrSky S.PORT','FrSky D','FrSky D (cable)','TBS Crossfire','Spektrum','AFHDS2A IBUS','Multi Telemetry']
STR_VTRIMINC = ['Expo','xFin','Fin','Medium','Grov','Exponentiell','Extra Fin','Fin','Medium','Grov']
STR_VDISPLAYTRIMS = ['Nej','Ändra','Ja']
STR_VBEEPCOUNTDOWN = ['Tyst','Pip','Röst','Vibrera']
STR_VVARIOCENTER = ['Pip','Tyst']
STR_CURVE_TYPES = ['Normal','Egen']
#if defined(PCBX9E)
STR_RETA123 = ['R','H','G','S','1','2','3','4','H','V','H','V']
#elif defined(PCBTARANIS) || defined(REVX)
STR_RETA123 = ['R','H','G','S','1','2','3','H','V']
#elif defined(PCBSKY9X)
STR_RETA123 = ['R','H','G','S','1','2','3','a']
#else
STR_RETA123 = ['R','H','G','S','1','2','3']
#endif
#if defined(PCBSKY9X) && defined(REVX)
STR_VOUTPUT_TYPE = ['OpenDrain','PushPull']
#endif
STR_VCURVEFUNC = ['---','x>0','x<0','|x|','f>0','f<0','|f|']
STR_VMLTPX = ['Addera','Förstärk','Ersätt']
STR_VMLTPX2 = ['+=','*=',':=']
#if defined(PCBHORUS)
STR_VMIXTRIMS = ['Av','På','Rod','Hjd','Gas','Ske','T5','T6']
#else
STR_VMIXTRIMS = ['Av','På','Rod','Hjd','Gas','Ske']
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
#if defined(OVERRIDE_CHANNEL_FUNCTION) && LCD_W >= 212
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
#else
#endif
#if defined(PCBTARANIS)
#else
#endif
STR_FUNCSOUNDS = ['Bp1','Bp2','Bp3','Wrn1','Wrn2','Chee','Rata','Tick','Sirn','Ring','SciF','Robt','Chrp','Tada','Crck','Alrm','Beep1','Beep2','Beep3','Warn1','Warn2','Cheep','Ratata','Tick','Siren','Ring','SciFi','Robot','Chirp','Tada','Crickt','AlmClk']
#if defined(PCBTARANIS)
#else
#endif
STR_VUNITSSYSTEM = ['Metri.','Imper.','Metriska','Imperial']
STR_VTELEMUNIT = ['-','V','A','mA','kts','m/s','f/s','kmh','mph','m','ft','@C','@F','%','mAh','W','mW','dB','rpm','g','@','rad','ml','fOz','mlm','Hz','mS','uS','km']


STR_VTELEMSCREENTYPE = ['Inget','Siffror','Staplar','Script']
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
STR_VTMRMODES = ['Av','På','GAs','GA%','GAt']
STR_VFAILSAFE = ['Ej givet','Lås Servo','Anpassat','Pulsfritt','Mottagare']
STR_VSENSORTYPES = ['Egen','Beräknad']
STR_VFORMULAS = ['Addera','Medelvärde','Min','Max','Multiplicera','Totalsumma','Cell','Förbrukning','Sträcka']
STR_VPREC = ['0.--','0.0','0.00']
STR_VCELLINDEX = ['Lägsta','1','2','3','4','5','6','Högsta','Skillnad']
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
