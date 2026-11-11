// SPDX-License-Identifier: GPL-2.0-only
#include "targets/mambo/inputs.h"

// Check documented nominal resistor ratios and the original right-vertical
// trim orientation, including idle/shorted/ambiguous boundary readings.
static_assert(mambo::decodeTrimLadder(4095) == 0);
static_assert(mambo::decodeTrimLadder(0) == 0);
static_assert(mambo::decodeTrimLadder(3071) == 0);
static_assert(mambo::decodeTrimLadder(4095 / 2) == 0x01);
static_assert(mambo::decodeTrimLadder(4095 / 3) == 0x02);
static_assert(mambo::decodeTrimLadder(4095 / 4) == 0x04);
static_assert(mambo::decodeTrimLadder(4095 / 5) == 0x08);
static_assert(mambo::decodeTrimLadder(4095 / 6) == 0x20);
static_assert(mambo::decodeTrimLadder(4095 / 7) == 0x10);
static_assert(mambo::decodeTrimLadder(4095 / 8) == 0x40);
static_assert(mambo::decodeTrimLadder(4095 / 9) == 0x80);
int main() { return 0; }
