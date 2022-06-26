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

#include "opentx.h"

#include "model_mixer_scripts.h"
#include "dataconstants.h"
#include "lua/lua_api.h"

#include "translations.h"
#include "menus.h"
#include "libopenui.h"

#include <string.h>

#define SET_DIRTY() storageDirty(EE_MODEL)

class ScriptEditWindow : public Page {
  public:
    explicit ScriptEditWindow(uint8_t idx) :
      Page(ICON_MODEL_LUA_SCRIPTS),
      idx(idx)
    {
      buildBody(&body);
      buildHeader(&header);
    }

  protected:
    const uint8_t idx;
    bool update = false;

    void checkEvents() override
    {
      if ((update) && (luaState == INTERPRETER_RUNNING)) {
        TRACE("rebuilding ScriptEditWindow...");
        rebuildBody(&body);
        update = false;
      }
      // note: 'update' is set from Page::checkEvents()
      Page::checkEvents();
    }

    void buildHeader(Window * window)
    {
      new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                     STR_MENUCUSTOMSCRIPTS, 0, COLOR_THEME_PRIMARY2);
      new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                      LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                     std::string("LUA") + std::to_string(idx + 1), 0, COLOR_THEME_PRIMARY2);
    }

    void buildBody(FormWindow * window, bool focusScript = false)
    {
      FormGridLayout grid;
      grid.spacer(PAGE_PADDING);

      // the general pattern seems to be using capture-by-value for the closures: so need to copy the pointers, not the objects
      ScriptData* const sd = &(g_model.scriptsData[idx]);
      ScriptInputsOutputs* const sio = &(scriptInputsOutputs[idx]);
      bool* const updatePtr = &update; 

      new StaticText(window, grid.getLabelSlot(), STR_SCRIPT, 0, COLOR_THEME_PRIMARY1);
      auto fc = new FileChoice(
          window, grid.getFieldSlot(), SCRIPTS_MIXES_PATH, SCRIPTS_EXT,
          LEN_SCRIPT_FILENAME,
          [=]() { return stringFromNtString(sd->file); },
          [=](std::string newValue) {
             clearStruct(*sd);
             clearStruct(*sio);
             if (!newValue.empty()) {
                 copyToUnTerminated(sd->file, newValue);
             }
            storageDirty(EE_MODEL);
            LUA_LOAD_MODEL_SCRIPT(idx); // async reload ...
            *updatePtr = true; // TODO: sets update==true and afterwards calls rebuildBody() via checkEvents()
          }, true);
      grid.nextLine();

      // Custom name
      new StaticText(window, grid.getLabelSlot(), STR_NAME, 0, COLOR_THEME_PRIMARY1);
      new ModelTextEdit(window, grid.getFieldSlot(), sd->name, sizeof(sd->name));
      grid.nextLine();

      if (sio->inputsCount > 0) {
        new Subtitle(window, grid.getLineSlot(), STR_INPUTS, 0, COLOR_THEME_PRIMARY1);
        grid.nextLine();

        auto gInputs = new FormGroup(window, grid.getFieldSlot(), FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
        GridLayout inputsGrid(gInputs);

        for (int i = 0; i < sio->inputsCount; i++) {
          ScriptInput& si = sio->inputs[i];
          new StaticText(window, grid.getLabelSlot(true), si.name, 0, COLOR_THEME_PRIMARY1);
          grid.nextLine();
          if (si.type == INPUT_TYPE_VALUE) {
            (new NumberEdit(gInputs, inputsGrid.getSlot(), si.min, si.max,
                            GET_SET_WITH_OFFSET(sd->inputs[i].value, si.def)))->setDefault(si.def);
          } else {
            new SourceChoice(gInputs, inputsGrid.getSlot(), 0,
                             MIXSRC_LAST_TELEM,
                             GET_SET_DEFAULT(sd->inputs[i].source));
          }
          inputsGrid.nextLine();
        }
        gInputs->setHeight(inputsGrid.getWindowHeight());
      }

      if (sio->outputsCount > 0) {
        new Subtitle(window, grid.getLabelSlot(), STR_OUTPUTS, 0, COLOR_THEME_PRIMARY1);
        grid.nextLine();

        auto gOutputs =
            new FormGroup(window, grid.getLineSlot(),
                          FORM_BORDER_FOCUS_ONLY | PAINT_CHILDREN_FIRST);
        FormGridLayout outputsGrid(gOutputs->width());

        for (int i = 0; i < sio->outputsCount; i++) {
          ScriptOutput* so = &(sio->outputs[i]);
          new DynamicText(gOutputs, outputsGrid.getLabelSlot(), [=]() {
            char s[16];
            getSourceString(s, MIXSRC_FIRST_LUA + (idx * MAX_SCRIPT_OUTPUTS) + i);
            return std::string(s, sizeof(s) - 1);
          }, COLOR_THEME_PRIMARY1);
          new DynamicNumber<int>(gOutputs, outputsGrid.getFieldSlot(), [=]() { return calcRESXto1000(so->value); }, COLOR_THEME_PRIMARY1);

          outputsGrid.nextLine();
        }
        gOutputs->setHeight(outputsGrid.getWindowHeight());
        grid.addWindow(gOutputs);
      }
    }
    
    void rebuildBody(FormWindow * window)
    {
      auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());  
      window->clear();
      buildBody(window);
      lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
    }
};

