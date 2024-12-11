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

/* PT translations author: who?
 * PT translations by: Zandor.sp<zandor.sp@gmail.com>
 */
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

#define TR_MINUTE_SINGULAR            "minuto"
#define TR_MINUTE_PLURAL1             "minutos"
#define TR_MINUTE_PLURAL2             "minutos"

#define TR_OFFON                       "DESL","LIG"
#define TR_MMMINV                      "---","INV"
#define TR_VBEEPMODE                   "Mudo","Alarmes","Sem Botão","Todos"
#define TR_VBLMODE                     "DESL","Botões",TR("Ctrl","Controles"),"Ambos","Ligado"
#define TR_TRNMODE                     "DESL",TR("+=","Adicionar"),TR(":=","Trocar")
#define TR_TRNCHN                      "CH1","CH2","CH3","CH4"
#define TR_AUX_SERIAL_MODES            "DESL","Espelhar Telem","Entr Telem","Trainer SBUS","LUA","CLI","GPS","Debug","SpaceMouse","External module"
#define TR_SWTYPES                     "Nenhum","Tátil","2POS","3POS"
#define TR_POTTYPES                    "Nenhum","Pot",TR("Pot c. trav","Pot com trava"),"Slider",TR("Multipos","Chave Multipos"),"Eixo X","Eixo Y","Chave"
#define TR_VPERSISTENT                 "DESL","Voo","Reset Manual"
#define TR_COUNTRY_CODES               TR("US","América"),TR("JP","Japão"),TR("EU","Europa")
#define TR_USBMODES                    "Pergunta",TR("Joyst","Joystick"),TR("SDCard","Storage"),"Serial"
#define TR_JACK_MODES                  "Pergunta","Audio","Trainer"

#define TR_SBUS_INVERSION_VALUES       "normal","não invertido"
#define TR_MULTI_CUSTOM                "Custom"
#define TR_VTRIMINC                    "Expo","ExFino","Fino","Médio","Largo"
#define TR_VDISPLAYTRIMS               "Não","Trocar","Sim"
#define TR_VBEEPCOUNTDOWN              "Silêncioso","Bipes","Voz","Vibrar",TR("B & V","Bipes & Vibrar"),TR("V & V","Voz & Vibrar")
#define TR_COUNTDOWNVALUES             "5s","10s","20s","30s"
#define TR_VVARIOCENTER                "Tom","silêncio"
#define TR_CURVE_TYPES                 "Normal","Custom"

#define TR_ADCFILTERVALUES             "Global","DESL","LIG"

#define TR_VCURVETYPE                  "Diff","Expo","Func","Cstm"
#define TR_VCURVEFUNC                  "---","x>0","x<0","|x|","f>0","f<0","|f|"
#define TR_VMLTPX                      "Adicionar","Multipl.","Trocar"
#define TR_VMLTPX2                     "+=","*=",":="

#if LCD_W >= 212
  #define TR_CSWTIMER                  "Timer"
  #define TR_CSWSTICKY                 "Stcky"
  #define TR_CSWSTAY                   "Edge"
#else
  #define TR_CSWTIMER                  "Tim"
  #define TR_CSWSTICKY                 "Stky"
  #define TR_CSWSTAY                   "Edge"
#endif

#define TR_CSWEQUAL                    "a=x"
#define TR_VCSWFUNC                    "---",TR_CSWEQUAL,"a" STR_CHAR_TILDE "x","a>x","a<x","|a|>x","|a|<x","AND","OR","XOR",TR_CSWSTAY,"a=b","a>b","a<b",STR_CHAR_DELTA "≥x","|" STR_CHAR_DELTA "|≥x",TR_CSWTIMER,TR_CSWSTICKY

#define TR_SF_TRAINER                  "Trainer"
#define TR_SF_INST_TRIM                "Trim Inst."
#define TR_SF_RESET                    "Reset"
#define TR_SF_SET_TIMER                "Definir"
#define TR_SF_VOLUME                   "Volume"
#define TR_SF_FAILSAFE                 "DefFailsafe"
#define TR_SF_RANGE_CHECK              "RangeCheck"
#define TR_SF_MOD_BIND                 "BindMódulo"
#define TR_SF_RGBLEDS                  "Leds RGB"
 
#define TR_SOUND                       "Tocar Som"
#define TR_PLAY_TRACK                  TR("Ply Trk", "Tocar Trilha")
#define TR_PLAY_VALUE                  TR("RepValor","Rep Valor")
#define TR_SF_HAPTIC                   "Vibrar"
#define TR_SF_PLAY_SCRIPT              TR("Lua", "Script Lua")
#define TR_SF_BG_MUSIC                 "BgMusica"
#define TR_SF_BG_MUSIC_PAUSE           "BgMusica ||"
#define TR_SF_LOGS                     "Logs SD"
#define TR_ADJUST_GVAR                 "Ajuste"
#define TR_SF_BACKLIGHT                "LuzFundo"
#define TR_SF_VARIO                    "Vario"
#define TR_SF_TEST                     "Test"

#if LCD_W >= 212
  #define TR_SF_SAFETY                 "Sobrescrv"
#else
  #define TR_SF_SAFETY                 "Overr."
#endif

#define TR_SF_SCREENSHOT               "Capt. Tela"
#define TR_SF_RACING_MODE              "ModCorrida"
#define TR_SF_DISABLE_TOUCH            "No Touch"
#define TR_SF_DISABLE_AUDIO_AMP        "Amp Audio Desl"
#define TR_SF_SET_SCREEN               "Def Tela Princ"
#define TR_SF_SET_TELEMSCREEN          "Def Tela"
#define TR_SF_PUSH_CUST_SWITCH         "Push CS"
#define TR_SF_LCD_TO_VIDEO             "LCD to Video"

#define TR_FSW_RESET_TELEM             TR("Telm", "Telemetria")
#define TR_FSW_RESET_TRIMS             "Trims"
#define TR_FSW_RESET_TIMERS            "Tmr1","Tmr2","Tmr3"

#define TR_VFSWRESET                   TR_FSW_RESET_TIMERS,TR("All","Flight"),TR_FSW_RESET_TELEM,TR_FSW_RESET_TRIMS

#define TR_FUNCSOUNDS                  TR("Bp1","Bipe1"),TR("Bp2","Bipe2"),TR("Bp3","Bipe3"),TR("Wrn1","Avis1"),TR("Wrn2","Avis2"),TR("Chee","Cheep"),TR("Rata","Ratata"),"Tick",TR("Sirn","Siren"),"Ring",TR("SciF","SciFi"),TR("Robt","Robô"),TR("Chrp","Chirp"),"Tada",TR("Crck","Crickt"),TR("Alrm","AlmClk")

#define LENGTH_UNIT_IMP                "ft"
#define SPEED_UNIT_IMP                 "mph"
#define LENGTH_UNIT_METR               "m"
#define SPEED_UNIT_METR                "kmh"

#define TR_VUNITSSYSTEM                "Metric",TR("Imper.","Imperial")
#define TR_VTELEMUNIT                  "-","V","A","mA","kts","m/s","f/s","kmh","mph","m","ft","°C","°F","%","mAh","W","mW","dB","rpm","g","°","rad","ml","fOz","mlm","Hz","ms","us","km","dBm"

#define STR_V                          (STR_VTELEMUNIT[1])
#define STR_A                          (STR_VTELEMUNIT[2])

#define TR_VTELEMSCREENTYPE            "Nenhum","Números","Barras","Script"
#define TR_GPSFORMAT                   "DMS","NMEA"


#define TR_VSWASHTYPE                  "---","120","120X","140","90"

#define TR_STICK_NAMES0                "Lem"
#define TR_STICK_NAMES1                "Pfd"
#define TR_STICK_NAMES2                "Mot"
#define TR_STICK_NAMES3                "Ail"
#define TR_SURFACE_NAMES0              "Dir"
#define TR_SURFACE_NAMES1              "Mot"

#if defined(PCBNV14)
#define  TR_RFPOWER_AFHDS2             "Default","High"
#endif

#define TR_ROTARY_ENCODERS
#define TR_ROTENC_SWITCHES

#define TR_ON_ONE_SWITCHES             "LIG","UM"

#if defined(USE_HATS_AS_KEYS)
#define TR_HATSMODE                    "Modo menu"
#define TR_HATSOPT                     "Só Trims","Só menu","Chaveável","Global"
#define TR_HATSMODE_TRIMS              "Modo trim: Trims"
#define TR_HATSMODE_KEYS               "Modo trim: Menu"
#define TR_HATSMODE_KEYS_HELP          "Lado Esq:\n"\
                                       "   Direita = MDL\n"\
                                       "   Cima = SYS\n"\
                                       "   Baixo = TELE\n"\
                                       "\n"\
                                       "Lado Dir:\n"\
                                       "   Esquerda = PAGE<\n"\
                                       "   Direita = PAGE>\n"\
                                       "   Cima = PREV/INC\n"\
                                       "   Baixo = NEXT/DEC"
#endif

#if defined(COLORLCD)
  #define TR_ROTARY_ENC_OPT         "Normal","Invert"
#else
  #define TR_ROTARY_ENC_OPT         "Normal","Invert","V-I H-N","V-I H-A","V-N E-I"
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
#define TR_SRC_TIME                    "Time"
#define TR_SRC_GPS                     "GPS"
#define TR_SRC_TIMER                   "Tmr"

