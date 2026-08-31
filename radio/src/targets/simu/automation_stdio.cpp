/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "automation_stdio.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <exception>
#include <limits>
#include <utility>

#include "automation_runtime.h"

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

void setError(std::string* error, const std::string& message)
{
  if (error != nullptr) *error = message;
}

void incrementSaturating(std::uint64_t* counter)
{
  if (*counter != (std::numeric_limits<std::uint64_t>::max)()) ++*counter;
}

std::uint64_t parseValidatedUnsigned(const std::string& value)
{
  std::uint64_t parsed = 0;
  for (const char byte : value)
    parsed = parsed * 10 + static_cast<std::uint64_t>(byte - '0');
  return parsed;
}

std::int32_t parseValidatedSigned(const std::string& value)
{
  const bool negative = value[0] == '-';
  const std::size_t offset = negative ? 1 : 0;
  const std::uint64_t magnitude = parseValidatedUnsigned(value.substr(offset));
  return negative ? -static_cast<std::int32_t>(magnitude)
                  : static_cast<std::int32_t>(magnitude);
}

Response captureResponse(const CaptureCompletion& completion)
{
  if (completion.ok) {
    return Response::successWithCapture(completion.id, completion.epoch,
                                        completion.artifact);
  }
  return Response::failure(completion.id, completion.epoch,
                           completion.errorCode, completion.message);
}

std::string defaultTelemetryName(std::uint16_t id)
{
  static constexpr char HEX[] = "0123456789ABCDEF";
  std::string name(MAX_TELEMETRY_LABEL_BYTES, '0');
  for (std::size_t index = 0; index < name.size(); ++index) {
    const std::size_t shift = (name.size() - index - 1) * 4;
    name[index] = HEX[(id >> shift) & 0x0f];
  }
  return name;
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

AutomationStdio::AutomationStdio(const TargetDescription& target) :
    targetDescription(target)
{
}

AutomationStdio::~AutomationStdio()
{
  capture.shutdown();
  stopOutputWriter();
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

void AutomationStdio::setTargetDescription(const TargetDescription& target)
{
  std::lock_guard<std::mutex> lock(stateMutex);
  targetDescription = target;
}

void AutomationStdio::setInputHandlers(const AutomationInputHandlers& handlers)
{
  std::lock_guard<std::mutex> lock(stateMutex);
  inputHandlers = handlers;
}

bool AutomationStdio::configureCapture(const std::string& outputRoot,
                                       std::uint16_t width,
                                       std::uint16_t height, std::uint8_t depth,
                                       std::string* error)
{
  return capture.configure(outputRoot, width, height, depth, error);
}

bool AutomationStdio::captureConfigured() const { return capture.configured(); }

void AutomationStdio::markRuntimeStarted()
{
  simuAutomationRuntimeStart();
  std::lock_guard<std::mutex> lock(stateMutex);
  runtimeRunning = true;
}

bool AutomationStdio::prepareRuntimeRestart(std::string* error)
{
  simuAutomationRuntimeResetAfterTaskJoin();
  AutomationInputHandlers handlers;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    runtimeRunning = false;
    if (!pendingRestart.active()) {
      setError(error, "no warm restart is pending");
      return false;
    }
    const TransitionResult transition = sessionState.restartTasksStarted(
        pendingRestart.id, pendingRestart.epoch);
    if (transition != TransitionResult::Applied) {
      setError(error, "cannot arm the restarted runtime epoch");
      return false;
    }
    handlers = inputHandlers;
  }
  if (handlers.clearAllAnalogs != nullptr) handlers.clearAllAnalogs();
  if (handlers.resetSwitches != nullptr) handlers.resetSwitches();
  return true;
}

void AutomationStdio::markRuntimeRestarted()
{
  std::lock_guard<std::mutex> lock(stateMutex);
  runtimeRunning = true;
}

void AutomationStdio::markRuntimeStopped()
{
  capture.shutdown();
  simuAutomationRuntimeStop();
  simuAutomationRuntimeResetAfterTaskJoin();
  std::lock_guard<std::mutex> lock(stateMutex);
  runtimeRunning = false;
  sessionState.stop();
}

void AutomationStdio::onDisplayFrame(const std::uint16_t* pixels,
                                     std::size_t pixelCount)
{
  DisplaySequence sequence = 0;
  SessionEpoch epoch = 0;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    const RequestId restarted = sessionState.onDisplayFrame();
    sequence = sessionState.displaySequence();
    epoch = sessionState.epoch();
    if (restarted != 0 && pendingRestart.active() &&
        restarted == pendingRestart.id) {
      completedRestart.id = restarted;
      completedRestart.epoch = epoch;
      completedRestart.sequence = sequence;
      pendingRestart.clear();
    }
    if (pendingFrameWait.active() && sequence >= pendingFrameWait.minimum) {
      const TransitionResult completion = sessionState.completeAsync(
          pendingFrameWait.id, pendingFrameWait.epoch);
      if (completion == TransitionResult::Applied) {
        completedFrameWait.id = pendingFrameWait.id;
        completedFrameWait.epoch = pendingFrameWait.epoch;
        completedFrameWait.sequence = sequence;
      }
      pendingFrameWait.clear();
    }
  }
  capture.onDisplayFrame(sequence, epoch, pixels, pixelCount);
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

  const int outputFlags = fcntl(outputFd, F_GETFL);
  if (outputFlags == -1 ||
      fcntl(outputFd, F_SETFL, outputFlags | O_NONBLOCK) == -1) {
    const int flagError = errno;
    (void)close(outputFd);
    outputFd = -1;
    if (restoreInputFlags) {
      (void)fcntl(STDIN_FILENO, F_SETFL, originalInputFlags);
      restoreInputFlags = false;
    }
    setError(error,
             std::string("cannot make automation stdout non-blocking: ") +
                 std::strerror(flagError));
    return false;
  }
#endif

  if (!startOutputWriter(error)) return false;
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

  const StdioPumpResult writerResult = checkOutputWriter(error);
  if (writerResult != StdioPumpResult::Continue) return writerResult;
  if (stopAfterFlush) {
    return outputFlushed() ? StdioPumpResult::StopRequested
                           : StdioPumpResult::Continue;
  }
  if (outputBackpressured()) return StdioPumpResult::Continue;

  const StdioPumpResult completedResult = drainCompletedResponses(error);
  if (completedResult != StdioPumpResult::Continue) return completedResult;

  if (outputBackpressured()) return StdioPumpResult::Continue;

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
  while (!pendingEvents.empty() && processed < STDIO_RECORD_BUDGET &&
         !outputBackpressured()) {
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
    {
      std::lock_guard<std::mutex> lock(outputMutex);
      if (outputStop) return WriteResult::Closed;
    }
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
    if (written < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
      std::unique_lock<std::mutex> lock(outputMutex);
      if (outputStop) return WriteResult::Closed;
      outputReady.wait_for(lock, std::chrono::milliseconds(1),
                           [this]() { return outputStop; });
      if (outputStop) return WriteResult::Closed;
      continue;
    }

    setError(error, std::string("cannot write automation stdout: ") +
                        std::strerror(errno));
    return WriteResult::Error;
#endif
  }
  return WriteResult::Complete;
}

