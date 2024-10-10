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

#include "curveedit.h"

#include "libopenui.h"
#include "edgetx.h"  // TODO for applyCustomCurve
#include "etx_lv_theme.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

static const lv_coord_t default_col_dsc[] = {LV_GRID_CONTENT,
                                             LV_GRID_TEMPLATE_LAST};
static const lv_coord_t default_row_dsc[] = {LV_GRID_CONTENT,
                                             LV_GRID_TEMPLATE_LAST};

class CurveEdit : public Window
{
 public:
  CurveEdit(Window* parent, const rect_t& rect, uint8_t index) :
      Window(parent, rect),
      preview(
          this, {0, 0, width(), height()},
          [=](int x) -> int { return applyCustomCurve(x, index); },
          [=]() -> int { return getValue(currentSource); }),
      index(index),
      current(0)
  {
    setWindowFlag(NO_FOCUS);
    updatePreview();
  }

  void setCurrentSource(mixsrc_t source)
  {
    currentSource = source;
    if (source)
      lockSource = true;
    else
      lockSource = false;
  }

  void updatePreview()
  {
    preview.clearPoints();
    CurveHeader& curve = g_model.curves[index];
    for (uint8_t i = 0; i < 5 + curve.points; i++) {
      preview.addPoint(getPoint(index, i));
    }
  }

 protected:
  Curve preview;
  uint8_t index;
  uint8_t current;
  mixsrc_t currentSource = 0;
  bool lockSource = false;

  void deleteLater(bool detach = true, bool trash = true) override
  {
    if (!_deleted) {
      preview.deleteLater(true, false);
      Window::deleteLater(detach, trash);
    }
  }

  void checkEvents(void) override
  {
    if (!lockSource) {
      int16_t val = getMovedSource(MIXSRC_FIRST_STICK);
      if (val > 0)
        currentSource = val;
    }
    Window::checkEvents();
  }
};

class CurveDataEdit : public Window
{
 public:
  CurveDataEdit(Window* parent, const rect_t& rect, uint8_t index) :
      Window(parent, rect), index(index)
  {
    etx_scrollbar(lvobj);
    lv_obj_set_style_max_height(lvobj, rect.h, LV_PART_MAIN);

    padAll(PAD_ZERO);
    padBottom(PAD_SMALL);
  }

  void setCurveEdit(CurveEdit* _curveEdit)
  {
    curveEdit = _curveEdit;
    update();
  }

  void update()
  {
    clear();

    memset(numEditX, 0, sizeof(numEditX));

    CurveHeader& curve = g_model.curves[index];
    uint8_t curvePointsCount = 5 + curve.points;

    coord_t y = 0;
    for (int i = 0; i < curvePointsCount; i += 5) {
      int count = 5;
      if (i + count > curvePointsCount) count = curvePointsCount - i;
      curvePointsRow(this, y, i, count, curvePointsCount,
                     curve.type == CURVE_TYPE_CUSTOM);
      y += ROW_HEIGHT;
    }
  }

  static LAYOUT_VAL(ROW_HEIGHT, 82, 82)
  static LAYOUT_VAL(NUM_BTN_WIDTH, 48, 48)
  static LAYOUT_VAL(NUM_HDR_HEIGHT, 15, 15)
  static LAYOUT_VAL(PTNUM_X, 15, 15)
  static LAYOUT_VAL(PTNUM_H, 13, 13)

 protected:
  uint8_t index;
  CurveEdit* curveEdit;
  NumberEdit* numEditX[16];

