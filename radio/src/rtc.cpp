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

#include <limits.h>
#include "edgetx.h"

extern void rtcdriver_settime(struct gtm * t);

#define LEAP_SECONDS_POSSIBLE 0

/* Shift A right by B bits portably, by dividing A by 2**B and
   truncating towards minus infinity.  A and B should be free of side
   effects, and B should be in the range 0 <= B <= INT_BITS - 2, where
   INT_BITS is the number of useful bits in an int.  GNU code can
   assume that INT_BITS is at least 32.

   ISO C99 says that A >> B is implementation-defined if A < 0.  Some
   implementations (e.g., UNICOS 9.0 on a Cray Y-MP EL) don't shift
   right in the usual way when A < 0, so SHR falls back on division if
   ordinary A >> B doesn't seem to be the usual signed shift.  */
#define SHR(a, b) (-1 >> 1 == -1 ? (a) >> (b) : (a) / (1 << (b)) - ((a) % (1 << (b)) < 0))

/* The extra casts in the following macros work around compiler bugs,
   e.g., in Cray C 5.0.3.0.  */

/* True if the arithmetic type T is an integer type.  bool counts as
   an integer.  */
#define TYPE_IS_INTEGER(t) ((t) 1.5 == 1)

/* True if negative values of the signed integer type T use two's
   complement, ones' complement, or signed magnitude representation,
   respectively.  Much GNU code assumes two's complement, but some
   people like to be portable to all possible C hosts.  */
#define TYPE_TWOS_COMPLEMENT(t) ((t) ~ (t) 0 == (t) -1)
#define TYPE_ONES_COMPLEMENT(t) ((t) ~ (t) 0 == 0)
#define TYPE_SIGNED_MAGNITUDE(t) ((t) ~ (t) 0 < (t) -1)

/* True if the arithmetic type T is signed.  */
#define TYPE_SIGNED(t) (! ((t) 0 < (t) -1))

/* The maximum and minimum values for the integer type T.  These
   macros have undefined behavior if T is signed and has padding bits.
   If this is a problem for you, please let us know how to fix it for
   your host.  */
#define TYPE_MINIMUM(t) \
  ((t) (! TYPE_SIGNED (t) \
        ? (t) 0 \
        : TYPE_SIGNED_MAGNITUDE (t) \
        ? ~ (t) 0 \
        : ~ (t) 0 << (sizeof (t) * CHAR_BIT - 1)))
#define TYPE_MAXIMUM(t) \
  ((t) (! TYPE_SIGNED (t) \
        ? (t) -1 \
        : ~ (~ (t) 0 << (sizeof (t) * CHAR_BIT - 1))))

#ifndef TIME_T_MIN
# define TIME_T_MIN TYPE_MINIMUM (gtime_t)
#endif
#ifndef TIME_T_MAX
# define TIME_T_MAX TYPE_MAXIMUM (gtime_t)
#endif
#define TIME_T_MIDPOINT (SHR (TIME_T_MIN + TIME_T_MAX, 1) + 1)



static_assert(TYPE_IS_INTEGER(gtime_t), "gtime_t is not integer");
static_assert(TYPE_TWOS_COMPLEMENT(int), "twos complement arithmetic");
/* The code also assumes that signed integer overflow silently wraps
   around, but this assumption can't be stated without causing a
   diagnostic on some hosts.  */

#define EPOCH_YEAR 1970
static_assert(TM_YEAR_BASE % 100 == 0, "base year is not a multiple of 100");

/* Return 1 if YEAR + TM_YEAR_BASE is a leap year.  */
static inline int leapyear(long int year)
{
  /* Don't add YEAR to TM_YEAR_BASE, as that might overflow.
     Also, work even if YEAR is negative.  */
  return ((year & 3) == 0
          && (year % 100 != 0
              || ((year / 100) & 3) == (-(TM_YEAR_BASE / 100) & 3)));
}

const unsigned short int __mon_yday[2][13] = {
  /* Normal years.  */
  { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 },
  /* Leap years.  */
  { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 }
};

/* Compute the `struct tm' representation of *T,
   offset OFFSET seconds east of UTC,
   and store year, yday, mon, mday, wday, hour, min, sec into *TP.
   Return nonzero if successful.  */
