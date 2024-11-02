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
 */

// This file is auto-generated via cfn_sorter.sh. Do not edit.

#include "dataconstants.h"

Functions cfn_sorted[] = {
#if defined(TRANSLATIONS_CN)
#if !defined(OLED_SCREEN)
  /* 背光 */ FUNC_BACKLIGHT,
#endif
  /* 播放背景音乐 */ FUNC_BACKGND_MUSIC,
  /* 播放声音 */ FUNC_PLAY_SOUND,
  /* 播放数值 */ FUNC_PLAY_VALUE,
  /* 播放音频文件 */ FUNC_PLAY_TRACK,
  /* 测距模式 */ FUNC_RANGECHECK,
#if defined(DEBUG)
  /* 测试 */ FUNC_TEST,
#endif
  /* 复位 */ FUNC_RESET,
  /* 关闭音频功放 */ FUNC_DISABLE_AUDIO_AMP,
  /* 记录日志到SD卡 */ FUNC_LOGS,
  /* 教练 */ FUNC_TRAINER,
  /* 截屏 */ FUNC_SCREENSHOT,
#if defined(COLORLCD)
  /* 禁用触摸 */ FUNC_DISABLE_TOUCH,
#endif
  /* 竞速模式 */ FUNC_RACING_MODE,
#if defined(OLED_SCREEN)
  /* 亮度 */ FUNC_BACKLIGHT,
#endif
  /* 模块对频 */ FUNC_BIND,
#if defined(VIDEO_SWITCH)
  /* 屏幕显示图传 */ FUNC_LCD_TO_VIDEO,
#endif
  /* 设置 */ FUNC_SET_TIMER,
  /* 设置失控保护 */ FUNC_SET_FAILSAFE,
#if !defined(COLORLCD)
  /* 设置主屏 */ FUNC_SET_SCREEN,
#endif
  /* 锁定通道值 */ FUNC_OVERRIDE_CHANNEL,
  /* 修改全局变量GV值 */ FUNC_ADJUST_GVAR,
#if defined(COLORLCD)
  /* 选择主屏 */ FUNC_SET_SCREEN,
#endif
  /* 摇杆值存储到微调 */ FUNC_INSTANT_TRIM,
  /* 音量 */ FUNC_VOLUME,
  /* 暂停背景音乐 */ FUNC_BACKGND_MUSIC_PAUSE,
  /* 振动 */ FUNC_HAPTIC,
  /* Lua脚本 */ FUNC_PLAY_SCRIPT,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RGB LED 灯 */ FUNC_RGB_LED,
  /* Vario传感器 */ FUNC_VARIO,
#elif defined(TRANSLATIONS_CZ)
#if defined(COLORLCD)
  /* Deaktivace dotyku */ FUNC_DISABLE_TOUCH,
#endif
  /* Hlasitost */ FUNC_VOLUME,
  /* Hlásit stav */ FUNC_PLAY_VALUE,
  /* Hrát zvuk */ FUNC_PLAY_SOUND,
  /* Hudba */ FUNC_BACKGND_MUSIC,
  /* Hudba pauza */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Insta-Trim */ FUNC_INSTANT_TRIM,
#if defined(OLED_SCREEN)
  /* Jas */ FUNC_BACKLIGHT,
#endif
  /* Kontrola dosahu */ FUNC_RANGECHECK,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Loguj na SD */ FUNC_LOGS,
  /* Lua Skript */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
  /* Nastav */ FUNC_ADJUST_GVAR,
  /* Nastavit Failsafe */ FUNC_SET_FAILSAFE,
#if !defined(COLORLCD)
  /* Nast obrazovku */ FUNC_SET_SCREEN,
#endif
#if !defined(OLED_SCREEN)
  /* Podsvětlení */ FUNC_BACKLIGHT,
#endif
  /* Přehrát wav */ FUNC_PLAY_TRACK,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* Reset */ FUNC_RESET,
  /* RGB světlo */ FUNC_RGB_LED,
  /* Snímek LCD */ FUNC_SCREENSHOT,
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Trenér */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Vibrovat */ FUNC_HAPTIC,
#if defined(COLORLCD)
  /* Vybrat hlavní obrazovku */ FUNC_SET_SCREEN,
#endif
  /* Vypnutí zesilovače zvuku */ FUNC_DISABLE_AUDIO_AMP,
  /* Zámek */ FUNC_OVERRIDE_CHANNEL,
  /* Závodní režim */ FUNC_RACING_MODE,
  /* Změna */ FUNC_SET_TIMER,
#elif defined(TRANSLATIONS_DA)
  /* Afspil lydfil */ FUNC_PLAY_TRACK,
  /* Afstand kontrol */ FUNC_RANGECHECK,
#if !defined(OLED_SCREEN)
  /* Baggrundslys */ FUNC_BACKLIGHT,
#endif
  /* BgMusik */ FUNC_BACKGND_MUSIC,
  /* BgMusik || */ FUNC_BACKGND_MUSIC_PAUSE,
#if defined(COLORLCD)
  /* Ikke berøringsaktiv */ FUNC_DISABLE_TOUCH,
#endif
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
  /* Juster */ FUNC_ADJUST_GVAR,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* Lydstyrke */ FUNC_VOLUME,
  /* Modul tilslut */ FUNC_BIND,
  /* Nulstil */ FUNC_RESET,
  /* Overskriv */ FUNC_OVERRIDE_CHANNEL,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RGB led */ FUNC_RGB_LED,
  /* Ræs tilstand */ FUNC_RACING_MODE,
  /* SD Log */ FUNC_LOGS,
#if defined(OLED_SCREEN)
  /* Skarphed */ FUNC_BACKLIGHT,
#endif
  /* Skærm klip */ FUNC_SCREENSHOT,
  /* Sluk audio amplifier */ FUNC_DISABLE_AUDIO_AMP,
  /* Spil lyd */ FUNC_PLAY_SOUND,
  /* Spil Værdi */ FUNC_PLAY_VALUE,
  /* Sæt */ FUNC_SET_TIMER,
  /* Sæt fejlsikring */ FUNC_SET_FAILSAFE,
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Træner */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Vibration */ FUNC_HAPTIC,
#if !defined(COLORLCD)
  /* Vis skærm */ FUNC_SET_SCREEN,
#endif
#if defined(COLORLCD)
  /* Vælg hoved skærm */ FUNC_SET_SCREEN,
#endif
#elif defined(TRANSLATIONS_DE)
  /* Ändere */ FUNC_ADJUST_GVAR,
  /* Audio Verst. Aus */ FUNC_DISABLE_AUDIO_AMP,
  /* Haptik */ FUNC_HAPTIC,
#if defined(OLED_SCREEN)
  /* Helligkeit */ FUNC_BACKLIGHT,
#endif
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
#if defined(COLORLCD)
  /* Kein Touch */ FUNC_DISABLE_TOUCH,
#endif
  /* Lautstr. */ FUNC_VOLUME,
#if !defined(OLED_SCREEN)
  /* LCD Licht */ FUNC_BACKLIGHT,
#endif
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lehrer */ FUNC_TRAINER,
  /* Lua Skript */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* RGB LED */ FUNC_RGB_LED,
  /* Rücksetz. */ FUNC_RESET,
  /* Sag Text */ FUNC_PLAY_TRACK,
  /* Sag Wert */ FUNC_PLAY_VALUE,
  /* Screenshot */ FUNC_SCREENSHOT,
  /* SD-Aufz. */ FUNC_LOGS,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
#if defined(COLORLCD)
  /* Set Main Screen */ FUNC_SET_SCREEN,
#endif
  /* Setze */ FUNC_SET_TIMER,
  /* Spiel Töne */ FUNC_PLAY_SOUND,
  /* StartMusik */ FUNC_BACKGND_MUSIC,
  /* Stop Musik */ FUNC_BACKGND_MUSIC_PAUSE,
#if !defined(COLORLCD)
  /* TelSeite anz */ FUNC_SET_SCREEN,
#endif
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Überschreibe */ FUNC_OVERRIDE_CHANNEL,
  /* Vario */ FUNC_VARIO,
#elif defined(TRANSLATIONS_ES)
#if !defined(COLORLCD)
  /* Ajus. pantalla */ FUNC_SET_SCREEN,
#endif
  /* Ajuste */ FUNC_SET_TIMER,
  /* Ajuste */ FUNC_ADJUST_GVAR,
  /* Audio Amp Off */ FUNC_DISABLE_AUDIO_AMP,
  /* BgMúsica */ FUNC_BACKGND_MUSIC,
  /* BgMúsica|| */ FUNC_BACKGND_MUSIC_PAUSE,
#if defined(OLED_SCREEN)
  /* Brillo */ FUNC_BACKLIGHT,
#endif
  /* Captura */ FUNC_SCREENSHOT,
  /* CheckRango */ FUNC_RANGECHECK,
  /* Enl.módulo */ FUNC_BIND,
  /* Entrenador */ FUNC_TRAINER,
  /* Failsafe */ FUNC_SET_FAILSAFE,
  /* Haptic */ FUNC_HAPTIC,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lua Script */ FUNC_PLAY_SCRIPT,
#if !defined(OLED_SCREEN)
  /* Luz Fondo */ FUNC_BACKLIGHT,
#endif
#if defined(COLORLCD)
  /* No Touch */ FUNC_DISABLE_TOUCH,
#endif
  /* Oir pista */ FUNC_PLAY_TRACK,
  /* Oir sonido */ FUNC_PLAY_SOUND,
  /* Oir valor */ FUNC_PLAY_VALUE,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RacingMode */ FUNC_RACING_MODE,
  /* Reset */ FUNC_RESET,
  /* RGB leds */ FUNC_RGB_LED,
  /* SD Logs */ FUNC_LOGS,
  /* Seguro */ FUNC_OVERRIDE_CHANNEL,
#if defined(COLORLCD)
  /* Set Main Screen */ FUNC_SET_SCREEN,
#endif
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Vario */ FUNC_VARIO,
  /* Volumen */ FUNC_VOLUME,
#elif defined(TRANSLATIONS_FI)
  /* Adjust */ FUNC_ADJUST_GVAR,
  /* Audio Amp Off */ FUNC_DISABLE_AUDIO_AMP,
#if !defined(OLED_SCREEN)
  /* Backlight */ FUNC_BACKLIGHT,
#endif
  /* BgMusic */ FUNC_BACKGND_MUSIC,
  /* BgMusic || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Haptic */ FUNC_HAPTIC,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
#if defined(OLED_SCREEN)
  /* Kirkkaus */ FUNC_BACKLIGHT,
#endif
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
#if defined(COLORLCD)
  /* No Touch */ FUNC_DISABLE_TOUCH,
#endif
  /* Play Sound */ FUNC_PLAY_SOUND,
  /* Play Track */ FUNC_PLAY_TRACK,
  /* Play Value */ FUNC_PLAY_VALUE,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Reset */ FUNC_RESET,
  /* RGB leds */ FUNC_RGB_LED,
  /* Safety */ FUNC_OVERRIDE_CHANNEL,
  /* Screenshot */ FUNC_SCREENSHOT,
  /* SD Logs */ FUNC_LOGS,
  /* Set */ FUNC_SET_TIMER,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
#if defined(COLORLCD)
  /* Set Main Screen */ FUNC_SET_SCREEN,
#endif
#if !defined(COLORLCD)
  /* Set Screen */ FUNC_SET_SCREEN,
#endif
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Trainer */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Volume */ FUNC_VOLUME,
#elif defined(TRANSLATIONS_FR)
#if !defined(COLORLCD)
  /* Aff. écran */ FUNC_SET_SCREEN,
#endif
  /* Ajuster */ FUNC_ADJUST_GVAR,
  /* Bind */ FUNC_BIND,
  /* Déf. */ FUNC_SET_TIMER,
#if defined(COLORLCD)
  /* Définir Écran Princ. */ FUNC_SET_SCREEN,
#endif
  /* Désact. Ampli Audio */ FUNC_DISABLE_AUDIO_AMP,
  /* Écolage */ FUNC_TRAINER,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
  /* Jouer fichier */ FUNC_PLAY_TRACK,
  /* Jouer son */ FUNC_PLAY_SOUND,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lire valeur */ FUNC_PLAY_VALUE,
  /* Logs SD */ FUNC_LOGS,
#if defined(OLED_SCREEN)
  /* Luminosité */ FUNC_BACKLIGHT,
#endif
  /* Musique */ FUNC_BACKGND_MUSIC,
#if defined(COLORLCD)
  /* Non Tactile */ FUNC_DISABLE_TOUCH,
#endif
  /* Pause Musique */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Photo Écran */ FUNC_SCREENSHOT,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* Racing Mode */ FUNC_RACING_MODE,
  /* Régle Failsafe */ FUNC_SET_FAILSAFE,
  /* Remise à 0 */ FUNC_RESET,
  /* Remplace */ FUNC_OVERRIDE_CHANNEL,
#if !defined(OLED_SCREEN)
  /* Rétroécl. */ FUNC_BACKLIGHT,
#endif
  /* RGB leds */ FUNC_RGB_LED,
  /* Script Lua */ FUNC_PLAY_SCRIPT,
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Test Port. */ FUNC_RANGECHECK,
  /* Vario */ FUNC_VARIO,
  /* Vibreur */ FUNC_HAPTIC,
  /* Volume */ FUNC_VOLUME,
#elif defined(TRANSLATIONS_HE)
  /* Adjust */ FUNC_ADJUST_GVAR,
  /* Audio Amp Off */ FUNC_DISABLE_AUDIO_AMP,
#if !defined(OLED_SCREEN)
  /* Backlight */ FUNC_BACKLIGHT,
#endif
  /* BgMusic */ FUNC_BACKGND_MUSIC,
  /* BgMusic || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Haptic */ FUNC_HAPTIC,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
  /* Override */ FUNC_OVERRIDE_CHANNEL,
  /* Play Sound */ FUNC_PLAY_SOUND,
  /* Play Track */ FUNC_PLAY_TRACK,
  /* Play Value */ FUNC_PLAY_VALUE,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Reset */ FUNC_RESET,
  /* RGB leds */ FUNC_RGB_LED,
  /* SD Logs */ FUNC_LOGS,
  /* Set */ FUNC_SET_TIMER,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
#if !defined(COLORLCD)
  /* Set Screen */ FUNC_SET_SCREEN,
#endif
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Trainer */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Volume */ FUNC_VOLUME,
#if defined(OLED_SCREEN)
  /* בהירות */ FUNC_BACKLIGHT,
#endif
#if defined(COLORLCD)
  /* הגדרת מסך ראשי */ FUNC_SET_SCREEN,
#endif
#if defined(COLORLCD)
  /* ללא מסך מגע */ FUNC_DISABLE_TOUCH,
#endif
  /* מצב תחרות */ FUNC_RACING_MODE,
  /* צילום מסך */ FUNC_SCREENSHOT,
#elif defined(TRANSLATIONS_IT)
  /* Amp Audio Off */ FUNC_DISABLE_AUDIO_AMP,
  /* Azzera */ FUNC_RESET,
  /* BindModulo */ FUNC_BIND,
  /* Blocco */ FUNC_OVERRIDE_CHANNEL,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Leds RGB */ FUNC_RGB_LED,
  /* Leggi Valore */ FUNC_PLAY_VALUE,
  /* Logs SDCard */ FUNC_LOGS,
#if defined(OLED_SCREEN)
  /* Luminosità */ FUNC_BACKLIGHT,
#endif
  /* Maestro */ FUNC_TRAINER,
  /* Modo Racing */ FUNC_RACING_MODE,
  /* Musica Sf */ FUNC_BACKGND_MUSIC,
  /* Musica Sf || */ FUNC_BACKGND_MUSIC_PAUSE,
#if defined(COLORLCD)
  /* No Touch */ FUNC_DISABLE_TOUCH,
#endif
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Regola */ FUNC_ADJUST_GVAR,
#if !defined(OLED_SCREEN)
  /* Retroillum. */ FUNC_BACKLIGHT,
#endif
  /* Screenshot */ FUNC_SCREENSHOT,
  /* Script Lua */ FUNC_PLAY_SCRIPT,
  /* Set */ FUNC_SET_TIMER,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
#if !defined(COLORLCD)
  /* Setta Schermo */ FUNC_SET_SCREEN,
#endif
#if defined(COLORLCD)
  /* Setta Schermo Princ. */ FUNC_SET_SCREEN,
#endif
  /* Suona */ FUNC_PLAY_SOUND,
  /* Suona Traccia */ FUNC_PLAY_TRACK,
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Vario */ FUNC_VARIO,
  /* Vibrazione */ FUNC_HAPTIC,
  /* Volume */ FUNC_VOLUME,
#elif defined(TRANSLATIONS_JP)
  /* BGM一時停止 */ FUNC_BACKGND_MUSIC_PAUSE,
  /* BGM再生 */ FUNC_BACKGND_MUSIC,
  /* Failsafe設定 */ FUNC_SET_FAILSAFE,
  /* G変数修正 */ FUNC_ADJUST_GVAR,
  /* Inst.トリム */ FUNC_INSTANT_TRIM,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* LUAスクリプト */ FUNC_PLAY_SCRIPT,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RGB LED */ FUNC_RGB_LED,
  /* SDログ出力 */ FUNC_LOGS,
  /* オーディオアンプ OFF */ FUNC_DISABLE_AUDIO_AMP,
  /* サウンド再生 */ FUNC_PLAY_SOUND,
  /* ダイヤル */ FUNC_VOLUME,
#if defined(DEBUG)
  /* テスト */ FUNC_TEST,
#endif
  /* トレーナー */ FUNC_TRAINER,
  /* バイブレート */ FUNC_HAPTIC,
#if !defined(OLED_SCREEN)
  /* バックライト */ FUNC_BACKLIGHT,
#endif
  /* バリオ */ FUNC_VARIO,
#if defined(COLORLCD)
  /* メイン画面設定 */ FUNC_SET_SCREEN,
#endif
  /* モジュールバインド */ FUNC_BIND,
  /* リセット */ FUNC_RESET,
  /* レースモード */ FUNC_RACING_MODE,
  /* レンジチェック */ FUNC_RANGECHECK,
  /* 音源再生 */ FUNC_PLAY_TRACK,
  /* 画面キャプチャ */ FUNC_SCREENSHOT,
#if !defined(COLORLCD)
  /* 画面設定 */ FUNC_SET_SCREEN,
#endif
#if defined(OLED_SCREEN)
  /* 輝度 */ FUNC_BACKLIGHT,
#endif
  /* 再生値 */ FUNC_PLAY_VALUE,
  /* 上書き */ FUNC_OVERRIDE_CHANNEL,
  /* 設定 */ FUNC_SET_TIMER,
#if defined(COLORLCD)
  /* 非タッチ */ FUNC_DISABLE_TOUCH,
#endif
#elif defined(TRANSLATIONS_NL)
  /* Audio Amp Off */ FUNC_DISABLE_AUDIO_AMP,
#if !defined(OLED_SCREEN)
  /* Backlight */ FUNC_BACKLIGHT,
#endif
  /* BgMusic */ FUNC_BACKGND_MUSIC,
  /* BgMusic || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Geluid */ FUNC_PLAY_SOUND,
  /* Haptic */ FUNC_HAPTIC,
#if defined(OLED_SCREEN)
  /* Helderheid */ FUNC_BACKLIGHT,
#endif
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
#if defined(COLORLCD)
  /* No Touch */ FUNC_DISABLE_TOUCH,
#endif
  /* Override */ FUNC_OVERRIDE_CHANNEL,
  /* Play Track */ FUNC_PLAY_TRACK,
  /* Play Value */ FUNC_PLAY_VALUE,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Reset */ FUNC_RESET,
  /* RGB leds */ FUNC_RGB_LED,
  /* Schermafdr */ FUNC_SCREENSHOT,
  /* SD Logs */ FUNC_LOGS,
  /* Set */ FUNC_SET_TIMER,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
#if defined(COLORLCD)
  /* Set Main Screen */ FUNC_SET_SCREEN,
#endif
#if !defined(COLORLCD)
  /* Set Screen */ FUNC_SET_SCREEN,
#endif
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Trainer */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Volume */ FUNC_VOLUME,
  /* Wijzig */ FUNC_ADJUST_GVAR,
#elif defined(TRANSLATIONS_PL)
  /* Bezpiecz */ FUNC_OVERRIDE_CHANNEL,
  /* Głośność */ FUNC_VOLUME,
  /* GrajDźwięk */ FUNC_PLAY_SOUND,
  /* GrajŚcieżk */ FUNC_PLAY_TRACK,
  /* GrajWartoś */ FUNC_PLAY_VALUE,
  /* Inst-Trim */ FUNC_INSTANT_TRIM,
#if defined(OLED_SCREEN)
  /* Jasność */ FUNC_BACKLIGHT,
#endif
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Logi->SD */ FUNC_LOGS,
  /* ModuleBind */ FUNC_BIND,
  /* Muz. tła */ FUNC_BACKGND_MUSIC,
  /* Muz. tła || */ FUNC_BACKGND_MUSIC_PAUSE,
#if defined(COLORLCD)
  /* No Touch */ FUNC_DISABLE_TOUCH,
#endif
#if !defined(OLED_SCREEN)
  /* Podświetl */ FUNC_BACKLIGHT,
#endif
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Resetuj */ FUNC_RESET,
  /* RGB ledy */ FUNC_RGB_LED,
#if defined(COLORLCD)
  /* Set Main Screen */ FUNC_SET_SCREEN,
#endif
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
  /* SkryptyLua */ FUNC_PLAY_SCRIPT,
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Trener */ FUNC_TRAINER,
  /* Ustaw */ FUNC_ADJUST_GVAR,
  /* Ustaw */ FUNC_SET_TIMER,
#if !defined(COLORLCD)
  /* Ustaw ekran */ FUNC_SET_SCREEN,
#endif
  /* Wario */ FUNC_VARIO,
  /* Wibracje */ FUNC_HAPTIC,
  /* Wycisz wzmacniacz audio */ FUNC_DISABLE_AUDIO_AMP,
  /* Zrzut Ekra */ FUNC_SCREENSHOT,
#elif defined(TRANSLATIONS_PT)
  /* Ajuste */ FUNC_ADJUST_GVAR,
  /* Amp Audio Desl */ FUNC_DISABLE_AUDIO_AMP,
  /* BgMusica */ FUNC_BACKGND_MUSIC,
  /* BgMusica || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* BindMódulo */ FUNC_BIND,
#if defined(OLED_SCREEN)
  /* Brilho */ FUNC_BACKLIGHT,
#endif
  /* Capt. Tela */ FUNC_SCREENSHOT,
  /* DefFailsafe */ FUNC_SET_FAILSAFE,
  /* Definir */ FUNC_SET_TIMER,
#if !defined(COLORLCD)
  /* Def Tela */ FUNC_SET_SCREEN,
#endif
#if defined(COLORLCD)
  /* Def Tela Princ */ FUNC_SET_SCREEN,
#endif
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Leds RGB */ FUNC_RGB_LED,
  /* Logs SD */ FUNC_LOGS,
#if !defined(OLED_SCREEN)
  /* LuzFundo */ FUNC_BACKLIGHT,
#endif
  /* ModCorrida */ FUNC_RACING_MODE,
#if defined(COLORLCD)
  /* No Touch */ FUNC_DISABLE_TOUCH,
#endif
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Rep Valor */ FUNC_PLAY_VALUE,
  /* Reset */ FUNC_RESET,
  /* Script Lua */ FUNC_PLAY_SCRIPT,
  /* Sobrescrv */ FUNC_OVERRIDE_CHANNEL,
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Tocar Som */ FUNC_PLAY_SOUND,
  /* Tocar Trilha */ FUNC_PLAY_TRACK,
  /* Trainer */ FUNC_TRAINER,
  /* Trim Inst. */ FUNC_INSTANT_TRIM,
  /* Vario */ FUNC_VARIO,
  /* Vibrar */ FUNC_HAPTIC,
  /* Volume */ FUNC_VOLUME,
#elif defined(TRANSLATIONS_RU)
  /* Bind модуль */ FUNC_BIND,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lua скрипт */ FUNC_PLAY_SCRIPT,
  /* Play Value */ FUNC_PLAY_VALUE,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RGB подсветка */ FUNC_RGB_LED,
  /* SD логи */ FUNC_LOGS,
#if !defined(COLORLCD)
  /* Set Screen */ FUNC_SET_SCREEN,
#endif
  /* Авар управ */ FUNC_SET_FAILSAFE,
  /* Варио */ FUNC_VARIO,
  /* Вибро */ FUNC_HAPTIC,
  /* Воспр звук */ FUNC_PLAY_SOUND,
  /* Воспр трек */ FUNC_PLAY_TRACK,
#if defined(COLORLCD)
  /* Выбр глав экран */ FUNC_SET_SCREEN,
#endif
  /* Вык аудио усил. */ FUNC_DISABLE_AUDIO_AMP,
  /* Громкость */ FUNC_VOLUME,
  /* Мгнов трим */ FUNC_INSTANT_TRIM,
  /* Муз || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Муз фон */ FUNC_BACKGND_MUSIC,
  /* Настройка */ FUNC_ADJUST_GVAR,
  /* Оменить */ FUNC_OVERRIDE_CHANNEL,
#if defined(OLED_SCREEN)
  /* Подсветка */ FUNC_BACKLIGHT,
#endif
#if !defined(OLED_SCREEN)
  /* Подсветка */ FUNC_BACKLIGHT,
#endif
  /* Пров диапаз */ FUNC_RANGECHECK,
  /* Режим гонка */ FUNC_RACING_MODE,
  /* Сброс */ FUNC_RESET,
#if defined(COLORLCD)
  /* Сенсор вык */ FUNC_DISABLE_TOUCH,
#endif
  /* Скриншот */ FUNC_SCREENSHOT,
  /* Таймер */ FUNC_TRAINER,
#if defined(DEBUG)
  /* Тест */ FUNC_TEST,
#endif
  /* Установ */ FUNC_SET_TIMER,
#elif defined(TRANSLATIONS_SE)
  /* Audioförstärkare av */ FUNC_DISABLE_AUDIO_AMP,
#if !defined(OLED_SCREEN)
  /* Belysning */ FUNC_BACKLIGHT,
#endif
#if defined(COLORLCD)
  /* Ej pekskärm */ FUNC_DISABLE_TOUCH,
#endif
  /* Haptisk */ FUNC_HAPTIC,
  /* Justera */ FUNC_ADJUST_GVAR,
#if defined(VIDEO_SWITCH)
  /* LCD till Video */ FUNC_LCD_TO_VIDEO,
#endif
#if defined(OLED_SCREEN)
  /* Ljusstyrka */ FUNC_BACKLIGHT,
#endif
  /* Logga data */ FUNC_LOGS,
  /* Lua-skript */ FUNC_PLAY_SCRIPT,
  /* Lås */ FUNC_OVERRIDE_CHANNEL,
  /* Lärare */ FUNC_TRAINER,
  /* Musik */ FUNC_BACKGND_MUSIC,
  /* Parkoppla modul */ FUNC_BIND,
  /* Pausa Musik */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Range check */ FUNC_RANGECHECK,
  /* RGB ledljus */ FUNC_RGB_LED,
  /* Skärmbild */ FUNC_SCREENSHOT,
  /* Spara trimmar */ FUNC_INSTANT_TRIM,
  /* Spela ljud */ FUNC_PLAY_SOUND,
  /* Spela upp */ FUNC_PLAY_TRACK,
  /* Säg värdet */ FUNC_PLAY_VALUE,
  /* Sätt */ FUNC_SET_TIMER,
  /* Sätt failsafe */ FUNC_SET_FAILSAFE,
#if defined(COLORLCD)
  /* Sätt huvudskärm */ FUNC_SET_SCREEN,
#endif
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
#if defined(FUNCTION_SWITCHES)
  /* Tryck CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* Tävlingsläge */ FUNC_RACING_MODE,
  /* Vario */ FUNC_VARIO,
#if !defined(COLORLCD)
  /* Visa skärm */ FUNC_SET_SCREEN,
#endif
  /* Volym */ FUNC_VOLUME,
  /* Återställ */ FUNC_RESET,
#elif defined(TRANSLATIONS_TW)
  /* Lua腳本 */ FUNC_PLAY_SCRIPT,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RGB led燈 */ FUNC_RGB_LED,
  /* Vario傳感器 */ FUNC_VARIO,
#if defined(OLED_SCREEN)
  /* 亮度 */ FUNC_BACKLIGHT,
#endif
  /* 修改全局變量GV值 */ FUNC_ADJUST_GVAR,
#if defined(VIDEO_SWITCH)
  /* 屏幕顯示圖傳 */ FUNC_LCD_TO_VIDEO,
#endif
  /* 截屏 */ FUNC_SCREENSHOT,
  /* 振動 */ FUNC_HAPTIC,
  /* 搖桿值存儲到微調 */ FUNC_INSTANT_TRIM,
  /* 播放數值 */ FUNC_PLAY_VALUE,
  /* 播放聲音 */ FUNC_PLAY_SOUND,
  /* 播放背景音樂 */ FUNC_BACKGND_MUSIC,
  /* 播放音頻文件 */ FUNC_PLAY_TRACK,
  /* 教練 */ FUNC_TRAINER,
  /* 暫停背景音樂 */ FUNC_BACKGND_MUSIC_PAUSE,
  /* 模塊對頻 */ FUNC_BIND,
#if defined(DEBUG)
  /* 測試 */ FUNC_TEST,
#endif
  /* 測距模式 */ FUNC_RANGECHECK,
#if defined(COLORLCD)
  /* 禁用觸摸 */ FUNC_DISABLE_TOUCH,
#endif
  /* 競速模式 */ FUNC_RACING_MODE,
#if !defined(OLED_SCREEN)
  /* 背光 */ FUNC_BACKLIGHT,
#endif
  /* 記錄日誌到SD卡 */ FUNC_LOGS,
  /* 設置 */ FUNC_SET_TIMER,
#if !defined(COLORLCD)
  /* 設置主屏 */ FUNC_SET_SCREEN,
#endif
  /* 設置失控保護 */ FUNC_SET_FAILSAFE,
#if defined(COLORLCD)
  /* 選擇主屏 */ FUNC_SET_SCREEN,
#endif
  /* 重啟 */ FUNC_RESET,
  /* 鎖定通道值 */ FUNC_OVERRIDE_CHANNEL,
  /* 關閉音頻功放 */ FUNC_DISABLE_AUDIO_AMP,
  /* 音量 */ FUNC_VOLUME,
#elif defined(TRANSLATIONS_UA)
  /* АварРежим */ FUNC_SET_FAILSAFE,
  /* Варіо */ FUNC_VARIO,
#if defined(COLORLCD)
  /* Вибір гол. екр. */ FUNC_SET_SCREEN,
#endif
  /* Вібро */ FUNC_HAPTIC,
  /* Встан. */ FUNC_SET_TIMER,
  /* Грати Звук */ FUNC_PLAY_SOUND,
  /* Грати Знач. */ FUNC_PLAY_VALUE,
  /* Грати Трек */ FUNC_PLAY_TRACK,
  /* Гучність */ FUNC_VOLUME,
  /* Митєв. трим. */ FUNC_INSTANT_TRIM,
  /* Модуль Bind */ FUNC_BIND,
  /* Налашт. */ FUNC_ADJUST_GVAR,
  /* Перевизнач. */ FUNC_OVERRIDE_CHANNEL,
  /* ПеревірДіап. */ FUNC_RANGECHECK,
#if !defined(OLED_SCREEN)
  /* Підсвіт. */ FUNC_BACKLIGHT,
#endif
  /* Підсил. вимк. */ FUNC_DISABLE_AUDIO_AMP,
  /* Режим перегонів */ FUNC_RACING_MODE,
#if defined(COLORLCD)
  /* Сенсор вимк. */ FUNC_DISABLE_TOUCH,
#endif
  /* Скид. */ FUNC_RESET,
  /* Скріншот */ FUNC_SCREENSHOT,
#if defined(DEBUG)
  /* Тест */ FUNC_TEST,
#endif
  /* Тренер */ FUNC_TRAINER,
  /* Фон.Муз. */ FUNC_BACKGND_MUSIC,
  /* Фон.Муз. || */ FUNC_BACKGND_MUSIC_PAUSE,
#if defined(OLED_SCREEN)
  /* Яскравість */ FUNC_BACKLIGHT,
#endif
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lua Скрипт */ FUNC_PLAY_SCRIPT,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RGB leds */ FUNC_RGB_LED,
  /* SD лог */ FUNC_LOGS,
#if !defined(COLORLCD)
  /* Set Screen */ FUNC_SET_SCREEN,
#endif
#else
  /* Adjust */ FUNC_ADJUST_GVAR,
  /* Audio Amp Off */ FUNC_DISABLE_AUDIO_AMP,
#if !defined(OLED_SCREEN)
  /* Backlight */ FUNC_BACKLIGHT,
#endif
  /* BgMusic */ FUNC_BACKGND_MUSIC,
  /* BgMusic || */ FUNC_BACKGND_MUSIC_PAUSE,
#if defined(OLED_SCREEN)
  /* Brightness */ FUNC_BACKLIGHT,
#endif
  /* Haptic */ FUNC_HAPTIC,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
#if defined(VIDEO_SWITCH)
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#endif
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
#if defined(COLORLCD)
  /* No Touch */ FUNC_DISABLE_TOUCH,
#endif
  /* Override */ FUNC_OVERRIDE_CHANNEL,
  /* Play Sound */ FUNC_PLAY_SOUND,
  /* Play Track */ FUNC_PLAY_TRACK,
  /* Play Value */ FUNC_PLAY_VALUE,
#if defined(FUNCTION_SWITCHES)
  /* Push CS */ FUNC_PUSH_CUST_SWITCH,
#endif
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Reset */ FUNC_RESET,
  /* RGB leds */ FUNC_RGB_LED,
  /* Screenshot */ FUNC_SCREENSHOT,
  /* SD Logs */ FUNC_LOGS,
  /* Set */ FUNC_SET_TIMER,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
#if defined(COLORLCD)
  /* Set Main Screen */ FUNC_SET_SCREEN,
#endif
#if !defined(COLORLCD)
  /* Set Screen */ FUNC_SET_SCREEN,
#endif
#if defined(DEBUG)
  /* Test */ FUNC_TEST,
#endif
  /* Trainer */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Volume */ FUNC_VOLUME,
#endif
};

uint8_t getFuncSortIdx(uint8_t func)
{
  for (uint8_t i = 0; i < FUNC_MAX; i += 1)
    if (cfn_sorted[i] == func)
      return i;
  return 0;
}
