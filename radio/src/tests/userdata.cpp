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

class UserDataTest : public EdgeTxTest {};

TEST_F(UserDataTest, SetGetString)
{
  EXPECT_TRUE(g_model.setUserData("key1", "hello"));
  auto ud = g_model.getUserData("key1");
  ASSERT_NE(ud, nullptr);
  EXPECT_EQ(ud->type, UD_STRING);
  EXPECT_EQ(ud->value, "hello");
}

TEST_F(UserDataTest, SetGetInt)
{
  EXPECT_TRUE(g_model.setUserData("key1", (int32_t)12345));
  auto ud = g_model.getUserData("key1");
  ASSERT_NE(ud, nullptr);
  EXPECT_EQ(ud->type, UD_INT);
  EXPECT_EQ(strtol(ud->value.c_str(), nullptr, 10), 12345);
}

TEST_F(UserDataTest, FloatRoundTrip)
{
  // std::to_string() fixes 6 decimal places and would truncate this to
  // "0.000000" - %.9g must preserve enough precision to round-trip.
  float f = 1.234567e-5f;
  EXPECT_TRUE(g_model.setUserData("key1", f));
  auto ud = g_model.getUserData("key1");
  ASSERT_NE(ud, nullptr);
  EXPECT_EQ(ud->type, UD_FLOAT);
  EXPECT_NEAR(strtof(ud->value.c_str(), nullptr), f, f * 1e-5f);
}

TEST_F(UserDataTest, UpdateExistingChangesType)
{
  // A value/type change on an existing key must persist even when the
  // string representation happens to be identical either side.
  EXPECT_TRUE(g_model.setUserData("key1", (int32_t)123));
  EXPECT_TRUE(g_model.setUserData("key1", "123"));
  auto ud = g_model.getUserData("key1");
  ASSERT_NE(ud, nullptr);
  EXPECT_EQ(ud->type, UD_STRING);
}

TEST_F(UserDataTest, DeleteRemovesEntry)
{
  g_model.setUserData("key1", "a");
  g_model.deleteUserData("key1");
  EXPECT_EQ(g_model.getUserData("key1"), nullptr);
  EXPECT_EQ(g_model.getUserDataCount(), 0);
}

TEST_F(UserDataTest, MaxEntriesRespected)
{
  char key[16];
  for (int i = 0; i < MAX_USER_DATA; i += 1) {
    snprintf(key, sizeof(key), "k%d", i);
    EXPECT_TRUE(g_model.setUserData(key, i));
  }
  EXPECT_FALSE(g_model.setUserData("overflow", 0));
  EXPECT_EQ(g_model.getUserDataCount(), MAX_USER_DATA);
}

TEST_F(UserDataTest, GetOrCreateSparseIndex)
{
  // Simulates the YAML reader encountering a non-contiguous index
  // (e.g. a hand-edited or Companion-generated model file): the entry
  // must land at the requested index, not be appended elsewhere.
  auto ud = g_model.getOrCreateUserData(3);
  ASSERT_NE(ud, nullptr);
  ud->key = "k3";
  EXPECT_EQ(g_model.getUserDataCount(), 4);
  EXPECT_EQ(g_model.getUserData(3)->key, "k3");
  // Gap slots exist but are inactive (empty key)
  EXPECT_FALSE(g_model.hasUserData(0));
  EXPECT_TRUE(g_model.hasUserData(3));
}

TEST_F(UserDataTest, GetOrCreateRespectsMax)
{
  EXPECT_EQ(g_model.getOrCreateUserData(MAX_USER_DATA), nullptr);
}
