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

#include <math.h>

#include "opentx.h"
#include "hal/usb_driver.h"
#include "usb_joystick.h"

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

static uint8_t _buttonState[(USBJ_BUTTON_SIZE+7) >> 3] = { };

static uint8_t _usbJoystickIfMode = 0;
static uint8_t _usbJoystickCircularCut = 0;

#define USBJ_CC_XYZrX   1
#define USBJ_CC_XYrXrZ  2

static uint8_t _usbJoystickButtonCount = 0;
static uint8_t _usbJoystickAxisCount = 0;

static uint8_t _usbJoystickAxisPairs[3][2] = { };

/*
  This USB HID endpoint report description defines a device with:
    * 24 digital buttons
    * 8 analog axes with 8bit resolution

  Report packet described as C struct is:

  struct {
    uint8_t buttons1; //bit 0 - button 1, bit 1 - button 2, ..., mapped to channels 9-16, on if channel > 0
    uint8_t buttons2; // mapped to channels 17-24, on if channel > 0
    uint8_t buttons3; // mapped to channels 25-32, on if channel > 0
    uint8_t X;  //analog value, mapped to channel 1
    uint8_t Y;  //analog value, mapped to channel 2
    uint8_t Z;  //analog value, mapped to channel 3
    uint8_t Rx; //analog value, mapped to channel 4
    uint8_t Ry  //analog value, mapped to channel 5
    uint8_t Rz; //analog value, mapped to channel 6
    uint8_t S1; //analog value, mapped to channel 7
    uint8_t S2; //analog value, mapped to channel 8
  }
*/
static const uint8_t HID_JOYSTICK_ReportDesc[] =
{
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x05,                    //     USAGE (Game Pad)
    0xa1, 0x01,                    //     COLLECTION (Application)
    0xa1, 0x00,                    //       COLLECTION (Physical)
    0x05, 0x09,                    //         USAGE_PAGE (Button)
    0x19, 0x01,                    //         USAGE_MINIMUM (Button 1)
    0x29, 0x18,                    //         USAGE_MAXIMUM (Button 24)
    0x15, 0x00,                    //         LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //         LOGICAL_MAXIMUM (1)
    0x95, 0x18,                    //         REPORT_COUNT (24)
    0x75, 0x01,                    //         REPORT_SIZE (1)
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
    0x16, 0x00, 0x00,              //         LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x07,              //         LOGICAL_MAXIMUM (2047)
    0x75, 0x10,                    //         REPORT_SIZE (16)
    0x95, 0x08,                    //         REPORT_COUNT (8)
    0x81, 0x02,                    //         INPUT (Data,Var,Abs)
    0xc0,                          //       END_COLLECTION
    0xc0                           //     END_COLLECTION
};

static bool isUniqueAxisType(int type) { return type < 6; }
static bool isUniqueSimType(int type) { return true; }

int usbJoystickExtMode()
{
  return g_model.usbJoystickExtMode;
}

int usbJoystickActive()
{
  return (usbPlugged() && getSelectedUsbMode() == USB_JOYSTICK_MODE);
}

int usbJoystickSettingsChanged()
{
  static uint32_t settingsChecksum = 0;

  if (!usbJoystickActive())
    return false;

  if ((_usbJoystickIfMode != g_model.usbJoystickIfMode) ||
      (_usbJoystickCircularCut != g_model.usbJoystickCircularCut))
    return true;

  uint32_t oldChecksum = settingsChecksum;
  settingsChecksum = hash((uint8_t*)&g_model.usbJoystickCh, sizeof(g_model.usbJoystickCh));
  
  return oldChecksum != settingsChecksum;
}

