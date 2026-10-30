// SPDX-License-Identifier: GPL-2.0-only
#include "hal/adc_driver.h"
#include "edgetx.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

BatteryTestSettings g_eeGeneral;
static uint16_t counts;
static bool available = true;
unsigned adcGetMaxInputs(unsigned type) { return type == ADC_INPUT_VBAT && available ? 1 : 0; }
unsigned adcGetInputOffset(unsigned) { return 7; }
uint16_t anaIn(unsigned input) { if (input != 7) std::abort(); return counts; }
static bool nearVoltage(int expected)
{
  int actual = getBatteryVoltage();
  if (std::abs(actual - expected) <= 1) return true;
  std::fprintf(stderr, "counts=%u, calibration=%d: got %d, expected %d\n",
               counts, g_eeGeneral.txVoltageCalibration, actual, expected);
  return false;
}

int main()
{
  // Exercise the production conversion over the full filtered ADC domain,
  // independently comparing its result to the stock counts / 4.55 formula.
  for (counts = 0; counts <= ADC_MAX_FILTERED; ++counts) {
    int expected = int(counts / 4.55);
    if (std::abs(int(getBatteryVoltage()) - expected) > 1) {
      std::fprintf(stderr, "counts=%u: got %u, expected %d\n", counts, getBatteryVoltage(), expected);
      return 1;
    }
  }
  counts = 1820; // 4.00 V nominal
  if (!nearVoltage(400)) return 1;
  g_eeGeneral.txVoltageCalibration = 8;
  if (!nearVoltage(425)) return 1;
  g_eeGeneral.txVoltageCalibration = -8;
  if (!nearVoltage(375)) return 1;
  available = false;
  if (getBatteryVoltage() != 0) return 1;
  std::puts("Battery conversion matches stock across 2048 ADC values; calibration and absent-input checks pass.");
}
