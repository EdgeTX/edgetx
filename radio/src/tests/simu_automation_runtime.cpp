/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 */

#include <cstring>
#include <limits>

#include "automation_runtime.h"
#include "gtests.h"

using namespace edgetx::automation;

TEST(SimuAutomationFirmwareMailbox, PreservesRequestFifoAndCapacity)
{
  AutomationFirmwareMailbox mailbox;
  FirmwareRequest request;
  request.operation = FirmwareOperation::Telemetry;

  for (std::size_t index = 0; index < FIRMWARE_MAILBOX_CAPACITY; ++index) {
    request.id = index + 1;
    EXPECT_TRUE(mailbox.enqueueRequest(request));
  }
  EXPECT_FALSE(mailbox.enqueueRequest(request));
  EXPECT_EQ(mailbox.requestDepth(), FIRMWARE_MAILBOX_CAPACITY);
  EXPECT_FALSE(mailbox.idle());

  for (std::size_t index = 0; index < FIRMWARE_MAILBOX_CAPACITY; ++index) {
    FirmwareRequest observed;
    ASSERT_TRUE(mailbox.dequeueRequest(&observed));
    EXPECT_EQ(observed.id, index + 1);
    EXPECT_EQ(observed.operation, FirmwareOperation::Telemetry);
  }
  EXPECT_FALSE(mailbox.dequeueRequest(&request));
  EXPECT_TRUE(mailbox.idle());
}

TEST(SimuAutomationFirmwareMailbox, PreservesCompletionFifoAndReset)
{
  AutomationFirmwareMailbox mailbox;
  FirmwareCompletion completion;
  completion.operation = FirmwareOperation::ReloadLua;

  for (std::size_t index = 0; index < FIRMWARE_MAILBOX_CAPACITY; ++index) {
    completion.id = index + 10;
    completion.generation = index + 1;
    EXPECT_TRUE(mailbox.enqueueCompletion(completion));
  }
  EXPECT_FALSE(mailbox.enqueueCompletion(completion));
  EXPECT_EQ(mailbox.completionDepth(), FIRMWARE_MAILBOX_CAPACITY);

  FirmwareCompletion observed;
  ASSERT_TRUE(mailbox.dequeueCompletion(&observed));
  EXPECT_EQ(observed.id, 10u);
  EXPECT_EQ(observed.generation, 1u);
  EXPECT_EQ(mailbox.completionDepth(), FIRMWARE_MAILBOX_CAPACITY - 1);

  mailbox.reset();
  EXPECT_TRUE(mailbox.idle());
  EXPECT_FALSE(mailbox.dequeueCompletion(&observed));
}

TEST(SimuAutomationAnalogOverrides, ReplacesClearsAndFallsBack)
{
  AutomationAnalogOverrides overrides;
  std::uint16_t value = 99;

  EXPECT_FALSE(overrides.get(0, &value));
  EXPECT_EQ(value, 99u);
  EXPECT_TRUE(overrides.set(0, 0));
  EXPECT_TRUE(overrides.get(0, &value));
  EXPECT_EQ(value, 0u);
  EXPECT_EQ(overrides.count(), 1u);

  EXPECT_TRUE(overrides.set(0, 4096));
  EXPECT_TRUE(overrides.get(0, &value));
  EXPECT_EQ(value, 4096u);
  EXPECT_EQ(overrides.count(), 1u);

  EXPECT_TRUE(overrides.set(MAX_ANALOG_INPUTS - 1, 2048));
  EXPECT_EQ(overrides.count(), 2u);
  EXPECT_FALSE(overrides.set(MAX_ANALOG_INPUTS, 1));
  EXPECT_FALSE(overrides.set(0, 4097));
  EXPECT_FALSE(overrides.get(MAX_ANALOG_INPUTS, &value));
  EXPECT_FALSE(overrides.get(0, nullptr));

  EXPECT_TRUE(overrides.clear(0));
  EXPECT_FALSE(overrides.get(0, &value));
  EXPECT_EQ(overrides.count(), 1u);
  EXPECT_FALSE(overrides.clear(MAX_ANALOG_INPUTS));
  overrides.clearAll();
  EXPECT_EQ(overrides.count(), 0u);
}

