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
#include "choiceex.h"
#include "bitfield.h"
#include "model_inputs.h"
#include "gvar_numberedit.h"
#include "dataconstants.h"
#include "input_mix_group.h"
#include "input_mix_button.h"
#include "mixer_edit.h"

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

uint8_t getMixesCount()
{
  uint8_t count = 0;
  uint8_t ch;

  for (int i = MAX_MIXERS - 1; i >= 0; i--) {
    ch = mixAddress(i)->srcRaw;
    if (ch != 0) {
      count++;
    }
  }
  return count;
}

bool reachMixesLimit()
{
  if (getMixesCount() >= MAX_MIXERS) {
    POPUP_WARNING(STR_NOFREEMIXER);
    return true;
  }
  return false;
}

void insertMix(uint8_t idx, uint8_t channel)
{
  pauseMixerCalculations();
  MixData *mix = mixAddress(idx);
  memmove(mix + 1, mix, (MAX_MIXERS - (idx + 1)) * sizeof(MixData));
  memclear(mix, sizeof(MixData));
  mix->destCh = channel;
  mix->srcRaw = channel + 1;
  if (!isSourceAvailable(mix->srcRaw)) {
    mix->srcRaw = (channel > 3 ? MIXSRC_Rud - 1 + channel
                               : MIXSRC_Rud - 1 + channelOrder(channel));
    while (!isSourceAvailable(mix->srcRaw)) {
      mix->srcRaw += 1;
    }
  }
  mix->weight = 100;
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

void deleteMix(uint8_t idx)
{
  pauseMixerCalculations();
  MixData * mix = mixAddress(idx);
  memmove(mix, mix + 1, (MAX_MIXERS - (idx + 1)) * sizeof(MixData));
  memclear(&g_model.mixData[MAX_MIXERS - 1], sizeof(MixData));
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

#if defined(LUA)
// This is necessary as the LUA API uses th old interface
// where insertMix() has only one param. The channel is
// passed through s_currCh
void insertMix(uint8_t idx)
{
  insertMix(idx, s_currCh - 1);
}
#endif

void copyMix(uint8_t source, uint8_t dest, int8_t ch)
{
  pauseMixerCalculations();
  MixData sourceMix;
  memcpy(&sourceMix, mixAddress(source), sizeof(MixData));
  MixData *mix = mixAddress(dest);
  size_t trailingMixes = MAX_MIXERS - (dest + 1);
  memmove(mix + 1, mix, trailingMixes * sizeof(MixData));
  memcpy(mix, &sourceMix, sizeof(MixData));
  mix->destCh = ch;
  resumeMixerCalculations();
  storageDirty(EE_MODEL);
}

bool swapMixes(uint8_t &idx, uint8_t up)
{
  MixData * x, * y;
  int8_t tgt_idx = (up ? idx - 1 : idx + 1);

  x = mixAddress(idx);

  if (tgt_idx < 0) {
    if (x->destCh == 0)
      return false;
    x->destCh--;
    return true;
  }

  if (tgt_idx == MAX_MIXERS) {
    if (x->destCh == MAX_OUTPUT_CHANNELS - 1)
      return false;
    x->destCh++;
    return true;
  }

  y = mixAddress(tgt_idx);
  uint8_t destCh = x->destCh;
  if (!y->srcRaw || destCh != y->destCh) {
    if (up) {
      if (destCh > 0) x->destCh--;
      else return false;
    }
    else {
      if (destCh < MAX_OUTPUT_CHANNELS - 1) x->destCh++;
      else return false;
    }
    return true;
  }

  pauseMixerCalculations();
  memswap(x, y, sizeof(MixData));
  resumeMixerCalculations();

  idx = tgt_idx;
  return true;
}

class MixLineButton : public InputMixButton
{
 public:
  MixLineButton(Window* parent, uint8_t index);
  void paint(BitmapBuffer* dc) override;
  void deleteLater(bool detach = true, bool trash = true) override;

 protected:
  size_t getLines() const override;
  bool isActive() const override { return isMixActive(index); }
};

static void mix_draw_mplex(lv_event_t* e)
{
  auto obj = (lv_obj_t*)lv_event_get_user_data(e);
  if (!obj || (lv_obj_get_index(obj) <= 1)) return;

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

size_t MixLineButton::getLines() const
{
  size_t lines = 1;
  const MixData* mix = mixAddress(index);

  uint8_t delayslow = 0;
  if (mix->speedDown || mix->speedUp) delayslow = 1;
  if (mix->delayUp || mix->delayDown) delayslow += 2;

  if (mix->flightModes || mix->name[0] || delayslow) {
    lines += 1;
  }

  return lines;
}

void MixLineButton::paint(BitmapBuffer* dc)
{
  const MixData& line = g_model.mixData[index];
  LcdFlags textColor = COLOR_THEME_SECONDARY1;

  coord_t border = lv_obj_get_style_border_width(lvobj, LV_PART_MAIN);
  coord_t pad_left = lv_obj_get_style_pad_left(lvobj, LV_PART_MAIN);
  coord_t pad_right = lv_obj_get_style_pad_right(lvobj, LV_PART_MAIN);

  coord_t left = pad_left + border;
  coord_t line_h = lv_obj_get_style_text_line_space(lvobj, LV_PART_MAIN)
    + getFontHeight(FONT(STD));

#if LCD_W > LCD_H
  const coord_t pad = 42;
#else
  const coord_t pad = 0;
#endif
  
  // first line ...
  coord_t y = 0;
  y += border;
  y += lv_obj_get_style_pad_top(lvobj, LV_PART_MAIN);

  coord_t x = left;
  drawValueOrGVar(dc, x, y, line.weight, MIX_WEIGHT_MIN, MIX_WEIGHT_MAX, textColor);
  x += 46 + pad;

  drawSource(dc, x, y, line.srcRaw, textColor);
  x += 60 + pad;

  // second line ...
  if (line.swtch || line.curve.value) {
    if (line.swtch) {
      if (pad) dc->drawMask(x - 20, y, mixerSetupSwitchIcon, textColor);
      drawSwitch(dc, x, y, line.swtch, textColor);
    }
    x += 44 + pad;
    if (line.curve.value) {
      if (pad) dc->drawMask(x - 20, y, mixerSetupCurveIcon, textColor);
      drawCurveRef(dc, x, y, line.curve, textColor);
    }
    // x += 48 + pad;
  }

  uint8_t delayslow = 0;
  if (line.speedDown || line.speedUp) delayslow = 1;
  if (line.delayUp || line.delayDown) delayslow += 2;

  if (line.flightModes || line.name[0] || delayslow) {
    y += line_h;
    x = left;

    if (line.flightModes) {
      drawFlightModes(dc, line.flightModes, textColor, x, y);
    }
    x += 104 + 3*pad/2;

    if (line.name[0]) {
      dc->drawMask(x, y, mixerSetupLabelIcon, textColor);
      dc->drawSizedText(x + 20, y, line.name, sizeof(line.name), textColor);
    }

    if (delayslow) {
      BitmapBuffer* delayslowbmp[] = {mixerSetupSlowIcon, mixerSetupDelayIcon,
                                      mixerSetupDelaySlowIcon};
      const BitmapBuffer* mask = delayslowbmp[delayslow - 1];
      coord_t w = lv_obj_get_width(lvobj);
      w -= mask->width();
      dc->drawMask(w - border - pad_right, y, mask, textColor);
    }
  }
}

ModelMixesPage::ModelMixesPage() :
  ModelInputsPage()
{
  setTitle(STR_MIXES);
  setIcon(ICON_MODEL_MIXER);
}

InputMixGroup* ModelMixesPage::getGroupByIndex(uint8_t index)
{
  MixData* mix = mixAddress(index);
  if (is_memclear(mix, sizeof(MixData))) return nullptr;

  int ch = mix->destCh;
  return getGroupBySrc(MIXSRC_CH1 + ch);
}

InputMixGroup* ModelMixesPage::createGroup(FormGroup* form, mixsrc_t src)
{
  auto group = new InputMixGroup(form, src);
  group->addMixerMonitor(src - MIXSRC_CH1);
  return group;
}

InputMixButton* ModelMixesPage::createLineButton(InputMixGroup *group, uint8_t index)
{
  auto button = new MixLineButton(group, index);
  lines.emplace_back(button);
  group->addLine(button);

  uint8_t ch = group->getMixSrc() - MIXSRC_CH1;
  button->setPressHandler([=]() -> uint8_t {
    Menu *menu = new Menu(form);
    menu->addLine(STR_EDIT, [=]() { editMix(ch, index); });
    if (!reachMixesLimit()) {
      menu->addLine(STR_INSERT_BEFORE, [=]() {
        uint8_t idx = button->getIndex();
        insertMix(ch, idx);
        _copyMode = 0;
      });
      menu->addLine(STR_INSERT_AFTER, [=]() {
        uint8_t idx = button->getIndex();
        insertMix(ch, idx + 1);
        _copyMode = 0;
      });
      menu->addLine(STR_COPY, [=]() {
        _copyMode = COPY_MODE;
        _copySrc = button;
      });
      if (s_copyMode != 0) {
        menu->addLine(STR_PASTE_BEFORE, [=]() {
          uint8_t idx = button->getIndex();
          pasteMixBefore(idx);
          _copyMode = 0;
        });
        menu->addLine(STR_PASTE_AFTER, [=]() {
          uint8_t idx = button->getIndex();
          pasteMixAfter(idx);
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
      deleteMix(idx);
      _copyMode = 0;
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

  addLineButton(MIXSRC_CH1 + channel, index);
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
      std::string ch_name(getSourceString(MIXSRC_CH1 + ch));
      menu->addLine(ch_name.c_str(), [=]() { insertMix(ch, index); });
    }
  }
}

void ModelMixesPage::editMix(uint8_t channel, uint8_t index)
{
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
  ::insertMix(index, channel);
  addLineButton(MIXSRC_CH1 + channel, index);
  editMix(channel, index);
}

void ModelMixesPage::deleteMix(uint8_t index)
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
  
  ::deleteMix(index);
}

void ModelMixesPage::pasteMix(uint8_t dst_idx, uint8_t channel)
{
  if (!_copyMode || !_copySrc) return;
  uint8_t src_idx = _copySrc->getIndex();

  ::copyMix(src_idx, dst_idx, channel);
  addLineButton(dst_idx);

  if (_copyMode == MOVE_MODE) {
    deleteInput(src_idx);
  }
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
  pasteInput(dst_idx, channel);
}

void ModelMixesPage::pasteMixAfter(uint8_t dst_idx)
{
  int channel = _mixChnFromIndex(dst_idx);
  pasteInput(dst_idx + 1, channel);
}

void ModelMixesPage::build(FormWindow * window)
{
  window->setFlexLayout();
  window->padRow(lv_dpx(8));
  
  form = new FormGroup(window, rect_t{});
  form->setFlexLayout();
  form->padRow(lv_dpx(8));

  auto form_obj = form->getLvObj();
  lv_obj_set_width(form_obj, lv_pct(100));

  auto btn = new TextButton(window, rect_t{}, LV_SYMBOL_PLUS, [=]() {
    newMix();
    return 0;
  });
  auto btn_obj = btn->getLvObj();
  lv_obj_set_width(btn_obj, lv_pct(100));

  groups.clear();
  lines.clear();

  uint8_t index = 0;
  MixData* line = g_model.mixData;
  for (uint8_t ch = 0; ch < MAX_OUTPUT_CHANNELS; ch++) {

    if (index >= MAX_MIXERS) break;

    bool skip_mix = (ch == 0 && is_memclear(line, sizeof(MixData)));
    if (line->destCh == ch && !skip_mix) {

      // one group for the complete mixer channel
      auto group = createGroup(form, MIXSRC_CH1 + ch);
      groups.emplace_back(group);
      while (index < MAX_MIXERS && (line->destCh == ch) && !skip_mix) {
        // one button per input line
        createLineButton(group, index);
        ++index;
        ++line;
        skip_mix = (ch == 0 && is_memclear(line, sizeof(MixData)));
      }
    }
  }
}

