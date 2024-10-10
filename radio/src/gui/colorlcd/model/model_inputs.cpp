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

#include "model_inputs.h"

#include <algorithm>

#include "hal/adc_driver.h"
#include "input_edit.h"
#include "edgetx.h"
#include "tasks/mixer_task.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

uint8_t getExposCount()
{
  uint8_t count = 0;
  uint8_t ch;

  for (int i = MAX_EXPOS - 1; i >= 0; i--) {
    ch = EXPO_VALID(expoAddress(i));
    if (ch != 0) {
      count++;
    }
  }
  return count;
}

// TODO: these functions need to be added to the generic API
//       used by all radios, and be removed from UI code
//
void copyExpo(uint8_t source, uint8_t dest, uint8_t input)
{
  mixerTaskStop();
  ExpoData sourceExpo;
  memcpy(&sourceExpo, expoAddress(source), sizeof(ExpoData));
  ExpoData* expo = expoAddress(dest);
  size_t trailingExpos = MAX_EXPOS - (dest + 1);
  memmove(expo + 1, expo, trailingExpos * sizeof(ExpoData));
  memcpy(expo, &sourceExpo, sizeof(ExpoData));
  expo->chn = input;
  mixerTaskStart();
  storageDirty(EE_MODEL);
}

void deleteExpo(uint8_t idx)
{
  mixerTaskStop();
  ExpoData* expo = expoAddress(idx);
  int input = expo->chn;
  memmove(expo, expo + 1, (MAX_EXPOS - (idx + 1)) * sizeof(ExpoData));
  memclear(&g_model.expoData[MAX_EXPOS - 1], sizeof(ExpoData));
  if (!isInputAvailable(input)) {
    memclear(&g_model.inputNames[input], LEN_INPUT_NAME);
  }
  mixerTaskStart();
  storageDirty(EE_MODEL);
}

// TODO port: avoid global s_currCh on ARM boards (as done here)...
int8_t s_currCh;

void insertExpo(uint8_t idx, uint8_t input)
{
  mixerTaskStop();
  ExpoData* expo = expoAddress(idx);
  memmove(expo + 1, expo, (MAX_EXPOS - (idx + 1)) * sizeof(ExpoData));
  memclear(expo, sizeof(ExpoData));
  if (input >= adcGetMaxInputs(ADC_INPUT_MAIN)) {
    expo->srcRaw = MIXSRC_FIRST_STICK + input;
  } else {
    expo->srcRaw = MIXSRC_FIRST_STICK + inputMappingChannelOrder(input);
  }
  expo->curve.type = CURVE_REF_EXPO;
  expo->mode = 3;  // pos+neg
  expo->chn = input;
  expo->weight = 100;
  mixerTaskStart();
  storageDirty(EE_MODEL);
}

class InputLineButton : public InputMixButtonBase
{
 public:
  InputLineButton(Window* parent, uint8_t index) :
    InputMixButtonBase(parent, index)
  {
    check(isActive());
  }

  void refresh() override
  {
    const ExpoData& line = g_model.expoData[index];
    setWeight(line.weight, -100, 100);
    setSource(line.srcRaw);

    char tmp_str[64];
    size_t maxlen = sizeof(tmp_str);

    char* s = tmp_str;
    *s = '\0';

    if (line.name[0]) {
      int cnt =
          lv_snprintf(s, maxlen, "%.*s ", (int)sizeof(line.name), line.name);
      if ((size_t)cnt >= maxlen)
        maxlen = 0;
      else {
        maxlen -= cnt;
        s += cnt;
      }
    }

    if (line.swtch) {
      char* sw_pos = getSwitchPositionName(line.swtch);
      int cnt = lv_snprintf(s, maxlen, "%s ", sw_pos);
      if ((size_t)cnt >= maxlen)
        maxlen = 0;
      else {
        maxlen -= cnt;
        s += cnt;
      }
    }

    if (line.curve.value != 0) {
      getCurveRefString(s, maxlen, line.curve);
      int cnt = strnlen(s, maxlen);
      if ((size_t)cnt >= maxlen)
        maxlen = 0;
      else {
        maxlen -= cnt;
        s += cnt;
      }
    }

    setOpts(tmp_str);

    setFlightModes(line.flightModes);
  }