int setupUSBJoystick()
{
  static const uint8_t axisTypeCodes[USBJOYS_AXIS_LAST + 1] =
    { 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38 };
  static const uint8_t simTypeCodes[USBJOYS_SIM_LAST + 1] =
    { 0xb0, 0xb8, 0xba, 0xbb };

  if (_hidReportDesc == nullptr) {
    _hidReportDesc = (uint8_t*)malloc(MAX_HID_REPORTDESC);
    if (_hidReportDesc == nullptr)
      return false;
  }

  if (_hidReport == nullptr) {
    _hidReport = (uint8_t*)malloc(MAX_HID_REPORT);
    if (_hidReport == nullptr)
      return false;
  }

  if (_usbJS == nullptr) {
    _usbJS = (struct _usbJSData*)malloc(sizeof(struct _usbJSData));
    if (_usbJS == nullptr)
      return false;
  }

  // properties to detect change
  uint8_t oldHIDReportDescSize = _hidReportDescSize;
  uint32_t oldChecksum = hash(_hidReportDesc, _hidReportDescSize);

  // copy the model snapshot
  _usbJoystickIfMode = g_model.usbJoystickIfMode;
  _usbJoystickCircularCut = g_model.usbJoystickCircularCut;

  // Init data
  memset(_hidReportDesc, 0, MAX_HID_REPORTDESC);
  _hidReportDescSize = 0;
  memset(_hidReport, 0, MAX_HID_REPORT);
  _hidReportSize = 0;
  memset(_buttonState, 0, ((USBJ_BUTTON_SIZE+7) >> 3));
  memset(_usbJS, 0, sizeof(struct _usbJSData));
  memset(_usbJS->_usbLastChannelOutput, 0xff, sizeof(_usbJS->_usbLastChannelOutput));

  _usbJoystickButtonCount = 0;
  _usbJoystickAxisCount = 0;
  memset(_usbJoystickAxisPairs, 0xff, sizeof(_usbJoystickAxisPairs));
  
  if (!usbJoystickExtMode()) {
    // Classic USB Joystick report description
    memcpy(_hidReportDesc, HID_JOYSTICK_ReportDesc, sizeof(HID_JOYSTICK_ReportDesc));
    _hidReportDescSize = sizeof(HID_JOYSTICK_ReportDesc);
    _hidReportSize = 19;

  } else {

    uint8_t genAxisCount = 0;
    uint8_t simAxisCount = 0;
    uint8_t buttonCount = 0;

    // sort channels by type
    uint8_t typeCount[USBJOYS_CH_LAST + 1] = { };
    uint8_t axisCount[USBJOYS_AXIS_LAST + 1] = { };
    uint8_t simCount[USBJOYS_SIM_LAST + 1] = { };
    uint8_t axisIx[USBJOYS_AXIS_LAST + 1] = { };
    uint8_t simIx[USBJOYS_SIM_LAST + 1] = { };
    uint8_t mode = 0;
    uint8_t submode = 0;
    for (uint8_t i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
      mode = g_model.usbJoystickCh[i].mode;

      if (g_model.usbJoystickCh[i].mode == USBJOYS_CH_BUTTON) {
        buttonCount += g_model.usbJoystickCh[i].btnCount();
        typeCount[mode]++;
      }
      else if (g_model.usbJoystickCh[i].mode == USBJOYS_CH_AXIS) {
        submode = g_model.usbJoystickCh[i].param;
        if (submode <= USBJOYS_AXIS_LAST) {
          if (!isUniqueAxisType(submode) || (axisCount[submode] == 0)) {
            axisCount[submode]++;
            typeCount[mode]++;
          }
        }
      }
      else if (g_model.usbJoystickCh[i].mode == USBJOYS_CH_SIM) {
        submode = g_model.usbJoystickCh[i].param;
        if (submode <= USBJOYS_SIM_LAST) {
          if (!isUniqueSimType(submode) || (simCount[submode] == 0)) {
            simCount[submode]++;
            typeCount[mode]++;
          }
        }
      }
    }
    uint8_t ixcount = 0;
    for (uint8_t i = 0; i <= USBJOYS_AXIS_LAST; i++) {
      axisIx[i] = ixcount;
      ixcount += axisCount[i];
    }
    ixcount = 0;
    for (uint8_t i = 0; i <= USBJOYS_SIM_LAST; i++) {
      simIx[i] = ixcount;
      ixcount += simCount[i];
    }
    ixcount = 0;
    for (uint8_t i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
      mode = g_model.usbJoystickCh[i].mode;

      if (g_model.usbJoystickCh[i].mode == USBJOYS_CH_BUTTON) {
        _usbJS->_usbJoystickButtonChannels[ixcount] = i;
        ixcount++;
      }
      else if (g_model.usbJoystickCh[i].mode == USBJOYS_CH_AXIS) {
        submode = g_model.usbJoystickCh[i].param;
        if (submode <= USBJOYS_AXIS_LAST) {
          _usbJS->_usbJoystickAxisChannels[axisIx[submode]] = i;
          if (!isUniqueAxisType(submode)) {
            axisIx[submode]++;
          }
          // save axis pairs
          if(submode == USBJOYS_AXIS_X) {
            _usbJoystickAxisPairs[0][0] = i;
          }
          else if(submode == USBJOYS_AXIS_Y) {
            _usbJoystickAxisPairs[0][1] = i;
          }
          else if(submode == USBJOYS_AXIS_Z) {
            _usbJoystickAxisPairs[1][0] = i;
          }
          else if(submode == USBJOYS_AXIS_RX) {
            _usbJoystickAxisPairs[1][1] = i;
            _usbJoystickAxisPairs[2][0] = i;
          }
          else if(submode == USBJOYS_AXIS_RY) {
            _usbJoystickAxisPairs[2][1] = i;
          }
        }
      }
      else if (g_model.usbJoystickCh[i].mode == USBJOYS_CH_SIM) {
        submode = g_model.usbJoystickCh[i].param;
        if (submode <= USBJOYS_SIM_LAST) {
          _usbJS->_usbJoystickAxisChannels[simIx[submode]] = i;
          if (!isUniqueSimType(submode)) {
            simIx[submode]++;
          }
        }
      }
    }

    // store counts
    _usbJoystickButtonCount = typeCount[USBJOYS_CH_BUTTON];
    genAxisCount = typeCount[USBJOYS_CH_AXIS];
    simAxisCount = typeCount[USBJOYS_CH_SIM];
    _usbJoystickAxisCount = typeCount[USBJOYS_CH_AXIS] + typeCount[USBJOYS_CH_SIM];

    // generate report desc

    // USAGE_PAGE (Generic Desktop)
    memcpy(_hidReportDesc, HID_JOYSTICK_ReportDesc, 24);
    _hidReportDescSize = 24;

    // USAGE (Joystick=0x04, Gamepad=0x05,  Multi-axis Controller=0x08)
    uint8_t joystickType = 0x04;
    if (_usbJoystickIfMode == USBJOYS_GAMEPAD) joystickType = 0x05;
    else if (_usbJoystickIfMode == USBJOYS_MULTIAXIS) joystickType = 0x08;

    _hidReportDesc[3] = joystickType;
    _hidReportDesc[13] = buttonCount ? buttonCount : 1;
    _hidReportDesc[19] = USBJ_BUTTON_SIZE;

    // generic axis types
    if (genAxisCount > 0) {

      // USAGE_PAGE (Generic Desktop)
      _hidReportDesc[_hidReportDescSize++] = 0x05;
      _hidReportDesc[_hidReportDescSize++] = 0x01;

      for (uint8_t i = 0; i <= USBJOYS_AXIS_LAST; i++) {
        for (uint8_t cnt = 0; cnt < axisCount[i]; cnt++) {
          // USAGE (#)
          _hidReportDesc[_hidReportDescSize++] = 0x09;
          _hidReportDesc[_hidReportDescSize++] = axisTypeCodes[i];
        }
      }

      memcpy(_hidReportDesc+_hidReportDescSize, HID_JOYSTICK_ReportDesc+42, 12);
      _hidReportDesc[_hidReportDescSize+9] = genAxisCount;
      _hidReportDescSize += 12;
    }

    // sim axis types
    if (simAxisCount > 0) {

      // USAGE_PAGE (Simulation Controls)
      _hidReportDesc[_hidReportDescSize++] = 0x05;
      _hidReportDesc[_hidReportDescSize++] = 0x02;

      for (uint8_t i = 0; i <= USBJOYS_SIM_LAST; i++) {
        for (uint8_t cnt = 0; cnt < simCount[i]; cnt++) {
          // USAGE (#)
          _hidReportDesc[_hidReportDescSize++] = 0x09;
          _hidReportDesc[_hidReportDescSize++] = simTypeCodes[i];
        }
      }

      memcpy(_hidReportDesc+_hidReportDescSize, HID_JOYSTICK_ReportDesc+42, 12);
      _hidReportDesc[_hidReportDescSize+9] = simAxisCount;
      _hidReportDescSize += 12;
    }

    // END_COLLECTION
    _hidReportDesc[_hidReportDescSize++] = 0xc0;

    // END_COLLECTION
    _hidReportDesc[_hidReportDescSize++] = 0xc0;

    // end of report desc

    _hidReportSize = ((USBJ_BUTTON_SIZE+7) / 8) + (_usbJoystickAxisCount * 2);
  }

  //compare with the old description
  if (_hidReportDescSize != oldHIDReportDescSize) return true;
  uint32_t newChecksum = hash(_hidReportDesc, _hidReportDescSize);
  return oldChecksum != newChecksum;
}

