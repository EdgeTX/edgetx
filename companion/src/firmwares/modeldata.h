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

#include "constants.h"
#include "curvedata.h"
#include "customfunctiondata.h"
#include "gvardata.h"
#include "flightmodedata.h"
#include "heli_data.h"
#include "input_data.h"
#include "logicalswitchdata.h"
#include "mixdata.h"
#include "moduledata.h"
#include "output_data.h"
#include "sensordata.h"
#include "telem_data.h"
#include "timerdata.h"
#include "customisation_data.h"
#include "generalsettings.h"

#include <QtCore>

class GeneralSettings;
class RadioDataConversionState;
class AbstractStaticItemModel;

constexpr char AIM_MODELDATA_TRAINERMODE[]  {"modeldata.trainermode"};
constexpr char AIM_MODELDATA_FUNCSWITCHCONFIG[]  {"modeldata.funcswitchconfig"};
constexpr char AIM_MODELDATA_FUNCSWITCHSTART[]  {"modeldata.funcswitchstart"};
constexpr char AIM_MODELDATA_FUNCSWITCHGROUPSTARTSWITCH[] = {"modeldata.funcswitchgroupstartswitch"};
constexpr char AIM_MODELDATA_FUNCSWITCHGROUPS[] = {"modeldata.funcswitchgroups"};

class RSSIAlarmData {
  public:
    RSSIAlarmData() { clear(); }
    int warning;
    int critical;
    bool disabled;
    void clear() {
      this->warning = 45;
      this->critical = 42;
      this->disabled = false;
    }
};

class ScriptData {
  public:
    ScriptData() { clear(); }
    char    filename[10+1];
    char    name[10+1];
    int     inputs[CPN_MAX_SCRIPT_INPUTS];
    void clear() { memset(reinterpret_cast<void *>(this), 0, sizeof(ScriptData)); }
};

enum TrainerMode {
  TRAINER_MODE_OFF,
  TRAINER_MODE_FIRST = TRAINER_MODE_OFF,
  TRAINER_MODE_MASTER_JACK,
  TRAINER_MODE_SLAVE_JACK,
  TRAINER_MODE_MASTER_SBUS_EXTERNAL_MODULE,
  TRAINER_MODE_MASTER_CPPM_EXTERNAL_MODULE,
  TRAINER_MODE_MASTER_SERIAL,
  TRAINER_MODE_MASTER_BLUETOOTH,
  TRAINER_MODE_SLAVE_BLUETOOTH,
  TRAINER_MODE_MULTI,
  TRAINER_MODE_LAST = TRAINER_MODE_MULTI
};

#define MODEL_NAME_LEN 15
#define INPUT_NAME_LEN 4
#define CPN_MAX_BITMAP_LEN 14

#define CPN_USBJ_MAX_JOYSTICK_CHANNELS 26

class USBJoystickChData {
  public:
    USBJoystickChData() { clear(); }
    unsigned int mode;
    unsigned int inversion;
    unsigned int param;
    unsigned int btn_num;
    unsigned int switch_npos;
    void clear() { memset(reinterpret_cast<void *>(this), 0, sizeof(USBJoystickChData)); }
};

class ModelData {
  Q_DECLARE_TR_FUNCTIONS(ModelData)

  public:
    ModelData();
    ModelData(const ModelData & src);
    ModelData & operator = (const ModelData & src);

    void convert(RadioDataConversionState & cstate);

    ExpoData * insertInput(const int idx);
    void removeInput(const int idx, bool clearName = true);

    bool isInputValid(const unsigned int idx) const;
    bool hasExpos(uint8_t inputIdx) const;
    bool hasMixes(uint8_t output) const;

    QVector<const ExpoData *> expos(int input) const;
    QVector<const MixData *> mixes(int channel) const;

    char      semver[8 + 1];
    bool      used;
    char      name[MODEL_NAME_LEN + 1];
    char      filename[16+1];
    char      labels[100];
    int       modelIndex;      // Companion only, temporary index position managed by data model.
    bool      modelUpdated;    // Companion only, used to highlight if changed in models list

    TimerData timers[CPN_MAX_TIMERS];
    bool      noGlobalFunctions;
    bool      thrTrim;            // Enable Throttle Trim
    int       trimInc;            // Trim Increments
    unsigned int trimsDisplay;
    bool      disableThrottleWarning;
    bool      enableCustomThrottleWarning;
    int       customThrottleWarningPosition;
    unsigned int jitterFilter;       // Added in EdgeTx 2.7 (#870)

