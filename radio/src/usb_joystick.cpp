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

#include "usb_joystick.h"

#include <math.h>

#include "hal/usb_driver.h"
#include "edgetx.h"

#include <cmath>

#define MAX_HID_REPORTDESC 160
#define MAX_HID_REPORT 80

uint8_t* _hidReportDesc = nullptr;
uint8_t _hidReportDescSize = 0;
uint8_t* _hidReport = nullptr;
uint8_t _hidReportSize = 0;

struct _usbJSData {
  uint16_t _usbLastChannelOutput[USBJ_MAX_JOYSTICK_CHANNELS];
  uint8_t _usbChannelTimerActive[USBJ_MAX_JOYSTICK_CHANNELS];
  uint8_t _usbChannelTimer[USBJ_MAX_JOYSTICK_CHANNELS];
  uint8_t _usbJoystickButtonChannels[USBJ_MAX_JOYSTICK_CHANNELS];
  uint8_t _usbJoystickAxisChannels[USBJ_MAX_JOYSTICK_CHANNELS];
};

static struct _usbJSData* _usbJS = nullptr;

static uint8_t _buttonState[(USBJ_BUTTON_SIZE + 7) >> 3] = {};

static uint8_t _usbJoystickIfMode = 0;
static uint8_t _usbJoystickCircularCut = 0;

static uint8_t _usbJoystickButtonCount = 0;
static uint8_t _usbJoystickAxisCount = 0;

static uint8_t _usbJoystickAxisPairs[2][2] = {};
static int8_t _usbJoystickDpadChannel = -1;

// clang-format off
/*
  This USB HID endpoint report description defines a device with:
    * 24 digital buttons
    * 8 analog axes with 11bit resolution
    * remaining battery capacity

  Report packet described as C struct is:

  struct {
    uint8_t buttons1; //bit 0 - button 1, bit 1 - button 2, ..., mapped to channels 9-16, on if channel > 0
    uint8_t buttons2; // mapped to channels 17-24, on if channel > 0
    uint8_t buttons3; // mapped to channels 25-32, on if channel > 0
    uint16_t X;  //analog value, mapped to channel 1
    uint16_t Y;  //analog value, mapped to channel 2
    uint16_t Z;  //analog value, mapped to channel 3
    uint16_t Rx; //analog value, mapped to channel 4
    uint16_t Ry  //analog value, mapped to channel 5
    uint16_t Rz; //analog value, mapped to channel 6
    uint16_t S1; //analog value, mapped to channel 7
    uint16_t S2; //analog value, mapped to channel 8
    uint8_t system; // remaining battery capacity
  }
*/
static const uint8_t HID_JOYSTICK_ReportDesc[] =
{
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    //     USAGE (Game Pad)
    0xa1, 0x01,                    //     COLLECTION (Application)
    0x09, 0x01,                    //       USAGE (Pointer)
    0xa1, 0x00,                    //       COLLECTION (Physical)
    0x05, 0x09,                    //         USAGE_PAGE (Button)
    0x75, 0x01,                    //         REPORT_SIZE (1)
    0x14,                          //         LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //         LOGICAL_MAXIMUM (1)
    0x19, 0x01,                    //         USAGE_MINIMUM (Button 1)
    0x29, 0x18,                    //         USAGE_MAXIMUM (Button 24)
    0x95, 0x18,                    //         REPORT_COUNT (24)
    0x81, 0x02,                    //         INPUT (Data,Var,Abs)
    0x05, 0x01,                    //         USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //         USAGE (X)
    0x09, 0x31,                    //         USAGE (Y)
    0x09, 0x32,                    //         USAGE (Z)
    0x09, 0x33,                    //         USAGE (Rx)
    0x09, 0x34,                    //         USAGE (Ry)
    0x09, 0x35,                    //         USAGE (Rz)
    0x09, 0x36,                    //         USAGE (Slider)
    0x09, 0x37,                    //         USAGE (Dial)
    0x14,                          //         LOGICAL_MINIMUM (0)
    0x26, 0x00, 0x08,              //         LOGICAL_MAXIMUM (2048)
    0x75, 0x10,                    //         REPORT_SIZE (16)
    0x95, 0x08,                    //         REPORT_COUNT (8)
    0x81, 0x02,                    //         INPUT (Data,Var,Abs)
    0xc0,                          //       END_COLLECTION
    0x05, 0x06,                    //       USAGE_PAGE (Generic Dev Ctrls)
    0x09, 0x20,                    //       USAGE (Battery Strength)
    0x14,                          //       LOGICAL_MINIMUM (0)
    0x25, 0x64,                    //       LOGICAL_MAXIMUM (100)
    0x75, 0x08,                    //       REPORT_SIZE (8)
    0x95, 0x01,                    //       REPORT_COUNT (1)
    0x81, 0x02,                    //       INPUT (Data,Var,Abs)
    0xc0                           //     END_COLLECTION
};
// clang-format on

