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

#include "radio_hardware.h"
#include "radio_calibration.h"
#include "radio_diagkeys.h"
#include "radio_diaganas.h"
#include "opentx.h"
#include "libopenui.h"
#include "hal/adc_driver.h"
#include "aux_serial_driver.h"

#define SET_DIRTY() storageDirty(EE_GENERAL)

static const lv_coord_t col_two_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t col_three_dsc[] = {LV_GRID_FR(2), LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

#if defined(PCBHORUS)
#define SWITCH_TYPE_MAX(sw)            ((MIXSRC_SF-MIXSRC_FIRST_SWITCH == sw || MIXSRC_SH-MIXSRC_FIRST_SWITCH == sw) ? SWITCH_2POS : SWITCH_3POS)
#else
#define SWITCH_TYPE_MAX(sw)            (SWITCH_3POS)
#endif

class SwitchDynamicLabel: public StaticText {
  public:
    SwitchDynamicLabel(Window * parent, const rect_t & rect, uint8_t index):
      StaticText(parent, rect, "", 0, COLOR_THEME_PRIMARY1),
      index(index)
    {
      update();
    }

    std::string label()
    {
      static const char* switchPositions[] = {
         STR_CHAR_UP,
         "-",
         STR_CHAR_DOWN
      };
      return TEXT_AT_INDEX(STR_VSRCRAW, (index + MIXSRC_FIRST_SWITCH - MIXSRC_Rud + 1)) + std::string(switchPositions[lastpos]);
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

class ModeChoice : public Choice
{
 public:
  ModeChoice(Window *parent, const rect_t &rect, const char **values,
             int vmin, int vmax, std::function<int()> getValue,
             std::function<void(int)> setValue = nullptr,
             bool *menuOpen = nullptr) :
      Choice(parent, rect, values, vmin, vmax, getValue, setValue),
      menuOpen(menuOpen)
  {
  }

 protected:
  void openMenu()
  {
    if (menuOpen) *menuOpen = true;
    Choice::openMenu();
  }

 private:
  bool *menuOpen;
};

class BluetoothConfigWindow : public FormGroup
{
  public:
    BluetoothConfigWindow(Window * parent, const rect_t &rect) :
      FormGroup(parent, rect, FORWARD_SCROLL // | FORM_FORWARD_FOCUS
                )
    {
      update();
    }

    void checkEvents() override
    {
      FormGroup::checkEvents();
      if (!rte) return;
      if (bluetooth.state != lastbluetoothstate) {
        lastbluetoothstate = bluetooth.state;
        if (!(modechoiceopen || rte->hasFocus())) update();
      }
    }

    void update()
    {
      clear();

      setFlexLayout();
      FlexGridLayout grid(col_three_dsc, row_dsc, 2);

      auto line = newLine(&grid);

      auto label = new StaticText(line, rect_t{}, STR_MODE, 0,
                     COLOR_THEME_PRIMARY1);
      lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
      modechoiceopen = false;
      btMode = new ModeChoice(
          line, rect_t{}, STR_BLUETOOTH_MODES, BLUETOOTH_OFF,
          BLUETOOTH_TRAINER, GET_DEFAULT(g_eeGeneral.bluetoothMode),
          [=](int32_t newValue) {
            g_eeGeneral.bluetoothMode = newValue;
            update();
            SET_DIRTY();
            // btMode->setFocus(SET_FOCUS_DEFAULT);
            modechoiceopen = false;
          },
          &modechoiceopen);
      line = newLine(&grid);

      if (g_eeGeneral.bluetoothMode != BLUETOOTH_OFF) {
        // Pin code (displayed for information only, not editable)
        if (g_eeGeneral.bluetoothMode == BLUETOOTH_TELEMETRY) {
          label = new StaticText(line, rect_t{}, STR_BLUETOOTH_PIN_CODE, 0, COLOR_THEME_PRIMARY1);
          lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
          new StaticText(line, rect_t{}, "000000", 0, COLOR_THEME_PRIMARY1);
          line = newLine(&grid);
        }

        // Local MAC
        label = new StaticText(line, rect_t{}, STR_BLUETOOTH_LOCAL_ADDR, 0, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
        new StaticText(line, rect_t{}, bluetooth.localAddr[0] == '\0' ? "---" : bluetooth.localAddr, 0, COLOR_THEME_PRIMARY1);
        line = newLine(&grid);

        // Remote MAC
        label = new StaticText(line, rect_t{}, STR_BLUETOOTH_DIST_ADDR, 0, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
        new StaticText(line, rect_t{}, bluetooth.distantAddr[0] == '\0' ? "---" : bluetooth.distantAddr, 0, COLOR_THEME_PRIMARY1);
        line = newLine(&grid);

        // BT radio name
        label = new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
        lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
        rte = new RadioTextEdit(line, rect_t{}, g_eeGeneral.bluetoothName, LEN_BLUETOOTH_NAME);
        line = newLine(&grid);
      }

      getParent()->moveWindowsTop(top() + 1, adjustHeight());
    }

   protected:
    Choice *btMode = nullptr;
    RadioTextEdit *rte = nullptr;

   private:
    bool modechoiceopen = false;
    uint8_t lastbluetoothstate = BLUETOOTH_STATE_OFF;
};
#endif

class SerialConfigWindow : public FormGroup
{
 public:
  SerialConfigWindow(Window *parent, const rect_t &rect) :
      FormGroup(parent, rect, FORWARD_SCROLL // | FORM_FORWARD_FOCUS
                )
  {
    update();
  }

  void update()
  {
    clear();

    setFlexLayout();
    FlexGridLayout grid(col_two_dsc, row_dsc, 2);

    auto line = newLine(&grid);

    bool display_ttl_warning = false;
    for (uint8_t port_nr = 0; port_nr < MAX_SERIAL_PORTS; port_nr++) {
      auto port = serialGetPort(port_nr);
      if (!port || !port->name) continue;

      display_ttl_warning = true;
      auto label = new StaticText(line, rect_t{}, port->name, 0,
                     COLOR_THEME_PRIMARY1);
      lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
      auto aux = new Choice(
          line, rect_t{}, STR_AUX_SERIAL_MODES, 0,
          UART_MODE_MAX, [=]() { return serialGetMode(port_nr); },
          [=](int value) {
            serialSetMode(port_nr, value);
            serialInit(port_nr, value);
            SET_DIRTY();
          });
      aux->setAvailableHandler(
          [=](int value) { return isSerialModeAvailable(port_nr, value); });
      line = newLine(&grid);

#if defined(SWSERIALPOWER)
      if (port_nr < SP_VCP)
      {
          auto label = new StaticText(line, rect_t{}, STR_AUX_SERIAL_PORT_POWER, 0, COLOR_THEME_PRIMARY1);
          lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
          new CheckBox(
              line, rect_t{},
                [=] { return serialGetPower(port_nr); },
                [=](int8_t newValue) {
                   serialSetPower(port_nr, (bool)newValue);
                   SET_DIRTY();
                }
          );
          line = newLine(&grid);
      }
#endif
    }

    if (display_ttl_warning) {
      grid.setColSpan(2);
      auto label = new StaticText(line, rect_t{}, STR_TTL_WARNING, 0,
                     COLOR_THEME_WARNING);
      lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
      grid.setColSpan(1);
      line = newLine(&grid);
    }

    getParent()->moveWindowsTop(top() + 1, adjustHeight());
  }
};

class InternalModuleWindow : public FormGroup {
 public:
  InternalModuleWindow(Window *parent, const rect_t &rect) :
    FormGroup(parent, rect, FORWARD_SCROLL),
    lastModule(g_eeGeneral.internalModule)
  {
    update();
  }

  void update()
  {
    clear();

    setFlexLayout();
    FlexGridLayout grid(col_two_dsc, row_dsc, 2);

    auto line = newLine(&grid);

    auto label = new StaticText(line, rect_t{}, TR_INTERNAL_MODULE, 0,
                   COLOR_THEME_PRIMARY1);
    lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
    auto internalModule = new Choice(line, rect_t{},STR_INTERNAL_MODULE_PROTOCOLS, MODULE_TYPE_NONE, MODULE_TYPE_COUNT - 1,
                                     GET_DEFAULT(g_eeGeneral.internalModule),
                                     [=](int moduleType) {
                                       if (g_model.moduleData[INTERNAL_MODULE].type != moduleType) {
                                         memclear(&g_model.moduleData[INTERNAL_MODULE], sizeof(ModuleData));
                                         storageDirty(EE_MODEL);
                                       }
                                       g_eeGeneral.internalModule = moduleType;
                                       SET_DIRTY();
                                     });

    internalModule->setAvailableHandler([](int module){
      return isInternalModuleSupported(module);
    });
    line = newLine(&grid);

#if defined(CROSSFIRE)
    if (isInternalModuleCrossfire()) {
      auto label = new StaticText(line, rect_t{}, STR_BAUDRATE, 0,COLOR_THEME_PRIMARY1);
      lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
      new Choice(line, rect_t{}, STR_CRSF_BAUDRATE, 0,CROSSFIRE_MAX_INTERNAL_BAUDRATE,
          [=]() -> int {
            return CROSSFIRE_STORE_TO_INDEX(g_eeGeneral.internalModuleBaudrate);
          },
          [=](int newValue) {
            g_eeGeneral.internalModuleBaudrate = CROSSFIRE_INDEX_TO_STORE(newValue);
            SET_DIRTY();
            restartModule(INTERNAL_MODULE);
          });
      line = newLine(&grid);
    }
#endif
    getParent()->moveWindowsTop(top() + 1, adjustHeight());
  }

  void checkEvents() override
  {
    if (g_eeGeneral.internalModule != lastModule) {
      lastModule = g_eeGeneral.internalModule;
      update();
    }

    FormGroup::checkEvents();
  }

 protected:
  uint8_t lastModule = 0;
};

void RadioHardwarePage::build(FormWindow * window)
{
  window->setFlexLayout();
  FlexGridLayout grid(col_three_dsc, row_dsc, 2);

  auto line = window->newLine(&grid);

  // Calibration
  new StaticText(line, rect_t{}, STR_INPUTS, 0, COLOR_THEME_PRIMARY1 | FONT(BOLD));
  auto calib = new TextButton(line, rect_t{}, STR_CALIBRATION);
  calib->setPressHandler([=]() -> uint8_t {
      // auto calibrationPage = 
      new RadioCalibrationPage();
      // calibrationPage->setCloseHandler([=]() {
      //     calib->setFocus(SET_FOCUS_DEFAULT);
      // });
      return 0;
  });
  line = window->newLine(&grid);

  // Sticks
  new Subtitle(line, rect_t{}, STR_STICKS, 0, COLOR_THEME_PRIMARY1);
  line = window->newLine(&grid);
  for (int i = 0; i < NUM_STICKS; i++) {
    auto label = new StaticText(line, rect_t{}, TEXT_AT_INDEX(STR_VSRCRAW, (i + 1)), 0, COLOR_THEME_PRIMARY1);
    lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
    auto textEdit = new RadioTextEdit(line, rect_t{}, g_eeGeneral.anaNames[i], LEN_ANA_NAME);
    lv_obj_set_style_grid_cell_x_align(textEdit->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
    line = window->newLine(&grid);
  }

#if defined(STICK_DEAD_ZONE)
  auto text = new StaticText(line, rect_t{}, STR_DEAD_ZONE);
  lv_obj_set_style_pad_left(text->getLvObj(), lv_dpx(8), LV_PART_MAIN);
  auto choice =
      new Choice(line, rect_t{}, 0, 7,
                 GET_DEFAULT(g_eeGeneral.stickDeadZone), [=](uint8_t newValue) {
                   g_eeGeneral.stickDeadZone = newValue;
                   SET_DIRTY();
                 });
  choice->setTextHandler([](uint8_t value) {
    return std::to_string(value ? 2 << (value - 1) : 0);
  });
  line = window->newLine(&grid);
#endif

  // Pots
  new Subtitle(line, rect_t{}, STR_POTS, 0, COLOR_THEME_PRIMARY1);
  line = window->newLine(&grid);
  for (int i = 0; i < NUM_POTS; i++) {
    // Display EX3 & EX4 (= last two pots) only when FlySky gimbals are present
#if !defined(SIMU) && defined(RADIO_FAMILY_T16)
      if (!globalData.flyskygimbals && (i >= (NUM_POTS - 2)))
        continue;
#endif

    auto label = new StaticText(line, rect_t{}, TEXT_AT_INDEX(STR_VSRCRAW, (i + NUM_STICKS + 1)), 0, COLOR_THEME_PRIMARY1);
    lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);

    auto textEdit = new RadioTextEdit(line, rect_t{}, g_eeGeneral.anaNames[i + NUM_STICKS], LEN_ANA_NAME);
    lv_obj_set_style_grid_cell_x_align(textEdit->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
    new Choice(line, rect_t{}, STR_POTTYPES, POT_NONE, POT_WITHOUT_DETENT,
               [=]() -> int {
                   return bfGet<uint32_t>(g_eeGeneral.potsConfig, 2*i, 2);
               },
               [=](int newValue) {
                   g_eeGeneral.potsConfig = bfSet<uint32_t>(g_eeGeneral.potsConfig, newValue, 2*i, 2);
                   SET_DIRTY();
               });
    line = window->newLine(&grid);
  }

  // Sliders
#if (NUM_SLIDERS > 0)
  new Subtitle(line, rect_t{}, STR_SLIDERS, 0, COLOR_THEME_PRIMARY1);
  line = window->newLine(&grid);
  for (int i = 0; i < NUM_SLIDERS; i++) {
    const int idx = i + NUM_STICKS + NUM_POTS;
    auto label  = new StaticText(line, rect_t{}, TEXT_AT_INDEX(STR_VSRCRAW, idx + 1), 0, COLOR_THEME_PRIMARY1);
    lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
    auto textEdit = new RadioTextEdit(line, rect_t{}, g_eeGeneral.anaNames[idx], LEN_ANA_NAME);
    lv_obj_set_style_grid_cell_x_align(textEdit->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
    new Choice(
        window, rect_t{}, STR_SLIDERTYPES, SLIDER_NONE,
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
    line = window->newLine(&grid);
  }
#endif

  // Switches
  new Subtitle(line, rect_t{}, STR_SWITCHES, 0, COLOR_THEME_PRIMARY1);
  line = window->newLine(&grid);
  for (int i = 0; i < NUM_SWITCHES; i++) {
    auto label = new SwitchDynamicLabel(line, rect_t{}, i);
    lv_obj_set_style_pad_left(label->getLvObj(), lv_dpx(8), LV_PART_MAIN);
    auto textEdit = new RadioTextEdit(line, rect_t{}, g_eeGeneral.switchNames[i], LEN_SWITCH_NAME);
    lv_obj_set_style_grid_cell_x_align(textEdit->getLvObj(), LV_GRID_ALIGN_STRETCH, 0);
    new Choice(line, rect_t{}, STR_SWTYPES, SWITCH_NONE, SWITCH_TYPE_MAX(i),
               [=]() -> int {
                   return SWITCH_CONFIG(i);
               },
               [=](int newValue) {
                   swconfig_t mask = (swconfig_t) 0x03 << (2 * i);
                   g_eeGeneral.switchConfig = (g_eeGeneral.switchConfig & ~mask) | ((swconfig_t(newValue) & 0x03) << (2 * i));
                   SET_DIRTY();
               });
    line = window->newLine(&grid);
  }

  // Bat calibration
  new StaticText(line, rect_t{}, STR_BATT_CALIB, 0, COLOR_THEME_PRIMARY1);
  auto batCal = new NumberEdit(line, rect_t{}, -127, 127, GET_SET_DEFAULT(g_eeGeneral.txVoltageCalibration));
  batCal->setDisplayHandler([](int32_t value) {
      return formatNumberAsString(getBatteryVoltage(), PREC2, 0, nullptr, "V");
  });
  batCal->setWindowFlags(REFRESH_ALWAYS);
  line = window->newLine(&grid);

  // RTC Batt display
  new StaticText(line, rect_t{}, STR_RTC_BATT, 0, COLOR_THEME_PRIMARY1);
  new DynamicNumber<uint16_t>(line, rect_t{}, [] {
      return getRTCBatteryVoltage();
  }, COLOR_THEME_PRIMARY1 | PREC2, nullptr, "V");
  line = window->newLine(&grid);

  // RTC Batt check enable
  new StaticText(line, rect_t{}, STR_RTC_CHECK, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_INVERTED(g_eeGeneral.disableRtcWarning ));
  line = window->newLine(&grid);

#if defined(HARDWARE_INTERNAL_MODULE)
  new Subtitle(line, rect_t{}, TR_INTERNALRF, 0, COLOR_THEME_PRIMARY1);
  line = window->newLine(&grid);
  lv_obj_set_style_pad_left(line->getLvObj(), 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(line->getLvObj(), 0, LV_PART_MAIN);
  // Date and Time
  grid.setColSpan(2);
  new InternalModuleWindow(line, rect_t{});
  grid.setColSpan(1);
  line = window->newLine(&grid);
#endif

#if defined(BLUETOOTH)
  // Bluetooth mode
  {
    new Subtitle(line, rect_t{}, STR_BLUETOOTH, 0, COLOR_THEME_PRIMARY1);
    line = window->newLine(&grid);
    lv_obj_set_style_pad_left(line->getLvObj(), 0, LV_PART_MAIN);
    lv_obj_set_style_pad_right(line->getLvObj(), 0, LV_PART_MAIN);
    grid.setColSpan(2);
    new BluetoothConfigWindow(line, rect_t{});
    grid.setColSpan(1);
    line = window->newLine(&grid);
  }
#endif

  new Subtitle(line, rect_t{}, STR_AUX_SERIAL_MODE, 0,
               COLOR_THEME_PRIMARY1);
  line = window->newLine(&grid);
  lv_obj_set_style_pad_left(line->getLvObj(), 0, LV_PART_MAIN);
  lv_obj_set_style_pad_right(line->getLvObj(), 0, LV_PART_MAIN);
  grid.setColSpan(2);
  new SerialConfigWindow(line, rect_t{});
  grid.setColSpan(1);
  line = window->newLine(&grid);

  // ADC filter
  new StaticText(line, rect_t{}, STR_JITTER_FILTER, 0, COLOR_THEME_PRIMARY1);
  new CheckBox(line, rect_t{}, GET_SET_INVERTED(g_eeGeneral.noJitterFilter));
  line = window->newLine(&grid);

  // Debugs
  new StaticText(line, rect_t{}, STR_DEBUG, 0, COLOR_THEME_PRIMARY1 | FONT(BOLD));
  auto debugAnas = new TextButton(line, rect_t{}, STR_ANALOGS_BTN);
  debugAnas->setPressHandler([=]() -> uint8_t {
      // auto debugAnalogsPage = 
      new RadioAnalogsDiagsViewPageGroup();
      // debugAnalogsPage->setCloseHandler([=]() {
      //     calib->setFocus(SET_FOCUS_DEFAULT);
      // });
      return 0;
  });

  auto debugKeys = new TextButton(line, rect_t{}, STR_KEYS_BTN);
  debugKeys->setPressHandler([=]() -> uint8_t {
    // auto debugKeysPage = 
    new RadioKeyDiagsPage();
    // debugKeysPage->setCloseHandler([=]() {
    //     calib->setFocus(SET_FOCUS_DEFAULT);
    // });
    return 0;
  });
  line = window->newLine(&grid);

// extra bottom padding if touchscreen
#if defined HARDWARE_TOUCH
  new StaticText(line, rect_t{});
#endif

}
