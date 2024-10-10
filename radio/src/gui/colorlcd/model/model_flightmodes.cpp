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

#include "model_flightmodes.h"

#include "libopenui.h"
#include "list_line_button.h"
#include "edgetx.h"
#include "page.h"
#include "switchchoice.h"
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static std::string getFMTrimStr(uint8_t mode, bool spacer)
{
  mode &= 0x1F;
  if (mode == TRIM_MODE_NONE) return "-";
  if (mode == TRIM_MODE_3POS) return "3P";
  std::string str((mode & 1) ? "+" : "=");
  if (spacer) str += " ";
  mode >>= 1;
  if (mode > MAX_FLIGHT_MODES - 1) mode = MAX_FLIGHT_MODES - 1;
  str += '0' + mode;
  return str;
}

static const lv_coord_t line_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                          LV_GRID_TEMPLATE_LAST};

static const lv_coord_t line_row_dsc[] = {LV_GRID_CONTENT,
                                          LV_GRID_TEMPLATE_LAST};

#if !PORTRAIT_LCD
static const lv_coord_t trims_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                           LV_GRID_TEMPLATE_LAST};
#else
static const lv_coord_t trims_col_dsc[] = {LV_GRID_FR(1),
                                           LV_GRID_TEMPLATE_LAST};
#endif

class TrimEdit : public Window
{
 public:
  TrimEdit(Window* parent, int trimId, int fmId) :
      Window(parent, rect_t{}), trimId(trimId), fmId(fmId)
  {
    setWindowFlag(NO_FOCUS);

    padAll(PAD_TINY);
    setFlexLayout(LV_FLEX_FLOW_ROW, PAD_SMALL, LV_SIZE_CONTENT);

    trim_t* tr = &g_model.flightModeData[fmId].trim[trimId];

    lastTrim = tr->value;

    auto tr_btn = new TextButton(
        this, rect_t{0, 0, TR_BTN_W, 0}, getSourceString(MIXSRC_FIRST_TRIM + trimId),
        [=]() {
          tr->mode = (tr->mode == TRIM_MODE_NONE) ? 0 : TRIM_MODE_NONE;
          tr_mode->setValue(tr->mode);
          showControls();
          SET_DIRTY();
          return tr->mode == 0;
        });

    if (tr->mode != TRIM_MODE_NONE) tr_btn->check();

    tr_mode = new Choice(this, rect_t{0, 0, TR_MODE_W, 0}, 0, 2 * MAX_FLIGHT_MODES,
                         GET_DEFAULT(tr->mode), [=](int val) {
                           tr->mode = val;
                           showControls();
                           SET_DIRTY();
                         });
    tr_mode->setTextHandler(
        [=](uint8_t mode) { return getFMTrimStr(mode, true); });
    tr_mode->setAvailableHandler([=](int mode) {
      if (fmId > 0)
        return ((mode & 1) == 0) || ((mode >> 1) != fmId) ||
               (mode == TRIM_MODE_3POS);
      return (mode == 0) || (mode == TRIM_MODE_3POS);
    });

    tr_value = new NumberEdit(
        this, rect_t{0, 0, TR_MODE_W, 0}, g_model.extendedTrims ? -512 : -128,
        g_model.extendedTrims ? 512 : 128, GET_SET_DEFAULT(tr->value));

    showControls();
  }

  static LAYOUT_VAL(TR_BTN_W, 65, 65)
  static LAYOUT_VAL(TR_MODE_W, 70, 70)

 protected:
  int trimId;
  int fmId;
  int lastTrim;
  Choice* tr_mode = {nullptr};
  NumberEdit* tr_value = {nullptr};

  void showControls()
  {
    uint8_t mode = g_model.flightModeData[fmId].trim[trimId].mode;

    bool checked = (mode != TRIM_MODE_NONE);
    bool showValue = (fmId == 0 && mode != TRIM_MODE_3POS) || ((mode & 1) || (mode >> 1 == fmId));

    tr_mode->show(checked);
    tr_value->show(checked && showValue);
  }