// clang-format off
/*
  This partial HID description defines an Android compliant dpad / hat switch

  https://source.android.com/docs/compatibility/14/android-14-cdd#726_game_controller_support

  In addtion the D-Pad keys are mapped for applications using custom HID raw
  processing.
*/
static const uint8_t HID_JOYSTICK_DpadDesc[] =
{
  0x14,             // LOGICAL_MINIMUM    0
  0x25, 0x07,       // LOGICAL_MAXIMUM    7
  0x34,             // PHYSICAL_MINIMUM   0
  0x46, 0x3B, 0x01, // PHYSICAL_MAXIMUM   315
  0x75, 0x04,       // REPORT_SIZE        4 Number of bits per field
  0x95, 0x01,       // REPORT_COUNT       1 Number of fields
  0x05, 0x01,       // USAGE_PAGE         Generic Desktop Page
  0x09, 0x39,       // USAGE              Hat switch (Dynamic Value)
  0x65, 0x14,       // UNIT               Rotation in degrees [1° units] (4=System=English Rotation, 1=Rotation=Degrees)
  0x81, 0x42,       // INPUT              (1 field x 4 bits) 1=Variable 1=Null
  0x19, 0x90,       // USAGE_MINIMUM      0x00010090 D-pad Up
  0x29, 0x93,       // USAGE_MAXIMUM      0x00010093 D-pad Left
  0x25, 0x01,       // LOGICAL_MAXIMUM    0x01 (1)
  0x44,             // PHYSICAL_MAXIMUM   (0)
  0x64,             // UNIT               No unit (0=None)
  0x75, 0x01,       // REPORT_SIZE        0x01 (1) Number of bits per field
  0x95, 0x04,       // REPORT_COUNT       0x04 (4) Number of fields
  0x81, 0x02,       // INPUT              0x00000002 (4 fields x 1 bit) 0=Data 1=Variable 0=Absolute 0=NoWrap 0=Linear 0=PrefState 0=NoNull 0=NonVolatile 0=Bitmap
};
// clang-format on

static bool isUniqueAxisType(int type) { return type < 6; }
static bool isUniqueSimType(int type) { return true; }

int usbJoystickExtMode() { return g_model.usbJoystickExtMode; }

int usbJoystickActive()
{
  return (usbPlugged() && getSelectedUsbMode() == USB_JOYSTICK_MODE);
}

int usbJoystickSettingsChanged()
{
  static uint32_t settingsChecksum = 0;

  if (!usbJoystickActive()) return false;

  if ((_usbJoystickIfMode != g_model.usbJoystickIfMode) ||
      (_usbJoystickCircularCut != g_model.usbJoystickCircularCut))
    return true;

  uint32_t oldChecksum = settingsChecksum;
  settingsChecksum =
      hash((uint8_t*)&g_model.usbJoystickCh, sizeof(g_model.usbJoystickCh));

  return oldChecksum != settingsChecksum;
}

