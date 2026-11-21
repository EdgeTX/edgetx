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

struct KeyMap {
  uint8_t rtn = 255;       // RTN (KEY_EXIT)
  uint8_t pageNext = 255;  // Page> (KEY_PAGEDN)
  uint8_t pagePrev = 255;  // <Page (KEY_PAGEUP)
  uint8_t sys = 255;       // SYS (KEY_SYS)
  uint8_t tele = 255;      // Tele (KEY_TELE)
  uint8_t mdl = 255;       // MDL (KEY_MODEL)
  uint8_t enter = 255;     // Enter (KEY_ENTER)
};

struct LcdSpec {
  int width;
  int height;
  int depth;
};

AnalogMap getAnalogMapForRadio(const QString& radioName);
KeyMap getKeyMapForRadio(const QString& radioName);
LcdSpec getLcdSpecForRadio(const QString& radioName);