bool AutomationStdio::startOutputWriter(std::string* error)
{
  try {
    outputThread = std::thread(&AutomationStdio::outputWriterLoop, this);
  } catch (const std::exception& exception) {
    setError(error, std::string("cannot start automation stdout writer: ") +
                        exception.what());
    return false;
  }
  return true;
}

void AutomationStdio::stopOutputWriter()
{
  {
    std::lock_guard<std::mutex> lock(outputMutex);
    outputStop = true;
    outputQueue.clear();
  }
  outputReady.notify_all();
#if defined(_WIN32)
  if (outputThread.joinable()) {
    (void)CancelSynchronousIo(
        reinterpret_cast<HANDLE>(outputThread.native_handle()));
  }
#endif
  if (outputThread.joinable()) outputThread.join();
}

void AutomationStdio::outputWriterLoop()
{
  while (true) {
    std::string record;
    {
      std::unique_lock<std::mutex> lock(outputMutex);
      outputReady.wait(lock,
                       [this]() { return outputStop || !outputQueue.empty(); });
      if (outputStop) return;
      record = outputQueue.front();
    }

    std::string error;
    const WriteResult result = writeOutput(record, &error);
    {
      std::lock_guard<std::mutex> lock(outputMutex);
      if (outputStop) return;
      if (result != WriteResult::Complete) {
        outputResult = result;
        outputError = std::move(error);
        outputQueue.clear();
        return;
      }
      outputQueue.pop_front();
    }
  }
}

StdioPumpResult AutomationStdio::checkOutputWriter(std::string* error) const
{
  std::lock_guard<std::mutex> lock(outputMutex);
  if (outputResult == WriteResult::Closed) return StdioPumpResult::PeerClosed;
  if (outputResult == WriteResult::Error) {
    setError(error, outputError.empty() ? "automation stdout writer failed"
                                        : outputError);
    return StdioPumpResult::Error;
  }
  return StdioPumpResult::Continue;
}

bool AutomationStdio::outputBackpressured() const
{
  std::lock_guard<std::mutex> lock(outputMutex);
  return outputQueue.size() >= STDIO_OUTPUT_HIGH_WATERMARK;
}

bool AutomationStdio::outputFlushed() const
{
  std::lock_guard<std::mutex> lock(outputMutex);
  return outputQueue.empty();
}

void AutomationStdio::queueEvents(std::vector<LineEvent>&& events)
{
  for (LineEvent& event : events) {
    if (event.type == LineEventType::Record && event.record.empty()) continue;
    if (pendingEvents.size() == MAX_PENDING_REQUESTS) {
      queueOverflowed = true;
      std::lock_guard<std::mutex> lock(stateMutex);
      incrementSaturating(&queueOverflowCount);
      continue;
    }
    pendingEvents.push_back(std::move(event));
  }
}

