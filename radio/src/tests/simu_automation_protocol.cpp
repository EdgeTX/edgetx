/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include <limits>

#include "automation_protocol.h"
#include "gtests.h"

using namespace edgetx::automation;

TEST(SimuAutomationLineBuffer, AcceptsLfCrlfBlankAndMultipleRecords)
{
  LineBuffer buffer;
  const std::string bytes = "v1 1 ping\nv1 2 status\r\n\n";
  const auto events = buffer.feed(bytes.data(), bytes.size());

  ASSERT_EQ(events.size(), 3u);
  EXPECT_EQ(events[0].type, LineEventType::Record);
  EXPECT_EQ(events[0].record, "v1 1 ping");
  EXPECT_EQ(events[1].record, "v1 2 status");
  EXPECT_TRUE(events[2].record.empty());
  EXPECT_TRUE(buffer.finish().empty());
}

TEST(SimuAutomationLineBuffer, PreservesUtf8AtEveryFeedSplit)
{
  const std::string record = std::string("v1 1 capture checkpoints/") +
                             std::string("\xc3\xa9\xe5\xb1\x8f.ppm\n", 10);

  for (std::size_t split = 0; split <= record.size(); ++split) {
    LineBuffer buffer;
    auto first = buffer.feed(record.data(), split);
    auto second = buffer.feed(record.data() + split, record.size() - split);

    ASSERT_EQ(first.size() + second.size(), 1u) << "split=" << split;
    const LineEvent& event = first.empty() ? second[0] : first[0];
    EXPECT_EQ(event.type, LineEventType::Record) << "split=" << split;
    EXPECT_EQ(event.record, record.substr(0, record.size() - 1))
        << "split=" << split;
  }
}

TEST(SimuAutomationLineBuffer, EnforcesLimitAndRecoversAfterNewline)
{
  LineBuffer buffer(8);
  auto exact = buffer.feed("12345678\r\n", 10);
  ASSERT_EQ(exact.size(), 1u);
  EXPECT_EQ(exact[0].type, LineEventType::Record);
  EXPECT_EQ(exact[0].record, "12345678");

  const std::string overflowAndRecovery = "123456789 ignored\nok\n";
  auto events =
      buffer.feed(overflowAndRecovery.data(), overflowAndRecovery.size());
  ASSERT_EQ(events.size(), 2u);
  EXPECT_EQ(events[0].type, LineEventType::LineTooLong);
  EXPECT_EQ(events[1].type, LineEventType::Record);
  EXPECT_EQ(events[1].record, "ok");
  EXPECT_LE(buffer.bufferedBytes(), 8u);
}

TEST(SimuAutomationLineBuffer, ReportsOnlyPartialOrOverflowAtEof)
{
  LineBuffer empty;
  EXPECT_TRUE(empty.finish().empty());

  LineBuffer partial;
  partial.feed("v1 1 ping", 9);
  auto partialEvent = partial.finish();
  ASSERT_EQ(partialEvent.size(), 1u);
  EXPECT_EQ(partialEvent[0].type, LineEventType::PartialRecordAtEof);
  EXPECT_EQ(partialEvent[0].record, "v1 1 ping");

  LineBuffer overflow(4);
  overflow.feed("12345", 5);
  auto overflowEvent = overflow.finish();
  ASSERT_EQ(overflowEvent.size(), 1u);
  EXPECT_EQ(overflowEvent[0].type, LineEventType::LineTooLong);
}

TEST(SimuAutomationParser, AcceptsVersionOneCommandSet)
{
  const char* records[] = {
      "v1 1 ping",
      "v1 2 status",
      "v1 3 describe",
      "v1 4 key-down ENTER",
      "v1 5 key-up ENTER",
      "v1 6 rotate -2",
      "v1 7 touch-down 120 80",
      "v1 8 touch-move 130 82",
      "v1 9 touch-up",
      "v1 10 set-switch SA 1",
      "v1 11 set-analog AIL 2048",
      "v1 12 clear-analog AIL",
      "v1 13 set-telemetry 61696 0 1 115 1 1 RSSI",
      "v1 14 reload-lua",
      "v1 15 wait-frame 185",
      "v1 16 capture checkpoints/home screen.ppm",
      "v1 17 restart",
      "v1 18 release-all",
      "v1 19 stop",
  };

  ProtocolParser parser;
  for (const char* record : records) {
    const ParseResult result = parser.parse(record);
    EXPECT_EQ(result.status, ParseStatus::Request) << record;
  }
  EXPECT_EQ(parser.lastRequestId(), 19u);
}

