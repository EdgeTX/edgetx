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

// FR translations author: Andre Bernet <bernet.andre@gmail.com>

/*
 * Formatting octal codes available in TR_ strings:
 *  \037\x           -sets LCD x-coord (x value in octal)
 *  \036             -newline
 *  \035             -horizontal tab (ARM only)
 *  \001 to \034     -extended spacing (value * FW/2)
 *  \0               -ends current string
 */


// NON ZERO TERMINATED STRINGS
#define TR_OFFON                       "OFF","ON"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Aucun","Alarm","NoKey","Tout"
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VBLMODE                     "OFF",TR("Btns","Touches"),TR("Ctrl","Controles"),"Tous","ON"
#define TR_TRNMODE                     "OFF","+=",":="
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES            "OFF","Recopie Telem","Télémétrie In","Ecolage SBUS","LUA","CLI","GPS","Debug"
#define TR_SWTYPES                     "Rien","Levier","2-POS","3-POS"
#define TR_POTTYPES                    "Rien",TR("Pot av. ctr","Pot avec centre"),TR("Multipos","Inter multi-pos""Potentiomètre"),TR("Pot","Potentiomètre")
#define TR_SLIDERTYPES                 "Rien","Slider"
#define TR_VLCD                        "Normal","Optrex"
#define TR_VPERSISTENT                 "OFF","Vol","Reset Manuel"
#define TR_COUNTRY_CODES               TR("US","USA"),TR("JP","Japon"),TR("Europe","EU")
#define TR_USBMODES                    TR("Popup","Demander"),TR("Joyst","Joystick"),TR("SDCard","Stockage"),"Série"
#define TR_JACK_MODES                  "Demander","Audio","Ecolage"
#define TR_TELEMETRY_PROTOCOLS         "FrSky S.PORT","FrSky D","FrSky D (cable)","TBS Crossfire","Spektrum","AFHDS2A IBUS","Multi Telemetry"

#define TR_SBUS_INVERSION_VALUES       "normal","not inverted"
#define TR_MULTI_TELEMETRY_MODE        "Off","On","Off+Aux","On+Aux"
#define TR_MULTI_CUSTOM                "Perso"
#define TR_VTRIMINC                    TR("Expo","Exponentiel"),TR("ExFin","Extra Fin"),"Fin","Moyen",TR("Gros","Grossier")
#define TR_VDISPLAYTRIMS               "Non","Change","Oui"
#define TR_VBEEPCOUNTDOWN              "Aucun","Bips","Voix","Haptic"
#define TR_VVARIOCENTER                "Tone","Silent"
#define TR_CURVE_TYPES                 "Standard","Libre"

#define TR_ADCFILTERVALUES             "Globale","Off","On"

#if defined(PCBX10)
  #define TR_RETA123                   "D","F","G","A","1","3","2","4","5","6","7","L","R"
#elif defined(PCBHORUS)
  #define TR_RETA123                   "D","P","G","A","1","3","2","4","5","L","R"
#elif defined(PCBX9E)
  #define TR_RETA123                   "D","P","G","A","1","2","3","4","L","R","L","R"
#elif defined(PCBTARANIS)
  #define TR_RETA123                   "D","P","G","A","1","2","3","L","R"
#else
  #define TR_RETA123                   "D","P","G","A","1","2","3"
#endif

#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      TR("Ajoute","Additionner"),TR("Multipl.","Multiplier"),TR("Remplace","Remplacer")
#define TR_VMLTPX2                     "+=","*=",":="

#if defined(PCBHORUS)
  #define TR_VMIXTRIMS                 "OFF","ON","Dir","Prf","Gaz","Ail","T5","T6"
#else
  #define TR_VMIXTRIMS                 "OFF","ON","Dir","Prf","Gaz","Ail"
#endif

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Tempo"
  #define TR_CSWSTICKY                 "Bistb"
  #define TR_CSWRANGE                  "Plage"
  #define TR_CSWSTAY                   "Flanc"
#else
  #define TR_CSWTIMER                  "Temp"
  #define TR_CSWSTICKY                 "Bist"
  #define TR_CSWRANGE                  "Zone"
  #define TR_CSWSTAY                   "Flnc"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a~x","a>x","a<x",TR_CSWRANGE,"|a|>x","|a|<x","ET","OU","OUX",TR_CSWSTAY,"a=b","a>b","a<b","Δ≥x","|Δ|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_TEXT_SIZE                   "Standard","Tiny","Small","Mid","Double"

#if defined(VARIO)
  #define TR_VVARIO                    "Vario"
#else
  #define TR_VVARIO                    "[Vario]"
#endif

#if defined(AUDIO)
  #define TR_SOUND                     "Jouer son"
#else
  #define TR_SOUND                     "Bip"
#endif

#if defined(HAPTIC)
  #define TR_HAPTIC                    "Vibreur"
#else
  #define TR_HAPTIC                    "[Vibreur]"
#endif

#if defined(VOICE)
  #define TR_PLAY_TRACK                "Jouer fich"
  #define TR_PLAY_BOTH                 "Jouer les 2"
  #define TR_PLAY_VALUE                "Lire valeur"
#else
  #define TR_PLAY_TRACK                "[Jouer fich.]"
  #define TR_PLAY_BOTH                 "[Jouer les 2]"
  #define TR_PLAY_VALUE                "[Lire valeur]"
#endif

#define TR_SF_BG_MUSIC                 "Musique","Pause Musique"

#if defined(SDCARD)
  #define TR_SDCLOGS                   "Logs SD"
#else
  #define TR_SDCLOGS                   "[Logs SD]"
#endif

#if defined(GVARS)
  #define TR_ADJUST_GVAR               "Ajuster"
#else
  #define TR_ADJUST_GVAR               "[AjusteGV]"
#endif

#if defined(LUA)
  #define TR_SF_PLAY_SCRIPT            "Script Lua"
#else
  #define TR_SF_PLAY_SCRIPT            "[Lua]"
#endif

#if defined(DEBUG)
  #define TR_SF_TEST                   "Test"
#else
  #define TR_SF_TEST
#endif

#if defined(OVERRIDE_CHANNEL_FUNCTION) && LCD_W >= 212
  #define TR_SF_SAFETY                 "Remplace"
#elif defined(OVERRIDE_CHANNEL_FUNCTION)
  #define TR_SF_SAFETY                 "Rempl."
#else
  #define TR_SF_SAFETY                 "---"
#endif

#define TR_SF_SCREENSHOT               "Photo Ecran"
#define TR_SF_RACING_MODE              "Racing Mode"
#define TR_SF_DISABLE_TOUCH            "No Touch"
#define TR_SF_RESERVE                  "[reserve]"

#define TR_VFSWFUNC                    TR_SF_SAFETY,"Ecolage","Trim instant.","Remise à 0","Déf.",TR_ADJUST_GVAR,"Volume","DéfFailsafe","Test Port.","Bind",TR_SOUND,TR_PLAY_TRACK,TR_PLAY_VALUE,TR_SF_RESERVE,TR_SF_PLAY_SCRIPT,TR_SF_RESERVE,TR_SF_BG_MUSIC,TR_VVARIO,TR_HAPTIC,TR_SDCLOGS,"Rétroécl.",TR_SF_SCREENSHOT,TR_SF_RACING_MODE,TR_SF_DISABLE_TOUCH,TR_SF_TEST

#define TR_FSW_RESET_TELEM             TR("Télm", "Télémétrie")

#if LCD_W >= 212
  #define TR_FSW_RESET_TIMERS          "Chrono 1","Chrono 2","Chrono 3"
#else
  #define TR_FSW_RESET_TIMERS          "Chr1","Chr2","Chr3"
#endif

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS, "Tout", TR_FSW_RESET_TELEM
#define TR_FUNCSOUNDS                  TR("Bp1","Beep1"),TR("Bp2","Beep2"),TR("Bp3","Beep3"),TR("Wrn1","Warn1"),TR("Wrn2","Warn2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robot"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define TR_TELEM_RESERVE               TR("[--]", "[---]")
#define TR_TELEM_TIME                  TR("Heur", "H:M")
#define TR_RAS                         "SWR"
#define TR_RX_BATT                     TR("BtRx", "BatRx")
#define TR_A3_A4                       "A3","A4"
#define TR_A3_A4_MIN                   "A3-","A4-"

#define TR_ASPD_MAX                    TR("ViA+", "VitA+")

