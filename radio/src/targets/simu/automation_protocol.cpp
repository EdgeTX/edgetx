#include "automation_protocol.h"

#include <algorithm>
#include <limits>

namespace edgetx
{
namespace automation
{

namespace
{

struct CommandSpec {
  const char* name;
  Command command;
  std::size_t minArguments;
  std::size_t maxArguments;
  bool remainderArgument;
};

constexpr CommandSpec COMMANDS[] = {
    {"ping", Command::Ping, 0, 0, false},
    {"status", Command::Status, 0, 0, false},
    {"describe", Command::Describe, 0, 0, false},
    {"key-down", Command::KeyDown, 1, 1, false},
    {"key-up", Command::KeyUp, 1, 1, false},
    {"rotate", Command::Rotate, 1, 1, false},
    {"touch-down", Command::TouchDown, 2, 2, false},
    {"touch-move", Command::TouchMove, 2, 2, false},
    {"touch-up", Command::TouchUp, 0, 0, false},
    {"set-switch", Command::SetSwitch, 2, 2, false},
    {"set-analog", Command::SetAnalog, 2, 2, false},
    {"clear-analog", Command::ClearAnalog, 1, 1, false},
    {"set-telemetry", Command::SetTelemetry, 6, 7, false},
    {"reload-lua", Command::ReloadLua, 0, 0, false},
    {"wait-frame", Command::WaitFrame, 1, 1, false},
    {"capture", Command::Capture, 1, 1, true},
    {"restart", Command::Restart, 0, 0, false},
    {"release-all", Command::ReleaseAll, 0, 0, false},
    {"stop", Command::Stop, 0, 0, false},
};

ParseResult makeError(ErrorCode code, const std::string& message,
                      RequestId id = 0, bool hasId = false)
{
  ParseResult result;
  result.status = ParseStatus::Error;
  result.error.code = code;
  result.error.message = message;
  result.error.requestId = id;
  result.error.hasRequestId = hasId;
  return result;
}

bool parseUnsigned(const std::string& token, std::uint64_t maximum,
                   std::uint64_t* value)
{
  if (token.empty()) return false;

  std::uint64_t parsed = 0;
  for (char byte : token) {
    if (byte < '0' || byte > '9') return false;
    const std::uint64_t digit = static_cast<std::uint64_t>(byte - '0');
    if (digit > maximum || parsed > (maximum - digit) / 10) return false;
    parsed = parsed * 10 + digit;
  }

  *value = parsed;
  return true;
}

bool parseSigned(const std::string& token, std::int64_t minimum,
                 std::int64_t maximum, std::int64_t* value)
{
  if (token.empty()) return false;

  bool negative = false;
  std::size_t offset = 0;
  if (token[0] == '-') {
    negative = true;
    offset = 1;
  } else if (token[0] == '+') {
    return false;
  }
  if (offset == token.size()) return false;

  const std::uint64_t positiveLimit = static_cast<std::uint64_t>(maximum);
  const std::uint64_t negativeLimit =
      static_cast<std::uint64_t>(-(minimum + 1)) + 1;
  std::uint64_t magnitude = 0;
  if (!parseUnsigned(token.substr(offset),
                     negative ? negativeLimit : positiveLimit, &magnitude)) {
    return false;
  }

  if (negative) {
    if (magnitude == negativeLimit) {
      *value = minimum;
    } else {
      *value = -static_cast<std::int64_t>(magnitude);
    }
  } else {
    *value = static_cast<std::int64_t>(magnitude);
  }
  return *value >= minimum && *value <= maximum;
}

bool isAsciiToken(const std::string& token)
{
  if (token.empty()) return false;
  for (unsigned char byte : token) {
    if (byte <= 0x20 || byte >= 0x7f) return false;
  }
  return true;
}

bool isTelemetryLabel(const std::string& token)
{
  if (token.empty() || token.size() > MAX_TELEMETRY_LABEL_BYTES) return false;
  for (const unsigned char byte : token) {
    const bool alpha =
        (byte >= 'A' && byte <= 'Z') || (byte >= 'a' && byte <= 'z');
    const bool digit = byte >= '0' && byte <= '9';
    if (!alpha && !digit && byte != '_' && byte != '-') return false;
  }
  return true;
}

std::vector<std::string> splitArguments(const std::string& arguments,
                                        bool* validSeparators)
{
  std::vector<std::string> result;
  *validSeparators = true;
  if (arguments.empty()) return result;

  std::size_t start = 0;
  while (start <= arguments.size()) {
    const std::size_t separator = arguments.find(' ', start);
    const std::size_t end =
        separator == std::string::npos ? arguments.size() : separator;
    if (end == start) {
      *validSeparators = false;
      return result;
    }
    result.push_back(arguments.substr(start, end - start));
    if (separator == std::string::npos) break;
    start = separator + 1;
  }
  return result;
}

const CommandSpec* findCommand(const std::string& name)
{
  for (const auto& spec : COMMANDS) {
    if (name == spec.name) return &spec;
  }
  return nullptr;
}

ErrorCode validateArguments(Command command,
                            const std::vector<std::string>& arguments)
{
  std::uint64_t unsignedValue = 0;
  std::int64_t signedValue = 0;

  switch (command) {
    case Command::KeyDown:
    case Command::KeyUp:
    case Command::ClearAnalog:
      return isAsciiToken(arguments[0]) ? ErrorCode::None
                                        : ErrorCode::InvalidArgument;

    case Command::Rotate:
      if (!parseSigned(arguments[0], -128, 128, &signedValue))
        return ErrorCode::OutOfRange;
      return signedValue == 0 ? ErrorCode::InvalidArgument : ErrorCode::None;

    case Command::TouchDown:
    case Command::TouchMove:
      for (const auto& argument : arguments) {
        if (!parseUnsigned(argument, std::numeric_limits<std::uint16_t>::max(),
                           &unsignedValue))
          return ErrorCode::OutOfRange;
      }
      return ErrorCode::None;

    case Command::SetSwitch:
      if (!isAsciiToken(arguments[0])) return ErrorCode::InvalidArgument;
      return parseSigned(arguments[1], std::numeric_limits<std::int32_t>::min(),
                         std::numeric_limits<std::int32_t>::max(), &signedValue)
                 ? ErrorCode::None
                 : ErrorCode::OutOfRange;

    case Command::SetAnalog:
      if (!isAsciiToken(arguments[0])) return ErrorCode::InvalidArgument;
      return parseUnsigned(arguments[1], 4096, &unsignedValue)
                 ? ErrorCode::None
                 : ErrorCode::OutOfRange;

    case Command::SetTelemetry: {
      const std::uint64_t limits[] = {65535, 7, 255};
      for (std::size_t index = 0; index < 3; ++index) {
        if (!parseUnsigned(arguments[index], limits[index], &unsignedValue))
          return ErrorCode::OutOfRange;
        if (index == 0 && unsignedValue == 0) return ErrorCode::OutOfRange;
      }
      if (!parseSigned(arguments[3], std::numeric_limits<std::int32_t>::min(),
                       std::numeric_limits<std::int32_t>::max(), &signedValue))
        return ErrorCode::OutOfRange;
      if (!parseUnsigned(arguments[4], 255, &unsignedValue) ||
          !parseUnsigned(arguments[5], 2, &unsignedValue))
        return ErrorCode::OutOfRange;
      if (arguments.size() == 7 && !isTelemetryLabel(arguments[6]))
        return ErrorCode::InvalidArgument;
      return ErrorCode::None;
    }

    case Command::WaitFrame:
      return parseUnsigned(arguments[0],
                           std::numeric_limits<std::uint64_t>::max(),
                           &unsignedValue)
                 ? ErrorCode::None
                 : ErrorCode::OutOfRange;

    case Command::Capture:
      if (arguments[0].size() > MAX_CAPTURE_PATH_BYTES)
        return ErrorCode::PathTooLong;
      return arguments[0].empty() ? ErrorCode::MissingArgument
                                  : ErrorCode::None;

    default:
      return ErrorCode::None;
  }
}

class BoundedJson
{
 public:
  explicit BoundedJson(std::size_t limit) : limit(limit)
  {
    value.reserve(std::min<std::size_t>(limit, 512));
  }

