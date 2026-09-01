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

#include "capability.h"
#include "datahelpers.h"
#include "helpers_json.h"

#include <QtCore>
#include <QObject>
#include <QString>

#include <string>
#include <vector>

class AbstractStaticItemModel;
class SemanticVersion;
class GeneralSettings;

// identiying names of static abstract item models
constexpr char AIM_BOARDS_POT_TYPE[]        {"boards.pottype"};
constexpr char AIM_BOARDS_SLIDER_TYPE[]     {"boards.slidertype"};
constexpr char AIM_BOARDS_SWITCH_TYPE[]     {"boards.switchtype"};
constexpr char AIM_BOARDS_MODULE_SIZE[]     {"boards.extmodulesize"};
constexpr char AIM_BOARDS_FLEX_TYPE[]       {"boards.flextype"};

// TODO merge into Boards class
namespace Board {

  typedef QString Type;

  constexpr char BOARD_UNKNOWN[] { "unknown" };

  enum PotType
  {
    POT_NONE,
    POT_WITH_DETENT,
    POT_MULTIPOS_SWITCH,
    POT_WITHOUT_DETENT,
    POT_SLIDER_WITH_DETENT,
    POT_TYPE_COUNT
  };

  enum SliderType
  {
    SLIDER_NONE,
    SLIDER_WITH_DETENT,
    SLIDER_TYPE_COUNT
  };

  enum SwitchType
  {
    SWITCH_NOT_AVAILABLE,
    SWITCH_TOGGLE,
    SWITCH_2POS,
    SWITCH_3POS,
    SWITCH_GLOBAL,
    SWITCH_FUNC,
    SWITCH_ADC,
    SWITCH_TYPE_COUNT
  };

  enum StickAxes {
    STICK_AXIS_LH = 0,
    STICK_AXIS_LV,
    STICK_AXIS_RV,
    STICK_AXIS_RH,
    STICK_AXIS_COUNT
  };

  enum TrimAxes {
    TRIM_AXIS_LH = 0,
    TRIM_AXIS_LV,
    TRIM_AXIS_RV,
    TRIM_AXIS_RH,
    TRIM_AXIS_T5,
    TRIM_AXIS_T6,
    TRIM_AXIS_T7,
    TRIM_AXIS_T8,
    TRIM_AXIS_COUNT
  };

  enum TrimSwitches
  {
    TRIM_SW_LH_DEC,
    TRIM_SW_LH_INC,
    TRIM_SW_LV_DEC,
    TRIM_SW_LV_INC,
    TRIM_SW_RV_DEC,
    TRIM_SW_RV_INC,
    TRIM_SW_RH_DEC,
    TRIM_SW_RH_INC,
    TRIM_SW_T5_DEC,
    TRIM_SW_T5_INC,
    TRIM_SW_T6_DEC,
    TRIM_SW_T6_INC,
    TRIM_SW_T7_DEC,
    TRIM_SW_T7_INC,
    TRIM_SW_T8_DEC,
    TRIM_SW_T8_INC,
    TRIM_SW_COUNT
  };

  enum StickAxesSurface {
    STICK_AXIS_SURFACE_RH = 0, // Steering
    STICK_AXIS_SURFACE_LV,     // Throttle
    STICK_AXIS_SURFACE_COUNT
  };

  enum TrimAxesSurface {
    TRIM_AXIS_SURFACE_RH = 0,
    TRIM_AXIS_SURFACE_LH, // Throttle axis vertical but its trim horizontal in lcd
    TRIM_AXIS_SURFACE_T3,
    TRIM_AXIS_SURFACE_T4,
    TRIM_AXIS_SURFACE_T5,
    TRIM_AXIS_SURFACE_T6,
    TRIM_AXIS_SURFACE_T7,
    TRIM_AXIS_SURFACE_T8,
    TRIM_AXIS_SURFACE_COUNT
  };

