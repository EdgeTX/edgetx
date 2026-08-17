#pragma once

#include <cstddef>
#include <cstdint>
#include <deque>
#include <set>
#include <string>
#include <vector>

namespace edgetx
{
namespace automation
{

constexpr std::size_t MAX_RECORD_BYTES = 16 * 1024;
constexpr std::size_t MAX_RESPONSE_BYTES = 16 * 1024;
constexpr std::size_t MAX_CAPTURE_PATH_BYTES = 1024;
constexpr std::size_t MAX_PENDING_REQUESTS = 64;

using RequestId = std::uint64_t;
using SessionEpoch = std::uint64_t;
using DisplaySequence = std::uint64_t;

enum class Command {
  Ping,
  Status,
  Describe,
  KeyDown,
  KeyUp,
  Rotate,
  TouchDown,
  TouchMove,
  TouchUp,
  SetSwitch,
  SetAnalog,
  ClearAnalog,
  SetTelemetry,
  ReloadLua,
  WaitFrame,
  Capture,
  Restart,
  ReleaseAll,
  Stop,
};

enum class ErrorCode {
  None,
  InvalidUtf8,
  LineTooLong,
  InvalidRecord,
  UnsupportedVersion,
  InvalidId,
  IdNotMonotonic,
  UnknownCommand,
  MissingArgument,
  ExtraArgument,
  InvalidArgument,
  OutOfRange,
  UnsupportedCommand,
  UnsupportedTarget,
  UnsupportedLcdDepth,
  KeyAlreadyDown,
  KeyNotDown,
  TouchAlreadyDown,
  TouchNotDown,
  QueueFull,
  FirmwareQueueFull,
  OperationBusy,
  ResponseTooLarge,
  SessionStopping,
  RestartFailed,
  LuaUnavailable,
  LuaPanic,
  UnsafePath,
  PathTooLong,
  ArtifactExists,
  CaptureFailed,
  CaptureCancelled,
  StdinClosed,
  StdoutClosed,
  IoError,
  InvariantViolation,
  InternalError,
};

enum class AsyncOperation {
  None,
  WaitFrame,
  Capture,
  Firmware,
  ReloadLua,
  Restart,
};

struct TargetCapabilities {
  bool rotary = false;
  bool touch = false;
  bool switches = false;
  bool analog = false;
  bool telemetry = false;
  bool lua = false;
  bool capture = false;
  bool warmRestart = false;
};

struct Request {
  RequestId id = 0;
  Command command = Command::Ping;
  std::vector<std::string> arguments;
};

struct ProtocolError {
  ErrorCode code = ErrorCode::None;
  std::string message;
  RequestId requestId = 0;
  bool hasRequestId = false;
};

enum class ParseStatus {
  Ignored,
  Request,
  Error,
};

struct ParseResult {
  ParseStatus status = ParseStatus::Ignored;
  Request request;
  ProtocolError error;
};

enum class LineEventType {
  Record,
  LineTooLong,
  PartialRecordAtEof,
};

struct LineEvent {
  LineEventType type = LineEventType::Record;
  std::string record;
};

class LineBuffer
{
 public:
  explicit LineBuffer(std::size_t maxRecordBytes = MAX_RECORD_BYTES);

  std::vector<LineEvent> feed(const char* bytes, std::size_t size);
  std::vector<LineEvent> finish();

  std::size_t bufferedBytes() const;
  bool isDiscarding() const;

 private:
  bool append(char byte);
  void resetRecord();

  const std::size_t maxRecordBytes;
  std::string buffer;
  bool pendingCarriageReturn = false;
  bool discarding = false;
};

class ProtocolParser
{
 public:
  ParseResult parse(const std::string& record);
  RequestId lastRequestId() const;

 private:
  RequestId lastId = 0;
};

struct Response {
  RequestId id = 0;
  bool ok = true;
  SessionEpoch epoch = 0;
  ErrorCode errorCode = ErrorCode::None;
  std::string message;

  static Response success(RequestId id, SessionEpoch epoch);
  static Response failure(RequestId id, SessionEpoch epoch, ErrorCode code,
                          const std::string& message);
};

enum class SerializeResult {
  Serialized,
  UsedSizeFallback,
  LimitTooSmall,
};

SerializeResult serializeResponse(const Response& response, std::string* output,
                                  std::size_t maxBytes = MAX_RESPONSE_BYTES);

const char* commandName(Command command);
const char* errorCodeName(ErrorCode code);
bool isValidUtf8(const std::string& value);

enum class TerminalClaimResult {
  Claimed,
  Duplicate,
  StaleEpoch,
};

class TerminalResponseOwner
{
 public:
  TerminalResponseOwner(RequestId requestId, SessionEpoch epoch);
  TerminalResponseOwner(const TerminalResponseOwner&) = delete;
  TerminalResponseOwner& operator=(const TerminalResponseOwner&) = delete;
  TerminalResponseOwner(TerminalResponseOwner&&) = delete;
  TerminalResponseOwner& operator=(TerminalResponseOwner&&) = delete;

  TerminalClaimResult claim(SessionEpoch currentEpoch);
  TerminalClaimResult cancel(SessionEpoch currentEpoch);
  bool isTerminal() const;
  RequestId requestId() const;

 private:
  RequestId id;
  SessionEpoch ownerEpoch;
  bool terminal = false;
};

enum class SessionPhase {
  Starting,
  Ready,
  Restarting,
  Stopped,
};

enum class TransitionResult {
  Applied,
  Duplicate,
  Busy,
  InvalidState,
  NotPending,
  StaleEpoch,
  QueueFull,
};

class SessionState
{
 public:
  SessionState();

  RequestId onDisplayFrame();
  TransitionResult keyDown(const std::string& key);
  TransitionResult keyUp(const std::string& key);
  TransitionResult touchDown(std::uint16_t x, std::uint16_t y);
  TransitionResult touchMove(std::uint16_t x, std::uint16_t y);
  TransitionResult touchUp();
  void releaseAll();

  TransitionResult enqueue(RequestId id);
  bool dequeue(RequestId* id);

  TransitionResult beginAsync(AsyncOperation operation, RequestId id);
  TransitionResult completeAsync(RequestId id, SessionEpoch completionEpoch);
  TransitionResult cancelAsync();
  TransitionResult restartTasksStarted(RequestId id, SessionEpoch requestEpoch);
  void stop();

  SessionPhase phase() const;
  SessionEpoch epoch() const;
  DisplaySequence displaySequence() const;
  AsyncOperation asyncOperation() const;
  std::size_t activeKeyCount() const;
  bool isTouchActive() const;
  std::size_t queuedRequestCount() const;

 private:
  void clearAsync();

  SessionPhase currentPhase = SessionPhase::Starting;
  SessionEpoch currentEpoch = 0;
  DisplaySequence currentDisplaySequence = 0;
  std::set<std::string> activeKeys;
  bool touchActive = false;
  std::uint16_t touchX = 0;
  std::uint16_t touchY = 0;
  std::deque<RequestId> pendingRequests;
  AsyncOperation activeOperation = AsyncOperation::None;
  RequestId activeRequestId = 0;
  SessionEpoch activeRequestEpoch = 0;
  RequestId lastTerminalAsyncRequestId = 0;
};

}  // namespace automation
}  // namespace edgetx
