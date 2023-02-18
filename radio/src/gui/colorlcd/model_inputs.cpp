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
#include "opentx.h"
#include "gvar_numberedit.h"
#include "libopenui.h"
#include "choiceex.h"
#include "model_curves.h"
#include "input_edit.h"
#include "input_mix_group.h"
#include "input_mix_button.h"

#include "tasks/mixer_task.h"
#include <algorithm>

#define SET_DIRTY() storageDirty(EE_MODEL)

uint8_t getExposCount()
{
  uint8_t count = 0;
  uint8_t ch ;

  for (int i=MAX_EXPOS-1 ; i>=0; i--) {
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
  ExpoData *expo = expoAddress(dest);
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
  ExpoData * expo = expoAddress(idx);
  int input = expo->chn;
  memmove(expo, expo+1, (MAX_EXPOS-(idx+1))*sizeof(ExpoData));
  memclear(&g_model.expoData[MAX_EXPOS-1], sizeof(ExpoData));
  if (!isInputAvailable(input)) {
    memclear(&g_model.inputNames[input], LEN_INPUT_NAME);
  }
  mixerTaskStart();
  storageDirty(EE_MODEL);
}

// TODO port: avoid global s_currCh on ARM boards (as done here)...
int8_t s_currCh;
uint8_t s_copyMode;
int8_t s_copySrcRow;

void insertExpo(uint8_t idx, uint8_t input)
{
  mixerTaskStop();
  ExpoData * expo = expoAddress(idx);
  memmove(expo+1, expo, (MAX_EXPOS-(idx+1))*sizeof(ExpoData));
  memclear(expo, sizeof(ExpoData));
  expo->srcRaw = (input >= 4 ? MIXSRC_Rud + input : MIXSRC_Rud + channelOrder(input + 1) - 1);
  expo->curve.type = CURVE_REF_EXPO;
  expo->mode = 3; // pos+neg
  expo->chn = input;
  expo->weight = 100;
  mixerTaskStart();
  storageDirty(EE_MODEL);
}

static bool getFreeInput(uint8_t& input, uint8_t& index)
{
  uint8_t chn = 0;
  ExpoData* line = g_model.expoData;
  for (uint8_t i = 0; i < MAX_EXPOS; i++) {
    if (!EXPO_VALID(line) || (line->chn > chn)) {
      if (i >= MAX_EXPOS) break;
      if (chn >= MAX_INPUTS) break;
      index = i;
      input = chn;
      return true;
    }
    chn = line->chn + 1;
    ++line;
  }

  return false;
}

class InputLineButton : public InputMixButton
{
 public:
  using InputMixButton::InputMixButton;

  void refresh() override
  {
    const ExpoData &line = g_model.expoData[index];
    setWeight(line.weight, -100, 100);
    setSource(line.srcRaw);

    char tmp_str[64];
    size_t maxlen = sizeof(tmp_str);

    char *s = tmp_str;
    *s = '\0';

    if (line.name[0]) {
      int cnt = lv_snprintf(s, maxlen, "%.*s ", (int)sizeof(line.name), line.name);
      if ((size_t)cnt >= maxlen) maxlen = 0;
      else { maxlen -= cnt; s += cnt; }
    }

    if (line.swtch || line.curve.value) {
       if (line.swtch) {
         char* sw_pos = getSwitchPositionName(line.swtch);
         int cnt = lv_snprintf(s, maxlen, "%s ", sw_pos);
         if ((size_t)cnt >= maxlen) maxlen = 0;
         else { maxlen -= cnt; s += cnt; }
       }
       if (line.curve.value != 0) {
         getCurveRefString(s, maxlen, line.curve);
         int cnt = strnlen(s, maxlen);
         if ((size_t)cnt >= maxlen) maxlen = 0;
         else { maxlen -= cnt; s += cnt; }
       }
    }
    lv_label_set_text_fmt(opts, "%.*s", (int)sizeof(tmp_str), tmp_str);

    setFlightModes(line.flightModes);
  }

protected:
  bool isActive() const override { return isExpoActive(index); }
};

ModelInputsPage::ModelInputsPage():
  PageTab(STR_MENUINPUTS, ICON_MODEL_INPUTS)
{
  setOnSetVisibleHandler([=]() {
    // reset clipboard
    _copyMode = 0;
  });
}

bool ModelInputsPage::reachExposLimit()
{
  if (getExposCount() >= MAX_EXPOS) {
    new MessageDialog(form, STR_WARNING, STR_NOFREEEXPO);
    return true;
  }
  return false;
}

InputMixGroup* ModelInputsPage::getGroupBySrc(mixsrc_t src)
{
  auto g =
      std::find_if(groups.begin(), groups.end(), [=](InputMixGroup* g) -> bool {
        return g->getMixSrc() == src;
      });

  if (g != groups.end()) return *g;

  return nullptr;
}

InputMixGroup* ModelInputsPage::getGroupByIndex(uint8_t index)
{
  ExpoData* expo = expoAddress(index);
  if (!EXPO_VALID(expo)) return nullptr;

  int input = expo->chn;
  return getGroupBySrc(MIXSRC_FIRST_INPUT + input);
}

InputMixButton* ModelInputsPage::getLineByIndex(uint8_t index)
{
  auto l = std::find_if(lines.begin(), lines.end(), [=](InputMixButton* l) {
    return l->getIndex() == index;
  });

  if (l != lines.end()) return *l;

  return nullptr;
}

void ModelInputsPage::removeGroup(InputMixGroup* g)
{
  auto group = std::find_if(groups.begin(), groups.end(),
                            [=](InputMixGroup* lh) -> bool { return lh == g; });
  if (group != groups.end()) groups.erase(group);
}

void ModelInputsPage::removeLine(InputMixButton* l)
{
  auto line = std::find_if(lines.begin(), lines.end(),
                            [=](InputMixButton* lh) -> bool { return lh == l; });
  if (line == lines.end()) return;
  
  line = lines.erase(line);
  while (line != lines.end()) {
    (*line)->setIndex((*line)->getIndex() - 1);
    ++line;
  }
}

InputMixGroup* ModelInputsPage::createGroup(FormGroup* form, mixsrc_t src)
{
  return new InputMixGroup(form, src);
}

InputMixButton* ModelInputsPage::createLineButton(InputMixGroup *group,
                                                  uint8_t index)
{
  auto button = new InputLineButton(group, index);
  button->refresh();
  
  lines.emplace_back(button);
  group->addLine(button);

  uint8_t input = group->getMixSrc() - MIXSRC_FIRST_INPUT;
  button->setPressHandler([=]() -> uint8_t {
    Menu *menu = new Menu(form);
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

  addLineButton(MIXSRC_FIRST_INPUT + input, index);
}

void ModelInputsPage::addLineButton(mixsrc_t src, uint8_t index)
{
  InputMixGroup* group_w = getGroupBySrc(src);
  if (!group_w) {    
    group_w = createGroup(form, src);
    // insertion sort
    groups.emplace_back(group_w);
    auto g = groups.rbegin();
    if (g != groups.rend()) {
      auto g_prev = g; ++g_prev;
      while (g_prev != groups.rend()) {
        if ((*g_prev)->getMixSrc() < (*g)->getMixSrc()) break;
        lv_obj_swap((*g)->getLvObj(), (*g_prev)->getLvObj());
        std::swap(*g, *g_prev);
        ++g; ++g_prev;
      }
    }
  }

  // create new line button
  auto btn = createLineButton(group_w, index);
  lv_group_focus_obj(btn->getLvObj());

  // insertion sort for the focus group
  auto l = lines.rbegin();
  if (l != lines.rend()) {
    auto l_prev = l; ++l_prev;
    while (l_prev != lines.rend()) {
      if ((*l_prev)->getIndex() < (*l)->getIndex()) break;
      // Swap elements (focus + line list)
      lv_obj_t* obj1 = (*l)->getLvObj();
      lv_obj_t* obj2 = (*l_prev)->getLvObj();
      if (lv_obj_get_parent(obj1) == lv_obj_get_parent(obj2)) {
        // same input group: swap obj + focus group
        lv_obj_swap(obj1, obj2);
      } else {
        // different input group: swap only focus group
        lv_group_swap_obj(obj1, obj2);
      }
      std::swap(*l, *l_prev);
      // Inc index of elements after 
      (*l)->setIndex((*l)->getIndex() + 1);
      ++l; ++l_prev;
    }
  }
}

void ModelInputsPage::editInput(uint8_t input, uint8_t index)
{
  _copyMode = 0;

  auto group = getGroupBySrc(MIXSRC_FIRST_INPUT + input);
  if (!group) return;

  auto line = getLineByIndex(index);
  if (!line) return;

  auto line_obj = line->getLvObj();
  auto group_obj = group->getLvObj();
  auto edit = new InputEditWindow(input, index);
  edit->setCloseHandler([=]() {
      lv_event_send(line_obj, LV_EVENT_VALUE_CHANGED, nullptr);
      lv_event_send(group_obj, LV_EVENT_VALUE_CHANGED, nullptr);
    });
}

void ModelInputsPage::insertInput(uint8_t input, uint8_t index)
{
  ::insertExpo(index, input);
  addLineButton(MIXSRC_FIRST_INPUT + input, index);
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
    removeLine(line);
  } else {
    line->deleteLater();
    removeLine(line);
  }
  
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

void ModelInputsPage::build(FormWindow *window)
{
  window->setFlexLayout();
  window->padRow(lv_dpx(8));
  
  form = new FormGroup(window, rect_t{});
  form->setFlexLayout();
  form->padRow(lv_dpx(4));

  auto form_obj = form->getLvObj();
  lv_obj_set_width(form_obj, lv_pct(100));

  auto btn = new TextButton(window, rect_t{}, LV_SYMBOL_PLUS, [=]() {
    uint8_t input, index;
    if (getFreeInput(input, index)) insertInput(input, index);
    return 0;
  });
  auto btn_obj = btn->getLvObj();
  lv_obj_set_width(btn_obj, lv_pct(100));

  groups.clear();
  lines.clear();

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
        createLineButton(group, index);
        ++index;
        ++line;
      }
    } else if (line->chn > input && EXPO_VALID(line)) {
      TRACE("missing input for channel #%d", input);
    } else if (!EXPO_VALID(line)) {
      TRACE("invalid line #%d", index);
      break;
    }
  }
}