  void curvePointsRow(Window* parent, coord_t y, int start, int count,
                      int curvePointsCount, bool isCustom)
  {
    // Point number
    for (int i = 0; i < count; i++) {
      new StaticText(parent,
                     {PTNUM_X + (i * (NUM_BTN_WIDTH + PAD_TINY)), y,
                      NUM_BTN_WIDTH, PTNUM_H},
                     std::to_string(i + start + 1),
                     COLOR_THEME_PRIMARY1_INDEX, FONT(XS) | CENTERED);
    }

    y += NUM_HDR_HEIGHT;

    new StaticText(
        parent, {1, y + PAD_MEDIUM, PTNUM_X, EdgeTxStyles::UI_ELEMENT_HEIGHT},
        "X", COLOR_THEME_PRIMARY1_INDEX, CENTERED);

    int8_t* points = curveAddress(index);

    // x value
    if (isCustom) {
      uint8_t i = 0;
      uint8_t c = count;
      if (start == 0) {
        new StaticText(
            parent,
            {PTNUM_X + (i * (NUM_BTN_WIDTH + PAD_TINY)), y + PAD_MEDIUM,
             NUM_BTN_WIDTH, EdgeTxStyles::UI_ELEMENT_HEIGHT},
            "-100", COLOR_THEME_SECONDARY1_INDEX, CENTERED);
        i += 1;
      }
      if ((start + count) == curvePointsCount) {
        c -= 1;
      }
      // Adjustable points for custom curves
      for (; i < c; i++) {
        uint8_t px = i + start - 1;
        numEditX[px] = new NumberEdit(
            parent,
            {PTNUM_X + (i * (NUM_BTN_WIDTH + PAD_TINY)), y, NUM_BTN_WIDTH,
             EdgeTxStyles::UI_ELEMENT_HEIGHT},
            (px == 0) ? -100 : points[curvePointsCount + px - 1],
            (px == curvePointsCount - 3) ? 100
                                         : points[curvePointsCount + px + 1],
            GET_VALUE(points[curvePointsCount + px]),
            [=](int32_t newValue) {
              points[curvePointsCount + px] = newValue;
              if (px > 0) {
                numEditX[px - 1]->setMax(newValue);
              }
              if (px < curvePointsCount - 3) {
                numEditX[px + 1]->setMin(newValue);
              }
              SET_DIRTY();
              curveEdit->updatePreview();
            },
            CENTERED);
      }
      if ((start + count) == curvePointsCount) {
        new StaticText(
            parent,
            {PTNUM_X + (i * (NUM_BTN_WIDTH + PAD_TINY)), y + PAD_MEDIUM,
             NUM_BTN_WIDTH, EdgeTxStyles::UI_ELEMENT_HEIGHT},
            "100", COLOR_THEME_SECONDARY1_INDEX, CENTERED);
      }
    } else {
      for (uint8_t i = 0; i < count; i++) {
        new StaticText(
            parent,
            rect_t{PTNUM_X + (i * (NUM_BTN_WIDTH + PAD_TINY)), y + PAD_MEDIUM,
                   NUM_BTN_WIDTH, EdgeTxStyles::UI_ELEMENT_HEIGHT},
            std::to_string(-100 + (200 * (i + start)) / (curvePointsCount - 1)),
            COLOR_THEME_SECONDARY1_INDEX, CENTERED);
      }
    }

    y += EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY;

    new StaticText(
        parent, {1, y + PAD_MEDIUM, PTNUM_X, EdgeTxStyles::UI_ELEMENT_HEIGHT},
        "Y", COLOR_THEME_PRIMARY1_INDEX, CENTERED);

    // y value
    for (uint8_t i = 0; i < count; i++) {
      new NumberEdit(parent,
          {PTNUM_X + (i * (NUM_BTN_WIDTH + PAD_TINY)), y, NUM_BTN_WIDTH,
           EdgeTxStyles::UI_ELEMENT_HEIGHT},
          -100, 100, GET_VALUE(points[i + start]),
          [=](int32_t newValue) {
            points[i + start] = newValue;
            SET_DIRTY();
            curveEdit->updatePreview();
          },
          CENTERED);
    }
  }
};

CurveEditWindow::CurveEditWindow(uint8_t index,
                                 std::function<void(void)> refreshView) :
    Page(ICON_MODEL_CURVES, PAD_ZERO), index(index), refreshView(refreshView)
{
  buildBody(body);
  buildHeader(header);
}

void CurveEditWindow::buildHeader(Window* window)
{
  header->setTitle(STR_MENUCURVES);
  char s[16];
  strAppendStringWithIndex(s, STR_CV, index + 1);
  header->setTitle2(s);
}

