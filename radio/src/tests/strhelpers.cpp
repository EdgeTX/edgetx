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
#include "strhelpers.h"

TEST(StrHelpers, ReplaceAllEmptyFrom)
{
  std::string s = "hello world";
  strReplaceAll(s, "", "x");
  EXPECT_EQ(s, "hello world");
}

TEST(StrHelpers, ReplaceAllEmptyTo)
{
  std::string s = "hello world";
  strReplaceAll(s, "o", "");
  EXPECT_EQ(s, "hell wrld");
}

TEST(StrHelpers, ReplaceAllNoMatch)
{
  std::string s = "hello world";
  strReplaceAll(s, "xyz", "abc");
  EXPECT_EQ(s, "hello world");
}

TEST(StrHelpers, ReplaceAllOverlappingMatches)
{
  std::string s = "aaaa";
  strReplaceAll(s, "aa", "a");
  EXPECT_EQ(s, "aa");
}

TEST(StrHelpers, ReplaceAllToContainsFrom)
{
  std::string s = "cat";
  strReplaceAll(s, "cat", "concatenate");
  EXPECT_EQ(s, "concatenate");
}

TEST(StrHelpers, ReplaceAllMultipleMatches)
{
  std::string s = "a,b,c,d";
  strReplaceAll(s, ",", " - ");
  EXPECT_EQ(s, "a - b - c - d");
}

TEST(StrHelpers, SwitchPositionNameSpecial)
{
  MODEL_RESET();
  char s[64];

  EXPECT_STREQ(STR_EMPTY, getSwitchPositionName(s, SWSRC_NONE));
  EXPECT_STREQ("Act", getSwitchPositionName(s, SWSRC_RADIO_ACTIVITY));
  EXPECT_STREQ("!Act", getSwitchPositionName(s, -SWSRC_RADIO_ACTIVITY));
  EXPECT_STREQ("Trn", getSwitchPositionName(s, SWSRC_TRAINER_CONNECTED));
  EXPECT_STREQ("Tele", getSwitchPositionName(s, SWSRC_TELEMETRY_STREAMING));
  EXPECT_STREQ("L01", getSwitchPositionName(s, SWSRC_FIRST_LOGICAL_SWITCH));
  EXPECT_STREQ("!L01", getSwitchPositionName(s, -SWSRC_FIRST_LOGICAL_SWITCH));
}

TEST(StrHelpers, SwitchPositionNameSensor)
{
  MODEL_RESET();
  char s[64];

  strncpy(g_model.telemetrySensors[0].label, "Tst", TELEM_LABEL_LEN);
  strncpy(g_model.telemetrySensors[MAX_TELEMETRY_SENSORS - 1].label, "Lst",
          TELEM_LABEL_LEN);

  EXPECT_STREQ("Tst", getSwitchPositionName(s, SWSRC_FIRST_SENSOR));
  EXPECT_STREQ("!Tst", getSwitchPositionName(s, -SWSRC_FIRST_SENSOR));
  EXPECT_STREQ("Lst", getSwitchPositionName(s, SWSRC_LAST_SENSOR));
}

TEST(StrHelpers, SwitchPositionNameOutOfRange)
{
  MODEL_RESET();
  char s[64];

  for (swsrc_t sw : {SWSRC_LAST + 1, SWSRC_LAST + 100, 511, 100000}) {
    EXPECT_STREQ("", getSwitchPositionName(s, sw)) << "switch " << sw;
    EXPECT_STREQ("", getSwitchPositionName(s, -sw)) << "switch " << -sw;
  }
}

TEST(StrHelpers, CurveString)
{
  MODEL_RESET();
  char s[64];

  EXPECT_STREQ(STR_MMMINV[0], getCurveString(s, 0));
  EXPECT_STREQ("CV1", getCurveString(s, 1));
  EXPECT_STREQ("-CV1", getCurveString(s, -1));
  EXPECT_STREQ("CV32", getCurveString(s, MAX_CURVES));
  EXPECT_STREQ("-CV32", getCurveString(s, -MAX_CURVES));

  strncpy(g_model.curves[0].name, "Abc", LEN_CURVE_NAME);
  strncpy(g_model.curves[MAX_CURVES - 1].name, "Xyz", LEN_CURVE_NAME);
  EXPECT_STREQ("Abc", getCurveString(s, 1));
  EXPECT_STREQ("-Abc", getCurveString(s, -1));
  EXPECT_STREQ("Xyz", getCurveString(s, MAX_CURVES));
}

TEST(StrHelpers, CurveStringOutOfRange)
{
  MODEL_RESET();
  char s[64];

  // No custom curve name lookup past the last curve
  EXPECT_STREQ("CV33", getCurveString(s, MAX_CURVES + 1));
  EXPECT_STREQ("-CV33", getCurveString(s, -(MAX_CURVES + 1)));
  EXPECT_STREQ("CV511", getCurveString(s, 511));
}

#if defined(COLORLCD)
static CurveRef makeCurveRef(uint8_t type, int value)
{
  SourceNumVal v;
  v.isSource = false;
  v.value = value;
  CurveRef curve;
  curve.type = type;
  curve.value = v.rawValue;
  return curve;
}

TEST(StrHelpers, CurveRefString)
{
  MODEL_RESET();
  char s[64];

  for (uint8_t type : {CURVE_REF_DIFF, CURVE_REF_EXPO, CURVE_REF_FUNC,
                       CURVE_REF_CUSTOM}) {
    memset(s, 0, sizeof(s));
    EXPECT_STREQ("", getCurveRefString(s, sizeof(s), makeCurveRef(type, 0)));
  }

  memset(s, 0, sizeof(s));
  EXPECT_STREQ("D50%", getCurveRefString(s, sizeof(s),
                                         makeCurveRef(CURVE_REF_DIFF, 50)));
  memset(s, 0, sizeof(s));
  EXPECT_STREQ("E-20%", getCurveRefString(s, sizeof(s),
                                          makeCurveRef(CURVE_REF_EXPO, -20)));

  for (int i = 1; i < CURVE_BASE; i++) {
    memset(s, 0, sizeof(s));
    EXPECT_STREQ(STR_VCURVEFUNC[i],
                 getCurveRefString(s, sizeof(s),
                                   makeCurveRef(CURVE_REF_FUNC, i)));
  }

  memset(s, 0, sizeof(s));
  EXPECT_STREQ("CV1", getCurveRefString(s, sizeof(s),
                                        makeCurveRef(CURVE_REF_CUSTOM, 1)));
  memset(s, 0, sizeof(s));
  EXPECT_STREQ("-CV32",
               getCurveRefString(s, sizeof(s),
                                 makeCurveRef(CURVE_REF_CUSTOM, -MAX_CURVES)));
}

TEST(StrHelpers, CurveRefStringOutOfRange)
{
  MODEL_RESET();
  char s[64];

  for (int value : {(int)CURVE_BASE, 100, 511, -1, -512}) {
    memset(s, 0, sizeof(s));
    EXPECT_STREQ("", getCurveRefString(s, sizeof(s),
                                       makeCurveRef(CURVE_REF_FUNC, value)))
        << "func " << value;
  }

  memset(s, 0, sizeof(s));
  EXPECT_STREQ("CV33",
               getCurveRefString(s, sizeof(s),
                                 makeCurveRef(CURVE_REF_CUSTOM, MAX_CURVES + 1)));
}
#endif