  void updatePos(coord_t x, coord_t y) override
  {
    setPos(x, y);
  }

  void swapLvglGroup(InputMixButtonBase* line2) override
  {
    InputLineButton* swapWith = (InputLineButton*)line2;

    // Swap elements (focus + line list)
    lv_obj_t* obj1 = getLvObj();
    lv_obj_t* obj2 = swapWith->getLvObj();
    if (lv_obj_get_parent(obj1) == lv_obj_get_parent(obj2)) {
      // same input group: swap obj + focus group
      lv_obj_swap(obj1, obj2);
    } else {
      // different input group: swap only focus group
      lv_group_swap_obj(obj1, obj2);
    }
  }

 protected:
  bool isActive() const override { return isExpoActive(index); }
};

class InputGroup : public InputMixGroupBase
{
 public:
  InputGroup(Window* parent, mixsrc_t idx) :
    InputMixGroupBase(parent, idx)
  {
    adjustHeight();

    lv_obj_set_pos(label, 2, 4);

    refresh();
  }
};

ModelInputsPage::ModelInputsPage() : InputMixPageBase(STR_MENUINPUTS, ICON_MODEL_INPUTS)
{
}

bool ModelInputsPage::reachExposLimit()
{
  if (getExposCount() >= MAX_EXPOS) {
    new MessageDialog(STR_WARNING, STR_NOFREEEXPO);
    return true;
  }
  return false;
}

InputMixGroupBase* ModelInputsPage::getGroupByIndex(uint8_t index)
{
  ExpoData* expo = expoAddress(index);
  if (!EXPO_VALID(expo)) return nullptr;

  int input = expo->chn;
  return getGroupBySrc(MIXSRC_FIRST_INPUT + input);
}

InputMixGroupBase* ModelInputsPage::createGroup(Window* form, mixsrc_t src)
{
  return new InputGroup(form, src);
}

InputMixButtonBase* ModelInputsPage::createLineButton(InputMixGroupBase* group,
                                                  uint8_t index)
{
  auto button = new InputLineButton(group, index);
  button->refresh();

  lines.emplace_back(button);
  group->addLine(button);

  uint8_t input = group->getMixSrc() - MIXSRC_FIRST_INPUT;
  button->setPressHandler([=]() -> uint8_t {
    Menu* menu = new Menu();
    menu->addLine(STR_EDIT, [=]() {
      uint8_t idx = button->getIndex();
      editInput(input, idx);
    });
    if (!reachExposLimit()) {
      if (this->_copyMode != 0) {
        menu->addLine(STR_PASTE_BEFORE, [=]() {
          uint8_t idx = button->getIndex();
          pasteInputBefore(idx);
        });
        menu->addLine(STR_PASTE_AFTER, [=]() {
          uint8_t idx = button->getIndex();
          pasteInputAfter(idx);
        });
      }
      menu->addLine(STR_INSERT_BEFORE, [=]() {
        uint8_t idx = button->getIndex();
        insertInput(input, idx);
      });
      menu->addLine(STR_INSERT_AFTER, [=]() {
        uint8_t idx = button->getIndex();
        insertInput(input, idx + 1);
      });
      menu->addLine(STR_COPY, [=]() {
        this->_copyMode = COPY_MODE;
        this->_copySrc = button;
      });
      menu->addLine(STR_MOVE, [=]() {
        this->_copyMode = MOVE_MODE;
        this->_copySrc = button;
      });
    }
    menu->addLine(STR_DELETE, [=]() {
      uint8_t idx = button->getIndex();
      deleteInput(idx);
    });
    return 0;
  });

  return button;
}

void ModelInputsPage::addLineButton(uint8_t index)
{
  ExpoData* expo = expoAddress(index);
  if (!EXPO_VALID(expo)) return;
  int input = expo->chn;

  InputMixPageBase::addLineButton(MIXSRC_FIRST_INPUT + input, index);
}