#define TR_VTMRMODES                   "DESL","LIGA","Strt","THs","TH%","THt"
#define TR_VTRAINER_MASTER_OFF         "DESL"
#define TR_VTRAINER_MASTER_JACK        "Mestre/Jack"
#define TR_VTRAINER_SLAVE_JACK         "Escrv/Jack"
#define TR_VTRAINER_MASTER_SBUS_MODULE "Mestre/SBUS Module"
#define TR_VTRAINER_MASTER_CPPM_MODULE "Mestre/CPPM Module"
#define TR_VTRAINER_MASTER_BATTERY     "Mestre/Serial"
#define TR_VTRAINER_BLUETOOTH          "Mestre/" TR("BT","Bluetooth"), "Escrv/" TR("BT","Bluetooth")
#define TR_VTRAINER_MULTI              "Mestre/Multi"
#define TR_VFAILSAFE                   "Nao def","Manter","Custom","Sem Pulsos","Receptor"
#define TR_VSENSORTYPES                "Custom","Calculado"
#define TR_VFORMULAS                   "Soma","Média","Min","Max","Multipl","Totalizar","Cell","Consumido","Distâcia"
#define TR_VPREC                       "0.--","0.0 ","0.00"
#define TR_VCELLINDEX                  "Menor","1","2","3","4","5","6","7","8","Maior","Delta"
#define TR_SUBTRIMMODES                STR_CHAR_DELTA " (center only)","= (symmetrical)"
#define TR_TIMER_DIR                   TR("Restan", "Restante"), TR("Passd.", "Passado")
#define TR_PPMUNIT                     "0.--","0.0","us"

#if defined(COLORLCD)
#if defined(BOLD)
#define TR_FONT_SIZES                  "STD"
#else
#define TR_FONT_SIZES                  "STD","BOLD","XXS","XS","L","XL","XXL"
#endif
#endif

#if defined(PCBFRSKY)
  #define TR_ENTER                     "[ENTER]"
#elif defined(PCBNV14) || defined(PCBPL18)
  #define TR_ENTER                     "[NEXT]"
#else
  #define TR_ENTER                     "[MENU]"
#endif

#if defined(COLORLCD)
  #define TR_EXIT                      "RTN"
  #define TR_OK                        TR_ENTER
#else
  #define TR_EXIT                      "EXIT"
  #define TR_OK                        TR("\010" "\010" "\010" "[OK]", "\010" "\010" "\010" "\010" "\010" "[OK]")
#endif

#if defined(PCBTARANIS)
  #define TR_POPUPS_ENTER_EXIT         TR(TR_EXIT "\010" TR_ENTER, TR_EXIT "\010" "\010" "\010" "\010" TR_ENTER)
#else
  #define TR_POPUPS_ENTER_EXIT         TR_ENTER "\010" TR_EXIT
#endif

#define TR_FREE                        "Livre"
#define TR_YES                         "SIM"
#define TR_NO                          "NAO"
#define TR_DELETEMODEL                 "APAGAR MODELO"
#define TR_COPYINGMODEL                "Copiando modelo.."
#define TR_MOVINGMODEL                 "Movendo modelo..."
#define TR_LOADINGMODEL                "Carreg. model..."
#define TR_UNLABELEDMODEL              "Sem Etiqueta"
#define TR_NAME                        "Nome"
#define TR_MODELNAME                   "Nome modelo"
#define TR_PHASENAME                   "Nome modo"
#define TR_MIXNAME                     "Nome mix"
#define TR_INPUTNAME                   TR("Entrd", "Nome entrada")
#define TR_EXPONAME                    TR("Nome", "Nome linha")
#define TR_BITMAP                      "Img modelo"
#define TR_NO_PICTURE                  "Sem foto"
#define TR_TIMER                       TR("Crono", "Cronômetro")
#define TR_START                       "Iniciar"
#define TR_ELIMITS                     TR("E.Limits", "Limites extend.")
#define TR_ETRIMS                      TR("E.Trims", "Trims extend.")
#define TR_TRIMINC                     "PassoTrim"
#define TR_DISPLAY_TRIMS               TR("Exb Trims", "Exibir trims")
#define TR_TTRACE                      TR("T-Source", "Source")
#define TR_TTRIM                       TR("T-Trim-Idle", "Trim idle only")
#define TR_TTRIM_SW                    TR("T-Trim-Sw", "Trim switch")
#define TR_BEEPCTR                     TR("Bip Cntr", "Bipe quando centralizado")
#define TR_USE_GLOBAL_FUNCS            TR("Funcs.Glob", "Usar func.global")
#define TR_PROTOCOL                    TR("Proto", "Protocolo")
  #define TR_PPMFRAME                  "frame PPM"
  #define TR_REFRESHRATE               TR("Refresh", "Refresh rate")
  #define STR_WARN_BATTVOLTAGE         TR("Saída em VBAT: ", "AVISO: Tensão de saída em VBAT: ")
#define TR_WARN_5VOLTS                 "AVISO: Tensão de saída em 5 volts"
#define TR_MS                          "ms"
#define TR_SWITCH                      "Chave"
#define TR_FUNCTION_SWITCHES           "Chaves customizáveis"
#define TR_FS_COLOR_LIST               "White","Red","Green","Yellow","Orange","Blue","Pink","Off","Custom"
#define TR_GROUP                       "Group"
#define TR_GROUP_ALWAYS_ON             "Always on"
#define TR_GROUPS                      "Always on groups"
#define TR_LAST                        "Last"
#define TR_MORE_INFO                   "More info"
#define TR_SWITCH_TYPE                 "Type"
#define TR_SWITCH_STARTUP              "Startup"
#define TR_SWITCH_GROUP                "Group"
#define TR_FUNCTION_SWITCH_GROUPS      "---", TR_SWITCH_GROUP" 1", TR_SWITCH_GROUP" 2", TR_SWITCH_GROUP" 3"
#define TR_SF_SWITCH                   "Disparo"
#define TR_TRIMS                       "Trims"
#define TR_FADEIN                      "Aparecer"
#define TR_FADEOUT                     "Desaparecer"
#define TR_DEFAULT                     "(padrão)"
#if defined(COLORLCD)
  #define TR_CHECKTRIMS                "Check FM Trims"
#else
  #define TR_CHECKTRIMS                "\006Check\012trims"
#endif
#define TR_SWASHTYPE                   "Tipo do Swash"
#define TR_COLLECTIVE                  TR("Coletivo", "Org. passo do Coletivo")
#define TR_AILERON                     TR("Rolagem", "Org. cíclico Rolagem")
#define TR_ELEVATOR                    TR("Arfagem", "Org. cíclico Arfagem")
#define TR_SWASHRING                   "Swash Ring"
#define TR_MODE                        "Modo"
#if !PORTRAIT_LCD
  #define TR_LEFT_STICK                "Esq"
#else
  #define TR_LEFT_STICK                "Esq"
#endif
#define TR_SUBTYPE                     "Subtype"
#define TR_NOFREEEXPO                  "No free expo!"
#define TR_NOFREEMIXER                 "No free mixer!"
#define TR_SOURCE                      "Fonte"
#define TR_WEIGHT                      "Peso"
#define TR_SIDE                        "Lado"
#define TR_OFFSET                      "Desvio"
#define TR_TRIM                        "Trim"
#define TR_DREX                        "DRex"
#define DREX_CHBOX_OFFSET              30
#define TR_CURVE                       "Curva"
#define TR_FLMODE                      TR("Modo", "Modos")
#define TR_MIXWARNING                  "Alerta"
#define TR_OFF                         "DESL"
#define TR_ANTENNA                     "Antena"
#define TR_NO_INFORMATION              TR("No info", "No information")
#define TR_MULTPX                      "Multiplex"
#define TR_DELAYDOWN                   TR("Delay dn", "Delay down")
#define TR_DELAYUP                     "Delay up"
#define TR_SLOWDOWN                    TR("Slow dn", "Slow down")
#define TR_SLOWUP                      "Slow up"
#define TR_MIXES                       "MIXES"
#define TR_CV                          "CV"
#if defined(PCBNV14) || defined(PCBPL18)
#define TR_GV                          "GV"
#else
#define TR_GV                          TR("G", "GV")
#endif
#define TR_RANGE                       "Alcance"
#define TR_CENTER                      "Centro"
#define TR_ALARM                       "Alarme"
#define TR_BLADES                      "Lâminas/Pás"
#define TR_SCREEN                      "Tela\001"
#define TR_SOUND_LABEL                 "Som"
#define TR_LENGTH                      "Comprimento"
#define TR_BEEP_LENGTH                 "Dur. bipe"
#define TR_BEEP_PITCH                  "Timbre bipe"
#define TR_HAPTIC_LABEL                "Vibrar"
#define TR_STRENGTH                    "Forca"
#define TR_IMU_LABEL                   "IMU"
#define TR_IMU_OFFSET                  "Offset"
#define TR_IMU_MAX                     "Max"
#define TR_CONTRAST                    "Contrast"
#define TR_ALARMS_LABEL                "Alarmes"
#define TR_BATTERY_RANGE               TR("Faixa Batt.", "Faixa Med. Bateria")
#define TR_BATTERYCHARGING             "Carregando..."
#define TR_BATTERYFULL                 "Batt. Cheia"
#define TR_BATTERYNONE                 "Nenhum!"
#define TR_BATTERYWARNING              "Bat. Baixa"
#define TR_INACTIVITYALARM             "Inatividade"
#define TR_MEMORYWARNING               "Memory low"
#define TR_ALARMWARNING                "Som desligado"
#define TR_RSSI_SHUTDOWN_ALARM         TR("RSSI ao desl.", "Checar RSSI ao desligar")
#define TR_TRAINER_SHUTDOWN_ALARM      TR("Trainer ao desl", "Checar trainer ao desligar")
#define TR_MODEL_STILL_POWERED         "Modelo ainda ligado"
#define TR_TRAINER_STILL_CONNECTED     "Trainer ainda conectado"
#define TR_USB_STILL_CONNECTED         "USB ainda ligado"
#define TR_MODEL_SHUTDOWN              "Desligar?"
#define TR_PRESS_ENTER_TO_CONFIRM      "Press enter p/ confirmar"
#define TR_THROTTLE_LABEL              "Motor"
#define TR_THROTTLE_START              "Iniciar Motor"
#define TR_THROTTLEREVERSE             TR("T-Reverso", "Reverso")
#define TR_MINUTEBEEP                  TR("Minuto", "Cada Minuto")
#define TR_BEEPCOUNTDOWN               "ContagemRegr"
#define TR_PERSISTENT                  TR("Persist.", "Persistente")
#define TR_BACKLIGHT_LABEL             "Backlight"
#define TR_GHOST_MENU_LABEL            "GHOST MENU"
#define TR_STATUS                      "Status"
#define TR_BLONBRIGHTNESS              "Brilho LIGA"
#define TR_BLOFFBRIGHTNESS             "Brilho DESL"
#define TR_KEYS_BACKLIGHT              "Keys backlight"
#define TR_BLCOLOR                     "Cor"
#define TR_SPLASHSCREEN                "Tela abertura"
#define TR_PLAY_HELLO                  "Som ao ligar"
#define TR_PWR_ON_DELAY                "Delay para LIGA"
#define TR_PWR_OFF_DELAY               "Delay para DESL"
#define TR_PWR_AUTO_OFF                TR("Pwr Auto Off","Power Auto Off")
#define TR_PWR_ON_OFF_HAPTIC           TR("Vibra ao LIG/DESL", "Vibrar ao Lig/Desl")
#define TR_THROTTLE_WARNING            TR("A-Aceler.", "Pos do acelerador")
#define TR_CUSTOM_THROTTLE_WARNING     TR("Cust-Pos", "Custom position?")
#define TR_CUSTOM_THROTTLE_WARNING_VAL TR("Pos. %", "Position %")
#define TR_SWITCHWARNING               TR("A-Chaves", "Pos das chaves")
#define TR_POTWARNINGSTATE             "Pots & sliders"
#define TR_POTWARNING                  TR("Pot warn.", "Pos dos Pots")
#define TR_TIMEZONE                    "Fuso horário"
#define TR_ADJUST_RTC                  "Ajuste RTC"
#define TR_GPS                         "GPS"
#define TR_DEF_CHAN_ORD                TR("Ordem canais", "Ordem padrão canais")
#define TR_STICKS                      "Eixos"
#define TR_POTS                        "Pots"
#define TR_SWITCHES                    "Chaves"
#define TR_SWITCHES_DELAY              TR("Play delay", "Play delay (sw. mid pos)")
#define TR_SLAVE                       "Escravo"
#define TR_MODESRC                     "Mode\006% Source"
#define TR_MULTIPLIER                  "Multiplier"
#define TR_CAL                         "Cal"
#define TR_CALIBRATION                 "Calibração"
#define TR_VTRIM                       "Trim - +"
#define TR_CALIB_DONE                  "Calibração completa"
#if defined(PCBHORUS)
  #define TR_MENUTOSTART               "Press [Enter] p/ iniciar"
  #define TR_SETMIDPOINT               "Centrar eixos/pots/sliders e press. [Enter]"
  #define TR_MOVESTICKSPOTS            "Mova eixos/pots/sliders e press. [Enter]"