    unsigned int beepANACenter;      // 1<<0->A1.. 1<<6->A7

    bool      extendedLimits; // TODO xml
    bool      extendedTrims;
    bool      throttleReversed;
    bool      checklistInteractive;
    FlightModeData flightModeData[CPN_MAX_FLIGHT_MODES];
    MixData   mixData[CPN_MAX_MIXERS];
    LimitData limitData[CPN_MAX_CHNOUT];

    char      inputNames[CPN_MAX_INPUTS][INPUT_NAME_LEN+1];
    ExpoData  expoData[CPN_MAX_EXPOS];

    CurveData curves[CPN_MAX_CURVES];
    LogicalSwitchData logicalSw[CPN_MAX_LOGICAL_SWITCHES];
    CustomFunctionData customFn[CPN_MAX_SPECIAL_FUNCTIONS];
    SwashRingData swashRingData;
    unsigned int thrTraceSrc;
    uint64_t switchWarningStates;
    unsigned int switchWarningEnable;
    unsigned int thrTrimSwitch;
    unsigned int potsWarningMode;
    bool potsWarnEnabled[CPN_MAX_INPUTS];
    int potsWarnPosition[CPN_MAX_INPUTS];
    bool displayChecklist;

    GVarData gvarData[CPN_MAX_GVARS];
    MavlinkData mavlink;
    unsigned int telemetryProtocol;
    FrSkyData frsky;
    unsigned int  rssiSource;
    RSSIAlarmData rssiAlarms;
    bool showInstanceIds;

    char bitmap[CPN_MAX_BITMAP_LEN + 1];

    unsigned int trainerMode;  // TrainerMode

    ModuleData moduleData[CPN_MAX_MODULES + 1/*trainer*/];

    ScriptData scriptData[CPN_MAX_SCRIPTS];

    SensorData sensorData[CPN_MAX_SENSORS];

    unsigned int toplcdTimer;

    RadioLayout::CustomScreens customScreens;
    TopBarPersistentData topBarData;
    uint8_t topbarWidgetWidth[MAX_TOPBAR_ZONES];
    unsigned int view;

    char registrationId[8+1];
    unsigned int hatsMode;

    // Radio level tabs control (global settings)
    unsigned int radioThemesDisabled;
    unsigned int radioGFDisabled;
    unsigned int radioTrainerDisabled;
    // Model level tabs control (global setting)
    unsigned int modelHeliDisabled;
    unsigned int modelFMDisabled;
    unsigned int modelCurvesDisabled;
    unsigned int modelGVDisabled;
    unsigned int modelLSDisabled;
    unsigned int modelSFDisabled;
    unsigned int modelCustomScriptsDisabled;
    unsigned int modelTelemetryDisabled;

    enum FunctionSwitchConfig {
      FUNC_SWITCH_CONFIG_NONE,
      FUNC_SWITCH_CONFIG_FIRST = FUNC_SWITCH_CONFIG_NONE,
      FUNC_SWITCH_CONFIG_TOGGLE,
      FUNC_SWITCH_CONFIG_2POS,
      FUNC_SWITCH_CONFIG_LAST = FUNC_SWITCH_CONFIG_2POS
    };

    enum FunctionSwitchStart {
      FUNC_SWITCH_START_ON,
      FUNC_SWITCH_START_FIRST = FUNC_SWITCH_START_ON,
      FUNC_SWITCH_START_OFF,
      FUNC_SWITCH_START_PREVIOUS,
      FUNC_SWITCH_START_LAST = FUNC_SWITCH_START_PREVIOUS
    };

    // Function switches
    unsigned int functionSwitchConfig;
    unsigned int functionSwitchGroup;
    unsigned int functionSwitchStartConfig;
    unsigned int functionSwitchLogicalState;
    char functionSwitchNames[CPN_MAX_SWITCHES_FUNCTION][HARDWARE_NAME_LEN + 1];

    // Custom USB joytsick mapping
    unsigned int usbJoystickExtMode;
    unsigned int usbJoystickIfMode;
    unsigned int usbJoystickCircularCut;
    USBJoystickChData usbJoystickCh[CPN_USBJ_MAX_JOYSTICK_CHANNELS];