extern "C" struct usbReport_t usbReportDesc()
{
  usbReport_t res = { _hidReportDesc, _hidReportDescSize };
  return res;
}

void usbClassicStateUpdate()
{
  if (_hidReport == nullptr) return;

  memset(_hidReport, 0, MAX_HID_REPORT);

  //buttons
  _hidReport[0] = 0;
  _hidReport[1] = 0;
  _hidReport[2] = 0;
  for (int i = 0; i < 8; ++i) {
    if ( channelOutputs[i+8] > 0 ) {
      _hidReport[0] |= (1 << i);
    }
    if ( channelOutputs[i+16] > 0 ) {
      _hidReport[1] |= (1 << i);
    }
    if ( channelOutputs[i+24] > 0 ) {
      _hidReport[2] |= (1 << i);
    }
  }

  //analog values
  //uint8_t * p = HID_Buffer + 1;
  for (int i = 0; i < 8; ++i) {

    int16_t value = channelOutputs[i] + 1024;
    if ( value > 2047 ) value = 2047;
    else if ( value < 0 ) value = 0;
    _hidReport[i*2 +3] = static_cast<uint8_t>(value & 0xFF);
    _hidReport[i*2 +4] = static_cast<uint8_t>((value >> 8) & 0x07);

  }
}

static void setBtnBits(uint8_t ix, uint8_t value, uint8_t size)
{
  for(uint8_t i = 0; i < size; i++) {
    uint8_t bitix = ix + i;
    if(bitix >= USBJ_BUTTON_SIZE) break;

    if(value & (1 << i)) {
      _buttonState[(bitix / 8)] |= 1 << (bitix % 8);
    }
    else {
      _buttonState[(bitix / 8)] &= ~(1 << (bitix % 8));
    }
  }
}

