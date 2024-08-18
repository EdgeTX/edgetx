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

#include "edgetx.h"

#if defined(LIBOPENUI)
  #include "libopenui.h"
#endif

extern int32_t getSourceNumFieldValue(int16_t val, int16_t min, int16_t max);

constexpr int DEFAULT_POINTS = 5;
constexpr int STD_CURVE_POINTS(int p) { return p + DEFAULT_POINTS; }
constexpr int CUSTOM_CURVE_POINTS(int p) { return 2 * p + (2 * DEFAULT_POINTS - 2); }

int8_t * curveEnd[MAX_CURVES];

uint8_t getCurvePoints(uint8_t index)
{
  if (index >= MAX_CURVES)
    return 0;

  const auto& curve = g_model.curves[index];
  if (curve.type == CURVE_TYPE_STANDARD)
    return STD_CURVE_POINTS(curve.points);
  else if (curve.type == CURVE_TYPE_CUSTOM)
    return CUSTOM_CURVE_POINTS(curve.points);

  return 0;
}

void loadCurves()
{
  bool showWarning= false;
  int8_t * tmp = g_model.points;
  for (int i=0; i<MAX_CURVES; i++) {
    switch (g_model.curves[i].type) {
      case CURVE_TYPE_STANDARD:
        tmp += STD_CURVE_POINTS(g_model.curves[i].points);
        break;
      case CURVE_TYPE_CUSTOM:
        tmp += CUSTOM_CURVE_POINTS(g_model.curves[i].points);
        break;
      default:
        TRACE("Wrong curve type! Fixing...");
        g_model.curves[i].type = CURVE_TYPE_STANDARD;
        tmp += STD_CURVE_POINTS(g_model.curves[i].points);
        break;
    }
    // Older version did not check if we exceeded the array
    int8_t * maxend = &g_model.points[MAX_CURVE_POINTS - 2*(MAX_CURVES-i-1)];
    if (tmp > maxend) {
      tmp = maxend;
      g_model.curves[i].type=CURVE_TYPE_STANDARD;
      g_model.curves[i].points=-3;
      showWarning=true;
    }
    curveEnd[i] = tmp;

  }
  if (showWarning) {
    POPUP_WARNING("Invalid curve data repaired", "check your curves, logic switches");
  }
}

int8_t * curveAddress(uint8_t idx)
{
  return idx==0 ? g_model.points : curveEnd[idx-1];
}

static void curveMove_unsafe(uint8_t index, int8_t shift)
{
  int8_t* src = curveAddress(index+1);
  int8_t* dst = src + shift;
  size_t points = curveEnd[MAX_CURVES-1] - curveEnd[index]
    + getCurvePoints(MAX_CURVES-1); // points in the last curve

  memmove(dst, src, points);
  if (shift < 0) {
    // clear points at the tail
    memclear(dst + points, src - dst);
  }

  while (index < MAX_CURVES) {
    curveEnd[index++] += shift;
  }
}

bool moveCurve(uint8_t index, int8_t shift)
{
  if (curveEnd[MAX_CURVES-1] + shift > g_model.points + sizeof(g_model.points)) {
    AUDIO_WARNING2();
    return false;
  }

  curveMove_unsafe(index, shift);
  
  storageDirty(EE_MODEL);
  return true;
}

int8_t getCurveX(int noPoints, int point)
{
  return -100 + divRoundClosest((point*2000) / (noPoints-1), 10);
}

void resetCustomCurveX(int8_t * points, int noPoints)
{
  for (int i=0; i<noPoints-2; i++) {
    points[noPoints+i] = getCurveX(noPoints, i+1);
  }
}

void curveClear(uint8_t index)
{
  if (index >= MAX_CURVES)
    return;
  
  CurveHeader * curve = &g_model.curves[index];
  int8_t * curvePoints = curveAddress(index);

  uint8_t nPoints = getCurvePoints(index);
  memclear(curvePoints, nPoints);

  memclear(curve, sizeof(CurveHeader));
  uint8_t newPoints = getCurvePoints(index);
  
  int8_t shift = newPoints - nPoints;
  if (shift != 0) {
    curveMove_unsafe(index, shift);
  }
}

void curveMirror(uint8_t index)
{
  if (index >= MAX_CURVES)
    return;
  
  CurveHeader & curve = g_model.curves[index];
  int8_t * points = curveAddress(index);

  // we only mirror Y axis: X axis does not change
  for (int i = 0; i < STD_CURVE_POINTS(curve.points); i++)
    points[i] = -points[i];
}

bool isCurveUsed(uint8_t index)
{
  return !is_memclear(&g_model.curves[index], sizeof(CurveHeader)) ||
         !is_memclear(curveAddress(index), DEFAULT_POINTS);
}

#define CUSTOM_POINT_X(points, count, idx) \
  ((idx) == 0 ? -100 : (((idx) == (count)-1) ? 100 : points[(count) + (idx)-1]))