  enum TrimSwitchesSurface
  {
    TRIM_SW_SURFACE_RH_DEC,
    TRIM_SW_SURFACE_RH_INC,
    TRIM_SW_SURFACE_LH_DEC, // Throttle axis vertical but its trim horizontal in lcd
    TRIM_SW_SURFACE_LH_INC, // Throttle axis vertical but its trim horizontal in lcd
    TRIM_SW_SURFACE_T3_DEC,
    TRIM_SW_SURFACE_T3_INC,
    TRIM_SW_SURFACE_T4_DEC,
    TRIM_SW_SURFACE_T4_INC,
    TRIM_SW_SURFACE_T5_DEC,
    TRIM_SW_SURFACE_T5_INC,
    TRIM_SW_SURFACE_T6_DEC,
    TRIM_SW_SURFACE_T6_INC,
    TRIM_SW_SURFACE_T7_DEC,
    TRIM_SW_SURFACE_T7_INC,
    TRIM_SW_SURFACE_T8_DEC,
    TRIM_SW_SURFACE_T8_INC,
    TRIM_SW_SURFACE_COUNT
  };

    struct SwitchPosition {
    SwitchPosition(unsigned int index, unsigned int position):
      index(index),
      position(position)
      {
      }
      unsigned int index;
      unsigned int position;
  };

  enum SwitchTypeMasks {
    SwitchTypeFlagNone    = 1 << 1,
    SwitchTypeFlag2Pos    = 1 << 2,
    SwitchTypeFlag3Pos    = 1 << 3,
    SwitchTypeContextNone = SwitchTypeFlagNone,
    SwitchTypeContext2Pos = SwitchTypeContextNone | SwitchTypeFlag2Pos,
    SwitchTypeContext3Pos = SwitchTypeContext2Pos | SwitchTypeFlag3Pos
  };

  enum ExternalModuleSizes {
    EXTMODSIZE_NONE,
    EXTMODSIZE_STD,
    EXTMODSIZE_SMALL,
    EXTMODSIZE_BOTH,
    EXTMODSIZE_COUNT
  };

  enum AnalogInputType
  {
    AIT_NONE,
    AIT_STICK,
    AIT_FLEX,
    AIT_VBAT,
    AIT_RTC_BAT,
    AIT_LUX,
    AIT_SWITCH,
  };

  enum FlexType {
    FLEX_NONE = 0,
    FLEX_POT,
    FLEX_POT_CENTER,
    FLEX_SLIDER,
    FLEX_MULTIPOS,
    FLEX_AXIS_X,
    FLEX_AXIS_Y,
    FLEX_SWITCH,
    FLEX_TYPE_COUNT
  };

  enum FlexTypeMasks {
    FlexTypeFlagNotSwitch   = 1 << 1,
    FlexTypeFlagSwitch      = 1 << 2,
    FlexTypeContextNoSwitch = FlexTypeFlagNotSwitch,
    FlexTypeContextSwitch   = FlexTypeContextNoSwitch | FlexTypeFlagSwitch
  };

  enum LookupValueType {
    LVT_TAG = 0,
    LVT_NAME
  };

  struct InputInfo {
    InputInfo() :
      type(AIT_NONE),
      tag(""),
      name(""),
      shortName(""),
      flexType(FLEX_NONE),
      inverted(false)
    {}

    AnalogInputType type;
    std::string tag;
    std::string name;
    std::string label;
    std::string shortName;
    FlexType flexType;
    bool inverted;
  };

  struct SwitchInfo {
    SwitchInfo() :
      type(SWITCH_NOT_AVAILABLE),
      tag(""),
      name(""),
      dflt(SWITCH_NOT_AVAILABLE),
      inverted(false)
    {}

    SwitchType type;
    std::string tag;
    std::string name;
    SwitchType dflt;
    bool inverted;
  };

  struct KeyInfo {
    KeyInfo() :
      key(""),
      name(""),
      label("")
    {}

    std::string key;
    std::string name;
    std::string label;
  };

  struct TrimInfo {
    TrimInfo() :
      name("")
    {}

    std::string name;
  };
}

class Boards : public JsonBase
{
  Q_DECLARE_TR_FUNCTIONS(Boards)

  public:

    enum YamlLookupType {
      YLT_CONFIG,
      YLT_REF
    };

    struct InputDefn {
      Board::AnalogInputType type      = Board::AIT_NONE;
      std::string tag                  = "";
      std::string name                 = "";
      std::string shortName            = "";
      Board::FlexType flexType         = Board::FLEX_NONE;
      bool inverted                    = false;
      Board::LookupValueType cfgYaml   = Board::LVT_TAG;
      Board::LookupValueType refYaml   = Board::LVT_TAG;

