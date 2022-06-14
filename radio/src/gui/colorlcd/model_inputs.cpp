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

bool reachExposLimit()
{
  if (getExposCount() >= MAX_EXPOS) {
    POPUP_WARNING(STR_NOFREEEXPO);
    return true;
  }
  return false;
}

// TODO: these functions need to be added to the generic API
//       used by all radios, and be removed from UI code
//
void copyExpo(uint8_t source, uint8_t dest, uint8_t input)
{
  pauseMixerCalculations();
  ExpoData sourceExpo;
  memcpy(&sourceExpo, expoAddress(source), sizeof(ExpoData));
  ExpoData *expo = expoAddress(dest);
  size_t trailingExpos = MAX_EXPOS - (dest + 1);
  memmove(expo + 1, expo, trailingExpos * sizeof(ExpoData));
  memcpy(expo, &sourceExpo, sizeof(ExpoData));
  expo->chn = input;
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

void deleteExpo(uint8_t idx)
{
  pauseMixerCalculations();
  ExpoData * expo = expoAddress(idx);
  int input = expo->chn;
  memmove(expo, expo+1, (MAX_EXPOS-(idx+1))*sizeof(ExpoData));
  memclear(&g_model.expoData[MAX_EXPOS-1], sizeof(ExpoData));
  if (!isInputAvailable(input)) {
    memclear(&g_model.inputNames[input], LEN_INPUT_NAME);
  }
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

// TODO port: avoid global s_currCh on ARM boards (as done here)...
int8_t s_currCh;
uint8_t s_copyMode;
int8_t s_copySrcRow;

void insertExpo(uint8_t idx, uint8_t input)
{
  pauseMixerCalculations();
  ExpoData * expo = expoAddress(idx);
  memmove(expo+1, expo, (MAX_EXPOS-(idx+1))*sizeof(ExpoData));
  memclear(expo, sizeof(ExpoData));
  expo->srcRaw = (input >= 4 ? MIXSRC_Rud + input : MIXSRC_Rud + channelOrder(input + 1) - 1);
  expo->curve.type = CURVE_REF_EXPO;
  expo->mode = 3; // pos+neg
  expo->chn = input;
  expo->weight = 100;
  resumeMixerCalculations();
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

  void paint(BitmapBuffer *dc) override
  {
    const ExpoData &line = g_model.expoData[index];
    LcdFlags textColor = COLOR_THEME_SECONDARY1;

    coord_t border = lv_obj_get_style_border_width(lvobj, LV_PART_MAIN);
    coord_t pad_left = lv_obj_get_style_pad_left(lvobj, LV_PART_MAIN);

    coord_t left = pad_left + border;
    coord_t line_h = lv_obj_get_style_text_line_space(lvobj, LV_PART_MAIN)
      + getFontHeight(FONT(STD));

    // first line ...
    coord_t y = 0;
    y += border;
    y += lv_obj_get_style_pad_top(lvobj, LV_PART_MAIN);

    coord_t x = left;
    drawValueOrGVar(dc, x, y, line.weight, -100, 100, textColor);
    drawSource(dc, x + 65, y, line.srcRaw, textColor);

    if (line.name[0]) {
      dc->drawMask(146, y, mixerSetupLabelIcon, textColor);
      dc->drawSizedText(166, y, line.name, sizeof(line.name), textColor);
    }

    // second line ...
    y += line_h;
    if (line.swtch || line.curve.value) {
      if (line.swtch) {
        dc->drawMask(x, y, mixerSetupSwitchIcon, textColor);
        drawSwitch(dc, x + 21, y, line.swtch, textColor);
      }
      if (line.curve.value != 0) {
        dc->drawMask(x + 65, y, mixerSetupCurveIcon, textColor);
        drawCurveRef(dc, x + 85, y, line.curve, textColor);
      }
    }

    if (line.flightModes) {
#if LCD_H > LCD_W
      // third line ...
      y += line_h;
      x = left;
#else
      x = 146;
#endif
      drawFlightModes(dc, line.flightModes, textColor, x, y);
    }
  }

protected:
  bool isActive() const override { return isExpoActive(index); }
  size_t getLines() const override {
    const ExpoData* line = expoAddress(index);
    size_t lines = 1;
#if LCD_W > LCD_H
    if (line->swtch || line->curve.value != 0 || line->flightModes) {
      lines += 1;
    }
#else
    if (line->swtch || line->curve.value != 0) {
      lines += 1;
    }
    if (line->flightModes) {
      lines += 1;
    }    
#endif
    return lines;
  }
};

ModelInputsPage::ModelInputsPage():
  PageTab(STR_MENUINPUTS, ICON_MODEL_INPUTS)
{
  setOnSetVisibleHandler([=]() {
    // reset clipboard
    _copyMode = 0;
  });
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
  lines.emplace_back(button);
  group->addLine(button);

  uint8_t input = group->getMixSrc() - MIXSRC_FIRST_INPUT;
  button->setPressHandler([=]() -> uint8_t {
    Menu *menu = new Menu(form);
    menu->addLine(STR_EDIT, [=]() { editInput(input, index); });
    if (!reachExposLimit()) {
      menu->addLine(STR_INSERT_BEFORE,
                    [=]() {
                      uint8_t idx = button->getIndex();
                      insertInput(input, idx);
                      _copyMode = 0;
                    });
      menu->addLine(STR_INSERT_AFTER,
                    [=]() {
                      uint8_t idx = button->getIndex();
                      insertInput(input, idx + 1);
                      _copyMode = 0;
                    });
      menu->addLine(STR_COPY, [=]() {
        _copyMode = COPY_MODE;
        _copySrc = button;
      });
      if (_copyMode != 0) {
        menu->addLine(STR_PASTE_BEFORE, [=]() {
          uint8_t idx = button->getIndex();
          pasteInputBefore(idx);
          _copyMode = 0;
        });
        menu->addLine(STR_PASTE_AFTER, [=]() {
          uint8_t idx = button->getIndex();
          pasteInputAfter(idx);
          _copyMode = 0;
        });
      }
    }
    menu->addLine(STR_MOVE, [=]() {
      _copyMode = MOVE_MODE;
      _copySrc = button;
    });
    menu->addLine(STR_DELETE, [=]() {
      uint8_t idx = button->getIndex();
      deleteInput(idx);
      _copyMode = 0;
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
    deleteInput(src_idx);
  }
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
  form->padRow(lv_dpx(8));

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

