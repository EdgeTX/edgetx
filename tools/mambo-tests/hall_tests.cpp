// SPDX-License-Identifier: GPL-2.0-only
#include "targets/mambo/gimbals.h"
#include "targets/mambo/diagnostics.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <initializer_list>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "line %d: %s\n", __LINE__, #x); std::exit(1); } ++checks; } while (0)
unsigned checks = 0;
unsigned calls = 0;
bool timeout = false, corrupt = false;
uint8_t measurementStatus = 0x03;
bool conversionPending[2] = {};
uint16_t memory[2][64] = {};
uint16_t raw[4] = {0x1234, 0x8000, 0x8001, 0x7fff};
uint8_t lastCommand = 0;

bool fakeTransfer(unsigned sensor, const uint8_t* command, size_t nc,
                  uint8_t* response, size_t nr)
{
  ++calls;
  lastCommand = command[0];
  if (timeout) return false;
  std::memset(response, 0, nr);
  CHECK(sensor < 2);
  switch (command[0]) {
    case 0x80: case 0xf0:
      CHECK(nc == 1 && nr == 1);
      if (command[0] == 0xf0) response[0] = 4;
      break;
    case 0x60:
      CHECK(nc == 4 && nr == 1);
      CHECK(!(command[3] & 3));
      memory[sensor][command[3] >> 2] = (uint16_t(command[1]) << 8) | command[2];
      break;
    case 0x50: {
      CHECK(nc == 2 && nr == 3);
      uint16_t value = memory[sensor][command[1] >> 2] ^ (corrupt ? 1 : 0);
      response[1] = value >> 8; response[2] = value;
      break;
    }
    case 0x3f:
      CHECK(nc == 1 && nr == 1);
      response[0] = 0x20;
      conversionPending[sensor] = true;
      break;
    case 0x4f:
      CHECK(nc == 1 && nr == 9);
      CHECK(conversionPending[sensor]);
      conversionPending[sensor] = false;
      response[0] = measurementStatus;
      for (unsigned i = 0; i < 4; ++i) {
        response[1 + i * 2] = raw[i] >> 8;
        response[2 + i * 2] = raw[i];
      }
      break;
    default: CHECK(false);
  }
  return true;
}

uint32_t initialise(mambo::HallSensor& sensor, unsigned index, uint32_t now)
{
  sensor.begin(now);
  sensor.poll(index, now, false, fakeTransfer);
  CHECK(lastCommand == 0x80);
  unsigned before = calls;
  sensor.poll(index, now + 999, false, fakeTransfer);
  CHECK(calls == before);
  sensor.poll(index, now + 1000, false, fakeTransfer);
  CHECK(lastCommand == 0xf0);
  before = calls;
  sensor.poll(index, now + 2999, false, fakeTransfer);
  CHECK(calls == before);
  sensor.poll(index, now + 3000, false, fakeTransfer);
  CHECK(lastCommand == 0x80);
  for (unsigned i = 0; i < 6; ++i)
    sensor.poll(index, now + 3000 + i, false, fakeTransfer);
  CHECK(memory[index][0] == 0);
  CHECK(memory[index][1] == 0x8400);
  CHECK(memory[index][2] == 0x2a4);
  CHECK(memory[index][4] == 0x8000 && memory[index][5] == 0x8000 && memory[index][6] == 0x8000);
  sensor.poll(index, now + 3010, false, fakeTransfer);
  CHECK(lastCommand == 0x3f);
  return now + 5010;
}