#elif defined(COLORLCD)
  #define TR_MENUTOSTART               TR_ENTER " P/ INICIAR"
  #define TR_SETMIDPOINT               "CENTRE EIXO/SLIDERS"
  #define TR_MOVESTICKSPOTS            "MOVA EIXO/POTS"
#else
  #define TR_MENUTOSTART               TR_ENTER " P/ INICIAR"
#if defined(SURFACE_RADIO)
  #define TR_SETMIDPOINT               "SET MEIO DOS POTS"
  #define TR_MOVESTICKSPOTS            "MOVA DIR/MOT/POTS/AXIS"
#else
  #define TR_SETMIDPOINT               TR("SET EIXO MEIO", "CENTRE AXIS/SLIDERS")
  #define TR_MOVESTICKSPOTS            "MOVA AXIS/POTS"
#endif
  #define TR_MENUWHENDONE              TR_ENTER " QUANDO TERM."
#endif
#define TR_TXnRX                       "Tx:\0Rx:"
#define OFS_RX                         4
#define TR_NODATA                      "SEM DADOS"
#define TR_US                          "us"
#define TR_HZ                          "Hz"
#define TR_TMIXMAXMS                   "Tmix max"
#define TR_FREE_STACK                  "Free stack"
#define TR_INT_GPS_LABEL               "Internal GPS"
#define TR_HEARTBEAT_LABEL             "Heartbeat"
#define TR_LUA_SCRIPTS_LABEL           "Lua scripts"
#define TR_FREE_MEM_LABEL              "Mem livre"
#define TR_DURATION_MS             TR("[D]","Duration(ms): ")
#define TR_INTERVAL_MS             TR("[I]","Interval(ms): ")
#define TR_MEM_USED_SCRIPT         "Script(B): "
#define TR_MEM_USED_WIDGET         "Widget(B): "
#define TR_MEM_USED_EXTRA          "Extra(B): "
#define TR_STACK_MIX                   "Mix: "
#define TR_STACK_AUDIO                 "Audio: "
#define TR_GPS_FIX_YES                 "Fix: Yes"
#define TR_GPS_FIX_NO                  "Fix: No"
#define TR_GPS_SATS                    "Sats: "
#define TR_GPS_HDOP                    "Hdop: "
#define TR_STACK_MENU                  "Menu: "
#define TR_TIMER_LABEL                 "Timer"
#define TR_THROTTLE_PERCENT_LABEL      "Throttle %"
#define TR_BATT_LABEL                  "Battery"
#define TR_SESSION                     "Session"
#define TR_MENUTORESET                 TR_ENTER " para reset"
#define TR_PPM_TRAINER                 "TR"
#define TR_CH                          "CH"
#define TR_MODEL                       "MODEL"
#if defined(SURFACE_RADIO)
#define TR_FM                          "MD"
#else
#define TR_FM                          "FM"
#endif
#define TR_EEPROMLOWMEM                "EEPROM low mem"
#define TR_PRESS_ANY_KEY_TO_SKIP       "Press tecla p/ sair"
#define TR_THROTTLE_NOT_IDLE           "Acelerador ativo"
#define TR_ALARMSDISABLED              "Alarmes desativado"
#define TR_PRESSANYKEY                 TR("\010Press any Key", "Press any key")
#define TR_BADEEPROMDATA               "EEPROM INVÁLIDA"
#define TR_BAD_RADIO_DATA              "Dados inválidos do rádio"
#define TR_RADIO_DATA_RECOVERED        TR3("Using backup radio data","Using backup radio settings","Radio settings recovered from backup")
#define TR_RADIO_DATA_UNRECOVERABLE    TR3("Radio settings invalid","Radio settings not valid", "Unable to read valid radio settings")
#define TR_EEPROMFORMATTING            "Formatando EEPROM"
#define TR_STORAGE_FORMAT              "Preparando armazenamento"
#define TR_EEPROMOVERFLOW              "EEPROM overflow"
#define TR_RADIO_SETUP                 "CONF. RÁDIO"
#define TR_MENUTRAINER                 "TRAINER"
#define TR_MENUSPECIALFUNCS            "FUNC. GLOBAIS"
#define TR_MENUVERSION                 "VERSÃO"
#define TR_MENU_RADIO_SWITCHES         TR("CHAVES", "TEST CHAVES")
#define TR_MENU_RADIO_ANALOGS          TR("ANALOGS", "TEST ANALOG")
#define TR_MENU_RADIO_ANALOGS_CALIB    "ANALOGS CALIBR."
#define TR_MENU_RADIO_ANALOGS_RAWLOWFPS "ANALOGS BRUTOS(5 Hz)"
#define TR_MENUCALIBRATION             "CALIBRAÇÃO"
#define TR_MENU_FSWITCH                "BOTÃO CUSTOMIZAVEL"
#if defined(COLORLCD)
  #define TR_TRIMS2OFFSETS             "Trims => Subtrims"
#else
  #define TR_TRIMS2OFFSETS             "\006Trims => Subtrims"
#endif
#define TR_CHANNELS2FAILSAFE           "Chanais=>Failsafe"
#define TR_CHANNEL2FAILSAFE            "Chanal=>Failsafe"
#define TR_MENUMODELSEL                TR("SEL MODELO", "SELEÇÃO MODELO")
#define TR_MENU_MODEL_SETUP            TR("CONFIG", "CONFIG. MODELO")
#if defined(SURFACE_RADIO)
#define TR_MENUFLIGHTMODES             "MODOS DIR"
#define TR_MENUFLIGHTMODE              "MODO DIR"
#else
#define TR_MENUFLIGHTMODES             "MODOS VOO"
#define TR_MENUFLIGHTMODE              "MODO DE VOO"
#endif
#define TR_MENUHELISETUP               "CONFIG. HELI"

  #define TR_MENUINPUTS                "ENTRADAS"
  #define TR_MENULIMITS                "SAÍDAS"
#define TR_MENUCURVES                  "CURVAS"
#define TR_MENUCURVE                   "CURVA"
#define TR_MENULOGICALSWITCH           "CHAVE LÓGICA"
#define TR_MENULOGICALSWITCHES         "CHAVES LÓGICAS"
#define TR_MENUCUSTOMFUNC              "FUNÇÃO ESPECIAL"
#define TR_MENUCUSTOMSCRIPTS           "SCRIPTS CUSTOM"
#define TR_MENUTELEMETRY               "TELEMETRIA"
#define TR_MENUSTAT                    "STATS"
#define TR_MENUDEBUG                   "DEBUG"
#define TR_MONITOR_CHANNELS1           "MONITOR CANAIS 1-8"
#define TR_MONITOR_CHANNELS2           "MONITOR CANAIS 9-16"
#define TR_MONITOR_CHANNELS3           "MONITOR CANAIS 17-24"
#define TR_MONITOR_CHANNELS4           "MONITOR CANAIS 25-32"
#define TR_MONITOR_SWITCHES            "MONITOR CHAV. LÓGICAS"
#define TR_MONITOR_OUTPUT_DESC         "Saídas"
#define TR_MONITOR_MIXER_DESC          "Mixers"
  #define TR_RECEIVER_NUM              TR("# Rx", "# Recept")
  #define TR_RECEIVER                  "Receptor"
