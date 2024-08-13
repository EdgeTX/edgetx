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

// This file is auto-generated. Do not edit.

#include "dataconstants.h"

Functions cfn_sorted[] = {
#if   defined(TRANSLATIONS_CN)

  /* Lua脚本 */ FUNC_PLAY_SCRIPT,
  /* RGB LED 灯 */ FUNC_RGB_LED,
  /* Vario传感器 */ FUNC_VARIO,
#if defined(OLED_SCREEN)
  /* 亮度 */ FUNC_BACKLIGHT,
#endif
  /* 复位 */ FUNC_RESET,
  /* 截屏 */ FUNC_SCREENSHOT,
  /* 振动 */ FUNC_HAPTIC,
  /* 教练 */ FUNC_TRAINER,
  /* 测试 */ FUNC_TEST,
#if !defined(OLED_SCREEN)
  /* 背光 */ FUNC_BACKLIGHT,
#endif
  /* 设置 */ FUNC_SET_TIMER,
  /* 音量 */ FUNC_VOLUME,
  /* 播放声音 */ FUNC_PLAY_SOUND,
  /* 播放数值 */ FUNC_PLAY_VALUE,
  /* 模块对频 */ FUNC_BIND,
  /* 测距模式 */ FUNC_RANGECHECK,
  /* 禁用触摸 */ FUNC_DISABLE_TOUCH,
  /* 竞速模式 */ FUNC_RACING_MODE,
  /* 选择主屏 */ FUNC_SET_SCREEN,
  /* 锁定通道值 */ FUNC_OVERRIDE_CHANNEL,
  /* 记录日志到SD卡 */ FUNC_LOGS,
  /* 关闭音频功放 */ FUNC_DISABLE_AUDIO_AMP,
  /* 屏幕显示图传 */ FUNC_LCD_TO_VIDEO,
  /* 播放背景音乐 */ FUNC_BACKGND_MUSIC,
  /* 播放音频文件 */ FUNC_PLAY_TRACK,
  /* 暂停背景音乐 */ FUNC_BACKGND_MUSIC_PAUSE,
  /* 设置失控保护 */ FUNC_SET_FAILSAFE,
  /* 修改全局变量GV值 */ FUNC_ADJUST_GVAR,
  /* 摇杆值存储到微调 */ FUNC_INSTANT_TRIM,

#elif defined(TRANSLATIONS_CZ)

  /* Deaktivace dotyku */ FUNC_DISABLE_TOUCH,
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
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Loguj na SD */ FUNC_LOGS,
  /* Lua Skript */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
  /* Nastav */ FUNC_ADJUST_GVAR,
  /* Nastavit Failsafe */ FUNC_SET_FAILSAFE,
#if !defined(OLED_SCREEN)
  /* Podsvětlení */ FUNC_BACKLIGHT,
#endif
  /* Přehrát wav */ FUNC_PLAY_TRACK,
  /* RGB světlo */ FUNC_RGB_LED,
  /* Reset */ FUNC_RESET,
  /* Snímek LCD */ FUNC_SCREENSHOT,
  /* Test */ FUNC_TEST,
  /* Trenér */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Vibrovat */ FUNC_HAPTIC,
  /* Vybrat hlavní obrazovku */ FUNC_SET_SCREEN,
  /* Vypnutí zesilovače zvuku */ FUNC_DISABLE_AUDIO_AMP,
  /* Změna */ FUNC_SET_TIMER,
  /* Zámek */ FUNC_OVERRIDE_CHANNEL,
  /* Závodní režim */ FUNC_RACING_MODE,

#elif defined(TRANSLATIONS_DA)

  /* Afspil lydfil */ FUNC_PLAY_TRACK,
  /* Afstand kontrol */ FUNC_RANGECHECK,
#if !defined(OLED_SCREEN)
  /* Baggrundslys */ FUNC_BACKLIGHT,
#endif
  /* BgMusik */ FUNC_BACKGND_MUSIC,
  /* BgMusik || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Ikke berøringsaktiv */ FUNC_DISABLE_TOUCH,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
  /* Juster */ FUNC_ADJUST_GVAR,
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* Lydstyrke */ FUNC_VOLUME,
  /* Modul tilslut */ FUNC_BIND,
  /* Nulstil */ FUNC_RESET,
  /* Overskriv */ FUNC_OVERRIDE_CHANNEL,
  /* RGB led */ FUNC_RGB_LED,
  /* Ræs tilstand */ FUNC_RACING_MODE,
  /* SD Log */ FUNC_LOGS,
  /* Sæt */ FUNC_SET_TIMER,
  /* Sæt fejlsikring */ FUNC_SET_FAILSAFE,
  /* Skærm klip */ FUNC_SCREENSHOT,
#if defined(OLED_SCREEN)
  /* Skarphed */ FUNC_BACKLIGHT,
#endif
  /* Sluk audio amplifier */ FUNC_DISABLE_AUDIO_AMP,
  /* Spil Værdi */ FUNC_PLAY_VALUE,
  /* Spil lyd */ FUNC_PLAY_SOUND,
  /* Test */ FUNC_TEST,
  /* Træner */ FUNC_TRAINER,
  /* Vælg hoved skærm */ FUNC_SET_SCREEN,
  /* Vario */ FUNC_VARIO,
  /* Vibration */ FUNC_HAPTIC,

#elif defined(TRANSLATIONS_DE)

  /* Ändere */ FUNC_ADJUST_GVAR,
  /* Audio Verst. Aus */ FUNC_DISABLE_AUDIO_AMP,
  /* Haptik */ FUNC_HAPTIC,
#if defined(OLED_SCREEN)
  /* Helligkeit */ FUNC_BACKLIGHT,
#endif
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
  /* Kein Touch */ FUNC_DISABLE_TOUCH,
#if !defined(OLED_SCREEN)
  /* LCD Licht */ FUNC_BACKLIGHT,
#endif
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lautstr. */ FUNC_VOLUME,
  /* Lehrer */ FUNC_TRAINER,
  /* Lua Skript */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
  /* RGB LED */ FUNC_RGB_LED,
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Rücksetz. */ FUNC_RESET,
  /* SD-Aufz. */ FUNC_LOGS,
  /* Sag Text */ FUNC_PLAY_TRACK,
  /* Sag Wert */ FUNC_PLAY_VALUE,
  /* Screenshot */ FUNC_SCREENSHOT,
  /* Set Main Screen */ FUNC_SET_SCREEN,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
  /* Setze */ FUNC_SET_TIMER,
  /* Spiel Töne */ FUNC_PLAY_SOUND,
  /* StartMusik */ FUNC_BACKGND_MUSIC,
  /* Stop Musik */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Test */ FUNC_TEST,
  /* Überschreibe */ FUNC_OVERRIDE_CHANNEL,
  /* Vario */ FUNC_VARIO,

#elif defined(TRANSLATIONS_ES)

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
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lua Script */ FUNC_PLAY_SCRIPT,
#if !defined(OLED_SCREEN)
  /* Luz Fondo */ FUNC_BACKLIGHT,
#endif
  /* No Touch */ FUNC_DISABLE_TOUCH,
  /* Oir pista */ FUNC_PLAY_TRACK,
  /* Oir sonido */ FUNC_PLAY_SOUND,
  /* Oir valor */ FUNC_PLAY_VALUE,
  /* RGB leds */ FUNC_RGB_LED,
  /* RacingMode */ FUNC_RACING_MODE,
  /* Reset */ FUNC_RESET,
  /* SD Logs */ FUNC_LOGS,
  /* Seguro */ FUNC_OVERRIDE_CHANNEL,
  /* Set Main Screen */ FUNC_SET_SCREEN,
  /* Test */ FUNC_TEST,
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
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
  /* No Touch */ FUNC_DISABLE_TOUCH,
  /* Play Sound */ FUNC_PLAY_SOUND,
  /* Play Track */ FUNC_PLAY_TRACK,
  /* Play Value */ FUNC_PLAY_VALUE,
  /* RGB leds */ FUNC_RGB_LED,
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Reset */ FUNC_RESET,
  /* SD Logs */ FUNC_LOGS,
  /* Safety */ FUNC_OVERRIDE_CHANNEL,
  /* Screenshot */ FUNC_SCREENSHOT,
  /* Set */ FUNC_SET_TIMER,
  /* Set Main Screen */ FUNC_SET_SCREEN,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
  /* Test */ FUNC_TEST,
  /* Trainer */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Volume */ FUNC_VOLUME,

#elif defined(TRANSLATIONS_FR)

  /* Ajuster */ FUNC_ADJUST_GVAR,
  /* Bind */ FUNC_BIND,
  /* Déf. */ FUNC_SET_TIMER,
  /* Définir Écran Princ. */ FUNC_SET_SCREEN,
  /* Désact. Ampli Audio */ FUNC_DISABLE_AUDIO_AMP,
  /* Écolage */ FUNC_TRAINER,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
  /* Jouer fichier */ FUNC_PLAY_TRACK,
  /* Jouer son */ FUNC_PLAY_SOUND,
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lire valeur */ FUNC_PLAY_VALUE,
  /* Logs SD */ FUNC_LOGS,
#if defined(OLED_SCREEN)
  /* Luminosité */ FUNC_BACKLIGHT,
#endif
  /* Musique */ FUNC_BACKGND_MUSIC,
  /* Non Tactile */ FUNC_DISABLE_TOUCH,
  /* Pause Musique */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Photo Écran */ FUNC_SCREENSHOT,
  /* RGB leds */ FUNC_RGB_LED,
  /* Racing Mode */ FUNC_RACING_MODE,
  /* Régle Failsafe */ FUNC_SET_FAILSAFE,
  /* Remise à 0 */ FUNC_RESET,
  /* Remplace */ FUNC_OVERRIDE_CHANNEL,
#if !defined(OLED_SCREEN)
  /* Rétroécl. */ FUNC_BACKLIGHT,
#endif
  /* Script Lua */ FUNC_PLAY_SCRIPT,
  /* Test */ FUNC_TEST,
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
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
  /* Override */ FUNC_OVERRIDE_CHANNEL,
  /* Play Sound */ FUNC_PLAY_SOUND,
  /* Play Track */ FUNC_PLAY_TRACK,
  /* Play Value */ FUNC_PLAY_VALUE,
  /* RGB leds */ FUNC_RGB_LED,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Reset */ FUNC_RESET,
  /* SD Logs */ FUNC_LOGS,
  /* Set */ FUNC_SET_TIMER,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
  /* Test */ FUNC_TEST,
  /* Trainer */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Volume */ FUNC_VOLUME,
  /* ללא מסך מגע */ FUNC_DISABLE_TOUCH,
  /* מצב תחרות */ FUNC_RACING_MODE,
  /* צילום מסך */ FUNC_SCREENSHOT,
  /* הגדרת מסך ראשי */ FUNC_SET_SCREEN,
#if defined(OLED_SCREEN)
  /* בהירות */ FUNC_BACKLIGHT,
#endif

#elif defined(TRANSLATIONS_IT)

  /* Amp Audio Off */ FUNC_DISABLE_AUDIO_AMP,
  /* Azzera */ FUNC_RESET,
  /* BindModulo */ FUNC_BIND,
  /* Blocco */ FUNC_OVERRIDE_CHANNEL,
  /* Inst. Trim */ FUNC_INSTANT_TRIM,
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
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
  /* No Touch */ FUNC_DISABLE_TOUCH,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Regola */ FUNC_ADJUST_GVAR,
#if !defined(OLED_SCREEN)
  /* Retroillum. */ FUNC_BACKLIGHT,
#endif
  /* Screenshot */ FUNC_SCREENSHOT,
  /* Script Lua */ FUNC_PLAY_SCRIPT,
  /* Set */ FUNC_SET_TIMER,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
  /* Setta Schermo Princ. */ FUNC_SET_SCREEN,
  /* Suona */ FUNC_PLAY_SOUND,
  /* Suona Traccia */ FUNC_PLAY_TRACK,
  /* Test */ FUNC_TEST,
  /* Vario */ FUNC_VARIO,
  /* Vibrazione */ FUNC_HAPTIC,
  /* Volume */ FUNC_VOLUME,

#elif defined(TRANSLATIONS_JP)

  /* BGM再生 */ FUNC_BACKGND_MUSIC,
  /* BGM一時停止 */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Failsafe設定 */ FUNC_SET_FAILSAFE,
  /* G変数修正 */ FUNC_ADJUST_GVAR,
  /* Inst.トリム */ FUNC_INSTANT_TRIM,
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* LUAスクリプト */ FUNC_PLAY_SCRIPT,
  /* RGB LED */ FUNC_RGB_LED,
  /* SDログ出力 */ FUNC_LOGS,
  /* 設定 */ FUNC_SET_TIMER,
#if defined(OLED_SCREEN)
  /* 輝度 */ FUNC_BACKLIGHT,
#endif
  /* テスト */ FUNC_TEST,
  /* バリオ */ FUNC_VARIO,
  /* 上書き */ FUNC_OVERRIDE_CHANNEL,
  /* 再生値 */ FUNC_PLAY_VALUE,
  /* ダイヤル */ FUNC_VOLUME,
  /* リセット */ FUNC_RESET,
  /* 非タッチ */ FUNC_DISABLE_TOUCH,
  /* 音源再生 */ FUNC_PLAY_TRACK,
  /* トレーナー */ FUNC_TRAINER,
  /* サウンド再生 */ FUNC_PLAY_SOUND,
  /* バイブレート */ FUNC_HAPTIC,
#if !defined(OLED_SCREEN)
  /* バックライト */ FUNC_BACKLIGHT,
#endif
  /* レースモード */ FUNC_RACING_MODE,
  /* メイン画面設定 */ FUNC_SET_SCREEN,
  /* レンジチェック */ FUNC_RANGECHECK,
  /* 画面キャプチャ */ FUNC_SCREENSHOT,
  /* オーディオアンプ OFF */ FUNC_DISABLE_AUDIO_AMP,
  /* モジュールバインド */ FUNC_BIND,

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
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
  /* No Touch */ FUNC_DISABLE_TOUCH,
  /* Override */ FUNC_OVERRIDE_CHANNEL,
  /* Play Track */ FUNC_PLAY_TRACK,
  /* Play Value */ FUNC_PLAY_VALUE,
  /* RGB leds */ FUNC_RGB_LED,
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Reset */ FUNC_RESET,
  /* SD Logs */ FUNC_LOGS,
  /* Schermafdr */ FUNC_SCREENSHOT,
  /* Set */ FUNC_SET_TIMER,
  /* Set Main Screen */ FUNC_SET_SCREEN,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
  /* Test */ FUNC_TEST,
  /* Trainer */ FUNC_TRAINER,
  /* Vario */ FUNC_VARIO,
  /* Volume */ FUNC_VOLUME,
  /* Wijzig */ FUNC_ADJUST_GVAR,

#elif defined(TRANSLATIONS_PL)

  /* Bezpiecz */ FUNC_OVERRIDE_CHANNEL,
  /* GrajDźwięk */ FUNC_PLAY_SOUND,
  /* GrajWartoś */ FUNC_PLAY_VALUE,
  /* GrajŚcieżk */ FUNC_PLAY_TRACK,
  /* Głośność */ FUNC_VOLUME,
  /* Inst-Trim */ FUNC_INSTANT_TRIM,
#if defined(OLED_SCREEN)
  /* Jasność */ FUNC_BACKLIGHT,
#endif
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Logi->SD */ FUNC_LOGS,
  /* ModuleBind */ FUNC_BIND,
  /* Muz. tła */ FUNC_BACKGND_MUSIC,
  /* Muz. tła || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* No Touch */ FUNC_DISABLE_TOUCH,
#if !defined(OLED_SCREEN)
  /* Podświetl */ FUNC_BACKLIGHT,
#endif
  /* RGB ledy */ FUNC_RGB_LED,
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Resetuj */ FUNC_RESET,
  /* Set Main Screen */ FUNC_SET_SCREEN,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
  /* SkryptyLua */ FUNC_PLAY_SCRIPT,
  /* Test */ FUNC_TEST,
  /* Trener */ FUNC_TRAINER,
  /* Ustaw */ FUNC_ADJUST_GVAR,
  /* Ustaw */ FUNC_SET_TIMER,
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
  /* Def Tela Princ */ FUNC_SET_SCREEN,
  /* DefFailsafe */ FUNC_SET_FAILSAFE,
  /* Definir */ FUNC_SET_TIMER,
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Leds RGB */ FUNC_RGB_LED,
  /* Logs SD */ FUNC_LOGS,
#if !defined(OLED_SCREEN)
  /* LuzFundo */ FUNC_BACKLIGHT,
#endif
  /* ModCorrida */ FUNC_RACING_MODE,
  /* No Touch */ FUNC_DISABLE_TOUCH,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Rep Valor */ FUNC_PLAY_VALUE,
  /* Reset */ FUNC_RESET,
  /* Script Lua */ FUNC_PLAY_SCRIPT,
  /* Sobrescrv */ FUNC_OVERRIDE_CHANNEL,
  /* Test */ FUNC_TEST,
  /* Tocar Som */ FUNC_PLAY_SOUND,
  /* Tocar Trilha */ FUNC_PLAY_TRACK,
  /* Trainer */ FUNC_TRAINER,
  /* Trim Inst. */ FUNC_INSTANT_TRIM,
  /* Vario */ FUNC_VARIO,
  /* Vibrar */ FUNC_HAPTIC,
  /* Volume */ FUNC_VOLUME,

#elif defined(TRANSLATIONS_RU)

  /* Bind модуль */ FUNC_BIND,
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lua скрипт */ FUNC_PLAY_SCRIPT,
  /* Play Value */ FUNC_PLAY_VALUE,
  /* RGB подсветка */ FUNC_RGB_LED,
  /* SD логи */ FUNC_LOGS,
  /* Муз || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Муз фон */ FUNC_BACKGND_MUSIC,
  /* Вык аудио усил. */ FUNC_DISABLE_AUDIO_AMP,
  /* Тест */ FUNC_TEST,
  /* Выбр глав экран */ FUNC_SET_SCREEN,
  /* Авар управ */ FUNC_SET_FAILSAFE,
  /* Пров диапаз */ FUNC_RANGECHECK,
  /* Варио */ FUNC_VARIO,
  /* Вибро */ FUNC_HAPTIC,
  /* Сброс */ FUNC_RESET,
  /* Воспр звук */ FUNC_PLAY_SOUND,
  /* Воспр трек */ FUNC_PLAY_TRACK,
  /* Мгнов трим */ FUNC_INSTANT_TRIM,
  /* Режим гонка */ FUNC_RACING_MODE,
  /* Таймер */ FUNC_TRAINER,
  /* Сенсор вык */ FUNC_DISABLE_TOUCH,
  /* Оменить */ FUNC_OVERRIDE_CHANNEL,
  /* Установ */ FUNC_SET_TIMER,
  /* Скриншот */ FUNC_SCREENSHOT,
  /* Громкость */ FUNC_VOLUME,
  /* Настройка */ FUNC_ADJUST_GVAR,
#if defined(OLED_SCREEN)
  /* Подсветка */ FUNC_BACKLIGHT,
#endif
#if defined(OLED_SCREEN)
  /* Подсветка */ FUNC_BACKLIGHT,
#endif

#elif defined(TRANSLATIONS_SE)

  /* Audioförstärkare av */ FUNC_DISABLE_AUDIO_AMP,
#if !defined(OLED_SCREEN)
  /* Belysning */ FUNC_BACKLIGHT,
#endif
  /* Ej pekskärm */ FUNC_DISABLE_TOUCH,
  /* Haptisk */ FUNC_HAPTIC,
  /* Justera */ FUNC_ADJUST_GVAR,
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
#if defined(OLED_SCREEN)
  /* Ljusstyrka */ FUNC_BACKLIGHT,
#endif
  /* Logga data */ FUNC_LOGS,
  /* Lua-skript */ FUNC_PLAY_SCRIPT,
  /* Lärare */ FUNC_TRAINER,
  /* Lås */ FUNC_OVERRIDE_CHANNEL,
  /* Musik */ FUNC_BACKGND_MUSIC,
  /* Parkoppla modul */ FUNC_BIND,
  /* Pausa Musik */ FUNC_BACKGND_MUSIC_PAUSE,
  /* RGB ledljus */ FUNC_RGB_LED,
  /* Range check */ FUNC_RANGECHECK,
  /* Skärmbild */ FUNC_SCREENSHOT,
  /* Spara trimmar */ FUNC_INSTANT_TRIM,
  /* Spela ljud */ FUNC_PLAY_SOUND,
  /* Spela upp */ FUNC_PLAY_TRACK,
  /* Säg värdet */ FUNC_PLAY_VALUE,
  /* Sätt */ FUNC_SET_TIMER,
  /* Sätt failsafe */ FUNC_SET_FAILSAFE,
  /* Sätt huvudskärm */ FUNC_SET_SCREEN,
  /* Test */ FUNC_TEST,
  /* Tävlingsläge */ FUNC_RACING_MODE,
  /* Vario */ FUNC_VARIO,
  /* Volym */ FUNC_VOLUME,
  /* Återställ */ FUNC_RESET,

#elif defined(TRANSLATIONS_TW)

  /* Lua腳本 */ FUNC_PLAY_SCRIPT,
  /* RGB led燈 */ FUNC_RGB_LED,
  /* Vario傳感器 */ FUNC_VARIO,
#if defined(OLED_SCREEN)
  /* 亮度 */ FUNC_BACKLIGHT,
#endif
  /* 截屏 */ FUNC_SCREENSHOT,
  /* 振動 */ FUNC_HAPTIC,
  /* 教練 */ FUNC_TRAINER,
  /* 測試 */ FUNC_TEST,
#if !defined(OLED_SCREEN)
  /* 背光 */ FUNC_BACKLIGHT,
#endif
  /* 設置 */ FUNC_SET_TIMER,
  /* 重啟 */ FUNC_RESET,
  /* 音量 */ FUNC_VOLUME,
  /* 播放數值 */ FUNC_PLAY_VALUE,
  /* 播放聲音 */ FUNC_PLAY_SOUND,
  /* 模塊對頻 */ FUNC_BIND,
  /* 測距模式 */ FUNC_RANGECHECK,
  /* 禁用觸摸 */ FUNC_DISABLE_TOUCH,
  /* 競速模式 */ FUNC_RACING_MODE,
  /* 選擇主屏 */ FUNC_SET_SCREEN,
  /* 鎖定通道值 */ FUNC_OVERRIDE_CHANNEL,
  /* 記錄日誌到SD卡 */ FUNC_LOGS,
  /* 屏幕顯示圖傳 */ FUNC_LCD_TO_VIDEO,
  /* 播放背景音樂 */ FUNC_BACKGND_MUSIC,
  /* 播放音頻文件 */ FUNC_PLAY_TRACK,
  /* 暫停背景音樂 */ FUNC_BACKGND_MUSIC_PAUSE,
  /* 設置失控保護 */ FUNC_SET_FAILSAFE,
  /* 關閉音頻功放 */ FUNC_DISABLE_AUDIO_AMP,
  /* 修改全局變量GV值 */ FUNC_ADJUST_GVAR,
  /* 搖桿值存儲到微調 */ FUNC_INSTANT_TRIM,

#elif defined(TRANSLATIONS_UA)

  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lua Скрипт */ FUNC_PLAY_SCRIPT,
  /* RGB leds */ FUNC_RGB_LED,
  /* SD лог */ FUNC_LOGS,
  /* Фон.Муз. */ FUNC_BACKGND_MUSIC,
  /* Фон.Муз. || */ FUNC_BACKGND_MUSIC_PAUSE,
  /* Тест */ FUNC_TEST,
  /* Скид. */ FUNC_RESET,
  /* Варіо */ FUNC_VARIO,
  /* Вібро */ FUNC_HAPTIC,
  /* Вибір гол. екр. */ FUNC_SET_SCREEN,
  /* Грати Звук */ FUNC_PLAY_SOUND,
  /* Грати Трек */ FUNC_PLAY_TRACK,
  /* Грати Знач. */ FUNC_PLAY_VALUE,
  /* Режим перегонів */ FUNC_RACING_MODE,
  /* Встан. */ FUNC_SET_TIMER,
  /* Митєв. трим. */ FUNC_INSTANT_TRIM,
  /* Тренер */ FUNC_TRAINER,
  /* Модуль Bind */ FUNC_BIND,
  /* Сенсор вимк. */ FUNC_DISABLE_TOUCH,
  /* Налашт. */ FUNC_ADJUST_GVAR,
  /* Підсил. вимк. */ FUNC_DISABLE_AUDIO_AMP,
#if !defined(OLED_SCREEN)
  /* Підсвіт. */ FUNC_BACKLIGHT,
#endif
  /* Гучність */ FUNC_VOLUME,
  /* Скріншот */ FUNC_SCREENSHOT,
  /* АварРежим */ FUNC_SET_FAILSAFE,
#if defined(OLED_SCREEN)
  /* Яскравість */ FUNC_BACKLIGHT,
#endif
  /* Перевизнач. */ FUNC_OVERRIDE_CHANNEL,
  /* ПеревірДіап. */ FUNC_RANGECHECK,

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
  /* LCD to Video */ FUNC_LCD_TO_VIDEO,
  /* Lua Script */ FUNC_PLAY_SCRIPT,
  /* ModuleBind */ FUNC_BIND,
  /* No Touch */ FUNC_DISABLE_TOUCH,
  /* Override */ FUNC_OVERRIDE_CHANNEL,
  /* Play Sound */ FUNC_PLAY_SOUND,
  /* Play Track */ FUNC_PLAY_TRACK,
  /* Play Value */ FUNC_PLAY_VALUE,
  /* RGB leds */ FUNC_RGB_LED,
  /* RacingMode */ FUNC_RACING_MODE,
  /* RangeCheck */ FUNC_RANGECHECK,
  /* Reset */ FUNC_RESET,
  /* SD Logs */ FUNC_LOGS,
  /* Screenshot */ FUNC_SCREENSHOT,
  /* Set */ FUNC_SET_TIMER,
  /* Set Main Screen */ FUNC_SET_SCREEN,
  /* SetFailsafe */ FUNC_SET_FAILSAFE,
  /* Test */ FUNC_TEST,
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
