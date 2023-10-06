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

static const lv_coord_t col_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

class CurveDataEdit : public Window
{
  public:
    CurveDataEdit(Window * parent, const rect_t & rect, uint8_t index);

    void setCurveEdit(CurveEdit* _curveEdit);

    void build();
    void buildSettings();
    void buildList();
    void buildEdit();
    void update();

    void showEdit(uint8_t point);
    void hideEdit();

    void onEvent(event_t event) override;

  protected:
    uint8_t index;
    FormWindow* form = nullptr;
    CurveEdit * curveEdit;
    StaticText* pointNText;
    StaticText* pointXText;
    StaticText* pointYText;
    TextButton* pointButtons[17];
    TextButton* decX1;
    TextButton* incX1;
    TextButton* decX2;
    TextButton* incX2;
    Window* editWindow = nullptr;
    Window* mainWindow = nullptr;
    Window* pointsWindow = nullptr;
    bool isEditing;
    bool symmetry;

    void setX(int8_t chg);
    void setY(int8_t chg);
    void symmetryAdjust();

    void setPointText();
    void setPointsListText(uint8_t n);
};

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
  build();
  setPointText();
}

void CurveDataEdit::setPointText()
{
  int n = curveEdit->getCurrent();
  int8_t x = curveEdit->getX();
  int8_t y = curveEdit->getY();

  char buf[32];
  sprintf(buf, "# %d", n+1);
  pointNText->setText(buf);
  sprintf(buf, "X %d", x);
  pointXText->setText(buf);
  sprintf(buf, "Y %d", y);
  pointYText->setText(buf);

  if (curveEdit->isEditableX()) {
    lv_obj_clear_state(incX1->getLvObj(), LV_STATE_DISABLED);
    lv_obj_clear_state(decX1->getLvObj(), LV_STATE_DISABLED);
    lv_obj_clear_state(incX2->getLvObj(), LV_STATE_DISABLED);
    lv_obj_clear_state(decX2->getLvObj(), LV_STATE_DISABLED);
  } else {
    lv_obj_add_state(incX1->getLvObj(), LV_STATE_DISABLED);
    lv_obj_add_state(decX1->getLvObj(), LV_STATE_DISABLED);
    lv_obj_add_state(incX2->getLvObj(), LV_STATE_DISABLED);
    lv_obj_add_state(decX2->getLvObj(), LV_STATE_DISABLED);
  }

  setPointsListText(n);
}

void CurveDataEdit::setPointsListText(uint8_t n)
{
  char buf[32];
  int8_t x = curveEdit->getX(n);
  int8_t y = curveEdit->getY(n);

  sprintf(buf, "%d,%d", x, y);
  pointButtons[n]->setText(buf);
}

