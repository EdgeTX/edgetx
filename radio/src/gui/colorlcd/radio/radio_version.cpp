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

#include "fw_version.h"
#include "hal/module_port.h"
#include "libopenui.h"
#include "edgetx.h"
#include "options.h"
#include "etx_lv_theme.h"

#if defined(CROSSFIRE)
#include "mixer_scheduler.h"
#endif

#if defined(PXX2)
char* getVersion(char* str, PXX2Version version)
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
#endif

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(2),
                                     LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class VersionDialog : public BaseDialog
{
  Window* int_module_name_w;
  StaticText* int_name;
  Window* int_module_status_w;
  StaticText* int_status;

  Window* int_rx_name_w;
  StaticText* int_rx_name;
  Window* int_rx_status_w;
  StaticText* int_rx_status;

  Window* ext_module_name_w;
  StaticText* ext_name;
  Window* ext_module_status_w;
  StaticText* ext_status;

  Window* ext_rx_name_w;
  StaticText* ext_rx_name;
  Window* ext_rx_status_w;
  StaticText* ext_rx_status;

 public:
  VersionDialog() :
      BaseDialog(STR_MODULES_RX_VERSION, true)
  {
#if defined(PXX2)
    memclear(&reusableBuffer.hardwareAndSettings.modules,
             sizeof(reusableBuffer.hardwareAndSettings.modules));
    reusableBuffer.hardwareAndSettings.updateTime = get_tmr10ms();
#if defined(HARDWARE_INTERNAL_MODULE)
    // Query modules
    if (isModulePXX2(INTERNAL_MODULE) && modulePortPowered(INTERNAL_MODULE)) {
      moduleState[INTERNAL_MODULE].readModuleInformation(
          &reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE],
          PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
    }
#endif

    if (isModulePXX2(EXTERNAL_MODULE) && modulePortPowered(EXTERNAL_MODULE)) {
      moduleState[EXTERNAL_MODULE].readModuleInformation(
          &reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE],
          PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
    }
#endif

    // define grid layout
    FlexGridLayout grid(col_dsc, row_dsc);

    auto g = lv_group_get_default();
    lv_group_set_editing(g, true);

    lv_obj_add_flag(form->getLvObj(), LV_OBJ_FLAG_SCROLLABLE);
    lv_group_add_obj(g, form->getLvObj());

    // headline "Internal module"
    new StaticText(form, rect_t{}, STR_INTERNAL_MODULE);

    // Internal module name
    int_module_name_w = form->newLine(grid);
    new StaticText(int_module_name_w, rect_t{}, STR_MODULE);
    int_name = new StaticText(int_module_name_w, rect_t{}, "");

    // internal module status
    int_module_status_w = form->newLine(grid);
    new StaticText(int_module_status_w, rect_t{}, STR_STATUS);
    int_status = new StaticText(int_module_status_w, rect_t{}, "");
    int_module_status_w->hide();

    // internal receiver name
    int_rx_name_w = form->newLine(grid);
    new StaticText(int_rx_name_w, rect_t{}, STR_RECEIVER);
    int_rx_name =
        new StaticText(int_rx_name_w, rect_t{}, "");
    int_rx_name_w->hide();

    // internal receiver status
    int_rx_status_w = form->newLine(grid);
    new StaticText(int_rx_status_w, rect_t{}, STR_STATUS);
    int_rx_status =
        new StaticText(int_rx_status_w, rect_t{}, "");
    int_rx_status_w->hide();

    // headline "External module"
    new StaticText(form, rect_t{}, STR_EXTERNAL_MODULE);

    // external module name
    ext_module_name_w = form->newLine(grid);
    new StaticText(ext_module_name_w, rect_t{}, STR_MODULE);
    ext_name = new StaticText(ext_module_name_w, rect_t{}, "");

    // external module status
    ext_module_status_w = form->newLine(grid);
    new StaticText(ext_module_status_w, rect_t{}, STR_STATUS);
    ext_status = new StaticText(ext_module_status_w, rect_t{}, "");
    ext_module_status_w->hide();

    // external receiver name
    ext_rx_name_w = form->newLine(grid);
    new StaticText(ext_rx_name_w, rect_t{}, STR_RECEIVER);
    ext_rx_name =
        new StaticText(ext_rx_name_w, rect_t{}, "");
    ext_rx_name_w->hide();

    // external receiver status
    ext_rx_status_w = form->newLine(grid);
    new StaticText(ext_rx_status_w, rect_t{}, STR_STATUS);
    ext_rx_status =
        new StaticText(ext_rx_status_w, rect_t{}, "");
    ext_rx_status_w->hide();

    // content->setWidth(LCD_W * 0.8);
    update();
  }

  void update()
  {
#if defined(HARDWARE_INTERNAL_MODULE)
    updateModule(INTERNAL_MODULE, int_name, int_module_status_w, int_status,
                 int_rx_name_w, int_rx_name, int_rx_status_w, int_rx_status);
#endif
    updateModule(EXTERNAL_MODULE, ext_name, ext_module_status_w, ext_status,
                 ext_rx_name_w, ext_rx_name, ext_rx_status_w, ext_rx_status);
  }

  void updateModule(uint8_t module, StaticText* name, Window* module_status_w,
                    StaticText* status, Window* rx_name_w, StaticText* rx_name,
                    Window* rx_status_w, StaticText* rx_status)
  {
    // initialize module name with module selection made in model settings
    // initialize to module does not provide status
    // PXX2 will overwrite name
    // CRSF, MPM, NV14 and PXX2 will overwrite status
    name->setText(STR_MODULE_PROTOCOLS[g_model.moduleData[module].type]);
    module_status_w->hide();

#if defined(CROSSFIRE)
    // CRSF is able to provide status
    if (isModuleCrossfire(module)) {
      char statusText[64];

      auto hz = 1000000 / getMixerSchedulerPeriod();
      // snprintf(statusText, 64, "%d Hz %" PRIu32 " Err", hz, telemetryErrors);
      snprintf(statusText, 64, "%d Hz", hz);
      status->setText(statusText);
      snprintf(statusText, 64, "%s V%u.%u.%u", crossfireModuleStatus[module].name, crossfireModuleStatus[module].major, crossfireModuleStatus[module].minor, crossfireModuleStatus[module].revision);
      name->setText(statusText);
      module_status_w->show();
    }
#endif

#if defined(PCBNV14) && defined(AFHDS2)
    // NV14 AFHDS2A internal module is able to provide FW version
    extern uint32_t NV14internalModuleFwVersion;
    if (isModuleAFHDS2A(module)) {
      sprintf(reusableBuffer.moduleSetup.msg, "FW Ver %d.%d.%d",
              (int)((NV14internalModuleFwVersion >> 16) & 0xFF),
              (int)((NV14internalModuleFwVersion >> 8) & 0xFF),
              (int)(NV14internalModuleFwVersion & 0xFF));
      status->setText(reusableBuffer.moduleSetup.msg);
      module_status_w->show();
    }
#endif

#if defined(MULTIMODULE)
    // MPM is able to provide status
    if (isModuleMultimodule(module)) {
      char statusText[64];

      getMultiModuleStatus(module).getStatusString(statusText);
      status->setText(statusText);
      module_status_w->show();
    }
#endif

#if defined(PXX2)
    // PXX2 modules are able to provide status
    if (isModulePXX2(module)) {
      char tmp[20];

      // PXX2 module name
      name->setText(
          getPXX2ModuleName(reusableBuffer.hardwareAndSettings.modules[module]
                                .information.modelID));

      // PXX2 module status
      std::string mod_ver;
      if (reusableBuffer.hardwareAndSettings.modules[module]
              .information.modelID) {
        mod_ver +=
            getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module]
                                .information.hwVersion);
        mod_ver += " / ";
        mod_ver +=
            getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module]
                                .information.swVersion);

        static const char* variants[] = {"FCC", "EU", "FLEX"};
        uint8_t variant = reusableBuffer.hardwareAndSettings.modules[module]
                              .information.variant -
                          1;
        if (variant < DIM(variants)) {
          mod_ver += " ";
          mod_ver += variants[variant];
        }
      }
      status->setText(mod_ver);
      module_status_w->show();

      // PXX2 Receivers
      std::string rx_n;
      std::string rx_ver;

      for (uint8_t receiver = 0; receiver < PXX2_MAX_RECEIVERS_PER_MODULE;
           receiver++) {
        if (reusableBuffer.hardwareAndSettings.modules[module]
                .receivers[receiver]
                .information.modelID) {
          if (!rx_ver.empty()) {
            rx_n += "\n";
            rx_ver += "\n";
          }

          // Receiver model
          uint8_t modelId = reusableBuffer.hardwareAndSettings.modules[module]
                                .receivers[receiver]
                                .information.modelID;
          rx_n += getPXX2ReceiverName(modelId);

          // Receiver version
          rx_ver +=
              getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module]
                                  .receivers[receiver]
                                  .information.hwVersion);
          rx_ver += " / ";
          rx_ver +=
              getVersion(tmp, reusableBuffer.hardwareAndSettings.modules[module]
                                  .receivers[receiver]
                                  .information.swVersion);
        }
      }

      if (!rx_n.empty() && !rx_ver.empty()) {
        // unhide RX labels
        rx_name->setText(rx_n);
        rx_name_w->show();
        rx_status->setText(rx_ver);
        rx_status_w->show();
      } else {
        // hide RX labels
        rx_name_w->hide();
        rx_status_w->hide();
      }
    }
