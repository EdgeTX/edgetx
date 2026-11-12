/*
 * Copyright (C) EdgeTX
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <algorithm>
#include <cstdlib>

#include "gtests.h"
#include "telemetry/telemetry.h"
#include "telemetry/flysky_ibus.h"

#if defined(MULTIMODULE) || defined(AFHDS3)

// Typical spacing between pressure samples (~120ms)
static constexpr tmr10ms_t STEP = 12;

TEST(FlySkyVSpeed, FirstSampleIsZero)
{
  FlySkyVSpeed v;
  v.update(5000, 1000);
  EXPECT_EQ(0, v.value());
}

TEST(FlySkyVSpeed, NeedsMinimumData)
{
  FlySkyVSpeed v;
  v.update(0, 1000);
  v.update(100, 1000 + STEP);
  EXPECT_EQ(0, v.value());  // only two samples

  FlySkyVSpeed w;
  w.update(0, 1000);
  w.update(5, 1005);
  w.update(10, 1010);
  EXPECT_EQ(0, w.value());  // three samples but less than MIN_SPAN
}

TEST(FlySkyVSpeed, SteadyClimbAndSink)
{
  FlySkyVSpeed v;
  tmr10ms_t t = 1000;
  int32_t alt = 0;

  // +1 m/s = 1 cm per 10ms tick
  for (int i = 0; i < 60; i++, t += STEP, alt += STEP) v.update(alt, t);
  EXPECT_NEAR(100, v.value(), 3);

  // -0.5 m/s, once the window has filled with sinking samples
  for (int i = 0; i < 60; i++, t += 2 * STEP, alt -= STEP) v.update(alt, t);
  EXPECT_NEAR(-50, v.value(), 3);
}

TEST(FlySkyVSpeed, UsesEverySample)
{
  FlySkyVSpeed v;
  tmr10ms_t t = 1000;
  int32_t alt = 0;
  for (int i = 0; i < 60; i++, t += STEP, alt += STEP) v.update(alt, t);
  ASSERT_NEAR(100, v.value(), 3);

  // Reverse to a 1 m/s sink: the estimate must move on every sample, not
  // every other one as with a minimum update interval
  alt -= STEP;
  int32_t prev = v.value();
  for (int i = 0; i < 6; i++, t += STEP) {
    alt -= STEP;
    v.update(alt, t);
    EXPECT_LT(v.value(), prev) << "sample " << i;
    prev = v.value();
  }
}

TEST(FlySkyVSpeed, RejectsQuantisationNoise)
{
  FlySkyVSpeed v;
  tmr10ms_t t = 1000;
  int32_t worst = 0;
  // Stationary, but altitude toggling by one 0.5m table step
  for (int i = 0; i < 100; i++, t += STEP) {
    v.update((i & 1) ? 50 : 0, t);
    if (i > 20) worst = std::max(worst, std::abs(v.value()));
  }
  EXPECT_LE(worst, 15);  // a two-point difference would give ~400 cm/s
}

TEST(FlySkyVSpeed, ToleratesSingleStep)
{
  FlySkyVSpeed v;
  tmr10ms_t t = 1000;
  int32_t worst = 0;
  // Stationary, altitude settles one 0.5m step higher (the wander seen on a
  // CAT-01 at rest). Unsmoothed this peaks at ~0.7 m/s.
  for (int i = 0; i < 100; i++, t += STEP) {
    v.update(i < 50 ? 0 : 50, t);
    if (i > 20) worst = std::max(worst, std::abs(v.value()));
  }
  // A real 0.5m step can only be told apart from a climb by time, so a
  // brief ~0.55 m/s blip is the expected cost of a 1s window.
  EXPECT_LE(worst, 60);
}

TEST(FlySkyVSpeed, RestartsAfterGap)
{
  FlySkyVSpeed v;
  tmr10ms_t t = 1000;
  int32_t alt = 0;
  for (int i = 0; i < 60; i++, t += STEP, alt += STEP) v.update(alt, t);
  ASSERT_NE(0, v.value());

  // no samples for longer than MAX_INTERVAL: no bogus jump, restart at 0
  v.update(alt + 10000, t + FlySkyVSpeed::MAX_INTERVAL + 1);
  EXPECT_EQ(0, v.value());
}

static uint32_t ibusPressure(uint32_t pa, int tempDeciC)
{
  return ((uint32_t)(tempDeciC + 400) << 19) | (pa & 0x7FFFF);
}

static void sendAfhds2aPressure(uint32_t raw)
{
  // AC frame sensor: [id][instance][len][value LE32]
  const uint8_t packet[] = {0x41,
                            0x01,
                            0x04,
                            (uint8_t)raw,
                            (uint8_t)(raw >> 8),
                            (uint8_t)(raw >> 16),
                            (uint8_t)(raw >> 24)};
  processFlySkySensor(packet, 0xAC);
}

static int findSensor(const char* label)
{
  for (int i = 0; i < MAX_TELEMETRY_SENSORS; i++) {
    if (!strncmp(g_model.telemetrySensors[i].label, label, TELEM_LABEL_LEN))
      return i;
  }
  return -1;
}

// Restore global state even if an ASSERT bails out early; other tests (Lua)
// depend on the clock.
struct TelemetryStateGuard {
  uint32_t tmr = g_tmr10ms;
  TelemetryStateGuard() { allowNewSensors = true; }
  ~TelemetryStateGuard()
  {
    allowNewSensors = false;
    g_tmr10ms = tmr;
  }
};

TEST(FlySkyIBus, PressureSensorProducesVSpd)
{
  TelemetryStateGuard guard;
  MODEL_RESET();
  for (auto& item : telemetryItems) item.clear();

  // Start well clear of any earlier sample so the decoder restarts
  g_tmr10ms += 100000;
  uint32_t pa = 101320;
  sendAfhds2aPressure(ibusPressure(pa, 200));

  int alt = findSensor("Alt");
  int vspd = findSensor("VSpd");
  ASSERT_GE(alt, 0);
  ASSERT_GE(vspd, 0);
  EXPECT_EQ(UNIT_METERS_PER_SECOND, g_model.telemetrySensors[vspd].unit);
  EXPECT_EQ(0, telemetryItems[vspd].value);

  // Climb: 6 Pa per 120ms is roughly 0.5m per sample, ~4 m/s
  for (int i = 0; i < 12; i++) {
    g_tmr10ms += STEP;
    sendAfhds2aPressure(ibusPressure(pa -= 6, 200));
  }
  EXPECT_GT(telemetryItems[vspd].value, 350);
  EXPECT_LT(telemetryItems[vspd].value, 500);

  // Sink at the same rate, long enough to fill the window
  for (int i = 0; i < 30; i++) {
    g_tmr10ms += STEP;
    sendAfhds2aPressure(ibusPressure(pa += 6, 200));
  }
  EXPECT_LT(telemetryItems[vspd].value, -350);
}

#endif
