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

#include "capabilities.h"
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

// TODO create a Board class with all these functions

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

    explicit Boards(const Board::Type & board, const QString & hwdefn);
    virtual ~Boards() {}

    Board::Type id() const { return m_boardType; }
    QString hwdefn() const { return m_hwdefn; }

    void setBoardType(const Board::Type & board);
    Board::Type getBoardType() const { return m_boardType; }
    Board::Type boardType() const { return m_boardType; }

    const uint32_t getFourCC() const { return getFourCC(m_boardType); }
    const int getEEpromSize() const { return getEEpromSize(m_boardType); }
    const int getFlashSize() const { return getFlashSize(m_boardType); }
    const int getCapability(Capability capability) const { return getCapability(m_boardType, capability); }
    const QString getCapabilityStr(Capability capability) const { return getCapabilityStr(m_boardType, capability); }
    const bool isBoardCompatible(Board::Type board2) const { return isBoardCompatible(m_boardType, board2); }

    static uint32_t getFourCC(Board::Type board);
    static int getEEpromSize(Board::Type board);
    static int getFlashSize(Board::Type board);
    static int getCapability(Board::Type board, Capability capability);
    static QString getCapabilityStr(Board::Type board, Capability capability);
    static QString getAxisName(int index);
    static bool isBoardCompatible(Board::Type board1, Board::Type board2);
    static QString getBoardName(Board::Type board);
    static QString switchTypeToString(int value);
    static AbstractStaticItemModel * switchTypeItemModel();
    static AbstractStaticItemModel * intModuleTypeItemModel();
    static QList<int> getSupportedInternalModules(Board::Type board);
    static int getDefaultInternalModules(Board::Type board);
    static int getDefaultExternalModuleSize(Board::Type board);
    static void getBattRange(Board::Type board, int& vmin, int& vmax, unsigned int& vwarn);
    static QString externalModuleSizeToString(int value);
    static AbstractStaticItemModel * externalModuleSizeItemModel();

    static BoardJson* getBoardJson(Board::Type board = Board::BOARD_UNKNOWN);

    static int getInputsCalibrated(Board::Type board = Board::BOARD_UNKNOWN);

    static Board::InputInfo getInputInfo(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getInputIndex(QString val, Board::LookupValueType lvt, Board::Type board = Board::BOARD_UNKNOWN);
    static QString getInputName(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getInputExtIndex(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getInputPotIndex(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getInputSliderIndex(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static QString getInputTag(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getInputTagOffset(QString tag, Board::Type board = Board::BOARD_UNKNOWN);
    static int getInputThrottleIndex(Board::Type board = Board::BOARD_UNKNOWN);
    static int getInputTypeOffset(Board::AnalogInputType type, Board::Type board = Board::BOARD_UNKNOWN);
    static int getInputYamlIndex(QString val, int ylt, Board::Type board = Board::BOARD_UNKNOWN);
    static QString getInputYamlName(int index, int ylt, Board::Type board = Board::BOARD_UNKNOWN);

    static Board::KeyInfo getKeyInfo(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getKeyIndex(QString key, Board::Type board = Board::BOARD_UNKNOWN);

    static Board::SwitchInfo getSwitchInfo(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getSwitchIndex(QString val, Board::LookupValueType lvt, Board::Type board = Board::BOARD_UNKNOWN);
    static int getCFSIndexForSwitch(int swIdx, Board::Type board = Board::BOARD_UNKNOWN);
    static int getSwitchIndexForCFS(int cfsIdx, Board::Type board = Board::BOARD_UNKNOWN);
    static int getSwitchIndexForCFSOffset(int offset, Board::Type board = Board::BOARD_UNKNOWN);
    static int getCFSOffsetForCFSIndex(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static QString getSwitchName(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static QString getSwitchTag(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getSwitchTagNum(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static int getSwitchTypeOffset(Board::SwitchType type, Board::Type board = Board::BOARD_UNKNOWN);
    static int getSwitchYamlIndex(QString val, int ylt, Board::Type board = Board::BOARD_UNKNOWN);
    static QString getSwitchYamlName(int index, int ylt, Board::Type board = Board::BOARD_UNKNOWN);

    static int getTrimYamlIndex(QString val, int ylt, Board::Type board = Board::BOARD_UNKNOWN);
    static QString getTrimYamlName(int index, int ylt, Board::Type board = Board::BOARD_UNKNOWN);

    STRINGTAGMAPPINGFUNCS(legacyTrimSourcesLookupTable, LegacyTrimSource);
    STRINGTAGMAPPINGFUNCS(trimSwitchesLookupTable, TrimSwitch);
    STRINGTAGMAPPINGFUNCS(rawSwitchTypesLookupTable, RawSwitchType);
    STRINGTAGMAPPINGFUNCS(rawSourceSpecialTypesLookupTable, RawSourceSpecialType);

    static bool isInputAvailable(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static bool isInputCalibrated(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static bool isInputConfigurable(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static bool isInputGyroAxis(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static bool isInputIgnored(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static bool isInputPot(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static bool isInputStick(int index, Board::Type board = Board::BOARD_UNKNOWN);

    static bool isSwitchConfigurable(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static bool isSwitchFlex(int index, Board::Type board = Board::BOARD_UNKNOWN);
    static bool isSwitchFunc(int index, Board::Type board = Board::BOARD_UNKNOWN);

    static QString flexTypeToString(int value);
    static AbstractStaticItemModel * flexTypeItemModel();

    static std::string getLegacyAnalogMappedInputTag(const char * legacytag, Board::Type board = Board::BOARD_UNKNOWN);
    static QString getRadioModeString(Board::Type board = Board::BOARD_UNKNOWN);
    // use capabilities
    static bool isAir(Board::Type board = Board::BOARD_UNKNOWN);
    static bool isSurface(Board::Type board = Board::BOARD_UNKNOWN);

    // temporary until Boards refactored
    static Board::Type getBoardForHwDefn(const QString & hwdefn);
    //

  private:

    Board::Type m_boardType;
    QString m_hwdefn;

    const StringTagMappingTable legacyTrimSourcesLookupTable;
    const StringTagMappingTable trimSwitchesLookupTable;
    const StringTagMappingTable rawSwitchTypesLookupTable;
    const StringTagMappingTable rawSourceSpecialTypesLookupTable;

    static StringTagMappingTable getLegacyAnalogsLookupTable(Board::Type board = Board::BOARD_UNKNOWN);
};

// temporary aliases for transition period, use Boards class instead.
#define getBoardCapability(b__, c__)   Boards::getCapability(b__, c__)
