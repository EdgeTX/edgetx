/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
// FR translations authors: Andre Bernet <bernet.andre@gmail.com>
// Pat6874 <andropat68@gmail.com>
/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */

#define TR_MIN_PLURAL2  2
#define TR_MAX_PLURAL2  2
// For this number of minute in the last decimal place singular form is used in
// plural
#define TR_USE_SINGULAR_IN_PLURAL 1
#define TR_USE_PLURAL2_SPECIAL_CASE 0
// If the number of minutes is above this value PLURAL2 is used
#define TR_USE_PLURAL2 INT_MAX

#define TR_MINUTE_SINGULAR             "minute"
#define TR_MINUTE_PLURAL1              "minutes"
#define TR_MINUTE_PLURAL2              "minutes"

// NON ZERO TERMINATED STRINGS
#define TR_OFFON                       "OFF","ON"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Aucun","Alarme","NoKey","Tout"
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VBLMODE                     "OFF",TR("Btns","Touches"),TR("Ctrl","Contrôles"),"Tous","ON"
#define TR_TRNMODE                     "OFF",TR("+=","Ajoute"),TR(":=","Remplace")
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES            "OFF","Recopie Télém.","Télémétrie In","Écolage SBUS","LUA","CLI","GPS","Débug","SpaceMouse","Module externe"

#define TR_SWTYPES                     "Rien","Monostable","2-POS","3-POS"
#define TR_POTTYPES                    "Rien",TR("Pots","Potentiomètre"),TR("Pots av. ctr","Pots avec centre"),"Curseurs",TR("Multipos.","Inter multi-pos""Potentiomètre"),"Axis X","Axis Y","Switch"

#define TR_VPERSISTENT                 "OFF","Vol","Reset Manuel"
#define TR_COUNTRY_CODES               TR("US","USA"),TR("JP","Japon"),TR("Europe","EU")
#define TR_USBMODES                    TR("Popup","Demander"),TR("Joyst","Joystick"),TR("SDCard","Stockage"),"Série"
#define TR_JACK_MODES                  "Demander","Audio","Écolage"

#define TR_SBUS_INVERSION_VALUES       "normal","non inversé"
#define TR_MULTI_CUSTOM                "Perso"
#define TR_VTRIMINC                    TR("Expo","Exponentiel"),TR("ExFin","Extra Fin"),"Fin","Moyen",TR("Gros","Grossier")
#define TR_VDISPLAYTRIMS               "Non","Change","Oui"
#define TR_VBEEPCOUNTDOWN              "Aucun","Bips","Voix","Vibreur",TR("B & H","Bips & Vibreur"),TR("V & H","Voix & Vibreur")
#define TR_VVARIOCENTER                "Son","Silencieux"
#define TR_CURVE_TYPES                 "Standard","Libre"

#define TR_ADCFILTERVALUES             "Global","Off","On"

#define TR_VCURVETYPE                  "Diff","Expo","Fonct","Perso."
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      TR("Ajoute","Additionner"),TR("Multipl.","Multiplier"),TR("Remplace","Remplacer")
#define TR_VMLTPX2                     "+=","*=",":="

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Chrono"
  #define TR_CSWSTICKY                 "Bistb"
  #define TR_CSWSTAY                   "Edge"
#else
  #define TR_CSWTIMER                  "Chrono"
  #define TR_CSWSTICKY                 "Bist"
  #define TR_CSWSTAY                   "Edge"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","ET","OU","OUX",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Écolage"
#define TR_SF_INST_TRIM                "Inst. Trim"
#define TR_SF_RESET                    "Remise à 0"
#define TR_SF_SET_TIMER                "Déf."
#define TR_SF_VOLUME                   "Volume"
#define TR_SF_FAILSAFE                 "Régle Failsafe"
#define TR_SF_RANGE_CHECK              "Test Port."
#define TR_SF_MOD_BIND                 "Bind"
#define TR_SF_RGBLEDS                  "RGB leds"
 
#define TR_SOUND                       "Jouer son"
#define TR_PLAY_TRACK                  TR("Ply Trk", "Jouer fichier")
#define TR_PLAY_VALUE                  "Lire valeur"
#define TR_SF_HAPTIC                   "Vibreur"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Script Lua")
#define TR_SF_BG_MUSIC                 "Musique"
#define TR_SF_BG_MUSIC_PAUSE           "Pause Musique"
#define TR_SF_LOGS                     "Logs SD"
#define TR_ADJUST_GVAR                 "Ajuster"
#define TR_SF_BACKLIGHT                "Rétroécl."
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "Test"
#define TR_SF_SAFETY                   TR("Rempl.", "Remplace")

#define TR_SF_SCREENSHOT               "Photo Écran"
#define TR_SF_RACING_MODE              "Racing Mode"
#define TR_SF_DISABLE_TOUCH            "Non Tactile"
#define TR_SF_DISABLE_AUDIO_AMP        "Désact. Ampli Audio"
#define TR_SF_SET_SCREEN               "Définir Écran Princ."
#define TR_SF_SET_TELEMSCREEN          "Aff. écran"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Télem.", "Télémétrie")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS            "Chr1","Chr2","Chr3"


#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,"Tout",TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS
#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#if LCD_W >= 212
#else
#endif

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "km/h"

#define TR_VUNITSSYSTEM                TR("Métr.","Métriques"),TR("Impér.","Impériales")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","km/h","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Rien","Valeurs","Barres","Script"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "--- ","120","120X","140","90"

#define TR_STICK_NAMES0                "Dir"
#define TR_STICK_NAMES1                "Prf"
#define TR_STICK_NAMES2                "Gaz"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "Dir"
#define TR_SURFACE_NAMES1              "Gaz"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Défaut","Haut"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ON","Un"

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "Mode joystick"
#define TR_HATSOPT                     "Trims uniquement","Touches uniquement","Commutable","Global"
#define TR_HATSMODE_TRIMS              "Mode joystick: trims"
#define TR_HATSMODE_KEYS               "Mode joystick: touches"
#define TR_HATSMODE_KEYS_HELP          "Left side:\n"\
                                       "   Right = MDL\n"\
                                       "   Up = SYS\n"\
                                       "   Down = TELE\n"\
                                       "\n"\
                                       "Right side:\n"\
                                       "   Left = PAGE<\n"\
                                       "   Right = PAGE>\n"\
                                       "   Up = PREV/INC\n"\
                                       "   Down = NEXT/DEC"
#endif

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Normal","Inversé"
#else
  #define TR_ROTARY_ENC_OPT         "Normal","Inversé","V-I H-N","V-I H-A","V-N E-I"
#endif

#if defined(IMU)
  #define TR_IMU_VSRCRAW               "TltX","TltY",
#else
  #define TR_IMU_VSRCRAW
#endif

#if defined(HELI)
  #define TR_CYC_VSRCRAW               "CYC1","CYC2","CYC3"
#else
  #define TR_CYC_VSRCRAW               "[C1]","[C2]","[C3]"
#endif


#define TR_SRC_BATT                    "Batt"
#define TR_SRC_TIME                    "H_M"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Chrono"

#define TR_VTMRMODES                   "OFF","ON","GZs","GZ%","GZt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Maître/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Élève/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Maître/SBUS Module"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Maître/CPPM Module"
#define TR_VTRAINER_MASTER_BATTERY     "Maître/Série"
#define TR_VTRAINER_BLUETOOTH          TR("Maître/BT","Maître/Bluetooth"),TR("Élève/BT","Élève/Bluetooth")
#define TR_VTRAINER_MULTI              "Maître/Multi"
#define TR_VFAILSAFE                   "Pas déf.","Maintien",TR("Prédéf.","Prédéfini"),"Pas d'imp","Récepteur"
#define TR_VSENSORTYPES                "Perso","Calculé"
#define TR_VFORMULAS                   "Addition","Moyenne","Min","Max","Multipl.","Totalise","Élém.","Consomm.","Distance"
#define TR_VPREC                       "0.--","0.0","0.00"
#define TR_VCELLINDEX                  "Mini.","1","2","3","4","5","6","7","8","Maxi.","Diff."
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (centre seulement)","= (symétrique)"
#define TR_TIMER_DIR                   TR("Reste", "Voir restant"), TR("Écoulé", "Voir écoulé")
#define TR_PPMUNIT                     "0.--","0.0","us"

