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
    struct InputDefn {
      Board::AnalogInputType type = AIT_NONE;
      std::string tag             = "";
      std::string name            = "";
      std::string shortName       = "";
      Board::FlexType flexType    = FLEX_NONE;
      bool inverted               = false;

      InputDefn() = default;
    };

    typedef std::vector<InputDefn> InputsTable;

    struct Display {
      unsigned int x = 0;
      unsigned int y = 0;
    };

    struct SwitchDefn {
      Board::SwitchType type = Board::SWITCH_NOT_AVAILABLE;
      std::string tag        = "";
      std::string name       = "";
      int flags              = 0;
      bool inverted          = false;
      Board::SwitchType dflt = Board::SWITCH_NOT_AVAILABLE;
      Display display;

      SwitchDefn() = default;
    };

    typedef std::vector<SwitchDefn> SwitchesTable;

    struct TrimDefn {
      std::string tag  = "";
      std::string name = "";

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

    const int getInputIndex(const QString tag) const;
    const Board::InputInfo getInputInfo(int index) const;
    const QString getInputName(int index) const;
    const int getInputPotIndex(int index);
    const int getInputSliderIndex(int index);
    const QString getInputTag(int index) const;
    const int getInputTagOffset(QString tag);
    const int getInputTypeOffset(Board::AnalogInputType type);

    const bool isInputAvailable(int index) const;
    const bool isInputCalibrated(int index) const;
    const bool isInputConfigurable(int index) const;
    const bool isInputIgnored(int index) const;
    const bool isInputPot(int index) const;
    const bool isInputStick(int index) const;

    const int getSwitchIndex(const QString tag) const;
    const Board::SwitchInfo getSwitchInfo(int index) const;
    const QString getSwitchName(int index) const;
    const QString getSwitchTag(int index) const;

private:
    Board::Type m_board;
    QString m_hwdefn;

    InputsTable *m_inputs;
    SwitchesTable *m_switches;
    TrimsTable *m_trims;

    unsigned int m_flexCnt;
    unsigned int m_funcSwitchesCnt;
    unsigned int m_gyroAxesCnt;
    unsigned int m_joystickAxesCnt;
    unsigned int m_potsCnt;
    unsigned int m_slidersCnt;
    unsigned int m_sticksCnt;
    unsigned int m_switchesCnt;
    unsigned int m_rtcbatCnt;
    unsigned int m_vbatCnt;

    static bool loadFile(Board::Type board, QString hwdefn, InputsTable * inputs, SwitchesTable * switches, TrimsTable * trims);
    // post loadFile fix ups
    static void addGyroAxes(Board::Type board, InputsTable * inputs);
    static std::string setStickName(int index);

    static int getInputsCalibrated(const InputsTable * inputs);

    static int getInputIndex(const InputsTable * inputs, QString tag);
    static Board::InputInfo getInputInfo(const InputsTable * inputs, int index);
    static QString getInputName(const InputsTable * inputs, int index);
    static QString getInputTag(const InputsTable * inputs, int index);
    static int getInputTagOffset(const InputsTable * inputs, QString tag);
    static int getInputTypeOffset(const InputsTable * inputs, Board::AnalogInputType type);

    static int getSwitchIndex(const SwitchesTable * switches, QString tag);
    static Board::SwitchInfo getSwitchInfo(const SwitchesTable * switches, int index);
    static QString getSwitchName(const SwitchesTable * switches, int index);
    static QString getSwitchTag(const SwitchesTable * switches, int index);

    static bool isInputAvailable(const InputDefn & defn);
    static bool isInputCalibrated(const InputDefn & defn);
    static bool isInputConfigurable(const InputDefn & defn);
    static bool isInputIgnored(const InputDefn & defn);

    static bool isFlex(const InputDefn & defn);
    static bool isGyroAxis(const InputDefn & defn);
    static bool isJoystickAxis(const InputDefn & defn);
    static bool isMultipos(const InputDefn & defn);
    static bool isPot(const InputDefn & defn);
    static bool isSlider(const InputDefn & defn);
    static bool isFlexSwitch(const InputDefn & defn);
    static bool isFuncSwitch(const SwitchDefn & defn);
    static bool isRTCBat(const InputDefn & defn);
    static bool isStick(const InputDefn & defn);
    static bool isSwitch(const SwitchDefn & defn);
    static bool isVBat(const InputDefn & defn);

    static unsigned int setFlexCount(const InputsTable * inputs);
    static unsigned int setFuncSwitchesCount(const SwitchesTable * switches);
    static unsigned int setGyroAxesCount(const InputsTable * inputs);
    static unsigned int setJoystickAxesCount(const InputsTable * inputs);
    static unsigned int setPotsCount(const InputsTable * inputs);
    static unsigned int setSlidersCount(const InputsTable * inputs);
    static unsigned int setSticksCount(const InputsTable * inputs);
    static unsigned int setSwitchesCount(const SwitchesTable * switches);
    static unsigned int setRTCBatCount(const InputsTable * inputs);
    static unsigned int setVBatCount(const InputsTable * inputs);
};
