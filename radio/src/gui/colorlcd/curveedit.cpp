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
#include "opentx.h" // TODO for applyCustomCurve
#include "libopenui.h"

#define SET_DIRTY()     storageDirty(EE_MODEL)

CurveDataEdit::CurveDataEdit(Window * parent, const rect_t & rect, uint8_t index) :
  Window(parent, rect),
  index(index)
{
  lv_obj_set_style_bg_color(lvobj, makeLvColor(COLOR_THEME_SECONDARY3), 0);
  lv_obj_set_scrollbar_mode(lvobj, LV_SCROLLBAR_MODE_AUTO);
}

void CurveDataEdit::setCurveEdit(CurveEdit* _curveEdit)
{
  curveEdit = _curveEdit;
  update();
}

#define BTN_H   36

void CurveDataEdit::setPointText()
{
  int n = curveEdit->getCurrent();
  char buf[32];
  sprintf(buf, "#%d X:%d Y:%d", n+1, curveEdit->getX(), curveEdit->getY());
  pointText->setText(buf);
  if (curveEdit->isEditableX()) {
    lv_obj_clear_state(incX1->getLvObj(), LV_STATE_DISABLED);
    lv_obj_clear_state(decX1->getLvObj(), LV_STATE_DISABLED);
    lv_obj_clear_state(incX5->getLvObj(), LV_STATE_DISABLED);
    lv_obj_clear_state(decX5->getLvObj(), LV_STATE_DISABLED);
  } else {
    lv_obj_add_state(incX1->getLvObj(), LV_STATE_DISABLED);
    lv_obj_add_state(decX1->getLvObj(), LV_STATE_DISABLED);
    lv_obj_add_state(incX5->getLvObj(), LV_STATE_DISABLED);
    lv_obj_add_state(decX5->getLvObj(), LV_STATE_DISABLED);
  }
}

void CurveDataEdit::update()
{
  clear();

  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  FormBuilder form(this, 4, col_dsc, row_dsc);

  auto line = form.newLine(4);
  line->padTop(4);

  form.cell((new TextButton(line, rect_t{0, 0, 50, BTN_H}, "Prev", [=]() {
    curveEdit->selectPoint(-1);
    setPointText();
    return 0;
  })), LV_GRID_ALIGN_CENTER, 0, 1);

  form.cell((new TextButton(line, rect_t{0, 0, 50, BTN_H}, "Y+1", [=]() {
    curveEdit->up(1);
    setPointText();
    return 0;
  })), LV_GRID_ALIGN_CENTER, 1, 1);

  form.cell((new TextButton(line, rect_t{0, 0, 50, BTN_H}, "Y+5", [=]() {
    curveEdit->up(5);
    setPointText();
    return 0;
  })), LV_GRID_ALIGN_CENTER, 2, 1);

  form.cell((new TextButton(line, rect_t{0, 0, 50, BTN_H}, "Next", [=]() {
    curveEdit->selectPoint(1);
    setPointText();
    return 0;
  })), LV_GRID_ALIGN_CENTER, 3, 1);

  line = form.newLine(4);

  decX1 = new TextButton(line, rect_t{0, 0, 50, BTN_H}, "X-1", [=]() {
    curveEdit->left(1);
    setPointText();
    return 0;
  });
  form.cell(decX1, LV_GRID_ALIGN_CENTER, 0, 1);

  pointText = new StaticText(line, rect_t{}, "", CENTERED|COLOR_THEME_PRIMARY1);
  form.cell(pointText, LV_GRID_ALIGN_CENTER, 1, 2);

  incX1 = new TextButton(line, rect_t{0, 0, 50, BTN_H}, "X+1", [=]() {
    curveEdit->right(1);
    setPointText();
    return 0;
  });
  form.cell(incX1, LV_GRID_ALIGN_CENTER, 3, 1);

  line = form.newLine(4);

  decX5 = new TextButton(line, rect_t{0, 0, 50, BTN_H}, "X-5", [=]() {
    curveEdit->left(5);
    setPointText();
    return 0;
  });
  form.cell(decX5, LV_GRID_ALIGN_CENTER, 0, 1);

  form.cell((new TextButton(line, rect_t{0, 0, 50, BTN_H}, "Y-1", [=]() {
    curveEdit->down(1);
    setPointText();
    return 0;
  })), LV_GRID_ALIGN_CENTER, 1, 1);

  form.cell((new TextButton(line, rect_t{0, 0, 50, BTN_H}, "Y-5", [=]() {
    curveEdit->down(5);
    setPointText();
    return 0;
  })), LV_GRID_ALIGN_CENTER, 2, 1);

  incX5 = new TextButton(line, rect_t{0, 0, 50, BTN_H}, "X+5", [=]() {
    curveEdit->right(5);
    setPointText();
    return 0;
  });
  form.cell(incX5, LV_GRID_ALIGN_CENTER, 3, 1);

  setPointText();
}

void CurveEdit::SetCurrentSource(uint32_t source)
{
  CurveEdit::currentSource = source;
  if (source)
    lockSource = true;
  else
    lockSource = false;
}

mixsrc_t CurveEdit::currentSource = 0;
bool CurveEdit::lockSource = false;