TEST(SimuAutomationParser, IgnoresBlankAndPreservesCaptureRemainder)
{
  ProtocolParser parser;
  EXPECT_EQ(parser.parse("").status, ParseStatus::Ignored);

  const ParseResult result =
      parser.parse("v1 1 capture checkpoints/home  screen.ppm");
  ASSERT_EQ(result.status, ParseStatus::Request);
  ASSERT_EQ(result.request.arguments.size(), 1u);
  EXPECT_EQ(result.request.arguments[0], "checkpoints/home  screen.ppm");
}

TEST(SimuAutomationParser, ValidatesVersionAndDecimalRequestId)
{
  ProtocolParser parser;
  ParseResult result = parser.parse("v2 1 ping");
  EXPECT_EQ(result.error.code, ErrorCode::UnsupportedVersion);
  EXPECT_TRUE(result.error.hasRequestId);
  EXPECT_EQ(parser.lastRequestId(), 0u);

  EXPECT_EQ(parser.parse("v1").error.code, ErrorCode::InvalidId);
  EXPECT_EQ(parser.parse("v1 0 ping").error.code, ErrorCode::InvalidId);
  EXPECT_EQ(parser.parse("v1 +1 ping").error.code, ErrorCode::InvalidId);
  EXPECT_EQ(parser.parse("v1 18446744073709551616 ping").error.code,
            ErrorCode::InvalidId);

  const ParseResult maximum = parser.parse("v1 18446744073709551615 ping");
  EXPECT_EQ(maximum.status, ParseStatus::Request);
  EXPECT_EQ(maximum.request.id, std::numeric_limits<std::uint64_t>::max());
}

TEST(SimuAutomationParser, ConsumesRecoveredIdAfterCommandFailure)
{
  ProtocolParser parser;
  EXPECT_EQ(parser.parse("v1 10 not-a-command").error.code,
            ErrorCode::UnknownCommand);
  EXPECT_EQ(parser.lastRequestId(), 10u);
  EXPECT_EQ(parser.parse("v1 10 ping").error.code, ErrorCode::IdNotMonotonic);
  EXPECT_EQ(parser.parse("v1 11 ping").status, ParseStatus::Request);
}

TEST(SimuAutomationParser, EnforcesArityAndNumericRanges)
{
  ProtocolParser parser;
  EXPECT_EQ(parser.parse("v1 1 key-down").error.code,
            ErrorCode::MissingArgument);
  EXPECT_EQ(parser.parse("v1 2 ping extra").error.code,
            ErrorCode::ExtraArgument);
  EXPECT_EQ(parser.parse("v1 3 rotate 0").error.code,
            ErrorCode::InvalidArgument);
  EXPECT_EQ(parser.parse("v1 4 rotate -129").error.code, ErrorCode::OutOfRange);
  EXPECT_EQ(parser.parse("v1 5 touch-down -1 2").error.code,
            ErrorCode::OutOfRange);
  EXPECT_EQ(parser.parse("v1 6 set-analog AIL 4097").error.code,
            ErrorCode::OutOfRange);
  EXPECT_EQ(parser.parse("v1 7 wait-frame 18446744073709551616").error.code,
            ErrorCode::OutOfRange);
  EXPECT_EQ(parser.parse("v1 8 ping ").error.code, ErrorCode::ExtraArgument);
}

TEST(SimuAutomationParser, RejectsMalformedSeparatorsNulAndInvalidUtf8)
{
  ProtocolParser parser;
  EXPECT_EQ(parser.parse(" v1 1 ping").error.code, ErrorCode::InvalidRecord);
  EXPECT_EQ(parser.parse("v1 1  ping").error.code, ErrorCode::InvalidRecord);

  std::string withNul("v1 2 capture bad", 16);
  withNul.push_back('\0');
  withNul += ".ppm";
  EXPECT_EQ(parser.parse(withNul).error.code, ErrorCode::InvalidRecord);

  std::string invalidUtf8 = "v1 3 capture bad";
  invalidUtf8.append("\xc3\x28", 2);
  invalidUtf8 += ".ppm";
  EXPECT_EQ(parser.parse(invalidUtf8).error.code, ErrorCode::InvalidUtf8);
}

