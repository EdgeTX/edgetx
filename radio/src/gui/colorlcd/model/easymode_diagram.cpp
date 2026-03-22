/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * Dynamic vehicle diagram for Easy Mode wizard.
 * Top-down view with colored control surfaces and channel labels
 * placed directly ON each surface for clear identification.
 */

#include "easymode_diagram.h"
#include "fonts.h"
#include "etx_lv_theme.h"

// Colors
#define COL_BG         lv_color_make(25, 28, 32)
#define COL_FUSELAGE   lv_color_make(85, 95, 110)
#define COL_FUSE_HI    lv_color_make(105, 115, 130)
#define COL_WING       lv_color_make(115, 130, 150)
#define COL_WING_HI    lv_color_make(135, 150, 170)

#define COL_AILERON    lv_color_make(30, 140, 255)
#define COL_ELEVATOR   lv_color_make(255, 170, 20)
#define COL_RUDDER     lv_color_make(230, 50, 50)
#define COL_THROTTLE   lv_color_make(40, 190, 70)
#define COL_FLAP       lv_color_make(170, 70, 230)
#define COL_STEERING   lv_color_make(230, 50, 50)
#define COL_MOTOR      lv_color_make(40, 190, 70)
#define COL_ROTOR      lv_color_make(150, 160, 170)
#define COL_WHITE      lv_color_white()

#define CX (DIAG_W / 2)
#define CY (DIAG_H / 2)

static char _chBuf[8];
const char* EasyModeDiagram::chLabel(int8_t ch)
{
  if (ch < 0) return "-";
  snprintf(_chBuf, sizeof(_chBuf), "%d", ch + 1);
  return _chBuf;
}

// ---- Construction ----