void CurveEditWindow::buildBody(Window* window)
{
  CurveHeader& curve = g_model.curves[index];
  int8_t* points = curveAddress(index);

  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid(default_col_dsc, default_row_dsc, PAD_ZERO);

  auto line = window->newLine(grid);
  line->padAll(PAD_ZERO);
  lv_obj_set_grid_align(line->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN,
                        LV_GRID_ALIGN_SPACE_BETWEEN);

#if PORTRAIT_LCD
  lv_obj_set_flex_flow(line->getLvObj(), LV_FLEX_FLOW_COLUMN);
  coord_t boxWidth = window->width();
  coord_t boxHeight = window->height() - CURVE_WIDTH;
#else
  lv_obj_set_flex_flow(line->getLvObj(), LV_FLEX_FLOW_ROW);
  coord_t boxWidth = window->width() - CURVE_WIDTH;
  coord_t boxHeight = window->height();
#endif

  auto box = new Window(line, rect_t{});
  box->setWidth(boxWidth);
  box->setHeight(boxHeight);
  box->padAll(PAD_ZERO);

  static const lv_coord_t controls_col_dsc[] = {
      LV_GRID_FR(5), LV_GRID_FR(8), LV_GRID_FR(5), LV_GRID_TEMPLATE_LAST};

  auto form = new Window(box, rect_t{});
  form->padAll(PAD_ZERO);
  form->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout iGrid(controls_col_dsc, default_row_dsc, PAD_ZERO);

  auto iLine = form->newLine(iGrid);
  iLine->padAll(PAD_TINY);
  lv_obj_set_grid_align(iLine->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN,
                        LV_GRID_ALIGN_SPACE_BETWEEN);

  // Name
  new StaticText(iLine, rect_t{}, STR_NAME);
  new ModelTextEdit(iLine, rect_t{}, curve.name, sizeof(curve.name));

  // Smooth
  auto smooth =
      new TextButton(iLine, rect_t{0, 0, NUMEDT_W, 0}, STR_SMOOTH, [=]() {
        g_model.curves[index].smooth = !g_model.curves[index].smooth;
        curveEdit->updatePreview();
        return g_model.curves[index].smooth;
      });
  smooth->check(g_model.curves[index].smooth);

  iLine = form->newLine(iGrid);
  iLine->padAll(PAD_TINY);
  lv_obj_set_grid_align(iLine->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN,
                        LV_GRID_ALIGN_SPACE_BETWEEN);

  // Type
  new StaticText(iLine, rect_t{}, STR_TYPE);
  new Choice(
      iLine, {0, 0, TextEdit::DEF_W, 0}, STR_CURVE_TYPES, 0, 1,
      GET_DEFAULT(g_model.curves[index].type), [=](int32_t newValue) {
        CurveHeader& curve = g_model.curves[index];
        if (newValue != curve.type) {
          for (int i = 1; i < 4 + curve.points; i++) {
            points[i] = calcRESXto100(applyCustomCurve(
                calc100toRESX(-100 + i * 200 / (4 + curve.points)), index));
          }
          if (moveCurve(index, newValue == CURVE_TYPE_CUSTOM
                                   ? 3 + curve.points
                                   : -3 - curve.points)) {
            if (newValue == CURVE_TYPE_CUSTOM) {
              resetCustomCurveX(points, 5 + curve.points);
            }
            curve.type = newValue;
          }
          SET_DIRTY();
          curveEdit->updatePreview();
          if (curveDataEdit) {
            curveDataEdit->update();
          }
        }
      });

  // Points count
  auto edit = new Choice(
      iLine, {0, 0, NUMEDT_W, 0}, 2, 17,
      GET_DEFAULT(g_model.curves[index].points + 5), [=](int32_t newValue) {
        newValue -= 5;
        CurveHeader& curve = g_model.curves[index];
        int newPoints[MAX_POINTS_PER_CURVE];
        newPoints[0] = points[0];
        newPoints[4 + newValue] = points[4 + curve.points];
        for (int i = 1; i < 4 + newValue; i++)
          newPoints[i] = calcRESXto100(
              applyCustomCurve(-RESX + (i * 2 * RESX) / (4 + newValue), index));
        if (moveCurve(index, (newValue - curve.points) *
                                 (curve.type == CURVE_TYPE_CUSTOM ? 2 : 1))) {
          for (int i = 0; i < 5 + newValue; i++) {
            points[i] = newPoints[i];
            if (curve.type == CURVE_TYPE_CUSTOM && i != 0 && i != 4 + newValue)
              points[5 + newValue + i - 1] = -100 + (i * 200) / (4 + newValue);
          }
          curve.points = newValue;
          SET_DIRTY();
          curveEdit->updatePreview();
          if (curveDataEdit) {
            curveDataEdit->update();
          }
        }
      });
  edit->setTextHandler([=](int value) {
    return std::to_string(value) + STR_PTS;
  });

  iLine = form->newLine(iGrid);
  iLine->padAll(PAD_ZERO);
  lv_obj_set_grid_align(iLine->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN,
                        LV_GRID_ALIGN_SPACE_BETWEEN);

  curveDataEdit = new CurveDataEdit(
      iLine,
      rect_t{
          0, 0, box->width(),
          box->height() - (EdgeTxStyles::UI_ELEMENT_HEIGHT + PAD_TINY * 2) * 2},
      index);

  // Curve editor
  lv_obj_set_flex_align(line->getLvObj(), LV_FLEX_ALIGN_CENTER,
                        LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
  curveEdit = new CurveEdit(line, {0, 0, CURVE_WIDTH, CURVE_WIDTH}, index);

  curveDataEdit->setCurveEdit(curveEdit);
}

void CurveEditWindow::onCancel()
{
  if (refreshView) refreshView();
  Page::onCancel();
}

void CurveEditWindow::setCurrentSource(mixsrc_t source)
{
  curveEdit->setCurrentSource(source);
}