class ScriptLineButton : public Button
{
 public:
  ScriptLineButton(FormGroup* parent, const rect_t& rect,
                   const ScriptData& scriptData,
                   const ScriptInternalData* runtimeData) :
      Button(parent, rect), scriptData(scriptData), runtimeData(runtimeData)
  {
  }

  void paint(BitmapBuffer* dc) override
  {
    LcdFlags textColor = COLOR_THEME_SECONDARY1;
    LcdFlags bgColor = COLOR_THEME_PRIMARY2;

    dc->drawSolidFilledRect(0, 0, width(), height(), bgColor);

    if (runtimeData) {
      coord_t x = 2*FIELD_PADDING_LEFT;
      coord_t y = FIELD_PADDING_TOP;

      x = dc->drawSizedText(x, y, scriptData.name, sizeof(scriptData.name), textColor);
      x += 4*FIELD_PADDING_LEFT;

      dc->drawSizedText(x, y, scriptData.file, sizeof(scriptData.file), textColor);

      x = width() - 2*FIELD_PADDING_LEFT;
      y = FIELD_PADDING_TOP;
      textColor |= RIGHT;
      
      // TODO: runtimeData->instructions has no value 
      switch (runtimeData->state) {
        case SCRIPT_SYNTAX_ERROR:
          dc->drawSizedText(x, y, STR_SCRIPT_ERROR, strlen(STR_SCRIPT_ERROR), textColor);
          break;
      case SCRIPT_NOFILE:
          dc->drawSizedText(x, y, STR_NEEDS_FILE, strlen(STR_NEEDS_FILE), textColor);
          break;
      case SCRIPT_OK:
          dc->drawSizedText(x, y, "-", 1, textColor);
          break;
      default:
          break;
      }
    }

    // bounding rect
    if (hasFocus())
      dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_FOCUS);
    else
      dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR_THEME_SECONDARY2);
  }

 protected:
  const ScriptData&         scriptData;
  const ScriptInternalData* runtimeData;
};

ModelMixerScriptsPage::ModelMixerScriptsPage() :
  PageTab(STR_MENUCUSTOMSCRIPTS, ICON_MODEL_LUA_SCRIPTS)
{
}

void ModelMixerScriptsPage::rebuild(FormWindow * window, int8_t focusIdx)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());  
  window->clear();
  build(window, focusIdx);
  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void ModelMixerScriptsPage::build(FormWindow * window, int8_t focusIdx)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(66);
  window->padAll(0);

  int8_t scriptIdx = 0;
  for (int8_t idx = 0; idx < MAX_SCRIPTS; idx++) {

    ScriptInternalData* runtimeData = nullptr;
    ScriptData* const sd = &(g_model.scriptsData[idx]);
    ScriptInputsOutputs* const sio = &(scriptInputsOutputs[idx]);

    if (sd->file[0] != '\0') {
      runtimeData = &(scriptInternalData[scriptIdx++]);
    }
    
    // LUAx label
    auto const txt = new StaticText(window, grid.getLabelSlot(),
                              std::string("LUA") + std::to_string(idx + 1),
                              BUTTON_BACKGROUND, COLOR_THEME_PRIMARY1 | CENTERED);
    
    Button* const button = new ScriptLineButton(window, grid.getFieldSlot(), *sd, runtimeData);

    button->setPressHandler([=]() -> uint8_t {
      Menu* const menu = new Menu(window);
      menu->addLine(STR_EDIT, [=]() { editLine(window, idx); });

      if (runtimeData != nullptr) {
        menu->addLine(STR_DELETE, [=]() {
            clearStruct(*sd);
            clearStruct(*sio);
          LUA_LOAD_MODEL_SCRIPTS();
          storageDirty(EE_MODEL);
          rebuild(window, idx);
        });
        return 0;
      }
      return 0;
    });

    button->setFocusHandler([=](bool focus) {
      if (focus) {
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED);
      } else {
        txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
        txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED);
      }
      txt->invalidate();
    });

    txt->setHeight(button->height());
    grid.spacer(button->height() + 5);
  }

  grid.nextLine();
}

void ModelMixerScriptsPage::editLine(FormWindow * window, uint8_t idx)
{
  Window * editWindow = new ScriptEditWindow(idx);
  editWindow->setCloseHandler([=]() {
    rebuild(window, idx);
  });
}
