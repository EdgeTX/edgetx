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

#include <QApplication>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory>
#include "gtests.h"

#include "storage.h"
#include "opentxinterface.h"
#include "customdebug.h"


using ::testing::TestEventListener;
using ::testing::EmptyTestEventListener;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;
using ::testing::InitGoogleTest;

namespace {

void printColorCode(int color)
{
#if !GTEST_OS_WINDOWS
  if (isatty(fileno(stdout)))
    fprintf(stdout, "\033[0;%dm", color);
#endif
}

/**
 * Replaces gtest's PrettyUnitTestResultPrinter with something less verbose.
 * Uses the default printer if/when the messages might be interesting,
 * e.g. for full error info or tests summary.
*/
class TersePrinter : public EmptyTestEventListener  {
  public:
    explicit TersePrinter(TestEventListener * prettyPrinter)
    : prettyPrinter(prettyPrinter), currentTestInfo(NULL)
    {
    }
    void OnTestProgramStart(const UnitTest & unit_test)
    {
      prettyPrinter->OnTestProgramStart(unit_test);
    }
    void OnTestIterationStart(const UnitTest & unit_test, int iteration)
    {
      printColorCode(36);
      fprintf(stdout,  "############ ");
      fprintf(stdout, "Running %d test(s) from %d test case(s), iteration: %d.\n", unit_test.test_to_run_count(), unit_test.test_case_to_run_count(), iteration + 1);
      printColorCode(0);
      fflush(stdout);
    }
    void OnTestCaseStart(const TestCase & test_case)
    {
      prettyPrinter->OnTestCaseStart(test_case);
    }
    void OnTestStart(const TestInfo & test_info)
    {
      currentTestInfo = &test_info;
    }
    void OnTestPartResult(const TestPartResult & test_part_result)
    {
      if (test_part_result.failed()) {
        prettyPrinter->OnTestStart(*currentTestInfo);
        prettyPrinter->OnTestPartResult(test_part_result);
      }
    }
    void OnTestEnd(const TestInfo & test_info)
    {
      currentTestInfo = NULL;
      if (test_info.result()->Failed()) {
        prettyPrinter->OnTestEnd(test_info);
      }
      else {
        printColorCode(32);
        fprintf(stdout, "+");
        printColorCode(0);
        fflush(stdout);
      }
    }
    void OnTestCaseEnd(const TestCase & test_case)
    {
      fprintf(stdout, "\n");
      fflush(stdout);
    }
    void OnTestIterationEnd(const UnitTest & unit_test, int iteration)
    {
      prettyPrinter->OnTestIterationEnd(unit_test, iteration);
    }
    void OnTestProgramEnd(const UnitTest & unit_test)
    {
      prettyPrinter->OnTestProgramEnd(unit_test);
    }

  private:
    // gtest's default unit test result printer.
    std::unique_ptr<TestEventListener> prettyPrinter;
    // The currently running TestInfo, if any.
    const TestInfo * currentTestInfo;
};

}  // anonymous namespace

int main(int argc, char **argv)
{
  QCoreApplication app(argc, argv);

  CustomDebug::setFilterRules();
  registerStorageFactories();
  registerOpenTxFirmwares();

  InitGoogleTest(&argc, argv);

  // use --verbose option to revert to gtest's default output format
  bool verbose = false;
  if (argc > 1 && !strcmp(argv[1], "--verbose"))
    verbose = true;

  if (!verbose) {
    TestEventListeners & listeners = UnitTest::GetInstance()->listeners();
    TestEventListener * defaultPrinter = listeners.Release(listeners.default_result_printer());
    listeners.Append(new TersePrinter(defaultPrinter));
  }

  return RUN_ALL_TESTS();
}
