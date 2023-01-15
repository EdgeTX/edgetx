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

class CurveButton : public Button {
  public:
    CurveButton(FormGroup * parent, const rect_t &rect, uint8_t index) :
      Button(parent, rect),
      index(index)
    {
      if (isCurveUsed(index)) {
        setHeight(130);
        new Curve(this, {5, 5, 120, 120},
                  [=](int x) -> int {
                    return applyCustomCurve(x, index);
                  });
      }
    }

    void paint(BitmapBuffer * dc) override
    {
      dc->drawSolidFilledRect(0, 0, rect.w, rect.h, COLOR_THEME_PRIMARY2);

      // bounding rect
      if (hasFocus()) {
        dc->drawSolidRect(0, 0, rect.w, rect.h, 2, COLOR_THEME_FOCUS);
      } else {
        dc->drawSolidRect(0, 0, rect.w, rect.h, 1, COLOR_THEME_SECONDARY2);
      }

      // curve characteristics
      if (isCurveUsed(index)) {
        CurveHeader &curve = g_model.curves[index];
        dc->drawNumber(130, 5, 5 + curve.points, LEFT | COLOR_THEME_SECONDARY1, 0, nullptr, STR_PTS);
        dc->drawTextAtIndex(130, 25, STR_CURVE_TYPES, curve.type, COLOR_THEME_SECONDARY1);
        if (curve.smooth)
          dc->drawText(130, 45, STR_SMOOTH, COLOR_THEME_SECONDARY1);
      }
    }

  protected:
    uint8_t index;
};

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

void ModelCurvesPage::rebuild(FormWindow * window, int8_t focusIndex)
{
  auto scroll_y = lv_obj_get_scroll_y(window->getLvObj());  
  window->clear();
  build(window, focusIndex);
  lv_obj_scroll_to_y(window->getLvObj(), scroll_y, LV_ANIM_OFF);
}

void ModelCurvesPage::editCurve(FormWindow * window, uint8_t curve)
{
  Window * editWindow = new CurveEditWindow(curve);
  editWindow->setCloseHandler([=]() {
    rebuild(window, curve);
  });
}

void ModelCurvesPage::build(FormWindow * window, int8_t focusIndex)
{
  FormGridLayout grid;
  grid.spacer(PAGE_PADDING);
  grid.setLabelWidth(66);
  window->padAll(0);

  CurveEdit::SetCurrentSource(0);

  for (uint8_t index = 0; index < MAX_CURVES; index++) {

    CurveHeader &curve = g_model.curves[index];
    int8_t * points = curveAddress(index);

    std::function<void(void)> presetCurveFct = [=]() {
      Menu *menu = new Menu(window);
      for (int angle = -45; angle <= 45; angle += 15) {
        char label[16];
        strAppend(strAppendSigned(label, angle), "°");
        menu->addLineBuffered(label, [=]() {
          int dx = 2000 / (5 + curve.points - 1);
          for (uint8_t i = 0; i < 5 + curve.points; i++) {
            int x = -1000 + i * dx;
            points[i] = divRoundClosest(angle * x, 450);
          }
          if (curve.type == CURVE_TYPE_CUSTOM) {
            resetCustomCurveX(points, 5 + curve.points);
          }
          storageDirty(EE_MODEL);
          rebuild(window, index);
        });
      }
      menu->updateLines();
    };

    if (isCurveUsed(index)) {
      // Curve label
      auto txt =
          new StaticText(window, grid.getLabelSlot(), getCurveString(1 + index),
                         BUTTON_BACKGROUND, COLOR_THEME_PRIMARY1 | CENTERED);

      // Curve drawing
      Button * button = new CurveButton(window, grid.getFieldSlot(), index);
      button->setPressHandler([=]() -> uint8_t {
          Menu * menu = new Menu(window);
          menu->addLine(STR_EDIT, [=]() {
              editCurve(window, index);
          });
          menu->addLine(STR_CURVE_PRESET, presetCurveFct);
          menu->addLine(STR_MIRROR, [=]() {
              curveMirror(index);
              storageDirty(EE_MODEL);
              button->invalidate();
          });
          menu->addLine(STR_CLEAR, [=]() {
              curveClear(index);
              storageDirty(EE_MODEL);
              rebuild(window, index);
          });
          return 0;
      });
      button->setFocusHandler([=](bool focus) {
        if (focus) {
          txt->setBackgroundColor(COLOR_THEME_FOCUS);
          txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED);
        } else {
          txt->setBackgroundColor(COLOR_THEME_SECONDARY2);
          txt->setTextFlags(COLOR_THEME_PRIMARY1 | CENTERED);
        }
        txt->invalidate();
      });

      // if (focusIndex == index) {
      //   button->setFocus(SET_FOCUS_DEFAULT);
      //   txt->setBackgroundColor(COLOR_THEME_FOCUS);
      //   txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED);
      //   txt->invalidate();
      // }

      txt->setHeight(button->height());
      grid.spacer(button->height() + 5);
    } else {
      auto button = new TextButton(window, grid.getLabelSlot(),
                                   getCurveString(1 + index));
      button->setPressHandler([=]() {
        Menu *menu = new Menu(window);
        menu->addLine(STR_EDIT, [=]() {
            initPoints(curve, points);
            editCurve(window, index);
        });
        menu->addLine(STR_CURVE_PRESET, presetCurveFct);
        return 0;
      });
      grid.spacer(button->height() + 5);
    }
  }

// extra bottom padding if touchscreen
#if defined HARDWARE_TOUCH
  grid.nextLine();
#endif

}
