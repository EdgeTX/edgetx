// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstdint>
struct BatteryTestSettings { int8_t txVoltageCalibration = 0; };
extern BatteryTestSettings g_eeGeneral;
