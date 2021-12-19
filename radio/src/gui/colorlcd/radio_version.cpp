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

#include "radio_version.h"
#include "opentx.h"
#include "options.h"
#include "libopenui.h"
#include "fw_version.h"

char *getVersion(char *str, PXX2Version version)
{
  if (version.major == 0xFF && version.minor == 0x0F &&
      version.revision == 0x0F) {
    return strAppend(str, "---", 4);
  } else {
    sprintf(str, "%u.%u.%u", (1 + version.major) % 0xFF, version.minor,
            version.revision);
    return str;
  }
}

class versionDialog: public Dialog
{
  public:
    versionDialog(Window * parent, rect_t rect) :
      Dialog(parent, STR_MODULES_RX_VERSION, rect)
    {
      memclear(&reusableBuffer.hardwareAndSettings.modules, sizeof(reusableBuffer.hardwareAndSettings.modules));
      reusableBuffer.hardwareAndSettings.updateTime = get_tmr10ms();
#if defined(HARDWARE_INTERNAL_MODULE)
      // Query modules
      if (isModulePXX2(INTERNAL_MODULE) && IS_INTERNAL_MODULE_ON()) {
        moduleState[INTERNAL_MODULE].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE], PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
      }
#endif

      if (isModulePXX2(EXTERNAL_MODULE) && IS_EXTERNAL_MODULE_ON()) {
        moduleState[EXTERNAL_MODULE].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE], PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
      }

      update();
    }

    void update()
    {
      FormGroup * form = &content->form;
      FormGridLayout grid(content->form.width());
      form->clear();

      grid.setLabelWidth(100);

#if defined(HARDWARE_INTERNAL_MODULE)
      // Internal module
      drawModuleVersion(form, &grid, INTERNAL_MODULE);
      grid.nextLine();
#endif

      // external module
      drawModuleVersion(form, &grid, EXTERNAL_MODULE);
      grid.nextLine();

      // Exit
      exitButton = new TextButton(form, grid.getLabelSlot(), "EXIT",
                                  [=]() -> int8_t {
                                      this->deleteLater();
                                      return 0;
                                  });
      exitButton->setFocus(SET_FOCUS_DEFAULT);
      grid.nextLine();

      grid.spacer(PAGE_PADDING);
      form->setHeight(grid.getWindowHeight());
      content->adjustHeight();
    }

    void drawModuleVersion(FormGroup * form, FormGridLayout *grid, uint8_t module)
    {
      char tmp[20];

      // Module
      if (module == INTERNAL_MODULE)
        new StaticText(form, grid->getLineSlot(), STR_INTERNAL_MODULE, 0, COLOR_THEME_PRIMARY1);
      else
        new StaticText(form, grid->getLineSlot(), STR_EXTERNAL_MODULE, 0, COLOR_THEME_PRIMARY1);
      grid->nextLine();

      new StaticText(form, grid->getLabelSlot(true), STR_MODULE, 0, COLOR_THEME_PRIMARY1);
      if (g_model.moduleData[module].type == MODULE_TYPE_NONE) {
        new StaticText(form, grid->getFieldSlot(1, 0), STR_OFF, 0, COLOR_THEME_PRIMARY1);
      }
#if defined(HARDWARE_EXTERNAL_ACCESS_MOD)
      else if (isModuleMultimodule(module)) {
        char statusText[64];
        new StaticText(form, grid->getFieldSlot(2, 0), "Multimodule", 0, COLOR_THEME_PRIMARY1);
        getMultiModuleStatus(module).getStatusString(statusText);
        new StaticText(form, grid->getFieldSlot(2, 1), statusText, 0, COLOR_THEME_PRIMARY1);
      }
#endif
      else if (!isModulePXX2(module)) {
        new StaticText(form, grid->getFieldSlot(1, 0), STR_NO_INFORMATION, 0, COLOR_THEME_PRIMARY1);
      }
      else {
        // PXX2 Module
        new StaticText(form, grid->getFieldSlot(4, 0), getPXX2ModuleName(reusableBuffer.hardwareAndSettings.modules[module].information.modelID), 0, COLOR_THEME_PRIMARY1);
        if (reusableBuffer.hardwareAndSettings.modules[module].information.modelID) {
          new StaticText(form, grid->getFieldSlot(4, 1), getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module].information.hwVersion), 0, COLOR_THEME_PRIMARY1);
          new StaticText(form, grid->getFieldSlot(4, 2), getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module].information.swVersion), 0, COLOR_THEME_PRIMARY1);
          static const char * variants[] = {"FCC", "EU", "FLEX"};
          uint8_t variant = reusableBuffer.hardwareAndSettings.modules[module].information.variant - 1;
          if (variant < DIM(variants)) {
            new StaticText(form, grid->getFieldSlot(4, 3), variants[variant], 0, COLOR_THEME_PRIMARY1);
          }
        }
        grid->nextLine();

        // PXX2 Receivers
        for (uint8_t receiver=0; receiver<PXX2_MAX_RECEIVERS_PER_MODULE; receiver++) {
          if (reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].information.modelID) {
            // Receiver model
            new StaticText(form, grid->getLabelSlot(true), STR_RECEIVER, 0, COLOR_THEME_PRIMARY1);
            uint8_t modelId = reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].information.modelID;
            new StaticText(form, grid->getFieldSlot(4, 0), getPXX2ReceiverName(modelId), 0, COLOR_THEME_PRIMARY1);

            // Receiver version
            new StaticText(form, grid->getFieldSlot(4, 1), getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].information.hwVersion), 0, COLOR_THEME_PRIMARY1);
            new StaticText(form, grid->getFieldSlot(4, 2), getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module].receivers[receiver].information.swVersion), 0, COLOR_THEME_PRIMARY1);
            grid->nextLine();
          }
        }
      }
    }

    void checkEvents() override
    {
      if (get_tmr10ms() >= reusableBuffer.hardwareAndSettings.updateTime) {
        // Query modules
#if defined(HARDWARE_INTERNAL_MODULE)
        if (isModulePXX2(INTERNAL_MODULE) && IS_INTERNAL_MODULE_ON()) {
          moduleState[INTERNAL_MODULE].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE], PXX2_HW_INFO_TX_ID,
                                                             PXX2_MAX_RECEIVERS_PER_MODULE - 1);
        }
