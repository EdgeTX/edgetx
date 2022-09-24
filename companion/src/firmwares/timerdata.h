/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#pragma once

#include "rawswitch.h"
#include "datahelpers.h"

#include <QtCore>

class RadioDataConversionState;
class AbstractStaticItemModel;

constexpr char AIM_TIMER_COUNTDOWNBEEP[]  {"timerdata.countdownBeep"};
constexpr char AIM_TIMER_COUNTDOWNSTART[] {"timerdata.countdownStart"};
constexpr char AIM_TIMER_PERSISTENT[]     {"timerdata.persistent"};
constexpr char AIM_TIMER_MODE[]           {"timerdata.mode"};
constexpr char AIM_TIMER_SHOWELAPSED[]    {"timerdata.showElapsed"};

constexpr int TIMER_NAME_LEN {8};

class TimerData {
  Q_DECLARE_TR_FUNCTIONS(TimerData)

  public:
    enum CountDownBeepType {
      COUNTDOWNBEEP_SILENT,
      COUNTDOWNBEEP_BEEPS,
      COUNTDOWNBEEP_VOICE,
      COUNTDOWNBEEP_HAPTIC,
      COUNTDOWNBEEP_COUNT
    };

    enum CountDownStart {
      COUNTDOWNSTART_5 = -2,
      COUNTDOWNSTART_FIRST = COUNTDOWNSTART_5,
      COUNTDOWNSTART_10,
      COUNTDOWNSTART_20,
      COUNTDOWNSTART_30,
      COUNTDOWNSTART_LAST = COUNTDOWNSTART_30
    };

    enum PersistentType {
      PERSISTENT_NOT,
      PERSISTENT_FLIGHT,
      PERSISTENT_MANUALRESET,
      PERSISTENT_COUNT
    };

    enum TimerMode {
      TIMERMODE_OFF,
      TIMERMODE_ON,
      TIMERMODE_START,
      TIMERMODE_THR,
      TIMERMODE_THR_REL,
      TIMERMODE_THR_START,
      TIMERMODE_COUNT,
      TIMERMODE_MAX = TIMERMODE_COUNT - 1
    };

  enum TimerShowDirection {
    TIMER_SHOW_REMAINING,
    TIMER_SHOW_ELAPSED,
    TIMER_SHOW_COUNT
  };

    TimerData() { clear(); }

    RawSwitch    swtch;
    unsigned int mode;
    char         name[TIMER_NAME_LEN + 1];
    bool         minuteBeep;
    unsigned int countdownBeep;
    unsigned int val;
    unsigned int persistent;
    int          countdownStart;
    int          pvalue;
    unsigned int showElapsed;

    void convert(RadioDataConversionState & cstate);
    void clear();
    bool isEmpty() const;
    bool isModeOff();
    QString nameToString(int index) const;
    QString countdownBeepToString() const;
    QString countdownStartToString() const;
    QString persistentToString(const bool verbose = true) const;
    QString pvalueToString() const;
    QString valToString() const;
    QString modeToString() const;
    QString showElapsedToString() const;

    void countdownBeepChanged();
    void modeChanged();

    static QString countdownBeepToString(const int value);
    static QString countdownStartToString(const int value);
    static QString persistentToString(const int value, const bool verbose = true);
    static QString pvalueToString(const int value);
    static QString valToString(const int value);
    static QString modeToString(const int value);
    static QString showElapsedToString(const unsigned int value);
    static AbstractStaticItemModel * countdownBeepItemModel();
    static AbstractStaticItemModel * countdownStartItemModel();
    static AbstractStaticItemModel * persistentItemModel();
    static AbstractStaticItemModel * modeItemModel();
    static AbstractStaticItemModel * showElapsedItemModel();
};
