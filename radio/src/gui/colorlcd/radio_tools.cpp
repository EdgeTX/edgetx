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

struct LuaScript
{
  std::string path;
  std::string label;
  bool operator < (const LuaScript &a) { return label < a.label; }
};

inline bool LuaScript_compare_nocase(LuaScript first, LuaScript second)
{
  return strcasecmp(first.label.c_str(), second.label.c_str()) < 0;
}

class FormGridLayoutEx : public FormGridLayout
{
  public:
    using FormGridLayout::FormGridLayout;

    void setLineHeight(uint8_t lineHeight)
    {
      _lineHeight = lineHeight;
    }

    rect_t getFieldSlot(uint8_t count = 1, uint8_t index = 0) const
    {
      auto rect = FormGridLayout::getFieldSlot(count, index);
      rect.h = _lineHeight;
      return rect;
    }

    rect_t getLabelSlot(bool indent = false) const
    {
      auto rect = FormGridLayout::getLabelSlot(indent);
      rect.h = _lineHeight;
      return rect;
    }

    void nextLine(coord_t height=PAGE_LINE_HEIGHT)
    {
      if (height == PAGE_LINE_HEIGHT)
        height = _lineHeight;
      spacer(height + PAGE_LINE_SPACING);
    }

  protected:
    uint8_t _lineHeight = PAGE_LINE_HEIGHT;
};

#if defined(LUA)
void buildLuaUi(std::vector<LuaScript> luaScripts, FormWindow *window, FormGridLayoutEx &grid)
{
  for (auto luaScript : luaScripts) {
    auto txt =
        new StaticText(window, grid.getLabelSlot(), "lua", BUTTON_BACKGROUND,
                       COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
    auto b = new TextButton(
        window, grid.getFieldSlot(1), luaScript.label,
        [window, luaScript]() -> uint8_t {

          char toolPath[FF_MAX_LFN + 1];
          strncpy(toolPath, luaScript.path.c_str(), sizeof(toolPath)-1);
          *((char *)getBasename(toolPath)-1) = '\0';
          f_chdir(toolPath);

          luaExec(luaScript.path.c_str());
          StandaloneLuaWindow::instance()->attach(window);

          return 0;
        },
        OPAQUE);
    b->setBgColorHandler([=]() {
      return COLOR_THEME_PRIMARY2; 
    });
    b->setFocusHandler([=](bool focus) {
      if (focus) {
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED | VCENTERED);
      } else {
        txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
        txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
      }
      txt->invalidate();
    });
    grid.nextLine();
  }
}
#endif

void RadioToolsPage::rebuild(FormWindow * window)
{
  FormGridLayoutEx grid;
  grid.setLineHeight(PAGE_LINE_HEIGHT + 10);
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(100);

  window->clear();
  Window::clearFocus();

// LUA scripts in TOOLS
#if defined(LUA)
  FILINFO fno;
  DIR dir;

#if defined(CROSSFIRE)
//  if (isFileAvailable(SCRIPTS_TOOLS_PATH "/CROSSFIRE/crossfire.lua"))
//    addRadioScriptTool(index++, SCRIPTS_TOOLS_PATH "/CROSSFIRE/crossfire.lua");
#endif

  FRESULT res = f_opendir(&dir, SCRIPTS_TOOLS_PATH);
  if (res == FR_OK) {
    std::vector<LuaScript> luaScripts;  // gather them all and then create UI after sort

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

        luaScripts.emplace_back(LuaScript{ path, label });
      }
    }

    std::sort(luaScripts.begin(), luaScripts.end(), LuaScript_compare_nocase);
    buildLuaUi(luaScripts, window, grid);
  }
#endif

#if defined(INTERNAL_MODULE_PXX2)
  // PXX2 modules tools
  if (isPXX2ModuleOptionAvailable(
          reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE]
              .information.modelID,
          MODULE_OPTION_SPECTRUM_ANALYSER)) {
    auto txt =
        new StaticText(window, grid.getLabelSlot(), "access", BUTTON_BACKGROUND,
                       COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
    auto b = new TextButton(
        window, grid.getFieldSlot(1), STR_SPECTRUM_ANALYSER_INT,
        [=]() -> uint8_t {
          new RadioSpectrumAnalyser(INTERNAL_MODULE);
          return 0;
        },
        OPAQUE);
    b->setBgColorHandler([=]() -> LcdFlags { return COLOR_THEME_PRIMARY2; });
    b->setFocusHandler([=](bool focus) {
      if (focus) {
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED | VCENTERED);
      } else {
        txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
        txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
      }
      txt->invalidate();
    });
    grid.nextLine();
  }

  if (isPXX2ModuleOptionAvailable(
          reusableBuffer.hardwareAndSettings.modules[INTERNAL_MODULE]
              .information.modelID,
          MODULE_OPTION_POWER_METER)) {
    auto txt =
        new StaticText(window, grid.getLabelSlot(), "access", BUTTON_BACKGROUND,
                       COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
    auto b = new TextButton(
        window, grid.getFieldSlot(1), STR_POWER_METER_INT,
        [=]() -> uint8_t {
          //        new RadioPowerMeter(INTERNAL_MODULE);
          return 0;
        },
        OPAQUE);
    b->setBgColorHandler([=]() -> LcdFlags { return COLOR_THEME_PRIMARY2; });
    b->setFocusHandler([=](bool focus) {
      if (focus) {
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED | VCENTERED);
      } else {
        txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
        txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
      }
      txt->invalidate();
    });
    grid.nextLine();
  }
