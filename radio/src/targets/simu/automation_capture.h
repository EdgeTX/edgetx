/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "automation_protocol.h"

namespace edgetx
{
namespace automation
{

struct CaptureArtifactPath {
  std::string relative;
  std::filesystem::path finalPath;
  std::filesystem::path temporaryPath;
};

struct CaptureCompletion {
  RequestId id = 0;
  SessionEpoch epoch = 0;
  bool ok = false;
  ErrorCode errorCode = ErrorCode::CaptureFailed;
  std::string message;
  CaptureResult artifact;

  bool active() const { return id != 0; }
};

struct CaptureOperationResult {
  bool ok = false;
  ErrorCode errorCode = ErrorCode::CaptureFailed;
  std::string message;
  std::uint64_t bytes = 0;

  static CaptureOperationResult success(std::uint64_t bytes);
  static CaptureOperationResult failure(ErrorCode code,
                                        const std::string& message);
};

struct CaptureWriteRequest {
  CaptureArtifactPath artifactPath;
  const std::uint16_t* pixels = nullptr;
  std::size_t pixelCount = 0;
  std::uint16_t width = 0;
  std::uint16_t height = 0;
};

using CaptureWriteFunction = CaptureOperationResult (*)(
    const CaptureWriteRequest& request, std::atomic<int>& commitState);

// commitState is 0 while cancellation may win, 1 after cancellation wins, and
// 2 after the worker owns the final publication step.
CaptureOperationResult writeRgb565Ppm(const CaptureWriteRequest& request,
                                      std::atomic<int>& commitState);

class AutomationCapture
{
 public:
  AutomationCapture() = default;
  ~AutomationCapture();

  AutomationCapture(const AutomationCapture&) = delete;
  AutomationCapture& operator=(const AutomationCapture&) = delete;

  bool configure(const std::string& outputRoot, std::uint16_t width,
                 std::uint16_t height, std::uint8_t depth, std::string* error,
                 CaptureWriteFunction writeFunction = writeRgb565Ppm);
  bool configured() const;

  CaptureOperationResult validatePath(const std::string& relativePath,
                                      RequestId id, SessionEpoch epoch,
                                      CaptureArtifactPath* artifactPath) const;
  CaptureOperationResult arm(RequestId id, SessionEpoch epoch,
                             DisplaySequence armedAfter,
                             CaptureArtifactPath&& artifactPath);
  void onDisplayFrame(DisplaySequence sequence, SessionEpoch epoch,
                      const std::uint16_t* pixels, std::size_t pixelCount);

  bool takeCompletion(CaptureCompletion* result);
  bool cancelAndWait(CaptureCompletion* result);
  void shutdown();

 private:
  enum class Stage {
    Disabled,
    Idle,
    Armed,
    SnapshotReady,
    Writing,
    Completed,
    Stopping,
  };

  struct PendingCapture {
    RequestId id = 0;
    SessionEpoch epoch = 0;
    DisplaySequence armedAfter = 0;
    DisplaySequence capturedSequence = 0;
    CaptureArtifactPath artifactPath;

    void clear();
  };

  void workerMain();
  void completeLocked(const CaptureOperationResult& result);
  bool takeCompletionLocked(CaptureCompletion* result);

  mutable std::mutex mutex;
  std::condition_variable condition;
  Stage stage = Stage::Disabled;
  std::filesystem::path canonicalRoot;
  std::uint16_t width = 0;
  std::uint16_t height = 0;
  std::uint8_t depth = 0;
  std::vector<std::uint16_t> snapshot;
  PendingCapture pending;
  CaptureCompletion completion;
  CaptureWriteFunction writeFunction = writeRgb565Ppm;
  std::atomic<int> commitState{0};
  std::thread worker;
};

void requestAutomationLcdInvalidation();
bool consumeAutomationLcdInvalidation();

}  // namespace automation
}  // namespace edgetx
