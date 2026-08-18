/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "automation_capture.h"

using namespace edgetx::automation;

namespace
{

class ScopedTempDirectory
{
 public:
  ScopedTempDirectory()
  {
    static std::atomic<unsigned int> nextId{0};
    const auto stamp =
        std::chrono::steady_clock::now().time_since_epoch().count();
    std::error_code error;
    for (unsigned int attempt = 0; attempt < 100; ++attempt) {
      directory = std::filesystem::temp_directory_path() /
                  ("edgetx-capture-test-" + std::to_string(stamp) + "-" +
                   std::to_string(nextId.fetch_add(1)));
      if (std::filesystem::create_directory(directory, error)) return;
      error.clear();
    }
    throw std::runtime_error("cannot create capture test directory");
  }

  ~ScopedTempDirectory()
  {
    std::error_code ignored;
    std::filesystem::remove_all(directory, ignored);
  }

  const std::filesystem::path& path() const { return directory; }

 private:
  std::filesystem::path directory;
};

std::vector<std::uint8_t> readBytes(const std::filesystem::path& path)
{
  std::ifstream stream(path, std::ios::binary);
  return std::vector<std::uint8_t>(std::istreambuf_iterator<char>(stream),
                                   std::istreambuf_iterator<char>());
}

bool waitForCompletion(AutomationCapture& capture,
                       CaptureCompletion* completion)
{
  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (std::chrono::steady_clock::now() < deadline) {
    if (capture.takeCompletion(completion)) return true;
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  return capture.takeCompletion(completion);
}

CaptureOperationResult injectedWriteFailure(const CaptureWriteRequest&,
                                            std::atomic<int>&)
{
  return CaptureOperationResult::failure(ErrorCode::CaptureFailed,
                                         "injected capture write failure");
}

CaptureOperationResult injectedWriterException(const CaptureWriteRequest&,
                                               std::atomic<int>&)
{
  throw std::runtime_error("injected writer exception");
}

std::atomic<bool> blockingWriterEntered{false};

CaptureOperationResult waitForCancellation(const CaptureWriteRequest&,
                                           std::atomic<int>& commitState)
{
  blockingWriterEntered.store(true, std::memory_order_release);
  while (commitState.load(std::memory_order_acquire) == 0) {
    std::this_thread::yield();
  }
  return CaptureOperationResult::failure(
      ErrorCode::CaptureCancelled, "injected writer observed cancellation");
}

void configureCapture(AutomationCapture& capture,
                      const std::filesystem::path& root,
                      CaptureWriteFunction writer = writeRgb565Ppm)
{
  std::string error;
  ASSERT_TRUE(capture.configure(root.u8string(), 2, 2, 16, &error, writer))
      << error;
  ASSERT_TRUE(capture.configured());
}

CaptureArtifactPath validatedPath(AutomationCapture& capture,
                                  const std::string& relative, RequestId id = 1)
{
  CaptureArtifactPath artifact;
  const CaptureOperationResult result =
      capture.validatePath(relative, id, 1, &artifact);
  EXPECT_TRUE(result.ok) << result.message;
  return artifact;
}

}  // namespace

TEST(SimuAutomationCapturePath, AcceptsContainedUtf8AndInternalSpaces)
{
  ScopedTempDirectory output;
  ASSERT_TRUE(
      std::filesystem::create_directory(output.path() / "nested folder"));
  AutomationCapture capture;
  configureCapture(capture, output.path());

  const std::string relative =
      "nested folder/\xc3\xa9"
      "cran image.ppm";
  CaptureArtifactPath artifact;
  const CaptureOperationResult result =
      capture.validatePath(relative, 7, 3, &artifact);

  ASSERT_TRUE(result.ok) << result.message;
  EXPECT_EQ(artifact.relative, relative);
  EXPECT_EQ(artifact.finalPath.filename(),
            std::filesystem::u8path("\xc3\xa9"
                                    "cran image.ppm"));
  EXPECT_EQ(artifact.finalPath.parent_path(),
            std::filesystem::canonical(output.path() / "nested folder"));
}

TEST(SimuAutomationCapturePath, RejectsUnsafeMissingAndExistingTargets)
{
  ScopedTempDirectory output;
  AutomationCapture capture;
  configureCapture(capture, output.path());
  const auto absolute = (output.path() / "absolute.ppm").u8string();

  const std::array<std::string, 7> unsafe = {
      "",
      "../escape.ppm",
      "./local.ppm",
      "wrong.PNG",
      "missing/child.ppm",
      absolute,
      "C:/rooted.ppm",
  };
  for (const std::string& path : unsafe) {
    CaptureArtifactPath artifact;
    const CaptureOperationResult result =
        capture.validatePath(path, 1, 1, &artifact);
    EXPECT_FALSE(result.ok) << path;
  }

  CaptureArtifactPath tooLongArtifact;
  const CaptureOperationResult tooLong = capture.validatePath(
      std::string(MAX_CAPTURE_PATH_BYTES + 1, 'a'), 1, 1, &tooLongArtifact);
  EXPECT_FALSE(tooLong.ok);
  EXPECT_EQ(tooLong.errorCode, ErrorCode::PathTooLong);

#if defined(_WIN32)
  CaptureArtifactPath reservedArtifact;
  const CaptureOperationResult reserved =
      capture.validatePath("CON.ppm", 1, 1, &reservedArtifact);
  EXPECT_FALSE(reserved.ok);
  EXPECT_EQ(reserved.errorCode, ErrorCode::UnsafePath);
#endif

  const std::filesystem::path existing = output.path() / "existing.ppm";
  std::ofstream(existing, std::ios::binary) << "keep";
  CaptureArtifactPath artifact;
  const CaptureOperationResult result =
      capture.validatePath("existing.ppm", 1, 1, &artifact);
  EXPECT_FALSE(result.ok);
  EXPECT_EQ(result.errorCode, ErrorCode::ArtifactExists);
  EXPECT_EQ(readBytes(existing),
            (std::vector<std::uint8_t>{'k', 'e', 'e', 'p'}));
}

TEST(SimuAutomationCapturePath, RejectsDanglingFinalAndEscapingParentSymlinks)
{
  ScopedTempDirectory output;
  ScopedTempDirectory outside;
  AutomationCapture capture;
  configureCapture(capture, output.path());

  std::error_code symlinkError;
  std::filesystem::create_symlink(output.path() / "missing-target",
                                  output.path() / "dangling.ppm", symlinkError);
  if (!symlinkError) {
    CaptureArtifactPath artifact;
    const CaptureOperationResult result =
        capture.validatePath("dangling.ppm", 1, 1, &artifact);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.errorCode, ErrorCode::ArtifactExists);
  }

