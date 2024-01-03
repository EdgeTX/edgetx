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
#include "libopenui.h"
#include "choice.h"
#include "bitfield.h"
#include "model_inputs.h"
#include "gvar_numberedit.h"
#include "dataconstants.h"
#include "input_mix_group.h"
#include "input_mix_button.h"
#include "mixer_edit.h"
#include "input_mapping.h"
#include "mixes.h"

#include "tasks/mixer_task.h"
#include "hal/adc_driver.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)
#define PASTE_BEFORE    -2
#define PASTE_AFTER     -1

static const uint8_t _mask_mplex_add[] = {
#include "mask_mplex_add.lbm"
};
STATIC_LZ4_BITMAP(mask_mplex_add);

static const uint8_t _mask_mplex_multi[] = {
#include "mask_mplex_multi.lbm"
};
STATIC_LZ4_BITMAP(mask_mplex_multi);

static const uint8_t _mask_mplex_replace[] = {
#include "mask_mplex_replace.lbm"
};
STATIC_LZ4_BITMAP(mask_mplex_replace);


class MixLineButton : public InputMixButton
{
 public:
  MixLineButton(Window* parent, uint8_t index);

  void deleteLater(bool detach = true, bool trash = true) override;
  void refresh() override;

 protected:
  bool isActive() const override { return isMixActive(index); }
};

static void mix_draw_mplex(lv_event_t* e)
{
  auto target = (lv_obj_t*)lv_event_get_target(e);
  auto group = (InputMixGroup*)lv_obj_get_user_data(target);
  uint32_t offset = group->mixerMonitorEnabled() ? 1 : 0;
  
  auto obj = (lv_obj_t*)lv_event_get_user_data(e);
  if (!obj || (lv_obj_get_index(obj) <= offset)) return;

  auto btn = (MixLineButton*)lv_obj_get_user_data(obj);
  if (!btn) return;

  lv_obj_draw_part_dsc_t* dsc = lv_event_get_draw_part_dsc(e);
  if (dsc->part != LV_PART_MAIN) return;

  MixData* mix = mixAddress(btn->getIndex());
  const uint8_t* mask_map = nullptr;
  if (mix->mltpx == MLTPX_ADD) {
    mask_map = mask_mplex_add;
  } else if (mix->mltpx == MLTPX_MUL) {
    mask_map = mask_mplex_multi;
  } else if (mix->mltpx == MLTPX_REPL) {
    mask_map = mask_mplex_replace;
  } else {
    return;
  }
  
  lv_area_t coords;
  lv_coord_t area_h = lv_area_get_height(&obj->coords);
  lv_coord_t mask_w = MASK_WIDTH(mask_map);
  lv_coord_t mask_h = MASK_HEIGHT(mask_map);
  lv_coord_t pad_left = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);

  coords.x1 = obj->coords.x1 - mask_w - pad_left;
  coords.x2 = coords.x1 + mask_w - 1;
  coords.y1 = obj->coords.y1 + (area_h - mask_h) / 2;
  coords.y2 = coords.y1 + mask_h - 1;

  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_opa = LV_OPA_COVER;
  rect_dsc.bg_color = makeLvColor(COLOR_THEME_SECONDARY1);

  lv_draw_mask_map_param_t m;
  int16_t mask_id;

  lv_draw_mask_map_init(&m, &coords, MASK_DATA(mask_map));
  mask_id = lv_draw_mask_add(&m, NULL);

  // draw masked symbol
  lv_draw_rect(dsc->draw_ctx, &rect_dsc, &coords);

  // free ressources
  lv_draw_mask_free_param(&m);
  lv_draw_mask_remove_id(mask_id);
}

MixLineButton::MixLineButton(Window* parent, uint8_t index) :
    InputMixButton(parent, index)
{
  lv_obj_t* p_obj = parent->getLvObj();
  lv_obj_add_event_cb(p_obj, mix_draw_mplex, LV_EVENT_DRAW_PART_END, lvobj);
}