void CurveDataEdit::showEdit(uint8_t point)
{
  curveEdit->showCurrent(true);
  curveEdit->setCurrent(point);
  setPointText();
  curveEdit->updatePreview();
  lv_obj_clear_flag(editWindow->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(mainWindow->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  isEditing = true;
}

void CurveDataEdit::hideEdit()
{
  curveEdit->showCurrent(false);
  curveEdit->updatePreview();
  lv_obj_clear_flag(mainWindow->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  lv_obj_add_flag(editWindow->getLvObj(), LV_OBJ_FLAG_HIDDEN);
  isEditing = false;
}

void CurveDataEdit::onEvent(event_t event)
{
#if defined(HARDWARE_KEYS)
  switch (event) {
    case EVT_KEY_FIRST(KEY_EXIT):
      if (isEditing) {
        killEvents(event);
        hideEdit();
        return;
      }
      break;
  }
#endif
  Window::onEvent(event);
}

#define BTN_W       42
#define BTN_H       40
#define LINE_H      36
#define LINE_BTNS   3

void CurveDataEdit::build()
{
  form = new FormWindow(this, rect_t{});
  form->padAll(0);
  form->setFlexLayout();
  
  FlexGridLayout grid(col_dsc, row_dsc, 4);

  mainWindow = form->newLine(&grid);
  mainWindow->padAll(0);
  lv_obj_set_flex_flow(mainWindow->getLvObj(), LV_FLEX_FLOW_COLUMN);

  buildSettings();
  buildList();
  buildEdit();
  hideEdit();

  setPointText();
  symmetryAdjust();
}

void CurveDataEdit::update()
{
  buildList();
}

void CurveDataEdit::buildSettings()
{
  CurveHeader & curve = g_model.curves[index];
  int8_t * points = curveAddress(index);

  auto box = new Window(mainWindow, rect_t{0, 0, width(), LINE_H * 2});
  box->padLeft(4);
  box->padRight(4);

  static const lv_coord_t col_dsc[] = {LV_GRID_FR(5), LV_GRID_FR(8), LV_GRID_FR(5), LV_GRID_TEMPLATE_LAST};
  static const lv_coord_t row_dsc[] = {LINE_H, LINE_H, LV_GRID_TEMPLATE_LAST};

  lv_obj_set_grid_dsc_array(box->getLvObj(), col_dsc, row_dsc);

  // Name
  lv_obj_set_grid_cell((new StaticText(box, rect_t{}, STR_NAME, 0, COLOR_THEME_PRIMARY1))->getLvObj(), LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  lv_obj_set_grid_cell((new ModelTextEdit(box, rect_t{0, 0, 100, 0}, curve.name, sizeof(curve.name)))->getLvObj(), LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  // Smooth
  auto smooth = new TextButton(box, rect_t{}, STR_SMOOTH, [=]() {
    g_model.curves[index].smooth = !g_model.curves[index].smooth;
    curveEdit->updatePreview();
    return g_model.curves[index].smooth;
  });
  smooth->check(g_model.curves[index].smooth);
  smooth->padAll(2);
  lv_obj_set_grid_cell(smooth->getLvObj(), LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  // Type
  lv_obj_set_grid_cell((new StaticText(box, rect_t{}, STR_TYPE, 0, COLOR_THEME_PRIMARY1))->getLvObj(), LV_GRID_ALIGN_START, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);
  lv_obj_set_grid_cell((new Choice(box, rect_t{0, 0, 100, 0}, STR_CURVE_TYPES, 0, 1, GET_DEFAULT(g_model.curves[index].type),
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
                   update();
                 }
             }))->getLvObj(), LV_GRID_ALIGN_START, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  // Points count
  auto edit = new NumberEdit(box, rect_t{}, 2, 17, GET_DEFAULT(g_model.curves[index].points + 5),
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
                                   update();
                                 }
                             });
  edit->setSuffix(STR_PTS);
  edit->setWidth(width() * 5 / 18 - 4);
  lv_obj_set_grid_cell(edit->getLvObj(), LV_GRID_ALIGN_START, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);
}

void CurveDataEdit::buildList()
{
  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  if (pointsWindow)
  {
    pointsWindow->clear();
  }
  else
  {
    pointsWindow = new Window(mainWindow, rect_t{0, 0, width(), height() - LINE_H * 2 - 4});
    pointsWindow->padLeft(4);
    pointsWindow->padRight(4);
    pointsWindow->padBottom(PAGE_PADDING);

    lv_obj_set_grid_dsc_array(pointsWindow->getLvObj(), col_dsc, row_dsc);
    lv_obj_set_style_pad_all(pointsWindow->getLvObj(), 4, LV_PART_MAIN);
    lv_obj_set_style_pad_row(pointsWindow->getLvObj(), 4, LV_PART_MAIN);
    lv_obj_set_style_pad_column(pointsWindow->getLvObj(), 4, LV_PART_MAIN);
  }

  memset(pointButtons, 0, sizeof(pointButtons));

  char buf[32];
  for (uint8_t i = 0; i < curveEdit->getCurvePointsCount(); i += 1) {
    int8_t x = curveEdit->getX(i);
    int8_t y = curveEdit->getY(i);
    sprintf(buf, "%d,%d", x, y);
    pointButtons[i] = new TextButton(pointsWindow, rect_t{0, 0, 0, BTN_H}, buf, [=]() {
      showEdit(i);
      return 0;
    });
    lv_obj_set_grid_cell(pointButtons[i]->getLvObj(), LV_GRID_ALIGN_STRETCH, i % LINE_BTNS, 1, LV_GRID_ALIGN_CENTER, i / LINE_BTNS, 1);
    sprintf(buf,"%d", i+1);
    lv_obj_set_grid_cell((new StaticText(pointsWindow, rect_t{}, buf, 0, FONT(XS)))->getLvObj(), LV_GRID_ALIGN_START, i % LINE_BTNS, 1, LV_GRID_ALIGN_START, i / LINE_BTNS, 1);
  }
}

void CurveDataEdit::buildEdit()
{
  editWindow = new Window(this, rect_t{0, 0, width(), height()});

  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_CONTENT, 80, LV_GRID_TEMPLATE_LAST};

  lv_obj_set_grid_dsc_array(editWindow->getLvObj(), col_dsc, row_dsc);
  lv_obj_set_style_pad_all(editWindow->getLvObj(), 4, LV_PART_MAIN);
  lv_obj_set_style_pad_row(editWindow->getLvObj(), 4, LV_PART_MAIN);
  lv_obj_set_style_pad_column(editWindow->getLvObj(), 4, LV_PART_MAIN);
  lv_obj_set_style_bg_opa(editWindow->getLvObj(), LV_OPA_100, LV_PART_MAIN);
  lv_obj_set_style_bg_color(editWindow->getLvObj(), makeLvColor(COLOR_THEME_SECONDARY3), LV_PART_MAIN);

  decX2 = new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "-10", [=]() {
    setX(-10);
    return 0;
  });
  lv_obj_set_grid_cell(decX2->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  decX1 = new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "-1", [=]() {
    setX(-1);
    return 0;
  });
  lv_obj_set_grid_cell(decX1->getLvObj(), LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  pointXText = new StaticText(editWindow, rect_t{}, "", CENTERED|COLOR_THEME_PRIMARY1);
  lv_obj_set_grid_cell(pointXText->getLvObj(), LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  incX1 = new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "+1", [=]() {
    setX(1);
    return 0;
  });
  lv_obj_set_grid_cell(incX1->getLvObj(), LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  incX2 = new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "+10", [=]() {
    setX(10);
    return 0;
  });
  lv_obj_set_grid_cell(incX2->getLvObj(), LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 0, 1);

  lv_obj_set_grid_cell((new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "-10", [=]() {
    setY(-10);
    return 0;
  }))->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_set_grid_cell((new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "-1", [=]() {
    setY(-1);
    return 0;
  }))->getLvObj(), LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  pointYText = new StaticText(editWindow, rect_t{}, "", CENTERED|COLOR_THEME_PRIMARY1);
  lv_obj_set_grid_cell(pointYText->getLvObj(), LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_set_grid_cell((new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "+1", [=]() {
    setY(1);
    return 0;
  }))->getLvObj(), LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_set_grid_cell((new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "+10", [=]() {
    setY(10);
    return 0;
  }))->getLvObj(), LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 1, 1);

  lv_obj_set_grid_cell((new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "|<", [=]() {
    curveEdit->setCurrent(0);
    curveEdit->updatePreview();
    symmetryAdjust();
    setPointText();
    return 0;
  }))->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_set_grid_cell((new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, "<", [=]() {
    curveEdit->selectPoint(-1);
    symmetryAdjust();
    setPointText();
    return 0;
  }))->getLvObj(), LV_GRID_ALIGN_CENTER, 1, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  pointNText = new StaticText(editWindow, rect_t{}, "", CENTERED|COLOR_THEME_PRIMARY1);
  lv_obj_set_grid_cell(pointNText->getLvObj(), LV_GRID_ALIGN_CENTER, 2, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_set_grid_cell((new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, ">", [=]() {
    curveEdit->selectPoint(1);
    symmetryAdjust();
    setPointText();
    return 0;
  }))->getLvObj(), LV_GRID_ALIGN_CENTER, 3, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  lv_obj_set_grid_cell((new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, ">|", [=]() {
    curveEdit->setCurrent(curveEdit->getCurvePointsCount() - 1);
    curveEdit->updatePreview();
    symmetryAdjust();
    setPointText();
    return 0;
  }))->getLvObj(), LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_CENTER, 2, 1);

  auto btn = new TextButton(editWindow, rect_t{0, 0, BTN_W*2, BTN_H}, "Symmetry", [=]() {
    symmetry = !symmetry;
    symmetryAdjust();
    return symmetry;
  });
  lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_CENTER, 0, 2, LV_GRID_ALIGN_END, 3, 1);
  btn->check(symmetry);

  btn = new TextButton(editWindow, rect_t{0, 0, BTN_W*2, BTN_H}, "Mirror", [=]() {
    for (uint8_t i = 0; i < curveEdit->getCurvePointsCount();i += 1)
    {
      curveEdit->setY(i, -curveEdit->getY(i));
    }
    return 0;
  });
  lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_CENTER, 2, 2, LV_GRID_ALIGN_END, 3, 1);

  btn = new TextButton(editWindow, rect_t{0, 0, BTN_W, BTN_H}, STR_EXIT, [=]() {
    hideEdit();
    return 0;
  });
  lv_obj_set_grid_cell(btn->getLvObj(), LV_GRID_ALIGN_CENTER, 4, 1, LV_GRID_ALIGN_END, 3, 1);
}

