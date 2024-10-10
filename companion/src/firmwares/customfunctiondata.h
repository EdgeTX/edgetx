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

#pragma once

#include "boards.h"
#include "constants.h"
#include "rawswitch.h"

#include <QtCore>
#include <QComboBox>

class Firmware;
class ModelData;
class GeneralSettings;
class RadioDataConversionState;
class AbstractStaticItemModel;

enum AssignFunc {
  FuncOverrideCH1 = 0,
  FuncOverrideCHLast = FuncOverrideCH1 + CPN_MAX_CHNOUT - 1,
  FuncTrainer,
  FuncTrainerRUD,
  FuncTrainerELE,
  FuncTrainerTHR,
  FuncTrainerAIL,
  FuncTrainerChannels,
  FuncInstantTrim,
  FuncPlaySound,
  FuncPlayHaptic,
  FuncReset,
  FuncSetTimer1,
  FuncSetTimerLast = FuncSetTimer1 + CPN_MAX_TIMERS - 1,
  FuncVario,
  FuncPlayPrompt,
  FuncPlayBoth,
  FuncPlayValue,
  FuncPlayScript,
  FuncLogs,
  FuncVolume,
  FuncBacklight,
  FuncScreenshot,
  FuncBackgroundMusic,
  FuncBackgroundMusicPause,
  FuncAdjustGV1,
  FuncAdjustGVLast = FuncAdjustGV1 + CPN_MAX_GVARS - 1,
  FuncSetFailsafe,
  FuncRangeCheckInternalModule,
  FuncRangeCheckExternalModule,
  FuncBindInternalModule,
  FuncBindExternalModule,
  FuncRacingMode,
  FuncDisableTouch,
  FuncSetScreen,
  FuncDisableAudioAmp,
  FuncRGBLed,
  FuncLCDtoVideo,
  FuncPushCustomSwitch1,
  FuncPushCustomSwitchLast = FuncPushCustomSwitch1 + CPN_MAX_SWITCHES_FUNCTION - 1,
  FuncCount,
  FuncReserve = -1
};

enum GVarAdjustModes
{
  FUNC_ADJUST_GVAR_CONSTANT,
  FUNC_ADJUST_GVAR_SOURCE,
  FUNC_ADJUST_GVAR_SOURCERAW,
  FUNC_ADJUST_GVAR_GVAR,
  FUNC_ADJUST_GVAR_INCDEC,
  FUNC_ADJUST_GVAR_COUNT
};

class CustomFunctionData {
  Q_DECLARE_TR_FUNCTIONS(CustomFunctionData)

  public:
    enum CustomFunctionContext
    {
      GlobalFunctionsContext  = 0x01,
      SpecialFunctionsContext = 0x02,

      AllFunctionContexts     = GlobalFunctionsContext | SpecialFunctionsContext
    };

    CustomFunctionData(AssignFunc func = FuncOverrideCH1) { clear(); this->func = func; }
    RawSwitch    swtch;
    AssignFunc   func;
    int param;
    char paramarm[10];
    unsigned int enabled; // TODO perhaps not any more the right name
    unsigned int adjustMode;
    int repeatParam;

    void convert(RadioDataConversionState & cstate);

    void clear();
    bool isEmpty() const;
    QString funcToString(const ModelData * model = nullptr) const;
    QString paramToString(const ModelData * model = nullptr) const;
    QString repeatToString(const bool abbrev) const;
    QString enabledToString() const;
    QString playSoundToString() const;
    QString harpicToString() const;
    QString gvarAdjustModeToString() const;
    bool isRepeatParamAvailable() const;
    bool isParamAvailable() const;

    static QString nameToString(const int index, const bool globalContext = false);
    static QString funcToString(const AssignFunc func, const ModelData * model = nullptr);
    static bool isFuncAvailable(const int index, const ModelData * model = nullptr);
    static int funcContext(const int index);
    static QString resetToString(const int value, const ModelData * model = nullptr);
    static int resetParamCount();
    static bool isResetParamAvailable(const int index, const ModelData * model = nullptr);
    static QString repeatToString(const int value, const AssignFunc func, const bool abbrev);
    static QString repeatToString(const int value, const bool abbrev);
    static bool isRepeatParamAvailable(const AssignFunc func);
    static QStringList playSoundStringList();
    static QString playSoundToString(const int value);
    static QString harpicToString(const int value);
    static QStringList gvarAdjustModeStringList();
    static QString gvarAdjustModeToString(const int value);
    static AbstractStaticItemModel * repeatItemModel();
    static AbstractStaticItemModel * repeatLuaItemModel();
    static AbstractStaticItemModel * repeatSetScreenItemModel();
    static AbstractStaticItemModel * playSoundItemModel();
    static AbstractStaticItemModel * harpicItemModel();
    static AbstractStaticItemModel * gvarAdjustModeItemModel();
};
