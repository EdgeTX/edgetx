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

#include "storage/yaml/yaml_tree_walker.h"
#include "storage/yaml/yaml_parser.h"
#include "storage/yaml/yaml_datastructs.h"
#include "hal/adc_driver.h"
#include "analogs.h"

// User supplied strings stored via custom YAML writers must be emitted as
// double-quoted, escaped scalars. Written raw, characters such as ': ', ' #'
// or a leading quote / bracket produce YAML that Companion cannot load
// (#7829), and '\' or '"' are mangled by the radio's own parser.

struct YamlStringCase {
  const char* raw;
  const char* quoted;  // expected YAML scalar as written by the radio
};

static const YamlStringCase stringCases[] = {
  {"Alt:", "\"Alt:\""},  // #7829: trailing colon
  {"a: b", "\"a: b\""},
  {"x #y", "\"x #y\""},
  {"[x] &y *z", "\"[x] &y *z\""},
  {"\"q\"", "\"\\x22q\\x22\""},
  {"back\\slash", "\"back\\x5Cslash\""},
};

// Analog custom labels are limited to LEN_ANA_NAME (3) characters
static const YamlStringCase analogCases[] = {
  {"a:", "\"a:\""},
  {"#x", "\"#x\""},
  {"a\"b", "\"a\\x22b\""},
  {"\\", "\"\\x5C\""},
};

static bool yaml_string_writer(void* opaque, const char* str, size_t len)
{
  static_cast<std::string*>(opaque)->append(str, len);
  return true;
}

static std::string writeYaml(const YamlNode* root, void* data)
{
  std::string yaml;
  YamlTreeWalker tree;
  tree.reset(root, (uint8_t*)data);
  EXPECT_TRUE(tree.generate(yaml_string_writer, &yaml));
  return yaml;
}

static void readYaml(const YamlNode* root, void* data, const std::string& yaml)
{
  YamlTreeWalker tree;
  tree.reset(root, (uint8_t*)data);
  YamlParser yp;
  yp.init(YamlTreeWalker::get_parser_calls(), &tree);
  yp.set_eof();
  yp.parse(yaml.c_str(), yaml.size());
}

// Value written for 'key' on the first line matching it after 'section'.
static std::string yamlValue(const std::string& yaml, const std::string& section,
                             const std::string& key)
{
  size_t pos = yaml.find(section);
  if (pos == std::string::npos) return "<no section " + section + ">";
  std::string tag = key + ": ";
  pos = yaml.find(tag, pos);
  if (pos == std::string::npos) return "<no key " + key + ">";
  pos += tag.size();
  return yaml.substr(pos, yaml.find_first_of("\r\n", pos) - pos);
}

// Remove quotes around 'value' if the writer added them
static void stripQuotes(std::string& yaml, const std::string& value)
{
  auto quoted = "\"" + value + "\"";
  auto pos = yaml.find(quoted);
  if (pos != std::string::npos) yaml.replace(pos, quoted.size(), value);
  EXPECT_NE(std::string::npos, yaml.find(": " + value + "\r\n")) << value;
}

class YamlStringTest : public EdgeTxTest
{
 protected:
  void SetUp() override
  {
    EdgeTxTest::SetUp();
    resetStorage();
  }

  void TearDown() override { resetStorage(); }

  // Screen / topbar data and analog labels live in static
  // storage outside g_model / g_eeGeneral, so they must be cleared explicitly.
  static void resetStorage()
  {
    g_model.resetScreenData();
    for (uint8_t type : {ADC_INPUT_MAIN, ADC_INPUT_FLEX}) {
      for (uint8_t i = 0; i < adcGetMaxInputs(type); i += 1)
        analogSetCustomLabel(type, i, "", LEN_ANA_NAME);
    }
  }

  static std::string writeModel()
  {
    return writeYaml(get_modeldata_nodes(), &g_model);
  }

  static void readModel(const std::string& yaml)
  {
    g_model.resetScreenData();
    readYaml(get_modeldata_nodes(), &g_model, yaml);
  }