int setupUSBJoystick()
{
  static const uint8_t axisTypeCodes[USBJOYS_AXIS_LAST + 1] = {
      0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38};
  static const uint8_t simTypeCodes[USBJOYS_SIM_LAST + 1] = {
      0xb0, 0xb8, 0xba, 0xbb, 0xc4, 0xc5, 0xc8, 0x00};

  if (_hidReportDesc == nullptr) {
    _hidReportDesc = (uint8_t*)malloc(MAX_HID_REPORTDESC);
    if (_hidReportDesc == nullptr) return false;
  }

  if (_hidReport == nullptr) {
    _hidReport = (uint8_t*)malloc(MAX_HID_REPORT);
    if (_hidReport == nullptr) return false;
  }

  // properties to detect change
  uint8_t oldHIDReportDescSize = _hidReportDescSize;
  uint32_t oldChecksum = hash(_hidReportDesc, _hidReportDescSize);

  // Init data
  memset(_hidReportDesc, 0, MAX_HID_REPORTDESC);
  _hidReportDescSize = 0;
  memset(_hidReport, 0, MAX_HID_REPORT);
  _hidReportSize = 0;

  if (!usbJoystickExtMode()) {
    // Classic USB Joystick report description
    memcpy(_hidReportDesc, HID_JOYSTICK_ReportDesc,
           sizeof(HID_JOYSTICK_ReportDesc));
    _hidReportDescSize = sizeof(HID_JOYSTICK_ReportDesc);
    _hidReportSize = 20;

  } else {
    // Init data only required for advanced mode
    if (_usbJS == nullptr) {
      _usbJS = (struct _usbJSData*)malloc(sizeof(struct _usbJSData));
      if (_usbJS == nullptr) return false;
    }

    memset(_buttonState, 0, ((USBJ_BUTTON_SIZE + 7) >> 3));
    memset(_usbJS, 0, sizeof(struct _usbJSData));
    memset(_usbJS->_usbLastChannelOutput, 0xff,
           sizeof(_usbJS->_usbLastChannelOutput));
    memset(_usbJoystickAxisPairs, 0xff, sizeof(_usbJoystickAxisPairs));

    // copy the model snapshot
    _usbJoystickIfMode = g_model.usbJoystickIfMode;
    _usbJoystickCircularCut = g_model.usbJoystickCircularCut;

    // prepeare for circularCutoutValue
    for (uint8_t i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
      uint8_t mode = g_model.usbJoystickCh[i].mode;

      if (mode == USBJOYS_CH_AXIS) {
        uint8_t submode = g_model.usbJoystickCh[i].param;

        // save axis pairs - not pretty but "if" is even more unreadeable
        switch (submode) {
          case USBJOYS_AXIS_X:
            switch (_usbJoystickCircularCut) {
              case USBJOYS_CC_XYZrX:
              case USBJOYS_CC_XYrXrY:
              case USBJOYS_CC_XYZrZ:
                _usbJoystickAxisPairs[0][0] = i;
                break;
              default:
                break;
            }
            break;
          case USBJOYS_AXIS_Y:
            switch (_usbJoystickCircularCut) {
              case USBJOYS_CC_XYZrX:
              case USBJOYS_CC_XYrXrY:
              case USBJOYS_CC_XYZrZ:
                _usbJoystickAxisPairs[0][1] = i;
                break;
              default:
                break;
            }
            break;
          case USBJOYS_AXIS_Z:
            switch (_usbJoystickCircularCut) {
              case USBJOYS_CC_XYZrX:
              case USBJOYS_CC_XYZrZ:
                _usbJoystickAxisPairs[1][0] = i;
                break;
              default:
                break;
            }
            break;
          case USBJOYS_AXIS_RX:
            switch (_usbJoystickCircularCut) {
              case USBJOYS_CC_XYZrX:
                _usbJoystickAxisPairs[1][1] = i;
                break;
              case USBJOYS_CC_XYrXrY:
                _usbJoystickAxisPairs[1][0] = i;
                break;
              default:
                break;
            }
            break;
          case USBJOYS_AXIS_RY:
            switch (_usbJoystickCircularCut) {
              case USBJOYS_CC_XYrXrY:
                _usbJoystickAxisPairs[1][1] = i;
                break;
              default:
                break;
            }
            break;
          case USBJOYS_AXIS_RZ:
            switch (_usbJoystickCircularCut) {
              case USBJOYS_CC_XYZrZ:
                _usbJoystickAxisPairs[1][1] = i;
                break;
              default:
                break;
            }
            break;
          default:
            break;
        }
      }
    }

    // start with classic descriptor
    memcpy(_hidReportDesc, HID_JOYSTICK_ReportDesc, 6);
    _hidReportDescSize = 6;
    uint8_t pageCurrent = 0x01;

    // HID support for game buttons is tricky :
    // a) some applications simply count buttons and ignore button IDs
    // b) in addition to a) some reverse the HID descriptor order
    // c) yet others correctly use the button IDs
    //
    // USAGE_MINIMUM / USAGE_MAXIMUM might result in some ghost buttons
    // without mixer input but everyone seems to process that consistently

    _usbJoystickButtonCount = 0;
    int8_t buttonMaxId = -1;

    // HID support for axis is tricky :
    // a) some applications simply count axis
    // b) in addition to a) some reverse the HID descriptor order
    // c) some applications ignore axis with the same IDs
    // d) some applications first process plain axis and then sim axis
    // e) some combine a), b), c) and d) in various ways
    // f) yet others correctly use the axis IDs
    //
    // advanced users know what they need
    // -> don't try to order axis
    // -> encode all axis (plain and sim) in one block
    // -> Dpad / hat switch gets special handling any way

    _usbJoystickAxisCount = 0;
    _usbJoystickDpadChannel = -1;

    for (uint8_t channelIx = 0; channelIx < USBJ_MAX_JOYSTICK_CHANNELS;
         channelIx++) {
      int8_t mode = g_model.usbJoystickCh[channelIx].mode;
      uint8_t typeIx = g_model.usbJoystickCh[channelIx].param;
      uint8_t pageTarget = 0;
      uint8_t usage;

      if (mode == USBJOYS_CH_BUTTON) {
        uint8_t btn_num = g_model.usbJoystickCh[channelIx].btn_num;
        uint8_t bits = g_model.usbJoystickCh[channelIx].switch_npos;
        uint8_t id = btn_num + bits + 1;

        if (id > buttonMaxId) {
          buttonMaxId = id;
        }
        _usbJS->_usbJoystickButtonChannels[_usbJoystickButtonCount++] =
            channelIx;
      } else if (mode == USBJOYS_CH_AXIS && typeIx <= USBJOYS_AXIS_LAST) {
        usage = axisTypeCodes[typeIx];
        pageTarget = 0x01;  // Generic Desktop Page (0x01)
      } else if (mode == USBJOYS_CH_SIM && typeIx <= USBJOYS_SIM_LAST) {
        usage = simTypeCodes[typeIx];
        if (usage == 0x00) {
          // it is the special dpad / hat switch
          _usbJoystickDpadChannel = channelIx;
        } else {
          pageTarget = 0x02;  // USAGE_PAGE Simulation Controls Page (0x02)
        }
      }

      if (pageTarget) {
        if (_usbJoystickAxisCount < 1) {
          // USAGE (Pointer)
          _hidReportDesc[_hidReportDescSize++] = 0x09;
          _hidReportDesc[_hidReportDescSize++] = 0x01;

          // COLLECTION (Physical)
          _hidReportDesc[_hidReportDescSize++] = 0xa1;
          _hidReportDesc[_hidReportDescSize++] = 0x00;
        }

        if (pageCurrent != pageTarget) {
          // USAGE_PAGE
          _hidReportDesc[_hidReportDescSize++] = 0x05;
          _hidReportDesc[_hidReportDescSize++] = pageTarget;
          pageCurrent = pageTarget;
        }

        // USAGE
        _hidReportDesc[_hidReportDescSize++] = 0x09;
        _hidReportDesc[_hidReportDescSize++] = usage;

        _usbJS->_usbJoystickAxisChannels[_usbJoystickAxisCount++] = channelIx;
      }
    }

    if (_usbJoystickAxisCount) {
      // report count and input - DO end collection
      memcpy(_hidReportDesc + _hidReportDescSize, HID_JOYSTICK_ReportDesc + 43,
             11);
      _hidReportDesc[_hidReportDescSize + 7] = _usbJoystickAxisCount;
      _hidReportDescSize += 11;

      _hidReportSize += _usbJoystickAxisCount * 2;
    }

    //
    // encode buttons
    //
    uint8_t joystickType;

    switch (_usbJoystickIfMode) {
      case USBJOYS_GAMEPAD:
        // 0x00010005 Game Pad (Application Collection)
        joystickType = 0x05;

        // USB HID Usage Tables 1.21 page 33 : at least 4 buttons
        buttonMaxId = buttonMaxId < 4 ? 4 : buttonMaxId;
        break;

      case USBJOYS_MULTIAXIS:
        // 0x00010008 Multi-axis Controller (Application Collection)
        joystickType = 0x08;
        break;

      default:
        // Joystick (Application Collection)
        joystickType = 0x04;

        // USB HID Usage Tables 1.21 page 33 : at least 2 buttons
        buttonMaxId = buttonMaxId < 2 ? 2 : buttonMaxId;
        break;
    }

    // set Apllication Collection's usage
    _hidReportDesc[3] = joystickType;

    if (buttonMaxId) {
      memcpy(_hidReportDesc + _hidReportDescSize, HID_JOYSTICK_ReportDesc + 10,
             15);
      _hidReportDesc[_hidReportDescSize + 10] = buttonMaxId;
      _hidReportDesc[_hidReportDescSize + 12] = buttonMaxId;
      _hidReportDescSize += 15;
    }

    // pad remaining space
    if (buttonMaxId < USBJ_BUTTON_SIZE) {
      _hidReportDesc[_hidReportDescSize++] = 0x75;
      _hidReportDesc[_hidReportDescSize++] = USBJ_BUTTON_SIZE - buttonMaxId;
      _hidReportDesc[_hidReportDescSize++] = 0x95;
      _hidReportDesc[_hidReportDescSize++] = 0x01;
      _hidReportDesc[_hidReportDescSize++] = 0x81;
      _hidReportDesc[_hidReportDescSize++] = 0x03;
    }
    _hidReportSize += ((USBJ_BUTTON_SIZE + 7) / 8);

    // battery status
    memcpy(_hidReportDesc + _hidReportDescSize, HID_JOYSTICK_ReportDesc + 54,
           13);
    _hidReportDescSize += 13;
    _hidReportSize += 1;

    // special dpad / hat switch
    if (0 <= _usbJoystickDpadChannel) {
      memcpy(_hidReportDesc + _hidReportDescSize, HID_JOYSTICK_DpadDesc,
             sizeof(HID_JOYSTICK_DpadDesc));
      _hidReportDescSize += sizeof(HID_JOYSTICK_DpadDesc);
      _hidReportSize += 1;
    }

    // END_COLLECTION
    _hidReportDesc[_hidReportDescSize++] = 0xc0;

    // end of report desc
  }

  // compare with the old description
  if (_hidReportDescSize != oldHIDReportDescSize) return true;
  uint32_t newChecksum = hash(_hidReportDesc, _hidReportDescSize);
  return oldChecksum != newChecksum;
}

