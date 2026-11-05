/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include <filesystem>
#include <string>
#include <vector>

#include "arg_parser.h"
#include "gtests.h"

namespace
{

class Arguments
{
 public:
  Arguments(std::initializer_list<const char*> arguments)
  {
    for (const char* argument : arguments) values.emplace_back(argument);
    for (std::string& value : values) pointers.push_back(value.data());
  }

  int count() const { return static_cast<int>(pointers.size()); }
  char** data() { return pointers.data(); }

 private:
  std::vector<std::string> values;
  std::vector<char*> pointers;
};

class OutputDirectory
{
 public:
  OutputDirectory() :
      path(std::filesystem::current_path() / "simu automation arg parser test")
  {
    std::error_code error;
    std::filesystem::remove_all(path, error);
    error.clear();
    EXPECT_TRUE(std::filesystem::create_directory(path, error));
    EXPECT_FALSE(error);
  }

  ~OutputDirectory()
  {
    std::error_code error;
    std::filesystem::remove_all(path, error);
  }

  std::filesystem::path path;
};

}  // namespace

TEST(SimuArgumentParser, AutomationIsDisabledByDefault)
{
  Arguments arguments{"simu", "--width", "800"};
  ArgumentParser parser("simu");

  EXPECT_TRUE(parser.parse(arguments.count(), arguments.data()));
  EXPECT_FALSE(parser.isAutomationStdio());
  EXPECT_FALSE(parser.hasAutomationOutputPath());
  EXPECT_EQ(parser.getWidth(), 800);
}

TEST(SimuArgumentParser, AutomationOptionsMustBeUsedTogether)
{
  Arguments missingOutput{"simu", "--automation-stdio"};
  ArgumentParser parserMissingOutput("simu");
  testing::internal::CaptureStderr();
  EXPECT_FALSE(
      parserMissingOutput.parse(missingOutput.count(), missingOutput.data()));
  EXPECT_FALSE(testing::internal::GetCapturedStderr().empty());

  Arguments outputWithoutMode{"simu", "--automation-output", "."};
  ArgumentParser parserOutputWithoutMode("simu");
  testing::internal::CaptureStdout();
  EXPECT_FALSE(parserOutputWithoutMode.parse(outputWithoutMode.count(),
                                             outputWithoutMode.data()));
  EXPECT_FALSE(testing::internal::GetCapturedStdout().empty());
}

TEST(SimuArgumentParser, CanonicalizesExistingAutomationOutputDirectory)
{
  OutputDirectory output;
  const std::string outputPath = output.path.string();
  Arguments arguments{"simu", "--automation-output", outputPath.c_str(),
                      "--automation-stdio"};
  ArgumentParser parser("simu");

  ASSERT_TRUE(parser.parse(arguments.count(), arguments.data()));
  EXPECT_TRUE(parser.isAutomationStdio());
  EXPECT_TRUE(parser.hasAutomationOutputPath());
  EXPECT_EQ(std::filesystem::path(parser.getAutomationOutputPath()),
            std::filesystem::canonical(output.path));
}

TEST(SimuArgumentParser, RejectsMissingAutomationOutputDirectory)
{
  const auto missing =
      std::filesystem::current_path() / "simu-automation-output-does-not-exist";
  std::error_code error;
  std::filesystem::remove_all(missing, error);
  const std::string missingPath = missing.string();
  Arguments arguments{"simu", "--automation-stdio", "--automation-output",
                      missingPath.c_str()};
  ArgumentParser parser("simu");

  testing::internal::CaptureStderr();
  EXPECT_FALSE(parser.parse(arguments.count(), arguments.data()));
  EXPECT_FALSE(testing::internal::GetCapturedStderr().empty());
}

TEST(SimuArgumentParser, AutomationParseErrorsLeaveStdoutEmpty)
{
  Arguments arguments{"simu", "--unknown", "--automation-stdio"};
  ArgumentParser parser("simu");

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();
  EXPECT_FALSE(parser.parse(arguments.count(), arguments.data()));
  const std::string standardError = testing::internal::GetCapturedStderr();
  const std::string standardOutput = testing::internal::GetCapturedStdout();

  EXPECT_TRUE(standardOutput.empty());
  EXPECT_NE(standardError.find("Unknown option: --unknown"), std::string::npos);
}

TEST(SimuArgumentParser, HelpSkipsAutomationDependencyValidation)
{
  Arguments arguments{"simu", "--automation-stdio", "--help"};
  ArgumentParser parser("simu");

  EXPECT_TRUE(parser.parse(arguments.count(), arguments.data()));
  EXPECT_TRUE(parser.isHelpRequested());

  testing::internal::CaptureStdout();
  testing::internal::CaptureStderr();
  parser.printHelp();
  const std::string standardError = testing::internal::GetCapturedStderr();
  const std::string standardOutput = testing::internal::GetCapturedStdout();

  EXPECT_TRUE(standardOutput.empty());
  EXPECT_NE(standardError.find("--automation-stdio"), std::string::npos);
  EXPECT_NE(standardError.find("--automation-output"), std::string::npos);
}