CurveEdit::CurveEdit(Window* parent, const rect_t& rect, uint8_t index) :
    FormField(parent, rect, NO_FOCUS),
    preview(
        this, {0, 0, width(), height()},
        [=](int x) -> int { return applyCustomCurve(x, index); },
        [=]()->int {
          return getValue(CurveEdit::currentSource);
        },
        [=]()->int { return getPoint(index, current).x; }),
    index(index),
    current(0)
{
  TRACE("CurveEdit::currentSource=%d\tCurveEdit::lockSource=%d",
        CurveEdit::currentSource, CurveEdit::lockSource);
  updatePreview();
}

void CurveEdit::updatePreview()
{
  // Reset current if # of points changed
  selectPoint(0);

  preview.clearPoints();
  for (uint8_t i = 0; i < getCurvePointsCount(); i++) {
    preview.addPoint(getPoint(index, i), COLOR_THEME_SECONDARY1);
  }
  invalidate();
}

void CurveEdit::selectPoint(int8_t chg)
{
  int8_t n = (int8_t)current + chg;
  int8_t max= getCurvePointsCount() - 1;
  if (n < 0)
    n = max;
  else if (n > max)
    n = (chg) ? 0 : max;
  if (current != n) {
    current = n;
    if (chg)
      updatePreview();
  }
}

void CurveEdit::up(int8_t amt)
{
  int8_t & point = curveAddress(index)[current];
  int8_t n = min<int8_t>(100, point + amt);
  if (n != point) {
    point = n;
    storageDirty(EE_MODEL);
    updatePreview();
  }
}

void CurveEdit::down(int8_t amt)
{
  int8_t & point = curveAddress(index)[current];
  int8_t n = max<int8_t>(-100, point - amt);
  if (n != point) {
    point = n;
    storageDirty(EE_MODEL);
    updatePreview();
  }
}

void CurveEdit::right(int8_t amt)
{
  if (isEditableX()) {
    CurveHeader & curve = g_model.curves[index];
    int8_t * point = &curveAddress(index)[5 + curve.points + current - 1];
    int8_t xmax = (current == (curve.points - 2) ? +100 : point[1]);
    int8_t n = min<int8_t>(*point + amt, xmax-1);
    if (n != *point) {
      *point = n;
      storageDirty(EE_MODEL);
      updatePreview();
    }
  }
}

void CurveEdit::left(int8_t amt)
{
  if (isEditableX()) {
    CurveHeader & curve = g_model.curves[index];
    int8_t * point = &curveAddress(index)[5 + curve.points + current - 1];
    int8_t xmin = (current == 1 ? -100 : point[-1]);
    int8_t n = max<int8_t>(xmin+1, *point - amt);
    if (n != *point) {
      *point = n;
      storageDirty(EE_MODEL);
      updatePreview();
    }
  }
}

bool CurveEdit::isCustomCurve() const
{
  return g_model.curves[index].type == CURVE_TYPE_CUSTOM;
}

bool CurveEdit::isEditableX() const
{
  return isCustomCurve() && (current > 0) &&  (current < getCurvePointsCount() - 1);
}

uint8_t CurveEdit::getCurvePointsCount() const
{
  return g_model.curves[index].points + 5;
}

int8_t CurveEdit::getX() const
{
  int n = getCurvePointsCount() - 1;
  if (isCustomCurve()) {
    if (current == 0) return -100;
    if (current == n) return 100;
    return curveAddress(index)[current + n];
  }
  return -100 + (200 * current) / n;
}

int8_t CurveEdit::getY() const
{
  return curveAddress(index)[current];
}

void CurveEdit::onEvent(event_t event)
{
  TRACE_WINDOWS("%s received event 0x%X", getWindowDebugString().c_str(), event);

  switch (event) {
#if defined(HARDWARE_TOUCH)
    case EVT_VIRTUAL_KEY_LEFT:
      left(1);
      break;

    case EVT_VIRTUAL_KEY_RIGHT:
      right(1);
      break;

    case EVT_VIRTUAL_KEY_UP:
      up(1);
      break;

    case EVT_VIRTUAL_KEY_DOWN:
      down(1);
      break;

    case EVT_VIRTUAL_KEY_PREVIOUS:
      selectPoint(-1);
      break;

    case EVT_VIRTUAL_KEY_NEXT:
      selectPoint(1);
      break;
#endif

    default:
      FormField::onEvent(event);
      break;
  }
}

void CurveEdit::checkEvents()
{
  if (!lockSource) {
    int16_t val = getMovedSource(MIXSRC_FIRST_INPUT);

    if (val > 0) {
      // TODO: this code seems odd
      if (val > MAX_STICKS + MAX_POTS)
        CurveEdit::currentSource = val + 1 - MIXSRC_FIRST_INPUT;
      else {
        CurveEdit::currentSource = expoAddress(val - 1)->srcRaw;
        TRACE("Detected source=%d", val);
      }
      TRACE("Applied source=%d", CurveEdit::currentSource);
    }
  }
  FormField::checkEvents();
}

