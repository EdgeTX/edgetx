/*
 * Copyright (C) OpenTX
 *
 * Based on code named
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

#include "radio_hardware.h"
#include "radio_calibration.h"
#include "radio_diagkeys.h"
#include "radio_diaganas.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

#if defined(PCBHORUS)
#define SWITCH_TYPE_MAX(sw)            ((MIXSRC_SF-MIXSRC_FIRST_SWITCH == sw || MIXSRC_SH-MIXSRC_FIRST_SWITCH == sw) ? SWITCH_2POS : SWITCH_3POS)
#else
#define SWITCH_TYPE_MAX(sw)            (SWITCH_3POS)
#endif

class SwitchDynamicLabel: public StaticText {
  public:
    SwitchDynamicLabel(Window * parent, const rect_t & rect, uint8_t index):
      StaticText(parent, rect),
      index(index)
    {
      update();
    }

    std::string label()
    {
      static const char switchPositions[] = {
        CHAR_UP,
        '-',
        CHAR_DOWN
      };
      return TEXT_AT_INDEX(STR_VSRCRAW, (index + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1)) + std::string(&switchPositions[lastpos], 1);
    }

    void update()
    {
      uint8_t newpos = position();
      if (newpos != lastpos) {
        lastpos = newpos;
        setText(label());
      }
    }

    uint8_t position()
    {
      auto value = getValue(MIXSRC_FIRST_SWITCH + index);
      if (value > 0)
        return 2;
      else if (value < 0)
        return 0;
      else
        return 1;
    }

    void checkEvents() override
    {
      update();
    }

  protected:
    uint8_t index;
    uint8_t lastpos = 0xff;
};

RadioHardwarePage::RadioHardwarePage():
  PageTab(STR_HARDWARE, ICON_RADIO_HARDWARE)
{
}

#if defined(BLUETOOTH)
class BluetoothConfigWindow : public FormGroup
{
  public:
    BluetoothConfigWindow(FormWindow * parent, const rect_t &rect) :
      FormGroup(parent, rect, FORWARD_SCROLL | FORM_FORWARD_FOCUS)
    {
      update();
    }

    void update()
    {
      FormGridLayout grid;
#if LCD_W > LCD_H
      grid.setLabelWidth(180);
#else
      grid.setLabelWidth(130);
#endif
      clear();

      new StaticText(this, grid.getLabelSlot(true), STR_MODE);
      btMode = new Choice(this, grid.getFieldSlot(), STR_BLUETOOTH_MODES, BLUETOOTH_OFF, BLUETOOTH_TRAINER, GET_DEFAULT(g_eeGeneral.bluetoothMode), [=](int32_t newValue) {
          g_eeGeneral.bluetoothMode = newValue;
          update();
          SET_DIRTY();
          btMode->setFocus(SET_FOCUS_DEFAULT);
      });
      grid.nextLine();

      if (g_eeGeneral.bluetoothMode != BLUETOOTH_OFF) {
        // Pin code (displayed for information only, not editable)
        if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY) {
          new StaticText(this, grid.getLabelSlot(true), STR_BLUETOOTH_PIN_CODE);
          new StaticText(this, grid.getFieldSlot(), "000000");
          grid.nextLine();
        }

        // Local MAC
        new StaticText(this, grid.getLabelSlot(true), STR_BLUETOOTH_LOCAL_ADDR);
        new StaticText(this, grid.getFieldSlot(), bluetooth.localAddr[0] == '\0' ? "---" : bluetooth.localAddr);
        grid.nextLine();

        // Remote MAC
        new StaticText(this, grid.getLabelSlot(true), STR_BLUETOOTH_DIST_ADDR);
        new StaticText(this, grid.getFieldSlot(), bluetooth.distantAddr[0] == '\0' ? "---" : bluetooth.distantAddr);
        grid.nextLine();

        // BT radio name
        new StaticText(this, grid.getLabelSlot(true), STR_NAME);
        new RadioTextEdit(this, grid.getFieldSlot(), g_eeGeneral.bluetoothName, LEN_BLUETOOTH_NAME);
        grid.nextLine();
      }

      getParent()->moveWindowsTop(top() + 1, adjustHeight());
    }

  protected:
    Choice * btMode = nullptr;
};
#endif

void RadioHardwarePage::build(FormWindow * window)
{
  FormGridLayout grid;
#if LCD_W > LCD_H
  grid.setLabelWidth(180);
#else
  grid.setLabelWidth(130);
#endif
  grid.spacer(PAGE_PADDING);

  // Calibration
  new StaticText(window, grid.getLabelSlot(), STR_INPUTS, 0, FONT(BOLD));
  auto calib = new TextButton(window, grid.getFieldSlot(), STR_CALIBRATION);
  calib->setPressHandler([=]() -> uint8_t {
      auto calibrationPage = new RadioCalibrationPage();
      calibrationPage->setCloseHandler([=]() {
          calib->setFocus(SET_FOCUS_DEFAULT);
      });
      return 0;
  });
  grid.nextLine();

  // Sticks
  new Subtitle(window, grid.getLineSlot(), STR_STICKS);
  grid.nextLine();
  for (int i = 0; i < NUM_STICKS; i++) {
    new StaticText(window, grid.getLabelSlot(true), TEXT_AT_INDEX(STR_VSRCRAW, (i + 1)));
    new RadioTextEdit(window, grid.getFieldSlot(2,0), g_eeGeneral.anaNames[i], LEN_ANA_NAME);
    grid.nextLine();
  }

  // Pots
  new Subtitle(window, grid.getLineSlot(), STR_POTS);
  grid.nextLine();
  for (int i = 0; i < NUM_POTS; i++) {
    new StaticText(window, grid.getLabelSlot(true), TEXT_AT_INDEX(STR_VSRCRAW, (i + NUM_STICKS + 1)));
    new RadioTextEdit(window, grid.getFieldSlot(2,0), g_eeGeneral.anaNames[i + NUM_STICKS], LEN_ANA_NAME);
    new Choice(window, grid.getFieldSlot(2,1), STR_POTTYPES, POT_NONE, POT_WITHOUT_DETENT,
               [=]() -> int {
                   return bfGet<uint32_t>(g_eeGeneral.potsConfig, 2*i, 2);
               },
               [=](int newValue) {
                   g_eeGeneral.potsConfig = bfSet<uint32_t>(g_eeGeneral.potsConfig, newValue, 2*i, 2);
                   SET_DIRTY();
               });
    grid.nextLine();
  }

  // Sliders
  new Subtitle(window, grid.getLineSlot(), STR_SLIDERS);
  grid.nextLine();
  for (int i = 0; i < NUM_SLIDERS; i++) {
    const int idx = i + NUM_STICKS + NUM_POTS;
    new StaticText(window, grid.getLabelSlot(true),
                   TEXT_AT_INDEX(STR_VSRCRAW, idx + 1));
    new RadioTextEdit(window, grid.getFieldSlot(2, 0),
                      g_eeGeneral.anaNames[idx], LEN_ANA_NAME);
    new Choice(
        window, grid.getFieldSlot(2, 1), STR_SLIDERTYPES, SLIDER_NONE,
        SLIDER_WITH_DETENT,
        [=]() -> int {
          uint8_t mask = (0x01 << i);
          return (g_eeGeneral.slidersConfig & mask) >> i;
        },
        [=](int newValue) {
          uint8_t mask = (0x01 << i);
          g_eeGeneral.slidersConfig &= ~mask;
          g_eeGeneral.slidersConfig |= (newValue << i);
          SET_DIRTY();
        });
    grid.nextLine();
  }

  // Switches
  new Subtitle(window, grid.getLineSlot(), STR_SWITCHES);
  grid.nextLine();
  for (int i = 0; i < NUM_SWITCHES; i++) {
    new SwitchDynamicLabel(window, grid.getLabelSlot(true), i);
    new RadioTextEdit(window, grid.getFieldSlot(2, 0), g_eeGeneral.switchNames[i], LEN_SWITCH_NAME);
    new Choice(window, grid.getFieldSlot(2, 1), STR_SWTYPES, SWITCH_NONE, SWITCH_TYPE_MAX(i),
               [=]() -> int {
                   return SWITCH_CONFIG(i);
               },
               [=](int newValue) {
                   swconfig_t mask = (swconfig_t) 0x03 << (2 * i);
                   g_eeGeneral.switchConfig = (g_eeGeneral.switchConfig & ~mask) | ((swconfig_t(newValue) & 0x03) << (2 * i));
                   SET_DIRTY();
               });
    grid.nextLine();
  }

  // Bat calibration
  new StaticText(window, grid.getLabelSlot(), STR_BATT_CALIB);
  auto batCal = new NumberEdit(window, grid.getFieldSlot(1,0), -127, 127, GET_SET_DEFAULT(g_eeGeneral.txVoltageCalibration));
  batCal->setDisplayHandler([](BitmapBuffer * dc, LcdFlags flags, int32_t value) {
      dc->drawNumber(FIELD_PADDING_LEFT, FIELD_PADDING_TOP, getBatteryVoltage(), flags | PREC2, 0, nullptr, "V");
  });
  batCal->setWindowFlags(REFRESH_ALWAYS);
  grid.nextLine();

  // RTC Batt display
  new StaticText(window, grid.getLabelSlot(), STR_RTC_BATT);
  new DynamicNumber<uint16_t>(window, grid.getFieldSlot(1,0), [] {
      return getRTCBatteryVoltage();
  }, PREC2, nullptr, "V");
  grid.nextLine();

  // RTC Batt check enable
  new StaticText(window, grid.getLabelSlot(), STR_RTC_CHECK);
  new CheckBox(window, grid.getFieldSlot(1,0), GET_SET_INVERTED(g_eeGeneral.disableRtcWarning ));
  grid.nextLine();

#if defined(CROSSFIRE) && SPORT_MAX_BAUDRATE < 400000
  // Max baud for external modules
  new StaticText(window, grid.getLabelSlot(), STR_MAXBAUDRATE);
  new Choice(window, grid.getFieldSlot(1,0), STR_CRSF_BAUDRATE, 0, 1, GET_SET_DEFAULT(g_eeGeneral.telemetryBaudrate));
  grid.nextLine();
#endif

#if defined(BLUETOOTH)
  // Bluetooth mode
  {
    new Subtitle(window, grid.getLineSlot(), STR_BLUETOOTH);
    grid.nextLine();
    grid.addWindow(new BluetoothConfigWindow(window, {0, grid.getWindowHeight(), LCD_W, 0}));
  }
#endif

#if defined(AUX_SERIAL)
  new StaticText(window, grid.getLabelSlot(), STR_AUX_SERIAL_MODE);
#if defined(TELEMETRY_MAVLINK) && !defined(CLI) && !defined(DEBUG)
  auto aux = new Choice(window, grid.getFieldSlot(2,0), STR_MAVLINK_AUX_SERIAL_MODES, 0, UART_MODE_MAX, GET_SET_DEFAULT(g_eeGeneral.auxSerialMode));
  new Choice(window, grid.getFieldSlot(2,1), STR_MAVLINK_AUX_BAUDRATES, 0, 3, GET_SET_DEFAULT(g_eeGeneral.mavlinkBaudrate));
#else
  auto aux = new Choice(window, grid.getFieldSlot(1,0), STR_AUX_SERIAL_MODES, 0, UART_MODE_MAX, GET_SET_DEFAULT(g_eeGeneral.auxSerialMode));
#endif
  aux->setAvailableHandler([=](int value) {
      return isAuxModeAvailable;
  });
  grid.nextLine();
#endif

#if defined(AUX2_SERIAL)
  new StaticText(window, grid.getLabelSlot(), STR_AUX2_SERIAL_MODE);
#if defined(TELEMETRY_MAVLINK) && !defined(CLI) && !defined(DEBUG)
  auto aux2 = new Choice(window, grid.getFieldSlot(2,0), STR_MAVLINK_AUX_SERIAL_MODES, 0, UART_MODE_MAX, GET_SET_DEFAULT(g_eeGeneral.aux2SerialMode));
  new Choice(window, grid.getFieldSlot(2,1), STR_MAVLINK_AUX_BAUDRATES, 0, 3, GET_SET_DEFAULT(g_eeGeneral.mavlinkBaudrate2));
#else
  auto aux2 = new Choice(window, grid.getFieldSlot(1,0), STR_AUX_SERIAL_MODES, 0, UART_MODE_MAX, GET_SET_DEFAULT(g_eeGeneral.aux2SerialMode));
#endif
  aux2->setAvailableHandler([=](int value) {
      return isAux2ModeAvailable;
  });
  grid.nextLine();
#endif

#if defined(AUX_SERIAL) || defined(AUX2_SERIAL)
  new StaticText(window, grid.getFieldSlot(1,0), STR_TTL_WARNING, 0, ALARM_COLOR);
  grid.nextLine();
#endif

  // ADC filter
  new StaticText(window, grid.getLabelSlot(), STR_JITTER_FILTER);
  new CheckBox(window, grid.getFieldSlot(1,0), GET_SET_INVERTED(g_eeGeneral.jitterFilter));
  grid.nextLine();

  // Debugs
  new StaticText(window, grid.getLabelSlot(), STR_DEBUG, 0, FONT(BOLD));
  auto debugAnas = new TextButton(window, grid.getFieldSlot(2, 0), STR_ANALOGS_BTN);
  debugAnas->setPressHandler([=]() -> uint8_t {
      auto debugAnalogsPage = new RadioAnalogsDiagsPage();
      debugAnalogsPage->setCloseHandler([=]() {
          calib->setFocus(SET_FOCUS_DEFAULT);
      });
      return 0;
  });

  auto debugKeys = new TextButton(window, grid.getFieldSlot(2, 1), STR_KEYS_BTN);
  debugKeys->setPressHandler([=]() -> uint8_t {
    auto debugKeysPage = new RadioKeyDiagsPage();
    debugKeysPage->setCloseHandler([=]() {
        calib->setFocus(SET_FOCUS_DEFAULT);
    });
    return 0;
  });
  grid.nextLine();

  window->setInnerHeight(grid.getWindowHeight());
}