#endif
        if (isModulePXX2(EXTERNAL_MODULE) && IS_EXTERNAL_MODULE_ON()) {
          moduleState[EXTERNAL_MODULE].readModuleInformation(&reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE], PXX2_HW_INFO_TX_ID,
                                                             PXX2_MAX_RECEIVERS_PER_MODULE - 1);
        }
        reusableBuffer.hardwareAndSettings.updateTime = get_tmr10ms() + 500 /* 5s*/;
      }
      update();
      Dialog::checkEvents();
    }

  protected:
    rect_t rect;
    TextButton * exitButton;
};

class OptionsText: public StaticText {
  public:
    OptionsText(Window * parent, const rect_t &rect) :
      StaticText(parent, rect, "", 0, COLOR_THEME_PRIMARY1)
    {
      coord_t optionWidth = 0;
      for (uint8_t i = 0; options[i]; i++) {
        const char * option = options[i];
        optionWidth += getTextWidth(option);
        if (optionWidth + 5 > width()) {
          setHeight(height() + 20);
          optionWidth = 0;
        }
      }
    };

    void paint(BitmapBuffer * dc) override
    {
      coord_t y = 2;
      coord_t x = 0;
      for (uint8_t i = 0; options[i]; i++) {
        const char * option = options[i];
        coord_t optionWidth = getTextWidth(option);
        if (x + 5 + optionWidth > width()) {
          dc->drawText(x, y, ",", COLOR_THEME_PRIMARY1);
          x = 0;
          y += FH;
        }
        if (i > 0 && x != 0)
          x = dc->drawText(x, y, ", ", COLOR_THEME_PRIMARY1);
        x = dc->drawText(x, y, option, COLOR_THEME_PRIMARY1);
      }
    }
};

RadioVersionPage::RadioVersionPage():
  PageTab(STR_MENUVERSION, ICON_RADIO_VERSION)
{
}

extern uint32_t NV14internalModuleFwVersion;
#if defined(PCBNV14) || defined(PCBPL18)
extern const char* boardLcdType;
#endif

void RadioVersionPage::build(FormWindow * window)
{
  FormGridLayout grid;
  grid.setLabelWidth(80);
  grid.spacer(PAGE_PADDING);

  // Radio type
  new StaticText(window, grid.getLineSlot(), fw_stamp, 0, COLOR_THEME_PRIMARY1);
  grid.nextLine();

  // Firmware version
  new StaticText(window, grid.getLineSlot(), vers_stamp, 0, COLOR_THEME_PRIMARY1);
  grid.nextLine();

  // Firmware date
  new StaticText(window, grid.getLineSlot(), date_stamp, 0, COLOR_THEME_PRIMARY1);
  grid.nextLine();

  // Firmware time
  new StaticText(window, grid.getLineSlot(), time_stamp, 0, COLOR_THEME_PRIMARY1);
  grid.nextLine();

  // Configuration version
  new StaticText(window, grid.getLineSlot(), cfgv_stamp, 0, COLOR_THEME_PRIMARY1);
  grid.nextLine();

  // Firmware options
  new StaticText(window, grid.getLabelSlot(), "OPTS:", 0, COLOR_THEME_PRIMARY1);
  auto options = new OptionsText(window, grid.getFieldSlot(1,0));
  grid.nextLine(options->height() + 4);

#if defined(SPI_FLASH)
  new StaticText(window, grid.getLabelSlot(), "SPI Flash ID:");
  sprintf(reusableBuffer.moduleSetup.msg, "%04X", flashSpiReadID());
  new StaticText(window, grid.getFieldSlot(1,0), reusableBuffer.moduleSetup.msg);
  grid.nextLine();
#endif

#if (defined(PCBNV14) || defined(PCBPL18)) && !defined(SIMU)
  new StaticText(window, grid.getLabelSlot(), "LCD:");
  new StaticText(window, grid.getFieldSlot(), boardLcdType);
  grid.nextLine();
#endif

#if defined(AFHDS2)
  new StaticText(window, grid.getLabelSlot(), "RF FW:");
  sprintf(reusableBuffer.moduleSetup.msg, "%d.%d.%d",
          (int)((NV14internalModuleFwVersion >> 16) & 0xFF),
          (int)((NV14internalModuleFwVersion >> 8) & 0xFF),
          (int)(NV14internalModuleFwVersion & 0xFF));
  new StaticText(window, grid.getFieldSlot(), reusableBuffer.moduleSetup.msg);
  grid.nextLine();
#endif

#if defined(PXX2)
  // Module and receivers versions
  auto moduleVersions =
      new TextButton(window, grid.getLineSlot(), STR_MODULES_RX_VERSION);
  moduleVersions->setPressHandler([=]() -> uint8_t {
    new versionDialog(window, {50, 30, LCD_W - 100, 0});
    return 0;
  });
#endif
}