  symlinkError.clear();
  std::filesystem::create_directory_symlink(
      outside.path(), output.path() / "outside", symlinkError);
  if (!symlinkError) {
    CaptureArtifactPath artifact;
    const CaptureOperationResult result =
        capture.validatePath("outside/escape.ppm", 2, 1, &artifact);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.errorCode, ErrorCode::UnsafePath);
  }
}

TEST(SimuAutomationCaptureInvalidation, CoalescesAndConsumesOneShotRequests)
{
  (void)consumeAutomationLcdInvalidation();
  requestAutomationLcdInvalidation();
  requestAutomationLcdInvalidation();

  EXPECT_TRUE(consumeAutomationLcdInvalidation());
  EXPECT_FALSE(consumeAutomationLcdInvalidation());
}

TEST(SimuAutomationCaptureWriter, EmitsCanonicalPpmAndKnownRgb565Values)
{
  ScopedTempDirectory output;
  const std::array<std::uint16_t, 5> pixels = {
      0x0000, 0xffff, 0xf800, 0x07e0, 0x001f,
  };
  CaptureWriteRequest request;
  request.artifactPath.relative = "colors.ppm";
  request.artifactPath.finalPath = output.path() / "colors.ppm";
  request.artifactPath.temporaryPath = output.path() / ".colors.ppm.tmp";
  request.pixels = pixels.data();
  request.pixelCount = pixels.size();
  request.width = 5;
  request.height = 1;
  std::atomic<int> commitState{0};

  const CaptureOperationResult result = writeRgb565Ppm(request, commitState);

  ASSERT_TRUE(result.ok) << result.message;
  const std::string header = "P6\n5 1\n255\n";
  const std::vector<std::uint8_t> bytes =
      readBytes(request.artifactPath.finalPath);
  ASSERT_EQ(bytes.size(), header.size() + 15);
  EXPECT_TRUE(std::equal(header.begin(), header.end(), bytes.begin()));
  const std::vector<std::uint8_t> expectedRaster = {
      0, 0, 0, 255, 255, 255, 255, 0, 0, 0, 255, 0, 0, 0, 255,
  };
  EXPECT_TRUE(std::equal(expectedRaster.begin(), expectedRaster.end(),
                         bytes.begin() + header.size()));
  EXPECT_EQ(result.bytes, bytes.size());
  EXPECT_FALSE(std::filesystem::exists(request.artifactPath.temporaryPath));
}

