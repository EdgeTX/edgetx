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

// radio.yml must always validate against its own stored checksum, including
// when it was loaded with "manuallyEdited: 1" and a valid checksum (as a
// third-party editor, or a hand edit that drops the checksum line, produces).

#include <filesystem>
#include <memory>

#include "gtests.h"
#include "location.h"
#include "storage/sdcard_common.h"
#include "storage/sdcard_yaml.h"
#include "storage/yaml/yaml_datastructs.h"
#include "storage/yaml/yaml_tree_walker.h"

namespace fs = std::filesystem;

class RadioSettingsFsTest : public ::testing::Test
{
 protected:
  fs::path scratchDir;

  void SetUp() override
  {
    scratchDir =
        fs::temp_directory_path() / fs::path("edgetx-gtest-radio-settings");
    std::error_code ec;
    fs::remove_all(scratchDir, ec);
    fs::create_directories(scratchDir / "RADIO", ec);
    ASSERT_FALSE(ec) << "could not create scratch RADIO directory";

    simuFatfsSetPaths(scratchDir.string().c_str(), nullptr);
    memclear(&g_eeGeneral, sizeof(g_eeGeneral));
  }

  void TearDown() override
  {
    simuFatfsSetPaths(TESTS_PATH, nullptr);
    memclear(&g_eeGeneral, sizeof(g_eeGeneral));

    std::error_code ec;
    fs::remove_all(scratchDir, ec);
  }

  // Parse radio.yml from the SD card with checksum validation, exactly as
  // done at boot, into a scratch buffer (g_eeGeneral is left untouched).
  ChecksumResult validateRadioYaml(uint8_t* manuallyEdited = nullptr)
  {
    auto data = std::make_unique<RadioData>();
    memclear(data.get(), sizeof(RadioData));

    YamlTreeWalker tree;
    tree.reset(get_radiodata_nodes(), (uint8_t*)data.get());

    ChecksumResult result = ChecksumResult::None;
    const char* err =
        readYamlFile(RADIO_SETTINGS_YAML_PATH,
                     YamlTreeWalker::get_parser_calls(), &tree, &result);
    EXPECT_EQ(err, nullptr) << err;
    if (manuallyEdited) *manuallyEdited = data->manuallyEdited;
    return result;
  }
};

TEST_F(RadioSettingsFsTest, SaveWithManuallyEditedSetValidates)
{
  g_eeGeneral.manuallyEdited = 1;
  ASSERT_EQ(writeGeneralSettings(), nullptr);

  uint8_t flag = 0xFF;
  EXPECT_EQ(validateRadioYaml(&flag), ChecksumResult::Success)
      << "radio.yml written by the radio fails its own checksum";
  EXPECT_EQ(flag, 0);
}

TEST_F(RadioSettingsFsTest, ValidChecksumWithManuallyEditedSurvivesSave)
{
  // Emulate an external editor: valid checksum AND manuallyEdited: 1
  g_eeGeneral.manuallyEdited = 1;
  uint16_t checksum = 0;
  ASSERT_TRUE(YamlFileChecksum(get_radiodata_nodes(), (uint8_t*)&g_eeGeneral,
                               &checksum));
  ASSERT_EQ(writeFileYaml(RADIO_SETTINGS_YAML_PATH, get_radiodata_nodes(),
                          (uint8_t*)&g_eeGeneral, checksum),
            nullptr);
  ASSERT_EQ(validateRadioYaml(), ChecksumResult::Success);

  // Boot: the file validates, so the manual-edit recovery branch is not taken
  memclear(&g_eeGeneral, sizeof(g_eeGeneral));
  ASSERT_EQ(loadRadioSettingsYaml(true), nullptr);
  EXPECT_EQ(g_eeGeneral.manuallyEdited, 0)
      << "manuallyEdited must not stay set after a successful load";

  // Next save must produce a file that validates on the following boot
  ASSERT_EQ(writeGeneralSettings(), nullptr);
  EXPECT_EQ(validateRadioYaml(), ChecksumResult::Success)
      << "radio.yml would be renamed to radio_error.yml on next boot";
}