  bool append(const std::string& text)
  {
    if (text.size() > limit - value.size()) return false;
    value += text;
    return true;
  }

  bool append(const char* text) { return append(std::string(text)); }

  bool appendNumber(std::uint64_t number)
  {
    return append(std::to_string(number));
  }

  bool appendSigned(std::int64_t number)
  {
    return append(std::to_string(number));
  }

  bool appendBoolean(bool boolean)
  {
    return append(boolean ? "true" : "false");
  }

  bool appendString(const std::string& text)
  {
    if (!isValidUtf8(text) || !append("\"")) return false;
    static const char HEX[] = "0123456789abcdef";

    for (unsigned char byte : text) {
      switch (byte) {
        case '"':
          if (!append("\\\"")) return false;
          break;
        case '\\':
          if (!append("\\\\")) return false;
          break;
        case '\b':
          if (!append("\\b")) return false;
          break;
        case '\f':
          if (!append("\\f")) return false;
          break;
        case '\n':
          if (!append("\\n")) return false;
          break;
        case '\r':
          if (!append("\\r")) return false;
          break;
        case '\t':
          if (!append("\\t")) return false;
          break;
        default:
          if (byte < 0x20) {
            char escaped[] = {
                '\\', 'u', '0', '0', HEX[byte >> 4], HEX[byte & 0x0f], '\0'};
            if (!append(escaped)) return false;
          } else if (!append(std::string(1, static_cast<char>(byte)))) {
            return false;
          }
      }
    }
    return append("\"");
  }

  std::string take() { return value; }

