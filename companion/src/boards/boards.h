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

constexpr char BDDEFNSDIR[] { ":/bddefs" };
constexpr char HWDEFNSDIR[] { ":/hwdefs" };

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

    struct Battery {
      int min   = 0;
      int max   = 0;
      int warn  = 0;
    };

    struct Contrast {
      int min = 0;
      int max = 0;
    };

    struct InternalModules {
      std::vector<int> supported  = {};
      int dflt                    = -1;
    };

    struct HardwareDefn {
      int has_audio_mute                = 0;
      int has_bling_leds                = 0;
      int has_ext_module_support        = 0;
      int has_int_module_support        = 0;
      int sport_max_baudrate            = 0;
      int surface                       = 0;
      std::string cpu                   = "";
      std::string cpu_type              = "";
      bool hasKeyLockCombo              = false;
      // these sourced from bddefn
      Battery battery;
      int backlightLevelMin             = 0;
      bool auxSerialMode                = false;
      bool aux2SerialMode               = false;
      bool bluetooth                    = false;
      bool externalAntenna              = false;
      bool hardwareAntennaSwitch        = false;
      bool imu                          = false;
      bool internalGPS                  = false;
      bool softwareSerialPower          = false;
      bool switchableJack               = false;
      std::string trainerModule         = "";  // cppm or sbus
      bool vcpSerialMode                = false;
      Contrast contrast;
      int maxVolume                     = 0;
      bool pwrButtonPress               = true;
      bool rotaryEncoderNavigation      = true;
      InternalModules internalModules;
      std::string defaultExternalModuleSize = ""; // std, small, both
      int fourCC                        = 0;
      int eepromSize                    = 0;
      int flashSize                     = 0;
    };

    typedef std::vector<TrimDefn> TrimsTable;

    explicit Boards(const Board::Type & id, const QString & hwdefn, const bool isSupported = true);
    explicit Boards(const Board::Type & id);
    virtual ~Boards() {}

    const Board::Type getId() const { return m_id; }
    const QString getManufacturer() const { return m_hardware.manufacturer.c_str(); }
    const QString getName() const { return m_hardware.name.c_str(); }
    const QString getHwDefn() const { return m_hwdefn; }

    void getBatteryRange(int & vmin, int & vmax, unsigned int & vwarn) const;

    int getCapability(const Capability capability) const;
    QString getCapabilityStr(const Capability capability) const;

    // inputs
    const int getInputIndex(const QString val, Board::LookupValueType lvt) const;
    const Board::InputInfo getInputInfo(int index) const;
    const QString getInputName(int index) const;
    const int getInputExtIndex(int index) const;
    const int getInputPotIndex(int index) const;
    const int getInputSliderIndex(int index) const;
    const QString getInputTag(int index) const;
    const int getInputTagOffset(QString tag) const;
    const int getInputThrottleIndex() const;
    const int getInputTypeOffset(Board::AnalogInputType type) const;
    const int getInputYamlIndex(const QString val, YamlLookupType ylt) const;
    const QString getInputYamlName(int index, YamlLookupType ylt) const;
    const int getInputsCalibrated() const;

    const bool isInputAvailable(int index) const;
    const bool isInputCalibrated(int index) const;
    const bool isInputConfigurable(int index) const;
    const bool isInputIgnored(int index) const;
    const bool isInputFlex(int index) const;
    const bool isInputFlexGyroAxis(int index) const;
    const bool isInputFlexJoystickAxis(int index) const;
    const bool isInputFlexPot(int index) const;
    const bool isInputFlexSwitch(int index) const;
    const bool isInputStick(int index) const;
    const bool isInputSwitch(int index) const;
    const bool isInputFlexPotMultipos(int index) const;
    const bool isInputFlexSlider(int index) const;
    const bool isInputRTCBat(int index) const;
    const bool isInputVBat(int index) const;

    // keys
    const Board::KeyInfo getKeyInfo(int index) const;
    const int getKeyIndex(const QString key) const;
    bool hasKeyLockCombo() const { return m_hasKeyLockCombo; }

    // switches
    const int getSwitchIndex(const QString val, Board::LookupValueType lvt) const;
    const int getCFSIndexForSwitch(int sw) const;
    const int getSwitchIndexForCFS(int customSwitchIdx) const;
    const int getSwitchIndexForCFSOffset(int offset) const;
    const int getCFSOffsetForCFSIndex(int index) const;
    const Board::SwitchInfo getSwitchInfo(int index) const;
    const QString getSwitchName(int index) const;
    const QString getSwitchTag(int index) const;
    const int getSwitchTagNum(int index) const;
    const int getSwitchTypeOffset(Board::SwitchType type) const;
    const int getSwitchYamlIndex(const QString val, YamlLookupType ylt) const;
    const QString getSwitchYamlName(int index, YamlLookupType ylt) const;

    const bool isSwitchConfigurable(int index) const;
    const bool isSwitchFlex(int index) const;
    const bool isSwitchFunc(int index) const;
    const bool isSwitchStd(int index) const;

    // trims
    const int getTrimIndex(const QString val, Board::LookupValueType lvt) const;
    const QString getTrimName(int index) const;
    const QString getTrimTag(int index) const;
    const int getTrimYamlIndex(const QString val, YamlLookupType ylt) const;
    const QString getTrimYamlName(int index, YamlLookupType ylt) const;

    // misc
    const int defaultInternalModule() const;
    const int defaultExternalModuleSize() const;
    const QList<int> supportedInternalModules() const;

    bool loadDefinitions();
    const QString radioModeString() const;
    const bool isLoaded() { return m_loaded; }

    // static
    static AbstractStaticItemModel * externalModuleSizeItemModel();
    static AbstractStaticItemModel * flexTypeItemModel();
    static AbstractStaticItemModel * intModuleTypeItemModel();
    static AbstractStaticItemModel * switchTypeItemModel();

    static int getFlashSize() { return getCurrentFirmwareBoard()->getCapability(Capability::FlashSize); }
    static QString getAxisName(int index);
    static int getNumericSuffix(const std::string str);

    // deprecated
    static std::string getLegacyAnalogMappedInputTag(const char * legacytag, const Board::Type & id = Board::BOARD_UNKNOWN);

  private:

    QString m_id;
    QString m_hwdefn;
    QString m_name;
    QString m_manufacturer;
    bool m_loaded;
    bool m_valid;

    InputsTable m_inputs;
    SwitchesTable m_switches;
    TrimsTable m_trims;
    KeysTable m_keys;
    DisplayDefn m_display;
    CustomSwitchesDefn m_cfs;
    HardwareDefn m_hardware;

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

    bool loadDefinition();
    bool loadDefinition(const QString & path);

    void loadADCInputs(QJsonObject::const_iterator & it);
    void loadInputs(QJsonObject::const_iterator & it);
    void loadDisplay(QJsonObject::const_iterator & it);
    void loadHardware(QJsonObject::const_iterator & it);
    void loadKeys(QJsonObject::const_iterator & it);
    void loadLEDS(QJsonObject::const_iterator & it);
    void loadSwitches(QJsonObject::const_iterator & it);
    void loadTrims(QJsonObject::const_iterator & it);

    void postLoadFixups();

    void setInputCounts();
    void setSwitchCounts();

    STRINGTAGMAPPINGFUNCS(trimSwitchesLookupTable, TrimSwitch);
    STRINGTAGMAPPINGFUNCS(rawSwitchTypesLookupTable, RawSwitchType);
    STRINGTAGMAPPINGFUNCS(rawSourceSpecialTypesLookupTable, RawSourceSpecialType);

    static QString externalModuleSizeToString(int value);
    static int externalModuleStringToSize(const QString & value);
    static QString flexTypeToString(int value);
    static QString switchTypeToString(int value);
};

// Helpers
Boards * getBoardForHwDefn(const QString & hwdefn);
Boards * getBoardForId(const Board::Type & id);