#if LCD_W >= 212
  #define TR_TELEM_RSSI_RX             "RSSI"
#else
  #define TR_TELEM_RSSI_RX             "Rx"
#endif

#define TR_TELEM_TIMERS                "Chr1","Chr2","Chr3"

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                TR("Métr.","Métriques"),TR("Impér.","Impériales")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","@C","@F","%","mAh","W","mW","dB","rpm","g","@","rad","ml","fOz","mlm","Hz","mS","uS","km"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Rien","Valeurs","Barres","Script"
#define TR_GPSFORMAT                   "DMS","NMEA"

#define TR_TEMPLATE_CLEAR_MIXES        "Suppr mixages"
#define TR_TEMPLATE_SIMPLE_4CH         "4 voies simple"
#define TR_TEMPLATE_STICKY_TCUT        "Coupure gaz"
#define TR_TEMPLATE_VTAIL              "Empennage V"
#define TR_TEMPLATE_DELTA              "Elevon\\Delta"
#define TR_TEMPLATE_ECCPM              "eCCPM"
#define TR_TEMPLATE_HELI               "Conf. Hélico"
#define TR_TEMPLATE_SERVO_TEST         "Test Servo"

#define TR_VSWASHTYPE                  "--- ","120","120X","140","90"

#define TR_STICKS_VSRCRAW              STR_CHAR_STICK "Dir", STR_CHAR_STICK "Prf", STR_CHAR_STICK "Gaz", STR_CHAR_STICK "Ail"

#if defined(PCBHORUS)
  #define TR_TRIMS_VSRCRAW               STR_CHAR_TRIM "Dir", STR_CHAR_TRIM "Prf", STR_CHAR_TRIM "Gaz", STR_CHAR_TRIM "Ail", STR_CHAR_TRIM "T5",  STR_CHAR_TRIM "T6"
#else
  #define TR_TRIMS_VSRCRAW               STR_CHAR_TRIM "Dir", STR_CHAR_TRIM "Prf", STR_CHAR_TRIM "Gaz", STR_CHAR_TRIM "Ail"
#endif

#if defined(PCBHORUS)
  #define TR_TRIMS_SWITCHES              STR_CHAR_TRIM "Dg", STR_CHAR_TRIM "Dd", STR_CHAR_TRIM "Pb", STR_CHAR_TRIM "Ph", STR_CHAR_TRIM "Gb", STR_CHAR_TRIM "Gh", STR_CHAR_TRIM "Ag", STR_CHAR_TRIM "Ad", STR_CHAR_TRIM "5d", STR_CHAR_TRIM "5u", STR_CHAR_TRIM "6d", STR_CHAR_TRIM "6u"
#else
#define TR_TRIMS_SWITCHES            TR("tDg",STR_CHAR_TRIM "Dg"),TR("tDd",STR_CHAR_TRIM "Dd"),TR("tPb",STR_CHAR_TRIM "Pb"),TR("tPh",STR_CHAR_TRIM "Ph"),TR("tGb",STR_CHAR_TRIM "Gb"),TR("tGh",STR_CHAR_TRIM "Gh"),TR("tAg",STR_CHAR_TRIM "Ag") TR("tAd",STR_CHAR_TRIM "Ad")
#endif

#if defined(PCBHORUS) || defined(PCBNV14)
  #define TR_VKEYS                     "PGUP","PGDN","ENTER","MDL","RTN","TELE","SYS"
#elif defined(PCBXLITE)
  #define TR_VKEYS                     "Shift","Exit","Enter","Down","Up","Right","Left"
#elif defined(RADIO_FAMILY_JUMPER_T12)
  #define TR_VKEYS                     "Exit","Enter","Down","Up","Right","Left"
#elif defined(RADIO_TX12) || defined(RADIO_TX12MK2)
  #define TR_VKEYS                     "Exit","Enter","PGUP","PGDN","SYS","MDL","TELE"
#elif defined(RADIO_T8)
  #define TR_VKEYS                     "RTN","ENTER","PGUP","PGDN","SYS","MDL","UP","DOWN"
#elif defined(RADIO_ZORRO)
  #define TR_VKEYS                     "RTN","ENTER","PGUP","PGDN","SYS","MDL","TELE"
#elif defined(PCBTARANIS)
  #define TR_VKEYS                     "Menu","Exit","Enter","Page","Plus","Minus"
#else
  #define TR_VKEYS                     "Menu","Exit","Down","Up","Right","Left"
#endif

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Default","High"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "ON","Un"

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Normal","Inverted"
#else
  #define TR_ROTARY_ENC_OPT         "Normal","Inverted","V-I H-N","V-I H-A"
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

#define TR_RESERVE_VSRCRAW             "[--]"
#define TR_EXTRA_VSRCRAW               "Batt","H:M","GPS",TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,TR_RESERVE_VSRCRAW,"Chr1","Chr2","Chr3"

#define TR_VTMRMODES                   "OFF","ON","GZs","GZ%","GZt"
#define TR_VTRAINER_MASTER_OFF         "OFF"
#define TR_VTRAINER_MASTER_JACK        "Maître/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Elève/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Maître/SBUS Module"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Maître/CPPM Module"
#define TR_VTRAINER_MASTER_BATTERY     "Maître/Série"
#define TR_VTRAINER_BLUETOOTH          TR("Maître/BT","Maître/Bluetooth"),TR("Elève/BT","Elève/Bluetooth")
#define TR_VTRAINER_MULTI              "Master/Multi"
#define TR_VFAILSAFE                   "Pas déf.","Maintien",TR("Prédéf.","Prédéfini"),"Pas d'imp","Récepteur"
#define TR_VSENSORTYPES                "Perso","Calculé"
#define TR_VFORMULAS                   "Addition","Moyenne","Min","Max","Multipl.","Totalise","Elément","Consomm.","Distance"
#define TR_VPREC                       "0.--","0.0","0.00"
#define TR_VCELLINDEX                  "Mini.","1","2","3","4","5","6","Maxi.","Diff."
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center only)","= (symetrical)"
#define TR_TIMER_DIR                   TR("Remain", "Show Remain"), TR("Elaps.", "Show Elapsed")

// ZERO TERMINATED STRINGS
#if defined(COLORLCD)
  #define INDENT
  #define LEN_INDENT                   3
  #define INDENT_WIDTH                 12
  #define BREAKSPACE                   "\036"
#else
  #define INDENT                       "\001"
  #define LEN_INDENT                   1
  #define INDENT_WIDTH                 (FW/2)
  #define BREAKSPACE                   " "
#endif

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "STD"
#else
#define TR_FONT_SIZES                  "STD","BOLD","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14)
  #define TR_ENTER                     "[NEXT]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#if defined(PCBHORUS)
  #define TR_EXIT                      "RTN"
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "EXIT"
  #define TR_OK                        "\010" "\010" "\010" TR("[OK]", "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_MENUWHENDONE                CENTER "\006" TR_ENTER " QUAND PRET"
