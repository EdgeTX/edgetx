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

#if defined(CROSSFIRE)
  #include "mixer_scheduler.h"
#endif

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

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class VersionDialog : public Dialog
{
  StaticText* int_name;
  StaticText* int_status;
  Window*     int_rx_line;
  StaticText* int_rx_name;
  StaticText* int_rx_status;

  StaticText* ext_name;
  StaticText* ext_status;
  Window*     ext_rx_line;
  StaticText* ext_rx_name;
  StaticText* ext_rx_status;
  
 public:
  VersionDialog(Window *parent) :
    Dialog(parent, STR_MODULES_RX_VERSION, rect_t{ 0, 0, 200, 100 })
  {
    memclear(&reusableBuffer.hardwareAndSettings.modules,
             sizeof(reusableBuffer.hardwareAndSettings.modules));
    reusableBuffer.hardwareAndSettings.updateTime = get_tmr10ms();

    // Query modules
    if (isModulePXX2(INTERNAL_MODULE) && IS_INTERNAL_MODULE_ON()) {
      moduleState[INTERNAL_MODULE].readModuleInformation(
          &reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE],
          PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
    }

    if (isModulePXX2(EXTERNAL_MODULE) && IS_EXTERNAL_MODULE_ON()) {
      moduleState[EXTERNAL_MODULE].readModuleInformation(
          &reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE],
          PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
    }

    setCloseWhenClickOutside(true);

    auto form = &content->form;
    form->setFlexLayout();
    
    FlexGridLayout grid(col_dsc, row_dsc);

    new StaticText(form, rect_t{}, STR_INTERNAL_MODULE, 0, COLOR_THEME_PRIMARY1);

    auto line = form->newLine(&grid);
    new StaticText(line, rect_t{}, STR_MODULE, 0, COLOR_THEME_PRIMARY1);
    int_name = new StaticText(line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);
    int_status = new StaticText(line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);

    int_rx_line = form->newLine(&grid);
    new StaticText(int_rx_line, rect_t{}, STR_RECEIVER, 0, COLOR_THEME_PRIMARY1);
    int_rx_name = new StaticText(int_rx_line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);
    int_rx_status = new StaticText(int_rx_line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);
    lv_obj_add_flag(int_rx_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);

    new StaticText(form, rect_t{}, STR_EXTERNAL_MODULE, 0, COLOR_THEME_PRIMARY1);

    line = form->newLine(&grid);
    new StaticText(line, rect_t{}, STR_MODULE, 0, COLOR_THEME_PRIMARY1);
    ext_name = new StaticText(line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);
    ext_status = new StaticText(line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);

    ext_rx_line = form->newLine(&grid);
    new StaticText(ext_rx_line, rect_t{}, STR_RECEIVER, 0, COLOR_THEME_PRIMARY1);
    ext_rx_name = new StaticText(ext_rx_line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);
    ext_rx_status = new StaticText(ext_rx_line, rect_t{}, "", 0, COLOR_THEME_PRIMARY1);
    lv_obj_add_flag(ext_rx_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);

    content->setWidth(LCD_W * 0.8);
    update();
  }

  void update()
  {
    updateModule(INTERNAL_MODULE, int_name, int_status, int_rx_line,
                 int_rx_name, int_rx_status);
    updateModule(EXTERNAL_MODULE, ext_name, ext_status, ext_rx_line,
                 ext_rx_name, ext_rx_status);
    content->updateSize();
  }

  void updateModule(uint8_t module, StaticText* name, StaticText* status,
                    Window* rx_line, StaticText* rx_name, StaticText* rx_status)
  {
    char tmp[20];

    if (g_model.moduleData[module].type == MODULE_TYPE_NONE) {
      name->setText(STR_OFF);
      status->setText("");
    }
#if defined(CROSSFIRE)
    else if (isModuleCrossfire(module)) {
      name->setText("CRSF");

      char statusText[64];
      auto hz = 1000000 / getMixerSchedulerPeriod();
      snprintf(statusText, 64, "%d Hz %" PRIu32 " Err", hz, telemetryErrors);
      status->setText(statusText);
    }
#endif
#if defined(MULTIMODULE)
    else if (isModuleMultimodule(module)) {
      name->setText("Multimodule");

      char statusText[64] = "";
      getMultiModuleStatus(module).getStatusString(statusText);
      status->setText(statusText);
    }
#endif
#if defined(PXX2)
    else if (isModulePXX2(module)) {

      // PXX2 Module
      name->setText(getPXX2ModuleName(reusableBuffer.hardwareAndSettings.modules[module]
                                      .information.modelID));

      std::string mod_ver;
      if (reusableBuffer.hardwareAndSettings.modules[module]
              .information.modelID) {

        mod_ver += getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module]
                          .information.hwVersion);
        mod_ver += " / ";
        mod_ver += getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module]
                          .information.swVersion);

        static const char *variants[] = {"FCC", "EU", "FLEX"};
        uint8_t variant = reusableBuffer.hardwareAndSettings.modules[module]
                              .information.variant - 1;
        if (variant < DIM(variants)) {
          mod_ver += " ";
          mod_ver += variants[variant];
        }
      }
      status->setText(mod_ver);

      // PXX2 Receivers
      std::string rx_n;
      std::string rx_ver;

      for (uint8_t receiver = 0; receiver < PXX2_MAX_RECEIVERS_PER_MODULE; receiver++) {
        if (reusableBuffer.hardwareAndSettings.modules[module]
                .receivers[receiver]
                .information.modelID) {

          if (!rx_ver.empty()){
            rx_n += "\n";
            rx_ver += "\n";
          }
          
          // Receiver model
          uint8_t modelId = reusableBuffer.hardwareAndSettings.modules[module]
                                .receivers[receiver]
                                .information.modelID;
          rx_n += getPXX2ReceiverName(modelId);

          // Receiver version
          rx_ver += getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module]
                               .receivers[receiver]
                               .information.hwVersion);
          rx_ver += " / ";
          rx_ver += getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module]
                               .receivers[receiver]
                               .information.swVersion);
        }
      }

      if (!rx_n.empty() && !rx_ver.empty()) {
        // unhide RX labels
        lv_obj_clear_flag(rx_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
        rx_name->setText(rx_n);
        rx_status->setText(rx_ver);
      } else {
        // hide RX labels
        lv_obj_add_flag(rx_line->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
    }
#endif
    else {
      name->setText(STR_NO_INFORMATION);
      status->setText("");
    }
  }

  void checkEvents() override
  {
    if (get_tmr10ms() >= reusableBuffer.hardwareAndSettings.updateTime) {
      // Query modules
      if (isModulePXX2(INTERNAL_MODULE) && IS_INTERNAL_MODULE_ON()) {
        moduleState[INTERNAL_MODULE].readModuleInformation(
            &reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE],
            PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
      }
      if (isModulePXX2(EXTERNAL_MODULE) && IS_EXTERNAL_MODULE_ON()) {
        moduleState[EXTERNAL_MODULE].readModuleInformation(
            &reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE],
            PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
      }
      reusableBuffer.hardwareAndSettings.updateTime = get_tmr10ms() + 500 /* 5s*/;
    }
    update();
    Dialog::checkEvents();
  }
};