      InputDefn() = default;
    };

    typedef std::vector<InputDefn> InputsTable;

    struct Display {
      unsigned int x = 0;
      unsigned int y = 0;
    };

    struct SwitchDefn {
      std::string tag                  = "";
      Board::SwitchType type           = Board::SWITCH_NOT_AVAILABLE;
      std::string name                 = "";
      int flags                        = 0;
      bool inverted                    = false;
      Board::SwitchType dflt           = Board::SWITCH_NOT_AVAILABLE;
      Display display;
      bool isCustomSwitch              = false;
      int customSwitchIdx              = -1;
      Board::LookupValueType cfgYaml   = Board::LVT_TAG;
      Board::LookupValueType refYaml   = Board::LVT_NAME;

      SwitchDefn() = default;
    };

    typedef std::vector<SwitchDefn> SwitchesTable;

    struct KeyDefn {
      std::string tag  = "";
      std::string name = "";
      std::string key = "";
      std::string label = "";
      Board::LookupValueType cfgYaml   = Board::LVT_TAG;
      Board::LookupValueType refYaml   = Board::LVT_NAME;

      KeyDefn() = default;
    };

    typedef std::vector<KeyDefn> KeysTable;

    struct TrimDefn {
      std::string tag                  = "";
      std::string name                 = "";
      Board::LookupValueType cfgYaml   = Board::LVT_TAG;
      Board::LookupValueType refYaml   = Board::LVT_NAME;

      TrimDefn() = default;
    };

    struct DisplayDefn {
      int w                            = 0;
      int h                            = 0;
      int phys_w                       = 0;
      int phys_h                       = 0;
      int depth                        = 0;
      int color                        = 0;
      int oled                         = 0;
      int backlight_color              = 0;
    };

    struct CustomSwitchesDefn {
      int rgb_led                      = 0;
      int groups                       = 0;
    };

    struct HardwareDefn {
      int has_audio_mute               = 0;
      int has_bling_leds               = 0;
      int has_ext_module_support       = 0;
      int has_int_module_support       = 0;
      int sport_max_baudrate           = 0;
      int surface                      = 0;
      std::string cpu                  = "";
      std::string cpu_type             = "";
    };

    struct Battery {
      int min;
      int max;
      int warn;
    };

    struct Contrast {
      int min;
      int max;
    };

    struct InternalModules {
      QList<int> supported;
      int dflt;
    };

    // values from bddefn not available from hwdefn
    struct BoardDefn {
      QString name;
      QString id;
      QString manufacturer;
      Battery battery;
      int backlightLevelMin;
      bool auxSerialMode;
      bool aux2SerialMode;
      bool bluetooth;
      bool externalAntenna;
      bool hardwareAntennaSwitch;
      bool imu;
      bool internalGPS;
      bool softwareSerialPower;
      bool switchableJack;
      bool trainerModuleCPPM;
      bool trainerModuleSBUS;
      bool vcpSerialMode;
      Contrast contrast;
      int maxVolume;
      bool pwrButtonPress;
      bool rotaryEncoderNavigation;
      InternalModules internalModules;
      int defaultExternalModuleSize;

      // TODO are these still relevant?
      int fourCC;
      int eepromSize;
      int flashSize;
    };

    typedef std::vector<TrimDefn> TrimsTable;

    explicit Boards(const Board::Type & board, const QString & hwdefn, const QString & bddefn);
    virtual ~Boards() {}

    const Board::Type id() const { return m_id; }
    const QString hwdefn() const { return m_hwdefn; }
    const QString dbdefn() const { return m_bddefn; }
    const QString name() const { return m_hwextra->name; }
    const QString manufacturer() const { return m_hwextra->manufacturer; }

    bool loadDefinition();

    Board::Type getBoardType() const { return m_id; }
    Board::Type boardType() const { return m_id; }

    const int getCapability(const Capability capability) const { return getCapability(m_id, capability); }
    const QString getCapabilityStr(const Capability capability) const { return getCapabilityStr(m_id, capability); }

    const bool isBoardCompatible(Board::Type other) const { return isBoardCompatible(m_id, other); }