#define TR_MULTI_RFTUNE                TR("Ajst Freq", "Ajuste fino Freq. RF")
#define TR_MULTI_RFPOWER               "RF power"
#define TR_MULTI_WBUS                  "Output"
#define TR_MULTI_TELEMETRY             "Telemetria"
#define TR_MULTI_VIDFREQ               TR("Freq. vid.", "Frequencia video")
#define TR_RF_POWER                    "Pot. RF"
#define TR_MULTI_FIXEDID               TR("IDFixo", "ID Fixo")
#define TR_MULTI_OPTION                TR("Opção", "Valor opção")
#define TR_MULTI_AUTOBIND              TR("Bind Ch.", "Bind on channel")
#define TR_DISABLE_CH_MAP              TR("No Ch. map", "Disable Ch. map")
#define TR_DISABLE_TELEM               TR("No Telem", "Disable Telemetry")
#define TR_MULTI_LOWPOWER              TR("Low power", "Low power mode")
#define TR_MULTI_LNA_DISABLE           "LNA disable"
#define TR_MODULE_TELEMETRY            TR("S.Port", "S.Port link")
#define TR_MODULE_TELEM_ON             TR("ON", "Enabled")
#define TR_DISABLE_INTERNAL            TR("Desatv RF intern", "Desatv RF interno")
#define TR_MODULE_NO_SERIAL_MODE       TR("!serial mode", "Not in serial mode")
#define TR_MODULE_NO_INPUT             TR("No input", "No serial input")
#define TR_MODULE_NO_TELEMETRY         TR3("Sem telemetr", "Sem MULTI_TELEMETRY", "Nenh. MULTI_TELEMETRY detectado")
#define TR_MODULE_WAITFORBIND          "Bind to load protocol"
#define TR_MODULE_BINDING              TR("Bind...","Bindando")
#define TR_MODULE_UPGRADE_ALERT        TR3("Upg. needed", "Module upgrade required", "Module\nUpgrade required")
#define TR_MODULE_UPGRADE              TR("Upg. advised", "Module update recommended")
#define TR_REBIND                      "Rebinding required"
#define TR_REG_OK                      "Registration ok"
#define TR_BIND_OK                     "Bind successful"
#define TR_BINDING_CH1_8_TELEM_ON      "Ch1-8 Telem LIGA"
#define TR_BINDING_CH1_8_TELEM_OFF     "Ch1-8 Telem DESL"
#define TR_BINDING_CH9_16_TELEM_ON     "Ch9-16 Telem LIGA"
#define TR_BINDING_CH9_16_TELEM_OFF    "Ch9-16 Telem DESL"
#define TR_PROTOCOL_INVALID            TR("Prot inválido", "Protocol inválido")
#define TR_MODULE_STATUS               TR("Status", "Module Status")
#define TR_MODULE_SYNC                 TR("Sync", "Proto Sync Status")
#define TR_MULTI_SERVOFREQ             TR("Servo rate", "Servo update rate")
#define TR_MULTI_MAX_THROW             TR("Max. Throw", "Enable max. throw")
#define TR_MULTI_RFCHAN                TR("RF Channel", "Select RF channel")
#define TR_AFHDS3_RX_FREQ              TR("RX freq.", "RX frequency")
#define TR_AFHDS3_ONE_TO_ONE_TELEMETRY TR("Unicast/Tel.", "Unicast/Telemetry")
#define TR_AFHDS3_ONE_TO_MANY          "Multicast"
#define TR_AFHDS3_ACTUAL_POWER         TR("Act. pow", "Actual power")
#define TR_AFHDS3_POWER_SOURCE         TR("Power src.", "Power source")
#define TR_FLYSKY_TELEMETRY            TR("FlySky RSSI #", "Use FlySky RSSI value without rescalling")
#define TR_GPS_COORDS_FORMAT           TR("GPS Coords", "Coordinate format")
#define TR_VARIO                       TR("Vario", "Variometer")
#define TR_PITCH_AT_ZERO               "Pitch zero"
#define TR_PITCH_AT_MAX                "Pitch max"
#define TR_REPEAT_AT_ZERO              "Repeat zero"
#define TR_BATT_CALIB                  TR("Batt. calib", "Battery calibration")
#define TR_CURRENT_CALIB               "Current calib"
#define TR_VOLTAGE                     TR("Tensão", "Origem tensão")
#define TR_SELECT_MODEL                "Selec. modelo"
#define TR_MANAGE_MODELS               "MANAGE MODELS"
#define TR_MODELS                      "Modelos"
#define TR_SELECT_MODE                 "Seleção de modo"
#define TR_CREATE_MODEL                "Criar Modelo"
#define TR_FAVORITE_LABEL              "Favoritos"
#define TR_MODELS_MOVED                "Unused models moved to"
#define TR_NEW_MODEL                   "Novo Modelo"
#define TR_INVALID_MODEL               "Modelo inválido"
#define TR_EDIT_LABELS                 "Editar Etiquetas"
#define TR_LABEL_MODEL                 "Etiquetar modelo"
#define TR_MOVE_UP                     "Mover cima"
#define TR_MOVE_DOWN                   "Mover baixo"
#define TR_ENTER_LABEL                 "Entrar Etiqueta"
#define TR_LABEL                       "Etiqueta"
#define TR_LABELS                      "Etiquetas"
#define TR_CURRENT_MODEL               "Atual"
#define TR_ACTIVE                      "Ativo"
#define TR_NEW                         "Novo"
#define TR_NEW_LABEL                   "Nova Etiqueta"
#define TR_RENAME_LABEL                "Renm Etiqueta"
#define TR_DELETE_LABEL                "Apag Etiqueta"
#define TR_DUPLICATE_MODEL             "Duplicar Modelo"
#define TR_COPY_MODEL                  "Copiar Modelo"
#define TR_MOVE_MODEL                  "Mover modelo"
#define TR_BACKUP_MODEL                "Backup Modelo"
#define TR_DELETE_MODEL                "Apagar Modelo"
#define TR_RESTORE_MODEL               "Restaura Modelo"
#define TR_DELETE_ERROR                "Erro ao apagar"
#define TR_SDCARD_ERROR                TR("Erro SD", "Cartao SD erro")
#define TR_SDCARD                      "Cartão SD"
#define TR_NO_FILES_ON_SD              "Sem aquiv. no SD!"
#define TR_NO_SDCARD                   "Sem Cart. SD"
#define TR_WAITING_FOR_RX              "Waiting for RX..."
#define TR_WAITING_FOR_TX              "Waiting for TX..."
#define TR_WAITING_FOR_MODULE          TR("Waiting module", "Waiting for module...")
#define TR_NO_TOOLS                    "No tools available"
#define TR_NORMAL                      "Normal"
#define TR_NOT_INVERTED                "Not inv"
#define TR_NOT_CONNECTED               TR("!Conectado", "Nao Conectado");
#define TR_CONNECTED                   "Connected"
#define TR_FLEX_915                    "Flex 915MHz"
#define TR_FLEX_868                    "Flex 868MHz"
#define TR_16CH_WITHOUT_TELEMETRY      TR("16CH without telem.", "16CH without telemetry")
#define TR_16CH_WITH_TELEMETRY         TR("16CH with telem.", "16CH with telemetry")
#define TR_8CH_WITH_TELEMETRY          TR("8CH with telem.", "8CH with telemetry")
#define TR_EXT_ANTENNA                 "Ext. antenna"
#define TR_PIN                         "Pin"
#define TR_UPDATE_RX_OPTIONS           "Update RX options?"
#define TR_UPDATE_TX_OPTIONS           "Update TX options?"
#define TR_MODULES_RX_VERSION          "Módulos / Vers. RX"
#define TR_SHOW_MIXER_MONITORS         "Show mixer monitors"
#define TR_MENU_MODULES_RX_VERSION     "MODULOS / Vers. RX"
#define TR_MENU_FIRM_OPTIONS           "OPCOES FIRMWARE"
#define TR_IMU                         "IMU"
#define TR_STICKS_POTS_SLIDERS         "Eixos/Pots/Sliders"
#define TR_PWM_STICKS_POTS_SLIDERS     "Eixo PWM/Pots/Sliders"
#define TR_RF_PROTOCOL                 "RF Protocol"
#define TR_MODULE_OPTIONS              "Module options"
#define TR_POWER                       "Potência"
#define TR_NO_TX_OPTIONS               "No TX options"
#define TR_RTC_BATT                    "Bat. RTC"
#define TR_POWER_METER_EXT             "Power Meter (EXT)"
#define TR_POWER_METER_INT             "Power Meter (INT)"
#define TR_SPECTRUM_ANALYSER_EXT       "Espectro (EXT)"
#define TR_SPECTRUM_ANALYSER_INT       "Espectro (INT)"
#define TR_SDCARD_FULL                 "Cart SD cheio"
#if defined(COLORLCD)
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\nLogs e captura de tela desativados"
#else
#define TR_SDCARD_FULL_EXT             TR_SDCARD_FULL "\036Logs e captura" LCDW_128_LINEBREAK "de tela desativados"
#endif
#define TR_NEEDS_FILE                  "NEEDS FILE"
#define TR_EXT_MULTI_SPEC              "opentx-inv"
#define TR_INT_MULTI_SPEC              "stm-opentx-noinv"
#define TR_INCOMPATIBLE                "Incompativel"
#define TR_WARNING                     "AVISO"
#define TR_STORAGE_WARNING             "STORAGE"
#define TR_THROTTLE_UPPERCASE          "ACELERAD"
#define TR_ALARMSWARN                  "ALARMS"
#define TR_SWITCHWARN                  TR("SWITCH", "CONTROL")
#define TR_FAILSAFEWARN                "FAILSAFE"
#define TR_TEST_WARNING                TR("TESTING", "TEST BUILD")
#define TR_TEST_NOTSAFE                "Use for tests only"
#define TR_WRONG_SDCARDVERSION         TR("Expected ver: ", "Expected version: ")
#define TR_WARN_RTC_BATTERY_LOW        "RTC Battery low"
#define TR_WARN_MULTI_LOWPOWER         "Low power mode"
#define TR_BATTERY                     "BATTERY"
#define TR_WRONG_PCBREV                "Wrong PCB detected"
#define TR_EMERGENCY_MODE              "EMERGENCY MODE"
#define TR_NO_FAILSAFE                 "Failsafe not set"
#define TR_KEYSTUCK                    "Key stuck"
#define TR_VOLUME                      "Volume"
#define TR_LCD                         "LCD"
#define TR_BRIGHTNESS                  "Brilho"
#define TR_CPU_TEMP                    "CPU temp."
#define TR_COPROC                      "CoProc."
#define TR_COPROC_TEMP                 "MB Temp."
#define TR_TTL_WARNING                 "Aviso: Não exceder 3,3V nos pinos TX/RX !"
#define TR_FUNC                        "Função"
#define TR_V1                          "V1"
#define TR_V2                          "V2"
#define TR_DURATION                    "Duração"
#define TR_DELAY                       "Atraso"
#define TR_SD_CARD                     "CARTÃO SD"
#define TR_SDHC_CARD                   "CARTÃO SD-HC"
#define TR_NO_SOUNDS_ON_SD             "Sem Som no SD"
#define TR_NO_MODELS_ON_SD             "Sem Modelo no SD"
#define TR_NO_BITMAPS_ON_SD            "No bitmaps on SD"
#define TR_NO_SCRIPTS_ON_SD            "No scripts on SD"
#define TR_SCRIPT_SYNTAX_ERROR         TR("Syntax error", "Script syntax error")
#define TR_SCRIPT_PANIC                "Script panic"
#define TR_SCRIPT_KILLED               "Script killed"
#define TR_SCRIPT_ERROR                "Unknown error"
#define TR_PLAY_FILE                   "Tocar"
#define TR_DELETE_FILE                 "Apagar"
#define TR_COPY_FILE                   "Copiar"
#define TR_RENAME_FILE                 "Renomear"
#define TR_ASSIGN_BITMAP               "Assign bitmap"
#define TR_ASSIGN_SPLASH               "Tela abertura"
#define TR_EXECUTE_FILE                "Executar"
#define TR_REMOVED                     " removido"
#define TR_SD_INFO                     "Informação"
#define TR_NA                          "N/A"
#define TR_HARDWARE                    "HARDWARE"
#define TR_FORMATTING                  "Formatando..."
#define TR_TEMP_CALIB                  "Temp. Calib"
#define TR_TIME                        "Hora"
#define TR_MAXBAUDRATE                 "Bauds Max"
#define TR_BAUDRATE                    "Baudrate"
#define TR_CRSF_ARMING_MODE            TR("Armar via", "Armar usando")
#define TR_CRSF_ARMING_MODES           TR_CH"5", TR_SWITCH
#define TR_SAMPLE_MODE                 TR("Sampling","Sample Mode")
#define TR_SAMPLE_MODES                "Normal","OneBit"
#define TR_LOADING                     "Carregando..."
#define TR_DELETE_THEME                "Apagar Tema?"
#define TR_SAVE_THEME                  "Salvar Tema?"
#define TR_EDIT_COLOR                  "Editar Cor"
#define TR_NO_THEME_IMAGE              "Sem image tema"
#define TR_BACKLIGHT_TIMER             "Timeout inativ"