#define TR_FREE                        "disp"
#define TR_YES                         "Yes"
#define TR_NO                          "No"
#define TR_DELETEMODEL                 "SUPPRIMER" BREAKSPACE "MODELE"
#define TR_COPYINGMODEL                "Copie..."
#define TR_MOVINGMODEL                 "Déplacement..."
#define TR_LOADINGMODEL                "Chargement..."
#define TR_NAME                        "Nom"
#define TR_MODELNAME                   TR("Nom modèle", "Nom du modèle")
#define TR_PHASENAME                   "Nom phase"
#define TR_MIXNAME                     TR("Nom ligne", "Nom du mixeur")
#define TR_INPUTNAME                   TR("Entrée", "Nom entrée")
#define TR_EXPONAME                    TR("Nom", "Nom ligne")
#define TR_BITMAP                      "Image du modèle"
#define TR_TIMER                       "Chrono "
#define TR_START                       "Start"
#define TR_ELIMITS                     TR("Limites ét.", "Limites étendues")
#define TR_ETRIMS                      TR("Trims ét.", "Trims étendus")
#define TR_TRIMINC                     TR("Pas Trim", "Pas des trims")
#define TR_DISPLAY_TRIMS               TR("Aff. trims", "Affichage trims")
#define TR_TTRACE                      TR("Source gaz", INDENT "Source")
#define TR_TTRIM                       TR("Trim gaz", INDENT "Trim ralenti uniq.")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", INDENT "Trim switch")
#define TR_BEEPCTR                     TR("Bips centr", "Bips centrage")
#define TR_USE_GLOBAL_FUNCS            TR("Fonc. glob.", "Fonctions globales")
#define TR_PROTOCOL                    TR("Proto.", "Protocole")
#define TR_PPMFRAME                    INDENT "Trame PPM"
#define TR_REFRESHRATE                 INDENT "Période"
#define STR_WARN_BATTVOLTAGE           TR(INDENT "Signal est VBAT: ", INDENT "ATTENTION: signal est à VBAT: ")
#define TR_WARN_5VOLTS                 "Warning: output level is 5 volts"
#define TR_MS                          "ms"
#define TR_FREQUENCY                   INDENT "Fréquence"
#define TR_SWITCH                      TR("Inter", "Interrupteur")
#define TR_TRIMS                       "Trims"
#define TR_FADEIN                      "Fondu ON"
#define TR_FADEOUT                     "Fondu OFF"
#define TR_DEFAULT                     "(défaut)"
#define TR_CHECKTRIMS                  "\006Vérif\012Trims"
#define OFS_CHECKTRIMS                 (9*FW)
#define TR_SWASHTYPE                   TR("Type de Plat.", "Type de plateau")
#define TR_COLLECTIVE                  TR("Collectif", "Source collectif")
#define TR_AILERON                     "Source cyc. lat."
#define TR_ELEVATOR                    "Source cyc. lon."
#define TR_SWASHRING                   TR("Limite Cycl.", "Limite du cyclique")
#define TR_ELEDIRECTION                TR("Inv. longitud.", "Inversion longitudinal")
#define TR_AILDIRECTION                TR("Inv. latéral", "Inversion latéral")
#define TR_COLDIRECTION                TR("Inv. collectif", "Inversion collectif")
#define TR_MODE                        "Mode"
#define TR_SUBTYPE                     INDENT "Sous-type"
#define TR_NOFREEEXPO                  "Max expos atteint!"
#define TR_NOFREEMIXER                 "Max mixages atteint!"
#define TR_SOURCE                      "Source"
#define TR_WEIGHT                      "Ratio"
#define TR_EXPO                        TR("Expo", "Exponentiel")
#define TR_SIDE                        "Coté"
#define TR_DIFFERENTIAL                "Différentiel"
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
#define TR_CV                          "CB"
#if defined(PCBNV14)
#define TR_GV                          "VG"
#else
#define TR_GV                          TR("G", "VG")
#endif
#define TR_ACHANNEL                    "A"
#define TR_RANGE                       INDENT "Plage"
#define TR_CENTER                      INDENT "Centre"
#define TR_BAR                         "Barre"
#define TR_ALARM                       "Alarme"
#define TR_USRDATA                     "Données"
#define TR_BLADES                      "Pales/Poles"
#define TR_SCREEN                      "Ecran "
#define TR_SOUND_LABEL                 "Son"
#define TR_LENGTH                      "Durée"
#define TR_BEEP_LENGTH                 "Durée bips"
#define TR_BEEP_PITCH                   "Tonalité"
#define TR_HAPTIC_LABEL                "Vibreur"
#define TR_STRENGTH                    "Force"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Décalage"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Contraste"
#define TR_ALARMS_LABEL                "Alarmes"
#define TR_BATTERY_RANGE               "Plage batterie"
#define TR_BATTERYCHARGING             "Charging..."
#define TR_BATTERYFULL                 "Battery full"
#define TR_BATTERYNONE                 "None!"
#define TR_BATTERYWARNING              TR("Batterie", "Batterie faible")
#define TR_INACTIVITYALARM             "Inactivité"
#define TR_MEMORYWARNING               "Mémoire pleine"
#define TR_ALARMWARNING                TR("Silence", "Sons désactivés")
#define TR_RSSI_SHUTDOWN_ALARM           TR("RSSI extinct.", "Vérif RSSI à l'extinction")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_MODEL_STILL_POWERED         TR("Modèle allumé", "Modèle encore allumé")
#define TR_USB_STILL_CONNECTED         "USB still connected"
#define TR_MODEL_SHUTDOWN              "Eteindre ?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Presser [Enter] pour confirmer"
#define TR_THROTTLE_LABEL              "Gaz"
#define TR_THROTTLEREVERSE             TR("Inv. gaz", INDENT "Inversion gaz")
#define TR_MINUTEBEEP                  TR("Bip min.", "Annonces minutes")
#define TR_BEEPCOUNTDOWN               TR(INDENT "Bip fin", INDENT "Compte à rebours")
#define TR_PERSISTENT                  TR(INDENT "Persist.", INDENT "Persistant")
#define TR_BACKLIGHT_LABEL             "Rétroéclairage"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLDELAY                     INDENT "Durée"
#define TR_BLONBRIGHTNESS              INDENT "Luminosité ON"
#define TR_BLOFFBRIGHTNESS             INDENT "Luminosité OFF"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR                     "Couleur"
#define TR_SPLASHSCREEN                "Logo d'accueil"
#define TR_PWR_ON_DELAY                "Délai btn ON"
#define TR_PWR_OFF_DELAY               "Délai btn OFF"
#define TR_THROTTLE_WARNING             TR(INDENT "Alerte gaz", INDENT "Alerte gaz")
#define TR_CUSTOM_THROTTLE_WARNING     TR(INDENT INDENT INDENT INDENT "Cust-Pos", INDENT INDENT INDENT INDENT "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR(INDENT "Alerte int", INDENT "Pos. interrupteurs")
#define TR_POTWARNINGSTATE             TR(INDENT "Pot&Slid.", INDENT "Pots & sliders")
#define TR_SLIDERWARNING               TR(INDENT "Slid. pos.", INDENT "Slider positions")
#define TR_POTWARNING                  TR(INDENT "Alerte pot", INDENT "Pos. potentios")
#define TR_TIMEZONE                    "Fuseau horaire"
#define TR_ADJUST_RTC                  TR("Ajust. RTC", "Ajust. heure auto")
#define TR_GPS                         "GPS"
#define TR_RXCHANNELORD                TR("Ordre voies RX","Ordre des voies préféré")
#define TR_STICKS                      "Manches"
#define TR_POTS                        "Potentiomètres"
#define TR_SWITCHES                    "Inters"
#define TR_SWITCHES_DELAY              "Délai inter son"
#define TR_SLAVE                       "Elève"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "Multiplieur"
#define TR_CAL                         "Cal"
#define TR_CALIBRATION                 "Calibration"
#define TR_VTRIM                       "Trim - +"
#define TR_BG                          "BG:"
#define TR_CALIB_DONE                  "Calibration completed"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Presser [Enter] pour commencer"
  #define TR_SETMIDPOINT               "Centrer manches/pots/sliders puis [Enter]"
  #define TR_MOVESTICKSPOTS            "Bouger manches/pots/sliders puis [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " POUR DEBUT"
  #define TR_SETMIDPOINT               "REGLER NEUTRES"
  #define TR_MOVESTICKSPOTS            "BOUGER STICKS/POTS"
#else
  #define TR_MENUTOSTART               CENTER"\006" TR_ENTER " POUR DEBUT"
  #define TR_SETMIDPOINT               CENTER"\010REGLER NEUTRES"
  #define TR_MOVESTICKSPOTS            CENTER"\004BOUGER STICKS/POTS"
#endif
#define TR_RXBATT                      "Batt.RX"
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_ACCEL                       "Acc:"
#define TR_NODATA                      CENTER "NO DATA"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMR1LATMINUS                "Tmr1Lat min\037\124us"

#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua scripts"
#define TR_FREE_MEM_LABEL              "Free mem"
#define TR_TIMER_LABEL                 "Timer"
#define TR_THROTTLE_PERCENT_LABEL      "Throttle %"
#define TR_BATT_LABEL                  "Battery"
#define TR_SESSION                     "Session"

