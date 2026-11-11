// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include "gimbals.h"

namespace mambo
{
struct AxisCorrection {
  float ratio, denominatorCrossZ, numeratorCrossZ;
  float positiveGain, negativeGain, centre;
};
struct CalibrationPoint {
  int16_t x, y;
};
struct GimbalCorrection {
  AxisCorrection axes[2];
  CalibrationPoint points[9];
  // Four projective maps, each taking signed local X/Y to a unit square.
  float maps[4][6];
};
struct FactoryCalibration {
  GimbalCorrection gimbals[2];
};

// Captured Tracer 6.19 profile only. Strict current format: file CRC and both
// angle/quadrant CRCs must validate. On failure, output is left unchanged.
bool decodeFactoryCalibration(const uint8_t* data, size_t size,
                              FactoryCalibration& output);
bool correctGimbalPlane(const GimbalCorrection& correction, int16_t x,
                        int16_t y, float& correctedX, float& correctedY);
bool calibratedHall(const HallSample& sample,
                    const GimbalCorrection& correction, uint16_t& x,
                    uint16_t& y);
}  // namespace mambo

enum class MamboCalibrationStatus { Missing, Invalid, Loaded };
void mamboGimbalsLoadCalibration();
MamboCalibrationStatus mamboGimbalsCalibrationStatus();