static void toggleBtnBit(uint8_t bitix)
{
  if(bitix >= USBJ_BUTTON_SIZE) return;
  _buttonState[(bitix / 8)] ^= 1 << (bitix % 8);
}

#define g_usbTmr10ms    (*(uint8_t*)&g_tmr10ms)
#define BTNPUSH_TIME  20 /* x10ms */

static int16_t circularCutoutValue(uint8_t chix)
{
  int32_t value = channelOutputs[chix];
  uint8_t chpair = 0xff;
  const int32_t limit = 1048576; /* 1024^2 */

  if(_usbJoystickCircularCut == USBJ_CC_XYZrX) {
    if(chix == _usbJoystickAxisPairs[0][0]) chpair = _usbJoystickAxisPairs[0][1];
    else if(chix == _usbJoystickAxisPairs[0][1]) chpair = _usbJoystickAxisPairs[0][0];
    else if(chix == _usbJoystickAxisPairs[1][0]) chpair = _usbJoystickAxisPairs[1][1];
    else if(chix == _usbJoystickAxisPairs[1][1]) chpair = _usbJoystickAxisPairs[1][0];
  }
  else if(_usbJoystickCircularCut == USBJ_CC_XYrXrZ) {
    if(chix == _usbJoystickAxisPairs[0][0]) chpair = _usbJoystickAxisPairs[0][1];
    else if(chix == _usbJoystickAxisPairs[0][1]) chpair = _usbJoystickAxisPairs[0][0];
    else if(chix == _usbJoystickAxisPairs[2][0]) chpair = _usbJoystickAxisPairs[2][1];
    else if(chix == _usbJoystickAxisPairs[2][1]) chpair = _usbJoystickAxisPairs[2][0];
  }

  if(chpair != 0xff) {
    int32_t pval = channelOutputs[chpair];
    int32_t sum = ((value*value) + (pval*pval));
    if(sum > limit) {
      double ratio = sqrt((double)limit / (double)sum);
      value *= ratio;
    }
  }

  return value;
}

