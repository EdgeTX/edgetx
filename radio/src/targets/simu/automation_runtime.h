/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#pragma once

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

#include "dataconstants.h"

namespace edgetx
{
namespace automation
{

constexpr std::size_t FIRMWARE_MAILBOX_CAPACITY = 16;

enum class FirmwareOperation : std::uint8_t {
  None,
  Telemetry,
  ReloadLua,
};

enum class FirmwareCompletionCode : std::uint8_t {
  None,
  TelemetryUnavailable,
  LuaUnavailable,
  LuaPanic,
  InternalError,
};

enum class AutomationLuaState : std::uint8_t {
  Unavailable,
  NotObserved,
  Idle,
  Reloading,
  Running,
  Panic,
};

struct FirmwareRequest {
  FirmwareOperation operation = FirmwareOperation::None;
  std::uint64_t id = 0;
  std::uint64_t epoch = 0;
  std::uint64_t generation = 0;
  std::uint16_t telemetryId = 0;
  std::uint8_t telemetrySubId = 0;
  std::uint8_t telemetryInstance = 0;
  std::int32_t telemetryValue = 0;
  std::uint8_t telemetryUnit = 0;
  std::uint8_t telemetryPrecision = 0;
  char telemetryName[TELEM_LABEL_LEN + 1] = {};
};

struct FirmwareCompletion {
  FirmwareOperation operation = FirmwareOperation::None;
  FirmwareCompletionCode code = FirmwareCompletionCode::None;
  std::uint64_t id = 0;
  std::uint64_t epoch = 0;
  std::uint64_t generation = 0;
  std::int32_t telemetryIndex = -1;
  AutomationLuaState luaState = AutomationLuaState::NotObserved;

  bool ok() const { return code == FirmwareCompletionCode::None; }
};

class AutomationFirmwareMailbox
{
 public:
  AutomationFirmwareMailbox();

  bool enqueueRequest(const FirmwareRequest& request);
  bool dequeueRequest(FirmwareRequest* request);
  bool enqueueCompletion(const FirmwareCompletion& completion);
  bool dequeueCompletion(FirmwareCompletion* completion);
  std::size_t requestDepth() const;
  std::size_t completionDepth() const;
  bool idle() const;
  void reset();

 private:
  static constexpr std::size_t STORAGE_SIZE = FIRMWARE_MAILBOX_CAPACITY + 1;

  std::array<FirmwareRequest, STORAGE_SIZE> requests{};
  std::array<FirmwareCompletion, STORAGE_SIZE> completions{};
  std::atomic<std::size_t> requestRead{0};
  std::atomic<std::size_t> requestWrite{0};
  std::atomic<std::size_t> completionRead{0};
  std::atomic<std::size_t> completionWrite{0};
};

class AutomationAnalogOverrides
{
 public:
  AutomationAnalogOverrides();

  bool set(std::size_t index, std::uint16_t value);
  bool get(std::size_t index, std::uint16_t* value) const;
  bool clear(std::size_t index);
  void clearAll();
  std::size_t count() const;

 private:
  static constexpr std::uint32_t ENABLED = UINT32_C(0x80000000);
  static constexpr std::uint32_t VALUE_MASK = UINT32_C(0x00001fff);

  std::array<std::atomic<std::uint32_t>, MAX_ANALOG_INPUTS> values{};
};

void simuAutomationRuntimeStart();
void simuAutomationRuntimeStop();
void simuAutomationRuntimeResetAfterTaskJoin();
bool simuAutomationRuntimeActive();

bool simuAutomationPostFirmwareRequest(const FirmwareRequest& request);
bool simuAutomationTakeFirmwareCompletion(FirmwareCompletion* completion);
std::size_t simuAutomationFirmwareRequestDepth();
std::size_t simuAutomationFirmwareCompletionDepth();
bool simuAutomationFirmwareIdle();

bool simuAutomationSetAnalogOverride(std::size_t index, std::uint16_t value);
bool simuAutomationGetAnalogOverride(std::size_t index, std::uint16_t* value);
bool simuAutomationClearAnalogOverride(std::size_t index);
void simuAutomationClearAnalogOverrides();
std::size_t simuAutomationAnalogOverrideCount();

AutomationLuaState simuAutomationLuaState();
const char* automationLuaStateName(AutomationLuaState state);

// These hooks run only in the firmware periodic context.
void simuAutomationBeforeUi();
void simuAutomationAfterUi();

}  // namespace automation
}  // namespace edgetx
