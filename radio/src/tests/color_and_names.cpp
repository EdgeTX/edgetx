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

// Theme RGB() parsing and default model names (colour targets)

#include "gtests.h"

#if defined(COLORLCD)

#include <filesystem>
#include <fstream>
#include <string>

#include "colors.h"
#include "location.h"
#include "sdcard.h"
#include "storage/modelslist.h"
#include "themes/theme_manager.h"

namespace fs = std::filesystem;

TEST(Theme, rgbColors)
{
  fs::path dir = fs::temp_directory_path() / "edgetx-gtest-theme";
  std::error_code ec;
  fs::remove_all(dir, ec);
  fs::create_directories(dir / "THEMES" / "Test", ec);
  ASSERT_FALSE(ec);
  {
    std::ofstream f(dir / "THEMES" / "Test" / "theme.yml");
    f << "---\n"
         "summary:\n"
         "  name: Test\n"
         "  author: gtest\n"
         "  info: RGB parsing\n"
         "colors:\n"
         "  PRIMARY1: RGB(8,16,24)\n"
         "  PRIMARY2: RGB( 40, 80, 120 )\n"
         "  PRIMARY3: RGB(200,100,48 )\n"
         "  SECONDARY1: RGB(,200,200)\n"
         "  SECONDARY2: RGB(200,200)\n"
         "  SECONDARY3: RGB(200 ,200,200)\n"
         "  FOCUS: 0x102030\n";
  }
  simuFatfsSetPaths(dir.string().c_str(), nullptr);

  ThemeFile theme(std::string(THEMES_PATH) + "/Test/theme.yml", true);
  auto color = [&](LcdColorIndex i) {
    auto* e = theme.getColorEntryByIndex(i);
    return e ? e->colorValue : 0xFFFFFFFFu;
  };
  EXPECT_EQ(color(COLOR_THEME_PRIMARY1_INDEX), (uint32_t)RGB(8, 16, 24));
  EXPECT_EQ(color(COLOR_THEME_PRIMARY2_INDEX), (uint32_t)RGB(40, 80, 120));
  // space before ')' is accepted, as sscanf did
  EXPECT_EQ(color(COLOR_THEME_PRIMARY3_INDEX), (uint32_t)RGB(200, 100, 48));
  // malformed values are rejected (0); components large enough to survive
  // the colour packing, so an accepted value can't also read as 0
  EXPECT_EQ(color(COLOR_THEME_SECONDARY1_INDEX), 0u);  // empty component
  EXPECT_EQ(color(COLOR_THEME_SECONDARY2_INDEX), 0u);  // two components
  EXPECT_EQ(color(COLOR_THEME_SECONDARY3_INDEX), 0u);  // space before ','
  EXPECT_EQ(color(COLOR_THEME_FOCUS_INDEX), (uint32_t)RGB(0x10, 0x20, 0x30));

  simuFatfsSetPaths(TESTS_PATH, nullptr);
  fs::remove_all(dir, ec);
}

#if defined(STORAGE_MODELSLIST)
TEST(ModelCell, defaultName)
{
  struct {
    const char* file;
    const char* name;
  } cases[] = {
      {"model5.yml", "MODEL05"},
      {"model123.yml", "MODEL123"},
      {"model999999.yml", "MODEL999999"},  // longest that fits modelName
      {"model1000000.yml", "model1000000"},  // falls back to the file name
      {"mymodel.yml", "mymodel"},
  };
  for (auto& c : cases) {
    ModelCell cell(c.file);
    cell.setDefaultName();
    EXPECT_STREQ(cell.modelName, c.name) << c.file;
  }
}
#endif

#endif  // COLORLCD