int32_t compute_tangent(CurveHeader* crv, const int8_t* points, int i)
{
  int32_t m = 0;
  uint8_t num_points = STD_CURVE_POINTS(crv->points);
#define MMULT 1024
  if (i == 0) {
    // linear interpolation between 1st 2 points
    // keep 3 decimal-places for m
    if (crv->type == CURVE_TYPE_CUSTOM) {
      int8_t x0 = CUSTOM_POINT_X(points, num_points, 0);
      int8_t x1 = CUSTOM_POINT_X(points, num_points, 1);
      if (x1 > x0) m = (MMULT * (points[1] - points[0])) / (x1 - x0);
    } else {
      int32_t delta = (2 * 100) / (num_points - 1);
      m = (MMULT * (points[1] - points[0])) / delta;
    }
  } else if (i == num_points - 1) {
    // linear interpolation between last 2 points
    // keep 3 decimal-places for m
    if (crv->type == CURVE_TYPE_CUSTOM) {
      int8_t x0 = CUSTOM_POINT_X(points, num_points, num_points - 2);
      int8_t x1 = CUSTOM_POINT_X(points, num_points, num_points - 1);
      if (x1 > x0)
        m = (MMULT * (points[num_points - 1] - points[num_points - 2])) /
            (x1 - x0);
    } else {
      int32_t delta = (2 * 100) / (num_points - 1);
      m = (MMULT * (points[num_points - 1] - points[num_points - 2])) / delta;
    }
  } else {
    // apply monotone rules from
    // http://en.wikipedia.org/wiki/Monotone_cubic_interpolation
    // 1) compute slopes of secant lines
    int32_t d0 = 0, d1 = 0;
    if (crv->type == CURVE_TYPE_CUSTOM) {
      int8_t x0 = CUSTOM_POINT_X(points, num_points, i - 1);
      int8_t x1 = CUSTOM_POINT_X(points, num_points, i);
      int8_t x2 = CUSTOM_POINT_X(points, num_points, i + 1);
      if (x1 > x0) d0 = (MMULT * (points[i] - points[i - 1])) / (x1 - x0);
      if (x2 > x1) d1 = (MMULT * (points[i + 1] - points[i])) / (x2 - x1);
    } else {
      int32_t delta = (2 * 100) / (num_points - 1);
      d0 = (MMULT * (points[i] - points[i - 1])) / (delta);
      d1 = (MMULT * (points[i + 1] - points[i])) / (delta);
    }
    // 2) compute initial average tangent
    m = (d0 + d1) / 2;
    // 3 check for horizontal lines
    if (d0 == 0 || d1 == 0 || (d0 > 0 && d1 < 0) || (d0 < 0 && d1 > 0)) {
      m = 0;
    } else if (MMULT * m / d0 > 3 * MMULT) {
      m = 3 * d0;
    } else if (MMULT * m / d1 > 3 * MMULT) {
      m = 3 * d1;
    }
  }
  return m;
}

/* The following is a hermite cubic spline.
   The basis functions can be found here:
   http://en.wikipedia.org/wiki/Cubic_Hermite_spline
   The tangents are computed via the 'cubic monotone' rules (allowing for local-maxima)
*/
int16_t hermite_spline(int16_t x, uint8_t idx)
{
  CurveHeader &crv = g_model.curves[idx];
  int8_t *points = curveAddress(idx);
  uint8_t count = STD_CURVE_POINTS(crv.points);
  bool custom = (crv.type == CURVE_TYPE_CUSTOM);

  if (x < -RESX)
    x = -RESX;
  else if (x > RESX)
    x = RESX;

  for (int i=0; i<count-1; i++) {
    int32_t p0x, p3x;
    if (custom) {
      p0x = (i>0 ? calc100toRESX(points[count+i-1]) : -RESX);
      p3x = (i<count-2 ? calc100toRESX(points[count+i]) : RESX);
    }
    else {
      p0x = -RESX + (i*2*RESX)/(count-1);
      p3x = -RESX + ((i+1)*2*RESX)/(count-1);
    }

    if (x >= p0x && x <= p3x) {
      int32_t p0y = calc100toRESX(points[i]);
      int32_t p3y = calc100toRESX(points[i+1]);
      int32_t m0 = compute_tangent(&crv, points, i);
      int32_t m3 = compute_tangent(&crv, points, i+1);
      int32_t y;
      int32_t h = p3x - p0x;
      int32_t t = (h > 0 ? (MMULT * (x - p0x)) / h : 0);
      int32_t t2 = t * t / MMULT;
      int32_t t3 = t2 * t / MMULT;
      int32_t h00 = 2*t3 - 3*t2 + MMULT;
      int32_t h10 = t3 - 2*t2 + t;
      int32_t h01 = -2*t3 + 3*t2;
      int32_t h11 = t3 - t2;
      y = p0y * h00 + h * (m0 * h10 / MMULT) + p3y * h01 + h * (m3 * h11 / MMULT);
      y /= MMULT;
      return y;
    }
  }
  return 0;
}