int __offtime(const gtime_t * t, long int offset, struct gtm * tp)
{
  long int days, rem, y;
  const unsigned short int * ip;

  days = *t / SECS_PER_DAY;
  rem = *t % SECS_PER_DAY;
  rem += offset;
  while (rem < 0) {
    rem += SECS_PER_DAY;
    --days;
  }
  while (rem >= (long int)SECS_PER_DAY) {
    rem -= SECS_PER_DAY;
    ++days;
  }
  tp->tm_hour = rem / SECS_PER_HOUR;
  rem %= SECS_PER_HOUR;
  tp->tm_min = rem / 60;
  tp->tm_sec = rem % 60;
  /* January 1, 1970 was a Thursday.  */
  tp->tm_wday = (4 + days) % 7;
  if (tp->tm_wday < 0)
    tp->tm_wday += 7;
  y = 1970;

#define DIV(a, b) ((a) / (b) - ((a) % (b) < 0))
#define LEAPS_THRU_END_OF(y) (DIV(y, 4) - DIV(y, 100) + DIV(y, 400))

  while (days < 0 || days >= (leapyear(y) ? 366 : 365)) {
    /* Guess a corrected year, assuming 365 days per year.  */
    long int yg = y + days / 365 - (days % 365 < 0);

    /* Adjust DAYS and Y to match the guessed year.  */
    days -= ((yg - y) * 365 + LEAPS_THRU_END_OF(yg - 1) - LEAPS_THRU_END_OF(y - 1));
    y = yg;
  }
  tp->tm_year = y - TM_YEAR_BASE;
  if (tp->tm_year != y - TM_YEAR_BASE) {
    /* The year cannot be represented due to overflow.  */
    // __set_errno (EOVERFLOW);
    return 0;
  }
  tp->tm_yday = days;
  ip = __mon_yday[leapyear(y)];
  for (y = 11; days < (long int)ip[y]; --y)
    continue;
  days -= ip[y];
  tp->tm_mon = y;
  tp->tm_mday = days + 1;
  return 1;
}

/* time_r function implementations */
// G: No time zones in our implementation so just do the converion from gtime_t to struct tm
struct gtm * __localtime_r(const gtime_t * t, struct gtm * tp)
{
  __offtime(t, 0, tp);
  return tp;
}


/* Return an integer value measuring (YEAR1-YDAY1 HOUR1:MIN1:SEC1) -
   (YEAR0-YDAY0 HOUR0:MIN0:SEC0) in seconds, assuming that the clocks
   were not adjusted between the time stamps.

   The YEAR values uses the same numbering as TP->tm_year.  Values
   need not be in the usual range.  However, YEAR1 must not be less
   than 2 * INT_MIN or greater than 2 * INT_MAX.

   The result may overflow.  It is the caller's responsibility to
   detect overflow.  */

static inline gtime_t ydhms_diff(long int year1, long int yday1, int hour1, int min1, int sec1,
                                 int year0, int yday0, int hour0, int min0, int sec0)
{
  static_assert(-1 / 2 == 0, "no C99 integer division");
  static_assert(INT_MAX <= LONG_MAX / 2 || TIME_T_MAX <= INT_MAX, "long int year and yday are not wide enough");

  /* Compute intervening leap days correctly even if year is negative.
     Take care to avoid integer overflow here.  */
  int a4 = SHR(year1, 2) + SHR(TM_YEAR_BASE, 2) - !(year1 & 3);
  int b4 = SHR(year0, 2) + SHR(TM_YEAR_BASE, 2) - !(year0 & 3);
  int a100 = a4 / 25 - (a4 % 25 < 0);
  int b100 = b4 / 25 - (b4 % 25 < 0);
  int a400 = SHR(a100, 2);
  int b400 = SHR(b100, 2);
  int intervening_leap_days = (a4 - b4) - (a100 - b100) + (a400 - b400);

  /* Compute the desired time in gtime_t precision.  Overflow might
     occur here.  */
  gtime_t tyear1 = year1;
  gtime_t years = tyear1 - year0;
  gtime_t days = 365 * years + yday1 - yday0 + intervening_leap_days;
  gtime_t hours = 24 * days + hour1 - hour0;
  gtime_t minutes = 60 * hours + min1 - min0;
  gtime_t seconds = 60 * minutes + sec1 - sec0;
  return seconds;
}