#define TR_MENUTORESET                 TR_ENTER" pour reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODELE"
#define TR_FM                          "PV"
#define TR_MIX                         "MIX"
#define TR_EEPROMLOWMEM                "EEPROM pleine!"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Touche pour ignorer"
#define TR_THROTTLE_NOT_IDLE           "Gaz pas à zéro"
#define TR_ALARMSDISABLED              "Alarmes Désactivées"
#define TR_PRESSANYKEY                 TR("Touche pour continuer", "Touche pour continuer")
#define TR_BADEEPROMDATA               "EEPROM corrompue"
#define TR_BAD_RADIO_DATA              "Réglages radio corrompus"
#define TR_EEPROMFORMATTING            "Formatage EEPROM"
#define TR_STORAGE_FORMAT              "Préparation stockage"
#define TR_EEPROMOVERFLOW              "Dépassement EEPROM"
#define TR_RADIO_SETUP                 "CONFIG RADIO"
#define TR_MENUDATEANDTIME             "DATE ET HEURE"
#define TR_MENUTRAINER                 "ECOLAGE"
#define TR_MENUSPECIALFUNCS            "FONCTIONS GLOBALES"
#define TR_MENUVERSION                 "VERSION"
#define TR_MENU_RADIO_SWITCHES         TR("INTERS", "TEST INTERRUPTEURS")
#define TR_MENU_RADIO_ANALOGS          TR("ANAS", "ENTREES ANALOGIQUES")
#define TR_MENU_RADIO_ANALOGS_CALIB    "CALIBRATED ANALOGS"
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "RAW ANALOGS (5 Hz)"
#define TR_MENUCALIBRATION             "CALIBRATION"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS             "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE          "Channels=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Channel=>Failsafe"
#define TR_MENUMODELSEL                "MODELES"
#define TR_MENU_MODEL_SETUP                   TR("CONF. MODELE", "CONFIGURATION")
#define TR_MENUFLIGHTMODE              "PHASE DE VOL"
#define TR_MENUFLIGHTMODES             "PHASES DE VOL"
#define TR_MENUHELISETUP               TR("CONF.HELI", "CONFIGURATION HELICO")

  #define TR_MENUINPUTS                "ENTREES"
  #define TR_MENULIMITS                "SORTIES"