CurveEditWindow::CurveEditWindow(uint8_t index):
  Page(ICON_MODEL_CURVES),
  index(index)
{
  buildBody(&body);
  buildHeader(&header);
}

void CurveEditWindow::buildHeader(Window * window)
{
  header.setTitle(STR_MENUCURVES);
  char s[16];
  strAppendStringWithIndex(s, STR_CV, index + 1);
  header.setTitle2(s);
}

void CurveEditWindow::buildBody(FormWindow * window)
{
  CurveHeader & curve = g_model.curves[index];
  int8_t * points = curveAddress(index);

  auto form = new FormWindow(window, rect_t{});
  form->padAll(0);
  form->setFlexLayout();

  FlexGridLayout grid(default_col_dsc, default_row_dsc, 0);
  
  auto line = form->newLine(&grid);
  line->padAll(0);
  lv_obj_set_grid_align(line->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_SPACE_BETWEEN);

#if LCD_H > LCD_W // portrait
  lv_obj_set_flex_flow(line->getLvObj(), LV_FLEX_FLOW_COLUMN);
  coord_t curveWidth = window->width() - 88;
  coord_t boxWidth = window->width();
  coord_t boxHeight = window->height() - curveWidth;
#else
  lv_obj_set_flex_flow(line->getLvObj(), LV_FLEX_FLOW_ROW);
  coord_t curveWidth = window->height() - 2 * PAGE_PADDING;
  coord_t boxWidth = window->width() - curveWidth;
  coord_t boxHeight = window->height();
#endif

  auto box = new Window(line, rect_t{});
  box->setWidth(boxWidth);
  box->setHeight(boxHeight);
  box->padAll(0);

  static const lv_coord_t controls_col_dsc[] = {LV_GRID_FR(5), LV_GRID_FR(8), LV_GRID_FR(5), LV_GRID_TEMPLATE_LAST};

  form = new FormWindow(box, rect_t{});
  form->padAll(0);
  form->setFlexLayout();

  FlexGridLayout iGrid(controls_col_dsc, default_row_dsc, 0);
  
  auto iLine = form->newLine(&iGrid);
  iLine->padAll(4);
  iLine->padBottom(0);
  lv_obj_set_grid_align(iLine->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_SPACE_BETWEEN);

  // Name
  new StaticText(iLine, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1);
  new ModelTextEdit(iLine, rect_t{0, 0, 100, 0}, curve.name, sizeof(curve.name));

  // Smooth
  auto smooth = new TextButton(iLine, rect_t{0, 0, 70, 0}, STR_SMOOTH, [=]() {
    g_model.curves[index].smooth = !g_model.curves[index].smooth;
    curveEdit->updatePreview();
    return g_model.curves[index].smooth;
  });
  smooth->check(g_model.curves[index].smooth);
  
  iLine = form->newLine(&iGrid);
  iLine->padAll(4);
  iLine->padBottom(0);
  lv_obj_set_grid_align(iLine->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_SPACE_BETWEEN);

  // Type
  new StaticText(iLine, rect_t{}, STR_TYPE, 0, COLOR_THEME_PRIMARY1);
  new Choice(iLine, rect_t{0, 0, 100, 0}, STR_CURVE_TYPES, 0, 1, GET_DEFAULT(g_model.curves[index].type),
             [=](int32_t newValue) {
                 CurveHeader &curve = g_model.curves[index];
                 if (newValue != curve.type) {
                   for (int i = 1; i < 4 + curve.points; i++) {
                     points[i] = calcRESXto100(applyCustomCurve(calc100toRESX(-100 + i * 200 / (4 + curve.points)), index));
                   }
                   if (moveCurve(index, newValue == CURVE_TYPE_CUSTOM ? 3 + curve.points : -3 - curve.points)) {
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
  auto edit = new NumberEdit(iLine, rect_t{0, 0, 70, 0}, 2, 17, GET_DEFAULT(g_model.curves[index].points + 5),
                             [=](int32_t newValue) {
                                 newValue -= 5;
                                 CurveHeader &curve = g_model.curves[index];
                                 int newPoints[MAX_POINTS_PER_CURVE];
                                 newPoints[0] = points[0];
                                 newPoints[4 + newValue] = points[4 + curve.points];
                                 for (int i = 1; i < 4 + newValue; i++)
                                   newPoints[i] = calcRESXto100(applyCustomCurve(-RESX + (i * 2 * RESX) / (4 + newValue), index));
                                 if (moveCurve(index, (newValue - curve.points) * (curve.type == CURVE_TYPE_CUSTOM ? 2 : 1))) {
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
  edit->setSuffix(STR_PTS);
  edit->setDefault(5);

  iLine = form->newLine(&iGrid);
  iLine->padAll(0);
  lv_obj_set_grid_align(iLine->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_SPACE_BETWEEN);

  curveDataEdit = new CurveDataEdit(iLine, rect_t{ 0, 0, box->width(), box->height() - 72 }, index);

  // Curve editor
  lv_obj_set_flex_align(line->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_AROUND);
  curveEdit = new CurveEdit(line, { 0, 0, curveWidth, curveWidth }, index);
  
  curveDataEdit->setCurveEdit(curveEdit);
}
