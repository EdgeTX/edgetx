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
