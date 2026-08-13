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

#if defined(COLORLCD)

#include "colors.h"
#include "widget.h"

TEST(color, RGB)
{
  // test conversion from RGB to RGB565 (5 bits for R, 6 bits for G, 5 bits for B)
  EXPECT_EQ(RGB(0, 0, 0), (uint16_t)0x0000);
  EXPECT_EQ(RGB(255, 255, 255), (uint16_t)0xFFFF);

  EXPECT_EQ(RGB(255, 0, 0), (uint16_t)0xF800);
  EXPECT_EQ(RGB(0, 255, 0), (uint16_t)0x07E0);
  EXPECT_EQ(RGB(0, 0, 255), (uint16_t)0x001F);

  EXPECT_EQ(RGB(30, 40, 150), (uint16_t)0x1952);
}

TEST(color, ARGB)
{
  // test conversion from RGB to ARGB4444 (4 bits for alpha, 4 bits for R, 4 bits for G, 4 bits for B)
  EXPECT_EQ(ARGB(0, 0, 0, 0), (uint16_t)0x0000);
  EXPECT_EQ(ARGB(255, 255, 255, 255), (uint16_t)0xFFFF);

  EXPECT_EQ(ARGB(255, 0, 0, 0), (uint16_t)0xF000);
  EXPECT_EQ(ARGB(0, 255, 0, 0), (uint16_t)0x0F00);
  EXPECT_EQ(ARGB(0, 0, 255, 0), (uint16_t)0x00F0);
  EXPECT_EQ(ARGB(0, 0, 0, 255), (uint16_t)0x000F);

  EXPECT_EQ(ARGB(128, 30, 40, 150), (uint16_t)0x8129);
}

TEST(Widget, focusStyleDoesNotLeak)
{
  lv_group_t* previousGroup = lv_group_get_default();
  lv_group_t* testGroup = lv_group_create();
  lv_group_set_default(testGroup);

  // Keep a separate object focused so the Widget focus handler is not invoked.
  lv_obj_t* groupAnchor = lv_obj_create(nullptr);
  lv_group_add_obj(testGroup, groupAnchor);

  auto exercise = [] {
    Widget widget(nullptr, nullptr, {0, 0, 64, 64}, 0, 0);
    for (int i = 0; i < 16; ++i) {
      widget.enableFocus(true);
      widget.enableFocus(false);
    }
    widget.enableFocus(true);
  };

  // Warm up LVGL's object allocation paths before measuring.
  exercise();

  lv_mem_monitor_t before = {};
  lv_mem_monitor(&before);

  for (int i = 0; i < 8; ++i) {
    exercise();
  }

  lv_mem_monitor_t after = {};
  lv_mem_monitor(&after);

  lv_group_set_default(previousGroup);
  lv_obj_del(groupAnchor);
  lv_group_del(testGroup);
  EXPECT_EQ(before.free_size, after.free_size);
}

#endif