RadioVersionPage::RadioVersionPage():
  PageTab(STR_MENUVERSION, ICON_RADIO_VERSION)
{
}

#if defined(PCBNV14)
extern uint32_t NV14internalModuleFwVersion;
extern const char* boardLcdType;
#endif

void RadioVersionPage::build(FormWindow * window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, lv_dpx(8));
  lv_obj_set_style_pad_all(window->getLvObj(), lv_dpx(8), 0);

  std::string nl("\n");
  std::string version;

  version += fw_stamp + nl;
  version += vers_stamp + nl;
  version += date_stamp + nl;
  version += time_stamp + nl;
  version += "OPTS: ";

  for (uint8_t i = 0; options[i]; i++) {
    if (i > 0) version += ", ";
    version += options[i];
  }

#if defined(PCBNV14) && !defined(SIMU)
  version += nl;
  version += "LCD: ";
  version += boardLcdType;

#if defined(AFHDS2)
  version += nl;
  version += "RF FW: ";
  sprintf(reusableBuffer.moduleSetup.msg, "%d.%d.%d",
          (int)((NV14internalModuleFwVersion >> 16) & 0xFF),
          (int)((NV14internalModuleFwVersion >> 8) & 0xFF),
          (int)(NV14internalModuleFwVersion & 0xFF));
  version += reusableBuffer.moduleSetup.msg;
#endif
#endif

  auto txt = new StaticText(window, rect_t{}, version, 0, COLOR_THEME_PRIMARY1);
  lv_obj_set_width(txt->getLvObj(), lv_pct(100));

  // Module and receivers versions
  auto btn = new TextButton(window, rect_t{}, STR_MODULES_RX_VERSION);
  btn->setPressHandler([=]() -> uint8_t {
    new VersionDialog(window);
    return 0;
  });
  lv_obj_set_width(btn->getLvObj(), lv_pct(100));
}
