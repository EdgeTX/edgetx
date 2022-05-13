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


class CurveEditWindow : public Page
{
  public:
    explicit CurveEditWindow(uint8_t index):
      Page(ICON_MODEL_CURVES),
      index(index)
    {
      buildBody(&body);
      buildHeader(&header);
    }

  protected:
    uint8_t index;
    CurveEdit * curveEdit = nullptr;
    CurveDataEdit * curveDataEdit = nullptr;

    void buildHeader(Window * window)
    {
      new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP, LCD_W - PAGE_TITLE_LEFT,
                      PAGE_LINE_HEIGHT},
                     STR_MENUCURVE, 0, COLOR_THEME_PRIMARY2);
      char s[16];
      strAppendStringWithIndex(s, STR_CV, index + 1);
      new StaticText(window,
                     {PAGE_TITLE_LEFT, PAGE_TITLE_TOP + PAGE_LINE_HEIGHT,
                      LCD_W - PAGE_TITLE_LEFT, PAGE_LINE_HEIGHT},
                     s, 0, COLOR_THEME_PRIMARY2);
    }

#if LCD_W > LCD_H
    void buildBody(FormWindow * window)
    {
      coord_t curveWidth = window->height() - 2 * PAGE_PADDING;

      CurveHeader & curve = g_model.curves[index];
      int8_t * points = curveAddress(index);

      // Curve editor
      curveEdit = new CurveEdit(window, { coord_t(LCD_W - curveWidth - PAGE_PADDING), PAGE_PADDING, curveWidth, curveWidth}, index);

      FormGridLayout grid;
      grid.setLabelWidth(PAGE_PADDING);
      grid.setMarginRight(curveWidth + 2 * PAGE_PADDING);
      grid.spacer(PAGE_PADDING);

      // Name
      new StaticText(window, grid.getFieldSlot(), STR_NAME, 0, COLOR_THEME_PRIMARY1);
      grid.nextLine();
      new ModelTextEdit(window, grid.getFieldSlot(), curve.name, sizeof(curve.name));
      grid.nextLine();

      // Type
      new StaticText(window, grid.getFieldSlot(), STR_TYPE, 0, COLOR_THEME_PRIMARY1);
      grid.nextLine();
      new Choice(window, grid.getFieldSlot(2, 0), STR_CURVE_TYPES, 0, 1, GET_DEFAULT(g_model.curves[index].type),
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
                         curveDataEdit->clear();
                         curveDataEdit->update();
                       }
                     }
                 });

      // Points count
      auto edit = new NumberEdit(window, grid.getFieldSlot(2, 1), 2, 17, GET_DEFAULT(g_model.curves[index].points + 5),
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
                                         curveDataEdit->clear();
                                         curveDataEdit->update();
                                       }
                                     }
                                 });
      edit->setSuffix(STR_PTS);
      grid.nextLine();

      // Smooth
      new StaticText(window, grid.getFieldSlot(2, 0), STR_SMOOTH, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(window, grid.getFieldSlot(2, 1), GET_DEFAULT(g_model.curves[index].smooth),
                   [=](int32_t newValue) {
                       g_model.curves[index].smooth = newValue;
                       SET_DIRTY();
                       curveEdit->updatePreview();
                   });
      grid.nextLine();

      curveDataEdit = new CurveDataEdit(window, {0, grid.getWindowHeight(), coord_t(LCD_W - curveWidth - PAGE_PADDING - 1), window->height() -  grid.getWindowHeight() - PAGE_PADDING}, index, curveEdit);
    }
#else
    void buildBody(FormWindow * window)
    {
      CurveHeader & curve = g_model.curves[index];
      int8_t * points = curveAddress(index);

      FormGridLayout grid;
      grid.setMarginLeft(PAGE_PADDING);
      grid.setMarginRight(PAGE_PADDING);
      grid.setLabelWidth(100);
      grid.spacer(PAGE_PADDING);

      // Curve editor
      curveEdit = new CurveEdit(window, { 42, grid.getWindowHeight(), LCD_W - 80, LCD_W - 80}, index);
      grid.spacer(curveEdit->height() + 15);

      // Name
      new StaticText(window, grid.getLabelSlot(), STR_NAME, 0, COLOR_THEME_PRIMARY1);
      new ModelTextEdit(window, grid.getFieldSlot(), curve.name, sizeof(curve.name));
      grid.nextLine();

      // Type
      new StaticText(window, grid.getLabelSlot(), STR_TYPE, 0, COLOR_THEME_PRIMARY1);
      new Choice(window, grid.getFieldSlot(2, 0), STR_CURVE_TYPES, 0, 1, GET_DEFAULT(g_model.curves[index].type),
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
                     curveDataEdit->clear();
                     curveDataEdit->update();
                   }
                 });

      // Points count
      auto edit = new NumberEdit(window, grid.getFieldSlot(2, 1), 2, 17, GET_DEFAULT(g_model.curves[index].points + 5),
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
                                     curveDataEdit->clear();
                                     curveDataEdit->update();
                                   }
                                 });
      edit->setSuffix(STR_PTS);
      grid.nextLine();

      // Smooth
      new StaticText(window, grid.getLabelSlot(), STR_SMOOTH, 0, COLOR_THEME_PRIMARY1);
      new CheckBox(window, grid.getFieldSlot(), GET_DEFAULT(g_model.curves[index].smooth),
                   [=](int32_t newValue) {
                     g_model.curves[index].smooth = newValue;
                     SET_DIRTY();
                     curveEdit->updatePreview();
                   });
      grid.nextLine();

      curveDataEdit = new CurveDataEdit(window, {PAGE_PADDING, grid.getWindowHeight(), coord_t(LCD_W - PAGE_PADDING - 1), window->height() -  grid.getWindowHeight() - PAGE_PADDING}, index, curveEdit);
    }
#endif
};

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

  for (uint8_t index = 0; index < MAX_CURVES; index++) {

    CurveHeader &curve = g_model.curves[index];
    int8_t * points = curveAddress(index);

    std::function<void(void)> presetCurveFct = [=]() {
      Menu *menu = new Menu(window);
      for (int angle = -45; angle <= 45; angle += 15) {
        char label[16];
        strAppend(strAppendSigned(label, angle), "@");
        menu->addLine(label, [=]() {
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

      if (focusIndex == index) {
        button->setFocus(SET_FOCUS_DEFAULT);
        txt->setBackgroundColor(COLOR_THEME_FOCUS);
        txt->setTextFlags(COLOR_THEME_PRIMARY2 | CENTERED);
        txt->invalidate();
      }

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
