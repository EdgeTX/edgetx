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

#include "opentx.h"
#include "view_logical_switches.h"


#if LCD_W > LCD_H  // Landscape

#define BTN_MATRIX_COL 8
#define BTN_HEIGHT 20
#define FOOTER_HEIGHT 20
#define V2_COL_CNT 1
#define ANDSW_ROW 0
#define ANDSW_COL 3

// Switch grid
static const lv_coord_t col_dsc[] = {
    LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
    LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1),
    LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};

// Footer grid
static const lv_coord_t f_col_dsc[] = {
    60, LV_GRID_FR(1), 112, LV_GRID_FR(1), 50,
    50, LV_GRID_TEMPLATE_LAST};

#else  // Portrait

#define BTN_MATRIX_COL 4
#define BTN_HEIGHT 21
#define FOOTER_HEIGHT 40
#define V2_COL_CNT 2
#define ANDSW_ROW 1
#define ANDSW_COL 1

static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_FR(1), LV_GRID_FR(1),
                                     LV_GRID_TEMPLATE_LAST};

// Footer grid
static const lv_coord_t f_col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_TEMPLATE_LAST};

#endif

static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT,
                                     LV_GRID_TEMPLATE_LAST};

void getsEdgeDelayParam(char* s, LogicalSwitchData* ls);

class LogicalSwitchDisplayFooter : public Window
{
 public:
  explicit LogicalSwitchDisplayFooter(Window* parent, rect_t rect) :
      Window(parent, rect, OPAQUE)
  {
    padAll(0);
    padLeft(4);
    padRight(4);
    lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY1), 0);

    lv_obj_set_layout(lvobj, LV_LAYOUT_GRID);
    lv_obj_set_grid_dsc_array(lvobj, f_col_dsc, row_dsc);
    lv_obj_set_style_pad_row(lvobj, 0, 0);
    lv_obj_set_style_pad_column(lvobj, 2, 0);

    lsFunc = lv_label_create(lvobj);
    lv_obj_set_style_text_align(lsFunc, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(lsFunc, LV_GRID_ALIGN_STRETCH, 0, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_style_text_color(lsFunc, makeLvColor(COLOR_THEME_PRIMARY2), 0);

    lsV1 = lv_label_create(lvobj);
    lv_obj_set_style_text_align(lsV1, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(lsV1, LV_GRID_ALIGN_STRETCH, 1, 1,
                         LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_style_text_color(lsV1, makeLvColor(COLOR_THEME_PRIMARY2), 0);

    lsV2 = lv_label_create(lvobj);
    lv_obj_set_style_text_align(lsV2, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(lsV2, LV_GRID_ALIGN_STRETCH, 2, V2_COL_CNT,
                         LV_GRID_ALIGN_CENTER, 0, 1);
    lv_obj_set_style_text_color(lsV2, makeLvColor(COLOR_THEME_PRIMARY2), 0);

    lsAnd = lv_label_create(lvobj);
    lv_obj_set_style_text_align(lsAnd, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(lsAnd, LV_GRID_ALIGN_STRETCH, ANDSW_COL, 1,
                         LV_GRID_ALIGN_CENTER, ANDSW_ROW, 1);
    lv_obj_set_style_text_color(lsAnd, makeLvColor(COLOR_THEME_PRIMARY2), 0);

    lsDuration = lv_label_create(lvobj);
    lv_obj_set_style_text_align(lsDuration, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(lsDuration, LV_GRID_ALIGN_STRETCH, ANDSW_COL + 1, 1,
                         LV_GRID_ALIGN_CENTER, ANDSW_ROW, 1);
    lv_obj_set_style_text_color(lsDuration, makeLvColor(COLOR_THEME_PRIMARY2),
                                0);

    lsDelay = lv_label_create(lvobj);
    lv_obj_set_style_text_align(lsDelay, LV_TEXT_ALIGN_LEFT, 0);
    lv_obj_set_grid_cell(lsDelay, LV_GRID_ALIGN_STRETCH, ANDSW_COL + 2, 1,
                         LV_GRID_ALIGN_CENTER, ANDSW_ROW, 1);
    lv_obj_set_style_text_color(lsDelay, makeLvColor(COLOR_THEME_PRIMARY2), 0);

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
      TextButton(parent, rect, std::move(text), nullptr, OPAQUE), index(index)
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
      invalidate();
    }
    Button::checkEvents();
  }

 protected:
  unsigned index = 0;
  bool value = false;
};

void LogicalSwitchesViewPage::build(FormWindow* window)
{
  window->padAll(0);

  auto form = new FormWindow(window, rect_t{});
  form->setFlexLayout();
  form->padAll(0);
  form->padTop(3);

  FlexGridLayout grid(col_dsc, row_dsc, 2);
  FormWindow::Line* line = nullptr;

  // Footer
  footer = new LogicalSwitchDisplayFooter(
      window,
      {0, window->height() - FOOTER_HEIGHT, window->width(), FOOTER_HEIGHT});

  // LSW table
  std::string lsString("L64");
  for (uint8_t i = 0; i < MAX_LOGICAL_SWITCHES; i++) {
    if ((i % BTN_MATRIX_COL) == 0) {
      line = form->newLine(&grid);
      line->padAll(1);
      line->padLeft(4);
      line->padRight(4);
    }

    LogicalSwitchData* ls = lswAddress(i);
    bool isActive = (ls->func != LS_FUNC_NONE);

    strAppendSigned(&lsString[1], i + 1, 2);

    if (isActive) {
      auto button = new LogicalSwitchDisplayButton(
          line, rect_t{0, 0, 0, BTN_HEIGHT}, lsString, i);
      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_STRETCH,
                           i % BTN_MATRIX_COL, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      button->setFocusHandler([=](bool focus) {
        if (focus) {
          footer->setIndex(i);
          footer->invalidate();
        }
        return 0;
      });
    } else {
      auto lbl = lv_label_create(line->getLvObj());
      lv_obj_set_height(lbl, BTN_HEIGHT);
      lv_obj_set_style_text_color(lbl, makeLvColor(COLOR_THEME_DISABLED), 0);
      lv_obj_set_style_text_align(lbl, LV_TEXT_ALIGN_CENTER, 0);
      lv_obj_set_grid_cell(lbl, LV_GRID_ALIGN_STRETCH, i % BTN_MATRIX_COL, 1,
                           LV_GRID_ALIGN_CENTER, 0, 1);
      lv_label_set_text(lbl, lsString.c_str());
    }
  }
}