// ZERO TERMINATED STRINGS

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "STD"
#else
#define TR_FONT_SIZES                  "STD","GRAS","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14) || defined(PCBPL18)
  #define TR_ENTER                     "[SUIVANT]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#if defined(PCBHORUS)
  #define TR_EXIT                      "RTN"
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "QUITTER"
  #define TR_OK                        "\010" "\010" "\010" TR("[OK]", "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_MENUWHENDONE                TR_ENTER " QUAND PRÊT"
#define TR_FREE                        "disp."
#define TR_YES                         "Oui"
#define TR_NO                          "Non"
#define TR_DELETEMODEL                 "SUPPR. MODÈLE"
#define TR_COPYINGMODEL                "Copie modèle"
#define TR_MOVINGMODEL                 "Déplacement modèle"
#define TR_LOADINGMODEL                "Chargement modèle"
#define TR_UNLABELEDMODEL              "Non catégorisé"
#define TR_NAME                        "Nom"
#define TR_MODELNAME                   TR("Nom Modèle", "Nom du Modèle")
#define TR_PHASENAME                   "Nom Phase"
#define TR_MIXNAME                     TR("Nom mixeur", "Nom du mixeur")
#define TR_INPUTNAME                   TR("Entrée", "Nom entrée")
#define TR_EXPONAME                    TR("Nom", "Nom ligne")
#define TR_BITMAP                      "Image Modèle"
#define TR_NO_PICTURE                  "Pas d'image"
#define TR_TIMER                       "Chrono "
#define TR_START                       "Start"
#define TR_ELIMITS                     TR("Limites ét.", "Limites étendues")
#define TR_ETRIMS                      TR("Trims ét.", "Trims étendus")
#define TR_TRIMINC                     TR("Pas Trim", "Pas du trim")
#define TR_DISPLAY_TRIMS               TR("Aff. trims", "Affichage trims")
#define TR_TTRACE                      TR("Source gaz", "Source")
#define TR_TTRIM                       TR("Trim gaz", "Trim ralenti uniq.")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", "Trim switch")
#define TR_BEEPCTR                     TR("Ctr Beep", "Bip quand centré")
#define TR_USE_GLOBAL_FUNCS            TR("Fonc. glob.", "Fonct. Globales")
#define TR_PROTOCOL                    TR("Proto.", "Protocole")
#define TR_PPMFRAME                    "Trame PPM"
#define TR_REFRESHRATE                 "Période"
#define STR_WARN_BATTVOLTAGE           TR("Signal is VBAT: ", "ATTENTION: niveau sortie est VBAT: ")
#define TR_WARN_5VOLTS                 "Attention: niveau de sortie 5 volts"
#define TR_MS                          "ms"
#define TR_SWITCH                      TR("Inter", "Interrupteur")
#define TR_FUNCTION_SWITCHES           "Inters paramétrables"
#define TR_GROUP                       "Group"
#define TR_GROUP_ALWAYS_ON             "Always on"
#define TR_GROUPS                      "Always on groups"
#define TR_LAST                        "Last"
#define TR_MORE_INFO                   "More info"
#define TR_SWITCH_TYPE                 "Type"
#define TR_SWITCH_STARTUP              "Startup"
#define TR_SWITCH_GROUP                "Group"
#define TR_FUNCTION_SWITCH_GROUPS      "---", TR_SWITCH_GROUP" 1", TR_SWITCH_GROUP" 2", TR_SWITCH_GROUP" 3"
#define TR_SF_SWITCH                   "Interrupteur"
#define TR_TRIMS                       "Trims"
#define TR_FADEIN                      "Fondu ON"
#define TR_FADEOUT                     "Fondu OFF"
#define TR_DEFAULT                     "(défaut)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Vérif. Trims"
#else
  #define TR_CHECKTRIMS                "\006Vérif\012Trims"
#endif
#define TR_SWASHTYPE                   "Type de plateau"
#define TR_COLLECTIVE                  TR("Collectif", "Source collectif")
#define TR_AILERON                     "Source cyc. lat."
#define TR_ELEVATOR                    "Source cyc. lon."
#define TR_SWASHRING                   TR("Limite Cycl.", "Limite du cyclique")
#define TR_MODE                        "Mode"
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "Gauche"
#else
  #define TR_LEFT_STICK                "Gauche"
#endif
#define TR_SUBTYPE                     "Sous-type"
#define TR_NOFREEEXPO                  "Max expos atteint!"
#define TR_NOFREEMIXER                 "Max mixages atteint!"
#define TR_SOURCE                      "Source"
#define TR_WEIGHT                      "Ratio"
#define TR_SIDE                        "Coté"
#define TR_OFFSET                      "Décalage"
#define TR_TRIM                        "Trim"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Courbe"
#define TR_FLMODE                      TR("Phase", "Phases")
#define TR_MIXWARNING                  "Alerte"
#define TR_OFF                         "OFF"
#define TR_ANTENNA                     "Antenne"
#define TR_NO_INFORMATION              TR("Pas d'info", "Pas d'information")
#define TR_MULTPX                      "Opération"
#define TR_DELAYDOWN                   "Retard bas"
#define TR_DELAYUP                     "Retard haut"
#define TR_SLOWDOWN                    "Ralenti bas"
#define TR_SLOWUP                      "Ralenti haut"
#define TR_MIXES                       "MIXEUR"
#define TR_CV                          "CV"
#if defined(PCBNV14) || defined(PCBPL18)
#define TR_GV                          "VG"
#else
#define TR_GV                          TR("G", "VG")
#endif
#define TR_RANGE                       "Plage"
#define TR_CENTER                      "Centre"
#define TR_ALARM                       "Alarme"
#define TR_BLADES                      "Pales/Poles"
#define TR_SCREEN                      "Écran "
#define TR_SOUND_LABEL                 "Son"
#define TR_LENGTH                      "Durée"
#define TR_BEEP_LENGTH                 "Durée bips"
#define TR_BEEP_PITCH                  "Tonalité"
#define TR_HAPTIC_LABEL                "Vibreur"
#define TR_STRENGTH                    "Force"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Décalage"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Contraste"
#define TR_ALARMS_LABEL                "Alarmes"
#define TR_BATTERY_RANGE               "Plage batterie"
#define TR_BATTERYCHARGING             "En charge..."
#define TR_BATTERYFULL                 "Batterie chargée"
#define TR_BATTERYNONE                 "Pas de batterie !"
#define TR_BATTERYWARNING              TR("Batterie", "Batterie faible")
#define TR_INACTIVITYALARM             "Inactivité"
#define TR_MEMORYWARNING               "Mémoire pleine"
#define TR_ALARMWARNING                TR("Silence", "Sons désact.")
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI extinct.", "Vérif. RSSI à l'extinction")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Utiliser valeur RSSI FlySky sans étalonner")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Trainer shutdown", "Vérifier écolage avant extinction")
#define TR_MODEL_STILL_POWERED         TR("Modèle alimenté", "Modèle encore alimenté")
#define TR_TRAINER_STILL_CONNECTED     "Élève encore connecté"
#define TR_USB_STILL_CONNECTED         "USB encore connecté"
#define TR_MODEL_SHUTDOWN              "Éteindre ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Appui [Enter] pour confirmer"
#define TR_THROTTLE_LABEL              "Gaz"
#define TR_THROTTLE_START              "Gaz Start"
#define TR_THROTTLEREVERSE             TR("Inv. gaz", "Inversion gaz")
#define TR_MINUTEBEEP                  TR("Bip min.", "Annonces minutes")
#define TR_BEEPCOUNTDOWN               TR("Bip fin", "Compte à rebours")
#define TR_PERSISTENT                  TR("Persist.", "Persistant")
#define TR_BACKLIGHT_LABEL             "Rétroéclairage"
#define TR_GHOST_MENU_LABEL            "MENU GHOST"
#define TR_STATUS                      "Version"
#define TR_BLONBRIGHTNESS              "Luminosité ON"
#define TR_BLOFFBRIGHTNESS             "Luminosité OFF"
#define TR_KEYS_BACKLIGHT              "Rétroéclairage touches"
#define TR_BLCOLOR                     "Couleur"
#define TR_SPLASHSCREEN                "Ecran d'accueil"
#define TR_PLAY_HELLO                  "Son de démarrage"
#define TR_PWR_ON_DELAY                "Délai btn ON"
#define TR_PWR_OFF_DELAY               "Délai btn OFF"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("Pwr ON/OFF Vibreur","Power ON/OFF Vibreur")
#define TR_THROTTLE_WARNING            TR("Alerte gaz", "Alerte gaz")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Pos. Perso", "Position perso ?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR("Alerte int", "Pos. Interrupteurs")
#define TR_POTWARNINGSTATE             "Pots & Curseurs"
#define TR_POTWARNING                  TR("Alerte pots", "Pos. Potentios")
#define TR_TIMEZONE                    "Fuseau horaire"
#define TR_ADJUST_RTC                  TR("Ajust. RTC", "Ajust. heure auto")
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Ordre ch déf.", "Ordre canaux prédéfinis")
#define TR_STICKS                      "Manches"
#define TR_POTS                        "Potentiomètres"
#define TR_SWITCHES                    "Inters"
#define TR_SWITCHES_DELAY              "Délai inter son"
#define TR_SLAVE                       "Élève"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "Multiplier"
#define TR_CAL                         "Cal"
#define TR_CALIBRATION                 "Calibration"
#define TR_VTRIM                       "Trim - +"
#define TR_CALIB_DONE                  "Calibration terminée"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Appui [Enter] pour commencer"
  #define TR_SETMIDPOINT               "Centrer manches/pots/curseurs puis [Enter]"
  #define TR_MOVESTICKSPOTS            "Bouger manches/pots/curseurs puis [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " POUR DÉBUTER"
  #define TR_SETMIDPOINT               "RÉGLER NEUTRES"
  #define TR_MOVESTICKSPOTS            "BOUGER STICKS/POTS"