#define TR_MENUCURVES                  "COURBES"
#define TR_MENUCURVE                   "COURBE"
#define TR_MENULOGICALSWITCH           "INTER LOG."
#define TR_MENULOGICALSWITCHES         TR("INTERS LOG.", "INTERS LOGIQUES")
#define TR_MENUCUSTOMFUNC              TR("FONCTIONS SPEC.", "FONCTIONS SPECIALES")
#define TR_MENUCUSTOMSCRIPTS           "SCRIPTS PERSOS"
#define TR_MENUTELEMETRY               "TELEMESURE"
#define TR_MENUTEMPLATES               "GABARITS"
#define TR_MENUSTAT                    TR("STATS", "STATISTIQUES")
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS1           "VOIES 1-8"
#define TR_MONITOR_CHANNELS2           "VOIES 9-16"
#define TR_MONITOR_CHANNELS3           "VOIES 17-24"
#define TR_MONITOR_CHANNELS4           "VOIES 25-32"
#define TR_MONITOR_SWITCHES            "INTERS LOGIQUES"
#define TR_MONITOR_OUTPUT_DESC         "Sorties"
#define TR_MONITOR_MIXER_DESC          "Mixeurs"
#define TR_RECEIVER_NUM                TR("NumRx", "No. récepteur")
#define TR_RECEIVER                    "Récept."
#define TR_MULTI_RFTUNE                TR(INDENT "Ajust.fréq", INDENT "Ajustement fréq.")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY             "Télémétrie"
#define TR_MULTI_VIDFREQ               TR(INDENT "Fréq. vidéo", INDENT "Fréquence vidéo")
#define TR_RF_POWER                     TR("Puiss. RF", "Puissance RF")
#define TR_MULTI_FIXEDID               "ID fixe"
#define TR_MULTI_OPTION                TR(INDENT "Option", INDENT "Option perso")
#define TR_MULTI_AUTOBIND              TR(INDENT "Bind voie", INDENT "Bind sur voie")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Désact réorg voies")
#define TR_DISABLE_TELEM               TR("No Telem", "Désact Télémétrie")
#define TR_MULTI_DSM_AUTODTECT         TR(INDENT "Autodét.", INDENT "Autodétection")
#define TR_MULTI_LOWPOWER              TR(INDENT "Basse puis.", INDENT "Mode basse puiss.")
#define TR_MULTI_LNA_DISABLE            INDENT "LNA disable"
#define TR_MODULE_TELEMETRY            TR(INDENT "S.Port", INDENT "Lien S.Port")
#define TR_MODULE_TELEM_ON             TR("ON", "Actif")
#define TR_DISABLE_INTERNAL            TR("Désact intRF", "Désact. RF interne")
#define TR_MODULE_NO_SERIAL_MODE       TR("Mode série?", "Pas en mode série")
#define TR_MODULE_NO_INPUT             TR("Pas de sign.", "Aucun signal série")
#define TR_MODULE_NO_TELEMETRY         TR3("Pas de télm.", "Télémétrie absente", "Télémétrie absente(act. MULTI_TELEMETRY)")
#define TR_MODULE_WAITFORBIND          "Binder d'abord"
#define TR_MODULE_BINDING              "Bind..."
#define TR_MODULE_UPGRADE_ALERT              "Mise à jour requise"
#define TR_MODULE_UPGRADE              TR("Upg. advised", "Module update recommended")
#define TR_REBIND                      "Rebind requis"
#define TR_REG_OK                      "Enregistr. ok"
#define TR_BIND_OK                     "Bind ok"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Télem ON"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Télem OFF"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Télem ON"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Télem OFF"
#define TR_PROTOCOL_INVALID            TR("Sél. invalide", "Protocole invalide")
#define TR_MODULE_STATUS               TR(INDENT "Etat", INDENT "Etat module")
#define TR_MODULE_SYNC                 TR(INDENT "Sync", INDENT "Proto Sync Status")
#define TR_MULTI_SERVOFREQ             TR(INDENT "Fréq.servo", INDENT "Fréquence servos")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("RF Channel", "Select RF channel")
#define TR_SYNCMENU                    "Sync [MENU]"
#define TR_LIMIT                       INDENT "Limite"
#define TR_MINRSSI                     "RSSI Min."
#define TR_LATITUDE                    "Latitude"
#define TR_LONGITUDE                   "Longitude"
#define TR_GPS_COORDS_FORMAT                    TR("Coordonnées", "Coordonnées GPS")
#define TR_VARIO                       TR("Vario", "Variomètre")
#define TR_PITCH_AT_ZERO               "Tonalité zéro"
#define TR_PITCH_AT_MAX                "Tonalité max"
#define TR_REPEAT_AT_ZERO              TR("Interv. zéro", "Intervalle au zéro")
#define TR_SHUTDOWN                    "ARRET EN COURS"
#define TR_SAVE_MODEL                   "Sauvegarde modèle..."
#define TR_BATT_CALIB                  "Calib. batt"
#define TR_CURRENT_CALIB               "Calib. cour"
#define TR_VOLTAGE                     TR(INDENT "Tension",INDENT "Source tension")
#define TR_CURRENT                     TR(INDENT "Courant",INDENT "Source courant")
#define TR_SELECT_MODEL                "Sélect. modèle"
#define TR_MODEL_CATEGORIES            "Model Categories"
#define TR_MODELS                      "Models"
#define TR_SELECT_MODE                 "Select mode"
#define TR_CREATE_CATEGORY             "Créer une catégorie"
#define TR_RENAME_CATEGORY             "Renommer la catégorie"
#define TR_DELETE_CATEGORY             "Supprimer la catégorie"
#define TR_CREATE_MODEL                "Créer modèle"
#define TR_DUPLICATE_MODEL             "Dupliquer modèle"
#define TR_COPY_MODEL                  "Copier modèle"
#define TR_MOVE_MODEL                  "Déplacer modèle"
#define TR_BACKUP_MODEL                "Archiver modèle"
#define TR_DELETE_MODEL                "Supprimer modèle"
#define TR_RESTORE_MODEL               "Restaurer modèle"
#define TR_DELETE_ERROR                "Effacement impossible"
#define TR_CAT_NOT_EMPTY               "Categorie non vide"
#define TR_SDCARD_ERROR                "Erreur carte SD"
#define TR_SDCARD                      "SD Card"
#define TR_NO_FILES_ON_SD              "No files on SD!"
#define TR_NO_SDCARD                   "Carte SD indisponible"
#define TR_WAITING_FOR_RX              "Attente du RX..."
#define TR_WAITING_FOR_TX              "Attente du TX..."
#define TR_WAITING_FOR_MODULE          TR("Waiting module", "Waiting for module...")
#define TR_NO_TOOLS                    "Pas d'outils dispo"
#define TR_NORMAL                      "Normal"
#define TR_NOT_INVERTED                "Non inv"
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
#define TR_MENU_MODULES_RX_VERSION     "VERSIONS MODULES / RX "
#define TR_MENU_FIRM_OPTIONS           "OPTIONS FIRMWARE"
#define TR_IMU                        "IMU"
#define TR_STICKS_POTS_SLIDERS         "Manches/Pots/leviers"
#define TR_PWM_STICKS_POTS_SLIDERS     "Manches PWM/Pots/leviers"
#define TR_RF_PROTOCOL                 "Protocole RF"
#define TR_MODULE_OPTIONS              "Options module"
#define TR_POWER                       "Puissance"
#define TR_NO_TX_OPTIONS               "Pas d'options TX"
#define TR_RTC_BATT                    "Pile RTC"
#define TR_POWER_METER_EXT             "Puissancemètre (EXT)"
#define TR_POWER_METER_INT             "Puissancemètre (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       TR("Spectre (EXT)", "Analyseur spectre (EXT)")
#define TR_SPECTRUM_ANALYSER_INT       TR("Spectre (INT)", "Analyseur spectre (INT)")
#define TR_SDCARD_FULL                 "Carte SD pleine"
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Incompatible"
#define TR_WARNING                     "ALERTE"
#define TR_EEPROMWARN                  "EEPROM"
#define TR_STORAGE_WARNING             "STOCKAGE"
#define TR_EEPROM_CONVERTING           "Conversion EEPROM"
#define TR_SDCARD_CONVERSION_REQUIRE   "SD card conversion required"
#define TR_CONVERTING                  "Converting: "
#define TR_THROTTLE_UPPERCASE                "GAZ"
#define TR_ALARMSWARN                  "SON"
#define TR_SWITCHWARN                  TR("INTERS","CONTROLES")
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                 TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE                 "Version de test uniq."
#define TR_WRONG_SDCARDVERSION         "Version requise: "
#define TR_WARN_RTC_BATTERY_LOW        "Batterie RTC faible"
#define TR_WARN_MULTI_LOWPOWER         "Mode basse puis."
#define TR_BATTERY                     "BATTERIE"
#define TR_WRONG_PCBREV                "PCB incorrect détecté"
#define TR_EMERGENCY_MODE              "MODE SECOURS"
#define TR_PCBREV_ERROR                "Erreur PCB"
#define TR_NO_FAILSAFE                 TR3("Failsafe pas déf.", "Failsafe pas déf.", "Failsafe pas défini")
#define TR_KEYSTUCK                    "Touche bloquée"
#define TR_INVERT_THR                  "Inverser gaz?"
#define TR_VOLUME                      "Volume"
#define TR_LCD                         "Afficheur"
#define TR_BRIGHTNESS                  "Luminosité"
#define TR_CPU_TEMP                    "Temp. CPU\016>"
#define TR_CPU_CURRENT                 "Courant\022>"
#define TR_CPU_MAH                     "Consomm."
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "Temp. MB \016>"
#define TR_CAPAWARNING                 INDENT "Capacité Basse"
#define TR_TEMPWARNING                 INDENT "Surchauffe"
#define TR_TTL_WARNING                 "Warning: use 3.3V logic levels"
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
#define TR_ASSIGN_BITMAP               "Sélectionner image"
#define TR_ASSIGN_SPLASH               "Logo d'accueil"
#define TR_EXECUTE_FILE                "Exécuter"
#define TR_REMOVED                     " supprimé"
#define TR_SD_INFO                     "Information"
#define TR_SD_FORMAT                   "Formater"
#define TR_NA                          "N/D"
#define TR_HARDWARE                    "MATERIEL"
#define TR_FORMATTING                  "Formatage..."
#define TR_TEMP_CALIB                  "Calib. temp"
#define TR_TIME                        "Heure"
#define TR_MAXBAUDRATE                 "Max bauds"
#define TR_BAUDRATE                    "Baudrate"
#define TR_SAMPLE_MODE                 "Sample Mode"
#define TR_SAMPLE_MODES                "Normal""OneBit"
#define TR_SELECT_TEMPLATE_FOLDER      "SELECT A TEMPLATE FOLDER:"
#define TR_SELECT_TEMPLATE             "SELECT A MODEL TEMPLATE:"
#define TR_NO_TEMPLATES                "No model templates were found in this folder"
#define TR_SAVE_TEMPLATE               "Save as template"
#define TR_BLANK_MODEL                 "Blank Model"
#define TR_BLANK_MODEL_INFO            "Create a blank model"
#define TR_FILE_EXISTS                 "FILE ALREADY EXISTS"
#define TR_ASK_OVERWRITE               "Do you want to overwrite?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Découvrir"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Addr dist."
#define TR_BLUETOOTH_LOCAL_ADDR        "Addr locale"
#define TR_BLUETOOTH_PIN_CODE          "Code PIN"
#define TR_BLUETOOTH_NODEVICES         "No Devices Found"
#define TR_BLUETOOTH_SCANNING          "Scanning..."
#define TR_BLUETOOTH_BAUDRATE          "Baudrate BT"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Activé"
#else
#define TR_BLUETOOTH_MODES             "---","Télémétrie","Ecolage"
#endif
#define TR_SD_INFO_TITLE               "INFO SD"
#define TR_SD_TYPE                     "Type:"
#define TR_SD_SPEED                    "Vitesse:"
#define TR_SD_SECTORS                  "Secteurs:"
#define TR_SD_SIZE                     "Taille:"
#define TR_TYPE                        INDENT "Type"
#define TR_GLOBAL_VARS                 "Variables Globales"
#define TR_GVARS                       "V. GLOBALES"
#define TR_GLOBAL_VAR                  "Variable globale"
#define TR_MENU_GLOBAL_VARS              "VARIABLES GLOBALES"
#define TR_OWN                         "Pers"
#define TR_DATE                        "Date"
#define TR_MONTHS                      { "Jan", "Fév", "Mar", "Avr", "Mai", "Jun", "Jul", "Aou", "Sep", "Oct", "Nov", "Dec" }
#define TR_ROTARY_ENCODER              "Enc.Rot."
#define TR_ROTARY_ENC_MODE             TR("RotEnc Mode","Rotary Encoder Mode")
#define TR_CHANNELS_MONITOR            "MONITEUR CANAUX"
#define TR_MIXERS_MONITOR              "MONITEUR MIXAGES "
#define TR_PATH_TOO_LONG               "Chemin trop long"
#define TR_VIEW_TEXT                   "Voir texte"
#define TR_FLASH_BOOTLOADER            "Flasher BootLoader"
#define TR_FLASH_DEVICE                TR("Flash device","Flash device")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Flasher S.Port", "Flasher S.Port externe")
#define TR_FLASH_RECEIVER_OTA          "Flasher RX OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX by ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX by int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash module BT", "Flash module Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Flasher pwr mngt unit"
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "Device not responding")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Device file prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "Device data refused")
#define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "Device access problem")
#define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "Device file refused")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", "Device file sig.")
#define TR_CURRENT_VERSION             "Version courante :"
#define TR_FLASH_INTERNAL_MODULE       TR("Flasher module int.", "Flasher module interne")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Int. Multi", "Flash Internal Multi")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flasher module ext.", "Flasher module externe")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Ext. Multi", "Flash External Multi")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash Ext. ELRS", "Flash External ELRS")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Erreur màj FW","Erreur de mise à jour")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Flash ok"
#define TR_WRITING                     "Ecriture..."
#define TR_CONFIRM_FORMAT              "Confirmer Formatage?"
#define TR_INTERNALRF                  "HF interne"
#define TR_INTERNAL_MODULE             TR("Module int.","Module interne")
#define TR_EXTERNAL_MODULE             TR("Module ext.","Module externe")
#define TR_OPENTX_UPGRADE_REQUIRED     "Màj OpenTX nécessaire"
#define TR_TELEMETRY_DISABLED          "Télém. désactivée"
#define TR_MORE_OPTIONS_AVAILABLE      "Autres options disponibles"
#define TR_NO_MODULE_INFORMATION       "Pas d'info module"
#define TR_EXTERNALRF                  "HF externe"
#define TR_FAILSAFE                    TR(INDENT "Failsafe", INDENT "Type failsafe")
#define TR_FAILSAFESET                 "REGLAGES FAILSAFE"
#define TR_REG_ID                      TR("ID Enr.", "ID Enregistr.")
#define TR_OWNER_ID                    "ID Radio"
#define TR_PINMAPSET                   "PINMAP"
#define TR_HOLD                        "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                        "None"
#define TR_NONE_UPPERCASE              "NONE"
#define TR_MENUSENSOR                  "CAPTEUR"
#define TR_POWERMETER_PEAK             "Pic"
#define TR_POWERMETER_POWER            "Puiss"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Freq."
#define TR_MENUTOOLS                   "OUTILS"
#define TR_TURN_OFF_RECEIVER           "Eteindre récept."
#define TR_STOPPING                    "Arret..."
#define TR_MENU_SPECTRUM_ANALYSER      "ANALYSEUR SPECTRE"
#define TR_MENU_POWER_METER            "MESURE PUISS."
#define TR_SENSOR                      "CAPTEUR"
#define TR_COUNTRY_CODE                 TR("Zone géo.", "Zone géographique")
#define TR_USBMODE                     "Mode USB"
#define TR_JACK_MODE                    "Mode Jack"
#define TR_VOICE_LANGUAGE                   TR("Langue voix", "Langue annonces vocales")
#define TR_UNITS_SYSTEM                 "Unités"
#define TR_EDIT                        "Editer"
#define TR_INSERT_BEFORE               "Insérer avant"
#define TR_INSERT_AFTER                "Insérer après"
#define TR_COPY                        "Copier"
#define TR_MOVE                        "Déplacer"
#define TR_PASTE                       "Coller"
#define TR_PASTE_AFTER                 "Paste After"
#define TR_PASTE_BEFORE                "Paste Before"
#define TR_DELETE                      "Supprimer"
#define TR_INSERT                      "Insérer"
#define TR_RESET_FLIGHT                TR("Réinit. vol", "Réinitialiser vol")
#define TR_RESET_TIMER1                TR("Réinit. Timer1", "Réinitialiser Timer1")
#define TR_RESET_TIMER2                TR("Réinit. Timer2", "Réinitialiser Timer2")
#define TR_RESET_TIMER3                TR("Réinit. Timer3", "Réinitialiser Timer3")
#define TR_RESET_TELEMETRY             TR("Réinit. Télém.", "Réinit. Télémétrie")
#define TR_STATISTICS                  "Statistiques"
#define TR_ABOUT_US                    "A propos"
#define TR_USB_JOYSTICK                "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE            "Stockage USB (SD)"
#define TR_USB_SERIAL                  "Port série (VCP)"
#define TR_SETUP_SCREENS               "Configuration écrans"
#define TR_MONITOR_SCREENS             "Moniteurs"
#define TR_AND_SWITCH                  "ET suppl."
#define TR_SF                          "FS"
#define TR_GF                          "FG"
#define TR_ANADIAGS_CALIB              "Calibrated analogs"
#define TR_ANADIAGS_FILTRAWDEV         "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW          "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX             "Min., max. and range"
#define TR_ANADIAGS_MOVE               "Move analogs to their extremes!"
#define TR_SPEAKER                     INDENT "Haut-p."
#define TR_BUZZER                      INDENT "Bipeur"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_POWERMETER_ATTN_NEEDED      "Atténuateur requis"
#define TR_PXX2_SELECT_RX              "Sélect RX..."
#define TR_PXX2_DEFAULT                "<défaut>"
#define TR_BT_SELECT_DEVICE            "Sélect appareil"
#define TR_DISCOVER                    "Découvrir"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Attente..."
#define TR_RECEIVER_DELETE             "Suppr récept.?"
#define TR_RECEIVER_RESET              "Reset récept.?"
#define TR_SHARE                       "Partg"
#define TR_BIND                        "Bind"
#define TR_REGISTER                    TR("Enr", "Enregistr.")
#define TR_MODULE_RANGE                BUTTON(TR("Prt", "Port."))
#define TR_RECEIVER_OPTIONS            TR("OPTIONS REC.", "OPTIONS RECEPTEUR")
#define TR_DEL_BUTTON                  BUTTON(TR("Eff", "Effacer"))
#define TR_RESET_BTN                   BUTTON("RAZ")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON(TR("Btns","Inters"))
#define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "Analogs"))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_SET                         BUTTON("Déf")
#define TR_TRAINER                     "Ecolage"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM              CENTER "Antenne radio défect.!"
#define TR_MODELIDUSED                 TR("ID affecté à :", "No de récepteur utilisé par :")
#define TR_MODULE                      "Module"
#define TR_RX_NAME                     "Nom Rx"
#define TR_TELEMETRY_TYPE              TR("Type tél.", "Type télémétrie")
#define TR_TELEMETRY_SENSORS           "Capteurs"
#define TR_VALUE                       "Valeur"
#define TR_REPEAT                      "Repeat"
#define TR_ENABLE                      "Enable"
#define TR_TOPLCDTIMER                 "Timer LCD haut"
#define TR_UNIT                        "Unité"
#define TR_TELEMETRY_NEWSENSOR         TR(INDENT"Nouveau capteur...", INDENT "Ajout d'un nouveau capteur...")
#define TR_CHANNELRANGE                TR(INDENT "Canaux", INDENT "Plage de canaux")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_ANTENNACONFIRM1             "Vraiment changer?"
#if defined(PCBX12S)
#define TR_ANTENNA_MODES               "Interne","Demander","Par modèle","Interne + Externe"
#else
#define TR_ANTENNA_MODES               "Interne","Demander","Par modèle","Externe"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Util antenne int", "Utiliser antenne interne")
#define TR_USE_EXTERNAL_ANTENNA        TR("Util antenne ext", "Utiliser antenne externe")
#define TR_ANTENNACONFIRM2             TR("Vérif antenne", "Installer l'antenne d'abord!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1        "Nécessite firm."
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1         "Nécessite FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1          "Nécessite EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2             "R9M non certifié"
#define TR_LOWALARM                    INDENT "Alarme basse"
#define TR_CRITICALALARM               INDENT "Alarme critique"
#define TR_RSSIALARM_WARN              TR("RSSI", "TELEMETRIE")
#define TR_NO_RSSIALARM                TR(INDENT "Alarmes désact.", "Alarme télémétrie désactivée")
#define TR_DISABLE_ALARM               TR(INDENT "Désact. alarme", INDENT "Désactiver alarme télémétrie")
#define TR_ENABLE_POPUP                "Activer popup"
#define TR_DISABLE_POPUP               "Désactiver popup"
#define TR_POPUP                       "Popup"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Courbe standard..."
#define TR_PRESET                      "Pente"
#define TR_MIRROR                      "Miroir"
#define TR_CLEAR                       "Effacer"
#define TR_RESET                       "Réinitialiser"
#define TR_RESET_SUBMENU               "Réinitialiser..."
#define TR_COUNT                       "Nb points"
#define TR_PT                          "pt"
#define TR_PTS                         "pts"
#define TR_SMOOTH                      "Lissage"
#define TR_COPY_STICKS_TO_OFS          TR("Cpy stick->subtrim", "Manche vers subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
#define TR_COPY_TRIMS_TO_OFS           TR("Cpy trim->subtrim", "Trim vers subtrim")
#define TR_INCDEC                      "Inc/décrementer"
#define TR_GLOBALVAR                   "Var. globale"
#define TR_MIXSOURCE                   "Source mixeur"
#define TR_CONSTANT                    "Constante"
#define TR_PERSISTENT_MAH              TR(INDENT "Enr. mAh", INDENT "Enregistrer mAh")
#define TR_PREFLIGHT                   "Vérifications avant vol"
#define TR_CHECKLIST                   TR(INDENT "Notes", INDENT "Afficher notes")
#define TR_FAS_OFFSET                  TR(INDENT "Corr FAS", INDENT "Correction FAS")
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
#define TR_NO_TELEMETRY_SCREENS        "No Telemetry Screens"
#define TR_TOUCH_PANEL                 "Touch panel:"
#define TR_FILE_SIZE                   "File size"
#define TR_FILE_OPEN                   "Open anyway?"

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Nom"
#define TR_PHASES_HEADERS_SW           "Inter"
#define TR_PHASES_HEADERS_RUD_TRIM     "Trim Dir"
#define TR_PHASES_HEADERS_ELE_TRIM     "Trim Prf"
#define TR_PHASES_HEADERS_THT_TRIM     "Trim Gaz"
#define TR_PHASES_HEADERS_AIL_TRIM     "Trim Ail"
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
#define TR_INVERTED                    "Inverted"

