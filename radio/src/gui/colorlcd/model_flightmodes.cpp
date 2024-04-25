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
#include "opentx.h"
#include "page.h"
#include "switchchoice.h"
#include "themes/etx_lv_theme.h"

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

#if LCD_W > LCD_H
#define TRIMS_PER_LINE 2
static const lv_coord_t trims_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                           LV_GRID_TEMPLATE_LAST};
#else
#define TRIMS_PER_LINE 1
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
        this, rect_t{0, 0, 65, 0}, getSourceString(MIXSRC_FIRST_TRIM + trimId),
        [=]() {
          tr->mode = (tr->mode == TRIM_MODE_NONE) ? 0 : TRIM_MODE_NONE;
          tr_mode->setValue(tr->mode);
          showControls();
          SET_DIRTY();
          return tr->mode == 0;
        });

    if (tr->mode != TRIM_MODE_NONE) tr_btn->check();

    tr_mode = new Choice(this, rect_t{0, 0, 70, 0}, 0, 2 * MAX_FLIGHT_MODES,
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
        this, rect_t{0, 0, 70, 0}, g_model.extendedTrims ? -512 : -128,
        g_model.extendedTrims ? 512 : 128, GET_SET_DEFAULT(tr->value));

    showControls();
  }

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

 protected:
  uint8_t index;
};

#if LCD_W > LCD_H  // Landscape

#define BTN_H 36
#define FMID_W 36
#define NAME_W 95
#define NAME_Y 8
#define SWTCH_Y 6
#define MAX_FMTRIMS 6
#define TRIM_W 30
#define TRIM_X (SWTCH_X + SWTCH_W + 2)
#define TRIM_Y 0
#define FADE_Y 6

#else  // Portrait

#define BTN_H 56
#define FMID_W 46
#define NAME_W 160
#define NAME_Y 0
#define SWTCH_Y 0
#define MAX_FMTRIMS 4
#define TRIM_W 40
#define TRIM_X (FMID_X + FMID_W + 2)
#define TRIM_Y 20
#define FADE_Y 24

#endif

#define FMID_X 2
#define FMID_Y (BTN_H / 2 - 12)
#define NAME_X (FMID_X + FMID_W + 2)
#define SWTCH_W 50
#define SWTCH_X (NAME_X + NAME_W + 2)
#define TRIMC_W (MAX_FMTRIMS * TRIM_W)
#define FADE_W 45
#define FADE_X (TRIM_X + TRIMC_W + 2)

static void fm_id_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
}

static const lv_obj_class_t fm_id_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_id_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FMID_W,
    .height_def = PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_name_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
  etx_font(obj, FONT_XS_INDEX);
}

static const lv_obj_class_t fm_name_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_name_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = NAME_W,
    .height_def = PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_switch_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_left, LV_PART_MAIN);
}

static const lv_obj_class_t fm_switch_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_switch_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = SWTCH_W,
    .height_def = PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_fade_constructor(const lv_obj_class_t* class_p, lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_right, LV_PART_MAIN);
}

static const lv_obj_class_t fm_fade_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_fade_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = FADE_W,
    .height_def = PAGE_LINE_HEIGHT,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

static void fm_trim_mode_constructor(const lv_obj_class_t* class_p,
                                     lv_obj_t* obj)
{
  etx_obj_add_style(obj, styles->text_align_center, LV_PART_MAIN);
}

static const lv_obj_class_t fm_trim_mode_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_trim_mode_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = TRIM_W,
    .height_def = 16,
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

static const lv_obj_class_t fm_trim_value_class = {
    .base_class = &lv_label_class,
    .constructor_cb = fm_trim_value_constructor,
    .destructor_cb = nullptr,
    .user_data = nullptr,
    .event_cb = nullptr,
    .width_def = TRIM_W,
    .height_def = 16,
    .editable = LV_OBJ_CLASS_EDITABLE_INHERIT,
    .group_def = LV_OBJ_CLASS_GROUP_DEF_INHERIT,
    .instance_size = sizeof(lv_label_t),
};

class FlightModeBtn : public ListLineButton
{
 public:
  FlightModeBtn(Window* parent, int index) : ListLineButton(parent, index)
  {
    padAll(PAD_ZERO);
    padColumn(PAD_ZERO);
    setHeight(BTN_H);

    check(isActive());

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
      lv_obj_set_pos(fmTrimValue[i], TRIM_X + i * TRIM_W, TRIM_Y + 16);
    }

    fmFadeIn = etx_create(&fm_fade_class, lvobj);
    lv_obj_set_pos(fmFadeIn, FADE_X, FADE_Y);
    fmFadeOut = etx_create(&fm_fade_class, lvobj);
    lv_obj_set_pos(fmFadeOut, FADE_X + FADE_W + 1, FADE_Y);
    lv_obj_update_layout(lvobj);
  }

  bool isActive() const override { return (getFlightMode() == index); }

  void checkEvents() override
  {
    ListLineButton::checkEvents();
    if (!refreshing && init) {
      refreshing = true;
      const auto& fm = g_model.flightModeData[index];
      for (int t = 0; t < keysGetMaxTrims(); t += 1) {
        if (lastTrim[t] != fm.trim[t].value) {
          lastTrim[t] = fm.trim[t].value;

          uint8_t mode = fm.trim[t].mode;
          bool checked = (mode != TRIM_MODE_NONE);
          bool showValue = (index == 0) || ((mode & 1) || (mode >> 1 == index));

          if (checked && showValue)
            lv_label_set_text(fmTrimValue[t],
                              formatNumberAsString(fm.trim[t].value).c_str());
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
      uint8_t mode = fm.trim[i].mode;
      bool checked = (mode != TRIM_MODE_NONE);
      bool showValue = (index == 0) || ((mode & 1) || (mode >> 1 == index));

      lv_label_set_text(fmTrimMode[i], getFMTrimStr(mode, false).c_str());

      if (checked && showValue)
        lv_label_set_text(fmTrimValue[i],
                          formatNumberAsString(fm.trim[i].value).c_str());
      else
        lv_label_set_text(fmTrimValue[i], "");
    }

    lv_label_set_text(
        fmFadeIn,
        formatNumberAsString(fm.fadeIn, PREC1, 0, nullptr, "s").c_str());
    lv_label_set_text(
        fmFadeOut,
        formatNumberAsString(fm.fadeOut, PREC1, 0, nullptr, "s").c_str());
  }

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
  int lastTrim[MAX_FMTRIMS];
};

ModelFlightModesPage::ModelFlightModesPage() :
    PageTab(STR_MENUFLIGHTMODES, ICON_MODEL_FLIGHT_MODES, PAD_MEDIUM)
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
    lv_obj_set_pos(btn->getLvObj(), 6, i * (BTN_H + 3) + 4);
    btn->setWidth(LCD_W - 12);

    btn->setPressHandler([=]() {
      new FlightModeEdit(i);
      return 0;
    });
  }

  trimCheck = new TextButton(
      form, rect_t{6, MAX_FLIGHT_MODES * (BTN_H + 3) + 8, LCD_W - 12, 40}, STR_CHECKTRIMS, [&]() -> uint8_t {
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