void CurveDataEdit::symmetryAdjust()
{
  if (symmetry) {
    uint8_t c = curveEdit->getCurrent();
    uint8_t n = curveEdit->getCurvePointsCount() - 1 - c;
    if (n != c) {
      int8_t x = curveEdit->getX();
      int8_t y = curveEdit->getY();
      curveEdit->setX(n, -x);
      curveEdit->setY(n, -y);
      setPointsListText(n);
    }
  }
}

void CurveDataEdit::setX(int8_t chg)
{
  curveEdit->setX(chg);
  symmetryAdjust();
  setPointText();
}

void CurveDataEdit::setY(int8_t chg)
{
  curveEdit->setY(chg);
  symmetryAdjust();
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
        [=]()->int { return showEdit ? getPoint(index, current).x : 200; }),
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
  setCurrent(current);

  preview.clearPoints();
  for (uint8_t i = 0; i < getCurvePointsCount(); i++) {
    preview.addPoint(getPoint(index, i), COLOR_THEME_SECONDARY1);
  }
  invalidate();
}

bool CurveEdit::setCurrent(uint8_t n)
{
  int8_t max= getCurvePointsCount() - 1;
  if (n < 0)
    n = 0;
  else if (n > max)
    n = max;
  if (current != n) {
    current = n;
    return true;
  }
  return false;
}

