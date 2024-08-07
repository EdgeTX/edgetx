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

#include "list_line_button.h"

#include <algorithm>

#include "edgetx.h"
#include "etx_lv_theme.h"

static void input_mix_line_constructor(const lv_obj_class_t* class_p,
                                       lv_obj_t* obj)
{
  etx_std_style(obj, LV_PART_MAIN, PAD_TINY);
}

static const lv_obj_class_t input_mix_line_class = {
    .base_class = &lv_btn_class,
    .constructor_cb = input_mix_line_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = LV_PCT(100),
    .height_def = ListLineButton::BTN_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_TRUE,
    .instance_size = sizeof(lv_btn_t),
};

static lv_obj_t* input_mix_line_create(lv_obj_t* parent)
{
  return etx_create(&input_mix_line_class, parent);
}

ListLineButton::ListLineButton(Window* parent, uint8_t index) :
    ButtonBase(parent, rect_t{}, nullptr, input_mix_line_create), index(index)
{
}

void ListLineButton::checkEvents()
{
  check(isActive());
  ButtonBase::checkEvents();
}

InputMixButtonBase::InputMixButtonBase(Window* parent, uint8_t index) :
    ListLineButton(parent, index)
{
  setWidth(BTN_W);
  setHeight(ListLineButton::BTN_H);
  padAll(PAD_ZERO);

  weight = lv_label_create(lvobj);
  lv_obj_set_pos(weight, WGT_X, WGT_Y);
  lv_obj_set_size(weight, WGT_W, WGT_H);
  etx_font(weight, FONT_XS_INDEX, LV_STATE_USER_1);

  source = lv_label_create(lvobj);
  lv_obj_set_pos(source, SRC_X, SRC_Y);
  lv_obj_set_size(source, SRC_W, SRC_H);

  opts = lv_label_create(lvobj);
  lv_obj_set_pos(opts, OPT_X, OPT_Y);
  lv_obj_set_size(opts, OPT_W, OPT_H);
  etx_font(opts, FONT_XS_INDEX, LV_STATE_USER_1);
}

InputMixButtonBase::~InputMixButtonBase()
{
  if (fm_buffer) free(fm_buffer);
}

void InputMixButtonBase::setWeight(gvar_t value, gvar_t min, gvar_t max)
{
  char s[32];
  getValueOrSrcVarString(s, sizeof(s), value, min, max, 0, "%");
  if (getTextWidth(s, 0, FONT(STD)) > WGT_W)
    lv_obj_add_state(weight, LV_STATE_USER_1);
  else
    lv_obj_clear_state(weight, LV_STATE_USER_1);

  lv_label_set_text(weight, s);
}

void InputMixButtonBase::setSource(mixsrc_t idx)
{
  char* s = getSourceString(idx);
  lv_label_set_text(source, s);
}

void InputMixButtonBase::setOpts(const char* s)
{
  if (getTextWidth(s, 0, FONT(STD)) > OPT_W)
    lv_obj_add_state(opts, LV_STATE_USER_1);
  else
    lv_obj_clear_state(opts, LV_STATE_USER_1);

  lv_label_set_text(opts, s);
}

void InputMixButtonBase::setFlightModes(uint16_t modes)
{
  if (!modelFMEnabled()) return;
  if (modes == fm_modes) return;
  fm_modes = modes;

  if (!fm_modes) {
    if (!fm_canvas) return;
    lv_obj_del(fm_canvas);
    free(fm_buffer);
    fm_canvas = nullptr;
    fm_buffer = nullptr;
#if PORTRAIT_LCD
    setHeight(ListLineButton::BTN_H);
#endif
    return;
  }

  if (!fm_canvas) {
    fm_canvas = lv_canvas_create(lvobj);
    fm_buffer = malloc(FM_CANVAS_WIDTH * FM_CANVAS_HEIGHT);
    lv_canvas_set_buffer(fm_canvas, fm_buffer, FM_CANVAS_WIDTH,
                         FM_CANVAS_HEIGHT, LV_IMG_CF_ALPHA_8BIT);
    lv_obj_set_pos(fm_canvas, FM_X, FM_Y);
#if PORTRAIT_LCD
    setHeight(ListLineButton::BTN_H + FM_CANVAS_HEIGHT + 2);
#endif

    lv_obj_set_style_img_recolor(fm_canvas, makeLvColor(COLOR_THEME_SECONDARY1),
                                 0);
    lv_obj_set_style_img_recolor_opa(fm_canvas, LV_OPA_COVER, 0);
  }

  lv_canvas_fill_bg(fm_canvas, lv_color_black(), LV_OPA_TRANSP);

  const MaskBitmap* mask = getBuiltinIcon(ICON_TEXTLINE_FM);
  lv_coord_t w = mask->width;
  lv_coord_t h = mask->height;

  coord_t x = 0;
  lv_canvas_copy_buf(fm_canvas, mask->data, x, 0, w, h);
  x += (w + PAD_TINY);

  lv_draw_label_dsc_t label_dsc;
  lv_draw_label_dsc_init(&label_dsc);

  lv_draw_rect_dsc_t rect_dsc;
  lv_draw_rect_dsc_init(&rect_dsc);
  rect_dsc.bg_opa = LV_OPA_COVER;

  const lv_font_t* font = getFont(FONT(XS));
  label_dsc.font = font;

  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    char s[] = " ";
    s[0] = '0' + i;
    if (fm_modes & (1 << i)) {
      label_dsc.color = lv_color_make(0x7f, 0x7f, 0x7f);
    } else {
      lv_canvas_draw_rect(fm_canvas, x, 0, FM_W, 3, &rect_dsc);
      label_dsc.color = lv_color_white();
    }
    lv_canvas_draw_text(fm_canvas, x, 0, FM_W, &label_dsc, s);
    x += FM_W;
  }
}