TEST(SimuAutomationParser, AcceptsValidUtf8CapturePath)
{
  ProtocolParser parser;
  std::string record = "v1 1 capture screenshots/";
  record.append("\xc3\xa9\xe5\xb1\x8f", 5);
  record += ".ppm";
  const ParseResult result = parser.parse(record);
  EXPECT_EQ(result.status, ParseStatus::Request);
  EXPECT_TRUE(isValidUtf8(result.request.arguments[0]));

  std::string telemetry = "v1 2 set-telemetry 61696 0 1 115 1 1 ";
  telemetry.append("\xc3\xa9", 2);
  EXPECT_EQ(parser.parse(telemetry).status, ParseStatus::Request);
}

TEST(SimuAutomationResponse, SerializesSuccessAndEscapesEveryControlByte)
{
  std::string successJson;
  EXPECT_EQ(serializeResponse(Response::success(7, 2), &successJson),
            SerializeResult::Serialized);
  EXPECT_EQ(successJson,
            "{\"version\":1,\"type\":\"response\",\"id\":7,\"ok\":true,"
            "\"epoch\":2}\n");

  Response invalidFailure = Response::failure(7, 2, ErrorCode::None, "bad");
  EXPECT_EQ(serializeResponse(invalidFailure, &successJson),
            SerializeResult::Serialized);
  EXPECT_NE(successJson.find("\"code\":\"internal_error\""), std::string::npos);

  std::string message;
  for (int byte = 0; byte < 0x20; ++byte)
    message.push_back(static_cast<char>(byte));
  message += "\"\\";
  message.append("\xc3\xa9", 2);

  std::string json;
  EXPECT_EQ(
      serializeResponse(
          Response::failure(8, 2, ErrorCode::InvalidArgument, message), &json),
      SerializeResult::Serialized);
  ASSERT_FALSE(json.empty());
  EXPECT_TRUE(isValidUtf8(json));
  EXPECT_NE(json.find("\\u0000"), std::string::npos);
  EXPECT_NE(json.find("\\\""), std::string::npos);
  EXPECT_NE(json.find("\\\\"), std::string::npos);
  EXPECT_NE(json.find("\xc3\xa9"), std::string::npos);

  for (std::size_t index = 0; index + 1 < json.size(); ++index) {
    EXPECT_GE(static_cast<unsigned char>(json[index]), 0x20u);
  }
  EXPECT_EQ(json.back(), '\n');
}

TEST(SimuAutomationResponse, FallsBackWithinResponseLimit)
{
  const std::string oversized(MAX_RESPONSE_BYTES, 'x');
  std::string json;
  EXPECT_EQ(
      serializeResponse(
          Response::failure(9, 3, ErrorCode::InternalError, oversized), &json),
      SerializeResult::UsedSizeFallback);
  EXPECT_LE(json.size(), MAX_RESPONSE_BYTES);
  EXPECT_NE(json.find("\"code\":\"response_too_large\""), std::string::npos);

  EXPECT_EQ(serializeResponse(Response::success(1, 1), &json, 4),
            SerializeResult::LimitTooSmall);
  EXPECT_TRUE(json.empty());
}