void CurveEdit::selectPoint(int8_t chg)
{
  if (setCurrent((int8_t)current + chg))
    updatePreview();
}

void CurveEdit::setX(int8_t chg)
{
  if (isEditableX()) {
    uint8_t pMax = getCurvePointsCount() - 1;
    int8_t * point = &curveAddress(index)[pMax + current];
    int8_t xmin = (current == 1 ? -100 : point[-1]);
    int8_t xmax = (current == pMax - 1 ? 100 : point[1]);
    int8_t n = max<int8_t>(min<int8_t>(*point + chg, xmax-1), xmin+1);
    if (n != *point) {
      *point = n;
      storageDirty(EE_MODEL);
      updatePreview();
    }
  }
}

void CurveEdit::setY(int8_t chg)
{
  int8_t & point = curveAddress(index)[current];
  int8_t n = max<int8_t>(-100, min<int8_t>(100, point + chg));
  if (n != point) {
    point = n;
    storageDirty(EE_MODEL);
    updatePreview();
  }
}

void CurveEdit::setX(uint8_t n, int8_t value)
{
  if (isEditableX()) {
    uint8_t pMax = getCurvePointsCount() - 1;
    int8_t & point = curveAddress(index)[pMax + n];
    if (value != point) {
      point = value;
      storageDirty(EE_MODEL);
      updatePreview();
    }
  }
}

void CurveEdit::setY(uint8_t n, int8_t value)
{
  int8_t & point = curveAddress(index)[n];
  if (value != point) {
    point = value;
    storageDirty(EE_MODEL);
    updatePreview();
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

int8_t CurveEdit::getX(uint8_t n) const
{
  int pMax = getCurvePointsCount() - 1;
  if (isCustomCurve()) {
    if (n == 0) return -100;
    if (n == pMax) return 100;
    return curveAddress(index)[n + pMax];
  }
  return -100 + (200 * n) / pMax;
}

int8_t CurveEdit::getY(uint8_t n) const
{
  return curveAddress(index)[n];
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
  auto form = new FormWindow(window, rect_t{});
  form->padAll(0);
  form->setFlexLayout();
  
  FlexGridLayout grid(col_dsc, row_dsc, 0);

  auto line = form->newLine(&grid);
  line->padAll(0);

#if LCD_H > LCD_W // portrait
  lv_obj_set_flex_flow(line->getLvObj(), LV_FLEX_FLOW_COLUMN);
  coord_t curveWidth = window->width() - 94;
  coord_t boxWidth = window->width();
  coord_t boxHeight = window->height() - curveWidth;
#else
  lv_obj_set_flex_flow(line->getLvObj(), LV_FLEX_FLOW_ROW);
  coord_t curveWidth = window->height() - 2 * PAGE_PADDING;
  coord_t boxWidth = window->width() - curveWidth;
  coord_t boxHeight = window->height();
#endif

  // Curve editor preview
  lv_obj_set_flex_align(line->getLvObj(), LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_BETWEEN);
  curveEdit = new CurveEdit(line, { 0, 0, curveWidth, curveWidth }, index);

  auto box = new Window(line, rect_t{0, 0, boxWidth, boxHeight});
  box->padAll(0);

  curveDataEdit = new CurveDataEdit(box, rect_t{0, 0, boxWidth, boxHeight}, index);

  curveDataEdit->setCurveEdit(curveEdit);
}