extern "C" struct usbReport_t usbReportDesc()
{
  usbReport_t res = {_hidReportDesc, _hidReportDescSize};
  return res;
}

static void setBatteryBits(int hid_pos)
{
  // vBatMin / vBatMax are encoded with offsets 90 / 120
  uint8_t percent = limit<uint8_t>(
      0,
      divRoundClosest((g_vbat100mV - g_eeGeneral.vBatMin - 90) * 100,
                      g_eeGeneral.vBatMax - g_eeGeneral.vBatMin + 30),
      100);

  _hidReport[hid_pos] = percent;
}

static void setDpadBits(int hid_pos, int channelIx)
{
  // Android's POV encoding:
  //  0 = North
  //  1 = North-East
  //  2 = East
  //  3 = South-East
  //  4 = South
  //  5 = South-West
  //  6 = West
  //  7 = North-West
  //  8-15 = no direction

  int16_t value = channelOutputs[channelIx] + 1024;

  // a span of 120 results in overflow at the top range
  // a span of 121 results in a too small top range
  // -> use 120 and slightly enlarge top and button ranges

  if (value > 2039) {
    value = 7;
  } else {
    value = ((value - 3) / 120) % 9;
  }

  // add explicit D-Pad keys for
  // applications using HID raw processing
  const uint8_t NORTH = 0x10;
  const uint8_t SOUTH = 0x20;
  const uint8_t EAST = 0x40;
  const uint8_t WEST = 0x80;

  // clang-format off
  const uint8_t key_lookup[] = {
    0 | NORTH,
    1 | NORTH | EAST,
    2 | EAST,
    3 | SOUTH | EAST,
    4 | SOUTH,
    5 | SOUTH | WEST,
    6 | WEST,
    7 | NORTH | WEST,
    8
  };
  // clang-format on

  _hidReport[hid_pos] = key_lookup[value & 0x0F];
}

