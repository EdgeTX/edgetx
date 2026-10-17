// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include "calibration.h"

namespace mambo
{
enum class HallHealth { Waiting, Fresh, Stale, Error };
inline HallHealth hallHealth(const HallSample& sample, uint32_t nowUs)
{
  if (!sample.valid)
    return sample.errors ? HallHealth::Error : HallHealth::Waiting;
  return uint32_t(nowUs - sample.timestamp) < 20000 ? HallHealth::Fresh
                                                    : HallHealth::Stale;
}
struct BenchDiagnostics {
  HallSample sensors[2];
  uint16_t sticks[4] = {2048, 2048, 2048, 2048};
  uint32_t frames = 0;
  uint32_t lastDurationUs = 0, maxDurationUs = 0;
  uint32_t resetFlags = 0;
  MamboCalibrationStatus calibration = MamboCalibrationStatus::Missing;
};
}  // namespace mambo
// Stable published snapshot, safe for the menu task. It never initiates SPI,
// SD I/O or changes calibration. Freshness is evaluated using the current time.
mambo::BenchDiagnostics mamboBenchDiagnostics();
