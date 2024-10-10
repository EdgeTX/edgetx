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

#include "radio_tools.h"

#include <algorithm>

#include "hal/module_port.h"
#include "libopenui.h"
#include "lua/lua_api.h"
#include "edgetx.h"
#include "radio_ghost_module_config.h"
#include "radio_spectrum_analyser.h"
#include "standalone_lua.h"
#include "etx_lv_theme.h"

extern uint8_t g_moduleIdx;

RadioToolsPage::RadioToolsPage() : PageTab(STR_MENUTOOLS, ICON_RADIO_TOOLS) {}

void RadioToolsPage::build(Window* window)
{
  this->window = window;

  memclear(&reusableBuffer.radioTools, sizeof(reusableBuffer.radioTools));
  waiting = 0;

#if defined(PXX2)
  for (uint8_t module = 0; module < NUM_MODULES; module++) {
    if (isModulePXX2(module) &&
        (module == INTERNAL_MODULE ? modulePortPowered(INTERNAL_MODULE)
                                   : modulePortPowered(EXTERNAL_MODULE))) {
      waiting |= (1 << module);
      moduleState[module].readModuleInformation(
          &reusableBuffer.radioTools.modules[module], PXX2_HW_INFO_TX_ID,
          PXX2_HW_INFO_TX_ID);
    }
  }
#endif

  rebuild(window);
}

void RadioToolsPage::checkEvents()
{
#if defined(PXX2)
  bool refresh = false;

  for (uint8_t module = 0; module < NUM_MODULES; module++) {
    if ((waiting & (1 << module)) &&
        reusableBuffer.radioTools.modules[module].information.modelID) {
      waiting &= ~(1 << module);
      refresh = true;
    }
  }

  if (refresh) {
    rebuild(window);
  }
#endif

  PageTab::checkEvents();
}

typedef void (*ToolExec)(Window* parent, const std::string& path);

struct ToolEntry {
  std::string label;
  std::string path;
  ToolExec exec;
};

inline bool tool_compare_nocase(const ToolEntry& first, const ToolEntry& second)
{
  return strcasecmp(first.label.c_str(), second.label.c_str()) < 0;
}

#if defined(LUA)
static void run_lua_tool(Window* parent, const std::string& path)
{
  char toolPath[FF_MAX_LFN + 1];
  strncpy(toolPath, path.c_str(), sizeof(toolPath) - 1);
  *((char*)getBasename(toolPath) - 1) = '\0';
  f_chdir(toolPath);

  luaExecStandalone(path.c_str());
}

// LUA scripts in TOOLS
static void scanLuaTools(std::list<ToolEntry>& scripts)
{
  FILINFO fno;
  DIR dir;

  FRESULT res = f_opendir(&dir, SCRIPTS_TOOLS_PATH);
  if (res == FR_OK) {
    for (;;) {
      TCHAR path[FF_MAX_LFN + 1] = SCRIPTS_TOOLS_PATH "/";
      res = f_readdir(&dir, &fno); /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0)
        break; /* Break on error or end of dir */
      if (fno.fattrib & (AM_DIR | AM_HID | AM_SYS))
        continue;  // skip subfolders, hidden files and system files
      if (fno.fname[0] == '.') continue; /* Ignore UNIX hidden files */

      strcat(path, fno.fname);
      if (isRadioScriptTool(fno.fname)) {
        char toolName[RADIO_TOOL_NAME_MAXLEN + 1] = {0};
        const char* label;
        char* ext = (char*)getFileExtension(path);
        if (readToolName(toolName, path)) {
          label = toolName;
        } else {
          *ext = '\0';
          label = getBasename(path);
        }

        scripts.emplace_back(ToolEntry{label, path, run_lua_tool});
      }
    }
  }
}
#endif

#if defined(PXX2) || defined(MULTIMODULE)

#if defined(HARDWARE_INTERNAL_MODULE)
static void run_spektrum_int(Window* parent, const std::string&)
{
  new RadioSpectrumAnalyser(INTERNAL_MODULE);
}
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
static void run_spektrum_ext(Window* parent, const std::string&)
{
  new RadioSpectrumAnalyser(EXTERNAL_MODULE);
}
#endif
#endif  // defined(PXX2) || defined(MULTIMODULE)