void usbClassicStateUpdate()
{
  if (_hidReport == nullptr) return;

  memset(_hidReport, 0, MAX_HID_REPORT);

  // buttons
  _hidReport[0] = 0;
  _hidReport[1] = 0;
  _hidReport[2] = 0;
  for (int i = 0; i < 8; ++i) {
    if (channelOutputs[i + 8] > 0) {
      _hidReport[0] |= (1 << i);
    }
    if (channelOutputs[i + 16] > 0) {
      _hidReport[1] |= (1 << i);
    }
    if (channelOutputs[i + 24] > 0) {
      _hidReport[2] |= (1 << i);
    }
  }

  // analog values
  // uint8_t * p = HID_Buffer + 1;
  for (int i = 0; i < 8; ++i) {
    int16_t value = limit<int16_t>(0, channelOutputs[i] + 1024, 2048);

    _hidReport[i * 2 + 3] = static_cast<uint8_t>(value & 0xFF);
    _hidReport[i * 2 + 4] = static_cast<uint8_t>(value >> 8);
  }

  // battery values
  setBatteryBits(8 * 2 + 3);
}

static void setBtnBits(uint8_t ix, uint8_t value, uint8_t size)
{
  for (uint8_t i = 0; i < size; i++) {
    uint8_t bitix = ix + i;
    if (bitix >= USBJ_BUTTON_SIZE) break;

    if (value & (1 << i)) {
      _buttonState[(bitix / 8)] |= 1 << (bitix % 8);
    } else {
      _buttonState[(bitix / 8)] &= ~(1 << (bitix % 8));
    }
  }
}