int main()
{
  mambo::HallSensor left, right;
  uint32_t now = initialise(left, 0, 0);
  CHECK(!left.fresh(now));
  unsigned before = calls;
  left.poll(0, now, false, fakeTransfer);
  CHECK(calls == before);
  left.poll(0, now, true, fakeTransfer);
  CHECK(left.fresh(now));
  CHECK(left.sample().x == 0 && left.sample().y == 1 && left.sample().z == -1);
  CHECK(left.sample().temperature == 0x1234 && left.sample().count == 1);
  CHECK(left.fresh(now + 19999) && !left.fresh(now + 20000));
  left.poll(0, now + 20000, false, fakeTransfer);
  CHECK(!left.sample().valid && left.sample().errors == 1);
  CHECK(left.sample().lastFault == mambo::HallFault::ReadyTimeout);
  CHECK(left.sample().faultCommand == 0x4f && left.sample().faultStatus == 0);
  before = calls;
  left.poll(0, now + 119999, true, fakeTransfer);
  CHECK(calls == before);
  left.poll(0, now + 120000, true, fakeTransfer);
  CHECK(lastCommand == 0x80);

  // Tick wrapping must preserve delays and freshness.
  now = initialise(right, 1, std::numeric_limits<uint32_t>::max() - 4000);
  raw[1] = 0; raw[2] = 0xffff; raw[3] = 0x8000;
  right.poll(1, now, true, fakeTransfer);
  CHECK(right.fresh(now + 1));
  CHECK(right.sample().x == -32768 && right.sample().y == 32767);

  // ERROR, reset indication, wrong byte count, and unexpected burst invalidate
  // readings. SED is informational and does not reject a measurement.
  for (uint8_t status : {uint8_t(0x13), uint8_t(0x07), uint8_t(0x02), uint8_t(0x83)}) {
    now = initialise(left, 0, 0);
    measurementStatus = status;
    left.poll(0, now, true, fakeTransfer);
    CHECK(!left.fresh(now) && left.sample().errors == 1);
    CHECK(left.sample().lastFault == mambo::HallFault::MeasurementStatus);
    CHECK(left.sample().faultCommand == 0x4f && left.sample().faultStatus == status);
  }
  now = initialise(left, 0, 0);
  measurementStatus = 0x0b;
  left.poll(0, now, true, fakeTransfer);
  CHECK(left.fresh(now));
  timeout = true;
  left.poll(0, now + 2000, true, fakeTransfer);
  CHECK(!left.fresh(now + 2000) && left.sample().errors == 1);
  CHECK(left.sample().lastFault == mambo::HallFault::Transport);
  CHECK(left.sample().faultCommand == 0x4f && left.sample().faultStatus == 0);
  timeout = false;
  left.begin(0);
  left.poll(0, 0, false, fakeTransfer);
  left.poll(0, 1000, false, fakeTransfer);
  left.poll(0, 3000, false, fakeTransfer);
  corrupt = true;
  left.poll(0, 3001, false, fakeTransfer);
  CHECK(left.sample().errors == 1 && !left.sample().valid);
  CHECK(left.sample().lastFault == mambo::HallFault::ReadbackMismatch);
  CHECK(left.sample().faultCommand == 0x50);

  // A single conversion has no autonomous follow-up. Every accepted read must
  // issue another SM, and never read early while the ready line is low.
  corrupt = false;
  measurementStatus = 0x23;
  now = initialise(left, 0, 0);
  for (unsigned i = 0; i < 100; ++i) {
    left.poll(0, now, true, fakeTransfer);
    CHECK(left.sample().count == i + 1 && left.fresh(now));
    CHECK(conversionPending[0] && lastCommand == 0x3f);
    before = calls;
    left.poll(0, now + 1000, false, fakeTransfer);
    CHECK(calls == before);
    now += 4000;
  }
  CHECK(left.sample().errors == 0);

  // Geometry invariants: origin rejected, polarity, endpoint bounds, Z sign
  // symmetry, and scale invariance. X/Y/Z sensitivity is not interchangeable.
  uint16_t x, y;
  mambo::HallSample sample;
  CHECK(!mambo::hallAngles(sample, x, y));
  sample.valid = true;
  CHECK(!mambo::hallAngles(sample, x, y));
  sample.z = 10000;
  CHECK(mambo::hallAngles(sample, x, y) && x == 2048 && y == 2048);
  sample.x = 10000;
  CHECK(mambo::hallAngles(sample, x, y) && x > 2700 && x < 2800 && y == 2048);
  uint16_t original = x;
  sample.z = -10000;
  CHECK(mambo::hallAngles(sample, x, y) && x == original);
  sample.x *= 2; sample.z *= 2;
  CHECK(mambo::hallAngles(sample, x, y) && x == original);
  sample.x = -10000; sample.z = 0;
  CHECK(mambo::hallAngles(sample, x, y) && x == 0 && y == 2048);
  sample.x = 10000;
  CHECK(mambo::hallAngles(sample, x, y) && x == 4095 && y == 2048);
  sample.x = 0; sample.y = -10000;
  CHECK(mambo::hallAngles(sample, x, y) && x == 2048 && y == 0);
  sample = {};
  CHECK(mambo::hallHealth(sample,0) == mambo::HallHealth::Waiting);
  sample.errors = 1;
  CHECK(mambo::hallHealth(sample,0) == mambo::HallHealth::Error);
  sample.valid = true;
  sample.timestamp = 0xfffffff0;
  CHECK(mambo::hallHealth(sample,100) == mambo::HallHealth::Fresh);
  CHECK(mambo::hallHealth(sample,uint32_t(sample.timestamp + 20000)) == mambo::HallHealth::Stale);
  std::printf("Mambo Hall backend: %u checks passed\n", checks);
}