#else
  #define TR_MENUTOSTART               TR_ENTER " POUR DÉBUT"
  #define TR_SETMIDPOINT               "REGLER NEUTRES"
  #define TR_MOVESTICKSPOTS            "BOUGER STICKS/POTS"
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      "NO DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"

#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Pile libre"
#define TR_INT_GPS_LABEL               "GPS interne"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua scripts"
#define TR_FREE_MEM_LABEL              "Mémoire libre"
#define TR_DURATION_MS                 TR("[D]","Durée(ms): ")
#define TR_INTERVAL_MS                 TR("[I]","Intervalle(ms): ")
#define TR_MEM_USED_SCRIPT             "Script(B): "
#define TR_MEM_USED_WIDGET             "Widget(B): "
#define TR_MEM_USED_EXTRA              "Extra(B): "
#define TR_STACK_MIX                   "Mixeurs: "
#define TR_STACK_AUDIO                 "Audio: "
#define TR_GPS_FIX_YES                 "Fix: Oui"
#define TR_GPS_FIX_NO                  "Fix: Non"
#define TR_GPS_SATS                    "Sats: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Menu: "
#define TR_TIMER_LABEL                 "Chrono"
#define TR_THROTTLE_PERCENT_LABEL      "Gaz %"
#define TR_BATT_LABEL                  "Batterie"
#define TR_SESSION                     "Session"

#define TR_MENUTORESET                 TR_ENTER" pour RAZ"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODÈLE"
#if defined(SURFACE_RADIO)
#define TR_FM                          "DM"
#else
#define TR_FM                          "PV"
#endif
#define TR_EEPROMLOWMEM                "EEPROM pleine!"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Touche pour ignorer"
#define TR_THROTTLE_NOT_IDLE           "Gaz pas à zéro"
#define TR_ALARMSDISABLED              "Alarmes désactivées"
#define TR_PRESSANYKEY                 "Appuyez sur une touche"
#define TR_BADEEPROMDATA               "EEPROM corrompue"
#define TR_BAD_RADIO_DATA              "Données invalides"
#define TR_RADIO_DATA_RECOVERED        TR3("Utilisation des données radio sauvegardées","Utilisation des paramètres radio sauvegardées","Réglages Radio restaurés de la Sauvegarde")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Réglages Radio invalides","Réglages Radio invalides", "Impossible lire réglages radio valides")
#define TR_EEPROMFORMATTING            "Formatage EEPROM"
#define TR_STORAGE_FORMAT              "Préparation stockage"
#define TR_EEPROMOVERFLOW              "Dépassement EEPROM"
#define TR_RADIO_SETUP                 "CONFIG RADIO"
#define TR_MENUTRAINER                 "ÉCOLAGE"
#define TR_MENUSPECIALFUNCS            "FONCTIONS GLOBALES"
#define TR_MENUVERSION                 "VERSION"
#define TR_MENU_RADIO_SWITCHES         TR("INTERS", "TEST INTERRUPTEURS")
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "ENTRÉES ANALOGIQUES")
#define TR_MENU_RADIO_ANALOGS_CALIB    "ANALOG. CALIBRÉS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION             "CALIBRATION"
#define TR_MENU_FSWITCH                "INTERS PARAMETRABLES"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS             "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE          "Channels=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Channel=>Failsafe"
#define TR_MENUMODELSEL                "MODÈLES"
#define TR_MENU_MODEL_SETUP            TR("CONF. MODÈLE", "CONFIGURATION")
#if defined(SURFACE_RADIO)
#define TR_MENUFLIGHTMODES             "DRIVE MODES"
#define TR_MENUFLIGHTMODE              "DRIVE MODE"
#else
#define TR_MENUFLIGHTMODE              "PHASE DE VOL"
#define TR_MENUFLIGHTMODES             "PHASES DE VOL"
#endif
#define TR_MENUHELISETUP               TR("CONF.HÉLI", "CONFIGURATION HÉLICO")

  #define TR_MENUINPUTS                "ENTRÉES"
  #define TR_MENULIMITS                "SORTIES"

