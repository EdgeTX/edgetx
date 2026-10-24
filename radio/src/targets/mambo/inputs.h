// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <stdint.h>

namespace mambo
{

constexpr uint32_t decodeTrimLadder(uint16_t value)
{
  // Single-key resistor-ladder decoder from the matching FreedomTX driver.
  constexpr uint16_t levels[] = {4095 / 2, 4095 / 3, 4095 / 4, 4095 / 5,
                                 4095 / 6, 4095 / 7, 4095 / 8, 4095 / 9};
  for (unsigned i = 0; i < 8; ++i) {
    const uint16_t hi = ((i ? levels[i - 1] : 4095) + levels[i]) / 2;
    const uint16_t lo = (levels[i] + (i == 7 ? 0 : levels[i + 1])) / 2;
    if (value < hi && value > lo) {
      const unsigned bit = i == 4 ? 5 : i == 5 ? 4 : i;
      return 1u << bit;
    }
  }
  return 0;
}

}  // namespace mambo
