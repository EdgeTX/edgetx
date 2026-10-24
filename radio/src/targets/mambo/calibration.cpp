// SPDX-License-Identifier: GPL-2.0-only
#include "calibration.h"

#include <cmath>
#include <cstring>
#include <initializer_list>

namespace mambo
{
namespace
{
uint16_t word(const uint8_t* data, unsigned index)
{
  return uint16_t(data[index * 2]) | (uint16_t(data[index * 2 + 1]) << 8);
}
int signed16(uint16_t value)
{
  return value < 32768 ? value : int(value) - 65536;
}
int signed8(uint8_t value) { return value < 128 ? value : int(value) - 256; }
uint32_t reflectedCrc(const uint8_t* data, size_t size)
{
  uint32_t crc = 0xffffffff;
  for (size_t i = 0; i < size; ++i) {
    crc ^= data[i];
    for (unsigned bit = 0; bit < 8; ++bit)
      crc = (crc >> 1) ^ ((crc & 1) ? 0xedb88320 : 0);
  }
  return crc;  // no final XOR, as used for corner records
}
uint32_t reverse32(uint32_t value)
{
  uint32_t result = 0;
  for (unsigned i = 0; i < 32; ++i) {
    result = (result << 1) | (value & 1);
    value >>= 1;
  }
  return result;
}
uint32_t legacyCrc(const uint8_t* data, size_t size)
{
  uint32_t crc = 0xffffffff;
  for (size_t i = 0; i < size; ++i) {
    uint8_t byte = reverse32(data[i]) >> 24;
    for (unsigned bit = 0; bit < 8; ++bit) {
      bool condition = (crc & 0x80000000) || (byte & 0x80);
      crc = (crc << 1) ^ (condition ? 0x04c11db7 : 0);
      byte <<= 1;
    }
  }
  return reverse32(crc) ^ 0xffffffff;
}
uint16_t angleCrc(const uint8_t* data, unsigned base)
{
  uint16_t crc = 0xffff;
  for (unsigned i = 0; i < 10; ++i) {
    auto value = word(data, base + i);
    for (unsigned byte : {unsigned(value >> 8), unsigned(value & 255)}) {
      crc ^= byte;
      for (unsigned bit = 0; bit < 8; ++bit)
        crc = (crc >> 1) ^ ((crc & 1) ? 0x8408 : 0);
    }
  }
  return crc;
}
int cross(CalibrationPoint a, CalibrationPoint b)
{
  return int(a.y) * b.x - int(a.x) * b.y;
}
CalibrationPoint relative(CalibrationPoint point, CalibrationPoint origin)
{
  return {int16_t(point.x - origin.x), int16_t(point.y - origin.y)};
}
bool buildMap(GimbalCorrection& correction, unsigned q)
{
  constexpr unsigned indices[4][3] = {
      {1, 2, 3}, {5, 4, 3}, {5, 6, 7}, {1, 8, 7}};
  const int sx = (q == 1 || q == 2) ? -1 : 1;
  const int sy = q >= 2 ? -1 : 1;
  float p[3][2];
  for (unsigned i = 0; i < 3; ++i) {
    auto point =
        relative(correction.points[indices[q][i]], correction.points[0]);
    p[i][0] = float(point.x) * sx;
    p[i][1] = float(point.y) * sy;
  }
  // Homography fixing the origin and mapping side/diagonal/side to
  // (1,0)/(1,1)/(0,1), equivalent to the stock inverse-matrix construction.
  float det = p[0][0] * p[2][1] - p[0][1] * p[2][0];
  if (det <= 1.0f) return false;
  float u = (p[1][0] * p[2][1] - p[1][1] * p[2][0]) / det;
  float v = (p[0][0] * p[1][1] - p[0][1] * p[1][0]) / det;
  float sum = u + v - 1.0f;
  if (u <= 0.01f || v <= 0.01f || sum <= 0.01f) return false;
  float a = sum / u, b = sum / v;
  auto m = correction.maps[q];
  float ix = p[2][1] / det, iy = -p[2][0] / det;
  float jx = -p[0][1] / det, jy = p[0][0] / det;
  m[0] = a * ix;
  m[1] = a * iy;
  m[2] = b * jx;
  m[3] = b * jy;
  m[4] = (a - 1) * ix + (b - 1) * jx;
  m[5] = (a - 1) * iy + (b - 1) * jy;
  for (unsigned i = 0; i < 6; ++i)
    if (!std::isfinite(m[i])) return false;
  return true;
}
float axisAngle(float primary, float secondary, float z,
                const AxisCorrection& axis)
{
  float side = (secondary - axis.denominatorCrossZ * z) * axis.ratio;
  float numerator = primary - axis.numeratorCrossZ * z;
  // Stock atan2f receives sqrt(...) as its first argument, the corrected
  // primary component as its second. The neutral angle is near 90 degrees.
  return std::atan2(std::sqrt(z * z + side * side), numerator) * 180.0f /
         3.14159265358979323846f;
}
bool scaledField(int32_t raw, float scale, float& value)
{
  if (raw < -32768 || raw > 32767) return false;
  value = std::trunc(raw * scale);
  // Stock narrows to int16. Reject overflow rather than wrap a magnetic field.
  return value >= -32768 && value <= 32767;
}
}  // namespace

bool decodeFactoryCalibration(const uint8_t* data, size_t size,
                              FactoryCalibration& output)
{
  if (!data || size < 604) return false;
  uint32_t stored = word(data, 300) | (uint32_t(word(data, 301)) << 16);
  if (stored != (reflectedCrc(data, 600) ^ 0xffffffff) &&
      stored != legacyCrc(data, 600))
    return false;
  FactoryCalibration candidate{};
  for (unsigned sensor = 0; sensor < 2; ++sensor) {
    unsigned base = 0x78 + sensor * 11, corner = 0xa0 + sensor * 20;
    if (word(data, base + 10) != angleCrc(data, base)) return false;
    uint32_t cornerStored =
        (uint32_t(word(data, corner + 18)) << 16) | word(data, corner + 19);
    if (cornerStored != reflectedCrc(data + corner * 2, 36)) return false;
    auto& correction = candidate.gimbals[sensor];
    for (unsigned i = 0; i < 2; ++i) {
      unsigned axisBase = base + i * 4;
      auto& axis = correction.axes[i];
      auto packed = word(data, axisBase + 1);
      axis = {signed16(word(data, axisBase)) / 1000.0f,
              signed8(packed >> 8) / 256.0f,
              signed8(packed & 255) / 256.0f,
              signed16(word(data, axisBase + 2)) / 1000.0f,
              signed16(word(data, axisBase + 3)) / 1000.0f,
              word(data, base + 8 + i) / 100.0f};
      if (axis.ratio <= 0 || axis.positiveGain == 0 || axis.negativeGain == 0 ||
          axis.centre > 180)
        return false;
    }
    for (unsigned i = 0; i < 9; ++i) {
      correction.points[i] = {
          int16_t(signed16(word(data, corner + 2 * i))),
          int16_t(signed16(word(data, corner + 2 * i + 1)))};
      // Subtraction and stock cross products must stay within their ranges.
      if (std::abs(int(correction.points[i].x)) > 16000 ||
          std::abs(int(correction.points[i].y)) > 16000)
        return false;
    }
    for (unsigned q = 0; q < 4; ++q)
      if (!buildMap(correction, q)) return false;
  }
  output = candidate;
  return true;
}

bool correctGimbalPlane(const GimbalCorrection& correction, int16_t x,
                        int16_t y, float& correctedX, float& correctedY)
{
  if (std::abs(int(x)) > 16000 || std::abs(int(y)) > 16000) return false;
  auto point = relative({x, y}, correction.points[0]);
  auto side1 = relative(correction.points[1], correction.points[0]);
  auto side3 = relative(correction.points[3], correction.points[0]);
  auto side5 = relative(correction.points[5], correction.points[0]);
  auto side7 = relative(correction.points[7], correction.points[0]);
  unsigned q;
  // Preserve stock boundary comparisons at 0x080c46a0, including equality.
  if (cross(point, side1) >= 0) {
    if (cross(point, side3) <= 0)
      q = 0;
    else
      q = cross(point, side5) <= 0 ? 1 : 2;
  } else if (cross(point, side7) >= 0)
    q = 3;
  else
    q = cross(point, side5) >= 0 ? 2 : 1;
  int sx = (q == 1 || q == 2) ? -1 : 1, sy = q >= 2 ? -1 : 1;
  float px = float(point.x) * sx, py = float(point.y) * sy;
  const auto m = correction.maps[q];
  float denominator = m[4] * px + m[5] * py + 1;
  if (!std::isfinite(denominator) || denominator <= 0.0001f) return false;
  correctedX = sx * 10000.0f * (m[0] * px + m[1] * py) / denominator;
  correctedY = sy * 10000.0f * (m[2] * px + m[3] * py) / denominator;
  return std::isfinite(correctedX) && std::isfinite(correctedY);
}

bool calibratedHall(const HallSample& sample,
                    const GimbalCorrection& correction, uint16_t& x,
                    uint16_t& y)
{
  if (!sample.valid || (!sample.x && !sample.y && !sample.z)) return false;
  float bx, by, bz;
  if (!scaledField(sample.x, 1.573f, bx) ||
      !scaledField(sample.y, 1.573f, by) || !scaledField(sample.z, 2.534f, bz))
    return false;
  float angles[2] = {axisAngle(bx, by, bz, correction.axes[0]),
                     axisAngle(by, bx, bz, correction.axes[1])};
  int16_t plane[2];
  for (unsigned i = 0; i < 2; ++i) {
    const auto& axis = correction.axes[i];
    float delta = angles[i] - axis.centre;
    float value =
        delta * (delta > 0 ? axis.positiveGain : axis.negativeGain) * 100;
    if (!std::isfinite(value) || value < -16000 || value > 16000) return false;
    plane[i] = int16_t(value);  // truncate, as in captured VCVT/STRH sequence
  }
  float correctedX, correctedY;
  if (!correctGimbalPlane(correction, plane[0], plane[1], correctedX,
                          correctedY))
    return false;
  // Match stock truncation before adapting its +/-10000 range to EdgeTX ADC.
  auto adc = [](float value) {
    if (value > 10000) value = 10000;
    if (value < -10000) value = -10000;
    return uint16_t(std::lround(2047.5f + int(value) * (4095.0f / 20000.0f)));
  };
  x = adc(correctedX);
  y = adc(correctedY);
  return true;
}
}  // namespace mambo
