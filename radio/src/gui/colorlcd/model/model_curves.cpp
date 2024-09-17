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

#include "curveedit.h"
#include "libopenui.h"
#include "edgetx.h"

#define SET_DIRTY() storageDirty(EE_MODEL)

class CurveButton : public Button
{
 public:
  CurveButton(Window *parent, const rect_t &rect, uint8_t index) :
      Button(parent, rect), index(index)
  {
    padAll(PAD_ZERO);

    // Title
    char buf[32];
    char *s = strAppendStringWithIndex(buf, STR_CV, index + 1);
    if (g_model.curves[index].name[0]) {
      s = strAppend(s, ":");
      strAppend(s, g_model.curves[index].name, LEN_CURVE_NAME);
    }
    title = new StaticText(this, {4, -1, width() - 12, TITLE_H + 1}, buf, 
                           COLOR_THEME_SECONDARY1_INDEX, CENTERED | FONT(BOLD));
    etx_txt_color(title->getLvObj(), COLOR_THEME_PRIMARY2_INDEX,
                  LV_PART_MAIN | LV_STATE_USER_1);
    etx_solid_bg(title->getLvObj(), COLOR_THEME_SECONDARY2_INDEX);
    etx_solid_bg(title->getLvObj(), COLOR_THEME_FOCUS_INDEX,
                 LV_PART_MAIN | LV_STATE_USER_1);

    hdrLeft = new StaticIcon(this, 0, 0, ICON_ROUND_TITLE_LEFT,
                             COLOR_THEME_SECONDARY2_INDEX);
    hdrRight = new StaticIcon(this, width() - 8, 0,
                              ICON_ROUND_TITLE_RIGHT,
                              COLOR_THEME_SECONDARY2_INDEX);

    // Preview
    preview = new CurveRenderer(
        this,
        {PAD_MEDIUM, PAD_MEDIUM + TITLE_H, width() - PAD_MEDIUM * 2 - 4,
         width() - PAD_MEDIUM * 2 - 4},
        [=](int x) -> int { return applyCustomCurve(x, index); });

    // Curve characteristics
    CurveHeader &curve = g_model.curves[index];
    snprintf(buf, 32, "%s %d %s", STR_CURVE_TYPES[curve.type], 5 + curve.points,
             STR_PTS);
    new StaticText(this, {0, height() - INFO_H + 1, LV_PCT(100), 16}, buf, 
                   COLOR_THEME_SECONDARY1_INDEX, CENTERED | FONT(BOLD));
  }

  void update() { preview->update(); }

  static LAYOUT_VAL(TITLE_H, 20, 20)
  static LAYOUT_VAL(INFO_H, 27, 27)
  static LAYOUT_VAL(CURVE_BTN_W, 142, 142)
  static constexpr coord_t CURVE_BTH_H = CURVE_BTN_W + TITLE_H + INFO_H - PAD_MEDIUM;

 protected:
  uint8_t index;
  StaticText *title;
  CurveRenderer *preview;
  StaticIcon *hdrLeft = nullptr;
  StaticIcon *hdrRight = nullptr;