EasyModeDiagram::EasyModeDiagram(Window* parent, const EasyModeData& data) :
    Window(parent, {0, 0, DIAG_W, DIAG_H})
{
  lv_obj_clear_flag(lvobj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_style_bg_opa(lvobj, LV_OPA_COVER, LV_PART_MAIN);
  lv_obj_set_style_bg_color(lvobj, COL_BG, LV_PART_MAIN);
  lv_obj_set_style_radius(lvobj, 8, LV_PART_MAIN);
  lv_obj_set_style_clip_corner(lvobj, true, LV_PART_MAIN);

  canvas = lv_canvas_create(lvobj);
  uint32_t bufSize = LV_CANVAS_BUF_SIZE_TRUE_COLOR(DIAG_W, DIAG_H);
  canvasBuf = (uint8_t*)lv_mem_alloc(bufSize);
  lv_canvas_set_buffer(canvas, canvasBuf, DIAG_W, DIAG_H, LV_IMG_CF_TRUE_COLOR);
  lv_obj_set_pos(canvas, 0, 0);
  lv_obj_clear_flag(canvas, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);

  emData = &data;
  redraw();
}

EasyModeDiagram::~EasyModeDiagram()
{
  if (canvasBuf) { lv_mem_free(canvasBuf); canvasBuf = nullptr; }
}

void EasyModeDiagram::update(const EasyModeData& data)
{
  emData = &data;
  redraw();
}

void EasyModeDiagram::clearLabels()
{
  for (auto& l : labels) lv_obj_del(l.obj);
  labels.clear();
}

// Label centered inside a control surface rectangle (x,y,w,h)
void EasyModeDiagram::labelOn(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                               const char* text, lv_color_t color)
{
  lv_obj_t* lbl = lv_label_create(lvobj);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_color(lbl, COL_WHITE, LV_PART_MAIN);
  lv_obj_set_style_text_font(lbl, getFont(FONT(XXS)), LV_PART_MAIN);
  // Measure text width to center it
  lv_coord_t tw = lv_txt_get_width(text, strlen(text), getFont(FONT(XXS)), 0, LV_TEXT_FLAG_NONE);
  lv_coord_t th = lv_font_get_line_height(getFont(FONT(XXS)));
  lv_coord_t lx = x + (w - tw) / 2;
  lv_coord_t ly = y + (h - th) / 2;
  if (lx < 1) lx = 1;
  if (ly < 1) ly = 1;
  lv_obj_set_pos(lbl, lx, ly);
  lv_obj_clear_flag(lbl, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  labels.push_back({lbl});
}

// Label at an exact position
void EasyModeDiagram::labelAt(lv_coord_t x, lv_coord_t y, const char* text,
                               lv_color_t color, bool bold)
{
  lv_obj_t* lbl = lv_label_create(lvobj);
  lv_label_set_text(lbl, text);
  lv_obj_set_style_text_color(lbl, color, LV_PART_MAIN);
  lv_obj_set_style_text_font(lbl, getFont(bold ? FONT(XS) : FONT(XXS)), LV_PART_MAIN);
  lv_obj_set_pos(lbl, x, y);
  lv_obj_clear_flag(lbl, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
  labels.push_back({lbl});
}

void EasyModeDiagram::redraw()
{
  if (!emData || !canvasBuf) return;
  clearLabels();
  canvasClear();
  switch (emData->modelType) {
    case EASYMODE_AIRPLANE:   drawAirplane(); break;
    case EASYMODE_HELICOPTER: drawHelicopter(); break;
    case EASYMODE_GLIDER:     drawGlider(); break;
    case EASYMODE_MULTIROTOR: drawMultirotor(); break;
    case EASYMODE_CAR:        drawCar(); break;
    case EASYMODE_BOAT:       drawBoat(); break;
    default: break;
  }
  lv_obj_invalidate(canvas);
}

// ---- Canvas primitives ----

void EasyModeDiagram::canvasClear()
{ lv_canvas_fill_bg(canvas, COL_BG, LV_OPA_COVER); }

void EasyModeDiagram::drawRect(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                                lv_color_t fill, lv_coord_t radius)
{
  lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d);
  d.bg_color = fill; d.bg_opa = LV_OPA_COVER; d.radius = radius; d.border_width = 0;
  lv_canvas_draw_rect(canvas, x, y, w, h, &d);
}

void EasyModeDiagram::drawRectOutline(lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
                                       lv_color_t color, lv_coord_t bw, lv_coord_t radius)
{
  lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d);
  d.bg_opa = LV_OPA_TRANSP; d.border_color = color; d.border_width = bw;
  d.border_opa = LV_OPA_COVER; d.radius = radius;
  lv_canvas_draw_rect(canvas, x, y, w, h, &d);
}

void EasyModeDiagram::drawLine(lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2,
                                lv_color_t color, lv_coord_t width)
{
  lv_draw_line_dsc_t d; lv_draw_line_dsc_init(&d);
  d.color = color; d.width = width; d.opa = LV_OPA_COVER;
  lv_point_t pts[2] = {{x1,y1},{x2,y2}};
  lv_canvas_draw_line(canvas, pts, 2, &d);
}

void EasyModeDiagram::drawPolygon(const lv_point_t* pts, uint32_t cnt, lv_color_t fill)
{
  lv_draw_rect_dsc_t d; lv_draw_rect_dsc_init(&d);
  d.bg_color = fill; d.bg_opa = LV_OPA_COVER;
  lv_canvas_draw_polygon(canvas, pts, cnt, &d);
}

void EasyModeDiagram::drawCircle(lv_coord_t cx, lv_coord_t cy, lv_coord_t r, lv_color_t fill)
{ drawRect(cx-r, cy-r, r*2, r*2, fill, LV_RADIUS_CIRCLE); }

void EasyModeDiagram::drawCircleOutline(lv_coord_t cx, lv_coord_t cy, lv_coord_t r,
                                         lv_color_t color, lv_coord_t width)
{ drawRectOutline(cx-r, cy-r, r*2, r*2, color, width, LV_RADIUS_CIRCLE); }

// ============================================================
// AIRPLANE  (top-down, 180x170)
// ============================================================
void EasyModeDiagram::drawAirplane()
{
  auto& em = *emData;
  auto& ch = em.channels;
  bool isElevon = (em.wingType == EASYWING_ELEVON || em.wingType == EASYWING_DELTA);
  bool dualAil = (em.wingType == EASYWING_DUAL_AIL || em.wingType == EASYWING_FLAPERON ||
                  em.wingType == EASYWING_2AIL_1FLAP || em.wingType == EASYWING_2AIL_2FLAP ||
                  em.wingType == EASYWING_2AIL_4FLAP || isElevon);
  bool hasFlap = (em.wingType == EASYWING_1AIL_1FLAP || em.wingType == EASYWING_2AIL_1FLAP ||
                  em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP);
  bool dualFlap = (em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP);
  bool hasFlap34 = (em.wingType == EASYWING_2AIL_4FLAP);

  int cx = CX, cy = CY + 8;
  labelAt(cx - 24, 1, "Airplane", COL_WHITE, true);

  // Motor
  if (em.motorType != EASYMOTOR_NONE) {
    int mx = cx - 8, my = cy - 56, mw = 16, mh = 16;
    drawCircle(cx, cy - 48, 8, COL_MOTOR);
    drawCircleOutline(cx, cy - 48, 10, COL_MOTOR, 2);
    labelOn(mx, my, mw, mh, chLabel(ch.throttle), COL_THROTTLE);
  }

  // Fuselage
  drawRect(cx - 7, cy - 42, 14, 84, COL_FUSELAGE, 5);
  drawRect(cx - 2, cy - 38, 4, 76, COL_FUSE_HI, 2);
  lv_point_t nose[] = {{(lv_coord_t)(cx-7),(lv_coord_t)(cy-42)},
                       {(lv_coord_t)cx,(lv_coord_t)(cy-50)},
                       {(lv_coord_t)(cx+7),(lv_coord_t)(cy-42)}};
  drawPolygon(nose, 3, COL_FUSELAGE);

  if (isElevon) {
    // Delta/elevon wings
    lv_point_t lw[] = {{(lv_coord_t)(cx-7),(lv_coord_t)(cy-20)},
                       {(lv_coord_t)(cx-76),(lv_coord_t)(cy+12)},
                       {(lv_coord_t)(cx-64),(lv_coord_t)(cy+24)},
                       {(lv_coord_t)(cx-7),(lv_coord_t)(cy+12)}};
    drawPolygon(lw, 4, COL_WING);
    lv_point_t rw[] = {{(lv_coord_t)(cx+7),(lv_coord_t)(cy-20)},
                       {(lv_coord_t)(cx+76),(lv_coord_t)(cy+12)},
                       {(lv_coord_t)(cx+64),(lv_coord_t)(cy+24)},
                       {(lv_coord_t)(cx+7),(lv_coord_t)(cy+12)}};
    drawPolygon(rw, 4, COL_WING);
    // Left elevon surface
    int leX = cx-64, leY = cy+12, leW = 40, leH = 12;
    lv_point_t lcs[] = {{(lv_coord_t)(cx-24),(lv_coord_t)(cy+8)},
                        {(lv_coord_t)(cx-66),(lv_coord_t)(cy+18)},
                        {(lv_coord_t)(cx-58),(lv_coord_t)(cy+26)},
                        {(lv_coord_t)(cx-18),(lv_coord_t)(cy+14)}};
    drawPolygon(lcs, 4, COL_AILERON);
    labelOn(cx-64, cy+12, 40, 14, chLabel(ch.aileron), COL_AILERON);
    // Right elevon
    lv_point_t rcs[] = {{(lv_coord_t)(cx+24),(lv_coord_t)(cy+8)},
                        {(lv_coord_t)(cx+66),(lv_coord_t)(cy+18)},
                        {(lv_coord_t)(cx+58),(lv_coord_t)(cy+26)},
                        {(lv_coord_t)(cx+18),(lv_coord_t)(cy+14)}};
    drawPolygon(rcs, 4, COL_AILERON);
    labelOn(cx+24, cy+12, 40, 14, chLabel(ch.aileron2), COL_AILERON);
  } else {
    // Conventional wings (swept back)
    int ws = 76, wc = 22;
    int rootY = cy - 12;   // leading edge at root
    int tipY = rootY + 8;  // leading edge at tip (swept back)
    lv_point_t lw[] = {{(lv_coord_t)(cx-7),(lv_coord_t)(rootY)},
                       {(lv_coord_t)(cx-ws),(lv_coord_t)(tipY)},
                       {(lv_coord_t)(cx-ws+4),(lv_coord_t)(tipY+wc)},
                       {(lv_coord_t)(cx-7),(lv_coord_t)(rootY+wc)}};
    drawPolygon(lw, 4, COL_WING);
    lv_point_t rw[] = {{(lv_coord_t)(cx+7),(lv_coord_t)(rootY)},
                       {(lv_coord_t)(cx+ws),(lv_coord_t)(tipY)},
                       {(lv_coord_t)(cx+ws-4),(lv_coord_t)(tipY+wc)},
                       {(lv_coord_t)(cx+7),(lv_coord_t)(rootY+wc)}};
    drawPolygon(rw, 4, COL_WING);

    // Control surfaces on trailing edge
    int sH = 14;
    int tY = tipY + wc - sH;

    if (hasFlap34) {
      // 6-surface: ail | flap | flap3 per side
      int aW = 22, fW = 20, bW = 16;
      int x0 = cx - ws + 5;
      drawRect(x0, tY, aW, sH, COL_AILERON, 2);
      labelOn(x0, tY, aW, sH, chLabel(ch.aileron), COL_AILERON);
      drawRect(x0+aW+1, tY, fW, sH, COL_FLAP, 2);
      labelOn(x0+aW+1, tY, fW, sH, chLabel(ch.flap), COL_FLAP);
      drawRect(x0+aW+fW+2, tY, bW, sH, COL_FLAP, 2);
      labelOn(x0+aW+fW+2, tY, bW, sH, chLabel(ch.flap3), COL_FLAP);
      int x1 = cx + 8;
      drawRect(x1, tY, bW, sH, COL_FLAP, 2);
      labelOn(x1, tY, bW, sH, chLabel(ch.flap4), COL_FLAP);
      drawRect(x1+bW+1, tY, fW, sH, COL_FLAP, 2);
      labelOn(x1+bW+1, tY, fW, sH, chLabel(ch.flap2), COL_FLAP);
      drawRect(x1+bW+fW+2, tY, aW, sH, COL_AILERON, 2);
      labelOn(x1+bW+fW+2, tY, aW, sH, chLabel(ch.aileron2), COL_AILERON);
    } else if (hasFlap) {
      int aW = 26, fW = 24;
      int x0 = cx - ws + 5;
      drawRect(x0, tY, aW, sH, COL_AILERON, 2);
      labelOn(x0, tY, aW, sH, chLabel(ch.aileron), COL_AILERON);
      drawRect(x0+aW+1, tY, fW, sH, COL_FLAP, 2);
      labelOn(x0+aW+1, tY, fW, sH, chLabel(ch.flap), COL_FLAP);
      int x1 = cx + 8;
      drawRect(x1, tY, fW, sH, COL_FLAP, 2);
      if (dualFlap) labelOn(x1, tY, fW, sH, chLabel(ch.flap2), COL_FLAP);
      drawRect(x1+fW+1, tY, aW, sH, COL_AILERON, 2);
      if (dualAil) labelOn(x1+fW+1, tY, aW, sH, chLabel(ch.aileron2), COL_AILERON);
    } else {
      int aW = dualAil ? 30 : 38;
      int laX = cx - ws + 5;
      int raX = cx + ws - aW - 5;
      drawRect(laX, tY, aW, sH, COL_AILERON, 2);
      labelOn(laX, tY, aW, sH, chLabel(ch.aileron), COL_AILERON);
      drawRect(raX, tY, aW, sH, COL_AILERON, 2);
      if (dualAil) labelOn(raX, tY, aW, sH, chLabel(ch.aileron2), COL_AILERON);
    }

    // Tail
    if (em.tailType != EASYTAIL_TAILLESS) {
      int tb = cy + 28;
      if (em.tailType == EASYTAIL_V_TAIL) {
        int vW = 26, vH = 12;
        lv_point_t lt[] = {{(lv_coord_t)(cx-4),(lv_coord_t)(tb)},
                           {(lv_coord_t)(cx-26),(lv_coord_t)(tb-8)},
                           {(lv_coord_t)(cx-22),(lv_coord_t)(tb+6)},
                           {(lv_coord_t)(cx-4),(lv_coord_t)(tb+8)}};
        drawPolygon(lt, 4, COL_ELEVATOR);
        labelOn(cx-26, tb-4, vW, vH, chLabel(ch.elevator), COL_ELEVATOR);
        lv_point_t rt[] = {{(lv_coord_t)(cx+4),(lv_coord_t)(tb)},
                           {(lv_coord_t)(cx+26),(lv_coord_t)(tb-8)},
                           {(lv_coord_t)(cx+22),(lv_coord_t)(tb+6)},
                           {(lv_coord_t)(cx+4),(lv_coord_t)(tb+8)}};
        drawPolygon(rt, 4, COL_ELEVATOR);
        if (ch.rudder >= 0)
          labelOn(cx, tb-4, vW, vH, chLabel(ch.rudder), COL_RUDDER);
      } else {
        // Horizontal stab + elevator
        int hsW = 24, hsH = 10, eH = 12;
        drawRect(cx-hsW-4, tb, hsW, hsH, COL_WING, 2);
        drawRect(cx+4, tb, hsW, hsH, COL_WING, 2);
        // Elevator surfaces
        drawRect(cx-hsW-4, tb+hsH, hsW, eH, COL_ELEVATOR, 2);
        drawRect(cx+4, tb+hsH, hsW, eH, COL_ELEVATOR, 2);
        labelOn(cx-hsW-4, tb+hsH, hsW, eH, chLabel(ch.elevator), COL_ELEVATOR);
        if (em.tailType == EASYTAIL_DUAL_ELEVATOR || em.tailType == EASYTAIL_AILEVATOR)
          labelOn(cx+4, tb+hsH, hsW, eH, chLabel(ch.elevator2), COL_ELEVATOR);
        // Vertical fin + rudder
        int finW = 6, rudW = 8, finH = 16;
        drawRect(cx-3, tb-finH, finW, finH, COL_WING, 2);
        drawRect(cx+3, tb-finH, rudW, finH, COL_RUDDER, 2);
        labelOn(cx+3, tb-finH, rudW, finH, chLabel(ch.rudder), COL_RUDDER);
      }
    }
  }
}

// ============================================================
// HELICOPTER
// ============================================================
void EasyModeDiagram::drawHelicopter()
{
  auto& ch = emData->channels;
  int cx = CX, cy = CY + 8;
  labelAt(cx - 28, 1, "Helicopter", COL_WHITE, true);

  // Fuselage
  drawRect(cx-12, cy-14, 24, 30, COL_FUSELAGE, 10);
  // Tail boom
  drawRect(cx-4, cy+16, 8, 42, COL_FUSELAGE, 3);
  // Tail fin — rudder surface
  int tfW = 30, tfH = 12;
  int tfX = cx - tfW/2, tfY = cy + 52;
  lv_point_t tf[] = {{(lv_coord_t)(cx-16),(lv_coord_t)(cy+50)},
                     {(lv_coord_t)cx,(lv_coord_t)(cy+46)},
                     {(lv_coord_t)(cx+16),(lv_coord_t)(cy+50)},
                     {(lv_coord_t)cx,(lv_coord_t)(cy+58)}};
  drawPolygon(tf, 4, COL_RUDDER);
  labelOn(tfX, tfY, tfW, tfH, chLabel(ch.rudder), COL_RUDDER);

  // Skids
  drawRect(cx-18, cy-4, 4, 22, COL_WING_HI, 2);
  drawRect(cx+14, cy-4, 4, 22, COL_WING_HI, 2);

  // Main rotor disc
  drawCircleOutline(cx, cy-20, 38, COL_ROTOR, 2);
  drawLine(cx-36, cy-20, cx+36, cy-20, COL_ROTOR, 3);
  drawLine(cx, cy-56, cx, cy+16, COL_ROTOR, 3);
  drawCircle(cx, cy-20, 6, lv_color_make(200,200,200));

  // Swash labels — placed on fuselage/rotor areas
  // Roll (cyclic lateral) — left of rotor disc
  int rlW = 28, rlH = 22;
  drawRect(2, cy-32, rlW, rlH, COL_AILERON, 4);
  labelOn(2, cy-32, rlW, 11, chLabel(ch.aileron), COL_AILERON);
  labelOn(2, cy-21, rlW, 11, "Roll", COL_AILERON);

  // Nick (cyclic longitudinal) — above rotor
  int ptW = 28, ptH = 22;
  drawRect(cx-ptW/2, 16, ptW, ptH, COL_ELEVATOR, 4);
  labelOn(cx-ptW/2, 16, ptW, 11, chLabel(ch.elevator), COL_ELEVATOR);
  labelOn(cx-ptW/2, 27, ptW, 11, "Nick", COL_ELEVATOR);

  // Pitch (collective) — on fuselage
  int thW = 28, thH = 22;
  drawRect(cx-thW/2, cy-2, thW, thH, COL_THROTTLE, 4);
  labelOn(cx-thW/2, cy-2, thW, 11, chLabel(ch.throttle), COL_THROTTLE);
  labelOn(cx-thW/2, cy+9, thW, 11, "Pitch", COL_THROTTLE);
}

// ============================================================
// GLIDER
// ============================================================
void EasyModeDiagram::drawGlider()
{
  auto& em = *emData;
  auto& ch = em.channels;
  bool dualAil = (em.wingType == EASYWING_DUAL_AIL || em.wingType == EASYWING_FLAPERON ||
                  em.wingType == EASYWING_2AIL_1FLAP || em.wingType == EASYWING_2AIL_2FLAP ||
                  em.wingType == EASYWING_2AIL_4FLAP);
  bool hasFlap = (em.wingType == EASYWING_1AIL_1FLAP || em.wingType == EASYWING_2AIL_1FLAP ||
                  em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP);
  bool dualFlap = (em.wingType == EASYWING_2AIL_2FLAP || em.wingType == EASYWING_2AIL_4FLAP);
  bool hasFlap34 = (em.wingType == EASYWING_2AIL_4FLAP);

  int cx = CX, cy = CY + 4;
  labelAt(cx - 16, 1, "Glider", COL_WHITE, true);

  // Slim fuselage
  drawRect(cx-4, cy-36, 8, 72, COL_FUSELAGE, 4);
  lv_point_t nose[] = {{(lv_coord_t)(cx-4),(lv_coord_t)(cy-36)},
                       {(lv_coord_t)cx,(lv_coord_t)(cy-44)},
                       {(lv_coord_t)(cx+4),(lv_coord_t)(cy-36)}};
  drawPolygon(nose, 3, COL_FUSELAGE);

  // Long wings (straight — typical glider)
  int ws = 86, wc = 18;
  int gRootY = cy - 8;
  int gTipY = gRootY + 4;  // slight sweep back
  lv_point_t lw[] = {{(lv_coord_t)(cx-4),(lv_coord_t)(gRootY)},
                     {(lv_coord_t)(cx-ws),(lv_coord_t)(gTipY)},
                     {(lv_coord_t)(cx-ws+3),(lv_coord_t)(gTipY+wc)},
                     {(lv_coord_t)(cx-4),(lv_coord_t)(gRootY+wc)}};
  drawPolygon(lw, 4, COL_WING);
  lv_point_t rw[] = {{(lv_coord_t)(cx+4),(lv_coord_t)(gRootY)},
                     {(lv_coord_t)(cx+ws),(lv_coord_t)(gTipY)},
                     {(lv_coord_t)(cx+ws-3),(lv_coord_t)(gTipY+wc)},
                     {(lv_coord_t)(cx+4),(lv_coord_t)(gRootY+wc)}};
  drawPolygon(rw, 4, COL_WING);

  // Control surfaces
  int sH = 10, sY = gTipY + wc - sH;

  if (hasFlap34) {
    int aW = 18, fW = 18, bW = 14;
    int x0 = cx - ws + 4;
    drawRect(x0, sY, aW, sH, COL_AILERON, 1);
    labelOn(x0, sY, aW, sH, chLabel(ch.aileron), COL_AILERON);
    drawRect(x0+aW+1, sY, fW, sH, COL_FLAP, 1);
    labelOn(x0+aW+1, sY, fW, sH, chLabel(ch.flap), COL_FLAP);
    drawRect(x0+aW+fW+2, sY, bW, sH, COL_FLAP, 1);
    labelOn(x0+aW+fW+2, sY, bW, sH, chLabel(ch.flap3), COL_FLAP);
    int x1 = cx + 6;
    drawRect(x1, sY, bW, sH, COL_FLAP, 1);
    labelOn(x1, sY, bW, sH, chLabel(ch.flap4), COL_FLAP);
    drawRect(x1+bW+1, sY, fW, sH, COL_FLAP, 1);
    if (dualFlap) labelOn(x1+bW+1, sY, fW, sH, chLabel(ch.flap2), COL_FLAP);
    drawRect(x1+bW+fW+2, sY, aW, sH, COL_AILERON, 1);
    if (dualAil) labelOn(x1+bW+fW+2, sY, aW, sH, chLabel(ch.aileron2), COL_AILERON);
  } else if (hasFlap) {
    int aW = 28, fW = 26;
    int x0 = cx - ws + 4;
    drawRect(x0, sY, aW, sH, COL_AILERON, 1);
    labelOn(x0, sY, aW, sH, chLabel(ch.aileron), COL_AILERON);
    drawRect(x0+aW+1, sY, fW, sH, COL_FLAP, 1);
    labelOn(x0+aW+1, sY, fW, sH, chLabel(ch.flap), COL_FLAP);
    int x1 = cx + 6;
    drawRect(x1, sY, fW, sH, COL_FLAP, 1);
    if (dualFlap) labelOn(x1, sY, fW, sH, chLabel(ch.flap2), COL_FLAP);
    drawRect(x1+fW+1, sY, aW, sH, COL_AILERON, 1);
    if (dualAil) labelOn(x1+fW+1, sY, aW, sH, chLabel(ch.aileron2), COL_AILERON);
  } else {
    int aW = 32;
    drawRect(cx-ws+4, sY, aW, sH, COL_AILERON, 1);
    labelOn(cx-ws+4, sY, aW, sH, chLabel(ch.aileron), COL_AILERON);
    drawRect(cx+ws-aW-4, sY, aW, sH, COL_AILERON, 1);
    if (dualAil) labelOn(cx+ws-aW-4, sY, aW, sH, chLabel(ch.aileron2), COL_AILERON);
  }

  // T-tail
  int tb = cy + 28;
  drawRect(cx-2, tb-10, 4, 12, COL_WING, 1);
  int eW = 12, eH = 6;
  drawRect(cx-eW-2, tb, eW, 6, COL_WING, 2);
  drawRect(cx+2, tb, eW, 6, COL_WING, 2);
  drawRect(cx-eW-2, tb+6, eW, eH, COL_ELEVATOR, 1);
  drawRect(cx+2, tb+6, eW, eH, COL_ELEVATOR, 1);
  labelOn(cx-eW-2, tb+6, eW*2+4, eH, chLabel(ch.elevator), COL_ELEVATOR);
  // Rudder
  drawRect(cx+2, tb-10, 3, 12, COL_RUDDER, 1);
  labelAt(cx+8, tb-12, chLabel(ch.rudder), COL_RUDDER);

  // Motor
  if (em.motorType != EASYMOTOR_NONE) {
    drawCircle(cx, cy-46, 4, COL_MOTOR);
    labelAt(cx+8, cy-52, chLabel(ch.throttle), COL_THROTTLE);
  }
}

// ============================================================
// MULTIROTOR
// ============================================================
void EasyModeDiagram::drawMultirotor()
{
  auto& ch = emData->channels;
  int cx = CX, cy = CY + 2;
  labelAt(cx - 28, 1, "Multirotor", COL_WHITE, true);

  int arm = 40, mr = 15;
  drawLine(cx-arm, cy-arm, cx+arm, cy+arm, COL_FUSELAGE, 5);
  drawLine(cx+arm, cy-arm, cx-arm, cy+arm, COL_FUSELAGE, 5);
  drawRect(cx-12, cy-12, 24, 24, COL_FUSELAGE, 6);
  drawRect(cx-6, cy-6, 12, 12, COL_FUSE_HI, 4);
  // Direction arrow
  lv_point_t arr[] = {{(lv_coord_t)cx,(lv_coord_t)(cy-18)},
                      {(lv_coord_t)(cx-6),(lv_coord_t)(cy-10)},
                      {(lv_coord_t)(cx+6),(lv_coord_t)(cy-10)}};
  drawPolygon(arr, 3, COL_WHITE);

  // Motors
  struct MP { int dx, dy; const char* l; };
  MP ms[] = {{-arm,-arm,"M1"},{arm,-arm,"M2"},{-arm,arm,"M3"},{arm,arm,"M4"}};
  for (auto& m : ms) {
    int mx = cx+m.dx, my = cy+m.dy;
    drawCircleOutline(mx, my, mr, COL_ROTOR, 2);
    drawCircle(mx, my, 5, COL_MOTOR);
    labelOn(mx-mr, my-mr, mr*2, mr*2, m.l, COL_WHITE);
  }

  // Control labels — colored boxes along edges
  int bW = 30, bH = 20;
  // Roll
  drawRect(1, cy-bH/2, bW, bH, COL_AILERON, 4);
  labelOn(1, cy-bH/2, bW, 10, chLabel(ch.aileron), COL_AILERON);
  labelOn(1, cy-bH/2+10, bW, 10, "Roll", COL_AILERON);
  // Nick
  drawRect(cx-bW/2, 16, bW, bH, COL_ELEVATOR, 4);
  labelOn(cx-bW/2, 16, bW, 10, chLabel(ch.elevator), COL_ELEVATOR);
  labelOn(cx-bW/2, 26, bW, 10, "Nick", COL_ELEVATOR);
  // Yaw
  drawRect(DIAG_W-bW-1, cy-bH/2, bW, bH, COL_RUDDER, 4);
  labelOn(DIAG_W-bW-1, cy-bH/2, bW, 10, chLabel(ch.rudder), COL_RUDDER);
  labelOn(DIAG_W-bW-1, cy-bH/2+10, bW, 10, "Yaw", COL_RUDDER);
  // Throttle — center
  int tw = 28;
  drawRect(cx-tw/2, cy+2, tw, 10, COL_THROTTLE, 3);
  labelOn(cx-tw/2, cy+2, tw, 10, chLabel(ch.throttle), COL_THROTTLE);
}

// ============================================================
// CAR
// ============================================================
void EasyModeDiagram::drawCar()
{
  auto& ch = emData->channels;
  int cx = CX, cy = CY + 2;
  labelAt(cx - 10, 2, "Car", COL_WHITE, true);

  int bw = 48, bh = 74;
  drawRect(cx-bw/2, cy-bh/2, bw, bh, COL_FUSELAGE, 8);
  drawRect(cx-bw/2+3, cy-bh/2+3, bw-6, bh-6, COL_FUSE_HI, 6);
  drawRect(cx-bw/2+6, cy-bh/2+14, bw-12, 14, lv_color_make(50,60,75), 3);
  drawRect(cx-bw/2+6, cy+bh/2-24, bw-12, 10, lv_color_make(50,60,75), 3);

  // Front wheels — steering surfaces
  int wW = 10, wH = 18;
  int flX = cx-bw/2-wW-2, frX = cx+bw/2+2, fwY = cy-bh/2+10;
  drawRect(flX, fwY, wW, wH, COL_STEERING, 4);
  drawRect(frX, fwY, wW, wH, COL_STEERING, 4);
  labelOn(flX, fwY, wW*2+bw+4, wH, chLabel(ch.steering), COL_STEERING);

  // Rear wheels — drive
  int rwY = cy+bh/2-28;
  drawRect(flX, rwY, wW, wH, COL_THROTTLE, 4);
  drawRect(frX, rwY, wW, wH, COL_THROTTLE, 4);
  labelOn(flX, rwY, wW*2+bw+4, wH, chLabel(ch.throttle), COL_THROTTLE);

  // Direction arrow
  lv_point_t arr[] = {{(lv_coord_t)cx,(lv_coord_t)(cy-bh/2-10)},
                      {(lv_coord_t)(cx-7),(lv_coord_t)(cy-bh/2-2)},
                      {(lv_coord_t)(cx+7),(lv_coord_t)(cy-bh/2-2)}};
  drawPolygon(arr, 3, COL_WHITE);

  // Steer/Drive legends
  labelAt(2, cy-bh/2+28, "Steer", COL_STEERING);
  labelAt(2, cy+bh/2-10, "Drive", COL_THROTTLE);
}

// ============================================================
// BOAT
// ============================================================
void EasyModeDiagram::drawBoat()
{
  auto& ch = emData->channels;
  int cx = CX, cy = CY + 2;
  labelAt(cx - 12, 2, "Boat", COL_WHITE, true);

  lv_point_t hull[] = {
    {(lv_coord_t)cx,(lv_coord_t)(cy-50)},
    {(lv_coord_t)(cx-30),(lv_coord_t)(cy-12)},
    {(lv_coord_t)(cx-26),(lv_coord_t)(cy+30)},
    {(lv_coord_t)(cx-16),(lv_coord_t)(cy+42)},
    {(lv_coord_t)(cx+16),(lv_coord_t)(cy+42)},
    {(lv_coord_t)(cx+26),(lv_coord_t)(cy+30)},
    {(lv_coord_t)(cx+30),(lv_coord_t)(cy-12)}};
  drawPolygon(hull, 7, COL_FUSELAGE);
  lv_point_t inner[] = {
    {(lv_coord_t)cx,(lv_coord_t)(cy-42)},
    {(lv_coord_t)(cx-22),(lv_coord_t)(cy-8)},
    {(lv_coord_t)(cx-18),(lv_coord_t)(cy+24)},
    {(lv_coord_t)(cx-10),(lv_coord_t)(cy+34)},
    {(lv_coord_t)(cx+10),(lv_coord_t)(cy+34)},
    {(lv_coord_t)(cx+18),(lv_coord_t)(cy+24)},
    {(lv_coord_t)(cx+22),(lv_coord_t)(cy-8)}};
  drawPolygon(inner, 7, COL_FUSE_HI);
  drawCircle(cx, cy-2, 16, lv_color_make(70,80,90));
  drawRect(cx-10, cy-8, 20, 14, lv_color_make(130,140,155), 4);
  drawLine(cx, cy-46, cx, cy-16, COL_WING_HI, 2);

  // Rudder — labeled surface
  int rudW = 18, rudH = 10;
  drawRect(cx-rudW/2, cy+36, rudW, rudH, COL_STEERING, 3);
  labelOn(cx-rudW/2, cy+36, rudW, rudH, chLabel(ch.steering), COL_STEERING);

  // Motor — labeled surface
  drawCircle(cx, cy+32, 5, COL_MOTOR);
  drawCircleOutline(cx, cy+32, 7, COL_MOTOR, 2);
  labelAt(cx+14, cy+28, chLabel(ch.throttle), COL_THROTTLE);

  // Direction arrow
  lv_point_t arr[] = {{(lv_coord_t)cx,(lv_coord_t)(cy-58)},
                      {(lv_coord_t)(cx-6),(lv_coord_t)(cy-50)},
                      {(lv_coord_t)(cx+6),(lv_coord_t)(cy-50)}};
  drawPolygon(arr, 3, COL_WHITE);

  labelAt(cx-rudW/2-2, cy+48, "Rudder", COL_STEERING);
  labelAt(cx+14, cy+40, "Motor", COL_THROTTLE);
}