StdioPumpResult AutomationStdio::processEvent(const LineEvent& event,
                                              std::string* error)
{
  if (event.type == LineEventType::LineTooLong) {
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      incrementSaturating(&lineOverflowCount);
    }
    return emitEvent(ErrorCode::LineTooLong, "record exceeds 16 KiB", error);
  }
  if (event.type == LineEventType::PartialRecordAtEof)
    return StdioPumpResult::Continue;

  const ParseResult parsed = parser.parse(event.record);
  if (parsed.status == ParseStatus::Ignored) return StdioPumpResult::Continue;
  if (parsed.status == ParseStatus::Error) {
    if (parsed.error.hasRequestId) {
      return emitResponse(
          Response::failure(parsed.error.requestId, currentEpoch(),
                            parsed.error.code, parsed.error.message),
          error);
    }
    return emitEvent(parsed.error.code, parsed.error.message, error);
  }

  if (!supportsCommand(parsed.request.command)) {
    return emitResponse(
        Response::failure(
            parsed.request.id, currentEpoch(), ErrorCode::UnsupportedCommand,
            std::string("command is not supported by this target: ") +
                commandName(parsed.request.command)),
        error);
  }

  if (parsed.request.command == Command::Ping) {
    return emitResponse(Response::success(parsed.request.id, currentEpoch()),
                        error);
  }
  if (parsed.request.command == Command::Status) {
    return emitResponse(makeStatusResponse(parsed.request.id), error);
  }
  if (parsed.request.command == Command::Describe) {
    return emitResponse(makeDescriptionResponse(parsed.request.id), error);
  }
  if (parsed.request.command == Command::KeyDown) {
    return processKey(parsed.request, true, error);
  }
  if (parsed.request.command == Command::KeyUp) {
    return processKey(parsed.request, false, error);
  }
  if (parsed.request.command == Command::Rotate) {
    return processRotate(parsed.request, error);
  }
  if (parsed.request.command == Command::TouchDown ||
      parsed.request.command == Command::TouchMove ||
      parsed.request.command == Command::TouchUp) {
    return processTouch(parsed.request, error);
  }
  if (parsed.request.command == Command::SetSwitch) {
    return processSetSwitch(parsed.request, error);
  }
  if (parsed.request.command == Command::SetAnalog) {
    return processSetAnalog(parsed.request, error);
  }
  if (parsed.request.command == Command::ClearAnalog) {
    return processClearAnalog(parsed.request, error);
  }
  if (parsed.request.command == Command::SetTelemetry) {
    return processSetTelemetry(parsed.request, error);
  }
  if (parsed.request.command == Command::ReloadLua) {
    return processReloadLua(parsed.request, error);
  }
  if (parsed.request.command == Command::WaitFrame) {
    return processWaitFrame(parsed.request, error);
  }
  if (parsed.request.command == Command::Capture) {
    return processCapture(parsed.request, error);
  }
  if (parsed.request.command == Command::Restart) {
    return processRestart(parsed.request, error);
  }
  if (parsed.request.command == Command::ReleaseAll) {
    return processReleaseAll(parsed.request, error);
  }
  if (parsed.request.command == Command::Stop) {
    return processStop(parsed.request, error);
  }

  return emitResponse(
      Response::failure(parsed.request.id, currentEpoch(),
                        ErrorCode::UnsupportedCommand,
                        std::string("command is not implemented yet: ") +
                            commandName(parsed.request.command)),
      error);
}

StdioPumpResult AutomationStdio::processKey(const Request& request,
                                            bool pressed, std::string* error)
{
  Response response;
  void (*handler)(const std::string&, bool) = nullptr;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    const SessionEpoch epoch = sessionState.epoch();
    const std::string& key = request.arguments[0];
    if (std::find(targetDescription.keys.begin(), targetDescription.keys.end(),
                  key) == targetDescription.keys.end()) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedTarget,
                            "key is not supported by this target: " + key);
    } else if (inputHandlers.setKey == nullptr) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedCommand,
                            "key input is not available");
    } else {
      const TransitionResult transition =
          pressed ? sessionState.keyDown(key) : sessionState.keyUp(key);
      if (transition == TransitionResult::Applied) {
        response = Response::success(request.id, epoch);
        handler = inputHandlers.setKey;
      } else if (transition == TransitionResult::Busy) {
        response =
            Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                              "an asynchronous operation is active");
      } else if (transition == TransitionResult::Duplicate) {
        response =
            Response::failure(request.id, epoch, ErrorCode::KeyAlreadyDown,
                              "key is already down: " + key);
      } else if (sessionState.phase() != SessionPhase::Ready) {
        response =
            Response::failure(request.id, epoch,
                              sessionState.phase() == SessionPhase::Stopped
                                  ? ErrorCode::SessionStopping
                                  : ErrorCode::OperationBusy,
                              "session is not ready for input");
      } else {
        response = Response::failure(request.id, epoch, ErrorCode::KeyNotDown,
                                     "key is not down: " + key);
      }
    }
  }

  if (handler != nullptr) handler(request.arguments[0], pressed);
  return emitResponse(response, error);
}

StdioPumpResult AutomationStdio::processRotate(const Request& request,
                                               std::string* error)
{
  Response response;
  void (*handler)(std::int32_t) = nullptr;
  const std::int32_t steps = parseValidatedSigned(request.arguments[0]);
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    const SessionEpoch epoch = sessionState.epoch();
    if (inputHandlers.rotate == nullptr ||
        !targetDescription.capabilities.rotary) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedCommand,
                            "rotary input is not available");
    } else if (sessionState.phase() != SessionPhase::Ready) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for input");
    } else if (sessionState.asyncOperation() != AsyncOperation::None) {
      response = Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                                   "an asynchronous operation is active");
    } else {
      response = Response::success(request.id, epoch);
      handler = inputHandlers.rotate;
    }
  }

  if (handler != nullptr) handler(steps);
  return emitResponse(response, error);
}