TEST(SimuAutomationCaptureWriter, PreservesExistingFinalAndCleansFailedPublish)
{
  ScopedTempDirectory output;
  const std::array<std::uint16_t, 1> pixels = {0xffff};
  const std::filesystem::path finalPath = output.path() / "existing.ppm";
  std::ofstream(finalPath, std::ios::binary) << "keep";
  CaptureWriteRequest request;
  request.artifactPath.finalPath = finalPath;
  request.artifactPath.temporaryPath = output.path() / ".existing.ppm.tmp";
  request.pixels = pixels.data();
  request.pixelCount = pixels.size();
  request.width = 1;
  request.height = 1;
  std::atomic<int> commitState{0};

  const CaptureOperationResult result = writeRgb565Ppm(request, commitState);

  EXPECT_FALSE(result.ok);
  EXPECT_EQ(result.errorCode, ErrorCode::ArtifactExists);
  EXPECT_EQ(readBytes(finalPath),
            (std::vector<std::uint8_t>{'k', 'e', 'e', 'p'}));
  EXPECT_FALSE(std::filesystem::exists(request.artifactPath.temporaryPath));
}

TEST(SimuAutomationCaptureWriter, ReportsOpenFailureWithoutPartialArtifact)
{
  ScopedTempDirectory output;
  const std::array<std::uint16_t, 1> pixels = {0};
  CaptureWriteRequest request;
  request.artifactPath.finalPath = output.path() / "never-created.ppm";
  request.artifactPath.temporaryPath =
      output.path() / "missing" / ".capture.tmp";
  request.pixels = pixels.data();
  request.pixelCount = pixels.size();
  request.width = 1;
  request.height = 1;
  std::atomic<int> commitState{0};

  const CaptureOperationResult result = writeRgb565Ppm(request, commitState);

  EXPECT_FALSE(result.ok);
  EXPECT_EQ(result.errorCode, ErrorCode::CaptureFailed);
  EXPECT_NE(result.message.find("open"), std::string::npos);
  EXPECT_FALSE(std::filesystem::exists(request.artifactPath.finalPath));
  EXPECT_FALSE(std::filesystem::exists(request.artifactPath.temporaryPath));
}

TEST(SimuAutomationCaptureCoordinator, RequiresNewerFrameAndReturnsMetadata)
{
  ScopedTempDirectory output;
  AutomationCapture capture;
  configureCapture(capture, output.path());
  CaptureArtifactPath artifact = validatedPath(capture, "fresh.ppm");
  ASSERT_TRUE(capture.arm(1, 1, 10, std::move(artifact)).ok);
  const std::array<std::uint16_t, 4> pixels = {0, 0xffff, 0xf800, 0x001f};

  capture.onDisplayFrame(10, 1, pixels.data(), pixels.size());
  CaptureCompletion completion;
  EXPECT_FALSE(capture.takeCompletion(&completion));
  capture.onDisplayFrame(11, 1, pixels.data(), pixels.size());

  ASSERT_TRUE(waitForCompletion(capture, &completion));
  ASSERT_TRUE(completion.ok) << completion.message;
  EXPECT_EQ(completion.id, 1u);
  EXPECT_EQ(completion.epoch, 1u);
  EXPECT_EQ(completion.artifact.displaySequence, 11u);
  EXPECT_EQ(completion.artifact.path, "fresh.ppm");
  EXPECT_EQ(completion.artifact.width, 2u);
  EXPECT_EQ(completion.artifact.height, 2u);
  EXPECT_EQ(completion.artifact.depth, 16u);
  EXPECT_TRUE(std::filesystem::exists(output.path() / "fresh.ppm"));
}

TEST(SimuAutomationCaptureCoordinator, EnforcesOneSlotAndCancelsArmedCapture)
{
  ScopedTempDirectory output;
  AutomationCapture capture;
  configureCapture(capture, output.path());
  CaptureArtifactPath first = validatedPath(capture, "first.ppm", 1);
  CaptureArtifactPath second = validatedPath(capture, "second.ppm", 2);
  ASSERT_TRUE(capture.arm(1, 1, 1, std::move(first)).ok);

  const CaptureOperationResult busy = capture.arm(2, 1, 1, std::move(second));
  EXPECT_FALSE(busy.ok);
  EXPECT_EQ(busy.errorCode, ErrorCode::OperationBusy);

  CaptureCompletion completion;
  ASSERT_TRUE(capture.cancelAndWait(&completion));
  EXPECT_FALSE(completion.ok);
  EXPECT_EQ(completion.errorCode, ErrorCode::CaptureCancelled);
  EXPECT_FALSE(std::filesystem::exists(output.path() / "first.ppm"));
}