TEST(SimuAutomationResponse, SerializesBoundedStatusAndDescriptionResults)
{
  TargetDescription target;
  target.flavour = "tx\"16s";
  target.lcdWidth = 480;
  target.lcdHeight = 272;
  target.lcdDepth = 16;
  target.commands = {Command::Ping, Command::Status, Command::Describe,
                     Command::Stop};
  target.capabilities.capture = true;
  target.keys = {"ENTER"};
  target.switches = {{"SA", -1, 1}};
  target.analogs = {{"AIL", 0, 4096}};
  target.outputRootReady = true;

  StatusSnapshot status;
  status.running = true;
  status.phase = SessionPhase::Ready;
  status.displaySequence = 17;
  status.requestQueueDepth = 2;
  status.lineOverflowCount = 3;
  status.touchActive = true;

  std::string json;
  EXPECT_EQ(serializeResponse(Response::successWithStatus(7, 1, status, target),
                              &json),
            SerializeResult::Serialized);
  EXPECT_NE(json.find("\"phase\":\"ready\""), std::string::npos);
  EXPECT_NE(json.find("\"target\":\"tx\\\"16s\""), std::string::npos);
  EXPECT_NE(json.find("\"display_seq\":17"), std::string::npos);
  EXPECT_NE(json.find("\"request_queue_depth\":2"), std::string::npos);
  EXPECT_NE(json.find("\"line_overflow_count\":3"), std::string::npos);
  EXPECT_NE(json.find("\"capture\":true"), std::string::npos);
  EXPECT_NE(json.find("\"output_root\":\"ready\""), std::string::npos);

  EXPECT_EQ(
      serializeResponse(Response::successWithDescription(8, 1, target), &json),
      SerializeResult::Serialized);
  EXPECT_NE(
      json.find("\"commands\":[\"ping\",\"status\",\"describe\",\"stop\"]"),
      std::string::npos);
  EXPECT_NE(json.find("\"keys\":[\"ENTER\"]"), std::string::npos);
  EXPECT_NE(json.find("{\"name\":\"SA\",\"min\":-1,\"max\":1}"),
            std::string::npos);
  EXPECT_NE(json.find("{\"name\":\"AIL\",\"min\":0,\"max\":4096}"),
            std::string::npos);
  EXPECT_LE(json.size(), MAX_RESPONSE_BYTES);
}

TEST(SimuAutomationResponse, DescriptionOverflowUsesTerminalFallback)
{
  TargetDescription target;
  target.flavour = "test";
  target.lcdWidth = 128;
  target.lcdHeight = 64;
  target.lcdDepth = 1;
  target.commands = {Command::Ping, Command::Status, Command::Describe,
                     Command::Stop};
  target.keys = {std::string(MAX_RESPONSE_BYTES, 'x')};

  std::string json;
  EXPECT_EQ(
      serializeResponse(Response::successWithDescription(1, 0, target), &json),
      SerializeResult::UsedSizeFallback);
  EXPECT_NE(json.find("\"code\":\"response_too_large\""), std::string::npos);
  EXPECT_LE(json.size(), MAX_RESPONSE_BYTES);
}

TEST(SimuAutomationResponse, SerializesUncorrelatedEvents)
{
  std::string json;
  EXPECT_EQ(
      serializeEvent(1, ErrorCode::InvalidRecord, "bad \"record\"", &json),
      SerializeResult::Serialized);
  EXPECT_EQ(json,
            "{\"version\":1,\"type\":\"event\",\"id\":null,\"epoch\":1,"
            "\"event\":{\"code\":\"invalid_record\",\"message\":\"bad "
            "\\\"record\\\"\"}}\n");
}

TEST(SimuAutomationTerminalResponse, HasOneOwnerAndRejectsDuplicates)
{
  TerminalResponseOwner owner(42, 3);
  EXPECT_EQ(owner.requestId(), 42u);
  EXPECT_EQ(owner.claim(2), TerminalClaimResult::StaleEpoch);
  EXPECT_FALSE(owner.isTerminal());
  EXPECT_EQ(owner.claim(3), TerminalClaimResult::Claimed);
  EXPECT_TRUE(owner.isTerminal());
  EXPECT_EQ(owner.cancel(3), TerminalClaimResult::Duplicate);
}

TEST(SimuAutomationSessionState, EnforcesKeyAndTouchTransitions)
{
  SessionState state;
  EXPECT_EQ(state.keyDown("ENTER"), TransitionResult::InvalidState);
  state.onDisplayFrame();
  EXPECT_EQ(state.phase(), SessionPhase::Ready);
  EXPECT_EQ(state.epoch(), 1u);

  EXPECT_EQ(state.keyDown("ENTER"), TransitionResult::Applied);
  EXPECT_EQ(state.keyDown("ENTER"), TransitionResult::Duplicate);
  EXPECT_EQ(state.keyUp("EXIT"), TransitionResult::InvalidState);
  EXPECT_EQ(state.keyUp("ENTER"), TransitionResult::Applied);

  EXPECT_EQ(state.touchMove(1, 2), TransitionResult::InvalidState);
  EXPECT_EQ(state.touchDown(1, 2), TransitionResult::Applied);
  EXPECT_EQ(state.touchDown(2, 3), TransitionResult::Duplicate);
  EXPECT_EQ(state.touchMove(2, 3), TransitionResult::Applied);
  EXPECT_EQ(state.touchUp(), TransitionResult::Applied);
  EXPECT_EQ(state.touchUp(), TransitionResult::InvalidState);
}

