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

#include <algorithm>
#include "radio_tools.h"
#include "radio_spectrum_analyser.h"
#include "radio_ghost_module_config.h"
#include "opentx.h"
#include "libopenui.h"
#include "lua/lua_api.h"
#include "standalone_lua.h"

extern uint8_t g_moduleIdx;

RadioToolsPage::RadioToolsPage():
  PageTab(STR_MENUTOOLS, ICON_RADIO_TOOLS)
{
}

void RadioToolsPage::build(FormWindow * window)
{
  this->window = window;

  memclear(&reusableBuffer.radioTools, sizeof(reusableBuffer.radioTools));
  waiting = 0;

#if defined(PXX2)
  for (uint8_t module = 0; module < NUM_MODULES; module++) {
    if (isModulePXX2(module) && (module == INTERNAL_MODULE ? IS_INTERNAL_MODULE_ON() : IS_EXTERNAL_MODULE_ON())) {
      waiting |= (1 << module);
      moduleState[module].readModuleInformation(&reusableBuffer.radioTools.modules[module], PXX2_HW_INFO_TX_ID, PXX2_HW_INFO_TX_ID);
    }
  }
#endif

  rebuild(window);
}

void RadioToolsPage::checkEvents()
{
  bool refresh = false;

  for (uint8_t module = 0; module < NUM_MODULES; module++) {
    if ((waiting & (1 << module)) && reusableBuffer.radioTools.modules[module].information.modelID) {
      waiting &= ~(1 << module);
      refresh = true;
    }
  }

  if (refresh) {
    rebuild(window);
  }

  PageTab::checkEvents();
}

typedef void (*ToolExec)(Window* parent, const std::string& path);

struct ToolEntry
{
  std::string label;
  std::string path;
  ToolExec    exec;
};

inline bool tool_compare_nocase(const ToolEntry& first, const ToolEntry& second)
{
  return strcasecmp(first.label.c_str(), second.label.c_str()) < 0;
}

#if defined(LUA)
static void run_lua_tool(Window* parent, const std::string& path)
{
  char toolPath[FF_MAX_LFN + 1];
  strncpy(toolPath, path.c_str(), sizeof(toolPath)-1);
  *((char *)getBasename(toolPath)-1) = '\0';
  f_chdir(toolPath);

  luaExec(path.c_str());
  auto lua_win = StandaloneLuaWindow::instance();
  lua_win->attach();
}

// LUA scripts in TOOLS
static void scanLuaTools(std::list<ToolEntry>& scripts)
{
  FILINFO fno;
  DIR dir;

  FRESULT res = f_opendir(&dir, SCRIPTS_TOOLS_PATH);
  if (res == FR_OK) {
    for (;;) {
      TCHAR path[FF_MAX_LFN+1] = SCRIPTS_TOOLS_PATH "/";
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      if (fno.fattrib & AM_DIR) continue;            /* Skip subfolders */
      if (fno.fattrib & AM_HID) continue;            /* Skip hidden files */
      if (fno.fattrib & AM_SYS) continue;            /* Skip system files */

      strcat(path, fno.fname);
      if (isRadioScriptTool(fno.fname)) {
        char toolName[RADIO_TOOL_NAME_MAXLEN + 1] = {0};
        const char * label;
        char * ext = (char *)getFileExtension(path);
        if (readToolName(toolName, path)) {
          label = toolName;
        }
        else {
          *ext = '\0';
          label = getBasename(path);
        }

        scripts.emplace_back(ToolEntry{ label, path, run_lua_tool });
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

static void run_spektrum_ext(Window* parent, const std::string&)
{
  new RadioSpectrumAnalyser(EXTERNAL_MODULE);
}
#endif // defined(PXX2) || defined(MULTIMODULE)

#if defined(INTERNAL_MODULE_PXX2)
static void run_pxx2_power(Window* parent, const std::string&)
{
#if 0 // disabled Power Meter: not yet implemented
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
    setWidth(LV_DPI_DEF);
    setHeight(LV_DPI_DEF / 2);

    lv_obj_set_width(label, lv_pct(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
  }
};

void RadioToolsPage::rebuild(FormWindow * window)
{
  window->clear();

  std::list<ToolEntry> tools;

#if defined(PXX2)
  auto hwSettings = &reusableBuffer.hardwareAndSettings;

#if defined(INTERNAL_MODULE_PXX2)
  auto intHwSettings = &hwSettings->modules[INTERNAL_MODULE];
  // PXX2 modules tools
  if (isPXX2ModuleOptionAvailable(
          intHwSettings->information.modelID,
          MODULE_OPTION_SPECTRUM_ANALYSER)) {
    tools.emplace_back(ToolEntry{ STR_SPECTRUM_ANALYSER_INT, {}, run_spektrum_int });
  }
  if (isPXX2ModuleOptionAvailable(
          intHwSettings->information.modelID,
          MODULE_OPTION_POWER_METER)) {
    tools.emplace_back(ToolEntry{ STR_POWER_METER_INT, {}, run_pxx2_power });
  }
#endif

  auto extHwSettings = &hwSettings->modules[EXTERNAL_MODULE];
  if (isPXX2ModuleOptionAvailable(
          extHwSettings->information.modelID,
          MODULE_OPTION_SPECTRUM_ANALYSER)) {
    tools.emplace_back(ToolEntry{ STR_SPECTRUM_ANALYSER_EXT, {}, run_spektrum_ext });
  }
#endif // defined(PXX2)

#if defined(HARDWARE_INTERNAL_MODULE) && defined(MULTIMODULE)
  if (g_eeGeneral.internalModule == MODULE_TYPE_MULTIMODULE) {
    tools.emplace_back(ToolEntry{ STR_SPECTRUM_ANALYSER_INT, {}, run_spektrum_int });
  }
#endif

#if defined(PXX2)|| defined(MULTIMODULE)
  if (isModuleMultimodule(EXTERNAL_MODULE)) {
    tools.emplace_back(ToolEntry{ STR_SPECTRUM_ANALYSER_EXT, {}, run_spektrum_ext });
  }
#endif

#if defined(GHOST)
  if (isModuleGhost(EXTERNAL_MODULE)) {
    tools.emplace_back(ToolEntry{ "Ghost module config", {}, run_ghost_config });
  }
#endif

#if defined(LUA)
  scanLuaTools(tools);
#endif

  tools.sort(tool_compare_nocase);

  window->setFlexLayout(LV_FLEX_FLOW_ROW_WRAP, lv_dpx(8));
  window->padRow(lv_dpx(8));
  
  for (const auto& tool : tools) {
    new ToolButton(window, tool);
  }
}
