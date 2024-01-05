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
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

static std::string getFMTrimStr(uint8_t mode, bool spacer)
{
  mode &= 0x1F;
  if (mode == TRIM_MODE_NONE)
    return "-";
  if (mode == TRIM_MODE_3POS)
    return "3P";
  std::string str((mode & 1) ? "+" : "=");
  if (spacer) str += " ";
  mode >>= 1;
  if (mode > MAX_FLIGHT_MODES - 1)
    mode = MAX_FLIGHT_MODES - 1;
  str += '0'+ mode;
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

class FlightModeEdit : public Page
{
 public:
    FlightModeEdit(uint8_t index) :
      Page(ICON_MODEL_FLIGHT_MODES),
      index(index)
    {
      std::string title2 = std::string(STR_FM) + std::to_string(index);
      header.setTitle(STR_MENUFLIGHTMODES);
      header.setTitle2(title2);

      FlexGridLayout grid(line_col_dsc, line_row_dsc, 2);
      auto form = new FormWindow(&body, rect_t{});
      form->padAll(8);
      form->setFlexLayout();

      FlightModeData* p_fm = &g_model.flightModeData[index];
  
      // Flight mode name
      auto line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
      new ModelTextEdit(line, rect_t{}, p_fm->name, LEN_FLIGHT_MODE_NAME);

      if (index > 0) {
        // Switch
        line = form->newLine(&grid);
        new StaticText(line, rect_t{}, STR_SWITCH, 0, COLOR_THEME_PRIMARY1);
        new SwitchChoice(line, rect_t{}, SWSRC_FIRST_IN_MIXES, SWSRC_LAST_IN_MIXES,
                         GET_SET_DEFAULT(p_fm->swtch));
      }

      // Fade in
      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_FADEIN, 0, COLOR_THEME_PRIMARY1);
      new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(p_fm->fadeIn),
                     SET_VALUE(p_fm->fadeIn, newValue), 0, PREC1);

      // Fade out
      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_FADEOUT, 0, COLOR_THEME_PRIMARY1);
      new NumberEdit(line, rect_t{}, 0, DELAY_MAX, GET_DEFAULT(p_fm->fadeOut),
                     SET_VALUE(p_fm->fadeOut, newValue), 0, PREC1);

      // Trims
      line = form->newLine(&grid);
      new StaticText(line, rect_t{}, STR_TRIMS, 0, COLOR_THEME_PRIMARY1);

      FlexGridLayout trim_grid(trims_col_dsc, line_row_dsc);

      for (int t = 0; t < keysGetMaxTrims(); t++) {
        lastTrim[t] = p_fm->trim[t].value;

        if ((t % TRIMS_PER_LINE) == 0) {
          line = form->newLine(&trim_grid);
          line->padLeft(10);
        }

        auto trim = new FormWindow(line, rect_t{});
        trim->setFlexLayout(LV_FLEX_FLOW_ROW, lv_dpx(8));

        auto trim_obj = trim->getLvObj();
        lv_obj_set_style_pad_column(trim_obj, lv_dpx(8), 0);
        lv_obj_set_style_flex_cross_place(trim_obj, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_grid_cell_x_align(trim_obj, LV_GRID_ALIGN_STRETCH, 0);

        trim_t* tr = &p_fm->trim[t];
        auto tr_btn = new TextButton(
            trim, rect_t{}, getSourceString(MIXSRC_FIRST_TRIM + t),
            [=]() {
              tr->mode = (tr->mode == TRIM_MODE_NONE) ? 0 : TRIM_MODE_NONE;
              tr_mode[t]->setValue(tr->mode);
              SET_DIRTY();
              showControls(t, tr->mode);
              return tr->mode == 0;
            });

        if (tr->mode != TRIM_MODE_NONE) tr_btn->check();
        tr_btn->setWidth(LV_DPI_DEF / 2);
        tr_btn->setHeight(33);

        tr_mode[t] = new Choice(trim, rect_t{}, 0, 2 * MAX_FLIGHT_MODES,
                                GET_DEFAULT(tr->mode),
                                [=](int val) {
                                  tr->mode = val;
                                  showControls(t, tr->mode);
                                  SET_DIRTY();
                                });
        tr_mode[t]->setTextHandler([=](uint8_t mode) { return getFMTrimStr(mode, true); });
        tr_mode[t]->setAvailableHandler([=](int mode) {
          if (index > 0)
            return ((mode & 1) == 0) || ((mode >> 1) != index) || (mode == TRIM_MODE_3POS);
          return (mode == 0) || (mode == TRIM_MODE_3POS);
        });

        tr_value[t] = new NumberEdit(trim, rect_t{0, 0, 70, 0}, 
                                     g_model.extendedTrims ? -512 : -128, g_model.extendedTrims ? 512 : 128,
                                     GET_SET_DEFAULT(tr->value));

        // show trim value choice iff btn->checked()
        showControls(t, tr->mode);
      }
    }

    void checkEvents() override
    {
      for (int i = 0; i < keysGetMaxTrims(); i += 1) {
        const auto& fm = g_model.flightModeData[index];
        if (lastTrim[i] != fm.trim[i].value) {
          lastTrim[i] = fm.trim[i].value;
          tr_value[i]->setValue(lastTrim[i]);
        }
      }
      Page::checkEvents();
    }

  protected:
    uint8_t index;
    Choice* tr_mode[MAX_TRIMS] = {nullptr};
    NumberEdit* tr_value[MAX_TRIMS] = {nullptr};
    int lastTrim[MAX_TRIMS];

    void showControls(int trim, uint8_t mode)
    {
      bool checked = (mode != TRIM_MODE_NONE);
      bool showValue = (index == 0 && mode != TRIM_MODE_3POS) || ((mode & 1) || (mode >> 1 == index));

      if (checked) {
        lv_obj_clear_flag(tr_mode[trim]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(tr_mode[trim]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
      if (checked && showValue) {
        lv_obj_clear_flag(tr_value[trim]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      } else {
        lv_obj_add_flag(tr_value[trim]->getLvObj(), LV_OBJ_FLAG_HIDDEN);
      }
    }
};

#if LCD_W > LCD_H  // Landscape

#define BTN_H 36
#define TRIM_W 30
#define FMID_W 36
#define NAME_W 95
#define SWTCH_W 50
#define FADE_W 45
#define TRIMC_W 180
#define FMID_TOP 6
#define LBL_TOP 6

#else  // Portrait

#define BTN_H 58
#define TRIM_W 40
#define FMID_W 46
#define NAME_W 160
#define SWTCH_W 50
#define FADE_W 45
#define TRIMC_W 160
#define FMID_TOP 24
#define LBL_TOP 0

#endif

class FMStyle
{
  public:
    FMStyle() {}

    void init()
    {
      if(!styleInitDone)
      {
        styleInitDone=true;

        lv_style_init(&fmTrimContStyle);
        lv_style_set_pad_all(&fmTrimContStyle, 0);
        lv_style_set_width(&fmTrimContStyle, TRIMC_W);
        lv_style_set_height(&fmTrimContStyle, 32);

        lv_style_init(&fmIdStyle);
        lv_style_set_text_font(&fmIdStyle, getFont(FONT(STD)));
        lv_style_set_text_align(&fmIdStyle, LV_TEXT_ALIGN_LEFT);
        lv_style_set_width(&fmIdStyle, FMID_W);
        lv_style_set_pad_left(&fmIdStyle, 2);

        lv_style_init(&fmNameStyle);
        lv_style_set_text_font(&fmNameStyle, getFont(FONT(XS)));
        lv_style_set_text_align(&fmNameStyle, LV_TEXT_ALIGN_LEFT);
        lv_style_set_width(&fmNameStyle, NAME_W);

        lv_style_init(&fmSwitchStyle);
        lv_style_set_text_font(&fmSwitchStyle, getFont(FONT(STD)));
        lv_style_set_text_align(&fmSwitchStyle, LV_TEXT_ALIGN_LEFT);
        lv_style_set_width(&fmSwitchStyle, SWTCH_W);

        lv_style_init(&fmFadeStyle);
        lv_style_set_text_font(&fmFadeStyle, getFont(FONT(STD)));
        lv_style_set_text_align(&fmFadeStyle, LV_TEXT_ALIGN_RIGHT);
        lv_style_set_width(&fmFadeStyle, FADE_W);

        lv_style_init(&fmTrimModeStyle);
        lv_style_set_text_font(&fmTrimModeStyle, getFont(FONT(STD)));
        lv_style_set_text_align(&fmTrimModeStyle, LV_TEXT_ALIGN_CENTER);
        lv_style_set_width(&fmTrimModeStyle, TRIM_W);
        lv_style_set_height(&fmTrimModeStyle, 16);

        fmTrimValueStyle = fmTrimModeStyle;
        lv_style_init(&fmTrimValueStyle);
        lv_style_set_text_font(&fmTrimValueStyle, getFont(FONT(XS)));
        lv_style_set_text_align(&fmTrimValueStyle, LV_TEXT_ALIGN_CENTER);
        lv_style_set_width(&fmTrimValueStyle, TRIM_W);
        lv_style_set_height(&fmTrimValueStyle, 16);
      }

      // Always update colors in case theme changes
      lv_style_set_text_color(&fmIdStyle, makeLvColor(COLOR_THEME_SECONDARY1));
      lv_style_set_text_color(&fmNameStyle, makeLvColor(COLOR_THEME_SECONDARY1));
      lv_style_set_text_color(&fmSwitchStyle, makeLvColor(COLOR_THEME_SECONDARY1));
      lv_style_set_text_color(&fmFadeStyle, makeLvColor(COLOR_THEME_SECONDARY1));
      lv_style_set_text_color(&fmTrimModeStyle, makeLvColor(COLOR_THEME_SECONDARY1));
      lv_style_set_text_color(&fmTrimValueStyle, makeLvColor(COLOR_THEME_SECONDARY1));
    }

    lv_obj_t* newId(lv_obj_t* parent)
    {
      auto obj = lv_label_create(parent);
      lv_obj_add_style(obj, &fmIdStyle, LV_PART_MAIN);

      return obj;
    }

    lv_obj_t* newGroup(lv_obj_t* parent)
    {
      lv_obj_t* obj = lv_obj_create(parent);
      lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP);
      lv_obj_set_style_flex_grow(obj, 2, LV_PART_MAIN);
      lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN);
      lv_obj_set_height(obj, LV_SIZE_CONTENT);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);
      lv_obj_set_flex_align(obj, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

      return obj;
    }

    lv_obj_t* newName(lv_obj_t* parent)
    {
      auto obj = lv_label_create(parent);
      lv_obj_add_style(obj, &fmNameStyle, LV_PART_MAIN);

      return obj;
    }

    lv_obj_t* newSwitch(lv_obj_t* parent)
    {
      auto obj = lv_label_create(parent);
      lv_obj_add_style(obj, &fmSwitchStyle, LV_PART_MAIN);

      return obj;
    }

    lv_obj_t* newTrimCont(lv_obj_t* parent)
    {
      auto obj = lv_obj_create(parent);
      lv_obj_add_style(obj, &fmTrimContStyle, LV_PART_MAIN);
      lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE);

      return obj;
    }

    lv_obj_t* newTrimMode(lv_obj_t* parent, int n)
    {
      auto obj = lv_label_create(parent);
      lv_obj_add_style(obj, &fmTrimModeStyle, LV_PART_MAIN);
      lv_obj_set_pos(obj, n * TRIM_W, 0);

      return obj;
    }

    lv_obj_t* newTrimValue(lv_obj_t* parent, int n)
    {
      auto obj = lv_label_create(parent);
      lv_obj_add_style(obj, &fmTrimValueStyle, LV_PART_MAIN);
      lv_obj_set_pos(obj, n * TRIM_W, 16);

      return obj;
    }

    lv_obj_t* newFade(lv_obj_t* parent)
    {
      auto obj = lv_label_create(parent);
      lv_obj_add_style(obj, &fmFadeStyle, LV_PART_MAIN);

      return obj;
    }

  private:
    lv_style_t fmTrimContStyle;
    lv_style_t fmIdStyle;
    lv_style_t fmNameStyle;
    lv_style_t fmSwitchStyle;
    lv_style_t fmFadeStyle;
    lv_style_t fmTrimModeStyle;
    lv_style_t fmTrimValueStyle;
    bool styleInitDone;
};