#define TR_MENUCURVES                  "COURBES"
#define TR_MENUCURVE                   "COURBE"
#define TR_MENULOGICALSWITCH           "INTER LOG."
#define TR_MENULOGICALSWITCHES         TR("INTERS LOG.", "INTERS LOGIQUES")
#define TR_MENUCUSTOMFUNC              TR("FONCTIONS SPEC.", "FONCTIONS SPÉCIALES")
#define TR_MENUCUSTOMSCRIPTS           "SCRIPTS PERSOS"
#define TR_MENUTELEMETRY               "TELÉMÉTRIE"
#define TR_MENUSTAT                    TR("STATS", "STATISTIQUES")
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS1           "VOIES 1-8"
#define TR_MONITOR_CHANNELS2           "VOIES 9-16"
#define TR_MONITOR_CHANNELS3           "VOIES 17-24"
#define TR_MONITOR_CHANNELS4           "VOIES 25-32"
#define TR_MONITOR_SWITCHES            "INTERS LOGIQUES"
#define TR_MONITOR_OUTPUT_DESC         "Sorties"
#define TR_MONITOR_MIXER_DESC          "Mixeurs"
#define TR_RECEIVER_NUM                TR("NumRx", "No. Récepteur")
#define TR_RECEIVER                    "Récept."
#define TR_MULTI_RFTUNE                TR("Ajust.fréq", "Ajust. fin fréq.")
#define TR_MULTI_RFPOWER               "Puissance RF"
#define TR_MULTI_WBUS                  "Sortie"
#define TR_MULTI_TELEMETRY             "Télémétrie"
#define TR_MULTI_VIDFREQ               TR("Fréq. vidéo", "Fréquence vidéo")
#define TR_RF_POWER                     TR("Puiss. RF", "Puissance RF")
#define TR_MULTI_FIXEDID               "ID fixe"
#define TR_MULTI_OPTION                TR("Option", "Option perso")
#define TR_MULTI_AUTOBIND              TR("Bind voie", "Bind sur voie")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Désact. réorg. voies")
#define TR_DISABLE_TELEM               TR("Pas de Télém.", "Désact. Télémétrie")
#define TR_MULTI_LOWPOWER              TR("Basse puis.", "Mode basse puiss.")
#define TR_MULTI_LNA_DISABLE           "LNA désactivé"
#define TR_MODULE_TELEMETRY            TR("S.Port", "Lien S.Port")
#define TR_MODULE_TELEM_ON             TR("ON", "Actif")
#define TR_DISABLE_INTERNAL            TR("Désact intRF", "Désact. RF interne")
#define TR_MODULE_NO_SERIAL_MODE       TR("Mode série?", "Pas en mode série")
#define TR_MODULE_NO_INPUT             TR("Pas de sign.", "Aucun signal série")
#define TR_MODULE_NO_TELEMETRY         TR3("Pas de télém.", "Télémétrie absente", "Télémétrie absente(act. MULTI_TELEMETRY)")
#define TR_MODULE_WAITFORBIND          "Binder d'abord"
#define TR_MODULE_BINDING              "Bind..."
#define TR_MODULE_UPGRADE_ALERT              "Mise à jour requise"
#define TR_MODULE_UPGRADE              TR("Upg. advised", "MàJ Disponible")
#define TR_REBIND                      "Rebind requis"
#define TR_REG_OK                      "Enregistr. ok"
#define TR_BIND_OK                     "Bind ok"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Télem ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Télem OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Télem ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Télem OFF"
#define TR_PROTOCOL_INVALID            TR("Sél. invalide", "Protocole invalide")
#define TR_MODULE_STATUS               TR("Etat", "État Module")
#define TR_MODULE_SYNC                 TR("Sync", "Statut Proto Sync.")
#define TR_MULTI_SERVOFREQ             TR("Fréq.servo", "Màj fréq. servos")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Activer max. throw")
#define TR_MULTI_RFCHAN                TR("RF Channel", "Sélection canal RF")
#define TR_GPS_COORDS_FORMAT           TR("Coordonnées", "Coordonnées GPS")
#define TR_VARIO                       TR("Vario", "Variomètre")
#define TR_PITCH_AT_ZERO               "Tonalité min"
#define TR_PITCH_AT_MAX                "Tonalité max"
#define TR_REPEAT_AT_ZERO              TR("Répét. zéro", "Répéter zéro")
#define TR_BATT_CALIB                  TR("Calib. batt", "Calib. batterie")
#define TR_CURRENT_CALIB               "Calib. cour"
#define TR_VOLTAGE                     TR("Tension","Source tension")
#define TR_SELECT_MODEL                "Sélection Modèle"
#define TR_MANAGE_MODELS               "GESTION MODÈLES"
#define TR_MODELS                      "Modèles"
#define TR_SELECT_MODE                 "Sélection mode"
#define TR_CREATE_MODEL                "Créer modèle"
#define TR_FAVORITE_LABEL              "Favoris"
#define TR_MODELS_MOVED                "Modèles non utilisés déplacés vers"
#define TR_NEW_MODEL                   "Nouveau Modèle"
#define TR_INVALID_MODEL               "Modèle Invalide "
#define TR_EDIT_LABELS                 "Éditer catégorie"
#define TR_LABEL_MODEL                 "Catégorie Modèle"
#define TR_MOVE_UP                     "Déplacer -> haut"
#define TR_MOVE_DOWN                   "Déplacer -> bas"
#define TR_ENTER_LABEL                 "Entrer Catégorie"
#define TR_LABEL                       "Catégorie"
#define TR_LABELS                      "Catégories"
#define TR_CURRENT_MODEL               "Courant"
#define TR_ACTIVE                      "Actif"
#define TR_NEW                         "Nouveau"
#define TR_NEW_LABEL                   "Nouv. Catégorie"
#define TR_RENAME_LABEL                "Renom. Catégorie"
#define TR_DELETE_LABEL                "Effacer Catégorie"
#define TR_DUPLICATE_MODEL             "Dupliquer Modèle"
#define TR_COPY_MODEL                  "Copier Modèle"
#define TR_MOVE_MODEL                  "Déplacer Modèle"
#define TR_BACKUP_MODEL                "Archiver Modèle"
#define TR_DELETE_MODEL                "Supprimer Modèle"
#define TR_RESTORE_MODEL               "Restaurer Modèle"
#define TR_DELETE_ERROR                "Effacement impossible"
#define TR_SDCARD_ERROR                "Erreur carte SD"
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "Pas de fichiers sur SD!"
#define TR_NO_SDCARD                   "Pas de Carte SD"
#define TR_WAITING_FOR_RX              "Attente du RX..."
#define TR_WAITING_FOR_TX              "Attente du TX..."
#define TR_WAITING_FOR_MODULE          TR("Attente module", "Attente du module...")
#define TR_NO_TOOLS                    "Pas d'outils dispo."
#define TR_NORMAL                      "Normal"
#define TR_NOT_INVERTED                "Non inv."
#define TR_NOT_CONNECTED               "!Connecté"
#define TR_CONNECTED                   "Connecté"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH sans télém.", "16CH sans télémétrie")
#define TR_16CH_WITH_TELEMETRY         TR("16CH avec télém.", "16CH avec télémétrie")
#define TR_8CH_WITH_TELEMETRY          TR("8CH avec télém.", "8CH avec télémétrie")
#define TR_EXT_ANTENNA                 "Antenne Ext."
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Sauver options RX?"
#define TR_UPDATE_TX_OPTIONS           "Sauver options TX?"
#define TR_MODULES_RX_VERSION          "Versions modules / RX"
#define TR_SHOW_MIXER_MONITORS         "Voir monit. mixeurs"
#define TR_MENU_MODULES_RX_VERSION     "VERSIONS MODULES / RX "
#define TR_MENU_FIRM_OPTIONS           "OPTIONS FIRMWARE"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Manches/Pots/Inters"
#define TR_PWM_STICKS_POTS_SLIDERS     "Manches PWM/Pots/Inters"
#define TR_RF_PROTOCOL                 "Protocole RF"
#define TR_MODULE_OPTIONS              "Options Module"
#define TR_POWER                       "Puissance"
#define TR_NO_TX_OPTIONS               "Pas d'options TX"
#define TR_RTC_BATT                    "Pile RTC"
#define TR_POWER_METER_EXT             "Puissancemètre (EXT)"
#define TR_POWER_METER_INT             "Puissancemètre (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       TR("Spectre (EXT)", "Analyseur Spectre (EXT)")
#define TR_SPECTRUM_ANALYSER_INT       TR("Spectre (INT)", "Analyseur Spectre (INT)")
#define TR_SDCARD_FULL                 "SD carte pleine"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\nJournaux et Impr. écran désactivé"
#else
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036Journaux et" LCDW_128_LINEBREAK "Impr. écran désactivé"
#endif
#define TR_NEEDS_FILE                  "FICHIER EXIGE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Incompatible"
#define TR_WARNING                     "ALERTE"
#define TR_STORAGE_WARNING             "STOCKAGE"
#define TR_THROTTLE_UPPERCASE          "GAZ"
#define TR_ALARMSWARN                  "SON"
#define TR_SWITCHWARN                  TR("INTERS","CONTRÔLES")
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE                "Version de test uniq."
#define TR_WRONG_SDCARDVERSION         "Version requise: "
#define TR_WARN_RTC_BATTERY_LOW        "Pile RTC faible"
#define TR_WARN_MULTI_LOWPOWER         "Mode basse puiss."
#define TR_BATTERY                     "BATTERIE"
#define TR_WRONG_PCBREV                "PCB incorrect détecté"
#define TR_EMERGENCY_MODE              "MODE SECOURS"
#define TR_NO_FAILSAFE                 TR3("Failsafe pas déf.", "Failsafe pas déf.", "Failsafe pas défini")
#define TR_KEYSTUCK                    "Touche bloquée"
#define TR_VOLUME                      "Volume"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Luminosité"
#define TR_CPU_TEMP                    "Temp. CPU\016>"
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "Temp. CM \016>"
#define TR_TTL_WARNING                 "Attention: Ne pas excéder 3.3V aux pins TX/RX !"
#define TR_FUNC                        "Fonction"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Durée"
#define TR_DELAY                       "Délai"
#define TR_SD_CARD                     "Carte SD"
#define TR_SDHC_CARD                   "Carte SD-HC"
#define TR_NO_SOUNDS_ON_SD             "Aucun son sur SD"
#define TR_NO_MODELS_ON_SD             "Aucun modèle SD"
#define TR_NO_BITMAPS_ON_SD            "Aucun Bitmap SD"
#define TR_NO_SCRIPTS_ON_SD            "Aucun Script SD"
#define TR_SCRIPT_SYNTAX_ERROR          TR("Erreur syntaxe", "Erreur syntaxe script")
#define TR_SCRIPT_PANIC                "Script bloqué"
#define TR_SCRIPT_KILLED               "Script interrompu"
#define TR_SCRIPT_ERROR                "Erreur inconnue"
#define TR_PLAY_FILE                   "Lire"
#define TR_DELETE_FILE                 "Supprimer"
#define TR_COPY_FILE                   "Copier"
#define TR_RENAME_FILE                 "Renommer"
#define TR_ASSIGN_BITMAP               "Assigner image"
#define TR_ASSIGN_SPLASH               "Logo d'accueil"
#define TR_EXECUTE_FILE                "Exécuter"
#define TR_REMOVED                     " supprimé"
#define TR_SD_INFO                     "Information"
#define TR_NA                          "N/D"
#define TR_HARDWARE                    "MATÉRIEL"
#define TR_FORMATTING                  "Formatage..."
#define TR_TEMP_CALIB                  "Calib. temp"
#define TR_TIME                        "Heure"
#define TR_MAXBAUDRATE                 "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE                 "Mode"
#define TR_SAMPLE_MODES                "Normal","OneBit"
#define TR_LOADING                     "Chargement..."
#define TR_DELETE_THEME                "Supprimer Thème?"
#define TR_SAVE_THEME                  "Sauvegarder Thème?"
#define TR_EDIT_COLOR                  "Editer Couleur"
#define TR_NO_THEME_IMAGE              "Pas d'image de thème"
#define TR_BACKLIGHT_TIMER             "Durée du rétroéclairage"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Sélect. rapide modèle"
  #define TR_LABELS_SELECT             "Label select"
  #define TR_LABELS_MATCH              "Label matching"
  #define TR_FAV_MATCH                 "Favorites matching"
  #define TR_LABELS_SELECT_MODE        "Multi select","Single select"
  #define TR_LABELS_MATCH_MODE         "Match all","Match any"
  #define TR_FAV_MATCH_MODE            "Must match","Optional match"
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "Sélect. dossier de modèles"
#define TR_SELECT_TEMPLATE             "SÉLECTIONNEZ UN EXEMPLE DE MODÈLE:"
#define TR_NO_TEMPLATES                "Aucun ex. de modèle trouvé dans ce dossier"
#define TR_SAVE_TEMPLATE               "Sauvegarder comme exemple"
#define TR_BLANK_MODEL                 "Modèle vierge"
#define TR_BLANK_MODEL_INFO            "Création modèle vierge"
#define TR_FILE_EXISTS                 "FICHIER DÉJÀ EXISTANT"
#define TR_ASK_OVERWRITE               "Voulez-vous écraser?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Découvrir"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Adr. dist."
#define TR_BLUETOOTH_LOCAL_ADDR        "Adr. locale"
#define TR_BLUETOOTH_PIN_CODE          "Code PIN"
#define TR_BLUETOOTH_NODEVICES         "Aucun périph. trouvé"
#define TR_BLUETOOTH_SCANNING          "Scanning..."
#define TR_BLUETOOTH_BAUDRATE          "Baudrate BT"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Activé"
#else
#define TR_BLUETOOTH_MODES             "---","Télémétrie","Écolage"
#endif
#define TR_SD_INFO_TITLE               "INFO SD"
#define TR_SD_SPEED                    "Vitesse:"
#define TR_SD_SECTORS                  "Secteurs:"
#define TR_SD_SIZE                     "Taille:"
#define TR_TYPE                        "Type"
#define TR_GLOBAL_VARS                 "Variables Globales"
#define TR_GVARS                       "V. GLOBALES"
#define TR_GLOBAL_VAR                  "Variable globale"
#define TR_MENU_GLOBAL_VARS            "VARIABLES GLOBALES"
#define TR_OWN                         "Perso"
#define TR_DATE                        "Date"
#define TR_MONTHS                      { "Jan", "Fév", "Mar", "Avr", "Mai", "Jun", "Jul", "Aou", "Sep", "Oct", "Nov", "Dec" }
#define TR_ROTARY_ENCODER              "Enc.Rot."
#define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Mode Encod. Rotatif")
#define TR_CHANNELS_MONITOR            "MONITEUR CANAUX"
#define TR_MIXERS_MONITOR              "MONITEUR MIXAGES "
#define TR_PATH_TOO_LONG               "Chemin trop long"
#define TR_VIEW_TEXT                   "Voir texte"
#define TR_FLASH_BOOTLOADER            "Flasher BootLoader"
#define TR_FLASH_DEVICE                TR("Flash device","Flasher Périph.")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Flash S.Port", "Flasher S.Port ext.")
#define TR_FLASH_RECEIVER_OTA          "Flasher RX via OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flasher RX par ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flasher RX par int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flasher via OTA Ext."
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flasher via OTA Int"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash module BT", "Flasher Module Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Flasher pwr unit. mngt"
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "Pas de réponse du Périph.")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Prob. fichier Périph.")
#define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "Data Périph. refusés")
#define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "Prob. accès au Périph.")
#define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "Fichier Périph. refusé")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", " Signat. Fich. Périph. Incomp.")
#define TR_CURRENT_VERSION             "Version courante :"
#define TR_FLASH_INTERNAL_MODULE       TR("Flash int. Module", "Flasher module interne")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Int. Multi", "Flasher Module Interne")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash ext. module", "Flasher module Externe")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Ext. Multi", "Flasher Mod Ext.")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flasher ELRS Ext. ")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Erreur màj FW","Erreur de mise à jour")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Flash ok"
#define TR_WRITING                     "Écriture..."
#define TR_CONFIRM_FORMAT              "Confirmer Formatage?"
#define TR_INTERNALRF                  "HF interne"
#define TR_INTERNAL_MODULE             TR("Module int.","Module interne")
#define TR_EXTERNAL_MODULE             TR("Module ext.","Module externe")
#define TR_EDGETX_UPGRADE_REQUIRED     "Màj EdgeTX nécessaire"
#define TR_TELEMETRY_DISABLED          "Télém. désactivée"
#define TR_MORE_OPTIONS_AVAILABLE      "Autres options disponibles"
#define TR_NO_MODULE_INFORMATION       "Pas d'info Module"
#define TR_EXTERNALRF                  "HF externe"
#define TR_FAILSAFE                    TR("Failsafe", "Type failsafe")
#define TR_FAILSAFESET                 "RÉGLAGES FAILSAFE"
#define TR_REG_ID                      TR("ID Enr.", "ID Enregistr.")
#define TR_OWNER_ID                    "ID Radio"
#define TR_HOLD                        "Maintien"
#define TR_HOLD_UPPERCASE              "MAINTIEN"
#define TR_NONE                        "Aucun"
#define TR_NONE_UPPERCASE              "AUCUN"
#define TR_MENUSENSOR                  "CAPTEUR"
#define TR_POWERMETER_PEAK             "Pic"
#define TR_POWERMETER_POWER            "Puiss"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Fréq."
#define TR_MENUTOOLS                   "OUTILS"
#define TR_TURN_OFF_RECEIVER           "Éteindre Récepteur."
#define TR_STOPPING                    "Arrêt..."
#define TR_MENU_SPECTRUM_ANALYSER      "ANALYSEUR SPECTRE"
#define TR_MENU_POWER_METER            "PUISSANCEMETRE."
#define TR_SENSOR                      "CAPTEUR"
#define TR_COUNTRY_CODE                 TR("Zone géo.", "Zone géographique")
#define TR_USBMODE                     "Mode USB"
#define TR_JACK_MODE                   "Mode Jack"
#define TR_VOICE_LANGUAGE               TR("Langue voix", "Langue annonces vocales")
#define TR_UNITS_SYSTEM                "Unités"
#define TR_UNITS_PPM                   "PPM Units"
#define TR_EDIT                        "Éditer"
#define TR_INSERT_BEFORE               "Insérer avant"
#define TR_INSERT_AFTER                "Insérer après"
#define TR_COPY                        "Copier"
#define TR_MOVE                        "Déplacer"
#define TR_PASTE                       "Coller"
#define TR_PASTE_AFTER                 "Coller Après"
#define TR_PASTE_BEFORE                "Coller Avant"
#define TR_DELETE                      "Supprimer"
#define TR_INSERT                      "Insérer"
#define TR_RESET_FLIGHT                TR("Réinit. vol", "Réinit. vol")
#define TR_RESET_TIMER1                TR("Réinit. Timer1", "Réinit. Chrono1")
#define TR_RESET_TIMER2                TR("Réinit. Timer2", "Réinit. Chrono2")
#define TR_RESET_TIMER3                TR("Réinit. Timer3", "Réinit. Chrono3")
#define TR_RESET_TELEMETRY             TR("Réinit. Télém.", "Réinit. Télém.")
#define TR_STATISTICS                  "Statistiques"
#define TR_ABOUT_US                    "A propos"
#define TR_USB_JOYSTICK                "Joystick USB (HID)"
#define TR_USB_MASS_STORAGE            "Stockage USB (SD)"
#define TR_USB_SERIAL                  "Port série (VCP)"
#define TR_SETUP_SCREENS               "Config. Écrans"
#define TR_MONITOR_SCREENS             "Moniteurs"
#define TR_AND_SWITCH                  "ET suppl."
#define TR_SF                          "FS"
#define TR_GF                          "FG"
#define TR_ANADIAGS_CALIB              "Analog. calibrés"
#define TR_ANADIAGS_FILTRAWDEV         "Analog. bruts filtrés avec déviation"
#define TR_ANADIAGS_UNFILTRAW          "Analog. bruts non filtrés"
#define TR_ANADIAGS_MINMAX             "Min., max. et courses"
#define TR_ANADIAGS_MOVE               "Déplacer les analogiques à leurs extrêmes!"
#define TR_SPEAKER                     "HP."
#define TR_BUZZER                      "Bipeur"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_MODULE_UNBIND               BUTTON("Délier")
#define TR_POWERMETER_ATTN_NEEDED      "Atténuateur requis"
#define TR_PXX2_SELECT_RX              "Sélection RX..."
#define TR_PXX2_DEFAULT                "<défaut>"
#define TR_BT_SELECT_DEVICE            "Sélect. périph."
#define TR_DISCOVER                    "Découvrir"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Attente..."
#define TR_RECEIVER_DELETE             "Suppr récept.?"
#define TR_RECEIVER_RESET              "Réinit. récept.?"
#define TR_SHARE                       "Partager"
#define TR_BIND                        "Bind"
#define TR_REGISTER                    TR("Enr", "Enregistr.")
#define TR_MODULE_RANGE                BUTTON(TR("Prt", "Port."))
#define TR_RANGE_TEST                  "Test de portée"
#define TR_RECEIVER_OPTIONS            TR("OPTIONS REC.", "OPTIONS RÉCEPTEUR")
#define TR_RESET_BTN                   BUTTON("RAZ")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON(TR("Btns","Inters"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "Analogs"))
#define TR_FS_BTN                      BUTTON(TR("Custom sw", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Écran tactile non trouvé"
#define TR_TOUCH_EXIT                  "Toucher écran pour quitter"
#define TR_SET                         BUTTON("Déf")
#define TR_TRAINER                     "Écolage"
#define TR_CHANS                       "Aléat."
#define TR_ANTENNAPROBLEM              "Antenne radio défect.!"
#define TR_MODELIDUSED                 TR("ID affecté à :", "No de récepteur utilisé par :")
#define TR_MODELIDUNIQUE               "ID est unique"
#define TR_MODULE                      "Module"
#define TR_RX_NAME                     "Nom Rx"
#define TR_TELEMETRY_TYPE              TR("Type tél.", "Type télémétrie")
#define TR_TELEMETRY_SENSORS           "Capteurs"
#define TR_VALUE                       "Valeur"
#define TR_PERIOD                      "Période"
#define TR_INTERVAL                    "Intervalle"
#define TR_REPEAT                      "Répéter"
#define TR_ENABLE                      "Activer"
#define TR_DISABLE                     "Désactiver"
#define TR_TOPLCDTIMER                 "Chrono LCD haut"
#define TR_UNIT                        "Unité"
#define TR_TELEMETRY_NEWSENSOR         TR("Nouveau capteur...", "Ajouter nouveau")
#define TR_CHANNELRANGE                TR("Canaux", "Plage de canaux")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "Fréquence RX")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetrie")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Puissance actuelle")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Source puissance")
#define TR_ANTENNACONFIRM1             "Vraiment changer?"
#if defined(PCBX12S)
#define TR_ANTENNA_MODES               "Interne","Demander","Par modèle","Interne + Externe"
#else
#define TR_ANTENNA_MODES               "Interne","Demander","Par modèle","Externe"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Util antenne int", "Utiliser antenne interne")
#define TR_USE_EXTERNAL_ANTENNA        TR("Util antenne ext", "Utiliser antenne externe")
#define TR_ANTENNACONFIRM2             TR("Vérif antenne", "Installer l'antenne d'abord!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1        "Nécessite FLEX"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1         "Nécessite FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1          "Nécessite EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2             "R9M non certifié"
#define TR_LOWALARM                    "Alarme basse"
#define TR_CRITICALALARM               "Alarme critique"
#define TR_DISABLE_ALARM               TR("Désact. alarme", "Désact. alarme télém.")
#define TR_POPUP                       "Popup"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Courbe standard..."
#define TR_PRESET                      "Pente"
#define TR_MIRROR                      "Miroir"
#define TR_CLEAR                       "Effacer"
#define TR_RESET                       "Réinit."
#define TR_RESET_SUBMENU               "Réinit..."
#define TR_COUNT                       "Nb points"
#define TR_PT                          "pt"
#define TR_PTS                         "pts"
#define TR_SMOOTH                      "Lissage"
#define TR_COPY_STICKS_TO_OFS          TR("Cpy stick->subtrim", "Copier Manche-> subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copier min/max/centre-> toutes sorties")
#define TR_COPY_TRIMS_TO_OFS           TR("Cpy trim->subtrim", "Trim vers subtrim")
#define TR_INCDEC                      "Inc/décrémenter"
#define TR_GLOBALVAR                   "Var. globale"
#define TR_MIXSOURCE                   "Source (%)"
#define TR_MIXSOURCERAW                "Source (valeur)"
#define TR_CONSTANT                    "Constante"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "Off","On","Auto"
#define TR_PREFLIGHT                   "Vérifs avant vol"
#define TR_CHECKLIST                   TR("checklist", "Afficher checklist")
#define TR_CHECKLIST_INTERACTIVE       TR3("C-Interact", "Interact. checklist", "Interactive checklist")
#define TR_AUX_SERIAL_MODE             "Port série"
#define TR_AUX2_SERIAL_MODE            "Port série 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port power"
#define TR_SCRIPT                      "Script"
#define TR_INPUTS                      "Entrées"
#define TR_OUTPUTS                     "Sorties"
#if defined(COLORLCD)
#define STR_EEBACKUP                   "Sauvegarder l'EEPROM"
#define STR_FACTORYRESET               "RAZ d'usine"
#elif defined(PCBXLITE)
#define STR_EEBACKUP                   "Sauvegarde"
#define STR_FACTORYRESET               "RAZ usine"
#else
#define STR_EEBACKUP                   TR("Sauvegarde", "Sauvegarder l'EEPROM")
#define STR_FACTORYRESET               "RAZ d'usine"
#endif
#define TR_CONFIRMRESET                TR("Effacer TOUT?","Effacer TOUS modèles/réglages?")
#define TR_TOO_MANY_LUA_SCRIPTS         "Trop de scripts lua!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "Pas d’Écrans de Télém."
#define TR_TOUCH_PANEL                 "Écran Tactile:"
#define TR_FILE_SIZE                   "Taille fichier"
#define TR_FILE_OPEN                   "Ouvrir quand même?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Nom"
#define TR_PHASES_HEADERS_SW           "Inter"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trim Dir."
#define TR_PHASES_HEADERS_ELE_TRIM     "Trim Prf."
#define TR_PHASES_HEADERS_THT_TRIM     "Trim Gaz"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trim Ail."
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Fondu ON"
#define TR_PHASES_HEADERS_FAD_OUT      "Fondu OFF"

