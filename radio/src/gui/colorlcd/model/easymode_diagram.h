/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Dynamic vehicle diagram widget for Easy Mode wizard.
 * Draws top-down control surface layout using LVGL canvas.
 * Channel labels are LVGL label widgets placed directly on surfaces.
 */

#pragma once

#include "window.h"
#include "easymode.h"

#include <vector>

#define DIAG_W 180
#define DIAG_H 170

class EasyModeDiagram : public Window
{
 public:
  EasyModeDiagram(Window* parent, const EasyModeData& data);
  ~EasyModeDiagram();

  void update(const EasyModeData& data);

#if defined(DEBUG_WINDOWS)
  std::string getName() const override { return "EasyModeDiagram"; }
#endif

 protected:
  lv_obj_t* canvas = nullptr;
  uint8_t* canvasBuf = nullptr;
  const EasyModeData* emData = nullptr;

  struct LabelInfo { lv_obj_t* obj; };
  std::vector<LabelInfo> labels;

  void redraw();
  void clearLabels();

  // Place label centered on a control surface rectangle
  void labelOn(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
               const char* text, lv_color_t color);
  // Place label adjacent to a surface (below or to the side)
  void labelAt(lv_coord_t x, lv_coord_t y, const char* text,
               lv_color_t color, bool bold = false);

  // Canvas primitives
  void canvasClear();
  void drawRect(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                lv_color_t fill, lv_coord_t radius = 0);
  void drawRectOutline(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                       lv_color_t color, lv_coord_t borderW = 1, lv_coord_t radius = 0);
  void drawLine(lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2,
                lv_color_t color, lv_coord_t width = 1);
  void drawPolygon(const lv_point_t* pts, uint32_t cnt, lv_color_t fill);
  void drawCircle(lv_coord_t cx, lv_coord_t cy, lv_coord_t r, lv_color_t fill);
  void drawCircleOutline(lv_coord_t cx, lv_coord_t cy, lv_coord_t r,
                         lv_color_t color, lv_coord_t width = 1);

  void drawAirplane();
  void drawHelicopter();
  void drawGlider();
  void drawMultirotor();
  void drawCar();
  void drawBoat();

  static const char* chLabel(int8_t ch);
};
