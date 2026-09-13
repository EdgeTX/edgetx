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

// Total wire size, including the LF or CRLF record delimiter.
constexpr std::size_t MAX_RECORD_BYTES = 16 * 1024;
constexpr std::size_t MAX_RESPONSE_BYTES = 16 * 1024;
constexpr std::size_t MAX_CAPTURE_PATH_BYTES = 1024;
constexpr std::size_t MAX_PENDING_REQUESTS = 64;
constexpr std::size_t MAX_TELEMETRY_LABEL_BYTES = 4;

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

enum class SessionPhase {
  Starting,
  Ready,
  Restarting,
  Stopped,
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

struct NamedRange {
  std::string name;
  std::int32_t minimum = 0;
  std::int32_t maximum = 0;
};

struct TargetDescription {
  std::string flavour;
  std::uint16_t lcdWidth = 0;
  std::uint16_t lcdHeight = 0;
  std::uint8_t lcdDepth = 0;
  std::vector<Command> commands;
  TargetCapabilities capabilities;
  std::vector<std::string> keys;
  std::vector<NamedRange> switches;
  std::vector<NamedRange> analogs;
  bool outputRootReady = false;
};

struct StatusSnapshot {
  bool running = false;
  SessionPhase phase = SessionPhase::Starting;
  DisplaySequence displaySequence = 0;
  AsyncOperation asyncOperation = AsyncOperation::None;
  std::size_t requestQueueDepth = 0;
  std::size_t firmwareMailboxDepth = 0;
  std::uint64_t lineOverflowCount = 0;
  std::uint64_t queueOverflowCount = 0;
  std::uint64_t staleCompletionCount = 0;
  std::size_t activeKeyCount = 0;
  bool touchActive = false;
  std::size_t analogOverrideCount = 0;
  std::string luaState = "unavailable";
};

struct CaptureResult {
  DisplaySequence displaySequence = 0;
  std::string path;
  std::uint16_t width = 0;
  std::uint16_t height = 0;
  std::uint8_t depth = 0;
  std::uint64_t bytes = 0;
};

struct LuaReloadResult {
  std::uint64_t generation = 0;
  std::string state;
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
  enum class ResultKind {
    None,
    Status,
    Description,
    Frame,
    Capture,
    LuaReload,
  };

  RequestId id = 0;
  bool ok = true;
  SessionEpoch epoch = 0;
  ErrorCode errorCode = ErrorCode::None;
  std::string message;
  ResultKind resultKind = ResultKind::None;
  StatusSnapshot status;
  TargetDescription target;
  DisplaySequence frameSequence = 0;
  CaptureResult capture;
  LuaReloadResult luaReload;

  static Response success(RequestId id, SessionEpoch epoch);
  static Response successWithStatus(RequestId id, SessionEpoch epoch,
                                    const StatusSnapshot& status,
                                    const TargetDescription& target);
  static Response successWithDescription(RequestId id, SessionEpoch epoch,
                                         const TargetDescription& target);
  static Response successWithFrame(RequestId id, SessionEpoch epoch,
                                   DisplaySequence displaySequence);
  static Response successWithCapture(RequestId id, SessionEpoch epoch,
                                     const CaptureResult& capture);
  static Response successWithLuaReload(RequestId id, SessionEpoch epoch,
                                       const LuaReloadResult& luaReload);
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
SerializeResult serializeEvent(SessionEpoch epoch, ErrorCode code,
                               const std::string& message, std::string* output,
                               std::size_t maxBytes = MAX_RESPONSE_BYTES);

const char* commandName(Command command);
const char* errorCodeName(ErrorCode code);
const char* sessionPhaseName(SessionPhase phase);
const char* asyncOperationName(AsyncOperation operation);
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
  std::vector<std::string> activeKeyNames() const;
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