TEST(SimuAutomationSessionState, AllowsOnlyOneAsyncOperationAndOneCompletion)
{
  SessionState state;
  state.onDisplayFrame();
  EXPECT_EQ(state.beginAsync(AsyncOperation::WaitFrame, 1),
            TransitionResult::Applied);
  EXPECT_EQ(state.keyDown("ENTER"), TransitionResult::Busy);
  EXPECT_EQ(state.beginAsync(AsyncOperation::Capture, 2),
            TransitionResult::Busy);
  EXPECT_EQ(state.completeAsync(2, 1), TransitionResult::NotPending);
  EXPECT_EQ(state.completeAsync(1, 1), TransitionResult::Applied);
  EXPECT_EQ(state.completeAsync(1, 1), TransitionResult::Duplicate);
}

TEST(SimuAutomationSessionState, CancellationAndStopReleaseOwnedState)
{
  SessionState state;
  state.onDisplayFrame();
  state.keyDown("ENTER");
  state.touchDown(10, 10);
  state.enqueue(2);
  state.beginAsync(AsyncOperation::Firmware, 1);

  EXPECT_EQ(state.cancelAsync(), TransitionResult::Applied);
  EXPECT_EQ(state.completeAsync(1, 1), TransitionResult::Duplicate);
  state.stop();

  EXPECT_EQ(state.phase(), SessionPhase::Stopped);
  EXPECT_EQ(state.activeKeyCount(), 0u);
  EXPECT_FALSE(state.isTouchActive());
  EXPECT_EQ(state.queuedRequestCount(), 0u);
  EXPECT_EQ(state.asyncOperation(), AsyncOperation::None);
  EXPECT_EQ(state.keyDown("ENTER"), TransitionResult::InvalidState);
}

TEST(SimuAutomationSessionState, RestartAdvancesEpochAndPurgesOldWork)
{
  SessionState state;
  state.onDisplayFrame();
  state.onDisplayFrame();
  EXPECT_EQ(state.displaySequence(), 2u);
  state.keyDown("ENTER");
  state.enqueue(2);
  EXPECT_EQ(state.beginAsync(AsyncOperation::Restart, 1),
            TransitionResult::Applied);
  EXPECT_EQ(state.phase(), SessionPhase::Restarting);
  EXPECT_EQ(state.onDisplayFrame(), 0u);
  EXPECT_EQ(state.phase(), SessionPhase::Restarting);
  EXPECT_EQ(state.asyncOperation(), AsyncOperation::Restart);

  EXPECT_EQ(state.restartTasksStarted(1, 1), TransitionResult::Applied);
  EXPECT_EQ(state.epoch(), 2u);
  EXPECT_EQ(state.phase(), SessionPhase::Starting);
  EXPECT_EQ(state.displaySequence(), 0u);
  EXPECT_EQ(state.activeKeyCount(), 0u);
  EXPECT_EQ(state.queuedRequestCount(), 0u);
  EXPECT_EQ(state.completeAsync(1, 1), TransitionResult::StaleEpoch);

  EXPECT_EQ(state.onDisplayFrame(), 1u);
  EXPECT_EQ(state.phase(), SessionPhase::Ready);
  EXPECT_EQ(state.epoch(), 2u);
  EXPECT_EQ(state.displaySequence(), 1u);
  EXPECT_EQ(state.asyncOperation(), AsyncOperation::None);
}

TEST(SimuAutomationSessionState, BoundsPendingQueue)
{
  SessionState state;
  state.onDisplayFrame();
  for (std::size_t index = 0; index < MAX_PENDING_REQUESTS; ++index) {
    EXPECT_EQ(state.enqueue(index + 1), TransitionResult::Applied);
  }
  EXPECT_EQ(state.enqueue(MAX_PENDING_REQUESTS + 1),
            TransitionResult::QueueFull);

  RequestId id = 0;
  EXPECT_TRUE(state.dequeue(&id));
  EXPECT_EQ(id, 1u);
  EXPECT_EQ(state.queuedRequestCount(), MAX_PENDING_REQUESTS - 1);
}