 private:
  std::size_t limit;
  std::string value;
};

bool appendLcdDescription(BoundedJson& json, const TargetDescription& target)
{
  return json.append("{\"width\":") && json.appendNumber(target.lcdWidth) &&
         json.append(",\"height\":") && json.appendNumber(target.lcdHeight) &&
         json.append(",\"depth\":") && json.appendNumber(target.lcdDepth) &&
         json.append("}");
}

bool appendCapabilities(BoundedJson& json,
                        const TargetCapabilities& capabilities)
{
  return json.append("{\"rotary\":") &&
         json.appendBoolean(capabilities.rotary) &&
         json.append(",\"touch\":") && json.appendBoolean(capabilities.touch) &&
         json.append(",\"switches\":") &&
         json.appendBoolean(capabilities.switches) &&
         json.append(",\"analog\":") &&
         json.appendBoolean(capabilities.analog) &&
         json.append(",\"telemetry\":") &&
         json.appendBoolean(capabilities.telemetry) &&
         json.append(",\"lua\":") && json.appendBoolean(capabilities.lua) &&
         json.append(",\"capture\":") &&
         json.appendBoolean(capabilities.capture) &&
         json.append(",\"warm_restart\":") &&
         json.appendBoolean(capabilities.warmRestart) && json.append("}");
}

bool appendCommands(BoundedJson& json, const std::vector<Command>& commands)
{
  if (!json.append("[")) return false;
  for (std::size_t index = 0; index < commands.size(); ++index) {
    if (index != 0 && !json.append(",")) return false;
    if (!json.appendString(commandName(commands[index]))) return false;
  }
  return json.append("]");
}

bool appendStrings(BoundedJson& json, const std::vector<std::string>& values)
{
  if (!json.append("[")) return false;
  for (std::size_t index = 0; index < values.size(); ++index) {
    if (index != 0 && !json.append(",")) return false;
    if (!json.appendString(values[index])) return false;
  }
  return json.append("]");
}

bool appendNamedRanges(BoundedJson& json, const std::vector<NamedRange>& ranges)
{
  if (!json.append("[")) return false;
  for (std::size_t index = 0; index < ranges.size(); ++index) {
    if (index != 0 && !json.append(",")) return false;
    const NamedRange& range = ranges[index];
    if (!json.append("{\"name\":") || !json.appendString(range.name) ||
        !json.append(",\"min\":") || !json.appendSigned(range.minimum) ||
        !json.append(",\"max\":") || !json.appendSigned(range.maximum) ||
        !json.append("}")) {
      return false;
    }
  }
  return json.append("]");
}

bool appendStatusResult(BoundedJson& json, const Response& response)
{
  const StatusSnapshot& status = response.status;
  const TargetDescription& target = response.target;
  if (!json.append(",\"result\":{\"protocol_version\":1,\"running\":") ||
      !json.appendBoolean(status.running) || !json.append(",\"phase\":") ||
      !json.appendString(sessionPhaseName(status.phase)) ||
      !json.append(",\"target\":") || !json.appendString(target.flavour) ||
      !json.append(",\"lcd\":") || !appendLcdDescription(json, target) ||
      !json.append(",\"display_seq\":") ||
      !json.appendNumber(status.displaySequence) ||
      !json.append(",\"async_operation\":") ||
      !json.appendString(asyncOperationName(status.asyncOperation)) ||
      !json.append(",\"request_queue_depth\":") ||
      !json.appendNumber(status.requestQueueDepth) ||
      !json.append(",\"firmware_mailbox_depth\":") ||
      !json.appendNumber(status.firmwareMailboxDepth) ||
      !json.append(",\"line_overflow_count\":") ||
      !json.appendNumber(status.lineOverflowCount) ||
      !json.append(",\"queue_overflow_count\":") ||
      !json.appendNumber(status.queueOverflowCount) ||
      !json.append(",\"stale_completion_count\":") ||
      !json.appendNumber(status.staleCompletionCount) ||
      !json.append(",\"active_key_count\":") ||
      !json.appendNumber(status.activeKeyCount) ||
      !json.append(",\"touch_active\":") ||
      !json.appendBoolean(status.touchActive) ||
      !json.append(",\"analog_override_count\":") ||
      !json.appendNumber(status.analogOverrideCount) ||
      !json.append(",\"lua_state\":") || !json.appendString(status.luaState) ||
      !json.append(",\"capabilities\":") ||
      !appendCapabilities(json, target.capabilities) ||
      !json.append(",\"output_root\":") ||
      !json.appendString(target.outputRootReady ? "ready" : "invalid") ||
      !json.append("}")) {
    return false;
  }
  return true;
}

bool appendDescriptionResult(BoundedJson& json, const Response& response)
{
  const TargetDescription& target = response.target;
  if (!json.append(",\"result\":{\"protocol_version\":1,\"target\":") ||
      !json.appendString(target.flavour) || !json.append(",\"lcd\":") ||
      !appendLcdDescription(json, target) || !json.append(",\"commands\":") ||
      !appendCommands(json, target.commands) ||
      !json.append(",\"capabilities\":") ||
      !appendCapabilities(json, target.capabilities) ||
      !json.append(",\"keys\":") || !appendStrings(json, target.keys) ||
      !json.append(",\"switches\":") ||
      !appendNamedRanges(json, target.switches) ||
      !json.append(",\"analogs\":") ||
      !appendNamedRanges(json, target.analogs) || !json.append("}")) {
    return false;
  }
  return true;
}

bool appendFrameResult(BoundedJson& json, const Response& response)
{
  return json.append(",\"result\":{\"display_seq\":") &&
         json.appendNumber(response.frameSequence) && json.append("}");
}

bool appendCaptureResult(BoundedJson& json, const Response& response)
{
  const CaptureResult& capture = response.capture;
  return json.append(",\"result\":{\"display_seq\":") &&
         json.appendNumber(capture.displaySequence) &&
         json.append(",\"path\":") && json.appendString(capture.path) &&
         json.append(",\"width\":") && json.appendNumber(capture.width) &&
         json.append(",\"height\":") && json.appendNumber(capture.height) &&
         json.append(",\"depth\":") && json.appendNumber(capture.depth) &&
         json.append(",\"bytes\":") && json.appendNumber(capture.bytes) &&
         json.append("}");
}

bool appendLuaReloadResult(BoundedJson& json, const Response& response)
{
  return json.append(",\"result\":{\"generation\":") &&
         json.appendNumber(response.luaReload.generation) &&
         json.append(",\"state\":") &&
         json.appendString(response.luaReload.state) && json.append("}");
}

bool buildResponse(const Response& response, ErrorCode code,
                   const std::string& message, std::size_t maxBytes,
                   std::string* output)
{
  BoundedJson json(maxBytes);
  if (!json.append("{\"version\":1,\"type\":\"response\",\"id\":") ||
      !json.appendNumber(response.id) || !json.append(",\"ok\":") ||
      !json.append(code == ErrorCode::None ? "true" : "false") ||
      !json.append(",\"epoch\":") || !json.appendNumber(response.epoch)) {
    return false;
  }

  if (code != ErrorCode::None) {
    if (!json.append(",\"error\":{\"code\":\"") ||
        !json.append(errorCodeName(code)) || !json.append("\",\"message\":") ||
        !json.appendString(message) || !json.append("}")) {
      return false;
    }
  } else if (response.resultKind == Response::ResultKind::Status) {
    if (!appendStatusResult(json, response)) return false;
  } else if (response.resultKind == Response::ResultKind::Description) {
    if (!appendDescriptionResult(json, response)) return false;
  } else if (response.resultKind == Response::ResultKind::Frame) {
    if (!appendFrameResult(json, response)) return false;
  } else if (response.resultKind == Response::ResultKind::Capture) {
    if (!appendCaptureResult(json, response)) return false;
  } else if (response.resultKind == Response::ResultKind::LuaReload) {
    if (!appendLuaReloadResult(json, response)) return false;
  }

  if (!json.append("}\n")) return false;
  *output = json.take();
  return true;
}

bool buildEvent(SessionEpoch epoch, ErrorCode code, const std::string& message,
                std::size_t maxBytes, std::string* output)
{
  BoundedJson json(maxBytes);
  if (!json.append(
          "{\"version\":1,\"type\":\"event\",\"id\":null,\"epoch\":") ||
      !json.appendNumber(epoch) || !json.append(",\"event\":{\"code\":\"") ||
      !json.append(errorCodeName(code)) || !json.append("\",\"message\":") ||
      !json.appendString(message) || !json.append("}}\n")) {
    return false;
  }

  *output = json.take();
  return true;
}

}  // namespace

LineBuffer::LineBuffer(std::size_t maxRecordBytes) :
    maxRecordBytes(maxRecordBytes)
{
  buffer.reserve(maxRecordBytes);
}

bool LineBuffer::append(char byte)
{
  if (buffer.size() == maxRecordBytes) {
    buffer.clear();
    discarding = true;
    pendingCarriageReturn = false;
    return false;
  }
  buffer.push_back(byte);
  return true;
}

void LineBuffer::resetRecord()
{
  buffer.clear();
  pendingCarriageReturn = false;
  discarding = false;
}

std::vector<LineEvent> LineBuffer::feed(const char* bytes, std::size_t size)
{
  std::vector<LineEvent> events;
  for (std::size_t index = 0; index < size; ++index) {
    const char byte = bytes[index];

    if (discarding) {
      if (byte == '\n') {
        events.push_back({LineEventType::LineTooLong, std::string()});
        resetRecord();
      }
      continue;
    }

    if (byte == '\n') {
      const std::size_t delimiterBytes = pendingCarriageReturn ? 2 : 1;
      if (delimiterBytes > maxRecordBytes ||
          buffer.size() > maxRecordBytes - delimiterBytes) {
        events.push_back({LineEventType::LineTooLong, std::string()});
        resetRecord();
        continue;
      }
      pendingCarriageReturn = false;
      events.push_back({LineEventType::Record, buffer});
      resetRecord();
      continue;
    }

    if (byte == '\r') {
      if (pendingCarriageReturn && !append('\r')) continue;
      pendingCarriageReturn = true;
      continue;
    }

    if (pendingCarriageReturn) {
      if (!append('\r')) continue;
      pendingCarriageReturn = false;
    }
    append(byte);
  }
  return events;
}

std::vector<LineEvent> LineBuffer::finish()
{
  std::vector<LineEvent> events;
  if (discarding) {
    events.push_back({LineEventType::LineTooLong, std::string()});
  } else {
    if (pendingCarriageReturn) append('\r');
    if (discarding) {
      events.push_back({LineEventType::LineTooLong, std::string()});
    } else if (!buffer.empty()) {
      events.push_back({LineEventType::PartialRecordAtEof, buffer});
    }
  }
  resetRecord();
  return events;
}

std::size_t LineBuffer::bufferedBytes() const { return buffer.size(); }

bool LineBuffer::isDiscarding() const { return discarding; }

ParseResult ProtocolParser::parse(const std::string& input)
{
  if (input.empty()) return ParseResult();
  if (input.size() >= MAX_RECORD_BYTES)
    return makeError(ErrorCode::LineTooLong, "record exceeds 16 KiB");

  std::string record = input;
  if (!record.empty() && record.back() == '\r') record.pop_back();
  if (record.empty()) return ParseResult();
  if (record[0] == ' ' || record[0] == '\t')
    return makeError(ErrorCode::InvalidRecord,
                     "leading whitespace is not allowed");

  const std::size_t versionEnd = record.find(' ');
  if (versionEnd == std::string::npos)
    return makeError(
        record == "v1" ? ErrorCode::InvalidId : ErrorCode::UnsupportedVersion,
        record == "v1" ? "request id is required"
                       : "unsupported protocol version");

  const std::string version = record.substr(0, versionEnd);
  const std::size_t idStart = versionEnd + 1;
  const std::size_t idEnd = record.find(' ', idStart);
  const std::string idToken =
      record.substr(idStart, idEnd == std::string::npos ? std::string::npos
                                                        : idEnd - idStart);
  std::uint64_t id = 0;
  if (!parseUnsigned(idToken, std::numeric_limits<std::uint64_t>::max(), &id) ||
      id == 0) {
    return makeError(ErrorCode::InvalidId, "request id must be 1..UINT64_MAX");
  }

  if (version != "v1") {
    return makeError(ErrorCode::UnsupportedVersion,
                     "unsupported protocol version", id, true);
  }

  if (id <= lastId) {
    return makeError(ErrorCode::IdNotMonotonic,
                     "request id must increase monotonically", id, true);
  }
  lastId = id;

  if (!isValidUtf8(record))
    return makeError(ErrorCode::InvalidUtf8, "record is not valid UTF-8", id,
                     true);
  if (record.find('\0') != std::string::npos)
    return makeError(ErrorCode::InvalidRecord, "NUL is not allowed", id, true);
  if (idEnd == std::string::npos || idEnd + 1 == record.size())
    return makeError(ErrorCode::MissingArgument, "command is required", id,
                     true);

  const std::size_t commandStart = idEnd + 1;
  const std::size_t commandEnd = record.find(' ', commandStart);
  const std::string commandToken =
      record.substr(commandStart, commandEnd == std::string::npos
                                      ? std::string::npos
                                      : commandEnd - commandStart);
  if (!isAsciiToken(commandToken))
    return makeError(ErrorCode::InvalidRecord, "invalid command token", id,
                     true);

  const CommandSpec* spec = findCommand(commandToken);
  if (spec == nullptr)
    return makeError(ErrorCode::UnknownCommand, "unknown command", id, true);

  std::vector<std::string> arguments;
  if (commandEnd != std::string::npos) {
    const std::string remainder = record.substr(commandEnd + 1);
    if (remainder.empty()) {
      return makeError(spec->minArguments == 0 ? ErrorCode::ExtraArgument
                                               : ErrorCode::MissingArgument,
                       "empty command argument", id, true);
    }
    if (spec->remainderArgument) {
      arguments.push_back(remainder);
    } else {
      bool separatorsValid = false;
      arguments = splitArguments(remainder, &separatorsValid);
      if (!separatorsValid)
        return makeError(ErrorCode::InvalidRecord,
                         "arguments must use one ASCII space", id, true);
    }
  }

  if (arguments.size() < spec->minArguments)
    return makeError(ErrorCode::MissingArgument, "missing command argument", id,
                     true);
  if (arguments.size() > spec->maxArguments)
    return makeError(ErrorCode::ExtraArgument, "too many command arguments", id,
                     true);

  const ErrorCode argumentError = validateArguments(spec->command, arguments);
  if (argumentError != ErrorCode::None)
    return makeError(argumentError, "invalid command argument", id, true);

  ParseResult result;
  result.status = ParseStatus::Request;
  result.request.id = id;
  result.request.command = spec->command;
  result.request.arguments = arguments;
  return result;
}

RequestId ProtocolParser::lastRequestId() const { return lastId; }

Response Response::success(RequestId id, SessionEpoch epoch)
{
  Response response;
  response.id = id;
  response.epoch = epoch;
  return response;
}

Response Response::successWithStatus(RequestId id, SessionEpoch epoch,
                                     const StatusSnapshot& status,
                                     const TargetDescription& target)
{
  Response response = success(id, epoch);
  response.resultKind = ResultKind::Status;
  response.status = status;
  response.target = target;
  return response;
}

Response Response::successWithDescription(RequestId id, SessionEpoch epoch,
                                          const TargetDescription& target)
{
  Response response = success(id, epoch);
  response.resultKind = ResultKind::Description;
  response.target = target;
  return response;
}

Response Response::successWithFrame(RequestId id, SessionEpoch epoch,
                                    DisplaySequence displaySequence)
{
  Response response = success(id, epoch);
  response.resultKind = ResultKind::Frame;
  response.frameSequence = displaySequence;
  return response;
}

Response Response::successWithCapture(RequestId id, SessionEpoch epoch,
                                      const CaptureResult& capture)
{
  Response response = success(id, epoch);
  response.resultKind = ResultKind::Capture;
  response.capture = capture;
  return response;
}

Response Response::successWithLuaReload(RequestId id, SessionEpoch epoch,
                                        const LuaReloadResult& luaReload)
{
  Response response = success(id, epoch);
  response.resultKind = ResultKind::LuaReload;
  response.luaReload = luaReload;
  return response;
}

Response Response::failure(RequestId id, SessionEpoch epoch, ErrorCode code,
                           const std::string& message)
{
  Response response;
  response.id = id;
  response.ok = false;
  response.epoch = epoch;
  response.errorCode = code;
  response.message = message;
  return response;
}

SerializeResult serializeResponse(const Response& response, std::string* output,
                                  std::size_t maxBytes)
{
  if (output == nullptr) return SerializeResult::LimitTooSmall;
  output->clear();

  const ErrorCode code = response.ok ? ErrorCode::None
                                     : (response.errorCode == ErrorCode::None
                                            ? ErrorCode::InternalError
                                            : response.errorCode);
  if (buildResponse(response, code, response.message, maxBytes, output))
    return SerializeResult::Serialized;

  output->clear();
  if (buildResponse(response, ErrorCode::ResponseTooLarge,
                    "response exceeded the protocol limit", maxBytes, output)) {
    return SerializeResult::UsedSizeFallback;
  }

  output->clear();
  return SerializeResult::LimitTooSmall;
}

SerializeResult serializeEvent(SessionEpoch epoch, ErrorCode code,
                               const std::string& message, std::string* output,
                               std::size_t maxBytes)
{
  if (output == nullptr) return SerializeResult::LimitTooSmall;
  output->clear();

  const ErrorCode eventCode =
      code == ErrorCode::None ? ErrorCode::InternalError : code;
  if (buildEvent(epoch, eventCode, message, maxBytes, output))
    return SerializeResult::Serialized;

  output->clear();
  if (buildEvent(epoch, ErrorCode::ResponseTooLarge,
                 "event exceeded the protocol limit", maxBytes, output)) {
    return SerializeResult::UsedSizeFallback;
  }

  output->clear();
  return SerializeResult::LimitTooSmall;
}

const char* commandName(Command command)
{
  for (const auto& spec : COMMANDS) {
    if (spec.command == command) return spec.name;
  }
  return "unknown";
}

const char* errorCodeName(ErrorCode code)
{
  switch (code) {
    case ErrorCode::None:
      return "none";
    case ErrorCode::InvalidUtf8:
      return "invalid_utf8";
    case ErrorCode::LineTooLong:
      return "line_too_long";
    case ErrorCode::InvalidRecord:
      return "invalid_record";
    case ErrorCode::UnsupportedVersion:
      return "unsupported_version";
    case ErrorCode::InvalidId:
      return "invalid_id";
    case ErrorCode::IdNotMonotonic:
      return "id_not_monotonic";
    case ErrorCode::UnknownCommand:
      return "unknown_command";
    case ErrorCode::MissingArgument:
      return "missing_argument";
    case ErrorCode::ExtraArgument:
      return "extra_argument";
    case ErrorCode::InvalidArgument:
      return "invalid_argument";
    case ErrorCode::OutOfRange:
      return "out_of_range";
    case ErrorCode::UnsupportedCommand:
      return "unsupported_command";
    case ErrorCode::UnsupportedTarget:
      return "unsupported_target";
    case ErrorCode::UnsupportedLcdDepth:
      return "unsupported_lcd_depth";
    case ErrorCode::KeyAlreadyDown:
      return "key_already_down";
    case ErrorCode::KeyNotDown:
      return "key_not_down";
    case ErrorCode::TouchAlreadyDown:
      return "touch_already_down";
    case ErrorCode::TouchNotDown:
      return "touch_not_down";
    case ErrorCode::QueueFull:
      return "queue_full";
    case ErrorCode::FirmwareQueueFull:
      return "firmware_queue_full";
    case ErrorCode::OperationBusy:
      return "operation_busy";
    case ErrorCode::ResponseTooLarge:
      return "response_too_large";
    case ErrorCode::SessionStopping:
      return "session_stopping";
    case ErrorCode::RestartFailed:
      return "restart_failed";
    case ErrorCode::LuaUnavailable:
      return "lua_unavailable";
    case ErrorCode::LuaPanic:
      return "lua_panic";
    case ErrorCode::UnsafePath:
      return "unsafe_path";
    case ErrorCode::PathTooLong:
      return "path_too_long";
    case ErrorCode::ArtifactExists:
      return "artifact_exists";
    case ErrorCode::CaptureFailed:
      return "capture_failed";
    case ErrorCode::CaptureCancelled:
      return "capture_cancelled";
    case ErrorCode::StdinClosed:
      return "stdin_closed";
    case ErrorCode::StdoutClosed:
      return "stdout_closed";
    case ErrorCode::IoError:
      return "io_error";
    case ErrorCode::InvariantViolation:
      return "invariant_violation";
    case ErrorCode::InternalError:
      return "internal_error";
  }
  return "internal_error";
}

const char* sessionPhaseName(SessionPhase phase)
{
  switch (phase) {
    case SessionPhase::Starting:
      return "starting";
    case SessionPhase::Ready:
      return "ready";
    case SessionPhase::Restarting:
      return "restarting";
    case SessionPhase::Stopped:
      return "stopped";
  }
  return "stopped";
}

const char* asyncOperationName(AsyncOperation operation)
{
  switch (operation) {
    case AsyncOperation::None:
      return "none";
    case AsyncOperation::WaitFrame:
      return "wait_frame";
    case AsyncOperation::Capture:
      return "capture";
    case AsyncOperation::Firmware:
      return "firmware";
    case AsyncOperation::ReloadLua:
      return "reload_lua";
    case AsyncOperation::Restart:
      return "restart";
  }
  return "none";
}

bool isValidUtf8(const std::string& value)
{
  std::size_t index = 0;
  while (index < value.size()) {
    const unsigned char first = static_cast<unsigned char>(value[index]);
    if (first <= 0x7f) {
      ++index;
      continue;
    }

    std::size_t continuationCount = 0;
    std::uint32_t codePoint = 0;
    if (first >= 0xc2 && first <= 0xdf) {
      continuationCount = 1;
      codePoint = first & 0x1f;
    } else if (first >= 0xe0 && first <= 0xef) {
      continuationCount = 2;
      codePoint = first & 0x0f;
    } else if (first >= 0xf0 && first <= 0xf4) {
      continuationCount = 3;
      codePoint = first & 0x07;
    } else {
      return false;
    }

    if (index + continuationCount >= value.size()) return false;
    for (std::size_t continuation = 1; continuation <= continuationCount;
         ++continuation) {
      const unsigned char byte =
          static_cast<unsigned char>(value[index + continuation]);
      if ((byte & 0xc0) != 0x80) return false;
      codePoint = (codePoint << 6) | (byte & 0x3f);
    }

    if ((continuationCount == 2 && codePoint < 0x800) ||
        (continuationCount == 3 && codePoint < 0x10000) ||
        (codePoint >= 0xd800 && codePoint <= 0xdfff) || codePoint > 0x10ffff) {
      return false;
    }
    index += continuationCount + 1;
  }
  return true;
}

TerminalResponseOwner::TerminalResponseOwner(RequestId requestId,
                                             SessionEpoch epoch) :
    id(requestId), ownerEpoch(epoch)
{
}

TerminalClaimResult TerminalResponseOwner::claim(SessionEpoch currentEpoch)
{
  if (terminal) return TerminalClaimResult::Duplicate;
  if (currentEpoch != ownerEpoch) return TerminalClaimResult::StaleEpoch;
  terminal = true;
  return TerminalClaimResult::Claimed;
}

TerminalClaimResult TerminalResponseOwner::cancel(SessionEpoch currentEpoch)
{
  return claim(currentEpoch);
}

bool TerminalResponseOwner::isTerminal() const { return terminal; }

RequestId TerminalResponseOwner::requestId() const { return id; }

SessionState::SessionState() = default;

RequestId SessionState::onDisplayFrame()
{
  if (currentPhase == SessionPhase::Stopped) return 0;
  if (currentDisplaySequence != (std::numeric_limits<DisplaySequence>::max)()) {
    ++currentDisplaySequence;
  }
  bool becameReady = false;
  if (currentPhase == SessionPhase::Starting) {
    if (currentEpoch == 0) currentEpoch = 1;
    currentPhase = SessionPhase::Ready;
    becameReady = true;
  }

  if (becameReady && activeOperation == AsyncOperation::Restart &&
      activeRequestEpoch == currentEpoch) {
    const RequestId completedId = activeRequestId;
    lastTerminalAsyncRequestId = completedId;
    clearAsync();
    return completedId;
  }
  return 0;
}

TransitionResult SessionState::keyDown(const std::string& key)
{
  if (currentPhase != SessionPhase::Ready || key.empty())
    return TransitionResult::InvalidState;
  if (activeOperation != AsyncOperation::None) return TransitionResult::Busy;
  if (activeKeys.count(key) != 0) return TransitionResult::Duplicate;
  activeKeys.insert(key);
  return TransitionResult::Applied;
}

TransitionResult SessionState::keyUp(const std::string& key)
{
  if (currentPhase != SessionPhase::Ready)
    return TransitionResult::InvalidState;
  if (activeOperation != AsyncOperation::None) return TransitionResult::Busy;
  const auto keyIterator = activeKeys.find(key);
  if (keyIterator == activeKeys.end()) return TransitionResult::InvalidState;
  activeKeys.erase(keyIterator);
  return TransitionResult::Applied;
}

TransitionResult SessionState::touchDown(std::uint16_t x, std::uint16_t y)
{
  if (currentPhase != SessionPhase::Ready)
    return TransitionResult::InvalidState;
  if (activeOperation != AsyncOperation::None) return TransitionResult::Busy;
  if (touchActive) return TransitionResult::Duplicate;
  touchActive = true;
  touchX = x;
  touchY = y;
  return TransitionResult::Applied;
}

TransitionResult SessionState::touchMove(std::uint16_t x, std::uint16_t y)
{
  if (currentPhase != SessionPhase::Ready)
    return TransitionResult::InvalidState;
  if (activeOperation != AsyncOperation::None) return TransitionResult::Busy;
  if (!touchActive) return TransitionResult::InvalidState;
  touchX = x;
  touchY = y;
  return TransitionResult::Applied;
}

TransitionResult SessionState::touchUp()
{
  if (currentPhase != SessionPhase::Ready)
    return TransitionResult::InvalidState;
  if (activeOperation != AsyncOperation::None) return TransitionResult::Busy;
  if (!touchActive) return TransitionResult::InvalidState;
  touchActive = false;
  return TransitionResult::Applied;
}

void SessionState::releaseAll()
{
  activeKeys.clear();
  touchActive = false;
}

TransitionResult SessionState::enqueue(RequestId id)
{
  if (currentPhase == SessionPhase::Stopped || id == 0)
    return TransitionResult::InvalidState;
  if (pendingRequests.size() == MAX_PENDING_REQUESTS)
    return TransitionResult::QueueFull;
  pendingRequests.push_back(id);
  return TransitionResult::Applied;
}

bool SessionState::dequeue(RequestId* id)
{
  if (id == nullptr || pendingRequests.empty()) return false;
  *id = pendingRequests.front();
  pendingRequests.pop_front();
  return true;
}

TransitionResult SessionState::beginAsync(AsyncOperation operation,
                                          RequestId id)
{
  if (currentPhase != SessionPhase::Ready ||
      operation == AsyncOperation::None || id == 0) {
    return TransitionResult::InvalidState;
  }
  if (activeOperation != AsyncOperation::None) return TransitionResult::Busy;

  activeOperation = operation;
  activeRequestId = id;
  activeRequestEpoch = currentEpoch;
  if (operation == AsyncOperation::Restart)
    currentPhase = SessionPhase::Restarting;
  return TransitionResult::Applied;
}

TransitionResult SessionState::completeAsync(RequestId id,
                                             SessionEpoch completionEpoch)
{
  if (completionEpoch != currentEpoch) return TransitionResult::StaleEpoch;
  if (activeOperation == AsyncOperation::None) {
    return id == lastTerminalAsyncRequestId ? TransitionResult::Duplicate
                                            : TransitionResult::NotPending;
  }
  if (activeOperation == AsyncOperation::Restart)
    return TransitionResult::InvalidState;
  if (id != activeRequestId) return TransitionResult::NotPending;
  if (completionEpoch != activeRequestEpoch)
    return TransitionResult::StaleEpoch;

  lastTerminalAsyncRequestId = id;
  clearAsync();
  return TransitionResult::Applied;
}

TransitionResult SessionState::cancelAsync()
{
  if (activeOperation == AsyncOperation::None)
    return TransitionResult::NotPending;
  if (currentPhase == SessionPhase::Restarting)
    currentPhase = SessionPhase::Ready;
  lastTerminalAsyncRequestId = activeRequestId;
  clearAsync();
  return TransitionResult::Applied;
}

TransitionResult SessionState::restartTasksStarted(RequestId id,
                                                   SessionEpoch requestEpoch)
{
  if (currentPhase != SessionPhase::Restarting ||
      activeOperation != AsyncOperation::Restart || id != activeRequestId)
    return TransitionResult::NotPending;
  if (requestEpoch != currentEpoch || requestEpoch != activeRequestEpoch)
    return TransitionResult::StaleEpoch;

  releaseAll();
  pendingRequests.clear();
  currentEpoch = currentEpoch == 0 ? 1 : currentEpoch + 1;
  activeRequestEpoch = currentEpoch;
  currentPhase = SessionPhase::Starting;
  return TransitionResult::Applied;
}

void SessionState::stop()
{
  if (activeOperation != AsyncOperation::None)
    lastTerminalAsyncRequestId = activeRequestId;
  clearAsync();
  releaseAll();
  pendingRequests.clear();
  currentPhase = SessionPhase::Stopped;
}

SessionPhase SessionState::phase() const { return currentPhase; }

SessionEpoch SessionState::epoch() const { return currentEpoch; }

DisplaySequence SessionState::displaySequence() const
{
  return currentDisplaySequence;
}

AsyncOperation SessionState::asyncOperation() const { return activeOperation; }

std::size_t SessionState::activeKeyCount() const { return activeKeys.size(); }

std::vector<std::string> SessionState::activeKeyNames() const
{
  return std::vector<std::string>(activeKeys.begin(), activeKeys.end());
}

bool SessionState::isTouchActive() const { return touchActive; }

std::size_t SessionState::queuedRequestCount() const
{
  return pendingRequests.size();
}

void SessionState::clearAsync()
{
  activeOperation = AsyncOperation::None;
  activeRequestId = 0;
  activeRequestEpoch = 0;
}

}  // namespace automation
}  // namespace edgetx