TEST(SimuAutomationRuntime, TelemetryIgnoresInteractiveDiscoveryGate)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  allowNewSensors = false;
  simuAutomationRuntimeStart();

  FirmwareRequest request;
  request.operation = FirmwareOperation::Telemetry;
  request.id = 42;
  request.epoch = 3;
  request.telemetryId = 0xf101;
  request.telemetryValue = std::numeric_limits<std::int32_t>::min();
  request.telemetryUnit = UNIT_DBM;
  std::memcpy(request.telemetryName, "RSSI", TELEM_LABEL_LEN);

  ASSERT_TRUE(simuAutomationPostFirmwareRequest(request));
  simuAutomationBeforeUi();
  simuAutomationAfterUi();

  FirmwareCompletion completion;
  ASSERT_TRUE(simuAutomationTakeFirmwareCompletion(&completion));
  ASSERT_TRUE(completion.ok());
  ASSERT_GE(completion.telemetryIndex, 0);
  EXPECT_FALSE(allowNewSensors);
  EXPECT_EQ(g_model.telemetrySensors[completion.telemetryIndex].id, 0xf101);
  EXPECT_EQ(telemetryItems[completion.telemetryIndex].value,
            std::numeric_limits<std::int32_t>::min());

  request.id += 1;
  request.telemetryValue = std::numeric_limits<std::int32_t>::max();
  ASSERT_TRUE(simuAutomationPostFirmwareRequest(request));
  simuAutomationBeforeUi();
  simuAutomationAfterUi();
  ASSERT_TRUE(simuAutomationTakeFirmwareCompletion(&completion));
  EXPECT_TRUE(completion.ok());
  EXPECT_FALSE(allowNewSensors);
  EXPECT_EQ(telemetryItems[completion.telemetryIndex].value,
            std::numeric_limits<std::int32_t>::max());

  simuAutomationRuntimeStop();
}

TEST(SimuAutomationRuntime, TelemetryUsesExactTupleWhenSensorIdsAreIgnored)
{
  MODEL_RESET();
  TELEMETRY_RESET();
  allowNewSensors = false;
  g_model.ignoreSensorIds = true;

  TelemetrySensor& original = g_model.telemetrySensors[0];
  original.id = 0xf101;
  original.subId = 0;
  original.instance = 0;
  original.init("OLD", UNIT_DBM, 0);
  telemetryItems[0].setValue(original, -10, UNIT_DBM, 0);

  simuAutomationRuntimeStart();

  FirmwareRequest request;
  request.operation = FirmwareOperation::Telemetry;
  request.id = 43;
  request.epoch = 4;
  request.telemetryId = 0xf101;
  request.telemetrySubId = 0;
  request.telemetryInstance = 1;
  request.telemetryValue = -73;
  request.telemetryUnit = UNIT_DBM;
  std::memcpy(request.telemetryName, "RSSI", TELEM_LABEL_LEN);

  ASSERT_TRUE(simuAutomationPostFirmwareRequest(request));
  simuAutomationBeforeUi();
  simuAutomationAfterUi();

  FirmwareCompletion completion;
  ASSERT_TRUE(simuAutomationTakeFirmwareCompletion(&completion));
  ASSERT_TRUE(completion.ok());
  ASSERT_GT(completion.telemetryIndex, 0);
  EXPECT_EQ(telemetryItems[0].value, -10);
  EXPECT_EQ(g_model.telemetrySensors[completion.telemetryIndex].instance, 1);
  EXPECT_EQ(telemetryItems[completion.telemetryIndex].value, -73);
  EXPECT_TRUE(g_model.ignoreSensorIds);
  EXPECT_FALSE(allowNewSensors);

  simuAutomationRuntimeStop();
}