StdioPumpResult AutomationStdio::processTouch(const Request& request,
                                              std::string* error)
{
  const bool hasCoordinates = request.command != Command::TouchUp;
  const std::uint16_t x =
      hasCoordinates ? static_cast<std::uint16_t>(
                           parseValidatedUnsigned(request.arguments[0]))
                     : 0;
  const std::uint16_t y =
      hasCoordinates ? static_cast<std::uint16_t>(
                           parseValidatedUnsigned(request.arguments[1]))
                     : 0;
  Response response;
  void (*positionHandler)(std::uint16_t, std::uint16_t) = nullptr;
  void (*releaseHandler)() = nullptr;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    const SessionEpoch epoch = sessionState.epoch();
    const bool callbacksReady = inputHandlers.touchDown != nullptr &&
                                inputHandlers.touchMove != nullptr &&
                                inputHandlers.touchUp != nullptr;
    if (!targetDescription.capabilities.touch || !callbacksReady) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedCommand,
                            "touch input is not available");
    } else if (hasCoordinates && (x >= targetDescription.lcdWidth ||
                                  y >= targetDescription.lcdHeight)) {
      response = Response::failure(request.id, epoch, ErrorCode::OutOfRange,
                                   "touch coordinate is outside the LCD");
    } else {
      TransitionResult transition = TransitionResult::InvalidState;
      if (request.command == Command::TouchDown) {
        transition = sessionState.touchDown(x, y);
      } else if (request.command == Command::TouchMove) {
        transition = sessionState.touchMove(x, y);
      } else {
        transition = sessionState.touchUp();
      }

      if (transition == TransitionResult::Applied) {
        response = Response::success(request.id, epoch);
        if (request.command == Command::TouchDown)
          positionHandler = inputHandlers.touchDown;
        else if (request.command == Command::TouchMove)
          positionHandler = inputHandlers.touchMove;
        else
          releaseHandler = inputHandlers.touchUp;
      } else if (transition == TransitionResult::Busy) {
        response =
            Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                              "an asynchronous operation is active");
      } else if (transition == TransitionResult::Duplicate) {
        response =
            Response::failure(request.id, epoch, ErrorCode::TouchAlreadyDown,
                              "touch is already down");
      } else if (sessionState.phase() != SessionPhase::Ready) {
        response =
            Response::failure(request.id, epoch,
                              sessionState.phase() == SessionPhase::Stopped
                                  ? ErrorCode::SessionStopping
                                  : ErrorCode::OperationBusy,
                              "session is not ready for input");
      } else {
        response = Response::failure(request.id, epoch, ErrorCode::TouchNotDown,
                                     "touch is not down");
      }
    }
  }

  if (positionHandler != nullptr) positionHandler(x, y);
  if (releaseHandler != nullptr) releaseHandler();
  return emitResponse(response, error);
}

StdioPumpResult AutomationStdio::processSetSwitch(const Request& request,
                                                  std::string* error)
{
  const std::int32_t position = parseValidatedSigned(request.arguments[1]);
  Response response;
  bool admitted = false;
  bool (*handler)(const std::string&, std::int8_t) = nullptr;
  SessionEpoch epoch = 0;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    epoch = sessionState.epoch();
    const auto target = std::find_if(targetDescription.switches.begin(),
                                     targetDescription.switches.end(),
                                     [&request](const NamedRange& item) {
                                       return item.name == request.arguments[0];
                                     });
    if (target == targetDescription.switches.end()) {
      response = Response::failure(
          request.id, epoch, ErrorCode::UnsupportedTarget,
          "switch is not supported by this target: " + request.arguments[0]);
    } else if (position < -1 || position > 1) {
      response = Response::failure(request.id, epoch, ErrorCode::OutOfRange,
                                   "switch position must be -1, 0, or 1");
    } else if (inputHandlers.setSwitch == nullptr) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedCommand,
                            "switch input is not available");
    } else if (sessionState.phase() != SessionPhase::Ready ||
               sessionState.asyncOperation() != AsyncOperation::None) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for switch input");
    } else {
      handler = inputHandlers.setSwitch;
      admitted = true;
    }
  }

  if (admitted) {
    if (handler(request.arguments[0], static_cast<std::int8_t>(position))) {
      response = Response::success(request.id, epoch);
    } else {
      response = Response::failure(
          request.id, epoch, ErrorCode::OutOfRange,
          "switch position is not supported by this switch type");
    }
  }
  return emitResponse(response, error);
}

StdioPumpResult AutomationStdio::processSetAnalog(const Request& request,
                                                  std::string* error)
{
  const std::uint16_t value =
      static_cast<std::uint16_t>(parseValidatedUnsigned(request.arguments[1]));
  Response response;
  bool admitted = false;
  bool (*handler)(const std::string&, std::uint16_t) = nullptr;
  SessionEpoch epoch = 0;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    epoch = sessionState.epoch();
    const auto target = std::find_if(targetDescription.analogs.begin(),
                                     targetDescription.analogs.end(),
                                     [&request](const NamedRange& item) {
                                       return item.name == request.arguments[0];
                                     });
    if (target == targetDescription.analogs.end()) {
      response = Response::failure(
          request.id, epoch, ErrorCode::UnsupportedTarget,
          "analog is not supported by this target: " + request.arguments[0]);
    } else if (inputHandlers.setAnalog == nullptr) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedCommand,
                            "analog override is not available");
    } else if (sessionState.phase() != SessionPhase::Ready ||
               sessionState.asyncOperation() != AsyncOperation::None) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for analog input");
    } else {
      handler = inputHandlers.setAnalog;
      admitted = true;
    }
  }

  if (admitted) {
    response =
        handler(request.arguments[0], value)
            ? Response::success(request.id, epoch)
            : Response::failure(request.id, epoch, ErrorCode::UnsupportedTarget,
                                "cannot resolve the analog input");
  }
  return emitResponse(response, error);
}