  void checkEvents() override
  {
    if (hasFocus()) {
      lv_obj_add_state(title->getLvObj(), LV_STATE_USER_1);
      hdrLeft->setColor(COLOR_THEME_FOCUS_INDEX);
      hdrRight->setColor(COLOR_THEME_FOCUS_INDEX);
    } else {
      lv_obj_clear_state(title->getLvObj(), LV_STATE_USER_1);
      hdrLeft->setColor(COLOR_THEME_SECONDARY2_INDEX);
      hdrRight->setColor(COLOR_THEME_SECONDARY2_INDEX);
    }
  }
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

ModelCurvesPage::ModelCurvesPage() : PageTab(STR_MENUCURVES, ICON_MODEL_CURVES)
{
}

// can be called from any other screen to edit a curve.
// currently called from model_mixes.cpp on longpress.
void ModelCurvesPage::pushEditCurve(int index, std::function<void(void)> refreshView, mixsrc_t source)
{
  if (!isCurveUsed(index)) {
    CurveHeader &curve = g_model.curves[index];
    int8_t *points = curveAddress(index);
    initPoints(curve, points);
  }

  auto cv = new CurveEditWindow(index, refreshView);
  cv->setCurrentSource(source);
}

void ModelCurvesPage::rebuild(Window *window)
{
  window->clear();
  build(window);
}

void ModelCurvesPage::editCurve(Window *window, uint8_t curve)
{
  Window *editWindow = new CurveEditWindow(curve);
  editWindow->setCloseHandler([=]() { rebuild(window); });
}

void ModelCurvesPage::presetMenu(Window *window, uint8_t index)
{
  Menu *menu = new Menu();
  menu->setTitle(STR_CURVE_PRESET);
  for (int angle = -45; angle <= 45; angle += 15) {
    char label[16];
    strAppend(strAppendSigned(label, angle), "°");
    menu->addLineBuffered(label, [=]() {
      CurveHeader &curve = g_model.curves[index];
      int8_t *points = curveAddress(index);

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

void ModelCurvesPage::newCV(Window *window, bool presetCV)
{
  Menu *menu = new Menu();
  menu->setTitle(STR_CURVE);
  char s[6] = "CVxx";

  // search for unused slot
  for (uint8_t i = 0; i < MAX_CURVES; i += 1) {
    if (!isCurveUsed(i)) {
      strAppendUnsigned(&s[2], i + 1);
      menu->addLineBuffered(s, [=]() {
        focusIndex = i;
        if (presetCV) {
          presetMenu(window, i);
        } else {
          CurveHeader &curve = g_model.curves[i];
          int8_t *points = curveAddress(i);
          initPoints(curve, points);
          editCurve(window, i);
        }
      });
    }
  }
  menu->updateLines();
}

void ModelCurvesPage::plusPopup(Window *window)
{
  Menu *menu = new Menu();
  menu->setTitle(STR_NEW);
  menu->addLine(STR_EDIT, [=]() { newCV(window, false); });
  menu->addLine(STR_CURVE_PRESET, [=]() { newCV(window, true); });
}

void ModelCurvesPage::build(Window *window)
{
#if !PORTRAIT_LCD
  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
#else
  static const lv_coord_t col_dsc[] = {LV_GRID_FR(1), LV_GRID_FR(1),
                                       LV_GRID_TEMPLATE_LAST};
#endif
  static const lv_coord_t row_dsc[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

  window->setFlexLayout(LV_FLEX_FLOW_COLUMN, PAD_ZERO);

  FlexGridLayout grid(col_dsc, row_dsc, PAD_TINY);

  FormLine *line = nullptr;

  bool hasFocusButton = false;

  uint8_t curveIndex = 0;
  CurveButton *firstCurveButton = nullptr;

  for (uint8_t index = 0; index < MAX_CURVES; index++) {
    if (isCurveUsed(index)) {
      if ((curveIndex % PER_ROW) == 0) {
        line = window->newLine(grid);
        lv_obj_set_grid_align(line->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN,
                              LV_GRID_ALIGN_SPACE_BETWEEN);
      }

      // Curve drawing
      auto button =
          new CurveButton(line, rect_t{0, 0, CurveButton::CURVE_BTN_W, CurveButton::CURVE_BTH_H}, index);
      button->setPressHandler([=]() -> uint8_t {
        Menu *menu = new Menu();
        menu->setTitle(STR_CURVE);
        menu->addLine(STR_EDIT, [=]() { editCurve(window, index); });
        menu->addLine(STR_CURVE_PRESET, [=]() { presetMenu(window, index); });
        menu->addLine(STR_MIRROR, [=]() {
          curveMirror(index);
          storageDirty(EE_MODEL);
          button->update();
        });
        menu->addLine(STR_CLEAR, [=]() {
          curveClear(index);
          storageDirty(EE_MODEL);
          rebuild(window);
        });
        return 0;
      });
      button->setFocusHandler([=](bool hasFocus) {
        if (hasFocus) focusIndex = index;
      });

      button->setLongPressHandler([=]() -> uint8_t {
        if (addButton) {
          lv_group_focus_obj(addButton->getLvObj());
          plusPopup(window);
        }
        return 0;
      });

      if (!firstCurveButton) {
        firstCurveButton = button;
      }

      if (index == focusIndex) {
        hasFocusButton = true;
        lv_group_focus_obj(button->getLvObj());
      }

      lv_obj_set_grid_cell(button->getLvObj(), LV_GRID_ALIGN_CENTER,
                           curveIndex % PER_ROW, 1, LV_GRID_ALIGN_CENTER, 0, 1);

      curveIndex += 1;
    }
  }

  if (!hasFocusButton && firstCurveButton) {
    lv_group_focus_obj(firstCurveButton->getLvObj());
  }

  if (curveIndex < MAX_CURVES) {
    if ((curveIndex % PER_ROW) == 0) {
      line = window->newLine(grid);
      lv_obj_set_grid_align(line->getLvObj(), LV_GRID_ALIGN_SPACE_BETWEEN,
                            LV_GRID_ALIGN_SPACE_BETWEEN);
    }

    addButton = new TextButton(line, rect_t{0, 0, CurveButton::CURVE_BTN_W, CurveButton::CURVE_BTH_H},
                               LV_SYMBOL_PLUS, [=]() {
                                 plusPopup(window);
                                 return 0;
                               });

    lv_obj_set_grid_cell(addButton->getLvObj(), LV_GRID_ALIGN_CENTER,
                         curveIndex % PER_ROW, 1, LV_GRID_ALIGN_CENTER, 0, 1);
  }
}