  void checkEvents() override
  {
    const auto& fm = g_model.flightModeData[fmId];
    if (lastTrim != fm.trim[trimId].value) {
      lastTrim = fm.trim[trimId].value;
      tr_value->setValue(lastTrim);
    }
    Window::checkEvents();
  }
};

class FlightModeEdit : public Page
{
 public:
  FlightModeEdit(uint8_t index) : Page(ICON_MODEL_FLIGHT_MODES), index(index)
  {
    std::string title2 = std::string(STR_FM) + std::to_string(index);
    header->setTitle(STR_MENUFLIGHTMODES);
    header->setTitle2(title2);

    FlexGridLayout grid(line_col_dsc, line_row_dsc, PAD_TINY);
    body->setFlexLayout();

    FlightModeData* p_fm = &g_model.flightModeData[index];

    // Flight mode name
    auto line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_NAME);
    new ModelTextEdit(line, rect_t{}, p_fm->name, LEN_FLIGHT_MODE_NAME);

    if (index > 0) {
      // Switch
      line = body->newLine(grid);
      new StaticText(line, rect_t{}, STR_SWITCH);
      new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES,
                       SWSRC_LAST_IN_MIXES, GET_SET_DEFAULT(p_fm->swtch));
    }

    // Fade in
    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_FADEIN);
    new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(p_fm->fadeIn),
                   SET_VALUE(p_fm->fadeIn, newValue), PREC1);

    // Fade out
    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_FADEOUT);
    new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(p_fm->fadeOut),
                   SET_VALUE(p_fm->fadeOut, newValue), PREC1);

    // Trims
    line = body->newLine(grid);
    new StaticText(line, rect_t{}, STR_TRIMS);

    FlexGridLayout trim_grid(trims_col_dsc, line_row_dsc, PAD_SMALL);

    for (int t = 0; t < keysGetMaxTrims(); t++) {
      if ((t % TRIMS_PER_LINE) == 0) {
        line = body->newLine(trim_grid);
        line->padAll(PAD_TINY);
        line->padLeft(10);
      }

      new TrimEdit(line, t, index);
    }
  }

  static LAYOUT_VAL(TRIMS_PER_LINE, 2, 1)

 protected:
  uint8_t index;
};