static void toggleBtnBit(uint8_t bitix)
{
  if (bitix >= USBJ_BUTTON_SIZE) return;
  _buttonState[(bitix / 8)] ^= 1 << (bitix % 8);
}

#define g_usbTmr10ms (*(uint8_t*)&g_tmr10ms)
#define BTNPUSH_TIME 20 /* x10ms */

static int16_t circularCutoutValue(uint8_t chix)
{
  int32_t value = channelOutputs[chix];
  uint8_t chpair = 0xff;
  const int32_t limit = 1048576; /* 1024^2 */

  if (chix == _usbJoystickAxisPairs[0][0])
    chpair = _usbJoystickAxisPairs[0][1];
  else if (chix == _usbJoystickAxisPairs[0][1])
    chpair = _usbJoystickAxisPairs[0][0];
  else if (chix == _usbJoystickAxisPairs[1][0])
    chpair = _usbJoystickAxisPairs[1][1];
  else if (chix == _usbJoystickAxisPairs[1][1])
    chpair = _usbJoystickAxisPairs[1][0];

  if (chpair != 0xff) {
    int32_t pval = channelOutputs[chpair];
    int32_t sum = ((value * value) + (pval * pval));
    if (sum > limit) {
      double ratio = sqrt((double)limit / (double)sum);
      value *= ratio;
    }
  }

  return value;
}

