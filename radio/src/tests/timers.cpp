/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
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

#include "gtests.h"

#if defined(SURFACE_RADIO)
#define THR_100    128      // approximately 100% full throttle
#define THR_50      96      // approximately 50% full throttle
#define THR_10      71      // approximately 10% full throttle
#define THR_0       64      // approximately 0% full throttle
#else
#define THR_100    128      // approximately 100% full throttle
#define THR_50      64      // approximately 50% full throttle
#define THR_10      13      // approximately 10% full throttle
#define THR_0        0      // approximately 0% full throttle
#endif

#define TEST_AB_EQUAL(a, b) if (a != b) { return ::testing::AssertionFailure() << \
                            #a "= " << (uint32_t)a << ", " << #b "= " << (uint32_t)b; };

void initModelTimer(uint32_t idx, uint8_t mode, int16_t start = 0)
{
  memset(&g_model.timers[idx], 0, sizeof(TimerData));
  g_model.timers[idx].mode = mode;
  g_model.timers[idx].start = start;
  g_model.timers[idx].countdownBeep = COUNTDOWN_SILENT;
}

/*
  Run timers for n seconds and test the end state
*/
::testing::AssertionResult evalTimersForNSecondsAndTest(unsigned int n, uint8_t throttle, uint32_t idx, uint8_t state, int value)
{
  unsigned int noLoops = n * 100;
  while (noLoops--) {
    evalTimers(throttle, 1);
  }
  TEST_AB_EQUAL(timersStates[idx].state, state);
  TEST_AB_EQUAL(timersStates[idx].val, value);
  return ::testing::AssertionSuccess();
}

TEST(Timers, timerReset)
{
  initModelTimer(0, TMRMODE_THR_REL, 200);
  timerReset(0);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(0, THR_100, 0, TMR_OFF, 200));

  initModelTimer(1, TMRMODE_THR_REL, 0);
  timerReset(1);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(0, THR_100, 1, TMR_OFF, 0));
}

TEST(Timers, timerSet)
{
  timerSet(0, 500);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(0, THR_100, 0, TMR_OFF, 500));

  timerSet(1, 300);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(0, THR_100, 1, TMR_OFF, 300));
}

TEST(Timers, timerGreaterThan9hours)
{
  initModelTimer(0, TMRMODE_ON, 0);
  timerSet(0, 0);
  // test with 24 hours
  EXPECT_TRUE(evalTimersForNSecondsAndTest(24*3600, THR_100, 0, TMR_RUNNING, 24*3600));
}

TEST(Timers, saveRestoreTimers)
{
  g_model.timers[0].persistent = 1;
  g_model.timers[1].persistent = 1;
  timerSet(0, 500);
  timerSet(1, 1500);
  saveTimers();
  EXPECT_EQ(g_model.timers[0].value, 500);
  EXPECT_EQ(g_model.timers[1].value, 1500);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(0, THR_100, 0, TMR_OFF,  500));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(0, THR_100, 1, TMR_OFF, 1500));

  timerReset(0);
  timerReset(1);
  restoreTimers();
  EXPECT_TRUE(evalTimersForNSecondsAndTest(0, THR_100, 0, TMR_OFF,  500));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(0, THR_100, 1, TMR_OFF, 1500));
}

TEST(Timers, timerOff)
{
  initModelTimer(0, TMRMODE_OFF, 0);
  timerReset(0);

  EXPECT_TRUE(evalTimersForNSecondsAndTest(0,  THR_100, 0, TMR_OFF,  0));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(10, THR_100, 0, TMR_OFF,  0));
}

TEST(Timers, timerAbsolute)
{
  initModelTimer(0, TMRMODE_ON, 0);
  timerReset(0);

  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,   THR_100, 0, TMR_RUNNING,    1));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_RUNNING,  101));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,   THR_0, 0, TMR_RUNNING,  201));

  // max timer value test
  timerSet(0, TIMER_MAX-10);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,   THR_100, 0, TMR_RUNNING, TIMER_MAX-9));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_RUNNING,   TIMER_MAX));

  // test down-running
  g_model.timers[0].start = 200;
  timerReset(0);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,   THR_100, 0, TMR_RUNNING,  199));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_RUNNING,   99));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_NEGATIVE,  -1));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_STOPPED, -101));

}

TEST(Timers, timerThrottle)
{
  initModelTimer(0, TMRMODE_THR, 0);
  timerReset(0);

  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,     THR_0, 0, TMR_RUNNING,   0));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_RUNNING, 100));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,  THR_10, 0, TMR_RUNNING, 200));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,   THR_0, 0, TMR_RUNNING, 200));

  // test down-running
  g_model.timers[0].start = 200;
  timerReset(0);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,   THR_100, 0, TMR_RUNNING, 199));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,     THR_0, 0, TMR_RUNNING, 199));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_RUNNING,  99));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,  THR_50, 0, TMR_NEGATIVE, -1));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_STOPPED,-101));
}

TEST(Timers, timerThrottleRelative)
{
  initModelTimer(0, TMRMODE_THR_REL, 0);

  timerReset(0);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,     THR_0, 0, TMR_RUNNING,   0));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_RUNNING, 100));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,  THR_50, 0, TMR_RUNNING, 150)); // 50% throttle == 50s
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,  THR_10, 0, TMR_RUNNING, 160)); // 10% throttle == 10s
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,   THR_0, 0, TMR_RUNNING, 160));

  // test down-running
  initModelTimer(0, TMRMODE_THR_REL, 200);
  timerReset(0);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,   THR_100, 0, TMR_RUNNING, 199));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,     THR_0, 0, TMR_RUNNING, 199));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_RUNNING,  99));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(200,  THR_50, 0, TMR_NEGATIVE, -1)); // 50% throttle == 100s
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,  THR_10, 0, TMR_NEGATIVE,-11)); // 10% throttle == 10s
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_100, 0, TMR_STOPPED,-111));
}

TEST(Timers, timerThrottleTriggered)
{
  initModelTimer(0, TMRMODE_THR_START, 0);

  timerReset(0);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,      THR_0, 0, TMR_OFF,       0));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_10-1, 0, TMR_OFF,       0));  // below threshold
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,   THR_50, 0, TMR_RUNNING, 100));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,  THR_100, 0, TMR_RUNNING, 200));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,    THR_0, 0, TMR_RUNNING, 300));

  // test down-running
  initModelTimer(0, TMRMODE_THR_START, 200);
  timerReset(0);
  EXPECT_TRUE(evalTimersForNSecondsAndTest(1,      THR_0, 0, TMR_OFF,     200));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100, THR_10-1, 0, TMR_OFF,     200));  // below threshold
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,  THR_100, 0, TMR_RUNNING, 100));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(101,   THR_50, 0, TMR_NEGATIVE, -1));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(10,     THR_0, 0, TMR_NEGATIVE,-11));
  EXPECT_TRUE(evalTimersForNSecondsAndTest(100,    THR_0, 0, TMR_STOPPED,-111));
}
