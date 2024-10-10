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

#include <QtCore>

#include <string>
#include <vector>

using namespace Board;

class BoardJson
{
  Q_DECLARE_TR_FUNCTIONS(BoardJson)

  public:
    enum YamlLookupType {
      YLT_CONFIG,
      YLT_REF
    };

    struct InputDefn {
      Board::AnalogInputType type      = AIT_NONE;
      std::string tag                  = "";
      std::string name                 = "";
      std::string shortName            = "";
      Board::FlexType flexType         = FLEX_NONE;
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
      Board::SwitchType type           = Board::SWITCH_NOT_AVAILABLE;
      std::string tag                  = "";
      std::string name                 = "";
      int flags                        = 0;
      bool inverted                    = false;
      Board::SwitchType dflt           = Board::SWITCH_NOT_AVAILABLE;
      Display display;
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

    typedef std::vector<TrimDefn> TrimsTable;

    explicit BoardJson(Board::Type board, QString hwdefn);
    virtual ~BoardJson();

    Board::Type board() const { return m_board; }
    QString hwdefn() const { return m_hwdefn; }

    bool loadDefinition();

    const int getCapability(const Board::Capability capability) const;
    const int getInputsCalibrated() const;

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
    const bool isInputFlexPot(int index) const;
    const bool isInputFlexSwitch(int index) const;
    const bool isInputStick(int index) const;
    const bool isInputSwitch(int index) const;

    const Board::KeyInfo getKeyInfo(int index) const;
    const int getKeyIndex(const QString key) const;

    const int getSwitchIndex(const QString val, Board::LookupValueType lvt) const;
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
private:
    Board::Type m_board;
    QString m_hwdefn;

    InputsTable *m_inputs;
    SwitchesTable *m_switches;
    TrimsTable *m_trims;
    KeysTable *m_keys;

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

    static bool loadFile(Board::Type board, QString hwdefn, InputsTable * inputs, SwitchesTable * switches,
                         KeysTable * keys, TrimsTable * trims);
    static void afterLoadFixups(Board::Type board, InputsTable * inputs, SwitchesTable * switches,
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