void usbStateUpdate()
{
  if (_hidReport == nullptr) return;

  const uint8_t button_ix = _usbJoystickAxisCount * 2;
  const uint8_t axis_ix = 0;
  const uint8_t battery_ix = button_ix + ((USBJ_BUTTON_SIZE + 7) >> 3);

  memset(_hidReport, 0, MAX_HID_REPORT);

  uint8_t chix = 0;

  int16_t value = 0;

  uint8_t swpos = 0;
  uint8_t btnval = 0;

  for (uint8_t i = 0; i < _usbJoystickButtonCount; i++) {
    chix = _usbJS->_usbJoystickButtonChannels[i];

    value = channelOutputs[chix] + 1024;
    if (value > 2047)
      value = 2047;
    else if (value < 0)
      value = 0;

    if (g_model.usbJoystickCh[chix].inversion) value = 2047 - value;

    swpos = g_model.usbJoystickCh[chix].switch_npos + 1;

    // Quantization of the values
    if ((g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_NORMAL) ||
        (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_ON_PULSE)) {
      if (swpos == 1) {
        btnval = (value > 1024);
      } else {
        btnval = static_cast<uint16_t>(value) / (2048 / swpos);
        if (btnval >= swpos) btnval = swpos - 1;
      }
    } else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_SW_EMU) {
      btnval = (value > 1024);
    } else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_DELTA) {
      btnval = (value >> 6);
    } else if (g_model.usbJoystickCh[chix].param ==
               USBJOYS_BTN_MODE_COMPANION) {
      if (swpos == 1 || swpos == 2) {
        btnval = (value > 1024);
      } else if (swpos == 3) {
        btnval = (value < 1024) ? 1 : (value > 1024) ? 2 : 0;
      } else {
        btnval = static_cast<uint16_t>(value) / (2048 / swpos);
        if (btnval >= swpos) btnval = swpos - 1;
      }
    }

    // Channel Output value changed
    if ((_usbJS->_usbLastChannelOutput[chix] == 0xffff) ||
        (_usbJS->_usbLastChannelOutput[chix] != btnval)) {
      if ((g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_NORMAL) ||
          (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_ON_PULSE)) {
        if (swpos == 1) {
          setBtnBits(g_model.usbJoystickCh[chix].btn_num, btnval, 1);
        } else {
          setBtnBits(g_model.usbJoystickCh[chix].btn_num, 1 << btnval, swpos);
        }

        // Timer
        if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_ON_PULSE) {
          _usbJS->_usbChannelTimerActive[chix] = 1;
          _usbJS->_usbChannelTimer[chix] = g_usbTmr10ms;
        }
      } else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_SW_EMU) {
        if ((_usbJS->_usbLastChannelOutput[chix] != 0xffff) &&
            (_usbJS->_usbLastChannelOutput[chix] < btnval)) {
          toggleBtnBit(g_model.usbJoystickCh[chix].btn_num);
        }
      } else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_DELTA) {
        if (_usbJS->_usbLastChannelOutput[chix] != 0xffff) {
          if (_usbJS->_usbLastChannelOutput[chix] < btnval) {
            setBtnBits(g_model.usbJoystickCh[chix].btn_num, 2, 2);
          } else if (_usbJS->_usbLastChannelOutput[chix] > btnval) {
            setBtnBits(g_model.usbJoystickCh[chix].btn_num, 1, 2);
          }
          _usbJS->_usbChannelTimerActive[chix] = 1;
          _usbJS->_usbChannelTimer[chix] = g_usbTmr10ms;
        }
      } else if (g_model.usbJoystickCh[chix].param ==
                 USBJOYS_BTN_MODE_COMPANION) {
        if (swpos == 1 || swpos == 2) {
          setBtnBits(g_model.usbJoystickCh[chix].btn_num, btnval, 1);
        } else if (swpos == 3) {
          setBtnBits(g_model.usbJoystickCh[chix].btn_num, btnval, 2);
        } else {
          setBtnBits(g_model.usbJoystickCh[chix].btn_num, 1 << btnval, swpos);
        }
      }

      _usbJS->_usbLastChannelOutput[chix] = btnval;
    }

    // Timer check
    if ((_usbJS->_usbChannelTimerActive[chix]) &&
        (((uint8_t)(g_usbTmr10ms - _usbJS->_usbChannelTimer[chix]) >=
          BTNPUSH_TIME))) {
      _usbJS->_usbChannelTimerActive[chix] = 0;

      if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_ON_PULSE) {
        setBtnBits(g_model.usbJoystickCh[chix].btn_num, 0, swpos);
      } else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_DELTA) {
        setBtnBits(g_model.usbJoystickCh[chix].btn_num, 0, 2);
      }
    }
  }

  memcpy(_hidReport + button_ix, _buttonState, ((USBJ_BUTTON_SIZE + 7) >> 3));

  for (uint8_t i = 0; i < _usbJoystickAxisCount; i++) {
    chix = _usbJS->_usbJoystickAxisChannels[i];

    value = limit<int16_t>(0, circularCutoutValue(chix) + 1024, 2048);

    if (g_model.usbJoystickCh[chix].inversion) value = 2048 - value;

    _hidReport[i * 2 + axis_ix] = static_cast<uint8_t>(value & 0xFF);
    _hidReport[i * 2 + axis_ix + 1] = static_cast<uint8_t>(value >> 8);
  }

  // battery values
  setBatteryBits(battery_ix);

  // dpad switch
  if (0 <= _usbJoystickDpadChannel) {
    setDpadBits(battery_ix + 1, _usbJoystickDpadChannel);
  }
}

