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

#include "model_curves.h"
#include "opentx.h"
#include "libopenui.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

#define CURVE_BTN_W 142
#define CURVE_BTH_H 180

DEFINE_LZ4_BITMAP(LBM_DOT);

class CurveButton : public Button {
  public:
    CurveButton(Window * parent, const rect_t &rect, uint8_t index) :
      Button(parent, rect, nullptr, 0, 0, lv_btn_create),
      index(index)
    {
      padAll(0);
      setWidth(CURVE_BTN_W);
      setHeight(CURVE_BTH_H);
      preview = new CurveRenderer({9, 29, CURVE_BTN_W - 18, CURVE_BTN_W - 18},
                                  [=](int x) -> int {
                                    return applyCustomCurve(x, index);
                                  });
    }

    void paint(BitmapBuffer * dc) override
    {
      LcdFlags bg_color = hasFocus() ? COLOR_THEME_FOCUS : COLOR_THEME_SECONDARY2;
      LcdFlags txt_color = hasFocus() ? COLOR_THEME_PRIMARY2 : COLOR_THEME_SECONDARY1;
      dc->drawSolidFilledRect(8, 0, CURVE_BTN_W - 16, 8, bg_color);
      dc->drawSolidFilledRect(0, 8, CURVE_BTN_W, 14, bg_color);
      dc->drawBitmapPattern(0, 0, LBM_DOT, bg_color);
      dc->drawBitmapPattern(CURVE_BTN_W-13, 0, LBM_DOT, bg_color);

      dc->drawText(CURVE_BTN_W / 2, 1, getCurveString(1 + index), txt_color|CENTERED|FONT(BOLD));

      preview->paint(dc);

      // curve characteristics
      char buf[32];
      CurveHeader &curve = g_model.curves[index];
      snprintf(buf, 32, "%s %d %s", STR_CURVE_TYPES[curve.type], 5 + curve.points, STR_PTS);
      dc->drawText(CURVE_BTN_W / 2, CURVE_BTH_H - 26, buf, COLOR_THEME_SECONDARY1|CENTERED|FONT(BOLD));
    }

    void select(bool selected) {
      if (selected)
        lv_obj_add_state(lvobj, LV_STATE_FOCUSED);
      else
        lv_obj_clear_state(lvobj, LV_STATE_FOCUSED);
    }

  protected:
    uint8_t index;
    CurveRenderer* preview;
};

// initialize a new curves points to the default for a 5 point
// curve.
void initPoints(const CurveHeader &curve, int8_t *points)
{
  int dx = 2000 / (5 + curve.points - 1);
  for (uint8_t i = 0; i < 5 + curve.points; i++) {
    int x = -1000 + i * dx;
    points[i] = x / 10;
  }
}

ModelCurvesPage::ModelCurvesPage() :
  PageTab(STR_MENUCURVES, ICON_MODEL_CURVES)
{
}

// can be called from any other screen to edit a curve.
// currently called from model_mixes.cpp on longpress.
void ModelCurvesPage::pushEditCurve(int index)
{
  if (! isCurveUsed(index)) {
    CurveHeader &curve = g_model.curves[index];
    int8_t * points = curveAddress(index);
    initPoints(curve, points);
  }
  
  new CurveEditWindow(index);
}

#if LCD_W > LCD_H
  #define   PER_ROW     3
#else
  #define   PER_ROW     2
#endif

void ModelCurvesPage::rebuild(FormWindow * window)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());  
  window->clear();
  build(window);

  if (focusButton) {
    lv_obj_scroll_to_y(window->getLvObj(), (focusIndex / PER_ROW) * (CURVE_BTH_H + 8), LV_ANIM_OFF);
  } else {
    lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
  }
}

void ModelCurvesPage::editCurve(FormWindow * window, uint8_t curve)
{
  Window * editWindow = new CurveEditWindow(curve);
  editWindow->setCloseHandler([=]() {
    rebuild(window);
  });
}