    // TODO needed for what?
    const uint32_t getFourCC() const { return m_hwextra->fourCC; }
    const int getEEpromSize() const { return m_hwextra->eepromSize; }
    const int getFlashSize() const { return m_hwextra->flashSize; }
    // ======================
    const int getInputIndex(const QString val, Board::LookupValueType lvt) const;
    const Board::InputInfo getInputInfo(int index) const;
    const QString getInputName(int index) const;
    const int getInputExtIndex(int index);
    const int getInputPotIndex(int index);
    const int getInputSliderIndex(int index);
    const QString getInputTag(int index) const;
    const int getInputTagOffset(QString tag);
    const int getInputThrottleIndex();
    const int getInputTypeOffset(Board::AnalogInputType type);
    const int getInputYamlIndex(const QString val, YamlLookupType ylt) const;
    const QString getInputYamlName(int index, YamlLookupType ylt) const;

    const bool isInputAvailable(int index) const;
    const bool isInputCalibrated(int index) const;
    const bool isInputConfigurable(int index) const;
    const bool isInputIgnored(int index) const;
    const bool isInputFlexGyroAxis(int index) const;
    const bool isInputFlexJoystickAxis(int index) const;
    const bool isInputFlexPot(int index) const;
    const bool isInputFlexSwitch(int index) const;
    const bool isInputStick(int index) const;
    const bool isInputSwitch(int index) const;

    const Board::KeyInfo getKeyInfo(int index) const;
    const int getKeyIndex(const QString key) const;
    bool hasKeyLockCombo() const { return m_hasKeyLockCombo; }

    const int getSwitchIndex(const QString val, Board::LookupValueType lvt) const;
    const int getCFSIndexForSwitch(int sw) const;
    const int getSwitchIndexForCFS(int customSwitchIdx) const;
    const int getSwitchIndexForCFSOffset(int offset) const;
    const int getCFSOffsetForCFSIndex(int index) const;
    const Board::SwitchInfo getSwitchInfo(int index) const;
    const QString getSwitchName(int index) const;
    const QString getSwitchTag(int index) const;
    const int getSwitchTagNum(int index) const;
    const int getSwitchTypeOffset(Board::SwitchType type);
    const int getSwitchYamlIndex(const QString val, YamlLookupType ylt) const;
    const QString getSwitchYamlName(int index, YamlLookupType ylt) const;

    const int getTrimIndex(const QString val, Board::LookupValueType lvt) const;
    const QString getTrimName(int index) const;
    const QString getTrimTag(int index) const;
    const int getTrimYamlIndex(const QString val, YamlLookupType ylt) const;
    const QString getTrimYamlName(int index, YamlLookupType ylt) const;

    const bool isSwitchConfigurable(int index) const;
    const bool isSwitchFlex(int index) const;
    const bool isSwitchFunc(int index) const;

    // static
    static Boards* getBoard(const Board::Type & id);
    static int getCapability(const Board::Type & id, const Capability capability);
    static QString getCapabilityStr(const Board::Type & id, const Capability capability);
    static QString getAxisName(int index);
    static bool isBoardCompatible(Board::Type id1, Board::Type id2);
    static QString getBoardName(Board::Type id);
    static QString switchTypeToString(int value);
    static AbstractStaticItemModel * switchTypeItemModel();
    static AbstractStaticItemModel * intModuleTypeItemModel();
    static QList<int> getSupportedInternalModules(Board::Type id);
    static int getDefaultInternalModules(Board::Type id);
    static int getDefaultExternalModuleSize(Board::Type id);
    static void getBattRange(const Board::Type & id, int & vmin, int & vmax, unsigned int & vwarn);
    static QString externalModuleSizeToString(int value);
    static AbstractStaticItemModel * externalModuleSizeItemModel();

    //static BoardJson* getBoardJson(Board::Type id = Board::BOARD_UNKNOWN);

    static int getInputsCalibrated(Board::Type id = Board::BOARD_UNKNOWN);