#if defined(COLORLCD)
  #define TR_MODEL_QUICK_SELECT        "Sel rapida modelo"
  #define TR_LABELS_SELECT             "Label select"
  #define TR_LABELS_MATCH              "Label matching"
  #define TR_FAV_MATCH                 "Favorites matching"
  #define TR_LABELS_SELECT_MODE        "Multi select","Single select"
  #define TR_LABELS_MATCH_MODE         "Match all","Match any"
  #define TR_FAV_MATCH_MODE            "Must match","Optional match"
#endif

#define TR_SELECT_TEMPLATE_FOLDER      "Select a template folder"
#define TR_SELECT_TEMPLATE             "SELECT A MODEL TEMPLATE:"
#define TR_NO_TEMPLATES                "No model templates were found in this folder"
#define TR_SAVE_TEMPLATE               "Save as template"
#define TR_BLANK_MODEL                 "Blank Model"
#define TR_BLANK_MODEL_INFO            "Create a blank model"
#define TR_FILE_EXISTS                 "FILE ALREADY EXISTS"
#define TR_ASK_OVERWRITE               "Do you want to overwrite?"

#define TR_BLUETOOTH                   "Bluetooth"
#define TR_BLUETOOTH_DISC              "Discover"
#define TR_BLUETOOTH_INIT              "Init"
#define TR_BLUETOOTH_DIST_ADDR         "Dist addr"
#define TR_BLUETOOTH_LOCAL_ADDR        "Local addr"
#define TR_BLUETOOTH_PIN_CODE          "PIN code"
#define TR_BLUETOOTH_NODEVICES         "No Devices Found"
#define TR_BLUETOOTH_SCANNING          "Scanning..."
#define TR_BLUETOOTH_BAUDRATE          "BT Baudrate"
#if defined(PCBX9E)
#define TR_BLUETOOTH_MODES             "---","Enabled"
#else
#define TR_BLUETOOTH_MODES             "---","Telemetry","Trainer"
#endif