StdioPumpResult AutomationStdio::processClearAnalog(const Request& request,
                                                    std::string* error)
{
  Response response;
  bool admitted = false;
  bool clearAll = request.arguments[0] == "all";
  bool (*handler)(const std::string&) = nullptr;
  void (*allHandler)() = nullptr;
  SessionEpoch epoch = 0;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    epoch = sessionState.epoch();
    const auto target = std::find_if(targetDescription.analogs.begin(),
                                     targetDescription.analogs.end(),
                                     [&request](const NamedRange& item) {
                                       return item.name == request.arguments[0];
                                     });
    if (!clearAll && target == targetDescription.analogs.end()) {
      response = Response::failure(
          request.id, epoch, ErrorCode::UnsupportedTarget,
          "analog is not supported by this target: " + request.arguments[0]);
    } else if ((clearAll && inputHandlers.clearAllAnalogs == nullptr) ||
               (!clearAll && inputHandlers.clearAnalog == nullptr)) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedCommand,
                            "analog override is not available");
    } else if (sessionState.phase() != SessionPhase::Ready ||
               sessionState.asyncOperation() != AsyncOperation::None) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for analog input");
    } else {
      handler = inputHandlers.clearAnalog;
      allHandler = inputHandlers.clearAllAnalogs;
      admitted = true;
    }
  }

  if (admitted) {
    if (clearAll) {
      allHandler();
      response = Response::success(request.id, epoch);
    } else {
      response = handler(request.arguments[0])
                     ? Response::success(request.id, epoch)
                     : Response::failure(request.id, epoch,
                                         ErrorCode::UnsupportedTarget,
                                         "cannot resolve the analog input");
    }
  }
  return emitResponse(response, error);
}

StdioPumpResult AutomationStdio::processSetTelemetry(const Request& request,
                                                     std::string* error)
{
  static_assert(MAX_TELEMETRY_LABEL_BYTES == TELEM_LABEL_LEN,
                "protocol and model telemetry labels must match");

  FirmwareRequest firmwareRequest;
  firmwareRequest.operation = FirmwareOperation::Telemetry;
  firmwareRequest.id = request.id;
  firmwareRequest.telemetryId =
      static_cast<std::uint16_t>(parseValidatedUnsigned(request.arguments[0]));
  firmwareRequest.telemetrySubId =
      static_cast<std::uint8_t>(parseValidatedUnsigned(request.arguments[1]));
  firmwareRequest.telemetryInstance =
      static_cast<std::uint8_t>(parseValidatedUnsigned(request.arguments[2]));
  firmwareRequest.telemetryValue = parseValidatedSigned(request.arguments[3]);
  firmwareRequest.telemetryUnit =
      static_cast<std::uint8_t>(parseValidatedUnsigned(request.arguments[4]));
  firmwareRequest.telemetryPrecision =
      static_cast<std::uint8_t>(parseValidatedUnsigned(request.arguments[5]));
  const std::string telemetryName =
      request.arguments.size() == 7
          ? request.arguments[6]
          : defaultTelemetryName(firmwareRequest.telemetryId);
  std::memcpy(firmwareRequest.telemetryName, telemetryName.data(),
              telemetryName.size());

  Response response;
  SessionEpoch epoch = 0;
  bool reserved = false;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    epoch = sessionState.epoch();
    firmwareRequest.epoch = epoch;
    if (firmwareRequest.telemetryUnit > UNIT_MAX) {
      response = Response::failure(request.id, epoch, ErrorCode::OutOfRange,
                                   "telemetry unit is not supported");
    } else if (!targetDescription.capabilities.telemetry) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedCommand,
                            "telemetry injection is not available");
    } else if (sessionState.phase() != SessionPhase::Ready) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for firmware work");
    } else if (sessionState.beginAsync(AsyncOperation::Firmware, request.id) !=
               TransitionResult::Applied) {
      response = Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                                   "an asynchronous operation is active");
    } else {
      pendingFirmware.id = request.id;
      pendingFirmware.epoch = epoch;
      pendingFirmware.operation = AsyncOperation::Firmware;
      reserved = true;
    }
  }

  if (!reserved) return emitResponse(response, error);
  if (simuAutomationPostFirmwareRequest(firmwareRequest))
    return StdioPumpResult::Continue;

  {
    std::lock_guard<std::mutex> lock(stateMutex);
    (void)sessionState.cancelAsync();
    pendingFirmware.clear();
  }
  return emitResponse(
      Response::failure(request.id, epoch, ErrorCode::FirmwareQueueFull,
                        "firmware request mailbox is full"),
      error);
}

StdioPumpResult AutomationStdio::processReloadLua(const Request& request,
                                                  std::string* error)
{
  FirmwareRequest firmwareRequest;
  firmwareRequest.operation = FirmwareOperation::ReloadLua;
  firmwareRequest.id = request.id;

  Response response;
  SessionEpoch epoch = 0;
  bool reserved = false;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    epoch = sessionState.epoch();
    firmwareRequest.epoch = epoch;
    if (!targetDescription.capabilities.lua) {
      response = Response::failure(request.id, epoch, ErrorCode::LuaUnavailable,
                                   "Lua is not available on this target");
    } else if (nextLuaGeneration == 0) {
      response =
          Response::failure(request.id, epoch, ErrorCode::InvariantViolation,
                            "Lua reload generation space is exhausted");
    } else if (sessionState.phase() != SessionPhase::Ready) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for Lua reload");
    } else if (sessionState.beginAsync(AsyncOperation::ReloadLua, request.id) !=
               TransitionResult::Applied) {
      response = Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                                   "an asynchronous operation is active");
    } else {
      firmwareRequest.generation = nextLuaGeneration++;
      pendingFirmware.id = request.id;
      pendingFirmware.epoch = epoch;
      pendingFirmware.operation = AsyncOperation::ReloadLua;
      pendingFirmware.generation = firmwareRequest.generation;
      reserved = true;
    }
  }

  if (!reserved) return emitResponse(response, error);
  if (simuAutomationPostFirmwareRequest(firmwareRequest))
    return StdioPumpResult::Continue;

  {
    std::lock_guard<std::mutex> lock(stateMutex);
    (void)sessionState.cancelAsync();
    pendingFirmware.clear();
  }
  return emitResponse(
      Response::failure(request.id, epoch, ErrorCode::FirmwareQueueFull,
                        "firmware request mailbox is full"),
      error);
}