void usbStateUpdate()
{
  if (_hidReport == nullptr) return;

  const uint8_t button_ix = 0;
  const uint8_t axis_ix = button_ix + ((USBJ_BUTTON_SIZE+7) >> 3);

  memset(_hidReport, 0, MAX_HID_REPORT);

  uint8_t chix = 0;

  int16_t value = 0;

  uint8_t swpos = 0;
  uint8_t btnval = 0;

  for (uint8_t i = 0; i < _usbJoystickButtonCount; i++) {
    chix = _usbJS->_usbJoystickButtonChannels[i];

    value = channelOutputs[chix] + 1024;
    if ( value > 2047 ) value = 2047;
    else if ( value < 0 ) value = 0;

    if (g_model.usbJoystickCh[chix].inversion) value = 2047 - value;

    swpos = g_model.usbJoystickCh[chix].switch_npos + 1;

    // Quantization of the values
    if ((g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_NORMAL)
       || (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_ON_PULSE)) {

      if (swpos == 1) {
        btnval = (value > 1024);
      } else {
        btnval = static_cast<uint16_t>(value) / (2048 / swpos);
        if (btnval >= swpos) btnval = swpos - 1;
      }
    }
    else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_SW_EMU) {
      btnval = (value > 1024);
    }
    else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_DELTA) {
      btnval = (value >> 6);
    }
    else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_COMPANION) {
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
    if ((_usbJS->_usbLastChannelOutput[chix] == 0xffff) || (_usbJS->_usbLastChannelOutput[chix] != btnval)) {

      if ((g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_NORMAL)
         || (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_ON_PULSE)) {

        if (swpos == 1) {
          setBtnBits(g_model.usbJoystickCh[chix].btn_num, btnval, 1);
        } else {
          setBtnBits(g_model.usbJoystickCh[chix].btn_num, 1 << btnval, swpos);
        }

        // Timer
        if(g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_ON_PULSE) {
          _usbJS->_usbChannelTimerActive[chix] = 1;
          _usbJS->_usbChannelTimer[chix] = g_usbTmr10ms;
        }
      }
      else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_SW_EMU) {
        if ((_usbJS->_usbLastChannelOutput[chix] != 0xffff) && (_usbJS->_usbLastChannelOutput[chix] < btnval)) {
          toggleBtnBit(g_model.usbJoystickCh[chix].btn_num);
        }
      }
      else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_DELTA) {
        if (_usbJS->_usbLastChannelOutput[chix] != 0xffff) {
          if (_usbJS->_usbLastChannelOutput[chix] < btnval) {
            setBtnBits(g_model.usbJoystickCh[chix].btn_num, 2, 2);
          }
          else if (_usbJS->_usbLastChannelOutput[chix] > btnval) {
            setBtnBits(g_model.usbJoystickCh[chix].btn_num, 1, 2);
          }
          _usbJS->_usbChannelTimerActive[chix] = 1;
          _usbJS->_usbChannelTimer[chix] = g_usbTmr10ms;
        }
      }
      else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_COMPANION) {
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
    if((_usbJS->_usbChannelTimerActive[chix]) && (((uint8_t)(g_usbTmr10ms - _usbJS->_usbChannelTimer[chix]) >= BTNPUSH_TIME))) {
      _usbJS->_usbChannelTimerActive[chix] = 0;

      if(g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_ON_PULSE) {
        setBtnBits(g_model.usbJoystickCh[chix].btn_num, 0, swpos);
      }
      else if (g_model.usbJoystickCh[chix].param == USBJOYS_BTN_MODE_DELTA) {
        setBtnBits(g_model.usbJoystickCh[chix].btn_num, 0, 2);
      }
    }
  }

  memcpy(_hidReport + button_ix, _buttonState, ((USBJ_BUTTON_SIZE+7) >> 3));

  for (uint8_t i = 0; i < _usbJoystickAxisCount; i++) {
    chix = _usbJS->_usbJoystickAxisChannels[i];

    value = circularCutoutValue(chix) + 1024;
    if ( value > 2047 ) value = 2047;
    else if ( value < 0 ) value = 0;

    if (g_model.usbJoystickCh[chix].inversion) value = 2047 - value;

    _hidReport[i*2 + axis_ix] = static_cast<uint8_t>(value & 0xFF);
    _hidReport[i*2 + axis_ix+1] = static_cast<uint8_t>((value >> 8) & 0x07);
  }  
}