class FlightModeBtn : public ListLineButton
{
 public:
  FlightModeBtn(Window* parent, int index) : ListLineButton(parent, index)
  {
    padAll(PAD_ZERO);
    padColumn(PAD_ZERO);
    setHeight(BTN_H);

    lv_obj_add_event_cb(lvobj, FlightModeBtn::on_draw, LV_EVENT_DRAW_MAIN_BEGIN,
                        nullptr);
  }

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (FlightModeBtn*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init();
      line->refresh();
    }
  }

  void delayed_init()
  {
    init = true;

    lv_obj_enable_style_refresh(false);

    check(isActive());

    fmID = etx_create(&fm_id_class, lvobj);
    lv_obj_set_pos(fmID, FMID_X, FMID_Y);
    char label[8];
    getFlightModeString(label, index + 1);
    lv_label_set_text(fmID, label);

    fmName = etx_create(&fm_name_class, lvobj);
    lv_obj_set_pos(fmName, NAME_X, NAME_Y);
    fmSwitch = etx_create(&fm_switch_class, lvobj);
    lv_obj_set_pos(fmSwitch, SWTCH_X, SWTCH_Y);

    for (int i = 0; i < keysGetMaxTrims() && i < MAX_FMTRIMS; i += 1) {
      fmTrimMode[i] = etx_create(&fm_trim_mode_class, lvobj);
      lv_obj_set_pos(fmTrimMode[i], TRIM_X + i * TRIM_W, TRIM_Y);
      fmTrimValue[i] = etx_create(&fm_trim_value_class, lvobj);
      lv_obj_set_pos(fmTrimValue[i], TRIM_X + i * TRIM_W, TRIM_Y + TRIM_H);
    }

    fmFadeIn = etx_create(&fm_fade_class, lvobj);
    lv_obj_set_pos(fmFadeIn, FADE_X, FADE_Y);
    fmFadeOut = etx_create(&fm_fade_class, lvobj);
    lv_obj_set_pos(fmFadeOut, FADE_X + FADE_W + 1, FADE_Y);
    lv_obj_update_layout(lvobj);
  
    lv_obj_enable_style_refresh(true);
    lv_obj_refresh_style(lvobj, LV_PART_ANY, LV_STYLE_PROP_ANY);
  }

  bool isActive() const override { return (getFlightMode() == index); }

  void setTrimValue(uint8_t t)
  {
    lastTrim[t] = g_model.flightModeData[index].trim[t].value;

    uint8_t mode = g_model.flightModeData[index].trim[t].mode;
    bool checked = (mode != TRIM_MODE_NONE);
    bool showValue = (index == 0) || ((mode & 1) || (mode >> 1 == index));

    if (checked && showValue)
      lv_label_set_text(fmTrimValue[t],
                        formatNumberAsString(lastTrim[t]).c_str());
    else
      lv_label_set_text(fmTrimValue[t], "");
  }

  void checkEvents() override
  {
    ListLineButton::checkEvents();
    if (!refreshing && init) {
      refreshing = true;
      for (int t = 0; t < keysGetMaxTrims(); t += 1) {
        if (lastTrim[t] != g_model.flightModeData[index].trim[t].value) {
          setTrimValue(t);
        }
      }
      refreshing = false;
    }
  }

  void refresh() override
  {
    if (!init) return;

    const auto& fm = g_model.flightModeData[index];

    if (fm.name[0] != '\0') {
      lv_label_set_text(fmName, fm.name);
    } else {
      lv_label_set_text(fmName, "");
    }

    if ((index > 0) && (fm.swtch != SWSRC_NONE)) {
      char label[16];
      getSwitchPositionName(label, fm.swtch);
      lv_label_set_text(fmSwitch, label);
    } else {
      lv_label_set_text(fmSwitch, "");
    }

    for (int i = 0; i < keysGetMaxTrims(); i += 1) {
      setTrimValue(i);
      lv_label_set_text(fmTrimMode[i], getFMTrimStr(fm.trim[i].mode, false).c_str());
    }

    lv_label_set_text(
        fmFadeIn,
        formatNumberAsString(fm.fadeIn, PREC1, 0, nullptr, "s").c_str());
    lv_label_set_text(
        fmFadeOut,
        formatNumberAsString(fm.fadeOut, PREC1, 0, nullptr, "s").c_str());
  }

  static LAYOUT_VAL(BTN_H, 36, 56)
  static LAYOUT_VAL(MAX_FMTRIMS, 6, 4)
  static constexpr coord_t FMID_X = PAD_TINY;
  static LAYOUT_VAL(FMID_Y, 6, 16)
  static LAYOUT_VAL(FMID_W, 36, 46)
  static constexpr coord_t NAME_X = FMID_X + FMID_W + PAD_TINY;
  static LAYOUT_VAL(NAME_Y, 8, 0)
  static LAYOUT_VAL(NAME_W, 95, 160)
  static constexpr coord_t SWTCH_X = NAME_X + NAME_W + PAD_TINY;
  static LAYOUT_VAL(SWTCH_Y, 6, 0)
  static LAYOUT_VAL(SWTCH_W, 50, 50)
  static LAYOUT_VAL(TRIM_X, SWTCH_X + SWTCH_W + PAD_TINY, FMID_X + FMID_W + PAD_TINY)
  static LAYOUT_VAL(TRIM_Y, 0, 20)
  static LAYOUT_VAL(TRIM_W, 30, 40)
  static LAYOUT_VAL(TRIM_H, 16, 16)
  static constexpr coord_t TRIMC_W = MAX_FMTRIMS * TRIM_W;
  static constexpr coord_t FADE_X = TRIM_X + TRIMC_W + PAD_TINY;
  static LAYOUT_VAL(FADE_Y, 6, 24)
  static LAYOUT_VAL(FADE_W, 45, 45)

 protected:
  bool init = false;
  bool refreshing = false;

  lv_obj_t* fmID = nullptr;
  lv_obj_t* fmName = nullptr;
  lv_obj_t* fmSwitch = nullptr;
  lv_obj_t* fmTrimMode[MAX_FMTRIMS] = {nullptr};
  lv_obj_t* fmTrimValue[MAX_FMTRIMS] = {nullptr};
  lv_obj_t* fmFadeIn = nullptr;
  lv_obj_t* fmFadeOut = nullptr;
  int lastTrim[MAX_FMTRIMS] = {0};

  static const lv_obj_class_t fm_id_class;
  static const lv_obj_class_t fm_name_class;
  static const lv_obj_class_t fm_switch_class;
  static const lv_obj_class_t fm_fade_class;
  static const lv_obj_class_t fm_trim_mode_class;
  static const lv_obj_class_t fm_trim_value_class;
};

