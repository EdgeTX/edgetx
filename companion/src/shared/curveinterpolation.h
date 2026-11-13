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

#pragma once

class CurveData;

constexpr int CURVE_MAX = 100;             // maximum absolute value of a coordinate on the curve
constexpr int CURVE_RANGE = 2 * CURVE_MAX; // full range from -CURVE_MAX to +CURVE_MAX

/**
 * Computes the value of a smooth curve at a given x-coordinate using monotone cubic tangents and Hermite interpolation.
 * This function matches the behavior of the radio's smooth curves, but uses floating-point arithmetic instead of the radio's fixed-point calculations.
 */
double hermite_spline(double x, const CurveData & curve);