StdioPumpResult AutomationStdio::processWaitFrame(const Request& request,
                                                  std::string* error)
{
  const DisplaySequence minimum = parseValidatedUnsigned(request.arguments[0]);
  Response response;
  bool immediate = false;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    const SessionEpoch epoch = sessionState.epoch();
    if (sessionState.phase() != SessionPhase::Ready) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for a frame barrier");
      immediate = true;
    } else if (sessionState.asyncOperation() != AsyncOperation::None ||
               pendingFrameWait.active() || completedFrameWait.active()) {
      response = Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                                   "an asynchronous operation is active");
      immediate = true;
    } else if (sessionState.displaySequence() >= minimum) {
      response = Response::successWithFrame(request.id, epoch,
                                            sessionState.displaySequence());
      immediate = true;
    } else if (sessionState.beginAsync(AsyncOperation::WaitFrame, request.id) !=
               TransitionResult::Applied) {
      response =
          Response::failure(request.id, epoch, ErrorCode::InvariantViolation,
                            "cannot arm the frame barrier");
      immediate = true;
    } else {
      pendingFrameWait.id = request.id;
      pendingFrameWait.epoch = epoch;
      pendingFrameWait.minimum = minimum;
    }
  }

  return immediate ? emitResponse(response, error) : StdioPumpResult::Continue;
}

StdioPumpResult AutomationStdio::processCapture(const Request& request,
                                                std::string* error)
{
  CaptureArtifactPath artifactPath;
  const SessionEpoch validationEpoch = currentEpoch();
  const CaptureOperationResult validation = capture.validatePath(
      request.arguments[0], request.id, validationEpoch, &artifactPath);
  if (!validation.ok) {
    return emitResponse(
        Response::failure(request.id, validationEpoch, validation.errorCode,
                          validation.message),
        error);
  }

  Response response;
  SessionEpoch epoch = 0;
  DisplaySequence armedAfter = 0;
  bool reserved = false;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    epoch = sessionState.epoch();
    armedAfter = sessionState.displaySequence();
    if (sessionState.phase() != SessionPhase::Ready) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for capture");
    } else if (sessionState.asyncOperation() != AsyncOperation::None ||
               pendingFrameWait.active() || completedFrameWait.active()) {
      response = Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                                   "an asynchronous operation is active");
    } else if (sessionState.beginAsync(AsyncOperation::Capture, request.id) !=
               TransitionResult::Applied) {
      response =
          Response::failure(request.id, epoch, ErrorCode::InvariantViolation,
                            "cannot reserve the capture operation");
    } else {
      reserved = true;
    }
  }

  if (!reserved) return emitResponse(response, error);

  CaptureOperationResult armResult =
      capture.arm(request.id, epoch, armedAfter, std::move(artifactPath));
  if (!armResult.ok) {
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      (void)sessionState.cancelAsync();
    }
    return emitResponse(
        Response::failure(request.id, epoch, armResult.errorCode,
                          armResult.message),
        error);
  }

  requestAutomationLcdInvalidation();
  return StdioPumpResult::Continue;
}

StdioPumpResult AutomationStdio::processRestart(const Request& request,
                                                std::string* error)
{
  const bool firmwareIdle = simuAutomationFirmwareIdle();
  Response response;
  SessionEpoch epoch = 0;
  bool reserved = false;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    epoch = sessionState.epoch();
    if (!targetDescription.capabilities.warmRestart) {
      response =
          Response::failure(request.id, epoch, ErrorCode::UnsupportedCommand,
                            "warm restart is not available");
    } else if (sessionState.phase() != SessionPhase::Ready) {
      response = Response::failure(request.id, epoch,
                                   sessionState.phase() == SessionPhase::Stopped
                                       ? ErrorCode::SessionStopping
                                       : ErrorCode::OperationBusy,
                                   "session is not ready for warm restart");
    } else if (sessionState.asyncOperation() != AsyncOperation::None ||
               pendingFrameWait.active() || completedFrameWait.active() ||
               pendingFirmware.active() || completedRestart.active()) {
      response = Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                                   "an asynchronous operation is active");
    } else if (!firmwareIdle) {
      response = Response::failure(request.id, epoch, ErrorCode::OperationBusy,
                                   "firmware mailbox is not idle");
    } else if (sessionState.beginAsync(AsyncOperation::Restart, request.id) !=
               TransitionResult::Applied) {
      response =
          Response::failure(request.id, epoch, ErrorCode::InvariantViolation,
                            "cannot reserve the warm restart");
    } else {
      pendingRestart.id = request.id;
      pendingRestart.epoch = epoch;
      reserved = true;
    }
  }

  if (!reserved) return emitResponse(response, error);
  releaseInputs();
  AutomationInputHandlers handlers;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    handlers = inputHandlers;
  }
  if (handlers.resetSwitches != nullptr) handlers.resetSwitches();
  return StdioPumpResult::RestartRequested;
}

StdioPumpResult AutomationStdio::processReleaseAll(const Request& request,
                                                   std::string* error)
{
  releaseInputs();
  return emitResponse(Response::success(request.id, currentEpoch()), error);
}