#define TR_LIMITS_HEADERS_NAME         "Nom"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Direction"
#define TR_LIMITS_HEADERS_CURVE        "Courbe"
#define TR_LIMITS_HEADERS_PPMCENTER    "Neutre PPM"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Mode subtrim"
#define TR_INVERTED                    "Inversé"


#define TR_LSW_DESCRIPTIONS            { "Type de comparaison ou fonction", "Première variable", "Seconde variable ou constante", "Seconde variable ou constante", "Condition supplémentaire pour activer la ligne", "Durée minimale d'activation de l'inter logique", "Durée min de la condition avant l'activation de l'inter" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "Premier Canal"
  #define TR_FILL_BACKGROUND           "Remplir arrière-plan?"
  #define TR_BG_COLOR                  "BG Color"
  #define TR_SLIDERS_TRIMS             "Curseurs+Trims"
  #define TR_SLIDERS                   "Curseurs"
  #define TR_FLIGHT_MODE               "Mode de vol"
  #define TR_INVALID_FILE              "Fichier invalide"
  #define TR_TIMER_SOURCE              "Source Chrono"
  #define TR_SIZE                      "Taille"
  #define TR_SHADOW                    "Ombre"
  #define TR_ALIGNMENT                 "Alignement"
  #define TR_ALIGN_LABEL               "Aligner Catégorie"
  #define TR_ALIGN_VALUE               "Aligner Valeur"
  #define TR_ALIGN_OPTS                { "Gauche", "Centre", "Droite" }
  #define TR_TEXT                      "Texte"
  #define TR_COLOR                     "Couleur"
  #define TR_MAIN_VIEW_X               "Vue Principale XX"
  #define TR_PANEL1_BACKGROUND         "Arrière-plan Ecran1"
  #define TR_PANEL2_BACKGROUND         "Arrière-plan Ecran2"
  #define TR_WIDGET_GAUGE              "Jauge"
  #define TR_WIDGET_MODELBMP           "Modèle"
  #define TR_WIDGET_OUTPUTS            "Sorties"
  #define TR_WIDGET_TEXT               "Texte"
  #define TR_WIDGET_TIMER              "Chrono"
  #define TR_WIDGET_VALUE              "Valeur"