void ModelCurvesPage::presetMenu(FormWindow * window, uint8_t index)
{
  Menu *menu = new Menu(window);
  for (int angle = -45; angle <= 45; angle += 15) {
    char label[16];
    strAppend(strAppendSigned(label, angle), "°");
    menu->addLineBuffered(label, [=]() {
      CurveHeader &curve = g_model.curves[index];
      int8_t * points = curveAddress(index);

      int dx = 2000 / (5 + curve.points - 1);
      for (uint8_t i = 0; i < 5 + curve.points; i++) {
        int x = -1000 + i * dx;
        points[i] = divRoundClosest(angle * x, 450);
      }
      if (curve.type == CURVE_TYPE_CUSTOM) {
       resetCustomCurveX(points, 5 + curve.points);
      }

      storageDirty(EE_MODEL);
      rebuild(window);
    });
  }
  menu->updateLines();
}

void ModelCurvesPage::build(FormWindow * window)
{
#if LCD_W > LCD_H
  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
#else
  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
#endif
  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  auto form = new FormWindow(window, rect_t{});
  form->setFlexLayout();

  FlexGridLayout grid(col_dsc, row_dsc);
  
  FormWindow::Line* line;

  CurveEdit::SetCurrentSource(0);

  focusButton = nullptr;

  uint8_t curveIndex = 0;
  uint8_t firstCurveIndex;
  CurveButton* firstCurveButton = nullptr;

  for (uint8_t index = 0; index < MAX_CURVES; index++) {
    if (isCurveUsed(index)) {
      if ((curveIndex % PER_ROW) == 0) {
        line = form->newLine(&grid);
        lv_obj_set_grid_align(line->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_SPACE_BETWEEN);
      }

      CurveHeader &curve = g_model.curves[index];
      int8_t * points = curveAddress(index);

      // Curve drawing
      auto * button = new CurveButton(line, rect_t{}, index);
      button->setPressHandler([=]() -> uint8_t {
          Menu * menu = new Menu(window);
          menu->addLine(STR_EDIT, [=]() {
              editCurve(window, index);
          });
          menu->addLine(STR_CURVE_PRESET, [=]() { presetMenu(window, index); });
          menu->addLine(STR_MIRROR, [=]() {
              curveMirror(index);
              storageDirty(EE_MODEL);
              button->invalidate();
          });
          menu->addLine(STR_CLEAR, [=]() {
              curveClear(index);
              storageDirty(EE_MODEL);
              rebuild(window);
          });
          return 0;
      });
      button->setFocusHandler([=](bool hasFocus) {
          if (focusButton) {
            focusButton->select(false);
          }
          if (hasFocus) {
            focusIndex = index;
            focusButton = button;
          }
      });

      if (!firstCurveButton) {
        firstCurveIndex = index;
        firstCurveButton = button;
      }

      if (index == focusIndex) {
        focusButton = button;
        focusButton->select(true);
      } else {
        button->select(false);
      }

      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER, curveIndex % PER_ROW, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      curveIndex += 1;
    }
  }

  if (!focusButton && firstCurveButton) {
    focusIndex = firstCurveIndex;
    focusButton = firstCurveButton;
    focusButton->select(true);
  }

  if (curveIndex < MAX_CURVES) {
    for (uint8_t n = 0; n < MAX_CURVES; n += 1) {
      if (!isCurveUsed(n)) {
        if ((curveIndex % PER_ROW) == 0) {
          line = form->newLine(&grid);
          lv_obj_set_grid_align(line->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_SPACE_BETWEEN);
        }

        CurveHeader &curve = g_model.curves[n];
        int8_t * points = curveAddress(n);

        auto button = new TextButton(line, rect_t{0, 0, CURVE_BTN_W, CURVE_BTH_H}, LV_SYMBOL_PLUS, [=]() {
          Menu *menu = new Menu(window);
          menu->addLine(STR_EDIT, [=]() {
              focusIndex = n;
              initPoints(curve, points);
              editCurve(window, n);
          });
          menu->addLine(STR_CURVE_PRESET, [=]() {
              focusIndex = n;
              presetMenu(window, n);
          });
          return 0;
        });

        lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER, curveIndex % PER_ROW, 1, LV_GRID_ALIGN_CENTER, 0, 1);

        break;
      }
    }
  }
}