static void group_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_std_style(obj, LV_PART_MAIN, PAD_TINY);
}

static const lv_obj_class_t group_class = {
    .base_class = &lv_obj_class,
    .constructor_cb = group_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = ListLineButton::GRP_W,
    .height_def = LV_SIZE_CONTENT,
    .editable = LV_OBJ_CLASS_EDITABLE_FALSE,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_FALSE,
    .instance_size = sizeof(lv_obj_t),
};

static lv_obj_t* group_create(lv_obj_t* parent)
{
  return etx_create(&group_class, parent);
}

InputMixGroupBase::InputMixGroupBase(Window* parent, mixsrc_t idx) :
    Window(parent, rect_t{}, group_create), idx(idx)
{
  setWindowFlag(NO_FOCUS);

  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_CLICKABLE);
  padAll(PAD_ZERO);

  label = lv_label_create(lvobj);
  etx_font(label, FONT_STD_INDEX);
}

void InputMixGroupBase::adjustHeight()
{
  if (getLineCount() == 0) setHeight(ListLineButton::BTN_H + 8);

  coord_t y = 2;
  for (auto it = lines.cbegin(); it != lines.cend(); ++it) {
    auto line = *it;
    line->updatePos(InputMixButtonBase::LN_X, y);
    y += line->height() + 2;
  }
  setHeight(y + 4);
}

void InputMixGroupBase::addLine(InputMixButtonBase* line)
{
  auto l = std::find_if(lines.begin(), lines.end(),
                        [=](const InputMixButtonBase* l) -> bool {
                          return line->getIndex() <= l->getIndex();
                        });

  if (l != lines.end())
    lines.insert(l, line);
  else
    lines.emplace_back(line);

  adjustHeight();
}

bool InputMixGroupBase::removeLine(InputMixButtonBase* line)
{
  auto l = std::find_if(
      lines.begin(), lines.end(),
      [=](const InputMixButtonBase* l) -> bool { return l == line; });

  if (l != lines.end()) {
    lines.erase(l);
    adjustHeight();
    return true;
  }

  return false;
}

void InputMixGroupBase::refresh()
{
  lv_label_set_text(label, getSourceString(idx));
}

InputMixGroupBase* InputMixPageBase::getGroupBySrc(mixsrc_t src)
{
  auto g = std::find_if(
      groups.begin(), groups.end(),
      [=](InputMixGroupBase* g) -> bool { return g->getMixSrc() == src; });

  if (g != groups.end()) return *g;

  return nullptr;
}

void InputMixPageBase::removeGroup(InputMixGroupBase* g)
{
  auto group = std::find_if(groups.begin(), groups.end(),
                            [=](InputMixGroupBase* lh) -> bool { return lh == g; });
  if (group != groups.end()) groups.erase(group);
}

InputMixButtonBase* InputMixPageBase::getLineByIndex(uint8_t index)
{
  auto l = std::find_if(lines.begin(), lines.end(), [=](InputMixButtonBase* l) {
    return l->getIndex() == index;
  });

  if (l != lines.end()) return *l;

  return nullptr;
}

void InputMixPageBase::removeLine(InputMixButtonBase* l)
{
  auto line = std::find_if(lines.begin(), lines.end(),
                           [=](InputMixButtonBase* lh) -> bool { return lh == l; });
  if (line == lines.end()) return;

  line = lines.erase(line);
  while (line != lines.end()) {
    (*line)->setIndex((*line)->getIndex() - 1);
    ++line;
  }
}

void InputMixPageBase::addLineButton(mixsrc_t src, uint8_t index)
{
  InputMixGroupBase* group_w = getGroupBySrc(src);
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
      (*l)->swapLvglGroup(*l_prev);
      std::swap(*l, *l_prev);
      // Inc index of elements after
      (*l)->setIndex((*l)->getIndex() + 1);
      ++l;
      ++l_prev;
    }
  }
}