/* Return a gtime_t value corresponding to (YEAR-YDAY HOUR:MIN:SEC),
   assuming that *T corresponds to *TP and that no clock adjustments
   occurred between *TP and the desired time.
   If TP is null, return a value not equal to *T; this avoids false matches.
   If overflow occurs, yield the minimal or maximal value, except do not
   yield a value equal to *T.  */
static gtime_t guess_time_tm(long int year, long int yday, int hour, int min, int sec,
                             gtime_t * t, struct gtm * tp)
{
  if (tp) {
    gtime_t d = ydhms_diff(year, yday, hour, min, sec,
                           tp->tm_year, tp->tm_yday,
                           tp->tm_hour, tp->tm_min, tp->tm_sec);
    gtime_t t1 = *t + d;
    if ((t1 < *t) == (TYPE_SIGNED(gtime_t) ? d < 0 : TIME_T_MAX / 2 < d))
      return t1;
  }

  /* Overflow occurred one way or another.  Return the nearest result
     that is actually in range, except don't report a zero difference
     if the actual difference is nonzero, as that would cause a false
     match; and don't oscillate between two values, as that would
     confuse the spring-forward gap detector.  */
  return (*t < TIME_T_MIDPOINT
          ? (*t <= TIME_T_MIN + 1 ? *t + 1 : TIME_T_MIN)
          : (TIME_T_MAX - 1 <= *t ? *t - 1 : TIME_T_MAX));
}

/* Use CONVERT to convert *T to a broken down time in *TP.
   If *T is out of range for conversion, adjust it so that
   it is the nearest in-range value and then convert that.  */
static struct gtm * ranged_convert(struct gtm *(*convert)(const gtime_t *, struct gtm *), gtime_t * t, struct gtm * tp)
{
  struct gtm * r = convert(t, tp);

  if (!r && *t) {
    gtime_t bad = *t;
    gtime_t ok = 0;

    /* BAD is a known unconvertible gtime_t, and OK is a known good one.
       Use binary search to narrow the range between BAD and OK until
       they differ by 1.  */
    while (bad != ok + (bad < 0 ? -1 : 1)) {
      gtime_t mid = *t = (bad < 0 ? bad + ((ok - bad) >> 1) : ok + ((bad - ok) >> 1));
      r = convert(t, tp);
      if (r) ok = mid;
      else  bad = mid;
    }

    if (!r && ok) {
      /* The last conversion attempt failed;
         revert to the most recent successful attempt.  */
      *t = ok;
      r = convert(t, tp);
    }
  }

  return r;
}

/* Convert *TP to a gtime_t value, inverting
   the monotonic and mostly-unit-linear conversion function CONVERT.
   Use *OFFSET to keep track of a guess at the offset of the result,
   compared to what the result would be for UTC without leap seconds.
   If *OFFSET's guess is correct, only one CONVERT call is needed.
   This function is external because it is used also by timegm.c.  */