#define TR_LSW_HEADERS_FUNCTION        "Fonction"
#define TR_LSW_HEADERS_V1              "V1"
#define TR_LSW_HEADERS_V2              "V2"
#define TR_LSW_HEADERS_ANDSW           "ET suppl."
#define TR_LSW_HEADERS_DURATION        "Durée"
#define TR_LSW_HEADERS_DELAY           "Délai"

#define TR_GVAR_HEADERS_NAME           "Nom"
#define TR_GVAR_HEADERS_FM0            "Valeur FM0"
#define TR_GVAR_HEADERS_FM1            "Valeur FM1"
#define TR_GVAR_HEADERS_FM2            "Valeur FM2"
#define TR_GVAR_HEADERS_FM3            "Valeur FM3"
#define TR_GVAR_HEADERS_FM4            "Valeur FM4"
#define TR_GVAR_HEADERS_FM5            "Valeur FM5"
#define TR_GVAR_HEADERS_FM6            "Valeur FM6"
#define TR_GVAR_HEADERS_FM7            "Valeur FM7"
#define TR_GVAR_HEADERS_FM8            "Valeur FM8"

// Horus footer descriptions
#define TR_LSW_DESCRIPTIONS            { "Type de comparaison ou fonction", "Première variable", "Seconde variable ou constante", "Seconde variable ou constante", "Condition supplémentaire pour activer la ligne", "Durée minimale d'activation de l'inter logique", "Durée min de la condition avant l'activation de l'inter" }