static void fm_id_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
}

const lv_obj_class_t FlightModeBtn::fm_id_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_id_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FlightModeBtn::FMID_W,
    .height_def = EdgeTxStyles::PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_name_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
  etx_font(obj, FONT_XS_INDEX);
}

const lv_obj_class_t FlightModeBtn::fm_name_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_name_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FlightModeBtn::NAME_W,
    .height_def = EdgeTxStyles::PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_switch_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
}

const lv_obj_class_t FlightModeBtn::fm_switch_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_switch_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FlightModeBtn::SWTCH_W,
    .height_def = EdgeTxStyles::PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_fade_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_right, LV_PART_MAIN);
}

const lv_obj_class_t FlightModeBtn::fm_fade_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_fade_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FlightModeBtn::FADE_W,
    .height_def = EdgeTxStyles::PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_trim_mode_constructor(const lv_obj_class_t* class_p,
                                     lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
}

const lv_obj_class_t FlightModeBtn::fm_trim_mode_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_trim_mode_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FlightModeBtn::TRIM_W,
    .height_def = FlightModeBtn::TRIM_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_trim_value_constructor(const lv_obj_class_t* class_p,
                                      lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
  etx_font(obj, FONT_XS_INDEX);
}

const lv_obj_class_t FlightModeBtn::fm_trim_value_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_trim_value_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FlightModeBtn::TRIM_W,
    .height_def = FlightModeBtn::TRIM_H,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

ModelFlightModesPage::ModelFlightModesPage() :
    PageTab(STR_MENUFLIGHTMODES, ICON_MODEL_FLIGHT_MODES)
{
}

static const lv_coord_t fmt_col_dsc[] = {LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

static const lv_coord_t fmt_row_dsc[] = {LV_GRID_CONTENT,
                                         LV_GRID_TEMPLATE_LAST};

void ModelFlightModesPage::build(Window* form)
{
  form->padAll(PAD_ZERO);
  form->padBottom(PAD_LARGE);

  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    auto btn = new FlightModeBtn(form, i);
    lv_obj_set_pos(btn->getLvObj(), PAD_SMALL, i * (FlightModeBtn::BTN_H + 3) + 4);
    btn->setWidth(ListLineButton::GRP_W);

    btn->setPressHandler([=]() {
      new FlightModeEdit(i);
      return 0;
    });
  }

  trimCheck = new TextButton(
      form, rect_t{6, MAX_FLIGHT_MODES * (FlightModeBtn::BTN_H + 3) + PAD_LARGE, ListLineButton::GRP_W, TRIM_CHK_H}, STR_CHECKTRIMS, [&]() -> uint8_t {
        if (trimsCheckTimer)
          trimsCheckTimer = 0;
        else
          trimsCheckTimer = 200;  // 2 seconds trims cancelled
        return trimsCheckTimer;
      });
}

void ModelFlightModesPage::checkEvents()
{
  trimCheck->check(trimsCheckTimer > 0);
}