#endif

// Bootloader common - ASCII characters only
#define TR_BL_USB_CONNECTED           "USB Connecte"
#define TR_BL_USB_PLUGIN              "ou branchez cable USB"
#define TR_BL_USB_MASS_STORE          "pour stockage de masse"
#define TR_BL_USB_PLUGIN_MASS_STORE   "ou branchez cable USB pour stockage de masse"
#define TR_BL_WRITE_FW                "Ecriture Firmware"
#define TR_BL_FORK                    "Fork:"
#define TR_BL_VERSION                 "Version:"
#define TR_BL_RADIO                   "Radio:"
#define TR_BL_EXIT                    "Quitter"
#define TR_BL_DIR_MISSING             " Repertoire absent"
#define TR_BL_DIR_EMPTY               " Repertoire vide"
#define TR_BL_WRITING_FW              "Ecriture Firmware ..."
#define TR_BL_WRITING_COMPL           "Ecriture terminée"

#if LCD_W >= 480
  #define TR_BL_INVALID_FIRMWARE       "Fichier firmware non valide"
#elif LCD_W >= 212
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN_MASS_STORE
  #define TR_BL_HOLD_ENTER_TO_START    "\012Appui [ENT] pour demarrer ecriture"
  #define TR_BL_INVALID_FIRMWARE       "\011 Fichier firmware non valide!        "
  #define TR_BL_INVALID_EEPROM         "\011Fichier EEPROM non valide !          "