  static std::string writeRadio()
  {
    return writeYaml(get_radiodata_nodes(), &g_eeGeneral);
  }

  static void readRadio(const std::string& yaml)
  {
    resetStorage();
    readYaml(get_radiodata_nodes(), &g_eeGeneral, yaml);
  }
};

TEST_F(YamlStringTest, WidgetOptionString)
{
  for (const auto& c : stringCases) {
    SCOPED_TRACE(c.raw);
    resetStorage();

    g_model.setScreenLayoutId(0, "Layout1x1");
    g_model.getScreenLayoutData(0)->setWidgetName(0, "Text");
    auto wd = g_model.getWidgetData(0, 0);
    wd->setType(0, WOV_String);
    wd->setString(0, c.raw);

    auto yaml = writeModel();
    EXPECT_EQ(c.quoted, yamlValue(yaml, "screenData:", "stringValue"));

    readModel(yaml);
    wd = g_model.getWidgetData(0, 0);
    EXPECT_EQ(WOV_String, wd->getType(0));
    EXPECT_EQ(c.raw, wd->getString(0));
  }
}

TEST_F(YamlStringTest, ScreenWidgetName)
{
  for (const auto& c : stringCases) {
    SCOPED_TRACE(c.raw);
    resetStorage();

    g_model.setScreenLayoutId(0, "Layout1x1");
    g_model.getScreenLayoutData(0)->setWidgetName(0, c.raw);

    auto yaml = writeModel();
    EXPECT_EQ(c.quoted, yamlValue(yaml, "screenData:", "widgetName"));

    readModel(yaml);
    EXPECT_STREQ(c.raw, g_model.getScreenLayoutData(0)->getWidgetName(0));
  }
}

TEST_F(YamlStringTest, TopbarWidgetName)
{
  for (const auto& c : stringCases) {
    SCOPED_TRACE(c.raw);
    resetStorage();

    g_model.getTopbarData()->setWidgetName(0, c.raw);

    auto yaml = writeModel();
    EXPECT_EQ(c.quoted, yamlValue(yaml, "topbarData:", "widgetName"));

    readModel(yaml);
    EXPECT_STREQ(c.raw, g_model.getTopbarData()->getWidgetName(0));
  }
}

TEST_F(YamlStringTest, AnalogLabels)
{
  for (const auto& c : analogCases) {
    SCOPED_TRACE(c.raw);
    resetStorage();

    analogSetCustomLabel(ADC_INPUT_MAIN, 0, c.raw, strlen(c.raw));
    analogSetCustomLabel(ADC_INPUT_FLEX, 0, c.raw, strlen(c.raw));

    auto yaml = writeRadio();
    EXPECT_EQ(c.quoted, yamlValue(yaml, "sticksConfig:", "name"));
    EXPECT_EQ(c.quoted, yamlValue(yaml, "potsConfig:", "name"));

    readRadio(yaml);
    EXPECT_STREQ(c.raw, analogGetCustomLabel(ADC_INPUT_MAIN, 0));
    EXPECT_STREQ(c.raw, analogGetCustomLabel(ADC_INPUT_FLEX, 0));
  }
}

// Files written before quoting was added must still load
TEST_F(YamlStringTest, ReadsLegacyUnquotedValues)
{
  g_model.setScreenLayoutId(0, "Layout1x1");
  g_model.getScreenLayoutData(0)->setWidgetName(0, "Text");
  auto wd = g_model.getWidgetData(0, 0);
  wd->setType(0, WOV_String);
  wd->setString(0, "Hello");

  auto model = writeModel();
  stripQuotes(model, "Text");
  stripQuotes(model, "Hello");

  readModel(model);
  EXPECT_STREQ("Text", g_model.getScreenLayoutData(0)->getWidgetName(0));
  EXPECT_EQ("Hello", g_model.getWidgetData(0, 0)->getString(0));
}

#endif
