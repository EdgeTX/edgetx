// SPDX-License-Identifier: GPL-2.0-only
#include "gimbals.h"

#include <cmath>

namespace mambo
{
namespace
{
constexpr uint8_t registers[] = {0, 1, 2, 4, 5, 6};
constexpr uint16_t values[] = {0, 0x8400, 0x02a4, 0x8000, 0x8000, 0x8000};
constexpr uint32_t staleUs = 20000;
uint16_t be16(const uint8_t* p) { return (uint16_t(p[0]) << 8) | p[1]; }
bool accepted(uint8_t status) { return !(status & 0x14); }  // ERROR or reset
}  // namespace

void HallSensor::begin(uint32_t now)
{
  state = State::Exit;
  changed = now;
  delay = 0;
  reg = 0;
  data = {};
}

void HallSensor::failed(uint32_t now, HallFault fault, uint8_t command,
                        uint8_t status)
{
  ++data.errors;
  data.lastFault = fault;
  data.faultCommand = command;
  data.faultStatus = status;
  data.valid = false;
  state = State::Exit;
  changed = now;
  delay = 100000;  // bounded retries for missing or faulty sensors
  reg = 0;
}

bool HallSensor::fresh(uint32_t now) const
{
  return data.valid && uint32_t(now - data.timestamp) < staleUs;
}

bool HallSensor::startMeasurement(unsigned sensor, uint32_t now,
                                  HallTransfer transfer)
{
  // Match Tracer's explicit SM(TXYZ) command rather than depending on burst
  // mode. Each successful read immediately arms the next conversion.
  const uint8_t command = 0x3f;
  uint8_t response = 0;
  if (!transfer(sensor, &command, 1, &response, 1)) {
    failed(now, HallFault::Transport, command);
    return false;
  }
  if (!accepted(response) || (response & 0xe0) != 0x20) {
    failed(now, HallFault::StartStatus, command, response);
    return false;
  }
  state = State::Read;
  changed = now;
  delay = 2000;
  return true;
}

void HallSensor::poll(unsigned sensor, uint32_t now, bool ready,
                      HallTransfer transfer)
{
  if (uint32_t(now - changed) < delay) return;
  uint8_t command[4] = {};
  uint8_t response[9] = {};
  switch (state) {
    case State::Exit:
      command[0] = 0x80;
      // EX may reject an already faulty memory; RT must still be attempted.
      if (!transfer(sensor, command, 1, response, 1)) {
        failed(now, HallFault::Transport, command[0]);
        return;
      }
      state = State::Reset;
      delay = 1000;
      break;
    case State::Reset:
      command[0] = 0xf0;
      if (!transfer(sensor, command, 1, response, 1)) {
        failed(now, HallFault::Transport, command[0]);
        return;
      }
      if (response[0] & 0x10) {
        failed(now, HallFault::ResetStatus, command[0], response[0]);
        return;
      }
      state = State::ClearReset;
      delay = 2000;
      break;
    case State::ClearReset:
      // Consume the reset indication before checking configuration replies.
      command[0] = 0x80;
      if (!transfer(sensor, command, 1, response, 1)) {
        failed(now, HallFault::Transport, command[0]);
        return;
      }
      if (response[0] & 0x10) {
        failed(now, HallFault::ResetStatus, command[0], response[0]);
        return;
      }
      state = State::Configure;
      delay = 0;
      break;
    case State::Configure: {
      auto value = values[reg];
      command[0] = 0x60;
      command[1] = value >> 8;
      command[2] = value;
      command[3] = registers[reg] << 2;
      if (!transfer(sensor, command, 4, response, 1)) {
        failed(now, HallFault::Transport, command[0]);
        return;
      }
      if (!accepted(response[0])) {
        failed(now, HallFault::ConfigurationStatus, command[0], response[0]);
        return;
      }
      command[0] = 0x50;
      command[1] = command[3];
      if (!transfer(sensor, command, 2, response, 3)) {
        failed(now, HallFault::Transport, command[0]);
        return;
      }
      if (!accepted(response[0]) || (response[0] & 3) != 0) {
        failed(now, HallFault::ReadbackStatus, command[0], response[0]);
        return;
      }
      if (be16(response + 1) != value) {
        failed(now, HallFault::ReadbackMismatch, command[0], response[0]);
        return;
      }
      if (++reg == sizeof(registers)) state = State::Start;
      delay = 0;
      break;
    }
    case State::Start:
      startMeasurement(sensor, now, transfer);
      return;
    case State::Read:
      if (!ready) {
        if (uint32_t(now - changed) >= staleUs)
          failed(now, HallFault::ReadyTimeout, 0x4f);
        return;
      }
      command[0] = 0x4f;
      if (!transfer(sensor, command, 1, response, 9)) {
        failed(now, HallFault::Transport, command[0]);
        return;
      }
      // SM may already be clear after conversion. Require eight data bytes,
      // no ERROR/reset, and no unexpected burst/wake-on-change mode.
      if (!accepted(response[0]) || (response[0] & 0xc3) != 0x03) {
        failed(now, HallFault::MeasurementStatus, command[0], response[0]);
        return;
      }
      data.status = response[0];
      data.temperature = be16(response + 1);
      // TCMP_EN + RES=1 and OFFSET=0x8000: captured RF subtracts 32768.
      data.x = int32_t(be16(response + 3)) - 32768;
      data.y = int32_t(be16(response + 5)) - 32768;
      data.z = int32_t(be16(response + 7)) - 32768;
      data.timestamp = now;
      ++data.count;
      data.valid = true;
      startMeasurement(sensor, now, transfer);
      return;
  }
  changed = now;
}

bool hallAngles(const HallSample& sample, uint16_t& x, uint16_t& y)
{
  if (!sample.valid || (!sample.x && !sample.y && !sample.z)) return false;
  // Captured constants at 0x080c4ed8/4edc, Hall gain=0, resolution=1.
  float bx = sample.x * 1.573f;
  float by = sample.y * 1.573f;
  float bz = sample.z * 2.534f;
  constexpr float scale = 4095.0f / 3.14159265358979323846f;
  x = uint16_t(std::lround(
      2047.5f + scale * std::atan2(bx, std::sqrt(by * by + bz * bz))));
  y = uint16_t(std::lround(
      2047.5f + scale * std::atan2(by, std::sqrt(bx * bx + bz * bz))));
  return true;
}
}  // namespace mambo