TEST(SimuAutomationCaptureCoordinator, CancelsWriterBeforeCommit)
{
  ScopedTempDirectory output;
  AutomationCapture capture;
  blockingWriterEntered.store(false, std::memory_order_release);
  configureCapture(capture, output.path(), waitForCancellation);
  CaptureArtifactPath artifact = validatedPath(capture, "cancelled.ppm");
  ASSERT_TRUE(capture.arm(1, 1, 1, std::move(artifact)).ok);
  const std::array<std::uint16_t, 4> pixels = {0, 0, 0, 0};
  capture.onDisplayFrame(2, 1, pixels.data(), pixels.size());

  const auto deadline =
      std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (!blockingWriterEntered.load(std::memory_order_acquire) &&
         std::chrono::steady_clock::now() < deadline) {
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }
  ASSERT_TRUE(blockingWriterEntered.load(std::memory_order_acquire));

  CaptureCompletion completion;
  ASSERT_TRUE(capture.cancelAndWait(&completion));
  EXPECT_EQ(completion.errorCode, ErrorCode::CaptureCancelled);
  EXPECT_FALSE(std::filesystem::exists(output.path() / "cancelled.ppm"));
}

TEST(SimuAutomationCaptureCoordinator, PropagatesFailureAndContainsExceptions)
{
  const std::array<std::uint16_t, 4> pixels = {0, 0, 0, 0};
  for (CaptureWriteFunction writer :
       {injectedWriteFailure, injectedWriterException}) {
    ScopedTempDirectory output;
    AutomationCapture capture;
    configureCapture(capture, output.path(), writer);
    CaptureArtifactPath artifact = validatedPath(capture, "failure.ppm");
    ASSERT_TRUE(capture.arm(1, 1, 1, std::move(artifact)).ok);
    capture.onDisplayFrame(2, 1, pixels.data(), pixels.size());

    CaptureCompletion completion;
    ASSERT_TRUE(waitForCompletion(capture, &completion));
    EXPECT_FALSE(completion.ok);
    EXPECT_EQ(completion.errorCode, ErrorCode::CaptureFailed);
    EXPECT_NE(completion.message.find("injected"), std::string::npos);
    EXPECT_FALSE(std::filesystem::exists(output.path() / "failure.ppm"));
  }
}

TEST(SimuAutomationCaptureWriter, IsStableAcrossTwentyRunsAndDetectsChange)
{
  ScopedTempDirectory output;
  const std::array<std::uint16_t, 4> staticPixels = {
      0x0000,
      0xffff,
      0xf800,
      0x001f,
  };
  std::vector<std::uint8_t> baseline;

  for (int index = 0; index < 20; ++index) {
    const std::string name = "static-" + std::to_string(index) + ".ppm";
    CaptureWriteRequest request;
    request.artifactPath.finalPath = output.path() / name;
    request.artifactPath.temporaryPath = output.path() / ("." + name + ".tmp");
    request.pixels = staticPixels.data();
    request.pixelCount = staticPixels.size();
    request.width = 2;
    request.height = 2;
    std::atomic<int> commitState{0};
    ASSERT_TRUE(writeRgb565Ppm(request, commitState).ok);
    const std::vector<std::uint8_t> bytes =
        readBytes(request.artifactPath.finalPath);
    if (index == 0)
      baseline = bytes;
    else
      EXPECT_EQ(bytes, baseline) << index;
  }

  const std::array<std::uint16_t, 4> changedPixels = {
      0x0000,
      0xffff,
      0xf800,
      0x07e0,
  };
  CaptureWriteRequest changed;
  changed.artifactPath.finalPath = output.path() / "changed.ppm";
  changed.artifactPath.temporaryPath = output.path() / ".changed.ppm.tmp";
  changed.pixels = changedPixels.data();
  changed.pixelCount = changedPixels.size();
  changed.width = 2;
  changed.height = 2;
  std::atomic<int> commitState{0};
  ASSERT_TRUE(writeRgb565Ppm(changed, commitState).ok);
  EXPECT_NE(readBytes(changed.artifactPath.finalPath), baseline);
}
