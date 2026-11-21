#pragma once

#include <QString>
#include <cstdint>

struct AnalogMap {
  uint8_t roll = 0;
  uint8_t pitch = 1;
  uint8_t throttle = 2;
  uint8_t yaw = 3;
  uint8_t s1 = 4;
  uint8_t s2 = 255;
  uint8_t ls = 255;
  uint8_t rs = 255;
  uint8_t pos6 = 255;
  uint8_t firstFuncSwitch = 255;  // Function switches SW1..SW6 (TX15)
};

struct LcdSpec {
  int width;
  int height;
  int depth;
};

AnalogMap getAnalogMapForRadio(const QString& radioName);
LcdSpec getLcdSpecForRadio(const QString& radioName);