uint8_t usbReportSize() { return _hidReportSize; }

struct usbReport_t usbReport()
{
  if (!usbJoystickExtMode()) {
    // Classic USB Joystick report
    usbClassicStateUpdate();
  } else {
    usbStateUpdate();
  }

  usbReport_t res = {_hidReport, _hidReportSize};
  return res;
}

void onUSBJoystickModelChanged()
{
  if (!usbJoystickActive()) return;

  if (setupUSBJoystick()) {
#if defined(STM32) && !defined(SIMU)
    usbJoystickRestart();
#endif
  }
}

int isUSBAxisCollision(uint8_t chIdx)
{
  USBJoystickChData* cch = usbJChAddress(chIdx);

  // non axis
  if (cch->mode != USBJOYS_CH_AXIS) return false;

  // non-unique axis check
  if (!isUniqueAxisType(cch->param)) {
    return false;
  }

  // collision check
  for (int i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
    if (i == chIdx) continue;

    USBJoystickChData* och = usbJChAddress(i);

    if (cch->mode != och->mode) continue;
    if (cch->param == och->param) {
      return true;
    }
  }

  return false;
}

int isUSBSimCollision(uint8_t chIdx)
{
  USBJoystickChData* cch = usbJChAddress(chIdx);

  // non axis
  if (cch->mode != USBJOYS_CH_SIM) return false;

  // non-unique axis check
  if (!isUniqueSimType(cch->param)) {
    return false;
  }

  // collision check
  for (int i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
    if (i == chIdx) continue;

    USBJoystickChData* och = usbJChAddress(i);

    if (cch->mode != och->mode) continue;
    if (cch->param == och->param) {
      return true;
    }
  }

  return false;
}

int isUSBBtnNumCollision(uint8_t chIdx)
{
  USBJoystickChData* cch = usbJChAddress(chIdx);

  // non button
  if (cch->mode != USBJOYS_CH_BUTTON) return false;

  // collision check
  uint8_t cfirst = cch->btn_num;
  uint8_t clast = cch->lastBtnNum();
  for (int i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
    if (i == chIdx) continue;

    USBJoystickChData* och = usbJChAddress(i);

    if (och->mode != USBJOYS_CH_BUTTON) continue;

    uint8_t ofirst = och->btn_num;
    uint8_t olast = och->lastBtnNum();

    // overlap
    if ((cfirst <= olast) && (ofirst <= clast)) return true;
  }

  return false;
}