#endif
  }

#if defined(PXX2)
  void checkEvents() override
  {
    if (get_tmr10ms() >= reusableBuffer.hardwareAndSettings.updateTime) {
      // Query modules
#if defined(HARDWARE_INTERNAL_MODULE)
      if (isModulePXX2(INTERNAL_MODULE) && modulePortPowered(INTERNAL_MODULE)) {
        moduleState[INTERNAL_MODULE].readModuleInformation(
            &reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE],
            PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
      }
#endif
      if (isModulePXX2(EXTERNAL_MODULE) && modulePortPowered(EXTERNAL_MODULE)) {
        moduleState[EXTERNAL_MODULE].readModuleInformation(
            &reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE],
            PXX2_HW_INFO_TX_ID, PXX2_MAX_RECEIVERS_PER_MODULE - 1);
      }
      reusableBuffer.hardwareAndSettings.updateTime =
          get_tmr10ms() + 500 /* 5s*/;
    }
    update();
    BaseDialog::checkEvents();
  }
#endif
};

RadioVersionPage::RadioVersionPage() :
    PageTab(STR_MENUVERSION, ICON_RADIO_VERSION)
{
}

#if defined(PCBNV14) || defined(PCBPL18)
extern const char* boardLcdType;
#endif

void RadioVersionPage::build(Window* window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_MEDIUM);

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

#if defined(RADIO_T15)
  version += nl;
  version += "PCBREV: ";
  version += '0' + hardwareOptions.pcbrev;
#endif

#if (defined(PCBNV14) || defined(PCBPL18)) && !defined(SIMU)
  version += nl;
  version += "LCD: ";
  version += boardLcdType;
#endif

  auto txt = new StaticText(window, rect_t{}, version);
  lv_obj_set_width(txt->getLvObj(), lv_pct(100));

  // Module and receivers versions
  auto btn = new TextButton(window, rect_t{}, STR_MODULES_RX_VERSION);
  btn->setPressHandler([=]() -> uint8_t {
    new VersionDialog();
    return 0;
  });
  lv_obj_set_width(btn->getLvObj(), lv_pct(100));
}