int intpol(int x, uint8_t idx) // -100, -75, -50, -25, 0 ,25 ,50, 75, 100
{
  CurveHeader& crv = g_model.curves[idx];
  int8_t* points = curveAddress(idx);
  uint8_t count = STD_CURVE_POINTS(crv.points);
  bool custom = (crv.type == CURVE_TYPE_CUSTOM);
  int16_t erg;

  x += RESXu;

  if (x <= 0) {
    erg = (int16_t)points[0] * (RESX / 4);
  } else if (x >= (RESX * 2)) {
    erg = (int16_t)points[count - 1] * (RESX / 4);
  } else {
    uint16_t a = 0, b = 0;
    uint8_t i;
    if (custom) {
      for (i = 0; i < count - 1; i++) {
        a = b;
        b = (i == count - 2 ? 2 * RESX
                            : RESX + calc100toRESX(points[count + i]));
        if ((uint16_t)x <= b) break;
      }
    } else {
      uint16_t d = (RESX * 2) / (count - 1);
      i = (uint16_t)x / d;
      a = i * d;
      b = a + d;
    }
    erg = (int16_t)points[i] * (RESX / 4) +
          ((int32_t)(x - a) * (points[i + 1] - points[i]) * (RESX / 4)) /
              ((b - a));
  }

  return erg / 25; // 100*D5/RESX;
}

int applyCurve(int x, CurveRef & curve)
{
  SourceNumVal v;
  v.rawValue = curve.value;

  switch (curve.type) {
    case CURVE_REF_DIFF:
    {
      int curveParam = getSourceNumFieldValue(curve.value, -100, 100);
      if (curveParam > 0 && x < 0)
        x = (x * (1000 - curveParam)) / 1000;
      else if (curveParam < 0 && x > 0)
        x = (x * (1000 + curveParam)) / 1000;
      return x;
    }

    case CURVE_REF_EXPO:
    {
      int curveParam = getSourceNumFieldValue(curve.value, -100, 100) / 10;
      return expo(x, curveParam);
    }

    case CURVE_REF_FUNC:
      switch (v.value) {
        case CURVE_X_GT0:
          if (x < 0) x = 0; //x|x>0
          return x;
        case CURVE_X_LT0:
          if (x > 0) x = 0; //x|x<0
          return x;
        case CURVE_ABS_X: // x|abs(x)
          return abs(x);
        case CURVE_F_GT0: //f|f>0
          return x > 0 ? RESX : 0;
        case CURVE_F_LT0: //f|f<0
          return x < 0 ? -RESX : 0;
        case CURVE_ABS_F: //f|abs(f)
          return x > 0 ? RESX : -RESX;
      }
      break;

    case CURVE_REF_CUSTOM:
    {
      int curveParam = v.value;
      if (curveParam < 0) {
        x = -x;
        curveParam = -curveParam;
      }
      if (curveParam > 0 && curveParam <= MAX_CURVES) {
        return applyCustomCurve(x, curveParam - 1);
      }
      break;
    }
  }

  return x;
}

int applyCustomCurve(int x, uint8_t idx)
{
  if (idx >= MAX_CURVES)
    return 0;

  CurveHeader & crv = g_model.curves[idx];
  if (crv.smooth)
    return hermite_spline(x, idx);
  else
    return intpol(x, idx);
}

point_t getPoint(uint8_t curveIndex, uint8_t index)
{
  point_t result = {0, 0};
  CurveHeader & crv = g_model.curves[curveIndex];
  int8_t * points = curveAddress(curveIndex);
  bool custom = (crv.type == CURVE_TYPE_CUSTOM);
  uint8_t count = STD_CURVE_POINTS(crv.points);
  if (index < count) {
    if (custom && index > 0 && index < count - 1) {
      result.x = calc100toRESX(points[count + index - 1]);
    }
    else {
      result.x = -RESX + calc100toRESX(200 * index / (count - 1));
    }
    result.y = calc100toRESX(points[index]);
  }
  return result;
}

#if !defined(COLORLCD)
point_t getPoint(uint8_t index)
{
  point_t result = getPoint(s_currIdxSubMenu, index);
  result.x = CURVE_CENTER_X + divRoundClosest(result.x * CURVE_SIDE_WIDTH, RESX);
  result.y = CURVE_CENTER_Y - divRoundClosest(result.y * CURVE_SIDE_WIDTH, RESX);
  return result;
}

int applyCurrentCurve(int x)
{
  return applyCustomCurve(x, s_currIdxSubMenu);
}
#endif

#if defined(COLORLCD)
char *getCurveRefString(char *dest, size_t len, const CurveRef& curve)
{
  if (!len) return dest;
  char *s = dest;

  SourceNumVal v;
  v.rawValue = curve.value;

  if (v.value != 0) {
    switch (curve.type) {
      case CURVE_REF_DIFF:
        *(s++) = 'D'; if (--len == 0) return dest;
        getValueOrSrcVarString(s, len, curve.value, -100, 100, 0, "%");
        return dest;

      case CURVE_REF_EXPO:
        *(s++) = 'E'; if (--len == 0) return dest;
        getValueOrSrcVarString(s, len, curve.value, -100, 100, 0, "%");
        return dest;

      case CURVE_REF_FUNC:
        strAppend(dest, STR_VCURVEFUNC[v.value], len);
        return dest;

      case CURVE_REF_CUSTOM:
        return getCurveString(dest, v.value);
    }
  }

  return dest;
}
#endif
