// SPDX-License-Identifier: GPL-2.0-only
#pragma once
#include <cstddef>
#include <cstdint>

namespace mambo
{
// Transport keeps CS asserted across command and response. Response starts
// with the status byte; command-phase MISO is discarded.
using HallTransfer = bool (*)(unsigned, const uint8_t*, size_t, uint8_t*,
                              size_t);
enum class HallFault : uint8_t {
  None,
  Transport,
  ResetStatus,
  ConfigurationStatus,
  ReadbackStatus,
  ReadbackMismatch,
  StartStatus,
  ReadyTimeout,
  MeasurementStatus
};
struct HallSample {
  uint16_t temperature = 0;
  int32_t x = 0, y = 0, z = 0;
  uint32_t timestamp = 0, count = 0, errors = 0;
  uint8_t status = 0;
  HallFault lastFault = HallFault::None;
  uint8_t faultCommand = 0, faultStatus = 0;
  bool valid = false;
};

class HallSensor
{
 public:
  void begin(uint32_t now);
  void poll(unsigned sensor, uint32_t now, bool ready, HallTransfer transfer);
  bool fresh(uint32_t now) const;
  const HallSample& sample() const { return data; }

 private:
  enum class State { Exit, Reset, ClearReset, Configure, Start, Read };
  State state = State::Exit;
  uint32_t changed = 0;
  uint32_t delay = 0;
  unsigned reg = 0;
  HallSample data;
  void failed(uint32_t now, HallFault fault, uint8_t command,
              uint8_t status = 0);
  bool startMeasurement(unsigned sensor, uint32_t now, HallTransfer transfer);
};

// Provisional projection used when factory calibration is unavailable.
// Output feeds EdgeTX's ordinary centre/travel calibration.
bool hallAngles(const HallSample& sample, uint16_t& x, uint16_t& y);
}  // namespace mambo

void mamboGimbalsInit();
void mamboGimbalsDeinit();
void mamboGimbalsRead();
// Read-only bench diagnostics; only call from the acquisition task.
const mambo::HallSample& mamboGimbalSample(unsigned sensor);