    void clear();
    bool isEmpty() const;
    void setDefaultInputs(const GeneralSettings & settings);
    void setDefaultMixes(const GeneralSettings & settings);
    void setDefaultFunctionSwitches(int functionSwitchCount);
    void setDefaultValues(unsigned int id, const GeneralSettings & settings);

    int getTrimValue(int phaseIdx, int trimIdx);
    void setTrimValue(int phaseIdx, int trimIdx, int value);

    bool isGVarLinked(int phaseIdx, int gvarIdx);
    bool isGVarLinkedCircular(int phaseIdx, int gvarIdx);
    int getGVarValue(int phaseIdx, int gvarIdx);
    float getGVarValuePrec(int phaseIdx, int gvarIdx);
    int getGVarFlightModeIndex(const int phaseIdx, const int gvarIdx);
    void setGVarFlightModeIndexToValue(const int phaseIdx, const int gvarIdx, const int useFmIdx);

    bool isREncLinked(int phaseIdx, int reIdx);
    bool isREncLinkedCircular(int phaseIdx, int reIdx);
    int getREncValue(int phaseIdx, int reIdx);
    int getREncFlightModeIndex(const int phaseIdx, const int reIdx);
    void setREncFlightModeIndexToValue(const int phaseIdx, const int reIdx, const int useFmIdx);

    ModelData removeGlobalVars();

    int linkedFlightModeIndexToValue(const int phaseIdx, const int useFmIdx, const int maxOwnValue);
    int linkedFlightModeValueToIndex(const int phaseIdx, const int val, const int maxOwnValue);

    void clearMixes();
    void sortMixes();
    void clearInputs();

    int getChannelsMax(bool forceExtendedLimits=false) const;

    bool isAvailable(const RawSwitch & swtch) const;
    bool isFunctionSwitchPositionAvailable(int index) const;
    bool isFunctionSwitchSourceAllowed(int index) const;

    enum ReferenceUpdateAction {
      REF_UPD_ACT_CLEAR,
      REF_UPD_ACT_SHIFT,
      REF_UPD_ACT_SWAP,
    };

    enum ReferenceUpdateType {
      REF_UPD_TYPE_CHANNEL,
      REF_UPD_TYPE_CURVE,
      REF_UPD_TYPE_FLIGHT_MODE,
      REF_UPD_TYPE_GLOBAL_VARIABLE,
      REF_UPD_TYPE_INPUT,
      REF_UPD_TYPE_LOGICAL_SWITCH,
      REF_UPD_TYPE_SCRIPT,
      REF_UPD_TYPE_SENSOR,
      REF_UPD_TYPE_TIMER,
    };

    struct UpdateReferenceParams
    {
      ReferenceUpdateType type;
      ReferenceUpdateAction action;
      int index1;
      int index2;
      int shift;

      UpdateReferenceParams() {}
      UpdateReferenceParams(ReferenceUpdateType t, ReferenceUpdateAction a, int i1, int i2 = 0, int s = 0) :
        type(t), action(a), index1(i1), index2(i2), shift(s) {}
    };

    int updateAllReferences(const ReferenceUpdateType type, const ReferenceUpdateAction action, const int index1, const int index2 = 0, const int shift = 0);
    bool isExpoParent(const int index);
    bool isExpoChild(const int index);
    bool hasExpoChildren(const int index);
    bool hasExpoSiblings(const int index);
    void removeMix(const int idx);
    QString thrTraceSrcToString() const;
    QString thrTraceSrcToString(const GeneralSettings * generalSettings, const int index) const;
    int thrTraceSrcCount() const;
    bool isThrTraceSrcAvailable(const GeneralSettings * generalSettings, const int index) const;

    void limitsClear(const int index);
    void limitsClearAll();
    void limitsDelete(const int index);
    void limitsGet(const int index, QByteArray & data);
    void limitsInsert(const int index);
    void limitsMove(const int index, const int offset);
    void limitsSet(const int index, const QByteArray & data);

