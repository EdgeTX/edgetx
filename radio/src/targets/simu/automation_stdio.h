/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <string>
#include <vector>

#include "automation_protocol.h"

namespace edgetx
{
namespace automation
{

constexpr std::size_t STDIO_READ_BUDGET = 4096;
constexpr std::size_t STDIO_RECORD_BUDGET = 8;

enum class StdioPumpResult {
  Continue,
  StopRequested,
  PeerClosed,
  Error,
};

struct AutomationInputHandlers {
  void (*setKey)(const std::string& key, bool pressed) = nullptr;
  void (*rotate)(std::int32_t steps) = nullptr;
  void (*touchDown)(std::uint16_t x, std::uint16_t y) = nullptr;
  void (*touchMove)(std::uint16_t x, std::uint16_t y) = nullptr;
  void (*touchUp)() = nullptr;
};

class AutomationStdio
{
 public:
  explicit AutomationStdio(
      const TargetDescription& target = TargetDescription());
  ~AutomationStdio();

  AutomationStdio(const AutomationStdio&) = delete;
  AutomationStdio& operator=(const AutomationStdio&) = delete;

  bool start(std::string* error);
  StdioPumpResult pump(std::string* error);
  void setTargetDescription(const TargetDescription& target);
  void setInputHandlers(const AutomationInputHandlers& handlers);
  void markRuntimeStarted();
  void markRuntimeStopped();
  void onDisplayFrame();

 private:
  enum class ReadResult {
    Data,
    WouldBlock,
    Closed,
    Error,
  };

  enum class WriteResult {
    Complete,
    Closed,
    Error,
  };

  ReadResult readInput(char* bytes, std::size_t capacity,
                       std::size_t* bytesRead, std::string* error);
  WriteResult writeOutput(const std::string& record, std::string* error);
  void queueEvents(std::vector<LineEvent>&& events);
  StdioPumpResult processEvent(const LineEvent& event, std::string* error);
  StdioPumpResult processKey(const Request& request, bool pressed,
                             std::string* error);
  StdioPumpResult processRotate(const Request& request, std::string* error);
  StdioPumpResult processTouch(const Request& request, std::string* error);
  StdioPumpResult processWaitFrame(const Request& request, std::string* error);
  StdioPumpResult processReleaseAll(const Request& request, std::string* error);
  StdioPumpResult processStop(const Request& request, std::string* error);
  StdioPumpResult drainCompletedResponses(std::string* error);
  StdioPumpResult emitResponse(const Response& response, std::string* error);
  StdioPumpResult emitEvent(ErrorCode code, const std::string& message,
                            std::string* error);
  StdioPumpResult writeSerialized(const std::string& record,
                                  std::string* error);
  SessionEpoch currentEpoch() const;
  Response makeStatusResponse(RequestId id) const;
  Response makeDescriptionResponse(RequestId id) const;
  bool supportsCommand(Command command) const;
  void releaseInputs();

  struct PendingFrameWait {
    RequestId id = 0;
    SessionEpoch epoch = 0;
    DisplaySequence minimum = 0;

    bool active() const { return id != 0; }
    void clear()
    {
      id = 0;
      epoch = 0;
      minimum = 0;
    }
  };

  struct CompletedFrameWait {
    RequestId id = 0;
    SessionEpoch epoch = 0;
    DisplaySequence sequence = 0;

    bool active() const { return id != 0; }
    void clear()
    {
      id = 0;
      epoch = 0;
      sequence = 0;
    }
  };

  LineBuffer lineBuffer;
  ProtocolParser parser;
  std::deque<LineEvent> pendingEvents;
  mutable std::mutex stateMutex;
  SessionState sessionState;
  TargetDescription targetDescription;
  AutomationInputHandlers inputHandlers;
  PendingFrameWait pendingFrameWait;
  CompletedFrameWait completedFrameWait;
  bool runtimeRunning = false;
  std::uint64_t lineOverflowCount = 0;
  std::uint64_t queueOverflowCount = 0;
  bool started = false;
  bool inputClosed = false;
  bool queueOverflowed = false;

#if defined(_WIN32)
  std::intptr_t inputHandle = 0;
  std::intptr_t outputHandle = 0;
  bool inputIsPipe = false;
#else
  int originalInputFlags = -1;
  int outputFd = -1;
  bool restoreInputFlags = false;
#endif
};

}  // namespace automation
}  // namespace edgetx
