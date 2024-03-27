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

#include "model_mixes.h"
#include "opentx.h"
#include "input_mix_group.h"
#include "input_mix_button.h"
#include "mixer_edit.h"
#include "mixes.h"
#include "channel_bar.h"

#include <algorithm>

#define SET_DIRTY()     storageDirty(EE_MODEL)

static const lv_coord_t col_dsc[] = {
    45,
    LV_GRID_FR(1),
    LV_GRID_TEMPLATE_LAST,
};

static const lv_coord_t col_dsc2[] = {
    26,
    LV_GRID_FR(1),
    LV_GRID_TEMPLATE_LAST,
};

static const lv_coord_t row_dsc[] = {
    LV_GRID_CONTENT,
    LV_GRID_TEMPLATE_LAST,
};

MixGroup::MixGroup(Window* parent, mixsrc_t idx) :
  InputMixGroupBase(parent, idx, col_dsc)
{
  lv_obj_t* chText = nullptr;
  if (idx >= MIXSRC_FIRST_CH && idx <= MIXSRC_LAST_CH &&
      g_model.limitData[idx - MIXSRC_FIRST_CH].name[0] != '\0') {
    chText = lv_label_create(lvobj);
    etx_font(chText, FONT_XS_INDEX);
    lv_label_set_text_fmt(chText, TR_CH "%" PRIu32,
                          UINT32_C(idx - MIXSRC_FIRST_CH + 1));
    lv_obj_set_style_pad_left(chText, PAD_TINY, 0);
    lv_obj_set_grid_cell(chText, LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_END,
                         0, 1);
  }

  lv_obj_set_style_pad_top(label, -1, 0);

  lv_obj_set_grid_cell(label, LV_GRID_ALIGN_START, 0, 1,
                       LV_GRID_ALIGN_START, 0, 1);

  lv_obj_t* outer = line_container;
  lv_obj_set_style_pad_all(outer, PAD_ZERO, LV_PART_MAIN);
  lv_obj_set_style_pad_row(outer, PAD_ZERO, LV_PART_MAIN);

  monitor = new MixerChannelBar(this, {0, 0, 100, 14}, idx - MIXSRC_FIRST_CH);
  lv_obj_set_parent(monitor->getLvObj(), outer); 
  lv_obj_set_style_translate_x(monitor->getLvObj(), -6, LV_PART_MAIN);
  lv_obj_set_grid_cell(monitor->getLvObj(), LV_GRID_ALIGN_END, 0, 1, LV_GRID_ALIGN_START,
                       0, 1);
  monitor->hide();

  lv_obj_t* inner = window_create(outer);
  lv_obj_set_size(inner, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_layout(inner, LV_LAYOUT_GRID);
  lv_obj_set_grid_dsc_array(inner, col_dsc2, row_dsc);
  lv_obj_set_style_pad_all(inner, PAD_ZERO, LV_PART_MAIN);
  lv_obj_set_style_pad_row(inner, PAD_ZERO, LV_PART_MAIN);

  mplex_container = window_create(inner);
  lv_obj_set_size(mplex_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(mplex_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_flex_cross_place(mplex_container, LV_FLEX_ALIGN_END, 0);
  lv_obj_set_style_pad_ver(mplex_container, PAD_TINY, LV_PART_MAIN);
  lv_obj_set_style_pad_hor(mplex_container, PAD_ZERO, LV_PART_MAIN);
  lv_obj_set_style_pad_row(mplex_container, PAD_TINY, LV_PART_MAIN);
  lv_obj_set_grid_cell(mplex_container, LV_GRID_ALIGN_STRETCH, 0, 1,
                       LV_GRID_ALIGN_START, 0, 1);

  line_container = window_create(inner);
  lv_obj_set_size(line_container, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(line_container, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_flex_cross_place(line_container, LV_FLEX_ALIGN_END, 0);
  lv_obj_set_style_pad_all(line_container, PAD_TINY, LV_PART_MAIN);
  lv_obj_set_style_pad_row(line_container, PAD_TINY, LV_PART_MAIN);
  lv_obj_set_grid_cell(line_container, LV_GRID_ALIGN_STRETCH, 1, 1,
                       LV_GRID_ALIGN_START, 0, 1);
}

void MixGroup::enableMixerMonitor()
{
  monitor->show();
}

void MixGroup::disableMixerMonitor()
{
  monitor->hide();
}

void MixGroup::addMPlex(Window* mplex)
{
  lv_obj_set_parent(mplex->getLvObj(), mplex_container);
}

class MPlexIcon : public Window
{
 public:
  MPlexIcon(Window* parent, uint8_t index) :
    Window(parent, {0, 0, 25, 29}),
    index(index)
    {
      MixData* mix = mixAddress(index);
      EdgeTxIcon n = ICON_MPLEX_ADD;
      if (mix->mltpx == MLTPX_MUL) {
        n = ICON_MPLEX_MULTIPLY;
      } else if (mix->mltpx == MLTPX_REPL) {
        n = ICON_MPLEX_REPLACE;
      }
      icon = new StaticIcon(this, 0, 0, n, COLOR_THEME_SECONDARY1);
      icon->center(width(), height());
    }

  void refresh()
  {
    if (icon) {
      icon->show(lv_obj_get_index(lvobj) != 0);
      MixData* mix = mixAddress(index);
      EdgeTxIcon n = ICON_MPLEX_ADD;
      if (mix->mltpx == MLTPX_MUL) {
        n = ICON_MPLEX_MULTIPLY;
      } else if (mix->mltpx == MLTPX_REPL) {
        n = ICON_MPLEX_REPLACE;
      }
      icon->setIcon(n);
    }
  }

  void setIndex(uint8_t i)
  {
    index = i;
  }

 protected:
  uint8_t index;
  StaticIcon* icon = nullptr;
};

class MixLineButton : public InputMixButton
{
 public:
  MixLineButton(Window* parent, uint8_t index, MPlexIcon* mplex) :
    InputMixButton(parent, index),
    mplex(mplex)
  {
  }

  void deleteLater(bool detach = true, bool trash = true) override
  {
    if (mplex) mplex->deleteLater(detach, trash);
    InputMixButton::deleteLater(detach, trash);
  }

  void refresh() override
  {
    const MixData& line = g_model.mixData[index];
    setWeight(line.weight, MIX_WEIGHT_MIN, MIX_WEIGHT_MAX);
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

    mplex->refresh();

    setFlightModes(line.flightModes);
  }

  void setIndex(uint8_t i) override
  {
    InputMixButton::setIndex(i);
    mplex->setIndex(i);
  }

  lv_obj_t* mplexLvObj() const { return mplex->getLvObj(); }

 protected:
  MPlexIcon* mplex = nullptr;
  bool isActive() const override { return isMixActive(index); }
};

ModelMixesPage::ModelMixesPage() : InputMixPageBase(STR_MIXES, ICON_MODEL_MIXER)
{
}

bool ModelMixesPage::reachMixesLimit()
{
  if (getMixCount() >= MAX_MIXERS) {
    new MessageDialog(form, STR_WARNING, STR_NOFREEMIXER);
    return true;
  }
  return false;
}

MixGroup* ModelMixesPage::getGroupByIndex(uint8_t index)
{
  MixData* mix = mixAddress(index);
  if (is_memclear(mix, sizeof(MixData))) return nullptr;

  int ch = mix->destCh;
  return (MixGroup*)getGroupBySrc(MIXSRC_FIRST_CH + ch);
}

MixLineButton* ModelMixesPage::getLineByIndex(uint8_t index)
{
  auto l = std::find_if(lines.begin(), lines.end(), [=](MixLineButton* l) {
    return l->getIndex() == index;
  });

  if (l != lines.end()) return *l;

  return nullptr;
}

void ModelMixesPage::removeLine(MixLineButton* l)
{
  auto line = std::find_if(lines.begin(), lines.end(),
                           [=](MixLineButton* lh) -> bool { return lh == l; });
  if (line == lines.end()) return;

  line = lines.erase(line);
  while (line != lines.end()) {
    (*line)->setIndex((*line)->getIndex() - 1);
    ++line;
  }
}

MixGroup* ModelMixesPage::createGroup(Window* form, mixsrc_t src)
{
  auto group = new MixGroup(form, src);
  if (showMonitors) group->enableMixerMonitor();
  return group;
}

MixLineButton* ModelMixesPage::createLineButton(MixGroup *group, uint8_t index)
{
  auto mplex = new MPlexIcon(group, index);
  group->addMPlex(mplex);

  auto button = new MixLineButton(group, index, mplex);
  button->refresh();

  lines.emplace_back(button);
  group->addLine(button);

  uint8_t ch = group->getMixSrc() - MIXSRC_FIRST_CH;
  button->setPressHandler([=]() -> uint8_t {
    Menu *menu = new Menu(form);
    menu->addLine(STR_EDIT, [=]() {
        uint8_t idx = button->getIndex();
        editMix(ch, idx);
      });
    if (!reachMixesLimit()) {
      if (this->_copyMode != 0) {
        menu->addLine(STR_PASTE_BEFORE, [=]() {
          uint8_t idx = button->getIndex();
          pasteMixBefore(idx);
        });
        menu->addLine(STR_PASTE_AFTER, [=]() {
          uint8_t idx = button->getIndex();
          pasteMixAfter(idx);
        });
      }
      menu->addLine(STR_INSERT_BEFORE, [=]() {
        uint8_t idx = button->getIndex();
        insertMix(ch, idx);
      });
      menu->addLine(STR_INSERT_AFTER, [=]() {
        uint8_t idx = button->getIndex();
        insertMix(ch, idx + 1);
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
      deleteMix(idx);
    });
    return 0;
  });

  return button;
}

void ModelMixesPage::addLineButton(uint8_t index)
{
  MixData* mix = mixAddress(index);
  if (is_memclear(mix, sizeof(MixData))) return;
  int channel = mix->destCh;

  addLineButton(MIXSRC_FIRST_CH + channel, index);
}

void ModelMixesPage::addLineButton(mixsrc_t src, uint8_t index)
{
  MixGroup* group_w = (MixGroup*)getGroupBySrc(src);
  if (!group_w) {
    group_w = createGroup(form, src);
    // insertion sort
    groups.emplace_back(group_w);
    auto g = groups.rbegin();
    if (g != groups.rend()) {
      auto g_prev = g;
      ++g_prev;
      while (g_prev != groups.rend()) {
        if ((*g_prev)->getMixSrc() < (*g)->getMixSrc()) break;
        lv_obj_swap((*g)->getLvObj(), (*g_prev)->getLvObj());
        std::swap(*g, *g_prev);
        ++g;
        ++g_prev;
      }
    }
  }

  // create new line button
  auto btn = createLineButton(group_w, index);
  lv_group_focus_obj(btn->getLvObj());

  // insertion sort for the focus group
  auto l = lines.rbegin();
  if (l != lines.rend()) {
    auto l_prev = l;
    ++l_prev;
    while (l_prev != lines.rend()) {
      if ((*l_prev)->getIndex() < (*l)->getIndex()) break;
      // Swap elements (focus + line list)
      lv_obj_t* obj1 = (*l)->getLvObj();
      lv_obj_t* obj2 = (*l_prev)->getLvObj();
      if (lv_obj_get_parent(obj1) == lv_obj_get_parent(obj2)) {
        // same input group: swap obj + focus group
        lv_obj_swap(obj1, obj2);
        lv_obj_swap((*l)->mplexLvObj(), (*l_prev)->mplexLvObj());
      } else {
        // different input group: swap only focus group
        lv_group_swap_obj(obj1, obj2);
        lv_group_swap_obj((*l)->mplexLvObj(), (*l_prev)->mplexLvObj());
      }
      std::swap(*l, *l_prev);
      // Inc index of elements after
      (*l)->setIndex((*l)->getIndex() + 1);
      ++l;
      ++l_prev;
    }
  }
}

void ModelMixesPage::newMix()
{
  Menu* menu = new Menu(Layer::back());
  menu->setTitle(STR_MENU_CHANNELS);

  uint8_t index = 0;
  MixData* line = mixAddress(0);

  // search for unused channels
  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {
    if (index >= MAX_MIXERS) break;
    bool skip_mix = (ch == 0 && is_memclear(line, sizeof(MixData)));
    if (line->destCh == ch && !skip_mix) {
      while (index < MAX_MIXERS && (line->destCh == ch) && !skip_mix) {
        ++index;
        ++line;
        skip_mix = (ch == 0 && is_memclear(line, sizeof(MixData)));
      }
    } else {
      std::string ch_name(getSourceString(MIXSRC_FIRST_CH + ch));
      menu->addLineBuffered(ch_name.c_str(), [=]() { insertMix(ch, index); });
    }
  }
  menu->updateLines();
}

void ModelMixesPage::editMix(uint8_t channel, uint8_t index)
{
  _copyMode = 0;

  auto line = getLineByIndex(index);
  if (!line) return;

  auto edit = new MixEditWindow(channel, index);
  edit->setCloseHandler([=]() {
    MixData* mix = mixAddress(index);
    if (is_memclear(mix, sizeof(MixData))) {
      deleteMix(index);
    } else {
      line->refresh();
    }
  });
}

void ModelMixesPage::insertMix(uint8_t channel, uint8_t index)
{
  _copyMode = 0;

  ::insertMix(index, channel);
  addLineButton(MIXSRC_FIRST_CH + channel, index);
  editMix(channel, index);
}

void ModelMixesPage::deleteMix(uint8_t index)
{
  _copyMode = 0;

  auto group = getGroupByIndex(index);
  if (!group) return;

  auto line = getLineByIndex(index);
  if (!line) return;

  ::deleteMix(index);

  group->removeLine(line);
  if (group->getLineCount() == 0) {
    group->deleteLater();
    removeGroup(group);
  } else {
    line->deleteLater();
  }
  removeLine(line);
}

void ModelMixesPage::pasteMix(uint8_t dst_idx, uint8_t channel)
{
  if (!_copyMode || !_copySrc) return;
  uint8_t src_idx = _copySrc->getIndex();

  ::copyMix(src_idx, dst_idx, channel);
  addLineButton(dst_idx);

  if (_copyMode == MOVE_MODE) {
    src_idx = _copySrc->getIndex();
    deleteMix(src_idx);
  }

  _copyMode = 0;
}

static int _mixChnFromIndex(uint8_t index)
{
  MixData* mix = mixAddress(index);
  if (is_memclear(mix, sizeof(MixData))) return -1;
  return mix->destCh;
}

void ModelMixesPage::pasteMixBefore(uint8_t dst_idx)
{
  int channel = _mixChnFromIndex(dst_idx);
  pasteMix(dst_idx, channel);
}

void ModelMixesPage::pasteMixAfter(uint8_t dst_idx)
{
  int channel = _mixChnFromIndex(dst_idx);
  pasteMix(dst_idx + 1, channel);
}

void ModelMixesPage::build(Window * window)
{
  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 3);

  form = new Window(window, rect_t{});
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, 3);

  auto box = new Window(window, rect_t{});
  box->padAll(PAD_TINY);
  box->setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL);
  box->padLeft(lv_dpx(8));

  auto box_obj = box->getLvObj();
  lv_obj_set_style_flex_cross_place(box_obj, LV_FLEX_ALIGN_CENTER, 0);

  new StaticText(box, rect_t{}, STR_SHOW_MIXER_MONITORS);
  new ToggleSwitch(
      box, rect_t{}, [=]() { return showMonitors; },
      [=](uint8_t val) { enableMonitors(val); });

  auto btn = new TextButton(window, rect_t{}, LV_SYMBOL_PLUS, [=]() {
    newMix();
    return 0;
  });
  auto btn_obj = btn->getLvObj();
  lv_obj_set_width(btn_obj, lv_pct(100));
  lv_group_focus_obj(btn_obj);

  groups.clear();
  lines.clear();

  bool focusSet = false;
  uint8_t index = 0;
  MixData* line = g_model.mixData;
  for (uint8_t ch = 0; (ch < MAX_OUTPUT_CHANNELS) && (index < MAX_MIXERS); ch++) {

    bool skip_mix = (ch == 0 && is_memclear(line, sizeof(MixData)));
    if (line->destCh == ch && !skip_mix) {

      // one group for the complete mixer channel
      auto group = createGroup(form, MIXSRC_FIRST_CH + ch);
      groups.emplace_back(group);
      while (index < MAX_MIXERS && (line->destCh == ch) && !skip_mix) {
        // one button per input line
        auto btn = createLineButton(group, index);
        if (!focusSet) {
          focusSet = true;
          lv_group_focus_obj(btn->getLvObj());
        }
        ++index;
        ++line;
        skip_mix = (ch == 0 && is_memclear(line, sizeof(MixData)));
      }
    }
  }
}

void ModelMixesPage::enableMonitors(bool enabled)
{
  if (showMonitors == enabled) return;
  showMonitors = enabled;

  for(auto* g : groups) {
    MixGroup* group = (MixGroup*)g;
    if (enabled) {
      group->enableMixerMonitor();
    } else {
      group->disableMixerMonitor();
    }
  }
}