#else
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
  #define TR_BL_HOLD_ENTER_TO_START    "\006Appui [ENT] pour demarrer"
  #define TR_BL_INVALID_FIRMWARE       "\004Firmware non valide!        "
  #define TR_BL_INVALID_EEPROM         "\004EEPROM non valide !          "
#endif

#if defined(PCBTARANIS)
   // Bootloader Taranis specific - ASCII characters only
  #define TR_BL_RESTORE_EEPROM        "Restaurer EEPROM"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Appuyez sur le bouton power."
    #define TR_BL_FLASH_EXIT          "Quitter mode flashage."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - ASCII characters only
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_SELECT_KEY            "[ENT] pour select. fichier"
  #define TR_BL_FLASH_KEY             "Appui long [ENT] pour flasher"
  #define TR_BL_ERASE_KEY             "Hold [ENT] long to erase"
  #define TR_BL_EXIT_KEY              "[RTN] pour quitter"
#elif defined(PCBPL18)
   // Bootloader PL18/NB4+ specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Firmware actuel:"
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_ENABLE                "Activer"
  #define TR_BL_DISABLE               "Désactiver"
  #if defined(RADIO_NV14_FAMILY)
    #define TR_BL_SELECT_KEY          "[R TRIM] pour sélect. fichier"
    #define TR_BL_FLASH_KEY           "Appui long [R TRIM] pour flasher"
    #define TR_BL_ERASE_KEY           "Appui long  [R TRIM] to erase"
    #define TR_BL_EXIT_KEY            " [L TRIM] pour quitter"
  #elif defined(RADIO_NB4P)
    #define TR_BL_SELECT_KEY          "[SW1A] pour sélect. fichier"
    #define TR_BL_FLASH_KEY           "Appui long [SW1A] pour flasher"
    #define TR_BL_ERASE_KEY           "Appui long [SW1A] to erase"
    #define TR_BL_EXIT_KEY            "[SW1B] pour quitter"
  #else
    #define TR_BL_SELECT_KEY          "[TR4 Dn] pour sélect. fichier"
    #define TR_BL_FLASH_KEY           "Appui long [TR4 Dn] pour flasher"
    #define TR_BL_ERASE_KEY           "Appui long [TR4 Dn] to erase"
    #define TR_BL_EXIT_KEY            "[TR4 U] pour quitter"
  #endif
#elif defined(PCBNV14)
   // Bootloader NV14 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Firmware actuel:"
  #define TR_BL_SELECT_KEY            "[R TRIM] pour sélect. fichier"
  #define TR_BL_FLASH_KEY             "Appui long [R TRIM] pour flasher"
  #define TR_BL_EXIT_KEY              " [L TRIM] pour quitter"
  #define TR_BL_ENABLE                "Activer"
  #define TR_BL_DISABLE               "Désactiver"
#endif

// About screen
#define TR_ABOUTUS                     TR(" A PROPOS ", "A PROPOS")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'E'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Volume bips"
#define TR_WAV_VOLUME                  "Volume audio"
#define TR_BG_VOLUME                   TR("Volume musique", "Volume musique de fond")

#define TR_TOP_BAR                     "Barre titre"
#define TR_FLASH_ERASE                 "Effacer Flash..."
#define TR_FLASH_WRITE                 "Ecrire Flash..."
#define TR_OTA_UPDATE                  "MàJ via OTA ..."
#define TR_MODULE_RESET                "RAZ Module..."
#define TR_UNKNOWN_RX                  "RX inconnu"
#define TR_UNSUPPORTED_RX              "RX non supporté"
#define TR_OTA_UPDATE_ERROR            "Erreur màj OTA"
#define TR_DEVICE_RESET                "RAZ Périphérique..."
#define TR_ALTITUDE                    "Altitude"
#define TR_SCALE                       "Échelle"
#define TR_VIEW_CHANNELS               "Voir voies"
#define TR_VIEW_NOTES                  "Voir notes"
#define TR_MODEL_SELECT                "Sélection Modèle"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Précision"
#define TR_RATIO                       "Ratio"
#define TR_FORMULA                     "Formule"
#define TR_CELLINDEX                   "Index élém."
#define TR_LOGS                        "Logs"
#define TR_OPTIONS                     "Options"
#define TR_FIRMWARE_OPTIONS            "Options Firmware"

#define TR_ALTSENSOR                   "Capteur Alt."
#define TR_CELLSENSOR                  "Capteur Elém."
#define TR_GPSSENSOR                   "Capteur GPS"
#define TR_CURRENTSENSOR               "Capteur"
#define TR_AUTOOFFSET                  "Offset auto"
#define TR_ONLYPOSITIVE                "Positif"
#define TR_FILTER                      "Filtre"
#define TR_TELEMETRYFULL               "Plus de capteurs libres!"
#define TR_IGNORE_INSTANCE             TR("Ign. inst", "Ignorer instance")
#define TR_SHOW_INSTANCE_ID            "Voir instance ID"
#define TR_DISCOVER_SENSORS            "Découvrir capteurs"
#define TR_STOP_DISCOVER_SENSORS       "Terminer découverte"
#define TR_DELETE_ALL_SENSORS          TR("Suppr. tous capteurs", "Suppr. tous capteurs")
#define TR_CONFIRMDELETE               TR("Tout effacer?", "Vraiment tout " LCDW_128_LINEBREAK "effacer ?")
#define TR_SELECT_WIDGET               "Sélection widget"
#define TR_WIDGET_FULLSCREEN           "Plein Écran"
#define TR_REMOVE_WIDGET               "Supprimer widget"
#define TR_WIDGET_SETTINGS             "Réglages widget"
#define TR_REMOVE_SCREEN               "Supprimer écran"
#define TR_SETUP_WIDGETS               "Configurer widgets"
#define TR_USER_INTERFACE              "Interface utilisateur"
#define TR_THEME                       "Thème"
#define TR_SETUP                       "Configuration"
#define TR_LAYOUT                      "Disposition"
#define TR_ADD_MAIN_VIEW               "Ajout. vue principale"
#define TR_TEXT_COLOR                  "Couleur Texte"