#define TR_SD_INFO_TITLE               "SD INFO"
#define TR_SD_SPEED                    "Vel:"
#define TR_SD_SECTORS                  "Sectores:"
#define TR_SD_SIZE                     "Tam:"
#define TR_TYPE                        "Tipo"
#define TR_GLOBAL_VARS                 "Variáveis Global"
#define TR_GVARS                       "GVARS"
#define TR_GLOBAL_VAR                  "Variável Global"
#define TR_MENU_GLOBAL_VARS              "VARIÁVEIS GLOBAIS"
#define TR_OWN                         "Own"
#define TR_DATE                        "Data"
#define TR_MONTHS                      { "Jan", "Fev", "Mar", "Abr", "Mai", "Jun", "Jul", "Ago", "Set", "Out", "Nov", "Dez" }
#define TR_ROTARY_ENCODER              "R.E."
#define TR_ROTARY_ENC_MODE             TR("Modo RotEnc","Modo Rotary Encoder")
#define TR_CHANNELS_MONITOR            "MONITOR CANAIS"
#define TR_MIXERS_MONITOR              "MONITOR MIXERS"
#define TR_PATH_TOO_LONG               "Path too long"
#define TR_VIEW_TEXT                   "View text"
#define TR_FLASH_BOOTLOADER            "Flash bootloader"
#define TR_FLASH_DEVICE                TR("Flash device","Flash device")
#define TR_FLASH_EXTERNAL_DEVICE       TR("Flash S.Port", "Flash S.Port device")
#define TR_FLASH_RECEIVER_OTA          "Flash receiver OTA"
#define TR_FLASH_RECEIVER_BY_EXTERNAL_MODULE_OTA "Flash RX por ext. OTA"
#define TR_FLASH_RECEIVER_BY_INTERNAL_MODULE_OTA "Flash RX por int. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_EXTERNAL_MODULE_OTA "Flash FC by ext. OTA"
#define TR_FLASH_FLIGHT_CONTROLLER_BY_INTERNAL_MODULE_OTA "Flash FC by int. OTA"
#define TR_FLASH_BLUETOOTH_MODULE      TR("Flash modulo BT", "Flash modulo Bluetooth")
#define TR_FLASH_POWER_MANAGEMENT_UNIT "Flash pwr mngt unit"
#define TR_DEVICE_NO_RESPONSE          TR("Device not responding", "Device not responding")
#define TR_DEVICE_FILE_ERROR           TR("Device file prob.", "Device file prob.")
#define TR_DEVICE_DATA_REFUSED         TR("Device data refused", "Device data refused")
#define TR_DEVICE_WRONG_REQUEST        TR("Device access problem", "Device access problem")
#define TR_DEVICE_FILE_REJECTED        TR("Device file refused", "Device file refused")
#define TR_DEVICE_FILE_WRONG_SIG       TR("Device file sig.", "Device file sig.")
#define TR_CURRENT_VERSION             TR("Curr Vers: ", "Current version: ")
#define TR_FLASH_INTERNAL_MODULE       TR("Flash modulo int.", "Flash modulo interno")
#define TR_FLASH_INTERNAL_MULTI        TR("Flash Multi Int.", "Flash Multi Interno")
#define TR_FLASH_EXTERNAL_MODULE       TR("Flash modulo ext.", "Flash modulo externo")
#define TR_FLASH_EXTERNAL_MULTI        TR("Flash Multi Ext.", "Flash Multi Externo")
#define TR_FLASH_EXTERNAL_ELRS         TR("Flash ELRS Ext.", "Flash ELRS Externo")
#define TR_FIRMWARE_UPDATE_ERROR       TR("Erro FW update", "Erro Firmware update")
#define TR_FIRMWARE_UPDATE_SUCCESS     "Flash sucesso"
#define TR_WRITING                     "Gravando..."
#define TR_CONFIRM_FORMAT              "Confirmar Formatar?"
#define TR_INTERNALRF                  "RF Interno"
#define TR_INTERNAL_MODULE             TR("Modulo Int.", "Modulo Interno")
#define TR_EXTERNAL_MODULE             TR("Modulo Ext.", "Modulo Externo")
#define TR_EDGETX_UPGRADE_REQUIRED     "EdgeTX upgrade required"
#define TR_TELEMETRY_DISABLED          "Telem. desativada"
#define TR_MORE_OPTIONS_AVAILABLE      "More options available"
#define TR_NO_MODULE_INFORMATION       "No module information"
#define TR_EXTERNALRF                  "RF Externo"
#define TR_FAILSAFE                    TR("Failsafe", "Failsafe mode")
#define TR_FAILSAFESET                 "FAILSAFE SETTINGS"
#define TR_REG_ID                      "ID Reg."
#define TR_OWNER_ID                    "ID dono"
#define TR_HOLD                        "Hold"
#define TR_HOLD_UPPERCASE              "HOLD"
#define TR_NONE                        "None"
#define TR_NONE_UPPERCASE              "NONE"
#define TR_MENUSENSOR                  "SENSOR"
#define TR_POWERMETER_PEAK             "Peak"
#define TR_POWERMETER_POWER            "Power"
#define TR_POWERMETER_ATTN             "Attn"
#define TR_POWERMETER_FREQ             "Freq."
#define TR_MENUTOOLS                   "FERRAMENTAS"
#define TR_TURN_OFF_RECEIVER           "Turn off receiver"
#define TR_STOPPING                    "Stopping..."
#define TR_MENU_SPECTRUM_ANALYSER      "SPECTRUM ANALYSER"
#define TR_MENU_POWER_METER            "POWER METER"
#define TR_SENSOR                      "SENSOR"
#define TR_COUNTRY_CODE                "Código pais"
#define TR_USBMODE                     "Modo USB"
#define TR_JACK_MODE                   "Jack mode"
#define TR_VOICE_LANGUAGE              "Idoma da Voz"
#define TR_UNITS_SYSTEM                "Unidades"
#define TR_UNITS_PPM                   "Unidade PPM"
#define TR_EDIT                        "Editar"
#define TR_INSERT_BEFORE               "Inserir antes"
#define TR_INSERT_AFTER                "Inserir depois"
#define TR_COPY                        "Copiar"
#define TR_MOVE                        "Mover"
#define TR_PASTE                       "Colar"
#define TR_PASTE_AFTER                 "Colar antes"
#define TR_PASTE_BEFORE                "Colar depois"
#define TR_DELETE                      "Apagar"
#define TR_INSERT                      "Inserir"
#define TR_RESET_FLIGHT                "Reinic. sessão"
#define TR_RESET_TIMER1                "Reset timer1"
#define TR_RESET_TIMER2                "Reset timer2"
#define TR_RESET_TIMER3                "Reset timer3"
#define TR_RESET_TELEMETRY             "Reset telemetry"
#define TR_STATISTICS                  "Statistics"
#define TR_ABOUT_US                    "About"
#define TR_USB_JOYSTICK                "USB Joystick (HID)"
#define TR_USB_MASS_STORAGE            "USB Storage (SD)"
#define TR_USB_SERIAL                  "USB Serial (VCP)"
#define TR_SETUP_SCREENS               "Setup screens"
#define TR_MONITOR_SCREENS             "Monitors"
#define TR_AND_SWITCH                  "Chave AND"
#define TR_SF                          "SF"
#define TR_GF                          "GF"
#define TR_ANADIAGS_CALIB              "Calibrated analogs"
#define TR_ANADIAGS_FILTRAWDEV         "Filtered raw analogs with deviation"
#define TR_ANADIAGS_UNFILTRAW          "Unfiltered raw analogs"
#define TR_ANADIAGS_MINMAX             "Min., max. and range"
#define TR_ANADIAGS_MOVE               "Move analogs to their extremes!"
#define TR_SPEAKER                     "Speaker"
#define TR_BUZZER                      "Buzzer"
#define TR_BYTES                       "bytes"
#define TR_MODULE_BIND                 BUTTON(TR("Bnd", "Bind"))
#define TR_MODULE_UNBIND               BUTTON("Unbind")
#define TR_POWERMETER_ATTN_NEEDED     "Attenuator needed"
#define TR_PXX2_SELECT_RX              "Select RX"
#define TR_PXX2_DEFAULT                "<default>"
#define TR_BT_SELECT_DEVICE            "Select device"
#define TR_DISCOVER                    "Discover"
#define TR_BUTTON_INIT                 BUTTON("Init")
#define TR_WAITING                     "Waiting..."
#define TR_RECEIVER_DELETE             "Delete receiver?"
#define TR_RECEIVER_RESET              "Reset receiver?"
#define TR_SHARE                       "Share"
#define TR_BIND                        "Bind"
#define TR_REGISTER                    TR("Reg", "Register")
#define TR_MODULE_RANGE                BUTTON(TR("Rng", "Range"))
#define TR_RANGE_TEST                  "Range Test"
#define TR_RECEIVER_OPTIONS            TR("REC. OPTIONS", "RECEIVER OPTIONS")
#define TR_RESET_BTN                   BUTTON("Reset")
#define TR_DEBUG                       "Debug"
#define TR_KEYS_BTN                    BUTTON("Keys")
#define TR_ANALOGS_BTN                 BUTTON(TR("Anas", "Analogs"))
#define TR_FS_BTN                      BUTTON(TR("Custom sw", TR_FUNCTION_SWITCHES))
#define TR_TOUCH_NOTFOUND              "Touch hardware not found"
#define TR_TOUCH_EXIT                  "Touch screen to exit"
#define TR_SET                         BUTTON("Set")
#define TR_TRAINER                     "Trainer"
#define TR_CHANS                       "Chans"
#define TR_ANTENNAPROBLEM              "TX antenna problem!"
#define TR_MODELIDUSED                 "ID used in:"
#define TR_MODELIDUNIQUE               "ID is unique"
#define TR_MODULE                      "Módulo"
#define TR_RX_NAME                     "Rx Name"
#define TR_TELEMETRY_TYPE              TR("Type", "Telemetry type")
#define TR_TELEMETRY_SENSORS           "Sensors"
#define TR_VALUE                       "Value"
#define TR_PERIOD                      "Period"
#define TR_INTERVAL                    "Interval"
#define TR_REPEAT                      "Repeat"
#define TR_ENABLE                      "Enable"
#define TR_DISABLE                     "Desativar"
#define TR_TOPLCDTIMER                 "Top LCD Timer"
#define TR_UNIT                        "Unit"
#define TR_TELEMETRY_NEWSENSOR         "Adic Sensor"
#define TR_CHANNELRANGE                TR("Range Ch.", "Range Canal")
#define TR_ANTENNACONFIRM1             "EXT. ANTENNA"
#if defined(PCBX12S)
  #define TR_ANTENNA_MODES             "Internal","Ask","Per model","Internal + External"
#else
  #define TR_ANTENNA_MODES             "Internal","Ask","Per model","External"
#endif
#define TR_USE_INTERNAL_ANTENNA        TR("Use int. antenna", "Use internal antenna")
#define TR_USE_EXTERNAL_ANTENNA        TR("Use ext. antenna", "Use external antenna")
#define TR_ANTENNACONFIRM2             TR("Check antenna", "Make sure antenna is installed!")
#define TR_MODULE_PROTOCOL_FLEX_WARN_LINE1   "Requires FLEX non"
#define TR_MODULE_PROTOCOL_FCC_WARN_LINE1    "Requires FCC"
#define TR_MODULE_PROTOCOL_EU_WARN_LINE1     "Requires EU"
#define TR_MODULE_PROTOCOL_WARN_LINE2        "certified firmware"
#define TR_LOWALARM                    "Alarme baixo"
#define TR_CRITICALALARM               "Alarme critic"
#define TR_DISABLE_ALARM               TR("Desativ alarm", "Desativar alames telemetria")
#define TR_POPUP                       "Popup"
#define TR_MIN                         "Min"
#define TR_MAX                         "Max"
#define TR_CURVE_PRESET                "Preset..."
#define TR_PRESET                      "Preset"
#define TR_MIRROR                      "Mirror"
#define TR_CLEAR                       "Clear"
#define TR_RESET                       "Reset"
#define TR_RESET_SUBMENU               "Reset..."
#define TR_COUNT                       "Count"
#define TR_PT                          "pt"
#define TR_PTS                         "pts"
#define TR_SMOOTH                      "Suave"
#define TR_COPY_STICKS_TO_OFS          TR("Cp. eixos->subtrim", "Copiar eixos p/ subtrim")
#define TR_COPY_MIN_MAX_TO_OUTPUTS     TR("Cpy min/max to all",  "Copy min/max/center to all outputs")
#define TR_COPY_TRIMS_TO_OFS           TR("Cpy trim->subtrim", "Copy trims to subtrim")
#define TR_INCDEC                      "Inc/Decrement"
#define TR_GLOBALVAR                   "Global var"
#define TR_MIXSOURCE                   "Fonte (%)"
#define TR_MIXSOURCERAW                "Fonte (valor)"
#define TR_CONSTANT                    "Constant"
#define TR_PREFLIGHT_POTSLIDER_CHECK   "DESL","LIGA","AUTO"
#define TR_PREFLIGHT                   "pré-Checks"
#define TR_CHECKLIST                   TR("Checklist", "Display checklist")
#define TR_CHECKLIST_INTERACTIVE       TR3("Cklist-int", "Checklist Interat.", "Checklist interativa")
#define TR_AUX_SERIAL_MODE             "Serial port"
#define TR_AUX2_SERIAL_MODE            "Serial port 2"
#define TR_AUX_SERIAL_PORT_POWER       "Port power"
#define TR_SCRIPT                      "Script"
#define TR_INPUTS                      "Inputs"
#define TR_OUTPUTS                     "Outputs"
#define STR_EEBACKUP                   "EEPROM backup"
#define STR_FACTORYRESET               "Factory reset"
#define TR_CONFIRMRESET                TR("Erase ALL", "Erase ALL models and settings?")
#define TR_TOO_MANY_LUA_SCRIPTS        "Too many Lua scripts!"
#define TR_SPORT_UPDATE_POWER_MODE     "SP Power"
#define TR_SPORT_UPDATE_POWER_MODES    "AUTO","ON"
#define TR_NO_TELEMETRY_SCREENS        "No Telemetry Screens"
#define TR_TOUCH_PANEL                 "Touch panel:"
#define TR_FILE_SIZE                   "File size"
#define TR_FILE_OPEN                   "Open anyway?"
#define TR_TIMER_MODES                 {TR_OFFON,TR_START,TR_THROTTLE_LABEL,TR_THROTTLE_PERCENT_LABEL,TR_THROTTLE_START}