void ModelInputsPage::newInput()
{
  Menu* menu = new Menu();
  menu->setTitle(STR_MENU_INPUTS);

  uint8_t chn = 0;
  uint8_t index = 0;
  ExpoData* line = g_model.expoData;

  // search for unused channels
  for (uint8_t i = 0; i < MAX_EXPOS && chn < MAX_INPUTS; i++) {
    if (!EXPO_VALID(line) || (line->chn > chn)) {
      uint8_t chnEnd = EXPO_VALID(line) ? line->chn : chn + 1;
      for (; chn < chnEnd; chn += 1) {
        std::string name(getSourceString(chn + 1));
        menu->addLineBuffered(name.c_str(), [=]() { insertInput(chn, index); });
      }
    }
    if (EXPO_VALID(line)) {
      chn = line->chn + 1;
      index += 1;
    }
    ++line;
  }

  menu->updateLines();
}

void ModelInputsPage::editInput(uint8_t input, uint8_t index)
{
  _copyMode = 0;

  auto group = getGroupBySrc(MIXSRC_FIRST_INPUT + input);
  if (!group) return;

  auto line = getLineByIndex(index);
  if (!line) return;

  auto edit = new InputEditWindow(input, index);
  edit->setCloseHandler([=]() {
    line->refresh();
    group->refresh();
    group->adjustHeight();
  });
}

void ModelInputsPage::insertInput(uint8_t input, uint8_t index)
{
  ::insertExpo(index, input);
  InputMixPageBase::addLineButton(MIXSRC_FIRST_INPUT + input, index);
  editInput(input, index);
}

void ModelInputsPage::deleteInput(uint8_t index)
{
  _copyMode = 0;

  auto group = getGroupByIndex(index);
  if (!group) return;

  auto line = getLineByIndex(index);
  if (!line) return;

  group->removeLine(line);
  if (group->getLineCount() == 0) {
    group->deleteLater();
    removeGroup(group);
  } else {
    line->deleteLater();
  }
  removeLine(line);

  ::deleteExpo(index);
}

void ModelInputsPage::pasteInput(uint8_t dst_idx, uint8_t input)
{
  if (!_copyMode || !_copySrc) return;
  uint8_t src_idx = _copySrc->getIndex();

  ::copyExpo(src_idx, dst_idx, input);
  addLineButton(dst_idx);

  if (_copyMode == MOVE_MODE) {
    src_idx = _copySrc->getIndex();
    deleteInput(src_idx);
  }

  _copyMode = 0;
}

static int _inputChnFromIndex(uint8_t index)
{
  ExpoData* expo = expoAddress(index);
  if (!EXPO_VALID(expo)) return -1;
  return expo->chn;
}

void ModelInputsPage::pasteInputBefore(uint8_t dst_idx)
{
  int input = _inputChnFromIndex(dst_idx);
  pasteInput(dst_idx, input);
}

void ModelInputsPage::pasteInputAfter(uint8_t dst_idx)
{
  int input = _inputChnFromIndex(dst_idx);
  pasteInput(dst_idx + 1, input);
}

void ModelInputsPage::build(Window* window)
{
  // reset clipboard
  _copyMode = 0;

  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  form = new Window(window, rect_t{});
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_TINY);

  auto btn = new TextButton(window, rect_t{}, LV_SYMBOL_PLUS, [=]() {
    newInput();
    return 0;
  });
  auto btn_obj = btn->getLvObj();
  lv_obj_set_width(btn_obj, lv_pct(100));
  lv_group_focus_obj(btn_obj);

  groups.clear();
  lines.clear();

  bool focusSet = false;
  uint8_t index = 0;
  ExpoData* line = g_model.expoData;
  for (uint8_t input = 0; input < MAX_INPUTS; input++) {
    if (index >= MAX_EXPOS) break;

    if (line->chn == input && EXPO_VALID(line)) {
      // one group for the complete input channel
      auto group = createGroup(form, MIXSRC_FIRST_INPUT + input);
      groups.emplace_back(group);
      while (index < MAX_EXPOS && line->chn == input && EXPO_VALID(line)) {
        // one button per input line
        auto btn = createLineButton(group, index);
        if (!focusSet) {
          focusSet = true;
          lv_group_focus_obj(btn->getLvObj());
        }
        ++index;
        ++line;
      }
    } else if (!EXPO_VALID(line)) {
      // End of list
      break;
    }
  }
}
