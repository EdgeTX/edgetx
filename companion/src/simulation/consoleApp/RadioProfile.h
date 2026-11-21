/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include <QString>
#include <cstdint>

namespace SimulatorCli
{

constexpr uint8_t INVALID_AXIS = 255;
constexpr uint8_t MAX_SWITCHES = 16;
constexpr uint8_t MAX_ANALOGS = 16;

struct AnalogMap {
  uint8_t roll = 0;
  uint8_t pitch = 1;
  uint8_t throttle = 2;
  uint8_t yaw = 3;
  uint8_t s1 = 4;
  uint8_t s2 = INVALID_AXIS;
  uint8_t ls = INVALID_AXIS;
  uint8_t rs = INVALID_AXIS;
  uint8_t pos6 = INVALID_AXIS;
  uint8_t firstFuncSwitch = INVALID_AXIS;  // Function switches SW1..SW6

  bool hasPos6() const { return pos6 != INVALID_AXIS; }
  bool hasFuncSwitches() const { return firstFuncSwitch != INVALID_AXIS; }
};

struct LcdSpec {
  int width = 128;
  int height = 64;
  int depth = 1;

  bool isColor() const { return depth == 16; }
};

struct RadioProfile {
  AnalogMap analogs;
  LcdSpec lcd;
  uint8_t numSwitches = 8;  // Default: SA..SH
};

AnalogMap getAnalogMapForRadio(const QString& radioName);
LcdSpec getLcdSpecForRadio(const QString& radioName);
RadioProfile getRadioProfile(const QString& radioName);

}  // namespace SimulatorCli