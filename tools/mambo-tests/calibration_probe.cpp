// SPDX-License-Identifier: GPL-2.0-only
// Host probe for offline verification. Reads a regular backup, never a radio.
#include "targets/mambo/calibration.h"
#include <cstdio>
#include <cstring>

int main(int argc, char** argv)
{
  if (argc != 2) return 2;
  auto file = std::fopen(argv[1], "rb");
  if (!file) return 2;
  uint8_t data[604];
  auto size = std::fread(data, 1, sizeof(data), file);
  std::fclose(file);
  mambo::FactoryCalibration calibration{};
  mambo::FactoryCalibration original = calibration;
  if (!mambo::decodeFactoryCalibration(data, size, calibration)) {
    if (std::memcmp(&calibration, &original, sizeof(calibration))) return 3;
    std::puts("rejected");
    return 1;
  }
  for (unsigned sensor = 0; sensor < 2; ++sensor) {
    const auto& correction = calibration.gimbals[sensor];
    for (unsigned i = 0; i < 9; ++i) {
      float x, y;
      auto point = correction.points[i];
      if (!mambo::correctGimbalPlane(correction, point.x, point.y, x, y)) return 4;
      std::printf("point %u %u %.6f %.6f\n", sensor, i, double(x), double(y));
    }
    for (int px = -4000; px <= 4000; px += 2000) {
      for (int py = -4000; py <= 4000; py += 2000) {
        mambo::HallSample sample;
        sample.valid = true; sample.x = px; sample.y = py; sample.z = 4000;
        uint16_t x, y;
        bool ok = mambo::calibratedHall(sample, correction, x, y);
        if (!ok) return 5;
        std::printf("sample %u %d %d %u %u\n", sensor, px, py, unsigned(x), unsigned(y));
      }
    }
    mambo::HallSample sample;
    uint16_t x = 123, y = 456;
    if (mambo::calibratedHall(sample, correction, x, y)) return 6;
    sample.valid = true;
    if (mambo::calibratedHall(sample, correction, x, y)) return 7;
    sample.x = 32767;
    if (mambo::calibratedHall(sample, correction, x, y)) return 8;
    if (x != 123 || y != 456) return 9;
  }
}