// Horus layouts and widgets
#define TR_FIRST_CHANNEL               "First channel"
#define TR_FILL_BACKGROUND             "Fill background?"
#define TR_BG_COLOR                    "BG Color"
#define TR_SLIDERS_TRIMS               "Sliders+Trims"
#define TR_SLIDERS                     "Sliders"
#define TR_FLIGHT_MODE                 "Flight mode"
#define TR_INVALID_FILE                "Invalid File"
#define TR_TIMER_SOURCE                "Timer source"
#define TR_SIZE                        "Size"
#define TR_SHADOW                      "Shadow"
#define TR_TEXT                        "Text"
#define TR_COLOR                       "Color"
#define TR_MAIN_VIEW_X                 "Main view X"
#define TR_PANEL1_BACKGROUND           "Panel1 background"
#define TR_PANEL2_BACKGROUND           "Panel2 background"

// Taranis About screen
#define TR_ABOUTUS                     TR(" A PROPOS ", "A PROPOS")

#define TR_CHR_SHORT                   's'
#define TR_CHR_LONG                    'l'
#define TR_CHR_TOGGLE                  't'
#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'E'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Volume bips"
#define TR_WAV_VOLUME                  "Volume audio"
#define TR_BG_VOLUME                   "Volume musique"

#define TR_TOP_BAR                     "Barre titre"
#define TR_FLASH_ERASE                 "Flash erase..."
#define TR_FLASH_WRITE                 "Flash write..."
#define TR_OTA_UPDATE                  "OTA update..."
#define TR_MODULE_RESET                "Module reset..."
#define TR_UNKNOWN_RX                  "Unknown RX"
#define TR_UNSUPPORTED_RX              "Unsupported RX"
#define TR_OTA_UPDATE_ERROR            "OTA update error"
#define TR_DEVICE_RESET                "Device reset..."
#define TR_ALTITUDE                    INDENT "Altitude"
#define TR_SCALE                       "Echelle"
#define TR_VIEW_CHANNELS               "Voir voies"
#define TR_VIEW_NOTES                  "Voir notes"
#define TR_MODEL_SELECT                "Sélection modèle"
#define TR_MODS_FORBIDDEN              "Modifications désactivées!"
#define TR_UNLOCKED                    "Déverrouillé"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Précision"
#define TR_RATIO                       "Ratio"
#define TR_FORMULA                     "Formule"
#define TR_CELLINDEX                   "Index élem."
#define TR_LOGS                        "Logs"
#define TR_OPTIONS                     "Options"
#define TR_FIRMWARE_OPTIONS            "Firmware options"

#define TR_ALTSENSOR                   "Capteur Alt"
#define TR_CELLSENSOR                  "Capteur Elém"
#define TR_GPSSENSOR                   "Capteur GPS"
#define TR_CURRENTSENSOR               "Capteur"
#define TR_AUTOOFFSET                  "Offset auto"
#define TR_ONLYPOSITIVE                "Positif"
#define TR_FILTER                      "Filtrage"
#define TR_TELEMETRYFULL               "Plus de capteurs libres!"
#define TR_SERVOS_OK                   "Servos OK"
#define TR_SERVOS_KO                   "Servos KO"
#define TR_INVERTED_SERIAL             INDENT "Inversé"
#define TR_IGNORE_INSTANCE             TR(INDENT "Ign. inst", INDENT "Ignorer instance")
#define TR_DISCOVER_SENSORS            "Découvrir capteurs"
#define TR_STOP_DISCOVER_SENSORS       "Terminer découverte"
#define TR_DELETE_ALL_SENSORS          TR("Suppr. tous capteurs", "Supprimer tous capteurs")
#define TR_CONFIRMDELETE               TR("Tout effacer?", "Vraiment tout " LCDW_128_480_LINEBREAK "effacer ?")
#define TR_SELECT_WIDGET               "Sélect. widget"
#define TR_REMOVE_WIDGET               "Supprimer widget"
#define TR_WIDGET_SETTINGS             "Réglages widget"
#define TR_REMOVE_SCREEN               "Supprimer écran"
#define TR_SETUP_WIDGETS               "Configurer widgets"
#define TR_USER_INTERFACE              "Interface utilisateur"
#define TR_THEME                       "Thème"
#define TR_SETUP                       "Configuration"
#define TR_LAYOUT                      "Disposition"
#define TR_ADD_MAIN_VIEW                 "Ajouter vue principale"
#define TR_BACKGROUND_COLOR            "Couleur de fond"
#define TR_MAIN_COLOR                  "Couleur principale"
#define TR_BAR2_COLOR                  "Secondary bar color"
#define TR_BAR1_COLOR                  "Main bar color"
#define TR_TEXT_COLOR                  "Text color"
#define TR_TEXT_VIEWER                 "Visualisateur de texte"

#define TR_MENU_INPUTS                  STR_CHAR_INPUT "Entrées"
#define TR_MENU_LUA                     STR_CHAR_LUA "Scripts Lua"
#define TR_MENU_STICKS                   STR_CHAR_STICK "Manches"
#define TR_MENU_POTS                    STR_CHAR_POT "Pots"
#define TR_MENU_MAX                     STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                    STR_CHAR_CYC "Cyclique"
#define TR_MENU_TRIMS                    STR_CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES                STR_CHAR_SWITCH "Inters"
#define TR_MENU_LOGICAL_SWITCHES        STR_CHAR_SWITCH "Inters logiques"
#define TR_MENU_TRAINER                 STR_CHAR_TRAINER "Ecolage"
#define TR_MENU_CHANNELS                STR_CHAR_CHANNEL "Canaux"
#define TR_MENU_GVARS                   STR_CHAR_SLIDER "Vars. glob."
#define TR_MENU_TELEMETRY               STR_CHAR_TELEMETRY "Télémétrie"
#define TR_MENU_DISPLAY                "AFFICHAGE"
#define TR_MENU_OTHER                  "Autres"
#define TR_MENU_INVERT                 "Inverser"
#define TR_JITTER_FILTER               "Filtre ADC"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Vérif. RTC", "Vérif. pile RTC")
#define TR_AUTH_FAILURE                "Auth-failure"
#define TR_RACING_MODE                 "Racing mode"

