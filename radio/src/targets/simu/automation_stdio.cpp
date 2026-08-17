/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "automation_stdio.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <utility>

#if defined(_WIN32)
#include <io.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>

#include <csignal>
#endif

namespace edgetx
{
namespace automation
{
namespace
{

constexpr SessionEpoch INITIAL_EPOCH = 0;

void setError(std::string* error, const std::string& message)
{
  if (error != nullptr) *error = message;
}

#if defined(_WIN32)

bool isClosedPipeError(DWORD error)
{
  return error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED ||
         error == ERROR_NO_DATA;
}

bool getHandleType(HANDLE handle, DWORD* type, std::string* error,
                   const char* streamName)
{
  if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
    setError(error,
             std::string("automation ") + streamName + " is not available");
    return false;
  }

  SetLastError(ERROR_SUCCESS);
  *type = GetFileType(handle);
  const DWORD typeError = GetLastError();
  if (*type == FILE_TYPE_UNKNOWN && typeError != ERROR_SUCCESS) {
    setError(error, std::string("cannot inspect automation ") + streamName +
                        " (Win32 error " + std::to_string(typeError) + ")");
    return false;
  }

  if (*type != FILE_TYPE_PIPE && *type != FILE_TYPE_DISK) {
    setError(error, std::string("automation ") + streamName +
                        " must be redirected to a pipe or file");
    return false;
  }
  return true;
}

#endif

}  // namespace

AutomationStdio::~AutomationStdio()
{
#if defined(_WIN32)
  if (outputHandle != 0) {
    (void)CloseHandle(reinterpret_cast<HANDLE>(outputHandle));
  }
#else
  if (outputFd != -1) (void)close(outputFd);
  if (restoreInputFlags) {
    (void)fcntl(STDIN_FILENO, F_SETFL, originalInputFlags);
  }
#endif
}

bool AutomationStdio::start(std::string* error)
{
  if (error != nullptr) error->clear();
  if (started) {
    setError(error, "automation stdio is already active");
    return false;
  }

#if defined(_WIN32)
  HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
  HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE diagnostics = GetStdHandle(STD_ERROR_HANDLE);
  DWORD inputType = FILE_TYPE_UNKNOWN;
  DWORD outputType = FILE_TYPE_UNKNOWN;
  if (!getHandleType(input, &inputType, error, "stdin") ||
      !getHandleType(output, &outputType, error, "stdout")) {
    return false;
  }
  if (diagnostics == nullptr || diagnostics == INVALID_HANDLE_VALUE) {
    setError(error, "automation stderr is not available");
    return false;
  }

  HANDLE protocolOutput = nullptr;
  // Keep a private binary handle for protocol records, then send every other
  // process-wide stdout writer (including firmware TRACE) to diagnostics.
  if (!DuplicateHandle(GetCurrentProcess(), output, GetCurrentProcess(),
                       &protocolOutput, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
    setError(error, "cannot preserve automation stdout (Win32 error " +
                        std::to_string(GetLastError()) + ")");
    return false;
  }

  const int stdoutFd = _fileno(stdout);
  const int stderrFd = _fileno(stderr);
  const int savedStdoutFd = stdoutFd == -1 ? -1 : _dup(stdoutFd);
  if (savedStdoutFd == -1 || stderrFd == -1) {
    if (savedStdoutFd != -1) (void)_close(savedStdoutFd);
    (void)CloseHandle(protocolOutput);
    setError(error, "cannot preserve the C stdout stream");
    return false;
  }

  (void)std::fflush(stdout);
  if (!SetStdHandle(STD_OUTPUT_HANDLE, diagnostics)) {
    const DWORD redirectError = GetLastError();
    (void)_close(savedStdoutFd);
    (void)CloseHandle(protocolOutput);
    setError(error, "cannot redirect automation diagnostics (Win32 error " +
                        std::to_string(redirectError) + ")");
    return false;
  }
  if (_dup2(stderrFd, stdoutFd) != 0) {
    (void)SetStdHandle(STD_OUTPUT_HANDLE, output);
    (void)_close(savedStdoutFd);
    (void)CloseHandle(protocolOutput);
    setError(error, "cannot redirect the C stdout stream");
    return false;
  }

  inputHandle = reinterpret_cast<std::intptr_t>(input);
  outputHandle = reinterpret_cast<std::intptr_t>(protocolOutput);
  inputIsPipe = inputType == FILE_TYPE_PIPE;
  (void)_close(savedStdoutFd);
#else
  if (isatty(STDIN_FILENO) != 0 || isatty(STDOUT_FILENO) != 0) {
    setError(error, "automation stdin and stdout must be redirected");
    return false;
  }

  originalInputFlags = fcntl(STDIN_FILENO, F_GETFL);
  if (originalInputFlags == -1 || fcntl(STDOUT_FILENO, F_GETFL) == -1 ||
      fcntl(STDERR_FILENO, F_GETFL) == -1) {
    setError(error, std::string("cannot inspect automation stdio: ") +
                        std::strerror(errno));
    return false;
  }

  if ((originalInputFlags & O_NONBLOCK) == 0) {
    if (fcntl(STDIN_FILENO, F_SETFL, originalInputFlags | O_NONBLOCK) == -1) {
      setError(error,
               std::string("cannot make automation stdin non-blocking: ") +
                   std::strerror(errno));
      return false;
    }
    restoreInputFlags = true;
  }

  if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
    if (restoreInputFlags) {
      (void)fcntl(STDIN_FILENO, F_SETFL, originalInputFlags);
      restoreInputFlags = false;
    }
    setError(error, "cannot ignore SIGPIPE for automation stdout");
    return false;
  }

  (void)std::fflush(stdout);
  // The duplicate remains the protocol channel after fd 1 becomes stderr.
  outputFd = dup(STDOUT_FILENO);
  if (outputFd == -1) {
    if (restoreInputFlags) {
      (void)fcntl(STDIN_FILENO, F_SETFL, originalInputFlags);
      restoreInputFlags = false;
    }
    setError(error, std::string("cannot preserve automation stdout: ") +
                        std::strerror(errno));
    return false;
  }
  if (dup2(STDERR_FILENO, STDOUT_FILENO) == -1) {
    const int redirectError = errno;
    (void)close(outputFd);
    outputFd = -1;
    if (restoreInputFlags) {
      (void)fcntl(STDIN_FILENO, F_SETFL, originalInputFlags);
      restoreInputFlags = false;
    }
    setError(error, std::string("cannot redirect automation diagnostics: ") +
                        std::strerror(redirectError));
    return false;
  }
#endif

  started = true;
  return true;
}

StdioPumpResult AutomationStdio::pump(std::string* error)
{
  if (error != nullptr) error->clear();
  if (!started) {
    setError(error, "automation stdio is not active");
    return StdioPumpResult::Error;
  }

  if (pendingEvents.empty() && !inputClosed) {
    char input[STDIO_READ_BUDGET];
    std::size_t bytesRead = 0;
    const ReadResult readResult =
        readInput(input, sizeof(input), &bytesRead, error);
    if (readResult == ReadResult::Data) {
      queueEvents(lineBuffer.feed(input, bytesRead));
    } else if (readResult == ReadResult::Closed) {
      inputClosed = true;
      // A partial record cannot receive a response after the peer closes stdin.
      (void)lineBuffer.finish();
    } else if (readResult == ReadResult::Error) {
      return StdioPumpResult::Error;
    }
  }

  std::size_t processed = 0;
  while (!pendingEvents.empty() && processed < STDIO_RECORD_BUDGET) {
    LineEvent event = std::move(pendingEvents.front());
    pendingEvents.pop_front();
    ++processed;

    const StdioPumpResult result = processEvent(event, error);
    if (result != StdioPumpResult::Continue) return result;
  }

  if (queueOverflowed) {
    queueOverflowed = false;
    const StdioPumpResult result =
        emitEvent(ErrorCode::QueueFull, "input queue capacity exceeded", error);
    if (result != StdioPumpResult::Continue) return result;
  }

  if (inputClosed && pendingEvents.empty()) return StdioPumpResult::PeerClosed;
  return StdioPumpResult::Continue;
}

AutomationStdio::ReadResult AutomationStdio::readInput(char* bytes,
                                                       std::size_t capacity,
                                                       std::size_t* bytesRead,
                                                       std::string* error)
{
  *bytesRead = 0;
#if defined(_WIN32)
  HANDLE input = reinterpret_cast<HANDLE>(inputHandle);
  DWORD requested = static_cast<DWORD>(capacity);
  if (inputIsPipe) {
    DWORD available = 0;
    if (!PeekNamedPipe(input, nullptr, 0, nullptr, &available, nullptr)) {
      const DWORD pipeError = GetLastError();
      if (isClosedPipeError(pipeError)) return ReadResult::Closed;
      setError(error, "cannot inspect automation stdin (Win32 error " +
                          std::to_string(pipeError) + ")");
      return ReadResult::Error;
    }
    if (available == 0) return ReadResult::WouldBlock;
    requested = requested < available ? requested : available;
  }

  DWORD received = 0;
  if (!ReadFile(input, bytes, requested, &received, nullptr)) {
    const DWORD readError = GetLastError();
    if (isClosedPipeError(readError)) return ReadResult::Closed;
    setError(error, "cannot read automation stdin (Win32 error " +
                        std::to_string(readError) + ")");
    return ReadResult::Error;
  }
  if (received == 0) return ReadResult::Closed;
  *bytesRead = received;
  return ReadResult::Data;
#else
  const ssize_t received = read(STDIN_FILENO, bytes, capacity);
  if (received > 0) {
    *bytesRead = static_cast<std::size_t>(received);
    return ReadResult::Data;
  }
  if (received == 0) return ReadResult::Closed;
  if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
    return ReadResult::WouldBlock;

  setError(error, std::string("cannot read automation stdin: ") +
                      std::strerror(errno));
  return ReadResult::Error;
#endif
}

AutomationStdio::WriteResult AutomationStdio::writeOutput(
    const std::string& record, std::string* error)
{
  std::size_t offset = 0;
  while (offset < record.size()) {
#if defined(_WIN32)
    HANDLE output = reinterpret_cast<HANDLE>(outputHandle);
    const DWORD remaining = static_cast<DWORD>(record.size() - offset);
    DWORD written = 0;
    if (!WriteFile(output, record.data() + offset, remaining, &written,
                   nullptr)) {
      const DWORD writeError = GetLastError();
      if (isClosedPipeError(writeError)) return WriteResult::Closed;
      setError(error, "cannot write automation stdout (Win32 error " +
                          std::to_string(writeError) + ")");
      return WriteResult::Error;
    }
    if (written == 0) return WriteResult::Closed;
    offset += written;
#else
    const ssize_t written =
        write(outputFd, record.data() + offset, record.size() - offset);
    if (written > 0) {
      offset += static_cast<std::size_t>(written);
      continue;
    }
    if (written < 0 && errno == EINTR) continue;
    if (written < 0 && errno == EPIPE) return WriteResult::Closed;

    setError(error, std::string("cannot write automation stdout: ") +
                        std::strerror(errno));
    return WriteResult::Error;
#endif
  }
  return WriteResult::Complete;
}

void AutomationStdio::queueEvents(std::vector<LineEvent>&& events)
{
  for (LineEvent& event : events) {
    if (event.type == LineEventType::Record && event.record.empty()) continue;
    if (pendingEvents.size() == MAX_PENDING_REQUESTS) {
      queueOverflowed = true;
      continue;
    }
    pendingEvents.push_back(std::move(event));
  }
}

StdioPumpResult AutomationStdio::processEvent(const LineEvent& event,
                                              std::string* error)
{
  if (event.type == LineEventType::LineTooLong) {
    return emitEvent(ErrorCode::LineTooLong, "record exceeds 16 KiB", error);
  }
  if (event.type == LineEventType::PartialRecordAtEof)
    return StdioPumpResult::Continue;

  const ParseResult parsed = parser.parse(event.record);
  if (parsed.status == ParseStatus::Ignored) return StdioPumpResult::Continue;
  if (parsed.status == ParseStatus::Error) {
    if (parsed.error.hasRequestId) {
      return emitResponse(
          Response::failure(parsed.error.requestId, INITIAL_EPOCH,
                            parsed.error.code, parsed.error.message),
          error);
    }
    return emitEvent(parsed.error.code, parsed.error.message, error);
  }

  if (parsed.request.command == Command::Ping) {
    return emitResponse(Response::success(parsed.request.id, INITIAL_EPOCH),
                        error);
  }
  if (parsed.request.command == Command::Stop) {
    const StdioPumpResult writeResult = emitResponse(
        Response::success(parsed.request.id, INITIAL_EPOCH), error);
    return writeResult == StdioPumpResult::Continue
               ? StdioPumpResult::StopRequested
               : writeResult;
  }

  return emitResponse(
      Response::failure(parsed.request.id, INITIAL_EPOCH,
                        ErrorCode::UnsupportedCommand,
                        std::string("command is not implemented yet: ") +
                            commandName(parsed.request.command)),
      error);
}

StdioPumpResult AutomationStdio::emitResponse(const Response& response,
                                              std::string* error)
{
  std::string record;
  if (serializeResponse(response, &record) == SerializeResult::LimitTooSmall) {
    setError(error, "cannot serialize automation response");
    return StdioPumpResult::Error;
  }
  return writeSerialized(record, error);
}

StdioPumpResult AutomationStdio::emitEvent(ErrorCode code,
                                           const std::string& message,
                                           std::string* error)
{
  std::string record;
  if (serializeEvent(INITIAL_EPOCH, code, message, &record) ==
      SerializeResult::LimitTooSmall) {
    setError(error, "cannot serialize automation event");
    return StdioPumpResult::Error;
  }
  return writeSerialized(record, error);
}

StdioPumpResult AutomationStdio::writeSerialized(const std::string& record,
                                                 std::string* error)
{
  const WriteResult result = writeOutput(record, error);
  if (result == WriteResult::Closed) return StdioPumpResult::PeerClosed;
  if (result == WriteResult::Error) return StdioPumpResult::Error;
  return StdioPumpResult::Continue;
}

}  // namespace automation
}  // namespace edgetx
