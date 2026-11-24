/*
 * Copyright (C) EdgeTX
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

// Tests the firmware's own printf (thirdparty/printf) and strtof, which
// replace the C library on radios. Expected values are exact (computed with
// arbitrary precision), not taken from the host C library.

#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <string>

#include "gtests.h"

extern "C" {
int snprintf_(char* s, size_t n, const char* format, ...);
float __wrap_strtof(const char* nptr, char** endptr);
void putchar_(char c) { (void)c; }
}

static float fromBits(uint32_t b)
{
  float f;
  memcpy(&f, &b, sizeof(f));
  return f;
}

static uint32_t toBits(float f)
{
  uint32_t b;
  memcpy(&b, &f, sizeof(b));
  return b;
}

static std::string fmt(const char* format, float f)
{
  char buf[512];
  int n = snprintf_(buf, sizeof(buf), format, (double)f);
  EXPECT_EQ((size_t)n, strlen(buf)) << format;
  return buf;
}

struct FormatCase {
  const char* format;
  uint32_t bits;
  const char* expected;
};

static const FormatCase formatCases[] = {
  // last-digit rounding
  {"%.7g", 0x3EAAAAABu, "0.3333333"},
  {"%.7g", 0x58635FA9u, "1e+15"},
  {"%.10g", 0x3EAAAAABu, "0.3333333433"},
  // more than 10 decimals
  {"%.20f", 0x3F000000u, "0.50000000000000000000"},
  {"%.9g", 0x3A83126Fu, "0.00100000005"},
  {"%.30f", 0x3DCCCCCDu, "0.100000001490116119384765625000"},
  {"%.12g", 0x3DCCCCCDu, "0.10000000149"},
  // subnormals
  {"%e", 0x00000001u, "1.401298e-45"},
  {"%g", 0x00000001u, "1.4013e-45"},
  {"%g", 0x80000001u, "-1.4013e-45"},
  {"%.3e", 0x00800000u, "1.175e-38"},
  {"%.149f", 0x00000001u, "0.00000000000000000000000000000000000000000000140129846432481707092372958328991613128026194187651577175706828388979108268586060148663818836212158203125"},
  {"%.112e", 0x007FFFFFu, "1.1754942106924410754870294448492873488270524287458933338571745305715888704756189042655023513361811637878417968750e-38"},
  // %f never switches to exponent form
  {"%.0f", 0x4F32D05Eu, "3000000000"},
  {"%f", 0x7F7FFFFFu, "340282346638528859811704183484516925440.000000"},
  {"%.0f", 0xFF7FFFFFu, "-340282346638528859811704183484516925440"},
  {"%.40g", 0x7F7FFFFFu, "340282346638528859811704183484516925440"},
  // ties to even
  {"%.0f", 0x3F000000u, "0"},
  {"%.0f", 0x3FC00000u, "2"},
  {"%.0f", 0x40200000u, "2"},
  {"%.1f", 0x3E800000u, "0.2"},
  {"%.2f", 0x3E000000u, "0.12"},
  {"%.0e", 0x40200000u, "2e+00"},
  {"%.3g", 0x490DB940u, "5.8e+05"},
  // %g rules
  {"%#g", 0x497423F8u, "1.00000e+06"},  // glibc prints 1.e+06
  {"%g", 0x47C35000u, "100000"},
  {"%g", 0x49742400u, "1e+06"},
  {"%g", 0x38D1B717u, "0.0001"},
  {"%g", 0x3727C5ACu, "1e-05"},
  {"%#g", 0x00000000u, "0.00000"},
  {"%g", 0x00000000u, "0"},
  {"%g", 0x80000000u, "-0"},
  {"%G", 0x2EDBE6FFu, "1E-10"},
  // flags and width
  {"%#.0f", 0x40400000u, "3."},
  {"%#.0e", 0x40400000u, "3.e+00"},
  {"%.3f", 0xB9D1B717u, "-0.000"},
  {"%+.2f", 0x3F80A3D7u, "+1.00"},
  {"% .1e", 0x42F6E979u, " 1.2e+02"},
  {"%010.3f", 0xC0500000u, "-00003.250"},
  {"%-10.1f|", 0x40200000u, "2.5       |"},
  {"%+012.4e", 0x3DCCCCCDu, "+01.0000e-01"},
  {"%E", 0x4640E400u, "1.234500E+04"},
  // inf and nan: sign, width and case as C prints them (spaces, not zeros)
  {"%f", 0x7F800000u, "inf"},
  {"%F", 0x7F800000u, "INF"},
  {"%E", 0xFF800000u, "-INF"},
  {"%G", 0x7FC00000u, "NAN"},
  {"%g", 0xFFC00000u, "-nan"},
  {"%+f", 0x7F800000u, "+inf"},
  {"%+g", 0x7FC00000u, "+nan"},
  {"% e", 0x7F800000u, " inf"},
  {"%8f", 0xFF800000u, "    -inf"},
  {"%-6g|", 0x7F800000u, "inf   |"},
  {"%08f", 0x7F800000u, "     inf"},
};

TEST(FloatConversion, printfExact)
{
  for (const auto& c : formatCases) {
    EXPECT_EQ(fmt(c.format, fromBits(c.bits)), c.expected)
        << c.format << " of 0x" << std::hex << c.bits;
  }
}

TEST(FloatConversion, printfLongOutput)
{
  // %.99f of -FLT_MAX: the longest Lua can request (140 characters)
  char buf[200];
  EXPECT_EQ(snprintf_(buf, sizeof(buf), "%.99f", (double)-fromBits(0x7F7FFFFFu)), 140);
  // truncated output still reports the full length
  EXPECT_EQ(snprintf_(buf, 10, "%.99f", (double)-fromBits(0x7F7FFFFFu)), 140);
  EXPECT_STREQ(buf, "-34028234");
}

struct ParseCase {
  const char* input;
  uint32_t bits;
  int consumed;
  bool erange;
};

static const ParseCase parseCases[] = {
  {"0.1", 0x3DCCCCCDu, 3, false},
  {"3.14", 0x4048F5C3u, 4, false},
  {"1.17549435e-38", 0x00800000u, 14, false},
  {"3.4028235e38", 0x7F7FFFFFu, 12, false},
  // ties to even, and digits past the 120 kept still count
  {"16777217", 0x4B800000u, 8, false},
  {"16777217.000000000000000000000000000000000000000000000000000000000000"
   "0000000000000000000000000000000000000000000000000000000000000000000001",
   0x4B800001u, 139, false},
  {"340282356779733661637539395458142568447", 0x7F7FFFFFu, 39, false},
  {"340282356779733661637539395458142568448", 0x7F800000u, 39, true},
  // underflow
  {"1e-45", 0x00000001u, 5, false},
  {"7.0064923216240853e-46", 0x00000000u, 22, true},
  {"7.0064923216240854e-46", 0x00000001u, 22, false},
  {"1e-50", 0x00000000u, 5, true},
  // hex floats; glibc 2.39 returns 0 for the first one
  {"0x1.000001p-150", 0x00000001u, 15, false},
  {"0x1p-150", 0x00000000u, 8, true},
  {"0x1.8p-150", 0x00000001u, 10, false},
  {"0x1.fffffep127", 0x7F7FFFFFu, 14, false},
  {"0x1.ffffffp127", 0x7F800000u, 14, true},
  {"0x1P+4", 0x41800000u, 6, false},
  // syntax
  {"  +1.5e-3x", 0x3AC49BA6u, 9, false},
  {".5", 0x3F000000u, 2, false},
  {"5.", 0x40A00000u, 2, false},
  {"1e", 0x3F800000u, 1, false},
  {"1e+", 0x3F800000u, 1, false},
  {"0x", 0x00000000u, 1, false},
  {"-0", 0x80000000u, 2, false},
  {"-Infinity", 0xFF800000u, 9, false},
  {"inf", 0x7F800000u, 3, false},
  {"e5", 0x00000000u, 0, false},
  {"-", 0x00000000u, 0, false},  // no conversion: +0, not -0
  {"-abc", 0x00000000u, 0, false},
  {"-0x", 0x80000000u, 2, false},  // "-0" converts
  {"", 0x00000000u, 0, false},
};

TEST(FloatConversion, strtofExact)
{
  for (const auto& c : parseCases) {
    char* end;
    errno = 0;
    float f = __wrap_strtof(c.input, &end);
    EXPECT_EQ(toBits(f), c.bits) << c.input;
    EXPECT_EQ(end - c.input, c.consumed) << c.input;
    EXPECT_EQ(errno == ERANGE, c.erange) << c.input;
  }

  char* end;
  const char* nan = "nan(123)";
  EXPECT_TRUE(std::isnan(__wrap_strtof(nan, &end)));
  EXPECT_EQ(end - nan, 8);
}

// Both conversions are correctly rounded, so 9 significant digits always
// read back as the same float.
TEST(FloatConversion, roundTrip)
{
  uint32_t x = 2463534242u;
  for (int i = 0; i < 200000; i++) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    float f = fromBits(x);
    if (!std::isfinite(f)) continue;
    for (const char* format : {"%.9g", "%.8e"}) {
      std::string s = fmt(format, f);
      EXPECT_EQ(toBits(__wrap_strtof(s.c_str(), nullptr)), x) << s;
    }
  }
}
