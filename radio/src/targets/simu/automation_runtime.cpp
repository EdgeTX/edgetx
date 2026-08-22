/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include "automation_runtime.h"

#include <algorithm>
#include <cstring>

#include "edgetx.h"
#include "simulib.h"

#if defined(LUA)
#include "lua/lua_api.h"
#endif

namespace edgetx
{
namespace automation
{
namespace
{

static_assert(std::atomic<std::uint32_t>::is_always_lock_free,
              "simulator analog overrides require lock-free 32-bit atomics");

AutomationFirmwareMailbox firmwareMailbox;
AutomationAnalogOverrides analogOverrides;
std::atomic<bool> runtimeActive{false};
std::atomic<AutomationLuaState> publishedLuaState{
#if defined(LUA)
    AutomationLuaState::NotObserved
#else
    AutomationLuaState::Unavailable
#endif
};

// Accessed only by the firmware periodic context, or by the SDL owner after
// simuStop() has joined every firmware task.
bool deferredCompletionActive = false;
FirmwareCompletion deferredCompletion;
bool luaReloadActive = false;
FirmwareRequest luaReloadRequest;

std::size_t nextIndex(std::size_t index)
{
  return (index + 1) % (FIRMWARE_MAILBOX_CAPACITY + 1);
}

std::size_t queueDepth(std::size_t read, std::size_t write)
{
  return write >= read ? write - read
                       : FIRMWARE_MAILBOX_CAPACITY + 1 - read + write;
}

AutomationLuaState observeLuaState()
{
#if defined(LUA)
  switch (luaState) {
    case 0:
      return AutomationLuaState::Idle;
    case INTERPRETER_RELOAD_PERMANENT_SCRIPTS:
    case INTERPRETER_LOADING:
    case INTERPRETER_START_RUNNING:
      return AutomationLuaState::Reloading;
    case INTERPRETER_RUNNING:
      return AutomationLuaState::Running;
    case INTERPRETER_PANIC:
      return AutomationLuaState::Panic;
    default:
      return AutomationLuaState::NotObserved;
  }
#else
  return AutomationLuaState::Unavailable;
#endif
}

FirmwareCompletion processTelemetry(const FirmwareRequest& request)
{
  FirmwareCompletion completion;
  completion.operation = request.operation;
  completion.id = request.id;
  completion.epoch = request.epoch;
  completion.generation = request.generation;

  int existingIndex = -1;
  for (int index = 0; index < MAX_TELEMETRY_SENSORS; ++index) {
    const TelemetrySensor& sensor = g_model.telemetrySensors[index];
    if (sensor.type == TELEM_TYPE_CUSTOM && sensor.id == request.telemetryId &&
        sensor.subId == request.telemetrySubId &&
        (sensor.instance == request.telemetryInstance ||
         g_model.ignoreSensorIds)) {
      existingIndex = index;
      break;
    }
  }

  // The interactive discovery toggle must not make an explicit automation
  // command nondeterministic. Preserve the user's setting while allowing the
  // established telemetry path to allocate a sensor when needed.
  const bool discoverSensors = allowNewSensors;
  allowNewSensors = true;
  int index = setTelemetryValue(
      PROTOCOL_TELEMETRY_LUA, request.telemetryId, request.telemetrySubId,
      request.telemetryInstance, request.telemetryValue, request.telemetryUnit,
      request.telemetryPrecision);
  allowNewSensors = discoverSensors;
  if (index < 0 && existingIndex >= 0) index = existingIndex;

  completion.telemetryIndex = index;
  if (index < 0) {
    completion.code = FirmwareCompletionCode::TelemetryUnavailable;
    return completion;
  }

  if (existingIndex < 0) {
    TelemetrySensor& sensor = g_model.telemetrySensors[index];
    sensor.id = request.telemetryId;
    sensor.subId = request.telemetrySubId;
    sensor.instance = request.telemetryInstance;
    sensor.init(request.telemetryName, request.telemetryUnit,
                request.telemetryPrecision);
    storageDirty(EE_MODEL);
  }
  return completion;
}

void deferCompletion(const FirmwareCompletion& completion)
{
  if (deferredCompletionActive) return;
  deferredCompletion = completion;
  deferredCompletionActive = true;
}

void processFirmwareRequest(const FirmwareRequest& request)
{
  if (request.operation == FirmwareOperation::Telemetry) {
    deferCompletion(processTelemetry(request));
    return;
  }

  if (request.operation == FirmwareOperation::ReloadLua) {
    FirmwareCompletion completion;
    completion.operation = request.operation;
    completion.id = request.id;
    completion.epoch = request.epoch;
    completion.generation = request.generation;
#if defined(LUA)
    if (luaReloadActive) {
      completion.code = FirmwareCompletionCode::InternalError;
      deferCompletion(completion);
      return;
    }
    luaReloadRequest = request;
    luaReloadActive = true;
    publishedLuaState.store(AutomationLuaState::Reloading,
                            std::memory_order_release);
    simuLuaReloadPermanentScripts();
#else
    completion.code = FirmwareCompletionCode::LuaUnavailable;
    completion.luaState = AutomationLuaState::Unavailable;
    deferCompletion(completion);
#endif
    return;
  }

  FirmwareCompletion completion;
  completion.operation = request.operation;
  completion.id = request.id;
  completion.epoch = request.epoch;
  completion.generation = request.generation;
  completion.code = FirmwareCompletionCode::InternalError;
  deferCompletion(completion);
}

}  // namespace

AutomationFirmwareMailbox::AutomationFirmwareMailbox() { reset(); }

bool AutomationFirmwareMailbox::enqueueRequest(const FirmwareRequest& request)
{
  const std::size_t write = requestWrite.load(std::memory_order_relaxed);
  const std::size_t next = nextIndex(write);
  if (next == requestRead.load(std::memory_order_acquire)) return false;
  requests[write] = request;
  requestWrite.store(next, std::memory_order_release);
  return true;
}

bool AutomationFirmwareMailbox::dequeueRequest(FirmwareRequest* request)
{
  if (request == nullptr) return false;
  const std::size_t read = requestRead.load(std::memory_order_relaxed);
  if (read == requestWrite.load(std::memory_order_acquire)) return false;
  *request = requests[read];
  requestRead.store(nextIndex(read), std::memory_order_release);
  return true;
}

bool AutomationFirmwareMailbox::enqueueCompletion(
    const FirmwareCompletion& completion)
{
  const std::size_t write = completionWrite.load(std::memory_order_relaxed);
  const std::size_t next = nextIndex(write);
  if (next == completionRead.load(std::memory_order_acquire)) return false;
  completions[write] = completion;
  completionWrite.store(next, std::memory_order_release);
  return true;
}

bool AutomationFirmwareMailbox::dequeueCompletion(
    FirmwareCompletion* completion)
{
  if (completion == nullptr) return false;
  const std::size_t read = completionRead.load(std::memory_order_relaxed);
  if (read == completionWrite.load(std::memory_order_acquire)) return false;
  *completion = completions[read];
  completionRead.store(nextIndex(read), std::memory_order_release);
  return true;
}

std::size_t AutomationFirmwareMailbox::requestDepth() const
{
  return queueDepth(requestRead.load(std::memory_order_acquire),
                    requestWrite.load(std::memory_order_acquire));
}

std::size_t AutomationFirmwareMailbox::completionDepth() const
{
  return queueDepth(completionRead.load(std::memory_order_acquire),
                    completionWrite.load(std::memory_order_acquire));
}

bool AutomationFirmwareMailbox::idle() const
{
  return requestDepth() == 0 && completionDepth() == 0;
}

void AutomationFirmwareMailbox::reset()
{
  requestRead.store(0, std::memory_order_relaxed);
  requestWrite.store(0, std::memory_order_relaxed);
  completionRead.store(0, std::memory_order_relaxed);
  completionWrite.store(0, std::memory_order_relaxed);
}

AutomationAnalogOverrides::AutomationAnalogOverrides() { clearAll(); }

bool AutomationAnalogOverrides::set(std::size_t index, std::uint16_t value)
{
  if (index >= values.size() || value > 4096) return false;
  values[index].store(ENABLED | value, std::memory_order_release);
  return true;
}

bool AutomationAnalogOverrides::get(std::size_t index,
                                    std::uint16_t* value) const
{
  if (index >= values.size() || value == nullptr) return false;
  const std::uint32_t packed = values[index].load(std::memory_order_acquire);
  if ((packed & ENABLED) == 0) return false;
  *value = static_cast<std::uint16_t>(packed & VALUE_MASK);
  return true;
}

bool AutomationAnalogOverrides::clear(std::size_t index)
{
  if (index >= values.size()) return false;
  values[index].store(0, std::memory_order_release);
  return true;
}

void AutomationAnalogOverrides::clearAll()
{
  for (auto& value : values) value.store(0, std::memory_order_release);
}

std::size_t AutomationAnalogOverrides::count() const
{
  return static_cast<std::size_t>(std::count_if(
      values.begin(), values.end(),
      [](const std::atomic<std::uint32_t>& value) {
        return (value.load(std::memory_order_acquire) & ENABLED) != 0;
      }));
}

void simuAutomationRuntimeStart()
{
  firmwareMailbox.reset();
  analogOverrides.clearAll();
  deferredCompletionActive = false;
  luaReloadActive = false;
  publishedLuaState.store(
#if defined(LUA)
      AutomationLuaState::NotObserved,
#else
      AutomationLuaState::Unavailable,
#endif
      std::memory_order_release);
  runtimeActive.store(true, std::memory_order_release);
}

void simuAutomationRuntimeStop()
{
  runtimeActive.store(false, std::memory_order_release);
  analogOverrides.clearAll();
}

void simuAutomationRuntimeResetAfterTaskJoin()
{
  firmwareMailbox.reset();
  deferredCompletionActive = false;
  luaReloadActive = false;
  publishedLuaState.store(
#if defined(LUA)
      AutomationLuaState::NotObserved,
#else
      AutomationLuaState::Unavailable,
#endif
      std::memory_order_release);
}

bool simuAutomationRuntimeActive()
{
  return runtimeActive.load(std::memory_order_acquire);
}

bool simuAutomationPostFirmwareRequest(const FirmwareRequest& request)
{
  return simuAutomationRuntimeActive() &&
         firmwareMailbox.enqueueRequest(request);
}

bool simuAutomationTakeFirmwareCompletion(FirmwareCompletion* completion)
{
  return firmwareMailbox.dequeueCompletion(completion);
}

std::size_t simuAutomationFirmwareRequestDepth()
{
  return firmwareMailbox.requestDepth();
}

std::size_t simuAutomationFirmwareCompletionDepth()
{
  return firmwareMailbox.completionDepth();
}

bool simuAutomationFirmwareIdle() { return firmwareMailbox.idle(); }

bool simuAutomationSetAnalogOverride(std::size_t index, std::uint16_t value)
{
  return simuAutomationRuntimeActive() && analogOverrides.set(index, value);
}

bool simuAutomationGetAnalogOverride(std::size_t index, std::uint16_t* value)
{
  return simuAutomationRuntimeActive() && analogOverrides.get(index, value);
}

bool simuAutomationClearAnalogOverride(std::size_t index)
{
  return analogOverrides.clear(index);
}

void simuAutomationClearAnalogOverrides() { analogOverrides.clearAll(); }

std::size_t simuAutomationAnalogOverrideCount()
{
  return analogOverrides.count();
}

AutomationLuaState simuAutomationLuaState()
{
  return publishedLuaState.load(std::memory_order_acquire);
}

const char* automationLuaStateName(AutomationLuaState state)
{
  switch (state) {
    case AutomationLuaState::Unavailable:
      return "unavailable";
    case AutomationLuaState::NotObserved:
      return "not_observed";
    case AutomationLuaState::Idle:
      return "idle";
    case AutomationLuaState::Reloading:
      return "reloading";
    case AutomationLuaState::Running:
      return "running";
    case AutomationLuaState::Panic:
      return "panic";
  }
  return "not_observed";
}

void simuAutomationBeforeUi()
{
  if (!simuAutomationRuntimeActive()) return;
  publishedLuaState.store(observeLuaState(), std::memory_order_release);

  for (std::size_t count = 0; count < 2 && !deferredCompletionActive; ++count) {
    FirmwareRequest request;
    if (!firmwareMailbox.dequeueRequest(&request)) break;
    processFirmwareRequest(request);
  }
}

void simuAutomationAfterUi()
{
  if (!simuAutomationRuntimeActive()) return;

  const AutomationLuaState observed = observeLuaState();
  publishedLuaState.store(observed, std::memory_order_release);

  if (deferredCompletionActive &&
      firmwareMailbox.enqueueCompletion(deferredCompletion)) {
    deferredCompletionActive = false;
  }

#if defined(LUA)
  if (!deferredCompletionActive && luaReloadActive &&
      (observed == AutomationLuaState::Running ||
       observed == AutomationLuaState::Panic)) {
    FirmwareCompletion completion;
    completion.operation = FirmwareOperation::ReloadLua;
    completion.id = luaReloadRequest.id;
    completion.epoch = luaReloadRequest.epoch;
    completion.generation = luaReloadRequest.generation;
    completion.luaState = observed;
    if (observed == AutomationLuaState::Panic)
      completion.code = FirmwareCompletionCode::LuaPanic;
    if (firmwareMailbox.enqueueCompletion(completion)) luaReloadActive = false;
  }
#endif
}

}  // namespace automation
}  // namespace edgetx