StdioPumpResult AutomationStdio::processStop(const Request& request,
                                             std::string* error)
{
  CaptureCompletion captureCompletion;
  const bool hasCaptureCompletion = capture.cancelAndWait(&captureCompletion);
  CompletedFrameWait frameCompletion;
  Response frameCancellation;
  bool frameCancelled = false;
  Response firmwareCancellation;
  bool firmwareCancelled = false;
  SessionEpoch epoch = 0;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    frameCompletion = completedFrameWait;
    completedFrameWait.clear();
    epoch = sessionState.epoch();
    if (hasCaptureCompletion) {
      const TransitionResult transition = sessionState.completeAsync(
          captureCompletion.id, captureCompletion.epoch);
      if (transition != TransitionResult::Applied) {
        captureCompletion.ok = false;
        captureCompletion.errorCode = ErrorCode::InvariantViolation;
        captureCompletion.message =
            "capture completion did not own the asynchronous operation";
      }
    }
    if (pendingFrameWait.active()) {
      const RequestId pendingId = pendingFrameWait.id;
      const SessionEpoch pendingEpoch = pendingFrameWait.epoch;
      (void)sessionState.cancelAsync();
      pendingFrameWait.clear();
      frameCancellation = Response::failure(
          pendingId, pendingEpoch, ErrorCode::SessionStopping,
          "frame barrier cancelled because the session is stopping");
      frameCancelled = true;
    }
    if (pendingFirmware.active()) {
      firmwareCancellation = Response::failure(
          pendingFirmware.id, pendingFirmware.epoch, ErrorCode::SessionStopping,
          "firmware operation cancelled because the session is stopping");
      (void)sessionState.cancelAsync();
      pendingFirmware.clear();
      firmwareCancelled = true;
    }
  }

  releaseInputs();
  if (frameCompletion.active()) {
    const StdioPumpResult result = emitResponse(
        Response::successWithFrame(frameCompletion.id, frameCompletion.epoch,
                                   frameCompletion.sequence),
        error);
    if (result != StdioPumpResult::Continue) return result;
  }
  if (frameCancelled) {
    const StdioPumpResult result = emitResponse(frameCancellation, error);
    if (result != StdioPumpResult::Continue) return result;
  }
  if (firmwareCancelled) {
    const StdioPumpResult result = emitResponse(firmwareCancellation, error);
    if (result != StdioPumpResult::Continue) return result;
  }
  if (hasCaptureCompletion) {
    const StdioPumpResult result =
        emitResponse(captureResponse(captureCompletion), error);
    if (result != StdioPumpResult::Continue) return result;
  }

  const StdioPumpResult writeResult =
      emitResponse(Response::success(request.id, epoch), error);
  if (writeResult != StdioPumpResult::Continue) return writeResult;
  stopAfterFlush = true;
  return StdioPumpResult::Continue;
}

StdioPumpResult AutomationStdio::drainCompletedResponses(std::string* error)
{
  FirmwareCompletion firmwareCompletion;
  while (simuAutomationTakeFirmwareCompletion(&firmwareCompletion)) {
    bool matched = false;
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      const bool generationMatches =
          pendingFirmware.operation != AsyncOperation::ReloadLua ||
          pendingFirmware.generation == firmwareCompletion.generation;
      const bool operationMatches =
          (pendingFirmware.operation == AsyncOperation::Firmware &&
           firmwareCompletion.operation == FirmwareOperation::Telemetry) ||
          (pendingFirmware.operation == AsyncOperation::ReloadLua &&
           firmwareCompletion.operation == FirmwareOperation::ReloadLua);
      if (!pendingFirmware.active() ||
          pendingFirmware.id != firmwareCompletion.id ||
          pendingFirmware.epoch != firmwareCompletion.epoch ||
          firmwareCompletion.epoch != sessionState.epoch() ||
          !generationMatches || !operationMatches) {
        incrementSaturating(&staleCompletionCount);
      } else {
        const TransitionResult transition = sessionState.completeAsync(
            firmwareCompletion.id, firmwareCompletion.epoch);
        if (transition == TransitionResult::Applied) {
          pendingFirmware.clear();
          matched = true;
        } else {
          incrementSaturating(&staleCompletionCount);
        }
      }
    }
    if (!matched) continue;

    Response response;
    if (firmwareCompletion.code == FirmwareCompletionCode::None &&
        firmwareCompletion.operation == FirmwareOperation::Telemetry) {
      response =
          Response::success(firmwareCompletion.id, firmwareCompletion.epoch);
    } else if (firmwareCompletion.code == FirmwareCompletionCode::None &&
               firmwareCompletion.operation == FirmwareOperation::ReloadLua &&
               firmwareCompletion.luaState == AutomationLuaState::Running) {
      LuaReloadResult luaReload;
      luaReload.generation = firmwareCompletion.generation;
      luaReload.state = automationLuaStateName(firmwareCompletion.luaState);
      response = Response::successWithLuaReload(
          firmwareCompletion.id, firmwareCompletion.epoch, luaReload);
    } else if (firmwareCompletion.code ==
               FirmwareCompletionCode::TelemetryUnavailable) {
      response = Response::failure(
          firmwareCompletion.id, firmwareCompletion.epoch,
          ErrorCode::UnsupportedTarget,
          "no telemetry sensor slot is available in the run fixture");
    } else if (firmwareCompletion.code ==
               FirmwareCompletionCode::LuaUnavailable) {
      response = Response::failure(
          firmwareCompletion.id, firmwareCompletion.epoch,
          ErrorCode::LuaUnavailable, "Lua is not available on this target");
    } else if (firmwareCompletion.code == FirmwareCompletionCode::LuaPanic) {
      response = Response::failure(
          firmwareCompletion.id, firmwareCompletion.epoch, ErrorCode::LuaPanic,
          "Lua reload generation " +
              std::to_string(firmwareCompletion.generation) +
              " reached interpreter panic");
    } else {
      response = Response::failure(
          firmwareCompletion.id, firmwareCompletion.epoch,
          ErrorCode::InternalError, "firmware operation did not complete");
    }

    const StdioPumpResult result = emitResponse(response, error);
    if (result != StdioPumpResult::Continue) return result;
  }

  CaptureCompletion captureCompletion;
  if (capture.takeCompletion(&captureCompletion)) {
    {
      std::lock_guard<std::mutex> lock(stateMutex);
      const TransitionResult transition = sessionState.completeAsync(
          captureCompletion.id, captureCompletion.epoch);
      if (transition != TransitionResult::Applied) {
        captureCompletion.ok = false;
        captureCompletion.errorCode = ErrorCode::InvariantViolation;
        captureCompletion.message =
            "capture completion did not own the asynchronous operation";
      }
    }
    const StdioPumpResult result =
        emitResponse(captureResponse(captureCompletion), error);
    if (result != StdioPumpResult::Continue) return result;
  }

  CompletedFrameWait completion;
  CompletedRestart restartCompletion;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    if (completedFrameWait.active()) {
      completion = completedFrameWait;
      completedFrameWait.clear();
    }
    if (completedRestart.active()) {
      restartCompletion = completedRestart;
      completedRestart.clear();
    }
  }
  if (completion.active()) {
    const StdioPumpResult result =
        emitResponse(Response::successWithFrame(completion.id, completion.epoch,
                                                completion.sequence),
                     error);
    if (result != StdioPumpResult::Continue) return result;
  }
  if (restartCompletion.active()) {
    return emitResponse(Response::successWithFrame(restartCompletion.id,
                                                   restartCompletion.epoch,
                                                   restartCompletion.sequence),
                        error);
  }
  return StdioPumpResult::Continue;
}