    static Board::InputInfo getInputInfo(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getInputIndex(QString val, Board::LookupValueType lvt, Board::Type id = Board::BOARD_UNKNOWN);
    static QString getInputName(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getInputExtIndex(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getInputPotIndex(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getInputSliderIndex(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static QString getInputTag(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getInputTagOffset(QString tag, Board::Type id = Board::BOARD_UNKNOWN);
    static int getInputThrottleIndex(Board::Type id = Board::BOARD_UNKNOWN);
    static int getInputTypeOffset(Board::AnalogInputType type, Board::Type id = Board::BOARD_UNKNOWN);
    static int getInputYamlIndex(QString val, int ylt, Board::Type id = Board::BOARD_UNKNOWN);
    static QString getInputYamlName(int index, int ylt, Board::Type id = Board::BOARD_UNKNOWN);

    static Board::KeyInfo getKeyInfo(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getKeyIndex(QString key, Board::Type id = Board::BOARD_UNKNOWN);

    static Board::SwitchInfo getSwitchInfo(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getSwitchIndex(QString val, Board::LookupValueType lvt, Board::Type id = Board::BOARD_UNKNOWN);
    static int getCFSIndexForSwitch(int swIdx, Board::Type id = Board::BOARD_UNKNOWN);
    static int getSwitchIndexForCFS(int cfsIdx, Board::Type id = Board::BOARD_UNKNOWN);
    static int getSwitchIndexForCFSOffset(int offset, Board::Type id = Board::BOARD_UNKNOWN);
    static int getCFSOffsetForCFSIndex(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static QString getSwitchName(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static QString getSwitchTag(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getSwitchTagNum(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static int getSwitchTypeOffset(Board::SwitchType type, Board::Type id = Board::BOARD_UNKNOWN);
    static int getSwitchYamlIndex(QString val, int ylt, Board::Type id = Board::BOARD_UNKNOWN);
    static QString getSwitchYamlName(int index, int ylt, Board::Type id = Board::BOARD_UNKNOWN);

    static int getTrimYamlIndex(QString val, int ylt, Board::Type id = Board::BOARD_UNKNOWN);
    static QString getTrimYamlName(int index, int ylt, Board::Type id = Board::BOARD_UNKNOWN);

    static bool isInputAvailable(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static bool isInputCalibrated(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static bool isInputConfigurable(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static bool isInputGyroAxis(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static bool isInputIgnored(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static bool isInputPot(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static bool isInputStick(int index, Board::Type id = Board::BOARD_UNKNOWN);

    static bool isSwitchConfigurable(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static bool isSwitchFlex(int index, Board::Type id = Board::BOARD_UNKNOWN);
    static bool isSwitchFunc(int index, Board::Type id = Board::BOARD_UNKNOWN);

    static QString flexTypeToString(int value);
    static AbstractStaticItemModel * flexTypeItemModel();

    static std::string getLegacyAnalogMappedInputTag(const char * legacytag, Board::Type id = Board::BOARD_UNKNOWN);
    static QString getRadioModeString(Board::Type id = Board::BOARD_UNKNOWN);

    static bool isAir(Board::Type id = Board::BOARD_UNKNOWN);
    static bool isSurface(Board::Type id = Board::BOARD_UNKNOWN);

    // temporary until Boards refactored
    static Board::Type getBoardForHwDefn(const QString & hwdefn);
    //

    STRINGTAGMAPPINGFUNCS(trimSwitchesLookupTable, TrimSwitch);
    STRINGTAGMAPPINGFUNCS(rawSwitchTypesLookupTable, RawSwitchType);
    STRINGTAGMAPPINGFUNCS(rawSourceSpecialTypesLookupTable, RawSourceSpecialType);

  private:

    Board::Type m_id;
    QString m_hwdefn;
    QString m_bddefn;

    InputsTable *m_inputs;
    SwitchesTable *m_switches;
    TrimsTable *m_trims;
    KeysTable *m_keys;
    DisplayDefn *m_display;
    CustomSwitchesDefn *m_cfs;
    HardwareDefn *m_hardware;
    BoardDefn *m_hwextra;
    bool m_hasKeyLockCombo = false;

    struct InputCounts {
      unsigned int flexGyroAxes;
      unsigned int flexJoystickAxes;
      unsigned int flexPots;
      unsigned int flexSliders;
      unsigned int flexSwitches;
      unsigned int rtcbat;
      unsigned int sticks;
      unsigned int switches;
      unsigned int vbat;
    };

    InputCounts m_inputCnt;

    struct SwitchCounts {
      unsigned int std;
      unsigned int flex;
      unsigned int func;
    };

    SwitchCounts m_switchCnt;

    const StringTagMappingTable trimSwitchesLookupTable;
    const StringTagMappingTable rawSwitchTypesLookupTable;
    const StringTagMappingTable rawSourceSpecialTypesLookupTable;

    static bool loadFile(Board::Type id, QString hwdefn, InputsTable * inputs, SwitchesTable * switches,
                         KeysTable * keys, TrimsTable * trims, DisplayDefn * lcd, CustomSwitchesDefn * cfs,
                         HardwareDefn * hardware, BoardDefn * hwextra, bool & hasKeyLockCombo);
    static void afterLoadFixups(Board::Type id, InputsTable * inputs, SwitchesTable * switches,
                                KeysTable * keys, TrimsTable * trims);

    static int getInputsCalibrated(const InputsTable * inputs);

    static int getInputIndex(const InputsTable * inputs, QString val, Board::LookupValueType lvt);
    static Board::InputInfo getInputInfo(const InputsTable * inputs, int index);
    static QString getInputName(const InputsTable * inputs, int index);
    static QString getInputTag(const InputsTable * inputs, int index);
    static int getInputTagOffset(const InputsTable * inputs, QString tag);
    static int getInputTypeOffset(const InputsTable * inputs, Board::AnalogInputType type);

    static int getKeyIndex(const KeysTable * keys, QString key);
    static Board::KeyInfo getKeyInfo(const KeysTable * keys, int index);

    static int getSwitchIndex(const SwitchesTable * switches, QString val, Board::LookupValueType lvt);
    static int getCFSIndexForSwitch(const SwitchesTable * switches, int sw);
    static int getSwitchIndexForCFS(const SwitchesTable * switches, int customSwitchIdx);
    static int getSwitchIndexForCFSOffset(const SwitchesTable * switches, int offset);
    static int getCFSOffsetForCFSIndex(const SwitchesTable * switches, int index);
    static Board::SwitchInfo getSwitchInfo(const SwitchesTable * switches, int index);
    static QString getSwitchName(const SwitchesTable * switches, int index);
    static QString getSwitchTag(const SwitchesTable * switches, int index);
    static int getSwitchTagNum(const SwitchesTable * switches, int index);
    static int getSwitchTypeOffset(const SwitchesTable * switches, Board::SwitchType type);

    static int getTrimIndex(const TrimsTable * trims, QString val, Board::LookupValueType lvt);
    static QString getTrimName(const TrimsTable * trims, int index);
    static QString getTrimTag(const TrimsTable * trims, int index);

    static bool isInputAvailable(const InputDefn & defn);
    static bool isInputCalibrated(const InputDefn & defn);
    static bool isInputConfigurable(const InputDefn & defn);
    static bool isInputFlex(const InputDefn & defn);
    static bool isInputFlexGyroAxis(const InputDefn & defn);
    static bool isInputFlexJoystickAxis(const InputDefn & defn);
    static bool isInputFlexPot(const InputDefn & defn);
    static bool isInputFlexPotMultipos(const InputDefn & defn);
    static bool isInputFlexSlider(const InputDefn & defn);
    static bool isInputFlexSwitch(const InputDefn & defn);
    static bool isInputIgnored(const InputDefn & defn);
    static bool isInputRTCBat(const InputDefn & defn);
    static bool isInputStick(const InputDefn & defn);
    static bool isInputSwitch(const InputDefn & defn);
    static bool isInputVBat(const InputDefn & defn);

    static bool isSwitchStd(const SwitchDefn & defn);
    static bool isSwitchFlex(const SwitchDefn & defn);
    static bool isSwitchFunc(const SwitchDefn & defn);

    static void setInputCounts(const InputsTable * inputs, InputCounts & inputCounts);
    static void setSwitchCounts(const SwitchesTable * switches, SwitchCounts & switchCounts);

    static int getNumericSuffix(const std::string str);
};

// temporary aliases for transition period, use Boards class instead.
#define getBoardCapability(b__, c__)   Boards::getCapability(b__, c__)