#define STR_VFR                       "VFR"
#define STR_RSSI                      "RSSI"
#define STR_R9PW                      "R9PW"
#define STR_RAS                       "SWR"
#define STR_A1                        "A1"
#define STR_A2                        "A2"
#define STR_A3                        "A3"
#define STR_A4                        "A4"
#define STR_BATT                      "BtRx"
#define STR_ALT                       "Alt"
#define STR_TEMP1                     "Tmp1"
#define STR_TEMP2                     "Tmp2"
#define STR_TEMP3                     "Tmp3"
#define STR_TEMP4                     "Tmp4"
#define STR_RPM2                      "RPM2"
#define STR_PRES                      "Pres"
#define STR_ODO1                      "Odo1"
#define STR_ODO2                      "Odo2"
#define STR_TXV                       "TX_V"
#define STR_CURR_SERVO1               "CSv1"
#define STR_CURR_SERVO2               "CSv2"
#define STR_CURR_SERVO3               "CSv3"
#define STR_CURR_SERVO4               "CSv4"
#define STR_DIST                      "Dist"
#define STR_ARM                       "Arm"
#define STR_C50                       "C50"
#define STR_C200                      "C200"
#define STR_RPM                       "RPM"
#define STR_FUEL                      "Fuel"
#define STR_VSPD                      "VSpd"
#define STR_ACCX                      "AccX"
#define STR_ACCY                      "AccY"
#define STR_ACCZ                      "AccZ"
#define STR_GYROX                     "GYRX"
#define STR_GYROY                     "GYRY"
#define STR_GYROZ                     "GYRZ"
#define STR_CURR                      "Curr"
#define STR_CAPACITY                  "Capa"
#define STR_VFAS                      "VFAS"
#define STR_BATT_PERCENT              "Bat%"
#define STR_ASPD                      "ASpd"
#define STR_GSPD                      "GSpd"
#define STR_HDG                       "Hdg"
#define STR_SATELLITES                "Sats"
#define STR_CELLS                     "Cels"
#define STR_GPSALT                    "GAlt"
#define STR_GPSDATETIME               "Date"
#define STR_BATT1_VOLTAGE             "RB1V"
#define STR_BATT2_VOLTAGE             "RB2V"
#define STR_BATT1_CURRENT             "RB1A"
#define STR_BATT2_CURRENT             "RB2A"
#define STR_BATT1_CONSUMPTION         "RB1C"
#define STR_BATT2_CONSUMPTION         "RB2C"
#define STR_BATT1_TEMP                "RB1T"
#define STR_BATT2_TEMP                "RB2T"
#define STR_RB_STATE                  "RBS"
#define STR_CHANS_STATE               "RBCS"
#define STR_RX_RSSI1                  "1RSS"
#define STR_RX_RSSI2                  "2RSS"
#define STR_RX_QUALITY                "RQly"
#define STR_RX_SNR                    "RSNR"
#define STR_RX_NOISE                  "RNse"
#define STR_RF_MODE                   "RFMD"
#define STR_TX_POWER                  "TPWR"
#define STR_TX_RSSI                   "TRSS"
#define STR_TX_QUALITY                "TQly"
#define STR_TX_SNR                    "TSNR"
#define STR_TX_NOISE                  "TNse"
#define STR_PITCH                     "Ptch"
#define STR_ROLL                      "Roll"
#define STR_YAW                       "Yaw"
#define STR_THROTTLE                  "Thr"
#define STR_QOS_A                     "FdeA"
#define STR_QOS_B                     "FdeB"
#define STR_QOS_L                     "FdeL"
#define STR_QOS_R                     "FdeR"
#define STR_QOS_F                     "FLss"
#define STR_QOS_H                     "Hold"
#define STR_LAP_NUMBER                "Lap "
#define STR_GATE_NUMBER               "Gate"
#define STR_LAP_TIME                  "LapT"
#define STR_GATE_TIME                 "GteT"
#define STR_ESC_VOLTAGE               "EscV"
#define STR_ESC_CURRENT               "EscA"
#define STR_ESC_RPM                   "Erpm"
#define STR_ESC_CONSUMPTION           "EscC"
#define STR_ESC_TEMP                  "EscT"
#define STR_SD1_CHANNEL               "Chan"
#define STR_GASSUIT_TEMP1             "GTp1"
#define STR_GASSUIT_TEMP2             "GTp2"
#define STR_GASSUIT_RPM               "GRPM"
#define STR_GASSUIT_FLOW              "GFlo"
#define STR_GASSUIT_CONS              "GFue"
#define STR_GASSUIT_RES_VOL           "GRVl"
#define STR_GASSUIT_RES_PERC          "GRPc"
#define STR_GASSUIT_MAX_FLOW          "GMFl"
#define STR_GASSUIT_AVG_FLOW          "GAFl"
#define STR_SBEC_VOLTAGE              "BecV"
#define STR_SBEC_CURRENT              "BecA"
#define STR_RB3040_EXTRA_STATE        "RBES"
#define STR_RB3040_CHANNEL1           "CH1A"
#define STR_RB3040_CHANNEL2           "CH2A"
#define STR_RB3040_CHANNEL3           "CH3A"
#define STR_RB3040_CHANNEL4           "CH4A"
#define STR_RB3040_CHANNEL5           "CH5A"
#define STR_RB3040_CHANNEL6           "CH6A"
#define STR_RB3040_CHANNEL7           "CH7A"
#define STR_RB3040_CHANNEL8           "CH8A"
#define STR_ESC_VIN                   "EVIN"
#define STR_ESC_TFET                  "TFET"
#define STR_ESC_CUR                   "ECUR"
#define STR_ESC_TBEC                  "TBEC"
#define STR_ESC_BCUR                  "CBEC"
#define STR_ESC_VBEC                  "VBEC"
#define STR_ESC_THR                   "ETHR"
#define STR_ESC_POUT                  "EOUT"
#define STR_SMART_BAT_BTMP            "BTmp"
#define STR_SMART_BAT_BCUR            "BCur"
#define STR_SMART_BAT_BCAP            "BUse"
#define STR_SMART_BAT_MIN_CEL         "CLMi"
#define STR_SMART_BAT_MAX_CEL         "CLMa"
#define STR_SMART_BAT_CYCLES          "Cycl"
#define STR_SMART_BAT_CAPACITY        "BCpT"
#define STR_CL01                      "Cel1"
#define STR_CL02                      "Cel2"
#define STR_CL03                      "Cel3"
#define STR_CL04                      "Cel4"
#define STR_CL05                      "Cel5"
#define STR_CL06                      "Cel6"
#define STR_CL07                      "Cel7"
#define STR_CL08                      "Cel8"
#define STR_CL09                      "Cel9"
#define STR_CL10                      "Cl10"
#define STR_CL11                      "Cl11"
#define STR_CL12                      "Cl12"
#define STR_CL13                      "Cl13"
#define STR_CL14                      "Cl14"
#define STR_CL15                      "Cl15"
#define STR_CL16                      "Cl16"
#define STR_CL17                      "Cl17"
#define STR_CL18                      "Cl18"
#define STR_FRAME_RATE                "FRat"
#define STR_TOTAL_LATENCY             "TLat"
#define STR_VTX_FREQ                  "VFrq"
#define STR_VTX_PWR                   "VPwr"
#define STR_VTX_CHAN                  "VChn"
#define STR_VTX_BAND                  "VBan"
#define STR_SERVO_CURRENT             "SrvA"
#define STR_SERVO_VOLTAGE             "SrvV"
#define STR_SERVO_TEMPERATURE         "SrvT"
#define STR_SERVO_STATUS              "SrvS"
#define STR_LOSS                      "Loss"
#define STR_SPEED                     "Spd "
#define STR_FLOW                      "Flow"

// The following content is Untranslated)

#define TR_USE_THEME_COLOR              "Use theme color"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Add all Trims to Subtrims"
#if LCD_W > LCD_H
  #define TR_OPEN_CHANNEL_MONITORS        "Open Channel Monitor"
#else
  #define TR_OPEN_CHANNEL_MONITORS        "Open Channel Mon."
#endif
#define TR_DUPLICATE                    "Duplicate"
#define TR_ACTIVATE                     "Set Active"
#define TR_RED                          "Red"
#define TR_BLUE                         "Blue"
#define TR_GREEN                        "Green"
#define TR_COLOR_PICKER                 "Color Picker"
#define TR_EDIT_THEME_DETAILS           "Edit Theme Details"
#define TR_AUTHOR                       "Author"
#define TR_DESCRIPTION                  "Description"
#define TR_SAVE                         "Save"
#define TR_CANCEL                       "Cancel"
#define TR_EDIT_THEME                   "EDIT THEME"
#define TR_DETAILS                      "Details"
#define TR_THEME_EDITOR                 "THEMES"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Select\nModel"
#define TR_MAIN_MENU_MODEL_NOTES        "Model\nNotes"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Channel\nMonitor"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Model\nSettings"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Radio\nSettings"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Screens\nSettings"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Reset\nTelemetry"
#define TR_MAIN_MENU_STATISTICS         "Statistics"
#define TR_MAIN_MENU_ABOUT_EDGETX       "About\nEdgeTX"
// End Main menu