    QString trainerModeToString() const;
    static QString trainerModeToString(const int value);
    bool isTrainerModeAvailable(const GeneralSettings & generalSettings, const Firmware * firmware, const int value);
    AbstractStaticItemModel * trainerModeItemModel(const GeneralSettings & generalSettings, const Firmware * firmware);
    unsigned int getFuncSwitchConfig(unsigned int index) const;
    void setFuncSwitchConfig(unsigned int index, unsigned int value);
    static QString funcSwitchConfigToString(unsigned int value);
    static AbstractStaticItemModel * funcSwitchConfigItemModel();
    static AbstractStaticItemModel * funcSwitchGroupStartSwitchModel(int switchcnt);
    static AbstractStaticItemModel * funcSwitchGroupsModel();

    unsigned int getFuncSwitchGroup(unsigned int index) const;
    void setFuncSwitchGroup(unsigned int index, unsigned int value);

    unsigned int getFuncSwitchAlwaysOnGroup(unsigned int group) const;
    unsigned int getFuncSwitchAlwaysOnGroupForSwitch(unsigned int index) const;
    void setFuncSwitchAlwaysOnGroup(unsigned int group, unsigned int value);
    void setGroupSwitchState(uint8_t group, int switchcnt);

    unsigned int getFuncSwitchStart(unsigned int index) const;
    void setFuncSwitchStart(unsigned int index, unsigned int value);
    int getFuncGroupSwitchCount(unsigned int group, int switchcnt) const;
    unsigned int getFuncGroupSwitchStart(unsigned int group, int switchcnt) const;
    void setFuncGroupSwitchStart(unsigned int group, unsigned int value, int switchcnt);
    static QString funcSwitchStartToString(unsigned int value);
    static AbstractStaticItemModel * funcSwitchStartItemModel();

    int getCustomScreensCount() const;

  protected:
    void removeGlobalVar(int & var);

  private:
    QVector<UpdateReferenceParams> *updRefList = nullptr;

    struct UpdateReferenceInfo
    {
      ReferenceUpdateType type;
      ReferenceUpdateAction action;
      int index1;
      int index2;
      int shift;
      int updcnt;
      int maxindex;
      int occurences;
      RawSourceType srcType;
      RawSwitchType swtchType;
    };
    UpdateReferenceInfo updRefInfo;

    int updateReference();
    void appendUpdateReferenceParams(const ReferenceUpdateType type, const ReferenceUpdateAction action, const int index1, const int index2 = 0, const int shift = 0);
    template <class R, typename T>
    void updateTypeIndexRef(R & curref, const T type, const int idxAdj = 0, const bool defClear = true, const int defType = 0, const int defIndex = 0);
    template <class R, typename T>
    void updateTypeValueRef(R & curref, const T type, const int idxAdj = 0, const bool defClear = true, const int defType = 0, const int defValue = 0);
    void updateAdjustRef(int & adj);
    void updateAssignFunc(CustomFunctionData * cfd);
    void updateCurveRef(CurveReference & crv);
    void updateDestCh(MixData * md);
    void updateLimitCurveRef(CurveReference & crv);
    void updateFlightModeFlags(unsigned int & flags);
    void updateTelemetryRef(int & idx);
    void updateTelemetryRef(unsigned int & idx);
    void updateModuleFailsafes(ModuleData * md);
    inline void updateSourceRef(RawSource & src) { updateTypeIndexRef<RawSource, RawSourceType>(src, updRefInfo.srcType, 1); }
    inline void updateSwitchRef(RawSwitch & swtch) { updateTypeIndexRef<RawSwitch, RawSwitchType>(swtch, updRefInfo.swtchType, 1); }
    inline void updateTimerMode(RawSwitch & swtch) { updateTypeIndexRef<RawSwitch, RawSwitchType>(swtch, updRefInfo.swtchType, 1, false, (int)SWITCH_TYPE_TIMER_MODE, 0); }
    inline void updateSourceIntRef(int & value)
    {
      RawSource src = RawSource(value);
      updateTypeIndexRef<RawSource, RawSourceType>(src, updRefInfo.srcType, 1);
      if (value != src.toValue())
        value = src.toValue();
    }
    inline void updateSwitchIntRef(int & value)
    {
      RawSwitch swtch = RawSwitch(value);
      updateTypeIndexRef<RawSwitch, RawSwitchType>(swtch, updRefInfo.swtchType, 1);
      if (value != swtch.toValue())
        value = swtch.toValue();
    }
    void updateResetParam(CustomFunctionData * cfd);
};
