/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
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

class AutomationStdio
{
 public:
  AutomationStdio() = default;
  ~AutomationStdio();

  AutomationStdio(const AutomationStdio&) = delete;
  AutomationStdio& operator=(const AutomationStdio&) = delete;

  bool start(std::string* error);
  StdioPumpResult pump(std::string* error);

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
  StdioPumpResult emitResponse(const Response& response, std::string* error);
  StdioPumpResult emitEvent(ErrorCode code, const std::string& message,
                            std::string* error);
  StdioPumpResult writeSerialized(const std::string& record,
                                  std::string* error);

  LineBuffer lineBuffer;
  ProtocolParser parser;
  std::deque<LineEvent> pendingEvents;
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