void MixLineButton::deleteLater(bool detach, bool trash)
{
  lv_obj_t* p_obj = parent->getLvObj();
  lv_obj_remove_event_cb_with_user_data(p_obj, mix_draw_mplex, lvobj);
  InputMixButton::deleteLater(detach, trash);
}

void MixLineButton::refresh()
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

  setFlightModes(line.flightModes);
}

ModelMixesPage::ModelMixesPage() :
  ModelInputsPage()
{
  setTitle(STR_MIXES);
  setIcon(ICON_MODEL_MIXER);
}

bool ModelMixesPage::reachMixesLimit()
{
  if (getMixCount() >= MAX_MIXERS) {
    new MessageDialog(form, STR_WARNING, STR_NOFREEMIXER);
    return true;
  }
  return false;
}

InputMixGroup* ModelMixesPage::getGroupByIndex(uint8_t index)
{
  MixData* mix = mixAddress(index);
  if (is_memclear(mix, sizeof(MixData))) return nullptr;

  int ch = mix->destCh;
  return getGroupBySrc(MIXSRC_FIRST_CH + ch);
}

InputMixGroup* ModelMixesPage::createGroup(FormWindow* form, mixsrc_t src)
{
  auto group = new InputMixGroup(form, src);
  if (showMonitors) group->enableMixerMonitor(src - MIXSRC_FIRST_CH);
  return group;
}

InputMixButton* ModelMixesPage::createLineButton(InputMixGroup *group, uint8_t index)
{
  auto button = new MixLineButton(group, index);
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

void ModelMixesPage::addLineButton(mixsrc_t src, uint8_t index)
{
  ModelInputsPage::addLineButton(src, index);
}

void ModelMixesPage::addLineButton(uint8_t index)
{
  MixData* mix = mixAddress(index);
  if (is_memclear(mix, sizeof(MixData))) return;
  int channel = mix->destCh;

  addLineButton(MIXSRC_FIRST_CH + channel, index);
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

  auto line_obj = line->getLvObj();
  auto edit = new MixEditWindow(channel, index);
  edit->setCloseHandler([=]() {
    MixData* mix = mixAddress(index);
    if (is_memclear(mix, sizeof(MixData))) {
      deleteMix(index);
    } else {
      lv_event_send(line_obj, LV_EVENT_VALUE_CHANGED, nullptr);
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

  group->removeLine(line);
  if (group->getLineCount() == 0) {
    group->deleteLater();
    removeGroup(group);
    removeLine(line);
  } else {
    line->deleteLater();
    removeLine(line);
  }
  
  ::deleteMix(index);
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

void ModelMixesPage::build(FormWindow * window)
{
  scroll_win = window->getParent();

  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, 3);

  form = new FormWindow(window, rect_t{});
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, 3);

  auto box = new FormWindow(window, rect_t{});
  box->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));
  box->padLeft(lv_dpx(8));

  auto box_obj = box->getLvObj();
  lv_obj_set_width(box_obj, lv_pct(100));
  lv_obj_set_style_flex_cross_place(box_obj, LV_FLEX_ALIGN_CENTER, 0);

  new StaticText(box, rect_t{}, STR_SHOW_MIXER_MONITORS, 0, COLOR_THEME_PRIMARY1);
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

  auto form_obj = form->getLvObj();
  auto h = lv_obj_get_height(form_obj);
  for(auto* group : groups) {
    if (enabled) {
      group->enableMixerMonitor(group->getMixSrc() - MIXSRC_FIRST_CH);
    } else {
      group->disableMixerMonitor();
    }
  }

  lv_obj_update_layout(form_obj);
  auto diff = h - lv_obj_get_height(form_obj);
  auto scroll_obj = scroll_win->getLvObj();
  lv_obj_scroll_by_bounded(scroll_obj, 0, diff, LV_ANIM_OFF);
  TRACE("diff = %d", diff);
}