#define TR_MENU_INPUTS                  STR_CHAR_INPUT "Entrées"
#define TR_MENU_LUA                     STR_CHAR_LUA "Scripts Lua"
#define TR_MENU_STICKS                   STR_CHAR_STICK "Manches"
#define TR_MENU_POTS                    STR_CHAR_POT "Pots"
#define TR_MENU_MIN                     STR_CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                     STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                    STR_CHAR_CYC "Cyclique"
#define TR_MENU_TRIMS                    STR_CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES                STR_CHAR_SWITCH "Inters"
#define TR_MENU_LOGICAL_SWITCHES        STR_CHAR_SWITCH "Inters logiques"
#define TR_MENU_TRAINER                 STR_CHAR_TRAINER "Écolage"
#define TR_MENU_CHANNELS                STR_CHAR_CHANNEL "Canaux"
#define TR_MENU_GVARS                   STR_CHAR_SLIDER "Var. Glob."
#define TR_MENU_TELEMETRY               STR_CHAR_TELEMETRY "Télémétrie"
#define TR_MENU_DISPLAY                "AFFICHAGE"
#define TR_MENU_OTHER                  "Autres"
#define TR_MENU_INVERT                 "Inverser"
#define TR_AUDIO_MUTE                  TR("Audio muet","Muet si pas de son")
#define TR_JITTER_FILTER               "Filtre ADC"
#define TR_DEAD_ZONE                   "Zone Neutre"
#define TR_RTC_CHECK                   TR("Vérif. RTC", "Vérif. pile RTC")
#define TR_AUTH_FAILURE                "Échec authentification"
#define TR_RACING_MODE                 "Mode Racing"

#undef  STR_SENSOR_BATT
#define STR_SENSOR_BATT                 "BtRx"

// The following content is Untranslated)

#define TR_USE_THEME_COLOR              "Utiliser couleur du thème"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Ajouter tous trims aux sub-trims"
#if !PORTRAIT_LCD
  #define TR_OPEN_CHANNEL_MONITORS        "Ouvrir Monit. de Canal"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Ouvrir Mon. de Canal"
#endif
#define TR_DUPLICATE                    "Dupliquer"
#define TR_ACTIVATE                     "Activer"
#define TR_RED                          "Rouge"
#define TR_BLUE                         "Bleu"
#define TR_GREEN                        "Vert"
#define TR_COLOR_PICKER                 "Pipette à couleurs"
#define TR_EDIT_THEME_DETAILS           "Éditer Détails Thème"
#define TR_THEME_COLOR_DEFAULT          "DEFAUT"
#define TR_THEME_COLOR_PRIMARY1         "PRIMAIRE 1"
#define TR_THEME_COLOR_PRIMARY2         "PRIMAIRE 2"
#define TR_THEME_COLOR_PRIMARY3         "PRIMAIRE 3"
#define TR_THEME_COLOR_SECONDARY1       "SECONDAIRE 1"
#define TR_THEME_COLOR_SECONDARY2       "SECONDAIRE 2"
#define TR_THEME_COLOR_SECONDARY3       "SECONDAIRE 3"
#define TR_THEME_COLOR_FOCUS            "FOCUS"
#define TR_THEME_COLOR_EDIT             "EDITER"
#define TR_THEME_COLOR_ACTIVE           "ACTIF"
#define TR_THEME_COLOR_WARNING          "ATTENTION"
#define TR_THEME_COLOR_DISABLED         "DESACTIVE"
#define TR_THEME_COLOR_CUSTOM           "PERSO"
#define TR_THEME_CHECKBOX               "Case à cocher"
#define TR_THEME_ACTIVE                 "Actif"
#define TR_THEME_REGULAR                "Par défaut"
#define TR_THEME_WARNING                "Attention"
#define TR_THEME_DISABLED               "Désactivé"
#define TR_THEME_EDIT                   "Editer"
#define TR_THEME_FOCUS                  "Focus"
#define TR_AUTHOR                       "Auteur"
#define TR_DESCRIPTION                  "Description"
#define TR_SAVE                         "Sauvegarder"
#define TR_CANCEL                       "Annuler"
#define TR_EDIT_THEME                   "ÉDITER THÈME"
#define TR_DETAILS                      "Détails"
#define TR_THEME_EDITOR                 "THÈMES"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Select\nModèle"
#define TR_MAIN_MENU_MANAGE_MODELS      "Gestion\nModèles"
#define TR_MAIN_MENU_MODEL_NOTES        "Notes\nModèles"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Moniteur\nVoies"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Éditer\nModèle"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Param.\nRadio"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Gestion\nÉcrans"
#define TR_MAIN_MENU_RESET_TELEMETRY    "RAZ\nTélémetrie"
#define TR_MAIN_MENU_STATISTICS         "Stats"
#define TR_MAIN_MENU_ABOUT_EDGETX       "A Propos\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "English"
#define TR_VOICE_CHINESE                "Chinese"
#define TR_VOICE_CZECH                  "Czech"
#define TR_VOICE_DANISH                 "Danish"
#define TR_VOICE_DEUTSCH                "Deutsch"
#define TR_VOICE_DUTCH                  "Dutch"
#define TR_VOICE_ESPANOL                "Espanol"
#define TR_VOICE_FRANCAIS               "Francais"
#define TR_VOICE_HUNGARIAN              "Hungarian"
#define TR_VOICE_ITALIANO               "Italiano"
#define TR_VOICE_POLISH                 "Polish"
#define TR_VOICE_PORTUGUES              "Portugues"
#define TR_VOICE_RUSSIAN                "Russian"
#define TR_VOICE_SLOVAK                 "Slovak"
#define TR_VOICE_SWEDISH                "Swedish"
#define TR_VOICE_TAIWANESE              "Taiwanese"
#define TR_VOICE_JAPANESE               "Japanese"
#define TR_VOICE_HEBREW                 "Hébreu"
#define TR_VOICE_UKRAINIAN               "Ukrainian"

#define TR_USBJOYSTICK_LABEL           "Joystick USB"
#define TR_USBJOYSTICK_EXTMODE         "Mode"
#define TR_VUSBJOYSTICK_EXTMODE        "Classique","Avancé"
#define TR_USBJOYSTICK_SETTINGS        "Paramètre voies"
#define TR_USBJOYSTICK_IF_MODE         TR("If. mode","Mode interface")
#define TR_VUSBJOYSTICK_IF_MODE        "Joystick","Gamepad","MultiAxes"
#define TR_USBJOYSTICK_CH_MODE         "Mode"
#define TR_VUSBJOYSTICK_CH_MODE        "Aucun","Btn","Axes","Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Mode Bouton"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "Normal","Pulse","SWEmu","Delta","Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Norm","Normal"),TR("Puls","Pulse"),TR("SWEm","SWEmul"),TR("Delt","Delta"),TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Positions"
#define TR_VUSBJOYSTICK_CH_SWPOS       "Pousser","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Axes"
#define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","rotX","rotY","rotZ","Curseur","Cadran","Volant"
#define TR_USBJOYSTICK_CH_SIM          "Voies Simu"
#define TR_VUSBJOYSTICK_CH_SIM         "Ail","Prof","Dir","Gaz","Acc","Frein","Gouv","Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Inversion"
#define TR_USBJOYSTICK_CH_BTNNUM       "Bouton no."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Bouton anti collision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Collision des Axes!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Déc. Circ.", "Découpe circulaire")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "Aucun","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Appliquer changement"

#define TR_DIGITAL_SERVO          "Servo 333HZ"
#define TR_ANALOG_SERVO           "Servo 50HZ"
#define TR_SIGNAL_OUTPUT          "Sortie Signal"
#define TR_SERIAL_BUS             "Serial bus"
#define TR_SYNC                   "Sync"

#define TR_ENABLED_FEATURES       TR("Fonctions activées", "Fonctionnalités activées")
#define TR_RADIO_MENU_TABS        "Onglets Menu Radio"
#define TR_MODEL_MENU_TABS        "Onglets Menu Modèles"

#define TR_SELECT_MENU_ALL        "All"
#define TR_SELECT_MENU_CLR        "Clear"
#define TR_SELECT_MENU_INV        "Invert"

#define TR_SORT_ORDERS            "Nom A-Z","Nom Z-A","Dernier utilisé","Couramment utilisé"
#define TR_SORT_MODELS_BY         "Trier modèles par"
#define TR_CREATE_NEW             "Créer"

#define TR_MIX_SLOW_PREC          TR("Slow prec", "Slow up/dn prec")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "Un thème avec le même nom existe déjà."

#define TR_DATE_TIME_WIDGET       "Date & Time"
#define TR_RADIO_INFO_WIDGET      "Radio Info"
#define TR_LOW_BATT_COLOR         "Low battery"
#define TR_MID_BATT_COLOR         "Mid battery"
#define TR_HIGH_BATT_COLOR        "High battery"

#define TR_WIDGET_SIZE            "Taille Widget"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