void AutomationStdio::releaseInputs()
{
  std::vector<std::string> keys;
  bool touchActive = false;
  AutomationInputHandlers handlers;
  {
    std::lock_guard<std::mutex> lock(stateMutex);
    keys = sessionState.activeKeyNames();
    touchActive = sessionState.isTouchActive();
    sessionState.releaseAll();
    handlers = inputHandlers;
  }

  if (handlers.setKey != nullptr) {
    for (const std::string& key : keys) handlers.setKey(key, false);
  }
  if (touchActive && handlers.touchUp != nullptr) handlers.touchUp();
  if (handlers.clearAllAnalogs != nullptr) handlers.clearAllAnalogs();
}

bool AutomationStdio::supportsCommand(Command command) const
{
  std::lock_guard<std::mutex> lock(stateMutex);
  return std::find(targetDescription.commands.begin(),
                   targetDescription.commands.end(),
                   command) != targetDescription.commands.end();
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
  if (serializeEvent(currentEpoch(), code, message, &record) ==
      SerializeResult::LimitTooSmall) {
    setError(error, "cannot serialize automation event");
    return StdioPumpResult::Error;
  }
  return writeSerialized(record, error);
}

SessionEpoch AutomationStdio::currentEpoch() const
{
  std::lock_guard<std::mutex> lock(stateMutex);
  return sessionState.epoch();
}

Response AutomationStdio::makeStatusResponse(RequestId id) const
{
  const std::size_t firmwareDepth = simuAutomationFirmwareRequestDepth() +
                                    simuAutomationFirmwareCompletionDepth();
  const std::size_t analogCount = simuAutomationAnalogOverrideCount();
  const std::string luaState = automationLuaStateName(simuAutomationLuaState());
  std::lock_guard<std::mutex> lock(stateMutex);
  StatusSnapshot status;
  status.running = runtimeRunning;
  status.phase = sessionState.phase();
  status.displaySequence = sessionState.displaySequence();
  status.asyncOperation = sessionState.asyncOperation();
  status.requestQueueDepth =
      pendingEvents.size() + sessionState.queuedRequestCount();
  status.lineOverflowCount = lineOverflowCount;
  status.queueOverflowCount = queueOverflowCount;
  status.staleCompletionCount = staleCompletionCount;
  status.activeKeyCount = sessionState.activeKeyCount();
  status.touchActive = sessionState.isTouchActive();
  status.firmwareMailboxDepth = firmwareDepth;
  status.analogOverrideCount = analogCount;
  status.luaState = luaState;
  return Response::successWithStatus(id, sessionState.epoch(), status,
                                     targetDescription);
}

Response AutomationStdio::makeDescriptionResponse(RequestId id) const
{
  std::lock_guard<std::mutex> lock(stateMutex);
  return Response::successWithDescription(id, sessionState.epoch(),
                                          targetDescription);
}

StdioPumpResult AutomationStdio::writeSerialized(const std::string& record,
                                                 std::string* error)
{
  {
    std::lock_guard<std::mutex> lock(outputMutex);
    if (outputResult == WriteResult::Closed) return StdioPumpResult::PeerClosed;
    if (outputResult == WriteResult::Error) {
      setError(error, outputError.empty() ? "automation stdout writer failed"
                                          : outputError);
      return StdioPumpResult::Error;
    }
    if (outputStop) return StdioPumpResult::PeerClosed;
    if (outputQueue.size() >= STDIO_OUTPUT_QUEUE_CAPACITY) {
      setError(error, "automation stdout queue capacity exceeded");
      return StdioPumpResult::Error;
    }
    outputQueue.push_back(record);
  }
  outputReady.notify_one();
  return StdioPumpResult::Continue;
}

}  // namespace automation
}  // namespace edgetx