#if defined(INTERNAL_MODULE_PXX2)
static void run_pxx2_power(Window* parent, const std::string&)
{
#if 0  // disabled Power Meter: not yet implemented
  new RadioPowerMeter(INTERNAL_MODULE);
#endif
}
#endif

#if defined(GHOST)
static void run_ghost_config(Window* parent, const std::string&)
{
  new RadioGhostModuleConfig(EXTERNAL_MODULE);
}
#endif

struct ToolButton : public TextButton {
  ToolButton(Window* parent, const ToolEntry& tool) :
      TextButton(parent, rect_t{}, tool.label, [=]() {
        tool.exec(parent, tool.path);
        return 0;
      })
  {
    setWidth(TOOLS_BTN_W);
    setHeight(TOOLS_BTN_H);

    lv_obj_set_width(label, lv_pct(100));
    etx_obj_add_style(label, styles->text_align_center, LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  }

  static LAYOUT_VAL(TOOLS_BTN_W, (LCD_W - 24) / 3, (LCD_W - 18) / 2)
  static LAYOUT_VAL(TOOLS_BTN_H, 48, 48)
};

void RadioToolsPage::rebuild(Window* window)
{
  window->clear();

  std::list<ToolEntry> tools;

#if defined(HARDWARE_INTERNAL_MODULE)
  bool intSpecAnalyser = false;
#endif
#if defined(HARDWARE_EXTERNAL_MODULE)
  bool extSpecAnalyser = false;
#endif

#if defined(PXX2)
  auto hwSettings = &reusableBuffer.hardwareAndSettings;

#if defined(INTERNAL_MODULE_PXX2)
  auto intHwSettings = &hwSettings->modules[INTERNAL_MODULE];
  // PXX2 modules tools
  if (isPXX2ModuleOptionAvailable(intHwSettings->information.modelID,
                                  MODULE_OPTION_SPECTRUM_ANALYSER)) {
    intSpecAnalyser = true;
  }
  if (isPXX2ModuleOptionAvailable(intHwSettings->information.modelID,
                                  MODULE_OPTION_POWER_METER)) {
    tools.emplace_back(ToolEntry{STR_POWER_METER_INT, {}, run_pxx2_power});
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  auto extHwSettings = &hwSettings->modules[EXTERNAL_MODULE];
  if (isPXX2ModuleOptionAvailable(extHwSettings->information.modelID,
                                  MODULE_OPTION_SPECTRUM_ANALYSER)) {
    extSpecAnalyser = true;
  }
#endif
#endif  // defined(PXX2)

#if defined(HARDWARE_INTERNAL_MODULE) && defined(MULTIMODULE)
  if (g_eeGeneral.internalModule == MODULE_TYPE_MULTIMODULE) {
    intSpecAnalyser = true;
  }
#endif

#if defined(HARDWARE_EXTERNAL_MODULE) && (defined(PXX2) || defined(MULTIMODULE))
  if (isModuleMultimodule(EXTERNAL_MODULE)) {
    extSpecAnalyser = true;
  }
#endif

#if defined(HARDWARE_INTERNAL_MODULE)
  if (intSpecAnalyser)
    tools.emplace_back(ToolEntry{STR_SPECTRUM_ANALYSER_INT, {}, run_spektrum_int});
#endif

#if defined(HARDWARE_EXTERNAL_MODULE)
  if (extSpecAnalyser)
    tools.emplace_back(ToolEntry{STR_SPECTRUM_ANALYSER_EXT, {}, run_spektrum_ext});
#endif

#if defined(GHOST)
  if (isModuleGhost(EXTERNAL_MODULE)) {
    tools.emplace_back(ToolEntry{"Ghost module config", {}, run_ghost_config});
  }
#endif

#if defined(LUA)
  scanLuaTools(tools);
#endif

  tools.sort(tool_compare_nocase);

  window->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, PAD_MEDIUM);

  for (const auto& tool : tools) {
    new ToolButton(window, tool);
  }
}