// Horus and Taranis column headers
#define TR_PHASES_HEADERS_NAME         "Nome"
#define TR_PHASES_HEADERS_SW           "Switch"
#define TR_PHASES_HEADERS_RUD_TRIM     "Rudder Trim"
#define TR_PHASES_HEADERS_ELE_TRIM     "Elevator Trim"
#define TR_PHASES_HEADERS_THT_TRIM     "Throttle Trim"
#define TR_PHASES_HEADERS_AIL_TRIM     "Aileron Trim"
#define TR_PHASES_HEADERS_CH5_TRIM     "Trim 5"
#define TR_PHASES_HEADERS_CH6_TRIM     "Trim 6"
#define TR_PHASES_HEADERS_FAD_IN       "Fade In"
#define TR_PHASES_HEADERS_FAD_OUT      "Fade Out"

#define TR_LIMITS_HEADERS_NAME         "Nome"
#define TR_LIMITS_HEADERS_SUBTRIM      "Subtrim"
#define TR_LIMITS_HEADERS_MIN          "Min"
#define TR_LIMITS_HEADERS_MAX          "Max"
#define TR_LIMITS_HEADERS_DIRECTION    "Direção"
#define TR_LIMITS_HEADERS_CURVE        "Curve"
#define TR_LIMITS_HEADERS_PPMCENTER    "Centro PPM"
#define TR_LIMITS_HEADERS_SUBTRIMMODE  "Modo subtrim"
#define TR_INVERTED                    "Invertido"


#define TR_LSW_DESCRIPTIONS            { "Comparison type or function", "First variable", "Second variable or constant", "Second variable or constant", "Additional condition for line to be enabled", "Minimum ON duration of the logical switch", "Minimum TRUE duration for the switch to become ON" }

#if defined(COLORLCD)
  // Horus layouts and widgets
  #define TR_FIRST_CHANNEL             "First channel"
  #define TR_FILL_BACKGROUND           "Fill background?"
  #define TR_BG_COLOR                  "BG Color"
  #define TR_SLIDERS_TRIMS             "Sliders+Trims"
  #define TR_SLIDERS                   "Sliders"
  #define TR_FLIGHT_MODE               "Flight mode"
  #define TR_INVALID_FILE              "Invalid File"
  #define TR_TIMER_SOURCE              "Timer source"
  #define TR_SIZE                      "Size"
  #define TR_SHADOW                    "Shadow"
  #define TR_ALIGNMENT                 "Alinhar"
  #define TR_ALIGN_LABEL               "Alinhar etiqueta"
  #define TR_ALIGN_VALUE               "Alinhar valor"
  #define TR_ALIGN_OPTS                { "Esquerda", "Centro", "Direita" }
  #define TR_TEXT                      "Text"
  #define TR_COLOR                     "Color"
  #define TR_MAIN_VIEW_X               "Main view XX"
  #define TR_PANEL1_BACKGROUND         "Panel1 background"
  #define TR_PANEL2_BACKGROUND         "Panel2 background"
  #define TR_WIDGET_GAUGE              "Gauge"
  #define TR_WIDGET_MODELBMP           "Info Modelo"
  #define TR_WIDGET_OUTPUTS            "Outputs"
  #define TR_WIDGET_TEXT               "Text"
  #define TR_WIDGET_TIMER              "Timer"
  #define TR_WIDGET_VALUE              "Value"
#endif

// Bootloader common - ASCII characters only
#define TR_BL_USB_CONNECTED           "USB Connected"
#define TR_BL_USB_PLUGIN              "Or plug in a USB cable"
#define TR_BL_USB_MASS_STORE          "for mass storage"
#define TR_BL_USB_PLUGIN_MASS_STORE   "Or plug in a USB cable for mass storage"
#define TR_BL_WRITE_FW                "Write Firmware"
#define TR_BL_FORK                    "Fork:"
#define TR_BL_VERSION                 "Version:"
#define TR_BL_RADIO                   "Radio:"
#define TR_BL_EXIT                    "Exit"
#define TR_BL_DIR_MISSING             " Directory is missing"
#define TR_BL_DIR_EMPTY               " Directory is empty"
#define TR_BL_WRITING_FW              "Gravando..."
#define TR_BL_WRITING_COMPL           "Grav. Completa"

#if LCD_W >= 480
  #define TR_BL_INVALID_FIRMWARE       "Not a valid firmware file"
#elif LCD_W >= 212
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN_MASS_STORE
  #define TR_BL_HOLD_ENTER_TO_START    "\012Hold [ENT] to start writing"
  #define TR_BL_INVALID_FIRMWARE       "\011Not a valid firmware file!        "
  #define TR_BL_INVALID_EEPROM         "\011Not a valid EEPROM file!          "
#else
  #define TR_BL_OR_PLUGIN_USB_CABLE    TR_BL_USB_PLUGIN
  #define TR_BL_HOLD_ENTER_TO_START    "\006Hold [ENT] to start"
  #define TR_BL_INVALID_FIRMWARE       "\004Not a valid firmware!        "
  #define TR_BL_INVALID_EEPROM         "\004Not a valid EEPROM!          "
#endif

#if defined(PCBTARANIS)
   // Bootloader Taranis specific - ASCII characters only
  #define TR_BL_RESTORE_EEPROM        "Restore EEPROM"
  #if defined(RADIO_COMMANDO8)
    #define TR_BL_POWER_KEY           "Press the power button."
    #define TR_BL_FLASH_EXIT          "Exit the flashing mode."
  #endif
#elif defined(PCBHORUS)
   // Bootloader Horus specific - ASCII characters only
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_SELECT_KEY            "[ENT] to select file"
  #define TR_BL_FLASH_KEY             "Hold [ENT] long to flash"
  #define TR_BL_ERASE_KEY             "Hold [ENT] long to erase"
  #define TR_BL_EXIT_KEY              "[RTN] to exit"
#elif defined(PCBNV14)
   // Bootloader NV14 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_CURRENT_FW            "Current Firmware:"
  #define TR_BL_SELECT_KEY            "[R TRIM] to select file"
  #define TR_BL_FLASH_KEY             "Hold [R TRIM] long to flash"
  #define TR_BL_EXIT_KEY              " [L TRIM] to exit"
  #define TR_BL_ENABLE                "Enable"
  #define TR_BL_DISABLE               "Desativar"
#elif defined(PCBPL18)
   // Bootloader PL18 specific - ASCII characters only
  #define TR_BL_RF_USB_ACCESS         "RF USB access"
  #define TR_BL_ERASE_INT_FLASH       "Erase Internal Flash Storage"
  #define TR_BL_ERASE_FLASH           "Erase Flash Storage"
  #define TR_BL_ERASE_FLASH_MSG       "This may take up to 200s"
  #define TR_BL_SELECT_KEY            " [TR4 Dn] to select file"
  #define TR_BL_FLASH_KEY             " Hold [TR4 Dn] long to flash"
  #define TR_BL_ERASE_KEY             " Hold [TR4 Dn] long to erase"
  #define TR_BL_EXIT_KEY              " [TR4 Up] to exit"
#endif

// About screen
#define TR_ABOUTUS                     TR(" ABOUT ", "ABOUT")

#define TR_CHR_HOUR                    'h'
#define TR_CHR_INPUT                   'I'   // Values between A-I will work

#define TR_BEEP_VOLUME                 "Volume Bipe"
#define TR_WAV_VOLUME                  "Volume Wav"
#define TR_BG_VOLUME                   TR("Bg volume", "Volume fundo")

#define TR_TOP_BAR                     "Top bar"
#define TR_FLASH_ERASE                 "Flash erase..."
#define TR_FLASH_WRITE                 "Flash write..."
#define TR_OTA_UPDATE                  "OTA update..."
#define TR_MODULE_RESET                "Module reset..."
#define TR_UNKNOWN_RX                  "Unknown RX"
#define TR_UNSUPPORTED_RX              "Unsupported RX"
#define TR_OTA_UPDATE_ERROR            "OTA update error"
#define TR_DEVICE_RESET                "Device reset..."
#define TR_ALTITUDE                    "Altitude"
#define TR_SCALE                       "Scale"
#define TR_VIEW_CHANNELS               "View Channels"
#define TR_VIEW_NOTES                  "View Notes"
#define TR_MODEL_SELECT                "Model Select"
#define TR_ID                          "ID"
#define TR_PRECISION                   "Precision"
#define TR_RATIO                       "Ratio"
#define TR_FORMULA                     "Formula"
#define TR_CELLINDEX                   "Cell index"
#define TR_LOGS                        "Logs"
#define TR_OPTIONS                     "Options"
#define TR_FIRMWARE_OPTIONS            "Firmware options"

#define TR_ALTSENSOR                   "Alt sensor"
#define TR_CELLSENSOR                  "Cell sensor"
#define TR_GPSSENSOR                   "GPS sensor"
#define TR_CURRENTSENSOR               "Sensor"
#define TR_AUTOOFFSET                  "Auto Offset"
#define TR_ONLYPOSITIVE                "Positive"
#define TR_FILTER                      "Filter"
#define TR_TELEMETRYFULL               TR("All slots full!", "All telemetry slots full!")
#define TR_IGNORE_INSTANCE             TR("No inst.", "Ignore instances")
#define TR_SHOW_INSTANCE_ID            "Show instance ID"
#define TR_DISCOVER_SENSORS            "Detect. novos"
#define TR_STOP_DISCOVER_SENSORS       "Stop"
#define TR_DELETE_ALL_SENSORS          "Apagar todos"
#define TR_CONFIRMDELETE               "Really " LCDW_128_LINEBREAK "delete all ?"
#define TR_SELECT_WIDGET               "Select widget"
#define TR_WIDGET_FULLSCREEN           "Full screen"
#define TR_REMOVE_WIDGET               "Remove widget"
#define TR_WIDGET_SETTINGS             "Widget settings"
#define TR_REMOVE_SCREEN               "Remove screen"
#define TR_SETUP_WIDGETS               "Setup widgets"
#define TR_USER_INTERFACE              "User interface"
#define TR_THEME                       "Tema"
#define TR_SETUP                       "Setup"
#define TR_LAYOUT                      "Layout"
#define TR_ADD_MAIN_VIEW               "Add main view"
#define TR_TEXT_COLOR                  "Text color"
#define TR_MENU_INPUTS                 STR_CHAR_INPUT "Inputs"
#define TR_MENU_LUA                    STR_CHAR_LUA "Lua scripts"
#define TR_MENU_STICKS                 STR_CHAR_STICK "Eixos"
#define TR_MENU_POTS                   STR_CHAR_POT "Pots"
#define TR_MENU_MIN                    STR_CHAR_FUNCTION "MIN"
#define TR_MENU_MAX                    STR_CHAR_FUNCTION "MAX"
#define TR_MENU_HELI                   STR_CHAR_CYC "Cyclic"
#define TR_MENU_TRIMS                  STR_CHAR_TRIM "Trims"
#define TR_MENU_SWITCHES               STR_CHAR_SWITCH "Switches"
#define TR_MENU_LOGICAL_SWITCHES       STR_CHAR_SWITCH "Logical switches"
#define TR_MENU_TRAINER                STR_CHAR_TRAINER "Trainer"
#define TR_MENU_CHANNELS               STR_CHAR_CHANNEL "Channels"
#define TR_MENU_GVARS                  STR_CHAR_SLIDER "GVars"
#define TR_MENU_TELEMETRY              STR_CHAR_TELEMETRY "Telemetry"
#define TR_MENU_DISPLAY                "DISPLAY"
#define TR_MENU_OTHER                  "Other"
#define TR_MENU_INVERT                 "Invert"
#define TR_AUDIO_MUTE                  TR("Audio mute","Mute if no sound")
#define TR_JITTER_FILTER               "Filtro ADC"
#define TR_DEAD_ZONE                   "Dead zone"
#define TR_RTC_CHECK                   TR("Check RTC", "Check RTC voltage")
#define TR_AUTH_FAILURE                "Auth-failure"
#define TR_RACING_MODE                 "Racing mode"