#endif
#if defined(INTERNAL_MODULE_MULTI)
  {
    auto txt =
        new StaticText(window, grid.getLabelSlot(), "multi", BUTTON_BACKGROUND,
                       COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
    auto b = new TextButton(
        window, grid.getFieldSlot(1), STR_SPECTRUM_ANALYSER_INT,
        [=]() -> uint8_t {
          new RadioSpectrumAnalyser(INTERNAL_MODULE);
          return 0;
        },
        OPAQUE);
    b->setBgColorHandler([=]() -> LcdFlags { return COLOR_THEME_PRIMARY2; });
    b->setFocusHandler([=](bool focus) {
      if (focus) {
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED | VCENTERED);
      } else {
        txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
        txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
      }
      txt->invalidate();
    });
    grid.nextLine();
  }
#endif
#if defined(PXX2)|| defined(MULTIMODULE)
  if (isPXX2ModuleOptionAvailable(
          reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE]
              .information.modelID,
          MODULE_OPTION_SPECTRUM_ANALYSER) ||
      isModuleMultimodule(EXTERNAL_MODULE)) {
    auto txt = new StaticText(
        window, grid.getLabelSlot(),
        isModuleMultimodule(EXTERNAL_MODULE) ? "multi" : "access",
        BUTTON_BACKGROUND, COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
    auto b = new TextButton(
        window, grid.getFieldSlot(1), STR_SPECTRUM_ANALYSER_EXT,
        [=]() -> uint8_t {
          new RadioSpectrumAnalyser(EXTERNAL_MODULE);
          return 0;
        },
        OPAQUE);
    b->setBgColorHandler([=]() -> LcdFlags { return COLOR_THEME_PRIMARY2; });
    b->setFocusHandler([=](bool focus) {
      if (focus) {
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED | VCENTERED);
      } else {
        txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
        txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
      }
      txt->invalidate();
    });
    grid.nextLine();
  }
#endif
#if defined(PXX2)
#if 0 // disabled Power Meter: not yet implemented
  if (isPXX2ModuleOptionAvailable(
          reusableBuffer.hardwareAndSettings.modules[EXTERNAL_MODULE]
              .information.modelID,
          MODULE_OPTION_POWER_METER)) {
    auto txt =
        new StaticText(window, grid.getLabelSlot(), "access", BUTTON_BACKGROUND,
                       COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
    auto b = new TextButton(
        window, grid.getFieldSlot(1), STR_POWER_METER_EXT,
        [=]() -> uint8_t {
          //        new RadioPowerMeter(EXTERNAL_MODULE);
          return 0;
        },
        OPAQUE);
    b->setBgColorHandler([=]() -> LcdFlags { return COLOR_THEME_PRIMARY2; });
    b->setFocusHandler([=](bool focus) {
      if (focus) {
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED | VCENTERED);
      } else {
        txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
        txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
      }
      txt->invalidate();
    });
    grid.nextLine();
  }
#endif
#endif

#if defined(GHOST)
  if (isModuleGhost(EXTERNAL_MODULE)) {
    auto txt = new StaticText(window, grid.getLabelSlot(), "ghost",
                              BUTTON_BACKGROUND, CENTERED | VCENTERED);
    auto b = new TextButton(
        window, grid.getFieldSlot(1), "Ghost module config",
        [=]() -> uint8_t {
          new RadioGhostModuleConfig(EXTERNAL_MODULE);
          return 0;
        },
        OPAQUE);
    b->setBgColorHandler([=]() -> LcdFlags { return COLOR_THEME_PRIMARY2; });
    b->setFocusHandler([=](bool focus) {
      if (focus) {
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED | VCENTERED);
      } else {
        txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
        txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED | VCENTERED);
      }
      txt->invalidate();
    });
    grid.nextLine();
  }
#endif

  window->setInnerHeight(grid.getWindowHeight());
}
