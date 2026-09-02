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

#include "gtests.h"

#include <string>
#include <vector>

#include "firmwares/boundedstring.h"
#include "firmwares/edgetx/yaml_ops.h"

// A representative spread of inputs, including boundary and over-length cases
// and a multi-byte UTF-8 sequence that straddles the truncation point.
static const std::vector<std::string> kInputs = {
  "",
  "model1.yml",
  "0123456789012345",          // 16 bytes: exactly the filename capacity
  "01234567890123456789",      // over the filename capacity
  std::string(160, 'X'),       // far over any capacity
  "caf\xC3\xA9",               // "café" (UTF-8), 5 bytes
};

// --- Construction & assignment -------------------------------------------

TEST(BoundedString, AssignTruncatesToCapacity)
{
  BoundedString<16> bs;
  bs = std::string(40, 'A');
  EXPECT_EQ(16u, bs.size());
  EXPECT_EQ(std::string(16, 'A'), bs.str());
}

TEST(BoundedString, ShortValueIsUntouched)
{
  BoundedString<16> bs("model1.yml");
  EXPECT_EQ("model1.yml", bs.str());
  EXPECT_FALSE(bs.empty());
}

TEST(BoundedString, AssignFromVariousSources)
{
  BoundedString<16> a("c-string");
  BoundedString<16> b(std::string("std-string"));
  BoundedString<16> c(QString("qstring"));
  EXPECT_EQ("c-string", a.str());
  EXPECT_EQ("std-string", b.str());
  EXPECT_EQ("qstring", c.str());
  EXPECT_EQ(QString("qstring"), c.toQString());
}

TEST(BoundedString, NullCStringIsEmpty)
{
  const char* p = nullptr;
  BoundedString<16> bs(p);
  EXPECT_TRUE(bs.empty());
}

TEST(BoundedString, Comparison)
{
  BoundedString<16> a("abc");
  BoundedString<16> b("abc");
  BoundedString<16> c("xyz");
  EXPECT_TRUE(a == b);
  EXPECT_TRUE(a != c);
  EXPECT_TRUE(a == std::string("abc"));
  EXPECT_TRUE(a != std::string("abcd"));
  EXPECT_TRUE(a == "abc");        // const char*
}

// --- YAML round-trip ------------------------------------------------------

TEST(BoundedString, YamlRoundTrip)
{
  for (const auto& in : kInputs) {
    BoundedString<16> src(in);  // truncates on construction
    YAML::Node node;
    node["v"] = src;

    BoundedString<16> dst;
    node["v"] >> dst;

    EXPECT_EQ(src.str(), dst.str()) << "round-trip mismatch for input: " << in;
  }
}

TEST(BoundedString, YamlDecodeTruncatesOverlongScalar)
{
  // A malicious/oversized scalar must be clamped on decode, not overflow.
  YAML::Node node;
  node["v"] = std::string(200, 'Z');

  BoundedString<16> dst;
  node["v"] >> dst;
  EXPECT_EQ(16u, dst.size());
}

// --- Byte-for-byte equivalence with the legacy char[N+1] path -------------
// This is the gate that protects radio YAML compatibility: BoundedString<N>
// must produce identical bytes to a legacy `char[N+1]` field on both encode
// and decode, for every input.

TEST(BoundedString, MatchesLegacyCharArrayOnDecode)
{
  constexpr size_t CAP = 16;
  for (const auto& in : kInputs) {
    YAML::Node node;
    node["v"] = in;

    char legacy[CAP + 1] = {0};
    node["v"] >> legacy;

    BoundedString<CAP> modern;
    node["v"] >> modern;

    EXPECT_EQ(std::string(legacy), modern.str())
        << "decode diverged from legacy char[] for input: " << in;
  }
}

TEST(BoundedString, MatchesLegacyCharArrayOnEncode)
{
  constexpr size_t CAP = 16;
  for (const auto& in : kInputs) {
    char legacy[CAP + 1] = {0};
    {
      // seed the legacy buffer the same way decode would
      YAML::Node seed;
      seed["v"] = in;
      seed["v"] >> legacy;
    }
    BoundedString<CAP> modern(in);

    YAML::Node a, b;
    a["v"] = legacy;
    b["v"] = modern;

    YAML::Emitter ea, eb;
    ea << a;
    eb << b;
    EXPECT_EQ(std::string(ea.c_str()), std::string(eb.c_str()))
        << "encode diverged from legacy char[] for input: " << in;
  }
}