uint8_t usbReportSize()
{
  return _hidReportSize;
}

struct usbReport_t usbReport()
{
  if (!usbJoystickExtMode()) {
    // Classic USB Joystick report
    usbClassicStateUpdate();
  } else {
    usbStateUpdate();
  }
  
  usbReport_t res = { _hidReport, _hidReportSize };
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
  USBJoystickChData * cch = usbJChAddress(chIdx);

  // non axis
  if (cch->mode != USBJOYS_CH_AXIS) return false;

  // non-unique axis check
  if (!isUniqueAxisType(cch->param)) {
    return false;
  }

  // collision check
  for (int i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
    if (i == chIdx) continue;

    USBJoystickChData * och = usbJChAddress(i);

    if (cch->mode != och->mode) continue;
    if (cch->param == och->param) {
      return true;
    }
  }

  return false;
}

int isUSBSimCollision(uint8_t chIdx)
{
  USBJoystickChData * cch = usbJChAddress(chIdx);

  // non axis
  if (cch->mode != USBJOYS_CH_SIM) return false;

  // non-unique axis check
  if (!isUniqueSimType(cch->param)) {
    return false;
  }

  // collision check
  for (int i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
    if (i == chIdx) continue;

    USBJoystickChData * och = usbJChAddress(i);

    if (cch->mode != och->mode) continue;
    if (cch->param == och->param) {
      return true;
    }
  }

  return false;
}

int isUSBBtnNumCollision(uint8_t chIdx)
{
  USBJoystickChData * cch = usbJChAddress(chIdx);

  // non button
  if (cch->mode != USBJOYS_CH_BUTTON) return false;

  // collision check
  uint8_t cfirst = cch->btn_num;
  uint8_t clast = cch->lastBtnNum();
  for (int i = 0; i < USBJ_MAX_JOYSTICK_CHANNELS; i++) {
    if (i == chIdx) continue;

    USBJoystickChData * och = usbJChAddress(i);

    if (och->mode != USBJOYS_CH_BUTTON) continue;

    uint8_t ofirst = och->btn_num;
    uint8_t olast = och->lastBtnNum();

    // overlap
    if ((cfirst <= olast) && (ofirst <= clast)) return true;
  }

  return false;
}