static FMStyle fmStyle;

class FlightModeBtn : public Button
{
 public:
  FlightModeBtn(Window* parent, int index) :
      Button(parent, rect_t{}, nullptr, 0, 0, input_mix_line_create),
      index(index)
  {
    padTop(0);
    padBottom(0);
    padLeft(3);
    padRight(6);
    setHeight(BTN_H);
    lv_obj_set_flex_flow(lvobj, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(lvobj, 0, 0);
    lv_obj_set_flex_align(lvobj, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);

    check(isActive());

    lv_obj_add_event_cb(lvobj, FlightModeBtn::on_draw,
                        LV_EVENT_DRAW_MAIN_BEGIN, nullptr);
  }

  static void on_draw(lv_event_t* e)
  {
    lv_obj_t* target = lv_event_get_target(e);
    auto line = (FlightModeBtn*)lv_obj_get_user_data(target);
    if (line) {
      if (!line->init)
        line->delayed_init(e);
      else
        line->refresh();
    }
  }

  void delayed_init(lv_event_t* e)
  {
    lv_obj_enable_style_refresh(false);

    fmID = fmStyle.newId(lvobj);

    lv_obj_t* container = fmStyle.newGroup(lvobj);

    fmName = fmStyle.newName(container);
    fmSwitch = fmStyle.newSwitch(container);

    lv_obj_t* trims_cont = fmStyle.newTrimCont(container);

    for (int i = 0; i < keysGetMaxTrims(); i += 1) {
      fmTrimMode[i] = fmStyle.newTrimMode(trims_cont, i);
      fmTrimValue[i] = fmStyle.newTrimValue(trims_cont, i);
    }

    fmFadeIn = fmStyle.newFade(container);
    fmFadeOut = fmStyle.newFade(container);

    init = true;
    refresh();

    lv_obj_enable_style_refresh(true);

    lv_obj_update_layout(lvobj);

    if (e) {
      auto param = lv_event_get_param(e);
      lv_event_send(lvobj, LV_EVENT_DRAW_MAIN, param);
    }
  }

  bool isActive() const
  {
    return (getFlightMode() == index);
  }

  void checkEvents() override
  {
    Button::checkEvents();
    check(isActive());
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
            lv_label_set_text(fmTrimValue[t], formatNumberAsString(fm.trim[t].value).c_str());
        }
      }
      refreshing = false;
    }
  }

  void refresh()
  {
    if (!init) return;

    const auto& fm = g_model.flightModeData[index];

    char label[8];
    getFlightModeString(label, index + 1);
    lv_label_set_text(fmID, label);

    if (fm.name[0] != '\0') {
      lv_label_set_text(fmName, fm.name);
    } else {
      lv_label_set_text(fmName, "");
    }

    if ((index > 0) && (fm.swtch != SWSRC_NONE)) {
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
        lv_label_set_text(fmTrimValue[i], formatNumberAsString(fm.trim[i].value).c_str());
      else
        lv_label_set_text(fmTrimValue[i], "");
    }

    lv_label_set_text(fmFadeIn, formatNumberAsString(fm.fadeIn, PREC1, 0, nullptr, "s").c_str());
    lv_label_set_text(fmFadeOut, formatNumberAsString(fm.fadeOut, PREC1, 0, nullptr, "s").c_str());
  }

 protected:
  bool init = false;
  bool refreshing = false;
  uint8_t index;

  lv_obj_t* fmID = nullptr;
  lv_obj_t* fmName = nullptr;
  lv_obj_t* fmSwitch = nullptr;
  lv_obj_t* fmTrimMode[MAX_TRIMS] = {nullptr};
  lv_obj_t* fmTrimValue[MAX_TRIMS] = {nullptr};
  lv_obj_t* fmFadeIn = nullptr;
  lv_obj_t* fmFadeOut = nullptr;
  int lastTrim[MAX_TRIMS];
};

ModelFlightModesPage::ModelFlightModesPage():
  PageTab(STR_MENUFLIGHTMODES, ICON_MODEL_FLIGHT_MODES)
{
  fmStyle.init();
}

static const lv_coord_t fmt_col_dsc[] = {LV_GRID_FR(1),
                                         LV_GRID_TEMPLATE_LAST};

static const lv_coord_t fmt_row_dsc[] = {LV_GRID_CONTENT,
                                         LV_GRID_TEMPLATE_LAST};

void ModelFlightModesPage::build(FormWindow * window)
{
  window->padAll(4);
  lv_obj_set_scrollbar_mode(window->getLvObj(), LV_SCROLLBAR_MODE_AUTO);

  FormWindow* form = new FormWindow(window, rect_t{});
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, 2);
  form->padRow(lv_dpx(4));

  for (int i = 0; i < MAX_FLIGHT_MODES; i++) {
    auto btn = new FlightModeBtn(form, i);
    btn->setPressHandler([=]() {
                           new FlightModeEdit(i);
                           return 0;
                         });
  }

  trimCheck = new TextButton(form, rect_t{0, 0, lv_pct(100), 40}, STR_CHECKTRIMS, [&]() -> uint8_t {
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