gtime_t __mktime_internal(struct gtm * tp,
                          struct gtm *(*convert)(const gtime_t *, struct gtm *),
                          gtime_t * offset)
{
  gtime_t t, gt, t0, t1, t2;
  struct gtm tm;

  /* The maximum number of probes (calls to CONVERT) should be enough
     to handle any combinations of time zone rule changes, solar time,
     leap seconds, and oscillations around a spring-forward gap.
     POSIX.1 prohibits leap seconds, but some hosts have them anyway.  */
  int remaining_probes = 6;

  /* Time requested.  Copy it in case CONVERT modifies *TP; this can
     occur if TP is localtime's returned value and CONVERT is localtime.  */
  int sec = tp->tm_sec;
  int min = tp->tm_min;
  int hour = tp->tm_hour;
  int mday = tp->tm_mday;
  int mon = tp->tm_mon;
  int year_requested = tp->tm_year;

  /* Ensure that mon is in range, and set year accordingly.  */
  int mon_remainder = mon % 12;
  int negative_mon_remainder = mon_remainder < 0;
  int mon_years = mon / 12 - negative_mon_remainder;
  long int lyear_requested = year_requested;
  long int year = lyear_requested + mon_years;

  /* The other values need not be in range:
     the remaining code handles minor overflows correctly,
     assuming int and gtime_t arithmetic wraps around.
     Major overflows are caught at the end.  */

  /* Calculate day of year from year, month, and day of month.
     The result need not be in range.  */
  int mon_yday = ((__mon_yday[leapyear(year)][mon_remainder + 12 * negative_mon_remainder]) - 1);
  long int lmday = mday;
  long int yday = mon_yday + lmday;

  gtime_t guessed_offset = *offset;

  int sec_requested = sec;

/*
   if (LEAP_SECONDS_POSSIBLE)
    {
      // Handle out-of-range seconds specially,
      // since ydhms_tm_diff assumes every minute has 60 seconds.
      if (sec < 0)
        sec = 0;
      if (59 < sec)
        sec = 59;
    }
*/

  /* Invert CONVERT by probing.  First assume the same offset as last
     time.  */

  t0 = ydhms_diff(year, yday, hour, min, sec, EPOCH_YEAR - TM_YEAR_BASE, 0, 0, 0, -guessed_offset);

  if (TIME_T_MAX / INT_MAX / 366 / 24 / 60 / 60 < 3) {
    /* gtime_t isn't large enough to rule out overflows, so check
       for major overflows.  A gross check suffices, since if t0
       has overflowed, it is off by a multiple of TIME_T_MAX -
       TIME_T_MIN + 1.  So ignore any component of the difference
       that is bounded by a small value.  */

    /* Approximate log base 2 of the number of time units per
       biennium.  A biennium is 2 years; use this unit instead of
       years to avoid integer overflow.  For example, 2 average
       Gregorian years are 2 * 365.2425 * 24 * 60 * 60 seconds,
       which is 63113904 seconds, and rint (log2 (63113904)) is
       26.  */
    int ALOG2_SECONDS_PER_BIENNIUM = 26;
    int ALOG2_MINUTES_PER_BIENNIUM = 20;
    int ALOG2_HOURS_PER_BIENNIUM = 14;
    int ALOG2_DAYS_PER_BIENNIUM = 10;
    int LOG2_YEARS_PER_BIENNIUM = 1;

    int approx_requested_biennia =
      (SHR(year_requested, LOG2_YEARS_PER_BIENNIUM)
       - SHR(EPOCH_YEAR - TM_YEAR_BASE, LOG2_YEARS_PER_BIENNIUM)
       + SHR(mday, ALOG2_DAYS_PER_BIENNIUM)
       + SHR(hour, ALOG2_HOURS_PER_BIENNIUM)
       + SHR(min, ALOG2_MINUTES_PER_BIENNIUM)
       + (LEAP_SECONDS_POSSIBLE
          ? 0
          : SHR(sec, ALOG2_SECONDS_PER_BIENNIUM)));

    int approx_biennia = SHR(t0, ALOG2_SECONDS_PER_BIENNIUM);
    int diff = approx_biennia - approx_requested_biennia;
    int abs_diff = diff < 0 ? -diff : diff;

    /* IRIX 4.0.5 cc miscalculates TIME_T_MIN / 3: it erroneously
       gives a positive value of 715827882.  Setting a variable
       first then doing math on it seems to work.
       (ghazi@caip.rutgers.edu) */
    gtime_t time_t_max = TIME_T_MAX;
    gtime_t time_t_min = TIME_T_MIN;
    gtime_t overflow_threshold = (time_t_max / 3 - time_t_min / 3) >> ALOG2_SECONDS_PER_BIENNIUM;

    if (overflow_threshold < abs_diff) {
      /* Overflow occurred.  Try repairing it; this might work if
         the time zone offset is enough to undo the overflow.  */
      gtime_t repaired_t0 = -1 - t0;
      approx_biennia = SHR(repaired_t0, ALOG2_SECONDS_PER_BIENNIUM);
      diff = approx_biennia - approx_requested_biennia;
      abs_diff = diff < 0 ? -diff : diff;
      if (overflow_threshold < abs_diff)
        return -1;
      guessed_offset += repaired_t0 - t0;
      t0 = repaired_t0;
    }
  }

  /* Repeatedly use the error to improve the guess.  */
  for (t = t1 = t2 = t0;
       (gt = guess_time_tm(year, yday, hour, min, sec, &t, ranged_convert(convert, &t, &tm)), t != gt);
       t1 = t2, t2 = t, t = gt) {
    if (t == t1 && t != t2)
      goto offset_found;
    else if (--remaining_probes == 0)
      return -1;
  }

 offset_found:
  *offset = guessed_offset + t - t0;

  if (LEAP_SECONDS_POSSIBLE && sec_requested != tm.tm_sec) {
    /* Adjust time to reflect the tm_sec requested, not the normalized value.
       Also, repair any damage from a false match due to a leap second.  */
    int sec_adjustment = (sec == 0 && tm.tm_sec == 60) - sec;
    t1 = t + sec_requested;
    t2 = t1 + sec_adjustment;
    if (((t1 < t) != (sec_requested < 0))
        | ((t2 < t1) != (sec_adjustment < 0))
        | !convert(&t2, &tm))
      return -1;
    t = t2;
  }

  *tp = tm;
  return t;
}

