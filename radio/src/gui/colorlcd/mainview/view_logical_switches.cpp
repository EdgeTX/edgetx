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

#include "view_logical_switches.h"

#include "edgetx.h"
#include "switches.h"
#include "etx_lv_theme.h"

#if PORTRAIT_LCD

// Footer grid
static const lv_coord_t f_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_TEMPLATE_LAST};

#else  // Landscape

// Footer grid
static const lv_coord_t f_col_dsc[] = {
    60, LV_GRID_FR(1), 112, LV_GRID_FR(1), 50, 50, LV_GRID_TEMPLATE_LAST};

#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

void getsEdgeDelayParam(char* s, LogicalSwitchData* ls);

class LogicalSwitchDisplayFooter : public Window
{
 public:
  explicit LogicalSwitchDisplayFooter(Window* parent, rect_t rect) :
      Window(parent, rect)
  {
    setWindowFlag(OPAQUE);

    padAll(PAD_ZERO);
    padLeft(PAD_SMALL);
    padRight(PAD_SMALL);
    etx_solid_bg(lvobj, COLOR_THEME_SECONDARY1_INDEX);

    lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(lvobj, f_col_dsc, row_dsc);
    lv_obj_set_style_pad_row(lvobj, 0, 0);
    lv_obj_set_style_pad_column(lvobj, 2, 0);

    lsFunc = lv_label_create(lvobj);
    etx_obj_add_style(lsFunc, styles->text_align_left, LV_PART_MAIN);
    etx_txt_color(lsFunc, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_grid_cell(lsFunc, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    lsV1 = lv_label_create(lvobj);
    etx_obj_add_style(lsV1, styles->text_align_left, LV_PART_MAIN);
    etx_txt_color(lsV1, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_grid_cell(lsV1, LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    lsV2 = lv_label_create(lvobj);
    etx_obj_add_style(lsV2, styles->text_align_left, LV_PART_MAIN);
    etx_txt_color(lsV2, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_grid_cell(lsV2, LV_GRID_ALIGN_STRETCH, 2, V2_COL_CNT,
                         LV_GRID_ALIGN_CENTER, 0, 1);

    lsAnd = lv_label_create(lvobj);
    etx_obj_add_style(lsAnd, styles->text_align_left, LV_PART_MAIN);
    etx_txt_color(lsAnd, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_grid_cell(lsAnd, LV_GRID_ALIGN_STRETCH, ANDSW_COL, 1,
                         LV_GRID_ALIGN_CENTER, ANDSW_ROW, 1);

    lsDuration = lv_label_create(lvobj);
    etx_obj_add_style(lsDuration, styles->text_align_left, LV_PART_MAIN);
    etx_txt_color(lsDuration, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_grid_cell(lsDuration, LV_GRID_ALIGN_STRETCH, ANDSW_COL + 1, 1,
                         LV_GRID_ALIGN_CENTER, ANDSW_ROW, 1);

    lsDelay = lv_label_create(lvobj);
    etx_obj_add_style(lsDelay, styles->text_align_left, LV_PART_MAIN);
    etx_txt_color(lsDelay, COLOR_THEME_PRIMARY2_INDEX);
    lv_obj_set_grid_cell(lsDelay, LV_GRID_ALIGN_STRETCH, ANDSW_COL + 2, 1,
                         LV_GRID_ALIGN_CENTER, ANDSW_ROW, 1);

    lv_obj_update_layout(parent->getLvObj());

    refresh();
  }

  void refresh()
  {
    LogicalSwitchData* ls = lswAddress(lsIndex);
    uint8_t lsFamily = lswFamily(ls->func);

    char s[20];

    lv_label_set_text(lsFunc, STR_VCSWFUNC[ls->func]);

    // CSW params - V1
    switch (lsFamily) {
      case LS_FAMILY_BOOL:
      case LS_FAMILY_STICKY:
      case LS_FAMILY_EDGE:
        lv_label_set_text(lsV1, getSwitchPositionName(ls->v1));
        break;
      case LS_FAMILY_TIMER:
        lv_label_set_text(lsV1, formatNumberAsString(lswTimerValue(ls->v1),
                                                     PREC1, 0, nullptr, "s")
                                    .c_str());
        break;
      default:
        lv_label_set_text(lsV1, getSourceString(ls->v1));
        break;
    }

    // CSW params - V2
    strcat(s, " ");
    switch (lsFamily) {
      case LS_FAMILY_BOOL:
      case LS_FAMILY_STICKY:
        lv_label_set_text(lsV2, getSwitchPositionName(ls->v2));
        break;
      case LS_FAMILY_EDGE:
        getsEdgeDelayParam(s, ls);
        lv_label_set_text(lsV2, s);
        break;
      case LS_FAMILY_TIMER:
        lv_label_set_text(lsV2, formatNumberAsString(lswTimerValue(ls->v2),
                                                     PREC1, 0, nullptr, "s")
                                    .c_str());
        break;
      case LS_FAMILY_COMP:
        lv_label_set_text(lsV2, getSourceString(ls->v2));
        break;
      default:
        lv_label_set_text(
            lsV2,
            getSourceCustomValueString(
                ls->v1,
                (ls->v1 <= MIXSRC_LAST_CH ? calc100toRESX(ls->v2) : ls->v2),
                0));
        break;
    }

    // AND switch
    lv_label_set_text(lsAnd, getSwitchPositionName(ls->andsw));

    // CSW duration
    if (ls->duration > 0) {
      lv_label_set_text(
          lsDuration,
          formatNumberAsString(ls->duration, PREC1, 0, nullptr, "s").c_str());
    } else {
      lv_label_set_text(lsDuration, "");
    }

    // CSW delay
    if (lsFamily != LS_FAMILY_EDGE && ls->delay > 0) {
      lv_label_set_text(
          lsDelay,
          formatNumberAsString(ls->delay, PREC1, 0, nullptr, "s").c_str());
    } else {
      lv_label_set_text(lsDelay, "");
    }
  }

  void setIndex(unsigned value)
  {
    lsIndex = value;
    refresh();
  }

  static LAYOUT_VAL(V2_COL_CNT, 1, 2)
  static LAYOUT_VAL(ANDSW_ROW, 0, 1)
  static LAYOUT_VAL(ANDSW_COL, 3, 1)

 protected:
  unsigned lsIndex = 0;
  lv_obj_t* lsFunc = nullptr;
  lv_obj_t* lsV1 = nullptr;
  lv_obj_t* lsV2 = nullptr;
  lv_obj_t* lsAnd = nullptr;
  lv_obj_t* lsDuration = nullptr;
  lv_obj_t* lsDelay = nullptr;
};

class LogicalSwitchDisplayButton : public TextButton
{
 public:
  LogicalSwitchDisplayButton(Window* parent, const rect_t& rect,
                             std::string text, unsigned index) :
      TextButton(parent, rect, std::move(text), nullptr), index(index)
  {
  }

  void checkEvents() override
  {
    bool newvalue = getSwitch(SWSRC_FIRST_LOGICAL_SWITCH + index);
    if (value != newvalue) {
      if (newvalue) {
        lv_obj_add_state(lvobj, LV_STATE_CHECKED);
      } else {
        lv_obj_clear_state(lvobj, LV_STATE_CHECKED);
      }
      value = newvalue;
    }
    ButtonBase::checkEvents();
  }

 protected:
  unsigned index = 0;
  bool value = false;
};

LogicalSwitchesViewPage::LogicalSwitchesViewPage() :
    PageTab(STR_MONITOR_SWITCHES, ICON_MONITOR_LOGICAL_SWITCHES)
{
}

void LogicalSwitchesViewPage::build(Window* window)
{
  window->padAll(PAD_ZERO);

  coord_t xo = (LCD_W - (BTN_MATRIX_COL * (BTN_WIDTH + PAD_TINY) - PAD_TINY)) / 2;
  coord_t yo = PAD_TINY;

  // Footer
  footer = new LogicalSwitchDisplayFooter(
      window,
      {0, window->height() - FOOTER_HEIGHT, window->width(), FOOTER_HEIGHT});

  // LSW table
  std::string lsString("L64");
  for (uint8_t i = 0; i < MAX_LOGICAL_SWITCHES; i++) {
    coord_t x = (i % BTN_MATRIX_COL) * (BTN_WIDTH + PAD_TINY) + xo;
    coord_t y = (i / BTN_MATRIX_COL) * (BTN_HEIGHT + PAD_TINY) + yo;

    LogicalSwitchData* ls = lswAddress(i);
    bool isActive = (ls->func != LS_FUNC_NONE);

    strAppendSigned(&lsString[1], i + 1, 2);

    if (isActive) {
      auto button = new LogicalSwitchDisplayButton(
          window, {x, y, BTN_WIDTH, BTN_HEIGHT}, lsString, i);

      button->setFocusHandler([=](bool focus) {
        if (focus) {
          footer->setIndex(i);
        }
        return 0;
      });
    } else {
      auto lbl = lv_label_create(window->getLvObj());
      lv_obj_set_size(lbl, BTN_WIDTH, BTN_HEIGHT);
      lv_obj_set_pos(lbl, x, y);
      etx_obj_add_style(lbl, styles->text_align_center, LV_PART_MAIN);
      etx_txt_color(lbl, COLOR_THEME_DISABLED_INDEX);
      lv_label_set_text(lbl, lsString.c_str());
    }
  }
}