#define TR_USE_THEME_COLOR              "Use theme color"

#define TR_ADD_ALL_TRIMS_TO_SUBTRIMS    "Add all Trims to Subtrims"
#if !PORTRAIT_LCD
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
#define TR_THEME_COLOR_DEFAULT         "DEFAULT"
#define TR_THEME_COLOR_PRIMARY1        "PRIMARY1"
#define TR_THEME_COLOR_PRIMARY2        "PRIMARY2"
#define TR_THEME_COLOR_PRIMARY3        "PRIMARY3"
#define TR_THEME_COLOR_SECONDARY1      "SECONDARY1"
#define TR_THEME_COLOR_SECONDARY2      "SECONDARY2"
#define TR_THEME_COLOR_SECONDARY3      "SECONDARY3"
#define TR_THEME_COLOR_FOCUS           "FOCUS"
#define TR_THEME_COLOR_EDIT            "EDITAR"
#define TR_THEME_COLOR_ACTIVE          "ACTIVE"
#define TR_THEME_COLOR_WARNING         "WARNING"
#define TR_THEME_COLOR_DISABLED        "DISABLED"
#define TR_THEME_COLOR_CUSTOM          "CUSTOM"
#define TR_THEME_CHECKBOX              "Checkbox"
#define TR_THEME_ACTIVE                "Active"
#define TR_THEME_REGULAR               "Regular"
#define TR_THEME_WARNING               "Warning"
#define TR_THEME_DISABLED              "Disabled"
#define TR_THEME_EDIT                  "Editar"
#define TR_THEME_FOCUS                 "Focus"
#define TR_AUTHOR                       "Author"
#define TR_DESCRIPTION                  "Description"
#define TR_SAVE                         "Save"
#define TR_CANCEL                       "Cancel"
#define TR_EDIT_THEME                   "EDIT THEME"
#define TR_DETAILS                      "Details"
#define TR_THEME_EDITOR                 "TEMAS"

// Main menu
#define TR_MAIN_MENU_SELECT_MODEL       "Selecionar\nModelo"
#define TR_MAIN_MENU_MANAGE_MODELS      "Gerenciar\nModelos"
#define TR_MAIN_MENU_MODEL_NOTES        "Notas\nModelo"
#define TR_MAIN_MENU_CHANNEL_MONITOR    "Monitor\nCanais"
#define TR_MAIN_MENU_MODEL_SETTINGS     "Config.\nModelo"
#define TR_MAIN_MENU_RADIO_SETTINGS     "Config.\nRádio"
#define TR_MAIN_MENU_SCREEN_SETTINGS    "Config.\nTela"
#define TR_MAIN_MENU_RESET_TELEMETRY    "Reset\nTelem."
#define TR_MAIN_MENU_STATISTICS         "Estatist."
#define TR_MAIN_MENU_ABOUT_EDGETX       "Sobre\nEdgeTX"
// End Main menu

// Voice in native language
#define TR_VOICE_ENGLISH                "Inglês"
#define TR_VOICE_CHINESE                "Chinês"
#define TR_VOICE_CZECH                  "Tcheco"
#define TR_VOICE_DANISH                 "Dinamarquês"
#define TR_VOICE_DEUTSCH                "Alemão"
#define TR_VOICE_DUTCH                  "Holandês"
#define TR_VOICE_ESPANOL                "Espanhol"
#define TR_VOICE_FRANCAIS               "Francês"
#define TR_VOICE_HUNGARIAN              "Húngaro"
#define TR_VOICE_ITALIANO               "Italiano"
#define TR_VOICE_POLISH                 "Polonês"
#define TR_VOICE_PORTUGUES              "Português"
#define TR_VOICE_RUSSIAN                "Russo"
#define TR_VOICE_SLOVAK                 "Eslovaco"
#define TR_VOICE_SWEDISH                "Sueco"
#define TR_VOICE_TAIWANESE              "Taiwanês"
#define TR_VOICE_JAPANESE               "Japonês"
#define TR_VOICE_HEBREW                 "Hebraico"
#define TR_VOICE_UKRAINIAN              "Ucraniano"

#define TR_USBJOYSTICK_LABEL           "Joystick USB"
#define TR_USBJOYSTICK_EXTMODE         "Modo"
#define TR_VUSBJOYSTICK_EXTMODE        "Clássico","Avançado"
#define TR_USBJOYSTICK_SETTINGS        "Conf. de Canais"
#define TR_USBJOYSTICK_IF_MODE         TR("Modo If.","Modo Interface")
#define TR_VUSBJOYSTICK_IF_MODE        "Joystick","Gamepad","MultiAxis"
#define TR_USBJOYSTICK_CH_MODE         "Mode"
#define TR_VUSBJOYSTICK_CH_MODE        "Nenhum","Btn","Eixo","Sim"
#define TR_VUSBJOYSTICK_CH_MODE_S      "-","B","A","S"
#define TR_USBJOYSTICK_CH_BTNMODE      "Modo Botão"
#define TR_VUSBJOYSTICK_CH_BTNMODE     "Normal","Pulso","SWEmu","Delta","Companion"
#define TR_VUSBJOYSTICK_CH_BTNMODE_S   TR("Norm","Normal"),TR("Puls","Pulse"),TR("SWEm","SWEmul"),TR("Delt","Delta"),TR("CPN","Companion")
#define TR_USBJOYSTICK_CH_SWPOS        "Posição"
#define TR_VUSBJOYSTICK_CH_SWPOS       "Push","2POS","3POS","4POS","5POS","6POS","7POS","8POS"
#define TR_USBJOYSTICK_CH_AXIS         "Eixo"
#define TR_VUSBJOYSTICK_CH_AXIS        "X","Y","Z","rotX","rotY","rotZ","Slider","Dial","Wheel"
#define TR_USBJOYSTICK_CH_SIM          "Sim axis"
#define TR_VUSBJOYSTICK_CH_SIM         "Ail","Pfd","Lem","Mot","Acc","Brk","Steer","Dpad"
#define TR_USBJOYSTICK_CH_INVERSION    "Inversion"
#define TR_USBJOYSTICK_CH_BTNNUM       "Button no."
#define TR_USBJOYSTICK_BTN_COLLISION   "!Button no. collision!"
#define TR_USBJOYSTICK_AXIS_COLLISION  "!Axis collision!"
#define TR_USBJOYSTICK_CIRC_COUTOUT    TR("Circ. cut", "Circular cutout")
#define TR_VUSBJOYSTICK_CIRC_COUTOUT   "Nenh","X-Y, Z-rX","X-Y, rX-rY","X-Y, Z-rZ"
#define TR_USBJOYSTICK_APPLY_CHANGES   "Appl mudanças"

#define TR_DIGITAL_SERVO          "Servo333HZ"
#define TR_ANALOG_SERVO           "Servo 50HZ"
#define TR_SIGNAL_OUTPUT          "Signal output"
#define TR_SERIAL_BUS             "Serial bus"
#define TR_SYNC                   "Sync"

#define TR_ENABLED_FEATURES       "Recursos ativos"
#define TR_RADIO_MENU_TABS        "Abas opções rádio"
#define TR_MODEL_MENU_TABS        "Abas opções modelo"

#define TR_SELECT_MENU_ALL        "Todas"
#define TR_SELECT_MENU_CLR        "Limpar"
#define TR_SELECT_MENU_INV        "Inverte"

#define TR_SORT_ORDERS            "Nome A-Z","Nome Z-A","Menos usado","Mais usado"
#define TR_SORT_MODELS_BY         "Ordenar modelos por"
#define TR_CREATE_NEW             "Criar"

#define TR_MIX_SLOW_PREC          TR("Prec. Lento", "Prec. mais/menos lento")
#define TR_MIX_DELAY_PREC         TR("Delay prec", "Delay up/dn prec")

#define TR_THEME_EXISTS           "Já existe um diretório de tema com o mesmo nome."

#define TR_DATE_TIME_WIDGET       "Data & Hora"
#define TR_RADIO_INFO_WIDGET      "Inf do Rádio"
#define TR_LOW_BATT_COLOR         "Bateria Baixa"
#define TR_MID_BATT_COLOR         "Bateria Média"
#define TR_HIGH_BATT_COLOR        "Bateria Alta"

#define TR_WIDGET_SIZE            "Tamanho do widget"

#define TR_DEL_DIR_NOT_EMPTY      "Directory must be empty before deletion"