/* Convert *TP to a gtime_t value.  */
gtime_t gmktime(struct gtm * tp)
{
  // no time zone stuff. Just do the math ;)
  static gtime_t localtime_offset;
  return __mktime_internal(tp, __localtime_r, &localtime_offset);
}

/* Fill a (struct tm) TP* from a given gtime_t time stamp */
gtime_t filltm(const gtime_t * t, struct gtm * tp)
{
  return __offtime(t, 0, tp);
}

gtime_t g_rtcTime;
uint8_t g_ms100 = 0; // global to allow time set function to reset to zero

void gettime(struct gtm * tm)
{
  filltm(&g_rtcTime, tm); // create a struct tm date/time structure from global unix time stamp
}

void rtcGetTime(struct gtm * t);

#define RTC_ADJUST_PERIOD      60   // how often RTC is checked for accuracy [seconds]
#define RTC_ADJUST_TRESHOLD    20   // how much clock must differ before adjustment is made [seconds]
/*
  Changes RTC date/time to the given UTC date/time if:
   * RTC_ADJUST_PERIOD seconds have elapsed from the last time this function adjusted the RTC clock
   * AND if actual RTC clock differs from the given clock by more than RTC_ADJUST_TRESHOLD seconds
   * Function does nothing for a minute around midnight, where date change could produce erroneous result
*/
uint8_t rtcAdjust(uint16_t year, uint8_t mon, uint8_t day, uint8_t hour, uint8_t min, uint8_t sec)
{
  static tmr10ms_t lastRtcAdjust = 0;
  if ((get_tmr10ms() - lastRtcAdjust) > (RTC_ADJUST_PERIOD * 100)) {
    lastRtcAdjust = get_tmr10ms();

    if (year == 0 || (hour == 0 && min == 0) || (hour == 23 && min == 59)) return 0;

    // convert given UTC time to local time (to seconds) and compare it with RTC
    struct gtm t;
    t.tm_year = year - TM_YEAR_BASE;
    t.tm_mon  = mon - 1;
    t.tm_mday = day;
    t.tm_hour = hour;
    t.tm_min  = min;
    t.tm_sec  = sec;
    gtime_t newTime = gmktime(&t) + timezoneOffsetSeconds(g_eeGeneral.timezone, g_eeGeneral.timezoneMinutes);
    gtime_t diff = (g_rtcTime > newTime) ? (g_rtcTime - newTime) : (newTime - g_rtcTime);

#if defined(DEBUG) && defined (PCBTARANIS)
    struct gtm utm;
    rtcGetTime(&utm);
    gtime_t rtcTime = gmktime(&utm);
    TRACE("rtc: %d, grtc: %d, gps: %d, diff: %d, ", rtcTime, g_rtcTime, newTime, diff);
#endif

    if (diff > RTC_ADJUST_TRESHOLD) {
      // convert newTime to struct gtm and set RTC clock
      filltm(&newTime, &t);
      g_rtcTime = gmktime(&t); // update local timestamp and get wday calculated
      rtcSetTime(&t);
      TRACE("RTC clock adjusted to %04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
      // TODO perhaps some kind of audio notification ???
      return 1;
    }
  }
  return 0;
}

bool rtcIsValid()
{
  struct gtm t;
  gettime(&t);

  // If year is 2000, rtc circuit aren't working
  // No RTC battery or other issue
  if (t.tm_year <= 100)
    return false;

  return true;
}
